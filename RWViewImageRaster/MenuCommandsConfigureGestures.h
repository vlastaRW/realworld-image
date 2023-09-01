// MenuCommandsConfigureGestures.h : Declaration of the CMenuCommandsConfigureGestures

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>



// CMenuCommandsConfigureGestures

class ATL_NO_VTABLE CMenuCommandsConfigureGestures :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsConfigureGestures, &CLSID_MenuCommandsConfigureGestures>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsConfigureGestures()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsConfigureGestures)

BEGIN_CATEGORY_MAP(CMenuCommandsConfigureGestures)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsConfigureGestures)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);


private:
	class ATL_NO_VTABLE CDocumentMenuItem :
		public CDocumentMenuCommandImpl<CDocumentMenuItem, IDS_MENU_CONFIGUREGESTURES_NAME, IDS_MENU_CONFIGUREGESTURES_DESC, NULL, 0>
	{
	public:
		void Init(IRasterEditView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pView->ConfigureGestures(a_hParent, a_tLocaleID);
		}

	private:
		CComPtr<IRasterEditView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsConfigureGestures), CMenuCommandsConfigureGestures)
