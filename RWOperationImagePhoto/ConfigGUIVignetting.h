
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>
#include <WTL_Curve.h>
#include <WTL_CurveAxis.h>

class ATL_NO_VTABLE CConfigGUIVignetting :
	public CCustomConfigWndImpl<CConfigGUIVignetting>,
	public CDialogResize<CConfigGUIVignetting>
{
public:
	CConfigGUIVignetting() : m_wndColor(false)
	{
	}

	enum { IDD = IDD_CONFIGGUI_VIGNETTING };

	BEGIN_MSG_MAP(CConfigGUIVignetting)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIVignetting>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIVignetting>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGVIG_CURVECTL, CCurveControl::CPC_POINT_MOVED, OnPointMoved)
		NOTIFY_HANDLER(IDC_CGVIG_CURVECTL, CCurveControl::CPC_DRAG_FINISHED, OnDragFinished)
		NOTIFY_HANDLER(IDC_CGVIG_CURVECTL, CCurveControl::CPC_DRAG_START, OnPointMoved)
		NOTIFY_HANDLER(IDC_CGVIG_COLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		COMMAND_HANDLER(IDC_CGVIG_X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGVIG_Y, EN_CHANGE, OnEditChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIVignetting)
		DLGRESIZE_CONTROL(IDC_CGVIG_CURVECTL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGVIG_AUTO, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_COLOR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_RADIO_BSPLINE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_RADIO_LINEAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_X, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_Y, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_X_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_Y_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_BAR, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGVIG_GROUP_INTER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGVIG_GROUP_POINT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIVignetting)
		CONFIGITEM_CHECKBOX(IDC_CGVIG_AUTO, CFGID_VIG_AUTO_UNVIG)
		CONFIGITEM_RADIO(IDC_CGVIG_RADIO_BSPLINE, CFGID_VIG_INTERPOLATION, CFGVAL_INTERPOLATION_BSPLINE)
		CONFIGITEM_RADIO(IDC_CGVIG_RADIO_LINEAR, CFGID_VIG_INTERPOLATION, CFGVAL_INTERPOLATION_LINEAR)
		CONFIGITEM_CONTEXTHELP_IDS(IDC_CGVIG_CURVECTL, IDS_HELP_CURVE)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		// initialize color button
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CGVIG_COLOR));
		m_wndColor.SetDefaultText(NULL);
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_bEditUpdateCurve = true;

		RECT rcWnd;
		CWindow wnd(GetDlgItem(IDC_CGVIG_CURVECTL));
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wnd.DestroyWindow();

		m_wndCurve.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGVIG_CURVECTL);

		CConfigValue cValCurve;
		M_Config()->ItemValueGet(CComBSTR(CFGID_VIG_CURVE), &cValCurve);
		TCurvePoints aPoints;
		m_wndCurve.ParseCurvePoints(cValCurve.operator BSTR(), aPoints);
		m_wndCurve.SetPoints(aPoints);

		// initialize color button
		CConfigValue cValColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_VIG_COLOR), &cValColor);

		m_wndColor.SetColor(cValColor.operator LONG());

		// Curve Axis
		CWindow wndY(GetDlgItem(IDC_CGVIG_BAR));
		wndY.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wndY.DestroyWindow();

		BYTE bR = cValColor.operator LONG() & 0xff;
		BYTE bG = (cValColor.operator LONG() & 0xff00) >> 8;
		BYTE bB = (cValColor.operator LONG() & 0xff0000) >> 16;
		m_wndAxisY.SetAxisType(CCurveAxisControl::ECurveAxisType::EATVignetting, bR, bG, bB);
		m_wndAxisY.Create(m_hWnd, &rcWnd, 0, 0, 0, IDC_CGVIG_BAR);

		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_CGVIG_X_SPIN)).SetRange(0, 100);
		CUpDownCtrl(GetDlgItem(IDC_CGVIG_Y_SPIN)).SetRange(-100, 100);

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);

		GetDlgItem(IDC_CGVIG_X).EnableWindow(FALSE);
		GetDlgItem(IDC_CGVIG_Y).EnableWindow(FALSE);
		GetDlgItem(IDC_CGVIG_X_SPIN).Invalidate();
		GetDlgItem(IDC_CGVIG_Y_SPIN).Invalidate();

		return 1;
	}

	LRESULT OnPointMoved(int, LPNMHDR, BOOL&)
	{
		float fX, fY;
		m_bEditUpdateCurve = false;
		CEdit wndEditX(GetDlgItem(IDC_CGVIG_X));
		CEdit wndEditY(GetDlgItem(IDC_CGVIG_Y));
		if(m_wndCurve.GetPointPosition(fX, fY))
		{
			int nX, nY;
			nX = 100.0f*fX/255.0f + 0.5f;
			if(fY >= 127.5f)
				nY = 200.0f*fY/255.0f - 99.5f;
			else
				nY = 200.0f*fY/255.0f - 100.0f;
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
		GetDlgItem(IDC_CGVIG_X_SPIN).Invalidate();
		GetDlgItem(IDC_CGVIG_Y_SPIN).Invalidate();
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

			CComBSTR cCFGID_CURVE(CFGID_VIG_CURVE);
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
			CComBSTR cCFGID_HUE(CFGID_VIG_COLOR);
			CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToCOLORREF()));
			BSTR aIDs[1];
			aIDs[0] = cCFGID_HUE;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);

			BYTE bR = pClrBtn->clr.ToCOLORREF() & 0xff;
			BYTE bG = (pClrBtn->clr.ToCOLORREF() & 0xff00) >> 8;
			BYTE bB = (pClrBtn->clr.ToCOLORREF() & 0xff0000) >> 16;
			m_wndAxisY.SetAxisType(CCurveAxisControl::ECurveAxisType::EATVignetting, bR, bG, bB);
			m_wndAxisY.Invalidate();
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
		if(!GetDlgItem(IDC_CGVIG_X).IsWindow() || !GetDlgItem(IDC_CGVIG_Y).IsWindow())
			return 0;
		int nXo, nYo;
		if(!m_wndCurve.GetPointPosition(nXo, nYo))
			return 0;
		TCHAR szText[32] = _T("");
		GetDlgItem(IDC_CGVIG_X).GetWindowText(szText, itemsof(szText));
		szText[itemsof(szText)-1] = _T('\0');
		int nX = _ttoi(szText);
		float fX = 2.55f * nX;
		GetDlgItem(IDC_CGVIG_Y).GetWindowText(szText, itemsof(szText));
		szText[itemsof(szText)-1] = _T('\0');
		int nY = _ttoi(szText);
		float fY = 255.0f * (nY + 100.0f) / 200.0f;
		if(nXo != nX || nYo != nY+127)
		{
			m_wndCurve.SetPointPosition(fX, fY);
			OnDragFinished(0, NULL, a_bHandled);
		}
		return 0;
	}
	void ExtraConfigNotify()
	{
		//if (m_wndColor.m_hWnd)
		//{
		//	if (m_wndColor.GetColor() != cValColor.operator LONG())
		//		m_wndColor.SetColor(cValColor.operator LONG());
		//}
		CConfigValue cValInterpolation;
		M_Config()->ItemValueGet(CComBSTR(CFGID_VIG_INTERPOLATION), &cValInterpolation);
		m_wndCurve.SetInterpolation(cValInterpolation.operator LONG() == CFGVAL_INTERPOLATION_BSPLINE);

		CConfigValue cValAuto;
		M_Config()->ItemValueGet(CComBSTR(CFGID_VIG_AUTO_UNVIG), &cValAuto);
		if(GetDlgItem(IDC_CGVIG_BAR).IsWindow()) GetDlgItem(IDC_CGVIG_BAR).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_CURVECTL).IsWindow()) GetDlgItem(IDC_CGVIG_CURVECTL).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_COLOR).IsWindow()) GetDlgItem(IDC_CGVIG_COLOR).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_GROUP_INTER).IsWindow()) GetDlgItem(IDC_CGVIG_GROUP_INTER).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_RADIO_BSPLINE).IsWindow()) GetDlgItem(IDC_CGVIG_RADIO_BSPLINE).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_RADIO_LINEAR).IsWindow()) GetDlgItem(IDC_CGVIG_RADIO_LINEAR).EnableWindow(!cValAuto.operator bool());
		if(GetDlgItem(IDC_CGVIG_GROUP_POINT).IsWindow()) GetDlgItem(IDC_CGVIG_GROUP_POINT).EnableWindow(!cValAuto.operator bool());
		float f1, f2;
		bool bPoint = m_wndCurve.GetPointPosition(f1, f2);
		if(GetDlgItem(IDC_CGVIG_X).IsWindow()) GetDlgItem(IDC_CGVIG_X).EnableWindow(!cValAuto.operator bool() && bPoint);
		if(GetDlgItem(IDC_CGVIG_Y).IsWindow()) GetDlgItem(IDC_CGVIG_Y).EnableWindow(!cValAuto.operator bool() && bPoint);
		if(GetDlgItem(IDC_CGVIG_X_SPIN).IsWindow()) GetDlgItem(IDC_CGVIG_X_SPIN).Invalidate();
		if(GetDlgItem(IDC_CGVIG_Y_SPIN).IsWindow()) GetDlgItem(IDC_CGVIG_Y_SPIN).Invalidate();
	}

private:
	CCurveControl m_wndCurve;
	CCurveAxisControl m_wndAxisY;
	CButtonColorPicker m_wndColor;
	bool m_bEditUpdateCurve;
};

