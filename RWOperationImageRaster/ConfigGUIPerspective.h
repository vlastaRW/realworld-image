
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <InPlaceCalc.h>

class CPerspectiveControl :
	public CWindowImpl<CPerspectiveControl>,
	public CThemeImpl<CPerspectiveControl>
{
public:
	CPerspectiveControl() : m_bNoBackBuffer(false), m_nDragIndex(-1),
		m_hCursorMove(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL))),
		m_hCursorStd(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)))
	{
		m_aCoords[0] = m_aCoords[1] = m_aCoords[3] = m_aCoords[6] = 0.0f;
		m_aCoords[2] = m_aCoords[4] = m_aCoords[5] = m_aCoords[7] = 1.0f;
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CPerspectiveControl()
	{
	}

	enum {
		CPC_POINT_MOVED = 1,
		CPC_DRAG_FINISHED = 2
	};


	DECLARE_WND_CLASS_EX(_T("PerspectiveControlWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CPerspectiveControl)
		CHAIN_MSG_MAP(CThemeImpl<CPerspectiveControl>)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	// operations
public:
	void SetPoints(float const* a_pPts)
	{
		for (int i = 0; i < 8; ++i)
			m_aCoords[i] = a_pPts[i];
		Invalidate(FALSE);
	}

	float const* GetPoints() const
	{
		return m_aCoords;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}

	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWnd)
		{
			DWORD dwPos = GetMessagePos();
			POINT tPt = {GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos)};
			ScreenToClient(&tPt);
			SetCursor(GetPtIndex(tPt.x, tPt.y) >= 0 ? m_hCursorMove : m_hCursorStd);
			return TRUE;
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
			SendNotification(CPC_DRAG_FINISHED);
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();
		m_nDragIndex = GetPtIndex(GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd && m_nDragIndex >= 0)
		{
			int nX = GET_X_LPARAM(a_lParam);
			int nY = GET_Y_LPARAM(a_lParam);
			RECT rcWin;
			GetClientRect(&rcWin);
			int nCenterX = rcWin.right>>1;
			int nCenterY = rcWin.bottom>>1;
			int nSize = (min(nCenterX, nCenterY))*2/3;

			m_aCoords[m_nDragIndex+m_nDragIndex] = (nX-nCenterX+nSize)/float(nSize+nSize);
			m_aCoords[m_nDragIndex+m_nDragIndex+1] = (nY-nCenterY+nSize)/float(nSize+nSize);
			SendNotification();
			Invalidate(FALSE);
		}
		return 0;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		PAINTSTRUCT ps;
		HDC hDC;
		HDC hDC2 = BeginPaint(&ps);
		HGDIOBJ hBmp;
		RECT rcWin;
		GetClientRect(&rcWin);
		if (m_bNoBackBuffer)
		{
			hDC = hDC2;
		}
		else
		{
			hDC = CreateCompatibleDC(hDC2);
			hBmp = SelectObject(hDC, CreateCompatibleBitmap(hDC2, rcWin.right, rcWin.bottom));
		}

		COLORREF clrPen = GetSysColor(COLOR_WINDOWTEXT);
		COLORREF clrBG = GetSysColor(COLOR_WINDOW);
		if (dwStyle & WS_DISABLED)
		{
			clrPen = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
					 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
					 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		}
		COLORREF clrDimmed = (((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
							 (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
							 (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));
		COLORREF clrFill = max(clrPen&0xff, clrBG&0xff) | //(((clrPen&0xfe)>>1)+((clrBG&0xfe)>>1)) |
						   (((clrPen&0xfe00)>>1)+((clrBG&0xfe00)>>1)) |
						   (((clrPen&0xfe0000)>>1)+((clrBG&0xfe0000)>>1));

		FillRect(hDC, &rcWin, reinterpret_cast<HBRUSH>(COLOR_WINDOW+1));
		int nCenterX = rcWin.right>>1;
		int nCenterY = rcWin.bottom>>1;
		int nSize = (min(nCenterX, nCenterY))*2/3;
		HGDIOBJ hSavePen = SelectObject(hDC, CreatePen(PS_DOT, 1, clrPen));
		HGDIOBJ hSaveBrush = SelectObject(hDC, CreateSolidBrush(clrFill));
		POINT p[5] =
		{
			{nCenterX-nSize, nCenterY-nSize},
			{nCenterX+nSize, nCenterY-nSize},
			{nCenterX+nSize, nCenterY+nSize},
			{nCenterX-nSize, nCenterY+nSize},
			{nCenterX-nSize, nCenterY-nSize},
		};
		Polyline(hDC, p, 5);
		POINT p2[5];
		for (int i = 0; i < 4; ++i)
		{
			p2[i].x = nCenterX-nSize + m_aCoords[i+i]*nSize*2 + 0.5f;
			p2[i].y = nCenterY-nSize + m_aCoords[i+i+1]*nSize*2 + 0.5f;
		};
		p2[4] = p2[0];
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, clrDimmed)));
		for (int i = 0; i < 4; ++ i)
		{
			MoveToEx(hDC, p[i].x, p[i].y, NULL);
			LineTo(hDC, p2[i].x, p2[i].y);
		}
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 3, clrPen)));
		Polyline(hDC, p2, 5);
		DeleteObject(SelectObject(hDC, CreatePen(PS_SOLID, 1, clrPen)));
		for (int i = 0; i < 4; ++ i)
		{
			Ellipse(hDC, p2[i].x-4, p2[i].y-4, p2[i].x+4, p2[i].y+4);
		}
		DeleteObject(SelectObject(hDC, hSavePen));
		DeleteObject(SelectObject(hDC, hSaveBrush));

		if (!m_bNoBackBuffer)
		{
			BitBlt(hDC2, 0, 0, rcWin.right, rcWin.bottom, hDC, 0, 0, SRCCOPY);
			DeleteObject(SelectObject(hDC, hBmp));
			DeleteDC(hDC);
		}
		EndPaint(&ps);

		return 0;
	}

private:
	void SendNotification(int a_nCode = CPC_POINT_MOVED)
	{
		HWND hPar = GetParent();
		if (hPar)
		{
			NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), a_nCode};
			SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
		}
	}
	int GetPtIndex(int a_nX, int a_nY)
	{
		RECT rcWin;
		GetClientRect(&rcWin);
		int nCenterX = rcWin.right>>1;
		int nCenterY = rcWin.bottom>>1;
		int nSize = (min(nCenterX, nCenterY))*2/3;
		for (int i = 0; i < 4; ++i)
		{
			int nX = nCenterX-nSize + m_aCoords[i+i]*nSize*2 + 0.5f;
			int nY = nCenterY-nSize + m_aCoords[i+i+1]*nSize*2 + 0.5f;
			if (((nX-a_nX)*(nX-a_nX) + (nY-a_nY)*(nY-a_nY)) <= 16)
				return i;
		}
		return -1;
	}

private:
	float m_aCoords[8];
	int m_nDragIndex;
	bool m_bNoBackBuffer;
	HCURSOR m_hCursorMove;
	HCURSOR m_hCursorStd;
};


class ATL_NO_VTABLE CConfigGUIPerspective :
	public CCustomConfigResourcelessWndImpl<CConfigGUIPerspective>,
	public CDialogResize<CConfigGUIPerspective>
{
public:
	CConfigGUIPerspective() : m_bUpdating(false)
	{
	}

	enum
	{
		IDC_MODE = 100,
		IDC_SCALE_LABEL,
		IDC_SCALE,
		IDC_ROTATION_LABEL,
		IDC_ROTATION,
		IDC_TRANSLATION_LABEL,
		IDC_TRANSLATION,
		IDC_CGPER_1LABEL,
		IDC_CGPER_1X,
		IDC_CGPER_1Y,
		IDC_CGPER_2LABEL,
		IDC_CGPER_2X,
		IDC_CGPER_2Y,
		IDC_CGPER_3LABEL,
		IDC_CGPER_3X,
		IDC_CGPER_3Y,
		IDC_CGPER_4LABEL,
		IDC_CGPER_4X,
		IDC_CGPER_4Y,
		IDC_CGPER_VISUAL,
	};

	BEGIN_DIALOG_EX(0, 0, 120, (M_Mode() == ECPMWithCanvas ? 76 : 130), 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Mode:[0405]Mód:"), IDC_STATIC, 0, 2, 65, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_MODE, 65, 0, 55, 100, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)

		int const off = 16;
	//if (M_Mode() != ECPMWithCanvas)
	{
		CONTROL_LTEXT(_T("[0409]Upper left corner:[0405]Levý horní roh:"), IDC_CGPER_1LABEL, 0, off+2, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGPER_1X, 65, off+0, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_EDITTEXT(IDC_CGPER_1Y, 93, off+0, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Upper right corner:[0405]Pravý horní roh:"), IDC_CGPER_2LABEL, 0, off+18, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGPER_2X, 65, off+16, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_EDITTEXT(IDC_CGPER_2Y, 93, off+16, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Lower right corner:[0405]Pravý spodní roh:"), IDC_CGPER_3LABEL, 0, off+34, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGPER_3X, 65, off+32, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_EDITTEXT(IDC_CGPER_3Y, 93, off+32, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Lower left corner:[0405]Levý spodní roh:"), IDC_CGPER_4LABEL, 0, off+50, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGPER_4X, 65, off+48, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_EDITTEXT(IDC_CGPER_4Y, 93, off+48, 26, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
	}
		CONTROL_LTEXT(_T("[0409]Scale:[0405]Škálování:"), IDC_SCALE_LABEL, 0, off+2, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_SCALE, 65, off+0, 55, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Rotation:[0405]Rotace:"), IDC_ROTATION_LABEL, 0, off+18, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ROTATION, 65, off+16, 55, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Move:[0405]Posun:"), IDC_TRANSLATION_LABEL, 0, off+34, 65, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_TRANSLATION, 65, off+32, 55, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)

		//CONTROL_CONTROL(_T(""), IDC_CGDS_SIZESPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 74, 0, 11, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Density [%]:[0405]Hustota [%]:"), IDC_STATIC, 0, 18, 66, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CGDS_DENSITYEDIT, 67, 16, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		//CONTROL_CONTROL(_T(""), IDC_CGDS_DENSITYSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 74, 16, 11, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Horizontal offset:[0405]Horizontální posun:"), IDC_STATIC, 0, 34, 66, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CGDS_OFFSETXEDIT, 67, 32, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		//CONTROL_CONTROL(_T(""), IDC_CGDS_OFFSETXSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 74, 32, 11, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Vertical offset:[0405]Vertikální posun:"), IDC_STATIC, 0, 50, 66, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CGDS_OFFSETYEDIT, 67, 48, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		//CONTROL_CONTROL(_T(""), IDC_CGDS_OFFSETYSPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 74, 48, 11, 12, 0)
		//CONTROL_LTEXT(_T("[0409]Color:[0405]Barva:"), IDC_STATIC, 0, 66, 66, 8, WS_VISIBLE, 0)
		//CONTROL_PUSHBUTTON(_T(""), IDC_CGDS_COLOR, 67, 64, 35, 12, WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("[0409]Standard drop shadow[0405]Stín podle pokrytí"), IDC_CGDS_STANDARD, 0, 80, 120, 12, WS_VISIBLE, 0)
		//CONTROL_RADIOBUTTON(_T("[0409]Edges-only shadow[0405]Stín pouze na hranách"), IDC_CGDS_OUTLINE, 0, 94, 120, 12, WS_VISIBLE, 0)
		//CONTROL_LTEXT(_T("[0409]Shadow position:[0405]Pozice stínu:"), IDC_STATIC, 0, 112, 66, 8, WS_VISIBLE, 0)
		//CONTROL_COMBOBOX(IDC_CGDS_BLENDING, 67, 110, 53/*75*/, 30, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		//CONTROL_LTEXT(_T("[0409]Mask ID:[0405]ID masky:"), IDC_CGDS_MASKIDLABEL, 0, 128, 66, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CGDS_MASKID, 67, 126, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIPerspective)
		COMMAND_HANDLER(IDC_CGPER_1X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_1Y, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_2X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_2Y, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_3X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_3Y, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_4X, EN_CHANGE, OnEditChange)
		COMMAND_HANDLER(IDC_CGPER_4Y, EN_CHANGE, OnEditChange)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIPerspective>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIPerspective>)
		NOTIFY_HANDLER(IDC_CGPER_VISUAL, CPerspectiveControl::CPC_POINT_MOVED, OnPointMoved)
		NOTIFY_HANDLER(IDC_CGPER_VISUAL, CPerspectiveControl::CPC_DRAG_FINISHED, OnDragFinished)
	END_MSG_MAP()

	const _AtlDlgResizeMap* GetDlgResizeMap()
	{
		static const _AtlDlgResizeMap theMap[] =
		{
		DLGRESIZE_CONTROL(IDC_CGPER_VISUAL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_MODE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGPER_1X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_1Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_2X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_2Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_3X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_3Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_4X, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGPER_4Y, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_SCALE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ROTATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRANSLATION, DLSZ_SIZE_X)
			{ -1, 0 },
		};
		return M_Mode() == ECPMWithCanvas ? theMap+1 : theMap;
	}

	SCustomConfigControlMap const* GetCustomConfigControlMap()
	{
		static SCustomConfigControlMap const sMap[] =
		{
		CONFIGITEM_CONTEXTHELP_IDS(IDC_CGPER_VISUAL, IDS_HELP_PERSPECTIVE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_1X, CFGID_PT1X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_1Y, CFGID_PT1Y)
		CONFIGITEM_VISIBILITY(IDC_CGPER_1LABEL, CFGID_PT1X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_2X, CFGID_PT2X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_2Y, CFGID_PT2Y)
		CONFIGITEM_VISIBILITY(IDC_CGPER_2LABEL, CFGID_PT2X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_3X, CFGID_PT3X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_3Y, CFGID_PT3Y)
		CONFIGITEM_VISIBILITY(IDC_CGPER_3LABEL, CFGID_PT3X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_4X, CFGID_PT4X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CGPER_4Y, CFGID_PT4Y)
		CONFIGITEM_VISIBILITY(IDC_CGPER_4LABEL, CFGID_PT4X)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_SCALE, CFGID_TRANSFORM_SCALE)
		CONFIGITEM_VISIBILITY(IDC_SCALE_LABEL, CFGID_TRANSFORM_SCALE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_ROTATION, CFGID_TRANSFORM_ANGLE)
		CONFIGITEM_VISIBILITY(IDC_ROTATION_LABEL, CFGID_TRANSFORM_ANGLE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_TRANSLATION, CFGID_TRANSFORM_TRANSLATION)
		CONFIGITEM_VISIBILITY(IDC_TRANSLATION_LABEL, CFGID_TRANSFORM_TRANSLATION)
		CONFIGITEM_COMBOBOX(IDC_MODE, CFGID_TRANSFORM_MODE)
		{ ECCCTInvalid, 0}
		};
		return M_Mode() == ECPMWithCanvas ? sMap+1 : sMap;
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (M_Mode() != ECPMWithCanvas)
		{
			RECT rcWnd = {0, 64+16, 120, 130};
			MapDialogRect(&rcWnd);
			m_wndPerspective.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_CGPER_VISUAL);
		}

		BOOL b;
		CCustomConfigResourcelessWndImpl<CConfigGUIPerspective>::OnInitDialog(0, 0, 0, b);

		OnEditChange(0, 0, NULL, b);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnPointMoved(int, LPNMHDR, BOOL&)
	{
		static UINT const aIDCs[8] =
		{
			IDC_CGPER_1X, IDC_CGPER_1Y, IDC_CGPER_2X, IDC_CGPER_2Y,
			IDC_CGPER_3X, IDC_CGPER_3Y, IDC_CGPER_4X, IDC_CGPER_4Y
		};
		float const* aVals = m_wndPerspective.GetPoints();

		m_bUpdating = true;
		for (int i = 0; i < 8; ++i)
		{
			TCHAR szText[32] = _T("");
			_stprintf(szText, _T("%g"), aVals[i]);
			GetDlgItem(aIDCs[i]).SetWindowText(szText);
		}
		m_bUpdating = false;
		return 0;
	}
	LRESULT OnDragFinished(int, LPNMHDR, BOOL&)
	{
		static UINT const aIDCs[8] =
		{
			IDC_CGPER_1X, IDC_CGPER_1Y, IDC_CGPER_2X, IDC_CGPER_2Y,
			IDC_CGPER_3X, IDC_CGPER_3Y, IDC_CGPER_4X, IDC_CGPER_4Y
		};
		BSTR aIDs[8];
		CComBSTR cCFGID_PT1X(CFGID_PT1X); aIDs[0] = cCFGID_PT1X;
		CComBSTR cCFGID_PT1Y(CFGID_PT1Y); aIDs[1] = cCFGID_PT1Y;
		CComBSTR cCFGID_PT2X(CFGID_PT2X); aIDs[2] = cCFGID_PT2X;
		CComBSTR cCFGID_PT2Y(CFGID_PT2Y); aIDs[3] = cCFGID_PT2Y;
		CComBSTR cCFGID_PT3X(CFGID_PT3X); aIDs[4] = cCFGID_PT3X;
		CComBSTR cCFGID_PT3Y(CFGID_PT3Y); aIDs[5] = cCFGID_PT3Y;
		CComBSTR cCFGID_PT4X(CFGID_PT4X); aIDs[6] = cCFGID_PT4X;
		CComBSTR cCFGID_PT4Y(CFGID_PT4Y); aIDs[7] = cCFGID_PT4Y;
		TConfigValue aVals[8];
		for (int i = 0; i < 8; ++i)
		{
			aVals[i].eTypeID = ECVTFloat;
			TCHAR szText[32] = _T("");
			GetDlgItem(aIDCs[i]).GetWindowText(szText, itemsof(szText));
			szText[itemsof(szText)-1] = _T('\0');
			LPTSTR p;
			aVals[i].fVal = _tcstod(szText, &p);
		}
		M_Config()->ItemValuesSet(8, aIDs, aVals);
		return 0;
	}
	LRESULT OnEditChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_wndPerspective.m_hWnd == NULL || m_bUpdating)
			return 0;

		static UINT const aIDCs[8] =
		{
			IDC_CGPER_1X, IDC_CGPER_1Y, IDC_CGPER_2X, IDC_CGPER_2Y,
			IDC_CGPER_3X, IDC_CGPER_3Y, IDC_CGPER_4X, IDC_CGPER_4Y
		};
		float aVals[8];

		for (int i = 0; i < 8; ++i)
		{
			TCHAR szText[32] = _T("");
			GetDlgItem(aIDCs[i]).GetWindowText(szText, itemsof(szText));
			szText[itemsof(szText)-1] = _T('\0');
			LPCTSTR p;
			aVals[i] = CInPlaceCalc::EvalExpression(szText, &p);
			if (*p && *p != _T(' '))
			{
				a_bHandled = FALSE;
				return 0;
			}
		}
		m_wndPerspective.SetPoints(aVals);
		a_bHandled = FALSE;
		return 0;
	}

private:
	CPerspectiveControl m_wndPerspective;
	bool m_bUpdating;
};

