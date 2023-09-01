// MenuCommandsCustomTools.h : Declaration of the CMenuCommandsCustomTools

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <WeakSingleton.h>



// CMenuCommandsCustomTools

class ATL_NO_VTABLE CMenuCommandsCustomTools :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsCustomTools, &CLSID_MenuCommandsCustomTools>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsCustomTools()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsCustomTools)

BEGIN_CATEGORY_MAP(CMenuCommandsCustomTools)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsCustomTools)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	IRasterImageEditToolsManager* M_Manager()
	{
		if (m_pManager)
			return m_pManager;
		ObjectLock cLock(this);
		if (m_pManager == NULL)
			RWCoCreateInstance(m_pManager, __uuidof(RasterImageEditToolsManager));
		return m_pManager;
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsCustomTools), CMenuCommandsCustomTools)
