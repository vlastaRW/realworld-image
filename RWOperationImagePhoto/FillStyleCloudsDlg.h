// FillStyleCloudsDlg.h : Declaration of the CFillStyleCloudsDlg

#pragma once

#include "resource.h"       // main symbols
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include <WTL_ColorPicker.h>



// CFillStyleCloudsDlg

class CFillStyleCloudsDlg : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CFillStyleCloudsDlg>,
	public CObserverImpl<CFillStyleCloudsDlg, ISharedStateObserver, TSharedStateChange>,
	public CDialogResize<CFillStyleCloudsDlg>,
	public CContextHelpDlg<CFillStyleCloudsDlg>,
	public CChildWindowImpl<CFillStyleCloudsDlg, IRasterImageEditToolWindow>
{
public:
	CFillStyleCloudsDlg() : m_bEnableUpdates(false)
	{
	}
	~CFillStyleCloudsDlg()
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}

	enum { IDD = IDD_FILLSTYLE_CLOUDS };

	BEGIN_MSG_MAP(CFillStyleCloudsDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CFillStyleCloudsDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ETW_AUTOCENTER, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ETW_DEFAULTCOLORS, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ETW_SEED, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ETW_SCALE, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ETW_STRETCH, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ETW_ANGLE, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ETC_AMOUNT, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ETC_BIAS, EN_CHANGE, OnChangeEdit)
		NOTIFY_HANDLER(IDC_ETW_COLOR1, CButtonColorPicker::BCPN_SELCHANGE, OnColor1Changed)
		NOTIFY_HANDLER(IDC_ETW_COLOR2, CButtonColorPicker::BCPN_SELCHANGE, OnColor2Changed)
		CHAIN_MSG_MAP(CDialogResize<CFillStyleCloudsDlg>)
		REFLECT_NOTIFICATIONS();
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFillStyleCloudsDlg)
		DLGRESIZE_CONTROL(IDC_ETW_SEED, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ETW_SCALE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ETW_STRETCH, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ETW_ANGLE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ETC_AMOUNT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ETC_BIAS, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CFillStyleCloudsDlg)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_PERSPECTIVECROP, IDS_HELP_PERSPECTIVECROP)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_NORMALCROP, IDS_HELP_NORMALCROP)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_JPEGCROP, IDS_HELP_JPEGCROP)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_ASPECTRATIO, IDS_HELP_ASPECTRATIO)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CFillStyleCloudsDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup)
	{
		m_tLocaleID = a_tLocaleID;
		m_pSharedState = a_pSharedState;
		m_bstrSyncToolData = a_bstrSyncGroup;
		m_pSharedState->ObserverIns(ObserverGet(), 0);
		return Win32LangEx::CLangDialogImpl<CFillStyleCloudsDlg>::Create(a_hParent);
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedState> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateString));
			CComBSTR bstr;
			m_cData.ToString(&bstr);
			pTmp->FromText(bstr);
			m_bEnableUpdates = false;
			m_pSharedState->StateSet(m_bstrSyncToolData, pTmp);
			m_bEnableUpdates = true;
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComBSTR bstr;
			a_tParams.pState->ToText(&bstr);
			if (bstr)
			{
				m_cData.FromString(bstr);
				if (m_bEnableUpdates)
					DataToGUI();
			}
		}
	}

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

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd && (
			a_pMsg->hwnd == m_wndSeed || a_pMsg->hwnd == m_wndScale ||
			a_pMsg->hwnd == m_wndStretch || a_pMsg->hwnd == m_wndAngle ||
			a_pMsg->hwnd == m_wndAmount || a_pMsg->hwnd == m_wndBias))
		{
			if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
				return S_OK;
			TranslateMessage(a_pMsg);
			DispatchMessage(a_pMsg);
			return S_OK;
		}
		return S_FALSE;
	}


	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndAutoCenter = GetDlgItem(IDC_ETW_AUTOCENTER);
		TColor const tColor1 = CFillStyleDataClouds::DefaultColor1();
		m_wndColor1.SubclassWindow(GetDlgItem(IDC_ETW_COLOR1));
		m_wndColor1.SetDefaultColor(CButtonColorPicker::SColor(tColor1.fR, tColor1.fG, tColor1.fB, tColor1.fA));
		TColor const tColor2 = CFillStyleDataClouds::DefaultColor2();
		m_wndColor2.SubclassWindow(GetDlgItem(IDC_ETW_COLOR2));
		m_wndColor2.SetDefaultColor(CButtonColorPicker::SColor(tColor2.fR, tColor2.fG, tColor2.fB, tColor2.fA));
		m_wndSeed = GetDlgItem(IDC_ETW_SEED);
		m_wndScale = GetDlgItem(IDC_ETW_SCALE);
		m_wndStretch = GetDlgItem(IDC_ETW_STRETCH);
		m_wndAngle = GetDlgItem(IDC_ETW_ANGLE);
		m_wndAmount = GetDlgItem(IDC_ETC_AMOUNT);
		m_wndBias = GetDlgItem(IDC_ETC_BIAS);
		m_aMap[0] = std::make_pair(&m_wndScale, &m_cData.scale);
		m_aMap[1] = std::make_pair(&m_wndStretch, &m_cData.stretch);
		m_aMap[2] = std::make_pair(&m_wndAngle, &m_cData.angle);
		m_aMap[3] = std::make_pair(&m_wndAmount, &m_cData.amount);
		m_aMap[4] = std::make_pair(&m_wndBias, &m_cData.bias);

		if (m_pSharedState != NULL)
		{
			CComPtr<ISharedState> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComBSTR bstr;
			if (pState != NULL) pState->ToText(&bstr);
			if (bstr) m_cData.FromString(bstr);
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

		GUIToData();
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeEdit(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		for (std::pair<CEdit*, float*>* p = m_aMap; p < m_aMap+itemsof(m_aMap); ++p)
		{
			if (a_hWndCtl == p->first->m_hWnd)
			{
				p->first->GetWindowText(szVal, itemsof(szVal));
				szVal[itemsof(szVal)-1] = _T('\0');
				if (1 != _stscanf(szVal, _T("%f"), p->second))
					return 0;
				GUIToData();
				DataToState();
				return 0;
			}
		}
		if (a_hWndCtl == m_wndSeed)
		{
			m_wndSeed.GetWindowText(szVal, itemsof(szVal));
			szVal[itemsof(szVal)-1] = _T('\0');
			if (1 != _stscanf(szVal, _T("%i"), &m_cData.nSeed))
				return 0;
			GUIToData();
			DataToState();
		}
		return 0;
	}
	LRESULT OnColor1Changed(int, LPNMHDR a_pHdr, BOOL&)
	{
		if (!m_bEnableUpdates)
			return 0;

		CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
		m_cData.tColor1.fR = pClr->clr.fR;
		m_cData.tColor1.fG = pClr->clr.fG;
		m_cData.tColor1.fB = pClr->clr.fB;
		m_cData.tColor1.fA = pClr->clr.fA;

		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnColor2Changed(int, LPNMHDR a_pHdr, BOOL&)
	{
		if (!m_bEnableUpdates)
			return 0;

		CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
		m_cData.tColor2.fR = pClr->clr.fR;
		m_cData.tColor2.fG = pClr->clr.fG;
		m_cData.tColor2.fB = pClr->clr.fB;
		m_cData.tColor2.fA = pClr->clr.fA;

		DataToState();
		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		for (std::pair<CEdit*, float*>* p = m_aMap; p < m_aMap+itemsof(m_aMap); ++p)
		{
			p->first->GetWindowText(szVal, itemsof(szVal));
			szVal[itemsof(szVal)-1] = _T('\0');
			_stscanf(szVal, _T("%f"), p->second);
		}
		m_cData.bAutoCenter = m_wndAutoCenter.GetCheck() == BST_CHECKED;
		m_wndSeed.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%i"), &m_cData.nSeed);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		TCHAR szPrev[33] = _T("");
		for (std::pair<CEdit*, float*>* p = m_aMap; p < m_aMap+itemsof(m_aMap); ++p)
		{
			p->first->GetWindowText(szPrev, itemsof(szPrev));
			float f = *p->second+1;
			_stscanf(szPrev, _T("%f"), &f);
			if (f != *p->second)
			{
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), *p->second);
				p->first->SetWindowText(szTmp);
			}
		}
		m_wndSeed.GetWindowText(szPrev, itemsof(szPrev));
		int f = m_cData.nSeed+1;
		_stscanf(szPrev, _T("%i"), &f);
		if (f != m_cData.nSeed)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%i"), m_cData.nSeed);
			m_wndSeed.SetWindowText(szTmp);
		}
		m_wndAutoCenter.SetCheck(m_cData.bAutoCenter ? BST_CHECKED : BST_UNCHECKED);
		m_wndColor1.SetColor(CButtonColorPicker::SColor(m_cData.tColor1.fR, m_cData.tColor1.fG, m_cData.tColor1.fB, m_cData.tColor1.fA), true);
		m_wndColor2.SetColor(CButtonColorPicker::SColor(m_cData.tColor2.fR, m_cData.tColor2.fG, m_cData.tColor2.fB, m_cData.tColor2.fA), true);
		m_bEnableUpdates = true;
	}

private:
	std::pair<CEdit*, float*> m_aMap[5];
	CButton m_wndAutoCenter;
	CButtonColorPicker m_wndColor1;
	CButtonColorPicker m_wndColor2;
	CEdit m_wndSeed;
	CEdit m_wndScale;
	CEdit m_wndStretch;
	CEdit m_wndAngle;
	CEdit m_wndAmount;
	CEdit m_wndBias;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncToolData;
	CFillStyleDataClouds m_cData;
	bool m_bEnableUpdates;
};


