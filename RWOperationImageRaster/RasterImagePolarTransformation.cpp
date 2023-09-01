// RasterImagePolarTransformation.cpp : Implementation of CRasterImagePolarTransformation

#include "stdafx.h"
#include "RasterImagePolarTransformation.h"
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>
#include <math.h>

const OLECHAR CFGID_PT_DIRECTION[] = L"Direction";
const LONG CFGVAL_PTDIR_TOPOLAR = 1;
const LONG CFGVAL_PTDIR_TORECTANGULAR = -1;


// CRasterImagePolarTransformation

STDMETHODIMP CRasterImagePolarTransformation::NameGet(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEPOLAR_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CRasterImagePolarTransformation::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Polar transformation[0405]Polární transformace";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cDir;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT_DIRECTION), &cDir);
		*a_ppName = new CMultiLanguageString(cDir.operator LONG() == CFGVAL_PTDIR_TOPOLAR ?
			L"[0409]Cartesian to polar[0405]Kartézske na polární":
			L"[0409]Polar to cartesian[0405]Polární na kartézské");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}


STDMETHODIMP CRasterImagePolarTransformation::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_PT_DIRECTION(CFGID_PT_DIRECTION);
		pCfgInit->ItemIns1ofN(cCFGID_PT_DIRECTION, _SharedStringTable.GetStringAuto(IDS_CFGID_PT_DIRECTION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PT_DIRECTION_DESC), CConfigValue(CFGVAL_PTDIR_TOPOLAR), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_PT_DIRECTION, CConfigValue(CFGVAL_PTDIR_TOPOLAR), _SharedStringTable.GetStringAuto(IDS_CFGVAL_PTDIR_TOPOLAR), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_PT_DIRECTION, CConfigValue(CFGVAL_PTDIR_TORECTANGULAR), _SharedStringTable.GetStringAuto(IDS_CFGVAL_PTDIR_TORECTANGULAR), 0, NULL);

		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&CLSID_DocumentOperationRasterImageDropShadow, CConfigGUIDropShadow>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImagePolarTransformation::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
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

STDMETHODIMP CRasterImagePolarTransformation::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
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
		if (tSize.nX <= 1 || tSize.nY <= 1)
			return S_FALSE; // image too small

		CConfigValue cDirection;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT_DIRECTION), &cDirection);

		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[tSize.nX*tSize.nY]);
		CAutoVectorPtr<TPixelChannel> pFrame(new TPixelChannel[tSize.nX*tSize.nY]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		float const fCenterX = tSize.nX*0.5f;
		float const fCenterY = tSize.nY*0.5f;
		float const fRelX = fCenterX-0.5f;
		float const fRelY = fCenterY-0.5f;
		float const fRadius = min(fRelX, fRelY);//fCenterX > fCenterY ? fCenterX : fCenterY;
		float const fScaleX = tSize.nX > tSize.nY ? (tSize.nY-1)/float(tSize.nX-1) : 1.0f;
		float const fScaleY = tSize.nY > tSize.nX ? (tSize.nX-1)/float(tSize.nY-1) : 1.0f;
		TPixelChannel* const p = pSrc.m_p;
		TPixelChannel* pD = pFrame;
		size_t const nLine = tSize.nX;
		for (ULONG nY = 0; nY < tSize.nY; ++nY)
		{
			for (ULONG nX = 0; nX < tSize.nX; ++nX, ++pD)
			{
				if (cDirection.operator LONG() == CFGVAL_PTDIR_TOPOLAR)
				{
					float const x = (nX-fRelX)*fScaleX;
					float const y = (nY-fRelY)*fScaleY;
					float const fX = (0.5f+atan2f(x, y)/(3.14159265f*2.0f))*tSize.nX;
					float const fY = sqrtf(x*x + y*y)*(tSize.nY-1)/fRadius;
					int const xx = (fX/*-0.5f*/)*256;
					int const x1 = ((xx>>8)+tSize.nX)%tSize.nX;
					int const x2 = (x1+1)%tSize.nX;
					int const xw = xx&255;
					int const yy = (fY/*-0.5f*/)*256;
					int const y1 = min(int(tSize.nY-1), yy>>8);
					int const y2 = min(int(tSize.nY-1), y1+1);
					int const yw = yy&255;
					int const w11 = (255-xw)*(255-yw);
					int const w12 = (255-xw)*yw;
					int const w21 = xw*(255-yw);
					int const w22 = xw*yw;
					TPixelChannel const* const p11 = p+nLine*y1+x1;
					TPixelChannel const* const p12 = p+nLine*y2+x1;
					TPixelChannel const* const p21 = p+nLine*y1+x2;
					TPixelChannel const* const p22 = p+nLine*y2+x2;
					if (bHasAlpha)
					{
						ULONG nA = (p11->bA*w11 + p12->bA*w12 + p21->bA*w21 + p22->bA*w22);
						if (nA > 65025)
						{
							pD->bA = nA/65025;
							pD->bR = (p11->bR*w11*p11->bA + p12->bR*w12*p12->bA + p21->bR*w21*p21->bA + p22->bR*w22*p22->bA)/nA;
							pD->bG = (p11->bG*w11*p11->bA + p12->bG*w12*p12->bA + p21->bG*w21*p21->bA + p22->bG*w22*p22->bA)/nA;
							pD->bB = (p11->bB*w11*p11->bA + p12->bB*w12*p12->bA + p21->bB*w21*p21->bA + p22->bB*w22*p22->bA)/nA;
						}
						else
						{
							pD->n = 0;
						}
					}
					else
					{
						pD->bA = 255;
						pD->bR = (p11->bR*w11 + p12->bR*w12 + p21->bR*w21 + p22->bR*w22)/65025;
						pD->bG = (p11->bG*w11 + p12->bG*w12 + p21->bG*w21 + p22->bG*w22)/65025;
						pD->bB = (p11->bB*w11 + p12->bB*w12 + p21->bB*w21 + p22->bB*w22)/65025;
					}
				}
				else
				{
					float const fX = fCenterX + sinf((nX-fRelX)*3.14159265f*2.0f/tSize.nX)*(nY+0.5f)/fRadius*float(tSize.nX-1)*0.25f;
					float const fY = fCenterY + cosf((nX-fRelX)*3.14159265f*2.0f/tSize.nX)*(nY+0.5f)/fRadius*float(tSize.nY-1)*0.25f;

					int const xx = (fX/*-0.5f*/)*256;
					int const x1 = ((xx>>8)+tSize.nX)%tSize.nX;
					int const x2 = (x1+1)%tSize.nX;
					int const xw = xx&255;
					int const yy = (fY/*-0.5f*/)*256;
					int const y1 = min(int(tSize.nY-1), yy>>8);
					int const y2 = min(int(tSize.nY-1), y1+1);
					int const yw = yy&255;
					int const w11 = (255-xw)*(255-yw);
					int const w12 = (255-xw)*yw;
					int const w21 = xw*(255-yw);
					int const w22 = xw*yw;
					TPixelChannel const* const p11 = p+nLine*y1+x1;
					TPixelChannel const* const p12 = p+nLine*y2+x1;
					TPixelChannel const* const p21 = p+nLine*y1+x2;
					TPixelChannel const* const p22 = p+nLine*y2+x2;
					if (bHasAlpha)
					{
						ULONG nA = (p11->bA*w11 + p12->bA*w12 + p21->bA*w21 + p22->bA*w22);
						if (nA >= 65025)
						{
							pD->bA = nA/65025;
							pD->bR = (p11->bR*w11*p11->bA + p12->bR*w12*p12->bA + p21->bR*w21*p21->bA + p22->bR*w22*p22->bA)/nA;
							pD->bG = (p11->bG*w11*p11->bA + p12->bG*w12*p12->bA + p21->bG*w21*p21->bA + p22->bG*w22*p22->bA)/nA;
							pD->bB = (p11->bB*w11*p11->bA + p12->bB*w12*p12->bA + p21->bB*w21*p21->bA + p22->bB*w22*p22->bA)/nA;
						}
						else
						{
							pD->n = 0;
						}
					}
					else
					{
						pD->bA = 255;
						pD->bR = (p11->bR*w11 + p12->bR*w12 + p21->bR*w21 + p22->bR*w22)/65025;
						pD->bG = (p11->bG*w11 + p12->bG*w12 + p21->bG*w21 + p22->bG*w22)/65025;
						pD->bB = (p11->bB*w11 + p12->bB*w12 + p21->bB*w21 + p22->bB*w22)/65025;
					}
				}
			}
		}
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, pFrame.m_p, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CRasterImagePolarTransformation::GetDefaultIcon(ULONG a_nSize)
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
	IRPathPoint const axis[] =
	{
		{9, 205, 0, 0, -11.7157, 11.7157},
		{205, 9, 11.7157, -11.7157, 0, 0},
		{247, 9, 11.7157, 11.7157, -11.7157, -11.7157},
		{247, 51, 0, 0, 11.7157, -11.7157},
		{51, 247, -11.7157, 11.7157, 0, 0},
		{9, 247, -11.7157, -11.7157, 11.7157, 11.7157},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	IRFill matWhite(0x7fffffff);
	IRFill matBlack(0x7f000000);
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(axis), axis, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas, itemsof(highlight), highlight, &matWhite);
	cRenderer(&canvas, itemsof(shade), shade, &matBlack);
	return cRenderer.get();
}
