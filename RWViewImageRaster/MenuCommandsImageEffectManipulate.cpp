
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include "ConfigGUILayerID.h"
#include <IconRenderer.h>
#include <SharedStateUndo.h>

extern wchar_t const EFFECT_APPLY_NAME[] = L"[0409]Done[0405]Hotovo";
extern wchar_t const EFFECT_APPLY_DESC[] = L"[0409]Save the effect and continue editing the layer itself.[0405]Uložit efekt a pokračovat v upravování vrstvy.";
extern wchar_t const EFFECT_REMOVE_NAME[] = L"[0409]Remove[0405]Odstranit";
extern wchar_t const EFFECT_REMOVE_DESC[] = L"[0409]Remove this effect from the layer style.[0405]Odstranit tento efekt ze stylu vrstvy.";

extern wchar_t const DELETEEFFECT_NAME[];

class ATL_NO_VTABLE CMenuCommandsImageEffectManipulate :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsImageEffectManipulate>,
	//public CDesignerFrameIconsImpl<1, &g_tIconIDLayerAddRaster, &ARICONID>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsImageEffectManipulate()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsImageEffectManipulate)

BEGIN_CATEGORY_MAP(CMenuCommandsImageEffectManipulate)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	//IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsImageEffectManipulate)
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
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Effect Finish[0405]Vrstvený obrázek - ukončit efekt");
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
				return E_NOINTERFACE;

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			{
				CComObject<CApply>* p = NULL;
				CComObject<CApply>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pCmd = p;
				p->Init(a_pDocument, pDLI, a_pStates, cVal);
				pCmds->Insert(pCmd);
			}
			{
				CComObject<CRemove>* p = NULL;
				CComObject<CRemove>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pCmd = p;
				p->Init(a_pDocument, pDLI, a_pStates, cVal);
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
	class ATL_NO_VTABLE CApply :
		public CDocumentMenuCommandMLImpl<CApply, EFFECT_APPLY_NAME, EFFECT_APPLY_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pStates, BSTR a_bstrSyncID)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				static GUID const tID = {0xcd15ebbf, 0x5efe, 0x4bbf, {0x9f, 0x4e, 0x9e, 0x86, 0xd7, 0x5a, 0x40, 0xbe}};
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
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				pSI->GetLayers(ESIConfirm, cRenderer, IRTarget(0.9f));
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState() { return EMCSShowButtonText; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			CComBSTR bstrID;
			m_pDLI->StatePrefix(&bstrID);
			bstrID += m_bstrSyncID;
			CComPtr<ISharedState> pState;
			m_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSelectedItems;
			m_pDLI->StateUnpack(pState, &pSelectedItems);
			CComPtr<IComparable> pItem;
			if (pSelectedItems) pSelectedItems->Get(0, &pItem);
			CComPtr<IComparable> pLayer;
			if (pItem) m_pDLI->LayerFromEffect(pItem, &pLayer);
			CComPtr<ISharedState> pNewState;
			if (pLayer) m_pDLI->StatePack(1, &(pLayer.p), &pNewState);
			if (pNewState) m_pStates->StateSet(bstrID, pNewState);
			return S_OK;
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
	};

	class ATL_NO_VTABLE CRemove :
		public CDocumentMenuCommandMLImpl<CRemove, EFFECT_REMOVE_NAME, EFFECT_REMOVE_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pStates, BSTR a_bstrSyncID)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				static GUID const tID = {0xa96d38f3, 0x446b, 0x4fee, {0x8b, 0x15, 0x56, 0xdf, 0x11, 0x38, 0xb0, 0x38}};
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
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				pSI->GetLayers(ESIDelete, cRenderer, IRTarget(0.8f));
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState() { return EMCSShowButtonText; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			CWriteLock<IDocument> cLock(m_pDoc.p);
			CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(DELETEEFFECT_NAME));

			CComBSTR bstrID;
			m_pDLI->StatePrefix(&bstrID);
			bstrID += m_bstrSyncID;
			CComPtr<ISharedState> pState;
			m_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSelectedItems;
			m_pDLI->StateUnpack(pState, &pSelectedItems);
			CComPtr<IComparable> pItem;
			if (pSelectedItems) pSelectedItems->Get(0, &pItem);
			CComPtr<IComparable> pLayer;
			if (pItem) m_pDLI->LayerFromEffect(pItem, &pLayer);
			CComPtr<ISharedState> pNewState;
			if (pLayer) m_pDLI->StatePack(1, &(pLayer.p), &pNewState);
			CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pStates, bstrID, pState);
			if (pNewState) m_pStates->StateSet(bstrID, pNewState);
			return m_pDLI->LayerEffectStepDelete(pItem);
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
	};
};

// {F8272EAC-1075-4b08-ADBD-EDEFA9E7FACF}
static const GUID CLSID_MenuCommandsImageEffectManipulate = {0xf8272eac, 0x1075, 0x4b08, {0xad, 0xbd, 0xed, 0xef, 0xa9, 0xe7, 0xfa, 0xcf}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsImageEffectManipulate, CMenuCommandsImageEffectManipulate)
