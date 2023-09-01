
#pragma once

#include <ConfigCustomGUIImpl.h>

__declspec(selectany) extern OLECHAR CFGID_SELSYNCGROUP[] = L"SyncID";

class ATL_NO_VTABLE CConfigGUISyncIDDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUISyncIDDlg>,
	public CDialogResize<CConfigGUISyncIDDlg>
{
public:
	BEGIN_DIALOG_EX(0, 0, 160, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_CGID_SELECTION = 100,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Selection synchronization ID:[0405]Synchronizační ID &výběru:"), IDC_STATIC, 0, 2, 94, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGID_SELECTION, 97, 0, 62, 12, WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, 0);
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUISyncIDDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISyncIDDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUISyncIDDlg>)
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

__declspec(selectany) extern GUID const tCGSyncID = {0xc0011334, 0x872b, 0x4959, {0xbc, 0x86, 0xfa, 0xe5, 0xd5, 0x8c, 0x6b, 0x62}};
