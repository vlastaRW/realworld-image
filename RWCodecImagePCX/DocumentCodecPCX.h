// DocumentCodecPCX.h : Declaration of the CDocumentCodecPCX

#pragma once
#include <RWDocumentImageRaster.h>


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsPCX[] = L"pcx";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathPCX[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNamePCX[] = L"[0409]PCX image files[0405]Soubory obrázků PCX";
extern __declspec(selectany) OLECHAR const g_pszTypeNamePCX[] = L"[0409]PCX Image[0405]Obrázek PCX";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNamePCX, g_pszTypeNamePCX, g_pszSupportedExtensionsPCX, 0, NULL/*IDI_DDS_FILETYPE, g_pszShellIconPathDDS*/> CDocumentTypeCreatorPCX;

extern __declspec(selectany) CLSID const CLSID_DocumentCodecPCX = { 0xfb085802, 0x48d9, 0x41f9, { 0x8c, 0x92, 0xec, 0xdb, 0xc4, 0x7d, 0x4f, 0xac } };
class DECLSPEC_UUID("FB085802-48D9-41F9-8C92-ECDBC47D4FAC") DocumentCodecPCX;


// CDocumentCodecPCX

class ATL_NO_VTABLE CDocumentCodecPCX :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCodecPCX, &CLSID_DocumentCodecPCX>,
	public CDocumentDecoderImpl<CDocumentCodecPCX, CDocumentTypeCreatorPCX, IDocumentFactoryRasterImage>
{
public:
	CDocumentCodecPCX()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCodecPCX)

BEGIN_CATEGORY_MAP(CDocumentCodecPCX)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCodecPCX)
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

public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentCodecPCX), CDocumentCodecPCX)
