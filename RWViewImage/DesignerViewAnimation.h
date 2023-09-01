// DesignerViewAnimation.h : Declaration of the CDesignerViewAnimation

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"
#include "ImageControl.h"
#include <ObserverImpl.h>
#include <RWDocumentAnimation.h>

extern __declspec(selectany) OLECHAR const CFGID_ANIVIEW_BACKGROUNDCOLOR[] = L"BgColor";
extern __declspec(selectany) OLECHAR const CFGID_ANIVIEW_DRAWFRAME[] = L"Frame";


// CDesignerViewAnimation

class ATL_NO_VTABLE CDesignerViewAnimation : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CImageControl<CDesignerViewAnimation>,
	public CThemeImpl<CDesignerViewAnimation>,
	public CDesignerViewWndImpl<CDesignerViewAnimation, IDesignerView>,
	public CObserverImpl<CDesignerViewAnimation, IAnimationObserver, TAnimationChange>
	//public CObserverImpl<CDesignerViewAnimation, IImageObserver, ULONG>,
	//public CObserverImpl<CDesignerViewAnimation, ISharedStateObserver, TSharedStateChange>
{
public:
	CDesignerViewAnimation() : m_bObserving(false), m_bUpdating(false), m_dwTimeOff(0),
		m_dwCurrentFrame(0), m_dwNextFrame(0), m_dwAniLength(0), m_pImgFrame(NULL), m_bFrame(false)
	{
		m_tLastIS.nX = m_tLastIS.nY = 0;
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	HRESULT Init(ISharedStateManager* a_pFrame, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID);

	DECLARE_WND_CLASS_EX(_T("AniViewWndClass"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, COLOR_WINDOW);

BEGIN_MSG_MAP(CDesignerViewAnimation)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewAnimation>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	CHAIN_MSG_MAP(CImageControl<CDesignerViewAnimation>)
	REFLECT_NOTIFICATIONS()
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewAnimation)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_bObserving)
			m_pAnimation->ObserverDel(CObserverImpl<CDesignerViewAnimation, IAnimationObserver, TAnimationChange>::ObserverGet(), 0);
	}
	void OnFinalMessage(HWND)
	{
		Release();
	}

	void OwnerNotify(TCookie, ULONG a_nChangeFlags);
	void OwnerNotify(TCookie, TSharedStateChange a_tStateChange);
	void OwnerNotify(TCookie, TAnimationChange a_eChange);

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		if (m_clrBackground&0xff000000)
			RefreshImage(m_pSubImg.p, m_bFrame, GetSysColor(COLOR_WINDOW));
		return 0;
	}
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		KillTimer(14);
		PrepareFrame();
		RefreshImage(m_pSubImg.p, m_bFrame, m_clrBackground&0xff000000 ? GetSysColor(COLOR_WINDOW) : m_clrBackground);
		return 0;
	}
	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnPaint(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled);

	bool DrawThemeClientEdge(HRGN hRgnUpdate)
	{
		CBrush cBG;
		cBG.CreateSolidBrush((m_clrBackground&0xff000000) == 0 ? m_clrBackground : GetSysColor(COLOR_WINDOW));
		return AtlDrawThemeClientEdge(m_hTheme, m_hWnd, hRgnUpdate, cBG, 0, 0);
	}

	void PrepareFrame();

	void AutoZoomChanged(bool a_bEnabled) {}
	void VieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY) {}
	void ControlledVieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY) {}

	// IDesignerView methods
public:

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentAnimation> m_pAnimation;
	DWORD m_dwTimeOff;
	DWORD m_dwCurrentFrame;
	DWORD m_dwNextFrame;
	DWORD m_dwAniLength;
	IUnknown* m_pImgFrame;
	CComPtr<IDocumentImage> m_pSubImg;
	TImageSize m_tLastIS;
	bool m_bUpdating;
	CComPtr<IConfig> m_pConfig;
	bool m_bObserving;
	bool m_bFrame;
	COLORREF m_clrBackground;
	LCID m_tLocaleID;
};

