// MenuCommandsFinishDrawing.h : Declaration of the CMenuCommandsFinishDrawing

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CMenuCommandsFinishDrawing

class ATL_NO_VTABLE CMenuCommandsFinishDrawing :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsFinishDrawing, &CLSID_MenuCommandsFinishDrawing>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsFinishDrawing()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsFinishDrawing)

BEGIN_CATEGORY_MAP(CMenuCommandsFinishDrawing)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsFinishDrawing)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuItem :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IRasterEditView* a_pView)
		{
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CDocumentMenuItem)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel);
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands);
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IRasterEditView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsFinishDrawing), CMenuCommandsFinishDrawing)
