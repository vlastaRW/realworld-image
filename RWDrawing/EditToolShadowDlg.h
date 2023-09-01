// EditToolShadowDlg.h : Declaration of the CEditToolShadowDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolShadowDlg

class CEditToolShadowDlg : 
	public CEditToolDlgBase<CEditToolShadowDlg, CEditToolDataShadow>,
	public CDialogResize<CEditToolShadowDlg>,
	public CContextHelpDlg<CEditToolShadowDlg>
{
public:
	CEditToolShadowDlg() : m_hIcon(NULL)
	{
	}
	~CEditToolShadowDlg()
	{
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

	enum { IDD = IDD_EDITTOOL_SHADOW };

	BEGIN_MSG_MAP(CEditToolShadowDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolShadowDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_NEARDENS, EN_CHANGE, OnChangeEdit)
		COMMAND_HANDLER(IDC_ET_FARDENS, EN_CHANGE, OnChangeEdit)
		CHAIN_MSG_MAP(CDialogResize<CEditToolShadowDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolShadowDlg)
		DLGRESIZE_CONTROL(IDC_ET_BLUR, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_BLURMAX, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_NEARDENS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_NEARDENSLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_NEARDENSSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_FARDENS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_FARDENSLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_FARDENSSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_HELPIMAGE, DLSZ_DIVMOVE_X(2)|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_ET_HELPTEXT, DLSZ_SIZE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolShadowDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BLUR, IDS_HELP_BLUR)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_NEARDENS, IDS_HELP_NEARDENS)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_NEARDENSSPIN, IDS_HELP_NEARDENS)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FARDENS, IDS_HELP_FARDENS)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FARDENSSPIN, IDS_HELP_FARDENS)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolShadowDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataShadow::ISharedStateToolData> pData(a_tParams.pState);
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
			return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* UNREF(a_pState))
	{
		//if (!m_bEnableUpdates)
		//	return;

		//if (m_pSharedState != NULL)
		//{
		//	CComPtr<CEditToolDataLine::ISharedStateToolData> pState;
		//	m_pSharedState->StateGet(m_bstrSyncGroup, __uuidof(CEditToolDataLine::ISharedStateToolData), reinterpret_cast<void**>(&pState));
		//	if (pState != NULL)
		//	{
		//		//pState->StateGet(NULL, NULL, NULL, NULL, &m_eRasterizationMode);
		//		m_cData.Deserialize(m_pszToolID, pState);
		//		DataToGUI();
		//	}
		//}
		return S_OK;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		AddWindowForPreTranslate(GetDlgItem(IDC_ET_NEARDENS));
		AddWindowForPreTranslate(GetDlgItem(IDC_ET_FARDENS));

		m_wndBlur = GetDlgItem(IDC_ET_BLUR);
		m_wndBlur.SetRange(0, 30);
		m_wndBlur.SetTicFreq(6);

		CUpDownCtrl(GetDlgItem(IDC_ET_NEARDENSSPIN)).SetRange(0, 100);
		CUpDownCtrl(GetDlgItem(IDC_ET_FARDENSSPIN)).SetRange(0, 100);

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataShadow::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataShadow::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}
		DataToGUI();

		CStatic cImg(GetDlgItem(IDC_ET_HELPIMAGE));
		RECT rcImg;
		cImg.GetWindowRect(&rcImg);
		rcImg.top = rcImg.bottom-64;
		rcImg.right = rcImg.left+64;
		ScreenToClient(&rcImg);
		cImg.MoveWindow(&rcImg);
		m_hIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_HOWTO_SHADOW), IMAGE_ICON, 64, 64, LR_DEFAULTCOLOR);
		cImg.SetIcon(m_hIcon);

		DlgResize_Init(false, false, 0);

		m_bEnableUpdates = true;

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
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

		m_cData.nNearDensity = GetDlgItemInt(IDC_ET_NEARDENS);
		m_cData.nFarDensity = GetDlgItemInt(IDC_ET_FARDENS);
		GUIToData();
		DataToState();
		return 0;
	}

	void GUIToData()
	{
		m_cData.nNearDensity = GetDlgItemInt(IDC_ET_NEARDENS);
		m_cData.nFarDensity = GetDlgItemInt(IDC_ET_FARDENS);
		m_cData.nBlur = m_wndBlur.GetPos();
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		if (GetDlgItemInt(IDC_ET_NEARDENS) != m_cData.nNearDensity)
			SetDlgItemInt(IDC_ET_NEARDENS, m_cData.nNearDensity);
		if (GetDlgItemInt(IDC_ET_FARDENS) != m_cData.nFarDensity)
			SetDlgItemInt(IDC_ET_FARDENS, m_cData.nFarDensity);
		m_wndBlur.SetPos(m_cData.nBlur);
		m_bEnableUpdates = true;
	}

private:
	CTrackBarCtrl m_wndBlur;
	HICON m_hIcon;
};


