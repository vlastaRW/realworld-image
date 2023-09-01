// DesignerViewFactoryAnimationFrames.h : Declaration of the CDesignerViewFactoryAnimationFrames

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"
#include <IconRenderer.h>
#include <RWConceptDesignerExtension.h>


// CDesignerViewFactoryAnimationFrames

class ATL_NO_VTABLE CDesignerViewFactoryAnimationFrames :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryAnimationFrames, &CLSID_DesignerViewFactoryAnimationFrames>,
	public IDesignerViewFactory,
	public IDesignerFrameIcons
{
public:
	CDesignerViewFactoryAnimationFrames()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryAnimationFrames)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryAnimationFrames)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryAnimationFrames)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp);
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs);
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon);

public:
	static bool GetFrameIconLayers(IStockIcons* pSI, CIconRendererReceiver& cRenderer, IRTarget const* target = NULL);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryAnimationFrames), CDesignerViewFactoryAnimationFrames)
