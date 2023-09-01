// RasterImageFill.cpp : Implementation of CRasterImageFill

#include "stdafx.h"
#include <RWDrawing.h>
#include "RasterImageFill.h"
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>
#include <RWViewImageRaster.h>
#include <StringParsing.h>


const OLECHAR CFGID_FL_FSID[] = L"StyleID";
//const OLECHAR CFGID_FL_FSCONFIG[] = L"StyleConfig";
//const OLECHAR CFGID_FL_FSSTATE[] = L"StyleState";
const OLECHAR CFGID_FL_COLOR1[] = L"Color1";
const OLECHAR CFGID_FL_COLOR2[] = L"Color2";
const OLECHAR CFGID_FL_RESPECTALPHA[] = L"RespectAlpha";
const OLECHAR CFGID_FL_BLENDINGMODE[] = L"BlendingMode";

#include <ConfigCustomGUIImpl.h>
#include <SubjectImpl.h>
#include <WTL_ColorPicker.h>

bool GetDefaultFillStyle(BSTR a_bstrStyleID, CComBSTR& bstrStyleParams, float const* a_pColor1, float const* a_pColor2)
{
	if (wcscmp(a_bstrStyleID, L"SOLID") == 0)
	{
		wchar_t sz[64] = L"";
		swprintf(sz, 64, L"%g,%g,%g,%g", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3]);
		bstrStyleParams = sz;
		return true;
	}
	else if (wcscmp(a_bstrStyleID, L"PATTERN") == 0)
	{
		wchar_t sz[128] = L"";
		swprintf(sz, 64, L"STOCK|0||%g,%g,%g,%g,%g,%g,%g,%g", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3], powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
		bstrStyleParams = sz;
		return true;
	}
	else if (wcscmp(a_bstrStyleID, L"LINEAR") == 0 || wcscmp(a_bstrStyleID, L"RADIAL") == 0 || wcscmp(a_bstrStyleID, L"CONICAL") == 0)
	{
		wchar_t sz[160] = L"";
		swprintf(sz, 160, L"0,%g,%g,%g,%g,65535,%g,%g,%g,%g,RELSHAPE", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3], powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
		bstrStyleParams = sz;
		return true;
	}
	return false;
}

void UpdateFillStyle(BSTR a_bstrStyleID, CComBSTR& bstrStyleParams, float const* a_pColor1, float const* a_pColor2)
{
	if (wcscmp(a_bstrStyleID, L"SOLID") == 0)
	{
		if (wcsncmp(bstrStyleParams, L"CUSTOM", 6) == 0)
		{
			TColor t = {0.5f, 0.5f, 0.5f, 1.0f};
			swscanf(bstrStyleParams+7, L"%f,%f,%f,%f", &t.fR, &t.fG, &t.fB, &t.fA);
			t.fR = powf(t.fR, 2.2f);
			t.fG = powf(t.fG, 2.2f);
			t.fB = powf(t.fB, 2.2f);
			wchar_t sz[64] = L"";
			swprintf(sz, 64, L"%g,%g,%g,%g", t.fR, t.fG, t.fB, t.fA);
			bstrStyleParams = sz;
		}
		else if (wcsncmp(bstrStyleParams, L"PRIMARY", 7) == 0)
		{
			wchar_t sz[64] = L"";
			swprintf(sz, 64, L"%g,%g,%g,%g", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3]);
			bstrStyleParams = sz;
		}
		else if (wcsncmp(bstrStyleParams, L"SECONDARY", 9) == 0)
		{
			wchar_t sz[64] = L"";
			swprintf(sz, 64, L"%g,%g,%g,%g", powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
			bstrStyleParams = sz;
		}
	}
	else if (wcscmp(a_bstrStyleID, L"PATTERN") == 0)
	{
		wchar_t sz[128] = L"";
		swprintf(sz, 64, L"|%g,%g,%g,%g,%g,%g,%g,%g", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3], powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
		bstrStyleParams += sz;
	}
	else if (wcscmp(a_bstrStyleID, L"LINEAR") == 0 || wcscmp(a_bstrStyleID, L"RADIAL") == 0 || wcscmp(a_bstrStyleID, L"CONICAL") == 0)
	{
		std::vector<std::pair<int, TColor> > cG;
		wchar_t const* psz = bstrStyleParams;
		while (*psz)
		{
			std::pair<int, TColor> t;
			int n = swscanf(psz, L"%i,%f,%f,%f,%f", &t.first, &t.second.fR, &t.second.fG, &t.second.fB, &t.second.fA);
			if (n != 5)
				break;
			for (int i = 0; *psz && i < 5; ++psz)
				if (*psz == L',')
					++i;
			t.second.fR = powf(t.second.fR, 2.2f);
			t.second.fG = powf(t.second.fG, 2.2f);
			t.second.fB = powf(t.second.fB, 2.2f);
			cG.push_back(t);
		}
		CComBSTR bstr2;
		if (cG.size() < 2)
		{
			wchar_t sz[160] = L"";
			swprintf(sz, 160, L"0,%g,%g,%g,%g,65535,%g,%g,%g,%g,", powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3], powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
			bstr2 = sz;
		}
		else
		{
			wchar_t sz[80] = L"";
			for (std::vector<std::pair<int, TColor> >::const_iterator i = cG.begin(); i != cG.end(); ++i)
			{
				swprintf(sz, 80, L"%i,%g,%g,%g,%g,", i->first, i->second.fR, i->second.fG, i->second.fB, i->second.fA);
				bstr2 += sz;
			}
		}
		bstr2 += psz;
		bstrStyleParams.Attach(bstr2.Detach());
	}
	else if (wcscmp(a_bstrStyleID, L"WOOD") == 0 || wcscmp(a_bstrStyleID, L"CLOUDS") == 0)
	{
		wchar_t const* psz = bstrStyleParams;
		int nCommas = 0;
		while (*psz && nCommas < 2) { if (*psz == L',') ++nCommas; ++psz; }
		if (nCommas != 2)
			return;
		wchar_t cAC = L'C';
		int colorset = 0;
		swscanf(bstrStyleParams, L"%c,%i", &cAC, &colorset);
		wchar_t sz[160] = L"";
		if (colorset)
		{
			if (wcscmp(a_bstrStyleID, L"WOOD") == 0)
				swprintf(sz, 160, L"%c,%g,%g,%g,%g,%g,%g,%g,%g,", cAC, 0.789f, 0.56f, 0.302f, 1.0f, 0.32f, 0.201f, 0.08f, 1.0f);
			else
				swprintf(sz, 160, L"%c,%g,%g,%g,%g,%g,%g,%g,%g,", cAC, 1.0f, 1.0f, 1.0f, 1.0f, 0.0385f, 0.187f, 0.612f, 1.0f);
		}
		else
		{
			swprintf(sz, 160, L"%c,%g,%g,%g,%g,%g,%g,%g,%g,", cAC, powf(a_pColor1[0], 2.2f), powf(a_pColor1[1], 2.2f), powf(a_pColor1[2], 2.2f), a_pColor1[3], powf(a_pColor2[0], 2.2f), powf(a_pColor2[1], 2.2f), powf(a_pColor2[2], 2.2f), a_pColor2[3]);
		}
		CComBSTR bstr(sz);
		bstr += psz;
		bstrStyleParams.Attach(bstr.Detach());
	}
}


#include "FillStylePreview.h"

class ATL_NO_VTABLE CConfigGUIFill :
	public CCustomConfigResourcelessWndImpl<CConfigGUIFill>,
	public CDialogResize<CConfigGUIFill>,
	public CSubjectImpl<ISharedStateManager, ISharedStateObserver, TSharedStateChange>,
	public IResizableConfigWindow
{
public:
	CConfigGUIFill() : m_bUpdating(true)
	{
	}

	enum
	{
		IDC_CGF_RESPECTALPHA = 100,
		IDC_CGF_BLENDING_LABEL,
		IDC_CGF_BLENDING,
		IDC_CGF_STYLEID_LABEL,
		IDC_CGF_STYLEID,
		IDC_CGF_STYLECONFIG,
		IDC_CGF_PREVIEW,
	};

	BEGIN_DIALOG_EX(0, 0, (M_Mode() == ECPMWithCanvas ? 134 : 242), 164, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()
    //CONTROL         "",IDC_CGF_STYLECONFIG,"Static",SS_GRAYRECT,110,50,132,112
    //CONTROL         "",IDC_CGF_PREVIEW,"Static",SS_WHITERECT,7,7,100,150

	BEGIN_CONTROLS_MAP()
		CONTROL_CHECKBOX(_T("[0409]Respect image opacity[0405]Respektovat průhlednost"), IDC_CGF_RESPECTALPHA, (M_Mode() == ECPMWithCanvas ? 7 : 115), 7, 120, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Blending:[0405]Míchání:"), IDC_CGF_BLENDING_LABEL, (M_Mode() == ECPMWithCanvas ? 7 : 115), 23, 39, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGF_BLENDING, (M_Mode() == ECPMWithCanvas ? 47 : 155), 21, 80, 122, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Fill style:[0405]Výplň:"), IDC_CGF_STYLEID_LABEL, (M_Mode() == ECPMWithCanvas ? 7 : 115), 39, 39, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGF_STYLEID, (M_Mode() == ECPMWithCanvas ? 47 : 155), 37, 80, 132, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIFill)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIFill>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIFill>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_CGF_STYLEID, CBN_SELCHANGE, OnStyleIDSelChange)
		NOTIFY_HANDLER(IDC_CGF_PREVIEW, CGradientPreview::GPN_STATECHANGED, OnFillStyleChange)
	END_MSG_MAP()

	const _AtlDlgResizeMap* GetDlgResizeMap()
	{
		static const _AtlDlgResizeMap theMap1[] = {
		DLGRESIZE_CONTROL(IDC_CGF_PREVIEW, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_RESPECTALPHA, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_BLENDING_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_BLENDING, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_STYLEID_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_STYLEID, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_STYLECONFIG, DLSZ_MOVE_X|DLSZ_SIZE_Y)
			{ -1, 0 }, };
		static const _AtlDlgResizeMap theMap2[] = {
		//DLGRESIZE_CONTROL(IDC_CGF_RESPECTALPHA, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_CGF_BLENDING_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_BLENDING, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_CGF_STYLEID_LABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGF_STYLEID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGF_STYLECONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			{ -1, 0 }, };
		return M_Mode() == ECPMWithCanvas ? theMap2 : theMap1;
	}

	BEGIN_CONFIGITEM_MAP(CConfigGUIFill)
		CONFIGITEM_CHECKBOX(IDC_CGF_RESPECTALPHA, CFGID_FL_RESPECTALPHA)
		CONFIGITEM_COMBOBOX(IDC_CGF_BLENDING, CFGID_FL_BLENDINGMODE)
		CONFIGITEM_CONTEXTHELP(IDC_CGF_STYLEID, CFGID_FL_FSID)
	END_CONFIGITEM_MAP()

	BEGIN_COM_MAP(CConfigGUIFill)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || m_pCurWnd == NULL || a_pSize == NULL)
			return E_UNEXPECTED;
		RECT rc = {0, 0, 134, 52};
		MapDialogRect(&rc);
		SIZE sz = {0, 0};
		m_pCurWnd->OptimumSize(&sz);
		a_pSize->cx = rc.right;
		a_pSize->cy = rc.bottom+sz.cy;
		return S_OK;
	}

	class CRasterImageBrushOwnerHelper : public IRasterImageBrushOwner
	{
	public:
		CRasterImageBrushOwnerHelper(ISharedStateManager* a_pStates, BSTR a_bstrID) : m_pStates(a_pStates), m_bstrID(a_bstrID) {}

		// IRasterImageBrushOwner methods
	public:
		STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
		{
			if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IRasterImageBrushOwner)))
			{
				*ppvObject = static_cast<IRasterImageBrushOwner*>(this);
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)()
		{
			return 2;
		}
		STDMETHOD_(ULONG, Release)()
		{
			return 1;
		}
		STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
		{
			*a_pSizeX = 1;
			*a_pSizeY = 1;
			return S_OK;
		}
		STDMETHOD(ControlPointsChanged)()
		{
			return S_FALSE;
		}
		STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
		{
			return S_FALSE;
		}
		STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
		{
			return S_FALSE;
		}
		STDMETHOD(ControlLinesChanged)()
		{
			return S_FALSE;
		}
		STDMETHOD(SetBrushState)(ISharedState* a_pState)
		{
			return m_pStates->StateSet(m_bstrID, a_pState);
		}

	private:
		BSTR m_bstrID;
		ISharedStateManager* m_pStates;
	};

	void ExtraInitDialog()
	{
		if (M_Mode() != ECPMWithCanvas)
		{
			RECT rc = {7, 7, 7+100, 7+150};
			MapDialogRect(&rc);
			m_wndPreview.Create(m_hWnd, &rc, _T("Gradient preview"), WS_VISIBLE|WS_CHILD, WS_EX_CLIENTEDGE, IDC_CGF_PREVIEW);
		}
		else
		{
			RECT rc = {0, 0, 0, 0};
			m_wndPreview.Create(m_hWnd, &rc, _T("Gradient preview"), WS_CHILD, WS_EX_CLIENTEDGE, IDC_CGF_PREVIEW);
		}
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		m_wndStyleID = GetDlgItem(IDC_CGF_STYLEID);

		RWCoCreateInstance(m_pFSMgr, __uuidof(RasterImageFillStyleManager));

		CComPtr<IEnumStrings> pIDs;
		m_pFSMgr->StyleIDsEnum(&pIDs);
		ULONG nIDs = 0;
		if (pIDs) pIDs->Size(&nIDs);
		CConfigValue cColor1;
		M_Config()->ItemValueGet(CComBSTR(CFGID_FL_COLOR1), &cColor1);
		CConfigValue cColor2;
		M_Config()->ItemValueGet(CComBSTR(CFGID_FL_COLOR2), &cColor2);
		CConfigValue cStyleID;
		M_Config()->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cStyleID);
		CComPtr<IConfig> pSubCfg;
		M_Config()->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pSubCfg);
		for (ULONG i = 0; i < nIDs; ++i)
		{
			CComBSTR bstrID;
			pIDs->Get(i, &bstrID);
			CComPtr<ILocalizedString> pName;
			m_pFSMgr->StyleNameGet(bstrID, &pName);
			CComBSTR bstrName;
			if (pName) pName->GetLocalized(m_tLocaleID, &bstrName);
			m_wndStyleID.SetItemData(m_wndStyleID.AddString(bstrName == NULL ? bstrID : bstrName), i);
			m_cItems.push_back(bstrID);
			CConfigValue cCfg;
			if (pSubCfg) pSubCfg->ItemValueGet(bstrID, &cCfg);
			if (cCfg.TypeGet() == ECVTString && cCfg.operator BSTR() && cCfg.operator BSTR()[0])
			{
				CComPtr<IRasterImageBrush> pFill;
				m_pFSMgr->FillStyleCreate(bstrID, NULL, &pFill);
				CComQIPtr<IRasterImageEditToolScripting> pScrpt(pFill);
				if (pScrpt)
				{
					CRasterImageBrushOwnerHelper cOwner(this, bstrID);
					pFill->Init(&cOwner);
					if (cColor1[3] != -1.0f)
					{
						CComBSTR bstrParams;
						bstrParams.Attach(cCfg.Detach().bstrVal);
						UpdateFillStyle(bstrID, bstrParams, cColor1, cColor2);
						pScrpt->FromText(bstrParams);
					}
					else
					{
						pScrpt->FromText(cCfg);
					}
				}
			}
			else if (cColor1[3] != -1.0f && bstrID == cStyleID.operator BSTR())
			{
				CComPtr<IRasterImageBrush> pFill;
				m_pFSMgr->FillStyleCreate(bstrID, NULL, &pFill);
				CComQIPtr<IRasterImageEditToolScripting> pScrpt(pFill);
				if (pScrpt)
				{
					CRasterImageBrushOwnerHelper cOwner(this, bstrID);
					pFill->Init(&cOwner);
					CComBSTR bstrParams;
					if (GetDefaultFillStyle(cStyleID, bstrParams, cColor1, cColor2))
						pScrpt->FromText(bstrParams);
				}
			}
		}

		RECT rc1;
		GetClientRect(&rc1);
		RECT rc2 = {M_Mode() == ECPMWithCanvas ? 2 : 110, 50, (M_Mode() == ECPMWithCanvas ? 2 : 110)+130, 50+112};
		MapDialogRect(&rc2);
		m_rcGaps.left = M_Mode() == ECPMWithCanvas ? rc2.left : rc2.left-rc1.right;
		m_rcGaps.right = rc2.right-rc1.right;
		m_rcGaps.top = rc2.top-rc1.top;
		m_rcGaps.bottom = rc2.bottom-rc1.bottom;

		m_bUpdating = false;
		ExtraConfigNotify();

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnStyleIDSelChange(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		int iSel = m_wndStyleID.GetItemData(m_wndStyleID.GetCurSel());
		if (iSel < 0 || ULONG(iSel) >= m_cItems.size())
			return 0;
		CComBSTR cCFGID_FL_FSID(CFGID_FL_FSID);
		M_Config()->ItemValuesSet(1, &(cCFGID_FL_FSID.m_str), CConfigValue(m_cItems[iSel]));
		return 0;
	}

	LRESULT OnFillStyleChange(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CConfigValue cID;
			M_Config()->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cID);
			CComPtr<IConfig> pSubCfg;
			M_Config()->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pSubCfg);
			CComBSTR bstrVal;
			m_wndPreview.Serialize(bstrVal);
			if (pSubCfg)
			{
				BSTR aIDs[1];
				aIDs[0] = cID;
				TConfigValue aVals[1];
				aVals[0].eTypeID = ECVTString;
				aVals[0].bstrVal = bstrVal;
				pSubCfg->ItemValuesSet(1, aIDs, aVals);
			}
			CComBSTR bstr1(CFGID_FL_COLOR1);
			CComBSTR bstr2(CFGID_FL_COLOR2);
			BSTR aIDs[2] = {bstr1, bstr2};
			TConfigValue aVals[2];
			aVals[0].eTypeID = aVals[1].eTypeID = ECVTVector4;
			aVals[0].vecVal[0] = aVals[1].vecVal[0] = 0.0f;
			aVals[0].vecVal[1] = aVals[1].vecVal[1] = 0.0f;
			aVals[0].vecVal[2] = aVals[1].vecVal[2] = 0.0f;
			aVals[0].vecVal[3] = aVals[1].vecVal[3] = -1.0f;
			M_Config()->ItemValuesSet(2, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_bUpdating || m_hWnd == NULL)
			return;

		CConfigValue cID;
		M_Config()->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cID);

		if (m_bstrCurID != cID.operator BSTR())
		{
			if (m_pCurWnd)
			{
				m_pCurWnd->Destroy();
				m_pCurWnd = NULL;
			}
			m_pFSMgr->WindowCreate(cID, m_hWnd, m_tLocaleID, this, cID, &m_pCurWnd);
			RWHWND h;
			if (m_pCurWnd)
			{
				m_pCurWnd->Handle(&h);
				::SetWindowLong(h, GWLP_ID, IDC_CGF_STYLECONFIG);
				RECT rc;
				GetClientRect(&rc);
				rc.top = m_rcGaps.top;
				rc.left = M_Mode() == ECPMWithCanvas ? m_rcGaps.left : rc.right+m_rcGaps.left;
				rc.bottom += m_rcGaps.bottom;
				rc.right += m_rcGaps.right;
				m_pCurWnd->Move(&rc);
				CStates::const_iterator iS = m_cStates.find(cID.operator BSTR());
				if (iS != m_cStates.end())
					m_pCurWnd->SetState(iS->second);
				m_pCurWnd->Show(TRUE);
			}

			int iSel = -1;
			for (CItems::iterator i = m_cItems.begin(); i != m_cItems.end(); ++i)
			{
				if (*i == cID.operator BSTR())
				{
					iSel = i-m_cItems.begin();
					break;
				}
			}
			for (int j = m_wndStyleID.GetCount()-1; j >= 0; --j)
			{
				if (m_wndStyleID.GetItemData(j) == iSel)
				{
					m_wndStyleID.SetCurSel(j);
					break;
				}
			}

			m_bstrCurID = cID;
		}

		if (m_wndPreview.m_hWnd)
		{
			CConfigValue cColor1;
			M_Config()->ItemValueGet(CComBSTR(CFGID_FL_COLOR1), &cColor1);
			CConfigValue cColor2;
			M_Config()->ItemValueGet(CComBSTR(CFGID_FL_COLOR2), &cColor2);
			CConfigValue cStyleID;
			M_Config()->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cStyleID);
			CComPtr<IConfig> pSubCfg;
			M_Config()->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pSubCfg);
			CConfigValue cParams;
			if (pSubCfg)
				pSubCfg->ItemValueGet(cID, &cParams);
			if (cParams.TypeGet() == ECVTString && SysStringLen(cParams) > 0)
			{
				CComBSTR bstrParams;
				bstrParams.Attach(cParams.Detach().bstrVal);
				if (cColor1[3] != -1.0f)
				{
					UpdateFillStyle(cStyleID, bstrParams, cColor1, cColor2);
				}
				m_wndPreview.UpdateStyle(m_pFSMgr, cID, bstrParams);
				CComPtr<ISharedState> pState;
				m_wndPreview.State(&pState);
				m_pCurWnd->SetState(pState);
			}
			else if (cColor1[3] != -1.0f)
			{
				CComBSTR bstrParams;
				if (GetDefaultFillStyle(cStyleID, bstrParams, cColor1, cColor2))
					m_wndPreview.UpdateStyle(m_pFSMgr, cID, bstrParams);
				else
					m_wndPreview.UpdateStyle(m_pFSMgr, cID, NULL);
			}
			else
			{
				m_wndPreview.UpdateStyle(m_pFSMgr, cID, NULL);
			}
		}
	}

	// ISharedStateManager methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (a_bstrCategoryName == NULL || *a_bstrCategoryName == L'\0')
			return E_FAIL;
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_bstrCategoryName == NULL || *a_bstrCategoryName == L'\0')
			return E_FAIL;

		if (a_pState == NULL)
		{
			m_cStates.erase(a_bstrCategoryName);
		}
		else
		{
			m_cStates[a_bstrCategoryName] = a_pState;
			TSharedStateChange tChg;
			tChg.bstrName = a_bstrCategoryName;
			tChg.pState = a_pState;
			Fire_Notify(tChg);
			if (/*m_wndPreview.m_hWnd && */m_bstrCurID == a_bstrCategoryName)
				m_wndPreview.State(a_pState);
		}
		return S_OK;
	}

private:
	typedef std::vector<CComBSTR> CItems;
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	CComboBox m_wndStyleID;
	bool m_bUpdating;

	CComPtr<IRasterImageFillStyleManager> m_pFSMgr;
	CItems m_cItems;
	CComBSTR m_bstrCurID;
	CComPtr<IRasterImageEditToolWindow> m_pCurWnd;
	RECT m_rcGaps;

	CStates m_cStates;

	CGradientPreview m_wndPreview;
};



// CRasterImageFill

STDMETHODIMP CRasterImageFill::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEFILL_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CRasterImageFill::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Fill[0405]Vyplnění";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cID);

		CComPtr<IRasterImageFillStyleManager> pFSMgr;
		RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
		CComPtr<IEnumStrings> pIDs;
		pFSMgr->StyleIDsEnum(&pIDs);
		ULONG nIDs = 0;
		if (pIDs) pIDs->Size(&nIDs);
		for (ULONG i = 0; i < nIDs; ++i)
		{
			CComBSTR bstrID;
			pIDs->Get(i, &bstrID);
			if (bstrID != cID.operator BSTR())
				continue;
			CComPtr<ILocalizedString> pName;
			return pFSMgr->StyleNameGet(bstrID, a_ppName);
		}
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageFill::PreviewIconID(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	try
	{
		if (a_pConfig)
		{
			CConfigValue cID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cID);

			CComPtr<IRasterImageFillStyleManager> pFSMgr;
			RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
			CComPtr<IEnumStrings> pIDs;
			pFSMgr->StyleIDsEnum(&pIDs);
			ULONG nIDs = 0;
			if (pIDs) pIDs->Size(&nIDs);
			for (ULONG i = 0; i < nIDs; ++i)
			{
				CComBSTR bstrID;
				pIDs->Get(i, &bstrID);
				if (bstrID != cID.operator BSTR())
					continue;
				CComPtr<ILocalizedString> pName;
				return pFSMgr->StyleIconIDGet(bstrID, a_pIconID);
			}
		}
		// {AE01093E-D902-4029-AC33-A0DB0168D8C3}
		static const GUID tID = {0xae01093e, 0xd902, 0x4029, {0xac, 0x33, 0xa0, 0xdb, 0x1, 0x68, 0xd8, 0xc3}};
		*a_pIconID = tID;//CLSID_RasterImageFill;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

//#include <RenderIcon.h>
#include <IconRenderer.h>

STDMETHODIMP CRasterImageFill::PreviewIcon(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		if (a_pConfig)
		{
			CConfigValue cID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cID);

			CComPtr<IRasterImageFillStyleManager> pFSMgr;
			RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
			CComPtr<IEnumStrings> pIDs;
			pFSMgr->StyleIDsEnum(&pIDs);
			ULONG nIDs = 0;
			if (pIDs) pIDs->Size(&nIDs);
			for (ULONG i = 0; i < nIDs; ++i)
			{
				CComBSTR bstrID;
				pIDs->Get(i, &bstrID);
				if (bstrID != cID.operator BSTR())
					continue;
				CComPtr<ILocalizedString> pName;
				return pFSMgr->StyleIconGet(bstrID, a_nSize, a_phIcon);
			}
		}

		*a_phIcon = GetDefaultIcon(a_nSize);
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HICON CRasterImageFill::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IIconRenderer> pIR;
	RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	IRPolyPoint const poly0[] = {{15.3847, 147.588}, {36.9062, 169.11}, {169.11, 36.9062}, {162.961, 30.7572}, {172.184, 21.5337}, {206.772, 56.1219}, {151.431, 160.655}, {183.714, 192.937}, {192.937, 183.714}, {169.11, 159.886}, {224.451, 55.3532}, {172.184, 3.0867}, {153.737, 21.5337}, {147.588, 15.3847}};
	IRPolyPoint const poly1[] = {{194.474, 163.73}, {255.965, 225.219}, {225.22, 255.964}, {163.729, 194.475}};
	IRPolyPoint const poly2[] = {{184.482, 61.5027}, {61.5017, 184.482}, {0.0120258, 122.992}, {122.993, 0.0123634}};
	IRPolyPoint const poly3[] = {{135.29, 43.0555}, {43.0549, 135.29}, {30.757, 122.992}, {122.992, 30.7574}};
	IRPolygon const shape0 = {itemsof(poly0), poly0};
	IRPolygon const shape1 = {itemsof(poly1), poly1};
	IRPolygon const shape2 = {itemsof(poly2), poly2};
	IRPolygon const shape3 = {itemsof(poly3), poly3};
	IRCanvas const canvas0 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	//IRFillWithInternalOutline roller(0xffed9191, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 256/20.0f);
	//IRFill highlight(0xffefc6c6);
	//IRFillWithInternalOutline handle(0xff000000|GetSysColor(COLOR_WINDOW), 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 256/20.0f);
	//IRFill structure(0xff000000|GetSysColor(COLOR_WINDOWTEXT));
	IRFill highlight(0x7fffffff);
	IRLayer const layers[] =
	{
		{&canvas0, 1, 0, &shape0, NULL, pSI->GetMaterial(ESMContrast)},//&structure},
		{&canvas0, 1, 0, &shape1, NULL, pSI->GetMaterial(ESMScheme1Color2)},//&handle},
		{&canvas0, 1, 0, &shape2, NULL, pSI->GetMaterial(ESMScheme1Color1)},//&roller},
		{&canvas0, 1, 0, &shape3, NULL, &highlight},
	};
	return pIR->CreateIcon(a_nSize, itemsof(layers), layers);
}

STDMETHODIMP CRasterImageFill::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComPtr<ISubConfig> pSubCfg;
		RWCoCreateInstance(pSubCfg, __uuidof(ConfigInMemory));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_FSID), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSID_DESC), CConfigValue(L"LINEAR"), pSubCfg, 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_FSCONFIG), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSCONFIG_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSCONFIG_DESC), CConfigValue(L""), NULL, 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_FSSTATE), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSSTATE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_FSSTATE_DESC), CConfigValue(L""), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_COLOR1), NULL, NULL, CConfigValue(0.0f, 0.0f, 0.0f, -1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_COLOR2), NULL, NULL, CConfigValue(0.0f, 0.0f, 0.0f, -1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FL_RESPECTALPHA), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_RESPECTALPHA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_RESPECTALPHA_DESC), CConfigValue(true), NULL, 0, NULL);
		CComBSTR cCFGID_FL_BLENDINGMODE(CFGID_FL_BLENDINGMODE);
		pCfgInit->ItemIns1ofN(cCFGID_FL_BLENDINGMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_FL_BLENDINGMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FL_BLENDINGMODE_DESC), CConfigValue(LONG(EBMDrawOver)), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FL_BLENDINGMODE, CConfigValue(LONG(EBMReplace)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_FLBM_REPLACE), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FL_BLENDINGMODE, CConfigValue(LONG(EBMDrawOver)), _SharedStringTable.GetStringAuto(IDS_CFGVAL_FLBM_PAINTOVER), 0, NULL);

		CConfigCustomGUI<&CLSID_RasterImageFill, CConfigGUIFill, false>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageFill::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentRasterImage)/*, __uuidof(IRasterImageControl)*/};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


class ATL_NO_VTABLE CFillDocOpBrushOwner :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrushOwner
{
public:
	void Init(ULONG a_nSizeX, ULONG a_nSizeY)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
	}

BEGIN_COM_MAP(CFillDocOpBrushOwner)
	COM_INTERFACE_ENTRY(IRasterImageBrushOwner)
END_COM_MAP()


	// IRasterImageBrushOwner methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		if (a_pSizeX) *a_pSizeX = m_nSizeX;
		if (a_pSizeY) *a_pSizeY = m_nSizeY;
		return S_OK;
	}
	STDMETHOD(ControlPointsChanged)() { return S_FALSE; }
	STDMETHOD(ControlPointChanged)(ULONG UNREF(a_nIndex)) { return S_FALSE; }
	STDMETHOD(RectangleChanged)(RECT const* UNREF(a_pChanged)) { return S_FALSE; }
	STDMETHOD(ControlLinesChanged)() { return S_FALSE; }
	STDMETHOD(SetBrushState)(ISharedState*) { return S_FALSE; }

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
};


STDMETHODIMP CRasterImageFill::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		TImageSize const tOrigSize = tSize;
		TRasterImageRect tR = {{0, 0}, {tSize.nX, tSize.nY}};

		CConfigValue cStyleID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cStyleID);
		//CConfigValue cConfig;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSCONFIG), &cConfig);
		//CConfigValue cState;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSSTATE), &cState);
		CConfigValue cColor1;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR1), &cColor1);
		CConfigValue cColor2;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR2), &cColor2);
		CConfigValue cRespectAlpha;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_RESPECTALPHA), &cRespectAlpha);
		CConfigValue cBlendingMode;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_BLENDINGMODE), &cBlendingMode);

		CComPtr<IRasterImageFillStyleManager> pFSMgr;
		RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
		CComPtr<IRasterImageBrush> pFS;
		pFSMgr->FillStyleCreate(cStyleID, NULL/* ??? */, &pFS);
		if (pFS == NULL)
			return E_FAIL;
		CComObject<CFillDocOpBrushOwner>* pOwner = NULL;
		CComObject<CFillDocOpBrushOwner>::CreateInstance(&pOwner);
		CComPtr<IRasterImageBrushOwner> pOwner2 = pOwner;
		pOwner->Init(tOrigSize.nX, tOrigSize.nY);
		pFS->Init(pOwner2);

		CComQIPtr<IRasterImageEditToolScripting> pScripting(pFS);
		CComPtr<IConfig> pStates;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pStates);
		if (pStates && pScripting)
		{
			CConfigValue cState;
			pStates->ItemValueGet(cStyleID, &cState);
			if (cState.TypeGet() == ECVTString && SysStringLen(cState) > 0)
			{
				CComBSTR bstrParams;
				bstrParams.Attach(cState.Detach().bstrVal);
				if (cColor1[3] != -1.0f)
				{
					UpdateFillStyle(cStyleID, bstrParams, cColor1, cColor2);
				}
				pScripting->FromText(bstrParams);
			}
			else if (cColor1[3] != -1.0f)
			{
				CComBSTR bstrParams;
				if (GetDefaultFillStyle(cStyleID, bstrParams, cColor1, cColor2))
					pScripting->FromText(bstrParams);
			}
		}

		TPixelCoords tCenter = {tOrigSize.nX*0.5f, tOrigSize.nY*0.5f};
		pFS->SetShapeBounds(&tCenter, tOrigSize.nX*0.5f, tOrigSize.nY*0.5f, 0.0f);

		if (pFS->NeedsPrepareShape() == S_OK)
		{
			// TODO: support fill styles, which require preparation
			//pFS->PrepareShape();
		}
		RECT const rc = {tR.tTL.nX, tR.tTL.nY, tR.tBR.nX, tR.tBR.nY};
		SIZE const sz = {rc.right-rc.left, rc.bottom-rc.top};
		//if (pFS->IsSolid(&rc)) // optimize?
		CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[sz.cx*sz.cy]);
		pFS->GetBrushTile(rc.left, rc.top, sz.cx, sz.cy, 2.2f, sz.cx, reinterpret_cast<TRasterImagePixel*>(pBuffer.m_p));

		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[tSize.nX*tSize.nY]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tR.tTL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		TPixelChannel* p = pSrc.m_p;
		LONG nSize = tSize.nX*tSize.nY;
		TPixelChannel* const pEnd = p + nSize;
		TPixelChannel const* pS = pBuffer;
		if (cRespectAlpha.operator bool())
		{
			if (cBlendingMode.operator LONG() == EBMDrawOver)
			{
				CAutoPtr<CGammaTables> pGT(new CGammaTables);
				while (p != pEnd)
				{
					if (p->bA)
					{
						ULONG const bIA = 255-pS->bA;
						p->bR = pGT->InvGamma((pGT->m_aGamma[p->bR]*bIA + pGT->m_aGamma[pS->bR]*ULONG(pS->bA))/255);
						p->bG = pGT->InvGamma((pGT->m_aGamma[p->bG]*bIA + pGT->m_aGamma[pS->bG]*ULONG(pS->bA))/255);
						p->bB = pGT->InvGamma((pGT->m_aGamma[p->bB]*bIA + pGT->m_aGamma[pS->bB]*ULONG(pS->bA))/255);
					}
					++p;
					++pS;
				}
			}
			else
			{
				while (p != pEnd)
				{
					if (p->bA == 255)
					{
						*p = *pS;
					}
					else if (p->bA)
					{
						p->bR = pS->bR;
						p->bG = pS->bG;
						p->bB = pS->bB;
						p->bA = ULONG(pS->bA)*p->bA/255;
					}
					++p;
					++pS;
				}
			}
		}
		else
		{
			if (cBlendingMode.operator LONG() == EBMDrawOver)
			{
				CAutoPtr<CGammaTables> pGT(new CGammaTables);
				while (p != pEnd)
				{
					if (pS->bA == 255)
					{
						*p = *pS;
					}
					else
					{
						ULONG nNewA = pS->bA*255 + (255-pS->bA)*p->bA;
						if (nNewA)
						{
							ULONG const bA1 = (255-pS->bA)*p->bA;
							ULONG const bA2 = pS->bA*255;
							p->bR = pGT->InvGamma((pGT->m_aGamma[p->bR]*bA1 + pGT->m_aGamma[pS->bR]*bA2)/nNewA);
							p->bG = pGT->InvGamma((pGT->m_aGamma[p->bG]*bA1 + pGT->m_aGamma[pS->bG]*bA2)/nNewA);
							p->bB = pGT->InvGamma((pGT->m_aGamma[p->bB]*bA1 + pGT->m_aGamma[pS->bB]*bA2)/nNewA);
						}
						else
						{
							p->bR = 0;
							p->bG = 0;
							p->bB = 0;
						}
						p->bA = nNewA/255;
					}
					++p;
					++pS;
				}
			}
			else
			{
				while (p != pEnd)
				{
					*p = *pS;
					++p;
					++pS;
				}
			}
		}
		return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageFill::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pContentTransform == NULL)
		return S_FALSE;

	try
	{
		CConfigValue cStyleID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &cStyleID);
		//CConfigValue cConfig;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSCONFIG), &cConfig);
		//CConfigValue cState;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSSTATE), &cState);
		CConfigValue cColor1;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR1), &cColor1);
		CConfigValue cColor2;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR2), &cColor2);
		CConfigValue cRespectAlpha;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_RESPECTALPHA), &cRespectAlpha);
		CConfigValue cBlendingMode;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_BLENDINGMODE), &cBlendingMode);

		CComPtr<IRasterImageFillStyleManager> pFSMgr;
		RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
		CComPtr<IRasterImageBrush> pFS;
		pFSMgr->FillStyleCreate(cStyleID, NULL/* ??? */, &pFS);
		if (pFS == NULL)
			return E_FAIL;
		CComObject<CFillDocOpBrushOwner>* pOwner = NULL;
		CComObject<CFillDocOpBrushOwner>::CreateInstance(&pOwner);
		CComPtr<IRasterImageBrushOwner> pOwner2 = pOwner;
		TImageSize tSize = {256, 256};
		if (a_pCanvas) tSize = *a_pCanvas;
		pOwner->Init(tSize.nX, tSize.nY);
		pFS->Init(pOwner2);

		CComQIPtr<IRasterImageEditToolScripting> pScripting(pFS);
		CComPtr<IConfig> pStates;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pStates);
		if (pStates && pScripting)
		{
			CConfigValue cState;
			pStates->ItemValueGet(cStyleID, &cState);
			CComBSTR bstrParams;
			if (cState.TypeGet() == ECVTString && SysStringLen(cState) > 0)
			{
				bstrParams.Attach(cState.Detach().bstrVal);
				if (cColor1[3] != -1.0f)
				{
					UpdateFillStyle(cStyleID, bstrParams, cColor1, cColor2);
				}
				pScripting->FromText(bstrParams);
			}
			else if (cColor1[3] != -1.0f)
			{
				if (GetDefaultFillStyle(cStyleID, bstrParams, cColor1, cColor2))
					pScripting->FromText(bstrParams);
			}
			if (S_OK == pFS->Transform(a_pContentTransform))
			{
				CComBSTR newVal;
				pScripting->ToText(&newVal);
				if (newVal.m_str && newVal != bstrParams)
				{
					TConfigValue tVal;
					tVal.eTypeID = ECVTString;
					tVal.bstrVal = newVal;
					BSTR b = cStyleID;
					return pStates->ItemValuesSet(1, &b, &tVal);
				}
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

// ICanvasInteractingWrapper methods

IRasterImageBrush* CRasterImageFill::CWrapper::Init(ULONG a_nSizeX, ULONG a_nSizeY, CConfigValue const& a_cStyleID, BSTR a_bstrParams)
{
	try
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		m_cStyleID = a_cStyleID;

		CComPtr<IRasterImageFillStyleManager> pFSMgr;
		RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
		pFSMgr->FillStyleCreate(m_cStyleID, NULL, &m_pFS);
		if (m_pFS == NULL)
			return NULL;
		CComObject<CFillDocOpBrushOwner>* pOwner = NULL;
		CComObject<CFillDocOpBrushOwner>::CreateInstance(&pOwner);
		CComPtr<IRasterImageBrushOwner> pOwner2 = pOwner;
		pOwner->Init(a_nSizeX, a_nSizeY);
		m_pFS->Init(pOwner2);

		CComQIPtr<IRasterImageEditToolScripting> pScripting(m_pFS);
		if (a_bstrParams)
			pScripting->FromText(a_bstrParams);

		return m_pFS;
	}
	catch (...)
	{
		return NULL;
	}
}

void CRasterImageFill::CWrapper::Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
{
	try
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSID), &m_cStyleID);
		//CConfigValue cConfig;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSCONFIG), &cConfig);
		//CConfigValue cState;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_FSSTATE), &cState);
		CConfigValue cColor1;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR1), &cColor1);
		CConfigValue cColor2;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FL_COLOR2), &cColor2);

		CComPtr<IRasterImageFillStyleManager> pFSMgr;
		RWCoCreateInstance(pFSMgr, __uuidof(RasterImageFillStyleManager));
		pFSMgr->FillStyleCreate(m_cStyleID, NULL, &m_pFS);
		if (m_pFS == NULL)
			return;
		CComObject<CFillDocOpBrushOwner>* pOwner = NULL;
		CComObject<CFillDocOpBrushOwner>::CreateInstance(&pOwner);
		CComPtr<IRasterImageBrushOwner> pOwner2 = pOwner;
		pOwner->Init(a_nSizeX, a_nSizeY);
		m_pFS->Init(pOwner2);

		CComQIPtr<IRasterImageEditToolScripting> pScripting(m_pFS);
		CComPtr<IConfig> pStates;
		a_pConfig->SubConfigGet(CComBSTR(CFGID_FL_FSID), &pStates);
		if (pStates && pScripting)
		{
			CConfigValue cState;
			pStates->ItemValueGet(m_cStyleID, &cState);
			if (cState.TypeGet() == ECVTString && SysStringLen(cState) > 0)
			{
				CComBSTR bstrParams;
				bstrParams.Attach(cState.Detach().bstrVal);
				if (cColor1[3] != -1.0f)
				{
					UpdateFillStyle(m_cStyleID, bstrParams, cColor1, cColor2);
				}
				pScripting->FromText(bstrParams);
			}
			else if (cColor1[3] != -1.0f)
			{
				CComBSTR bstrParams;
				if (GetDefaultFillStyle(m_cStyleID, bstrParams, cColor1, cColor2))
					pScripting->FromText(bstrParams);
			}

		}
	}
	catch (...)
	{
	}
}

STDMETHODIMP_(ULONG) CRasterImageFill::CWrapper::GetControlPointCount()
{
	ULONG n = 0;
	if (m_pFS)
		m_pFS->GetControlPointCount(&n);
	return n;
}

STDMETHODIMP CRasterImageFill::CWrapper::GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
{
	if (m_pFS == NULL)
		return E_RW_INDEXOUTOFRANGE;
	return m_pFS->GetControlPoint(a_nIndex, a_pPos, a_pClass);
}

STDMETHODIMP CRasterImageFill::CWrapper::SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel)
{
	if (m_pFS == NULL)
		return E_RW_INDEXOUTOFRANGE;
	if (a_ppNew == NULL)
		return E_POINTER;

	try
	{
		CComBSTR bstrParams;
		CComQIPtr<IRasterImageEditToolScripting> pScripting(m_pFS);
		if (pScripting)
			pScripting->ToText(&bstrParams);

		CComObject<CWrapper>* p = NULL;
		CComObject<CWrapper>::CreateInstance(&p);
		CComPtr<ICanvasInteractingWrapper> pNew = p;
		IRasterImageBrush* pFS = p->Init(m_nSizeX, m_nSizeY, m_cStyleID, bstrParams);
		if (pFS)
			pFS->SetControlPoint(a_nIndex, a_pPos, a_bReleased, a_fPointSize);
		*a_ppNew = pNew.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageFill::CWrapper::GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
{
	if (m_pFS == NULL)
		return E_RW_INDEXOUTOFRANGE;
	return m_pFS->GetControlPointDesc(a_nIndex, a_ppDescription);
}

STDMETHODIMP CRasterImageFill::CWrapper::GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
{
	if (m_pFS == NULL)
		return E_NOTIMPL;
	return m_pFS->GetControlLines(a_pLines);
}

STDMETHODIMP CRasterImageFill::CWrapper::ToConfig(IConfig* a_pConfig)
{
	CComBSTR bstrCFGID_FL_FSID(CFGID_FL_FSID);
	a_pConfig->ItemValuesSet(1, &(bstrCFGID_FL_FSID.m_str), m_cStyleID);
	CComPtr<IConfig> pSub;
	a_pConfig->SubConfigGet(bstrCFGID_FL_FSID, &pSub);
	CComBSTR bstrParam;
	CComQIPtr<IRasterImageEditToolScripting> pScripting(m_pFS);
	if (pScripting)
		pScripting->ToText(&bstrParam);
	if (bstrParam)
	{
		BSTR b = m_cStyleID;
		pSub->ItemValuesSet(1, &b, CConfigValue(bstrParam));
	}
	return S_OK;
}
