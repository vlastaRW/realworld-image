// DissolveFilter.cpp : Implementation of CDissolveFilter

#include "stdafx.h"
#include "DissolveFilter.h"
#include <MultiLanguageString.h>
#include <SimpleLocalizedString.h>
#include <PrintfLocalizedString.h>
#include <RWDocumentImageRaster.h>


// CDissolveFilter

STDMETHODIMP CDissolveFilter::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	// Using multi-languge string format: http://www.rw-designer.com/localized-strings
	// Feel free to use just plain English strings.
	*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Dissolve[0405]Rastrový obrázek - rozpuštění");
	return S_OK;
}

STDMETHODIMP CDissolveFilter::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	*a_ppDefaultConfig = NULL;
	CComPtr<IConfigWithDependencies> pCfg;
	RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
	pCfg->ItemInsRanged(
		CComBSTR(L"Amount"),
		CMultiLanguageString::GetAuto(L"[0409]Amount[0405]Množství"),
		CMultiLanguageString::GetAuto(L"[0409]Amount of deleted pixels in percents.[0405]Množství smazaných pixelů v procentech."),
		CConfigValue(20.0f), NULL, // default
		CConfigValue(0.0f), CConfigValue(100.0f), CConfigValue(0.1f), // range and step
		0, NULL);
	pCfg->Finalize(NULL);
	*a_ppDefaultConfig = pCfg.Detach();
	return S_OK;
}

STDMETHODIMP CDissolveFilter::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pContext)
{
	// TODO: check if the document supports the required interface
	return a_pDocument ? S_OK : S_FALSE;
}

STDMETHODIMP CDissolveFilter::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pContext, RWHWND a_hParent, LCID a_tLocaleID)
{
	CComPtr<IDocumentRasterImage> pImg;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pImg));
	if (pImg == NULL)
		return E_NOINTERFACE;

	CConfigValue cAmount;
	a_pConfig->ItemValueGet(CComBSTR(L"Amount"), &cAmount);
	long nLimit = 0x1000000 * (cAmount.operator float()*0.01f);

	TImagePoint tOrig = {0, 0};
	TImageSize tSize = {0, 0};
	pImg->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
	ULONG nPixels = tSize.nX*tSize.nY;
	CAutoVectorPtr<TPixelChannel> pBuffer;
	if (!pBuffer.Allocate(nPixels))
		return E_FAIL;
	pImg->TileGet(EICIRGBA, &tOrig, &tSize, NULL, nPixels, pBuffer, NULL, EIRIAccurate);
	static CPixelChannel const tEmpty;

	enum
	{
		// random number generator
		MODULUS = 2147483647,
		MULTIPLIER = 48271,
		CHECK = 399268537,
		A256 = 22925,
		DEFAULT = 123456789,
	};
	static long const Q = MODULUS / MULTIPLIER;
	static long const R = MODULUS % MULTIPLIER;
	long nRandom = DEFAULT;

	TPixelChannel* p = pBuffer;
	for (TPixelChannel* pEnd = p+nPixels; p < pEnd; ++p)
	{
		// get pseudo random number
        long t = MULTIPLIER * (nRandom % Q) - R * (nRandom / Q);
		nRandom = t > 0 ? t : (t + MODULUS);
		if ((nRandom&0xffffff) < nLimit)
			*p = tEmpty;
	}
	return pImg->TileSet(EICIRGBA, &tOrig, &tSize, NULL, nPixels, pBuffer, TRUE);
}

STDMETHODIMP CDissolveFilter::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		CComPtr<ILocalizedString> pName;
		pName.Attach(new CMultiLanguageString(L"[0409]Dissolve[0405]Rozpuštění"));
		if (a_pConfig == NULL)
		{
			*a_ppName = pName.Detach();
			return S_OK;
		}
		CConfigValue cOpacity;
		a_pConfig->ItemValueGet(CComBSTR(L"Amount"), &cOpacity);
		//if (cOpacity.operator float() == 100.0f)
		//{
		//	*a_ppName = pName.Detach();
		//	return S_OK;
		//}

		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %i%%")));
		pStr->Init(pTempl, pName, static_cast<int>(cOpacity.operator float()+0.5f));
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

// the following methods define a menu item

STDMETHODIMP CDissolveFilter::Name(ILocalizedString** a_ppName)
{
	*a_ppName = new CMultiLanguageString(L"[0409]Dissolve...[0405]Rozpuštění...");
	return S_OK;
}

STDMETHODIMP CDissolveFilter::Description(ILocalizedString** a_ppDesc)
{
	*a_ppDesc = new CMultiLanguageString(L"[0409]Make given percentage of pixels in the image transparent.[0405]Smazat z obrázku dané procento pixelů.");
	return S_OK;
}

STDMETHODIMP CDissolveFilter::IconID(GUID* a_pIconID)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDissolveFilter::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDissolveFilter::Configuration(IConfig* a_pOperationCfg)
{
	// Adjust operation configuration of the menu item.
	// Use GUID of the Display Configuration operation
	// and make the Amount paremeter configurable.

	// "Operation" is a hardcoded ID of the root configuration item
	// and you must set it to the GUID of the main operation.
	CComBSTR bstr1(L"Operation");
	// It is set to __uuidof(DocumentOperationShowConfig), because
	// we want a configuration dialog. If no configuration dialog
	// is necessary, just set it to GUID of your operation.
	CConfigValue cID1(__uuidof(DocumentOperationShowConfig));

	// Since I am using the "Show Configuration" operation
	// http://wiki.rw-designer.com/Operation_Display_Configuration
	// (selected in previous step), I'll have to set its internals.
	// First, the actual operation used (this very "Dissolve" op).
	CComBSTR bstr2(L"Operation\\CfgOperation");
	CConfigValue cID2(__uuidof(DissolveFilter));

	// dialog window caption
	CComBSTR bstr3(L"Operation\\Caption");
	CConfigValue cID3(L"[0409]Configure Dissolve[0405]Konfigurovat rozpuštění");

	// use single slider as configuration
	CComBSTR bstr4(L"Operation\\Parameter");
	CConfigValue cID4(L"Amount");

	// select view for preview
	CComBSTR bstr5(L"Operation\\Preview");
	// http://wiki.rw-designer.com/Image_-_Viewer
	static const GUID tImageViewer = {0x9432886a, 0xd01d, 0x40f1, {0x99, 0x7c, 0x0b, 0x8f, 0x7a, 0x72, 0x2b, 0xfa}};
	CConfigValue cID5(tImageViewer);

	// turn on auto-refresh
	CComBSTR bstr6(L"Operation\\AutoUpdate");
	CConfigValue cID6(true);

	//// TODO: create custom help string
	//CComBSTR bstr7(L"Operation\\HelpTopic");
	//CConfigValue cID7(L"Dissolve deletes given percentage of pixels from image.<br><a href=\" link to a page with documentation \">More information</a>");

	// now, set all those values
	BSTR aIDs[] = {bstr1.m_str, bstr2.m_str, bstr3.m_str, bstr4.m_str, bstr5.m_str, bstr6.m_str};
	TConfigValue aVals[] = {cID1, cID2, cID3, cID4, cID5, cID6};
	return a_pOperationCfg->ItemValuesSet(sizeof(aIDs)/sizeof(*aIDs), aIDs, aVals);
}

