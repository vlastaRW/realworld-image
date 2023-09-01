
#include "stdafx.h"
#include <RWProcessing.h>
#include <WeakSingleton.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <SharedStateUndo.h>
#include <PlugInCache.h>


// CMenuCommandsToggleEffect

// {DE608178-4FFD-4e6f-B511-1E22FF1028AF}
extern GUID const CLSID_MenuCommandsToggleEffect = {0xde608178, 0x4ffd, 0x4e6f, {0xb5, 0x11, 0x1e, 0x22, 0xff, 0x10, 0x28, 0xaf}};
static const OLECHAR CFGID_OPERATIONID[] = L"Operation";
static const OLECHAR CFGID_SELECTIONSYNC[] = L"LayerID";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIToggleEffectDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIToggleEffectDlg>,
	public CDialogResize<CConfigGUIToggleEffectDlg>
{
public:
	enum
	{
		IDC_OPERATION = 100,
		IDC_LAYERSYNCID,
	};

	BEGIN_DIALOG_EX(0, 0, 180, 33, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Operation:[0405]Operace:"), IDC_STATIC, 0, 2, 60, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_OPERATION, 60, 0, 119, 160, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizace výběru:"), IDC_STATIC, 0, 18, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_LAYERSYNCID, 60, 16, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIToggleEffectDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIToggleEffectDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIToggleEffectDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIToggleEffectDlg)
		DLGRESIZE_CONTROL(IDC_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LAYERSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIToggleEffectDlg)
		CONFIGITEM_COMBOBOX(IDC_OPERATION, CFGID_OPERATIONID)
		CONFIGITEM_EDITBOX(IDC_LAYERSYNCID, CFGID_SELECTIONSYNC)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

class ATL_NO_VTABLE CMenuCommandsToggleEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsToggleEffect, &CLSID_MenuCommandsToggleEffect>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsToggleEffect()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsToggleEffect)

BEGIN_CATEGORY_MAP(CMenuCommandsToggleEffect)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsToggleEffect)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Toggle Effect[0405]Vrstvený obrázek - přepnout efekt");
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

			CComQIPtr<IOperationManager> pMgr(a_pManager);

			CComPtr<IConfigItemCustomOptions> pCustOpts;
			CComObject<CCustomOptions>* pCO = NULL;
			CComObject<CCustomOptions>::CreateInstance(&pCO);
			pCO->Init(pMgr ? pMgr : M_OperationManager());
			pCustOpts = pCO;
			pCfg->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_OPERATIONID), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Document operation associated with this command.[0405]Dokumentová operace spojená s tímto příkazem."), CConfigValue(GUID_NULL), pCustOpts, NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_MenuCommandsToggleEffect, CConfigGUIToggleEffectDlg>::FinalizeConfig(pCfg);

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

			CConfigValue cOpID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATIONID), &cOpID);

			CComObject<CDocumentMenuCommand>* p = NULL;
			CComObject<CDocumentMenuCommand>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;

			CComQIPtr<IOperationManager> pMgr(a_pManager);

			p->Init(a_pDocument, pDLI, pItem, a_pStates, bstrID, cOpID, pMgr ? pMgr : M_OperationManager(), a_pView);

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));
			pCmds->Insert(p);

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	IOperationManager* M_OperationManager()
	{
		ObjectLock cLock(this);
		if (m_pOpsMgr)
			return m_pOpsMgr;
		RWCoCreateInstance(m_pOpsMgr, __uuidof(OperationManager));
		if (m_pOpsMgr == NULL)
			throw E_UNEXPECTED;
		return m_pOpsMgr;
	}

private:
	class ATL_NO_VTABLE CCustomOptions :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IConfigItemCustomOptions
	{
	public:
		void Init(IOperationManager* a_pOverride)
		{
			m_pOverride = a_pOverride;
		}

	BEGIN_COM_MAP(CCustomOptions)
		COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
		COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
	END_COM_MAP()

		// IConfigItemCustomOptions methods
	public:
		STDMETHOD(GetValueName)(TConfigValue const* a_pValue, ILocalizedString** a_ppName)
		{
			CComPtr<IConfigDescriptor> p;
			if (a_pValue && a_pValue->eTypeID == ECVTGUID)
				RWCoCreateInstance(p, a_pValue->guidVal);
			if (p)
				return p->Name(m_pOverride, NULL, a_ppName);
			return E_FAIL;
		}

		// IEnumConfigItemOptions methods
	public:
		struct size
		{
			ULONG* a_pnSize;
			size(ULONG* a_pnSize) : a_pnSize(a_pnSize) {}
			void operator()(CLSID const* begin, CLSID const* const end) const { *a_pnSize = end-begin; }
		};
		STDMETHOD(Size)(ULONG* a_pnSize)
		{
			CPlugInEnumerator::EnumCategoryCLSIDs(CATID_DocumentOperation, size(a_pnSize));
			return S_OK;
		}
		struct copy
		{
			ULONG a_nIndexFirst;
			ULONG a_nCount;
			TConfigValue* a_atItems;
			ULONG& copied;
			copy(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems, ULONG& copied) :
			a_nIndexFirst(a_nIndexFirst), a_nCount(a_nCount), a_atItems(a_atItems), copied(copied) {}
			void operator()(CLSID const* begin, CLSID const* end)
			{
				begin += a_nIndexFirst;
				if (begin + a_nCount < end) end = begin + a_nCount;
				while (begin < end)
				{
					a_atItems->eTypeID = ECVTGUID;
					a_atItems->guidVal = *begin;
					++begin;
					++a_atItems;
					++copied;
				}
			}
		};
		STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
		{
			ULONG copied = 0;
			CPlugInEnumerator::EnumCategoryCLSIDs(CATID_DocumentOperation, copy(a_nIndex, 1, a_ptItem, copied));
			return copied == 1 ? S_OK : E_RW_INDEXOUTOFRANGE;
		}
		STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
		{
			ULONG copied = 0;
			CPlugInEnumerator::EnumCategoryCLSIDs(CATID_DocumentOperation, copy(a_nIndexFirst, a_nCount, a_atItems, copied));
			return copied == a_nCount ? S_OK : E_RW_INDEXOUTOFRANGE;
		}

	private:
		CComPtr<IOperationManager> m_pOverride;
	};

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CDocumentMenuCommand()
		{
		}
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IComparable* a_pItem, IOperationContext* a_pStates, BSTR a_bstrSyncID, GUID const& a_tOpID, IOperationManager* a_pOpsMgr, IDesignerView* a_pView)
		{
			m_pItem = a_pItem;
			m_tOpID = a_tOpID;
			m_pOpsMgr = a_pOpsMgr;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
			RWCoCreateInstance(m_pOp, a_tOpID);
			m_pDescriptor = m_pOp;
		}

	BEGIN_COM_MAP(CDocumentMenuCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = NULL;
				return m_pDescriptor->Name(m_pOpsMgr, NULL, a_ppText);
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]Toggle effect on the selected layer.[0405]Přepnout efekt na vybrané vrstvě.");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (m_pDescriptor && SUCCEEDED(m_pDescriptor->PreviewIconID(m_pOpsMgr, NULL, a_pIconID)))
				return S_OK;
			return E_FAIL;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (m_pDescriptor && SUCCEEDED(m_pDescriptor->PreviewIcon(m_pOpsMgr, NULL, a_nSize, a_phIcon)))
				return S_OK;
			return E_FAIL;
			//try
			//{
			//	CComPtr<IDesignerFrameIcons> pIcons;
			//	RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
			//	return pIcons->GetIcon(m_tOpID, a_nSize, a_phIcon);
			//}
			//catch (...)
			//{
			//	return a_phIcon ? E_UNEXPECTED : E_POINTER;
			//}
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands)) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				CComPtr<IEnumUnknowns> pEffects;
				m_pDLI->LayerEffectsEnum(m_pItem, &pEffects);
				ULONG nEffects = 0;
				if (pEffects) pEffects->Size(&nEffects);
				CComPtr<IComparable> pToRemove;
				for (ULONG i = 0; i < nEffects; ++i)
				{
					CComPtr<IComparable> pEffect;
					pEffects->Get(i, &pEffect);
					GUID tID = GUID_NULL;
					m_pDLI->LayerEffectStepGet(pEffect, NULL, &tID, NULL);
					if (IsEqualGUID(tID, m_tOpID))
					{
						*a_peState = EMCSChecked;
						return S_OK;
					}
				}
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

	//[helpstring("method LayerEffectStepGet")] HRESULT LayerEffectStepGet([in] IComparable* a_pEffect, [out] BYTE* a_pEnabled, [out] GUID* a_pOpID, [out] IConfig** a_ppOpCfg);
	//[helpstring("method LayerEffectStepSet")] HRESULT LayerEffectStepSet([in] IComparable* a_pEffect, [in] BYTE* a_pEnabled, [in] GUID* a_pOpID, [in] IConfig* a_pOpCfg);
	//[helpstring("method LayerEffectStepAppend")] HRESULT LayerEffectStepAppend([in] IComparable* a_pLayer, [in] BYTE a_bEnabled, [in] REFGUID a_tOpID, [in] IConfig* a_pOpCfg, [out] IComparable** a_ppNew);
	//[helpstring("method LayerEffectStepDelete")] HRESULT LayerEffectStepDelete([in] IComparable* a_pEffect);
	//[helpstring("method IsLayer")] HRESULT IsLayer([in] IComparable* a_pItem, [out] ULONG* a_pLevel, [out] INT_PTR* a_pParentHandle, [out] GUID* a_pBuilder);
	//[helpstring("method LayerEffectsEnum")] HRESULT LayerEffectsEnum([in] IComparable* a_pLayer, [out] IEnumUnknowns** a_ppEffects);
	//[helpstring("method LayerFromEffect")] HRESULT LayerFromEffect([in] IComparable* a_pEffect, [out] IComparable** a_ppLayer);
				CWriteLock<IDocument> cLock(m_pDocument.p);
				CUndoBlock cUndo(m_pDocument, CMultiLanguageString::GetAuto(L"[0409]Toggle effect[0405]Přepnout efekt"));

				CComPtr<IEnumUnknowns> pEffects;
				m_pDLI->LayerEffectsEnum(m_pItem, &pEffects);
				ULONG nEffects = 0;
				if (pEffects) pEffects->Size(&nEffects);
				CComPtr<IComparable> pToRemove;
				for (ULONG i = 0; i < nEffects; ++i)
				{
					CComPtr<IComparable> pEffect;
					pEffects->Get(i, &pEffect);
					GUID tID = GUID_NULL;
					m_pDLI->LayerEffectStepGet(pEffect, NULL, &tID, NULL);
					if (IsEqualGUID(tID, m_tOpID))
					{
						pToRemove = pEffect;
						break;
					}
				}

				if (pToRemove)
				{
					CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrSyncID);
					m_pDLI->LayerEffectStepDelete(pToRemove);
					CComPtr<ISharedState> pNewState;
					m_pDLI->StatePack(1, &(m_pItem.p), &pNewState);
					m_pStates->StateSet(m_bstrSyncID, pNewState);
					return S_OK;
				}

				// insert effect
				CComPtr<IComparable> pBefore;
				CComPtr<ILayerStyle> pNew;
				RWCoCreateInstance(pNew, m_tOpID);
				if (pNew)
				{
					BYTE bPrio = pNew->ExecutionPriority();
					CComPtr<IEnumUnknowns> pEffects;
					m_pDLI->LayerEffectsEnum(m_pItem, &pEffects);
					ULONG nEffects = 0;
					if (pEffects) pEffects->Size(&nEffects);
					for (ULONG i = 0; i < nEffects; ++i)
					{
						CComPtr<IComparable> p;
						pEffects->Get(i, &p);
						GUID tID = GUID_NULL;
						m_pDLI->LayerEffectStepGet(p, NULL, &tID, NULL);
						CComPtr<ILayerStyle> pOld;
						RWCoCreateInstance(pOld, tID);
						if (pOld && pOld->IsPriorityAnchor() == S_OK)
						{
							BYTE bOldPrio = pOld->ExecutionPriority();
							if (bPrio < bOldPrio)
							{
								pBefore = p;
								break;
							}
						}
					}
				}
				CComPtr<IComparable> pNewSel;
				m_pDLI->LayerEffectStepAppend(pBefore.p ? pBefore.p : m_pItem.p, TRUE, m_tOpID, NULL, &pNewSel);
				if (pNewSel)
				{
					CComPtr<ISharedState> pNewState;
					m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
					m_pStates->StateSet(m_bstrSyncID, pNewState);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrSyncID);
				}

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IComparable> m_pItem;
		CComPtr<IDocumentOperation> m_pOp;
		CComQIPtr<IConfigDescriptor> m_pDescriptor;
		GUID m_tOpID;
		CComPtr<IOperationManager> m_pOpsMgr;
		CComBSTR m_bstrSyncID;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	CComPtr<IOperationManager> m_pOpsMgr;
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsToggleEffect, CMenuCommandsToggleEffect)
