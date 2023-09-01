// DocumentEncoderWebPAnimation.h : Declaration of the CDocumentEncoderWebPAnimation

#pragma once
#include "RWCodecImageWebP.h"
#include <RWDocumentImageRaster.h>



extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsWebPAni[] = L"webp";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathWebP[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameWebPAni[] = L"[0409]Animated WebP files[0405]Soubory animací WebP";
extern __declspec(selectany) OLECHAR const g_pszTypeNameWebPAni[] = L"[0409]Animated WebP Image[0405]Animovaný obrázek WebP";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameWebPAni, g_pszTypeNameWebPAni, g_pszSupportedExtensionsWebPAni, 0, NULL/*IDI_WEBP_FILETYPE, g_pszShellIconPathWebP*/> CDocumentTypeCreatorWebPAni;


// CDocumentEncoderWebPAnimation

class ATL_NO_VTABLE CDocumentEncoderWebPAnimation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderWebPAnimation, &CLSID_DocumentEncoderWebPAnimation>,
	public IDocumentEncoder,
	public CConfigDescriptorImpl
{
public:
	CDocumentEncoderWebPAnimation()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderWebPAnimation)

BEGIN_CATEGORY_MAP(CDocumentEncoderWebPAnimation)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderWebPAnimation)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
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

	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderWebPAnimation), CDocumentEncoderWebPAnimation)
