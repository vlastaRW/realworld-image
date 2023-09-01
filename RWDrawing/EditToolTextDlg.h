// EditToolTextDlg.h : Declaration of the CEditToolTextDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <WTL_FontCombo.h>
#include <XPGUI.h>


struct CEditToolDataText
{
	MIDL_INTERFACE("24FCB5FE-B463-425B-A95A-5D668434D3C7")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataText const*, InternalData)() = 0;
	};
	enum ETextAlign
	{
		ETALeft = 0,
		ETACenter,
		ETARight,
		ETAJustify,
		ETACOUNT
	};

	CEditToolDataText() : fSize(12.0f), fAngle(0.0f), fBend(0.0f), bBold(false), bItalic(false), eAlign(ETALeft), dwTextID(0)
	{
		wcsncpy(lfFaceName, L"Tahoma", itemsof(lfFaceName));
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		LPCOLESTR p = wcschr(a_bstr, L'\n');
		if (p == NULL || p-a_bstr > LF_FACESIZE)
			return S_FALSE;
		wcsncpy(lfFaceName, a_bstr, p-a_bstr);
		lfFaceName[p-a_bstr] = L'\0';
		LPOLESTR p2 = const_cast<LPOLESTR>(p)+1;
		double d = wcstod(p+1, &p2);
		if (p2 == p+1)
			return S_FALSE;
		fSize = d;
		p = wcschr(p2, L'\n');
		if (p == NULL)
			return S_FALSE;
		bItalic = false;
		bBold = false;
		eAlign = ETALeft;
		if (*p2 == L'|')
		{
			++p2;
			while (p2 < p && *p2 != L'|')
			{
				if (*p2 == L'I')
					bItalic = true;
				else if (*p2 == L'B')
					bBold = true;
				else if (*p2 == L'L')
					eAlign = ETALeft;
				else if (*p2 == L'R')
					eAlign = ETARight;
				else if (*p2 == L'C')
					eAlign = ETACenter;
				else if (*p2 == L'J')
					eAlign = ETAJustify;
				++p2;
			}
		}
		fAngle = 0.0f;
		fBend = 0.0f;
		if (*p2 == L'|')
		{
			LPOLESTR p3;
			double d = wcstod(p2+1, &p3);
			if (p3 > p2+1)
				fAngle = d;
			p2 = p3;
			if (*p2 == L'|')
			{
				p3 = NULL;
				double d = wcstod(p2+1, &p3);
				if (p3 > p2+1)
					fBend = d;
			}
		}
		strText = p+1;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = NULL;
		wchar_t szTmp[LF_FACESIZE+64];
		swprintf(szTmp, L"%s\n%g|%c%s|%g|%g\n", lfFaceName, fSize, eAlign == ETALeft ? L'L' : (eAlign == ETACenter ? L'C' : (eAlign == ETARight ? L'R' : L'J')), bBold ? (bItalic ? L"BI" : L"B") : (bItalic ? L"I" : L""), fAngle, fBend);
		int n = wcslen(szTmp);
		BSTR bstr = SysAllocStringLen(NULL, n+strText.length());
		wcscpy(bstr, szTmp);
		wcscpy(bstr+n, strText.c_str());
		*a_pbstr = bstr;
		return S_OK;
	}

	WCHAR lfFaceName[LF_FACESIZE+1];
	float fSize;
	float fAngle;
	float fBend;
	bool bBold;
	bool bItalic;
	ETextAlign eAlign;
	std::wstring strText;
	DWORD dwTextID;
};


// CEditToolTextDlg

class CEditToolTextDlg : 
	public CEditToolDlgBase<CEditToolTextDlg, CEditToolDataText>,
	public CDialogResize<CEditToolTextDlg>,
	public CContextHelpDlg<CEditToolTextDlg>
{
public:
	CEditToolTextDlg()
	{
	}
	~CEditToolTextDlg()
	{
		m_cImageList.Destroy();
	}

	enum { IDD = IDD_EDITTOOL_TEXT, ID_BOLD = 333, ID_ITALIC, ID_ALIGN_LEFT, ID_ALIGN_CENTER, ID_ALIGN_RIGHT };

	BEGIN_MSG_MAP(CEditToolTextDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolTextDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		//COMMAND_HANDLER(IDC_ET_FONT, BN_CLICKED, OnClickedFont)
		COMMAND_HANDLER(IDC_ET_TEXT, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_SIZE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_ANGLE, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_BEND, EN_CHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_FONT, CBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(ID_BOLD, BN_CLICKED, OnChangeStyle)
		COMMAND_HANDLER(ID_ITALIC, BN_CLICKED, OnChangeStyle)
		COMMAND_HANDLER(ID_ALIGN_LEFT, BN_CLICKED, OnChangeAlign)
		COMMAND_HANDLER(ID_ALIGN_CENTER, BN_CLICKED, OnChangeAlign)
		COMMAND_HANDLER(ID_ALIGN_RIGHT, BN_CLICKED, OnChangeAlign)
		CHAIN_MSG_MAP(CDialogResize<CEditToolTextDlg>)
		NOTIFY_HANDLER(IDC_ET_SIZESPIN, UDN_DELTAPOS, OnUpDownChange)
		NOTIFY_HANDLER(IDC_ET_ANGLESPIN, UDN_DELTAPOS, OnAngleUpDownChange)
		NOTIFY_HANDLER(IDC_ET_BENDSPIN, UDN_DELTAPOS, OnBendUpDownChange)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolTextDlg)
		DLGRESIZE_CONTROL(IDC_ET_FONT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_FONT_STYLE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_ALIGN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_ANGLE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_ANGLESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_DEGREES, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_BEND, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_BENDSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_BENDUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_TEXT, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolTextDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_TEXT, IDS_HELP_TEXT)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SIZE, IDS_HELP_FONT)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_ANGLE, IDS_HELP_FONT)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FONT, IDS_HELP_FONT)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolTextDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* UNREF(a_pState))
	{
		return S_OK;
	}

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataText::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				DataToGUI();
			}
		}
	}

	static int CALLBACK MyEnumFontProc(const ENUMLOGFONTEX *lpelfe, const NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
	{
		if (FontType != RASTER_FONTTYPE)
		{
			TCHAR szName[LF_FACESIZE+1];
			_tcsncpy(szName, lpelfe->elfLogFont.lfFaceName, LF_FACESIZE); // (TCHAR*)lpelfe->elfFullName
			szName[LF_FACESIZE] = _T('\0');
			std::vector<std::tstring>* pFnts = reinterpret_cast<std::vector<std::tstring>*>(lParam);
			for (std::vector<std::tstring>::const_iterator i = pFnts->begin(); i != pFnts->end(); ++i)
				if (_tcscmp(i->c_str(), szName) == 0)
					return 1;
			pFnts->push_back(szName);
		}
		return 1;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		// init font combo
		HDC hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		LOGFONT lf;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfFaceName[0] = _T('\0');
		lf.lfPitchAndFamily = 0;
		std::vector<std::tstring> cNames;
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)MyEnumFontProc, (LPARAM)&cNames, 0);
		DeleteDC(hDC);
		m_wndFontCombo.SubclassWindow(cNames.begin(), cNames.end(), GetDlgItem(IDC_ET_FONT));
		//m_wndFontCombo.SetFont(GetFont());

		m_wndText = GetDlgItem(IDC_ET_TEXT);
		AddWindowForPreTranslate(m_wndText);
		m_wndSize = GetDlgItem(IDC_ET_SIZE);
		AddWindowForPreTranslate(m_wndSize);
		m_wndAngle = GetDlgItem(IDC_ET_ANGLE);
		AddWindowForPreTranslate(m_wndAngle);
		m_wndBend = GetDlgItem(IDC_ET_BEND);
		AddWindowForPreTranslate(m_wndBend);

		TCHAR szAlignments[256] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_ET_TEXTALIGNMENTS, szAlignments, itemsof(szAlignments), LANGIDFROMLCID(m_tLocaleID));
		for (TCHAR *p = szAlignments; *p; ++p)
			if (*p == _T('|')) *p = _T('\0');

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataText::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataText::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		int nIconSize = XPGUI::GetSmallIconSize();
		m_cImageList.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), 5, 0);
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			IRPathPoint const outside[] =
			{
				{1, 35, 0, 0, 0, 0},
				{1, 0, 0, 0, 0, 0},
				{147, 0, 22.3021, 0, 0, 0},
				{199, 8, 26, 11, -12.6806, -5.36489},
				{239, 62, 0, 26, 0, -23},
				{191, 121, 39, 10, 31, -11},
				{248, 183, 0, 28, 0, -30},
				{201, 247, -14.8023, 6.41433, 30, -13},
				{135, 256, 0, 0, 28.8438, 0},
				{1, 256, 0, 0, 0, 0},
				{1, 221, 0, 0, 0, 0},
				{19, 221, 6.89583, 0, 0, 0},
				{32, 208, 0, 0, 0, 7.5},
				{32, 48, 0, -7.625, 0, 0},
				{19, 35, 0, 0, 6.89583, 0},
			};
			IRPathPoint const top[] =
			{
				{95, 104, 0, 0, 0, 0},
				{135, 104, 28, 0, 0, 0},
				{177, 72, 0, -17.1208, 0, 17.6731},
				{140, 41, 0, 0, 25, 0},
				{95, 41, 0, 0, 0, 0},
			};
			IRPathPoint const bot[] =
			{
				{95, 141, 0, 0, 0, 0},
				{95, 208, 0, 3, 0, 0},
				{101, 215, 0, 0, -4, 0},
				{137, 215, 29.2711, 0, 0, 0},
				{182, 176, 0, -19.33, 0, 21.5391},
				{140, 141, 0, 0, 29.8234, 0},
			};
			IRPath const bold[] = { {itemsof(outside), outside}, {itemsof(top), top}, {itemsof(bot), bot} };
			IRGridItem const grid[] = { {0, 1}, {0, 32}, {0, 95} };
			IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), 0, grid, NULL};
			cRenderer(&canvas, itemsof(bold), bold, pSI->GetMaterial(ESMInterior));
			HICON hTmp = cRenderer.get(); m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			IRPathPoint const italic[] =
			{
				{93, 0, 0, 0, 0, 0},
				{218, 0, 0, 0, 0, 0},
				{210, 35, 0, 0, 0, 0},
				{193, 35, -7.375, 0, 0, 0},
				{177, 48, 0, 0, 1.71875, -8.48958},
				{141, 208, -1.34375, 5.07292, 0, 0},
				{152, 221, 0, 0, -7.84375, 0},
				{171, 221, 0, 0, 0, 0},
				{163, 256, 0, 0, 0, 0},
				{38, 256, 0, 0, 0, 0},
				{46, 221, 0, 0, 0, 0},
				{63, 221, 6.95833, 0, 0, 0},
				{79, 208, 0, 0, -1.91667, 8.72917},
				{115, 48, 1.22917, -5.28125, 0, 0},
				{104, 35, 0, 0, 7.8125, 0},
				{85, 35, 0, 0, 0, 0},
			};
			IRGridItem const grid[] = { {0, 0}, {0, 35}, {0, 221}, {0, 256} };
			IRCanvas const canvas = {0, 0, 256, 256, 0, itemsof(grid), NULL, grid};
			cRenderer(&canvas, itemsof(italic), italic, pSI->GetMaterial(ESMInterior));
			HICON hTmp = cRenderer.get(); m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			IRPolyPoint const left[] =
			{
				{0, 32}, {192, 32}, {192, 96}, {96, 96}, {96, 160}, {144, 160}, {144, 224}, {0, 224},
			};
			IRGridItem const gridX[] = { {0, 0}, {0, 96}, {0, 144}, {0, 192} };
			IRGridItem const gridY[] = { {0, 32}, {0, 96}, {0, 160} };
			IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
			cRenderer(&canvas, itemsof(left), left, pSI->GetMaterial(ESMInterior));
			HICON hTmp = cRenderer.get(); m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			IRPolyPoint const center[] =
			{
				{32, 32}, {224, 32}, {224, 96}, {176, 96}, {176, 160}, {200, 160}, {200, 224}, {56, 224}, {56, 160}, {80, 160}, {80, 96}, {32, 96},
			};
			IRGridItem const gridX[] = { {0, 32}, {0, 56}, {0, 80}, {0, 176}, {0, 200}, {0, 224} };
			IRGridItem const gridY[] = { {0, 32}, {0, 96}, {0, 160} };
			IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
			cRenderer(&canvas, itemsof(center), center, pSI->GetMaterial(ESMInterior));
			HICON hTmp = cRenderer.get(); m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			IRPolyPoint const right[] =
			{
				{256, 32}, {64, 32}, {64, 96}, {160, 96}, {160, 160}, {112, 160}, {112, 224}, {256, 224},
			};
			IRGridItem const gridX[] = { {0, 64}, {0, 112}, {0, 160}, {0, 256} };
			IRGridItem const gridY[] = { {0, 32}, {0, 96}, {0, 160} };
			IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
			cRenderer(&canvas, itemsof(right), right, pSI->GetMaterial(ESMInterior));
			HICON hTmp = cRenderer.get(); m_cImageList.AddIcon(hTmp); DestroyIcon(hTmp);
		}

		CComBSTR bstrBold;
		CMultiLanguageString::GetLocalized(L"[0409]Bold[0405]Tučně", m_tLocaleID, &bstrBold);
		CComBSTR bstrItalic;
		CMultiLanguageString::GetLocalized(L"[0409]Italic[0405]Kurzíva", m_tLocaleID, &bstrItalic);

		m_wndFontStyle = GetDlgItem(IDC_ET_FONT_STYLE);
		m_wndFontStyle.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndFontStyle.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndFontStyle.SetImageList(m_cImageList);
		TBBUTTON aButtonsStyle[] =
		{
			{0, ID_BOLD, TBSTATE_ENABLED|(m_cData.bBold ? TBSTATE_CHECKED : 0), BTNS_BUTTON, TBBUTTON_PADDING, 0, INT_PTR(bstrBold.m_str)},
			{1, ID_ITALIC, TBSTATE_ENABLED|(m_cData.bItalic ? TBSTATE_CHECKED : 0), BTNS_BUTTON, TBBUTTON_PADDING, 0, INT_PTR(bstrItalic.m_str)},
		};
		m_wndFontStyle.AddButtons(itemsof(aButtonsStyle), aButtonsStyle);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndFontStyle.SetButtonSize(nButtonSize, nButtonSize);
		}

		RECT rcToolbar;
		m_wndFontStyle.GetItemRect(m_wndFontStyle.GetButtonCount()-1, &rcToolbar);
		RECT rcRef;
		m_wndFontCombo.GetWindowRect(&rcRef);
		ScreenToClient(&rcRef);
		RECT rcActual;
		m_wndFontStyle.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		//LONG nDelta = rcActual.bottom-(rcActual.top+rcToolbar.bottom);
		rcActual.top = ((rcRef.top+rcRef.bottom)>>1)-(rcToolbar.bottom>>1);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcRef.right -= rcActual.left-(rcActual.right-rcToolbar.right);
		rcActual.left = rcActual.right-rcToolbar.right;
		m_wndFontStyle.MoveWindow(&rcActual, FALSE);
		m_wndFontCombo.MoveWindow(&rcRef, FALSE);

		m_wndAlignment = GetDlgItem(IDC_ET_ALIGN);
		m_wndAlignment.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndAlignment.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndAlignment.SetImageList(m_cImageList);
		m_wndAlignment.AddStrings(szAlignments);
		TBBUTTON aButtonsAlign[] =
		{
			{2, ID_ALIGN_LEFT, TBSTATE_ENABLED|(m_cData.eAlign == CEditToolDataText::ETALeft ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 0},
			{3, ID_ALIGN_CENTER, TBSTATE_ENABLED|(m_cData.eAlign == CEditToolDataText::ETACenter ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
			{4, ID_ALIGN_RIGHT, TBSTATE_ENABLED|(m_cData.eAlign == CEditToolDataText::ETARight ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 2},
		};
		m_wndAlignment.AddButtons(itemsof(aButtonsAlign), aButtonsAlign);
		if (CTheme::IsThemingSupported() && IsAppThemed())
		{
			int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			m_wndAlignment.SetButtonSize(nButtonSize, nButtonSize);
		}

		m_wndAlignment.GetItemRect(m_wndAlignment.GetButtonCount()-1, &rcToolbar);
		m_wndSize.GetWindowRect(&rcRef);
		ScreenToClient(&rcRef);
		RECT rcSpin;
		CWindow wndSpin = GetDlgItem(IDC_ET_SIZESPIN);
		wndSpin.GetWindowRect(&rcSpin);
		ScreenToClient(&rcSpin);
		m_wndAlignment.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		//LONG nDelta = rcActual.bottom-(rcActual.top+rcToolbar.bottom);
		rcActual.top = ((rcRef.top+rcRef.bottom)>>1)-(rcToolbar.bottom>>1);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcRef.right -= rcActual.left-(rcActual.right-rcToolbar.right);
		rcSpin.left -= rcActual.left-(rcActual.right-rcToolbar.right);
		rcSpin.right -= rcActual.left-(rcActual.right-rcToolbar.right);
		rcActual.left = rcActual.right-rcToolbar.right;
		m_wndAlignment.MoveWindow(&rcActual, FALSE);
		m_wndSize.MoveWindow(&rcRef, FALSE);
		wndSpin.MoveWindow(&rcSpin, FALSE);

		m_bEnableUpdates = true;

		DataToGUI();

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}

	//LRESULT OnClickedFont(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	//{
	//	LOGFONT lfTmp = // stupid LOGFONTA-LOGFONTW code
	//	{
	//		m_cData.sHelp.lfHeight,
	//		m_cData.sHelp.lfWidth,
	//		m_cData.sHelp.lfEscapement,
	//		m_cData.sHelp.lfOrientation,
	//		m_cData.sHelp.lfWeight,
	//		m_cData.sHelp.lfItalic,
	//		m_cData.sHelp.lfUnderline,
	//		m_cData.sHelp.lfStrikeOut,
	//		m_cData.sHelp.lfCharSet,
	//		m_cData.sHelp.lfOutPrecision,
	//		m_cData.sHelp.lfClipPrecision,
	//		/*m_cData.sHelp.bAntialias ? */ANTIALIASED_QUALITY/* : NONANTIALIASED_QUALITY*/,
	//		m_cData.sHelp.lfPitchAndFamily,
	//	};
	//	_tcsncpy(lfTmp.lfFaceName, CW2CT(m_cData.sHelp.lfFaceName), itemsof(lfTmp.lfFaceName));
	//	CFontDialog cFontDlg(&lfTmp);
	//	if (cFontDlg.DoModal(m_hWnd) == IDOK)
	//	{
	//		cFontDlg.GetCurrentFont(&lfTmp);
	//		m_cData.sHelp.lfHeight = lfTmp.lfHeight;
	//		m_cData.sHelp.lfWidth = lfTmp.lfWidth;
	//		m_cData.sHelp.lfEscapement = lfTmp.lfEscapement;
	//		m_cData.sHelp.lfOrientation = lfTmp.lfOrientation;
	//		m_cData.sHelp.lfWeight = lfTmp.lfWeight;
	//		m_cData.sHelp.lfItalic = lfTmp.lfItalic;
	//		m_cData.sHelp.lfUnderline = lfTmp.lfUnderline;
	//		m_cData.sHelp.lfStrikeOut = lfTmp.lfStrikeOut;
	//		m_cData.sHelp.lfCharSet = lfTmp.lfCharSet;
	//		m_cData.sHelp.lfOutPrecision = lfTmp.lfOutPrecision;
	//		m_cData.sHelp.lfClipPrecision = lfTmp.lfClipPrecision;
	//		m_cData.sHelp.lfPitchAndFamily = lfTmp.lfPitchAndFamily;
	//		wcsncpy(m_cData.sHelp.lfFaceName, CT2CW(lfTmp.lfFaceName), itemsof(m_cData.sHelp.lfFaceName));
	//		GUIToData();
	//		DataToState();
	//		SetFontToEdit();
	//	}
	//	return 0;
	//}
	LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fSize > 1.0f)
			{
				m_cData.fSize = max(1.0f, m_cData.fSize-1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fSize);
				m_wndSize.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fSize < 1000.0f)
			{
				m_cData.fSize = min(1000.0f, m_cData.fSize+1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fSize);
				m_wndSize.SetWindowText(szTmp);
			}
		}

		return 0;
	}
	LRESULT OnAngleUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			m_cData.fAngle = ceilf((m_cData.fAngle-0.5f)/15.0f)*15.0f-15.0f;
			if (m_cData.fAngle < 0.0f) m_cData.fAngle = 345.0f;
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fAngle);
			m_wndAngle.SetWindowText(szTmp);
		}
		else
		{
			m_cData.fAngle = floorf((m_cData.fAngle+0.5f)/15.0f)*15.0f+15.0f;
			if (m_cData.fAngle >= 360.0f) m_cData.fAngle = 0.0f;
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fAngle);
			m_wndAngle.SetWindowText(szTmp);
		}

		return 0;
	}
	LRESULT OnBendUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			m_cData.fBend = ceilf((m_cData.fBend-0.5f)/10.0f)*10.0f-10.0f;
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fBend);
			m_wndBend.SetWindowText(szTmp);
		}
		else
		{
			m_cData.fBend = floorf((m_cData.fBend+0.5f)/10.0f)*10.0f+10.0f;
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fBend);
			m_wndBend.SetWindowText(szTmp);
		}

		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnChangeStyle(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		if (a_wID == ID_BOLD)
			m_cData.bBold = !m_cData.bBold;
		else
			m_cData.bItalic = !m_cData.bItalic;
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeAlign(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		m_cData.eAlign = a_wID == ID_ALIGN_CENTER ? CEditToolDataText::ETACenter : (a_wID == ID_ALIGN_RIGHT ? CEditToolDataText::ETARight : CEditToolDataText::ETALeft);
		DataToGUI();
		DataToState();
		return 0;
	}

	//void UpdateValues()
	//{
	//	if (!m_bEnableUpdates)
	//		return;

	//	if (m_pSharedState != NULL)
	//	{
	//		CComPtr<ISharedStateImageEdit> pState;
	//		m_pSharedState->StateGet(m_bstrSyncGroup, __uuidof(ISharedStateImageEdit), reinterpret_cast<void**>(&pState));
	//		if (pState != NULL)
	//		{
	//			m_cData.Deserialize(m_pszToolID, pState);
	//			DataToGUI();
	//		}
	//	}
	//}
	void GUIToData()
	{
		CComBSTR bstrVal;
		m_wndText.GetWindowText(&bstrVal);
		m_cData.strText = bstrVal == NULL ? L"" : bstrVal.m_str;
		int i = m_wndFontCombo.GetCurSel();
		if (i < 0 || i > m_wndFontCombo.GetCount())
			return ;
		CComBSTR bstr = NULL;
		m_wndFontCombo.GetLBTextBSTR(i, bstr.m_str);
		wcsncpy(m_cData.lfFaceName, bstr, itemsof(m_cData.lfFaceName));
		TCHAR szVal[32] = _T("");
		m_wndSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		float fSize = 0.0f;
		_stscanf(szVal, _T("%f"), &fSize);
		if (fSize != 0.0f)
			m_cData.fSize = fSize;
		szVal[0] = _T('\0');
		m_wndAngle.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		float fAngle = -1000.0f;
		_stscanf(szVal, _T("%f"), &fAngle);
		if (fAngle != -1000.0f)
			m_cData.fAngle = fAngle;
		szVal[0] = _T('\0');
		m_wndBend.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fBend);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		int nCount = m_wndFontCombo.GetCount();
		for (int i = 0; i < nCount; ++i)
		{
			CComBSTR bstr;
			m_wndFontCombo.GetLBTextBSTR(i, bstr.m_str);
			if (wcscmp(bstr, m_cData.lfFaceName) == 0)
				m_wndFontCombo.SetCurSel(i);
		}
		CComBSTR bstrPrev;
		m_wndText.GetWindowText(&bstrPrev);
		if (bstrPrev != m_cData.strText.c_str())
		{
			m_wndText.SetWindowText(COLE2T(m_cData.strText.c_str()));
		}
		SetFontToEdit();
		m_wndFontStyle.CheckButton(ID_BOLD, m_cData.bBold);
		m_wndFontStyle.CheckButton(ID_ITALIC, m_cData.bItalic);
		TCHAR szPrev[33] = _T("");
		m_wndSize.GetWindowText(szPrev, itemsof(szPrev));
		float f = m_cData.fSize+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fSize)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fSize);
			m_wndSize.SetWindowText(szTmp);
		}
		szPrev[0] = _T('\0');
		m_wndAngle.GetWindowText(szPrev, itemsof(szPrev));
		f = m_cData.fAngle+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fAngle)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fAngle);
			m_wndAngle.SetWindowText(szTmp);
		}
		m_wndBend.GetWindowText(szPrev, itemsof(szPrev));
		f = m_cData.fBend+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fBend)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fBend);
			m_wndBend.SetWindowText(szTmp);
		}
		m_wndAlignment.CheckButton(ID_ALIGN_LEFT, m_cData.eAlign == CEditToolDataText::ETALeft);
		m_wndAlignment.CheckButton(ID_ALIGN_CENTER, m_cData.eAlign == CEditToolDataText::ETACenter);
		m_wndAlignment.CheckButton(ID_ALIGN_RIGHT, m_cData.eAlign == CEditToolDataText::ETARight);
		m_bEnableUpdates = true;
	}
	void SetFontToEdit()
	{
		// not that good at all

		//CFont cTmp;
		//cTmp.Attach(m_cFont.Detach());
		//m_cFont.CreateFont(
		//	m_cData.tFontInfo.lfHeight,
		//	m_cData.tFontInfo.lfWidth,
		//	m_cData.tFontInfo.lfEscapement,
		//	m_cData.tFontInfo.lfOrientation,
		//	m_cData.tFontInfo.lfHeight,
		//	m_cData.tFontInfo.lfItalic,
		//	m_cData.tFontInfo.lfUnderline,
		//	m_cData.tFontInfo.lfStrikeOut,
		//	m_cData.tFontInfo.lfCharSet,
		//	m_cData.tFontInfo.lfOutPrecision,
		//	m_cData.tFontInfo.lfClipPrecision,
		//	m_cData.tFontInfo.lfQuality,
		//	m_cData.tFontInfo.lfPitchAndFamily,
		//	CW2CT(m_cData.tFontInfo.lfFaceName)
		//	);
		//m_wndText.SetFont(m_cFont);
	}

private:
	CEdit m_wndText;
	//CFont m_cFont;
	CFontComboBox m_wndFontCombo;
	CEdit m_wndSize;
	CEdit m_wndAngle;
	CEdit m_wndBend;
	CToolBarCtrl m_wndFontStyle;
	CToolBarCtrl m_wndAlignment;
	CImageList m_cImageList;
};


