// DocumentOperationRasterImageHLS.cpp : Implementation of CDocumentOperationRasterImageHLS

#include "stdafx.h"
#include "DocumentOperationRasterImageHLS.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_HLS_HUE[] = L"Hue";
const OLECHAR CFGID_HLS_SATURATION[] = L"Saturation";
const OLECHAR CFGID_HLS_LIGHTNESS[] = L"Lightness";
const OLECHAR CFGID_HLS_AREA[] = L"Area";
const OLECHAR CFGID_HLS_AREA1[] = L"Area1";
const OLECHAR CFGID_HLS_AREA2[] = L"Area2";
const OLECHAR CFGID_HLS_AREA3[] = L"Area3";
const OLECHAR CFGID_HLS_AREA4[] = L"Area4";

#include "ConfigGUIHLS.h"

// CDocumentOperationRasterImageHLS

STDMETHODIMP CDocumentOperationRasterImageHLS::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEHLS_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
//#include <PrintfLocalizedString.h>
//#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageHLS::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		//if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Adjust HLS[0405]Změnit HLS");
			return S_OK;
		}
		//CConfigValue cSat;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_SATURATION), &cSat);
		//if (cSat.operator float() == 0.0f)
		//{
		//	*a_ppName = new CMultiLanguageString(L"[0409]Desaturate[0405]Desaturovat");
		//	return S_OK;
		//}
		//CConfigValue cHue;
		//a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_HUE), &cHue);
		//CColorBaseHLSA cClr;
		//cClr.SetHLS(cHue, 0.5f, cSat);
		//CComPtr<INamedColors> pNC;
		//RWCoCreateInstance(pNC, __uuidof(NamedColors));
		//CComPtr<ILocalizedString> pColorName;
		//if (pNC) pNC->ColorToName(0xff000000|cClr.GetRGB(), &pColorName);
		//if (pColorName == NULL)
		//{
		//	*a_ppName = new CMultiLanguageString(L"[0409]Colorize[0405]Obarvit");
		//	return S_OK;
		//}
		//CComObject<CPrintfLocalizedString>* pStr = NULL;
		//CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		//CComPtr<ILocalizedString> pTmp = pStr;
		//pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Colorize - %s[0405]Obarvit - %s"), pColorName);
		//*a_ppName = pTmp.Detach();
		//return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageHLS::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_HLS_HUE), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_HUE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_HUE_NAME),  CConfigValue(0L), NULL, CConfigValue(-180L), CConfigValue(180L), CConfigValue(1L), 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_HUE), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_HUE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_HUE_NAME), CConfigValue((LONG)0), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_HLS_SATURATION), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_SATURATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_SATURATION_NAME), CConfigValue(0L), NULL, CConfigValue(-100L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_SATURATION), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_SATURATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_SATURATION_NAME), CConfigValue((LONG)0), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_HLS_LIGHTNESS), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_LIGHTNESS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_LIGHTNESS_NAME), CConfigValue(0L), NULL, CConfigValue(-100L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_LIGHTNESS), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_LIGHTNESS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_LIGHTNESS_NAME), CConfigValue((LONG)0), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_AREA), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA_NAME), CConfigValue(false), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_AREA1), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA1_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA1_NAME), CConfigValue((LONG)150), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_AREA2), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA2_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA2_NAME), CConfigValue((LONG)170), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_AREA3), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA3_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA3_NAME), CConfigValue((LONG)190), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HLS_AREA4), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA4_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HLS_AREA4_NAME), CConfigValue((LONG)210), NULL, 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageHLS, CConfigGUIHLS>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageHLS::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, 1, aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void NormalizeRGB(BYTE& a_bR, BYTE& a_bG, BYTE& a_bB);

STDMETHODIMP CDocumentOperationRasterImageHLS::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		CConfigValue cValHue, cValSaturation, cValLightness, cValArea, cValArea1, cValArea2, cValArea3, cValArea4;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_HUE), &cValHue);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_SATURATION), &cValSaturation);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_LIGHTNESS), &cValLightness);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_AREA), &cValArea);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_AREA1), &cValArea1);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_AREA2), &cValArea2);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_AREA3), &cValArea3);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HLS_AREA4), &cValArea4);

		int nHue, nSat, nLight, bArea, nArea1, nArea2, nArea3, nArea4;
		nHue = cValHue.operator LONG();
		nSat = cValSaturation.operator LONG();
		nLight = cValLightness.operator LONG();
		if(nHue == 0 && nSat == 0 && nLight == 0)
		{
			return E_FAIL;
		}
		bArea = cValArea.operator bool();
		nArea1 = cValArea1.operator LONG();
		nArea2 = cValArea2.operator LONG();
		nArea3 = cValArea3.operator LONG();
		nArea4 = cValArea4.operator LONG();

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL);
		TRasterImageRect tR = {tOrigin, {tOrigin.nX+tSize.nX, tOrigin.nY+tSize.nY}};

		ULONG const nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);
		pRI->TileGet(EICIRGBA, &tR.tTL, &tSize, NULL, nPixels, pPixels, NULL, EIRIAccurate);
		CColorBaseHLSA cColor;
		for(ULONG p=0; p<nPixels; ++p)
		{
			float rIntensity = 1.0f;
			DWORD dwColor = pPixels[p].bB + (pPixels[p].bG << 8) + (pPixels[p].bR << 16);
			cColor.SetBGR(dwColor);
			if(bArea && (rIntensity = GetIntensity(cColor.GetH(), nArea1, nArea2, nArea3, nArea4)) == 0.0f)
				continue;
			cColor.SetH(cColor.GetH() + nHue);
			float rLight = cColor.GetL() + nLight/100.0f;
			if(rLight < 0.0f) rLight = 0.0f;
			if(rLight > 1.0f) rLight = 1.0f;
			cColor.SetL(rLight);
			float rSat = cColor.GetS() + nSat/100.0f;
			if(rSat < 0.0f) rSat = 0.0f;
			if(rSat > 1.0f) rSat = 1.0f;
			cColor.SetS(rSat);
			DWORD dwColorNew = cColor.GetBGR();
			pPixels[p].bB = (dwColorNew & 0xff) * rIntensity + (dwColor & 0xff) * (1.0f-rIntensity);
			pPixels[p].bG = ((dwColorNew >> 8) & 0xff) * rIntensity + ((dwColor >> 8) & 0xff) * (1.0f-rIntensity);
			pPixels[p].bR = ((dwColorNew >> 16) & 0xff) * rIntensity + ((dwColor >> 16) & 0xff) * (1.0f-rIntensity);;
		}
		return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, nPixels, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
