// MenuCommandsHideHandles.h : Declaration of the CMenuCommandsHideHandles

#include "stdafx.h"
#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>


extern wchar_t const HIDEHANDLES_NAME[] = L"[0409]Hide handles[0405]Skrývat body";
extern wchar_t const HIDEHANDLES_DESC[] = L"[0409]Hide control handles when mouse leaves the canvas window.[0405]Skrýt kontrolní body, když myš opustí okno s plátnem.";

// CMenuCommandsHideHandles

class ATL_NO_VTABLE CMenuCommandsHideHandles :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsHideHandles>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsHideHandles()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsHideHandles)

BEGIN_CATEGORY_MAP(CMenuCommandsHideHandles)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsHideHandles)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL)
			return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Image Editor - Hide Handles[0405]Obrázkový editor - skrývat kontrolní body");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CDocumentMenuCommand>* p = NULL;
				CComObject<CDocumentMenuCommand>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CDocumentMenuCommandMLImpl<CDocumentMenuCommand, HIDEHANDLES_NAME, HIDEHANDLES_DESC, NULL, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			BYTE b = 0;
			return pRIG && pRIG->CanHideHandles() == S_OK && SUCCEEDED(pRIG->GetHideHandles(&b)) ? (b ? EMCSChecked : EMCSNormal) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IEnumUnknownsInit> p;
				RWCoCreateInstance(p, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
				CComPtr<IRasterEditView> pRIG;
				p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
				BYTE b = 0;
				return pRIG ? SUCCEEDED(pRIG->GetHideHandles(&b)) && SUCCEEDED(pRIG->SetHideHandles(!b)) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};
};

// {FDF2DA11-ADEA-4009-AC7D-E6E67179826D}
static const GUID CLSID_MenuCommandsHideHandles = {0xfdf2da11, 0xadea, 0x4009, {0xac, 0x7d, 0xe6, 0xe6, 0x71, 0x79, 0x82, 0x6d}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsHideHandles, CMenuCommandsHideHandles)
