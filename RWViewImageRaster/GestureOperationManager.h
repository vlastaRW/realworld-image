
#pragma once

#include <MultiLanguageString.h>

struct SGestureOperation
{
	UINT nIDS;
	GUID tID;
	HRESULT (CDesignerViewRasterEdit::*pMemFun)(IConfig* a_pCfg);
	void (*pfnConfig)(IConfigWithDependencies* a_pConfig);
};

extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHTOOLID[] = L"ToolID";
extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHSTYLEID[] = L"StyleID";
extern __declspec(selectany) OLECHAR const CFGID_GESTUREOUTLINEDDELTA[] = L"OutlineDelta";

template<OLECHAR const* t_pszID, UINT t_uName, UINT t_uDesc>
inline void GestCfgToolOrStyle(IConfigWithDependencies* a_pConfig)
{
	a_pConfig->ItemInsSimple(CComBSTR(t_pszID), _SharedStringTable.GetStringAuto(t_uName), _SharedStringTable.GetStringAuto(t_uDesc), CConfigValue(L""), NULL, 0, NULL);
}

extern __declspec(selectany) OLECHAR const CFGID_GESTUREMODETYPE[] = L"ModeType";
static LONG const CFGVAL_GMT_BLEND = 0;
static LONG const CFGVAL_GMT_RASTER = 1;
static LONG const CFGVAL_GMT_COORDS = 3;
extern __declspec(selectany) OLECHAR const CFGID_GESTUREBLENDMODE[] = L"BlendMode";
extern __declspec(selectany) OLECHAR const CFGID_GESTURERASTERMODE[] = L"RasterMode";
extern __declspec(selectany) OLECHAR const CFGID_GESTURECOORDSMODE[] = L"CoordsMode";

inline void GestCfgDrawMode(IConfigWithDependencies* a_pConfig)
{
	CComBSTR cCFGID_GESTUREMODETYPE(CFGID_GESTUREMODETYPE);
	a_pConfig->ItemIns1ofN(cCFGID_GESTUREMODETYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_GESTUREMODETYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_GESTUREMODETYPE_DESC), CConfigValue(CFGVAL_GMT_BLEND), NULL);
	a_pConfig->ItemOptionAdd(cCFGID_GESTUREMODETYPE, CConfigValue(CFGVAL_GMT_BLEND), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_BLENDMODE_NAME), 0, NULL);
	a_pConfig->ItemOptionAdd(cCFGID_GESTUREMODETYPE, CConfigValue(CFGVAL_GMT_RASTER), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_RASTERMODE_NAME), 0, NULL);
	a_pConfig->ItemOptionAdd(cCFGID_GESTUREMODETYPE, CConfigValue(CFGVAL_GMT_COORDS), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COORDSMODE_NAME), 0, NULL);
	TConfigOptionCondition tCond;
	tCond.bstrID = cCFGID_GESTUREMODETYPE;
	tCond.eConditionType = ECOCEqual;
	tCond.tValue.eTypeID = ECVTInteger;
	// blending mode
	tCond.tValue.iVal = CFGVAL_GMT_BLEND;
	CComBSTR cCFGID_CT_BLENDMODE(CFGID_GESTUREBLENDMODE);
	a_pConfig->ItemIns1ofN(cCFGID_CT_BLENDMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_BLENDMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_BLENDMODE_DESC), CConfigValue(LONG(EBMDrawOver)), NULL);
	a_pConfig->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMDrawOver)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDOVER_NAME), 1, &tCond);
	a_pConfig->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMReplace)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDREPLACE_NAME), 1, &tCond);
	a_pConfig->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMDrawUnder)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDUNDER_NAME), 1, &tCond);
	// rasterization mode
	tCond.tValue.iVal = CFGVAL_GMT_RASTER;
	CComBSTR cCFGID_CT_RASTERMODE(CFGID_GESTURERASTERMODE);
	a_pConfig->ItemIns1ofN(cCFGID_CT_RASTERMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_RASTERMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_RASTERMODE_DESC), CConfigValue(LONG(ERMSmooth)), NULL);
	a_pConfig->ItemOptionAdd(cCFGID_CT_RASTERMODE, CConfigValue(LONG(ERMSmooth)), _SharedStringTable.GetStringAuto(IDS_MENU_RASTERSMOOTH_NAME), 1, &tCond);
	a_pConfig->ItemOptionAdd(cCFGID_CT_RASTERMODE, CConfigValue(LONG(ERMBinary)), _SharedStringTable.GetStringAuto(IDS_MENU_RASTERBINARY_NAME), 1, &tCond);
	// coordinates mode
	tCond.tValue.iVal = CFGVAL_GMT_COORDS;
	CComBSTR cCFGID_CT_COORDSMODE(CFGID_GESTURECOORDSMODE);
	a_pConfig->ItemIns1ofN(cCFGID_CT_COORDSMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COORDSMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COORDSMODE_DESC), CConfigValue(LONG(ECMFloatingPoint)), NULL);
	a_pConfig->ItemOptionAdd(cCFGID_CT_COORDSMODE, CConfigValue(LONG(ECMFloatingPoint)), _SharedStringTable.GetStringAuto(IDS_MENU_COORDSFLOAT_NAME), 1, &tCond);
	a_pConfig->ItemOptionAdd(cCFGID_CT_COORDSMODE, CConfigValue(LONG(ECMIntegral)), _SharedStringTable.GetStringAuto(IDS_MENU_COORDSINT_NAME), 1, &tCond);
}

inline void GestCfgOutline(IConfigWithDependencies* a_pConfig)
{
	a_pConfig->ItemInsSimple(CComBSTR(CFGID_GESTUREOUTLINEDDELTA), _SharedStringTable.GetStringAuto(IDS_CFGID_GESTUREOUTLINEDDELTA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_GESTUREOUTLINEDDELTA_DESC), CConfigValue(1.0f), NULL, 0, NULL);
}

extern __declspec(selectany) SGestureOperation const g_aGestureOperations[] =
{
	{IDS_GOID_UNDO, {0xe2338bc9, 0x2091, 0x4f7f, {0x85, 0xde, 0xbd, 0xa5, 0x12, 0x3f, 0x5b, 0x9f}}, &CDesignerViewRasterEdit::GestureUndo, NULL },
	{IDS_GOID_REDO, {0x80f0f964, 0x6a19, 0x4952, {0xbd, 0xd9, 0x2d, 0xc4, 0xa1, 0x09, 0x54, 0x90}}, &CDesignerViewRasterEdit::GestureRedo, NULL },
	{IDS_GOID_APPLY, {0xac6edf28, 0x16c7, 0x48b5, {0xba, 0x0d, 0x00, 0x39, 0x04, 0xef, 0x78, 0x22}}, &CDesignerViewRasterEdit::GestureApply, NULL },
	{IDS_GOID_DRAWMODE, {0xcd079b01, 0xe63f, 0x419f, {0x8d, 0x54, 0xf8, 0x48, 0x6e, 0x9b, 0x07, 0xc7}}, &CDesignerViewRasterEdit::GestureDrawMode, &GestCfgDrawMode },
	{IDS_GOID_OUTLINE, {0x299e9b6d, 0x8c10, 0x4fdb, {0x85, 0xc0, 0x3b, 0x7f, 0xcf, 0x55, 0x95, 0x69}}, &CDesignerViewRasterEdit::GestureOutline, &GestCfgOutline },
	{IDS_GOID_SWITCHTOOL, {0x5442bd9a, 0xe682, 0x420d, {0xb2, 0xf4, 0x42, 0xca, 0xc3, 0xc2, 0xea, 0x6e}}, &CDesignerViewRasterEdit::GestureSwitchTool, &GestCfgToolOrStyle<CFGID_GESTURESWITCHTOOLID, IDS_CFGID_CT_TOOLID_NAME, IDS_CFGID_CT_TOOLID_DESC> },
	{IDS_GOID_SWITCHSTYLE, {0xa9d285ec, 0xb8de, 0x41a1, {0xbb, 0x73, 0xd4, 0x20, 0xd2, 0x1c, 0x33, 0xba}}, &CDesignerViewRasterEdit::GestureSwitchStyle, &GestCfgToolOrStyle<CFGID_GESTURESWITCHSTYLEID, IDS_CFGID_CT_STYLEID_NAME, IDS_CFGID_CT_STYLEID_DESC> },
	{IDS_GOID_AUTOZOOM, {0x9c8d8a7d, 0x4184, 0x4849, {0xa4, 0x3b, 0x82, 0x09, 0xec, 0x9a, 0x3d, 0x2a}}, &CDesignerViewRasterEdit::GestureAutoZoom, NULL },
	{IDS_GOID_SWAPCOLORS, {0x4921fe4e, 0x9b70, 0x4615, {0x8d, 0x23, 0x41, 0x55, 0x5b, 0xe4, 0x8a, 0x4f}}, &CDesignerViewRasterEdit::GestureSwapColors, NULL },
};


class ATL_NO_VTABLE CGestureOperationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationManager
{
public:
	CGestureOperationManager()
	{
	}
	void Init(IOperationManager* a_pManager, CDesignerViewRasterEdit* a_pView, bool a_bDeactivate = true)
	{
		m_bDeactivate = a_bDeactivate;
		m_pManager = a_pManager;
		m_pView = a_pView;
	}

BEGIN_COM_MAP(CGestureOperationManager)
	COM_INTERFACE_ENTRY(IOperationManager)
	COM_INTERFACE_ENTRY(ILateConfigCreator)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// ILateConfigCreator methods
public:
	STDMETHOD(CreateConfig)(TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		return CreateConfigEx(this, a_ptControllerID, a_ppConfig);
	}

	// IOperationManager methods
public:
	STDMETHOD(CreateConfigEx)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptControllerID, IConfig** a_ppConfig)
	{
		try
		{
			*a_ppConfig = NULL;
			for (SGestureOperation const* p = g_aGestureOperations; p < g_aGestureOperations+itemsof(g_aGestureOperations); ++p)
			{
				if (IsEqualGUID(p->tID, a_ptControllerID->guidVal))
				{
					if (p->pfnConfig)
					{
						CComPtr<IConfigWithDependencies> pTmp;
						RWCoCreateInstance(pTmp, __uuidof(ConfigWithDependencies));
						p->pfnConfig(pTmp);
						pTmp->Finalize(NULL);
						*a_ppConfig = pTmp.Detach();
					}
					return S_OK;
				}
			}
			return m_pManager->CreateConfigEx(a_pOverrideForItem, a_ptControllerID, a_ppConfig);
		}
		catch (...)
		{
			return a_ppConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	STDMETHOD(ItemGetCount)(ULONG* a_pnCount)
	{
		try
		{
			ULONG nCount = 0;
			HRESULT hRes = m_pManager->ItemGetCount(&nCount);
			*a_pnCount = nCount+itemsof(g_aGestureOperations);
			return hRes;
		}
		catch (...)
		{
			return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ItemIDGetDefault)(TConfigValue* a_ptDefaultOpID)
	{
		return m_pManager->ItemIDGetDefault(a_ptDefaultOpID);
	}
	STDMETHOD(ItemIDGet)(IOperationManager* a_pOverrideForItem, ULONG a_nIndex, TConfigValue* a_ptOperationID, ILocalizedString** a_ppName)
	{
		try
		{
			if (a_nIndex < itemsof(g_aGestureOperations))
			{
				*a_ppName = NULL;
				a_ptOperationID->eTypeID = ECVTGUID;
				a_ptOperationID->guidVal = g_aGestureOperations[a_nIndex].tID;
				*a_ppName = _SharedStringTable.GetString(g_aGestureOperations[a_nIndex].nIDS);
				return S_OK;
			}
			else
			{
				return m_pManager->ItemIDGet(a_pOverrideForItem ? a_pOverrideForItem : this, a_nIndex-itemsof(g_aGestureOperations), a_ptOperationID, a_ppName);
			}
		}
		catch (...)
		{
			return a_ppName == NULL || a_ptOperationID == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(InsertIntoConfigAs)(IOperationManager* a_pOverrideForItem, IConfigWithDependencies* a_pConfig, BSTR a_bstrID, ILocalizedString* a_pItemName, ILocalizedString* a_pItemDesc, ULONG a_nItemConditions, TConfigOptionCondition const* a_aItemConditions)
	{
		try
		{
			CComPtr<ISubConfigSwitchLate> pSubCfg;
			RWCoCreateInstance(pSubCfg, __uuidof(SubConfigSwitchLate));
			HRESULT hRes = pSubCfg->Init(a_pOverrideForItem ? a_pOverrideForItem : this);
			if (FAILED(hRes)) return hRes;
			CConfigValue cDefault;
			hRes = m_pManager->ItemIDGetDefault(&cDefault);
			if (FAILED(hRes)) return hRes;
			hRes = a_pConfig->ItemIns1ofN(a_bstrID, a_pItemName, a_pItemDesc, cDefault, pSubCfg);
			if (FAILED(hRes)) return hRes;

			ULONG nCount = 0;
			hRes = ItemGetCount(&nCount);
			if (FAILED(hRes))
				return hRes;

			for (ULONG i = 0; i != nCount; ++i)
			{
				CComPtr<ILocalizedString> pStr;
				CConfigValue cVal;
				ItemIDGet(a_pOverrideForItem, i, &cVal, &pStr);
				a_pConfig->ItemOptionAdd(a_bstrID, cVal, pStr, a_nItemConditions, a_aItemConditions);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			for (SGestureOperation const* p = g_aGestureOperations; p < g_aGestureOperations+itemsof(g_aGestureOperations); ++p)
			{
				if (IsEqualGUID(p->tID, a_ptOperationID->guidVal))
					return m_pView ? S_OK : S_FALSE;
			}
			return m_pManager->CanActivate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pOverrideForItem, IDocument* a_pDocument, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			for (SGestureOperation const* p = g_aGestureOperations; p < g_aGestureOperations+itemsof(g_aGestureOperations); ++p)
			{
				if (IsEqualGUID(p->tID, a_ptOperationID->guidVal))
				{
					return m_pView ? ((*m_pView).*(p->pMemFun))(a_pConfig) : E_FAIL;
				}
			}
			if (m_pView && m_bDeactivate)
				m_pView->DeactivateAll(FALSE);
			CUndoBlock cUndo(a_pDocument, CMultiLanguageString::GetAuto(L"[0409]Gesture[0405]Gesto"));
			return m_pManager->Activate(a_pOverrideForItem ? a_pOverrideForItem : this, a_pDocument, a_ptOperationID, a_pConfig, a_pStates, a_hParent, a_tLocaleID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Visit)(IOperationManager* a_pOverrideForItem, TConfigValue const* a_ptOperationID, IConfig* a_pConfig, IPlugInVisitor* a_pVisitor)
	{
		return m_pManager->Visit(a_pOverrideForItem ? a_pOverrideForItem : this, a_ptOperationID, a_pConfig, a_pVisitor);
	}

private:
	CComPtr<IOperationManager> m_pManager;
	CDesignerViewRasterEdit* m_pView;
	bool m_bDeactivate;
};

