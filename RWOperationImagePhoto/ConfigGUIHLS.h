
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>
#include <WTL_Hue.h>

class ATL_NO_VTABLE CConfigGUIHLS :
	public CCustomConfigResourcelessWndImpl<CConfigGUIHLS>,
	public CDialogResize<CConfigGUIHLS>
{
public:
	enum
	{
		IDC_CGHLS_HUE_TEXT = 100, IDC_CGHLS_HUE_EDIT, IDC_CGHLS_HUE_SPIN, IDC_CGHLS_HUE_SLIDER,
		IDC_CGHLS_LIGHTNESS_TEXT, IDC_CGHLS_LIGHTNESS_EDIT, IDC_CGHLS_LIGHTNESS_SPIN, IDC_CGHLS_LIGHTNESS_SLIDER,
		IDC_CGHLS_SATURATION_TEXT, IDC_CGHLS_SATURATION_EDIT, IDC_CGHLS_SATURATION_SPIN, IDC_CGHLS_SATURATION_SLIDER,
		IDC_CGHLS_AREA, IDC_CGHLS_PICK, IDC_CGHLS_PICKADD, IDC_CGHLS_PICKREMOVE,
		IDC_CGHLS_HUE_TO, IDC_CGHLS_HUE_FROMTO, IDC_CGHLS_HUE_FROM, IDC_CGHLS_HUEBAR
	};

	BEGIN_DIALOG_EX(0, 0, 120, 141, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Hue:[0405]Odstín:"), IDC_CGHLS_HUE_TEXT, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGHLS_HUE_EDIT, 44, 0, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_HUE_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 0, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_HUE_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 14, 120, 12, 0)
		CONTROL_LTEXT(_T("[0409]Lightness:[0405]Světelnost:"), IDC_CGHLS_LIGHTNESS_TEXT, 0, 32, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGHLS_LIGHTNESS_EDIT, 44, 30, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_LIGHTNESS_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 30, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_LIGHTNESS_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 44, 120, 12, 0)
		CONTROL_LTEXT(_T("[0409]Saturation:[0405]Sytost:"), IDC_CGHLS_SATURATION_TEXT, 0, 62, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGHLS_SATURATION_EDIT, 44, 60, 75, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_SATURATION_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 60, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_SATURATION_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 74, 120, 12, 0)
		CONTROL_CHECKBOX(_T("[0409]Apply to selected area only...[0405]Aplikovat na vybranou oblast..."), IDC_CGHLS_AREA, 0, 90, 120, 10, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_CGHLS_PICK, 0, 104, 14, 12, BS_ICON | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_CGHLS_PICKADD, 15, 104, 14, 12, BS_ICON | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_CGHLS_PICKREMOVE, 30, 104, 14, 12, BS_ICON | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T(""), IDC_CGHLS_HUE_TO, 90, 106, 30, 8, WS_VISIBLE, 0)
		CONTROL_CTEXT(_T("-"), IDC_CGHLS_HUE_FROMTO, 78, 106, 8, 8, WS_VISIBLE, 0)
		CONTROL_RTEXT(_T(""), IDC_CGHLS_HUE_FROM, 45, 106, 30, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGHLS_HUEBAR, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 120, 120, 21, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIHLS)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIHLS>)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIHLS>)
		COMMAND_HANDLER(IDC_CGHLS_PICK, BN_CLICKED, OnPickColor)
		COMMAND_HANDLER(IDC_CGHLS_PICKADD, BN_CLICKED, OnPickColorAdd)
		COMMAND_HANDLER(IDC_CGHLS_PICKREMOVE, BN_CLICKED, OnPickColorRemove)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGHLS_HUEBAR, CHueBarControl::CHB_AREA_MOVED, OnAreaMoved)
		NOTIFY_HANDLER(IDC_CGHLS_HUEBAR, CHueBarControl::CHB_DRAG_FINISHED, OnAreaFinished)
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 1.0f; return TRUE; }
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIHLS)
		CONFIGITEM_EDITBOX(IDC_CGHLS_HUE_EDIT, CFGID_HLS_HUE)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGHLS_HUE_SLIDER, CFGID_HLS_HUE)
		CONFIGITEM_EDITBOX(IDC_CGHLS_LIGHTNESS_EDIT, CFGID_HLS_LIGHTNESS)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGHLS_LIGHTNESS_SLIDER, CFGID_HLS_LIGHTNESS)
		CONFIGITEM_EDITBOX(IDC_CGHLS_SATURATION_EDIT, CFGID_HLS_SATURATION)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGHLS_SATURATION_SLIDER, CFGID_HLS_SATURATION)
		CONFIGITEM_CHECKBOX(IDC_CGHLS_AREA, CFGID_HLS_AREA)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIHLS)
		DLGRESIZE_CONTROL(IDC_CGHLS_HUE_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_LIGHTNESS_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_SATURATION_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_HUE_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_LIGHTNESS_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_SATURATION_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_HUE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_LIGHTNESS_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_SATURATION_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGHLS_HUEBAR, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// Sliders
		CTrackBarCtrl wnd = GetDlgItem(IDC_CGHLS_HUE_SLIDER);
		wnd.SetTicFreq(256);
		wnd.SetPageSize(256);
		wnd = GetDlgItem(IDC_CGHLS_LIGHTNESS_SLIDER);
		wnd.SetTicFreq(201);
		//wnd.SetPageSize(20);
		wnd = GetDlgItem(IDC_CGHLS_SATURATION_SLIDER);
		wnd.SetTicFreq(201);
		//wnd.SetPageSize(20);

		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_CGHLS_HUE_SPIN)).SetRange(-180, 180);
		CUpDownCtrl(GetDlgItem(IDC_CGHLS_LIGHTNESS_SPIN)).SetRange(-100, 100);
		CUpDownCtrl(GetDlgItem(IDC_CGHLS_SATURATION_SPIN)).SetRange(-100, 100);

		// Pick Buttons
		CButton(GetDlgItem(IDC_CGHLS_PICK)).SetIcon((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PICK_COLOR), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		CButton(GetDlgItem(IDC_CGHLS_PICKADD)).SetIcon((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PICK_COLOR_ADD), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
		CButton(GetDlgItem(IDC_CGHLS_PICKREMOVE)).SetIcon((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PICK_COLOR_REMOVE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));

		// Hue
		RECT rcWnd;
		CWindow wndX(GetDlgItem(IDC_CGHLS_HUEBAR));
		wndX.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wndX.DestroyWindow();
		m_wndHue.Create(m_hWnd, &rcWnd, 0, 0, 0, IDC_CGHLS_HUEBAR);
		CConfigValue cValArea1, cValArea2, cValArea3, cValArea4;
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_AREA1), &cValArea1);
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_AREA2), &cValArea2);
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_AREA3), &cValArea3);
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_AREA4), &cValArea4);
		m_wndHue.SetArea(cValArea1.operator LONG(), cValArea2.operator LONG(), cValArea3.operator LONG(), cValArea4.operator LONG());

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);
		return 1;
	}

	LRESULT OnPickColor(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseHLSA cColor;
		cColor.SetRGB(tCR);
		int nHue = cColor.GetH();
		m_wndHue.SetArea(nHue-20, nHue-10, nHue+10, nHue+20);
		BOOL t;
		OnAreaMoved(0,0,t);
		OnAreaFinished(0,0,t);
		return 0;
	}
	LRESULT OnPickColorAdd(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseHLSA cColor;
		cColor.SetRGB(tCR);
		int nHue = cColor.GetH();
		int aArea[4];
		m_wndHue.GetArea(aArea[0], aArea[1], aArea[2], aArea[3]);
		if(IsBetween(1, nHue, aArea[1], aArea[2]))
			return 0;
		int d1 = HueDistance(nHue, aArea[1]);
		int d2 = HueDistance(nHue, aArea[2]);
		if(d1 < d2)
		{
			int d = HueDistance(aArea[0], aArea[1]);
			LinearizeArea(aArea);
			aArea[1] -= d1;
			aArea[0] = aArea[1] - d;
			if(aArea[3] - aArea[0] > 359)
			{
				aArea[1] += aArea[3] - aArea[0] - 359;
				aArea[0] += aArea[3] - aArea[0] - 359;
			}
		}else
		{
			int d = HueDistance(aArea[2], aArea[3]);
			LinearizeArea(aArea);
			aArea[2] += d2;
			aArea[3] = aArea[2] + d;
			if(aArea[3] - aArea[0] > 359)
			{
				aArea[2] -= aArea[3] - aArea[0] - 359;
				aArea[3] -= aArea[3] - aArea[0] - 359;
			}
		}
		m_wndHue.SetArea(aArea[0], aArea[1], aArea[2], aArea[3]);
		BOOL t;
		OnAreaMoved(0,0,t);
		OnAreaFinished(0,0,t);
		return 0;
	}
	LRESULT OnPickColorRemove(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		COLORREF tCR;
		if(!CPixelColorPicker::PickColor(&tCR))
			return 0;
		CColorBaseHLSA cColor;
		cColor.SetRGB(tCR);
		int nHue = cColor.GetH();
		int aArea[4];
		m_wndHue.GetArea(aArea[0], aArea[1], aArea[2], aArea[3]);
		if(!IsBetween(1, nHue, aArea[0], aArea[3]))
			return 0;
		int d0 = HueDistance(nHue, aArea[0]);
		int d3 = HueDistance(nHue, aArea[3]);
		if(d0 < d3)
		{
			int d = HueDistance(aArea[0], aArea[1]);
			LinearizeArea(aArea);
			aArea[0] += d0;
			aArea[1] = aArea[0] + d;
			if(aArea[2] <= aArea[1])
				aArea[1] = aArea[2]-1;
			if(aArea[1] <= aArea[0])
				aArea[1] = aArea[0]+1;
			if(aArea[2] <= aArea[1])
				aArea[2] = aArea[1]+1;
		}else
		{
			int d = HueDistance(aArea[2], aArea[3]);
			LinearizeArea(aArea);
			aArea[3] -= d3;
			aArea[2] = aArea[3] - d;
			if(aArea[2] <= aArea[1])
				aArea[2] = aArea[1]+1;
			if(aArea[3] <= aArea[2])
				aArea[2] = aArea[3]-1;
			if(aArea[2] <= aArea[1])
				aArea[1] = aArea[2]-1;
		}
		m_wndHue.SetArea(aArea[0], aArea[1], aArea[2], aArea[3]);
		BOOL t;
		OnAreaMoved(0,0,t);
		OnAreaFinished(0,0,t);
		return 0;
	}

	void ExtraConfigNotify()
	{
		CConfigValue cValArea;
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_AREA), &cValArea);
		if(GetDlgItem(IDC_CGHLS_PICK).IsWindow())
		{
			GetDlgItem(IDC_CGHLS_PICK).EnableWindow(cValArea.operator bool());
			GetDlgItem(IDC_CGHLS_PICKADD).EnableWindow(cValArea.operator bool());
			GetDlgItem(IDC_CGHLS_PICKREMOVE).EnableWindow(cValArea.operator bool());
		}
		if(GetDlgItem(IDC_CGHLS_HUE_FROM).IsWindow())
		{
			GetDlgItem(IDC_CGHLS_HUE_FROM).ShowWindow(cValArea.operator bool() ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_CGHLS_HUE_FROMTO).ShowWindow(cValArea.operator bool() ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_CGHLS_HUE_TO).ShowWindow(cValArea.operator bool() ? SW_SHOW : SW_HIDE);
		}
		CConfigValue cValHue, cValLight, cValSat;
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_HUE), &cValHue);
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_LIGHTNESS), &cValLight);
		M_Config()->ItemValueGet(CComBSTR(CFGID_HLS_SATURATION), &cValSat);
		m_wndHue.SetHLS(cValArea.operator bool(), cValHue.operator LONG(), cValLight.operator LONG(), cValSat.operator LONG());
	}

	LRESULT OnAreaMoved(int, LPNMHDR, BOOL&)
	{
		int nArea1, nArea2, nArea3, nArea4;
		m_wndHue.GetArea(nArea1, nArea2, nArea3, nArea4);
		ATL::CWindow wndFrom(GetDlgItem(IDC_CGHLS_HUE_FROM));
		TCHAR psz[14];
		_stprintf(psz, _T("%d/%d"), nArea1, nArea2);
		wndFrom.SetWindowText(psz);
		ATL::CWindow wndTo(GetDlgItem(IDC_CGHLS_HUE_TO));
		_stprintf(psz, _T("%d/%d"), nArea3, nArea4);
		wndTo.SetWindowText(psz);
		return 0;
	}

	LRESULT OnAreaFinished(int, LPNMHDR, BOOL&)
	{
		int nArea1, nArea2, nArea3, nArea4;
		m_wndHue.GetArea(nArea1, nArea2, nArea3, nArea4);
		CComBSTR cCFGIDArea1(CFGID_HLS_AREA1);
		CComBSTR cCFGIDArea2(CFGID_HLS_AREA2);
		CComBSTR cCFGIDArea3(CFGID_HLS_AREA3);
		CComBSTR cCFGIDArea4(CFGID_HLS_AREA4);
		CConfigValue cValArea1(static_cast<LONG>(nArea1));
		CConfigValue cValArea2(static_cast<LONG>(nArea2));
		CConfigValue cValArea3(static_cast<LONG>(nArea3));
		CConfigValue cValArea4(static_cast<LONG>(nArea4));
		BSTR aIDs[4];
		aIDs[0] = cCFGIDArea1;
		aIDs[1] = cCFGIDArea2;
		aIDs[2] = cCFGIDArea3;
		aIDs[3] = cCFGIDArea4;
		TConfigValue aVals[4];
		aVals[0] = cValArea1;
		aVals[1] = cValArea2;
		aVals[2] = cValArea3;
		aVals[3] = cValArea4;
		M_Config()->ItemValuesSet(4, aIDs, aVals);
		return 0;
	}

private:
	CHueBarControl m_wndHue;
};
