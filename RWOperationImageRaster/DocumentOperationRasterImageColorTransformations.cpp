// DocumentOperationRasterImageColorTransformations.cpp : Implementation of CDocumentOperationRasterImageColorTransformations

#include "stdafx.h"
#include "DocumentOperationRasterImageColorTransformations.h"
#include <SharedStringTable.h>
#include <math.h>
#include <IconRenderer.h>

const OLECHAR CFGID_BRIGHTNESS[] = L"Brightness";
const OLECHAR CFGID_CONTRAST[] = L"Contrast";
const OLECHAR CFGID_GAMMA[] = L"Gamma";
const OLECHAR CFGID_SATURATION[] = L"Saturation";

#include "ConfigGUIColorTransformations.h"


// CDocumentOperationRasterImageColorTransformations

STDMETHODIMP CDocumentOperationRasterImageColorTransformations::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGECOLORTRANSFORMATIONS_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageColorTransformations::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Adjust exposure[0405]Upravit expozici";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BRIGHTNESS), &cVal);
		LONG nBrightness = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CONTRAST), &cVal);
		LONG nContrast = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_GAMMA), &cVal);
		float fGamma = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SATURATION), &cVal);
		LONG nSaturation = cVal;
		LPCOLESTR pszTempl = NULL;
		int iVal = 0;
		if (nBrightness == 0 && nContrast == 100 && fGamma >= 0.99 && fGamma <= 1.01 && nSaturation == 100)
		{
			pszName = L"[0409]No adjustment[0405]Žádné úpravy";
		}
		else if ((nBrightness < -40 || nBrightness > 40) &&
			(nContrast > 95 && nContrast < 105) &&
			(fGamma > 0.95f && fGamma < 1.05f) &&
			(nSaturation > 95 && nSaturation < 105))
		{
			pszName = nBrightness > 0 ? L"[0409]Increase brightness[0405]Zvýšit jas" : L"[0409]Decrease brightness[0405]Snížit jas";
			pszTempl = nBrightness > 0 ? L"%s (+%i%%)" : L"%s (%i%%)";
			iVal = (nBrightness*100+127)/255;
		}
		else if ((nBrightness > -10 && nBrightness < 10) &&
			(nContrast < 90 || nContrast > 110) &&
			(fGamma > 0.85f && fGamma < 1.15f) &&
			(nSaturation > 95 && nSaturation < 105))
		{
			pszName = nContrast > 100 ? L"[0409]Increase contrast[0405]Zvýšit kontrast" : L"[0409]Decrease contrast[0405]Snížit kontrast";
			pszTempl = nContrast > 100 ? L"%s (+%i%%)" : L"%s (%i%%)";
			iVal = nContrast-100;
		}
		else if ((nBrightness > -10 && nBrightness < 10) &&
			(nContrast > 95 && nContrast < 105) &&
			(fGamma < 0.7f || fGamma > 1.3f) &&
			(nSaturation > 95 && nSaturation < 105))
		{
			pszName = L"[0409]Adjust gamma[0405]Upravit gamu";
		}
		else if ((nBrightness > -10 && nBrightness < 10) &&
			(nContrast > 95 && nContrast < 105) &&
			(fGamma > 0.95f && fGamma < 1.05f) &&
			(nSaturation < 90 || nSaturation > 110))
		{
			pszName = nSaturation > 100 ? L"[0409]Increase saturation[0405]Zvýšit saturaci" : L"[0409]Desaturate[0405]Desaturovat";
			pszTempl = nSaturation > 100 ? L"%s (+%i%%)" : L"%s (%i%%)";
			iVal = nSaturation-100;
		}
		if (pszTempl)
		{
			CComPtr<ILocalizedString> pTemplate;
			pTemplate.Attach(new CSimpleLocalizedString(SysAllocString(pszTempl)));
			CComPtr<ILocalizedString> pArg1;
			pArg1.Attach(new CMultiLanguageString(pszName));
			CComObject<CPrintfLocalizedString>* p = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(pTemplate, pArg1, iVal);
			*a_ppName = pTmp.Detach();
		}
		else
		{
			*a_ppName = new CMultiLanguageString(pszName);
		}
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HICON CDocumentOperationRasterImageColorTransformations::GetDefaultIcon(ULONG a_nSize)
{
	IRPathPoint const white[] =
	{
		{164, 128, 0, -39.7645, 0, 39.7645},
		{128, 56, -39.7645, 0, 39.7645, 0},
		{56, 128, 0, 39.7645, 0, -39.7645},
		{128, 200, 39.7645, 0, -39.7645, 0},
		//{160, 128, 0, -35.3462, 0, 35.3462},
		//{128, 64, -35.3462, 0, 35.3462, 0},
		//{64, 128, 0, 35.3462, 0, -35.3462},
		//{128, 192, 35.3462, 0, -35.3462, 0},
	};
	IRPathPoint const black[] =
	{
		{200, 128, 0, -39.7645, 0, 39.7645},
		{128, 56, 0, 0, 39.7645, 0},
		{128, 200, 39.7645, 0, 0, 0},
		//{192, 128, 0, -35.3462, 0, 35.3462},
		//{128, 64, 0, 0, 35.3462, 0},
		//{128, 192, 35.3462, 0, 0, 0},
	};
	//float r1 = 128;
	//float r2 = 128-48;
	//float d1 = r1*0.707f;
	//float d2 = r2*0.707f;
	//IRPolyPoint const lines[] =
	//{
	//	{128, 128-r1}, {128, 128-r2},
	//	{128-r1, 128}, {128-r2, 128},
	//	{128, 128+r1}, {128, 128+r2},
	//	{128+r1, 128}, {128+r2, 128},
	//	{128-d1, 128-d1}, {128-d2, 128-d2},
	//	{128+d1, 128+d1}, {128+d2, 128+d2},
	//	{128-d1, 128+d1}, {128-d2, 128+d2},
	//	{128+d1, 128-d1}, {128+d2, 128-d2},
	//};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(white), white, pSI->GetMaterial(ESMBackground));//pSI->GetMaterial(ESMScheme1Color3));
	cRenderer(&canvas, itemsof(black), black, pSI->GetMaterial(ESMContrast));
	//IRStroke stroke(pSI->GetSRGBColor(ESMContrast), 1.0f/12.0f);
	//cRenderer(&canvas, 2, lines, &stroke);
	//cRenderer(&canvas, 2, lines+2, &stroke);
	//cRenderer(&canvas, 2, lines+4, &stroke);
	//cRenderer(&canvas, 2, lines+6, &stroke);
	//cRenderer(&canvas, 2, lines+8, &stroke);
	//cRenderer(&canvas, 2, lines+10, &stroke);
	//cRenderer(&canvas, 2, lines+12, &stroke);
	//cRenderer(&canvas, 2, lines+14, &stroke);

	//IRPolyPoint const tri1[] = { {117, 48}, {128, 0}, {139, 48} };
	//IRPolyPoint const tri2[] = { {139, 208}, {128, 256}, {117, 208} };
	//IRPolyPoint const tri3[] = { {208, 117}, {256, 128}, {208, 139} };
	//IRPolyPoint const tri4[] = { {48, 139}, {0, 128}, {48, 117} };
	//IRPolyPoint const tri5[] = { {177, 64}, {219, 37}, {192, 79} };
	//IRPolyPoint const tri6[] = { {79, 192}, {37, 219}, {64, 177} };
	//IRPolyPoint const tri7[] = { {192, 177}, {219, 219}, {177, 192} };
	//IRPolyPoint const tri8[] = { {64, 79}, {37, 37}, {79, 64} };
	IRPolyPoint const tri1[] = { {150, 44}, {181, 0}, {172, 52} };
	IRPolyPoint const tri2[] = { {106, 212}, {75, 256}, {84, 204} };
	IRPolyPoint const tri3[] = { {212, 150}, {256, 181}, {204, 172} };
	IRPolyPoint const tri4[] = { {44, 106}, {0, 75}, {52, 84} };
	IRPolyPoint const tri5[] = { {204, 84}, {256, 75}, {212, 106} };
	IRPolyPoint const tri6[] = { {52, 172}, {0, 181}, {44, 150} };
	IRPolyPoint const tri7[] = { {172, 204}, {181, 256}, {150, 212} };
	IRPolyPoint const tri8[] = { {84, 52}, {75, 0}, {106, 44} };
	cRenderer(&canvas, 3, tri1, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri2, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri3, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri4, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri5, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri6, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri7, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, 3, tri8, pSI->GetMaterial(ESMContrast));
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRasterImageColorTransformations::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_BRIGHTNESS), _SharedStringTable.GetStringAuto(IDS_CFGID_BRIGHTNESS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BRIGHTNESS_DESC), CConfigValue(0L), NULL, CConfigValue(-255L), CConfigValue(255L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_CONTRAST), _SharedStringTable.GetStringAuto(IDS_CFGID_CONTRAST_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CONTRAST_DESC), CConfigValue(100L), NULL, CConfigValue(0L), CConfigValue(200L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_GAMMA), _SharedStringTable.GetStringAuto(IDS_CFGID_GAMMA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_GAMMA_DESC), CConfigValue(1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_SATURATION), _SharedStringTable.GetStringAuto(IDS_CFGID_SATURATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SATURATION_DESC), CConfigValue(100L), NULL, CConfigValue(0L), CConfigValue(200L), CConfigValue(1L), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageColorTransformations, CConfigGUIColorTransformations>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageColorTransformations::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentRasterImage)/*, __uuidof(IRasterImageControl)*/};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <GammaCorrection.h>

struct CColorTransformOp
{
	BYTE const* pLUT;
	void Process(TPixelChannel* __restrict pD, TPixelChannel const* __restrict pS, size_t const s, size_t const n)
	{
		BYTE const* pLUT = this->pLUT;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			ULONG s = pS->n;
			ULONG a = s&0xff000000;
			ULONG r = (s&0xff0000)>>16;
			ULONG g = (s&0xff00)>>8;
			ULONG b = s&0xff;
			pD->n = a|(ULONG(pLUT[r])<<16)|(ULONG(pLUT[g])<<8)|pLUT[b];
		}
	}
};

struct CColorTransformDirectOp
{
	int nContrast;
	int nBrightness;
	float fGamma;
	void Process(TPixelChannel* __restrict pD, TPixelChannel const* __restrict pS, size_t const s, size_t const n)
	{
		int const nContrast = this->nContrast;
		int const nBrightness = this->nBrightness;
		float const fGamma = this->fGamma;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			ULONG s = pS->n;
			ULONG a = s&0xff000000;
			ULONG r = (s&0xff0000)>>16;
			ULONG g = (s&0xff00)>>8;
			ULONG b = s&0xff;
			//float valR = CGammaTables::FromSRGB(r);
			//float valG = CGammaTables::FromSRGB(g);
			//float valB = CGammaTables::FromSRGB(b);
			//float xR = (powf(valR, fGamma)-0.5f) * nContrast / 100.0f + 0.5f + nBrightness/255.0f;
			//float xG = (powf(valG, fGamma)-0.5f) * nContrast / 100.0f + 0.5f + nBrightness/255.0f;
			//float xB = (powf(valB, fGamma)-0.5f) * nContrast / 100.0f + 0.5f + nBrightness/255.0f;
			//ULONG nR = CGammaTables::ToSRGB(xR);
			//ULONG nG = CGammaTables::ToSRGB(xG);
			//ULONG nB = CGammaTables::ToSRGB(xB);
			ULONG nR = min(r+10, 255);
			ULONG nG = min(g+10, 255);
			ULONG nB = min(b+10, 255);
			pD->n = a|(nR<<16)|(nG<<8)|nB;
		}
	}
};

struct CColorTransformWithSaturationOp
{
	LONG const* pLUT;
	LONG fIR, fIG, fIB;
	LONG fAR, fAG, fAB;
	CGammaTables const* pGT;
	void SetSat(LONG nSat)
	{
		float const fSat1 = nSat/100.0f*1024.0f;
		float const fSat2 = 1024.0f-fSat1;
		fIR = fSat2*0.299f+0.5f;
		fIG = fSat2*0.587f+0.5f;
		fIB = fSat2*0.114f+0.5f;
		fAR = fSat1+fSat2*0.299f+0.5f;
		fAG = fSat1+fSat2*0.587f+0.5f;
		fAB = fSat1+fSat2*0.114f+0.5f;
	}
	void Process(TPixelChannel* pD, TPixelChannel const* pS, size_t const s, size_t const n)
	{
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			if (pS->bA == 0)
			{
				pD->n = 0;
				continue;
			}
			LONG const fR = pLUT[pS->bR];
			LONG const fG = pLUT[pS->bG];
			LONG const fB = pLUT[pS->bB];
			LONG fRR = fAR*fR + fIG*fG + fIB*fB;
			if (fRR < 0) fRR = 0; else if (fRR > 0xffff<<10) fRR = 0xffff<<10;
			LONG fGG = fIR*fR + fAG*fG + fIB*fB;
			if (fGG < 0) fGG = 0; else if (fGG > 0xffff<<10) fGG = 0xffff<<10;
			LONG fBB = fIR*fR + fIG*fG + fAB*fB;
			if (fBB < 0) fBB = 0; else if (fBB > 0xffff<<10) fBB = 0xffff<<10;
			TPixelChannel const t =
			{
				pGT->InvGamma(fBB>>10),
				pGT->InvGamma(fGG>>10),
				pGT->InvGamma(fRR>>10),
				pS->bA,
			};
			*pD = t;
		}
	}
};

#include "PixelLevelTask.h"

STDMETHODIMP CDocumentOperationRasterImageColorTransformations::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TImageSize tSize = {1, 1};
		TImagePoint tOrig = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
		TRasterImageRect tR = {tOrig, {tOrig.nX+tSize.nX, tOrig.nY+tSize.nY}};
		TPixelChannel tDefault;
		tDefault.n = 0;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BRIGHTNESS), &cVal);
		LONG const nBrightness = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CONTRAST), &cVal);
		LONG const nContrast = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_GAMMA), &cVal);
		float fGamma = cVal;
		if (fGamma > 1000.0f) fGamma = 1000.0f;
		else if (fGamma < 0.001f) fGamma = 0.001f;
		fGamma = 1.0f/fGamma;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SATURATION), &cVal);
		LONG const nSaturation = cVal;

		CAutoPixelBuffer cDst(pRI, tSize);

		CComPtr<IThreadPool> pThPool;
		if (tSize.nY >= 4 && tSize.nX*tSize.nY > 64*64)
			RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

		if (nSaturation == 100L)
		{
			//CColorTransformDirectOp cOp;
			//cOp.nContrast = nContrast;
			//cOp.nBrightness = nBrightness;
			//cOp.fGamma = fGamma;
			//CComObjectStackEx<CPixelLevelTask<CColorTransformDirectOp> > cXform;
			CColorTransformOp cOp;
			BYTE aXLate[256];
			for (LONG i = 0; i < 256; ++i)
			{
				float val = CGammaTables::FromSRGB(i);
				float x = (powf(val, fGamma)-0.5f) * nContrast / 100.0f + 0.5f + nBrightness/255.0f;
				aXLate[i] = CGammaTables::ToSRGB(x);
			}
			cOp.pLUT = aXLate;
			CComObjectStackEx<CPixelLevelTask<CColorTransformOp> > cXform;
			cXform.Init(pRI, tR, cDst.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}
		else
		{
			CColorTransformWithSaturationOp cOp;
			LONG aXLate[256];
			for (LONG i = 0; i < 256; ++i)
			{
				float val = CGammaTables::FromSRGB(i);
				float x = (powf(val, fGamma)-0.5f) * nContrast / 100.0f + 0.5f + nBrightness/255.0f;
				aXLate[i] = 65535*x;
			}
			cOp.pLUT = aXLate;
			cOp.SetSat(nSaturation);
			CComPtr<IGammaTableCache> pGTC;
			RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
			cOp.pGT = pGTC->GetSRGBTable();
			CComObjectStackEx<CPixelLevelTask<CColorTransformWithSaturationOp> > cXform;
			cXform.Init(pRI, tR, cDst.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		return cDst.Replace(tOrig, &tOrig, &tSize, NULL, tDefault);
		//return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, nPixels, pSrc.m_p, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

