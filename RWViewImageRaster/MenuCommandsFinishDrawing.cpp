// MenuCommandsFinishDrawing.cpp : Implementation of CMenuCommandsFinishDrawing

#include "stdafx.h"
#include "MenuCommandsFinishDrawing.h"
#include <SharedStringTable.h>
#include <IconRenderer.h>


// CMenuCommandsFinishDrawing

STDMETHODIMP CMenuCommandsFinishDrawing::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_FINISHDRAWING);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFinishDrawing::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pViews;
		RWCoCreateInstance(pViews, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, pViews);
		CComPtr<IRasterEditView> pView;
		pViews->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pView));
		if (pView == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuItem>* p = NULL;
			CComObject<CDocumentMenuItem>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(pView);
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

#include <SimpleLocalizedString.h>

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(IDS_MENU_FINISHDRAWING_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(IDS_MENU_FINISHDRAWING_DESC);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::IconID(GUID* a_pIconID)
{
	try
	{
		static GUID const tID = {0xcd15ebbf, 0x5efe, 0x4bbf, {0x9f, 0x4e, 0x9e, 0x86, 0xd7, 0x5a, 0x40, 0xbe}};
		*a_pIconID = tID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		pSI->GetLayers(ESIConfirm, cRenderer, IRTarget(0.9f));
		*a_phIcon = cRenderer.get();
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = static_cast<EMenuCommandState>(S_OK == m_pView->CanApplyChanges() ? EMCSNormal|EMCSShowButtonText : EMCSDisabled|EMCSShowButtonText);
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFinishDrawing::CDocumentMenuItem::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		return m_pView->ApplyChanges();
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

