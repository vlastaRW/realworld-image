
#pragma once

#include <DocumentMenuCommandImpl.h>
#include <InPlaceCalc.h>
#include <SimpleLocalizedString.h>
#include <PrintfLocalizedString.h>

//extern OLECHAR const HANDLECOORDINATES_NAME[] = L"[0409]Handle coordinates...[0405]Upravit souřadnice bodu...";
extern __declspec(selectany) OLECHAR const HANDLECOORDINATES_DESC[] = L"[0409]Manually set coordinates of the clicked control handle.[0405]Ručně nastavit souřadnice kliknutého kontrolního bodu.";

struct TEditToolCoordsHandler
{
	TEditToolCoordsHandler() : pTool(NULL), nHandle(0) {}
	TEditToolCoordsHandler(IRasterImageEditTool* pTool, ULONG nHandle) : pTool(pTool), nHandle(nHandle) {}
	TPixelCoords get()
	{
		TPixelCoords tPixel = {0, 0};
		ULONG dummy;
		pTool->GetControlPoint(nHandle, &tPixel, &dummy);
		return tPixel;
	}
	HRESULT set(TPixelCoords tCoords)
	{
		return pTool->SetControlPoint(nHandle, &tCoords, true, 0.0f);
	}
private:
	CComPtr<IRasterImageEditTool> pTool;
	ULONG nHandle;
};


template<typename THandler>
class ATL_NO_VTABLE CManualHandleCoordinates :
	public CDocumentMenuCommandMLImpl<CManualHandleCoordinates<THandler>, NULL/*HANDLECOORDINATES_NAME*/, HANDLECOORDINATES_DESC, NULL, 0>
{
public:
	void Init(THandler a_tHandler)
	{
		m_tHandler = a_tHandler;
	}

	// IDocumentMenuCommand
public:
	class CDlg :
		public Win32LangEx::CLangIndirectDialogImpl<CDlg>
	{
	public:
		CDlg(LCID a_tLocaleID, TPixelCoords* a_pCoords) :
			Win32LangEx::CLangIndirectDialogImpl<CDlg>(a_tLocaleID), m_pCoords(a_pCoords)
		{
		}

		enum {IDC_HANDLE_X = 300, IDC_HANDLE_X_UPDOWN, IDC_HANDLE_Y, IDC_HANDLE_Y_UPDOWN, IDC_SEPARATORLINE};

		BEGIN_DIALOG_EX(0, 0, 220, 94, 0)
			DIALOG_CAPTION(_T("[0409]Modify Handle Coordinates[0405]Upravit souřadnice vrcholu"))
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_SETFONT|DS_MODALFRAME|DS_FIXEDSYS|WS_POPUP|WS_CAPTION|WS_SYSMENU)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_LTEXT(_T("[0409]&X coordinate:[0405]Souřadnice &X:"), IDC_STATIC, 7, 9, 51, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_HANDLE_X, 59, 7, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
			CONTROL_CONTROL(_T(""), IDC_HANDLE_X_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 97, 7, 10, 12, 0)
			CONTROL_LTEXT(_T("[0409]&Y coordinate:[0405]Souřadnice &Y:"), IDC_STATIC, 113, 9, 51, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_HANDLE_Y, 165, 7, 48, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
			CONTROL_CONTROL(_T(""), IDC_HANDLE_Y_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 203, 7, 10, 12, 0)
			CONTROL_CONTROL(_T(""), IDC_SEPARATORLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 7, 26, 206, 1, 0)
			CONTROL_LTEXT(_T("[0409]Note: handle coordinates will be sent to the tool exactly as entered, regardless of current coordinate mode. In case of invalid values, the results can be unexpected.[0405]Poznámka: souřadnice bodu budou předány přímo kreslicímu nástoji bez jakýcholi zaokrouhlení nebo korekcí. V případě neplatných souřadnic není výsledek zaručen."), IDC_STATIC, 7, 34, 206, 32, WS_VISIBLE, 0)
			CONTROL_DEFPUSHBUTTON(_T("[0409]OK[0405]OK"), IDOK, 109, 73, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
			CONTROL_PUSHBUTTON(_T("[0409]Cancel[0405]Storno"), IDOK, 163, 73, 50, 14, WS_VISIBLE | WS_TABSTOP, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
			NOTIFY_HANDLER(IDC_HANDLE_X_UPDOWN, UDN_DELTAPOS, OnUpDownChange)
			NOTIFY_HANDLER(IDC_HANDLE_Y_UPDOWN, UDN_DELTAPOS, OnUpDownChange)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
		{
			TCHAR szTmp[64];
			_stprintf(szTmp, _T("%g"), m_pCoords->fX);
			SetDlgItemText(IDC_HANDLE_X, szTmp);
			_stprintf(szTmp, _T("%g"), m_pCoords->fY);
			SetDlgItemText(IDC_HANDLE_Y, szTmp);
			// center the dialog on the screen
			CenterWindow();
			return TRUE;
		}
		LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
		{
			TCHAR szTmp[256];
			GetDlgItemText(IDC_HANDLE_X, szTmp, itemsof(szTmp));
			szTmp[itemsof(szTmp)-1] = _T('\0');
			LPCTSTR pEnd = szTmp;
			m_pCoords->fX = CInPlaceCalc::EvalExpression(szTmp, &pEnd);
			if (*pEnd != _T('\0'))
			{
				GetDlgItem(IDC_HANDLE_X).SetFocus();
				MessageBeep(MB_ICONASTERISK);
				return 0;
			}
			GetDlgItemText(IDC_HANDLE_Y, szTmp, itemsof(szTmp));
			szTmp[itemsof(szTmp)-1] = _T('\0');
			pEnd = szTmp;
			m_pCoords->fY = CInPlaceCalc::EvalExpression(szTmp, &pEnd);
			if (*pEnd != _T('\0'))
			{
				GetDlgItem(IDC_HANDLE_Y).SetFocus();
				MessageBeep(MB_ICONASTERISK);
				return 0;
			}
			EndDialog(IDOK);
			return 0;
		}
		LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
		{
			EndDialog(IDCANCEL);
			return 0;
		}
		LRESULT OnUpDownChange(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& a_bHandled)
		{
			LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
			CEdit wndEdit((HWND)::SendMessage(a_pNMHDR->hwndFrom, UDM_GETBUDDY, 0, 0));
			if (wndEdit.m_hWnd == NULL)
			{
				a_bHandled = FALSE;
				return 0;
			}
			int nTextLen = wndEdit.GetWindowTextLength();
			if (nTextLen < 0)
				nTextLen = 0;
			CAutoVectorPtr<TCHAR> psz(new TCHAR[nTextLen+1]);
			wndEdit.GetWindowText(psz, nTextLen+1);
			psz[nTextLen] = _T('\0');
			LPCTSTR p = psz;
			double d = CInPlaceCalc::EvalExpression(psz, &p);
			d -= pNMUD->iDelta;
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), d);
			wndEdit.SetWindowText(szTmp);
			return 0;
		}

	private:
		TPixelCoords* const m_pCoords;
	};

	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			TPixelCoords tPixel = m_tHandler.get();
			wchar_t sz[64];
			swprintf(sz, L"%g, %g", int(tPixel.fX*100+0.5f)/100.0f, int(tPixel.fY*100+0.5f)/100.0f);
			CComPtr<ILocalizedString> pNum;
			pNum.Attach(new CSimpleLocalizedString(SysAllocString(sz)));
			CComObject<CPrintfLocalizedString>* pPF = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
			CComPtr<ILocalizedString> pOut = pPF;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s...\t[%s]")));
			pPF->Init(pTempl, CMultiLanguageString::GetAuto(L"[0409]Coordinates[0405]Souřadnice"), pNum);
			*a_ppText = pOut.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			TPixelCoords tPixel = m_tHandler.get();
			CDlg cDlg(a_tLocaleID, &tPixel);
			if (cDlg.DoModal(a_hParent) != IDOK)
				return S_FALSE;
			return m_tHandler.set(tPixel);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	THandler m_tHandler;
};
