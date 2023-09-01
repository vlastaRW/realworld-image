
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUISoftenSkin :
	public CCustomConfigWndImpl<CConfigGUISoftenSkin>,
	public CDialogResize<CConfigGUISoftenSkin>
{
public:
	CConfigGUISoftenSkin() : m_wndColor(false)
	{
	}
	enum { IDD = IDD_CONFIGGUI_SOFTENSKIN };

	BEGIN_MSG_MAP(CConfigGUISoftenSkin)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUISoftenSkin>)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISoftenSkin>)
		NOTIFY_HANDLER(IDC_CGSS_COLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISoftenSkin)
		CONFIGITEM_EDITBOX(IDC_CGSS_STRENGTH_EDIT, CFGID_SS_STRENGTH)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGSS_STRENGTH_SLIDER, CFGID_SS_STRENGTH)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISoftenSkin)
		DLGRESIZE_CONTROL(IDC_CGSS_STRENGTH_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGSS_STRENGTH_EDIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGSS_STRENGTH_SPIN, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	void ExtraInitDialog()
	{
		// initialize color button
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CGSS_COLOR));
		m_wndColor.SetDefaultText(NULL);
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// Slider
		CTrackBarCtrl wnd = GetDlgItem(IDC_CGSS_STRENGTH_SLIDER);
		wnd.SetTicFreq(101);
		//wnd.SetPageSize(101);

		// Spin button
		CUpDownCtrl(GetDlgItem(IDC_CGSS_STRENGTH_SPIN)).SetRange(0, 100);

		// Color button
		CConfigValue cValColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_SS_COLOR), &cValColor);
		m_wndColor.SetColor(CButtonColorPicker::SColor(cValColor.operator LONG()));
		m_wndColor.ShowWindow(SW_SHOW);

		DlgResize_Init(false, false, 0);
		return 1;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_COLOR(CFGID_SS_COLOR);
			CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToCOLORREF()));
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
	}

private:
	CButtonColorPicker m_wndColor;
};
