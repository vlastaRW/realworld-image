// DocumentOperationRasterImageVignetting.cpp : Implementation of CDocumentOperationRasterImageVignetting

#include "stdafx.h"
#include "DocumentOperationRasterImageVignetting.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_VIG_AUTO_UNVIG[] = L"AutoUnVig";
const OLECHAR CFGID_VIG_COLOR[] = L"Color";
const OLECHAR CFGID_VIG_CURVE[] = L"Curve";
const OLECHAR CFGID_VIG_INTERPOLATION[] = L"Interpolation";
const LONG CFGVAL_INTERPOLATION_BSPLINE = 0;
const LONG CFGVAL_INTERPOLATION_LINEAR = 1;

#include "ConfigGUIVignetting.h"

// CDocumentOperationRasterImageVignetting

STDMETHODIMP CDocumentOperationRasterImageVignetting::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEVIGNETTING_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRasterImageVignetting::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Vignetting[0405]Vinětace";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cAuto;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VIG_AUTO_UNVIG), &cAuto);
		if (cAuto.operator bool())
			pszName = L"[0409]Auto un-vignette[0405]Automatické odstranění vinětace";
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImageVignetting::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	static IRPathPoint const center[] =
	{
		{183, 128, 0, -30.3757, 0, 30.3757},
		{128, 73, -30.3757, 0, 30.3757, 0},
		{73, 128, 0, 30.3757, 0, -30.3757},
		{128, 183, 30.3757, 0, -30.3757, 0},
	};
	static IRPathPoint const left[] =
	{
		{16, 64, 0, 0, 0, 0},
		{96, 64, -22.0914, 11.0457, 0, 0},
		{56, 128, 0, 24.3005, 0, -24.3005},
		{96, 192, 0, 0, -22.0914, -11.0457},
		{16, 192, 0, 0, 0, 0},
	};
	static IRPathPoint const right[] =
	{
		{240, 192, 0, 0, 0, 0},
		{160, 192, 22.0914, -11.0457, 0, 0},
		{200, 128, 0, -24.3005, 0, 24.3005},
		{160, 64, 0, 0, 22.0914, 11.0457},
		{240, 64, 0, 0, 0, 0},
	};
	static IRCanvas const canvas = {16, 64, 240, 192, 0, 0, NULL, NULL};
	cRenderer(&canvas, itemsof(center), center, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(left), left, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(right), right, pSI->GetMaterial(ESMAltBackground));
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRasterImageVignetting::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_VIG_AUTO_UNVIG), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_AUTO_UNVIG_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_AUTO_UNVIG_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_VIG_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_COLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_COLOR_DESC), CConfigValue((LONG)0x000000), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_VIG_CURVE), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_CURVE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_CURVE_DESC), CConfigValue(_T("0,127.5;255,127.5;")), NULL, 0, NULL);
		CComBSTR bstrCFGID_INTERPOLATION(CFGID_VIG_INTERPOLATION);
		pCfgInit->ItemIns1ofN(bstrCFGID_INTERPOLATION, _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_INTERPOLATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_VIG_INTERPOLATION_DESC), CConfigValue(CFGVAL_INTERPOLATION_BSPLINE), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_INTERPOLATION, CConfigValue(CFGVAL_INTERPOLATION_BSPLINE), _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION_BSPLINE), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_INTERPOLATION, CConfigValue(CFGVAL_INTERPOLATION_LINEAR), _SharedStringTable.GetStringAuto(IDS_CFGID_INTERPOLATION_LINEAR), 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageVignetting, CConfigGUIVignetting>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageVignetting::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

STDMETHODIMP CDocumentOperationRasterImageVignetting::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CWriteLock<IDocument> cLock(a_pDocument);

		CConfigValue cAuto;
		CConfigValue cColor;
		CConfigValue cCurve;
		CConfigValue cInterpolation;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VIG_AUTO_UNVIG), &cAuto);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VIG_COLOR), &cColor);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VIG_CURVE), &cCurve);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_VIG_INTERPOLATION), &cInterpolation);
		DWORD dwColor = cColor.operator LONG();
		BYTE bR = dwColor & 0xff;
		BYTE bG = (dwColor >> 8) & 0xff;
		BYTE bB = (dwColor >> 16) & 0xff;

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[tSize.nX * tSize.nY]);
		pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX * tSize.nY, pPixels, NULL, EIRIAccurate);

		TCurvePoints aCurvePts;
		CCurveControl::ParseCurvePoints(cCurve.operator BSTR(), aCurvePts);
		std::vector<float> aValues;
		int nValues = 1+sqrtf((tSize.nX/2.0f)*(tSize.nX/2.0f) + (tSize.nY/2.0f)*(tSize.nY/2.0f));
		if(cAuto.operator bool())
		{
			LONG x,y, n1=0, n2=0, n3=0;
			float f1=0.0f, f2=0.0f, f3=0.0f;
			for(x=0; x<tSize.nX*0.2f; ++x)
				for(y=0; y<tSize.nY*0.2f; ++y)
				{
					LONG p = tSize.nX * y + x;
					f3 += (pPixels[p].bR + pPixels[p].bG + pPixels[p].bB) / 768.0f;
					++n3;
				}
			f3 /= n3;
			for(x=tSize.nX*0.2f; x<tSize.nX*0.4f; ++x)
				for(y=tSize.nY*0.2f; y<tSize.nY*0.4f; ++y)
				{
					LONG p = tSize.nX * y + x;
					f2 += (pPixels[p].bR + pPixels[p].bG + pPixels[p].bB) / 768.0f;
					++n2;
				}
			f2 /= n2;
			for(x=tSize.nX*0.4f; x<tSize.nX*0.6f; ++x)
				for(y=tSize.nY*0.4f; y<tSize.nY*0.6f; ++y)
				{
					LONG p = tSize.nX * y + x;
					f1 += (pPixels[p].bR + pPixels[p].bG + pPixels[p].bB) / 768.0f;
					++n1;
				}
			f1 /= n1;
			if(f1 < f2)
			{
				f1 = 0.0f;
				f3 -= f2;
				f2 = 0.0f;
			}else
			{
				f2 -= f1;
				f3 -= f1;
				f1 = 0.0f;
			}
			if(f2 > 0) f2 = 0.0f;
			if(f3 > 0) f3 = 0.0f;
			aValues.resize(nValues);
			for(int i=0; i<nValues/2.0f; ++i)
				aValues[i] = f1*((nValues/2.0f-i)/(nValues/2.0f)) + f2*(1.0f-(nValues/2.0f-i)/(nValues/2.0f));
			for(ULONG i=nValues/2.0f; i<aValues.size(); ++i)
				aValues[i] = f2*((nValues-i)/(nValues/2.0f)) + f3*(1.0f-(nValues-i)/(nValues/2.0f));
		}
		else
		{
			GetCurveValues(aCurvePts, aValues, 0.0f, nValues, 256.0f/nValues, cInterpolation.operator LONG() == CFGVAL_INTERPOLATION_BSPLINE);
			for(ULONG i=0; i<aValues.size(); ++i)
				aValues[i] = (aValues[i]-127.5f)/127.5f;
		}

		float fCenterX = (tSize.nX-1)/2.0f;
		float fCenterY = (tSize.nY-1)/2.0f;
		//for(LONG x=0; x<=fCenterX; ++x)
		//	for(LONG y=0; y<=fCenterY; ++y)
		for(ULONG x=0; x<tSize.nX; ++x)
			for(ULONG y=0; y<tSize.nY; ++y)
			{
				float fDistX = fCenterX - x;
				float fDistY = fCenterY - y;
				int nDist = sqrtf(fDistX*fDistX + fDistY*fDistY);
				if(aValues[nDist] == 0.0f)
					continue;
				LONG p = tSize.nX * y + x;
				if(aValues[nDist]<0)
				{
					pPixels[p].bR = (pPixels[p].bR - pPixels[p].bR * aValues[nDist] >= 255) ? 255 : pPixels[p].bR - pPixels[p].bR * aValues[nDist];
					pPixels[p].bG = (pPixels[p].bG - pPixels[p].bG * aValues[nDist] >= 255) ? 255 : pPixels[p].bG - pPixels[p].bG * aValues[nDist];
					pPixels[p].bB = (pPixels[p].bB - pPixels[p].bB * aValues[nDist] >= 255) ? 255 : pPixels[p].bB - pPixels[p].bB * aValues[nDist];
				}else
				{
					pPixels[p].bR = (pPixels[p].bR + bR * aValues[nDist] >= 255) ? 255 : pPixels[p].bR + bR * aValues[nDist];
					pPixels[p].bG = (pPixels[p].bG + bG * aValues[nDist] >= 255) ? 255 : pPixels[p].bG + bG * aValues[nDist];
					pPixels[p].bB = (pPixels[p].bB + bB * aValues[nDist] >= 255) ? 255 : pPixels[p].bB + bB * aValues[nDist];
				}
		}

		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX * tSize.nY, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
