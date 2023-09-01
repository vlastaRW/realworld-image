
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_ORIGIN[] = L"Origin";
static OLECHAR const CFGID_SCALE[] = L"Scale";
static OLECHAR const CFGID_ROTATION[] = L"Rotation";
static OLECHAR const CFGID_TRANSLATION[] = L"Translation";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIShapeTransformDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIShapeTransformDlg>,
	public CDialogResize<CConfigGUIShapeTransformDlg>
{
public:
	enum
	{
		IDC_SCALE_EDIT = 100,
		IDC_SCALE_SLIDER,
		IDC_ROTATION_EDIT,
		IDC_ROTATION_SLIDER,
		IDC_TRANSLATION,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 72, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Scale:[0405]Škála:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_SCALE_EDIT, 50, 0, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_SCALE_SLIDER, TRACKBAR_CLASS, TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 12, 100, 12, 0)
		CONTROL_LTEXT(_T("[0409]Rotation:[0405]Rotace:"), IDC_STATIC, 0, 30, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ROTATION_EDIT, 50, 28, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_ROTATION_SLIDER, TRACKBAR_CLASS, TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 40, 100, 12, 0)
		CONTROL_LTEXT(_T("[0409]Translation:[0405]Posun:"), IDC_STATIC, 0, 58, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_TRANSLATION, 50, 60, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIShapeTransformDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIShapeTransformDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIShapeTransformDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIShapeTransformDlg)
		DLGRESIZE_CONTROL(IDC_SCALE_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_SCALE_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ROTATION_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ROTATION_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_TRANSLATION, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIShapeTransformDlg)
		CONFIGITEM_EDITBOX(IDC_SCALE_EDIT, CFGID_SCALE)
		CONFIGITEM_EDITBOX(IDC_ROTATION_EDIT, CFGID_ROTATION)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_ROTATION_SLIDER, CFGID_ROTATION)
		CONFIGITEM_EDITBOX(IDC_TRANSLATION, CFGID_TRANSLATION)
	END_CONFIGITEM_MAP()

	void ExtraConfigNotify()
	{
		if (m_scaleBar.m_hWnd)
		{
			CConfigValue scale;
			M_Config()->ItemValueGet(CComBSTR(CFGID_SCALE), &scale);
			float val = 50.5f+logf(sqrtf(fabsf(scale[0]*scale[1])))/logf(8.0f)*50.0f;
			if (val < 0) val = 0; else if (val > 100) val = 100;
			m_scaleBar.SetPos(val);
		}
	}
	void ExtraInitDialog()
	{
		m_scaleBar = GetDlgItem(IDC_SCALE_SLIDER);
		m_scaleBar.SetRange(0, 100);
		//m_scaleBar.Set
	}
	bool ValueToText(WCHAR const* id, TConfigValue const& val, TCHAR* text, ULONG len)
	{
		if (wcscmp(id, CFGID_SCALE) != 0 || val.vecVal[0] != val.vecVal[1])
			return false;
		_sntprintf(text, len, _T("%g"), val.vecVal[0]);
		return true;
	}
	bool TextToValue(WCHAR const* id, TCHAR const* text, CConfigValue const& old, CConfigValue& val)
	{
		if (wcscmp(id, CFGID_SCALE) != 0 || _tcschr(text, _T(',')))
			return false;
		LPCTSTR p = text;
		float const f = CInPlaceCalc::EvalExpression(text, &p);
		if (*p == 0 || *p == _T(' '))
		{
			val = CConfigValue(f, f);
			return true;
		}
		return false;
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_lParam) == m_scaleBar.m_hWnd)
		{
			CComBSTR bstr(CFGID_SCALE);
			CConfigValue scale;
			M_Config()->ItemValueGet(bstr, &scale);
			float const s = sqrtf(fabsf(scale[0]*scale[1]));
			float const n = exp((m_scaleBar.GetPos()-50)/50.0f*logf(8.0f));
			M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(scale[0]*n/s, scale[1]*n/s));
		}
		else
		{
			a_bHandled = FALSE;
		}
		return 0;
	}

public:
	CTrackBarCtrl m_scaleBar;
};


// CDocumentOperationTransformShape

extern GUID const CLSID_DocumentOperationTransformShape = {0xf0016949, 0x8f30, 0x4551, {0x97, 0xfc, 0xd2, 0x63, 0xd4, 0x91, 0xe5, 0x7e}};

class ATL_NO_VTABLE CDocumentOperationTransformShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationTransformShape, &CLSID_DocumentOperationTransformShape>,
	public IDocumentOperation
{
public:
	CDocumentOperationTransformShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationTransformShape)

BEGIN_CATEGORY_MAP(CDocumentOperationTransformShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationTransformShape)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Transform object[0405]Vektorový obrázek - transformovat objekt");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
			pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);

			//pCfgInit->ItemInsSimple(CComBSTR(CFGID_ORIGIN), CMultiLanguageString::GetAuto(L"[0409]Origin[0405]Počátek"), CMultiLanguageString::GetAuto(L"[0409]Origin point for scaling and rotation.[0405]Počáteční bod pro škálování a rotaci."), CConfigValue(0.0f, 0.0f, 0.0f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SCALE), CMultiLanguageString::GetAuto(L"[0409]Scale[0405]Škálování"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(1.0f, 1.0f), NULL, 0, NULL);
			pCfgInit->ItemInsRanged(CComBSTR(CFGID_ROTATION), CMultiLanguageString::GetAuto(L"[0409]Rotation[0405]Otočení"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(0.0f), NULL, CConfigValue(-180.0f), CConfigValue(180.0f), CConfigValue(0.0f), 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSLATION), CMultiLanguageString::GetAuto(L"[0409]Translation[0405]Posun"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(0.0f, 0.0f), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_DocumentOperationTransformShape, CConfigGUIShapeTransformDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (a_pDocument == NULL)
				return S_FALSE;
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return S_FALSE;
			if (a_pConfig == NULL)
				return S_OK;
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			ULONG nSelected = 0;
			pDVI->StateUnpack(pState, &CEnumItemCounter<IEnum2UInts, ULONG>(&nSelected));
			return nSelected ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return E_FAIL;
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> cIDs;
			pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				return S_FALSE;

			CConfigValue cScale;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SCALE), &cScale);
			CConfigValue cRotation;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ROTATION), &cRotation);
			CConfigValue cTranslation;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSLATION), &cTranslation);
			TMatrix3x3f const tScale = {cScale[0], 0.0f, 0.0f, 0.0f, cScale[1], 0.0f, 0.0f, 0.0f, 1.0f};
			float const fSin = sinf(cRotation.operator float()*3.14159265359f/180.0f);
			float const fCos = cosf(cRotation.operator float()*3.14159265359f/180.0f);
			TMatrix3x3f const tRotation = {fCos, fSin, 0.0f, -fSin, fCos, 0.0f, 0.0f, 0.0f, 1.0f};
			TMatrix3x3f const tTranslation = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, cTranslation[0], cTranslation[1], 1.0f};

			CWriteLock<IBlockOperations> cLock(a_pDocument);

			TMatrix3x3f tOrigin = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			{
				CComPtr<IDocumentImage> pDI;
				a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
				if (pDI)
				{
					TImageSize tSize = {0.0f, 0.0f};
					pDI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
					tOrigin._31 = -0.5f*(tSize.nX);
					tOrigin._32 = -0.5f*(tSize.nY);
				}
			}
			TMatrix3x3f tmp1, tmp2;
			Matrix3x3fMultiply(tOrigin, tScale, &tmp1);
			Matrix3x3fMultiply(tmp1, tRotation, &tmp2);
			Matrix3x3fMultiply(tmp2, tTranslation, &tmp1);
			tOrigin._31 = -tOrigin._31;
			tOrigin._32 = -tOrigin._32;
			Matrix3x3fMultiply(tmp1, tOrigin, &tmp2);

			ULONG nNewSel = 0;
			ULONG nPrevFrame = 0;
			bool bSelectNext = false;
			for (std::vector<ULONG>::const_iterator i = cIDs.begin(); i != cIDs.end(); ++i)
				pDVI->ObjectTransform(*i, &tmp2);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationTransformShape, CDocumentOperationTransformShape)
