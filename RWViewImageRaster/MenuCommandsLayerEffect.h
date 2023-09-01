// MenuCommandsLayerEffect.h : Declaration of the CMenuCommandsLayerEffect

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <DesignerFrameIconsImpl.h>

extern __declspec(selectany) UINT s_aLayerEffectIconResIDs = IDI_LAYER_OPERATION;


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern __declspec(selectany) GUID const g_tIconIDLayerOperation = {0xcab75d84, 0xa4fe, 0x42de, {0x86, 0x4, 0xe0, 0x96, 0xf4, 0xe7, 0x11, 0x13}};


// CMenuCommandsLayerEffect

class ATL_NO_VTABLE CMenuCommandsLayerEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerEffect, &CLSID_MenuCommandsLayerEffect>,
	public IDocumentMenuCommands,
	public CDesignerFrameIconsImpl<1, &CLSID_MenuCommandsLayerEffect, &s_aLayerEffectIconResIDs>
{
public:
	CMenuCommandsLayerEffect()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerEffect)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerEffect)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerEffect)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsLayerEffect), CMenuCommandsLayerEffect)
