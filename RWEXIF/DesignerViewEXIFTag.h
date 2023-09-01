// DesignerViewEXIF.h : Declaration of the CDesignerViewEXIFTag

#pragma once
#include "RWEXIF_i.h"
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include "PropertyList.h"

#include "config.h"

#include "libexif/exif-data.h"
#include "libexif/exif-ifd.h"
#include "libexif/exif-loader.h"


// CDesignerViewEXIFTag

class ATL_NO_VTABLE CDesignerViewEXIFTag :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CDesignerViewEXIFTag>,
	public CThemeImpl<CDesignerViewEXIFTag>,
	public CDialogResize<CDesignerViewEXIFTag>,
	public CDesignerViewWndImpl<CDesignerViewEXIFTag, IDesignerView>,
	public CObserverImpl<CDesignerViewEXIFTag, ISharedStateObserver, TSharedStateChange>
{
public:
	CDesignerViewEXIFTag() : m_bBorder(false)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	void Init(IConfig* a_pConfig, ISharedStateManager* a_pFrame, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IImageMetaData* a_pIMD)
	{
		m_bBorder = (a_nStyle & EDVWSBorderMask) == EDVWSBorder;

		m_tLocaleID = a_tLocaleID;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cVal);

		m_bstrSelectionID = cVal;
		if (m_bstrSelectionID.Length())
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewEXIFTag, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pFrame = a_pFrame;
		}
		m_pIMD = a_pIMD;

		m_pDoc = a_pDoc;

		// create self
		if (Create(a_hParent) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}

		MoveWindow(const_cast<LPRECT>(a_rcWindow));
		ShowWindow(SW_SHOW);
	}

	enum
	{
		IDC_ET_NAME = 100,
		IDC_ET_ID,
		IDC_ET_DESCRIPTION,
	};

BEGIN_DIALOG_EX(0, 0, m_bBorder ? 151 : 150, 100, 0)
	DIALOG_FONT_AUTO()
	DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
	DIALOG_EXSTYLE(m_bBorder ? WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE : WS_EX_CONTROLPARENT)
END_DIALOG()

BEGIN_CONTROLS_MAP()
	CONTROL_LTEXT(_T("[0000]<tag name>"), IDC_ET_NAME, 5, 5, 92, 8, WS_VISIBLE | SS_ENDELLIPSIS, 0);
	CONTROL_RTEXT(_T("[0000]0/0x0000"), IDC_ET_ID, m_bBorder ? 109 : 108, 5, 37, 8, WS_VISIBLE, 0);
	CONTROL_LTEXT(_T("[0000]<tag description>"), IDC_ET_DESCRIPTION, 5, 18, m_bBorder ? 141 : 140, 77, WS_VISIBLE, 0);
END_CONTROLS_MAP()

BEGIN_COM_MAP(CDesignerViewEXIFTag)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewEXIFTag)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewEXIFTag>)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	CHAIN_MSG_MAP(CDialogResize<CDesignerViewEXIFTag>)
	REFLECT_NOTIFICATIONS()
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CDesignerViewAnimatedCursorInfo)
	DLGRESIZE_CONTROL(IDC_ET_NAME, DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_ET_ID, DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_ET_DESCRIPTION, DLSZ_SIZE_X|DLSZ_SIZE_Y)
END_DLGRESIZE_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_pFrame)
			m_pFrame->ObserverDel(CObserverImpl<CDesignerViewEXIFTag, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	// IChildWindow methods
public:

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			SIZE sz;
			if (!GetDialogSize(&sz, LANGIDFROMLCID(m_tLocaleID)))
				return E_FAIL;
			if (a_pSize->cx < sz.cx) a_pSize->cx = sz.cx;
			if (a_pSize->cy < sz.cy) a_pSize->cy = sz.cy;
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndName = GetDlgItem(IDC_ET_NAME);
		m_wndID = GetDlgItem(IDC_ET_ID);
		m_wndDesc = GetDlgItem(IDC_ET_DESCRIPTION);
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfHeight = (lf.lfHeight*5)/6;
		m_cSmallFont.CreateFontIndirect(&lf);
		m_wndDesc.SetFont(m_cSmallFont);

		DlgResize_Init(false, false, 0);

		CComPtr<ISharedState> pState;
		if (m_pFrame)
			m_pFrame->StateGet(m_bstrSelectionID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState)
			pState->ToText(&m_bstrIFDAndTag);
		Data2GUI();

		return 0;
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW)); return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }

public:
	void OwnerNotify(TCookie, TSharedStateChange a_tChange)
	{
		if (m_hWnd && m_bstrSelectionID == a_tChange.bstrName)
		{
			CComBSTR bstr;
			if (SUCCEEDED(a_tChange.pState->ToText(&bstr)) && m_bstrIFDAndTag != bstr)
			{
				m_bstrIFDAndTag = bstr;
				Data2GUI();
			}
		}
	}

	void Data2GUI()
	{
		if (m_bstrIFDAndTag)
		{
			int nIFD = -1;
			int nTag = -1;
			if (2 == swscanf(m_bstrIFDAndTag, L"%i/%i", &nIFD, &nTag))
			{
				TCHAR szTmp[32];
				_stprintf(szTmp, _T("%i/0x%04x"), nIFD, nTag);
				m_wndID.SetWindowText(szTmp);
				const char* pName = exif_tag_get_title_in_ifd(static_cast<ExifTag>(nTag), static_cast<ExifIfd>(nIFD));
				if (pName)
				{
					m_wndName.SetWindowText(CA2CT(pName));
				}
				else
				{
					m_wndDesc.SetWindowText(_T("Unknown tag"));
				}
				const char* pDesc = exif_tag_get_description_in_ifd(static_cast<ExifTag>(nTag), static_cast<ExifIfd>(nIFD));
				if (pDesc)
				{
					m_wndDesc.SetWindowText(CA2CT(pDesc));
				}
				else
				{
					m_wndDesc.SetWindowText(_T("No description is available for this tag."));
				}
				return;
			}
		}
		m_wndName.SetWindowText(_T("No tag selected"));
		m_wndID.SetWindowText(_T(""));
		m_wndDesc.SetWindowText(_T(""));
	}

private:
	bool m_bBorder;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IImageMetaData> m_pIMD;
	CComPtr<ISharedStateManager> m_pFrame;
	CComBSTR m_bstrSelectionID;
	CComBSTR m_bstrIFDAndTag;

	CStatic m_wndName;
	CStatic m_wndID;
	CStatic m_wndDesc;
	CFont m_cSmallFont;
};

