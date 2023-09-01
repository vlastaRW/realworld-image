
#pragma once


#include <ConfigCustomGUIImpl.h>
#include <WTL_ColorPicker.h>
#include <WTL_FontCombo.h>
#include <WTL_2DPosition.h>
#include <WTL_ColorArea.h>
#include <WTL_Angle.h>
#include <RenderIcon.h>
#include <atlgdix.h>
#include <atlctl.h>
#include <atlcoll.h>
#include <atlstr.h>
#define _WTL_NO_CSTRING
#include <CustomTabCtrl.h>
#include <DotNetTabCtrl.h>
extern __declspec(selectany) TCHAR const COLORAREAAWNDCLASS[] = _T("WTL_ColorAreaA");


class ATL_NO_VTABLE CConfigGUIWatermark :
	public CCustomConfigResourcelessWndImpl<CConfigGUIWatermark>,
	public CDialogResize<CConfigGUIWatermark>,
	public CContextMenuWithIcons<CConfigGUIWatermark>
{
public:
	CConfigGUIWatermark() : m_wndColor(false), m_bInitializing(true)
	{
	}
	~CConfigGUIWatermark()
	{
		m_cImageList.Destroy();
	}
	enum
	{
		IDC_CG_POSX_EDIT = 300,
		IDC_CG_POSY_EDIT,
		IDC_CG_ANGLE_LABEL,
		IDC_CG_ANGLE_EDIT,
		IDC_ANGLE_AREA,
		IDC_CG_MARGINS,
		IDC_TYPETAB,
		IDC_CG_FONT,
		IDC_CG_COLOR,
		IDC_CG_PATH_EDIT,
		IDC_CG_PATH_BTN,
		IDC_IMGSIZETYPE,
		IDC_IMGRELSIZE,
		IDC_IMGRELSIZE_SPIN,
		IDC_IMGRELSIZE_UNIT,
		IDC_EFFECT_LABEL,
		IDC_EFFECT,
		IDC_TILED,
		IDC_CG_ABSSIZE_EDIT,
		IDC_CG_RELSIZE_EDIT,
		IDC_CG_TEXTTEMPLATE,
		IDC_ALIGNBOX,
		IDC_TOOLBAR,
		IDC_OPACITY_LABEL,
		IDC_OPACITY_AREA,
		ID_TEXTSIZE = 400,
		ID_SIZE_PERCENT,
		ID_SIZE_PIXELS,
		ID_BOLD,
		ID_ITALIC,
		ID_SHADOW,
		ID_ALIGN,
		ID_ALIGN_LEFT,
		ID_ALIGN_CENTER,
		ID_ALIGN_RIGHT,
	};

	BEGIN_DIALOG_EX(0, 0, 120, 140, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Position:[0405]Pozice:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_POSX_EDIT, 50, 0, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_EDITTEXT(IDC_CG_POSY_EDIT, 50, 16, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CHECKBOX(_T("[0409]Margins[0405]Okraje"), IDC_CG_MARGINS, 0, 17, 50, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Angle:[0405]Úhel:"), IDC_CG_ANGLE_LABEL, 0, 34, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_ANGLE_EDIT, 50, 32, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_LTEXT(_T("[0409]Opacity:[0405]Pokrytí:"), IDC_OPACITY_LABEL, 0, 50, 50, 8, WS_VISIBLE, 0)
		//CONTROL_LTEXT(_T("[0409]Margins:[0405]Okraje:"), IDC_STATIC, 0, 50, 50, 8, WS_VISIBLE, 0)
		//CONTROL_EDITTEXT(IDC_CG_MARGINS, 50, 48, 25, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_COMBOBOX(IDC_CG_FONT, 0, 78, 86, 100, CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_SORT | CBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("[0409]Color[0405]Barva"), IDC_CG_COLOR, 86, 78, 34, 12, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_ABSSIZE_EDIT, 0, 94, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_EDITTEXT(IDC_CG_RELSIZE_EDIT, 0, 94, 35, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_CONTROL(_T(""), IDC_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 35, 94, 85, 12, 0)
		CONTROL_EDITTEXT(IDC_CG_TEXTTEMPLATE, 0, 110, 120, 29, WS_VISIBLE | WS_TABSTOP | ES_MULTILINE | ES_AUTOHSCROLL | ES_WANTRETURN | WS_VSCROLL, 0)//ES_NUMBER
		CONTROL_EDITTEXT(IDC_CG_PATH_EDIT, 0, 78, 105, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_PUSHBUTTON(_T("[0000]..."), IDC_CG_PATH_BTN, 105, 78, 15, 12, WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Resize to[0405]Přeškálovat na"), IDC_IMGSIZETYPE, 0, 95, 60, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_EDITTEXT(IDC_IMGRELSIZE, 60, 94, 45, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)//ES_NUMBER
		CONTROL_CONTROL(_T(""), IDC_IMGRELSIZE_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 94, 94, 11, 12, 0)
		CONTROL_LTEXT(_T("%"), IDC_IMGRELSIZE_UNIT, 109, 96, 10, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Effect:[0405]Efekt:"), IDC_EFFECT_LABEL, 0, 112, 46, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_EFFECT, 50, 110, 70, 100, CBS_DROPDOWNLIST | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CHECKBOX(_T("[0409]Tiled watermark[0405]Dlaždicový vodotisk"), IDC_TILED, 0, 126, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIWatermark)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIWatermark>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIWatermark>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CG_COLOR, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		NOTIFY_HANDLER(IDC_ALIGNBOX, CRectanglePosition::C2DP_POSITION_CHANGED, OnPositionChanged)
		COMMAND_HANDLER(IDC_CG_FONT, CBN_SELCHANGE, OnFontChange)
		COMMAND_HANDLER(IDC_CG_PATH_BTN, BN_CLICKED, OnPathClicked)
		COMMAND_HANDLER(ID_BOLD, BN_CLICKED, OnClickedBold)
		COMMAND_HANDLER(ID_ITALIC, BN_CLICKED, OnClickedItalic)
		COMMAND_HANDLER(ID_SHADOW, BN_CLICKED, OnClickedShadow)
		//NOTIFY_HANDLER(IDC_CG_OPACITY_SPIN, UDN_DELTAPOS, OnUpDownChange)
		NOTIFY_HANDLER(IDC_TOOLBAR, TBN_DROPDOWN, OnDropdownClick)
		NOTIFY_HANDLER(IDC_OPACITY_AREA, CColorAreaHorzA::CAN_COLOR_CHANGED, OnAlphaChanged)
		NOTIFY_HANDLER(IDC_ANGLE_AREA, C2DRotation::C2DR_ANGLE_CHANGED, OnAngleChanged)
		NOTIFY_HANDLER(IDC_TYPETAB, CTCN_SELCHANGE, OnTcnSelchange)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CConfigGUIWatermark>)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIWatermark)
		CONFIGITEM_EDITBOX(IDC_CG_POSX_EDIT, CFGID_WM_XPOSITION)
		CONFIGITEM_EDITBOX(IDC_CG_POSY_EDIT, CFGID_WM_YPOSITION)
		CONFIGITEM_CHECKBOX_FLAG_VISIBILITY(IDC_CG_MARGINS, CFGID_WM_MARGINS, -1)
		//CONFIGITEM_EDITBOX(IDC_CG_MARGINS, CFGID_WM_MARGINS)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CG_PATH_EDIT, CFGID_WM_IMAGE)
		CONFIGITEM_VISIBILITY(IDC_CG_PATH_BTN, CFGID_WM_IMAGE)
		CONFIGITEM_CONTEXTHELP(IDC_OPACITY_AREA, CFGID_WM_OPACITY) 
		CONFIGITEM_CONTEXTHELP(IDC_CG_PATH_BTN, CFGID_WM_IMAGE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CG_TEXTTEMPLATE, CFGID_WM_TEXT)
		CONFIGITEM_CONTEXTHELP(IDC_CG_FONT, CFGID_WM_FONT)
		CONFIGITEM_CONTEXTHELP(IDC_CG_COLOR, CFGID_WM_COLOR)
		CONFIGITEM_VISIBILITY(IDC_TOOLBAR, CFGID_WM_TEXT)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CG_ANGLE_EDIT, CFGID_WM_ANGLE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CG_ABSSIZE_EDIT, CFGID_WM_ABSSIZE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_CG_RELSIZE_EDIT, CFGID_WM_RELSIZE)
		CONFIGITEM_CHECKBOX_FLAG_VISIBILITY(IDC_IMGSIZETYPE, CFGID_WM_IMGSIZETYPE, CFGID_WMIT_RELATIVE)
		CONFIGITEM_EDITBOX(IDC_IMGRELSIZE, CFGID_WM_IMGRELSIZE)
		CONFIGITEM_VISIBILITY(IDC_IMGRELSIZE, CFGID_WM_IMAGE)
		CONFIGITEM_VISIBILITY(IDC_IMGRELSIZE_UNIT, CFGID_WM_IMAGE)
		CONFIGITEM_VISIBILITY(IDC_IMGRELSIZE_SPIN, CFGID_WM_IMAGE)
		CONFIGITEM_VISIBILITY(IDC_EFFECT_LABEL, CFGID_WM_IMAGE)
		CONFIGITEM_COMBOBOX_VISIBILITY(IDC_EFFECT, CFGID_WM_FILTER)
		CONFIGITEM_CHECKBOX_VISIBILITY(IDC_TILED, CFGID_WM_TILED)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIWatermark)
		DLGRESIZE_CONTROL(IDC_CG_POSX_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_POSY_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ALIGNBOX, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ANGLE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ANGLE_AREA, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CG_MARGINS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TYPETAB, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_FONT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_COLOR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_PATH_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_PATH_BTN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ABSSIZE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_RELSIZE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_TEXTTEMPLATE, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_TOOLBAR, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_OPACITY_AREA, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_IMGRELSIZE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_IMGRELSIZE_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_IMGRELSIZE_UNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EFFECT, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	void ExtraConfigNotify()
	{
		CWindow wndPathEdit = GetDlgItem(IDC_CG_PATH_EDIT);
		CWindow wndPathBtn = GetDlgItem(IDC_CG_PATH_BTN);
		if (wndPathEdit.m_hWnd)
		{
			DWORD dwEdit = wndPathEdit.GetStyle()&WS_VISIBLE;
			DWORD dwBtn = wndPathBtn.GetStyle()&WS_VISIBLE;
			if (dwEdit != dwBtn)
			{
				wndPathBtn.ShowWindow(dwEdit == 0 ? SW_SHOW : SW_HIDE);
				m_wndOpacity.ShowWindow(dwEdit == 0 ? SW_SHOW : SW_HIDE);
			}
			if (dwEdit)
			{
				CConfigValue val;
				M_Config()->ItemValueGet(CComBSTR(CFGID_WM_OPACITY), &val);
				CColorBaseA cCol;
				cCol.SetA(val.operator LONG()*0.01f);
				m_wndOpacity.SetColor(cCol);
			}
		}
		if (m_wndColor.m_hWnd)
		{
			CComBSTR cCFGID_WM_COLOR(CFGID_WM_COLOR);
			CComPtr<IConfigItemSimple> pInfo;
			M_Config()->ItemGetUIInfo(cCFGID_WM_COLOR, __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pInfo));
			if (pInfo)
			{
				DWORD dwStyle = m_wndColor.GetStyle()&WS_VISIBLE;
				bool bEnabled = S_OK == pInfo->IsEnabled();
				if (bEnabled == (dwStyle == 0))
					m_wndColor.ShowWindow(bEnabled ? SW_SHOW : SW_HIDE);
			}
			CConfigValue cValColor;
			M_Config()->ItemValueGet(cCFGID_WM_COLOR, &cValColor);
			if (m_wndColor.GetColor() != CButtonColorPicker::SColor(cValColor.operator LONG()))
			{
				DWORD clr = cValColor.operator LONG();
				if ((clr&0xff000000) == 0)
					clr |= 0xff000000;
				m_wndColor.SetColor(CButtonColorPicker::SColor(clr, true));
			}
		}
		if (m_wndOpacity.m_hWnd)
		{
			CConfigValue cOpacity;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_OPACITY), &cOpacity);
			if (m_wndOpacity.GetA() != cOpacity.operator LONG());
			{
				CColorBaseA clr;
				clr.SetA(cOpacity.operator LONG()/100.0f);
				m_wndOpacity.SetColor(clr);
			}
		}
		if (m_wndFontCombo.m_hWnd)
		{
			CComBSTR cCFGID_WM_FONT(CFGID_WM_FONT);
			CComPtr<IConfigItemSimple> pInfo;
			M_Config()->ItemGetUIInfo(cCFGID_WM_FONT, __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pInfo));
			if (pInfo)
			{
				DWORD dwStyle = m_wndFontCombo.GetStyle()&WS_VISIBLE;
				bool bEnabled = S_OK == pInfo->IsEnabled();
				if (bEnabled == (dwStyle == 0))
					m_wndFontCombo.ShowWindow(bEnabled ? SW_SHOW : SW_HIDE);
			}
			CConfigValue cValFont;
			M_Config()->ItemValueGet(cCFGID_WM_FONT, &cValFont);
			int i = m_wndFontCombo.GetCurSel();
			CComBSTR bstr = NULL;
			int nCount = m_wndFontCombo.GetCount();
			if (i >= 0 || i < nCount)
				m_wndFontCombo.GetLBTextBSTR(i, bstr.m_str);
			if (bstr.m_str == NULL || wcscmp(bstr, cValFont))
			{
				for (i = 0; i < nCount; ++i)
				{
					bstr.Empty();
					m_wndFontCombo.GetLBTextBSTR(i, bstr.m_str);
					if (wcscmp(bstr, cValFont) == 0)
						m_wndFontCombo.SetCurSel(i);
				}
			}

		}

		if (m_wndAngle.m_hWnd)
		{
			CConfigValue a;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_ANGLE), &a);
			m_wndAngle.SetAngle(a.operator LONG());
		}

		if (m_wndTab.m_hWnd)
		{
			CConfigValue type;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_TYPE), &type);
			m_wndTab.SetCurSel(type.operator LONG());
		}

		if (m_wndAlignment.m_hWnd)
		{
			CConfigValue x;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_XPOSITION), &x);
			CConfigValue y;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_YPOSITION), &y);
			m_wndAlignment.SetPosition(x.operator LONG()*0.02f-1.0f, y.operator LONG()*0.02f-1.0f);
		}

		if (m_wndTools.m_hWnd &&  m_wndTools.GetStyle()&WS_VISIBLE)
		{
			CConfigValue bold;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_BOLD), &bold);
			m_wndTools.SetButtonInfo(ID_BOLD, TBIF_STATE, 0, bold ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, 0, 0, 0, 0, 0);
			CConfigValue italic;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_ITALIC), &italic);
			m_wndTools.SetButtonInfo(ID_ITALIC, TBIF_STATE, 0, italic ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, 0, 0, 0, 0, 0);
			CConfigValue shadow;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_SHADOW), &shadow);
			m_wndTools.SetButtonInfo(ID_SHADOW, TBIF_STATE, 0, shadow ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, 0, 0, 0, 0, 0);

			CConfigValue textSize;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_SIZETYPE), &textSize);
			CComBSTR bstr;
			if (textSize.operator LONG() == CFGID_WMH_ABSOLUTE)
				CMultiLanguageString::GetLocalized(L"[0409]px[0405]px", m_tLocaleID, &bstr);
			else
				bstr = L"%";
			TBBUTTONINFO tTBBI;
			ZeroMemory(&tTBBI, sizeof tTBBI);
			tTBBI.cbSize = sizeof tTBBI;
			tTBBI.dwMask = TBIF_SIZE;
			m_wndTools.GetButtonInfo(ID_TEXTSIZE, &tTBBI);
			tTBBI.dwMask = TBIF_TEXT;
			tTBBI.pszText = bstr;
			m_wndTools.SetButtonInfo(ID_TEXTSIZE, &tTBBI);
			tTBBI.dwMask = TBIF_SIZE;
			m_wndTools.SetButtonInfo(ID_TEXTSIZE, &tTBBI);

			CConfigValue text;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_TEXT), &text);
			int multiline = 0;
			for (wchar_t const* p = text; *p; ++p)
				if (*p == L'\r' || *p == L'\n')
				{
					multiline = 1;
				}
				else if (multiline == 1)
				{
					multiline = 2;
					break;
				}
			CConfigValue alignment;
			M_Config()->ItemValueGet(CComBSTR(CFGID_WM_ALIGNMENT), &alignment);
			tTBBI.dwMask = TBIF_IMAGE|TBIF_STATE;
			m_wndTools.GetButtonInfo(ID_ALIGN, &tTBBI);
			if (tTBBI.iImage != 3+alignment.operator LONG() || ((tTBBI.fsState&TBSTATE_ENABLED) == TBSTATE_ENABLED) != (multiline == 2))
			{
				tTBBI.iImage = 3+alignment.operator LONG();
				tTBBI.fsState = (tTBBI.fsState&~TBSTATE_ENABLED)|((multiline == 2)*TBSTATE_ENABLED);
				m_wndTools.SetButtonInfo(ID_ALIGN, &tTBBI);
			}
			CWindow wndText = GetDlgItem(IDC_CG_TEXTTEMPLATE);
			ATLASSERT(CFGID_WMA_LEFT == ES_LEFT && CFGID_WMA_CENTER == ES_CENTER && CFGID_WMA_RIGHT == ES_RIGHT);
			DWORD style = alignment.operator LONG();
			if (wndText.ModifyStyle((ES_LEFT|ES_CENTER|ES_RIGHT) - style, style))
				wndText.Invalidate(FALSE);
		}
	}

	void ExtraInitDialog()
	{
		RECT rcA = {80, 0, 120, 28};
		MapDialogRect(&rcA);
		LONG diff = (rcA.bottom-rcA.top)-(rcA.right-rcA.left);
		rcA.left -= diff;
		m_wndAlignment.Create(m_hWnd, &rcA, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, WS_EX_CLIENTEDGE, IDC_ALIGNBOX);
		m_wndAlignment.SetVisualizer(CRectangleVisualizer(0.25f, 0.25f));
        m_wndAlignment.SetWindowPos(GetDlgItem(IDC_CG_POSY_EDIT), 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

		RECT rcR = {80, 32, 120, 60};
		MapDialogRect(&rcR);
		rcR.left = rcA.left;
		rcR.right = rcA.right;
		m_wndAngle.Create(m_hWnd, &rcR, _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP, 0/*WS_EX_CLIENTEDGE*/, IDC_ANGLE_AREA);
        m_wndAngle.SetWindowPos(GetDlgItem(IDC_CG_ANGLE_EDIT), 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

		if (diff)
		{
			CWindow w = GetDlgItem(IDC_CG_POSX_EDIT);
			w.GetWindowRect(&rcA);
			ScreenToClient(&rcA);
			rcA.right -= diff;
			w.MoveWindow(&rcA);

			w = GetDlgItem(IDC_CG_POSY_EDIT);
			w.GetWindowRect(&rcA);
			ScreenToClient(&rcA);
			rcA.right -= diff;
			w.MoveWindow(&rcA);

			w = GetDlgItem(IDC_CG_ANGLE_EDIT);
			w.GetWindowRect(&rcA);
			ScreenToClient(&rcA);
			rcA.right -= diff;
			w.MoveWindow(&rcA);
		}

		RECT rcO = {50, 49, 75, 59};
		MapDialogRect(&rcO);
		rcO.right -= diff;
		m_wndOpacity.Create(m_hWnd, rcO, _T("Alpha"), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE, IDC_OPACITY_AREA);
        m_wndOpacity.SetWindowPos(GetDlgItem(IDC_OPACITY_LABEL), 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);

		int iconSize = XPGUI::GetSmallIconSize();
		m_cImageList.Create(iconSize, iconSize, XPGUI::GetImageListColorFlags(), 6, 1);
		{ // bold icon
			const int steps = 7;
			TPolyCoords aVtx[3+steps+steps+3+steps+3+steps];
			aVtx[0].fX = 0.1f;
			aVtx[0].fY = 0.0f;
			TPolyCoords* aVtx1 = aVtx+3+steps+steps;
			TPolyCoords* aVtx2 = aVtx1+3+steps;
			for (int i = 0; i < steps; ++i)
			{
				float a = 3.14159265f*2.0f*(0.25f-0.5f*i/(steps-1));
				float c = cosf(a);
				float s = -sinf(a);
				aVtx[1+i].fX = 0.5f+0.22f*c;
				aVtx[1+i].fY = 0.22f+0.22f*s;
				aVtx[1+steps+i].fX = 0.5f+0.28f*c;
				aVtx[1+steps+i].fY = 0.72f+0.28f*s;
				aVtx1[1+i].fX = 0.45f+0.07f*c;
				aVtx1[1+i].fY = 0.24f+0.07f*s;
				aVtx2[1+i].fX = 0.45f+0.13f*c;
				aVtx2[1+i].fY = 0.70f+0.13f*s;
			};
			aVtx[1+steps+steps].fX = 0.1f;
			aVtx[1+steps+steps].fY = 1.0f;
			aVtx[2+steps+steps] = aVtx[0];
			aVtx1[0].fX = 0.3f;
			aVtx1[0].fY = aVtx1[1].fY-0.001f;
			aVtx1[1+steps].fX = 0.3f;
			aVtx1[1+steps].fY = aVtx1[steps].fY+0.001f;
			aVtx1[2+steps] = aVtx1[0];
			aVtx2[0].fX = 0.3f;
			aVtx2[0].fY = aVtx2[1].fY-0.001f;
			aVtx2[1+steps].fX = 0.3f;
			aVtx2[1+steps].fY = aVtx2[steps].fY+0.001f;
			aVtx2[2+steps] = aVtx2[0];
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // italic icon
			static TPolyCoords const aVtx[] =
			{
				{0.39f, 0.0f}, {0.74f, 0.0f}, {0.7f, 0.15f}, {0.6f, 0.15f}, {0.4f, 0.85f}, {0.5f, 0.85f},
				{0.46f, 1.0f}, {0.11f, 1.0f}, {0.15f, 0.85f}, {0.25f, 0.85f}, {0.45f, 0.15f}, {0.35f, 0.15f},
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // shadow icon
			const int steps = 13;
			TPolyCoords aVtx[steps*2];
			for (int i = 0; i < steps; ++i)
			{
				float a1 = 3.14159265f*2.0f*(0.5f+(i-((steps-1)*0.5f))/steps*0.85f);
				float a2 = 3.14159265f*2.0f*(0.5f-(i-((steps-1)*0.5f))/steps*0.75f);
				aVtx[i].fX = 0.5f+0.5f*cosf(a1);
				aVtx[i].fY = 0.5f+0.5f*sinf(a1);
				aVtx[steps+i].fX = 0.65f+0.35f*cosf(a2);
				aVtx[steps+i].fY = 0.5f+0.35f*sinf(a2);
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // align left icon
			static TPolyCoords const aVtx[] =
			{
				{0.0f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.4f}, {0.4f, 0.4f}, {0.4f, 0.6f}, {0.65f, 0.6f}, {0.65f, 0.8f}, {0.0f, 0.8f},
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // align center icon
			static TPolyCoords const aVtx[] =
			{
				{0.9f, 0.2f}, {0.9f, 0.4f}, {0.7f, 0.4f}, {0.7f, 0.6f}, {0.775f, 0.6f}, {0.775f, 0.8f},
				{0.225f, 0.8f}, {0.225f, 0.6f}, {0.3f, 0.6f}, {0.3f, 0.4f}, {0.1f, 0.4f}, {0.1f, 0.2f}, 
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // align right icon
			static TPolyCoords const aVtx[] =
			{
				{1.0f, 0.2f}, {0.2f, 0.2f}, {0.2f, 0.4f}, {0.6f, 0.4f}, {0.6f, 0.6f}, {0.35f, 0.6f}, {0.35f, 0.8f}, {1.0f, 0.8f},
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // text icon
			static TPolyCoords const aVtx[] =
			{
				{0.1f, 0.9f}, {0.4f, 0.0f}, {0.6f, 0.0f}, {0.9f, 0.9f}, {0.7f, 0.9f}, {0.6f, 0.6f}, {0.4f, 0.6f}, {0.3f, 0.9f}, {0.1f, 0.9f},
				{0.5f, 0.25f}, {0.55f, 0.4f}, {0.45f, 0.4f},
			};
			HICON h = IconFromPolygon(itemsof(aVtx), aVtx, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}
		{ // image icon
			static TPolyCoords const aVtx1[] =
			{
				{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.8f}, {0.0f, 0.8f},
			};
			static TPolyCoords const c = {0.25f, 0.25f};
			static float const r = 0.1f;
			static float const d = r*0.707f;
			static TPolyCoords const aVtx2[] =
			{
				{0.15f, 0.7f}, {0.35f, 0.4f}, {0.45f, 0.55f}, {0.6f, 0.25f}, {0.85f, 0.7f}, {0.15f, 0.7f},
				{c.fX+r, c.fY}, {c.fX+d, c.fY+d}, {c.fX, c.fY+r}, {c.fX-d, c.fY+d},
				{c.fX-r, c.fY}, {c.fX+d, c.fY-d}, {c.fX, c.fY-r}, {c.fX-d, c.fY-d},
			};
			HICON h = IconFromPolygon(itemsof(aVtx1), aVtx1, itemsof(aVtx2), aVtx2, iconSize, false);
			m_cImageList.AddIcon(h);
			DestroyIcon(h);
		}

		RECT rcT = {0, 63, 120, 75};
		MapDialogRect(&rcT);
		m_wndTab.Create(m_hWnd, rcT, _T("Tab"), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|CTCS_BOLDSELECTEDTAB|CTCS_NOEDGE, 0, IDC_TYPETAB);
        m_wndTab.SetWindowPos(m_wndOpacity, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
		m_wndTab.SetImageList(m_cImageList);
		CComBSTR bstrText;
		CMultiLanguageString::GetLocalized(L"[0409]Text[0405]Text", m_tLocaleID, &bstrText);
		m_wndTab.InsertItem(0, bstrText, 6);
		CComBSTR bstrImage;
		CMultiLanguageString::GetLocalized(L"[0409]Image[0405]Obrázek", m_tLocaleID, &bstrImage);
		m_wndTab.InsertItem(1, bstrImage, 7);
		m_bInitializing = false;

		m_wndTools = GetDlgItem(IDC_TOOLBAR);
		m_wndTools.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndTools.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndTools.SetImageList(m_cImageList);

		CComBSTR bstrPx;
		CMultiLanguageString::GetLocalized(L"[0409]px[0405]px", m_tLocaleID, &bstrPx);
		CComBSTR bstrBold;
		CMultiLanguageString::GetLocalized(L"[0409]Bold[0405]Tučné", m_tLocaleID, &bstrBold);
		CComBSTR bstrItalic;
		CMultiLanguageString::GetLocalized(L"[0409]Italic[0405]Kurzíva", m_tLocaleID, &bstrItalic);
		CComBSTR bstrShadow;
		CMultiLanguageString::GetLocalized(L"[0409]Shadow[0405]Stín", m_tLocaleID, &bstrShadow);
		CComBSTR bstrAlignment;
		CMultiLanguageString::GetLocalized(L"[0409]Paragraph alignment[0405]Zarovnání odstavce", m_tLocaleID, &bstrAlignment);
		TBBUTTON atButtons[] =
		{
			{I_IMAGENONE, ID_TEXTSIZE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN|BTNS_SHOWTEXT, TBBUTTON_PADDING, 0, INT_PTR(bstrPx.m_str)},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{0, ID_BOLD, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, INT_PTR(bstrBold.m_str)},
			{1, ID_ITALIC, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, INT_PTR(bstrItalic.m_str)},
			{2, ID_SHADOW, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, INT_PTR(bstrShadow.m_str)},
			{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{3, ID_ALIGN, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, INT_PTR(bstrAlignment.m_str)},
		};
		m_wndTools.AddButtons(itemsof(atButtons), atButtons);
		RECT rcWnd;
		m_wndTools.GetWindowRect(&rcWnd);
		SIZE sz = {0, 0};
		m_wndTools.GetButtonSize(sz);
		sz.cy = rcWnd.bottom-rcWnd.top;
		m_wndTools.SetButtonSize(sz);
		RECT rcTB;
		m_wndTools.GetItemRect(itemsof(atButtons)-1, &rcTB);
		rcTB.left = rcWnd.right-rcTB.right;
		rcTB.top = rcWnd.top;
		rcTB.right = rcWnd.right;
		LONG const nDYB = rcTB.bottom-(rcWnd.bottom-rcWnd.top);
		rcTB.bottom = rcWnd.top+rcTB.bottom;
		ScreenToClient(&rcTB);
		m_wndTools.MoveWindow(&rcTB);

		CWindow wndSize = GetDlgItem(IDC_CG_ABSSIZE_EDIT);
		wndSize.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		rcWnd.right = rcTB.left-((rcTB.bottom-rcTB.top)/5);
		wndSize.MoveWindow(&rcWnd);
		GetDlgItem(IDC_CG_RELSIZE_EDIT).MoveWindow(&rcWnd);

		// initialize color button
		m_wndColor.m_tLocaleID = m_tLocaleID;
		m_wndColor.SubclassWindow(GetDlgItem(IDC_CG_COLOR));
		m_wndColor.SetDefaultText(NULL);

		// int font combo
		HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfFaceName[0] = _T('\0');
		lf.lfPitchAndFamily = 0;
		std::vector<std::tstring> cNames;
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)MyEnumFontProc, (LPARAM)&cNames, 0);
		DeleteDC(hDC);
		m_wndFontCombo.SubclassWindow(cNames.begin(), cNames.end(), GetDlgItem(IDC_CG_FONT));
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	//LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	//{
	//	LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

	//	LONG nVal = GetDlgItemInt(IDC_CG_OPACITY);
	//	if (pNMUD->iDelta > 0)
	//	{
	//		if (nVal <= 0)
	//			return 0;
	//		nVal = max(0, nVal-10);
	//	}
	//	else
	//	{
	//		if (nVal >= 100)
	//			return 0;
	//		nVal = min(100, nVal+10);
	//	}
	//	CComBSTR cCFGID_WM_OPACITY(CFGID_WM_OPACITY);
	//	M_Config()->ItemValuesSet(1, &(cCFGID_WM_OPACITY.m_str), CConfigValue(nVal));

	//	return 0;
	//}

	static int CALLBACK MyEnumFontProc(const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
	{
		if (FontType == TRUETYPE_FONTTYPE)
		{
			std::vector<std::tstring>* pFnts = reinterpret_cast<std::vector<std::tstring>*>(lParam);
			for (std::vector<std::tstring>::const_iterator i = pFnts->begin(); i != pFnts->end(); ++i)
				if (_tcscmp(i->c_str(), (TCHAR*)lpelfe->elfFullName) == 0)
					return 1;
			pFnts->push_back((TCHAR*)lpelfe->elfFullName);
		}
		return 1;
	}

	LRESULT OnFontChange(WORD /*a_wNotifyCode*/, WORD /*a_wID*/, HWND /*a_hWndCtl*/, BOOL& /*a_bHandled*/)
	{
		try
		{
			int i = m_wndFontCombo.GetCurSel();
			if (i < 0 || i > m_wndFontCombo.GetCount())
				return 0;
			CComBSTR bstr = NULL;
			m_wndFontCombo.GetLBTextBSTR(i, bstr.m_str);
			CComBSTR cCFGID_WM_FONT(CFGID_WM_FONT);
			CConfigValue cValFont(bstr);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_WM_FONT;
			TConfigValue aVals[1];
			aVals[0] = cValFont;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}
		return 0;
	}

	void ToggleFlag(wchar_t const* id)
	{
		CComBSTR bstrID(id);
		CConfigValue val;
		M_Config()->ItemValueGet(bstrID, &val);
		val = !val;
		M_Config()->ItemValuesSet(1, &(bstrID.m_str), val);
	}
	LRESULT OnClickedBold(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ToggleFlag(CFGID_WM_BOLD);
		return 0;
	}
	LRESULT OnClickedItalic(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ToggleFlag(CFGID_WM_ITALIC);
		return 0;
	}
	LRESULT OnClickedShadow(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		ToggleFlag(CFGID_WM_SHADOW);
		return 0;
	}
	LRESULT OnDropdownClick(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
	{
		NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
		CMenu cMenu;

		CComBSTR bstrID;
		if (pTB->iItem == ID_TEXTSIZE)
		{
			Reset(m_cImageList);
			cMenu.CreatePopupMenu();
			bstrID = CFGID_WM_SIZETYPE;
			CConfigValue val;
			M_Config()->ItemValueGet(bstrID, &val);

			CComBSTR bstrPix;
			CMultiLanguageString::GetLocalized(L"[0409]pixels[0405]pixely", m_tLocaleID, &bstrPix);
			CComBSTR bstrPerc;
			CMultiLanguageString::GetLocalized(L"[0409]percent[0405]procenta", m_tLocaleID, &bstrPerc);
			AddItem(cMenu, CFGID_WMH_ABSOLUTE+1, bstrPix, I_IMAGENONE, val.operator LONG() == CFGID_WMH_ABSOLUTE ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
			AddItem(cMenu, CFGID_WMH_RELATIVE+1, bstrPerc, I_IMAGENONE, val.operator LONG() == CFGID_WMH_RELATIVE ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
		}
		else if (pTB->iItem == ID_ALIGN)
		{
			Reset(m_cImageList);
			cMenu.CreatePopupMenu();
			bstrID = CFGID_WM_ALIGNMENT;
			CConfigValue val;
			M_Config()->ItemValueGet(bstrID, &val);
			CComPtr<IConfigItemOptions> pItem;
			M_Config()->ItemGetUIInfo(bstrID, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
			CComPtr<IEnumConfigItemOptions> pOptions;
			ULONG nOptions = 0;
			if (pItem == NULL || FAILED(pItem->OptionsEnum(&pOptions)) || pOptions == NULL || FAILED(pOptions->Size(&nOptions)) || nOptions == 0)
				return TBDDRET_DEFAULT;
			for (ULONG i = 0; i < nOptions; ++i)
			{
				CConfigValue opt;
				pOptions->Get(i, &opt);
				CComBSTR bstrText;
				CComPtr<ILocalizedString> pStr;
				pItem->ValueGetName(opt, &pStr);
				if (pStr) pStr->GetLocalized(m_tLocaleID, &bstrText);
				if (bstrText == NULL) bstrText = L"err";
				AddItem(cMenu, opt.operator LONG()+1, bstrText, 3+opt.operator LONG(), val == opt ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
			}
		}
		else
			return TBDDRET_DEFAULT;

		POINT ptBtn = {pTB->rcButton.left, pTB->rcButton.bottom};
		m_wndTools.ClientToScreen(&ptBtn);

		TPMPARAMS tPMParams;
		ZeroMemory(&tPMParams, sizeof tPMParams);
		tPMParams.cbSize = sizeof tPMParams;
		tPMParams.rcExclude = pTB->rcButton;
		::MapWindowPoints(pTB->hdr.hwndFrom, NULL, reinterpret_cast<POINT*>(&tPMParams.rcExclude), 2);
		UINT nSelection = cMenu.TrackPopupMenuEx(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_NONOTIFY|TPM_RETURNCMD, ptBtn.x, ptBtn.y, m_hWnd, &tPMParams);
		if (nSelection == 0)
			return TBDDRET_DEFAULT;

		M_Config()->ItemValuesSet(1, &(bstrID.m_str), CConfigValue(LONG(nSelection-1)));

		return TBDDRET_DEFAULT;
	}

	LRESULT OnPathClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		try
		{
			CComBSTR bstrPrev;
			GetDlgItemText(IDC_CG_PATH_EDIT, bstrPrev.m_str);

			CComPtr<IStorageManager> pStMgr;
			RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
			static const GUID tCtxID = {0x20f6399a, 0x63b4, 0x40ce, {0x9b, 0xa1, 0x8e, 0xd, 0x53, 0x89, 0x81, 0x33}};
			CComPtr<IInputManager> pInMgr;
			RWCoCreateInstance(pInMgr, __uuidof(InputManager));
			CComPtr<IEnumUnknowns> pBuilders;
			pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
			CComPtr<IEnumUnknowns> pFilters;
			pInMgr->DocumentTypesEnumEx(pBuilders, &pFilters);

			CComPtr<IStorageFilter> pNewLocation;
			pStMgr->FilterCreateInteractivelyUID(bstrPrev, EFTOpenExisting|EFTHintNoStream, m_hWnd, pFilters, NULL, tCtxID, _SharedStringTable.GetStringAuto(IDS_SELECTIMAGEWATERMARK), NULL, m_tLocaleID, &pNewLocation);
			CComBSTR bstrNewVal;
			if (pNewLocation && SUCCEEDED(pNewLocation->ToText(NULL, &bstrNewVal)) && bstrNewVal && (bstrPrev == NULL || wcscmp(bstrNewVal, bstrPrev)))
			{
				CComBSTR cCFGID_WM_IMAGE(CFGID_WM_IMAGE);
				CConfigValue cValFont(bstrNewVal);
				BSTR aIDs[1];
				aIDs[0] = cCFGID_WM_IMAGE;
				TConfigValue aVals[1];
				aVals[0] = cValFont;
				M_Config()->ItemValuesSet(1, aIDs, aVals);
			}
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
			CComBSTR cCFGID_COLOR(CFGID_WM_COLOR);
			CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToRGBA()));
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
	LRESULT OnAlphaChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR id(CFGID_WM_OPACITY);
			M_Config()->ItemValuesSet(1, &(id.m_str), CConfigValue(LONG(m_wndOpacity.GetColor().GetA()*100.0f+0.5f)));
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnAngleChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR id(CFGID_WM_ANGLE);
			LONG l = floorf(m_wndAngle.GetAngle()+0.5f);
			if (l > 180) l -= 360;
			M_Config()->ItemValuesSet(1, &(id.m_str), CConfigValue(l));
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnPositionChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR cCFGID_X(CFGID_WM_XPOSITION);
			CComBSTR cCFGID_Y(CFGID_WM_YPOSITION);
			CConfigValue cValX(static_cast<LONG>(m_wndAlignment.GetPositionX()*50.0f+50.5f));
			CConfigValue cValY(static_cast<LONG>(m_wndAlignment.GetPositionY()*50.0f+50.5f));
			BSTR aIDs[2];
			aIDs[0] = cCFGID_X;
			aIDs[1] = cCFGID_Y;
			TConfigValue aVals[2];
			aVals[0] = cValX;
			aVals[1] = cValY;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	LRESULT OnTcnSelchange(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/)
	{
		if (m_bInitializing)
			return 0;

		CComBSTR id(CFGID_WM_TYPE);
		M_Config()->ItemValuesSet(1, &(id.m_str), CConfigValue(LONG(m_wndTab.GetCurSel())));

		return 0;
	}

private:
	template<class TColorBase>
	struct CColorInterpreterA
	{
		static void XY2Color(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			if (a_fX < 0.0f) a_fX = 0.0f; else if (a_fX > 1.0f) a_fX = 1.0f;
			a_tColor.SetA(a_fX);
		}
		static void Color2XY(TColorBase const& a_tColor, float& a_fX, float& a_fY)
		{
			a_fX = a_tColor.GetA();
			if (a_fX <= 0.0f) a_fX = 0.0f; else if (a_fX >= 1.0f) a_fX = 1.0f;
			a_fY = 0.5f;
		}

		static void InitColorArea(DWORD* a_pDst, int a_nSizeX, int a_nSizeY, float a_fStepX, float a_fStepY, TColorBase const& a_tCurrent)
		{
			DWORD const clr1 = GetSysColor(COLOR_3DLIGHT);
			DWORD const clr2 = GetSysColor(COLOR_3DSHADOW);
			float const aClr[8] =
			{
				powf(GetBValue(clr1)/255.0f, 2.2f), powf(GetGValue(clr1)/255.0f, 2.2f), powf(GetRValue(clr1)/255.0f, 2.2f), 0,
				powf(GetBValue(clr2)/255.0f, 2.2f), powf(GetGValue(clr2)/255.0f, 2.2f), powf(GetRValue(clr2)/255.0f, 2.2f), 0,
			};

			int a_nSquare = 4;

			float aBase[3] = {0.5f, 0.5f, 0.5f};

			for (; a_nSizeY > 0; --a_nSizeY)
			{
				float x = 0.0f;
				for (int nSizeX = a_nSizeX; nSizeX > 0; --nSizeX, x+=a_fStepX)
				{
					float nx = 1.0f-x;
					float const* p = aClr + ((((nSizeX/a_nSquare)^(a_nSizeY/a_nSquare))&1)<<2);
					*(a_pDst++) = RGB(powf((aBase[0]*x+p[0]*nx), 1.0f/2.2f)*255.0f+0.5f, powf((aBase[1]*x+p[1]*nx), 1.0f/2.2f)*255.0f+0.5f, powf((aBase[2]*x+p[2]*nx), 1.0f/2.2f)*255.0f+0.5f);
				}
			}
		}

		static void XY2ColorInternal(float a_fX, float a_fY, TColorBase& a_tColor)
		{
			XY2Color(a_fX, a_fY, a_tColor);
		}

		static bool InvalidateArea(TColorBase const& a1, TColorBase const& a2)
		{
			return false;
		}
		static bool InvalidateThumb(TColorBase const& a1, TColorBase const& a2)
		{
			if (a1.GetA() != a2.GetA())
				return true;
			return false;
		}
	};

	class CColorBaseA
	{
	public:
		CColorBaseA() : m_fAlpha(1.0f)
		{
		}

		void SetA(float a_fVal) { m_fAlpha = a_fVal; }
		float GetA() const { return m_fAlpha; }
		DWORD GetRGB() const { return RGB(0xbb, 0xbb, 0xbb); }

	private:
		float m_fAlpha;
	};
	typedef CColorArea<CColorInterpreterA, CColorBaseA, COLORAREAAWNDCLASS> CColorAreaHorzA;

	template <class TItem = CCustomTabItem>
	class CSimpleTabCtrl : public CDotNetTabCtrlImpl<CSimpleTabCtrl<TItem>, TItem>
	{
	public:
		DECLARE_WND_CLASS_EX(_T("WTL_3DFaceDotNetTabCtrl"), CS_DBLCLKS, COLOR_3DFACE)

		void InitializeDrawStruct(LPNMCTCCUSTOMDRAW lpNMCustomDraw)
		{
			CDotNetTabCtrlImpl<CSimpleTabCtrl, TItem>::InitializeDrawStruct(lpNMCustomDraw);
			lpNMCustomDraw->clrBtnHighlight = ::GetSysColor(COLOR_3DSHADOW);
			lpNMCustomDraw->hBrushBackground = reinterpret_cast<HBRUSH>(GetParent().SendMessage(WM_CTLCOLORDLG));
		}
	};

private:
	CToolBarCtrl m_wndTools;
	CImageList m_cImageList;
	CButtonColorPicker m_wndColor;
	CFontComboBox m_wndFontCombo;
	CRectanglePosition m_wndAlignment;
	CColorAreaHorzA m_wndOpacity;
	C2DRotation m_wndAngle;
	CSimpleTabCtrl<> m_wndTab;
	bool m_bInitializing;
};
