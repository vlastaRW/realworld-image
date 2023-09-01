// DocumentEncoderAPNG.h : Declaration of the CDocumentEncoderAPNG

#pragma once
#include <RWInput.h>
#include "RWImageCodecPNG.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsAPNG[] = L"png|apng";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathTGA[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameAPNG[] = L"[0409]Animated PNG files[0405]Soubory animací PNG";
extern __declspec(selectany) OLECHAR const g_pszTypeNameAPNG[] = L"[0409]Animated PNG Image[0405]Animovaný obrázek PNG";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameAPNG, g_pszTypeNameAPNG, g_pszSupportedExtensionsAPNG, 0, NULL/*IDI_PNG_FILETYPE, g_pszShellIconPathPNG*/> CDocumentTypeCreatorAPNG;


// CDocumentEncoderAPNG

class ATL_NO_VTABLE CDocumentEncoderAPNG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderAPNG, &CLSID_DocumentEncoderAPNG>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderAPNG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderAPNG)

BEGIN_CATEGORY_MAP(CDocumentEncoderAPNG)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderAPNG)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
END_COM_MAP()


	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderAPNG), CDocumentEncoderAPNG)
