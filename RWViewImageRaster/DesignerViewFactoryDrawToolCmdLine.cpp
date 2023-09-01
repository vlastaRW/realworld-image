// DesignerViewFactoryDrawToolCmdLine.cpp : Implementation of CDesignerViewFactoryDrawToolCmdLine

#include "stdafx.h"
#include "DesignerViewFactoryDrawToolCmdLine.h"

#include "DesignerViewDrawToolCmdLine.h"

#include "MultiLanguageString.h"


// CDesignerViewFactoryDrawToolCmdLine

STDMETHODIMP CDesignerViewFactoryDrawToolCmdLine::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Raster Editor - Drawing Tool Command Line[0405]Rastrový obrázek - příkazový řádek kreslicího nástroje");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDrawToolCmdLine::ConfigCreate(IViewManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CMDLINE_SYNCID), CMultiLanguageString::GetAuto(L"[0409]Tool command sync ID[0405]Sync ID nástrojové řádky"), CMultiLanguageString::GetAuto(L"[0409]Identifier of drawing tool command line synchronization.[0405]Identifikátor pro synchronizaci Sync ID nástrojové řádky"), CConfigValue(L"DRAWTOOLCMDLINE"), NULL, 0, NULL);

		CComBSTR cCFGID_CMDLINE_SYNTAX(CFGID_CMDLINE_SYNTAX);
		pCfgInit->ItemIns1ofN(cCFGID_CMDLINE_SYNTAX, CMultiLanguageString::GetAuto(L"[0409]Syntax[0405]Syntax"), CMultiLanguageString::GetAuto(L"[0409]Format of the drawing commands.[0405]Formát kreslících příkazů."), CConfigValue(CFGVAL_CLS_SIMPLE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CMDLINE_SYNTAX, CConfigValue(CFGVAL_CLS_SIMPLE), CMultiLanguageString::GetAuto(L"[0409]Simple[0405]Jednoduchá"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CMDLINE_SYNTAX, CConfigValue(CFGVAL_CLS_SCRIPT), CMultiLanguageString::GetAuto(L"[0409]JavaScript[0405]JavaScript"), 0, NULL);

		// finalize the initialization of the config
		//CConfigCustomGUI<&CLSID_DesignerViewFactoryRasterEdit, CConfigGUIRasterEditDlg>::FinalizeConfig(pCfgInit);
		pCfgInit->Finalize(NULL);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDrawToolCmdLine::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		CComObject<CDesignerViewDrawToolCmdLine>* pWnd = NULL;
		CComObject<CDesignerViewDrawToolCmdLine>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pOut(pWnd);

		pWnd->Init(a_pFrame, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID/*, a_pDoc*/);

		*a_ppDVWnd = pOut.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryDrawToolCmdLine::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentRasterImage> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentRasterImage));
	else a_pCallback->Missing(__uuidof(IDocumentRasterImage));
	return S_OK;
}

