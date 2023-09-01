// RasterImageEditToolsText.cpp : Implementation of CRasterImageEditToolsText

#include "stdafx.h"
#include "RasterImageEditToolsText.h"

#include <IconRenderer.h>

HICON GetToolIconTEXT(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const letter[] =
	{
		{229, 11, 0, 0, 0, 0},
		{256, 87, 0, 0, 0, 0},
		{240, 87, -13, -17, 0, 0},
		{160, 59, 0, 0, 37, -5},
		{160, 186, 0, 23, 0, 0},
		{210, 231, 0, 0, -16, -5},
		{210, 242, 0, 0, 0, 0},
		{46, 242, 0, 0, 0, 0},
		{46, 231, 16, -5, 0, 0},
		{96, 186, 0, 0, 0, 23},
		{96, 59, -37, -5, 0, 0},
		{16, 87, 0, 0, 13, -17},
		{0, 87, 0, 0, 0, 0},
		{27, 11, 0, 0, 0, 0},
	};
	static IRGridItem gridX[] = { {0, 96}, {0, 160} };
	static IRGridItem gridY[] = { {0, 11}, {0, 242} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(letter), letter, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

// CRasterImageEditToolsText

STDMETHODIMP CRasterImageEditToolsText::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		if (wcscmp(a_bstrID, L"TEXT") == 0 || wcscmp(a_bstrID, L"GDITEXT") == 0)
		{
			*a_phIcon = GetToolIconTEXT(a_nSize);
			return S_OK;
		}

		return CRasterImageEditToolsFactoryImpl<g_aTextTools, sizeof(g_aTextTools)/sizeof(g_aTextTools[0])>::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}
