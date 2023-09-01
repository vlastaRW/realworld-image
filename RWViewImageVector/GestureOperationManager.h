
#pragma once

#include <MultiLanguageString.h>

struct SGestureOperation
{
	wchar_t const* pszName;
	GUID tID;
	HRESULT (CDesignerViewVectorImageEditor::*pMemFun)(IConfig* a_pCfg);
	void (*pfnConfig)(IConfigWithDependencies* a_pConfig);
};

//extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHTOOLID[] = L"ToolID";
//extern __declspec(selectany) OLECHAR const CFGID_GESTURESWITCHSTYLEID[] = L"StyleID";
//extern __declspec(selectany) OLECHAR const CFGID_GESTUREOUTLINEDDELTA[] = L"OutlineDelta";
//
//extern __declspec(selectany) OLECHAR const GESTURESWITCHTOOLID_NAME[] = L"[0409]Tool ID[0405]ID nástroje";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHTOOLID_DESC[] = L"[0409]Identifier of the active draw tool.[0405]Identifikátor activního kreslicího nástroje.";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHSTYLEID_NAME[] = L"[0409]Style ID[0405]ID stylu";
//extern __declspec(selectany) OLECHAR const GESTURESWITCHSTYLEID_DESC[] = L"[0409]Identifier of the active fill style.[0405]Identifikátor aktivního stylu vyplňování.";
//
//template<OLECHAR const* t_pszID, OLECHAR const* t_pszName, OLECHAR const* t_pszDesc>
//inline void GestCfgToolOrStyle(IConfigWithDependencies* a_pConfig)
//{
//	a_pConfig->ItemInsSimple(CComBSTR(t_pszID), CMultiLanguageString::GetAuto(t_pszName), CMultiLanguageString::GetAuto(t_pszDesc), CConfigValue(L""), NULL, 0, NULL);
//}


extern __declspec(selectany) SGestureOperation const g_aGestureOperations[] =
{
	{L"[0409]Gestures - Undo[0405]Gesta - zpět", {0xe2338bc9, 0x2091, 0x4f7f, {0x85, 0xde, 0xbd, 0xa5, 0x12, 0x3f, 0x5b, 0x9f}}, &CDesignerViewVectorImageEditor::GestureUndo, NULL },
	{L"[0409]Gestures - Redo[0405]Gesta - vpřed", {0x80f0f964, 0x6a19, 0x4952, {0xbd, 0xd9, 0x2d, 0xc4, 0xa1, 0x09, 0x54, 0x90}}, &CDesignerViewVectorImageEditor::GestureRedo, NULL },
	{L"[0409]Gestures - Apply[0405]Gesta - dokončit kreslení", {0xac6edf28, 0x16c7, 0x48b5, {0xba, 0x0d, 0x00, 0x39, 0x04, 0xef, 0x78, 0x22}}, &CDesignerViewVectorImageEditor::GestureApply, NULL },
	//{L"[0409]Gestures - Draw Mode[0405]Gesta - způsob kreslení", {0xcd079b01, 0xe63f, 0x419f, {0x8d, 0x54, 0xf8, 0x48, 0x6e, 0x9b, 0x07, 0xc7}}, &CDesignerViewVectorImageEditor::GestureDrawMode, &GestCfgDrawMode },
	//{L"[0409]Gestures - Outline[0405]Gesta - obrys", {0x299e9b6d, 0x8c10, 0x4fdb, {0x85, 0xc0, 0x3b, 0x7f, 0xcf, 0x55, 0x95, 0x69}}, &CDesignerViewVectorImageEditor::GestureOutline, &GestCfgOutline },
	//{L"[0409]Gestures - Switch Tool[0405]Gesta - změnit nástroj", {0x5442bd9a, 0xe682, 0x420d, {0xb2, 0xf4, 0x42, 0xca, 0xc3, 0xc2, 0xea, 0x6e}}, &CDesignerViewVectorImageEditor::GestureSwitchTool, &GestCfgToolOrStyle<CFGID_GESTURESWITCHTOOLID, GESTURESWITCHTOOLID_NAME, GESTURESWITCHTOOLID_DESC> },
	//{L"[0409]Gestures - Fill Style[0405]Gesta - styl vyplňování", {0xa9d285ec, 0xb8de, 0x41a1, {0xbb, 0x73, 0xd4, 0x20, 0xd2, 0x1c, 0x33, 0xba}}, &CDesignerViewVectorImageEditor::GestureSwitchStyle, &GestCfgToolOrStyle<CFGID_GESTURESWITCHSTYLEID, GESTURESWITCHSTYLEID_NAME, GESTURESWITCHSTYLEID_DESC> },
	{L"[0409]Gestures - Automatic Zoom[0405]Gesta - automatické zvětšení", {0x9c8d8a7d, 0x4184, 0x4849, {0xa4, 0x3b, 0x82, 0x09, 0xec, 0x9a, 0x3d, 0x2a}}, &CDesignerViewVectorImageEditor::GestureAutoZoom, NULL },
	//{L"[0409]Gestures - Swap Colors[0405]Gesta - prohodit barvy", {0x4921fe4e, 0x9b70, 0x4615, {0x8d, 0x23, 0x41, 0x55, 0x5b, 0xe4, 0x8a, 0x4f}}, &CDesignerViewVectorImageEditor::GestureSwapColors, NULL },
};


class ATL_NO_VTABLE CGestureOperationManager : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationManager
{
public:
	CGestureOperationManager()
	{
	}
	void Init(IOperationManager* a_pManager, CDesignerViewVectorImageEditor* a_pView, bool a_bDeactivate = true)
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
				*a_ppName = new CMultiLanguageString(g_aGestureOperations[a_nIndex].pszName);
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
	CDesignerViewVectorImageEditor* m_pView;
	bool m_bDeactivate;
};

