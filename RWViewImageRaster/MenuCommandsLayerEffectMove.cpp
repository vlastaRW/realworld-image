
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <SharedStateUndo.h>
//#include <PlugInCache.h>
#include <DocumentMenuCommandImpl.h>


// CMenuCommandsLayerEffectMove

// {78D3CF20-1285-4d10-A425-241E3AEB6D05}
extern GUID const CLSID_MenuCommandsLayerEffectMove = {0x78d3cf20, 0x1285, 0x4d10, {0xa4, 0x25, 0x24, 0x1e, 0x3a, 0xeb, 0x6d, 0x5}};
static const OLECHAR CFGID_SELECTIONSYNC[] = L"LayerID";

extern wchar_t const MOVEEFFECTUP_NAME[] = L"Move effect up";
extern wchar_t const MOVEEFFECTDOWN_NAME[] = L"Move effect down";
extern wchar_t const MOVEEFFECT_DESC[] = L"Change the order of effects.";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUILayerEffectMoveDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILayerEffectMoveDlg>,
	public CDialogResize<CConfigGUILayerEffectMoveDlg>
{
public:
	enum
	{
		IDC_OPERATION = 100,
		IDC_LAYERSYNCID,
	};

	BEGIN_DIALOG_EX(0, 0, 180, 17, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizace výběru:"), IDC_STATIC, 0, 2, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_LAYERSYNCID, 60, 0, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILayerEffectMoveDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayerEffectMoveDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILayerEffectMoveDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayerEffectMoveDlg)
		DLGRESIZE_CONTROL(IDC_LAYERSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayerEffectMoveDlg)
		CONFIGITEM_EDITBOX(IDC_LAYERSYNCID, CFGID_SELECTIONSYNC)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

class ATL_NO_VTABLE CMenuCommandsLayerEffectMove :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerEffectMove, &CLSID_MenuCommandsLayerEffectMove>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerEffectMove()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerEffectMove)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerEffectMove)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerEffectMove)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Move Effect[0405]Vrstvený obrázek - přesunout efekt");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;
			CComPtr<IConfigWithDependencies> pCfg;
			RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

			pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection sync ID[0405]Synchronizace výběru"), CMultiLanguageString::GetAuto(L"[0409]Image selection is synchronized by the given ID.[0405]Vybraná oblast v obrázku je dopstupná a sychronizována přes zadané ID."), CConfigValue(L"LAYER"), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_MenuCommandsLayerEffectMove, CConfigGUILayerEffectMoveDlg>::FinalizeConfig(pCfg);

			*a_ppDefaultConfig = pCfg.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return S_FALSE;

			CConfigValue cSelID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSelID);
			CComBSTR bstrID;
			pDLI->StatePrefix(&bstrID);
			if (bstrID.Length())
			{
				bstrID += cSelID.operator BSTR();
			}
			else
			{
				bstrID = cSelID.operator BSTR();
			}

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));

			CComPtr<IEnumUnknowns> pItems;
			pDLI->StateUnpack(pState, &pItems);
			ULONG nItems = 0;
			if (pItems) pItems->Size(&nItems);
			if (nItems != 1)
				return S_FALSE;

			CComPtr<IComparable> pSel;
			pItems->Get(0, &pSel);

			CComPtr<IComparable> pLayer;
			pDLI->LayerFromEffect(pSel, &pLayer);
			if (pLayer == NULL)
				return S_FALSE;

			CComPtr<IEnumUnknowns> pEffects;
			pDLI->LayerEffectsEnum(pLayer, &pEffects);
			ULONG nEffects = 0;
			if (pEffects) pEffects->Size(&nEffects);
			if (nEffects < 2)
				return S_FALSE;

			CComPtr<IComparable> pPrev;
			CComPtr<IComparable> pNext;
			for (ULONG i = 0; i < nEffects; ++i)
			{
				CComPtr<IComparable> pEffect;
				pEffects->Get(i, &pEffect);
				if (S_OK == pEffect->Compare(pSel))
				{
					if (i > 0)
						pEffects->Get(i-1, &pPrev);
					if (i+1 < nEffects)
						pEffects->Get(i+1, &pNext);
					break;
				}
			}

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			// prev/next swapped due to inverse viewv order
			{
				CComObject<CMove<MOVEEFFECTUP_NAME> >* p = NULL;
				CComObject<CMove<MOVEEFFECTUP_NAME> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pSel, pNext, a_pStates, bstrID, pNext, a_pView);

				pCmds->Insert(p);
			}
			{
				CComObject<CMove<MOVEEFFECTDOWN_NAME> >* p = NULL;
				CComObject<CMove<MOVEEFFECTDOWN_NAME> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pPrev, pSel, a_pStates, bstrID, pPrev, a_pView);

				pCmds->Insert(p);
			}

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	template<wchar_t const* TNAME>
	class ATL_NO_VTABLE CMove :
		public CDocumentMenuCommandMLImpl<CMove<TNAME>, TNAME, MOVEEFFECT_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IComparable* a_pItem1, IComparable* a_pItem2, IOperationContext* a_pStates, BSTR a_bstrID, IComparable* a_pSel, IDesignerView* a_pView)
		{
			m_pItem1 = a_pItem1;
			m_pItem2 = a_pItem2;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pStates = a_pStates;
			m_bstrID = a_bstrID;
			m_pSel = a_pSel;
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState() { return m_pItem1 && m_pItem2 ? EMCSNormal : EMCSDisabled; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView)
					m_pView->DeactivateAll(FALSE);

				CWriteLock<IDocument> cLock(m_pDocument.p);
				CUndoBlock cUndo(m_pDocument, CMultiLanguageString::GetAuto(TNAME));

				BYTE bEnabled1 = 1;
				GUID tOpID1 = GUID_NULL;
				CComPtr<IConfig> pOpCfg1;
				m_pDLI->LayerEffectStepGet(m_pItem1, &bEnabled1, &tOpID1, &pOpCfg1);
				BYTE bEnabled2 = 1;
				GUID tOpID2 = GUID_NULL;
				CComPtr<IConfig> pOpCfg2;
				m_pDLI->LayerEffectStepGet(m_pItem2, &bEnabled2, &tOpID2, &pOpCfg2);
				m_pDLI->LayerEffectStepSet(m_pItem1, &bEnabled2, &tOpID2, pOpCfg2);
				m_pDLI->LayerEffectStepSet(m_pItem2, &bEnabled1, &tOpID1, pOpCfg1);

				CComPtr<ISharedState> pNewState;
				m_pDLI->StatePack(1, &(m_pSel.p), &pNewState);
				m_pStates->StateSet(m_bstrID, pNewState);
				CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrID);

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IComparable> m_pItem1;
		CComPtr<IComparable> m_pItem2;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrID;
		CComPtr<IComparable> m_pSel;
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerEffectMove, CMenuCommandsLayerEffectMove)
