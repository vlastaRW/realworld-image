// EditToolRetouchDlg.h : Declaration of the CEditToolRetouchDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolRetouchDlg

class CEditToolRetouchDlg : 
	public CEditToolDlgBase<CEditToolRetouchDlg, CEditToolDataRetouch>,
	public CDialogResize<CEditToolRetouchDlg>,
	public CContextHelpDlg<CEditToolRetouchDlg>,
	public ISharedStateManager
{
public:
	CEditToolRetouchDlg()
	{
	}

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolsManager* a_pManager, LPCOLESTR a_pszInternal)
	{
		m_pManager = a_pManager;
		m_bstrInternal = a_pszInternal;
		return CEditToolDlgBase<CEditToolRetouchDlg, CEditToolDataRetouch>::Create(a_pszToolID, a_hParent, a_tLocaleID, a_pSharedState, a_bstrSyncGroup);
	}

	enum { IDD = IDD_EDITTOOL_RETOUCH };

	BEGIN_MSG_MAP(CEditToolRetouchDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolRetouchDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_RETOUCHEFFECT, CBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_RETROUCHSTREDIT, EN_CHANGE, OnChangeEdit)
		NOTIFY_HANDLER(IDC_ET_RETROUCHSTRSPIN, UDN_DELTAPOS, OnUpDownChange)
		CHAIN_MSG_MAP(CDialogResize<CEditToolRetouchDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolRetouchDlg)
		DLGRESIZE_CONTROL(IDC_ET_RETOUCHEFFECT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_RETROUCHSTREDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_RETROUCHSTRSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_RETROUCHSTRUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE2, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolRetouchDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_RETOUCHEFFECT, IDS_HELP_RETOUCHEFFECT)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_RETROUCHSTREDIT, IDS_HELP_RETOUCHSTRENGTH)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_RETROUCHSTRSPIN, IDS_HELP_RETOUCHSTRENGTH)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolRetouchDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataRetouch::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				DataToGUI();
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
	STDMETHOD(ObserverIns)(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pSharedState->ObserverIns(a_pObserver, a_tCookie);
	}
	STDMETHOD(ObserverDel)(ISharedStateObserver* a_pObserver, TCookie a_tCookie)
	{
		return m_pSharedState->ObserverDel(a_pObserver, a_tCookie);
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pInternal && S_OK == m_pInternal->PreTranslateMessage(a_pMsg, a_bBeforeAccel))
			return S_OK;
		return CEditToolDlgBase<CEditToolRetouchDlg, CEditToolDataRetouch>::PreTranslateMessage(a_pMsg, a_bBeforeAccel);
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
		m_wndStrength = GetDlgItem(IDC_ET_RETROUCHSTREDIT);
		AddWindowForPreTranslate(m_wndStrength);
		m_wndEffect = GetDlgItem(IDC_ET_RETOUCHEFFECT);
		CEditToolDataRetouch::SModeList* pList = CEditToolDataRetouch::GetModeList();
		while (pList->second)
		{
			TCHAR szTmp[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), pList->uIDName, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
			m_wndEffect.AddString(szTmp);
			++pList;
		}
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataRetouch::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataRetouch::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
	LRESULT OnChangeEdit(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndStrength.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fStrength))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fStrength > -500.0f)
			{
				m_cData.fStrength = max(-500.0f, m_cData.fStrength-10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fStrength);
				m_wndStrength.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fStrength < 500.0f)
			{
				m_cData.fStrength = min(500.0f, m_cData.fStrength+10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fStrength);
				m_wndStrength.SetWindowText(szTmp);
			}
		}

		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		m_wndStrength.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fStrength);
		m_cData.eMode = static_cast<CEditToolDataRetouch::EMode>(m_wndEffect.GetCurSel());
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		TCHAR szPrev[33] = _T("");
		m_wndStrength.GetWindowText(szPrev, itemsof(szPrev));
		float f = m_cData.fStrength+1.0f;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fStrength)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fStrength);
			m_wndStrength.SetWindowText(szTmp);
		}
		m_wndEffect.SetCurSel(m_cData.eMode);
		m_bEnableUpdates = true;
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComBSTR m_bstrInternal;
	CComPtr<IRasterImageEditToolWindow> m_pInternal;
	CComboBox m_wndEffect;
	CEdit m_wndStrength;
};


