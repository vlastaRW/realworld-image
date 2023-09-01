
#include "stdafx.h"
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <math.h>
#include <XPGUI.h>
#include <RenderIcon.h>
#include <ContextMenuWithIcons.h>


// CDesignerViewLayerEffect

class ATL_NO_VTABLE CDesignerViewLayerEffect : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CChildWindowImpl<CDesignerViewLayerEffect, IDesignerView>,
	public Win32LangEx::CLangIndirectDialogImpl<CDesignerViewLayerEffect>,
	public CContextMenuWithIcons<CDesignerViewLayerEffect>,
	public CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>,
	public CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>
{
public:
	CDesignerViewLayerEffect() : m_bEnableUpdates(false), m_nToolbars(0), m_nOffset(0), m_nTotalHeight(0), m_nWheelDelta(0)
	{
		RWCoCreateInstance(m_pOM, __uuidof(OperationManager));
		RWCoCreateInstance(m_pDFI, __uuidof(DesignerFrameIconsManager));
	}
	~CDesignerViewLayerEffect()
	{
		m_cImageList.Destroy();
	}

	enum EScrollbarMode
	{
		ESMNever = 0,
		ESMAuto,
		ESMAlways,
	};

	void Init(ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGrp, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc)
	{
		m_pDoc = a_pDoc;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&m_pImage));
		CComBSTR bstrID;
		if (m_pImage)
		{
			m_pImage->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
			m_pImage->StatePrefix(&bstrID);
		}
		else
		{
			throw E_FAIL;
		}
		if (bstrID.Length())
		{
			m_strSelGrp = bstrID;
			m_strSelGrp.append(a_pszSyncGrp);
		}
		else
		{
			m_strSelGrp = a_pszSyncGrp;
		}
		if (!m_strSelGrp.empty())
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pSharedState = a_pFrame;
		}

		m_eScrollbars = ESMAlways;

		CComPtr<ISharedState> pState;
		CComBSTR bstr(m_strSelGrp.c_str());
		m_pSharedState->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		if (pState)
			m_pImage->StateUnpack(pState, &pItems);
		else
			m_pImage->ItemsEnum(NULL, &pItems);

		CComPtr<IComparable> pLayer;
		if (pItems)
			pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pLayer));

		if ((m_pLayer == NULL && pLayer != NULL) ||
			(pLayer == NULL && m_pLayer != NULL) ||
			(pLayer != NULL && m_pLayer != NULL && m_pLayer->Compare(pLayer) != S_OK))
		{
			m_pLayer = pLayer;
		}

		m_tLocaleID = a_tLocaleID;

		Win32LangEx::CLangIndirectDialogImpl<CDesignerViewLayerEffect>::Create(a_hParent);
		if (!IsWindow()) throw E_FAIL;

		MoveWindow(a_rcWindow);
		ShowWindow(SW_SHOW);

		Data2GUI();
		m_bEnableUpdates = true;
	}

	enum
	{
		IDC_TOOLBAR_BASE = 200,
		IDC_TOOLBAR_STEP = 10,
		IDC_STEPNAME_BASE = 201,
		IDC_STEPNAME_SKIP = 201,
		ID_STEP_MOVE_UP = 202,
		ID_STEP_MOVE_DOWN = 203,
		ID_STEP_DELETE = 204,
		LINE_HEIGHT = 20,
		MARGIN_X = 5,
	};

BEGIN_DIALOG_EX(0, 0, 100, 100, 0)
	DIALOG_FONT_AUTO()
	DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL|WS_VSCROLL)
	DIALOG_EXSTYLE(0)
END_DIALOG()

BEGIN_CONTROLS_MAP()
	//CONTROL_LTEXT(_T("[0409]Position:[0405]Pozice:"), IDC_POSITION_LABEL, 5, 7, 39, 8, WS_VISIBLE, 0)
	//CONTROL_EDITTEXT(IDC_POSITION, 45, 5, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	//CONTROL_LTEXT(_T("[0409]px[0405]px"), IDC_POSITION_UNIT, 109, 7, 10, 8, WS_VISIBLE, 0)
	//CONTROL_LTEXT(_T("[0409]Rotation:[0405]Rotace:"), IDC_ROTATION_LABEL, 5, 23, 39, 8, WS_VISIBLE, 0)
	//CONTROL_EDITTEXT(IDC_ROTATION, 45, 21, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	//CONTROL_LTEXT(_T("°"), IDC_ROTATION_UNIT, 109, 23, 10, 8, WS_VISIBLE, 0)
	//CONTROL_LTEXT(_T("[0409]Scale:[0405]Škála:"), IDC_ZOOM_LABEL, 5, 39, 39, 8, WS_VISIBLE, 0)
	//CONTROL_EDITTEXT(IDC_ZOOM, 45, 37, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	//CONTROL_LTEXT(_T("%"), IDC_ZOOM_UNIT, 79, 39, 10, 8, WS_VISIBLE, 0)
	//CONTROL_LTEXT(_T("[0409]Depth:[0405]Hloubka:"), IDC_DEPTH_LABEL, 90, 39, 39, 8, WS_VISIBLE, 0)
	//CONTROL_EDITTEXT(IDC_DEPTH, 130, 37, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
END_CONTROLS_MAP()

BEGIN_MSG_MAP(CDesignerViewLayerEffect)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	//MESSAGE_HANDLER(WM_HELP, OnHelp)
	//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnColor2Parent)
	//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnColor2Parent)
	//MESSAGE_HANDLER(WM_CTLCOLORBTN, OnColor2Parent)
	MESSAGE_HANDLER(WM_THEMECHANGED, OnThemeChanged)
	COMMAND_CODE_HANDLER(BN_CLICKED, OnClick)
	NOTIFY_CODE_HANDLER(TBN_DROPDOWN, OnButtonDropDown)
	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewLayerEffect>)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewLayerEffect)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IStructuredObserver, TStructuredChanges>::ObserverGet(), 0);
		if (m_pEffect)
			m_pEffect->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		try
		{
			if (m_pImage == NULL || !m_bEnableUpdates || m_strSelGrp.compare(a_tParams.bstrName) != 0)
				return;

			CComPtr<IComparable> pLayer;
			CComPtr<IEnumUnknowns> pItems;
			m_pImage->StateUnpack(a_tParams.pState, &pItems);

			if (pItems)
				pItems->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pLayer));

			if ((m_pLayer == NULL && pLayer != NULL) ||
				(pLayer == NULL && m_pLayer != NULL) ||
				(pLayer != NULL && m_pLayer != NULL && m_pLayer->Compare(pLayer) != S_OK))
			{
				m_pLayer = pLayer;
				m_bEnableUpdates = false;
				Data2GUI();
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
			if (!m_bEnableUpdates)
				return;

			if (m_pLayer)
			{
				for (ULONG i = 0; i < a_tChanges.nChanges; ++i)
				{
					if (a_tChanges.aChanges[i].nChangeFlags&ESCContent &&
						m_pLayer->Compare(a_tChanges.aChanges[i].pItem) == S_OK)
					{
						m_bEnableUpdates = false;
						bool bChange = Data2GUI();
						m_bEnableUpdates = true;
						if (bChange)
							GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
					}
				}
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
			if (!m_bEnableUpdates)
				return;

			if (m_pLayer)
			{
				m_bEnableUpdates = false;
				GUI2Data();
				//m_pImage->LayerEffectSet(m_pLayer, m_pEffect);
				m_bEnableUpdates = true;
			}
		}
		catch (...)
		{
		}
	}

protected:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// TODO: ?? set control styles (no auto check, drop list combo, want return edit)

		//bool bContxtTips = true;
		//CComPtr<IGlobalConfigManager> pMgr;
		//RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		//if (pMgr)
		//{
		//	CComPtr<IConfig> pCfg;
		//	// hacks: copied CLSID and CFGVAL
		//	static CLSID const tID = {0x2e85563c, 0x4ff0, 0x4820, {0xa8, 0xba, 0x1b, 0x47, 0x63, 0xab, 0xcc, 0x1c}}; // CLSID_GlobalConfigMainFrame
		//	pMgr->Config(tID, &pCfg);
		//	CConfigValue cVal;
		//	if (pCfg) pCfg->ItemValueGet(CComBSTR(L"CtxHelpTips"), &cVal);
		//	if (cVal.TypeGet() == ECVTBool) bContxtTips = cVal;
		//}

		//if (m_pConfig)
		//	m_pConfig->ObserverIns(ObserverGet(), 0);

		m_theme.OpenThemeData(m_hWnd, _T("REBAR"));

		RECT rcSize = {0, 0, MARGIN_X, LINE_HEIGHT};
		MapDialogRect(&rcSize);
		m_nLineHeight = rcSize.bottom;
		m_nMarginX = rcSize.right;

		m_nIconSize = XPGUI::GetSmallIconSize();
		m_cImageList.Create(m_nIconSize, m_nIconSize, XPGUI::GetImageListColorFlags(), 8, 8);
		{
			static const TPolyCoords aVtx[] =
			{
				//{161, 217}, {95, 217}, {128, 160}, {161, 217},
				////{199, 172}, {218, 172}, {218, 161}, {248, 187}, {218, 213}, {218, 202}, {182, 202}, {144, 135}, {112, 135}, {74, 202}, {23, 202}, {23, 172}, {57, 172}, {95, 105}, {161, 105}, {199, 172},
				////{206, 162}, {218, 162}, {218, 141}, {248, 182}, {218, 223}, {218, 202}, {182, 202}, {144, 135}, {112, 135}, {74, 202}, {23, 202}, {23, 162}, {50, 162}, {88, 95}, {168, 95}, {206, 162},
				//{206, 162}, {218, 162}, {238, 182}, {218, 202}, {182, 202}, {144, 135}, {112, 135}, {74, 202}, {23, 202}, {43, 182}, {23, 162}, {50, 162}, {88, 95}, {168, 95}, {206, 162},
				{0, 64}, {56, 64}, {104, 128}, {56, 192}, {0, 192}, {0, 64},
				{152, 64}, {256, 64}, {256, 192}, {152, 192}, {200, 128}, {152, 64},
			};

			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, m_nIconSize, true);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
			//TPolyCoords aVtx[4+4*10];
			//TPolyCoords* p = aVtx;
		}
		{
			static int const arc = 7;
			static int const extra = 5;
			TPolyCoords aVtx[5*4+1+10];
			TPolyCoords* p = aVtx;
			for (int i = 0; i < 5; ++i)
			{
				float const a1 = (i*0.4f+0.05f)*3.14159265359f;
				float const a2 = (i*0.4f+0.13f)*3.14159265359f;
				float const a3 = (i*0.4f+0.27f)*3.14159265359f;
				float const a4 = (i*0.4f+0.35f)*3.14159265359f;
				p->fX = 0.5f + cosf(a1)*0.25f;
				p->fY = 0.5f + sinf(a1)*0.25f;
				++p;
				p->fX = 0.5f + cosf(a2)*0.4f;
				p->fY = 0.5f + sinf(a2)*0.4f;
				++p;
				p->fX = 0.5f + cosf(a3)*0.4f;
				p->fY = 0.5f + sinf(a3)*0.4f;
				++p;
				p->fX = 0.5f + cosf(a4)*0.25f;
				p->fY = 0.5f + sinf(a4)*0.25f;
				++p;
			}
			*p++ = *aVtx;
			for (int i = 0; i < 10; ++i)
			{
				float const a1 = (i*0.2f+0.05f)*3.14159265359f;
				p->fX = 0.5f + cosf(a1)*0.11f;
				p->fY = 0.5f + sinf(a1)*0.11f;
				++p;
			}
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, m_nIconSize, true);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		m_szButtons.cy = m_nIconSize;
		m_szButtons.cx = m_szButtons.cy*2;

		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		//lf.lfHeight += lf.lfHeight>>1;
		m_font.CreateFontIndirect(&lf);

		UpdateControls(true);
		//m_bEnableEditUpdates = true;

		a_bHandled = FALSE;
		return 1;
	}
	LRESULT OnEraseBkgnd(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		HDC hDC = (HDC)a_wParam;
		RECT rc;
		GetClientRect(&rc);
		int nMaxScroll = max(m_nTotalHeight-rc.bottom, 0);
		int y = -min(m_nOffset, nMaxScroll);
		RECT rc2 = rc;
		for (CItems::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
		{
			rc2.top = y;
			rc2.bottom = y+m_nLineHeight;
			if (rc2.top < rc.bottom && rc2.bottom > 0)
			{
				if (!m_theme.IsThemeNull())
				{
					m_theme.DrawThemeBackground(hDC, RP_BACKGROUND, 0, &rc2);
				}
				else
				{
					FillRect(hDC, &rc2, (HBRUSH)::GetStockObject(WHITE_BRUSH));
				}
				if (i->strName.Length())
				{
					int const iImg = GetImageIndex(i->tID);
					int const prev = rc2.left;
					rc2.left = m_nMarginX;
					if (iImg > 0)
					{
						m_cImageList.Draw(hDC, iImg, rc2.left, (rc2.bottom+rc2.top-m_nIconSize)>>1, ILD_TRANSPARENT);
						rc2.left += m_nIconSize*5/4;
					}
					int mode = SetBkMode(hDC, TRANSPARENT);
					HGDIOBJ font = SelectObject(hDC, m_font);
					DrawText(hDC, i->strName, -1, &rc2, DT_LEFT|DT_END_ELLIPSIS|DT_SINGLELINE|DT_VCENTER);
					SelectObject(hDC, font);
					SetBkMode(hDC, mode);
					rc2.left = prev;
				}
			}
			rc2.top = rc2.bottom;
			rc2.bottom += i->nHeight;
			y = rc2.bottom;
		}
		if (y < rc.bottom)
		{
			if (y > rc.top)
				rc.top = y;
			COLORREF clrOld = ::SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
			::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
			::SetBkColor(hDC, clrOld);
		}
		return 1;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		//if (m_pConfig)
		//	m_pConfig->ObserverDel(ObserverGet(), 0);

		//if (m_wndToolTip.IsWindow())
		//	m_wndToolTip.DestroyWindow();
		for (size_t i = 0; i < m_cItems.size(); ++i)
		{
			CWindow wnd(GetDlgItem(IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP));
			wnd.DestroyWindow();
			if (m_cItems[i].pWindow)
				m_cItems[i].pWindow->Destroy();
		}
		m_cItems.clear();

		m_theme.CloseThemeData();

		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnThemeChanged(UINT UNREF(uMsg), WPARAM UNREF(wParam), LPARAM UNREF(lParam), BOOL& bHandled)
	{
		m_theme.CloseThemeData();
		m_theme.OpenThemeData(m_hWnd, _T("REBAR"));
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		SIZE sz = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};

		RepositionControls(sz);
		return 0;
	}
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RECT rc;
		GetClientRect(&rc);
		int nMaxScroll = max(m_nTotalHeight-rc.bottom, 0);
		switch (LOWORD(wParam))
		{
		case SB_BOTTOM:		SetScrollPos(SB_VERT, m_nOffset = rc.bottom-m_nTotalHeight); break;
		case SB_TOP:		SetScrollPos(SB_VERT, m_nOffset = 0); break;
		case SB_LINEUP:		SetScrollPos(SB_VERT, m_nOffset = max(0, m_nOffset-m_nLineHeight)); break;
		case SB_LINEDOWN:	SetScrollPos(SB_VERT, m_nOffset = min(nMaxScroll, m_nOffset+m_nLineHeight)); break;
		case SB_PAGEUP:		SetScrollPos(SB_VERT, m_nOffset = max(0, m_nOffset-rc.bottom)); break;
		case SB_PAGEDOWN:	SetScrollPos(SB_VERT, m_nOffset = min(nMaxScroll, m_nOffset+rc.bottom)); break;
		case SB_THUMBTRACK:	SetScrollPos(SB_VERT, m_nOffset = HIWORD(wParam)); break;
		default:			return TRUE;
		}
		SIZE sz = {rc.right, rc.bottom};
		RepositionControls(sz);
		return TRUE;
	}
	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);
		int nDelta = m_nWheelDelta < 0 ? -((-m_nWheelDelta)>>2) : (m_nWheelDelta>>2);
		if (nDelta)
		{
			m_nWheelDelta -= nDelta<<2;

			int nOldOffset = m_nOffset;
			RECT rc;
			GetClientRect(&rc);
			int nMax = max(0, m_nTotalHeight-rc.bottom);
			int nProposed = m_nOffset-nDelta;
			if (nProposed < 0)
			{
				m_nOffset = 0;
			}
			else if (nProposed > nMax)
			{
				m_nOffset = nMax;
			}
			else
			{
				m_nOffset = nProposed;
			}

			if (m_nOffset != nOldOffset)
			{
				SIZE sz = {rc.right, rc.bottom};
				RepositionControls(sz);
			}
		}
		return 0;
	}
	LRESULT OnClick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (a_wID >= IDC_STEPNAME_BASE && a_wID < IDC_STEPNAME_BASE+m_cItems.size()*IDC_TOOLBAR_STEP)
		{
			UINT nItem = (a_wID-IDC_STEPNAME_BASE)/IDC_TOOLBAR_STEP;
			switch  (a_wID-nItem*IDC_TOOLBAR_STEP)
			{
			case IDC_STEPNAME_SKIP:
				{
					m_cItems[nItem].bSkip = !m_cItems[nItem].bSkip;
					CToolBarCtrl wnd(GetDlgItem(IDC_TOOLBAR_BASE+nItem*IDC_TOOLBAR_STEP));
					wchar_t sz[40];
					swprintf(sz, L"Effect\\SeqSteps\\%08x\\SeqSkipStep", nItem);
					CComBSTR bstr(sz);
					CConfigValue val(m_cItems[nItem].bSkip);
					m_pEffect->ItemValuesSet(1, &(bstr.m_str), val);
					GUI2Data();
				}
				break;
			case ID_STEP_MOVE_UP:
				if (nItem > 0)
				{
					CComPtr<IConfig> pSubCfg;
					m_pEffect->SubConfigGet(CComBSTR(L"Effect\\SeqSteps"), &pSubCfg);
					CComQIPtr<IConfigVector> pV(pSubCfg);
					pV->Swap(nItem-1, nItem);
					//GUI2Data();
					UpdateControls(false);
					RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
				}
				break;
			case ID_STEP_MOVE_DOWN:
				if (nItem < m_cItems.size())
				{
					CComPtr<IConfig> pSubCfg;
					m_pEffect->SubConfigGet(CComBSTR(L"Effect\\SeqSteps"), &pSubCfg);
					CComQIPtr<IConfigVector> pV(pSubCfg);
					pV->Swap(nItem, nItem+1);
					//GUI2Data();
					UpdateControls(false);
					RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
				}
				break;
			case ID_STEP_DELETE:
				if (nItem < m_cItems.size())
				{
					CComPtr<IConfig> pSubCfg;
					CComBSTR bstrSteps(L"Effect\\SeqSteps");
					m_pEffect->SubConfigGet(bstrSteps, &pSubCfg);
					CComQIPtr<IConfigVector> pV(pSubCfg);
					m_bEnableUpdates = false;
					if (nItem+1 != m_cItems.size())
						pV->Move(nItem, m_cItems.size()-1);
					m_pEffect->ItemValuesSet(1, &(bstrSteps.m_str), CConfigValue(LONG(m_cItems.size()-1)));
					m_bEnableUpdates = true;
					GUI2Data();
					UpdateControls(false);
					RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
				}
				break;
			}
		}
		else
		{
			a_bHandled = FALSE;
		}
		return 0;
	}
	LRESULT OnButtonDropDown(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
	{
		NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
		int nItem = (pTB->iItem-IDC_STEPNAME_BASE)/IDC_TOOLBAR_STEP;
		int nCmd = (pTB->iItem-IDC_STEPNAME_BASE)-(nItem*IDC_TOOLBAR_STEP);
		if (nItem >= 0 && size_t(nItem) < m_cItems.size())
		{
			CToolBarCtrl wndToolBar(GetDlgItem(IDC_TOOLBAR_BASE+nItem*IDC_TOOLBAR_STEP));

			POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
			wndToolBar.ClientToScreen(&ptBtn);

			Reset(m_cImageList);

			CMenu cMenu;
			cMenu.CreatePopupMenu();

			UINT nMenuID = 1;
			AddItem(cMenu, ID_STEP_MOVE_UP+IDC_TOOLBAR_STEP*nItem, L"Move up", -1, 0);
			AddItem(cMenu, ID_STEP_MOVE_DOWN+IDC_TOOLBAR_STEP*nItem, L"Move down", -1, 0);
			cMenu.AppendMenu(MFT_SEPARATOR);
			AddItem(cMenu, ID_STEP_DELETE+IDC_TOOLBAR_STEP*nItem, L"Remove", -1, 0);

			TPMPARAMS tPMParams;
			ZeroMemory(&tPMParams, sizeof tPMParams);
			tPMParams.cbSize = sizeof tPMParams;
			tPMParams.rcExclude = pTB->rcButton;
			::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
			UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY/*|TPM_RETURNCMD*/, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
			if (nSelection != 0)
			{
				//CWaitCursor cWait;
				//m_pContext->ResetErrorMessage();
				//HandleOperationResult(m_cCommands[nSelection-1]->Execute(m_hWnd, m_tLocaleID));
			}
		}
		return TBDDRET_DEFAULT;
	}

	void RepositionControls(SIZE sz)
	{
		BOOL bVisible = IsWindowVisible();
		if (bVisible)
			SetRedraw(FALSE);

		int nMaxScroll = max(m_nTotalHeight-sz.cy, 0);
		int y = -min(m_nOffset, nMaxScroll);
		for (size_t i = 0; i < m_cItems.size(); ++i)
		{
			CWindow wnd(GetDlgItem(IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP));
			RECT rc = {sz.cx-m_nMarginX-m_szButtons.cx, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1), sz.cx-m_nMarginX, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1)+m_szButtons.cy};
			wnd.SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
			if (m_cItems[i].pWindow)
			{
				SIZE sz2 = {sz.cx, 100};
				m_cItems[i].pWindow->OptimumSize(&sz2);
				rc.left = 0;
				rc.right = sz.cx;
				rc.top = y+m_nLineHeight;
				rc.bottom = rc.top+sz2.cy;
				m_cItems[i].pWindow->Move(&rc);
				m_cItems[i].nHeight = sz2.cy;
			}
			y = rc.bottom;
		}
		m_nTotalHeight = y+min(m_nOffset, nMaxScroll);
		SCROLLINFO tSI;
		ZeroMemory(&tSI, sizeof tSI);
		tSI.cbSize = sizeof tSI;
		tSI.fMask = SIF_PAGE|SIF_DISABLENOSCROLL|SIF_POS|SIF_RANGE;
		tSI.nPos = m_nOffset;
		tSI.nMin = 0;
		tSI.nMax = m_nTotalHeight;
		tSI.nPage = sz.cy;
		SetScrollInfo(SB_VERT, &tSI, TRUE);

		if (bVisible)
		{
			SetRedraw(TRUE);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
	}

	LRESULT OnColor2Parent(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		for (CItems::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
		{
			if (i->pWindow)
			{
				HRESULT hRes = i->pWindow->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
				if (hRes == S_OK)
					return S_OK;
			}
		}
		return S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OnIdle)()
	{
		RECT rc = {0, 0, 0, 0};
		if (m_hWnd == NULL)
			return S_FALSE;
		GetClientRect(&rc);
		if (rc.bottom <= 0 || rc.right  <= 0)
			return S_FALSE;
		bool bChange = false;
		for (size_t i = 0; i < m_cItems.size(); ++i)
		{
			SIZE sz = {rc.right, m_cItems[i].nHeight};
			m_cItems[i].pWindow->OptimumSize(&sz);
			if (m_cItems[i].nHeight != sz.cy)
				bChange = true;
		}
		if (bChange)
		{
			SIZE sz = {rc.right, rc.bottom};
			RepositionControls(sz);
		}
		return S_FALSE;
	}
	STDMETHOD(OnDeactivate)(BOOL /*a_bCancelChanges*/) {return S_OK;}
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter /*a_eFilter*/, IEnumUnknownsInit* a_pInterfaces)
	{
		try
		{
			CComPtr<IUnknown> p;
			QueryInterface(a_iid, reinterpret_cast<void**>(&p));
			if (p == NULL)
				return S_FALSE;
			return a_pInterfaces->Insert(p);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		if (a_pSize == NULL)
			return E_POINTER;
		SIZE sz = {100, 100};
		if (a_pSize->cx < sz.cx) a_pSize->cx = sz.cx;
		if (a_pSize->cy < sz.cy) a_pSize->cy = sz.cy;
		return S_OK;
		//try
		//{
		//	SIZE sz;
		//	if (!Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, &sz, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)))
		//		return E_FAIL;
		//	if (a_pSize->cx < sz.cx) a_pSize->cx = sz.cx;
		//	if (a_pSize->cy < sz.cy) a_pSize->cy = sz.cy;
		//	return S_OK;
		//}
		//catch (...)
		//{
		//	return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		//}
	}
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		return S_OK;
	}

private:
	void GUI2Data()
	{
		if (m_pImage && m_pLayer && m_pEffect)
		{
			m_pImage->LayerEffectSet(m_pLayer, m_pEffect);
		}
	}
	bool Data2GUI()
	{
		CComPtr<IConfig> pEffect;
		m_pImage->LayerEffectGet(m_pLayer, &pEffect, NULL);
		if (m_pEffect == pEffect)
			return false;
		if (CompareConfigValues(m_pEffect, pEffect) == S_OK)
			return false;
		if (m_pEffect != NULL)
			m_pEffect->ObserverDel(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		m_pEffect = pEffect;
		if (m_pEffect != NULL)
			m_pEffect->ObserverIns(CObserverImpl<CDesignerViewLayerEffect, IConfigObserver, IUnknown*>::ObserverGet(), 0);

		if (m_hWnd != NULL)
		{
			UpdateControls(false);
			RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASENOW | RDW_ALLCHILDREN);
		}
		return true;
	}
	void UpdateControls(bool a_bResetScroll)
	{
		CItems cItems;
		if (m_pEffect)
			ReadOperations(m_pEffect, cItems);
		std::swap(m_cItems, cItems);

		RECT rcClient = {0, 0, 0, 0};
		GetClientRect(&rcClient);
		if (a_bResetScroll)
			m_nOffset = 0;
		int nMaxScroll = max(m_nTotalHeight-rcClient.bottom, 0);
		int nEffectiveOffset = m_nOffset;//min(, nMaxScroll);
		int y = -nEffectiveOffset;
		for (size_t i = 0; i < m_cItems.size(); ++i)
		{
			if (i >= m_nToolbars)
			{
				CToolBarCtrl wndLine;
				RECT rc = {rcClient.right-m_nMarginX-m_szButtons.cx, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1), rcClient.right-m_nMarginX, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1)+m_szButtons.cy};
				wndLine.Create(m_hWnd, rc, NULL, TBSTYLE_TRANSPARENT|TBSTYLE_LIST|TBSTYLE_FLAT|TBSTYLE_TOOLTIPS|CCS_NOMOVEY|CCS_NORESIZE|CCS_NOPARENTALIGN|CCS_NODIVIDER|WS_TABSTOP|WS_CHILD|WS_VISIBLE, 0, IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP);
				wndLine.SetImageList(m_cImageList);
				wndLine.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
				wndLine.SetButtonStructSize(sizeof(TBBUTTON));
				TBBUTTON aButtons[] =
				{
					{0, IDC_STEPNAME_BASE+i*IDC_TOOLBAR_STEP, m_cItems[i].bSkip ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED, BTNS_BUTTON|BTNS_CHECK, TBBUTTON_PADDING, 0, (INT_PTR)L"Skip"},
					{1, IDC_STEPNAME_BASE+1+i*IDC_TOOLBAR_STEP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, (INT_PTR)L"Menu"},
				};
				wndLine.AddButtons(itemsof(aButtons), aButtons);
				RECT rc1;
				wndLine.GetItemRect(1, &rc1);
				if (m_szButtons.cx != rc1.right || m_szButtons.cy != rc1.bottom)
				{
					m_szButtons.cx = rc1.right;
					m_szButtons.cy = rc1.bottom;
					rc.left = rcClient.right-m_nMarginX-m_szButtons.cx;
					rc.top = y+(m_nLineHeight>>1)-(m_szButtons.cy>>1);
					rc.bottom = y+(m_nLineHeight>>1)-(m_szButtons.cy>>1)+m_szButtons.cy;
					wndLine.MoveWindow(&rc);
				}
				CComPtr<IConfigWnd> pWnd;
				RWCoCreateInstance(pWnd, __uuidof(AutoConfigWnd));
				RECT rcSubWnd = {rcClient.left, y+m_nLineHeight, rcClient.right, y+m_nLineHeight+100};
				pWnd->Create(m_hWnd, &rcSubWnd, 0, m_tLocaleID, FALSE, ECWBMMargin);
				pWnd->ConfigSet(m_cItems[i].pConfig, ECPMSimplified);
				SIZE sz = {rcSubWnd.right-rcSubWnd.left, 100};
				pWnd->OptimumSize(&sz);
				rcSubWnd.bottom = rcSubWnd.top+sz.cy;
				pWnd->Move(&rcSubWnd);
				pWnd->Show(TRUE);
				m_cItems[i].pWindow = pWnd;
				m_cItems[i].nHeight = sz.cy;
				y = rcSubWnd.bottom;
			}
			else
			{
				CToolBarCtrl wndLine(GetDlgItem(IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP));
				wndLine.DeleteButton(1);
				wndLine.DeleteButton(0);
				TBBUTTON aButtons[] =
				{
					{0, IDC_STEPNAME_BASE+i*IDC_TOOLBAR_STEP, m_cItems[i].bSkip ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED, BTNS_BUTTON|BTNS_CHECK, TBBUTTON_PADDING, 0, (INT_PTR)L"Skip"},
					{1, IDC_STEPNAME_BASE+1+i*IDC_TOOLBAR_STEP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, (INT_PTR)L"Menu"},
				};
				wndLine.AddButtons(itemsof(aButtons), aButtons);
				RECT rc = {rcClient.right-m_nMarginX-m_szButtons.cx, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1), rcClient.right-m_nMarginX, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1)+m_szButtons.cy};
				wndLine.MoveWindow(&rc);
				m_cItems[i].pWindow = cItems[i].pWindow;
				cItems[i].pWindow = NULL;
				m_cItems[i].pWindow->ConfigSet(m_cItems[i].pConfig, ECPMSimplified);
				SIZE sz = {rcClient.right, 100};
				m_cItems[i].pWindow->OptimumSize(&sz);
				RECT rcSubWnd = {0, y+m_nLineHeight, rcClient.right, y+m_nLineHeight+sz.cy};
				m_cItems[i].pWindow->Move(&rcSubWnd);
				m_cItems[i].nHeight = sz.cy;
				y = rcSubWnd.bottom;
			}
		}
		for (size_t i = m_cItems.size(); i < cItems.size(); ++i)
		{
			CWindow wnd(GetDlgItem(IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP));
			wnd.DestroyWindow();
			if (cItems[i].pWindow)
				cItems[i].pWindow->Destroy();
		}
		m_nToolbars = m_cItems.size();

		m_nTotalHeight = y+nEffectiveOffset;
		nMaxScroll = max(m_nTotalHeight-rcClient.bottom, 0);
		int nOffset = min(m_nOffset, nMaxScroll);
		if (nEffectiveOffset != nOffset)
		{
			y = -nOffset;
			for (size_t i = 0; i < m_cItems.size(); ++i)
			{
				CWindow wnd(GetDlgItem(IDC_TOOLBAR_BASE+i*IDC_TOOLBAR_STEP));
				RECT rc = {rcClient.right-m_nMarginX-m_szButtons.cx, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1), rcClient.right-m_nMarginX, y+(m_nLineHeight>>1)-(m_szButtons.cy>>1)+m_szButtons.cy};
				wnd.SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOACTIVATE);
				if (m_cItems[i].pWindow)
				{
					rc.left = 0;
					rc.right = rcClient.right;
					rc.top = y+m_nLineHeight;
					rc.bottom = rc.top+m_cItems[i].nHeight;
					m_cItems[i].pWindow->Move(&rc);
				}
				y = rc.bottom;
			}
		}

		SCROLLINFO tSI;
		ZeroMemory(&tSI, sizeof tSI);
		tSI.cbSize = sizeof tSI;
		tSI.fMask = SIF_PAGE|SIF_DISABLENOSCROLL|SIF_POS|SIF_RANGE;
		tSI.nPos = m_nOffset;
		tSI.nMin = 0;
		tSI.nMax = m_nTotalHeight;
		tSI.nPage = rcClient.bottom;
		SetScrollInfo(SB_VERT, &tSI, TRUE);
	}

	HRESULT CompareConfigValues(IConfig* a_p1, IConfig* a_p2)
	{
		if (a_p1 == NULL || a_p2 == NULL)
			return E_POINTER;

		CComPtr<IEnumStrings> pES1;
		a_p1->ItemIDsEnum(&pES1);
		ULONG nItems1 = 0;
		pES1->Size(&nItems1);
		CComPtr<IEnumStrings> pES2;
		a_p2->ItemIDsEnum(&pES2);
		ULONG nItems2 = 0;
		pES2->Size(&nItems2);
		if (nItems1 != nItems2)
			return S_FALSE;

		HRESULT hr = S_OK;
		CAutoVectorPtr<BSTR> aIDs(new BSTR[nItems1]);
		pES1->GetMultiple(0, nItems1, aIDs);
		for (ULONG i = 0; i < nItems1; i++)
		{
			CConfigValue c1;
			CConfigValue c2;
			a_p1->ItemValueGet(aIDs[i], &c1);
			a_p2->ItemValueGet(aIDs[i], &c2);
			if (c1 != c2)
			{
				hr = S_FALSE;
				break;
			}
		}

		for (ULONG i = 0; i < nItems1; i++)
		{
			SysFreeString(aIDs[i]);
		}

		return hr;
	}

private:
	struct SItem
	{
		GUID tID;
		CComPtr<IConfig> pConfig;
		CComBSTR strName;
		CComPtr<IConfigWnd> pWindow;
		bool bSkip;
		int nHeight;
	};
	typedef std::vector<SItem> CItems;

	void GetStepName(CLSID tID, CComPtr<IConfig> pOpCfg, CComBSTR& bstrName)
	{
		CComPtr<IDocumentOperation> pOp;
		RWCoCreateInstance(pOp, tID);
		CComQIPtr<IConfigDescriptor> pOA(pOp);
		if (pOA)
		{
			CComPtr<ILocalizedString> pDisplayName;
			pOA->Name(m_pOM, pOpCfg, &pDisplayName);
			if (pDisplayName)
				pDisplayName->GetLocalized(m_tLocaleID, &bstrName);
		}
		if (bstrName.Length() == 0)
		{
			CComPtr<ILocalizedString> pStr;
			if (pOp) pOp->NameGet(NULL, &pStr);
			if (pStr) pStr->GetLocalized(m_tLocaleID, &bstrName);
		}
		if (bstrName.Length() == 0)
			bstrName = L"Unknown effect";
	}

	void ReadOperations(IConfig* pDup, CItems& cItems)
	{
		CComBSTR bstrEffect(L"Effect");
		CConfigValue cEffect;
		pDup->ItemValueGet(bstrEffect, &cEffect);
		if (IsEqualGUID(cEffect, __uuidof(DocumentOperationNULL)))
		{
		}
		else if (IsEqualGUID(cEffect, __uuidof(DocumentOperationSequence)))
		{
			CComPtr<IConfig> pSeq;
			pDup->SubConfigGet(bstrEffect, &pSeq);
			CConfigValue cSteps;
			CComBSTR bstrSteps(L"SeqSteps");
			pSeq->ItemValueGet(bstrSteps, &cSteps);
			CComPtr<IConfig> pSteps;
			pSeq->SubConfigGet(bstrSteps, &pSteps);
			for (LONG i = 0; i < cSteps.operator LONG(); ++i)
			{
				OLECHAR sz[32];
				swprintf(sz, L"%08x\\SeqOperation", i);
				CComBSTR bstrSubID(sz);
				CConfigValue cStep;
				pSteps->ItemValueGet(bstrSubID, &cStep);
				SItem tOp;
				tOp.tID = cStep.operator const GUID &();
				pSteps->SubConfigGet(bstrSubID, &tOp.pConfig);
				GetStepName(tOp.tID, tOp.pConfig, tOp.strName);
				swprintf(sz, L"%08x\\SeqSkipStep", i);
				CConfigValue cSkip;
				pSteps->ItemValueGet(CComBSTR(sz), &cSkip);
				tOp.bSkip = cSkip.TypeGet() == ECVTBool && cSkip.operator bool();
				cItems.push_back(tOp);
			}
		}
		else
		{
			SItem tOp;
			tOp.bSkip = false;
			tOp.tID = cEffect.operator const GUID &();
			pDup->SubConfigGet(bstrEffect, &tOp.pConfig);
			GetStepName(tOp.tID, tOp.pConfig, tOp.strName);
			cItems.push_back(tOp);
		}
	}

struct lessCLSID
{
	bool operator()(GUID const& a_1, GUID const& a_2) const
	{
		return a_1.Data1 < a_2.Data1 ||
			(a_1.Data1 == a_2.Data1 && (a_1.Data2 < a_2.Data2 ||
			(a_1.Data2 == a_2.Data2 && (a_1.Data3 < a_2.Data3 ||
			(a_1.Data3 == a_2.Data3 && memcmp(a_1.Data4, a_2.Data4, sizeof a_1.Data4)<0)))));
	}
};
	typedef std::map<GUID, int, lessCLSID> CIconIndexMap;
	int GetImageIndex(GUID opID)
	{
		CIconIndexMap::const_iterator i = m_cIconIndexMap.find(opID);
		if (i != m_cIconIndexMap.end())
			return i->second;
		HICON hIcon = NULL;
		m_pDFI->GetIcon(opID, m_nIconSize, &hIcon);
		if (hIcon == NULL)
			return I_IMAGENONE;
		int nIcon = m_cImageList.AddIcon(hIcon);
		DestroyIcon(hIcon);
		m_cIconIndexMap[opID] = nIcon;
		return nIcon;
	}

private:
	CComPtr<IOperationManager> m_pOM;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentLayeredImage> m_pImage;
	CComPtr<IComparable> m_pLayer;
	bool m_bEnableUpdates;
	std::wstring m_strSelGrp;
	CComPtr<IConfig> m_pEffect;

	int m_nMarginX;
	int m_nLineHeight;
	int m_nIconSize;
	SIZE m_szButtons;
	CItems m_cItems;
	size_t m_nToolbars;
	int m_nExpanded;
	int m_nOffset;
	int m_nTotalHeight;
	int m_nWheelDelta;

	EScrollbarMode m_eScrollbars;

	CComPtr<IDesignerFrameIcons> m_pDFI;
	CIconIndexMap m_cIconIndexMap;
	CImageList m_cImageList;
	CTheme m_theme;
	CFont m_font;
};


// CDesignerViewFactoryLayerEffect

class ATL_NO_VTABLE CDesignerViewFactoryLayerEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryLayerEffect>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryLayerEffect()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryLayerEffect)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryLayerEffect)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryLayerEffect)
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
			*a_ppName = new CMultiLanguageString(L"[0409]Layered Image - Layer Style[0405]Vrstvený obrázek - styl vrstvy");
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
			CComObject<CDesignerViewLayerEffect>* pWnd = NULL;
			CComObject<CDesignerViewLayerEffect>::CreateInstance(&pWnd);
			CComPtr<IDesignerView> pOut(pWnd);

			CConfigValue cSyncGroup;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSyncGroup);
			pWnd->Init(a_pFrame, cSyncGroup.operator BSTR(), a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc);

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

// {AB2A7282-01BC-4d20-B470-71F8E2D51764}
static const GUID CLSID_DesignerViewFactoryLayerEffect = {0xab2a7282, 0x1bc, 0x4d20, {0xb4, 0x70, 0x71, 0xf8, 0xe2, 0xd5, 0x17, 0x64}};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryLayerEffect, CDesignerViewFactoryLayerEffect)
