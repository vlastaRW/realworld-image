
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <SharedStateUndo.h>
#include <IconRenderer.h>

extern wchar_t const DOGROUP_NAME[] = L"[0409]Group layers[0405]Seskupit vrstvy";
extern wchar_t const DOGROUP_DESC[] = L"[0409]Move selected layers into a new group.[0405]Přesunout vybrané vrstvy do skupiny.";
extern wchar_t const UNGROUP_NAME[] = L"[0409]Dissolve group[0405]Rozpustit skupinu";
extern wchar_t const UNGROUP_DESC[] = L"[0409]Move layers from a group directly into parent layer.[0405]Přesunout vrstvy ze skupiny přímo do rodičovské vrstvy.";
// {3140B71B-BDA6-482f-8D66-925A86D377BE}
extern GUID const DOGROUP_ICONID = {0x3140b71b, 0xbda6, 0x482f, {0x8d, 0x66, 0x92, 0x5a, 0x86, 0xd3, 0x77, 0xbe}};
// {2E0F18B6-C2C7-490d-B7BF-E8608256ABBA}
extern GUID const UNGROUP_ICONID = {0x2e0f18b6, 0xc2c7, 0x490d, {0xb7, 0xbf, 0xe8, 0x60, 0x82, 0x56, 0xab, 0xba}};


class ATL_NO_VTABLE CMenuCommandsLayerGrouping :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerGrouping>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerGrouping()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerGrouping)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerGrouping)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerGrouping)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Toggle Group[0405]Vrstvený obrázek - přepnout skupinu");
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

			std::vector<CComPtr<IComparable> > cLayers;
			INT_PTR nRoot = 0;
			bool bValid = true;
			for (ULONG i = 0; i < nItems; ++i)
			{
				CComPtr<IComparable> pItem;
				pItems->Get(i, &pItem);
				INT_PTR nR = 0;
				if (S_OK != pDLI->IsLayer(pItem, NULL, &nR, NULL))
				{
					bValid = false; // non-layer selected
					break;
				}
				if (nR == 0 || (nRoot != 0 && nRoot != nR))
				{
					bValid = false; // items having different parents
					break;
				}
				nRoot = nR;
				cLayers.push_back(pItem);
			}

			if (bValid && cLayers.size() == 1)
			{
				GUID tID = GUID_NULL;
				if (S_OK == pDLI->IsLayer(cLayers[0], NULL, NULL, &tID) && IsEqualGUID(tID, __uuidof(DocumentFactoryLayeredImage)))
				{
					// ungroup
					CComObject<CUngroup>* pSM = NULL;
					CComObject<CUngroup>::CreateInstance(&pSM);
					CComPtr<IDocumentMenuCommand> pCmd = pSM;
					pSM->Init(a_pDocument, pDLI, cLayers[0], a_pView, a_pStates, bstrID);

					CComPtr<IEnumUnknownsInit> pCmds;
					RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));
					pCmds->Insert(pCmd);
					*a_ppSubCommands = pCmds.Detach();
					return S_OK;
				}
			}

			CComObject<CGroup>* pSM = NULL;
			CComObject<CGroup>::CreateInstance(&pSM);
			CComPtr<IDocumentMenuCommand> pCmd = pSM;
			pSM->Init(a_pDocument, pDLI, cLayers, a_pView, a_pStates, bstrID, bValid);

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			pCmds->Insert(pCmd);

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	static HICON ToggleGroupIcon(ULONG a_nSize)
	{
		static IRPolyPoint const aVertices1[] = {{16, 16}, {96, 16}, {96, 64}, {64, 64}, {64, 192}, {96, 192}, {96, 240}, {16, 240}};
		static IRPolyPoint const aVertices2[] = {{160, 64}, {160, 16}, {240, 16}, {240, 240}, {160, 240}, {160, 192}, {192, 192}, {192, 64}};
		static IRPolygon const aPolys[] = {{itemsof(aVertices1), aVertices1}, {itemsof(aVertices2), aVertices2}};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));

		static IRGridItem const gridX[] = { {EGIFInteger, 16.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 96.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 192.0f}, {EGIFInteger, 240.0f}};
		static IRGridItem const gridY[] = { {EGIFInteger, 16.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 192.0f}, {EGIFInteger, 240.0f}};
		static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(aPolys), aPolys, pSI->GetMaterial(ESMScheme2Color2));//ESMInterior));
		return cRenderer.get();
	}

	class ATL_NO_VTABLE CUngroup :
		public CDocumentMenuCommandMLImpl<CUngroup, UNGROUP_NAME, UNGROUP_DESC, &DOGROUP_ICONID, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IComparable* a_pItem, IDesignerView* a_pView, IOperationContext* a_pSSM, BSTR a_bstrSyncID)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pItem = a_pItem;
			m_pView = a_pView;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand methods
	public:
		EMenuCommandState IntState() { return EMCSChecked; }
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				*a_phIcon = ToggleGroupIcon(a_nSize);
				return S_OK;
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
		{
			try
			{
				if (m_pView) m_pView->DeactivateAll(FALSE);

				CComPtr<ISharedState> pState;
				m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				if (pState == NULL) return E_FAIL;

				CWriteLock<IDocument> cLock(m_pDoc.p);
				CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(UNGROUP_NAME));

				//CComPtr<ISubDocumentID> pSDID;
				//m_pDLI->ItemFeatureGet(m_pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
				//CComPtr<IDocument> pLayerDoc;
				//pSDID->SubDocumentGet(&pLayerDoc);
				CComPtr<IEnumUnknowns> pSubItems;
				m_pDLI->ItemsEnum(m_pItem, &pSubItems);
				ULONG nSubItems = 0;
				if (pSubItems) pSubItems->Size(&nSubItems);
				std::vector<CComPtr<IComparable> > cNewSel;
				for (ULONG i = 0; i < nSubItems; ++i)
				{
					CComPtr<IComparable> pItem;
					pSubItems->Get(nSubItems-1-i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
					if (S_OK != m_pDLI->IsLayer(pItem, NULL, NULL, NULL))
						continue;
					//CComPtr<ISubDocumentID> pSDID;
					//m_pDLI->ItemFeatureGet(m_pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
					//CComPtr<IDocument> pLayerDoc;
					//pSDID->SubDocumentGet(&pLayerDoc);

					CImageLayerCreatorDocument cCopier(m_pDLI, pItem);
					CComPtr<IComparable> pNew;
					HRESULT hRes = m_pDLI->LayerInsert(m_pItem, ELIPBelow, &cCopier, &pNew);
					if (SUCCEEDED(hRes) && pNew)
					{
						CComBSTR bstrName;
						m_pDLI->LayerNameGet(pItem, &bstrName);
						m_pDLI->LayerNameSet(pNew, bstrName);
						ELayerBlend eBlend = EBEAlphaBlend;
						BYTE bVisible = 1;
						m_pDLI->LayerPropsGet(pItem, &eBlend, &bVisible);
						m_pDLI->LayerPropsSet(pNew, &eBlend, &bVisible);
						CComPtr<IConfig> pEffect;
						m_pDLI->LayerEffectGet(pItem, &pEffect, NULL);
						m_pDLI->LayerEffectSet(pNew, pEffect);
						cNewSel.push_back(pNew);
					}
				}
				m_pDLI->LayerDelete(m_pItem);
				CAutoVectorPtr<IComparable*> tmp(new IComparable*[cNewSel.size()]);
				for (size_t i = 0; i < cNewSel.size(); ++i)
					tmp[i] = cNewSel[i];
				CComPtr<ISharedState> pStateNew;
				m_pDLI->StatePack(cNewSel.size(), tmp.m_p, &pStateNew);
				m_pSSM->StateSet(m_bstrSyncID, pStateNew);
				CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID, pState);

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
			return S_OK;
		}


	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IComparable> m_pItem;
		CComPtr<IDesignerView> m_pView;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
	};

	class ATL_NO_VTABLE CGroup :
		public CDocumentMenuCommandMLImpl<CGroup, DOGROUP_NAME, DOGROUP_DESC, &DOGROUP_ICONID, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, std::vector<CComPtr<IComparable> >& a_cItems, IDesignerView* a_pView, IOperationContext* a_pSSM, BSTR a_bstrSyncID, bool a_bValid)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			std::swap(m_cItems, a_cItems);
			std::sort(m_cItems.begin(), m_cItems.end(), lessComparable);
			m_pView = a_pView;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
			m_bValid = a_bValid;
		}
		static bool lessComparable(CComPtr<IComparable> const& c1, CComPtr<IComparable> const& c2)
		{
			return c1->Compare(c2) == S_LESS;
		}

		// IDocumentMenuCommand methods
	public:
		EMenuCommandState IntState() { return m_bValid ? EMCSNormal : EMCSDisabled; }
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				*a_phIcon = ToggleGroupIcon(a_nSize);
				return S_OK;
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID a_tLocaleID)
		{
			if (!m_bValid)
				return S_FALSE;
			try
			{
				if (m_pView) m_pView->DeactivateAll(FALSE);

				CComPtr<ISharedState> pState;
				m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				if (pState == NULL) return E_FAIL;

				CWriteLock<IDocument> cLock(m_pDoc.p);
				CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(DOGROUP_NAME));

				CComPtr<IDocumentImage> pImg;
				m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
				TImageSize tSize = {0, 0};
				pImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

				// reorder selection by parent
				CComPtr<IEnumUnknowns> pAll;
				{
					CComPtr<IEnumUnknowns> pParents;
					m_pDLI->ParentsEnum(m_cItems[0], &pParents);
					CComPtr<IComparable> pParent;
					if (pParents) pParents->Get(0, &pParent);
					m_pDLI->LayersEnum(pParent, &pAll);
				}
				ULONG nAll = 0;
				if (pAll) pAll->Size(&nAll);
				size_t nSel = 0;
				for (ULONG i = 0; i < nAll && nSel < m_cItems.size(); ++i)
				{
					CComPtr<IComparable> p;
					pAll->Get(i, &p);
					for (size_t j = nSel; j < m_cItems.size(); ++j)
					{
						if (p->Compare(m_cItems[j]) == S_OK)
						{
							if (j != nSel)
								std::swap(m_cItems[j].p, m_cItems[nSel].p);
							++nSel;
							break;
						}
					}
				}

				// insert group with layer copies
				CComObjectStackEx<CCreator> cCreator;
				cCreator.Init(m_pDLI, tSize, m_cItems.begin(), m_cItems.end());
				CComPtr<IComparable> pNew;
				HRESULT hRes = m_pDLI->LayerInsert(m_cItems[0], ELIPBelow, &cCreator, &pNew);

				if (SUCCEEDED(hRes) && pNew)
				{
					// delete copied layers
					for (std::vector<CComPtr<IComparable> >::const_iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
						m_pDLI->LayerDelete(*i);

					// set name
					CComBSTR bstrName;
					CMultiLanguageString::GetLocalized(L"[0409]unnamed[0405]nepojmenovná", a_tLocaleID, &bstrName);
					m_pDLI->LayerNameSet(pNew, bstrName);

					// selection
					CComPtr<ISharedState> pStateNew;
					m_pDLI->StatePack(1, &(pNew.p), &pStateNew);
					m_pSSM->StateSet(m_bstrSyncID, pStateNew);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID, pState);
				}

				return hRes;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

		class ATL_NO_VTABLE CCreator :
			public CComObjectRootEx<CComMultiThreadModel>,
			public IImageLayerCreator
		{
		public:
			void Init(IDocumentLayeredImage* a_pDLI, TImageSize a_tSize, std::vector<CComPtr<IComparable> >::const_iterator a_b, std::vector<CComPtr<IComparable> >::const_iterator a_e)
			{
				m_pDLI = a_pDLI;
				m_tSize = a_tSize;
				m_b = a_b;
				m_e = a_e;
			}

		BEGIN_COM_MAP(CCreator)
			COM_INTERFACE_ENTRY(IImageLayerCreator)
		END_COM_MAP()

			// IImageLayerCreator methods
		public:
			STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase *a_pBase)
			{
				CComPtr<IDocumentFactoryLayeredImage> pFact;
				RWCoCreateInstance(pFact, __uuidof(DocumentFactoryLayeredImage));
				pFact->Create(a_bstrID, a_pBase);
				for (std::vector<CComPtr<IComparable> >::const_iterator i = m_b; i != m_e; ++i)
				{
					CComBSTR bstrName;
					m_pDLI->LayerNameGet(*i, &bstrName);
					TImageLayer tProps;
					tProps.fOpacity = 1.0f;
					m_pDLI->LayerPropsGet(*i, &tProps.eBlend, &tProps.bVisible);
					CComPtr<IConfig> pEffects;
					m_pDLI->LayerEffectGet(*i, &pEffects, NULL);
					//CComObjectStackEx<CCopier> cCopier(m_pDLI, *i);
					CImageLayerCreatorDocument cCopier(m_pDLI, *i);
					pFact->AddLayer(a_bstrID, a_pBase, TRUE, &cCopier, bstrName, &tProps, pEffects);
				}
				pFact->SetSize(a_bstrID, a_pBase, &m_tSize);
				return S_OK;
			}

		private:
			CComPtr<IDocumentLayeredImage> m_pDLI;
			TImageSize m_tSize;
			std::vector<CComPtr<IComparable> >::const_iterator m_b;
			std::vector<CComPtr<IComparable> >::const_iterator m_e;
		};

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		std::vector<CComPtr<IComparable> > m_cItems;
		CComPtr<IDesignerView> m_pView;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
		bool m_bValid;
	};
};

// {6FBB04D2-E3A2-4882-972E-1156B79CE64F}
static const GUID CLSID_MenuCommandsLayerGrouping = {0x6fbb04d2, 0xe3a2, 0x4882, {0x97, 0x2e, 0x11, 0x56, 0xb7, 0x9c, 0xe6, 0x4f}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerGrouping, CMenuCommandsLayerGrouping)
