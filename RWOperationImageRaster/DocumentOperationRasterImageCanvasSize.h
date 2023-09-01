// DocumentOperationRasterImageCanvasSize.h : Declaration of the CDocumentOperationRasterImageCanvasSize

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"
#include <RWDocumentImageRaster.h>


// CDocumentOperationRasterImageCanvasSize

class ATL_NO_VTABLE CDocumentOperationRasterImageCanvasSize :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageCanvasSize, &CLSID_DocumentOperationRasterImageCanvasSize>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter
{
public:
	CDocumentOperationRasterImageCanvasSize()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageCanvasSize)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageCanvasSize)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageCanvasSize)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
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

	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageCanvasSize), CDocumentOperationRasterImageCanvasSize)
