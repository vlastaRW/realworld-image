// DocumentDecoderBMP.cpp : Implementation of CDocumentDecoderBMP

#include "stdafx.h"
#include "DocumentDecoderBMP.h"
#include "ConfigIDsBMPConvertor.h"

#include "MultiLanguageString.h"


void inline MaskToOffsetAndWidth(DWORD a_nMask, BYTE* a_pOffset, BYTE* a_pWidth)
{
	ATLASSERT(a_pOffset);
	ATLASSERT(a_pWidth);

	if (a_nMask == 0)
	{
		*a_pOffset = 0;
		*a_pWidth = 0;
	}
	else
	{
		for (*a_pOffset = 0; (a_nMask&1) == 0; (*a_pOffset)++)
		{
			a_nMask = a_nMask>>1;
		}
		for (*a_pWidth = 0; a_nMask; (*a_pWidth)++)
		{
			a_nMask = a_nMask>>1;
		}
	}
}

// CDocumentDecoderBMP

#include "../RWCodecImageJPEG/RWImageCodecJPEG.h"
#include "../RWCodecImagePNG/RWImageCodecPNG.h"

HRESULT CDocumentDecoderBMP::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
{
	if (a_nLen < (sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)))
		return E_RW_UNKNOWNINPUTFORMAT;

	BITMAPFILEHEADER const* pFileHeader = reinterpret_cast<BITMAPFILEHEADER const*>(a_pData);
	if (pFileHeader->bfType != 0x4d42 || a_nLen < pFileHeader->bfSize)
		return E_RW_UNKNOWNINPUTFORMAT;
	BYTE const* pSrc = reinterpret_cast<BYTE const*>(pFileHeader)+pFileHeader->bfOffBits;

	BITMAPINFOHEADER const* pInfoHeader = reinterpret_cast<BITMAPINFOHEADER const*>(pFileHeader+1);
	CAutoVectorPtr<BYTE> pShadow;
	if (pInfoHeader->biSize == 12) // actually  an OS/2 header
	{
		struct OS2Header {DWORD size; WORD width, height, planes, bits;};
		OS2Header const* pOS2 = reinterpret_cast<OS2Header const*>(pInfoHeader);
		ULONG const nPal = (pFileHeader->bfOffBits-12)/3;
		pShadow.Allocate(sizeof(BITMAPINFOHEADER)+nPal*4);
		BITMAPINFOHEADER* pHeader = reinterpret_cast<BITMAPINFOHEADER*>(pShadow.m_p);
		pHeader->biSize = sizeof(BITMAPINFOHEADER);
		pHeader->biWidth = pOS2->width;
		pHeader->biHeight = pOS2->height;
		pHeader->biPlanes = pOS2->planes;
		pHeader->biBitCount = pOS2->bits;
		pHeader->biCompression = BI_RGB;
		pHeader->biClrUsed = pHeader->biClrImportant = nPal;
		RGBQUAD* pPal = reinterpret_cast<RGBQUAD*>(reinterpret_cast<BYTE*>(pHeader)+pHeader->biSize);
		BYTE const* pPS = reinterpret_cast<BYTE const*>(pOS2+1);
		for (ULONG i = 0; i < nPal; ++i, ++pPal, pPS+=3)
		{
			pPal->rgbBlue = pPS[0];
			pPal->rgbGreen = pPS[1];
			pPal->rgbRed = pPS[2];
			pPal->rgbReserved = 0;
		}
		pInfoHeader = pHeader;
	}

	if (pInfoHeader->biCompression == BI_JPEG)
	{
		CComPtr<IDocumentDecoder> pJPEGDecoder;
		RWCoCreateInstance(pJPEGDecoder, __uuidof(DocumentDecoderJPEG));
		if (pJPEGDecoder == NULL)
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		HRESULT hRes = pJPEGDecoder->Parse(a_nLen-pFileHeader->bfOffBits, pSrc, a_pLocation, 1, reinterpret_cast<IDocumentBuilder*const*>(&a_pBuilder), a_bstrPrefix, a_pBase, NULL, NULL, a_pControl);
		if (SUCCEEDED(hRes) && a_pEncoderID) *a_pEncoderID = __uuidof(DocumentEncoderBMP);
		return hRes;
	}
	if (pInfoHeader->biCompression == BI_PNG)
	{
		CComPtr<IDocumentDecoder> pPNGDecoder;
		RWCoCreateInstance(pPNGDecoder, __uuidof(DocumentDecoderPNG));
		if (pPNGDecoder == NULL)
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		HRESULT hRes = pPNGDecoder->Parse(a_nLen-pFileHeader->bfOffBits, pSrc, a_pLocation, 1, reinterpret_cast<IDocumentBuilder*const*>(&a_pBuilder), a_bstrPrefix, a_pBase, NULL, NULL, a_pControl);
		if (SUCCEEDED(hRes) && a_pEncoderID) *a_pEncoderID = __uuidof(DocumentEncoderBMP);
		return hRes;
	}

	TImageSize const tSize = {pInfoHeader->biWidth, abs(pInfoHeader->biHeight)};
	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);

	//size_t const nRowDelta = pInfoHeader->biHeight < 0 ? static_cast<size_t>(m_cImageFormat.GetDimSize(1)) : -static_cast<size_t>(m_cImageFormat.GetDimSize(1));
	//BYTE * const pDst = pInfoHeader->biHeight < 0 ? pDstOrig : (pDstOrig + m_cImageFormat.GetDimSize(1)*(m_cImageFormat->atDims[1].nItems-1));

	RGBQUAD const* const pPal = reinterpret_cast<RGBQUAD const*>(reinterpret_cast<BYTE const*>(pInfoHeader)+pInfoHeader->biSize);
	LONG nFormat = CFGVAL_BMPFORMAT_32BIT;

	switch (pInfoHeader->biBitCount)
	{
	case 24:
		if (pInfoHeader->biCompression != BI_RGB)
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		nFormat = CFGVAL_BMPFORMAT_24BIT;
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
			BYTE const* pS = pSrc+y*((pInfoHeader->biWidth*3+3)&0xfffffffc);
			for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, pS+=3)
			{
				pD->bB = pS[0];
				pD->bG = pS[1];
				pD->bR = pS[2];
				pD->bA = 0xff;
			}
		}
		break;

	case 8:
		switch (pInfoHeader->biCompression)
		{
		case BI_RGB:
			nFormat = CFGVAL_BMPFORMAT_8BIT;
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
				BYTE const* pS = pSrc+y*((pInfoHeader->biWidth+3)&0xfffffffc);
				for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, ++pS)
				{
					RGBQUAD const* pPixSrc = pPal + *pS;
					pD->bR = pPixSrc->rgbRed;
					pD->bG = pPixSrc->rgbGreen;
					pD->bB = pPixSrc->rgbBlue;
					pD->bA = 0xff;
				}
			}
			break;

		case BI_RLE8:
			nFormat = CFGVAL_BMPFORMAT_8BITRLE;
			{
				int nRepeat = 0;
				RGBQUAD const* pRepeat = NULL;
				int nCopy = 0;
				BYTE const* pCopyIndex = NULL;
				int nSkip = 0;
				for (ULONG y = 0; y < tSize.nY; ++y)
				{
					TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
					for (ULONG j = 0; j < tSize.nX;)
					{
						if (nRepeat+nCopy+nSkip == 0)
						{
							if (*pSrc != 0)
							{
								nRepeat = *(pSrc++);
								pRepeat = pPal + *(pSrc++);
							}
							else
							{
								if (pSrc[1] == 0) // line end
								{
									pSrc += 2;
									if (j != 0)
										break;
									else
										continue;
								}
								else if (pSrc[1] == 1) // image end (lazy ... do not advance in the source data and end line)
								{
									break;
								}
								else if (pSrc[1] == 2) // delta
								{
									nSkip = pSrc[3]*tSize.nX+pSrc[2];
									pSrc += 4;
								}
								else
								{
									nCopy = pSrc[1];
									pCopyIndex = pSrc+2;
									pSrc += (2+nCopy+1)&(~1);
								}
							}
						}
						if (nRepeat)
						{
							pD->bR = pRepeat->rgbRed;
							pD->bG = pRepeat->rgbGreen;
							pD->bB = pRepeat->rgbBlue;
							pD->bA = 0xff;
							--nRepeat;
						}
						else if (nCopy)
						{
							RGBQUAD const* const p = pPal + *(pCopyIndex++);
							pD->bR = p->rgbRed;
							pD->bG = p->rgbGreen;
							pD->bB = p->rgbBlue;
							pD->bA = 0xff;
							--nCopy;
						}
						else
						{
							pD->n = 0;
							--nSkip;
						}
						++pD;
						++j;
					}
				}
			}
			break;

		default:
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		}
		break;

	case 4:
		switch (pInfoHeader->biCompression)
		{
		case BI_RGB:
			nFormat = CFGVAL_BMPFORMAT_4BIT;
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
				BYTE const* pS = pSrc+y*((((pInfoHeader->biWidth+1)>>1)+3)&0xfffffffc);
				bool bLowPart = false;
				BYTE bSrc = 0;
				for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD)
				{
					RGBQUAD const* pPixSrc;
					if (bLowPart)
					{
						pPixSrc = pPal + bSrc;
						bLowPart = false;
					}
					else
					{
						pPixSrc = pPal + ((*pS)>>4);
						bSrc = ((*(pS++))&15);
						bLowPart = true;
					}
					pD->bR = pPixSrc->rgbRed;
					pD->bG = pPixSrc->rgbGreen;
					pD->bB = pPixSrc->rgbBlue;
					pD->bA = 0xff;
				}
			}
			break;

		case BI_RLE4:
			nFormat = CFGVAL_BMPFORMAT_4BITRLE;
			{
				int nRepeat = 0;
				RGBQUAD const* pRepeat1 = NULL;
				RGBQUAD const* pRepeat2 = NULL;
				int nCopy = 0;
				BYTE const* pCopyIndex = NULL;
				bool bLowPart = false;
				int nSkip = 0;
				for (ULONG y = 0; y < tSize.nY; ++y)
				{
					TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
					for (ULONG j = 0; j < tSize.nX;)
					{
						if (nRepeat+nCopy+nSkip == 0)
						{
							if (*pSrc != 0)
							{
								nRepeat = *(pSrc++);
								pRepeat1 = pPal + (pSrc[0]>>4);
								pRepeat2 = pPal + (pSrc[0]&15);
								++pSrc;
								bLowPart = false;
							}
							else
							{
								if (pSrc[1] == 0) // line end
								{
									pSrc += 2;
									if (j != 0)
										break;
									else
										continue;
								}
								else if (pSrc[1] == 1) // image end (lazy ... do not advance in the source data and end line)
								{
									break;
								}
								else if (pSrc[1] == 2) // delta
								{
									nSkip = pSrc[3]*tSize.nX+pSrc[2];
									pSrc += 4;
								}
								else
								{
									nCopy = pSrc[1];
									pCopyIndex = pSrc+2;
									pSrc += 2 + (((nCopy+3)&(~3))>>1);
									bLowPart = false;
								}
							}
						}
						if (nRepeat)
						{
							if (bLowPart)
							{
								pD->bR = pRepeat2->rgbRed;
								pD->bG = pRepeat2->rgbGreen;
								pD->bB = pRepeat2->rgbBlue;
								pD->bA = 0xff;
								bLowPart = false;
							}
							else
							{
								pD->bR = pRepeat1->rgbRed;
								pD->bG = pRepeat1->rgbGreen;
								pD->bB = pRepeat1->rgbBlue;
								pD->bA = 0xff;
								bLowPart = true;
							}
							--nRepeat;
						}
						else if (nCopy)
						{
							RGBQUAD const* const p = pPal + (bLowPart ? (pCopyIndex[0]&15) : (pCopyIndex[0]>>4));
							pD->bR = p->rgbRed;
							pD->bG = p->rgbGreen;
							pD->bB = p->rgbBlue;
							pD->bA = 0xff;
							pCopyIndex += bLowPart ? 1 : 0;
							bLowPart = !bLowPart;
							--nCopy;
						}
						else
						{
							static TPixelChannel const t0 = {0, 0, 0, 0};
							*pD = t0;
							--nSkip;
						}
						++pD;
						++j;
					}
					nRepeat = 0;
				}
			}
			break;

		default:
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		}
		break;

	case 1:
		nFormat = CFGVAL_BMPFORMAT_1BIT;
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
			BYTE const* pS = pSrc+y*((((pInfoHeader->biWidth+7)>>3)+3)&0xfffffffc);
			WORD bSrc = 0x8000;
			for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD)
			{
				if (bSrc == 0x8000)
					bSrc = 0x80|(WORD(*(pS++))<<8);
				RGBQUAD const* const pPixSrc = pPal + (bSrc>>15);
				pD->bR = pPixSrc->rgbRed;
				pD->bG = pPixSrc->rgbGreen;
				pD->bB = pPixSrc->rgbBlue;
				pD->bA = 0xff;
				bSrc <<= 1;
			}
		}
		break;

	case 32:
		switch (pInfoHeader->biCompression)
		{
		case BI_RGB:
			nFormat = CFGVAL_BMPFORMAT_32BIT;
			{
				bool bAllTransparent = true;
				bool bAllBlack = true;
				for (ULONG y = 0; y < tSize.nY; ++y)
				{
					TPixelChannel* pD = reinterpret_cast<TPixelChannel*>(pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX);
					TPixelChannel const* pS = reinterpret_cast<TPixelChannel const*>(pSrc+y*(pInfoHeader->biWidth<<2));
					CopyMemory(pD, pS, sizeof(TPixelChannel)*tSize.nX);
					if (bAllTransparent)
					{
						for (TPixelChannel* const end = pD+tSize.nX; pD != end; ++pD)
						{
							if (pD->bA)
							{
								bAllTransparent = false;
								break;
							}
							if (bAllBlack && (pD->n&0xffffff))
								bAllBlack = false;
						}
					}
				}
				if (bAllTransparent && !bAllBlack)
				{ // completely transparent image with some content -> make it opaque
					TPixelChannel* p = pBuffer.m_p;
					for (TPixelChannel* const end = p+tSize.nY*tSize.nX; p != end; ++p)
						p->bA = 0xff;
				}
			}
			break;

		case BI_BITFIELDS:
		case 6://BI_ALPHABITFIELDS:
			nFormat = CFGVAL_BMPFORMAT_32BIT;
			//nFormat = CFGVAL_BMPFORMAT_BITFIELDS;
			{
				TPixelFormat tPF;
				tPF.nSizeX = pInfoHeader->biWidth;
				tPF.nSizeY = abs(pInfoHeader->biHeight);
				tPF.nStrideX = 4;
				tPF.nStrideY = tPF.nStrideX*tPF.nSizeX;
				BYTE const* pRow = pSrc;
				if (pInfoHeader->biHeight > 0)
				{
					pRow += tPF.nStrideY*(tPF.nSizeY-1);
					tPF.nStrideY = -tPF.nStrideY;
				}
				DWORD const* pMasks = reinterpret_cast<DWORD const*>(reinterpret_cast<BYTE const*>(pInfoHeader)+sizeof(*pInfoHeader));
				MaskToOffsetAndWidth(pMasks[2], &tPF.tBlue.nOffset, &tPF.tBlue.nWidth);
				MaskToOffsetAndWidth(pMasks[1], &tPF.tGreen.nOffset, &tPF.tGreen.nWidth);
				MaskToOffsetAndWidth(pMasks[0], &tPF.tRed.nOffset, &tPF.tRed.nWidth);
				if (reinterpret_cast<DWORD const*>(pSrc) >= pMasks+4)
					MaskToOffsetAndWidth(pMasks[3], &tPF.tAlpha.nOffset, &tPF.tAlpha.nWidth);
				else
					tPF.tAlpha.nOffset = tPF.tAlpha.nWidth = 0;
				CComPtr<IPixelFormatConverter> pPFC;
				RWCoCreateInstance(pPFC, __uuidof(PixelFormatConverter));
				TPixelFormat tDst = tPF;
				tDst.nStrideX = 4;
				tDst.nStrideY = 4*tDst.nSizeX;
				tDst.tRed.nWidth = tDst.tGreen.nWidth = tDst.tBlue.nWidth = tDst.tAlpha.nWidth = 8;
				tDst.tRed.nOffset = 16;
				tDst.tGreen.nOffset = 8;
				tDst.tBlue.nOffset = 0;
				tDst.tAlpha.nOffset = 24;
				pPFC->Convert(&tPF, tPF.nStrideY*tPF.nSizeY, pRow, &tDst, tDst.nStrideY*tDst.nSizeY, reinterpret_cast<BYTE*>(pBuffer.m_p));
				if (tPF.tAlpha.nWidth == 0)
				{ // completely transparent image with some content -> make it opaque
					TPixelChannel* p = pBuffer.m_p;
					for (TPixelChannel* const end = p+tSize.nY*tSize.nX; p != end; ++p)
						p->bA = 0xff;
				}
			}
			break;

		default:
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		}
		break;

	case 16:
		switch (pInfoHeader->biCompression)
		{
		case BI_RGB:
			nFormat = CFGVAL_BMPFORMAT_16BIT;
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				DWORD* pD = reinterpret_cast<DWORD*>(pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX);
				WORD const* pS = reinterpret_cast<WORD const*>(pSrc+y*(((pInfoHeader->biWidth<<1)+3)&0xfffffffc));
				for (DWORD* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, ++pS)
				{
					DWORD const dw = *pS;
					*pD = 0xff000000|((dw&0x1f)<<3)|((dw&0x1c)>>2)|((dw&0x3e0)<<6)|((dw&0x380)<<1)|((dw&0x7c00)<<9)|((dw&0x7000)<<4);
				}
			}
			break;

		case BI_BITFIELDS:
		case 6://BI_ALPHABITFIELDS:
			nFormat = CFGVAL_BMPFORMAT_16BIT;
			//nFormat = CFGVAL_BMPFORMAT_BITFIELDS;
			{
				TPixelFormat tPF;
				tPF.nSizeX = pInfoHeader->biWidth;
				tPF.nSizeY = abs(pInfoHeader->biHeight);
				tPF.nStrideX = 2;
				tPF.nStrideY = tPF.nStrideX*((tPF.nSizeX+1)&0xfffffffe);
				BYTE const* pRow = pSrc;
				if (pInfoHeader->biHeight > 0)
				{
					pRow += tPF.nStrideY*(tPF.nSizeY-1);
					tPF.nStrideY = -tPF.nStrideY;
				}
				DWORD const* pMasks = reinterpret_cast<DWORD const*>(reinterpret_cast<BYTE const*>(pInfoHeader)+sizeof(*pInfoHeader));
				MaskToOffsetAndWidth(pMasks[2]&0xffff, &tPF.tBlue.nOffset, &tPF.tBlue.nWidth);
				MaskToOffsetAndWidth(pMasks[1]&0xffff, &tPF.tGreen.nOffset, &tPF.tGreen.nWidth);
				MaskToOffsetAndWidth(pMasks[0]&0xffff, &tPF.tRed.nOffset, &tPF.tRed.nWidth);
				if (reinterpret_cast<DWORD const*>(pSrc) >= pMasks+4)
					MaskToOffsetAndWidth(pMasks[3]&0xffff, &tPF.tAlpha.nOffset, &tPF.tAlpha.nWidth);
				else
					tPF.tAlpha.nOffset = tPF.tAlpha.nWidth = 0;
				CComPtr<IPixelFormatConverter> pPFC;
				RWCoCreateInstance(pPFC, __uuidof(PixelFormatConverter));
				TPixelFormat tDst = tPF;
				tDst.nStrideX = 4;
				tDst.nStrideY = 4*tDst.nSizeX;
				tDst.tRed.nWidth = tDst.tGreen.nWidth = tDst.tBlue.nWidth = tDst.tAlpha.nWidth = 8;
				tDst.tRed.nOffset = 16;
				tDst.tGreen.nOffset = 8;
				tDst.tBlue.nOffset = 0;
				tDst.tAlpha.nOffset = 24;
				pPFC->Convert(&tPF, tPF.nStrideY*tPF.nSizeY, pRow, &tDst, tDst.nStrideY*tDst.nSizeY, reinterpret_cast<BYTE*>(pBuffer.m_p));
				if (tPF.tAlpha.nWidth == 0)
				{ // completely transparent image with some content -> make it opaque
					TPixelChannel* p = pBuffer.m_p;
					for (TPixelChannel* const end = p+tSize.nY*tSize.nX; p != end; ++p)
						p->bA = 0xff;
				}
				else
					nFormat = CFGVAL_BMPFORMAT_32BIT;
			}
			break;

		default:
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		}
		break;

	case 2:
		switch (pInfoHeader->biCompression)
		{
		case BI_RGB:
			nFormat = CFGVAL_BMPFORMAT_4BIT;
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				TPixelChannel* pD = pInfoHeader->biHeight < 0 ? pBuffer.m_p+y*tSize.nX : pBuffer.m_p+(tSize.nY-1-y)*tSize.nX;
				BYTE const* pS = pSrc+y*((((pInfoHeader->biWidth+3)>>2)+3)&0xfffffffc);
				int nShift = 0;
				for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD)
				{
					RGBQUAD const* const pPixSrc = pPal+(((*pS)>>(6-nShift))&3);
					pD->bR = pPixSrc->rgbRed;
					pD->bG = pPixSrc->rgbGreen;
					pD->bB = pPixSrc->rgbBlue;
					pD->bA = 0xff;
					nShift += 2;
					pS += (nShift>>3);
					nShift &= 6;
				}
			}
			break;

		default:
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		}
		break;

	default:
		return E_RW_UNKNOWNINPUTFORMAT;
	}

	HRESULT hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 0.0f, CImageTile(tSize.nX, tSize.nY, pBuffer));
	if (FAILED(hRes)) return hRes;
	if (a_pEncoderID) *a_pEncoderID = __uuidof(DocumentEncoderBMP);
	if (a_ppEncoderCfg)
	{
		*a_ppEncoderCfg = CDocumentEncoderBMP::Config();
		CComBSTR bstr(CFGID_BMPFORMAT);
		(*a_ppEncoderCfg)->ItemValuesSet(1, &(bstr.m_str), CConfigValue(nFormat));
	}

	return S_OK;
}

