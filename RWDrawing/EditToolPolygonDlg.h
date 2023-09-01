// EditToolBrushDlg.h : Declaration of the CEditToolPolygonDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <XPGUI.h>
#include <IconRenderer.h>
#include <set>


// CEditToolPolygonDlg

class CEditToolPolygonDlg : 
	public CEditToolDlgBase<CEditToolPolygonDlg, CEditToolDataPolygon, Win32LangEx::CLangIndirectDialogImpl<CEditToolPolygonDlg> >
{
public:
	CEditToolPolygonDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolPolygonDlg()
	{
		m_cImageList.Destroy();
	}

	enum { IDD_POLYGON_STARTWITH = 100 };

	BEGIN_DIALOG_EX(0, 0, 100, 24, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDD_POLYGON_STARTWITH, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 5, 5, 90, 14, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolPolygonDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_RANGE_CODE_HANDLER(IDC_ET_PENCILSHAPE, IDC_ET_PENCILSHAPE+CEditToolDataPolygon::ESWCount, BN_CLICKED, OnChange)
	END_MSG_MAP()

	BEGIN_COM_MAP(CEditToolPolygonDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataPolygon::ISharedStateToolData> pData(a_tParams.pState);
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
			CComPtr<CEditToolDataPolygon::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataPolygon::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		m_cImageList.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), CEditToolDataPolygon::ESWCount+1, 0);
		ATLASSERT(CEditToolDataPolygon::ESWCount == itemsof(g_aStartPolys));
		for (SStartPoly const* p = g_aStartPolys; p < g_aStartPolys+itemsof(g_aStartPolys); ++p)
		{
			IRAutoCanvas canvas(p->n, reinterpret_cast<IRPolyPoint const*>(p->aPoly));

			if (p->scale < 1.0f)
			{
				float dx = 0.5f*(canvas.x1-canvas.x0)*(1-p->scale)/p->scale;
				canvas.x0 -= dx;
				canvas.x1 += dx;
				float dy = 0.5f*(canvas.y1-canvas.y0)*(1-p->scale)/p->scale;
				canvas.y0 -= dy;
				canvas.y1 += dy;
			}
			IRPolygon poly = {p->n, reinterpret_cast<IRPolyPoint const*>(p->aPoly)};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));//IconFromPolygon(p->n, p->aPoly, nIconSize, true);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		// add custom shape
		{
			static IRPolyPoint const aPoly[] =
			{
				{0.1f, 0.4f}, {0.5f, 0.6f}, {0.9f, 0.1f}, {0.9f, 0.9f}, {0.1f, 0.9f},
			};
			IRPolygon poly = {itemsof(aPoly), aPoly};
			static IRGridItem const gr[] = {{0, 0.1f}, {0, 0.9f}};
			IRCanvas canvas = {0, 0, 1, 1, itemsof(gr), itemsof(gr), gr, gr};
			HICON hTmp = pIR->CreateIcon(nIconSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));//IconFromPolygon(itemsof(aPoly), aPoly, nIconSize, true);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}

		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pD = szTooltipStrings;
		static wchar_t const* const s_aNames[] =
		{
			L"[0409]Triangle[0405]Trojúhelník",
			L"[0409]Square[0405]Čtverec",
			L"[0409]Pentagon[0405]Pětiúhelník",
			L"[0409]Hexagon[0405]Šestiúhelník",
			L"[0409]Octagon[0405]Osmiúhelník",
			L"[0409]Five-pointed star[0405]Pěticípá hvězda",
			L"[0409]Six-pointed star[0405]Šesticípá hvězda",
			L"[0409]Arrow[0405]Šipka",
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

		m_wndToolBar = GetDlgItem(IDD_POLYGON_STARTWITH);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		m_wndToolBar.AddStrings(szTooltipStrings);
		CAutoVectorPtr<TBBUTTON> pButtons(new TBBUTTON[CEditToolDataPolygon::ESWCount+2]);
		for (int i = 0; i < CEditToolDataPolygon::ESWCount+1; ++i)
		{
			pButtons[i].iBitmap = i;
			pButtons[i].idCommand = i+IDC_ET_PENCILSHAPE;
			pButtons[i].fsState = TBSTATE_ENABLED|(m_cData.eStartWith == i ? TBSTATE_CHECKED : 0);
			pButtons[i].fsStyle = BTNS_BUTTON|BTNS_CHECKGROUP;
			//pButtons[i].bReserved[2 or 6];          // padding for alignment
			pButtons[i].dwData = 0;
			pButtons[i].iString = i;
		}
		pButtons[CEditToolDataPolygon::ESWCount+1] = pButtons[CEditToolDataPolygon::ESWCount];
		pButtons[CEditToolDataPolygon::ESWCount].iBitmap = 0;
		pButtons[CEditToolDataPolygon::ESWCount].idCommand = 0;
		pButtons[CEditToolDataPolygon::ESWCount].fsState = 0;
		pButtons[CEditToolDataPolygon::ESWCount].fsStyle = BTNS_SEP;
		pButtons[CEditToolDataPolygon::ESWCount].iString = 0;
		m_wndToolBar.AddButtons(CEditToolDataPolygon::ESWCount+2, pButtons);
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

		m_cData.eStartWith = static_cast<CEditToolDataPolygon::EStartWith>(a_wID-IDC_ET_PENCILSHAPE);
		DataToState();
		return 0;
	}

	void DataToGUI()
	{
		m_bEnableUpdates = false;
		//if (!m_wndToolBar.IsButtonChecked(IDC_ET_PENCILSHAPE+m_cData.eStartWith))
		{
			for (int i = 0; i <= CEditToolDataPolygon::ESWCount; ++i)
				m_wndToolBar.SetState(IDC_ET_PENCILSHAPE+i, m_cData.eStartWith == i ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		}
		m_bEnableUpdates = true;
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tOptimumSize;
};


