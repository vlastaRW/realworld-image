// DocumentDecoderGIMP.h : Declaration of the CDocumentDecoderGIMP

#pragma once
#include "resource.h"       // main symbols

#include "RWCodecImageGIMP.h"


#include "DocumentEncoderGIMP.h"
#include <RWDocumentImageRaster.h>


// CDocumentDecoderGIMP

class ATL_NO_VTABLE CDocumentDecoderGIMP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderGIMP, &CLSID_DocumentDecoderGIMP>,
	public CDocumentDecoderImpl<CDocumentDecoderGIMP, CDocumentTypeCreatorGIMP, IDocumentFactoryLayeredImage>
{
public:
	CDocumentDecoderGIMP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderGIMP)

BEGIN_CATEGORY_MAP(CDocumentDecoderGIMP)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderGIMP)
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
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderGIMP), CDocumentDecoderGIMP)
