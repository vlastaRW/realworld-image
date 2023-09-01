// DocumentOperationRasterImagePerspective.h : Declaration of the CDocumentOperationRasterImagePerspective

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"
#include <RWDocumentImageRaster.h>
#include <RWDrawing.h>
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>


// CDocumentOperationRasterImagePerspective

class ATL_NO_VTABLE CDocumentOperationRasterImagePerspective :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImagePerspective, &CLSID_DocumentOperationRasterImagePerspective>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public ICanvasInteractingOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImagePerspective()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImagePerspective)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImagePerspective)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageTransformation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImagePerspective)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(ICanvasInteractingOperation)
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
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetDefaultIcon(a_nSize);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	static HICON GetDefaultIcon(ULONG a_nSize);

	// ICanvasInteractingOperation
public:
	STDMETHOD(CreateWrapper)(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY, ICanvasInteractingWrapper** a_ppWrapper);

	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform);
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect);
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect);
	STDMETHOD(Process)(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix);
	STDMETHOD(AdjustTransform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f* a_pTransform);


private:
	static TMatrix3x3f GetMatrix(TImageSize a_tCanvas, IConfig* a_pConfig);
	static TMatrix3x3f GetMatrix(IDocumentImage* a_pImg, IConfig* a_pConfig);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImagePerspective), CDocumentOperationRasterImagePerspective)
