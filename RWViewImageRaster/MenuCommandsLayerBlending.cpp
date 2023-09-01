
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <IconRenderer.h>

// {AD03A282-20D5-4cea-9294-E3D1FA53C480}
static const GUID CLSID_MenuCommandsLayerBlending = {0xad03a282, 0x20d5, 0x4cea, {0x92, 0x94, 0xe3, 0xd1, 0xfa, 0x53, 0xc4, 0x80}};

class ATL_NO_VTABLE CMenuCommandsLayerBlending :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerBlending>,
	public IDesignerFrameIcons,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerBlending()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerBlending)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerBlending)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerBlending)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Blending Modes[0405]Vrstvený obrázek - míchací módy");
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
			if (pItems == NULL)
				return S_FALSE;

			bool bFound = false;
			int iActive = -1;
			ULONG nItems = 0;
			if (pItems) pItems->Size(&nItems);
			for (ULONG i = 0; i < nItems; ++i)
			{
				CComPtr<IComparable> pItem;
				pItems->Get(i, &pItem);
				ELayerBlend eBlend = EBEAlphaBlend;
				CComPtr<IComparable> pLayer;
				if (SUCCEEDED(pDLI->LayerPropsGet(pItem, &eBlend, NULL)) ||
					(SUCCEEDED(pDLI->LayerFromEffect(pItem, &pLayer)) && pLayer && SUCCEEDED(pDLI->LayerPropsGet(pLayer, &eBlend, NULL))))
				{
					bFound = true;
					if (iActive == -1)
						iActive = eBlend;
					else if (iActive != eBlend)
					{
						iActive = -2;
						break;
					}
				}
			}

			//if (iActive == -1)
			//	return S_FALSE; // no layer selected

			static ELayerBlend const s_aModes[] =
			{
				EBEAlphaBlend,
				EBEModulate,
				EBEScreen,
				EBEAdd,
				EBESubtract,
				EBEAverage,
				EBEDifference,
				EBEMinimum,
				EBEMaximum,
				EBEOverlay,
				ELBHLSReplaceHue,
				ELBHLSReplaceSaturation,
				ELBHLSReplaceLuminance,
				ELBHLSReplaceColor,
				EBEMultiplyInvAlpha,
			};

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			for (ELayerBlend const* p = s_aModes; p < s_aModes+sizeof(s_aModes)/sizeof(*s_aModes); ++p)
			{
				CComObject<CSetMode>* pSM = NULL;
				CComObject<CSetMode>::CreateInstance(&pSM);
				CComPtr<IDocumentMenuCommand> pCmd = pSM;
				pSM->Init(a_pDocument, pDLI, pItems, *p, iActive != -1, iActive == *p, L"[0409]Normal|Multiply|Screen|Add|Subtract|Average|Difference|Minimum (darken)|Maximum (lighten)|Overlay|Replace hue|Replace saturation|Replace brightness|Replace color|Silhouette[0405]Normální|Vynásobit|Vynásobit negativ|Sečíst|Odečíst|Průměr|Rozdíl|Minimum (ztmavit)|Maximum (zesvětlit)|Překryv|Nahradit odstín|Nahradit saturaci|Nahradit světlost|Nahradit barvu|Silueta", p-s_aModes);

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

	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp)
	{
		if (a_pTimeStamp == NULL) return E_POINTER;
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
			pTmp->Insert(CLSID_MenuCommandsLayerBlending);
			*a_ppIDs = pTmp;
			pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON *a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			static IRPathPoint const top[] =
			{
				{0, 24, 0, -13.2548, 0, 0},
				{24, 0, 0, 0, -13.2548, 0},
				{168, 0, 13.2548, 0, 0, 0},
				{192, 24, 0, 0, 0, -13.2548},
				{192, 168, 0, 13.2548, 0, 0},
				{168, 192, 0, 0, 13.2548, 0},
				{24, 192, -13.2548, 0, 0, 0},
				{0, 168, 0, 0, 0, 13.2548},
			};
			static IRPathPoint const bot[] =
			{
				{64, 88, 0, -13.2548, 0, 0},
				{88, 64, 0, 0, -13.2548, 0},
				{232, 64, 13.2548, 0, 0, 0},
				{256, 88, 0, 0, 0, -13.2548},
				{256, 232, 0, 13.2548, 0, 0},
				{232, 256, 0, 0, 13.2548, 0},
				{88, 256, -13.2548, 0, 0, 0},
				{64, 232, 0, 0, 0, 13.2548},
			};
			static IRGridItem const grid[] = { {0, 0}, {0, 64}, {0, 192}, {0, 256} };
			static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
			IRFill redFill(0xc0000000|(0xffffff&pSI->GetSRGBColor(ESMDelete)));
			IRFill blueFill(0xc0000000|(0xffffff&pSI->GetSRGBColor(ESMManipulate)));
			IROutlinedFill red(&redFill, pSI->GetMaterial(ESMContrast));
			IROutlinedFill blue(&blueFill, pSI->GetMaterial(ESMContrast));
			cRenderer(&canvas, itemsof(top), top, &blue);
			cRenderer(&canvas, itemsof(bot), bot, &red);
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CSetMode :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand,
		public ILocalizedString
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pItems, ELayerBlend a_eBlend, bool a_bEnabled, bool a_bActive, wchar_t const* a_pszNames, int a_iName)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pItems = a_pItems;
			m_eBlend = a_eBlend;
			m_bEnabled = a_bEnabled;
			m_bActive = a_bActive;
			m_pszNames = a_pszNames;
			m_iName = a_iName;
		}

	BEGIN_COM_MAP(CSetMode)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			*a_ppText = this;
			AddRef();
			return S_OK;
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]Select how to merge the current layer with the underlying layers.[0405]Změnit způsob, jak smíchat aktuální vrstvu s vrstvami ležícími pod ní.");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			return E_NOTIMPL;
		}
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
				*a_peState = m_bEnabled ? (m_bActive ? EMCSRadioChecked : EMCSRadio) : EMCSDisabledRadio;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
		{
			ULONG nItems = 0;
			if (m_pItems) m_pItems->Size(&nItems);
			if (nItems == 0)
				return S_FALSE;
			CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(L"[0409]Change layer blending[0405]Změnit míchání vrstvy"));
			for (ULONG i = 0; i < nItems; ++i)
			{
				CComPtr<IComparable> pItem;
				m_pItems->Get(i, &pItem);
				CComPtr<IComparable> pExtra;
				if (SUCCEEDED(m_pDLI->LayerFromEffect(pItem, &pExtra)) && pExtra)
					std::swap(pExtra.p, pItem.p);
				m_pDLI->LayerPropsSet(pItem, &m_eBlend, NULL);
			}
			return S_OK;
		}

		// ILocalizedString methods
		STDMETHOD(Get)(BSTR* a_pbstrString)
		{
			return GetLocalized(GetThreadLocale(), a_pbstrString);
		}
		STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
		{
			if (a_pbstrString == NULL)
				return E_POINTER;
			try
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(m_pszNames, a_tLCID, &bstr);
				wchar_t const* p = bstr;
				for (int i = 0; i < m_iName; ++i)
				{
					wchar_t const* pBar = wcschr(p, _T('|'));
					if (!pBar)
						return E_FAIL;
					p = pBar+1;
				}
				wchar_t const* pEnd = wcschr(p, _T('|'));
				// leak if exception
				*a_pbstrString = pEnd ? ::SysAllocStringLen(p, pEnd-p) : ::SysAllocString(p);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IEnumUnknowns> m_pItems;
		ELayerBlend m_eBlend;
		bool m_bEnabled;
		bool m_bActive;
		wchar_t const* m_pszNames;
		int m_iName;
	};
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerBlending, CMenuCommandsLayerBlending)
