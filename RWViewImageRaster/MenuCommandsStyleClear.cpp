
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include "ConfigGUILayerID.h"
#include <SharedStateUndo.h>

extern OLECHAR const CMDNAME_LAYEREFFECTCLEAR[] = L"[0409]Remove style[0405]Odstranit styl";
extern OLECHAR const CMDDESC_LAYEREFFECTCLEAR[] = L"[0409]Remove the effects assigned to this layer.[0405]Odstranit efekty přiřazené této vrstvě.";


class ATL_NO_VTABLE CMenuCommandsStyleClear :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsStyleClear>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsStyleClear()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsStyleClear)

BEGIN_CATEGORY_MAP(CMenuCommandsStyleClear)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsStyleClear)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Clear Style[0405]Vrstvený obrázek - odstranit styl");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return CConfigGUILayerIDDlg::CreateConfig(a_ppDefaultConfig);
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		if (a_ppSubCommands == NULL)
			return E_POINTER;

		try
		{
			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return E_FAIL;

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);
			CComBSTR bstrID;
			pDLI->StatePrefix(&bstrID);
			if (bstrID.Length())
			{
				bstrID += cVal;
			}
			else
			{
				bstrID.Attach(cVal.Detach().bstrVal);
			}

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSel;
			pDLI->StateUnpack(pState, &pSel);
			if (pSel == NULL)
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CLayerEffectClear>* p = NULL;
				CComObject<CLayerEffectClear>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel, a_pStates, bstrID);
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
	class ATL_NO_VTABLE CLayerEffectClear :
		public CDocumentMenuCommandMLImpl<CLayerEffectClear, CMDNAME_LAYEREFFECTCLEAR, CMDDESC_LAYEREFFECTCLEAR, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pSel, IOperationContext* a_pSSM, BSTR a_bstrSyncID)
	{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pSel = a_pSel;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			ULONG nSel = 0;
			m_pSel->Size(&nSel);
			if (nSel == 0) return EMCSDisabled;
			CComPtr<IComparable> pItem;
			m_pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			if (pItem == NULL) return EMCSDisabled;
			if (S_OK != m_pDLI->IsLayer(pItem, NULL, NULL, NULL)) return EMCSNormal; // effect selected -> there is at least one
			CComPtr<IEnumUnknowns> pEffects;
			m_pDLI->LayerEffectsEnum(pItem, &pEffects);
			if (pEffects == NULL) return EMCSDisabled;
			ULONG nEffects = 0;
			pEffects->Size(&nEffects);
			return nEffects ? EMCSNormal : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IComparable> pItem;
				m_pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
				if (pItem == NULL) return E_FAIL;
				CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(CMDNAME_LAYEREFFECTCLEAR));
				if (S_OK == m_pDLI->IsLayer(pItem, NULL, NULL, NULL))
				{
					return m_pDLI->LayerEffectSet(pItem, NULL);
				}
				else
				{
					CComPtr<IComparable> pLayer;
					m_pDLI->LayerFromEffect(pItem, &pLayer);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID);
					return m_pDLI->LayerEffectSet(pLayer, NULL);
				}
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IEnumUnknowns> m_pSel;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
	};
};

// {1F22766B-FD77-463f-8167-2419D2CD3E6D}
static const GUID CLSID_MenuCommandsStyleClear = {0x1f22766b, 0xfd77, 0x463f, {0x81, 0x67, 0x24, 0x19, 0xd2, 0xcd, 0x3e, 0x6d}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsStyleClear, CMenuCommandsStyleClear)
