// RasterImageEditToolsSpecial.cpp : Implementation of CRasterImageEditToolsSpecial

#include "stdafx.h"
#include "RasterImageEditToolsSpecial.h"


HICON GetToolIconPSHADOW(ULONG a_nSize)
{
	IRPathPoint const sphere[] =
	{
		{224, 96, 0, -53.0193, 0, 53.0193},
		{128, 0, -53.0193, 0, 53.0193, 0},
		{32, 96, 0, 53.0193, 0, -53.0193},
		{128, 192, 53.0193, 0, -53.0193, 0},
	};
	IRPathPoint const highlight[] =
	{
		{105, 50, -4.79583, -5.71544, 4.79583, 5.71544},
		{83, 50, -6.90661, 5.79534, 6.90661, -5.79534},
		{79, 70, 4.79583, 5.71544, -4.79583, -5.71544},
		{101, 70, 6.90661, -5.79534, -6.90661, 5.79534},
	};
	IRPathPoint const shade[] =
	{
		{210, 60, 13.8071, 30.3757, -9, 70},
		{191, 159, -23.7482, 23.7482, 23.7482, -23.7482},
		{92, 178, 70, -9, 30.3757, 13.8071},
	};
	IRPathPoint const shadow1[] =
	{
		{128, 255, 75.25, 0, -75.25, 0},
		{254, 213, -5.83151, -22.743, 7.00002, 27.3},
		{128, 177, -52.5, 0, 52.5, 0},
		{2, 213, -7, 27.3, 5.83153, -22.743},
	};
	IRPathPoint const shadow2[] =
	{
		{128, 252, 69.23, 0, -69.23, 0},
		{244, 213, -5.36499, -20.9236, 6.44, 25.116},
		{128, 180, -48.3, 0, 48.3, 0},
		{12, 213, -6.44, 25.116, 5.36501, -20.9236},
	};
	IRPathPoint const shadow3[] =
	{
		{128, 248, 63.6916, 0, -63.6916, 0},
		{235, 212, -4.93579, -19.2497, 5.9248, 23.1067},
		{128, 182, -44.436, 0, 44.436, 0},
		{21, 212, -5.9248, 23.1067, 4.93581, -19.2497},
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
	cRenderer(&canvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(highlight), highlight, &matWhite);
	cRenderer(&canvas, itemsof(shade), shade, &matBlack);
	return cRenderer.get();
}

HICON GetToolIconFLOODFILL(ULONG a_nSize)
{
	IRPathPoint const bucket[] =
	{
		{74, 13, 0, 0, 0, 0},
		{203, 113, 0, 0, 0, 0},
		{107, 212, -23.4139, 22.8429, 0, 0},
		{2, 130, 0, 0, -16.3025, 28.3591},
	};
	IRPathPoint const label[] =
	{
		{64, 36, 12, 33, 0, 0},
		{184, 130, 0, 0, -40, -3},
		{118, 200, -24, 12, 0, 0},
		{12, 114, 0, 0, -9, 31},
	};
	IRPathPoint const top[] =
	{
		{203, 113, 20.1906, -12.8139, -11.2965, 7.16929},
		{159, 37, -25.601, -19.8582, 25.6011, 19.8581},
		{74, 13, -4.13473, 12.7245, 7.39015, -22.743},
		{124, 81, 42.6684, 33.097, -42.6684, -33.097},
	};
	IRPathPoint const paint[] =
	{
		{131, 86, 15, 12, 0, 0},
		{178, 112, 0, 0, -8, 0},
		{178, 202, -25, 1, 0, 0},
		{128, 230, 0, 21, 0, -21},
		{193, 256, 31, 0, -31.0161, 0},
		{256, 229, 0, -20, 0, 20},
		{207, 202, 0, 0, 25, 0},
		{207, 108, 0, 0, 0, 0},
		{203, 86, 0, 0, 8, 12},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(bucket), bucket, pSI->GetMaterial(ESMScheme2Color2));
	//cRenderer(&canvas, itemsof(bucket), bucket, pSI->GetMaterial(ESMInterior));
	//cRenderer(&canvas, itemsof(label), label, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(top), top, pSI->GetMaterial(ESMAltBackground));
	cRenderer(&canvas, itemsof(paint), paint, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconFILL(ULONG a_nSize)
{
	IRPolyPoint const connection[] =
	{
		{147.588, 240.615}, {169.11, 219.094}, {36.9062, 86.89}, {30.7572, 93.039}, {21.5337, 83.816}, {56.1219, 49.228}, {160.655, 104.569}, {192.937, 72.286}, {183.714, 63.063}, {159.886, 86.89}, {55.3532, 31.549}, {3.0867, 83.816}, {21.5337, 102.263}, {15.3847, 108.412},
	};
	IRPolyPoint const pin[] =
	{
		{61.5027, 71.5178}, {184.482, 194.499}, {122.992, 255.988}, {0.0123672, 133.007},
	};
	IRPolyPoint const handle[] =
	{
		{163.729, 61.5255}, {225.22, 0.0354996}, {255.965, 30.7805}, {194.474, 92.2705},
	};
	IRPolyPoint const hilight[] =
	{
		{43.0554, 120.709}, {135.29, 212.945}, {122.992, 225.243}, {30.7574, 133.007},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(connection), connection, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(pin), pin, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(handle), handle, pSI->GetMaterial(ESMScheme2Color2));
	IRFill lightMat(0x3fffffff);
	cRenderer(&canvas, itemsof(hilight), hilight, &lightMat);
	return cRenderer.get();
}

HICON GetToolIconDROPPER(ULONG a_nSize)
{
	IRPathPoint const paint[] =
	{
		{7, 249, 5, -17, 37, -8},
		{28, 197, 0, 0, -9, 16},
		{103, 195, -21, 24, 0, 0},
	};
	IRPathPoint const glass[] =
	{
		{0.4, 255.6, 0, 0, 0, 0},
		{48.4, 159.6, 35.3551, -35.3555, -31.6228, 31.623},
		{168.4, 39.6, 8.40001, -22.8, -14.1422, 14.1422},
		{216.4, 87.6, -14.1422, 14.1419, 24, -9.60001},
		{96.4, 207.6, -31.6228, 31.623, 35.3551, -35.3555},
	};
	IRPathPoint const rubber[] =
	{
		{162, 22, 21.8285, -36.3803, -30, 49.9998},
		{234, 22, 19.9998, 20, -19.9998, -20},
		{234, 94, -49.9998, 30, 36.3805, -21.8282},
		{138, 118, -40.0002, -40.0002, 40.0002, 40.0002},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRFill glassFillMat(0x7fffffff);
	IROutlinedFill glassMat(&glassFillMat, pSI->GetMaterial(ESMContrast));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(paint), paint, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(glass), glass, &glassMat);
	cRenderer(&canvas, itemsof(rubber), rubber, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}


// CRasterImageEditToolsSpecial

STDMETHODIMP CRasterImageEditToolsSpecial::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		if (wcscmp(a_bstrID, L"PSHADOW") == 0)
		{
			*a_phIcon = GetToolIconPSHADOW(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"FLOODFILL") == 0)
		{
			*a_phIcon = GetToolIconFLOODFILL(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"FILL") == 0)
		{
			*a_phIcon = GetToolIconFILL(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"DROPPER") == 0)
		{
			*a_phIcon = GetToolIconDROPPER(a_nSize);
			return S_OK;
		}

		return CRasterImageEditToolsFactoryImpl<g_aSpecialTools, sizeof(g_aSpecialTools)/sizeof(g_aSpecialTools[0])>::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}
