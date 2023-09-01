// DocumentOperationRasterImageResample.h : Declaration of the CDocumentOperationRasterImageResample

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"


// CDocumentOperationRasterImageResample

class ATL_NO_VTABLE CDocumentOperationRasterImageResample : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageResample, &CLSID_DocumentOperationRasterImageResample>,
	public IDocumentOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImageResample()
	{
	}

DECLARE_NO_REGISTRY(IDR_DOCUMENTOPERATIONRASTERIMAGERESAMPLE)
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageResample)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageResample)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageResample)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
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

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageResample), CDocumentOperationRasterImageResample)
