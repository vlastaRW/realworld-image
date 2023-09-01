// FillStyleSolidDlg.h : Declaration of the CFillStyleSolidDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <WTL_ColorPicker.h>


// CFillStyleSolidDlg

class CFillStyleSolidDlg : 
	public CEditToolDlgBase<CFillStyleSolidDlg, CFillStyleDataSolid, Win32LangEx::CLangIndirectDialogImpl<CFillStyleSolidDlg> >,
	public CObserverImpl<CFillStyleSolidDlg, IColorPickerObserver, ULONG>
{
public:
	CFillStyleSolidDlg()
	{
	}

	enum { IDC_COLOR = 100 };

	BEGIN_DIALOG_EX(0, 0, 100, 24, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CFillStyleSolidDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
	END_MSG_MAP()

	BEGIN_COM_MAP(CFillStyleSolidDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CFillStyleDataSolid::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				if (m_bEnableUpdates)
					DataToGUI();
			}
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (m_pWnd)
			{
				if (a_pSize->cx)
					a_pSize->cx -= m_rcMargins.left<<1;
				if (a_pSize->cy)
					a_pSize->cy -= m_rcMargins.top<<1;
				HRESULT hRes = m_pWnd->OptimumSize(a_pSize);
				a_pSize->cx += m_rcMargins.left<<1;
				a_pSize->cy += m_rcMargins.top<<1;
				return hRes;
			}
			//if (m_tOptimumSize.cx && m_tOptimumSize.cy)
			//	*a_pSize = m_tOptimumSize;
			else
				GetDialogSize(a_pSize, m_tLocaleID);
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		if (!m_bEnableUpdates)
			return S_FALSE;

		CComQIPtr<CFillStyleDataSolid::ISharedStateToolData> pState(a_pState);
		if (pState != NULL)
		{
			m_cData = *(pState->InternalData());
			DataToGUI();
		}
		return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_pWnd ? m_pWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pSharedState != NULL)
		{
			CComPtr<CFillStyleDataSolid::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CFillStyleDataSolid::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		RECT rcClient = {0, 0, 0, 0};
		GetClientRect(&rcClient);
		m_rcMargins.top = m_rcMargins.left = 5;
		m_rcMargins.bottom = m_rcMargins.right = 10;
		MapDialogRect(&m_rcMargins);
		RECT rcWnd = {m_rcMargins.left, m_rcMargins.top, rcClient.right-m_rcMargins.left, rcClient.bottom-m_rcMargins.top};
		RWCoCreateInstance(m_pWnd, __uuidof(ColorPicker));
		m_pWnd->Create(m_hWnd, &rcWnd, TRUE, m_tLocaleID, NULL, FALSE, CComBSTR(L"SOLID"), NULL);
		m_pWnd->ObserverIns(CObserverImpl<CFillStyleSolidDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);

		m_bEnableUpdates = true;

		DataToGUI();

		return 1;  // Let the system set the focus
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_pWnd->ObserverDel(CObserverImpl<CFillStyleSolidDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rcWnd = {m_rcMargins.left, m_rcMargins.top, GET_X_LPARAM(a_lParam)-m_rcMargins.left, GET_Y_LPARAM(a_lParam)-m_rcMargins.top};
		if (m_pWnd) m_pWnd->Move(&rcWnd);
		return 0;
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		if (!m_bEnableUpdates)
			return;

		if (a_nFlags&ECPCColor)
		{
			GUIToData();
			DataToState();
		}
	}

	void GUIToData()
	{
		m_pWnd->ColorGet(&m_cData.tColor);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_pWnd->ColorSet(&m_cData.tColor);
		m_bEnableUpdates = true;
	}

private:
	CComPtr<IColorPicker> m_pWnd;
	RECT m_rcMargins;
};


