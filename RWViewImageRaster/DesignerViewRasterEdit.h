// DesignerViewRasterEdit.h : Declaration of the CDesignerViewRasterEdit

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include "RWViewImage.h"
#include <ObserverImpl.h>
#include "ConfigIDsRasterEdit.h"
#include <atlscrl.h>
#include <htmlhelp.h>
#include <Win32LangEx.h>
#include "RasterImageEditWindowCallbackHelper.h"
#include "WinTabWrapper.h"
#include <ContextMenuWithIcons.h>
#include <StringParsing.h>
#include <RWBaseEnumUtils.h>
#include <rtscom.h>
#include "StylusSupport.h"
#define PERFMON
#include <RasterImagePainting.h>

class MLNet;


// CDesignerViewRasterEdit

class ATL_NO_VTABLE CDesignerViewRasterEdit : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewRasterEdit>,
	public CThemeImpl<CDesignerViewRasterEdit>,
	public CDesignerViewWndImpl<CDesignerViewRasterEdit, IDesignerView>,
	public CObserverImpl<CDesignerViewRasterEdit, IImageObserver, TImageChange>,
	public CObserverImpl<CDesignerViewRasterEdit, ISharedStateObserver, TSharedStateChange>,
	public IDesignerViewStatusBar,
	public IDesignerViewClipboardHandler,
	public IDragAndDropHandler,
	public IDesignerViewUndoOverride,
	public IRasterImageFloatingSelection,
	public IRasterImageEditWindow,
	public CContextMenuWithIcons<CDesignerViewRasterEdit>,
	public CRasterImagePainting<CDesignerViewRasterEdit>
{
public:
	CDesignerViewRasterEdit() : m_eDragState(EDSNothing), m_nWheelDelta(0), /*m_pSelectionDoc(NULL),*/ m_pCallback(NULL),
		m_tLocaleID(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT)), 
		m_bstrCoordsPane(L"RasterImageCoord"), m_iActiveHandle(0),
		m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth),
		m_eCoordinatesMode(ECMIntegral), m_bFill(TRUE), m_bOutline(FALSE),
		m_fOutlineWidth(1.0f), m_fOutlinePos(-1.0f), m_eOutlineJoins(EOJTRound),
		m_hWTCtx(NULL), m_bChangingSharedState(false),
		m_nScrollIDLast(0xc0000000), m_fScrollOrigX(0.0f), m_fScrollOrigY(0.0f),
		m_fHelpLinesXMin(1e6f), m_fHelpLinesYMin(1e6f), m_fHelpLinesXMax(-1e6f), m_fHelpLinesYMax(-1e6f),
		m_bTabletDown(false), m_bTabletBlock(false), m_hLastCursor(NULL),
		m_bShowCommandDesc(false), m_bSelectionSupport(true), m_fScale(1.0f), m_pSSP(NULL),
		m_bTrackMouse(false), m_bPenLeft(false), m_bToolCmdLineUpdatePosted(false), m_bToolCmdLineUpdating(false),
		m_bMouseOut(true), m_nHotHandle(0xffffffff)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_tOutlineColor.fR = m_tOutlineColor.fG = m_tOutlineColor.fB = 0.0f;
		m_tOutlineColor.fA = 1.0f;
		//static const TRasterImagePixel INITIAL = {0xff, 0xff, 0xff, 0xff};
		//m_tColor1 = INITIAL;
		//m_tColor2 = INITIAL;
		m_tSentPos.fX = m_tSentPos.fY = 1e10f; // something invalid
		m_tLastPos.fX = m_tLastPos.fY = 0.0f;
		m_fLastPressure = 0.0f;
		m_hHandCursor = ::LoadCursor(NULL, IDC_HAND);
		if (m_hHandCursor == NULL)
			m_hHandCursor = ::LoadCursor(NULL, IDC_ARROW);
		//CComObject<CSelectionDocument>::CreateInstance(&m_pSelectionDoc);
		//if (m_pSelectionDoc)
		//	m_pSelectionDoc->AddRef();
	}
	~CDesignerViewRasterEdit()
	{
		m_cImageList.Destroy();
		if (m_pCallback)
		{
			m_pCallback->SetOwner(NULL);
			m_pCallback->Release();
		}
		if (m_hLastCursor)
			DestroyCursor(m_hLastCursor);
		if (m_pSSP) // should never happen
			m_pSSP->WindowDestroyed();
		//if (m_pSelectionDoc)
		//{
		//	m_pSelectionDoc->SetOwner(NULL);
		//	m_pSelectionDoc->Release();
		//}
	}

	void Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, CWinTabWrapper* a_pWinTab, IViewManager* a_pViewManager, IDocumentRasterImage* a_pImage);

	DECLARE_WND_CLASS_EX(_T("RWViewImageEdit"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	enum { SCROLL_TIMERID = 21, SCROLL_TICK = 100, TABLETBLOCKTIME = 500, WM_RW_UPDATETOOLCMDLINE = WM_APP+278 };

BEGIN_MSG_MAP(CDesignerViewRasterEdit)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewRasterEdit>)
	CHAIN_MSG_MAP(CRasterImagePainting<CDesignerViewRasterEdit>)

	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
	MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
	MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
	MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
	MESSAGE_HANDLER(WM_XBUTTONDOWN, OnXButtonDown)
	MESSAGE_HANDLER(WM_XBUTTONUP, OnXButtonUp)
	MESSAGE_HANDLER(WM_SETFOCUS, OnChangeFocus)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnChangeFocus)
	MESSAGE_HANDLER(WM_KEYDOWN, OnKeyChange)
	MESSAGE_HANDLER(WM_KEYUP, OnKeyChange)

	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)

	//MESSAGE_HANDLER(WM_SELDOC_GETDATA, OnSelDocGetData)
	//MESSAGE_HANDLER(WM_SELDOC_SETDATA, OnSelDocSetData)

	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)

	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)

	MESSAGE_HANDLER(WT_PACKET, OnWTPacket)
	MESSAGE_HANDLER(WT_STYLUSPACKET, OnStylusPacket)

	MESSAGE_HANDLER(WM_HELP, OnHelp)

	MESSAGE_HANDLER(WM_RW_UPDATETOOLCMDLINE, OnUpdateToolCmdLine)

	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewRasterEdit>)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewRasterEdit)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	COM_INTERFACE_ENTRY(IImageZoomControl)
	COM_INTERFACE_ENTRY(IRasterEditView)
	COM_INTERFACE_ENTRY(IDesignerViewClipboardHandler)
	COM_INTERFACE_ENTRY(IDragAndDropHandler)
	COM_INTERFACE_ENTRY(IDesignerViewUndoOverride)
	COM_INTERFACE_ENTRY(IRasterImageFloatingSelection)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewRasterEdit, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_pStateMgr)
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewRasterEdit, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	void OnFinalMessage(HWND)
	{
		Release();
	}

	// observer handler
public:
	void OwnerNotify(TCookie a_tCookie, TImageChange a_tChange);
	void OwnerNotify(TCookie, TSharedStateChange a_tState);

	// message handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		
		SetFocus();

		return MA_ACTIVATE;
	}
	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSelDocGetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled));
	LRESULT OnSelDocSetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled));
	LRESULT OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
		if (pHelpInfo->iContextType == HELPINFO_WINDOW)
		{
			TCHAR szBuffer[512] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_RE_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
			RECT rcItem;
			::GetWindowRect(m_hWnd, &rcItem);
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
	LRESULT OnWTPacket(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnStylusPacket(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel);

	// IDesignerView methods
public:
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges);

	// IDesignerViewClipboardHandler methods
public:
	STDMETHOD(Priority)(BYTE* a_pPrio) { *a_pPrio = 192; return S_OK; }
	STDMETHOD(ObjectName)(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName);
	STDMETHOD(ObjectIconID)(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID);
	STDMETHOD(ObjectIcon)(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay);
	STDMETHOD(Check)(EDesignerViewClipboardAction a_eAction);
	STDMETHOD(Exec)(EDesignerViewClipboardAction a_eAction);

	STDMETHOD(CanCut)();
	STDMETHOD(Cut)();
	STDMETHOD(CanCopy)();
	STDMETHOD(Copy)();
	STDMETHOD(CanPaste)();
	STDMETHOD(Paste)();
	STDMETHOD(CanSelectAll)();
	STDMETHOD(SelectAll)();
	STDMETHOD(CanInvertSelection)();
	STDMETHOD(InvertSelection)();
	STDMETHOD(CanDelete)();
	STDMETHOD(Delete)();
	STDMETHOD(CanDuplicate)();
	STDMETHOD(Duplicate)();

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IDesignerViewUndoOverride methods
public:
	STDMETHOD(CanUndo)();
	STDMETHOD(Undo)();
	STDMETHOD(UndoName)(ILocalizedString** a_ppName);
	STDMETHOD(CanRedo)() {return E_NOTIMPL;}
	STDMETHOD(Redo)() {return E_NOTIMPL;}
	STDMETHOD(RedoName)(ILocalizedString** a_ppName) { return E_NOTIMPL; }

	// IRasterImageFloatingSelection methods
public:
	STDMETHOD(SelectionExists)();
	STDMETHOD(GetSelectionTool)(IRasterImageEditToolFloatingSelection** a_ppTool);

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar);

	// IImageZoomControl methods
public:

	// IRasterEditView methods
public:
	STDMETHOD(CanApplyChanges)();
	//STDMETHOD(ApplyChanges)(); // same method below in IRasterImageEditWindow interface
	STDMETHOD(ConfigureGestures)(RWHWND a_hParent, LCID a_tLocaleID);

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY);
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault);
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer);
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle);
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer);
	STDMETHOD(ControlPointsChanged)();
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex);
	STDMETHOD(ControlLinesChanged)();
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged);
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta);
	STDMETHOD(ApplyChanges)();
	STDMETHOD(SetState)(ISharedState* a_pState);
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState);
	STDMETHOD(Handle)(RWHWND* a_phWnd);
	STDMETHOD(Document)(IDocument** a_ppDocument);
	STDMETHOD(Checkpoint)() { return E_NOTIMPL; }

	//void SetEditTool(IEditTool* a_pTool, bool a_bDeactivate = true);
	//IEditTool* GetEditTool() {return m_pTool;}
	HRESULT ApplyChanges(BOOL a_bExplicit);

	// handlers
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnRButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnXButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnXButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnChangeFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSettingChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& bHandled);
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	int CanvasPadding() const { return 160*m_fScale; }

	IDocument* M_Doc() const { return m_pDoc; }

	void UpdateScalableContent() { ControlLinesChanged(); }
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
		if (m_pActiveTool)
			m_pActiveTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, 2.2f, a_nStride, a_pBuffer);
	}
	void PostRenderImage(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride);
	void GetSelInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (m_pActiveTool)
			m_pActiveTool->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	bool GetSelTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pActiveTool && SUCCEEDED(m_pActiveTool->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer));
	}


	// mouse gesture functions
public:
	HRESULT GestureUndo(IConfig* a_pCfg);
	HRESULT GestureRedo(IConfig* a_pCfg);
	HRESULT GestureApply(IConfig* a_pCfg);
	HRESULT GestureOutline(IConfig* a_pCfg);
	HRESULT GestureDrawMode(IConfig* a_pCfg);
	HRESULT GestureSwitchTool(IConfig* a_pCfg);
	HRESULT GestureSwitchStyle(IConfig* a_pCfg);
	HRESULT GestureAutoZoom(IConfig* a_pCfg);
	HRESULT GestureSwapColors(IConfig* a_pCfg);

private:
	typedef std::vector<std::pair<TPixelCoords, ULONG> > CHandles;
	typedef std::vector<POINT> CGesturePoints;
	enum EDragState
	{
		EDSNothing = 0,
		EDSPanning,
		EDSZooming,
		EDSGesture,
		EDSTool,
		EDSHandle
	};
	struct SHelpLinePoint
	{
		float fX;
		float fY;
		bool bLineTo;
		bool bClose;
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

	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};
	typedef map<GUID, int, lessBinary<GUID> > CImageMap;
	typedef vector<CComPtr<IDocumentMenuCommand> > CContextOps;

	class ATL_NO_VTABLE COperationContextFromStateManager :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
		void Init(ISharedStateManager* a_pOrig)
		{
			m_pOrig = a_pOrig;
		}
		ILocalizedString* M_ErrorMessage()
		{
			return m_pMessage;
		}
		void ResetErrorMessage()
		{
			m_pMessage = NULL;
		}


	BEGIN_COM_MAP(COperationContextFromStateManager)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// ISharedStateManager methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			return m_pOrig->StateGet(a_bstrCategoryName, a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
		{
			return m_pOrig->StateSet(a_bstrCategoryName, a_pState);
		}
		STDMETHOD(IsCancelled)() { return S_FALSE; }
		STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
		{
			if (a_pItemIndex) *a_pItemIndex = 0;
			if (a_pItemsRemaining) *a_pItemsRemaining = 0;
			if (a_pStepIndex) *a_pStepIndex = 0;
			if (a_pStepsRemaining) *a_pStepsRemaining = 0;
			return S_OK;
		}
		STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
		{
			m_pMessage = a_pMessage;
			return S_OK;
		}

	private:
		CComPtr<ISharedStateManager> m_pOrig;
		CComPtr<ILocalizedString> m_pMessage;
	};

private:
	void ActivateWindow();

	void GetPixelFromPoint(POINT a_tPoint, TPixelCoords* a_pPixel, TPixelCoords* a_pPointerSize = NULL, ULONG const* a_pControlPointIndex = NULL, EControlKeysState a_eKeysState = ECKSNone) const;
	BOOL UpdateCursor();
	bool TestHandle(TPixelCoords const& a_tPos, ULONG* a_pIndex) const;
	bool TestHandle(POINT const& a_tPos, ULONG* a_pIndex) const;
	void InitTool()
	{
		std::vector<ULONG> aPaints;
		m_pTools->SupportedStates(m_bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
		bool bOutlineSupported = false;
		for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
		{
			if ((*i&ETBTIdentifierMask) == ETBTIdentifierOutline)
			{
				bOutlineSupported = true;
				break;
			}
		}
		m_pActiveTool->Init(m_pCallback);
		m_pActiveTool->SetBrush(m_bFill || !bOutlineSupported ? m_pActiveBrush.p : NULL);
		m_pActiveTool->SetGlobals(m_eBlendingMode, m_eRasterizationMode, m_eCoordinatesMode);
		m_pActiveTool->SetOutline(m_bOutline, m_fOutlineWidth, m_fOutlinePos, m_eOutlineJoins, &m_tOutlineColor);
		if (m_pStateMgr && m_bstrToolStateSync.Length() && m_bstrToolID.Length())
		{
			CComBSTR bstr(m_bstrToolStateSync);
			bstr += m_bstrToolID;
			CComPtr<ISharedState> pState;
			m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			if (pState)
				m_pActiveTool->SetState(pState);
		}
	}
	void InitFill()
	{
		if (m_pActiveBrush && m_pStateMgr && m_bstrToolStateSync.Length() && m_bstrStyleID.Length())
		{
			CComBSTR bstr(m_bstrToolStateSync);
			bstr += m_bstrStyleID;
			CComPtr<ISharedState> pState;
			m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			if (pState)
				m_pActiveBrush->SetState(pState);
		}
	}
	void ExecuteGesture(TConfigValue const& a_tOpID, IConfig* a_pOpCfg);

	void SendSelectionUpdate()
	{
		if (m_bstrSelectionStateSync.Length())
		{
			CComPtr<ISharedStateImageSelection> pSel;
			RWCoCreateInstance(pSel, __uuidof(SharedStateImageSelection));
			pSel->Init(m_rcSelection.left, m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, m_rcSelection.bottom-m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, m_aSelection);
			m_bChangingSharedState = true;
			m_pStateMgr->StateSet(m_bstrSelectionStateSync, pSel);
			m_bChangingSharedState = false;
		}
	}
	void SendDrawToolCmdLineUpdate()
	{
		if (!m_bToolCmdLineUpdating && m_bstrToolCommandStateSync.m_str && m_bstrToolCommandStateSync[0])
		{
			CComBSTR bstrCmdLine;
			CComQIPtr<IRasterImageEditToolScripting> pTool(m_pActiveTool);
			if (pTool)
				pTool->ToText(&bstrCmdLine);
			CComBSTR bstr(m_bstrToolID);
			bstr += L"\n";
			bstr += bstrCmdLine;

			CComPtr<ISharedState> pOldSel;
			m_pStateMgr->StateGet(m_bstrToolCommandStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldSel));
			if (pOldSel)
			{
				CComBSTR bstrOld;
				pOldSel->ToText(&bstrOld);
				if (bstrOld == bstr)
					return; // no change
			}

			CComPtr<ISharedState> pSel;
			RWCoCreateInstance(pSel, __uuidof(SharedStateString));
			pSel->FromText(bstr);

			m_bChangingSharedState = true;
			m_pStateMgr->StateSet(m_bstrToolCommandStateSync, pSel);
			m_bChangingSharedState = false;
		}
	}
	void SendDrawToolCmdLineUpdateLater()
	{
		if (m_hWnd && !m_bToolCmdLineUpdating && m_bstrToolCommandStateSync.m_str && m_bstrToolCommandStateSync[0] && !m_bToolCmdLineUpdatePosted)
		{
			PostMessage(WM_RW_UPDATETOOLCMDLINE);
			m_bToolCmdLineUpdatePosted = true;
		}
	}
	LRESULT OnUpdateToolCmdLine(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_bToolCmdLineUpdatePosted = false;
		SendDrawToolCmdLineUpdate();
		return 0;
	}

	HRESULT PasteSelection(float a_fX, float a_fY, ULONG a_nSizeX, ULONG a_nSizeY, TPixelChannel const* a_pData);
	HRESULT FinishCopy(HANDLE a_hMem, BITMAPV5HEADER* a_pHeader, ULONG a_nSizeX, ULONG a_nSizeY);
	void ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos);
	void InsertMenuItems(IEnumUnknowns* a_pOps, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID);
	void InitPaintConstants();
	static bool OptimizeSelection(CAutoVectorPtr<BYTE>& a_pData, RECT* a_pBounds);

	void InitRTS();

private:
	// document
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentRasterImage> m_pImage;

	// infrastructure
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditToolsManager> m_pTools;
	CComPtr<IRasterImageFillStyleManager> m_pFills;
	CComPtr<IConfig> m_pConfig;
	CComPtr<CWinTabWrapper> m_pWinTab;

	// context menu
	CComPtr<IMenuCommandsManager> m_pCmdMgr;
	CImageList m_cImageList;
	CImageMap m_cImageMap;
	CContextOps m_cContextOps;

	// selection
	bool m_bSelectionSupport;
	RECT m_rcSelection;
	CAutoVectorPtr<BYTE> m_aSelection;
	CComBSTR m_bstrPasteTool;

	// window
	LCID m_tLocaleID;
	int m_nWheelDelta;
	// tablet
	CComPtr<IRealTimeStylus> m_pRTS;
	CRWStylusSyncPlugin* m_pSSP;
	HCTX m_hWTCtx;
	float m_fTabletXZoom;
	float m_fTabletYZoom;
	float m_fTabletPressureZoom;
	DWORD m_dwLastMessage;
	bool m_bTabletDown;
	bool m_bTabletBlock;

	// edit state
	bool m_bChangingSharedState;
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<CComObject<COperationContextFromStateManager> > m_pContext;
	CComBSTR m_bstrToolStateSync;
	CComBSTR m_bstrSelectionStateSync;
	// local state values
	BOOL m_bFill;
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
	BOOL m_bOutline;
	TColor m_tOutlineColor;
	float m_fOutlineWidth;
	float m_fOutlinePos;
	EOutlineJoinType m_eOutlineJoins;
	CComBSTR m_bstrToolID;
	CComBSTR m_bstrStyleID;
	CComPtr<IRasterImageEditTool> m_pActiveTool;
	CComPtr<IRasterImageBrush> m_pActiveBrush;
	ULONG m_nScrollIDLast;
	float m_fScrollOrigX;
	float m_fScrollOrigY;

	CComBSTR m_bstrToolCommandStateSync;

	// scrolling
	bool m_bAutoScroll;
	DWORD m_dwScrollStart;
	POINT m_tScrollMousePos;

	CHelpLines m_cHelpLines;
	float m_fHelpLinesXMin;
	float m_fHelpLinesYMin;
	float m_fHelpLinesXMax;
	float m_fHelpLinesYMax;

	// cached visualization values
	float m_fScale;
	CHandles m_cHandles;
	size_t m_iActiveHandle;

	// edit state
	EDragState m_eDragState;
	TPixelCoords m_tLastPos;
	float m_fLastPressure;
	//EMouseButton m_eButton;
	ULONG m_nHandleIndex;
	HCURSOR m_hHandCursor;
	bool m_bTrackMouse;
	bool m_bPenLeft;
	bool m_bMouseOut;
	ULONG m_nHotHandle;

	// dragging with middle button -> move or zoom image
	POINT m_tDragStartClient;
	TPixelCoords m_tDragStartImage;
	float m_fOrigOffsetX;
	float m_fOrigOffsetY;
	float m_fOrigZoom;

	// status bar
	TPixelCoords m_tSentPos;
	CComPtr<IStatusBarObserver> m_pStatusBar;
	CComBSTR m_bstrCoordsPane;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;

	// mouse gestures
	CGesturePoints m_cGesturePoints;
	LONG m_nGestureXMin;
	LONG m_nGestureYMin;
	LONG m_nGestureXMax;
	LONG m_nGestureYMax;
	CComPtr<IMouseGesturesHelper> m_pMGH;
	CComPtr<IOperationManager> m_pOpMgr;

	// viewport sync
	CComBSTR m_bstrViewportStateSync;

	HCURSOR m_hLastCursor;

	bool m_bToolCmdLineUpdatePosted;
	bool m_bToolCmdLineUpdating;
	// selection
	//CComObject<CSelectionDocument>* m_pSelectionDoc;
};

