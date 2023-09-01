// MenuCommandsSwapColors.h : Declaration of the CMenuCommandsSwapColors

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>


extern __declspec(selectany) const GUID const tIconIDSwapColors = {0x92bf2f20, 0x78f3, 0x4cff, {0x84, 0x8e, 0xc4, 0x70, 0xc9, 0x39, 0x4e, 0x94}};

// CMenuCommandsSwapColors

class ATL_NO_VTABLE CMenuCommandsSwapColors :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsSwapColors, &CLSID_MenuCommandsSwapColors>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsSwapColors()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsSwapColors)

BEGIN_CATEGORY_MAP(CMenuCommandsSwapColors)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsSwapColors)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);


private:
	class ATL_NO_VTABLE CDocumentMenuItem :
		public CDocumentMenuCommandImpl<CDocumentMenuItem, IDS_MENU_SWAPCOLORS_NAME, IDS_MENU_SWAPCOLORS_DESC, &tIconIDSwapColors, IDI_MENU_SWAPCOLORS>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncColor1, BSTR a_bstrSyncColor2)
		{
			m_pStates = a_pStates;
			m_bstrSyncColor1 = a_bstrSyncColor1;
			m_bstrSyncColor2 = a_bstrSyncColor2;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				TColor tColor1 = {0.0f, 0.0f, 0.0f, 1.0f};
				TColor tColor2 = {0.0f, 0.0f, 0.0f, 0.0f};
				CComPtr<ISharedStateColor> pS1;
				m_pStates->StateGet(m_bstrSyncColor1, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pS1));
				if (pS1 == NULL)
				{
					RWCoCreateInstance(pS1, __uuidof(SharedStateColor));
					pS1->RGBASet(&tColor2.fR, NULL);
				}
				CComPtr<ISharedStateColor> pS2;
				m_pStates->StateGet(m_bstrSyncColor2, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pS2));
				if (pS2 == NULL)
				{
					RWCoCreateInstance(pS2, __uuidof(SharedStateColor));
					pS2->RGBASet(&tColor1.fR, NULL);
				}
				m_pStates->StateSet(m_bstrSyncColor1, pS2);
				m_pStates->StateSet(m_bstrSyncColor2, pS1);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncColor1;
		CComBSTR m_bstrSyncColor2;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsSwapColors), CMenuCommandsSwapColors)
