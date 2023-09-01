// FillStylePatternDlg.h : Declaration of the CFillStylePatternDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <XPGUI.h>
#include "EditToolDlg.h"


// CFillStylePatternDlg

class CFillStylePatternDlg : 
	public CEditToolDlgBase<CFillStylePatternDlg, CFillStyleDataPattern, Win32LangEx::CLangIndirectDialogImpl<CFillStylePatternDlg> >,
	public CObserverImpl<CFillStylePatternDlg, IColorPickerObserver, ULONG>
{
public:
	CFillStylePatternDlg() : m_nIconSize(0), m_nPreviewSize(0), m_nGap(0), m_nHeight1(0), m_nHeight2(0), m_nTBHeight(0), m_nSepHeight(0)
	{
		m_tBorders.cx = m_tBorders.cy = 0;
		m_tBtnSize.cx = m_tBtnSize.cy = 20;
	}
	~CFillStylePatternDlg()
	{
		m_cPatterns.Destroy();
	}

	enum { IDC_PATTERN_LIST = 100, IDC_PREVIEW, IDC_CUSTOM_PATTERN, IDC_CLIPBOARD_PATTERN, IDC_SEPLINE, IDC_PATTERN_FIRST = 200, IDC_PATTERN_LAST = IDC_PATTERN_FIRST+sizeof(g_aPatterns)+sizeof(g_aPatterns[0]) };

	BEGIN_DIALOG_EX(0, 0, 100, 56, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CONTROL(_T(""), IDC_PREVIEW, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 5, 5, 29, 29, 0)
		CONTROL_CONTROL(_T(""), IDC_PATTERN_LIST, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 34, 5, 56, 28, 0)
		CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 5, 56, 90, 1, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CFillStylePatternDlg)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_RANGE_HANDLER(IDC_PATTERN_FIRST, IDC_PATTERN_LAST, OnStockPattern)
		COMMAND_HANDLER(IDC_CUSTOM_PATTERN, BN_CLICKED, OnCustomPattern)
		COMMAND_HANDLER(IDC_CLIPBOARD_PATTERN, BN_CLICKED, OnClipboardPattern)
	END_MSG_MAP()

	BEGIN_COM_MAP(CFillStylePatternDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CFillStyleDataPattern::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				DataToGUI();
			}
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (m_nHeight1)
				a_pSize->cy = m_cData.bStock ? m_nHeight2 : m_nHeight1;
			else
				GetDialogSize(a_pSize, m_tLocaleID);
			return S_OK;
			//return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		if (!m_bEnableUpdates)
			return S_FALSE;

		CComQIPtr<CFillStyleDataPattern::ISharedStateToolData> pState(a_pState);
		if (pState != NULL)
		{
			m_cData = *(pState->InternalData());
			if (m_cData.nIndex > itemsof(g_aPatterns))
				m_cData.nIndex = itemsof(g_aPatterns);
			DataToGUI();
		}
		return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pWnd1)
		{
			HRESULT hRes = m_pWnd1->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
			if (hRes == S_OK)
				return S_OK;
		}
		if (m_pWnd2)
		{
			HRESULT hRes = m_pWnd2->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
			if (hRes == S_OK)
				return S_OK;
		}
		return CEditToolDlgBase<CFillStylePatternDlg, CFillStyleDataPattern, Win32LangEx::CLangIndirectDialogImpl<CFillStylePatternDlg> >::PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<CFillStyleDataPattern::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CFillStyleDataPattern::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
				if (m_cData.nIndex > itemsof(g_aPatterns))
					m_cData.nIndex = itemsof(g_aPatterns);
			}
		}

		m_wndToolBar = GetDlgItem(IDC_PATTERN_LIST);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

		RECT rcMap = {0, 4, 5, 5};
		MapDialogRect(&rcMap);
		m_tBorders.cx = rcMap.right;
		m_tBorders.cy = rcMap.bottom;
		m_nGap = rcMap.top;

		m_bEnableUpdates = false;
		m_nIconSize = XPGUI::GetSmallIconSize();
		ULONG nIconDelta = (m_nIconSize>>1)-8;
		ULONG nGapLeft = nIconDelta>>1;
		ULONG nGapRight = nIconDelta-nGapLeft;
		RECT padding = {nGapLeft, 0, nGapRight, 0};
		m_cPatterns.Create(m_nIconSize+nIconDelta, m_nIconSize, XPGUI::GetImageListColorFlags(), itemsof(g_aPatterns), 0);
		CAutoVectorPtr<DWORD> pData(new DWORD[(m_nIconSize+nIconDelta)*m_nIconSize]);
		HDC hDC = GetDC();
		BITMAPINFO tBMI;
		ZeroMemory(&tBMI, sizeof tBMI);
		tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
		tBMI.bmiHeader.biWidth = m_nIconSize+nIconDelta;
		tBMI.bmiHeader.biHeight = m_nIconSize;
		tBMI.bmiHeader.biPlanes = 1;
		tBMI.bmiHeader.biBitCount = 32;
		tBMI.bmiHeader.biCompression = BI_RGB;

		CAutoVectorPtr<TBBUTTON> aButtons(new TBBUTTON[itemsof(g_aPatterns)+2]);
		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pTSD = szTooltipStrings;
		for (SPattern const* p = g_aPatterns; p != g_aPatterns+itemsof(g_aPatterns); ++p)
		{
			DWORD* pD = pData;
			for (ULONG y = 0; y < m_nIconSize; ++y)
			{
				for (ULONG x = 0; x < nGapLeft; ++x, ++pD)
					*pD = 0;
				for (ULONG x = 0; x < m_nIconSize; ++x, ++pD)
				{
					DWORD pix = 255-p->pP[p->nX*((m_nIconSize-1-y)%p->nY)+(x%p->nX)];
					*pD = 0xff000000|pix|(pix<<8)|(pix<<16);
				}
				for (ULONG x = 0; x < nGapRight; ++x, ++pD)
					*pD = 0;
			}
			HBITMAP hBmp = CreateDIBitmap(hDC, &tBMI.bmiHeader, CBM_INIT, pData.m_p, &tBMI, DIB_RGB_COLORS);
			m_cPatterns.Add(hBmp);
			DeleteObject(hBmp);
			size_t i = p-g_aPatterns;
			TBBUTTON const tBtn = {i, IDC_PATTERN_FIRST+i, TBSTATE_ENABLED|(m_cData.bStock && m_cData.nIndex == i ? TBSTATE_CHECKED : 0), BTNS_BUTTON, TBBUTTON_PADDING, 0, i};
			aButtons[i] = tBtn;
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(p->pName, m_tLocaleID, &bstr);
			wcscpy(pTSD, bstr.m_str == NULL ? L"" : bstr);
			pTSD += bstr.Length()+1;
			*pTSD = L'\0';
		}
		ReleaseDC(hDC);
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		{
			CIconRendererReceiver cRenderer(m_nIconSize);
			pSI->GetLayers(ESIPicture, cRenderer, IRTarget(1, 0, 1));
			pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.65f, 1, -1));
			HICON hIc = cRenderer.get(padding);
			m_cPatterns.AddIcon(hIc);
			DestroyIcon(hIc);
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Custom image[0405]Vlastní obrázek", m_tLocaleID, &bstr);
			wcscpy(pTSD, bstr.m_str == NULL ? L"" : bstr);
			pTSD += bstr.Length()+1;
			*pTSD = L'\0';
			TBBUTTON const tBtn = {itemsof(g_aPatterns), IDC_CUSTOM_PATTERN, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, itemsof(g_aPatterns)};
			aButtons[itemsof(g_aPatterns)] = tBtn;
		}
		{
			CIconRendererReceiver cRenderer(m_nIconSize);
			pSI->GetLayers(ESIPicture, cRenderer, IRTarget(1, 0, 1));
			static IRPolyPoint const poly0[] = {{0, 160}, {0, 256}, {256, 256}, {256, 160}};
			static IRPolyPoint const poly1[] = {{80, 0}, {80, 114}, {29, 114}, {128, 213}, {227, 114}, {176, 114}, {176, 0}};
			IRTarget target(0.65f, 1.0f, -1.0f);
			static IRGridItem const gridx[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 80.0f}, {EGIFInteger, 176.0f}, {EGIFInteger, 256.0f}};
			static IRGridItem const gridy[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 114.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 256.0f}};
			static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridx), itemsof(gridy), gridx, gridy};
			cRenderer(&canvas, itemsof(poly0), poly0, pSI->GetMaterial(ESMManipulate), &target);
			cRenderer(&canvas, itemsof(poly1), poly1, pSI->GetMaterial(ESMManipulate), &target);
			HICON hIc = cRenderer.get(padding);
			m_cPatterns.AddIcon(hIc);
			DestroyIcon(hIc);
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Image from clipboard[0405]Obrázek ze schránky", m_tLocaleID, &bstr);
			wcscpy(pTSD, bstr.m_str == NULL ? L"" : bstr);
			pTSD += bstr.Length()+1;
			*pTSD = L'\0';
			TBBUTTON const tBtn = {itemsof(g_aPatterns)+1, IDC_CLIPBOARD_PATTERN, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, itemsof(g_aPatterns)+1};
			aButtons[itemsof(g_aPatterns)+1] = tBtn;
		}

		m_wndToolBar.SetImageList(m_cPatterns);
		m_wndToolBar.AddStrings(szTooltipStrings);
		m_wndToolBar.AddButtons(itemsof(g_aPatterns)+2, aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
			m_wndToolBar.SetButtonSize(m_nIconSize+8, m_nIconSize+(m_nIconSize>>1)+1);

		m_bEnableUpdates = true;

		RECT rcClient = {0, 0, 0, 0};
		GetClientRect(&rcClient);

		RECT rcToolbar;
		m_wndToolBar.GetItemRect(0, &rcToolbar);
		m_tBtnSize.cx = rcToolbar.right-rcToolbar.left;
		m_tBtnSize.cy = rcToolbar.bottom-rcToolbar.top;
		m_nPreviewSize = 2*(rcToolbar.bottom-rcToolbar.top);
		m_wndToolBar.SetWindowPos(NULL, m_tBorders.cx+m_nPreviewSize+m_nGap, m_tBorders.cy, rcClient.right-(m_tBorders.cx*2+m_nPreviewSize+m_nGap), m_nPreviewSize, SWP_NOZORDER);

		CWindow wndPreview = GetDlgItem(IDC_PREVIEW);
		wndPreview.SetWindowPos(NULL, m_tBorders.cx, m_tBorders.cy, m_nPreviewSize, m_nPreviewSize, SWP_NOZORDER);

		RECT rcWnd1 = {m_tBorders.cx, m_tBorders.cy+2*(rcToolbar.bottom-rcToolbar.top)+m_nGap, rcClient.right-m_tBorders.cx, m_tBorders.cy+2*(rcToolbar.bottom-rcToolbar.top)+m_nGap+20};
		RWCoCreateInstance(m_pWnd1, __uuidof(ColorPicker));
		m_pWnd1->Create(m_hWnd, &rcWnd1, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Primary color[0405]Hlavní barva"), FALSE, CComBSTR(L"PATT1"), NULL);
		m_pWnd1->ObserverIns(CObserverImpl<CFillStylePatternDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);

		m_wndSepLine = GetDlgItem(IDC_SEPLINE);
		RECT rcSepOrig = {0, 0, 0, 0};
		m_wndSepLine.GetWindowRect(&rcSepOrig);
		m_nSepHeight = rcSepOrig.bottom-rcSepOrig.top;
		RECT rcSep = {m_tBorders.cx, rcWnd1.bottom+m_nGap, rcClient.right-m_tBorders.cx, rcWnd1.bottom+m_nGap+m_nSepHeight};
		m_wndSepLine.SetWindowPos(NULL, &rcSep, SWP_NOZORDER|SWP_NOACTIVATE);

		RECT rcWnd2 = {m_tBorders.cx, rcSep.top+m_nGap, rcClient.right-m_tBorders.cx, rcSep.top+m_nGap+20};
		RWCoCreateInstance(m_pWnd2, __uuidof(ColorPicker));
		m_pWnd2->Create(m_hWnd, &rcWnd2, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Secondary color[0405]Vedlejší barva"), FALSE, CComBSTR(L"PATT2"), NULL);
		m_pWnd2->ObserverIns(CObserverImpl<CFillStylePatternDlg, IColorPickerObserver, ULONG>::ObserverGet(), 1);

		DataToGUI();

		BOOL b;
		OnSize(0, 0, MAKELPARAM(rcClient.right, rcClient.bottom), b);

		return 1;  // Let the system set the focus
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_pWnd1->ObserverDel(CObserverImpl<CFillStylePatternDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		m_pWnd2->ObserverDel(CObserverImpl<CFillStylePatternDlg, IColorPickerObserver, ULONG>::ObserverGet(), 1);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SIZE const sz = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};

		int nBtns = m_wndToolBar.GetButtonCount();
		m_nTBHeight = m_tBtnSize.cy*2;
		if (int(sz.cx-m_tBorders.cx-m_tBorders.cx-m_nPreviewSize-m_nGap)/m_tBtnSize.cx < (nBtns+1)/2)
		{
			GetDlgItem(IDC_PREVIEW).ShowWindow(SW_HIDE);
			int nPerRow = (sz.cx-m_tBorders.cx-m_tBorders.cx)/m_tBtnSize.cx;
			int nRows = nPerRow ? (nBtns+nPerRow-1)/nPerRow : 1;
			m_wndToolBar.SetWindowPos(NULL, m_tBorders.cx, m_tBorders.cy, sz.cx-(m_tBorders.cx*2), m_nTBHeight = m_tBtnSize.cy*nRows, SWP_NOZORDER);
		}
		else
		{
			GetDlgItem(IDC_PREVIEW).ShowWindow(SW_SHOW);
			m_wndToolBar.SetWindowPos(NULL, m_tBorders.cx+m_nPreviewSize+m_nGap, m_tBorders.cx, sz.cx-(m_tBorders.cx*2+m_nPreviewSize+m_nGap), m_tBtnSize.cy*2, SWP_NOZORDER);
		}

		SIZE tSize1 = {sz.cx-m_tBorders.cx*2, 0};
		m_pWnd1->OptimumSize(&tSize1);
		RECT rc1 = {m_tBorders.cx, m_tBorders.cy+m_nTBHeight+m_nGap, sz.cx-m_tBorders.cx, m_tBorders.cy+m_nTBHeight+m_nGap+tSize1.cy};
		m_pWnd1->Move(&rc1);

		RECT rcSep = {m_tBorders.cx, rc1.bottom+m_nGap, sz.cx-m_tBorders.cx, rc1.bottom+m_nGap+m_nSepHeight};
		m_wndSepLine.SetWindowPos(NULL, &rcSep, SWP_NOZORDER|SWP_NOACTIVATE);

		SIZE tSize2 = {sz.cx-m_tBorders.cx*2, 0};
		m_pWnd2->OptimumSize(&tSize2);
		RECT rc2 = {m_tBorders.cx, rcSep.top+m_nGap, sz.cx-m_tBorders.cx, rcSep.top+m_nGap+tSize2.cy};
		m_pWnd2->Move(&rc2);

		m_nHeight1 = m_tBorders.cy+m_nTBHeight+m_tBorders.cy;
		m_nHeight2 = rc2.bottom+m_tBorders.cy;
		return 0;
	}
	LRESULT OnDrawItem(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam != IDC_PREVIEW)
		{
			a_bHandled = FALSE;
			return 0;
		}
		DRAWITEMSTRUCT* pDIS = reinterpret_cast<DRAWITEMSTRUCT*>(a_lParam);
		int nX = pDIS->rcItem.right-pDIS->rcItem.left;
		int nY = pDIS->rcItem.bottom-pDIS->rcItem.top;
		if (nX < 4 && nY < 4)
			return 1;

		CAutoVectorPtr<DWORD> aBuffer(new DWORD[nX*nY]);
		DWORD* pBuffer = aBuffer;
		if (m_cData.bStock)
		{
			SPattern const* pPat = g_aPatterns+m_cData.nIndex;
			DWORD aMap[256];
			COLORREF clrBG = GetSysColor(COLOR_3DFACE);
			for (int i = 0; i < 256; ++i)
			{
				float fR = (m_cData.tColor1.fA*m_cData.tColor1.fR*i + m_cData.tColor2.fA*m_cData.tColor2.fR*(255-i))/255.0f;
				float fG = (m_cData.tColor1.fA*m_cData.tColor1.fG*i + m_cData.tColor2.fA*m_cData.tColor2.fG*(255-i))/255.0f;
				float fB = (m_cData.tColor1.fA*m_cData.tColor1.fB*i + m_cData.tColor2.fA*m_cData.tColor2.fB*(255-i))/255.0f;
				float fA = (m_cData.tColor1.fA*i+m_cData.tColor2.fA*(255-i))/255.0f;
				if (fA < 1.0f)
				{
					fR += powf(GetRValue(clrBG)/255.0f, 2.2f)*(1.0f-fA);
					fG += powf(GetGValue(clrBG)/255.0f, 2.2f)*(1.0f-fA);
					fB += powf(GetBValue(clrBG)/255.0f, 2.2f)*(1.0f-fA);
					fA = 1.0f;
				}

				aMap[i] = 0xff000000|RGB(int(powf(fB/fA, 1.0f/2.2f)*255.0f+0.5f), int(powf(fG/fA, 1.0f/2.2f)*255.0f+0.5f), int(powf(fR/fA, 1.0f/2.2f)*255.0f+0.5f));
			}
			COLORREF clrBG2 = GetSysColor(COLOR_3DSHADOW);
			DWORD* pBuffer = aBuffer;
			std::fill_n(pBuffer, nX, clrBG2); pBuffer += nX;
			*(pBuffer++) = clrBG2;
			std::fill_n(pBuffer, nX-2, clrBG); pBuffer += nX-2;
			*(pBuffer++) = clrBG2;
			for (int y = 2; y+2 < nY; ++y)
			{
				ULONG const nDY = pPat->nX*(y%pPat->nY);
				*(pBuffer++) = clrBG2;
				*(pBuffer++) = clrBG;
				for (int x = 2; x+2 < nX; ++x)
					*(pBuffer++) = aMap[pPat->pP[(x%pPat->nX)+nDY]];
				*(pBuffer++) = clrBG;
				*(pBuffer++) = clrBG2;
			}
			*(pBuffer++) = clrBG2;
			std::fill_n(pBuffer, nX-2, clrBG); pBuffer += nX-2;
			*(pBuffer++) = clrBG2;
			std::fill_n(pBuffer, nX, clrBG2); pBuffer += nX;
		}
		else
		{
			ObjectLock cLock(this);
			if (m_pCustBrush.m_p == NULL)
				m_pCustBrush.Allocate(m_nPreviewSize*m_nPreviewSize);
			pBuffer = reinterpret_cast<DWORD*>(m_pCustBrush.m_p);
			if (m_strPatPath != m_cData.strPath)
			{
				ZeroMemory(m_pCustBrush.m_p, m_nPreviewSize*m_nPreviewSize*sizeof*m_pCustBrush.m_p);
				m_strPatPath = m_cData.strPath;
				try
				{
					if (m_strPatPath.substr(0, 10) == L"clipboard:")
					{
						TImageSize tSize = {0, 0};
						CAutoVectorPtr<TPixelChannel> pData;
						GetClipboardImage(m_hWnd, tSize, pData);
						TPixelChannel* pD = m_pCustBrush.m_p;
						for (ULONG y = 0; y < m_nPreviewSize; ++y)
						{
							TPixelChannel const* pS = pData.m_p+tSize.nX*(y%m_nPreviewSize);
							for (ULONG x = 0; x < m_nPreviewSize; ++x, ++pD)
								*pD = pS[x%m_nPreviewSize];
						}
					}
					else
					{
						CComPtr<IInputManager> pInMgr;
						RWCoCreateInstance(pInMgr, __uuidof(InputManager));
						CComPtr<IEnumUnknowns> pBuilders;
						pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
						CComPtr<IDocument> pDoc;
						pInMgr->DocumentCreateEx(pBuilders, CStorageFilter(m_cData.strPath.c_str()), NULL, &pDoc);
						CComPtr<IDocumentImage> pImage;
						pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
						TImageSize tSize;
						pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
						for (ULONG y = 0; y < m_nPreviewSize; y+=tSize.nY)
						{
							for (ULONG x = 0; x < m_nPreviewSize; x+=tSize.nX)
							{
								CImageSize cSize(min(m_nPreviewSize-x, tSize.nX), min(m_nPreviewSize-y, tSize.nY));
								pImage->TileGet(EICIRGBA, CImagePoint(0, 0), &cSize, CImageStride(1, m_nPreviewSize), m_nPreviewSize*cSize.nY, m_pCustBrush.m_p+y*m_nPreviewSize+x, NULL, EIRIPreview);
							}
						}
					}
					COLORREF clrBG = GetSysColor(COLOR_3DFACE);
					COLORREF clrBG2 = GetSysColor(COLOR_3DSHADOW);
					float fR = powf(GetRValue(clrBG)/255.0f, 2.2f);
					float fG = powf(GetGValue(clrBG)/255.0f, 2.2f);
					float fB = powf(GetBValue(clrBG)/255.0f, 2.2f);
					TPixelChannel* p = m_pCustBrush;
					for (TPixelChannel* const pEnd = p+m_nPreviewSize*m_nPreviewSize; p != pEnd; ++p)
					{
						if (p->bA == 0xff)
							continue;
						float fA = p->bA/255.0f;
						float fCR = powf(p->bR/255.0f, 2.2f)*fA;
						float fCG = powf(p->bG/255.0f, 2.2f)*fA;
						float fCB = powf(p->bB/255.0f, 2.2f)*fA;
						float fIA = 1.0f-fA;

						p->bR = int(powf(fR*fIA+fCR, 1.0f/2.2f)*255.0f+0.5f);
						p->bG = int(powf(fG*fIA+fCG, 1.0f/2.2f)*255.0f+0.5f);
						p->bB = int(powf(fB*fIA+fCB, 1.0f/2.2f)*255.0f+0.5f);
						p->bA = 255;
					}
					DWORD* pBuffer = reinterpret_cast<DWORD*>(m_pCustBrush.m_p);
					std::fill_n(pBuffer, nX, clrBG2); pBuffer += nX;
					*(pBuffer++) = clrBG2;
					std::fill_n(pBuffer, nX-2, clrBG); pBuffer += nX-2;
					*(pBuffer++) = clrBG2;
					for (int y = 2; y+2 < nY; ++y)
					{
						*(pBuffer++) = clrBG2;
						*(pBuffer++) = clrBG;
						pBuffer += nX-4;
						*(pBuffer++) = clrBG;
						*(pBuffer++) = clrBG2;
					}
					*(pBuffer++) = clrBG2;
					std::fill_n(pBuffer, nX-2, clrBG); pBuffer += nX-2;
					*(pBuffer++) = clrBG2;
					std::fill_n(pBuffer, nX, clrBG2); pBuffer += nX;
				}
				catch (...)
				{
				}
			}
		}

		BITMAPINFO bmp_info;
		ZeroMemory(&bmp_info, sizeof bmp_info);
		bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmp_info.bmiHeader.biWidth = nX;
		bmp_info.bmiHeader.biHeight = -nY;
		bmp_info.bmiHeader.biPlanes = 1;
		bmp_info.bmiHeader.biBitCount = 32;
		bmp_info.bmiHeader.biCompression = BI_RGB;

		::SetDIBitsToDevice(pDIS->hDC, pDIS->rcItem.left, pDIS->rcItem.top, nX, nY, 0, 0, 0, nY, pBuffer, &bmp_info, 0);

		return 1;
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnStockPattern(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_cData.bStock || m_cData.nIndex != a_wID)
		{
			m_cData.bStock = true;
			m_cData.nIndex = a_wID-IDC_PATTERN_FIRST;
			DataToGUI();
			DataToState();
		}
		return 0;
	}
	LRESULT OnCustomPattern(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pLoc;
		static const GUID tID = {0x6989e4f1, 0x354b, 0x4f59, {0xbe, 0xe8, 0xe7, 0xed, 0x9, 0x23, 0x9e, 0x29}};
		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		CComPtr<IEnumUnknowns> pTypes;
		pInMgr->DocumentTypesEnumEx(pBuilders, &pTypes);
		pStMgr->FilterCreateInteractivelyUID(CComBSTR(m_cData.strPath.c_str()), EFTOpenExisting, m_hWnd, pTypes, NULL, tID, _SharedStringTable.GetStringAuto(IDS_OPENCUSTOMPATTERN), NULL, m_tLocaleID, &pLoc);
		if (pLoc)
		{
			if (m_cData.bStock)
			{
				m_cData.bStock = false;
				DataToGUI();
			}
			CComBSTR bstr;
			pLoc->ToText(NULL, &bstr);
			if (m_cData.strPath.compare(bstr) != 0)
			{
				m_cData.strPath = bstr;
				DataToState();
			}
		}
		return 0;
	}
	LRESULT OnClipboardPattern(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CClipboardHandler cClipboard(m_hWnd);

		bool bBitmap = false;
		bool bInternal = false;
		ULONG iFmt = 0;
		while (true)
		{
			iFmt = EnumClipboardFormats(iFmt);
			if (iFmt == 0)
				break;
			if (iFmt == CF_DIBV5)
				bInternal = true;
			if (iFmt == CF_BITMAP)
				bBitmap = true;
		}
		if (!bInternal && !bBitmap)
		{
			CComPtr<IApplicationInfo> pAI;
			RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
			CComPtr<ILocalizedString> pName;
			if (pAI) pAI->Name(&pName);
			CComBSTR bstrCaption;
			pName->GetLocalized(m_tLocaleID, &bstrCaption);
			CComBSTR bstrMessage;
			CMultiLanguageString::GetLocalized(L"[0409]An image must be placed on the clipboard first.[0405]Nejprve musíte do schránky umístit obrázek.", m_tLocaleID, &bstrMessage);
			MessageBox(bstrMessage, bstrCaption, MB_OK|MB_ICONEXCLAMATION);
			return 0;
		}

		if (m_cData.bStock)
		{
			m_cData.bStock = false;
			DataToGUI();
		}
		wchar_t sz[64];
		static int i = 0;
		swprintf(sz, L"clipboard:%i", ++i);
		m_cData.strPath = sz;
		DataToState();
		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		GUIToData();
		DataToGUI();
		DataToState();
		return 0;
	}
	void OwnerNotify(TCookie a_tCookie, ULONG a_nFlags)
	{
		if (!m_bEnableUpdates)
			return;

		if (a_nFlags&ECPCColor)
		{
			if (a_tCookie)
				m_pWnd2->ColorGet(&m_cData.tColor2);
			else
				m_pWnd1->ColorGet(&m_cData.tColor1);
			if (m_cData.bStock)
				GetDlgItem(IDC_PREVIEW).Invalidate(FALSE);
			DataToState();
		}
		if (a_nFlags&ECPCLayout)
		{
			RECT rc = {0, 0, 100, 0};
			GetClientRect(&rc);
			SIZE tSize1 = {rc.right-m_tBorders.cx*2, 0};
			m_pWnd1->OptimumSize(&tSize1);
			RECT rc1 = {m_tBorders.cx, m_tBorders.cy+m_nTBHeight+m_nGap, rc.right-m_tBorders.cx, m_tBorders.cy+m_nTBHeight+m_nGap+tSize1.cy};
			m_pWnd1->Move(&rc1);

			RECT rcSep = {m_tBorders.cx, rc1.bottom+m_nGap, rc.right-m_tBorders.cx, rc1.bottom+m_nGap+m_nSepHeight};
			m_wndSepLine.SetWindowPos(NULL, &rcSep, SWP_NOZORDER|SWP_NOACTIVATE);

			SIZE tSize2 = {rc.right-m_tBorders.cx*2, 0};
			m_pWnd2->OptimumSize(&tSize2);
			RECT rc2 = {m_tBorders.cx, rcSep.top+m_nGap, rc.right-m_tBorders.cx, rcSep.top+m_nGap+tSize2.cy};
			m_pWnd2->Move(&rc2);

			m_nHeight1 = m_tBorders.cy+m_nTBHeight+m_tBorders.cy;
			m_nHeight2 = rc2.bottom+m_tBorders.cy;
		}
	}

	void GUIToData()
	{
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		for (size_t i = 0; i != itemsof(g_aPatterns); ++i)
			m_wndToolBar.CheckButton(IDC_PATTERN_FIRST+i, m_cData.bStock && m_cData.nIndex == i);
		m_pWnd1->ColorSet(&m_cData.tColor1);
		m_pWnd2->ColorSet(&m_cData.tColor2);
		m_pWnd1->Show(m_cData.bStock);
		m_pWnd2->Show(m_cData.bStock);
		m_bEnableUpdates = true;
		GetDlgItem(IDC_PREVIEW).Invalidate(FALSE);
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cPatterns;
	CComPtr<IColorPicker> m_pWnd1;
	CComPtr<IColorPicker> m_pWnd2;
	CWindow m_wndSepLine;
	ULONG m_nSepHeight;
	ULONG m_nIconSize;
	ULONG m_nPreviewSize;
	SIZE m_tBtnSize;
	ULONG m_nGap;
	SIZE m_tBorders;
	ULONG m_nHeight1;
	ULONG m_nHeight2;
	ULONG m_nTBHeight;
	std::wstring m_strPatPath;
	CAutoVectorPtr<TPixelChannel> m_pCustBrush;
};


