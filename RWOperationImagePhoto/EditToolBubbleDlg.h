// EditToolBubbleDlg.h : Declaration of the CEditToolBubbleDlg

#pragma once

#include "resource.h"       // main symbols
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <SubjectImpl.h>
#include <WTL_ColorPicker.h>

/*
class CSharedStateInternal :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISharedStateManager
{
public:
	void SetOwner(ISharedStateManager* a_pOwner)
	{
		m_pOwner = a_pOwner;
	}

	BEGIN_COM_MAP(CSharedStateInternal)
		COM_INTERFACE_ENTRY(ISharedStateManager)
	END_COM_MAP()

	// ISharedStateManager methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		return m_pOwner->StateGet(a_bstrCategoryName, a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		return m_pOwner->StateSet(a_bstrCategoryName, a_pState);
	}
	STDMETHOD(ObserverIns)(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pOwner->ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ObserverDel)(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pOwner->ObserverDel(a_pObserver, a_tCookie);
	}

private:
	ISharedStateManager* m_pOwner;
};
*/

// CEditToolBubbleDlg

class CEditToolBubbleDlg : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CEditToolBubbleDlg>,
	public CObserverImpl<CEditToolBubbleDlg, ISharedStateObserver, TSharedStateChange>,
	public CChildWindowImpl<CEditToolBubbleDlg, IRasterImageEditToolWindow>,
	public CDialogResize<CEditToolBubbleDlg>,
	public CContextHelpDlg<CEditToolBubbleDlg>,
	public CSubjectImpl<ISharedStateManager, ISharedStateObserver, TSharedStateChange>
{
public:
	CEditToolBubbleDlg() : m_bEnableUpdates(false)
	{
	}
	~CEditToolBubbleDlg()
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolsManager* a_pManager, LPCOLESTR a_pszInternal)
	{
		m_pManager = a_pManager;
		m_bstrInternal = a_pszInternal;
		m_tLocaleID = a_tLocaleID;
		m_pSharedState = a_pSharedState;
		m_bstrSyncToolData = a_bstrSyncGroup;
		m_pSharedState->ObserverIns(ObserverGet(), 0);
		return Win32LangEx::CLangDialogImpl<CEditToolBubbleDlg>::Create(a_hParent);
	}

	enum { IDD = IDD_EDITTOOL_BUBBLE, IDC_ET_SUBTOOL = 300 };

	BEGIN_MSG_MAP(CEditToolBubbleDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolBubbleDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		NOTIFY_HANDLER(IDC_ET_BUBBLECOLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		CHAIN_MSG_MAP(CDialogResize<CEditToolBubbleDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolBubbleDlg)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_ET_BUBBLECOLOR, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SUBTOOL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolBubbleDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BUBBLECOLOR, IDS_HELP_BUBBLECOLOR)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolBubbleDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<ISharedStateBubble> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				if (m_bEnableUpdates)
					DataToGUI();
				if (m_cData.pInternal)
				{
					TSharedStateChange tChange;
					tChange.bstrName = m_bstrInternal;
					tChange.pState = m_cData.pInternal;
					Fire_Notify(tChange);
				}
			}
		}
		else
		{
			Fire_Notify(a_tParams);
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			SIZE sz1 = {0, 0};
			HRESULT hRes1 = Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, &sz1, m_tLocaleID);
			SIZE sz2 = {0, 0};
			HRESULT hRes2 = m_pInternal ? m_pInternal->OptimumSize(&sz2) : S_OK;
			a_pSize->cx = max(sz1.cx, sz2.cx);
			a_pSize->cy = sz1.cy + sz2.cy;
			return SUCCEEDED(hRes1) ? hRes2 : hRes1;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return m_pInternal ? m_pInternal->SetState(a_pState) : S_OK;
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pInternal && S_OK == m_pInternal->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
			return S_OK;
		return S_FALSE;
	}

	// ISharedStateManager methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (m_bstrInternal && wcscmp(a_bstrCategoryName, m_bstrInternal) == 0)
		{
			return m_cData.pInternal ? m_cData.pInternal->QueryInterface(a_iid, a_ppState) : E_RW_ITEMNOTFOUND;
		}
		return m_pSharedState->StateGet(a_bstrCategoryName, a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (m_bstrInternal && wcscmp(a_bstrCategoryName, m_bstrInternal) == 0)
		{
			m_cData.pInternal = a_pState;
			if (m_bEnableUpdates)
				DataToState();
			return S_OK;
		}
		return m_pSharedState->StateSet(a_bstrCategoryName, a_pState);
	}

public:
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndColor.SubclassWindow(GetDlgItem(IDC_ET_BUBBLECOLOR));
		m_wndColor.SetDefaultColor(CButtonColorPicker::SColor(1.0f, 1.0f, 1.0f, 1.0f));

		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedStateBubble> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(ISharedStateBubble), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}
		CComObject<CSharedStateInternal>* pIntState = NULL;
		CComObject<CSharedStateInternal>::CreateInstance(&pIntState);
		m_pInternalSharedState = pIntState;
		pIntState->SetOwner(this);
		DataToGUI();
		m_pManager->WindowCreate(m_bstrInternal, m_hWnd, m_tLocaleID, m_pInternalSharedState, m_bstrInternal, &m_pInternal);
		if (m_pInternal)
		{
			RWHWND hInternal = 0;
			m_pInternal->Handle(&hInternal);
			::SetWindowLong(hInternal, GWLP_ID, IDC_ET_SUBTOOL);
			RECT rcInternal;
			GetClientRect(&rcInternal);
			rcInternal.top = rcInternal.bottom;
			m_pInternal->Move(&rcInternal);
			m_pInternal->Show(TRUE);
		}
		else
		{
			GetDlgItem(IDC_ET_SEPLINE1).ShowWindow(SW_HIDE);
			CStatic wnd;
			wnd.Create(m_hWnd, 0, 0, WS_CHILD, 0, IDC_ET_SUBTOOL);
		}

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	//LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	//{
	//	if (!m_bEnableUpdates)
	//		return 0;

	//	GUIToData();
	//	DataToGUI();
	//	DataToState();
	//	return 0;
	//}

	LRESULT OnColorChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		if (!m_bEnableUpdates)
			return 0;

		CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
		m_cData.tFill.fR = pClr->clr.fR;
		m_cData.tFill.fG = pClr->clr.fG;
		m_cData.tFill.fB = pClr->clr.fB;
		m_cData.tFill.fA = pClr->clr.fA;

		GUIToData();
		DataToState();
		return 0;
	}

	void GUIToData()
	{
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_wndColor.SetColor(CButtonColorPicker::SColor(m_cData.tFill.fR, m_cData.tFill.fG, m_cData.tFill.fB, m_cData.tFill.fA), true);
		m_bEnableUpdates = true;
	}

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComObject<CSharedStateBubble>* p = NULL;
			CComObject<CSharedStateBubble>::CreateInstance(&p);
			CComPtr<ISharedState> pTmp = p;
			p->Init(m_cData);
			m_bEnableUpdates = false;
			m_pSharedState->StateSet(m_bstrSyncToolData, pTmp);
			m_bEnableUpdates = true;
		}
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComBSTR m_bstrInternal;
	CComPtr<IRasterImageEditToolWindow> m_pInternal;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComPtr<ISharedStateManager> m_pInternalSharedState;
	CComBSTR m_bstrSyncToolData;
	CButtonColorPicker m_wndColor;
	CEditToolDataBubble m_cData;
	bool m_bEnableUpdates;
};


