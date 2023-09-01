
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <WTL_2DPosition.h>


class ATL_NO_VTABLE CConfigGUIRemoveBorder :
	public CCustomConfigResourcelessWndImpl<CConfigGUIRemoveBorder>,
	public CDialogResize<CConfigGUIRemoveBorder>
{
public:
	enum { IDC_CGRB_ALIGN_AREA = 222 };

	BEGIN_DIALOG_EX(0, 0, 145, 42, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Alignment:[0405]Pozice:"), IDC_STATIC, 0, 2, 46, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGRB_ALIGN, 48, 0, 41, 82, WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Image size:[0405]Velikost:"), IDC_STATIC, 0, 18, 46, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGRB_SIZEEDIT, 48, 16, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_CGRB_SIZESPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | WS_VISIBLE, 62, 16, 12, 12, 0)
		CONTROL_LTEXT(_T("%"), IDC_CGRB_SIZEUNIT, 78, 18, 8, 8, WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]S&quare[0405]Čtverec"), IDC_CGRB_SQUARE, 0, 32, 41, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIRemoveBorder)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRemoveBorder>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIRemoveBorder>)
		NOTIFY_HANDLER(IDC_CGRB_ALIGN_AREA, CRectanglePosition::C2DP_POSITION_CHANGED, OnAlignmentChanged)
		COMMAND_HANDLER(IDC_CGRB_ALIGN, CBN_SELCHANGE, OnAlignSelChange)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRemoveBorder)
		DLGRESIZE_CONTROL(IDC_CGRB_ALIGN, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGRB_SIZEEDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGRB_SIZESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRB_SIZEUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGRB_ALIGN_AREA, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRemoveBorder)
		CONFIGITEM_EDITBOX(IDC_CGRB_SIZEEDIT, CFGID_RELSIZE)
		CONFIGITEM_CONTEXTHELP(IDC_CGRB_SIZESPIN, CFGID_RELSIZE)
		CONFIGITEM_CHECKBOX(IDC_CGRB_SQUARE, CFGID_SQUARE)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		RECT rc = {95, 0, 145, 38};
		MapDialogRect(&rc);
		m_wndAlign.Create(m_hWnd, &rc, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, IDC_CGRB_ALIGN_AREA);
		m_wndAlignCombo = GetDlgItem(IDC_CGRB_ALIGN);
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

		m_contentSize.nX = m_contentSize.nY = m_canvasSize.nX = m_canvasSize.nY = 0;
		CComPtr<IDocument> pDoc;
		GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
		CComPtr<IDocumentImage> pDI;
		if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
		if (pDI)pDI->CanvasGet(NULL, NULL, NULL, &m_contentSize, NULL);
	}
	void ExtraConfigNotify()
	{
		CConfigValue aX;
		M_Config()->ItemValueGet(CComBSTR(CFGID_ALIGNMENTX), &aX);
		CConfigValue aY;
		M_Config()->ItemValueGet(CComBSTR(CFGID_ALIGNMENTY), &aY);
		m_wndAlign.SetPosition(aX, aY);
		int x = -1;
		int y = -1;
		if (aX.operator float() == -1.0f) x = 0;
		else if (aX.operator float() == 0.0f) x = 1;
		else if (aX.operator float() == 1.0f) x = 2;
		if (aY.operator float() == -1.0f) y = 0;
		else if (aY.operator float() == 0.0f) y = 1;
		else if (aY.operator float() == 1.0f) y = 2;
		if (x != -1 && y != -1)
		{
			m_wndAlignCombo.SetCurSel(x+3*y);
		}
		else
		{
			m_wndAlignCombo.SetCurSel(9);
		}
		if (m_contentSize.nX*m_contentSize.nY)
		{
			// possibly update the position control
			CConfigValue cSize;
			M_Config()->ItemValueGet(CComBSTR(CFGID_RELSIZE), &cSize);
			CConfigValue cSquare;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SQUARE), &cSquare);
			LONG nCanvasX = static_cast<LONG>(0.5f+(m_contentSize.nX*100.0f)/cSize.operator float());
			LONG nCanvasY = static_cast<LONG>(0.5f+(m_contentSize.nY*100.0f)/cSize.operator float());
			if (cSquare)
			{
				if (nCanvasX > nCanvasY)
					nCanvasY = nCanvasX;
				else
					nCanvasX = nCanvasY;
			}
			if (m_canvasSize.nX != nCanvasX || m_canvasSize.nY != nCanvasY)
			{
				m_canvasSize.nX = nCanvasX;
				m_canvasSize.nY = nCanvasY;
				m_wndAlign.SetVisualizer(CRectangleVisualizer(float(m_contentSize.nX)/m_canvasSize.nX, float(m_contentSize.nY)/m_canvasSize.nY));
			}
		}
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CUpDownCtrl wnd = GetDlgItem(IDC_CGRB_SIZESPIN);
		wnd.SetRange(10, 100);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnAlignmentChanged(int, LPNMHDR, BOOL& b)
	{
		CComBSTR bstrCFGID_ALIGNMENTX(CFGID_ALIGNMENTX);
		CComBSTR bstrCFGID_ALIGNMENTY(CFGID_ALIGNMENTY);
		BSTR aIDs[2] = {bstrCFGID_ALIGNMENTX, bstrCFGID_ALIGNMENTY};
		TConfigValue aVals[2];
		aVals[0].eTypeID = aVals[1].eTypeID = ECVTFloat;
		aVals[0].fVal = m_wndAlign.GetPositionX();
		aVals[1].fVal = m_wndAlign.GetPositionY();
		M_Config()->ItemValuesSet(2, aIDs, aVals);
		return 0;
	}

	LRESULT OnAlignSelChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int const sel = m_wndAlignCombo.GetCurSel();
		if (sel >= 0 && sel < 9)
		{
			CComBSTR bstrCFGID_ALIGNMENTX(CFGID_ALIGNMENTX);
			CComBSTR bstrCFGID_ALIGNMENTY(CFGID_ALIGNMENTY);
			BSTR aIDs[2] = {bstrCFGID_ALIGNMENTX, bstrCFGID_ALIGNMENTY};
			TConfigValue aVals[2];
			aVals[0].eTypeID = aVals[1].eTypeID = ECVTFloat;
			aVals[0].fVal = (sel%3)-1;
			aVals[1].fVal = (sel/3)-1;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
		}
		return 0;
	}

private:
	CRectanglePosition m_wndAlign;
	CComboBox m_wndAlignCombo;
	TImageSize m_contentSize;
	TImageSize m_canvasSize;
};

