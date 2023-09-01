// DocumentOperationRasterImageDropShadow.cpp : Implementation of CDocumentOperationRasterImageDropShadow

#include "stdafx.h"
#include "DocumentOperationRasterImageDropShadow.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <math.h>
#include <GammaCorrection.h>

const OLECHAR CFGID_DENSITY[] = L"Density";
const OLECHAR CFGID_SIZE[] = L"Size";
const OLECHAR CFGID_OFFSETX[] = L"OffsetX";
const OLECHAR CFGID_OFFSETY[] = L"OffsetY";
const OLECHAR CFGID_TYPE[] = L"Type";
const LONG CFGVAL_TYPE_STANDARD = 0;
const LONG CFGVAL_TYPE_OUTLINE = 1;
const OLECHAR CFGID_COLOR[] = L"Color";
const OLECHAR CFGID_MASKID[] = L"MaskID";
const OLECHAR CFGID_BLENDING[] = L"Blending";
const LONG CFGVAL_BLEND_OUTER = 0;
const LONG CFGVAL_BLEND_INNER = 1;
const LONG CFGVAL_BLEND_REPLACE = 2;

#include "ConfigGUIDropShadow.h"


// CDocumentOperationRasterImageDropShadow

STDMETHODIMP CDocumentOperationRasterImageDropShadow::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEDROPSHADOW_NAME);
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

STDMETHODIMP CDocumentOperationRasterImageDropShadow::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Shadow[0405]Stín";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cSize;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cSize);
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BLENDING), &cType);
		switch (cType.operator LONG())
		{
		case CFGVAL_BLEND_OUTER: pszName = L"[0409]Outer shadow[0405]Vnější stín"; break;
		case CFGVAL_BLEND_INNER: pszName = L"[0409]Inner shadow[0405]Vnitřní stín"; break;
		case CFGVAL_BLEND_REPLACE: pszName = L"[0409]Shadow only[0405]Pouze stín"; break;
		}
		CComPtr<ILocalizedString> pMethod;
		pMethod.Attach(new CMultiLanguageString(pszName));
		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_COLOR), &cColor);
		CComPtr<INamedColors> pNC;
		RWCoCreateInstance(pNC, __uuidof(NamedColors));
		CComPtr<ILocalizedString> pColorName;
		if (pNC) pNC->ColorToName(0xff000000|cColor.operator LONG(), &pColorName);
		CComPtr<ILocalizedString> pParams;
		if (pColorName)
		{
			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			pParams = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s, %ipx")));
			pStr->Init(pTempl, pColorName, int(cSize.operator float()+0.5f));
		}
		else
		{
			OLECHAR szTmp[16];
			swprintf(szTmp, L"%g", int(cSize.operator float()*10+0.5f)*0.1f);
			pParams.Attach(new CSimpleLocalizedString(szTmp));
		}
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
		pStr->Init(pTempl, pMethod, pParams);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::PreviewIconID(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	//// {6D348198-2F99-420b-AB9A-290557FC1831}
	//static const GUID tID = {0x6d348198, 0x2f99, 0x420b, {0xab, 0x9a, 0x29, 0x5, 0x57, 0xfc, 0x18, 0x31}};
	*a_pIconID = CLSID_DocumentOperationRasterImageDropShadow;//tID;
	return S_OK;
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImageDropShadow::GetDefaultIcon(ULONG a_nSize)
{
	IRPathPoint const sphere[] =
	{
		{192+32, 96+32, 0, -53.0193, 0, 53.0193},
		{96+32, 0+32, -53.0193, 0, 53.0193, 0},
		{0+32, 96+32, 0, 53.0193, 0, -53.0193},
		{96+32, 192+32, 53.0193, 0, -53.0193, 0},
	};
	IRPathPoint const highlight[] =
	{
		{73+32, 50+32, -4.79583, -5.71544, 4.79583, 5.71544},
		{51+32, 50+32, -6.90661, 5.79534, 6.90661, -5.79534},
		{47+32, 70+32, 4.79583, 5.71544, -4.79583, -5.71544},
		{69+32, 70+32, 6.90661, -5.79534, -6.90661, 5.79534},
	};
	IRPathPoint const shade[] =
	{
		{178+32, 60+32, 13.8071, 30.3757, -9, 70},
		{159+32, 159+32, -23.7482, 23.7482, 23.7482, -23.7482},
		{60+32, 178+32, 70, -9, 30.3757, 13.8071},
	};
	IRPathPoint const shadow1[] =
	{
		{256, 128, 0, -70.6925, 0, 70.6925},
		{128, 0, -70.6925, 0, 70.6925, 0},
		{0, 128, 0, 70.6925, 0, -70.6925},
		{128, 256, 70.6925, 0, -70.6925, 0},
		//{256, 136, 0, -66.2742, 0, 66.2742},
		//{136, 16, -66.2742, 0, 66.2742, 0},
		//{16, 136, 0, 66.2742, 0, -66.2742},
		//{136, 256, 66.2742, 0, -66.2742, 0},
	};
	IRPathPoint const shadow2[] =
	{
		{248, 128, 0, -66.2742, 0, 66.2742},
		{128, 8, -66.2742, 0, 66.2742, 0},
		{8, 128, 0, 66.2742, 0, -66.2742},
		{128, 248, 66.2742, 0, -66.2742, 0},
		//{248, 136, 0, -61.8559, 0, 61.8559},
		//{136, 24, -61.8559, 0, 61.8559, 0},
		//{24, 136, 0, 61.8559, 0, -61.8559},
		//{136, 248, 61.8559, 0, -61.8559, 0},
	};
	IRPathPoint const shadow3[] =
	{
		{240, 128, 0, -61.8559, 0, 61.8559},
		{128, 16, -61.8559, 0, 61.8559, 0},
		{16, 128, 0, 61.8559, 0, -61.8559},
		{128, 240, 61.8559, 0, -61.8559, 0},
		//{240, 136, 0, -57.4376, 0, 57.4376},
		//{136, 32, -57.4376, 0, 57.4376, 0},
		//{32, 136, 0, 57.4376, 0, -57.4376},
		//{136, 240, 57.4376, 0, -57.4376, 0},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	IRFill matWhite(0x7fffffff);
	IRFill matBlack(0x7f000000);
	IRFill matShadow(0x18000000);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shadow1), shadow1, &matShadow);
	cRenderer(&canvas, itemsof(shadow2), shadow2, &matShadow);
	cRenderer(&canvas, itemsof(shadow3), shadow3, &matShadow);
	cRenderer(&canvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(highlight), highlight, &matWhite);
	cRenderer(&canvas, itemsof(shade), shade, &matBlack);
	return cRenderer.get();
}

//static inline agg::rgba8 GetIconShadowColor()
//{
//	COLORREF clr3D = GetSysColor(COLOR_WINDOWTEXT);
//	float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
//	float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
//	float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
//	float const f3DA = 0.64f;
//	float const fA = 0.36f;
//	return agg::rgba8(255.0f*(f3DR*f3DA+fA)+0.5f, 255.0f*(f3DG*f3DA+fA)+0.5f, 255.0f*(f3DB*f3DA+fA)+0.5f, 255);
//}
//
//#include <RenderIcon.h>
//
//STDMETHODIMP CDocumentOperationRasterImageDropShadow::PreviewIcon(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
//{
//	if (a_phIcon == NULL)
//		return E_POINTER;
//	try
//	{
//		static float const f = 1.0f/256.0f;
//		COLORREF clrOut = GetSysColor(COLOR_WINDOWTEXT);
//		static TPolyCoords const aCoords1[] =
//		{
//			{f*186, f*100},
//			{0, -f*47.4965}, {f*47.4965, 0}, {f*100, f*14},
//			{-f*47.4965, 0}, {0, -f*47.4965}, {f*14, f*100},
//			{0, f*47.4965}, {-f*47.4965, 0}, {f*100, f*186},
//			{f*47.4965, 0}, {0, f*47.4965}, {f*186, f*100}
//		};
//		static TPolyCoords const aCoords2[] =
//		{
//			{f*240, f*130},
//			{0, f*60.7513}, {f*60.7513, 0}, {f*130, f*240},
//			{-f*42.7638, 0}, {f*18.1994, f*35.6398}, {f*32, f*180},
//			{f*18.3241, f*15.5994}, {-f*25.9506, 0}, {f*100, f*205},
//			{f*57.9899, 0}, {0, f*57.9899}, {f*205, f*100},
//			{0, -f*25.9506}, {f*15.5994, f*18.3241}, {f*180, f*32},
//			{f*35.6398, f*18.1994}, {0, -f*42.7638}, {f*240, f*130},
//		};
//		TIconPolySpec tPolySpec[2];
//		tPolySpec[0].nVertices = itemsof(aCoords2);
//		tPolySpec[0].pVertices = aCoords2;
//		tPolySpec[0].interior = GetIconShadowColor();
//		tPolySpec[0].outline = tPolySpec[0].interior;//agg::rgba8(0, 0, 0, 0);
//		tPolySpec[0].outline.a = 128;
//		tPolySpec[1].nVertices = itemsof(aCoords1);
//		tPolySpec[1].pVertices = aCoords1;
//		tPolySpec[1].interior = GetIconFillColor();
//		tPolySpec[1].outline = agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
//		*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
//		return S_OK;
//	}
//	catch (...)
//	{
//		return E_UNEXPECTED;
//	}
//}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_DENSITY), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWDENSITY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWDENSITY_DESC), CConfigValue(150.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SIZE), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWSIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWSIZE_DESC), CConfigValue(3.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_OFFSETX), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWOFFSET_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWOFFSET_DESC), CConfigValue(1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_OFFSETY), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWOFFSET_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWOFFSET_DESC), CConfigValue(1.0f), NULL, 0, NULL);
		CComBSTR cCFGID_TYPE(CFGID_TYPE);
		pCfgInit->ItemIns1ofN(cCFGID_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWTYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWTYPE_DESC), CConfigValue(CFGVAL_TYPE_STANDARD), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TYPE, CConfigValue(CFGVAL_TYPE_STANDARD), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TYPE_STANDARD), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TYPE, CConfigValue(CFGVAL_TYPE_OUTLINE), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TYPE_OUTLINE), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWCOLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SHADOWCOLOR_DESC), CConfigValue(LONG(0)), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_MASKID), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		CComBSTR cCFGID_BLENDING(CFGID_BLENDING);
		pCfgInit->ItemIns1ofN(cCFGID_BLENDING, CMultiLanguageString::GetAuto(L"[0409]Blending[0405]Míchání"), CMultiLanguageString::GetAuto(L"[0409]How is the shadow combined with the original shape.[0405]Jak se stín zkombinuje s původním obsahem."), CConfigValue(CFGVAL_BLEND_OUTER), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BLENDING, CConfigValue(CFGVAL_BLEND_OUTER), CMultiLanguageString::GetAuto(L"[0409]Outer shadow[0405]Vnější stín"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BLENDING, CConfigValue(CFGVAL_BLEND_INNER), CMultiLanguageString::GetAuto(L"[0409]Inner shadow[0405]Vnitřní stín"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_BLENDING, CConfigValue(CFGVAL_BLEND_REPLACE), CMultiLanguageString::GetAuto(L"[0409]Shadow only[0405]Pouze stín"), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageDropShadow, CConfigGUIDropShadow>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return S_FALSE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void DropShadow(TRasterImagePixel* a_pData, int a_nSizeX, int a_nSizeY, float a_fRadius, float a_fOffsetX, float a_fOffsetY, float a_fDensity, /*float a_fBluriness, */bool a_bOutlineOnly, LONG a_nColor, BYTE* a_pMask, LONG a_eBlending)
{
	//if (a_fBluriness < 0.001f)
	//	a_fBluriness = 0.001f;
	if (a_fRadius < 0.0f)
		a_fRadius = 0.0f;
	else if (a_fRadius > 250.0f)
		a_fRadius = 250.0f;
	int nDensity = a_fDensity*256.0f+0.5f;
	if (nDensity < 1) nDensity = 1;
	else if (nDensity > 16*256) nDensity = 16*256;

	CAutoVectorPtr<DWORD> aShadow(new DWORD[a_nSizeX*a_nSizeY]);
	DWORD *pD = aShadow;
	DWORD *pDEnd = pD+a_nSizeX*a_nSizeY;
	TRasterImagePixel *pS = a_pData;
	//if (a_eBlending == CFGVAL_BLEND_INNER)
	//{
	//	if (a_pMask)
	//	{
	//		BYTE* pM = a_pMask;
	//		while (pD < pDEnd)
	//		{
	//			*pD = 255-((DWORD(pS->bA)**pM)>>8);
	//			++pD;
	//			++pS;
	//			++pM;
	//		}
	//	}
	//	else
	//	{
	//		while (pD < pDEnd)
	//		{
	//			*pD = 255-pS->bA;
	//			++pD;
	//			++pS;
	//		}
	//	}
	//}
	//else
	{
		if (a_pMask)
		{
			BYTE* pM = a_pMask;
			while (pD < pDEnd)
			{
				*pD = (DWORD(pS->bA)**pM)>>8;
				++pD;
				++pS;
				++pM;
			}
		}
		else
		{
			while (pD < pDEnd)
			{
				*pD = pS->bA;
				++pD;
				++pS;
			}
		}
	}

	// outline only shadow
	if (a_bOutlineOnly)
	{
		// TODO: speed optimization
		CAutoVectorPtr<bool> aOutlineMask(new bool[a_nSizeX*a_nSizeY]);
		int nMaxRadSq = (a_fRadius+1)*(a_fRadius+1)*0.5f+0.5f;
		int nRadius = a_fRadius+0.5f;
		int nPatternSize = nRadius+nRadius+1;
		if (nMaxRadSq < 2)
			nMaxRadSq = 2;
		for (int nY = 0; nY < a_nSizeY; nY++)
		{
			int nX;
			for (nX = 0; nX < a_nSizeX; nX++)
			{
				if (aShadow[nX+a_nSizeX*nY] == 0xff)
				{
					aOutlineMask[nX+a_nSizeX*nY] = true;
				}
				else if (a_pData[nX+a_nSizeX*nY].bA == 0)
				{
					aOutlineMask[nX+a_nSizeX*nY] = false;
				}
				else
				{
					bool& b = aOutlineMask[nX+a_nSizeX*nY];
					b = false;
					int nXSubOff = nX-nRadius;
					int nYSubOff = nY-nRadius;
					for (int nYSub = 0; !b && nYSub < nPatternSize; nYSub++)
					{
						if ((nYSub+nYSubOff) < 0 || (nYSub+nYSubOff) >= a_nSizeY)
						{
							if ((nYSub-nRadius)*(nYSub-nRadius) <= nMaxRadSq)
							{
								b = true;
								break;
							}
							else
							{
								continue;
							}
						}
						for (int nXSub = 0; nXSub < nPatternSize; nXSub++)
						{
							if ((nXSub+nXSubOff) < 0 || (nXSub+nXSubOff) >= a_nSizeX)
							{
								if ((nXSub-nRadius)*(nXSub-nRadius) <= nMaxRadSq)
								{
									b = true;
									break;
								}
								else
								{
									continue;
								}
							}
							TRasterImagePixel const* const pSrcPixel = a_pData + (nXSub+nXSubOff)+(nYSub+nYSubOff)*a_nSizeX;
							int const nCurSqRad = (nXSub-nRadius)*(nXSub-nRadius)+(nYSub-nRadius)*(nYSub-nRadius);
							if ((nCurSqRad <= nMaxRadSq && pSrcPixel->bA == 0) || (nCurSqRad <= 2 && pSrcPixel->bA == 255))
							{
								b = true;
								break;
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < a_nSizeX*a_nSizeY; ++i)
			if (!aOutlineMask[i])
				aShadow[i] = 0;
	}

	// X-direction blur
	int nX1 = floorf(a_fOffsetX-a_fRadius);
	int nX2 = ceilf(a_fOffsetX+a_fRadius+1);
	//a_fOffsetX = floorf(a_fOffsetX)+1.0f+floorf(a_fOffsetX)-a_fOffsetX;
	int const nDX = nX2-nX1;
	CAutoVectorPtr<DWORD> aShadowX(new DWORD[nDX]);
	DWORD dwShadowSumX = 0;
	for (int i = nX1; i < nX2; ++i)
	{
		double val = 0;
		for (int j = 0; j < 31; ++j)
		{
			double const d = 1.0f-fabsf(a_fOffsetX+0.5f-(i+(j+0.5)/31.0))/(a_fRadius+1.0f);
			if (d > 0) val += d;
		}
		dwShadowSumX += aShadowX[i-nX1] = int(255*val/31.0+0.5);//(1.0f-fabsf(a_fOffsetX-(i+0.5f))/a_fRadius);
	}
	pD = aShadow;
	{
		CAutoVectorPtr<DWORD> cLineX(new DWORD[a_nSizeX]);
		for (int nY = 0; nY < a_nSizeY; ++nY)
		{
			DWORD* pL = cLineX;
			DWORD* pS = pD+a_nSizeX*nY;
			for (int nX = 0; nX < a_nSizeX; ++nX)
			{
				DWORD dw = 0;
				int n1 = nX+nX1;
				int n2 = nX+nX2;
				DWORD* pShadow = aShadowX;
				if (n1 < 0)
				{
					pShadow -= n1;
					n1 = 0;
				}
				if (n2 > a_nSizeX)
					n2 = a_nSizeX;
				for (int i = n1; i < n2; ++i)
				{
					dw += pS[i]**pShadow;
					++pShadow;
				}
				*pL = dw>>8;
				++pL;
			}
			CopyMemory(pD+a_nSizeX*nY, cLineX.m_p, a_nSizeX*sizeof*pD);
		}
	}

	// Y-direction blur
	int nY1 = floorf(a_fOffsetY-a_fRadius);
	int nY2 = ceilf(a_fOffsetY+a_fRadius+1);
	if (nY2 == nY1) ++nY2;
	int const nDY = nY2-nY1;
	CAutoVectorPtr<DWORD> aShadowY(new DWORD[nDY]);
	DWORD dwShadowSumY = 0;
	for (int i = nY1; i < nY2; ++i)
	{
		double val = 0;
		for (int j = 0; j < 31; ++j)
		{
			double const d = 1.0f-fabsf(a_fOffsetY+0.5f-(i+(j+0.5)/31.0))/(a_fRadius+1.0f);
			if (d > 0) val += d;
		}
		dwShadowSumY += aShadowY[i-nY1] = int(255*val/31.0+0.5);//(1.0f-fabsf(a_fOffsetY-(i+0.5f))/a_fRadius);
	}
	pD = aShadow;
	{
		CAutoVectorPtr<DWORD> cLineY(new DWORD[a_nSizeY]);
		for (int nX = 0; nX < a_nSizeX; ++nX)
		{
			DWORD* pL = cLineY;
			DWORD* pS = pD+nX;
			for (int nY = 0; nY < a_nSizeY; ++nY)
			{
				DWORD dw = 0;
				int n1 = nY+nY1;
				int n2 = nY+nY2;
				DWORD* pShadow = aShadowY;
				if (n1 < 0)
				{
					pShadow -= n1;
					n1 = 0;
					// TODO: take outer color into account
					//	while (n1 < 0)
					//	{
					//		dw += 
					//		++n1;
					//		++pShadow;
					//	}
				}
				if (n2 > a_nSizeY)
					n2 = a_nSizeY;
				for (int i = n1; i < n2; ++i)
				{
					dw += pS[a_nSizeX*i]**pShadow;
					++pShadow;
				}
				*pL = dw>>8;
				++pL;
			}
			for (int nY = 0; nY < a_nSizeY; ++nY)
				pS[a_nSizeX*nY] = cLineY[nY];
		}
	}

	// compose blurred shadow and image
	if (a_eBlending == CFGVAL_BLEND_OUTER)
	{
		CGammaTables g; // sRGB

		DWORD *pS = aShadow;
		DWORD *pSEnd = pS+a_nSizeX*a_nSizeY;
		TRasterImagePixel *pD = a_pData;
		TRasterImagePixel const tShadowColor = {GetBValue(a_nColor), GetGValue(a_nColor), GetRValue(a_nColor), 0xff};
		struct TGammaPixel {WORD wR, wG, wB;};
		TGammaPixel const tShadowGamma = {g.m_aGamma[tShadowColor.bR], g.m_aGamma[tShadowColor.bG], g.m_aGamma[tShadowColor.bB]};
		if (a_pMask)
		{
			BYTE* pM = a_pMask;
			ULONG nMul = 0x80000000/((dwShadowSumX*dwShadowSumY)>>9);
			while (pS < pSEnd)
			{
				ULONG nA = (((nMul**pS)>>24)*nDensity)>>8;
				if (nA)
				{
					if (*pM == 0)
					{
						if (nA > 255)
						{
							*pD = tShadowColor;
						}
						else
						{
							ULONG nNewA = nA*255 + (255-nA)*pD->bA;
							ULONG const bA1 = (255-nA)*pD->bA;
							ULONG const bA2 = nA*255;
							pD->bR = g.InvGamma((tShadowGamma.wR*bA2 + g.m_aGamma[pD->bR]*bA1)/nNewA);
							pD->bG = g.InvGamma((tShadowGamma.wG*bA2 + g.m_aGamma[pD->bG]*bA1)/nNewA);
							pD->bB = g.InvGamma((tShadowGamma.wB*bA2 + g.m_aGamma[pD->bB]*bA1)/nNewA);
							pD->bA = nNewA/255;
						}
					}
					else if (*pM == 255)
					{
						if (pD->bA != 255)
						{
							if (nA > 255)
							{
								ULONG const bA1 = (255-pD->bA);
								ULONG const bA2 = pD->bA;
								pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/255);
								pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/255);
								pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/255);
								pD->bA = 255;
							}
							else
							{
								ULONG nNewA = pD->bA*255 + (255-pD->bA)*nA;
								ULONG const bA1 = (255-pD->bA)*nA;
								ULONG const bA2 = pD->bA*255;
								pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/nNewA);
								pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/nNewA);
								pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/nNewA);
								pD->bA = nNewA/255;
							}
						}
					}
					else
					{
						ULONG const nA2 = nA*(255-*pM)/255;
						ULONG const nA1 = nA-nA2;
						if (nA1 && pD->bA != 255)
						{
							if (nA1 > 255)
							{
								ULONG const bA1 = (255-pD->bA);
								ULONG const bA2 = pD->bA;
								pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/255);
								pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/255);
								pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/255);
								pD->bA = 255;
							}
							else
							{
								ULONG nNewA = pD->bA*255 + (255-pD->bA)*nA1;
								ULONG const bA1 = (255-pD->bA)*nA1;
								ULONG const bA2 = pD->bA*255;
								pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/nNewA);
								pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/nNewA);
								pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/nNewA);
								pD->bA = nNewA/255;
							}
						}
						if (nA2 > 255)
						{
							*pD = tShadowColor;
						}
						else if (nA2)
						{
							ULONG nNewA = nA2*255 + (255-nA2)*pD->bA;
							ULONG const bA1 = (255-nA2)*pD->bA;
							ULONG const bA2 = nA2*255;
							pD->bR = g.InvGamma((tShadowGamma.wR*bA2 + g.m_aGamma[pD->bR]*bA1)/nNewA);
							pD->bG = g.InvGamma((tShadowGamma.wG*bA2 + g.m_aGamma[pD->bG]*bA1)/nNewA);
							pD->bB = g.InvGamma((tShadowGamma.wB*bA2 + g.m_aGamma[pD->bB]*bA1)/nNewA);
							pD->bA = nNewA/255;
						}
					}
				}
				++pD;
				++pS;
				++pM;
			}
		}
		else
		{
			ULONG nMul = 0x80000000/((dwShadowSumX*dwShadowSumY)>>9);
			while (pS < pSEnd)
			{
				if (pD->bA != 255)
				{
					ULONG nA = (((nMul**pS)>>24)*nDensity)>>8;
					if (nA > 255)
					{
						ULONG const bA1 = (255-pD->bA);
						ULONG const bA2 = pD->bA;
						pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/255);
						pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/255);
						pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/255);
						pD->bA = 255;
					}
					else if (nA)
					{
						ULONG nNewA = pD->bA*255 + (255-pD->bA)*nA;
						ULONG const bA1 = (255-pD->bA)*nA;
						ULONG const bA2 = pD->bA*255;
						pD->bR = g.InvGamma((tShadowGamma.wR*bA1 + g.m_aGamma[pD->bR]*bA2)/nNewA);
						pD->bG = g.InvGamma((tShadowGamma.wG*bA1 + g.m_aGamma[pD->bG]*bA2)/nNewA);
						pD->bB = g.InvGamma((tShadowGamma.wB*bA1 + g.m_aGamma[pD->bB]*bA2)/nNewA);
						pD->bA = nNewA/255;
					}
				}
				++pD;
				++pS;
			}
		}
	}
	else if (a_eBlending == CFGVAL_BLEND_REPLACE)
	{
		TRasterImagePixel const tTransparent = {0, 0, 0, 0};
		DWORD *pS = aShadow;
		DWORD *pSEnd = pS+a_nSizeX*a_nSizeY;
		TRasterImagePixel *pD = a_pData;
		TRasterImagePixel const tShadowColor = {GetRValue(a_nColor), GetGValue(a_nColor), GetBValue(a_nColor), 0xff};
		ULONG nMul = 0x80000000/((dwShadowSumX*dwShadowSumY)>>9);
		while (pS < pSEnd)
		{
			ULONG nA = (((nMul**pS)>>24)*nDensity)>>8;
			if (nA == 0)
			{
				*pD = tTransparent;
			}
			else
			{
				*pD = tShadowColor;
				pD->bA = min(nA, 255);
			}
			++pD;
			++pS;
		}
	}
	else // CFGVAL_BLEND_INNER
	{
		CGammaTables g; // sRGB

		DWORD *pS = aShadow;
		DWORD *pSEnd = pS+a_nSizeX*a_nSizeY;
		TRasterImagePixel *pD = a_pData;
		TRasterImagePixel const tShadowColor = {GetBValue(a_nColor), GetGValue(a_nColor), GetRValue(a_nColor), 0xff};
		struct TGammaPixel {WORD wR, wG, wB;};
		TGammaPixel const tShadowGamma = {g.m_aGamma[tShadowColor.bR], g.m_aGamma[tShadowColor.bG], g.m_aGamma[tShadowColor.bB]};
		DWORD const shadowMax = dwShadowSumX*dwShadowSumY;
		if (a_pMask)
		{
			BYTE* pM = a_pMask;
			ULONG nMul = 0x80000000/(shadowMax>>9);
			while (pS < pSEnd)
			{
				if (pD->bA != 0)
				{
					LONG nA = ((256-((((nMul**pS)>>24)*nDensity)>>8))**pM)>>8;//(((nMul**pS)>>24)*nDensity**pM)>>16;
					if (nA > 255)
					{
						pD->bR = tShadowColor.bR;
						pD->bG = tShadowColor.bG;
						pD->bB = tShadowColor.bB;
					}
					else if (nA > 0)
					{
						pD->bR = g.InvGamma((tShadowGamma.wR*nA + g.m_aGamma[pD->bR]*(256-nA))>>8);
						pD->bG = g.InvGamma((tShadowGamma.wG*nA + g.m_aGamma[pD->bG]*(256-nA))>>8);
						pD->bB = g.InvGamma((tShadowGamma.wB*nA + g.m_aGamma[pD->bB]*(256-nA))>>8);
					}
				}
				++pD;
				++pS;
				++pM;
			}
		}
		else
		{
			ULONG nMul = 0x80000000/(shadowMax>>9);
			while (pS < pSEnd)
			{
				if (pD->bA != 0)
				{
					LONG nA = 256-((((nMul**pS)>>24)*nDensity)>>8);
					if (nA > 255)
					{
						pD->bR = tShadowColor.bR;
						pD->bG = tShadowColor.bG;
						pD->bB = tShadowColor.bB;
					}
					else if (nA > 0)
					{
						pD->bR = g.InvGamma((tShadowGamma.wR*nA + g.m_aGamma[pD->bR]*(256-nA))>>8);
						pD->bG = g.InvGamma((tShadowGamma.wG*nA + g.m_aGamma[pD->bG]*(256-nA))>>8);
						pD->bB = g.InvGamma((tShadowGamma.wB*nA + g.m_aGamma[pD->bB]*(256-nA))>>8);
					}
				}
				++pD;
				++pS;
			}
		}
	}
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		// read configuration values
		CConfigValue cVal;
		float fDensity;	a_pConfig->ItemValueGet(CComBSTR(CFGID_DENSITY), &cVal);fDensity = cVal.operator float()*0.01f;
		float fSize;	a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);	fSize = cVal;
		float fOffsetX;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETX), &cVal);fOffsetX = cVal;
		float fOffsetY;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETY), &cVal);fOffsetY = cVal;
		LONG  eType;	a_pConfig->ItemValueGet(CComBSTR(CFGID_TYPE), &cVal);	eType = cVal;
		LONG  nColor;	a_pConfig->ItemValueGet(CComBSTR(CFGID_COLOR), &cVal);	nColor = cVal;
		LONG  nBlend;	a_pConfig->ItemValueGet(CComBSTR(CFGID_BLENDING), &cVal);nBlend = cVal;

		int nXM = ceilf(fSize-fOffsetX); if (nXM < 0) nXM = 0;
		int nXP = ceilf(fSize+fOffsetX); if (nXP < 0) nXP = 0;
		int nYM = ceilf(fSize-fOffsetY); if (nYM < 0) nYM = 0;
		int nYP = ceilf(fSize+fOffsetY); if (nYP < 0) nYP = 0;

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		if (FAILED(pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL)))
			return E_FAIL;

		TRasterImageRect tRect = {{tOrigin.nX-nXM, tOrigin.nY-nYM}, {tOrigin.nX+tSize.nX+nXP, tOrigin.nY+tSize.nY+nYP}};
		if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
			return S_FALSE; // not a valid rectangle

		a_pConfig->ItemValueGet(CComBSTR(CFGID_MASKID), &cVal);
		CComPtr<ISharedStateImageSelection> pSel;
		if (a_pStates) a_pStates->StateGet(cVal, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pSel));
		CAutoVectorPtr<BYTE> cMask;
		if (pSel)
		{
			LONG nX = tRect.tTL.nX;
			LONG nY = tRect.tTL.nY;
			ULONG nDX = (tRect.tBR.nX-tRect.tTL.nX);
			ULONG nDY = (tRect.tBR.nY-tRect.tTL.nY);
			if (SUCCEEDED(pSel->Bounds(&nX, &nY, &nDX, &nDY)))
			{
				if (nX <= LONG(tRect.tTL.nX) && nY <= LONG(tRect.tTL.nY) &&
					nDX >= ULONG(tRect.tBR.nX-tRect.tTL.nX) && nDY >= ULONG(tRect.tBR.nY-tRect.tTL.nY) && S_OK == pSel->IsEmpty())
				{
					// everything is selected
				}
				else
				{
					nX = tRect.tTL.nX;
					nY = tRect.tTL.nY;
					nDX = (tRect.tBR.nX-tRect.tTL.nX);
					nDY = (tRect.tBR.nY-tRect.tTL.nY);
					cMask.Allocate(nDX*nDY);
					pSel->GetTile(nX, nY, nDX, nDY, nDX, cMask.m_p);
				}
			}
		}

		CWriteLock<IDocument> cLock(a_pDocument);
		ULONG nPixels = (tRect.tBR.nX-tRect.tTL.nX)*(tRect.tBR.nY-tRect.tTL.nY);
		CAutoVectorPtr<TRasterImagePixel> pSrc(new TRasterImagePixel[nPixels]);
		TImageSize const tRS = {tRect.tBR.nX-tRect.tTL.nX, tRect.tBR.nY-tRect.tTL.nY};
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tRS, NULL, nPixels, reinterpret_cast<TPixelChannel*>(pSrc.m_p), NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		DropShadow(pSrc.m_p, tRect.tBR.nX-tRect.tTL.nX, tRect.tBR.nY-tRect.tTL.nY, fSize, -fOffsetX, -fOffsetY, fDensity, eType == CFGVAL_TYPE_OUTLINE, nColor, cMask, nBlend);
		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tRS, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(pSrc.m_p), TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float fScaleX = 1.0f;
	float fScaleY = 1.0f;
	Matrix3x3fDecompose(*a_pContentTransform, &fScaleX, &fScaleY, NULL, NULL);
	float const f = sqrtf(fScaleX*fScaleY);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrSIZE(CFGID_SIZE);
	CComBSTR bstrOFFSETX(CFGID_OFFSETX);
	CComBSTR bstrOFFSETY(CFGID_OFFSETY);
	BSTR aIDs[] = {bstrSIZE, bstrOFFSETX, bstrOFFSETY};
	TConfigValue cVal[3];
	a_pConfig->ItemValueGet(bstrSIZE, &cVal[0]);
	a_pConfig->ItemValueGet(bstrOFFSETX, &cVal[1]);
	a_pConfig->ItemValueGet(bstrOFFSETY, &cVal[2]);
	cVal[0].fVal *= f;
	cVal[1].fVal *= fScaleX;
	cVal[2].fVal *= fScaleY;
	return a_pConfig->ItemValuesSet(3, aIDs, cVal);
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cVal;
		float fDensity;	a_pConfig->ItemValueGet(CComBSTR(CFGID_DENSITY), &cVal);fDensity = cVal.operator float()*0.01f;
		float fSize;	a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);	fSize = cVal;
		float fOffsetX;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETX), &cVal);fOffsetX = cVal;
		float fOffsetY;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETY), &cVal);fOffsetY = cVal;

		int nXM = ceilf(fSize-fOffsetX); if (nXM < 0) nXM = 0;
		int nXP = ceilf(fSize+fOffsetX); if (nXP < 0) nXP = 0;
		int nYM = ceilf(fSize-fOffsetY); if (nYM < 0) nYM = 0;
		int nYP = ceilf(fSize+fOffsetY); if (nYP < 0) nYP = 0;

		a_pRect->tTL.nX -= nXM;
		a_pRect->tTL.nY -= nYM;
		a_pRect->tBR.nX += nXP;
		a_pRect->tBR.nY += nYP;
	}
	return S_OK;
}

STDMETHODIMP CDocumentOperationRasterImageDropShadow::NeededToCompute(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cVal;
		float fDensity;	a_pConfig->ItemValueGet(CComBSTR(CFGID_DENSITY), &cVal);fDensity = cVal.operator float()*0.01f;
		float fSize;	a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);	fSize = cVal;
		float fOffsetX;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETX), &cVal);fOffsetX = cVal;
		float fOffsetY;	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSETY), &cVal);fOffsetY = cVal;

		int nXM = ceilf(fSize-fOffsetX); if (nXM < 0) nXM = 0;
		int nXP = ceilf(fSize+fOffsetX); if (nXP < 0) nXP = 0;
		int nYM = ceilf(fSize-fOffsetY); if (nYM < 0) nYM = 0;
		int nYP = ceilf(fSize+fOffsetY); if (nYP < 0) nYP = 0;

		a_pRect->tTL.nX -= nXP;
		a_pRect->tTL.nY -= nYP;
		a_pRect->tBR.nX += nXM;
		a_pRect->tBR.nY += nYM;
	}
	return S_OK;
}

