// DocumentOperationRasterImageRemoveHue.cpp : Implementation of CDocumentOperationRasterImageRemoveHue

#include "stdafx.h"
#include "DocumentOperationRasterImageRemoveHue.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>
#include <WTL_Curve.h>

const OLECHAR CFGID_HUE[] = L"Hue";
const OLECHAR CFGID_CURVE[] = L"Curve";
const OLECHAR CFGID_X_AXIS[] = L"XAxis";
const LONG CFGVAL_X_AXIS_HUE = 0;
const LONG CFGVAL_X_AXIS_COLOR = 1;
const OLECHAR CFGID_INTERPOLATION[] = L"Interpolation";
const LONG CFGVAL_INTERPOLATION_BSPLINE = 0;
const LONG CFGVAL_INTERPOLATION_LINEAR = 1;

#include "ConfigGUIRemoveHue.h"

// CDocumentOperationRasterImageRemoveHue

STDMETHODIMP CDocumentOperationRasterImageRemoveHue::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEREMOVEHUE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRemoveHue::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_HUE), _SharedStringTable.GetStringAuto(IDS_CFGID_HUE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_HUE_DESC), CConfigValue((LONG)0x000000), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CURVE), _SharedStringTable.GetStringAuto(IDS_CFGID_CURVE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CURVE_DESC), CConfigValue(_T("0,0;255,255;")), NULL, 0, NULL);
		CComBSTR bstrCFGID_X_AXIS(CFGID_X_AXIS);
		pCfgInit->ItemIns1ofN(bstrCFGID_X_AXIS, _SharedStringTable.GetStringAuto(IDS_CFGID_X_AXIS), _SharedStringTable.GetStringAuto(IDS_CFGID_X_AXIS), CConfigValue(CFGVAL_X_AXIS_HUE), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_X_AXIS, CConfigValue(CFGVAL_X_AXIS_HUE), _SharedStringTable.GetStringAuto(IDS_CFGID_X_AXIS_HUE), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_X_AXIS, CConfigValue(CFGVAL_X_AXIS_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_X_AXIS_COLOR), 0, NULL);
		CComBSTR bstrCFGID_INTERPOLATION(CFGID_INTERPOLATION);
		pCfgInit->ItemIns1ofN(bstrCFGID_INTERPOLATION, _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION), _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION), CConfigValue(CFGVAL_INTERPOLATION_BSPLINE), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_INTERPOLATION, CConfigValue(CFGVAL_INTERPOLATION_BSPLINE), _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION_BSPLINE), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_INTERPOLATION, CConfigValue(CFGVAL_INTERPOLATION_LINEAR), _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION_LINEAR), 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageRemoveHue, CConfigGUIRemoveHue>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRemoveHue::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

void NormalizeRGB(BYTE& a_bR, BYTE& a_bG, BYTE& a_bB)
{
	if(a_bR == a_bG && a_bG == a_bB)
	{
		a_bR = 255;
		a_bG = 255;
		a_bB = 255;
	}else if(a_bR >= a_bG && a_bR >= a_bB)
	{
		float f = 255.0f / a_bR;
		a_bR = 255;
		a_bG *= f;
		a_bB *= f;
	}else if(a_bG >= a_bR && a_bG >= a_bB)
	{
		float f = 255.0f / a_bG;
		a_bG = 255;
		a_bR *= f;
		a_bB *= f;
	}else if(a_bB >= a_bG && a_bB >= a_bR)
	{
		float f = 255.0f / a_bB;
		a_bB = 255;
		a_bG *= f;
		a_bR *= f;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRemoveHue::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CWriteLock<IDocument> cLock(a_pDocument);

		CConfigValue cHue;
		CConfigValue cCurve;
		CConfigValue cXAxis;
		CConfigValue cInterpolation;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_HUE), &cHue);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CURVE), &cCurve);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_X_AXIS), &cXAxis);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INTERPOLATION), &cInterpolation);
		DWORD dwHue = cHue.operator LONG();
		BYTE bR = dwHue & 0xff;
		BYTE bG = (dwHue >> 8) & 0xff;
		BYTE bB = (dwHue >> 16) & 0xff;
		if(cXAxis.operator LONG() == CFGVAL_X_AXIS_HUE)
			NormalizeRGB(bR, bG, bB);
		TCurvePoints aCurvePts;
		CCurveControl::ParseCurvePoints(cCurve.operator BSTR(), aCurvePts);
		std::vector<float> aValues;
		GetCurveValues(aCurvePts, aValues, 0.0f, 256, 1.0f, cInterpolation.operator LONG() == CFGVAL_INTERPOLATION_BSPLINE);
		for(std::vector<float>::iterator i = aValues.begin(); i != aValues.end(); i++)
		{
			if(*i < 0.0f) *i=0.0f;
			if(*i > 255.0f) *i=255.0f;
		}

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[tSize.nX * tSize.nY]);
		pRI->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX * tSize.nY, pPixels, NULL, EIRIAccurate);
		for (ULONG p=0; p<tSize.nX * tSize.nY; ++p)
		{
			ULONG nDist;
			if(cXAxis.operator LONG() == CFGVAL_X_AXIS_HUE)
			{
				BYTE bPixelR = pPixels[p].bR;
				BYTE bPixelG = pPixels[p].bG;
				BYTE bPixelB = pPixels[p].bB;
				NormalizeRGB(bPixelR, bPixelG, bPixelB);
				nDist = (abs(bPixelR - bR) + abs(bPixelG - bG) + abs(bPixelB - bB)) / 3;
			}
			else
				nDist = (abs(pPixels[p].bR - bR) + abs(pPixels[p].bG - bG) + abs(pPixels[p].bB - bB)) / 3;
			if(nDist < aValues.size())
				pPixels[p].bA = pPixels[p].bA * (aValues[nDist] / 256.0f);
			if(pPixels[p].bA == 0)
			{
				pPixels[p].bR = 0;
				pPixels[p].bG = 0;
				pPixels[p].bB = 0;
			}
		}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX * tSize.nY, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
