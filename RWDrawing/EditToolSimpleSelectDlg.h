// EditToolMoveSelectDlg.h : Declaration of the CEditToolSimpleSelectDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolSimpleSelectDlg

class CEditToolSimpleSelectDlg : 
	public CEditToolDlgBase<CEditToolSimpleSelectDlg, CEditToolDataSimpleSelect>,
	//public CDialogResize<CEditToolSimpleSelectDlg>,
	public CContextHelpDlg<CEditToolSimpleSelectDlg>
{
public:
	CEditToolSimpleSelectDlg()
	{
	}

	enum { IDD = IDD_EDITTOOL_MOVESELECT };

	BEGIN_MSG_MAP(CEditToolSimpleSelectDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolSimpleSelectDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_NOFILTERING, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_LINEARFILTERING, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_CUBICFILTERING, BN_CLICKED, OnChange)
		//CHAIN_MSG_MAP(CDialogResize<CEditToolSimpleSelectDlg>)
	END_MSG_MAP()

	//BEGIN_DLGRESIZE_MAP(CEditToolSimpleSelectDlg)
	//	DLGRESIZE_CONTROL(IDC_ET_ORIGINALSIZE, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_FILLAREA, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
	//END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolSimpleSelectDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_NOFILTERING, IDS_HELP_NOFILTERING)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_LINEARFILTERING, IDS_HELP_LINEARFILTERING)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_CUBICFILTERING, IDS_HELP_CUBICFILTERING)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolSimpleSelectDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataSimpleSelect::ISharedStateToolData> pData(a_tParams.pState);
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
		return S_OK;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataSimpleSelect::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataSimpleSelect::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
	LRESULT ClickedOriginalSize(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		++m_cData.bOriginal;
		return OnChange(a_wNotifyCode, a_wID, a_hWndCtl, a_bHandled);
	}
	LRESULT ClickedEntireArea(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		++m_cData.bEntireArea;
		return OnChange(a_wNotifyCode, a_wID, a_hWndCtl, a_bHandled);
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
		m_cData.eFilteringType = IsDlgButtonChecked(IDC_ET_NOFILTERING) ? CEditToolDataSimpleSelect::EFTNearest :
			(IsDlgButtonChecked(IDC_ET_LINEARFILTERING) ? CEditToolDataSimpleSelect::EFTLinear : CEditToolDataSimpleSelect::EFTCubic);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_ET_NOFILTERING, m_cData.eFilteringType == CEditToolDataSimpleSelect::EFTNearest ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_LINEARFILTERING, m_cData.eFilteringType == CEditToolDataSimpleSelect::EFTLinear ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_CUBICFILTERING, m_cData.eFilteringType == CEditToolDataSimpleSelect::EFTCubic ? BST_CHECKED : BST_UNCHECKED);
		m_bEnableUpdates = true;
	}

private:
};


