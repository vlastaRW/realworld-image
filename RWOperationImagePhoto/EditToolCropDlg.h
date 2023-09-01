// EditToolCropDlg.h : Declaration of the CEditToolCropDlg

#pragma once

#include "resource.h"       // main symbols
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include <ObserverImpl.h>


extern __declspec(selectany) CEditToolDataCrop::EProportionsMode const m_aProportions[]=
{
	CEditToolDataCrop::EPMArbitrary,
	CEditToolDataCrop::EPM1_1,
	CEditToolDataCrop::EPM3_2,
	CEditToolDataCrop::EPM4_3,
	CEditToolDataCrop::EPM5_4,
	CEditToolDataCrop::EPM16_10,
	CEditToolDataCrop::EPM16_9,
};

// CEditToolCropDlg

class CEditToolCropDlg : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangDialogImpl<CEditToolCropDlg>,
	public CObserverImpl<CEditToolCropDlg, ISharedStateObserver, TSharedStateChange>,
	public CDialogResize<CEditToolCropDlg>,
	public CContextHelpDlg<CEditToolCropDlg>,
	public CChildWindowImpl<CEditToolCropDlg, IRasterImageEditToolWindow>
{
public:
	CEditToolCropDlg() : m_bEnableUpdates(false)
	{
	}
	~CEditToolCropDlg()
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}

	enum { IDD = IDD_EDITTOOL_CROP };

	BEGIN_MSG_MAP(CEditToolCropDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolCropDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_PERSPECTIVECROP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_NORMALCROP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_JPEGCROP, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_ASPECTRATIO, CBN_SELCHANGE, OnChange)
		CHAIN_MSG_MAP(CDialogResize<CEditToolCropDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolCropDlg)
		DLGRESIZE_CONTROL(IDC_ET_SEPLINE1, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_ASPECTRATIO, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolCropDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_PERSPECTIVECROP, IDS_HELP_PERSPECTIVECROP)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_NORMALCROP, IDS_HELP_NORMALCROP)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_JPEGCROP, IDS_HELP_JPEGCROP)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_ASPECTRATIO, IDS_HELP_ASPECTRATIO)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolCropDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup)
	{
		m_tLocaleID = a_tLocaleID;
		m_pSharedState = a_pSharedState;
		m_bstrSyncToolData = a_bstrSyncGroup;
		m_pSharedState->ObserverIns(ObserverGet(), 0);
		return Win32LangEx::CLangDialogImpl<CEditToolCropDlg>::Create(a_hParent);
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(WM_CTLCOLORDLG, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComObject<CSharedStateEditToolCrop>* pNew = NULL;
			CComObject<CSharedStateEditToolCrop>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			m_bEnableUpdates = false;
			m_pSharedState->StateSet(m_bstrSyncToolData, pTmp);
			m_bEnableUpdates = true;
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataCrop::ISharedStateToolData> pData(a_tParams.pState);
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
		m_wndAspect = GetDlgItem(IDC_ET_ASPECTRATIO);
		for (size_t i = 0; i < itemsof(m_aProportions); ++i)
		{
			TCHAR szTmp[32];
			if (m_aProportions[i])
			{
				_stprintf(szTmp, _T("%i : %i"), m_aProportions[i]&0xffff, m_aProportions[i]>>16);
			}
			else
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_CROP_NOPROPORTIONS, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
			}
			m_wndAspect.AddString(szTmp);
		}

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataCrop::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataCrop::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
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

	void GUIToData()
	{
		m_cData.eCropMode = IsDlgButtonChecked(IDC_ET_PERSPECTIVECROP) ? CEditToolDataCrop::ECMPerspective :
			(IsDlgButtonChecked(IDC_ET_NORMALCROP) ? CEditToolDataCrop::ECMStandard : CEditToolDataCrop::ECMLosslessJPEG);
		int i = m_wndAspect.GetCurSel();
		if (i >= 0 && size_t(i) < itemsof(m_aProportions))
			m_cData.eProportionsMode = m_aProportions[i];
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_wndAspect.EnableWindow(m_cData.eCropMode != CEditToolDataCrop::ECMPerspective);
		CheckDlgButton(IDC_ET_PERSPECTIVECROP, m_cData.eCropMode == CEditToolDataCrop::ECMPerspective ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_NORMALCROP, m_cData.eCropMode == CEditToolDataCrop::ECMStandard ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_JPEGCROP, m_cData.eCropMode == CEditToolDataCrop::ECMLosslessJPEG ? BST_CHECKED : BST_UNCHECKED);
		for (size_t i = 0; i < itemsof(m_aProportions); ++i)
		{
			if (m_aProportions[i] == m_cData.eProportionsMode)
				m_wndAspect.SetCurSel(i);
		}
		m_bEnableUpdates = true;
	}

private:
	CComboBox m_wndAspect;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncToolData;
	CEditToolDataCrop m_cData;
	bool m_bEnableUpdates;
};


