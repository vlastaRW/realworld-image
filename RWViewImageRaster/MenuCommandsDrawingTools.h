// MenuCommandsDrawingTools.h : Declaration of the CMenuCommandsDrawingTools

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <RWProcessing.h>
#include <WeakSingleton.h>


// CMenuCommandsDrawingTools

class ATL_NO_VTABLE CMenuCommandsDrawingTools :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsDrawingTools, &CLSID_MenuCommandsDrawingTools>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsDrawingTools()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsDrawingTools)

BEGIN_CATEGORY_MAP(CMenuCommandsDrawingTools)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsDrawingTools)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuSwitchTool :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand,
		public IEnumUnknowns,
		public ILocalizedString
	{
	public:
		void Init(IRasterImageEditToolsManager* a_pManager, IOperationContext* a_pStates, BSTR a_bstrSyncID, std::vector<CComBSTR> const& a_aIDs, size_t a_iActive, wchar_t a_cAccel)
		{
			m_pManager = a_pManager;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_aIDs = a_aIDs;
			m_iActive = a_iActive;
			m_cAccel = a_cAccel;
		}

	BEGIN_COM_MAP(CDocumentMenuSwitchTool)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
		COM_INTERFACE_ENTRY(IEnumUnknowns)
		COM_INTERFACE_ENTRY(ILocalizedString)
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

		// IEnumUnknowns
	public:
		STDMETHOD(Size)(ULONG *a_pnSize);
		STDMETHOD(Get)(ULONG a_nIndex, REFIID a_iid, void** a_ppItem);
		STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems);

		// ILocalizedString
	public:
		STDMETHOD(Get)(BSTR* a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
		STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString);

	private:
		CComPtr<IRasterImageEditToolsManager> m_pManager;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		std::vector<CComBSTR> m_aIDs;
		size_t m_iActive;
		wchar_t m_cAccel; 
	};

	class ATL_NO_VTABLE CDocumentMenuSwitchSubTool :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IRasterImageEditToolsManager* a_pManager, IOperationContext* a_pStates, BSTR a_bstrSyncID, LPCWSTR a_pszID, std::vector<CComBSTR> const& a_aIDs)
		{
			m_pManager = a_pManager;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bstrToolID = a_pszID;
			m_aIDs = a_aIDs;
		}

	BEGIN_COM_MAP(CDocumentMenuSwitchSubTool)
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
		CComPtr<IRasterImageEditToolsManager> m_pManager;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		CComBSTR m_bstrToolID;
		std::vector<CComBSTR> m_aIDs;
	};

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

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsDrawingTools), CMenuCommandsDrawingTools)
