// DocumentOperationRasterImageVignetting.h : Declaration of the CDocumentOperationRasterImageVignetting

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImagePhoto.h"
#include <RWProcessingTags.h>


// CDocumentOperationRasterImageVignetting

class ATL_NO_VTABLE CDocumentOperationRasterImageVignetting :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageVignetting, &CLSID_DocumentOperationRasterImageVignetting>,
	public IDocumentOperation,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImageVignetting()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageVignetting)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageVignetting)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImagePhotofix)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageVignetting)
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
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageVignetting), CDocumentOperationRasterImageVignetting)
