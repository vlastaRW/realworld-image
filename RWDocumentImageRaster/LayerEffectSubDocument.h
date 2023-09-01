
#pragma once

extern GUID const CLSID_DocumentFactoryImageEffect;

class ATL_NO_VTABLE CLayerEffectSubDocument :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocument,
	//public IDocumentImage,
	public IDocumentImageLayerEffect
{
public:
	CLayerEffectSubDocument() : m_pDoc(NULL), m_pLI(NULL)
	{
	}
	~CLayerEffectSubDocument()
	{
		if (m_pDoc)
			m_pDoc->Release();
		if (m_pLI)
			m_pLI->Release();
	}
	void Init(IDocumentBase* a_pDoc, CDocumentLayeredImage* a_pLI, ULONG a_nLayerID, ULONG a_nEffectID)
	{
		(m_pDoc = a_pDoc)->AddRef();
		(m_pLI = a_pLI)->AddRef();
		m_nLayerID = a_nLayerID;
		m_nEffectID = a_nEffectID;
	}

	static HRESULT WINAPI QISDUndo(void* a_pThis, REFIID UNREF(a_iid), void** a_ppv, DWORD_PTR UNREF(a_dw))
	{
		*a_ppv = NULL;
		CLayerEffectSubDocument* pThis = reinterpret_cast<CLayerEffectSubDocument*>(a_pThis);
		if (pThis->m_pDoc == NULL)
			return E_NOINTERFACE;
		return pThis->m_pDoc->QueryInterface(__uuidof(IDocumentUndo), a_ppv);
	}

	//static HRESULT WINAPI QIDocumentImage(void* a_pThis, REFIID UNREF(a_iid), void** a_ppv, DWORD_PTR UNREF(a_dw))
	//{
	//	*a_ppv = NULL;
	//	CLayerEffectSubDocument* pThis = reinterpret_cast<CLayerEffectSubDocument*>(a_pThis);
	//	if (pThis->m_pDoc == NULL)
	//		return E_NOINTERFACE;
	//	return pThis->m_pDoc->QueryInterface(__uuidof(IDocumentUndo), a_ppv);
	//}

BEGIN_COM_MAP(CLayerEffectSubDocument)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IBlockOperations)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IDocumentUndo), 0, QISDUndo)
	//COM_INTERFACE_ENTRY(IDocumentBase)
	//COM_INTERFACE_ENTRY_FUNC(__uuidof(IDocumentImage), 0, QIDocumentImage)
END_COM_MAP()

	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)()	{ return m_pDoc->WriteLock(); }
	STDMETHOD(WriteUnlock)(){ return m_pDoc->WriteUnlock(); }
	STDMETHOD(ReadLock)()	{ return m_pDoc->ReadLock(); }
	STDMETHOD(ReadUnlock)()	{ return m_pDoc->ReadUnlock(); }

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
	{
		*a_pguidBuilder = CLSID_DocumentFactoryImageEffect;
		return S_OK;
	}
	STDMETHOD(EncoderGet)(CLSID* UNREF(a_pEncoderID), IConfig** UNREF(a_ppConfig))	{ return E_NOTIMPL; }
	STDMETHOD(EncoderSet)(REFCLSID UNREF(a_tEncoderID), IConfig* UNREF(a_pConfig))	{ return E_NOTIMPL; }
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
	{
		if (IsEqualGUID(a_iid, __uuidof(IDocumentImage)))
		{
			//return m_pDoc->DataBlockGet(NULL, a_iid, a_ppFeatureInterface);
			*a_ppFeatureInterface = static_cast<IDocumentImage*>(m_pLI);
			m_pLI->AddRef();
			return S_OK;
		}
		if (IsEqualGUID(a_iid, __uuidof(IDocumentImageLayerEffect)))
		{
			*a_ppFeatureInterface = static_cast<IDocumentImageLayerEffect*>(this);
			AddRef();
			return S_OK;
		}
		if (IsEqualGUID(a_iid, __uuidof(IRasterImageComposedPreview)))
		{
			CComPtr<IDocument> pSubDoc;
			m_pDoc->DataBlockDoc(m_pLI->M_DataID(), &pSubDoc);
			if (pSubDoc)
				return pSubDoc->QueryFeatureInterface(a_iid, a_ppFeatureInterface);
		}
		return E_NOINTERFACE;
	}
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(LocationSet)(IStorageFilter* UNREF(a_pLocation))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsDirty)()
	{
		return E_NOTIMPL;//m_pDoc->IsDirty();
	}
	STDMETHOD(SetDirty)()
	{
		return E_NOTIMPL;//m_pDoc->SetDirty();
	}
	STDMETHOD(ClearDirty)()
	{
		return E_NOTIMPL;//m_pDoc->SetDirty(a_bDirty);
	}
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)	
	{
		//IDocumentData* p = m_pDoc->GetData(m_bstrID);
		return /*p ? p->Aspects(a_pEnumAspects) :*/ E_FAIL;
	}
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
	{
		return E_NOTIMPL;
		//try
		//{
		//	IDocumentData* p = m_pDoc->GetData(m_bstrID);
		//	CopyName(this, a_bstrPrefix, a_pBase);
		//	if (p) return p->DataCopy(a_eHint, a_bstrPrefix, a_pBase);
		//	return S_FALSE;
		//}
		//catch (...)
		//{
		//	return E_UNEXPECTED;
		//}
	}
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
	{
		return E_NOTIMPL;
		//try
		//{
		//	IDocumentData* p = m_pDoc->GetData(m_bstrID);
		//	if (p) return p->QuickInfo(a_nInfoIndex, a_ppInfo);
		//	return E_RW_ITEMNOTFOUND;
		//}
		//catch (...)
		//{
		//	return E_UNEXPECTED;
		//}
	}
	STDMETHOD(ObserverIns)(IDocumentObserver* a_pObserver, TCookie a_tCookie)
	{
		return E_NOTIMPL;//m_pDoc->ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ObserverDel)(IDocumentObserver* a_pObserver, TCookie a_tCookie)
	{
		return E_NOTIMPL;//m_pDoc->ObserverDel(a_pObserver, a_tCookie);
	}

//	// IDocumentImage methods
//public:
//	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity);
//	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults);
//	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
//	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);

	// IDocumentImageLayerEffect methods
public:
	STDMETHOD(Get)(BYTE* a_pEnabled, GUID* a_pOpID, IConfig** a_ppOpCfg)
	{
		bool b = true;
		HRESULT hRes = m_pLI->LayerEffectStepGet(m_nLayerID, m_nEffectID, &b, a_pOpID, a_ppOpCfg);
		if (a_pEnabled)
			*a_pEnabled = b;
		return hRes;
	}

	STDMETHOD(Set)(BYTE* a_pEnabled, GUID* a_pOpID, IConfig* a_pOpCfg)
	{
		bool b = a_pEnabled ? *a_pEnabled : true;
		return m_pLI->LayerEffectStepSet(m_nLayerID, m_nEffectID, a_pEnabled ? &b : NULL, a_pOpID, a_pOpCfg);
	}

	STDMETHOD(ObserverIns)(ILayerEffectObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pLI->EffectObserverIns(a_pObserver, a_tCookie, m_nEffectID);
	}
	STDMETHOD(ObserverDel)(ILayerEffectObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pLI->EffectObserverDel(a_pObserver, a_tCookie, m_nEffectID);
	}

private:
	IDocumentBase* m_pDoc;
	CDocumentLayeredImage* m_pLI;
	ULONG m_nLayerID;
	ULONG m_nEffectID;
};
