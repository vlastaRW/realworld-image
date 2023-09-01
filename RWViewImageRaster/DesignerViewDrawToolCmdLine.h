// DesignerViewDrawToolCmdLine.h : Declaration of the CDesignerViewDrawToolCmdLine

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <ObserverImpl.h>
#include <InPlaceCalc.h>
#include <boost/spirit.hpp>
using namespace boost::spirit;
//#include <Win32LangEx.h>

static OLECHAR const CFGID_CMDLINE_SYNCID[] = L"ToolCmdSyncID";
static OLECHAR const CFGID_CMDLINE_SYNTAX[] = L"Syntax";
static LONG const CFGVAL_CLS_SIMPLE = 0L;
static LONG const CFGVAL_CLS_SCRIPT = 1L;

// CDesignerViewDrawToolCmdLine

class ATL_NO_VTABLE CDesignerViewDrawToolCmdLine : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewDrawToolCmdLine>,
	public CObserverImpl<CDesignerViewDrawToolCmdLine, ISharedStateObserver, TSharedStateChange>,
	public CDesignerViewWndImpl<CDesignerViewDrawToolCmdLine, IDesignerView>
{
public:
	CDesignerViewDrawToolCmdLine() : bUpdating(false), m_eSyntax(CFGVAL_CLS_SIMPLE)
	{
	}
	~CDesignerViewDrawToolCmdLine()
	{
	}
	void Init(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID)
	{
		m_pConfig = a_pConfig;
		CConfigValue cVal;
		m_pConfig->ItemValueGet(CComBSTR(CFGID_CMDLINE_SYNCID), &cVal);
		m_bstrSyncGroup = cVal;
		if (a_pStateMgr && m_bstrSyncGroup.Length())
		{
			m_pStateMgr = a_pStateMgr;
			m_pStateMgr->ObserverIns(ObserverGet(), 0);
		}
		m_pConfig->ItemValueGet(CComBSTR(CFGID_CMDLINE_SYNTAX), &cVal);
		m_eSyntax = cVal;
		if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("DTCmdLine"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, /*(a_nStyle&EDVWSBorderMask) == EDVWSNoBorder ?*/ 0 /*: WS_EX_CLIENTEDGE*/) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}
	}

	DECLARE_WND_CLASS_EX(_T("DrawToolCmdLine"), CS_OWNDC | CS_VREDRAW | CS_HREDRAW, COLOR_3DFACE);

BEGIN_COM_MAP(CDesignerViewDrawToolCmdLine)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewDrawToolCmdLine)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	COMMAND_HANDLER(1, EN_CHANGE, OnChange)
	COMMAND_HANDLER(1, EN_KILLFOCUS, OnKillFocus)
	//MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()


DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pStateMgr)
			m_pStateMgr->ObserverDel(CObserverImpl<CDesignerViewDrawToolCmdLine, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	// IChildWindow metods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd)
		{
			if (m_wndEdit.m_hWnd == a_pMsg->hwnd)
			{
				if (a_pMsg->message == WM_KEYDOWN && a_pMsg->wParam == 'A' && GetKeyState(VK_CONTROL)&0x8000)
				{
					m_wndEdit.SetSelAll();
					return S_OK;
				}
				TranslateMessage(a_pMsg);
				DispatchMessage(a_pMsg);
				return S_OK;
			}
		}
		return S_FALSE;
	}

	// IDesignerView metods
public:

public:
	void OwnerNotify(TCookie a_tCookie, TSharedStateChange a_tParameter)
	{
		try
		{
			if (m_hWnd && m_wndEdit.m_hWnd && !bUpdating && m_bstrSyncGroup == a_tParameter.bstrName && GetFocus() != m_wndEdit.m_hWnd)
			{
				CComBSTR bstrNew;
				if (a_tParameter.pState)
					a_tParameter.pState->ToText(&bstrNew);
				bool bValid = false;
				std::wstring strNewTool;
				CParams cNewParams;
				if (bstrNew.m_str)
					bValid = ParseCmdLine(bstrNew, strNewTool, cNewParams);

				if (!bValid)
					return;

				CComBSTR bstrOld;
				m_wndEdit.GetWindowText(&bstrOld);
				if (bstrOld.m_str)
				{
					std::wstring strOldTool;
					CParams cOldParams;
					if (ParseCmdLine(bstrOld, strOldTool, cOldParams, m_eSyntax))
					{
						if (strNewTool == strOldTool && cNewParams.size() == cOldParams.size() &&
							std::equal(cNewParams.begin(), cNewParams.end(), cOldParams.begin()))
							return; // no real change (whitespace, etc.)
					}
				}

				CComBSTR bstr;
				if (m_eSyntax == CFGVAL_CLS_SCRIPT)
				{
					bstr = L"DrawTool.";
					bstr += strNewTool.c_str();
					if (cNewParams.empty())
					{
						bstr += L"();";
					}
					else
					{
						bstr += L"(Document";
						for (CParams::const_iterator i = cNewParams.begin(); i != cNewParams.end(); ++i)
						{
							bstr += L", ";
							if (i->eType == SParam::ETBool)
								bstr += i->b ? L"true" : L"false";
							else if (i->eType == SParam::ETString)
								bstr += i->str.c_str();
							else if (i->eType == SParam::ETNumber)
							{
								wchar_t sz[32] = L"";
								swprintf(sz, 32, L"%g", i->d);
								bstr += sz;
							}
						}
						bstr += L");";
					}
				}
				else
				{
					bstr = strNewTool.c_str();
					bstr += L" ";
					if (!cNewParams.empty())
					{
						for (CParams::const_iterator i = cNewParams.begin(); i != cNewParams.end(); ++i)
						{
							if (i != cNewParams.begin())
								bstr += L", ";
							if (i->eType == SParam::ETBool)
								bstr += i->b ? L"true" : L"false";
							else if (i->eType == SParam::ETString)
								bstr += i->str.c_str();
							else if (i->eType == SParam::ETNumber)
							{
								wchar_t sz[32] = L"";
								swprintf(sz, 32, L"%g", i->d);
								bstr += sz;
							}
						}
					}
				}
				bUpdating = true;
				m_wndEdit.SetWindowText(bstr);
				bUpdating = false;
			}
		}
		catch (...)
		{
		}
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc;
		GetClientRect(&rc);
		HDC hDC = GetDC();
		float fScale = GetDeviceCaps(hDC, LOGPIXELSX)/96.0f;
		ReleaseDC(hDC);
		m_wndEdit.Create(m_hWnd, rc, NULL, WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL, 0, 1);
		m_cFont.CreateFont(-(12*fScale+0.5f), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, _T("Courier New"));
		m_wndEdit.SetFont(m_cFont);
		if (m_pStateMgr)
		{
			CComPtr<ISharedState> pSS;
			m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pSS));
			TSharedStateChange tState;
			tState.bstrName = m_bstrSyncGroup;
			tState.pState = pSS;
			OwnerNotify(0, tState);
		}
		return 0;
	}
	LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndEdit.SetFocus();
		return 0;
	}
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);

		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			m_wndEdit.SetFocus();
		}

		return lRet;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndEdit.MoveWindow(0, 0, LOWORD(a_lParam), HIWORD(a_lParam));
		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!bUpdating)
		{
			CComBSTR bstr;
			m_wndEdit.GetWindowText(&bstr);
			if (bstr.m_str == NULL)
				return 0;

			std::wstring strTool;
			CParams cParams;
			if (!ParseCmdLine(bstr, strTool, cParams, m_eSyntax))
				return 0;

			CComPtr<ISharedState> pOld;
			m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pOld));
			CComBSTR bstrOld;
			if (pOld) pOld->ToText(&bstrOld);
			if (bstrOld.Length())
			{
				std::wstring strOldTool;
				CParams cOldParams;
				if (ParseCmdLine(bstrOld, strOldTool, cOldParams))
				{
					if (strTool == strOldTool && cParams.size() == cOldParams.size() &&
						std::equal(cParams.begin(), cParams.end(), cOldParams.begin()))
						return 0; // no real change (whitespace, etc.)
				}
			}

			// send the changed command line
			bstr = strTool.c_str();
			bstr += L"\n";
			if (!cParams.empty())
			{
				for (CParams::const_iterator i = cParams.begin(); i != cParams.end(); ++i)
				{
					if (i != cParams.begin())
						bstr += L", ";
					if (i->eType == SParam::ETBool)
						bstr += i->b ? L"true" : L"false";
					else if (i->eType == SParam::ETString)
						bstr += i->str.c_str();
					else if (i->eType == SParam::ETNumber)
					{
						wchar_t sz[32] = L"";
						swprintf(sz, 32, L"%g", i->d);
						bstr += sz;
					}
				}
			}
			CComPtr<ISharedState> pSel;
			RWCoCreateInstance(pSel, __uuidof(SharedStateString));
			pSel->FromText(bstr);
			bUpdating = true;
			m_pStateMgr->StateSet(m_bstrSyncGroup, pSel);
			bUpdating = false;
		}
		return 0;
	}
	LRESULT OnKillFocus(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_pStateMgr)
		{
			CComPtr<ISharedState> pSS;
			m_pStateMgr->StateGet(m_bstrSyncGroup, __uuidof(ISharedState), reinterpret_cast<void**>(&pSS));
			TSharedStateChange tState;
			tState.bstrName = m_bstrSyncGroup;
			tState.pState = pSS;
			OwnerNotify(0, tState);
		}
		return 0;
	}
	//LRESULT OnSelectionChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);

private:
	struct SParam
	{
		SParam() : eType(ETInvalid), b(false), d(0.0) {}
		SParam(wchar_t const* a_pB, wchar_t const* a_pE) { eType = ETString; b = false; d = 0.0; str.assign(a_pB, a_pE); }
		void operator =(bool a_b) { eType = ETBool; b = a_b; d = 0.0; str.clear(); }
		void operator =(double a_d) { eType = ETNumber; b = false; d = a_d; str.clear(); }
		void assign(wchar_t const* a_pB, wchar_t const* a_pE) { eType = ETString; b = false; d = 0.0; str.assign(a_pB, a_pE); }

		bool operator ==(SParam const& a_rhs) const
		{
			if (eType != a_rhs.eType)
				return false;
			if (eType == ETNumber)
				return fabs(d - a_rhs.d) <= ( (fabs(d) < fabs(a_rhs.d) ? fabs(d) : fabs(a_rhs.d)) * 1e-6);
			if (eType == ETString)
				return str == a_rhs.str;
			if (eType == ETBool)
				return b == a_rhs.b;
			return true; // 2 invalid values
		}

		enum EType { ETInvalid = 0, ETBool, ETNumber, ETString };

		EType eType;
		bool b;
		double d;
		std::wstring str;
	};
	typedef std::vector<SParam> CParams;
	struct calc_actor
	{
		calc_actor(SParam& a_par) : par(a_par) {}

		void operator()(wchar_t const* first, wchar_t const* last) const
		{
			std::wstring str(first, last);
			par = CInPlaceCalc::EvalExpression(str.c_str());
		}

		SParam& par;
	};
	struct calculator : public grammar<calculator>
	{
		template <typename ScannerT>
		struct definition
		{
			definition(calculator const& /*self*/)
			{
				expression
					=   term
						>> *(   (L'+' >> term)
							|   (L'-' >> term)
							)
					;

				term
					=   factor
						>> *(   (L'*' >> factor)
							|   (L'/' >> factor)
							)
					;

				factor
					=   real_p
					|	L"pi"
					|   L'(' >> expression >> L')'
					|   (L'-' >> factor)
					|   (L'+' >> factor)
					;
			}

			rule<ScannerT> expression, term, factor;

			rule<ScannerT> const&
			start() const { return expression; }
		};
	};

	static bool ParseCmdLine(BSTR a_bstrCmdLine, std::wstring& a_strTool, CParams& a_cParams, LONG a_eSyntax = -1)
	{
		rule<scanner<wchar_t*> > separator_p = *space_p>>L','>>*space_p;
		rule<scanner<wchar_t*> > identifier_p = (alpha_p|L'_')>>*(alnum_p|L'_');
		calculator calc;
		SParam sParam;
		rule<scanner<wchar_t*> > parameter_p =
			confix_p(L'"', (*c_escape_ch_p), L'"')[assign_a(sParam)]
			|
			calc[calc_actor(sParam)]
			|
			as_lower_d[L"true"][assign_a(sParam, true)]
			|
			as_lower_d[L"false"][assign_a(sParam, false)];

		if (a_eSyntax == CFGVAL_CLS_SCRIPT)
		{
			return parse(a_bstrCmdLine, a_bstrCmdLine+::SysStringLen(a_bstrCmdLine),
				*space_p>>L"DrawTool">>*space_p>>L'.'>>*space_p>>
				identifier_p[assign_a(a_strTool)]>> // tool id
				*space_p>>L'('>>*space_p>>
				!(
					L"Document">>separator_p>>
					parameter_p[push_back_a(a_cParams, sParam)]>>
					*(separator_p>>parameter_p[push_back_a(a_cParams, sParam)])
				)>>
				L')'>>*space_p>>L';'>>*space_p
			).full;
		}
		else if (a_eSyntax == CFGVAL_CLS_SIMPLE)
		{
			return parse(a_bstrCmdLine, a_bstrCmdLine+::SysStringLen(a_bstrCmdLine),
				*space_p>>
				identifier_p[assign_a(a_strTool)]>> // tool id
				!(
					+space_p>>
					parameter_p[push_back_a(a_cParams, sParam)]>>
					*(separator_p>>parameter_p[push_back_a(a_cParams, sParam)])
				)
			).full;
		}
		else
		{
			return parse(a_bstrCmdLine, a_bstrCmdLine+::SysStringLen(a_bstrCmdLine),
				*space_p>>
				identifier_p[assign_a(a_strTool)]>> // tool id
				L"\n">>
				!(
					parameter_p[push_back_a(a_cParams, sParam)]>>
					*(separator_p>>parameter_p[push_back_a(a_cParams, sParam)])
				)
			).full;
		}
	}

private:
	CComPtr<ISharedStateManager> m_pStateMgr;
	CComPtr<IConfig> m_pConfig;
	//CComPtr<IDocument> m_pOriginal;

	// synchronization
	CComBSTR m_bstrSyncGroup;
	bool bUpdating;

	CEdit m_wndEdit;
	CFont m_cFont;

	LONG m_eSyntax;
};

