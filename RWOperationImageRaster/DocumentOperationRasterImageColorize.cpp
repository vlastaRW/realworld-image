// DocumentOperationRasterImageColorize.cpp : Implementation of CDocumentOperationRasterImageColorize

#include "stdafx.h"
#include "DocumentOperationRasterImageColorize.h"
#include <SharedStringTable.h>
#include "PixelLevelTask.h"


const OLECHAR CFGID_CLRZ_MODE[] = L"Mode";
const LONG CFGVAL_CLMD_HLS = 0;
const LONG CFGVAL_CLMD_HSV = 1;
const OLECHAR CFGID_CLRZ_HUE[] = L"Hue";
const OLECHAR CFGID_CLRZ_SATURATION[] = L"Saturation";

#include "ConfigGUIColorize.h"

static float hls_value(float n1, float n2, float h)
{
	h += 360.0f;
	float hue = h - 360.0f*(int)(h/360.0f);

	if (hue < 60.0f)
		return n1 + ( n2 - n1 ) * hue / 60.0f;
	else if (hue < 180.0f)
		return n2;
	else if (hue < 240.0f)
		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
	else
		return n1;
}


// CDocumentOperationRasterImageColorize

STDMETHODIMP CDocumentOperationRasterImageColorize::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGECOLORIZE_NAME);
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

STDMETHODIMP CDocumentOperationRasterImageColorize::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Colorize[0405]Obarvit");
			return S_OK;
		}
		CConfigValue cSat;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_SATURATION), &cSat);
		if (cSat.operator float() == 0.0f)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Desaturate[0405]Desaturovat");
			return S_OK;
		}
		CConfigValue cHue;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_HUE), &cHue);
		CColorBaseHLSA cClr;
		cClr.SetHLS(cHue, 0.5f, cSat);
		CComPtr<INamedColors> pNC;
		RWCoCreateInstance(pNC, __uuidof(NamedColors));
		CComPtr<ILocalizedString> pColorName;
		if (pNC) pNC->ColorToName(0xff000000|cClr.GetRGB(), &pColorName);
		if (pColorName == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Colorize[0405]Obarvit");
			return S_OK;
		}
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Colorize - %s[0405]Obarvit - %s"), pColorName);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

struct SColorizeRenderer : IStockIcons::ILayerReceiver
{
	SColorizeRenderer(IStockIcons::ILayerReceiver* base) : base(*base) {}
	bool operator()(IRLayer const& layer, IRTarget const* target = NULL)
	{
		IRLayer l = layer;

		if (l.material->type == EIRMFill)
		{
			IRFill f(convert(reinterpret_cast<IRFill const*>(layer.material)->color));
			l.material = &f;
			return base(l, target);
		}
		else if (l.material->type == EIRMFillWithInternalOutline)
		{
			IROutlinedFill const* s = reinterpret_cast<IROutlinedFill const*>(layer.material);
			if (s->inside->type == EIRMFill)
			{
				IRFill f(convert(reinterpret_cast<IRFill const*>(s->inside)->color));
				IROutlinedFill of(&f, s->outline, s->widthRelative, s->widthAbsolute);
				l.material = &of;
				return base(l, target);
			}
		}
		return base(layer, target);
	}
	DWORD convert(DWORD color)
	{
		int r = GetRValue(color);
		int g = GetGValue(color);
		int b = GetBValue(color);
		int rgbmax = r>g ? r : g;
		int rgbmin = rgbmax^r^g;
		rgbmax = rgbmax > b ? rgbmax : b;
		rgbmin = rgbmin < b ? rgbmin : b;
		int i = (rgbmax + rgbmin) >> 1;

		float fHue = 0.0f;
		float fSat = 0.7f;
		float m2 = i + (i < 128 ? i*fSat : fSat*255 - i*fSat);
		float m1 = 2.0f * i - m2;
		return (0xff000000&color)|RGB(hls_value(m1, m2, fHue-120.0f),
				hls_value(m1, m2, fHue),
				hls_value(m1, m2, fHue+120.0f));
	}
	IStockIcons::ILayerReceiver& base;
};

HICON CDocumentOperationRasterImageColorize::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	SColorizeRenderer cGSRenderer(&cRenderer);
	pSI->GetLayers(ESIPicture, cGSRenderer);
	return cRenderer.get();
}

//HICON CDocumentOperationRasterImageColorize::GetDefaultIcon(ULONG a_nSize)
//{
//	IRPolyPoint const sky[] = { {0, 32}, {256, 32}, {256, 208}, {0, 208} };
//	IRPolyPoint const hills[] = { {64, 128}, {128, 176}, {192, 96}, {256, 176}, {256, 224}, {0, 224}, {0, 176} };
//	IRPathPoint const sun[] =
//	{
//		{128, 80, 0, -17.6731, 0, 17.6731},
//		{96, 48, -17.6731, 0, 17.6731, 0},
//		{64, 80, 0, 17.6731, 0, -17.6731},
//		{96, 112, 17.6731, 0, -17.6731, 0},
//	};
//	CComPtr<IStockIcons> pSI;
//	RWCoCreateInstance(pSI, __uuidof(StockIcons));
//	IRFill fillSky(0xfff2dab5);//67b5eb);
//	IRFill fillHills(0xffe2ac59);//71eb67);
//	IRFill fillSun(0xfffcf5ea);//ebe867);
//	IROutlinedFill matSky(&fillSky, pSI->GetMaterial(ESMContrast));
//	IROutlinedFill matHills(&fillHills, matSky.outline);
//	IROutlinedFill matSun(&fillSun, matSky.outline);
//	static IRGridItem const tGridX[] = {{EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
//	static IRGridItem const tGridY[] = {{EGIFInteger, 32.0f}, {EGIFInteger, 224.0f}};
//	IRCanvas canvas = {0, 0, 256, 256, itemsof(tGridX), itemsof(tGridY), tGridX, tGridY};
//	CIconRendererReceiver cRenderer(a_nSize);
//	cRenderer(&canvas, itemsof(sky), sky, &matSky);
//	cRenderer(&canvas, itemsof(hills), hills, &matHills);
//	cRenderer(&canvas, itemsof(sun), sun, &matSun);
//	return cRenderer.get();
//
//	//CComPtr<IIconRenderer> pIR;
//	//RWCoCreateInstance(pIR, __uuidof(IconRenderer));
//	//IRPathPoint const path0[] =
//	//{
//	//	{99.0944, 7.59946, 0, 0, 0, 0},
//	//	{227.788, 136.293, 0, 0, 0, 0},
//	//	{108.287, 231.752, -28.9914, 21.9203, 0, 0},
//	//	{3.635, 127.1, 0, 0, -21.9203, 28.9914},
//	//};
//	//IRPathPoint const path1[] =
//	//{
//	//	{227.788, 136.293, 24.0416, -11.3137, -13.4511, 6.32993},
//	//	{189.604, 45.7832, -25.4558, -25.4558, 25.4559, 25.4558},
//	//	{99.0944, 7.59946, -6.32993, 13.4511, 11.3137, -24.0416},
//	//	{145.056, 90.3309, 42.4264, 42.4264, -42.4264, -42.4264},
//	//};
//	//IRPathPoint const path2[] =
//	//{
//	//	{99.8015, 105.887, 14.1421, 21.2132, 0, 0},
//	//	{178.997, 169.527, 0, 0, -24.7487, -9.1924},
//	//	{132.328, 207.711, -17.6777, 2.12131, 0, 0},
//	//	{60.2035, 151.142, 0, 0, 8.48529, 18.3848},
//	//};
//	//IRPathPoint const path3[] =
//	//{
//	//	{223, 154, -24, 24, 24, 24},
//	//	{223, 248, 44, 0, -44, 0},
//	//};
//	//IRPath const shape0 = {itemsof(path0), path0};
//	//IRPath const shape1 = {itemsof(path1), path1};
//	//IRPath const shape2 = {itemsof(path2), path2};
//	//IRPath const shape3 = {itemsof(path3), path3};
//	//IRCanvas const canvas0 = {0, 0, 256, 256, 0, 0, NULL, NULL};
//	//IRFillWithInternalOutline white(0xff000000|GetSysColor(COLOR_WINDOW), 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 256/20.0f);
//	//IRFillWithInternalOutline color1(0xffd39a56, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 256/20.0f);
//	//IRFillWithInternalOutline color2(0xffed9191, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 256/20.0f);
//	//IRLayer const layers[] =
//	//{
//	//	{&canvas0, 0, 1, NULL, &shape0, &color1},
//	//	{&canvas0, 0, 1, NULL, &shape1, &white},
//	//	{&canvas0, 0, 1, NULL, &shape2, &white},
//	//	{&canvas0, 0, 1, NULL, &shape3, &color2},
//	//};
//	//*a_phIcon = pIR->CreateIcon(a_nSize, itemsof(layers), layers);
//}

STDMETHODIMP CDocumentOperationRasterImageColorize::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR bstrMode(CFGID_CLRZ_MODE);
		pCfgInit->ItemIns1ofN(bstrMode, CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), NULL, CConfigValue(CFGVAL_CLMD_HLS), NULL);
		pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_CLMD_HLS), CMultiLanguageString::GetAuto(L"[0409]Replace hue and saturation[0405]Nahradit barvu a saturaci"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrMode, CConfigValue(CFGVAL_CLMD_HSV), CMultiLanguageString::GetAuto(L"[0409]Replace hue, preserve saturation[0405]Nahradit barvu, zachovat saturaci"), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CLRZ_HUE), _SharedStringTable.GetStringAuto(IDS_CFGID_CLRZ_HUE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CLRZ_HUE_DESC), CConfigValue(0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CLRZ_SATURATION), _SharedStringTable.GetStringAuto(IDS_CFGID_CLRZ_SATURATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CLRZ_SATURATION_DESC), CConfigValue(1.0f), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageColorize, CConfigGUIColorize>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageColorize::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
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

//static float hls_value_r(float n1, float n2, float h)
//{
//	h += 120.0f;
//	float hue = h - 360.0f*(int)(h/360.0f);
//
//	if (hue < 60.0f)
//		return n1 + ( n2 - n1 ) * hue / 60.0f;
//	else if (hue < 180.0f)
//		return n2;
//	else if (hue < 240.0f)
//		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
//	else
//		return n1;
//}

//struct CColorizeOp
//{
//	BYTE const* bR;
//	BYTE const* bG;
//	BYTE const* bB;
//	bool bGrey;
//	void Process(TRasterImagePixel* pD, TRasterImagePixel const* pS, size_t const s, size_t const n)
//	{
//		for (TRasterImagePixel* const pE = pD+n; pD != pE; ++pD, pS+=s)
//		{
//			if (pS->bA == 0) continue;
//			int const rgbmax = pS->bR>pS->bG ? (pS->bR>pS->bB ? pS->bR : pS->bB) : (pS->bG>pS->bB ? pS->bG : pS->bB);
//			int const rgbmin = pS->bR<pS->bG ? (pS->bR<pS->bB ? pS->bR : pS->bB) : (pS->bG<pS->bB ? pS->bG : pS->bB);
//			int l = (rgbmax + rgbmin) >> 1;
//			if (bGrey)
//			{
//				TRasterImagePixel const t =
//				{
//					l,
//					l,
//					l,
//					pS->bA,
//				};
//				*pD = t;
//			}
//			else
//			{
//				TRasterImagePixel const t =
//				{
//					bB[l],
//					bG[l],
//					bR[l],
//					pS->bA,
//				};
//				*pD = t;
//			}
//		}
//	}
//};

struct CColorizeOp
{
	DWORD const* aRGB;
	bool bGrey;
	LONGLONG* pTotalA;
	void Process(TPixelChannel* __restrict pD, TPixelChannel const* __restrict pS, size_t const s, size_t const n)
	{
		//memset(pD, -1, n*sizeof*pS);return;
		//memcpy(pD, pS, n*sizeof*pS);return;
        bool const bGrey = this->bGrey;
        DWORD const* pT = this->aRGB;
		ULONGLONG totalA = 0;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
            DWORD nA = *reinterpret_cast<DWORD const*>(pS)&0xff000000;
			totalA += nA;
			//if (nA == 0)
			//{
			//	*reinterpret_cast<DWORD*>(pD) = 0;
			//	continue;
			//}
			//if (pS->bA == 0) continue;
			int rgbmax = pS->bR>pS->bG ? pS->bR : pS->bG;
            int rgbmin = rgbmax^pS->bR^pS->bG;
            rgbmax = rgbmax > pS->bB ? rgbmax : pS->bB;
			rgbmin = rgbmin < pS->bB ? rgbmin : pS->bB;
			int l = (rgbmax + rgbmin) >> 1;
			//if (bGrey)
			{
                //*reinterpret_cast<int*>(pD) = l|(l<<8)|(l<<16)|nA;
            //    *reinterpret_cast<DWORD*>(pD) = (l*0x10101)|nA;
				// TRasterImagePixel const t =
				// {
				// 	(BYTE)l,
				// 	(BYTE)l,
				// 	(BYTE)l,
				// 	pS->bA,
				// };
				// *pD = t;
			}
			//else
			{
                //*reinterpret_cast<int*>(pD) = bB[l]|(int(bG[l])<<8)|(int(bR[l])<<16)|nA;
                *reinterpret_cast<DWORD*>(pD) = aRGB[l]|nA;
				// TRasterImagePixel const t =
				// {
				// 	bB[l],
				// 	bG[l],
				// 	bR[l],
				// 	pS->bA,
				// };
				// *pD = t;
			}
		}
		totalA>>=24;
		InterlockedAdd64(pTotalA, totalA);
	}
};

struct CColorizeKeepSatOp
{
	DWORD const* aRGB;
	LONGLONG* pTotalA;
	void Process(TPixelChannel* __restrict pD, TPixelChannel const* __restrict pS, size_t const s, size_t const n)
	{
        DWORD const* pT = this->aRGB;
		ULONGLONG totalA = 0;
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
            DWORD nA = *reinterpret_cast<DWORD const*>(pS)&0xff000000;
			totalA += nA;
			//if (nA == 0)
			//{
			//	*reinterpret_cast<DWORD*>(pD) = 0;
			//	continue;
			//}
			//if (pS->bA == 0) continue;
			int rgbmax = pS->bR>pS->bG ? pS->bR : pS->bG;
            int rgbmin = rgbmax^pS->bR^pS->bG;
            rgbmax = rgbmax > pS->bB ? rgbmax : pS->bB;
			rgbmin = rgbmin < pS->bB ? rgbmin : pS->bB;
			unsigned int const nL = rgbmax + rgbmin;
			unsigned int const rgbdelta = rgbmax - rgbmin;
			int l = nL >> 1;
			if (rgbdelta)
			{
			unsigned int const nS = nL <= 0xff ? (rgbdelta<<8) / nL : (rgbdelta<<8) / (510 - nL);
			{
                //*reinterpret_cast<int*>(pD) = bB[l]|(int(bG[l])<<8)|(int(bR[l])<<16)|nA;
				ULONG nRGB = aRGB[l];
				ULONG nR = nRGB&0xff;
				ULONG nG = (nRGB&0xff00)>>8;
				ULONG nB = (nRGB&0xff0000)>>16;
				nR = (nR*nS + l*(0x100-nS))>>8;
				nG = (nG*nS + l*(0x100-nS))&0xff00;
				nB = ((nB*nS + l*(0x100-nS))<<8)&0xff0000;
                *reinterpret_cast<DWORD*>(pD) = nR|nG|nB|nA;
				// TRasterImagePixel const t =
				// {
				// 	bB[l],
				// 	bG[l],
				// 	bR[l],
				// 	pS->bA,
				// };
				// *pD = t;
			}
			}
			else
			{
                *reinterpret_cast<DWORD*>(pD) = (0x10101*l)|nA;
			}
		}
		totalA>>=24;
		InterlockedAdd64(pTotalA, totalA);
	}
};

STDMETHODIMP CDocumentOperationRasterImageColorize::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_NOINTERFACE;

		TImageSize tSize = {1, 1};
		TImagePoint tOrig = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
		TRasterImageRect tR = {tOrig, {tOrig.nX+tSize.nX, tOrig.nY+tSize.nY}};
		TPixelChannel  tDefault;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		CConfigValue cMode;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_MODE), &cMode);
		CConfigValue cHue;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_HUE), &cHue);
		CConfigValue cSaturation;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CLRZ_SATURATION), &cSaturation);
		float fSat = cSaturation.operator float() < 0.0f ? 0.0f : (cSaturation.operator float() > 1.0f ? 1.0f : cSaturation.operator float());
		DWORD aRGB[256];
		for (int i = 0; i < 256; ++i)
		{
			float m2 = i + (i < 128 ? i*fSat : fSat*255 - i*fSat);
			float m1 = 2.0f * i - m2;
			aRGB[i] = RGB(hls_value(m1, m2, cHue.operator float()-120.0f),
				hls_value(m1, m2, cHue.operator float()),
				hls_value(m1, m2, cHue.operator float()+120.0f));
		}
		{
			// compute new default for layer
			int rgbmax = tDefault.bR>tDefault.bG ? tDefault.bR : tDefault.bG;
            int rgbmin = rgbmax^tDefault.bR^tDefault.bG;
            rgbmax = rgbmax > tDefault.bB ? rgbmax : tDefault.bB;
			rgbmin = rgbmin < tDefault.bB ? rgbmin : tDefault.bB;
			int l = (rgbmax + rgbmin) >> 1;
			tDefault.n = aRGB[l]|(tDefault.n&0xff000000);
		}

		CAutoPixelBuffer cBuf(pRI, tSize);

		CComPtr<IThreadPool> pThPool;
		if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
			RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

		ULONGLONG totalA = 0;
		if (cMode.operator LONG() == CFGVAL_CLMD_HSV)
		{
			CColorizeKeepSatOp cOp;
			cOp.aRGB = aRGB;
			cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
			CComObjectStackEx<CPixelLevelTask<CColorizeKeepSatOp> > cXform;
			cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}
		else
		{
			CColorizeOp cOp;
			cOp.bGrey = fSat < 1e-4f;
			cOp.aRGB = aRGB;
			cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
			CComObjectStackEx<CPixelLevelTask<CColorizeOp> > cXform;
			cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		return cBuf.Replace(tR.tTL, &tR.tTL, &tSize, &totalA, tDefault);
		//return pRI->BufferReplace(/*EICIRGBA,*/ tR.tTL, tSize, &tR.tTL, &tSize, NULL, pSrc, pDeleter);
		//return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

