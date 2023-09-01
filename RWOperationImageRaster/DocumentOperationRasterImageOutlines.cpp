// DocumentOperationRasterImageOutlines.cpp : Implementation of CDocumentOperationRasterImageOutlines

#include "stdafx.h"
#include "DocumentOperationRasterImageOutlines.h"

#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <math.h>
#include <IconRenderer.h>


// CDocumentOperationRasterImageOutlines

STDMETHODIMP CDocumentOperationRasterImageOutlines::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEOUTLINES_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_OUT_WIDTH[] = L"Width";
static OLECHAR const CFGID_OUT_POSITON[] = L"Position";
static LONG const CFGVAL_OP_INNER = 0;
static LONG const CFGVAL_OP_CENTER = 1;
static LONG const CFGVAL_OP_OUTER = 2;
const OLECHAR CFGID_OUT_COLOR[] = L"Color";
const OLECHAR CFGID_OUT_BLENDINGMODE[] = L"BlendingMode";
static LONG const CFGVAL_OC_OUTLINEONLY = 0;
static LONG const CFGVAL_OC_REPLACE = 1;
static LONG const CFGVAL_OC_BLEND = 2;

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageOutlines::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Outline[0405]Obrys";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cSize;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_WIDTH), &cSize);
		CComPtr<ILocalizedString> pMethod;
		pMethod.Attach(new CMultiLanguageString(pszName));
		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_COLOR), &cColor);
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
			pParams.Attach(new CSimpleLocalizedString(szTmp));
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

HICON CDocumentOperationRasterImageOutlines::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IIconRenderer> pIR;
	RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRPathPoint const path0[] =
	{
		{56, 183, 5, 0, 10, 21},
		{76, 183, -5, -6, -4, 2},
		{54, 148, 7, 7, 2, 10},
		{83, 167, -6, -16, -11, -5},
		{84, 102, 3, 3, -13, 23},
		{99, 111, 10, -34, -3, -2},
		{154, 29, 1, 7, -23, 17},
		{156, 49, 9, -12, 2, -6},
		{234, 2, 0, 19, -29, 3},
		{182, 92, 4, -1, 12, -18},
		{197, 90, -1, 15, -4, 2},
		{156, 140, 5, -1, 11, -9},
		{176, 140, -7, 20, -4, 0},
		{130, 172, 4, 2, 10, -6},
		{140, 185, -15, 5, 1, -5},
		{103, 194, 3, 3, 11, 0},
		{120, 208, -28, 4, -7, -3},
	};
	IRPathPoint const path1[] =
	{
		{167, 73, -66, 93, -84, 85},
		{62, 244, -6, -6, 12, -2},
	};
	IRPathPoint const path2[] =
	{
		{88, 246, 46, 3, -5.98728, -0.390475},
		{169, 221, -5.2256, -21.9475, 5, 21},
		{219, 189, -22, 3, -24, 16},
		{157, 219, 6, 19, -6.86685, -21.745},
		{89, 236, -5.98578, 0.412813, 29, -2},
	};
	IRPath const shape0 = {itemsof(path0), path0};
	IRPath const shape1 = {itemsof(path1), path1};
	IRPath const shape2 = {itemsof(path2), path2};
	IRCanvas const canvas0 = {2, 2, 248, 248, 0, 0, NULL, NULL};
	//IRFillWithInternalOutline feather(/*0xff000000|GetSysColor(COLOR_WINDOW)*/0xffefc6c6, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 240/20.0f);
	//IRFill quill(0xff000000|GetSysColor(COLOR_WINDOWTEXT));//0xffefc6c6);
	//IRFill stroke(0xff000000|GetSysColor(COLOR_WINDOWTEXT));
	IRLayer const layers[] =
	{
		{&canvas0, 0, 1, NULL, &shape0, pSI->GetMaterial(ESMScheme1Color1)},
		{&canvas0, 0, 1, NULL, &shape1, pSI->GetMaterial(ESMContrast)},
		{&canvas0, 0, 1, NULL, &shape2, pSI->GetMaterial(ESMContrast)},
	};
	return pIR->CreateIcon(a_nSize, itemsof(layers), layers);
}

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>


class ATL_NO_VTABLE CConfigGUIOutline :
	public CCustomConfigResourcelessWndImpl<CConfigGUIOutline>,
	public CDialogResize<CConfigGUIOutline>,
	public CObserverImpl<CConfigGUIOutline, IColorPickerObserver, ULONG>,
	public IResizableConfigWindow
{
public:
	CConfigGUIOutline()
	{
	}

	enum
	{
		IDC_CGO_COLOR = 100,
		IDC_CGO_BLENDING,
		IDC_CGO_POSITION,
		IDC_CGO_WIDTH_EDIT,
		IDC_CGO_WIDTH_SLIDER,
	};

	BEGIN_DIALOG_EX(0, 0, 120, 148, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Blending:[0405]Míchání:"), IDC_STATIC, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGO_BLENDING, 44, 0, 75, 30, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Position:[0405]Pozice:"), IDC_STATIC, 0, 18, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGO_POSITION, 44, 16, 75, 30, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Width:[0405]Šířka:"), IDC_STATIC, 0, 34, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGO_WIDTH_EDIT, 44, 32, 29, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGO_WIDTH_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 73, 32, 47, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Color:[0405]Barva:"), IDC_STATIC, 0, 2, 44, 8, WS_VISIBLE, 0)
		//CONTROL_PUSHBUTTON(_T("[0409]Color[0405]Color"), IDC_CGO_COLOR, 44, 0, 35, 12, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIOutline)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIOutline>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIOutline>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnWidthScroll)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIOutline)
		DLGRESIZE_CONTROL(IDC_CGO_BLENDING, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGO_POSITION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGO_WIDTH_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGO_COLOR, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIOutline)
		CONFIGITEM_COMBOBOX(IDC_CGO_POSITION, CFGID_OUT_POSITON)
		CONFIGITEM_EDITBOX(IDC_CGO_WIDTH_EDIT, CFGID_OUT_WIDTH)
		//CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGO_WIDTH_SLIDER, CFGID_OUT_WIDTH)
		CONFIGITEM_COMBOBOX(IDC_CGO_BLENDING, CFGID_OUT_BLENDINGMODE)
		CONFIGITEM_CONTEXTHELP(IDC_CGO_COLOR, CFGID_OUT_COLOR)
	END_CONFIGITEM_MAP()

	BEGIN_COM_MAP(CConfigGUIOutline)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || m_pColor == NULL || a_pSize == NULL)
			return E_UNEXPECTED;
		RECT rc = {0, 0, 120, 48};
		MapDialogRect(&rc);
		SIZE sz = {0, 0};
		m_pColor->OptimumSize(&sz);
		a_pSize->cx = rc.right;
		a_pSize->cy = rc.bottom+sz.cy;
		return S_OK;
	}

	void ExtraInitDialog()
	{
		m_wndWidth = GetDlgItem(IDC_CGO_WIDTH_SLIDER);
		m_wndWidth.SetRange(0, 100);
		m_wndWidth.SetPageSize(10);

		RECT rcColor = {0, 48, 120, 48};
		MapDialogRect(&rcColor);
		RWCoCreateInstance(m_pColor, __uuidof(ColorPicker));
		m_pColor->Create(m_hWnd, &rcColor, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Color of the outline.[0405]Barva ohraničení."), FALSE, CComBSTR(L"OUTLINES"), NULL);
		RWHWND hColor;
		m_pColor->Handle(&hColor);
		::SetWindowLong(hColor, GWLP_ID, IDC_CGO_COLOR);
		SIZE tSize = {rcColor.right-rcColor.left, 0};
		m_pColor->OptimumSize(&tSize);
		rcColor.bottom = rcColor.top+tSize.cy;
		m_pColor->Move(&rcColor);
		m_pColor->ObserverIns(CObserverImpl<CConfigGUIOutline, IColorPickerObserver, ULONG>::ObserverGet(), 0);
	}

	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		//if (!m_bEnableUpdates)
		//	return;

		if (a_nFlags&ECPCColor)
		{
			TColor clr;
			m_pColor->ColorGet(&clr);
			CConfigValue cVal(clr.fR, clr.fG, clr.fB, clr.fA);
			CComBSTR bstr(CFGID_OUT_COLOR);
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
			m_pColor->ObserverDel(CObserverImpl<CConfigGUIOutline, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnWidthScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndWidth == reinterpret_cast<HWND>(a_lParam))
		{
			LONG nPos = m_wndWidth.GetPos();
			float fPos = expf(nPos*0.01f*logf(50.0f))*0.5f;
			if (fPos > 300)
				fPos = int(fPos+0.5f);
			else if (fPos > 3)
				fPos = int(fPos*10.0f+0.5f)*0.1f;
			else
				fPos = int(fPos*100.0f+0.5f)*0.01f;
			CComBSTR bstrID(CFGID_OUT_WIDTH);
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(fPos));
			return 0;
		}
		a_bHandled = FALSE;
		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_hWnd == NULL)
			return;

		if (m_pColor)
		{
			CConfigValue cColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_OUT_COLOR), &cColor);
			TColor clr = {cColor[0], cColor[1], cColor[2], cColor[3]};
			m_pColor->ColorSet(&clr);
		}

		if (m_wndWidth.m_hWnd)
		{
			CConfigValue cWidth;
			M_Config()->ItemValueGet(CComBSTR(CFGID_OUT_WIDTH), &cWidth);
			float f = cWidth.operator float();
			if (f < 0.5) f = 0.5f;
			else if (f > 25) f = 25.0f;
			LONG nPos = logf(f*2.0f)*100.0f/logf(50.0f)+0.5f;
			if (m_wndWidth.GetPos() != nPos)
				m_wndWidth.SetPos(nPos);
		}
	}

private:
	CComPtr<IColorPicker> m_pColor;
	CTrackBarCtrl m_wndWidth;
};

STDMETHODIMP CDocumentOperationRasterImageOutlines::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_OUT_WIDTH), CMultiLanguageString::GetAuto(L"[0409]Width[0405]Šířka"), CMultiLanguageString::GetAuto(L"[0409]Width of the outline.[0405]Šířka hran."), CConfigValue(2.0f), NULL, 0, NULL);
		CComBSTR cCFGID_OUT_POSITON(CFGID_OUT_POSITON);
		pCfgInit->ItemIns1ofN(cCFGID_OUT_POSITON, CMultiLanguageString::GetAuto(L"[0409]Position[0405]Pozice"), CMultiLanguageString::GetAuto(L"[0409]Defines, where to place the outline.[0405]Určuje, kde bude obrys umístěn."), CConfigValue(CFGVAL_OP_CENTER), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_POSITON, CConfigValue(CFGVAL_OP_INNER), CMultiLanguageString::GetAuto(L"[0409]Inside[0405]Uvnitř"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_POSITON, CConfigValue(CFGVAL_OP_CENTER), CMultiLanguageString::GetAuto(L"[0409]Centered[0405]Na středu"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_POSITON, CConfigValue(CFGVAL_OP_OUTER), CMultiLanguageString::GetAuto(L"[0409]Outside[0405]Venku"), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_OUT_COLOR), CMultiLanguageString::GetAuto(L"[0409]Color[0405]Barva"), CMultiLanguageString::GetAuto(L"[0409]Color of the outline.[0405]Barva ohraničení."), CConfigValue(0.0f, 0.0f, 0.0f, 1.0f), NULL, 0, NULL);
		CComBSTR cCFGID_OUT_BLENDINGMODE(CFGID_OUT_BLENDINGMODE);
		pCfgInit->ItemIns1ofN(cCFGID_OUT_BLENDINGMODE, CMultiLanguageString::GetAuto(L"[0409]Composition mode[0405]Způsob kompozice"), CMultiLanguageString::GetAuto(L"[0409]This setting controls how to combine the outline with the original shape.[0405]Toto nastavení určuje, jak zkombinovat ohraničení s původním tvarem."), CConfigValue(CFGVAL_OC_REPLACE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_BLENDINGMODE, CConfigValue(CFGVAL_OC_OUTLINEONLY), CMultiLanguageString::GetAuto(L"[0409]Outline only[0405]Pouze ohraničení"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_BLENDINGMODE, CConfigValue(CFGVAL_OC_REPLACE), CMultiLanguageString::GetAuto(L"[0409]Replace outline[0405]Nahradit ohraničení"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_OUT_BLENDINGMODE, CConfigValue(CFGVAL_OC_BLEND), CMultiLanguageString::GetAuto(L"[0409]Blend transparent outline[0405]Smíchat průhledné ohraničení"), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageOutlines, CConfigGUIOutline>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageOutlines::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

struct SOutlineOnly
{
	SOutlineOnly(TPixelChannel t) : tCol(t) {}
	TPixelChannel const tCol;
	void Blend(TPixelChannel* pD, ULONG a_nCover, ULONG a_nMaxBg) const
	{
		BYTE const bA = (tCol.bA*a_nCover*4112UL+0x10000)>>20;
		if (bA)
		{
			*pD = tCol;
			pD->bA = bA;
		}
		else
		{
			pD->n = 0;
		}
	}
};

struct SReplaceOutline
{
	SReplaceOutline(TPixelChannel t, CGammaTables const* p) :
		tCol(t), pGT(p), nR(p->m_aGamma[t.bR]), nG(p->m_aGamma[t.bG]), nB(p->m_aGamma[t.bB])
	{
	}
	TPixelChannel const tCol;
	CGammaTables const* const pGT;
	LONG const nR;
	LONG const nG;
	LONG const nB;
	void Blend(TPixelChannel* pD, ULONG a_nCover, ULONG a_nSubBg) const
	{
		ULONG nOA = (tCol.bA*a_nCover*4112UL+0x80000)>>20;
		if (a_nCover == 255 || pD->bA <= a_nSubBg)
		{
			if (nOA == 0)
			{
				pD->n = 0;
			}
			else
			{
				*pD = tCol;
				pD->bA = nOA;
			}
		}
		else if (a_nCover)
		{
			a_nSubBg = pD->bA-a_nSubBg;
			if (a_nCover+a_nSubBg > 255)
				a_nSubBg = 255-a_nCover;
			//a_nSubBg = (a_nSubBg*(255-a_nCover)*4112UL+0x80000)>>20;
			ULONG nA = nOA+a_nSubBg;
			if (nA)
			{
				ULONG bR = (nR*nOA+pGT->m_aGamma[pD->bR]*a_nSubBg)/nA;
				ULONG bG = (nG*nOA+pGT->m_aGamma[pD->bG]*a_nSubBg)/nA;
				ULONG bB = (nB*nOA+pGT->m_aGamma[pD->bB]*a_nSubBg)/nA;
				pD->bR = pGT->InvGamma(bR);
				pD->bG = pGT->InvGamma(bG);
				pD->bB = pGT->InvGamma(bB);
				pD->bA = nA;
			}
			else
			{
				pD->bR = pD->bG = pD->bB = pD->bA = 0;
			}
		}
	}
};

struct SBlendOutline
{
	SBlendOutline(TPixelChannel t, CGammaTables const* p) :
		tCol(t), pGT(p), nR(p->m_aGamma[t.bR]), nG(p->m_aGamma[t.bG]), nB(p->m_aGamma[t.bB])
	{
	}
	TPixelChannel const tCol;
	CGammaTables const* const pGT;
	LONG const nR;
	LONG const nG;
	LONG const nB;
	void Blend(TPixelChannel* pD, ULONG a_nCover, ULONG a_nMaxBg) const
	{
		ULONG nOA = (tCol.bA*a_nCover*4112UL+0x80000)>>20;
		if (nOA == 255 || pD->bA == 0)
		{
			if (nOA == 0)
			{
				pD->n = 0;
			}
			else
			{
				*pD = tCol;
				pD->bA = nOA;
			}
		}
		else if (nOA)
		{
			ULONG nA = (nOA<<8)+(((255UL-nOA)*pD->bA*4112UL+0x800)>>12);
			ULONG bR = (nR*nOA*255UL+pGT->m_aGamma[pD->bR]*ULONG(pD->bA)*(255UL-nOA))/nA;
			ULONG bG = (nG*nOA*255UL+pGT->m_aGamma[pD->bG]*ULONG(pD->bA)*(255UL-nOA))/nA;
			ULONG bB = (nB*nOA*255UL+pGT->m_aGamma[pD->bB]*ULONG(pD->bA)*(255UL-nOA))/nA;
			pD->bR = pGT->InvGamma(bR);
			pD->bG = pGT->InvGamma(bG);
			pD->bB = pGT->InvGamma(bB);
			pD->bA = (nA+0x80)>>8;
		}
	}
};

const DWORD one = 256;
DWORD distanceMap(DWORD* map, int width, int height, DWORD ceil, DWORD def = one);

template<typename TMixer>
void drawOutline(DWORD* map1, DWORD* map2, TPixelChannel* a_pD, int width, int height, float amount, TMixer const& a_tMixer)
{
	map1 += width+3;
	map2 += width+3;
	LONG const nLim1 = amount+0.5f;
	LONG const nLim2 = nLim1-one;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++map1, ++map2, ++a_pD)
		{
			LONG n;
			LONG b = 0;
			if (*map1 < DWORD(nLim1) && *map2 < DWORD(nLim1))
			{
				n = 255;
				if (*map1 > DWORD(nLim2))
					b = (n = (nLim1-*map1)*255/one);
				if (*map2 > DWORD(nLim2))
				{
					n = (nLim1-*map2)*n/one;
					//b = a_pD->bA;
				}
			}
			else
				n = 0;
			a_tMixer.Blend(a_pD, n, b);
		}
		map1 += 2;
		map2 += 2;
	}
}

template<bool t_bInside, typename TMixer>
void drawOutline(DWORD* map, TPixelChannel* a_pD, int width, int height, float amount, TMixer const& a_tMixer)
{
	map += width+3;
	LONG const nLim1 = amount+0.5f;
	LONG const nLim2 = nLim1-one;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x, ++map, ++a_pD)
		{
			LONG n;
			LONG b;
			if (*map < DWORD(nLim1))
			{
				if (t_bInside)
				{
					if (*map > DWORD(nLim2))
						n = (nLim1-*map)*a_pD->bA/one;
					else
						n = a_pD->bA;
					b = n;
				}
				else
				{
					if (*map > DWORD(nLim2))
						n = (nLim1-*map)*(255-a_pD->bA)/one;
					else
						n = 255-a_pD->bA;
					b = 0;
				}
			}
			else
			{
				n = 0;
				b = 0;
			}
			a_tMixer.Blend(a_pD, n, b);
		}
		map += 2;
	}
}

STDMETHODIMP CDocumentOperationRasterImageOutlines::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_WIDTH), &cVal);
		float fWidth = cVal;
		if (fWidth <= 0.1f) fWidth = 0.1f;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_POSITON), &cVal);
		LONG nPos = cVal;
		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_COLOR), &cColor);
		TPixelChannel const tCol =
		{
			CGammaTables::ToSRGB(cColor[2]),
			CGammaTables::ToSRGB(cColor[1]),
			CGammaTables::ToSRGB(cColor[0]),
			cColor[3] < 0.0f ? 0 : (cColor[3] > 1.0f ? 255 : cColor[3]*255.0f+0.5f),
		};
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_BLENDINGMODE), &cVal);
		LONG eBlending = cVal;

		int nExtraSpace = ceilf(fWidth)+1;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		CAutoPtr<CGammaTables> pGT(NULL);
			pGT.Attach(new CGammaTables(/*fGamma*/));

		bool bHasAlpha = true;

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		if (FAILED(pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL)))
			return E_FAIL;

		TRasterImageRect tRect = {{tOrigin.nX-nExtraSpace, tOrigin.nY-nExtraSpace}, {tOrigin.nX+tSize.nX+nExtraSpace+nExtraSpace, tOrigin.nY+tSize.nY+nExtraSpace+nExtraSpace}};
		tSize.nX += nExtraSpace+nExtraSpace;
		tSize.nY += nExtraSpace+nExtraSpace;
		//if (tRect.tBR.nX > tSize.nX) tRect.tBR.nX = tSize.nX;
		//if (tRect.tBR.nY > tSize.nY) tRect.tBR.nY = tSize.nY;
		if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
			return S_FALSE; // not a valid rectangle

		//CConfigValue cSource;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_BV_SOURCE), &cSource);

		ULONG nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
		CAutoVectorPtr<DWORD> pHeight(new DWORD[(tSize.nX+2)*(tSize.nY+2)]);
		CAutoVectorPtr<DWORD> pDepth(new DWORD[(tSize.nX+2)*(tSize.nY+2)]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		LONG nAmount = ceilf(fWidth)+3; // some margin
		DWORD const dwMaxHeight = one*nAmount;

		ZeroMemory(pHeight.m_p, (tSize.nX+2)*sizeof*pHeight.m_p);
		ZeroMemory(pHeight.m_p+(tSize.nX+2)*(tSize.nY+1), (tSize.nX+2)*sizeof*pHeight.m_p);
		std::fill_n(pDepth.m_p, tSize.nX+2, dwMaxHeight);
		std::fill_n(pDepth.m_p+(tSize.nX+2)*(tSize.nY+1), tSize.nX+2, dwMaxHeight);
		TPixelChannel* p = pSrc.m_p;
		DWORD* pH = pHeight.m_p+tSize.nX+2;
		DWORD* pD = pDepth.m_p+tSize.nX+2;
		DWORD nMax = min(tSize.nX, tSize.nY);
		for (ULONG nY = 0; nY < tSize.nY; ++nY)
		{
			*(pH++) = 0;
			*(pD++) = dwMaxHeight;
			for (ULONG nX = 0; nX < tSize.nX; ++nX, ++pH, ++pD, ++p)
			{
				if (p->bA == 0)
				{
					*pH = 0;
					*pD = dwMaxHeight;
				}
				else if (p->bA == 255)
				{
					*pH = dwMaxHeight;
					*pD = 0;
				}
				else
				{
					*pH = (p->bA*one)>>8;
					*pD = ((255-p->bA)*one)>>8;
				}
				//*pH = p->bA*one*nMax;
			}
			*(pH++) = 0;
			*(pD++) = dwMaxHeight;
		}
		distanceMap(pHeight, tSize.nX+2, tSize.nY+2, one*nAmount);
		distanceMap(pDepth, tSize.nX+2, tSize.nY+2, one*nAmount, one*nAmount);
		if (eBlending == CFGVAL_OC_OUTLINEONLY)
		{
			if (nPos == CFGVAL_OP_INNER)
				drawOutline<true>(pHeight, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SOutlineOnly(tCol));
			else if (nPos == CFGVAL_OP_OUTER)
				drawOutline<false>(pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SOutlineOnly(tCol));
			else
				drawOutline(pHeight, pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth*0.5f+1.0f), SOutlineOnly(tCol));
		}
		else if (eBlending == CFGVAL_OC_REPLACE || tCol.bA == 255)
		{
			if (nPos == CFGVAL_OP_INNER)
				drawOutline<true>(pHeight, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SReplaceOutline(tCol, pGT));
			else if (nPos == CFGVAL_OP_OUTER)
				drawOutline<false>(pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SReplaceOutline(tCol, pGT));
			else
				drawOutline(pHeight, pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth*0.5f+1.0f), SReplaceOutline(tCol, pGT));
		}
		else
		{
			if (nPos == CFGVAL_OP_INNER)
				drawOutline<true>(pHeight, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SBlendOutline(tCol, pGT));
			else if (nPos == CFGVAL_OP_OUTER)
				drawOutline<false>(pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth+1.0f), SBlendOutline(tCol, pGT));
			else
				drawOutline(pHeight, pDepth, pSrc.m_p, tSize.nX, tSize.nY, one*(fWidth*0.5f+1.0f), SBlendOutline(tCol, pGT));
		}
		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageOutlines::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrWIDTH(CFGID_OUT_WIDTH);
	CConfigValue cVal;
	a_pConfig->ItemValueGet(bstrWIDTH, &cVal);
	if (cVal.TypeGet() != ECVTFloat)
		return E_FAIL;
	cVal = cVal.operator float()*f;
	return a_pConfig->ItemValuesSet(1, &(bstrWIDTH.m_str), cVal);
}

STDMETHODIMP CDocumentOperationRasterImageOutlines::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_OUT_WIDTH), &cVal);
		float fWidth = cVal.operator float();
		if (fWidth <= 0.1f) fWidth = 0.1f;

		int nExtraSpace = ceilf(fWidth)+1;

		a_pRect->tTL.nX -= nExtraSpace;
		a_pRect->tTL.nY -= nExtraSpace;
		a_pRect->tBR.nX += nExtraSpace;
		a_pRect->tBR.nY += nExtraSpace;
	}
	return S_OK;
}

