// EditToolRecolorDlg.h : Declaration of the CEditToolRecolorDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <WTL_ColorPicker.h>


// CEditToolRecolorDlg

class CEditToolRecolorDlg : 
	public CEditToolDlgBase<CEditToolRecolorDlg, CEditToolDataRecolor, Win32LangEx::CLangIndirectDialogImpl<CEditToolRecolorDlg> >,
	public CDialogResize<CEditToolRecolorDlg>,
	public CContextHelpDlg<CEditToolRecolorDlg>,
	public ISharedStateManager
{
public:
	CEditToolRecolorDlg() : m_wndColor(false)
	{
	}

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolsManager* a_pManager, LPCOLESTR a_pszInternal)
	{
		m_pManager = a_pManager;
		m_bstrInternal = a_pszInternal;
		return CEditToolDlgBase<CEditToolRecolorDlg, CEditToolDataRecolor, Win32LangEx::CLangIndirectDialogImpl<CEditToolRecolorDlg> >::Create(a_pszToolID, a_hParent, a_tLocaleID, a_pSharedState, a_bstrSyncGroup);
	}

	enum
	{
		IDC_COLORPICKER = 100,
		IDC_TOLERANCE_EDIT,
		IDC_TOLERANCE_SPIN,
		IDC_TOLERANCE_UNIT,
		IDC_SEPARATORLINE,
		IDC_SUBTOOLDLD,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 38, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Replace:[0409]Nahradit:"), IDC_STATIC, 5, 7, 45, 8, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_COLORPICKER, 53, 5, 35, 12, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Tolerance:[0409]Tolerance:"), IDC_STATIC, 5, 23, 45, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_TOLERANCE_EDIT, 53, 21, 16, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_TOLERANCE_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 58, 21, 11, 12, 0)
		CONTROL_LTEXT(_T("%"), IDC_TOLERANCE_UNIT, 74, 23, 10, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SEPARATORLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 5, 37, 90, 1, 0)
	END_CONTROLS_MAP()


	BEGIN_MSG_MAP(CEditToolRecolorDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolRecolorDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		//COMMAND_HANDLER(IDC_ET_RETOUCHEFFECT, CBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(IDC_TOLERANCE_EDIT, EN_CHANGE, OnChangeEdit)
		NOTIFY_HANDLER(IDC_TOLERANCE_SPIN, UDN_DELTAPOS, OnUpDownChange)
		NOTIFY_HANDLER(IDC_COLORPICKER, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		CHAIN_MSG_MAP(CDialogResize<CEditToolRecolorDlg>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolRecolorDlg)
		//DLGRESIZE_CONTROL(IDC_COLORPICKER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TOLERANCE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TOLERANCE_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TOLERANCE_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SEPARATORLINE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SUBTOOLDLD, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolRecolorDlg)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_RETOUCHEFFECT, IDS_HELP_RETOUCHEFFECT)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_RETROUCHSTREDIT, IDS_HELP_RETOUCHSTRENGTH)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_RETROUCHSTRSPIN, IDS_HELP_RETOUCHSTRENGTH)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolRecolorDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataRecolor::ISharedStateToolData> pData(a_tParams.pState);
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
			HRESULT hRes1 = GetDialogSize(&sz1, m_tLocaleID);
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
		return CEditToolDlgBase<CEditToolRecolorDlg, CEditToolDataRecolor, Win32LangEx::CLangIndirectDialogImpl<CEditToolRecolorDlg> >::PreTranslateMessage(a_pMsg, a_bBeforeAccel);
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
		m_wndColor.SubclassWindow(GetDlgItem(IDC_COLORPICKER));
		m_wndColor.SetDefaultColor(CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f));
		CComBSTR bstr;
		CMultiLanguageString::GetLocalized(L"[0409]Clicked pixel[0405]Klinutý pixel", m_tLocaleID, &bstr);
		m_wndColor.SetDefaultText(bstr);

		m_wndTolerance = GetDlgItem(IDC_TOLERANCE_EDIT);
		AddWindowForPreTranslate(m_wndTolerance);
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataRecolor::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataRecolor::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
			::SetWindowLong(hInternal, GWLP_ID, IDC_SUBTOOLDLD);
			RECT rcInternal;
			GetClientRect(&rcInternal);
			rcInternal.top = rcInternal.bottom;
			m_pInternal->Move(&rcInternal);
			m_pInternal->Show(TRUE);
		}
		else
		{
			GetDlgItem(IDC_SEPARATORLINE).ShowWindow(SW_HIDE);
			CStatic wnd;
			wnd.Create(m_hWnd, 0, 0, WS_CHILD, 0, IDC_SUBTOOLDLD);
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
		m_wndTolerance.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fTolerance))
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
			if (m_cData.fTolerance > 0.0f)
			{
				m_cData.fTolerance = max(0.0f, m_cData.fTolerance-10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fTolerance);
				m_wndTolerance.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fTolerance < 100.0f)
			{
				m_cData.fTolerance = min(100.0f, m_cData.fTolerance+10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fTolerance);
				m_wndTolerance.SetWindowText(szTmp);
			}
		}

		return 0;
	}

	LRESULT OnColorChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		if (!m_bEnableUpdates)
			return 0;

		CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
		m_cData.tToReplace.fR = pClr->clr.fR;
		m_cData.tToReplace.fG = pClr->clr.fG;
		m_cData.tToReplace.fB = pClr->clr.fB;
		m_cData.tToReplace.fA = pClr->clr.fA;

		GUIToData();
		DataToState();
		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		m_wndTolerance.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fTolerance);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		TCHAR szPrev[33] = _T("");
		m_wndTolerance.GetWindowText(szPrev, itemsof(szPrev));
		float f = m_cData.fTolerance+1.0f;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fTolerance)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fTolerance);
			m_wndTolerance.SetWindowText(szTmp);
		}
		m_bEnableUpdates = true;
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComBSTR m_bstrInternal;
	CComPtr<IRasterImageEditToolWindow> m_pInternal;
	CButtonColorPicker m_wndColor;
	CEdit m_wndTolerance;
};


