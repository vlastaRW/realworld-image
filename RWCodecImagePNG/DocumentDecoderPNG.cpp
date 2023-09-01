// DocumentDecoderPNG.cpp : Implementation of CDocumentDecoderPNG

#include "stdafx.h"
#include "DocumentDecoderPNG.h"

#include <MultiLanguageString.h>
#include "PNGDataSource.h"
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>
#include <RWDocumentAnimationUtils.h>
//#include "libpng/pngpriv.h"


void PNGAPI PNGErrorFnc(png_structp, png_const_charp);
void PNGAPI PNGWarningFnc(png_structp, png_const_charp);

struct CAutoDeleteReadStruct
{
public:
	CAutoDeleteReadStruct(png_structp png_ptr_, png_infop info_ptr_) :
		png_ptr(png_ptr_), info_ptr(info_ptr_)
	{
	}
	~CAutoDeleteReadStruct()
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	}

private:
	png_structp png_ptr;
	png_infop info_ptr;
};

// CDocumentDecoderPNG

STDMETHODIMP CDocumentDecoderPNG::Priority(ULONG* a_pnPriority)
{
	try
	{
		*a_pnPriority = EDPAverage;
		return S_OK;
	}
	catch (...)
	{
		return a_pnPriority ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderPNG::DocumentType(IDocumentType** a_ppDocumentType)
{
	try
	{
		*a_ppDocumentType = NULL;
		*a_ppDocumentType = CDocumentTypeCreatorPNG::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderPNG::IsCompatible(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
{
	try
	{
		for (ULONG i = 0; i < a_nBuilders; ++i)
		{
			CComPtr<IDocumentFactoryRasterImage> pI;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryRasterImage), reinterpret_cast<void**>(&pI));
			if (pI) return S_OK;
			CComPtr<IDocumentFactoryAnimation> pA;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryAnimation), reinterpret_cast<void**>(&pA));
			if (pA) return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

BYTE To8Bits(ULONG val, ULONG depth)
{
	while (depth < 8)
	{
		val |= val<<depth;
		depth <<= 1;
	}
	if (depth == 8)
		return val;
	return val>>(depth-8);
}

STDMETHODIMP CDocumentDecoderPNG::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		if (a_nLen < 4 || a_pData == NULL || png_sig_cmp(const_cast<png_bytep>(a_pData), (png_size_t)0, 4))
			return E_RW_UNKNOWNINPUTFORMAT;

		CComPtr<IDocumentFactoryAnimation> pAni;
		CComPtr<IDocumentFactoryRasterImage> pRIF;
		for (ULONG i = 0; i < a_nBuilders && pRIF == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pRIF);
		for (ULONG i = 0; i < a_nBuilders && pAni == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pAni);
		if (pAni == NULL && pRIF == NULL)
			return E_FAIL;

		CPNGDataSource cSrc(a_nLen, a_pData);

		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, PNGErrorFnc, PNGWarningFnc);
		png_infop info_ptr = png_create_info_struct(png_ptr);
		CAutoDeleteReadStruct cAutoDelete(png_ptr, info_ptr);
		png_set_read_fn(png_ptr, (void *)&cSrc, CPNGDataSource::PngRWPtr);

		png_read_info(png_ptr, info_ptr);

		ULONG const nFrames = png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL) ? png_get_num_frames(png_ptr, info_ptr) : 1;
		ULONG const nPlays = png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL) ? png_get_num_plays(png_ptr, info_ptr) : 0;

		png_uint_32 nWidth = 0;
		png_uint_32 nHeight = 0;
		int nBitDepth = 0;
		int nColorType = 0;
		int nInterlaceType = 0;

		png_get_IHDR(png_ptr, info_ptr, &nWidth, &nHeight, &nBitDepth, &nColorType, &nInterlaceType, NULL, NULL);
		if (nWidth == 0 || nHeight == 0)
			return E_RW_UNKNOWNINPUTFORMAT;

		// expand paletted colors into true RGB triplets
		switch (nColorType)
		{
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(png_ptr); // png_set_expand <- ???
			//if ((nColorType&PNG_COLOR_MASK_ALPHA) == 0)
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) == 0)
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			break;
		case PNG_COLOR_TYPE_GRAY:
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			png_set_gray_to_rgb(png_ptr);
		case PNG_COLOR_TYPE_RGB:
		case PNG_COLOR_TYPE_RGB_ALPHA:
			if ((nColorType&PNG_COLOR_MASK_ALPHA) == 0)
				png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
			png_set_expand(png_ptr);
			if (nBitDepth == 16)
				png_set_strip_16(png_ptr);
				//png_set_swap(png_ptr);
			break;
		default:
			return E_FAIL; // unsupported color format
		}
		png_set_bgr(png_ptr);

		int nSRGBIntent;
  		if (!png_get_sRGB(png_ptr, info_ptr, &nSRGBIntent))
		{
			double fImgGamma;
  			if (png_get_gAMA(png_ptr, info_ptr, &fImgGamma) && (fImgGamma > 0.454555 || fImgGamma < 0.454535))
				png_set_gamma(png_ptr, 2.2, fImgGamma);
		}
	
		//png_read_reinit(png_ptr, info_ptr);

		TImageSize const tSize = {nWidth, nHeight};
		CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);

		CAutoVectorPtr<png_bytep> row_pointers(new png_bytep[nHeight]);
		for (png_uint_32 i = 0; i < nHeight; i++)
			row_pointers[i] = reinterpret_cast<png_bytep>(pBuffer.m_p + i*tSize.nX);

		HRESULT hRes = E_FAIL;
		bool bEncAni = true;
		int nExtraAttrs = 0;
		if (pAni && nFrames > 1)
		{
			hRes = pAni->Init(a_bstrPrefix, a_pBase);
			if (FAILED(hRes))
				return hRes;

			CAutoVectorPtr<TPixelChannel> pPrev(new TPixelChannel[tSize.nX*tSize.nY]);
			ZeroMemory(pPrev.m_p, tSize.nX*tSize.nY*sizeof*pPrev.m_p);

			for (ULONG i = 0; i < nFrames; ++i)
			{
				png_read_frame_head(png_ptr, info_ptr);

				if (0 == png_get_valid(png_ptr, info_ptr, PNG_INFO_fcTL))
				{
					// the first frame doesn't have an fcTL so it's expected to be hidden -> skip it
					for (ULONG j = 0; j < tSize.nY; ++j)
						row_pointers[j] = reinterpret_cast<png_bytep>(pBuffer.m_p + j*tSize.nX);
					png_read_image(png_ptr, row_pointers);
					continue;
				}

				png_uint_16 nDelayNum = 0;
				png_uint_16 nDelayDen = 0;
				png_uint_32 nFrameSizeX = 0;
				png_uint_32 nFrameSizeY = 0;
				png_uint_32 nFrameOffsetX = 0;
				png_uint_32 nFrameOffsetY = 0;
				png_byte nFrameDisposeOp = 0;
				png_byte nFrameBlendOp = 0;
				if (nFrameSizeX > tSize.nX || nFrameSizeY > tSize.nY || (nFrameOffsetX+nFrameSizeX) > tSize.nX || (nFrameOffsetY+nFrameSizeY) > tSize.nY)
				{
					hRes = E_FAIL;
					break;
				}

				png_get_next_frame_fcTL(png_ptr, info_ptr, &nFrameSizeX, &nFrameSizeY, &nFrameOffsetX, &nFrameOffsetY, &nDelayNum, &nDelayDen, &nFrameDisposeOp, &nFrameBlendOp);

				for (ULONG j = 0; j < nFrameSizeY; ++j)
					row_pointers[j] = reinterpret_cast<png_bytep>(pBuffer.m_p + (j+nFrameOffsetY)*tSize.nX+nFrameOffsetX);
				png_read_image(png_ptr, row_pointers);

				// if frame is smaller than the image, copy the borders
				if (nFrameOffsetY > 0)
					CopyMemory(pBuffer.m_p, pPrev.m_p, nFrameOffsetY*tSize.nX*sizeof*pBuffer.m_p);
				if (nFrameOffsetX > 0)
					for (ULONG y = nFrameOffsetY; y < nFrameSizeY+nFrameOffsetY; ++y)
						CopyMemory(pBuffer.m_p+y*tSize.nX, pPrev.m_p+y*tSize.nX, nFrameOffsetX*sizeof*pBuffer.m_p);
				if (nFrameSizeX+nFrameOffsetX < tSize.nX)
					for (ULONG y = nFrameOffsetY; y < nFrameSizeY+nFrameOffsetY; ++y)
						CopyMemory(pBuffer.m_p+y*tSize.nX+nFrameSizeX+nFrameOffsetX, pPrev.m_p+y*tSize.nX+nFrameSizeX+nFrameOffsetX, (tSize.nX-nFrameSizeX-nFrameOffsetX)*sizeof*pBuffer.m_p);
				if (nFrameSizeY+nFrameOffsetY < tSize.nY)
					CopyMemory(pBuffer.m_p+(nFrameSizeY+nFrameOffsetY)*tSize.nX, pPrev.m_p+(nFrameSizeY+nFrameOffsetY)*tSize.nX, (tSize.nY-nFrameSizeY-nFrameOffsetY)*tSize.nX*sizeof*pBuffer.m_p);

				// blend with previous content
				if (nFrameBlendOp == PNG_BLEND_OP_OVER && i > 0)
				{
					TPixelChannel const* pS = pPrev.m_p+tSize.nX*nFrameOffsetY+nFrameOffsetX;
					TPixelChannel* pD = pBuffer.m_p+tSize.nX*nFrameOffsetY+nFrameOffsetX;
					for (ULONG y = 0; y < nFrameSizeY; ++y, pS+=tSize.nX-nFrameSizeX, pD+=tSize.nX-nFrameSizeX)
					{
						for (ULONG x = 0; x < nFrameSizeX; ++x, ++pS, ++pD)
						{
							if (pS->bA == 0 || pD->bA == 255)
								continue;
							// blend pixels
							ULONG nNewA = pD->bA*255 + (255-pD->bA)*pS->bA;
							if (nNewA)
							{
								ULONG const bA1 = (255-pD->bA)*pS->bA;
								ULONG const bA2 = pD->bA*255;
								pD->bR = (pS->bR*bA1 + pD->bR*bA2)/nNewA;
								pD->bG = (pS->bG*bA1 + pD->bG*bA2)/nNewA;
								pD->bB = (pS->bB*bA1 + pD->bB*bA2)/nNewA;
							}
							else
							{
								pD->bR = 0;
								pD->bG = 0;
								pD->bB = 0;
							}
							pD->bA = nNewA/255;
						}
					}
				}

				hRes = pAni->AppendFrame(&CAnimationFrameCreatorRasterImage(tSize.nX, tSize.nY, reinterpret_cast<TPixelChannel const*>(pBuffer.m_p)), nDelayNum ? float(nDelayNum)/(nDelayDen ? nDelayDen : 100) : 0.1f, a_bstrPrefix, a_pBase);
				if (FAILED(hRes))
					break;
				switch (nFrameDisposeOp)
				{
				case PNG_DISPOSE_OP_BACKGROUND:
					if (tSize.nX > nFrameSizeX)
					{
						for (ULONG y = 0; y < nFrameSizeY; ++y)
							ZeroMemory(pPrev.m_p+tSize.nX*(nFrameOffsetY+y)+nFrameOffsetX, nFrameSizeX*sizeof*pPrev.m_p);
					}
					else
					{
						ZeroMemory(pPrev.m_p+tSize.nX*nFrameOffsetY, tSize.nX*nFrameSizeY*sizeof*pPrev.m_p);
					}
					break;
				case PNG_DISPOSE_OP_PREVIOUS:
					break;
				case PNG_DISPOSE_OP_NONE:
					std::swap(pBuffer.m_p, pPrev.m_p);
					break;
				}
			}
			pAni->SetLoopCount(a_bstrPrefix, a_pBase, nPlays);
		}
		else
		{
			png_read_image(png_ptr, row_pointers);
			png_read_end(png_ptr, info_ptr);
			float fGamma = 0.0f;
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA))
			{
				double d = 1.0;
				png_get_gAMA(png_ptr, info_ptr, &d);
				fGamma = int(100.0/d+0.5)/100.0f;
				++nExtraAttrs;
			}

			TImageResolution tRes;
			TImageResolution* pRes = NULL;
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_pHYs))
			{
				png_uint_32 resX = 0;
				png_uint_32 resY = 0;
				int unit = 0;
				png_get_pHYs(png_ptr, info_ptr, &resX, &resY, &unit);
				if (unit == 1)
				{
					tRes.nNumeratorX = (resX*254+5000)/10000;
					tRes.nDenominatorX = 254;
					tRes.nNumeratorY = (resY*254+5000)/10000;
					tRes.nDenominatorY = 254;
					pRes = &tRes;
					++nExtraAttrs;
				}
			}
			CPixelChannel tDef;
			if (png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD))
			{
				png_color_16p pBG = NULL;
				png_get_bKGD(png_ptr, info_ptr, &pBG);
				switch (nColorType)
				{
				case PNG_COLOR_TYPE_PALETTE:
					break;
				case PNG_COLOR_TYPE_GRAY:
				case PNG_COLOR_TYPE_GRAY_ALPHA:
					tDef.bA = 255;
					tDef.bR = tDef.bG = tDef.bB = To8Bits(pBG->gray, nBitDepth);
					++nExtraAttrs;
					break;
				case PNG_COLOR_TYPE_RGB:
				case PNG_COLOR_TYPE_RGB_ALPHA:
					tDef.bA = 255;
					tDef.bR = To8Bits(pBG->red, nBitDepth);
					tDef.bG = To8Bits(pBG->green, nBitDepth);
					tDef.bB = To8Bits(pBG->blue, nBitDepth);
					++nExtraAttrs;
					break;
				}
			}

			if (pRIF)
			{
				hRes = pRIF->Create(a_bstrPrefix, a_pBase, &tSize, pRes, 1, CChannelDefault(EICIRGBA, tDef), fGamma, CImageTile(tSize.nX, tSize.nY, pBuffer));
				bEncAni = false;
			}
			else
			{
				hRes = pAni->Init(a_bstrPrefix, a_pBase);
				if (SUCCEEDED(hRes))
					hRes = pAni->AppendFrame(&CAnimationFrameCreatorRasterImage(&tSize, pRes, 1, CChannelDefault(EICIRGBA, tDef), fGamma, CImageTile(tSize.nX, tSize.nY, pBuffer)), 0.1f, a_bstrPrefix, a_pBase);
			}
		}
		if (FAILED(hRes))
			return hRes;

		if (a_pEncoderID) *a_pEncoderID = bEncAni ? __uuidof(DocumentEncoderAPNG) : __uuidof(DocumentEncoderPNG);
		if (a_ppEncoderCfg)
		{
			*a_ppEncoderCfg = NULL;
			if (!bEncAni)
			{
				CComPtr<IConfig> pConfig;
				pConfig.Attach(CDocumentEncoderPNG::Config());
				if (nInterlaceType == PNG_INTERLACE_ADAM7)
				{
					CComBSTR bstr(L"Interlacing");
					pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(true));
				}
				if (nExtraAttrs == 0)
				{
					CComBSTR bstr(L"ExtraAttrs");
					pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(false));
				}
				*a_ppEncoderCfg = pConfig.Detach();
			}
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

