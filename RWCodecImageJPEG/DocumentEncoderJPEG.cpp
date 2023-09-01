// DocumentEncoderJPEG.cpp : Implementation of CDocumentEncoderJPEG

#include "stdafx.h"
#include "DocumentEncoderJPEG.h"

#include <MultiLanguageString.h>
#include <RWEXIF_i.h>
#include <ReturnedData.h>
#include <Resampling.h>


// CDocumentEncoderJPEG

STDMETHODIMP CDocumentEncoderJPEG::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorJPEG::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

#include "JPEGConfigIDs.h"
#include <ConfigCustomGUIImpl.h>
#include <ContextMenuWithIcons.h>
#include <XPGUI.h>
#include <vector>
#define AUTOCANVASSUPPORT
#include <IconRenderer.h>

bool LosslessSupport()
{
	//static int iLossless = -1;
	//if (iLossless == -1)
	//{
	//	CComPtr<IApplicationInfo> pAI;
	//	RWCoCreateInstance(pAI, __uuidof(ApplicationInfo));
	//	CComBSTR bstrAppID;
	//	if (pAI) pAI->Identifier(&bstrAppID);
	//	iLossless = bstrAppID == L"RWPhotos" || bstrAppID == L"RWDesigner";
	//}
	//return iLossless != 0;
	return false;
}

class ATL_NO_VTABLE CConfigGUIEncoderJPEGDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderJPEGDlg>,
	public CDialogResize<CConfigGUIEncoderJPEGDlg>,
	public CContextMenuWithIcons<CConfigGUIEncoderJPEGDlg>
{
public:
	CConfigGUIEncoderJPEGDlg() :
		m_CFGID_LOSSLESS(CFGID_LOSSLESS), m_CFGID_METADATA(CFGID_METADATA),
		m_CFGID_CHROMASUBSAMPLING(CFGID_CHROMASUBSAMPLING), m_CFGID_ARITHMETIC(CFGID_ARITHMETIC)
	{
	}
	~CConfigGUIEncoderJPEGDlg()
	{
		m_images.Destroy();
	}

	enum
	{
		IDC_CGJPEG_QUALITYEDIT = 100, IDC_CGJPEG_QUALITYSLIDER, IDC_CGJPEG_TOOLBAR,
		ID_BTN_SUBSAMPLING = 200, ID_BTN_LOSSLESS, ID_BTN_METADATA, ID_BTN_ENCODING,
		IC_CHROMA_1X1 = 0, IC_CHROMA_2X1, IC_CHROMA_1X2, IC_CHROMA_2X2,
		IC_LOSSLESS, IC_META_NONE, IC_META_ORIGINAL, IC_META_UPDATE,
		IC_ARITHMETIC
	};

	BEGIN_DIALOG_EX(0, 0, 200, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Quality:[0405]Kvalita:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGJPEG_QUALITYEDIT, 57, 0, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGJPEG_QUALITYSLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 90, 0, 10, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGJPEG_TOOLBAR, TOOLBARCLASSNAME, TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE | CCS_NOMOVEY | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | WS_TABSTOP | WS_VISIBLE, 100, 0, 100, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderJPEGDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderJPEGDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderJPEGDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGJPEG_TOOLBAR, TBN_DROPDOWN, OnDropdownClick)
		COMMAND_HANDLER(ID_BTN_ENCODING, BN_CLICKED, OnEncodingClick)
		COMMAND_HANDLER(ID_BTN_LOSSLESS, BN_CLICKED, OnLosslessClick)
		CHAIN_MSG_MAP(CContextMenuWithIcons<CConfigGUIEncoderJPEGDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderJPEGDlg)
		DLGRESIZE_CONTROL(IDC_CGJPEG_QUALITYSLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGJPEG_TOOLBAR, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderJPEGDlg)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGJPEG_QUALITYSLIDER, CFGID_QUALITY)
		CONFIGITEM_EDITBOX(IDC_CGJPEG_QUALITYEDIT, CFGID_QUALITY)
		CONFIGITEM_CONTEXTHELP_CALLBACK(IDC_CGJPEG_TOOLBAR)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wndTools = GetDlgItem(IDC_CGJPEG_TOOLBAR);
		m_wndTools.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndTools.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		int const iconSize = XPGUI::GetSmallIconSize();
		ULONG nIconDelta = (iconSize >> 1) - 8;
		ULONG nGapLeft = nIconDelta >> 1;
		ULONG nGapRight = nIconDelta - nGapLeft;
		RECT padding = { nGapLeft, 0, nGapRight, 0 };
		m_images.Create(iconSize + nIconDelta, iconSize, XPGUI::GetImageListColorFlags(), 4, 1);
		m_wndTools.SetImageList(m_images);
		InitIcons(m_images, iconSize, nIconDelta != 0 ? &padding : nullptr);

		m_chroma = CFGVAL_CH_1x1;
		m_meta = CFGVAL_MD_KEEP;
		m_lossless = LosslessSupport();
		m_ari = false;
		{
			CComBSTR bstrChroma;
			CComBSTR bstrMeta;
			CComBSTR bstrLossless;
			CComBSTR bstrArith;
			CMultiLanguageString::GetLocalized(CFGNAME_CHROMASUBSAMPLING, m_tLocaleID, &bstrChroma);
			CMultiLanguageString::GetLocalized(CFGNAME_METADATA, m_tLocaleID, &bstrMeta);
			CMultiLanguageString::GetLocalized(CFGNAME_LOSSLESS, m_tLocaleID, &bstrLossless);
			CMultiLanguageString::GetLocalized(CFGNAME_ARITHMETIC, m_tLocaleID, &bstrArith);

			TBBUTTON aButtons[] =
			{
				{IC_CHROMA_1X1, ID_BTN_SUBSAMPLING, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrChroma.m_str)},
				{IC_ARITHMETIC, ID_BTN_ENCODING, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrArith.m_str)},
				{0, 0, m_lossless ? 0 : TBSTATE_HIDDEN, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
				{IC_LOSSLESS, ID_BTN_LOSSLESS, m_lossless ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_HIDDEN, BTNS_BUTTON, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrLossless.m_str)},
				{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
				{IC_META_ORIGINAL, ID_BTN_METADATA, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_WHOLEDROPDOWN, TBBUTTON_PADDING, 0, reinterpret_cast<INT_PTR>(bstrMeta.m_str)},
			};
			m_wndTools.AddButtons(itemsof(aButtons), aButtons);
			RECT rcClient{ 0, 0, 0, 0 };
			GetClientRect(&rcClient);
			m_wndTools.SetButtonSize(iconSize + 8, min(iconSize + (iconSize >> 1) + 1, rcClient.bottom));
		}
		RECT rcSlider;
		CWindow wndSlider = GetDlgItem(IDC_CGJPEG_QUALITYSLIDER);
		wndSlider.GetWindowRect(&rcSlider);
		ScreenToClient(&rcSlider);
		RECT rcToolbar;
		m_wndTools.GetWindowRect(&rcToolbar);
		ScreenToClient(&rcToolbar);
		RECT rcItem;
		m_wndTools.GetItemRect(m_wndTools.GetButtonCount()-1, &rcItem);
		rcToolbar.top = (rcToolbar.top+rcToolbar.bottom-rcItem.bottom)>>1;
		rcToolbar.bottom = rcToolbar.top+rcItem.bottom;
		rcSlider.right = rcToolbar.left = rcToolbar.right-rcItem.right;
		m_wndTools.MoveWindow(&rcToolbar);
		wndSlider.MoveWindow(&rcSlider);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	void ExtraConfigNotify()
	{
		CConfigValue cChroma;
		M_Config()->ItemValueGet(CComBSTR(m_CFGID_CHROMASUBSAMPLING), &cChroma);
		if (cChroma.operator LONG() != m_chroma)
		{
			m_chroma = cChroma;
			int const img = m_chroma == CFGVAL_CH_1x1 ? IC_CHROMA_1X1 : (
				m_chroma == CFGVAL_CH_2x1 ? IC_CHROMA_2X1 : (
				m_chroma == CFGVAL_CH_1x2 ? IC_CHROMA_1X2 : IC_CHROMA_2X2));
			m_wndTools.SetButtonInfo(ID_BTN_SUBSAMPLING, TBIF_IMAGE, 0, 0, 0, img, 0, 0, 0);
		}
		CConfigValue cArtihm;
		M_Config()->ItemValueGet(CComBSTR(m_CFGID_ARITHMETIC), &cArtihm);
		if (cArtihm.operator bool() != m_ari)
		{
			m_ari = cArtihm;
			m_wndTools.SetButtonInfo(ID_BTN_ENCODING, TBIF_STATE, 0, m_ari ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, 0, 0, 0, 0, 0);
		}
		if (LosslessSupport())
		{
			CConfigValue cLossless;
			M_Config()->ItemValueGet(CComBSTR(m_CFGID_LOSSLESS), &cLossless);
			if ((cLossless.operator LONG() == CFGVAL_LL_ALL) != m_lossless)
			{
				m_lossless = cLossless.operator LONG() == CFGVAL_LL_ALL;
				m_wndTools.SetButtonInfo(ID_BTN_LOSSLESS, TBIF_STATE, 0, m_lossless ? TBSTATE_ENABLED|TBSTATE_CHECKED : TBSTATE_ENABLED, 0, 0, 0, 0, 0);
			}
		}
		CConfigValue cMeta;
		M_Config()->ItemValueGet(CComBSTR(m_CFGID_METADATA), &cMeta);
		if (cMeta.operator LONG() != m_meta)
		{
			m_meta = cMeta;
			int const img = m_meta == CFGVAL_MD_REMOVE ? IC_META_NONE : (m_meta == CFGVAL_MD_KEEP ? IC_META_ORIGINAL : IC_META_UPDATE);
			m_wndTools.SetButtonInfo(ID_BTN_METADATA, TBIF_IMAGE, 0, 0, 0, img, 0, 0, 0);
		}
	}

	LRESULT OnDropdownClick(WPARAM UNREF(a_wParam), LPNMHDR a_pNMHdr, BOOL& UNREF(a_bHandled))
	{
		NMTOOLBAR* pTB = reinterpret_cast<NMTOOLBAR*>(a_pNMHdr);
		CMenu cMenu;

		if (pTB->iItem == ID_BTN_SUBSAMPLING)
		{
			Reset(m_images);
			cMenu.CreatePopupMenu();
			struct SSSInfo {wchar_t const* name; wchar_t const* info; int icon; int val;};
			static SSSInfo const s_ss[] =
			{
				{OPTNAME_CH_1x1, L" (4:4:4)", IC_CHROMA_1X1, CFGVAL_CH_1x1},
				{OPTNAME_CH_1x2, L" (4:4:0)", IC_CHROMA_1X2, CFGVAL_CH_1x2},
				{OPTNAME_CH_2x1, L" (4:2:2)", IC_CHROMA_2X1, CFGVAL_CH_2x1},
				{OPTNAME_CH_2x2, L" (4:2:0)", IC_CHROMA_2X2, CFGVAL_CH_2x2},
			};
			for (SSSInfo const* p = s_ss; p < s_ss+sizeof(s_ss)/sizeof(*s_ss); ++p)
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(p->name, m_tLocaleID, &bstr);
				bstr += p->info;
				AddItem(cMenu, p->val, bstr.m_str, p->icon, m_chroma == p->val ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
			}
		}
		else if (pTB->iItem == ID_BTN_METADATA)
		{
			Reset(m_images);
			cMenu.CreatePopupMenu();
			struct SMInfo {wchar_t const* name; int icon; int val;};
			static SMInfo const s_m[] =
			{
				{OPTNAME_MD_REMOVE, IC_META_NONE, CFGVAL_MD_REMOVE},
				{OPTNAME_MD_KEEP, IC_META_ORIGINAL, CFGVAL_MD_KEEP},
				{OPTNAME_MD_UPDATE, IC_META_UPDATE, CFGVAL_MD_UPDATE},
			};
			for (SMInfo const* p = s_m; p < s_m+sizeof(s_m)/sizeof(*s_m); ++p)
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(p->name, m_tLocaleID, &bstr);
				AddItem(cMenu, p->val+1, bstr.m_str, p->icon, m_meta == p->val ? MFT_RADIOCHECK|MFS_CHECKED : MFT_RADIOCHECK);
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

		if (pTB->iItem == ID_BTN_SUBSAMPLING)
		{
			M_Config()->ItemValuesSet(1, &(m_CFGID_CHROMASUBSAMPLING.m_str), CConfigValue(LONG(nSelection)));
		}
		else if (pTB->iItem == ID_BTN_METADATA)
		{
			M_Config()->ItemValuesSet(1, &(m_CFGID_METADATA.m_str), CConfigValue(LONG(nSelection-1)));
		}

		return TBDDRET_DEFAULT;
	}

	LRESULT OnEncodingClick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		M_Config()->ItemValuesSet(1, &(m_CFGID_ARITHMETIC.m_str), CConfigValue(!m_ari));
		return 0;
	}

	LRESULT OnLosslessClick(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		M_Config()->ItemValuesSet(1, &(m_CFGID_LOSSLESS.m_str), CConfigValue(m_lossless ? CFGVAL_LL_NEVER : CFGVAL_LL_ALL));
		return 0;
	}

	void GetHelpString(UINT UNREF(iCtrlId), POINT tMousePos, wchar_t* pszBuffer, size_t nBuffer)
	{
		m_wndTools.ScreenToClient(&tMousePos);
		struct SMap {int index; wchar_t const* desc;};
		static SMap s_map[] =
		{
			{0, CFGDESC_CHROMASUBSAMPLING},
			{1, CFGDESC_ARITHMETIC},
			{3, CFGDESC_LOSSLESS},
			{5, CFGDESC_METADATA},
		};
		wchar_t const* d = NULL;
		for (int i = 0; i < 4; ++i)
		{
			RECT rc;
			m_wndTools.GetItemRect(s_map[i].index, &rc);
			if (tMousePos.x >= rc.left && tMousePos.y >= rc.top && tMousePos.x < rc.right && tMousePos.y < rc.bottom)
			{
				d = s_map[i].desc;
				break;
			}
		}
		if (d)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(d, m_tLocaleID, &bstr);
			if (bstr.Length())
			{
				wcsncpy(pszBuffer, bstr, nBuffer);
			}
		}
		pszBuffer[nBuffer-1] = L'\0';
	}

	static void InitIcons(CImageList& images, int const iconSize, RECT const* padding)
	{
		IRPolyPoint p1x1[] = { {0.25f, 0.25f}, {0.25f, 0.75f}, {0.75f, 0.75f}, {0.75f, 0.25f} };
		IRPolyPoint p2x1_1[] = { {0.4f, 0.3f}, {0.4f, 0.7f}, {0.0f, 0.7f}, {0.0f, 0.3f} };
		IRPolyPoint p2x1_2[] = { {0.6f, 0.3f}, {1.0f, 0.3f}, {1.0f, 0.7f}, {0.6f, 0.7f} };
		IRPolygon p2x1[] = { {itemsof(p2x1_1), p2x1_1}, {itemsof(p2x1_2), p2x1_2} };
		IRPolyPoint p1x2_1[] = { {0.3f, 0.4f}, {0.7f, 0.4f}, {0.7f, 0.0f}, {0.3f, 0.0f} };
		IRPolyPoint p1x2_2[] = { {0.3f, 0.6f}, {0.3f, 1.0f}, {0.7f, 1.0f}, {0.7f, 0.6f} };
		IRPolygon p1x2[] = { {itemsof(p1x2_1), p1x2_1}, {itemsof(p1x2_2), p1x2_2} };
		IRPolyPoint p2x2_1[] = { {0.4f, 0.4f}, {0.0f, 0.4f}, {0.0f, 0.0f}, {0.4f, 0.0f} };
		IRPolyPoint p2x2_2[] = { {0.6f, 0.4f}, {0.6f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.4f} };
		IRPolyPoint p2x2_3[] = { {0.4f, 0.6f}, {0.4f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.6f} };
		IRPolyPoint p2x2_4[] = { {0.6f, 0.6f}, {1.0f, 0.6f}, {1.0f, 1.0f}, {0.6f, 1.0f} };
		IRPolygon p2x2[] = { {itemsof(p2x2_1), p2x2_1}, {itemsof(p2x2_2), p2x2_2}, {itemsof(p2x2_3), p2x2_3}, {itemsof(p2x2_4), p2x2_4} };
		IRPolyPoint arth1[16+3*4] = {{0.45f, 0.45f}, {0.45f, 0.3f}, {0.55f, 0.3f}, {0.55f, 0.45f}, {0.7f, 0.45f}, {0.7f, 0.55f}, {0.55f, 0.55f}, {0.55f, 0.7f}, {0.45f, 0.7f}, {0.45f, 0.55f}, {0.3f, 0.55f}, {0.3f, 0.45f} };
		for (int i = 0; i < 16; ++i)
		{
			arth1[i+3*4].x = 0.5f+0.35f*sinf(i*3.1415927f/8.0f);
			arth1[i+3*4].y = 0.5f+0.35f*cosf(i*3.1415927f/8.0f);
		}
		IRPolygon arth[] = { {3*4, arth1}, {16, arth1+3*4} };
		IRPolyPoint loss[] = {{0.5f, 0.8f}, {0.9f, 0.4f}, {0.7f, 0.2f}, {0.3f, 0.2f}, {0.1f, 0.4f}};
		IRPolyPoint none[] = {{0.5f, 0.4f}, {0.7f, 0.2f}, {0.8f, 0.3f}, {0.6f, 0.5f}, {0.8f, 0.7f}, {0.7f, 0.8f}, {0.5f, 0.6f}, {0.3f, 0.8f}, {0.2f, 0.7f}, {0.4f, 0.5f}, {0.2f, 0.3f}, {0.3f, 0.2f}};
		IRPolyPoint save[8*2];
		for (int i = 0; i < 8; ++i)
		{
			save[i].x = (i+0.7f)/9.0f;
			save[i].y = 0.4f+0.2f*cosf((i+2.0f)*3.1415927f/4.0f);
			save[15-i].x = (i+1.3f)/9.0f;
			save[15-i].y = 0.6f+0.2f*cosf((i+3.0f)*3.1415927f/4.0f);
		}
		IRPolyPoint updt[8*2];
		for (int i = 0; i < 8; ++i)
		{
			updt[i+i].x = 0.5f+0.4f*sinf(i*3.1415927f/4.0f);
			updt[i+i].y = 0.5f+0.4f*cosf(i*3.1415927f/4.0f);
			updt[i+i+1].x = 0.5f+0.2f*sinf((i+0.5f)*3.1415927f/4.0f);
			updt[i+i+1].y = 0.5f+0.2f*cosf((i+0.5f)*3.1415927f/4.0f);
		}
		struct SPolyList { IRPolyPoint const* p1; IRPolygon const* p2; int n; bool z; };
		SPolyList const s_polys[] =
		{
			{p1x1, nullptr, sizeof(p1x1)/sizeof(*p1x1), false},	// IC_CHROMA_1X1
			{nullptr, p2x1, sizeof(p2x1)/sizeof(*p2x1), true},	// IC_CHROMA_2X1
			{nullptr, p1x2, sizeof(p1x2)/sizeof(*p1x2), true},	// IC_CHROMA_1X2
			{nullptr, p2x2, sizeof(p2x2)/sizeof(*p2x2), true},	// IC_CHROMA_2X2
			{loss, nullptr, sizeof(loss)/sizeof(*loss), false},	// IC_LOSSLESS
			{none, nullptr, sizeof(none)/sizeof(*none), false},	// IC_META_NONE
			{save, nullptr, sizeof(save)/sizeof(*save), false},	// IC_META_ORIGINAL
			{updt, nullptr, sizeof(updt)/sizeof(*updt), false},	// IC_META_UPDATE
			{nullptr, arth, sizeof(arth)/sizeof(*arth), true},	// IC_ARITHMETIC
		};
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CComPtr<IIconRenderer> pIR;
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));
		for (SPolyList const* p = s_polys; p < s_polys+sizeof(s_polys)/sizeof(*s_polys); ++p)
		{
			HICON h = p->p1 ? IconFromPolygon(pSI, pIR, p->n, p->p1, iconSize, p->z, padding) : IconFromPolygon(pSI, pIR, p->n, p->p2, iconSize, p->z, padding);
			images.AddIcon(h);
			DestroyIcon(h);
		}
	}

private:
	CToolBarCtrl m_wndTools;
	CImageList m_images;

	LONG m_chroma;
	LONG m_meta;
	bool m_lossless;
	bool m_ari;

	CComBSTR m_CFGID_LOSSLESS;
	CComBSTR m_CFGID_METADATA;
	CComBSTR m_CFGID_CHROMASUBSAMPLING;
	CComBSTR m_CFGID_ARITHMETIC;
};

IConfig* CDocumentEncoderJPEG::Config()
{
	CComPtr<IConfigWithDependencies> pCfgInit;
	RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

	CComBSTR cCFGID_LOSSLESS(CFGID_LOSSLESS);
	pCfgInit->ItemIns1ofN(cCFGID_LOSSLESS, CMultiLanguageString::GetAuto(CFGNAME_LOSSLESS), CMultiLanguageString::GetAuto(CFGDESC_LOSSLESS), CConfigValue(LosslessSupport() ? CFGVAL_LL_ALL : CFGVAL_LL_NEVER), NULL);
	pCfgInit->ItemOptionAdd(cCFGID_LOSSLESS, CConfigValue(CFGVAL_LL_NEVER), CMultiLanguageString::GetAuto(L"[0409]Disabled[0405]Zakázáno"), 0, NULL);
	if (LosslessSupport())
	{
		pCfgInit->ItemOptionAdd(cCFGID_LOSSLESS, CConfigValue(CFGVAL_LL_SIMPLE), CMultiLanguageString::GetAuto(L"[0409]Only for not-rotated images[0405]Jen pro neotočené obrázky"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_LOSSLESS, CConfigValue(CFGVAL_LL_ALL), CMultiLanguageString::GetAuto(L"[0409]Enabled[0405]Povoleno"), 0, NULL);
	}

	CComBSTR cCFGID_QUALITY(CFGID_QUALITY);
	pCfgInit->ItemInsRanged(cCFGID_QUALITY, CMultiLanguageString::GetAuto(CFGNAME_QUALITY), CMultiLanguageString::GetAuto(CFGDESC_QUALITY), CConfigValue(85L), NULL, CConfigValue(0L), CConfigValue(100L), CConfigValue(1L), 0, NULL);

	CComBSTR cCFGID_METADATA(CFGID_METADATA);
	pCfgInit->ItemIns1ofN(cCFGID_METADATA, CMultiLanguageString::GetAuto(CFGNAME_METADATA), CMultiLanguageString::GetAuto(CFGDESC_METADATA), CConfigValue(CFGVAL_MD_KEEP), NULL);
	pCfgInit->ItemOptionAdd(cCFGID_METADATA, CConfigValue(CFGVAL_MD_REMOVE), CMultiLanguageString::GetAuto(OPTNAME_MD_REMOVE), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_METADATA, CConfigValue(CFGVAL_MD_KEEP), CMultiLanguageString::GetAuto(OPTNAME_MD_KEEP), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_METADATA, CConfigValue(CFGVAL_MD_UPDATE), CMultiLanguageString::GetAuto(OPTNAME_MD_UPDATE), 0, NULL);

	CComBSTR cCFGID_CHROMASUBSAMPLING(CFGID_CHROMASUBSAMPLING);
	pCfgInit->ItemIns1ofN(cCFGID_CHROMASUBSAMPLING, CMultiLanguageString::GetAuto(CFGNAME_CHROMASUBSAMPLING), CMultiLanguageString::GetAuto(CFGDESC_CHROMASUBSAMPLING), CConfigValue(CFGVAL_CH_2x2), NULL);
	pCfgInit->ItemOptionAdd(cCFGID_CHROMASUBSAMPLING, CConfigValue(CFGVAL_CH_1x1), CMultiLanguageString::GetAuto(OPTNAME_CH_1x1), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_CHROMASUBSAMPLING, CConfigValue(CFGVAL_CH_1x2), CMultiLanguageString::GetAuto(OPTNAME_CH_1x2), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_CHROMASUBSAMPLING, CConfigValue(CFGVAL_CH_2x1), CMultiLanguageString::GetAuto(OPTNAME_CH_2x1), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_CHROMASUBSAMPLING, CConfigValue(CFGVAL_CH_2x2), CMultiLanguageString::GetAuto(OPTNAME_CH_2x2), 0, NULL);

	pCfgInit->ItemInsSimple(CComBSTR(CFGID_ARITHMETIC), CMultiLanguageString::GetAuto(CFGNAME_ARITHMETIC), CMultiLanguageString::GetAuto(CFGDESC_ARITHMETIC), CConfigValue(false), NULL, 0, NULL);

	CConfigCustomGUI<&CLSID_DocumentEncoderJPEG, CConfigGUIEncoderJPEGDlg>::FinalizeConfig(pCfgInit);

	return pCfgInit.Detach();
}

STDMETHODIMP CDocumentEncoderJPEG::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		*a_ppDefCfg = Config();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderJPEG::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_META);
		return pDocImg ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "JPEGUtils.h"
enum ECompareTile
{
	ECTDifferent = 0,
	ECTEqual,
	ECTTrivial, // single colored tile
	ECTRot90,
	ECTRot180,
	ECTRot270,
};
#define IGNORE_TRIVIAL 1

inline ECompareTile CompareTile(ULONG a_nSizeX, ULONG a_nSizeY,
								BYTE const* a_pImage1, ULONG a_nStride1,
								BYTE const* a_pImage2, ULONG a_nStride2)
{
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
		}
		a_pImage1 += a_nStride1-3*a_nSizeX;
		a_pImage2 += a_nStride2-3*a_nSizeX;
	}
#ifdef IGNORE_TRIVIAL
	return ECTEqual;
#else
	a_pImage1 -= a_nStride1*a_nSizeY;
	a_pImage2 -= a_nStride2*a_nSizeY;
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage1++) != a_pImage2[0]) return ECTEqual;
			if (*(a_pImage1++) != a_pImage2[1]) return ECTEqual;
			if (*(a_pImage1++) != a_pImage2[2]) return ECTEqual;
		}
	}
	return ECTTrivial;
#endif
}

inline ECompareTile CompareTile180(ULONG a_nSizeX, ULONG a_nSizeY,
								   BYTE const* a_pImage1, ULONG a_nStride1,
								   BYTE const* a_pImage2, ULONG a_nStride2)
{
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*a_pImage1 != *(a_pImage2++)) return ECTDifferent;
			a_pImage1-=5;
		}
		a_pImage1 -= a_nStride1-3*a_nSizeX;
		a_pImage2 += a_nStride2-3*a_nSizeX;
	}
#ifdef IGNORE_TRIVIAL
	return ECTEqual;
#else
	a_pImage1 += a_nStride1*a_nSizeY;
	a_pImage2 -= a_nStride2*a_nSizeY;
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage2++) != a_pImage1[0]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[1]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[2]) return ECTEqual;
		}
	}
	return ECTTrivial;
#endif
}

inline ECompareTile CompareTile90(ULONG a_nSizeX, ULONG a_nSizeY,
								  BYTE const* a_pImage1, ULONG a_nStride1,
								  BYTE const* a_pImage2, ULONG a_nStride2)
{
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*a_pImage1 != *(a_pImage2++)) return ECTDifferent;
			a_pImage1 -= 2+a_nStride1;
		}
		a_pImage1 += a_nStride1*a_nSizeX+3;
		a_pImage2 += a_nStride2-3*a_nSizeX;
	}
#ifdef IGNORE_TRIVIAL
	return ECTEqual;
#else
	a_pImage1 -= 3*a_nSizeY;
	a_pImage2 -= a_nStride2*a_nSizeY;
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage2++) != a_pImage1[0]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[1]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[2]) return ECTEqual;
		}
	}
	return ECTTrivial;
#endif
}

inline ECompareTile CompareTile270(ULONG a_nSizeX, ULONG a_nSizeY,
								   BYTE const* a_pImage1, ULONG a_nStride1,
								   BYTE const* a_pImage2, ULONG a_nStride2)
{
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*(a_pImage1++) != *(a_pImage2++)) return ECTDifferent;
			if (*a_pImage1 != *(a_pImage2++)) return ECTDifferent;
			a_pImage1 += a_nStride1-2;
		}
		a_pImage1 -= a_nStride1*a_nSizeX+3;
		a_pImage2 += a_nStride2-3*a_nSizeX;
	}
#ifdef IGNORE_TRIVIAL
	return ECTEqual;
#else
	a_pImage1 += 3*a_nSizeY;
	a_pImage2 -= a_nStride2*a_nSizeY;
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			if (*(a_pImage2++) != a_pImage1[0]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[1]) return ECTEqual;
			if (*(a_pImage2++) != a_pImage1[2]) return ECTEqual;
		}
	}
	return ECTTrivial;
#endif
}


static DWORD const g_aCrc32Table[256] =
{
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};

void ComputeHashes(ULONG a_nSizeX, ULONG a_nSizeY, BYTE const* a_pImage, ULONG a_nStride, ULONG a_nTileSizeX, ULONG a_nTileSizeY, DWORD* a_pHashes)
{
	for (ULONG y = 0; y < a_nSizeY; y += a_nTileSizeY)
	{
		ULONG nDY = min(a_nSizeY-y, a_nTileSizeY);
		for (ULONG x = 0; x < a_nSizeX; x += a_nTileSizeX)
		{
			ULONG nDX = min(a_nSizeX-x, a_nTileSizeX);
			*a_pHashes = 0;
			for (ULONG yy = 0; yy < nDY; ++yy)
			{
				BYTE const* p = a_pImage + (y+yy)*a_nStride + x*3;
				for (ULONG xx = 0; xx < nDX; ++xx)
				{
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
				}
			}
			++a_pHashes;
		}
	}
}

void ComputeHashes(ULONG a_nSizeX, ULONG a_nSizeY, BYTE const* a_pImage, ULONG a_nStride, ULONG a_nTileSizeX, ULONG a_nTileSizeY, DWORD* a_pHashes, DWORD* a_pHashes90, DWORD* a_pHashes180, DWORD* a_pHashes270)
{
	for (ULONG y = 0; y < a_nSizeY; y += a_nTileSizeY)
	{
		ULONG nDY = min(a_nSizeY-y, a_nTileSizeY);
		for (ULONG x = 0; x < a_nSizeX; x += a_nTileSizeX)
		{
			BYTE const* pTile = a_pImage + y*a_nStride + x*3;
			ULONG nDX = min(a_nSizeX-x, a_nTileSizeX);
			*a_pHashes = 0;
			for (ULONG yy = 0; yy < nDY; ++yy)
			{
				BYTE const* p = pTile + yy*a_nStride;
				for (ULONG xx = 0; xx < nDX; ++xx)
				{
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
					*a_pHashes = ((*a_pHashes) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes) & 0x000000FF)];
					++p;
				}
			}
			++a_pHashes;
			// 180
			*a_pHashes180 = 0;
			for (int yy = nDY-1; yy >= 0; --yy)
			{
				BYTE const* p = pTile + yy*a_nStride + (nDX-1)*3;
				for (ULONG xx = 0; xx < nDX; ++xx)
				{
					*a_pHashes180 = ((*a_pHashes180) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes180) & 0x000000FF)];
					++p;
					*a_pHashes180 = ((*a_pHashes180) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes180) & 0x000000FF)];
					++p;
					*a_pHashes180 = ((*a_pHashes180) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes180) & 0x000000FF)];
					p-=5;
				}
			}
			++a_pHashes180;
			// 90 - to the right
			*a_pHashes90 = 0;
			for (ULONG xx = 0; xx < nDX; ++xx)
			{
				BYTE const* p = pTile + (nDY-1)*a_nStride + xx*3;
				for (ULONG yy = 0; yy < nDY; ++yy)
				{
					*a_pHashes90 = ((*a_pHashes90) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes90) & 0x000000FF)];
					++p;
					*a_pHashes90 = ((*a_pHashes90) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes90) & 0x000000FF)];
					++p;
					*a_pHashes90 = ((*a_pHashes90) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes90) & 0x000000FF)];
					p-=2+a_nStride;
				}
			}
			++a_pHashes90;
			// 270 - to the left
			*a_pHashes270 = 0;
			for (int xx = nDX-1; xx >= 0; --xx)
			{
				BYTE const* p = pTile + xx*3;
				for (ULONG yy = 0; yy < nDY; ++yy)
				{
					*a_pHashes270 = ((*a_pHashes270) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes270) & 0x000000FF)];
					++p;
					*a_pHashes270 = ((*a_pHashes270) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes270) & 0x000000FF)];
					++p;
					*a_pHashes270 = ((*a_pHashes270) >> 8) ^ g_aCrc32Table[(*p) ^ ((*a_pHashes270) & 0x000000FF)];
					p+=a_nStride-2;
				}
			}
			++a_pHashes270;
		}
	}
}

ECompareTile CompareTiles(ULONG a_nSize1X, ULONG a_nSize1Y,
						  BYTE const* a_pImage1, ULONG a_nStride1,
						  ULONG a_nSize2X, ULONG a_nSize2Y,
						  BYTE const* a_pImage2, ULONG a_nStride2,
						  ULONG a_nTileSizeX, ULONG a_nTileSizeY,
						  int* a_pMapping)
{
	ULONG const nTiles1X = (a_nSize1X+a_nTileSizeX-1)/a_nTileSizeX;
	ULONG const nTiles1Y = (a_nSize1Y+a_nTileSizeY-1)/a_nTileSizeY;
	ULONG const nTiles2X = (a_nSize2X+a_nTileSizeX-1)/a_nTileSizeX;
	ULONG const nTiles2Y = (a_nSize2Y+a_nTileSizeY-1)/a_nTileSizeY;
	CAutoVectorPtr<DWORD> cHashes1(new DWORD[nTiles1X*nTiles1Y]);
	CAutoVectorPtr<DWORD> cHashes2(new DWORD[nTiles2X*nTiles2Y]);
	ComputeHashes(a_nSize1X, a_nSize1Y, a_pImage1, a_nStride1, a_nTileSizeX, a_nTileSizeY, cHashes1);
	ComputeHashes(a_nSize2X, a_nSize2Y, a_pImage2, a_nStride2, a_nTileSizeX, a_nTileSizeY, cHashes2);
	std::multimap<DWORD, int> cSourceTiles;
	for (ULONG i = 0; i < nTiles1X*nTiles1Y; ++i)
		cSourceTiles.insert(std::make_pair(cHashes1[i], i));

	ECompareTile eCT = ECTTrivial;
	for (ULONG y = 0; y < nTiles2Y; ++y)
	{
		ULONG const nTileSizeY = (y+1)*a_nTileSizeY > a_nSize2Y ? a_nSize2Y%a_nTileSizeY : a_nTileSizeY;
		for (ULONG x = 0; x < nTiles2X; ++x)
		{
			*a_pMapping = -1;
			ULONG const nTileSizeX = (x+1)*a_nTileSizeX > a_nSize2X ? a_nSize2X%a_nTileSizeX : a_nTileSizeX;
			std::pair<std::multimap<DWORD, int>::const_iterator, std::multimap<DWORD, int>::const_iterator> sEqual = cSourceTiles.equal_range(cHashes2[x+nTiles2X*y]);
			for (std::multimap<DWORD, int>::const_iterator i = sEqual.first; i != sEqual.second; ++i)
			{
				// verify if this tile is really equivalent
				ULONG nSrcTileY = i->second/nTiles1X;
				ULONG nSrcTileX = i->second - nSrcTileY*nTiles1X;
				ULONG nSrcSizeX = min(a_nSize1X-nSrcTileX*a_nTileSizeX, a_nTileSizeX);
				ULONG nSrcSizeY = min(a_nSize1Y-nSrcTileY*a_nTileSizeY, a_nTileSizeY);
				if (nSrcSizeX < nTileSizeX || nSrcSizeY < nTileSizeY)
					continue; // source tile fraction is smaller than the required tile
				ECompareTile eCT1 = CompareTile(min(nTileSizeX, nSrcSizeX), min(nTileSizeY, nSrcSizeY), a_pImage1 + a_nTileSizeY*nSrcTileY*a_nStride1 + a_nTileSizeX*nSrcTileX*3, a_nStride1, a_pImage2 + a_nTileSizeY*y*a_nStride2 + a_nTileSizeX*x*3, a_nStride2);
				if (eCT1 == ECTEqual)
				{
					*a_pMapping = i->second;
					eCT = ECTEqual;
					break;
				}
				else if (eCT1 == ECTDifferent)
				{
					if (eCT == ECTTrivial)
						eCT = ECTDifferent;
				}
			}
			++a_pMapping;
		}
	}
	return eCT;
}

ECompareTile CompareTilesRot(ULONG a_nSize1X, ULONG a_nSize1Y,
							 BYTE const* a_pImage1, ULONG a_nStride1,
							 ULONG a_nSize2X, ULONG a_nSize2Y,
							 BYTE const* a_pImage2, ULONG a_nStride2,
							 ULONG a_nTileSizeX, ULONG a_nTileSizeY,
							 int* a_pMapping)
{
	ULONG const nTiles1X = (a_nSize1X+a_nTileSizeX-1)/a_nTileSizeX;
	ULONG const nTiles1Y = (a_nSize1Y+a_nTileSizeY-1)/a_nTileSizeY;
	ULONG const nTiles2X = (a_nSize2X+a_nTileSizeX-1)/a_nTileSizeX;
	ULONG const nTiles2Y = (a_nSize2Y+a_nTileSizeY-1)/a_nTileSizeY;
	ULONG const nTiles2RX = (a_nSize2X+a_nTileSizeY-1)/a_nTileSizeY;
	ULONG const nTiles2RY = (a_nSize2Y+a_nTileSizeX-1)/a_nTileSizeX;
	CAutoVectorPtr<DWORD> cHashes1(new DWORD[nTiles1X*nTiles1Y]);
	CAutoVectorPtr<DWORD> cHashes190(new DWORD[nTiles1X*nTiles1Y]);
	CAutoVectorPtr<DWORD> cHashes1180(new DWORD[nTiles1X*nTiles1Y]);
	CAutoVectorPtr<DWORD> cHashes1270(new DWORD[nTiles1X*nTiles1Y]);
	CAutoVectorPtr<DWORD> cHashes2(new DWORD[nTiles2X*nTiles2Y]);
	CAutoVectorPtr<DWORD> cHashes2R(new DWORD[nTiles2RX*nTiles2RY]);
	ComputeHashes(a_nSize1X, a_nSize1Y, a_pImage1, a_nStride1, a_nTileSizeX, a_nTileSizeY, cHashes1, cHashes190, cHashes1180, cHashes1270);
	ComputeHashes(a_nSize2X, a_nSize2Y, a_pImage2, a_nStride2, a_nTileSizeX, a_nTileSizeY, cHashes2);
	ComputeHashes(a_nSize2X, a_nSize2Y, a_pImage2, a_nStride2, a_nTileSizeY, a_nTileSizeX, cHashes2R);
	std::multimap<DWORD, int> cSourceTiles;
	for (ULONG i = 0; i < nTiles1X*nTiles1Y; ++i)
		cSourceTiles.insert(std::make_pair(cHashes1[i], i));
	CAutoVectorPtr<int> cMapping(new int[nTiles2X*nTiles2Y]);
	int* pMapping = cMapping;

	int nSame = 0;
	int nTrivial = 0;
	int nTotal = 0;
	for (ULONG y = 0; y < nTiles2Y; ++y)
	{
		ULONG const nTileSizeY = (y+1)*a_nTileSizeY > a_nSize2Y ? a_nSize2Y%a_nTileSizeY : a_nTileSizeY;
		for (ULONG x = 0; x < nTiles2X; ++x)
		{
			*pMapping = -1;
			ULONG const nTileSizeX = (x+1)*a_nTileSizeX > a_nSize2X ? a_nSize2X%a_nTileSizeX : a_nTileSizeX;
			std::pair<std::multimap<DWORD, int>::const_iterator, std::multimap<DWORD, int>::const_iterator> sEqual = cSourceTiles.equal_range(cHashes2[x+nTiles2X*y]);
			for (std::multimap<DWORD, int>::const_iterator i = sEqual.first; i != sEqual.second; ++i)
			{
				// verify if this tile is really equivalent
				ULONG nSrcTileY = i->second/nTiles1X;
				ULONG nSrcTileX = i->second - nSrcTileY*nTiles1X;
				ULONG nSrcSizeX = min(a_nSize1X-nSrcTileX*a_nTileSizeX, a_nTileSizeX);
				ULONG nSrcSizeY = min(a_nSize1Y-nSrcTileY*a_nTileSizeY, a_nTileSizeY);
				if (nSrcSizeX < nTileSizeX || nSrcSizeY < nTileSizeY)
					continue; // source tile fraction is smaller than the required tile
				ECompareTile eCT1 = CompareTile(min(nTileSizeX, nSrcSizeX), min(nTileSizeY, nSrcSizeY), a_pImage1 + a_nTileSizeY*nSrcTileY*a_nStride1 + a_nTileSizeX*nSrcTileX*3, a_nStride1, a_pImage2 + a_nTileSizeY*y*a_nStride2 + a_nTileSizeX*x*3, a_nStride2);
				if (eCT1 == ECTEqual)
				{
					*pMapping = i->second;
					++nSame;
					break;
				}
				else if (eCT1 == ECTTrivial)
				{
					++nTrivial;
					break;
				}
			}
			++pMapping;
			++nTotal;
		}
	}

	// 180
	cSourceTiles.clear();
	for (ULONG i = 0; i < nTiles1X*nTiles1Y; ++i)
		cSourceTiles.insert(std::make_pair(cHashes1180[i], i));
	CAutoVectorPtr<int> cMapping180(new int[nTiles2X*nTiles2Y]);
	pMapping = cMapping180;

	int nSame180 = 0;
	int nTrivial180 = 0;
	for (ULONG y = 0; y < nTiles2Y; ++y)
	{
		ULONG const nTileSizeY = (y+1)*a_nTileSizeY > a_nSize2Y ? a_nSize2Y%a_nTileSizeY : a_nTileSizeY;
		for (ULONG x = 0; x < nTiles2X; ++x)
		{
			*pMapping = -1;
			ULONG const nTileSizeX = (x+1)*a_nTileSizeX > a_nSize2X ? a_nSize2X%a_nTileSizeX : a_nTileSizeX;
			std::pair<std::multimap<DWORD, int>::const_iterator, std::multimap<DWORD, int>::const_iterator> sEqual = cSourceTiles.equal_range(cHashes2[x+nTiles2X*y]);
			for (std::multimap<DWORD, int>::const_iterator i = sEqual.first; i != sEqual.second; ++i)
			{
				// verify if this tile is really equivalent
				ULONG nSrcTileY = i->second/nTiles1X;
				ULONG nSrcTileX = i->second - nSrcTileY*nTiles1X;
				ULONG nSrcSizeX = min(a_nSize1X-nSrcTileX*a_nTileSizeX, a_nTileSizeX);
				ULONG nSrcSizeY = min(a_nSize1Y-nSrcTileY*a_nTileSizeY, a_nTileSizeY);
				if (nSrcSizeX < a_nTileSizeX || nSrcSizeY < a_nTileSizeY)
					continue; // when rotating only full source tiles can be used
				ECompareTile eCT1 = CompareTile180(min(nTileSizeX, nSrcSizeX), min(nTileSizeY, nSrcSizeY), a_pImage1 + (a_nTileSizeY*nSrcTileY+a_nTileSizeY-1)*a_nStride1 + (a_nTileSizeX*nSrcTileX+a_nTileSizeX-1)*3, a_nStride1, a_pImage2 + a_nTileSizeY*y*a_nStride2 + a_nTileSizeX*x*3, a_nStride2);
				if (eCT1 == ECTEqual)
				{
					*pMapping = i->second;
					++nSame180;
					break;
				}
				else if (eCT1 == ECTTrivial)
				{
					++nTrivial180;
					break;
				}
			}
			++pMapping;
		}
	}

	// 90
	cSourceTiles.clear();
	for (ULONG i = 0; i < nTiles1X*nTiles1Y; ++i)
		cSourceTiles.insert(std::make_pair(cHashes190[i], i));
	CAutoVectorPtr<int> cMapping90(new int[nTiles2RX*nTiles2RY]);
	pMapping = cMapping90;

	int nSame90 = 0;
	int nTrivial90 = 0;
	for (ULONG y = 0; y < nTiles2RY; ++y)
	{
		ULONG const nTileSizeY = (y+1)*a_nTileSizeX > a_nSize2Y ? a_nSize2Y%a_nTileSizeX : a_nTileSizeX;
		for (ULONG x = 0; x < nTiles2RX; ++x)
		{
			*pMapping = -1;
			ULONG const nTileSizeX = (x+1)*a_nTileSizeY > a_nSize2X ? a_nSize2X%a_nTileSizeY : a_nTileSizeY;
			std::pair<std::multimap<DWORD, int>::const_iterator, std::multimap<DWORD, int>::const_iterator> sEqual = cSourceTiles.equal_range(cHashes2R[x+nTiles2RX*y]);
			for (std::multimap<DWORD, int>::const_iterator i = sEqual.first; i != sEqual.second; ++i)
			{
				// verify if this tile is really equivalent
				ULONG nSrcTileY = i->second/nTiles1X;
				ULONG nSrcTileX = i->second - nSrcTileY*nTiles1X;
				ULONG nSrcSizeX = min(a_nSize1X-nSrcTileX*a_nTileSizeX, a_nTileSizeX);
				ULONG nSrcSizeY = min(a_nSize1Y-nSrcTileY*a_nTileSizeY, a_nTileSizeY);
				if (nSrcSizeX < a_nTileSizeX || nSrcSizeY < a_nTileSizeY)
					continue; // when rotating only full source tiles can be used
				ECompareTile eCT1 = CompareTile90(min(nTileSizeX, nSrcSizeY), min(nTileSizeY, nSrcSizeX), a_pImage1 + (a_nTileSizeY*nSrcTileY+a_nTileSizeY-1)*a_nStride1 + (a_nTileSizeX*nSrcTileX)*3, a_nStride1, a_pImage2 + a_nTileSizeX*y*a_nStride2 + a_nTileSizeY*x*3, a_nStride2);
				if (eCT1 == ECTEqual)
				{
					*pMapping = i->second;
					++nSame90;
					break;
				}
				else if (eCT1 == ECTTrivial)
				{
					++nTrivial90;
					break;
				}
			}
			++pMapping;
		}
	}


	// 270
	cSourceTiles.clear();
	for (ULONG i = 0; i < nTiles1X*nTiles1Y; ++i)
		cSourceTiles.insert(std::make_pair(cHashes1270[i], i));
	CAutoVectorPtr<int> cMapping270(new int[nTiles2RX*nTiles2RY]);
	pMapping = cMapping270;

	int nSame270 = 0;
	int nTrivial270 = 0;
	for (ULONG y = 0; y < nTiles2RY; ++y)
	{
		ULONG const nTileSizeY = (y+1)*a_nTileSizeX > a_nSize2Y ? a_nSize2Y%a_nTileSizeX : a_nTileSizeX;
		for (ULONG x = 0; x < nTiles2RX; ++x)
		{
			*pMapping = -1;
			ULONG const nTileSizeX = (x+1)*a_nTileSizeY > a_nSize2X ? a_nSize2X%a_nTileSizeY : a_nTileSizeY;
			std::pair<std::multimap<DWORD, int>::const_iterator, std::multimap<DWORD, int>::const_iterator> sEqual = cSourceTiles.equal_range(cHashes2R[x+nTiles2RX*y]);
			for (std::multimap<DWORD, int>::const_iterator i = sEqual.first; i != sEqual.second; ++i)
			{
				// verify if this tile is really equivalent
				ULONG nSrcTileY = i->second/nTiles1X;
				ULONG nSrcTileX = i->second - nSrcTileY*nTiles1X;
				ULONG nSrcSizeX = min(a_nSize1X-nSrcTileX*a_nTileSizeX, a_nTileSizeX);
				ULONG nSrcSizeY = min(a_nSize1Y-nSrcTileY*a_nTileSizeY, a_nTileSizeY);
				if (nSrcSizeX < a_nTileSizeX || nSrcSizeY < a_nTileSizeY)
					continue; // when rotating only full source tiles can be used
				ECompareTile eCT1 = CompareTile270(min(nTileSizeX, nSrcSizeY), min(nTileSizeY, nSrcSizeX), a_pImage1 + (a_nTileSizeY*nSrcTileY)*a_nStride1 + (a_nTileSizeX*nSrcTileX+a_nTileSizeX-1)*3, a_nStride1, a_pImage2 + a_nTileSizeX*y*a_nStride2 + a_nTileSizeY*x*3, a_nStride2);
				if (eCT1 == ECTEqual)
				{
					*pMapping = i->second;
					++nSame270;
					break;
				}
				else if (eCT1 == ECTTrivial)
				{
					++nTrivial270;
					break;
				}
			}
			++pMapping;
		}
	}

	if (nSame == 0 && nSame180 == 0 && nSame90 == 0 && nSame270 == 0)
		return ECTDifferent;
	if (nSame >= nSame180 && nSame >= nSame90 && nSame >= nSame270)
	{
		if (ULONG((nSame+nTrivial)*10) < nTiles2X*nTiles2Y)
			return ECTDifferent; // too few copyable tiles
		CopyMemory(a_pMapping, cMapping.m_p, sizeof(int)*nTiles2X*nTiles2Y);
		return ECTEqual;
	}
	else if (nSame180 >= nSame && nSame180 >= nSame90 && nSame180 >= nSame270)
	{
		if (ULONG((nSame180+nTrivial180)*10) < nTiles2X*nTiles2Y)
			return ECTDifferent; // too few copyable tiles
		CopyMemory(a_pMapping, cMapping180.m_p, sizeof(int)*nTiles2X*nTiles2Y);
		return ECTRot180;
	}
	else if (nSame90 >= nSame && nSame90 >= nSame180 && nSame90 >= nSame270)
	{
		if (ULONG((nSame90+nTrivial90)*10) < nTiles2RX*nTiles2RY)
			return ECTDifferent; // too few copyable tiles
		CopyMemory(a_pMapping, cMapping90.m_p, sizeof(int)*nTiles2RX*nTiles2RY);
		return ECTRot90;
	}
	else
	{
		if (ULONG((nSame270+nTrivial270)*10) < nTiles2RX*nTiles2RY)
			return ECTDifferent; // too few copyable tiles
		CopyMemory(a_pMapping, cMapping270.m_p, sizeof(int)*nTiles2RX*nTiles2RY);
		return ECTRot270;
	}
}

void jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row, JDIMENSION num_blocks)
/* Copy a row of coefficient blocks from one place to another. */
{
  memcpy(output_row, input_row, num_blocks * (DCTSIZE2 * sizeof(JCOEF)));
}


void do_drop (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	 JDIMENSION x_crop_offset, JDIMENSION y_crop_offset,
	 jvirt_barray_ptr *src_coef_arrays,
	 j_decompress_ptr dropinfo, JDIMENSION drop_x_offset, JDIMENSION drop_y_offset,
	 jvirt_barray_ptr *drop_coef_arrays)
{
	for (int ci = 0; ci < dstinfo->num_components; ci++)
	{
		jpeg_component_info* compptr = dstinfo->comp_info + ci;
		JDIMENSION x_dst_blocks = x_crop_offset * compptr->h_samp_factor;
		JDIMENSION y_dst_blocks = y_crop_offset * compptr->v_samp_factor;
		JDIMENSION x_drop_blocks = drop_x_offset * compptr->h_samp_factor;
		JDIMENSION y_drop_blocks = drop_y_offset * compptr->v_samp_factor;
		JBLOCKARRAY dst_buffer = (*srcinfo->mem->access_virt_barray)
			((j_common_ptr) srcinfo, src_coef_arrays[ci], y_dst_blocks,
			(JDIMENSION) compptr->v_samp_factor, TRUE);
		if (ci < dropinfo->num_components)
		{
			JBLOCKARRAY src_buffer = (*srcinfo->mem->access_virt_barray)
				((j_common_ptr) srcinfo, drop_coef_arrays[ci], y_drop_blocks,
				(JDIMENSION) compptr->v_samp_factor, FALSE);
			for (int offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
			{
				jcopy_block_row(src_buffer[offset_y] + x_drop_blocks,
					dst_buffer[offset_y] + x_dst_blocks,
					compptr->h_samp_factor);
			}
		}
		else
		{
			for (int offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
			{
				ZeroMemory(dst_buffer[offset_y] + x_dst_blocks, compptr->h_samp_factor * sizeof(JBLOCK));
			} 	
		}
	}
}

void do_drop180(j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
	 JDIMENSION x_crop_offset, JDIMENSION y_crop_offset,
	 jvirt_barray_ptr *src_coef_arrays,
	 j_decompress_ptr dropinfo, JDIMENSION drop_x_offset, JDIMENSION drop_y_offset,
	 jvirt_barray_ptr *drop_coef_arrays)
{
	for (int ci = 0; ci < dstinfo->num_components; ci++)
	{
		jpeg_component_info* compptr = dstinfo->comp_info + ci;
		JDIMENSION x_dst_blocks = x_crop_offset * compptr->h_samp_factor;
		JDIMENSION y_dst_blocks = y_crop_offset * compptr->v_samp_factor;
		JDIMENSION x_drop_blocks = drop_x_offset * compptr->h_samp_factor;
		JDIMENSION y_drop_blocks = drop_y_offset * compptr->v_samp_factor;
		JBLOCKARRAY dst_buffer = (*srcinfo->mem->access_virt_barray)
			((j_common_ptr) srcinfo, src_coef_arrays[ci], y_dst_blocks,
			(JDIMENSION) compptr->v_samp_factor, TRUE);
		if (ci < dropinfo->num_components)
		{
			JBLOCKARRAY src_buffer = (*srcinfo->mem->access_virt_barray)
				((j_common_ptr) srcinfo, drop_coef_arrays[ci], y_drop_blocks,
				(JDIMENSION) compptr->v_samp_factor, FALSE);
			for (int offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
			{
				for (int offset_x = 0; offset_x < compptr->h_samp_factor; offset_x++)
				{
					JBLOCKROW src_block = src_buffer[offset_y] + x_drop_blocks + offset_x;
					JBLOCKROW dst_block = dst_buffer[compptr->v_samp_factor-offset_y-1] + x_dst_blocks + compptr->h_samp_factor-offset_x-1;
					//memcpy(dst_block, src_block, (DCTSIZE2 * sizeof(JCOEF)));
					JCOEF* src_ptr = (src_block[0]);
					JCOEF* dst_ptr = (dst_block[0]);
					for (int i = 0; i < DCTSIZE; i += 2)
					{
						/* For even row, negate every odd column. */
						for (int j = 0; j < DCTSIZE; j += 2)
						{
							*dst_ptr++ = *src_ptr++;
							*dst_ptr++ = - *src_ptr++;
						}
						/* For odd row, negate every even column. */
						for (int j = 0; j < DCTSIZE; j += 2)
						{
							*dst_ptr++ = - *src_ptr++;
							*dst_ptr++ = *src_ptr++;
						}
					}
				}
			}
		}
		else
		{
			for (int offset_y = 0; offset_y < compptr->v_samp_factor; offset_y++)
			{
				ZeroMemory(dst_buffer[offset_y] + x_dst_blocks, compptr->h_samp_factor * sizeof(JBLOCK));
			} 	
		}
	}
}

void do_drop90(j_compress_ptr a_pDstInfo, jvirt_barray_ptr* a_pDstArrays, JDIMENSION a_nDstX, JDIMENSION a_nDstY,
			   j_decompress_ptr a_pSrcInfo, jvirt_barray_ptr* a_pSrcArrays, JDIMENSION a_nSrcX, JDIMENSION a_nSrcY)
{
	for (int iComp = 0; iComp < a_pDstInfo->num_components; ++iComp)
	{
		jpeg_component_info* pDstComp = a_pDstInfo->comp_info + iComp;
		int const nDstSampFactorX = pDstComp->h_samp_factor;
		int const nDstSampFactorY = pDstComp->v_samp_factor;
		JDIMENSION const nDstX = a_nDstX * nDstSampFactorX;
		JDIMENSION const nDstY = a_nDstY * nDstSampFactorY;
		JBLOCKARRAY aDstBuffer = (*a_pSrcInfo->mem->access_virt_barray)
			((j_common_ptr) a_pSrcInfo, a_pDstArrays[iComp], nDstY, nDstSampFactorY, TRUE);

		if (iComp < a_pSrcInfo->num_components)
		{
			ATLASSERT(a_pDstInfo->comp_info[iComp].h_samp_factor == a_pSrcInfo->comp_info[iComp].v_samp_factor &&
					  a_pDstInfo->comp_info[iComp].v_samp_factor == a_pSrcInfo->comp_info[iComp].h_samp_factor);

			int const nSrcSampFactorX = nDstSampFactorY;
			int const nSrcSampFactorY = nDstSampFactorX;
			JDIMENSION const nSrcX = a_nSrcX * nSrcSampFactorX;
			JDIMENSION const nSrcY = a_nSrcY * nSrcSampFactorY;
			JBLOCKARRAY aSrcBuffer = (*a_pSrcInfo->mem->access_virt_barray)
				((j_common_ptr) a_pSrcInfo, a_pSrcArrays[iComp], nSrcY, nSrcSampFactorY, FALSE);

			for (int nY = 0; nY < nDstSampFactorY; ++nY)
			{
				for (int nX = 0; nX < nDstSampFactorX; ++nX)
				{
					JCOEF* pSrc = (aSrcBuffer[nSrcSampFactorY-nX-1] + nSrcX + nY)[0];
					JCOEF* pDst = (aDstBuffer[nY] + nDstX + nX)[0];
					for (int i = 0; i < DCTSIZE; i++)
					{
						for (int j = 0; j < DCTSIZE; j++)
							pDst[j*DCTSIZE+i] = pSrc[i*DCTSIZE+j];
						i++;
						for (int j = 0; j < DCTSIZE; j++)
							pDst[j*DCTSIZE+i] = -pSrc[i*DCTSIZE+j];
					}
				}
			}
		}
		else
		{
			for (int nY = 0; nY < nDstSampFactorY; ++nY)
				ZeroMemory(aDstBuffer[nY] + nDstX, nDstSampFactorX * sizeof(JBLOCK));
		}
	}
}

void do_drop270(j_compress_ptr a_pDstInfo, jvirt_barray_ptr* a_pDstArrays, JDIMENSION a_nDstX, JDIMENSION a_nDstY,
				j_decompress_ptr a_pSrcInfo, jvirt_barray_ptr* a_pSrcArrays, JDIMENSION a_nSrcX, JDIMENSION a_nSrcY)
{
	for (int iComp = 0; iComp < a_pDstInfo->num_components; ++iComp)
	{
		jpeg_component_info* pDstComp = a_pDstInfo->comp_info + iComp;
		int const nDstSampFactorX = pDstComp->h_samp_factor;
		int const nDstSampFactorY = pDstComp->v_samp_factor;
		JDIMENSION const nDstX = a_nDstX * nDstSampFactorX;
		JDIMENSION const nDstY = a_nDstY * nDstSampFactorY;
		JBLOCKARRAY aDstBuffer = (*a_pSrcInfo->mem->access_virt_barray)
			((j_common_ptr) a_pSrcInfo, a_pDstArrays[iComp], nDstY, nDstSampFactorY, TRUE);

		if (iComp < a_pSrcInfo->num_components)
		{
			ATLASSERT(a_pDstInfo->comp_info[iComp].h_samp_factor == a_pSrcInfo->comp_info[iComp].v_samp_factor &&
					  a_pDstInfo->comp_info[iComp].v_samp_factor == a_pSrcInfo->comp_info[iComp].h_samp_factor);

			int const nSrcSampFactorX = nDstSampFactorY;
			int const nSrcSampFactorY = nDstSampFactorX;
			JDIMENSION const nSrcX = a_nSrcX * nSrcSampFactorX;
			JDIMENSION const nSrcY = a_nSrcY * nSrcSampFactorY;
			JBLOCKARRAY aSrcBuffer = (*a_pSrcInfo->mem->access_virt_barray)
				((j_common_ptr) a_pSrcInfo, a_pSrcArrays[iComp], nSrcY, nSrcSampFactorY, FALSE);

			for (int nY = 0; nY < nDstSampFactorY; ++nY)
			{
				for (int nX = 0; nX < nDstSampFactorX; ++nX)
				{
					JCOEF* pSrc = (aSrcBuffer[nX] + nSrcX + nSrcSampFactorX-nY-1)[0];
					JCOEF* pDst = (aDstBuffer[nY] + nDstX + nX)[0];
					for (int i = 0; i < DCTSIZE; i++)
					{
						for (int j = 0; j < DCTSIZE; j++)
						{
							pDst[j*DCTSIZE+i] = pSrc[i*DCTSIZE+j];
							j++;
							pDst[j*DCTSIZE+i] = -pSrc[i*DCTSIZE+j];
						}
					}
				}
			}
		}
		else
		{
			for (int nY = 0; nY < nDstSampFactorY; ++nY)
				ZeroMemory(aDstBuffer[nY] + nDstX, nDstSampFactorX * sizeof(JBLOCK));
		}
	}
}

void transpose_critical_parameters(j_compress_ptr dstinfo)
{
	int tblno, i, j, ci, itemp;
	jpeg_component_info *compptr;
	JQUANT_TBL *qtblptr;
	UINT16 qtemp;

	/* Transpose sampling factors */
	for (ci = 0; ci < dstinfo->num_components; ci++)
	{
		compptr = dstinfo->comp_info + ci;
		itemp = compptr->h_samp_factor;
		compptr->h_samp_factor = compptr->v_samp_factor;
		compptr->v_samp_factor = itemp;
	}

	/* Transpose quantization tables */
	for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
	{
		qtblptr = dstinfo->quant_tbl_ptrs[tblno];
		if (qtblptr != NULL) {
			for (i = 0; i < DCTSIZE; i++)
			{
				for (j = 0; j < i; j++)
				{
					qtemp = qtblptr->quantval[i*DCTSIZE+j];
					qtblptr->quantval[i*DCTSIZE+j] = qtblptr->quantval[j*DCTSIZE+i];
					qtblptr->quantval[j*DCTSIZE+i] = qtemp;
				}
			}
		}
	}
}

void SetResolution(jpeg_compress_struct& tCompress, TImageResolution const& tResolution)
{
	if (tResolution.nNumeratorX && tResolution.nNumeratorY &&
		tResolution.nDenominatorX && tResolution.nDenominatorY)
	{
		// we have some resolution info
		if (tResolution.nDenominatorX == 254 && tResolution.nDenominatorY == 254)
		{
			tCompress.density_unit = 1;
			tCompress.X_density = tResolution.nNumeratorX;
			tCompress.Y_density = tResolution.nNumeratorY;
		}
		else if (tResolution.nDenominatorX == 100 && tResolution.nDenominatorY == 100)
		{
			tCompress.density_unit = 2;
			tCompress.X_density = tResolution.nNumeratorX;
			tCompress.Y_density = tResolution.nNumeratorY;
		}
		else
		{
			tCompress.density_unit = 1;
			tCompress.X_density = tResolution.nNumeratorX*254/tResolution.nDenominatorX;
			tCompress.Y_density = tResolution.nNumeratorY*254/tResolution.nDenominatorY;
		}
	}
}

STDMETHODIMP CDocumentEncoderJPEG::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CConfigValue cQuality;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_QUALITY), &cQuality);
		CConfigValue cLossless;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_LOSSLESS), &cLossless);
		CConfigValue cMetadata;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_METADATA), &cMetadata);
		CConfigValue cChroma;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_CHROMASUBSAMPLING), &cChroma);
		CConfigValue cArithmetic;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_ARITHMETIC), &cArithmetic);

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI, false))
			return E_FAIL;
		cBuffer.ToRGB();
		ULONG const nSizeX = cBuffer.tSize.nX;
		ULONG const nSizeY = cBuffer.tSize.nY;
		ULONG nStride = cBuffer.nStride;

		BYTE const* pBuffer = cBuffer.aData.m_p;

		CJPEGCompressStruct tCompress;
		CJPEGDstStream cDataSink(a_pDst);
		tCompress.dest = &cDataSink;

		CComPtr<IImageMetaData> pIMD;
		a_pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		ULONG nSizeEXIF = 0;
		ULONG nSizeICC = 0;
		ULONG nSizeOrig = 0;
		CAutoVectorPtr<BYTE> pDataEXIF;
		CAutoVectorPtr<BYTE> pDataICC;
		CAutoVectorPtr<BYTE> pDataOrig;
		if (pIMD)
		{
			if (cMetadata.operator LONG() != CFGVAL_MD_REMOVE)
			{
				CComBSTR bstrEXIF(L"EXIF");
				pIMD->GetBlockSize(bstrEXIF, &nSizeEXIF);
				if (nSizeEXIF)
				{
					pDataEXIF.Allocate(nSizeEXIF);
					pIMD->GetBlock(bstrEXIF, nSizeEXIF, pDataEXIF);
				}
				if (cMetadata.operator LONG() == CFGVAL_MD_UPDATE)
				{
					CComPtr<IRWEXIF> pExif;
					RWCoCreateInstance(pExif, __uuidof(RWEXIF));
					if (pExif)
					{
						pExif->Load(nSizeEXIF, pDataEXIF);
						int nThumbnailSize = 160;
						CReturnedData cData;
						pExif->ThumbnailGet(&cData);
						if (cData.size()) try
						{
							CJPEGDecompressStruct cinfo;
							CJPEGDataSource cDataSrc(cData.begin(), cData.size());
							cinfo.src = &cDataSrc;
							jpeg_read_header(&cinfo, TRUE);
							nThumbnailSize = max(cinfo.image_height, cinfo.image_width);
						} catch (...) {}

						ULONG nTSX = nSizeX >= nSizeY ? nThumbnailSize : nSizeX*nThumbnailSize/nSizeY;
						ULONG nTSY = nSizeX >= nSizeY ? nSizeY*nThumbnailSize/nSizeX : nThumbnailSize;
						CAutoVectorPtr<SResamplingRGB8> cThumbnail(new SResamplingRGB8[nTSX*nTSY]);
						CResamplingRGB8 cRsmp(nTSX, nTSY, nSizeX, nSizeY, cThumbnail, sizeof(SResamplingRGB8)*nTSX, reinterpret_cast<SResamplingRGB8 const*>(pBuffer), nStride);
						cRsmp.Linear();
						CAutoVectorPtr<JSAMPLE const*> apRows(new JSAMPLE const*[nTSY]);
						for (ULONG i = 0; i < nTSY; ++i)
							apRows[i] = reinterpret_cast<JSAMPLE const*>(cThumbnail.m_p+nTSX*i);

						CJPEGCompressStruct tThumbComp;
						CJPEGMemDataSink cThumbSink;
						tThumbComp.dest = &cThumbSink;
						tThumbComp.image_width = nTSX;
						tThumbComp.image_height = nTSY;
						tThumbComp.input_components = 3;
						tThumbComp.in_color_space = JCS_RGB;
						jpeg_set_defaults(&tThumbComp);
						jpeg_set_quality(&tThumbComp, 80, false);
						jpeg_start_compress(&tThumbComp, TRUE);
						jpeg_write_scanlines(&tThumbComp, const_cast<JSAMPARRAY>(apRows.m_p), nTSY);
						jpeg_finish_compress(&tThumbComp);

						pExif->ThumbnailSet(/*cData.size(), cData.begin());// */cThumbSink.Length(), cThumbSink.Data());

						pExif->TagSetInt(2, 0xa002, 0, nSizeX); // EXIF_TAG_PIXEL_X_DIMENSION
						pExif->TagSetInt(2, 0xa003, 0, nSizeY); // EXIF_TAG_PIXEL_Y_DIMENSION
						if (cBuffer.tResolution.nDenominatorX == 254 && cBuffer.tResolution.nDenominatorY == 254)
						{
							pExif->TagSetInt(0, 0x0128, 0, 2); // EXIF_TAG_RESOLUTION_UNIT
							pExif->TagSetRat(0, 0x011a, 0, cBuffer.tResolution.nNumeratorX, 1); // EXIF_TAG_X_RESOLUTION
							pExif->TagSetRat(0, 0x011b, 0, cBuffer.tResolution.nNumeratorY, 1); // EXIF_TAG_Y_RESOLUTION
						}
						else if (cBuffer.tResolution.nDenominatorX == 100 && cBuffer.tResolution.nDenominatorY == 100)
						{
							pExif->TagSetInt(0, 0x0128, 0, 3); // EXIF_TAG_RESOLUTION_UNIT
							pExif->TagSetRat(0, 0x011a, 0, cBuffer.tResolution.nNumeratorX, 1); // EXIF_TAG_X_RESOLUTION
							pExif->TagSetRat(0, 0x011b, 0, cBuffer.tResolution.nNumeratorY, 1); // EXIF_TAG_Y_RESOLUTION
						}

						CReturnedData cNewExif;
						pExif->Save(&cNewExif);
						nSizeEXIF = cNewExif.size();
						pDataEXIF.Free();
						pDataEXIF.Attach(cNewExif.Detach());
					}
				}
			}
			CComBSTR bstrICC(L"ICC");
			pIMD->GetBlockSize(bstrICC, &nSizeICC);
			if (nSizeICC)
			{
				pDataICC.Allocate(nSizeICC);
				pIMD->GetBlock(bstrICC, nSizeEXIF, pDataICC);
			}
			if (cLossless.operator LONG() != CFGVAL_LL_NEVER)
			{
				CComBSTR bstrJPEG(L"JPEG");
				pIMD->GetBlockSize(bstrJPEG, &nSizeOrig);
				if (nSizeOrig)
				{
					pDataOrig.Allocate(nSizeOrig);
					pIMD->GetBlock(bstrJPEG, nSizeOrig, pDataOrig);
				}
			}
		}
		try
		{
			bool bSaveNormally = true;
			if (pDataOrig.m_p)
			{
				// try partially lossless saving
				CJPEGDataSource cDataSrc(pDataOrig.m_p, nSizeOrig);

				CJPEGDecompressStruct cinfo;
				cinfo.src = &cDataSrc;
				jpeg_read_header(&cinfo, TRUE);
				cinfo.dct_method = JDCT_FLOAT;
				//cinfo.out_color_space = JCS_RGB;
				jpeg_start_decompress(&cinfo);

				JSAMPLE** apRows = reinterpret_cast<JSAMPLE**>(_alloca(sizeof(JSAMPLE*)*cinfo.image_height));
				CAutoVectorPtr<BYTE> cOrigImage(new BYTE[3*cinfo.image_width*cinfo.image_height]);
				for (ULONG i = 0; i < cinfo.image_height; i++)
					apRows[i] = reinterpret_cast<JSAMPLE*>(cOrigImage.m_p+3*cinfo.image_width*i);
				for (ULONG i = 0; i < cinfo.image_height; )
					i += jpeg_read_scanlines(&cinfo, apRows+i, cinfo.image_height-i);
				jpeg_finish_decompress(&cinfo);
				ULONG nTilesX = (cinfo.image_width+cinfo.max_h_samp_factor*DCTSIZE-1)/(cinfo.max_h_samp_factor*DCTSIZE);
				ULONG nTilesY = (cinfo.image_height+cinfo.max_v_samp_factor*DCTSIZE-1)/(cinfo.max_v_samp_factor*DCTSIZE);
				ULONG nTilesX2 = (nSizeX+cinfo.max_h_samp_factor*DCTSIZE-1)/(cinfo.max_h_samp_factor*DCTSIZE);
				ULONG nTilesY2 = (nSizeY+cinfo.max_v_samp_factor*DCTSIZE-1)/(cinfo.max_v_samp_factor*DCTSIZE);
				ULONG nTilesXR2 = (nSizeX+cinfo.max_v_samp_factor*DCTSIZE-1)/(cinfo.max_v_samp_factor*DCTSIZE);
				ULONG nTilesYR2 = (nSizeY+cinfo.max_h_samp_factor*DCTSIZE-1)/(cinfo.max_h_samp_factor*DCTSIZE);
				CAutoVectorPtr<int> cMapping(new int[max(nTilesX2*nTilesY2, nTilesXR2*nTilesYR2)]);
				//CAutoVectorPtr<BYTE> cMatches(new BYTE[nTilesX*nTilesY]);
				switch (cLossless.operator LONG() != CFGVAL_LL_SIMPLE ?
					CompareTilesRot(cinfo.image_width, cinfo.image_height, cOrigImage, cinfo.image_width*3, nSizeX, nSizeY, pBuffer, nStride, cinfo.max_h_samp_factor*DCTSIZE, cinfo.max_v_samp_factor*DCTSIZE, cMapping) :
					CompareTiles(cinfo.image_width, cinfo.image_height, cOrigImage, cinfo.image_width*3, nSizeX, nSizeY, pBuffer, nStride, cinfo.max_h_samp_factor*DCTSIZE, cinfo.max_v_samp_factor*DCTSIZE, cMapping))
				{
				case ECTEqual:
					{
						cDataSrc.Restart();
						jpeg_read_header(&cinfo, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays = jpeg_read_coefficients(&cinfo);
						pDataOrig.Free();

						// very inefficient - saves the new image to memory and then re-reads it
						// TODO: do color space conversion and DCT for modified tiles only
						CJPEGCompressStruct tTmpCompress;
						CJPEGMemDataSink cTmpDataSink;

						CAutoVectorPtr<JSAMPLE const*> apTmpRows(new JSAMPLE const*[nSizeY]);
						for (ULONG i = 0; i < nSizeY; i++)
							apTmpRows[i] = reinterpret_cast<JSAMPLE const*>(pBuffer+nStride*i);

						jpeg_copy_critical_parameters(&cinfo, &tTmpCompress);
						tTmpCompress.image_width = nSizeX;
						tTmpCompress.image_height = nSizeY;
						tTmpCompress.in_color_space = JCS_RGB;

						tTmpCompress.dest = &cTmpDataSink;

						jpeg_start_compress(&tTmpCompress, TRUE);
						jpeg_write_scanlines(&tTmpCompress, const_cast<JSAMPARRAY>(apTmpRows.m_p), nSizeY);
						jpeg_finish_compress(&tTmpCompress);

						// now re-read the file as coeficients
						CJPEGDecompressStruct cinfo2;
						CJPEGDataSource cTmpDataSrc(cTmpDataSink.Data(), cTmpDataSink.Length());
						cinfo2.src = &cTmpDataSrc;
						jpeg_read_header(&cinfo2, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays2 = jpeg_read_coefficients(&cinfo2);

						// finally compose and save the image
						bSaveNormally = false;
						tCompress.input_components = 3;
						tCompress.in_color_space = JCS_RGB;
						jpeg_set_defaults(&tCompress);
						jpeg_copy_critical_parameters(&cinfo2, &tCompress);
						tCompress.image_width = nSizeX;
						tCompress.image_height = nSizeY;

						// merge the coefficients arrays
						int const* pMapping = cMapping;
						for (ULONG y = 0; y < nTilesY2; ++y)
						{
							for (ULONG x = 0; x < nTilesX2; ++x)
							{
								if (0 <= *pMapping)
								{
									int nSrcY = *pMapping/nTilesX;
									int nSrcX = *pMapping - nSrcY*nTilesX;
									do_drop(&cinfo2, &tCompress, x, y, src_coef_arrays2, &cinfo, nSrcX, nSrcY, src_coef_arrays);
								}
								++pMapping;
							}
						}

						SetResolution(tCompress, cBuffer.tResolution);

						tCompress.arith_code = cArithmetic.operator bool();
						jpeg_write_coefficients(&tCompress, src_coef_arrays2);
						SaveMetaData(&tCompress, pIMD, cMetadata, nSizeEXIF, pDataEXIF.m_p, nSizeICC, pDataICC.m_p);
						jpeg_finish_compress(&tCompress);
					}
					break;

				case ECTRot180:
					{
						cDataSrc.Restart();
						jpeg_read_header(&cinfo, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays = jpeg_read_coefficients(&cinfo);
						pDataOrig.Free();

						// very inefficient - saves the new image to memory and then re-reads it
						// TODO: do color space conversion and DCT for modified tiles only
						CJPEGCompressStruct tTmpCompress;
						CJPEGMemDataSink cTmpDataSink;

						CAutoVectorPtr<JSAMPLE const*> apTmpRows(new JSAMPLE const*[nSizeY]);
						for (ULONG i = 0; i < nSizeY; i++)
							apTmpRows[i] = reinterpret_cast<JSAMPLE const*>(pBuffer+nStride*i);

						jpeg_copy_critical_parameters(&cinfo, &tTmpCompress);
						tTmpCompress.image_width = nSizeX;
						tTmpCompress.image_height = nSizeY;
						tTmpCompress.in_color_space = JCS_RGB;

						tTmpCompress.dest = &cTmpDataSink;

						jpeg_start_compress(&tTmpCompress, TRUE);
						jpeg_write_scanlines(&tTmpCompress, const_cast<JSAMPARRAY>(apTmpRows.m_p), nSizeY);
						jpeg_finish_compress(&tTmpCompress);

						// now re-read the file as coeficients
						CJPEGDecompressStruct cinfo2;
						CJPEGDataSource cTmpDataSrc(cTmpDataSink.Data(), cTmpDataSink.Length());
						cinfo2.src = &cTmpDataSrc;
						jpeg_read_header(&cinfo2, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays2 = jpeg_read_coefficients(&cinfo2);

						// finally compose and save the image
						bSaveNormally = false;
						tCompress.input_components = 3;
						tCompress.in_color_space = JCS_RGB;
						jpeg_set_defaults(&tCompress);
						jpeg_copy_critical_parameters(&cinfo2, &tCompress);
						tCompress.image_width = nSizeX;
						tCompress.image_height = nSizeY;

						// merge the coefficients arrays
						int const* pMapping = cMapping;
						for (ULONG y = 0; y < nTilesY2; ++y)
						{
							for (ULONG x = 0; x < nTilesX2; ++x)
							{
								if (0 <= *pMapping)
								{
									int nSrcY = *pMapping/nTilesX;
									int nSrcX = *pMapping - nSrcY*nTilesX;
									do_drop180(&cinfo2, &tCompress, x, y, src_coef_arrays2, &cinfo, nSrcX, nSrcY, src_coef_arrays);
								}
								++pMapping;
							}
						}

						SetResolution(tCompress, cBuffer.tResolution);

						tCompress.arith_code = cArithmetic.operator bool();
						jpeg_write_coefficients(&tCompress, src_coef_arrays2);
						SaveMetaData(&tCompress, pIMD, cMetadata, nSizeEXIF, pDataEXIF.m_p, nSizeICC, pDataICC.m_p);
						jpeg_finish_compress(&tCompress);
					}
					break;

				case ECTRot90:
					{
						cDataSrc.Restart();
						jpeg_read_header(&cinfo, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays = jpeg_read_coefficients(&cinfo);
						pDataOrig.Free();

						// very inefficient - saves the new image to memory and then re-reads it
						// TODO: do color space conversion and DCT for modified tiles only
						CJPEGCompressStruct tTmpCompress;
						CJPEGMemDataSink cTmpDataSink;

						CAutoVectorPtr<JSAMPLE const*> apTmpRows(new JSAMPLE const*[nSizeY]);
						for (ULONG i = 0; i < nSizeY; i++)
							apTmpRows[i] = reinterpret_cast<JSAMPLE const*>(pBuffer+nStride*i);

						jpeg_copy_critical_parameters(&cinfo, &tTmpCompress);
						transpose_critical_parameters(&tTmpCompress);
						tTmpCompress.image_width = nSizeX;
						tTmpCompress.image_height = nSizeY;
						tTmpCompress.in_color_space = JCS_RGB;

						tTmpCompress.dest = &cTmpDataSink;

						jpeg_start_compress(&tTmpCompress, TRUE);
						jpeg_write_scanlines(&tTmpCompress, const_cast<JSAMPARRAY>(apTmpRows.m_p), nSizeY);
						jpeg_finish_compress(&tTmpCompress);

						// now re-read the file as coeficients
						CJPEGDecompressStruct cinfo2;
						CJPEGDataSource cTmpDataSrc(cTmpDataSink.Data(), cTmpDataSink.Length());
						cinfo2.src = &cTmpDataSrc;
						jpeg_read_header(&cinfo2, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays2 = jpeg_read_coefficients(&cinfo2);

						// finally compose and save the image
						bSaveNormally = false;
						tCompress.input_components = 3;
						tCompress.in_color_space = JCS_RGB;
						jpeg_set_defaults(&tCompress);
						jpeg_copy_critical_parameters(&cinfo, &tCompress);
						transpose_critical_parameters(&tCompress);
						tCompress.image_width = nSizeX;
						tCompress.image_height = nSizeY;
						jpeg_calc_jpeg_dimensions(&tCompress);

						// merge the coefficients arrays
						int const* pMapping = cMapping;
						for (ULONG y = 0; y < nTilesYR2; ++y)
						{
							for (ULONG x = 0; x < nTilesXR2; ++x)
							{
								if (0 <= *pMapping)
								{
									int nSrcY = *pMapping/nTilesX;
									int nSrcX = *pMapping - nSrcY*nTilesX;
									do_drop90(&tCompress, src_coef_arrays2, x, y, &cinfo, src_coef_arrays, nSrcX, nSrcY);
								}
								++pMapping;
							}
						}

						SetResolution(tCompress, cBuffer.tResolution);

						tCompress.arith_code = cArithmetic.operator bool();
						jpeg_write_coefficients(&tCompress, src_coef_arrays2);
						SaveMetaData(&tCompress, pIMD, cMetadata, nSizeEXIF, pDataEXIF.m_p, nSizeICC, pDataICC.m_p);
						jpeg_finish_compress(&tCompress);
					}
					break;

				case ECTRot270:
					{
						cDataSrc.Restart();
						jpeg_read_header(&cinfo, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays = jpeg_read_coefficients(&cinfo);
						pDataOrig.Free();

						// very inefficient - saves the new image to memory and then re-reads it
						// TODO: do color space conversion and DCT for modified tiles only
						CJPEGCompressStruct tTmpCompress;
						CJPEGMemDataSink cTmpDataSink;

						CAutoVectorPtr<JSAMPLE const*> apTmpRows(new JSAMPLE const*[nSizeY]);
						for (ULONG i = 0; i < nSizeY; i++)
							apTmpRows[i] = reinterpret_cast<JSAMPLE const*>(pBuffer+nStride*i);

						jpeg_copy_critical_parameters(&cinfo, &tTmpCompress);
						transpose_critical_parameters(&tTmpCompress);
						tTmpCompress.image_width = nSizeX;
						tTmpCompress.image_height = nSizeY;
						tTmpCompress.in_color_space = JCS_RGB;

						tTmpCompress.dest = &cTmpDataSink;

						jpeg_start_compress(&tTmpCompress, TRUE);
						jpeg_write_scanlines(&tTmpCompress, const_cast<JSAMPARRAY>(apTmpRows.m_p), nSizeY);
						jpeg_finish_compress(&tTmpCompress);

						// now re-read the file as coeficients
						CJPEGDecompressStruct cinfo2;
						CJPEGDataSource cTmpDataSrc(cTmpDataSink.Data(), cTmpDataSink.Length());
						cinfo2.src = &cTmpDataSrc;
						jpeg_read_header(&cinfo2, TRUE);
						//cinfo.out_color_space = JCS_RGB;
						jvirt_barray_ptr* src_coef_arrays2 = jpeg_read_coefficients(&cinfo2);

						// finally compose and save the image
						bSaveNormally = false;
						tCompress.input_components = 3;
						tCompress.in_color_space = JCS_RGB;
						jpeg_set_defaults(&tCompress);
						jpeg_copy_critical_parameters(&cinfo, &tCompress);
						transpose_critical_parameters(&tCompress);
						tCompress.image_width = nSizeX;
						tCompress.image_height = nSizeY;
						jpeg_calc_jpeg_dimensions(&tCompress);

						// merge the coefficients arrays
						int const* pMapping = cMapping;
						for (ULONG y = 0; y < nTilesYR2; ++y)
						{
							for (ULONG x = 0; x < nTilesXR2; ++x)
							{
								if (0 <= *pMapping)
								{
									int nSrcY = *pMapping/nTilesX;
									int nSrcX = *pMapping - nSrcY*nTilesX;
									do_drop270(&tCompress, src_coef_arrays2, x, y, &cinfo, src_coef_arrays, nSrcX, nSrcY);
								}
								++pMapping;
							}
						}

						SetResolution(tCompress, cBuffer.tResolution);

						tCompress.arith_code = cArithmetic.operator bool();
						jpeg_write_coefficients(&tCompress, src_coef_arrays2);
						SaveMetaData(&tCompress, pIMD, cMetadata, nSizeEXIF, pDataEXIF.m_p, nSizeICC, pDataICC.m_p);
						jpeg_finish_compress(&tCompress);
					}
					break;
				}
			}
			if (bSaveNormally)
			{
				JSAMPLE const** apRows = reinterpret_cast<JSAMPLE const**>(_alloca(sizeof(JSAMPLE*)*nSizeY));
				ULONG i;
				for (i = 0; i < nSizeY; i++)
				{
					apRows[i] = reinterpret_cast<JSAMPLE const*>(pBuffer+nStride*i);
				}

				tCompress.image_width = nSizeX;
				tCompress.image_height = nSizeY;
				tCompress.input_components = 3;
				tCompress.in_color_space = JCS_RGB;
				jpeg_set_defaults(&tCompress);
				LONG nSS = cChroma;
				if (nSS)
				{
					tCompress.comp_info[0].h_samp_factor = (nSS == CFGVAL_CH_2x2 || nSS == CFGVAL_CH_2x1) ? 2 : 1;
					tCompress.comp_info[0].v_samp_factor = (nSS == CFGVAL_CH_2x2 || nSS == CFGVAL_CH_1x2) ? 2 : 1;
				}
				BYTE nQuality = cQuality.operator LONG();
				if (nQuality >= 1 && nQuality <= 101)
				{
					jpeg_set_quality(&tCompress, nQuality-1, false);
				}

				SetResolution(tCompress, cBuffer.tResolution);

				tCompress.arith_code = cArithmetic.operator bool();
				jpeg_start_compress(&tCompress, TRUE);
				SaveMetaData(&tCompress, pIMD, cMetadata, nSizeEXIF, pDataEXIF.m_p, nSizeICC, pDataICC.m_p);
				jpeg_write_scanlines(&tCompress, const_cast<JSAMPARRAY>(apRows), nSizeY);
				jpeg_finish_compress(&tCompress);
			}
		}
		catch (...)
		{
			hRes = E_UNEXPECTED;
		}
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}

	return hRes;
}

extern "C"
{
#include "iccjpeg.h"
}

void CDocumentEncoderJPEG::SaveMetaData(jpeg_compress_struct* a_pCompress, IImageMetaData* a_pIMD, LONG a_nMode, ULONG a_nEXIF, BYTE const* a_pEXIF, ULONG a_nICC, BYTE const* a_pICC)
{
	if (a_nMode == CFGVAL_MD_REMOVE)
		return;

	for (int iApp = JPEG_APP0+1; iApp < 0x100; ++iApp)
	{
		if (iApp == JPEG_APP0+1/* && a_nMode == CFGVAL_MD_UPDATE*/)
		{
			if (a_nEXIF && a_pEXIF)
				jpeg_write_marker(a_pCompress, JPEG_APP0+1, a_pEXIF, a_nEXIF);
			continue;
		}
		if (iApp == JPEG_APP0+2)
		{
			if (a_nICC && a_pICC)
				write_icc_profile(a_pCompress, a_pICC, a_nICC);
			continue;
		}
		int i = 0;
		while (true)
		{
			wchar_t sz[16];
			swprintf(sz, L"JPEG%i-%i", iApp-JPEG_APP0, i);
			++i;
			CComBSTR bstr(sz);
			ULONG nSize = 0;
			if (FAILED(a_pIMD->GetBlockSize(bstr, &nSize)))
				break;
			if (nSize == 0)
				break; // or continue; ??
			CAutoVectorPtr<BYTE> cTmp(new BYTE[nSize]);
			if (FAILED(a_pIMD->GetBlock(bstr, nSize, cTmp)))
				break;
			jpeg_write_marker(a_pCompress, iApp, cTmp.m_p, nSize);
		}
	}
}

//#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentEncoderJPEG::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	*a_ppName = NULL;
	if (a_pConfig == NULL)
		return E_FAIL;

	CConfigValue quality;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_QUALITY), &quality);
	wchar_t sz[32];
	_swprintf(sz, L"%i%%", quality.operator LONG());
	//CComObject<CPrintfLocalizedString>* p = NULL;
	//CComObject<CPrintfLocalizedString>::CreateInstance(&p);
	//CComPtr<ILocalizedString> out = p;
	//CComPtr<ILocalizedString> templ;
	//templ.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %i%%")));
	//p->Init(templ, CMultiLanguageString::GetAuto(g_pszTypeNameJPEG), quality.operator LONG());
	*a_ppName = new CSimpleLocalizedString(SysAllocString(sz));
	return S_OK;
}

