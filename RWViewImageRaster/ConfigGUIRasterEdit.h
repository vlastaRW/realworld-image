
#pragma once

#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUIRasterEditDlg :
	public CCustomConfigWndImpl<CConfigGUIRasterEditDlg>,
	public CDialogResize<CConfigGUIRasterEditDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_RASTEREDIT };

	BEGIN_MSG_MAP(CConfigGUIRasterEditDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRasterEditDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIRasterEditDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CGRE_GESTURES, BN_CLICKED, OnGesturesClicked)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRasterEditDlg)
		DLGRESIZE_CONTROL(IDC_CGRE_SYNCGRP, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_SELECTIONIDLABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_SELECTIONID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_GRID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGRE_VIEWID, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_PASTE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_SELECTIONMASK, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGRE_CONTEXTMENU, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGRE_SUBCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGRE_GESTURES, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRasterEditDlg)
		CONFIGITEM_EDITBOX(IDC_CGRE_SYNCGRP, CFGID_2DEDIT_TOOLSYNC)
		CONFIGITEM_EDITBOX(IDC_CGRE_PASTE, CFGID_2DEDIT_PASTETOOL)
		CONFIGITEM_EDITBOX(IDC_CGRE_SELECTIONID, CFGID_2DEDIT_SELECTIONSYNC)
		CONFIGITEM_EDITBOX(IDC_CGRE_VIEWID, CFGID_2DEDIT_VIEWPORTSYNC)
		CONFIGITEM_COMBOBOX(IDC_CGRE_GRID, CFGID_IMGEDIT_GRID)
		CONFIGITEM_CHECKBOX(IDC_CGRE_AUTOZOOM, CFGID_IMGEDIT_STARTWITHAUTOZOOM)
		CONFIGITEM_CHECKBOX(IDC_CGRE_SELECTIONMASK, CFGID_2DEDIT_SELECTIONSUPPORT)
		CONFIGITEM_COMBOBOX(IDC_CGRE_CONTEXTMENU, CFGID_2DEDIT_CONTEXTMENU)
		CONFIGITEM_SUBCONFIG(IDC_CGRE_SUBCONFIG, CFGID_2DEDIT_CONTEXTMENU)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnGesturesClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		CComPtr<IMouseGesturesHelper> pMGH;
		RWCoCreateInstance(pMGH, __uuidof(MouseGesturesHelper));
		if (pMGH)
			pMGH->Configure(m_hWnd, m_tLocaleID, M_Config());
		return 0;
	}
};
