// ScriptingInterfaceAnimation.h : Declaration of the CScriptingInterfaceAnimation

#pragma once
#include "resource.h"       // main symbols

#include "RWDocumentAnimation.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CScriptingInterfaceAnimation

class ATL_NO_VTABLE CScriptingInterfaceAnimation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CScriptingInterfaceAnimation, &CLSID_ScriptingInterfaceAnimation>,
	public IScriptingInterface
{
public:
	CScriptingInterfaceAnimation()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CScriptingInterfaceAnimation)

BEGIN_CATEGORY_MAP(CScriptingInterfaceAnimation)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CScriptingInterfaceAnimation)
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

OBJECT_ENTRY_AUTO(__uuidof(ScriptingInterfaceAnimation), CScriptingInterfaceAnimation)
