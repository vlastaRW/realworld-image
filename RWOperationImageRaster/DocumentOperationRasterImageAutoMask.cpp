// DocumentOperationRasterImageAutoMask.cpp : Implementation of CDocumentOperationRasterImageAutoMask

#include "stdafx.h"
#include "DocumentOperationRasterImageAutoMask.h"
#include <RWDocumentImageRaster.h>
#include <SharedStringTable.h>

#include "ConfigGUIMask.h"


// CDocumentOperationRasterImageAutoMask

STDMETHODIMP CDocumentOperationRasterImageAutoMask::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEAUTOMASK_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageAutoMask::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNCID), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELECTIONSYNCID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_OPERATION), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_OPERATION_DESC), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageAutoMask, CConfigGUIMaskDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageAutoMask::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;

		if (a_pConfig == NULL)
			return S_OK;

		CComBSTR bstr(CFGID_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);
		return a_pManager->CanActivate(a_pManager, a_pDocument, cVal, pCfg, a_pStates);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CMaskMixer :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IImageVisitor
{
public:
	void Init(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, BYTE const* a_pMask, TPixelChannel* a_pData)
	{
		m_nX = a_nX;
		m_nY = a_nY;
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		m_pMask = a_pMask;
		m_pData = a_pData;
	}

BEGIN_COM_MAP(CMaskMixer)
	COM_INTERFACE_ENTRY(IImageVisitor)
END_COM_MAP()

	// IImageVisitor methods
public:
	STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* a_pControl)
	{
		for (TImageTile const* const pEnd = a_aTiles+a_nTiles; a_aTiles < pEnd; ++a_aTiles)
		{
			TPixelChannel* pD = m_pData+(a_aTiles->tOrigin.nY-m_nY)*m_nSizeX+a_aTiles->tOrigin.nX-m_nX;
			BYTE const* pM = m_pMask+(a_aTiles->tOrigin.nY-m_nY)*m_nSizeX+a_aTiles->tOrigin.nX-m_nX;
			TPixelChannel const* pS = a_aTiles->pData;
			for (ULONG y = 0; y < a_aTiles->tSize.nY; ++y)
			{
				for (ULONG x = 0; x < a_aTiles->tSize.nX; ++x, ++pM, ++pD, pS+=a_aTiles->tStride.nX)
				{
					Mix(*pD, *pS, 255-*pM);
				}
				pM += m_nSizeX-a_aTiles->tSize.nX;
				pD += m_nSizeX-a_aTiles->tSize.nX;
				pS += a_aTiles->tStride.nY-a_aTiles->tSize.nX*a_aTiles->tStride.nX;
			}
		}
		return S_OK;
	}

private:
	static void Mix(TPixelChannel& a_tP1, TPixelChannel const& a_tP2, ULONG a_nCoverage)
	{
		ULONG const nA1 = a_tP1.bA*a_nCoverage;
		ULONG const nA2 = a_tP2.bA*(255-a_nCoverage);
		ULONG const nA = nA1+nA2;
		if (nA >= 255)
		{
			a_tP1.bR = (a_tP1.bR*nA1 + a_tP2.bR*nA2)/nA;
			a_tP1.bG = (a_tP1.bG*nA1 + a_tP2.bG*nA2)/nA;
			a_tP1.bB = (a_tP1.bB*nA1 + a_tP2.bB*nA2)/nA;
			a_tP1.bA = nA/255;
		}
		else
		{
			a_tP1.bR = a_tP1.bG = a_tP1.bB = a_tP1.bA = 0;
		}
	}

private:
	LONG m_nX;
	LONG m_nY;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	BYTE const* m_pMask;
	TPixelChannel* m_pData;
};

STDMETHODIMP CDocumentOperationRasterImageAutoMask::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComBSTR bstr(CFGID_OPERATION);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstr, &cVal);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstr, &pCfg);

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return a_pManager->Activate(a_pManager, a_pDocument, cVal, pCfg, a_pStates, a_hParent, a_tLocaleID);

		TImageSize tSize = {1, 1};
		if (FAILED(pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL)))
			return E_FAIL;

		CConfigValue cSelID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNCID), &cSelID);
		CComPtr<ISharedStateImageSelection> pMask;
		a_pStates->StateGet(cSelID, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pMask));
		bool bMasked = false;
		LONG nMaskX = 0;
		LONG nMaskY = 0;
		ULONG nMaskDX = 0;
		ULONG nMaskDY = 0;
		if (pMask)
		{
			if (SUCCEEDED(pMask->Bounds(&nMaskX, &nMaskY, &nMaskDX, &nMaskDY)) &&
				(pMask->IsEmpty() != S_OK || nMaskX > 0 || nMaskY > 0 || nMaskDX < tSize.nX || nMaskDY < tSize.nY))
				bMasked = true;
		}

		if (!bMasked)
			return a_pManager->Activate(a_pManager, a_pDocument, cVal, pCfg, a_pStates, a_hParent, a_tLocaleID);

		CAutoVectorPtr<BYTE> cMask;
		if (pMask->IsEmpty() != S_OK)
		{
			cMask.Allocate(nMaskDX*nMaskDY);
			pMask->GetTile(nMaskX, nMaskY, nMaskDX, nMaskDY, nMaskDX, cMask);
		}
		//if (nScale > 1)
		//{
		//	ULONG nMaskX2 = (nMaskX+nScale-1)/nScale;
		//	ULONG nMaskY2 = (nMaskY+nScale-1)/nScale;
		//	ULONG nMaskDX2 = (nMaskX+nMaskDX)/nScale-nMaskX2;
		//	ULONG nMaskDY2 = (nMaskY+nMaskDY)/nScale-nMaskY2;
		//	if (cMask.m_p)
		//	{
		//		if (nMaskDX2 == 0 || nMaskDX2 > 0x80000000 || nMaskDY2 == 0 || nMaskDY2 > 0x80000000)
		//		{
		//			nMaskDX2 = nMaskDY2 = 0;
		//			cMask.Free();
		//		}
		//		else
		//		{
		//			CAutoVectorPtr<BYTE> cMask2(new BYTE[nMaskDX2*nMaskDY2]);
		//			BYTE* pS = cMask;
		//			BYTE* pD = cMask2;
		//			for (ULONG yy = 0; yy < nMaskDY2; ++yy)
		//			{
		//				for (ULONG xx = 0; xx < nMaskDX2; ++xx)
		//				{
		//					*pD = *pS;
		//					++pD;
		//					pS += nScale;
		//				}
		//				pS += (nMaskDX-nMaskDX2)*nScale;
		//			}
		//			std::swap(cMask.m_p, cMask2.m_p);
		//		}
		//	}
		//	nMaskX = nMaskX2;
		//	nMaskY = nMaskY2;
		//	nMaskDX = nMaskDX2;
		//	nMaskDY = nMaskDY2;
		//}

		CComPtr<IDocument> pCopy;
		RWCoCreateInstance(pCopy, __uuidof(DocumentBase));
		a_pDocument->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pCopy), NULL, NULL);
		HRESULT hRes = a_pManager->Activate(a_pManager, pCopy, cVal, pCfg, a_pStates, a_hParent, a_tLocaleID);
		if (FAILED(hRes)) return hRes;
		CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[nMaskDX*nMaskDY]);
		CComPtr<IDocumentImage> pRICopy;
		pCopy->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pRICopy));
		if (cMask.m_p)
		{
			pRI->TileGet(EICIRGBA, CImagePoint(nMaskX, nMaskY), CImageSize(nMaskDX, nMaskDY), NULL, nMaskDX*nMaskDY, cBuffer, NULL, EIRIAccurate);
			CComObjectStackEx<CMaskMixer> cMixer;
			cMixer.Init(nMaskX, nMaskY, nMaskDX, nMaskDY, cMask, cBuffer);
			pRICopy->Inspect(EICIRGBA, CImagePoint(nMaskX, nMaskY), CImageSize(nMaskDX, nMaskDY), &cMixer, NULL, EIRIAccurate);
		}
		else
		{
			pRICopy->TileGet(EICIRGBA, CImagePoint(nMaskX, nMaskY), CImageSize(nMaskDX, nMaskDY), NULL, nMaskDX*nMaskDY, cBuffer, NULL, EIRIAccurate);
		}
		return pRI->TileSet(EICIRGBA, CImagePoint(nMaskX, nMaskY), CImageSize(nMaskDX, nMaskDY), NULL, nMaskDX*nMaskDY, cBuffer, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

