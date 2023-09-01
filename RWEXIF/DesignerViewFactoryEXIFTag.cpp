// DesignerViewFactoryEXIFTag.cpp : Implementation of CDesignerViewFactoryEXIFTag

#include "stdafx.h"
#include "DesignerViewFactoryEXIFTag.h"
#include <MultiLanguageString.h>
#include <RWImaging.h>
#include "ConfigGUISyncID.h"
#include "DesignerViewEXIFTag.h"


// CDesignerViewFactoryEXIFTag

STDMETHODIMP CDesignerViewFactoryEXIFTag::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	if (a_ppName == nullptr)
		return E_POINTER;
	try
	{
		*a_ppName = new CMultiLanguageString(L"[0409]Image - EXIF Tag Info[0405]Obrázek - EXIF popis značky");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryEXIFTag::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
		pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, CMultiLanguageString::GetAuto(L"[0409]Selection Synchronization ID[0405]Synchronizační ID výběru"), CMultiLanguageString::GetAuto(L"[0409]The selection in this view will be synchronized with other views with the same ID.[0405]Výběr položek v tomto pohledu bude synchronizován s výběrem v pohledech se shodným ID."), CConfigValue(L"EXIFTAG"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryEXIFTag::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComPtr<IImageMetaData> pIMD;
		a_pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD == NULL)
			return E_FAIL;

		CComObject<CDesignerViewEXIFTag>* pWnd = NULL;
		CComObject<CDesignerViewEXIFTag>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;

		pWnd->Init(a_pConfig, a_pFrame, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc, pIMD);

		*a_ppDVWnd = pDummy.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryEXIFTag::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IImageMetaData> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IImageMetaData));
	else a_pCallback->Missing(__uuidof(IImageMetaData));
	return S_OK;
}

