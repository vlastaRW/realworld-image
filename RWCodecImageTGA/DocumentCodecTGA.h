// DocumentEncoderTGA.h : Declaration of the CDocumentEncoderTGA

#pragma once

#include <RWInput.h>
#include <RWDocumentImageRaster.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsTGA[] = L"tga";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathTGA[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameTGA[] = L"[0409]TGA image files[0405]Soubory obrázků TGA";
extern __declspec(selectany) OLECHAR const g_pszTypeNameTGA[] = L"[0409]TGA Image[0405]Obrázek TGA";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameTGA, g_pszTypeNameTGA, g_pszSupportedExtensionsTGA, 0, NULL/*IDI_TGA_FILETYPE, g_pszShellIconPathTGA*/> CDocumentTypeCreatorTGA;

extern __declspec(selectany) CLSID const CLSID_DocumentEncoderTGA = { 0xba27ad75, 0x1400, 0x4363, { 0xba, 0x07, 0xcd, 0x3d, 0x7a, 0x0c, 0xa7, 0xca } };
class DECLSPEC_UUID("BA27AD75-1400-4363-BA07-CD3D7A0CA7CA") DocumentCodecTGA;


// CDocumentEncoderTGA

class ATL_NO_VTABLE CDocumentEncoderTGA :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderTGA, &CLSID_DocumentEncoderTGA>,
	public CDocumentDecoderImpl<CDocumentEncoderTGA, CDocumentTypeCreatorTGA, IDocumentFactoryRasterImage, EDPAverage, IDocumentCodec>
{
public:
	CDocumentEncoderTGA()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderTGA)

BEGIN_CATEGORY_MAP(CDocumentEncoderTGA)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderTGA)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecTGA), CDocumentEncoderTGA)
