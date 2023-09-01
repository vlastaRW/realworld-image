
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <ContextHelpDlg.h>
#include <math.h>


class ATL_NO_VTABLE CConfigGUIConvolution :
	public CCustomConfigResourcelessWndImpl<CConfigGUIConvolution>,
	public CDialogResize<CConfigGUIConvolution>,
	public CContextHelpDlg<CConfigGUIConvolution>
{
public:
	CConfigGUIConvolution() : m_bUpdating(true)
	{
	}

	enum
	{
		IDC_CGCV_QUICKEFFECT_LABEL = 100, IDC_CGCV_QUICKEFFECT,
		IDC_CGCV_EFFECTSTRENGTH_LABEL, IDC_CGCV_EFFECTSTRENGTH,
		IDC_CGCV_MATRIX_LABEL, IDC_CGCV_MATRIX,
		IDC_CGCV_DIVISOR_LABEL, IDC_CGCV_DIVISOR, IDC_CGCV_BIAS_LABEL, IDC_CGCV_BIAS,
		IDC_CGCV_IGNOREALPHA
	};

	BEGIN_DIALOG_EX(0, 0, 120, 136, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Effect:[0405]Efekt:"), IDC_CGCV_QUICKEFFECT_LABEL, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGCV_QUICKEFFECT, 44, 0, 75, 30, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Strength:[0405]Síla:"), IDC_CGCV_EFFECTSTRENGTH_LABEL, 0, 18, 44, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGCV_EFFECTSTRENGTH, TRACKBAR_CLASS, TBS_AUTOTICKS | TBS_BOTH | WS_TABSTOP | WS_VISIBLE, 44, 12, 75, 24, 0)
		CONTROL_LTEXT(_T("[0409]Convolution matrix:[0405]Konvoluční matice:"), IDC_CGCV_MATRIX_LABEL, 0, 40, 120, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCV_MATRIX, 0, 50, 120, 56, WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Divisor:[0405]Dělitel:"), IDC_CGCV_DIVISOR_LABEL, 0, 112, 35, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCV_DIVISOR, 35, 110, 21, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Bias:[0405]Hladina:"), IDC_CGCV_BIAS_LABEL, 63, 112, 35, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCV_BIAS, 98, 110, 21, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CHECKBOX(_T("[0409]Ignore alpha[0405]Ignorovat alfu"), IDC_CGCV_IGNOREALPHA, 0, 126, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIConvolution)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIConvolution>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIConvolution>)
		CHAIN_MSG_MAP(CContextHelpDlg<CConfigGUIConvolution>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		COMMAND_HANDLER(IDC_CGCV_QUICKEFFECT, CBN_SELCHANGE, OnQuickEffectSelChange)
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 0.5f; return TRUE; }
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIConvolution)
		DLGRESIZE_CONTROL(IDC_CGCV_MATRIX, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGCV_DIVISOR_LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGCV_DIVISOR, DLSZ_DIVSIZE_X(2)|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGCV_BIAS_LABEL, DLSZ_DIVMOVE_X(2)|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGCV_BIAS, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2)|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGCV_QUICKEFFECT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGCV_EFFECTSTRENGTH, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGCV_IGNOREALPHA, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIConvolution)
		CONFIGITEM_EDITBOX(IDC_CGCV_MATRIX, CFGID_MATRIX)
		CONFIGITEM_EDITBOX(IDC_CGCV_DIVISOR, CFGID_DIVISOR)
		CONFIGITEM_EDITBOX(IDC_CGCV_BIAS, CFGID_BIAS)
		CONFIGITEM_CHECKBOX(IDC_CGCV_IGNOREALPHA, CFGID_IGNOREALPHA)
	END_CONFIGITEM_MAP()

	BEGIN_CTXHELP_MAP(CConfigGUIConvolution)
		CTXHELP_CONTROL_RESOURCE(IDC_CGCV_QUICKEFFECT, IDS_HELP_QUICKEFFECT)
		CTXHELP_CONTROL_RESOURCE(IDC_CGCV_EFFECTSTRENGTH, IDS_HELP_EFFECTSTRENGTH)
	END_CTXHELP_MAP()

	enum EQuickEffectType
	{
		EQETCustom = 0,
		EQETSharpen,
		EQETSoften,
	};

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndEffectCombo = GetDlgItem(IDC_CGCV_QUICKEFFECT);
		m_wndEffectSlider = GetDlgItem(IDC_CGCV_EFFECTSTRENGTH);

		TCHAR szTmp[128] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_QUICKEFFECT_CUSTOM, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		m_wndEffectCombo.AddString(szTmp);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_QUICKEFFECT_SHARPEN, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		m_wndEffectCombo.AddString(szTmp);
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_QUICKEFFECT_SOFTEN, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
		m_wndEffectCombo.AddString(szTmp);

		m_wndEffectSlider.SetRange(0, 40);
		m_wndEffectSlider.SetTicFreq(10);
		m_wndEffectSlider.SetPageSize(10);

		m_bUpdating = false;
		ExtraConfigNotify();

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnHScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_lParam && reinterpret_cast<HWND>(a_lParam) != m_wndEffectSlider)
		{
			a_bHandled = FALSE;
			return 0;
		}

		if (LOWORD(a_wParam) == SB_ENDSCROLL)
		{
			CutomEffectToConfig();
			return 0;
		}
		else
		{
			//TCHAR sz[32] = _T("");
			//_stprintf(sz, _T("%g"), powf(10.0f, m_wndGammaSlider.GetPos()*0.05f-1.0f));
			//SetDlgItemText(IDC_CGCT_GAMMA_EDIT, sz);
			return 1;
		}
	}

	LRESULT OnQuickEffectSelChange(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_wndEffectCombo.GetCurSel() == EQETCustom)
		{
			m_wndEffectSlider.EnableWindow(FALSE);
			m_wndEffectSlider.SetPos(20);
		}
		else
		{
			m_wndEffectSlider.EnableWindow();
			m_wndEffectSlider.SetPos(20);
			CutomEffectToConfig();
		}
		return 0;
	}

	void CutomEffectToConfig()
	{
		CConfigValue cMatrix;
		CConfigValue cDivisor;
		CConfigValue cBias;
		int nPos = m_wndEffectSlider.GetPos();
		OLECHAR szTmp[128] = L"";
		switch (m_wndEffectCombo.GetCurSel())
		{
		case EQETSharpen:
			_swprintf(szTmp, L"0 -2 0\r\n-2 %g -2\r\n0 -2 0", 11.0f+0.5f*(40-nPos));
			cMatrix = szTmp;
			cDivisor = 3.0f+0.5f*(40-nPos);
			cBias = 0.0f;
			break;
		case EQETSoften:
			_swprintf(szTmp, L"1 1 1\r\n1 %g 1\r\n1 1 1", powf(3, (40.0f-nPos)/20.0f));
			cMatrix = szTmp;
			cDivisor = 8.0f+powf(3, (40.0f-nPos)/20.0f);
			cBias = 0.0f;
			break;
		default:
			ATLASSERT(0);
			return;
		}
		CComBSTR cCFGID_MATRIX(CFGID_MATRIX);
		CComBSTR cCFGID_DIVISOR(CFGID_DIVISOR);
		CComBSTR cCFGID_BIAS(CFGID_BIAS);
		BSTR aIDs[3] = {cCFGID_MATRIX, cCFGID_DIVISOR, cCFGID_BIAS};
		TConfigValue aVals[3] = {cMatrix, cDivisor, cBias};
		m_bUpdating = true;
		M_Config()->ItemValuesSet(3, aIDs, aVals);
		m_bUpdating = false;
	}

	void ExtraConfigNotify()
	{
		if (m_bUpdating || m_hWnd == NULL)
			return;

		CConfigValue cMatrix;
		CConfigValue cDivisor;
		CConfigValue cBias;
		M_Config()->ItemValueGet(CComBSTR(CFGID_MATRIX), &cMatrix);
		M_Config()->ItemValueGet(CComBSTR(CFGID_DIVISOR), &cDivisor);
		M_Config()->ItemValueGet(CComBSTR(CFGID_BIAS), &cBias);
		// try to find out whether it is a sharpen or soften matrix and set controls accordingly
		EQuickEffectType eType = EQETCustom;
		if (cBias.operator float() == 0.0f)
		{
			std::vector<float> cKernel;
			size_t nKernelX = 0;
			size_t nKernelY = 0;
			if (ParseMatrix(cMatrix, cKernel, nKernelX, nKernelY) && nKernelX == 3 && nKernelY == 3)
			{
				if (cKernel[0] == 0.0f && cKernel[1] ==-2.0f && cKernel[2] == 0.0f && 
					cKernel[3] ==-2.0f && fabsf(cKernel[4]-cDivisor.operator float()-8.0f) < 0.001f && cKernel[5] ==-2.0f && 
					cKernel[6] == 0.0f && cKernel[7] ==-2.0f && cKernel[8] == 0.0f)
				{
					eType = EQETSharpen;
					m_wndEffectSlider.SetPos(40-(cDivisor.operator float()-3.0f)*2.0f + 0.5f);
				}
				else
				if (cKernel[0] == 1.0f && cKernel[1] == 1.0f && cKernel[2] == 1.0f && 
					cKernel[3] == 1.0f && fabsf(cKernel[4]-cDivisor.operator float()+8.0f) < 0.001f && cKernel[5] == 1.0f && 
					cKernel[6] == 1.0f && cKernel[7] == 1.0f && cKernel[8] == 1.0f)
				{
					eType = EQETSoften;
					m_wndEffectSlider.SetPos(40 - logf(cDivisor.operator float()-8.0f)*18.2047845325367f + 0.5f);
				}
			}
		}
		m_wndEffectCombo.SetCurSel(eType);
		if (eType == EQETCustom)
			m_wndEffectSlider.SetPos(20);
		m_wndEffectSlider.EnableWindow(eType != EQETCustom);
	}

private:
	CComboBox m_wndEffectCombo;
	CTrackBarCtrl m_wndEffectSlider;
	bool m_bUpdating;
};

