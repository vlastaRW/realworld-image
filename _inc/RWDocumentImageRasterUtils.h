#pragma once

#ifdef __cplusplus

class CImageLayerCreatorStorage :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorStorage(IStorageFilter* a_pFile) : m_pFile(a_pFile)
	{
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		return pIM->DocumentCreateData(pBuilders, m_pFile, a_bstrID, a_pBase);
	}

private:
	CComPtr<IStorageFilter> m_pFile;
};


class CImageLayerCreatorDocument :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorDocument(IDocument* a_pDoc) : m_pDoc(a_pDoc)
	{
	}
	CImageLayerCreatorDocument(IDocumentLayeredImage* a_pDoc, IComparable* a_pLayer) : m_pDoc(NULL)
	{
		CComPtr<ISubDocumentID> pSDI;
		a_pDoc->ItemFeatureGet(a_pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDI));
		pSDI->SubDocumentGet(&m_pDoc);
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		return m_pDoc->DocumentCopy(a_bstrID, a_pBase, NULL, NULL);
	}

private:
	CComPtr<IDocument> m_pDoc;
};

class CImageLayerCreatorRasterImage :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorRasterImage(TImageSize const& a_tSize, TPixelChannel const* a_pPixels) :
		m_pResolution(NULL), m_fGamma(0.0f), m_pPixels(a_pPixels)
	{
		m_tCanvasSize = m_tContentSize = a_tSize;
		m_tContentOffset.nX = m_tContentOffset.nY = 0;
		m_tDefault.n = 0;
		m_tStride.nX = 1;
		m_tStride.nY = m_tCanvasSize.nX;
		m_nPixels = m_tContentSize.nY*m_tStride.nY;
	}
	CImageLayerCreatorRasterImage(TImageSize const& a_tCanvasSize, TImageResolution const* a_pResolution, float a_fGamma, TPixelChannel a_tDefault, TPixelChannel const* a_pPixels, TImagePoint const* a_pContentOffset = NULL, TImageSize const* a_pContentSize = NULL, TImageStride const* a_pStride = NULL) :
		m_tCanvasSize(a_tCanvasSize), m_pResolution(a_pResolution), m_pPixels(a_pPixels), m_fGamma(a_fGamma), m_tDefault(a_tDefault)
	{
		if (a_pContentOffset)
			m_tContentOffset = *a_pContentOffset;
		else
			m_tContentOffset.nX = m_tContentOffset.nY = 0;
		if(a_pContentSize)
			m_tContentSize = *a_pContentSize;
		else
			m_tContentSize = m_tCanvasSize;
		if (a_pStride)
			m_tStride = *a_pStride;
		else
			m_tStride = CImageStride(1, m_tContentSize.nX);
		m_nPixels = m_tContentSize.nY*m_tStride.nY;
		if (m_nPixels == 0 && m_pPixels) m_nPixels = 1;
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		CComPtr<IDocumentFactoryRasterImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
		TImageTile t = { EICIRGBA, m_tContentOffset, m_tContentSize, m_tStride, m_nPixels, m_pPixels};
		return pFct->Create(a_bstrID, a_pBase, &m_tCanvasSize, m_pResolution, 1, CChannelDefault(EICIRGBA, m_tDefault), m_fGamma, m_pPixels ? &t : NULL);
	}

private:
	TImageSize m_tCanvasSize;
	TImageResolution const * m_pResolution;
	TImagePoint m_tContentOffset;
	TImageSize m_tContentSize;
	TImageStride m_tStride;
	float m_fGamma;
	TPixelChannel m_tDefault;
	ULONG m_nPixels;
	TPixelChannel const* m_pPixels;
};

class CRasterImageOperationTransform :
	public IPlugInVisitor
{
public:
	CRasterImageOperationTransform(TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform) : m_pCanvas(a_pCanvas), m_pContentTransform(a_pContentTransform) {}
	CRasterImageOperationTransform(TMatrix3x3f const* a_pContentTransform) : m_pCanvas(NULL), m_pContentTransform(a_pContentTransform) {}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IPlugInVisitor)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IPlugInVisitor methods
public:
	STDMETHOD(Run)(IUnknown* /*a_pManager*/, IConfig* a_pConfig, IUnknown* a_pOperation)
	{
		CComQIPtr<IRasterImageFilter> pFilter(a_pOperation);
		if (pFilter == NULL) return E_NOINTERFACE;
		return pFilter->Transform(a_pConfig, m_pCanvas, m_pContentTransform);
	}

private:
	TImageSize const* m_pCanvas;
	TMatrix3x3f const* m_pContentTransform;
};

class CTrivialRasterImageFilter : public IRasterImageFilter
{
public:
	STDMETHOD(Transform)(IConfig*, TImageSize const*, TMatrix3x3f const*) { return S_FALSE; }
	STDMETHOD(AdjustDirtyRect)(IConfig*, TImageSize const*, TRasterImageRect*) { return S_FALSE; }
	STDMETHOD(NeededToCompute)(IConfig*, TImageSize const*, TRasterImageRect*) { return S_FALSE; }
	STDMETHOD(Process)(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix) { return E_NOTIMPL; }
	STDMETHOD(AdjustTransform)(IConfig*, TImageSize const*, TMatrix3x3f*) { return S_FALSE; }
};

struct CAutoPixelBuffer
{
	CAutoPixelBuffer(IDocumentRasterImage* pRI, TImageSize tSize) : m_pRI(pRI), m_tSize(tSize), pDeleter(NULL), pBuffer(NULL)
	{
		m_pRI->BufferAllocate(m_tSize, &pBuffer, &pDeleter);
	}
	~CAutoPixelBuffer()
	{
		if (pDeleter)
			(*pDeleter)(pBuffer);
	}

	HRESULT Replace(TImagePoint a_tAllocOrigin, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel a_tDefault)
	{
		EImageChannelID e = EICIRGBA;
		m_pRI->ChannelsSet(1, &e, &a_tDefault);
		HRESULT hRes = m_pRI->BufferReplace(/*EICIRGBA, */a_tAllocOrigin, m_tSize, a_pContentOrigin, a_pContentSize, a_pContentAlphaSum, /*a_tDefault, */pBuffer, pDeleter);
		if (SUCCEEDED(hRes))
		{
			pDeleter = NULL;
			pBuffer = NULL;
		}
		return hRes;
	}
	TPixelChannel* Buffer() { return pBuffer; }

private:
	IDocumentRasterImage* m_pRI;
	TImageSize m_tSize;
	fnDeleteBuffer pDeleter;
	TPixelChannel* pBuffer;
};

#endif//__cplusplus

