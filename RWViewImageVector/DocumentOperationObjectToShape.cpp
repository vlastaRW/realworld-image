// DocumentOperationObjectToShape.cpp : Implementation of CDocumentOperationObjectToShape

#include "stdafx.h"
#include "DocumentOperationObjectToShape.h"
#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_TARGETTOOL[] = L"TargetTool";

// CDocumentOperationObjectToShape

STDMETHODIMP CDocumentOperationObjectToShape::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Convert to Shape[0405]Vektorový obrázek - převézt na Tvar");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationObjectToShape::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
		pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TARGETTOOL), CMultiLanguageString::GetAuto(L"[0409]Convert to[0405]Konvertovat na"), NULL, CConfigValue(L"SHAPE"), NULL, 0, NULL);

		// finalize the initialization of the config
		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationObjectToShape::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI == NULL)
			return S_FALSE;
		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CComBSTR bstrState;
		pDVI->StatePrefix(&bstrState);
		bstrState.Append(cSyncID.operator BSTR());
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> aSelected;
		pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		if (aSelected.size() != 1)
			return S_FALSE;
		CComBSTR bstrToolID;
		pDVI->ObjectGet(aSelected[0], &bstrToolID, NULL);
		CConfigValue cTargetTool;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TARGETTOOL), &cTargetTool);
		return bstrToolID != cTargetTool.operator BSTR() ? S_OK : S_FALSE;
		//// TODO: use actual tool properties when tools are redesigned
		//return bstrToolID.Length() && bstrToolID != L"LINE" && bstrToolID != L"CURVE" && bstrToolID != L"SHAPE" ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationObjectToShape::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI == NULL)
			return E_FAIL;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CComBSTR bstrState;
		pDVI->StatePrefix(&bstrState);
		bstrState.Append(cSyncID.operator BSTR());
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> aSelected;
		pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		if (aSelected.size() != 1)
			return E_FAIL;

		CComBSTR bstrPathID(L"SHAPE");
		CConfigValue cTargetTool;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TARGETTOOL), &cTargetTool);
		bool toPoly = false;
		if (cTargetTool.operator ==(L"POLYGON"))
		{
			bstrPathID = L"POLYGON";
			toPoly = true;
		}

		CWriteLock<IBlockOperations> cLock(a_pDocument);

		ULONG objectID = aSelected[0];
		CComBSTR bstrPathParams;
		CComObjectStackEx<CToolWindow> cWnd;
		{
			CComBSTR bstrID;
			CComBSTR bstrParams;
			HRESULT hRes = pDVI->ObjectGet(objectID, &bstrID, &bstrParams);
			if (FAILED(hRes)) return hRes;
			CComPtr<IRasterImageEditToolsManager> pToolMgr;
			RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));
			CComPtr<IRasterImageEditTool> pTool;
			pToolMgr->EditToolCreate(bstrID, NULL, &pTool);
			if (pTool == NULL) return E_FAIL;
			CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
			if (pToolScript == NULL) return E_FAIL;
			CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
			if (pToolPoly == NULL) return E_FAIL;
			pTool->Init(&cWnd);
			pToolScript->FromText(bstrParams);

			CComPtr<IRasterImageEditTool> pPath;
			pToolMgr->EditToolCreate(bstrPathID, NULL, &pPath);
			CComQIPtr<IRasterImageEditToolScripting> pPathScript(pPath);
			CComQIPtr<IRasterImageEditToolPolygon> pPathPoly(pPath);
			pPath->Init(&cWnd);
			if (toPoly)
				hRes = pToolPoly->ToPolygon(pPathPoly);
			else
				hRes = pToolPoly->ToPath(pPathPoly);
			if (FAILED(hRes))
				return hRes;
			pPathScript->ToText(&bstrPathParams);
		}

		ECoordinatesMode eCM = ECMFloatingPoint;
		pDVI->ObjectStateSet(objectID, NULL, NULL, &eCM, NULL, NULL, NULL, NULL, NULL); // usually needed, otherwise points are moved
		return pDVI->ObjectSet(&objectID, bstrPathID, bstrPathParams);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>
HICON GetIconConvertToShape(ULONG a_nSize)
{
	static IRPolyPoint const arrow[] =
	{
		{32, 0}, {96, 64}, {126, 34}, {134, 134}, {34, 126}, {64, 96}, {0, 32},
	};
	static IRPathPoint const shape[] =
	{
		{113, 68, 30.8517, -7.07018, -36, 8.25},
		{177, 106, 9.75, 30, -9.75, -30},
		{222, 156, 51, 16.5, -37.0102, -11.9739},
		{212, 252, -45.5497, 9.96399, 48, -10.5},
		{77, 190, -28.5, -62.25, 27.3694, 59.7806},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}

HICON GetIconConvertToPoly(ULONG a_nSize)
{
	static IRPolyPoint const arrow[] =
	{
		{32, 0}, {96, 64}, {126, 34}, {134, 134}, {34, 126}, {64, 96}, {0, 32},
	};
	static IRPolyPoint const shape[] =
	{
		{219, 246}, {102, 246}, {66, 135}, {160, 66}, {255, 135},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}
