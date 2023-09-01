// DesignerViewImage.h : Declaration of the CDesignerViewImage

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"
#include "ImageControl.h"
#include <ObserverImpl.h>


// CDesignerViewImage

class ATL_NO_VTABLE CDesignerViewImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CImageControl<CDesignerViewImage>,
	public CThemeImpl<CDesignerViewImage>,
	public CDesignerViewWndImpl<CDesignerViewImage, IDesignerView>,
	public CObserverImpl<CDesignerViewImage, IImageObserver, TImageChange>,
	public CObserverImpl<CDesignerViewImage, ISharedStateObserver, TSharedStateChange>,
	public IImageZoomControl
{
public:
	CDesignerViewImage() : m_bObserving(false), m_bUpdating(false), m_bFrame(false)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	HRESULT Init(ISharedStateManager* a_pFrame, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID);

	DECLARE_WND_CLASS_EX(_T("ImageControlWndClass"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, COLOR_3DFACE);

BEGIN_MSG_MAP(CDesignerViewImage)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewImage>)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	if (uMsg == WM_CREATE)
		AddRef();
	CHAIN_MSG_MAP(CImageControl<CDesignerViewImage>)
	REFLECT_NOTIFICATIONS()
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()

	static HRESULT WINAPI QIImageZoomControl(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CDesignerViewImage* p = reinterpret_cast<CDesignerViewImage*>(pv);
		if (p->m_bstrControlledID.m_str == NULL || p->m_bstrControlledID.m_str[0] == L'\0')
		{
			*ppv = static_cast<IImageZoomControl*>(p);
			p->AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

BEGIN_COM_MAP(CDesignerViewImage)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY_FUNC(__uuidof(IImageZoomControl), 0, QIImageZoomControl)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_bObserving)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewImage, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_bstrViewportID.Length() || m_bstrControlledID.Length())
			m_pStates->ObserverDel(CObserverImpl<CDesignerViewImage, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}
	void OnFinalMessage(HWND)
	{
		Release();
	}

	void OwnerNotify(TCookie, TImageChange a_tChange);
	void OwnerNotify(TCookie, TSharedStateChange a_tStateChange);

	// handlers
public:
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled)
	{
		if (!m_bCustomBackground)
			RefreshImage(m_pImage.p, m_bFrame, GetSysColor(COLOR_WINDOW));
		return 0;
	}
	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled);

	bool DrawThemeClientEdge(HRGN hRgnUpdate)
	{
		CBrush cBG;
		cBG.CreateSolidBrush(m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
		return AtlDrawThemeClientEdge(m_hTheme, m_hWnd, hRgnUpdate, cBG, 0, 0);
	}

	// IDesignerView methods
public:

	// IImageZoomControl methods
public:
	STDMETHOD(CanSetZoom)(float* a_pVal);
	STDMETHOD(SetZoom)(float a_fVal);
	STDMETHOD(GetZoom)(float* a_pVal);
	STDMETHOD(SupportsAutoZoom)();
	STDMETHOD(IsAutoZoomEnabled)();
	STDMETHOD(EnableAutoZoom)(BOOL a_bEnable);

	void AutoZoomChanged(bool a_bEnabled);
	void VieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY);
	void ControlledVieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY);

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentImage> m_pImage;
	CComPtr<ISharedStateManager> m_pStates;
	CComBSTR m_bstrViewportID;
	CComBSTR m_bstrControlledID;
	bool m_bUpdating;
	CComPtr<IConfig> m_pConfig;
	bool m_bObserving;
	COLORREF m_clrBackground;
	bool m_bCustomBackground;
	bool m_bFrame;
	LCID m_tLocaleID;
};

