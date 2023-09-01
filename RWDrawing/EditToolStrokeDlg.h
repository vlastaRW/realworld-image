// EditToolPolygonDlg.h : Declaration of the CEditToolStrokeDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <XPGUI.h>


// CEditToolStrokeDlg

class CEditToolStrokeDlg : 
	public CEditToolDlgBase<CEditToolStrokeDlg, CEditToolDataStroke, Win32LangEx::CLangIndirectDialogImpl<CEditToolStrokeDlg> >,
	public CDialogResize<CEditToolStrokeDlg>
{
public:
	CEditToolStrokeDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolStrokeDlg()
	{
		m_cImageList.Destroy();
	}

	enum
	{
		IDD_STROKE_MODES = 100, IDC_STROKE_WIDTHLABEL, IDC_STROKE_WIDTH, IDC_STROKE_WIDTHSPIN, IDC_STROKE_PIXELS, IDC_STROKE_DASHSWITCH, IDC_STROKE_DASHPATTERN,
		IDC_STROKE_CLOSED, IDC_STROKE_SQUARECAP, IDC_STROKE_ROUNDCAP, IDC_STROKE_BUTTCAP, IDC_STROKE_MITERJOIN, IDC_STROKE_ROUNDJOIN, IDC_STROKE_BEVELJOIN,
		IDD_SHAPE_STARTWITH, IDC_SHAPE_SMOOTHINGLABEL, IDC_SHAPE_SMOOTHING,
		ID_START_HANDLES = 200, ID_START_FREEFORM,
	};

	BEGIN_DIALOG_EX(0, 0, 110, 90, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDD_STROKE_MODES, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 100, 14, 0)
		CONTROL_LTEXT(_T("[0409]Width:[0405]Šířka:"), IDC_STROKE_WIDTHLABEL, 5, 25, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_STROKE_WIDTH, 57, 23, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_STROKE_WIDTHSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE, 69, 23, 12, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STROKE_PIXELS, 85, 25, 20, 8, WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Dash[0405]Čerchování"), IDC_STROKE_DASHSWITCH, 5, 40, 50, 10, BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_EDITTEXT(IDC_STROKE_DASHPATTERN, 57, 39, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)

		CONTROL_CONTROL(_T(""), IDD_SHAPE_STARTWITH, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 55, 100, 14, 0)
		CONTROL_LTEXT(_T("[0409]Smoothing:[0405]Vyhlazování:"), IDC_SHAPE_SMOOTHINGLABEL, 5, 75, 50, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SHAPE_SMOOTHING, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 55, 73, 45, 12, 0)
		//CONTROL_EDITTEXT(IDC_SHAPE_SMOOTHING, 57, 23, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolStrokeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_STROKE_SQUARECAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_ROUNDCAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_BUTTCAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_MITERJOIN, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_ROUNDJOIN, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_BEVELJOIN, BN_CLICKED, OnChange)
		//COMMAND_HANDLER(IDC_STROKE_CLOSED, BN_CLICKED, OnClosedChange)
		COMMAND_HANDLER(IDC_STROKE_DASHSWITCH, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_STROKE_WIDTH, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_STROKE_DASHPATTERN, EN_CHANGE, OnChangeEdit)
		NOTIFY_HANDLER(IDC_STROKE_WIDTHSPIN, UDN_DELTAPOS, OnUpDownChange)
		COMMAND_HANDLER(ID_START_HANDLES, BN_CLICKED, OnChange)
		COMMAND_HANDLER(ID_START_FREEFORM, BN_CLICKED, OnChange)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		CHAIN_MSG_MAP(CDialogResize<CEditToolStrokeDlg>)
	END_MSG_MAP()

	BEGIN_COM_MAP(CEditToolStrokeDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolStrokeDlg)
		DLGRESIZE_CONTROL(IDC_STROKE_PIXELS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_WIDTH, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_WIDTHSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STROKE_DASHPATTERN, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SHAPE_SMOOTHING, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataStroke::ISharedStateToolData> pData(a_tParams.pState);
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
			*a_pSize = m_tOptimumSize;
			return m_tOptimumSize.cx ? S_OK : E_FAIL;
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

		RECT rcSize;
		GetClientRect(&rcSize);
		m_tOptimumSize.cx = rcSize.right;
		m_tOptimumSize.cy = rcSize.bottom;

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataStroke::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataStroke::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		m_cImageList.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 2, 0);
		
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

		static IRPolyPoint const s_aMiterJoins[] =
		{ {0.1f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aMiterJoins), s_aMiterJoins, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aBevelJoins[] =
		{ {0.1f, 0.2f}, {0.4f, 0.2f}, {0.8f, 0.6f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aBevelJoins), s_aBevelJoins, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aRoundJoins[] =
		{ {0.1f, 0.2f}, {0.2f, 0.2f}, {0.4296f, 0.2457f}, {0.6243f, 0.3757f}, {0.7543f, 0.5704f}, {0.8f, 0.8f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aRoundJoins), s_aRoundJoins, nIconSize, false);
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pD = szTooltipStrings;
		static wchar_t const* const s_aNames[] =
		{
			L"[0409]Closed[0405]Uzavřená",
			L"[0409]No caps[0405]Bez zakončení",
			L"[0409]Square caps[0405]Čtvercová zakončení",
			L"[0409]Round caps[0405]Kulatá zakončení",
			L"[0409]Miter joins[0405]Ostré zlomy",
			L"[0409]Bevel joins[0405]Oříznuté zlomy",
			L"[0409]Round joins[0405]Kulaté zlomy",
		};
		for (wchar_t const* const* p = s_aNames; p != s_aNames+sizeof(s_aNames)/sizeof(*s_aNames); ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			wcscpy(pD, bstr.m_str == NULL ? L"" : bstr);
			pD += bstr.Length()+1;
			*pD = L'\0';
		}

		m_wndLineProps = GetDlgItem(IDD_STROKE_MODES);
		m_wndLineProps.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndLineProps.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndLineProps.SetImageList(m_cImageList);
		m_wndLineProps.AddStrings(szTooltipStrings);
		TBBUTTON aButtonsLP[] =
		{
			//{0, IDC_STROKE_CLOSED, TBSTATE_ENABLED|(m_cData.bClosed ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, 0},
			//{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{1, IDC_STROKE_BUTTCAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::butt_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
			{2, IDC_STROKE_SQUARECAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::square_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 2},
			{3, IDC_STROKE_ROUNDCAP, TBSTATE_ENABLED|(m_cData.eCapMode == agg::round_cap ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 3},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{4, IDC_STROKE_MITERJOIN, TBSTATE_ENABLED|(m_cData.eJoinMode == agg::miter_join ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 4},
			{5, IDC_STROKE_BEVELJOIN, TBSTATE_ENABLED|(m_cData.eJoinMode == agg::bevel_join ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 5},
			{6, IDC_STROKE_ROUNDJOIN, TBSTATE_ENABLED|(m_cData.eJoinMode == agg::round_join ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 6},
		};
		m_wndLineProps.AddButtons(itemsof(aButtonsLP), aButtonsLP);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndLineProps.SetButtonSize(nButtonSize, nButtonSize);
		}
		{
			RECT rcToolbar;
			m_wndLineProps.GetItemRect(m_wndLineProps.GetButtonCount()-1, &rcToolbar);
			RECT rcActual;
			m_wndLineProps.GetWindowRect(&rcActual);
			ScreenToClient(&rcActual);
			int delta = rcActual.bottom-rcActual.top-rcToolbar.bottom;
			MoveWindows(rcActual.bottom, delta);
			rcActual.bottom += delta;
			rcActual.right = rcActual.left+rcToolbar.right;
			m_wndLineProps.MoveWindow(&rcActual, FALSE);
			m_tOptimumSize.cy += delta;
		}


		int iconIndex = m_cImageList.GetImageCount();
		// add control handles
		{
			static IRPolyPoint const aPoly[] =
			{
				{0.0f, 1.0f}, {0.0f, 0.7f}, {0.15f, 0.75f}, {0.35f, 0.55f}, {0.38f, 0.42f}, {0.42f, 0.38f}, {0.55f, 0.35f}, {0.75f, 0.15f}, {0.7f, 0.0f},
				{1.0f, 0.0f}, {1.0f, 0.3f}, {0.85f, 0.25f}, {0.65f, 0.45f}, {0.62f, 0.58f}, {0.58f, 0.62f}, {0.45f, 0.65f}, {0.25f, 0.85f}, {0.3f, 1.0f},
			};
			HICON hTmp = IconFromPolygon(pSI, pIR, itemsof(aPoly), aPoly, nIconSize, true);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		// add custom shape
		{
			static IRPolyPoint const aPoly[] =
			{
				{0.1f, 0.4f}, {0.5f, 0.6f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f},
			};
			HICON hTmp = IconFromPolygon(pSI, pIR, itemsof(aPoly), aPoly, nIconSize, false);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}

		pD = szTooltipStrings;
		static wchar_t const* const s_aNamesEM[] =
		{
			L"[0409]Place handles[0405]Vkládat body",
			L"[0409]Draw shape[0405]Kreslit tvar",
		};
		for (wchar_t const* const* p = s_aNamesEM; p != s_aNamesEM+sizeof(s_aNamesEM)/sizeof(*s_aNamesEM); ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			wcscpy(pD, bstr.m_str == NULL ? L"" : bstr);
			pD += bstr.Length()+1;
			*pD = L'\0';
			// TODO: range check
		}

		m_wndEditMode = GetDlgItem(IDD_SHAPE_STARTWITH);
		m_wndEditMode.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndEditMode.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndEditMode.SetImageList(m_cImageList);
		m_wndEditMode.AddStrings(szTooltipStrings);
		TBBUTTON aButtons[] =
		{
			{iconIndex+0, ID_START_HANDLES, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 0},
			{iconIndex+1, ID_START_FREEFORM, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
		};
		m_wndEditMode.AddButtons(2, aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndEditMode.SetButtonSize(nButtonSize, nButtonSize);
		}
		{
			RECT rcToolbar;
			m_wndEditMode.GetItemRect(m_wndEditMode.GetButtonCount()-1, &rcToolbar);
			RECT rcActual;
			m_wndEditMode.GetWindowRect(&rcActual);
			ScreenToClient(&rcActual);
			int delta = rcActual.bottom-rcActual.top-rcToolbar.bottom;
			MoveWindows(rcActual.bottom, delta);
			rcActual.bottom += delta;
			rcActual.right = rcActual.left+rcToolbar.right;
			m_wndEditMode.MoveWindow(&rcActual, FALSE);
			m_tOptimumSize.cy += delta;
		}

		m_wndLabel = GetDlgItem(IDC_SHAPE_SMOOTHINGLABEL);
		m_wndSmoothing = GetDlgItem(IDC_SHAPE_SMOOTHING);

		m_wndSmoothing.SetRange(0, 100);
		m_wndSmoothing.SetPageSize(10);

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
	//LRESULT OnClosedChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	//{
	//	if (!m_bEnableUpdates)
	//		return 0;

	//	m_cData.bClosed = !m_cData.bClosed;
	//	DataToGUI();
	//	DataToState();
	//	return 0;
	//}
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
	LRESULT OnScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		m_cData.fSmoothing = exp((m_wndSmoothing.GetPos()-50)/50.0f*logf(4.0f));
		DataToState();
		return 0;
	}

	void GUIToData()
	{
		//CEditToolDataStroke::EPathMode prev = m_cData.eMode;
		m_cData.eMode = m_wndEditMode.IsButtonChecked(ID_START_HANDLES) ? CEditToolDataStroke::EPMControlHandles : CEditToolDataStroke::EPMFreeform;
		//if (prev != m_cData.eMode)
		//	m_cData.eState = CEditToolDataStroke::EESPassive;
		//m_cData.bClosed = m_wndLineProps.IsButtonChecked(IDC_STROKE_CLOSED);
		m_cData.eCapMode = m_wndLineProps.IsButtonChecked(IDC_STROKE_SQUARECAP) ? agg::square_cap :
			(m_wndLineProps.IsButtonChecked(IDC_STROKE_ROUNDCAP) ? agg::round_cap : agg::butt_cap);
		m_cData.eJoinMode = m_wndLineProps.IsButtonChecked(IDC_STROKE_MITERJOIN) ? agg::miter_join :
			(m_wndLineProps.IsButtonChecked(IDC_STROKE_ROUNDJOIN) ? agg::round_join : agg::bevel_join);
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
		m_wndLineProps.CheckButton(IDC_STROKE_MITERJOIN, m_cData.eJoinMode == agg::miter_join);
		m_wndLineProps.CheckButton(IDC_STROKE_ROUNDJOIN, m_cData.eJoinMode == agg::round_join);
		m_wndLineProps.CheckButton(IDC_STROKE_BEVELJOIN, m_cData.eJoinMode == agg::bevel_join);
		//m_wndLineProps.CheckButton(IDC_STROKE_CLOSED, m_cData.bClosed);
		m_wndLineProps.CheckButton(IDC_STROKE_SQUARECAP, m_cData.eCapMode == agg::square_cap);
		m_wndLineProps.CheckButton(IDC_STROKE_BUTTCAP, m_cData.eCapMode == agg::butt_cap);
		m_wndLineProps.CheckButton(IDC_STROKE_ROUNDCAP, m_cData.eCapMode == agg::round_cap);
		//m_wndLineProps.EnableButton(IDC_STROKE_SQUARECAP, !m_cData.bClosed || m_cData.bUseDash);
		//m_wndLineProps.EnableButton(IDC_STROKE_ROUNDCAP, !m_cData.bClosed || m_cData.bUseDash);
		//m_wndLineProps.EnableButton(IDC_STROKE_BUTTCAP, !m_cData.bClosed || m_cData.bUseDash);
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

		UINT id = m_cData.eMode == CEditToolDataStroke::EPMControlHandles ? ID_START_HANDLES : ID_START_FREEFORM;
		if (!m_wndEditMode.IsButtonChecked(id))
		{
			m_wndEditMode.SetState(ID_START_HANDLES, m_cData.eMode == CEditToolDataStroke::EPMControlHandles ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
			m_wndEditMode.SetState(ID_START_FREEFORM, m_cData.eMode == CEditToolDataStroke::EPMFreeform ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		}
		m_wndSmoothing.SetPos(logf(m_cData.fSmoothing)/logf(4.0f)*50+50);
		m_wndLabel.EnableWindow(m_cData.eMode == CEditToolDataStroke::EPMFreeform);
		m_wndSmoothing.EnableWindow(m_cData.eMode == CEditToolDataStroke::EPMFreeform);
		m_bEnableUpdates = true;
	}

	struct SMoveHelper
	{
		HWND hwnd;
		int from;
		int delta;
	};
	static BOOL CALLBACK MoveWindowClbk(HWND hwnd, LPARAM lParam)
	{
		SMoveHelper const* params = reinterpret_cast<SMoveHelper const*>(lParam);
		RECT rc;
		::GetWindowRect(hwnd, &rc);
		::ScreenToClient(params->hwnd, reinterpret_cast<LPPOINT>(&rc));
		if (rc.top >= params->from)
			::SetWindowPos(hwnd, NULL, rc.left, rc.top+params->delta, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOREDRAW|SWP_NOZORDER);
		return TRUE;
	}
	void MoveWindows(int from, int delta)
	{

		SMoveHelper params = {m_hWnd, from, delta};
		EnumChildWindows(m_hWnd, MoveWindowClbk, reinterpret_cast<LPARAM>(&params));
	}

private:
	CToolBarCtrl m_wndEditMode;
	CImageList m_cImageList;
	CTrackBarCtrl m_wndSmoothing;
	CWindow m_wndLabel;
	CToolBarCtrl m_wndLineProps;
	CEdit m_wndWidth;
	CEdit m_wndDash;
	SIZE m_tOptimumSize;
};


