// DissolveFilter.h : Declaration of the CDissolveFilter

#pragma once
#include <RWBase.h>
#include <RWConfig.h>
#include <RWProcessing.h>
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>
#include <AutoCategories.h>


class DECLSPEC_UUID("72AA8CF7-FBC1-400D-8B49-1FDA78D7B195")
DissolveFilter;

// CDissolveFilter

class ATL_NO_VTABLE CDissolveFilter :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDissolveFilter>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public IAutoOperation
{
public:
	CDissolveFilter()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDissolveFilter)

BEGIN_CATEGORY_MAP(CMenuCommandsAutoOperation)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_AutoImageEffect)
	IMPLEMENTED_CATEGORY(CATID_TagImageColorAdjustment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDissolveFilter)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IAutoOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pContext);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pContext, RWHWND a_hParent, LCID a_tLocaleID);

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

OBJECT_ENTRY_AUTO(__uuidof(DissolveFilter), CDissolveFilter)
