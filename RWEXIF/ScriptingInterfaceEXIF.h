// ScriptingInterfaceEXIF.h : Declaration of the CScriptingInterfaceEXIF

#pragma once
#include "RWEXIF_i.h"


// CScriptingInterfaceEXIF

class ATL_NO_VTABLE CScriptingInterfaceEXIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CScriptingInterfaceEXIF, &CLSID_ScriptingInterfaceEXIF>,
	public IScriptingInterface
{
public:
	CScriptingInterfaceEXIF()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CScriptingInterfaceEXIF)

BEGIN_CATEGORY_MAP(CScriptingInterfaceEXIF)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CScriptingInterfaceEXIF)
	COM_INTERFACE_ENTRY(IScriptingInterface)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IScriptingInterface methods
public:
	STDMETHOD(GetGlobalObjects)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);

};

OBJECT_ENTRY_AUTO(__uuidof(ScriptingInterfaceEXIF), CScriptingInterfaceEXIF)
