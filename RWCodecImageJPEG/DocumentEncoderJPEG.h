// DocumentEncoderJPEG.h : Declaration of the CDocumentEncoderJPEG

#pragma once
#include "resource.h"       // main symbols

#include "RWImageCodecJPEG.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsJPEG[] = L"jpg|jpeg|jfif";
//extern __declspec(selectany) OLECHAR const g_pszShellIconPathJPEG[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameJPEG[] = L"[0409]JPEG image files[0405]Soubory obrázků JPEG";
extern __declspec(selectany) OLECHAR const g_pszTypeNameJPEG[] = L"[0409]JPEG Image[0405]Obrázek JPEG";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameJPEG, g_pszTypeNameJPEG, g_pszSupportedExtensionsJPEG, 0, NULL/*IDI_DDS_FILETYPE, g_pszShellIconPathDDS*/> CDocumentTypeCreatorJPEG;

extern __declspec(selectany) CLSID const CLSID_DocumentEncoderJPEG = { 0x35ae7667, 0x8041, 0x4f2f, { 0x98, 0x6b, 0x4f, 0x49, 0x45, 0xf8, 0x42, 0xf0 } };
class DECLSPEC_UUID("35AE7667-8041-4F2F-986B-4F4945F842F0") DocumentEncoderJPEG;


// CDocumentEncoderJPEG

class ATL_NO_VTABLE CDocumentEncoderJPEG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderJPEG, &CLSID_DocumentEncoderJPEG>,
	public IDocumentEncoder,
	public CConfigDescriptorImpl
{
public:
	CDocumentEncoderJPEG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderJPEG)

BEGIN_CATEGORY_MAP(CDocumentEncoderJPEG)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderJPEG)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
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

	static void SaveMetaData(jpeg_compress_struct* a_pCompress, IImageMetaData* a_pIMD, LONG a_nMode, ULONG a_nEXIF, BYTE const* a_pEXIF, ULONG a_nICC, BYTE const* a_pICC);
	static IConfig* Config();

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderJPEG), CDocumentEncoderJPEG)
