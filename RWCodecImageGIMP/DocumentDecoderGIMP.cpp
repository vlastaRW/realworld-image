// DocumentDecoderGIMP.cpp : Implementation of CDocumentDecoderGIMP

#include "stdafx.h"
#include "DocumentDecoderGIMP.h"

#include "MultiLanguageString.h"


// CDocumentDecoderGIMP

template<typename T>
T ReadGIMPNumber(BYTE const* a_p)
{
	BYTE t[sizeof(T)];
	for (BYTE i = 0; i < sizeof(T); ++i)
		t[sizeof(T)-1-i] = a_p[i];
	return *reinterpret_cast<T*>(t);
}

bool ReadGIMPString(BYTE const*& a_pData, BYTE const* const a_pDataEnd, BSTR* a_pbstr)
{
	*a_pbstr = NULL;
	if (a_pData+4 > a_pDataEnd) return false;
	ULONG nLen = ReadGIMPNumber<ULONG>(a_pData);
	a_pData += 4;
	if (nLen == 0) return true;
	if (a_pData+nLen > a_pDataEnd) return false;
	ULONG nWLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLen-1, NULL, 0);
	*a_pbstr = ::SysAllocStringLen(NULL, nWLen);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLen-1, *a_pbstr, nWLen+1);
	a_pData += nLen;
	return true;
}

typedef enum
{
  PROP_END                =  0,
  PROP_COLORMAP           =  1,
  PROP_ACTIVE_LAYER       =  2,
  PROP_ACTIVE_CHANNEL     =  3,
  PROP_SELECTION          =  4,
  PROP_FLOATING_SELECTION =  5,
  PROP_OPACITY            =  6,
  PROP_MODE               =  7,
  PROP_VISIBLE            =  8,
  PROP_LINKED             =  9,
  PROP_LOCK_ALPHA         = 10,
  PROP_APPLY_MASK         = 11,
  PROP_EDIT_MASK          = 12,
  PROP_SHOW_MASK          = 13,
  PROP_SHOW_MASKED        = 14,
  PROP_OFFSETS            = 15,
  PROP_COLOR              = 16,
  PROP_COMPRESSION        = 17,
  PROP_GUIDES             = 18,
  PROP_RESOLUTION         = 19,
  PROP_TATTOO             = 20,
  PROP_PARASITES          = 21,
  PROP_UNIT               = 22,
  PROP_PATHS              = 23,
  PROP_USER_UNIT          = 24,
  PROP_VECTORS            = 25,
  PROP_TEXT_LAYER_FLAGS   = 26,
  PROP_SAMPLE_POINTS      = 27,
  PROP_LOCK_CONTENT       = 28,
  PROP_GROUP_ITEM         = 29,
  PROP_ITEM_PATH          = 30
} PropType;

typedef enum
{
  COMPRESS_NONE              =  0,
  COMPRESS_RLE               =  1,
  COMPRESS_ZLIB              =  2,  /* unused */
  COMPRESS_FRACTAL           =  3   /* unused */
} XcfCompressionType;

bool ReadGIMPTile(BYTE const* a_pData, BYTE a_bCompression, BYTE* a_pTile, ULONG a_nTileSizeX, ULONG a_nTileSizeY, ULONG a_nChannels)
{
	if (a_bCompression == COMPRESS_NONE)
	{
		CopyMemory(a_pTile, a_pData, a_nTileSizeX*a_nTileSizeY*a_nChannels);
		return true;
	}
	if (a_bCompression != COMPRESS_RLE)
		return false;
	for (ULONG nCh = 0; nCh < a_nChannels; ++nCh)
	{
		BYTE* pDst = a_pTile+nCh;
		BYTE* const pDstEnd = pDst+a_nTileSizeX*a_nTileSizeY*a_nChannels;
		while (pDst < pDstEnd)
		{
			if (*a_pData < 128)
			{
				ULONG nLen = *a_pData < 127 ? (*a_pData)+1 : a_pData[1]*256+a_pData[2];
				a_pData += *a_pData < 127 ? 1 : 3;
				if (pDst+nLen <= pDstEnd)
				{
					for (; nLen > 0; --nLen, pDst+=a_nChannels)
						*pDst = *a_pData;
					++a_pData;
				}
				else
				{
					return false;
				}
			}
			else
			{
				ULONG nLen = *a_pData > 128 ? 256-(*a_pData) : a_pData[1]*256+a_pData[2];
				a_pData += *a_pData > 128 ? 1 : 3;
				if (pDst+nLen <= pDstEnd)
				{
					for (; nLen > 0; --nLen, pDst+=a_nChannels, ++a_pData)
						*pDst = *a_pData;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return true;
}

void PlaceTile(TPixelChannel* a_pLayer, ULONG a_nLayerX, ULONG a_nLayerY, LONG a_nOffX, LONG a_nOffY, BYTE const* a_pTile, ULONG a_nTileX, ULONG a_nTileY, ULONG a_nColorType, BYTE const* a_pPal)
{
	if (a_nOffX+LONG(a_nTileX) < 0 || a_nOffX > LONG(a_nLayerX) ||
		a_nOffY+LONG(a_nTileY) < 0 || a_nOffY > LONG(a_nLayerY))
		return; // tile is out of layer
	ULONG nBPP = 0;
	switch (a_nColorType)
	{
	case 0: nBPP = 3; break;
	case 1: nBPP = 4; break;
	case 2: nBPP = 1; break;
	case 3: nBPP = 2; break;
	case 4: nBPP = 1; break;
	case 5: nBPP = 2; break;
	case 6: nBPP = 1; break; // layer mask
	}
	if (a_nOffY < 0)
	{
		a_pTile += a_nTileX*nBPP*(-a_nOffY);
		a_nTileY -= a_nOffY;
		a_nOffY = 0;
	}
	if (a_nTileY+a_nOffY > a_nLayerY)
		a_nTileY = a_nLayerY-a_nOffY;
	for (ULONG nY = 0; nY < a_nTileY; ++nY)
	{
		TPixelChannel* const pD = a_pLayer+(a_nOffY+nY)*a_nLayerX;
		BYTE const* pS = a_pTile+a_nTileX*nBPP*nY;
		for (ULONG nX = 0; nX < a_nTileX; ++nX, pS+=nBPP)
		{
			if (LONG(a_nOffX+nX) >= 0 && LONG(a_nOffX+nX) < LONG(a_nLayerX))
			{
				TPixelChannel* const p = pD+a_nOffX+nX;
				switch (a_nColorType)
				{
				case 0: p->bR = pS[0]; p->bG = pS[1]; p->bB = pS[2]; p->bA = 255; break;
				case 1: p->bR = pS[0]; p->bG = pS[1]; p->bB = pS[2]; p->bA = pS[3]; break;
				case 2: p->bR = p->bG = p->bB = pS[0]; p->bA = 255; break;
				case 3: p->bR = p->bG = p->bB = pS[0]; p->bA = pS[1]; break;
				case 4: {BYTE const* pPal = a_pPal+pS[0]; p->bR = pPal[0]; p->bG = pPal[1]; p->bB = pPal[2];} p->bA = 255; break;
				case 5: {BYTE const* pPal = a_pPal+pS[0]; p->bR = pPal[0]; p->bG = pPal[1]; p->bB = pPal[2];} p->bA = pS[1]; break;
				case 6: p->bA = ULONG(p->bA)*pS[0]/255; break;
				}
			}
		}
	}
}

HRESULT CDocumentDecoderGIMP::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
{
	try
	{

		if (a_nLen < 14+12)
			return E_RW_UNKNOWNINPUTFORMAT;

		if (memcmp(a_pData, "gimp xcf file", 14) != 0 &&
			memcmp(a_pData, "gimp xcf v001", 14) != 0 &&
			memcmp(a_pData, "gimp xcf v002", 14) != 0)
			return E_RW_UNKNOWNINPUTFORMAT;
		ULONG nSizeX = ReadGIMPNumber<ULONG>(a_pData+14);
		ULONG nSizeY = ReadGIMPNumber<ULONG>(a_pData+18);
		ULONG eColorType = ReadGIMPNumber<ULONG>(a_pData+22);
		if (eColorType > 2)
			return E_RW_UNSUPPORTEDINPUTFORMAT;

		float fResX = 100.0f;
		float fResY = 100.0f;
		BYTE bCompression = COMPRESS_RLE;
		ULONG nPalette = 0;
		BYTE const* pPalette = NULL;

		BYTE const* const pDataEnd = a_pData+a_nLen;
		BYTE const* pData = a_pData+26;
		bool bBreak = false;
		while (pData+8 <= pDataEnd && !bBreak)
		{
			ULONG eProp = ReadGIMPNumber<ULONG>(pData);
			pData += 4;
			ULONG nLen = ReadGIMPNumber<ULONG>(pData);
			pData += 4;
			switch (eProp)
			{
			case PROP_COMPRESSION:
				if (pData+1 <= pDataEnd)
					bCompression = *pData;
				break;
			case PROP_RESOLUTION:
				if (pData+8 <= pDataEnd)
				{
					fResX = ReadGIMPNumber<float>(pData);
					fResY = ReadGIMPNumber<float>(pData+4);
				}
				break;
			case PROP_COLORMAP:
				if (pData+4 <= pDataEnd)
				{
					nPalette = ReadGIMPNumber<ULONG>(pData);
					pPalette = pData+4;
					nLen = nPalette*3+4;
				}
				break;
			case PROP_END:
				bBreak = true;
				break;
			}
			pData += nLen;
		}

		if (pData+8 > pDataEnd)
			return E_FAIL;

		std::vector<ULONG> aLayerOff;
		while (pData+4 <= pDataEnd)
		{
			ULONG nOff = ReadGIMPNumber<ULONG>(pData);
			pData += 4;
			if (nOff)
				aLayerOff.push_back(nOff);
			else
				break;
		}
		// channel offsets
		while (pData+4 <= pDataEnd)
		{
			ULONG nOff = ReadGIMPNumber<ULONG>(pData);
			pData += 4;
			if (nOff == 0)
				break;
		}

		a_pBuilder->Create(a_bstrPrefix, a_pBase);

		CAutoVectorPtr<TPixelChannel> pLayer;
		ULONG nLayer = 0;
		CAutoVectorPtr<BYTE> pTile(new BYTE[64*64*4]);

		for (std::vector<ULONG>::const_iterator i = aLayerOff.begin(); i != aLayerOff.end(); ++i)
		{
			BYTE const* pData = a_pData+*i;
			if (pData+12 > pDataEnd)
				continue;
			ULONG nLaySizeX = ReadGIMPNumber<ULONG>(pData);
			ULONG nLaySizeY = ReadGIMPNumber<ULONG>(pData+4);
			ULONG nLayType = ReadGIMPNumber<ULONG>(pData+8);
			pData += 12;
			ULONG nBPP = 0;
			switch (nLayType)
			{
			case 0: nBPP = 3; break;
			case 1: nBPP = 4; break;
			case 2: nBPP = 1; break;
			case 3: nBPP = 2; break;
			case 4: nBPP = 1; break;
			case 5: nBPP = 2; break;
			}
			if (nBPP == 0)
				continue;
			CComBSTR bstrName;
			if (!ReadGIMPString(pData, pDataEnd, &bstrName))
				continue;

			TImageLayer tProps;
			ZeroMemory(&tProps, sizeof tProps);
			tProps.eBlend = EBEAlphaBlend;

			bool bFloating = false;
			ULONG nOffX = 0;
			ULONG nOffY = 0;

			bBreak = false;
			while (pData+8 <= pDataEnd && !bBreak)
			{
				ULONG eProp = ReadGIMPNumber<ULONG>(pData);
				pData += 4;
				ULONG nLen = ReadGIMPNumber<ULONG>(pData);
				pData += 4;
				switch (eProp)
				{
				case PROP_FLOATING_SELECTION:
					bFloating = true;
					break;
				case PROP_MODE:
					switch (ReadGIMPNumber<ULONG>(pData))
					{
					case 3: tProps.eBlend = EBEModulate; break;
					case 4: tProps.eBlend = EBEScreen; break;
					case 5: tProps.eBlend = EBEOverlay; break;
					case 6: tProps.eBlend = EBEDifference; break;
					case 7: tProps.eBlend = EBEAdd; break;
					case 8: tProps.eBlend = EBESubtract; break;
					case 9: tProps.eBlend = EBEMinimum; break;
					case 10: tProps.eBlend = EBEMaximum; break;
					case 11: tProps.eBlend = ELBHLSReplaceHue; break;
					case 12: tProps.eBlend = ELBHLSReplaceSaturation; break;
					case 13: tProps.eBlend = ELBHLSReplaceColor; break;
					case 14: tProps.eBlend = ELBHLSReplaceLuminance; break;
					case 16: tProps.eBlend = EBEMaximum; break;
					case 17: tProps.eBlend = EBEMinimum; break;
					case 18: tProps.eBlend = EBEOverlay; break;
					case 19: tProps.eBlend = EBEOverlay; break;
					}
					break;
				case PROP_OFFSETS:
					nOffX = ReadGIMPNumber<ULONG>(pData);
					nOffY = ReadGIMPNumber<ULONG>(pData+4);
					break;
				case PROP_OPACITY:
					tProps.fOpacity = int(ReadGIMPNumber<ULONG>(pData)/0.255f+0.5f)*0.001f;
					break;
				case PROP_VISIBLE:
					tProps.bVisible = ReadGIMPNumber<ULONG>(pData) ? 1 : 0;
					break;
				case PROP_END:
					bBreak = true;
					break;
				}
				pData += nLen;
			}
			if (bFloating)
				continue; // ignore floating selection

			ULONG nDataOff = ReadGIMPNumber<ULONG>(pData);
			ULONG nMaskOff = ReadGIMPNumber<ULONG>(pData+4);
			pData = a_pData+nDataOff;
			if (pData+20 > pDataEnd)
				continue; // incomplete file, read at least some layers
			ULONG nCheckSizeX = ReadGIMPNumber<ULONG>(pData);
			ULONG nCheckSizeY = ReadGIMPNumber<ULONG>(pData+4);
			ULONG nCheckBPP = ReadGIMPNumber<ULONG>(pData+8);
			ULONG nLevelsPtr = ReadGIMPNumber<ULONG>(pData+12);
			if (nCheckSizeX != nLaySizeX || nCheckSizeY != nLaySizeY || nCheckBPP !=nBPP)
				continue;

			ULONG nMaskLevelPtr = 0;
			if (nMaskOff)
			{
				pData = a_pData+nMaskOff;
				if (pData+8 > pDataEnd)
					continue;
				ULONG nMaskSizeX = ReadGIMPNumber<ULONG>(pData);
				ULONG nMaskSizeY = ReadGIMPNumber<ULONG>(pData+4);
				pData += 8;
				CComBSTR bstr;
				if (!ReadGIMPString(pData, pDataEnd, &bstr))
					continue;
				bBreak = false;
				while (pData+8 <= pDataEnd && !bBreak)
				{
					ULONG eProp = ReadGIMPNumber<ULONG>(pData);
					pData += 4;
					ULONG nLen = ReadGIMPNumber<ULONG>(pData);
					pData += 4;
					switch (eProp)
					{
					case PROP_END:
						bBreak = true;
						break;
					}
					pData += nLen;
				}
				if (pData+4 <= pDataEnd)
					nMaskLevelPtr = ReadGIMPNumber<ULONG>(pData);
			}

			if (nLayer < nLaySizeX*nLaySizeY)
			{
				pLayer.Free();
				pLayer.Attach(new TPixelChannel[nLaySizeX*nLaySizeY]);
				nLayer = nLaySizeX*nLaySizeY;
			}

			ULONG const nTilesX = ((nLaySizeX+63)>>6);
			ULONG const nTilesY = ((nLaySizeY+63)>>6);
			pData = a_pData+nLevelsPtr;
			if (pData+8+4*nTilesX*nTilesY > pDataEnd)
				continue;
			pData += 8;
			BYTE const* pMaskData = nMaskLevelPtr ? a_pData+ReadGIMPNumber<ULONG>(a_pData+nMaskLevelPtr+12)+8 : NULL;
			for (ULONG nTY = 0; nTY < nTilesY; ++nTY)
			{
				ULONG nTileSizeY = nTY == nTilesY-1 ? nLaySizeY-nTY*64 : 64;
				for (ULONG nTX = 0; nTX < nTilesX; ++nTX)
				{
					ULONG nTileSizeX = nTX == nTilesX-1 ? nLaySizeX-nTX*64 : 64;
					ReadGIMPTile(a_pData+ReadGIMPNumber<ULONG>(pData), bCompression, pTile, nTileSizeX, nTileSizeY, nBPP);
					pData += 4;
					PlaceTile(pLayer, nLaySizeX, nLaySizeY, nTX<<6, nTY<<6, pTile, nTileSizeX, nTileSizeY, nLayType, pPalette);
					if (pMaskData)
					{
						ReadGIMPTile(a_pData+ReadGIMPNumber<ULONG>(pMaskData), bCompression, pTile, nTileSizeX, nTileSizeY, 1);
						pMaskData += 4;
						PlaceTile(pLayer, nLaySizeX, nLaySizeY, nTX<<6, nTY<<6, pTile, nTileSizeX, nTileSizeY, 6, NULL);
					}
				}
			}
			HRESULT hRes = a_pBuilder->AddLayer(a_bstrPrefix, a_pBase, 1, &CImageLayerCreatorRasterImage(CImageSize(nSizeX, nSizeY), NULL, 0.0f, CPixelChannel(0, 0, 0, 0), pLayer, CImagePoint(nOffX, nOffY), CImageSize(nLaySizeX, nLaySizeY)), bstrName, &tProps, NULL);
			if (FAILED(hRes)) return hRes;
		}
		a_pBuilder->SetSize(a_bstrPrefix, a_pBase, CImageSize(nSizeX, nSizeY));
		if (a_pEncoderID) *a_pEncoderID = __uuidof(DocumentEncoderGIMP);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
