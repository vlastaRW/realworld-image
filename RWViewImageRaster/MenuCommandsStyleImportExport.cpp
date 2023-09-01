
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include "ConfigGUILayerID.h"
#include <SharedStateUndo.h>
#include <IconRenderer.h>


extern OLECHAR const CMDNAME_LAYEREFFECTLOAD[] = L"[0409]Open style...[0405]Otevřít styl...";
extern OLECHAR const CMDDESC_LAYEREFFECTLOAD[] = L"[0409]Pick a file with layer style and assign it to this layer.[0405]Vybrat soubor se stylem vrstvy a přiřadit ho tého vrstvě.";
extern OLECHAR const CMDNAME_LAYEREFFECTSAVE[] = L"[0409]Save style...[0405]Uložit styl...";
extern OLECHAR const CMDDESC_LAYEREFFECTSAVE[] = L"[0409]Store the style of the current layer into a file.[0405]Uložit styl vybrané vrstvy do souboru.";
static OLECHAR const RWEFFECT_FILTER[] = L"[0409]Layer effect files[0405]Soubory efektů vrstev";
static OLECHAR const RWEFFECT_OPEN[] = L"[0409]Open Layer Effect[0405]Otevřít efekt vrstvy";
static OLECHAR const RWEFFECT_SAVE[] = L"[0409]Save Layer Effect[0405]Uložit efekt vrstvy";
// {7652E8ED-2915-4395-BB2A-0E9267F843C0}
extern GUID const RWEFFECT_ICONID_OPEN = {0x7652e8ed, 0x2915, 0x4395, {0xbb, 0x2a, 0x0e, 0x92, 0x67, 0xf8, 0x43, 0xc0}};
// {0E568DB5-2B48-49b4-A761-ED4C718422A8}
extern GUID const RWEFFECT_ICONID_SAVE = {0x0e568db5, 0x2b48, 0x49b4, {0xa7, 0x61, 0xed, 0x4c, 0x71, 0x84, 0x22, 0xa8}};


class ATL_NO_VTABLE CMenuCommandsStyleImportExport :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsStyleImportExport>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsStyleImportExport()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsStyleImportExport)

BEGIN_CATEGORY_MAP(CMenuCommandsStyleImportExport)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsStyleImportExport)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Import/Export Style[0405]Vrstvený obrázek - import/export styl");
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

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSel;
			pDLI->StateUnpack(pState, &pSel);
			if (pSel == NULL)
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CLayerEffectLoad>* p = NULL;
				CComObject<CLayerEffectLoad>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CLayerEffectSave>* p = NULL;
				CComObject<CLayerEffectSave>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel);
				pItems->Insert(pTmp);
			}
			//{
			//	CComObject<CLayerEffectClear>* p = NULL;
			//	CComObject<CLayerEffectClear>::CreateInstance(&p);
			//	CComPtr<IDocumentMenuCommand> pTmp = p;
			//	p->Init(a_pDocument, pDLI, pSel, a_pStates, bstrID);
			//	pItems->Insert(pTmp);
			//}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	class ATL_NO_VTABLE CLayerEffectLoad :
		public CDocumentMenuCommandMLImpl<CLayerEffectLoad, CMDNAME_LAYEREFFECTLOAD, CMDDESC_LAYEREFFECTLOAD, &RWEFFECT_ICONID_OPEN, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pSel)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pSel = a_pSel;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				GearIcon(pSI, cRenderer);
				pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState()
		{
			ULONG nSel = 0;
			m_pSel->Size(&nSel);
			return nSel ? EMCSNormal : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IComparable> pItem;
				m_pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
				if (pItem == NULL) return E_FAIL;
				CComPtr<IConfig> pConfig;
				m_pDLI->LayerEffectGet(pItem, &pConfig, NULL);
				if (pConfig == NULL) return E_FAIL;

				CComPtr<IDocumentTypeWildcards> pDocType;
				RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
				CComBSTR bstrExt(L"rweffect");
				CComBSTR bstrFilter(L"*.");
				bstrFilter += bstrExt;
				pDocType->InitEx(CMultiLanguageString::GetAuto(RWEFFECT_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
				CComPtr<IEnumUnknownsInit> pDocTypes;
				RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
				pDocTypes->Insert(pDocType);
				CComPtr<IStorageManager> pStMgr;
				RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
				CComPtr<IStorageFilter> pStorage;
				static GUID const tEffectCtx = {0xe9f06e51, 0xf93, 0x4794, {0xbf, 0x49, 0x8a, 0xc5, 0xab, 0xe4, 0x6a, 0xc7}};
				pStMgr->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, a_hParent, pDocTypes, NULL, tEffectCtx, CMultiLanguageString::GetAuto(RWEFFECT_OPEN), NULL, a_tLocaleID, &pStorage);
				if (pStorage == NULL)
					return S_FALSE;

				CComPtr<IDataSrcDirect> pSrc;
				pStorage->SrcOpen(&pSrc);
				ULONG nSize = 0;
				if (pSrc == NULL || FAILED(pSrc->SizeGet(&nSize)) || nSize == 0)
				{
					// TODO: message
					return E_FAIL;
				}
				CDirectInputLock cData(pSrc, nSize);

				CComPtr<IConfigInMemory> pMemCfg;
				RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
				pMemCfg->DataBlockSet(nSize, cData.begin());

				CopyConfigValues(pConfig, pMemCfg);

				CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(CMDNAME_LAYEREFFECTLOAD));
				return m_pDLI->LayerEffectSet(pItem, pConfig);
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IEnumUnknowns> m_pSel;
	};

	class ATL_NO_VTABLE CLayerEffectSave :
		public CDocumentMenuCommandMLImpl<CLayerEffectSave, CMDNAME_LAYEREFFECTSAVE, CMDDESC_LAYEREFFECTSAVE, &RWEFFECT_ICONID_SAVE, 0>
	{
	public:
		void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pSel)
		{
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pSel = a_pSel;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				GearIcon(pSI, cRenderer);
				pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState()
		{
			ULONG nSel = 0;
			m_pSel->Size(&nSel);
			if (nSel == 0) return EMCSDisabled;
			CComPtr<IComparable> pItem;
			m_pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			if (pItem == NULL) return EMCSDisabled;
			CComPtr<IConfig> pConfig;
			m_pDLI->LayerEffectGet(pItem, &pConfig, NULL);
			if (pConfig == NULL) return EMCSDisabled;
			CConfigValue cVal;
			pConfig->ItemValueGet(CComBSTR(L"Effect"), &cVal);
			return IsEqualGUID(cVal, __uuidof(DocumentOperationNULL)) ? EMCSDisabled : EMCSNormal;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IComparable> pItem;
				m_pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
				if (pItem == NULL) return E_FAIL;
				CComPtr<IConfig> pConfig;
				m_pDLI->LayerEffectGet(pItem, &pConfig, NULL);
				if (pConfig == NULL) return E_FAIL;

				CComPtr<IConfigInMemory> pMemCfg;
				RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
				CopyConfigValues(pMemCfg, pConfig);
				ULONG nSize = 0;
				pMemCfg->DataBlockGetSize(&nSize);
				if (nSize == 0)
					return E_FAIL;

				CAutoVectorPtr<BYTE> pMem(new BYTE[nSize]);

				pMemCfg->DataBlockGet(nSize, pMem.m_p);

				CComPtr<IDocumentTypeWildcards> pDocType;
				RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
				CComBSTR bstrExt(L"rweffect");
				CComBSTR bstrFilter(L"*.");
				bstrFilter += bstrExt;
				pDocType->InitEx(CMultiLanguageString::GetAuto(RWEFFECT_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
				CComPtr<IEnumUnknownsInit> pDocTypes;
				RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
				pDocTypes->Insert(pDocType);
				CComPtr<IStorageManager> pStMgr;
				RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
				CComPtr<IStorageFilter> pStorage;
				static GUID const tEffectCtx = {0xe9f06e51, 0xf93, 0x4794, {0xbf, 0x49, 0x8a, 0xc5, 0xab, 0xe4, 0x6a, 0xc7}};
				pStMgr->FilterCreateInteractivelyUID(NULL, EFTCreateNew, a_hParent, pDocTypes, NULL, tEffectCtx, CMultiLanguageString::GetAuto(RWEFFECT_SAVE), NULL, a_tLocaleID, &pStorage);
				if (pStorage == NULL)
					return S_FALSE;
				CComPtr<IDataDstStream> pDst;
				pStorage->DstOpen(&pDst);
				if (pDst == NULL || FAILED(pDst->Write(nSize, pMem.m_p)) || FAILED(pDst->Close()))
				{
					// TODO: message
					return E_FAIL;
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
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IEnumUnknowns> m_pSel;
	};

	static void GearIcon(IStockIcons* pSI, CIconRendererReceiver& cRenderer)
	{
		static int const arc = 7;
		static int const extra = 5;
		static float const cx = 0.42f;
		static float const cy = 0.58f;
		static float const scale = 1.0f;
		IRPolyPoint aInner[5*4];
		IRPolyPoint aOuter[10];
		IRPolyPoint* p = aInner;
		for (int i = 0; i < 5; ++i)
		{
			float const a1 = (i*0.4f+0.05f)*3.14159265359f;
			float const a2 = (i*0.4f+0.13f)*3.14159265359f;
			float const a3 = (i*0.4f+0.27f)*3.14159265359f;
			float const a4 = (i*0.4f+0.35f)*3.14159265359f;
			p->x = cx + cosf(a1)*0.27f*scale;
			p->y = cy + sinf(a1)*0.27f*scale;
			++p;
			p->x = cx + cosf(a2)*0.42f*scale;
			p->y = cy + sinf(a2)*0.42f*scale;
			++p;
			p->x = cx + cosf(a3)*0.42f*scale;
			p->y = cy + sinf(a3)*0.42f*scale;
			++p;
			p->x = cx + cosf(a4)*0.27f*scale;
			p->y = cy + sinf(a4)*0.27f*scale;
			++p;
		}
		p = aOuter;
		for (int i = 0; i < 10; ++i)
		{
			float const a1 = (i*0.2f+0.05f)*3.14159265359f;
			p->x = cx + cosf(a1)*0.1f*scale;
			p->y = cy + sinf(a1)*0.1f*scale;
			++p;
		}
		IRPolygon const aPoly[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
		static IRCanvas const canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};
		cRenderer(&canvas, itemsof(aPoly), aPoly, pSI->GetMaterial(ESMScheme2Color2));
	}
};

// {478E57BF-6FE4-40cd-925E-0C9F1BAA85AE}
static const GUID CLSID_MenuCommandsStyleImportExport = {0x478e57bf, 0x6fe4, 0x40cd, {0x92, 0x5e, 0xc, 0x9f, 0x1b, 0xaa, 0x85, 0xae}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsStyleImportExport, CMenuCommandsStyleImportExport)
