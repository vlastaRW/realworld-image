
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_2DRotation.h>


class ATL_NO_VTABLE CConfigGUIRotate :
	public CCustomConfigResourcelessWndImpl<CConfigGUIRotate>,
	public CDialogResize<CConfigGUIRotate>
{
public:
	enum { IDC_CGROT_ANGLE = 212, IDC_CGROT_ANGLELABEL, IDC_CGROT_ANGLECTL, IDC_CGROT_ANGLESPIN, IDC_CGROT_CANVAS, IDC_CGROT_CANVASLABEL };

	BEGIN_DIALOG_EX(0, 0, 120, 121, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Angle:[0405]Ú&hel:"), IDC_STATIC, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGROT_ANGLE, 45, 0, 64, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGROT_ANGLESPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 97, 0, 12, 12, 0)
		CONTROL_LTEXT(_T("°"), IDC_CGROT_ANGLELABEL, 112, 2, 8, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Adjust canvas:[0405]Přizpůsobit plátno:"), IDC_CGROT_CANVASLABEL, 0, 110, 60, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGROT_CANVAS, 60, 108, 59, 42, CBS_DROPDOWNLIST | WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIRotate)
		COMMAND_HANDLER(IDC_CGROT_ANGLE, EN_CHANGE, OnEditChange)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRotate>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIRotate>)
		NOTIFY_HANDLER(IDC_CGROT_ANGLECTL, C2DRotation::C2DR_DRAG_FINISHED, OnDragFinished)
		NOTIFY_HANDLER(IDC_CGROT_ANGLECTL, C2DRotation::C2DR_ANGLE_CHANGED, OnAngleChanged)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRotate)
		DLGRESIZE_CONTROL(IDC_CGROT_ANGLE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGROT_ANGLESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGROT_ANGLELABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGROT_CANVASLABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGROT_CANVAS, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGROT_ANGLECTL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRotate)
		CONFIGITEM_EDITBOX(IDC_CGROT_ANGLE, CFGID_ANGLE)
		CONFIGITEM_CONTEXTHELP_IDS(IDC_CGROT_ANGLECTL, IDS_HELP_ANGLE)
		CONFIGITEM_COMBOBOX(IDC_CGROT_CANVAS, CFGID_EXTEND)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		RECT rcWnd = {0, 16, 120, 104};
		MapDialogRect(&rcWnd);
		m_wndRotation.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGROT_ANGLECTL);

		BOOL b;
		CCustomConfigResourcelessWndImpl<CConfigGUIRotate>::OnInitDialog(0, 0, 0, b);

		ExtraConfigNotify();

		CUpDownCtrl(GetDlgItem(IDC_CGROT_ANGLESPIN)).SetRange(0, 360);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnDragFinished(int, LPNMHDR, BOOL&)
	{
		CComBSTR cCFGID_ANGLE(CFGID_ANGLE);
		M_Config()->ItemValuesSet(1, &(cCFGID_ANGLE.m_str), CConfigValue(m_wndRotation.GetAngle()));
		return 0;
	}
	LRESULT OnAngleChanged(int, LPNMHDR, BOOL& b)
	{
		TCHAR szText[32] = _T("");
		_stprintf(szText, _T("%g"), m_wndRotation.GetAngle());
		GetDlgItem(IDC_CGROT_ANGLE).SetWindowText(szText);
		OnDragFinished(0, NULL, b);
		return 0;
	}
	LRESULT OnEditChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_wndRotation.m_hWnd == NULL)
			return 0;
		TCHAR szText[32] = _T("");
		GetDlgItem(IDC_CGROT_ANGLE).GetWindowText(szText, itemsof(szText));
		szText[itemsof(szText)-1] = _T('\0');
		LPTSTR p;
		float const f = _tcstod(szText, &p);
		if (*p == 0 || *p == _T(' '))
			m_wndRotation.SetAngle(f);
		a_bHandled = FALSE;
		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_wndRotation.m_hWnd == NULL)
			return;
		CConfigValue cVal;
		M_Config()->ItemValueGet(CComBSTR(CFGID_ANGLE), &cVal);
		m_wndRotation.SetAngle(cVal);
	}

private:
	C2DRotation m_wndRotation;
};

