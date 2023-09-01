
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>

extern wchar_t const EFFECT_ENABLE_NAME[] = L"[0409]Use effect[0405]Použít efekt";
extern wchar_t const EFFECT_DISABLE_NAME[] = L"[0409]Skip effect[0405]Přeskočit efekt";
extern wchar_t const TOGGLE_DESC[] = L"[0409]Enable or disable the selected step of a layer style.[0405]Povolit nebo zakázat vybraný krok stylu vrstvy.";

class ATL_NO_VTABLE CMenuCommandsImageEffectToggle :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsImageEffectToggle>,
	//public CDesignerFrameIconsImpl<1, &g_tIconIDLayerAddRaster, &ARICONID>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsImageEffectToggle()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsImageEffectToggle)

BEGIN_CATEGORY_MAP(CMenuCommandsImageEffectToggle)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	//IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsImageEffectToggle)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	//COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Effect State[0405]Vrstvený obrázek - stav efektu");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return S_FALSE;
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		if (a_ppSubCommands == NULL)
			return E_POINTER;

		try
		{
			CComPtr<IDocumentImageLayerEffect> pEffect;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImageLayerEffect), reinterpret_cast<void**>(&pEffect));
			if (pEffect == NULL)
				return E_NOINTERFACE;

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			{
				CComObject<CEnable>* p = NULL;
				CComObject<CEnable>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pCmd = p;
				p->Init(a_pDocument, pEffect);
				pCmds->Insert(pCmd);
			}
			{
				CComObject<CDisable>* p = NULL;
				CComObject<CDisable>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pCmd = p;
				p->Init(a_pDocument, pEffect);
				pCmds->Insert(pCmd);
			}

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CEnable :
		public CDocumentMenuCommandMLImpl<CEnable, EFFECT_ENABLE_NAME, TOGGLE_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentImageLayerEffect* a_pEffect)
		{
			m_pDoc = a_pDoc;
			m_pEffect = a_pEffect;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				static GUID const tID = {0x207abc2c, 0xe453, 0x4d31, {0x86, 0x93, 0xf1, 0xd2, 0x49, 0x21, 0xc0, 0xa9}};
				*a_pIconID = tID;
				return S_OK;
			}
			catch (...)
			{
				return a_pIconID ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				static IRPathPoint const area[] =
				{
					{0, 56, 0, -13.2548, 0, 0},
					{24, 32, 0, 0, -13.2548, 0},
					{184, 32, 13.2548, 0, 0, 0},
					{208, 56, 0, 0, 0, -13.2548},
					{208, 216, 0, 13.2548, 0, 0},
					{184, 240, 0, 0, 13.2548, 0},
					{24, 240, -13.2548, 0, 0, 0},
					{0, 216, 0, 0, 0, 13.2548},
				};
				static IRPolyPoint const check[] =
				{
					{24, 112}, {76, 60}, {120, 104}, {204, 20}, {256, 72}, {120, 208}
				};
				static IRGridItem const gridX[] = { {0, 0}, {0, 208} };
				static IRGridItem const gridY[] = { {0, 32}, {0, 240} };
				static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
				static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 0, NULL, NULL};
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&canvas1, itemsof(area), area, pSI->GetMaterial(ESMAltBackground/*ESMScheme2Color2*/), IRTarget(0.875f));
				cRenderer(&canvas2, itemsof(check), check, pSI->GetMaterial(ESMScheme2Color1), IRTarget(0.875f));
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			BYTE b = 1;
			m_pEffect->Set(&b, NULL, NULL);
			return S_OK;
		}

		EMenuCommandState IntState()
		{
			BYTE b;
			m_pEffect->Get(&b, NULL, NULL);
			return b ? static_cast<EMenuCommandState>(EMCSShowButtonText|EMCSChecked) : EMCSShowButtonText;
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentImageLayerEffect> m_pEffect;
	};

	class ATL_NO_VTABLE CDisable :
		public CDocumentMenuCommandMLImpl<CDisable, EFFECT_DISABLE_NAME, TOGGLE_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentImageLayerEffect* a_pEffect)
		{
			m_pDoc = a_pDoc;
			m_pEffect = a_pEffect;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				static GUID const tID = {0x91e7fca8, 0x27ae, 0x45ae, {0xb8, 0xf6, 0x3e, 0xda, 0x62, 0x90, 0x79, 0x94}};
				*a_pIconID = tID;
				return S_OK;
			}
			catch (...)
			{
				return a_pIconID ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				static IRPathPoint const area[] =
				{
					{0, 56, 0, -13.2548, 0, 0},
					{24, 32, 0, 0, -13.2548, 0},
					{184, 32, 13.2548, 0, 0, 0},
					{208, 56, 0, 0, 0, -13.2548},
					{208, 216, 0, 13.2548, 0, 0},
					{184, 240, 0, 0, 13.2548, 0},
					{24, 240, -13.2548, 0, 0, 0},
					{0, 216, 0, 0, 0, 13.2548},
				};
				static IRGridItem const gridX[] = { {0, 0}, {0, 208} };
				static IRGridItem const gridY[] = { {0, 32}, {0, 240} };
				static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&canvas1, itemsof(area), area, pSI->GetMaterial(ESMAltBackground/*ESMScheme2Color2*/), IRTarget(0.875f));
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			BYTE b = 0;
			m_pEffect->Set(&b, NULL, NULL);
			return S_OK;
		}

		EMenuCommandState IntState()
		{
			BYTE b;
			m_pEffect->Get(&b, NULL, NULL);
			return b ? EMCSShowButtonText : static_cast<EMenuCommandState>(EMCSShowButtonText|EMCSChecked);
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentImageLayerEffect> m_pEffect;
	};
};

// {E4EFB597-A8CC-4e86-931F-5BDDF49F8F88}
static const GUID CLSID_MenuCommandsImageEffectToggle = {0xe4efb597, 0xa8cc, 0x4e86, {0x93, 0x1f, 0x5b, 0xdd, 0xf4, 0x9f, 0x8f, 0x88}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsImageEffectToggle, CMenuCommandsImageEffectToggle)
