
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"
#define AUTOCANVASSUPPORT
#include <IconRenderer.h>

extern __declspec(selectany) wchar_t const CMDNAME_CENTERSHAPESX[] = L"[0409]Center horizontally[0405]Vycentrovat horizontálně";
extern __declspec(selectany) wchar_t const CMDNAME_CENTERSHAPESY[] = L"[0409]Center vertically[0405]Vycentrovat vertikálně";
extern __declspec(selectany) wchar_t const CMDDESC_CENTERSHAPES[] = L"[0409]Move the selected object or objects into the center of the canvas.[0405]Posunout vybraný objekt nebo objekty do středu plátna.";

class ATL_NO_VTABLE CCenterShapes :
	public CDocumentMenuCommandMLImpl<CCenterShapes, 0, CMDDESC_CENTERSHAPES, NULL, 0>
{
public:
	void Init(IDocument* a_pDoc, IDocumentVectorImage* a_pDVI, IOperationContext* a_pSSM, BSTR a_bstrSyncID, BSTR a_bstrFocID, bool a_bVertical)
	{
		m_pDoc = a_pDoc;
		m_pDVI = a_pDVI;
		m_pSSM = a_pSSM;
		m_bstrSyncID = a_bstrSyncID;
		m_bstrFocID = a_bstrFocID;
		m_bVertical = a_bVertical;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			*a_ppText = new CMultiLanguageString(m_bVertical ? CMDNAME_CENTERSHAPESY : CMDNAME_CENTERSHAPESX);
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
			static GUID const vertical = {0x184b4d8c, 0xb460, 0x498e, {0x98, 0x19, 0x61, 0xa1, 0x77, 0x50, 0x36, 0x82}};
			static GUID const horizontal = {0xa67b2118, 0x8365, 0x492e, {0xb6, 0x45, 0x3d, 0xb5, 0x2c, 0x04, 0x85, 0x14}};
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
			static IRPolyPoint const vertical[] =
			{
				{0.2f, 0.4f}, {0.4f, 0.4f}, {0.5f, 0.3f}, {0.6f, 0.4f}, {0.8f, 0.4f}, {0.8f, 0.5f}, {0.6f, 0.5f}, {0.5f, 0.6f}, {0.4f, 0.5f}, {0.2f, 0.5f},
			};
			static IRPolyPoint const horizontal[] =
			{
				{0.4f, 0.2f}, {0.4f, 0.4f}, {0.3f, 0.5f}, {0.4f, 0.6f}, {0.4f, 0.8f}, {0.5f, 0.8f}, {0.5f, 0.6f}, {0.6f, 0.5f}, {0.5f, 0.4f}, {0.5f, 0.2f},
			};
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CComPtr<IIconRenderer> pIR;
			RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			*a_phIcon = IconFromPolygon(pSI, pIR, m_bVertical ? itemsof(vertical) : itemsof(horizontal), m_bVertical ? vertical : horizontal, a_nSize, true);
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
		m_pSSM->StateGet(m_bstrFocID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState)
			return EMCSNormal;
		m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> cIDs;
		if (pState)
			m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
		return cIDs.empty() ? EMCSDisabled : EMCSNormal;
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
			pState = NULL;
			m_pSSM->StateGet(m_bstrFocID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			if (cIDs.empty())
			{
				if (pState)
					m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			}
			else
			{
				std::vector<ULONG> cFocIDs;
				if (pState)
					m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cFocIDs));
				if (cFocIDs.size() == 1)
				{
					bool found = false;
					for (size_t i = 0; !found && i < cIDs.size(); ++i)
						found = cIDs[i] == cFocIDs[0];
					if (!found)
						std::swap(cFocIDs, cIDs);
				}
			}
			if (cIDs.empty())
				return S_FALSE;

			CWriteLock<IDocument> lock(m_pDoc);
			CComPtr<IDocumentImage> pDI;
			m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
			TImageSize tSize = {256, 256};
			pDI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			TPixelCoords tLT = {0, 0}, tRB = {256, 256};
			m_pDVI->ObjectBounds(cIDs[0], &tLT, &tRB);
			for (size_t i = 1; i < cIDs.size(); ++i)
			{
				TPixelCoords tLT1 = {0, 0}, tRB1 = {256, 256};
				m_pDVI->ObjectBounds(cIDs[i], &tLT1, &tRB1);
				if (tLT1.fX < tLT.fX) tLT.fX = tLT1.fX;
				if (tLT1.fY < tLT.fY) tLT.fY = tLT1.fY;
				if (tRB1.fX > tRB.fX) tRB.fX = tRB1.fX;
				if (tRB1.fY > tRB.fY) tRB.fY = tRB1.fY;
			}
			TMatrix3x3f t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			if (m_bVertical)
			{
				if (tLT.fY+tRB.fY == tSize.nY)
					return S_OK; // no transform needed
				t._32 = (tSize.nY-tLT.fY-tRB.fY)*0.5f;
			}
			else
			{
				if (tLT.fX+tRB.fX == tSize.nX)
					return S_OK; // no transform needed
				t._31 = (tSize.nX-tLT.fX-tRB.fX)*0.5f;
			}
			for (size_t i = 0; i < cIDs.size(); ++i)
				m_pDVI->ObjectTransform(cIDs[i], &t);
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
static OLECHAR const CFGID_FOCSYNCGROUP[] = L"FocusSyncGroup";


// CMenuCommandsCenterShapes

extern GUID const CLSID_MenuCommandsCenterShapes = {0xe33d5804, 0x7a8e, 0x44d4, {0x8b, 0xf, 0xfc, 0xd0, 0x64, 0x47, 0xf5, 0x87}};

class ATL_NO_VTABLE CMenuCommandsCenterShapes :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsCenterShapes, &CLSID_MenuCommandsCenterShapes>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsCenterShapes()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsCenterShapes)

BEGIN_CATEGORY_MAP(CMenuCommandsCenterShapes)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsCenterShapes)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Center objects[0405]Vektorový obrázek - vycentrovat objekty");
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
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_FOCSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CConfigValue(L"FOCUSEDSHAPE"), NULL, 0, NULL);

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
			CConfigValue cFocID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_FOCSYNCGROUP), &cFocID);

			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			CComBSTR bstrFocus = bstrState;

			bstrState.Append(cSyncID.operator BSTR());
			bstrFocus.Append(cFocID.operator BSTR());

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CCenterShapes>* p = NULL;
				CComObject<CCenterShapes>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus, false);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CCenterShapes>* p = NULL;
				CComObject<CCenterShapes>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus, true);
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

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsCenterShapes, CMenuCommandsCenterShapes)
