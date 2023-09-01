// DesignerViewColorSwatch.h : Declaration of the CDesignerViewColorSwatch

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include <math.h>

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

// CDesignerViewColorSwatch

class ATL_NO_VTABLE CDesignerViewColorSwatch : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewColorSwatch>,
	public CScrollImpl<CDesignerViewColorSwatch>,
//	public CContextHelpDlg<CDesignerViewColorSwatch>,
	public CThemeImpl<CDesignerViewColorSwatch>,
	public CDesignerViewWndImpl<CDesignerViewColorSwatch, IDesignerView>,
	public CObserverImpl<CDesignerViewColorSwatch, ISharedStateObserver, TSharedStateChange>,
	public IColorSwatchView,
	public IDesignerViewStatusBar
{
public:
	CDesignerViewColorSwatch() : m_bEnableUpdates(true), m_bTrackingMouse(false),
		m_nLastColor(-1)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CDesignerViewColorSwatch()
	{
		try { ColorsToCofig(); } catch (...) {}
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
	}

	DECLARE_WND_CLASS_EX(_T("ColorSwatchClass"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	void Init(ISharedStateManager* a_pFrame, IConfig* a_pConfig, LPCOLESTR a_pszSyncGrp1, LPCOLESTR a_pszSyncGrp2, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_pConfig = a_pConfig;
		if (m_pConfig)
		{
			CConfigValue cSwatches;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CLRSWCH_SWATCHFLAGS), &cSwatches);
			m_nSwatches = cSwatches.operator LONG();
		}
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		pMgr->Config(CLSID_DesignerViewFactoryColorSwatch, &m_pColorsCfg);
		CConfigValue cClrs;
		m_pColorsCfg->ItemValueGet(CComBSTR(L"Colors"), &cClrs);
		if (cClrs.TypeGet() == ECVTString && cClrs.operator BSTR() && cClrs.operator BSTR()[0])
		{
			wchar_t const* psz = cClrs.operator BSTR();
			wchar_t const* pszEnd = psz+SysStringLen(cClrs.operator BSTR());
			TSwatchColor tColor;
			tColor.eSpace = ESCSRGB;
			tColor.bstrName = NULL;
			tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
			tColor.fA = 1.0f;
			float* aDsts[10];
			int nDsts = 0;
			while (psz < pszEnd)
			{
				switch (*psz)
				{
				case L'R':
					aDsts[nDsts++] = &tColor.f1;
					break;
				case L'G':
					aDsts[nDsts++] = &tColor.f2;
					break;
				case L'B':
					aDsts[nDsts++] = &tColor.f3;
					break;
				case L'A':
					aDsts[nDsts++] = &tColor.fA;
					break;
				case L'H':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f1;
					break;
				case L'L':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f2;
					break;
				case L'S':
					tColor.eSpace = ESCSHLS;
					aDsts[nDsts++] = &tColor.f3;
					break;
				case L',':
					break;
				case L'|':
					{
						wchar_t const* psz2 = psz+1;
						while (psz2 < pszEnd && *psz != L'\r' && *psz != L'\n')
							++psz2;
						if (psz2 > psz)
						{
							tColor.bstrName = SysAllocStringLen(psz, psz2-psz);
							psz = psz2;
						}
						else
						{
							break;
						}
					}
				case L'\r':
				case L'\n':
					if (psz+1 < pszEnd && psz[1] == *psz^('\r'^'\n'))
						++psz;
					m_aColors.push_back(tColor);
					nDsts = 0;
					tColor.eSpace = ESCSRGB;
					tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
					tColor.fA = 1.0f;
					tColor.bstrName = NULL;
					break;
				case L':':
					if (psz+1 < pszEnd)
						++psz;
				default:
					{
						wchar_t* pszNext = const_cast<wchar_t*>(psz);
						float f = wcstod(psz, &pszNext);
						if (pszNext > psz)
						{
							for (int i = 0; i < nDsts; ++i)
								*(aDsts[i]) = f;
							psz = pszNext-1;
						}
						nDsts = 0;
					}
					break;
				}
				++psz;
			}
			m_aColors.push_back(tColor);
		}
		// hues
		{
			TSwatchColor* pC = m_aColorsHues;
			for (int i = 0; i < 16; ++i, ++pC)
			{
				pC->f1 = pC->f2 = pC->f3 = i/15.0f;
				pC->bstrName = NULL;
				pC->eSpace = ESCSRGB;
				pC->f4 = 0.0f;
				pC->fA = 1.0f;
			}
			for (int h = 0; h < 12; ++h)
			{
				for (int i = 0; i < 16; ++i, ++pC)
				{
					HLS2RGB(h*360.0f/12.0f, (i+i+1)/32.0f, 1.0f, pC->f1, pC->f2, pC->f3);
					pC->bstrName = NULL;
					pC->eSpace = ESCSRGB;
					pC->f4 = 0.0f;
					pC->fA = 1.0f;
				}
			}
			for (int i = 0; i < 16; ++i, ++pC)
			{
				pC->f1 = pC->f2 = pC->f3 = 1.0f;
				pC->fA = i/15.0f;
				pC->bstrName = NULL;
				pC->eSpace = ESCSRGB;
				pC->f4 = 0.0f;
			}
			for (int i = 0; i < 16; ++i, ++pC)
			{
				pC->f1 = pC->f2 = pC->f3 = 0.0f;
				pC->fA = i/15.0f;
				pC->bstrName = NULL;
				pC->eSpace = ESCSRGB;
				pC->f4 = 0.0f;
			}
		}
		// windows colors
		{
			static const RGBQUAD aPal4bit[] =
			{
				{0x00, 0x00, 0x00, 0x00},
				{0x00, 0x00, 0x80, 0x00},
				{0x00, 0x80, 0x00, 0x00},
				{0x00, 0x80, 0x80, 0x00},
				{0x80, 0x00, 0x00, 0x00},
				{0x80, 0x00, 0x80, 0x00},
				{0x80, 0x80, 0x00, 0x00},
				{0x80, 0x80, 0x80, 0x00},
				{0xc0, 0xc0, 0xc0, 0x00},
				{0x00, 0x00, 0xff, 0x00},
				{0x00, 0xff, 0x00, 0x00},
				{0x00, 0xff, 0xff, 0x00},
				{0xff, 0x00, 0x00, 0x00},
				{0xff, 0x00, 0xff, 0x00},
				{0xff, 0xff, 0x00, 0x00},
				{0xff, 0xff, 0xff, 0x00}
			};
			for (int i = 0; i < 16; ++i)
			{
				m_aColorsWindows[i].f1 = aPal4bit[i].rgbRed/255.0f;
				m_aColorsWindows[i].f2 = aPal4bit[i].rgbGreen/255.0f;
				m_aColorsWindows[i].f3 = aPal4bit[i].rgbBlue/255.0f;
				m_aColorsWindows[i].bstrName = NULL;
				m_aColorsWindows[i].eSpace = ESCSRGB;
				m_aColorsWindows[i].f4 = 0.0f;
				m_aColorsWindows[i].fA = 1.0f;
			}
		}
		// web colors
		{
			TSwatchColor* pC = m_aColorsWebSafe;
			TSwatchColor t;
			t.bstrName = NULL;
			t.eSpace = ESCSRGB;
			t.f4 = 0.0f;
			for (int i = 0; i < 3; ++i)
				for (int k = 0; k < 6; ++k)
					for (int j = 0; j < 2; ++j)
					{
						t.f1 = t.f2 = t.f3 = t.fA = 0.0f;
						*(pC++) = t;
						t.fA = 1.0f;
						for (int l = 0; l < 6; ++l)
						{
							t.f1 = (i+i+j)/5.0f;
							t.f2 = k/5.0f;
							t.f3 = l/5.0f;
							*(pC++) = t;
						}
						t.f1 = t.f2 = t.f3 = t.fA = 0.0f;
						*(pC++) = t;
					}
		}

		m_tLocaleID = a_tLocaleID;

		m_bstrSelGrp1 = a_pszSyncGrp1;
		m_bstrSelGrp2 = a_pszSyncGrp2;
		if (m_bstrSelGrp1.Length() || m_bstrSelGrp2.Length())
		{
			a_pFrame->ObserverIns(ObserverGet(), 0);
		}
		m_pSharedState = a_pFrame;

		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ColorSwatch"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL, /*(a_nStyle&EDVWSBorderMask) == EDVWSNoBorder ?*/ 0 /*: WS_EX_CLIENTEDGE*/) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

BEGIN_COM_MAP(CDesignerViewColorSwatch)
	COM_INTERFACE_ENTRY(IDesignerView)
	//COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	COM_INTERFACE_ENTRY(IColorSwatchView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewColorSwatch)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewColorSwatch>)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	CHAIN_MSG_MAP(CScrollImpl<CDesignerViewColorSwatch>)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	//NOTIFY_HANDLER(ID_COLORAREAHS, CColorAreaHS::CAN_COLOR_CHANGED, OnColorAreaHSChanged)
	//NOTIFY_HANDLER(ID_COLORAREALA, CColorAreaLA::CAN_COLOR_CHANGED, OnColorAreaLAChanged)
//	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
//	CHAIN_MSG_MAP(CContextHelpDlg<CDesignerViewColorSwatch>)
END_MSG_MAP()

//BEGIN_CTXHELP_MAP(CDesignerViewColorSwatch)
//	CTXHELP_CONTROL_RESOURCE(ID_COLORAREAHS, IDS_HELP_COLORAREAHS)
//	CTXHELP_CONTROL_RESOURCE(ID_COLORAREALA, IDS_HELP_COLORAREALA)
//END_CTXHELP_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pSharedState != NULL)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
	}

	template<typename TIter>
	void PaintBoxes(CDCHandle a_cDC, COLORREF clr1, COLORREF clr2, TIter i, TIter iEnd, LONG a_nOffsetY)
	{
		RECT rc;
		int x = 0;
		int y = a_nOffsetY;
		while (i != iEnd)
		{
			float fR = i->f1;
			float fG = i->f2;
			float fB = i->f3;
			if (i->eSpace == ESCSHLS)
				HLS2RGB(i->f1, i->f2, i->f3, fR, fG, fB);
			if (fR < 0.0f) fR = 0.0f; else if (fR > 1.0f) fR = 1.0f;
			if (fG < 0.0f) fG = 0.0f; else if (fG > 1.0f) fG = 1.0f;
			if (fB < 0.0f) fB = 0.0f; else if (fB > 1.0f) fB = 1.0f;
			COLORREF clr = RGB(ULONG(fR*255.0f+0.5f), ULONG(fG*255.0f+0.5f), ULONG(fB*255.0f+0.5f));
			if (i->fA >= 1.0f)
			{
				HBRUSH hBr = CreateSolidBrush(clr);
				rc.left = x;
				rc.top = y;
				rc.right = x+m_nSquareSize;
				rc.bottom = y+m_nSquareSize;
				a_cDC.FillRect(&rc, hBr);
				DeleteObject(hBr);
			}
			else
			{
				int bA = i->fA < 0.0f ? 0 : ULONG(i->fA*255.0f+0.5f);
				int bB = GetBValue(clr);
				int bG = GetGValue(clr);
				int bR = GetRValue(clr);
				COLORREF c1 = RGB((bA*bR+(255-bA)*GetRValue(clr1))/255, (bA*bG+(255-bA)*GetGValue(clr1))/255, (bA*bB+(255-bA)*GetBValue(clr1))/255);
				COLORREF c2 = RGB((bA*bR+(255-bA)*GetRValue(clr2))/255, (bA*bG+(255-bA)*GetGValue(clr2))/255, (bA*bB+(255-bA)*GetBValue(clr2))/255);
				HBRUSH hBr1 = CreateSolidBrush(c1);
				HBRUSH hBr2 = CreateSolidBrush(c2);
				rc.left = x;
				rc.top = y;
				rc.right = x+(m_nSquareSize>>1);
				rc.bottom = y+(m_nSquareSize>>1);
				a_cDC.FillRect(&rc, hBr1);
				rc.top = rc.bottom;
				rc.bottom = y+m_nSquareSize;
				a_cDC.FillRect(&rc, hBr2);
				rc.left = rc.right;
				rc.right = x+m_nSquareSize;
				a_cDC.FillRect(&rc, hBr1);
				rc.bottom = rc.top;
				rc.top = y;
				a_cDC.FillRect(&rc, hBr2);
				DeleteObject(hBr1);
				DeleteObject(hBr2);
			}
			a_cDC.SetPixel(x, y, 0x666666);
			a_cDC.SetPixel(x+1, y, 0x404040);
			a_cDC.SetPixel(x, y+1, 0x404040);
			a_cDC.SetPixel(x+m_nSquareSize-1, y, 0x9e9e9e);
			a_cDC.SetPixel(x+m_nSquareSize-2, y, 0x9e9e9e);
			a_cDC.SetPixel(x+m_nSquareSize-1, y+1, 0x9e9e9e);
			a_cDC.SetPixel(x, y+m_nSquareSize-1, 0x9e9e9e);
			a_cDC.SetPixel(x+1, y+m_nSquareSize-1, 0x9e9e9e);
			a_cDC.SetPixel(x, y+m_nSquareSize-2, 0x9e9e9e);
			a_cDC.SetPixel(x+m_nSquareSize-1, y+m_nSquareSize-1, 0xcccccc);
			a_cDC.SetPixel(x+m_nSquareSize-2, y+m_nSquareSize-1, 0xffffff);
			a_cDC.SetPixel(x+m_nSquareSize-1, y+m_nSquareSize-2, 0xffffff);
			x += m_nSquareSize;
			if (x >= int(m_nPerLine*m_nSquareSize))
			{
				x = 0;
				y += m_nSquareSize;
			}
			++i;
		}
	}
	void DoPaint(CDCHandle a_cDC)
	{
		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);
		if (m_nY1Hues < m_nY2Hues)
			PaintBoxes(a_cDC, clr1, clr2, m_aColorsHues, m_aColorsHues+itemsof(m_aColorsHues), m_nY1Hues);
		if (m_nY1Windows < m_nY2Windows)
			PaintBoxes(a_cDC, clr1, clr2, m_aColorsWindows, m_aColorsWindows+itemsof(m_aColorsWindows), m_nY1Windows);
		if (m_nY1WebSafe < m_nY2WebSafe)
			PaintBoxes(a_cDC, clr1, clr2, m_aColorsWebSafe, m_aColorsWebSafe+itemsof(m_aColorsWebSafe), m_nY1WebSafe);
		if (m_nY1Custom < m_nY2Custom)
			PaintBoxes(a_cDC, clr1, clr2, m_aColors.begin(), m_aColors.end(), m_nY1Custom);
	}

	// message handlers
protected:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetScrollExtendedStyle(SCRL_DISABLENOSCROLLV|SCRL_ERASEBACKGROUND);
		HDC hDC = GetDC();
		int nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hDC);
		m_nSquareSize = (nDPI+6)/8+2;
		m_wndTips.Create(m_hWnd, NULL, _T("ColorTip"), WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);

		m_tToolTip.cbSize = TTTOOLINFO_V1_SIZE;//sizeof m_tToolTip;
		m_tToolTip.uFlags = TTF_IDISHWND|TTF_TRACK|TTF_ABSOLUTE;
		m_tToolTip.hwnd = m_hWnd;
		m_tToolTip.hinst = _pModule->get_m_hInst();
		m_tToolTip.lpszText = _T("N/A");
		m_tToolTip.uId = (UINT_PTR)m_hWnd;
		GetClientRect(&m_tToolTip.rect);
		m_wndTips.AddTool(&m_tToolTip);
		m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		m_wndTips.SetMaxTipWidth(800);

		return 0;
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		a_bHandled = FALSE;
		if (m_wndTips.IsWindow())
			m_wndTips.DestroyWindow();
		return 0;
	}
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bTrackingMouse)
		{
			// request notification when the mouse leaves
			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0 };
			TrackMouseEvent(&tme);
			m_bTrackingMouse = true;
		}

		int newX = GET_X_LPARAM(a_lParam);
		int newY = GET_Y_LPARAM(a_lParam)+m_ptOffset.y;

		ESwatchType eType = ESTInvalid;
		int nColor = -1;
		TSwatchColor tColor;
		if (newX < LONG(m_nSquareSize*m_nPerLine))
		{
			if (newY >= m_nY1Hues && newY < m_nY2Hues)
			{
				newY -= m_nY1Hues;
				eType = ESTHues;
				nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
				tColor = m_aColorsHues[nColor];
			}
			else if (newY >= m_nY1Windows && newY < m_nY2Windows)
			{
				newY -= m_nY1Windows;
				eType = ESTWindows;
				nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
				tColor = m_aColorsWindows[nColor];
			}
			else if (newY >= m_nY1WebSafe && newY < m_nY2WebSafe)
			{
				newY -= m_nY1WebSafe;
				eType = ESTWebSafe;
				nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
				tColor = m_aColorsWebSafe[nColor];
			}
			else if (newY >= m_nY1Custom && newY < m_nY2Custom)
			{
				newY -= m_nY1Custom;
				eType = ESTCustom;
				nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
				if (ULONG(nColor) < m_aColors.size())
					tColor = m_aColors[nColor];
				else
					nColor = -1;
			}
		}

		if (nColor >= 0)
		{
			if (m_nLastColor != nColor || m_eLastType != eType)
			{
				if (m_nLastColor < 0)
				{
					// activate the ToolTip
					m_wndTips.TrackActivate(&m_tToolTip, TRUE);
				}
				TCHAR szTmp[512];
				CConfigValue cRng;
				m_pColorsCfg->ItemValueGet(CComBSTR(L"Factor"), &cRng);
				CConfigValue cDec;
				m_pColorsCfg->ItemValueGet(CComBSTR(L"Decimal"), &cDec);
				float const fMul1 = cRng.operator float()*powf(10, cDec.operator LONG());
				float const fMul2 = powf(10, -cDec.operator LONG());
				float const fHMul1 = 10.0f;
				float const fHMul2 = 0.1f;
				float const f[4] =
				{
					tColor.eSpace == ESCSRGB ? int(tColor.f1*fMul1+0.5f)*fMul2 : int(tColor.f1*fHMul1+0.5f)*fHMul2,
					int(tColor.f2*fMul1+0.5f)*fMul2,
					int(tColor.f3*fMul1+0.5f)*fMul2,
					int(tColor.fA*fMul1+0.5f)*fMul2,
				};
				if (tColor.bstrName && SysStringLen(tColor.bstrName) < 420)
				{
					CW2T str(tColor.bstrName);
					_stprintf_s(szTmp, itemsof(szTmp), tColor.eSpace == ESCSRGB ? _T("%s\r\nR: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("%s\r\nH: %g\r\nL: %g\r\nS: %g\r\nA: %g"), str.operator LPTSTR(), f[0], f[1], f[2], f[3]);
				}
				else
				{
					_stprintf_s(szTmp, itemsof(szTmp), tColor.eSpace == ESCSRGB ? _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("H: %g\r\nL: %g\r\nS: %g\r\nA: %g"), f[0], f[1], f[2], f[3]);
				}

				m_tToolTip.lpszText = szTmp;
				m_wndTips.SetToolInfo(&m_tToolTip);

				POINT pt = { (nColor%m_nPerLine+1)*m_nSquareSize+1, (nColor/m_nPerLine+1)*m_nSquareSize+1 }; 
				ClientToScreen(&pt);
				pt.y -= newY-GET_Y_LPARAM(a_lParam);
				m_wndTips.TrackPosition(pt.x, pt.y);
				m_nLastColor = nColor;
				m_eLastType = eType;
			}
		}
		else if (m_nLastColor >= 0)
		{
			m_nLastColor = -1;
			m_eLastType = ESTInvalid;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}

		return 0;
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_nLastColor >= 0)
		{
			m_nLastColor = -1;
			m_eLastType = ESTInvalid;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
		m_bTrackingMouse = false;
		return 0;
	}
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW/* && pHelpInfo->iCtrlId >= 0*/)
		{
			TCHAR szBuffer[1024] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_HELP_COLORSWATCH, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
			RECT rcItem;
			GetWindowRect(&rcItem);
			HH_POPUP hhp;
			hhp.cbStruct = sizeof(hhp);
			hhp.hinst = _pModule->get_m_hInst();
			hhp.idString = 0;
			hhp.pszText = szBuffer;
			hhp.pt.x = (rcItem.right+rcItem.left)>>1;
			hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
			hhp.clrForeground = 0xffffffff;
			hhp.clrBackground = 0xffffffff;
			hhp.rcMargins.left = -1;
			hhp.rcMargins.top = -1;
			hhp.rcMargins.right = -1;
			hhp.rcMargins.bottom = -1;
			hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
			HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
			return 0;
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		UpdateScrollSize(GET_X_LPARAM(a_lParam));
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int newX = GET_X_LPARAM(a_lParam);
		int newY = GET_Y_LPARAM(a_lParam)+m_ptOffset.y;

		if (newX >= int(m_nSquareSize*m_nPerLine))
			return 0;

		ESwatchType eType = ESTInvalid;
		int nColor = -1;
		TSwatchColor tColor;
		if (newY >= m_nY1Hues && newY < m_nY2Hues)
		{
			newY -= m_nY1Hues;
			eType = ESTHues;
			nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
			tColor = m_aColorsHues[nColor];
		}
		else if (newY >= m_nY1Windows && newY < m_nY2Windows)
		{
			newY -= m_nY1Windows;
			eType = ESTWindows;
			nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
			tColor = m_aColorsWindows[nColor];
		}
		else if (newY >= m_nY1WebSafe && newY < m_nY2WebSafe)
		{
			newY -= m_nY1WebSafe;
			eType = ESTWebSafe;
			nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
			tColor = m_aColorsWebSafe[nColor];
		}
		else if (newY >= m_nY1Custom)
		{
			newY -= m_nY1Custom;
			eType = ESTCustom;
			nColor = (newX/m_nSquareSize)+(newY/m_nSquareSize)*m_nPerLine;
			if (ULONG(nColor) < m_aColors.size())
				tColor = m_aColors[nColor];
			else
				nColor = -1;
		}

		RECT rc;
		GetClientRect(&rc);
		int nPerLine = GetPerLineCount(rc.right);
		int nLines = (m_aColors.size()+nPerLine-1)/nPerLine;
		int nLastLine = m_aColors.size()-(nLines-1)*nPerLine;
		int x = GET_X_LPARAM(a_lParam)/m_nSquareSize;
		int y = (GET_Y_LPARAM(a_lParam)+m_ptOffset.y)/m_nSquareSize;
		if (nColor < 0 && eType == ESTCustom)
		{
			CComPtr<ISharedStateColor> pState;
			m_pSharedState->StateGet((a_wParam&MK_SHIFT) ? m_bstrSelGrp2 : m_bstrSelGrp1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pState));
			float aColor[4];
			if (pState == NULL || FAILED(pState->RGBAGet(aColor)))
				return 0;
			TSwatchColor t = {NULL, ESCSRGB, aColor[0], aColor[1], aColor[2], 0.0f, aColor[3]};
			m_aColors.push_back(t);
			Invalidate();
			UpdateScrollSize();
			return 0;
		}

		CComPtr<ISharedStateColor> pPrev;
		m_pSharedState->StateGet((a_wParam&MK_SHIFT) ? m_bstrSelGrp2 : m_bstrSelGrp1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pPrev));
		float aColor[4] = {tColor.f1, tColor.f2, tColor.f3, tColor.fA};
		CComPtr<ISharedStateColor> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateColor));
		if (S_OK == (tColor.eSpace == ESCSRGB ? pTmp->RGBASet(aColor, pPrev) : pTmp->HLSASet(aColor, pPrev)))
			m_pSharedState->StateSet((a_wParam&MK_SHIFT) ? m_bstrSelGrp2 : m_bstrSelGrp1, pTmp);
		return 0;
	}
	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int nLines = (m_aColors.size()+m_nPerLine-1)/m_nPerLine;
		int nLastLine = m_aColors.size()-(nLines-1)*m_nPerLine;
		int x = GET_X_LPARAM(a_lParam)/m_nSquareSize;
		int y = (GET_Y_LPARAM(a_lParam)-m_nY1Custom+m_ptOffset.y)/m_nSquareSize;
		if (y < 0 || x >= m_nPerLine || y >= nLines || (y == nLines-1 && x >= nLastLine))
			return 0;
		int nClr = x+y*m_nPerLine;
		ATLASSERT(ULONG(nClr) < m_aColors.size());
		BSTR bstr = m_aColors[nClr].bstrName;
		m_aColors.erase(m_aColors.begin()+nClr);
		if (bstr) SysFreeString(bstr);
		++m_nLastColor;
		Invalidate();
		UpdateScrollSize();
		POINT tPt;
		GetCursorPos(&tPt);
		ScreenToClient(&tPt);
		BOOL b;
		OnMouseMove(WM_MOUSEMOVE, 0, MAKELPARAM(tPt.x, tPt.y), b);
		return 0;
	}
//	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return S_FALSE;
	}

	// IDesignerView methods
public:

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		try
		{
			if (m_bTrackingMouse && m_nLastColor >= 0)
			{
				WCHAR szBuffer[1024] = L"";
				Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_HELP_COLORSWATCH, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
				a_pStatusBar->SimpleModeSet(CComBSTR(szBuffer));
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IColorSwatchView methods
public:
	STDMETHOD(Count)(ULONG* a_pSize)
	{
		try
		{
			*a_pSize = m_aColors.size();
			return S_OK;
		}
		catch (...)
		{
			return a_pSize ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Value)(ULONG a_nIndex, TSwatchColor* a_pColor)
	{
		try
		{
			if (a_nIndex < m_aColors.size())
			{
				a_pColor->eSpace = ESCSRGB;
				*a_pColor = m_aColors[a_nIndex];
				if (a_pColor->bstrName)
					a_pColor->bstrName = SysAllocString(a_pColor->bstrName);
				return S_OK;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		catch (...)
		{
			return a_pColor ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Delete)(ULONG a_nFirst, ULONG a_nLast)
	{
		try
		{
			if (a_nFirst < a_nLast && a_nFirst < m_aColors.size())
			{
				if (a_nLast > m_aColors.size())
					a_nLast = m_aColors.size();
				for (ULONG i = a_nFirst; i < a_nLast; ++i)
					if (m_aColors[i].bstrName)
						SysFreeString(m_aColors[i].bstrName);
				m_aColors.erase(m_aColors.begin()+a_nFirst, m_aColors.begin()+a_nLast);
				Invalidate();
				UpdateScrollSize();
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Insert)(ULONG a_nBefore, TSwatchColor const* a_pColor)
	{
		try
		{
			if (a_nBefore > m_aColors.size())
				a_nBefore = m_aColors.size();
			TSwatchColor t = *a_pColor;
			CComBSTR bstr(t.bstrName);
			t.bstrName = bstr;
			m_aColors.insert(m_aColors.begin()+a_nBefore, t);
			bstr.Detach();
			Invalidate();
			UpdateScrollSize();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Change)(ULONG a_nIndex, TSwatchColor const* a_pColor)
	{
		try
		{
			if (a_nIndex >= m_aColors.size())
				return E_RW_INDEXOUTOFRANGE;
			TSwatchColor t = *a_pColor;
			if (m_aColors[a_nIndex].bstrName)
				SysFreeString(m_aColors[a_nIndex].bstrName);
			m_aColors[a_nIndex] = t;
			if (m_aColors[a_nIndex].bstrName)
				m_aColors[a_nIndex].bstrName = SysAllocString(m_aColors[a_nIndex].bstrName);
			Invalidate();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(SwatchEnable)(ESwatchType a_eType)
	{
		if (m_nSwatches&a_eType)
			return S_FALSE;
		m_nSwatches |= a_eType;
		CComBSTR bstr(CFGID_CLRSWCH_SWATCHFLAGS);
		m_pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(m_nSwatches)));
		Invalidate();
		UpdateScrollSize();
		return S_OK;
	}
	STDMETHOD(SwatchDisable)(ESwatchType a_eType)
	{
		if (m_nSwatches&a_eType)
		{
			m_nSwatches &= 0xffffffff^a_eType;
			CComBSTR bstr(CFGID_CLRSWCH_SWATCHFLAGS);
			m_pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(m_nSwatches)));
			Invalidate();
			UpdateScrollSize();
			return S_OK;
		}
		return S_FALSE;
	}
	STDMETHOD(SwatchesSet)(ULONG a_nTypes)
	{
		if (m_nSwatches == a_nTypes)
			return S_FALSE;
		m_nSwatches = a_nTypes;
		CComBSTR bstr(CFGID_CLRSWCH_SWATCHFLAGS);
		m_pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(m_nSwatches)));
		Invalidate();
		UpdateScrollSize();
		return S_OK;
	}
	STDMETHOD(SwatchesGet)(ULONG* a_pTypes)
	{
		try
		{
			*a_pTypes = m_nSwatches;
			return S_OK;
		}
		catch (...)
		{
			return a_pTypes ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Defaults)(TSwatchColor* a_pColor1, TSwatchColor* a_pColor2)
	{
		try
		{
			a_pColor1->eSpace = ESCSRGB;
			a_pColor1->bstrName = NULL;
			a_pColor1->f1 = a_pColor1->f2 = a_pColor1->f3 = a_pColor1->f4 = 0.0f;
			a_pColor1->fA = 1.0f;
			CComPtr<ISharedStateColor> pColor1;
			m_pSharedState->StateGet(m_bstrSelGrp1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pColor1));
			float f[4];
			if (pColor1 && SUCCEEDED(pColor1->RGBAGet(f)))
			{
				a_pColor1->f1 = f[0];
				a_pColor1->f2 = f[1];
				a_pColor1->f3 = f[2];
				a_pColor1->fA = f[3];
			}

			a_pColor2->eSpace = ESCSRGB;
			a_pColor2->bstrName = NULL;
			a_pColor2->f1 = a_pColor2->f2 = a_pColor2->f3 = a_pColor2->f4 = 0.0f;
			a_pColor2->fA = 0.0f;
			CComPtr<ISharedStateColor> pColor2;
			m_pSharedState->StateGet(m_bstrSelGrp2, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pColor2));
			if (pColor2 && SUCCEEDED(pColor2->RGBAGet(f)))
			{
				a_pColor2->f1 = f[0];
				a_pColor2->f2 = f[1];
				a_pColor2->f3 = f[2];
				a_pColor2->fA = f[3];
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pColor1 && a_pColor2 ? E_UNEXPECTED : E_POINTER;
		}
	}

public:
	LCID m_tLocaleID;

private:
	int GetPerLineCount(int a_nSizeX)
	{
		int n = max(1, a_nSizeX/m_nSquareSize);
		for (int i = 0; (n&(1<<i)) != n; ++i)
		{
			n &= ~(1<<i);
		}
		return n;
	}
	void UpdateScrollSize(LONG a_nSize = -1)
	{
		if (a_nSize < 0)
		{
			RECT rc;
			GetClientRect(&rc);
			a_nSize = rc.right;
		}
		m_nPerLine = GetPerLineCount(a_nSize);
		bool bSep = false;
		ULONG nHeight = 0;
		if (m_nSwatches&ESTHues)
		{
			m_nY1Hues = nHeight;
			nHeight = ((15*16+m_nPerLine-1)/m_nPerLine)*m_nSquareSize;
			bSep = true;
			m_nY2Hues = nHeight;
		}
		else
		{
			m_nY1Hues = m_nY2Hues = nHeight;
		}
		if (m_nSwatches&ESTWindows)
		{
			if (bSep) nHeight += 2;
			m_nY1Windows = nHeight;
			nHeight += ((16+m_nPerLine-1)/m_nPerLine)*m_nSquareSize;
			bSep = true;
			m_nY2Windows = nHeight;
		}
		else
		{
			m_nY1Windows = m_nY2Windows = nHeight;
		}
		if (m_nSwatches&ESTWebSafe)
		{
			if (bSep) nHeight += 2;
			m_nY1WebSafe = nHeight;
			nHeight += ((18*16+m_nPerLine-1)/m_nPerLine)*m_nSquareSize;
			bSep = true;
			m_nY2WebSafe = nHeight;
		}
		else
		{
			m_nY1WebSafe = m_nY2WebSafe = nHeight;
		}
		if (m_nSwatches&ESTCustom && !m_aColors.empty())
		{
			if (bSep) nHeight += 2;
			m_nY1Custom = nHeight;
			nHeight += ((m_aColors.size()+m_nPerLine-1)/m_nPerLine)*m_nSquareSize;
			bSep = true;
			m_nY2Custom = nHeight;
		}
		else
		{
			m_nY1Custom = m_nY2Custom = nHeight;
		}
		SetScrollSize(m_nPerLine*m_nSquareSize, nHeight);
	}
	static bool CompareColors(TSwatchColor const& a_clr1, TSwatchColor const& a_clr2)
	{
		if (a_clr1.eSpace != a_clr2.eSpace)
			return false;
		if ((a_clr1.bstrName && !a_clr1.bstrName) || (!a_clr1.bstrName && a_clr1.bstrName) ||
			(a_clr1.bstrName && a_clr1.bstrName && wcscmp(a_clr1.bstrName, a_clr1.bstrName)))
			return false;
		if (fabsf(a_clr1.f1-a_clr2.f1) > 1e3f || fabsf(a_clr1.f2-a_clr2.f2) > 1e3f ||
			fabsf(a_clr1.f3-a_clr2.f3) > 1e3f || fabsf(a_clr1.fA-a_clr2.fA) > 1e3f)
			return false;
		return true;
	}
	void ColorsToCofig()
	{
		std::vector<wchar_t> cData;
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			TSwatchColor tColor = *i;
			wchar_t const* pChnls = tColor.eSpace == ESCSRGB ? L"RGBA" : L"HLSA";
			float pVals[4] = {tColor.f1, tColor.f2, tColor.f3, tColor.fA};
			static float const pDefs[4] = {0.0f, 0.0f, 0.0f, 1.0f};
			wchar_t szTmp[128] = L""; // RG:0.5,B:1    G:1|Green
			wchar_t* psz = szTmp;
			for (int j = 0; j < 4; ++j)
			{
				if (pVals[j] != pDefs[j])
				{
					if (psz != szTmp)
						*(psz++) = L',';
					*(psz++) = pChnls[j];
					for (int k = j+1; k < 4; ++k)
					{
						if (pVals[k] != pDefs[k] && pVals[k] == pVals[j])
						{
							*(psz++) = pChnls[k];
							pVals[k] = pDefs[k];
						}
					}
					*(psz++) = L':';
					swprintf(psz, L"%g", pVals[j]);
					psz += wcslen(psz);
				}
			}
			if (tColor.bstrName && tColor.bstrName[0])
			{
				*(psz++) = L'|';
				for (wchar_t* psz2 = szTmp; psz2 < psz; ++psz2)
					cData.push_back(*psz2);
				for (wchar_t* psz2 = tColor.bstrName; *psz2; ++psz2)
					cData.push_back(*psz2);
				cData.push_back(L'\r');
				cData.push_back(L'\n');
			}
			else
			{
				if (i+1 != m_aColors.end())
				{
					*(psz++) = L'\r';
					*(psz++) = L'\n';
				}
				for (wchar_t* psz2 = szTmp; psz2 < psz; ++psz2)
					cData.push_back(*psz2);
			}
		}
		cData.push_back(L'\0');
		CComBSTR bstr(L"Colors");
		m_pColorsCfg->ItemValuesSet(1, &(bstr.m_str), CConfigValue(&(cData[0])));
	}

	void Data2GUI();
	void SendUpdate();

	typedef std::vector<TSwatchColor> CColors;

private:
	CComPtr<ISharedStateManager> m_pSharedState;
	CComPtr<IConfig> m_pConfig;
	bool m_bEnableUpdates;
	CComBSTR m_bstrSelGrp1;
	CComBSTR m_bstrSelGrp2;
	CColors m_aColors;
	TSwatchColor m_aColorsHues[15*16];
	TSwatchColor m_aColorsWindows[16];
	TSwatchColor m_aColorsWebSafe[18*16];
	ULONG m_nSquareSize;
	CToolTipCtrl m_wndTips;
	TTTOOLINFO m_tToolTip;
	bool m_bTrackingMouse;
	int m_nLastColor;
	ESwatchType m_eLastType;
	CComPtr<IConfig> m_pColorsCfg;
	ULONG m_nSwatches;
	LONG m_nPerLine;
	LONG m_nY1Hues;
	LONG m_nY2Hues;
	LONG m_nY1Windows;
	LONG m_nY2Windows;
	LONG m_nY1WebSafe;
	LONG m_nY2WebSafe;
	LONG m_nY1Custom;
	LONG m_nY2Custom;
};

