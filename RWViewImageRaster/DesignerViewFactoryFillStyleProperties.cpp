// DesignerViewFactoryFillStyleProperties.cpp : Implementation of CDesignerViewFactoryFillStyleProperties

#include "stdafx.h"
#include "DesignerViewFactoryFillStyleProperties.h"

#include "DesignerViewFillStyleProperties.h"
#include "ConfigIDsEditToolProperties.h"
#include <SharedStringTable.h>
#include "ConfigGUIEditToolProperties.h"


// CDesignerViewFactoryFillStyleProperties

STDMETHODIMP CDesignerViewFactoryFillStyleProperties::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_FILLPROPS_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryFillStyleProperties::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_DESC), CConfigValue(L"FILLSTYLE"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryEditToolProperties, CConfigGUIEditToolPropertiesDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryFillStyleProperties::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* UNREF(a_pDoc), RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		CComObject<CDesignerViewFillStyleProperties>* pWnd = NULL;
		CComObject<CDesignerViewFillStyleProperties>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pOut(pWnd);

		CConfigValue cSyncGroup;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), &cSyncGroup);
		pWnd->Init(a_pFrame, cSyncGroup.operator BSTR(), a_hParent, a_prcWindow, a_nStyle, a_tLocaleID);

		*a_ppDVWnd = pOut.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryFillStyleProperties::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* UNREF(a_pDocument), ICheckSuitabilityCallback* UNREF(a_pCallback))
{
	return S_OK;
}
