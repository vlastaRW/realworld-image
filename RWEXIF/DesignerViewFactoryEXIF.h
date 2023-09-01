// DesignerViewFactoryEXIF.h : Declaration of the CDesignerViewFactoryEXIF

#pragma once
#include "RWEXIF_i.h"


// CDesignerViewFactoryEXIF

class ATL_NO_VTABLE CDesignerViewFactoryEXIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryEXIF, &CLSID_DesignerViewFactoryEXIF>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryEXIF()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryEXIF)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryEXIF)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryEXIF)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryEXIF), CDesignerViewFactoryEXIF)
