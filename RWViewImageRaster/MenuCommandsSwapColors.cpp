// MenuCommandsSwapColors.cpp : Implementation of CMenuCommandsSwapColors

#include "stdafx.h"
#include "MenuCommandsSwapColors.h"
#include <SharedStringTable.h>

static OLECHAR const CFGID_SYNCIDCOLOR1[] = L"Color1SyncID";
static OLECHAR const CFGID_SYNCIDCOLOR2[] = L"Color2SyncID";

// CMenuCommandsSwapColors

STDMETHODIMP CMenuCommandsSwapColors::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_SWAPCOLORS);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSwapColors::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(ConfigWithDependencies));
		pTmp->ItemInsSimple(CComBSTR(CFGID_SYNCIDCOLOR1), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_COLOR1SYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_COLOR1SYNC_DESC), CConfigValue(L"COLOR1"), NULL, 0, NULL);
		pTmp->ItemInsSimple(CComBSTR(CFGID_SYNCIDCOLOR2), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_COLOR2SYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_COLOR2SYNC_DESC), CConfigValue(L"COLOR2"), NULL, 0, NULL);
		pTmp->Finalize(NULL);
		*a_ppDefaultConfig = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSwapColors::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cColor1;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SYNCIDCOLOR1), &cColor1);
		CConfigValue cColor2;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SYNCIDCOLOR2), &cColor2);

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuItem>* p = NULL;
			CComObject<CDocumentMenuItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cColor1, cColor2);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}
