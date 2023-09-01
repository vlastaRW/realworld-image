// RasterImageEditToolsDrawing.cpp : Implementation of CRasterImageEditToolsDrawing

#include "stdafx.h"
#include "RasterImageEditToolsDrawing.h"

#include <IconRenderer.h>

HICON GetToolIconBRUSH(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const hairs[] =
	{
		{18, 252, 42, 13, 3, -14},
		{102, 201, -2, -14, -1, 41},
		{62, 169, -34, 1, 14, -5},
		{13, 229, 5, 3, 1, -16},
		{33, 230, -7, 5, -5, 2},
	};
	static IRPathPoint const handle[] =
	{
		{87, 144, -5, -31, -10, 21},
		{138, 68, 15.6869, -9.10855, -31, 18},
		{205, 1, 4, -4, -15.5081, 15.5081},
		{237, 27, -11, 19, 3.61303, -6.24068},
		{186, 103, -9, 33, 4.74341, -17.3925},
		{122, 171, -23, 14, 32, -2},
	};
	static IRPathPoint const join[] =
	{
		{62, 169, 6, 13, 3, -6},
		{102, 201, 6, -4, -15, -4},
		{123, 171, -9, -4, -4, 12},
		{86, 142, -7, 7, 4, 7},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(hairs), hairs, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(handle), handle, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(join), join, pSI->GetMaterial(ESMAltBackground));
	return cRenderer.get();
}

HICON GetToolIconPENCIL(ULONG a_nSize)
{
	IRPolyPoint const eraser[] =
	{
		{256, 56}, {256, 72}, {226, 102}, {154, 30}, {184, 0}, {200, 0},
	};
	IRPolyPoint const holder[] =
	{
		{160, 24}, {232, 96}, {213, 115}, {141, 43},
	};
	IRPolyPoint const body[] =
	{
		{147, 37}, {219, 109}, {90, 238}, {18, 166},
	};
	IRPolyPoint const slope[] =
	{
		{18, 166}, {24, 160}, {96, 232}, {90, 238}, {45, 247}, {9, 211},
	};
	IRPolyPoint const tip[] =
	{
		{11, 201}, {55, 245}, {0, 256},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(eraser), eraser, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(holder), holder, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(body), body, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(slope), slope, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(tip), tip, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconLASSO(ULONG a_nSize)
{
	IRPathPoint const ellipse0[] =
	{
		{248, 90, 0, 43.0782, 0, -43.0782},
		{124, 168, -68.4833, 0, 68.4833, 0},
		{0, 90, 0, -43.0782, 0, 43.0782},
		{124, 12, 68.4833, 0, -68.4833, 0},
	};
	IRPathPoint const ellipse1[] =
	{
		{124, 52, 46.3919, 0, -46.3919, 0},
		{208, 90, 0, 20.9868, 0, -20.9868},
		{124, 128, -46.3919, 0, 46.3919, 0},
		{40, 90, 0, -20.9868, 0, 20.9868},
	};
	IRPath const ellipse[] = { {itemsof(ellipse0), ellipse0}, {itemsof(ellipse1), ellipse1} };
	IRPathPoint const rope[] =
	{
		{256, 117, 0, 12.8315, 0, -23.196},
		{241, 149, 3.06103, 36.8271, 9.06871, -7.70397},
		{203, 256, 0, 0, -11.0247, -42.2614},
		{163, 256, -11.8553, -38.9532, 0, 0},
		{199, 156, -15.7195, -6.0727, -7.21907, 31.2392},
		{172, 117, 0, -23.196, 0, 17.8618},
		{214, 75, 23.196, 0, -23.196, 0},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(ellipse), ellipse, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(rope), rope, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}


// CRasterImageEditToolsDrawing

STDMETHODIMP CRasterImageEditToolsDrawing::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		if (wcscmp(a_bstrID, L"BRUSH") == 0)
		{
			*a_phIcon = GetToolIconBRUSH(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"PENCIL") == 0)
		{
			*a_phIcon = GetToolIconPENCIL(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"LASSO") == 0)
		{
			*a_phIcon = GetToolIconLASSO(a_nSize);
			return S_OK;
		}

		return CRasterImageEditToolsFactoryImpl<g_aDrawingTools, sizeof(g_aDrawingTools)/sizeof(g_aDrawingTools[0])>::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}
