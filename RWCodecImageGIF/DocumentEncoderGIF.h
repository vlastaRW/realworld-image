// DocumentEncoderGIF.h : Declaration of the CDocumentEncoderGIF

#pragma once
#include "RWImageCodecGIF.h"


// CDocumentEncoderGIF

class ATL_NO_VTABLE CDocumentEncoderGIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderGIF, &CLSID_DocumentEncoderGIF>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderGIF()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderGIF)

BEGIN_CATEGORY_MAP(CDocumentEncoderGIF)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderGIF)
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
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	static IConfig* Config();
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderGIF), CDocumentEncoderGIF)
