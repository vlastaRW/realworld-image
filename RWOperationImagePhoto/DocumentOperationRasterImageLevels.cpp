// DocumentOperationRasterImageLevels.cpp : Implementation of CDocumentOperationRasterImageLevels

#include "stdafx.h"
#include "DocumentOperationRasterImageLevels.h"
#include <SharedStringTable.h>

const OLECHAR CFGID_L_INBLACK[] = L"InBlack";
const OLECHAR CFGID_L_INWHITE[] = L"InWhite";
const OLECHAR CFGID_L_GAMMA[] = L"Gamma";
const OLECHAR CFGID_L_OUTBLACK[] = L"OutBlack";
const OLECHAR CFGID_L_OUTWHITE[] = L"OutWhite";

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorArea.h>
#include <WTL_ColorPicker.h>
#include <XPGUI.h>
#include <IconRenderer.h>

extern __declspec(selectany) TCHAR const  COLORAREAHORZHWNDCLASS[] = _T("WTL_ColorAreaHorzH");
extern __declspec(selectany) TCHAR const  COLORAREABLACKWNDCLASS[] = _T("WTL_ColorAreaBlack");
extern __declspec(selectany) TCHAR const  COLORAREAWHITEWNDCLASS[] = _T("WTL_ColorAreaWhite");
extern __declspec(selectany) TCHAR const  COLORAREAGRAYWNDCLASS[] = _T("WTL_ColorAreaGray");

class ATL_NO_VTABLE CConfigGUILevels :
	public CCustomConfigResourcelessWndImpl<CConfigGUILevels>,
	public CDialogResize<CConfigGUILevels>
{
public:
	~CConfigGUILevels()
	{
		m_cImageList.Destroy();
	}
	enum
	{
		ID_BLACK_PICK = 300, ID_GRAY_PICK, ID_WHITE_PICK, ID_GRAY_RESET, ID_BLACK_AUTO, ID_WHITE_AUTO,
		IDC_CGL_BLACK_LABEL = 100, IDC_CGL_BLACK_TOOLBAR, IDC_CGL_BLACK_2D, IDC_CGL_BLACK_1D,
		IDC_CGL_GRAY_LABEL, IDC_CGL_GRAY_TOOLBAR, IDC_CGL_GRAY_2D, IDC_CGL_GRAY_1D,
		IDC_CGL_WHITE_LABEL, IDC_CGL_WHITE_TOOLBAR, IDC_CGL_WHITE_2D, IDC_CGL_WHITE_1D
	};

	BEGIN_DIALOG_EX(0, 0, 120, 191, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Black point:[0405]Úroveň černé:"), IDC_CGL_BLACK_LABEL, 0, 2, 46, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_BLACK_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 50, 0, 70, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_BLACK_2D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 12, 106, 47, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_BLACK_1D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 110, 12, 10, 47, 0)
		CONTROL_LTEXT(_T("[0409]Gray point:[0405]Úroveň šedé:"), IDC_CGL_GRAY_LABEL, 0, 68, 46, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_GRAY_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 50, 66, 70, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_GRAY_2D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 78, 106, 47, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_GRAY_1D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 110, 78, 10, 47, 0)
		CONTROL_LTEXT(_T("[0409]White point:[0405]Úroveň bílé:"), IDC_CGL_WHITE_LABEL, 0, 134, 46, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_WHITE_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 50, 132, 70, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_WHITE_2D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 144, 106, 47, 0)
		CONTROL_CONTROL(_T(""), IDC_CGL_WHITE_1D, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 110, 144, 10, 47, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILevels)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILevels>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILevels>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGL_BLACK_2D, CColorAreaBlack::CAN_COLOR_CHANGED, OnBlack2DChanged)
		NOTIFY_HANDLER(IDC_CGL_WHITE_2D, CColorAreaWhite::CAN_COLOR_CHANGED, OnWhite2DChanged)
		NOTIFY_HANDLER(IDC_CGL_GRAY_2D, CColorAreaHue::CAN_COLOR_CHANGED, OnGray2DChanged)
		NOTIFY_HANDLER(IDC_CGL_BLACK_1D, CColorAreaVertH::CAN_COLOR_CHANGED, OnBlack1DChanged)
		NOTIFY_HANDLER(IDC_CGL_GRAY_1D, CColorAreaVertH::CAN_COLOR_CHANGED, OnGray1DChanged)
		NOTIFY_HANDLER(IDC_CGL_WHITE_1D, CColorAreaVertH::CAN_COLOR_CHANGED, OnWhite1DChanged)
		COMMAND_HANDLER(ID_BLACK_PICK, BN_CLICKED, OnPickBlack)
		COMMAND_HANDLER(ID_GRAY_PICK, BN_CLICKED, OnPickGray)
		COMMAND_HANDLER(ID_WHITE_PICK, BN_CLICKED, OnPickWhite)
		COMMAND_HANDLER(ID_GRAY_RESET, BN_CLICKED, OnResetGray)
		COMMAND_HANDLER(ID_BLACK_AUTO, BN_CLICKED, OnAutoBlack)
		COMMAND_HANDLER(ID_WHITE_AUTO, BN_CLICKED, OnAutoWhite)
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 0.5f; return TRUE; }
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILevels)
		CONFIGITEM_CONTEXTHELP(IDC_CGL_BLACK_2D, CFGID_L_INBLACK)
		CONFIGITEM_CONTEXTHELP(IDC_CGL_GRAY_2D, CFGID_L_GAMMA)
		CONFIGITEM_CONTEXTHELP(IDC_CGL_WHITE_2D, CFGID_L_INWHITE)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILevels)
		//DLGRESIZE_CONTROL(IDC_CGL_BLACK_LABEL, 0)
		DLGRESIZE_CONTROL(IDC_CGL_BLACK_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGL_BLACK_2D, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGL_BLACK_1D, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGL_GRAY_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGL_GRAY_2D, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGL_GRAY_1D, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGL_WHITE_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGL_WHITE_2D, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGL_WHITE_1D, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	void ExtraInitDialog()
	{
		m_wndTbBlack = GetDlgItem(IDC_CGL_BLACK_TOOLBAR);
		m_wndTbGray = GetDlgItem(IDC_CGL_GRAY_TOOLBAR);
		m_wndTbWhite = GetDlgItem(IDC_CGL_WHITE_TOOLBAR);

		m_cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 2, 1);
		HICON hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PICK_COLOR), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
		hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_RESET_GRAY), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
		hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_LEVEL_AUTO), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR);
		m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);

		m_wndTbBlack.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndTbBlack.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndTbBlack.SetImageList(m_cImageList);
		m_wndTbGray.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndTbGray.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndTbGray.SetImageList(m_cImageList);
		m_wndTbWhite.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndTbWhite.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndTbWhite.SetImageList(m_cImageList);

		TCHAR szTooltipStrings[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_LEVELS_TIPS, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(m_tLocaleID));
		for (TCHAR* p = szTooltipStrings; *p; ++p)
			if (*p == _T('|')) *p = _T('\0');
		m_wndTbBlack.AddStrings(szTooltipStrings);
		m_wndTbGray.AddStrings(szTooltipStrings);
		m_wndTbWhite.AddStrings(szTooltipStrings);
		TBBUTTON atButtons[] =
		{
			{2, ID_BLACK_AUTO, m_pImage ? TBSTATE_ENABLED : 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{0, ID_BLACK_PICK, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
			{2, ID_WHITE_AUTO, m_pImage ? TBSTATE_ENABLED : 0, BTNS_BUTTON, TBBUTTON_PADDING, 0, 2},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{0, ID_WHITE_PICK, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
			{1, ID_GRAY_RESET, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 1},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{0, ID_GRAY_PICK, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 0},
		};
		m_wndTbBlack.AddButtons(3, atButtons);
		RECT rcWnd;
		m_wndTbBlack.GetWindowRect(&rcWnd);
		RECT rcTB;
		m_wndTbBlack.GetItemRect(2, &rcTB);
		rcTB.left = rcWnd.right-rcTB.right;
		rcTB.top = rcWnd.top;
		rcTB.right = rcWnd.right;
		LONG const nDYB = rcTB.bottom-(rcWnd.bottom-rcWnd.top);
		rcTB.bottom = rcWnd.top+rcTB.bottom;
		ScreenToClient(&rcTB);
		m_wndTbBlack.MoveWindow(&rcTB);
		m_wndTbBlack.ShowWindow(SW_SHOW);

		m_wndTbGray.AddButtons(3, atButtons+6);
		m_wndTbGray.GetWindowRect(&rcWnd);
		m_wndTbGray.GetItemRect(2, &rcTB);
		rcTB.left = rcWnd.right-rcTB.right;
		rcTB.top = rcWnd.top;
		rcTB.right = rcWnd.right;
		LONG const nDYG = rcTB.bottom-(rcWnd.bottom-rcWnd.top);
		rcTB.bottom = rcWnd.top+rcTB.bottom;
		ScreenToClient(&rcTB);
		m_wndTbGray.MoveWindow(&rcTB);
		m_wndTbGray.ShowWindow(SW_SHOW);

		m_wndTbWhite.AddButtons(3, atButtons+3);
		m_wndTbWhite.GetWindowRect(&rcWnd);
		m_wndTbWhite.GetItemRect(2, &rcTB);
		rcTB.left = rcWnd.right-rcTB.right;
		rcTB.top = rcWnd.top;
		rcTB.right = rcWnd.right;
		LONG const nDYW = rcTB.bottom-(rcWnd.bottom-rcWnd.top);
		rcTB.bottom = rcWnd.top+rcTB.bottom;
		ScreenToClient(&rcTB);
		m_wndTbWhite.MoveWindow(&rcTB);
		m_wndTbWhite.ShowWindow(SW_SHOW);

		CWindow wnd(GetDlgItem(IDC_CGL_BLACK_2D));
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYB;
		wnd.DestroyWindow();
		m_wndLSBlack.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_BLACK_2D);

		wnd = GetDlgItem(IDC_CGL_BLACK_1D);
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYB;
		wnd.DestroyWindow();
		m_wndHueBlack.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_BLACK_1D);

		wnd = GetDlgItem(IDC_CGL_GRAY_2D);
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYG;
		wnd.DestroyWindow();
		m_wndLSGray.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_GRAY_2D);

		wnd = GetDlgItem(IDC_CGL_GRAY_1D);
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYG;
		wnd.DestroyWindow();
		m_wndHueGray.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_GRAY_1D);

		wnd = GetDlgItem(IDC_CGL_WHITE_2D);
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYW;
		wnd.DestroyWindow();
		m_wndLSWhite.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_WHITE_2D);

		wnd = GetDlgItem(IDC_CGL_WHITE_1D);
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.top += nDYW;
		wnd.DestroyWindow();
		m_wndHueWhite.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGL_WHITE_1D);
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CConfigValue cBlack;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INBLACK), &cBlack);
		CColorBaseRGBHLSA tBlack;
		tBlack.SetR(cBlack[0]);
		tBlack.SetG(cBlack[1]);
		tBlack.SetB(cBlack[2]);
		m_wndHueBlack.SetColor(tBlack);
		m_wndLSBlack.SetColor(tBlack);

		CConfigValue cWhite;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INWHITE), &cWhite);
		CColorBaseRGBHLSA tWhite;
		tWhite.SetR(cWhite[0]);
		tWhite.SetG(cWhite[1]);
		tWhite.SetB(cWhite[2]);
		m_wndHueWhite.SetColor(tWhite);
		m_wndLSWhite.SetColor(tWhite);

		GetDocument();

		UpdateGray();

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnPickBlack(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseRGBHLSA cColor;
		cColor.SetRGB(tCR);
		m_wndLSBlack.SetColor(cColor);
		BOOL b;
		OnBlack2DChanged(0, NULL, b);
		return 0;
	}
	LRESULT OnAutoBlack(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		try
		{
			TImageSize tSize = {1, 1};
			m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			ULONG const nPixels = tSize.nX*tSize.nY;
			if (nPixels == 0)
				return 0;
			CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);
			if (FAILED(m_pImage->TileGet(EICIRGBA, NULL, &tSize, NULL, nPixels, pPixels, NULL, EIRIAccurate)))
				return 0;
			float const fTolerance = 0.3f;

			ULONG aHist[256];
			ZeroMemory(aHist, sizeof aHist);
			TPixelChannel* p = pPixels;
			ULONG nRealPixels = 0;
			for (TPixelChannel* const pEnd = p+nPixels; p != pEnd; ++p)
			{
				if (p->bA == 0) // ignore transparent pixels // TODO: weight of semitransparent ones should be lower
					continue;
				ULONG n = (p->bR*5 + (ULONG(p->bG)<<3) + p->bB*3 + 8)>>4;
				++aHist[n];
				++nRealPixels;
			}

			if (nRealPixels == 0)
				return 0;

			ULONG const nLimit = nRealPixels*0.01f*fTolerance + 0.5f;

			ULONG n = 0;
			ULONG nSum = 0;
			while ((nSum += aHist[n]) < nLimit && n < 85) ++n;

			CColorBaseRGBHLSA cColor;
			cColor.SetRGB(RGB(n, n, n));
			m_wndLSBlack.SetColor(cColor);
			BOOL b;
			OnBlack2DChanged(0, NULL, b);
		}
		catch (...)
		{
		}
		return 0;
	}
	LRESULT OnPickGray(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseRGBHLSA cColor;
		cColor.SetRGB(tCR);
		m_wndLSGray.SetColor(cColor);
		BOOL b;
		OnGray2DChanged(0, NULL, b);
		return 0;
	}
	LRESULT OnPickWhite(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseRGBHLSA cColor;
		cColor.SetRGB(tCR);
		m_wndLSWhite.SetColor(cColor);
		BOOL b;
		OnWhite2DChanged(0, NULL, b);
		return 0;
	}
	LRESULT OnAutoWhite(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		try
		{
			TImageSize tSize = {1, 1};
			m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			ULONG const nPixels = tSize.nX*tSize.nY;
			if (nPixels == 0)
				return 0;
			CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);
			if (FAILED(m_pImage->TileGet(EICIRGBA, NULL, &tSize, NULL, nPixels, pPixels, NULL, EIRIAccurate)))
				return 0;
			float const fTolerance = 0.3f;

			ULONG aHist[256];
			ZeroMemory(aHist, sizeof aHist);
			TPixelChannel* p = pPixels;
			ULONG nRealPixels = 0;
			for (TPixelChannel* const pEnd = p+nPixels; p != pEnd; ++p)
			{
				if (p->bA == 0) // ignore transparent pixels // TODO: weight of semitransparent ones should be lower
					continue;
				ULONG n = (p->bR*5 + (ULONG(p->bG)<<3) + p->bB*3 + 8)>>4;
				++aHist[n];
				++nRealPixels;
			}

			if (nRealPixels == 0)
				return 0;

			ULONG const nLimit = nRealPixels*0.01f*fTolerance + 0.5f;

			ULONG n = 255;
			ULONG nSum = 0;
			while ((nSum += aHist[n]) < nLimit && n > 170) --n;

			CColorBaseRGBHLSA cColor;
			cColor.SetRGB(RGB(n, n, n));
			m_wndLSWhite.SetColor(cColor);
			BOOL b;
			OnWhite2DChanged(0, NULL, b);
		}
		catch (...)
		{
		}
		return 0;
	}
	LRESULT OnResetGray(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComBSTR cCFGID_L_GAMMA(CFGID_L_GAMMA);
		M_Config()->ItemValuesSet(1, &(cCFGID_L_GAMMA.m_str), CConfigValue(1.0f, 1.0f, 1.0f));
		UpdateGray();
		return 0;
	}

	void UpdateGray()
	{
		CConfigValue cBlack;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INBLACK), &cBlack);
		CConfigValue cWhite;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INWHITE), &cWhite);
		CConfigValue cGamma;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_GAMMA), &cGamma);
		CColorBaseRGBHLSA tGray = m_wndLSGray.GetColor();
		tGray.SetR(powf((cWhite[0]+cBlack[0])*0.5f, 1.0f/cGamma[0]));
		tGray.SetG(powf((cWhite[1]+cBlack[1])*0.5f, 1.0f/cGamma[1]));
		tGray.SetB(powf((cWhite[2]+cBlack[2])*0.5f, 1.0f/cGamma[2]));

		m_wndLSGray.SetColor(tGray);
		m_wndHueGray.SetColor(tGray);
	}
	void SaveGray()
	{
		CConfigValue cBlack;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INBLACK), &cBlack);
		CConfigValue cWhite;
		M_Config()->ItemValueGet(CComBSTR(CFGID_L_INWHITE), &cWhite);
		CConfigValue cGamma;
		CComBSTR cCFGID_L_GAMMA(CFGID_L_GAMMA);
		M_Config()->ItemValueGet(cCFGID_L_GAMMA, &cGamma);
		CColorBaseRGBHLSA tGray = m_wndLSGray.GetColor();
		float fR = logf((cWhite[0]+cBlack[0])*0.5f)/logf(tGray.GetR());
		float fG = logf((cWhite[1]+cBlack[1])*0.5f)/logf(tGray.GetG());
		float fB = logf((cWhite[2]+cBlack[2])*0.5f)/logf(tGray.GetB());

		M_Config()->ItemValuesSet(1, &(cCFGID_L_GAMMA.m_str), CConfigValue(fR, fG, fB));
	}
	LRESULT OnBlack2DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndHueBlack.SetColor(m_wndLSBlack.GetColor());
			CComBSTR cCFGID_COLOR(CFGID_L_INBLACK);
			CConfigValue cValColor(m_wndLSBlack.GetColor().GetR(), m_wndLSBlack.GetColor().GetG(), m_wndLSBlack.GetColor().GetB());
			M_Config()->ItemValuesSet(1, &(cCFGID_COLOR.m_str), cValColor);
			UpdateGray();
		}
		catch (...)
		{
		}

		return 0;
	}
	LRESULT OnBlack1DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndLSBlack.SetColor(m_wndHueBlack.GetColor());
			CComBSTR cCFGID_COLOR(CFGID_L_INBLACK);
			CConfigValue cValColor(m_wndHueBlack.GetColor().GetR(), m_wndHueBlack.GetColor().GetG(), m_wndHueBlack.GetColor().GetB());
			M_Config()->ItemValuesSet(1, &(cCFGID_COLOR.m_str), cValColor);
			UpdateGray();
		}
		catch (...)
		{
		}

		return 0;
	}
	LRESULT OnWhite2DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndHueWhite.SetColor(m_wndLSWhite.GetColor());
			CComBSTR cCFGID_COLOR(CFGID_L_INWHITE);
			CConfigValue cValColor(m_wndLSWhite.GetColor().GetR(), m_wndLSWhite.GetColor().GetG(), m_wndLSWhite.GetColor().GetB());
			M_Config()->ItemValuesSet(1, &(cCFGID_COLOR.m_str), cValColor);
			UpdateGray();
		}
		catch (...)
		{
		}

		return 0;
	}
	LRESULT OnWhite1DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndLSWhite.SetColor(m_wndHueWhite.GetColor());
			CComBSTR cCFGID_COLOR(CFGID_L_INWHITE);
			CConfigValue cValColor(m_wndHueWhite.GetColor().GetR(), m_wndHueWhite.GetColor().GetG(), m_wndHueWhite.GetColor().GetB());
			M_Config()->ItemValuesSet(1, &(cCFGID_COLOR.m_str), cValColor);
			UpdateGray();
		}
		catch (...)
		{
		}

		return 0;
	}
	LRESULT OnGray2DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndHueGray.SetColor(m_wndLSGray.GetColor());
			SaveGray();
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnGray1DChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			m_wndLSGray.SetColor(m_wndHueGray.GetColor());
			SaveGray();
		}
		catch (...)
		{
		}

		return 0;
	}

	void ExtraConfigNotify()
	{
		//if (m_wndAreaHue.m_hWnd)
		{
			//CConfigValue cValColor;
			//M_Config()->ItemValueGet(CComBSTR(CFGID_COLOR), &cValColor);
			//if (m_wndAreaHue.GetRGB() != cValColor.operator LONG())
			//	m_wndAreaHue.SetRGBA(cValColor.operator LONG(), 255);
		}
	}

private:
	enum EChannel
	{
		EChR = 0,
		EChG,
		EChB,
		EChH,
		EChL,
		EChS,
		EChA,
		EChCnt
	};
	class CColorBaseRGBHLSA
	{
	public:
		CColorBaseRGBHLSA() : m_bValidRGB(true), m_bValidHLS(true)
		{
			m_aColor[EChR] = m_aColor[EChG] = m_aColor[EChB] = m_aColor[EChH] = m_aColor[EChL] = m_aColor[EChS] = 0;
			m_aColor[EChA] = 1.0f;
		}

		void SetAll(float const* a_pAll7) { CopyMemory(m_aColor, a_pAll7, sizeof m_aColor); m_bValidRGB = m_bValidHLS = true; }
		void SetH(float a_fVal) { SyncHLS(); m_aColor[EChH] = a_fVal; m_bValidRGB = false; }
		void SetL(float a_fVal) { SyncHLS(); m_aColor[EChL] = a_fVal; m_bValidRGB = false; }
		void SetS(float a_fVal) { SyncHLS(); m_aColor[EChS] = a_fVal; m_bValidRGB = false; }
		void SetR(float a_fVal) { SyncRGB(); m_aColor[EChR] = a_fVal; m_bValidHLS = false; }
		void SetG(float a_fVal) { SyncRGB(); m_aColor[EChG] = a_fVal; m_bValidHLS = false; }
		void SetB(float a_fVal) { SyncRGB(); m_aColor[EChB] = a_fVal; m_bValidHLS = false; }
		void SetCh(EChannel a_eCh, float a_fVal) { if (a_eCh < EChH) {SyncRGB(); m_bValidHLS = false;} else if (a_eCh < EChA) {SyncHLS(); m_bValidRGB = false;} m_aColor[a_eCh] = a_fVal; }
		void SetHLS(float a_fH, float a_fL, float a_fS) { m_aColor[EChH] = a_fH; m_aColor[EChL] = a_fL; m_aColor[EChS] = a_fS; m_bValidRGB = false; }
		void SetRGB(float a_fR, float a_fG, float a_fB) { m_aColor[EChR] = a_fR; m_aColor[EChG] = a_fG; m_aColor[EChB] = a_fB; m_bValidHLS = false; }
		void SetRGB(DWORD a_tRGB) { SetRGB(GetRValue(a_tRGB)/255.0f, GetGValue(a_tRGB)/255.0f, GetBValue(a_tRGB)/255.0f); }
		void SetBGR(DWORD a_tBGR) { SetRGB(GetBValue(a_tBGR)/255.0f, GetGValue(a_tBGR)/255.0f, GetRValue(a_tBGR)/255.0f); }
		void SetA(float a_fVal) { m_aColor[EChA] = a_fVal; }

		void GetAll(float* a_pAll7) const { SyncHLS(); SyncRGB(); CopyMemory(a_pAll7, m_aColor, sizeof m_aColor); }
		float GetH() const { SyncHLS(); return m_aColor[EChH]; }
		float GetL() const { SyncHLS(); return m_aColor[EChL]; }
		float GetS() const { SyncHLS(); return m_aColor[EChS]; }
		float GetR() const { SyncRGB(); return m_aColor[EChR]; }
		float GetG() const { SyncRGB(); return m_aColor[EChG]; }
		float GetB() const { SyncRGB(); return m_aColor[EChB]; }
		float GetCh(EChannel a_eCh) const { if (a_eCh < EChH) SyncRGB(); else if (a_eCh < EChA) SyncHLS(); return m_aColor[a_eCh]; }
		void GetHLS(float* a_pH, float* a_pL, float* a_pS) const { SyncHLS(); *a_pH = m_aColor[EChH]; *a_pL = m_aColor[EChL]; *a_pS = m_aColor[EChS]; }
		void GetRGB(float* a_pR, float* a_pG, float* a_pB) const { SyncRGB(); *a_pR = m_aColor[EChR]; *a_pG = m_aColor[EChG]; *a_pB = m_aColor[EChB]; }
		DWORD GetRGB() const { SyncRGB(); return RGB(((int)(m_aColor[EChR]*255.0f+0.5f)), ((int)(m_aColor[EChG]*255.0f+0.5f)), ((int)(m_aColor[EChB]*255.0f+0.5f))); }
		DWORD GetBGR() const { SyncRGB(); return RGB(((int)(m_aColor[EChB]*255.0f+0.5f)), ((int)(m_aColor[EChG]*255.0f+0.5f)), ((int)(m_aColor[EChR]*255.0f+0.5f))); }
		float GetA() const { return m_aColor[EChA]; }

	private:
		void SyncHLS() const
		{
			if (!m_bValidHLS)
			{
				RGB2HLS(m_aColor[EChR], m_aColor[EChG], m_aColor[EChB], m_aColor[EChH], m_aColor[EChL], m_aColor[EChS]);
				m_bValidHLS = true;
			}
		}
		void SyncRGB() const
		{
			if (!m_bValidRGB)
			{
				HLS2RGB(m_aColor[EChH], m_aColor[EChL], m_aColor[EChS], m_aColor[EChR], m_aColor[EChG], m_aColor[EChB]);
				m_bValidRGB = true;
			}
		}
		static float hls_value(float n1, float n2, float h)
		{
			h += 360.0f;
			float hue = h - 360.0f*(int)(h/360.0f);

			if (hue < 60.0f)
				return n1 + ( n2 - n1 ) * hue / 60.0f;
			else if (hue < 180.0f)
				return n2;
			else if (hue < 240.0f)
				return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
			else
				return n1;
		}
		static void HLS2RGB(float h, float l, float s, float& r, float& g, float& b)
		{ // h from <0, 360)
			float m1, m2;
			m2 = l + (l <= 0.5f ? l*s : s - l*s);
			m1 = 2.0f * l - m2;
			if (s == 0.0f)
				r = g = b = l;
			else
			{
				r = hls_value(m1, m2, h+120.0f);
				g = hls_value(m1, m2, h);
				b = hls_value(m1, m2, h-120.0f);
			}
			if (r > 1.0f) r = 1.0f;
			if (g > 1.0f) g = 1.0f;
			if (b > 1.0f) b = 1.0f;
			if (r < 0.0f) r = 0.0f;
			if (g < 0.0f) g = 0.0f;
			if (b < 0.0f) b = 0.0f;
		}
		static void RGB2HLS(float r, float g, float b, float& h, float& l, float& s)
		{
			float bc, gc, rc, rgbmax, rgbmin;

			// Compute luminosity.
			rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
			rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
			l = (rgbmax + rgbmin) * 0.5f;

			// Compute saturation.
			if (rgbmax == rgbmin)
				s = 0.0f;
			else if (l <= 0.5f)
				s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
			else
				s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

			// Compute the hue.
			if (rgbmax == rgbmin)
				h = 0.0f;
			else
			{
				rc = (rgbmax - r) / (rgbmax - rgbmin);
				gc = (rgbmax - g) / (rgbmax - rgbmin);
				bc = (rgbmax - b) / (rgbmax - rgbmin);

				if (r == rgbmax)
					h = bc - gc;
				else if (g == rgbmax)
					h = 2.0f + rc - bc;
				else
					h = 4.0f + gc - rc;

				h *= 60.0f;
				h += 360.0f;
				h = h - 360.0f*(int)(h/360.0f);
			}
		}

	private:
		float mutable m_aColor[EChCnt];
		bool mutable m_bValidRGB;
		bool mutable m_bValidHLS;
	};

	template<class TColorBase>
	struct CColorInterpreterVertH
	{
		static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			a_tColor.SetH(a_fY*360.0f);
		}
		static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fY = a_tColor.GetH()/360.0f;
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
			a_fX = 0.5f;
		}

		static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			TColorBase tCur;
			tCur.SetL(0.5f);
			tCur.SetS(1.0f);

			float y = 0.0f;
			for (int nSizeY = a_nSizeY; nSizeY > 0; --nSizeY, y+=a_fStepY)
			{
				float aBase[3];
				tCur.SetH(y*360.0f);
				tCur.GetRGB(aBase+2, aBase+1, aBase);
				if (aBase[0] <= 0.0f) aBase[0] = 0.0f; else if (aBase[0] >= 1.0f) aBase[0] = 255.0f; else aBase[0] *= 255.0f;
				if (aBase[1] <= 0.0f) aBase[1] = 0.0f; else if (aBase[1] >= 1.0f) aBase[1] = 255.0f; else aBase[1] *= 255.0f;
				if (aBase[2] <= 0.0f) aBase[2] = 0.0f; else if (aBase[2] >= 1.0f) aBase[2] = 255.0f; else aBase[2] *= 255.0f;
				DWORD dw = RGB(aBase[0]+0.5f, aBase[1]+0.5f, aBase[2]+0.5f);
				for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX)
				{
					*a_pDst = dw;
					++a_pDst;
				}
			}
		}

		static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			a_tColor.SetL(0.5f);
			a_tColor.SetS(1.0f);
			XY2Color(a_fX, a_fY, a_tColor);
		}

		static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return false;
		}
		static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			return a1.GetH() != a2.GetH();
		}
	};

	typedef CColorArea<CColorInterpreterVertH, CColorBaseRGBHLSA, COLORAREAHORZHWNDCLASS> CColorAreaVertH;

	template<class TColorBase>
	struct CColorInterpreterBlack
	{
		void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			a_tColor.SetL(a_fX/3.0f);
			a_fY = a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			a_tColor.SetS(a_fY);
		}
		void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = a_tColor.GetL()*3.0f;
			a_fY = a_tColor.GetS();
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			TColorBase tCur = a_tCurrent;
			for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
			{
				tCur.SetS(y);
				int nSizeX = a_nSizeX;
				for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
				{
					tCur.SetL(x/3.0f);
					float aBase[3];
					tCur.GetRGB(aBase+2, aBase+1, aBase);
					if (aBase[0] <= 0.0f) aBase[0] = 0.0f; else if (aBase[0] >= 1.0f) aBase[0] = 255.0f; else aBase[0] *= 255.0f;
					if (aBase[1] <= 0.0f) aBase[1] = 0.0f; else if (aBase[1] >= 1.0f) aBase[1] = 255.0f; else aBase[1] *= 255.0f;
					if (aBase[2] <= 0.0f) aBase[2] = 0.0f; else if (aBase[2] >= 1.0f) aBase[2] = 255.0f; else aBase[2] *= 255.0f;
					*(a_pDst++) = RGB(aBase[0]+0.5f, aBase[1]+0.5f, aBase[2]+0.5f);
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return a1.GetH() != a2.GetH();
		}
		bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetS() != a2.GetS() || a1.GetL() != a2.GetL())
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}
	};

	typedef CColorArea<CColorInterpreterBlack, CColorBaseRGBHLSA, COLORAREABLACKWNDCLASS> CColorAreaBlack;

	template<class TColorBase>
	struct CColorInterpreterWhite
	{
		void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			a_tColor.SetL((2.0f+a_fX)/3.0f);
			a_fY = a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			a_tColor.SetS(a_fY);
		}
		void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = a_tColor.GetL()*3.0f-2.0f;
			a_fY = a_tColor.GetS();
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			TColorBase tCur = a_tCurrent;
			for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
			{
				tCur.SetS(y);
				int nSizeX = a_nSizeX;
				for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
				{
					tCur.SetL((2.0f+x)/3.0f);
					float aBase[3];
					tCur.GetRGB(aBase+2, aBase+1, aBase);
					if (aBase[0] <= 0.0f) aBase[0] = 0.0f; else if (aBase[0] >= 1.0f) aBase[0] = 255.0f; else aBase[0] *= 255.0f;
					if (aBase[1] <= 0.0f) aBase[1] = 0.0f; else if (aBase[1] >= 1.0f) aBase[1] = 255.0f; else aBase[1] *= 255.0f;
					if (aBase[2] <= 0.0f) aBase[2] = 0.0f; else if (aBase[2] >= 1.0f) aBase[2] = 255.0f; else aBase[2] *= 255.0f;
					*(a_pDst++) = RGB(aBase[0]+0.5f, aBase[1]+0.5f, aBase[2]+0.5f);
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return a1.GetH() != a2.GetH();
		}
		bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetS() != a2.GetS() || a1.GetL() != a2.GetL())
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}
	};

	typedef CColorArea<CColorInterpreterWhite, CColorBaseRGBHLSA, COLORAREAWHITEWNDCLASS> CColorAreaWhite;

	template<class TColorBase>
	struct CColorInterpreterGray
	{
		void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			a_tColor.SetL((0.5f+a_fX)/2.0f);
			a_fY = a_fY; if (a_fY < 0.0f) a_fY = 0.0f; else if (a_fY > 1.0f) a_fY = 1.0f;
			a_tColor.SetS(a_fY);
		}
		void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = a_tColor.GetL()*2.0f-0.5f;
			a_fY = a_tColor.GetS();
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			if (a_fY <= 0.0f) a_fY = 0.0f; else if (a_fY >= 1.0f) a_fY = 1.0f;
		}

		void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			TColorBase tCur = a_tCurrent;
			for (float y = 0.0f; a_nSizeY > 0; y+=a_fStepY, --a_nSizeY)
			{
				tCur.SetS(y);
				int nSizeX = a_nSizeX;
				for (float x = 0.0f; nSizeX > 0; x+=a_fStepX, --nSizeX)
				{
					tCur.SetL((0.5f+x)/2.0f);
					float aBase[3];
					tCur.GetRGB(aBase+2, aBase+1, aBase);
					if (aBase[0] <= 0.0f) aBase[0] = 0.0f; else if (aBase[0] >= 1.0f) aBase[0] = 255.0f; else aBase[0] *= 255.0f;
					if (aBase[1] <= 0.0f) aBase[1] = 0.0f; else if (aBase[1] >= 1.0f) aBase[1] = 255.0f; else aBase[1] *= 255.0f;
					if (aBase[2] <= 0.0f) aBase[2] = 0.0f; else if (aBase[2] >= 1.0f) aBase[2] = 255.0f; else aBase[2] *= 255.0f;
					*(a_pDst++) = RGB(aBase[0]+0.5f, aBase[1]+0.5f, aBase[2]+0.5f);
				}
			}
		}

		void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return a1.GetH() != a2.GetH();
		}
		bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetS() != a2.GetS() || a1.GetL() != a2.GetL())
				return true;
			float fR1, fG1, fB1, fR2, fG2, fB2;
			a1.GetRGB(&fR1, &fG1, &fB1);
			a2.GetRGB(&fR2, &fG2, &fB2);
			return fR1 != fR2 || fG1 != fG2 || fB1!= fB2;
		}
	};

	typedef CColorArea<CColorInterpreterGray, CColorBaseRGBHLSA, COLORAREAGRAYWNDCLASS> CColorAreaGray;

	// IDocumentForConfig methods
public:
	void GetDocument()
	{
		try
		{
			CComPtr<IDocument> pDoc;
			GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
			m_pImage = NULL;
			m_pDocument = NULL;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&m_pImage));
			if (m_pImage)
			{
				m_pDocument = pDoc;
				if (m_wndTbBlack.m_hWnd)
				{
					m_wndTbBlack.EnableButton(ID_BLACK_AUTO);
					m_wndTbWhite.EnableButton(ID_WHITE_AUTO);
				}
			}
		}
		catch (...)
		{
		}
	}

private:
	CColorAreaBlack m_wndLSBlack;
	CColorAreaWhite m_wndLSWhite;
	CColorAreaGray m_wndLSGray;
	CColorAreaVertH m_wndHueBlack;
	CColorAreaVertH m_wndHueGray;
	CColorAreaVertH m_wndHueWhite;
	CToolBarCtrl m_wndTbBlack;
	CToolBarCtrl m_wndTbGray;
	CToolBarCtrl m_wndTbWhite;
	CImageList m_cImageList;
	CComPtr<IDocument> m_pDocument;
	CComPtr<IDocumentRasterImage> m_pImage;
};


// CDocumentOperationRasterImageLevels

STDMETHODIMP CDocumentOperationRasterImageLevels::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGELEVELS_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRasterImageLevels::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Levels[0405]Úrovně";
		//if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		//CConfigValue cType;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_RN_METHOD), &cType);
		//if (cType.operator LONG() == CFGVAL_RNM_BLUR9OF25)
		//	pszName = L"[0409]Selective blur[0405]Výběrové rozmazání";
		//else
		//	pszName = L"[0409]Blur uniform areas[0405]Rozmazat souvislé oblasti";
		//*a_ppName = new CMultiLanguageString(pszName);
		//return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HICON CDocumentOperationRasterImageLevels::GetDefaultIcon(ULONG a_nSize)
{
	static IRPolyPoint const bar1[] = { {16, 96}, {80, 96}, {80, 225}, {16, 225} };
	static IRPolyPoint const bar2[] = { {96, 48}, {160, 48}, {160, 225}, {96, 225} };
	static IRPolyPoint const bar3[] = { {176, 127}, {240, 127}, {240, 225}, {176, 225} };
	static IRGridItem const gridX[] = { {0, 16}, {1, 80}, {2, 96}, {1, 160}, {2, 176}, {1, 240} };
	static IRGridItem const gridY[] = { {0, 96}, {0, 127}, {0, 160}, {0, 225} };
	static IRCanvas const canvas = {16, 16, 240, 240, itemsof(gridX), itemsof(gridY), gridX, gridY};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	IRTarget target(0.94f, 0, 0.5f);
	cRenderer(&canvas, itemsof(bar1), bar1, pSI->GetMaterial(ESMScheme1Color2), target);
	cRenderer(&canvas, itemsof(bar2), bar2, pSI->GetMaterial(ESMScheme1Color1), target);
	cRenderer(&canvas, itemsof(bar3), bar3, pSI->GetMaterial(ESMInterior), target);
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRasterImageLevels::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_L_INBLACK), _SharedStringTable.GetStringAuto(IDS_CFGID_L_INBLACK_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_L_INBLACK_DESC), CConfigValue(0.0f, 0.0f, 0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_L_INWHITE), _SharedStringTable.GetStringAuto(IDS_CFGID_L_INWHITE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_L_INWHITE_DESC), CConfigValue(1.0f, 1.0f, 1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_L_GAMMA), _SharedStringTable.GetStringAuto(IDS_CFGID_L_GAMMA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_L_GAMMA_DESC), CConfigValue(1.0f, 1.0f, 1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_L_OUTBLACK), _SharedStringTable.GetStringAuto(IDS_CFGID_L_OUTBLACK_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_L_OUTBLACK_DESC), CConfigValue(0.0f, 0.0f, 0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_L_OUTWHITE), _SharedStringTable.GetStringAuto(IDS_CFGID_L_OUTWHITE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_L_OUTWHITE_DESC), CConfigValue(1.0f, 1.0f, 1.0f), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageLevels, CConfigGUILevels>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageLevels::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, 1, aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void NormalizeRGB(BYTE& a_bR, BYTE& a_bG, BYTE& a_bB);

#include "../RWOperationImageRaster/PixelLevelTask.h"

struct CLUTOp
{
	BYTE const* pLUTR;
	BYTE const* pLUTG;
	BYTE const* pLUTB;
	LONGLONG* pTotalA;
	static inline TPixelChannel ProcessPixel(TPixelChannel tS, BYTE const* pLUTR, BYTE const* pLUTG, BYTE const* pLUTB)
	{
		TPixelChannel t;
		t.n = pLUTR[tS.n&0xff]|(ULONG(pLUTG[(tS.n>>8)&0xff])<<8)|(ULONG(pLUTB[(tS.n>>16)&0xff])<<16)|(tS.n&0xff000000);
		return t;
	}
	void Process(TPixelChannel* pD, TPixelChannel const* pS, size_t const s, size_t const n)
	{
		BYTE const* pLUTR = this->pLUTR;
		BYTE const* pLUTG = this->pLUTG;
		BYTE const* pLUTB = this->pLUTB;
		ULONGLONG totalA = 0;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			TPixelChannel tS = *pS;
			DWORD nA = tS.n&0xff000000;
			totalA += nA;

			*pD = ProcessPixel(tS, pLUTR, pLUTG, pLUTB);
		}
		totalA>>=24;
		InterlockedAdd64(pTotalA, totalA);
	}
};

STDMETHODIMP CDocumentOperationRasterImageLevels::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		CConfigValue cInBlack;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_L_INBLACK), &cInBlack);
		CConfigValue cInWhite;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_L_INWHITE), &cInWhite);
		CConfigValue cGamma;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_L_GAMMA), &cGamma);
		CConfigValue cOutBlack;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_L_OUTBLACK), &cOutBlack);
		CConfigValue cOutWhite;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_L_OUTWHITE), &cOutWhite);

		BYTE aLUTR[256];
		BYTE aLUTG[256];
		BYTE aLUTB[256];
		for (ULONG i = 0; i < 256; ++i)
		{
			float a = i/255.0f;
			float f[3];
			for (ULONG j = 0; j < 3; ++j)
			{
				float s = (a-cInBlack[j])/(cInWhite[j]-cInBlack[j]);
				f[j] = cOutBlack[j] + (cOutWhite[j]-cOutBlack[j])*(s < 0 ? -powf(-s, cGamma[j]) : powf(s, cGamma[j]));
			}
			aLUTR[i] = f[0] <= 0.0f ? 0 : (f[0] >= 1.0f ? 255 : BYTE(f[0]*255.0f+0.5f));
			aLUTG[i] = f[1] <= 0.0f ? 0 : (f[1] >= 1.0f ? 255 : BYTE(f[1]*255.0f+0.5f));
			aLUTB[i] = f[2] <= 0.0f ? 0 : (f[2] >= 1.0f ? 255 : BYTE(f[2]*255.0f+0.5f));
		}

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL);
		TRasterImageRect tR = {tOrigin, {tOrigin.nX+tSize.nX, tOrigin.nY+tSize.nY}};
		TPixelChannel  tDefault;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		CLUTOp cOp;
		cOp.pLUTR = aLUTR;
		cOp.pLUTG = aLUTG;
		cOp.pLUTB = aLUTB;
		ULONGLONG totalA = 0;
		cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
		tDefault = CLUTOp::ProcessPixel(tDefault, aLUTR, aLUTG, aLUTB);

		CAutoPixelBuffer cBuf(pRI, tSize);

		{
			CComPtr<IThreadPool> pThPool;
			if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
				RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

			CComObjectStackEx<CPixelLevelTask<CLUTOp> > cXform;
			cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		return cBuf.Replace(tR.tTL, &tR.tTL, &tSize, &totalA, tDefault);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
