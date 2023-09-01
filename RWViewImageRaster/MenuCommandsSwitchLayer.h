// MenuCommandsSwitchLayer.h : Declaration of the CMenuCommandsSwitchLayer

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <IconRenderer.h>


// CMenuCommandsSwitchLayer

class ATL_NO_VTABLE CMenuCommandsSwitchLayer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsSwitchLayer, &CLSID_MenuCommandsSwitchLayer>,
	public IDesignerFrameIcons,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsSwitchLayer()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsSwitchLayer)

BEGIN_CATEGORY_MAP(CMenuCommandsSwitchLayer)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsSwitchLayer)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp)
	{
		*a_pTimeStamp = 0;
		return S_OK;
	}
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs)
	{
		if (a_ppIDs == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IEnumGUIDsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
			pTmp->Insert(CLSID_MenuCommandsSwitchLayer);
			*a_ppIDs = pTmp;
			pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			if (!IsEqualGUID(a_tIconID, CLSID_MenuCommandsSwitchLayer))
				return E_RW_ITEMNOTFOUND;

			static IRPolyPoint const aBack[] = { {0, 36}, {220, 36}, {220, 256}, {0, 256}, };
			static IRPolyPoint const aFront[] = { {36, 161}, {36, 0}, {256, 0}, {256, 220}, {95, 220}, };
			static IRPolyPoint const aCorner[] = { {44, 155}, {101, 155}, {101, 212}, };
			static IRPolygon const tBack = {itemsof(aBack), aBack};
			static IRPolygon const tFront = {itemsof(aFront), aFront};
			static IRPolygon const tCorner = {itemsof(aCorner), aCorner};
			static IRGridItem const tGridBackX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 220.0f}};
			static IRGridItem const tGridBackY[] = { {EGIFInteger, 36.0f}, {EGIFInteger, 256.0f}};
			static IRCanvas const tCanvasBack = {0, 0, 256, 256, itemsof(tGridBackX), itemsof(tGridBackY), tGridBackX, tGridBackY};
			static IRGridItem const tGridFrontX[] = { {EGIFInteger, 36.0f}, {EGIFMidPixel, 101.0f}, {EGIFInteger, 256.0f}};
			static IRGridItem const tGridFrontY[] = { {EGIFInteger, 0.0f}, {EGIFMidPixel, 155.0f}, {EGIFInteger, 220.0f}};
			static IRCanvas const tCanvasFront = {0, 0, 256, 256, itemsof(tGridFrontX), itemsof(tGridFrontY), tGridFrontX, tGridFrontY};
			static IRTarget const tTarget(0.92f);

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&tCanvasBack, 1, &tBack, pSI->GetMaterial(ESMAltBackground), &tTarget);
			cRenderer(&tCanvasFront, 1, &tFront, pSI->GetMaterial(ESMBackground), &tTarget);
			cRenderer(&tCanvasFront, 1, &tCorner, pSI->GetMaterial(ESMOutlineSoft), &tTarget);
			*a_phIcon = cRenderer.get();

			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CSwitchCommand : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand,
		public ILocalizedString
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrID, IDocumentLayeredImage* a_pLI, IComparable* a_pID, IStructuredItemsRichGUI* a_pRG, bool a_bDefault)
		{
			m_pStates = a_pStates;
			m_bstrID = a_bstrID;
			m_pLI = a_pLI;
			m_pID = a_pID;
			m_pRG = a_pRG;
			m_bDefault = a_bDefault;
		}

	BEGIN_COM_MAP(CSwitchCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

		// ILocalizedString methods
	public:
		STDMETHOD(Get)(BSTR* a_pbstrString) { return m_pLI->LayerNameGet(m_pID, a_pbstrString); }
		STDMETHOD(GetLocalized)(LCID UNREF(a_tLCID), BSTR* a_pbstrString) { return Get(a_pbstrString); }

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = this;
				AddRef();
				return S_OK;
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				CComPtr<ISharedState> pState;
				m_pStates->StateGet(m_bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				CComPtr<IEnumUnknowns> pItems;
				if (pState) m_pLI->StateUnpack(pState, &pItems);
				ULONG nItems = 0;
				if (pItems) pItems->Size(&nItems);
				for (ULONG i = 0; i < nItems; ++i)
				{
					CComPtr<IComparable> pItem;
					pItems->Get(i, &pItem);
					if (m_pID->Compare(pItem) == S_OK)
					{
						*a_peState = EMCSRadioChecked;
						return S_OK;
					}
				}
				*a_peState = nItems == 0 && m_bDefault ? EMCSRadioChecked : EMCSNormal;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
		{
			CComPtr<ISharedState> pState;
			m_pLI->StatePack(1, &(m_pID.p), &pState);
			return m_pStates->StateSet(m_bstrID, pState);
		}

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrID;
		CComPtr<IDocumentLayeredImage> m_pLI;
		CComPtr<IComparable> m_pID;
		CComPtr<IStructuredItemsRichGUI> m_pRG;
		bool m_bDefault;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsSwitchLayer), CMenuCommandsSwitchLayer)
