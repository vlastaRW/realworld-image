// DesignerViewFactoryRasterEdit.h : Declaration of the CDesignerViewFactoryRasterEdit

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"

#include <WeakSingleton.h>
#include "WinTabWrapper.h"


// CDesignerViewFactoryRasterEdit

class ATL_NO_VTABLE CDesignerViewFactoryRasterEdit : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryRasterEdit, &CLSID_DesignerViewFactoryRasterEdit>,
	public IDesignerViewFactory
{
public:
	CDesignerViewFactoryRasterEdit()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDesignerViewFactoryRasterEdit)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryRasterEdit)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryRasterEdit)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

private:
	CWinTabWrapper* GetWinTab();

private:
	CComPtr<CWinTabWrapper> m_pWinTab;
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryRasterEdit), CDesignerViewFactoryRasterEdit)
