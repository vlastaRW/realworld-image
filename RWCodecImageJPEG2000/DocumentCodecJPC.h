// DocumentCodecJPC.h : Declaration of the CDocumentCodecJPC

#pragma once
#include <RWInput.h>
#include <RWDocumentImageRaster.h>
#include <RWImageCodecJPEG2000.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsJPC[] = L"jpc";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathJPC[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameJPC[] = L"[[0409]JPEG2000 codestream files[0405]Soubory obrázků JPEG2000 (codestream)";
extern __declspec(selectany) OLECHAR const g_pszTypeNameJPC[] = L"[0409]JPEG2000 Image (codestream)[0405]Obrázek JPEG2000 (codestream)";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameJPC, g_pszTypeNameJPC, g_pszSupportedExtensionsJPC, 0, NULL/*IDI_JPC_FILETYPE, g_pszShellIconPathJPC*/> CDocumentTypeCreatorJPC;


// CDocumentCodecJPC

class ATL_NO_VTABLE CDocumentCodecJPC :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCodecJPC, &CLSID_DocumentCodecJPC>,
	public CDocumentDecoderImpl<CDocumentCodecJPC, CDocumentTypeCreatorJPC, IDocumentFactoryRasterImage, EDPAverage, IDocumentCodec>
{
public:
	CDocumentCodecJPC()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCodecJPC)

BEGIN_CATEGORY_MAP(CDocumentCodecJPC)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCodecJPC)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
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
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecJPC), CDocumentCodecJPC)
