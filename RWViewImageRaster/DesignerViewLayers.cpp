
#include "stdafx.h"
/*#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <ContextHelpDlg.h>
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <math.h>
#include <XPGUI.h>
#include <RenderIcon.h>
#include <ContextMenuWithIcons.h>


// CDesignerViewFactoryLayerEffect

class ATL_NO_VTABLE CDesignerViewFactoryLayers :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryLayers>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryLayers()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryLayers)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryLayers)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryLayers)
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
			*a_ppName = new CMultiLanguageString(L"[0409]Layered Image - Layers[0405]Vrstvený obrázek - vrstvy");
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
			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"LAYER"), NULL, 0, NULL);
			CConfigCustomGUI<&CLSID_DesignerViewFactoryLayerProperties, CConfigGUILayerIDDlg>::FinalizeConfig(pCfgInit);

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
			CComObject<CDesignerViewLayers>* pWnd = NULL;
			CComObject<CDesignerViewLayers>::CreateInstance(&pWnd);
			CComPtr<IDesignerView> pOut(pWnd);

			CConfigValue cSyncGroup;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSyncGroup);
			pWnd->Init(a_pFrame, cSyncGroup.operator BSTR(), a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc);

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
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&p));
		if (p) a_pCallback->Used(__uuidof(IDocumentLayeredImage));
		else a_pCallback->Missing(__uuidof(IDocumentLayeredImage));
		return S_OK;
	}

};

// {2A4E5107-CAD9-4b4a-AA62-52008CE24729}
static const GUID CLSID_DesignerViewFactoryLayers = {0x2a4e5107, 0xcad9, 0x4b4a, {0xaa, 0x62, 0x52, 0x0, 0x8c, 0xe2, 0x47, 0x29}};

OBJECT_ENTRY_AUTO(CLSID_DesignerViewFactoryLayers, CDesignerViewFactoryLayers)
*/