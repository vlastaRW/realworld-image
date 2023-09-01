// MenuCommandsCustomTools.cpp : Implementation of CMenuCommandsCustomTools

#include "stdafx.h"
#include "MenuCommandsCustomTools.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <StringParsing.h>
#include "SharedStateToolMode.h"
#include <DocumentMenuCommandImpl.h>
#include <XPGUI.h>
#include <DPIUtils.h>
#include <ContextHelpDlg.h>


// CMenuCommandsCustomTools

STDMETHODIMP CMenuCommandsCustomTools::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_CUSTOMTOOLS);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static const OLECHAR CFGID_CT_TOOLSYNC[] = L"ToolSyncGroup";
static const OLECHAR CFGID_CT_HIDEMANAGER[] = L"HideManager";
static const OLECHAR CFGID_CT_COMMANDS[] = L"Commands";
static const OLECHAR CFGID_CT_COMMANDNAME[] = L"Name";
static const OLECHAR CFGID_CT_COMMANDDESC[] = L"Desc";
static const OLECHAR CFGID_CT_COMMANDICONID[] = L"IconID";
static const OLECHAR CFGID_CT_COMMANDSHORTCUT[] = L"Shortcut";
static const OLECHAR CFGID_CT_USETOOLID[] = L"UseToolID";
static const OLECHAR CFGID_CT_TOOLID[] = L"ToolID";
static const OLECHAR CFGID_CT_TOOLCONFIG[] = L"ToolConfig";
static const OLECHAR CFGID_CT_USESTYLEID[] = L"UseStyleID";
static const OLECHAR CFGID_CT_STYLEID[] = L"StyleID";
static const OLECHAR CFGID_CT_STYLECONFIG[] = L"StyleConfig";
static const OLECHAR CFGID_CT_USEBLENDMODE[] = L"UseBlendMode";
static const OLECHAR CFGID_CT_BLENDMODE[] = L"BlendMode";
static const OLECHAR CFGID_CT_USERASTERMODE[] = L"UseRasterMode";
static const OLECHAR CFGID_CT_RASTERMODE[] = L"RasterMode";
static const OLECHAR CFGID_CT_USECOORDSMODE[] = L"UseCoordsMode";
static const OLECHAR CFGID_CT_COORDSMODE[] = L"CoordsMode";
static const OLECHAR CFGID_CT_USEOUTLINE[] = L"UseOutline";
static const OLECHAR CFGID_CT_OUTENABLED[] = L"OutEnabled";
static const OLECHAR CFGID_CT_OUTWIDTH[] = L"OutWidth";
static const OLECHAR CFGID_CT_OUTCOLOR[] = L"OutColor";
static const OLECHAR CFGID_CT_USEMASK[] = L"UseMask";

#include "ConfigGUICustomTools.h"

class ATL_NO_VTABLE CToolPresetName :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IVectorItemName
{
public:

BEGIN_COM_MAP(CToolPresetName)
	COM_INTERFACE_ENTRY(IVectorItemName)
END_COM_MAP()

	// IVectorItemName methods
public:
	STDMETHOD(Get)(ULONG a_nIndex, IConfig* a_pItemConfig, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			CConfigValue cVal;
			a_pItemConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDNAME), &cVal);
			if (cVal.TypeGet() != ECVTString)
				return E_FAIL;
			*a_ppName = new CMultiLanguageString(cVal.Detach().bstrVal);
			return S_OK;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED: E_POINTER;
		}
	}
};

STDMETHODIMP CMenuCommandsCustomTools::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_CT_TOOLSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLSYNC_DESC), CConfigValue(L"RASTEREDITSTATE"), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_CT_HIDEMANAGER), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_HIDEMANAGER_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_HIDEMANAGER_DESC), CConfigValue(false), NULL, 0, NULL);
		CComPtr<ILocalizedString> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		pCfg->ItemInsSimple(CComBSTR(CFGID_CT_USEMASK), pDummy, pDummy, CConfigValue(12L), NULL, 0, NULL);

		CComPtr<IConfigWithDependencies> pCmd;
		RWCoCreateInstance(pCmd, __uuidof(ConfigWithDependencies));
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_COMMANDNAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDNAME_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDNAME_DESC), CConfigValue(L"<name>"), NULL, 0, NULL);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_COMMANDDESC), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDDESC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDDESC_DESC), CConfigValue(L""), NULL, 0, NULL);
		CComBSTR cCFGID_ICONID(CFGID_CT_COMMANDICONID);
		CComPtr<IConfigItemCustomOptions> pCustIconIDs;
		RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
		if (pCustIconIDs != NULL)
			pCmd->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
		else
			pCmd->ItemInsSimple(cCFGID_ICONID, _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ICONID_DESC), CConfigValue(GUID_NULL), NULL, 0, NULL);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_COMMANDSHORTCUT), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDSHORTCUT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDSHORTCUT_DESC), CConfigValue(0L), NULL, 0, NULL);
		// outline
		CComBSTR cCFGID_CT_USEOUTLINE(CFGID_CT_USEOUTLINE);
		pCmd->ItemInsSimple(cCFGID_CT_USEOUTLINE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USEOUTLINE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(false), NULL, 0, NULL);
		TConfigOptionCondition aConds[1];
		aConds[0].bstrID = cCFGID_CT_USEOUTLINE;
		aConds[0].eConditionType = ECOCEqual;
		aConds[0].tValue.eTypeID = ECVTBool;
		aConds[0].tValue.bVal = true;
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_OUTENABLED), CMultiLanguageString::GetAuto(L"[0409]Enable outline[0405]Povolit obrys"), CMultiLanguageString::GetAuto(L"[0409]Enable outline[0405]Povolit obrys"), CConfigValue(true), NULL, 1, aConds);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_OUTCOLOR), CMultiLanguageString::GetAuto(L"[0409]Outline color[0405]Barva obrysu"), CMultiLanguageString::GetAuto(L"[0409]Outline color[0405]Barva obrysu"), CConfigValue(0.0f, 0.0f, 0.0f, 1.0f), NULL, 1, aConds);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_OUTWIDTH), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_OUTLINE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_OUTLINE_DESC), CConfigValue(1.0f), NULL, 1, aConds);
		// tool
		CComBSTR cCFGID_CT_USETOOLID(CFGID_CT_USETOOLID);
		pCmd->ItemInsSimple(cCFGID_CT_USETOOLID, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USETOOLID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(true), NULL, 0, NULL);
		aConds[0].bstrID = cCFGID_CT_USETOOLID;
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_TOOLID), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_TOOLID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_TOOLID_DESC), CConfigValue(L""), NULL, 1, aConds);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_TOOLCONFIG), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_TOOLCONFIG_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_TOOLCONFIG_DESC), CConfigValue(L""), NULL, 1, aConds);
		// style
		CComBSTR cCFGID_CT_USESTYLEID(CFGID_CT_USESTYLEID);
		pCmd->ItemInsSimple(cCFGID_CT_USESTYLEID, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USESTYLEID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(false), NULL, 0, NULL);
		aConds[0].bstrID = cCFGID_CT_USESTYLEID;
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_STYLEID), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_STYLEID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_STYLEID_DESC), CConfigValue(L""), NULL, 1, aConds);
		pCmd->ItemInsSimple(CComBSTR(CFGID_CT_STYLECONFIG), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_STYLECONFIG_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_STYLECONFIG_DESC), CConfigValue(L""), NULL, 1, aConds);
		// blend mode
		CComBSTR cCFGID_CT_USEBLENDMODE(CFGID_CT_USEBLENDMODE);
		pCmd->ItemInsSimple(cCFGID_CT_USEBLENDMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USEBLENDMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(false), NULL, 0, NULL);
		aConds[0].bstrID = cCFGID_CT_USEBLENDMODE;
		CComBSTR cCFGID_CT_BLENDMODE(CFGID_CT_BLENDMODE);
		pCmd->ItemIns1ofN(cCFGID_CT_BLENDMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_BLENDMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_BLENDMODE_DESC), CConfigValue(LONG(EBMDrawOver)), NULL);
		pCmd->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMDrawOver)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDOVER_NAME), 1, aConds);
		pCmd->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMReplace)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDREPLACE_NAME), 1, aConds);
		pCmd->ItemOptionAdd(cCFGID_CT_BLENDMODE, CConfigValue(LONG(EBMDrawUnder)), _SharedStringTable.GetStringAuto(IDS_MENU_BLENDUNDER_NAME), 1, aConds);
		// rasterization mode
		CComBSTR cCFGID_CT_USERASTERMODE(CFGID_CT_USERASTERMODE);
		pCmd->ItemInsSimple(cCFGID_CT_USERASTERMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USERASTERMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(false), NULL, 0, NULL);
		aConds[0].bstrID = cCFGID_CT_USERASTERMODE;
		CComBSTR cCFGID_CT_RASTERMODE(CFGID_CT_RASTERMODE);
		pCmd->ItemIns1ofN(cCFGID_CT_RASTERMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_RASTERMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_RASTERMODE_DESC), CConfigValue(LONG(ERMSmooth)), NULL);
		pCmd->ItemOptionAdd(cCFGID_CT_RASTERMODE, CConfigValue(LONG(ERMSmooth)), _SharedStringTable.GetStringAuto(IDS_MENU_RASTERSMOOTH_NAME), 1, aConds);
		pCmd->ItemOptionAdd(cCFGID_CT_RASTERMODE, CConfigValue(LONG(ERMBinary)), _SharedStringTable.GetStringAuto(IDS_MENU_RASTERBINARY_NAME), 1, aConds);
		// coordinates mode
		CComBSTR cCFGID_CT_USECOORDSMODE(CFGID_CT_USECOORDSMODE);
		pCmd->ItemInsSimple(cCFGID_CT_USECOORDSMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_USECOORDSMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_ENABLER_DESC), CConfigValue(false), NULL, 0, NULL);
		aConds[0].bstrID = cCFGID_CT_USECOORDSMODE;
		CComBSTR cCFGID_CT_COORDSMODE(CFGID_CT_COORDSMODE);
		pCmd->ItemIns1ofN(cCFGID_CT_COORDSMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COORDSMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COORDSMODE_DESC), CConfigValue(LONG(ECMFloatingPoint)), NULL);
		pCmd->ItemOptionAdd(cCFGID_CT_COORDSMODE, CConfigValue(LONG(ECMFloatingPoint)), _SharedStringTable.GetStringAuto(IDS_MENU_COORDSFLOAT_NAME), 1, aConds);
		pCmd->ItemOptionAdd(cCFGID_CT_COORDSMODE, CConfigValue(LONG(ECMIntegral)), _SharedStringTable.GetStringAuto(IDS_MENU_COORDSINT_NAME), 1, aConds);

		CConfigCustomGUI<&TCUSTOMTOOLCONFIGGUIID, CConfigGUICustomToolDlg>::FinalizeConfig(pCmd);

		CComPtr<ISubConfigVector> pConfigVector;
		RWCoCreateInstance(pConfigVector, __uuidof(SubConfigVector));

		CComObject<CToolPresetName>* pN = NULL;
		CComObject<CToolPresetName>::CreateInstance(&pN);
		CComPtr<IVectorItemName> pName = pN;

		pConfigVector->InitName(pName, pCmd);

		pCfg->ItemInsSimple(CComBSTR(CFGID_CT_COMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CT_COMMANDS_DESC), CConfigValue(0L), pConfigVector.p, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_MenuCommandsCustomTools, CConfigGUICustomToolsDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CDocumentMenuCustomTool :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentMenuCommand
{
public:
	void Init(IRasterImageEditToolsManager* a_pManager, IOperationContext* a_pStates, IDesignerView* a_pView, IConfig* a_pItemConfig, BSTR a_bstrToolState)
	{
		m_pManager = a_pManager;
		m_pStates = a_pStates;
		m_pView = a_pView;
		m_pConfig = a_pItemConfig;
		m_bstrToolState = a_bstrToolState;
	}

BEGIN_COM_MAP(CDocumentMenuCustomTool)
	COM_INTERFACE_ENTRY(IDocumentMenuCommand)
END_COM_MAP()

	// IDocumentMenuCommand methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			CConfigValue cVal;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDNAME), &cVal);
			BSTR bstr = cVal;
			*a_ppText = new CMultiLanguageString(bstr);
			cVal.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Description)(ILocalizedString** a_ppText)
	{
		try
		{
			*a_ppText = NULL;
			CConfigValue cVal;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDDESC), &cVal);
			BSTR bstr = cVal;
			*a_ppText = new CMultiLanguageString(bstr);
			cVal.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		try
		{
			CConfigValue cVal;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDICONID), &cVal);
			*a_pIconID = cVal;
			return S_OK;
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			CConfigValue cVal;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDICONID), &cVal);
			CComPtr<IDesignerFrameIcons> pIcons;
			RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
			return pIcons->GetIcon(cVal, a_nSize, a_phIcon);
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
	{
		try
		{
			CConfigValue cVal;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDSHORTCUT), &cVal);
			if ((cVal.operator LONG()&0xffff) == 0)
				return E_NOTIMPL;
			a_pAccel->wKeyCode = cVal.operator LONG();
			a_pAccel->fVirtFlags = cVal.operator LONG()>>16;
			return S_OK;
		}
		catch (...)
		{
			return a_pAccel ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(State)(EMenuCommandState* a_peState)
	{
		try
		{
			*a_peState = EMCSNormal;
			return S_OK;
		}
		catch (...)
		{
			return a_peState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
	{
		try
		{
			if (m_pView)
				// TODO: make the apply configurable?
				m_pView->DeactivateAll(FALSE);

			CConfigValue cVal;
			EBlendingMode const* pBlendingMode = NULL;
			EBlendingMode eBlendingMode;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USEBLENDMODE), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_BLENDMODE), &cVal);
				eBlendingMode = static_cast<EBlendingMode>(cVal.operator LONG());
				pBlendingMode = &eBlendingMode;
			}
			ERasterizationMode const* pRasterizationMode = NULL;
			ERasterizationMode eRasterizationMode;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USERASTERMODE), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_RASTERMODE), &cVal);
				eRasterizationMode = static_cast<ERasterizationMode>(cVal.operator LONG());
				pRasterizationMode = &eRasterizationMode;
			}
			ECoordinatesMode const* pCoordinatesMode = NULL;
			ECoordinatesMode eCoordinatesMode;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USECOORDSMODE), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COORDSMODE), &cVal);
				eCoordinatesMode = static_cast<ECoordinatesMode>(cVal.operator LONG());
				pCoordinatesMode = &eCoordinatesMode;
			}
			float const* pOutline = NULL;
			float fOutline;
			TColor const* pOutColor = NULL;
			TColor tColor;
			BOOL const* pOutEnable = NULL;
			BOOL bOutEnable;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USEOUTLINE), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_OUTENABLED), &cVal);
				bOutEnable = cVal.operator bool();
				pOutEnable = &bOutEnable;

				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_OUTWIDTH), &cVal);
				fOutline = cVal;
				pOutline = &fOutline;

				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_OUTCOLOR), &cVal);
				tColor.fR = cVal[0];
				tColor.fG = cVal[1];
				tColor.fB = cVal[2];
				tColor.fA = cVal[3];
				pOutColor = &tColor;
			}

			CComBSTR bstrToolID;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USETOOLID), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_TOOLID), &cVal);
				bstrToolID.Attach(cVal.Detach().bstrVal);
			}
			CComBSTR bstrStyleID;
			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USESTYLEID), &cVal);
			if (cVal)
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_STYLEID), &cVal);
				bstrStyleID.Attach(cVal.Detach().bstrVal);
			}

			//float const* pGamma;

			if (bstrToolID || bstrStyleID || pBlendingMode || pRasterizationMode || pCoordinatesMode || pOutline)
			{
				CComPtr<ISharedStateToolMode> pPrev;
				m_pStates->StateGet(m_bstrToolState, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
				CComObject<CSharedStateToolMode>* pNew = NULL;
				CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (S_OK == pNew->Set(bstrToolID, NULL, bstrStyleID, pBlendingMode, pRasterizationMode, pCoordinatesMode, pOutEnable, pOutColor, pOutline, NULL, NULL, pPrev))
					m_pStates->StateSet(m_bstrToolState, pTmp);
			}

			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USETOOLID), &cVal);
			if (cVal && bstrToolID && bstrToolID[0])
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_TOOLCONFIG), &cVal);
				LPCOLESTR psz = cVal;
				GUID tID = GUID_NULL;
				if (GUIDFromString(psz, &tID))
				{
					CComPtr<ISharedState> pState;
					RWCoCreateInstance(pState, tID);
					if (pState)
					{
						pState->FromText(CComBSTR(psz+36));
						CComBSTR bstrT(m_bstrToolState);
						bstrT += bstrToolID;
						m_pStates->StateSet(bstrT, pState);
					}
				}
			}

			m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_USESTYLEID), &cVal);
			if (cVal && bstrStyleID && bstrStyleID[0])
			{
				m_pConfig->ItemValueGet(CComBSTR(CFGID_CT_STYLECONFIG), &cVal);
				LPCOLESTR psz = cVal;
				GUID tID = GUID_NULL;
				if (GUIDFromString(psz, &tID))
				{
					CComPtr<ISharedState> pState;
					RWCoCreateInstance(pState, tID);
					if (pState)
					{
						pState->FromText(CComBSTR(psz+36));
						CComBSTR bstrT(m_bstrToolState);
						bstrT += bstrStyleID;
						m_pStates->StateSet(bstrT, pState);
					}
				}
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComPtr<IOperationContext> m_pStates;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IDesignerView> m_pView;
	CComBSTR m_bstrToolState;
};

extern const GUID TIconIDPresetAdd = {0x59ff7278, 0x4c3b, 0x4083, {0x8a, 0xe3, 0x57, 0x7a, 0xa6, 0x16, 0x3c, 0x21}};

class ATL_NO_VTABLE CDocumentMenuAddCustomTool :
	public CDocumentMenuCommandImpl<CDocumentMenuAddCustomTool, IDS_MN_ADDCUSTOMTOOL, IDS_MN_ADDCUSTOMTOOL, &TIconIDPresetAdd, IDI_PRESET_ADD>
{
public:
	void Init(IRasterImageEditToolsManager* a_pManager, IOperationContext* a_pStates, IConfig* a_pConfig)
	{
		m_pManager = a_pManager;
		m_pStates = a_pStates;
		m_pConfig = a_pConfig;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IConfig> pCopy;
			m_pConfig->DuplicateCreate(&pCopy);

			CComBSTR cCFGID_CT_COMMANDS(CFGID_CT_COMMANDS);
			CConfigValue cCommands;
			pCopy->ItemValueGet(cCFGID_CT_COMMANDS, &cCommands);
			OLECHAR szTmp[64];
			swprintf(szTmp, L"%s\\%08x", CFGID_CT_COMMANDS, cCommands.operator LONG());
			cCommands = cCommands.operator LONG()+1L;
			pCopy->ItemValuesSet(1, &(cCFGID_CT_COMMANDS.m_str), cCommands);
			CComPtr<IConfig> pItem;
			pCopy->SubConfigGet(CComBSTR(szTmp), &pItem);

			CComBSTR cCFGID_CT_USETOOLID(CFGID_CT_USETOOLID);
			CComBSTR cCFGID_CT_USESTYLEID(CFGID_CT_USESTYLEID);
			CComBSTR cCFGID_CT_USEBLENDMODE(CFGID_CT_USEBLENDMODE);
			CComBSTR cCFGID_CT_USERASTERMODE(CFGID_CT_USERASTERMODE);
			CComBSTR cCFGID_CT_USECOORDSMODE(CFGID_CT_USECOORDSMODE);
			CComBSTR cCFGID_CT_USEOUTLINE(CFGID_CT_USEOUTLINE);

			CConfigValue cState;
			pCopy->ItemValueGet(CComBSTR(CFGID_CT_TOOLSYNC), &cState);
			CComPtr<ISharedStateToolMode> pToolState;
			m_pStates->StateGet(cState, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pToolState));
			if (pToolState)
			{
				CComBSTR bstrToolID;
				CComBSTR bstrStyleID;
				EBlendingMode eBlendingMode;
				ERasterizationMode eRasterizationMode;
				ECoordinatesMode eCoordinatesMode;
				float fGamma;
				float fOutlineWidth;
				BOOL bOutlineEnabled = FALSE;
				TColor tOutlineColor;
				if (SUCCEEDED(pToolState->Get(&bstrToolID, NULL, &bstrStyleID, &eBlendingMode, &eRasterizationMode, &eCoordinatesMode, &bOutlineEnabled, &tOutlineColor, &fOutlineWidth, NULL, NULL)))
				{
					CComBSTR cCFGID_CT_TOOLID(CFGID_CT_TOOLID);
					CComBSTR cCFGID_CT_STYLEID(CFGID_CT_STYLEID);
					CComBSTR cCFGID_CT_BLENDMODE(CFGID_CT_BLENDMODE);
					CComBSTR cCFGID_CT_RASTERMODE(CFGID_CT_RASTERMODE);
					CComBSTR cCFGID_CT_COORDSMODE(CFGID_CT_COORDSMODE);
					CComBSTR cCFGID_CT_OUTENABLED(CFGID_CT_OUTENABLED);
					CComBSTR cCFGID_CT_OUTCOLOR(CFGID_CT_OUTCOLOR);
					CComBSTR cCFGID_CT_OUTWIDTH(CFGID_CT_OUTWIDTH);

					BSTR aIDs[14];
					aIDs[0] = cCFGID_CT_USETOOLID;
					aIDs[1] = cCFGID_CT_USESTYLEID;
					aIDs[2] = cCFGID_CT_USEBLENDMODE;
					aIDs[3] = cCFGID_CT_USERASTERMODE;
					aIDs[4] = cCFGID_CT_USECOORDSMODE;
					aIDs[5] = cCFGID_CT_USEOUTLINE;
					TConfigValue aVals[14];
					for (int i = 0; i < 6; ++i)
					{
						aVals[i].eTypeID = ECVTBool;
						aVals[i].bVal = true;
					}
					aIDs[6] = cCFGID_CT_TOOLID;
					aVals[6].eTypeID = ECVTString;
					aVals[6].bstrVal = bstrToolID;
					aIDs[7] = cCFGID_CT_STYLEID;
					aVals[7].eTypeID = ECVTString;
					aVals[7].bstrVal = bstrStyleID;
					aIDs[8] = cCFGID_CT_BLENDMODE;
					aVals[8].eTypeID = ECVTInteger;
					aVals[8].iVal = eBlendingMode;
					aIDs[9] = cCFGID_CT_RASTERMODE;
					aVals[9].eTypeID = ECVTInteger;
					aVals[9].iVal = eRasterizationMode;
					aIDs[10] = cCFGID_CT_COORDSMODE;
					aVals[10].eTypeID = ECVTInteger;
					aVals[10].iVal = eCoordinatesMode;
					aIDs[11] = cCFGID_CT_OUTENABLED;
					aVals[11].eTypeID = ECVTBool;
					aVals[11].bVal = bOutlineEnabled;
					aIDs[12] = cCFGID_CT_OUTCOLOR;
					aVals[12].eTypeID = ECVTVector4;
					aVals[12].vecVal[0] = tOutlineColor.fR;
					aVals[12].vecVal[1] = tOutlineColor.fG;
					aVals[12].vecVal[2] = tOutlineColor.fB;
					aVals[12].vecVal[3] = tOutlineColor.fA;
					aIDs[13] = cCFGID_CT_OUTWIDTH;
					aVals[13].eTypeID = ECVTFloat;
					aVals[13].fVal = fOutlineWidth;
					pItem->ItemValuesSet(14, aIDs, aVals);

					if (bstrToolID.Length())
					{
						CComBSTR bstrState(cState.operator BSTR());
						bstrState += bstrToolID;
						CComPtr<ISharedState> pCfgState;
						m_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pCfgState));
						CComBSTR bstrCfg;
						GUID tID;
						if (pCfgState && SUCCEEDED(pCfgState->CLSIDGet(&tID)) && SUCCEEDED(pCfgState->ToText(&bstrCfg)))
						{
							CComBSTR bstrVal;
							bstrVal.Attach(::SysAllocStringLen(NULL, 36+bstrCfg.Length()));
							StringFromGUID(tID, bstrVal);
							wcscpy(bstrVal.m_str+36, bstrCfg);
							CComBSTR cCFGID_CT_TOOLCONFIG(CFGID_CT_TOOLCONFIG);
							aIDs[0] = cCFGID_CT_TOOLCONFIG;
							aVals[0].eTypeID = ECVTString;
							aVals[0].bstrVal = bstrVal;
							pItem->ItemValuesSet(1, aIDs, aVals);
						}
					}

					if (bstrStyleID.Length())
					{
						CComBSTR bstrState(cState.operator BSTR());
						bstrState += bstrStyleID;
						CComPtr<ISharedState> pCfgState;
						m_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pCfgState));
						CComBSTR bstrCfg;
						GUID tID;
						if (pCfgState && SUCCEEDED(pCfgState->CLSIDGet(&tID)) && SUCCEEDED(pCfgState->ToText(&bstrCfg)))
						{
							CComBSTR bstrVal;
							bstrVal.Attach(::SysAllocStringLen(NULL, 36+bstrCfg.Length()));
							StringFromGUID(tID, bstrVal);
							wcscpy(bstrVal.m_str+36, bstrCfg);
							CComBSTR cCFGID_CT_STYLECONFIG(CFGID_CT_STYLECONFIG);
							aIDs[0] = cCFGID_CT_STYLECONFIG;
							aVals[0].eTypeID = ECVTString;
							aVals[0].bstrVal = bstrVal;
							pItem->ItemValuesSet(1, aIDs, aVals);
						}
					}
				}
			}

			TConfigValue tUseMask;
			CComBSTR cCFGID_CT_USEMASK(CFGID_CT_USEMASK);
			pCopy->ItemValueGet(cCFGID_CT_USEMASK, &tUseMask);
			BSTR aMaskIDs[] = {
				cCFGID_CT_USETOOLID,
				cCFGID_CT_USESTYLEID,
				cCFGID_CT_USEBLENDMODE,
				cCFGID_CT_USERASTERMODE,
				cCFGID_CT_USECOORDSMODE,
				cCFGID_CT_USEOUTLINE,
			};
			TConfigValue aMaskVals[sizeof aMaskIDs/sizeof *aMaskIDs];
			for (size_t i = 0; i < itemsof(aMaskIDs); ++i)
			{
				CConfigValue cVal;
				pItem->ItemValueGet(aMaskIDs[i], &cVal);
				aMaskVals[i].eTypeID = ECVTBool;
				aMaskVals[i].bVal = cVal && (tUseMask.iVal>>i)&1;
			}
			pItem->ItemValuesSet(itemsof(aMaskIDs), aMaskIDs, aMaskVals);

			CDlg cDlg(a_tLocaleID, pItem);
			if (IDOK == cDlg.DoModal(a_hParent))
			{
				for (size_t i = 0; i < itemsof(aMaskIDs); ++i)
				{
					CConfigValue cVal;
					pItem->ItemValueGet(aMaskIDs[i], &cVal);
					tUseMask.iVal = (tUseMask.iVal&~(1<<i)) | (cVal ? 1<<i : 0);
				}
				pCopy->ItemValuesSet(1, &(cCFGID_CT_USEMASK.m_str), &tUseMask);
				CopyConfigValues(m_pConfig, pCopy);
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class CDlg :
		public Win32LangEx::CLangDialogImpl<CDlg>,
		public CDialogResize<CDlg>,
		public CContextHelpDlg<CDlg>
		//public CObserverImpl<CDlg, IConfigObserver, IUnknown*>
	{
	public:
		enum { IDD = IDD_ADDCUSTOMTOOL };

		CDlg(LCID a_tLocaleID, IConfig* a_pCfg) :
			Win32LangEx::CLangDialogImpl<CDlg>(a_tLocaleID),
			m_pConfig(a_pCfg), m_hIcon(NULL),
			CContextHelpDlg<CDlg>(_T("http://www.rw-designer.com/configure-tool-presets"))
		{
		}
		~CDlg()
		{
			if (m_hIcon) DestroyIcon(m_hIcon);
		}

	private:
		BEGIN_MSG_MAP(CDlg)
			CHAIN_MSG_MAP(CContextHelpDlg<CDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnOKOrCancel)
			COMMAND_ID_HANDLER(IDCANCEL, OnOKOrCancel)
			CHAIN_MSG_MAP(CDialogResize<CDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CDlg)
			DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_RT_CONFIG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_CTXHELP_MAP(CDlg)
			//CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
			//CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
			//CTXHELP_CONTROL_RESOURCE(IDC_SHLASSOC_LIST, IDS_HELP_SHLASSOC_LIST)
		END_CTXHELP_MAP()

		LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			m_hIcon = DPIUtils::PrepareIconForCaption((HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PRESET_ADD), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR));
			SetIcon(m_hIcon, FALSE);

			HWND hPlaceHolder = GetDlgItem(IDC_RT_CONFIG);
			RECT rc;
			::GetWindowRect(hPlaceHolder, &rc);
			ScreenToClient(&rc);
			RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
			m_pConfigWnd->Create(m_hWnd, &rc, IDC_RT_CONFIG, m_tLocaleID, TRUE, ECWBMMargin);
			m_pConfigWnd->ConfigSet(m_pConfig, ECPMFull);
			RWHWND h = NULL;
			m_pConfigWnd->Handle(&h);
			if (h)
			{
				::SetWindowPos(h, hPlaceHolder, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				::SetFocus(h);
			}
			::DestroyWindow(hPlaceHolder);

			CenterWindow();

			DlgResize_Init();

			return TRUE;
		}
		LRESULT OnOKOrCancel(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			EndDialog(wID);
			return 0;
		}


	private:
		CComPtr<IConfigWnd> m_pConfigWnd;
		CComPtr<IConfig> m_pConfig;
		HICON m_hIcon;
	};

private:
	CComPtr<IRasterImageEditToolsManager> m_pManager;
	CComPtr<IOperationContext> m_pStates;
	CComPtr<IConfig> m_pConfig;
};

extern const GUID TIconIDPresetManage = {0xc21295f9, 0x751c, 0x4303, {0xa5, 0xb6, 0x7b, 0xef, 0xcd, 0xa5, 0x31, 0xfd}};

class ATL_NO_VTABLE CDocumentMenuManageCustomTools :
	public CDocumentMenuCommandImpl<CDocumentMenuManageCustomTools, IDS_MN_MANAGECUSTOMTOOLS, IDS_MD_MANAGECUSTOMTOOLS, &TIconIDPresetManage, IDI_PRESET_MANAGE>
{
public:
	void Init(IConfig* a_pConfig)
	{
		m_pConfig = a_pConfig;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IConfig> pCopy;
			m_pConfig->DuplicateCreate(&pCopy);
			CDlg cDlg(a_tLocaleID, pCopy);
			if (IDOK == cDlg.DoModal(a_hParent))
			{
				CopyConfigValues(m_pConfig, pCopy);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class CDlg :
		public Win32LangEx::CLangDialogImpl<CDlg>,
		public CDialogResize<CDlg>,
		public CContextHelpDlg<CDlg>
		//public CObserverImpl<CDlg, IConfigObserver, IUnknown*>
	{
	public:
		enum { IDD = IDD_MANAGECUSTOMTOOLS };

		CDlg(LCID a_tLocaleID, IConfig* a_pCfg) :
			Win32LangEx::CLangDialogImpl<CDlg>(a_tLocaleID),
			m_pConfig(a_pCfg), m_hIcon(NULL),
			CContextHelpDlg<CDlg>(_T("http://www.rw-designer.com/configure-tool-presets"))
		{
		}
		~CDlg()
		{
			if (m_hIcon) DestroyIcon(m_hIcon);
		}

	private:
		BEGIN_MSG_MAP(CDlg)
			CHAIN_MSG_MAP(CContextHelpDlg<CDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnOKOrCancel)
			COMMAND_ID_HANDLER(IDCANCEL, OnOKOrCancel)
			CHAIN_MSG_MAP(CDialogResize<CDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CDlg)
			DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_RT_CONFIG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_CTXHELP_MAP(CDlg)
			//CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
			//CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
			//CTXHELP_CONTROL_RESOURCE(IDC_SHLASSOC_LIST, IDS_HELP_SHLASSOC_LIST)
		END_CTXHELP_MAP()

		LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			m_hIcon = DPIUtils::PrepareIconForCaption((HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_PRESET_MANAGE), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR));
			SetIcon(m_hIcon, FALSE);

			HWND hPlaceHolder = GetDlgItem(IDC_RT_CONFIG);
			RECT rc;
			::GetWindowRect(hPlaceHolder, &rc);
			ScreenToClient(&rc);
			RWCoCreateInstance(m_pConfigWnd, __uuidof(AutoConfigWnd));
			m_pConfigWnd->Create(m_hWnd, &rc, IDC_RT_CONFIG, m_tLocaleID, TRUE, ECWBMMargin);
			m_pConfigWnd->ConfigSet(m_pConfig, ECPMFull);
			RWHWND h = NULL;
			m_pConfigWnd->Handle(&h);
			if (h)
			{
				::SetWindowPos(h, hPlaceHolder, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				::SetFocus(h);
			}
			::DestroyWindow(hPlaceHolder);

			CenterWindow();

			DlgResize_Init();

			return TRUE;
		}
		LRESULT OnOKOrCancel(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			EndDialog(wID);
			return 0;
		}


	private:
		CComPtr<IConfigWnd> m_pConfigWnd;
		CComPtr<IConfig> m_pConfig;
		HICON m_hIcon;
	};

private:
	CComPtr<IConfig> m_pConfig;
};

STDMETHODIMP CMenuCommandsCustomTools::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cCommands;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CT_COMMANDS), &cCommands);
		CConfigValue cHideManager;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CT_HIDEMANAGER), &cHideManager);
		if (cCommands.operator LONG() == 0 && cHideManager.operator bool())
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		if (cCommands.operator LONG())
		{
			CConfigValue cToolState;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CT_TOOLSYNC), &cToolState);
			for (LONG i = 0; i < cCommands.operator LONG(); ++i)
			{
				OLECHAR szTmp[64];
				swprintf(szTmp, L"%s\\%08x", CFGID_CT_COMMANDS, i);
				CComPtr<IConfig> pItemCfg;
				a_pConfig->SubConfigGet(CComBSTR(szTmp), &pItemCfg);
				CComObject<CDocumentMenuCustomTool>* p = NULL;
				CComObject<CDocumentMenuCustomTool>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(M_Manager(), a_pStates, a_pView, pItemCfg, cToolState);
				pItems->Insert(pTmp);
			}
		}

		if (cCommands.operator LONG() && !cHideManager.operator bool())
		{
			CComPtr<IDocumentMenuCommand> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
			pItems->Insert(pTmp);
		}

		if (!cHideManager.operator bool())
		{
			{
				CComObject<CDocumentMenuAddCustomTool>* p = NULL;
				CComObject<CDocumentMenuAddCustomTool>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(M_Manager(), a_pStates, a_pConfig);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuManageCustomTools>* p = NULL;
				CComObject<CDocumentMenuManageCustomTools>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pConfig);
				pItems->Insert(pTmp);
			}
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}
