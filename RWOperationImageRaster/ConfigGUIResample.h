
#pragma once

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>
#include <RenderIcon.h>
//#include "../../_extlibs/agg2/src/agg_vcgen_stroke.cpp"

class ATL_NO_VTABLE CConfigGUIResample :
	public CCustomConfigResourcelessWndImpl<CConfigGUIResample>,
	public CDialogResize<CConfigGUIResample>
{
public:
	CConfigGUIResample()
	{
		m_tSize.nX = m_tSize.nY = 0;
		m_tRes.nNumeratorX = m_tRes.nDenominatorX = m_tRes.nNumeratorY = m_tRes.nDenominatorY = 0;
		m_szTemplate[0] = _T('\0');
	}
	~CConfigGUIResample()
	{
		m_cIcons.Destroy();
	}

	enum
	{
		IDC_MODE = 100,
		IDC_SIZEABS, IDC_SIZEABS_SPIN, IDC_PIXELS,
		IDC_SIZEX, IDC_SIZEX_SPIN, IDC_TIMES,
		IDC_SIZEY, IDC_SIZEY_SPIN, IDC_PIXELSXY,
		IDC_SIZEREL, IDC_SIZEREL_SPIN, IDC_PERCENT,
		IDC_METHOD,
		IDC_RESOLUTIONMODE, IDC_RESOLUTION, IDC_DPI,
		IDC_MESSAGE,
	};

	BEGIN_DIALOG_EX(0, 0, 150, 80, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Specify:[0405]Nastavit:"), IDC_STATIC, 0, 2, 66, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_MODE, WC_COMBOBOXEX, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 67, 0, 83, 200, 0)
		//CONTROL_COMBOBOX(IDC_MODE, 67, 0, 83, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Value:[0405]Hodnota:"), IDC_STATIC, 0, 18, 66, 8, WS_VISIBLE, 0)

		CONTROL_EDITTEXT(IDC_SIZEABS, 67, 16, 56, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_SIZEABS_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 112, 16, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_PIXELS, 127, 18, 22, 8, WS_VISIBLE, 0)

		CONTROL_EDITTEXT(IDC_SIZEX, 67, 16, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_SIZEX_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 80, 16, 11, 12, 0)
		CONTROL_CTEXT(_T("[0409]x[0405]x"), IDC_TIMES, 91, 18, 8, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_SIZEY, 99, 16, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_SIZEY_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 112, 16, 11, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_PIXELSXY, 127, 18, 22, 8, WS_VISIBLE, 0)

		CONTROL_EDITTEXT(IDC_SIZEREL, 67, 16, 56, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_SIZEREL_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 112, 16, 11, 12, 0)
		CONTROL_LTEXT(_T("[0000]%"), IDC_PERCENT, 127, 18, 22, 8, WS_VISIBLE, 0)

		CONTROL_LTEXT(_T("[0409]Method:[0405]&Metoda:"), IDC_STATIC, 0, 34, 66, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_METHOD, 67, 32, 83, 12, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, 0)

		CONTROL_LTEXT(_T("[0409]Resolution:[0405]Rozlišení:"), IDC_STATIC, 0, 50, 66, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_RESOLUTIONMODE, 67, 48, 24, 12, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, 0)
		CONTROL_EDITTEXT(IDC_RESOLUTION, 99, 48, 24, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]DPI[0405]DPI"), IDC_DPI, 127, 50, 22, 8, WS_VISIBLE, 0)

		CONTROL_LTEXT(_T("[0409]Resizing from %s x %s to %s x %s pixels.[0405]Změna velikosti z %s x %s na %s x %s pixelů."), IDC_MESSAGE, 0, 64, 150, 16, WS_VISIBLE, 0)
	END_CONTROLS_MAP()



	BEGIN_MSG_MAP(CConfigGUIResample)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIResample>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIResample>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_MODE, CBN_SELCHANGE, OnModeChange)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIResample)
		DLGRESIZE_CONTROL(IDC_MODE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SIZEABS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SIZEABS_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_PIXELS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SIZEX, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_SIZEX_SPIN, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_TIMES, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_SIZEY, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_SIZEY_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_PIXELSXY, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SIZEREL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SIZEREL_SPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_PERCENT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_METHOD, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_RESOLUTIONMODE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_RESOLUTION, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_DPI, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_MESSAGE, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIResample)
		//CONFIGITEM_COMBOBOX(IDC_MODE, CFGID_SIZETYPE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_SIZEABS, CFGID_SIZE)
		CONFIGITEM_VISIBILITY(IDC_SIZEABS_SPIN, CFGID_SIZE)
		CONFIGITEM_VISIBILITY(IDC_PIXELS, CFGID_SIZE)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_SIZEX, CFGID_SIZEX)
		CONFIGITEM_VISIBILITY(IDC_SIZEX_SPIN, CFGID_SIZEX)
		CONFIGITEM_VISIBILITY(IDC_TIMES, CFGID_SIZEX)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_SIZEY, CFGID_SIZEY)
		CONFIGITEM_VISIBILITY(IDC_SIZEY_SPIN, CFGID_SIZEY)
		CONFIGITEM_VISIBILITY(IDC_PIXELSXY, CFGID_SIZEY)
		CONFIGITEM_EDITBOX_VISIBILITY(IDC_SIZEREL, CFGID_SIZEREL)
		CONFIGITEM_VISIBILITY(IDC_SIZEREL_SPIN, CFGID_SIZEREL)
		CONFIGITEM_VISIBILITY(IDC_PERCENT, CFGID_SIZEREL)
		CONFIGITEM_COMBOBOX(IDC_METHOD, CFGID_RESAMPLEMETHOD)
		CONFIGITEM_COMBOBOX(IDC_RESOLUTIONMODE, CFGID_RESOLUTIONMODE)
		CONFIGITEM_EDITBOX(IDC_RESOLUTION, CFGID_RESOLUTION)
	END_CONFIGITEM_MAP()


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
				if (SUCCEEDED(pDI->CanvasGet(&m_tSize, &m_tRes, NULL, NULL, NULL)))
					UpdateMessage();
				return;
			}
			CComPtr<IDocumentAnimation> pDA;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDA));
			if (pDA)
			{
				m_tSize = GetAnimationSize(pDA, &m_tRes);
				if (m_tSize.nX*m_tSize.nY)
					UpdateMessage();
				return;
			}
		}
		catch (...)
		{
		}
	}


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		GetDocumentInfo();

		UpdateMessage();

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnModeChange(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		LONG val = m_wndMode.GetItemData(m_wndMode.GetCurSel());
		CComBSTR bstr(CFGID_SIZETYPE);
		M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(val));
		return 0;
	}

	void ExtraInitDialog()
	{
		m_wndMessage = GetDlgItem(IDC_MESSAGE);
		m_wndMessage.GetWindowText(m_szTemplate, itemsof(m_szTemplate));

		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_SIZEREL_SPIN)).SetRange(1, 10000);
		CUpDownCtrl(GetDlgItem(IDC_SIZEABS_SPIN)).SetRange(1, 32767);
		CUpDownCtrl(GetDlgItem(IDC_SIZEX_SPIN)).SetRange(1, 32767);
		CUpDownCtrl(GetDlgItem(IDC_SIZEY_SPIN)).SetRange(1, 32767);

		m_wndMode = GetDlgItem(IDC_MODE);
		int size = XPGUI::GetSmallIconSize();
		m_cIcons.Create(size, size, XPGUI::GetImageListColorFlags(), 4, 4);
		m_wndMode.SetImageList(m_cIcons);

		CComPtr<IConfigItemOptions> pItem;
		M_Config()->ItemGetUIInfo(CComBSTR(CFGID_SIZETYPE), __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pItem));
		CComPtr<IEnumConfigItemOptions> pOptions;
		ULONG nOptions = 0;
		if (pItem == NULL || FAILED(pItem->OptionsEnum(&pOptions)) || pOptions == NULL || FAILED(pOptions->Size(&nOptions)) || nOptions == 0)
			return;
		CConfigValue cVal;
		M_Config()->ItemValueGet(CComBSTR(CFGID_SIZETYPE), &cVal);
		CConfigValue cOption;
		for (ULONG i = 0; SUCCEEDED(pOptions->Get(i, &cOption)); ++i)
		{
			CComBSTR bstrText;
			CComPtr<ILocalizedString> pStr;
			if (SUCCEEDED(pItem->ValueGetName(cOption, &pStr)) &&
				SUCCEEDED(pStr->GetLocalized(m_tLocaleID, &bstrText)))
			{
				int image = I_IMAGENONE;
				switch (cOption.operator LONG())
				{
				case CFGVAL_SIZETYPE_REL:
					{
						static TPolyCoords const aVtx[] =
						{
							{0.1f, 0.2f}, {0.2f, 0.1f}, {0.3f, 0.2f}, {0.2f, 0.3f}, {0.1f, 0.2f},
							{0.8f, 0.1f}, {0.9f, 0.2f}, {0.2f, 0.9f}, {0.1f, 0.8f}, {0.8f, 0.1f},
							{0.7f, 0.8f}, {0.8f, 0.7f}, {0.9f, 0.8f}, {0.8f, 0.9f}, {0.7f, 0.8f},
						};
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_X:
					{
						static TPolyCoords const aVtx[] = { {0.0f, 0.5f}, {0.2f, 0.3f}, {0.2f, 0.4f}, {0.8f, 0.4f}, {0.8f, 0.3f}, {1.0f, 0.5f}, {0.8f, 0.7f}, {0.8f, 0.6f}, {0.2f, 0.6f}, {0.2f, 0.7f}, };
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_Y:
					{
						static TPolyCoords const aVtx[] = { {0.5f, 0.0f}, {0.3f, 0.2f}, {0.4f, 0.2f}, {0.4f, 0.8f}, {0.3f, 0.8f}, {0.5f, 1.0f}, {0.7f, 0.8f}, {0.6f, 0.8f}, {0.6f, 0.2f}, {0.7f, 0.2f}, };
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_LONGER:
					{
						static TPolyCoords const aVtx[] = { {0.0f, 0.3f}, {0.1f, 0.4f}, {0.9f, 0.4f}, {1.0f, 0.3f}, {1.0f, 0.7f}, {0.9f, 0.6f}, {0.1f, 0.6f}, {0.0f, 0.7f}, };
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_SHORTER:
					{
						static TPolyCoords const aVtx[] = { {0.2f, 0.3f}, {0.3f, 0.4f}, {0.7f, 0.4f}, {0.8f, 0.3f}, {0.8f, 0.7f}, {0.7f, 0.6f}, {0.3f, 0.6f}, {0.2f, 0.7f}, };
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_ABS:
					{
						static TPolyCoords const aVtx[] =
						{
							{0.4f, 0.6f}, {0.2f, 0.6f}, {0.2f, 0.7f}, {0.0f, 0.5f}, {0.2f, 0.3f}, {0.2f, 0.4f},
							{0.4f, 0.4f}, {0.4f, 0.2f}, {0.3f, 0.2f}, {0.5f, 0.0f}, {0.7f, 0.2f}, {0.6f, 0.2f},
							{0.6f, 0.4f}, {0.8f, 0.4f}, {0.8f, 0.3f}, {1.0f, 0.5f}, {0.8f, 0.7f}, {0.8f, 0.6f},
							{0.6f, 0.6f}, {0.6f, 0.8f}, {0.7f, 0.8f}, {0.5f, 1.0f}, {0.3f, 0.8f}, {0.4f, 0.8f},
						};
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_FRAME:
					{
						static TPolyCoords const aVtx[] = { {0.0f, 0.2f}, {1.0f, 0.2f}, {1.0f, 0.8f}, {0.0f, 0.8f}, };
						HICON h = IconFromPolygon(itemsof(aVtx), aVtx, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				case CFGVAL_SIZETYPE_CROP:
				case CFGVAL_SIZETYPE_EXTEND:
					{
						static TPolyCoords const aVtx1[] = { {0.0f, 0.2f}, {1.0f, 0.2f}, {1.0f, 0.8f}, {0.0f, 0.8f}, };
						static TPolyCoords const aVtx2[] = { {0.2f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.8f}, {0.2f, 0.8f}, };
						COLORREF clr3D = GetSysColor(COLOR_3DFACE);
						float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
						float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
						float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
						float const f3DA = 0.64f;
						float const fA = 0.36f;
						agg::rgba8 clrB(255.0f*(f3DR*f3DA)+0.5f, 255.0f*(f3DG*f3DA)+0.5f, 255.0f*(f3DB*f3DA)+0.5f, 255);
						agg::rgba8 clrW(255.0f*(f3DR*f3DA+fA)+0.5f, 255.0f*(f3DG*f3DA+fA)+0.5f, 255.0f*(f3DB*f3DA+fA)+0.5f, 255);
						TIconPolySpec const t[2] = 
						{
							{itemsof(aVtx1), aVtx1, cOption.operator LONG() == CFGVAL_SIZETYPE_EXTEND ? clrB : clrW, agg::rgba8(0, 0, 0, 255)},
							{itemsof(aVtx2), aVtx2, cOption.operator LONG() == CFGVAL_SIZETYPE_EXTEND ? clrW : clrB, agg::rgba8(0, 0, 0, 255)},
						};
						HICON h = IconFromPolygon(2, t, size, false);
						image = m_cIcons.AddIcon(h);
						DestroyIcon(h);
					}
					break;
				}
				m_wndMode.InsertItem(i, COLE2CT(bstrText), image, image, 0, cOption.operator LONG());
			}
			if (cOption == cVal)
				m_wndMode.SetCurSel(i);
		}
	}
	void ExtraConfigNotify()
	{
		//M_Config()->ItemValueGet(CComBSTR(CFGID_SIZETYPE), &cVal);
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
			LONG nFrameX = 0;
			LONG nFrameY = 0;
			LONG nSizeX = 0;
			LONG nSizeY = 0;
			if (m_tSize.nX && m_tSize.nY)
			{
				_stprintf(szSrcX, _T("%i"), m_tSize.nX);
				_stprintf(szSrcY, _T("%i"), m_tSize.nY);
				GetResampledSize(M_Config(), m_tSize, nSizeX, nSizeY, nFrameX, nFrameY);
				_stprintf(szDstX, _T("%i"), nFrameX);
				_stprintf(szDstY, _T("%i"), nFrameY);
			}
			else
			{
				CConfigValue cVal;
				M_Config()->ItemValueGet(CComBSTR(CFGID_SIZETYPE), &cVal);
				switch (cVal.operator LONG())
				{
				case CFGVAL_SIZETYPE_ABS:
				case CFGVAL_SIZETYPE_CROP:
				case CFGVAL_SIZETYPE_EXTEND:
					M_Config()->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
					_stprintf(szDstX, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
					M_Config()->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
					_stprintf(szDstY, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
					break;
				case CFGVAL_SIZETYPE_X:
					M_Config()->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
					_stprintf(szDstX, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
					break;
				case CFGVAL_SIZETYPE_Y:
					M_Config()->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
					_stprintf(szDstY, _T("%i"), cVal.operator LONG() < 1 ? 1 : cVal.operator LONG());
					break;
				}
			}
			TCHAR szMsg[384];
			_stprintf(szMsg, m_szTemplate, szSrcX, szSrcY, szDstX, szDstY);
			if (m_tRes.nNumeratorX && m_tRes.nNumeratorY && m_tRes.nDenominatorX && m_tRes.nDenominatorY)
			{
				TImageResolution tNewRes = GetResolution(m_tRes, M_Config(), m_tSize, CImageSize(nSizeX, nSizeY));
				_stprintf(szMsg+_tcslen(szMsg), _T(" %g x %g cm @%i DPI."),
					floor(nFrameX/(double(tNewRes.nNumeratorX)/tNewRes.nDenominatorX*100.0)*10.0+0.5)*0.1, floor(nFrameY/(double(tNewRes.nNumeratorY)/tNewRes.nDenominatorY*100.0)*10.0+0.5)*0.1,
					LONG(double(tNewRes.nNumeratorX)/tNewRes.nDenominatorX*254.0+0.5));
			}
			m_wndMessage.SetWindowText(szMsg);
		}
	}

private:
	CComboBoxEx m_wndMode;
	CImageList m_cIcons;
	CStatic m_wndMessage;
	TCHAR m_szTemplate[256];
	TImageSize m_tSize;
	TImageResolution m_tRes;
};

