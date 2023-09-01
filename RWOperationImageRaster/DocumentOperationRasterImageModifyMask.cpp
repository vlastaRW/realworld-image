// DocumentOperationRasterImageModifyMask.cpp : Implementation of CDocumentOperationRasterImageModifyMask

#include "stdafx.h"
#include "DocumentOperationRasterImageModifyMask.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

#include "ConfigGUIMask.h"


// CDocumentOperationRasterImageModifyMask

STDMETHODIMP CDocumentOperationRasterImageModifyMask::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEMODIFYMASK_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageModifyMask::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNCID), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_OPERATION), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageModifyMask, CConfigGUIMaskDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageModifyMask::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* UNREF(a_pDocument), IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNCID), &cID);
		CComPtr<ISharedStateImageSelection> pState;
		a_pStates->StateGet(cID, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pState));

		return pState ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

template<BYTE t_nVal>
static bool IsRowConstant(TPixelChannel const* p, ULONG a_nLen)
{
	TPixelChannel const* const pEnd = p+a_nLen;
	while (p < pEnd)
	{
		if (t_nVal != p->bR)
			return false;
		++p;
	}
	return true;
}

template<BYTE t_nVal>
static bool IsColumnConstant(TPixelChannel const* p, ULONG a_nLen, ULONG a_nStride)
{
	TPixelChannel const* const pEnd = p+a_nLen*a_nStride;
	while (p < pEnd)
	{
		if (t_nVal != p->bR)
			return false;
		p += a_nStride;
	}
	return true;
}

STDMETHODIMP CDocumentOperationRasterImageModifyMask::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentImage> pI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		if (pI == NULL)
			return E_FAIL;

		TImageSize tSize = {0, 0};
		if (FAILED(pI->CanvasGet(&tSize, NULL, NULL, NULL, NULL)))
			return E_FAIL;

		CAutoVectorPtr<TPixelChannel> aMask;
		TPixelChannel tTransparent = {0, 0, 0, 0};

		CConfigValue cID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNCID), &cID);
		CComPtr<ISharedStateImageSelection> pState;
		a_pStates->StateGet(cID, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pState));
		TImagePoint tOrg = {0, 0};
		TImageSize tSz = {0, 0};
		if (pState)
		{
			pState->Bounds(&tOrg.nX, &tOrg.nY, &tSz.nX, &tSz.nY);
			if (tSz.nX*tSz.nY)
			{
				aMask.Attach(new TPixelChannel[tSz.nX*tSz.nY]);
				BYTE* pS = reinterpret_cast<BYTE*>(aMask.m_p)+3*tSz.nX*tSz.nY;
				pState->GetTile(tOrg.nX, tOrg.nY, tSz.nX, tSz.nY, tSz.nX, pS);
				TPixelChannel* pD = aMask.m_p;
				for (ULONG y = 0; y < tSz.nY; ++y)
				{
					for (ULONG x = 0; x < tSz.nX; ++x)
					{
						pD->bR = pD->bG = pD->bB = 0;
						pD->bA = *pS;
						++pS;
						++pD;
					}
				}
			}
		}

		CComPtr<IDocumentFactoryRasterImage> pDF;
		RWCoCreateInstance(pDF, __uuidof(DocumentFactoryRasterImage));
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		TImageTile t = {EICIRGBA, tOrg, tSz, {1, tSz.nX}, tSz.nX*tSz.nY, aMask};
		if (FAILED(pDF->Create(NULL, pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, tTransparent), 1.0f, aMask.m_p ? &t : NULL)))
			return E_FAIL;
		aMask.Free();
		CComQIPtr<IDocument> pDoc(pBase);

		CComBSTR bstr(CFGID_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		HRESULT hRes = a_pManager->Activate(a_pManager, pDoc, cVal, pCfg, a_pStates, a_hParent, a_tLocaleID);
		if (FAILED(hRes))
			return hRes;
		CComPtr<IDocumentRasterImage> pRI2;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI2));
		pRI2->CanvasGet(NULL, NULL, &tOrg, &tSz, NULL);
		if (tSz.nX*tSz.nY)
		{
			aMask.Attach(new TPixelChannel[tSz.nX*tSz.nY]);
			pRI2->TileGet(EICIRGBA, &tOrg, &tSz, NULL, tSz.nX*tSz.nY, aMask, NULL, EIRIAccurate);
		}

		CComPtr<ISharedStateImageSelection> pS;
		RWCoCreateInstance(pS, __uuidof(SharedStateImageSelection));
		if (tSz.nX*tSz.nY)
		{
			BYTE* p = reinterpret_cast<BYTE*>(aMask.m_p);
			BYTE* pD = p;
			TPixelChannel* pSrc = aMask.m_p;
			for (ULONG y = 0; y < tSz.nY; ++y)
			{
				for (ULONG x = 0; x < tSz.nX; ++x)
				{
					*pD = pSrc->bA;
					++pD;
					++pSrc;
				}
			}
			pS->Init(tOrg.nX, tOrg.nY, tSz.nX, tSz.nY, tSz.nX, p);
		}
		else
		{
			pS->Init(0, 0, tSz.nX, tSz.nY, 0, NULL);
		}
		return a_pStates->StateSet(cID, pS);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

