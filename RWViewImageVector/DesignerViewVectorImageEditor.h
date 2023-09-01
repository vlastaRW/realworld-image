// DesignerViewVectorImageEditor.h : Declaration of the CDesignerViewVectorImageEditor

#pragma once
#include "RWDocumentImageRaster.h"
#include "RWDocumentImageVector.h"
#include "../RWViewImage/RWViewImage.h"
#include "../RWViewImageRaster/RWViewImageRaster.h"
#include <ObserverImpl.h>
//#include "ConfigIDsRasterEdit.h"
#include <atlscrl.h>
#include <htmlhelp.h>
#include <Win32LangEx.h>
#include <ContextMenuWithIcons.h>
#include <StringParsing.h>
#include <math.h>
#define PERFMON
#include <RasterImagePainting.h>
#include <RasterImageMouseInput.h>
#include <RWBaseEnumUtils.h>
//#include "../../_extlibs/clipper.hpp"
#include "VectorDrawingToolHelper.h"
#include <MultiLanguageString.h>


//struct TPixelCoords { float fX, fY; };

static const OLECHAR CFGID_RVIEDIT_SELECTIONSYNC[] = L"SelectionSyncGroup";
static const OLECHAR CFGID_RVIEDIT_VIEWPORTSYNC[] = L"ViewportSyncGroup";
static const OLECHAR CFGID_RVIEDIT_CONTEXTMENU[] = L"ContextMenu";
static const OLECHAR CFGID_RVIEDIT_NEWTOOLSYNC[] = L"NewToolSyncGroup";
static const OLECHAR CFGID_RVIEDIT_TOOLSYNC[] = L"ToolSyncGroup";
static const OLECHAR CFGID_RVIEDIT_EDITTOOLSTATES[] = L"EditToolStates";
static const OLECHAR CFGID_RVIEDIT_TOOLMODE[] = L"ToolMode";
static const OLECHAR CFGID_RVIEDIT_TOOLCMDLINE[] = L"ToolCmdSyncID";

// CDesignerViewVectorImageEditor

class ATL_NO_VTABLE CDesignerViewVectorImageEditor : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewVectorImageEditor>,
	public CThemeImpl<CDesignerViewVectorImageEditor>,
	public CDesignerViewWndImpl<CDesignerViewVectorImageEditor, IDesignerView>,
	public CObserverImpl<CDesignerViewVectorImageEditor, IImageObserver, TImageChange>,
	public CObserverImpl<CDesignerViewVectorImageEditor, IVectorImageObserver, TVectorImageChanges>,
	public CObserverImpl<CDesignerViewVectorImageEditor, ISharedStateObserver, TSharedStateChange>,
	public IDesignerViewStatusBar,
	public IDesignerViewClipboardHandler,
	public IDragAndDropHandler,
	public IDesignerViewUndoOverride,
	public CContextMenuWithIcons<CDesignerViewVectorImageEditor>,
	public CRasterImagePainting<CDesignerViewVectorImageEditor>,
	public CRasterImageMouseInput<CDesignerViewVectorImageEditor>
{
public:
	CDesignerViewVectorImageEditor() : m_nWheelDelta(0),
		m_tLocaleID(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT)), 
		m_bChangingSharedState(false), m_bInitializing(false), m_nClipboardFormat(0),
		m_hLastCursor(NULL), m_nHotShape(0), m_nActiveShape(0), m_bTransforming(false),
		m_fScale(1.0f), m_iActiveHandle(-1), m_bShowCommandDesc(false),
		m_bToolCmdLineUpdatePosted(false), m_bToolCmdLineUpdating(false),
		m_bTrackMouse(false), m_eMS(EMSUnknown), m_eAutoUnselect(EAUGone),
		m_bMouseOut(true), m_nHotHandle(0xffffffff)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
		m_hHandCursor = ::LoadCursor(NULL, IDC_HAND);
		if (m_hHandCursor == NULL)
			m_hHandCursor = ::LoadCursor(NULL, IDC_ARROW);
		m_sActiveShape.bEnable = true;
		m_sActiveShape.eRM = ERMSmooth;
		m_sActiveShape.eCM = ECMFloatingPoint;
		m_sActiveShape.fWidth = 1.0f;
		m_sActiveShape.fPos = 0.0f;
		m_sActiveShape.eJoins = EOJTRound;
		m_sActiveShape.bFill = TRUE;
		m_sActiveShape.bOutline = FALSE;
		m_sActiveShape.tColor.fR = m_sActiveShape.tColor.fG = m_sActiveShape.tColor.fB = 0.0f;
		m_sActiveShape.tColor.fA = 1.0f;
		m_sActiveShape.pCallback = NULL;
		m_tMousePos.fX = m_tMousePos.fY = 0.0f;
		m_tSentPos.fX = m_tSentPos.fY = 1e9f;
		m_bstrCoordsPane = L"RasterImageCoord";
	}
	~CDesignerViewVectorImageEditor()
	{
		m_cImageList.Destroy();
		if (m_hLastCursor)
			DestroyCursor(m_hLastCursor);
	}

	void Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IViewManager* a_pViewManager, IDocumentVectorImage* a_pImage);

	DECLARE_WND_CLASS_EX(_T("RWViewRVIEditor"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_WINDOW);

	enum { SCROLL_TIMERID = 21, SCROLL_TICK = 100, TABLETBLOCKTIME = 500, WM_RW_UPDATETOOLCMDLINE = WM_APP+278 };

BEGIN_MSG_MAP(CDesignerViewVectorImageEditor)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewVectorImageEditor>)
	CHAIN_MSG_MAP(CRasterImagePainting<CDesignerViewVectorImageEditor>)
	CHAIN_MSG_MAP(CRasterImageMouseInput<CDesignerViewVectorImageEditor>)

	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
//MESSAGE_HANDLER(WM_SIZE, OnSize)
	//MESSAGE_HANDLER(WM_SETFOCUS, OnChangeFocus)
	//MESSAGE_HANDLER(WM_KILLFOCUS, OnChangeFocus)
	MESSAGE_HANDLER(WM_KEYDOWN, OnKeyChange)
	MESSAGE_HANDLER(WM_KEYUP, OnKeyChange)

	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

	MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)

	//MESSAGE_HANDLER(WM_HELP, OnHelp)

	MESSAGE_HANDLER(WM_RW_UPDATETOOLCMDLINE, OnUpdateToolCmdLine)

	CHAIN_MSG_MAP(CContextMenuWithIcons<CDesignerViewVectorImageEditor>)
END_MSG_MAP()

BEGIN_COM_MAP(CDesignerViewVectorImageEditor)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	COM_INTERFACE_ENTRY(IImageZoomControl)
	COM_INTERFACE_ENTRY(IDesignerViewClipboardHandler)
	COM_INTERFACE_ENTRY(IDragAndDropHandler)
	COM_INTERFACE_ENTRY(IRasterEditView)
	COM_INTERFACE_ENTRY(IDesignerViewUndoOverride)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		RWCoCreateInstance(m_pToolMgr, __uuidof(RasterImageEditToolsManager));
		RWCoCreateInstance(m_pFillMgr, __uuidof(RasterImageFillStyleManager));

		CComObject<CCallbackHelper<CDesignerViewVectorImageEditor> >::CreateInstance(&m_sActiveShape.pCallback.p);
		m_sActiveShape.pCallback.p->AddRef();
		m_sActiveShape.pCallback->SetOwner(this, 0);

		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_sActiveShape.pCallback)
		{
			m_sActiveShape.pCallback->SetOwner(NULL, 0);
			m_sActiveShape.pCallback = NULL;
		}
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewVectorImageEditor, IVectorImageObserver, TVectorImageChanges>::ObserverGet(), 0);
		if (m_pImage2)
			m_pImage2->ObserverDel(CObserverImpl<CDesignerViewVectorImageEditor, IImageObserver, TImageChange>::ObserverGet(), 0);
		if (m_pStateMgr)
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewVectorImageEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	void OnFinalMessage(HWND)
	{
		Release();
	}

	// observer handler
public:
	void OwnerNotify(TCookie a_tCookie, TVectorImageChanges a_tChanges)
	{
		if (m_hWnd == NULL)
			return;

		try
		{
			bool const bPrevEmpty = m_cShapes.empty();
			if (a_tChanges.nGlobalChangeFlags&EVICImageProps)
			{
				TImageSize tSize = {1, 1};
				m_pImage2->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				ResizeImage(tSize);
			}
			if (a_tChanges.nGlobalChangeFlags&EVICObjects)
			{
				std::vector<ULONG> aOrder;
				m_pImage->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aOrder));
				for (std::vector<ULONG>::const_iterator i = aOrder.begin(); i != aOrder.end(); ++i)
				{
					if (m_cShapes.find(*i) == m_cShapes.end())
					{
						RefreshShape(*i, m_cShapes[*i]);
						m_cShapes[*i].bEnable = m_pImage->ObjectIsEnabled(*i) != S_FALSE;
					}
				}
				if (aOrder.size() < m_cShapes.size())
				{
					std::set<ULONG> aIDs;
					for (std::vector<ULONG>::const_iterator i = aOrder.begin(); i != aOrder.end(); ++i)
						aIDs.insert(*i);
					for (CShapes::iterator i = m_cShapes.begin(); i != m_cShapes.end(); )
					{
						if (aIDs.find(i->first) == aIDs.end())
						{
							CShapes::iterator i2 = i;
							++i;
							m_cShapes.erase(i2);
						}
						else
						{
							++i;
						}
					}
				}
				std::swap(m_cShapeOrder, aOrder);
			}
			bool bActiveChanged = false;
			for (ULONG j = 0; j < a_tChanges.nObjects; ++j)
			{
				if (a_tChanges.aObjects[j].nChangeFlags&ECIVObjectShape)
				{
					CShapes::iterator i = m_cShapes.find(a_tChanges.aObjects[j].nID);
					if (i != m_cShapes.end())
					{
						if (a_tChanges.aObjects[j].nChangeFlags&ECIVObjectVisibility)
							i->second.bEnable = m_pImage->ObjectIsEnabled(i->first) != S_FALSE;
						if ((a_tChanges.aObjects[j].nChangeFlags&ECIVObjectShape) != ECIVObjectVisibility)
						{
							RefreshShape(i->first, i->second);
							if (m_nActiveShape == i->first)
								bActiveChanged = true;
						}
					}
				}
			}
			CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
			if (iSh == m_cShapes.end())
			{
				InvalidateHandles(m_cHandles);
				m_cHandles.clear();
				InvalidateHelpLines(m_cActiveHelpLines);
				m_cActiveHelpLines.clear();
				CComObjectStackEx<CControlLines> cLines;
				cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
				for (std::vector<ULONG>::const_iterator j = m_aSelected.begin(); j != m_aSelected.end(); ++j)
				{
					CShapes::const_iterator iSh = m_cShapes.find(*j);
					if (iSh != m_cShapes.end())
					{
						iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
					}
				}
				InvalidateHelpLines(m_cActiveHelpLines);
				if (m_cShapes.empty() && !bPrevEmpty)
				{
					CComPtr<ISharedStateToolMode> pOldTM;
					m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOldTM));
					CComPtr<ISharedStateToolMode> pTM;
					RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
					if (S_OK == pTM->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pOldTM))
						m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);
				}
			}
			else if (bActiveChanged)
			{
				InvalidateHandles(m_cHandles);
				ULONG nHandles = 0;
				iSh->second.pTool->GetControlPointCount(&nHandles);
				m_cHandles.resize(nHandles);
				for (ULONG i = 0; i != nHandles; ++i)
					iSh->second.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);
				InvalidateHandles(m_cHandles);

				CComObjectStackEx<CControlLines> cLines;
				InvalidateHelpLines(m_cActiveHelpLines);
				m_cActiveHelpLines.clear();
				cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
				iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
				cLines.SetType(ECLTHelp);
				iSh->second.pTool->GetControlLines(&cLines, ECLTHelp);
				InvalidateHelpLines(m_cActiveHelpLines);

				UpdateStates(iSh->second);
			}
			m_cDirty.clear();
		}
		catch (...)
		{
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tState);
	void OwnerNotify(TCookie, TImageChange a_tChange)
	{
		if (m_hWnd == NULL)
			return;

		try
		{
			if (a_tChange.nGlobalFlags & EICDimensions)
			{
				TImageSize tSize = {1, 1};
				m_pImage2->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				if (m_sActiveShape.pTool)
					m_sActiveShape.pTool->Reset();
				ResizeImage(tSize);
				//SendSelectionUpdate();
			}
			else if (a_tChange.tSize.nX*a_tChange.tSize.nY > 0)
			{
				RECT rc = {a_tChange.tOrigin.nX, a_tChange.tOrigin.nY, a_tChange.tOrigin.nX+a_tChange.tSize.nX, a_tChange.tOrigin.nY+a_tChange.tSize.nY};
				InvalidateImageRectangle(rc);
			}
		}
		catch (...) {}
	}

	// message handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
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
	//LRESULT OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

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

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar);

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG a_nShapeID, ULONG* a_pSizeX, ULONG* a_pSizeY) { *a_pSizeX = M_ImageSize().cx; *a_pSizeY = M_ImageSize().cy; return S_OK; }
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault) {	return m_pImage2->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, reinterpret_cast<TPixelChannel*>(a_pDefault))); }
	STDMETHOD(GetImageTile)(ULONG a_nShapeID, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		return S_OK;//m_pImage2->TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer), NULL, EIRIPreview);
	}
	STDMETHOD(ControlPointsChanged)(ULONG a_nShapeID)
	{
		if (m_sActiveShape.pTool && a_nShapeID == 0)
		{
			CHandles cHandles;
			ULONG nHandles = 0;
			m_sActiveShape.pTool->GetControlPointCount(&nHandles);
			cHandles.resize(nHandles);
			for (ULONG i = 0; i != nHandles; ++i)
				m_sActiveShape.pTool->GetControlPoint(i, &cHandles[i].first, &cHandles[i].second);
			std::swap(cHandles, m_cHandles);
			InvalidateHandles(cHandles);
			InvalidateHandles(m_cHandles);
			SetUpdateWindowFlag();
		}
		else if (a_nShapeID == m_nActiveShape)
		{
			CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
			if (iSh != m_cShapes.end())
			{
				CHandles cHandles;
				ULONG nHandles = 0;
				iSh->second.pTool->GetControlPointCount(&nHandles);
				cHandles.resize(nHandles);
				for (ULONG i = 0; i != nHandles; ++i)
					iSh->second.pTool->GetControlPoint(i, &cHandles[i].first, &cHandles[i].second);
				std::swap(cHandles, m_cHandles);
				InvalidateHandles(cHandles);
				InvalidateHandles(m_cHandles);
				SetUpdateWindowFlag();
			}
		}
		return S_OK;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nShapeID, ULONG a_nIndex)
	{
		return ControlPointsChanged(a_nShapeID);
	}
	STDMETHOD(ControlLinesChanged)(ULONG a_nShapeID)
	{
		if (m_sActiveShape.pTool && a_nShapeID == 0)
		{
			CComObjectStackEx<CControlLines> cLines;
			InvalidateHelpLines(m_cActiveHelpLines);
			m_cActiveHelpLines.clear();
			cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
			m_sActiveShape.pTool->GetControlLines(&cLines, ECLTSelection);
			cLines.SetType(ECLTHelp);
			m_sActiveShape.pTool->GetControlLines(&cLines, ECLTHelp);
			InvalidateHelpLines(m_cActiveHelpLines);
			SetUpdateWindowFlag();
		}
		else if (a_nShapeID == m_nActiveShape)
		{
			CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
			if (iSh != m_cShapes.end())
			{
				CComObjectStackEx<CControlLines> cLines;
				InvalidateHelpLines(m_cActiveHelpLines);
				m_cActiveHelpLines.clear();
				cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
				iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
				cLines.SetType(ECLTHelp);
				iSh->second.pTool->GetControlLines(&cLines, ECLTHelp);
				InvalidateHelpLines(m_cActiveHelpLines);
				SetUpdateWindowFlag();
			}
		}
		return S_OK;
	}
	STDMETHOD(RectangleChanged)(ULONG a_nShapeID, RECT const* a_pChanged)
	{
		if (m_bInitializing)
			return S_FALSE;
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			if (a_pChanged)
			{
				InvalidateImageRectangle(*a_pChanged);
			}
			else
			{
				Invalidate(FALSE);
			}
			SendDrawToolCmdLineUpdateLater();
			SetUpdateWindowFlag();
			return S_OK;
		}
		CShapes::const_iterator iSh = m_cShapes.find(a_nShapeID);
		if (iSh != m_cShapes.end())
		{
			m_cDirty.insert(iSh->first);
			if (a_pChanged)
			{
				InvalidateImageRectangle(*a_pChanged);
			}
			else
			{
				Invalidate(FALSE);
			}
			SendDrawToolCmdLineUpdateLater();
			SetUpdateWindowFlag();
			return S_OK;
		}
		return S_OK;
	}
	STDMETHOD(ApplyChanges)(BOOL a_bExplicit, ULONG a_nShapeID)
	{
		if (m_bInitializing)
			return S_FALSE;
		m_bApplyFlag = true;
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
			if (pCustApply)
			{
				pCustApply->ApplyChanges(a_bExplicit);
			}
			else
			{
			ULONG nID = 0;
			if (S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
			{
				CComQIPtr<IRasterImageEditToolScripting> pScripting(m_sActiveShape.pTool);
				CComBSTR bstrToolParams;
				if (pScripting)
					pScripting->ToText(&bstrToolParams);
				pScripting = m_sActiveShape.pFill;
				CComBSTR bstrFillParams;
				if (pScripting)
					pScripting->ToText(&bstrFillParams);
				{
					CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(L"[0409]Modify object(s)[0405]Upravit objekt(y)"));
					{
						CWriteLock<IDocument> cLock(m_pDoc);
						m_pImage->ObjectSet(&nID, m_sActiveShape.bstrTool, bstrToolParams);
						m_pImage->ObjectNameSet(nID, NULL);
						m_pImage->ObjectStyleSet(nID, m_sActiveShape.bstrFill, bstrFillParams);
						m_pImage->ObjectStateSet(nID, &m_sActiveShape.bFill, &m_sActiveShape.eRM, &m_sActiveShape.eCM, &m_sActiveShape.bOutline, &m_sActiveShape.tColor, &m_sActiveShape.fWidth, &m_sActiveShape.fPos, &m_sActiveShape.eJoins);
						m_eAutoUnselect = EAUSensing;
						m_sActiveShape.pCallback->SetOwner(this, nID);
						m_cShapes[nID] = m_sActiveShape;
						m_sActiveShape.pCallback = NULL;
						CComObject<CCallbackHelper<CDesignerViewVectorImageEditor> >::CreateInstance(&m_sActiveShape.pCallback.p);
						m_sActiveShape.pCallback.p->AddRef();
						m_sActiveShape.pCallback->SetOwner(this, 0);
						m_sActiveShape.pTool = NULL;
						m_sActiveShape.pFill = NULL;
						m_cShapeOrder.push_back(nID);
					}
				}

				// restore tool state
				CComPtr<ISharedStateToolMode> pMyState;
				m_pStateMgr->StateGet(m_bstrNewToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pMyState));
				if (pMyState)
				{
					CComBSTR bstrToolID;
					pMyState->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

					if (m_sActiveShape.bstrFill && *m_sActiveShape.bstrFill)
						m_pFillMgr->FillStyleCreate(m_sActiveShape.bstrFill, NULL, &m_sActiveShape.pFill);
					m_pToolMgr->EditToolCreate(m_sActiveShape.bstrTool, NULL, &m_sActiveShape.pTool);
					if (m_sActiveShape.pTool)
					{
						m_sActiveShape.pTool->Init(m_sActiveShape.pCallback);
						m_sActiveShape.pTool->SetBrush(m_sActiveShape.pFill);
						//m_sActiveShape.pTool->SetColors(&m_sActiveShape.tColor1, &m_sActiveShape.tColor2);
						//if (m_sActiveShape.pFill)
						//	m_sActiveShape.pFill->SetColors(&m_sActiveShape.tColor1, &m_sActiveShape.tColor2);
						m_sActiveShape.pTool->SetGlobals(EBMDrawOver, m_sActiveShape.eRM, m_sActiveShape.eCM);
						m_sActiveShape.pTool->SetOutline(m_sActiveShape.bOutline, m_sActiveShape.fWidth, m_sActiveShape.fPos, m_sActiveShape.eJoins, &m_sActiveShape.tColor);
						if (m_pStateMgr && m_bstrToolStateSync.Length())
						{
							CComBSTR bstr(m_bstrToolStateSync);
							bstr += m_sActiveShape.bstrTool;
							CComPtr<ISharedState> pState;
							m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
							if (pState)
								m_sActiveShape.pTool->SetState(pState);
							if (m_sActiveShape.pFill)
							{
								bstr = m_bstrToolStateSync;
								bstr += m_sActiveShape.bstrFill;
								pState = NULL;
								m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
								if (pState)
									m_sActiveShape.pFill->SetState(pState);
							}
						}
					}
				}
				//switch (m_eMS)
				//{
				//case EMSCreating: m_eMS = EMSModifying; break;
				//case EMSCreatingForwarding: m_eMS = EMSModifyingForwarding; break;
				//case EMSCreatingHandle: m_eMS = EMSModifyingHandle; break;
				//}
			}
			if (nID)
			{
				m_aSelected.clear();
				m_aSelected.push_back(nID);
				m_nActiveShape = nID;
				CComPtr<ISharedState> pState;
				m_pImage->StatePack(1, &nID, &pState);
				m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
				ControlLinesChanged(nID);
				ControlPointsChanged(nID);
			}
			}
		}
		else if (!m_cDirty.empty())
		{
			CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(L"[0409]Modify object(s)[0405]Upravit objekt(y)"));
			{
				CWriteLock<IDocument> cLock(m_pDoc);
				for (std::set<ULONG>::const_iterator i = m_cDirty.begin(); i != m_cDirty.end(); ++i)
				{
					CShapes::const_iterator iSh = m_cShapes.find(*i);
					if (iSh == m_cShapes.end())
						continue;
					SShape s = iSh->second;
					CComQIPtr<IRasterImageEditToolScripting> pScripting(s.pTool);
					CComBSTR bstrToolParams;
					CComBSTR bstrFillParams;
					if (pScripting)
						pScripting->ToText(&bstrToolParams);
					pScripting = s.pFill;
					if (pScripting)
						pScripting->ToText(&bstrFillParams);
					ULONG nID = *i;
					m_pImage->ObjectSet(&nID, s.bstrTool, bstrToolParams);
					m_pImage->ObjectStyleSet(*i, s.bstrFill, bstrFillParams);
					m_pImage->ObjectStateSet(*i, &iSh->second.bFill, &iSh->second.eRM, &iSh->second.eCM, &iSh->second.bOutline, &iSh->second.tColor, &iSh->second.fWidth, &iSh->second.fPos, &iSh->second.eJoins);
				}
				m_cDirty.clear();
			}

			//m_nActiveShape = 0;
		}
		m_iActiveHandle = -1;
		//InvalidateHandles(m_cHandles);
		//m_cHandles.clear();
		SendDrawToolCmdLineUpdateLater();
		return S_OK;
	}
	STDMETHOD(SetToolState)(ULONG a_nShapeID, ISharedState* a_pState)
	{
		if (a_nShapeID)
		{
			CShapes::iterator iSh = m_cShapes.find(a_nShapeID);
			if (iSh != m_cShapes.end())
			{
				iSh->second.pToolState = a_pState;
				if (!m_bInitializing && m_nActiveShape == a_nShapeID)
				{
					CComBSTR bstr(m_bstrToolStateSync);
					bstr += iSh->second.bstrTool;
					m_pStateMgr->StateSet(bstr, a_pState);
				}
			}
		}
		else if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			m_sActiveShape.pToolState = a_pState;
			if (!m_bInitializing)
			{
				CComBSTR bstr(m_bstrToolStateSync);
				bstr += m_sActiveShape.bstrTool;
				m_pStateMgr->StateSet(bstr, a_pState);
			}
		}
		return S_OK;
	}
	STDMETHOD(SetBrushState)(ULONG a_nShapeID, BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		if (a_nShapeID)
		{
			CShapes::iterator iSh = m_cShapes.find(a_nShapeID);
			if (iSh != m_cShapes.end())
			{
				iSh->second.pFillState = a_pState;
				if (!m_bInitializing && m_nActiveShape == a_nShapeID)
				{
					CComBSTR bstr(m_bstrToolStateSync);
					bstr += iSh->second.bstrFill;
					m_pStateMgr->StateSet(bstr, a_pState);
				}
			}
		}
		return S_OK;
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		if (a_ppDocument == NULL)
			return E_UNEXPECTED;
		(*a_ppDocument = m_pDoc)->AddRef();
		return S_OK;
	}
	STDMETHOD(Checkpoint)(ULONG a_nShapeID) { return E_NOTIMPL; }

	// IRasterEditView methods
public:
	STDMETHOD(CanApplyChanges)()
	{
		return m_sActiveShape.pTool ? m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL) : (m_cDirty.empty() ? S_FALSE : S_OK);
	}
	STDMETHOD(ApplyChanges)();
	STDMETHOD(ConfigureGestures)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		return m_pMGH ? m_pMGH->Configure(a_hParent ? a_hParent : m_hWnd, a_tLocaleID ? a_tLocaleID : m_tLocaleID, m_pConfig) : E_NOTIMPL;
	}

	// IDesignerViewUndoOverride methods
public:
	STDMETHOD(CanUndo)() { return CanApplyChanges(); }
	STDMETHOD(Undo)();
	STDMETHOD(UndoName)(ILocalizedString** a_ppName);
	STDMETHOD(CanRedo)() {return E_NOTIMPL;}
	STDMETHOD(Redo)() {return E_NOTIMPL;}
	STDMETHOD(RedoName)(ILocalizedString** a_ppName) { return E_NOTIMPL; }

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

	// handlers
public:
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnChangeFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMenuSelect(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

	int CanvasPadding() const { return 160*m_fScale; }

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
	void GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer);
	void PostRenderImage(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride);
	void GetPixelFromPoint(POINT a_tPoint, TPixelCoords* a_pPixel, TPixelCoords* a_pPointerSize = NULL, ULONG const* a_pControlPointIndex = NULL, EControlKeysState a_eKeysState = ECKSNone) const
	{
		float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
		float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
		a_pPixel->fX = (a_tPoint.x - M_ImagePos().x/*+ fX*//* - tZoomedSize.cx*0.5f*/) / fZoomX;
		a_pPixel->fY = (a_tPoint.y - M_ImagePos().y/*+ fY*//* - tZoomedSize.cy*0.5f*/) / fZoomY;
		TPixelCoords tPointerSize = {1.0f/fZoomX, 1.0f/fZoomY};
		if (a_pPointerSize) *a_pPointerSize = tPointerSize;
	}
	void ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime);
	void MouseMoved(TPixelCoords const& a_tPos)
	{
		m_tMousePos = a_tPos;
		if (m_tMousePos.fX != m_tSentPos.fX || m_tMousePos.fY != m_tSentPos.fY && m_pStatusBar)
			m_pStatusBar->Notify(0, 0);
	}
	void OnContextMenu(POINT const* a_pPoint);
	void UpdateScalableContent() { ControlLinesChanged(m_nActiveShape); }
	void ProcessGesture(ULONG a_nPoints, POINT const* a_pPoints);

private:
	struct SHelpLinePoint
	{
		float fX;
		float fY;
		bool bLineTo;
		bool bClose;
		BYTE type;
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
			m_type = ECLTSelection;
		}
		void SetType(BYTE type)
		{
			m_type = type;
		}

	BEGIN_COM_MAP(CControlLines)
		COM_INTERFACE_ENTRY(IEditToolControlLines)
	END_COM_MAP()

		// IEditToolControlLines methods
	public:
		STDMETHOD(MoveTo)(float a_fX, float a_fY)
		{
			SHelpLinePoint s = {a_fX, a_fY, false, false, m_type};
			m_pLines->push_back(s);
			return S_OK;
		}
		STDMETHOD(LineTo)(float a_fX, float a_fY)
		{
			SHelpLinePoint s = {a_fX, a_fY, true, false, m_type};
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
		BYTE m_type;
	};

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
		void SetFocusedState(BSTR a_bstrFocused = NULL, ISharedState* a_pFocused = NULL)
		{
			m_bstrFocused = a_bstrFocused;
			m_pFocused = a_pFocused;
		}


	BEGIN_COM_MAP(COperationContextFromStateManager)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// ISharedStateManager methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			if (m_pFocused && m_bstrFocused && m_bstrFocused == a_bstrCategoryName)
				return m_pFocused->QueryInterface(a_iid, a_ppState);
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
		CComBSTR m_bstrFocused;
		CComPtr<ISharedState> m_pFocused;
	};
	typedef std::vector<std::pair<TPixelCoords, ULONG> > CHandles;
	//typedef std::map<ULONG, SObjectInfo> CObjects;

	struct SShape
	{
		bool bEnable;
		CComBSTR bstrTool;
		CComPtr<IRasterImageEditTool> pTool;
		CComPtr<ISharedState> pToolState;

		BOOL bFill;
		CComBSTR bstrFill;
		CComPtr<IRasterImageBrush> pFill;
		CComPtr<ISharedState> pFillState;

		CComPtr<CComObject<CCallbackHelper<CDesignerViewVectorImageEditor> > > pCallback;

		ERasterizationMode eRM;
		ECoordinatesMode eCM;

		BOOL bOutline;
		TColor tColor;
		float fWidth;
		float fPos;
		EOutlineJoinType eJoins;
	};
	typedef std::map<ULONG, SShape> CShapes;

	enum EAutoUnselect
	{
		EAUSensing = 0,
		EAUWatching,
		EAUGone
	};

	enum EMouseState
	{
		EMSUnknown = 0,
		EMSModifying,
		EMSModifyingHandle,
		EMSModifyingForwarding,
		EMSModifyingSelecting,
		EMSTransforming,
		EMSCreating,
		EMSCreatingHandle,
		EMSCreatingForwarding,
	};

private:
	void UpdateZoom();
	void ActivateWindow();
	void DrawHelpLines(CHelpLines::const_iterator begin, CHelpLines::const_iterator end, float a_fZoomX, float a_fZoomY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride);

	void SetScrollOffset(int x, int y, BOOL bRedraw = TRUE);
	BOOL UpdateCursor();
	void UpdateImageSize();
	void UpdateImagePos();

	void ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos);
	void InsertMenuItems(IEnumUnknowns* a_pOps, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID);
	//bool IsSelected(ULONG a_nItemID);

	bool TestHandle(POINT const& a_tPos, ULONG* a_pIndex) const;
	bool TestHandle(TPixelCoords const& a_tPos, ULONG* a_pIndex) const;
	CShapes::const_iterator ShapeFromPoint(TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize) const;

	void RefreshShape(ULONG a_nID, SShape& a_sShape);
	void InvalidateHandles(CHandles const& a_cHandles);
	void InvalidateHelpLines(CHelpLines const& a_cHelpLines);
	void UpdateStates(SShape const& a_sShape);
	void RestoreToolStates();
	void SendDrawToolCmdLineUpdate();
	void SendDrawToolCmdLineUpdateLater();
	LRESULT OnUpdateToolCmdLine(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

private:
	// document
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentVectorImage> m_pImage;
	CComPtr<IDocumentImage> m_pImage2;
	CComPtr<IStructuredItemsRichGUI> m_pRichGUI;

	// infrastructure
	CComPtr<IConfig> m_pConfig;

	// context menu
	CComPtr<IMenuCommandsManager> m_pCmdMgr;
	CImageList m_cImageList;
	CImageMap m_cImageMap;
	CContextOps m_cContextOps;

	// window
	LCID m_tLocaleID;
	int m_nWheelDelta;

	// edit state
	bool m_bChangingSharedState;
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<CComObject<COperationContextFromStateManager> > m_pContext;
	CComBSTR m_bstrNewToolStateSync;
	CComBSTR m_bstrToolStateSync;
	CComBSTR m_bstrSelectionStateSync;
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
	ULONG m_nHandleIndex;
	HCURSOR m_hHandCursor;
	bool m_bTrackMouse;
	bool m_bMouseOut;
	ULONG m_nHotHandle;

	// status bar
	CComPtr<IStatusBarObserver> m_pStatusBar;
	bool m_bShowCommandDesc;
	CComBSTR m_bstrCommandDesc;
	TPixelCoords m_tMousePos;
	TPixelCoords m_tSentPos;
	CComBSTR m_bstrCoordsPane;

	// command line
	CComBSTR m_bstrToolCommandStateSync;
	bool m_bToolCmdLineUpdatePosted;
	bool m_bToolCmdLineUpdating;

	// viewport sync
	CComBSTR m_bstrViewportStateSync;

	HCURSOR m_hLastCursor;

	// mouse gestures
	CComPtr<IMouseGesturesHelper> m_pMGH;

	// selection
	std::vector<ULONG> m_aSelected;
	UINT m_nClipboardFormat;

	// editation
	CShapes m_cShapes;
	std::vector<ULONG> m_cShapeOrder;
	bool m_bInitializing;
	ULONG m_nHotShape;
	ULONG m_nActiveShape;
	std::set<ULONG> m_cDirty;
	CHelpLines m_cHotHelpLines;

	bool m_bTransforming;
	bool m_bTransformingShift;
	TPixelCoords m_tTransformStart;
	TPixelCoords m_tTransformCenter;
	std::map<ULONG, CComBSTR> m_cTransformStart;

	CComPtr<IRasterImageEditToolsManager> m_pToolMgr;
	CComPtr<IRasterImageFillStyleManager> m_pFillMgr;

	EMouseState m_eMS;
	TPixelCoords m_tLastPos;
	SShape m_sActiveShape;
	CHandles m_cHandles;
	CHelpLines m_cActiveHelpLines;
	ULONG m_iActiveHandle;
	TPixelCoords m_tHandleDragOffset;

	EAutoUnselect m_eAutoUnselect;
	TPixelCoords m_tAutoUnselect;

	bool m_bApplyFlag;
};

