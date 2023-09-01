// DocumentDecoderRLI.h : Declaration of the CDocumentDecoderRLI

#pragma once
#include "resource.h"       // main symbols

#include "RWCodecImageRealWorld.h"


#include "DocumentEncoderRLI.h"


// CDocumentDecoderRLI

class ATL_NO_VTABLE CDocumentDecoderRLI :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderRLI, &CLSID_DocumentDecoderRLI>,
	public IDocumentDecoder
{
public:
	CDocumentDecoderRLI()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderRLI)

BEGIN_CATEGORY_MAP(CDocumentDecoderRLI)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderRLI)
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

	// IDocumentDecoder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority);
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocumentType);
	STDMETHOD(IsCompatible)(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders);
	STDMETHOD(Parse)(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderRLI), CDocumentDecoderRLI)
