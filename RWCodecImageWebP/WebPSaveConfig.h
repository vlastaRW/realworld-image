
#pragma once

#include <MultiLanguageString.h>
#include <Win32LangEx.h>

static OLECHAR const CFGID_WEBPLOSSLESS[] = L"WebPLossless";
static OLECHAR const CFGID_WEBPQUALITY[] = L"WebPQuality";
static OLECHAR const CFGID_WEBPOPTIMIZATION[] = L"WebPOptimization";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderWebPDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderWebPDlg>,
	public CDialogResize<CConfigGUIEncoderWebPDlg>
{
public:
	enum { IDC_CGWEBP_QUALITYEDIT = 100, IDC_CGWEBP_QUALITYSLIDER, IDC_CGWEBP_QUALITYLABEL, IDC_CGWEBP_METHOD, IDC_CGWEBP_LOSSLESS };

	BEGIN_DIALOG_EX(0, 0, 150, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CHECKBOX(_T("[0409]Lossless[0405]Bezeztrátově"), IDC_CGWEBP_LOSSLESS, 0, 1, 50, 10, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_LTEXT(_T("[0409]Quality:[0405]Kvalita:"), IDC_CGWEBP_QUALITYLABEL, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGWEBP_QUALITYEDIT, 57, 0, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGWEBP_QUALITYSLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 90, 0, 10, 12, 0)
		CONTROL_COMBOBOX(IDC_CGWEBP_METHOD, 117, 0, 33, 12, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderWebPDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderWebPDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderWebPDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderWebPDlg)
		//DLGRESIZE_CONTROL(IDC_CGWEBP_QUALITYLABEL, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGWEBP_QUALITYEDIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGWEBP_QUALITYSLIDER, DLSZ_MULDIVSIZE_X(2, 3))
		DLGRESIZE_CONTROL(IDC_CGWEBP_METHOD, DLSZ_MULDIVMOVE_X(2, 3)|DLSZ_DIVSIZE_X(3))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderWebPDlg)
		CONFIGITEM_CHECKBOX(IDC_CGWEBP_LOSSLESS, CFGID_WEBPLOSSLESS)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGWEBP_QUALITYSLIDER, CFGID_WEBPQUALITY)
		CONFIGITEM_EDITBOX(IDC_CGWEBP_QUALITYEDIT, CFGID_WEBPQUALITY)
		CONFIGITEM_COMBOBOX(IDC_CGWEBP_METHOD, CFGID_WEBPOPTIMIZATION)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

inline HRESULT InitWebPConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_WEBPLOSSLESS(CFGID_WEBPLOSSLESS);
		pCfgInit->ItemInsSimple(cCFGID_WEBPLOSSLESS, CMultiLanguageString::GetAuto(L"[0409]Lossless[0405]Bezeztrátově"), CMultiLanguageString::GetAuto(L"[0409]Lossless compression preserves image exactly. Not recommended for photos.[0405]Bezaztrátová komprese zachová obrázek přesně. Není doporučeno pro fotografie."), CConfigValue(true), NULL, 0, NULL);
		TConfigOptionCondition tOption;
		tOption.bstrID = cCFGID_WEBPLOSSLESS;
		tOption.eConditionType = ECOCEqual;
		tOption.tValue.eTypeID = ECVTBool;
		tOption.tValue.bVal = false;
		CComBSTR cCFGID_WEBPQUALITY(CFGID_WEBPQUALITY);
		pCfgInit->ItemInsRanged(cCFGID_WEBPQUALITY, CMultiLanguageString::GetAuto(L"[0409]Compression[0405]Komprese"), CMultiLanguageString::GetAuto(L"[0409]Higher value means higher image quality, but also larger file size.[0405]Vyšší hodnota znamená větší kvalitu obrázku, ale také větší velikost souboru."), CConfigValue(75.0f), NULL, CConfigValue(0.0f), CConfigValue(100.0f), CConfigValue(1.0f), 1, &tOption);
		CComBSTR cCFGID_WEBPOPTIMIZATION(CFGID_WEBPOPTIMIZATION);
		pCfgInit->ItemIns1ofN(cCFGID_WEBPOPTIMIZATION, CMultiLanguageString::GetAuto(L"[0409]Method[0405]Metoda"), CMultiLanguageString::GetAuto(L"[0409]How long to search for the most effective way of data compression.[0405]Jak dlouho hledat nejefektivnější způsob redukce dat."), CConfigValue(4L), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WEBPOPTIMIZATION, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]Fast compression[0405]Rychlá komprese"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WEBPOPTIMIZATION, CConfigValue(4L), CMultiLanguageString::GetAuto(L"[0409]Default compression[0405]Doporučená komprese"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WEBPOPTIMIZATION, CConfigValue(6L), CMultiLanguageString::GetAuto(L"[0409]Maximum compression[0405]Nejvyšší komprese"), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentEncoderWebP, CConfigGUIEncoderWebPDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefCfg = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

