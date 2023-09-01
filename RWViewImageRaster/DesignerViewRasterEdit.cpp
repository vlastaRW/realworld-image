// DesignerViewRasterEdit.cpp : Implementation of CDesignerViewRasterEdit

#include "stdafx.h"
#include "DesignerViewRasterEdit.h"

#include <XPGUI.h>
#include <Win32LangEx.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>

#include "ConfigIDsRasterEdit.h"
#include "SharedStateToolMode.h"

#include <math.h>
#include "EditToolNull.h"
#include <RWDesignerCore.h>


// CDesignerViewRasterEdit

void CDesignerViewRasterEdit::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, CWinTabWrapper* a_pWinTab, IViewManager* a_pViewManager, IDocumentRasterImage* a_pImage)
{
	m_pDoc = a_pDoc;
	m_pImage = a_pImage;

	TImageSize tSize = {1, 1};
	m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
	InitVisualization(a_pConfig, tSize.nX, tSize.nY);

	m_rcSelection.left = 0;
	m_rcSelection.top = 0;
	m_rcSelection.right = tSize.nX;
	m_rcSelection.bottom = tSize.nY;

	m_pWinTab = a_pWinTab;
	RWCoCreateInstance(m_pMGH, __uuidof(MouseGesturesHelper));

	a_pViewManager->QueryInterface(&m_pCmdMgr);
	if (m_pCmdMgr == NULL)
		RWCoCreateInstance(m_pCmdMgr, __uuidof(MenuCommandsManager));
	a_pViewManager->QueryInterface(&m_pOpMgr);
	if (m_pOpMgr == NULL)
		RWCoCreateInstance(m_pOpMgr, __uuidof(OperationManager));

	{
		CComObject<CCallbackHelper<IRasterImageEditWindow> >::CreateInstance(&m_pCallback);
		m_pCallback->AddRef();
		m_pCallback->SetOwner(this);
	}
	RWCoCreateInstance(m_pTools, __uuidof(RasterImageEditToolsManager));
	RWCoCreateInstance(m_pFills, __uuidof(RasterImageFillStyleManager));

	m_pStateMgr = a_pFrame;
	CComObject<COperationContextFromStateManager>::CreateInstance(&m_pContext.p);
	m_pContext.p->AddRef();
	m_pContext->Init(a_pFrame);

	m_tLocaleID = a_tLocaleID;
	m_pConfig = a_pConfig;

	CConfigValue cStateSync;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_TOOLSYNC), &cStateSync);
	m_bstrToolStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_SELECTIONSYNC), &cStateSync);
	m_bstrSelectionStateSync.Attach(cStateSync.Detach().bstrVal);
	//m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_FILLSTYLESYNC), &cStateSync);
	//m_bstrFillStyleStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_VIEWPORTSYNC), &cStateSync);
	m_bstrViewportStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_PASTETOOL), &cStateSync);
	m_bstrPasteTool.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_SELECTIONSUPPORT), &cStateSync);
	m_bSelectionSupport = cStateSync;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_TOOLCMDLINE), &cStateSync);
	m_bstrToolCommandStateSync.Attach(cStateSync.Detach().bstrVal);

	m_tOutlineColor.fR = m_tOutlineColor.fG = m_tOutlineColor.fB = 0.0f;
	m_tOutlineColor.fA = 1.0f;
	m_bOutline = false;
	m_eBlendingMode = EBMDrawOver;
	m_eRasterizationMode = ERMSmooth;
	m_eCoordinatesMode = ECMFloatingPoint;
	m_fOutlineWidth = 1.0f;

	//m_bstrToolID.Empty();
	if (m_pStateMgr)
	{
		InitImageEditorState(m_pStateMgr, a_pConfig);
		// initialize states
		if (m_bstrToolStateSync.Length())
		{
			CComPtr<ISharedStateToolMode> pState;
			m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pState));
			if (pState)
			{
				pState->Get(&m_bstrToolID, &m_bFill, &m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins);
			}
			else
			{
				RWCoCreateInstance(pState, __uuidof(SharedStateToolMode));
				CConfigValue cToolMode;
				m_pConfig->ItemValueGet(CComBSTR(CFGID_2DEDIT_TOOLMODE), &cToolMode);
				if (cToolMode.TypeGet() == ECVTString && cToolMode.operator BSTR())
				{
					pState->FromText(cToolMode);
					pState->Get(&m_bstrToolID, &m_bFill, &m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins);
				}
				else
				{
					m_bstrToolID = L"SELECT";
					m_bstrStyleID = L"SOLID";
					pState->Set(m_bstrToolID, &m_bFill, m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins, NULL);
				}
				m_pStateMgr->StateSet(m_bstrToolStateSync, pState);
			}
			CComPtr<IConfig> pSubCfg;
			m_pConfig->SubConfigGet(CComBSTR(CFGID_2DEDIT_EDITTOOLSTATES), &pSubCfg);
			if (pSubCfg)
			{
				CComPtr<IEnumStrings> pIDs;
				pSubCfg->ItemIDsEnum(&pIDs);
				ULONG nIDs = 0;
				if (pIDs) pIDs->Size(&nIDs);
				for (ULONG i = 0; i < nIDs; ++i)
				{
					CComBSTR bstrID;
					pIDs->Get(i, &bstrID);
					if (bstrID.m_str == NULL)
						continue;
					CComBSTR bstrFullID(m_bstrToolStateSync);
					bstrFullID += bstrID;
					CComPtr<ISharedState> pStateTool;
					m_pStateMgr->StateGet(bstrFullID, __uuidof(ISharedState), reinterpret_cast<void**>(&pStateTool));
					if (pStateTool.p)
						continue; // TODO: update config state?
					CConfigValue cVal;
					pSubCfg->ItemValueGet(bstrID, &cVal);
					if (cVal.TypeGet() != ECVTString)
						continue;
					CLSID tClsID;
					if (!GUIDFromString(cVal.operator BSTR(), &tClsID))
						continue;
					RWCoCreateInstance(pStateTool, tClsID);
					if (pStateTool.p == NULL)
						continue;
					if (SUCCEEDED(pStateTool->FromText(CComBSTR(cVal.operator BSTR()+36))))
						m_pStateMgr->StateSet(bstrFullID, pStateTool);
				}
			}
		}
		if (m_bstrSelectionStateSync.Length())
		{
			CComPtr<ISharedStateImageSelection> pMyState;
			m_pStateMgr->StateGet(m_bstrSelectionStateSync, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pMyState));
			if (pMyState)
			{
				LONG nX = 0;
				LONG nY = 0;
				ULONG nDX = 0;
				ULONG nDY = 0;
				pMyState->Bounds(&nX, &nY, &nDX, &nDY);
				RECT rcBounds = {nX, nY, nX+nDX, nY+nDY};
				bool bEntire = pMyState->IsEmpty() == S_OK;
				m_rcSelection = rcBounds;
				m_aSelection.Free();
				if (!bEntire && nDX && nDY)
				{
					CAutoVectorPtr<BYTE> pNew(new BYTE[nDX*nDY]);
					pMyState->GetTile(nX, nY, nDX, nDY, nDX, pNew);
					std::swap(pNew.m_p, m_aSelection.m_p);
					OptimizeSelection(m_aSelection, &m_rcSelection);
				}
				else if (bEntire)
				{
					m_rcSelection.left = 0;
					m_rcSelection.top = 0;
					m_rcSelection.right = M_ImageSize().cx;
					m_rcSelection.bottom = M_ImageSize().cy;
				}
			}
		}
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
					m_bChangingSharedState = true;
					if (bAutoZoom)
						MoveViewport();
					else
						MoveViewport(fOffsetX, fOffsetY, fZoom);
					m_bChangingSharedState = false;
					//m_fOffsetX = fOffsetX;
					//m_fOffsetY = fOffsetY;
					//M_ImageZoom() = fZoom;
					//m_bAutoZoom = bAutoZoom;
					////UpdatePosition();
				}
			}
		}
		m_pStateMgr->ObserverIns(CObserverImpl<CDesignerViewRasterEdit, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}
	if (m_bstrStyleID.Length())
		m_pFills->FillStyleCreate(m_bstrStyleID, a_pDoc, &m_pActiveBrush);
	if (m_bstrToolID == NULL)
		m_bstrToolID = L"SELECT";
	m_pTools->EditToolCreate(m_bstrToolID, a_pDoc, &m_pActiveTool);
	if (m_pActiveTool == NULL)
	{
		m_bstrToolID.Empty();
		CComObject<CEditToolNull>* p = NULL;
		CComObject<CEditToolNull>::CreateInstance(&p);
		m_pActiveTool = p;
	}
	InitFill();
	InitTool();

	m_pImage->ObserverIns(CObserverImpl<CDesignerViewRasterEdit, IImageObserver, TImageChange>::ObserverGet(), 0);

	m_pStatusBar = a_pStatusBar;

	m_dwLastMessage = GetTickCount()-50000;
	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("ImageEditFrame"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, (a_nStyle&EDVWSBorderMask) != EDVWSBorder ? 0 : WS_EX_CLIENTEDGE) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}

	MoveWindow(const_cast<LPRECT>(a_rcWindow));
	ShowWindow(SW_SHOW);
}

void CDesignerViewRasterEdit::InitRTS()
{
	m_pRTS.CoCreateInstance(__uuidof(RealTimeStylus));
	if (!m_pRTS)
		return;
    HRESULT hRes = m_pRTS->put_HWND(reinterpret_cast<HANDLE_PTR>(m_hWnd));
	if (FAILED(hRes))
	{
		m_pRTS = NULL;
		return;
	}

	m_pSSP = new CRWStylusSyncPlugin(m_hWnd);

    hRes = m_pRTS->AddStylusSyncPlugin(0, m_pSSP);
    if (FAILED(hRes))
    {
		m_pSSP->WindowDestroyed();
		m_pSSP = NULL;
		m_pRTS = NULL;
        return;
    }

	GUID lWantedProps[] = {GUID_PACKETPROPERTY_GUID_X, GUID_PACKETPROPERTY_GUID_Y, GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE};
	m_pRTS->SetDesiredPacketDescription(3, lWantedProps);
	m_pRTS->put_Enabled(true);

	if (!m_pSSP->InitTablets(m_pRTS))
	{
		m_pRTS->RemoveStylusSyncPlugin(0, NULL);
		m_pSSP->WindowDestroyed();
		m_pSSP = NULL;
		m_pRTS = NULL;
	}
}

LRESULT CDesignerViewRasterEdit::OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
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
	SendDrawToolCmdLineUpdate();

	InitRTS();

	if (m_pRTS == NULL && m_pWinTab->WTInfo(0, 0, NULL))
	{
		tagAXIS TpOri[3]; // The capabilities of tilt
		UINT tilt_support = m_pWinTab->WTInfo(WTI_DEVICES, DVC_ORIENTATION, &TpOri);
		//if (TpOri[0].axResolution && TpOri[1].axResolution) {

		tagAXIS TabletX, TabletY, NPressure;
		m_pWinTab->WTInfo(WTI_DEVICES, DVC_X, &TabletX);
		m_pWinTab->WTInfo(WTI_DEVICES, DVC_Y, &TabletY);
		m_pWinTab->WTInfo(WTI_DEVICES, DVC_NPRESSURE, &NPressure);
		m_fTabletXZoom = 1.0f/(TabletX.axMax+1);
		m_fTabletYZoom = 1.0f/(TabletY.axMax+1);
		m_fTabletPressureZoom = 1.0f/NPressure.axMax;
		//	/* convert azimuth resulution to double */
		//	tpvar = FIX_DOUBLE(TpOri[0].axResolution);
		//	/* convert from resolution to radians */
		//	aziFactor = tpvar/(2*pi);  
		//	
		//	/* convert altitude resolution to double */
		//	tpvar = FIX_DOUBLE(TpOri[1].axResolution);
		//	/* scale to arbitrary value to get decent line length */ 
		//	altFactor = tpvar/1000; 
		//	 /* adjust for maximum value at vertical */
		//	altAdjust = (double)TpOri[1].axMax/altFactor;
		//}
		//else {  /* no so dont do tilt stuff */
		//	tilt_support = FALSE;
		//}

		LOGCONTEXT tCtx;
		ZeroMemory(&tCtx, sizeof tCtx);
		m_pWinTab->WTInfo(WTI_DEFCONTEXT, 0, &tCtx);
		_stprintf(tCtx.lcName, _T("RWRasterEditor%x"), m_hWnd);
		tCtx.lcOptions |= CXO_SYSTEM|CXO_MESSAGES;
	//tCtx.lcStatus = CXS_ONTOP;
	//	tCtx.lcLocks = CXL_INSIZE;//CXL_INASPECT;
	//tCtx.lcMsgBase;
	//tCtx.lcDevice;
	//tCtx.lcPktRate;
		tCtx.lcPktData = PACKETDATA;
		tCtx.lcPktMode = PACKETMODE;
		tCtx.lcMoveMask = PACKETDATA;
		tCtx.lcBtnUpMask = tCtx.lcBtnDnMask;
	//tCtx.lcInOrgX;
	//tCtx.lcInOrgY;
	//tCtx.lcInOrgZ;
	//tCtx.lcInExtX;
	//tCtx.lcInExtY;
	//tCtx.lcInExtZ;
	//tCtx.lcOutOrgX;
	//tCtx.lcOutOrgY;
	//tCtx.lcOutOrgZ;
	//tCtx.lcOutExtX;
	//tCtx.lcOutExtY;
	//tCtx.lcOutExtZ;
	//tCtx.lcSensX;
	//tCtx.lcSensY;
	//tCtx.lcSensZ;
	//tCtx.lcSysMode;
	//tCtx.lcSysOrgX;
	//tCtx.lcSysOrgY;
	//tCtx.lcSysExtX;
	//tCtx.lcSysExtY;
	//tCtx.lcSysSensX;
	//tCtx.lcSysSensY;
		tCtx.lcInOrgX = 0;
		tCtx.lcInOrgY = 0;
		tCtx.lcInExtX = TabletX.axMax+1;
		tCtx.lcInExtY = TabletY.axMax+1;
		tCtx.lcOutOrgX = tCtx.lcOutOrgY = 0;
		tCtx.lcOutExtX = TabletX.axMax+1;
		tCtx.lcOutExtY = -TabletY.axMax-1;
		//// ugly hack -> maybe workaroung a wintab bug???
		//if (tCtx.lcSysExtX)
		//{
		//	m_fTabletXZoom /= GetSystemMetrics(SM_CXSCREEN);
		//	m_fTabletXZoom *= tCtx.lcSysExtX;
		//}
		//if (tCtx.lcSysExtY)
		//{
		//	m_fTabletYZoom /= GetSystemMetrics(SM_CYSCREEN);
		//	m_fTabletYZoom *= tCtx.lcSysExtY;
		//}
		m_hWTCtx = m_pWinTab->WTOpen(m_hWnd, &tCtx, TRUE);
	}
	ControlPointsChanged();
	ControlLinesChanged();

	AddRef();

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pRTS)
	{
		m_pRTS->RemoveStylusSyncPlugin(0, NULL);
		m_pRTS = NULL;
		if (m_pSSP)
		{
			m_pSSP->WindowDestroyed();
			m_pSSP = NULL;
		}
	}

	if (m_hWTCtx)
		m_pWinTab->WTClose(m_hWTCtx);
	if (m_pImage)
	{
		m_pImage->ObserverDel(CObserverImpl<CDesignerViewRasterEdit, IImageObserver, TImageChange>::ObserverGet(), 0);
		m_pImage = NULL;
	}
	if (m_pStateMgr)
	{
		m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewRasterEdit, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		m_pStateMgr = NULL;
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewRasterEdit::OnWTPacket(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_dwLastMessage = GetTickCount();
	PACKET tPkt;
	if (m_hWTCtx && m_pWinTab->WTPacket(m_hWTCtx, a_wParam, &tPkt))
	{
		bool bPrevDown = m_bTabletDown;
		float fPressure = tPkt.pkNormalPressure*m_fTabletPressureZoom;
		if (bPrevDown)
		{
			if (fPressure < 0.01f)
				m_bTabletDown = false;
		}
		else
		{
			if (fPressure > 0.5f)
				m_bTabletDown = true;
		}
		if (fPressure < 0.01f)
		{
			m_bTabletBlock = false;
			fPressure = 0.0f;
		}
		if (m_eDragState != EDSNothing)
			return 0; // something is already happening with the mouse

		POINT tPt = {0, 0};
		ScreenToClient(&tPt);
		TPixelCoords tCoords = {tPkt.pkX*m_fTabletXZoom*GetSystemMetrics(SM_CXSCREEN)+tPt.x, tPkt.pkY*m_fTabletYZoom*GetSystemMetrics(SM_CYSCREEN)+tPt.y};
		if (tCoords.fX < 0.0f || tCoords.fY < 0.0f ||
			tCoords.fX >= M_WindowSize().cx || tCoords.fY >= M_WindowSize().cy)
		{
			if (m_bTabletDown && !bPrevDown)
				m_bTabletBlock = true;
			if (fPressure < 0.01f && !m_bPenLeft)
			{
				m_bPenLeft = true;
				DWORD dw;
				m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
				HRESULT hRes = m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
				if (hRes&ETPAApply)
					ApplyChanges();
				if ((hRes&ETPAActionMask) == ETPAStartNew)
					m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
			}
			return 0; // not over the client area
		}
		m_bPenLeft = false;

		if (m_bTabletDown && !bPrevDown)
		{
			POINT pt = {tCoords.fX+0.5f, tCoords.fY+0.5f};
			if (TestHandle(pt, &m_nHandleIndex))
			{
				m_eDragState = EDSHandle;
				SetCapture();
				return 0;
			}
		}

		if (m_bTabletBlock)
			return 0;

		//SetCursorPos(tPkt.pkX*m_fTabletXZoom*GetSystemMetrics(SM_CXSCREEN), tPkt.pkY*m_fTabletYZoom*GetSystemMetrics(SM_CYSCREEN));
		tCoords.fX = (tCoords.fX - M_ImagePos().x) / M_ImageZoom();
		tCoords.fY = (tCoords.fY - M_ImagePos().y) / M_ImageZoom();
		TPixelCoords tPointerSize = {1.0f/M_ImageZoom(), 1.0f/M_ImageZoom()};
		EControlKeysState eCKS = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
		m_pActiveTool->AdjustCoordinates(eCKS, &tCoords, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
		DWORD dw = 0;
		HRESULT hRes = m_pActiveTool->ProcessInputEvent(eCKS, &tCoords, &tPointerSize, m_fLastPressure = fPressure, tPkt.pkTangentPressure, tPkt.pkOrientation.orAzimuth, -1.0f, -1.0f, &dw);
		if (hRes&ETPAApply)
			ApplyChanges();
		if ((hRes&ETPAActionMask) == ETPAStartNew)
			m_pActiveTool->ProcessInputEvent(eCKS, &tCoords, &tPointerSize, m_fLastPressure = fPressure, tPkt.pkTangentPressure, tPkt.pkOrientation.orAzimuth, -1.0f, -1.0f, &dw);
	}
	return 0;
}

LRESULT CDesignerViewRasterEdit::OnStylusPacket(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_dwLastMessage = GetTickCount();
	if (m_pSSP)
	{
		m_pSSP->m_iReceived = m_pSSP->m_iSent;
		bool bPrevDown = m_bTabletDown;
		float fPressure = m_pSSP->m_sPacket.n;
		if (bPrevDown)
		{
			if (fPressure < 0.01f)
				m_bTabletDown = false;
		}
		else
		{
			if (fPressure > 0.5f)
				m_bTabletDown = true;
		}
		if (fPressure < 0.01f)
		{
			m_bTabletBlock = false;
			fPressure = 0.0f;
		}
		if (m_eDragState != EDSNothing)
			return 0; // something is already happening with the mouse

		POINT tPt = {0, 0};
		//ScreenToClient(&tPt);
		TPixelCoords tCoords = {m_pSSP->m_sPacket.x*GetSystemMetrics(SM_CXSCREEN)+tPt.x, m_pSSP->m_sPacket.y*GetSystemMetrics(SM_CYSCREEN)+tPt.y};
		if (tCoords.fX < 0.0f || tCoords.fY < 0.0f ||
			tCoords.fX >= M_WindowSize().cx || tCoords.fY >= M_WindowSize().cy)
		{
			if (m_bTabletDown && !bPrevDown)
				m_bTabletBlock = true;
			if (fPressure < 0.01f && !m_bPenLeft)
			{
				m_bPenLeft = true;
				DWORD dw;
				m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
				HRESULT hRes = m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
				if (hRes&ETPAApply)
					ApplyChanges();
				if ((hRes&ETPAActionMask) == ETPAStartNew)
					m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
			}
			return 0; // not over the client area
		}
		m_bPenLeft = false;

		if (m_bTabletDown && !bPrevDown)
		{
			POINT pt = {tCoords.fX+0.5f, tCoords.fY+0.5f};
			if (TestHandle(pt, &m_nHandleIndex))
			{
				m_eDragState = EDSHandle;
				SetCapture();
				return 0;
			}
		}

		if (m_bTabletBlock)
			return 0;

		//SetCursorPos(tPkt.pkX*m_fTabletXZoom*GetSystemMetrics(SM_CXSCREEN), tPkt.pkY*m_fTabletYZoom*GetSystemMetrics(SM_CYSCREEN));
		tCoords.fX = (tCoords.fX - M_ImagePos().x) / M_ImageZoom();
		tCoords.fY = (tCoords.fY - M_ImagePos().y) / M_ImageZoom();
		TPixelCoords tPointerSize = {1.0f/M_ImageZoom(), 1.0f/M_ImageZoom()};
		EControlKeysState eCKS = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
		m_pActiveTool->AdjustCoordinates(eCKS, &tCoords, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
		DWORD dw = 0;
		HRESULT hRes = m_pActiveTool->ProcessInputEvent(eCKS, &tCoords, &tPointerSize, m_fLastPressure = fPressure, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
		if (hRes&ETPAApply)
			ApplyChanges();
		if ((hRes&ETPAActionMask) == ETPAStartNew)
			m_pActiveTool->ProcessInputEvent(eCKS, &tCoords, &tPointerSize, m_fLastPressure = fPressure, 0.0f, 0.0f, -1.0f, -1.0f, &dw);
	}
	return 0;
}

LRESULT CDesignerViewRasterEdit::OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);
	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	ScreenToClient(&tPt);

	while (m_nWheelDelta >= WHEEL_DELTA)
	{
		m_nWheelDelta -= WHEEL_DELTA;
		ZoomIn(tPt);
		SendViewportUpdate();
	}
	while ((-m_nWheelDelta) >= WHEEL_DELTA)
	{
		m_nWheelDelta += WHEEL_DELTA;
		ZoomOut(tPt);
		SendViewportUpdate();
	}

	return 0;
}

void CDesignerViewRasterEdit::OwnerNotify(TCookie UNREF(a_tCookie), TImageChange a_tChange)
{
	try
	{
		if (a_tChange.nGlobalFlags & EICDimensions)
		{
			TImageSize tSize = {1, 1};
			m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			ResizeImage(tSize);
			m_rcSelection.left = 0;
			m_rcSelection.top = 0;
			m_rcSelection.right = tSize.nX;
			m_rcSelection.bottom = tSize.nY;
			m_aSelection.Free();
			m_pActiveTool->Reset();
			SendSelectionUpdate();
			//RECT rc = {0, 0, M_ImageSize().cx, M_ImageSize().cy};
			//RectangleChanged(&rc);
		}
		else if (a_tChange.tSize.nX*a_tChange.tSize.nY > 0)
		{
				m_pActiveTool->Reset();
				RECT rc = {a_tChange.tOrigin.nX, a_tChange.tOrigin.nY, a_tChange.tOrigin.nX+a_tChange.tSize.nX, a_tChange.tOrigin.nY+a_tChange.tSize.nY};
				RectangleChanged(&rc);
		}
	}
	catch (...)
	{
	}
}

void CDesignerViewRasterEdit::OwnerNotify(TCookie, TSharedStateChange a_tState)
{
	try
	{
		if (m_bChangingSharedState)
			return;

		if (m_bstrViewportStateSync == a_tState.bstrName)
		{
			if (!M_AutoZoom() && !m_bChangingSharedState)
			{
				CComQIPtr<ISharedStateImageViewport> pMyState(a_tState.pState);
				float fOX = M_OffsetX();
				float fOY = M_OffsetY();
				pMyState->Get(&fOX, &fOY, /*&M_ImageZoom()*/NULL, NULL, NULL);
				if (fOX != M_OffsetX() || fOY != M_OffsetY())
				{
					m_bChangingSharedState = true;
					MoveViewport(fOX, fOY, M_ImageZoom());
					m_bChangingSharedState = false;
				}

				//pMyState->Get(&m_fOffsetX, &m_fOffsetY, /*&M_ImageZoom()*/NULL, NULL, NULL);
				//UpdatePosition();
			}
		}
		else if (m_bstrToolCommandStateSync == a_tState.bstrName)
		{
			if (m_eDragState == EDSNothing)
				return;

			CComBSTR bstr;
			if (a_tState.pState)
				a_tState.pState->ToText(&bstr);
			ULONG nLen = bstr.Length();
			LPWSTR psz = bstr;
			LPWSTR pArgs = wcschr(psz, L'\n');
			if (pArgs && psz < pArgs)
			{
				*pArgs = L'\0';
				++pArgs;
				bool bOK = true;
				if (m_bstrToolID != psz)
				{
					CComBSTR bstrToolID(psz);
					//ApplyChanges(FALSE);
					CComPtr<IRasterImageEditTool> pNewTool;
					m_pTools->EditToolCreate(bstrToolID, m_pDoc, &pNewTool);
					if (pNewTool)
					{
						m_pActiveTool->Reset();
						m_pActiveTool = pNewTool;
						m_bstrToolID = bstrToolID;
						InitTool();
						ControlPointsChanged();
						ControlLinesChanged();

						CComPtr<ISharedStateToolMode> pPrev;
						m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
						CComObject<CSharedStateToolMode>* pNew = NULL;
						CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
						CComPtr<ISharedState> pTmp = pNew;
						if (S_OK == pNew->Set(m_bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
						{
							m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
						}
					}
					else
					{
						bOK = false;
					}
				}
				if (bOK)
				{
					CComQIPtr<IRasterImageEditToolScripting> pScript(m_pActiveTool);
					if (pScript)
					{
						CComBSTR bstrCur;
						pScript->ToText(&bstrCur);
						CComBSTR bstrNew(pArgs);
						if (bstrCur != bstrNew)
						{
							m_bToolCmdLineUpdating = true;
							pScript->FromText(bstrNew);
							m_bToolCmdLineUpdating = false;
						}
					}
				}
			}
		}
		else if (m_bstrSelectionStateSync == a_tState.bstrName)
		{
			CComQIPtr<ISharedStateImageSelection> pMyState(a_tState.pState);
			if (pMyState == NULL)
				return;
			LONG nX = 0;
			LONG nY = 0;
			ULONG nDX = 0;
			ULONG nDY = 0;
			pMyState->Bounds(&nX, &nY, &nDX, &nDY);
			RECT rcBounds = {nX, nY, nX+nDX, nY+nDY};
			bool bEntire = pMyState->IsEmpty() == S_OK;
			m_rcSelection = rcBounds;
			m_aSelection.Free();
			if (!bEntire && nDX && nDY)
			{
				CAutoVectorPtr<BYTE> pNew(new BYTE[nDX*nDY]);
				pMyState->GetTile(nX, nY, nDX, nDY, nDX, pNew);
				std::swap(pNew.m_p, m_aSelection.m_p);
				OptimizeSelection(m_aSelection, &m_rcSelection);
			}
			else if (nDX == 0 || nDY == 0)
				m_rcSelection = RECT_EMPTY;
			Invalidate(FALSE);
		}
		else
		{
			LPCWSTR p1 = m_bstrToolStateSync;
			LPCWSTR p2 = a_tState.bstrName;
			while (*p1 == *p2 && *p1) { ++p1; ++p2; }
			if (*p1 == L'\0')
			{
				if (*p2)
				{
					if (m_bstrToolID == p2)
					{
						m_pActiveTool->SetState(a_tState.pState);
					}
					else if (m_bstrStyleID == p2)
					{
						m_pActiveBrush->SetState(a_tState.pState);
					}
					CLSID tID;
					CComBSTR bstrData;
					if (SUCCEEDED(a_tState.pState->CLSIDGet(&tID)) && SUCCEEDED(a_tState.pState->ToText(&bstrData)) && bstrData.m_str)
					{
						OLECHAR szTmp[64];
						StringFromGUID(tID, szTmp);
						CComBSTR bstr(szTmp);
						bstr += bstrData;
						CComPtr<IConfig> pSubCfg;
						m_pConfig->SubConfigGet(CComBSTR(CFGID_2DEDIT_EDITTOOLSTATES), &pSubCfg);
						CComBSTR bstrTool(p2);
						pSubCfg->ItemValuesSet(1, &(bstrTool.m_str), CConfigValue(bstr));
					}
				}
				else
				{
					CComQIPtr<ISharedStateToolMode> pMyState(a_tState.pState);
					if (pMyState == NULL)
						return;

					CComBSTR bstrPersistent;
					if (SUCCEEDED(pMyState->ToText(&bstrPersistent)) && bstrPersistent.m_str)
					{
						CComBSTR bstrID(CFGID_2DEDIT_TOOLMODE);
						m_pConfig->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(bstrPersistent));
					}

					CComBSTR bstrToolID;
					CComBSTR bstrStyleID;
					BOOL bFill = m_bFill;
					pMyState->Get(&bstrToolID, &m_bFill, &bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins);
					//std::vector<ULONG> aPaints;
					//m_pTools->SupportedStates(bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
					//bool bRasterImage = false;
					//bool bSolidColor = false;
					//bool bOutline = false;
					//for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
					//{
					//	if ((*i&ETBTIdentifierMask) == ETBTIdentifierInterior)
					//	{
					//		if ((*i&ETBTTypeMask) == ETBTSingleColor)
					//			bSolidColor = true;
					//		if ((*i&ETBTTypeMask) == ETBTRasterImage)
					//			bRasterImage = true;
					//	}
					//	else if ((*i&ETBTIdentifierMask) == ETBTIdentifierOutline)
					//		bOutline = true;
					//}

					bool const bToolChange = bstrToolID.Length() && bstrToolID != m_bstrToolID;
					if (bToolChange)
					{
						ApplyChanges(FALSE);
					}
					if (bstrStyleID.Length() && bstrStyleID != m_bstrStyleID)
					{
						CComPtr<IRasterImageBrush> pNewStyle;
						m_pFills->FillStyleCreate(bstrStyleID, m_pDoc, &pNewStyle);
						if (pNewStyle)
						{
							m_pActiveBrush = pNewStyle;
							m_bstrStyleID = bstrStyleID;
							CComBSTR bstr(m_bstrToolStateSync);
							bstr += bstrStyleID;
							CComPtr<ISharedState> pState;
							m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
							if (pState)
								m_pActiveBrush->SetState(pState);
							if (!bToolChange)
							{
								m_pActiveTool->SetBrush(m_bFill ? m_pActiveBrush.p : NULL);
								bFill = m_bFill;
								ControlPointsChanged();
								ControlLinesChanged();
							}
						}
					}
					if (bToolChange)
					{
						CComPtr<IRasterImageEditTool> pNewTool;
						m_pTools->EditToolCreate(bstrToolID, m_pDoc, &pNewTool);
						if (pNewTool)
						{
							m_pActiveTool = pNewTool;
							m_bstrToolID = bstrToolID;
							InitTool();
							ControlPointsChanged();
							ControlLinesChanged();
						}
					}
					else
					{
						if (bFill != m_bFill)
							m_pActiveTool->SetBrush(m_bFill ? m_pActiveBrush.p : NULL);
						m_pActiveTool->SetGlobals(m_eBlendingMode, m_eRasterizationMode, m_eCoordinatesMode);
						m_pActiveTool->SetOutline(m_bOutline, m_fOutlineWidth, m_fOutlinePos, m_eOutlineJoins, &m_tOutlineColor);
					}
				}
				return;
			}
		}
	}
	catch (...)
	{
	}
}

#include "EditToolPixelMixer.h"

STDMETHODIMP CDesignerViewRasterEdit::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	try
	{
		if (!a_bBeforeAccel || a_pMsg->hwnd != m_hWnd)
			return S_FALSE;
		if (m_pActiveTool)
		{
			HRESULT hRes = m_pActiveTool->PreTranslateMessage(a_pMsg);
			if (S_OK == hRes)
				return S_OK;
			if (hRes&ETPAApply)
			{
				ApplyChanges();
				return S_OK;
			}
		}
		//if (a_pMsg->message == WM_KEYDOWN && a_pMsg->wParam == VK_DELETE && m_pActiveTool)
		//{
		//	// delete selected region if tool is selection exists or is in progress
		//	RECT rcImage = RECT_EMPTY;
		//	BOOL bAll = FALSE;
		//	RECT rcSelection = RECT_EMPTY;
		//	if (S_OK == m_pActiveTool->IsDirty(&rcImage, &bAll, &rcSelection))
		//	{
		//		if (rcImage.left < rcImage.right && rcImage.top < rcImage.bottom)
		//			return S_FALSE; // tool changes image -> no deleting
		//		rcSelection.left = 0;
		//		rcSelection.top = 0;
		//		rcSelection.right = M_ImageSize().cx;
		//		rcSelection.bottom = M_ImageSize().cy;
		//		bAll = TRUE;
		//		m_pActiveTool->GetSelectionInfo(&rcSelection, &bAll);
		//		if (rcSelection.left == 0 && rcSelection.top == 0 &&
		//			rcSelection.right == M_ImageSize().cx && rcSelection.bottom == M_ImageSize().cy &&
		//			bAll)
		//			return S_FALSE; // everything will be selected -> do noting
		//		ApplyChanges();
		//	}
		//	rcSelection.left = 0;
		//	rcSelection.top = 0;
		//	rcSelection.right = M_ImageSize().cx;
		//	rcSelection.bottom = M_ImageSize().cy;
		//	bAll = TRUE;
		//	GetSelectionInfo(&rcSelection, &bAll);
		//	if (rcSelection.left == 0 && rcSelection.top == 0 &&
		//		rcSelection.right == M_ImageSize().cx && rcSelection.bottom == M_ImageSize().cy &&
		//		bAll)
		//		return S_FALSE; // everything is selected -> do noting
		//	ULONG const nPixels = (rcSelection.right-rcSelection.left)*(rcSelection.bottom-rcSelection.top);
		//	CPixelChannel const tEmpty(0, 0, 0, 0); // TODO: use default?
		//	CImagePoint const tOrig(rcSelection.left, rcSelection.top);
		//	CImageSize const tSize(rcSelection.right-rcSelection.left, rcSelection.bottom-rcSelection.top);
		//	if (bAll)
		//	{
		//		// erase rectangle
		//		m_pImage->TileSet(EICIRGBA, tOrig, tSize, CImageStride(0, 0), 1, &tEmpty, FALSE);
		//	}
		//	else if (nPixels)
		//	{
		//		// erase with mask
		//		CAutoVectorPtr<TRasterImagePixel> cData(new TRasterImagePixel[nPixels]);
		//		CAutoVectorPtr<BYTE> cSel(new BYTE[nPixels]);
		//		GetSelectionTile(rcSelection.left, rcSelection.top, rcSelection.right-rcSelection.left, rcSelection.bottom-rcSelection.top, rcSelection.right-rcSelection.left, cSel);
		//		m_pImage->TileGet(EICIRGBA, &tOrig, &tSize, NULL, nPixels, reinterpret_cast<TPixelChannel*>(cData.m_p), NULL, EIRIAccurate);
		//		static TRasterImagePixel const tEmpty2 = {0, 0, 0, 0};
		//		for (ULONG i = 0; i < nPixels; ++i)
		//			if (cSel[i])
		//				CPixelMixerReplace::Mix(cData[i], tEmpty2, cSel[i]);
		//		m_pImage->TileSet(EICIRGBA, &tOrig, &tSize, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(cData.m_p), FALSE);
		//	}
		//	m_rcSelection.left = 0;
		//	m_rcSelection.top = 0;
		//	m_rcSelection.right = M_ImageSize().cx;
		//	m_rcSelection.bottom = M_ImageSize().cy;
		//	m_aSelection.Free();
		//	SendSelectionUpdate();
		//	RectangleChanged(NULL);
		//	return S_OK;
		//}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
	return  S_FALSE;
}

STDMETHODIMP CDesignerViewRasterEdit::OnDeactivate(BOOL a_bCancelChanges)
{
	try
	{
		if (!a_bCancelChanges)
			ApplyChanges(FALSE);
		m_pActiveTool->Reset();
		ControlPointsChanged();
		ControlLinesChanged();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanCut()
{
	try
	{
		if (m_bSelectionSupport)
			return S_OK;
		CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
		if (pSel == NULL)
			return S_FALSE;
		ULONG nX = 0;
		ULONG nY = 0;
		pSel->Size(&nX, &nY);
		return nX && nY ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanCopy()
{
	return CanCut();
}

STDMETHODIMP CDesignerViewRasterEdit::CanPaste()
{
	return IsClipboardFormatAvailable(CF_BITMAP) || IsClipboardFormatAvailable(CF_DIBV5) ? S_OK : S_FALSE;
}

#include "EditToolPixelMixer.h"

STDMETHODIMP CDesignerViewRasterEdit::Cut()
{
	try
	{
		HRESULT hRes = Copy();
		if (FAILED(hRes)) return hRes;

		return Delete();
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ImageClipboardUtils.h>

HRESULT CDesignerViewRasterEdit::FinishCopy(HANDLE hMem, BITMAPV5HEADER* pHeader, ULONG nSizeX, ULONG nSizeY)
{
	ZeroMemory(pHeader, sizeof *pHeader);
	pHeader->bV5Size = sizeof *pHeader;
	pHeader->bV5Width = nSizeX;
	pHeader->bV5Height = -nSizeY;
	pHeader->bV5Planes = 1;
	pHeader->bV5BitCount = 32;
	pHeader->bV5Compression = BI_BITFIELDS;
	pHeader->bV5SizeImage = 4*nSizeX*nSizeY;
	//pHeader->bV5XPelsPerMeter = 0;
	//pHeader->bV5YPelsPerMeter = 0;
	//pHeader->bV5ClrUsed = 0;
	//pHeader->bV5ClrImportant = 0;
	pHeader->bV5RedMask = 0x00ff0000;
	pHeader->bV5GreenMask = 0x0000ff00;
	pHeader->bV5BlueMask = 0x000000ff;
	pHeader->bV5AlphaMask = 0xff000000;
	pHeader->bV5CSType = LCS_sRGB;
	pHeader->bV5Intent = LCS_GM_IMAGES;

	CClipboardHandler cClipboard(*this);
	EmptyClipboard();
	SetClipboardData(CF_DIBV5, hMem);

	//HDC hDCWin = GetDC();
	//HDC hDC = CreateCompatibleDC(hDCWin);
	//HBITMAP hBmp = CreateCompatibleBitmap(hDCWin, nSizeX, nSizeY);
	//HGDIOBJ hPrevBmp = SelectObject(hDC, hBmp);
	//BITMAPINFO tBI;
	//ZeroMemory(&tBI, sizeof tBI);
	//tBI.bmiHeader.biSize = sizeof tBI.bmiHeader;
	//tBI.bmiHeader.biWidth = nSizeX;
	//tBI.bmiHeader.biHeight = nSizeY;
	//tBI.bmiHeader.biPlanes = 1;
	//tBI.bmiHeader.biBitCount = 24;
	//tBI.bmiHeader.biCompression = BI_RGB;
	//int nRowLen = (tBI.bmiHeader.biWidth*3+3)&0xfffffffc;
	//tBI.bmiHeader.biSizeImage = nRowLen*nSizeY;
	//CAutoVectorPtr<BYTE> cBmpData(new BYTE[tBI.bmiHeader.biSizeImage]);
	//for (ULONG y = 0; y < nSizeY; ++y)
	//{
	//	TRasterImagePixel* pS = reinterpret_cast<TRasterImagePixel*>(pHeader+1)+y*nSizeX;
	//	BYTE* pD = cBmpData.m_p + nRowLen*(nSizeY-y-1);
	//	for (ULONG x = 0; x < nSizeX; ++x)
	//	{
	//		pD[0] = pS->bB;
	//		pD[1] = pS->bG;
	//		pD[2] = pS->bR;
	//		pD += 3;
	//		++pS;
	//	}
	//}
	//SetDIBitsToDevice(hDC, 0, 0, nSizeX, nSizeY, 0, 0, 0, nSizeY, cBmpData.m_p, &tBI, DIB_RGB_COLORS);
	//SelectObject(hDC, hPrevBmp);
	//DeleteDC(hDC);
	//ReleaseDC(hDCWin);
	//SetClipboardData(CF_BITMAP, hBmp);

	GlobalUnlock(hMem);

	return S_OK;
}

STDMETHODIMP CDesignerViewRasterEdit::Copy()
{
	try
	{
		if (!IsWindow())
			return E_FAIL;
		CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
		if (pSel)
		{
			// copy floating selection from active tool
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			pSel->Size(&nSizeX, &nSizeY);
			if (nSizeX && nSizeY)
			{
				HANDLE hMem = GlobalAlloc(GHND, sizeof(BITMAPV5HEADER)+(nSizeX*nSizeY)*sizeof(TRasterImagePixel)+3*sizeof(RGBQUAD));
				if (hMem == NULL)
					return E_FAIL;
				BITMAPV5HEADER* pHeader = reinterpret_cast<BITMAPV5HEADER*>(GlobalLock(hMem));
				pSel->Data(0, 0, nSizeX, nSizeY, nSizeX, reinterpret_cast<TRasterImagePixel*>(pHeader+1));
				return FinishCopy(hMem, pHeader, nSizeX, nSizeY);
			}
		}
		if (m_bSelectionSupport)
		{
			// copy selected portion of image
			CAutoVectorPtr<BYTE> cMask(new BYTE[M_ImageSize().cx*M_ImageSize().cy]);
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			HANDLE hMem = NULL;
			BITMAPV5HEADER* pHeader = NULL;
			if (SUCCEEDED(m_pActiveTool->GetSelectionTile(0, 0, M_ImageSize().cx, M_ImageSize().cy, M_ImageSize().cx, cMask.m_p)))
			{
				ULONG nX1 = M_ImageSize().cx;
				ULONG nY1 = M_ImageSize().cy;
				ULONG nX2 = 0;
				ULONG nY2 = 0;
				BYTE const* pM = cMask;
				for (ULONG y = 0; y < ULONG(M_ImageSize().cy); ++y)
				{
					for (ULONG x = 0; x < ULONG(M_ImageSize().cx); ++x)
					{
						if (*pM)
						{
							if (nX1 > x) nX1 = x;
							if (nY1 > y) nY1 = y;
							if (nX2 <= x) nX2 = x+1;
							if (nY2 <= y) nY2 = y+1;
						}
						++pM;
					}
				}
				if (nX1 >= nX2)
					return S_FALSE;
				nSizeX = nX2-nX1;
				nSizeY = nY2-nY1;
				hMem = GlobalAlloc(GHND, sizeof(BITMAPV5HEADER)+(nSizeX*nSizeY)*sizeof(TRasterImagePixel)+3*sizeof(RGBQUAD));
				pHeader = reinterpret_cast<BITMAPV5HEADER*>(GlobalLock(hMem));
				TRasterImagePixel* pData = reinterpret_cast<TRasterImagePixel*>(pHeader+1);
				m_pActiveTool->GetImageTile(nX1, nY1, nSizeX, nSizeY, 2.2f, nSizeX, pData);
				pM = cMask.m_p+nY1*M_ImageSize().cx+nX1;
				TRasterImagePixel* pD = pData;
				for (ULONG y = 0; y < nSizeY; ++y)
				{
					for (ULONG x = 0; x < nSizeX; ++x)
					{
						if (*pM == 0)
						{
							pD->bR = pD->bG = pD->bB = pD->bA = 0;
						}
						else if (*pM != 255)
						{
							pD->bA = ULONG(*pM)*pD->bA/255;
						}
						++pM;
						++pD;
					}
					pM += M_ImageSize().cx-nSizeX;
				}
			}
			else
			{
				nSizeX = M_ImageSize().cx;
				nSizeY = M_ImageSize().cy;
				hMem = GlobalAlloc(GHND, sizeof(BITMAPV5HEADER)+(nSizeX*nSizeY)*sizeof(TRasterImagePixel)+3*sizeof(RGBQUAD));
				pHeader = reinterpret_cast<BITMAPV5HEADER*>(GlobalLock(hMem));
				m_pActiveTool->GetImageTile(0, 0, M_ImageSize().cx, M_ImageSize().cy, 2.2f, M_ImageSize().cx, reinterpret_cast<TRasterImagePixel*>(pHeader+1));
			}
			return FinishCopy(hMem, pHeader, nSizeX, nSizeY);
		}
		return E_FAIL; // cannot copy
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Paste()
{
	try
	{
		if (!IsWindow())
			return E_FAIL;

		POINT pt = {0, 0};
		TPixelCoords tPos;
		GetPixelFromPoint(pt, &tPos);
		if (tPos.fX < 0) tPos.fX = 0.0f;
		if (tPos.fY < 0) tPos.fY = 0.0f;

		TImageSize tSize = {0, 0};
		CAutoVectorPtr<TPixelChannel> pPixels;
		if (GetClipboardImage(m_hWnd, tSize, pPixels))
		{
			// switch tool
			return PasteSelection(tPos.fX, tPos.fY, tSize.nX, tSize.nY, pPixels);
		}

		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanDelete()
{
	try
	{
		CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
		if (pSel)
		{
			ULONG nX = 0;
			ULONG nY = 0;
			pSel->Size(&nX, &nY);
			return nX && nY ? S_OK : S_FALSE;
		}

		// enable if selection exists or is in progress
		RECT rcImage = RECT_EMPTY;
		BOOL bAll = FALSE;
		RECT rcSelection = RECT_EMPTY;
		if (S_OK == m_pActiveTool->IsDirty(&rcImage, &bAll, &rcSelection))
		{
			if (rcImage.left < rcImage.right && rcImage.top < rcImage.bottom)
				return S_FALSE; // tool changes image -> no deleting
			rcSelection.left = 0;
			rcSelection.top = 0;
			rcSelection.right = M_ImageSize().cx;
			rcSelection.bottom = M_ImageSize().cy;
			bAll = TRUE;
			m_pActiveTool->GetSelectionInfo(&rcSelection, &bAll);
			if (rcSelection.left == 0 && rcSelection.top == 0 &&
				rcSelection.right == M_ImageSize().cx && rcSelection.bottom == M_ImageSize().cy &&
				bAll)
				return S_FALSE; // everything will be selected -> do nothing
			return S_OK;
			//ApplyChanges();
		}
		return m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
			m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy && m_aSelection.m_p == NULL ? S_FALSE : S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Delete()
{
	try
	{
		CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
		if (pSel)
		{
			if (pSel->Delete()&ETPAApply)
				ApplyChanges();
		}
		else
		{
			RECT rcAll = RECT_EMPTY;
			BOOL bAll = TRUE;
			m_pActiveTool->GetSelectionInfo(&rcAll, &bAll);
			if (rcAll.left < 0) rcAll.left = 0;
			if (rcAll.top < 0) rcAll.top = 0;
			if (rcAll.right > M_ImageSize().cx) rcAll.right = M_ImageSize().cx;
			if (rcAll.bottom > M_ImageSize().cy) rcAll.bottom = M_ImageSize().cy;
			if (rcAll.left < rcAll.right && rcAll.top < rcAll.bottom)
			{
				TPixelChannel tEmpty = {0, 0, 0, 0};
				m_pImage->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tEmpty));
				CImagePoint const t0(rcAll.left, rcAll.top);
				CImageSize const t10(rcAll.right-rcAll.left, rcAll.bottom-rcAll.top);
				if (bAll)
				{
					m_pImage->TileSet(EICIRGBA, t0, t10, CImageStride(0, 0), 1, &tEmpty, FALSE);
				}
				else
				{
					// TODO: support different color formats
					CAutoVectorPtr<TRasterImagePixel> cPixels(new TRasterImagePixel[t10.nX*t10.nY]);
					m_pImage->TileGet(EICIRGBA, t0, t10, NULL, t10.nX*t10.nY, reinterpret_cast<TPixelChannel*>(cPixels.m_p), NULL, EIRIAccurate);
					CAutoVectorPtr<BYTE> cSelection(new BYTE[t10.nX*t10.nY]);
					m_pActiveTool->GetSelectionTile(t0.nX, t0.nY, t10.nX, t10.nY, t10.nX, cSelection);
					TRasterImagePixel* pP = cPixels;
					TRasterImagePixel* pEnd = pP+t10.nX*t10.nY;
					BYTE const* pS = cSelection;
					while (pP != pEnd)
					{
						CPixelMixerReplace::Mix(*pP, *reinterpret_cast<TRasterImagePixel*>(&tEmpty), *pS);
						++pP;
						++pS;
					}
					m_pImage->TileSet(EICIRGBA, t0, t10, NULL, t10.nX*t10.nY, reinterpret_cast<TPixelChannel const*>(cPixels.m_p), FALSE);
				}
			}
			m_rcSelection.left = 0;
			m_rcSelection.top = 0;
			m_rcSelection.right = M_ImageSize().cx;
			m_rcSelection.bottom = M_ImageSize().cy;
			m_aSelection.Free();
			SendSelectionUpdate();
			RectangleChanged(NULL);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanDuplicate()
{
	return S_FALSE;
}

STDMETHODIMP CDesignerViewRasterEdit::Duplicate()
{
	return E_NOTIMPL;
}

STDMETHODIMP CDesignerViewRasterEdit::ObjectName(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_CLIPBOARDDATA_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ObjectIconID(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID)
{
	try
	{
		GUID const tID = {0x41dd8ebb, 0xf2cf, 0x4395, {0x9d, 0xab, 0x4b, 0x4c, 0xaa, 0x29, 0xae, 0x9a}};
		*a_pIconID = tID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ObjectIcon(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	try
	{
		if (a_phIcon) *a_phIcon = NULL;
		if (a_pOverlay) *a_pOverlay = TRUE;
		if (a_phIcon)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			pSI->GetLayers(ESIPicture, cRenderer, IRTarget(1, -1, 1));
			*a_phIcon = cRenderer.get();
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Check(EDesignerViewClipboardAction a_eAction)
{
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:				return CanCut();
		case EDVCACopy:				return CanCopy();
		case EDVCAPaste:			return CanPaste();
		case EDVCASelectAll:		return CanSelectAll();
		case EDVCAInvertSelection:	return CanInvertSelection();
		case EDVCADelete:			return CanDelete();
		case EDVCADuplicate:		return CanDuplicate();
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Exec(EDesignerViewClipboardAction a_eAction)
{
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:				return Cut();
		case EDVCACopy:				return Copy();
		case EDVCAPaste:			return Paste();
		case EDVCASelectAll:		return SelectAll();
		case EDVCAInvertSelection:	return InvertSelection();
		case EDVCADelete:			return Delete();
		case EDVCADuplicate:		return Duplicate();
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanSelectAll()
{
	CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
	if (pSel)
	{
		float fX = 0.0f;
		float fY = 0.0f;
		ULONG nX = 0;
		ULONG nY = 0;
		if (FAILED(pSel->Position(&fX, &fY)) || FAILED(pSel->Size(&nX, &nY)) ||
			fX != 0.0f || fY != 0.0f || nX != M_ImageSize().cx || nY != M_ImageSize().cy)
			return S_OK;
		return m_pActiveTool->IsDirty(NULL, NULL, NULL);
	}
	return S_OK;
	//return m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
	//		m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy &&
	//		m_aSelection.m_p == NULL ? S_FALSE : S_OK;
}
STDMETHODIMP CDesignerViewRasterEdit::SelectAll()
{
	try
	{
		ApplyChanges(FALSE);
		if (m_rcSelection.left != 0 || m_rcSelection.top != 0 ||
			m_rcSelection.right != M_ImageSize().cx || m_rcSelection.bottom != M_ImageSize().cy ||
			m_aSelection.m_p)
		{
			m_rcSelection.left = 0;
			m_rcSelection.top = 0;
			m_rcSelection.right = M_ImageSize().cx;
			m_rcSelection.bottom = M_ImageSize().cy;
			m_aSelection.Free();
			RectangleChanged(NULL);
		}
		CComPtr<IRasterImageEditTool> pNewTool;
		m_pTools->EditToolCreate(m_bstrPasteTool, m_pDoc, &pNewTool);
		CComQIPtr<IRasterImageEditToolFloatingSelection> pSelection(pNewTool);
		if (pSelection == NULL)
			return E_FAIL;

		m_pActiveTool = pNewTool;
		m_bstrToolID = m_bstrPasteTool;
		InitTool();
		pSelection->SelectAll();
		ControlPointsChanged();
		ControlLinesChanged();
		if (m_bstrToolStateSync.Length())
		{
			CComPtr<ISharedStateToolMode> pState;
			RWCoCreateInstance(pState, __uuidof(SharedStateToolMode));
			pState->Set(m_bstrToolID, &m_bFill, m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins, NULL);
			m_bChangingSharedState = true;
			m_pStateMgr->StateSet(m_bstrToolStateSync, pState);
			m_bChangingSharedState = false;
			m_pActiveTool->SetGlobals(m_eBlendingMode, m_eRasterizationMode, m_eCoordinatesMode);
			m_pActiveTool->SetOutline(m_bOutline, m_fOutlineWidth, m_fOutlinePos, m_eOutlineJoins, &m_tOutlineColor);
		}
		SendSelectionUpdate();
		ActivateWindow();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

template<BYTE t_nVal>
static bool IsRowConstant(BYTE const* p, ULONG a_nLen)
{
	BYTE const* const pEnd = p+a_nLen;
	while (p < pEnd)
	{
		if (t_nVal != *p)
			return false;
		++p;
	}
	return true;
}

template<BYTE t_nVal>
static bool IsColumnConstant(BYTE const* p, ULONG a_nLen, ULONG a_nStride)
{
	BYTE const* const pEnd = p+a_nLen*a_nStride;
	while (p < pEnd)
	{
		if (t_nVal != *p)
			return false;
		p += a_nStride;
	}
	return true;
}

bool CDesignerViewRasterEdit::OptimizeSelection(CAutoVectorPtr<BYTE>& a_pData, RECT* a_pBounds)
{
	if (a_pData.m_p == NULL)
		return false;
	if (a_pBounds->top >= a_pBounds->bottom)
	{
		a_pData.Free();
		*a_pBounds = RECT_EMPTY;
		return true;
	}

	BYTE* p = a_pData;
	RECT rcFinal = *a_pBounds;
	SIZE const szOrig = {a_pBounds->right-a_pBounds->left, a_pBounds->bottom-a_pBounds->top};
	while (rcFinal.top < rcFinal.bottom && IsRowConstant<0>(p+(rcFinal.top-a_pBounds->top)*szOrig.cx, szOrig.cx))
		++rcFinal.top;
	while (rcFinal.top < rcFinal.bottom && IsRowConstant<0>(p+(rcFinal.bottom-1-a_pBounds->top)*szOrig.cx, szOrig.cx))
		--rcFinal.bottom;

	if (rcFinal.top >= rcFinal.bottom)
	{
		// nothing selected
		a_pData.Free();
		*a_pBounds = RECT_EMPTY;
		return true;
	}

	p += (rcFinal.top-a_pBounds->top)*szOrig.cx;
	ULONG nNewSizeY = rcFinal.bottom-rcFinal.top;
	while (rcFinal.left < rcFinal.right && IsColumnConstant<0>(p+rcFinal.left-a_pBounds->left, nNewSizeY, szOrig.cx))
		++rcFinal.left;
	while (rcFinal.left < rcFinal.right && IsColumnConstant<0>(p+rcFinal.right-1-a_pBounds->left, nNewSizeY, szOrig.cx))
		--rcFinal.right;
	bool bChanged = rcFinal.left != a_pBounds->left || rcFinal.right != a_pBounds->right ||
		rcFinal.top != a_pBounds->top || rcFinal.bottom != a_pBounds->bottom;
	ULONG nNewSizeX = rcFinal.right-rcFinal.left;
	if (bChanged)
	{
		p = a_pData;
		for (LONG y = rcFinal.top; y < rcFinal.bottom; ++y, p+=nNewSizeX)
			MoveMemory(p, a_pData+(y-a_pBounds->top)*szOrig.cx+rcFinal.left-a_pBounds->left, nNewSizeX);
		*a_pBounds = rcFinal;
	}
	if (IsRowConstant<0xff>(a_pData, nNewSizeX*nNewSizeY))
	{
		a_pData.Free();
		return true;
	}
	return bChanged;
}

STDMETHODIMP CDesignerViewRasterEdit::CanInvertSelection()
{
	if (!m_bSelectionSupport)
		return S_FALSE;
	CComQIPtr<IRasterImageEditToolFloatingSelection> pSel(m_pActiveTool);
	return pSel ? S_FALSE : S_OK;
}

STDMETHODIMP CDesignerViewRasterEdit::InvertSelection()
{
	try
	{
		if (!m_bSelectionSupport)
			return E_FAIL;;

		ApplyChanges(FALSE);
		if (m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
			m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy &&
			m_aSelection.m_p == NULL)
		{
			// -> select nothing
			m_rcSelection = RECT_EMPTY;
			RectangleChanged(NULL);
			ActivateWindow();
			return S_OK;
		}
		if (m_rcSelection.left >= m_rcSelection.right || m_rcSelection.top >= m_rcSelection.bottom)
		{
			m_rcSelection.left = 0;
			m_rcSelection.top = 0;
			m_rcSelection.right = M_ImageSize().cx;
			m_rcSelection.bottom = M_ImageSize().cy;
			RectangleChanged(NULL);
			ActivateWindow();
			return S_OK;
		}
		CAutoVectorPtr<BYTE> cSel(new BYTE[M_ImageSize().cx*M_ImageSize().cy]);
		GetSelectionTile(0, 0, M_ImageSize().cx, M_ImageSize().cy, M_ImageSize().cx, cSel);
		BYTE* pEnd = cSel.m_p+M_ImageSize().cx*M_ImageSize().cy;
		for (BYTE* p = cSel.m_p; p != pEnd; ++p)
			*p = 255-*p;
		std::swap(cSel.m_p, m_aSelection.m_p);
		m_rcSelection.left = 0;
		m_rcSelection.top = 0;
		m_rcSelection.right = M_ImageSize().cx;
		m_rcSelection.bottom = M_ImageSize().cy;
		OptimizeSelection(m_aSelection, &m_rcSelection);
		RectangleChanged(NULL);
		SendSelectionUpdate();
		ActivateWindow();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	try
	{
		if (!IsWindow() || a_pFileNames == NULL)
			return E_FAIL;
		RECT rc;
		GetWindowRect(&rc);
		if (a_pt.x < rc.left || a_pt.x >= rc.right || a_pt.y < rc.top || a_pt.y >= rc.bottom)
			return E_FAIL;

		ULONG nItems = 0;
		a_pFileNames->Size(&nItems);
		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		CComPtr<IEnumUnknowns> pImageFormats;
		pInMgr->DocumentTypesEnumEx(pBuilders, &pImageFormats);
		ULONG nFormats = 0;
		pImageFormats->Size(&nFormats);
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComBSTR bstr;
			a_pFileNames->Get(i, &bstr);
			for (ULONG j = 0; j < nFormats; ++j)
			{
				CComPtr<IDocumentType> pType;
				pImageFormats->Get(j, __uuidof(IDocumentType), reinterpret_cast<void**>(&pType));
				if (pType->MatchFilename(bstr) == S_OK)
				{
					if (a_pdwEffect) *a_pdwEffect = DROPEFFECT_COPY;
					if (a_ppFeedback) *a_ppFeedback = _SharedStringTable.GetString(IDS_DROPMSG_PASTEIMAGE);
					return S_OK;
				}
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
{
	try
	{
		if (a_pDataObj == NULL && a_pFileNames == NULL)
			return E_RW_CANCELLEDBYUSER; // cancelled d'n'd operation

		if (!IsWindow() || a_pFileNames == NULL)
			return E_FAIL;
		RECT rc;
		GetWindowRect(&rc);
		if (a_pt.x < rc.left || a_pt.x >= rc.right || a_pt.y < rc.top || a_pt.y >= rc.bottom)
			return E_FAIL;

		ULONG nItems = 0;
		a_pFileNames->Size(&nItems);
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		HRESULT hRes = S_FALSE;
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComBSTR bstr;
			a_pFileNames->Get(i, &bstr);
			CStorageFilter cInPath(bstr.m_str);
			CComPtr<IDocument> pImgDoc;
			RWCoCreateInstance(pImgDoc, __uuidof(DocumentBase));
			if (FAILED(pIM->DocumentCreateData(pBuilders, cInPath, NULL, CComQIPtr<IDocumentBase>(pImgDoc))))
				continue;
			CComPtr<IDocumentImage> pImage;
			pImgDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
			CBGRABuffer cBuffer;
			if (!cBuffer.Init(pImage))
				continue;

			hRes = PasteSelection(0.0f, 0.0f, cBuffer.tSize.nX, cBuffer.tSize.nY, reinterpret_cast<TPixelChannel const*>(cBuffer.aData.m_p));
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDesignerViewRasterEdit::PasteSelection(float a_fX, float a_fY, ULONG a_nSizeX, ULONG a_nSizeY, TPixelChannel const* a_pData)
{
	ApplyChanges(FALSE);
	CComPtr<IRasterImageEditTool> pNewTool;
	m_pTools->EditToolCreate(m_bstrPasteTool, m_pDoc, &pNewTool);
	CComQIPtr<IRasterImageEditToolFloatingSelection> pSelection(pNewTool);
	if (pSelection == NULL)
		return E_FAIL;

	m_pActiveTool = pNewTool;
	m_bstrToolID = m_bstrPasteTool;
	InitTool();
	pSelection->Set(a_fX, a_fY, a_nSizeX, a_nSizeY, a_nSizeX, reinterpret_cast<TRasterImagePixel const*>(a_pData), FALSE);
	ControlPointsChanged();
	ControlLinesChanged();
	if (m_bstrToolStateSync.Length())
	{
		CComPtr<ISharedStateToolMode> pState;
		RWCoCreateInstance(pState, __uuidof(SharedStateToolMode));
		pState->Set(m_bstrToolID, &m_bFill, m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins, NULL);
		m_bChangingSharedState = true;
		m_pStateMgr->StateSet(m_bstrToolStateSync, pState);
		m_bChangingSharedState = false;
		m_pActiveTool->SetGlobals(m_eBlendingMode, m_eRasterizationMode, m_eCoordinatesMode);
		m_pActiveTool->SetOutline(m_bOutline, m_fOutlineWidth, m_fOutlinePos, m_eOutlineJoins, &m_tOutlineColor);
	}
	ActivateWindow();
	return S_OK;
}

STDMETHODIMP CDesignerViewRasterEdit::SelectionExists()
{
	CComQIPtr<IRasterImageEditToolFloatingSelection> pSelTool(m_pActiveTool);
	if (pSelTool == NULL)
		return S_FALSE;
	ULONG nX = 0, nY = 0;
	pSelTool->Size(&nX, &nY);
	return nX && nY ? S_OK : S_FALSE;
}

STDMETHODIMP CDesignerViewRasterEdit::GetSelectionTool(IRasterImageEditToolFloatingSelection** a_ppTool)
{
	return m_pActiveTool->QueryInterface(a_ppTool);
}


STDMETHODIMP CDesignerViewRasterEdit::CanUndo()
{
	try
	{
		if (m_pActiveTool->IsDirty(NULL, NULL, NULL) == S_OK)
			return S_OK; // tool handles undo
		// if selection exist, remove selection on undo
		return m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
			   m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy &&
			   m_aSelection.m_p == NULL ? S_FALSE : S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Undo()
{
	try
	{
		if (m_pActiveTool->IsDirty(NULL, NULL, NULL) == S_OK)
			return m_pActiveTool->Reset();
		if (m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
			m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy &&
			m_aSelection.m_p == NULL)
			return E_FAIL;
		m_rcSelection.left = 0;
		m_rcSelection.top = 0;
		m_rcSelection.right = M_ImageSize().cx;
		m_rcSelection.bottom = M_ImageSize().cy;
		m_aSelection.Free();
		SendSelectionUpdate();
		RectangleChanged(NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::UndoName(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (m_pActiveTool->IsDirty(NULL, NULL, NULL) == S_OK)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Cancel drawn shape[0405]Zrušit nakreslený tvar");
			return S_OK;
		}
		if (m_rcSelection.left == 0 && m_rcSelection.top == 0 &&
			m_rcSelection.right == M_ImageSize().cx && m_rcSelection.bottom == M_ImageSize().cy &&
			m_aSelection.m_p == NULL)
			return E_FAIL;
		*a_ppName = new CMultiLanguageString(L"[0409]Cancel image selection[0405]Zrušit výběr obrázku");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Update(IDesignerStatusBar* a_pStatusBar)
{
	try
	{
		if (m_bShowCommandDesc)
		{
			a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
			return S_OK;
		}
		if (m_tSentPos.fX != m_tLastPos.fX || m_tSentPos.fY != m_tLastPos.fY)
		{
			m_tSentPos = m_tLastPos;
			OLECHAR sz[128] = L"";
			swprintf(sz, L"%g, %g", floorf(m_tSentPos.fX*10.0f+0.5f)*0.1f, floorf(m_tSentPos.fY*10.0f+0.5f)*0.1f);

			static HICON hIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_STATUS_COORDS), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR|LR_SHARED);
			a_pStatusBar->PaneSet(m_bstrCoordsPane, hIcon, CComBSTR(sz), 110, -100);
		}
		else
		{
			a_pStatusBar->PaneKeep(m_bstrCoordsPane);
		}
		CComQIPtr<IDesignerViewStatusBar> pToolSB(m_pActiveTool);
		if (pToolSB)
			pToolSB->Update(a_pStatusBar);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::CanApplyChanges()
{
	return m_pActiveTool->IsDirty(NULL, NULL, NULL);
}

STDMETHODIMP CDesignerViewRasterEdit::ConfigureGestures(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		return m_pMGH ? m_pMGH->Configure(a_hParent ? a_hParent : m_hWnd, a_tLocaleID ? a_tLocaleID : m_tLocaleID, m_pConfig) : E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CDesignerViewRasterEdit::OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	if (m_eDragState != EDSTool && m_eDragState != EDSHandle)
	{
		m_bAutoScroll = false;
		return 0;
	}

	int nSteps = (GetTickCount()-m_dwScrollStart)/SCROLL_TICK;
	if (nSteps < 1)
	{
		SetTimer(SCROLL_TIMERID, SCROLL_TICK);
		return 0;
	}

	ULONG nDX = 0;
	if (m_tScrollMousePos.x < -2)
	{
		nDX = m_tScrollMousePos.x < -15 ? -30 : -10;
	}
	else if (m_tScrollMousePos.x >= M_WindowSize().cx+2)
	{
		nDX = m_tScrollMousePos.x >= M_WindowSize().cx+15 ? 30 : 10;
	}

	ULONG nDY = 0;
	if (m_tScrollMousePos.y < -2)
	{
		nDY = m_tScrollMousePos.y < -20 ? -40 : -15;
	}
	else if (m_tScrollMousePos.y >= M_WindowSize().cy+2)
	{
		nDY = m_tScrollMousePos.y >= M_WindowSize().cy+20 ? 40 : 15;
	}

	nDX *= nSteps;
	nDY *= nSteps;
	m_dwScrollStart += nSteps*SCROLL_TICK;

	if (nDX || nDY)
	{
		bool bMoved = false;
		float fOffsetX = M_OffsetX();
		float fOffsetY = M_OffsetY();
		if (nDX && M_ZoomedSize().cx != M_WindowSize().cx)
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;
			if (GetScrollInfo(SB_HORZ, &si))
			{
				int nNew = si.nPos + nDX;
				if (nNew < si.nMin) nNew = si.nMin;
				else if (nNew > int(si.nMax-si.nPage)) nNew = si.nMax-si.nPage;
				if (nNew != si.nPos)
				{
					si.nPos = nNew;
					si.fMask = SIF_POS;
					float const fOffsetBound = 0.5f*static_cast<float>(M_ZoomedSize().cx-M_WindowSize().cx+CanvasPadding()*2)/static_cast<float>(M_ZoomedSize().cx);
					fOffsetX = static_cast<float>(nNew)*(fOffsetBound*2)/(M_ZoomedSize().cx-1+(CanvasPadding()*2)-M_WindowSize().cx)-fOffsetBound;
					SetScrollInfo(SB_HORZ, &si);
					bMoved = true;
				}
			}
		}
		if (nDY && M_ZoomedSize().cy != M_WindowSize().cy)
		{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;
			if (GetScrollInfo(SB_VERT, &si))
			{
				int nNew = si.nPos + nDY;
				if (nNew < si.nMin) nNew = si.nMin;
				else if (nNew > int(si.nMax-si.nPage)) nNew = si.nMax-si.nPage;
				if (nNew != si.nPos)
				{
					si.nPos = nNew;
					si.fMask = SIF_POS;
					float const fOffsetBound = 0.5f*static_cast<float>(M_ZoomedSize().cy-M_WindowSize().cy+CanvasPadding()*2)/static_cast<float>(M_ZoomedSize().cy);
					fOffsetY = static_cast<float>(nNew)*(fOffsetBound*2)/(M_ZoomedSize().cy-1+(CanvasPadding()*2)-M_WindowSize().cy)-fOffsetBound;
					SetScrollInfo(SB_VERT, &si);
					bMoved = true;
				}
			}
		}
		if (bMoved)
		{
			TPixelCoords tPos;
			if (m_eDragState == EDSHandle)
			{
				GetPixelFromPoint(m_tScrollMousePos, &tPos, NULL, &m_nHandleIndex);
				m_pActiveTool->SetControlPoint(m_nHandleIndex, &tPos, false, (M_HandleRadius()+0.5f)/M_ImageZoom());
			}
			else
			{
				TPixelCoords tPointerSize;
				EControlKeysState eKeysState = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
				GetPixelFromPoint(m_tScrollMousePos, &tPos, &tPointerSize, NULL, eKeysState);
				DWORD dwDummy = 0;
				m_pActiveTool->ProcessInputEvent(eKeysState, &tPos, &tPointerSize, m_fLastPressure = 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			}
			MoveViewport(fOffsetX, fOffsetY, M_ImageZoom());
			//Invalidate(FALSE);
			//SendViewportUpdate();
		}
	}

	SetTimer(SCROLL_TIMERID, SCROLL_TICK);

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
	POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	GetPixelFromPoint(pt, &m_tLastPos, NULL, m_eDragState == EDSHandle ? &m_nHandleIndex : NULL, eKeysState);

	switch (m_eDragState)
	{
	case EDSNothing:
		if ((GetTickCount()-m_dwLastMessage) > TABLETBLOCKTIME)
		{
			ULONG const nPrevHot = m_nHotHandle;
			m_nHotHandle = 0xffffffff;

			DWORD dwDummy = 0;
			TPixelCoords tPointerSize = {float(M_ImageSize().cx)/float(M_ZoomedSize().cx), float(M_ImageSize().cy)/float(M_ZoomedSize().cy)};
			m_pActiveTool->ProcessInputEvent(eKeysState, &m_tLastPos, &tPointerSize, m_fLastPressure = 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			if (!m_bTrackMouse)
			{
				m_bTrackMouse = true;
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = m_hWnd;
				tME.dwFlags = TME_LEAVE;
				TrackMouseEvent(&tME);
			}
			if (M_HideHandles() && m_bMouseOut)
			{
				m_bMouseOut = false;
				Invalidate(FALSE);
			}
			TestHandle(pt, &m_nHotHandle);
			if (nPrevHot != m_nHotHandle)
			{
				SIZE szZoomed = M_ZoomedSize();
				SIZE szOrig = M_ImageSize();
				float fZoomX = float(szZoomed.cx)/szOrig.cx;
				float fZoomY = float(szZoomed.cy)/szOrig.cy;

				if (nPrevHot < m_cHandles.size())
				{
					TPixelCoords tPos = m_cHandles[nPrevHot].first;
					LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
					LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
					RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
					InvalidateRect(&rcPt);
				}
				if (m_nHotHandle < m_cHandles.size())
				{
					TPixelCoords tPos = m_cHandles[m_nHotHandle].first;
					LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
					LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
					RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
					InvalidateRect(&rcPt);
				}
			}
		}
		break;
	case EDSPanning:
		MoveViewport(
			m_fOrigOffsetX-float(pt.x-m_tDragStartClient.x)/M_ZoomedSize().cx,
			m_fOrigOffsetY-float(pt.y-m_tDragStartClient.y)/M_ZoomedSize().cy,
			M_ImageZoom());
		UpdateWindow();
		break;
	case EDSZooming:
		{
			float fZoom = m_fOrigZoom*powf(1.01f, m_tDragStartClient.y-pt.y);
			RECT rcClient;
			GetClientRect(&rcClient);
			LONG fDX = m_tDragStartClient.x-(rcClient.right>>1);
			LONG fDY = m_tDragStartClient.y-(rcClient.bottom>>1);
			if (fZoom > 32.0f)
			{
				fZoom = 32.0f;
			}
			else
			{
				LONG nSize = min(M_ImageSize().cx, M_ImageSize().cy);
				float fMinZoom = 1.0f/nSize;
				if (fZoom <= fMinZoom)
					fZoom = fMinZoom;
			}
			MoveViewport(
				m_fOrigOffsetX + (fDX/m_fOrigZoom-fDX/fZoom)/M_ImageSize().cx,
				m_fOrigOffsetY + (fDY/m_fOrigZoom-fDY/fZoom)/M_ImageSize().cy,
				fZoom);
			UpdateWindow();
		}
		break;
	case EDSGesture:
		{
			POINT const tLast = *m_cGesturePoints.rbegin();
			m_cGesturePoints.push_back(pt);
			if (m_nGestureXMin > pt.x) m_nGestureXMin = pt.x;
			if (m_nGestureXMax < pt.x) m_nGestureXMax = pt.x;
			if (m_nGestureYMin > pt.y) m_nGestureYMin = pt.y;
			if (m_nGestureYMax < pt.y) m_nGestureYMax = pt.y;
			RECT rc = {min(tLast.x, pt.x)-2, min(tLast.y, pt.y)-2, max(tLast.x, pt.x)+3, max(tLast.y, pt.y)+3};
			InvalidateRect(&rc, FALSE);
		}
		break;
	case EDSTool:
	case EDSHandle:
		// scroll window if dragged outside of window
		if (pt.x < -2 || pt.y < -2 || pt.x >= M_WindowSize().cx+2 || pt.y >= M_WindowSize().cy+2)
		{
			m_tScrollMousePos = pt;
			if (!m_bAutoScroll)
			{
				m_bAutoScroll = true;
				SetTimer(SCROLL_TIMERID, SCROLL_TICK);
				m_dwScrollStart = GetTickCount();
			}
			else if ((GetTickCount()-m_dwScrollStart) > SCROLL_TICK)
			{
				BOOL b;
				OnTimer(0, 0, 0, b);
			}
		}
		else
		{
			if (m_bAutoScroll)
			{
				m_bAutoScroll = false;
				KillTimer(SCROLL_TIMERID);
			}
		}
		if (m_eDragState == EDSTool)
		{
			DWORD dwDummy = 0;
			TPixelCoords tPointerSize = {float(M_ImageSize().cx)/float(M_ZoomedSize().cx), float(M_ImageSize().cy)/float(M_ZoomedSize().cy)};
			m_pActiveTool->ProcessInputEvent(eKeysState, &m_tLastPos, &tPointerSize, m_fLastPressure = 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
		}
		else
		{
			m_pActiveTool->SetControlPoint(m_nHandleIndex, &m_tLastPos, false, (M_HandleRadius()+0.5f)/M_ImageZoom());
		}
		UpdateWindow();
		break;
	}
	if (m_tLastPos.fX != m_tSentPos.fX || m_tLastPos.fY != m_tSentPos.fY)
		m_pStatusBar->Notify(0, 0);

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSNothing:
		{
			if (m_bTrackMouse)
			{
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = m_hWnd;
				tME.dwFlags = TME_LEAVE|TME_CANCEL;
				TrackMouseEvent(&tME);
				m_bTrackMouse = false;
			}
			if ((GetTickCount()-m_dwLastMessage) <= TABLETBLOCKTIME)
				break; // block mouse messages when tablet is active
			SetCapture();
			POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
			if (TestHandle(pt, &m_nHandleIndex))
			{
				m_eDragState = EDSHandle;
			}
			else
			{
				m_eDragState = EDSTool;
				EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
				TPixelCoords tPos;
				TPixelCoords tPointerSize;
				GetPixelFromPoint(pt, &tPos, &tPointerSize, NULL, eKeysState);
				DWORD dwDummy = 0;
				if ((m_pActiveTool->PointTest(eKeysState, &tPos, TRUE, sqrtf(tPointerSize.fX*tPointerSize.fY))&ETPAActionMask) == ETPAStartNew)
					ApplyChanges();
				m_pActiveTool->ProcessInputEvent(eKeysState, &tPos, &tPointerSize, m_fLastPressure = 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			}
		}
		break;
	case EDSPanning:
	case EDSZooming:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSTool:
		{
			ReleaseCapture();
			m_eDragState = EDSNothing;
			POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
			TPixelCoords tPos;
			TPixelCoords tPointerSize;
			EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
			GetPixelFromPoint(pt, &tPos, &tPointerSize, NULL, eKeysState);
			DWORD dwDummy = 0;
			HRESULT hRes = m_pActiveTool->ProcessInputEvent(eKeysState, &tPos, &tPointerSize, m_fLastPressure = 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			if (hRes&ETPAApply)
				ApplyChanges();
			if ((hRes&ETPAActionMask) == ETPAStartNew)
				m_pActiveTool->ProcessInputEvent(eKeysState, &tPos, &tPointerSize, m_fLastPressure = 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
//SendDrawToolCmdLineUpdate();
		}
		break;
	case EDSHandle:
		{
			ReleaseCapture();
			m_eDragState = EDSNothing;
			POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
			TPixelCoords tPos;
			GetPixelFromPoint(pt, &tPos, NULL, &m_nHandleIndex);
			m_pActiveTool->SetControlPoint(m_nHandleIndex, &tPos, true, (M_HandleRadius()+0.5f)/M_ImageZoom());
//SendDrawToolCmdLineUpdate();
		}
		break;
	case EDSNothing:
	case EDSPanning:
	case EDSZooming:
	case EDSGesture:
		break;
	}

	if (m_bAutoScroll)
	{
		m_bAutoScroll = false;
		KillTimer(SCROLL_TIMERID);
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSNothing:
		{
			if (m_bTrackMouse)
			{
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = m_hWnd;
				tME.dwFlags = TME_LEAVE|TME_CANCEL;
				TrackMouseEvent(&tME);
				m_bTrackMouse = false;
			}
			SetCapture();
			m_eDragState = EDSGesture;
			POINT const pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
			m_cGesturePoints.push_back(pt);
			m_nGestureXMin = m_nGestureXMax = pt.x;
			m_nGestureYMin = m_nGestureYMax = pt.y;
		}
		break;
	case EDSPanning:
	case EDSZooming:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

#include "HandleCoordinates.h"
//#include <DocumentMenuCommandImpl.h>
//#include <InPlaceCalc.h>
//
//class ATL_NO_VTABLE CManualHandleCoordinates :
//	public CDocumentMenuCommandImpl<CManualHandleCoordinates, IDS_MENU_HANDLECOORDINATES_NAME, IDS_MENU_HANDLECOORDINATES_DESC, NULL, 0>
//{
//public:
//	void Init(IRasterImageEditTool* a_pTool, ULONG a_nHandle)
//	{
//		m_pTool = a_pTool;
//		m_nHandle = a_nHandle;
//	}
//
//	// IDocumentMenuCommand
//public:
//	class CDlg :
//		public Win32LangEx::CLangDialogImpl<CDlg>
//	{
//	public:
//		CDlg(LCID a_tLocaleID, TPixelCoords* a_pCoords) :
//			Win32LangEx::CLangDialogImpl<CDlg>(a_tLocaleID), m_pCoords(a_pCoords)
//		{
//		}
//
//		enum {IDD = IDD_MODIFYHANDLE};
//
//		BEGIN_MSG_MAP(CDlg)
//			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//			COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
//			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
//		END_MSG_MAP()
//
//		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
//		{
//			TCHAR szTmp[64];
//			_stprintf(szTmp, _T("%g"), m_pCoords->fX);
//			SetDlgItemText(IDC_MH_X, szTmp);
//			_stprintf(szTmp, _T("%g"), m_pCoords->fY);
//			SetDlgItemText(IDC_MH_Y, szTmp);
//			// center the dialog on the screen
//			CenterWindow();
//			return TRUE;
//		}
//		LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
//		{
//			TCHAR szTmp[256];
//			GetDlgItemText(IDC_MH_X, szTmp, itemsof(szTmp));
//			szTmp[itemsof(szTmp)-1] = _T('\0');
//			LPCTSTR pEnd = szTmp;
//			m_pCoords->fX = CInPlaceCalc::EvalExpression(szTmp, &pEnd);
//			if (*pEnd != _T('\0'))
//			{
//				GetDlgItem(IDC_MH_X).SetFocus();
//				MessageBeep(MB_ICONASTERISK);
//				return 0;
//			}
//			GetDlgItemText(IDC_MH_Y, szTmp, itemsof(szTmp));
//			szTmp[itemsof(szTmp)-1] = _T('\0');
//			pEnd = szTmp;
//			m_pCoords->fY = CInPlaceCalc::EvalExpression(szTmp, &pEnd);
//			if (*pEnd != _T('\0'))
//			{
//				GetDlgItem(IDC_MH_Y).SetFocus();
//				MessageBeep(MB_ICONASTERISK);
//				return 0;
//			}
//			EndDialog(IDOK);
//			return 0;
//		}
//		LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
//		{
//			EndDialog(IDCANCEL);
//			return 0;
//		}
//
//	private:
//		TPixelCoords* const m_pCoords;
//	};
//
//	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
//	{
//		try
//		{
//			TPixelCoords tPixel;
//			ULONG dummy;
//			if (FAILED(m_pTool->GetControlPoint(m_nHandle, &tPixel, &dummy)))
//				return E_FAIL;
//			CDlg cDlg(a_tLocaleID, &tPixel);
//			if (cDlg.DoModal(a_hParent) != IDOK)
//				return S_FALSE;
//			return m_pTool->SetControlPoint(m_nHandle, &tPixel, true, 0.0f);
//		}
//		catch (...)
//		{
//			return E_UNEXPECTED;
//		}
//	}
//
//private:
//	CComPtr<IRasterImageEditTool> m_pTool;
//	ULONG m_nHandle;
//};

LRESULT CDesignerViewRasterEdit::OnRButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bAutoScroll)
	{
		m_bAutoScroll = false;
		KillTimer(SCROLL_TIMERID);
	}

	switch (m_eDragState)
	{
	case EDSGesture:
		{
			ReleaseCapture();
			m_eDragState = EDSNothing;
			if (m_cGesturePoints.empty())
				break;
			POINT ptCenter = m_cGesturePoints[0];
			int nDragX = GetSystemMetrics(SM_CXDRAG);
			int nDragY = GetSystemMetrics(SM_CYDRAG);
			if (m_nGestureXMin < (ptCenter.x-nDragX) || m_nGestureXMax > (ptCenter.x+nDragX) ||
				m_nGestureYMin < (ptCenter.y-nDragY) || m_nGestureYMax > (ptCenter.y+nDragY))
			{
				CConfigValue cOpID;
				CComPtr<IConfig> pOpCfg;
				HRESULT hRes = m_pMGH ? m_pMGH->Recognize(m_cGesturePoints.size(), &(m_cGesturePoints[0]), m_pConfig, &cOpID, &pOpCfg) : E_FAIL;
				m_cGesturePoints.clear();
				RECT rc = {m_nGestureXMin-2, m_nGestureYMin-2, m_nGestureXMax+3, m_nGestureYMax+3};
				InvalidateRect(&rc, FALSE);
				if (SUCCEEDED(hRes))
					ExecuteGesture(cOpID, pOpCfg);
			}
			else
			{
				RECT rc = {m_nGestureXMin-2, m_nGestureYMin-2, m_nGestureXMax+3, m_nGestureYMax+3};
				InvalidateRect(&rc, FALSE);

				CComPtr<IEnumUnknownsInit> pOps;
				RWCoCreateInstance(pOps, __uuidof(EnumUnknowns));

				if (!m_cGesturePoints.empty())
				{
					TPixelCoords tCoords = {0, 0};
					ULONG nHandle = 0xffffffff;
					TestHandle(m_cGesturePoints[0], &nHandle);
					GetPixelFromPoint(m_cGesturePoints[0], &tCoords, NULL, &nHandle);
					if (nHandle != 0xffffffff)
					{
						// TODO: manual handle coordinates
						CComObject<CManualHandleCoordinates<TEditToolCoordsHandler> >* p = NULL;
						CComObject<CManualHandleCoordinates<TEditToolCoordsHandler> >::CreateInstance(&p);
						CComPtr<IDocumentMenuCommand> pTmp = p;
						p->Init(TEditToolCoordsHandler(m_pActiveTool, nHandle));
						pOps->Insert(pTmp);
						CComPtr<IDocumentMenuCommand> pSep;
						RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
						pOps->Insert(pSep);
					}

					CComQIPtr<IRasterImageEditToolContextMenu> pToolMenu(m_pActiveTool);
					if (pToolMenu)
					{
						CComPtr<IEnumUnknowns> pToolOps;
						pToolMenu->CommandsEnum(&tCoords, nHandle, &pToolOps);
						if (pToolOps)
						{
							pOps->InsertFromEnum(pToolOps);
							CComPtr<IDocumentMenuCommand> pSep;
							RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
							pOps->Insert(pSep);
						}
					}
				}
				m_cGesturePoints.clear();

				CConfigValue cOpID;
				CComPtr<IConfig> pOpCfg;
				CComBSTR bstrCFGID_2DEDIT_CONTEXTMENU(CFGID_2DEDIT_CONTEXTMENU);
				m_pConfig->ItemValueGet(bstrCFGID_2DEDIT_CONTEXTMENU, &cOpID);
				m_pConfig->SubConfigGet(bstrCFGID_2DEDIT_CONTEXTMENU, &pOpCfg);
				CComPtr<IEnumUnknowns> pCtxOps;
				m_pCmdMgr->CommandsEnum(m_pCmdMgr, cOpID, pOpCfg, m_pContext, this, m_pDoc, &pCtxOps);
				if (pCtxOps)
				{
					pOps->InsertFromEnum(pCtxOps);
				}

				ULONG nSize = 0;
				if (pOps == NULL || FAILED(pOps->Size(&nSize)) || nSize == 0)
					return 0;

				ClientToScreen(&ptCenter);
				ProcessContextMenu(pOps, ptCenter.x, ptCenter.y);
			}
		}
		break;
	case EDSNothing:
	case EDSPanning:
	case EDSZooming:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnMButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSNothing:
		{
			if (m_bTrackMouse)
			{
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = m_hWnd;
				tME.dwFlags = TME_LEAVE|TME_CANCEL;
				TrackMouseEvent(&tME);
				m_bTrackMouse = false;
			}
			SetCapture();
			m_eDragState = a_wParam&MK_CONTROL ? EDSZooming : EDSPanning;
			m_tDragStartClient.x = GET_X_LPARAM(a_lParam);
			m_tDragStartClient.y = GET_Y_LPARAM(a_lParam);
			GetPixelFromPoint(m_tDragStartClient, &m_tDragStartImage);
			m_fOrigOffsetX = M_OffsetX();
			m_fOrigOffsetY = M_OffsetY();
			m_fOrigZoom = M_ImageZoom();
		}
		break;
	case EDSPanning:
	case EDSZooming:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnMButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSPanning:
	case EDSZooming:
		{
			ReleaseCapture();
			m_eDragState = EDSNothing;
		}
		break;
	case EDSNothing:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnXButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSNothing:
		{
			SetCapture();
			m_eDragState = HIWORD(a_wParam) == XBUTTON1 ? EDSZooming : EDSPanning;
			m_tDragStartClient.x = GET_X_LPARAM(a_lParam);
			m_tDragStartClient.y = GET_Y_LPARAM(a_lParam);
			GetPixelFromPoint(m_tDragStartClient, &m_tDragStartImage);
			m_fOrigOffsetX = M_OffsetX();
			m_fOrigOffsetY = M_OffsetY();
			m_fOrigZoom = M_ImageZoom();
		}
		break;
	case EDSPanning:
	case EDSZooming:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnXButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	switch (m_eDragState)
	{
	case EDSPanning:
	case EDSZooming:
		{
			ReleaseCapture();
			m_eDragState = EDSNothing;
		}
		break;
	case EDSNothing:
	case EDSGesture:
	case EDSTool:
	case EDSHandle:
		break;
	}

	return 0;
}

void CDesignerViewRasterEdit::GetPixelFromPoint(POINT a_tPoint, TPixelCoords* a_pPixel, TPixelCoords* a_pPointerSize, ULONG const* a_pControlPointIndex, EControlKeysState a_eKeysState) const
{
	float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
	float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
	a_pPixel->fX = (a_tPoint.x - M_ImagePos().x/*+ fX*//* - tZoomedSize.cx*0.5f*/) / fZoomX;
	a_pPixel->fY = (a_tPoint.y - M_ImagePos().y/*+ fY*//* - tZoomedSize.cy*0.5f*/) / fZoomY;
	TPixelCoords tPointerSize = {1.0f/fZoomX, 1.0f/fZoomY};
	if (a_pPointerSize) *a_pPointerSize = tPointerSize;
	m_pActiveTool->AdjustCoordinates(a_eKeysState, a_pPixel, &tPointerSize, a_pControlPointIndex, (M_HandleRadius()+0.5f)/M_ImageZoom());
}

LRESULT CDesignerViewRasterEdit::OnChangeFocus(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
{
	a_bHandled = FALSE;
	//Invalidate(FALSE); // TODO: invalidate affected region
	return 0;
}

bool CDesignerViewRasterEdit::TestHandle(POINT const& a_tPos, ULONG* a_pIndex) const
{
	float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
	float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
	float const fRadSq = (M_HandleRadius()+0.5f)*(M_HandleRadius()+0.5f);
	for (CHandles::const_reverse_iterator i = m_cHandles.rbegin(); i != m_cHandles.rend(); ++i)
	{
		LONG const nX = fZoomX*i->first.fX+M_ImagePos().x;
		LONG const nY = fZoomY*i->first.fY+M_ImagePos().y;
		float const fDistSq = (a_tPos.x-nX)*(a_tPos.x-nX) + (a_tPos.y-nY)*(a_tPos.y-nY);
		if (fDistSq <= fRadSq)
		{
			*a_pIndex = m_cHandles.rend()-i-1;
			return true;
		}
	}
	return false;
}

BOOL CDesignerViewRasterEdit::UpdateCursor()
{
	//DWORD dwPos = ;
	POINT tPt;// = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
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
	else
	{
		EControlKeysState eState/* = m_eKeysState;
		if (m_eDragState == EDSNothing)
		{
			eState*/ = (GetAsyncKeyState(VK_CONTROL)&0x8000) ?
				((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) :
				((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
		//}
		HCURSOR hCur = NULL;
		TPixelCoords tPos;
		GetPixelFromPoint(tPt, &tPos, NULL, NULL, eState);
		bool bDestroy = S_FALSE == m_pActiveTool->GetCursor(eState, &tPos, &hCur);
		if (hCur != NULL)
		{
			SetCursor(hCur);
			if (m_hLastCursor)
			{
				DestroyCursor(m_hLastCursor);
				m_hLastCursor = NULL;
			}
			if (bDestroy)
				m_hLastCursor = hCur;
			return TRUE;
		}
	}
	return FALSE;
}

LRESULT CDesignerViewRasterEdit::OnSetCursor(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pActiveTool != NULL && reinterpret_cast<HWND>(a_wParam) == m_hWnd && LOWORD(a_lParam) == HTCLIENT)
	{
		if (UpdateCursor())
			return TRUE;
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewRasterEdit::OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (a_wParam == VK_SHIFT || a_wParam == VK_CONTROL)
	{
		RECT rc;
		GetClientRect(&rc);
		DWORD dwPos = GetMessagePos();
		POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
		ScreenToClient(&tPt);
		DWORD dwDummy = 0;
		EControlKeysState eCKS = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
		TPixelCoords tPointerSize = {float(M_ImageSize().cx)/float(M_ZoomedSize().cx), float(M_ImageSize().cy)/float(M_ZoomedSize().cy)};
		m_pActiveTool->ProcessInputEvent(eCKS, &m_tLastPos, &tPointerSize, m_fLastPressure, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
		if (tPt.x >= 0 && tPt.x < rc.right && tPt.y >= 0 && tPt.y < rc.bottom)
		{
			if (!UpdateCursor())
			{
				static HCURSOR hArrow = ::LoadCursor(NULL, IDC_ARROW);
				SetCursor(hArrow);
			}
		}
	}
	else if (a_uMsg == WM_KEYDOWN && (a_lParam&0x40000000) == 0)
	{
		if (a_wParam == VK_RETURN)
		{
			if (S_OK == m_pActiveTool->IsDirty(NULL, NULL, NULL))
				ApplyChanges();
			return 0;
		}
		else if (a_wParam == VK_ESCAPE)
		{
			if (S_OK == m_pActiveTool->IsDirty(NULL, NULL, NULL))
				m_pActiveTool->Reset();
			else if (m_rcSelection.left || m_rcSelection.top ||
					 m_rcSelection.right != M_ImageSize().cx || m_rcSelection.bottom != M_ImageSize().cy ||
					 m_aSelection.m_p)
			{
				m_rcSelection.left = 0;
				m_rcSelection.top = 0;
				m_rcSelection.right = M_ImageSize().cx;
				m_rcSelection.bottom = M_ImageSize().cy;
				m_aSelection.Free();
				SendSelectionUpdate();
				RectangleChanged(NULL);
			}
			return 0;
		}
	}
	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewRasterEdit::OnSelDocGetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	//OnDeactivate(FALSE);
	//TRasterImageCoord* pSize = reinterpret_cast<TRasterImageCoord*>(a_wParam);
	//std::pair<TRasterImagePixel*, BYTE*>* ptSource = reinterpret_cast<std::pair<TRasterImagePixel*, BYTE*>*>(a_lParam);
	//CAutoVectorPtr<TRasterImagePixel> pData(M_ImageSize().cx*M_ImageSize().cy);
	//CAutoVectorPtr<BYTE> pSelection(M_ImageSize().cx*M_ImageSize().cy);
	//pSize->n0 = M_ImageSize().cx;
	//pSize->n1 = M_ImageSize().cy;
	//pSize->n2 = 1;
	//pSize->n3 = 1;
	//if (pData == NULL || pSelection == NULL)
	//	return 1;
	//if (FAILED(GetImageTile(0, 0, M_ImageSize().cx, M_ImageSize().cy, pData)))
	//	return 1;
	//if (FAILED(GetSelectionTile(0, 0, M_ImageSize().cx, M_ImageSize().cy, pSelection)))
	//	pSelection.Free();
	//ptSource->first = pData.Detach();
	//ptSource->second = pSelection.Detach();

	return 0;
}

LRESULT CDesignerViewRasterEdit::OnSelDocSetData(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
{
	//OnDeactivate(TRUE);
	//TRasterImageCoord* pSize = reinterpret_cast<TRasterImageCoord*>(a_wParam);
	//std::pair<TRasterImagePixel*, BYTE*>* ptSource = reinterpret_cast<std::pair<TRasterImagePixel*, BYTE*>*>(a_lParam);
	//if (pSize == NULL || ptSource == NULL)
	//	return 1;
	//if (pSize && pSize->n0 != M_ImageSize().cx || pSize->n1 != M_ImageSize().cy)
	//{
	//	m_p
	//}
	////if (GetEditTool() && wcscmp(GetEditTool()->ToolID(), TOOLID_MOVESELECT) == 0)
	////{
	////	CEditToolSelection<CDesignerViewRasterEdit>* pTool = static_cast<CEditToolSelection<CDesignerViewRasterEdit>*>(GetEditTool());
	////	TRasterImageCoord* pSize = reinterpret_cast<TRasterImageCoord*>(a_wParam);
	////	TRasterImagePixel* pData = reinterpret_cast<TRasterImagePixel*>(a_lParam);
	////	pTool->ModifySelection(pSize->n0, pSize->n1, pData);
	////}

	return 0;
}

void CDesignerViewRasterEdit::ActivateWindow()
{
	if (IsWindow())
	{
		HWND h = m_hWnd;
		GetParent().SendMessage(WM_RW_GOTFOCUS, reinterpret_cast<WPARAM>(m_hWnd), reinterpret_cast<LPARAM>(static_cast<IDesignerView*>(this)));
		SetFocus();
	}
}

#include "GestureOperationManager.h"

HRESULT CDesignerViewRasterEdit::GestureUndo(IConfig* UNREF(a_pCfg))
{
	if (S_OK == CanUndo())
	{
		return Undo();
	}
	else
	{
		CComQIPtr<IDocumentUndo> pUndo(m_pDoc);
		if (pUndo)
		{
			pUndo->Undo(1); // ignore return value
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CDesignerViewRasterEdit::GestureRedo(IConfig* UNREF(a_pCfg))
{
	CComQIPtr<IDocumentUndo> pUndo(m_pDoc);
	if (pUndo)
	{
		pUndo->Redo(1);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CDesignerViewRasterEdit::GestureApply(IConfig* UNREF(a_pCfg))
{
	return ApplyChanges(TRUE);
}

HRESULT CDesignerViewRasterEdit::GestureAutoZoom(IConfig* UNREF(a_pCfg))
{
	EnableAutoZoom(TRUE);
	return S_OK;
}

HRESULT CDesignerViewRasterEdit::GestureSwapColors(IConfig* UNREF(a_pCfg))
{
	// TODO
	//if (m_tColor1.fR != m_tColor2.fR || m_tColor1.fG != m_tColor2.fG ||
	//	m_tColor1.fB != m_tColor2.fB || m_tColor1.fA != m_tColor2.fA)
	//{
	//	TColor t = m_tColor1;
	//	m_tColor1 = m_tColor2;
	//	m_tColor2 = t;
	//	if (m_bstrColor1StateSync.Length())
	//	{
	//		CComPtr<ISharedStateColor> pState;
	//		RWCoCreateInstance(pState, __uuidof(SharedStateColor));
	//		pState->RGBASet(&m_tColor1.fR, NULL);
	//		m_bChangingSharedState = true;
	//		m_pStateMgr->StateSet(m_bstrColor1StateSync, pState);
	//		m_bChangingSharedState = false;
	//	}
	//	if (m_bstrColor2StateSync.Length())
	//	{
	//		CComPtr<ISharedStateColor> pState;
	//		RWCoCreateInstance(pState, __uuidof(SharedStateColor));
	//		pState->RGBASet(&m_tColor2.fR, NULL);
	//		m_bChangingSharedState = true;
	//		m_pStateMgr->StateSet(m_bstrColor2StateSync, pState);
	//		m_bChangingSharedState = false;
	//	}
	//	m_pActiveTool->SetColors(&m_tColor1, &m_tColor2);
	//	if (m_pActiveBrush)
	//		m_pActiveBrush->SetColors(&m_tColor1, &m_tColor2);
	//}
	return S_OK;
}

HRESULT CDesignerViewRasterEdit::GestureOutline(IConfig* a_pCfg)
{
	try
	{
		CConfigValue cVal;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTUREOUTLINEDDELTA), &cVal);
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		float fWidth = 1.0f;
		if (pPrev)
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fWidth, NULL, NULL);
		fWidth += cVal.operator float();
		if (fWidth < 0.0f)
			fWidth = 0.0f;
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fWidth, NULL, NULL, pPrev))
		{
			m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDesignerViewRasterEdit::GestureDrawMode(IConfig* a_pCfg)
{
	try
	{
		CConfigValue cVal;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTUREMODETYPE), &cVal);
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		EBlendingMode const* pBM = NULL;
		ERasterizationMode const* pRM = NULL;
		ECoordinatesMode const* pCM = NULL;
		TConfigValue tVal;
		switch (cVal.operator LONG())
		{
		case CFGVAL_GMT_BLEND:
			a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTUREBLENDMODE), &tVal);
			pBM = reinterpret_cast<EBlendingMode const*>(&tVal.iVal);
			break;
		case CFGVAL_GMT_RASTER:
			a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTUREBLENDMODE), &tVal);
			pRM = reinterpret_cast<ERasterizationMode const*>(&tVal.iVal);
			break;
		case CFGVAL_GMT_COORDS:
			a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTUREBLENDMODE), &tVal);
			pCM = reinterpret_cast<ECoordinatesMode const*>(&tVal.iVal);
			break;
		}
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(NULL, NULL, NULL, pBM, pRM, pCM, NULL, NULL, NULL, NULL, NULL, pPrev))
		{
			m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDesignerViewRasterEdit::GestureSwitchTool(IConfig* a_pCfg)
{
	try
	{
		CConfigValue cVal;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTURESWITCHTOOLID), &cVal);
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(cVal, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
		{
			m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDesignerViewRasterEdit::GestureSwitchStyle(IConfig* a_pCfg)
{
	try
	{
		CConfigValue cVal;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_GESTURESWITCHSTYLEID), &cVal);
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		BOOL bFill = TRUE;
		if (S_OK == pNew->Set(NULL, &bFill, cVal, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
		{
			m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


#include "SelectionDocument.h"
#include "SelectionBlockingOperationContext.h"

void CDesignerViewRasterEdit::ExecuteGesture(TConfigValue const& a_tOpID, IConfig* a_pOpCfg)
{
	CComObject<CGestureOperationManager>* pGestureOpMgr = NULL;
	CComObject<CGestureOperationManager>::CreateInstance(&pGestureOpMgr);
	CComPtr<IOperationManager> pGestureOperations = pGestureOpMgr;

	CComPtr<IDocument> pDoc;
	CComPtr<IOperationContext> pStatesTmp = m_pContext;
	CComQIPtr<IRasterImageEditToolFloatingSelection> pSelTool(m_pActiveTool);
	if (pSelTool)
	{
		ULONG nX = 0, nY = 0;
		pSelTool->Size(&nX, &nY);
		if (nX && nY)
		{
			CComObject<CSelectionDocument>* p = NULL;
			CComObject<CSelectionDocument>::CreateInstance(&p);
			pDoc = p;
			p->Init(pSelTool);
		}
		CComObject<CSelectionBlockingOperationContext>* pStates = NULL;
		CComObject<CSelectionBlockingOperationContext>::CreateInstance(&pStates);
		pStatesTmp = pStates;
		pStates->Init(m_bstrSelectionStateSync, m_pContext);
	}
	pGestureOpMgr->Init(m_pOpMgr, this, pSelTool.p == NULL);

	m_pContext->ResetErrorMessage();
	HRESULT hRes = pGestureOpMgr->Activate(pGestureOpMgr, pDoc.p ? pDoc.p : m_pDoc.p, &a_tOpID, a_pOpCfg, pStatesTmp, m_hWnd, m_tLocaleID);
	if (FAILED(hRes) && hRes != E_RW_CANCELLEDBYUSER)
	{
		CComBSTR bstr;
		if (m_pContext->M_ErrorMessage())
			m_pContext->M_ErrorMessage()->GetLocalized(m_tLocaleID, &bstr);
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
			TCHAR szTemplate[256] = _T("");
			Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_GENERICERROR, szTemplate, itemsof(szTemplate), LANGIDFROMLCID(m_tLocaleID));
			TCHAR szMsg[256];
			_stprintf(szMsg, szTemplate, hRes);
			::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

// IRasterImageEditWindow implementation

STDMETHODIMP CDesignerViewRasterEdit::Size(ULONG* a_pSizeX, ULONG* a_pSizeY)
{
	try
	{
		TImageSize tSize;
		m_pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		*a_pSizeX = tSize.nX;
		*a_pSizeY = tSize.nY;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::GetDefaultColor(TRasterImagePixel* a_pDefault)
{
	if (m_pImage == NULL) return E_UNEXPECTED;
	return m_pImage->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, reinterpret_cast<TPixelChannel*>(a_pDefault)));
}

STDMETHODIMP CDesignerViewRasterEdit::GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent UNREF(a_eIntent), TRasterImagePixel* a_pBuffer)
{
	if (m_pImage == NULL) return E_UNEXPECTED;
	return m_pImage->TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer), NULL, EIRIPreview);
}

STDMETHODIMP CDesignerViewRasterEdit::GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
{
	try
	{
		if (a_pBoundingRectangle)
		{
			*a_pBoundingRectangle = m_rcSelection;
		}
		if (a_bEntireRectangle)
		{
			*a_bEntireRectangle = m_aSelection.m_p ? FALSE : TRUE;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
{
	try
	{
		// unselected top margin
		while (a_nY < m_rcSelection.top && a_nSizeY)
		{
			FillMemory(a_pBuffer, a_nSizeX, 0);
			++a_nY;
			--a_nSizeY;
			a_pBuffer += a_nStride;
		}

		if (a_nSizeY == 0 || m_rcSelection.left >= m_rcSelection.right)
			return S_OK;

		// middle part
		int const nLMargin = min(int(a_nSizeX), int(m_rcSelection.left-a_nX));
		int const nRMargin = min(int(a_nSizeX), int(a_nX+a_nSizeX-m_rcSelection.right));
		int const nMiddle = a_nSizeX-max(0, nLMargin)-max(0, nRMargin);
		BYTE const* pSelection = m_aSelection.m_p+max(0, LONG(a_nX-m_rcSelection.left))+(m_rcSelection.right-m_rcSelection.left)*(a_nY-m_rcSelection.top);
		while (a_nY < m_rcSelection.bottom && a_nSizeY)
		{
			if (nLMargin > 0)
			{
				FillMemory(a_pBuffer, nLMargin, 0);
				a_pBuffer += nLMargin;
			}
			if (nMiddle > 0)
			{
				if (m_aSelection.m_p)
				{
					CopyMemory(a_pBuffer, pSelection, nMiddle);
					pSelection += m_rcSelection.right-m_rcSelection.left;
				}
				else
				{
					FillMemory(a_pBuffer, nMiddle, 0xff);
				}
				a_pBuffer += nMiddle;
			}
			if (nRMargin > 0)
			{
				FillMemory(a_pBuffer, nRMargin, 0);
				a_pBuffer += nRMargin;
			}
			a_pBuffer += a_nStride-a_nSizeX;
			++a_nY;
			--a_nSizeY;
		}

		// unselected bottom margin
		while (a_nSizeY)
		{
			FillMemory(a_pBuffer, a_nSizeX, 0);
			--a_nSizeY;
			a_pBuffer += a_nStride;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ControlPointsChanged()
{
	try
	{
		CHandles cOrig; // for invalid region handling
		bool const bActiveValid = m_iActiveHandle < m_cHandles.size();
		std::pair<TPixelCoords, ULONG> tPrevAct;
		if (bActiveValid) tPrevAct = m_cHandles[m_iActiveHandle];
		cOrig = m_cHandles;
		ULONG nCount = 0;
		m_pActiveTool->GetControlPointCount(&nCount);
		m_cHandles.resize(nCount);
		m_iActiveHandle = nCount;
		RECT rc = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
		for (ULONG i = 0; i < nCount; ++i)
		{
			std::pair<TPixelCoords, ULONG>& t = m_cHandles[i];
			m_pActiveTool->GetControlPoint(i, &t.first, &t.second);
			if (t.first.fX == tPrevAct.first.fX && t.first.fY == tPrevAct.first.fY)
				m_iActiveHandle = i;
			bool bFound = false;
			for (CHandles::iterator j = cOrig.begin(); j != cOrig.end(); ++j)
			{
				if (j->first.fX == t.first.fX && j->first.fY == t.first.fY && j->second == t.second)
				{
					bFound = true;
					cOrig.erase(j);
					break;
				}
			}
			if (!bFound)
			{
				LONG const nX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx)*t.first.fX+M_ImagePos().x;
				LONG const nY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy)*t.first.fY+M_ImagePos().y;
				if (rc.left > nX) rc.left = nX;
				if (rc.top > nY) rc.top = nY;
				if (rc.right < nX) rc.right = nX;
				if (rc.bottom < nY) rc.bottom = nY;
			}
		}
		for (CHandles::iterator j = cOrig.begin(); j != cOrig.end(); ++j)
		{
			LONG const nX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx)*j->first.fX+M_ImagePos().x;
			LONG const nY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy)*j->first.fY+M_ImagePos().y;
			if (rc.left > nX) rc.left = nX;
			if (rc.top > nY) rc.top = nY;
			if (rc.right < nX) rc.right = nX;
			if (rc.bottom < nY) rc.bottom = nY;
		}

		if (rc.left <= rc.right)
			SendDrawToolCmdLineUpdateLater();

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
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ControlPointChanged(ULONG a_nIndex)
{
	try
	{
		if (a_nIndex >= m_cHandles.size())
			return E_RW_INDEXOUTOFRANGE;
		float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
		float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
		std::pair<TPixelCoords, ULONG>& t = m_cHandles[a_nIndex];
		LONG const nX1 = fZoomX*t.first.fX+M_ImagePos().x;
		LONG const nY1 = fZoomY*t.first.fY+M_ImagePos().y;
		m_pActiveTool->GetControlPoint(a_nIndex, &t.first, &t.second);
		LONG const nX2 = fZoomX*t.first.fX+M_ImagePos().x;
		LONG const nY2 = fZoomY*t.first.fY+M_ImagePos().y;
		RECT rc = {min(nX1, nX2)-M_HandleRadius(), min(nY1, nY2)-M_HandleRadius(), max(nX1, nX2)+M_HandleRadius()+1, max(nY1, nY2)+M_HandleRadius()+1};
		if (rc.left < 0) rc.left = 0;
		if (rc.top < 0) rc.top = 0;
		if (rc.right > M_WindowSize().cx) rc.right = M_WindowSize().cx;
		if (rc.bottom > M_WindowSize().cy) rc.bottom = M_WindowSize().cy;
		if (rc.left < rc.right && rc.top < rc.bottom)
			InvalidateRect(&rc, FALSE);
		SendDrawToolCmdLineUpdateLater();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ControlLinesChanged()
{
	try
	{
		float fHelpLinesXMin = 1e6f;
		float fHelpLinesYMin = 1e6f;
		float fHelpLinesXMax = -1e6f;
		float fHelpLinesYMax = -1e6f;
		m_cHelpLines.clear();
		CComObjectStackEx<CControlLines> cLines;
		cLines.Init(m_cHelpLines, M_HandleRadius()/M_ImageZoom());
		if (m_pActiveTool)
			m_pActiveTool->GetControlLines(&cLines, ECLTHelp|ECLTSelection);
		for (CHelpLines::const_iterator i = m_cHelpLines.begin(); i != m_cHelpLines.end(); ++i)
		{
			if (fHelpLinesXMin > i->fX) fHelpLinesXMin = i->fX;
			if (fHelpLinesYMin > i->fY) fHelpLinesYMin = i->fY;
			if (fHelpLinesXMax < i->fX) fHelpLinesXMax = i->fX;
			if (fHelpLinesYMax < i->fY) fHelpLinesYMax = i->fY;
		}
		if (m_fHelpLinesXMin > fHelpLinesXMin) m_fHelpLinesXMin = fHelpLinesXMin;
		if (m_fHelpLinesYMin > fHelpLinesYMin) m_fHelpLinesYMin = fHelpLinesYMin;
		if (m_fHelpLinesXMax < fHelpLinesXMax) m_fHelpLinesXMax = fHelpLinesXMax;
		if (m_fHelpLinesYMax < fHelpLinesYMax) m_fHelpLinesYMax = fHelpLinesYMax;
		float const fZoomX = float(M_ZoomedSize().cx)/float(M_ImageSize().cx);
		float const fZoomY = float(M_ZoomedSize().cy)/float(M_ImageSize().cy);
		RECT rc =
		{
			fZoomX*m_fHelpLinesXMin+M_ImagePos().x-2,
			fZoomY*m_fHelpLinesYMin+M_ImagePos().y-2,
			fZoomX*m_fHelpLinesXMax+M_ImagePos().x+2,
			fZoomY*m_fHelpLinesYMax+M_ImagePos().y+2
		};
		m_fHelpLinesXMin = fHelpLinesXMin;
		m_fHelpLinesYMin = fHelpLinesYMin;
		m_fHelpLinesXMax = fHelpLinesXMax;
		m_fHelpLinesYMax = fHelpLinesYMax;
		if (rc.left < 0) rc.left = 0;
		if (rc.top < 0) rc.top = 0;
		if (rc.right > M_WindowSize().cx) rc.right = M_WindowSize().cx;
		if (rc.bottom > M_WindowSize().cy) rc.bottom = M_WindowSize().cy;
		if (rc.left < rc.right && rc.top < rc.bottom)
			InvalidateRect(&rc, FALSE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::RectangleChanged(RECT const* a_pChanged)
{
	if (m_hWnd == NULL)
		return S_FALSE;

	try
	{
		SendDrawToolCmdLineUpdateLater();
		if (a_pChanged == NULL)
		{
			Invalidate(FALSE);
			return S_OK;
		}
		InvalidateImageRectangle(*a_pChanged);
		return S_OK;
		//RECT rcChanged = *a_pChanged;
		//if (m_pComposedPreview)
		//	m_pComposedPreview->AdjustDirtyRect(&rcChanged);
		//RECT const rcSrc =
		//{
		//	rcChanged.left > 0 ? rcChanged.left : 0,
		//	rcChanged.top > 0 ? rcChanged.top : 0,
		//	rcChanged.right < M_ImageSize().cx ? rcChanged.right : M_ImageSize().cx,
		//	rcChanged.bottom < M_ImageSize().cy ? rcChanged.bottom : M_ImageSize().cy,
		//};
		//if (rcSrc.left < rcSrc.right && rcSrc.top < rcSrc.bottom)
		//{
		//	RECT rc =
		//	{
		//		float(rcSrc.left)*float(M_ZoomedSize().cx)/float(M_ImageSize().cx)+M_ImagePos().x,
		//		float(rcSrc.top)*float(M_ZoomedSize().cy)/float(M_ImageSize().cy)+M_ImagePos().y,
		//		float(rcSrc.right)*float(M_ZoomedSize().cx)/float(M_ImageSize().cx)+M_ImagePos().x+0.9999f,
		//		float(rcSrc.bottom)*float(M_ZoomedSize().cy)/float(M_ImageSize().cy)+M_ImagePos().y+0.9999f,
		//	};
		//	InvalidateRect(&rc, FALSE);
		//	return S_OK;
		//}
		//return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ScrollWindow(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
{
	try
	{
		if (m_nScrollIDLast != a_nScrollID)
		{
			m_nScrollIDLast == a_nScrollID;
			m_fScrollOrigX = M_OffsetX();
			m_fScrollOrigY = M_OffsetY();
		}
		MoveViewport(m_fScrollOrigX+2.0f*a_pDelta->fX/M_ZoomedSize().cx, m_fScrollOrigX+2.0f*a_pDelta->fY/M_ZoomedSize().cy, M_ImageZoom());
		//m_fOffsetX = m_fScrollOrigX+2.0f*a_pDelta->fX/M_ZoomedSize().cx;
		//m_fOffsetY = m_fScrollOrigX+2.0f*a_pDelta->fY/M_ZoomedSize().cy;
		//UpdatePosition();
		//UpdateWindow();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::ApplyChanges()
{
	return ApplyChanges(TRUE);
}

HRESULT CDesignerViewRasterEdit::ApplyChanges(BOOL a_bExplicit)
{
	try
	{
		CComQIPtr<IRasterImageEditToolCustomApply> pCustomApply(m_pActiveTool);
		if (pCustomApply)
		{
			HRESULT hRes = pCustomApply->ApplyChanges(a_bExplicit);
			if (SUCCEEDED(hRes))
			{
				m_pActiveTool->Reset();
				return hRes;
			}
		}
		RECT rcImage = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
		BOOL bOptimize = FALSE;
		RECT rcSelection = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
		m_pActiveTool->IsDirty(&rcImage, &bOptimize, &rcSelection);
		if (rcImage.left < 0) rcImage.left = 0;
		if (rcImage.top < 0) rcImage.top = 0;
		if (rcImage.right > M_ImageSize().cx) rcImage.right = M_ImageSize().cx;
		if (rcImage.bottom > M_ImageSize().cy) rcImage.bottom = M_ImageSize().cy;
		if (rcImage.left < rcImage.right && rcImage.top < rcImage.bottom)
		{
			CComPtr<ILocalizedString> pName;
			m_pTools->ToolNameGet(m_bstrToolID, &pName);
			CUndoBlock cUndo(m_pDoc, pName);

			ULONG const nSizeX = (rcImage.right-rcImage.left);
			ULONG const nSizeY = (rcImage.bottom-rcImage.top);
			ULONG const nPixels = nSizeX*nSizeY;
			TImagePoint const t1 = {rcImage.left, rcImage.top};
			TImageSize const t21 = {rcImage.right-rcImage.left, rcImage.bottom-rcImage.top};
			if (bOptimize && nPixels > 64)
			{
				CWriteLock<IDocument> cLock(m_pDoc);
				CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nPixels*2]);
				m_pActiveTool->GetImageTile(rcImage.left, rcImage.top, nSizeX, nSizeY, 2.2f, nSizeX, cBuffer);
				m_pImage->TileGet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel*>(cBuffer.m_p+nPixels), NULL, EIRIPreview);
				ATLASSERT(sizeof(DWORD) == sizeof(TRasterImagePixel));
				DWORD* const p1 = reinterpret_cast<DWORD*>(cBuffer.m_p);
				DWORD const* const p2 = p1+nPixels;
				ULONG nDifferent = 0;
				for (ULONG i = 0; i < nPixels; ++i)
					if (p1[i] != p2[i])
						++nDifferent;
				if (nDifferent*3 > nPixels)
				{
					m_pImage->TileSet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(cBuffer.m_p), FALSE);
				}
				else
				{
					ULONG i = 0;
					for (ULONG y = 0; y < nSizeY; ++y)
					{
						TImagePoint tPt = {0, y+t1.nY};
						TImageSize tSz = {0, 1};
						DWORD const* pStart = NULL;
						for (ULONG x = 0; x < nSizeX; ++x, ++i)
						{
							if (p1[i] != p2[i])
							{
								if (pStart == NULL)
								{
									tPt.nX = x+t1.nX;
									pStart = p1+i;
								}
							}
							else if (pStart)
							{
								tSz.nX = x+t1.nX-tPt.nX;
								m_pImage->TileSet(EICIRGBA, &tPt, &tSz, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(pStart), FALSE);
								pStart = NULL;
							}
						}
						if (pStart)
						{
							tSz.nX = nSizeX+t1.nX-tPt.nX;
							m_pImage->TileSet(EICIRGBA, &tPt, &tSz, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(pStart), FALSE);
							pStart = NULL;
						}
					}
				}
			}
			else
			{
				CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nPixels]);
				m_pActiveTool->GetImageTile(rcImage.left, rcImage.top, nSizeX, nSizeY, 2.2f, nSizeX, cBuffer);
				m_pImage->TileSet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(cBuffer.m_p), FALSE);
			}
		}
		if (m_bSelectionSupport && rcSelection.left < rcSelection.right && rcSelection.top < rcSelection.bottom)
		{
			RECT rcBounds = {0, 0, M_ImageSize().cx, M_ImageSize().cy};
			BOOL bEntire = TRUE;
			m_pActiveTool->GetSelectionInfo(&rcBounds, &bEntire);
			if (rcBounds.left == m_rcSelection.left && rcBounds.right == m_rcSelection.right &&
				rcBounds.top == m_rcSelection.top && rcBounds.bottom == m_rcSelection.bottom)
			{
				if (bEntire)
				{
					m_aSelection.Free();
				}
				else
				{
					if (m_aSelection.m_p == NULL)
					{
						CAutoVectorPtr<BYTE> pNew(new BYTE[(m_rcSelection.right-m_rcSelection.left)*(m_rcSelection.bottom-m_rcSelection.top)]);
						m_pActiveTool->GetSelectionTile(m_rcSelection.left, m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, m_rcSelection.bottom-m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, pNew.m_p);
						std::swap(pNew.m_p, m_aSelection.m_p);
					}
					else // small hack, reusing the buffer ~~~
						m_pActiveTool->GetSelectionTile(m_rcSelection.left, m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, m_rcSelection.bottom-m_rcSelection.top, m_rcSelection.right-m_rcSelection.left, m_aSelection.m_p);
				}
			}
			else
			{
				if (bEntire)
				{
					m_rcSelection = rcBounds;
					m_aSelection.Free();
				}
				else
				{
					CAutoVectorPtr<BYTE> pNew(new BYTE[(rcBounds.right-rcBounds.left)*(rcBounds.bottom-rcBounds.top)]);
					m_pActiveTool->GetSelectionTile(rcBounds.left, rcBounds.top, rcBounds.right-rcBounds.left, rcBounds.bottom-rcBounds.top, rcBounds.right-rcBounds.left, pNew.m_p);
					m_rcSelection = rcBounds;
					std::swap(pNew.m_p, m_aSelection.m_p);
				}
			}
			OptimizeSelection(m_aSelection, &m_rcSelection);
			SendSelectionUpdate();
		}
		m_pActiveTool->Reset();
		SendDrawToolCmdLineUpdateLater();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::SetState(ISharedState* a_pState)
{
	try
	{
		CComBSTR bstr(m_bstrToolStateSync);
		bstr += m_bstrToolID;
		m_bChangingSharedState = true;
		HRESULT hRes = m_pStateMgr->StateSet(bstr, a_pState);
		m_bChangingSharedState = false;
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::SetBrushState(BSTR a_bstrStyleID, ISharedState* a_pState)
{
	try
	{
		CComBSTR bstr(m_bstrToolStateSync);
		bstr += a_bstrStyleID ? a_bstrStyleID : m_bstrStyleID;
		m_bChangingSharedState = true;
		HRESULT hRes = m_pStateMgr->StateSet(bstr, a_pState);
		if (m_bstrStyleID != a_bstrStyleID)
		{
			CComPtr<ISharedStateToolMode> pPrev;
			m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
			CComObject<CSharedStateToolMode>* pNew = NULL;
			CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			BOOL bFill = TRUE;
			if (S_OK == pNew->Set(NULL, &bFill, a_bstrStyleID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
			{
				m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
			}
		}
		m_bChangingSharedState = false;
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Handle(RWHWND* a_phWnd)
{
	try
	{
		*a_phWnd = m_hWnd;
		return S_OK;
	}
	catch (...)
	{
		return a_phWnd ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewRasterEdit::Document(IDocument** a_ppDocument)
{
	try
	{
		*a_ppDocument = NULL;
		(*a_ppDocument = m_pDoc)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocument ? E_UNEXPECTED : E_POINTER;
	}
}

LRESULT CDesignerViewRasterEdit::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
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

void CDesignerViewRasterEdit::ProcessContextMenu(IEnumUnknowns* a_pOps, int a_xPos, int a_yPos)
{
	Reset(m_cImageList);
	// TODO: check if the image list is too big and eventually delete it

	ATLASSERT(m_cContextOps.empty());
	CMenu cMenu;
	cMenu.CreatePopupMenu();

	UINT nMenuID = 1000;
	InsertMenuItems(a_pOps, m_cContextOps, cMenu.m_hMenu, &nMenuID);
	if (nMenuID == 1000)
		return;

	UINT nSelection = cMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, a_xPos, a_yPos, m_hWnd, NULL);
	if (nSelection != 0)
	{
		CWaitCursor cWait;
		m_cContextOps[nSelection-1000]->Execute(reinterpret_cast<RWHWND>(m_hWnd), m_tLocaleID);
	}
	m_cContextOps.clear();
}

void CDesignerViewRasterEdit::InsertMenuItems(IEnumUnknowns* a_pCmds, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID)
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

LRESULT CDesignerViewRasterEdit::OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bTrackMouse)
	{
		DWORD dwDummy;
		if (m_pActiveTool)
			m_pActiveTool->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
		m_bTrackMouse = false;

		if (M_HideHandles() && !m_bMouseOut)
		{
			m_bMouseOut = true;
			Invalidate(FALSE);
		}
	}
	return 0;
}

