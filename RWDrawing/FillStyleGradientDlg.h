// FillStyleGradientDlg.h : Declaration of the CFillStyleGradientDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <WTL_ColorPicker.h>


struct CFillStyleDataGradient
{
	MIDL_INTERFACE("DA7ADEF6-0CA6-43F7-9FA4-7F075C841F56")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CFillStyleDataGradient const*, InternalData)() = 0;
	};

	CFillStyleDataGradient() : bAutoCenter(true)
	{
		cGradient[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
		cGradient[0xffff] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr)
		{
			OLECHAR const* psz = a_bstr;
			cGradient.clear();
			bAutoCenter = false;
			if (_wcsnicmp(psz, L"AUTO;", 5) == 0)
			{
				bAutoCenter = true;
				psz += 5;
			}
			else if (_wcsnicmp(psz, L"DEFAULT;", 8) == 0)
			{
				psz += 8;
			}
			if (_wcsnicmp(psz, L"AUTO;", 5) == 0)
			{
				bAutoCenter = true;
				psz += 5;
			}
			else if (_wcsnicmp(psz, L"DEFAULT;", 8) == 0)
			{
				psz += 8;
			}
			while (true)
			{
				int wPos;
				CButtonColorPicker::SColor sColor;
				if (5 == swscanf(psz, L"%i,%g,%g,%g,%g;", &wPos, &sColor.fR, &sColor.fG, &sColor.fB, &sColor.fA))
					cGradient[wPos] = sColor;
				else
					break;
				while (*psz && *psz != L';') ++psz;
				if (*psz)
					++psz;
				else
					break;
			}
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		CComBSTR bstrGrad;
		for (CGradientColorPicker::CGradient::const_iterator i = cGradient.begin(); i != cGradient.end(); ++i)
		{
			wchar_t psz[128];
			swprintf(psz, L"%i,%g,%g,%g,%g;", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstrGrad.Append(psz);
		}
		if (!bAutoCenter)
		{
			*a_pbstr = bstrGrad.Detach();
			return S_OK;
		}
		CComBSTR bstr(L"AUTO;");
		bstr += bstrGrad;
		*a_pbstr = bstr.Detach();
		return S_OK;
	}
	bool operator!=(CFillStyleDataGradient const& a_cData) const
	{
		return bAutoCenter != a_cData.bAutoCenter || a_cData.cGradient != cGradient;
	}

	bool bAutoCenter;
	CGradientColorPicker::CGradient cGradient;
};

// CFillStyleGradientTemplDlg

template<typename T, typename TData>
class CFillStyleGradientTemplDlg : 
	public CEditToolDlgBase<T, TData, Win32LangEx::CLangIndirectDialogImpl<T> >,
	public CObserverImpl<CFillStyleGradientTemplDlg<T, TData>, IColorPickerObserver, ULONG>,
	public CDialogResize<T>,
	public CContextHelpDlg<T>
{
public:
	typedef CFillStyleGradientTemplDlg<T, TData> TThis;

	CFillStyleGradientTemplDlg() : m_wndGradient(true), m_wStopPos(0), m_nHeightDiff(0)
	{
	}

	enum { IDC_GRAD_SHAPECENTER = 100, IDC_GRAD_GRADIENT, IDC_GRAD_COLOR, IDC_GRAD_POSITION, IDC_GRAD_POSITION_UNIT, IDC_GRAD_LAST };

	BEGIN_MSG_MAP(TThis)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CContextHelpDlg<T>)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		NOTIFY_HANDLER(IDC_GRAD_GRADIENT, CGradientColorPicker::GCPN_ACTIVESTOPCHANGED, OnGradientStopChanged)
		COMMAND_HANDLER(IDC_GRAD_SHAPECENTER, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_GRAD_POSITION, EN_CHANGE, OnChangePosition)
		CHAIN_MSG_MAP(CDialogResize<T>)
	END_MSG_MAP()

	BEGIN_COM_MAP(TThis)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<TData::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				static_cast<T*>(this)->DataToGUI();
			}
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			if (m_pWnd)
			{
				auto pDlg = m_OrigTemplate.GetTemplateExPtr();
				RECT rc = {10, 10, pDlg->cx, pDlg->cy};
				MapDialogRect(&rc);
				SIZE tClrSize = {0, 0};
				if (a_pSize->cx)
					tClrSize.cx = a_pSize->cx-rc.left;
				if (a_pSize->cy)
					tClrSize.cy = a_pSize->cy-rc.top;
				HRESULT hRes = m_pWnd->OptimumSize(&tClrSize);
				a_pSize->cx = max(rc.right, rc.left+tClrSize.cx);
				a_pSize->cy = rc.bottom+tClrSize.cy-m_nHeightDiff;
				return hRes;
			}
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

		CComQIPtr<TData::ISharedStateToolData> pState(a_pState);
		if (pState != NULL)
		{
			m_cData = *(pState->InternalData());
			DataToGUI();
		}
		return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_pWnd)
		{
			HRESULT hRes = m_pWnd->PreTranslateMessage(a_pMsg, a_bBeforeAccel);
			if (hRes == S_OK)
				return S_OK;
		}
		return CEditToolDlgBase<T, TData, Win32LangEx::CLangIndirectDialogImpl<T> >::PreTranslateMessage(a_pMsg, a_bBeforeAccel);
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndStopPosLabel = GetDlgItem(IDC_GRAD_POSITION_UNIT);
		m_wndStopPos = GetDlgItem(IDC_GRAD_POSITION);
		m_wndStopPos.SetWindowText(_T("0"));

		HDC hdc = GetDC();
		float fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
		ReleaseDC(hdc);

		RECT rcBase;
		m_wndStopPos.GetWindowRect(&rcBase);
		ScreenToClient(&rcBase);

		RECT rcGrad = {5, 4, 100, 36};
		MapDialogRect(&rcGrad);
		m_wndGradient.SetMargin(rcGrad.left);
		rcGrad.left = 0;
		rcGrad.top += rcBase.bottom;
		rcGrad.bottom = rcGrad.top+fScale*28;

		m_wndGradient.Create(m_hWnd, rcGrad, _T("Gradient"), WS_CHILD|WS_TABSTOP|WS_VISIBLE, 0, IDC_GRAD_GRADIENT);
		//m_wndGradient.SetWindowPos(wnd, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOREDRAW);
		AddWindowForPreTranslate(m_wndStopPos);
		//AddWindowForPreTranslate(m_wndGradient);

		if (m_pSharedState != NULL)
		{
			CComPtr<TData::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(TData::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		auto pDlg = m_OrigTemplate.GetTemplateExPtr();
		RECT rcColor = {5, 5, 95, pDlg->cy-5};
		MapDialogRect(&rcColor);
		rcColor.top += rcGrad.bottom;
		m_nHeightDiff = rcColor.bottom - rcColor.top;
		RWCoCreateInstance(m_pWnd, __uuidof(ColorPicker));
		m_pWnd->Create(m_hWnd, &rcColor, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Selected stop color[0405]Barva vybraného zastavení"), FALSE, CComBSTR(L"GRADIENT"), NULL);
		RWHWND h = NULL;
		m_pWnd->Handle(&h);
		::SetWindowLong(h, GWLP_ID, IDC_GRAD_COLOR);
		m_pWnd->ObserverIns(CObserverImpl<CFillStyleGradientTemplDlg<T, TData>, IColorPickerObserver, ULONG>::ObserverGet(), 0);

		static_cast<T*>(this)->DataToGUI();
		BOOL b;
		CContextHelpDlg<T>::OnInitDialog(0, 0, 0, b);

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_pWnd->ObserverDel(CObserverImpl<CFillStyleGradientTemplDlg<T, TData>, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		static_cast<T*>(this)->GUIToData();
		static_cast<T*>(this)->DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangePosition(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;
		TCHAR szTmp[32] = _T("");
		m_wndStopPos.GetWindowText(szTmp, itemsof(szTmp)-1);
		szTmp[itemsof(szTmp)-1] = _T('\0');
		TCHAR* pEnd;
		double fVal = _tcstod(szTmp, &pEnd);
		if (fVal < 0.0) fVal = 0.0; else if (fVal > 100.0) fVal = 100.0;
		if (*pEnd == _T('\0') || *pEnd == _T(' ') || *pEnd == '\t')
		{
			int nCheck = fVal*10.0+0.5;
			if (nCheck != ((m_wStopPos*1000+0x7fff)/0xffff) && nCheck != m_wStopPos)
			{
				m_wndGradient.MoveStop(m_wStopPos, fVal*0xffff/100.0 + 0.5);
			}
		}
		return 0;
	}
	LRESULT OnGradientStopChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		CGradientColorPicker::NMGRADIENT* pGrad = reinterpret_cast<CGradientColorPicker::NMGRADIENT*>(a_pHdr);

		TColor tClr = {pGrad->clr.fR, pGrad->clr.fG, pGrad->clr.fB, pGrad->clr.fA};
		m_pWnd->ColorSet(&tClr);
		if (pGrad->pos != m_wStopPos)
		{
			TCHAR szTmp[32] = _T("");
			m_wndStopPos.GetWindowText(szTmp, itemsof(szTmp)-1);
			szTmp[itemsof(szTmp)-1] = _T('\0');
			TCHAR* pEnd;
			double fVal = _tcstod(szTmp, &pEnd);
			int nCheck = fVal*10.0+0.5;
			if (nCheck != ((pGrad->pos*1000+0x7fff)/0xffff) && nCheck != pGrad->pos)
			{
				TCHAR sz[32];
				_stprintf(sz, _T("%g"), ((pGrad->pos*1000+0x7fff)/0xffff)*0.1f);
				bool b = m_bEnableUpdates;
				m_bEnableUpdates = false;
				m_wndStopPos.SetWindowText(sz);
				m_bEnableUpdates = b;
			}
			m_wStopPos = pGrad->pos;
			m_wndStopPos.ShowWindow(m_wStopPos && m_wStopPos != 0xffff ? SW_SHOW : SW_HIDE);
			m_wndStopPosLabel.ShowWindow(m_wStopPos && m_wStopPos != 0xffff ? SW_SHOW : SW_HIDE);
		}

		if (!m_bEnableUpdates)
			return 0;

		static_cast<T*>(this)->GUIToData();
		static_cast<T*>(this)->DataToGUI();
		DataToState();
		return 0;
	}
	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		if (a_nFlags&ECPCColor)
		{
			TColor tClr = {0.0f, 0.0f, 0.0f, 0.0f};
			m_pWnd->ColorGet(&tClr);
			m_bEnableUpdates = false;
			m_wndGradient.SetStop(m_wStopPos, CButtonColorPicker::SColor(tClr.fR, tClr.fG, tClr.fB, tClr.fA));
			m_bEnableUpdates = true;
			static_cast<T*>(this)->GUIToData();
			DataToState();
		}
	}

	void GUIToData()
	{
		m_cData.bAutoCenter = IsDlgButtonChecked(IDC_GRAD_SHAPECENTER) == BST_CHECKED;
		m_cData.cGradient = m_wndGradient.GetGradient();
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		CheckDlgButton(IDC_GRAD_SHAPECENTER, m_cData.bAutoCenter ? BST_CHECKED : BST_UNCHECKED);
		m_wndGradient.SetGradient(m_cData.cGradient);
		WORD wPos = m_wndGradient.GetStop();
		if (wPos != m_wStopPos)
		{
			TCHAR szTmp[32] = _T("");
			m_wndStopPos.GetWindowText(szTmp, itemsof(szTmp)-1);
			szTmp[itemsof(szTmp)-1] = _T('\0');
			TCHAR* pEnd;
			double fVal = _tcstod(szTmp, &pEnd);
			int nCheck = fVal*10.0+0.5;
			if (nCheck != ((wPos*1000+0x7fff)/0xffff) && nCheck != wPos)
			{
				TCHAR sz[32];
				_stprintf(sz, _T("%g"), ((wPos*1000+0x7fff)/0xffff)*0.1f);
				m_wndStopPos.SetWindowText(sz);
			}
			m_wStopPos = wPos;
			m_wndStopPos.ShowWindow(m_wStopPos && m_wStopPos != 0xffff ? SW_SHOW : SW_HIDE);
			m_wndStopPosLabel.ShowWindow(m_wStopPos && m_wStopPos != 0xffff ? SW_SHOW : SW_HIDE);
		}
		CButtonColorPicker::SColor const& sClr = m_wndGradient.GetStopColor(m_wStopPos);
		TColor tClr = {sClr.fR, sClr.fG, sClr.fB, sClr.fA};
		m_pWnd->ColorSet(&tClr);
		m_bEnableUpdates = true;
	}

private:
	CGradientColorPicker m_wndGradient;
	CComPtr<IColorPicker> m_pWnd;
	CEdit m_wndStopPos;
	CWindow m_wndStopPosLabel;
	WORD m_wStopPos;
	int m_nHeightDiff;
};


// CFillStyleGradientDlg

class CFillStyleGradientDlg : public CFillStyleGradientTemplDlg<CFillStyleGradientDlg, CFillStyleDataGradient>
{
	typedef CFillStyleGradientTemplDlg<CFillStyleGradientDlg, CFillStyleDataGradient> TBase;
public:
	BEGIN_DIALOG_EX(0, 0, 100, 60, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CHECKBOX(_T("[0409]Follow drawn shape[0405]Následovat kreslený tvar"), IDC_GRAD_SHAPECENTER, 5, 6, 50, 10, BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_EDITTEXT(IDC_GRAD_POSITION, 56, 5, 28, 12, WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_LTEXT(_T("%"), IDC_GRAD_POSITION_UNIT, 88, 7, 8, 8, 0, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CFillStyleGradientDlg)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFillStyleGradientDlg)
		DLGRESIZE_CONTROL(IDC_GRAD_GRADIENT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_SHAPECENTER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_COLOR, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_GRAD_POSITION, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_POSITION_UNIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CFillStyleGradientDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_SHAPECENTER, IDS_HELP_FSCENTER)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_GRADIENT, IDS_HELP_GRADIENTCTL)
		//CTXHELP_CONTROL_RESOURCE(IDC_GRAD_COLOR, IDS_HELP_GRADIENTSTOPCLR)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_POSITION, IDS_HELP_GRADIENTSTOPPOS)
	END_CTXHELP_MAP()

};

struct CFillStyleDataDiamond
{
	MIDL_INTERFACE("A34B5331-5DA0-435B-A574-6B2C113C387D")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CFillStyleDataDiamond const*, InternalData)() = 0;
	};

	CFillStyleDataDiamond() : bAutoCenter(true), nSides(4)
	{
		cGradient[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
		cGradient[0xffff] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr)
		{
			OLECHAR const* psz = a_bstr;
			cGradient.clear();
			bAutoCenter = false;
			if (_wcsnicmp(psz, L"AUTO;", 5) == 0)
			{
				bAutoCenter = true;
				psz += 5;
			}
			else if (_wcsnicmp(psz, L"DEFAULT;", 8) == 0)
			{
				psz += 8;
			}
			if (_wcsnicmp(psz, L"AUTO;", 5) == 0)
			{
				bAutoCenter = true;
				psz += 5;
			}
			else if (_wcsnicmp(psz, L"DEFAULT;", 8) == 0)
			{
				psz += 8;
			}
			while (true)
			{
				int wPos;
				CButtonColorPicker::SColor sColor;
				if (5 == swscanf(psz, L"%i,%g,%g,%g,%g;", &wPos, &sColor.fR, &sColor.fG, &sColor.fB, &sColor.fA))
					cGradient[wPos] = sColor;
				else
					break;
				while (*psz && *psz != L';') ++psz;
				if (*psz)
					++psz;
				else
					break;
			}
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		CComBSTR bstrGrad;
		for (CGradientColorPicker::CGradient::const_iterator i = cGradient.begin(); i != cGradient.end(); ++i)
		{
			wchar_t psz[128];
			swprintf(psz, L"%i,%g,%g,%g,%g;", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstrGrad.Append(psz);
		}
		if (!bAutoCenter)
		{
			*a_pbstr = bstrGrad.Detach();
			return S_OK;
		}
		CComBSTR bstr(L"AUTO;");
		bstr += bstrGrad;
		*a_pbstr = bstr.Detach();
		return S_OK;
	}
	bool operator!=(CFillStyleDataDiamond const& a_cData) const
	{
		return bAutoCenter != a_cData.bAutoCenter || a_cData.nSides != nSides || a_cData.cGradient != cGradient;
	}

	bool bAutoCenter;
	CGradientColorPicker::CGradient cGradient;
	int nSides;
};

