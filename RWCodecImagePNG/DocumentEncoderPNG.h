// DocumentEncoderPNG.h : Declaration of the CDocumentEncoderPNG

#pragma once
#include <RWInput.h>
#include "RWImageCodecPNG.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsPNG[] = L"png";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathTGA[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNamePNG[] = L"[0409]PNG image files[0405]Soubory obrázků PNG";
extern __declspec(selectany) OLECHAR const g_pszTypeNamePNG[] = L"[0409]PNG Image[0405]Obrázek PNG";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNamePNG, g_pszTypeNamePNG, g_pszSupportedExtensionsPNG, 0, NULL/*IDI_PNG_FILETYPE, g_pszShellIconPathPNG*/> CDocumentTypeCreatorPNG;


// CDocumentEncoderPNG

class ATL_NO_VTABLE CDocumentEncoderPNG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderPNG, &CLSID_DocumentEncoderPNG>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderPNG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderPNG)

BEGIN_CATEGORY_MAP(CDocumentEncoderPNG)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderPNG)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
END_COM_MAP()


	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	static IConfig* Config();
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderPNG), CDocumentEncoderPNG)
