// DesignerViewFactoryOutlineProperties.h : Declaration of the CDesignerViewFactoryOutlineProperties

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CDesignerViewFactoryOutlineProperties

class ATL_NO_VTABLE CDesignerViewFactoryOutlineProperties :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryOutlineProperties, &CLSID_DesignerViewFactoryOutlineProperties>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryOutlineProperties()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryOutlineProperties)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryOutlineProperties)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryOutlineProperties)
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

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryOutlineProperties), CDesignerViewFactoryOutlineProperties)
