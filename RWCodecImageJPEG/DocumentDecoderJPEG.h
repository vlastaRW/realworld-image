// DocumentDecoderJPEG.h : Declaration of the CDocumentDecoderJPEG

#pragma once
#include "resource.h"       // main symbols

#include "DocumentEncoderJPEG.h"

extern __declspec(selectany) CLSID const CLSID_DocumentDecoderJPEG = { 0x1df031dc, 0x4c60, 0x45e1, { 0x96, 0x2e, 0x12, 0x7f, 0xe8, 0xc6, 0x72, 0xe8 } };
class DECLSPEC_UUID("1DF031DC-4C60-45E1-962E-127FE8C672E8") DocumentDecoderJPEG;


// CDocumentDecoderJPEG

class ATL_NO_VTABLE CDocumentDecoderJPEG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderJPEG, &CLSID_DocumentDecoderJPEG>,
	public CDocumentDecoderImpl<CDocumentDecoderJPEG, CDocumentTypeCreatorJPEG, IDocumentFactoryRasterImage>
{
public:
	CDocumentDecoderJPEG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderJPEG)

BEGIN_CATEGORY_MAP(CDocumentDecoderJPEG)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderJPEG)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderJPEG), CDocumentDecoderJPEG)
