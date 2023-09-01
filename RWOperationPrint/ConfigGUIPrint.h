
#pragma once

#include <ConfigCustomGUIImpl.h>
#include "WTL_PageSetup.h"
#include <XPGUI.h>
#include <vector>

void DevModeToConfigValue(const DEVMODE* a_pDevMode, TConfigValue* a_pConfigValue)
{
	a_pConfigValue->eTypeID = ECVTString;
	if (a_pDevMode == NULL || a_pDevMode->dmSize == 0)
	{
		a_pConfigValue->bstrVal = SysAllocString(L"");
	}
	else
	{
		WORD const nLen = a_pDevMode->dmSize+a_pDevMode->dmDriverExtra;
		a_pConfigValue->bstrVal = SysAllocStringLen(NULL, nLen);
		BYTE const* p = reinterpret_cast<BYTE const*>(a_pDevMode);
		for (WORD i = 0; i < nLen; ++i)
			a_pConfigValue->bstrVal[i] = p[i]+0x20;
	}
}

void DevModeFromConfigValue(TConfigValue const* a_pConfigValue, CDevMode& a_cDevMode)
{
	if (a_pConfigValue && a_pConfigValue->eTypeID == ECVTString &&
		a_pConfigValue->bstrVal && SysStringLen(a_pConfigValue->bstrVal) >= sizeof(DEVMODE))
	{
		UINT nLen = SysStringLen(a_pConfigValue->bstrVal);
		BYTE* p = new BYTE[nLen];
		for (UINT i = 0; i < nLen; ++i)
			p[i] = a_pConfigValue->bstrVal[i]-0x20;
		a_cDevMode.CopyFromDEVMODE(reinterpret_cast<DEVMODE*>(p));
	}
}

class ATL_NO_VTABLE CConfigGUIPrint :
	public CCustomConfigResourcelessWndImpl<CConfigGUIPrint>,
	public CDialogResize<CConfigGUIPrint>
{
public:
	CConfigGUIPrint() : m_nImgW(3000), m_nImgH(2000), m_fImgResW(100.0f/254.0f), m_fImgResH(100.0f/254.0f), m_hConfigureIcon(NULL), m_nPrinterW(2000), m_nPrinterH(3000), m_nPrinterResW(300), m_nPrinterResH(300), m_fLastUnits(0.0f)
	{
		m_rcPrinterMargins.top = m_rcPrinterMargins.left = m_rcPrinterMargins.bottom = m_rcPrinterMargins.right = 0;
	}
	~CConfigGUIPrint()
	{
		if (m_hConfigureIcon) DestroyIcon(m_hConfigureIcon);
	}

	enum
	{
		IDC_COMBO_PRINTER = 100,
		IDC_BUTTON_PRINTER_SETUP,
		IDC_CHECK_CENTER,
		IDC_CHECK_ROTATE,
		IDC_RADIO_FIT_PAPER,
		IDC_RADIO_ORIGINAL_SIZE,
		IDC_RADIO_CUSTOM_WIDTH,
		IDC_RADIO_CUSTOM_HEIGHT,
		IDC_EDIT_WIDTH,
		IDC_SPIN_WIDTH,
		IDC_EDIT_HEIGHT,
		IDC_SPIN_HEIGHT,
		IDC_COMBO_UNITS,
		IDC_STATIC_COPIES,
		IDC_EDIT_COPIES,
		IDC_SPIN_COPIES,
		IDC_PAGECTRL,
		IDC_CHECK_COMBINE,
		WM_UPDATEDEFAULTPRINTER = WM_USER+1726
	};

	BEGIN_DIALOG_EX(0, 0, 266, 131, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_COMBOBOX(IDC_COMBO_PRINTER, 136, 0, 112, 98, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T("."), IDC_BUTTON_PRINTER_SETUP, 252, 0, 14, 12, BS_ICON | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Center image on paper[0405]Vycentrovat obrázek na papíru"), IDC_CHECK_CENTER, 136, 20, 130, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Automatic rotation[0405]Automatické otočení"), IDC_CHECK_ROTATE, 136, 34, 130, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_RADIOBUTTON(_T("[0409]Scale to fit paper[0405]Roztáhnout na papír"), IDC_RADIO_FIT_PAPER, 136, 54, 130, 10, WS_GROUP | WS_VISIBLE, 0)
		CONTROL_RADIOBUTTON(_T("[0409]Original size[0405]Původní velikost"), IDC_RADIO_ORIGINAL_SIZE, 136, 69, 130, 10, WS_VISIBLE, 0)
		CONTROL_RADIOBUTTON(_T("[0409]Width[0405]Šířka"), IDC_RADIO_CUSTOM_WIDTH, 136, 84, 48, 10, WS_VISIBLE, 0)
		CONTROL_RADIOBUTTON(_T("[0409]Height[0405]Výška"), IDC_RADIO_CUSTOM_HEIGHT, 136, 99, 48, 10, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_EDIT_WIDTH, 187, 84, 35, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SPIN_WIDTH, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 211, 84, 12, 12, 0)
		CONTROL_EDITTEXT(IDC_EDIT_HEIGHT, 187, 99, 35, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SPIN_HEIGHT, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 211, 99, 12, 12, 0)
		CONTROL_COMBOBOX(IDC_COMBO_UNITS, 226, 92, 40, 30, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Copies:[0405]Kopie:"), IDC_STATIC_COPIES, 136, 119, 36, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_EDIT_COPIES, 173, 117, 35, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_SPIN_COPIES, UPDOWN_CLASS, UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 198, 117, 12, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_PAGECTRL, WC_STATIC, SS_BLACKRECT | WS_CLIPCHILDREN | WS_VISIBLE, 0, 0, 131, 131, 0)
		CONTROL_CHECKBOX(_T("[0409]Combine[0405]Skládat"), IDC_CHECK_COMBINE, 219, 118, 47, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIPrint)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIPrint>)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIPrint>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_BUTTON_PRINTER_SETUP, BN_CLICKED, OnPrinterSetup)
		COMMAND_HANDLER(IDC_COMBO_PRINTER, CBN_SELCHANGE, OnPrinterSelChange)
		COMMAND_HANDLER(IDC_COMBO_UNITS, CBN_SELCHANGE, OnUnitsSelChange)
		MESSAGE_HANDLER(WM_UPDATEDEFAULTPRINTER, OnUpdateDefaultPrinter)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIPrint)
		DLGRESIZE_CONTROL(IDC_PAGECTRL, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_COMBO_PRINTER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_BUTTON_PRINTER_SETUP, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CHECK_CENTER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CHECK_ROTATE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RADIO_FIT_PAPER, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RADIO_ORIGINAL_SIZE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RADIO_CUSTOM_WIDTH, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RADIO_CUSTOM_HEIGHT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_WIDTH, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SPIN_WIDTH, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_HEIGHT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SPIN_HEIGHT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_COMBO_UNITS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_STATIC_COPIES, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_EDIT_COPIES, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SPIN_COPIES, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CHECK_COMBINE, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIPrint)
		CONFIGITEM_RADIO(IDC_RADIO_FIT_PAPER, L"ImageSize", 0L)
		CONFIGITEM_RADIO(IDC_RADIO_ORIGINAL_SIZE, L"ImageSize", 1L)
		CONFIGITEM_RADIO(IDC_RADIO_CUSTOM_WIDTH, L"ImageSize", 2L)
		CONFIGITEM_RADIO(IDC_RADIO_CUSTOM_HEIGHT, L"ImageSize", 3L)
		CONFIGITEM_CHECKBOX(IDC_CHECK_CENTER, L"CenterImage")
		CONFIGITEM_CHECKBOX(IDC_CHECK_ROTATE, L"AutoRotate")
		CONFIGITEM_EDITBOX(IDC_EDIT_COPIES, L"Copies")
		CONFIGITEM_CHECKBOX(IDC_CHECK_COMBINE, L"Combine")
		CONFIGITEM_EDITBOX(IDC_EDIT_WIDTH, L"ImageWidth")
		CONFIGITEM_EDITBOX(IDC_EDIT_HEIGHT, L"ImageHeight")
		//CONFIGITEM_COMBOBOX(IDC_COMBO_UNITS, L"Unit")
		CONFIGITEM_CONTEXTHELP(IDC_COMBO_PRINTER, L"DeviceName")
		CONFIGITEM_CONTEXTHELP(IDC_BUTTON_PRINTER_SETUP, L"DeviceMode")
	END_CONFIGITEM_MAP()

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
				TImageSize tSize = {0, 0};
				TImageResolution tRes = {0, 0, 0, 0};
				if (SUCCEEDED(pDI->CanvasGet(&tSize, &tRes, NULL, NULL, NULL)))
				{
					m_nImgW = tSize.nX;
					m_nImgH = tSize.nY;
					if (tRes.nNumeratorX && tRes.nDenominatorX && tRes.nNumeratorY && tRes.nDenominatorY)
					{
						m_fImgResW = float(tRes.nNumeratorX)/tRes.nDenominatorX;
						m_fImgResH = float(tRes.nNumeratorY)/tRes.nDenominatorY;
					}
					else
					{
						m_fImgResW = m_fImgResH = 100.0f/254.0f;
					}
					m_cBuffer.Allocate(m_nImgW*m_nImgH);
					pDI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(m_cBuffer.m_p), NULL, EIRIPreview);
					ExtraConfigNotify();
				}
			}
		}
		catch (...)
		{
		}
	}

	void ExtraInitDialog()
	{
		// units
		m_wndUnits = GetDlgItem(IDC_COMBO_UNITS);
		{
			m_wndUnits.AddString(L"mm");
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]inch[0405]palec", m_tLocaleID, &bstr);
			m_wndUnits.AddString(bstr);
		}

		// printers
		m_wndPrinter = GetDlgItem(IDC_COMBO_PRINTER);
		DWORD needed = 0;
		DWORD returned = 0;
		EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, NULL, 2, NULL, 0, &needed, &returned);
		CAutoVectorPtr<BYTE> cPIBuffer(new BYTE[needed+1]);
		EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS, NULL, 2, cPIBuffer.m_p, needed, &needed, &returned);
		PRINTER_INFO_2* prninfo2 = reinterpret_cast<PRINTER_INFO_2*>(cPIBuffer.m_p);
		m_aPrinterNames.resize(returned);
		for(DWORD i=0; i<returned; ++i)
		{
			m_aPrinterNames[i] = prninfo2[i].pPrinterName;
			m_wndPrinter.AddString(prninfo2[i].pPrinterName);
		}

		// page setup control
		RECT rcWnd;
		CWindow wnd(GetDlgItem(IDC_PAGECTRL));
		wnd.GetWindowRect(&rcWnd);
		ScreenToClient(&rcWnd);
		wnd.DestroyWindow();
		m_wndPageSetup.Create(m_hWnd, &rcWnd, 0, 0, WS_EX_CLIENTEDGE, IDC_PAGECTRL);

		// spin buttons
		CUpDownCtrl(GetDlgItem(IDC_SPIN_WIDTH)).SetRange(1, 10000);
		CUpDownCtrl(GetDlgItem(IDC_SPIN_HEIGHT)).SetRange(1, 10000);
		CUpDownCtrl(GetDlgItem(IDC_SPIN_COPIES)).SetRange(1, 100);

		// configure button icon
		CButton wndButton(GetDlgItem(IDC_BUTTON_PRINTER_SETUP));
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(XPGUI::GetSmallIconSize());
			pSI->GetLayers(ESIModify, cRenderer);
			m_hConfigureIcon = cRenderer.get();
		}
		wndButton.SetIcon(m_hConfigureIcon);

		GetDocumentInfo();
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnUpdateDefaultPrinter(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		// default printer
		CPrinter cPrinter;
		if (cPrinter.OpenDefaultPrinter())
		{
			CDevMode cDevMode;
			cDevMode.CopyFromPrinter(cPrinter.m_hPrinter);
			CConfigValue cValDeviceName(cDevMode.m_pDevMode->dmDeviceName);
			CConfigValue cValDeviceMode;
			DevModeToConfigValue(cDevMode.m_pDevMode, &cValDeviceMode);
			CComBSTR idDeviceName(L"DeviceName");
			CComBSTR idDeviceMode(L"DeviceMode");
			BSTR aIDs[2];
			aIDs[0] = idDeviceName;
			aIDs[1] = idDeviceMode;
			TConfigValue aVals[2];
			aVals[0] = cValDeviceName;
			aVals[1] = cValDeviceMode;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
		}
		return 0;
	}

	LRESULT OnPrinterSetup(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CConfigValue cValDeviceName, cValDeviceMode;
		M_Config()->ItemValueGet(CComBSTR(L"DeviceName"), &cValDeviceName);
		M_Config()->ItemValueGet(CComBSTR(L"DeviceMode"), &cValDeviceMode);
		CDevMode cDevMode;
		DevModeFromConfigValue(cValDeviceMode, cDevMode);

		CPrinter cPrinter;
		if(cPrinter.OpenPrinter(cValDeviceName.operator BSTR(), cDevMode.m_pDevMode))
		{
			if(cDevMode.DocumentProperties(cPrinter.m_hPrinter))
			{
				DevModeToConfigValue(cDevMode.m_pDevMode, &cValDeviceMode);
				CComBSTR idDeviceMode(L"DeviceMode");
				BSTR aIDs[1];
				aIDs[0] = idDeviceMode;
				TConfigValue aVals[1];
				aVals[0] = cValDeviceMode;
				M_Config()->ItemValuesSet(1, aIDs, aVals);
			}
		}

		return 0;
	}

	LRESULT OnPrinterSelChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CConfigValue cValDeviceName, cValDeviceMode;
		M_Config()->ItemValueGet(CComBSTR(L"DeviceName"), &cValDeviceName);
		M_Config()->ItemValueGet(CComBSTR(L"DeviceMode"), &cValDeviceMode);
		CConfigValue cValDeviceNameNew(m_aPrinterNames[m_wndPrinter.GetCurSel()]);
		if(cValDeviceName != cValDeviceNameNew)
		{
			CPrinter cPrinter;
			if(cPrinter.OpenPrinter(cValDeviceNameNew.operator BSTR()))
			{
				CDevMode cDevMode;
				cDevMode.CopyFromPrinter(cPrinter.m_hPrinter);
				DevModeToConfigValue(cDevMode.m_pDevMode, &cValDeviceMode);
				CComBSTR idDeviceName(L"DeviceName");
				CComBSTR idDeviceMode(L"DeviceMode");
				BSTR aIDs[2];
				aIDs[0] = idDeviceName;
				aIDs[1] = idDeviceMode;
				TConfigValue aVals[2];
				aVals[0] = cValDeviceNameNew;
				aVals[1] = cValDeviceMode;
				m_fLastUnits = 0.0f;
				M_Config()->ItemValuesSet(2, aIDs, aVals);
			}
		}
		return 0;
	}

	LRESULT OnUnitsSelChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComBSTR bstrUnits(L"Unit");
		CConfigValue cNewVal(m_wndUnits.GetCurSel() ? 25.4f : 1.0f);
		CConfigValue cOldVal;
		M_Config()->ItemValueGet(bstrUnits, &cOldVal);
		if (cOldVal != cNewVal)
		{
			CComBSTR bstrWidth(L"ImageWidth");
			CComBSTR bstrHeight(L"ImageHeight");
			BSTR aIDs[3];
			aIDs[0] = bstrUnits;
			aIDs[1] = bstrWidth;
			aIDs[2] = bstrHeight;
			TConfigValue aVals[3];
			aVals[0] = cNewVal;
			ConfigValueInit(aVals[1]);
			ConfigValueInit(aVals[2]);
			M_Config()->ItemValueGet(bstrWidth, aVals+1);
			M_Config()->ItemValueGet(bstrHeight, aVals+2);
			aVals[1].fVal *= cOldVal.operator float()/cNewVal.operator float();
			aVals[2].fVal *= cOldVal.operator float()/cNewVal.operator float();
			// rounding
			RoundLengths(aVals[1].fVal, aVals[2].fVal, cNewVal);
			M_Config()->ItemValuesSet(3, aIDs, aVals);
		}
		return 0;
	}
	void RoundLengths(float& x, float& y, float unit)
	{
		// rounding
		if (unit == 25.4f)
		{
			x = int(x*100.0f+0.5f)/100.0f;
			y = int(y*100.0f+0.5f)/100.0f;
		}
		else
		{
			x = int(x+0.5f);
			y = int(y+0.5f);
		}
	}

	void ExtraConfigNotify()
	{
		// units
		CConfigValue cValUnit;
		M_Config()->ItemValueGet(CComBSTR(L"Unit"), &cValUnit);
		m_wndUnits.SetCurSel(cValUnit.operator float() == 25.4f ? 1 : 0);

		// printer
		CConfigValue cValDeviceName;
		M_Config()->ItemValueGet(CComBSTR(L"DeviceName"), &cValDeviceName);

		size_t selPrinter = m_aPrinterNames.size();
		for(size_t i = 0; i < m_aPrinterNames.size(); ++i)
		{
			if (m_aPrinterNames[i] == cValDeviceName)
			{
				selPrinter = i;
				break;
			}
		}
		if (selPrinter == m_aPrinterNames.size())
		{
			PostMessage(WM_UPDATEDEFAULTPRINTER);
		}
		else
		{
			m_wndPrinter.SetCurSel(selPrinter);

			// page setup
			CConfigValue cValImageSize;
			M_Config()->ItemValueGet(CComBSTR(L"ImageSize"), &cValImageSize);
			CConfigValue cValDeviceName, cValDeviceMode, cValCenterImage, cValImageWidth, cValImageHeight, cValUnit, cValCopies, cValCombine, cAutoRotate;
			M_Config()->ItemValueGet(CComBSTR(L"DeviceName"), &cValDeviceName);
			M_Config()->ItemValueGet(CComBSTR(L"DeviceMode"), &cValDeviceMode);
			M_Config()->ItemValueGet(CComBSTR(L"CenterImage"), &cValCenterImage);
			M_Config()->ItemValueGet(CComBSTR(L"ImageWidth"), &cValImageWidth);
			M_Config()->ItemValueGet(CComBSTR(L"ImageHeight"), &cValImageHeight);
			M_Config()->ItemValueGet(CComBSTR(L"Unit"), &cValUnit);
			M_Config()->ItemValueGet(CComBSTR(L"Copies"), &cValCopies);
			M_Config()->ItemValueGet(CComBSTR(L"Combine"), &cValCombine);
			M_Config()->ItemValueGet(CComBSTR(L"AutoRotate"), &cAutoRotate);

			if (m_bstrLastDevMode == NULL || m_bstrLastDevMode != cValDeviceMode.operator BSTR())
			{
				CDevMode cDevMode;
				DevModeFromConfigValue(cValDeviceMode, cDevMode);
				CPrinter cPrinter;
				if(!cPrinter.OpenPrinter(cValDeviceName, cDevMode.m_pDevMode))
					return;
				CDC cDC;
				cDC.Attach(cPrinter.CreatePrinterDC(cDevMode.m_pDevMode));
				if(cDC.IsNull())
					return;

				m_nPrinterW = cDC.GetDeviceCaps(HORZRES);
				m_nPrinterH = cDC.GetDeviceCaps(VERTRES);
				m_nPrinterResW = cDC.GetDeviceCaps(LOGPIXELSX);
				m_nPrinterResH = cDC.GetDeviceCaps(LOGPIXELSX);
				int nPhysX = cDC.GetDeviceCaps(PHYSICALWIDTH);
				int nPhysY = cDC.GetDeviceCaps(PHYSICALHEIGHT);
				int nMX = cDC.GetDeviceCaps(PHYSICALOFFSETX);
				int nMY = cDC.GetDeviceCaps(PHYSICALOFFSETY);
				if (nPhysX > m_nPrinterW && nMX <= (nPhysX-m_nPrinterW) &&
					nPhysY > m_nPrinterH && nMY <= (nPhysY-m_nPrinterH))
				{
					m_rcPrinterMargins.left = nMX;
					m_rcPrinterMargins.top = nMY;
					m_rcPrinterMargins.right = nPhysX-m_nPrinterW-nMX;
					m_rcPrinterMargins.bottom = nPhysY-m_nPrinterH-nMY;
				}
				m_bstrLastDevMode = cValDeviceMode.operator BSTR();
			}
			int nImgW = m_nImgW;
			int nImgH = m_nImgH;
			AdjustImageSize(cValImageSize, cAutoRotate, cValImageWidth, cValImageHeight, cValUnit, m_nPrinterW, m_nPrinterH, m_nPrinterResW, m_nPrinterResH, nImgW, nImgH, m_fImgResW, m_fImgResH);
			m_wndPageSetup.Init(m_nPrinterW, m_nPrinterH, m_rcPrinterMargins, nImgW, nImgH, cValCopies.operator LONG(), cValCombine, cValCenterImage, cAutoRotate, m_nImgW, m_nImgH, m_cBuffer);
		}

		// report sizes
		if (cValUnit.operator float() != m_fLastUnits)
		{
			m_fLastUnits = cValUnit;
			wchar_t const* pszUnit = m_fLastUnits == 25.4f ? L"\"" : L"mm";

			CWindow wndPaper = GetDlgItem(IDC_RADIO_FIT_PAPER);
			float px = 25.4f*float(m_nPrinterW)/m_nPrinterResW/m_fLastUnits;
			float py = 25.4f*float(m_nPrinterH)/m_nPrinterResH/m_fLastUnits;
			RoundLengths(px, py, m_fLastUnits);
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(L"[0409]Scale to fit paper[0405]Roztáhnout na papír", m_tLocaleID, &bstr);
			wchar_t sz[128];
			swprintf(sz, L" - %gx%g%s", px, py, pszUnit);
			bstr += sz;
			wndPaper.SetWindowText(bstr);

			if (m_nImgW*m_nImgH)
			{
				CWindow wndOriginal = GetDlgItem(IDC_RADIO_ORIGINAL_SIZE);
				float px = m_nImgW/m_fImgResW/m_fLastUnits/10;
				float py = m_nImgH/m_fImgResH/m_fLastUnits/10;
				RoundLengths(px, py, m_fLastUnits);
				bstr.Empty();
				CMultiLanguageString::GetLocalized(L"[0409]Original size[0405]Původní velikost", m_tLocaleID, &bstr);
				wchar_t sz[128];
				swprintf(sz, L" - %gx%g%s", px, py, pszUnit);
				bstr += sz;
				wndOriginal.SetWindowText(bstr);
			}
		}
	}

private:
	CPageSetupControl m_wndPageSetup;
	CComboBox m_wndPrinter;
	std::vector<CComBSTR> m_aPrinterNames;
	CComboBox m_wndUnits;
	float m_fLastUnits;

	HICON m_hConfigureIcon;
	int m_nImgW;
	int m_nImgH;
	CAutoVectorPtr<TPixelChannel> m_cBuffer;
	float m_fImgResW;
	float m_fImgResH;
	CComBSTR m_bstrLastDevMode;
	int m_nPrinterW;
	int m_nPrinterH;
	int m_nPrinterResW;
	int m_nPrinterResH;
	RECT m_rcPrinterMargins;
};

