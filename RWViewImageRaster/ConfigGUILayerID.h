
#pragma once

extern __declspec(selectany) OLECHAR const CFGID_SELECTIONSYNC[] = L"SelectionSyncGroup";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE DECLSPEC_UUID("16A449BB-AF7D-4411-B6DA-E6094115C038") CConfigGUILayerIDDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILayerIDDlg>,
	public CDialogResize<CConfigGUILayerIDDlg>
{
public:
	static HRESULT CreateConfig(IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection sync ID[0405]Synchronizace výběru"), CMultiLanguageString::GetAuto(L"[0409]Image selection is synchronized by the given ID.[0405]Vybraná oblast v obrázku je dopstupná a sychronizována přes zadané ID."), CConfigValue(L"LAYER"), NULL, 0, NULL);
			CConfigCustomGUI<&__uuidof(CConfigGUILayerIDDlg), CConfigGUILayerIDDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	enum { IDC_CG_LAYERID = 100 };

	BEGIN_DIALOG_EX(0, 0, 145, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizační ID &výběru:"), IDC_STATIC, 0, 2, 85, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_LAYERID, 85, 0, 59, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILayerIDDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayerIDDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILayerIDDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayerIDDlg)
		DLGRESIZE_CONTROL(IDC_CG_LAYERID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayerIDDlg)
		CONFIGITEM_EDITBOX(IDC_CG_LAYERID, CFGID_SELECTIONSYNC)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};

