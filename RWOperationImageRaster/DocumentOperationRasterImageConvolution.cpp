// DocumentOperationRasterImageConvolution.cpp : Implementation of CDocumentOperationRasterImageConvolution

#include "stdafx.h"
#include "DocumentOperationRasterImageConvolution.h"
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>

const OLECHAR CFGID_MATRIX[] = L"Matrix";
const OLECHAR CFGID_DIVISOR[] = L"Div";
const OLECHAR CFGID_BIAS[] = L"Bias";
const OLECHAR CFGID_IGNOREALPHA[] = L"IgnoreAlpha";

bool ParseMatrix(wchar_t const* a_pString, std::vector<float>& a_cData, size_t& a_nSizeX, size_t& a_nSizeY)
{
	// skip initial whitespace
	while (*a_pString == L' ' || *a_pString == L'\t' || *a_pString == L'\r' || *a_pString == L'\n') ++a_pString;

	size_t nX = 0;
	size_t nInRow = 0;
	while (*a_pString)
	{
		wchar_t* psz = NULL;
		double val = wcstod(a_pString, &psz/*, L"English"*/);
		if (psz == a_pString)
			return false; // invalid character
		a_cData.push_back(val);
		a_pString = psz;
		++nInRow;
		bool bEOL = false;
		while (*a_pString == L' ' || *a_pString == L'\t' || *a_pString == L'\r' || *a_pString == L'\n')
		{
			if (*a_pString == L'\r' || *a_pString == L'\n')
				bEOL = true;
			++a_pString;
		}
		if (bEOL)
		{
			if (nX == 0)
				nX = nInRow;
			else if (nX != nInRow)
				return false; // not a matrix
			nInRow = 0;
		}
	}
	if (nInRow)
	{
		if (nX == 0)
			nX = nInRow;
		else if (nX != nInRow)
			return false; // not a matrix
	}
	a_nSizeX = nX;
	a_nSizeY = a_cData.size()/nX;
	return (a_nSizeX&1) && (a_nSizeY&1); // dimensions must be odd
}

#include "ConfigGUIConvolution.h"


// CDocumentOperationRasterImageConvolution

STDMETHODIMP CDocumentOperationRasterImageConvolution::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGECONVOLUTION_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
STDMETHODIMP CDocumentOperationRasterImageConvolution::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Convolution matrix[0405]Konvoluční matice";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cMatrix;
		CConfigValue cDivisor;
		CConfigValue cBias;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_MATRIX), &cMatrix);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DIVISOR), &cDivisor);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BIAS), &cBias);
		// try to find out whether it is a sharpen or soften matrix and set controls accordingly
		if (cBias.operator float() == 0.0f)
		{
			std::vector<float> cKernel;
			size_t nKernelX = 0;
			size_t nKernelY = 0;
			if (ParseMatrix(cMatrix, cKernel, nKernelX, nKernelY) && nKernelX == 3 && nKernelY == 3)
			{
				if (cKernel[0] == 0.0f && cKernel[1] ==-2.0f && cKernel[2] == 0.0f && 
					cKernel[3] ==-2.0f && fabsf(cKernel[4]-cDivisor.operator float()-8.0f) < 0.001f && cKernel[5] ==-2.0f && 
					cKernel[6] == 0.0f && cKernel[7] ==-2.0f && cKernel[8] == 0.0f)
				{
					pszName = L"[0409]Sharpen[0405]Zaostření";
				}
				else
				if (cKernel[0] == 1.0f && cKernel[1] == 1.0f && cKernel[2] == 1.0f && 
					cKernel[3] == 1.0f && fabsf(cKernel[4]-cDivisor.operator float()+8.0f) < 0.001f && cKernel[5] == 1.0f && 
					cKernel[6] == 1.0f && cKernel[7] == 1.0f && cKernel[8] == 1.0f)
				{
					pszName = L"[0409]Soften[0405]Rozmazání";
				}
			}
		}
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageConvolution::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_MATRIX), _SharedStringTable.GetStringAuto(IDS_CFGID_MATRIX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_MATRIX_DESC), CConfigValue(L"0 0 0\r\n0 1 0\r\n0 0 0"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_DIVISOR), _SharedStringTable.GetStringAuto(IDS_CFGID_DIVISOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_DIVISOR_DESC), CConfigValue(1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BIAS), _SharedStringTable.GetStringAuto(IDS_CFGID_BIAS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BIAS_DESC), CConfigValue(0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IGNOREALPHA), _SharedStringTable.GetStringAuto(IDS_CFGID_IGNOREALPHA_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_IGNOREALPHA_DESC), CConfigValue(false), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageConvolution, CConfigGUIConvolution>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageConvolution::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
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

template<bool t_bAlpha>
void Convolution(TRasterImagePixel const* a_pS, TRasterImagePixel* a_pD, LONG a_nSizeX, LONG a_nSizeY, int a_nKernelX, int a_nKernelY, std::vector<float> const& a_pKernel, float a_fDiv, float a_fBias)
{
	float const fMul = 1.0f/a_fDiv;
	int nDeltaX = (a_nKernelX-1)>>1;
	int nDeltaY = (a_nKernelY-1)>>1;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < a_nSizeX; ++x)
		{
			if (!t_bAlpha && a_pS[a_nSizeX*y+x].bA == 0)
			{
				*a_pD = a_pS[a_nSizeX*y+x];
				++a_pD;
				continue;
			}

			float r = 0;
			float g = 0;
			float b = 0;
			float a = 0;
			std::vector<float>::const_iterator pKernel = a_pKernel.begin();
			for (int yy = y-nDeltaY; yy <= y+nDeltaY; ++yy)
			{
				int yyy = yy < 0 ? 0 : (yy >= a_nSizeY ? a_nSizeY-1 : yy);
				for (int xx = x-nDeltaX; xx <= x+nDeltaX; ++xx)
				{
					int xxx = xx < 0 ? 0 : (xx >= a_nSizeX ? a_nSizeX-1 : xx);
					TRasterImagePixel const* const pPx = a_pS+a_nSizeX*yyy+xxx;
					r += pPx->bR**pKernel;
					g += pPx->bG**pKernel;
					b += pPx->bB**pKernel;
					if (t_bAlpha)
						a += pPx->bA**pKernel;
					++pKernel;
				}
			}
			r = r*fMul+a_fBias;
			g = g*fMul+a_fBias;
			b = b*fMul+a_fBias;
			if (t_bAlpha)
			{
				a = a*fMul+a_fBias;
				BYTE bA = a < 0.0f ? 0 : (a > 255.0f ? 255 : BYTE(a+0.5f));
				if (a == 0)
				{
					a_pD->bR = a_pD->bG = a_pD->bB = a_pD->bA = 0;
				}
				else
				{
					a_pD->bR = r < 0.0f ? 0 : (r > bA ? bA : BYTE(r+0.5f));
					a_pD->bG = g < 0.0f ? 0 : (g > bA ? bA : BYTE(g+0.5f));
					a_pD->bB = b < 0.0f ? 0 : (b > bA ? bA : BYTE(b+0.5f));
					a_pD->bA = bA;
				}
			}
			else
			{
				a_pD->bR = r < 0.0f ? 0 : (r > 255.0f ? 255 : BYTE(r+0.5f));
				a_pD->bG = g < 0.0f ? 0 : (g > 255.0f ? 255 : BYTE(g+0.5f));
				a_pD->bB = b < 0.0f ? 0 : (b > 255.0f ? 255 : BYTE(b+0.5f));
				a_pD->bA = a_pS[a_nSizeX*y+x].bA;
			}
			++a_pD;
		}
	}
}

STDMETHODIMP CDocumentOperationRasterImageConvolution::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		bool bHasAlpha = true;

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_MATRIX), &cVal);
		std::vector<float> cKernel;
		size_t nKernelX = 0;
		size_t nKernelY = 0;
		if (!ParseMatrix(cVal, cKernel, nKernelX, nKernelY) || nKernelX > 31 || nKernelY > 31)
			return E_RW_INVALIDPARAM;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DIVISOR), &cVal);
		float const fDiv = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BIAS), &cVal);
		float const fBias = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IGNOREALPHA), &cVal);
		bool const bIgnoreAlpha = cVal;

		CAutoVectorPtr<TRasterImagePixel> pSrc(new TRasterImagePixel[tSize.nX*tSize.nY]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pSrc.m_p), NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		CAutoVectorPtr<TRasterImagePixel> pDst(new TRasterImagePixel[tSize.nX*tSize.nY]);
		TRasterImagePixel* pD = pDst.m_p;
		TRasterImagePixel* pS = pSrc.m_p;
		LONG nSize = tSize.nX*tSize.nY;
		if (bHasAlpha && !bIgnoreAlpha)
		{
			for (TRasterImagePixel* p = pS; nSize; --nSize, ++p)
			{
				// premultiply
				p->bR = (unsigned(p->bR)*p->bA)/255;
				p->bG = (unsigned(p->bG)*p->bA)/255;
				p->bB = (unsigned(p->bB)*p->bA)/255;
			}
			Convolution<true>(pS, pD, tSize.nX, tSize.nY, nKernelX, nKernelY, cKernel, fDiv, fBias);
			nSize = tSize.nX*tSize.nY;
			if (bHasAlpha && !bIgnoreAlpha) for (TRasterImagePixel* p = pD; nSize; --nSize, ++p)
			{
				// demultiply
				if (p->bA)
				{
					p->bR = (unsigned(p->bR)*255)/p->bA;
					p->bG = (unsigned(p->bG)*255)/p->bA;
					p->bB = (unsigned(p->bB)*255)/p->bA;
				}
				else
				{
					p->bR = p->bG = p->bB = 0;
				}
			}
		}
		else
		{
			Convolution<false>(pS, pD, tSize.nX, tSize.nY, nKernelX, nKernelY, cKernel, fDiv, fBias);
		}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel const*>(pDst.m_p), TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImageConvolution::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRPathPoint const circle[] =
	{
		{256, 128, 0, -70.6925, 0, 70.6925},
		{128, 0, -70.6925, 0, 70.6925, 0},
		{0, 128, 0, 70.6925, 0, -70.6925},
		{128, 256, 70.6925, 0, -70.6925, 0},
	};
	IRPolyPoint const sign[] =
	{
		{193, 80}, {145, 128}, {193, 176}, {176, 193}, {128, 145}, {80, 193}, {63, 176}, {111, 128}, {63, 80}, {80, 63}, {128, 111}, {176, 63},
	};
	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	IRTarget target(0.8f);
	cRenderer(&canvas, itemsof(circle), circle, pSI->GetMaterial(ESMInterior), target);
	cRenderer(&canvas, itemsof(sign), sign, pSI->GetMaterial(ESMContrast), target);
	return cRenderer.get();
}

