// DocumentEncoderWebPAnimation.cpp : Implementation of CDocumentCodecWebP

#include "stdafx.h"
#include "DocumentEncoderWebPAnimation.h"

#include "WebPSaveConfig.h"
#include <RWDocumentAnimation.h>


// CDocumentCodecWebP

STDMETHODIMP CDocumentEncoderWebPAnimation::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorWebPAni::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentEncoderWebPAnimation::DefaultConfig(IConfig** a_ppDefCfg)
{
	return InitWebPConfig(a_ppDefCfg);
}

STDMETHODIMP CDocumentEncoderWebPAnimation::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg)
			return S_FALSE; // the standard WebP encoder should take care of this
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_ANIMATION ENCFEAT_IMAGE_ALPHA);
		CComPtr<IDocumentAnimation> pDocAni;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDocAni));
		return pDocAni ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "webp/encode.h"
#include "webp/mux.h"
#include "WebPUtils.h"

struct SWebPFrame
{
	enum { ECompareMask = 0xfffefefe };
	SWebPFrame() : nSizeX(0), nSizeY(0), fDelay(0.1f), bAlpha(true) {}
	HRESULT Init(IDocumentAnimation* a_pAni, IUnknown* a_pFrame, ULONG a_nAniSizeX, ULONG a_nAniSizeY)
	{
		CComPtr<IDocument> pFrameDoc;
		if (FAILED(a_pAni->FrameGetDoc(a_pFrame, &pFrameDoc)) || pFrameDoc == NULL)
			return E_FAIL;
		CComPtr<IDocumentImage> pFrameImg;
		pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
		if (pFrameImg == NULL)
			return E_FAIL;
		TImageSize tSize = {1, 1};
		pFrameImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		nSizeX = a_nAniSizeX;
		nSizeY = a_nAniSizeY;
		nX0 = (a_nAniSizeX-tSize.nX)>>1;
		nY0 = (a_nAniSizeY-tSize.nY)>>1;
		nX1 = nX0+tSize.nX;
		nY1 = nY0+tSize.nY;
		bDispose = WEBP_MUX_DISPOSE_BACKGROUND;
		pData.Attach(new BYTE[nSizeX*nSizeY<<2]);
		if (nSizeX > tSize.nX || nSizeY > tSize.nY)
			ZeroMemory(pData.m_p, nSizeX*nSizeY<<2);
		TImageStride const tStride = {1, nSizeX};
		if (FAILED(pFrameImg->TileGet(EICIRGBA, NULL, &tSize, &tStride, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pData.m_p+((nSizeX*nY0+nX0)<<2)), NULL, EIRIAccurate)))
			return E_FAIL;
		Trim();
		nX0 &= ~1;
		nY0 &= ~1;
		if (nX0 > 0 || nY0 > 0 || nX1 < nSizeX || nY1 < nSizeY)
		{
			bAlpha = true;
		}
		else
		{
			bAlpha = false;
			for (ULONG y = nY0; !bAlpha && y < nY1; ++y)
			{
				BYTE const* p = pData.m_p+((nSizeX*y+nX0)<<2) + 3;
				for (ULONG x = nX0; x < nX1; ++x, p+=4)
					if (*p != 255)
					{
						bAlpha = true;
						break;
					}
			}
		}
		a_pAni->FrameGetTime(a_pFrame, &fDelay);
		return S_OK;
	}
	bool IsOpaque() const { return !bAlpha; }
	static bool RowEmpty(BYTE const* p, ULONG const n)
	{
		for (BYTE const* const pEnd = p+(n<<2); p < pEnd; p+=4)
			if (p[3])
				return false;
		return true;
	}
	static bool ColEmpty(BYTE const* p, ULONG const n, ULONG const line)
	{
		for (BYTE const* const pEnd = p+n*line; p < pEnd; p+=line)
			if (p[3])
				return false;
		return true;
	}
	bool Trim()
	{
		// check if there is an empty border and if yes, reduce the frame dimensions
		bool bChange = false;
		ULONG n;
		BYTE const* p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nY0; n < nY1; ++n, p+=nSizeX<<2)
			if (!RowEmpty(p, nX1-nX0))
				break;
		if (n == nY1)
		{
			// image is completely transparent, reduce it to single pixel
			nX0 = nY0 = 0;
			nX1 = nY1 = 1;
			return true;
		}
		if (n > nY0)
		{
			bChange = true;
			nY0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY1+(nX0<<2);
		for (n = nY1; n > nY0; --n, p-=nSizeX<<2)
			if (!RowEmpty(p-(nSizeX<<2), nX1-nX0))
				break;
		if (n < nY1)
		{
			bChange = true;
			nY1 = n;
		}

		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nX0; n < nX1; ++n, p+=4)
			if (!ColEmpty(p, nY1-nY0, nSizeX<<2))
				break;
		if (n > nX0)
		{
			bChange = true;
			nX0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY0+(nX1<<2);
		for (n = nX1; n > nX0; --n, p-=4)
			if (!ColEmpty(p-4, nY1-nY0, nSizeX<<2))
				break;
		if (n < nX1)
		{
			bChange = true;
			nX1 = n;
		}
		if (!bChange)
			return true;//bAlpha;

		bool bNewAlpha = false;
		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (ULONG nY = nY0; nY < nY1; ++nY)
		{
			for (ULONG nX = nX0; nX < nX1; ++nX, p+=4)
			{
				if (p[3] == 0)
					bNewAlpha = true;
			}
			p += (nSizeX-nX1+nX0)<<2;
		}
		return bNewAlpha;
	}

	template<typename TIterator>
	static void OptimizeRectangles(TIterator a_tBegin, TIterator a_tEnd) // optimize logical screen sizes and disposal methods
	{
		if (a_tBegin == a_tEnd)
			return;
		TIterator p = a_tBegin;
		TIterator c = a_tBegin;
		for (++c; c != a_tEnd; ++c, ++p)
		{
			DWORD const* pP = reinterpret_cast<DWORD const*>(p->pData.m_p);
			DWORD const* pC = reinterpret_cast<DWORD const*>(c->pData.m_p);
			LONG nX0 = min(p->nX0, c->nX0);
			LONG nX1 = max(p->nX1, c->nX1);
			LONG nY0 = min(p->nY0, c->nY0);
			LONG nY1 = max(p->nY1, c->nY1);
			LONG nChgX0 = nX1+1;
			LONG nChgX1 = nX0-1;
			LONG nChgY0 = nY1+1;
			LONG nChgY1 = nY0-1;
			ULONG nChg = 0;
			ULONG n = nY0*c->nSizeX+nX0;
			bool bOpaque = true;
			for (LONG nY = nY0; nY < nY1; ++nY)
			{
				for (LONG nX = nX0; nX < nX1; ++nX, ++n)
				{
					if ((pP[n]^pC[n])&ECompareMask)
					{
						if (nChgX0 > nX) nChgX0 = nX;
						if (nChgY0 > nY) nChgY0 = nY;
						if (nChgX1 <= nX) nChgX1 = nX+1;
						if (nChgY1 <= nY) nChgY1 = nY+1;
						++nChg;
						if (bOpaque && (pC[n]&0xff000000) != 0xff)
							bOpaque = false;
					}
				}
				n += c->nSizeX - (nX1-nX0);
			}
			ULONG nCostNoneOver = (nChgY1-nChgY0)*(nChgX1-nChgX0)-1;
			ULONG nCostNoneBlend = bOpaque ? nChg : 0xffffffff;
			if (nChgX0 >= nChgX1)
			{
				// frames are the same - reduce to 1 pixel and keep the previous one
				c->nX0 = c->nY0 = 0;
				c->nX1 = c->nY1 = 1;
				p->bDispose = WEBP_MUX_DISPOSE_NONE;
			}
			else
			{
				if (min(nCostNoneOver, nCostNoneBlend)+1 < (c->nX1-c->nX0)*(c->nY1-c->nY0)) // for transparent images, the default just may work better
				{
					p->bDispose = WEBP_MUX_DISPOSE_NONE;
					//c->bBlend = nCostNoneOver < nCostNoneBlend ? PNG_BLEND_OP_SOURCE : PNG_BLEND_OP_OVER;
					c->nX0 = nChgX0;
					c->nX1 = nChgX1;
					c->nY0 = nChgY0;
					c->nY1 = nChgY1;
				}
			}
		}
	}
	template<typename TIterator>
	static void OptimizePixels(TIterator a_tBegin, TIterator a_tEnd) // delete duplicate pixels
	{
		if (a_tBegin == a_tEnd)
			return;
		--a_tEnd;
		TIterator p = a_tEnd;
		TIterator c = a_tEnd;
		for (--p; c != a_tBegin; --c, --p)
		{
			//if (p->bBlend == PNG_BLEND_OP_SOURCE)
			//	continue;
			if (p->bDispose != WEBP_MUX_DISPOSE_NONE)
				continue;
			DWORD const* pP = reinterpret_cast<DWORD const*>(p->pData.m_p);
			DWORD* pC = reinterpret_cast<DWORD*>(c->pData.m_p);
			ULONG n = c->nY0*c->nSizeX+c->nX0;
			for (ULONG nY = c->nY0; nY < c->nY1; ++nY)
			{
				for (ULONG nX = c->nX0; nX < c->nX1; ++nX, ++n)
				{
					if (((pP[n]^pC[n])&ECompareMask) == 0)
					{
						pC[n] = 0;
						c->bAlpha = true;
					}
				}
				n += c->nSizeX - (c->nX1-c->nX0);
			}
		}
	}

	ULONG nSizeX;
	ULONG nSizeY;
	CAutoVectorPtr<BYTE> pData;
	bool bAlpha;
	float fDelay;
	ULONG nX0;
	ULONG nY0;
	ULONG nX1;
	ULONG nY1;
	WebPMuxAnimDispose bDispose;
};

STDMETHODIMP CDocumentEncoderWebPAnimation::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CConfigValue cQuality;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPQUALITY), &cQuality);
		CConfigValue cLossless;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPLOSSLESS), &cLossless);
		CConfigValue cMethod;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPOPTIMIZATION), &cMethod);

		CComPtr<IDocumentAnimation> pA;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
			return E_NOINTERFACE;

		CComPtr<IEnumUnknowns> pFrames;
		pA->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		if (pFrames) pFrames->Size(&nFrames);
		if (nFrames == 0)
			return E_FAIL; // no frames...
		ULONG nLoopCount = 0;
		pA->LoopCountGet(&nLoopCount);
		CAutoVectorPtr<SWebPFrame> aWebPFrames(new SWebPFrame[nFrames]);
		// get animation size (bounding rectangle)
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			if (pFrame) pA->FrameGetDoc(pFrame, &pFrameDoc);
			CComPtr<IDocumentImage> pFrameImg;
			if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
			TImageSize tSize = {0, 0};
			if (pFrameImg) pFrameImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			if (tSize.nX > nSizeX) nSizeX = tSize.nX;
			if (tSize.nY > nSizeY) nSizeY = tSize.nY;
		}
		if (nSizeX*nSizeY == 0)
			return E_FAIL;
		bool bSemitransparent = false;
		// read individual frames
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			SWebPFrame& sF = aWebPFrames.m_p[i];
			if (FAILED(sF.Init(pA, pFrame, nSizeX, nSizeY)))
				return E_FAIL;
			if (!bSemitransparent && sF.bAlpha)
				bSemitransparent = true;
		}

		if (!bSemitransparent)
		{
			// find rectangles that change
			SWebPFrame::OptimizeRectangles(aWebPFrames.m_p, aWebPFrames.m_p+nFrames);
			// webp limitation
			for (SWebPFrame* p = aWebPFrames.m_p; p != aWebPFrames.m_p+nFrames; ++p)
			{
				p->nX0 &= ~1;
				p->nY0 &= ~1;
			}
			// delete same pixels
			SWebPFrame::OptimizePixels(aWebPFrames.m_p, aWebPFrames.m_p+nFrames);
		}

		CWebPMuxPtr pMux(WebPMuxNew());

		WebPConfig tCfg;
		ZeroMemory(&tCfg, sizeof tCfg);
		WebPConfigPreset(&tCfg, WEBP_PRESET_DEFAULT, cQuality);
		if (cLossless.operator bool())
			tCfg.lossless = 1;
		tCfg.method = cMethod.operator LONG();

		WebPMuxAnimParams tParams = {0, nLoopCount};
		WebPMuxSetAnimationParams(pMux, &tParams);
		WebPMuxSetCanvasSize(pMux, nSizeX, nSizeY);

		for (ULONG i = 0; i < nFrames; ++i)
		{
			SWebPFrame& sF = aWebPFrames.m_p[i];

			CWebPMemoryWriter cMem;

			CWebPPicture tPic;
			tPic.colorspace = sF.bAlpha ? WEBP_YUV420A : WEBP_YUV420;
			tPic.width = sF.nX1-sF.nX0;
			tPic.height = sF.nY1-sF.nY0;
			tPic.writer = WebPMemoryWrite;
			tPic.custom_ptr = &cMem;
			tPic.use_argb = tCfg.lossless;
			int nOK = sF.bAlpha ?
				WebPPictureImportBGRA(&tPic, sF.pData.m_p+((sF.nSizeX*sF.nY0+sF.nX0)<<2), sF.nSizeX<<2) :
				WebPPictureImportBGRX(&tPic, sF.pData.m_p+((sF.nSizeX*sF.nY0+sF.nX0)<<2), sF.nSizeX<<2);

			if (0 == WebPEncode(&tCfg, &tPic))
				return E_FAIL;

			WebPMuxFrameInfo tFrame;
			tFrame.id = WEBP_CHUNK_ANMF;
			tFrame.dispose_method = sF.bDispose;
			tFrame.bitstream.bytes = cMem.mem;
			tFrame.bitstream.size = cMem.size;
			tFrame.duration = 1000.0f*sF.fDelay+0.5f;
			tFrame.x_offset = sF.nX0;
			tFrame.y_offset = sF.nY0;

			if (WEBP_MUX_OK != WebPMuxPushFrame(pMux, &tFrame, 1))
				return E_FAIL;
		}

		CComPtr<IImageMetaData> pIMD;
		a_pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD)
		{
			ULONG nLen = 0;
			pIMD->GetBlockSize(CComBSTR(L"EXIF"), &nLen);
			if (nLen)
			{
				CAutoVectorPtr<BYTE> cData(new BYTE[nLen]);
				pIMD->GetBlock(CComBSTR(L"EXIF"), nLen, cData);
				WebPData tEXIF;
				tEXIF.bytes = cData;
				tEXIF.size = nLen;
				if (WEBP_MUX_OK != WebPMuxSetChunk(pMux, "EXIF", &tEXIF, 1))
					return E_FAIL;
			}
			nLen = 0;
			pIMD->GetBlockSize(CComBSTR(L"XMP"), &nLen);
			if (nLen)
			{
				CAutoVectorPtr<BYTE> cData(new BYTE[nLen]);
				pIMD->GetBlock(CComBSTR(L"XMP"), nLen, cData);
				WebPData tXMP;
				tXMP.bytes = cData;
				tXMP.size = nLen;
				if (WEBP_MUX_OK != WebPMuxSetChunk(pMux, "XMP ", &tXMP, 1))
					return E_FAIL;
			}
		}

		CWebPData cData;
		if (WEBP_MUX_OK != WebPMuxAssemble(pMux, &cData))
			return E_FAIL;
		return a_pDst->Write(cData.size, cData.bytes);
	}
	catch (...)
	{
		hRes = a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}

	return hRes;
}

#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentEncoderWebPAnimation::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	*a_ppName = NULL;
	if (a_pConfig == NULL)
		return E_FAIL;

	CConfigValue lossless;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WEBPLOSSLESS), &lossless);
	if (lossless)
	{
		*a_ppName = new CMultiLanguageString(L"[0409]Lossless[0405]Bezeztrátově");
		return S_OK;
	}

	CConfigValue quality;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WEBPQUALITY), &quality);
	wchar_t sz[32];
	_swprintf(sz, L"%i%%", quality.operator LONG());
	*a_ppName = new CSimpleLocalizedString(SysAllocString(sz));
	return S_OK;
}

