// MenuCommandsFillStyle.h : Declaration of the CMenuCommandsFillStyle

#pragma once
#include "resource.h"       // main symbols
#include <WeakSingleton.h>
#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern __declspec(selectany) wchar_t const NAME_NOFILL[] = L"[0409]No fill[0405]Bez výplně";
extern __declspec(selectany) wchar_t const DESC_NOFILL[] = L"[0409]Do not fill the interior of the drawn shape.[0405]Nechat vnitřek nakresleného tvaru prázdný.";
extern __declspec(selectany) GUID const ICONID_NOPAINT = {0x3794256a, 0xc63, 0x4c6a, {0x9a, 0x88, 0xa1, 0xc5, 0x5b, 0xc, 0x27, 0xa3}};



// CMenuCommandsFillStyle

class ATL_NO_VTABLE CMenuCommandsFillStyle :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsFillStyle, &CLSID_MenuCommandsFillStyle>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsFillStyle()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsFillStyle)

BEGIN_CATEGORY_MAP(CMenuCommandsFillStyle)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsFillStyle)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuSwitchStyle :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand,
		public IEnumUnknowns,
		public ILocalizedString
	{
	public:
		void Init(IRasterImageFillStyleManager* a_pManager, IOperationContext* a_pStates, BSTR a_bstrSyncID, std::vector<CComBSTR> const& a_aIDs, size_t a_iActive, wchar_t a_cAccel, bool a_bForceEnabled)
		{
			m_pManager = a_pManager;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_aIDs = a_aIDs;
			m_iActive = a_iActive;
			m_cAccel = a_cAccel;
			m_bForceEnabled = a_bForceEnabled;
		}

	BEGIN_COM_MAP(CDocumentMenuSwitchStyle)
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
		CComPtr<IRasterImageFillStyleManager> m_pManager;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		std::vector<CComBSTR> m_aIDs;
		size_t m_iActive;
		wchar_t m_cAccel;
		bool m_bForceEnabled;
	};

	class ATL_NO_VTABLE CDocumentMenuSwitchSubStyle :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IRasterImageFillStyleManager* a_pManager, IOperationContext* a_pStates, BSTR a_bstrSyncID, LPCWSTR a_pszID, std::vector<CComBSTR> const& a_aIDs)
		{
			m_pManager = a_pManager;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bstrStyleID = a_pszID;
			m_aIDs = a_aIDs;
		}

	BEGIN_COM_MAP(CDocumentMenuSwitchSubStyle)
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
		CComPtr<IRasterImageFillStyleManager> m_pManager;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		CComBSTR m_bstrStyleID;
		std::vector<CComBSTR> m_aIDs;
	};

	class ATL_NO_VTABLE CDocumentMenuNoFillStyle :
		public CDocumentMenuCommandMLImpl<CDocumentMenuNoFillStyle, NAME_NOFILL, DESC_NOFILL, &ICONID_NOPAINT, IDI_NO_PAINT>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, bool a_bEnabled, bool a_bActive)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bActive = a_bActive;
			m_bEnabled = a_bEnabled;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState() { return m_bEnabled ? (m_bActive ? EMCSRadioChecked : EMCSRadio) : EMCSDisabledRadio; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		bool m_bActive;
		bool m_bEnabled;
	};

	IRasterImageFillStyleManager* M_Manager()
	{
		if (m_pManager)
			return m_pManager;
		ObjectLock cLock(this);
		if (m_pManager == NULL)
			RWCoCreateInstance(m_pManager, __uuidof(RasterImageFillStyleManager));
		return m_pManager;
	}

	IRasterImageEditToolsManager* M_ToolManager()
	{
		if (m_pToolManager)
			return m_pToolManager;
		ObjectLock cLock(this);
		if (m_pToolManager)
			return m_pToolManager;
		RWCoCreateInstance(m_pToolManager, __uuidof(RasterImageEditToolsManager));
		return m_pToolManager;
	}

private:
	CComPtr<IRasterImageFillStyleManager> m_pManager;
	CComPtr<IRasterImageEditToolsManager> m_pToolManager;
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsFillStyle), CMenuCommandsFillStyle)
