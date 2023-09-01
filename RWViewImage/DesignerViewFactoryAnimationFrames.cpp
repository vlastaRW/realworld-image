// DesignerViewFactoryAnimationFrames.cpp : Implementation of CDesignerViewFactoryAnimationFrames

#include "stdafx.h"
#include "DesignerViewFactoryAnimationFrames.h"
#include "DesignerViewAnimationFrames.h"
#include <SharedStringTable.h>
#include <GammaCorrection.h>


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
#include "ConfigGUISyncID.h"

// CDesignerViewFactoryAnimationFrames

STDMETHODIMP CDesignerViewFactoryAnimationFrames::NameGet(IViewManager* UNREF(a_pManager), ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = _SharedStringTable.GetString(IDS_FRAMES_VIEWNAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryAnimationFrames::ConfigCreate(IViewManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
		pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_HELP), CConfigValue(L"SELECTION"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryAnimationFrames::CreateWnd(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* UNREF(a_pStatusBar), IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd)
{
	try
	{
		*a_ppDVWnd = NULL;

		CComObject<CDesignerViewAnimationFrames>* pWnd = NULL;
		CComObject<CDesignerViewAnimationFrames>::CreateInstance(&pWnd);
		CComPtr<IDesignerView> pDummy = pWnd;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cVal);

		pWnd->Init(a_pFrame, cVal, a_hParent, a_prcWindow, a_nStyle, a_tLocaleID, a_pDoc);

		*a_ppDVWnd = pDummy.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDVWnd == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewFactoryAnimationFrames::CheckSuitability(IViewManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback)
{
	CComPtr<IDocumentAnimation> p;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&p));
	if (p) a_pCallback->Used(__uuidof(IDocumentAnimation));
	else a_pCallback->Missing(__uuidof(IDocumentAnimation));
	return S_OK;
}


STDMETHODIMP CDesignerViewFactoryAnimationFrames::TimeStamp(ULONG* a_pTimeStamp)
{
	if (a_pTimeStamp)
		*a_pTimeStamp = 0;
	return S_OK;
}

static GUID const ICONID_ANISPEED = {0x1a025de0, 0xf8dd, 0x44ce, {0x80, 0x9a, 0x8, 0xd6, 0xe6, 0x78, 0xc9, 0xd9}}; // {1A025DE0-F8DD-44ce-809A-08D6E678C9D9}
extern GUID const CLSID_AnimationMorph;
static GUID const ICONID_REVERSE = {0xdd4191f5, 0x393a, 0x485b, {0xa8, 0x5b, 0x69, 0x1b, 0x7b, 0x97, 0x8a, 0x9e}}; // {DD4191F5-393A-485b-A85B-691B7B978A9E}

STDMETHODIMP CDesignerViewFactoryAnimationFrames::EnumIconIDs(IEnumGUIDs** a_ppIDs)
{
	if (a_ppIDs == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IEnumGUIDsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
		static GUID const aIDs[] =
		{
			CLSID_DocumentOperationInsertFrame,
			ICONID_ANISPEED,
			CLSID_AnimationMorph,
			ICONID_REVERSE,
		};
		pTmp->InsertMultiple(itemsof(aIDs), aIDs);
		*a_ppIDs = pTmp;
		pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool CDesignerViewFactoryAnimationFrames::GetFrameIconLayers(IStockIcons* pSI, CIconRendererReceiver& cRenderer, IRTarget const* target)
{
	static IRPathPoint const path0[] = {
		{48, 112, 0, 0, 0, 0},
		{160, 112, 0, 0, 0, 0},
		{160, 96, 0, -8.83656, 0, 0},
		{176, 80, 0, 0, -8.83656, 0},
		{192, 80, 8.83656, 0, 0, 0},
		{208, 96, 0, 0, 0, -8.83656},
		{208, 240, 0, 8.83656, 0, 0},
		{192, 256, 0, 0, 8.83656, 0},
		{176, 256, -8.83656, 0, 0, 0},
		{160, 240, 0, 0, 0, 8.83656},
		{160, 224, 0, 0, 0, 0},
		{48, 224, 0, 0, 0, 0},
		{48, 240, 0, 8.83656, 0, 0},
		{32, 256, 0, 0, 8.83656, 0},
		{16, 256, -8.83656, 0, 0, 0},
		{0, 240, 0, 0, 0, 8.83656},
		{0, 96, 0, -8.83656, 0, 0},
		{16, 80, 0, 0, -8.83656, 0},
		{32, 80, 8.83656, 0, 0, 0},
		{48, 96, 0, 0, 0, -8.83656},
	};
	static IRPath const shape0 = {itemsof(path0), path0};
	static IRPolyPoint const poly0[] = {{16, 96}, {32, 96}, {32, 112}, {16, 112}};
	static IRPolyPoint const poly1[] = {{16, 128}, {32, 128}, {32, 144}, {16, 144}};
	static IRPolyPoint const poly2[] = {{16, 160}, {32, 160}, {32, 176}, {16, 176}};
	static IRPolyPoint const poly3[] = {{16, 192}, {32, 192}, {32, 208}, {16, 208}};
	static IRPolyPoint const poly4[] = {{16, 224}, {32, 224}, {32, 240}, {16, 240}};
	static IRPolyPoint const poly5[] = {{176, 96}, {192, 96}, {192, 112}, {176, 112}};
	static IRPolyPoint const poly6[] = {{176, 128}, {192, 128}, {192, 144}, {176, 144}};
	static IRPolyPoint const poly7[] = {{176, 160}, {192, 160}, {192, 176}, {176, 176}};
	static IRPolyPoint const poly8[] = {{176, 192}, {192, 192}, {192, 208}, {176, 208}};
	static IRPolyPoint const poly9[] = {{176, 224}, {192, 224}, {192, 240}, {176, 240}};
	static IRPolyPoint const poly10[] = {{48, 128}, {160, 128}, {160, 208}, {48, 208}};
	static IRPolygon const polygons[] = {
		{itemsof(poly0), poly0},
		{itemsof(poly1), poly1},
		{itemsof(poly2), poly2},
		{itemsof(poly3), poly3},
		{itemsof(poly4), poly4},
		{itemsof(poly5), poly5},
		{itemsof(poly6), poly6},
		{itemsof(poly7), poly7},
		{itemsof(poly8), poly8},
		{itemsof(poly9), poly9},
		{itemsof(poly10), poly10},
	};
	//IRGridItem const gridX[] = {{0, 0}, {1, 16}, {2, 32}, {1, 48}, {3, 160}, {1, 176}, {2, 192}, {1, 208}, {1, 224}};
	static IRGridItem const gridX[] = {{0, 0}, {1, 48}, {0, 160}, {1, 208}};
	//IRGridItem const gridY[] = {{10, 80}, {1, 96}, {2, 112}, {13, 224}, {2, 240}, {1, 256}, };
	static IRGridItem const gridY[] = {{0, 80}, {0, 112}, {0, 224}, {0, 256}, };
	static IRCanvas const canvas0 = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

	return
		cRenderer(&canvas0, 1, &shape0, pSI->GetMaterial(ESMContrast, true), target) &&
		cRenderer(&canvas0, itemsof(polygons), polygons, pSI->GetMaterial(ESMBackground, true), target);
}

STDMETHODIMP CDesignerViewFactoryAnimationFrames::GetIcon(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		if (IsEqualGUID(a_tIconID, CLSID_DocumentOperationInsertFrame))
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			GetFrameIconLayers(pSI, cRenderer);
			pSI->GetLayers(ESIPlus, cRenderer, IRTarget(0.65f, 1, -1));
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		else if (IsEqualGUID(a_tIconID, ICONID_ANISPEED))
		{
			IRPolyPoint const path0[] = { {224, 0}, {96, 128}, {160, 152}, {128, 256}, {256, 128}, {192, 104} };
			IRPolyPoint const path1[] = { {192, 0}, {64, 8}, {176, 16} };
			IRPolyPoint const path2[] = { {140, 52}, {31, 60}, {124, 68} };
			IRPolyPoint const path3[] = { {88, 104}, {0, 112}, {72, 120} };
			IRPolyPoint const path4[] = { {128, 160}, {45, 168}, {122, 176} };
			IRPolygon const shape0 = {itemsof(path0), path0};
			IRPolygon const shape1[] = { {itemsof(path1), path1}, {itemsof(path2), path2}, {itemsof(path3), path3}, {itemsof(path4), path4} };
			IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			//IRFill yellow(0xffebd467);
			//IROutlinedFill flash(&yellow, pSI->GetMaterial(ESMContrast));
			IRLayer const layers[] =
			{
				{&canvas, 1, 0, &shape0, NULL, pSI->GetMaterial(ESMScheme1Color3)},//ESMBrightLight)},
				{&canvas, itemsof(shape1), 0, shape1, NULL, pSI->GetMaterial(ESMContrast)},
			};
			CComPtr<IIconRenderer> pIR;
			RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			*a_phIcon = pIR->CreateIcon(a_nSize, itemsof(layers), layers);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		else if (IsEqualGUID(a_tIconID, CLSID_AnimationMorph))
		{
			IRPolyPoint const path0[] = { {0, 128}, {128, 128}, {128, 256}, {0, 256} };
			IRPathPoint const path1[] =
			{
				{64, 96, 0, -17.6731, 0, 0},
				{96, 64, 0, 0, -17.6731, 0},
				{160, 64, 17.6731, 0, 0, 0},
				{192, 96, 0, 0, 0, -17.6731},
				{192, 160, 0, 17.6731, 0, 0},
				{160, 192, 0, 0, 17.6731, 0},
				{96, 192, -17.6731, 0, 0, 0},
				{64, 160, 0, 0, 0, 17.6731},
			};
			IRPathPoint const path2[] =
			{
				{256, 64, 0, -35.3462, 0, 35.3462},
				{192, 0, -35.3462, 0, 35.3462, 0},
				{128, 64, 0, 35.3462, 0, -35.3462},
				{192, 128, 35.3462, 0, -35.3462, 0},
			};
			IRPolygon const shape0 = {itemsof(path0), path0};
			IRPath const shape1 = {itemsof(path1), path1};
			IRPath const shape2 = {itemsof(path2), path2};
			IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			IRFill midFill(CGammaTables::BlendSRGBA(pSI->GetSRGBColor(ESMScheme1Color3), pSI->GetSRGBColor(ESMScheme1Color1), 0.5f));
			IROutlinedFill mid(&midFill, pSI->GetMaterial(ESMContrast));
			IRLayer const layers[] =
			{
				{&canvas, 1, 0, &shape0, NULL, pSI->GetMaterial(ESMScheme1Color3)},
				{&canvas, 0, 1, NULL, &shape1, &mid},
				{&canvas, 0, 1, NULL, &shape2, pSI->GetMaterial(ESMScheme1Color1)},
			};
			CComPtr<IIconRenderer> pIR;
			RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			*a_phIcon = pIR->CreateIcon(a_nSize, itemsof(layers), layers);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		else if (IsEqualGUID(a_tIconID, ICONID_REVERSE))
		{
			IRPolyPoint const path0[] = { {256, 72}, {192, 8}, {192, 48}, {32, 48}, {32, 96}, {192, 96}, {192, 136} };
			IRGridItem const gridX0[] = {{0, 32}, {0, 192}, {0, 256}};
			IRGridItem const gridY0[] = {{0, 48}, {0, 96}};
			IRCanvas const canvas0 = {0, 0, 256, 256, itemsof(gridX0), itemsof(gridY0), gridX0, gridY0};
			IRPolyPoint const path1[] = { {0, 184}, {64, 248}, {64, 208}, {224, 208}, {224, 160}, {64, 160}, {64, 120} };
			IRGridItem const gridX1[] = {{0, 0}, {0, 64}, {0, 224}};
			IRGridItem const gridY1[] = {{0, 160}, {0, 208}};
			IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(gridX1), itemsof(gridY1), gridX1, gridY1};

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&canvas0, itemsof(path0), path0, pSI->GetMaterial(ESMScheme1Color2));
			cRenderer(&canvas1, itemsof(path1), path1, pSI->GetMaterial(ESMScheme1Color1));
			*a_phIcon = cRenderer.get();

			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

