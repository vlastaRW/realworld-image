// RasterImageEditToolsPhoto.cpp : Implementation of CRasterImageEditToolsPhoto

#include "stdafx.h"
#include "RasterImageEditToolsPhoto.h"

#include "EditToolClone.h"
#include "EditToolBubble.h"

#include <IconRenderer.h>


// CRasterImageEditToolsPhoto

STDMETHODIMP CRasterImageEditToolsPhoto::ToolIDsEnum(IRasterImageEditToolsManager* a_pManager, IEnumStrings** a_ppToolIDs)
{
	try
	{
		HRESULT hRes = CImpl::ToolIDsEnum(a_pManager, a_ppToolIDs);
		CComQIPtr<IEnumStringsInit>(*a_ppToolIDs)->Insert(CComBSTR(L"CLONE_"));
		CComQIPtr<IEnumStringsInit>(*a_ppToolIDs)->Insert(CComBSTR(L"BUBBLE_"));
		return hRes;
	}
	catch (...)
	{
		return a_ppToolIDs ? E_UNEXPECTED : E_POINTER;
	}
}

class CLocalizedStringPrintf :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ILocalizedString
{
public:
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_nInt = a_nInt;
		m_bUseInt = true;
		m_bStringFirst = true;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_nInt = a_nInt;
		m_bUseInt = true;
		m_bStringFirst = false;
	}
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_bUseInt = false;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_nInt = a_nInt;
		m_bUseInt = true;
	}

	BEGIN_COM_MAP(CLocalizedStringPrintf)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;

			CComBSTR bstrTemplate;
			m_pTemplate->GetLocalized(a_tLCID, &bstrTemplate);
			CComBSTR bstrString;
			if (m_pString)
				m_pString->GetLocalized(a_tLCID, &bstrString);
			CAutoVectorPtr<wchar_t> szBuffer(new wchar_t[bstrTemplate.Length()+bstrString.Length()+128]);
			if (m_bUseInt)
			{
				if (m_pString)
					if (m_bStringFirst)
						swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str, m_nInt);
					else
						swprintf(szBuffer.m_p, bstrTemplate, m_nInt, bstrString.m_str);
				else
					swprintf(szBuffer.m_p, bstrTemplate, m_nInt);
			}
			else
			{
				if (m_pString)
					swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str);
				else
					wcscpy(szBuffer.m_p, bstrTemplate);
			}
			*a_pbstrString = SysAllocString(szBuffer.m_p);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	CComPtr<ILocalizedString> m_pTemplate;
	CComPtr<ILocalizedString> m_pString;
	int m_nInt;
	bool m_bUseInt;
	bool m_bStringFirst;
};

STDMETHODIMP CRasterImageEditToolsPhoto::ToolNameGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]Clone with: %s[0405]Klonovat pomocí: %s"), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]%s in bubble[0405]%s v bublině"), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		return CImpl::ToolNameGet(a_pManager, a_bstrID, a_ppName);
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsPhoto::ToolDescGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]Copy pixels from other part of the image. Pixels to overwrite are marked using tool \"%s\".[0405]Kopírovat pixely z jiné části obrázku. Pixely pro přepsání budou označeny pomocí nástroje \"%s\"."), pInternal);
			*a_ppDesc = pTmp.Detach();
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]Use the \"%s\" tool and draw a bubble around it.[0405]Použít nástroj \"%s\" a okolo něj nakreslit bublinu."), pInternal);
			*a_ppDesc = pTmp.Detach();
			return S_OK;
		}
		return CImpl::ToolDescGet(a_pManager, a_bstrID, a_ppDesc);
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsPhoto::ToolIconIDGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, GUID* a_ptDefaultIcon)
{
	try
	{
		*a_ptDefaultIcon = GUID_NULL;
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			static GUID const tCloneIconID = {0x91999163, 0xa4c4, 0x4e4d, {0x8c, 0xff, 0xab, 0xff, 0x82, 0x8c, 0x28, 0xe4}};
			*a_ptDefaultIcon = tCloneIconID;
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			GUID tIcon = GUID_NULL;
			HRESULT hRes = a_pManager->ToolIconIDGet(bstrInternal, &tIcon);
			if (FAILED(hRes) || IsEqualGUID(tIcon, GUID_NULL))
			{
				static GUID const tBubbleIconID = {0xcd50417d, 0x1771, 0x426b, {0x80, 0xaf, 0x4a, 0x0a, 0x78, 0x60, 0xfe, 0x3d}};
				*a_ptDefaultIcon = tBubbleIconID;
				return S_OK;
			}
			SKey sKey(tIcon, 0);
			CIconIds::const_iterator i = m_cIconIds.find(sKey);
			if (i != m_cIconIds.end())
			{
				*a_ptDefaultIcon = i->second;
				return S_OK;
			}
			CoCreateGuid(a_ptDefaultIcon);
			m_cIconIds[sKey] = *a_ptDefaultIcon;
			return S_OK;
		}
		return CImpl::ToolIconIDGet(a_pManager, a_bstrID, a_ptDefaultIcon);
	}
	catch (...)
	{
		return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
	}
}

HICON GetToolIconREDEYE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const eye[] =
	{
		{256, 128, -23.4869, 46.8361, -23.4869, -46.8361},
		{128, 207, -55.9636, 0, 55.9637, 0},
		{0, 128, 23.487, -46.8359, 23.487, 46.8359},
		{128, 49, 55.9637, 0, -55.9636, 0},
	};
	static IRPathPoint const iris[] =
	{
		{188, 128, 0, -33.1371, 0, 33.1371},
		{128, 68, -33.1371, 0, 33.1371, 0},
		{68, 128, 0, 33.1371, 0, -33.1371},
		{128, 188, 33.1371, 0, -33.1371, 0},
	};
	static IRPathPoint const pupil[] =
	{
		{153, 128, 0, -13.8071, 0, 13.8071},
		{128, 103, -13.8071, 0, 13.8071, 0},
		{103, 128, 0, 13.8071, 0, -13.8071},
		{128, 153, 13.8071, 0, -13.8071, 0},
	};
	static IRPathPoint const hilight[] =
	{
		{125, 111, 0, -6.62742, 0, 6.62742},
		{113, 99, -6.62742, 0, 6.62742, 0},
		{101, 111, 0, 6.62742, 0, -6.62742},
		{113, 123, 6.62742, 0, -6.62742, 0},
	};
	static IRPathPoint const brow[] =
	{
		{260, 67, -35.0171, -26.8701, -27.3217, -36.1493},
		{128, 24, -50.6595, 0, 50.6598, 0},
		{-4, 67, 27.3217, -36.1491, 35.017, -26.8698},
		{128, 7, 55.7081, 0, -55.7079, 0},
	};
	static IRCanvas canvas = {0, 24, 256, 208, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	IRFill hilightMat(0x7fffffff);
	IRFill irisFillMat(0xffef6562);
	IROutlinedFill irisMat(&irisFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(eye), eye, pSI->GetMaterial(ESMBackground));
	cRenderer(&canvas, itemsof(iris), iris, &irisMat);
	cRenderer(&canvas, itemsof(pupil), pupil, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(hilight), hilight, &hilightMat);
	cRenderer(&canvas, itemsof(brow), brow, pSI->GetMaterial(ESMContrast));
	return cRenderer.get();
}

HICON GetToolIconCROP(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const leg1outline[] =
	{
		{214.806, 0.5356, 0, 0, -81.9775, 13.0067},
		{115.893, 178.289, 26.1513, 22.6828, 0, 0},
		{101.169, 251.557, -19.4951, 10.5546, 17.0341, -9.22224},
		{55.7048, 225.347, 1.56339, -25.0311, -1.62552, 26.0261},
		{94.826, 174.656, -5.46538, -83.9174, -22.9441, 3.40096},
	};
	static IRPathPoint const leg1hole[] =
	{
		{84.7702, 228.84, 6.64731, 3.66953, -6.64731, -3.66953},
		{97.297, 201.617, -7.42096, -3.69814, 7.79524, 3.88464},
	};
	static IRPath const leg1[] = { {itemsof(leg1outline), leg1outline}, {itemsof(leg1hole), leg1hole} };
	static IRPathPoint const leg2outline[] =
	{
		{253.574, 41.808, 0, 0, -11.574, 82.192},
		{78, 144, -23.1358, -25.7515, 0, 0},
		{5, 160, -10.2128, 19.6763, 8.92355, -17.1925},
		{32, 205, 25, -2, -25.9938, 2.0795},
		{82, 165, 84, 4, -3, 23},
	};
	static IRPathPoint const leg2hole[] =
	{
		{28, 176, -3.78498, -6.58224, 3.78498, 6.58224},
		{55, 163, 3.82708, 7.35529, -4.0201, -7.72626},
	};
	static IRPath const leg2[] = { {itemsof(leg2outline), leg2outline}, {itemsof(leg2hole), leg2hole} };
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(leg1), leg1, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(leg2), leg2, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconCLONE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const handle[] =
	{
		{172, 164, 0, -22, 0, 0},
		{157, 114, 0, -28, 0, 23.5372},
		{185, 52, 0, -30, 0, 32},
		{128, 0, -34, 0, 34, 0},
		{71, 52, 0, 32, 0, -30},
		{99, 114, 0, 23.5372, 0, -28},
		{84, 164, 0, 0, 0, -22},
		//{172, 176, 0, -22, 0, 0},
		//{157, 118, 0, -28, 0, 23.5372},
		//{185, 52, 0, -30, 0, 34},
		//{128, 0, -34, 0, 34, 0},
		//{71, 52, 0, 34, 0, -30},
		//{99, 118, 0, 23.5372, 0, -28},
		//{84, 176, 0, 0, 0, -22},
	};
	static IRGridItem handleY[] = { {0, 0}, {0, 176} };
	static IRCanvas canvasHandle = {0, 0, 256, 256, 0, itemsof(handleY), NULL, handleY};
	static IRPolyPoint const body[] =
	{
		{164, 152}, {164, 180}, {224, 180}, {240, 200}, {244, 244}, {12, 244}, {16, 200}, {32, 180}, {92, 180}, {92, 152},
		//{164, 160}, {164, 192}, {224, 192}, {240, 208}, {244, 244}, {12, 244}, {16, 208}, {32, 192}, {92, 192}, {92, 160},
	};
	static IRGridItem bodyX[] = { {0, 92}, {0, 164} };
	static IRGridItem bodyY[] = { {0, 192}, {0, 244} };
	static IRCanvas canvasBody = {0, 0, 256, 256, itemsof(bodyX), itemsof(bodyY), bodyX, bodyY};
	static IRPolyPoint const rubber[] =
	{
		{224, 256}, {224, 224}, {32, 224}, {32, 256},
	};
	static IRGridItem bot = {0, 256};
	static IRCanvas canvasRubber = {0, 0, 256, 256, 0, 1, NULL, &bot};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvasRubber, itemsof(rubber), rubber, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvasBody, itemsof(body), body, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvasHandle, itemsof(handle), handle, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

void GetLayersBUBBLE(CIconRendererReceiver& cRenderer)
{
	IRPathPoint const bubblePoints[] =
	{
		{256, 126, 0, 55.7808, 0, -55.7808},
		{128, 227, -12.039, 0, 70.6925, 0},
		{93, 223, 0, 0, 11.0457, 2.45175},
		{18, 239, 0, 0, 0, 0},
		{29, 190, -18.2181, -17.449, 0, 0},
		{0, 126, 0, -55.7808, 0, 24.3729},
		{128, 25, 70.6925, 0, -70.6925, 0},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	cRenderer(&canvas, itemsof(bubblePoints), bubblePoints, pSI->GetMaterial(ESMInterior));
}

HICON GetToolIconBUBBLE(ULONG a_nSize)
{
	CIconRendererReceiver cRenderer(a_nSize);
	GetLayersBUBBLE(cRenderer);
	return cRenderer.get();
}

HICON GetToolIconSHAPESHIFT(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const shape[] =
	{
		{0, 58, 71, -59, 60, 33},
		{256, 26, -55, 7, -75, -47},
		{118, 90, 32, -13, 21, -40},
		{227, 109, -48, 2, -28, -27},
		{128, 163, 30, -9, 13, -34},
		{205, 182, -50, 4, -21, -22},
		{112, 256, -6, -66, 17, -38},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetShapeShiftIconForward(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const shape[] =
	{
		{0, 144, 56, 2, 0, 0},
		{128, 36, 66, 0, -66, 0},
		{256, 144, 0, 0, -56, 2},
		{256, 216, -92, -8, 0, 0},
		{128, 108, -24, 0, 24, 0},
		{0, 216, 0, 0, 92, -8},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetShapeShiftIconSideways(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const shape[] =
	{
		{112, 0, -2, 56, 0, 0},
		{220, 128, 0, 66, 0, -66},
		{112, 256, 0, 0, -2, -56},
		{40, 256, 8, -92, 0, 0},
		{148, 128, 0, -24, 0, 24},
		{40, 0, 0, 0, 8, 92},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetShapeShiftIconRestore(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static ULONG gap = 22;
	static ULONG width = 72;
	static ULONG both = gap+width;
	static IRPolyPoint const outside[] =
	{
		{both, gap}, {256-both, gap}, {256-both, 0}, {256-gap, 0}, {256-gap, gap}, {256, gap}, {256, both}, {256-gap, both}, {256-gap, 256-both}, {256, 256-both}, {256, 256-gap}, {256-gap, 256-gap}, {256-gap, 256}, {256-both, 256}, {256-both, 256-gap}, {both, 256-gap}, {both, 256}, {gap, 256}, {gap, 256-gap}, {0, 256-gap}, {0, 256-both}, {gap, 256-both}, {gap, both}, {0, both}, {0, gap}, {gap, gap}, {gap, 0}, {both, 0},
	};
	static IRPolyPoint const inside[] =
	{
		{256-both, 256-both}, {both, 256-both}, {both, both}, {256-both, both},
	};
	static IRPolygon const shape[] = { {itemsof(outside), outside}, {itemsof(inside), inside} };
	static IRGridItem grid[] = { {0, 0}, {0, gap}, {0, both}, {0, 256-both}, {0, 256-gap}, {0, 256} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetShapeShiftIconExpand(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const shape1[] =
	{
		{46, 256, 0, -64, 0, 0},
		{0, 128, 0, -60, 0, 60},
		{46, 0, 0, 0, 0, 64},
		{118, 0, 0, 88, 0, 0},
		{72, 128, 0, 40, 0, -40},
		{118, 256, 0, 0, 0, -88},
	};
	static IRPathPoint const shape2[] =
	{
		{210, 0, 0, 64, 0, 0},
		{256, 128, 0, 60, 0, -60},
		{210, 256, 0, 0, 0, -64},
		{138, 256, 0, -88, 0, 0},
		{184, 128, 0, -40, 0, 40},
		{138, 0, 0, 0, 0, 88},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape1), shape1, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(shape2), shape2, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetShapeShiftIconCollapse(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const shape1[] =
	{
		{72, 0, 0, 64, 0, 0},
		{118, 128, 0, 60, 0, -60},
		{72, 256, 0, 0, 0, -64},
		{0, 256, 0, -88, 0, 0},
		{46, 128, 0, -40, 0, 40},
		{0, 0, 0, 0, 0, 88},
	};
	static IRPathPoint const shape2[] =
	{
		{184, 256, 0, -64, 0, 0},
		{138, 128, 0, -60, 0, 60},
		{184, 0, 0, 0, 0, 64},
		{256, 0, 0, 88, 0, 0},
		{210, 128, 0, 40, 0, -40},
		{256, 256, 0, 0, 0, -88},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape1), shape1, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(shape2), shape2, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

STDMETHODIMP CRasterImageEditToolsPhoto::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		//if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		//{
		//	*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_EDITTOOL_CLONE), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
		//	return S_OK;
		//}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			HICON hIcon = NULL;
			HRESULT hRes = a_pManager->ToolIconGet(bstrInternal, a_nSize, &hIcon);
			if (FAILED(hRes) || hIcon == NULL)
			{
				*a_phIcon = GetToolIconBUBBLE(a_nSize);
				return S_OK;
			}
			*a_phIcon = CreateBubbleIcon(hIcon);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"CROP") == 0)
		{
			*a_phIcon = GetToolIconCROP(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"SHAPESHIFT") == 0)
		{
			*a_phIcon = GetToolIconSHAPESHIFT(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"REDEYE") == 0)
		{
			*a_phIcon = GetToolIconREDEYE(a_nSize);
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			*a_phIcon = GetToolIconCLONE(a_nSize);
			return S_OK;
		}
		return CImpl::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsPhoto::SupportedStates(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
{
	try
	{
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			if (a_pBlendingModes) *a_pBlendingModes = EBMDrawOver;
			return a_pManager->SupportedStates(bstrInternal, NULL, a_pRasterizationModes, a_pCoordinatesModes, NULL);
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			if (a_pBlendingModes) *a_pBlendingModes = EBMDrawOver|EBMDrawUnder|EBMReplace;
			return a_pManager->SupportedStates(bstrInternal, NULL, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
		}
		return CImpl::SupportedStates(a_pManager, a_bstrID, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageEditToolsPhoto::EditToolCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool)
{
	try
	{
		*a_ppTool = NULL;
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolClone>* p = NULL;
			CComObject<CEditToolClone>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolBubble>* p = NULL;
			CComObject<CEditToolBubble>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		return CImpl::EditToolCreate(a_pManager, a_bstrID, a_pDocument, a_ppTool);
	}
	catch (...)
	{
		return a_ppTool ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsPhoto::WindowCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		if (wcsncmp(a_bstrID, L"CLONE_", 6) == 0)
		{
			CComObject<CEditToolCloneDlg>* p = NULL;
			HRESULT hRes = CComObject<CEditToolCloneDlg>::CreateInstance(&p);
			CComPtr<IRasterImageEditToolWindow> pTmp = p;
			if (p)
				if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+6))
					return E_FAIL;
			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		if (wcsncmp(a_bstrID, L"BUBBLE_", 7) == 0)
		{
			CComObject<CEditToolBubbleDlg>* p = NULL;
			HRESULT hRes = CComObject<CEditToolBubbleDlg>::CreateInstance(&p);
			CComPtr<IRasterImageEditToolWindow> pTmp = p;
			if (p)
				if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+7))
					return E_FAIL;
			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		return CImpl::WindowCreate(a_pManager, a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
	}
	catch (...)
	{
		return a_ppWindow ? E_UNEXPECTED : E_POINTER;
	}
}

HICON CRasterImageEditToolsPhoto::CreateBubbleIcon(HICON a_hSource)
{
	if (a_hSource == NULL)
		return NULL;

	ICONINFO tInfo;
	ZeroMemory(&tInfo, sizeof tInfo);
	GetIconInfo(a_hSource, &tInfo);
	if (tInfo.hbmMask)
		DeleteObject(tInfo.hbmMask);
	if (tInfo.hbmColor == NULL)
		return a_hSource;
	BITMAP tBmp;
	ZeroMemory(&tBmp, sizeof tBmp);
	GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
	if (tBmp.bmBitsPixel != 32)
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // ignore low quality icons
	}
	int nSmSizeX = tBmp.bmWidth;
	int nSmSizeY = tBmp.bmHeight;
	DWORD nXOR = nSmSizeY*nSmSizeX<<2;
	DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pInterRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pInterRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pInterRes.m_p+sizeof BITMAPINFOHEADER))
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // failure
	}

	DeleteObject(tInfo.hbmColor);

	CIconRendererReceiver cRenderer(nSmSizeX);
	GetLayersBUBBLE(cRenderer);


	for (int y = 0; y+1 < tBmp.bmHeight; y+=2)
	{
		TRasterImagePixel* pDst = reinterpret_cast<TRasterImagePixel*>(cRenderer.pixelRow((tBmp.bmHeight>>2)+(y>>1)))+(tBmp.bmWidth>>2);
		TRasterImagePixel const* pSrc = reinterpret_cast<TRasterImagePixel const*>(pInterRes.m_p+sizeof BITMAPINFOHEADER)+tBmp.bmWidth*y;
		for (int x = 0; x+1 < tBmp.bmWidth; x+=2)
		{
			ULONG nR = (ULONG(pSrc[0].bR)*pSrc[0].bA + ULONG(pSrc[1].bR)*pSrc[1].bA + ULONG(pSrc[tBmp.bmWidth].bR)*pSrc[tBmp.bmWidth].bA + ULONG(pSrc[tBmp.bmWidth+1].bR)*pSrc[tBmp.bmWidth+1].bA+2)>>2;
			ULONG nG = (ULONG(pSrc[0].bG)*pSrc[0].bA + ULONG(pSrc[1].bG)*pSrc[1].bA + ULONG(pSrc[tBmp.bmWidth].bG)*pSrc[tBmp.bmWidth].bA + ULONG(pSrc[tBmp.bmWidth+1].bG)*pSrc[tBmp.bmWidth+1].bA+2)>>2;
			ULONG nB = (ULONG(pSrc[0].bB)*pSrc[0].bA + ULONG(pSrc[1].bB)*pSrc[1].bA + ULONG(pSrc[tBmp.bmWidth].bB)*pSrc[tBmp.bmWidth].bA + ULONG(pSrc[tBmp.bmWidth+1].bB)*pSrc[tBmp.bmWidth+1].bA+2)>>2;
			ULONG const nA = (ULONG(pSrc[0].bA) + ULONG(pSrc[1].bA) + ULONG(pSrc[tBmp.bmWidth].bA) + ULONG(pSrc[tBmp.bmWidth+1].bA)+2)>>2;
			if (nA == 255)
			{
				pDst->bR = nR/nA;
				pDst->bG = nG/nA;
				pDst->bB = nB/nA;
				pDst->bA = nA;
			}
			else if (nA != 0)
			{
				// blend pixels
				ULONG nNewA = nA*255 + (255-nA)*pDst->bA;
				if (nNewA >= 255)
				{
					ULONG const bA1 = (255-nA)*pDst->bA;
					pDst->bR = (pDst->bR*bA1 + nR*255)/nNewA;
					pDst->bG = (pDst->bG*bA1 + nG*255)/nNewA;
					pDst->bB = (pDst->bB*bA1 + nB*255)/nNewA;
				}
				else
				{
					pDst->bR = pDst->bG = pDst->bB = 0;
				}
				pDst->bA = nNewA/255;
			}
			++pDst;
			pSrc += 2;
		}
	}
	return cRenderer.get();
}

TRasterImagePixel TColorToTRasterImagePixel(TColor const& a_tColor, float a_fGamma)
{
	TRasterImagePixel t;
	if (a_fGamma <= 0.0f || (a_fGamma >= 2.1f && a_fGamma <= 2.3f))
	{
		t.bR = CGammaTables::ToSRGB(a_tColor.fR);
		t.bG = CGammaTables::ToSRGB(a_tColor.fG);
		t.bB = CGammaTables::ToSRGB(a_tColor.fB);
	}
	else
	{
		if (a_fGamma < 0.1f || a_fGamma > 10.0f) a_fGamma = 1.0f/2.2f; else a_fGamma = 1.0f/a_fGamma;
		t.bR = a_tColor.fR >= 1.0f ? 255 : (a_tColor.fR <= 0.0f ? 0 : BYTE(powf(a_tColor.fR, a_fGamma)*255.0f+0.5f));
		t.bG = a_tColor.fG >= 1.0f ? 255 : (a_tColor.fG <= 0.0f ? 0 : BYTE(powf(a_tColor.fG, a_fGamma)*255.0f+0.5f));
		t.bB = a_tColor.fB >= 1.0f ? 255 : (a_tColor.fB <= 0.0f ? 0 : BYTE(powf(a_tColor.fB, a_fGamma)*255.0f+0.5f));
	}
	t.bA = a_tColor.fA >= 1.0f ? 255 : (a_tColor.fA <= 0.0f ? 0 : BYTE(a_tColor.fA*255.0f+0.5f));
	return t;
}
