// DesignerViewFactoryImage.cpp : Implementation of CDesignerViewFactoryImage

#include "stdafx.h"
#include "DesignerViewFactoryImage.h"

#include "DesignerViewImage.h"
#include "ConfigIDsImageView.h"
#include <SharedStringTable.h>
#include <ConfigCustomGUIImpl.h>
#define COLORPICKER_NOGRADIENT
#include <WTL_ColorPicker.h>


class ATL_NO_VTABLE CConfigGUIImageDlg :
	public CCustomConfigWndImpl<CConfigGUIImageDlg>,
	public CDialogResize<CConfigGUIImageDlg>
{
public:
	CConfigGUIImageDlg() : m_wndColor(false) {}

	enum { IDD = IDD_CONFIGGUI_IMGVIEW };

	BEGIN_MSG_MAP(CConfigGUIImageDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIImageDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIImageDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGIMGVIEW_COLORBTN, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIImageDlg)
		DLGRESIZE_CONTROL(IDC_CGIMGVIEW_VIEWSYNC, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGIMGVIEW_CTRLSYNC, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIImageDlg)
		CONFIGITEM_CONTEXTHELP(IDC_CGIMGVIEW_COLORBTN, CFGID_IMGVIEW_BACKGROUNDCOLOR)
		CONFIGITEM_CHECKBOX(IDC_CGIMGVIEW_DRAWFRAME, CFGID_IMGVIEW_DRAWFRAME)
		CONFIGITEM_CHECKBOX(IDC_CGIMGVIEW_AUTOZOOM, CFGID_IMGVIEW_AUTOZOOM)
		CONFIGITEM_EDITBOX(IDC_CGIMGVIEW_VIEWSYNC, CFGID_IMGVIEW_MYVIEWPORT)
		CONFIGITEM_EDITBOX(IDC_CGIMGVIEW_CTRLSYNC, CFGID_IMGVIEW_CONTROLLEDVIEPORT)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CGIMGVIEW_COLORBTN));
		m_wndColor.SetDefaultColor(GetSysColor(COLOR_WINDOW));

		DlgResize_Init(false, false, 0);

		return 1;
	}
	void ExtraConfigNotify()
	{
		CConfigValue cValCustom;
		M_Config()->ItemValueGet(CComBSTR(CFGID_IMGVIEW_CUSTOMBACKGROUND), &cValCustom);
		CConfigValue cValColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_IMGVIEW_BACKGROUNDCOLOR), &cValColor);

		m_wndColor.SetColor(cValCustom.operator bool() ? CButtonColorPicker::SColor(cValColor.operator LONG()) : CButtonColorPicker::SColor(CButtonColorPicker::ECTDefault));
	}
	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_IMGVIEW_CUSTOMBACKGROUND(CFGID_IMGVIEW_CUSTOMBACKGROUND);
			CComBSTR cCFGID_IMGVIEW_BACKGROUNDCOLOR(CFGID_IMGVIEW_BACKGROUNDCOLOR);
			if (pClrBtn->clr.eCT == CButtonColorPicker::ECTDefault)
			{
				M_Config()->ItemValuesSet(1, &(cCFGID_IMGVIEW_CUSTOMBACKGROUND.m_str), CConfigValue(false));
			}
			else
			{
				CConfigValue cValCustom(true);
				CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToCOLORREF()));
				BSTR aIDs[2];
				aIDs[0] = cCFGID_IMGVIEW_CUSTOMBACKGROUND;
				aIDs[1] = cCFGID_IMGVIEW_BACKGROUNDCOLOR;
				TConfigValue aVals[2];
				aVals[0] = cValCustom;
				aVals[1] = cValColor;
				M_Config()->ItemValuesSet(2, aIDs, aVals);
			}
		}
		catch (...)
		{
		}
		return 0;
	}

private:
	CButtonColorPicker m_wndColor;
};


// CDesignerViewFactoryImage

STDMETHODIMP CDesignerViewFactoryImage::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryImage::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_IMGVIEW_CUSTOMBACKGROUND(CFGID_IMGVIEW_CUSTOMBACKGROUND);
		pCfgInit->ItemInsSimple(cCFGID_IMGVIEW_CUSTOMBACKGROUND, _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_CUSTOMBACKGROUND_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_CUSTOMBACKGROUND_DESC), CConfigValue(false), NULL, 0, NULL);
		CComBSTR cCFGID_IMGVIEW_BACKGROUNDCOLOR(CFGID_IMGVIEW_BACKGROUNDCOLOR);
		TConfigOptionCondition tCondition;
		tCondition.bstrID = cCFGID_IMGVIEW_CUSTOMBACKGROUND;
		tCondition.eConditionType = ECOCEqual;
		tCondition.tValue = CConfigValue(true);
		pCfgInit->ItemInsSimple(cCFGID_IMGVIEW_BACKGROUNDCOLOR, _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_BACKGROUNDCOLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_BACKGROUNDCOLOR_DESC), CConfigValue(static_cast<LONG>(0xffffff)), NULL, 1, &tCondition);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IMGVIEW_DRAWFRAME), CMultiLanguageString::GetAuto(L"[0409]Show frame[0405]Zobrazit rám"), CMultiLanguageString::GetAuto(L"[0409]Draw a dotted rectangle around the image.[0405]Nakreslit okolo obrázku tečkovaný obdélník."), CConfigValue(false), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IMGVIEW_AUTOZOOM), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_AUTOZOOM_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_AUTOZOOM_DESC), CConfigValue(false), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IMGVIEW_MYVIEWPORT), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_MYVIEWPORT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_MYVIEWPORT_DESC), CConfigValue(L""), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IMGVIEW_CONTROLLEDVIEPORT), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_CONTROLLEDVIEPORT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IMGVIEW_CONTROLLEDVIEPORT_DESC), CConfigValue(L""), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DesignerViewFactoryImage, CConfigGUIImageDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryImage::CreateWnd(IViewManager* UNREF(a_pManager), IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	CComObject<CDesignerViewImage>* pWnd = NULL;
	CComObject<CDesignerViewImage>::CreateInstance(&pWnd);
	CComPtr<IDesignerView> pOut(pWnd);

	if (FAILED(pWnd->Init(a_pFrame, a_pConfig, a_hParent, a_prcWindow, a_nStyle, a_pDoc, a_tLocaleID)))
		return E_FAIL;

	*a_ppDVWnd = pOut.Detach();

	return S_OK;
}

STDMETHODIMP CDesignerViewFactoryImage::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentImage> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentImage));
	else a_pCallback->Missing(__uuidof(IDocumentImage));
	return S_OK;
}

