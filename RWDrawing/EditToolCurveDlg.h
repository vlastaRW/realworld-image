// EditToolCurveDlg.h : Declaration of the CEditToolCurveDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <XPGUI.h>
#include "EditToolDlg.h"


// CEditToolCurveDlg

class CEditToolCurveDlg : 
	public CEditToolDlgBase<CEditToolCurveDlg, CEditToolDataCurve, Win32LangEx::CLangIndirectDialogImpl<CEditToolCurveDlg> >,
	public CDialogResize<CEditToolCurveDlg>,
	public CContextHelpDlg<CEditToolCurveDlg>
{
public:
	CEditToolCurveDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolCurveDlg()
	{
		m_cImageList.Destroy();
	}

	enum
	{
		IDC_STROKE_MODES = 100, IDC_STROKE_WIDTHLABEL, IDC_STROKE_WIDTH, IDC_STROKE_WIDTHSPIN, IDC_STROKE_PIXELS, IDC_STROKE_DASHSWITCH, IDC_STROKE_DASHPATTERN,
		IDC_STROKE_CLOSED, IDC_STROKE_SQUARECAP, IDC_STROKE_ROUNDCAP, IDC_STROKE_BUTTCAP,
	};

	BEGIN_DIALOG_EX(0, 0, 110, 56, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_STROKE_MODES, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 90, 14, 0)
		CONTROL_LTEXT(_T("[0409]Width:[0405]Šířka:"), IDC_STROKE_WIDTHLABEL, 5, 25, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_STROKE_WIDTH, 57, 23, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_STROKE_WIDTHSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE, 69, 23, 12, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STROKE_PIXELS, 85, 25, 20, 8, WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Dash[0405]Čerchování"), IDC_STROKE_DASHSWITCH, 5, 40, 50, 10, BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_EDITTEXT(IDC_STROKE_DASHPATTERN, 57, 39, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolCurveDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolCurveDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_STROKE_SQUARECAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_ROUNDCAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_BUTTCAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_CLOSED, BN_CLICKED, OnClosedChange)
		COMMAND_HANDLER(IDC_STROKE_DASHSWITCH, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_WIDTH, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_STROKE_DASHPATTERN, EN_CHANGE, OnChangeEdit)
		NOTIFY_HANDLER(IDC_STROKE_WIDTHSPIN, UDN_DELTAPOS, OnUpDownChange)
		CHAIN_MSG_MAP(CDialogResize<CEditToolCurveDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolCurveDlg)
		DLGRESIZE_CONTROL(IDC_STROKE_PIXELS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_WIDTH, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_WIDTHSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_DASHPATTERN, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolCurveDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_STROKE_WIDTH, IDS_HELP_WIDTH)
		CTXHELP_CONTROL_RESOURCE(IDC_STROKE_WIDTHSPIN, IDS_HELP_WIDTH)
		CTXHELP_CONTROL_RESOURCE(IDC_STROKE_DASHSWITCH, IDS_HELP_DASHPATTERN)
		CTXHELP_CONTROL_RESOURCE(IDC_STROKE_DASHPATTERN, IDS_HELP_DASHPATTERN)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolCurveDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataCurve::ISharedStateToolData> pData(a_tParams.pState);
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
			if (m_tOptimumSize.cx && m_tOptimumSize.cy)
				*a_pSize = m_tOptimumSize;
			else
				GetDialogSize(a_pSize, m_tLocaleID);
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* UNREF(a_pState))
	{
		return S_OK;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndWidth = GetDlgItem(IDC_STROKE_WIDTH);
		AddWindowForPreTranslate(m_wndWidth);
		m_wndDash = GetDlgItem(IDC_STROKE_DASHPATTERN);
		AddWindowForPreTranslate(m_wndDash);

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataCurve::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataCurve::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CComPtr<IIconRenderer> pIR;
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));

		int nIconSize = XPGUI::GetSmallIconSize();
		m_cImageList.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 4, 0);

		static IRPolyPoint const s_aClosed[] =
		{ {0.1f, 0.5f}, {0.1586f, 0.3586f}, {0.3f, 0.3f}, {0.7f, 0.7f}, {0.8414f, 0.6414f}, {0.9f, 0.5f}, {0.8414f, 0.3586f}, {0.7f, 0.3f}, {0.3f, 0.7f}, {0.1586f, 0.6414f} };
		HICON hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aClosed), s_aClosed, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aNoCaps[] =
		{ {0.1f, 0.2f}, {0.5f, 0.2f}, {0.5f, 0.8f}, {0.1f, 0.8f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aNoCaps), s_aNoCaps, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aSquareCaps[] =
		{ {0.1f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.1f, 0.8f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aSquareCaps), s_aSquareCaps, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aRoundCaps[] =
		{ {0.1f, 0.2f}, {0.5f, 0.2f}, {0.6148f, 0.2228f}, {0.7121f, 0.2879f}, {0.7772f, 0.3852}, {0.8f, 0.5f}, {0.7772f, 0.6148f}, {0.7121f, 0.7121f}, {0.6148f, 0.7772f}, {0.5f, 0.8f}, {0.1f, 0.8f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aRoundCaps), s_aRoundCaps, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pD = szTooltipStrings;
		static wchar_t const* const s_aNames[] =
		{
			L"[0409]Closed[0405]Uzavřená",
			L"[0409]No caps[0405]Bez zakončení",
			L"[0409]Square caps[0405]Čtvercová zakončení",
			L"[0409]Round caps[0405]Kulatá zakončení",
		};
		for (wchar_t const* const* p = s_aNames; p != s_aNames+sizeof(s_aNames)/sizeof(*s_aNames); ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			wcscpy(pD, bstr.m_str == NULL ? L"" : bstr);
			pD += bstr.Length()+1;
			*pD = L'\0';
		}

		m_wndToolBar = GetDlgItem(IDC_STROKE_MODES);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		m_wndToolBar.AddStrings(szTooltipStrings);
		TBBUTTON aButtons[] =
		{
			{0, IDC_STROKE_CLOSED, TBSTATE_ENABLED|(m_cData.bClosed ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, 0},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{1, IDC_STROKE_BUTTCAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::butt_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
			{2, IDC_STROKE_SQUARECAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::square_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 2},
			{3, IDC_STROKE_ROUNDCAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::round_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 3},
		};
		m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
		}

		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		int iDelta = (rcToolbar.bottom-rcToolbar.top)-(rcActual.bottom-rcActual.top);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcActual.right = rcActual.left+rcToolbar.right;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);
		RECT rcWindow = {0, 0, 0, 0};
		GetClientRect(&rcWindow);
		m_tOptimumSize.cx = rcWindow.right-rcWindow.left;
		m_tOptimumSize.cy = rcWindow.bottom-rcWindow.top+iDelta;
		for (UINT i = IDC_STROKE_WIDTHLABEL; i <= IDC_STROKE_DASHPATTERN; ++i)
		{
			HWND hWnd = GetDlgItem(i);
			RECT rc;
			::GetWindowRect(hWnd, &rc);
			ScreenToClient(&rc);
			::SetWindowPos(hWnd, NULL, rc.left, rc.top+iDelta, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOREDRAW);
		}

		DataToGUI();

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
	LRESULT OnClosedChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		m_cData.bClosed = !m_cData.bClosed;
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeEdit(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndWidth.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fWidth))
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
			if (m_cData.fWidth > 0.0f)
			{
				m_cData.fWidth = max(0.0f, m_cData.fWidth-1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fWidth);
				m_wndWidth.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fWidth < 100.0f)
			{
				m_cData.fWidth = min(100.0f, m_cData.fWidth+1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fWidth);
				m_wndWidth.SetWindowText(szTmp);
			}
		}

		return 0;
	}

	void GUIToData()
	{
		m_cData.bClosed = m_wndToolBar.IsButtonChecked(IDC_STROKE_CLOSED);
		m_cData.eCapMode = m_wndToolBar.IsButtonChecked(IDC_STROKE_SQUARECAP) ? agg::square_cap :
			(m_wndToolBar.IsButtonChecked(IDC_STROKE_ROUNDCAP) ? agg::round_cap : agg::butt_cap);
		TCHAR szVal[32] = _T("");
		m_wndWidth.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fWidth);
		m_cData.bUseDash = IsDlgButtonChecked(IDC_STROKE_DASHSWITCH);
		TCHAR szTmp[128];
		int nLen = m_wndDash.GetWindowText(szTmp, itemsof(szTmp));
		if (nLen > 32) nLen = 32;
		m_cData.nDashLen = nLen;
		m_cData.dwDash = 0;
		for (int i = 0; i < nLen; ++i)
		{
			if (szTmp[i] != _T(' '))
				m_cData.dwDash |= 1<<i;
		}
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		TCHAR szPrev[32] = _T("");
		m_wndWidth.GetWindowText(szPrev, itemsof(szPrev));
		float f = m_cData.fWidth+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fWidth)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fWidth);
			m_wndWidth.SetWindowText(szTmp);
		}
		m_wndToolBar.CheckButton(IDC_STROKE_CLOSED, m_cData.bClosed);
		m_wndToolBar.CheckButton(IDC_STROKE_SQUARECAP, m_cData.eCapMode == agg::square_cap);
		m_wndToolBar.CheckButton(IDC_STROKE_BUTTCAP, m_cData.eCapMode == agg::butt_cap);
		m_wndToolBar.CheckButton(IDC_STROKE_ROUNDCAP, m_cData.eCapMode == agg::round_cap);
		m_wndToolBar.EnableButton(IDC_STROKE_SQUARECAP, !m_cData.bClosed || m_cData.bUseDash);
		m_wndToolBar.EnableButton(IDC_STROKE_ROUNDCAP, !m_cData.bClosed || m_cData.bUseDash);
		m_wndToolBar.EnableButton(IDC_STROKE_BUTTCAP, !m_cData.bClosed || m_cData.bUseDash);
		CheckDlgButton(IDC_STROKE_DASHSWITCH, m_cData.bUseDash ? BST_CHECKED : BST_UNCHECKED);
		m_wndDash.EnableWindow(m_cData.bUseDash);
		m_wndDash.GetWindowText(szPrev, itemsof(szPrev));
		TCHAR szTmp[33] = _T("");
		int i;
		for (i = 0; i < m_cData.nDashLen; ++i)
		{
			szTmp[i] = m_cData.dwDash & (1<<i) ? _T('-') : _T(' ');
		}
		szTmp[i] = _T('\0');
		if (_tcsncmp(szTmp, szPrev, 32) != 0)
		{
			m_wndDash.SetWindowText(szTmp);
		}
		m_bEnableUpdates = true;
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	CEdit m_wndWidth;
	CEdit m_wndDash;
	SIZE m_tOptimumSize;
};


