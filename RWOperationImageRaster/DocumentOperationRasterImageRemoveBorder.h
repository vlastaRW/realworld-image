// DocumentOperationRasterImageRemoveBorder.h : Declaration of the CDocumentOperationRasterImageRemoveBorder

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"


// CDocumentOperationRasterImageRemoveBorder

class ATL_NO_VTABLE CDocumentOperationRasterImageRemoveBorder :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageRemoveBorder, &CLSID_DocumentOperationRasterImageRemoveBorder>,
	public IDocumentOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImageRemoveBorder()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageRemoveBorder)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageRemoveBorder)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageRemoveBorder)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
END_COM_MAP()



	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageRemoveBorder), CDocumentOperationRasterImageRemoveBorder)
