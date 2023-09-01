
#pragma once

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEditToolPropertiesDlg :
	public CCustomConfigWndImpl<CConfigGUIEditToolPropertiesDlg>,
	public CDialogResize<CConfigGUIEditToolPropertiesDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_EDITTOOLPROPERTIES };

	BEGIN_MSG_MAP(CConfigGUIEditToolPropertiesDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEditToolPropertiesDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIEditToolPropertiesDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEditToolPropertiesDlg)
		DLGRESIZE_CONTROL(IDC_CGETP_SYNCGRP, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEditToolPropertiesDlg)
		CONFIGITEM_EDITBOX(IDC_CGETP_SYNCGRP, CFGID_TOOLPROPS_TOOLSTATESYNC)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};
