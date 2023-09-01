// EditToolRectangleDlg.h : Declaration of the CEditToolRectangleDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolRectangleDlg

class CEditToolRectangleDlg : 
	public CEditToolDlgBase<CEditToolRectangleDlg, CEditToolDataRectangle, Win32LangEx::CLangIndirectDialogImpl<CEditToolRectangleDlg> >,
	public CDialogResize<CEditToolRectangleDlg>,
	public CContextHelpDlg<CEditToolRectangleDlg>
{
public:
	CEditToolRectangleDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolRectangleDlg()
	{
		m_cImageList.Destroy();
	}

	enum { IDC_TYPE = 100, IDC_RADIUS_LABEL, IDC_RADIUS, IDC_RADIUSSPIN, IDC_RADIUS_UNIT, ID_SQUARE = 120, ID_RECTANGLE, };

	BEGIN_DIALOG_EX(0, 0, 110, 40, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_TYPE, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 90, 14, 0)
		CONTROL_LTEXT(_T("[0409]Corner radius:[0405]Poloměr rohů:"), IDC_RADIUS_LABEL, 5, 26, 52, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_RADIUS, 58, 24, 23, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_RADIUSSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE, 71, 24, 12, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_RADIUS_UNIT, 86, 26, 23, 8, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolRectangleDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolRectangleDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(ID_SQUARE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(ID_RECTANGLE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_RADIUS, EN_CHANGE, OnChangeRadius)
		NOTIFY_HANDLER(IDC_RADIUSSPIN, UDN_DELTAPOS, OnUpDownChangeRadius)
		CHAIN_MSG_MAP(CDialogResize<CEditToolRectangleDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolRectangleDlg)
		DLGRESIZE_CONTROL(IDC_RADIUS_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RADIUS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_RADIUSSPIN, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolRectangleDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_RADIUS, IDS_HELP_RADIUS)
		CTXHELP_CONTROL_RESOURCE(IDC_RADIUSSPIN, IDS_HELP_RADIUS)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolRectangleDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

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

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataRectangle::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				DataToGUI();
			}
		}
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndRadius = GetDlgItem(IDC_RADIUS);
		AddWindowForPreTranslate(m_wndRadius);

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataRectangle::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataRectangle::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		{
			static IRPolyPoint const aPoly[] = { {0.1f, 0.1f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f} };
			static IRPolygon const poly = {itemsof(aPoly), aPoly};
			static IRGridItem const gr[] = {{0, 0.1f}, {0, 0.9f}};
			IRCanvas canvas = {0, 0, 1, 1, itemsof(gr), itemsof(gr), gr, gr};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
			m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}
		{
			static IRPolyPoint const aPoly[] = { {0.0f, 0.2f}, {1.0f, 0.2f}, {1.0f, 0.8f}, {0.0f, 0.8f} };
			static IRPolygon const poly = {itemsof(aPoly), aPoly};
			static IRGridItem const grx[] = {{0, 0.0f}, {0, 1.0f}};
			static IRGridItem const gry[] = {{0, 0.2f}, {0, 0.8f}};
			IRCanvas canvas = {0, 0, 1, 1, itemsof(grx), itemsof(gry), grx, gry};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
			m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}

		CComBSTR bstrSquare;
		CMultiLanguageString::GetLocalized(L"[0409]Square[0405]Čtverec", m_tLocaleID, &bstrSquare);
		CComBSTR bstrRectangle;
		CMultiLanguageString::GetLocalized(L"[0409]Rectangle[0405]Obdélník", m_tLocaleID, &bstrRectangle);

		m_wndToolBar = GetDlgItem(IDC_TYPE);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		TBBUTTON aButtons[] =
		{
			{0, ID_SQUARE, TBSTATE_ENABLED|(m_cData.eMode == CEditToolDataRectangle::EMSquare ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, INT_PTR(bstrSquare.m_str)},
			{1, ID_RECTANGLE, TBSTATE_ENABLED|(m_cData.eMode == CEditToolDataRectangle::EMRectangle ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, INT_PTR(bstrRectangle.m_str)},
		};
		m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
		}

		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcMargins = {10, 10, 110, 40};
		MapDialogRect(&rcMargins);
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		LONG nDelta = rcActual.bottom-(rcActual.top+rcToolbar.bottom);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcActual.right = rcActual.left+rcToolbar.right;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);
		m_tOptimumSize.cx = rcMargins.right;
		m_tOptimumSize.cy = rcMargins.bottom-nDelta;

		for (UINT nID = IDC_RADIUS_LABEL; nID <= IDC_RADIUS_UNIT; ++nID)
		{
			CWindow wnd = GetDlgItem(nID);
			RECT rc = {0, 0, 0, 0};
			wnd.GetWindowRect(&rc);
			ScreenToClient(&rc);
			rc.top -= nDelta;
			rc.bottom -= nDelta;
			wnd.MoveWindow(&rc, FALSE);
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

		m_cData.eMode = a_wID == ID_SQUARE ? CEditToolDataRectangle::EMSquare : CEditToolDataRectangle::EMRectangle;
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeRadius(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndRadius.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fCornerRadius))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnUpDownChangeRadius(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fCornerRadius > 0.0f)
			{
				m_cData.fCornerRadius = max(0.0f, m_cData.fCornerRadius-1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fCornerRadius);
				m_wndRadius.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fCornerRadius < 100.0f)
			{
				m_cData.fCornerRadius = min(100.0f, m_cData.fCornerRadius+1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fCornerRadius);
				m_wndRadius.SetWindowText(szTmp);
			}
		}

		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		m_wndRadius.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fCornerRadius);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_wndToolBar.SetState(ID_SQUARE, m_cData.eMode == CEditToolDataRectangle::EMSquare ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		m_wndToolBar.SetState(ID_RECTANGLE, m_cData.eMode == CEditToolDataRectangle::EMRectangle ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		TCHAR szPrevRadius[32] = _T("");
		m_wndRadius.GetWindowText(szPrevRadius, itemsof(szPrevRadius));
		float f = m_cData.fCornerRadius+1;
		_stscanf(szPrevRadius, _T("%f"), &f);
		if (f != m_cData.fCornerRadius)
		{
			TCHAR szRadius[32] = _T("");
			_stprintf(szRadius, _T("%g"), m_cData.fCornerRadius);
			m_wndRadius.SetWindowText(szRadius);
		}
		m_bEnableUpdates = true;
	}

private:
	CEdit m_wndRadius;
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tOptimumSize;
};


