// DocumentEncoderGIMP.h : Declaration of the CDocumentEncoderGIMP

#pragma once
#include "resource.h"       // main symbols

#include "RWCodecImageGIMP.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsGIMP[] = L"xcf";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathGIMP[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameGIMP[] = L"[0409]GIMP image files[0405]Soubory obrázků GIMPu";
extern __declspec(selectany) OLECHAR const g_pszTypeNameGIMP[] = L"[0409]GIMP Image[0405]Obrázek GIMPu";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameGIMP, g_pszTypeNameGIMP, g_pszSupportedExtensionsGIMP, 0, NULL/*IDI_DDS_FILETYPE, g_pszShellIconPathDDS*/> CDocumentTypeCreatorGIMP;


// CDocumentEncoderGIMP

class ATL_NO_VTABLE CDocumentEncoderGIMP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderGIMP, &CLSID_DocumentEncoderGIMP>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderGIMP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderGIMP)

BEGIN_CATEGORY_MAP(CDocumentEncoderGIMP)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderGIMP)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
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

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderGIMP), CDocumentEncoderGIMP)
