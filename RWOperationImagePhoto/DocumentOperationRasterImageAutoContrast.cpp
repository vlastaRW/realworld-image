// DocumentOperationRasterImageAutoContrast.cpp : Implementation of CDocumentOperationRasterImageAutoContrast

#include "stdafx.h"
#include "DocumentOperationRasterImageAutoContrast.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_AL_TYPE[] = L"Levels";
const LONG CFGVAL_ALT_CONTRAST = 0;
const LONG CFGVAL_ALT_LEVELS = 1;
const OLECHAR CFGID_TOLERANCE[] = L"Tolerance";

// CDocumentOperationRasterImageAutoContrast

STDMETHODIMP CDocumentOperationRasterImageAutoContrast::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEAUTOCONTRAST_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRasterImageAutoContrast::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Automatic contrast[0405]Automatický kontrast";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_AL_TYPE), &cType);
		if (cType.operator LONG() == CFGVAL_ALT_LEVELS)
			pszName = L"[0409]Automatic levels[0405]Automatické úrovně";
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageAutoContrast::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_AL_TYPE(CFGID_AL_TYPE);
		pCfgInit->ItemIns1ofN(cCFGID_AL_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_AL_TYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_AL_TYPE_DESC), CConfigValue(CFGVAL_ALT_CONTRAST), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_AL_TYPE, CConfigValue(CFGVAL_ALT_CONTRAST), _SharedStringTable.GetStringAuto(IDS_CFGVAL_ALT_CONTRAST), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_AL_TYPE, CConfigValue(CFGVAL_ALT_LEVELS), _SharedStringTable.GetStringAuto(IDS_CFGVAL_ALT_LEVELS), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TOLERANCE), _SharedStringTable.GetStringAuto(IDS_CFGID_TOLERANCE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOLERANCE_DESC), CConfigValue(0.5f), NULL, 0, NULL);

		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DocumentOperationRasterImageGreyscale, CConfigGUIGreyscale>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageAutoContrast::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

STDMETHODIMP CDocumentOperationRasterImageAutoContrast::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CWriteLock<IDocument> cLock(a_pDocument);

		CConfigValue cType;
		CConfigValue cTolerance;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_AL_TYPE), &cType);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOLERANCE), &cTolerance);
		float const fTolerance = cTolerance;

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		ULONG const nPixels = tSize.nX * tSize.nY;
		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, NULL, NULL, nPixels, pPixels, NULL, EIRIAccurate);
		if (FAILED(hRes)) return hRes;

		ULONG nWhiteR = 255;
		ULONG nWhiteG = 255;
		ULONG nWhiteB = 255;
		ULONG nBlackR = 0;
		ULONG nBlackG = 0;
		ULONG nBlackB = 0;
		switch (cType.operator LONG())
		{
		case CFGVAL_ALT_LEVELS:
			{
				ULONG aHistR[256];
				ULONG aHistG[256];
				ULONG aHistB[256];
				ZeroMemory(aHistR, sizeof aHistR);
				ZeroMemory(aHistG, sizeof aHistG);
				ZeroMemory(aHistB, sizeof aHistB);
				TPixelChannel* p = pPixels;
				ULONG nRealPixels = 0;
				for (TPixelChannel* const pEnd = p+nPixels; p != pEnd; ++p)
				{
					if (p->bA == 0) // ignore transparent pixels // TODO: weight of semitransparent ones should be lower
						continue;
					++aHistR[p->bR];
					++aHistG[p->bG];
					++aHistB[p->bB];
					++nRealPixels;
				}

				if (nRealPixels == 0)
					return S_FALSE;

				ULONG const nLimit = nRealPixels*0.01f*fTolerance + 0.5f;

				ULONG nSum = 0;
				while ((nSum += aHistR[nWhiteR]) < nLimit && nWhiteR > 170) --nWhiteR;
				nSum = 0;
				while ((nSum += aHistG[nWhiteG]) < nLimit && nWhiteG > 170) --nWhiteG;
				nSum = 0;
				while ((nSum += aHistB[nWhiteB]) < nLimit && nWhiteB > 170) --nWhiteB;

				nSum = 0;
				while ((nSum += aHistR[nBlackR]) < nLimit && nBlackR < 85) ++nBlackR;
				nSum = 0;
				while ((nSum += aHistG[nBlackG]) < nLimit && nBlackG < 85) ++nBlackG;
				nSum = 0;
				while ((nSum += aHistB[nBlackB]) < nLimit && nBlackB < 85) ++nBlackB;
			}
			break;
		case CFGVAL_ALT_CONTRAST:
			{
				ULONG aHistR[256];
				ULONG aHistG[256];
				ULONG aHistB[256];
				ZeroMemory(aHistR, sizeof aHistR);
				ZeroMemory(aHistG, sizeof aHistG);
				ZeroMemory(aHistB, sizeof aHistB);
				TPixelChannel* p = pPixels;
				ULONG nRealPixels = 0;
				for (TPixelChannel* const pEnd = p+nPixels; p != pEnd; ++p)
				{
					if (p->bA == 0) // ignore transparent pixels // TODO: weight of semitransparent ones should be lower
						continue;
					++aHistR[p->bR];
					++aHistG[p->bG];
					++aHistB[p->bB];
					++nRealPixels;
				}

				if (nRealPixels == 0)
					return S_FALSE;

				ULONG const nLimit = nRealPixels*0.03f*fTolerance + 0.5f;

				ULONG nWhite = 255;
				ULONG nSum = 0;
				while ((nSum += aHistR[nWhite] + aHistG[nWhite] + aHistB[nWhite]) < nLimit && nWhite > 170) --nWhite;
				nWhiteR = nWhiteG = nWhiteB = nWhite;

				ULONG nBlack = 0;
				nSum = 0;
				while ((nSum += aHistR[nBlack] + aHistG[nBlack] + aHistB[nBlack]) < nLimit && nBlack < 85) ++nBlack;
				nBlackR = nBlackG = nBlackB = nBlack;
			}
			break;
		default:
			return E_RW_INVALIDPARAM;
		}
		if (nBlackR == 0 && nBlackG == 0 && nBlackB == 0 && nWhiteR == 255 && nWhiteG == 255 && nWhiteB == 255)
			return S_FALSE;

		BYTE aLUTR[256];
		BYTE aLUTG[256];
		BYTE aLUTB[256];
		for (ULONG i = 0; i < 256; ++i)
		{
			aLUTR[i] = i <= nBlackR ? 0 : (i >= nWhiteR ? 255 : ( ((i-nBlackR)*255+((nWhiteR-nBlackR)>>1))/(nWhiteR-nBlackR) ));
			aLUTG[i] = i <= nBlackG ? 0 : (i >= nWhiteG ? 255 : ( ((i-nBlackG)*255+((nWhiteG-nBlackG)>>1))/(nWhiteG-nBlackG) ));
			aLUTB[i] = i <= nBlackB ? 0 : (i >= nWhiteB ? 255 : ( ((i-nBlackB)*255+((nWhiteB-nBlackB)>>1))/(nWhiteB-nBlackB) ));
		}

		TPixelChannel* const pEnd = pPixels.m_p+nPixels;
		for (TPixelChannel* p = pPixels.m_p; p != pEnd; ++p)
			if (p->bA)
			{
				p->bR = aLUTR[p->bR];
				p->bG = aLUTG[p->bG];
				p->bB = aLUTB[p->bB];
			}

		return pRI->TileSet(EICIRGBA, NULL, NULL, NULL, nPixels, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
