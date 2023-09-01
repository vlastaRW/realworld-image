// EditToolTransformDlg.h : Declaration of the CEditToolTransformationDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolTransformationDlg

class CEditToolTransformationDlg : 
	public CEditToolDlgBase<CEditToolTransformationDlg, CEditToolDataTransformation, Win32LangEx::CLangIndirectDialogImpl<CEditToolTransformationDlg>>,
	//public CDialogResize<CEditToolTransformationDlg>,
	public CContextHelpDlg<CEditToolTransformationDlg>
{
public:
	CEditToolTransformationDlg()
	{
	}

	enum
	{
		IDC_ET_RECTANGULARXFORM = 100,
		IDC_ET_PERSPECTIVEXFORM,
		IDC_ET_BEZIERXFORM,
		IDC_ET_PRESERVEASPECT,
	};

	BEGIN_DIALOG_EX(0, 0, 110, 62, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_AUTORADIOBUTTON(_T("[0409]Move, resize and rotate[0405]Posun, škálování a rotace"), IDC_ET_RECTANGULARXFORM, 5, 5, 100, 10, WS_GROUP | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_AUTORADIOBUTTON(_T("[0409]Perspective transformation[0405]Perspektivní transformace"), IDC_ET_PERSPECTIVEXFORM, 5, 33, 100, 10, WS_VISIBLE, 0)
		CONTROL_AUTORADIOBUTTON(_T("[0409]Bézier transformation[0405]Bézierova transformace"), IDC_ET_BEZIERXFORM, 5, 47, 100, 10, WS_VISIBLE, 0)
		CONTROL_AUTOCHECKBOX(_T("[0409]Preserve aspect ratio[0405]Zachovat poměr stran"), IDC_ET_PRESERVEASPECT, 16, 19, 89, 10, WS_GROUP | WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CEditToolTransformationDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolTransformationDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_RECTANGULARXFORM, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_PERSPECTIVEXFORM, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_BEZIERXFORM, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_PRESERVEASPECT, BN_CLICKED, OnChange)
		//CHAIN_MSG_MAP(CDialogResize<CEditToolTransformationDlg>)
	END_MSG_MAP()

	//BEGIN_DLGRESIZE_MAP(CEditToolTransformationDlg)
	//	DLGRESIZE_CONTROL(IDC_ET_ORIGINALSIZE, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_FILLAREA, DLSZ_DIVMOVE_X(2))
	//	DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
	//END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolTransformationDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_RECTANGULARXFORM, IDS_HELP_RECTANGULARXFORM)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_PERSPECTIVEXFORM, IDS_HELP_PERSPECTIVEXFORM)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BEZIERXFORM, IDS_HELP_BEZIERXFORM)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolTransformationDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataTransformation::ISharedStateToolData> pData(a_tParams.pState);
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
			GetDialogSize(a_pSize, m_tLocaleID);
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
			CComPtr<CEditToolDataTransformation::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataTransformation::ISharedStateToolData), reinterpret_cast<void**>(&pState));
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
		m_cData.eMode = IsDlgButtonChecked(IDC_ET_RECTANGULARXFORM) ? CEditToolDataTransformation::EMSimple :
			(IsDlgButtonChecked(IDC_ET_PERSPECTIVEXFORM) ? CEditToolDataTransformation::EMPerspective : CEditToolDataTransformation::EMBezier);
		m_cData.bAspectLock = IsDlgButtonChecked(IDC_ET_PRESERVEASPECT);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_ET_RECTANGULARXFORM, m_cData.eMode == CEditToolDataTransformation::EMSimple ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_PERSPECTIVEXFORM, m_cData.eMode == CEditToolDataTransformation::EMPerspective ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_BEZIERXFORM, m_cData.eMode == CEditToolDataTransformation::EMBezier ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_PRESERVEASPECT, m_cData.bAspectLock ? BST_CHECKED : BST_UNCHECKED);
		GetDlgItem(IDC_ET_PRESERVEASPECT).EnableWindow(m_cData.eMode == CEditToolDataTransformation::EMSimple);
		m_bEnableUpdates = true;
	}

private:
};


