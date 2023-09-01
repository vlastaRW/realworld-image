// EditToolMoveDlg.h : Declaration of the CEditToolMoveDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolMoveDlg

class CEditToolMoveDlg : 
	public CEditToolDlgBase<CEditToolMoveDlg, CEditToolDataMove, Win32LangEx::CLangIndirectDialogImpl<CEditToolMoveDlg> >,
	//public CDialogResize<CEditToolMoveDlg>,
	public CContextHelpDlg<CEditToolMoveDlg>
{
public:
	CEditToolMoveDlg()
	{
	}

	BEGIN_DIALOG_EX(0, 0, 100, 35, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_ET_MOVEWRAP = 100,
		IDC_ET_MOVEFILL,
		IDC_ET_FILLCOLOR,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_AUTORADIOBUTTON(_T("[0409]Wrap[0405]Obtočit"), IDC_ET_MOVEWRAP, 5, 5, 55, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_AUTORADIOBUTTON(_T("[0409]Fill[0405]Vyplnit"), IDC_ET_MOVEFILL, 5, 20, 55, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolMoveDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolMoveDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_MOVEWRAP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_MOVEFILL, BN_CLICKED, OnChange)
		//CHAIN_MSG_MAP(CDialogResize<CEditToolMoveDlg>)
	END_MSG_MAP()

	//BEGIN_DLGRESIZE_MAP(CEditToolMoveDlg)
	//	DLGRESIZE_CONTROL(IDC_ET_ORIGINALSIZE, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_FILLAREA, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
	//END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolMoveDlg)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_NOFILTERING, IDS_HELP_NOFILTERING)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_LINEARFILTERING, IDS_HELP_LINEARFILTERING)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_CUBICFILTERING, IDS_HELP_CUBICFILTERING)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolMoveDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataMove::ISharedStateToolData> pData(a_tParams.pState);
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
			AtlGetDialogSize((LPCDLGTEMPLATE)m_OrigTemplate.GetTemplatePtr(), a_pSize);
			return S_OK;
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

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataMove::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataMove::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}
		DataToGUI();

		//DlgResize_Init(false, false, 0);

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

	void GUIToData()
	{
		m_cData.eExterior = IsDlgButtonChecked(IDC_ET_MOVEWRAP) ? CEditToolDataMove::EEWrap : CEditToolDataMove::EEFill;
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_ET_MOVEWRAP, m_cData.eExterior == CEditToolDataMove::EEWrap ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_MOVEFILL, m_cData.eExterior == CEditToolDataMove::EEFill ? BST_CHECKED : BST_UNCHECKED);
		m_bEnableUpdates = true;
	}
};


