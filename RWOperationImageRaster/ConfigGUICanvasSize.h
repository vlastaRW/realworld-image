
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_2DPosition.h>


class ATL_NO_VTABLE CConfigGUICanvasSize :
	public CCustomConfigResourcelessWndImpl<CConfigGUICanvasSize>,
	public CDialogResize<CConfigGUICanvasSize>
{
public:
	CConfigGUICanvasSize() : m_bInitialized(false),
		m_nSizeX(0), m_nSizeY(0)
	{
		m_szTemplate[0] = _T('\0');
	}

	enum
	{
		IDC_CGCS_RELATIVE = 100, IDC_CGCS_ABSOLUTE,
		IDC_CGCS_SIZE_X, IDC_CGCS_SIZE_X_SPIN, IDC_CGCS_SIZE_LABEL, IDC_CGCS_SIZE_Y, IDC_CGCS_SIZE_Y_SPIN, IDC_CGCS_SIZE_UNIT,
		IDC_CGCS_ABSSIZE_X, IDC_CGCS_ABSSIZE_X_SPIN, IDC_CGCS_ABSSIZE_LABEL, IDC_CGCS_ABSSIZE_Y, IDC_CGCS_ABSSIZE_Y_SPIN, IDC_CGCS_ABSSIZE_UNIT,
		IDC_CGCS_MESSAGE, IDC_CGCS_ALIGNMENT, IDC_CGCS_ALIGN_AREA
	};

	BEGIN_DIALOG_EX(0, 0, 120, 110, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_RADIOBUTTON(_T("[0409]Adjust size by:[0405]Změnit velikost o:"), IDC_CGCS_RELATIVE, 0, 0, 120, 12, WS_GROUP | WS_VISIBLE, 0)
		CONTROL_RADIOBUTTON(_T("[0409]Set size to:[0405]Nastavit velikost na:"), IDC_CGCS_ABSOLUTE, 0, 30, 120, 12, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCS_SIZE_X, 10, 14, 36, 12, WS_GROUP | WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGCS_SIZE_X_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 34, 14, 11, 12, 0)
		CONTROL_LTEXT(_T("x"), IDC_CGCS_SIZE_LABEL, 50, 16, 8, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCS_SIZE_Y, 58, 14, 36, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGCS_SIZE_Y_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 82, 14, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixelů"), IDC_CGCS_SIZE_UNIT, 99, 16, 21, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCS_ABSSIZE_X, 10, 44, 36, 12, WS_GROUP | WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGCS_ABSSIZE_X_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 34, 44, 11, 12, 0)
		CONTROL_LTEXT(_T("x"), IDC_CGCS_ABSSIZE_LABEL, 50, 46, 8, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGCS_ABSSIZE_Y, 58, 44, 36, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGCS_ABSSIZE_Y_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 82, 44, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixelů"), IDC_CGCS_ABSSIZE_UNIT, 99, 46, 21, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Alignment:[0405]Pozice:"), IDC_STATIC, 0, 61, 44, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGCS_ALIGNMENT, 0, 73, 64, 100, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("\\"), IDC_CGCS_TOP_LEFT, 78, 60, 13, 12, BS_ICON | BS_PUSHLIKE | WS_GROUP | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("|"), IDC_CGCS_TOP, 92, 60, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("/"), IDC_CGCS_TOP_RIGHT, 106, 60, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("-"), IDC_CGCS_LEFT, 78, 73, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("+"), IDC_CGCS_CENTER, 92, 73, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("-"), IDC_CGCS_RIGHT, 106, 73, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("/"), IDC_CGCS_BOTTOM_LEFT, 78, 86, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("|"), IDC_CGCS_BOTTOM, 92, 86, 13, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("\\"), IDC_CGCS_BOTTOM_RIGHT, 106, 86, 12, 12, BS_ICON | BS_PUSHLIKE | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Resizing from %s x %s to %s x %s pixels.[0405]Změna velikosti z %s x %s na %s x %s pixelů."), IDC_CGCS_MESSAGE, 0, 102, 120, 8, SS_ENDELLIPSIS | WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUICanvasSize)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUICanvasSize>)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUICanvasSize>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGCS_ALIGN_AREA, CRectanglePosition::C2DP_POSITION_CHANGED, OnAlignmentChanged)
		COMMAND_HANDLER(IDC_CGCS_ALIGNMENT, CBN_SELCHANGE, OnAlignSelChange)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICanvasSize)
		CONFIGITEM_RADIO(IDC_CGCS_RELATIVE, CFGID_CS_TYPE, CFGVAL_CST_DELTA)
		CONFIGITEM_RADIO(IDC_CGCS_ABSOLUTE, CFGID_CS_TYPE, CFGVAL_CST_SIZE)
		CONFIGITEM_EDITBOX(IDC_CGCS_SIZE_X, CFGID_CS_DELTA_X)
		CONFIGITEM_EDITBOX(IDC_CGCS_SIZE_Y, CFGID_CS_DELTA_Y)
		CONFIGITEM_EDITBOX(IDC_CGCS_ABSSIZE_X, CFGID_CS_SIZE_X)
		CONFIGITEM_EDITBOX(IDC_CGCS_ABSSIZE_Y, CFGID_CS_SIZE_Y)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUICanvasSize)
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_X_SPIN, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_LABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_Y_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGCS_SIZE_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_X_SPIN, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_LABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_Y_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGCS_ABSSIZE_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGCS_ALIGN_AREA, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	void ExtraInitDialog()
	{
		RECT rc = {68, 60, 118, 98};
		MapDialogRect(&rc);
		m_wndAlign.Create(m_hWnd, &rc, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, IDC_CGCS_ALIGN_AREA);
		m_wndAlignCombo = GetDlgItem(IDC_CGCS_ALIGNMENT);
		static LPCWSTR options[] =
		{
			L"[0409]Top left[0405]Nahoře vlevo",
			L"[0409]Top[0405]Nahoře",
			L"[0409]Top right[0405]Nahoře vpravo",
			L"[0409]Left[0405]Vlevo",
			L"[0409]Center[0405]Uprostřed",
			L"[0409]Right[0405]Vpravo",
			L"[0409]Bottom left[0405]Dole vlevo",
			L"[0409]Bottom[0405]Dole",
			L"[0409]Bottom right[0405]Dole vpravo",
			L"[0409]Custom[0405]Vlastní",
		};
		for (LPCWSTR* p = options, *const e = options+sizeof(options)/sizeof(options[0]); p != e; ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			m_wndAlignCombo.AddString(bstr);
		}

		m_wndMessage = GetDlgItem(IDC_CGCS_MESSAGE);
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfHeight = (lf.lfHeight*5)/6;
		m_cMessageFont.CreateFontIndirect(&lf);
		m_wndMessage.SetFont(m_cMessageFont);
		m_wndMessage.GetWindowText(m_szTemplate, itemsof(m_szTemplate));
	}

	// IDocumentForConfig methods
public:
	void GetDocumentInfo()
	{
		try
		{
			CComPtr<IDocument> pDoc;
			GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
			CComPtr<IDocumentImage> pDI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
			if (pDI)
			{
				TImageSize tIS = {0, 0};
				if (SUCCEEDED(pDI->CanvasGet(&tIS, NULL, NULL, NULL, NULL)))
				{
					m_nSizeX = tIS.nX;
					m_nSizeY = tIS.nY;
				}
				return;
			}
			CComPtr<IDocumentAnimation> pDA;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDA));
			if (pDA)
			{
				TImageSize tSize = GetAnimationSize(pDA);
				if (tSize.nX*tSize.nY)
				{
					m_nSizeX = tSize.nX;
					m_nSizeY = tSize.nY;
				}
				return;
			}
		}
		catch (...)
		{
		}
	}

	// message handlers
public:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_CGCS_SIZE_X_SPIN)).SetRange(-32767, 32767);
		CUpDownCtrl(GetDlgItem(IDC_CGCS_SIZE_Y_SPIN)).SetRange(-32767, 32767);
		CUpDownCtrl(GetDlgItem(IDC_CGCS_ABSSIZE_X_SPIN)).SetRange(1, 32767);
		CUpDownCtrl(GetDlgItem(IDC_CGCS_ABSSIZE_Y_SPIN)).SetRange(1, 32767);

		GetDocumentInfo();

		if (m_nSizeX && m_nSizeY)
			InitAbsSize();
		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);
		m_bInitialized = true;
		return 1;
	}
	LRESULT OnLButtonDown(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		ClientToScreen(&tPt);

		struct SMap {UINT uID; LONG iVal;} aIDs[] =
		{
			{IDC_CGCS_SIZE_X, CFGVAL_CST_DELTA},
			{IDC_CGCS_SIZE_Y, CFGVAL_CST_DELTA},
			{IDC_CGCS_ABSSIZE_X, CFGVAL_CST_SIZE},
			{IDC_CGCS_ABSSIZE_Y, CFGVAL_CST_SIZE},
		};
		CComBSTR cCFGID_CS_TYPE(CFGID_CS_TYPE);
		for (size_t i = 0; i < itemsof(aIDs); ++i)
		{
			CWindow wnd = GetDlgItem(aIDs[i].uID);
			RECT rc;
			wnd.GetWindowRect(&rc);
			if (tPt.x < rc.left || tPt.x >= rc.right ||
				tPt.y < rc.top || tPt.y >= rc.bottom)
				continue;
			if (wnd.GetStyle()&WS_DISABLED)
			{
				M_Config()->ItemValuesSet(1, &(cCFGID_CS_TYPE.m_str), CConfigValue(aIDs[i].iVal));
				if ((wnd.GetStyle()&WS_DISABLED) == 0)
					GotoDlgCtrl(wnd);
				return 0;
			}
			break;
		}
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnAlignmentChanged(int, LPNMHDR, BOOL& b)
	{
		CComBSTR bstrCFGID_CS_POSITION(CFGID_CS_POSITION);
		CConfigValue cVal(m_wndAlign.GetPositionX(), m_wndAlign.GetPositionY());
		M_Config()->ItemValuesSet(1, &(bstrCFGID_CS_POSITION.m_str), cVal);
		return 0;
	}

	LRESULT OnAlignSelChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int const sel = m_wndAlignCombo.GetCurSel();
		if (sel >= 0 && sel < 9)
		{
			CComBSTR bstrCFGID_CS_POSITION(CFGID_CS_POSITION);
			CConfigValue cVal(float((sel%3)-1), float((sel/3)-1));
			M_Config()->ItemValuesSet(1, &(bstrCFGID_CS_POSITION.m_str), cVal);
		}
		return 0;
	}

	void ExtraConfigNotify()
	{
		CConfigValue cPos;
		M_Config()->ItemValueGet(CComBSTR(CFGID_CS_POSITION), &cPos);
		m_wndAlign.SetPosition(cPos[0], cPos[1]);
		int x = -1;
		int y = -1;
		if (cPos[0] == -1.0f) x = 0;
		else if (cPos[0] == 0.0f) x = 1;
		else if (cPos[0] == 1.0f) x = 2;
		if (cPos[1] == -1.0f) y = 0;
		else if (cPos[1] == 0.0f) y = 1;
		else if (cPos[1] == 1.0f) y = 2;
		if (x != -1 && y != -1)
		{
			m_wndAlignCombo.SetCurSel(x+3*y);
		}
		else
		{
			m_wndAlignCombo.SetCurSel(9);
		}
		//if (m_nSizeX*m_nSizeY)
		//{
		//	// possibly update the position control
		//	CConfigValue cSize;
		//	M_Config()->ItemValueGet(CComBSTR(CFGID_RELSIZE), &cSize);
		//	LONG nCanvasX = static_cast<LONG>(0.5f+(m_nSizeX*100.0f)/cSize.operator float());
		//	LONG nCanvasY = static_cast<LONG>(0.5f+(m_nSizeY*100.0f)/cSize.operator float());
		//	if (cSquare)
		//	{
		//		if (nCanvasX > nCanvasY)
		//			nCanvasY = nCanvasX;
		//		else
		//			nCanvasX = nCanvasY;
		//	}
		//	if (m_canvasSize.nX != nCanvasX || m_canvasSize.nY != nCanvasY)
		//	{
		//		m_canvasSize.nX = nCanvasX;
		//		m_canvasSize.nY = nCanvasY;
		//		m_wndAlign.SetVisualizer(CRectangleVisualizer(float(m_contentSize.nX)/m_canvasSize.nX, float(m_contentSize.nY)/m_canvasSize.nY));
		//	}
		//}
		UpdateMessage();
	}

	void UpdateMessage()
	{
		if (m_wndMessage.m_hWnd)
		{
			TCHAR szSrcX[32] = _T("?");
			TCHAR szSrcY[32] = _T("?");
			TCHAR szDstX[32] = _T("?");
			TCHAR szDstY[32] = _T("?");
			if (m_nSizeX && m_nSizeY)
			{
				_stprintf(szSrcX, _T("%i"), m_nSizeX);
				_stprintf(szSrcY, _T("%i"), m_nSizeY);
			}
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CS_TYPE), &cVal);
			switch (cVal.operator LONG())
			{
			case CFGVAL_CST_DELTA:
				if (m_nSizeX && m_nSizeY)
				{
					M_Config()->ItemValueGet(CComBSTR(CFGID_CS_DELTA_X), &cVal);
					LONG nSizeX = m_nSizeX + cVal.operator LONG();
					M_Config()->ItemValueGet(CComBSTR(CFGID_CS_DELTA_Y), &cVal);
					LONG nSizeY = m_nSizeY + cVal.operator LONG();
					if (nSizeX < 1) nSizeX = 1;
					if (nSizeY < 1) nSizeY = 1;
					_stprintf(szDstX, _T("%i"), nSizeX);
					_stprintf(szDstY, _T("%i"), nSizeY);
				}
				break;
			case CFGVAL_CST_SIZE:
				M_Config()->ItemValueGet(CComBSTR(CFGID_CS_SIZE_X), &cVal);
				_stprintf(szDstX, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
				M_Config()->ItemValueGet(CComBSTR(CFGID_CS_SIZE_Y), &cVal);
				_stprintf(szDstY, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
				break;
			}
			TCHAR szMsg[384];
			_stprintf(szMsg, m_szTemplate, szSrcX, szSrcY, szDstX, szDstY);
			m_wndMessage.SetWindowText(szMsg);
		}
	}
	void InitAbsSize()
	{
		if (M_Config() == NULL)
			return;

		CComBSTR bstrLastX(CFGID_CS_LASTSIZE_X);
		CComBSTR bstrLastY(CFGID_CS_LASTSIZE_Y);
		CConfigValue cLastSizeX;
		CConfigValue cLastSizeY;
		M_Config()->ItemValueGet(bstrLastX, &cLastSizeX);
		M_Config()->ItemValueGet(bstrLastY, &cLastSizeY);
		if (m_nSizeX == cLastSizeX.operator LONG() && m_nSizeY == cLastSizeY.operator LONG())
			return; // do not set absolute size to current size if last size was the same
		CComBSTR bstrX(CFGID_CS_SIZE_X);
		CComBSTR bstrY(CFGID_CS_SIZE_Y);
		BSTR aIDs[] = {bstrX, bstrY, bstrLastX, bstrLastY};
		TConfigValue aVals[4];
		aVals[0].eTypeID = ECVTInteger;
		aVals[0].iVal = m_nSizeX;
		aVals[1].eTypeID = ECVTInteger;
		aVals[1].iVal = m_nSizeY;
		aVals[2].eTypeID = ECVTInteger;
		aVals[2].iVal = m_nSizeX;
		aVals[3].eTypeID = ECVTInteger;
		aVals[3].iVal = m_nSizeY;
		M_Config()->ItemValuesSet(4, aIDs, aVals);
	}

private:
	bool m_bInitialized;
	CRectanglePosition m_wndAlign;
	CComboBox m_wndAlignCombo;
	CStatic m_wndMessage;
	CFont m_cMessageFont;
	TCHAR m_szTemplate[256];
	ULONG m_nSizeX;
	ULONG m_nSizeY;
};
