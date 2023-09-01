// DocumentCreatorCaptureDesktop.cpp : Implementation of CDocumentCreatorCaptureDesktop

#include "stdafx.h"
#include "DocumentCreatorCaptureDesktop.h"
#include "DocumentLayeredImage.h"
#include <XPGUI.h>

#include <MultiLanguageString.h>
#include <IconRenderer.h>


// CDocumentCreatorCaptureDesktop

class CCaptureDesktopWnd : public CWindowImpl<CCaptureDesktopWnd>
{
public:
	DECLARE_WND_CLASS_EX(_T("CaptureDesktop"), 0, -1) // no background brush

	static bool Capture(CAutoVectorPtr<TPixelChannel>& a_cData, LONG& a_nSizeX, LONG& a_nSizeY)
	{
		CCaptureDesktopWnd wnd;
		wnd.Create(NULL, 0, _T("Capture Desktop"), WS_POPUP|WS_VISIBLE|WS_MAXIMIZE, WS_EX_TRANSPARENT|WS_EX_TOPMOST);
		RECT rc;
		wnd.GetWindowRect(&rc);
		a_nSizeX = rc.right-rc.left;
		a_nSizeY = rc.bottom-rc.top;
		if (!a_cData.Allocate(a_nSizeX*a_nSizeY))
		{
			wnd.DestroyWindow();
			return false;
		}
		HDC hDC = wnd.GetDC();
		HDC hDCBmp = CreateCompatibleDC(hDC);
		HBITMAP hBmp = CreateCompatibleBitmap(hDC, a_nSizeX, a_nSizeY);
		HGDIOBJ hOldBmp = SelectObject(hDCBmp, hBmp);
		BitBlt(hDCBmp, 0, 0, a_nSizeX, a_nSizeY, hDC, 0, 0, XPGUI::IsVista() ? SRCCOPY : SRCCOPY|CAPTUREBLT);
		wnd.ReleaseDC(hDC);
		wnd.DestroyWindow();
		SelectObject(hDCBmp, hOldBmp);
		BITMAPINFO tBI;
		ZeroMemory(&tBI, sizeof tBI);
		tBI.bmiHeader.biSize = sizeof tBI.bmiHeader;
		tBI.bmiHeader.biWidth = a_nSizeX;
		tBI.bmiHeader.biHeight = -a_nSizeY;
		tBI.bmiHeader.biPlanes = 1;
		tBI.bmiHeader.biBitCount = 32;
		tBI.bmiHeader.biCompression = BI_RGB;
		tBI.bmiHeader.biSizeImage = a_nSizeX*a_nSizeY*4;
		tBI.bmiHeader.biXPelsPerMeter = 0;
		tBI.bmiHeader.biYPelsPerMeter = 0;
		tBI.bmiHeader.biClrUsed = 0;
		tBI.bmiHeader.biClrImportant = 0;
		GetDIBits(hDCBmp, hBmp, 0, a_nSizeY, a_cData.m_p, &tBI, DIB_RGB_COLORS);
		DeleteObject(hBmp);
		DeleteDC(hDCBmp);
		return true;
	}

	BEGIN_MSG_MAP(CCaptureDesktopWnd)
	END_MSG_MAP()
};


STDMETHODIMP CDocumentCreatorCaptureDesktop::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvořit");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>

//static OLECHAR const CFGID_U3DPARAMETERS[] = L"Parameters";
//static OLECHAR const CFGID_U3DROOTMODULE[] = L"Root";

class ATL_NO_VTABLE CConfigGUICaptureDesktop : 
	public CCustomConfigResourcelessWndImpl<CConfigGUICaptureDesktop>
{
public:
	BEGIN_DIALOG_EX(0, 0, 156, 20, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Capture desktop[0405]Zachytit plochu"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]This wizard creates a new image from the current content of your desktop. This window will disappear for a moment while capturing is performed.[0405]Tento průvodce vytvoří nový obrázek z aktuálního obsahu pracovní plochy. Toto okno během zachytávání na moment zmizí."), IDC_STATIC, 0, 24, 156, 48, WS_VISIBLE, 0)
		//CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 80, 41, 139, 1, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUICaptureDesktop)
		//MESSAGE_HANDLER(WM_SIZE, OnSize)
		//CHAIN_MSG_MAP(CDialogResize<CConfigGUITranslatorDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUICaptureDesktop>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//COMMAND_HANDLER(IDC_ROOTFCC, CBN_SELCHANGE, OnRootFCCChange)
		//COMMAND_HANDLER(IDC_ROOTFCC, CBN_DROPDOWN, OnRootFCCDropDown)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICaptureDesktop)
		//CONFIGITEM_CHECKBOX(IDC_PARAMETRIC, CFGID_U3DPARAMETERS)
		//CONFIGITEM_CONTEXTHELP(IDC_ROOTFCC, CFGID_U3DROOTMODULE)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfUnderline = 1;
		lf.lfHeight *= 2;
		GetDlgItem(IDC_TITLE).SetFont(m_font.CreateFontIndirect(&lf));

		//m_wndRootFCC = GetDlgItem(IDC_ROOTFCC);

		return 1;  // Let the system set the focus
	}

private:
	CFont m_font;
};

STDMETHODIMP CDocumentCreatorCaptureDesktop::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		//pCfg->ItemInsSimple(CComBSTR(CFGID_U3DPARAMETERS), CMultiLanguageString::GetAuto(L"[0409]Parameterizable[0405]Parametrizovatelný"), CMultiLanguageString::GetAuto(L"[0409]Allows defining parameters and using expressions instead of constants for higher re-usability of 3D objects.[0405]Umožní definovat parametry a používat výrazy místo konstant pro opakované využití 3D modelu."), CConfigValue(true), NULL, 0, NULL);
		//pCfg->ItemInsSimple(CComBSTR(CFGID_U3DROOTMODULE), CMultiLanguageString::GetAuto(L"[0409]Root module[0405]Kořenový modul"), CMultiLanguageString::GetAuto(L"[0409]Parameterizable[0405]Parametrizovatelný"), CConfigValue(0L), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentCreatorCaptureDesktop, CConfigGUICaptureDesktop>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentCreatorCaptureDesktop::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CWindow wndFrame = a_hParentWnd;
		while (wndFrame.m_hWnd && wndFrame.GetStyle() & WS_CHILD)
			wndFrame = wndFrame.GetParent();
		if (wndFrame.m_hWnd)
		{
			wndFrame.ShowWindow(SW_HIDE);
			Sleep(500); // give the system (XP) time to redraw the screen...
		}

		CAutoVectorPtr<TPixelChannel> cData;
		LONG nSizeX = 0;
		LONG nSizeY = 0;
		if (!CCaptureDesktopWnd::Capture(cData, nSizeX, nSizeY))
		{
			if (wndFrame.m_hWnd)
				wndFrame.ShowWindow(SW_SHOW);
			return E_FAIL;
		}

		if (wndFrame.m_hWnd)
		{
			wndFrame.ShowWindow(SW_SHOW);
			wndFrame.RedrawWindow();
		}

		TPixelChannel* const pEnd = cData.m_p+nSizeX*nSizeY;
		for (TPixelChannel* p = cData.m_p; p < pEnd; ++p)
			p->bA = 0xff;

		TImageSize tSize = {nSizeX, nSizeY};
		TImageTile t = {EICIRGBA, {0, 0}, tSize, {1, tSize.nX}, tSize.nX*tSize.nY, cData};
		return a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 0.0f, &t);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentCreatorCaptureDesktop::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

		static IRPolyPoint const skybox[] = { {8, 20}, {248, 20}, {248, 140}, {8, 140} };
		static IRPathPoint const clouds1[] =
		{
			{72, 108, 4, -10, -4, 10},
			{108, 100, 18, -14, -10, -4},
			{137, 97, 15, -11, 6, -10},
			{175, 88, 26, -2, -7, -8},
			{222, 95, 17, -11, -8, -30},
			{247, 97, 0, 0, 0, 0},
			{247, 122, 0, 0, 0, 0},
		};
		static IRPathPoint const clouds2[] =
		{
			{114, 106, 6, -3, -6, 3},
			{147, 107, 6, -2, -9, -3},
			{189, 113, 6, -4, -10, -7},
			{227, 118, 9, -1, -9, -6},
			{244, 133, -10, 1, 5, -16},
			{113, 116, 0, 0, 0, 0},
		};
		static IRPathPoint const clouds3[] =
		{
			{161, 86, 2.5, -6.5, 10, 0},
			{179, 75, 7, -8, -15.125, -1.25},
			{202, 72, 13, -1, -5, -6},
			{214, 85, -9, -2, 5, -5},
			{183, 95, -1, -5, 10, -5},
		};
		static IRPathPoint const hillsBase[] =
		{
			{8, 116, 63, -29, 0, 0},
			{248, 132, 0, 0, -68, -1},
			{248, 179, 0, 0, 0, 0},
			{8, 180, 0, 0, 0, 0},
		};
		static IRPathPoint const hillsShadow[] =
		{
			{128, 146, 21, 8, -16.3201, -6.21719},
			{8, 165, 0, 0, 74, -3},
			{8, 121, 19, 20, 0, 0},
		};
		static IRPathPoint const hillsDistant[] =
		{
			{248, 124, 0, 0, -10, 1},
			{248, 142, -57, -6, 0, 0},
			{67, 106, 23, -1, 40, 7},
		};
		static IRPathPoint const hillsClose[] =
		{
			{8, 163, 19, -8, 0, 0},
			{248, 150, 0, 0, -31, 8},
			{248, 185, 0, 0, 0, 0},
			{8, 185, 0, 0, 0, 0},
		};
		static IRPathPoint const cloud1[] =
		{
			{76, 52, 2.97256, -2.6001, -2.9726, 2.60011},
			{96, 52, -1.26353, 2.15458, 1.26354, -2.15458},
			{97, 64, -7.28174, 0.0730286, 7.28175, -0.0730362},
			{76, 61, 0.891037, 3.41811, -0.891037, -3.41811},
			{65, 67, -3.86362, -0.818016, 3.86362, 0.818016},
			{49, 69, -5.57268, -0.37249, 5.57269, 0.372498},
			{55, 63, 4.68165, -3.04563, -4.68165, 3.04562},
		};
		static IRPathPoint const cloud2[] =
		{
			{51, 60, -3.11865, -11.9634, -6.83622, 1.78208},
			{63, 46, 4.68164, -3.04562, -7.28174, 0.0730324},
			{76, 46, 6.39072, -3.49115, -4.30915, -2.52706},
			{93, 53, -8.9908, 0.518562, 8.9908, -0.518555},
			{68, 57, -2.52705, 4.30916, 2.52707, -4.30915},
			{45, 67, -6.01822, -2.08154, 6.0182, 2.08155},
		};
		static IRFill skyboxMat(0xff6293ef);
		static IRFill cloudShadowMat(0xff99b3f6);
		static IRFill cloudMat(0xffdee8f3);
		static IRFill hillsMat(0xff6c9217);
		static IRFill hillsShadowMat(0xff52711b);
		static IRFill hillsDistMat(0xff5d8d84);
		static IRFill hillsCloseMat(0xff425e0e);
		static IRCanvas const canvasWallpaper = {0, 0, 256, 256, 0, 0, NULL, NULL};
		cRenderer(&canvasWallpaper, itemsof(skybox), skybox, &skyboxMat);
		cRenderer(&canvasWallpaper, itemsof(clouds1), clouds1, &cloudShadowMat);
		cRenderer(&canvasWallpaper, itemsof(clouds2), clouds2, &cloudMat);
		cRenderer(&canvasWallpaper, itemsof(clouds3), clouds3, &cloudMat);
		cRenderer(&canvasWallpaper, itemsof(hillsBase), hillsBase, &hillsMat);
		cRenderer(&canvasWallpaper, itemsof(hillsShadow), hillsShadow, &hillsShadowMat);
		cRenderer(&canvasWallpaper, itemsof(hillsDistant), hillsDistant, &hillsDistMat);
		cRenderer(&canvasWallpaper, itemsof(hillsClose), hillsClose, &hillsCloseMat);
		cRenderer(&canvasWallpaper, itemsof(cloud1), cloud1, &cloudShadowMat);
		cRenderer(&canvasWallpaper, itemsof(cloud2), cloud2, &cloudMat);

		static IRPathPoint const frameOuter[] =
		{
			{0, 18, 0, -3.31371, 0, 0},
			{6, 12, 0, 0, -3.31371, 0},
			{250, 12, 3.31371, 0, 0, 0},
			{256, 18, 0, 0, 0, -3.31371},
			{256, 203, 0, 3.31371, 0, 0},
			{250, 209, 0, 0, 3.31371, 0},
			{6, 209, -3.31371, 0, 0, 0},
			{0, 203, 0, 0, 0, 3.31371},
		};
		static IRPathPoint const frameInner[] =
		{	
			{16, 28, 0, 0, 0, 0},
			{240, 28, 0, 0, 0, 0},
			{240, 176, 0, 0, 0, 0},
			{16, 176, 0, 0, 0, 0},
		};
		static IRPath const frame[] = { {itemsof(frameOuter), frameOuter}, {itemsof(frameInner), frameInner} };
		static IRPolyPoint const button[] = { {124, 189}, {132, 189}, {132, 195}, {124, 195} };
		static IRPathPoint const stand[] =
		{
			{75, 250, 16, -13.0909, 0, 0},
			{110, 189, 0, 0, 0, 29.091},
			{145, 189, 0, 29.091, 0, 0},
			{183, 250, 0, 0, -16, -13.0909},
		};
		static IRGridItem const gridX[] = {{0, 0}, {0, 16}, {0, 240}, {0, 256}};
		static IRGridItem const gridY[] = {{0, 12}, {0, 28}, {0, 176}, {0, 209}, {0, 250}};
		static IRCanvas const canvasDisplay = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

		IROutlinedFill plasticMat(pSI->GetMaterial(ESMAltBackground, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		cRenderer(&canvasDisplay, itemsof(stand), stand, &plasticMat);
		cRenderer(&canvasDisplay, itemsof(frame), frame, &plasticMat);
		cRenderer(&canvasDisplay, itemsof(button), button, pSI->GetMaterial(ESMContrast));

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

