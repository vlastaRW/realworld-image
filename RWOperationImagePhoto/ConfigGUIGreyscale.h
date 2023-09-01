
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorArea.h>

class ATL_NO_VTABLE CConfigGUIGreyscale :
	public CCustomConfigResourcelessWndImpl<CConfigGUIGreyscale>,
	public CDialogResize<CConfigGUIGreyscale>
{
public:
	enum { IDC_CGGS_HUECTRL = 100 };

	BEGIN_DIALOG_EX(0, 0, 120, 60, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_CGGS_HUECTRL, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 0, 120, 60, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIGreyscale)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIGreyscale>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIGreyscale>)
		NOTIFY_HANDLER(IDC_CGGS_HUECTRL, CColorAreaHue::CAN_COLOR_CHANGED, OnColorChanged)
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 1.0f; return TRUE; }
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIGreyscale)
		CONFIGITEM_CONTEXTHELP(IDC_CGGS_HUECTRL, CFGID_COLOR)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIGreyscale)
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
		CCustomConfigResourcelessWndImpl<CConfigGUIGreyscale>::OnInitDialog(0, 0, 0, b);

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR cCFGID_COLOR(CFGID_COLOR);
			CConfigValue cValColor(static_cast<LONG>(m_wndAreaHue.GetRGB()));
			BSTR aIDs[1];
			aIDs[0] = cCFGID_COLOR;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
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
			CConfigValue cValColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_COLOR), &cValColor);
			if (m_wndAreaHue.GetRGB() != cValColor.operator LONG())
				m_wndAreaHue.SetRGBA(cValColor.operator LONG(), 255);

		}
	}

private:
	CColorAreaHue m_wndAreaHue;
};
