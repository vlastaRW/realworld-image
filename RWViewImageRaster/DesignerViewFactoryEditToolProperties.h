// DesignerViewFactoryEditToolProperties.h : Declaration of the CDesignerViewFactoryEditToolProperties

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"






// CDesignerViewFactoryEditToolProperties

class ATL_NO_VTABLE CDesignerViewFactoryEditToolProperties : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryEditToolProperties, &CLSID_DesignerViewFactoryEditToolProperties>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryEditToolProperties()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryEditToolProperties)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryEditToolProperties)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryEditToolProperties)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryEditToolProperties), CDesignerViewFactoryEditToolProperties)
