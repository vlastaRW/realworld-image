// DocumentOperationRasterImageLevels.h : Declaration of the CDocumentOperationRasterImageLevels

#pragma once
#include "resource.h"       // main symbols

#include "RWOperationImagePhoto.h"
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CDocumentOperationRasterImageLevels

class ATL_NO_VTABLE CDocumentOperationRasterImageLevels :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageLevels, &CLSID_DocumentOperationRasterImageLevels>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImageLevels()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageLevels)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageLevels)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageColorAdjustment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageLevels)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageLevels), CDocumentOperationRasterImageLevels)
