// DesignerViewFactoryVectorImageEditor.cpp : Implementation of CDesignerViewFactoryVectorImageEditor

#include "stdafx.h"
#include "DesignerViewFactoryVectorImageEditor.h"

#include "DesignerViewVectorImageEditor.h"
#include <MultiLanguageString.h>


// CDesignerViewFactoryVectorImageEditor

STDMETHODIMP CDesignerViewFactoryVectorImageEditor::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Vector Image - Editor[0405]Vektorový obrázek - editor");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include "GestureOperationManager.h"

STDMETHODIMP CDesignerViewFactoryVectorImageEditor::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CDesignerViewVectorImageEditor::InitPaintingConfig(pCfgInit);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_SELECTIONSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409][0405]"), CConfigValue(L"SHAPE"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_VIEWPORTSYNC), CMultiLanguageString::GetAuto(L"[0409]Viewport ID[0405]ID náhledu"), CMultiLanguageString::GetAuto(L"[0409][0405]"), CConfigValue(L"VIEWPORT"), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_TOOLSYNC), CMultiLanguageString::GetAuto(L"[0409]Edit tools synchronization[0405]Synchronizace editačních nástrojů"), CMultiLanguageString::GetAuto(L"[0409]The active tool will be synchronized with other views in the same group.[0405]Aktivní editovací nástroj bude synchronizován s ostaními náhledy ve stejné skupině."), CConfigValue(L"VECTOREDITSTATE"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_NEWTOOLSYNC), CMultiLanguageString::GetAuto(L"[0409]Edit tools synchronization[0405]Synchronizace editačních nástrojů"), CMultiLanguageString::GetAuto(L"[0409]The active tool will be synchronized with other views in the same group.[0405]Aktivní editovací nástroj bude synchronizován s ostaními náhledy ve stejné skupině."), CConfigValue(L"NEWSHAPETOOL"), NULL, 0, NULL);

		// context menu
		CComPtr<IMenuCommandsManager> pMenuCmds;
		a_pManager->QueryInterface(__uuidof(IMenuCommandsManager), reinterpret_cast<void**>(&pMenuCmds));
		if (pMenuCmds == NULL)
		{
			RWCoCreateInstance(pMenuCmds, __uuidof(MenuCommandsManager));
		}
		pMenuCmds->InsertIntoConfigAs(pMenuCmds, pCfgInit, CComBSTR(CFGID_RVIEDIT_CONTEXTMENU), CMultiLanguageString::GetAuto(L"[0409]Context menu[0405]Kontextové menu"), CMultiLanguageString::GetAuto(L"[0409][0405]"), 0, NULL);

		// saved tool states
		CComPtr<ISubConfig> pCfgToolStates;
		RWCoCreateInstance(pCfgToolStates, __uuidof(ConfigInMemory));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_EDITTOOLSTATES), NULL, NULL, CConfigValue(false), pCfgToolStates, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_TOOLMODE), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RVIEDIT_TOOLCMDLINE), CMultiLanguageString::GetAuto(L"[0409]Tool command sync ID[0405]Sync ID nástrojové řádky"), CMultiLanguageString::GetAuto(L"[0409]Identifier of drawing tool command line synchronization.[0405]Identifikátor pro synchronizaci nástrojové řádky"), CConfigValue(L"VECTORCMDLINE"), NULL, 0, NULL);

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

		// finalize the initialization of the config
		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DesignerViewFactoryRasterEdit, CConfigGUIRasterEditDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryVectorImageEditor::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		CComObject<CDesignerViewVectorImageEditor>* pWnd = NULL;
		CComObject<CDesignerViewVectorImageEditor>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pOut(pWnd);

		CComPtr<IDocumentVectorImage> pImage;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pImage));
		if (pImage == NULL)
			return E_NOINTERFACE;

		pWnd->Init(a_pFrame, a_pStatusBar, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, a_pManager, pImage);

		*a_ppDVWnd = pOut.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryVectorImageEditor::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentVectorImage> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentVectorImage));
	else a_pCallback->Missing(__uuidof(IDocumentVectorImage));
	return S_OK;
}

STDMETHODIMP CDesignerViewFactoryVectorImageEditor::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		if (wcscmp(a_bstrID, L"VECTORSELECT") == 0)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			static IRPathPoint const shape[] =
			{
				{89, 23, 0, 0, -6.39764, -6.39764},
				{214, 148, 7.13768, 7.13771, 0, 0},
				{218, 177, -4.05377, 7.62313, 4.76703, -8.96443},
				{197, 190, 0, 0, 9.19985, -3.35491e-005},
				{133, 190, 0, 0, 0, 0},
				{91, 231, -6.67555, 9.00403, 0, 0},
				{61, 239, -8.73758, -4.25113, 9.80348, 4.76973},
				{48, 213, 0, 0, -1.46836, 9.92429},
				{48, 40, 0, 0, 0, 0},
				{48, 40, 0.154751, -8.67232, 0, 0},
				{62, 18, 8.46672, -3.74325, -7.62456, 3.37091},
			};
			static IRPathPoint const hole[] =
			{
				{96, 98, 0, 0, 0, 0},
				{140, 142, 0, 0, 0, 0},
				{127, 142, -8.2843, 3.05176e-005, 0, 0},
				{104, 150, 0, 0, 4.41825, -4.41826},
				{96, 159, 0, 0, 0, 0},
			};
			static IRPath const arrow[] = { {itemsof(shape), shape}, {itemsof(hole), hole} };
			static IRGridItem gridX[] = { {0, 48}, };
			static IRCanvas canvas = {0, 0, 256, 256, itemsof(gridX), 0, gridX, NULL};
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMScheme2Color1));
			*a_phIcon = cRenderer.get();
			return S_OK;
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

