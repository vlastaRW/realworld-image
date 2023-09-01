// DocumentDecoderWebP.cpp : Implementation of CDocumentDecoderWebP

#include "stdafx.h"
#include "DocumentDecoderWebP.h"
#include "DocumentEncoderWebP.h"

#include <lcms.h>
#include "WebPSaveConfig.h"
#include <RWDocumentAnimation.h>


// CDocumentDecoderWebP

STDMETHODIMP CDocumentDecoderWebP::Priority(ULONG* a_pnPriority)
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

STDMETHODIMP CDocumentDecoderWebP::DocumentType(IDocumentType** a_ppDocumentType)
{
	try
	{
		*a_ppDocumentType = NULL;
		*a_ppDocumentType = CDocumentTypeCreatorWebP::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderWebP::IsCompatible(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
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

#include "webp/decode.h"
#include "webp/mux.h"

struct CWebPData : public WebPData
{
	CWebPData()
	{
		WebPDataInit(this);
	}
	~CWebPData()
	{
		WebPDataClear(this);
	}

private:
	CWebPData(CWebPData const&);
};

struct CWebPMuxPtr
{
	WebPMux* p;

	CWebPMuxPtr() : p(NULL) {}
	CWebPMuxPtr(WebPMux* a_p) : p(a_p) {}
	~CWebPMuxPtr() { WebPMuxDelete(p); }
	operator WebPMux*() { return p; }

private:
	CWebPMuxPtr(CWebPMuxPtr const&);
};

struct SAutoDeleteLCMS
{
	cmsHPROFILE hInProfile;
	cmsHPROFILE hOutProfile;
	cmsHTRANSFORM hTransform;

	SAutoDeleteLCMS() : hInProfile(0), hOutProfile(0), hTransform(0) {}
	~SAutoDeleteLCMS()
	{
		if (hTransform) cmsDeleteTransform(hTransform);
		if (hInProfile) cmsCloseProfile(hInProfile);
		if (hOutProfile) cmsCloseProfile(hOutProfile);
	}
};

#include <GammaCorrection.h>

class CWebPFrameCreator :
	public IAnimationFrameCreator
{
public:
	CWebPFrameCreator(TImageSize const& a_tSize, TPixelChannel* a_pBuffer, TPixelChannel a_tBackground) :
		m_tSize(a_tSize), m_pBuffer(a_pBuffer), m_tBackground(a_tBackground), m_bBackgroundDispose(false), m_pGamma(NULL), m_bBlend(false)
	{
		m_tLastOrigin.nX = m_tLastOrigin.nY = 0;
		m_tLastSize.nX = m_tLastSize.nY = 0;
	}
	~CWebPFrameCreator()
	{
		if (m_pGamma) delete m_pGamma;
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IAnimationFrameCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IAnimationFrameCreator methods
public:
	STDMETHOD(Create)(IDocumentBuilder* a_pDefaultBuilder, BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		CComQIPtr<IDocumentFactoryRasterImage> pFct(a_pDefaultBuilder);
		if (pFct == NULL)
			RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));

		return pFct->Create(a_bstrID, a_pBase, &m_tSize, NULL, 1, CChannelDefault(EICIRGBA, m_tBackground), 2.2f, CImageTile(m_tSize.nX, m_tSize.nY, m_pBuffer));
	}

	bool Combine(WebPMuxFrameInfo const& a_tFrame, cmsHTRANSFORM a_hICC)
	{
		SAutoFree cImg;
		{
			int nSizeX = 0;
			int nSizeY = 0;
			cImg.p = WebPDecodeBGRA(a_tFrame.bitstream.bytes, a_tFrame.bitstream.size, &nSizeX, &nSizeY);
			m_tLastSize.nX = nSizeX;
			m_tLastSize.nY = nSizeY;
			m_tLastOrigin.nX = a_tFrame.x_offset;
			m_tLastOrigin.nY = a_tFrame.y_offset;
			m_bBackgroundDispose = a_tFrame.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND;
			m_bBlend = a_tFrame.blend_method == WEBP_MUX_BLEND;
		}
		if (cImg.p == NULL || m_tLastSize.nX == 0 || m_tLastSize.nY == 0 || ULONG(m_tLastSize.nX+m_tLastOrigin.nX) > m_tSize.nX || ULONG(m_tLastSize.nY+m_tLastOrigin.nY) > m_tSize.nY)
			return false;
		TPixelChannel* pD = m_pBuffer+m_tLastOrigin.nY*m_tSize.nX+m_tLastOrigin.nX;
		TPixelChannel* pS = reinterpret_cast<TPixelChannel*>(cImg.p);
		for (ULONG y = 0; y < m_tLastSize.nY; ++y)
		{
			if (a_hICC)
				cmsDoTransform(a_hICC, pS, pS, m_tLastSize.nX);
			if (!m_bBlend)
			{
				CopyMemory(pD, pS, m_tLastSize.nX*sizeof*pD);
				pD += m_tSize.nX;
				pS += m_tLastSize.nX;
			}
			else
			{
				for (ULONG x = 0; x < m_tLastSize.nX; ++x, ++pS, ++pD)
				{
					if (pS->bA == 0)
						continue;
					if (pD->bA == 0 || pS->bA == 255)
					{
						*pD = *pS;
						continue;
					}
					// blend pixels
					if (m_pGamma == NULL)
						m_pGamma = new CGammaTables(2.2f);
					ULONG nNewA = pS->bA*255 + (255-pS->bA)*pD->bA;
					if (nNewA)
					{
						ULONG const bA1 = (255-pS->bA)*pD->bA;
						ULONG const bA2 = pS->bA*255;
						pD->bR = m_pGamma->InvGamma((m_pGamma->m_aGamma[pD->bR]*bA1 + m_pGamma->m_aGamma[pS->bR]*bA2)/nNewA);
						pD->bG = m_pGamma->InvGamma((m_pGamma->m_aGamma[pD->bG]*bA1 + m_pGamma->m_aGamma[pS->bG]*bA2)/nNewA);
						pD->bB = m_pGamma->InvGamma((m_pGamma->m_aGamma[pD->bB]*bA1 + m_pGamma->m_aGamma[pS->bB]*bA2)/nNewA);
					}
					else
					{
						pD->bR = 0;
						pD->bG = 0;
						pD->bB = 0;
					}
					pD->bA = nNewA/255;
				}
				pD += m_tSize.nX-m_tLastSize.nX;
			}
		}
		return true;
	}

	void Dispose()
	{
		if (m_bBackgroundDispose)
		{
			TPixelChannel* pD = m_pBuffer+m_tLastOrigin.nY*m_tSize.nX+m_tLastOrigin.nX;
			for (ULONG y = 0; y < m_tLastSize.nY; ++y)
			{
				for (ULONG x = 0; x < m_tLastSize.nX; ++x, ++pD)
					*pD = m_tBackground;
				pD += m_tSize.nX-m_tLastSize.nX;
			}
		}
	}

private:
	struct SAutoFree
	{
		SAutoFree() : p(NULL) {}
		SAutoFree(BYTE* a_p) : p(a_p) {}
		~SAutoFree() { if (p) free(p); }
		BYTE* p;
	};

private:
	TImageSize const m_tSize;
	TPixelChannel* m_pBuffer;
	TPixelChannel const m_tBackground;
	CGammaTables* m_pGamma;
	// cached values for Dispose()
	TImagePoint m_tLastOrigin;
	TImageSize m_tLastSize;
	bool m_bBackgroundDispose;
	bool m_bBlend;
};

// TODO: remove when API exists
struct WebPChunk {
  uint32_t        tag_;
  int             owner_;  // True if *data_ memory is owned internally.
                           // VP8X, ANIM, and other internally created chunks
                           // like ANMF/FRGM are always owned.
  WebPData        data_;
  WebPChunk*      next_;
};
struct WebPMux {
  void*   images_;
  WebPChunk*      iccp_;
  WebPChunk*      exif_;
  WebPChunk*      xmp_;
  WebPChunk*      anim_;
  WebPChunk*      vp8x_;

  WebPChunk*  unknown_;
};

STDMETHODIMP CDocumentDecoderWebP::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		WebPData tData;
		tData.size = a_nLen;
		tData.bytes = a_pData;

		CWebPMuxPtr pMux(WebPMuxCreate(&tData, 0));

  //ANIMATION_FLAG  = 0x00000002,
  //ALPHA_FLAG      = 0x00000010,
		uint32_t nFlags = 0;
		if (WEBP_MUX_OK != WebPMuxGetFeatures(pMux, &nFlags))
			return E_RW_UNKNOWNINPUTFORMAT;

		CComPtr<IDocumentFactoryAnimation> pAni;
		CComPtr<IDocumentFactoryRasterImage> pRIF;
		for (ULONG i = 0; i < a_nBuilders && pRIF == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pRIF);
		for (ULONG i = 0; i < a_nBuilders && pAni == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pAni);
		if (pAni == NULL && pRIF == NULL)
			return E_FAIL;

		WebPData tICCP = { NULL, 0 };
		if (nFlags&ICCP_FLAG)
			WebPMuxGetChunk(pMux, "ICCP", &tICCP);

		SAutoDeleteLCMS cXform;
		if (tICCP.size && tICCP.bytes)
		{
			try
			{
				cXform.hInProfile = cmsOpenProfileFromMem(const_cast<uint8_t*>(tICCP.bytes), tICCP.size);
				if (cXform.hInProfile)
				{
					cXform.hOutProfile = cmsCreate_sRGBProfile();
					if (cXform.hOutProfile)
						cXform.hTransform = cmsCreateTransform(cXform.hInProfile, TYPE_BGRA_8, cXform.hOutProfile, TYPE_BGRA_8, INTENT_PERCEPTUAL, 0);
				}
			}
			catch (...) {}
		}

		WebPData tXMP = { NULL, 0 };
		if (nFlags&XMP_FLAG)
			WebPMuxGetChunk(pMux, "XMP ", &tICCP);

		WebPData tEXIF = { NULL, 0 };
		if (nFlags&EXIF_FLAG)
			WebPMuxGetChunk(pMux, "EXIF", &tEXIF);

		if (nFlags&ANIMATION_FLAG)
		{
			WebPMuxFrameInfo tMain;
			ZeroMemory(&tMain, sizeof tMain);
			WebPMuxGetFrame(pMux, 1, &tMain);


			// TODO: change this when method for getting canvas size is added
			TImageSize tCanvas = {256, 256};
			WebPData data = pMux.p->vp8x_->data_;
			tCanvas.nX = ULONG(data.bytes[4])+(ULONG(data.bytes[5])<<8)+(ULONG(data.bytes[6])<<16)+1;
			tCanvas.nY = ULONG(data.bytes[7])+(ULONG(data.bytes[8])<<8)+(ULONG(data.bytes[9])<<16)+1;

			WebPMuxAnimParams tAnimPar = {0, 0};
			WebPMuxGetAnimationParams(pMux, &tAnimPar);
			tAnimPar.bgcolor = 0; // it gets ugly otherwise

			CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tCanvas.nX*tCanvas.nY]);
			std::fill_n(pBuffer.m_p, tCanvas.nX*tCanvas.nY, CPixelChannel(ULONG(tAnimPar.bgcolor)));
			CWebPFrameCreator cBuffer(tCanvas, pBuffer, CPixelChannel(ULONG(tAnimPar.bgcolor)));
			cBuffer.Combine(tMain, cXform.hTransform);

			WebPMuxFrameInfo tFrame;
			ZeroMemory(&tFrame, sizeof tFrame);
			bool bCreated = false;
			for (int nFrame = 2; WEBP_MUX_OK == WebPMuxGetFrame(pMux, nFrame, &tFrame); ++nFrame)
			{
				if (!bCreated)
				{
					if (tMain.duration == 0) // first frame with 0 duration, wait for others
					{
						cBuffer.Dispose();
						cBuffer.Combine(tFrame, cXform.hTransform);
						tMain = tFrame;
						continue;
					}
					if (pAni == NULL)
						break;
					// create the animation
					pAni->Init(a_bstrPrefix, a_pBase);
					if (tAnimPar.loop_count) pAni->SetLoopCount(a_bstrPrefix, a_pBase, tAnimPar.loop_count);
					bCreated = true;
					pAni->AppendFrame(&cBuffer, tMain.duration/1000.0f, a_bstrPrefix, a_pBase);
					cBuffer.Dispose();
				}
				cBuffer.Combine(tFrame, cXform.hTransform);
				if (tFrame.duration)
					pAni->AppendFrame(&cBuffer, tFrame.duration/1000.0f, a_bstrPrefix, a_pBase);
				cBuffer.Dispose();
			}
			CComQIPtr<IDocumentFactoryMetaData> pMDF;
			if (bCreated)
			{
				pMDF = pAni;
				if (a_pEncoderID)
					*a_pEncoderID = __uuidof(DocumentEncoderWebPAnimation);
			}
			else
			{
				if (pRIF)
				{
					pRIF->Create(a_bstrPrefix, a_pBase, &tCanvas, NULL, 1, CChannelDefault(EICIRGBA, ULONG(tAnimPar.bgcolor)), 2.2f, CImageTile(tCanvas.nX, tCanvas.nY, pBuffer));
					pMDF = pRIF;
					if (a_pEncoderID)
						*a_pEncoderID = __uuidof(DocumentEncoderWebP);
				}
				else
				{
					pAni->Init(a_bstrPrefix, a_pBase);
					if (tAnimPar.loop_count) pAni->SetLoopCount(a_bstrPrefix, a_pBase, tAnimPar.loop_count);
					pAni->AppendFrame(&cBuffer, tMain.duration/1000.0f, a_bstrPrefix, a_pBase);
					pMDF = pAni;
					if (a_pEncoderID)
						*a_pEncoderID = __uuidof(DocumentEncoderWebP);
				}
			}
			if (pMDF)
			{
				if (tEXIF.bytes && tEXIF.size)
					pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"EXIF"), tEXIF.size, tEXIF.bytes);
				if (tXMP.bytes && tXMP.size)
					pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"XMP"), tXMP.size, tXMP.bytes);
			}
			return S_OK;
		}
		//else if (nFlags&FRAGMENTS_FLAG)
		//{
		//	TImageSize tCanvas = {256, 256};
		//	WebPData data = pMux.p->vp8x_->data_;
		//	tCanvas.nX = ULONG(data.bytes[4])+(ULONG(data.bytes[5])<<8)+(ULONG(data.bytes[6])<<16)+1;
		//	tCanvas.nY = ULONG(data.bytes[7])+(ULONG(data.bytes[8])<<8)+(ULONG(data.bytes[9])<<16)+1;

		//	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tCanvas.nX*tCanvas.nY]);

		//	WebPMuxFrameInfo tFrame;
		//	ZeroMemory(&tFrame, sizeof tFrame);
		//	for (int nFrame = 1; WEBP_MUX_OK == WebPMuxGetFrame(pMux, nFrame, &tFrame); ++nFrame)
		//	{
		//		int nSizeX = 0;
		//		int nSizeY = 0;
		//		TPixelChannel* pImg = reinterpret_cast<TPixelChannel*>(WebPDecodeBGRA(tFrame.bitstream.bytes, tFrame.bitstream.size, &nSizeX, &nSizeY));
		//		for (int y = 0; y < nSizeY; ++y)
		//		{
		//			if (cXform.hTransform)
		//				cmsDoTransform(cXform.hTransform, pImg+y*nSizeX, pImg+y*nSizeX, nSizeX);
		//			CopyMemory(pBuffer.m_p+(y+tFrame.y_offset)*tCanvas.nX+tFrame.x_offset, pImg+y*nSizeX, nSizeX*4);
		//		}
		//		free(pImg);
		//	}
		//	CComQIPtr<IDocumentFactoryMetaData> pMDF;
		//	if (pRIF)
		//	{
		//		pRIF->Create(a_bstrPrefix, a_pBase, &tCanvas, NULL, 1, CChannelDefault(EICIRGBA, ULONG(0)), 2.2f, CImageTile(tCanvas.nX, tCanvas.nY, pBuffer));
		//		pMDF = pRIF;
		//		if (a_pEncoderID)
		//			*a_pEncoderID = __uuidof(DocumentEncoderWebP);
		//	}
		//	else
		//	{
		//		pAni->Init(a_bstrPrefix, a_pBase);
		//		pAni->AppendFrame(&CWebPFrameCreator(tCanvas, pBuffer, CPixelChannel(ULONG(0))), 1.0f, a_bstrPrefix, a_pBase);
		//		pMDF = pAni;
		//		if (a_pEncoderID)
		//			*a_pEncoderID = __uuidof(DocumentEncoderWebP);
		//	}
		//	if (pMDF)
		//	{
		//		if (tEXIF.bytes && tEXIF.size)
		//			pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"EXIF"), tEXIF.size, tEXIF.bytes);
		//		if (tXMP.bytes && tXMP.size)
		//			pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"XMP"), tXMP.size, tXMP.bytes);
		//	}
		//	return S_OK;
		//}

		int nSizeX = 0;
		int nSizeY = 0;
		WebPMuxFrameInfo tMain;
		ZeroMemory(&tMain, sizeof tMain);
		if (WEBP_MUX_OK != WebPMuxGetFrame(pMux, 1, &tMain))
			return E_RW_UNKNOWNINPUTFORMAT;
		BYTE* pImg = WebPDecodeBGRA(tMain.bitstream.bytes, tMain.bitstream.size, &nSizeX, &nSizeY);
		if (pImg == NULL || nSizeX == 0 || nSizeY == 0)
			return E_RW_UNKNOWNINPUTFORMAT;
		if (cXform.hTransform)
			cmsDoTransform(cXform.hTransform, pImg, pImg, nSizeX*nSizeY);
		HRESULT hRes = pRIF->Create(a_bstrPrefix, a_pBase, CImageSize(nSizeX, nSizeY), NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 2.2f, CImageTile(nSizeX, nSizeY, reinterpret_cast<TPixelChannel const*>(pImg)));
		free(pImg);
		CComQIPtr<IDocumentFactoryMetaData> pMDF(pRIF);
		if (pMDF)
		{
			if (tEXIF.bytes && tEXIF.size)
				pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"EXIF"), tEXIF.size, tEXIF.bytes);
			if (tXMP.bytes && tXMP.size)
				pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"XMP"), tXMP.size, tXMP.bytes);
		}
		if (a_pEncoderID)
			*a_pEncoderID = __uuidof(DocumentEncoderWebP);
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

