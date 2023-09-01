// DocumentOperationRasterImageReduceColors.cpp : Implementation of CDocumentOperationRasterImageReduceColors

#include "stdafx.h"
#include "DocumentOperationRasterImageReduceColors.h"
#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>

// CDocumentOperationRasterImageReduceColors

STDMETHODIMP CDocumentOperationRasterImageReduceColors::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Reduce Colors[0405]Rastrový obrázek - redukovat barvy");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_RC_MAXCOLORS[] = L"Colors";
static OLECHAR const CFGID_RC_DITHER[] = L"Dither";
//static LONG const CFGVAL_OP_INNER = 0;
//static LONG const CFGVAL_OP_CENTER = 1;
//static LONG const CFGVAL_OP_OUTER = 2;

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageReduceColors::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Reduce colors[0405]Redukovat barvy");
			return S_OK;
		}
		CConfigValue cColors;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RC_MAXCOLORS), &cColors);
		LONG nColors = cColors;
		if (nColors < 2) nColors = 2;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CMultiLanguageString(L"[0409]Reduce to %i colors[0405]Redukovat na %i barev"));
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		pStr->Init(pTempl, nColors);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}


#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUIReduceColors :
	public CCustomConfigResourcelessWndImpl<CConfigGUIReduceColors>,
	public CDialogResize<CConfigGUIReduceColors>
{
public:
	CConfigGUIReduceColors()
	{
	}

	enum { IDC_MAX_COLORS = 100, IDC_MAX_SLIDER, IDC_DITHER };

	BEGIN_DIALOG_EX(0, 0, 120, 26, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Colors:[0405]Barvy:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_MAX_COLORS, 40, 0, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_MAX_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 70, 0, 50, 12, 0)
		CONTROL_CHECKBOX(_T("[0409]Dithering[0405]Ditherování"), IDC_DITHER, 0, 16, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIReduceColors)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIReduceColors>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIReduceColors>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIReduceColors)
		DLGRESIZE_CONTROL(IDC_MAX_SLIDER, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_MAX_COLORS, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIReduceColors)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_MAX_SLIDER, CFGID_RC_MAXCOLORS)
		CONFIGITEM_EDITBOX(IDC_MAX_COLORS, CFGID_RC_MAXCOLORS)
		CONFIGITEM_CHECKBOX(IDC_DITHER, CFGID_RC_DITHER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_lParam)
			*reinterpret_cast<float*>(a_lParam) = 1.0f;
		return 0;
	}

	bool GetSliderRange(wchar_t const* UNREF(a_pszName), TConfigValue* a_pFrom, TConfigValue* a_pTo, TConfigValue* a_pStep)
	{
		a_pFrom->eTypeID = a_pTo->eTypeID = a_pStep->eTypeID = ECVTInteger;
		a_pFrom->iVal = 8;
		a_pTo->iVal = 256;
		a_pStep->iVal = 1;
		return true;
	}
};

STDMETHODIMP CDocumentOperationRasterImageReduceColors::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RC_MAXCOLORS), CMultiLanguageString::GetAuto(L"[0409]Colors[0405]Barvy"), CMultiLanguageString::GetAuto(L"[0409]Maximum number of colors in the image.[0405]Maximlní počet barvev v obrázku."), CConfigValue(100L), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RC_DITHER), CMultiLanguageString::GetAuto(L"[0409]Dithering[0405]Ditherování"), CMultiLanguageString::GetAuto(L"[0409]Dithering method used when convering image to paletized format.[0405]Metoda převodu obrázku do nižší barevné hloubky."), CConfigValue(true), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageReduceColors, CConfigGUIReduceColors>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageReduceColors::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentRasterImage)/*, __uuidof(IRasterImageControl)*/};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageReduceColors::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RC_MAXCOLORS), &cVal);
		LONG nColors = cVal;
		if (nColors < 8) nColors = 8;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RC_DITHER), &cVal);
		bool bDither = cVal;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;
		TImagePoint tOffset = {0, 0};
		TImageSize tSize = {1, 1};
		if (FAILED(pRI->CanvasGet(NULL, NULL, &tOffset, &tSize, NULL)))
			return E_FAIL;
		static TImagePoint const t00 = {0, 0};
		CComPtr<IColorQuantizer> pCQ;
		RWCoCreateInstance(pCQ, __uuidof(ColorQuantizer));
		pCQ->Init(nColors, bDither ? ECDDithering : ECDNoDithering, 0, NULL, &tSize);
		pRI->Inspect(EICIRGBA, &t00, &tSize, pCQ, NULL, EIRIAccurate);

		ULONG nNeeded = 0;
		pCQ->Colors(&CEnumItemCounter<IEnum2UInts, ULONG>(&nNeeded));
		if (nNeeded < ULONG(nColors))
			return S_FALSE; // image already has requested number of colors

		TRasterImageRect tRect = {tOffset, {tOffset.nX+tSize.nX, tOffset.nY+tSize.nY}};
		if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
			return S_FALSE; // not a valid rectangle

		ULONG nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		pCQ->Process(&tRect.tTL, tSize, NULL, tSize.nX*tSize.nY, pSrc, NULL);

		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

