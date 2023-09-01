// DesignerViewOutlineProperties.h : Declaration of the CDesignerViewOutlineProperties

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ObserverImpl.h>
#include <RWBaseEnumUtils.h>
#include "SharedStateToolMode.h"


// CDesignerViewOutlineProperties

class ATL_NO_VTABLE CDesignerViewOutlineProperties : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewOutlineProperties>,
	public CThemeImpl<CDesignerViewOutlineProperties>,
	public CDesignerViewWndImpl<CDesignerViewOutlineProperties, IDesignerView>,
	public CObserverImpl<CDesignerViewOutlineProperties, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewOutlineProperties, IColorPickerObserver, ULONG>
{
public:
	CDesignerViewOutlineProperties() : m_bVisible(false)
	{
		m_tBorders.cx = m_tBorders.cy = 0;
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CDesignerViewOutlineProperties()
	{
	}

	void Init(ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGroup, HWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_tLocaleID = a_tLocaleID;

		if (a_pszSyncGroup != NULL && a_pszSyncGroup[0] != L'\0')
		{
			m_bstrSyncGroup = a_pszSyncGroup;
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewOutlineProperties, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pSharedState = a_pFrame;
		}

		static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
		if (tVersion.dwMajorVersion == 0)
			GetVersionEx(&tVersion);
		m_tBorders.cx = m_tBorders.cy = 5;
		NONCLIENTMETRICS ncm;
		if (tVersion.dwMajorVersion >= 6)
		{
			ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
			static int nLogPixelsY = 0;
			if (nLogPixelsY == 0)
			{
				HDC hDC = ::GetDC(NULL);
				nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
				::ReleaseDC(NULL, hDC);
			}
			_DialogSizeHelper::ConvertDialogUnitsToPixels(ncm.lfMessageFont.lfFaceName, MulDiv(ncm.lfMessageFont.lfHeight < 0 ? -ncm.lfMessageFont.lfHeight : ncm.lfMessageFont.lfHeight, 72, nLogPixelsY), &m_tBorders, false);
		}
		else
		{
			_DialogSizeHelper::ConvertDialogUnitsToPixels(_T("MS Shell Dlg"), 8, &m_tBorders, false);
		}

		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("OutlineProperties View"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT : WS_EX_CONTROLPARENT) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

	DECLARE_WND_CLASS_EX(_T("RWViewOutlineProperties"), 0, COLOR_WINDOW);


BEGIN_COM_MAP(CDesignerViewOutlineProperties)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewOutlineProperties)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewOutlineProperties>)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
END_MSG_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pSharedState != NULL)
			m_pSharedState->ObserverDel(CObserverImpl<CDesignerViewOutlineProperties, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pPickerWindow != NULL)
			m_pPickerWindow->ObserverDel(CObserverImpl<CDesignerViewOutlineProperties, IColorPickerObserver, ULONG>::ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		try
		{
			if (m_hWnd == NULL || m_pManager == NULL)
				return;

			if (m_bstrSyncGroup != a_tParams.bstrName)
				return; // notification is not for our group

			CComQIPtr<ISharedStateToolMode> pSel(a_tParams.pState);
			if (pSel == NULL)
				return; // unknown data format

			CComBSTR bstrToolID;
			TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
			BOOL bOutlineEnabled = FALSE;
			std::vector<ULONG> aPaints;
			pSel->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, &bOutlineEnabled, &tColor, NULL, NULL, NULL);
			if (m_pToolsManager) m_pToolsManager->SupportedStates(bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
			bool bOutlineSupported = false;
			for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
			{
				if ((*i&ETBTIdentifierMask) != ETBTIdentifierOutline)
					continue;
				bOutlineSupported = true;
			}

			if (!bOutlineSupported)
				bOutlineEnabled = FALSE;

			if (m_bVisible != !!bOutlineEnabled)
			{
				m_bVisible = !!bOutlineEnabled;
				m_pPickerWindow->Show(m_bVisible);
			}
			TColor tColorOld = tColor;
			m_pPickerWindow->ColorGet(&tColorOld);
			if (fabsf(tColor.fR-tColorOld.fR) > 1e-4f || fabsf(tColor.fG-tColorOld.fG) > 1e-4f ||
				fabsf(tColor.fB-tColorOld.fB) > 1e-4f || fabsf(tColor.fA-tColorOld.fA) > 1e-4f)
			{
				m_pPickerWindow->ColorSet(&tColor);
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, ULONG a_tParams)
	{
		try
		{
			if (a_tParams&ECPCColor)
			{
				if (m_pSharedState)
				{
					TColor tColor;
					m_pPickerWindow->ColorGet(&tColor);
					CComPtr<ISharedStateToolMode> pPrev;
					m_pSharedState->StateGet(m_bstrSyncGroup, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
					CComObject<CSharedStateToolMode>* pNew = NULL;
					CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
					CComPtr<ISharedState> pTmp = pNew;
					BOOL bOutline = TRUE;
					if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, &tColor, NULL, NULL, NULL, pPrev))
						m_pSharedState->StateSet(m_bstrSyncGroup, pTmp);
				}
			}
		}
		catch (...)
		{
		}
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_pPickerWindow ? m_pPickerWindow->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (!m_bVisible)
			{
				a_pSize->cx = a_pSize->cy = 0;
				return S_OK;
			}

			if (!IsWindow() || m_pPickerWindow == NULL)
				return E_FAIL;

			SIZE szMax = *a_pSize;
			if (szMax.cx) szMax.cx -= m_tBorders.cx*2;
			m_pPickerWindow->OptimumSize(&szMax);
			szMax.cx += m_tBorders.cx*2;
			szMax.cy += m_tBorders.cy*2;

			if (GetWindowLong(GWL_EXSTYLE)&WS_EX_CLIENTEDGE)
			{
				int cxEdge = GetSystemMetrics(SM_CXEDGE);
				int cyEdge = GetSystemMetrics(SM_CYEDGE);
				szMax.cx += cxEdge + cxEdge;
				szMax.cy += cyEdge + cyEdge;
			}
			if (a_pSize->cx < szMax.cx)
				a_pSize->cx = szMax.cx;
			a_pSize->cy = szMax.cy;
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// message handlers
protected:
	LRESULT OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		try
		{
			RWCoCreateInstance(m_pManager, __uuidof(RasterImageFillStyleManager));
			RWCoCreateInstance(m_pToolsManager, __uuidof(RasterImageEditToolsManager));

			RECT rc = {0, 0, 0, 0};
			GetClientRect(&rc);
			rc.left += m_tBorders.cx;
			rc.top += m_tBorders.cy;
			rc.right -= m_tBorders.cx;
			rc.bottom -= m_tBorders.cy;
			RWCoCreateInstance(m_pPickerWindow, __uuidof(ColorPicker));
			m_pPickerWindow->Create(m_hWnd, &rc, FALSE, m_tLocaleID, NULL, 0, CComBSTR(L"OUTLINE"), NULL);
			m_pPickerWindow->ObserverIns(CObserverImpl<CDesignerViewOutlineProperties, IColorPickerObserver, ULONG>::ObserverGet(), 0);

			CComPtr<ISharedState> pState;
			if (m_pSharedState)
				m_pSharedState->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			if (pState)
			{
				TSharedStateChange tParams;
				tParams.bstrName = m_bstrSyncGroup;
				tParams.pState = pState;
				OwnerNotify(0, tParams);
			}

			return 0;
		}
		catch (...)
		{
			return -1;
		}
	}
	LRESULT OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pPickerWindow && IsWindowVisible())
		{
			RWHWND h = NULL;
			m_pPickerWindow->Handle(&h);
			::SetFocus(h);
		}

		return 0;
	}
	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pPickerWindow)
		{
			RECT rc = {m_tBorders.cx, m_tBorders.cy, LOWORD(a_lParam)-m_tBorders.cx, HIWORD(a_lParam)-m_tBorders.cy};
			m_pPickerWindow->Move(&rc);
		}
		return 0;
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, (LPARAM)m_hWnd);
		if (lRes)
		{
			RECT rc = {0, 0, 0, 0};
			GetClientRect(&rc);
			FillRect((HDC)a_wParam, &rc, (HBRUSH)lRes);
			return 1;
		}
		else
		{
			return 0;
		}
	}

private:
	LCID m_tLocaleID;
	CComPtr<IRasterImageFillStyleManager> m_pManager;
	CComPtr<IRasterImageEditToolsManager> m_pToolsManager;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncGroup;
	CComBSTR m_bstrStyleID;
	CComPtr<IColorPicker> m_pPickerWindow;
	SIZE m_tBorders;
	bool m_bVisible;
};


