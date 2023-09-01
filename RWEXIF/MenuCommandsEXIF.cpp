// MenuCommandsEXIF.cpp : Implementation of CMenuCommandsEXIF

#include "stdafx.h"
#include "MenuCommandsEXIF.h"
#include <MultiLanguageString.h>


// CMenuCommandsEXIF

STDMETHODIMP CMenuCommandsEXIF::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	if (a_ppOperationName == nullptr)
		return E_POINTER;
	try
	{
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Image - EXIF Tags[0405]Obrázek - EXIF značky");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsEXIF::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		//CComPtr<IConfigWithDependencies> pCfg;
		//RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		//pCfg->Finalize(NULL);
		//*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <RWImaging.h>

extern OLECHAR const g_pszNameRemoveExif[] = L"[0409]Remove EXIF data[0405]Odstranit EXIF data";
extern OLECHAR const g_pszDescRemoveExif[] = L"[0409]Remove the EXIF data block from the current image.[0405]Odstranit datový blok EXIF z aktuálního obrázku.";

class ATL_NO_VTABLE CDocumentMenuRemoveEXIF :
	public CDocumentMenuCommandMLImpl<CDocumentMenuRemoveEXIF, g_pszNameRemoveExif, g_pszDescRemoveExif, NULL, 0>
{
public:
	void Init(IDocument* a_pDocument, IImageMetaData* a_pIMD)
	{
		m_pDocument = a_pDocument;
		m_pIMD = a_pIMD;
	}

	// IDocumentMenuCommand
public:
	EMenuCommandState IntState()
	{
		CComBSTR bstrEXIF(L"EXIF");
		ULONG nSize = 0;
		m_pIMD->GetBlockSize(bstrEXIF, &nSize);
		return nSize == 0 ? EMCSDisabled : EMCSNormal;
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		CComBSTR bstrEXIF(L"EXIF");
		return m_pIMD->SetBlock(bstrEXIF, 0, NULL);
	}

private:
	CComPtr<IDocument> m_pDocument;
	CComPtr<IImageMetaData> m_pIMD;
};

STDMETHODIMP CMenuCommandsEXIF::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* UNREF(a_pView), IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IImageMetaData> pIMD;
		a_pDocument->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD == NULL)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuRemoveEXIF>* p = NULL;
			CComObject<CDocumentMenuRemoveEXIF>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pDocument, pIMD);
			pItems->Insert(pTmp);
		}
		//{
		//	CComObject<CDocumentMenuZoomPopup>* p = NULL;
		//	CComObject<CDocumentMenuZoomPopup>::CreateInstance(&p);
		//	CComPtr<IDocumentMenuCommand> pTmp = p;
		//	p->Init(a_pView);
		//	pItems->Insert(pTmp);
		//}
		//{
		//	CComObject<CDocumentMenuZoomIn>* p = NULL;
		//	CComObject<CDocumentMenuZoomIn>::CreateInstance(&p);
		//	CComPtr<IDocumentMenuCommand> pTmp = p;
		//	p->Init(a_pView);
		//	pItems->Insert(pTmp);
		//}
		//{
		//	CComObject<CDocumentMenuAutoZoom>* p = NULL;
		//	CComObject<CDocumentMenuAutoZoom>::CreateInstance(&p);
		//	CComPtr<IDocumentMenuCommand> pTmp = p;
		//	p->Init(a_pView);
		//	pItems->Insert(pTmp);
		//}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

