
#pragma once

#include "resource.h"       // main symbols
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <SubjectImpl.h>
#include <ObserverImpl.h>


// CEditToolEffectDlg

class CEditToolEffectDlg : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CEditToolEffectDlg>,
	public CObserverImpl<CEditToolEffectDlg, ISharedStateObserver, TSharedStateChange>,
	public CChildWindowImpl<CEditToolEffectDlg, IRasterImageEditToolWindow>,
	public CDialogResize<CEditToolEffectDlg>,
	public CSubjectImpl<ISharedStateManager, ISharedStateObserver, TSharedStateChange>
{
public:
	CEditToolEffectDlg() : m_bEnableUpdates(false)
	{
	}
	~CEditToolEffectDlg()
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
		return Win32LangEx::CLangIndirectDialogImpl<CEditToolEffectDlg>::Create(a_hParent);
	}

	enum
	{
		IDC_ET_SEPLINE1 = 200,
		IDC_ET_SEPLINE2,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 100, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		//CONTROL_LTEXT(_T("[0409]Scale:[0405]Škála:"), IDC_SCALE_LABEL, 0, 2, 44, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_SCALE, 45, 0, 30, 12, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_CONTROL(_T(""), IDC_SCALE_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 75, 0, 45, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolEffectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		CHAIN_MSG_MAP(CDialogResize<CEditToolEffectDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolEffectDlg)
		//DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_ET_SHADOWSIZE, DLSZ_DIVSIZE_X(2))
		//DLGRESIZE_CONTROL(IDC_ET_SHADOWSIZE_UNIT, DLSZ_DIVMOVE_X(2))
		//DLGRESIZE_CONTROL(IDC_ET_SHADOWDENSITY, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		//DLGRESIZE_CONTROL(IDC_ET_SHADOWDENSITY_UNIT, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_COM_MAP(CEditToolEffectDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<ISharedStateEffect> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
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
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			SIZE sz1 = {0, 0};
			HRESULT hRes1 = S_OK;//Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, &sz1, m_tLocaleID);
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

	// ISharedStateManager methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (m_bstrInternal != a_bstrCategoryName)
			return m_pSharedState->StateGet(a_bstrCategoryName, a_iid, a_ppState);
		if (m_cData.pInternal)
			return m_cData.pInternal->QueryInterface(a_iid, a_ppState);
		return E_RW_ITEMNOTFOUND;
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (m_bstrInternal != a_bstrCategoryName)
			return m_pSharedState->StateSet(a_bstrCategoryName, a_pState);
		m_cData.pInternal = a_pState;
		if (m_bEnableUpdates)
			DataToState();
		return S_OK;
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pInternal && S_OK == m_pInternal->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
			return S_OK;
		return S_FALSE;
	}

	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pInternal)
		{
			m_pInternal->Destroy();
			m_pInternal = NULL;
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedStateEffect> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(ISharedStateEffect), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}
		DataToGUI();
		m_pManager->WindowCreate(m_bstrInternal, m_hWnd, m_tLocaleID, this, m_bstrInternal, &m_pInternal);
		if (m_pInternal)
		{
			RWHWND hInternal = 0;
			m_pInternal->Handle(&hInternal);
			::SetWindowLong(hInternal, GWLP_ID, IDC_ET_SEPLINE2);
			RECT rcInternal;
			GetClientRect(&rcInternal);
			//rcInternal.top = rcInternal.bottom;
			m_pInternal->Move(&rcInternal);
			m_pInternal->Show(TRUE);
		}
		else
		{
			GetDlgItem(IDC_ET_SEPLINE1).ShowWindow(SW_HIDE);
			CStatic wnd;
			wnd.Create(m_hWnd, 0, 0, WS_CHILD, 0, IDC_ET_SEPLINE2);
		}

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComObject<CSharedStateEffect>* pNew = NULL;
			CComObject<CSharedStateEffect>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			m_bEnableUpdates = false;
			m_pSharedState->StateSet(m_bstrSyncToolData, pTmp);
			m_bEnableUpdates = true;
		}
	}
	void GUIToData()
	{
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_bEnableUpdates = true;
	}

private:
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncToolData;
	CEditToolDataEffect m_cData;
	bool m_bEnableUpdates;
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComBSTR m_bstrInternal;
	CComPtr<IRasterImageEditToolWindow> m_pInternal;
};

