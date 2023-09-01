// DocumentEncoderGIMP.cpp : Implementation of CDocumentEncoderGIMP

#include "stdafx.h"
#include "DocumentEncoderGIMP.h"

#include "RWDocumentImageRaster.h"
#include <MultiLanguageString.h>


// CDocumentEncoderGIMP

STDMETHODIMP CDocumentEncoderGIMP::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorGIMP::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentEncoderGIMP::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		return S_FALSE;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderGIMP::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentLayeredImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDocImg));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA ENCFEAT_IMAGE_LAYER);
		return pDocImg ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CLayerOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
BEGIN_COM_MAP(CLayerOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_pState)
			m_cStates[a_bstrCategoryName] = a_pState;
		else
			m_cStates.erase(a_bstrCategoryName);
		return S_OK;
	}
	STDMETHOD(IsCancelled)()
	{
		return S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = 0;
		if (a_pItemsRemaining) *a_pItemsRemaining = 0;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		return S_FALSE;
	}

private:
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	CStates m_cStates;
};

template<ULONG t_nInitial = 32768>
class CBlockStreamWriter
{
public:
	CBlockStreamWriter() : m_pBuffer(NULL), m_nAllocated(0), m_nWritten()
	{
	}

	template<bool t_bReverse>
	void Write(ULONG a_nSize, BYTE const* a_pData)
	{
		if (a_nSize == 0)
			return;
		if (a_nSize+m_nWritten > m_nAllocated)
		{
			ULONG nNewSize = max(m_nAllocated << 1, t_nInitial);
			while (a_nSize+m_nWritten > nNewSize)
				nNewSize <<= 1;
			BYTE* p = new BYTE[nNewSize];
			if (m_pBuffer)
			{
				memcpy(p, m_pBuffer, m_nWritten);
				delete[] m_pBuffer;
			}
			m_pBuffer = p;
			m_nAllocated = nNewSize;
		}
		if (t_bReverse)
		{
			BYTE* pD = m_pBuffer+m_nWritten+a_nSize-1;
			for (BYTE const* const pEnd = a_pData+a_nSize; a_pData != pEnd; ++a_pData, --pD)
				*pD = *a_pData;
		}
		else
		{
			memcpy(m_pBuffer+m_nWritten, a_pData, a_nSize);
		}
		m_nWritten += a_nSize;
	}
	ULONG Pointer() const { return m_nWritten; }
	void WriteAt(ULONG a_nAt, ULONG a_nData)
	{
		ATLASSERT(a_nAt+4 <= m_nWritten);
		m_pBuffer[a_nAt] = a_nData>>24;
		m_pBuffer[a_nAt+1] = a_nData>>16;
		m_pBuffer[a_nAt+2] = a_nData>>8;
		m_pBuffer[a_nAt+3] = a_nData;
	}
	void WriteLenAt(ULONG a_nAt)
	{
		WriteAt(a_nAt, m_nWritten-a_nAt-4);
	}
	HRESULT ToStream(IReturnedData* a_pDst)
	{
		return a_pDst->Write(m_nWritten, m_pBuffer);
	}

	CBlockStreamWriter& operator<< (char const* a_psz)
	{
		static DWORD const b0 = 0;
		if (a_psz == NULL || *a_psz == '\0')
		{
			Write<false>(4, reinterpret_cast<BYTE const*>(&b0));
		}
		else
		{
			ULONG nLen = strlen(a_psz)+1;
			Write<true>(4, reinterpret_cast<BYTE const*>(&nLen));
			Write<false>(nLen, reinterpret_cast<BYTE const*>(a_psz));
		}
		return *this;
	}
	CBlockStreamWriter& operator<< (ULONG a_n)
	{
		Write<true>(sizeof(a_n), reinterpret_cast<BYTE const*>(&a_n));
		return *this;
	}
	CBlockStreamWriter& operator<< (float a_f)
	{
		Write<true>(sizeof(a_f), reinterpret_cast<BYTE const*>(&a_f));
		return *this;
	}
	//CBlockStreamWriter& operator<< (WORD a_n)
	//{
	//	Write<true>(sizeof(a_n), reinterpret_cast<BYTE const*>(&a_n));
	//	return *this;
	//}
	CBlockStreamWriter& operator<< (BYTE a_b)
	{
		Write<false>(sizeof(a_b), &a_b);
		return *this;
	}
	//CBlockStreamWriter& operator<< (bool a_b)
	//{
	//	BYTE b = a_b;
	//	Write<false>(sizeof(b), &b);
	//	return *this;
	//}

private:
	BYTE* m_pBuffer;
	ULONG m_nAllocated;
	ULONG m_nWritten;
};

template<class T>
void CompressTile(BYTE const* data, ULONG size, T& cdst)
{
	int  state  = 0;
	int  length = 0;
	int  count  = 0;
	ULONG last   = -1;

	while (size > 0)
	{
		switch (state)
		{
		case 0:
			/* in state 0 we try to find a long sequence of
			*  matching values.
			*/
			if ((length == 32768) ||
				((size - length) <= 0) ||
				((length > 1) && (last != *data)))
			{
				count += length;

				if (length >= 128)
					cdst << BYTE(127) << BYTE(length >> 8) << BYTE(length&0x00ff) << BYTE(last);
				else
					cdst << BYTE(length - 1) << BYTE(last);

				size -= length;
				length = 0;
			}
			else if ((length == 1) && (last != *data))
			{
				state = 1;
			}
			break;

		case 1:
			/* in state 1 we try and find a long sequence of
			*  non-matching values.
			*/
			if ((length == 32768) ||
				((size - length) == 0) ||
				((length > 0) && (last == *data) &&
				((size - length) == 1 || last == data[1])))
			{
				count += length;
				state = 0;

				if (length >= 128)
					cdst << BYTE(255 - 127) << BYTE(length >> 8) << BYTE(length&0x00ff);
				else
					cdst << BYTE(255 - (length - 1));

				cdst.Write<false>(length, data-length);

				size -= length;
				length = 0;
			}
			break;
		}

		if (size > 0)
		{
			++length;
			last = *data;
			++data;
		}
	}

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

typedef enum
{
  XCF_ORIENTATION_HORIZONTAL = 1,
  XCF_ORIENTATION_VERTICAL   = 2
} XcfOrientationType;

typedef enum
{
  XCF_STROKETYPE_STROKE        = 0,
  XCF_STROKETYPE_BEZIER_STROKE = 1
} XcfStrokeType;

STDMETHODIMP CDocumentEncoderGIMP::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentLayeredImage> pLI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
		CComPtr<IEnumUnknowns> pLayers;
		pLI->LayersEnum(NULL, &pLayers);
		ULONG nLayers = 0;
		pLayers->Size(&nLayers);
		if (nLayers == 0)
			return E_INVALIDARG;
		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		TImageSize tSize;
		pI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		ULONG const nImgSizeX = tSize.nX;
		ULONG const nImgSizeY = tSize.nY;
		CAutoVectorPtr<CComPtr<IComparable> > aLayerIDs(new CComPtr<IComparable>[nLayers]);
		for (ULONG i = 0; i < nLayers; ++i)
			pLayers->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&(aLayerIDs[i])));

		CBlockStreamWriter<> cdst;
		cdst.Write<false>(14, reinterpret_cast<BYTE const*>("gimp xcf file"));
		cdst << nImgSizeX << nImgSizeY << 0UL; // width, height, rgb color
		cdst << ULONG(PROP_COMPRESSION) << 1UL << BYTE(1); // RLE compression
		cdst << ULONG(PROP_RESOLUTION) << 8UL << 100.0f << 100.0f; // resolution in DPI
		cdst << ULONG(PROP_TATTOO) << 4UL << (1UL+nLayers); // max. tattoo in the file
		cdst << ULONG(PROP_UNIT) << 4UL << 1UL; // use inches in user interface
		cdst << ULONG(PROP_PARASITES);
		ULONG nParasitesPos = cdst.Pointer();
		cdst << 0UL;
		cdst << "gimp-image-grid" << 1UL << "(style solid)\n"
			"(fgcolor (color-rgba 0.000000 0.000000 0.000000 1.000000))\n"
			"(bgcolor (color-rgba 1.000000 1.000000 1.000000 1.000000))\n"
			"(xspacing 10.000000)\n"
			"(yspacing 10.000000)\n"
			"(spacing-unit inches)\n"
			"(xoffset 0.000000)\n"
			"(yoffset 0.000000)\n"
			"(offset-unit inches)\n";
		cdst.WriteLenAt(nParasitesPos);
		cdst << ULONG(PROP_END) << 0UL;
		ULONG nLayersPos = cdst.Pointer();
		for (ULONG i = 0; i < nLayers; ++i)
			cdst << 0UL; // layer offsets
		cdst << 0UL; // end of layers
		cdst << 0UL; // end of channels (no channels specified)

		CComPtr<IOperationManager> pOM;

		for (ULONG i = 0; i < nLayers; ++i)
		{
			IComparable* pItem = aLayerIDs[i];

			CComPtr<ISubDocumentID> pSDID;
			pLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pSubDoc;
			pSDID->SubDocumentGet(&pSubDoc);
			CComPtr<IDocumentImage> pSubImage;
			pSubDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pSubImage));

			TImageLayer tLayerProps;
			pLI->LayerPropsGet(pItem, &tLayerProps.eBlend, &tLayerProps.bVisible);

			CComPtr<IConfig> pConfig;
			pLI->LayerEffectGet(pItem, &pConfig, &tLayerProps.fOpacity);
			CComBSTR bstrID(L"Effect");
			CConfigValue cEffectID;
			if (pConfig) pConfig->ItemValueGet(bstrID, &cEffectID);

			if (cEffectID.TypeGet() == ECVTGUID && !IsEqualGUID(__uuidof(DocumentOperationNULL), cEffectID))
			{
				TImagePoint tSubPos = {0, 0};
				TImageSize tSubSize = {0, 0};
				TImageSize tSize = {0, 0};
				TImageResolution tRes = {100, 254, 100, 254};
				pSubImage->CanvasGet(&tSize, &tRes, &tSubPos, &tSubSize, NULL);
				CAutoVectorPtr<TPixelChannel> cBuffer(tSubSize.nX*tSubSize.nY ? new TPixelChannel[tSubSize.nX*tSubSize.nY] : NULL);
				pSubImage->TileGet(EICIRGBA, &tSubPos, &tSubSize, NULL, tSubSize.nX*tSubSize.nY, cBuffer, NULL, EIRIAccurate);
				float fGamma = 0.0f;
				CPixelChannel tDef;
				pSubImage->ChannelsGet(NULL, &fGamma, &CImageChannelDefaultGetter(EICIRGBA, &tDef));

				CComPtr<IConfig> pEffect;
				pConfig->SubConfigGet(bstrID, &pEffect);

				CComPtr<IDocument> pTmpDoc;
				RWCoCreateInstance(pTmpDoc, __uuidof(DocumentBase));
				CComPtr<IDocumentFactoryRasterImage> pFactory;
				RWCoCreateInstance(pFactory, __uuidof(DocumentFactoryRasterImage));
				TImageTile tTile = {EICIRGBA, tSubPos, tSubSize, {1, tSubSize.nX}, tSubSize.nX*tSubSize.nY, cBuffer};
				pFactory->Create(NULL, CComQIPtr<IDocumentBase>(pTmpDoc), &tSize, &tRes, 1, CChannelDefault(EICIRGBA, tDef), fGamma, &tTile);

				CComObjectStackEx<CLayerOperationContext> cLOC;
				if (pOM == NULL) RWCoCreateInstance(pOM, __uuidof(OperationManager));
				if (pOM) pOM->Activate(pOM, pTmpDoc, cEffectID, pEffect, &cLOC, NULL, 0);

				CComPtr<IDocumentImage> pTmpImage;
				pTmpDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pTmpImage));
				pSubImage = pTmpImage;
				pSubDoc = pTmpDoc;
			}

			TImagePoint tSubPos = {0, 0};
			TImageSize tSubSize = {0, 0};
			pSubImage->CanvasGet(NULL, NULL, &tSubPos, &tSubSize, NULL);
			if (tSubSize.nX*tSubSize.nY == 0)
			{
				// GIMP crashes when it encounters layer with zero size -> create layer filled with background
				tSubPos.nX = tSubPos.nY = 0;
				tSubSize = tSize;
			}

			cdst.WriteAt(nLayersPos+i*4, cdst.Pointer());
			cdst << tSubSize.nX << tSubSize.nY << 1UL; // RGBA
			CComBSTR bstrName;
			pLI->LayerNameGet(pItem, &bstrName);
			CW2AEX<> strName(bstrName, CP_UTF8);
			cdst << static_cast<char const*>(strName);
			if (i == 0)
				cdst << ULONG(PROP_ACTIVE_LAYER) << 0UL;

			cdst << ULONG(PROP_OPACITY) << 4UL << ULONG(tLayerProps.fOpacity*255.0f+0.5f);
			cdst << ULONG(PROP_VISIBLE) << 4UL << (tLayerProps.bVisible ? 1UL : 0UL);
			cdst << ULONG(PROP_LINKED) << 4UL << 0UL;
			cdst << ULONG(PROP_LOCK_ALPHA) << 4UL << 0UL;
			cdst << ULONG(PROP_APPLY_MASK) << 4UL << 0UL;
			cdst << ULONG(PROP_EDIT_MASK) << 4UL << 0UL;
			cdst << ULONG(PROP_SHOW_MASK) << 4UL << 0UL;
			cdst << ULONG(PROP_OFFSETS) << 8UL << ULONG(tSubPos.nX) << ULONG(tSubPos.nY);
			ULONG nBlendMode = 0;
			switch (tLayerProps.eBlend)
			{
			case EBEModulate:	nBlendMode = 3; break;
			case EBEScreen:		nBlendMode = 4; break;
			case EBEAdd:		nBlendMode = 7; break;
			case EBESubtract:	nBlendMode = 8; break;
			case EBEDifference:	nBlendMode = 6; break;
			case EBEMinimum:	nBlendMode = 17; break;
			case EBEMaximum:	nBlendMode = 16; break;
			case EBEOverlay:	nBlendMode = 5; break;
			case ELBHLSReplaceHue:			nBlendMode = 11; break;
			case ELBHLSReplaceSaturation:	nBlendMode = 12; break;
			case ELBHLSReplaceLuminance:	nBlendMode = 14; break;
			case ELBHLSReplaceColor:		nBlendMode = 13; break;
			}
			cdst << ULONG(PROP_MODE) << 4UL << nBlendMode;
			cdst << ULONG(PROP_TATTOO) << 4UL << (2UL+i);
			cdst << ULONG(PROP_END) << 0UL;

			cdst << cdst.Pointer()+8UL;
			cdst << 0UL; // no layer mask

			cdst << tSubSize.nX << tSubSize.nY << 4UL;
			// dummy image sizes
			ULONG nLayerLevelPos = cdst.Pointer();
			cdst << 0UL;
			for (ULONG n = max(tSubSize.nX, tSubSize.nY); n > 64; n >>= 1)
				cdst << 0UL;
			cdst << 0UL; // end

			cdst.WriteAt(nLayerLevelPos, cdst.Pointer());
			cdst << tSubSize.nX << tSubSize.nY;
			ULONG nLayerTilePos = cdst.Pointer();
			for (ULONG n = ((tSubSize.nX+63)>>6)*((tSubSize.nY+63)>>6); n > 0; --n)
				cdst << 0UL;
			cdst << 0UL; // end

			CAutoVectorPtr<BYTE> cBuffer(new BYTE[tSubSize.nX*tSubSize.nY<<2]);
			pSubImage->TileGet(EICIRGBA, &tSubPos, &tSubSize, NULL, tSubSize.nX*tSubSize.nY, reinterpret_cast<TPixelChannel*>(cBuffer.m_p), NULL, EIRIAccurate);

			ULONG nTile = 0;
			BYTE aSrc[64*64];
			for (ULONG nY = 0; nY < tSubSize.nY; nY+=64)
			{
				for (ULONG nX = 0; nX < tSubSize.nX; nX+=64, ++nTile)
				{
					cdst.WriteAt(nLayerTilePos+4*nTile, cdst.Pointer());
					for (ULONG nCh2 = 0; nCh2 < 4; ++nCh2)
					{
						ULONG const nCh = nCh2 == 3 ? 3 : 2-nCh2;
						BYTE* pD = aSrc;
						ULONG const nSX = min(tSubSize.nX, nX+64)-nX;
						ULONG const nSY = min(tSubSize.nY, nY+64)-nY;
						for (ULONG y = 0; y < nSY; ++y)
						{
							BYTE* pS = cBuffer.m_p+nCh+((nY+y)*tSubSize.nX+nX)*4;
							for (ULONG x = 0; x < nSX; ++x, ++pD, pS+=4)
								*pD = *pS;
						}
						CompressTile(aSrc, nSX*nSY, cdst);
					}
				}
			}

			{
				ULONG nX = tSubSize.nX;
				ULONG nY = tSubSize.nY;
				ULONG nI = 4;
				while (nX > 64 && nY > 64)
				{
					nX >>= 1;
					nY >>= 1;
					cdst.WriteAt(nLayerLevelPos+nI, cdst.Pointer());
					nI += 4;
					cdst << nX << nY << 0UL;
				}
			}
		}
		return cdst.ToStream(a_pDst);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

