// DocumentOperationRasterImageGreyscale.cpp : Implementation of CDocumentOperationRasterImageGreyscale

#include "stdafx.h"
#include "DocumentOperationRasterImageGreyscale.h"
#include <RWDocumentImageRaster.h>
#include <RWViewImageRaster.h>
#include <SharedStringTable.h>

const OLECHAR CFGID_COLOR[] = L"Color";

#include "ConfigGUIGreyscale.h"

// CDocumentOperationRasterImageGreyscale

STDMETHODIMP CDocumentOperationRasterImageGreyscale::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEGRAYSCALE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRasterImageGreyscale::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Grayscale[0405]Šedotón";
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

struct SGrayScaleRenderer : IStockIcons::ILayerReceiver
{
	SGrayScaleRenderer(IStockIcons::ILayerReceiver* base) : base(*base) {}
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
		float g = (CGammaTables::FromSRGB(GetRValue(color))+CGammaTables::FromSRGB(GetGValue(color))+CGammaTables::FromSRGB(GetBValue(color)))/3;
		ULONG n = CGammaTables::ToSRGB(g);
		return (color&0xff000000)|(n<<16)|(n<<8)|n;
	}
	IStockIcons::ILayerReceiver& base;
};

HICON CDocumentOperationRasterImageGreyscale::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	SGrayScaleRenderer cGSRenderer(&cRenderer);
	pSI->GetLayers(ESIPicture, cGSRenderer);
	return cRenderer.get();
}

STDMETHODIMP CDocumentOperationRasterImageGreyscale::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_COLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_COLOR_DESC), CConfigValue((LONG)0xffffff), NULL, 0, NULL);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageGreyscale, CConfigGUIGreyscale>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageGreyscale::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

#include <GammaCorrection.h>
#include "../RWOperationImageRaster/PixelLevelTask.h"

struct CGreyscaleOp
{
	CGreyscaleOp(BYTE bR, BYTE bG, BYTE bB, TPixelChannel tDefault, CGammaTables const* pGT) : pGT(pGT), nTotalA(0)
	{
		if (pGT)
		{
			ULONG const nDiv = pGT->m_aGamma[bR] + pGT->m_aGamma[bG] + pGT->m_aGamma[bB];
			nR = ULONG(0x10000*pGT->m_aGamma[bR]) / nDiv;
			nG = ULONG(0x10000*pGT->m_aGamma[bG]) / nDiv;
			nB = ULONG(0x10000*pGT->m_aGamma[bB]) / nDiv;
		}
		else
		{
			ULONG const nDiv = ULONG(bR)+ULONG(bG)+ULONG(bB);
			nR = ULONG(0x1000000*bR) / nDiv;
			nG = ULONG(0x1000000*bG) / nDiv;
			nB = ULONG(0x1000000*bB) / nDiv;
		}
		Process(&tDef, &tDefault, 1, 1);
		nTotalA = 0;
	}

	LONGLONG nTotalA;
	TPixelChannel tDef;
	CGammaTables const* pGT;
	ULONG nR;
	ULONG nG;
	ULONG nB;
	void Process(TPixelChannel* __restrict pD, TPixelChannel const* __restrict pS, size_t const s, size_t const n)
	{
		ULONG const nR = this->nR;
		ULONG const nG = this->nG;
		ULONG const nB = this->nB;
		LONGLONG nTA = 0;
		if (pGT)
		{
			CGammaTables const* pGT = this->pGT;
			for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
			{
				ULONG n = pS->n;
				ULONG nSA = n&0xff000000;
				ULONG nSR = (n&0xff0000)>>16;
				ULONG nSG = (n&0xff00)>>8;
				ULONG nSB = n&0xff;
				nTA += nSA;
				//if (nSA == 0)
				//{
				//	pD->n = 0;
				//	continue;
				//}
				ULONG const l = pGT->InvGamma((pGT->m_aGamma[nSR]*nR + pGT->m_aGamma[nSG]*nG + pGT->m_aGamma[nSB]*nB + 0x8000)>>16)*0x10101;
				pD->n = l|nSA;
			}
		}
		else
		{
			for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
			{
				ULONG n = pS->n;
				ULONG nSA = n&0xff000000;
				ULONG nSR = (n&0xff0000)>>16;
				ULONG nSG = (n&0xff00)>>8;
				ULONG nSB = pS->n&0xff;
				nTA += nSA;
				//if (nSA == 0)
				//{
				//	pD->n = 0;
				//	continue;
				//}
				ULONG const l = ((nSR*nR + nSG*nG + nSB*nB + 0x800000)>>24)*0x10101;
				pD->n = l|nSA;
			}
		}
		nTA >>= 24;
		InterlockedAdd64(&nTotalA, nTA);
	}
};

STDMETHODIMP CDocumentOperationRasterImageGreyscale::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TImageSize tSize = {1, 1};
		TImagePoint tOrig = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
		TRasterImageRect tR = {tOrig, {tOrig.nX+tSize.nX, tOrig.nY+tSize.nY}};
		ULONG const nPixels = tSize.nX*tSize.nY;
		TPixelChannel tDefault;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

		CConfigValue cColor;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_COLOR), &cColor);
		DWORD dwColor = cColor.operator LONG();

		BYTE bR = dwColor & 0xff;
		BYTE bG = (dwColor >> 8) & 0xff;
		BYTE bB = (dwColor >> 16) & 0xff;
		NormalizeRGB(bR, bG, bB);

		CComPtr<IGammaTableCache> pGTC;
		RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
		CGreyscaleOp cOp(bR, bG, bB, tDefault, pGTC ? pGTC->GetSRGBTable() : NULL);

		if (nPixels == 0)
		{
			EImageChannelID e = EICIRGBA;
			pRI->ChannelsSet(1, &e, &cOp.tDef);
			return S_FALSE;
		}

		CAutoPixelBuffer cBuf(pRI, tSize);
		//CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);

		{
			CComPtr<IThreadPool> pThPool;
			if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
				RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

			CComObjectStackEx<CPixelLevelTask<CGreyscaleOp> > cXform;
			cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		ULONGLONG totalA = cOp.nTotalA;
		return cBuf.Replace(tR.tTL, &tR.tTL, &tSize, &totalA, cOp.tDef);
		//return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, nPixels, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
