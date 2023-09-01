
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>
#include <WTL_Curve.h>
#include <WTL_CurveAxis.h>


class ATL_NO_VTABLE CConfigGUIRemoveHue :
	public CCustomConfigWndImpl<CConfigGUIRemoveHue>,
	public CDialogResize<CConfigGUIRemoveHue>
{
public:
	CConfigGUIRemoveHue() : m_wndColor(false)
	{
	}

	enum { IDD = IDD_CONFIGGUI_REMOVEHUE };

	BEGIN_MSG_MAP(CConfigGUIRemoveHue)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRemoveHue>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIRemoveHue>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGRHUE_CURVECTL, CCurveControl::CPC_POINT_MOVED, OnPointMoved)
		NOTIFY_HANDLER(IDC_CGRHUE_CURVECTL, CCurveControl::CPC_DRAG_FINISHED, OnDragFinished)
		NOTIFY_HANDLER(IDC_CGRHUE_CURVECTL, CCurveControl::CPC_DRAG_START, OnPointMoved)
		NOTIFY_HANDLER(IDC_CGRHUE_COLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		COMMAND_HANDLER(IDC_CGRHUE_X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGRHUE_Y, EN_CHANGE, OnEditChange)
		REFLECT_NOTIFICATIONS()
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 1.0f; return TRUE; }
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRemoveHue)
		DLGRESIZE_CONTROL(IDC_CGRHUE_CURVECTL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGRHUE_COLOR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_RADIO_HUE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_RADIO_COLOR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_RADIO_BSPLINE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_RADIO_LINEAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_X, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_Y, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_X_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_Y_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_COLORBAR, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_ALPHABAR, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGRHUE_GROUP_AXISX, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_GROUP_INTER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRHUE_GROUP_POINT, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGRHUE_ALPHABAR, DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRemoveHue)
		CONFIGITEM_RADIO(IDC_CGRHUE_RADIO_HUE, CFGID_X_AXIS, CFGVAL_X_AXIS_HUE)
		CONFIGITEM_RADIO(IDC_CGRHUE_RADIO_COLOR, CFGID_X_AXIS, CFGVAL_X_AXIS_COLOR)
		CONFIGITEM_RADIO(IDC_CGRHUE_RADIO_BSPLINE, CFGID_INTERPOLATION, CFGVAL_INTERPOLATION_BSPLINE)
		CONFIGITEM_RADIO(IDC_CGRHUE_RADIO_LINEAR, CFGID_INTERPOLATION, CFGVAL_INTERPOLATION_LINEAR)
		CONFIGITEM_CONTEXTHELP_IDS(IDC_CGRHUE_CURVECTL, IDS_HELP_CURVE)
		//CONFIGITEM_EDITBOX(IDC_CGRHUE_ANGLE, CFGID_HUE)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		// initialize color button
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CGRHUE_COLOR));
		m_wndColor.SetDefaultText(NULL);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_bEditUpdateCurve = true;

		RECT rcWnd;
		CWindow wnd(GetDlgItem(IDC_CGRHUE_CURVECTL));
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wnd.DestroyWindow();

		m_wndCurve.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGRHUE_CURVECTL);

		CConfigValue cValCurve;
		M_Config()->ItemValueGet(CComBSTR(CFGID_CURVE), &cValCurve);
		TCurvePoints aPoints;
		m_wndCurve.ParseCurvePoints(cValCurve.operator BSTR(), aPoints);
		m_wndCurve.SetPoints(aPoints);

		// Curve Axis
		CWindow wndX(GetDlgItem(IDC_CGRHUE_COLORBAR));
		wndX.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wndX.DestroyWindow();

		m_wndAxisX.Create(m_hWnd, &rcWnd, 0, 0, 0, IDC_CGRHUE_COLORBAR);

		CWindow wndY(GetDlgItem(IDC_CGRHUE_ALPHABAR));
		wndY.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wndY.DestroyWindow();

		m_wndAxisY.SetAxisType(CCurveAxisControl::ECurveAxisType::EATAlpha);
		m_wndAxisY.Create(m_hWnd, &rcWnd, 0, 0, 0, IDC_CGRHUE_ALPHABAR);

		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_CGRHUE_X_SPIN)).SetRange(0, 255);
		CUpDownCtrl(GetDlgItem(IDC_CGRHUE_Y_SPIN)).SetRange(0, 255);

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);

		GetDlgItem(IDC_CGRHUE_X).EnableWindow(FALSE);
		GetDlgItem(IDC_CGRHUE_Y).EnableWindow(FALSE);
		GetDlgItem(IDC_CGRHUE_X_SPIN).Invalidate();
		GetDlgItem(IDC_CGRHUE_Y_SPIN).Invalidate();

		return 1;
	}

	LRESULT OnPointMoved(int, LPNMHDR, BOOL&)
	{
		int nX, nY;
		m_bEditUpdateCurve = false;
		CEdit wndEditX(GetDlgItem(IDC_CGRHUE_X));
		CEdit wndEditY(GetDlgItem(IDC_CGRHUE_Y));
		if(m_wndCurve.GetPointPosition(nX, nY))
		{
			TCHAR psz[10];
			_itot(nX, psz, 10);
			wndEditX.EnableWindow(TRUE);
			wndEditX.SetWindowText(psz);
			_itot(nY, psz, 10);
			wndEditY.EnableWindow(TRUE);
			wndEditY.SetWindowText(psz);
		}else
		{
			wndEditX.SetWindowText(NULL);
			wndEditX.EnableWindow(FALSE);
			wndEditY.SetWindowText(NULL);
			wndEditY.EnableWindow(FALSE);
		}
		GetDlgItem(IDC_CGRHUE_X_SPIN).Invalidate();
		GetDlgItem(IDC_CGRHUE_Y_SPIN).Invalidate();
		m_bEditUpdateCurve = true;
		return 0;
	}
	LRESULT OnDragFinished(int, LPNMHDR, BOOL&)
	{
		try
		{
			TCurvePoints aPoints;
			m_wndCurve.GetPoints(&aPoints);
			CComBSTR cPoints;
			m_wndCurve.SerializeCurvePoints(aPoints, cPoints);

			CComBSTR cCFGID_CURVE(CFGID_CURVE);
			CConfigValue cValCurve(cPoints);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_CURVE;
			TConfigValue aVals[1];
			aVals[0] = cValCurve;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_HUE(CFGID_HUE);
			CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToCOLORREF()));
			BSTR aIDs[1];
			aIDs[0] = cCFGID_HUE;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnEditChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if(!m_bEditUpdateCurve)
			return 0;
		if(!GetDlgItem(IDC_CGRHUE_X).IsWindow() || !GetDlgItem(IDC_CGRHUE_Y).IsWindow())
			return 0;
		int nXo, nYo;
		if(!m_wndCurve.GetPointPosition(nXo, nYo))
			return 0;
		TCHAR szText[32] = _T("");
		GetDlgItem(IDC_CGRHUE_X).GetWindowText(szText, itemsof(szText));
		szText[itemsof(szText)-1] = _T('\0');
		int nX = _ttoi(szText);
		GetDlgItem(IDC_CGRHUE_Y).GetWindowText(szText, itemsof(szText));
		szText[itemsof(szText)-1] = _T('\0');
		int nY = _ttoi(szText);
		if(nXo != nX || nYo != nY)
		{
			m_wndCurve.SetPointPosition(nX, nY);
			OnDragFinished(0, NULL, a_bHandled);
		}
		return 0;
	}
	void ExtraConfigNotify()
	{
		CConfigValue cValHue, cValType;
		M_Config()->ItemValueGet(CComBSTR(CFGID_HUE), &cValHue);
		M_Config()->ItemValueGet(CComBSTR(CFGID_X_AXIS), &cValType);
		BYTE bR = cValHue.operator LONG() & 0xff;
		BYTE bG = (cValHue.operator LONG() & 0xff00) >> 8;
		BYTE bB = (cValHue.operator LONG() & 0xff0000) >> 16;
		if(cValType.operator LONG() == CFGVAL_X_AXIS_HUE)
			m_wndAxisX.SetAxisType(CCurveAxisControl::ECurveAxisType::EATHueDst, bR, bG, bB);
		else
			m_wndAxisX.SetAxisType(CCurveAxisControl::ECurveAxisType::EATColorDst, bR, bG, bB);
		if(m_wndAxisX.m_hWnd)
			m_wndAxisX.Invalidate();

		if (m_wndColor.m_hWnd)
		{
			CConfigValue cValColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_HUE), &cValColor);
			if (m_wndColor.GetColor() != CButtonColorPicker::SColor(cValColor.operator LONG()))
				m_wndColor.SetColor(CButtonColorPicker::SColor(cValColor.operator LONG()));
		}
		CConfigValue cValInterpolation;
		M_Config()->ItemValueGet(CComBSTR(CFGID_INTERPOLATION), &cValInterpolation);
		m_wndCurve.SetInterpolation(cValInterpolation.operator LONG() == CFGVAL_INTERPOLATION_BSPLINE);
	}

private:
	CCurveControl m_wndCurve;
	CCurveAxisControl m_wndAxisX;
	CCurveAxisControl m_wndAxisY;
	CButtonColorPicker m_wndColor;
	bool m_bEditUpdateCurve;
};

