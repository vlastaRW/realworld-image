
#pragma once

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISyncIDDlg :
	public CCustomConfigWndImpl<CConfigGUISyncIDDlg>,
	public CDialogResize<CConfigGUISyncIDDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_SYNCID };

	BEGIN_MSG_MAP(CConfigGUISyncIDDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISyncIDDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUISyncIDDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISyncIDDlg)
		DLGRESIZE_CONTROL(IDC_CGID_SELECTION, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISyncIDDlg)
		CONFIGITEM_EDITBOX(IDC_CGID_SELECTION, CFGID_SELSYNCGROUP)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

__declspec(selectany) extern GUID const tCGSyncID = {0xd137cf6f, 0x2d9, 0x4810, {0x85, 0x0, 0x0, 0x44, 0x37, 0xdf, 0xca, 0xdd}};
