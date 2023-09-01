// EditToolVistaGlassDlg.h : Declaration of the CEditToolStylizeDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <SubjectImpl.h>
#include "EditToolDlg.h"


// CEditToolStylizeDlg

class CEditToolStylizeDlg : 
	public CEditToolDlgBase<CEditToolStylizeDlg, CEditToolDataStylize>,
	public CDialogResize<CEditToolStylizeDlg>,
	public CContextHelpDlg<CEditToolStylizeDlg>,
	public CSubjectImpl<ISharedStateManager, ISharedStateObserver, TSharedStateChange>
{
public:
	CEditToolStylizeDlg()
	{
	}

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolsManager* a_pManager, LPCOLESTR a_pszInternal)
	{
		m_pManager = a_pManager;
		m_bstrInternal = a_pszInternal;
		return CEditToolDlgBase<CEditToolStylizeDlg, CEditToolDataStylize>::Create(a_pszToolID, a_hParent, a_tLocaleID, a_pSharedState, a_bstrSyncGroup);
	}

	enum { IDD = IDD_EDITTOOL_VISTAGLASS };

	BEGIN_MSG_MAP(CEditToolStylizeDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolStylizeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_BLURBACKGROUND, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_DROPSHADOW, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_ADDREFLECTION, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_SHADOWSIZE, EN_CHANGE, OnChangeSize)
		COMMAND_HANDLER(IDC_ET_SHADOWDENSITY, EN_CHANGE, OnChangeDensity)
		CHAIN_MSG_MAP(CDialogResize<CEditToolStylizeDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolStylizeDlg)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SHADOWSIZE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_SHADOWSIZE_UNIT, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_SHADOWDENSITY, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_SHADOWDENSITY_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolStylizeDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BLURBACKGROUND, IDS_HELP_BLURBACKGROUND)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_DROPSHADOW, IDS_HELP_DROPSHADOW)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SHADOWSIZE, IDS_HELP_SHADOWSIZE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SHADOWDENSITY, IDS_HELP_SHADOWDENSITY)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_ADDREFLECTION, IDS_HELP_ADDREFLECTION)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolStylizeDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataStylize::ISharedStateToolData> pData(a_tParams.pState);
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
		if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd)
		{
			if (m_wndShSize == a_pMsg->hwnd || m_wndShDens == a_pMsg->hwnd)
			{
				if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
					return S_OK;
				TranslateMessage(a_pMsg);
				DispatchMessage(a_pMsg);
				return S_OK;
			}
		}
		return S_FALSE;
	}

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
		m_wndShSize = GetDlgItem(IDC_ET_SHADOWSIZE);
		m_wndShDens = GetDlgItem(IDC_ET_SHADOWDENSITY);
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataStylize::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataStylize::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
			rcInternal.top = rcInternal.bottom;
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
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		GUIToData();
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeSize(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndShSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%i"), &m_cData.nShadowSize))
			return 0;
		DataToState();
		return 0;
	}
	LRESULT OnChangeDensity(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndShDens.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%i"), &m_cData.nShadowDensity))
			return 0;
		DataToState();
		return 0;
	}

	void GUIToData()
	{
		m_cData.bBlur = IsDlgButtonChecked(IDC_ET_BLURBACKGROUND);
		m_cData.bShadow = IsDlgButtonChecked(IDC_ET_DROPSHADOW);
		m_cData.bReflection = IsDlgButtonChecked(IDC_ET_ADDREFLECTION);
		TCHAR szVal[32] = _T("");
		m_wndShSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%i"), &m_cData.nShadowSize);
		m_wndShDens.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%i"), &m_cData.nShadowDensity);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_ET_BLURBACKGROUND, m_cData.bBlur ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_DROPSHADOW, m_cData.bShadow ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_ADDREFLECTION, m_cData.bReflection ? BST_CHECKED : BST_UNCHECKED);
		TCHAR szPrev[32] = _T("");
		m_wndShSize.GetWindowText(szPrev, itemsof(szPrev));
		int nPrev = m_cData.nShadowSize+1;
		_stscanf(szPrev, _T("%i"), &nPrev);
		if (nPrev != m_cData.nShadowSize)
		{
			TCHAR sz[32] = _T("");
			_stprintf(sz, _T("%i"), m_cData.nShadowSize);
			m_wndShSize.SetWindowText(sz);
		}
		m_wndShDens.GetWindowText(szPrev, itemsof(szPrev));
		nPrev = m_cData.nShadowDensity+1;
		_stscanf(szPrev, _T("%i"), &nPrev);
		if (nPrev != m_cData.nShadowDensity)
		{
			TCHAR sz[32] = _T("");
			_stprintf(sz, _T("%i"), m_cData.nShadowDensity);
			m_wndShDens.SetWindowText(sz);
		}
		m_wndShSize.EnableWindow(m_cData.bShadow);
		GetDlgItem(IDC_ET_SHADOWSIZE_UNIT).EnableWindow(m_cData.bShadow);
		m_wndShDens.EnableWindow(m_cData.bShadow);
		GetDlgItem(IDC_ET_SHADOWDENSITY_UNIT).EnableWindow(m_cData.bShadow);
		m_bEnableUpdates = true;
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComBSTR m_bstrInternal;
	CComPtr<IRasterImageEditToolWindow> m_pInternal;
	CEdit m_wndShSize;
	CEdit m_wndShDens;
};

