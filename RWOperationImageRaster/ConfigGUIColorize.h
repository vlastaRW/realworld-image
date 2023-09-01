
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorArea.h>

class ATL_NO_VTABLE CConfigGUIColorize :
	public CCustomConfigResourcelessWndImpl<CConfigGUIColorize>,
	public CDialogResize<CConfigGUIColorize>
{
public:
	enum { IDC_CGGS_MODE = 100, IDC_CGGS_HUECTRL };

	BEGIN_DIALOG_EX(0, 0, 120, 76, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_COMBOBOX(IDC_CGGS_MODE, 0, 0, 120, 54, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGGS_HUECTRL, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 16, 120, 60, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIColorize)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIColorize>)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIColorize>)
		NOTIFY_HANDLER(IDC_CGGS_HUECTRL, CColorAreaHue::CAN_COLOR_CHANGED, OnColorChanged)
		MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIColorize)
		CONFIGITEM_COMBOBOX(IDC_CGGS_MODE, CFGID_CLRZ_MODE)
		CONFIGITEM_CONTEXTHELP(IDC_CGGS_HUECTRL, CFGID_CLRZ_HUE)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIColorize)
		DLGRESIZE_CONTROL(IDC_CGGS_MODE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGGS_HUECTRL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CWindow wnd(GetDlgItem(IDC_CGGS_HUECTRL));
		RECT rcWnd;
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wnd.DestroyWindow();
		m_wndAreaHue.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGGS_HUECTRL);

		BOOL b;
		CCustomConfigResourcelessWndImpl<CConfigGUIColorize>::OnInitDialog(0, 0, 0, b);

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_lParam)
			*reinterpret_cast<float*>(a_lParam) = 1.0f;
		return 0;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR cCFGID_CLRZ_HUE(CFGID_CLRZ_HUE);
			CComBSTR cCFGID_CLRZ_SATURATION(CFGID_CLRZ_SATURATION);
			float h, l, s, a;
			m_wndAreaHue.GetHLSA(&h, &l, &s, &a);
			CConfigValue cHue(h);
			CConfigValue cSaturation(s);
			BSTR aIDs[2];
			aIDs[0] = cCFGID_CLRZ_HUE;
			aIDs[1] = cCFGID_CLRZ_SATURATION;
			TConfigValue aVals[2];
			aVals[0] = cHue;
			aVals[1] = cSaturation;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_wndAreaHue.m_hWnd)
		{
			CConfigValue cHue;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CLRZ_HUE), &cHue);
			CConfigValue cSaturation;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CLRZ_SATURATION), &cSaturation);
			m_wndAreaHue.SetHLSA(cHue, 0.5f, cSaturation, 1.0f);

		}
	}

private:
	CColorAreaHueSat m_wndAreaHue;
};
