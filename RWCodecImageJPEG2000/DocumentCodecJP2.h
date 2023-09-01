// DocumentCodecJP2.h : Declaration of the CDocumentCodecJP2

#pragma once
#include <RWInput.h>
#include <RWDocumentImageRaster.h>
#include <RWImageCodecJPEG2000.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsJP2[] = L"jp2";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathJP2[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameJP2[] = L"[[0409]JPEG2000 image files[0405]Soubory obrázků JPEG2000";
extern __declspec(selectany) OLECHAR const g_pszTypeNameJP2[] = L"[0409]JPEG2000 Image[0405]Obrázek JPEG2000";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameJP2, g_pszTypeNameJP2, g_pszSupportedExtensionsJP2, 0, NULL/*IDI_JP2_FILETYPE, g_pszShellIconPathJP2*/> CDocumentTypeCreatorJP2;


// CDocumentCodecJP2

class ATL_NO_VTABLE CDocumentCodecJP2 :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCodecJP2, &CLSID_DocumentCodecJP2>,
	public CDocumentDecoderImpl<CDocumentCodecJP2, CDocumentTypeCreatorJP2, IDocumentFactoryRasterImage, EDPAverage, IDocumentCodec>
{
public:
	CDocumentCodecJP2()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCodecJP2)

BEGIN_CATEGORY_MAP(CDocumentCodecJP2)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCodecJP2)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecJP2), CDocumentCodecJP2)
