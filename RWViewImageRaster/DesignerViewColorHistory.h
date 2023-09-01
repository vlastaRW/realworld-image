// DesignerViewColorHistory.h : Declaration of the CDesignerViewColorHistory

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include <math.h>
#include <GammaCorrection.h>

void HLS2RGB(float h, float l, float s, float& r, float& g, float& b);

// CDesignerViewColorHistory

class ATL_NO_VTABLE CDesignerViewColorHistory : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewColorHistory>,
//	public CContextHelpDlg<CDesignerViewColorHistory>,
	public CThemeImpl<CDesignerViewColorHistory>,
	public CDesignerViewWndImpl<CDesignerViewColorHistory, IDesignerView>,
	public CObserverImpl<CDesignerViewColorHistory, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewColorHistory, IConfigObserver, IUnknown*>,
	public CObserverImpl<CDesignerViewColorHistory, IImageObserver, TImageChange>//,
	//public IDesignerViewStatusBar
{
public:
	CDesignerViewColorHistory() : m_bEnableUpdates(true), m_bTrackingMouse(false),
		m_nLastColor(HTEmptySpace)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_tActiveColor.eSpace = ESCSRGB;
		m_tActiveColor.bstrName = NULL;
		m_tActiveColor.f1 = m_tActiveColor.f2 = m_tActiveColor.f3 = m_tActiveColor.f4 = 0.0f;
		m_tActiveColor.fA = 1.0f;
	}
	~CDesignerViewColorHistory()
	{
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
	}
	enum { MAXLASTCOLORS = 24, HTEmptySpace = -2, HTActiveColor = -1, };

	DECLARE_WND_CLASS_EX(_T("ColorHistoryClass"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	void Init(IDocument* a_pDoc, ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGrp1, LPCOLESTR a_pszSyncGrp2, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		if (a_pDoc)
		{
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pDocImage));
			if (m_pDocImage)
				m_pDocImage->ObserverIns(CObserverImpl<CDesignerViewColorHistory, IImageObserver, TImageChange>::ObserverGet(), 0);
		}
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		pMgr->Config(CLSID_DesignerViewFactoryColorSwatch, &m_pColorsCfg);
		m_pColorsCfg->ObserverIns(CObserverImpl<CDesignerViewColorHistory, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		ParseColors();

		m_tLocaleID = a_tLocaleID;

		m_bstrSelGrp1 = a_pszSyncGrp1;
		m_bstrSelGrp2 = a_pszSyncGrp2;
		if (m_bstrSelGrp1.Length() || m_bstrSelGrp2.Length())
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewColorHistory, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		}
		m_pSharedState = a_pFrame;

		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ColorHistory"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, /*(a_nStyle&EDVWSBorderMask) == EDVWSNoBorder ?*/ 0 /*: WS_EX_CLIENTEDGE*/) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

BEGIN_COM_MAP(CDesignerViewColorHistory)
	COM_INTERFACE_ENTRY(IDesignerView)
	//COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewColorHistory)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewColorHistory>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
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
//	CHAIN_MSG_MAP(CContextHelpDlg<CDesignerViewColorHistory>)
END_MSG_MAP()

//BEGIN_CTXHELP_MAP(CDesignerViewColorHistory)
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
			m_pSharedState->ObserverDel(CObserverImpl<CDesignerViewColorHistory, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pDocImage)
			m_pDocImage->ObserverDel(CObserverImpl<CDesignerViewColorHistory, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_pColorsCfg)
			m_pColorsCfg->ObserverDel(CObserverImpl<CDesignerViewColorHistory, IConfigObserver, IUnknown*>::ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		try
		{
			if (m_pSharedState && m_bstrSelGrp1 == a_tParams.bstrName && a_tParams.pState)
			{
				CComQIPtr<ISharedStateColor> pState(a_tParams.pState);
				if (pState)
				{
					float aColor[4];
					pState->RGBAGet(aColor);
					TSwatchColor t;
					t.eSpace = ESCSRGB;
					t.bstrName = NULL;
					t.f1 = aColor[0];
					t.f2 = aColor[1];
					t.f3 = aColor[2];
					t.f4 = 0.0f;
					t.fA = aColor[3];
					if (!CompareColors(t, m_tActiveColor))
					{
						m_tActiveColor = t;
						Invalidate(FALSE);
					}
				}
			}
		}
		catch (...)
		{
		}
	}

	void OwnerNotify(TCookie, TImageChange a_tChange)
	{
		try
		{
			if (m_pSharedState && m_bstrSelGrp1 && m_bstrSelGrp1[0])
			{
				AddColor(&m_tActiveColor);
			}
		}
		catch (...)
		{
		}
	}

	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (ParseColors())
				Invalidate(FALSE);
		}
		catch (...)
		{
		}
	}

	static void DrawSquare(COLORREF clr1, COLORREF clr2, CDCHandle a_cDC, RECT const& a_rc, TSwatchColor const a_clr)
	{
		float fR = a_clr.f1;
		float fG = a_clr.f2;
		float fB = a_clr.f3;
		if (a_clr.eSpace == ESCSHLS)
			HLS2RGB(a_clr.f1, a_clr.f2, a_clr.f3, fR, fG, fB);
		if (a_clr.fA >= 1.0f)
		{
			BYTE nR = CGammaTables::ToSRGB(fR);
			BYTE nG = CGammaTables::ToSRGB(fG);
			BYTE nB = CGammaTables::ToSRGB(fB);
			COLORREF clr = RGB(nR, nG, nB);
			HBRUSH hBr = CreateSolidBrush(clr);
			a_cDC.FillRect(&a_rc, hBr);
			DeleteObject(hBr);
		}
		else
		{
			float const ad = a_clr.fA;
			float const ai = 1.0f-a_clr.fA;
			BYTE nR1 = CGammaTables::ToSRGB(ad*fR+ai*CGammaTables::FromSRGB(GetRValue(clr1)));
			BYTE nG1 = CGammaTables::ToSRGB(ad*fG+ai*CGammaTables::FromSRGB(GetGValue(clr1)));
			BYTE nB1 = CGammaTables::ToSRGB(ad*fB+ai*CGammaTables::FromSRGB(GetBValue(clr1)));
			BYTE nR2 = CGammaTables::ToSRGB(ad*fR+ai*CGammaTables::FromSRGB(GetRValue(clr2)));
			BYTE nG2 = CGammaTables::ToSRGB(ad*fG+ai*CGammaTables::FromSRGB(GetGValue(clr2)));
			BYTE nB2 = CGammaTables::ToSRGB(ad*fB+ai*CGammaTables::FromSRGB(GetBValue(clr2)));
			COLORREF c1 = RGB(nR1, nG1, nB1);
			COLORREF c2 = RGB(nR2, nG2, nB2);
			//int bA = a_clr.fA < 0.0f ? 0 : ULONG(a_clr.fA*255.0f+0.5f);
			//int bB = GetBValue(clr);
			//int bG = GetGValue(clr);
			//int bR = GetRValue(clr);
			//COLORREF c1 = RGB((bA*bR+(255-bA)*GetRValue(clr1))/255, (bA*bG+(255-bA)*GetGValue(clr1))/255, (bA*bB+(255-bA)*GetBValue(clr1))/255);
			//COLORREF c2 = RGB((bA*bR+(255-bA)*GetRValue(clr2))/255, (bA*bG+(255-bA)*GetGValue(clr2))/255, (bA*bB+(255-bA)*GetBValue(clr2))/255);
			HBRUSH hBr1 = CreateSolidBrush(c1);
			HBRUSH hBr2 = CreateSolidBrush(c2);
			RECT rc;
			rc.left = a_rc.left;
			rc.top = a_rc.top;
			rc.right = (a_rc.left+a_rc.right)>>1;
			rc.bottom = (a_rc.top+a_rc.bottom)>>1;
			a_cDC.FillRect(&rc, hBr1);
			rc.top = rc.bottom;
			rc.bottom = a_rc.bottom;
			a_cDC.FillRect(&rc, hBr2);
			rc.left = rc.right;
			rc.right = a_rc.right;
			a_cDC.FillRect(&rc, hBr1);
			rc.bottom = rc.top;
			rc.top = a_rc.top;
			a_cDC.FillRect(&rc, hBr2);
			DeleteObject(hBr1);
			DeleteObject(hBr2);
		}
	}
	void DoPaint(CDCHandle a_cDC)
	{
		RECT rcWnd;
		GetClientRect(&rcWnd);
		HBRUSH hBgBr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG, (WPARAM)a_cDC.m_hDC);
		if (hBgBr)
		{
			a_cDC.FillRect(&rcWnd, hBgBr);
		}
		else
		{
			a_cDC.FillSolidRect(&rcWnd, GetSysColor(COLOR_WINDOW));
		}
		RECT rc;
		rc.left = rc.top = m_nGapSize;
		rc.bottom = m_nGapSize+m_nSquareSize;
		COLORREF clr1 = GetSysColor(COLOR_3DLIGHT);
		COLORREF clr2 = GetSysColor(COLOR_3DSHADOW);
		if (!m_aColors.empty() && !CompareColors(m_tActiveColor, m_aColors[0]))
		{
			RECT rcSq = {rc.left, rc.top, rc.left+m_nSquareSize, rc.bottom};
			DrawSquare(clr1, clr2, a_cDC, rcSq, m_tActiveColor);
			if (m_nLastColor == HTActiveColor)
				a_cDC.Draw3dRect(rc.left-2, rc.top-2, m_nSquareSize+4, m_nSquareSize+4, clr2, clr2);
			rc.left += m_nSquareSize+m_nSmallGapSize;
			a_cDC.FillSolidRect(rc.left, rc.top-(m_nGapSize>>1), 1, rc.bottom-rc.top+m_nGapSize, GetSysColor(COLOR_3DSHADOW));
			rc.left += 1+m_nSmallGapSize;
		}
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
		{
			if (rc.left >= rcWnd.right)
				break;

			rc.right = rc.left+m_nSquareSize;
			DrawSquare(clr1, clr2, a_cDC, rc, *i);
			if (m_nLastColor == i-m_aColors.begin())
				a_cDC.Draw3dRect(rc.left-2, rc.top-2, m_nSquareSize+4, m_nSquareSize+4, clr2, clr2);
			rc.left += m_nSquareSize+m_nSmallGapSize;
		}
	}

	// message handlers
protected:
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		PAINTSTRUCT ps;
		CDCHandle cDC(BeginPaint(&ps));

		DoPaint(cDC);

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		int nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hDC);
		m_nSquareSize = (nDPI+6)/8;
		m_nGapSize = (nDPI+12)/24;
		m_nSmallGapSize = (nDPI+16)/32;
		m_wndTips.Create(m_hWnd, NULL, _T("ColorTip"), WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, WS_EX_TOPMOST);

		if (m_pSharedState && m_bstrSelGrp1 && *m_bstrSelGrp1)
		{
			CComPtr<ISharedStateColor> pState;
			m_pSharedState->StateGet(m_bstrSelGrp1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pState));
			float aColor[4];
			if (pState && SUCCEEDED(pState->RGBAGet(aColor)))
			{
				m_tActiveColor.f1 = aColor[0];
				m_tActiveColor.f2 = aColor[1];
				m_tActiveColor.f3 = aColor[2];
				m_tActiveColor.fA = aColor[3];
			}
		}

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
		if (m_pColorsCfg)
		{
			m_pColorsCfg->ObserverDel(CObserverImpl<CDesignerViewColorHistory, IConfigObserver, IUnknown*>::ObserverGet(), 0);
			m_pColorsCfg = NULL;
		}
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

		POINT pt = { GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam) };

		UpdateTooltip(pt);

		return 0;
	}
	void UpdateTooltip(POINT pt)
	{
		int nColor = ColorHitTest(pt);

		if (nColor != HTEmptySpace)
		{
			if (m_nLastColor != nColor)
			{
				if (m_nLastColor == HTEmptySpace)
				{
					// activate the ToolTip
					m_wndTips.TrackActivate(&m_tToolTip, TRUE);
				}
				else
				{
					RECT rcSq;
					GetExtSquareRect(m_nLastColor, &rcSq);
					InvalidateRect(&rcSq, FALSE);
				}
				TCHAR szTmp[512];
				if (nColor == HTActiveColor)
				{
					Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_COLOR_CURRENTTOLAST, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
				}
				else
				{
					TSwatchColor const& t = m_aColors[nColor];
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
						t.eSpace == ESCSRGB ? int(t.f1*fMul1+0.5f)*fMul2 : int(t.f1*fHMul1+0.5f)*fHMul2,
						int(t.f2*fMul1+0.5f)*fMul2,
						int(t.f3*fMul1+0.5f)*fMul2,
						int(t.fA*fMul1+0.5f)*fMul2,
					};
					if (t.bstrName && SysStringLen(t.bstrName) < 420)
					{
						CW2T str(t.bstrName);
						_stprintf_s(szTmp, itemsof(szTmp), t.eSpace == ESCSRGB ? _T("%s\r\nR: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("%s\r\nH: %g\r\nL: %g\r\nS: %g\r\nA: %g"), str.operator LPTSTR(), f[0], f[1], f[2], f[3]);
					}
					else
					{
						_stprintf_s(szTmp, itemsof(szTmp), t.eSpace == ESCSRGB ? _T("R: %g\r\nG: %g\r\nB: %g\r\nA: %g") : _T("H: %g\r\nL: %g\r\nS: %g\r\nA: %g"), f[0], f[1], f[2], f[3]);
					}
				}
				m_tToolTip.lpszText = szTmp;
				m_wndTips.SetToolInfo(&m_tToolTip);
				{
					RECT rcSq;
					GetExtSquareRect(nColor, &rcSq);
					InvalidateRect(&rcSq, FALSE);
				}

				POINT pt =
				{
					nColor == HTActiveColor ? m_nSquareSize+m_nGapSize+2 :
					(m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]) ? m_nGapSize+m_nSquareSize+2+m_nSquareSize+m_nSmallGapSize+m_nSmallGapSize+1+(m_nSquareSize+m_nSmallGapSize)*nColor :m_nGapSize+m_nSquareSize+2+(m_nSquareSize+m_nSmallGapSize)*nColor),
					m_nSquareSize+m_nGapSize+2
				}; 
				ClientToScreen(&pt);
				m_wndTips.TrackPosition(pt.x, pt.y);
				m_nLastColor = nColor;
			}
		}
		else if (m_nLastColor != HTEmptySpace)
		{
			RECT rcSq;
			GetExtSquareRect(m_nLastColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_nLastColor = HTEmptySpace;
			m_wndTips.TrackActivate(&m_tToolTip, FALSE);
		}
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_nLastColor != HTEmptySpace)
		{
			RECT rcSq;
			GetExtSquareRect(m_nLastColor, &rcSq);
			InvalidateRect(&rcSq, FALSE);
			m_nLastColor = HTEmptySpace;
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
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_HELP_COLORHISTORY, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
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
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		int nColor = ColorHitTest(pt);
		if (nColor == HTActiveColor)
		{
			AddColor(&m_tActiveColor);
		}
		else if (ULONG(nColor) < m_aColors.size())
		{
			TSwatchColor& clr = m_aColors[nColor];
			CComPtr<ISharedStateColor> pPrev;
			m_pSharedState->StateGet((a_wParam&MK_SHIFT) ? m_bstrSelGrp2 : m_bstrSelGrp1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pPrev));
			float aColor[4] = {clr.f1, clr.f2, clr.f3, clr.fA};
			CComPtr<ISharedStateColor> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateColor));
			if (S_OK == (clr.eSpace == ESCSRGB ? pTmp->RGBASet(aColor, pPrev) : pTmp->HLSASet(aColor, pPrev)))
				m_pSharedState->StateSet((a_wParam&MK_SHIFT) ? m_bstrSelGrp2 : m_bstrSelGrp1, pTmp);
		}
		UpdateTooltip(pt);
		return 0;
	}
	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		//RECT rc;
		//GetClientRect(&rc);
		//int nPerLine = GetPerLineCount(rc.right);
		//int nLines = (m_aColors.size()+nPerLine-1)/nPerLine;
		//int nLastLine = m_aColors.size()-(nLines-1)*nPerLine;
		//int x = GET_X_LPARAM(a_lParam)/m_nSquareSize;
		//int y = (GET_Y_LPARAM(a_lParam)+m_ptOffset.y)/m_nSquareSize;
		//if (x >= nPerLine || y >= nLines || (y == nLines-1 && x >= nLastLine))
		//	return 0;
		//int nClr = x+y*nPerLine;
		//ATLASSERT(nClr < m_aColors.size());
		//BSTR bstr = m_aColors[nClr].bstrName;
		//m_aColors.erase(m_aColors.begin()+nClr);
		//if (bstr) SysFreeString(bstr);
		//if (m_nLastColor >= nClr)
		//	--m_nLastColor;
		//Invalidate();
		//SetScrollSize(nPerLine*m_nSquareSize, ((m_aColors.size()+nPerLine-1)/nPerLine)*m_nSquareSize);
		//POINT tPt;
		//GetCursorPos(&tPt);
		//ScreenToClient(&tPt);
		//BOOL b;
		//OnMouseMove(WM_MOUSEMOVE, 0, MAKELPARAM(tPt.x, tPt.y), b);
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
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		if (a_pSize)
		{
			a_pSize->cx = m_nSquareSize*16+m_nSmallGapSize*15;
			a_pSize->cy = m_nSquareSize+m_nGapSize+m_nGapSize;
		}
		return S_OK;
	}

//	// IDesignerViewStatusBar methods
//public:
//	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
//	{
//		try
//		{
//			if (m_bTrackingMouse && m_nLastColor >= 0)
//			{
//				WCHAR szBuffer[1024] = L"";
//				Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_HELP_COLORSWATCH, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
//				a_pStatusBar->SimpleModeSet(CComBSTR(szBuffer));
//			}
//			return S_OK;
//		}
//		catch (...)
//		{
//			return E_UNEXPECTED;
//		}
//	}

	void AddColor(TSwatchColor const* a_pColor)
	{
		CColors::iterator i;
		for (i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (CompareColors(*i, *a_pColor))
				break;
		bool bChange = false;
		if (i == m_aColors.end())
		{
			// add new color
			TSwatchColor t = *a_pColor;
			CComBSTR bstr(t.bstrName);
			t.bstrName = bstr;
			m_aColors.insert(m_aColors.begin(), t);
			bstr.Detach();
			bChange = true;
		}
		else if (i != m_aColors.begin())
		{
			// already in last colors, move it to top
			TSwatchColor t = *i;
			m_aColors.erase(i);
			m_aColors.insert(m_aColors.begin(), t);
			bChange = true;
		}
		// delete old colors
		while (m_aColors.size() > MAXLASTCOLORS)
		{
			BSTR bstr = m_aColors[m_aColors.size()-1].bstrName;
			if (bstr) SysFreeString(bstr);
			m_aColors.resize(m_aColors.size()-1);
			bChange = true;
		}
		if (bChange)
		{
			ColorsToCofig();
			Invalidate();
		}
	}


public:
	LCID m_tLocaleID;

private:
	int ColorHitTest(POINT a_pt)
	{
		if (a_pt.y < LONG(m_nGapSize) || a_pt.y >= LONG(m_nGapSize+m_nSquareSize) || a_pt.x < LONG(m_nGapSize))
			return HTEmptySpace;
		if (m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]))
		{
			if (a_pt.x < LONG(m_nGapSize+m_nSquareSize))
				return HTActiveColor;
			a_pt.x -= m_nSmallGapSize+m_nSmallGapSize+m_nSquareSize+1;
			if (a_pt.x < LONG(m_nGapSize))
				return HTEmptySpace;
		}
		ULONG const nColor = (a_pt.x-m_nGapSize)/(m_nSquareSize+m_nSmallGapSize);
		ULONG const nReminder = (a_pt.x-m_nGapSize)%(m_nSquareSize+m_nSmallGapSize);
		if (nReminder >= m_nSquareSize || nColor >= m_aColors.size())
			return HTEmptySpace;
		return nColor;
	}
	void GetExtSquareRect(int a_nColor, RECT* a_prc)
	{
		a_prc->top = m_nGapSize-2;
		a_prc->bottom = m_nGapSize+m_nSquareSize+2;
		if (a_nColor == HTActiveColor)
		{
			a_prc->left = m_nGapSize-2;
			a_prc->right = m_nGapSize+m_nSquareSize+2;
		}
		else
		{
			a_prc->left = m_aColors.empty() || !CompareColors(m_tActiveColor, m_aColors[0]) ? m_nSmallGapSize+m_nSmallGapSize+1+m_nSquareSize : 0;
			a_prc->left += m_nGapSize-2+a_nColor*(m_nSmallGapSize+m_nSquareSize);
			a_prc->right = a_prc->left+m_nSquareSize+4;
		}
	}
	static bool CompareColors(TSwatchColor const& a_clr1, TSwatchColor const& a_clr2)
	{
		if (a_clr1.eSpace != a_clr2.eSpace)
			return false;
		if ((a_clr1.bstrName && !a_clr1.bstrName) || (!a_clr1.bstrName && a_clr1.bstrName) ||
			(a_clr1.bstrName && a_clr1.bstrName && wcscmp(a_clr1.bstrName, a_clr1.bstrName)))
			return false;
		if (fabsf(a_clr1.f1-a_clr2.f1) > 1e-3f || fabsf(a_clr1.f2-a_clr2.f2) > 1e-3f ||
			fabsf(a_clr1.f3-a_clr2.f3) > 1e-3f || fabsf(a_clr1.fA-a_clr2.fA) > 1e-3f)
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
		CComBSTR bstr(L"LastColors");
		m_pColorsCfg->ItemValuesSet(1, &(bstr.m_str), CConfigValue(&(cData[0])));
	}
	bool ParseColors()
	{
		CConfigValue cClrs;
		m_pColorsCfg->ItemValueGet(CComBSTR(L"LastColors"), &cClrs);
		if (cClrs == m_cClrs)
			return false;
		for (CColors::const_iterator i = m_aColors.begin(); i != m_aColors.end(); ++i)
			if (i->bstrName)
				SysFreeString(i->bstrName);
		m_aColors.clear();
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
		m_cClrs = cClrs;
		return true;
	}

	void Data2GUI();
	void SendUpdate();

	typedef std::vector<TSwatchColor> CColors;

private:
	CComPtr<IDocumentImage> m_pDocImage;
	CComPtr<ISharedStateManager> m_pSharedState;
	bool m_bEnableUpdates;
	CComBSTR m_bstrSelGrp1;
	CComBSTR m_bstrSelGrp2;
	CColors m_aColors;
	ULONG m_nSquareSize;
	ULONG m_nGapSize;
	ULONG m_nSmallGapSize;
	CToolTipCtrl m_wndTips;
	TTTOOLINFO m_tToolTip;
	bool m_bTrackingMouse;
	int m_nLastColor;
	CComPtr<IConfig> m_pColorsCfg;
	TSwatchColor m_tActiveColor;
	CConfigValue m_cClrs;
};

