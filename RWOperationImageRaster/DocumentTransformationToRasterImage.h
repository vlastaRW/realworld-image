// DocumentTransformationToRasterImage.h : Declaration of the CDocumentTransformationToRasterImage

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"


// CDocumentTransformationToRasterImage

class ATL_NO_VTABLE CDocumentTransformationToRasterImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentTransformationToRasterImage, &CLSID_DocumentTransformationToRasterImage>,
	public IDocumentTransformation
{
public:
	CDocumentTransformationToRasterImage()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentTransformationToRasterImage)

BEGIN_CATEGORY_MAP(CDocumentTransformationToRasterImage)
	IMPLEMENTED_CATEGORY(CATID_DocumentTransformation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentTransformationToRasterImage)
	COM_INTERFACE_ENTRY(IDocumentTransformation)
END_COM_MAP()


	// IDocumentTransformation methods
public:
	STDMETHOD(NameGet)(ITransformationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(ITransformationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(ITransformationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentTransformationToRasterImage), CDocumentTransformationToRasterImage)
