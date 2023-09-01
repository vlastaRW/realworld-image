// DocumentFactoryLayeredImage.cpp : Implementation of CDocumentFactoryLayeredImage

#include "stdafx.h"
#include "DocumentFactoryLayeredImage.h"

#include "DocumentFactoryRasterImage.h"
#include <PlugInCache.h>
#include <algorithm>


// CDocumentFactoryLayeredImage

HRESULT CDocumentFactoryLayeredImage::DBPriority(ULONG* a_pnPriority)
{
	try
	{
		*a_pnPriority = EDPAverage-1;
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr == NULL) return S_OK;
		CComPtr<IConfig> pGlbCfg;
		pMgr->Config(CLSID_GlobalConfigDefaultImageFormat, &pGlbCfg);
		if (pGlbCfg == NULL) return S_OK;
		CConfigValue cVal;
		pGlbCfg->ItemValueGet(CComBSTR(CFGID_DIF_BUILDER), &cVal);
		if (cVal.TypeGet() == ECVTGUID && IsEqualGUID(cVal, __uuidof(DocumentFactoryLayeredImage)))
			*a_pnPriority = EDPAverage;
		return S_OK;
	}
	catch (...)
	{
		return a_pnPriority == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::TypeName(ILocalizedString** a_ppType)
{
	if (a_ppType == nullptr)
		return E_POINTER;
	try
	{
		*a_ppType = new CMultiLanguageString(L"[0409]Layered Image[0405]Vrstvený obrázek");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentFactoryLayeredImage::DBIcon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		*a_phIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_DOCUMENT_LAYEREDIMAGE), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		return S_OK;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::FormatInfo(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon)
{
	try
	{
		if (a_ppFormat)
			*a_ppFormat = NULL;
		if (a_pbstrShellIcon)
			*a_pbstrShellIcon = NULL;
		if (a_ppFormat)
			*a_ppFormat = new CMultiLanguageString(L"[0409]Layered image files[0405]Soubory vrstvených obrázků");
		if (a_pbstrShellIcon)
		{
			OLECHAR szModuleName[MAX_PATH] = L"";
			GetModuleFileName(_pModule->get_m_hInst(), szModuleName, MAX_PATH);
			wcscat(szModuleName, L",2");
			*a_pbstrShellIcon = SysAllocString(szModuleName);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::HasFeatures(ULONG a_nCount, IID const* a_aiidRequired)
{
	try
	{
		return SupportsAllFeatures(itemsof(g_aSupportedLayered), g_aSupportedLayered, a_nCount, a_aiidRequired) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentFactoryLayeredImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
{
	try
	{
		CComPtr<IDocumentData> pTmp;
		HRESULT hRes = CDocumentFactoryRasterImage::CreateData(pTmp, a_pSize, a_pResolution, a_nChannels, a_aChannelDefaults, a_fGamma, a_pTile);
		if (FAILED(hRes)) return hRes;

		// create layered image and add the layer
		CComObject<CDocumentLayeredImage>* pLayDoc = NULL;
		CComObject<CDocumentLayeredImage>::CreateInstance(&pLayDoc);
		CComPtr<IDocumentData> pD = pLayDoc;

		pLayDoc->Init(a_pResolution);
		hRes = a_pBase->DataBlockSet(a_bstrPrefix, pD);
		if (FAILED(hRes)) return hRes;
		pLayDoc->InitLayer(pTmp, GetThreadLocale(), *a_pSize);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground)
{
	try
	{
		CComPtr<IDocumentFactoryVectorImage> pDFVI;
		RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryVectorImage));
		if (pDFVI == NULL)
			return E_FAIL;
		CComBSTR bstrSubID(a_bstrPrefix);
		bstrSubID += L"L1;";
		HRESULT hRes = pDFVI->Create(bstrSubID, a_pBase, a_pSize, a_pResolution, a_aBackground);
		if (FAILED(hRes)) return hRes;

		// create layered image and add the layer
		CComObject<CDocumentLayeredImage>* pLayDoc = NULL;
		CComObject<CDocumentLayeredImage>::CreateInstance(&pLayDoc);
		CComPtr<IDocumentData> pD = pLayDoc;

		pLayDoc->Init(a_pResolution);
		hRes = a_pBase->DataBlockSet(a_bstrPrefix, pD);
		if (FAILED(hRes)) return hRes;
		pLayDoc->InitLayer(NULL, GetThreadLocale(), *a_pSize);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::AddObject(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled)
{
	try
	{
		CComPtr<IDocumentFactoryVectorImage> pDFVI;
		RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryVectorImage));
		if (pDFVI == NULL)
			return E_FAIL;
		CComBSTR bstrSubID(a_bstrPrefix);
		bstrSubID += L"L1;";
		return pDFVI->AddObject(bstrSubID, a_pBase, a_bstrName, a_bstrToolID, a_bstrParams, a_bstrStyleID, a_bstrStyleParams, a_pOutlineWidth, a_pOutlinePos, a_pOutlineJoins, a_pOutlineColor, a_pRasterizationMode, a_pCoordinatesMode, a_pEnabled);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		// create layered image and add the layer
		CComObject<CDocumentLayeredImage>* pLayDoc = NULL;
		CComObject<CDocumentLayeredImage>::CreateInstance(&pLayDoc);
		CComPtr<IDocumentData> pD = pLayDoc;

		return a_pBase->DataBlockSet(a_bstrPrefix, pD);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

extern const GUID CLSID_ImageFilterOpacity;

STDMETHODIMP CDocumentFactoryLayeredImage::AddLayer(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BYTE a_bBot, IImageLayerCreator* a_pCreator, BSTR a_bstrName, TImageLayer const* a_pProperties, IConfig* a_pOperation)
{
	try
	{
		CComPtr<IDocumentLayeredImage> pLI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
		if (pLI == NULL)
			return E_RW_ITEMNOTFOUND;
		//CComPtr<IComparable> pUnder;
		//if (a_bTop)
		//{
		//	CComPtr<IEnumUnknowns> pLayers;
		//	pLI->LayersEnum(&pLayers);
		//	ULONG nLayers = 0;
		//	if (pLayers) pLayers->Size(&nLayers);
		//	if (nLayers) pLayers->Get(nLayers-1, &pUnder);
		//}
		CComPtr<IComparable> pNew;
		//pLI->LayerInsert(pUnder, ELIPBelow, a_pCreator, &pNew);
		pLI->LayerInsert(NULL, a_bBot ? ELIPBelow : ELIPAbove, a_pCreator, &pNew);
		if (pNew == NULL)
			return E_FAIL;
		if (a_bstrName)
			pLI->LayerNameSet(pNew, a_bstrName);
		if (a_pOperation)
			pLI->LayerEffectSet(pNew, a_pOperation);
		if (a_pProperties)
		{
			TImageLayer tNewProps = *a_pProperties;
			if (tNewProps.fOpacity == 0.0f)
				tNewProps.bVisible = 0;
			tNewProps.fOpacity = 1.0f;
			pLI->LayerPropsSet(pNew, &tNewProps.eBlend, &tNewProps.bVisible);
			if (a_pProperties->fOpacity < 1.0f && a_pProperties->fOpacity > 0.0f)
			{
				CComPtr<IConfigInMemory> pCfg;
				RWCoCreateInstance(pCfg, __uuidof(ConfigInMemory));
				CComBSTR bstr(L"Opacity");
				pCfg->ItemValuesSet(1, &(bstr.m_str), CConfigValue(a_pProperties->fOpacity));
				pLI->LayerEffectStepAppend(pNew, 1, CLSID_ImageFilterOpacity, pCfg, NULL);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::SetResolution(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageResolution const* a_pResolution)
{
	try
	{
		CComPtr<IDocumentEditableImage> pEI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
		if (pEI == NULL)
			return E_RW_ITEMNOTFOUND;
		return pEI->CanvasSet(NULL, a_pResolution, NULL, NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::SetSize(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize)
{
	try
	{
		CComPtr<IDocumentEditableImage> pEI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
		if (pEI == NULL)
			return E_RW_ITEMNOTFOUND;
		ATLASSERT(a_pSize);
		return pEI->CanvasSet(a_pSize, NULL, NULL, NULL);
		/*CComPtr<IDocumentLayeredImage> pLI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
		if (pLI == NULL)
			return E_RW_ITEMNOTFOUND;
		CComPtr<IEnumUnknowns> pLayers;
		pLI->ItemsEnum(NULL, &pLayers);
		ULONG nLayers = 0;
		if (pLayers) pLayers->Size(&nLayers);
		TImagePoint tTL = {LONG_MAX, LONG_MAX};
		TImagePoint tBR = {LONG_MIN, LONG_MIN};
		for (ULONG i = 0; i < nLayers; ++i)
		{
			CComPtr<IComparable> pID;
			pLayers->Get(i, &pID);
			CComPtr<ISubDocumentID> pSDID;
			if (pID) pLI->ItemFeatureGet(pID, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pSubDoc;
			if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
			CComPtr<IDocumentImage> pSubImg;
			if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pSubImg));
			if (pSubImg)
			{
				TImagePoint tPt = {0, 0};
				TImageSize tSz = {0, 0};
				pSubImg->CanvasGet(NULL, NULL, &tPt, &tSz, NULL);
				if (tSz.nX*tSz.nY)
				{
					if (tTL.nX > tPt.nX) tTL.nX = tPt.nX;
					if (tTL.nY > tPt.nY) tTL.nY = tPt.nY;
					if (tBR.nX < tPt.nX+LONG(tSz.nX)) tBR.nX = tPt.nX+LONG(tSz.nX);
					if (tBR.nY < tPt.nY+LONG(tSz.nY)) tBR.nY = tPt.nY+LONG(tSz.nY);
				}
			}
		}
		if (tTL.nX < tBR.nX && tTL.nY < tBR.nY)
		{
			TImageSize tSz = {tBR.nX-tTL.nX, tBR.nY-tTL.nY};
			pEI->CanvasSet(&tSz, NULL, NULL, NULL);
		}
		return S_OK;*/
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::RearrangeLayers(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nLayers, TImagePoint const* a_aOffsets)
{
	try
	{
		CComPtr<IDocumentLayeredImage> pLI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
		if (pLI == NULL)
			return E_RW_ITEMNOTFOUND;
		CComPtr<IEnumUnknowns> pLayers;
		pLI->LayersEnum(NULL, &pLayers);
		ULONG nLayers = 0;
		if (pLayers) pLayers->Size(&nLayers);
		if (nLayers < a_nLayers) nLayers = a_nLayers;
		for (ULONG i = 0; i < nLayers; ++i)
		{
			if (a_aOffsets[i].nX == 0 && a_aOffsets[i].nY == 0)
				continue;
			CComPtr<IComparable> pID;
			pLayers->Get(i, &pID);
			CComPtr<ISubDocumentID> pSDID;
			if (pID) pLI->ItemFeatureGet(pID, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pDoc;
			if (pSDID) pSDID->SubDocumentGet(&pDoc);
			CComPtr<IDocumentEditableImage> pEI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
			TMatrix3x3f const tMtx = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  a_aOffsets[i].nX, a_aOffsets[i].nY, 1.0f};
			pEI->CanvasSet(NULL, NULL, &tMtx, NULL);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::AddBlock(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		CComPtr<IImageMetaData> pIMD;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD == NULL)
			return E_RW_ITEMNOTFOUND;
		return pIMD->SetBlock(a_bstrID, a_nSize, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CDocumentFactoryLayeredImage::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	return S_FALSE;
}

#include "ScriptedLayeredImage.h"

STDMETHODIMP CDocumentFactoryLayeredImage::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IDocumentLayeredImage> pDLI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
		if (pDLI)
		{
			CComObject<CScriptedLayeredImage>* p = NULL;
			CComObject<CScriptedLayeredImage>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pScriptingMgr, a_pDocument, pDLI);
			a_pSite->AddItem(CComBSTR(L"LayeredImage"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pSecondary->Insert(CComBSTR(L"LayeredImage"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

// new image wizard

HRESULT CDocumentFactoryLayeredImage::DWIcon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

		pSI->GetLayers(ESIPicture|ESIFancy|ESILarge, cRenderer);

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvořit");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_SIZEX[] = L"SizeX";
static OLECHAR const CFGID_SIZEY[] = L"SizeY";
static OLECHAR const CFGID_RESOLUTION[] = L"Resolution";
static OLECHAR const CFGID_BACKGROUND[] = L"Background";
static OLECHAR const CFGID_DOCTYPE[] = L"DocType";
static OLECHAR const CFGID_HISTORY[] = L"History";

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUINewImage : 
	public CCustomConfigResourcelessWndImpl<CConfigGUINewImage>
{
public:
	CConfigGUINewImage() : m_tHelpTextID(__uuidof(DocumentFactoryRasterImage)), m_bHistoryRead(false) {}

	BEGIN_DIALOG_EX(0, 0, 156, 20, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
		IDC_DESCRIPTION,
		IDC_WIDTH_EDIT,
		IDC_WIDTH_SPIN,
		IDC_HEIGHT_EDIT,
		IDC_HEIGHT_SPIN,
		IDC_RESOLUTION_EDIT,
		IDC_RESOLUTION_SPIN,
		IDC_DOCUMENTATION,
		IDC_BACKGROUND,
		IDC_DOCTYPE,
		IDC_HISTORY_SEP,
		IDC_HISTORY,
		WM_UPDATEHISTORY = WM_APP+639,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Create image[0405]Vytvořit obrázek"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]A raster image is a rectangular area of dots called pixels. A picture is created by assigning each dot a color using various drawing tools. All photographs and most pictures found on the web are in fact raster images.[0405]Rastrový obrázek je obdélníková oblast bodů (nazývaných pixely). Obrázek je vytvořen obarvením každého pixelu pomocí kreslicích nástrojů. Všechny fotografie a většina obrázků na Internetu jsou rastrovými obrázky."), IDC_DESCRIPTION, 0, 24, 156, 40, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]More information[0405]Více informací"), IDC_DOCUMENTATION, 0, 68, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)

		CONTROL_LTEXT(_T("[0409]Start with:[0405]Začít s:"), IDC_STATIC, 0, 85, 66, 8, WS_VISIBLE, 0)
		//CONTROL_CONTROL(_T(""), IDC_DOCTYPE, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 75, 147, 81, 100, 0)
		CONTROL_COMBOBOX(IDC_DOCTYPE, 75, 83, 81, 100, WS_VISIBLE | WS_TABSTOP | CBS_SORT | CBS_DROPDOWNLIST, 0);

		CONTROL_LTEXT(_T("[0409]Canvas width:[0405]Šířka plátna:"), IDC_STATIC, 0, 101, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_WIDTH_EDIT, 75, 99, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_WIDTH_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 99, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STATIC, 123, 101, 23, 8, WS_VISIBLE, 0)

		CONTROL_LTEXT(_T("[0409]Canvas height:[0405]Výška plátna:"), IDC_STATIC, 0, 117, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_HEIGHT_EDIT, 75, 115, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_HEIGHT_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 115, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STATIC, 123, 117, 23, 8, WS_VISIBLE, 0)

		CONTROL_LTEXT(_T("[0409]Resolution:[0405]Rozlišení:"), IDC_STATIC, 0, 133, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_RESOLUTION_EDIT, 75, 131, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_RESOLUTION_SPIN, UPDOWN_CLASS, UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 131, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]DPI[0405]DPI"), IDC_STATIC, 123, 133, 23, 8, WS_VISIBLE, 0)

		CONTROL_LTEXT(_T("[0409]Background color:[0405]Barva pozadí:"), IDC_STATIC, 0, 149, 73, 8, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_BACKGROUND, 75, 147, 42, 12, WS_VISIBLE, 0)

		CONTROL_CONTROL(_T(""), IDC_HISTORY_SEP, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 0, 165, 156, 1, 0)
		CONTROL_CONTROL(_T(""), IDC_HISTORY, WC_LISTVIEW, LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_NOCOLUMNHEADER | LVS_NOSCROLL | WS_TABSTOP | WS_VISIBLE, 0, 170, 156, 100, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUINewImage)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUINewImage>)
		NOTIFY_HANDLER(IDC_HISTORY, NM_CUSTOMDRAW, OnCustomDraw)
		NOTIFY_HANDLER(IDC_HISTORY, LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_HANDLER(IDC_HISTORY, LVN_ITEMACTIVATE, OnItemActivate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		NOTIFY_HANDLER(IDC_BACKGROUND, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		MESSAGE_HANDLER(WM_UPDATEHISTORY, OnUpdateHistory)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUINewImage)
		CONFIGITEM_EDITBOX(IDC_WIDTH_EDIT, CFGID_SIZEX)
		CONFIGITEM_EDITBOX(IDC_HEIGHT_EDIT, CFGID_SIZEY)
		CONFIGITEM_EDITBOX(IDC_RESOLUTION_EDIT, CFGID_RESOLUTION)
		CONFIGITEM_COMBOBOX(IDC_DOCTYPE, CFGID_DOCTYPE)
		CONFIGITEM_CONTEXTHELP(IDC_BACKGROUND, CFGID_BACKGROUND)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wndColor.SubclassWindow(GetDlgItem(IDC_BACKGROUND));
		m_wndColor.SetDefaultColor(CButtonColorPicker::SColor(0, true));
		CUpDownCtrl wnd;
		wnd = GetDlgItem(IDC_WIDTH_SPIN);
		wnd.SetRange(1, 30000);
		wnd = GetDlgItem(IDC_HEIGHT_SPIN);
		wnd.SetRange(1, 30000);
		wnd = GetDlgItem(IDC_RESOLUTION_SPIN);
		wnd.SetRange(10, 1000);

		LOGBRUSH logBrush;
		GetObject((HBRUSH)GetParent().SendMessage(WM_CTLCOLORDLG), sizeof(LOGBRUSH), &logBrush);
		m_clrBk = logBrush.lbColor;

		//m_iconSize = XPGUI::GetSmallIconSize();
		//m_icons.Create(m_iconSize, m_iconSize, XPGUI::GetImageListColorFlags(), 4, 1);
		m_list = GetDlgItem(IDC_HISTORY);
		//m_list.SetImageList(m_icons, LVSIL_SMALL);
		m_list.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT|LVS_EX_BORDERSELECT);
		if (XPGUI::IsVista() && CTheme::IsThemingSupported())
		{
			::SetWindowTheme(m_list, L"explorer", NULL);
		}
		m_list.SetBkColor(m_clrBk);
		RECT rc = {0};
		m_list.GetClientRect(&rc);
		LVCOLUMN col = {0};
		col.iSubItem = 0;
		col.cx = rc.right;//-GetSystemMetrics(SM_CXVSCROLL);
		col.mask = LVCF_WIDTH;
		m_list.InsertColumn(0, &col);
	}

	void ExtraConfigNotify()
	{
		CConfigValue cColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cColor);
		m_wndColor.SetColor(CButtonColorPicker::SColor(cColor[0], cColor[1], cColor[2], cColor[3]), true);
		CConfigValue cLayerType;
		M_Config()->ItemValueGet(CComBSTR(CFGID_DOCTYPE), &cLayerType);
		if (!IsEqualGUID(cLayerType, m_tHelpTextID))
		{
			CComPtr<IImageLayerFactory> pILF;
			RWCoCreateInstance(pILF, cLayerType);
			if (pILF)
			{
				CComPtr<ILocalizedString> pStr;
				pILF->LayerDescription(&pStr);
				CComBSTR bstr;
				if (pStr) pStr->GetLocalized(m_tLocaleID, &bstr);
				if (bstr) SetDlgItemText(IDC_DESCRIPTION, bstr);
				m_tHelpTextID = cLayerType;
			}
		}
		PostMessage(WM_UPDATEHISTORY);
	}

	LRESULT OnUpdateHistory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SItem item;
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
			item.x = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
			item.y = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_RESOLUTION), &cVal);
			item.dpi = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cVal);
			item.r = cVal[0];
			item.g = cVal[1];
			item.b = cVal[2];
			item.a = cVal[3];
			M_Config()->ItemValueGet(CComBSTR(CFGID_DOCTYPE), &cVal);
			item.layer = cVal;
		}
		CComBSTR bstrCFGID_HISTORY(CFGID_HISTORY);
		if (!m_bHistoryRead)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(bstrCFGID_HISTORY, &cVal);
			BSTR bstr = cVal;
			ULONG len = SysStringLen(bstr);
			wchar_t const* psz = bstr;
			while (psz && *psz)
			{
				SItem item;
				psz = ReadItem(psz, item);
				if (psz)
				{
					m_cHistoryOld.push_back(item);
					CComPtr<IImageLayerFactory> pILF;
					RWCoCreateInstance(pILF, item.layer);
					CComPtr<ILocalizedString> pName;
					if (pILF) pILF->LayerName(0, &pName);
					CComBSTR bstrName;
					if (pName) pName->GetLocalized(m_tLocaleID, &bstrName);
					wchar_t sz[256];
					swprintf(sz, L"%ix%i, %iDPI, %s", item.x, item.y, item.dpi, bstrName.m_str ? bstrName.m_str : L"");
					m_list.AddItem(m_cHistoryOld.size(), 0, sz);
				}
			}
			m_bHistoryRead = true;
		}
		std::vector<SItem>::const_iterator i = std::find(m_cHistoryOld.begin(), m_cHistoryOld.end(), item);
		m_cHistoryNew = m_cHistoryOld;
		if (i != m_cHistoryOld.end())
		{
			size_t index = i-m_cHistoryOld.begin();
			m_list.SelectItem(index);
			if (i != m_cHistoryOld.begin())
				std::rotate(m_cHistoryNew.begin(), m_cHistoryNew.begin()+index, m_cHistoryNew.begin()+index+1);
		}
		else
		{
			m_cHistoryNew.insert(m_cHistoryNew.begin(), item);
			int sel = m_list.GetSelectedIndex();
			if (sel >= 0)
				m_list.SetItemState(sel, 0, LVIS_SELECTED);
			if (m_cHistoryNew.size() > 8)
				m_cHistoryNew.resize(8);
		}
		CComBSTR bstrHistory;
		for (std::vector<SItem>::const_iterator j = m_cHistoryNew.begin(); j != m_cHistoryNew.end(); ++j)
		{
			wchar_t sz[256];
			swprintf(sz, L"|%i,%i,%i,%f,%f,%f,%f,%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
				j->x, j->y, j->dpi, j->r, j->g, j->b, j->a,
				j->layer.Data1, (DWORD)j->layer.Data2, (DWORD)j->layer.Data3,
				(DWORD)j->layer.Data4[0], (DWORD)j->layer.Data4[1],
				(DWORD)j->layer.Data4[2], (DWORD)j->layer.Data4[3],
				(DWORD)j->layer.Data4[4], (DWORD)j->layer.Data4[5],
				(DWORD)j->layer.Data4[6], (DWORD)j->layer.Data4[7]);
			bstrHistory += j == m_cHistoryNew.begin() ? sz+1 : sz;
		}
		TConfigValue tVal;
		ZeroMemory(&tVal, sizeof tVal);
		tVal.eTypeID = ECVTString;
		tVal.bstrVal = bstrHistory;
		M_Config()->ItemValuesSet(1, &(bstrCFGID_HISTORY.m_str), &tVal);

		return 0;
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfUnderline = 1;
		lf.lfHeight *= 2;
		GetDlgItem(IDC_TITLE).SetFont(m_font.CreateFontIndirect(&lf));

		//m_wndRootFCC = GetDlgItem(IDC_ROOTFCC);

		m_wndDocumentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/raster-image"));
		return 1;  // Let the system set the focus
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RECT rc = {0};
		m_list.GetWindowRect(&rc);
		ScreenToClient(&rc);
		rc.right = GET_X_LPARAM(lParam);
		rc.bottom = GET_Y_LPARAM(lParam);
		m_list.MoveWindow(&rc);
		return 0;
	}

	LRESULT OnItemChanged(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
	{
		int sel = m_list.GetSelectedIndex();
		if (sel >= 0 && sel < int(m_cHistoryOld.size()))
		{
			SItem const& item = m_cHistoryOld[sel];
			CComBSTR bstr1(CFGID_SIZEX);
			CComBSTR bstr2(CFGID_SIZEY);
			CComBSTR bstr3(CFGID_RESOLUTION);
			CComBSTR bstr4(CFGID_BACKGROUND);
			CComBSTR bstr5(CFGID_DOCTYPE);
			BSTR aIDs[5] = {bstr1, bstr2, bstr3, bstr4, bstr5};
			TConfigValue aVals[5];
			ZeroMemory(aVals, sizeof(aVals));
			aVals[0].eTypeID = ECVTInteger;
			aVals[0].iVal = item.x;
			aVals[1].eTypeID = ECVTInteger;
			aVals[1].iVal = item.y;
			aVals[2].eTypeID = ECVTInteger;
			aVals[2].iVal = item.dpi;
			aVals[3].eTypeID = ECVTVector4;
			aVals[3].vecVal[0] = item.r;
			aVals[3].vecVal[1] = item.g;
			aVals[3].vecVal[2] = item.b;
			aVals[3].vecVal[3] = item.a;
			aVals[4].eTypeID = ECVTGUID;
			aVals[4].guidVal = item.layer;
			M_Config()->ItemValuesSet(5, aIDs, aVals);
		}
		return 0;
	}

	LRESULT OnItemActivate(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
	{
		GetParent().GetParent().PostMessage(WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED));
		return 0;
	}

	LRESULT OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
	{
		NMLVCUSTOMDRAW* p = reinterpret_cast<NMLVCUSTOMDRAW*>(a_pNMHeader);
		switch (p->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
		case CDDS_ITEMPREPAINT:
			p->clrTextBk = m_clrBk;
			return CDRF_NEWFONT;
		}
		return CDRF_DODEFAULT;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
		CComBSTR cCFGID_BACKGROUND(CFGID_BACKGROUND);
		CConfigValue cColor(pClrBtn->clr.fR, pClrBtn->clr.fG, pClrBtn->clr.fB, pClrBtn->clr.fA);
		M_Config()->ItemValuesSet(1, &(cCFGID_BACKGROUND.m_str), cColor);

		return 0;
	}

private:
	struct SItem
	{
		LONG x;
		LONG y;
		LONG dpi;
		float r;
		float g;
		float b;
		float a;
		GUID layer;
		bool operator==(SItem const& rhs) const
		{
			return x == rhs.x && y == rhs.y && dpi == rhs.dpi &&
				fabsf(r-rhs.r) < 1e-4f && fabsf(g-rhs.g) < 1e-4f && fabsf(b-rhs.b) < 1e-4f && fabsf(a-rhs.a) < 1e-4f &&
				IsEqualGUID(layer, rhs.layer);
		}
	};

	wchar_t const* ReadItem(wchar_t const* psz, SItem& item)
	{
		if (7 != swscanf(psz, L"%i,%i,%i,%f,%f,%f,%f", &item.x, &item.y, &item.dpi, &item.r, &item.g, &item.b, &item.a))
			return NULL;
		for (int i = 0; i < 7; ++i)
		{
			while (*psz && *psz != L',') ++psz;
			if (*psz != L',')
				return NULL;
			++psz;
		}
		if (!GUIDFromString(psz, &item.layer))
			return NULL;
		psz += 36;
		if (*psz == L'|')
			++psz;
		return psz;
	}

private:
	CFont m_font;
	CHyperLink m_wndDocumentation;
	CButtonColorPicker m_wndColor;
	GUID m_tHelpTextID;
	CListViewCtrl m_list;
	COLORREF m_clrBk;
	std::vector<SItem> m_cHistoryOld;
	std::vector<SItem> m_cHistoryNew;
	bool m_bHistoryRead;
};

class CLayerTypeOptions :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigItemCustomOptions
{
public:
BEGIN_COM_MAP(CLayerTypeOptions)
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
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = m_cPlugIns.Map();
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
			*a_pnSize = 0;
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = m_cPlugIns.Map();
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
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = m_cPlugIns.Map();
			for (CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type::const_iterator i = cMap.begin(); i != cMap.end(); ++i)
			{
				if (i->second == NULL)
					continue; // TODO: fix plugin cache to not include items without requested interface
				if (a_nIndex == 0)
				{
					a_ptItem->eTypeID = ECVTGUID;
					a_ptItem->guidVal = i->first;
					return S_OK;
				}
				--a_nIndex;
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
		if (a_nCount == 0)
			return S_FALSE;
		try
		{
			ObjectLock cLock(this);
			CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory>::map_type const& cMap = m_cPlugIns.Map();
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

private:
	CPlugInCache<&CATID_DocumentBuilder, IImageLayerFactory> m_cPlugIns;
};


STDMETHODIMP CDocumentFactoryLayeredImage::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEX), NULL, NULL, CConfigValue(256L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEY), NULL, NULL, CConfigValue(256L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_RESOLUTION), NULL, NULL, CConfigValue(100L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_BACKGROUND), NULL, NULL, CConfigValue(0.0f, 0.0f, 0.0f, 0.0f), NULL, 0, NULL);

		CComObject<CLayerTypeOptions>* pOpts;
		CComObject<CLayerTypeOptions>::CreateInstance(&pOpts);
		CComPtr<IConfigItemCustomOptions> p = pOpts;
		pCfg->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_DOCTYPE), CMultiLanguageString::GetAuto(L"[0409]Layer type[0405]Typ vrstvy"), CMultiLanguageString::GetAuto(L"[0409]Type of the initial layer.[0405]Typ výchozí vrstvy."), CConfigValue(__uuidof(DocumentFactoryRasterImage)), pOpts, NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(CFGID_HISTORY), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentFactoryLayeredImage, CConfigGUINewImage>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		TImageSize tSize = {256, 256};
		LONG nResolution = 100;
		float aBackground[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		GUID tLayerBuilder = __uuidof(DocumentFactoryRasterImage);
		CConfigValue cVal;
		if (a_pConfig)
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
			tSize.nX = cVal.operator LONG();
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
			tSize.nY = cVal.operator LONG();
			a_pConfig->ItemValueGet(CComBSTR(CFGID_RESOLUTION), &cVal);
			nResolution = cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cVal);
			aBackground[0] = cVal[0];
			aBackground[1] = cVal[1];
			aBackground[2] = cVal[2];
			aBackground[3] = cVal[3];
			a_pConfig->ItemValueGet(CComBSTR(CFGID_DOCTYPE), &cVal);
			tLayerBuilder = cVal;
		}
		if (tSize.nX < 1) tSize.nX = 1;
		if (tSize.nY < 1) tSize.nY = 1;
		if (nResolution < 1) nResolution = 1;

		TImageResolution tRes = {nResolution, 254, nResolution, 254};
		a_pBuilder->Create(a_bstrPrefix, a_pBase);
		CComPtr<IImageLayerFactory> pBuilder;
		RWCoCreateInstance(pBuilder, tLayerBuilder);
		if (pBuilder == NULL)
			RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryRasterImage));
		CComPtr<IImageLayerCreator> pCreator;
		pBuilder->LayerCreatorGet(tSize, aBackground, &pCreator);
		a_pBuilder->SetResolution(a_bstrPrefix, a_pBase, &tRes);
		a_pBuilder->SetSize(a_bstrPrefix, a_pBase, &tSize);

		CComBSTR bstrPrefix;
		CMultiLanguageString::GetLocalized(L"[0409]layer[0405]vrstva", a_tLocaleID, &bstrPrefix);
		bstrPrefix += L" 0";

		return a_pBuilder->AddLayer(a_bstrPrefix, a_pBase, TRUE, pCreator, bstrPrefix, NULL, NULL);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryLayeredImage::GetThumbnail(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds))
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg == nullptr)
			return E_RW_UNKNOWNINPUTFORMAT;

		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pDocImg, false))
			return E_FAIL;
		HRESULT hRes = fnRescaleImage(cBuffer.tSize.nX, cBuffer.tSize.nY, reinterpret_cast<DWORD const*>(cBuffer.aData.m_p), true, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds);
		if (a_pbstrInfo)
		{
			wchar_t sz[128] = L"";
			CComBSTR bstrTempl;
			CMultiLanguageString::GetLocalized(L"[0409]Dimensions: %ix%i pixels[0405]Rozměry: %ix%i pixelů", a_tLocaleID, &bstrTempl);
			swprintf(sz, bstrTempl, cBuffer.tSize.nX, cBuffer.tSize.nY);
			*a_pbstrInfo = SysAllocString(sz);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

