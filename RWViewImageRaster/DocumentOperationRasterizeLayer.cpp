// DocumentOperationRasterizeLayer.cpp : Implementation of CDocumentOperationRasterizeLayer

#include "stdafx.h"
#include "DocumentOperationRasterizeLayer.h"

#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <SharedStateUndo.h>
#include "ConfigGUILayerID.h"

static wchar_t const CFGID_TRANSFORMATION[] = L"Transform";

// CDocumentOperationRasterizeLayer

STDMETHODIMP CDocumentOperationRasterizeLayer::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Rasterize Layer[0405]Vrstvený obrázek - rasterizovat vrstvu");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterizeLayer::ConfigCreate(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"LAYER"), NULL, 0, NULL);
		CComPtr<ITransformationManager> pTM;
		a_pManager->QueryInterface(&pTM);
		if (pTM == NULL) RWCoCreateInstance(pTM, __uuidof(TransformationManager));
		pTM->InsertIntoConfigAs(pTM, pCfg, CComBSTR(CFGID_TRANSFORMATION), CMultiLanguageString::GetAuto(L"[0490]Transformation[0405]Transformace"), CMultiLanguageString::GetAuto(L"[0490]Transformation applied to the selected layer.[0405]Transformace aplikovaná na vybranou vrstvu."), 0, NULL);
		pCfg->Finalize(NULL);
		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterizeLayer::CanActivate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;
		CComPtr<IDocumentLayeredImage> pDLI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
		if (pDLI == NULL)
			return S_FALSE;
		if (a_pConfig == NULL)
			return S_OK;

		CComPtr<ITransformationManager> pTM;
		a_pManager->QueryInterface(&pTM);
		if (pTM == NULL) RWCoCreateInstance(pTM, __uuidof(TransformationManager));
		CComBSTR bstrXform(CFGID_TRANSFORMATION);
		CConfigValue cID;
		a_pConfig->ItemValueGet(bstrXform, &cID);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstrXform, &pCfg);

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);
		CComBSTR bstrID;
		pDLI->StatePrefix(&bstrID);
		if (bstrID.Length())
		{
			bstrID += cVal;
		}
		else
		{
			bstrID.Attach(cVal.Detach().bstrVal);
		}

		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		if (pState) pDLI->StateUnpack(pState, &pItems);
		ULONG nItems = 0;
		if (pItems) pItems->Size(&nItems);
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComPtr<IComparable> pItem;
			pItems->Get(i, &pItem);
			CComPtr<ISubDocumentID> pSDID;
			pDLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pDoc;
			if (pSDID) pSDID->SubDocumentGet(&pDoc);
			if (pTM->CanActivate(pTM, pDoc, cID, pCfg, a_pStates) == S_OK)
				return S_OK;
			//GUID tID = GUID_NULL;
			//if (pDoc) pDoc->BuilderID(&tID);
			//if (!IsEqualGUID(tID, __uuidof(DocumentFactoryRasterImage)))
			//	return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CImageLayerCreatorTransformation :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorTransformation(ITransformationManager* a_pTM, TConfigValue const* a_pID, IConfig* a_pCfg, IDocument* a_pDoc, IOperationContext* a_pCtx, RWHWND a_hParent, LCID a_tLocaleID) :
		m_pTM(a_pTM), m_pID(a_pID), m_pCfg(a_pCfg), m_pDoc(a_pDoc), m_pCtx(a_pCtx), m_hParent(a_hParent), m_tLocaleID(a_tLocaleID)
	{
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		return m_pTM->Activate(m_pTM, m_pDoc, m_pID, m_pCfg, m_pCtx, m_hParent, m_tLocaleID, a_bstrID, a_pBase);
	}

private:
	ITransformationManager* m_pTM;
	TConfigValue const* m_pID;
	IConfig* m_pCfg;
	IDocument* m_pDoc;
	IOperationContext* m_pCtx;
	RWHWND m_hParent;
	LCID m_tLocaleID;
};

STDMETHODIMP CDocumentOperationRasterizeLayer::Activate(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentLayeredImage> pDLI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
		if (pDLI == NULL)
			return E_FAIL;

		CWriteLock<IDocument> cLock(a_pDocument);

		CComPtr<ITransformationManager> pTM;
		a_pManager->QueryInterface(&pTM);
		if (pTM == NULL) RWCoCreateInstance(pTM, __uuidof(TransformationManager));
		CComBSTR bstrXform(CFGID_TRANSFORMATION);
		CConfigValue cID;
		a_pConfig->ItemValueGet(bstrXform, &cID);
		CComPtr<IConfig> pCfg;
		a_pConfig->SubConfigGet(bstrXform, &pCfg);

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);
		CComBSTR bstrID;
		pDLI->StatePrefix(&bstrID);
		if (bstrID.Length())
		{
			bstrID += cVal;
		}
		else
		{
			bstrID.Attach(cVal.Detach().bstrVal);
		}

		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pItems;
		if (pState) pDLI->StateUnpack(pState, &pItems);
		ULONG nItems = 0;
		if (pItems) pItems->Size(&nItems);
		std::vector<CComPtr<IComparable> > aNewSel2;
		std::vector<IComparable*> aNewSel;
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComPtr<IComparable> pItem;
			pItems->Get(i, &pItem);
			CComPtr<ISubDocumentID> pSDID;
			pDLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
			CComPtr<IDocument> pDoc;
			if (pSDID) pSDID->SubDocumentGet(&pDoc);
			//GUID tID = GUID_NULL;
			//if (pDoc) pDoc->BuilderID(&tID);
			//if (!IsEqualGUID(tID, __uuidof(DocumentFactoryRasterImage)))
			if (pTM->CanActivate(pTM, pDoc, cID, pCfg, a_pStates) == S_OK)
			{
				//CComPtr<IDocumentImage> pDocImg;
				//pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
				//TImageSize tSize = {0, 0};
				//TImagePoint tOrg = {0, 0};
				//TImageSize tCont = {0, 0};
				//TImageResolution tRes = {100, 254, 100, 254};
				//pDocImg->CanvasGet(&tSize, &tRes, &tOrg, &tCont, NULL);
				//float fGamma = 1.0f;
				//TPixelChannel tDef;
				//tDef.n = 0;
				//pDocImg->ChannelsGet(NULL, &fGamma, &CImageChannelDefaultGetter(EICIRGBA, &tDef));
				//CAutoVectorPtr<TPixelChannel> pBuffer(tCont.nX*tCont.nY ? new TPixelChannel[tCont.nX*tCont.nY] : NULL);
				//pDocImg->TileGet(EICIRGBA, &tOrg, &tCont, NULL, tCont.nX*tCont.nY, pBuffer, NULL, EIRIAccurate);

				CComPtr<IComparable> pNew;
				//pDLI->LayerInsert(pItem, &CImageLayerCreatorRasterImage(tSize, &tRes, fGamma, tDef, pBuffer, &tOrg, &tCont, NULL), &pNew);
				HRESULT hRes = pDLI->LayerInsert(pItem, ELIPBelow, &CImageLayerCreatorTransformation(pTM, cID, pCfg, pDoc, a_pStates, a_hParent, a_tLocaleID), &pNew);
				if (FAILED(hRes)) return hRes;

				CComPtr<IConfig> pCfg;
				pDLI->LayerEffectGet(pItem, &pCfg, NULL);
				if (pCfg)
					pDLI->LayerEffectSet(pNew, pCfg);
				CComBSTR bstrName;
				pDLI->LayerNameGet(pItem, &bstrName);
				if (bstrName.m_str)
					pDLI->LayerNameSet(pNew, bstrName);
				ELayerBlend eBlend = EBEAlphaBlend;
				BYTE bVisible = TRUE;
				pDLI->LayerPropsGet(pItem, &eBlend, &bVisible);
				pDLI->LayerPropsSet(pNew, &eBlend, &bVisible);
				pDLI->LayerDelete(pItem);
				aNewSel2.push_back(pNew);
				aNewSel.push_back(pNew);
			}
		}
		if (!aNewSel.empty())
		{
			CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrID, pState);
			pState = NULL;
			pDLI->StatePack(aNewSel.size(), &aNewSel[0], &pState);
			a_pStates->StateSet(bstrID, pState);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

