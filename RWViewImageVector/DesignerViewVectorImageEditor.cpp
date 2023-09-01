// DesignerViewVectorImageEditor.cpp : Implementation of window area rendering of CDesignerViewVectorImageEditor

#include "stdafx.h"
#include "DesignerViewVectorImageEditor.h"
#include "../RWViewImageRaster/ProcessorCount.h"
#include <MultiLanguageString.h>
#include <math.h>
#include <XPGUI.h>
#include <DocumentName.h>
#include <ReturnedData.h>
#include <SharedStateUndo.h>


void CDesignerViewVectorImageEditor::Init(ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IViewManager* a_pViewManager, IDocumentVectorImage* a_pImage)
{
	m_pDoc = a_pDoc;
	m_pImage = a_pImage;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pImage2));
	a_pDoc->QueryFeatureInterface(__uuidof(IStructuredItemsRichGUI), reinterpret_cast<void**>(&m_pRichGUI));

	a_pViewManager->QueryInterface(&m_pCmdMgr);
	if (m_pCmdMgr == NULL)
		RWCoCreateInstance(m_pCmdMgr, __uuidof(MenuCommandsManager));

	m_pStateMgr = a_pFrame;
	CComObject<COperationContextFromStateManager>::CreateInstance(&m_pContext.p);
	m_pContext.p->AddRef();
	m_pContext->Init(a_pFrame);

	m_tLocaleID = a_tLocaleID;
	m_pConfig = a_pConfig;

	TImageSize tSize = {1, 1};
	m_pImage2->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
	InitVisualization(a_pConfig, tSize.nX, tSize.nY);

	RWCoCreateInstance(m_pMGH, __uuidof(MouseGesturesHelper));

	CConfigValue cStateSync;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_NEWTOOLSYNC), &cStateSync);
	m_bstrNewToolStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_TOOLSYNC), &cStateSync);
	m_bstrToolStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_SELECTIONSYNC), &cStateSync);
	m_pImage->StatePrefix(&m_bstrSelectionStateSync);
	m_bstrSelectionStateSync += cStateSync.Detach().bstrVal;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_VIEWPORTSYNC), &cStateSync);
	m_bstrViewportStateSync.Attach(cStateSync.Detach().bstrVal);
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_TOOLCMDLINE), &cStateSync);
	m_bstrToolCommandStateSync.Attach(cStateSync.Detach().bstrVal);

	CConfigValue cToolMode;
	m_pConfig->ItemValueGet(CComBSTR(CFGID_RVIEDIT_TOOLMODE), &cToolMode);
	if (cToolMode.TypeGet() == ECVTString && cToolMode.operator BSTR() && *cToolMode.operator BSTR())
	{
		CComPtr<ISharedStateToolMode> pState;
		RWCoCreateInstance(pState, __uuidof(SharedStateToolMode));
		pState->FromText(cToolMode);
		pState->Get(NULL, &m_sActiveShape.bFill, &m_sActiveShape.bstrFill, NULL, &m_sActiveShape.eRM, &m_sActiveShape.eCM, &m_sActiveShape.bOutline, &m_sActiveShape.tColor, &m_sActiveShape.fWidth, &m_sActiveShape.fPos, &m_sActiveShape.eJoins);
	}

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
		m_pStateMgr->ObserverIns(CObserverImpl<CDesignerViewVectorImageEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	m_pImage->ObserverIns(CObserverImpl<CDesignerViewVectorImageEditor, IVectorImageObserver, TVectorImageChanges>::ObserverGet(), 0);
	m_pImage2->ObserverIns(CObserverImpl<CDesignerViewVectorImageEditor, IImageObserver, TImageChange>::ObserverGet(), 0);

	m_pStatusBar = a_pStatusBar;

	m_pImage->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(m_cShapeOrder));
	for (std::vector<ULONG>::const_iterator i = m_cShapeOrder.begin(); i != m_cShapeOrder.end(); ++i)
	{
		RefreshShape(*i, m_cShapes[*i]);
		m_cShapes[*i].bEnable = m_pImage->ObjectIsEnabled(*i) != S_FALSE;
	}

	if (m_pStateMgr)
	{
		CComPtr<ISharedState> pSelection;
		m_pStateMgr->StateGet(m_bstrSelectionStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pSelection));
		ULONG nSelected = 0;
		if (pSelection)
			m_pImage->StateUnpack(pSelection, &CEnumItemCounter<IEnum2UInts, ULONG>(&nSelected));

		// initial tool state
		CComPtr<ISharedStateToolMode> pNewTool;
		m_pStateMgr->StateGet(m_bstrNewToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pNewTool));
		CComBSTR bstrToolID;
		if (pNewTool)
			pNewTool->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		if (pSelection == NULL)
		{
			if (bstrToolID.m_str && bstrToolID != L"VECTORSELECT")
			{
				m_pImage->StatePack(0, NULL, &pSelection);
				if (pSelection)
					m_pStateMgr->StateSet(m_bstrSelectionStateSync, pSelection);
			}
			//else
			//{
			//	std::vector<ULONG> cAll;
			//	m_pImage->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cAll));
			//	m_pImage->StatePack(cAll.empty() ? 0 : 1, cAll.empty() ? NULL : &cAll[0], &pSelection);
			//	m_pStateMgr->StateSet(m_bstrSelectionStateSync, pSelection);
			//	nSelected = 1;
			//}
		}

		if (bstrToolID.m_str == NULL || (nSelected && bstrToolID.m_str != L"VECTORSELECT"))
		{
			CComPtr<ISharedStateToolMode> pNewTool2;
			RWCoCreateInstance(pNewTool2, __uuidof(SharedStateToolMode));
			pNewTool2->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			m_pStateMgr->StateSet(m_bstrNewToolStateSync, pNewTool2);
			m_sActiveShape.bstrTool = L"VECTORSELECT";
			m_sActiveShape.pTool = NULL;
		}

		if (nSelected == 0 && bstrToolID.m_str && bstrToolID.m_str != L"VECTORSELECT")
		{
			m_sActiveShape.bstrTool = bstrToolID;
			m_pToolMgr->EditToolCreate(m_sActiveShape.bstrTool, NULL, &m_sActiveShape.pTool);
			m_eAutoUnselect = EAUSensing;
		}

		if (m_sActiveShape.bstrFill && *m_sActiveShape.bstrFill)
			m_pFillMgr->FillStyleCreate(m_sActiveShape.bstrFill, NULL, &m_sActiveShape.pFill);
		if (m_sActiveShape.pTool)
		{
			m_bInitializing = true;
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
			m_bInitializing = false;
		}

		//CComPtr<ISharedState> pNewTool;
		//m_pStateMgr->StateGet(m_bstrNewToolStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pNewTool));
	}

	//m_dwLastMessage = GetTickCount()-50000;
	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("RVIEditFrame"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, (a_nStyle&EDVWSBorderMask) != EDVWSBorder ? 0 : WS_EX_CLIENTEDGE) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}

	MoveWindow(const_cast<LPRECT>(a_rcWindow));
	ShowWindow(SW_SHOW);
}

LRESULT CDesignerViewVectorImageEditor::OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	try
	{
		//m_cToolList.ReadInitialState();
		//SetEditTool(m_cToolList.ActiveTool());

		HDC hDC = GetDC();
		m_fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
		ReleaseDC(hDC);


		m_cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 16, 16);

		InitVisualizationPostCreate();
		SendViewportUpdate();

		CComPtr<ISharedState> pShapeSel;
		if (m_pStateMgr)
			m_pStateMgr->StateGet(m_bstrSelectionStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pShapeSel));
		TSharedStateChange tState;
		tState.bstrName = m_bstrSelectionStateSync;
		tState.pState = pShapeSel;
		OwnerNotify(0, tState);
		if (m_aSelected.size() != 1)
		{
			CComPtr<ISharedStateToolMode> pOldTM;
			m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOldTM));
			CComPtr<ISharedStateToolMode> pTM;
			RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
			static EBlendingMode const eBM = EBMDrawOver;
			// TODO
			if (S_OK == pTM->Set(m_sActiveShape.bstrTool, &m_sActiveShape.bFill, m_sActiveShape.bstrFill, &eBM, &m_sActiveShape.eRM, &m_sActiveShape.eCM, &m_sActiveShape.bOutline, &m_sActiveShape.tColor, &m_sActiveShape.fWidth, &m_sActiveShape.fPos, &m_sActiveShape.eJoins, pOldTM))
				m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);

			if (m_sActiveShape.pToolState)
			{
				CComBSTR bstr(m_bstrToolStateSync);
				bstr += m_sActiveShape.bstrTool;
				CComPtr<ISharedState> pOldTool;
				m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldTool));
				if (pOldTool != m_sActiveShape.pToolState)
					m_pStateMgr->StateSet(bstr, m_sActiveShape.pToolState);
			}
			if (m_sActiveShape.pFillState && m_sActiveShape.bstrFill.Length())
			{
				CComBSTR bstr(m_bstrToolStateSync);
				bstr += m_sActiveShape.bstrFill;
				CComPtr<ISharedState> pOldFill;
				m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldFill));
				if (pOldFill != m_sActiveShape.pFillState)
					m_pStateMgr->StateSet(bstr, m_sActiveShape.pFillState);
			}
		}
		//if (pShapeSel)
		//{
		//	m_pImage->StateUnpack(pShapeSel, &CEnumToVector<IEnum2UInts, ULONG>(m_aSelected));
		//	if (m_aSelected.size() == 1)
		//	{
		//		m_nActiveShape = m_aSelected[0];
		//		CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
		//		if (iSh != m_cShapes.end())
		//		{
		//			ULONG nHandles = 0;
		//			iSh->second.pTool->GetControlPointCount(&nHandles);
		//			m_cHandles.resize(nHandles);
		//			for (ULONG i = 0; i != nHandles; ++i)
		//				iSh->second.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);

		//			CComObjectStackEx<CControlLines> cLines;
		//			cLines.Init(m_cActiveHelpLines, m_nHandleRadius/M_ImageZoom());
		//			iSh->second.pTool->GetControlLines(&cLines, ECLTHelp|ECLTSelection);
		//		}
		//	}
		//	else
		//	{
		//		CComObjectStackEx<CControlLines> cLines;
		//		cLines.Init(m_cActiveHelpLines, m_nHandleRadius/M_ImageZoom());
		//		for (std::vector<ULONG>::const_iterator j = m_aSelected.begin(); j != m_aSelected.end(); ++j)
		//		{
		//			CShapes::const_iterator iSh = m_cShapes.find(*j);
		//			if (iSh != m_cShapes.end())
		//			{
		//				iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
		//			}
		//		}
		//	}
		//	SendDrawToolCmdLineUpdateLater();
		//}

		//ControlPointsChanged();
		//ControlLinesChanged();
	}
	catch (...)
	{
	}

	AddRef();

	return 0;
}

LRESULT CDesignerViewVectorImageEditor::OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_pImage)
	{
		m_pImage->ObserverDel(CObserverImpl<CDesignerViewVectorImageEditor, IVectorImageObserver, TVectorImageChanges>::ObserverGet(), 0);
		m_pImage = NULL;
	}
	if (m_pStateMgr)
	{
		m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewVectorImageEditor, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		m_pStateMgr = NULL;
	}
	a_bHandled = FALSE;
	return 0;
}

void CDesignerViewVectorImageEditor::RefreshShape(ULONG a_nID, SShape& a_sShape)
{
	m_bInitializing = true;
	bool bSetFill = false;
	bool bSetMisc = false;
	if (a_sShape.pCallback == NULL)
	{
		CComObject<CCallbackHelper<CDesignerViewVectorImageEditor> >::CreateInstance(&a_sShape.pCallback.p);
		a_sShape.pCallback.p->AddRef();
		a_sShape.pCallback->SetOwner(this, a_nID);
	}
	CComBSTR bstrToolID;
	CComBSTR bstrParams;
	m_pImage->ObjectGet(a_nID, &bstrToolID, &bstrParams);
	if (a_sShape.bstrTool != bstrToolID || a_sShape.pTool == NULL)
	{
		a_sShape.pTool = NULL;
		m_pToolMgr->EditToolCreate(bstrToolID, NULL, &a_sShape.pTool);
		if (a_sShape.pTool == NULL)
		{
			m_bInitializing = false;
			return;
		}
		a_sShape.bstrTool = bstrToolID;
		a_sShape.pTool->Init(a_sShape.pCallback);
		bSetFill = true;
		bSetMisc = true;
		CComQIPtr<IRasterImageEditToolScripting> pScr(a_sShape.pTool);
		if (pScr)
		{
			CComBSTR bstrParamsOld;
			pScr->ToText(&bstrParamsOld);
			if (bstrParamsOld != bstrParams)
				pScr->FromText(bstrParams);
		}
	}
	else
	{
		CComQIPtr<IRasterImageEditToolScripting> pScr(a_sShape.pTool);
		if (pScr)
		{
			CComBSTR bstrParamsOld;
			pScr->ToText(&bstrParamsOld);
			if (bstrParamsOld != bstrParams)
				pScr->FromText(bstrParams);
		}
	}

	TColor tColor = a_sShape.tColor;
	BOOL bFill = a_sShape.bFill;
	BOOL bOutline = a_sShape.bOutline;
	ERasterizationMode eRM = a_sShape.eRM;
	ECoordinatesMode eCM = a_sShape.eCM;
	float fWidth = a_sShape.fWidth;
	float fPos = a_sShape.fPos;
	EOutlineJoinType eJoins = a_sShape.eJoins;
	m_pImage->ObjectStateGet(a_nID, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);

	CComBSTR bstrStyleID;
	CComBSTR bstrStyleParams;
	m_pImage->ObjectStyleGet(a_nID, &bstrStyleID, &bstrStyleParams);
	if (a_sShape.bstrFill != bstrStyleID || a_sShape.pFill == NULL)
	{
		if (a_sShape.pFill && a_sShape.bFill)
			a_sShape.pTool->SetBrush(NULL);
		a_sShape.pFill = NULL;
		bSetMisc = true;
		if (bstrStyleID && *bstrStyleID)
		{
			a_sShape.bstrFill = bstrStyleID;
			m_pFillMgr->FillStyleCreate(bstrStyleID, NULL, &a_sShape.pFill);
			a_sShape.pTool->SetBrush(bFill ? a_sShape.pFill.p : NULL);
			if (bFill)
			{
				CComQIPtr<IRasterImageEditToolScripting> pScripting(a_sShape.pFill);
				if (pScripting && bstrStyleParams)
					pScripting->FromText(bstrStyleParams);
				if (a_sShape.pFillState == NULL)
					a_sShape.pFill->GetState(&a_sShape.pFillState);
			}
		}
	}
	else
	{
		a_sShape.pTool->SetBrush(bFill ? a_sShape.pFill.p : NULL);
		if (bFill)
		{
			CComQIPtr<IRasterImageEditToolScripting> pScr(a_sShape.pFill);
			if (pScr)
			{
				CComBSTR bstrParamsOld;
				pScr->ToText(&bstrParamsOld);
				if (bstrParamsOld != bstrStyleParams)
				{
					pScr->FromText(bstrStyleParams);
				}
			}
			if (a_sShape.pFillState == NULL)
				a_sShape.pFill->GetState(&a_sShape.pFillState);
		}
	}
	if (bSetMisc || eRM != a_sShape.eRM || eCM != a_sShape.eCM || bFill != a_sShape.bFill || bOutline != a_sShape.bOutline || fabsf(fWidth-a_sShape.fWidth) > 1e-3f || fabsf(fPos-a_sShape.fPos) > 1e-3f || eJoins != a_sShape.eJoins || memcmp(&tColor, &a_sShape.tColor, sizeof tColor))
	{
		a_sShape.eRM = eRM;
		a_sShape.eCM = eCM;
		a_sShape.bFill = bFill;
		a_sShape.bOutline = bOutline;
		a_sShape.tColor = tColor;
		a_sShape.fWidth = fWidth;
		a_sShape.fPos = fPos;
		a_sShape.eJoins = eJoins;
		a_sShape.pTool->SetGlobals(EBMDrawOver, eRM, eCM);
		a_sShape.pTool->SetOutline(bOutline, fWidth, fPos, eJoins, &tColor);
	}
	m_bInitializing = false;
}

LRESULT CDesignerViewVectorImageEditor::OnKeyChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	/*if (a_wParam == VK_SHIFT || a_wParam == VK_CONTROL)
	{
		RECT rc;
		GetClientRect(&rc);
		DWORD dwPos = GetMessagePos();
		POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
		ScreenToClient(&tPt);
		DWORD dwDummy = 0;
		EControlKeysState eCKS = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
		TPixelCoords tPointerSize = {float(m_tImageSize.cx)/float(m_tZoomedSize.cx), float(m_tImageSize.cy)/float(m_tZoomedSize.cy)};
		m_sActiveShape.pTool->ProcessInputEvent(eCKS, &m_tLastPos, &tPointerSize, m_fLastPressure, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
		if (tPt.x >= 0 && tPt.x < rc.right && tPt.y >= 0 && tPt.y < rc.bottom)
		{
			if (!UpdateCursor())
			{
				static HCURSOR hArrow = ::LoadCursor(NULL, IDC_ARROW);
				SetCursor(hArrow);
			}
		}
	}
	else */if (a_uMsg == WM_KEYDOWN && (a_lParam&0x40000000) == 0)
	{
		if (a_wParam == VK_RETURN)
		{
			if (S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
				ApplyChanges();
			return 0;
		}
		else if (a_wParam == VK_ESCAPE)
		{
			Undo();
			//if (S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
			//	m_sActiveShape.pTool->Reset();
			//else if (m_rcSelection.left || m_rcSelection.top ||
			//		 m_rcSelection.right != m_tImageSize.cx || m_rcSelection.bottom != m_tImageSize.cy ||
			//		 m_aSelection.m_p)
			//{
			//	m_rcSelection.left = 0;
			//	m_rcSelection.top = 0;
			//	m_rcSelection.right = m_tImageSize.cx;
			//	m_rcSelection.bottom = m_tImageSize.cy;
			//	m_aSelection.Free();
			//	SendSelectionUpdate();
			//	RectangleChanged(NULL);
			//}
			return 0;
		}
	}
	a_bHandled = FALSE;
	return 0;
}

STDMETHODIMP CDesignerViewVectorImageEditor::PreTranslateMessage(MSG const* a_pMsg, BOOL a_bBeforeAccel)
{
	try
	{
		if (!a_bBeforeAccel || a_pMsg->hwnd != m_hWnd)
			return S_FALSE;
		if (m_aSelected.size() > 1)
		{
			if (a_pMsg->message == WM_KEYDOWN &&
				(a_pMsg->wParam == VK_LEFT || a_pMsg->wParam == VK_RIGHT ||
				 a_pMsg->wParam == VK_UP || a_pMsg->wParam == VK_DOWN))
			{
				TMatrix3x3f tM =
				{
					1.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					0.0f, 0.0f, 1.0f
				};
				tM._31 = a_pMsg->wParam == VK_LEFT ? -1 : (a_pMsg->wParam == VK_RIGHT ? 1 : 0);
				tM._32 = a_pMsg->wParam == VK_UP ? -1 : (a_pMsg->wParam == VK_DOWN ? 1 : 0);
				for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
				{
					CShapes::const_iterator j = m_cShapes.find(*i);
					if (j == m_cShapes.end())
						continue;
					j->second.pTool->Transform(&tM);
				}

				InvalidateHelpLines(m_cActiveHelpLines);
				m_cActiveHelpLines.clear();
				m_cHotHelpLines.clear();
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

				return S_OK;
			}
			else if (a_pMsg->message == WM_KEYUP &&
				(a_pMsg->wParam == VK_LEFT || a_pMsg->wParam == VK_RIGHT ||
				 a_pMsg->wParam == VK_UP || a_pMsg->wParam == VK_DOWN))
			{
				ApplyChanges(FALSE, 0);
			}
		}
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			if (S_OK == m_sActiveShape.pTool->PreTranslateMessage(a_pMsg))
			{
				if (a_pMsg->message == WM_KEYUP && S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
				{
					ApplyChanges(FALSE, 0);
				}
				ProcessUpdateWindow();
				return S_OK;
			}
		}
		else if (m_nActiveShape)
		{
			CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
			if (iSh != m_cShapes.end() && S_OK == iSh->second.pTool->PreTranslateMessage(a_pMsg))
			{
				if (a_pMsg->message == WM_KEYUP && S_OK == iSh->second.pTool->IsDirty(NULL, NULL, NULL))
				{
					ApplyChanges(FALSE, 0);
				}
				ProcessUpdateWindow();
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool IsSameColor(TColor const& a_1, TColor const& a_2)
{
	float const f =
		(a_1.fR*a_1.fA-a_2.fR*a_2.fA)*(a_1.fR*a_1.fA-a_2.fR*a_2.fA) +
		(a_1.fG*a_1.fA-a_2.fG*a_2.fA)*(a_1.fG*a_1.fA-a_2.fG*a_2.fA) +
		(a_1.fB*a_1.fA-a_2.fB*a_2.fA)*(a_1.fB*a_1.fA-a_2.fB*a_2.fA) +
		(a_1.fA-a_2.fA)*(a_1.fA-a_2.fA);
	return f < 1.0f/(255*255);
}

void CDesignerViewVectorImageEditor::OwnerNotify(TCookie, TSharedStateChange a_tState)
{
	if (m_hWnd == NULL)
		return;

	try
	{
		if (m_bstrSelectionStateSync == a_tState.bstrName && a_tState.pState)
		{
			std::vector<ULONG> aSelected;
			m_pImage->StateUnpack(a_tState.pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
			if (aSelected.size() == m_aSelected.size())
			{
				ULONG i = 0;
				for (ULONG n = m_aSelected.size(); i < n; ++i)
					if (aSelected[i] != m_aSelected[i])
						break;
				if (i == m_aSelected.size())
					return; // no changes
			}
			if (aSelected != m_aSelected)
			{
				ApplyChanges(FALSE, 0);

				std::swap(aSelected, m_aSelected);
				ULONG nPrevActiveShape = m_nActiveShape;
				if (m_aSelected.size() == 1)
				{
					m_nActiveShape = m_aSelected[0];
				}
				else
				{
					m_nActiveShape = 0;
				}
				CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
				if (iSh == m_cShapes.end())
				{
					m_nActiveShape = 0;
				}

				if (m_nActiveShape)
				{
					InvalidateHandles(m_cHandles);
					ULONG nHandles = 0;
					iSh->second.pTool->GetControlPointCount(&nHandles);
					m_cHandles.resize(nHandles);
					for (ULONG i = 0; i != nHandles; ++i)
						iSh->second.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);
					InvalidateHandles(m_cHandles);
					m_iActiveHandle = -1;

					CComObjectStackEx<CControlLines> cLines;
					InvalidateHelpLines(m_cActiveHelpLines);
					m_cActiveHelpLines.clear();
					cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
					iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
					cLines.SetType(ECLTHelp);
					iSh->second.pTool->GetControlLines(&cLines, ECLTHelp);
					InvalidateHelpLines(m_cActiveHelpLines);

					UpdateStates(iSh->second);

					CComPtr<ISharedStateToolMode> pTMN;
					RWCoCreateInstance(pTMN, __uuidof(SharedStateToolMode));
					pTMN->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
					m_pStateMgr->StateSet(m_bstrNewToolStateSync, pTMN);
				}
				else
				{
					InvalidateHandles(m_cHandles);
					m_cHandles.clear();
					InvalidateHelpLines(m_cActiveHelpLines);
					m_cActiveHelpLines.clear();
					m_iActiveHandle = -1;

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

					m_eMS = EMSUnknown;

					if (m_aSelected.size() < 2)
					{
						RestoreToolStates();
						if (m_aSelected.empty())
						{
							//CComPtr<ISharedStateToolMode> pTMN;
							//RWCoCreateInstance(pTMN, __uuidof(SharedStateToolMode));
							//pTMN->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
							//m_pStateMgr->StateSet(m_bstrNewToolStateSync, pTMN);
							CComPtr<ISharedStateToolMode> pOldTM;
							m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOldTM));
							CComPtr<ISharedStateToolMode> pTM;
							RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
							if (S_OK == pTM->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pOldTM))
								m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);
						}
					}
				}
			}
			SendDrawToolCmdLineUpdateLater();
		}
		else if (m_bstrToolCommandStateSync == a_tState.bstrName)
		{
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
				if (m_aSelected.empty() && m_sActiveShape.pTool)
				{
					bool bOK = true;
					if (m_sActiveShape.bstrTool != psz)
					{
						//CComBSTR bstrToolID(psz);
						//CComPtr<IRasterImageEditTool> pNewTool;
						//m_pToolMgr->EditToolCreate(bstrToolID, m_pDoc, &pNewTool);
						//if (pNewTool)
						//{
						//	m_pActiveTool->Reset();
						//	m_pActiveTool = pNewTool;
						//	m_bstrToolID = bstrToolID;
						//	InitTool();
						//	ControlPointsChanged();
						//	ControlLinesChanged();

						//	CComPtr<ISharedStateToolMode> pPrev;
						//	m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
						//	CComObject<CSharedStateToolMode>* pNew = NULL;
						//	CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
						//	CComPtr<ISharedState> pTmp = pNew;
						//	if (S_OK == pNew->Set(m_bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
						//	{
						//		m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
						//	}
						//}
						//else
						//{
							bOK = false;
						//}
					}
					if (bOK)
					{
						CComQIPtr<IRasterImageEditToolScripting> pScript(m_sActiveShape.pTool);
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
				else if (m_nActiveShape)
				{
					CShapes::iterator iSh = m_cShapes.find(m_nActiveShape);
					if (iSh != m_cShapes.end())
					{
						bool bOK = true;
						if (iSh->second.bstrTool != psz)
						{
							//CComBSTR bstrToolID(psz);
							//CComPtr<IRasterImageEditTool> pNewTool;
							//m_pToolMgr->EditToolCreate(bstrToolID, m_pDoc, &pNewTool);
							//if (pNewTool)
							//{
							//	m_pActiveTool->Reset();
							//	m_pActiveTool = pNewTool;
							//	m_bstrToolID = bstrToolID;
							//	InitTool();
							//	ControlPointsChanged();
							//	ControlLinesChanged();

							//	CComPtr<ISharedStateToolMode> pPrev;
							//	m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
							//	CComObject<CSharedStateToolMode>* pNew = NULL;
							//	CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
							//	CComPtr<ISharedState> pTmp = pNew;
							//	if (S_OK == pNew->Set(m_bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
							//	{
							//		m_pStateMgr->StateSet(m_bstrToolStateSync, pTmp);
							//	}
							//}
							//else
							//{
								bOK = false;
							//}
						}
						if (bOK)
						{
							CComQIPtr<IRasterImageEditToolScripting> pScript(iSh->second.pTool);
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
									m_cDirty.insert(m_nActiveShape);
								}
							}
						}
					}
				}
			}
		}
		else if (m_bstrNewToolStateSync == a_tState.bstrName)
		{
			CComQIPtr<ISharedStateToolMode> pMyState(a_tState.pState);
			if (pMyState == NULL)
				return;

			CComBSTR bstrToolID;
			pMyState->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			if (m_sActiveShape.bstrTool != bstrToolID)
			{
				ApplyChanges(FALSE, 0);
				m_sActiveShape.bstrTool = bstrToolID;
				if (m_sActiveShape.bstrTool == L"VECTORSELECT")
				{
					m_sActiveShape.pTool = NULL;
					m_eMS = EMSUnknown;
				}
				else
				{
					m_eMS = EMSUnknown;

					// deselect shape
					CComPtr<ISharedState> pState;
					m_pImage->StatePack(0, NULL, &pState);
					m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
					m_aSelected.clear();
					m_nActiveShape = 0;
					m_iActiveHandle = -1;
					InvalidateHandles(m_cHandles);
					m_cHandles.clear();

					m_sActiveShape.pFill = NULL;
					if (m_sActiveShape.bstrFill && *m_sActiveShape.bstrFill)
						m_pFillMgr->FillStyleCreate(m_sActiveShape.bstrFill, NULL, &m_sActiveShape.pFill);
					m_sActiveShape.pTool = NULL;
					m_pToolMgr->EditToolCreate(m_sActiveShape.bstrTool, NULL, &m_sActiveShape.pTool);
					if (m_sActiveShape.pTool)
					{
						m_sActiveShape.pTool->Init(m_sActiveShape.pCallback);
						m_sActiveShape.pTool->SetBrush(m_sActiveShape.bFill ? m_sActiveShape.pFill.p : NULL);
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
						InvalidateHandles(m_cHandles);
						ULONG nHandles = 0;
						m_sActiveShape.pTool->GetControlPointCount(&nHandles);
						m_cHandles.resize(nHandles);
						for (ULONG i = 0; i != nHandles; ++i)
							m_sActiveShape.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);
						InvalidateHandles(m_cHandles);
					}

					CComPtr<ISharedStateToolMode> pOld;
					m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOld));
					CComPtr<ISharedStateToolMode> pTM;
					RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
					static EBlendingMode const eBM = EBMDrawOver;
					// TODO
					pTM->Set(m_sActiveShape.bstrTool, &m_sActiveShape.bFill, m_sActiveShape.bstrFill, &eBM, &m_sActiveShape.eRM, &m_sActiveShape.eCM, &m_sActiveShape.bOutline, &m_sActiveShape.tColor, &m_sActiveShape.fWidth, &m_sActiveShape.fPos, &m_sActiveShape.eJoins, pOld);
					m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);

					RestoreToolStates();

					m_eAutoUnselect = EAUSensing;
				}
				SendDrawToolCmdLineUpdateLater();
			}
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
					CShapes::iterator j = m_cShapes.end();
					if ((m_aSelected.empty() && m_sActiveShape.pTool) ||
						(m_aSelected.size() == 1 && (j = m_cShapes.find(m_aSelected[0])) != m_cShapes.end() && j->second.bstrTool == m_sActiveShape.bstrTool))
					{
						if (m_sActiveShape.bstrTool == p2)
						{
							m_sActiveShape.pToolState = a_tState.pState;
							m_sActiveShape.pTool->SetState(a_tState.pState);
							if (j != m_cShapes.end())
							{
								j->second.pToolState = a_tState.pState;
								j->second.pTool->SetState(a_tState.pState);
							}
						}
						else if (m_sActiveShape.pFill && m_sActiveShape.bstrFill == p2)
						{
							m_sActiveShape.pFillState = a_tState.pState;
							m_sActiveShape.pFill->SetState(a_tState.pState);
							if (j != m_cShapes.end())
							{
								j->second.pFillState = a_tState.pState;
								j->second.pFill->SetState(a_tState.pState);
							}
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
							m_pConfig->SubConfigGet(CComBSTR(CFGID_RVIEDIT_EDITTOOLSTATES), &pSubCfg);
							CComBSTR bstrTool(p2);
							pSubCfg->ItemValuesSet(1, &(bstrTool.m_str), CConfigValue(bstr));
						}
						ApplyChanges(FALSE, 0);
					}
					if (!m_aSelected.empty() || m_sActiveShape.pTool == NULL)
					{
						for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
						{
							CShapes::iterator iSh = m_cShapes.find(*i);
							if (iSh == m_cShapes.end())
								continue;
							if (iSh->second.bstrTool == p2)
							{
								if (iSh->second.pToolState.p != a_tState.pState)
								{
									iSh->second.pToolState = a_tState.pState;
									iSh->second.pTool->SetState(a_tState.pState);
									m_cDirty.insert(iSh->first);
								}
								continue;
							}
							if (iSh->second.bstrFill == p2)
							{
								if (iSh->second.pFillState.p != a_tState.pState)
								{
									iSh->second.pFillState = a_tState.pState;
									iSh->second.pFill->SetState(a_tState.pState);
									m_cDirty.insert(iSh->first);
								}
							}
						}
					}
				}
				else
				{
					CComQIPtr<ISharedStateToolMode> pMyState(a_tState.pState);
					if (pMyState == NULL)
						return;

					CComBSTR bstrToolID;
					CComBSTR bstrStyleID;
					ERasterizationMode eRM = ERMSmooth;
					ECoordinatesMode eCM = ECMFloatingPoint;
					BOOL bFill = TRUE;
					BOOL bOutline = FALSE;
					float fWidth = 1.0f;
					float fPos = 0.0f;
					EOutlineJoinType eJoins = EOJTRound;
					TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
					pMyState->Get(&bstrToolID, &bFill, &bstrStyleID, NULL, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
					CShapes::iterator j;
					if ((m_aSelected.empty() && m_sActiveShape.pTool) ||
						(m_aSelected.size() == 1 && (j = m_cShapes.find(m_aSelected[0])) != m_cShapes.end() && j->second.bstrTool == m_sActiveShape.bstrTool))
					{
						CComBSTR bstrPersistent;
						if (SUCCEEDED(pMyState->ToText(&bstrPersistent)) && bstrPersistent.m_str)
						{
							CComBSTR bstrID(CFGID_RVIEDIT_TOOLMODE);
							m_pConfig->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(bstrPersistent));
						}

						if (m_sActiveShape.bstrFill != bstrStyleID || bFill != m_sActiveShape.bFill)
						{
							m_sActiveShape.bstrFill = bstrStyleID;
							m_sActiveShape.pFill = NULL;
							m_pFillMgr->FillStyleCreate(bstrStyleID, NULL, &m_sActiveShape.pFill);
							m_sActiveShape.pTool->SetBrush(bFill ? m_sActiveShape.pFill.p : NULL);
							if (m_sActiveShape.pFill)
							{
								CComBSTR bstr(m_bstrToolStateSync);
								bstr += m_sActiveShape.bstrFill;
								CComPtr<ISharedState> pState;
								m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
								if (pState)
								{
									m_sActiveShape.pFillState = pState;
									m_sActiveShape.pFill->SetState(pState);
								}
								//m_sActiveShape.pFill->SetColors(&m_sActiveShape.tColor1, &m_sActiveShape.tColor2);
							}
						}
						m_sActiveShape.eRM = eRM;
						m_sActiveShape.eCM = eCM;
						m_sActiveShape.bFill = bFill;
						m_sActiveShape.bOutline = bOutline;
						m_sActiveShape.fWidth = fWidth;
						m_sActiveShape.fPos = fPos;
						m_sActiveShape.eJoins = eJoins;
						m_sActiveShape.tColor = tColor;
						m_sActiveShape.pTool->SetGlobals(EBMDrawOver, m_sActiveShape.eRM, m_sActiveShape.eCM);
						m_sActiveShape.pTool->SetOutline(m_sActiveShape.bOutline, m_sActiveShape.fWidth, m_sActiveShape.fPos, m_sActiveShape.eJoins, &m_sActiveShape.tColor);
					}
					if (!m_aSelected.empty() || m_sActiveShape.pTool == NULL)
					{
						bool bApply = false;
						for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
						{
							CShapes::iterator iSh = m_cShapes.find(*i);
							if (iSh == m_cShapes.end())
								continue;
							if (iSh->second.bstrFill != bstrStyleID)
							{
								iSh->second.bstrFill = bstrStyleID;
								iSh->second.pFill = NULL;
								m_pFillMgr->FillStyleCreate(bstrStyleID, NULL, &iSh->second.pFill);
								iSh->second.pTool->SetBrush(bFill ? iSh->second.pFill.p : NULL);
								if (iSh->second.pFill)
								{
									CComBSTR bstr(m_bstrToolStateSync);
									bstr += iSh->second.bstrFill;
									CComPtr<ISharedState> pState;
									m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
									if (pState)
									{
										iSh->second.pFillState = pState;
										iSh->second.pFill->SetState(pState);
									}
									//iSh->second.pFill->SetColors(&iSh->second.tColor1, &iSh->second.tColor2);
								}
							}
							iSh->second.eRM = eRM;
							iSh->second.eCM = eCM;
							iSh->second.bFill = bFill;
							iSh->second.bOutline = bOutline;
							iSh->second.fWidth = fWidth;
							iSh->second.fPos = fPos;
							iSh->second.eJoins = eJoins;
							iSh->second.tColor = tColor;
							iSh->second.pTool->SetGlobals(EBMDrawOver, iSh->second.eRM, iSh->second.eCM);
							iSh->second.pTool->SetOutline(iSh->second.bOutline, iSh->second.fWidth, iSh->second.fPos, iSh->second.eJoins, &iSh->second.tColor);
							m_cDirty.insert(iSh->first);
							bApply = true;
						}
						if (bApply)
							ApplyChanges(FALSE, 0);
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

static int const MARGIN_TOP = 3;
static int const MARGIN_BOTTOM = 3;
static int const MARGIN_LEFT = 3;
static int const MARGIN_RIGHT = 3;




// ----- image painting -----

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_conv_dash.h>
#include <agg_span_allocator.h>

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

void CDesignerViewVectorImageEditor::GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
{
	TPixelChannel tBackground;
	float fGamma = 2.2f;
	m_pImage2->ChannelsGet(NULL, &fGamma, CImageChannelDefaultGetter(EICIRGBA, &tBackground));

	if (a_nStride != a_nSizeX)
	{
		for (ULONG y = 0; y < a_nSizeY; ++y)
			std::fill_n(a_pBuffer+a_nStride*y, a_nSizeX, *reinterpret_cast<TRasterImagePixel*>(&tBackground));
	}
	else
	{
		std::fill_n(a_pBuffer, a_nSizeX*a_nSizeY, *reinterpret_cast<TRasterImagePixel*>(&tBackground));
	}
	//m_pToolWindow->SetROI(a_tOrigin, a_tSize, a_pBuffer, tBackground, CImageSize(m_nSizeX, m_nSizeY));
	for (std::vector<ULONG>::const_iterator iO = m_cShapeOrder.begin(); iO != m_cShapeOrder.end(); ++iO)
	{

		CShapes::const_iterator i = m_cShapes.find(*iO);
		if (i != m_cShapes.end())
		{
			if (i->second.bEnable == 0)
				continue;
			RECT rcDirty = {0, 0, 0, 0};
			if (S_OK == i->second.pTool->IsDirty(&rcDirty, NULL, NULL))
			{
				if (rcDirty.left < a_nX) rcDirty.left = a_nX;
				if (rcDirty.top < a_nY) rcDirty.top = a_nY;
				if (rcDirty.right > LONG(a_nX+a_nSizeX)) rcDirty.right = a_nX+a_nSizeX;
				if (rcDirty.bottom > LONG(a_nY+a_nSizeY)) rcDirty.bottom = a_nY+a_nSizeY;
				if (rcDirty.left < rcDirty.right && rcDirty.top < rcDirty.bottom)
					i->second.pTool->GetImageTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, fGamma, a_nStride, reinterpret_cast<TRasterImagePixel*>(a_pBuffer)+a_nStride*(rcDirty.top-a_nY)+rcDirty.left-a_nX);
			}
		}
	}

	if (m_sActiveShape.pTool)
		m_sActiveShape.pTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, fGamma, a_nStride, a_pBuffer);
	//else
	//	m_pImage2->TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer), NULL, EIRIPreview);
}


void CDesignerViewVectorImageEditor::DrawHelpLines(CHelpLines::const_iterator begin, CHelpLines::const_iterator end, float fZoomX, float fZoomY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
{
	float fHelpLinesXMin = 1e6f;
	float fHelpLinesYMin = 1e6f;
	float fHelpLinesXMax = -1e6f;
	float fHelpLinesYMax = -1e6f;
	CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
	for (CHelpLines::const_iterator i = begin; i != end; ++i)
	{
		TVector2f t = {i->fX, i->fY};
		if (pCustApply == NULL)
			t = TransformVector2(M_InputTransform(), t);
		if (fHelpLinesXMin > t.x) fHelpLinesXMin = t.x;
		if (fHelpLinesYMin > t.y) fHelpLinesYMin = t.y;
		if (fHelpLinesXMax < t.x) fHelpLinesXMax = t.x;
		if (fHelpLinesYMax < t.y) fHelpLinesYMax = t.y;
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
		for (CHelpLines::const_iterator i = begin; i != end; ++i)
		{
			TVector2f t = {i->fX, i->fY};
			if (pCustApply == NULL)
				t = TransformVector2(M_InputTransform(), t);
			if (i->bLineTo)
			{
				path.line_to(fZoomX*t.x+M_ImagePos().x-a_rcDirty.left, fZoomY*t.y+M_ImagePos().y-a_rcDirty.top);
				if (i->bClose)
					path.close_polygon();
			}
			else
			{
				path.move_to(fZoomX*t.x+M_ImagePos().x-a_rcDirty.left, fZoomY*t.y+M_ImagePos().y-a_rcDirty.top);
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

void CDesignerViewVectorImageEditor::PostRenderImage(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
{
	//if (m_bMouseOut)
	//	return;

	SIZE szZoomed = M_ZoomedSize();
	SIZE szOrig = M_ImageSize();
	float fZoomX = float(szZoomed.cx)/szOrig.cx;
	float fZoomY = float(szZoomed.cy)/szOrig.cy;

	//std::fill_n(a_pBuffer, a_rcDirty.right-a_rcDirty.left, 0xff0000);
	//std::fill_n(a_pBuffer+(a_rcDirty.right-a_rcDirty.left)*(a_rcDirty.bottom-a_rcDirty.top-1), a_rcDirty.right-a_rcDirty.left, 0xff0000);
	CShapes::const_iterator const iHotShape = m_cShapes.find(m_nHotShape);
	CShapes::const_iterator const iActiveShape = m_cShapes.find(m_nActiveShape);


	if (M_HideHandles() && m_bMouseOut)
	{
		if (!m_cActiveHelpLines.empty())
		{
			CHelpLines::const_iterator end = m_cActiveHelpLines.begin();
			while (end != m_cActiveHelpLines.end() && end->type == ECLTSelection) ++end;
			DrawHelpLines(m_cActiveHelpLines.begin(), end, fZoomX, fZoomY, a_rcDirty, a_pBuffer, a_nStride);
		}
		return;
	}

	if (!m_cActiveHelpLines.empty())
		DrawHelpLines(m_cActiveHelpLines.begin(), m_cActiveHelpLines.end(), fZoomX, fZoomY, a_rcDirty, a_pBuffer, a_nStride);

	if (!m_cHotHelpLines.empty() && iActiveShape != iHotShape)
		DrawHelpLines(m_cHotHelpLines.begin(), m_cHotHelpLines.end(), fZoomX, fZoomY, a_rcDirty, a_pBuffer, a_nStride);

	// draw handles
	CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
	for (CHandles::const_iterator i = m_cHandles.begin(); i != m_cHandles.end(); ++i)
	{
		TVector2f t = {i->first.fX, i->first.fY};
		if (pCustApply == NULL)
			t = TransformVector2(M_InputTransform(), t);
		TPixelCoords tPos = {t.x, t.y};
		ULONG nClass = i->second;
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
		bool const hot = m_nHotHandle == i-m_cHandles.begin() || m_iActiveHandle == i-m_cHandles.begin();
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

	RenderMouseTrail(a_rcImage, a_nWindowX, a_nWindowY, a_rcDirty, a_pBuffer, a_nStride);
}

//bool CDesignerViewVectorImageEditor::IsSelected(ULONG a_nItemID)
//{
//	for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
//	{
//		if (*i == a_nItemID)
//			return true;
//	}
//	return false;
//}

bool CDesignerViewVectorImageEditor::TestHandle(POINT const& a_tPos, ULONG* a_pIndex) const
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

bool CDesignerViewVectorImageEditor::TestHandle(TPixelCoords const& a_tPos, ULONG* a_pIndex) const
{
	float const fRadSq = (M_HandleRadius()+0.5f)*(M_HandleRadius()+0.5f)/M_ImageZoom()/M_ImageZoom();
	for (CHandles::const_reverse_iterator i = m_cHandles.rbegin(); i != m_cHandles.rend(); ++i)
	{
		float const fDistSq = (a_tPos.fX-i->first.fX)*(a_tPos.fX-i->first.fX) + (a_tPos.fY-i->first.fY)*(a_tPos.fY-i->first.fY);
		if (fDistSq <= fRadSq)
		{
			*a_pIndex = m_cHandles.rend()-i-1;
			return true;
		}
	}
	return false;
}

CDesignerViewVectorImageEditor::CShapes::const_iterator CDesignerViewVectorImageEditor::ShapeFromPoint(TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize) const
{
	for (std::vector<ULONG>::const_reverse_iterator iO = m_cShapeOrder.rbegin(); iO != m_cShapeOrder.rend(); ++iO)
	{
		CShapes::const_iterator i = m_cShapes.find(*iO);
		if (i != m_cShapes.end() && i->second.pTool)
		{
			TPixelCoords tPos = *a_pPos;
			i->second.pTool->AdjustCoordinates(ECKSNone, &tPos, a_pPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
			if ((i->second.pTool->PointTest(ECKSNone, &tPos, FALSE, sqrtf(a_pPointerSize->fX*a_pPointerSize->fY))&ETPAHitMask) == ETPAHit)
				return i;
		}
	}
	return m_cShapes.end();
}

void CDesignerViewVectorImageEditor::ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
{
	bool bPrevMouseOut = m_bMouseOut;
	m_bMouseOut = a_pPos == NULL && a_fNormalPressure < 0.333f;
	if (M_HideHandles() && bPrevMouseOut != m_bMouseOut)
		Invalidate(FALSE);
	ULONG const nPrevHot = m_nHotHandle;
	m_nHotHandle = 0xffffffff;

	TPixelCoords tPos;
	if (a_pPos)
	{
		CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
		TVector2f t = {a_pPos->fX, a_pPos->fY};
		if (pCustApply == NULL)
		{
			t = TransformVector2(M_InputInvTransform(), t);
			tPos.fX = t.x;
			tPos.fY = t.y;
			a_pPos = &tPos;
		}
	}

	CShapes::const_iterator iActiveShape = m_cShapes.find(m_nActiveShape);

	if (m_eMS == EMSUnknown && a_fNormalPressure < 0.333f)
	{
		m_eMS = m_aSelected.empty() && m_sActiveShape.pTool ? EMSCreating : (m_aSelected.size() <= 1 ? EMSModifying : EMSTransforming);
	}

	TPixelCoords const tPointerSize = {float(M_ImageSize().cx)/float(M_ZoomedSize().cx), float(M_ImageSize().cy)/float(M_ZoomedSize().cy)};

	if (m_aSelected.size() == 1)
	{
		switch (m_eMS)
		{
		case EMSCreating: m_eMS = EMSModifying; break;
		case EMSCreatingForwarding: m_eMS = EMSModifyingForwarding; break;
		case EMSCreatingHandle: m_eMS = EMSModifyingHandle; break;
		}
	}
	else if (m_aSelected.empty() && m_sActiveShape.pTool)
	{
		switch (m_eMS)
		{
		case EMSModifying: m_eMS = EMSCreating; break;
		case EMSModifyingForwarding: m_eMS = EMSCreatingForwarding; break;
		case EMSModifyingHandle: m_eMS = EMSCreatingHandle; break;
		}
	}
	//else if (m_aSelected.size() > 1)
	//{
	//	m_eMS = EMSTransforming;
	//}

	switch (m_eMS)
	{
	case EMSModifying:
		if (a_fNormalPressure >= 0.666f)
		{
			ULONG nPt = 0;
			if (TestHandle(*a_pPos, &nPt))
			{
				m_eMS = EMSModifyingHandle;
				m_tHandleDragOffset.fX = (m_cHandles[nPt].first.fX-a_pPos->fX)*M_ImageZoom();
				m_tHandleDragOffset.fY = (m_cHandles[nPt].first.fY-a_pPos->fY)*M_ImageZoom();
				m_iActiveHandle = nPt;
				break;
			}

			if (a_eKeysState != ECKSNone)
			{
				// multi-selecting?
				if (a_pPos == NULL)
					break;

				CShapes::const_iterator i = ShapeFromPoint(a_pPos, &tPointerSize);
				if (i == m_cShapes.end())
					break;
				bool bSelChanged = false;
				std::vector<ULONG> aSelected = m_aSelected;
				if (a_eKeysState&ECKSControl)
				{
					for (std::vector<ULONG>::const_iterator j = aSelected.begin(); j != aSelected.end(); ++j)
						if (*j == i->first)
						{
							bSelChanged = true;
							aSelected.erase(j);
							break;
						}
					if (!bSelChanged)
					{
						bSelChanged = true;
						aSelected.push_back(i->first);
					}
				}
				else if (a_eKeysState&ECKSShift)
				{
					bSelChanged = true;
					for (std::vector<ULONG>::const_iterator j = aSelected.begin(); j != aSelected.end(); ++j)
						if (*j == i->first)
						{
							bSelChanged = false;
							break;
						}
					if (bSelChanged)
					{
						aSelected.push_back(i->first);
					}
				}
				if (bSelChanged)
				{
					ApplyChanges(FALSE, 0);

					std::swap(aSelected, m_aSelected);

					m_nActiveShape = m_aSelected.size() != 1 ? 0 : m_aSelected[0];
					CComPtr<ISharedState> pState;
					m_pImage->StatePack(m_aSelected.size(), m_aSelected.size() ? &m_aSelected[0] : NULL, &pState);
					m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);

					if (m_nActiveShape)
					{
						CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
						InvalidateHandles(m_cHandles);
						ULONG nHandles = 0;
						iSh->second.pTool->GetControlPointCount(&nHandles);
						m_cHandles.resize(nHandles);
						for (ULONG i = 0; i != nHandles; ++i)
							iSh->second.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);
						InvalidateHandles(m_cHandles);
						m_iActiveHandle = -1;

						CComObjectStackEx<CControlLines> cLines;
						InvalidateHelpLines(m_cActiveHelpLines);
						m_cActiveHelpLines.clear();
						cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
						iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
						cLines.SetType(ECLTHelp);
						iSh->second.pTool->GetControlLines(&cLines, ECLTHelp);
						InvalidateHelpLines(m_cActiveHelpLines);

						UpdateStates(iSh->second);

						m_eMS = EMSModifyingSelecting;
					}
					else
					{
						InvalidateHandles(m_cHandles);
						m_cHandles.clear();
						InvalidateHelpLines(m_cActiveHelpLines);
						m_cActiveHelpLines.clear();
						m_iActiveHandle = -1;

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

						if (m_aSelected.size() < 2)
							RestoreToolStates();

						m_eMS = EMSModifyingSelecting;
					}

					if (m_sActiveShape.pTool)
					{
						CComPtr<ISharedStateToolMode> pTMN;
						RWCoCreateInstance(pTMN, __uuidof(SharedStateToolMode));
						pTMN->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
						m_pStateMgr->StateSet(m_bstrNewToolStateSync, pTMN);
					}

					SendDrawToolCmdLineUpdateLater();
					break;
				}
				if (a_eKeysState != ECKSShift)
					break;
			}

			if (iActiveShape != m_cShapes.end())
			{
				// no handle hit, forward call to the tool
				CComPtr<IRasterImageEditTool> pTmp(iActiveShape->second.pTool); // m_sActiveShape.pTool can be deleted in the middle of the action
				if (a_pPos)
				{
					TPixelCoords tPos = *a_pPos;
					pTmp->AdjustCoordinates(ECKSNone, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
					HRESULT hHitTest = iActiveShape->second.pTool->PointTest(a_eKeysState, &tPos, TRUE, sqrtf(tPointerSize.fX*tPointerSize.fY));
					switch (hHitTest&ETPAActionMask)
					{
					case ETPAStartNew:
						ApplyChanges(FALSE, iActiveShape->first);
						iActiveShape = m_cShapes.end();
						break;
					case ETPATransform:
						if (iActiveShape != ShapeFromPoint(a_pPos, &tPointerSize))
						{
							iActiveShape = m_cShapes.end();
							break;
						}
						else if (a_eKeysState == ECKSShift)
						{
							m_eMS = EMSTransforming;
							m_tTransformStart = *a_pPos;
							m_cTransformStart.clear();
							m_bTransforming = true;
							m_bTransformingShift = true;
							m_tTransformCenter.fX = 0.0f;
							m_tTransformCenter.fY = 0.0f;
							RECT rcBounds = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
							iActiveShape->second.pTool->IsDirty(&rcBounds, NULL, NULL);
							CComQIPtr<IRasterImageEditToolScripting> pScr(iActiveShape->second.pTool);
							if (pScr)
							{
								CComBSTR bstr;
								pScr->ToText(&bstr);
								m_cTransformStart[iActiveShape->first].Attach(bstr.Detach());
							}
							if (rcBounds.left < rcBounds.right && rcBounds.top < rcBounds.bottom)
							{
								m_tTransformCenter.fX = 0.5f*(rcBounds.left+rcBounds.right);
								m_tTransformCenter.fY = 0.5f*(rcBounds.top+rcBounds.bottom);
							}
							break;
						}
					case ETPACustomAction:
						pTmp->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
						m_eMS = EMSModifyingForwarding;
						break;
					}
				}
			}

			if (iActiveShape == m_cShapes.end())
			{
				m_eMS = EMSUnknown;
				if (m_sActiveShape.pTool)
				{
					if (a_pPos)
					{
						TPixelCoords tPos = *a_pPos;
						m_sActiveShape.pTool->AdjustCoordinates(ECKSNone, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
						m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
					}
					else
						m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
					m_eMS = EMSCreatingForwarding;
					m_aSelected.clear();
					m_nActiveShape = 0;
					ControlLinesChanged(0);
					ControlPointsChanged(0);
					SendDrawToolCmdLineUpdateLater();
					if (m_eAutoUnselect == EAUSensing)
					{
						m_tAutoUnselect = *a_pPos;
						m_eAutoUnselect = EAUWatching;
					}
				}
				else
				{
					CShapes::const_iterator i = ShapeFromPoint(a_pPos, &tPointerSize);
					if (i != m_cShapes.end())
					{
						if (m_aSelected.size() != 1 || m_aSelected[0] != i->first)
						{
							CComPtr<ISharedState> pState;
							m_pImage->StatePack(1, &i->first, &pState);
							m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
							m_aSelected.clear();
							m_aSelected.push_back(i->first);
							m_nActiveShape = i->first;
							UpdateStates(i->second);
						}
						iActiveShape = i;
						TPixelCoords tPos = *a_pPos;
						iActiveShape->second.pTool->AdjustCoordinates(ECKSNone, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
						iActiveShape->second.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
						m_eMS = EMSModifyingForwarding;
						ControlLinesChanged(i->first);
						ControlPointsChanged(i->first);
						SendDrawToolCmdLineUpdateLater();
					}
					else if (m_nActiveShape != 0)
					{
						CComPtr<ISharedState> pState;
						m_pImage->StatePack(0, NULL, &pState);
						m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
						m_nActiveShape = 0;
						m_aSelected.clear();
						iActiveShape = m_cShapes.end();
						m_eMS = EMSModifying;
						InvalidateHelpLines(m_cActiveHelpLines);
						m_cActiveHelpLines.clear();
						InvalidateHandles(m_cHandles);
						m_cHandles.clear();
						SetUpdateWindowFlag();
						SendDrawToolCmdLineUpdateLater();

						CComPtr<ISharedStateToolMode> pOldTM;
						m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOldTM));
						CComPtr<ISharedStateToolMode> pTM;
						RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
						if (S_OK == pTM->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pOldTM))
							m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);
					}
				}
			}
		}
		else
		{
			// waiting for a click...
			ULONG nPt = 0xffffffff;
			if (a_pPos && TestHandle(*a_pPos, &nPt))
			{
				m_nHotHandle = nPt;
			}
			else if (iActiveShape != m_cShapes.end() && iActiveShape->second.pTool)
			{
				if (a_pPos)
				{
					TPixelCoords tPos = *a_pPos;
					iActiveShape->second.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
					iActiveShape->second.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
				}
				else
					iActiveShape->second.pTool->ProcessInputEvent(ECKSNone, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			}
		}
		break;
	case EMSModifyingHandle:
		if (iActiveShape == m_cShapes.end())
		{
			m_eMS = EMSUnknown;
			break;
		}
		if (a_pPos)
		{
			TPixelCoords tPos = *a_pPos;
			tPos.fX += m_tHandleDragOffset.fX/M_ImageZoom();
			tPos.fY += m_tHandleDragOffset.fY/M_ImageZoom();
			iActiveShape->second.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, &m_iActiveHandle, (M_HandleRadius()+0.5f)/M_ImageZoom());
			iActiveShape->second.pTool->SetControlPoint(m_iActiveHandle, &tPos, a_fNormalPressure < 0.5f, (M_HandleRadius()+0.5f)/M_ImageZoom());
		}
		else
			iActiveShape->second.pTool->SetControlPoint(m_iActiveHandle, a_pPos, a_fNormalPressure < 0.5f, (M_HandleRadius()+0.5f)/M_ImageZoom());
		if (a_fNormalPressure < 0.333f)
		{
			ApplyChanges(FALSE, 0);
			m_eMS = EMSModifying;
			m_iActiveHandle = -1;
		}
		break;
	case EMSModifyingForwarding:
		if (iActiveShape == m_cShapes.end())
		{
			m_eMS = EMSUnknown;
			break;
		}
		{
			CComPtr<IRasterImageEditTool> pTmp(iActiveShape->second.pTool); // m_sActiveShape.pTool can be deleted in the middle of the action
			HRESULT hRes;
			if (a_pPos)
			{
				TPixelCoords tPos = *a_pPos;
				pTmp->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
				hRes = pTmp->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			}
			else
				hRes = pTmp->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			if (hRes&ETPAApply)
				ApplyChanges(TRUE, 0);
		}
		if (a_fNormalPressure < 0.333f)
		{
			ApplyChanges(FALSE, 0);
			m_eMS = EMSModifying;
		}
		break;
	case EMSModifyingSelecting:
		if (a_fNormalPressure < 0.333f)
			m_eMS = m_aSelected.size() <= 1 ? EMSModifying : EMSTransforming;
		break;
	case EMSTransforming:
		if (a_fNormalPressure > 0.666f && a_eKeysState != ECKSNone && !m_bTransforming)
		{
			// multi-selecting?
			if (a_pPos == NULL)
				break;

			CShapes::const_iterator i = ShapeFromPoint(a_pPos, &tPointerSize);
			if (i == m_cShapes.end())
				break;
			bool bSelChanged = false;
			std::vector<ULONG> aSelected = m_aSelected;
			if (a_eKeysState&ECKSControl)
			{
				for (std::vector<ULONG>::const_iterator j = aSelected.begin(); j != aSelected.end(); ++j)
					if (*j == i->first)
					{
						bSelChanged = true;
						aSelected.erase(j);
						break;
					}
				if (!bSelChanged)
				{
					bSelChanged = true;
					aSelected.push_back(i->first);
				}
			}
			else if (a_eKeysState&ECKSShift)
			{
				bSelChanged = true;
				for (std::vector<ULONG>::const_iterator j = aSelected.begin(); j != aSelected.end(); ++j)
					if (*j == i->first)
					{
						bSelChanged = false;
						break;
					}
				if (bSelChanged)
				{
					aSelected.push_back(i->first);
				}
			}
			if (bSelChanged)
			{
				ApplyChanges(FALSE, 0);

				std::swap(m_aSelected, aSelected);

				m_nActiveShape = m_aSelected.size() != 1 ? 0 : m_aSelected[0];
				CComPtr<ISharedState> pState;
				m_pImage->StatePack(m_aSelected.size(), m_aSelected.size() ? &m_aSelected[0] : NULL, &pState);
				m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);

				if (m_nActiveShape)
				{
					CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
					InvalidateHandles(m_cHandles);
					ULONG nHandles = 0;
					iSh->second.pTool->GetControlPointCount(&nHandles);
					m_cHandles.resize(nHandles);
					for (ULONG i = 0; i != nHandles; ++i)
						iSh->second.pTool->GetControlPoint(i, &m_cHandles[i].first, &m_cHandles[i].second);
					InvalidateHandles(m_cHandles);
					m_iActiveHandle = -1;

					CComObjectStackEx<CControlLines> cLines;
					InvalidateHelpLines(m_cActiveHelpLines);
					m_cActiveHelpLines.clear();
					cLines.Init(m_cActiveHelpLines, M_HandleRadius()/M_ImageZoom());
					iSh->second.pTool->GetControlLines(&cLines, ECLTSelection);
					cLines.SetType(ECLTHelp);
					iSh->second.pTool->GetControlLines(&cLines, ECLTHelp);
					InvalidateHelpLines(m_cActiveHelpLines);

					UpdateStates(iSh->second);

					m_eMS = EMSModifyingSelecting;
				}
				else
				{
					InvalidateHandles(m_cHandles);
					m_cHandles.clear();
					InvalidateHelpLines(m_cActiveHelpLines);
					m_cActiveHelpLines.clear();
					m_iActiveHandle = -1;

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

					if (m_aSelected.size() < 2)
						RestoreToolStates();

					m_eMS = EMSModifyingSelecting;
				}

				if (m_sActiveShape.pTool)
				{
					CComPtr<ISharedStateToolMode> pTMN;
					RWCoCreateInstance(pTMN, __uuidof(SharedStateToolMode));
					pTMN->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
					m_pStateMgr->StateSet(m_bstrNewToolStateSync, pTMN);
				}

				SendDrawToolCmdLineUpdateLater();
				break;
			}
			if (a_eKeysState != ECKSShift)
				break;
		}
		if (a_pPos == NULL)
			break;
		if (m_bTransforming)
		{
			if (a_fNormalPressure < 0.333f)
			{
				ApplyChanges(FALSE, 0);

				m_bTransforming = false;
				m_cTransformStart.clear();
				m_eMS = m_aSelected.size() <= 1 ? EMSModifying : EMSTransforming;
			}
			else
			{
				if (m_tLastPos.fX != a_pPos->fX || m_tLastPos.fY != a_pPos->fY)
				{
					TMatrix3x3f tM =
					{
						1.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f
					};
					if (m_bTransformingShift)
					{
						float fAngle = atan2f(a_pPos->fY-m_tTransformCenter.fY, a_pPos->fX-m_tTransformCenter.fX) -
									   atan2f(m_tTransformStart.fY-m_tTransformCenter.fY, m_tTransformStart.fX-m_tTransformCenter.fX);
						float fZoom = sqrtf((a_pPos->fX-m_tTransformCenter.fX)*(a_pPos->fX-m_tTransformCenter.fX) + (a_pPos->fY-m_tTransformCenter.fY)*(a_pPos->fY-m_tTransformCenter.fY)) /
									  sqrtf((m_tTransformStart.fX-m_tTransformCenter.fX)*(m_tTransformStart.fX-m_tTransformCenter.fX) + (m_tTransformStart.fY-m_tTransformCenter.fY)*(m_tTransformStart.fY-m_tTransformCenter.fY));
						tM._11 = tM._22 = fZoom*cosf(fAngle);
						tM._21 = -(tM._12 = fZoom*sinf(fAngle));
						tM._31 = m_tTransformCenter.fX - (m_tTransformCenter.fX*tM._11+m_tTransformCenter.fY*tM._21);
						tM._32 = m_tTransformCenter.fY - (m_tTransformCenter.fX*tM._12+m_tTransformCenter.fY*tM._22);
					}
					else
					{
						tM._31 = a_pPos->fX-m_tTransformStart.fX;
						tM._32 = a_pPos->fY-m_tTransformStart.fY;
					}
					for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
					{
						CShapes::const_iterator j = m_cShapes.find(*i);
						if (j == m_cShapes.end())
							continue;
						CComQIPtr<IRasterImageEditToolScripting> pScr(j->second.pTool);
						if (pScr)
						{
							std::map<ULONG, CComBSTR>::const_iterator k = m_cTransformStart.find(*i);
							if (k != m_cTransformStart.end())
							{
								pScr->FromText(k->second);
								j->second.pTool->Transform(&tM);
							}
						}
					}

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
				}
			}
		}
		else
		{
			if (a_fNormalPressure > 0.666f)
			{
				CShapes::const_iterator i = ShapeFromPoint(a_pPos, &tPointerSize);
				if (i != m_cShapes.end())
				{
					bool bSelChanged = true;
					for (std::vector<ULONG>::const_iterator j = m_aSelected.begin(); j != m_aSelected.end(); ++j)
						if (*j == i->first)
						{
							bSelChanged = false;
							break;
						}
					if (bSelChanged)
					{
						CComPtr<ISharedState> pState;
						m_pImage->StatePack(1, &i->first, &pState);
						m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
						m_aSelected.clear();
						m_aSelected.push_back(i->first);
						m_nActiveShape = i->first;
						UpdateStates(i->second);
						iActiveShape = i;
						TPixelCoords tPos = *a_pPos;
						iActiveShape->second.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
						iActiveShape->second.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
						m_eMS = EMSModifyingForwarding;
						ControlLinesChanged(i->first);
						ControlPointsChanged(i->first);
						SendDrawToolCmdLineUpdateLater();
						break;
					}
				}
				else if (!m_aSelected.empty())
				{
					CComPtr<ISharedState> pState;
					m_pImage->StatePack(0, NULL, &pState);
					m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
					m_nActiveShape = 0;
					m_aSelected.clear();
					iActiveShape = m_cShapes.end();
					m_eMS = EMSModifying;
					InvalidateHelpLines(m_cActiveHelpLines);
					m_cActiveHelpLines.clear();
					InvalidateHandles(m_cHandles);
					m_cHandles.clear();
					SetUpdateWindowFlag();
					SendDrawToolCmdLineUpdateLater();
					break;
				}

				m_tTransformStart = *a_pPos;
				m_cTransformStart.clear();
				m_bTransforming = true;
				m_bTransformingShift = a_eKeysState&ECKSShift;
				RECT rcBounds = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
				m_tTransformCenter.fX = 0.0f;
				m_tTransformCenter.fY = 0.0f;
				for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
				{
					CShapes::const_iterator j = m_cShapes.find(*i);
					if (j == m_cShapes.end())
						continue;
					if (m_bTransformingShift)
					{
						RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
						j->second.pTool->IsDirty(&rc, NULL, NULL);
						if (rcBounds.left > rc.left) rcBounds.left = rc.left;
						if (rcBounds.top > rc.top) rcBounds.top = rc.top;
						if (rcBounds.right < rc.right) rcBounds.right = rc.right;
						if (rcBounds.bottom < rc.bottom) rcBounds.bottom = rc.bottom;
					}
					CComQIPtr<IRasterImageEditToolScripting> pScr(j->second.pTool);
					if (pScr)
					{
						CComBSTR bstr;
						pScr->ToText(&bstr);
						m_cTransformStart[*i].Attach(bstr.Detach());
					}
				}
				if (rcBounds.left < rcBounds.right && rcBounds.top < rcBounds.bottom)
				{
					m_tTransformCenter.fX = 0.5f*(rcBounds.left+rcBounds.right);
					m_tTransformCenter.fY = 0.5f*(rcBounds.top+rcBounds.bottom);
				}
			}
		}
		break;
	case EMSCreating:
		if (m_sActiveShape.pTool == NULL)
		{
			m_eMS = EMSUnknown;
			break;
		}
		if (a_fNormalPressure > 0.666f)
		{
			ULONG nPt = 0;
			if (TestHandle(*a_pPos, &nPt))
			{
				m_eMS = EMSCreatingHandle;
				m_iActiveHandle = nPt;
				break;
			}

			// adding new shape
			HRESULT hRes;
			if (a_pPos)
			{
				TPixelCoords tPos = *a_pPos;
				m_sActiveShape.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			}
			else
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			if (hRes&ETPAApply)
			{
				ApplyChanges(TRUE, 0);
				m_eMS = EMSModifyingForwarding;
			}
			else
			{
				m_eMS = EMSCreatingForwarding;
			}

			if (m_eAutoUnselect == EAUSensing)
			{
				m_tAutoUnselect = *a_pPos;
				m_eAutoUnselect = EAUWatching;
			}
		}
		else
		{
			// adding new shape
			HRESULT hRes;
			if (a_pPos)
			{
				TPixelCoords tPos = *a_pPos;
				m_sActiveShape.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			}
			else
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			if (hRes&ETPAApply)
			{
				ApplyChanges(TRUE, 0);
			}
		}
		break;
	case EMSCreatingHandle:
		if (m_sActiveShape.pTool == NULL)
		{
			m_eMS = EMSUnknown;
			break;
		}
		if (a_pPos)
		{
			TPixelCoords tPos = *a_pPos;
			m_sActiveShape.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, &m_iActiveHandle, (M_HandleRadius()+0.5f)/M_ImageZoom());
			m_sActiveShape.pTool->SetControlPoint(m_iActiveHandle, &tPos, a_fNormalPressure < 0.5f, (M_HandleRadius()+0.5f)/M_ImageZoom());
		}
		else
			m_sActiveShape.pTool->SetControlPoint(m_iActiveHandle, a_pPos, a_fNormalPressure < 0.5f, (M_HandleRadius()+0.5f)/M_ImageZoom());
		if (a_fNormalPressure < 0.333f)
		{
			m_eMS = EMSCreating;
			m_iActiveHandle = -1;
		}
		break;
	case EMSCreatingForwarding:
		if (m_sActiveShape.pTool == NULL)
		{
			m_eMS = EMSUnknown;
			break;
		}
		{
			// adding new shape
			HRESULT hRes;
			if (a_pPos)
			{
				TPixelCoords tPos = *a_pPos;
				m_sActiveShape.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, &tPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			}
			else
				hRes = m_sActiveShape.pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
			if (hRes&ETPAApply)
				ApplyChanges(TRUE, 0);
		}
		if (m_eAutoUnselect == EAUSensing)
		{
			if (a_fNormalPressure > 0.5f)
			{
				m_tAutoUnselect = *a_pPos;
				m_eAutoUnselect = EAUWatching;
			}
		}

		if (m_eAutoUnselect == EAUWatching)
		{
			if (a_pPos && m_tAutoUnselect.fX == a_pPos->fX && m_tAutoUnselect.fY == a_pPos->fY)
			{
				if (a_fNormalPressure < 0.5f && m_sActiveShape.pTool && m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL) == S_FALSE)
				{
					// clicked in one place and nothing was produced
					for (std::vector<ULONG>::const_reverse_iterator iO = m_cShapeOrder.rbegin(); iO != m_cShapeOrder.rend(); ++iO)
					{
						CShapes::const_iterator i = m_cShapes.find(*iO);
						if (i == m_cShapes.end())
							continue;
						TPixelCoords tPos = *a_pPos;
						i->second.pTool->AdjustCoordinates(a_eKeysState, &tPos, &tPointerSize, NULL, (M_HandleRadius()+0.5f)/M_ImageZoom());
						if (i->second.pTool && (i->second.pTool->PointTest(a_eKeysState, &tPos, FALSE, sqrtf(tPointerSize.fX*tPointerSize.fY))&ETPAHitMask) == ETPAHit)
						{
							CComPtr<ISharedState> pState;
							m_pImage->StatePack(1, &i->first, &pState);
							m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
							m_nHotShape = 0;
							InvalidateHelpLines(m_cHotHelpLines);
							m_cHotHelpLines.clear();

							CComPtr<ISharedStateToolMode> pTMN;
							RWCoCreateInstance(pTMN, __uuidof(SharedStateToolMode));
							pTMN->Set(CComBSTR(L"VECTORSELECT"), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
							m_pStateMgr->StateSet(m_bstrNewToolStateSync, pTMN);

							m_sActiveShape.pTool = NULL;
							SendDrawToolCmdLineUpdateLater();
							return;
						}
					}
					m_eAutoUnselect = EAUSensing;
				}
			}
			else
			{
				m_eAutoUnselect = EAUGone;
			}
		}
		if (a_fNormalPressure < 0.333f)
		{
			if (S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
			{
				ApplyChanges(FALSE, 0);
				m_eMS = EMSModifying;
			}
			else
			{
				m_eMS = EMSCreating;
			}
		}
		break;
	}

	CShapes::const_iterator iHot = m_cShapes.end();
	if (a_pPos && a_fNormalPressure < 0.333f && m_sActiveShape.pTool == NULL)
	{
		ULONG nPt = 0;
		if (!TestHandle(*a_pPos, &nPt))
		{
			iActiveShape = m_cShapes.find(m_nActiveShape);
			HRESULT hHit = iActiveShape != m_cShapes.end() ? iActiveShape->second.pTool->PointTest(a_eKeysState, a_pPos, TRUE, sqrtf(tPointerSize.fX*tPointerSize.fY)) : ETPAMissed|ETPATransform;
			if ((hHit&ETPAActionMask) != ETPACustomAction)
			{
				iHot = ShapeFromPoint(a_pPos, &tPointerSize);
			}
		}
	}
	ULONG nHotShape = iHot == m_cShapes.end() ? 0 : iHot->first;
	if (nHotShape != m_nHotShape)
	{
		InvalidateHelpLines(m_cHotHelpLines);
		m_cHotHelpLines.clear();
		if (nHotShape)
		{
			if (nHotShape == m_nActiveShape)
				m_cHotHelpLines = m_cActiveHelpLines;
			else
			{
				CComObjectStackEx<CControlLines> cLines;
				cLines.Init(m_cHotHelpLines, M_HandleRadius()/M_ImageZoom());
				iHot->second.pTool->GetControlLines(&cLines, ECLTSelection);
			}
			InvalidateHelpLines(m_cHotHelpLines);
		}
		m_nHotShape = nHotShape;
	}

	if (a_pPos)
		m_tLastPos = *a_pPos;

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

	ProcessUpdateWindow();
}

STDMETHODIMP CDesignerViewVectorImageEditor::OnDeactivate(BOOL a_bCancelChanges)
{
	try
	{
		if (!a_bCancelChanges)
		{
			ApplyChanges(FALSE, 0);
		}
		//Invalidate(FALSE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewVectorImageEditor::ApplyChanges()
{
	//if (m_sActiveShape.pTool)
	//{
	//	ApplyChanges(0);
	//	if (m_sActiveShape.pTool)
	//		m_sActiveShape.pTool->Reset();
	//}
	//else if (!m_cDirty.empty())
	{
		ApplyChanges(TRUE, 0);
	}
	return S_OK;
}

void CDesignerViewVectorImageEditor::InvalidateHandles(CHandles const& a_cHandles)
{
	if (m_hWnd == NULL)
		return;

	SIZE szZoomed = M_ZoomedSize();
	SIZE szOrig = M_ImageSize();
	float fZoomX = float(szZoomed.cx)/szOrig.cx;
	float fZoomY = float(szZoomed.cy)/szOrig.cy;

	CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
	for (CHandles::const_iterator i = a_cHandles.begin(); i != a_cHandles.end(); ++i)
	{
		TVector2f t = {i->first.fX, i->first.fY};
		if (pCustApply == NULL)
			t = TransformVector2(M_InputTransform(), t);
		TPixelCoords tPos = {t.x, t.y};
		ULONG nClass = i->second;
		LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
		LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
		RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
		RECT rcIntersection =
		{
			max(rcPt.left, 0),
			max(rcPt.top, 0),
			min(rcPt.right, M_WindowSize().cx),
			min(rcPt.bottom, M_WindowSize().cy),
		};
		if (rcIntersection.left >= rcIntersection.right || rcIntersection.top >= rcIntersection.bottom)
			continue;
		InvalidateRect(&rcIntersection);
	}
}

void CDesignerViewVectorImageEditor::InvalidateHelpLines(CHelpLines const& a_cHelpLines)
{
	if (m_hWnd == NULL)
		return;

	SIZE szZoomed = M_ZoomedSize();
	SIZE szOrig = M_ImageSize();
	float fZoomX = float(szZoomed.cx)/szOrig.cx;
	float fZoomY = float(szZoomed.cy)/szOrig.cy;

	float fHelpLinesXMin = 1e6f;
	float fHelpLinesYMin = 1e6f;
	float fHelpLinesXMax = -1e6f;
	float fHelpLinesYMax = -1e6f;
	CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
	for (CHelpLines::const_iterator i = a_cHelpLines.begin(); i != a_cHelpLines.end(); ++i)
	{
		TVector2f t = {i->fX, i->fY};
		if (pCustApply == NULL)
			t = TransformVector2(M_InputTransform(), t);
		if (fHelpLinesXMin > t.x) fHelpLinesXMin = t.x;
		if (fHelpLinesYMin > t.y) fHelpLinesYMin = t.y;
		if (fHelpLinesXMax < t.x) fHelpLinesXMax = t.x;
		if (fHelpLinesYMax < t.y) fHelpLinesYMax = t.y;
	}
	RECT rc =
	{
		fZoomX*fHelpLinesXMin+M_ImagePos().x-2,
		fZoomY*fHelpLinesYMin+M_ImagePos().y-2,
		fZoomX*fHelpLinesXMax+M_ImagePos().x+2,
		fZoomY*fHelpLinesYMax+M_ImagePos().y+2
	};
	if (rc.left < 0) rc.left = 0;
	if (rc.top < 0) rc.top = 0;
	if (rc.right > M_WindowSize().cx) rc.right = M_WindowSize().cx;
	if (rc.bottom > M_WindowSize().cy) rc.bottom = M_WindowSize().cy;
	if (rc.left < rc.right && rc.top < rc.bottom)
		InvalidateRect(&rc, FALSE);
}

STDMETHODIMP CDesignerViewVectorImageEditor::UndoName(ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (m_sActiveShape.pTool && m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL) == S_OK)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Cancel drawn shape[0405]Zrušit nakreslený tvar");
			return S_OK;
		}
		else if (!m_cDirty.empty())
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Cancel changes[0405]Zrušit změny");
			return S_OK;
		}
		return E_FAIL;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewVectorImageEditor::Undo()
{
	try
	{
		if (m_sActiveShape.pTool && S_OK == m_sActiveShape.pTool->IsDirty(NULL, NULL, NULL))
		{
			m_sActiveShape.pTool->Reset();
		}
		else
		{
			for (std::set<ULONG>::const_iterator i = m_cDirty.begin(); i != m_cDirty.end(); ++i)
			{
				CShapes::iterator iSh = m_cShapes.find(*i);
				if (iSh == m_cShapes.end())
					continue;
				RefreshShape(*i, iSh->second);
			}
			if (m_cDirty.size() == 1 && m_aSelected.size() == 1 && *m_cDirty.begin() == *m_aSelected.begin())
			{
				CShapes::iterator iSh = m_cShapes.find(*m_cDirty.begin());
				UpdateStates(iSh->second);
			}
			m_cDirty.clear();
			Invalidate(FALSE);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}

}

STDMETHODIMP CDesignerViewVectorImageEditor::Update(IDesignerStatusBar* a_pStatusBar)
{
	try
	{
		if (m_bShowCommandDesc)
		{
			a_pStatusBar->SimpleModeSet(m_bstrCommandDesc);
			return S_OK;
		}
		if (m_tSentPos.fX != m_tMousePos.fX || m_tMousePos.fY != m_tLastPos.fY)
		{
			m_tSentPos = m_tMousePos;
			OLECHAR sz[128] = L"";
			swprintf(sz, L"%g, %g", floorf(m_tSentPos.fX*10.0f+0.5f)*0.1f, floorf(m_tSentPos.fY*10.0f+0.5f)*0.1f);

			static HICON hIcon = NULL;//(HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_STATUS_COORDS), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR|LR_SHARED);
			a_pStatusBar->PaneSet(m_bstrCoordsPane, hIcon, CComBSTR(sz), 110, -100);
		}
		else
		{
			a_pStatusBar->PaneKeep(m_bstrCoordsPane);
		}
		CComQIPtr<IDesignerViewStatusBar> pToolSB;
		if (m_sActiveShape.pTool)
			pToolSB = m_sActiveShape.pTool;
		else if (m_nActiveShape)
		{
			CShapes::const_iterator i = m_cShapes.find(m_nActiveShape);
			if (i != m_cShapes.end())
				pToolSB = i->second.pTool;
		}
		if (pToolSB)
			pToolSB->Update(a_pStatusBar);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CDesignerViewVectorImageEditor::OnSetCursor(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (reinterpret_cast<HWND>(a_wParam) == m_hWnd && LOWORD(a_lParam) == HTCLIENT)
	{
		if (UpdateCursor())
			return TRUE;
	}
	a_bHandled = FALSE;
	return 0;
}

BOOL CDesignerViewVectorImageEditor::UpdateCursor()
{
	POINT tPt;
	GetCursorPos(&tPt);
	ScreenToClient(&tPt);

	EControlKeysState eState/* = m_eKeysState;
	if (m_eDragState == EDSNothing)
	{
		eState*/ = (GetAsyncKeyState(VK_CONTROL)&0x8000) ?
			((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) :
			((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
	//}
	HCURSOR hCur = NULL;
	TPixelCoords tPos;
	TPixelCoords tPointerSize;
	GetPixelFromPoint(tPt, &tPos, &tPointerSize, NULL, eState);

	CComQIPtr<IRasterImageEditToolCustomApply> pCustApply(m_sActiveShape.pTool);
	if (pCustApply == NULL)
	{
		TVector2f t = {tPos.fX, tPos.fY};
		TVector2f t2 = TransformVector2(M_InputInvTransform(), t);
		tPos.fX = t2.x;
		tPos.fY = t2.y;
	}

	ULONG n;
	if (TestHandle(tPos, &n))
	{
		SetCursor(m_hHandCursor);
		if (m_hLastCursor)
		{
			DestroyCursor(m_hLastCursor);
			m_hLastCursor = NULL;
		}
		return TRUE;
	}

	{
		bool bDestroy = true;
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			bDestroy = S_FALSE == m_sActiveShape.pTool->GetCursor(eState, &tPos, &hCur);
		}
		else if (m_nActiveShape)
		{
			CShapes::const_iterator i = m_cShapes.find(m_nActiveShape);
			if (i != m_cShapes.end())
			{
				HRESULT hRes = i->second.pTool->PointTest(eState, &tPos, FALSE, sqrtf(tPointerSize.fX*tPointerSize.fY));
				if ((hRes&ETPAHitMask) == ETPAHit || m_sActiveShape.pTool && (hRes&ETPAStartNew))
					bDestroy = S_FALSE == i->second.pTool->GetCursor(eState, &tPos, &hCur);
			}
		}
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

void CDesignerViewVectorImageEditor::UpdateStates(SShape const& a_sShape)
{
	CComPtr<ISharedStateToolMode> pOldTM;
	m_pStateMgr->StateGet(m_bstrToolStateSync, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pOldTM));
	CComPtr<ISharedStateToolMode> pTM;
	RWCoCreateInstance(pTM, __uuidof(SharedStateToolMode));
	static EBlendingMode const eBM = EBMDrawOver;
	// TODO
	if (S_OK == pTM->Set(a_sShape.bstrTool, &a_sShape.bFill, a_sShape.bstrFill, &eBM, &a_sShape.eRM, &a_sShape.eCM, &a_sShape.bOutline, &a_sShape.tColor, &a_sShape.fWidth, &a_sShape.fPos, &a_sShape.eJoins, pOldTM))
		m_pStateMgr->StateSet(m_bstrToolStateSync, pTM);

	if (a_sShape.pToolState)
	{
		CComBSTR bstr(m_bstrToolStateSync);
		bstr += a_sShape.bstrTool;
		CComPtr<ISharedState> pOldTool;
		m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldTool));
		if (pOldTool != a_sShape.pToolState)
			m_pStateMgr->StateSet(bstr, a_sShape.pToolState);
	}
	if (a_sShape.pFillState && a_sShape.bstrFill.Length())
	{
		CComBSTR bstr(m_bstrToolStateSync);
		bstr += a_sShape.bstrFill;
		CComPtr<ISharedState> pOldFill;
		m_pStateMgr->StateGet(bstr, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldFill));
		if (pOldFill != a_sShape.pFillState)
			m_pStateMgr->StateSet(bstr, a_sShape.pFillState);
	}
}

void CDesignerViewVectorImageEditor::RestoreToolStates()
{
	CComPtr<IConfig> pSubCfg;
	m_pConfig->SubConfigGet(CComBSTR(CFGID_RVIEDIT_EDITTOOLSTATES), &pSubCfg);
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
			CConfigValue cVal;
			pSubCfg->ItemValueGet(bstrID, &cVal);
			if (cVal.TypeGet() != ECVTString)
				continue;
			CLSID tClsID;
			if (!GUIDFromString(cVal.operator BSTR(), &tClsID))
				continue;
			CComBSTR bstrFullID(m_bstrToolStateSync);
			bstrFullID += bstrID;
			CComPtr<ISharedState> pStateTool;
			m_pStateMgr->StateGet(bstrFullID, __uuidof(ISharedState), reinterpret_cast<void**>(&pStateTool));
			CComBSTR bstrPrev;
			if (pStateTool.p)
				pStateTool->ToText(&bstrPrev);
			if (bstrPrev == cVal.operator BSTR()+36)
				continue;
			pStateTool = NULL;
			RWCoCreateInstance(pStateTool, tClsID);
			if (pStateTool.p == NULL)
				continue;
			if (SUCCEEDED(pStateTool->FromText(CComBSTR(cVal.operator BSTR()+36))))
				m_pStateMgr->StateSet(bstrFullID, pStateTool);
		}
	}
}

void CDesignerViewVectorImageEditor::SendDrawToolCmdLineUpdate()
{
	if (!m_bToolCmdLineUpdating && m_bstrToolCommandStateSync.m_str && m_bstrToolCommandStateSync[0])
	{
		CComBSTR bstrCmdLine;
		SShape const* pShape = NULL;
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			pShape = &m_sActiveShape;
		}
		else if (m_nActiveShape)
		{
			CShapes::const_iterator iSh = m_cShapes.find(m_nActiveShape);
			if (iSh != m_cShapes.end())
				pShape = &iSh->second;
		}

		CComQIPtr<IRasterImageEditToolScripting> pTool(pShape ? pShape->pTool.p : NULL);
		if (pTool)
			pTool->ToText(&bstrCmdLine);
		CComBSTR bstr(pShape == NULL ? L"VECTORSELECT" : pShape->bstrTool);
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

void CDesignerViewVectorImageEditor::SendDrawToolCmdLineUpdateLater()
{
	if (m_hWnd && !m_bToolCmdLineUpdating && m_bstrToolCommandStateSync.m_str && m_bstrToolCommandStateSync[0] && !m_bToolCmdLineUpdatePosted)
	{
		PostMessage(WM_RW_UPDATETOOLCMDLINE);
		m_bToolCmdLineUpdatePosted = true;
	}
}

LRESULT CDesignerViewVectorImageEditor::OnUpdateToolCmdLine(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_bToolCmdLineUpdatePosted = false;
	SendDrawToolCmdLineUpdate();
	return 0;
}

#include "../RWViewImageRaster/HandleCoordinates.h"
//#include <DocumentMenuCommandImpl.h>
//#include <InPlaceCalc.h>
//#include <SimpleLocalizedString.h>
//#include <PrintfLocalizedString.h>
//
////extern OLECHAR const HANDLECOORDINATES_NAME[] = L"[0409]Handle coordinates...[0405]Upravit souřadnice bodu...";
//extern OLECHAR const HANDLECOORDINATES_DESC[] = L"[0409]Manually set coordinates of the clicked control handle.[0405]Ručně nastavit souřadnice kliknutého kontrolního bodu.";
//
//class ATL_NO_VTABLE CManualHandleCoordinates :
//	public CDocumentMenuCommandMLImpl<CManualHandleCoordinates, NULL/*HANDLECOORDINATES_NAME*/, HANDLECOORDINATES_DESC, NULL, 0>
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
//		public Win32LangEx::CLangIndirectDialogImpl<CDlg>
//	{
//	public:
//		CDlg(LCID a_tLocaleID, TPixelCoords* a_pCoords) :
//			Win32LangEx::CLangIndirectDialogImpl<CDlg>(a_tLocaleID), m_pCoords(a_pCoords)
//		{
//		}
//
//		enum {IDC_MH_X = 300, IDC_MH_X_UPDOWN, IDC_MH_Y, IDC_MH_Y_UPDOWN, IDC_SEPLINE};
//
//		BEGIN_DIALOG_EX(0, 0, 220, 94, 0)
//			DIALOG_CAPTION(_T("[0409]Modify Handle Coordinates[0405]Upravit souřadnice vrcholu"))
//			DIALOG_FONT_AUTO()
//			DIALOG_STYLE(WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_SETFONT|DS_MODALFRAME|DS_FIXEDSYS|WS_POPUP|WS_CAPTION|WS_SYSMENU)
//			DIALOG_EXSTYLE(0)
//		END_DIALOG()
//
//		BEGIN_CONTROLS_MAP()
//			CONTROL_LTEXT(_T("[0409]&X coordinate:[0405]Souřadnice &X:"), IDC_STATIC, 7, 9, 51, 8, WS_VISIBLE, 0)
//			CONTROL_EDITTEXT(IDC_MH_X, 59, 7, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
//			CONTROL_CONTROL(_T(""), IDC_MH_X_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 97, 7, 10, 12, 0)
//			CONTROL_LTEXT(_T("[0409]&Y coordinate:[0405]Souřadnice &Y:"), IDC_STATIC, 113, 9, 51, 8, WS_VISIBLE, 0)
//			CONTROL_EDITTEXT(IDC_MH_Y, 165, 7, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
//			CONTROL_CONTROL(_T(""), IDC_MH_Y_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 203, 7, 10, 12, 0)
//			CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 7, 26, 206, 1, 0)
//			CONTROL_LTEXT(_T("[0409]Note: handle coordinates will be sent to the tool exactly as entered, regardless of current coordinate mode. In case of invalid values, the results can be unexpected.[0405]Poznámka: souřadnice bodu budou předány přímo kreslicímu nástoji bez jakýcholi zaokrouhlení nebo korekcí. V případě neplatných souřadnic není výsledek zaručen."), IDC_STATIC, 7, 34, 206, 32, WS_VISIBLE, 0)
//			CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 109, 73, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
//			CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDOK, 163, 73, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
//		END_CONTROLS_MAP()
//
//		BEGIN_MSG_MAP(CDlg)
//			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
//			COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
//			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
//			NOTIFY_HANDLER(IDC_MH_X_UPDOWN, UDN_DELTAPOS, OnUpDownChange)
//			NOTIFY_HANDLER(IDC_MH_Y_UPDOWN, UDN_DELTAPOS, OnUpDownChange)
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
//		LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& a_bHandled)
//		{
//			LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
//			CEdit wndEdit((HWND)::SendMessage(a_pNMHDR->hwndFrom, UDM_GETBUDDY, 0, 0));
//			if (wndEdit.m_hWnd == NULL)
//			{
//				a_bHandled = FALSE;
//				return 0;
//			}
//			int nTextLen = wndEdit.GetWindowTextLength();
//			if (nTextLen < 0)
//				nTextLen = 0;
//			CAutoVectorPtr<TCHAR> psz(new TCHAR[nTextLen+1]);
//			wndEdit.GetWindowText(psz, nTextLen+1);
//			psz[nTextLen] = _T('\0');
//			LPCTSTR p = psz;
//			double d = CInPlaceCalc::EvalExpression(psz, &p);
//			d -= pNMUD->iDelta;
//			TCHAR szTmp[32] = _T("");
//			_stprintf(szTmp, _T("%g"), d);
//			wndEdit.SetWindowText(szTmp);
//			return 0;
//		}
//
//	private:
//		TPixelCoords* const m_pCoords;
//	};
//
//	STDMETHOD(Name)(ILocalizedString** a_ppText)
//	{
//		try
//		{
//			*a_ppText = NULL;
//			TPixelCoords tPixel = {0, 0};
//			ULONG dummy;
//			m_pTool->GetControlPoint(m_nHandle, &tPixel, &dummy);
//			wchar_t sz[64];
//			swprintf(sz, L"%g, %g", int(tPixel.fX*100+0.5f)/100.0f, int(tPixel.fY*100+0.5f)/100.0f);
//			CComPtr<ILocalizedString> pNum;
//			pNum.Attach(new CSimpleLocalizedString(SysAllocString(sz)));
//			CComObject<CPrintfLocalizedString>* pPF = NULL;
//			CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
//			CComPtr<ILocalizedString> pOut = pPF;
//			CComPtr<ILocalizedString> pTempl;
//			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s...\t[%s]")));
//			pPF->Init(pTempl, CMultiLanguageString::GetAuto(L"[0409]Coordinates[0405]Souřadnice"), pNum);
//			*a_ppText = pOut.Detach();
//			return S_OK;
//		}
//		catch (...)
//		{
//			return a_ppText ? E_UNEXPECTED : E_POINTER;
//		}
//	}
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

LRESULT CDesignerViewVectorImageEditor::OnMenuSelect(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
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

void CDesignerViewVectorImageEditor::OnContextMenu(POINT const* a_pPoint)
{
	CComPtr<IEnumUnknownsInit> pOps;
	RWCoCreateInstance(pOps, __uuidof(EnumUnknowns));

	CShapes::const_iterator iFocused = m_cShapes.end();

	if (a_pPoint)
	{
		TPixelCoords tCoords = {0, 0};
		ULONG nHandle = 0xffffffff;
		TestHandle(*a_pPoint, &nHandle);
		GetPixelFromPoint(*a_pPoint, &tCoords, NULL, &nHandle);
		IRasterImageEditTool* pActiveTool = NULL;
		if (m_aSelected.empty() && m_sActiveShape.pTool)
		{
			pActiveTool = m_sActiveShape.pTool;
		}
		else if (m_nActiveShape)
		{
			CShapes::const_iterator i = m_cShapes.find(m_nActiveShape);
			if (i != m_cShapes.end())
				pActiveTool = i->second.pTool;
		}
		if (nHandle != 0xffffffff)
		{
			// TODO: manual handle coordinates
			CComObject<CManualHandleCoordinates<TEditToolCoordsHandler> >* p = NULL;
			CComObject<CManualHandleCoordinates<TEditToolCoordsHandler> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(TEditToolCoordsHandler(pActiveTool, nHandle));
			pOps->Insert(pTmp);
			CComPtr<IDocumentMenuCommand> pSep;
			RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
			pOps->Insert(pSep);
		}
		else
		{ // right-clicked a shape?
			TPixelCoords const tPointerSize = {float(M_ImageSize().cx)/float(M_ZoomedSize().cx), float(M_ImageSize().cy)/float(M_ZoomedSize().cy)};
			iFocused = ShapeFromPoint(&tCoords, &tPointerSize);
		}

		CComQIPtr<IRasterImageEditToolContextMenu> pToolMenu(pActiveTool);
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
		//else if (pActiveTool == NULL)
		//{
		//	pOps->Insert();
		//}
	}

	if (iFocused != m_cShapes.end())
	{
		CComPtr<ISharedState> pFocused;
		m_pImage->StatePack(1, &(iFocused->first), &pFocused);
		if (pFocused)
		{
			CComBSTR bstr;
			m_pImage->StatePrefix(&bstr);
			bstr += L"FOCUSEDSHAPE";
			m_pContext->SetFocusedState(bstr, pFocused);
		}
	}

	CConfigValue cOpID;
	CComPtr<IConfig> pOpCfg;
	CComBSTR bstrCONTEXTMENU(CFGID_RVIEDIT_CONTEXTMENU);
	m_pConfig->ItemValueGet(bstrCONTEXTMENU, &cOpID);
	m_pConfig->SubConfigGet(bstrCONTEXTMENU, &pOpCfg);
	CComPtr<IEnumUnknowns> pCtxOps;
	m_pCmdMgr->CommandsEnum(m_pCmdMgr, cOpID, pOpCfg, m_pContext, this, m_pDoc, &pCtxOps);
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
	m_pContext->SetFocusedState();
	m_cContextOps.clear();
}

void CDesignerViewVectorImageEditor::InsertMenuItems(IEnumUnknowns* a_pCmds, CContextOps& a_cContextOps, CMenuHandle a_cMenu, UINT* a_pnMenuID)
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

STDMETHODIMP CDesignerViewVectorImageEditor::ObjectName(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName)
{
	return m_pRichGUI ? m_pRichGUI->ClipboardName(static_cast<ERichGUIClipboardAction>(a_eAction), NULL, a_ppName) : E_NOTIMPL;
}

STDMETHODIMP CDesignerViewVectorImageEditor::ObjectIconID(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID)
{
	return m_pRichGUI ? m_pRichGUI->ClipboardIconID(static_cast<ERichGUIClipboardAction>(a_eAction), NULL, a_pIconID) : E_NOTIMPL;
	//try
	//{
	//	GUID const tID = {0x7fa8dc5f, 0x430a, 0x4381, {0xb1, 0xe0, 0xf0, 0x83, 0x2c, 0x6d, 0xe3, 0xb7}};
	//	*a_pIconID = tID;
	//	return S_OK;
	//}
	//catch (...)
	//{
	//	return a_pIconID ? E_UNEXPECTED : E_POINTER;
	//}
}

STDMETHODIMP CDesignerViewVectorImageEditor::ObjectIcon(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	return m_pRichGUI ? m_pRichGUI->ClipboardIcon(static_cast<ERichGUIClipboardAction>(a_eAction), NULL, a_nSize, a_phIcon, a_pOverlay) : E_NOTIMPL;
	//try
	//{
	//	if (a_phIcon) *a_phIcon = NULL;
	//	if (a_pOverlay) *a_pOverlay = TRUE;
	//	if (a_phIcon)
	//		*a_phIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_CLIPBOARDVECTOR), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
	//	return S_OK;
	//}
	//catch (...)
	//{
	//	return E_UNEXPECTED;
	//}
}

STDMETHODIMP CDesignerViewVectorImageEditor::Check(EDesignerViewClipboardAction a_eAction)
{
	if (m_pRichGUI == NULL)
		return E_NOTIMPL;
	CComPtr<ISharedState> pState;
	m_pStateMgr->StateGet(m_bstrSelectionStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
	return m_pRichGUI->ClipboardCheck(static_cast<ERichGUIClipboardAction>(a_eAction), m_hWnd, pState);
	//try
	//{
	//	switch (a_eAction)
	//	{
	//	case EDVCACut:
	//	case EDVCACopy:				return m_aSelected.empty() ? S_FALSE : S_OK;
	//	case EDVCAPaste:
	//		if (m_nClipboardFormat == 0)
	//			m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));
	//		return IsClipboardFormatAvailable(m_nClipboardFormat) ? S_OK : S_FALSE;
	//	case EDVCASelectAll:		return m_cShapes.size() <= m_aSelected.size() ? S_FALSE : S_OK;
	//	case EDVCAInvertSelection:	return m_cShapes.empty() ? S_FALSE : S_OK;
	//	}
	//	return E_NOTIMPL;
	//}
	//catch (...)
	//{
	//	return E_UNEXPECTED;
	//}
}

bool CopyObjects(IDocumentVectorImage* a_pDst, IDocumentVectorImage* a_pSrc, std::vector<ULONG>& a_aIDs)
{
	for (std::vector<ULONG>::iterator i = a_aIDs.begin(); i != a_aIDs.end(); ++i)
	{
		CComBSTR bstrName;
		a_pSrc->ObjectNameGet(*i, &bstrName);
		CComBSTR bstrToolID;
		CComBSTR bstrToolParams;
		a_pSrc->ObjectGet(*i, &bstrToolID, &bstrToolParams);
		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;
		a_pSrc->ObjectStyleGet(*i, &bstrStyleID, &bstrStyleParams);
		bool bEnabled = a_pSrc->ObjectIsEnabled(*i) != S_FALSE;
		TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
		ERasterizationMode eRM = ERMSmooth;
		ECoordinatesMode eCM = ECMFloatingPoint;
		BOOL bFill = TRUE;
		BOOL bOutline = FALSE;
		float fWidth = 1.0f;
		float fPos = 0.0f;
		EOutlineJoinType eJoins = EOJTRound;
		a_pSrc->ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
		ULONG nNew = 0;
		if (FAILED(a_pDst->ObjectSet(&nNew, bstrToolID, bstrToolParams)) || nNew == 0)
			return false;
		if (bstrName.m_str)
			a_pDst->ObjectNameSet(nNew, bstrName);
		a_pDst->ObjectStyleSet(nNew, bstrStyleID, bstrStyleParams);
		a_pDst->ObjectStateSet(nNew, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
		a_pDst->ObjectEnable(nNew, bEnabled);
		*i = nNew;
	}
	return true;
}

//class CClipboardHandler
//{
//public:
//	CClipboardHandler(HWND a_hWnd)
//	{
//		if (!::OpenClipboard(a_hWnd))
//			throw E_FAIL;
//	}
//	~CClipboardHandler()
//	{
//		::CloseClipboard();
//	}
//};

STDMETHODIMP CDesignerViewVectorImageEditor::Exec(EDesignerViewClipboardAction a_eAction)
{
	if (m_pRichGUI == NULL)
		return E_NOTIMPL;
	CComPtr<ISharedState> pState;
	m_pStateMgr->StateGet(m_bstrSelectionStateSync, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
	CComPtr<ISharedState> pNewState;
	CWriteLock<IDocument> cLock(m_pDoc);
	HRESULT hRes = m_pRichGUI->ClipboardRun(static_cast<ERichGUIClipboardAction>(a_eAction), m_hWnd, m_tLocaleID, pState, &pNewState);
	if (SUCCEEDED(hRes))
	{
		m_aSelected.clear();
		m_eMS = EMSUnknown;// EMSModifying
		CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync, pState);
		m_pStateMgr->StateSet(m_bstrSelectionStateSync, pNewState);
	}
	return hRes;

	//try
	//{
	//	switch (a_eAction)
	//	{
	//	case EDVCACut:
	//	case EDVCACopy:
	//		if (m_nClipboardFormat == 0)
	//			m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));
	//		if (!m_aSelected.empty())
	//		{
	//			CClipboardHandler cClipboard(m_hWnd);
	//			EmptyClipboard();

	//			CComPtr<IEnumUnknownsInit> pNewItems;
	//			RWCoCreateInstance(pNewItems, __uuidof(EnumUnknowns));

	//			CComPtr<IDocumentBase> pBase;
	//			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
	//			CComPtr<IDocumentFactoryVectorImage> pDFVI;
	//			RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryVectorImage));
	//			float a0[] = {0.0f, 0.0f, 0.0f, 0.0f};
	//			pDFVI->Create(NULL, pBase, CImageSize(256, 256), NULL, a0);
	//			std::set<ULONG> cSelected(m_aSelected.begin(), m_aSelected.end());
	//			for (std::vector<ULONG>::const_iterator i = m_cShapeOrder.begin(); i != m_cShapeOrder.end(); ++i)
	//			{
	//				if (cSelected.find(*i) == cSelected.end())
	//					continue;
	//				CComBSTR bstrName;
	//				m_pImage->ObjectNameGet(*i, &bstrName);
	//				CComBSTR bstrToolID;
	//				CComBSTR bstrToolParams;
	//				m_pImage->ObjectGet(*i, &bstrToolID, &bstrToolParams);
	//				CComBSTR bstrStyleID;
	//				CComBSTR bstrStyleParams;
	//				m_pImage->ObjectStyleGet(*i, &bstrStyleID, &bstrStyleParams);
	//				TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
	//				ERasterizationMode eRM = ERMSmooth;
	//				ECoordinatesMode eCM = ECMFloatingPoint;
	//				BOOL bFill = TRUE;
	//				BOOL bOutline = FALSE;
	//				float fWidth = 1.0f;
	//				float fPos = 0.0f;
	//				EOutlineJoinType eJoins = EOJTRound;
	//				m_pImage->ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
	//				boolean bEnabled = m_pImage->ObjectIsEnabled(*i) != S_FALSE;
	//				pDFVI->AddObject(NULL, pBase, bstrName, bstrToolID, bstrToolParams, bFill ? bstrStyleID.m_str : NULL, bFill ? bstrStyleParams.m_str : NULL, bOutline ? &fWidth : NULL, bOutline ? &fPos : NULL, bOutline ? &eJoins : NULL, bOutline ? &tColor : NULL, &eRM, &eCM, &bEnabled);
	//			}
	//			
	//			CComQIPtr<IDocument> pDoc2(pBase);
	//			CComPtr<IInputManager> pIM;
	//			RWCoCreateInstance(pIM, __uuidof(InputManager));
	//			CComBSTR bstrRVI(L"{51C87837-B028-4252-A3B3-940F80181770}");
	//			float const fWeight = 1.0f;
	//			GUID tID = GUID_NULL;
	//			CComPtr<IConfig> pCfg;
	//			pIM->FindBestEncoderEx(pDoc2, 1, &(bstrRVI.m_str), &fWeight, &tID, &pCfg);
	//			CComPtr<IDocumentEncoder> pEnc;
	//			if (IsEqualGUID(tID, GUID_NULL) || FAILED(RWCoCreateInstance(pEnc, tID)))
	//				return E_FAIL;
	//			CReturnedData cData;
	//			if (FAILED(pEnc->Serialize(pDoc2, pCfg, &cData, NULL, NULL)) || cData.size() == 0)
	//				return E_FAIL;

	//			HANDLE hMem = GlobalAlloc(GHND, cData.size());
	//			BYTE* pData = reinterpret_cast<BYTE*>(GlobalLock(hMem));
	//			CopyMemory(pData, cData.begin(), cData.size());
	//			GlobalUnlock(hMem);
	//			SetClipboardData(m_nClipboardFormat, hMem);
	//			if (a_eAction == EDVCACut)
	//			{
	//				CWriteLock<IDocument> cLock(m_pDoc);
	//				for (std::vector<ULONG>::const_iterator i = m_aSelected.begin(); i != m_aSelected.end(); ++i)
	//				{
	//					ULONG n = *i;
	//					m_pImage->ObjectSet(&n, NULL, NULL);
	//				}
	//				CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync);
	//				CComPtr<ISharedState> pState;
	//				m_pImage->StatePack(0, NULL, &pState);
	//				m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
	//			}
	//			return S_OK;
	//		}
	//		return E_FAIL;

	//	case EDVCAPaste:
	//		if (m_nClipboardFormat == 0)
	//			m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));
	//		if (IsClipboardFormatAvailable(m_nClipboardFormat))
	//		{
	//			CClipboardHandler cClipboard(m_hWnd);

	//			HANDLE hMem = GetClipboardData(m_nClipboardFormat);
	//			if (hMem == NULL)
	//				return E_FAIL;

	//			CComPtr<IDocumentBase> pDocBase;
	//			RWCoCreateInstance(pDocBase, __uuidof(DocumentBase));
	//			CComQIPtr<IDocument> pDoc;

	//			BYTE const* pData = reinterpret_cast<BYTE const*>(GlobalLock(hMem));
	//			SIZE_T nSize = GlobalSize(hMem);
	//			try
	//			{
	//				CComObject<CDocumentName>* pName = NULL;
	//				CComObject<CDocumentName>::CreateInstance(&pName);
	//				CComPtr<IStorageFilter> pName2 = pName;
	//				pName->Init(L"pasted.rvi");
	//				CComPtr<IInputManager> pIM;
	//				RWCoCreateInstance(pIM, __uuidof(InputManager));
	//				CComPtr<IDocumentBuilder> pBuilder;
	//				RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryVectorImage));
	//				if (SUCCEEDED(pIM->DocumentCreateDataEx(pBuilder, nSize, pData, pName2, NULL, pDocBase, NULL, NULL, NULL)))
	//					pDoc = pDocBase;
	//			}
	//			catch (...)
	//			{
	//			}
	//			GlobalUnlock(hMem);
	//			hMem = NULL;
	//			CComPtr<IDocumentVectorImage> pDVI;
	//			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
	//			if (pDVI)
	//			{
	//				std::vector<ULONG> aIDs;
	//				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aIDs));
	//				if (!aIDs.empty())
	//				{
	//					CWriteLock<IDocument> cLock(m_pDoc);
	//					if (CopyObjects(m_pImage, pDVI, aIDs))
	//					{
	//						CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync);
	//						CComPtr<ISharedState> pState;
	//						m_pImage->StatePack(aIDs.size(), &(aIDs[0]), &pState);
	//						m_aSelected.clear();
	//						m_eMS = EMSUnknown;// EMSModifying
	//						m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
	//					}
	//				}
	//			}
	//			return S_OK;
	//		}
	//		return E_FAIL;

	//	case EDVCASelectAll:
	//		{
	//			CComPtr<ISharedState> pState;
	//			m_pImage->StatePack(m_cShapeOrder.size(), m_cShapeOrder.size() ? &(m_cShapeOrder[0]) : NULL, &pState);
	//			m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
	//		}
	//		return S_OK;

	//	case EDVCAInvertSelection:
	//		{
	//			LONG nIDs = m_cShapeOrder.size()-m_aSelected.size();
	//			CAutoVectorPtr<ULONG> aIDs(new ULONG[m_cShapeOrder.size()]);
	//			if (nIDs > 0)
	//			{
	//				std::set<ULONG> cSel;
	//				std::copy(m_aSelected.begin(), m_aSelected.end(), std::inserter(cSel, cSel.end()));
	//				std::set<ULONG> cAll;
	//				std::copy(m_cShapeOrder.begin(), m_cShapeOrder.end(), std::inserter(cAll, cAll.end()));
	//				std::set_difference(cAll.begin(), cAll.end(), cSel.begin(), cSel.end(), aIDs.m_p);
	//			}
	//			CComPtr<ISharedState> pState;
	//			m_pImage->StatePack(nIDs, aIDs, &pState);
	//			m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
	//		}
	//		return S_OK;
	//	}
	//	return E_NOTIMPL;
	//}
	//catch (...)
	//{
	//	return E_UNEXPECTED;
	//}
}

#include "GestureOperationManager.h"

HRESULT CDesignerViewVectorImageEditor::GestureUndo(IConfig* UNREF(a_pCfg))
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

HRESULT CDesignerViewVectorImageEditor::GestureRedo(IConfig* UNREF(a_pCfg))
{
	CComQIPtr<IDocumentUndo> pUndo(m_pDoc);
	if (pUndo)
	{
		pUndo->Redo(1);
		return S_OK;
	}
	return S_FALSE;
}

HRESULT CDesignerViewVectorImageEditor::GestureApply(IConfig* UNREF(a_pCfg))
{
	return ApplyChanges(TRUE, 0);
}

HRESULT CDesignerViewVectorImageEditor::GestureAutoZoom(IConfig* UNREF(a_pCfg))
{
	EnableAutoZoom(TRUE);
	return S_OK;
}

void CDesignerViewVectorImageEditor::ProcessGesture(ULONG a_nPoints, POINT const* a_pPoints)
{
	CConfigValue cOpID;
	CComPtr<IConfig> pOpCfg;
	if (FAILED(m_pMGH->Recognize(a_nPoints, a_pPoints, m_pConfig, &cOpID, &pOpCfg)))
		return;

	CComObject<CGestureOperationManager>* pGestureOpMgr = NULL;
	CComObject<CGestureOperationManager>::CreateInstance(&pGestureOpMgr);
	CComPtr<IOperationManager> pGestureOperations = pGestureOpMgr;

	CComPtr<IDocument> pDoc;
	CComPtr<IOperationContext> pStatesTmp = m_pContext;
	CComPtr<IOperationManager> pOpMgr;
	RWCoCreateInstance(pOpMgr, __uuidof(OperationManager));
	pGestureOpMgr->Init(pOpMgr, this, true);

	m_pContext->ResetErrorMessage();
	HRESULT hRes = pGestureOpMgr->Activate(pGestureOpMgr, pDoc.p ? pDoc.p : m_pDoc.p, cOpID, pOpCfg, pStatesTmp, m_hWnd, m_tLocaleID);
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
			CComBSTR bstrTemplate;
			CMultiLanguageString::GetLocalized(L"[0409]The attempted operation failed with error code 0x%08x. Please verify that there is enough free memory and that the configuration of the operation is correct.[0405]Provedení operace se nezdařilo a byl vrácen chybový kód 0x%08x. Prosím ověřte, že je dostatek volné paměti a konfigurace operace je správná.", m_tLocaleID, &bstrTemplate);
			TCHAR szMsg[256];
			szMsg[255] = _T('\0');
			_sntprintf(szMsg, 255, bstrTemplate, hRes);
			::MessageBox(m_hWnd, szMsg, CW2T(bstrCaption), MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

STDMETHODIMP CDesignerViewVectorImageEditor::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
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
		pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentVectorImage), &pBuilders);
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
					if (a_ppFeedback) *a_ppFeedback = new CMultiLanguageString(L"[0409]Add shape to the canvas.[0405]Přidat tvar na plátno.");
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

STDMETHODIMP CDesignerViewVectorImageEditor::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
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

		CWriteLock<IBlockOperations> cLock(m_pDoc);
		ULONG nItems = 0;
		a_pFileNames->Size(&nItems);
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentVectorImage), &pBuilders);
		HRESULT hRes = S_FALSE;
		std::vector<ULONG> newSel;
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComBSTR bstr;
			a_pFileNames->Get(i, &bstr);

			CComPtr<IDocumentBase> pDocBase;
			RWCoCreateInstance(pDocBase, __uuidof(DocumentBase));
			CComQIPtr<IDocument> pDoc;

			if (SUCCEEDED(pIM->DocumentCreateData(pBuilders, CStorageFilter(bstr), NULL, pDocBase)))
				pDoc = pDocBase;

			CComPtr<IDocumentVectorImage> pDVI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI)
			{
				std::vector<ULONG> aIDs;
				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aIDs));
				if (!aIDs.empty())
				{
					CWriteLock<IDocument> cLock(m_pDoc);
					if (CopyObjects(m_pImage, pDVI, aIDs))
					{
						newSel.insert(newSel.end(), aIDs.begin(), aIDs.end());
						hRes = S_OK;
					}
				}
			}
		}
		if (!newSel.empty())
		{
			CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync);
			CComPtr<ISharedState> pState;
			m_pImage->StatePack(newSel.size(), &(newSel[0]), &pState);
			m_aSelected.clear();
			m_eMS = EMSUnknown;// EMSModifying
			m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

