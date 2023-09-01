// EditToolFloodFillDlg.h : Declaration of the CEditToolFloodFillDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolFloodFillDlg

class CEditToolFloodFillDlg : 
	public CEditToolDlgBase<CEditToolFloodFillDlg, CEditToolDataFloodFill>,
	public CDialogResize<CEditToolFloodFillDlg>,
	public CContextHelpDlg<CEditToolFloodFillDlg>
{
public:
	CEditToolFloodFillDlg()
	{
	}
	~CEditToolFloodFillDlg()
	{
	}

	enum { IDD = IDD_EDITTOOL_FLOODFILL };

	BEGIN_MSG_MAP(CEditToolFloodFillDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolFloodFillDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_SIMPLEFILL, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_MULTIPOINT, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_RGBA, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_HUE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_ALPHA, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_BRIGHTNESS, BN_CLICKED, OnChange)
		//COMMAND_HANDLER(IDC_ET_SELECTION, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_TOLERANCE, EN_CHANGE, OnChangeEdit)
		CHAIN_MSG_MAP(CDialogResize<CEditToolFloodFillDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolFloodFillDlg)
		DLGRESIZE_CONTROL(IDC_ET_TOLERANCE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_TOLERANCESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_TOLERANCELABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIMPLEFILL, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_MULTIPOINT, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_RGBA, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_HUE, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_ALPHA, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_BRIGHTNESS, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE2, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolFloodFillDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_TOLERANCE, IDS_HELP_TOLERANCE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_TOLERANCESPIN, IDS_HELP_TOLERANCE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_RGBA, IDS_HELP_MATCHRGBA)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_HUE, IDS_HELP_MATCHHUE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_ALPHA, IDS_HELP_MATCHALPHA)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BRIGHTNESS, IDS_HELP_MATCHBRIGHTNESS)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_SELECTION, IDS_HELP_MATCHALL)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_MULTIPOINT, IDS_HELP_MULTIPOINT)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolFloodFillDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataFloodFill::ISharedStateToolData> pData(a_tParams.pState);
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
		AddWindowForPreTranslate(GetDlgItem(IDC_ET_TOLERANCE));

		CUpDownCtrl(GetDlgItem(IDC_ET_TOLERANCESPIN)).SetRange(0, 255);
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataFloodFill::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataFloodFill::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}
		DataToGUI();

		DlgResize_Init(false, false, 0);

		m_bEnableUpdates = true;

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

		BOOL b = FALSE;
		UINT n = GetDlgItemInt(IDC_ET_TOLERANCE, &b, FALSE);
		if (!b)
			return 0;
		m_cData.nTolerance = n;
		GUIToData();
		DataToState();
		return 0;
	}

	void GUIToData()
	{
		m_cData.bMultipoint = IsDlgButtonChecked(IDC_ET_MULTIPOINT);
		m_cData.eMode = IsDlgButtonChecked(IDC_ET_RGBA) ? CEditToolDataFloodFill::EMMRGBA : (
			IsDlgButtonChecked(IDC_ET_HUE) ? CEditToolDataFloodFill::EMMHue : (
			IsDlgButtonChecked(IDC_ET_ALPHA) ? CEditToolDataFloodFill::EMMAlpha : (
			/*IsDlgButtonChecked(IDC_ET_BRIGHTNESS) ? */CEditToolDataFloodFill::EMMBrightness/* : (
			CEditToolDataFloodFill::EMMSelection)*/)));
		BOOL b = FALSE;
		UINT n = GetDlgItemInt(IDC_ET_TOLERANCE, &b, FALSE);
		if (b)
			m_cData.nTolerance = n;
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_ET_MULTIPOINT, m_cData.bMultipoint ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_SIMPLEFILL, m_cData.bMultipoint ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(IDC_ET_RGBA, m_cData.eMode == CEditToolDataFloodFill::EMMRGBA ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_HUE, m_cData.eMode == CEditToolDataFloodFill::EMMHue ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_ALPHA, m_cData.eMode == CEditToolDataFloodFill::EMMAlpha ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_BRIGHTNESS, m_cData.eMode == CEditToolDataFloodFill::EMMBrightness ? BST_CHECKED : BST_UNCHECKED);
		//CheckDlgButton(IDC_ET_SELECTION, m_cData.eMode == CEditToolDataFloodFill::EMMSelection ? BST_CHECKED : BST_UNCHECKED);
		if (m_cData.nTolerance != GetDlgItemInt(IDC_ET_TOLERANCE))
			SetDlgItemInt(IDC_ET_TOLERANCE, m_cData.nTolerance, FALSE);
		m_bEnableUpdates = true;
	}
};


