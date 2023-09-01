// EditToolPolygonDlg.h : Declaration of the CEditToolShapeDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <XPGUI.h>


// CEditToolShapeDlg

class CEditToolShapeDlg : 
	public CEditToolDlgBase<CEditToolShapeDlg, CEditToolDataPath, Win32LangEx::CLangIndirectDialogImpl<CEditToolShapeDlg> >,
	public CDialogResize<CEditToolShapeDlg>
{
public:
	CEditToolShapeDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolShapeDlg()
	{
		m_cImageList.Destroy();
	}

	enum
	{
		IDD_SHAPE_STARTWITH = 100, IDC_SHAPE_SMOOTHINGLABEL, IDC_SHAPE_SMOOTHING,
		ID_START_HANDLES = 200, ID_START_FREEFORM,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 40, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDD_SHAPE_STARTWITH, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 90, 14, 0)
		CONTROL_LTEXT(_T("[0409]Smoothing:[0405]Vyhlazování:"), IDC_SHAPE_SMOOTHINGLABEL, 5, 25, 50, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SHAPE_SMOOTHING, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 55, 23, 35, 12, 0)
		//CONTROL_EDITTEXT(IDC_SHAPE_SMOOTHING, 57, 23, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolShapeDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(ID_START_HANDLES, BN_CLICKED, OnChange)
		COMMAND_HANDLER(ID_START_FREEFORM, BN_CLICKED, OnChange)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		CHAIN_MSG_MAP(CDialogResize<CEditToolShapeDlg>)
	END_MSG_MAP()

	BEGIN_COM_MAP(CEditToolShapeDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolRectangleDlg)
		DLGRESIZE_CONTROL(IDC_SHAPE_SMOOTHING, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataPath::ISharedStateToolData> pData(a_tParams.pState);
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
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataPath::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataPath::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		// add control handles
		{
			static IRPolyPoint const aPoly[] =
			{
				{0.0f, 1.0f}, {0.0f, 0.7f}, {0.15f, 0.75f}, {0.35f, 0.55f}, {0.38f, 0.42f}, {0.42f, 0.38f}, {0.55f, 0.35f}, {0.75f, 0.15f}, {0.7f, 0.0f},
				{1.0f, 0.0f}, {1.0f, 0.3f}, {0.85f, 0.25f}, {0.65f, 0.45f}, {0.62f, 0.58f}, {0.58f, 0.62f}, {0.45f, 0.65f}, {0.25f, 0.85f}, {0.3f, 1.0f},
			};
			IRAutoCanvas canvas(itemsof(aPoly), aPoly);
			static IRPolygon const poly = {itemsof(aPoly), aPoly};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		// add custom shape
		{
			static IRPolyPoint const aPoly[] =
			{
				{0.1f, 0.4f}, {0.5f, 0.6f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f},
			};
			static IRGridItem const gr[] = {{0, 0.1f}, {0, 0.9f}};
			IRCanvas canvas = {0, 0, 1, 1, itemsof(gr), itemsof(gr), gr, gr};
			static IRPolygon const poly = {itemsof(aPoly), aPoly};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}

		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pD = szTooltipStrings;
		static wchar_t const* const s_aNames[] =
		{
			L"[0409]Place handles[0405]Vkládat body",
			L"[0409]Draw shape[0405]Kreslit tvar",
		};
		for (wchar_t const* const* p = s_aNames; p != s_aNames+sizeof(s_aNames)/sizeof(*s_aNames); ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			wcscpy(pD, bstr.m_str == NULL ? L"" : bstr);
			pD += bstr.Length()+1;
			*pD = L'\0';
			// TODO: range check
		}

		m_wndToolBar = GetDlgItem(IDD_SHAPE_STARTWITH);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		m_wndToolBar.AddStrings(szTooltipStrings);
		TBBUTTON aButtons[] =
		{
			{0, ID_START_HANDLES, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 0},
			{1, ID_START_FREEFORM, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
		};
		m_wndToolBar.AddButtons(2, aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
		}
		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcMargins = {0, 18, 100, 10};
		MapDialogRect(&rcMargins);
		m_tOptimumSize.cx = /*rcToolbar.right+*/rcMargins.right;
		m_tOptimumSize.cy = rcToolbar.bottom+rcMargins.top+rcMargins.bottom;
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		LONG nYDiff = rcToolbar.bottom-rcActual.bottom+rcActual.top;
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcActual.right = rcActual.left+rcToolbar.right;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);

		m_wndLabel = GetDlgItem(IDC_SHAPE_SMOOTHINGLABEL);
		m_wndLabel.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		rcActual.top -= nYDiff;
		rcActual.bottom -= nYDiff;
		m_wndLabel.MoveWindow(&rcActual);

		m_wndSmoothing = GetDlgItem(IDC_SHAPE_SMOOTHING);
		m_wndSmoothing.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		rcActual.top -= nYDiff;
		rcActual.bottom -= nYDiff;
		m_wndSmoothing.MoveWindow(&rcActual);

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

		CEditToolDataPath::EPathMode prev = m_cData.eMode;
		m_cData.eMode = a_wID == ID_START_HANDLES ? CEditToolDataPath::EPMControlHandles : CEditToolDataPath::EPMFreeform;
		if (prev != m_cData.eMode)
			m_cData.eState = CEditToolDataPath::EESPassive;
		DataToState();
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

	void DataToGUI()
	{
		m_bEnableUpdates = false;
		UINT id = m_cData.eMode == CEditToolDataPath::EPMControlHandles ? ID_START_HANDLES : ID_START_FREEFORM;
		if (!m_wndToolBar.IsButtonChecked(id))
		{
			m_wndToolBar.SetState(ID_START_HANDLES, m_cData.eMode == CEditToolDataPath::EPMControlHandles ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
			m_wndToolBar.SetState(ID_START_FREEFORM, m_cData.eMode == CEditToolDataPath::EPMFreeform ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		}
		m_wndSmoothing.SetPos(logf(m_cData.fSmoothing)/logf(4.0f)*50+50);
		m_wndLabel.EnableWindow(m_cData.eMode == CEditToolDataPath::EPMFreeform);
		m_wndSmoothing.EnableWindow(m_cData.eMode == CEditToolDataPath::EPMFreeform);
		m_bEnableUpdates = true;
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	CTrackBarCtrl m_wndSmoothing;
	CWindow m_wndLabel;
	SIZE m_tOptimumSize;
};


