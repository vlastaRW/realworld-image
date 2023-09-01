
#include "stdafx.h"
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include "../RWViewImage/RWViewImage.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <math.h>
#include <XPGUI.h>
#include <RenderIcon.h>
#include <ContextMenuWithIcons.h>
#define PERFMON
#include <RasterImagePainting.h>
#include <RasterImageMouseInput.h>

#include <math.h>
#include <float.h>
#include <XPGUI.h>
#include <MultiLanguageString.h>
#include <SharedStateUndo.h>
#include "HandleCoordinates.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_conv_dash.h>
#include <agg_span_allocator.h>

#include <agg_ellipse.h>


static const OLECHAR CFGID_IEEEDIT_VIEWPORTSYNC[] = L"ViewportSyncGroup";
static const OLECHAR CFGID_IEEEDIT_CONTEXTMENU[] = L"ContextMenu";

// ----- image painting -----

static int const MARGIN_TOP = 3;
static int const MARGIN_BOTTOM = 3;
static int const MARGIN_LEFT = 3;
static int const MARGIN_RIGHT = 3;


class ATL_NO_VTABLE CDesignerViewImageEffectEditor;

struct SGestureOperationEffect
{
	wchar_t const* pszName;
	GUID tID;
	HRESULT (CDesignerViewImageEffectEditor::*pMemFun)(IConfig* a_pCfg);
	void (*pfnConfig)(IConfigWithDependencies* a_pConfig);
};

//extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHTOOLID[] = L"ToolID";
//extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHSTYLEID[] = L"StyleID";
//extern __declspec(selectany) OLECHAR const CFGID_GESTUREOUTLINEDDELTA[] = L"OutlineDelta";
//
//extern __declspec(selectany) OLECHAR const GESTURESWITCHTOOLID_NAME[] = L"[0409]Tool ID[0405]ID nástroje";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHTOOLID_DESC[] = L"[0409]Identifier of the active draw tool.[0405]Identifikátor activního kreslicího nástroje.";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHSTYLEID_NAME[] = L"[0409]Style ID[0405]ID stylu";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHSTYLEID_DESC[] = L"[0409]Identifier of the active fill style.[0405]Identifikátor aktivního stylu vyplňování.";
//
//template<OLECHAR const* t_pszID, OLECHAR const* t_pszName, OLECHAR const* t_pszDesc>
//inline void GestCfgToolOrStyle(IConfigWithDependencies* a_pConfig)
//{
//	a_pConfig->ItemInsSimple(CComBSTR(t_pszID), CMultiLanguageString::GetAuto(t_pszName), CMultiLanguageString::GetAuto(t_pszDesc), CConfigValue(L""), NULL, 0, NULL);
//}


// CDesignerViewImageEffectEditor

class ATL_NO_VTABLE CDesignerViewImageEffectEditor : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewImageEffectEditor>,
	public CThemeImpl<CDesignerViewImageEffectEditor>,
	public CDesignerViewWndImpl<CDesignerViewImageEffectEditor, IDesignerView>,
	public CObserverImpl<CDesignerViewImageEffectEditor, IImageObserver, TImageChange>,
	public CObserverImpl<CDesignerViewImageEffectEditor, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewImageEffectEditor, ILayerEffectObserver, ULONG>,
	public IDesignerViewStatusBar,
	public CContextMenuWithIcons<CDesignerViewImageEffectEditor>,
	public CRasterImagePainting<CDesignerViewImageEffectEditor>,
	public CRasterImageMouseInput<CDesignerViewImageEffectEditor>
{
public:
	CDesignerViewImageEffectEditor() : m_nWheelDelta(0), m_dwVisibleHandles(0),
		m_tLocaleID(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT)), 
		m_hLastCursor(NULL), m_delayedGlobalChanges(0),
		m_fScale(1.0f), m_bChangingSharedState(true),
		m_bTrackMouse(false), m_nDraggedHandle(0), m_bSendingEffect(true),
		m_bMouseOut(true), m_nHotHandle(0xffffffff)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_hHandCursor = ::LoadCursor(NULL, IDC_HAND);
		if (m_hHandCursor == NULL)
			m_hHandCursor = ::LoadCursor(NULL, IDC_ARROW);
		m_nHotItem = 0;
		m_rcDrawnLines.top = m_rcDrawnLines.left = LONG_MAX;
		m_rcDrawnLines.bottom = m_rcDrawnLines.right = LONG_MIN;
	}
	~CDesignerViewImageEffectEditor()
	{
		m_cImageList.Destroy();
		if (m_hLastCursor)
			DestroyCursor(m_hLastCursor);
	}

	void Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IViewManager* a_pViewManager, IDocumentImage* a_pImage)
	{
		m_pDoc = a_pDoc;
		m_pImage = a_pImage;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImageLayerEffect), reinterpret_cast<void**>(&m_pImageEffect));

		TImageSize tSize = {1, 1};
		m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		a_pViewManager->QueryInterface(&m_pCmdMgr);
		if (m_pCmdMgr == NULL)
			RWCoCreateInstance(m_pCmdMgr, __uuidof(MenuCommandsManager));

		m_pStateMgr = a_pFrame;
		//CComObject<COperationContextFromStateManager>::CreateInstance(&m_pContext.p);
		//m_pContext.p->AddRef();
		//m_pContext->Init(a_pFrame);

		m_tLocaleID = a_tLocaleID;
		m_pConfig = a_pConfig;

		InitVisualization(a_pConfig, tSize.nX, tSize.nY);

		RWCoCreateInstance(m_pMGH, __uuidof(MouseGesturesHelper));

		CConfigValue cStateSync;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_IEEEDIT_VIEWPORTSYNC), &cStateSync);
		m_bstrViewportStateSync.Attach(cStateSync.Detach().bstrVal);

		//m_bstrToolID.Empty();
		if (m_pStateMgr)
		{
			InitImageEditorState(m_pStateMgr, a_pConfig);
			// initialize states
			if (m_bstrViewportStateSync.Length())
			{
				CComPtr<ISharedStateImageViewport> pMyState;
				m_pStateMgr->StateGet(m_bstrViewportStateSync, __uuidof(ISharedStateImageViewport), reinterpret_cast<void**>(&pMyState));
				if (pMyState)
				{
					float fOffsetX = 0.0f;
					float fOffsetY = 0.0f;
					float fZoom = 1.0f;
					boolean bAutoZoom = false;
					ULONG nSizeX = 0;
					ULONG nSizeY = 0;
					pMyState->GetEx(&fOffsetX, &fOffsetY, &fZoom, NULL, NULL, &nSizeX, &nSizeY, &bAutoZoom);
					if (M_ImageSize().cx == nSizeX && M_ImageSize().cy == nSizeY)
					{
						if (bAutoZoom)
							MoveViewport();
						else
							MoveViewport(fOffsetX, fOffsetY, fZoom);
					}
				}
			}
			m_pStateMgr->ObserverIns(CObserverImpl<CDesignerViewImageEffectEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		}

		m_pImage->ObserverIns(CObserverImpl<CDesignerViewImageEffectEditor, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_pImageEffect)
			m_pImageEffect->ObserverIns(CObserverImpl<CDesignerViewImageEffectEditor, ILayerEffectObserver, ULONG>::ObserverGet(), 0);

		m_pStatusBar = a_pStatusBar;

		//m_dwLastMessage = GetTickCount()-50000;
		// create self
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("EffectEditFrame"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, (a_nStyle&EDVWSBorderMask) != EDVWSBorder ? 0 : WS_EX_CLIENTEDGE) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}

		MoveWindow(const_cast<LPRECT>(a_rcWindow));
		ShowWindow(SW_SHOW);
	}

	DECLARE_WND_CLASS_EX(_T("RWViewEffectEditor"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	enum { SCROLL_TIMERID = 21, SCROLL_TICK = 100, TABLETBLOCKTIME = 500, WM_RW_UPDATETOOLCMDLINE = WM_APP+278 };

BEGIN_MSG_MAP(CDesignerViewImageEffectEditor)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewImageEffectEditor>)
	CHAIN_MSG_MAP(CRasterImagePainting<CDesignerViewImageEffectEditor>)
	CHAIN_MSG_MAP(CRasterImageMouseInput<CDesignerViewImageEffectEditor>)

	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
//MESSAGE_HANDLER(WM_SIZE, OnSize)
	//MESSAGE_HANDLER(WM_SETFOCUS, OnChangeFocus)
	//MESSAGE_HANDLER(WM_KILLFOCUS, OnChangeFocus)
	//MESSAGE_HANDLER(WM_KEYDOWN, OnKeyChange)
	//MESSAGE_HANDLER(WM_KEYUP, OnKeyChange)

	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

	//MESSAGE_HANDLER(WM_SELDOC_GETDATA, OnSelDocGetData)
	//MESSAGE_HANDLER(WM_SELDOC_SETDATA, OnSelDocSetData)

	MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)

	//MESSAGE_HANDLER(WM_HELP, OnHelp)

	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewImageEffectEditor>)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewImageEffectEditor)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	COM_INTERFACE_ENTRY(IImageZoomControl)
	COM_INTERFACE_ENTRY(IRasterEditView)
	//COM_INTERFACE_ENTRY(IRenderedImageEditor)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_pStateMgr)
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pImageEffect)
			m_pImageEffect->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, ILayerEffectObserver, ULONG>::ObserverGet(), 0);
	}

	void OnFinalMessage(HWND)
	{
		Release();
	}

	// observer handler
public:
	void OwnerNotify(TCookie a_tCookie, ULONG a_dwFlags)
	{
		if (m_hWnd == NULL || m_bSendingEffect)
			return;

		try
		{
			if (a_dwFlags&(ELECOpCfg|ELECOpID))
			{
				BYTE bEnabled = 0;
				m_tOpID = GUID_NULL;
				m_pOpCfg = NULL;
				m_pImageEffect->Get(&bEnabled, &m_tOpID, &m_pOpCfg);
				CComPtr<ICanvasInteractingOperation> pCIO;
				RWCoCreateInstance(pCIO, m_tOpID);
				CComPtr<ICanvasInteractingWrapper> pNew;
				if (pCIO)
					pCIO->CreateWrapper(m_pOpCfg, M_ImageSize().cx, M_ImageSize().cy, &pNew);

				if (pNew)
				{
					CControlHandles cHandles;
					GetDifferentHandles(pNew, cHandles);
					InvalidateHandles(cHandles);

					CHelpLines cLines;
					CComObjectStackEx<CControlLines> cReceiver;
					cReceiver.Init(cLines, 10.0f);
					pNew->GetControlLines(&cReceiver, 0);
					if (cLines != m_cDrawnLines)
					{
						if (!m_cDrawnLines.empty())
							InvalidateRect(&m_rcDrawnLines, FALSE);
						if (!cLines.empty())
						{
							RECT rc = {0, 0, 0, 0};
							SIZE szZoomed = M_ZoomedSize();
							SIZE szOrig = M_ImageSize();
							float fZoomX = float(szZoomed.cx)/szOrig.cx;
							float fZoomY = float(szZoomed.cy)/szOrig.cy;
							GetHelpLinesBounds(cLines, fZoomX, fZoomY, &rc);
							InvalidateRect(&rc, FALSE);
						}
					}
				}

				m_pWrapper = pNew;
			}
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, TImageChange a_tChange)
	{
		if (m_hWnd == NULL)
			return;

		try
		{
			if (a_tChange.nGlobalFlags & EICDimensions)
			{
				TImageSize tSize = {1, 1};
				m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				ResizeImage(tSize);
			}
			else if (a_tChange.tSize.nX*a_tChange.tSize.nY > 0)
			{
				RECT rc = {a_tChange.tOrigin.nX, a_tChange.tOrigin.nY, a_tChange.tOrigin.nX+a_tChange.tSize.nX, a_tChange.tOrigin.nY+a_tChange.tSize.nY};
				InvalidateImageRectangle(rc);
				if (!m_bSendingEffect)
					ProcessUpdateWindow();
			}
		}
		catch (...) {}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tState)
	{
		//if (m_bstrSelectionStateSync == a_tState.bstrName && a_tState.pState)
		//{
		//	std::vector<ULONG> aSelected;
		//	m_pImage->StateUnpack(a_tState.pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		//	if (aSelected.size() == m_aSelected.size())
		//	{
		//		ULONG i = 0;
		//		for (ULONG n = m_aSelected.size(); i < n; ++i)
		//			if (aSelected[i] != m_aSelected[i])
		//				break;
		//		if (i == m_aSelected.size())
		//			return; // no changes
		//	}
		//	std::swap(aSelected, m_aSelected);
		//	UpdateDragHandles();
		//	InvalidateAllGuides();
		//	//Invalidate(FALSE);
		//}
	}

	// message handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		//m_cToolList.ReadInitialState();
		//SetEditTool(m_cToolList.ActiveTool());

		HDC hDC = GetDC();
		m_fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
		ReleaseDC(hDC);

		//if (m_pSelectionDoc)
		//	m_pSelectionDoc->SetOwner(m_hWnd);

		m_cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);

		InitVisualizationPostCreate();
		SendViewportUpdate();

		//ControlPointsChanged();
		//ControlLinesChanged();

		AddRef();

		BYTE bEnabled = 0;
		m_tOpID = GUID_NULL;
		if (m_pImageEffect)
		{
			m_pImageEffect->Get(&bEnabled, &m_tOpID, &m_pOpCfg);
			CComPtr<ICanvasInteractingOperation> pCIO;
			RWCoCreateInstance(pCIO, m_tOpID);
			if (pCIO)
				pCIO->CreateWrapper(m_pOpCfg, M_ImageSize().cx, M_ImageSize().cy, &m_pWrapper);
		}
		m_bSendingEffect = false;
		m_bChangingSharedState = false;

		return 0;
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pImage)
		{
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, IImageObserver, TImageChange>::ObserverGet(), 0);
			m_pImage = NULL;
		}
		if (m_pStateMgr)
		{
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pStateMgr = NULL;
		}
		if (m_pImageEffect)
		{
			m_pImageEffect->ObserverDel(CObserverImpl<CDesignerViewImageEffectEditor, ILayerEffectObserver, ULONG>::ObserverGet(), 0);
			m_pImageEffect = NULL;
		}
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSelDocGetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled));
	LRESULT OnSelDocSetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled));
	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		ATLASSERT(0);
		//if (pHelpInfo->iContextType == HELPINFO_WINDOW)
		//{
		//	TCHAR szBuffer[512] = _T("");
		//	Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RE_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
		//	RECT rcItem;
		//	::GetWindowRect(m_hWnd, &rcItem);
		//	HH_POPUP hhp;
		//	hhp.cbStruct = sizeof(hhp);
		//	hhp.hinst = _pModule->get_m_hInst();
		//	hhp.idString = 0;
		//	hhp.pszText = szBuffer;
		//	hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		//	hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		//	hhp.clrForeground = 0xffffffff;
		//	hhp.clrBackground = 0xffffffff;
		//	hhp.rcMargins.left = -1;
		//	hhp.rcMargins.top = -1;
		//	hhp.rcMargins.right = -1;
		//	hhp.rcMargins.bottom = -1;
		//	hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
		//	HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
		//	return 0;
		//}
		a_bHandled = FALSE;
		return 0;
	}

	// IChildWindow methods
public:
	//STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	//STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);

	// IDesignerViewClipboardHandler methods
public:
	STDMETHOD(Priority)(BYTE* a_pPrio) { *a_pPrio = 192; return S_OK; }
	STDMETHOD(ObjectName)(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName) { return E_NOTIMPL; }
	STDMETHOD(ObjectIconID)(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID) { return E_NOTIMPL; }
	STDMETHOD(ObjectIcon)(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay) { return E_NOTIMPL; }
	STDMETHOD(Check)(EDesignerViewClipboardAction a_eAction) { return E_NOTIMPL; }
	STDMETHOD(Exec)(EDesignerViewClipboardAction a_eAction) { return E_NOTIMPL; }

	STDMETHOD(CanCut)() { return E_NOTIMPL; }
	STDMETHOD(Cut)() { return E_NOTIMPL; }
	STDMETHOD(CanCopy)() { return E_NOTIMPL; }
	STDMETHOD(Copy)() { return E_NOTIMPL; }
	STDMETHOD(CanPaste)() { return E_NOTIMPL; }
	STDMETHOD(Paste)() { return E_NOTIMPL; }
	STDMETHOD(CanSelectAll)() { return E_NOTIMPL; }
	STDMETHOD(SelectAll)() { return E_NOTIMPL; }
	STDMETHOD(CanInvertSelection)() { return E_NOTIMPL; }
	STDMETHOD(InvertSelection)() { return E_NOTIMPL; }

//	// IDragAndDropHandler methods
//public:
//	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
//	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IRasterEditView methods
public:
	STDMETHOD(ConfigureGestures)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pMGH ? m_pMGH->Configure(a_hParent ? a_hParent : m_hWnd, a_tLocaleID ? a_tLocaleID : m_tLocaleID, m_pConfig) : E_NOTIMPL;
	}

	// IDesignerViewUndoOverride methods
public:
	STDMETHOD(CanUndo)() { return E_NOTIMPL; }
	STDMETHOD(Undo)() { return E_NOTIMPL; }
	STDMETHOD(UndoName)(ILocalizedString** a_ppName) { return E_NOTIMPL; }
	STDMETHOD(CanRedo)() {return E_NOTIMPL;}
	STDMETHOD(Redo)() {return E_NOTIMPL;}
	STDMETHOD(RedoName)(ILocalizedString** a_ppName) { return E_NOTIMPL; }

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		return S_OK;
	}

public:
	//void SetEditTool(IEditTool* a_pTool, bool a_bDeactivate = true);
	//IEditTool* GetEditTool() {return m_pTool;}
	HRESULT ApplyChanges(BOOL a_bExplicit);

	int CanvasPadding() const { return 160*m_fScale; }

	// handlers
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd && LOWORD(a_lParam) == HTCLIENT)
		{
			if (UpdateCursor())
				return TRUE;
		}
		a_bHandled = FALSE;
		return 0;
	}

	BOOL UpdateCursor()
	{
		POINT tPt;
		GetCursorPos(&tPt);
		ScreenToClient(&tPt);
		ULONG n;
		if (TestHandle(tPt, &n))
		{
			SetCursor(m_hHandCursor);
			if (m_hLastCursor)
			{
				DestroyCursor(m_hLastCursor);
				m_hLastCursor = NULL;
			}
			return TRUE;
		}

		return FALSE;
	}

	bool TestHandle(POINT const& a_tPos, ULONG* a_pIndex) const
	{
		if (m_pWrapper == NULL)
			return false;
		float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
		float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
		float const fRadSq = (M_HandleRadius()+0.5f)*(M_HandleRadius()+0.5f);
		//for (CHandles::const_reverse_iterator i = m_cHandles.rbegin(); i != m_cHandles.rend(); ++i)
		for (ULONG i = m_pWrapper->GetControlPointCount(); i > 0; )
		{
			--i;
			TPixelCoords tPos;
			ULONG dummy;
			m_pWrapper->GetControlPoint(i, &tPos, &dummy);
			LONG const nX = fZoomX*tPos.fX+M_ImagePos().x;
			LONG const nY = fZoomY*tPos.fY+M_ImagePos().y;
			float const fDistSq = (a_tPos.x-nX)*(a_tPos.x-nX) + (a_tPos.y-nY)*(a_tPos.y-nY);
			if (fDistSq <= fRadSq)
			{
				*a_pIndex = i;
				return true;
			}
		}
		return false;
	}
	bool TestHandle(TPixelCoords const& a_tPos, ULONG* a_pIndex) const
	{
		if (m_pWrapper == NULL)
			return false;
		float const fRadSq = (M_HandleRadius()+0.5f)*(M_HandleRadius()+0.5f)/M_ImageZoom()/M_ImageZoom();
		//for (CHandles::const_reverse_iterator i = m_cHandles.rbegin(); i != m_cHandles.rend(); ++i)
		for (ULONG i = m_pWrapper->GetControlPointCount(); i > 0; )
		{
			--i;
			TPixelCoords tPos;
			ULONG dummy;
			m_pWrapper->GetControlPoint(i, &tPos, &dummy);
			float const fDistSq = (a_tPos.fX-tPos.fX)*(a_tPos.fX-tPos.fX) + (a_tPos.fY-tPos.fY)*(a_tPos.fY-tPos.fY);
			if (fDistSq <= fRadSq)
			{
				*a_pIndex = i;
				return true;
			}
		}
		return false;
	}

	LRESULT OnChangeFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		WORD wFlags = HIWORD(a_wParam);
		if (wFlags == 0xFFFF && a_lParam == NULL)
		{ // menu closing
			m_bShowCommandDesc = false;
		}
		else
		{
			m_bShowCommandDesc = true;
			m_bstrCommandDesc.Empty();
			if (wFlags & MF_POPUP)
			{
				// TODO: submenu descriptions
			}
			else
			{
				WORD wID = LOWORD(a_wParam)-1000;
				if (wID < m_cContextOps.size())
				{
					CComPtr<ILocalizedString> pStr;
					m_cContextOps[wID]->Description(&pStr);
					if (pStr)
					{
						pStr->GetLocalized(m_tLocaleID, &m_bstrCommandDesc);
						if (m_bstrCommandDesc) for (LPOLESTR p = m_bstrCommandDesc; *p; ++p)
							if (*p == L'\n') { *p = L'\0'; break; }
					}
				}
			}
		}
		if (m_pStatusBar) m_pStatusBar->Notify(0, 0);

		return 1;
	}
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	IDocument* M_Doc() const { return m_pDoc; }
	void SendViewportUpdate()
	{
		if (!m_bChangingSharedState && m_bstrViewportStateSync.Length())
		{
			CComPtr<ISharedStateImageViewport> pIV;
			RWCoCreateInstance(pIV, __uuidof(SharedStateImageViewport));
			if (pIV)
			{
				pIV->InitEx(M_OffsetX(), M_OffsetY(), M_ImageZoom(), M_WindowSize().cx, M_WindowSize().cy, M_ImageSize().cx, M_ImageSize().cy, M_AutoZoom());
				m_bChangingSharedState = true;
				m_pStateMgr->StateSet(m_bstrViewportStateSync, pIV.p);
				m_bChangingSharedState = false;
			}
		}
	}

	void GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		CComPtr<IDocumentImage> pImg;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
		pImg->TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer), NULL, EIRIPreview);
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

	struct bw_dash
	{
		bw_dash(float width, int dash) : lineWidth(width*128+128.5f), patternWidth(dash<<9), dashLength(dash) {}

		typedef agg::rgba8 color_type;

 		int pattern_width() const { return dashLength; }
		int line_width()    const { return lineWidth; }
		void pixel(agg::rgba8* p, int x, int y) const
		{
			int const f = x&0xff;
			int const i = 0xff-f;
			int const b = x>>8;
			p->r = p->g = p->b = ((b/dashLength)&1)*i + (((b+1)/dashLength)&1)*f;
			int const mask = y >> sizeof(int) * CHAR_BIT - 1;
			int const absy = (y + mask) ^ mask;
			int const cov = lineWidth-absy;
			int const cov2 = cov + ((255 - cov) & ((255 - cov) >> (sizeof(int) * CHAR_BIT - 1))); // min
			p->a = cov2 - (cov2 & (cov2 >> (sizeof(int) * CHAR_BIT - 1))); // max
			//p->a = absy < (line_width-0x100) ? 0xff : absy < line_width ? line_width-absy : 0;//-abs(y-0x100);
		}

		int const lineWidth;
		int const patternWidth;
		int const dashLength;
	};

	struct SHelpLinePoint
	{
		float fX;
		float fY;
		bool bLineTo;
		bool bClose;

		bool operator ==(SHelpLinePoint const& rhs) const
		{
			return fX == rhs.fX && fY == rhs.fY && bLineTo == rhs.bLineTo && bClose == rhs.bClose;
		}
	};
	typedef std::vector<SHelpLinePoint> CHelpLines;
	class CControlLines :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IEditToolControlLines
	{
	public:
		void Init(CHelpLines& a_cLines, float a_fRadiusInImagePixels)
		{
			m_pLines = &a_cLines;
			m_fRadiusInImagePixels = a_fRadiusInImagePixels;
		}

	BEGIN_COM_MAP(CControlLines)
		COM_INTERFACE_ENTRY(IEditToolControlLines)
	END_COM_MAP()

		// IEditToolControlLines methods
	public:
		STDMETHOD(MoveTo)(float a_fX, float a_fY)
		{
			SHelpLinePoint s = {a_fX, a_fY, false, false};
			m_pLines->push_back(s);
			return S_OK;
		}
		STDMETHOD(LineTo)(float a_fX, float a_fY)
		{
			SHelpLinePoint s = {a_fX, a_fY, true, false};
			m_pLines->push_back(s);
			return S_OK;
		}
		STDMETHOD(Close)()
		{
			if (m_pLines->empty())
				return E_FAIL;
			m_pLines->rbegin()->bClose = true;
			return S_OK;
		}
		STDMETHOD(HandleSize)(float* a_pRadiusInImagePixels)
		{
			if (a_pRadiusInImagePixels == NULL)
				return E_POINTER;
			*a_pRadiusInImagePixels = m_fRadiusInImagePixels;
			return S_OK;
		}

	private:
		CHelpLines* m_pLines;
		float m_fRadiusInImagePixels;
	};

	void GetHelpLinesBounds(CHelpLines const& cHelpLines, float fZoomX, float fZoomY, RECT* a_pLinesRect)
	{
		float fHelpLinesXMin = 1e6f;
		float fHelpLinesYMin = 1e6f;
		float fHelpLinesXMax = -1e6f;
		float fHelpLinesYMax = -1e6f;
		for (CHelpLines::const_iterator i = cHelpLines.begin(); i != cHelpLines.end(); ++i)
		{
			if (fHelpLinesXMin > i->fX) fHelpLinesXMin = i->fX;
			if (fHelpLinesYMin > i->fY) fHelpLinesYMin = i->fY;
			if (fHelpLinesXMax < i->fX) fHelpLinesXMax = i->fX;
			if (fHelpLinesYMax < i->fY) fHelpLinesYMax = i->fY;
		}

		// draw help lines
		float fXMin = fZoomX*fHelpLinesXMin+M_ImagePos().x;
		float fYMin = fZoomY*fHelpLinesYMin+M_ImagePos().y;
		float fXMax = fZoomX*fHelpLinesXMax+M_ImagePos().x;
		float fYMax = fZoomY*fHelpLinesYMax+M_ImagePos().y;
		LONG nXMin = fXMin-2;
		LONG nYMin = fYMin-2;
		LONG nXMax = fXMax+2;
		LONG nYMax = fYMax+2;
		if (a_pLinesRect)
		{
			a_pLinesRect->left = nXMin;
			a_pLinesRect->top = nYMin;
			a_pLinesRect->right = nXMax;
			a_pLinesRect->bottom = nYMax;
		}
	}
	void DrawHelpLines(CHelpLines const& cHelpLines, float fZoomX, float fZoomY, RECT const& a_rcDirty, RECT* a_pLinesRect, COLORREF* a_pBuffer, ULONG a_nStride)
	{
		float fHelpLinesXMin = 1e6f;
		float fHelpLinesYMin = 1e6f;
		float fHelpLinesXMax = -1e6f;
		float fHelpLinesYMax = -1e6f;
		for (CHelpLines::const_iterator i = cHelpLines.begin(); i != cHelpLines.end(); ++i)
		{
			if (fHelpLinesXMin > i->fX) fHelpLinesXMin = i->fX;
			if (fHelpLinesYMin > i->fY) fHelpLinesYMin = i->fY;
			if (fHelpLinesXMax < i->fX) fHelpLinesXMax = i->fX;
			if (fHelpLinesYMax < i->fY) fHelpLinesYMax = i->fY;
		}

		// draw help lines
		float fXMin = fZoomX*fHelpLinesXMin+M_ImagePos().x;
		float fYMin = fZoomY*fHelpLinesYMin+M_ImagePos().y;
		float fXMax = fZoomX*fHelpLinesXMax+M_ImagePos().x;
		float fYMax = fZoomY*fHelpLinesYMax+M_ImagePos().y;
		LONG nXMin = fXMin-2;
		LONG nYMin = fYMin-2;
		LONG nXMax = fXMax+2;
		LONG nYMax = fYMax+2;
		if (a_pLinesRect)
		{
			a_pLinesRect->left = nXMin;
			a_pLinesRect->top = nYMin;
			a_pLinesRect->right = nXMax;
			a_pLinesRect->bottom = nYMax;
		}
		if (fXMax >= fXMin && fYMax >= fYMin &&
			a_rcDirty.left < nXMax && a_rcDirty.top < nYMax &&
			a_rcDirty.right > nXMin && a_rcDirty.bottom > nYMin)
		{
			agg::rendering_buffer rbuf;
			rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, a_nStride*sizeof*a_pBuffer);
			agg::pixfmt_bgra32 pixf(rbuf);
			agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
			agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_p8 sl;

			agg::path_storage path;
			for (CHelpLines::const_iterator i = cHelpLines.begin(); i != cHelpLines.end(); ++i)
			{
				if (i->bLineTo)
				{
					path.line_to(fZoomX*i->fX+M_ImagePos().x-a_rcDirty.left, fZoomY*i->fY+M_ImagePos().y-a_rcDirty.top);
					if (i->bClose)
						path.close_polygon();
				}
				else
				{
					path.move_to(fZoomX*i->fX+M_ImagePos().x-a_rcDirty.left, fZoomY*i->fY+M_ImagePos().y-a_rcDirty.top);
				}
			}
			agg::conv_dash<agg::path_storage> dash(path);
			dash.add_dash(5, 5);
			agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke(dash);
			stroke.line_join(agg::bevel_join);
			stroke.line_cap(agg::butt_cap);
			stroke.width(1.0f);
			ren.color(agg::rgba8(255, 255, 255, 255));
			ras.add_path(stroke);
			agg::render_scanlines(ras, sl, ren);

			agg::conv_dash<agg::path_storage> dash2(path);
			dash2.add_dash(5, 5);
			dash2.dash_start(5);
			agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke2(dash2);
			stroke2.line_join(agg::bevel_join);
			stroke2.line_cap(agg::butt_cap);
			stroke2.width(1.0f);
			ren.color(agg::rgba8(0, 0, 0, 255));
			ras.add_path(stroke2);
			agg::render_scanlines(ras, sl, ren);
		}
	}

	void PostRenderImage(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
	{
		if (M_HideHandles() && m_bMouseOut)
		{
			RenderMouseTrail(a_rcImage, a_nWindowX, a_nWindowY, a_rcDirty, a_pBuffer, a_nStride);
			return;
		}

		//LONG const nScale = min(a_rcImage.right-a_rcImage.left, a_rcImage.bottom-a_rcImage.top);
		//float const fXScale = nScale*0.5;
		//float const fYScale = -nScale*0.5;
		//float const fXOffset = (a_rcImage.left+a_rcImage.right)*0.5f;
		//float const fYOffset = (a_rcImage.bottom+a_rcImage.top)*0.5f;

		//agg::rendering_buffer rbuf;
		//rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, a_nStride*sizeof*a_pBuffer);
		//agg::pixfmt_bgra32 pixf(rbuf);
		//agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		//agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		//agg::span_allocator<agg::rgba8> span_alloc;

		SIZE szZoomed = M_ZoomedSize();
		SIZE szOrig = M_ImageSize();
		float fZoomX = float(szZoomed.cx)/szOrig.cx;
		float fZoomY = float(szZoomed.cy)/szOrig.cy;
		if (m_pWrapper)
		{
			m_cDrawnLines.clear();
			CComObjectStackEx<CControlLines> cReceiver;
			cReceiver.Init(m_cDrawnLines, 10.0f);
			m_pWrapper->GetControlLines(&cReceiver, 0);
			DrawHelpLines(m_cDrawnLines, fZoomX, fZoomY, a_rcDirty, &m_rcDrawnLines, a_pBuffer, a_nStride);

			// draw handles
			m_cDrawnHandles.clear();
			WrapperToHandles(m_pWrapper, m_cDrawnHandles);
			for (CControlHandles::const_iterator i = m_cDrawnHandles.begin(); i != m_cDrawnHandles.end(); ++i)
			{
				TPixelCoords tPos = i->tPos;
				ULONG nClass = i->nClass;
				LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
				LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
				RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
				RECT rcIntersection =
				{
					max(rcPt.left, a_rcDirty.left),
					max(rcPt.top, a_rcDirty.top),
					min(rcPt.right, a_rcDirty.right),
					min(rcPt.bottom, a_rcDirty.bottom),
				};
				if (rcIntersection.left >= rcIntersection.right || rcIntersection.top >= rcIntersection.bottom)
					continue;
				bool const hot = m_nHotHandle == i-m_cDrawnHandles.begin() || (m_pDragged && m_nDraggedHandle == i-m_cDrawnHandles.begin());
				float const l = hot ? 0.9f : 0.8f;
				float const s = hot ? 1.0f : 0.9f;
				float const m2 = l + (l <= 0.5f ? l*s : s - l*s);
				float const m1 = 2.0f * l - m2;
				double hue = double(nClass)*360.0f/4.95f+200.0f;
				hue = 360.0-(hue-360.0f*floor(hue/360.0f));
				float r = hls_value(m1, m2, hue+120.0f);
				float g = hls_value(m1, m2, hue);
				float b = hls_value(m1, m2, hue-120.0f);
				if (r > 1.0f) r = 1.0f;
				if (g > 1.0f) g = 1.0f;
				if (b > 1.0f) b = 1.0f;
				if (r < 0.0f) r = 0.0f;
				if (g < 0.0f) g = 0.0f;
				if (b < 0.0f) b = 0.0f;
				BYTE bClassR = r*255.0f+0.5f;
				BYTE bClassG = g*255.0f+0.5f;
				BYTE bClassB = b*255.0f+0.5f;
				for (LONG y = rcIntersection.top; y < rcIntersection.bottom; ++y)
				{
					BYTE* pO = reinterpret_cast<BYTE*>(a_pBuffer + (y-a_rcDirty.top)*a_nStride + rcIntersection.left-a_rcDirty.left);
					BYTE const* pI = M_HandleImage() + (((y-rcPt.top)*(M_HandleRadius()+M_HandleRadius()+1) + rcIntersection.left-rcPt.left)<<1);
					for (LONG x = rcIntersection.left; x < rcIntersection.right; ++x)
					{
						if (pI[0])
						{
							if (pI[0] == 255)
							{
								pO[0] = (ULONG(pI[1])*bClassB)/255;
								pO[1] = (ULONG(pI[1])*bClassG)/255;
								pO[2] = (ULONG(pI[1])*bClassR)/255;
							}
							else
							{
								ULONG clr = ULONG(pI[1])*pI[0];
								ULONG inv = (255-pI[0])*255;
								pO[0] = (pO[0]*inv + clr*bClassB)/(255*255);
								pO[1] = (pO[1]*inv + clr*bClassG)/(255*255);
								pO[2] = (pO[2]*inv + clr*bClassR)/(255*255);
							}
						}
						pO += 4;
						pI += 2;
					}
				}
			}
		}

		RenderMouseTrail(a_rcImage, a_nWindowX, a_nWindowY, a_rcDirty, a_pBuffer, a_nStride);
	}
	void GetPixelFromPoint(POINT a_tPoint, TPixelCoords* a_pPixel, TPixelCoords* a_pPointerSize = NULL, ULONG const* a_pControlPointIndex = NULL, EControlKeysState a_eKeysState = ECKSNone) const
	{
		float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
		float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
		a_pPixel->fX = (a_tPoint.x - M_ImagePos().x/*+ fX*//* - tZoomedSize.cx*0.5f*/) / fZoomX;
		a_pPixel->fY = (a_tPoint.y - M_ImagePos().y/*+ fY*//* - tZoomedSize.cy*0.5f*/) / fZoomY;
		TPixelCoords tPointerSize = {1.0f/fZoomX, 1.0f/fZoomY};
		if (a_pPointerSize) *a_pPointerSize = tPointerSize;
		//m_pActiveTool->AdjustCoordinates(a_eKeysState, a_pPixel, &tPointerSize, a_pControlPointIndex);
	}
	ULONG SetControlHandle(ICanvasInteractingWrapper* a_pWrapper, ULONG a_nHandle, TPixelCoords const* a_pPos, bool a_bReleased)
	{
		CComPtr<ICanvasInteractingWrapper> pNew;
		ULONG nHandle = m_nDraggedHandle;
		a_pWrapper->SetControlPoint(a_nHandle, a_pPos, a_bReleased, (M_HandleRadius()+0.5f)/M_ImageZoom(), &pNew, &nHandle);

		CControlHandles cHandles;
		GetDifferentHandles(pNew, cHandles);
		InvalidateHandles(cHandles);

		CHelpLines cLines;
		CComObjectStackEx<CControlLines> cReceiver;
		cReceiver.Init(cLines, 10.0f);
		pNew->GetControlLines(&cReceiver, 0);
		if (cLines != m_cDrawnLines)
		{
			if (!m_cDrawnLines.empty())
				InvalidateRect(&m_rcDrawnLines, FALSE);
			if (!cLines.empty())
			{
				RECT rc = {0, 0, 0, 0};
				SIZE szZoomed = M_ZoomedSize();
				SIZE szOrig = M_ImageSize();
				float fZoomX = float(szZoomed.cx)/szOrig.cx;
				float fZoomY = float(szZoomed.cy)/szOrig.cy;
				GetHelpLinesBounds(cLines, fZoomX, fZoomY, &rc);
				InvalidateRect(&rc, FALSE);
			}
		}

		m_pWrapper = pNew;
		pNew->ToConfig(m_pOpCfg);
		m_bSendingEffect = true;
		m_pImageEffect->Set(NULL, NULL, m_pOpCfg);
		m_bSendingEffect = false;
		return nHandle;
	}
	void ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		if (m_pWrapper == NULL)
			return;

		bool bPrevMouseOut = m_bMouseOut;
		m_bMouseOut = a_pPos == NULL && a_fNormalPressure < 0.333f;
		if (M_HideHandles() && bPrevMouseOut != m_bMouseOut)
			Invalidate(FALSE);
		ULONG const nPrevHot = m_nHotHandle;
		m_nHotHandle = 0xffffffff;

		//if (a_pPos == NULL)
		//{
		////	if (m_eHotHandle != EMSNothing || m_nHotItem != 0)
		////	{
		////		InvalidateAllGuides();
		////		m_eHotHandle = EMSNothing;
		////		m_nHotItem = 0;
		////		ApplyDelayedChanges();
		////	}
		//	return; // TODO: handle mouse-leave event (remove hot states)
		//}

		if (m_pDragged)
		{
			bool bReleased = a_pPos == NULL || a_fNormalPressure < 0.333f;
			if (a_pPos)
			{
				SetControlHandle(m_pDragged, m_nDraggedHandle, a_pPos, bReleased);
			}
			if (bReleased)
			{
				m_pDragged = NULL;
			}
		}
		if (m_pDragged == NULL) // not dragging handle
		{
			if (a_pPos && a_fNormalPressure > 0.666f)
			{
				if (TestHandle(*a_pPos, &m_nDraggedHandle))
				{
					m_tLastPos = *a_pPos;
					m_pDragged = m_pWrapper;
					//return;
				}
			}
			else
			{
				ULONG nPt = 0xffffffff;
				if (a_pPos && TestHandle(*a_pPos, &nPt))
				{
					m_nHotHandle = nPt;
				}
			}
		}

		if (nPrevHot != m_nHotHandle)
		{
			SIZE szZoomed = M_ZoomedSize();
			SIZE szOrig = M_ImageSize();
			float fZoomX = float(szZoomed.cx)/szOrig.cx;
			float fZoomY = float(szZoomed.cy)/szOrig.cy;

			if (nPrevHot < m_cDrawnHandles.size())
			{
				TPixelCoords tPos = m_cDrawnHandles[nPrevHot].tPos;
				LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
				LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
				RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
				InvalidateRect(&rcPt);
			}
			if (m_nHotHandle < m_cDrawnHandles.size())
			{
				TPixelCoords tPos = m_cDrawnHandles[m_nHotHandle].tPos;
				LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
				LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
				RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
				InvalidateRect(&rcPt);
			}
		}

		//if (a_pPos == NULL)
		//{
		//	if (m_eHotHandle != EMSNothing || m_nHotItem != 0)
		//	{
		//		InvalidateAllGuides();
		//		m_eHotHandle = EMSNothing;
		//		m_nHotItem = 0;
		//		ApplyDelayedChanges();
		//	}
		//	return; // TODO: handle mouse-leave event (remove hot states)
		//}

		ProcessUpdateWindow();
	}
	struct TCanvasWrapperHandler
	{
		TCanvasWrapperHandler() : pEditor(NULL), nHandle(0) {}
		TCanvasWrapperHandler(CDesignerViewImageEffectEditor* pEditor, ULONG nHandle) : pEditor(pEditor), nHandle(nHandle) {}
		TPixelCoords get()
		{
			TPixelCoords tPixel = {0, 0};
			if (nHandle < pEditor->m_cDrawnHandles.size())
				tPixel = pEditor->m_cDrawnHandles[nHandle].tPos;
			return tPixel;
		}
		HRESULT set(TPixelCoords tCoords)
		{
			pEditor->SetControlHandle(pEditor->m_pWrapper, nHandle, &tCoords, true);
			return S_OK;
		}
	private:
		CDesignerViewImageEffectEditor* pEditor;
		ULONG nHandle;
	};
	void OnContextMenu(POINT const* a_pPoint)
	{
		CComPtr<IEnumUnknownsInit> pOps;
		RWCoCreateInstance(pOps, __uuidof(EnumUnknowns));

		CComPtr<ISharedState> pOverrideState;
		if (a_pPoint)
		{
			TPixelCoords tCoords = {0, 0};
			ULONG nHandle = 0xffffffff;
			TestHandle(*a_pPoint, &nHandle);
			GetPixelFromPoint(*a_pPoint, &tCoords, NULL, &nHandle);
			if (nHandle != 0xffffffff)
			{
				// TODO: manual handle coordinates
				CComObject<CManualHandleCoordinates<TCanvasWrapperHandler> >* p = NULL;
				CComObject<CManualHandleCoordinates<TCanvasWrapperHandler> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(TCanvasWrapperHandler(this, nHandle));
				pOps->Insert(pTmp);
				CComPtr<IDocumentMenuCommand> pSep;
				RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
				pOps->Insert(pSep);
			}
		}

		//if (pOverrideState)
		//	m_pContext->OverrideState(m_bstrSelectionStateSync, pOverrideState);

		CConfigValue cOpID;
		CComPtr<IConfig> pOpCfg;
		CComBSTR bstrCONTEXTMENU(CFGID_IEEEDIT_CONTEXTMENU);
		m_pConfig->ItemValueGet(bstrCONTEXTMENU, &cOpID);
		m_pConfig->SubConfigGet(bstrCONTEXTMENU, &pOpCfg);
		CComPtr<IEnumUnknowns> pCtxOps;
		m_pCmdMgr->CommandsEnum(m_pCmdMgr, cOpID, pOpCfg, NULL/*m_pContext*/, this, m_pDoc, &pCtxOps);
		if (pCtxOps)
		{
			pOps->InsertFromEnum(pCtxOps);
		}

		ULONG nSize = 0;
		if (pOps == NULL || FAILED(pOps->Size(&nSize)) || nSize == 0)
			return;

		POINT ptCenter;
		if (a_pPoint)
		{
			ptCenter = *a_pPoint;
		}
		else
		{
			ptCenter.x = M_WindowSize().cx>>1;
			ptCenter.y = M_WindowSize().cy>>1;
		}
		ClientToScreen(&ptCenter);

		Reset(m_cImageList);
		// TODO: check if the image list is too big and eventually delete it

		ATLASSERT(m_cContextOps.empty());
		CMenu cMenu;
		cMenu.CreatePopupMenu();

		UINT nMenuID = 1000;
		InsertMenuItems(pOps, m_cContextOps, cMenu.m_hMenu, &nMenuID);
		if (nMenuID == 1000)
			return;

		UINT nSelection = cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, ptCenter.x, ptCenter.y, m_hWnd, NULL);
		if (nSelection != 0)
		{
			CWaitCursor cWait;
			m_cContextOps[nSelection-1000]->Execute(reinterpret_cast<RWHWND>(m_hWnd), m_tLocaleID);
		}
		//if (pOverrideState)
		//	m_pContext->OverrideState(NULL, NULL);
		m_cContextOps.clear();
	}
	void ProcessGesture(ULONG a_nPoints, POINT const* a_pPoints)
	{
		CConfigValue cOpID;
		CComPtr<IConfig> pOpCfg;
		if (FAILED(m_pMGH->Recognize(a_nPoints, a_pPoints, m_pConfig, &cOpID, &pOpCfg)))
			return;

		CComObject<CGestureOperationManager>* pGestureOpMgr = NULL;
		CComObject<CGestureOperationManager>::CreateInstance(&pGestureOpMgr);
		CComPtr<IOperationManager> pGestureOperations = pGestureOpMgr;

		CComPtr<IDocument> pDoc;
		CComPtr<IOperationContext> pStatesTmp;// = m_pContext;
		CComPtr<IOperationManager> pOpMgr;
		RWCoCreateInstance(pOpMgr, __uuidof(OperationManager));
		pGestureOpMgr->Init(pOpMgr, this, true);

		//m_pContext->ResetErrorMessage();
		HRESULT hRes = pGestureOpMgr->Activate(pGestureOpMgr, pDoc.p ? pDoc.p : m_pDoc.p, cOpID, pOpCfg, pStatesTmp, m_hWnd, m_tLocaleID);
		if (FAILED(hRes) && hRes != E_RW_CANCELLEDBYUSER)
		{
			CComBSTR bstr;
			//if (m_pContext->M_ErrorMessage())
			//	m_pContext->M_ErrorMessage()->GetLocalized(m_tLocaleID, &bstr);
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			CComPtr<ILocalizedString> pCaption;
			if (pAI) pAI->Name(&pCaption);
			CComBSTR bstrCaption;
			if (pCaption) pCaption->GetLocalized(m_tLocaleID, &bstrCaption);
			if (bstrCaption == NULL) bstrCaption = L"Error";
			if (bstr != NULL && bstr[0])
			{
				::MessageBox(m_hWnd, CW2T(bstr), CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				CComBSTR bstrTemplate;
				CMultiLanguageString::GetLocalized(L"[0409]The attempted operation failed with error code 0x%08x. Please verify that there is enough free memory and that the configuration of the operation is correct.[0405]Provedení operace se nezdařilo a byl vrácen chybový kód 0x%08x. Prosím ověřte, že je dostatek volné paměti a konfigurace operace je správná.", m_tLocaleID, &bstrTemplate);
				TCHAR szMsg[256];
				szMsg[255] = _T('\0');
				_sntprintf(szMsg, 255, bstrTemplate, hRes);
				::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
			}
		}
	}


	// mouse gesture functions
public:
	HRESULT GestureUndo(IConfig* a_pCfg)
	{
		CComQIPtr<IDocumentUndo> pUndo(m_pDoc);
		if (pUndo)
		{
			pUndo->Undo(1); // ignore return value
			return S_OK;
		}
		return S_FALSE;
	}
	HRESULT GestureRedo(IConfig* a_pCfg)
	{
		CComQIPtr<IDocumentUndo> pUndo(m_pDoc);
		if (pUndo)
		{
			pUndo->Redo(1);
			return S_OK;
		}
		return S_FALSE;
	}
	//HRESULT GestureApply(IConfig* a_pCfg);
	//HRESULT GestureOutline(IConfig* a_pCfg);
	//HRESULT GestureDrawMode(IConfig* a_pCfg);
	//HRESULT GestureSwitchTool(IConfig* a_pCfg);
	//HRESULT GestureSwitchStyle(IConfig* a_pCfg);
	HRESULT GestureAutoZoom(IConfig* a_pCfg)
	{
		EnableAutoZoom(TRUE);
		return S_OK;
	}
	//HRESULT GestureSwapColors(IConfig* a_pCfg);

private:
	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};
	typedef std::map<GUID, int, lessBinary<GUID> > CImageMap;
	typedef std::vector<CComPtr<IDocumentMenuCommand> > CContextOps;

	typedef std::map<CComBSTR, TPixelCoords> CHandles;

public:
	class ATL_NO_VTABLE CGestureOperationManager : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationManager
	{
	public:
		CGestureOperationManager()
		{
		}
		void Init(IOperationManager* a_pManager, CDesignerViewImageEffectEditor* a_pView, bool a_bDeactivate = true)
		{
			m_bDeactivate = a_bDeactivate;
			m_pManager = a_pManager;
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CGestureOperationManager)
		COM_INTERFACE_ENTRY(IOperationManager)
		COM_INTERFACE_ENTRY(ILateConfigCreator)
	END_COM_MAP()


		DECLARE_PROTECT_FINAL_CONSTRUCT()

		HRESULT FinalConstruct()
		{
			return S_OK;
		}
		
		void FinalRelease() 
		{
		}

		// ILateConfigCreator methods
	public:
		STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
		{
			return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
		}

		// IOperationManager methods
	public:
		STDMETHOD(CreateConfigEx)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
		{
			try
			{
				*a_ppConfig = NULL;
				SGestureOperationEffect const* p = NULL;
				SGestureOperationEffect const* pEnd = NULL;
				GetGOs(&p, &pEnd);
				for (; p < pEnd; ++p)
				{
					if (IsEqualGUID(p->tID, a_ptControllerID->guidVal))
					{
						if (p->pfnConfig)
						{
							CComPtr<IConfigWithDependencies> pTmp;
							RWCoCreateInstance(pTmp, __uuidof(ConfigWithDependencies));
							p->pfnConfig(pTmp);
							pTmp->Finalize(NULL);
							*a_ppConfig = pTmp.Detach();
						}
						return S_OK;
					}
				}
				return m_pManager->CreateConfigEx(a_pOverrideForItem, a_ptControllerID, a_ppConfig);
			}
			catch (...)
			{
				return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
			}
		}

		STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
		{
			try
			{
				ULONG nCount = 0;
				HRESULT hRes = m_pManager->ItemGetCount(&nCount);
				SGestureOperationEffect const* p = NULL;
				SGestureOperationEffect const* pEnd = NULL;
				GetGOs(&p, &pEnd);
				*a_pnCount = nCount+(pEnd-p);
				return hRes;
			}
			catch (...)
			{
				return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
			}
		}
		STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
		{
			return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
		}
		STDMETHOD(ItemIDGet)(IOperationManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
		{
			try
			{
				SGestureOperationEffect const* p = NULL;
				SGestureOperationEffect const* pEnd = NULL;
				GetGOs(&p, &pEnd);
				if (a_nIndex < ULONG(pEnd-p))
				{
					*a_ppName = NULL;
					a_ptOperationID->eTypeID = ECVTGUID;
					a_ptOperationID->guidVal = p[a_nIndex].tID;
					*a_ppName = new CMultiLanguageString(p[a_nIndex].pszName);
					return S_OK;
				}
				else
				{
					return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-(pEnd-p), a_ptOperationID, a_ppName);
				}
			}
			catch (...)
			{
				return a_ppName == NULL || a_ptOperationID == NULL ? E_POINTER : E_UNEXPECTED;
			}
		}
		STDMETHOD(InsertIntoConfigAs)(IOperationManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
		{
			try
			{
				CComPtr<ISubConfigSwitchLate> pSubCfg;
				RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
				HRESULT hRes = pSubCfg->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
				if (FAILED(hRes)) return hRes;
				CConfigValue cDefault;
				hRes = m_pManager->ItemIDGetDefault(&cDefault);
				if (FAILED(hRes)) return hRes;
				hRes = a_pConfig->ItemIns1ofN(a_bstrID, a_pItemName, a_pItemDesc, cDefault, pSubCfg);
				if (FAILED(hRes)) return hRes;

				ULONG nCount = 0;
				hRes = ItemGetCount(&nCount);
				if (FAILED(hRes))
					return hRes;

				for (ULONG i = 0; i != nCount; ++i)
				{
					CComPtr<ILocalizedString> pStr;
					CConfigValue cVal;
					ItemIDGet(a_pOverrideForItem, i, &cVal, &pStr);
					a_pConfig->ItemOptionAdd(a_bstrID, cVal, pStr, a_nItemConditions, a_aItemConditions);
				}
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(CanActivate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates)
		{
			try
			{
				SGestureOperationEffect const* p = NULL;
				SGestureOperationEffect const* pEnd = NULL;
				GetGOs(&p, &pEnd);
				for (; p < pEnd; ++p)
				{
					if (IsEqualGUID(p->tID, a_ptOperationID->guidVal))
						return m_pView ? S_OK : S_FALSE;
				}
				return m_pManager->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates);
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Activate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				SGestureOperationEffect const* p = NULL;
				SGestureOperationEffect const* pEnd = NULL;
				GetGOs(&p, &pEnd);
				for (; p < pEnd; ++p)
				{
					if (IsEqualGUID(p->tID, a_ptOperationID->guidVal))
					{
						return m_pView ? ((*m_pView).*(p->pMemFun))(a_pConfig) : E_FAIL;
					}
				}
				if (m_pView && m_bDeactivate)
					m_pView->DeactivateAll(FALSE);
				CUndoBlock cUndo(a_pDocument, CMultiLanguageString::GetAuto(L"[0409]Gesture[0405]Gesto"));
				return m_pManager->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Visit)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
		{
			return m_pManager->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_ptOperationID, a_pConfig, a_pVisitor);
		}

		void GetGOs(SGestureOperationEffect const** first, SGestureOperationEffect const** last)
		{
			static SGestureOperationEffect const g_aGestureOperationsEffect[] =
			{
				{L"[0409]Gestures - Undo[0405]Gesta - zpět", {0xe2338bc9, 0x2091, 0x4f7f, {0x85, 0xde, 0xbd, 0xa5, 0x12, 0x3f, 0x5b, 0x9f}}, &CDesignerViewImageEffectEditor::GestureUndo, NULL },
				{L"[0409]Gestures - Redo[0405]Gesta - vpřed", {0x80f0f964, 0x6a19, 0x4952, {0xbd, 0xd9, 0x2d, 0xc4, 0xa1, 0x09, 0x54, 0x90}}, &CDesignerViewImageEffectEditor::GestureRedo, NULL },
				//{L"[0409]Gestures - Apply[0405]Gesta - dokončit kreslení", {0xac6edf28, 0x16c7, 0x48b5, {0xba, 0x0d, 0x00, 0x39, 0x04, 0xef, 0x78, 0x22}}, &CDesignerViewImageEffectEditor::GestureApply, NULL },
				//{L"[0409]Gestures - Draw Mode[0405]Gesta - způsob kreslení", {0xcd079b01, 0xe63f, 0x419f, {0x8d, 0x54, 0xf8, 0x48, 0x6e, 0x9b, 0x07, 0xc7}}, &CDesignerViewVectorImageEditor::GestureDrawMode, &GestCfgDrawMode },
				//{L"[0409]Gestures - Outline[0405]Gesta - obrys", {0x299e9b6d, 0x8c10, 0x4fdb, {0x85, 0xc0, 0x3b, 0x7f, 0xcf, 0x55, 0x95, 0x69}}, &CDesignerViewVectorImageEditor::GestureOutline, &GestCfgOutline },
				//{L"[0409]Gestures - Switch Tool[0405]Gesta - změnit nástroj", {0x5442bd9a, 0xe682, 0x420d, {0xb2, 0xf4, 0x42, 0xca, 0xc3, 0xc2, 0xea, 0x6e}}, &CDesignerViewVectorImageEditor::GestureSwitchTool, &GestCfgToolOrStyle<CFGID_GESTURESWITCHTOOLID, GESTURESWITCHTOOLID_NAME, GESTURESWITCHTOOLID_DESC> },
				//{L"[0409]Gestures - Fill Style[0405]Gesta - styl vyplňování", {0xa9d285ec, 0xb8de, 0x41a1, {0xbb, 0x73, 0xd4, 0x20, 0xd2, 0x1c, 0x33, 0xba}}, &CDesignerViewVectorImageEditor::GestureSwitchStyle, &GestCfgToolOrStyle<CFGID_GESTURESWITCHSTYLEID, GESTURESWITCHSTYLEID_NAME, GESTURESWITCHSTYLEID_DESC> },
				{L"[0409]Gestures - Automatic Zoom[0405]Gesta - automatické zvětšení", {0x9c8d8a7d, 0x4184, 0x4849, {0xa4, 0x3b, 0x82, 0x09, 0xec, 0x9a, 0x3d, 0x2a}}, &CDesignerViewImageEffectEditor::GestureAutoZoom, NULL },
				//{L"[0409]Gestures - Swap Colors[0405]Gesta - prohodit barvy", {0x4921fe4e, 0x9b70, 0x4615, {0x8d, 0x23, 0x41, 0x55, 0x5b, 0xe4, 0x8a, 0x4f}}, &CDesignerViewVectorImageEditor::GestureSwapColors, NULL },
			};
			*first = g_aGestureOperationsEffect;
			*last = g_aGestureOperationsEffect+itemsof(g_aGestureOperationsEffect);
		}

	private:
		CComPtr<IOperationManager> m_pManager;
		CDesignerViewImageEffectEditor* m_pView;
		bool m_bDeactivate;
	};

private:
	void UpdateZoom();
	void ActivateWindow();

	void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE);
	void UpdateImageSize();
	void UpdateImagePos();

	void ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos);
	void InsertMenuItems(IEnumUnknowns* a_pCmds, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID)
	{
		bool bInsertSeparator = false;
		bool bFirst = true;
		ULONG nItems = 0;
		if (a_pCmds)
			a_pCmds->Size(&nItems);
		for (ULONG i = 0; i < nItems; i++)
		{
			CComPtr<IDocumentMenuCommand> pCmd;
			a_pCmds->Get(i, __uuidof(IDocumentMenuCommand), reinterpret_cast<void**>(&pCmd));
			if (pCmd == NULL)
				continue;
			EMenuCommandState eState = EMCSNormal;
			pCmd->State(&eState);
			if (eState & EMCSSeparator)
			{
				bInsertSeparator = true;
				continue;
			}
			if (eState & EMCSDisabled)
				continue; // do no show disabled items in context menu
			int nIcon = -1;
			GUID tIconID;
			HRESULT hRes;
			if (SUCCEEDED(hRes = pCmd->IconID(&tIconID)) && !IsEqualGUID(tIconID, GUID_NULL))
			{
				CImageMap::const_iterator j = m_cImageMap.find(tIconID);
				if (j != m_cImageMap.end())
				{
					nIcon = j->second;
					if (hRes == S_FALSE)
					{
						HICON hIcon = NULL;
						pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
						if (hIcon)
						{
							m_cImageList.ReplaceIcon(nIcon, hIcon);
							DestroyIcon(hIcon);
						}
					}
				}
				else
				{
					HICON hIcon = NULL;
					pCmd->Icon(XPGUI::GetSmallIconSize(), &hIcon);
					if (hIcon)
					{
						m_cImageList.AddIcon(hIcon);
						DestroyIcon(hIcon);
						nIcon = m_cImageMap[tIconID] = m_cImageList.GetImageCount()-1;
					}
				}
			}
			CComPtr<ILocalizedString> pName;
			pCmd->Name(&pName);
			CComBSTR bstrName;
			if (pName)
				pName->GetLocalized(m_tLocaleID, &bstrName);
			COLE2CT strName(bstrName.Length() ? bstrName.operator BSTR() : L"");
			if (eState & EMCSSubMenu)
			{
				CComPtr<IEnumUnknowns> pSubCmds;
				pCmd->SubCommands(&pSubCmds);
				ULONG nSubCmds = 0;
				if (pSubCmds && SUCCEEDED(pSubCmds->Size(&nSubCmds)) && nSubCmds)
				{
					CMenu cSubMenu;
					cSubMenu.CreatePopupMenu();
					InsertMenuItems(pSubCmds, a_cContextOps, cSubMenu.m_hMenu, a_pnMenuID);
					AddItem(reinterpret_cast<UINT>(cSubMenu.operator HMENU()), strName, nIcon);
					if (bInsertSeparator && !bFirst)
						a_cMenu.AppendMenu(MFT_SEPARATOR);
					a_cMenu.AppendMenu(MF_POPUP|MFT_OWNERDRAW, reinterpret_cast<UINT_PTR>(cSubMenu.Detach()), LPCTSTR(0));
					bInsertSeparator = bFirst = false;
				}
			}
			else
			{
				UINT uFlags = MFT_OWNERDRAW;
				if (eState & EMCSDisabled)
					uFlags |= MFS_DISABLED|MFS_GRAYED;
				if (eState & EMCSChecked)
					uFlags |= MFS_CHECKED;
				if (eState & EMCSRadio)
					uFlags |= MFT_RADIOCHECK;
				if (eState & EMCSBreak)
					uFlags |= MFT_MENUBREAK;

				a_cContextOps.push_back(pCmd); // should rollback it if the next op fails

				AddItem(*a_pnMenuID, strName, nIcon);
				if (bInsertSeparator && !bFirst)
					a_cMenu.AppendMenu(MFT_SEPARATOR);
				a_cMenu.AppendMenu(uFlags, (*a_pnMenuID)++, LPCTSTR(NULL));
				bInsertSeparator = bFirst = false;
			}
		}
	}
	bool IsSelected(ULONG a_nItemID)
	{
		for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
		{
			if (*i == a_nItemID)
				return true;
		}
		return false;
	}


	void InvalidateAllGuides() // invalidate rectangle covering outlines, control handles and transformation arrows
	{
		Invalidate(FALSE);
		//InvalidateRect();
	}

	struct SControlHandle
	{
		TPixelCoords tPos;
		ULONG nClass;

		bool operator<(SControlHandle rhs) const
		{
			return tPos.fX < rhs.tPos.fX || (tPos.fX == rhs.tPos.fX && (tPos.fY < rhs.tPos.fY || (tPos.fY == rhs.tPos.fY && nClass < rhs.nClass)));
		}
	};
	typedef std::vector<SControlHandle> CControlHandles;
	static void WrapperToHandles(ICanvasInteractingWrapper* pWrapper, CControlHandles& cHandles)
	{
		if (pWrapper == NULL)
			return;
		for (ULONG i = 0; i < pWrapper->GetControlPointCount(); ++i)
		{
			SControlHandle s;
			pWrapper->GetControlPoint(i, &s.tPos, &s.nClass);
			cHandles.push_back(s);
		}
	}
	void GetDifferentHandles(ICanvasInteractingWrapper* pNew, CControlHandles& cHandles)
	{
		CControlHandles cNew;
		WrapperToHandles(pNew, cNew);
		std::sort(cNew.begin(), cNew.end());
		CControlHandles cDrawn(m_cDrawnHandles);
		std::sort(cDrawn.begin(), cDrawn.end());
		std::set_symmetric_difference(cDrawn.begin(), cDrawn.end(), cNew.begin(), cNew.end(), std::back_inserter(cHandles));
	}
	void InvalidateHandles(CControlHandles const& cHandles)
	{
		RECT rc = {INT_MAX, INT_MAX, INT_MIN, INT_MIN};
		for (CControlHandles::const_iterator j = cHandles.begin(); j != cHandles.end(); ++j)
		{
			LONG const nX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx)*j->tPos.fX+M_ImagePos().x;
			LONG const nY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy)*j->tPos.fY+M_ImagePos().y;
			if (rc.left > nX) rc.left = nX;
			if (rc.top > nY) rc.top = nY;
			if (rc.right < nX) rc.right = nX;
			if (rc.bottom < nY) rc.bottom = nY;
		}

		//if (rc.left <= rc.right)
		//	SendDrawToolCmdLineUpdateLater();

		rc.left -= M_HandleRadius();
		rc.top -= M_HandleRadius();
		rc.right += M_HandleRadius()+1;
		rc.bottom += M_HandleRadius()+1;
		if (rc.left < 0) rc.left = 0;
		if (rc.top < 0) rc.top = 0;
		if (rc.right > M_WindowSize().cx) rc.right = M_WindowSize().cx;
		if (rc.bottom > M_WindowSize().cy) rc.bottom = M_WindowSize().cy;
		if (rc.left < rc.right && rc.top < rc.bottom)
			InvalidateRect(&rc, FALSE);
	}

private:
	// document
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentImage> m_pImage;
	CComPtr<IDocumentImageLayerEffect> m_pImageEffect;

	// infrastructure
	CComPtr<IConfig> m_pConfig;
	DWORD m_dwVisibleHandles;

	// context menu
	CComPtr<IMenuCommandsManager> m_pCmdMgr;
	CImageList m_cImageList;
	CImageMap m_cImageMap;
	CContextOps m_cContextOps;

	// selection

	// window
	LCID m_tLocaleID;
	int m_nWheelDelta;

	// edit state
	bool m_bChangingSharedState;
	CComPtr<ISharedStateManager> m_pStateMgr;
	GUID m_tOpID;
	CComPtr<IConfig> m_pOpCfg;
	CComPtr<ICanvasInteractingWrapper> m_pWrapper;

	CControlHandles m_cDrawnHandles;
	CHelpLines m_cDrawnLines;
	RECT m_rcDrawnLines;

	//CComPtr<CComObject<COperationContextFromStateManager> > m_pContext;
	// local state values
	//ULONG m_nScrollIDLast;
	//float m_fScrollOrigX;
	//float m_fScrollOrigY;

	bool m_bAutoScroll;
	DWORD m_dwScrollStart;
	POINT m_tScrollMousePos;

	// cached visualization values
	float m_fScale;
	COLORREF m_tSelection;

	// edit state
	//TPixelCoords m_tLastPos;
	//EMouseButton m_eButton;
	HCURSOR m_hHandCursor;
	bool m_bTrackMouse;
	bool m_bMouseOut;
	ULONG m_nHotHandle;

	// status bar
	CComPtr<IStatusBarObserver> m_pStatusBar;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;

	// viewport sync
	CComBSTR m_bstrViewportStateSync;

	HCURSOR m_hLastCursor;

	// selection
	std::vector<ULONG> m_aSelected;
	ULONG m_nHotItem;
	//CComObject<CSelectionDocument>* m_pSelectionDoc;

	// editation
	CComPtr<ICanvasInteractingWrapper> m_pDragged;
	TPixelCoords m_tLastPos;
	ULONG m_nDraggedHandle;

	std::map<ULONG, ULONG> m_delayedObjectChanges;
	ULONG m_delayedGlobalChanges;

	bool m_bSendingEffect;

	// mouse gestures
	CComPtr<IMouseGesturesHelper> m_pMGH;
};


// CDesignerViewFactoryImageEffectEditor

class ATL_NO_VTABLE CDesignerViewFactoryImageEffectEditor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryImageEffectEditor>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryImageEffectEditor()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryImageEffectEditor)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryImageEffectEditor)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryImageEffectEditor)
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
			*a_ppName = new CMultiLanguageString(L"[0409]Layered Image - Effect Editor[0405]Vrstvený obrázek - editor stylu vrstvy");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		if (a_ppDefaultConfig == NULL)
			return E_POINTER;

		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			CDesignerViewImageEffectEditor::InitPaintingConfig(pCfgInit);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_IEEEDIT_VIEWPORTSYNC), CMultiLanguageString::GetAuto(L"[0409]Viewport ID[0405]ID náhledu"), CMultiLanguageString::GetAuto(L"[0409][0405]"), CConfigValue(L"VIEWPORT"), NULL, 0, NULL);


			// context menu
			CComPtr<IMenuCommandsManager> pMenuCmds;
			a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
			if (pMenuCmds == NULL)
			{
				RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
			}
			pMenuCmds->InsertIntoConfigAs(pMenuCmds, pCfgInit, CComBSTR(CFGID_IEEEDIT_CONTEXTMENU), CMultiLanguageString::GetAuto(L"[0409]Context menu[0405]Kontextové menu"), CMultiLanguageString::GetAuto(L"[0409][0405]"), 0, NULL);

			// extra mouse gestures -> document operation
			CComPtr<IOperationManager> pOperations;
			a_pManager->QueryInterface(__uuidof(IOperationManager), reinterpret_cast<void**>(&pOperations));
			if (pOperations == NULL)
			{
				RWCoCreateInstance(pOperations, __uuidof(OperationManager));
			}
			CComObject<CDesignerViewImageEffectEditor::CGestureOperationManager>* pGestureOpMgr = NULL;
			CComObject<CDesignerViewImageEffectEditor::CGestureOperationManager>::CreateInstance(&pGestureOpMgr);
			CComPtr<IOperationManager> pGestureOperations = pGestureOpMgr;
			pGestureOpMgr->Init(pOperations, NULL);
			CComPtr<IMouseGesturesHelper> pMGH;
			RWCoCreateInstance(pMGH, __uuidof(MouseGesturesHelper));
			if (pMGH)
				pMGH->InitConfig(pGestureOperations, pCfgInit);

			// finalize the initialization of the config
			pCfgInit->Finalize(NULL);
			//CConfigCustomGUI<&CLSID_DesignerViewFactoryRasterEdit, CConfigGUIRasterEditDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
	{
		if (a_ppDVWnd == NULL)
			return E_POINTER;

		try
		{
			CComObject<CDesignerViewImageEffectEditor>* pWnd = NULL;
			CComObject<CDesignerViewImageEffectEditor>::CreateInstance(&pWnd);
			CComPtr<IDesignerView> pOut(pWnd);

			CComPtr<IDocumentImage> pImage;
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
			if (pImage == NULL)
				return E_NOINTERFACE;

			pWnd->Init(a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, a_pManager, pImage);

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
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&p));
		if (p) a_pCallback->Used(__uuidof(IDocumentImage));
		else a_pCallback->Missing(__uuidof(IDocumentImage));
		return S_OK;
	}

};

// {91B4B7BF-3290-4104-9B7F-C7177EC38497}
static const GUID CLSID_DesignerViewFactoryImageEffectEditor = {0x91b4b7bf, 0x3290, 0x4104, {0x9b, 0x7f, 0xc7, 0x17, 0x7e, 0xc3, 0x84, 0x97}};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryImageEffectEditor, CDesignerViewFactoryImageEffectEditor)
