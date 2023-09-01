// DocumentOperationRasterImageRenderFractal.h : Declaration of the CDocumentOperationRasterImageRenderFractal

#pragma once
#include "resource.h"       // main symbols

#include <AutoCategories.h>
#include <RWProcessing.h>
#include <DesignerFrameIconsImpl.h>
#include <RWDocumentImageRaster.h>


extern __declspec(selectany) UINT s_aFractalResID = IDI_FRACTAL;

extern __declspec(selectany) CLSID const CLSID_DocumentOperationRasterImageRenderFractal = { 0x55238c91, 0x45a3, 0x4d92, { 0x90, 0xe1, 0x3e, 0xc9, 0x7a, 0x82, 0x99, 0x2d } };
class DECLSPEC_UUID("55238C91-45A3-4D92-90E1-3EC97A82992D") DocumentOperationRasterImageRenderFractal;


// CDocumentOperationRasterImageRenderFractal

class ATL_NO_VTABLE CDocumentOperationRasterImageRenderFractal :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageRenderFractal, &CLSID_DocumentOperationRasterImageRenderFractal>,
	public IDocumentOperation,
	public IAutoOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter,
	public CDesignerFrameIconsImpl<1, &CLSID_DocumentOperationRasterImageRenderFractal, &s_aFractalResID>
{
public:
	CDocumentOperationRasterImageRenderFractal()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageRenderFractal)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageRenderFractal)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_AutoImageGenerator)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageRenderFractal)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IAutoOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
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

	// IAutoOperation methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(IconID)(GUID* a_pIconID);
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel);
	STDMETHOD(Configuration)(IConfig* a_pOperationCfg);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageRenderFractal), CDocumentOperationRasterImageRenderFractal)
