// DocumentOperationRasterImageFade.cpp : Implementation of CDocumentOperationRasterImageFade

#include "stdafx.h"
#include "DocumentOperationRasterImageFade.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_FD_STRENGTH[] = L"Strength";
const OLECHAR CFGID_FD_OPERATION[] = L"Operation";


// CDocumentOperationRasterImageFade

STDMETHODIMP CDocumentOperationRasterImageFade::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEFADE_NAME);
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

STDMETHODIMP CDocumentOperationRasterImageFade::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Fade effect[0405]Potlačení efektu");
			return S_OK;
		}
		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(L"[0409]Fade[0405]Potlačit"));
		CComBSTR bstrID(CFGID_FD_OPERATION);
		CComPtr<IConfig> pSubCfg;
		a_pConfig->SubConfigGet(bstrID, &pSubCfg);
		CConfigValue cID;
		a_pConfig->ItemValueGet(bstrID, &cID);
		CComPtr<IDocumentOperation> pOp;
		RWCoCreateInstance(pOp, cID.operator const GUID &());
		CComQIPtr<IConfigDescriptor> pOA(pOp);
		CComPtr<ILocalizedString> pSubName;
		if (pOA) pOA->Name(a_pContext, pSubCfg, &pSubName);
		if (pSubName == NULL && pOp) pOp->NameGet(CComQIPtr<IOperationManager>(a_pContext), &pSubName);
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s: %s")));
		pStr->Init(pTempl, pName, pSubName);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIFadeDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIFadeDlg>,
	public CDialogResize<CConfigGUIFadeDlg>,
	public IResizableConfigWindow
{
public:
	CConfigGUIFadeDlg()
	{
	}

	enum
	{
		IDC_CGF_OPERATION_LABEL = 100,
		IDC_CGF_OPERATION,
		IDC_CGF_CONFIG,
		IDC_CGF_LABEL,
		IDC_CGF_EDIT,
		IDC_CGF_UPDOWN,
		IDC_CGF_SLIDER,
		IDC_CGF_PERCENT,
	};

	BEGIN_DIALOG_EX(0, 0, 120, (M_Mode() == ECPMFull ? 34 : 18), 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	if (M_Mode() == ECPMFull)
	{
		CONTROL_LTEXT(_T("[0409]Operation:[0405]Operace:"), IDC_CGF_OPERATION_LABEL, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGF_OPERATION, 50, 0, 70, 170, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CGF_CONFIG, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 16, 120, 2, 0)
		CONTROL_LTEXT(_T("[0409]Strength:[0405]Síla:"), IDC_CGF_LABEL, 0, 24, 45, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGF_EDIT, 78, 22, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGF_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 22, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGF_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 22, 33, 12, 0)
		CONTROL_RTEXT(_T("%"), IDC_CGF_PERCENT, 112, 24, 8, 8, WS_VISIBLE, 0)
	}
	else
	{
		CONTROL_CONTROL(_T(""), IDC_CGF_CONFIG, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 0, 120, 2, 0)
		CONTROL_LTEXT(_T("[0409]Strength:[0405]Síla:"), IDC_CGF_LABEL, 0, 8, 45, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CGF_EDIT, 78, 6, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CGF_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 6, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_CGF_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 6, 33, 12, 0)
		CONTROL_RTEXT(_T("%"), IDC_CGF_PERCENT, 112, 8, 8, 8, WS_VISIBLE, 0)
	}
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIFadeDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIFadeDlg>)
		NOTIFY_HANDLER(IDC_CGF_UPDOWN, UDN_DELTAPOS, OnUpDownChange)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIFadeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	const _AtlDlgResizeMap* GetDlgResizeMap()
	{
		static const _AtlDlgResizeMap theMap[] =
		{
		DLGRESIZE_CONTROL(IDC_CGF_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGF_LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_SLIDER, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_EDIT, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_UPDOWN, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_PERCENT, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CGF_CONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			{ -1, 0 },
		};
		return M_Mode() == ECPMFull ? theMap : theMap+1;
	}

	SCustomConfigControlMap const* GetCustomConfigControlMap()
	{
		static SCustomConfigControlMap const sMap[] =
		{
		CONFIGITEM_COMBOBOX(IDC_CGF_OPERATION, CFGID_FD_OPERATION)
		CONFIGITEM_EDITBOX(IDC_CGF_EDIT, CFGID_FD_STRENGTH)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGF_SLIDER, CFGID_FD_STRENGTH)
		CONFIGITEM_SUBCONFIG_NOMARGINS(IDC_CGF_CONFIG, CFGID_FD_OPERATION)
		{ ECCCTInvalid, 0}
		};
		return M_Mode() == ECPMFull ? sMap : sMap+1;
	}

	BEGIN_COM_MAP(CConfigGUIFadeDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || m_pSubWnd == NULL || a_pSize == NULL)
			return E_UNEXPECTED;
		RECT rc = {0, 0, 120, 16};
		MapDialogRect(&rc);
		SIZE sz = {0, 0};
		m_pSubWnd->OptimumSize(&sz);
		a_pSize->cx = max(rc.right, sz.cx);
		a_pSize->cy = rc.bottom+sz.cy;
		return S_OK;
	}

	void SubConfigWindowCreated(UINT nIDC, IConfigWnd* pConfigWnd)
	{
		if (nIDC == IDC_CGF_CONFIG)
			m_pSubWnd = pConfigWnd;
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CUpDownCtrl(GetDlgItem(IDC_CGF_UPDOWN)).SetRange(-500, 500);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnUpDownChange(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		NMUPDOWN tNMUD = *reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);
		tNMUD.iDelta *= 10;
		return CCustomConfigResourcelessWndImpl<CConfigGUIFadeDlg>::OnUpDownChange(a_idCtrl, &tNMUD.hdr, a_bHandled);
	}

	// scaling for CFGID_FD_STRENGTH: 0-100% on screen <-> 0-1 internally
	bool GetSliderRange(wchar_t const* UNREF(a_pszName), TConfigValue* a_pFrom, TConfigValue* a_pTo, TConfigValue* a_pStep)
	{
		a_pFrom->eTypeID = a_pTo->eTypeID = a_pStep->eTypeID = ECVTFloat;
		a_pFrom->fVal = -500;
		a_pTo->fVal = 500;
		a_pStep->fVal = 10.0f;
		return true;
	}
	void FloatToValue(WCHAR const* id, float f, CConfigValue& val)
	{
		val = f/100.0f;
	}
	float ValueToFloat(WCHAR const* id, TConfigValue const& val)
	{
		return floorf(val.fVal*10000.0f+0.5f)/100.0f;
	}

private:
	CComPtr<IConfigWnd> m_pSubWnd;
};


STDMETHODIMP CDocumentOperationRasterImageFade::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_FD_STRENGTH), _SharedStringTable.GetStringAuto(IDS_CFGID_FD_STRENGTH_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FD_STRENGTH_DESC), CConfigValue(0.5f), NULL, CConfigValue(-5.0f), CConfigValue(5.0f), CConfigValue(0.05f), 0, NULL);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_FD_OPERATION), _SharedStringTable.GetStringAuto(IDS_CFGID_FD_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FD_OPERATION_DESC), 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageFade, CConfigGUIFadeDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageFade::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

#include "FadeDocument.h"

STDMETHODIMP CDocumentOperationRasterImageFade::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CConfigValue cStrength;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FD_STRENGTH), &cStrength);

		CComBSTR bstr(CFGID_FD_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);

		CComObject<CFadeDocument>* pDoc = NULL;
		CComObject<CFadeDocument>::CreateInstance(&pDoc);
		CComPtr<IDocument> pTmp = pDoc;
		pDoc->Init(a_pDocument, pRI, cStrength);

		return a_pManager->Activate(a_pManager, pTmp, cVal, pCfg, a_pStates, a_hParent, a_tLocaleID);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageFade::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	try
	{
		CComBSTR bstr(CFGID_FD_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);

		CComPtr<IRasterImageFilter> pSubRIF;
		RWCoCreateInstance(pSubRIF, cVal);
		if (pSubRIF == NULL)
			return S_FALSE;

		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		return pSubRIF->Transform(pCfg, a_pCanvas, a_pContentTransform);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageFade::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	try
	{
		CComBSTR bstr(CFGID_FD_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);

		CComPtr<IRasterImageFilter> pSubRIF;
		RWCoCreateInstance(pSubRIF, cVal);
		if (pSubRIF == NULL)
			return E_NOTIMPL;

		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		return pSubRIF->AdjustDirtyRect(pCfg, a_pCanvas, a_pRect);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageFade::NeededToCompute(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	try
	{
		CComBSTR bstr(CFGID_FD_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);

		CComPtr<IRasterImageFilter> pSubRIF;
		RWCoCreateInstance(pSubRIF, cVal);
		if (pSubRIF == NULL)
			return E_NOTIMPL;

		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		return pSubRIF->AdjustDirtyRect(pCfg, a_pCanvas, a_pRect);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageFade::AdjustTransform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f* a_pTransform)
{
	try
	{
		CComBSTR bstr(CFGID_FD_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);

		CComPtr<IRasterImageFilter> pSubRIF;
		RWCoCreateInstance(pSubRIF, cVal);
		if (pSubRIF == NULL)
			return E_NOTIMPL;

		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		return pSubRIF->AdjustTransform(pCfg, a_pCanvas, a_pTransform);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
