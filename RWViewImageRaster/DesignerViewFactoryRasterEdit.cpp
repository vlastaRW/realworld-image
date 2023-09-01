// DesignerViewFactoryRasterEdit.cpp : Implementation of CDesignerViewFactoryRasterEditEdit

#include "stdafx.h"
#include "DesignerViewFactoryRasterEdit.h"

#include "DesignerViewRasterEdit.h"
#include "ConfigIDsRasterEdit.h"
#include <SharedStringTable.h>
#include "ConfigGUIRasterEdit.h"


// CDesignerViewFactoryRasterEditEdit

STDMETHODIMP CDesignerViewFactoryRasterEdit::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_IMGEDIT_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include "GestureOperationManager.h"

STDMETHODIMP CDesignerViewFactoryRasterEdit::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CDesignerViewRasterEdit::InitPaintingConfig(pCfgInit);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_TOOLSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLSYNC_DESC), CConfigValue(L"RASTEREDITSTATE"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_FILLSTYLESYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_FILLSTYLESYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_FILLSTYLESYNC_DESC), CConfigValue(L"FILLSTYLE"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_VIEWPORTSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_VIEWPORTSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_VIEWPORTSYNC_DESC), CConfigValue(L"VIEWPORT"), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_PASTETOOL), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_PASTETOOL_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_PASTETOOL_DESC), CConfigValue(L"TRANSFORM"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_SELECTIONSUPPORT), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSUPPORT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSUPPORT_DESC), CConfigValue(true), NULL, 0, NULL);

		// context menu
		CComPtr<IMenuCommandsManager> pMenuCmds;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
		if (pMenuCmds == NULL)
		{
			RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
		}
		pMenuCmds->InsertIntoConfigAs(pMenuCmds, pCfgInit, CComBSTR(CFGID_2DEDIT_CONTEXTMENU), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_CONTEXTMENU_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_CONTEXTMENU_DESC), 0, NULL);

		// extra mouse gestures -> document operation
		CComPtr<IOperationManager> pOperations;
		a_pManager->QueryInterface(__uuidof(IOperationManager), reinterpret_cast<void**>(&pOperations));
		if (pOperations == NULL)
		{
			RWCoCreateInstance(pOperations, __uuidof(OperationManager));
		}
		CComObject<CGestureOperationManager>* pGestureOpMgr = NULL;
		CComObject<CGestureOperationManager>::CreateInstance(&pGestureOpMgr);
		CComPtr<IOperationManager> pGestureOperations = pGestureOpMgr;
		pGestureOpMgr->Init(pOperations, NULL);
		CComPtr<IMouseGesturesHelper> pMGH;
		RWCoCreateInstance(pMGH, __uuidof(MouseGesturesHelper));
		if (pMGH)
			pMGH->InitConfig(pGestureOperations, pCfgInit);

		// saved tool states
		CComPtr<ILocalizedStringInit> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		//CComPtr<IConfigWithDependencies> pCfgState;
		//RWCoCreateInstance(pCfgState, __uuidof(ConfigWithDependencies));
		//pCfgState->ItemInsSimple(CComBSTR(CFGID_2DEDITSTATE_ID), pDummy, pDummy, CConfigValue(GUID_NULL), NULL, 0, NULL);
		//pCfgState->ItemInsSimple(CComBSTR(CFGID_2DEDITSTATE_VAL), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);
		//pCfgState->Finalize(NULL);
		//CComPtr<ISubConfigVector> pCfgStates;
		//RWCoCreateInstance(pCfgStates, __uuidof(SubConfigVector));
		//pCfgStates->Init(FALSE, pCfgState);
		CComPtr<ISubConfig> pCfgToolStates;
		RWCoCreateInstance(pCfgToolStates, __uuidof(ConfigInMemory));
		CComPtr<ISubConfig> pCfgStyleStates;
		RWCoCreateInstance(pCfgStyleStates, __uuidof(ConfigInMemory));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_EDITTOOLSTATES), pDummy, pDummy, CConfigValue(false), pCfgToolStates, 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_FILLSTYLESTATES), pDummy, pDummy, CConfigValue(false), pCfgStyleStates, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_TOOLMODE), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_FILLSTYLE), pDummy, pDummy, CConfigValue(L""), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_2DEDIT_TOOLCMDLINE), CMultiLanguageString::GetAuto(L"[0409]Tool command sync ID[0405]Sync ID nástrojové řádky"), CMultiLanguageString::GetAuto(L"[0409]Identifier of drawing tool command line synchronization.[0405]Identifikátor pro synchronizaci nástrojové řádky"), CConfigValue(L"DRAWTOOLCMDLINE"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryRasterEdit, CConfigGUIRasterEditDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryRasterEdit::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		CComObject<CDesignerViewRasterEdit>* pWnd = NULL;
		CComObject<CDesignerViewRasterEdit>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pOut(pWnd);

		CComPtr<IDocumentRasterImage> pImage;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pImage));
		if (pImage == NULL)
			return E_NOINTERFACE;

		pWnd->Init(a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, GetWinTab(), a_pManager, pImage);

		*a_ppDVWnd = pOut.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryRasterEdit::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentRasterImage> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentRasterImage));
	else a_pCallback->Missing(__uuidof(IDocumentRasterImage));
	return S_OK;
}

CWinTabWrapper* CDesignerViewFactoryRasterEdit::GetWinTab()
{
	if (m_pWinTab)
		return m_pWinTab;
	ObjectLock cLock(this);
	if (m_pWinTab)
		return m_pWinTab;
	CComObject<CWinTabWrapper>* p = NULL;
	CComObject<CWinTabWrapper>::CreateInstance(&p);
	m_pWinTab = p;
	p->Init();
	return m_pWinTab;
}
