
#include "stdafx.h"
/*#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <SharedStateUndo.h>
//#include <PlugInCache.h>
#include <DocumentMenuCommandImpl.h>


// CMenuCommandsLayerEffectDelete

// {C1A24525-101F-4578-B2E4-D9F20B069412}
extern GUID const CLSID_MenuCommandsLayerEffectDelete = {0xc1a24525, 0x101f, 0x4578, {0xb2, 0xe4, 0xd9, 0xf2, 0xb, 0x6, 0x94, 0x12}};
static const OLECHAR CFGID_SELECTIONSYNC[] = L"LayerID";

extern wchar_t const DELETEEFFECT_NAME[] = L"Remove effect";
extern wchar_t const DELETEEFFECT_DESC[] = L"Remove selected effects from a layer.";
extern wchar_t const DELETESTYLE_NAME[] = L"Remove style";
extern wchar_t const DELETESTYLE_DESC[] = L"Remove all effects from selected layers.";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUILayerEffectDeleteDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILayerEffectDeleteDlg>,
	public CDialogResize<CConfigGUILayerEffectDeleteDlg>
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

	BEGIN_MSG_MAP(CConfigGUILayerEffectDeleteDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayerEffectDeleteDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILayerEffectDeleteDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayerEffectDeleteDlg)
		DLGRESIZE_CONTROL(IDC_LAYERSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayerEffectDeleteDlg)
		CONFIGITEM_EDITBOX(IDC_LAYERSYNCID, CFGID_SELECTIONSYNC)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

class ATL_NO_VTABLE CMenuCommandsLayerEffectDelete :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerEffectDelete, &CLSID_MenuCommandsLayerEffectDelete>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerEffectDelete()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerEffectDelete)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerEffectDelete)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerEffectDelete)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Delete Effect[0405]Vrstvený obrázek - odstranit efekt");
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

			CConfigCustomGUI<&CLSID_MenuCommandsLayerEffectDelete, CConfigGUILayerEffectDeleteDlg>::FinalizeConfig(pCfg);

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
			if (nItems == 0)
				return S_FALSE;

			bool bAllEffects = true;
			bool bAllLayers = true;
			bool bLayerHasStyle = false;
			bool bAllSameParent = true;
			INT_PTR nParent = 0;
			for (ULONG i = 0; i < nItems; ++i)
			{
				CComPtr<IComparable> pItem;
				pItems->Get(i, &pItem);
				INT_PTR nPar = 0;
				if (S_OK == pDLI->IsLayer(pItem, NULL, &nPar, NULL))
				{
					bAllEffects = false;
					if (!bLayerHasStyle)
					{
						CComPtr<IConfig> pEffect;
						pDLI->LayerEffectGet(pItem, &pEffect, NULL);
						if (pEffect)
						{
							CConfigValue cID;
							pEffect->ItemValueGet(CComBSTR(L"Effect"), &cID);
							bLayerHasStyle = cID.TypeGet() == ECVTGUID && !IsEqualGUID(cID, __uuidof(DocumentOperationNULL));
						}
					}
				}
				else
					bAllLayers = false;
				if (nParent == 0)
					nParent = nPar;
				if (nParent != nPar)
					bAllSameParent = false;
			}
			if (!bAllSameParent || (!bAllLayers && !bAllEffects))
				return S_FALSE;
			if (bAllLayers && !bLayerHasStyle)
				return S_FALSE;

			CComPtr<IComparable> pItem;
			pItems->Get(0, &pItem);
			if (pDLI->IsLayer(pItem, NULL, NULL, NULL) != S_OK)
			{
				CComPtr<IComparable> pItem2;
				pDLI->LayerFromEffect(pItem, &pItem2);
				if (pItem2 == NULL)
					return S_FALSE; // weird
				std::swap(pItem.p, pItem2.p);
			}

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			if (bAllLayers)
			{
				CComObject<CDeleteStyle>* p = NULL;
				CComObject<CDeleteStyle>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, a_pView);

				pCmds->Insert(p);
			}
			else
			{
				CComObject<CDeleteEffect>* p = NULL;
				CComObject<CDeleteEffect>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, a_pStates, bstrID, a_pView);

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
	class ATL_NO_VTABLE CDeleteEffect :
		public CDocumentMenuCommandMLImpl<CDeleteEffect, DELETEEFFECT_NAME, DELETEEFFECT_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pItems, IOperationContext* a_pStates, BSTR a_bstrSyncID, IDesignerView* a_pView)
		{
			m_pItems = a_pItems;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				*a_peState = EMCSNormal;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView)
					m_pView->DeactivateAll(FALSE);

				CWriteLock<IDocument> cLock(m_pDocument.p);
				CUndoBlock cUndo(m_pDocument, CMultiLanguageString::GetAuto(DELETEEFFECT_NAME));

				CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrSyncID);
				CComPtr<IComparable> pNewSel;
				ULONG nItems = 0;
				m_pItems->Size(&nItems);
				for (ULONG i = 0; i < nItems; ++i)
				{
					CComPtr<IComparable> pEffect;
					m_pItems->Get(i, &pEffect);
					if (pNewSel == NULL)
						m_pDLI->LayerFromEffect(pEffect, &pNewSel);
					m_pDLI->LayerEffectStepDelete(pEffect);
				}
				CComPtr<ISharedState> pNewState;
				m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
				m_pStates->StateSet(m_bstrSyncID, pNewState);

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IEnumUnknowns> m_pItems;
		CComBSTR m_bstrSyncID;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	class ATL_NO_VTABLE CDeleteStyle :
		public CDocumentMenuCommandMLImpl<CDeleteStyle, DELETESTYLE_NAME, DELETESTYLE_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pItems, IDesignerView* a_pView)
		{
			m_pItems = a_pItems;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				*a_peState = EMCSNormal;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView)
					m_pView->DeactivateAll(FALSE);

				CWriteLock<IDocument> cLock(m_pDocument.p);
				CUndoBlock cUndo(m_pDocument, CMultiLanguageString::GetAuto(DELETESTYLE_NAME));

				ULONG nItems = 0;
				m_pItems->Size(&nItems);
				for (ULONG i = 0; i < nItems; ++i)
				{
					CComPtr<IComparable> pLayer;
					m_pItems->Get(i, &pLayer);

					CComPtr<IConfig> pConfig;
					m_pDLI->LayerEffectGet(pLayer, &pConfig, NULL);
					if (pConfig == NULL) return E_FAIL;
					CComBSTR c(L"Effect");
					pConfig->ItemValuesSet(1, &(c.m_str), CConfigValue(__uuidof(DocumentOperationNULL)));
					m_pDLI->LayerEffectSet(pLayer, pConfig);
				}

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IEnumUnknowns> m_pItems;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerEffectDelete, CMenuCommandsLayerEffectDelete)
*/