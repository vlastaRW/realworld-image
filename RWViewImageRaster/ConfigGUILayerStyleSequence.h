
#pragma once

#include <XPGUI.h>


class ATL_NO_VTABLE CCustomConfigWndLayerStyleSequence :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CChildWindowImpl<CCustomConfigWndLayerStyleSequence, IChildWindow>,
	public Win32LangEx::CLangIndirectDialogImpl<CCustomConfigWndLayerStyleSequence>,
	public CObserverImpl<CCustomConfigWndLayerStyleSequence, IConfigObserver, IUnknown*>
{
public:
	CCustomConfigWndLayerStyleSequence() : m_nToolbars(0)
	{
	}

	void Create(HWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig)
	{
		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;

		Win32LangEx::CLangIndirectDialogImpl<CCustomConfigWndLayerStyleSequence>::Create(a_hParent);
		if (!IsWindow()) throw E_FAIL;

		MoveWindow(a_prcPositon);
		SetWindowLong(GWL_ID, a_nCtlID);
		ShowWindow(a_bVisible ? SW_SHOW : SW_HIDE);
	}


	BEGIN_COM_MAP(CCustomConfigWndLayerStyleSequence)
		COM_INTERFACE_ENTRY(IChildWindow)
	END_COM_MAP()

	BEGIN_MSG_MAP(CCustomConfigWndLayerStyleSequence)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		//MESSAGE_HANDLER(WM_HELP, OnHelp)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColor2Parent)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnColor2Parent)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnColor2Parent)
	END_MSG_MAP()

	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
		}
		catch (...)
		{
		}
	}

	IConfig* M_Config() const
	{
		return m_pConfig;
	}

	int GetIconIndex(GUID const&)
	{
		return -1;
	}
	HIMAGELIST M_ImageList()
	{
		return NULL;
	}

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// TODO: ?? set control styles (no auto check, drop list combo, want return edit)

		//bool bContxtTips = true;
		//CComPtr<IGlobalConfigManager> pMgr;
		//RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		//if (pMgr)
		//{
		//	CComPtr<IConfig> pCfg;
		//	// hacks: copied CLSID and CFGVAL
		//	static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
		//	pMgr->Config(tID, &pCfg);
		//	CConfigValue cVal;
		//	if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
		//	if (cVal.TypeGet() == ECVTBool) bContxtTips = cVal;
		//}

		if (m_pConfig)
			m_pConfig->ObserverIns(ObserverGet(), 0);

		OwnerNotify(0, NULL);
		//m_bEnableEditUpdates = true;

		a_bHandled = FALSE;
		return 1;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (m_pConfig)
			m_pConfig->ObserverDel(ObserverGet(), 0);

		//if (m_wndToolTip.IsWindow())
		//	m_wndToolTip.DestroyWindow();

		a_bHandled = FALSE;
		return 0;
	}

	//void AddToolTip(HWND window, wchar_t const* desc)
	//{
	//	if (m_wndToolTip.m_hWnd == NULL)
	//	{
	//		m_wndToolTip.Create(static_cast<T const*>(this)->m_hWnd);
	//		HDC hDC = ::GetDC(static_cast<T const*>(this)->m_hWnd);
	//		int nWidth = 420 * GetDeviceCaps(hDC, LOGPIXELSX) / 96;
	//		::ReleaseDC(static_cast<T const*>(this)->m_hWnd, hDC);
	//		m_wndToolTip.SetMaxTipWidth(nWidth);
	//	}
	//	COLE2T strDesc(desc);
	//	TOOLINFO tTI;
	//	ZeroMemory(&tTI, sizeof tTI);
	//	tTI.cbSize = TTTOOLINFO_V1_SIZE;
	//	tTI.hwnd = static_cast<T const*>(this)->m_hWnd;
	//	tTI.uId = reinterpret_cast<UINT_PTR>(window);
	//	tTI.uFlags = TTF_PARSELINKS|TTF_SUBCLASS|TTF_IDISHWND;
	//	tTI.lpszText = strDesc;
	//	m_wndToolTip.AddTool(&tTI);
	//}

private:
	struct SItem
	{
		GUID tID;
		CComPtr<IConfig> pConfig;
		CComBSTR strName;
		CComPtr<IChildWindow> pWindow;
	};
	typedef std::vector<SItem> CItems;

	static void ReadOperations(IConfig* pDup, CItems& cItems)
	{
		CComBSTR bstrEffect(L"Effect");
		CConfigValue cEffect;
		pDup->ItemValueGet(bstrEffect, &cEffect);
		if (IsEqualGUID(cEffect, __uuidof(DocumentOperationNULL)))
		{
		}
		else if (IsEqualGUID(cEffect, __uuidof(DocumentOperationSequence)))
		{
			CComPtr<IConfig> pSeq;
			pDup->SubConfigGet(bstrEffect, &pSeq);
			CConfigValue cSteps;
			CComBSTR bstrSteps(L"SeqSteps");
			pSeq->ItemValueGet(bstrSteps, &cSteps);
			CComPtr<IConfig> pSteps;
			pSeq->SubConfigGet(bstrSteps, &pSteps);
			for (LONG i = 0; i < cSteps.operator LONG(); ++i)
			{
				OLECHAR sz[32];
				swprintf(sz, L"%08x\\SeqOperation", i);
				CComBSTR bstrSubID(sz);
				CConfigValue cStep;
				pSteps->ItemValueGet(bstrSubID, &cStep);
				SItem tOp;
				tOp.tID = cStep.operator const GUID &();
				pSteps->SubConfigGet(bstrSubID, &tOp.pConfig);
				cItems.push_back(tOp);
			}
		}
		else
		{
			SItem tOp;
			tOp.tID = cEffect.operator const GUID &();
			pDup->SubConfigGet(bstrEffect, &tOp.pConfig);
			cItems.push_back(tOp);
		}
	}

private:
	CComPtr<IConfig> m_pConfig;
	CItems m_cItems;
	size_t m_nToolbars;
	//CToolTipCtrl m_wndToolTip;
};



class ATL_NO_VTABLE CConfigCustomGUILayerStyleSequence :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigCustomGUI
{
public:
	BEGIN_COM_MAP(CConfigCustomGUILayerStyleSequence)
		COM_INTERFACE_ENTRY(IConfigCustomGUI)
	END_COM_MAP()

	// IConfigCustomGUI methods
public:
	STDMETHOD(UIDGet)(GUID* a_pguid)
	{
		try
		{
			static const GUID tID = {0x41bb7dbf, 0xe2e6, 0x48d0, {0xb9, 0xc0, 0xb7, 0x51, 0x1c, 0xc2, 0x74, 0x14}};
			*a_pguid = tID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(RequiresMargins)()
	{
		return S_OK;
	}
	STDMETHOD(MinSizeGet)(IConfig* a_pConfig, LCID a_tLocaleID, EConfigPanelMode a_eMode, ULONG* a_nSizeX, ULONG* a_nSizeY)
	{
		try
		{
			*a_nSizeX = 100;
			*a_nSizeY = 10;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(RWHWND a_hParent, RECT const* a_prcPositon, UINT a_nCtlID, LCID a_tLocaleID, BOOL a_bVisible, BOOL a_bParentBorder, IConfig* a_pConfig, EConfigPanelMode a_eMode, IChildWindow** a_ppWindow)
	{
		try
		{
			CComObject<CCustomConfigWndLayerStyleSequence>* pWnd = NULL;
			CComObject<CCustomConfigWndLayerStyleSequence>::CreateInstance(&pWnd);
			CComPtr<IChildWindow> pTmp = pWnd;

			pWnd->Create(a_hParent, a_prcPositon, a_nCtlID, a_tLocaleID, a_bVisible, a_bParentBorder, a_pConfig);

			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppWindow == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
};

// CDesignerViewLayerEffect

class ATL_NO_VTABLE CDesignerViewLayerEffect : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView,
	public CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>,
	public CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>
{
public:
	CDesignerViewLayerEffect() : m_bEnableUpdates(false)
	{
	}

	void Init(ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGrp, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc)
	{
		m_pDoc = a_pDoc;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&m_pImage));
		CComBSTR bstrID;
		if (m_pImage)
		{
			m_pImage->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
			m_pImage->StatePrefix(&bstrID);
		}
		else
		{
			throw E_FAIL;
		}
		if (bstrID.Length())
		{
			m_strSelGrp = bstrID;
			m_strSelGrp.append(a_pszSyncGrp);
		}
		else
		{
			m_strSelGrp = a_pszSyncGrp;
		}
		if (!m_strSelGrp.empty())
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pSharedState = a_pFrame;
		}


		CComPtr<ISharedState> pState;
		CComBSTR bstr(m_strSelGrp.c_str());
		m_pSharedState->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		if (pState)
			m_pImage->StateUnpack(pState, &pItems);
		else
			m_pImage->ItemsEnum(NULL, &pItems);

		CComPtr<IComparable> pLayer;
		if (pItems)
			pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pLayer));

		if ((m_pLayer == NULL && pLayer != NULL) ||
			(pLayer == NULL && m_pLayer != NULL) ||
			(pLayer != NULL && m_pLayer != NULL && m_pLayer->Compare(pLayer) != S_OK))
		{
			m_pLayer = pLayer;
		}

		RWCoCreateInstance(m_pWnd, __uuidof(AutoConfigWnd));
		Data2GUI();
		m_pWnd->Create(a_hParent, a_rcWindow, 123, a_tLocaleID, TRUE, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? ECWBMMarginAndOutline : ECWBMMargin);
		m_bEnableUpdates = true;
	}

BEGIN_COM_MAP(CDesignerViewLayerEffect)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		if (m_pEffect)
			m_pEffect->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		try
		{
			if (m_pImage == NULL || !m_bEnableUpdates || m_strSelGrp.compare(a_tParams.bstrName) != 0)
				return;

			CComPtr<IComparable> pLayer;
			CComPtr<IEnumUnknowns> pItems;
			m_pImage->StateUnpack(a_tParams.pState, &pItems);

			if (pItems)
				pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pLayer));

			if ((m_pLayer == NULL && pLayer != NULL) ||
				(pLayer == NULL && m_pLayer != NULL) ||
				(pLayer != NULL && m_pLayer != NULL && m_pLayer->Compare(pLayer) != S_OK))
			{
				m_pLayer = pLayer;
				m_bEnableUpdates = false;
				Data2GUI();
				m_bEnableUpdates = true;
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, TStructuredChanges a_tChanges)
	{
		try
		{
			if (!m_bEnableUpdates)
				return;

			if (m_pLayer)
			{
				for (ULONG i = 0; i < a_tChanges.nChanges; ++i)
				{
					if (a_tChanges.aChanges[i].nChangeFlags&ESCContent &&
						m_pLayer->Compare(a_tChanges.aChanges[i].pItem) == S_OK)
					{
						m_bEnableUpdates = false;
						Data2GUI();
						m_bEnableUpdates = true;
					}
				}
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (!m_bEnableUpdates)
				return;

			if (m_pLayer)
			{
				m_bEnableUpdates = false;
				m_pImage->LayerEffectSet(m_pLayer, m_pEffect);
				m_bEnableUpdates = true;
			}
		}
		catch (...)
		{
		}
	}

	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND* a_pHandle)
	{
		return m_pWnd ? m_pWnd->Handle(a_pHandle) : E_UNEXPECTED;
	}
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{
		return m_pWnd ? m_pWnd->SendMessage(a_uMsg, a_wParam, a_lParam) : E_UNEXPECTED;
	}
	STDMETHOD(Show)(BOOL a_bShow)
	{
		return m_pWnd ? m_pWnd->Show(a_bShow) : E_UNEXPECTED;
	}
	STDMETHOD(Move)(RECT const* a_prcPosition)
	{
		return m_pWnd ? m_pWnd->Move(a_prcPosition) : E_UNEXPECTED;
	}
	STDMETHOD(Destroy)()
	{
		return m_pWnd ? m_pWnd->Destroy() : E_UNEXPECTED;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_pWnd ? m_pWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)() {return S_FALSE;}
	STDMETHOD(OnDeactivate)(BOOL /*a_bCancelChanges*/) {return S_OK;}
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter /*a_eFilter*/, IEnumUnknownsInit* a_pInterfaces)
	{
		try
		{
			CComPtr<IUnknown> p;
			QueryInterface(a_iid, reinterpret_cast<void**>(&p));
			if (p == NULL)
				return S_FALSE;
			return a_pInterfaces->Insert(p);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		if (m_pWnd)
			return m_pWnd->OptimumSize(a_pSize);
		return E_NOTIMPL;
		//try
		//{
		//	SIZE sz;
		//	if (!Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, &sz, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)))
		//		return E_FAIL;
		//	if (a_pSize->cx < sz.cx) a_pSize->cx = sz.cx;
		//	if (a_pSize->cy < sz.cy) a_pSize->cy = sz.cy;
		//	return S_OK;
		//}
		//catch (...)
		//{
		//	return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		//}
	}
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		return S_OK;
	}

private:
	void Data2GUI()
	{
		CComPtr<IConfig> pEffect;
		m_pImage->LayerEffectGet(m_pLayer, &pEffect);
		if (m_pEffect == pEffect)
			return;
		if (CompareConfigValues(m_pEffect, pEffect) == S_OK)
			return;
		if (m_pEffect != NULL)
			m_pEffect->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		m_pEffect = pEffect;
		m_pEffect->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		m_pWnd->ConfigSet(m_pEffect);
	}

	HRESULT CompareConfigValues(IConfig* a_p1, IConfig* a_p2)
	{
		if (a_p1 == NULL || a_p2 == NULL)
			return E_POINTER;

		CComPtr<IEnumStrings> pES1;
		a_p1->ItemIDsEnum(&pES1);
		ULONG nItems1 = 0;
		pES1->Size(&nItems1);
		CComPtr<IEnumStrings> pES2;
		a_p2->ItemIDsEnum(&pES2);
		ULONG nItems2 = 0;
		pES2->Size(&nItems2);
		if (nItems1 != nItems2)
			return S_FALSE;

		HRESULT hr = S_OK;
		CAutoVectorPtr<BSTR> aIDs(new BSTR[nItems1]);
		pES1->GetMultiple(0, nItems1, aIDs);
		for (ULONG i = 0; i < nItems1; i++)
		{
			CConfigValue c1;
			CConfigValue c2;
			a_p1->ItemValueGet(aIDs[i], &c1);
			a_p2->ItemValueGet(aIDs[i], &c2);
			if (c1 != c2)
			{
				hr = S_FALSE;
				break;
			}
		}

		for (ULONG i = 0; i < nItems1; i++)
		{
			SysFreeString(aIDs[i]);
		}

		return hr;
	}

private:
	CComPtr<ISharedStateManager> m_pSharedState;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentLayeredImage> m_pImage;
	CComPtr<IComparable> m_pLayer;
	bool m_bEnableUpdates;
	std::wstring m_strSelGrp;
	CComPtr<IConfigWndCustom> m_pWnd;
	CComPtr<IConfig> m_pEffect;
};
