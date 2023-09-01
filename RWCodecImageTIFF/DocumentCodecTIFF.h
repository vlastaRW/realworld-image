// DocumentCodecTIFF.h : Declaration of the CDocumentCodecTIFF

#pragma once
#include <RWImaging.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsTIFF[] = L"tif|tiff";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathTIFF[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameTIFF[] = L"[0409]TIFF image files[0405]Soubory obrázků TIFF";
extern __declspec(selectany) OLECHAR const g_pszTypeNameTIFF[] = L"[0409]TIFF Image[0405]Obrázek TIFF";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameTIFF, g_pszTypeNameTIFF, g_pszSupportedExtensionsTIFF, 0, NULL/*IDI_TIFF_FILETYPE, g_pszShellIconPathTIFF*/> CDocumentTypeCreatorTIFF;

extern __declspec(selectany) CLSID const CLSID_DocumentCodecTIFF = { 0x48b70a2b, 0x0998, 0x40e7, { 0x87, 0x0e, 0x21, 0x64, 0x69, 0xfb, 0x9f, 0x8c } };
class DECLSPEC_UUID("48B70A2B-0998-40E7-870E-216469FB9F8C") DocumentCodecTIFF;


// CDocumentCodecTIFF

class ATL_NO_VTABLE CDocumentCodecTIFF : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCodecTIFF, &CLSID_DocumentCodecTIFF>,
	public CDocumentDecoderImpl<CDocumentCodecTIFF, CDocumentTypeCreatorTIFF, IDocumentFactoryRasterImage, EDPAverage, IDocumentCodec>
{
public:
	CDocumentCodecTIFF()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCodecTIFF)

BEGIN_CATEGORY_MAP(CDocumentCodecTIFF)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCodecTIFF)
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
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg) { return E_NOTIMPL; }
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecTIFF), CDocumentCodecTIFF)
