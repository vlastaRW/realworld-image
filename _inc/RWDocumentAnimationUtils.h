#pragma once

#ifdef __RWDocumentImageRaster_h__

class CAnimationFrameCreatorRasterImage :
	public IAnimationFrameCreator
{
public:
	CAnimationFrameCreatorRasterImage(TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile) :
		m_pSize(a_pSize), m_pResolution(a_pResolution), m_nChannels(a_nChannels),
		m_aChannelDefaults(a_aChannelDefaults), m_fGamma(a_fGamma), m_pTile(a_pTile)
	{
	}
		CAnimationFrameCreatorRasterImage(ULONG a_nSizeX, ULONG a_nSizeY, TPixelChannel const* a_pData) :
		m_pSize(&m_tTile.tSize), m_pResolution(NULL), m_nChannels(1),
		m_aChannelDefaults(NULL), m_fGamma(0.0f), m_pTile(&m_tTile)
	{
		static CChannelDefault cChD(EICIRGBA);
		m_aChannelDefaults = cChD;
		m_tTile.nChannelIDs = EICIRGBA;
		m_tTile.tSize.nX = a_nSizeX;
		m_tTile.tSize.nY = a_nSizeY;
		m_tTile.tOrigin.nX = m_tTile.tOrigin.nY = 0;
		m_tTile.tStride.nX = 1;
		m_tTile.tStride.nY = a_nSizeX;
		m_tTile.pData = a_pData;
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
		CComQIPtr<IDocumentFactoryRasterImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		return pFct->Create(a_bstrID, a_pBase, m_pSize, m_pResolution, m_nChannels, m_aChannelDefaults, m_fGamma, m_pTile);
	}

private:
	TImageSize const* m_pSize;
	TImageResolution const* m_pResolution;
	ULONG m_nChannels;
	TChannelDefault const* m_aChannelDefaults;
	float m_fGamma;
	TImageTile const* m_pTile;
	TImageTile m_tTile;
};

#endif //__RWDocumentImageRaster_h__


//class CImageLayerCreatorStorage :
//	public IImageLayerCreator
//{
//public:
//	CImageLayerCreatorStorage(IStorageFilter* a_pFile) : m_pFile(a_pFile)
//	{
//	}
//
//	// IUnknown methods
//public:
//	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
//	{
//		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
//		{
//			*a_ppvObject = this;
//			return S_OK;
//		}
//		return E_NOINTERFACE;
//	}
//	STDMETHOD_(ULONG, AddRef)() { return 2; }
//	STDMETHOD_(ULONG, Release)() { return 1; }
//
//	// IImageLayerCreator methods
//public:
//	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
//	{
//		CComPtr<IInputManager> pIM;
//		RWCoCreateInstance(pIM, __uuidof(InputManager));
//		CComPtr<IEnumUnknowns> pBuilders;
//		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
//		return pIM->DocumentCreateData(pBuilders, m_pFile, a_bstrID, a_pBase);
//	}
//
//private:
//	CComPtr<IStorageFilter> m_pFile;
//};
//
//
//
class CAnimationFrameCreatorDocument :
	public IAnimationFrameCreator
{
public:
	CAnimationFrameCreatorDocument(IDocument* a_pDoc, bool a_bViaCopy = false) : m_pDoc(a_pDoc), m_bCopy(a_bViaCopy)
	{
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
		if (m_bCopy)
		{
			CComPtr<IDocument> pTmpDoc;
			RWCoCreateInstance(pTmpDoc, __uuidof(DocumentBase));
			if (FAILED(m_pDoc->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pTmpDoc), NULL, NULL)))
				return E_FAIL;
			return pTmpDoc->DocumentCopy(a_bstrID, a_pBase, NULL, NULL);
		}
		else
		{
			return m_pDoc->DocumentCopy(a_bstrID, a_pBase, NULL, NULL);
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	bool m_bCopy;
};


