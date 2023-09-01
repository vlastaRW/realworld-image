// DocumentOperationRasterImageUnsharpMask.cpp : Implementation of CDocumentOperationRasterImageUnsharpMask

#include "stdafx.h"
#include "DocumentOperationRasterImageUnsharpMask.h"
#include <MultiLanguageString.h>
#include <RWDocumentImageRaster.h>
#include <IconRenderer.h>
#include <agg_pixfmt_rgba.h>
#include <agg_blur.h>

const OLECHAR CFGID_BLUM_RADIUS[] = L"Radius";
const OLECHAR CFGID_BLUM_AMOUNT[] = L"Amount";
const OLECHAR CFGID_BLUM_THRESHOLD[] = L"Threshold";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIUnsharpMask :
	public CCustomConfigResourcelessWndImpl<CConfigGUIUnsharpMask>,
	public CDialogResize<CConfigGUIUnsharpMask>
{
public:
	CConfigGUIUnsharpMask() : m_bFromSlider(false)
	{
	}

	enum
	{
		IDC_CGBLUM_RADIUS_TEXT = 100, IDC_CGBLUM_RADIUS_EDIT, IDC_CGBLUM_RADIUS_SPIN, IDC_CGBLUM_RADIUS_SLIDER,
		IDC_CGBLUM_AMOUNT_TEXT, IDC_CGBLUM_AMOUNT_EDIT, IDC_CGBLUM_AMOUNT_SPIN, IDC_CGBLUM_AMOUNT_SLIDER,
		IDC_CGBLUM_THRESHOLD_TEXT, IDC_CGBLUM_THRESHOLD_EDIT, IDC_CGBLUM_THRESHOLD_SPIN, IDC_CGBLUM_THRESHOLD_SLIDER
	};

	BEGIN_DIALOG_EX(0, 0, 120, 108, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Radius:[0405]Rádius:"), IDC_CGBLUM_RADIUS_TEXT, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGBLUM_RADIUS_EDIT, 44, 0, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_RADIUS_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 0, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_RADIUS_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 16, 120, 12, 0)
		CONTROL_LTEXT(_T("[0409]Amount:[0405]Síla:"), IDC_CGBLUM_AMOUNT_TEXT, 0, 37, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGBLUM_AMOUNT_EDIT, 44, 35, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_AMOUNT_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 35, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_AMOUNT_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 51, 120, 12, 0)
		CONTROL_LTEXT(_T("[0409]Threshold:[0405]Práh:"), IDC_CGBLUM_THRESHOLD_TEXT, 0, 72, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGBLUM_THRESHOLD_EDIT, 44, 70, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_THRESHOLD_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 70, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGBLUM_THRESHOLD_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 86, 120, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIUnsharpMask)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIUnsharpMask>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIUnsharpMask>)
		MESSAGE_HANDLER(WM_HSCROLL, OnRadiusSlider)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 0.5f; return TRUE; }
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIUnsharpMask)
		DLGRESIZE_CONTROL(IDC_CGBLUM_RADIUS_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_AMOUNT_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_THRESHOLD_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_RADIUS_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_AMOUNT_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_THRESHOLD_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_RADIUS_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_AMOUNT_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBLUM_THRESHOLD_EDIT, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIUnsharpMask)
		CONFIGITEM_EDITBOX(IDC_CGBLUM_RADIUS_EDIT, CFGID_BLUM_RADIUS)
		//CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGBLUM_RADIUS_SLIDER, CFGID_BLUM_RADIUS)
		CONFIGITEM_EDITBOX(IDC_CGBLUM_AMOUNT_EDIT, CFGID_BLUM_AMOUNT)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGBLUM_AMOUNT_SLIDER, CFGID_BLUM_AMOUNT)
		CONFIGITEM_EDITBOX(IDC_CGBLUM_THRESHOLD_EDIT, CFGID_BLUM_THRESHOLD)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGBLUM_THRESHOLD_SLIDER, CFGID_BLUM_THRESHOLD)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_CGBLUM_RADIUS_SPIN)).SetRange(0, 300);
		CUpDownCtrl(GetDlgItem(IDC_CGBLUM_AMOUNT_SPIN)).SetRange(0, 400);
		// Sliders
		CUpDownCtrl(GetDlgItem(IDC_CGBLUM_THRESHOLD_SPIN)).SetRange(0, 255);
		CTrackBarCtrl(GetDlgItem(IDC_CGBLUM_RADIUS_SLIDER)).SetRange(0, 345);

		DlgResize_Init(false, false, 0);
		return 1;
	}

	LRESULT OnRadiusSlider(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		try
		{
			CTrackBarCtrl wndSlider = GetDlgItem(IDC_CGBLUM_RADIUS_SLIDER);
			int nPos = wndSlider.GetPos();
			CComBSTR cCFGID_Radius(CFGID_BLUM_RADIUS);
			CConfigValue cVal(nPos > 50 ? nPos - 45.0f : nPos / 10.0f);
			//CConfigValue cVal((float)nPos);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_Radius;
			TConfigValue aVals[1];
			aVals[0] = cVal;
			m_bFromSlider = true;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}
		return 0;
	}

	void ExtraConfigNotify()
	{
		try
		{
			CConfigValue cValRadius;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BLUM_RADIUS), &cValRadius);
			CTrackBarCtrl wndSlider = GetDlgItem(IDC_CGBLUM_RADIUS_SLIDER);
			int nVal = cValRadius.operator float() > 5.0f ? cValRadius.operator float() + 45 : cValRadius.operator float() * 10.0f;
			//int nVal = cValRadius.operator float();
			if(!m_bFromSlider)
				wndSlider.SetPos(nVal);
			m_bFromSlider = false;
		}
		catch (...)
		{
		}
	}
private:
	bool m_bFromSlider;
};

// CDocumentOperationRasterImageUnsharpMask

STDMETHODIMP CDocumentOperationRasterImageUnsharpMask::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	if (a_ppOperationName == nullptr)
		return E_POINTER;
	try
	{
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Unsharp Mask[0405]Rastrový obrázek - maskování neostrosti");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

//#include <PrintfLocalizedString.h>
//#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageUnsharpMask::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Unsharp mask[0405]Maskování neostrosti";
		//if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HICON CDocumentOperationRasterImageUnsharpMask::GetDefaultIcon(ULONG a_nSize)
{
	static IRPolyPoint const handle1[] = { {215, 160}, {256, 230}, {216, 251}, {182, 179} };
	static IRPathPoint const blade1[] =
	{
		{131, 4, -30, 41, 0, 0},
		{163, 200, 0, 0, -46, -60},
		{221, 170, 0, 0, 0, 0},
	};
	static IRPathPoint const hilight1[] =
	{
		{166, 189, -35, -41, 0, 0},
		{130, 20, -2, 63, -18, 47},
		{174, 183, 0, 0, -35, -44},
	};
	static IRPolyPoint const handle2[] = { {63, 128}, {0, 178}, {29, 212}, {87, 158} };
	static IRPathPoint const blade2[] =
	{
		{199, 14, 12.8002, 49.1646, 0, 0},
		{97, 184, 0, 0, 64.8572, -38.8528},
		{54, 135, 0, 0, 0, 0},
	};
	static IRPathPoint const hilight2[] =
	{
		{97, 174, 47.6353, -25.2365, 0, 0},
		{193, 30, -20.898, 59.1521, 0.102005, 49.1521},
		{92, 165, 0, 0, 48.7397, -28.0258},
	};
	static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(handle1), handle1, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(blade1), blade1, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(hilight1), hilight1, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(handle2), handle2, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(blade2), blade2, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(hilight2), hilight2, pSI->GetMaterial(ESMBackground));
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRasterImageUnsharpMask::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_BLUM_RADIUS), _SharedStringTable.GetStringAuto(IDS_CFGID_BLUM_RADIUS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BLUM_RADIUS_DESC), CConfigValue(1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BLUM_RADIUS), CMultiLanguageString::GetAuto(L"[0409]Radius[0405]Rádius"), CMultiLanguageString::GetAuto(L"[0409]Area of Effect[0405]Oblast efektu"), CConfigValue(1.0f), NULL, CConfigValue(0.0f), CConfigValue(300.0f), CConfigValue(0.1f), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BLUM_AMOUNT), CMultiLanguageString::GetAuto(L"[0409]Amount[0405]Síla"), CMultiLanguageString::GetAuto(L"[0409]Power of Effect[0405]Síla aplikovaného efektu"), CConfigValue(50L), NULL, CConfigValue(0L), CConfigValue(400L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BLUM_THRESHOLD), CMultiLanguageString::GetAuto(L"[0409]Threshold[0405]Práh"), CMultiLanguageString::GetAuto(L"[0409]Where the Effect has to be applied[0405]Kde se bude dodstřovat"), CConfigValue(0L), NULL, CConfigValue(0L), CConfigValue(255L), CConfigValue(1L), 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageUnsharpMask, CConfigGUIUnsharpMask>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageUnsharpMask::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, 1, aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageUnsharpMask::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CConfigValue cRadius;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BLUM_RADIUS), &cRadius);
		float const fRadius = cRadius;
		CConfigValue cTreshold;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BLUM_THRESHOLD), &cTreshold);
		long const nTreshold = cTreshold.operator long();
		CConfigValue cAmount;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BLUM_AMOUNT), &cAmount);
		float const fAmount = cAmount.operator long() / 100.0f;

		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[tSize.nX*tSize.nY]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		TPixelChannel* p = pSrc.m_p;

		agg::rendering_buffer rbuf(reinterpret_cast<BYTE*>(p), tSize.nX, tSize.nY, tSize.nX<<2);
		agg::pixfmt_rgba32_plain img(rbuf);
		//if (bHasAlpha)
			img.premultiply();
		agg::recursive_blur<agg::rgba8, agg::recursive_blur_calc_rgba<> > rb;
		rb.blur(img, fRadius);
		//if (bHasAlpha)
			img.demultiply();

		CAutoVectorPtr<TPixelChannel> pSrcOrig(new TPixelChannel[tSize.nX*tSize.nY]);
		hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pSrcOrig.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		for (ULONG i = 0; i < tSize.nX*tSize.nY; ++i)
		{
			TPixelChannel tO = pSrcOrig[i];
			TPixelChannel tB = pSrc[i];
			if(abs(pSrcOrig[i].bR - pSrc[i].bR) >= nTreshold)
				pSrcOrig[i].bR = max(0, min(255, fAmount * (pSrcOrig[i].bR - pSrc[i].bR) + pSrcOrig[i].bR));
			if(abs(pSrcOrig[i].bG - pSrc[i].bG) >= nTreshold)
				pSrcOrig[i].bG = max(0, min(255, fAmount * (pSrcOrig[i].bG - pSrc[i].bG) + pSrcOrig[i].bG));
			if(abs(pSrcOrig[i].bB - pSrc[i].bB) >= nTreshold)
				pSrcOrig[i].bB = max(0, min(255, fAmount * (pSrcOrig[i].bB - pSrc[i].bB) + pSrcOrig[i].bB));
		}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pSrcOrig.m_p, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
