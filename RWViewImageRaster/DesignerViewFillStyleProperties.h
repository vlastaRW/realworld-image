// DesignerViewFillStyleProperties.h : Declaration of the CDesignerViewFillStyleProperties

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <Win32LangEx.h>
#include <XPGUI.h>
#include <ObserverImpl.h>
#include <RWBaseEnumUtils.h>


// CDesignerViewFillStyleProperties

class ATL_NO_VTABLE CDesignerViewFillStyleProperties : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView,
	public CObserverImpl<CDesignerViewFillStyleProperties, ISharedStateObserver, TSharedStateChange>
{
public:
	CDesignerViewFillStyleProperties() : m_bFirstChange(true), m_bVisible(false)
	{
		m_rcWnd.left = m_rcWnd.top = m_rcWnd.right = m_rcWnd.bottom = 0;
	}
	~CDesignerViewFillStyleProperties()
	{
	}

	void Init(ISharedStateManager* a_pFrame, LPCOLESTR a_pszSyncGroup, HWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_tLocaleID = a_tLocaleID;
		m_rcWnd = *a_rcWindow;
		m_bVisible = true;
		m_hParent = a_hParent;

		if (a_pszSyncGroup == NULL || a_pszSyncGroup[0] == L'\0' || a_pFrame == NULL)
			throw E_FAIL;

		RWCoCreateInstance(m_pManager, __uuidof(RasterImageFillStyleManager));
		RWCoCreateInstance(m_pToolsManager, __uuidof(RasterImageEditToolsManager));

		m_bstrSyncGroup = a_pszSyncGroup;
		a_pFrame->ObserverIns(ObserverGet(), 0);
		m_pSharedState = a_pFrame;
		CComPtr<ISharedState> pState;
		m_pSharedState->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));

		if (pState)
		{
			TSharedStateChange tParams;
			tParams.bstrName = m_bstrSyncGroup;
			tParams.pState = pState;
			OwnerNotify(0, tParams);
		}
		else
		{
			SwitchTool(CComBSTR(L"SOLID"));
		}

		//if (m_pToolWindow == NULL)
		//	throw E_FAIL;
	}


BEGIN_COM_MAP(CDesignerViewFillStyleProperties)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pSharedState != NULL)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		try
		{
			if (m_pManager == NULL)
				return;

			if (m_bstrSyncGroup != a_tParams.bstrName)
				return; // notification is not for our group

			CComQIPtr<ISharedStateToolMode> pSel(a_tParams.pState);
			if (pSel == NULL)
				return; // unknown data format

			CComBSTR bstrToolID;
			CComBSTR bstrActiveStyleID;
			std::vector<ULONG> aPaints;
			BOOL bFill = TRUE;
			pSel->Get(&bstrToolID, &bFill, &bstrActiveStyleID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			if (m_pToolsManager) m_pToolsManager->SupportedStates(bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
			bool bRasterImage = false;
			bool bSolidColor = false;
			for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
			{
				if ((*i&ETBTIdentifierMask) != ETBTIdentifierInterior)
					continue;
				if ((*i&ETBTTypeMask) == ETBTSingleColor)
					bSolidColor = true;
				if ((*i&ETBTTypeMask) == ETBTRasterImage)
					bRasterImage = true;
			}
			if (!bRasterImage)
				if (bSolidColor)
					bstrActiveStyleID = L"SOLID";
				else
					bstrActiveStyleID.Empty();
			if (!bFill && aPaints.size() > 1)
				bstrActiveStyleID.Empty();

			if (bstrActiveStyleID != m_bstrStyleID || m_bstrStyleID.m_str == NULL)
			{
				bool bFocus = !m_bFirstChange && m_bstrStyleID.Length() && bstrActiveStyleID.Length();
				SwitchTool(bstrActiveStyleID);
				if (bFocus)
				{
					::SendMessage(m_hParent, WM_RW_GOTFOCUS, 0, 0);
				}
				m_bFirstChange = false;
			}
		}
		catch (...)
		{
		}
	}

	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND *a_pHandle)
	{
		return m_pToolWindow ? m_pToolWindow->Handle(a_pHandle) : E_FAIL;
	}
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{
		return m_pToolWindow ? m_pToolWindow->SendMessage(a_uMsg, a_wParam, a_lParam) : E_FAIL;
	}
	STDMETHOD(Show)(BOOL a_bShow)
	{
		bool bShow = a_bShow;
		if (bShow == m_bVisible)
			return S_FALSE;
		m_bVisible = bShow;
		return m_pToolWindow ? m_pToolWindow->Show(m_bVisible) : E_FAIL;
	}
	STDMETHOD(Move)(RECT const* a_prcPosition)
	{
		if (a_prcPosition == NULL) return E_POINTER;
		if (a_prcPosition->left == m_rcWnd.left && a_prcPosition->right == m_rcWnd.right &&
			a_prcPosition->top == m_rcWnd.top && a_prcPosition->bottom == m_rcWnd.bottom)
			return S_FALSE;
		m_rcWnd = *a_prcPosition;
		return m_pToolWindow ? m_pToolWindow->Move(&m_rcWnd) : E_FAIL;
	}
	STDMETHOD(Destroy)()
	{
		for (CCachedWindows::const_iterator i = m_cCachedWindows.begin(); i != m_cCachedWindows.end(); ++i)
			i->second->Destroy();
		m_pToolWindow = NULL;
		return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		return m_pToolWindow ? m_pToolWindow->PreTranslateMessage(a_pMsg, a_bBeforeAccel) : S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (m_pToolWindow == NULL)
				return E_FAIL;

			SIZE szMax = *a_pSize;
			m_pToolWindow->OptimumSize(&szMax);

			//if (GetWindowLong(GWL_EXSTYLE)&WS_EX_CLIENTEDGE)
			//{
			//	int cxEdge = GetSystemMetrics(SM_CXEDGE);
			//	int cyEdge = GetSystemMetrics(SM_CYEDGE);
			//	a_pSize->cx = szMax.cx + cxEdge + cxEdge;
			//	a_pSize->cy = szMax.cy + cyEdge + cyEdge;
			//}
			//else
			{
				a_pSize->cx = szMax.cx;
				a_pSize->cy = szMax.cy;
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter UNREF(a_eFilter), IEnumUnknownsInit* a_pInterfaces)
	{
		return S_FALSE;
	}
	STDMETHOD(OnIdle)()
	{
		return S_FALSE;
	}
	STDMETHOD(OnDeactivate)(BOOL UNREF(a_bCancelChanges))
	{
		return S_OK;
	}
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{
		::SendMessage(m_hParent, WM_RW_DEACTIVATE, a_bCancelChanges, 0);
		return S_OK;
	}

private:
	typedef std::map<CComBSTR, CComPtr<IRasterImageEditToolWindow> > CCachedWindows;

	template<UINT t_uIDD>
	class CEditToolWindowNoOptions :
		public CComObjectRootEx<CComMultiThreadModel>,
		public Win32LangEx::CLangDialogImpl<CEditToolWindowNoOptions<t_uIDD> >,
		public CDialogResize<CEditToolWindowNoOptions<t_uIDD> >,
		public CChildWindowImpl<CEditToolWindowNoOptions<t_uIDD>, IRasterImageEditToolWindow>
	{
	public:
		enum { IDD = t_uIDD };

		BEGIN_COM_MAP(CEditToolWindowNoOptions<t_uIDD>)
			COM_INTERFACE_ENTRY(IChildWindow)
			COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
		END_COM_MAP()

		BEGIN_MSG_MAP(CEditToolWindowNoOptions<t_uIDD>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColor)
			MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
			CHAIN_MSG_MAP(CDialogResize<CEditToolWindowNoOptions<t_uIDD> >)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CEditToolNoOptionsDlg<t_uIDD>)
			DLGRESIZE_CONTROL(IDC_ET_NOOPTIONS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			DlgResize_Init(false, false, 0);

			return 1;  // Let the system set the focus
		}
		LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			return 0;
		}
		LRESULT OnCtlColor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
		STDMETHOD(OptimumSize)(SIZE* a_pSize)
		{
			try
			{
				a_pSize->cx = a_pSize->cy = 0;
				return S_OK;
				//return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
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
	};

private:
	void SwitchTool(CComBSTR& a_bstrID)
	{
		CCachedWindows::iterator i = m_cCachedWindows.find(a_bstrID);
		if (i == m_cCachedWindows.end())
		{
			CComPtr<IRasterImageEditToolWindow> pWnd;
			if (a_bstrID == NULL)
			{
				CComObject<CEditToolWindowNoOptions<IDD_FILLSTYLE_UNAPPLICABLE> >* p = NULL;
				CComObject<CEditToolWindowNoOptions<IDD_FILLSTYLE_UNAPPLICABLE> >::CreateInstance(&p);
				pWnd = p;
				p->m_tLocaleID = m_tLocaleID;
				p->Create(m_hParent);
			}
			else
			{
				CComBSTR bstr = m_bstrSyncGroup;
				bstr += a_bstrID;
				m_pManager->WindowCreate(a_bstrID, m_hParent, m_tLocaleID, m_pSharedState, bstr, &pWnd);
				if (pWnd == NULL)
				{
					CComObject<CEditToolWindowNoOptions<IDD_FILLSTYLE_NOOPTIONS> >* p = NULL;
					CComObject<CEditToolWindowNoOptions<IDD_FILLSTYLE_NOOPTIONS> >::CreateInstance(&p);
					pWnd = p;
					p->m_tLocaleID = m_tLocaleID;
					p->Create(m_hParent);
				}
			}
			m_cCachedWindows[a_bstrID] = pWnd;
			i = m_cCachedWindows.find(a_bstrID);
		}
		m_bstrStyleID = i->first;
		if (m_pToolWindow != i->second)
		{
			if (m_pToolWindow)
				m_pToolWindow->Show(FALSE);
			m_pToolWindow = i->second;
			m_pToolWindow->Move(&m_rcWnd);
			m_pToolWindow->Show(m_bVisible);
		}
	}

private:
	LCID m_tLocaleID;
	bool m_bVisible;
	RECT m_rcWnd;
	HWND m_hParent;
	CComPtr<IRasterImageFillStyleManager> m_pManager;
	CComPtr<IRasterImageEditToolsManager> m_pToolsManager;
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncGroup;
	CComBSTR m_bstrStyleID;
	CComPtr<IRasterImageEditToolWindow> m_pToolWindow;
	CCachedWindows m_cCachedWindows;
	bool m_bFirstChange;
};


