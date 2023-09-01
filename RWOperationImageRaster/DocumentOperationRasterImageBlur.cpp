// DocumentOperationRasterImageBlur.cpp : Implementation of CDocumentOperationRasterImageBlur

#include "stdafx.h"
#include "DocumentOperationRasterImageBlur.h"
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>


const OLECHAR CFGID_BL_DIRECTION[] = L"Direction";
const LONG CFGVAL_BLD_BOTH = 0;
const LONG CFGVAL_BLD_VERTICAL = 1;
const LONG CFGVAL_BLD_HORIZONTAL = 2;
const LONG CFGVAL_BLD_RADIAL = 3;
const LONG CFGVAL_BLD_ZOOM = 4;
const OLECHAR CFGID_BL_METHOD[] = L"Method";
const LONG CFGVAL_BLM_BOX = 0;
const LONG CFGVAL_BLM_STACK = 1;
const LONG CFGVAL_BLM_QUAD = 2;
const LONG CFGVAL_BLM_IRR = 3;
const OLECHAR CFGID_BL_EDGEREPLICATION[] = L"EdgeReplication";
const OLECHAR CFGID_BL_RADIUS[] = L"Radius";
const OLECHAR CFGID_BL_CENTER[] = L"Center";
const OLECHAR CFGID_BL_ANGLE[] = L"Angle";
const OLECHAR CFGID_BL_ZOOM[] = L"Zoom";


#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUIBlur :
	public CCustomConfigResourcelessWndImpl<CConfigGUIBlur>,
	public CDialogResize<CConfigGUIBlur>,
	public IResizableConfigWindow
{
public:
	CConfigGUIBlur()
	{
	}

	enum {
		IDC_CGB_DIRECTION = 100,
		IDC_CGB_AMOUNT_RADIUS,
		IDC_CGB_AMOUNT_ANGULAR,
		IDC_CGB_AMOUNT_ZOOM,
		IDC_CGB_AMOUNT_SLIDER,
		IDC_CGB_METHOD_LABEL,
		IDC_CGB_METHOD,
		IDC_CGB_EDGEREPLICATION,
	};

	BEGIN_DIALOG_EX(0, 0, 120, 58, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Direction:[0405]Směr:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGB_DIRECTION, 50, 0, 70, 80, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Amount:[0405]Velikost:"), IDC_STATIC, 0, 18, 50, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGB_AMOUNT_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 50, 16, 40, 12, 0)
		CONTROL_EDITTEXT(IDC_CGB_AMOUNT_RADIUS, 90, 16, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_EDITTEXT(IDC_CGB_AMOUNT_ANGULAR, 90, 16, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_EDITTEXT(IDC_CGB_AMOUNT_ZOOM, 90, 16, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_LTEXT(_T("[0409]Spread:[0405]Rozptyl:"), IDC_CGB_METHOD_LABEL, 0, 34, 50, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGB_METHOD, 50, 32, 70, 80, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Canvas edge replication[0405]Replikace okrajů plátna"), IDC_CGB_EDGEREPLICATION, 0, 48, 120, 20, BS_MULTILINE | BS_TOP | WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_CONTROL(_T(""), IDC_OPACITY_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 0, 11, 12, 0)
		//CONTROL_RTEXT(_T("%"), IDC_OPACITY_UNIT, 112, 2, 8, 8, WS_VISIBLE, 0)
	END_CONTROLS_MAP()


	BEGIN_MSG_MAP(CConfigGUIBlur)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIBlur>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIBlur>)
		MESSAGE_HANDLER(WM_HSCROLL, OnWidthScroll)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIBlur)
		DLGRESIZE_CONTROL(IDC_CGB_DIRECTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_METHOD, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT_RADIUS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT_ANGULAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT_ZOOM, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT_SLIDER, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIBlur)
		CONFIGITEM_COMBOBOX(IDC_CGB_DIRECTION, CFGID_BL_DIRECTION)
		CONFIGITEM_COMBOBOX_VISIBILITY(IDC_CGB_METHOD, CFGID_BL_METHOD)
		CONFIGITEM_VISIBILITY(IDC_CGB_METHOD_LABEL, CFGID_BL_METHOD)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGB_AMOUNT_RADIUS, CFGID_BL_RADIUS)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGB_AMOUNT_ANGULAR, CFGID_BL_ANGLE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGB_AMOUNT_ZOOM, CFGID_BL_ZOOM)
		CONFIGITEM_CHECKBOX_VISIBILITY(IDC_CGB_EDGEREPLICATION, CFGID_BL_EDGEREPLICATION)
	END_CONFIGITEM_MAP()

	BEGIN_COM_MAP(CConfigGUIBlur)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || a_pSize == NULL || M_Config() == NULL)
			return E_UNEXPECTED;
		CConfigValue cVal;
		M_Config()->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cVal);
		RECT rc = {0, 0, 120, cVal.TypeGet() == ECVTInteger && cVal.operator LONG() <= CFGVAL_BLD_HORIZONTAL ? 58 : 32};//46};
		MapDialogRect(&rc);
		a_pSize->cx = rc.right;
		a_pSize->cy = rc.bottom;
		return S_OK;
	}

	void ExtraInitDialog()
	{
		m_wndRadius = GetDlgItem(IDC_CGB_AMOUNT_SLIDER);
		m_wndRadius.SetRange(0, 100);
		m_wndRadius.SetPageSize(10);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		//CUpDownCtrl wnd = GetDlgItem(IDC_CGRB_SIZESPIN);
		//wnd.SetRange(1, 100);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnWidthScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndRadius == reinterpret_cast<HWND>(a_lParam))
		{
			LONG nPos = m_wndRadius.GetPos();
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cVal);
			switch (cVal.operator LONG())
			{
			case CFGVAL_BLD_BOTH:
			case CFGVAL_BLD_VERTICAL:
			case CFGVAL_BLD_HORIZONTAL:
				{
					float fPos = expf(nPos*0.01f*logf(500.0f))*0.5f;
					if (fPos > 20)
						fPos = int(fPos+0.5f);
					else if (fPos > 3)
						fPos = int(fPos*10.0f+0.5f)*0.1f;
					else
						fPos = int(fPos*100.0f+0.5f)*0.01f;
					CComBSTR bstrID(CFGID_BL_RADIUS);
					M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(fPos));
				}
				break;
			case CFGVAL_BLD_RADIAL:
				{
					CComBSTR bstrID(CFGID_BL_ANGLE);
					M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(0.2f*float(nPos)));
				}
				break;
			case CFGVAL_BLD_ZOOM:
				{
					CComBSTR bstrID(CFGID_BL_ZOOM);
					M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(nPos));
				}
				break;
			}
			return 0;
		}
		a_bHandled = FALSE;
		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_hWnd == NULL)
			return;

		if (m_wndRadius.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cVal);
			LONG nPos = -1;
			switch (cVal.operator LONG())
			{
			case CFGVAL_BLD_BOTH:
			case CFGVAL_BLD_VERTICAL:
			case CFGVAL_BLD_HORIZONTAL:
				{
					CConfigValue cVal;
					M_Config()->ItemValueGet(CComBSTR(CFGID_BL_RADIUS), &cVal);
					float f = cVal.operator float();
					if (f < 0.5) f = 0.5f;
					else if (f > 250) f = 250.0f;
					nPos = logf(f*2.0f)*100.0f/logf(500.0f)+0.5f;
				}
				break;
			case CFGVAL_BLD_RADIAL:
				{
					CConfigValue cVal;
					M_Config()->ItemValueGet(CComBSTR(CFGID_BL_ANGLE), &cVal);
					float f = cVal.operator float();
					if (f < 0.0f) f = 0.0f;
					else if (f > 20.0f) f = 20.0f;
					nPos = f*5 + 0.5f;
				}
				break;
			case CFGVAL_BLD_ZOOM:
				{
					CConfigValue cVal;
					M_Config()->ItemValueGet(CComBSTR(CFGID_BL_ZOOM), &cVal);
					nPos = cVal;
					if (nPos > 100) nPos = 100;
				}
				break;
			}
			if (m_wndRadius.GetPos() != nPos)
				m_wndRadius.SetPos(nPos);
		}
	}

private:
	CTrackBarCtrl m_wndRadius;
};



// CDocumentOperationRasterImageBlur

STDMETHODIMP CDocumentOperationRasterImageBlur::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Blur[0405]Rastrový obrázek - rozostřit");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBlur::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Blur[0405]Rozostřit");
			return S_OK;
		}
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cType);
		CComPtr<ILocalizedString> pMethod;
		OLECHAR const* pszAmount = NULL;
		OLECHAR const* pTemplate = L"%s - %ipx";
		switch (cType.operator LONG())
		{
		case CFGVAL_BLD_BOTH: pMethod.Attach(new CMultiLanguageString(L"[0409]Blur[0405]Rozostření")); pszAmount = CFGID_BL_RADIUS; break;
		case CFGVAL_BLD_VERTICAL: pMethod.Attach(new CMultiLanguageString(L"[0409]Vertical blur[0405]Svislé rozostření")); pszAmount = CFGID_BL_RADIUS; break;
		case CFGVAL_BLD_HORIZONTAL: pMethod.Attach(new CMultiLanguageString(L"[0409]Horizontal blur[0405]Vodorovné rozostření")); pszAmount = CFGID_BL_RADIUS; break;
		case CFGVAL_BLD_RADIAL: pMethod.Attach(new CMultiLanguageString(L"[0409]Radial blur[0405]Rotační rozostření")); pszAmount = CFGID_BL_ANGLE; pTemplate = L"%s - %i°"; break;
		case CFGVAL_BLD_ZOOM: pMethod.Attach(new CMultiLanguageString(L"[0409]Zoom blur[0405]Zvětšovací rozostření")); pszAmount = CFGID_BL_ZOOM; break;
		}
		CConfigValue cAmount;
		a_pConfig->ItemValueGet(CComBSTR(pszAmount), &cAmount);
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		LONG l = cAmount.TypeGet() == ECVTFloat ? LONG(cAmount.operator float()+0.5f) : cAmount;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(pTemplate)));
		pStr->Init(pTempl, pMethod, l);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImageBlur::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const aCoords[] =
	{
		{32, 32, 55, 60, -75, 55},
		{224, 32, -60, 55, -55, -75},
		{224, 224, -55, -60, 75, -55},
		{32, 224, 60, -55, 55, 75},
	};
	static IRPathPoint const aStroke1[] =
	{
		{224, 128, -55, -60, 0, 0},
		{32, 128, 0, 0, 55, 60},
		//{111.07, 63.3734, -4.9103, 40.372, 0, 0},
		//{145.025, 191.792, 0, 0, 4.25397, -40.4705},
	};
	static IRPathPoint const aStroke2[] =
	{
		{128, 32, -60, 55, 0, 0},
		{128, 224, 0, 0, 60, -55},
		//{193, 111, -40.372, -4.91031, 0, 0},
		//{64, 145, 0, 0, 40.4705, 4.25397},
	};
	static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&tCanvas, itemsof(aCoords), aCoords, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&tCanvas, itemsof(aStroke1), aStroke1, pSI->GetMaterial(ESMOutlineSoft));
	cRenderer(&tCanvas, itemsof(aStroke2), aStroke2, pSI->GetMaterial(ESMOutlineSoft));
	return cRenderer.get();
}

//STDMETHODIMP CDocumentOperationRasterImageBlur::PreviewIcon(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
//{
//	if (a_phIcon == NULL)
//		return E_POINTER;
//	try
//	{
//		static float const f = 1.0f/256.0f;
//		COLORREF clrOut = GetSysColor(COLOR_WINDOWTEXT);
//		static TPolyCoords const aCoords1[] =
//		{
//			{f*203, f*43},
//			{-f*43.3426, -f*16.6305}, {f*38.0823, -f*31.9548}, {f*71, f*65},
//			{-f*36.5814, f*30.6954}, {-f*6.66141, -f*43.9963}, {f*25, f*186},
//			{-f*29.9534, -f*47.8515}, {-f*44.4785, f*37.3218}, {f*49, f*37},
//			{f*45.743, -f*38.3829}, {-f*41.763, -f*40.9676}, {f*203, f*43},
//		};
//		static TPolyCoords const aCoords2[] =
//		{
//			{f*236, f*113},
//			{f*21.7781, f*46.7033}, {f*45.2038, -f*23.6382}, {f*194, f*238},
//			{f*25.1948, -f*30.9517}, {f*17.9985, f*38.5979}, {f*208, f*125},
//			{-f*17.3177, -f*37.1378}, {f*38.2321, f*2.18636}, {f*117, f*63},
//			{f*46.2801, -f*16.8751}, {-f*21.2078, -f*45.4803}, {f*236, f*113},
//		};
//		static TPolyCoords const aCoords3[] =
//		{
//			{f*108, f*215},
//			{f*33.2895, f*8.92006}, {-f*20.9797, f*24.5736}, {f*196, f*188},
//			{-f*12.5314, f*39.5136}, {f*40.4473, f*10.838}, {f*102, f*239},
//			{-f*40.1471, -f*10.7576}, {-f*8.348, f*40.1867}, {f*46, f*148},
//			{f*6.18922, f*31.3358}, {-f*32.9361, -f*8.82537}, {f*108, f*215},
//		};
//		static TPolyCoords const aCoords4[] =
//		{
//			{f*159, f*105},
//			{-f*16.9314, -f*3.72429}, {f*16.1967, -f*9.35112}, {f*108, f*113},
//			{-f*19.7115, f*11.3804}, {f*3.18176, -f*20.8773}, {f*73, f*164},
//			{-f*11.9489, -f*23.2005}, {-f*21.973, f*15.3856}, {f*90, f*97},
//			{f*22.2237, -f*15.5612}, {-f*17.64, -f*19.7407}, {f*159, f*105},
//		};
//		static TPolyCoords const aCoords5[] =
//		{
//			{f*171, f*173},
//			{-f*18.1378, f*15.2194}, {f*18.4466, f*14.3968}, {f*108, f*173},
//			{f*16.9826, f*0.467123}, {-f*14.0379, f*11.7792}, {f*156, f*156},
//			{f*14.5471, -f*12.2065}, {-f*2.13354, f*17.4715}, {f*181, f*110},
//			{f*11.7309, f*20.8442}, {f*18.4658, -f*15.4947}, {f*171, f*173},
//		};
//		TIconPolySpec tPolySpec[5];
//		tPolySpec[0].nVertices = itemsof(aCoords1);
//		tPolySpec[0].pVertices = aCoords1;
//		tPolySpec[0].interior = agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);//GetIconFillColor();
//		tPolySpec[0].outline = agg::rgba8(0, 0, 0, 0);//agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		tPolySpec[1].nVertices = itemsof(aCoords2);
//		tPolySpec[1].pVertices = aCoords2;
//		tPolySpec[1].interior = tPolySpec[0].interior;//GetIconFillColor();
//		tPolySpec[1].outline = tPolySpec[0].outline;//agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		tPolySpec[2].nVertices = itemsof(aCoords3);
//		tPolySpec[2].pVertices = aCoords3;
//		tPolySpec[2].interior = tPolySpec[0].interior;//GetIconFillColor();
//		tPolySpec[2].outline = tPolySpec[0].outline;//agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		tPolySpec[3].nVertices = itemsof(aCoords4);
//		tPolySpec[3].pVertices = aCoords4;
//		tPolySpec[3].interior = tPolySpec[0].interior;//GetIconFillColor();
//		tPolySpec[3].outline = tPolySpec[0].outline;//agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		tPolySpec[4].nVertices = itemsof(aCoords5);
//		tPolySpec[4].pVertices = aCoords5;
//		tPolySpec[4].interior = tPolySpec[0].interior;//GetIconFillColor();
//		tPolySpec[4].outline = tPolySpec[0].outline;//agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
//		return S_OK;
//	}
//	catch (...)
//	{
//		return E_UNEXPECTED;
//	}
//}

STDMETHODIMP CDocumentOperationRasterImageBlur::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_BL_DIRECTION(CFGID_BL_DIRECTION);
		pCfgInit->ItemIns1ofN(cCFGID_BL_DIRECTION, CMultiLanguageString::GetAuto(L"[0409]Direction[0405]Směr"), CMultiLanguageString::GetAuto(L"[0409]Direction for blurring.[0405]Směr rozostření."), CConfigValue(CFGVAL_BLD_BOTH), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_DIRECTION, CConfigValue(CFGVAL_BLD_BOTH), CMultiLanguageString::GetAuto(L"[0409]Horizontal and vertical blur[0405]Vodorovné a svislé rozostření"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_DIRECTION, CConfigValue(CFGVAL_BLD_HORIZONTAL), CMultiLanguageString::GetAuto(L"[0409]Horizontal blur[0405]Vodorovné rozostření"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_DIRECTION, CConfigValue(CFGVAL_BLD_VERTICAL), CMultiLanguageString::GetAuto(L"[0409]Vertical blur[0405]Svislé rozostření"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_DIRECTION, CConfigValue(CFGVAL_BLD_RADIAL), CMultiLanguageString::GetAuto(L"[0409]Radial blur[0405]Rotační rozostření"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_DIRECTION, CConfigValue(CFGVAL_BLD_ZOOM), CMultiLanguageString::GetAuto(L"[0409]Zoom blur[0405]Zvětšovací rozostření"), 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_BL_DIRECTION;
		tCond.eConditionType = ECOCLessEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_BLD_HORIZONTAL;
		CComBSTR cCFGID_BL_METHOD(CFGID_BL_METHOD);
		pCfgInit->ItemIns1ofN(cCFGID_BL_METHOD, CMultiLanguageString::GetAuto(L"[0409]Distribution[0405]Rozdělení"), CMultiLanguageString::GetAuto(L"[0409]Method for spreading pixel color to its neighborhood.[0405]Metoda pro rozptýlení barvy pixelu do svého okolí."), CConfigValue(CFGVAL_BLM_STACK), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BL_METHOD, CConfigValue(CFGVAL_BLM_BOX), CMultiLanguageString::GetAuto(L"[0409]Constant[0405]Konstantní"), 1, &tCond);
		pCfgInit->ItemOptionAdd(cCFGID_BL_METHOD, CConfigValue(CFGVAL_BLM_STACK), CMultiLanguageString::GetAuto(L"[0409]Linear[0405]Lineární"), 1, &tCond);
		//pCfgInit->ItemOptionAdd(cCFGID_BL_METHOD, CConfigValue(CFGVAL_BLM_QUAD), CMultiLanguageString::GetAuto(L"[0409]Sigmoid[0405]Sigmoidní"), 1, &tCond);
		pCfgInit->ItemOptionAdd(cCFGID_BL_METHOD, CConfigValue(CFGVAL_BLM_IRR), CMultiLanguageString::GetAuto(L"[0409]Gaussian[0405]Gaussovské"), 1, &tCond);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BL_EDGEREPLICATION), CMultiLanguageString::GetAuto(L"[0409]Canvas edge replication[0405]Replikace okrajů plátna"), CMultiLanguageString::GetAuto(L"[0409]Assume the pixel colors outside of canvas matches the image edges.[0405]Předpokládat, že pixely mimo plátno jsou shodné s okraji obrázku."), CConfigValue(false), NULL, 1, &tCond);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BL_RADIUS), CMultiLanguageString::GetAuto(L"[0409]Radius[0405]Poloměr"), CMultiLanguageString::GetAuto(L"[0409]Maximum distance in pixels that each pixel influences.[0405]Maximální vzdálenost v pixelech, kterou každý pixel ovlivňuje."), CConfigValue(10.0f), NULL, 1, &tCond);
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.iVal = CFGVAL_BLD_RADIAL;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BL_ANGLE), CMultiLanguageString::GetAuto(L"[0409]Angle[0405]Úhel"), NULL, CConfigValue(2.0f), NULL, 1, &tCond);
		tCond.tValue.iVal = CFGVAL_BLD_ZOOM;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BL_ZOOM), CMultiLanguageString::GetAuto(L"[0409]Amount[0405]Velikost"), CMultiLanguageString::GetAuto(L"[0409]Maximum distance in pixels that each pixel influences.[0405]Maximální vzdálenost v pixelech, kterou každý pixel ovlivňuje."), CConfigValue(10L), NULL, 1, &tCond);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageBlur, CConfigGUIBlur>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBlur::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

inline void Rotate(int& fx, int& fy, int fr)
{
    int const cx = fx;
    int const cy = fy;

    //sin(x) ~~ x
    //cos(x)~~ 1 - x^2/2
    fx = cx - ((cy >> 8) * fr >> 8) - ((cx >> 14) * (fr * fr >> 11) >> 8);
    fy = cy + ((cx >> 8) * fr >> 8) - ((cy >> 14) * (fr * fr >> 11) >> 8);
}

template<bool t_bPremultiply>
void DoRadialBlur(ULONG const a_nSizeX, ULONG const a_nSizeY, TPixelChannel const* a_pSrc, TPixelChannel* a_pDst, float a_fAngle)
{
	LONG fcx = a_nSizeX << 15;
	LONG fcy = a_nSizeY << 15;
    int fr = (int)(a_fAngle * 3.141528f * 65536.0f / 181.0f);
    for (ULONG y = 0; y < a_nSizeY; ++y)
    {
		TPixelChannel* pDst = a_pDst+a_nSizeX*y;

        for (ULONG x = 0; x < a_nSizeX; ++x)
        {
            LONG fx = (x << 16) - fcx;
            LONG fy = (y << 16) - fcy;
            const int n = 64;

            int fsr = fr / n;

            int sr = 0;
            int sg = 0;
            int sb = 0;
            int sa = 0;
            int sc = 0;

		    TPixelChannel const* pSrc = a_pSrc+y*a_nSizeX+x;

			if (t_bPremultiply)
			{
				int const a = pSrc->bA;
				sr += pSrc->bR * a;
				sg += pSrc->bG * a;
				sb += pSrc->bB * a;
				sa += a;
			}
			else
			{
				sr += pSrc->bR;
				sg += pSrc->bG;
				sb += pSrc->bB;
			}
			++sc;

            int ox1 = fx;
            int ox2 = fx;
            int oy1 = fy;
            int oy2 = fy;

            for (int i = 0; i < n; ++i)
            {
                int u;
                int v;

                Rotate(ox1, oy1, fsr);
                Rotate(ox2, oy2, -fsr);
                
                u = ox1 + fcx + 32768 >> 16;
                v = oy1 + fcy + 32768 >> 16;

                if (u >= 0 && v >= 0 && u < int(a_nSizeX) && v < int(a_nSizeY))
                {
					pSrc = a_pSrc+v*a_nSizeX+u;

					if (t_bPremultiply)
					{
						int const a = pSrc->bA;
						sr += pSrc->bR * a;
						sg += pSrc->bG * a;
						sb += pSrc->bB * a;
						sa += a;
					}
					else
					{
						sr += pSrc->bR;
						sg += pSrc->bG;
						sb += pSrc->bB;
					}
					++sc;
                }

                u = ox2 + fcx + 32768 >> 16;
                v = oy2 + fcy + 32768 >> 16;

                if (u >= 0 && v >= 0 && u < int(a_nSizeX) && v < int(a_nSizeY))
                {
					pSrc = a_pSrc+v*a_nSizeX+u;

					if (t_bPremultiply)
					{
						int const a = pSrc->bA;
						sr += pSrc->bR * a;
						sg += pSrc->bG * a;
						sb += pSrc->bB * a;
						sa += a;
					}
					else
					{
						sr += pSrc->bR;
						sg += pSrc->bG;
						sb += pSrc->bB;
					}
					++sc;
                }
            }
     
			if (t_bPremultiply)
			{
				if (sa == 0 || sa < sc)
				{
					pDst->bR = pDst->bG = pDst->bB = pDst->bA = 0;
				}
				else
				{
					pDst->bR = sr / sa;
					pDst->bG = sg / sa;
					pDst->bB = sb / sa;
					pDst->bA = sa / sc;
				}
			}
			else
            {
				pDst->bR = sr / sc;
				pDst->bG = sg / sc;
				pDst->bB = sb / sc;
				pDst->bA = 255;
            }
			++pDst;
        }
    }
}

template<bool t_bPremultiply>
void DoZoomBlur(ULONG const a_nSizeX, ULONG const a_nSizeY, TPixelChannel const* a_pSrc, TPixelChannel* a_pDst, LONG a_nAmount)
{
    LONG fcx = a_nSizeX << 15;
    LONG fcy = a_nSizeY << 15;
    
    for (ULONG y = 0; y < a_nSizeY; ++y)
    {
		TPixelChannel* pDst = a_pDst+y*a_nSizeX;
        TPixelChannel const* pSrc = a_pSrc+y*a_nSizeX;

        for (ULONG x = 0; x < a_nSizeX; ++x)
        {
            LONG fx = (x << 16) - fcx;
            LONG fy = (y << 16) - fcy;
            const int n = 64;

            int sr = 0;
            int sg = 0;
            int sb = 0;
            int sa = 0;
            int sc = 1+n;

			if (t_bPremultiply)
			{
				int const a = pSrc->bA;
				sr += pSrc->bR * a;
				sg += pSrc->bG * a;
				sb += pSrc->bB * a;
				sa += a;
			}
			else
			{
				sr += pSrc->bR;
				sg += pSrc->bG;
				sb += pSrc->bB;
			}

            for (int i = 0; i < n; ++i)
            {
                fx -= ((fx >> 4) * a_nAmount) >> 10;
                fy -= ((fy >> 4) * a_nAmount) >> 10;

                int u = (int)(fx + fcx + 32768 >> 16);
                int v = (int)(fy + fcy + 32768 >> 16);

		        TPixelChannel const* pSrc2 = a_pSrc+v*a_nSizeX+u;

				if (t_bPremultiply)
				{
					int const a = pSrc2->bA;
					sr += pSrc2->bR * a;
					sg += pSrc2->bG * a;
					sb += pSrc2->bB * a;
					sa += a;
				}
				else
				{
					sr += pSrc2->bR;
					sg += pSrc2->bG;
					sb += pSrc2->bB;
				}
            }
     
			if (t_bPremultiply)
			{
				if (sa == 0 || sa < sc)
				{
					pDst->bR = pDst->bG = pDst->bB = pDst->bA = 0;
				}
				else
				{
					pDst->bR = sr / sa;
					pDst->bG = sg / sa;
					pDst->bB = sb / sa;
					pDst->bA = sa / sc;
				}
			}
			else
            {
				pDst->bR = sr / sc;
				pDst->bG = sg / sc;
				pDst->bB = sb / sc;
				pDst->bA = 255;
            }

            ++pSrc;
            ++pDst;
        }
    }
}

#include <GammaCorrection.h>

struct TRasterImagePixel16
{
	WORD wB, wG, wR, wA;
};

struct TPixelFloat
{
	float fB, fG, fR, fA;
	inline void operator +=(TPixelFloat rhs)
	{
		fB += rhs.fB;
		fG += rhs.fG;
		fR += rhs.fR;
		fA += rhs.fA;
	}
	inline void operator +=(float rhs)
	{
		fB += rhs;
		fG += rhs;
		fR += rhs;
		fA += rhs;
	}
	inline void operator -=(TPixelFloat rhs)
	{
		fB -= rhs.fB;
		fG -= rhs.fG;
		fR -= rhs.fR;
		fA -= rhs.fA;
	}
	inline void MulShift(float rhs)
	{
		fB *= rhs;
		fG *= rhs;
		fR *= rhs;
		fA *= rhs;
	}
};

template<class T=double>
struct recursive_blur_calc_rgba
{
    typedef T value_type;
    typedef recursive_blur_calc_rgba<T> self_type;

    value_type r,g,b,a;

    inline void calc(value_type b1, value_type b2, value_type b3, value_type b4, const self_type& c1, const self_type& c2, const self_type& c3, const self_type& c4)
    {
        r = b1*c1.r + b2*c2.r + b3*c3.r + b4*c4.r;
        g = b1*c1.g + b2*c2.g + b3*c3.g + b4*c4.g;
        b = b1*c1.b + b2*c2.b + b3*c3.b + b4*c4.b;
        a = b1*c1.a + b2*c2.a + b3*c3.a + b4*c4.a;
    }
};

struct TCalcPixel
{
	DWORD nB, nG, nR, nA;
	inline void operator +=(TCalcPixel rhs)
	{
		nB += rhs.nB;
		nG += rhs.nG;
		nR += rhs.nR;
		nA += rhs.nA;
	}
	inline void operator +=(ULONG rhs)
	{
		nB += rhs;
		nG += rhs;
		nR += rhs;
		nA += rhs;
	}
	inline void operator -=(TCalcPixel rhs)
	{
		nB -= rhs.nB;
		nG -= rhs.nG;
		nR -= rhs.nR;
		nA -= rhs.nA;
	}
	inline void operator *=(ULONG rhs)
	{
		nB *= rhs;
		nG *= rhs;
		nR *= rhs;
		nA *= rhs;
	}
	inline void operator >>=(ULONG rhs)
	{
		nB >>= rhs;
		nG >>= rhs;
		nR >>= rhs;
		nA >>= rhs;
	}
	inline void MulShift(ULONG rhs)
	{
		nB = (nB*rhs+0x8000)>>16;
		nG = (nG*rhs+0x8000)>>16;
		nR = (nR*rhs+0x8000)>>16;
		nA = (nA*rhs+0x8000)>>16;
	}
};

struct CBoxBlur
{
	CBoxBlur(ULONG nSize, ULONG nRadius, TCalcPixel const* pDefault) : nSize(nSize), nRadius(nRadius), pDefault(pDefault) {}

	void init() {}

	template<typename TSrc, typename TDst>
	void process(TSrc tSrc, TDst tDst)
	{
		TCalcPixel tDef0;
		TCalcPixel tDef1;
		if (pDefault)
		{
			tDef0 = *pDefault;
			tDef1 = *pDefault;
		}
		else
		{
			TSrc t = tSrc;
			tDef0 = t.get();
			t.offset(LONG(nSize)-2);
			tDef1 = t.get();
		}

		ULONG const nMul = 0x10000/(nRadius+nRadius+1);
		ULONG nInit = nRadius+nRadius;
		TCalcPixel t = tDef0;
		t *= nInit;
		TSrc tSrcSub = tSrc;
		if (nRadius+nRadius <= nSize)
		{
			ULONG nE0 = nRadius+nRadius;
			ULONG nE2 = nSize;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t += tSrc.get();
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE2)
			{
				t += tSrc.get();
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
			while (x < nE4)
			{
				t += tDef1;
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
		}
		else
		{
			ULONG nE0 = nSize;
			ULONG nE2 = nRadius+nRadius;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t += tSrc.get();
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE2)
			{
				t += tDef1;
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE4)
			{
				t += tDef1;
				TCalcPixel o = t;
				o.MulShift(nMul);
				tDst.put(o);
				//tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
		}
	}

private:
	ULONG nSize;
	ULONG nRadius;
	TCalcPixel const* pDefault;
};

struct CFractionBoxBlurFloat
{
	CFractionBoxBlurFloat(ULONG nSize, float fRadius, TPixelFloat const* pDefault0, TPixelFloat const* pDefault1) : nSize(nSize), nRadius(ceilf(fRadius)), pDefault0(pDefault0), pDefault1(pDefault1)
	{
		fMul = 1.0f/(fRadius+fRadius+1);
		fInit = fRadius+fRadius+1;
		fFirst = fRadius+1-ceilf(fRadius);
	}

	void init() {}

	template<typename TSrc, typename TDst>
	void process(TSrc tSrc, TDst tDst)
	{
		TPixelFloat tDef0;
		if (pDefault0)
		{
			tDef0 = *pDefault0;
		}
		else
		{
			TSrc t = tSrc;
			tDef0 = t.get();
		}
		TPixelFloat tDef1;
		if (pDefault1)
		{
			tDef1 = *pDefault1;
		}
		else
		{
			TSrc t = tSrc;
			t.offset(LONG(nSize)-1);
			tDef1 = t.get();
		}

		float const nMul = fMul;
		float const nInit = fInit;
		float const nFirst = fFirst;
		float const nSecond = 1.0f-nFirst;
		TPixelFloat t = tDef0;
		TPixelFloat tPrev = tDef0;
		TPixelFloat tNext = tPrev;
		tPrev.MulShift(nSecond);
		tNext -= tPrev;
		t.MulShift(nInit);
		TSrc tSrcSub = tSrc;
		if (nRadius+nRadius <= nSize)
		{
			ULONG nE0 = nRadius+nRadius;
			ULONG nE2 = nSize;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t -= tDef0;

				t += tPrev;
				TPixelFloat s = tSrc.get();
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;
			}
			while (x < nE2)
			{
				t -= tNext;
				TPixelFloat s = tSrcSub.get();
				tNext = s;
				s.MulShift(nSecond);
				t -= s;
				tNext -= s;

				t += tPrev;
				s = tSrc.get();
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;
			}
			{
				t -= tNext;
				TPixelFloat s = tSrcSub.get();
				tNext = s;
				s.MulShift(nSecond);
				t -= s;
				tNext -= s;

				t += tPrev;
				s = tDef1;
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;
			}
			while (x < nE4)
			{
				t -= tNext;
				TPixelFloat s = tSrcSub.get();
				tNext = s;
				s.MulShift(nSecond);
				t -= s;
				tNext -= s;

				t += tDef1;

				tDst.put(t, nMul);
				++x;
			}
		}
		else
		{
			ULONG nE0 = nSize;
			ULONG nE2 = nRadius+nRadius;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t -= tDef0;

				t += tPrev;
				TPixelFloat s = tSrc.get();
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;

				//t += tSrc.get();
				//tDst.put(t, nMul);
				//++x;
				//t -= tDef0;
			}
			while (x < nE2)
			{
				t -= tNext;
				TPixelFloat s = tDef0;
				tNext = s;
				s.MulShift(nSecond);
				t -= s;
				tNext -= s;

				t += tPrev;
				s = tDef1;
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;

				//t += tDef1;
				//tDst.put(t, nMul);
				//++x;
				//t -= tDef0;
			}
			while (x < nE4)
			{
				t -= tNext;
				TPixelFloat s = tSrcSub.get();
				tNext = s;
				s.MulShift(nSecond);
				t -= s;
				tNext -= s;

				t += tPrev;
				s = tDef1;
				tPrev = s;
				s.MulShift(nFirst);
				t += s;
				tPrev -= s;

				tDst.put(t, nMul);
				++x;

				//t += tDef1;
				//tDst.put(t, nMul);
				//++x;
				//t -= tSrcSub.get();
			}
		}
	}

private:
	ULONG nSize;
	float fMul;
	float fInit;
	float fFirst;
	ULONG nRadius;
	TPixelFloat const* pDefault0;
	TPixelFloat const* pDefault1;
};

struct CBoxBlurFloat
{
	CBoxBlurFloat(ULONG nSize, ULONG nRadius, TPixelFloat const* pDefault0, TPixelFloat const* pDefault1) : nSize(nSize), nRadius(nRadius), pDefault0(pDefault0), pDefault1(pDefault1) {}

	void init() {}

	template<typename TSrc, typename TDst>
	void process(TSrc tSrc, TDst tDst)
	{
		TPixelFloat tDef0;
		if (pDefault0)
		{
			tDef0 = *pDefault0;
		}
		else
		{
			TSrc t = tSrc;
			tDef0 = t.get();
		}
		TPixelFloat tDef1;
		if (pDefault1)
		{
			tDef1 = *pDefault1;
		}
		else
		{
			TSrc t = tSrc;
			t.offset(LONG(nSize)-1);
			tDef1 = t.get();
		}

		float const nMul = 1.0f/(nRadius+nRadius+1);
		float const nInit = nRadius+nRadius;
		TPixelFloat t = tDef0;
		t.MulShift(nInit);
		TSrc tSrcSub = tSrc;
		if (nRadius+nRadius <= nSize)
		{
			ULONG nE0 = nRadius+nRadius;
			ULONG nE2 = nSize;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t += tSrc.get();
				tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE2)
			{
				t += tSrc.get();
				tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
			while (x < nE4)
			{
				t += tDef1;
				tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
		}
		else
		{
			ULONG nE0 = nSize;
			ULONG nE2 = nRadius+nRadius;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			while (x < nE0)
			{
				t += tSrc.get();
				tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE2)
			{
				t += tDef1;
				tDst.put(t, nMul);
				++x;
				t -= tDef0;
			}
			while (x < nE4)
			{
				t += tDef1;
				tDst.put(t, nMul);
				++x;
				t -= tSrcSub.get();
			}
		}
	}

private:
	ULONG nSize;
	ULONG nRadius;
	TPixelFloat const* pDefault0;
	TPixelFloat const* pDefault1;
};

struct CLinearBlurFloat
{
	CLinearBlurFloat(ULONG nSize, ULONG nRadius, TPixelFloat const* pDefault0, TPixelFloat const* pDefault1) : nSize(nSize), nRadius(nRadius), pDefault0(pDefault0), pDefault1(pDefault1) {}

	void init() {}

	template<typename TSrc, typename TDst>
	void process(TSrc tSrc, TDst tDst)
	{
		TPixelFloat tDef0;
		if (pDefault0)
		{
			tDef0 = *pDefault0;
		}
		else
		{
			TSrc t = tSrc;
			tDef0 = t.get();
		}
		TPixelFloat tDef1;
		if (pDefault1)
		{
			tDef1 = *pDefault1;
		}
		else
		{
			TSrc t = tSrc;
			t.offset(LONG(nSize)-1);
			tDef1 = t.get();
		}

		float const nMul = 1.0f/((nRadius+1)*(nRadius+1));//(nRadius+nRadius+1);
		float const nInit = (nRadius+1)*(nRadius+1)-(nRadius+1);
		TPixelFloat t = tDef0;
		t.MulShift(nInit);
		TPixelFloat tN = tDef0;
		tN.MulShift(nRadius);
		TPixelFloat tP = tN;
		TSrc tSrcSub = tSrc;
		TSrc tSrcMid = tSrc;
		if (nRadius+nRadius <= nSize)
		{
			ULONG nE0 = nRadius;
			ULONG nE1 = nRadius+nRadius;
			ULONG nE2 = nSize;
			ULONG nE3 = nRadius+nSize;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;
			tN += tDef0;
			while (x < nE0)
			{
				tP += tSrc.get();
				t += tP;
				tP -= tDef0;
				tDst.put(t, nMul);
				t -= tN;
				++x;
			}
			tN -= tDef0;
			while (x < nE1)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tSrc.get();
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE2)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tSrc.get();
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
			while (x < nE3)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
			tP += tDef1;
			while (x < nE4)
			{
				t += tP;
				tDst.put(t, nMul);
				tN += tDef1;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
			//tP -= tDef1;
		}
		else if (nRadius <= nSize)
		{
			ULONG nE0 = nRadius;
			ULONG nE1 = nSize;
			ULONG nE2 = nRadius+nRadius;
			ULONG nE3 = nRadius+nSize;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;

			tN += tDef0;
			while (x < nE0)
			{
				tP += tSrc.get();
				t += tP;
				tP -= tDef0;
				tDst.put(t, nMul);
				t -= tN;
				++x;
			}
			tN -= tDef0;
			while (x < nE1)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tSrc.get();
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE2)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE3)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
			while (x < nE4)
			{
				TPixelFloat m = tDef1;
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
		}
		else
		{
			ULONG nE0 = nSize;
			ULONG nE1 = nRadius;
			ULONG nE2 = nRadius+nSize;
			ULONG nE3 = nRadius+nRadius;
			ULONG nE4 = nRadius+nRadius+nSize;

			ULONG x = 0;

			tN += tDef0;
			while (x < nE0)
			{
				tP += tSrc.get();
				t += tP;
				tP -= tDef0;
				tDst.put(t, nMul);
				t -= tN;
				++x;
			}
			tN -= tDef0;
			while (x < nE1)
			{
				TPixelFloat m = tDef0;
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE2)
			{
				TPixelFloat m = tSrcMid.get();
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE3)
			{
				TPixelFloat m = tDef1;
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tDef0;
				++x;
			}
			while (x < nE4)
			{
				TPixelFloat m = tDef1;
				tP += tDef1;
				t += tP;
				tP -= m;
				tDst.put(t, nMul);
				tN += m;
				t -= tN;
				tN -= tSrcSub.get();
				++x;
			}
		}
	}

private:
	ULONG nSize;
	ULONG nRadius;
	TPixelFloat const* pDefault0;
	TPixelFloat const* pDefault1;
};

struct CFloatSRGBPreMulSrc
{
	CFloatSRGBPreMulSrc(TPixelChannel const* p, ULONG s, CFloatSRGB const* pGT) : p(p), s(s), pGT(pGT) {}

	void get(recursive_blur_calc_rgba<double>& t)
	{
		t.a = p->bA*(1.0f/255.0f);
		t.r = pGT->m_aFromSRGB[p->bR]*t.a;
		t.g = pGT->m_aFromSRGB[p->bG]*t.a;
		t.b = pGT->m_aFromSRGB[p->bB]*t.a;
		++p;
	}
	TPixelFloat get()
	{
		TPixelFloat t;
		t.fA = p->bA*(1.0f/255.0f);
		t.fR = pGT->m_aFromSRGB[p->bR]*t.fA;
		t.fG = pGT->m_aFromSRGB[p->bG]*t.fA;
		t.fB = pGT->m_aFromSRGB[p->bB]*t.fA;
		++p;
		return t;
	}
	void offset(LONG n) { p += n; }
	void skipLine(ULONG n) { p += n*s; }

private:
	CFloatSRGB const* __restrict pGT;
	TPixelChannel const* __restrict p;
	ULONG const s;
};

struct CSRGBPreMulSrc
{
	CSRGBPreMulSrc(TPixelChannel const* p, ULONG s, CGammaTables const* pGT) : p(p), s(s), pGT(pGT) {}

	TCalcPixel get()
	{
		//TPixelChannel s = *p;
		//ULONG mul = (ULONG(s.bA)<<24)/255;
		//TCalcPixel t =
		//{
		//	(s.bB*mul+0x8000)>>16,
		//	(s.bG*mul+0x8000)>>16,
		//	(s.bR*mul+0x8000)>>16,
		//	(ULONG(s.bA)<<8)|s.bA
		//	//(pGT->m_aGamma[p->bB]*mul+0x8000)>>16,
		//	//(pGT->m_aGamma[p->bG]*mul+0x8000)>>16,
		//	//(pGT->m_aGamma[p->bR]*mul+0x8000)>>16,
		//	//(ULONG(p->bA)<<8)|p->bA
		//};
		ULONG n = p->n;
		//ULONG mul = (n&0xff000000)/255;
		//ULONG mul = ((n&0xff000000)/255)>>8;
		ULONG mul = (ULONG(n&0xff000000)>>8)/255;
		TCalcPixel t =
		{
			//((n&0xff)*mul)>>16,
			//(((n>>8)&0xff)*mul)>>16,
			//(((n>>16)&0xff)*mul)>>16,
			//((n&0xff000000)>>16)
			(pGT->m_aGamma[(n&0xff)]*mul+0x8000)>>16,
			(pGT->m_aGamma[((n>>8)&0xff)]*mul+0x8000)>>16,
			(pGT->m_aGamma[((n>>16)&0xff)]*mul+0x8000)>>16,
			((n&0xff000000)>>16)|((n&0xff000000)>>24)
			//(ULONG(p->bA)<<8)|p->bA
		};
		++p;
		return t;
	}
	//inline void add(TCalcPixel& m)
	//{
	//	ULONG n = p->n;
	//	ULONG mul = (n&0xff000000)/255;
	//	m.nB += ((n&0xff)*mul)>>16;
	//	m.nG += (((n>>8)&0xff)*mul)>>16;
	//	m.nR += (((n>>16)&0xff)*mul)>>16;
	//	m.nA += ((n&0xff000000)>>16);
	//	++p;
	//}
	//inline void sub(TCalcPixel& m)
	//{
	//	ULONG n = p->n;
	//	ULONG mul = (n&0xff000000)/255;
	//	m.nB -= ((n&0xff)*mul)>>16;
	//	m.nG -= (((n>>8)&0xff)*mul)>>16;
	//	m.nR -= (((n>>16)&0xff)*mul)>>16;
	//	m.nA -= ((n&0xff000000)>>16);
	//	++p;
	//}
	void offset(LONG n) { p += n; }
	void skipLine(ULONG n) { p += n*s; }

private:
	CGammaTables const* __restrict pGT;
	TPixelChannel const* __restrict p;
	ULONG const s;
};

struct CFloatSRGBPreMulTransposingSrc
{
	CFloatSRGBPreMulTransposingSrc(TPixelChannel const* p, ULONG s, CFloatSRGB const* pGT) : p(p), s(s), pGT(pGT) {}

	void get(recursive_blur_calc_rgba<double>& t)
	{
		t.a = p->bA*(1.0f/255.0f);
		t.r = pGT->m_aFromSRGB[p->bR]*t.a;
		t.g = pGT->m_aFromSRGB[p->bG]*t.a;
		t.b = pGT->m_aFromSRGB[p->bB]*t.a;
		p += s;
	}
	TPixelFloat get()
	{
		TPixelFloat t;
		t.fA = p->bA*(1.0f/255.0f);
		t.fR = pGT->m_aFromSRGB[p->bR]*t.fA;
		t.fG = pGT->m_aFromSRGB[p->bG]*t.fA;
		t.fB = pGT->m_aFromSRGB[p->bB]*t.fA;
		p += s;
		return t;
	}
	void offset(LONG n) { p += n*LONG(s); }
	void skipLine(ULONG n) { p += n; }

private:
	CFloatSRGB const* __restrict pGT;
	TPixelChannel const* __restrict p;
	ULONG const s;
};

struct CSRGBPreMulTransposingSrc
{
	CSRGBPreMulTransposingSrc(TPixelChannel const* p, ULONG s, CGammaTables const* pGT) : p(p), s(s), pGT(pGT) {}

	TCalcPixel get()
	{
		ULONG mul = (ULONG(p->bA)<<16)/255;
		TCalcPixel t =
		{
			(pGT->m_aGamma[p->bB]*mul+0x8000)>>16,
			(pGT->m_aGamma[p->bG]*mul+0x8000)>>16,
			(pGT->m_aGamma[p->bR]*mul+0x8000)>>16,
			(ULONG(p->bA)<<8)|p->bA
		};
		p += s;
		return t;
	}
	void offset(LONG n) { p += n*LONG(s); }
	void skipLine(ULONG n) { p += n; }

private:
	CGammaTables const* pGT;
	TPixelChannel const* p;
	ULONG const s;
};

struct CWideSrc
{
	CWideSrc(TRasterImagePixel16 const* p, ULONG const s) : p(p), s(s) {}

	TCalcPixel get()
	{
		TCalcPixel t = {p->wB, p->wG, p->wR, p->wA};
		++p;
		return t;
	}
	void get(recursive_blur_calc_rgba<double>& t)
	{
		t.a = short(p->wA)*(1.0f/16384.0f);
		t.r = short(p->wR)*(1.0f/16384.0f);
		t.g = short(p->wG)*(1.0f/16384.0f);
		t.b = short(p->wB)*(1.0f/16384.0f);
		++p;
	}
	void offset(LONG n) { p += n; }
	void skipLine(ULONG n) { p += n*s; }

private:
	TRasterImagePixel16 const* p;
	ULONG const s;
};

struct CFloatWideSrc
{
	CFloatWideSrc(TRasterImagePixel16 const* p, ULONG const s) : p(p), s(s) {}

	TPixelFloat get()
	{
		TPixelFloat t = {p->wB*(1.0f/32768.0f), p->wG*(1.0f/32768.0f), p->wR*(1.0f/32768.0f), p->wA*(1.0f/32768.0f)};
		++p;
		return t;
	}
	void offset(LONG n) { p += n; }
	void skipLine(ULONG n) { p += n*s; }

private:
	TRasterImagePixel16 const* p;
	ULONG const s;
};

struct CWideTransposingDst
{
	CWideTransposingDst(TRasterImagePixel16* p, ULONG s) : p(p), s(s) {}

	void put(TPixelFloat t, float f)
	{
		p->wB = 32768.0*f*t.fB;
		p->wG = 32768.0*f*t.fG;
		p->wR = 32768.0*f*t.fR;
		p->wA = 32768.0*f*t.fA;
		p += s;
	}
	void put(TCalcPixel t)
	{
		p->wB = t.nB;
		p->wG = t.nG;
		p->wR = t.nR;
		p->wA = t.nA;
		p += s;
	}
	void put_rev(recursive_blur_calc_rgba<double> t)
	{
		p->wB = short(16384.0*t.b);
		p->wG = short(16384.0*t.g);
		p->wR = short(16384.0*t.r);
		p->wA = short(16384.0*t.a);
		p -= s;
	}
	void skip(LONG n) { p += LONG(s)*n; }
	void skipLine(ULONG n) { p += n; }

private:
	TRasterImagePixel16* p;
	ULONG const s;
};

struct CSRGBDeMulDst
{
	CSRGBDeMulDst(TPixelChannel* p, ULONG s, CGammaTables const* pGT) : p(p), s(s), pGT(pGT) {}

	void put(TCalcPixel t, ULONG mulShift)
	{
		//ULONG n = (t.nA*mulShift)&0xff000000;
		//if (n)
		//{
		//	ULONG mul2 = 0x1000000/t.nA;
		//	n |= ULONG(t.nR*mul2)&0xff0000;
		//	n |= ULONG((t.nG*mul2)>>8)&0xff00;
		//	n |= ULONG((t.nB*mul2)>>16);
		//}

		//ULONG tA = (t.nA*mulShift)>>16;
		ULONG n = (t.nA*mulShift)&0xff000000;
		if (n)
		{
			//ULONG mul = 0xffff00/tA;
		//ULONG mul2 = 0x80000000/t.nA;
			//n |= ULONG(t.nR*mul2)&0xff0000;
			//n |= ULONG((t.nG*mul2)>>8)&0xff00;
			//n |= ULONG((t.nB*mul2)>>16);
			//ULONG tR = ((t.nR*mulShift+0x8000)>>16);
			//n |= ULONG(((t.nR*mul+0x8000)>>16))<<16;
			//n |= ULONG(((t.nG*mul+0x8000)>>16))<<8;
			//n |= ULONG(((t.nB*mul+0x8000)>>16));
			//ULONG mul = 0xffff0000/t.nA;
			n |= ULONG(pGT->InvGamma(t.nR/(t.nA>>16)))<<16;
			n |= ULONG(pGT->InvGamma(t.nG/(t.nA>>16)))<<8;
			n |= ULONG(pGT->InvGamma(t.nB/(t.nA>>16)));
			//n |= ULONG(pGT->InvGamma(((t.nR>>7)*mul2+0x8000)>>16))<<16;
			//n |= ULONG(pGT->InvGamma(((t.nG>>7)*mul2+0x8000)>>16))<<8;
			//n |= ULONG(pGT->InvGamma(((t.nB>>7)*mul2+0x8000)>>16));
		}
		p->n = n;
		++p;
	}
	void put(TCalcPixel t)
	{
		ULONG n = (t.nA<<16)&0xff000000;
		if (n)
		{
			//ULONG mul = 0xffff00/t.nA;
			//n |= ULONG(((t.nR*mul+0x8000)>>16))<<16;
			//n |= ULONG(((t.nG*mul+0x8000)>>16))<<8;
			//n |= ULONG(((t.nB*mul+0x8000)>>16));
			ULONG mul = 0xffff0000/t.nA;
			n |= ULONG(pGT->InvGamma((t.nR*mul+0x8000)>>16))<<16;
			n |= ULONG(pGT->InvGamma((t.nG*mul+0x8000)>>16))<<8;
			n |= ULONG(pGT->InvGamma((t.nB*mul+0x8000)>>16));
		}
		p->n = n;
		++p;
	}
	void skipLine(ULONG n) { p += n*s; }

private:
	CGammaTables const* const pGT;
	TPixelChannel* p;
	ULONG const s;
};

struct CFloatSRGBDeMulDst
{
	CFloatSRGBDeMulDst(TPixelChannel* p, ULONG s, CFloatSRGB const* pGT) : p(p), s(s), pGT(pGT) {}

	void put(recursive_blur_calc_rgba<double> t)
	{
		if (t.a > (0.5/255.0))
		{
			float rec = 1.0f/t.a;
			ULONG a = 255.0*t.a+0.5;
			ULONG r = pGT->ToSRGB(float(t.r)*rec);
			ULONG g = pGT->ToSRGB(float(t.g)*rec);
			ULONG b = pGT->ToSRGB(float(t.b)*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		++p;
	}
	void put(TPixelFloat t, float f)
	{
		if (t.fA*f > (0.5f/255.0f))
		{
			float rec = 1.0f/t.fA;
			ULONG a = 255.0f*f*t.fA+0.5f;
			ULONG r = pGT->ToSRGB(t.fR*rec);
			ULONG g = pGT->ToSRGB(t.fG*rec);
			ULONG b = pGT->ToSRGB(t.fB*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		++p;
	}
	void put(TPixelFloat t)
	{
		if (t.fA > (0.5/255.0))
		{
			float rec = 1.0f/t.fA;
			ULONG a = 255.0*t.fA+0.5;
			ULONG r = pGT->ToSRGB(float(t.fR)*rec);
			ULONG g = pGT->ToSRGB(float(t.fG)*rec);
			ULONG b = pGT->ToSRGB(float(t.fB)*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		++p;
	}
	void put_rev(recursive_blur_calc_rgba<double> t)
	{
		if (t.a > (0.5/255.0))
		{
			float rec = 1.0f/t.a;
			ULONG a = 255.0*t.a+0.5;
			a = min(a, 255);
			ULONG r = pGT->ToSRGB(float(t.r)*rec);
			ULONG g = pGT->ToSRGB(float(t.g)*rec);
			ULONG b = pGT->ToSRGB(float(t.b)*rec);
			ATLASSERT(r <= 255);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		--p;
	}
	void skip(LONG n) { p += n; }
	void skipLine(ULONG n) { p += n*s; }

private:
	CFloatSRGB const* const __restrict pGT;
	TPixelChannel* p;
	ULONG const s;
};

struct CSRGBDeMulTransposingDst
{
	CSRGBDeMulTransposingDst(TPixelChannel* p, ULONG s, CGammaTables const* pGT) : p(p), s(s), pGT(pGT) {}

	void put(TCalcPixel t)
	{
		ULONG n = (t.nA<<16)&0xff000000;
		if (n)
		{
			ULONG mul = 0xffff0000/t.nA;
			n |= ULONG(pGT->InvGamma((t.nR*mul+0x8000)>>16))<<16;
			n |= ULONG(pGT->InvGamma((t.nG*mul+0x8000)>>16))<<8;
			n |= ULONG(pGT->InvGamma((t.nB*mul+0x8000)>>16));
		}
		p->n = n;
		p += s;
	}
	void skipLine(ULONG n) { p += n; }

private:
	CGammaTables const* pGT;
	TPixelChannel* p;
	ULONG s;
};

struct CFloatSRGBDeMulTransposingDst
{
	CFloatSRGBDeMulTransposingDst(TPixelChannel* p, ULONG s, CFloatSRGB const* pGT) : p(p), s(s), pGT(pGT) {}

	void put(TPixelFloat t, float f)
	{
		if (t.fA*f > (0.5f/255.0f))
		{
			float rec = 1.0f/t.fA;
			ULONG a = 255.0f*f*t.fA+0.5f;
			ULONG r = pGT->ToSRGB(t.fR*rec);
			ULONG g = pGT->ToSRGB(t.fG*rec);
			ULONG b = pGT->ToSRGB(t.fB*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		p += s;
	}
	void put(recursive_blur_calc_rgba<double> t)
	{
		if (t.a > (0.5/255.0))
		{
			float rec = 1.0f/t.a;
			ULONG a = 255.0*t.a+0.5;
			ULONG r = pGT->ToSRGB(float(t.r)*rec);
			ULONG g = pGT->ToSRGB(float(t.g)*rec);
			ULONG b = pGT->ToSRGB(float(t.b)*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		p += s;
	}
	void put_rev(recursive_blur_calc_rgba<double> t)
	{
		if (t.a > (0.5/255.0))
		{
			float rec = 1.0f/t.a;
			ULONG a = 255.0*t.a+0.5;
			a = min(a, 255);
			ULONG r = pGT->ToSRGB(float(t.r)*rec);
			ULONG g = pGT->ToSRGB(float(t.g)*rec);
			ULONG b = pGT->ToSRGB(float(t.b)*rec);
			p->n = (a<<24)|(r<<16)|(g<<8)|b;
		}
		else
		{
			p->n = 0;
		}
		p -= s;
	}
	void skip(LONG n) { p += n*LONG(s); }
	void skipLine(ULONG n) { p += n; }

private:
	CFloatSRGB const* pGT;
	TPixelChannel* p;
	ULONG s;
};

template<class CalculatorT = recursive_blur_calc_rgba<double>>
class CRecursiveBlur
{
public:
    typedef CalculatorT calculator_type;
    typedef typename CalculatorT::value_type calc_type;

	CRecursiveBlur(ULONG nSize, double fRadius, CalculatorT const* pDefault0, CalculatorT const* pDefault1) : nSize(nSize), fRadius(fRadius)
	{
        if (fRadius < 0.62)
			fRadius = 0.62;
		nRadius = ceil(fRadius);
		nDstSize = nSize+nRadius+nRadius;

        calc_type s = calc_type(fRadius * 0.5);
        calc_type q = calc_type((s < 2.5) ?
                                3.97156 - 4.14554 * sqrt(1 - 0.26891 * s) :
                                0.98711 * s - 0.96330);

        calc_type q2 = calc_type(q * q);
        calc_type q3 = calc_type(q2 * q);

        calc_type b0 = calc_type(1.0 / (1.578250 + 2.444130 * q + 1.428100 * q2 + 0.422205 * q3));

        b1 = calc_type( 2.44413 * q + 2.85619 * q2 + 1.26661 * q3);

        b2 = calc_type(-1.42810 * q2 + -1.26661 * q3);

        b3 = calc_type(0.422205 * q3);

        b  = calc_type(1 - (b1 + b2 + b3) * b0);

        b1 *= b0;
        b2 *= b0;
        b3 *= b0;

		if (pDefault0)
		{
			tDefault0 = *pDefault0;
			bReplicate0 = false;
		}
		else
		{
			bReplicate0 = true;
		}
		if (pDefault1)
		{
			tDefault1 = *pDefault1;
			bReplicate1 = false;
		}
		else
		{
			bReplicate1 = true;
		}
	}

	ULONG nSize;
	ULONG nDstSize;
	double fRadius;
	ULONG nRadius;
	bool bReplicate0;
	bool bReplicate1;
	calculator_type tDefault0;
	calculator_type tDefault1;

	calc_type b1;
	calc_type b2;
	calc_type b3;
	calc_type b;

    CAutoVectorPtr<calculator_type> m_sum1;
    CAutoVectorPtr<calculator_type> m_sum2;
    //CAutoVectorPtr<color_type> m_buf;

	void init()
	{
        //if (img.width() < 3) return;


        //int w = img.width();
        //int wm = w-1;
        //int x, y;

        m_sum1.Allocate(nDstSize+nRadius);
        m_sum2.Allocate(nDstSize+nRadius);
        //m_buf.allocate(nDstSize);
	}

	template<typename TSrc, typename TDst>
	void process(TSrc tSrc, TDst tDst)
	{
        calculator_type c;
		if (bReplicate0)
		{
			TSrc s = tSrc;
			s.get(c);
		}
		else
		{
			c = tDefault0;
		}
		m_sum1[0].calc(b, b1, b2, b3, c, c, c, c);
		if (nRadius < 2)
			tSrc.get(c);
		m_sum1[1].calc(b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0]);
		if (nRadius < 3)
			tSrc.get(c);
		m_sum1[2].calc(b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0]);
		//tSrc.get(c);
  //      m_sum1[0].calc(b, b1, b2, b3, c, c, c, c);
		//tSrc.get(c);
  //      m_sum1[1].calc(b, b1, b2, b3, c, m_sum1[0], m_sum1[0], m_sum1[0]);
		//tSrc.get(c);
  //      m_sum1[2].calc(b, b1, b2, b3, c, m_sum1[1], m_sum1[0], m_sum1[0]);

		int x = 3;
		for(; x < int(nRadius); ++x)
            m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);

		int w = nSize;
		int e = nSize+nRadius-1;
        for(; x < e; ++x)
        {
			tSrc.get(c);
            m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
        }
		tSrc.get(c);
		e += nRadius+1+nRadius;
		if (!bReplicate1)
		{
            m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
			++x;
			c = tDefault1;
		}
        for(; x < e; ++x)
        {
            m_sum1[x].calc(b, b1, b2, b3, c, m_sum1[x-1], m_sum1[x-2], m_sum1[x-3]);
        }

		int wm = nDstSize-1+nRadius;
        m_sum2[wm  ].calc(b, b1, b2, b3, m_sum1[wm  ], m_sum1[wm  ], m_sum1[wm], m_sum1[wm]);
        m_sum2[wm-1].calc(b, b1, b2, b3, m_sum1[wm-1], m_sum2[wm  ], m_sum2[wm], m_sum2[wm]);
        m_sum2[wm-2].calc(b, b1, b2, b3, m_sum1[wm-2], m_sum2[wm-1], m_sum2[wm], m_sum2[wm]);
		tDst.skip(nDstSize-1);
		//tDst.put_rev(m_sum2[wm  ]);
		if (nRadius < 2)
			tDst.put_rev(m_sum2[wm-1]);
		if (nRadius < 3)
			tDst.put_rev(m_sum2[wm-2]);

        for(x = wm-3; x >= int(nDstSize); --x)
        {
            m_sum2[x].calc(b, b1, b2, b3, m_sum1[x], m_sum2[x+1], m_sum2[x+2], m_sum2[x+3]);
		}
        for(; x >= 0; --x)
        {
            m_sum2[x].calc(b, b1, b2, b3, m_sum1[x], m_sum2[x+1], m_sum2[x+2], m_sum2[x+3]);

			tDst.put_rev(m_sum2[x]);
            //m_sum2[x].to_pix(m_buf[x]);
        }
        //img.copy_color_hspan(0, y, w, &m_buf[0]);
    }

};

#include <RWBaseEnumUtils.h>

template<typename TSrc, typename TDst, typename TOp>
class CBlurTask : public CStackUnknown<IThreadedTask>
{
public:
	CBlurTask(TSrc a_tSrc, TDst a_tDst, TOp a_tOp, ULONG a_nLines) : m_tSrc(a_tSrc), m_tDst(a_tDst), m_tOp(a_tOp), m_nLines(a_nLines) {}

	// IThreadedTask methods
public:
	STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
	{
		if (a_nIndex > m_nLines)
			return S_FALSE;
		if (a_nTotal > m_nLines)
			a_nTotal = m_nLines;

		TOp tOp = m_tOp;
		tOp.init();

		ULONG nL0 = m_nLines*a_nIndex/a_nTotal;
		ULONG nL1 = m_nLines*(a_nIndex+1)/a_nTotal;
		TSrc tSrc = m_tSrc;
		tSrc.skipLine(nL0);
		TDst tDst = m_tDst;
		tDst.skipLine(nL0);
		while (nL0 < nL1)
		{
			tOp.process(tSrc, tDst);
			tSrc.skipLine(1);
			tDst.skipLine(1);
			++nL0;
		}
		return S_OK;
	}

private:
	TSrc m_tSrc;
	TDst m_tDst;
	TOp m_tOp;
	ULONG m_nLines;
};

STDMETHODIMP CDocumentOperationRasterImageBlur::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;

		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cVal);
		LONG nDirection = cVal;

		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_METHOD), &cVal);
		LONG nMethod = cVal;

		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_RADIUS), &cVal);
		float fRadius = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_ANGLE), &cVal);
		float const fAngle = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_ZOOM), &cVal);
		LONG const nZoom = cVal;

		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_EDGEREPLICATION), &cVal);
		bool const bReplication = cVal;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		TImageSize tCanvas = {1, 1};
		if (FAILED(pRI->CanvasGet(&tCanvas, NULL, &tOrigin, &tSize, NULL)))
			return E_FAIL;

		TPixelChannel tDefault;
		tDefault.n = 0;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		TPixelChannel const* pDefaultT = NULL;
		TPixelChannel const* pDefaultB = NULL;
		TPixelChannel const* pDefaultL = NULL;
		TPixelChannel const* pDefaultR = NULL;

		if (bReplication)
		{
			if (tOrigin.nX <= 0) { tSize.nX -= tOrigin.nX; tOrigin.nX = 0; } else pDefaultL = &tDefault;
			if (tOrigin.nY <= 0) { tSize.nY -= tOrigin.nY; tOrigin.nY = 0; } else pDefaultT = &tDefault;
			if (LONG(tOrigin.nX+tSize.nX) >= LONG(tCanvas.nX)) { tSize.nX = tCanvas.nX-tOrigin.nX; } else pDefaultR = &tDefault;
			if (LONG(tOrigin.nY+tSize.nY) >= LONG(tCanvas.nY)) { tSize.nY = tCanvas.nY-tOrigin.nY; } else pDefaultB = &tDefault;
		}
		else
		{
			pDefaultT = pDefaultB = pDefaultL = pDefaultR = &tDefault;
		}

		if (tSize.nX*tSize.nY == 0)
			return S_FALSE;

		bool bHasAlpha = true;

		if (nDirection == CFGVAL_BLD_RADIAL || nDirection == CFGVAL_BLD_ZOOM)
		{
			ULONG nPixels = tCanvas.nX*tCanvas.nY;
			CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels*2]);
			HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tCanvas, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
			if (FAILED(hRes))
				return hRes;
			if (nDirection == CFGVAL_BLD_RADIAL)
			{
				if (bHasAlpha)
					DoRadialBlur<true>(tCanvas.nX, tCanvas.nY, pSrc, pSrc.m_p+nPixels, fAngle);
				else
					DoRadialBlur<false>(tCanvas.nX, tCanvas.nY, pSrc, pSrc.m_p+nPixels, fAngle);
			}
			else
			{
				if (bHasAlpha)
					DoZoomBlur<true>(tCanvas.nX, tCanvas.nY, pSrc, pSrc.m_p+nPixels, nZoom);
				else
					DoZoomBlur<false>(tCanvas.nX, tCanvas.nY, pSrc, pSrc.m_p+nPixels, nZoom);
			}
			return pRI->TileSet(EICIRGBA, NULL, &tCanvas, NULL, nPixels, pSrc.m_p+nPixels, TRUE);
		}

		int nExtraSpaceV = 0;
		int nExtraSpaceH = 0;
		switch (nDirection)
		{
			case CFGVAL_BLD_BOTH:		nExtraSpaceV = nExtraSpaceH = ceilf(fRadius); break; // the agg blur is inaccurate
			case CFGVAL_BLD_VERTICAL:	nExtraSpaceV = ceilf(fRadius); break;
			case CFGVAL_BLD_HORIZONTAL:	nExtraSpaceH = ceilf(fRadius); break;
			default: return E_FAIL;
		}

		if (fRadius < 0.1f)
			return S_FALSE;

		TImagePoint tDstOrigin = {tOrigin.nX-nExtraSpaceH, tOrigin.nY-nExtraSpaceV};
		TImageSize tDstSize = {tSize.nX+2*nExtraSpaceH, tSize.nY+2*nExtraSpaceV};

		CAutoPixelBuffer cDst(pRI, tDstSize);

		{
		SLockedImageBuffer cSrc(pRI);
		TPixelChannel const* pSrc = cSrc.pData + cSrc.tAllocSize.nX*(tOrigin.nY-cSrc.tAllocOrigin.nY) + (tOrigin.nX-cSrc.tAllocOrigin.nX);

		CComPtr<IGammaTableCache> pGTC;
		RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
		CGammaTables const* pGT = pGTC->GetSRGBTable();
		CFloatSRGB const* pFSRGB = pGTC->GetFloatSRGB();

		CComPtr<IThreadPool> pThPool;
		if (tSize.nY >= 16 && tSize.nX*tSize.nY >= 128*128)
			RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

		TPixelFloat tDefFloat = CFloatSRGBPreMulSrc(&tDefault, 1, pFSRGB).get();
		recursive_blur_calc_rgba<double> tDefDouble;
		tDefDouble.r = tDefFloat.fR;
		tDefDouble.g = tDefFloat.fG;
		tDefDouble.b = tDefFloat.fB;
		tDefDouble.a = tDefFloat.fA;

		if (nMethod == CFGVAL_BLM_STACK && fRadius <= 1.0f)
		{
			nMethod = CFGVAL_BLM_BOX;
			fRadius = 0.668707f*fRadius - 0.247619f*fRadius*fRadius + 0.361905f*fRadius*fRadius*fRadius - 0.282993f*fRadius*fRadius*fRadius;
		}

		switch (nDirection)
		{
		case CFGVAL_BLD_BOTH:
			{
				CAutoVectorPtr<TRasterImagePixel16> cBuffer(new TRasterImagePixel16[tDstSize.nX*tSize.nY]);

				switch (nMethod)
				{
					case CFGVAL_BLM_BOX:
						{
							CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
							CWideTransposingDst dst(cBuffer, tSize.nY);
							if (ceilf(fRadius) != fRadius)
							{
								CFractionBoxBlurFloat blur(tSize.nX, fRadius, pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

								CBlurTask<CFloatSRGBPreMulSrc, CWideTransposingDst, CFractionBoxBlurFloat> task(src, dst, blur, tSize.nY);
								if (pThPool)
									pThPool->Execute(0, &task);
								else
									task.Execute(0, 1);
							}
							else
							{
								CBoxBlurFloat blur(tSize.nX, ceilf(fRadius), pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

								CBlurTask<CFloatSRGBPreMulSrc, CWideTransposingDst, CBoxBlurFloat> task(src, dst, blur, tSize.nY);
								if (pThPool)
									pThPool->Execute(0, &task);
								else
									task.Execute(0, 1);
							}
						}
						{
							CFloatWideSrc src(cBuffer, tSize.nY);
							CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
							if (ceilf(fRadius) != fRadius)
							{
								CFractionBoxBlurFloat blur(tSize.nY, fRadius, pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

								CBlurTask<CFloatWideSrc, CFloatSRGBDeMulTransposingDst, CFractionBoxBlurFloat> task(src, dst, blur, tDstSize.nX);
								if (pThPool)
									pThPool->Execute(0, &task);
								else
									task.Execute(0, 1);
							}
							else
							{
								CBoxBlurFloat blur(tSize.nY, ceilf(fRadius), pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

								CBlurTask<CFloatWideSrc, CFloatSRGBDeMulTransposingDst, CBoxBlurFloat> task(src, dst, blur, tDstSize.nX);
								if (pThPool)
									pThPool->Execute(0, &task);
								else
									task.Execute(0, 1);
							}
						}
						break;
					case CFGVAL_BLM_STACK:
						{
							CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
							CWideTransposingDst dst(cBuffer, tSize.nY);
							CLinearBlurFloat blur(tSize.nX, ceilf(fRadius), pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

							CBlurTask<CFloatSRGBPreMulSrc, CWideTransposingDst, CLinearBlurFloat> task(src, dst, blur, tSize.nY);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						{
							CFloatWideSrc src(cBuffer, tSize.nY);
							CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
							CLinearBlurFloat blur(tSize.nY, ceilf(fRadius), pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

							CBlurTask<CFloatWideSrc, CFloatSRGBDeMulTransposingDst, CLinearBlurFloat> task(src, dst, blur, tDstSize.nX);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						break;
					case CFGVAL_BLM_IRR:
						{
							CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
							CWideTransposingDst dst(cBuffer, tSize.nY);
							CRecursiveBlur<> blur(tSize.nX, fRadius, pDefaultL ? &tDefDouble : NULL, pDefaultR ? &tDefDouble : NULL);

							CBlurTask<CFloatSRGBPreMulSrc, CWideTransposingDst, CRecursiveBlur<> > task(src, dst, blur, tSize.nY);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						{
							CWideSrc src(cBuffer, tSize.nY);
							CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
							CRecursiveBlur<> blur(tSize.nY, fRadius, pDefaultT ? &tDefDouble : NULL, pDefaultB ? &tDefDouble : NULL);

							CBlurTask<CWideSrc, CFloatSRGBDeMulTransposingDst, CRecursiveBlur<> > task(src, dst, blur, tDstSize.nX);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						break;
				}
			}
			break;
		case CFGVAL_BLD_VERTICAL:
			switch (nMethod)
			{
				case CFGVAL_BLM_BOX:
					{
						CFloatSRGBPreMulTransposingSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						if (ceilf(fRadius) != fRadius)
						{
							CFractionBoxBlurFloat blur(tSize.nY, fRadius, pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

							CBlurTask<CFloatSRGBPreMulTransposingSrc, CFloatSRGBDeMulTransposingDst, CFractionBoxBlurFloat> task(src, dst, blur, tSize.nX);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						else
						{
							CBoxBlurFloat blur(tSize.nY, ceilf(fRadius), pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

							CBlurTask<CFloatSRGBPreMulTransposingSrc, CFloatSRGBDeMulTransposingDst, CBoxBlurFloat> task(src, dst, blur, tSize.nX);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
					}
					break;
				case CFGVAL_BLM_STACK:
					{
						CFloatSRGBPreMulTransposingSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						CLinearBlurFloat blur(tSize.nY, ceilf(fRadius), pDefaultT ? &tDefFloat : NULL, pDefaultB ? &tDefFloat : NULL);

						CBlurTask<CFloatSRGBPreMulTransposingSrc, CFloatSRGBDeMulTransposingDst, CLinearBlurFloat> task(src, dst, blur, tSize.nX);
						if (pThPool)
							pThPool->Execute(0, &task);
						else
							task.Execute(0, 1);
					}
					break;
				case CFGVAL_BLM_IRR:
					{
						CFloatSRGBPreMulTransposingSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulTransposingDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						CRecursiveBlur<> blur(tSize.nY, fRadius, pDefaultT ? &tDefDouble : NULL, pDefaultB ? &tDefDouble : NULL);

						CBlurTask<CFloatSRGBPreMulTransposingSrc, CFloatSRGBDeMulTransposingDst, CRecursiveBlur<> > task(src, dst, blur, tSize.nX);
						if (pThPool)
							pThPool->Execute(0, &task);
						else
							task.Execute(0, 1);
					}
					break;
			}
			break;
		case CFGVAL_BLD_HORIZONTAL:
			switch (nMethod)
			{
				case CFGVAL_BLM_BOX:
					{
						CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						if (ceilf(fRadius) != fRadius)
						{
							CFractionBoxBlurFloat blur(tSize.nX, fRadius, pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

							CBlurTask<CFloatSRGBPreMulSrc, CFloatSRGBDeMulDst, CFractionBoxBlurFloat> task(src, dst, blur, tSize.nY);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
						else
						{
							CBoxBlurFloat blur(tSize.nX, ceilf(fRadius), pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

							CBlurTask<CFloatSRGBPreMulSrc, CFloatSRGBDeMulDst, CBoxBlurFloat> task(src, dst, blur, tSize.nY);
							if (pThPool)
								pThPool->Execute(0, &task);
							else
								task.Execute(0, 1);
						}
					}
					break;
				case CFGVAL_BLM_STACK:
					{
						CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						CLinearBlurFloat blur(tSize.nX, ceilf(fRadius), pDefaultL ? &tDefFloat : NULL, pDefaultR ? &tDefFloat : NULL);

						CBlurTask<CFloatSRGBPreMulSrc, CFloatSRGBDeMulDst, CLinearBlurFloat> task(src, dst, blur, tSize.nY);
						if (pThPool)
							pThPool->Execute(0, &task);
						else
							task.Execute(0, 1);
					}
					break;
				case CFGVAL_BLM_IRR:
					{
						CFloatSRGBPreMulSrc src(pSrc, cSrc.tAllocSize.nX, pFSRGB);
						CFloatSRGBDeMulDst dst(cDst.Buffer(), tDstSize.nX, pFSRGB);
						CRecursiveBlur<> blur(tSize.nX, fRadius, pDefaultL ? &tDefDouble : NULL, pDefaultR ? &tDefDouble : NULL);

						CBlurTask<CFloatSRGBPreMulSrc, CFloatSRGBDeMulDst, CRecursiveBlur<> > task(src, dst, blur, tSize.nY);
						if (pThPool)
							pThPool->Execute(0, &task);
						else
							task.Execute(0, 1);
					}
					break;
			}
		}
		}
		return cDst.Replace(tDstOrigin, &tDstOrigin, &tDstSize, NULL, tDefault);
		//return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBlur::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float fScaleX = 1.0f;
	float fScaleY = 1.0f;
	Matrix3x3fDecompose(*a_pContentTransform, &fScaleX, &fScaleY, NULL, NULL);
	float const f = sqrtf(fScaleX*fScaleY);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrRADIUS(CFGID_BL_RADIUS);
	CComBSTR bstrZOOM(CFGID_BL_ZOOM);
	BSTR aIDs[] = {bstrRADIUS, bstrZOOM};
	TConfigValue cVal[sizeof(aIDs)/sizeof(aIDs[0])];
	a_pConfig->ItemValueGet(bstrRADIUS, &cVal[0]);
	a_pConfig->ItemValueGet(bstrZOOM, &cVal[1]);
	cVal[0].fVal *= f;
	//cVal[1].iVal *= f;
	//cVal[2].iVal *= fScaleY;
	//cVal[3].iVal *= fScaleX;
	cVal[1].iVal *= f;
	return a_pConfig->ItemValuesSet(sizeof(aIDs)/sizeof(aIDs[0]), aIDs, cVal);
}

STDMETHODIMP CDocumentOperationRasterImageBlur::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cDirection;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_DIRECTION), &cDirection);

		CConfigValue cMethod;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_METHOD), &cMethod);

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_RADIUS), &cVal);
		float const fRadius = cVal.operator float();
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_ANGLE), &cVal);
		float const fAngle = cVal.operator float();
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BL_ZOOM), &cVal);
		LONG const nZoom = cVal.operator LONG();

		if (cDirection.operator LONG() == CFGVAL_BLD_ZOOM || cDirection.operator LONG() == CFGVAL_BLD_RADIAL)
		{
			// TODO: reimplement zoom and radial blur and compute rectangles
			if (a_pCanvas == NULL)
				return E_FAIL;
			a_pRect->tTL.nX = a_pRect->tTL.nY = 0;
			a_pRect->tBR.nX = a_pCanvas->nX;
			a_pRect->tBR.nY = a_pCanvas->nY;
			return S_OK;
		}
		if (cMethod.operator LONG() == CFGVAL_BLM_IRR)
		{
			if (a_pCanvas == NULL)
				return E_FAIL;
			a_pRect->tTL.nX = a_pRect->tTL.nY = 0;
			a_pRect->tBR.nX = a_pCanvas->nX;
			a_pRect->tBR.nY = a_pCanvas->nY; // gaussian blur has infinite range
			return S_OK;
		}

		int nExtraSpaceV = 0;
		int nExtraSpaceH = 0;
		switch (cDirection.operator LONG())
		{
		case CFGVAL_BLD_BOTH:		nExtraSpaceV = nExtraSpaceH = ceilf(fRadius); break;
		case CFGVAL_BLD_VERTICAL:	nExtraSpaceV = ceilf(fRadius); break;
		case CFGVAL_BLD_HORIZONTAL:	nExtraSpaceH = ceilf(fRadius); break;
		}

		a_pRect->tTL.nX -= nExtraSpaceH;
		a_pRect->tTL.nY -= nExtraSpaceV;
		a_pRect->tBR.nX += nExtraSpaceH;
		a_pRect->tBR.nY += nExtraSpaceV;
	}
	return S_OK;
}

