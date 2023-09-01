// EditToolShapeShiftDlg.h : Declaration of the CEditToolShapeShiftDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ObserverImpl.h>


HICON GetShapeShiftIconForward(ULONG a_nSize);
HICON GetShapeShiftIconSideways(ULONG a_nSize);
HICON GetShapeShiftIconRestore(ULONG a_nSize);
HICON GetShapeShiftIconExpand(ULONG a_nSize);
HICON GetShapeShiftIconCollapse(ULONG a_nSize);

// CEditToolShapeShiftDlg

class CEditToolShapeShiftDlg : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CEditToolShapeShiftDlg>,
	public CObserverImpl<CEditToolShapeShiftDlg, ISharedStateObserver, TSharedStateChange>,
	public CChildWindowImpl<CEditToolShapeShiftDlg, IRasterImageEditToolWindow>,
	public CDialogResize<CEditToolShapeShiftDlg>,
	public CContextHelpDlg<CEditToolShapeShiftDlg>
{
public:
	CEditToolShapeShiftDlg() : m_bEnableUpdates(false)
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolShapeShiftDlg()
	{
		m_cImageList.Destroy();
		if (m_pSharedState)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}
	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup)
	{
		m_tLocaleID = a_tLocaleID;
		m_pSharedState = a_pSharedState;
		m_bstrSyncToolData = a_bstrSyncGroup;
		m_pSharedState->ObserverIns(ObserverGet(), 0);
		return Win32LangEx::CLangDialogImpl<CEditToolShapeShiftDlg>::Create(a_hParent);
	}


	enum { IDD = IDD_EDITTOOL_SHAPESHIFT };

	BEGIN_MSG_MAP(CEditToolShapeShiftDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolShapeShiftDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDC_ET_SIZEEDIT, EN_CHANGE, OnChangeSize)
		COMMAND_HANDLER(IDC_ET_INTENSITYEDIT, EN_CHANGE, OnChangeIntensity)
		COMMAND_HANDLER(IDC_ET_INTENSITYCHECK, BN_CLICKED, OnChangeIntPressure)
		NOTIFY_HANDLER(IDC_ET_SIZESPIN, UDN_DELTAPOS, OnUpDownChangeSize)
		NOTIFY_HANDLER(IDC_ET_INTENSITYSPIN, UDN_DELTAPOS, OnUpDownChangeIntensity)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_RANGE_CODE_HANDLER(IDC_ET_SHAPETOOL+1, IDC_ET_SHAPETOOL+CEditToolDataShapeShift::ESTCount, BN_CLICKED, OnChange)
		CHAIN_MSG_MAP(CDialogResize<CEditToolShapeShiftDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolShapeShiftDlg)
		DLGRESIZE_CONTROL(IDC_ET_SHAPETOOL, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZEEDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZEUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_INTENSITYEDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_INTENSITYSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_INTENSITYUNIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolShapeShiftDlg)
		CTXHELP_CONTROL_STRING(IDC_ET_SIZEEDIT, L"[0409]Diameter of the affected region in pixels.[0405]Velikost ovlivněné oblasti v pixelech.")
		CTXHELP_CONTROL_STRING(IDC_ET_SIZESPIN, L"[0409]Diameter of the affected region in pixels.[0405]Velikost ovlivněné oblasti v pixelech.")
		CTXHELP_CONTROL_STRING(IDC_ET_INTENSITYEDIT, L"[0409]Intensity of the deformation effect.[0405]Intenzita deformačního efektu.")
		CTXHELP_CONTROL_STRING(IDC_ET_INTENSITYSPIN, L"[0409]Intensity of the deformation effect.[0405]Intenzita deformačního efektu.")
		CTXHELP_CONTROL_STRING(IDC_ET_INTENSITYCHECK, L"[0409]When checked, the stylus pressure will be taken into account.[0405]Pokud je zaškrtnuto, bude brán v úvahu tlak pera tabletu.")
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolShapeShiftDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComBSTR bstr;
			a_tParams.pState->ToText(&bstr);
			if (bstr)
			{
				m_cData.FromString(bstr);
				DataToGUI();
			}
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			*a_pSize = m_tOptimumSize;
			return m_tOptimumSize.cx ? S_OK : E_FAIL;
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

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd)
		{
			if (m_wndSize == a_pMsg->hwnd)
			{
				if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
					return S_OK;
				TranslateMessage(a_pMsg);
				DispatchMessage(a_pMsg);
				return S_OK;
			}
		}
		return S_FALSE;
	}


	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndSize = GetDlgItem(IDC_ET_SIZEEDIT);
		m_wndIntensity = GetDlgItem(IDC_ET_INTENSITYEDIT);
		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedState> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				CComBSTR bstr;
				pState->ToText(&bstr);
				if (bstr)
					m_cData.FromString(bstr);
			}
		}

		int nIconSize = XPGUI::GetSmallIconSize();
		m_cImageList.Create(nIconSize, nIconSize, XPGUI::GetImageListColorFlags(), CEditToolDataShapeShift::ESTCount, 0);
		{
			HICON hTmp = GetShapeShiftIconForward(nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		{
			HICON hTmp = GetShapeShiftIconSideways(nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		{
			HICON hTmp = GetShapeShiftIconRestore(nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		{
			HICON hTmp = GetShapeShiftIconCollapse(nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		{
			HICON hTmp = GetShapeShiftIconExpand(nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}
		//UINT aIconIDs[] = {IDI_SHAPESHIFT_TANGENT, IDI_SHAPESHIFT_NORMAL, IDI_SHAPESHIFT_RESTORE, IDI_SHAPESHIFT_COLLAPSE, IDI_SHAPESHIFT_EXPAND, IDI_SHAPESHIFT_TWIRL, IDI_SHAPESHIFT_TURBULENCE};
		//for (int i = 0; i < CEditToolDataShapeShift::ESTCount; ++i)
		//{
		//	HICON hTmp = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(aIconIDs[i]), IMAGE_ICON, nIconSize, nIconSize, LR_DEFAULTCOLOR);
		//	m_cImageList.AddIcon(hTmp);
		//	DestroyIcon(hTmp);
		//}

		CComBSTR bstr;
		OLECHAR szTooltipStrings[1024] = L"";
		CMultiLanguageString::GetLocalized(L"[0409]Push forward[0405]Posunout dopředu", m_tLocaleID, &bstr);
		OLECHAR* p = szTooltipStrings;
		OLECHAR* q = bstr;
		while (*(p++) = *(q++)) ;
		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Push right[0405]Odsunout doprava", m_tLocaleID, &bstr);
		q = bstr;
		while (*(p++) = *(q++)) ;
		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Restore[0405]Obnovit", m_tLocaleID, &bstr);
		q = bstr;
		while (*(p++) = *(q++)) ;
		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Collapse[0405]Smrštit", m_tLocaleID, &bstr);
		q = bstr;
		while (*(p++) = *(q++)) ;
		bstr.Empty();
		CMultiLanguageString::GetLocalized(L"[0409]Expand[0405]Roztáhnout", m_tLocaleID, &bstr);
		q = bstr;
		while (*(p++) = *(q++)) ;
		*p = L'\0';
		//"Tangent push|Normal push|Revert|Collapse|Expand|Twirl|Turbulence|");

		m_wndToolBar = GetDlgItem(IDC_ET_SHAPETOOL);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		//if (IsThemingSupported() && !IsThemeNull())
		{
			//int nButtonSize = XPGUI::GetSmallIconSize()*1.625f + 0.5f;
			//m_wndToolBar.SetButtonSize(nButtonSize, nButtonSize);
			//m_wndToolBar.SetButtonWidth(nButtonSize, nButtonSize*16);
			//m_cToolbars[i].wndToolBar.SetPadding(0, 0);
		}
		m_wndToolBar.SetImageList(m_cImageList);
		m_wndToolBar.AddStrings(szTooltipStrings);
		TBBUTTON pButtons[CEditToolDataShapeShift::ESTCount];
		for (int i = 0; i < CEditToolDataShapeShift::ESTCount; ++i)
		{
			pButtons[i].iBitmap = i;
			pButtons[i].idCommand = i+1+IDC_ET_SHAPETOOL;
			pButtons[i].fsState = TBSTATE_ENABLED|(m_cData.eTool == i ? TBSTATE_CHECKED : 0);
			pButtons[i].fsStyle = BTNS_BUTTON|BTNS_CHECKGROUP;
			//pButtons[i].bReserved[2 or 6];          // padding for alignment
			pButtons[i].dwData = 0;
			pButtons[i].iString = i;
		}
		RECT rcOrig;
		GetClientRect(&rcOrig);
		m_wndToolBar.AddButtons(CEditToolDataShapeShift::ESTCount, pButtons);
		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcMargins = {0, 0, 10, 10};
		MapDialogRect(&rcMargins);
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		m_tOptimumSize.cx = rcOrig.right;//rcToolbar.right+rcMargins.right;
		LONG nDeltaY = rcToolbar.bottom-rcActual.bottom+rcActual.top;
		m_tOptimumSize.cy = rcOrig.bottom+nDeltaY;
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);
		static LONG s_aToMove[] = {
			IDC_ET_SIZELABEL, IDC_ET_SIZEEDIT, IDC_ET_SIZESPIN, IDC_ET_SIZEUNIT,
			IDC_ET_INTENSITYLABEL, IDC_ET_INTENSITYCHECK, IDC_ET_INTENSITYEDIT, IDC_ET_INTENSITYSPIN, IDC_ET_INTENSITYUNIT};
		for (ULONG i = 0; i < itemsof(s_aToMove); ++i)
		{
			CWindow wnd = GetDlgItem(s_aToMove[i]);
			RECT rc;
			wnd.GetWindowRect(&rc);
			ScreenToClient(&rc);
			rc.top += nDeltaY;
			rc.bottom += nDeltaY;
			wnd.MoveWindow(&rc);
		}


		DataToGUI();

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		m_cData.eTool = static_cast<CEditToolDataShapeShift::EShapeTool>(a_wID-IDC_ET_SHAPETOOL-1);
		m_wndToolBar.CheckRadioButton(IDC_ET_SHAPETOOL+1, IDC_ET_SHAPETOOL+CEditToolDataShapeShift::ESTCount, a_wID);
		DataToState();
		return 0;
	}
	LRESULT OnChangeIntPressure(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;
		m_cData.bIntPressure = IsDlgButtonChecked(IDC_ET_INTENSITYCHECK);
		DataToState();
		return 0;
	}
	LRESULT OnChangeSize(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fSize))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnChangeIntensity(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndIntensity.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fIntensity))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnUpDownChangeSize(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
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
			if (m_cData.fSize < 300.0f)
			{
				m_cData.fSize = min(300.0f, m_cData.fSize+1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fSize);
				m_wndSize.SetWindowText(szTmp);
			}
		}

		return 0;
	}
	LRESULT OnUpDownChangeIntensity(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fIntensity > 10.0f)
			{
				m_cData.fIntensity = max(10.0f, m_cData.fIntensity-10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fIntensity);
				m_wndIntensity.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fIntensity < 150.0f)
			{
				m_cData.fIntensity = min(150.0f, m_cData.fIntensity+10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fIntensity);
				m_wndIntensity.SetWindowText(szTmp);
			}
		}

		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		m_wndSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fSize);
		szVal[0] = _T('\0');
		m_wndIntensity.GetWindowText(szVal, itemsof(szVal));
		_stscanf(szVal, _T("%f"), &m_cData.fIntensity);
		m_cData.bIntPressure = IsDlgButtonChecked(IDC_ET_INTENSITYCHECK);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		if (!m_wndToolBar.IsButtonChecked(IDC_ET_SHAPETOOL+1+m_cData.eTool))
			m_wndToolBar.CheckRadioButton(IDC_ET_SHAPETOOL+1, IDC_ET_SHAPETOOL+CEditToolDataShapeShift::ESTCount, IDC_ET_SHAPETOOL+1+m_cData.eTool);
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
		m_wndIntensity.GetWindowText(szPrev, itemsof(szPrev));
		f = m_cData.fIntensity+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fIntensity)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fIntensity);
			m_wndIntensity.SetWindowText(szTmp);
		}
		CheckDlgButton(IDC_ET_INTENSITYCHECK, m_cData.bIntPressure ? BST_CHECKED : BST_UNCHECKED);
		m_bEnableUpdates = true;
	}

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedState> pState;
			RWCoCreateInstance(pState, __uuidof(SharedStateString));
			CComBSTR bstr;
			m_cData.ToString(&bstr);
			pState->FromText(bstr);
			m_bEnableUpdates = false;
			m_pSharedState->StateSet(m_bstrSyncToolData, pState);
			m_bEnableUpdates = true;
		}
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tOptimumSize;
	CEdit m_wndSize;
	CEdit m_wndIntensity;

	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncToolData;
	CEditToolDataShapeShift m_cData;
	bool m_bEnableUpdates;
};


