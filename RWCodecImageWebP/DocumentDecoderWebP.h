// DocumentDecoderWebP.h : Declaration of the CDocumentDecoderWebP

#pragma once
#include "RWCodecImageWebP.h"
#include <RWDocumentImageRaster.h>



// CDocumentDecoderWebP

class ATL_NO_VTABLE CDocumentDecoderWebP :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderWebP, &CLSID_DocumentDecoderWebP>,
	public IDocumentDecoder
{
public:
	CDocumentDecoderWebP()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderWebP)

BEGIN_CATEGORY_MAP(CDocumentDecoderWebP)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderWebP)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
END_COM_MAP()


	// IDocumentDecoder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority);
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocumentType);
	STDMETHOD(IsCompatible)(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders);
	STDMETHOD(Parse)(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentDecoderWebP), CDocumentDecoderWebP)
