// DocumentOperationSeamCarving.h : Declaration of the CDocumentOperationSeamCarving

#pragma once
#include "resource.h"       // main symbols

#include "RWOperationImagePhoto.h"
#include "RWProcessingTags.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CDocumentOperationSeamCarving

class ATL_NO_VTABLE CDocumentOperationSeamCarving :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSeamCarving, &CLSID_DocumentOperationSeamCarving>,
	public IDocumentOperation
{
public:
	CDocumentOperationSeamCarving()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationSeamCarving)

BEGIN_CATEGORY_MAP(CDocumentOperationSeamCarving)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSeamCarving)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationSeamCarving), CDocumentOperationSeamCarving)
