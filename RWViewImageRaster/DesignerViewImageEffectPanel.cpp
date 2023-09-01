
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <MultiLanguageString.h>
#include <XPGUI.h>
#include <RenderIcon.h>
#include "ConfigGUILayerID.h"
#include <SharedStringTable.h>
#include <SharedStateUndo.h>
#include <ContextMenuWithIcons.h>
#include <IconRenderer.h>


extern wchar_t const DELETEEFFECT_NAME[] = L"[0409]Remove effect[0405]Odstranit efekt";


class ATL_NO_VTABLE CDesignerViewImageEffectPanel : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewImageEffectPanel>,
	public CThemeImpl<CDesignerViewImageEffectPanel>,
	public CDesignerViewWndImpl<CDesignerViewImageEffectPanel, IDesignerView>,
	public CObserverImpl<CDesignerViewImageEffectPanel, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewImageEffectPanel, IStructuredObserver, TStructuredChanges>,
	public CObserverImpl<CDesignerViewImageEffectPanel, IConfigObserver, IUnknown*>,
	public CContextMenuWithIcons<CDesignerViewImageEffectPanel>
{
public:
	CDesignerViewImageEffectPanel() : m_bEnableUpdates(NULL), m_nBarHeight(20), m_nIconSize(16), m_nTitleIconSize(20), m_nGapX(3), m_nGapY(2), m_hOpIcon(NULL)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CDesignerViewImageEffectPanel()
	{
		if (m_hOpIcon) DestroyIcon(m_hOpIcon);
		m_cImageList.Destroy();
	}


	void Init(RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, ISharedStateManager* a_pFrame, BSTR a_bstrSyncID)
	{
		m_tLocaleID = a_tLocaleID;
		m_pDoc = a_pDoc;
		m_pDLI = a_pDLI;

		CComBSTR bstrID;
		m_pDLI->ObserverIns(CObserverImpl<CDesignerViewImageEffectPanel, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		m_pDLI->StatePrefix(&bstrID);
		if (bstrID.Length())
		{
			m_bstrSyncID = bstrID;
			m_bstrSyncID += a_bstrSyncID;
		}
		else
		{
			m_bstrSyncID = a_bstrSyncID;
		}
		if (m_bstrSyncID)
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewImageEffectPanel, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pFrame = a_pFrame;
		}

		CComPtr<ISharedState> pState;
		if (m_pFrame)
			m_pFrame->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		m_pDLI->StateUnpack(pState, &pItems);

		CComPtr<IComparable> pItem;
		if (pItems) pItems->Get(0, &pItem);

		m_pItem = pItem;

		m_pDLI->LayerEffectStepGet(pItem, NULL, &m_tOpID, &m_pCfg);

		m_pCfg->ObserverIns(CObserverImpl<CDesignerViewImageEffectPanel, IConfigObserver, IUnknown*>::ObserverGet(), 0);

		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ImageEffectPanel View"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, (a_nStyle&EDVWSBorderMask) == EDVWSBorder ? WS_EX_CLIENTEDGE|WS_EX_CONTROLPARENT : WS_EX_CONTROLPARENT) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}

		m_bEnableUpdates = true;
	}

	DECLARE_WND_CLASS_EX(_T("RWViewImageEffectPanel"), 0, COLOR_WINDOW);

	enum { IDC_TOOL_HELP = 100, IDC_TOOL_PRESETS, IDC_TOOL_DONE, IDC_TOOL_REMOVE };

BEGIN_COM_MAP(CDesignerViewImageEffectPanel)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewImageEffectPanel)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewImageEffectPanel>)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewImageEffectPanel>)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
	COMMAND_HANDLER(IDC_TOOL_HELP, BN_CLICKED, OnClickedHelp)
	COMMAND_HANDLER(IDC_TOOL_DONE, BN_CLICKED, OnClickedDone)
	COMMAND_HANDLER(IDC_TOOL_REMOVE, BN_CLICKED, OnClickedRemove)
	NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnButtonDropDown)
END_MSG_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pCfg)
			m_pCfg->ObserverDel(CObserverImpl<CDesignerViewImageEffectPanel, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		if (m_pDLI)
			m_pDLI->ObserverDel(CObserverImpl<CDesignerViewImageEffectPanel, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		if (m_pFrame)
			m_pFrame->ObserverDel(CObserverImpl<CDesignerViewImageEffectPanel, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pWnd == NULL)
			return S_FALSE;
		return m_pWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		try
		{
			if (m_hWnd == NULL || m_pWnd == NULL)
				return E_FAIL;

			SIZE szMax = {0, XPGUI::GetSmallIconSize()*10};
			m_pWnd->OptimumSize(&szMax);
			szMax.cy += m_nBarHeight*2;

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
//	STDMETHOD(OnIdle)()
//	{
//		RECT rc = {0, 0, 0, 0};
//		if (m_hWnd == NULL)
//			return S_FALSE;
//		GetClientRect(&rc);
//		if (rc.bottom <= 0 || rc.right  <= 0)
//			return S_FALSE;
//		bool bChange = false;
//		for (size_t i = 0; i < m_cItems.size(); ++i)
//		{
//			SIZE sz = {rc.right, m_cItems[i].nHeight};
//			m_cItems[i].pWindow->OptimumSize(&sz);
//			if (m_cItems[i].nHeight != sz.cy)
//				bChange = true;
//		}
//		if (bChange)
//		{
//			SIZE sz = {rc.right, rc.bottom};
//			RepositionControls(sz);
//		}
//		return S_FALSE;
//	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, IUnknown*)
	{
		try
		{
			if (!m_bEnableUpdates)
				return;

			//if (m_pLayer)
			{
				m_bEnableUpdates = false;
				m_pDLI->LayerEffectStepSet(m_pItem, NULL, NULL, m_pCfg);
				//GUI2Data();
				//m_pImage->LayerEffectSet(m_pLayer, m_pEffect);
				m_bEnableUpdates = true;
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, TStructuredChanges a_tChanges)
	{
		try
		{
			if (!m_bEnableUpdates || m_pItem == NULL)
				return;
			for (ULONG i = 0; i < a_tChanges.nChanges; ++i)
			{
				if ((a_tChanges.aChanges[i].nChangeFlags&ESCContent) &&
					S_OK == m_pItem->Compare(a_tChanges.aChanges[i].pItem))
				{
					Data2GUI();
					break;
				}
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tChange)
	{
		try
		{
			if (!m_bEnableUpdates)
				return;

			if (m_bstrSyncID != a_tChange.bstrName)
				return;

			CComPtr<IEnumUnknowns> pItems;
			m_pDLI->StateUnpack(a_tChange.pState, &pItems);

			CComPtr<IComparable> pItem;
			if (pItems) pItems->Get(0, &pItem);

			if (pItem == m_pItem)
				return;
			if (pItem != NULL && m_pItem != NULL && m_pItem->Compare(pItem) == S_OK)
				return;
			m_pItem = pItem;

			//if (a_dwFlags&(ELECOpCfg|ELECOpID))
			Data2GUI();
		}
		catch (...)
		{
		}
	}
	void Data2GUI()
	{
		m_bEnableUpdates = false;
		GUID tID = GUID_NULL;
		CComPtr<IConfig> pNew;
		m_pDLI->LayerEffectStepGet(m_pItem, NULL, &tID, &pNew);
		if (IsEqualGUID(tID, m_tOpID))
		{
			CopyConfigValues(m_pCfg, pNew);
		}
		else
		{
			if (m_pCfg)
				m_pCfg->ObserverDel(CObserverImpl<CDesignerViewImageEffectPanel, IConfigObserver, IUnknown*>::ObserverGet(), 0);
			m_tOpID = tID;
			m_pCfg = pNew;
			if (m_pCfg)
				m_pCfg->ObserverIns(CObserverImpl<CDesignerViewImageEffectPanel, IConfigObserver, IUnknown*>::ObserverGet(), 0);
			m_pWnd->ConfigSet(m_pCfg, ECPMWithCanvas);
		}
		//GUI2Data();
		//m_pImage->LayerEffectSet(m_pLayer, m_pEffect);
		m_bEnableUpdates = true;
	}

	// message handlers
protected:
	LRESULT OnCreate(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		try
		{
			HDC hDC = GetDC();
			float fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
			ReleaseDC(hDC);
			m_nGapX = int(3*fScale+0.5f);
			m_nGapY = int(2*fScale+0.5f);
			m_nIconSize = XPGUI::GetSmallIconSize();
			m_nTitleIconSize = m_nIconSize*1.25f;
			m_cImageList.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 1, 0);
			//static const GUID tHelpIconID = {0xa31e03a7, 0x2ea4, 0x4d0b, {0x8b, 0xf1, 0xdb, 0x6b, 0x1f, 0x92, 0x9c, 0x75}};
			//CComPtr<IDesignerFrameIcons> pIcMgr;
			//RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
			//HICON hIcon = NULL;
			//if (pIcMgr) pIcMgr->GetIcon(tHelpIconID, m_nIconSize, &hIcon);
			//m_cImageList.AddIcon(hIcon);
			//if (hIcon) DestroyIcon(hIcon);
			InitIcons();
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

			m_wndControls = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, 0, 100, 0, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR), _pModule->get_m_hInst(), NULL);
			m_wndControls.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS);
			m_wndControls.SetButtonStructSize(sizeof(TBBUTTON));
			m_wndControls.SetImageList(m_cImageList);

			CComBSTR bstrDone;
			CMultiLanguageString::GetLocalized(L"[0409]Done[0405]Hotovo", m_tLocaleID, &bstrDone);
			CComBSTR bstrRemove;
			CMultiLanguageString::GetLocalized(L"[0409]Remove[0405]Odstranit", m_tLocaleID, &bstrRemove);
			TBBUTTON aBtns[] =
			{
				{2, IDC_TOOL_DONE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrDone.m_str)},
				{1, IDC_TOOL_REMOVE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrRemove.m_str)},
			};
			m_wndControls.AddButtons(2, aBtns);
			if (IsThemingSupported() && !IsThemeNull())
			{
				int nButtonSize = m_nIconSize*1.625f + 0.5f;
				m_wndControls.SetButtonSize(nButtonSize, nButtonSize);
			}
			m_wndControls.GetItemRect(1, &rc);
			m_wndControls.SetWindowPos(NULL, 0, rcCl.bottom-m_nBarHeight, rc.right, rc.bottom, SWP_NOZORDER);

			m_wndPresets = ::CreateWindowEx(0, TOOLBARCLASSNAME, NULL, (~WS_VISIBLE) & (ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST), 0, 0, 100, 0, m_hWnd, (HMENU)LongToHandle(ATL_IDW_TOOLBAR), _pModule->get_m_hInst(), NULL);
			m_wndPresets.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS|TBSTYLE_EX_HIDECLIPPEDBUTTONS|TBSTYLE_EX_DRAWDDARROWS);
			m_wndPresets.SetButtonStructSize(sizeof(TBBUTTON));
			m_wndPresets.SetImageList(m_cImageList);

			CComBSTR bstrPresets;
			CMultiLanguageString::GetLocalized(L"[0409]Presets[0405]Předvolby", m_tLocaleID, &bstrPresets);
			TBBUTTON tBtnPres = {3, IDC_TOOL_PRESETS, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_AUTOSIZE|BTNS_SHOWTEXT|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, INT_PTR(bstrPresets.m_str)};
			m_wndPresets.AddButtons(1, &tBtnPres);
			if (IsThemingSupported() && !IsThemeNull())
			{
				int nButtonSize = m_nIconSize*1.625f + 0.5f;
				m_wndPresets.SetButtonSize(nButtonSize, nButtonSize);
			}
			m_wndPresets.GetItemRect(0, &rc);
			m_wndPresets.SetWindowPos(NULL, m_nGapX, rcCl.bottom-m_nBarHeight, rc.right, rc.bottom, SWP_NOZORDER);

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

			RECT rcWindow = rcCl;
			rcWindow.top = m_nBarHeight;
			RWCoCreateInstance(m_pWnd, __uuidof(AutoConfigWnd));
			m_pWnd->Create(m_hWnd, &rcWindow, 0, m_tLocaleID, FALSE, ECWBMMargin);
			m_pWnd->ConfigSet(m_pCfg, ECPMWithCanvas);
			SIZE sz = {rcWindow.right-rcWindow.left, 100};
			m_pWnd->OptimumSize(&sz);
			rcWindow.bottom = rcWindow.top+sz.cy;
			m_pWnd->Move(&rcWindow);
			m_pWnd->Show(TRUE);

			return 0;
		}
		catch (...)
		{
			return -1;
		}
	}
	LRESULT OnSetFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pWnd && IsWindowVisible())
		{
			RWHWND h = NULL;
			m_pWnd->Handle(&h);
			::SetFocus(h);
		}

		return 0;
	}
	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_pWnd)
		{
			RECT rc = {0, m_nBarHeight, LOWORD(a_lParam), HIWORD(a_lParam)-m_nBarHeight};
			m_pWnd->Move(&rc);
		}
		RECT rcTB = {0, 0, 0, 0};
		m_wndHelp.GetWindowRect(&rcTB);
		m_wndHelp.SetWindowPos(NULL, LOWORD(a_lParam)-m_nGapX-rcTB.right+rcTB.left, m_nGapY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		m_wndControls.GetWindowRect(&rcTB);
		m_wndControls.SetWindowPos(NULL, LOWORD(a_lParam)-m_nGapX-rcTB.right+rcTB.left, HIWORD(a_lParam)-m_nBarHeight+m_nGapY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		m_wndPresets.GetWindowRect(&rcTB);
		m_wndPresets.SetWindowPos(NULL, m_nGapX, HIWORD(a_lParam)-m_nBarHeight+m_nGapY, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
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
		if (!IsEqualGUID(m_tCachedOpID, m_tOpID))
		{
			m_bstrOpName.Empty();
			if (m_hOpIcon)
			{
				DestroyIcon(m_hOpIcon);
				m_hOpIcon = NULL;
			}
			m_tCachedOpID = m_tOpID;
		}
		if (!IsEqualGUID(GUID_NULL, m_tOpID))
		{
			CComPtr<IConfigDescriptor> pCD;
			RWCoCreateInstance(pCD, m_tOpID);
			if (pCD)
			{
				if (m_bstrOpName.Length() == 0)
				{
					CComPtr<ILocalizedString> pName;
					pCD->Name(NULL, NULL, &pName);
					if (pName) pName->GetLocalized(m_tLocaleID, &m_bstrOpName);
				}
				if (m_hOpIcon == NULL)
				{
					pCD->PreviewIcon(NULL, NULL, m_nTitleIconSize, &m_hOpIcon);
				}
			}
		}
		if (m_hOpIcon)
			dc.DrawIconEx(m_nGapX+m_nGapX, (m_nBarHeight-m_nTitleIconSize)>>1, m_hOpIcon, m_nTitleIconSize, m_nTitleIconSize);
		if (m_bstrOpName.m_str && *m_bstrOpName.m_str)
		{
			HFONT hOld = NULL;
			if (m_cFont) hOld = dc.SelectFont(m_cFont);
			RECT rcTB = {0, 0, 0, 0};
			m_wndHelp.GetWindowRect(&rcTB);
			ScreenToClient(&rcTB);
			RECT rcText = {m_nGapX+m_nGapX+m_nTitleIconSize+m_nGapX, 0, rcTB.left-m_nGapX, m_nBarHeight};
			dc.DrawText(m_bstrOpName, -1, &rcText, DT_END_ELLIPSIS|DT_LEFT|DT_VCENTER|DT_SINGLELINE);
			if (hOld) dc.SelectFont(hOld);
		}
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnClickedHelp(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		TCHAR szGUID[64];
		StringFromGUID(m_tOpID, szGUID);
		TCHAR szURL[256];
		_stprintf(szURL, L"http://www.rw-designer.com/helpredir/%s/%i", szGUID, LANGIDFROMLCID(m_tLocaleID));
		::ShellExecute(0, _T("open"), szURL, 0, 0, SW_SHOWNORMAL);
		return 0;
	}
	LRESULT OnClickedDone(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComPtr<IComparable> pNewSel;
		m_pDLI->LayerFromEffect(m_pItem, &pNewSel);
		CComPtr<ISharedState> pNewState;
		m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
		m_pFrame->StateSet(m_bstrSyncID, pNewState);
		return 0;
	}
	LRESULT OnClickedRemove(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		//if (m_pView)
		//	m_pView->DeactivateAll(FALSE);

		CWriteLock<IDocument> cLock(m_pDoc.p);
		CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(DELETEEFFECT_NAME));

		CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc.p, m_pFrame, m_bstrSyncID);
		CComPtr<IComparable> pNewSel;
		//ULONG nItems = 0;
		//m_pItems->Size(&nItems);
		//for (ULONG i = 0; i < nItems; ++i)
		//{
		//	CComPtr<IComparable> pEffect;
		//	m_pItems->Get(i, &pEffect);
		//	if (pNewSel == NULL)
		//		m_pDLI->LayerFromEffect(pEffect, &pNewSel);
		//	m_pDLI->LayerEffectStepDelete(pEffect);
		//}
		m_pDLI->LayerFromEffect(m_pItem, &pNewSel);
		m_pDLI->LayerEffectStepDelete(m_pItem);
		CComPtr<ISharedState> pNewState;
		m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
		m_pFrame->StateSet(m_bstrSyncID, pNewState);

		return 0;
	}
	LRESULT OnButtonDropDown(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
		if (pTB->iItem != IDC_TOOL_PRESETS)
		{
			a_bHandled = FALSE;
			return 0;
		}

		POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
		m_wndPresets.ClientToScreen(&ptBtn);

		Reset(m_cImageList);

		CMenu cMenu;
		cMenu.CreatePopupMenu();

		UINT nMenuID = 1;
		AddItem(cMenu, nMenuID, _T("Reset"), -1);

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
		if (nSelection != 0)
		{
			//CWaitCursor cWait;
			//m_pContext->ResetErrorMessage();
			//HandleOperationResult(m_cCommands[nSelection-1]->Execute(m_hWnd, m_tLocaleID));
		}

		return TBDDRET_DEFAULT;
	}

private:
	void InitIcons()
	{
		{
			//static float const f = 1.0f/256.0f;
			//static TPolyCoords const aVertices[] =
			//{
			//	{f*240, f*128},
			//	{0, -f*61.856}, {f*61.856, 0}, {f*128, f*16},
			//	{-f*61.856, 0}, {0, -f*61.856}, {f*16, f*128},
			//	{0, f*61.856}, {-f*61.856, 0}, {f*128, f*240},
			//	{f*61.856, 0}, {0, f*61.856}, {f*240, f*128}
			//};
			//static TPolyCoords const aQuestion1[] =
			//{
			//	{f*139, f*157},
			//	{-f*0.114583, -f*6.67708}, {-f*0.0625, -f*3.16667}, {f*117, f*157},
			//	{0, 0}, {0, -f*7.13542}, {f*117, f*151},
			//	{0, f*0.6875}, {f*2.32292, -f*4.60417}, {f*120, f*134},
			//	{-f*2.33333, f*4.60417}, {f*6.96875, -f*5.75}, {f*134, f*118},
			//	{-f*6.97917, f*5.75}, {f*2.09375, -f*2.82292}, {f*147, f*107},
			//	{-f*1.36458, f*1.78125}, {0, -f*4.79167}, {f*150, f*97},
			//	{0, f*3.39583}, {-f*3.78125, -f*3.41667}, {f*144, f*85},
			//	{f*3.78125, f*3.42708}, {-f*6.17708, 0}, {f*129, f*80},
			//	{f*6.39583, 0}, {-f*4.15625, f*3.13542}, {f*114, f*85},
			//	{f*4.14583, -f*3.125}, {0, 0}, {f*105, f*99},
			//	{f*1.5625, -f*6.40625}, {f*0.635417, -f*9.73958}, {f*83, f*97},
			//	{0, 0}, {f*8.15625, -f*6.79167}, {f*96, f*72},
			//	{-f*8.16667, f*6.80208}, {f*13.9583, 0}, {f*128, f*62},
			//	{-f*13.2604, 0}, {f*8.23958, f*7.23958}, {f*162, f*73},
			//	{-f*8.25, -f*7.23958}, {0, f*5.35417}, {f*174, f*98},
			//	{0, -f*9.61458}, {-f*3.01042, f*4.78125}, {f*169, f*113},
			//	{f*3.01042, -f*4.77083}, {-f*5.09375, f*4.26042}, {f*150, f*133},
			//	{f*9.86458, -f*8.21875}, {-f*1.23958, f*2.59375}, {f*141, f*143},
			//	{f*1.22917, -f*2.58333}, {0, 0}, {f*139, f*157},
			//};
			//static TPolyCoords const aQuestion2[] =
			//{
			//	{f*117, f*189},
			//	{0, 0}, {0, 0}, {f*117, f*165},
			//	{0, 0}, {0, 0}, {f*139, f*165},
			//	{0, 0}, {0, 0}, {f*139, f*189},
			//	{0, 0}, {0, 0}, {f*117, f*189},
			//};
			////"S", 139, 157, 0, 0, -0.114583, -6.67708,
			////"S", 117, 157, -0.0625, -3.16667, 0, 0,
			////"C", 117, 151, 0, -7.13542, 0, 0.6875,
			////"S", 120, 134, 2.32292, -4.60417, -2.33333, 4.60417,
			////"C", 134, 118, 6.96875, -5.75, -6.97917, 5.75,
			////"S", 147, 107, 2.09375, -2.82292, -1.36458, 1.78125,
			////"C", 150, 97, 0, -4.79167, 0, 3.39583,
			////"S", 144, 85, -3.78125, -3.41667, 3.78125, 3.42708,
			////"C", 129, 80, -6.17708, 0, 6.39583, 0,
			////"C", 114, 85, -4.15625, 3.13542, 4.14583, -3.125,
			////"S", 105, 99, 0, 0, 1.5625, -6.40625,
			////"S", 83, 97, 0.635417, -9.73958, 0, 0,
			////"C", 96, 72, 8.15625, -6.79167, -8.16667, 6.80208,
			////"C", 128, 62, 13.9583, 0, -13.2604, 0,
			////"C", 162, 73, 8.23958, 7.23958, -8.25, -7.23958,
			////"S", 174, 98, 0, 5.35417, 0, -9.61458,
			////"C", 169, 113, -3.01042, 4.78125, 3.01042, -4.77083,
			////"S", 150, 133, -5.09375, 4.26042, 9.86458, -8.21875,
			////"E", 141, 143, -1.23958, 2.59375, 1.22917, -2.58333,
			////
			////"S", 117, 189, 0, 0, 0, 0,
			////"S", 117, 165, 0, 0, 0, 0,
			////"S", 139, 165, 0, 0, 0, 0,
			////"E", 139, 189, 0, 0, 0, 0
			//TIconPolySpec tPolySpec[3];
			//tPolySpec[0].nVertices = itemsof(aVertices);
			//tPolySpec[0].pVertices = aVertices;
			//tPolySpec[0].interior = agg::rgba8(0x67, 0x82, 0xeb, 0xff);//agg::rgba8(0x11, 0x57, 0xb8, 0xff);//GetIconFillColor();
			//tPolySpec[0].outline = agg::rgba8(0, 0, 0, 255);
			//tPolySpec[1].nVertices = itemsof(aQuestion1);
			//tPolySpec[1].pVertices = aQuestion1;
			//tPolySpec[1].interior = agg::rgba8(255, 255, 255, 255);
			//tPolySpec[1].outline = agg::rgba8(0, 0, 0, 0);
			//tPolySpec[2].nVertices = itemsof(aQuestion2);
			//tPolySpec[2].pVertices = aQuestion2;
			//tPolySpec[2].interior = agg::rgba8(255, 255, 255, 255);
			//tPolySpec[2].outline = agg::rgba8(0, 0, 0, 0);
			//HICON hIc = IconFromPath(itemsof(tPolySpec), tPolySpec, m_nIconSize, false);
			//m_cImageList.AddIcon(hIc);
			//DestroyIcon(hIc);

			static const GUID tHelpIconID = {0xa31e03a7, 0x2ea4, 0x4d0b, {0x8b, 0xf1, 0xdb, 0x6b, 0x1f, 0x92, 0x9c, 0x75}};
			CComPtr<IDesignerFrameIcons> pIcMgr;
			RWCoCreateInstance(pIcMgr, __uuidof(DesignerFrameIconsManager));
			HICON hIc = NULL;
			if (pIcMgr) pIcMgr->GetIcon(tHelpIconID, m_nIconSize, &hIc);

			//CComPtr<IStockIcons> pSI;
			//RWCoCreateInstance(pSI, __uuidof(StockIcons));
			//CIconRendererReceiver cRenderer(m_nIconSize);
			//pSI->GetLayers(ESIHelp, cRenderer);
			//HICON hIc = cRenderer.get();
			m_cImageList.AddIcon(hIc);
			DestroyIcon(hIc);
		}
		{
			//static float const f = 1.0f/256.0f;
			//static TPolyCoords const aVertices[] =
			//{
			//	{f*179, f*26},
			//	{f*230, f*77},
			//	{f*179, f*128},
			//	{f*230, f*179},
			//	{f*179, f*230},
			//	{f*128, f*179},
			//	{f*77, f*230},
			//	{f*26, f*179},
			//	{f*77, f*128},
			//	{f*26, f*77},
			//	{f*77, f*26},
			//	{f*128, f*77},
			//};
			//HICON hIc = IconFromPolygon(itemsof(aVertices), aVertices, agg::rgba8(0xf6, 0x97, 0x97, 0xff), agg::rgba8(0, 0, 0, 0xff), m_nIconSize, false);
			//m_cImageList.AddIcon(hIc);
			//DestroyIcon(hIc);

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(m_nIconSize);
			pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
			HICON hIc = cRenderer.get();
			m_cImageList.AddIcon(hIc);
			DestroyIcon(hIc);
		}
		{
			//static float const f = 1.0f/256.0f;
			//static TPolyCoords const aVertices[] =
			//{
			//	{f*24, f*126},
			//	{f*75, f*75},
			//	{f*120, f*120},
			//	{f*190, f*50},
			//	{f*241, f*101},
			//	{f*120, f*222},
			//};
			//HICON hIc = IconFromPolygon(itemsof(aVertices), aVertices, agg::rgba8(0x78, 0xf2, 0x78, 0xff), agg::rgba8(0, 0, 0, 0xff), m_nIconSize, false);
			//m_cImageList.AddIcon(hIc);
			//DestroyIcon(hIc);

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(m_nIconSize);
			pSI->GetLayers(ESIConfirm, cRenderer, IRTarget(0.8f));
			HICON hIc = cRenderer.get();
			m_cImageList.AddIcon(hIc);
			DestroyIcon(hIc);
		}
		{
			static float const f = 1.0f/256.0f;
			static TPolyCoords const aVertices1[] = { {f*16, f*48}, {0, 0}, {0, 0}, {f*240, f*48}, {0, 0}, {0, 0}, {f*240, f*64}, {0, 0}, {0, 0}, {f*16, f*64} };
			static TPolyCoords const aVertices2[] = { {f*16, f*120}, {0, 0}, {0, 0}, {f*240, f*120}, {0, 0}, {0, 0}, {f*240, f*136}, {0, 0}, {0, 0}, {f*16, f*136} };
			static TPolyCoords const aVertices3[] = { {f*16, f*192}, {0, 0}, {0, 0}, {f*240, f*192}, {0, 0}, {0, 0}, {f*240, f*208}, {0, 0}, {0, 0}, {f*16, f*208} };
			static TPolyCoords const aVertices4[] =
			{
				{f*203, f*56},
				{0, f*13.2548}, {-f*13.2548, 0}, {f*173, f*26},
				{f*13.2548, 0}, {0, f*13.2548}, {f*143, f*56},
				{0, -f*13.2548}, {f*13.2548, 0}, {f*173, f*86},
				{-f*13.2548, 0}, {0, -f*13.2548}, {f*203, f*56},
			};
			static TPolyCoords const aVertices5[] =
			{
				{f*109, f*128},
				{0, f*13.2548}, {-f*13.2548, 0}, {f*79, f*98},
				{f*13.2548, 0}, {0, f*13.2548}, {f*49, f*128},
				{0, -f*13.2548}, {f*13.2548, 0}, {f*79, f*158},
				{-f*13.2548, 0}, {0, -f*13.2548}, {f*109, f*128},
			};
			static TPolyCoords const aVertices6[] =
			{
				{f*168, f*200},
				{0, f*13.2548}, {-f*13.2548, 0}, {f*138, f*170},
				{f*13.2548, 0}, {0, f*13.2548}, {f*108, f*200},
				{0, -f*13.2548}, {f*13.2548, 0}, {f*138, f*230},
				{-f*13.2548, 0}, {0, -f*13.2548}, {f*168, f*200},
			};
			TIconPolySpec tPolySpec[6];
			tPolySpec[0].nVertices = itemsof(aVertices1);
			tPolySpec[0].pVertices = aVertices1;
			tPolySpec[0].interior = agg::rgba8(0x0, 0x0, 0x0, 0xff);
			tPolySpec[0].outline = agg::rgba8(0, 0, 0, 0);
			tPolySpec[1].nVertices = itemsof(aVertices2);
			tPolySpec[1].pVertices = aVertices2;
			tPolySpec[1].interior = agg::rgba8(0x0, 0x0, 0x0, 0xff);
			tPolySpec[1].outline = agg::rgba8(0, 0, 0, 0);
			tPolySpec[2].nVertices = itemsof(aVertices3);
			tPolySpec[2].pVertices = aVertices3;
			tPolySpec[2].interior = agg::rgba8(0x0, 0x0, 0x0, 0xff);
			tPolySpec[2].outline = agg::rgba8(0, 0, 0, 0);
			tPolySpec[3].nVertices = itemsof(aVertices4);
			tPolySpec[3].pVertices = aVertices4;
			tPolySpec[3].interior = agg::rgba8(0x67, 0x82, 0xeb, 0xff);
			tPolySpec[3].outline = agg::rgba8(0, 0, 0, 0xff);
			tPolySpec[4].nVertices = itemsof(aVertices5);
			tPolySpec[4].pVertices = aVertices5;
			tPolySpec[4].interior = agg::rgba8(0x67, 0x82, 0xeb, 0xff);
			tPolySpec[4].outline = agg::rgba8(0, 0, 0, 0xff);
			tPolySpec[5].nVertices = itemsof(aVertices6);
			tPolySpec[5].pVertices = aVertices6;
			tPolySpec[5].interior = agg::rgba8(0x67, 0x82, 0xeb, 0xff);
			tPolySpec[5].outline = agg::rgba8(0, 0, 0, 0xff);
			HICON hIc = IconFromPath(itemsof(tPolySpec), tPolySpec, m_nIconSize, false);
			m_cImageList.AddIcon(hIc);
			DestroyIcon(hIc);
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentLayeredImage> m_pDLI;
	CComPtr<ISharedStateManager> m_pFrame;
	CComBSTR m_bstrSyncID;
	LCID m_tLocaleID;

	CComPtr<IComparable> m_pItem;
	GUID m_tOpID;
	GUID m_tCachedOpID;
	CComBSTR m_bstrOpName;
	HICON m_hOpIcon;
	CComPtr<IConfig> m_pCfg;
	CComPtr<IConfigWnd> m_pWnd;

	bool m_bEnableUpdates;

	CFont m_cFont;
	ULONG m_nBarHeight;
	CToolBarCtrl m_wndHelp;
	CToolBarCtrl m_wndControls;
	CToolBarCtrl m_wndPresets;
	CImageList m_cImageList;
	int m_nIconSize;
	int m_nTitleIconSize;
	int m_nGapX;
	int m_nGapY;
};


// CDesignerViewFactoryImageEffectPanel

class ATL_NO_VTABLE CDesignerViewFactoryImageEffectPanel :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryImageEffectPanel>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryImageEffectPanel()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryImageEffectPanel)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryImageEffectPanel)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryImageEffectPanel)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;

		try
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Layered Image - Effect Panel[0405]Vrstvený obrázek - panel stylu");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return CConfigGUILayerIDDlg::CreateConfig(a_ppDefaultConfig);
	}
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
	{
		if (a_ppDVWnd == NULL)
			return E_POINTER;

		try
		{
			CComObject<CDesignerViewImageEffectPanel>* pWnd = NULL;
			CComObject<CDesignerViewImageEffectPanel>::CreateInstance(&pWnd);
			CComPtr<IDesignerView> pOut(pWnd);

			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return E_NOINTERFACE;

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);

			pWnd->Init(a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, pDLI, a_pFrame, cVal);

			*a_ppDVWnd = pOut.Detach();

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
	{
		CComPtr<IDocumentLayeredImage> p;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&p));
		if (p) a_pCallback->Used(__uuidof(IDocumentLayeredImage));
		else a_pCallback->Missing(__uuidof(IDocumentLayeredImage));
		return S_OK;
	}

};

// {C9A5A321-782F-4CE4-A125-460D8E72240F}
static const GUID CLSID_DesignerViewFactoryImageEffectPanel = {0xc9a5a321, 0x782f, 0x4ce4, {0xa1, 0x25, 0x46, 0xd, 0x8e, 0x72, 0x24, 0x0f}};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryImageEffectPanel, CDesignerViewFactoryImageEffectPanel)
