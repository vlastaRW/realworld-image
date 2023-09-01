// RasterImageBevel.cpp : Implementation of CRasterImageBevel

#include "stdafx.h"
#include "RasterImageBevel.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>

#include <math.h>
#include <Quaternion.h>

const OLECHAR CFGID_BV_SOURCE[] = L"Source";
const LONG CFGVAL_BVS_OPACITY = 1;
const LONG CFGVAL_BVS_MASK = 2;
const LONG CFGVAL_BVS_OPACITYANDMASK = 3;
const OLECHAR CFGID_BV_SELECTIONID[] = L"SelectionID";
const OLECHAR CFGID_BV_AMOUNT[] = L"Amount";
const OLECHAR CFGID_BV_BLUR[] = L"Blur";
const OLECHAR CFGID_BV_METHOD[] = L"Method";
const LONG CFGVAL_BVM_SLOPE = 0;
const LONG CFGVAL_BVM_ROUND = 1;
const LONG CFGVAL_BVM_RIDGE = 2;
const LONG CFGVAL_BVM_OLD = -1;
const OLECHAR CFGID_BV_ANGLE[] = L"Angle";
const OLECHAR CFGID_BV_LIGHT[] = L"Light";
const OLECHAR CFGID_BV_KEEPFLAT[] = L"KeepFlat";
const OLECHAR CFGID_BV_SUNKEN[] = L"Sunken";
const OLECHAR CFGID_BV_SPECCLR[] = L"SpecClr";
const OLECHAR CFGID_BV_OUTPUT[] = L"Output";
const LONG CFGVAL_BVO_IMAGE = 0;
const LONG CFGVAL_BVO_NORMALMAP = 1;

#include <ConfigCustomGUIImpl.h>
#include <WTL_Rotation.h>
#include <WTL_ColorPicker.h>


class ATL_NO_VTABLE CConfigGUIBevel :
	public CCustomConfigResourcelessWndImpl<CConfigGUIBevel>,
	public CDialogResize<CConfigGUIBevel>
{
public:
	enum
	{
		IDC_CGB_LIGHT = 100,
		IDC_CGB_LIGHTTEXT,
		IDC_CGB_AMOUNT,
		IDC_CGB_AMOUNT_SPIN,
		IDC_CGB_BLUR,
		IDC_CGB_BLUR_LABEL,
		IDC_CGB_BLUR_SPIN,
		IDC_CGB_SHAPE,
		IDC_CGB_ANGLE,
		IDC_CGB_ANGLE_LABEL,
		IDC_CGB_KEEPFLAT,
		IDC_CGB_SUNKEN,
		IDC_CGB_HIGHLIGHTS,
		IDC_CGB_HIGHLIGHTS_LABEL,
		IDC_CGB_SOURCE,
		IDC_CGB_SOURCE_LABEL,
		IDC_CGB_MASKID,
	};

	BEGIN_DIALOG_EX(0, 0, 120, (M_Mode() == ECPMWithCanvas ? 122 : 154), 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Light:[0405]Světlo:"), IDC_STATIC, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGB_LIGHT, WC_STATIC, SS_BLACKRECT | WS_TABSTOP | WS_VISIBLE, 44, 0, 75, 28, 0)
		CONTROL_LTEXT(_T(""), IDC_CGB_LIGHTTEXT, 0, 14, 44, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Amount:[0405]Velikost:"), IDC_STATIC, 0, 34, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGB_AMOUNT, 44, 32, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGB_AMOUNT_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 32, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]Blur:[0405]Zaoblení:"), IDC_CGB_BLUR_LABEL, 0, 50, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGB_BLUR, 44, 48, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGB_BLUR_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 48, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]Method:[0405]Metoda:"), IDC_STATIC, 0, 66, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGB_SHAPE, 44, 64, 75, 30, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Angle:[0405]Úhel:"), IDC_CGB_ANGLE_LABEL, 0, 82, 34, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGB_ANGLE, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 34, 80, 86, 12, 0)
		CONTROL_CHECKBOX(_T("[0409]Adjust light level to preserve flat surfaces[0405]Nastavit úrověň světla pro zachování ploch"), IDC_CGB_KEEPFLAT, 0, 96, 120, 20, BS_MULTILINE | BS_TOP | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Sunken shape[0405]Vyrytý tvar"), IDC_CGB_SUNKEN, 0, 96, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Highlights:[0405]Odlesky:"), IDC_CGB_HIGHLIGHTS_LABEL, 0, 112, 44, 8, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Color[0405]Color"), IDC_CGB_HIGHLIGHTS, 44, 110, 35, 12, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Outline:[0405]Obrys:"), IDC_CGB_SOURCE_LABEL, 0, 128, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGB_SOURCE, 44, 126, 75, 54, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGB_MASKID, 44, 142, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIBevel)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIBevel>)
		NOTIFY_HANDLER(IDC_CGB_AMOUNT_SPIN, UDN_DELTAPOS, OnUpDownChange)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIBevel>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGB_LIGHT, CRotation::RTN_ROTATED, OnRotated)
		NOTIFY_HANDLER(IDC_CGB_HIGHLIGHTS, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIBevel)
		DLGRESIZE_CONTROL(IDC_CGB_SOURCE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_MASKID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_AMOUNT_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGB_BLUR, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_BLUR_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGB_SHAPE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGB_ANGLE, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_CGB_LIGHT_LABEL, DLSZ_MULDIVMOVE_X(2, 3))
		DLGRESIZE_CONTROL(IDC_CGB_LIGHT, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIBevel)
		CONFIGITEM_COMBOBOX(IDC_CGB_SOURCE, CFGID_BV_SOURCE)
		CONFIGITEM_EDITBOX(IDC_CGB_MASKID, CFGID_BV_SELECTIONID)
		CONFIGITEM_EDITBOX(IDC_CGB_AMOUNT, CFGID_BV_AMOUNT)
		CONFIGITEM_EDITBOX(IDC_CGB_BLUR, CFGID_BV_BLUR)
		CONFIGITEM_COMBOBOX(IDC_CGB_SHAPE, CFGID_BV_METHOD)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGB_ANGLE, CFGID_BV_ANGLE)
		CONFIGITEM_CHECKBOX_VISIBILITY(IDC_CGB_KEEPFLAT, CFGID_BV_KEEPFLAT)
		CONFIGITEM_CHECKBOX_VISIBILITY(IDC_CGB_SUNKEN, CFGID_BV_SUNKEN)
		CONFIGITEM_CONTEXTHELP(IDC_CGB_LIGHT, CFGID_BV_LIGHT)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		// initialize color button
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CGB_HIGHLIGHTS));
		m_wndColor.SetDefaultText(NULL);

		CWindow wnd = GetDlgItem(IDC_CGB_LIGHT);
		RECT rc;
		wnd.GetWindowRect(&rc);
		ScreenToClient(&rc);
		wnd.DestroyWindow();
		m_wndRotation.Create(m_hWnd, &rc, _T("direction"), WS_VISIBLE|WS_CHILD|WS_TABSTOP, WS_EX_CLIENTEDGE, IDC_CGB_LIGHT);
		m_wndRotation.LoadGeometryFromResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDR_DIRECTION));
		m_wndRotationAngles = GetDlgItem(IDC_CGB_LIGHTTEXT);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CUpDownCtrl(GetDlgItem(IDC_CGB_AMOUNT_SPIN)).SetRange(0, 200);
		CUpDownCtrl(GetDlgItem(IDC_CGB_BLUR_SPIN)).SetRange(0, 50);

		CComPtr<IDocument> pDoc;
		if (M_Mode() != ECPMWithCanvas)
			GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
		if (M_Mode() == ECPMWithCanvas || pDoc)
		{
			CEdit wnd = GetDlgItem(IDC_CGB_MASKID);
			wnd.SetReadOnly(TRUE);
			wnd.ModifyStyle(WS_TABSTOP, 0);
			wnd.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CGB_SOURCE_LABEL).ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CGB_SOURCE).ShowWindow(SW_HIDE);
		}

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnUpDownChange(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		NMUPDOWN tNMUD = *reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
		tNMUD.iDelta *= 5;
		return CCustomConfigResourcelessWndImpl<CConfigGUIBevel>::OnUpDownChange(a_idCtrl, &tNMUD.hdr, a_bHandled);
	}

	LRESULT OnRotated(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR cCFGID_BV_LIGHT(CFGID_BV_LIGHT);
			CConfigValue cValLight(m_wndRotation.GetQuaternion().fX, m_wndRotation.GetQuaternion().fY, m_wndRotation.GetQuaternion().fZ, m_wndRotation.GetQuaternion().fW);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_BV_LIGHT;
			TConfigValue aVals[1];
			aVals[0] = cValLight;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_COLOR(CFGID_BV_SPECCLR);
			CConfigValue cValColor(pClrBtn->clr.fR, pClrBtn->clr.fG, pClrBtn->clr.fB, true);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_COLOR;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_wndRotation.m_hWnd)
		{
			CConfigValue cValLight;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BV_LIGHT), &cValLight);
			TQuaternionf tQuat = {cValLight[0], cValLight[1], cValLight[2], cValLight[3]};
			//if (m_wndColor.GetColor() != cValColor.operator LONG())
			m_wndRotation.SetQuaternion(tQuat);
			TVector3f tDir = {0.0f, 0.0f, 1.0f};
			TVector3f tDir2 = TransformVector3(tQuat, tDir);
			TCHAR sz[64] = _T("");
			float f1 = asinf(tDir2.z);
			float f2 = atan2f(tDir2.y, tDir2.x);
			_stprintf(sz, _T("%i°, %i°"), int(f2*180.0f/3.141593f + 180.5f), int(f1*180.0f/3.141593f + 0.5f));
			m_wndRotationAngles.SetWindowText(sz);
		}
		if (m_wndColor.m_hWnd)
		{
			CConfigValue cValType;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BV_METHOD), &cValType);
			if (CFGVAL_BVM_OLD == cValType.operator LONG())
			{
				m_wndColor.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CGB_HIGHLIGHTS_LABEL).ShowWindow(SW_HIDE);
			}
			else
			{
				m_wndColor.ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CGB_HIGHLIGHTS_LABEL).ShowWindow(SW_SHOW);
			}
			CConfigValue cValColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_BV_SPECCLR), &cValColor);
			if (m_wndColor.GetColor() != CButtonColorPicker::SColor(cValColor[0], cValColor[1], cValColor[2], 1.0f))
				m_wndColor.SetColor(CButtonColorPicker::SColor(cValColor[0], cValColor[1], cValColor[2], 1.0f));
		}
	}

private:
	CRotation m_wndRotation;
	CWindow m_wndRotationAngles;
	CButtonColorPicker m_wndColor;
};


// CRasterImageBevel

STDMETHODIMP CRasterImageBevel::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEBEVEL_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CRasterImageBevel::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Bevel[0405]Zkosení");
			return S_OK;
		}
		CConfigValue cMethod;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_METHOD), &cMethod);
		CComPtr<ILocalizedString> pMethod;
		switch (cMethod.operator LONG())
		{
		case CFGVAL_BVM_SLOPE: pMethod.Attach(new CMultiLanguageString(L"[0409]Bevel edges[0405]Zkosené hrany")); break;
		case CFGVAL_BVM_ROUND: pMethod.Attach(new CMultiLanguageString(L"[0409]Round edges[0405]Zaoblené hrany")); break;
		case CFGVAL_BVM_RIDGE: pMethod.Attach(new CMultiLanguageString(L"[0409]Ridged edges[0405]Vyvýšené hrany")); break;
		case CFGVAL_BVM_OLD: pMethod.Attach(new CMultiLanguageString(L"[0409]Simple bevel[0405]Jednoduché zkosení")); break;
		}
		CConfigValue cAmount;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_AMOUNT), &cAmount);
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %ipx")));
		pStr->Init(pTempl, pMethod, cAmount.operator LONG());
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageBevel::PreviewIconID(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	//// {2AC62460-B0FF-44a6-9460-BBB3A7C9DDDC}
	//static const GUID tIconID = {0x2ac62460, 0xb0ff, 0x44a6, {0x94, 0x60, 0xbb, 0xb3, 0xa7, 0xc9, 0xdd, 0xdc}};
	*a_pIconID = CLSID_RasterImageBevel;//tIconID;
	return S_OK;
}

#include <IconRenderer.h>

HICON CRasterImageBevel::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const aBack[] =
	{
		{16, 184}, {16, 72}, {72, 16}, {184, 16}, {240, 72}, {240, 184}, {184, 240}, {72, 240},
	};
	static IRPolygon const tBack = {itemsof(aBack), aBack};
	static IRPolyPoint const aFront[] =
	{
		{72, 164}, {72, 92}, {92, 72}, {164, 72}, {184, 92}, {184, 164}, {164, 184}, {92, 184},
	};
	static IRPolygon const tFront = {itemsof(aFront), aFront};
	static IRPolyPoint const aHilight[] =
	{
		{72, 164}, {24, 181}, {24, 75}, {75, 24}, {181, 24}, {164, 72},
	};
	static IRPolygon const tHilight = {itemsof(aHilight), aHilight};
	static IRPolyPoint const aShadow[] =
	{
		{184, 92}, {232, 75}, {232, 181}, {181, 232}, {75, 232}, {92, 184},
	};
	static IRPolygon const tShadow = {itemsof(aShadow), aShadow};
	static IRGridItem const grid[] = {{0, 16}, {0, 72}, {0, 92}, {0, 164}, {0, 184}, {0, 240}};
	static IRCanvas const tCanvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&tCanvas, 1, &tBack, pSI->GetMaterial(ESMScheme1Color2));
	IRFill hl(0x4fffffff);
	IRFill sh(0x4f000000);
	cRenderer(&tCanvas, 1, &tHilight, &hl);
	cRenderer(&tCanvas, 1, &tShadow, &sh);
	cRenderer(&tCanvas, 1, &tFront, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();


	//static float const f = 1.0f/256.0f;
	////static TPolyCoords const aCoords1[] =
	////{
	////	{f*24, f*24}, {f*232, f*24}, {f*232, f*232}, {f*24, f*232},
	////};
	////static TPolyCoords const aCoords2[] =
	////{
	////	{f*232, f*24}, {f*232, f*232}, {f*24, f*232},
	////};
	////static TPolyCoords const aCoords3[] =
	////{
	////	{f*72, f*72}, {f*184, f*72}, {f*184, f*184}, {f*72, f*184},
	////};
	////COLORREF clrOut = GetSysColor(COLOR_WINDOWTEXT);
	////COLORREF clr3D = GetSysColor(COLOR_3DFACE);
	//////return agg::rgba8(GetRValue(clr3D), GetGValue(clr3D), GetBValue(clr3D), 255);
	////float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
	////float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
	////float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
	////float const fOutR = powf(GetRValue(clrOut)/255.0f, 2.2f);
	////float const fOutG = powf(GetGValue(clrOut)/255.0f, 2.2f);
	////float const fOutB = powf(GetBValue(clrOut)/255.0f, 2.2f);
	////float const fOutA = 0.33f;
	////float const f3DA = 0.43f;
	////float const fA = 0.24f;
	////TIconPolySpec tPolySpec[3];
	////tPolySpec[0].nVertices = itemsof(aCoords1);
	////tPolySpec[0].pVertices = aCoords1;
	////tPolySpec[0].interior = GetIconFillColor();
	////tPolySpec[0].outline = agg::rgba8(255.0f*fOutR+0.5f, 255.0f*fOutG+0.5f, 255.0f*fOutB+0.5f, 255);
	////tPolySpec[1].nVertices = itemsof(aCoords2);
	////tPolySpec[1].pVertices = aCoords2;
	////tPolySpec[1].interior = tPolySpec[0].outline;
	////tPolySpec[1].interior.a = 192;
	////tPolySpec[1].outline = agg::rgba8(0, 0, 0, 0);
	////tPolySpec[2].nVertices = itemsof(aCoords3);
	////tPolySpec[2].pVertices = aCoords3;
	////tPolySpec[2].interior = agg::rgba8(255.0f*(f3DR*f3DA+fA+fOutR*fOutA)+0.5f, 255.0f*(f3DG*f3DA+fA+fOutG*fOutA)+0.5f, 255.0f*(f3DB*f3DA+fA+fOutB*fOutA)+0.5f, 255);
	////tPolySpec[2].outline = agg::rgba8(0, 0, 0, 0);

	//static TPolyCoords const aCoords1[] =
	//{
	//	//{f*24, f*184}, {f*24, f*72}, {f*72, f*24}, {f*184, f*24}, {f*232, f*72}, {f*232, f*184}, {f*184, f*232}, {f*72, f*232},
	//	{f*16, f*184}, {f*16, f*72}, {f*72, f*16}, {f*184, f*16}, {f*240, f*72}, {f*240, f*184}, {f*184, f*240}, {f*72, f*240},
	//};
	//static TPolyCoords const aCoords2[] =
	//{
	//	//{f*184, f*72}, {f*232, f*72}, {f*232, f*184}, {f*184, f*232}, {f*72, f*232}, {f*72, f*184}, {f*184, f*184},
	//	{f*184, f*72}, {f*240, f*72}, {f*240, f*184}, {f*184, f*240}, {f*72, f*240}, {f*72, f*184}, {f*184, f*184},
	//};
	//static TPolyCoords const aCoords3[] =
	//{
	//	{f*72, f*72}, {f*184, f*72}, {f*184, f*184}, {f*72, f*184}, {f*72, f*72},
	//	//{f*72, f*232}, {f*24, f*184}, {f*72, f*184}, {f*72, f*232},
	//	//{f*184, f*24}, {f*232, f*72}, {f*184, f*72},
	//	{f*72, f*240}, {f*16, f*184}, {f*72, f*184}, {f*72, f*240},
	//	{f*184, f*16}, {f*240, f*72}, {f*184, f*72},
	//};
	//COLORREF clrOut = GetSysColor(COLOR_WINDOWTEXT);
	//float const fOutR = powf(GetRValue(clrOut)/255.0f, 2.2f);
	//float const fOutG = powf(GetGValue(clrOut)/255.0f, 2.2f);
	//float const fOutB = powf(GetBValue(clrOut)/255.0f, 2.2f);
	//TIconPolySpec tPolySpec[3];
	//tPolySpec[0].nVertices = itemsof(aCoords1);
	//tPolySpec[0].pVertices = aCoords1;
	//tPolySpec[0].interior = GetIconFillColor();
	//tPolySpec[0].outline = agg::rgba8(255.0f*fOutR+0.5f, 255.0f*fOutG+0.5f, 255.0f*fOutB+0.5f, 255);
	//tPolySpec[1].nVertices = itemsof(aCoords2);
	//tPolySpec[1].pVertices = aCoords2;
	//tPolySpec[1].interior = tPolySpec[0].outline;
	//tPolySpec[1].interior.a = 192;
	//tPolySpec[1].outline = agg::rgba8(0, 0, 0, 0);
	//tPolySpec[2].nVertices = itemsof(aCoords3);
	//tPolySpec[2].pVertices = aCoords3;
	//tPolySpec[2].interior = tPolySpec[1].interior;
	//tPolySpec[2].interior.a = 96;
	//tPolySpec[2].outline = agg::rgba8(0, 0, 0, 0);

	//*a_phIcon = IconFromPolygon(itemsof(tPolySpec), tPolySpec, a_nSize, false);
}

STDMETHODIMP CRasterImageBevel::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_BV_SOURCE(CFGID_BV_SOURCE);
		pCfgInit->ItemIns1ofN(cCFGID_BV_SOURCE, _SharedStringTable.GetStringAuto(IDS_CFGID_BV_SOURCE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_SOURCE_DESC), CConfigValue(CFGVAL_BVS_OPACITY), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_SOURCE, CConfigValue(CFGVAL_BVS_OPACITY), _SharedStringTable.GetStringAuto(IDS_CFGVAL_BVS_OPACITY), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_SOURCE, CConfigValue(CFGVAL_BVS_OPACITYANDMASK), _SharedStringTable.GetStringAuto(IDS_CFGVAL_BVS_OPACITYANDMASK), 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_BV_SOURCE;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_BVS_OPACITYANDMASK;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_SELECTIONID), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_SELECTIONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_SELECTIONID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_AMOUNT), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_AMOUNT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_AMOUNT_DESC), CConfigValue(10L), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_BLUR), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_BLUR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_BLUR_DESC), CConfigValue(2L), NULL, 0, NULL);
		CComBSTR cCFGID_BV_METHOD(CFGID_BV_METHOD);
		pCfgInit->ItemIns1ofN(cCFGID_BV_METHOD, CMultiLanguageString::GetAuto(L"[0409]Method[0405]Metoda"), CMultiLanguageString::GetAuto(L"[0409]Choose one of the beveling methods.[0405]Vyberte jeden ze zkosovacích způsobů."), CConfigValue(CFGVAL_BVM_ROUND), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_METHOD, CConfigValue(CFGVAL_BVM_SLOPE), CMultiLanguageString::GetAuto(L"[0409]Bevel edges[0405]Zkosené hrany"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_METHOD, CConfigValue(CFGVAL_BVM_ROUND), CMultiLanguageString::GetAuto(L"[0409]Round edges[0405]Zaoblené hrany"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_METHOD, CConfigValue(CFGVAL_BVM_RIDGE), CMultiLanguageString::GetAuto(L"[0409]Ridged edges[0405]Vyvýšené hrany"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_METHOD, CConfigValue(CFGVAL_BVM_OLD), CMultiLanguageString::GetAuto(L"[0409]Simple bevel[0405]Jednoduché zkosení"), 0, NULL);
		tCond.bstrID = cCFGID_BV_METHOD;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_BVM_SLOPE;
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BV_ANGLE), CMultiLanguageString::GetAuto(L"[0409]Angle[0405]Úhel"), CMultiLanguageString::GetAuto(L"[0409]Angle of the cut edge.[0405]Úhel, pod kterým je hrana zkosena."), CConfigValue(45L), NULL, CConfigValue(20L), CConfigValue(70L), CConfigValue(1L), 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_LIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_LIGHT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_LIGHT_DESC), CConfigValue(0.27059805f, 0.27059805f, 0.0f, 0.92387953f), NULL, 0, NULL);
		tCond.tValue.iVal = CFGVAL_BVM_OLD;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_KEEPFLAT), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_KEEPFLAT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BV_KEEPFLAT_DESC), CConfigValue(false), NULL, 1, &tCond);
		tCond.eConditionType = ECOCNotEqual;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_SUNKEN), CMultiLanguageString::GetAuto(L"[0409]Sunken[0405]Zanořené"), CMultiLanguageString::GetAuto(L"[0409]If enabled, the shape will appear sunken insted of pulled out.[0405]Je-li povoleno, bude se tvar jevit vyrytý místo vystouplý."), CConfigValue(false), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BV_SPECCLR), CMultiLanguageString::GetAuto(L"[0409]Specular color[0405]Barva odrazů"), CMultiLanguageString::GetAuto(L"[0409]Color of the light that is reflected by the surface without diffusion.[0405]Barva světla, které je odraženo bez rozptylu."), CConfigValue(0.5f, 0.5f, 0.5f, true), NULL, 1, &tCond);
		CComBSTR cCFGID_BV_OUTPUT(CFGID_BV_OUTPUT);
		pCfgInit->ItemIns1ofN(cCFGID_BV_OUTPUT, CMultiLanguageString::GetAuto(L"[0409]Output[0405]Výstup"), CMultiLanguageString::GetAuto(L"[0409]Output[0405]Výstup"), CConfigValue(CFGVAL_BVO_IMAGE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_OUTPUT, CConfigValue(CFGVAL_BVO_IMAGE), CMultiLanguageString::GetAuto(L"[0409]Image[0405]Obrázek"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BV_OUTPUT, CConfigValue(CFGVAL_BVO_NORMALMAP), CMultiLanguageString::GetAuto(L"[0409]Normal map[0405]Normálová mapa"), 0, NULL);

		CConfigCustomGUI<&CLSID_RasterImageBevel, CConfigGUIBevel>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageBevel::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

#include <agg_blur.h>

void stack_blur_gray32(DWORD* a_pBuffer, unsigned const w, unsigned const h, unsigned rx, unsigned ry)
{
    unsigned x, y, xp, yp, i;
    unsigned stack_ptr;
    unsigned stack_start;

	const DWORD* src_pix_ptr;
          DWORD* dst_pix_ptr;
    agg::int64u pix;
    agg::int64u stack_pix;
    agg::int64u sum;
    agg::int64u sum_in;
    agg::int64u sum_out;

    unsigned wm  = w - 1;
    unsigned hm  = h - 1;

    unsigned div;
    unsigned mul_sum;
    unsigned shr_sum;

    agg::pod_vector<DWORD> stack;

    if(rx > 0)
    {
        if(rx > 254) rx = 254;
        div = rx * 2 + 1;
        mul_sum = agg::stack_blur_tables<int>::g_stack_blur8_mul[rx];
        shr_sum = agg::stack_blur_tables<int>::g_stack_blur8_shr[rx];
        stack.allocate(div);

        for(y = 0; y < h; y++)
        {
            sum = sum_in = sum_out = 0;

            src_pix_ptr = a_pBuffer + y*w;
            pix = *src_pix_ptr;
            for(i = 0; i <= rx; i++)
            {
                stack[i] = pix;
                sum     += pix * (i + 1);
                sum_out += pix;
            }
            for(i = 1; i <= rx; i++)
            {
                if(i <= wm) ++src_pix_ptr;
                pix = *src_pix_ptr; 
                stack[i + rx] = pix;
                sum    += pix * (rx + 1 - i);
                sum_in += pix;
            }

            stack_ptr = rx;
            xp = rx;
            if(xp > wm) xp = wm;
            src_pix_ptr = a_pBuffer + y*w+xp;
            dst_pix_ptr = a_pBuffer + y*w;
            for(x = 0; x < w; x++)
            {
                *dst_pix_ptr = (sum * mul_sum) >> shr_sum;
                ++dst_pix_ptr;

                sum -= sum_out;
   
                stack_start = stack_ptr + div - rx;
                if(stack_start >= div) stack_start -= div;
                sum_out -= stack[stack_start];

                if(xp < wm) 
                {
                    ++src_pix_ptr;
                    pix = *src_pix_ptr;
                    ++xp;
                }
    
                stack[stack_start] = pix;
    
                sum_in += pix;
                sum    += sum_in;
    
                ++stack_ptr;
                if(stack_ptr >= div) stack_ptr = 0;
                stack_pix = stack[stack_ptr];

                sum_out += stack_pix;
                sum_in  -= stack_pix;
            }
        }
    }

    if(ry > 0)
    {
        if(ry > 254) ry = 254;
        div = ry * 2 + 1;
		mul_sum = agg::stack_blur_tables<int>::g_stack_blur8_mul[ry];
		shr_sum = agg::stack_blur_tables<int>::g_stack_blur8_shr[ry];
        stack.allocate(div);

        int const stride = w;
        for(x = 0; x < w; x++)
        {
            sum = sum_in = sum_out = 0;

            src_pix_ptr = a_pBuffer + x;
            pix = *src_pix_ptr;
            for(i = 0; i <= ry; i++)
            {
                stack[i] = pix;
                sum     += pix * (i + 1);
                sum_out += pix;
            }
            for(i = 1; i <= ry; i++)
            {
                if(i <= hm) src_pix_ptr += stride; 
                pix = *src_pix_ptr; 
                stack[i + ry] = pix;
                sum    += pix * (ry + 1 - i);
                sum_in += pix;
            }

            stack_ptr = ry;
            yp = ry;
            if(yp > hm) yp = hm;
            src_pix_ptr = a_pBuffer + yp*w+x;
            dst_pix_ptr = a_pBuffer + x;
            for(y = 0; y < h; y++)
            {
                *dst_pix_ptr = (sum * mul_sum) >> shr_sum;
                dst_pix_ptr += stride;

                sum -= sum_out;
   
                stack_start = stack_ptr + div - ry;
                if(stack_start >= div) stack_start -= div;
                sum_out -= stack[stack_start];

                if(yp < hm) 
                {
                    src_pix_ptr += stride;
                    pix = *src_pix_ptr;
                    ++yp;
                }
    
                stack[stack_start] = pix;
    
                sum_in += pix;
                sum    += sum_in;
    
                ++stack_ptr;
                if(stack_ptr >= div) stack_ptr = 0;
                stack_pix = stack[stack_ptr];

                sum_out += stack_pix;
                sum_in  -= stack_pix;
            }
        }
    }
}

const DWORD one   = 256;
const DWORD sqrt2 = (int)(256*1.414213562373);
const DWORD sqrt5 = (int)(256*2.236067977499789696);

void applyMap(DWORD* map, int strideH, TPixelChannel* a_pD, int stride, int width, int height, LONG amount, TVector3f a_tLight, bool a_bKeepFlat)
{
	stride -= width;
	strideH -= width;

	float fLevel = a_bKeepFlat ? 255.0f*a_tLight.z : 255.0f*cosf(3.141592f*0.25f);

	size_t const nRowU = -width-strideH;
	size_t const nRowD = width+strideH;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++map, ++a_pD)
		{
			if (a_pD->bA == 0) continue;

			float fNX = (int(map[-1]) - int(map[1]));//*fFactor;
			float fNY = (int(map[nRowD]) - int(map[nRowU]));//*fFactor;
			float fNZ = one*2.0f;//1.0f-sqrtf(fNX*fNX + fNY*fNY);
			//ATLASSERT(fNZ >= 0.0f);
			float fLen = 1.0f/sqrtf(fNX*fNX + fNY*fNY + fNZ*fNZ);

			int fL = (a_tLight.x*fNX + a_tLight.y*fNY + a_tLight.z*fNZ)*255.0f*fLen-fLevel;
			int iR = fL+a_pD->bR;
			a_pD->bR = iR < 0 ? 0 : (iR > 255 ? 255 : iR);
			int iG = fL+a_pD->bG;
			a_pD->bG = iG < 0 ? 0 : (iG > 255 ? 255 : iG);
			int iB = fL+a_pD->bB;
			a_pD->bB = iB < 0 ? 0 : (iB > 255 ? 255 : iB);

			//DWORD v = (*map<<8)/(one*amount);//max;
			//pixels->bR = pixels->bG = pixels->bB = v < 255 ? v : 255;;
			//pixels->bA = 0xff;
		}
		map += strideH;
		a_pD += stride;
	}
}

#include <GammaCorrection.h>

void applyMapWithSpec(DWORD* map, int strideH, TPixelChannel* a_pD, int stride, int width, int height, float angle, TVector3f a_tLight, bool a_bKeepFlat, float const a_fSpecularR, float const a_fSpecularG, float const a_fSpecularB, int a_nImgSizeX, int a_nImgSizeY, int a_nTileOffX, int a_nTileOffY, bool a_bSunken, CGammaTables const* a_pGamma)
{
	stride -= width;
	strideH -= width;

	float fLevel = a_bKeepFlat ? 255.0f/max(a_tLight.z, 0.1f) : 255.0f;
	float const fDefZ = cosf(angle*3.1415/180.0f)/cosf(45.0f*3.1415/180.0f)*one*2.0f;

	float const fSpecInt = sqrtf(a_fSpecularR*a_fSpecularR+a_fSpecularG*a_fSpecularG+a_fSpecularB*a_fSpecularB)/sqrtf(3.0f);
	bool bSpec = fSpecInt > 0.001f;
	float const fB = 175.0/9.0f;
	float const fSpecPow = fSpecInt < 0.1f ? 10.0f : (fSpecInt < 1.0f ? fB-10.0f+(60.0f-3.0f*fB)*fSpecInt+fB*2.0f*fSpecInt*fSpecInt : 50.0f);

	size_t const nRowU = a_bSunken ? width+strideH : -width-strideH;
	size_t const nRowD = a_bSunken ? -width-strideH : width+strideH;
	size_t const nColP = a_bSunken ? 1 : -1;
	size_t const nColN = a_bSunken ? -1 : 1;

	float const fAbsSize = a_nImgSizeX > a_nImgSizeY ? a_nImgSizeX : a_nImgSizeY;
	float const fRelSize = 0.4f;
	float const fF = fRelSize*2.0f/fAbsSize;
	float const fYH = -fRelSize+fF*(0.5f+0.5f*(fAbsSize-a_nImgSizeY)+a_nTileOffY);
	float const fXH = fRelSize-fF*(0.5f+0.5f*(fAbsSize-a_nImgSizeX)+a_nTileOffX);
	for (int y = 0; y < height; ++y)
	{
		float const fVY = y*fF+fYH;
		for (int x = 0; x < width; ++x, ++map, ++a_pD)
		{
			if (a_pD->bA == 0) continue;

			float fNX = (int(map[nColP]) - int(map[nColN]));
			float fNY = (int(map[nRowD]) - int(map[nRowU]));
			float fNZ = fDefZ;
			float const fLen = 1.0f/sqrtf(fNX*fNX + fNY*fNY + fNZ*fNZ);
			fNX *= fLen;
			fNY *= fLen;
			fNZ *= fLen;

			float const fLN = a_tLight.x*fNX + a_tLight.y*fNY + a_tLight.z*fNZ;
			float fL = 0.0f;
			int fD = 0.0f;
			if (bSpec)
			{
				float const fVX = fXH-x*fF;
				float const fVZ = sqrtf(1.0f-fVX*fVX-fVY*fVY);
				float fL1 = (a_tLight.x-(fNX+fNX)*fLN)*fVX + (a_tLight.y-(fNY+fNY)*fLN)*fVY + (a_tLight.z-(fNZ+fNZ)*fLN)*fVZ;
				if (fL1 >= 0)
					fL = 0;
				else
					fL = powf(-fL1, fSpecPow)*255.0f*256.0f*0.75f;
			}
			fD = (fLN*fLevel)*0.5f;
			if (fD < 0.0f) fD = 0.0f;
			if (a_pGamma)
			{
				int iR = fL*a_fSpecularR+((a_pGamma->m_aGamma[a_pD->bR]*fD)>>7);
				a_pD->bR = a_pGamma->InvGamma(iR < 0 ? 0 : (iR > 255*256 ? 255*256 : iR));
				int iG = fL*a_fSpecularG+((a_pGamma->m_aGamma[a_pD->bG]*fD)>>7);
				a_pD->bG = a_pGamma->InvGamma(iG < 0 ? 0 : (iG > 255*256 ? 255*256 : iG));
				int iB = fL*a_fSpecularB+((a_pGamma->m_aGamma[a_pD->bB]*fD)>>7);
				a_pD->bB = a_pGamma->InvGamma(iB < 0 ? 0 : (iB > 255*256 ? 255*256 : iB));
			}
			else
			{
				fL *= (1.0f/256.0f);
				int iR = fL*a_fSpecularR+((a_pD->bR*fD)>>7);
				a_pD->bR = iR < 0 ? 0 : (iR > 255 ? 255 : iR);
				int iG = fL*a_fSpecularG+((a_pD->bG*fD)>>7);
				a_pD->bG = iG < 0 ? 0 : (iG > 255 ? 255 : iG);
				int iB = fL*a_fSpecularB+((a_pD->bB*fD)>>7);
				a_pD->bB = iB < 0 ? 0 : (iB > 255 ? 255 : iB);
			}
		}
		map += strideH;
		a_pD += stride;
	}
}

void createNormalMap(DWORD* map, int strideH, TPixelChannel* a_pD, int stride, int width, int height, float angle, bool a_bSunken)
{
	stride -= width;
	strideH -= width;
	float const fDefZ = cosf(angle*3.1415/180.0f)/cosf(45.0f*3.1415/180.0f)*one*2.0f;

	size_t const nRowU = a_bSunken ? width+stride : -width-stride;
	size_t const nRowD = a_bSunken ? -width-stride : width+stride;
	size_t const nColP = a_bSunken ? 1 : -1;
	size_t const nColN = a_bSunken ? -1 : 1;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++map, ++a_pD)
		{
			float fNX = (int(map[nColP]) - int(map[nColN]));
			float fNY = (int(map[nRowD]) - int(map[nRowU]));
			float fNZ = fDefZ;
			float const fLen = 1.0f/sqrtf(fNX*fNX + fNY*fNY + fNZ*fNZ);
			fNX *= fLen;
			fNY *= fLen;
			fNZ *= fLen;
			a_pD->bR = floorf(fNX*127.0f+127.5f);
			a_pD->bG = floorf(fNY*127.0f+127.5f);
			a_pD->bB = floorf(fNZ*127.0f+127.5f);
			a_pD->bA = 255;
		}
		map += strideH;
		a_pD += stride;
	}
}

DWORD setEdgeValue(int x, int y, DWORD* p, int width, int xmax, int ymax, int def)
{
	DWORD* r1 = p - width - width - 2;
	DWORD* r2 = r1 + width;
	DWORD* r3 = r2 + width;
	DWORD* r4 = r3 + width;
	DWORD* r5 = r4 + width;

	if (y == 0 || x == 0 || y == ymax+2 || x == xmax+2)
		return *p = def;

	DWORD min = r3[2];

	DWORD v = r2[2] + one;
	if (v < min)
		min = v;
	
	v = r3[1] + one;
	if (v < min)
		min = v;
	
	v = r3[3] + one;
	if (v < min)
		min = v;
	
	v = r4[2] + one;
	if (v < min)
		min = v;
	
	v = r2[1] + sqrt2;
	if (v < min)
		min = v;
		
	v = r2[3] + sqrt2;
	if (v < min)
		min = v;
		
	v = r4[1] + sqrt2;
	if (v < min)
		min = v;
		
	v = r4[3] + sqrt2;
	if (v < min)
		min = v;
	
	if (y == 1 || x == 1 || y == ymax+1 || x == xmax+1)
		return *p = min;

	v = r1[1] + sqrt5;
	if (v < min)
		min = v;
		
	v = r1[3] + sqrt5;
	if (v < min)
		min = v;
		
	v = r2[4] + sqrt5;
	if (v < min)
		min = v;
		
	v = r4[4] + sqrt5;
	if (v < min)
		min = v;
		
	v = r5[3] + sqrt5;
	if (v < min)
		min = v;
		
	v = r5[1] + sqrt5;
	if (v < min)
		min = v;
		
	v = *r4 + sqrt5;
	if (v < min)
		min = v;
		
	v = *r2 + sqrt5;
	if (v < min)
		min = v;

	return *p = min;
}

DWORD setValue(DWORD* p, int width)
{
	DWORD* r1 = p - width - width - 2;
	DWORD* r2 = r1 + width;
	DWORD* r3 = r2 + width;
	DWORD* r4 = r3 + width;
	DWORD* r5 = r4 + width;

	DWORD min = r3[2];
	DWORD v = r2[2] + one;
	if (v < min)
		min = v;
	v = r3[1] + one;
	if (v < min)
		min = v;
	v = r3[3] + one;
	if (v < min)
		min = v;
	v = r4[2] + one;
	if (v < min)
		min = v;
	
	v = r2[1] + sqrt2;
	if (v < min)
		min = v;
	v = r2[3] + sqrt2;
	if (v < min)
		min = v;
	v = r4[1] + sqrt2;
	if (v < min)
		min = v;
	v = r4[3] + sqrt2;
	if (v < min)
		min = v;
	
	v = r1[1] + sqrt5;
	if (v < min)
		min = v;
	v = r1[3] + sqrt5;
	if (v < min)
		min = v;
	v = r2[4] + sqrt5;
	if (v < min)
		min = v;
	v = r4[4] + sqrt5;
	if (v < min)
		min = v;
	v = r5[3] + sqrt5;
	if (v < min)
		min = v;
	v = r5[1] + sqrt5;
	if (v < min)
		min = v;
	v = *r4 + sqrt5;
	if (v < min)
		min = v;
	v = *r2 + sqrt5;
	if (v < min)
		min = v;

	return *p = min;
}

DWORD distanceMap(DWORD* map, int width, int height, DWORD ceil, DWORD def = one)
{
	int xmax = width - 3;
	int ymax = height - 3;
	DWORD max = 0;
	DWORD v;

	DWORD* p = map;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++p)
		{
			if (*p > 0)
			{
				if (x < 2 || x > xmax || y < 2 || y > ymax)
					v = setEdgeValue(x, y, p, width, xmax, ymax, def);
				else
					v = setValue(p, width);
				//if (v > max)
				//	max = v;
			}
		}
	}
	--p;
	for (int y = height-1; y >= 0; --y)
	{
		for (int x = width-1; x >= 0; --x, --p)
		{
			if (*p > 0)
			{
				if (x < 2 || x > xmax || y < 2 || y > ymax)
					v = setEdgeValue(x, y, p, width, xmax, ymax, def);
				else
					v = setValue(p, width);
				if (*p > ceil)
					*p = ceil;
				//if (v > max)
				//	max = v;
			}
		}
	}
	return max;
}

void roundDistance(DWORD* map, int count, unsigned int const ceil)
{
	//float const lim = 1.0f/(float(ceil)*ceil);
	for (DWORD* const end = map+count; map < end; ++map)
	{
		if (*map)
		{
			double const d = *map;
			*map = sqrt(((ceil<<1)-d)*d);
		}
	}
}

void ridgeDistance(DWORD* map, int count, unsigned int const ceil)
{
	for (DWORD* const end = map+count; map < end; ++map)
	{
		if (*map)
		{
			double const d = *map;
			*map = sqrt((ceil-d)*d);
		}
	}
}

#include <RWViewImageRaster.h> // TODO: move color-related stuff to RWImaging

class ATL_NO_VTABLE CBevelTask :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IThreadedTask
{
public:
	void Init(LONG a_eSource, IConfig* a_pConfig, IOperationContext* a_pStates,
		CGammaTables const* a_pGT, TRasterImageRect a_tRect, IDocumentRasterImage* a_pRI,
		LONG a_nAmount, LONG a_nBlur, LONG a_nMethod, LONG a_nAngle, bool a_bSunken, TVector3f a_tRot,
		TImageSize a_tRealSize, TImageSize a_tSize2, ULONG a_nPixels, TPixelChannel* a_pSrc)
	{
		eSource = a_eSource;
		pConfig = a_pConfig;
		pStates = a_pStates;
		pGT = a_pGT;
		tRect = a_tRect;
		pRI = a_pRI;
		nAmount = a_nAmount;
		nBlur = a_nBlur;
		nMethod = a_nMethod;
		nAngle = a_nAngle;
		bSunken = a_bSunken;
		tRot = a_tRot;
		tRealSize = a_tRealSize;
		tSize2 = a_tSize2;
		nPixels = a_nPixels;
		pSrc = a_pSrc;
	}

BEGIN_COM_MAP(CBevelTask)
	COM_INTERFACE_ENTRY(IThreadedTask)
END_COM_MAP()


	static float GetSquareDiff(LONG nTX, LONG nTY, LONG nSX, LONG nSY)
	{
		float const fX = float(nTX)/nSX;
		float const fY = float(nTY)/nSY;
		return fX > fY ? fX/fY : fY/fX;
	}
	STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
	{
		try
		{
			TRasterImageRect tRectIn = tRect;
			TRasterImageRect tRectOut = tRect;
			if (a_nTotal > 1)
			{
				int nExtraSpace = nAmount+nBlur;
				LONG nTY = tRect.tBR.nY-tRect.tTL.nY;
				LONG nTX = tRect.tBR.nX-tRect.tTL.nX;
				LONG nSX = max(1, nTX/(64+nExtraSpace)); // want at least 64 pixels when splitting
				LONG nSY = max(1, nTY/(64+nExtraSpace));
				ULONG nMaxPar = nSX*nSY;
				if (a_nIndex >= nMaxPar)
					return S_FALSE;
				if (a_nTotal > nMaxPar)
					a_nTotal = nMaxPar;
				LONG nBestX;
				if (nSX > nSY)
				{
					float diff = GetSquareDiff(nTX, nTY, a_nTotal, 1);
					nBestX = a_nTotal;
					for (LONG i = 2; i <= nSY; ++i)
					{
						LONG nX = a_nTotal/i;
						if (a_nTotal != nX*i)
							continue;
						float d = GetSquareDiff(nTX, nTY, nX, i);
						if (d > diff)
							break;
						diff = d;
						nBestX = nX;
					}
				}
				else
				{
					float diff = GetSquareDiff(nTX, nTY, 1, a_nTotal);
					nBestX = 1;
					for (LONG i = 2; i <= nSX; ++i)
					{
						LONG nY = a_nTotal/i;
						if (a_nTotal != nY*i)
							continue;
						float d = GetSquareDiff(nTX, nTY, i, nY);
						if (d > diff)
							break;
						diff = d;
						nBestX = i;
					}
				}
				LONG nBestY = a_nTotal/nBestX;
				LONG nX = a_nIndex%nBestX;
				LONG nY = a_nIndex/nBestX;
				if (nX > 0)
				{
					tRectOut.tTL.nX = tRect.tTL.nX+nTX*nX/nBestX;
					tRectIn.tTL.nX = tRectOut.tTL.nX-nExtraSpace;
				}
				if (nX+1 < nBestX)
				{
					tRectOut.tBR.nX = tRect.tTL.nX+nTX*(nX+1)/nBestX;
					tRectIn.tBR.nX = tRectOut.tBR.nX+nExtraSpace;
				}
				if (nY > 0)
				{
					tRectOut.tTL.nY = tRect.tTL.nY+nTY*nY/nBestY;
					tRectIn.tTL.nY = tRectOut.tTL.nY-nExtraSpace;
				}
				if (nY+1 < nBestY)
				{
					tRectOut.tBR.nY = tRect.tTL.nY+nTY*(nY+1)/nBestY;
					tRectIn.tBR.nY = tRectOut.tBR.nY+nExtraSpace;
				}
			}

			CAutoVectorPtr<BYTE> cMask;
			if (eSource & CFGVAL_BVS_MASK)
			{
				CConfigValue cMaskID;
				pConfig->ItemValueGet(CComBSTR(CFGID_BV_SELECTIONID), &cMaskID);
				CComPtr<ISharedStateImageSelection> pSel;
				pStates->StateGet(cMaskID, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pSel));
				if (pSel)
				{
					LONG nX = tRectIn.tTL.nX;
					LONG nY = tRectIn.tTL.nY;
					ULONG nDX = tRectIn.tBR.nX-tRectIn.tTL.nX;
					ULONG nDY = tRectIn.tBR.nY-tRectIn.tTL.nY;
					if (SUCCEEDED(pSel->Bounds(&nX, &nY, &nDX, &nDY)))
					{
						if (nX <= LONG(tRectIn.tTL.nX) && nY <= LONG(tRectIn.tTL.nY) &&
							nDX >= ULONG(tRectIn.tBR.nX-tRectIn.tTL.nX) && nDY >= ULONG(tRectIn.tBR.nY-tRectIn.tTL.nY) && S_OK == pSel->IsEmpty())
						{
							// everything is selected
						}
						else
						{
							nX = tRectIn.tTL.nX;
							nY = tRectIn.tTL.nY;
							nDX = tRectIn.tBR.nX-tRectIn.tTL.nX;
							nDY = tRectIn.tBR.nY-tRectIn.tTL.nY;
							cMask.Allocate(nDX*nDY);
							pSel->GetTile(nX, nY, nDX, nDY, nDX, cMask.m_p);
						}
					}
				}
			}

			CConfigValue cKeepFlat;
			pConfig->ItemValueGet(CComBSTR(CFGID_BV_KEEPFLAT), &cKeepFlat);
			CConfigValue cColor;
			pConfig->ItemValueGet(CComBSTR(CFGID_BV_SPECCLR), &cColor);
			CConfigValue cOutput;
			pConfig->ItemValueGet(CComBSTR(CFGID_BV_OUTPUT), &cOutput);

			TImageSize tSize = {tRectIn.tBR.nX-tRectIn.tTL.nX, tRectIn.tBR.nY-tRectIn.tTL.nY};
			TImageSize tSize2 = {tSize.nX+2, tSize.nY+2};
			ULONG nPixels = tSize2.nX*tSize2.nY;
			//CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
			CAutoVectorPtr<DWORD> pHeight(new DWORD[nPixels]);
			//HRESULT hRes = pRI->TileGet(EICIRGBA, CImagePoint(tRect.tTL.nX-1, tRect.tTL.nY-1), &tSize2, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
			//if (FAILED(hRes))
			//	return hRes;

			// TODO: handle boundary conditions
			TPixelChannel* p = pSrc+(tRectIn.tTL.nX-tRect.tTL.nX)+this->tSize2.nX*(tRectIn.tTL.nY-tRect.tTL.nY);
			DWORD* pH = pHeight.m_p;
			//DWORD nMax = min(tSize2.nX, tSize2.nY);
			BYTE* pMask = cMask;
			DWORD const dwMaxHeight = one*nAmount;
			for (ULONG nY = 0; nY < tSize2.nY; ++nY)
			{
				if (cMask.m_p)
				{
					if (nY == 0 || nY+1 == tSize2.nY)
					{
						ZeroMemory(pH, tSize2.nX*sizeof*pH);
						pH += tSize2.nX;
						p += tSize2.nX;
						continue;
					}
					*(pH++) = 0;
					++p;
					for (ULONG nX = 0; nX < tSize.nX; ++nX, ++pH, ++p,  ++pMask)
					{
						DWORD const dw = DWORD(p->bA)**pMask;
						if (dw < 255)
							*pH = 0;
						else if (dw > 64516)
							*pH = dwMaxHeight;
						else
							*pH = (dw*one)>>16;
						//*pH = p->bA*one*nMax**pMask>>8;
					}
					p += this->tSize2.nX-tSize2.nX;
					*(pH++) = 0;
					++p;
					continue;
				}
				for (ULONG nX = 0; nX < tSize2.nX; ++nX, ++pH, ++p)
				{
					if (p->bA == 0)
						*pH = 0;
					else if (p->bA == 255)
						*pH = dwMaxHeight;
					else
						*pH = (p->bA*one)>>8;
					//*pH = p->bA*one*nMax;
				}
				p += this->tSize2.nX-tSize2.nX;
			}
			DWORD max = distanceMap(pHeight, tSize2.nX, tSize2.nY, one*nAmount);
			if (nMethod == CFGVAL_BVM_ROUND)
				roundDistance(pHeight, nPixels, one*nAmount);
			else if (nMethod == CFGVAL_BVM_RIDGE)
				ridgeDistance(pHeight, nPixels, one*nAmount);
			if (nBlur)
				stack_blur_gray32(pHeight.m_p, tSize2.nX, tSize2.nY, nBlur, nBlur);
			p = pSrc + (tRectOut.tTL.nX-tRect.tTL.nX)+this->tSize2.nX*(tRectOut.tTL.nY-tRect.tTL.nY) + this->tSize2.nX+1;
			size_t nHeightOff = (tRectOut.tTL.nX-tRectIn.tTL.nX)+tSize2.nX*(tRectOut.tTL.nY-tRectIn.tTL.nY);
			TImageSize tSizeOut = {tRectOut.tBR.nX-tRectOut.tTL.nX, tRectOut.tBR.nY-tRectOut.tTL.nY};
			if (cOutput.operator LONG() == CFGVAL_BVO_NORMALMAP)
				createNormalMap(pHeight.m_p+tSize2.nX+1 + nHeightOff, tSize2.nX, p, this->tSize2.nX, tSizeOut.nX, tSizeOut.nY, nMethod == CFGVAL_BVM_SLOPE ? nAngle : 45, bSunken);
			else if (nMethod == CFGVAL_BVM_OLD)
				applyMap(pHeight.m_p+tSize2.nX+1 + nHeightOff, tSize2.nX, p, this->tSize2.nX, tSizeOut.nX, tSizeOut.nY, nAmount, tRot, cKeepFlat);
			else
				applyMapWithSpec(pHeight.m_p+tSize2.nX+1 + nHeightOff, tSize2.nX, p, this->tSize2.nX, tSizeOut.nX, tSizeOut.nY, nMethod == CFGVAL_BVM_SLOPE ? nAngle : 45, tRot, true, cColor[0], cColor[1], cColor[2], tRealSize.nX, tRealSize.nY, tRectOut.tTL.nX, tRectOut.tTL.nY, bSunken, pGT);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	LONG eSource;
	IConfig* pConfig;
	IOperationContext* pStates;
	CGammaTables const* pGT;
	TRasterImageRect tRect;
	IDocumentRasterImage* pRI;
	LONG nAmount;
	LONG nBlur;
	LONG nMethod;
	LONG nAngle;
	bool bSunken;
	TVector3f tRot;
	TImageSize tRealSize;
	TImageSize tSize2;
	ULONG nPixels;
	TPixelChannel* pSrc;
};

STDMETHODIMP CRasterImageBevel::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_AMOUNT), &cVal);
		LONG nAmount = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_BLUR), &cVal);
		LONG nBlur = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_METHOD), &cVal);
		LONG nMethod = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_ANGLE), &cVal);
		LONG nAngle = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_SUNKEN), &cVal);
		bool bSunken = cVal;

		int nExtraSpace = nAmount+nBlur;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		CComPtr<IGammaTableCache> pGTC;
		RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
		CGammaTables const* const pGT = pGTC->GetSRGBTable();

		TPixelChannel tDefault;
		tDefault.n = 0;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		bool bHasAlpha = true;

		TImageSize tSize = {1, 1};
		TImagePoint tContentOrigin = {0, 0};
		TImageSize tContentSize = {0, 0};
		if (FAILED(pRI->CanvasGet(&tSize, NULL, &tContentOrigin, &tContentSize, NULL)))
			return E_FAIL;

		if (tDefault.bA == 0)
		{
			if (tContentSize.nX*tContentSize.nY == 0)
				return S_FALSE;
		}
		else
		{
			if (tContentOrigin.nX > 0) { tContentOrigin.nX = 0; tContentSize.nX += tContentOrigin.nX; }
			if (tContentOrigin.nY > 0) { tContentOrigin.nY = 0; tContentSize.nY += tContentOrigin.nY; }
			if (tContentOrigin.nX+LONG(tContentSize.nX) < LONG(tSize.nX)) tContentSize.nX = tSize.nX-tContentOrigin.nX;
			if (tContentOrigin.nY+LONG(tContentSize.nY) < LONG(tSize.nY)) tContentSize.nY = tSize.nY-tContentOrigin.nY;
		}

		TImageSize tRealSize = tSize;

		TRasterImageRect tRect = {tContentOrigin, {tContentOrigin.nX+tContentSize.nX, tContentOrigin.nY+tContentSize.nY}};
		if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
			return S_FALSE; // not a valid rectangle

		CConfigValue cSource;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_SOURCE), &cSource);
		CConfigValue cLight;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_LIGHT), &cLight);
		TQuaternionf tQuat = {cLight[0], cLight[1], cLight[2], cLight[3]};
		CQuaternionf cQuat(tQuat);
		TVector3f tVec = {0.0f, 0.0f, 1.0f};
		TVector3f tRot = TransformVector3(cQuat, tVec);

		tSize.nX = tRect.tBR.nX-tRect.tTL.nX;
		tSize.nY = tRect.tBR.nY-tRect.tTL.nY;
		TImageSize tSize2 = {tSize.nX+2, tSize.nY+2};
		ULONG nPixels = tSize2.nX*tSize2.nY;
		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, CImagePoint(tRect.tTL.nX-1, tRect.tTL.nY-1), &tSize2, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		CComPtr<IThreadPool> pThPool;
		RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

		CComObjectStackEx<CBevelTask> cTask;
		cTask.Init(cSource, a_pConfig, a_pStates, pGT, tRect, pRI,
			nAmount, nBlur, nMethod, nAngle, bSunken, tRot,
			tRealSize, tSize2, nPixels, pSrc);

		if (pThPool)
			hRes = pThPool->Execute(0, &cTask);
		else
			hRes = cTask.Execute(0, 1);
		if (FAILED(hRes))
			return hRes;

		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, CImageStride(1, tSize2.nX), tSize2.nX*tSize.nY, pSrc.m_p+tSize2.nX+1, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageBevel::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrAMOUNT(CFGID_BV_AMOUNT);
	CComBSTR bstrBLUR(CFGID_BV_BLUR);
	BSTR aIDs[] = {bstrAMOUNT, bstrBLUR};
	TConfigValue cVal[2];
	a_pConfig->ItemValueGet(bstrAMOUNT, &cVal[0]);
	a_pConfig->ItemValueGet(bstrBLUR, &cVal[1]);
	cVal[0].iVal = cVal[0].iVal ? max(1, (LONG)(f*cVal[0].iVal+0.5f)) : 0;
	cVal[1].iVal = cVal[1].iVal ? max(1, (LONG)(f*cVal[1].iVal+0.5f)) : 0;
	return a_pConfig->ItemValuesSet(2, aIDs, cVal);
}

STDMETHODIMP CRasterImageBevel::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_AMOUNT), &cVal);
		LONG nAmount = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_BLUR), &cVal);
		LONG nBlur = cVal;

		int nExtraSpace = nAmount+nBlur;

		a_pRect->tTL.nX -= nExtraSpace;
		a_pRect->tTL.nY -= nExtraSpace;
		a_pRect->tBR.nX += nExtraSpace;
		a_pRect->tBR.nY += nExtraSpace;
	}
	return S_OK;
}

