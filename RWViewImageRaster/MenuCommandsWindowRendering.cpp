// MenuCommandsWindowRendering.cpp : Implementation of CMenuCommandsWindowRendering

#include "stdafx.h"
#include "MenuCommandsWindowRendering.h"
#include <SharedStringTable.h>


// CMenuCommandsWindowRendering

STDMETHODIMP CMenuCommandsWindowRendering::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_WINDOWRENDERING);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsWindowRendering::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
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

extern wchar_t STYLE_MENU_CLASSIC[] = L"[0409]Classic style[0405]Klasický styl";
extern wchar_t STYLE_MENU_SUBTLE[] = L"[0409]Subtle style[0405]Jemný styl";
extern wchar_t STYLE_MENU_CLEAN[] = L"[0409]Minimal style[0405]Minimální styl";
extern wchar_t STYLE_MENUDESC[] = L"[0409]Canvas style option affects canvas colors and outline.[0405]Volba vzhledu plátna ovlivňuje barvy plátna a okraje.";

STDMETHODIMP CMenuCommandsWindowRendering::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

/*
		{
			CComObject<CMode<IDS_MN_RENDERNEAREST, IDS_MD_RENDERNEAREST, NULL, 0, EIQNearest> >* p = NULL;
			CComObject<CMode<IDS_MN_RENDERNEAREST, IDS_MD_RENDERNEAREST, NULL, 0, EIQNearest> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CMode<IDS_MN_RENDERLINEAR, IDS_MD_RENDERLINEAR, NULL, 0, EIQLinear> >* p = NULL;
			CComObject<CMode<IDS_MN_RENDERLINEAR, IDS_MD_RENDERLINEAR, NULL, 0, EIQLinear> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CMode<IDS_MN_RENDERCOVERAGE, IDS_MD_RENDERCOVERAGE, NULL, 0, EIQCoverage> >* p = NULL;
			CComObject<CMode<IDS_MN_RENDERCOVERAGE, IDS_MD_RENDERCOVERAGE, NULL, 0, EIQCoverage> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComPtr<IDocumentMenuCommand> pSep;
			RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
			pItems->Insert(pSep);
		}
*/
		{
			CComObject<CStyle<STYLE_MENU_CLASSIC, STYLE_MENUDESC, 0> >* p = NULL;
			CComObject<CStyle<STYLE_MENU_CLASSIC, STYLE_MENUDESC, 0> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CStyle<STYLE_MENU_SUBTLE, STYLE_MENUDESC, 3> >* p = NULL;
			CComObject<CStyle<STYLE_MENU_SUBTLE, STYLE_MENUDESC, 3> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CStyle<STYLE_MENU_CLEAN, STYLE_MENUDESC, 1> >* p = NULL;
			CComObject<CStyle<STYLE_MENU_CLEAN, STYLE_MENUDESC, 1> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComPtr<IDocumentMenuCommand> pSep;
			RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
			pItems->Insert(pSep);
		}
		{
			CComObject<CInvertedPixels>* p = NULL;
			CComObject<CInvertedPixels>::CreateInstance(&p);
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

