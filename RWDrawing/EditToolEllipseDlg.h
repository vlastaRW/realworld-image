// EditToolEllipseDlg.h : Declaration of the CEditToolEllipseDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolEllipseDlg

class CEditToolEllipseDlg : 
	public CEditToolDlgBase<CEditToolEllipseDlg, CEditToolDataEllipse, Win32LangEx::CLangIndirectDialogImpl<CEditToolEllipseDlg> >
{
public:
	CEditToolEllipseDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolEllipseDlg()
	{
		m_cImageList.Destroy();
	}

	enum { IDC_TYPE = 100, ID_CIRCLE = 120, ID_ELLIPSE, };

	BEGIN_DIALOG_EX(0, 0, 100, 24, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_TYPE, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 90, 14, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolEllipseDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(ID_CIRCLE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(ID_ELLIPSE, BN_CLICKED, OnChange)
	END_MSG_MAP()

	BEGIN_COM_MAP(CEditToolEllipseDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataEllipse::ISharedStateToolData> pData(a_tParams.pState);
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
			CComPtr<CEditToolDataEllipse::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataEllipse::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		IRCanvas canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};
		IRPolyPoint aShape[16];
		GetEllipse(aShape, itemsof(aShape), 0.8f, 0.8f);
		IRPolygon poly = {itemsof(aShape), aShape};
		HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		GetEllipse(aShape, itemsof(aShape), 1.0f, 0.6f);
		hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
		m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);

		CComBSTR bstrCircle;
		CMultiLanguageString::GetLocalized(L"[0409]Circle[0405]Kružnice", m_tLocaleID, &bstrCircle);
		CComBSTR bstrEllipse;
		CMultiLanguageString::GetLocalized(L"[0409]Ellipse[0405]Elipsa", m_tLocaleID, &bstrEllipse);

		m_wndToolBar = GetDlgItem(IDC_TYPE);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		TBBUTTON aButtons[] =
		{
			{0, ID_CIRCLE, TBSTATE_ENABLED|(m_cData.eMode == CEditToolDataEllipse::EMCircle ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, INT_PTR(bstrCircle.m_str)},
			{1, ID_ELLIPSE, TBSTATE_ENABLED|(m_cData.eMode == CEditToolDataEllipse::EMEllipse ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, INT_PTR(bstrEllipse.m_str)},
		};
		m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
		}

		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcMargins = {0, 0, 10, 10};
		MapDialogRect(&rcMargins);
		m_tOptimumSize.cx = rcToolbar.right+rcMargins.right;
		m_tOptimumSize.cy = rcToolbar.bottom+rcMargins.bottom;
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcActual.right = rcActual.left+rcToolbar.right;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);

		DataToGUI();

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

		m_cData.eMode = a_wID == ID_CIRCLE ? CEditToolDataEllipse::EMCircle : CEditToolDataEllipse::EMEllipse;
		DataToGUI();
		DataToState();
		return 0;
	}

	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_wndToolBar.SetState(ID_CIRCLE, m_cData.eMode == CEditToolDataEllipse::EMCircle ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		m_wndToolBar.SetState(ID_ELLIPSE, m_cData.eMode == CEditToolDataEllipse::EMEllipse ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		m_bEnableUpdates = true;
	}

private:
	void GetEllipse(IRPolyPoint* a_p, int a_n, float a_fX, float a_fY)
	{
		for (int i = 0; i < a_n; ++i)
		{
			a_p[i].x = cosf(i*3.1415926536f*2.0f/a_n)*a_fX*0.5f+0.5f;
			a_p[i].y = sinf(i*3.1415926536f*2.0f/a_n)*a_fY*0.5f+0.5f;
		}
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tOptimumSize;
};


