// DocumentEncoderWebP.h : Declaration of the CDocumentEncoderWebP

#pragma once
#include "RWCodecImageWebP.h"
#include <RWDocumentImageRaster.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsWebP[] = L"webp";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathWebP[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameWebP[] = L"[0409]WebP image files[0405]Soubory obrázků WebP";
extern __declspec(selectany) OLECHAR const g_pszTypeNameWebP[] = L"[0409]WebP Image[0405]Obrázek WebP";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameWebP, g_pszTypeNameWebP, g_pszSupportedExtensionsWebP, 0, NULL/*IDI_WEBP_FILETYPE, g_pszShellIconPathWebP*/> CDocumentTypeCreatorWebP;


// CDocumentEncoderWebP

class ATL_NO_VTABLE CDocumentEncoderWebP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderWebP, &CLSID_DocumentEncoderWebP>,
	public IDocumentEncoder,
	public CConfigDescriptorImpl
{
public:
	CDocumentEncoderWebP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderWebP)

BEGIN_CATEGORY_MAP(CDocumentEncoderWebP)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderWebP)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
END_COM_MAP()


	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocumentType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderWebP), CDocumentEncoderWebP)
