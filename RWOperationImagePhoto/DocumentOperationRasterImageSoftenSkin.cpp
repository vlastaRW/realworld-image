// DocumentOperationRasterImageSoftenSkin.cpp : Implementation of CDocumentOperationRasterImageSoftenSkin

#include "stdafx.h"
#include "DocumentOperationRasterImageSoftenSkin.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_SS_COLOR[] = L"Color";
const OLECHAR CFGID_SS_STRENGTH[] = L"Strength";

// CDocumentOperationRasterImageSoftenSkin

#include "ConfigGUISoftenSkin.h"
#include <WTL_ColorArea.h>

STDMETHODIMP CDocumentOperationRasterImageSoftenSkin::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGE_SOFTENSKIN_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageSoftenSkin::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SS_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_SS_COLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SS_COLOR_DESC), CConfigValue((LONG)0xa2acdc), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_SS_STRENGTH), _SharedStringTable.GetStringAuto(IDS_CFGID_SS_STRENGTH_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SS_STRENGTH_DESC), CConfigValue(50L), NULL, CConfigValue(0L), CConfigValue(100L), CConfigValue(1L), 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageSoftenSkin, CConfigGUISoftenSkin>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageSoftenSkin::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

STDMETHODIMP CDocumentOperationRasterImageSoftenSkin::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		HRESULT hRes = a_pDocument->WriteLock();
		if (FAILED(hRes))
			return hRes;

		CConfigValue cValColor, cValStrength;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SS_COLOR), &cValColor);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SS_STRENGTH), &cValStrength);

		int nColor, nStrength;
		nColor = cValColor.operator LONG();
		nStrength = cValStrength.operator LONG();
		if(nStrength == 0)
		{
			a_pDocument->WriteUnlock();
			return hRes;
		}

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[tSize.nX * tSize.nY]);
		pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX * tSize.nY, pPixels, NULL, EIRIAccurate);

		LONG nRadius = 1 + nStrength * tSize.nX / 10000.0f;

		CColorBaseHLSA cColor;
		CColorBaseHLSA cColor2;
		for(ULONG x=0; x<tSize.nX; ++x)
			for(ULONG y=0; y<tSize.nY; ++y)
			{
				DWORD dwColor = pPixels[x<tSize.nX + y].bB + (pPixels[x<tSize.nX + y].bG << 8) + (pPixels[x<tSize.nX + y].bR << 16);
				cColor.SetBGR(dwColor);
				float fColorDiv=0.0f, fR=0.0f, fG=0.0f, fB=0.0f;
				float fHueDiv=0.0f, fHueDist=0.0f;
				for(LONG xx=x-nRadius; xx<=LONG(x+nRadius); ++xx)
					for(LONG yy=y-nRadius; yy>=0 && yy<LONG(tSize.nY) && yy<=LONG(y+nRadius); ++yy)
					{
						if(xx < 0 || xx >= LONG(tSize.nX) || yy < 0 || yy >= LONG(tSize.nY))
							continue;
						float fDist = (nRadius - sqrtf((xx-x)*(xx-x)+(yy-y)*(yy-y)))/nRadius;
						if(fDist <= 0.0f)
							continue;
						fHueDiv += 1.0f;
						fColorDiv += 1;//fDist;
						fR += pPixels[xx*tSize.nX + yy].bR;
						fG += pPixels[xx*tSize.nX + yy].bG;
						fB += pPixels[xx*tSize.nX + yy].bB;
						DWORD dwColor2 = pPixels[xx*tSize.nX + yy].bB + (pPixels[xx*tSize.nX + yy].bG << 8) + (pPixels[xx*tSize.nX + yy].bR << 16);
						cColor2.SetBGR(dwColor2);

						int nHueDist = abs(cColor2.GetH() - cColor.GetH());
						if(nHueDist>180)
							nHueDist = 360-nHueDist;
						fHueDist += nHueDist / 360.0f;
					}
				if(fHueDiv < 1.0f)
					continue;
				fR /= fColorDiv;
				fG /= fColorDiv;
				fB /= fColorDiv;
				fHueDist /= fHueDiv;
				if(fHueDist > 0.3f)
					continue;
				pPixels[x*tSize.nX + y].bR = pPixels[x*tSize.nX + y].bR * fHueDist + fR * (1.0f - fHueDist);
				pPixels[x*tSize.nX + y].bG = pPixels[x*tSize.nX + y].bG * fHueDist + fG * (1.0f - fHueDist);
				pPixels[x*tSize.nX + y].bB = pPixels[x*tSize.nX + y].bB * fHueDist + fB * (1.0f - fHueDist);
			}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX * tSize.nY, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
