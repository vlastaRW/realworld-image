// MenuCommandsConfigureGestures.cpp : Implementation of CMenuCommandsConfigureGestures

#include "stdafx.h"
#include "MenuCommandsConfigureGestures.h"
#include <SharedStringTable.h>


// CMenuCommandsConfigureGestures

STDMETHODIMP CMenuCommandsConfigureGestures::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_CONFIGUREGESTURES);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsConfigureGestures::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsConfigureGestures::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
		CComPtr<IRasterEditView> pRIG;
		p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
		if (pRIG == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuItem>* p = NULL;
			CComObject<CDocumentMenuItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pRIG);
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
