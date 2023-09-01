// DocumentOperationInsertFrame.h : Declaration of the CDocumentOperationInsertFrame

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"


// CDocumentOperationInsertFrame

class ATL_NO_VTABLE CDocumentOperationInsertFrame :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationInsertFrame, &CLSID_DocumentOperationInsertFrame>,
	public IDocumentOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationInsertFrame()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationInsertFrame)

BEGIN_CATEGORY_MAP(CDocumentOperationInsertFrame)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationInsertFrame)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationInsertFrame), CDocumentOperationInsertFrame)
