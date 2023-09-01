// DocumentOperationMoveShape.h : Declaration of the CDocumentOperationMoveShape

#pragma once
#include <RWProcessing.h>


extern __declspec(selectany) GUID const CLSID_DocumentOperationMoveShape = { 0xf9b2ba4a, 0x5561, 0x4637, { 0x98, 0x80, 0x12, 0x0f, 0x4e, 0xfd, 0x29, 0x0a } };
class DECLSPEC_UUID("F9B2BA4A-5561-4637-9880-120F4EFD290A") DocumentOperationMoveShape;


// CDocumentOperationMoveShape

class ATL_NO_VTABLE CDocumentOperationMoveShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationMoveShape, &CLSID_DocumentOperationMoveShape>,
	public IDocumentOperation
{
public:
	CDocumentOperationMoveShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationMoveShape)

BEGIN_CATEGORY_MAP(CDocumentOperationMoveShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationMoveShape)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationMoveShape), CDocumentOperationMoveShape)
