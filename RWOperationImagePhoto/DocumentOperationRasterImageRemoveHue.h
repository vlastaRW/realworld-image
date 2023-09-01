// DocumentOperationRasterImageRemoveHue.h : Declaration of the CDocumentOperationRasterImageRemoveHue

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImagePhoto.h"


// CDocumentOperationRasterImageRemoveHue

class ATL_NO_VTABLE CDocumentOperationRasterImageRemoveHue :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageRemoveHue, &CLSID_DocumentOperationRasterImageRemoveHue>,
	public IDocumentOperation
{
public:
	CDocumentOperationRasterImageRemoveHue()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageRemoveHue)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageRemoveHue)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageRemoveHue)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageRemoveHue), CDocumentOperationRasterImageRemoveHue)
