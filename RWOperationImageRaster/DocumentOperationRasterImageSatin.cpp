// DocumentOperationRasterImageSatin.cpp : Implementation of CDocumentOperationRasterImageSatin

#include "stdafx.h"
#include "DocumentOperationRasterImageSatin.h"

#include <MultiLanguageString.h>
#include <agg_pixfmt_gray.h>
#include <agg_blur.h>
#include <IconRenderer.h>


// CDocumentOperationRasterImageSatin

STDMETHODIMP CDocumentOperationRasterImageSatin::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Satin[0405]Rastrový obrázek - satén");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_SAT_SIZE[] = L"Size";
static OLECHAR const CFGID_SAT_ANGLE[] = L"Angle";
static OLECHAR const CFGID_SAT_BLUR[] = L"Blur";
static OLECHAR const CFGID_SAT_COLOR[] = L"Color";
static OLECHAR const CFGID_SAT_INVERT[] = L"Invert";

#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageSatin::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Satin[0405]Satén";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cSize;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_SIZE), &cSize);
		CComPtr<ILocalizedString> pMethod;
		pMethod.Attach(new CMultiLanguageString(pszName));
		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_COLOR), &cColor);
		CComPtr<INamedColors> pNC;
		RWCoCreateInstance(pNC, __uuidof(NamedColors));
		CComPtr<ILocalizedString> pColorName;
		if (pNC) pNC->ColorToName(0xff000000|RGB(powf(cColor[0], 1.0f/2.2f)*255.0f+0.5f, powf(cColor[1], 1.0f/2.2f)*255.0f+0.5f, powf(cColor[2], 1.0f/2.2f)*255.0f+0.5f), &pColorName);
		CComPtr<ILocalizedString> pParams;
		if (pColorName)
		{
			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			pParams = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s, %ipx")));
			pStr->Init(pTempl, pColorName, int(cSize.operator float()+0.5f));
		}
		else
		{
			OLECHAR szTmp[16];
			swprintf(szTmp, L"%g", int(cSize.operator float()*10+0.5f)*0.1f);
			pParams.Attach(new CSimpleLocalizedString(SysAllocString(szTmp)));
		}
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
		pStr->Init(pTempl, pMethod, pParams);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HICON CDocumentOperationRasterImageSatin::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const aBack[] =
	{
		{114, 9, 16, -18, -6, 33},
		{248, 63, -32, 41, -31, -40},
		{166, 255, -23, -15, 20, -51},
		{66, 221, 37, -109, 32, -5},
	};
	static IRPath const tBack = {itemsof(aBack), aBack};
	static IRPathPoint const aFold[] =
	{
		{114, 9, 8, 17, -6, 14},
		{142, 44, -2, 10, -2, -14},
		{102, 82, 5, -21, 10, 0},
	};
	static IRPath const tFold = {itemsof(aFold), aFold};
	static IRPathPoint const aFront[] =
	{
		{41, 15, 18, 12, 10, 71},
		{142, 44, -4, 49, -26, 5},
		{76, 240, -15, 10, 13.1559, -8.77058},
		{10, 215, 20, -57, 13, 13},
	};
	static IRPath const tFront = {itemsof(aFront), aFront};
	static IRPathPoint const aHilight1[] =
	{
		{113, 74, -1, 19, 1.23709, -23.5047},
		{66, 221, 22, -88, 28, -72},
		{98, 88, 3.64949, -20.6805, -3, 17},
	};
	static IRPathPoint const aHilight2[] =
	{
		{216, 65, -23, 35, -25, 16},
		{155, 219, -4, -36, 11, -48},
		{166, 136, 9, -24, -9.45429, 25.2114},
	};
	static IRPath const aHilight[] = { {itemsof(aHilight1), aHilight1}, {itemsof(aHilight2), aHilight2} };
	static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&tCanvas, 1, &tBack, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&tCanvas, 1, &tFold, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&tCanvas, 1, &tFront, pSI->GetMaterial(ESMScheme1Color1));
	IRFill hl(0xffffffff);
	cRenderer(&tCanvas, itemsof(aHilight), aHilight, &hl);//pSI->GetMaterial(ESMScheme1Color1));
	return cRenderer.get();
}

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISatin :
	public CCustomConfigResourcelessWndImpl<CConfigGUISatin>,
	public CDialogResize<CConfigGUISatin>,
	public CObserverImpl<CConfigGUISatin, IColorPickerObserver, ULONG>,
	public IResizableConfigWindow
{
public:
	enum
	{
		IDC_DISTANCE = 100,
		IDC_DISTANCE_SLIDER,
		IDC_ANGLE_LABEL,
		IDC_ANGLE,
		IDC_ANGLE_SLIDER,
		IDC_BLUR_LABEL,
		IDC_BLUR,
		IDC_BLUR_SLIDER,
		IDC_COLOR,
		IDC_INVERT,
	};

	BEGIN_DIALOG_EX(0, 0, 120, 210, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Distance:[0405]Vzdálenost:"), IDC_STATIC, 0, 2, 58, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_DISTANCE, 60, 0, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_DISTANCE_SLIDER, TRACKBAR_CLASS, TBS_TOP | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 16, 120, 10, 0)

		CONTROL_LTEXT(_T("[0409]Angle:[0405]Úhel:"), IDC_ANGLE_LABEL, 0, 34, 58, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ANGLE, 60, 32, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_ANGLE_SLIDER, TRACKBAR_CLASS, TBS_TOP | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 48, 120, 10, 0)

		CONTROL_LTEXT(_T("[0409]Blur:[0405]Rozmazání:"), IDC_BLUR_LABEL, 0, 66, 58, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_BLUR, 60, 64, 60, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_BLUR_SLIDER, TRACKBAR_CLASS, TBS_TOP | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 80, 120, 10, 0)

		CONTROL_CHECKBOX(_T("[0409]Invert[0405]Inverzní"), IDC_INVERT, 0, 96, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUISatin)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISatin>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUISatin>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISatin)
		//DLGRESIZE_CONTROL(IDC_INTENSITY_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_BLUR, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_BLUR_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_DISTANCE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_DISTANCE_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_COLOR, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISatin)
		CONFIGITEM_CONTEXTHELP(IDC_COLOR, CFGID_SAT_COLOR)
		CONFIGITEM_CHECKBOX(IDC_INVERT, CFGID_SAT_INVERT)
		CONFIGITEM_EDITBOX(IDC_DISTANCE, CFGID_SAT_SIZE)
		CONFIGITEM_CONTEXTHELP(IDC_DISTANCE_SLIDER, CFGID_SAT_SIZE)
		CONFIGITEM_EDITBOX(IDC_ANGLE, CFGID_SAT_ANGLE)
		CONFIGITEM_CONTEXTHELP(IDC_ANGLE_SLIDER, CFGID_SAT_ANGLE)
		CONFIGITEM_EDITBOX(IDC_BLUR, CFGID_SAT_BLUR)
		CONFIGITEM_CONTEXTHELP(IDC_BLUR_SLIDER, CFGID_SAT_BLUR)
	END_CONFIGITEM_MAP()

	BEGIN_COM_MAP(CConfigGUISatin)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || m_pColor == NULL || a_pSize == NULL)
			return E_UNEXPECTED;
		RECT rc = {0, 0, 120, 110};
		MapDialogRect(&rc);
		SIZE sz = {0, 0};
		m_pColor->OptimumSize(&sz);
		a_pSize->cx = rc.right;
		a_pSize->cy = rc.bottom+sz.cy;
		return S_OK;
	}

	void ExtraInitDialog()
	{
		m_wndBlur = GetDlgItem(IDC_BLUR_SLIDER);
		m_wndBlur.SetRange(0, 100);
		m_wndBlur.SetPageSize(10);
		m_wndDistance = GetDlgItem(IDC_DISTANCE_SLIDER);
		m_wndDistance.SetRange(0, 100);
		m_wndDistance.SetPageSize(10);
		m_wndAngle = GetDlgItem(IDC_ANGLE_SLIDER);
		m_wndAngle.SetRange(0, 180);
		m_wndAngle.SetPageSize(10);

		RECT rcColor = {0, 110, 120, 110};
		MapDialogRect(&rcColor);
		RWCoCreateInstance(m_pColor, __uuidof(ColorPicker));
		m_pColor->Create(m_hWnd, &rcColor, TRUE, m_tLocaleID, NULL, FALSE, CComBSTR(L"SATIN"), NULL);
		RWHWND hColor;
		m_pColor->Handle(&hColor);
		::SetWindowLong(hColor, GWLP_ID, IDC_COLOR);
		SIZE tSize = {rcColor.right-rcColor.left, 0};
		m_pColor->OptimumSize(&tSize);
		rcColor.bottom = rcColor.top+tSize.cy;
		m_pColor->Move(&rcColor);
		m_pColor->ObserverIns(CObserverImpl<CConfigGUISatin, IColorPickerObserver, ULONG>::ObserverGet(), 0);

	}
	void ExtraConfigNotify()
	{
		if (m_hWnd == NULL)
			return;

		if (m_pColor)
		{
			CConfigValue cColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SAT_COLOR), &cColor);
			TColor clr = {cColor[0], cColor[1], cColor[2], cColor[3]};
			m_pColor->ColorSet(&clr);
		}

		if (m_wndBlur.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SAT_BLUR), &cVal);
			float f = cVal.operator float();
			if (f < 0.5) f = 0.5f;
			else if (f > 100) f = 100.0f;
			LONG nPos = logf(f*2.0f)*100.0f/logf(200.0f)+0.5f;
			if (m_wndBlur.GetPos() != nPos)
				m_wndBlur.SetPos(nPos);
		}

		if (m_wndDistance.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SAT_SIZE), &cVal);
			float f = cVal.operator float();
			if (f < 0.5) f = 0.5f;
			else if (f > 100) f = 100.0f;
			LONG nPos = logf(f*2.0f)*100.0f/logf(200.0f)+0.5f;
			if (m_wndDistance.GetPos() != nPos)
				m_wndDistance.SetPos(nPos);
		}

		if (m_wndAngle.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SAT_ANGLE), &cVal);
			LONG i = cVal.operator float();
			if (i < 0)
				i = 360-((-i)%360);
			i = i%360;
			if (i > 180)
				i = 360-i;
			if (m_wndAngle.GetPos() != i)
				m_wndAngle.SetPos(i);
		}
	}

	//void OwnerNotify(TCookie, IUnknown*)

	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		//if (!m_bEnableUpdates)
		//	return;

		if (a_nFlags&ECPCColor)
		{
			TColor clr;
			m_pColor->ColorGet(&clr);
			CConfigValue cVal(clr.fR, clr.fG, clr.fB, clr.fA);
			CComBSTR bstr(CFGID_SAT_COLOR);
			M_Config()->ItemValuesSet(1, &(bstr.m_str), cVal);
		}
		if (a_nFlags&(ECPCLayout|ECPCWindowHeight))
		{
			RWHWND h = NULL;
			m_pColor->Handle(&h);
			RECT rcWindow;
			::GetWindowRect(h, &rcWindow);
			ScreenToClient(&rcWindow);
			SIZE sz = {rcWindow.right-rcWindow.left, rcWindow.bottom-rcWindow.top};
			m_pColor->OptimumSize(&sz);
			rcWindow.bottom = rcWindow.top+sz.cy;
			m_pColor->Move(&rcWindow);
		}
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (m_pColor)
			m_pColor->ObserverDel(CObserverImpl<CConfigGUISatin, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndBlur == reinterpret_cast<HWND>(a_lParam))
		{
			LONG nPos = m_wndBlur.GetPos();
			float fPos = expf(nPos*0.01f*logf(200.0f))*0.5f;
			if (fPos > 300)
				fPos = int(fPos+0.5f);
			else if (fPos > 3)
				fPos = int(fPos*10.0f+0.5f)*0.1f;
			else
				fPos = int(fPos*100.0f+0.5f)*0.01f;
			CComBSTR bstrID(CFGID_SAT_BLUR);
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(fPos));
			return 0;
		}
		if (m_wndDistance == reinterpret_cast<HWND>(a_lParam))
		{
			LONG nPos = m_wndDistance.GetPos();
			float fPos = expf(nPos*0.01f*logf(200.0f))*0.5f;
			if (fPos > 300)
				fPos = int(fPos+0.5f);
			else if (fPos > 3)
				fPos = int(fPos*10.0f+0.5f)*0.1f;
			else
				fPos = int(fPos*100.0f+0.5f)*0.01f;
			CComBSTR bstrID(CFGID_SAT_SIZE);
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(fPos));
			return 0;
		}
		if (m_wndAngle == reinterpret_cast<HWND>(a_lParam))
		{
			float fPos = m_wndAngle.GetPos();
			CComBSTR bstrID(CFGID_SAT_ANGLE);
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(fPos));
			return 0;
		}
		a_bHandled = FALSE;
		return 0;
	}

private:
	CComPtr<IColorPicker> m_pColor;
	CTrackBarCtrl m_wndBlur;
	CTrackBarCtrl m_wndAngle;
	CTrackBarCtrl m_wndDistance;
};

STDMETHODIMP CDocumentOperationRasterImageSatin::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SAT_SIZE), CMultiLanguageString::GetAuto(L"[0409]Size[0405]Velikost"), CMultiLanguageString::GetAuto(L"[0409]Distance of the folds from the shape outline.[0405]Vzdálenost záhybů od okraje tvarů."), CConfigValue(8.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SAT_ANGLE), CMultiLanguageString::GetAuto(L"[0409]Angle[0405]Úhel"), CMultiLanguageString::GetAuto(L"[0409]Direction of the folds.[0405]Směr záhybů."), CConfigValue(30.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SAT_BLUR), CMultiLanguageString::GetAuto(L"[0409]Blur[0405]Rozmazání"), CMultiLanguageString::GetAuto(L"[0409]Sharpness of the folds.[0405]Ostrost záhybů."), CConfigValue(5.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SAT_COLOR), CMultiLanguageString::GetAuto(L"[0409]Color[0405]Barva"), CMultiLanguageString::GetAuto(L"[0409]Color of the fold shadows.[0405]Barva stínů záhybů."), CConfigValue(0.0f, 0.0f, 0.0f, 0.5f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SAT_INVERT), CMultiLanguageString::GetAuto(L"[0409]Invert[0405]Invertovat"), CMultiLanguageString::GetAuto(L"[0409]Swap the colors.[0405]Prohodit barvy."), CConfigValue(true), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageSatin, CConfigGUISatin>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageSatin::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

#include <GammaCorrection.h>
#include <RWViewImageRaster.h> // TODO: move color-related stuff to RWImaging

STDMETHODIMP CDocumentOperationRasterImageSatin::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_SIZE), &cVal);
		float fSize = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_BLUR), &cVal);
		float fBlur = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_ANGLE), &cVal);
		float fAngle = cVal;
		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_COLOR), &cColor);
		TRasterImagePixel const tCol =
		{
			CGammaTables::ToSRGB(cColor[2]),
			CGammaTables::ToSRGB(cColor[1]),
			CGammaTables::ToSRGB(cColor[0]),
			cColor[3] < 0.0f ? 0 : (cColor[3] > 1.0f ? 255 : cColor[3]*255.0f+0.5f),
		};
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_INVERT), &cVal);
		bool bInvert = cVal;

		float fShiftX = fSize*cosf(fAngle*3.14159265359f/180.0f);
		float fShiftY = -fSize*sinf(fAngle*3.14159265359f/180.0f);
		LONG const nBlur = ceilf(fBlur);
		LONG const nExtraSpaceX = ceilf(fabsf(fShiftX))+nBlur;
		LONG const nExtraSpaceY = ceilf(fabsf(fShiftY))+nBlur;
		LONG const nShiftX = fShiftX < 0 ? -(-fShiftX+0.5f) : (fShiftX+0.5f);
		LONG const nShiftY = fShiftY < 0 ? -(-fShiftY+0.5f) : (fShiftY+0.5f);

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		if (tCol.bA == 0)
			return S_FALSE; // would not have any effect

		CAutoPtr<CGammaTables> pGT(NULL);
		//float fGamma = 1.0f;
		//CComPtr<IGlobalConfigManager> pGCM;
		//RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		//if (pGCM)
		//{
		//	CComPtr<IConfig> pConfig;
		//	pGCM->Config(__uuidof(ColorWindow), &pConfig);
		//	if (pConfig)
		//	{
		//		CConfigValue cVal;
		//		pConfig->ItemValueGet(CComBSTR(L"Gamma"), &cVal);
		//		if (cVal.TypeGet() == ECVTFloat)
		//		{
		//			fGamma = cVal;
		//			if (fGamma < 0.1f) fGamma = 1.0f; else if (fGamma > 10.0f) fGamma = 1.0f;
		//		}
		//	}
		//}
		//if (fGamma != 1.0f)
		//	pGT.Attach(new CGammaTables(fGamma));
		pGT.Attach(new CGammaTables());

		TImagePoint tOffset = {0, 0};
		TImageSize tSize = {0, 0};
		if (FAILED(pRI->CanvasGet(NULL, NULL, &tOffset, &tSize, NULL)))
			return E_FAIL;

		TRasterImageRect tRect = {tOffset, {tOffset.nX+tSize.nX, tOffset.nY+tSize.nY}};
		if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
			return S_FALSE; // not a valid rectangle

		ULONG nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		CAutoVectorPtr<BYTE> pBlur(new BYTE[(tSize.nX+nBlur+nBlur)*(tSize.nY+nBlur+nBlur)]);

		ULONG const nClrR = pGT.m_p ? pGT->m_aGamma[tCol.bR] : 0;
		ULONG const nClrG = pGT.m_p ? pGT->m_aGamma[tCol.bG] : 0;
		ULONG const nClrB = pGT.m_p ? pGT->m_aGamma[tCol.bB] : 0;

		TPixelChannel* p = pSrc.m_p;
		BYTE* pB = pBlur.m_p;
		ZeroMemory(pB, nBlur*(tSize.nX+nBlur+nBlur));
		pB += nBlur*(tSize.nX+nBlur+nBlur);
		for (ULONG nY = 0; nY < tSize.nY; ++nY)
		{
			ZeroMemory(pB, nBlur);
			pB += nBlur;
			for (ULONG nX = 0; nX < tSize.nX; ++nX, ++pB, ++p)
				*pB = p->bA;
			ZeroMemory(pB, nBlur);
			pB += nBlur;
		}
		ZeroMemory(pB, nBlur*(tSize.nX+nBlur+nBlur));
		agg::rendering_buffer rbuf2(pBlur.m_p, tSize.nX+nBlur+nBlur, tSize.nY+nBlur+nBlur, tSize.nX+nBlur+nBlur);
		agg::pixfmt_gray8 img2(rbuf2);
		agg::stack_blur_gray8(img2, nBlur, nBlur);

		LONG const nBlurX = tSize.nX+nBlur+nBlur;
		LONG const nBlurY = tSize.nY+nBlur+nBlur;

		p = pSrc.m_p;
		pB = pBlur.m_p;
		for (ULONG nY = 0; nY < tSize.nY; ++nY)
		{
			for (ULONG nX = 0; nX < tSize.nX; ++nX, ++p)
			{
				if (p->bA == 0)
					continue;
				BYTE b1 = 0;
				LONG const nY1 = nY+nShiftY+nBlur;
				if (nY1 >= 0 && nY1 < nBlurY)
				{
					LONG const nX1 = nX+nShiftX+nBlur;
					if (nX1 >= 0 && nX1 < nBlurX)
						b1 = pB[nBlurX*nY1+nX1];
				}

				BYTE b2 = 0;
				LONG const nY2 = nY-nShiftY+nBlur;
				if (nY2 >= 0 && nY2 < nBlurY)
				{
					LONG const nX2 = nX-nShiftX+nBlur;
					if (nX2 >= 0 && nX2 < nBlurX)
						b2 = pB[nBlurX*nY2+nX2];
				}

				BYTE b = b1 > b2 ? b1-b2 : b2-b1;
				if (bInvert)
					b = b^0xff;
				if (b)
				{
					ULONG const a1 = tCol.bA*b;
					ULONG const a2 = 255*255-a1;
					if (pGT.m_p)
					{
						ULONG const nR = pGT->m_aGamma[p->bR]*a2+nClrR*a1;
						p->bR = pGT->InvGamma((nR+(nR>>7)+(nR>>15)+(nR>>16)+0x7fff)>>16);
						ULONG const nG = pGT->m_aGamma[p->bG]*a2+nClrG*a1;
						p->bG = pGT->InvGamma((nG+(nG>>7)+(nG>>15)+(nG>>16)+0x7fff)>>16);
						ULONG const nB = pGT->m_aGamma[p->bB]*a2+nClrB*a1;
						p->bB = pGT->InvGamma((nB+(nB>>7)+(nB>>15)+(nB>>16)+0x7fff)>>16);
					}
					else
					{
						ULONG const nR = p->bR*a2+tCol.bR*a1;
						p->bR = (nR+(nR>>7)+(nR>>15)+(nR>>16)+0x7fff)>>16;
						ULONG const nG = p->bG*a2+tCol.bG*a1;
						p->bG = (nG+(nG>>7)+(nG>>15)+(nG>>16)+0x7fff)>>16;
						ULONG const nB = p->bB*a2+tCol.bB*a1;
						p->bB = (nB+(nB>>7)+(nB>>15)+(nB>>16)+0x7fff)>>16;
					}
				}
			}
		}
		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageSatin::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrSIZE(CFGID_SAT_SIZE);
	CComBSTR bstrBLUR(CFGID_SAT_BLUR);
	BSTR aIDs[] = {bstrSIZE, bstrBLUR};
	TConfigValue cVal[2];
	a_pConfig->ItemValueGet(bstrSIZE, &cVal[0]);
	a_pConfig->ItemValueGet(bstrBLUR, &cVal[1]);
	cVal[0].fVal *= f;
	cVal[1].fVal *= f;
	return a_pConfig->ItemValuesSet(2, aIDs, cVal);
}

STDMETHODIMP CDocumentOperationRasterImageSatin::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_SIZE), &cVal);
		float fSize = cVal.operator float();
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_BLUR), &cVal);
		float fBlur = cVal.operator float();
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SAT_ANGLE), &cVal);
		float fAngle = cVal;

		float fShiftX = fSize*cosf(fAngle*3.14159265359f/180.0f);
		float fShiftY = -fSize*sinf(fAngle*3.14159265359f/180.0f);
		LONG const nBlur = ceilf(fBlur);
		LONG const nExtraSpaceX = ceilf(fabsf(fShiftX))+nBlur;
		LONG const nExtraSpaceY = ceilf(fabsf(fShiftY))+nBlur;

		a_pRect->tTL.nX -= nExtraSpaceX;
		a_pRect->tTL.nY -= nExtraSpaceY;
		a_pRect->tBR.nX += nExtraSpaceX;
		a_pRect->tBR.nY += nExtraSpaceY;
	}
	return S_OK;
}
