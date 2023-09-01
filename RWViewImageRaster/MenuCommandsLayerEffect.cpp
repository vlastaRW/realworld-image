// MenuCommandsLayerEffect.cpp : Implementation of CMenuCommandsLayerEffect

#include "stdafx.h"
#include "MenuCommandsLayerEffect.h"
#include <SharedStringTable.h>
#include <XPGUI.h>
#include <DPIUtils.h>

#include "ConfigGUILayerID.h"

static OLECHAR const CFGID_SIZEX[] = L"SizeX";
static OLECHAR const CFGID_SIZEY[] = L"SizeY";
static OLECHAR const CFGID_PREVIEW[] = L"Preview";
static OLECHAR const CFGID_EXPAND[] = L"Expand";

// CMenuCommandsLayerEffect

STDMETHODIMP CMenuCommandsLayerEffect::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_LAYEREFFECT);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsLayerEffect::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"LAYER"), NULL, 0, NULL);
		CComPtr<ILocalizedString> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEX), pDummy, pDummy, CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEY), pDummy, pDummy, CConfigValue(LONG(CW_USEDEFAULT)), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_PREVIEW), pDummy, pDummy, CConfigValue(true), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_EXPAND), pDummy, pDummy, CConfigValue(0L), NULL, 0, NULL);
		pCfg->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DesignerViewFactoryLayerProperties, CConfigGUILayerIDDlg>::FinalizeConfig(pCfg);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}



class CClipboardHandler
{
public:
	CClipboardHandler(HWND a_hWnd)
	{
		if (!::OpenClipboard(a_hWnd))
			throw E_FAIL;
	}
	~CClipboardHandler()
	{
		::CloseClipboard();
	}
};

static UINT s_nClipboardFormat = 0;


extern __declspec(selectany) OLECHAR const CMDNAME_LAYEREFFECTCOPY[] = L"[0409]Copy style[0405]Kopírovat styl";
extern __declspec(selectany) OLECHAR const CMDDESC_LAYEREFFECTCOPY[] = L"[0409]Copy the style of the current layer to the clipboard.[0405]Zkop8rovat styl vybrané vrstvy do schránky.";
class ATL_NO_VTABLE CLayerEffectCopy :
	public CDocumentMenuCommandMLImpl<CLayerEffectCopy, CMDNAME_LAYEREFFECTCOPY, CMDDESC_LAYEREFFECTCOPY, NULL, 0>
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

			if (s_nClipboardFormat == 0) s_nClipboardFormat = RegisterClipboardFormat(_T("RWCONFIG"));

			CComPtr<IConfigInMemory> pMemCfg;
			RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
			CopyConfigValues(pMemCfg, pConfig);
			ULONG nSize = 0;
			pMemCfg->DataBlockGetSize(&nSize);
			if (nSize == 0)
				return 0;

			CClipboardHandler cClipboard(a_hParent);
			EmptyClipboard();

			HANDLE hMem = GlobalAlloc(GHND, nSize+sizeof nSize);
			if (hMem == NULL)
				return 0;

			BYTE* pMem = reinterpret_cast<BYTE*>(GlobalLock(hMem));
			*reinterpret_cast<ULONG*>(pMem) = nSize;
			pMemCfg->DataBlockGet(nSize, pMem+sizeof nSize);

			GlobalUnlock(hMem);

			SetClipboardData(s_nClipboardFormat, hMem);

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

extern __declspec(selectany) OLECHAR const CMDNAME_LAYEREFFECTPASTE[] = L"[0409]Paste style[0405]Vložit styl";
extern __declspec(selectany) OLECHAR const CMDDESC_LAYEREFFECTPASTE[] = L"[0409]Store the style of the current layer into a file.[0405]Uložit styl vybrané vrstvy do souboru.";
class ATL_NO_VTABLE CLayerEffectPaste :
	public CDocumentMenuCommandMLImpl<CLayerEffectPaste, CMDNAME_LAYEREFFECTPASTE, CMDDESC_LAYEREFFECTPASTE, NULL, 0>
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
	EMenuCommandState IntState()
	{
		ULONG nSel = 0;
		m_pSel->Size(&nSel);
		if (nSel == 0) return EMCSDisabled;
		if (s_nClipboardFormat == 0) s_nClipboardFormat = RegisterClipboardFormat(_T("RWCONFIG"));
		return IsClipboardFormatAvailable(s_nClipboardFormat) ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			ULONG nItems = 0;
			m_pSel->Size(&nItems);
			if (nItems == 0) return E_FAIL;

			if (s_nClipboardFormat == 0) s_nClipboardFormat = RegisterClipboardFormat(_T("RWCONFIG"));

			CClipboardHandler cClipboard(a_hParent);

			HANDLE hMem = GetClipboardData(s_nClipboardFormat);
			if (hMem == NULL)
				return 0;

			ULONG const* pMem = reinterpret_cast<ULONG const*>(GlobalLock(hMem));
			CComPtr<IConfigInMemory> pMemCfg;
			RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
			pMemCfg->DataBlockSet(*pMem, reinterpret_cast<BYTE const*>(pMem+1));

			GlobalUnlock(hMem);

			CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(CMDNAME_LAYEREFFECTPASTE));
			{
				CWriteLock<IDocument> cLock(m_pDoc);
				HRESULT hRes = E_FAIL;
				for (ULONG i = 0; i < nItems; ++i)
				{
					CComPtr<IComparable> pItem;
					m_pSel->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
					CComPtr<IConfig> pConfig;
					m_pDLI->LayerEffectGet(pItem, &pConfig, NULL);
					if (pConfig == NULL) continue;
					CopyConfigValues(pConfig, pMemCfg);
					if (pItem && SUCCEEDED(m_pDLI->LayerEffectSet(pItem, pConfig)))
						hRes = S_OK;
				}
				return hRes;
			}
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


#include "ConfigureLayerEffectDlg.h"

extern __declspec(selectany) OLECHAR const CMDNAME_LAYEREFFECTCONFIGURE[] = L"[0409]Modify style...[0405]Upravit styl...";
extern __declspec(selectany) OLECHAR const CMDDESC_LAYEREFFECTCONFIGURE[] = L"[0409]Change an operation (or a sequence thereof) applied on the selected layer.[0405]Změnit operaci (nebo sekvenci operací) aplikovanou na vybranou vrstvu.";
class ATL_NO_VTABLE CLayerEffectConfigure :
	public CDocumentMenuCommandMLImpl<CLayerEffectConfigure, CMDNAME_LAYEREFFECTCONFIGURE, CMDDESC_LAYEREFFECTCONFIGURE, NULL, 0>//&g_tIconIDLayerOperation, IDI_LAYER_OPERATION>
{
public:
	void Init(IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pSel, IConfig* a_pConfig)
	{
		m_pDoc = a_pDoc;
		m_pDLI = a_pDLI;
		m_pSel = a_pSel;
		m_pConfig = a_pConfig;
	}

	// IDocumentMenuCommand
public:
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
			CComPtr<ISubDocumentID> pSDID;
			m_pDLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pSubDoc;
			if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
			CComPtr<IDocumentImage> pImage;
			if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));

			CConfigureLayerEffectDlg cDlg(pConfig, a_tLocaleID, m_pConfig, pImage);
			if (IDOK == cDlg.DoModal(a_hParent))
			{
				CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(CMDNAME_LAYEREFFECTCONFIGURE));
				return m_pDLI->LayerEffectSet(pItem, pConfig);
			}
			return S_FALSE;
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
	CComPtr<IConfig> m_pConfig;
};


STDMETHODIMP CMenuCommandsLayerEffect::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

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
		//if (pState == NULL)
		//	return S_FALSE;
		CComPtr<IEnumUnknowns> pSel;
		pDLI->StateUnpack(pState, &pSel);
		if (pSel == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		if (pDLI)
		{
			CComPtr<IDocumentMenuCommand> pSep;
			RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));

			{
				CComObject<CLayerEffectConfigure>* p = NULL;
				CComObject<CLayerEffectConfigure>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel, a_pConfig);
				pItems->Insert(pTmp);
			}
			pItems->Insert(pSep);
			{
				CComObject<CLayerEffectCopy>* p = NULL;
				CComObject<CLayerEffectCopy>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CLayerEffectPaste>* p = NULL;
				CComObject<CLayerEffectPaste>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDLI, pSel);
				pItems->Insert(pTmp);
			}
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

