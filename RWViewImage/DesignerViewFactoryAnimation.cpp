// DesignerViewFactoryAnimation.cpp : Implementation of CDesignerViewFactoryAnimation

#include "stdafx.h"
#include "DesignerViewFactoryAnimation.h"

#include "DesignerViewAnimation.h"
#include <MultiLanguageString.h>
#include <ConfigCustomGUIImpl.h>
#define COLORPICKER_NOGRADIENT
#include <WTL_ColorPicker.h>


class ATL_NO_VTABLE CConfigGUIAnimationDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIAnimationDlg>
{
public:
	CConfigGUIAnimationDlg() : m_wndColor(false) {}

	enum { IDC_BACKGROUNDCOLOR = 100, IDC_DRAWFRAME };

	BEGIN_DIALOG_EX(0, 0, 105, 26, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Background color:[0405]Barva pozadí:"), IDC_STATIC, 0, 2, 68, 8, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_BACKGROUNDCOLOR, 70, 0, 35, 12, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Show frame[0405]Zobrazit rám"), IDC_DRAWFRAME, 0, 16, 1050, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIAnimationDlg)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIAnimationDlg>)
		NOTIFY_HANDLER(IDC_BACKGROUNDCOLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIAnimationDlg)
		CONFIGITEM_CONTEXTHELP(IDC_BACKGROUNDCOLOR, CFGID_ANIVIEW_BACKGROUNDCOLOR)
		CONFIGITEM_CHECKBOX(IDC_DRAWFRAME, CFGID_ANIVIEW_DRAWFRAME)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_BACKGROUNDCOLOR));
		m_wndColor.SetDefaultColor(GetSysColor(COLOR_WINDOW));
	}
	void ExtraConfigNotify()
	{
		CConfigValue cValColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_ANIVIEW_BACKGROUNDCOLOR), &cValColor);

		m_wndColor.SetColor(cValColor.operator LONG()&0xff000000 ? CButtonColorPicker::SColor(CButtonColorPicker::ECTDefault) : CButtonColorPicker::SColor(cValColor.operator LONG()));
	}
	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_ANIVIEW_BACKGROUNDCOLOR(CFGID_ANIVIEW_BACKGROUNDCOLOR);
			M_Config()->ItemValuesSet(1, &(cCFGID_ANIVIEW_BACKGROUNDCOLOR.m_str), CConfigValue(LONG((pClrBtn->clr.eCT == CButtonColorPicker::ECTDefault ? 0xff000000 : (pClrBtn->clr.ToCOLORREF())))));
		}
		catch (...)
		{
		}
		return 0;
	}

private:
	CButtonColorPicker m_wndColor;
};


// CDesignerViewFactoryAnimation

STDMETHODIMP CDesignerViewFactoryAnimation::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]Animation - Viewer[0405]Animace - náhled");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryAnimation::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_ANIVIEW_BACKGROUNDCOLOR), CMultiLanguageString::GetAuto(L"[0409]Background color[0405]Barva pozadí"), CMultiLanguageString::GetAuto(L"[0409]RGB value for window background.[0405]RGB hodnota pro výplň pozadí."), CConfigValue(static_cast<LONG>(0xff000000)), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_ANIVIEW_DRAWFRAME), CMultiLanguageString::GetAuto(L"[0409]Show frame[0405]Zobrazit rám"), CMultiLanguageString::GetAuto(L"[0409]Draw a dotted rectangle around the image.[0405]Nakreslit okolo obrázku tečkovaný obdélník."), CConfigValue(false), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewFactoryAnimation, CConfigGUIAnimationDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryAnimation::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	CComObject<CDesignerViewAnimation>* pWnd = NULL;
	CComObject<CDesignerViewAnimation>::CreateInstance(&pWnd);
	CComPtr<IDesignerView> pOut(pWnd);

	if (FAILED(pWnd->Init(a_pFrame, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID)))
		return E_FAIL;

	*a_ppDVWnd = pOut.Detach();

	return S_OK;
}

STDMETHODIMP CDesignerViewFactoryAnimation::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentImage> pA;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
	if (pA) a_pCallback->Used(__uuidof(IDocumentAnimation));
	else a_pCallback->Missing(__uuidof(IDocumentAnimation));
	return S_OK;
}

