// DocumentDecoderPNG.h : Declaration of the CDocumentDecoderPNG

#pragma once
#include "RWImageCodecPNG.h"
#include "DocumentEncoderPNG.h"


// CDocumentDecoderPNG

class ATL_NO_VTABLE CDocumentDecoderPNG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderPNG, &CLSID_DocumentDecoderPNG>,
	public IDocumentDecoder
{
public:
	CDocumentDecoderPNG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderPNG)

BEGIN_CATEGORY_MAP(CDocumentDecoderPNG)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderPNG)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
END_COM_MAP()


public:
	STDMETHOD(Priority)(ULONG* a_pnPriority);
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocumentType);
	STDMETHOD(IsCompatible)(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders);
	STDMETHOD(Parse)(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderPNG), CDocumentDecoderPNG)
