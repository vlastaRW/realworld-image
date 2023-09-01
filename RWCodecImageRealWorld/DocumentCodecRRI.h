// DocumentCodecRRI.h : Declaration of the CDocumentCodecRRI

#pragma once
#include "resource.h"       // main symbols

#include "RWCodecImageRealWorld.h"
#include <RWDocumentImageRaster.h>

extern __declspec(selectany) wchar_t const g_pszFormatNameRRI[] = L"[0409]RealWorld raster image files[0405]Soubory rastrových obrázků RealWorldu";
extern __declspec(selectany) wchar_t const g_pszTypeNameRRI[] = L"[0409]RealWorld Raster Image[0405]Rastrový obrázek RealWorldu";
//extern __declspec(selectany) wchar_t const g_pszShellIconPathRRI[] = L"%MODULE%,0";
extern __declspec(selectany) wchar_t const g_pszSupportedExtensionsRRI[] = L"rri";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameRRI, g_pszTypeNameRRI, g_pszSupportedExtensionsRRI, 0, NULL/*IDI_RASTERIMAGE, g_pszShellIconPathRVI*/> CDocumentTypeCreatorRRI;



// CDocumentCodecRRI

class ATL_NO_VTABLE CDocumentCodecRRI :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCodecRRI, &CLSID_DocumentCodecRRI>,
	public CDocumentDecoderImpl<CDocumentCodecRRI, CDocumentTypeCreatorRRI, IDocumentFactoryRasterImage, EDPAverage, IDocumentCodec>
{
public:
	CDocumentCodecRRI()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCodecRRI)

BEGIN_CATEGORY_MAP(CDocumentCodecRRI)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCodecRRI)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
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
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg) { return E_NOTIMPL; }
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecRRI), CDocumentCodecRRI)
