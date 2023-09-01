// SelectionDocument.h : Declaration of the CSelectionDocument

#pragma once
#include "RWViewImageRaster.h"
#include <ObserverImpl.h>



class ATL_NO_VTABLE CSelectionDocument : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocument,
	public CObserverImpl<CSelectionDocument, IImageObserver, TImageChange>
{
public:
	CSelectionDocument() : m_fX(0.0f), m_fY(0.0f)
	{
	}
	void Init(IRasterImageEditToolFloatingSelection* a_pTool)
	{
		m_pTool = a_pTool;
	}


BEGIN_COM_MAP(CSelectionDocument)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IBlockOperations)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pRI)
			m_pRI->ObserverDel(ObserverGet(), 0);
	}

	void OwnerNotify(TCookie, TImageChange a_tChange)
	{
		if (m_pTool == NULL || m_pRI == NULL)
			return;
		TImageSize tCanvasSize = {0, 0};
		TImageSize tContentSize = {0, 0};
		TImagePoint tContentOrigin = {0, 0};
		m_pRI->CanvasGet(&tCanvasSize, NULL, &tContentOrigin, &tContentSize, NULL);
		if (tContentSize.nX*tContentSize.nY == 0)
			return;
		CAutoVectorPtr<TRasterImagePixel> cPixels;
		if (!cPixels.Allocate(tContentSize.nX*tContentSize.nY))
			return;
		m_pRI->TileGet(EICIRGBA, &tContentOrigin, &tContentSize, NULL, tContentSize.nX*tContentSize.nY, reinterpret_cast<TPixelChannel*>(cPixels.m_p), NULL, EIRIAccurate);
		m_pTool->Set(m_fX+tContentOrigin.nX, m_fY+tContentOrigin.nY, tContentSize.nX, tContentSize.nY, tContentSize.nX, cPixels, TRUE);
	}

	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)();
	STDMETHOD(WriteUnlock)();
	STDMETHOD(ReadLock)();
	STDMETHOD(ReadUnlock)();

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder) { return E_NOTIMPL; }
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface);
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation);
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation);
	STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig) { return E_NOTIMPL; }
	STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig) { return E_NOTIMPL; }
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsDirty)();
	STDMETHOD(SetDirty)();
	STDMETHOD(ClearDirty)();
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG /*a_nInfoIndex*/, ILocalizedString** /*a_ppInfo*/) { return E_NOTIMPL; }
	STDMETHOD(ObserverIns)(IDocumentObserver* a_pObserver, TCookie a_tCookie);
	STDMETHOD(ObserverDel)(IDocumentObserver* a_pObserver, TCookie a_tCookie);

private:
	void InitInternDoc()
	{
		if (m_pDoc)
			return;
		ObjectLock cLock(this);
		if (m_pDoc)
			return;
		CComPtr<IDocumentFactoryRasterImage> pDF;
		RWCoCreateInstance(pDF, __uuidof(DocumentFactoryRasterImage));
		if (pDF == NULL)
			return;
		m_fX = m_fY = 0.0f;
		m_pTool->Position(&m_fX, &m_fY);
		m_pTool->Size(&m_nSizeX, &m_nSizeY);
		TImageSize tSize = {m_nSizeX, m_nSizeY};
		if (tSize.nX*tSize.nY == 0)
			return;
		CAutoVectorPtr<TPixelChannel> cPixels;
		if (!cPixels.Allocate(tSize.nX*tSize.nY))
			return;
		m_pTool->Data(0, 0, tSize.nX, tSize.nY, tSize.nX, reinterpret_cast<TRasterImagePixel*>(cPixels.m_p));
		RWCoCreateInstance(m_pDoc, __uuidof(DocumentBase));
		if (FAILED(pDF->Create(NULL, CComQIPtr<IDocumentBase>(m_pDoc), &tSize, NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(tSize.nX, tSize.nY, cPixels))))
		{
			m_pDoc = NULL;
			return;
		}
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&m_pRI));
		if (m_pRI)
			m_pRI->ObserverIns(ObserverGet(), 0);
	}

private:
	CComPtr<IRasterImageEditToolFloatingSelection> m_pTool;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentRasterImage> m_pRI;

	float m_fX;
	float m_fY;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
};

