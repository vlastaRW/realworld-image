// MenuCommandsDrawMode.cpp : Implementation of CMenuCommandsDrawMode

#include "stdafx.h"
#include "MenuCommandsDrawMode.h"
#include "ConfigIDsEditToolProperties.h"
#include <SharedStringTable.h>
#include <SimpleLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include <math.h>


static const OLECHAR CFGID_DRAWMODE_TYPE[] = L"DrawModeType";
static const LONG CFGVAL_DRAWMODE_RASTERIZATION = 0;
static const LONG CFGVAL_DRAWMODE_BLENDING = 1;
static const LONG CFGVAL_DRAWMODE_COORDINATES = 2;
static const LONG CFGVAL_DRAWMODE_SHAPEFILL = 3;
static const LONG CFGVAL_DRAWMODE_OUTLINEWIDTH = 4;


// CMenuCommandsDrawMode

STDMETHODIMP CMenuCommandsDrawMode::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_DRAWMODE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsDrawMode::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_DESC), CConfigValue(L"RASTEREDITSTATE"), NULL, 0, NULL);

		CComBSTR cCFGID_DRAWMODE_TYPE(CFGID_DRAWMODE_TYPE);
		pCfg->ItemIns1ofN(cCFGID_DRAWMODE_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_DRAWMODE_TYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DRAWMODE_TYPE_DESC), CConfigValue(CFGVAL_DRAWMODE_RASTERIZATION), NULL);
		pCfg->ItemOptionAdd(cCFGID_DRAWMODE_TYPE, CConfigValue(CFGVAL_DRAWMODE_RASTERIZATION), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DRAWMODE_RASTERIZATION), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_DRAWMODE_TYPE, CConfigValue(CFGVAL_DRAWMODE_BLENDING), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DRAWMODE_BLENDING), 0, NULL);
		pCfg->ItemOptionAdd(cCFGID_DRAWMODE_TYPE, CConfigValue(CFGVAL_DRAWMODE_COORDINATES), _SharedStringTable.GetStringAuto(IDS_CFGVAL_DRAWMODE_COORDINATES), 0, NULL);

		// finalize the initialization of the config
		pCfg->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_MenuCommandsDrawingTools, CConfigGUIEditToolPropertiesDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

extern const GUID IconIDRasterBinary = {0x68abbc47, 0xb51e, 0x4df1, {0xa0, 0x17, 0xf0, 0x2a, 0x86, 0x75, 0x75, 0x7f}};
extern const GUID IconIDRasterSmooth = {0x51f4e148, 0x72d9, 0x4213, {0x8f, 0x65, 0xb9, 0x70, 0x38, 0xb0, 0x6f, 0x50}};
extern const GUID IconIDBlendOver = {0x4c3721a8, 0x5166, 0x4ebf, {0x84, 0x6d, 0xce, 0x92, 0x42, 0x59, 0xb1, 0x73}};
extern const GUID IconIDBlendReplace = {0x6694c0c9, 0x4865, 0x453d, {0xb9, 0x13, 0x17, 0xa4, 0xec, 0x40, 0x3b, 0x8b}};
extern const GUID IconIDBlendUnder = {0x42aa8ef1, 0xbdbb, 0x479f, {0xba, 0x55, 0xa5, 0xcb, 0x57, 0x78, 0xc4, 0xbc}};
extern const GUID IconIDCoordsFloat = {0x01e1914e, 0x9750, 0x4ad1, {0xb0, 0x3d, 0xd9, 0x87, 0xcc, 0xc4, 0x5e, 0xa4}};
extern const GUID IconIDCoordsInt = {0x6509d694, 0x9c1e, 0x4067, {0xb1, 0xfd, 0x2f, 0x1d, 0xa2, 0xe6, 0x58, 0xab}};

STDMETHODIMP CMenuCommandsDrawMode::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* UNREF(a_pView), IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), &cSyncID);
		if (cSyncID.operator BSTR() == NULL || cSyncID.operator BSTR()[0] == L'\0')
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		DWORD dwBlending = 0xffffffff;
		DWORD dwRasterization = 0xffffffff;
		DWORD dwCoordinates = 0xffffffff;
		CComPtr<ISharedStateToolMode> pMode;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pMode));
		if (pMode)
		{
			CComBSTR bstrToolID;
			pMode->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			M_Manager()->SupportedStates(bstrToolID, &dwBlending, &dwRasterization, &dwCoordinates, NULL);
		}

		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DRAWMODE_TYPE), &cType);
		switch (cType.operator LONG())
		{
		case CFGVAL_DRAWMODE_RASTERIZATION:
			{
				CComObject<CDocumentMenuRasterizationMode<IDS_MENU_RASTERBINARY_NAME, IDS_MENU_RASTERBINARY_DESC, &IconIDRasterBinary> >* p = NULL;
				CComObject<CDocumentMenuRasterizationMode<IDS_MENU_RASTERBINARY_NAME, IDS_MENU_RASTERBINARY_DESC, &IconIDRasterBinary> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, ERMBinary, dwRasterization);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuRasterizationMode<IDS_MENU_RASTERSMOOTH_NAME, IDS_MENU_RASTERSMOOTH_DESC, &IconIDRasterSmooth> >* p = NULL;
				CComObject<CDocumentMenuRasterizationMode<IDS_MENU_RASTERSMOOTH_NAME, IDS_MENU_RASTERSMOOTH_DESC, &IconIDRasterSmooth> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, ERMSmooth, dwRasterization);
				pItems->Insert(pTmp);
			}
			break;
		case CFGVAL_DRAWMODE_BLENDING:
			{
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDOVER_NAME, IDS_MENU_BLENDOVER_DESC, &IconIDBlendOver> >* p = NULL;
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDOVER_NAME, IDS_MENU_BLENDOVER_DESC, &IconIDBlendOver> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, EBMDrawOver, dwBlending);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDREPLACE_NAME, IDS_MENU_BLENDREPLACE_DESC, &IconIDBlendReplace> >* p = NULL;
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDREPLACE_NAME, IDS_MENU_BLENDREPLACE_DESC, &IconIDBlendReplace> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, EBMReplace, dwBlending);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDUNDER_NAME, IDS_MENU_BLENDUNDER_DESC, &IconIDBlendUnder> >* p = NULL;
				CComObject<CDocumentMenuBlendingMode<IDS_MENU_BLENDUNDER_NAME, IDS_MENU_BLENDUNDER_DESC, &IconIDBlendUnder> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, EBMDrawUnder, dwBlending);
				pItems->Insert(pTmp);
			}
			break;
		case CFGVAL_DRAWMODE_COORDINATES:
			{
				CComObject<CDocumentMenuCoordinatesMode<IDS_MENU_COORDSFLOAT_NAME, IDS_MENU_COORDSFLOAT_DESC, &IconIDCoordsFloat> >* p = NULL;
				CComObject<CDocumentMenuCoordinatesMode<IDS_MENU_COORDSFLOAT_NAME, IDS_MENU_COORDSFLOAT_DESC, &IconIDCoordsFloat> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, ECMFloatingPoint, dwCoordinates);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuCoordinatesMode<IDS_MENU_COORDSINT_NAME, IDS_MENU_COORDSINT_DESC, &IconIDCoordsInt> >* p = NULL;
				CComObject<CDocumentMenuCoordinatesMode<IDS_MENU_COORDSINT_NAME, IDS_MENU_COORDSINT_DESC, &IconIDCoordsInt> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, ECMIntegral, dwCoordinates);
				pItems->Insert(pTmp);
			}
			break;
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

