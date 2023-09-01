// DocumentDecoderBMP.h : Declaration of the CDocumentDecoderBMP

#pragma once
#include "RWImageCodecBMP.h"
#include "DocumentEncoderBMP.h"


// CDocumentDecoderBMP

class ATL_NO_VTABLE CDocumentDecoderBMP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderBMP, &CLSID_DocumentDecoderBMP>,
	public CDocumentDecoderImpl<CDocumentDecoderBMP, CDocumentTypeCreatorBMP, IDocumentFactoryRasterImage>
{
public:
	CDocumentDecoderBMP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderBMP)

BEGIN_CATEGORY_MAP(CDocumentDecoderBMP)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderBMP)
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

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderBMP), CDocumentDecoderBMP)
