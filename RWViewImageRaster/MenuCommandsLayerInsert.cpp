
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <PlugInCache.h>
#include <WeakSingleton.h>
#include <SharedStateUndo.h>
#include <IconRenderer.h>


static OLECHAR const CFGID_PRESENTMODE[] = L"Mode";
static LONG const CFGVAL_PM_INPLACE = 0;
static LONG const CFGVAL_PM_SUBMENU = 1;
static LONG const CFGVAL_PM_COMBINED_AUTO = 2;
static LONG const CFGVAL_PM_COMBINED_LAST = 3;
static LONG const CFGVAL_PM_COMBINED_FIXED = 4;
static LONG const CFGVAL_PM_SINGLE = 10;
static OLECHAR const CFGID_SPECIFICTYPE[] = L"LayerType";

class ATL_NO_VTABLE CMenuCommandsLayerInsert :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerInsert>,
	public IDocumentMenuCommands,
	public IConfigItemCustomOptions,
	private CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>
{
public:
	CMenuCommandsLayerInsert()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsLayerInsert)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerInsert)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerInsert)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
	COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
END_COM_MAP()


	// IConfigItemCustomOptions methods
public:
	STDMETHOD(GetValueName)(TConfigValue const* a_pValue, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			if (IsEqualGUID(a_pValue->guidVal, GUID_NULL))
			{
				*a_ppName = new CMultiLanguageString(L"[0409]Not specified[0405]Nespecifikováno");
				return S_OK;
			}
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = Map();
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.find(a_pValue->guidVal);
			if (i != cMap.end() && i->second != NULL)
				return i->second->LayerName(FALSE, a_ppName);
			return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IEnumConfigItemOptions methods
public:
	STDMETHOD(Size)(ULONG* a_pnSize)
	{
		if (a_pnSize == NULL)
			return E_POINTER;
		try
		{
			*a_pnSize = 1;
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = Map();
			for (CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.begin(); i != cMap.end(); ++i)
			{
				if (i->second == NULL)
					continue; // TODO: fix plugin cache to not include items without requested interface
				++*a_pnSize;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
	{
		if (a_ptItem == NULL)
			return E_POINTER;
		try
		{
			a_ptItem->eTypeID = ECVTEmpty;
			if (a_nIndex == 0)
			{
				a_ptItem->eTypeID = ECVTGUID;
				a_ptItem->guidVal = GUID_NULL;
				return S_OK;
			}
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = Map();
			for (CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.begin(); i != cMap.end(); ++i)
			{
				if (i->second == NULL)
					continue; // TODO: fix plugin cache to not include items without requested interface
				--a_nIndex;
				if (a_nIndex == 0)
				{
					a_ptItem->eTypeID = ECVTGUID;
					a_ptItem->guidVal = i->first;
					return S_OK;
				}
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
	{
		if (a_atItems == NULL)
			return E_POINTER;
		try
		{
			if (a_nIndexFirst == 0)
			{
				a_atItems->eTypeID = ECVTGUID;
				a_atItems->guidVal = GUID_NULL;
				++a_atItems;
				++a_nIndexFirst;
				--a_nCount;
			}
			if (a_nCount == 0)
				return S_OK;
			--a_nIndexFirst;
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = Map();
			for (CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.begin(); i != cMap.end(); ++i)
			{
				if (i->second == NULL)
					continue; // TODO: fix plugin cache to not include items without requested interface
				if (a_nIndexFirst == 0)
				{
					a_atItems->eTypeID = ECVTGUID;
					a_atItems->guidVal = i->first;
					++a_atItems;
					++a_nIndexFirst;
					--a_nCount;
					if (a_nCount == 0)
						return S_OK;
				}
				else
					--a_nIndexFirst;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Create Layer[0405]Vrstvený obrázek - vytvořit vrstvu");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"LAYER"), NULL, 0, NULL);
			CComBSTR bstrMode(CFGID_PRESENTMODE);
			pCfgInit->ItemIns1ofN(bstrMode, CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), NULL, CConfigValue(CFGVAL_PM_INPLACE), NULL);
			pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_INPLACE), CMultiLanguageString::GetAuto(L"[0409]Direct[0405]Seznam"), 0, NULL);
			pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_SUBMENU), CMultiLanguageString::GetAuto(L"[0409]Menu[0405]Menu"), 0, NULL);
			//pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_COMBINED_AUTO), CMultiLanguageString::GetAuto(L"[0409]Quick - auto[0405]Zrychleně - automaticky"), 0, NULL);
			pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_COMBINED_LAST), CMultiLanguageString::GetAuto(L"[0409]Last option + menu[0405]Minulá volba + menu"), 0, NULL);
			pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_COMBINED_FIXED), CMultiLanguageString::GetAuto(L"[0409]Fixed option + menu[0405]Pevná volba + menu"), 0, NULL);
			pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_PM_SINGLE), CMultiLanguageString::GetAuto(L"[0409]Fixed option[0405]Pevná volba"), 0, NULL);
			TConfigOptionCondition tCond;
			tCond.bstrID = bstrMode;
			tCond.eConditionType = ECOCGreaterEqual;
			tCond.tValue = CConfigValue(CFGVAL_PM_COMBINED_LAST);
			pCfgInit->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_SPECIFICTYPE), CMultiLanguageString::GetAuto(L"[0409]Layer type[0405]Typ vrstvy"), NULL, CConfigValue(GUID_NULL), this, NULL, 1, &tCond);
			pCfgInit->Finalize(NULL);
			//CConfigCustomGUI<&__uuidof(CConfigGUILayerIDDlg), CConfigGUILayerIDDlg>::FinalizeConfig(pCfgInit);

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

			CConfigValue cMode;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PRESENTMODE), &cMode);

			CConfigValue cItem;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SPECIFICTYPE), &cItem);

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			switch (cMode.operator LONG())
			{
			case CFGVAL_PM_INPLACE:
				InsertAllPlugIns(a_pView, a_pDocument, pDLI, a_pStates, bstrID, true, NULL, pCmds);
				break;
			case CFGVAL_PM_SUBMENU:
				{
					CComObject<CSubMenu>* p = NULL;
					CComObject<CSubMenu>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pCmd = p;
					p->Init(this, a_pView, a_pDocument, pDLI, a_pStates, bstrID, cMode, NULL, NULL);
					pCmds->Insert(pCmd);
				}
				break;
			case CFGVAL_PM_COMBINED_LAST:
			case CFGVAL_PM_COMBINED_FIXED:
			case CFGVAL_PM_COMBINED_AUTO:
				{
					CComPtr<IImageLayerFactory> pILF;
					if (!IsEqualGUID(GUID_NULL, cItem))
						RWCoCreateInstance(pILF, cItem);
					CComObject<CSubMenu>* p = NULL;
					CComObject<CSubMenu>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pCmd = p;
					p->Init(this, a_pView, a_pDocument, pDLI, a_pStates, bstrID, cMode, a_pConfig, pILF);
					pCmds->Insert(pCmd);
				}
				break;
			case CFGVAL_PM_SINGLE:
				if (!IsEqualGUID(GUID_NULL, cItem))
				{
					CComPtr<IImageLayerFactory> pILF;
					RWCoCreateInstance(pILF, cItem);
					if (pILF)
					{
						CComObject<CInsert>* p = NULL;
						CComObject<CInsert>::CreateInstance(&p);
						CComPtr<IDocumentMenuCommand> pCmd = p;
						p->Init(a_pView, a_pDocument, pDLI, a_pStates, bstrID, cItem, pILF, true, NULL);
						pCmds->Insert(pCmd);
					}
				}
				break;
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
	void InsertAllPlugIns(IDesignerView* a_pView, IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pStates, BSTR bstrID, bool a_bWithMangling, IConfig* a_pConfig, IEnumUnknownsInit* a_pCmds)
	{
		ObjectLock cLock(this);
		CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = Map();
		for (CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.begin(); i != cMap.end(); ++i)
		{
			if (i->second == NULL)
				continue; // TODO: fix plugin cache to not include items without requested interface
			CComObject<CInsert>* p = NULL;
			CComObject<CInsert>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pCmd = p;
			p->Init(a_pView, a_pDocument, a_pDLI, a_pStates, bstrID, i->first, i->second, a_bWithMangling, a_pConfig);
			a_pCmds->Insert(pCmd);
		}
	}

	class ATL_NO_VTABLE CInsert :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IDesignerView* a_pView, IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pSSM, BSTR a_bstrSyncID, GUID a_tFactoryID, IImageLayerFactory* a_pFactory, bool a_bWithMangling, IConfig* a_pConfig)
		{
			m_pView = a_pView;
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
			m_tFactoryID = a_tFactoryID;
			m_pFactory = a_pFactory;
			m_bWithMangling = a_bWithMangling;
			m_pConfig = a_pConfig;
		}

	BEGIN_COM_MAP(CInsert)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			return m_pFactory->LayerName(m_bWithMangling, a_ppText);
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]Insert a new layer atop the currently selected one.[0405]Vložit novou vrstvu nad právě vybranou.");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (m_bWithMangling)
			{
				// {5684EB46-FD49-496b-95A8-336F0D1E6742}
				static const GUID tXOR = {0x5684eb46, 0xfd49, 0x496b, {0x95, 0xa8, 0x33, 0x6f, 0xd, 0x1e, 0x67, 0x42}};
				GUID tSub = GUID_NULL;
				HRESULT hRes = m_pFactory->LayerIconID(&tSub);
				if (FAILED(hRes)) return hRes;
				a_pIconID->Data1 = tXOR.Data1^tSub.Data1;
				a_pIconID->Data2 = tXOR.Data1^tSub.Data2;
				a_pIconID->Data3 = tXOR.Data1^tSub.Data3;
				for (int i = 0; i < 8; ++i)
					a_pIconID->Data4[i] = tXOR.Data4[i]^tSub.Data4[i];
				return S_OK;
			}
			return m_pFactory->LayerIconID(a_pIconID);
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (!m_bWithMangling)
				return m_pFactory->LayerIcon(a_nSize, a_phIcon);

			CIconRendererReceiver cRenderer(a_nSize);

			HICON hLayer = NULL;
			HRESULT hRes = m_pFactory->LayerIcon(a_nSize*4/5, &hLayer);
			if (FAILED(hRes)) return hRes;

			ICONINFO tInfo;
			ZeroMemory(&tInfo, sizeof tInfo);
			if (hLayer)
				GetIconInfo(hLayer, &tInfo);
			if (tInfo.hbmMask)
				DeleteObject(tInfo.hbmMask);
			if (tInfo.hbmColor)
			{
				BITMAP tBmp;
				ZeroMemory(&tBmp, sizeof tBmp);
				GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
				if (tBmp.bmBitsPixel == 32)
				{
					int nSmSizeX = tBmp.bmWidth;
					int nSmSizeY = tBmp.bmHeight;
					DWORD nXOR = nSmSizeY*nSmSizeX<<2;
					DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
					CAutoVectorPtr<BYTE> pInterRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
					ZeroMemory(pInterRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

					if (GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pInterRes.m_p+sizeof BITMAPINFOHEADER))
					{
						DWORD *pSrc = reinterpret_cast<DWORD *>(pInterRes.m_p+sizeof BITMAPINFOHEADER);
						if (nSmSizeX <= int(a_nSize) && nSmSizeY < int(a_nSize))
						{
							for (int y = 0; y < nSmSizeY; ++y)
								CopyMemory(cRenderer.pixelRow(y+a_nSize-nSmSizeY), pSrc+y*nSmSizeX, nSmSizeX<<2);
						}
					}
				}

				DeleteObject(tInfo.hbmColor);
			}
			DestroyIcon(hLayer);

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.65f, 1, -1));
			*a_phIcon = cRenderer.get();

			return hRes;
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return m_pFactory->LayerAccelerator(a_pAccel);
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			if (a_peState == NULL)
				return E_POINTER;
			*a_peState = EMCSNormal;
			return S_OK;
		}
		STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID a_tLocaleID)
		{
			try
			{
				if (m_pView) m_pView->DeactivateAll(FALSE);

				CComPtr<IDocumentImage> pDI;
				m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
				TImageSize tSize1;
				pDI->CanvasGet(&tSize1, NULL, NULL, NULL, NULL);
				CComPtr<IComparable> pItem;
				CWriteLock<IDocument> cLock(m_pDoc.p);
				CUndoBlock cUndo(m_pDoc.p, _SharedStringTable.GetStringAuto(IDS_MN_APPENDLAYER));

				// insertion point
				CComPtr<IComparable> pWhere; // create new layer on top if something weird is selected
				{
					CComPtr<ISharedState> pState;
					m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
					CComPtr<IEnumUnknowns> pSel;
					m_pDLI->StateUnpack(pState, &pSel);
					if (pSel)
					{
						ULONG nSize = 0;
						pSel->Size(&nSize);
						if (nSize == 1)
						{
							pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pWhere));
							if (m_pDLI->IsLayer(pWhere, NULL, NULL, NULL) != S_OK)
								pWhere = NULL;
						}
					}
				}

				CComPtr<IImageLayerCreator> pCreator;
				HRESULT hRes = m_pFactory->LayerCreatorGet(tSize1, NULL, &pCreator);

				if (SUCCEEDED(hRes))
					hRes = m_pDLI->LayerInsert(pWhere, ELIPDefault, pCreator, &pItem);
				if (SUCCEEDED(hRes) && pItem)
				{
					CComBSTR bstrPrefix;
					CMultiLanguageString::GetLocalized(L"[0409]layer[0405]vrstva", a_tLocaleID, &bstrPrefix);
					bstrPrefix += L" ";
					int max = -1;
					CComPtr<IEnumUnknowns> pParents;
					m_pDLI->ParentsEnum(pItem, &pParents);
					CComPtr<IComparable> pParent;
					if (pParents) pParents->Get(0, &pParent);
					CComPtr<IEnumUnknowns> pSiblings;
					m_pDLI->LayersEnum(pParent, &pSiblings);
					ULONG nSiblings = 0;
					if (pSiblings) pSiblings->Size(&nSiblings);
					for (ULONG i = 0; i < nSiblings; ++i)
					{
						CComPtr<IComparable> p;
						pSiblings->Get(i, &p);
						if (p && S_OK != p->Compare(pItem))
						{
							CComBSTR bstr;
							m_pDLI->LayerNameGet(p, &bstr);
							if (bstr.Length() > bstrPrefix.Length() && wcsncmp(bstr, bstrPrefix, bstrPrefix.Length()) == 0)
							{
								int n = _wtoi(bstr.m_str+bstrPrefix.Length());
								if (n != 0 || bstr.m_str[bstrPrefix.Length()] == L'0')
								{
									if (n > max) max = n;
								}
							}
						}
					}

					wchar_t sz[64] = L"";
					wsprintf(sz, L"%i", max+1);
					bstrPrefix += sz;
					m_pDLI->LayerNameSet(pItem, bstrPrefix);

					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID);
					CComPtr<ISharedState> pState;
					m_pDLI->StatePack(1, &(pItem.p), &pState);
					m_pSSM->StateSet(m_bstrSyncID, pState);
				}
				if (m_pConfig)
				{
					CComBSTR bstr(CFGID_SPECIFICTYPE);
					CConfigValue cID(m_tFactoryID);
					m_pConfig->ItemValuesSet(1, &(bstr.m_str), cID);
				}
				return hRes;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	protected:
		CComPtr<IDesignerView> m_pView;
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
		GUID m_tFactoryID;
		CComPtr<IImageLayerFactory> m_pFactory;
		CComPtr<IConfig> m_pConfig;
		bool m_bWithMangling;
	};

	class ATL_NO_VTABLE CSubMenu : public CInsert
	{
	public:
		void Init(CMenuCommandsLayerInsert* a_pParent, IDesignerView* a_pView, IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pSSM, BSTR a_bstrSyncID, LONG a_ePresentMode, IConfig* a_pConfig, IImageLayerFactory* a_pFactory)
		{
			CInsert::Init(a_pView, a_pDoc, a_pDLI, a_pSSM, a_bstrSyncID, GUID_NULL, a_pFactory, true, NULL);
			m_pParent.Attach(a_pParent);
			a_pParent->AddRef();
			m_ePresentMode = a_ePresentMode;
			m_pConfig = m_ePresentMode == CFGVAL_PM_COMBINED_LAST ? a_pConfig : NULL;
		}

		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]Create layer[0405]Vytvořit vrstvu");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (a_pIconID == NULL)
				return E_POINTER;
			if (m_ePresentMode == CFGVAL_PM_SUBMENU || m_pFactory == NULL)
			{
				// {95C81ED1-BF55-4232-9E91-4F79CAB7135E}
				static const GUID tIconID = {0x95c81ed1, 0xbf55, 0x4232, {0x9e, 0x91, 0x4f, 0x79, 0xca, 0xb7, 0x13, 0x5e}};
				*a_pIconID = tIconID;
				return S_OK;
			}
			return CInsert::IconID(a_pIconID);
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (m_ePresentMode == CFGVAL_PM_SUBMENU || m_pFactory == NULL)
			{
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
				static IRTarget const tTarget(0.8f, -1, 1);

				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&tCanvasBack, 1, &tBack, pSI->GetMaterial(ESMAltBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tFront, pSI->GetMaterial(ESMBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tCorner, pSI->GetMaterial(ESMOutlineSoft), &tTarget);
				pSI->GetLayers(ESICreate, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();

				return S_OK;
			}
			return CInsert::Icon(a_nSize, a_phIcon);
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
		{
			if (a_ppSubCommands == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IEnumUnknownsInit> pInit;
				RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));
				m_pParent->InsertAllPlugIns(m_pView, m_pDoc, m_pDLI, m_pSSM, m_bstrSyncID, false, m_pConfig, pInit);
				*a_ppSubCommands = pInit;
				pInit.Detach();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			if (a_peState == NULL)
				return E_POINTER;
			*a_peState = m_ePresentMode == CFGVAL_PM_SUBMENU ? EMCSSubMenu : EMCSExecuteSubMenu;
			return S_OK;
		}

	private:
		CComPtr<CMenuCommandsLayerInsert> m_pParent;
		LONG m_ePresentMode;
		CComPtr<IConfig> m_pConfig;
	};
};

// {BFE869C4-A824-4801-8EEF-A6521AF219C4}
static const GUID CLSID_MenuCommandsLayerInsert = {0xbfe869c4, 0xa824, 0x4801, {0x8e, 0xef, 0xa6, 0x52, 0x1a, 0xf2, 0x19, 0xc4}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerInsert, CMenuCommandsLayerInsert)
