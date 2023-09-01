// DesignerViewEditToolProperties.h : Declaration of the CDesignerViewEditToolProperties

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ObserverImpl.h>
#include <MultiLanguageString.h>


// CDesignerViewEditToolProperties

class ATL_NO_VTABLE CDesignerViewEditToolProperties : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewEditToolProperties>,
	public CThemeImpl<CDesignerViewEditToolProperties>,
	public CDesignerViewWndImpl<CDesignerViewEditToolProperties, IDesignerView>,
	public CObserverImpl<CDesignerViewEditToolProperties, ISharedStateObserver, TSharedStateChange>
{
public:
	CDesignerViewEditToolProperties() : m_bFirstChange(true), m_nBarHeight(20), m_nIconSize(16), m_nGapX(3), m_nGapY(2), m_hToolIcon(NULL)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CDesignerViewEditToolProperties()
	{
		if (m_hToolIcon) DestroyIcon(m_hToolIcon);
		m_cImageList.Destroy();
	}

	void Init(ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGroup, HWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_tLocaleID = a_tLocaleID;

		if (a_pszSyncGroup != NULL && a_pszSyncGroup[0] != L'\0')
		{
			m_bstrSyncGroup = a_pszSyncGroup;
			a_pFrame->ObserverIns(ObserverGet(), 0);
			m_pSharedState = a_pFrame;
		}

		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("EditToolProperties View"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT : WS_EX_CONTROLPARENT) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

	DECLARE_WND_CLASS_EX(_T("RWViewEditToolProperties"), 0, COLOR_WINDOW);

	enum { IDC_TOOL_HELP = 100 };

BEGIN_COM_MAP(CDesignerViewEditToolProperties)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewEditToolProperties)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewEditToolProperties>)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
	COMMAND_HANDLER(IDC_TOOL_HELP, BN_CLICKED, OnClickedHelp)
END_MSG_MAP()


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
		try
		{
			if (m_hWnd == NULL || m_pManager == NULL)
				return;

			if (m_bstrSyncGroup != a_tParams.bstrName)
				return; // notification is not for our group

			CComQIPtr<ISharedStateToolMode> pSel(a_tParams.pState);
			if (pSel == NULL)
				return; // unknown data format

			CComBSTR bstrActiveToolID;
			pSel->Get(&bstrActiveToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

			if (bstrActiveToolID != m_bstrToolID)
			{
				bool bFocus = !m_bFirstChange && m_bstrToolID.Length() && bstrActiveToolID.Length();
				SwitchTool(bstrActiveToolID);
				if (bFocus) // commented out because it would grab focus too often otherwise (when cycling tools via keyboard accelerators) (it is still good if it scrolls into view)
					GetParent().SendMessage(WM_RW_GOTFOCUS);//, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
				m_bFirstChange = false;
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
		return m_pToolWindow ? m_pToolWindow->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (!IsWindow() || m_pToolWindow == NULL)
				return E_FAIL;

			SIZE szMax = {0, 0};
			m_pToolWindow->OptimumSize(&szMax);
			szMax.cy += m_nBarHeight;

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
			RWCoCreateInstance(m_pManager, __uuidof(RasterImageEditToolsManager));

			HDC hDC = GetDC();
			float fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
			ReleaseDC(hDC);
			m_nGapX = int(3*fScale+0.5f);
			m_nGapY = int(2*fScale+0.5f);
			m_nIconSize = XPGUI::GetSmallIconSize();
			m_nTitleIconSize = m_nIconSize*1.25f;
			m_cImageList.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 1, 0);
			static const GUID tHelpIconID = {0xa31e03a7, 0x2ea4, 0x4d0b, {0x8b, 0xf1, 0xdb, 0x6b, 0x1f, 0x92, 0x9c, 0x75}};
			CComPtr<IDesignerFrameIcons> pIcMgr;
			RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
			HICON hIcon = NULL;
			if (pIcMgr) pIcMgr->GetIcon(tHelpIconID, m_nIconSize, &hIcon);
			m_cImageList.AddIcon(hIcon);
			if (hIcon) DestroyIcon(hIcon);
			m_wndHelp = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, 0, 100, 0, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR), _pModule->get_m_hInst(), NULL);
			m_wndHelp.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS);
			m_wndHelp.SetButtonStructSize(sizeof(TBBUTTON));
			m_wndHelp.SetImageList(m_cImageList);
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Help[0405]Nápověda", m_tLocaleID, &bstr);
			TBBUTTON tBtn = {0, IDC_TOOL_HELP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstr.m_str)};
			m_wndHelp.AddButtons(1, &tBtn);
			if (IsThemingSupported() && !IsThemeNull())
			{
				int nButtonSize = m_nIconSize*1.625f + 0.5f;
				m_wndHelp.SetButtonSize(nButtonSize, nButtonSize);
			}
			RECT rcCl = {0, 0, 0, 0};
			GetClientRect(&rcCl);
			RECT rc = {0, 0, 0, 0};
			m_wndHelp.GetItemRect(0, &rc);
			RECT rcTB = {rcCl.right-m_nGapX-rc.right, m_nGapY, rcCl.right-m_nGapX, m_nGapY+rc.bottom};
			m_wndHelp.SetWindowPos(NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
			m_nBarHeight = rc.bottom+m_nGapY+m_nGapY;

			NONCLIENTMETRICS ncm;
			if (IsThemingSupported() && !IsThemeNull())
			{
				ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
				SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
				ncm.lfMessageFont.lfHeight = ncm.lfMessageFont.lfHeight*1.3+0.5f;
				ncm.lfMessageFont.lfWeight = FW_BOLD;
				m_cFont.CreateFontIndirect(&ncm.lfMessageFont);
			}
			else
			{
				m_cFont.CreateFont(11*1.3*fScale+0.5f, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, _T("MS Shell Dlg"));
			}

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
			if (m_pToolWindow == NULL)
			{
				CComPtr<IEnumStrings> pIDs;
				m_pManager->ToolIDsEnum(&pIDs);
				CComBSTR bstrID;
				pIDs->Get(0, &bstrID);
				SwitchTool(bstrID);
			}

			return m_pToolWindow ? 0 : -1;
		}
		catch (...)
		{
			return -1;
		}
	}
	LRESULT OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		//static bool bRecursion = false; // TODO: W9x workaround -> solve differently !!!!
		if (/*!bRecursion && */m_pToolWindow && IsWindowVisible())
		{
			//bRecursion = true;
			RWHWND h = NULL;
			m_pToolWindow->Handle(&h);
			::SetFocus(h);
			//bRecursion = false;
		}

		return 0;
	}
	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pToolWindow)
		{
			RECT rc = {0, m_nBarHeight, LOWORD(a_lParam), HIWORD(a_lParam)};
			m_pToolWindow->Move(&rc);
		}
		RECT rcTB = {0, 0, 0, 0};
		m_wndHelp.GetWindowRect(&rcTB);
		m_wndHelp.SetWindowPos(NULL, LOWORD(a_lParam)-m_nGapX-rcTB.right+rcTB.left, m_nGapY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		return 0;
	}
	LRESULT OnPaint(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		if (a_wParam != NULL)
		{
			CDCHandle dc = (HDC)a_wParam;
			DoPaint(dc);
		}
		else
		{
			CPaintDC dc(m_hWnd);
			DoPaint(dc.m_hDC);
		}
		return 0;
	}
	void DoPaint(CDCHandle dc)
	{
		if (m_bstrCachedToolID != m_bstrToolID)
		{
			m_bstrToolName.Empty();
			if (m_hToolIcon)
			{
				DestroyIcon(m_hToolIcon);
				m_hToolIcon = NULL;
			}
			m_bstrCachedToolID = m_bstrToolID;
		}
		if (m_bstrToolID.Length())
		{
			if (m_bstrToolName.Length() == 0)
			{
				CComPtr<ILocalizedString> pName;
				m_pManager->ToolNameGet(m_bstrToolID, &pName);
				if (pName) pName->GetLocalized(m_tLocaleID, &m_bstrToolName);
			}
			if (m_hToolIcon == NULL)
			{
				m_pManager->ToolIconGet(m_bstrToolID, m_nTitleIconSize, &m_hToolIcon);
			}
		}
		if (m_hToolIcon)
			dc.DrawIconEx(m_nGapX+m_nGapX, (m_nBarHeight-m_nTitleIconSize)>>1, m_hToolIcon, m_nTitleIconSize, m_nTitleIconSize);
		if (m_bstrToolName.m_str && *m_bstrToolName.m_str)
		{
			HFONT hOld = NULL;
			if (m_cFont) hOld = dc.SelectFont(m_cFont);
			RECT rcTB = {0, 0, 0, 0};
			m_wndHelp.GetWindowRect(&rcTB);
			ScreenToClient(&rcTB);
			RECT rcText = {m_nGapX+m_nGapX+m_nTitleIconSize+m_nGapX, 0, rcTB.left-m_nGapX, m_nBarHeight};
			dc.DrawText(m_bstrToolName, -1, &rcText, DT_END_ELLIPSIS|DT_LEFT|DT_VCENTER|DT_SINGLELINE);
			if (hOld) dc.SelectFont(hOld);
		}
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnClickedHelp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nIDLen = m_bstrToolID.Length();
		if (nIDLen)
		{
			CAutoVectorPtr<wchar_t> sz(new wchar_t[m_bstrToolID.Length()+50]);
			wchar_t* psz = sz;
			wchar_t const* pszWeb = L"http://www.rw-designer.com/";
			while (*psz = *pszWeb++) ++psz;
			for (int i = 0; i < nIDLen; ++i)
			{
				if (m_bstrToolID[i] >= L'A' && m_bstrToolID[i] <= L'Z')
					*psz = m_bstrToolID[i]-L'A'+L'a';
				else if (m_bstrToolID[i] == L'_')
					*psz = L'-';
				else
					*psz = m_bstrToolID[i];
				++psz;
			}
			wchar_t const* pszTool = L"-tool";
			while (*psz++ = *pszTool++);
			::ShellExecute(0, _T("open"), sz, 0, 0, SW_SHOWNORMAL);
		}
		return 0;
	}

private:
	typedef std::map<CComBSTR, CComPtr<IRasterImageEditToolWindow> > CCachedWindows;

	class CEditToolWindowNoOptions :
		public CComObjectRootEx<CComMultiThreadModel>,
		public Win32LangEx::CLangDialogImpl<CEditToolWindowNoOptions>,
		public CDialogResize<CEditToolWindowNoOptions>,
		public CChildWindowImpl<CEditToolWindowNoOptions, IRasterImageEditToolWindow>
	{
	public:
		enum { IDD = IDD_EDITTOOL_NOOPTIONS };

		BEGIN_COM_MAP(CEditToolWindowNoOptions)
			COM_INTERFACE_ENTRY(IChildWindow)
			COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
		END_COM_MAP()

		BEGIN_MSG_MAP(CEditToolWindowNoOptions)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
			CHAIN_MSG_MAP(CDialogResize<CEditToolWindowNoOptions>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CEditToolNoOptionsDlg)
			DLGRESIZE_CONTROL(IDC_ET_NOOPTIONS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			DlgResize_Init(false, false, 0);

			return 1;  // Let the system set the focus
		}
		LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			return 0;
		}
		LRESULT OnCtlColor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
		STDMETHOD(OptimumSize)(SIZE* a_pSize)
		{
			try
			{
				a_pSize->cx = a_pSize->cy = 0;
				return S_OK;
				//return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
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
	};

private:
	void SwitchTool(CComBSTR& a_bstrID)
	{
		CCachedWindows::iterator i = m_cCachedWindows.find(a_bstrID);
		if (i == m_cCachedWindows.end())
		{
			CComPtr<IRasterImageEditToolWindow> pWnd;
			CComBSTR bstr = m_bstrSyncGroup;
			bstr += a_bstrID;
			m_pManager->WindowCreate(a_bstrID, m_hWnd, m_tLocaleID, m_pSharedState, bstr, &pWnd);
			if (pWnd == NULL)
			{
				CComObject<CEditToolWindowNoOptions>* p = NULL;
				CComObject<CEditToolWindowNoOptions>::CreateInstance(&p);
				pWnd = p;
				p->m_tLocaleID = m_tLocaleID;
				p->Create(m_hWnd);
			}
			m_cCachedWindows[a_bstrID] = pWnd;
			i = m_cCachedWindows.find(a_bstrID);
		}
		m_bstrToolID = i->first;
		if (m_pToolWindow != i->second)
		{
			if (m_pToolWindow)
				m_pToolWindow->Show(FALSE);
			m_pToolWindow = i->second;
			RECT rc = {0, 0, 200, 200};
			GetClientRect(&rc);
			rc.top += m_nBarHeight;
			m_pToolWindow->Move(&rc);
			m_pToolWindow->Show(TRUE);
		}
		Invalidate(TRUE);//m_nBarHeight
	}

private:
	LCID m_tLocaleID;
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncGroup;
	CComBSTR m_bstrToolID;
	CComBSTR m_bstrCachedToolID;
	CComBSTR m_bstrToolName;
	HICON m_hToolIcon;
	CFont m_cFont;
	CComPtr<IRasterImageEditToolWindow> m_pToolWindow;
	CCachedWindows m_cCachedWindows;
	bool m_bFirstChange;
	ULONG m_nBarHeight;
	CToolBarCtrl m_wndHelp;
	CImageList m_cImageList;
	int m_nIconSize;
	int m_nTitleIconSize;
	int m_nGapX;
	int m_nGapY;
};

