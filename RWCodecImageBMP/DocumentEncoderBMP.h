// DocumentEncoderBMP.h : Declaration of the CDocumentEncoderBMP

#pragma once
#include "RWImageCodecBMP.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsBMP[] = L"bmp";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathBMP[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameBMP[] = L"[0409]BMP image files[0405]Soubory obrázků BMP";
extern __declspec(selectany) OLECHAR const g_pszTypeNameBMP[] = L"[0409]BMP Image[0405]Obrázek BMP";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameBMP, g_pszTypeNameBMP, g_pszSupportedExtensionsBMP, 0, NULL/*IDI_DDS_FILETYPE, g_pszShellIconPathDDS*/> CDocumentTypeCreatorBMP;


// CDocumentEncoderBMP

class ATL_NO_VTABLE CDocumentEncoderBMP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderBMP, &CLSID_DocumentEncoderBMP>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderBMP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderBMP)

BEGIN_CATEGORY_MAP(CDocumentEncoderBMP)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderBMP)
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

	static IConfig* Config();
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderBMP), CDocumentEncoderBMP)
