// MenuCommandsSelectionDocument.h : Declaration of the CMenuCommandsSelectionDocument

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <DocumentMenuCommandImpl.h>


// CMenuCommandsSelectionDocument

class ATL_NO_VTABLE CMenuCommandsSelectionDocument :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsSelectionDocument, &CLSID_MenuCommandsSelectionDocument>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsSelectionDocument()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsSelectionDocument)

BEGIN_CATEGORY_MAP(CMenuCommandsSelectionDocument)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsSelectionDocument)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsSelectionDocument), CMenuCommandsSelectionDocument)
