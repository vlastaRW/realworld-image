// DocumentTransformationToRasterImage.cpp : Implementation of CDocumentTransformationToRasterImage

#include "stdafx.h"
#include "DocumentTransformationToRasterImage.h"

#include <RWDocumentImageRaster.h>
#include <DocumentName.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>


// CDocumentTransformationToRasterImage

STDMETHODIMP CDocumentTransformationToRasterImage::NameGet(ITransformationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_TORASTERIMAGE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_IMAGEFACTORY[] = L"ImageFactory";

STDMETHODIMP CDocumentTransformationToRasterImage::ConfigCreate(ITransformationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_IMAGEFACTORY(CFGID_IMAGEFACTORY);
		pCfgInit->ItemIns1ofN(cCFGID_IMAGEFACTORY, CMultiLanguageString::GetAuto(L"[0409]Image type[0405]Typ obrázku"), CMultiLanguageString::GetAuto(L"[0409]Choosing \"Default\" creates an image defined in the Raster Image options.[0405]Volba\"Výchozí\" vyrvoří obrázek nastavený v možnostech aplikace pro Rastrový obrázek."), CConfigValue(GUID_NULL), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_IMAGEFACTORY, CConfigValue(GUID_NULL), CMultiLanguageString::GetAuto(L"[0409]Default[0405]Výchozí"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_IMAGEFACTORY, CConfigValue(__uuidof(DocumentFactoryRasterImage)), CMultiLanguageString::GetAuto(L"[0409]Raster image[0405]Rastrový obrázek"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_IMAGEFACTORY, CConfigValue(__uuidof(DocumentFactoryLayeredImage)), CMultiLanguageString::GetAuto(L"[0409]Layered image[0405]Vrstvený obrázek"), 0, NULL);

		pCfgInit->Finalize(NULL);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationToRasterImage::CanActivate(ITransformationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		return a_pDocument != NULL ? (SupportsAllFeatures(a_pDocument, 1, &__uuidof(IDocumentImage)) ? S_OK : S_FALSE) : E_POINTER;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentTransformationToRasterImage::Activate(ITransformationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID), BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComPtr<IDocumentImage> pInDoc;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pInDoc));

		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMAGEFACTORY), &cType);
		CComPtr<IDocumentFactoryRasterImage> pImageFact;
		if (!IsEqualGUID(cType, GUID_NULL))
			RWCoCreateInstance(pImageFact, cType.operator const GUID &());
		if (pImageFact == NULL)
		{
			CComPtr<IGlobalConfigManager> pMgr;
			RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
			CComPtr<IConfig> pGlbCfg;
			if (pMgr) pMgr->Config(__uuidof(GlobalConfigDefaultImageFormat), &pGlbCfg);
			CConfigValue cVal;
			if (pGlbCfg) pGlbCfg->ItemValueGet(CComBSTR(CFGID_DIF_BUILDER), &cVal);
			RWCoCreateInstance(pImageFact, cVal);
		}
		if (pImageFact == NULL)
			return E_FAIL;

		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pInDoc, false))
			return E_FAIL;

		if (FAILED(pImageFact->Create(a_bstrPrefix, a_pBase, &cBuffer.tSize, &cBuffer.tResolution, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(cBuffer.tSize.nX, cBuffer.tSize.nY, reinterpret_cast<TPixelChannel const*>(cBuffer.aData.m_p)))))
			return E_FAIL;

		CComQIPtr<IDocument> pOutDoc(a_pBase);
		if (pOutDoc)
		{
			CComPtr<IConfig> pCfg;
			GUID tEncID = GUID_NULL;
			pOutDoc->EncoderGet(&tEncID, &pCfg);
			CComPtr<IDocumentEncoder> pEnc;
			if (!IsEqualGUID(tEncID, GUID_NULL)) RWCoCreateInstance(pEnc, tEncID);
			CComPtr<IDocumentType> pDocType;
			if (pEnc) pEnc->DocumentType(&pDocType);
			CComBSTR bstrExt;
			if (pDocType)
				pDocType->DefaultExtensionGet(&bstrExt);
			if (bstrExt.Length())
				CDocumentName::ChangeExtension(a_pDocument, bstrExt, pOutDoc);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

