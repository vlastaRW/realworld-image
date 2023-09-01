// DocumentOperationRasterImageShiftHue.cpp : Implementation of CDocumentOperationRasterImageShiftHue

#include "stdafx.h"
#include "DocumentOperationRasterImageShiftHue.h"
#include <SharedStringTable.h>
#include <ConfigCustomGUIImpl.h>


const OLECHAR CFGID_SH_ANGLE[] = L"Angle";

// CDocumentOperationRasterImageShiftHue

STDMETHODIMP CDocumentOperationRasterImageShiftHue::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGESHIFTHUE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageShiftHue::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Shift hue[0405]Posunout odstín";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cAngle;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SH_ANGLE), &cAngle);
		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(pszName));
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %i°")));
		pStr->Init(pTempl, pName, int(cAngle.operator float() < 0.0f ? cAngle.operator float()+360.5f : cAngle.operator float()+0.5f));
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

//HICON CDocumentOperationRasterImageShiftHue::GetDefaultIcon(ULONG a_nSize)
//{
//	IRPolyPoint const sky[] = { {0, 32}, {256, 32}, {256, 208}, {0, 208} };
//	IRPolyPoint const hills[] = { {64, 128}, {128, 176}, {192, 96}, {256, 176}, {256, 224}, {0, 224}, {0, 176} };
//	IRPathPoint const sun[] =
//	{
//		{128, 80, 0, -17.6731, 0, 17.6731},
//		{96, 48, -17.6731, 0, 17.6731, 0},
//		{64, 80, 0, 17.6731, 0, -17.6731},
//		{96, 112, 17.6731, 0, -17.6731, 0},
//	};
//	CComPtr<IStockIcons> pSI;
//	RWCoCreateInstance(pSI, __uuidof(StockIcons));
//	IRFill fillSky(0xffe766eb);
//	IRFill fillHills(0xff66abeb);
//	IRFill fillSun(0xff66ebad);
//	IROutlinedFill matSky(&fillSky, pSI->GetMaterial(ESMContrast));
//	IROutlinedFill matHills(&fillHills, matSky.outline);
//	IROutlinedFill matSun(&fillSun, matSky.outline);
//	static IRGridItem const tGridX[] = {{EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
//	static IRGridItem const tGridY[] = {{EGIFInteger, 32.0f}, {EGIFInteger, 224.0f}};
//	IRCanvas canvas = {0, 0, 256, 256, itemsof(tGridX), itemsof(tGridY), tGridX, tGridY};
//	CIconRendererReceiver cRenderer(a_nSize);
//	cRenderer(&canvas, itemsof(sky), sky, &matSky);
//	cRenderer(&canvas, itemsof(hills), hills, &matHills);
//	cRenderer(&canvas, itemsof(sun), sun, &matSun);
//	return cRenderer.get();
//}

class ATL_NO_VTABLE CConfigGUIShiftHue :
	public CCustomConfigResourcelessWndImpl<CConfigGUIShiftHue>,
	public CDialogResize<CConfigGUIShiftHue>
{
public:
	CConfigGUIShiftHue()
	{
	}

	enum { IDC_ANGLE_EDIT = 100, IDC_ANGLE_SLIDER, IDC_ANGLE_SPIN, IDC_ANGLE_UNIT };

	BEGIN_DIALOG_EX(0, 0, 120, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Angle:[0405]Úhel:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ANGLE_EDIT, 78, 0, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_ANGLE_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 0, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_ANGLE_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 0, 33, 12, 0)
		CONTROL_RTEXT(_T("°"), IDC_ANGLE_UNIT, 112, 2, 8, 8, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIShiftHue)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIShiftHue>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIShiftHue>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIShiftHue)
		DLGRESIZE_CONTROL(IDC_ANGLE_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE_EDIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE_UNIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIShiftHue)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_ANGLE_SLIDER, CFGID_SH_ANGLE)
		CONFIGITEM_EDITBOX(IDC_ANGLE_EDIT, CFGID_SH_ANGLE)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		CUpDownCtrl wndUD = GetDlgItem(IDC_ANGLE_SPIN);
		wndUD.SetRange(-180, 180);

		return 1;
	}
	LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_lParam)
			*reinterpret_cast<float*>(a_lParam) = 1.0f;
		return 0;
	}

	//bool GetSliderRange(wchar_t const* UNREF(a_pszName), TConfigValue* a_pFrom, TConfigValue* a_pTo, TConfigValue* a_pStep)
	//{
	//	a_pFrom->eTypeID = a_pTo->eTypeID = a_pStep->eTypeID = ECVTFloat;
	//	a_pFrom->fVal = -180;
	//	a_pTo->fVal = 180;
	//	a_pStep->fVal = 1.0f;
	//	return true;
	//}
};

STDMETHODIMP CDocumentOperationRasterImageShiftHue::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_SH_ANGLE), _SharedStringTable.GetStringAuto(IDS_CFGID_SH_ANGLE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SH_ANGLE_DESC), CConfigValue(0.0f), NULL, CConfigValue(-180.0f), CConfigValue(180.0f), CConfigValue(1.0f), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageShiftHue, CConfigGUIShiftHue>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageShiftHue::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
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

unsigned IntHLS(unsigned a_nMin, unsigned a_nMax, unsigned a_nAngle)
{
	unsigned a = (a_nAngle+0x2aaa)&0xffff;
	if (a >= 0x8000) a = 0xffff-a;
	if (a < 0x2aaa) a = 0;
	else if (a >= 0x5555) a = 0x2aaa;
	else a -= 0x2aaa;
	return (a_nMin + (a_nMax-a_nMin)*a/0x2aaa)>>8;
}

#include <GammaCorrection.h>
#include "PixelLevelTask.h"

struct CShiftHueOp
{
	//CGammaTables const* pGT;
	LONG nAngle;
	LONGLONG* pTotalA;
	static TPixelChannel ProcessPixel(TPixelChannel tS, LONG nAngle)
	{
		TPixelChannel t;
		int rgbmax = tS.bR>tS.bG ? tS.bR : tS.bG;
        int rgbmin = rgbmax^tS.bR^tS.bG;
        rgbmax = rgbmax > tS.bB ? rgbmax : tS.bB;
		rgbmin = rgbmin < tS.bB ? rgbmin : tS.bB;
		unsigned int const nL = (rgbmax + rgbmin)<<7;
		unsigned int nH = 0;
		unsigned int nS = 0;
		unsigned int const rgbdelta = rgbmax - rgbmin;
		if (rgbdelta)
		{
			if (nL <= 0x7fff)
				nS = ((rgbmax - rgbmin)*(0xffff)) / (rgbmax + rgbmin);
			else
				nS = ((rgbmax - rgbmin)*(0xffff)) / (510 - rgbmax - rgbmin);

			if (tS.bR == rgbmax)
				nH = 0xffff&((tS.bG - tS.bB)*0xffff / int(rgbdelta*6));
			else if (tS.bG == rgbmax)
				nH = 0xffff&(((rgbdelta + rgbdelta + tS.bB - tS.bR)*0xffff) / int(rgbdelta*6));
			else
				nH = 0xffff&((((rgbdelta<<2) + tS.bR - tS.bG)*0xffff) / int(rgbdelta*6));

			// new hue
			unsigned int nH2 = (nH+nAngle)&0xffff;

			// back to rgb
			unsigned int m2 = nL + (nL <= 0x7fff ? (nL*nS)/0xffff : nS - (nL*nS)/0xffff);
			unsigned int m1 = (nL<<1) - m2;
			t.n = IntHLS(m1, m2, nH2-0x5555)|
				(IntHLS(m1, m2, nH2)<<8)|
				(IntHLS(m1, m2, nH2+0x5555)<<16)|
				(tS.n&0xff000000);
		}
		else
		{
			ULONG const l = nL>>8;
			t.n = (l*0x10101)|(tS.n&0xff000000);
		}
		return t;
	}
	void Process(TPixelChannel* pD, TPixelChannel const* pS, size_t const s, size_t const n)
	{
		LONG nAngle = this->nAngle;
		ULONGLONG totalA = 0;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			TPixelChannel tS = *pS;
			DWORD nA = tS.n&0xff000000;
			totalA += nA;

			*pD = ProcessPixel(tS, nAngle);
		}
		totalA>>=24;
		InterlockedAdd64(pTotalA, totalA);
	}
};

STDMETHODIMP CDocumentOperationRasterImageShiftHue::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_NOINTERFACE;

		TImageSize tSize = {1, 1};
		TImagePoint tOrig = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
		TRasterImageRect tR = {tOrig, {tOrig.nX+tSize.nX, tOrig.nY+tSize.nY}};
		TPixelChannel  tDefault;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SH_ANGLE), &cVal);

		CShiftHueOp cOp;
		ULONGLONG totalA = 0;
		cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
		cOp.nAngle = cVal.operator float()/360.0f*0xffff;
		tDefault = CShiftHueOp::ProcessPixel(tDefault, cOp.nAngle);

		CAutoPixelBuffer cBuf(pRI, tSize);

		{
			CComPtr<IThreadPool> pThPool;
			if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
				RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

			CComObjectStackEx<CPixelLevelTask<CShiftHueOp> > cXform;
			cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		return cBuf.Replace(tR.tTL, &tR.tTL, &tSize, &totalA, tDefault);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

struct SShiftHueRenderer : IStockIcons::ILayerReceiver
{
	SShiftHueRenderer(IStockIcons::ILayerReceiver* base) : base(*base) {}
	bool operator()(IRLayer const& layer, IRTarget const* target = NULL)
	{
		IRLayer l = layer;

		if (l.material->type == EIRMFill)
		{
			IRFill f(convert(reinterpret_cast<IRFill const*>(layer.material)->color));
			l.material = &f;
			return base(l, target);
		}
		else if (l.material->type == EIRMFillWithInternalOutline)
		{
			IROutlinedFill const* s = reinterpret_cast<IROutlinedFill const*>(layer.material);
			if (s->inside->type == EIRMFill)
			{
				IRFill f(convert(reinterpret_cast<IRFill const*>(s->inside)->color));
				IROutlinedFill of(&f, s->outline, s->widthRelative, s->widthAbsolute);
				l.material = &of;
				return base(l, target);
			}
		}
		return base(layer, target);
	}
	DWORD convert(DWORD color)
	{
		//CShiftHueOp cOp;
		//ULONGLONG totalA = 0;
		//cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
		//cOp.nAngle = cVal.operator float()/360.0f*0xffff;
		//tDefault = CShiftHueOp::ProcessPixel(tDefault, cOp.nAngle);

		int r = GetRValue(color);
		int g = GetGValue(color);
		int b = GetBValue(color);
		return (0xff000000&color)|RGB(g, b, r);
	}
	IStockIcons::ILayerReceiver& base;
};

HICON CDocumentOperationRasterImageShiftHue::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	SShiftHueRenderer cGSRenderer(&cRenderer);
	pSI->GetLayers(ESIPicture, cGSRenderer);
	return cRenderer.get();
}

