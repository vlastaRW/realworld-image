
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"
#define AUTOCANVASSUPPORT
#include <IconRenderer.h>

extern __declspec(selectany) wchar_t const CMDNAME_DISTRIBUTESHAPESX[] = L"[0409]Distribute horizontally[0405]Rozmístit horizontálně";
extern __declspec(selectany) wchar_t const CMDNAME_DISTRIBUTESHAPESY[] = L"[0409]Distribute vertically[0405]Rozmístit vertikálně";
extern __declspec(selectany) wchar_t const CMDDESC_DISTRIBUTESHAPES[] = L"[0409]Make the spacing between three or more objects equal.[0405]Posunout tři nebo více objektů tak, aby meni nimi byly stejné mezery.";

class ATL_NO_VTABLE CDistributeShapes :
	public CDocumentMenuCommandMLImpl<CDistributeShapes, 0, CMDDESC_DISTRIBUTESHAPES, NULL, 0>
{
	struct less_second
	{
		bool operator()(std::pair<ULONG, float> l, std::pair<ULONG, float> r) const
		{ return l.second < r.second; }
	};

public:
	void Init(IDocument* a_pDoc, IDocumentVectorImage* a_pDVI, IOperationContext* a_pSSM, BSTR a_bstrSyncID, bool a_bVertical)
	{
		m_pDoc = a_pDoc;
		m_pDVI = a_pDVI;
		m_pSSM = a_pSSM;
		m_bstrSyncID = a_bstrSyncID;
		m_bVertical = a_bVertical;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			*a_ppText = new CMultiLanguageString(m_bVertical ? CMDNAME_DISTRIBUTESHAPESY : CMDNAME_DISTRIBUTESHAPESX);
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			static GUID const vertical = {0x170e9ea9, 0xc946, 0x4464, {0xb5, 0x55, 0xb3, 0xf6, 0xf8, 0x6b, 0x78, 0xff}};
			static GUID const horizontal = {0x31fc2da0, 0xc8e9, 0x437a, {0xaa, 0xd2, 0xcd, 0x4b, 0x61, 0x09, 0x52, 0x66}};
			*a_pIconID = m_bVertical ? vertical : horizontal;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			static IRPolyPoint const vertical1[] = { {0.25f, 0.1f}, {0.75f, 0.1f}, {0.75f, 0.3f}, {0.25f, 0.3f} };
			static IRPolyPoint const vertical2[] = { {0.25f, 0.4f}, {0.75f, 0.4f}, {0.75f, 0.6f}, {0.25f, 0.6f} };
			static IRPolyPoint const vertical3[] = { {0.25f, 0.7f}, {0.75f, 0.7f}, {0.75f, 0.9f}, {0.25f, 0.9f} };
			static IRPolygon const vertical[] = { {itemsof(vertical1), vertical1}, {itemsof(vertical2), vertical2}, {itemsof(vertical3), vertical3} };
			static IRPolyPoint const horizontal1[] = { {0.1f, 0.25f}, {0.1f, 0.75f}, {0.3f, 0.75f}, {0.3f, 0.25f} };
			static IRPolyPoint const horizontal2[] = { {0.4f, 0.25f}, {0.4f, 0.75f}, {0.6f, 0.75f}, {0.6f, 0.25f} };
			static IRPolyPoint const horizontal3[] = { {0.7f, 0.25f}, {0.7f, 0.75f}, {0.9f, 0.75f}, {0.9f, 0.25f} };
			static IRPolygon const horizontal[] = { {itemsof(horizontal1), horizontal1}, {itemsof(horizontal2), horizontal2}, {itemsof(horizontal3), horizontal3} };
			static IRGridItem const grid1[] = { {0, 0.1f}, {1, 0.3f}, {2, 0.4f}, {1, 0.6f}, {2, 0.7f}, {1, 0.9f} };
			static IRGridItem const grid2[] = { {0, 0.25f}, {0, 0.75f} };
			static IRCanvas const canvasH = {0.05f, 0.25f, 0.95f, 0.75f, itemsof(grid1), itemsof(grid2), grid1, grid2};
			static IRCanvas const canvasV = {0.25f, 0.05f, 0.75f, 0.95f, itemsof(grid2), itemsof(grid1), grid2, grid1};
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			if (m_bVertical)
				cRenderer(&canvasV, itemsof(vertical), vertical, pSI->GetMaterial(ESMInterior));
			else
				cRenderer(&canvasH, itemsof(horizontal), horizontal, pSI->GetMaterial(ESMInterior));
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	EMenuCommandState IntState()
	{
		CComPtr<ISharedState> pState;
		m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> cIDs;
		if (pState)
			m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
		return cIDs.size() > 1 ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<ISharedState> pState;
			m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> cIDs;
			if (pState)
				m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				m_pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
			{
				pState = NULL;
				m_pSSM->StateGet(m_bstrFocID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				if (pState)
					m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			}
			if (cIDs.empty())
				return S_FALSE;

			CWriteLock<IDocument> lock(m_pDoc);
			TPixelCoords tLT = {0, 0}, tRB = {256, 256};
			m_pDVI->ObjectBounds(cIDs[0], &tLT, &tRB);
			TPixelCoords tSize = {tRB.fX-tLT.fX, tRB.fY-tLT.fY};
			std::vector<std::pair<ULONG, float> > cMids(cIDs.size());
			cMids[0].first = cIDs[0];
			cMids[0].second = m_bVertical ? tLT.fY+tRB.fY : tLT.fX+tRB.fX;
			for (size_t i = 1; i < cIDs.size(); ++i)
			{
				TPixelCoords tLT1 = {0, 0}, tRB1 = {256, 256};
				m_pDVI->ObjectBounds(cIDs[i], &tLT1, &tRB1);
				cMids[i].first = cIDs[i];
				cMids[i].second = m_bVertical ? tLT1.fY+tRB1.fY : tLT1.fX+tRB1.fX;
				if (tLT1.fX < tLT.fX) tLT.fX = tLT1.fX;
				if (tLT1.fY < tLT.fY) tLT.fY = tLT1.fY;
				if (tRB1.fX > tRB.fX) tRB.fX = tRB1.fX;
				if (tRB1.fY > tRB.fY) tRB.fY = tRB1.fY;
				tSize.fX += tRB1.fX-tLT1.fX;
				tSize.fY += tRB1.fY-tLT1.fY;
			}
			std::stable_sort(cMids.begin(), cMids.end(), less_second());
			TMatrix3x3f t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			float f = m_bVertical ? tLT.fY : tLT.fX;
			float extra = m_bVertical ? tRB.fY-tLT.fY-tSize.fY : tRB.fX-tLT.fX-tSize.fX;
			for (size_t i = 0; i < cMids.size(); ++i)
			{
				TPixelCoords tLT1 = {0, 0}, tRB1 = {256, 256};
				m_pDVI->ObjectBounds(cMids[i].first, &tLT1, &tRB1);
				if (m_bVertical)
				{
					float y = f+extra/(cMids.size()-1)*i;
					t._32 = y-tLT1.fY;
					m_pDVI->ObjectTransform(cMids[i].first, &t);
					f += tRB1.fY-tLT1.fY;
				}
				else
				{
					float x = f+extra/(cMids.size()-1)*i;
					t._31 = x-tLT1.fX;
					m_pDVI->ObjectTransform(cMids[i].first, &t);
					f += tRB1.fX-tLT1.fX;
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentVectorImage> m_pDVI;
	CComPtr<IOperationContext> m_pSSM;
	CComBSTR m_bstrSyncID;
	CComBSTR m_bstrFocID;
	bool m_bVertical;
};


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";


// CMenuCommandsDistributeShapes

extern GUID const CLSID_MenuCommandsDistributeShapes = {0x8f65497f, 0x7057, 0x4f6a, {0xa5, 0xd5, 0x62, 0x95, 0xe7, 0x37, 0x14, 0x34}};

class ATL_NO_VTABLE CMenuCommandsDistributeShapes :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsDistributeShapes, &CLSID_MenuCommandsDistributeShapes>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsDistributeShapes()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsDistributeShapes)

BEGIN_CATEGORY_MAP(CMenuCommandsDistributeShapes)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsDistributeShapes)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Distribute objects[0405]Vektorový obrázek - rozmístit objekty");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);

			// finalize the initialization of the config
			pCfgInit->Finalize(NULL);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			if (a_ppSubCommands == NULL) return E_POINTER;

			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return E_FAIL;

			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);

			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);

			bstrState.Append(cSyncID.operator BSTR());

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CDistributeShapes>* p = NULL;
				CComObject<CDistributeShapes>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, false);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDistributeShapes>* p = NULL;
				CComObject<CDistributeShapes>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, true);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsDistributeShapes, CMenuCommandsDistributeShapes)
