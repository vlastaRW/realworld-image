// MenuCommandsGrid.cpp : Implementation of CMenuCommandsGrid

#include "stdafx.h"
#include "MenuCommandsGrid.h"
#include <SharedStringTable.h>


// CMenuCommandsGrid

STDMETHODIMP CMenuCommandsGrid::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_GRID);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsGrid::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

extern GUID const IconIDGridNone = {0x20eaad07, 0x5162, 0x4157, {0x8a, 0xa4, 0x10, 0x8c, 0x79, 0xb5, 0xc7, 0x93}};
extern GUID const IconIDGrid1 = {0xa6fffc2d, 0x3a9d, 0x408c, {0x8a, 0xf6, 0x20, 0xb2, 0x8a, 0x95, 0x30, 0x65}};
extern GUID const IconIDGrid8 = {0x52c47bd6, 0x6f74, 0x4a37, {0xa0, 0x3e, 0x1c, 0xb6, 0x3b, 0xbb, 0x2e, 0x51}};

STDMETHODIMP CMenuCommandsGrid::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID1_NAME, IDS_MENU_GRID1_DESC, &IconIDGridNone, 1> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID1_NAME, IDS_MENU_GRID1_DESC, &IconIDGridNone, 1> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID8_NAME, IDS_MENU_GRID8_DESC, &IconIDGrid1, 8> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID8_NAME, IDS_MENU_GRID8_DESC, &IconIDGrid1, 8> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID0_NAME, IDS_MENU_GRID0_DESC, &IconIDGrid8, 0> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MENU_GRID0_NAME, IDS_MENU_GRID0_DESC, &IconIDGrid8, 0> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
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

