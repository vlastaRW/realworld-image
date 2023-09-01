// DocumentOperationRemoveNoise.cpp : Implementation of CDocumentOperationRemoveNoise

#include "stdafx.h"
#include "DocumentOperationRemoveNoise.h"
#include <SharedStringTable.h>

const OLECHAR CFGID_RN_METHOD[] = L"Method";
const LONG CFGVAL_RNM_BLUR9OF25 = 0;
const LONG CFGVAL_RNM_BLURENERGY = 1;
const OLECHAR CFGID_RN_THRESHOLD[] = L"Threshold";


// CDocumentOperationRemoveNoise

STDMETHODIMP CDocumentOperationRemoveNoise::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_DOCOPREMOVENOISE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRemoveNoise::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Noise removal[0405]Odstranit šum";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RN_METHOD), &cType);
		if (cType.operator LONG() == CFGVAL_RNM_BLUR9OF25)
			pszName = L"[0409]Selective blur[0405]Výběrové rozmazání";
		else
			pszName = L"[0409]Blur uniform areas[0405]Rozmazat souvislé oblasti";
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRemoveNoise::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	static IRPathPoint const cord[] =
	{
		{219, 111, 0, 0, 0, 0},
		{243, 85, 13, -11, 0, 0},
		{227, 43, 0, 0, 29, 0},
		{186, 43, -20, 0, 0, 0},
		{166, 2, 8.53925, -9.85298, -13, 15},
	};
	static IRGridItem const grid = {0, 240};
	static IRCanvas const canvas = {0, 0, 256, 256, 0, 1, NULL, &grid};
	cRenderer(&canvas, itemsof(cord), cord, pSI->GetMaterial(ESMOutline));
	static IRPolyPoint const temp[] = { {134, 180}, {146, 170}, {186, 170}, {186, 199}, {134, 199}, };
	cRenderer(&canvas, itemsof(temp), temp, pSI->GetMaterial(ESMInterior));
	static IRPathPoint const vapor[] =
	{
		{50, 149, 4.41828, -4.41827, -4.41828, 4.41827},
		{66, 149, 0, 0, -4.41828, -4.41827},
		{81, 164, 0, 0, 0, 0},
		{65, 180, 0, 0, 0, 0},
		{50, 165, -4.41828, -4.41827, 0, 0},
	};
	cRenderer(&canvas, itemsof(vapor), vapor, pSI->GetMaterial(ESMInterior));
	static IRPathPoint const body1[] =
	{
		{12, 228, 31, -77, 0, 0},
		{215, 99, 6.97581, -0.581322, -96, 8},
		{231, 110, 7, 24, -2.30894, -7.91636},
		{255, 191, 2, 7, -1.73749, -6.08121},
		{246, 204, -10, 0, 8, 0},
		{224, 228, 0, 0, 0, -13},
	};
	static IRPathPoint const body2[] =
	{
		{126, 159, 12, -10, -11.0795, 9.23288},
		{197, 131, 4.04892, -0.778641, -26, 5},
		{204, 135, 2, 9, -0.433868, -1.95238},
		{213, 173, 0.808609, 4.04305, -1, -5},
		{208, 181, -12, 1, 4.98273, -0.415222},
		{134, 185, -12, 0, 14, 0},
	};
	static IRPath const body[] = { {itemsof(body1), body1}, {itemsof(body2), body2}, };
	cRenderer(&canvas, itemsof(body), body, pSI->GetMaterial(ESMScheme1Color1));
	static IRPathPoint const iron[] =
	{
		{0, 240, 3, -10, 0, 0},
		{22, 216, 0, 0, -16, 1},
		{231, 216, 7, 4, 0, 0},
		{235, 240, 0, 0, 6, -8},
	};
	cRenderer(&canvas, itemsof(iron), iron, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRemoveNoise::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_RN_METHOD(CFGID_RN_METHOD);
		pCfgInit->ItemIns1ofN(cCFGID_RN_METHOD, _SharedStringTable.GetStringAuto(IDS_CFGID_RN_METHOD_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RN_METHOD_DESC), CConfigValue(CFGVAL_RNM_BLURENERGY), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_RN_METHOD, CConfigValue(CFGVAL_RNM_BLUR9OF25), _SharedStringTable.GetStringAuto(IDS_CFGVAL_RNM_BLUR9OF25), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_RN_METHOD, CConfigValue(CFGVAL_RNM_BLURENERGY), _SharedStringTable.GetStringAuto(IDS_CFGVAL_RNM_BLURENERGY), 0, NULL);

		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_RN_METHOD;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_RNM_BLURENERGY;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RN_THRESHOLD), _SharedStringTable.GetStringAuto(IDS_CFGID_RN_THRESHOLD_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RN_THRESHOLD_DESC), CConfigValue(100L), NULL, 1, &tCond);

		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DocumentOperationRemoveNoise, CConfigGUIGreyscale>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRemoveNoise::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, sizeof(aFts)/sizeof(*aFts), aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

inline int minmax(int const val, int const min, int const max)
{
	return min > val ? min : (max < val ? max : val);
}
struct SOffset {int x,y;};
struct SArea {SOffset a[9];};
void RemoveNoise(TPixelChannel const* a_pS, TPixelChannel* a_pD, ULONG a_nSizeX, ULONG a_nSizeY)
{
	static SArea const s_aAreas[] =
	{
		{ { {-1,-1}, { 0,-1}, { 1,-1}, {-1, 0}, { 0, 0}, { 1, 0}, {-1, 1}, { 0, 1}, { 1, 1} } },
		{ { {-2,-2}, {-1,-2}, { 0,-2}, {-2,-1}, {-1,-1}, { 0,-1}, {-2, 0}, {-1, 0}, { 0, 0} } },
		{ { {-2, 0}, {-1, 0}, { 0, 0}, {-2, 1}, {-1, 1}, { 0, 1}, {-2, 2}, {-1, 2}, { 0, 2} } },
		{ { { 0,-2}, { 1,-2}, { 2,-2}, { 0,-1}, { 1,-1}, { 1,-1}, { 0, 0}, { 1, 0}, { 2, 0} } },
		{ { { 0, 0}, { 1, 0}, { 2, 0}, { 0, 1}, { 1, 1}, { 2, 1}, { 0, 2}, { 1, 2}, { 2, 2} } },
		{ { {-2,-2}, {-1,-2}, { 0,-2}, { 1,-2}, { 2,-2}, {-1,-1}, { 0,-1}, { 1,-1}, { 0, 0} } },
		{ { {-2, 2}, {-1, 2}, { 0, 2}, { 1, 2}, { 2, 2}, {-1, 1}, { 0, 1}, { 1, 1}, { 0, 0} } },
		{ { {-2,-2}, {-2,-1}, {-2, 0}, {-2, 1}, {-2, 2}, {-1,-1}, {-1, 0}, {-1, 1}, { 0, 0} } },
		{ { { 2,-2}, { 2,-1}, { 2, 0}, { 2, 1}, { 2, 2}, { 1,-1}, { 1, 0}, { 1, 1}, { 0, 0} } },
	};

	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (ULONG x = 0; x < a_nSizeX;)
		{
			int aDevs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvRs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvGs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvBs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			for (ULONG i = 0; i < itemsof(s_aAreas); ++i)
			{
				aDevs[i] = aAvRs[i] = aAvGs[i] = aAvBs[i] = 0;
				SOffset const* pOff = s_aAreas[i].a;
				for (ULONG o = 0; o < sizeof(SArea)/sizeof(SOffset); ++o)
				{
					TPixelChannel const* pPix = a_pS + a_nSizeX*minmax(y+pOff[o].y, 0, a_nSizeY-1) + minmax(x+pOff[o].x, 0, a_nSizeX-1);
					aAvRs[i] += pPix->bR;
					aAvGs[i] += pPix->bG;
					aAvBs[i] += pPix->bB;
				}
				aAvRs[i] /= 9;
				aAvGs[i] /= 9;
				aAvBs[i] /= 9;
				for (ULONG o = 0; o < sizeof(SArea)/sizeof(SOffset); ++o)
				{
					TPixelChannel const* pPix = a_pS + a_nSizeX*minmax(y+pOff[o].y, 0, a_nSizeY-1) + minmax(x+pOff[o].x, 0, a_nSizeX-1);
					int nDR = aAvRs[i]-pPix->bR;
					int nDG = aAvGs[i]-pPix->bG;
					int nDB = aAvBs[i]-pPix->bB;
					aDevs[i] += nDR*nDR + nDG*nDG + nDB*nDB;
				}
			}
			ULONG nBest = 0;
			for (ULONG i = 1; i < itemsof(s_aAreas); ++i)
			{
				if (aDevs[i] < aDevs[nBest])
					nBest = i;
			}
			TPixelChannel* pPix = a_pD + a_nSizeX*y + x;
			//if (aDevs[nBest] < 1000)
			//{
				pPix->bR = aAvRs[nBest];
				pPix->bG = aAvGs[nBest];
				pPix->bB = aAvBs[nBest];
				pPix->bA = a_pS[pPix-a_pD].bA;
			//}
			//else
			//{
			//	*pPix = a_pS[pPix-a_pD];
			//}
			++x;
			if (y < 2 || y+3 > a_nSizeY ||
				x < 2 || x+3 > a_nSizeX)
				continue;
			if (x+3 < a_nSizeX)
				x = a_nSizeX-2;
		}
	}

	if (a_nSizeX <= 4 || a_nSizeY <= 4)
		return;

	for (ULONG y = 2; y < a_nSizeY-2; ++y)
	{
		for (ULONG x = 2; x < a_nSizeX-2; ++x)
		{
			int aDevs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvRs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvGs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			int aAvBs[sizeof(s_aAreas)/sizeof(*s_aAreas)];
			for (ULONG i = 0; i < itemsof(s_aAreas); ++i)
			{
				aDevs[i] = aAvRs[i] = aAvGs[i] = aAvBs[i] = 0;
				SOffset const* pOff = s_aAreas[i].a;
				for (int o = 0; o < sizeof(SArea)/sizeof(SOffset); ++o)
				{
					TPixelChannel const* pPix = a_pS + a_nSizeX*(y+pOff[o].y) + x+pOff[o].x;
					aAvRs[i] += pPix->bR;
					aAvGs[i] += pPix->bG;
					aAvBs[i] += pPix->bB;
				}
				aAvRs[i] /= 9;
				aAvGs[i] /= 9;
				aAvBs[i] /= 9;
				for (ULONG o = 0; o < sizeof(SArea)/sizeof(SOffset); ++o)
				{
					TPixelChannel const* pPix = a_pS + a_nSizeX*(y+pOff[o].y) + x+pOff[o].x;
					int nDR = aAvRs[i]-pPix->bR;
					int nDG = aAvGs[i]-pPix->bG;
					int nDB = aAvBs[i]-pPix->bB;
					aDevs[i] += nDR*nDR + nDG*nDG + nDB*nDB;
				}
			}
			ULONG nBest = 0;
			for (ULONG i = 1; i < itemsof(s_aAreas); ++i)
			{
				if (aDevs[i] < aDevs[nBest])
					nBest = i;
			}
			TPixelChannel* pPix = a_pD + a_nSizeX*y + x;
			//if (aDevs[nBest] < 1000)
			//{
				pPix->bR = aAvRs[nBest];
				pPix->bG = aAvGs[nBest];
				pPix->bB = aAvBs[nBest];
				pPix->bA = a_pS[pPix-a_pD].bA;
			//}
			//else
			//{
			//	*pPix = a_pS[pPix-a_pD];
			//}
		}
	}
}

#include <agg_pixfmt_rgba.h>
#include <agg_blur.h>

void RemoveNoiseByEnergy(TPixelChannel const* a_pS, TPixelChannel* a_pD, ULONG const a_nSizeX, ULONG const a_nSizeY, LONG const a_nThreshold)
{
	CAutoVectorPtr<int> pEnergy(new int[a_nSizeX*a_nSizeY]);

	// calculate energy
	{
		TPixelChannel const* pSourcePixel = a_pS;
		int* pEnergyDataDest = pEnergy;
		for (ULONG iY = 0; iY < a_nSizeY; ++iY)
		{
			for (ULONG iX = 0; iX < a_nSizeX; ++iX)
			{
				ULONG iDiffTotal = 0;
				ULONG iDiffCount = 0;
				ULONG const nYMax = min((iY+4), a_nSizeY);
				for (ULONG iYSample = iY < 3 ? 0 : (iY-3); iYSample < nYMax; ++iYSample)
				{
					ULONG const nXMax = min((iX+4), a_nSizeX);
					for(ULONG iXSample = iX < 3 ? 0 : (iX-3); iXSample < nXMax; ++iXSample)
					{
						iDiffTotal += abs(int(pSourcePixel->bR) - int(a_pS[iYSample*a_nSizeX + iXSample].bR));
						iDiffTotal += abs(int(pSourcePixel->bG) - int(a_pS[iYSample*a_nSizeX + iXSample].bG));
						iDiffTotal += abs(int(pSourcePixel->bB) - int(a_pS[iYSample*a_nSizeX + iXSample].bB));
						iDiffTotal += abs(int(pSourcePixel->bA) - int(a_pS[iYSample*a_nSizeX + iXSample].bA));
						++iDiffCount;
					}
				}
				*pEnergyDataDest = ((iDiffTotal + (iDiffCount>>1))<<1) / iDiffCount;

				++pSourcePixel;
				++pEnergyDataDest;
			}
		}
	}
	CopyMemory(a_pD, a_pS, a_nSizeX*a_nSizeY*sizeof*a_pD);
	agg::rendering_buffer rbuf(reinterpret_cast<BYTE*>(a_pD), a_nSizeX, a_nSizeY, a_nSizeX<<2);
	agg::pixfmt_rgba32_plain img(rbuf);
	agg::recursive_blur<agg::rgba8, agg::recursive_blur_calc_rgba<> > rb;
	rb.blur(img, 4.0);
	ULONG const nPixels = a_nSizeX*a_nSizeY;
	int const* pE = pEnergy;
	TPixelChannel const* pS = a_pS;
	TPixelChannel* pD = a_pD;
	int const nF = 0x1000000/a_nThreshold;
	for (TPixelChannel* const pEnd = pD+nPixels; pD < pEnd; ++pE, ++pD, ++pS)
	{
		if (*pE >= a_nThreshold)
		{
			*pD = *pS;
		}
		else
		{
			int const n1 = nF**pE;
			int const n2 = 0x1000000-n1;
			pD->bR = (pD->bR*n1 + pS->bR*n2)>>24;
			pD->bG = (pD->bG*n1 + pS->bG*n2)>>24;
			pD->bB = (pD->bB*n1 + pS->bB*n2)>>24;
			pD->bA = (pD->bA*n1 + pS->bA*n2)>>24;
		}
	}
}

STDMETHODIMP CDocumentOperationRemoveNoise::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CWriteLock<IDocument> cLock(a_pDocument);

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CConfigValue cMethod;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RN_METHOD), &cMethod);

		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[tSize.nX*tSize.nY]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		CAutoVectorPtr<TPixelChannel> pDst(new TPixelChannel[tSize.nX*tSize.nY]);
		TPixelChannel* pD = pDst.m_p;
		TPixelChannel* pS = pSrc.m_p;
		LONG const nSize = tSize.nX*tSize.nY;
		if (cMethod.operator LONG() == CFGVAL_RNM_BLURENERGY)
		{
			CConfigValue cThreshold;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_RN_THRESHOLD), &cThreshold);
			RemoveNoiseByEnergy(pS, pD, tSize.nX, tSize.nY, cThreshold.operator LONG() < 2L ? 2L : cThreshold);
		}
		else
		//if (bHasAlpha && !bIgnoreAlpha)
		//{
		//	for (TRasterImagePixel* p = pS; nSize; --nSize, ++p)
		//	{
		//		// premultiply
		//		p->bR = (unsigned(p->bR)*p->bA)/255;
		//		p->bG = (unsigned(p->bG)*p->bA)/255;
		//		p->bB = (unsigned(p->bB)*p->bA)/255;
		//	}
		//	Convolution<true>(pS, pD, tSize.n0, tSize.n1, nKernelX, nKernelY, cKernel, fDiv, fBias);
		//	nSize = tSize.n0*tSize.n1;
		//	if (bHasAlpha && !bIgnoreAlpha) for (TRasterImagePixel* p = pD; nSize; --nSize, ++p)
		//	{
		//		// demultiply
		//		if (p->bA)
		//		{
		//			p->bR = (unsigned(p->bR)*255)/p->bA;
		//			p->bG = (unsigned(p->bG)*255)/p->bA;
		//			p->bB = (unsigned(p->bB)*255)/p->bA;
		//		}
		//		else
		//		{
		//			p->bR = p->bG = p->bB = 0;
		//		}
		//	}
		//}
		//else
		{
			RemoveNoise(pS, pD, tSize.nX, tSize.nY);
		}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pDst.m_p, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
