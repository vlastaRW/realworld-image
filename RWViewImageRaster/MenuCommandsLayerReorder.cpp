
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include "ConfigGUILayerID.h"
//#include <SharedStateUndo.h>
//#include <RenderIcon.h>


class ATL_NO_VTABLE CMenuCommandsLayerReorder :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerReorder>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerReorder()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerReorder)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerReorder)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerReorder)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Move Layer[0405]Vrstvený obrázek - přesunout vrstvu");
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

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CMove<IDS_MN_MOVELAYERUP> >* p = NULL;
				CComObject<CMove<IDS_MN_MOVELAYERUP> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(pDLI, a_pStates, bstrID);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMove<IDS_MN_MOVELAYERDOWN> >* p = NULL;
				CComObject<CMove<IDS_MN_MOVELAYERDOWN> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(pDLI, a_pStates, bstrID);
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
	template<UINT t_nNameID>
	class ATL_NO_VTABLE CMove :
		public CDocumentMenuCommandImpl<CMove<t_nNameID>, t_nNameID, IDS_MD_MOVELAYER, NULL, 0>
	{
	public:
		void Init(IDocumentLayeredImage* a_pDLI, IOperationContext* a_pSSM, BSTR a_bstrSyncID)
		{
			m_pDLI = a_pDLI;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			CComPtr<ISharedState> pState;
			m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSel;
			m_pDLI->StateUnpack(pState, &pSel);
			ULONG nSel = 0;
			pSel->Size(&nSel);
			if (nSel != 1) return EMCSDisabled;
			CComPtr<IComparable> pSelItem;
			pSel->Get(0, &pSelItem);
			if (S_OK != m_pDLI->IsLayer(pSelItem, NULL, NULL, NULL))
				return EMCSDisabled;

			CComPtr<IEnumUnknowns> pParents;
			m_pDLI->ParentsEnum(pSelItem, &pParents);
			CComPtr<IComparable> pParent;
			if (pParents) pParents->Get(0, &pParent);

			CComPtr<IEnumUnknowns> pItems;
			m_pDLI->LayersEnum(pParent, &pItems);
			ULONG nItems = 0;
			pItems->Size(&nItems);
			if (nItems <= 1) return EMCSDisabled;
			CComPtr<IComparable> pCheck;
			pItems->Get(t_nNameID == IDS_MN_MOVELAYERDOWN ? nItems-1 : 0, __uuidof(IComparable), reinterpret_cast<void**>(&pCheck));
			HRESULT hRes = pSelItem->Compare(pCheck);
			return SUCCEEDED(hRes) && hRes != S_OK ? EMCSNormal : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<ISharedState> pState;
				m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				CComPtr<IEnumUnknowns> pSel;
				m_pDLI->StateUnpack(pState, &pSel);
				if (pSel == NULL) return E_FAIL;
				CComPtr<IComparable> pSelItem;
				pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pSelItem));
				if (pSelItem == NULL) return E_FAIL;

				CComPtr<IEnumUnknowns> pParents;
				m_pDLI->ParentsEnum(pSelItem, &pParents);
				CComPtr<IComparable> pParent;
				if (pParents) pParents->Get(0, &pParent);

				CComPtr<IEnumUnknowns> pItems;
				m_pDLI->LayersEnum(pParent, &pItems);

				ULONG nItems = 0;
				pItems->Size(&nItems);
				if (nItems <= 1) return E_FAIL;
				for (ULONG i = 0; i < nItems; ++i)
				{
					CComPtr<IComparable> pTmp;
					pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pTmp));
					if (S_OK == pSelItem->Compare(pTmp))
					{
						pTmp = NULL;
						if (t_nNameID == IDS_MN_MOVELAYERDOWN)
						{
							if (i == nItems-1) return E_FAIL; // already last item
							pItems->Get(i+1, __uuidof(IComparable), reinterpret_cast<void**>(&pTmp));
							return m_pDLI->LayerMove(pSelItem, pTmp, ELIPBelow);
						}
						else
						{
							if (i == 0) return E_FAIL; // already first item
							pItems->Get(i-1, __uuidof(IComparable), reinterpret_cast<void**>(&pTmp));
							return m_pDLI->LayerMove(pSelItem, pTmp, ELIPAbove);
						}
					}
				}
				return E_RW_ITEMNOTFOUND;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
	};
};

// {1BC8D240-9DCB-4655-A04E-F407DC5DFC5A}
static const GUID CLSID_MenuCommandsLayerReorder = {0x1bc8d240, 0x9dcb, 0x4655, {0xa0, 0x4e, 0xf4, 0x7, 0xdc, 0x5d, 0xfc, 0x5a}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerReorder, CMenuCommandsLayerReorder)
