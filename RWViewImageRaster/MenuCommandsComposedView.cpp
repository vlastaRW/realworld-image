// MenuCommandsComposedView.cpp : Implementation of CMenuCommandsComposedView

#include "stdafx.h"
#include "MenuCommandsComposedView.h"
#include <SharedStringTable.h>


// CMenuCommandsComposedView

STDMETHODIMP CMenuCommandsComposedView::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_COMPOSEDVIEW);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsComposedView::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

//extern GUID const IconIDGrid8 = {0x52c47bd6, 0x6f74, 0x4a37, {0xa0, 0x3e, 0x1c, 0xb6, 0x3b, 0xbb, 0x2e, 0x51}};
__declspec(selectany) extern const GUID TIconIDShowComposed = {0x7098ab84, 0xfe8b, 0x4b06, {0xbe, 0x42, 0xaa, 0x5b, 0x63, 0x97, 0x13, 0x3c}};
__declspec(selectany) extern const GUID TIconIDShowTransparent = {0x908d3a36, 0x4ea1, 0x4dd9, {0x85, 0x6f, 0x39, 0x57, 0x89, 0x58, 0x90, 0x79}};
__declspec(selectany) extern const GUID TIconIDShowLayer = {0x7bab8f26, 0xfec5, 0x48ca, {0xa5, 0x52, 0x56, 0x31, 0x87, 0x39, 0x7c, 0x34}};


STDMETHODIMP CMenuCommandsComposedView::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWLAYER, IDS_MD_SHOWLAYER, &TIconIDShowLayer> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWLAYER, IDS_MD_SHOWLAYER, &TIconIDShowLayer> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView, EICActiveLayer);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWTRANSPARENT, IDS_MD_SHOWTRANSPARENT, &TIconIDShowTransparent> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWTRANSPARENT, IDS_MD_SHOWTRANSPARENT, &TIconIDShowTransparent> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView, EICEmphasizedLayer);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWCOMPOSED, IDS_MD_COMPOSEDVIEW, &TIconIDShowComposed> >* p = NULL;
			CComObject<CDocumentMenuCommand<IDS_MN_SHOWCOMPOSED, IDS_MD_COMPOSEDVIEW, &TIconIDShowComposed> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView, EICFinalImage);
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

