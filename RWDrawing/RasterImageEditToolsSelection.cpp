// RasterImageEditToolsSelection.cpp : Implementation of CRasterImageEditToolsSelection

#include "stdafx.h"
#include "RasterImageEditToolsSelection.h"

#include "EditToolMetaSelect.h"
#include <PrintfLocalizedString.h>

HICON GetToolIconTRANSFORM(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const tl[] =
	{
		{96, 48, 0, 13.2548, 0, -13.2548},
		{72, 72, 0, 0, 13.2548, 0},
		{48, 72, 0, 0, 0, 0},
		{48, 96, 0, 13.2548, 0, 0},
		{24, 120, -13.2548, 0, 13.2548, 0},
		{0, 96, 0, 0, 0, 13.2548},
		{0, 48, 0, -13.2548, 0, 0},
		{24, 24, 0, 0, -13.2548, 0},
		{72, 24, 13.2548, 0, 0, 0},
	};
	static IRPathPoint const tr[] =
	{
		{256, 48, 0, 0, 0, -13.2548},
		{256, 96, 0, 13.2548, 0, 0},
		{232, 120, -13.2548, 0, 13.2548, 0},
		{208, 96, 0, 0, 0, 13.2548},
		{208, 72, 0, 0, 0, 0},
		{184, 72, -13.2548, 0, 0, 0},
		{160, 48, 0, -13.2548, 0, 13.2548},
		{184, 24, 0, 0, -13.2548, 0},
		{232, 24, 13.2548, 0, 0, 0},
	};
	static IRPathPoint const bl[] =
	{
		{48, 160, 0, 0, 0, -13.2548},
		{48, 184, 0, 0, 0, 0},
		{72, 184, 13.2548, 0, 0, 0},
		{96, 208, 0, 13.2548, 0, -13.2548},
		{72, 232, 0, 0, 13.2548, 0},
		{24, 232, -13.2548, 0, 0, 0},
		{0, 208, 0, 0, 0, 13.2548},
		{0, 160, 0, -13.2548, 0, 0},
		{24, 136, 13.2548, 0, -13.2548, 0},
	};
	static IRPathPoint const br[] =
	{
		{160, 208, 0, -13.2548, 0, 13.2548},
		{184, 184, 0, 0, -13.2548, 0},
		{208, 184, 0, 0, 0, 0},
		{208, 160, 0, -13.2548, 0, 0},
		{232, 136, 13.2548, 0, -13.2548, 0},
		{256, 160, 0, 0, 0, -13.2548},
		{256, 208, 0, 13.2548, 0, 0},
		{232, 232, 0, 0, 13.2548, 0},
		{184, 232, -13.2548, 0, 0, 0},
	};
	static IRGridItem gridX[] = { {0, 0}, {0, 48}, {0, 208}, {0, 256} };
	static IRGridItem gridY[] = { {0, 24}, {0, 72}, {0, 184}, {0, 232} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(tl), tl, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(tr), tr, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(bl), bl, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(br), br, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconMOVE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const move[] =
	{
		{0, 128}, {56, 184}, {56, 156}, {100, 156}, {100, 200}, {72, 200}, {128, 256}, {184, 200}, {156, 200}, {156, 156}, {200, 156}, {200, 184}, {256, 128}, {200, 72}, {200, 100}, {156, 100}, {156, 56}, {184, 56}, {128, 0}, {72, 56}, {100, 56}, {100, 100}, {56, 100}, {56, 72},
	};
	static IRGridItem grid[] = { {0, 0}, {0, 56}, {0, 100}, {0, 156}, {0, 200}, {0, 256} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(move), move, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconMAGICWAND(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const star1[] =
	{
		{238, 64}, {201, 128}, {238, 191}, {165, 191}, {128, 255}, {91, 191}, {18, 191}, {55, 128}, {18, 64}, {91, 64}, {128, 1}, {165, 64},
	};
	static IRPolyPoint const star2[] =
	{
		{201, 86}, {176, 128}, {201, 170}, {152, 170}, {128, 212}, {104, 170}, {55, 170}, {80, 128}, {55, 86}, {104, 86}, {128, 44}, {152, 86},
	};
	static IRPolyPoint const star3[] =
	{
		{164, 107}, {152, 128}, {164, 149}, {140, 149}, {128, 170}, {116, 149}, {92, 149}, {104, 128}, {92, 107}, {116, 107}, {128, 86}, {140, 107},
	};
	static IRPathPoint const top[] =
	{
		{34, 7, 6.24839, -6.24839, 0, 0},
		{56, 7, 0, 0, -6.24839, -6.24839},
		{97, 48, 6.24839, 6.24839, 0, 0},
		{97, 70, 0, 0, 6.24839, -6.24839},
		{70, 97, -6.24839, 6.24839, 0, 0},
		{48, 97, 0, 0, 6.24839, 6.24839},
		{7, 56, -6.24839, -6.24839, 0, 0},
		{7, 34, 0, 0, -6.24839, 6.24839},
		//{30, 3, 4.68629, -4.68629, 0, 0},
		//{46, 3, 0, 0, -4.68629, -4.68629},
		//{89, 46, 4.68629, 4.68629, 0, 0},
		//{89, 62, 0, 0, 4.68629, -4.68629},
		//{62, 89, -4.68629, 4.68629, 0, 0},
		//{46, 89, 0, 0, 4.68629, 4.68629},
		//{3, 46, -4.68629, -4.68629, 0, 0},
		//{3, 30, 0, 0, -4.68629, 4.68629},
	};
	static IRPolyPoint const mid[] =
	{
		{72, 35}, {221, 184}, {184, 221}, {35, 72},
		//{69, 38}, {218, 187}, {187, 218}, {38, 69},
	};
	static IRPathPoint const bot[] =
	{
		{186, 159, 6.24839, -6.24839, 0, 0},
		{208, 159, 0, 0, -6.24839, -6.24839},
		{249, 200, 6.24839, 6.24839, 0, 0},
		{249, 222, 0, 0, 6.24839, -6.24839},
		{222, 249, -6.24839, 6.24839, 0, 0},
		{200, 249, 0, 0, 6.24839, 6.24839},
		{159, 208, -6.24839, -6.24839, 0, 0},
		{159, 186, 0, 0, -6.24839, 6.24839},
		//{194, 167, 4.68629, -4.68629, 0, 0},
		//{210, 167, 0, 0, -4.68629, -4.68629},
		//{253, 210, 4.68629, 4.68629, 0, 0},
		//{253, 226, 0, 0, 4.68629, -4.68629},
		//{226, 253, -4.68629, 4.68629, 0, 0},
		//{210, 253, 0, 0, 4.68629, 4.68629},
		//{167, 210, -4.68629, -4.68629, 0, 0},
		//{167, 194, 0, 0, -4.68629, 4.68629},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	IRTarget magic(0.7f, -1, -1);
	IRTarget wand(0.8f, 1, 1);
	IRFill lightMat(0x55f8fb7c);
	IRFill magicFillMat(0x55ebd467);
	IROutlinedFill magicMat(&magicFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(star1), star1, &magicMat, magic);
	cRenderer(&canvas, itemsof(star2), star2, &lightMat, magic);
	cRenderer(&canvas, itemsof(star3), star3, &lightMat, magic);
	cRenderer(&canvas, itemsof(mid), mid, pSI->GetMaterial(ESMScheme2Color2), wand);
	cRenderer(&canvas, itemsof(top), top, pSI->GetMaterial(ESMScheme2Color1), wand);
	cRenderer(&canvas, itemsof(bot), bot, pSI->GetMaterial(ESMScheme2Color1), wand);
	return cRenderer.get();
}

HICON GetToolIconSIMPLESELECT(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const rect[] =
	{
		{0, 0}, {180, 0}, {180, 128}, {0, 128},
	};
	static IRPolyPoint const arrow[] =
	{
		{174, 123}, {174, 238.2}, {198, 214.2}, {213, 250.2}, {237, 240.6}, {222, 204.6}, {255.6, 204.6},
	};
	static IRGridItem gridX[] = { {0, 0}, {0, 180} };
	static IRGridItem gridY[] = { {0, 0}, {0, 128} };
	static IRCanvas canvasRect = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	static IRCanvas canvasArrow = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvasRect, itemsof(rect), rect, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvasArrow, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	return cRenderer.get();
}


// CRasterImageEditToolsSelection

STDMETHODIMP CRasterImageEditToolsSelection::ToolIDsEnum(IRasterImageEditToolsManager* a_pManager, IEnumStrings** a_ppToolIDs)
{
	try
	{
		HRESULT hRes = CImpl::ToolIDsEnum(a_pManager, a_ppToolIDs);
		CComQIPtr<IEnumStringsInit>(*a_ppToolIDs)->Insert(CComBSTR(L"SELECT_"));
		return hRes;
	}
	catch (...)
	{
		return a_ppToolIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsSelection::ToolNameGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CPrintfLocalizedString>* p = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_METASELECT), pInternal);
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

STDMETHODIMP CRasterImageEditToolsSelection::ToolDescGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CPrintfLocalizedString>* p = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLDESC_METASELECT), pInternal);
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

STDMETHODIMP CRasterImageEditToolsSelection::ToolIconIDGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, GUID* a_ptDefaultIcon)
{
	try
	{
		*a_ptDefaultIcon = GUID_NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			GUID tIcon = GUID_NULL;
			HRESULT hRes = a_pManager->ToolIconIDGet(bstrInternal, &tIcon);
			if (FAILED(hRes) || IsEqualGUID(tIcon, GUID_NULL))
				return hRes;
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
			//static GUID const tIconID = {0xb33b0053, 0x7a01, 0x4504, {0x92, 0x45, 0x1, 0xf5, 0xe3, 0x63, 0xae, 0x6b}};
			//*a_ptDefaultIcon = tIconID;
			//return S_OK;
		}
		return CImpl::ToolIconIDGet(a_pManager, a_bstrID, a_ptDefaultIcon);
	}
	catch (...)
	{
		return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsSelection::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			HICON hIcon = NULL;
			HRESULT hRes = a_pManager->ToolIconGet(bstrInternal, a_nSize, &hIcon);
			if (FAILED(hRes) || hIcon == NULL)
				return hRes;
			*a_phIcon = CreateSelectIcon(hIcon);
			return S_OK;
			//*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_EDITTOOL_MOVESELECT), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			//return S_OK;
		}
		if (wcscmp(a_bstrID, L"TRANSFORM") == 0)
		{
			*a_phIcon = GetToolIconTRANSFORM(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"MOVE") == 0)
		{
			*a_phIcon = GetToolIconMOVE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"MAGICWAND") == 0)
		{
			*a_phIcon = GetToolIconMAGICWAND(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"SIMPLESELECT") == 0 || wcscmp(a_bstrID, L"SELECT") == 0)
		{
			*a_phIcon = GetToolIconSIMPLESELECT(a_nSize);
			return S_OK;
		}
		return CImpl::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsSelection::SupportedStates(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
{
	try
	{
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			if (a_pBlendingModes) *a_pBlendingModes = 0;
			return a_pManager->SupportedStates(bstrInternal, NULL, a_pRasterizationModes, a_pCoordinatesModes, NULL);
		}
		return CImpl::SupportedStates(a_pManager, a_bstrID, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageEditToolsSelection::EditToolCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool)
{
	try
	{
		*a_ppTool = NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolMetaSelect>* p = NULL;
			CComObject<CEditToolMetaSelect>::CreateInstance(&p);
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

STDMETHODIMP CRasterImageEditToolsSelection::WindowCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		if (wcsncmp(a_bstrID, L"SELECT_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			return a_pManager->WindowCreate(bstrInternal, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
		}
		return CImpl::WindowCreate(a_pManager, a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
	}
	catch (...)
	{
		return a_ppWindow ? E_UNEXPECTED : E_POINTER;
	}
}

HICON CRasterImageEditToolsSelection::CreateSelectIcon(HICON a_hSource)
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
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pIconRes.m_p+sizeof BITMAPINFOHEADER))
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // failure
	}

	DeleteObject(tInfo.hbmColor);

	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pHead->biSize = sizeof(BITMAPINFOHEADER);
	pHead->biWidth = nSmSizeX;
	pHead->biHeight = nSmSizeY<<1;
	pHead->biPlanes = 1;
	pHead->biBitCount = 32;
	pHead->biCompression = BI_RGB;
	pHead->biSizeImage = nXOR+nAND;
	pHead->biXPelsPerMeter = 0;
	pHead->biYPelsPerMeter = 0;
	TRasterImagePixel *pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes.m_p+sizeof BITMAPINFOHEADER);
	COLORREF clr = GetSysColor(COLOR_MENUHILIGHT);
	TRasterImagePixel const tColor = *reinterpret_cast<TRasterImagePixel*>(&clr);
	for (int y = 0; y < tBmp.bmHeight; ++y)
	{
		for (int x = 0; x < tBmp.bmWidth; ++x)
		{
			ULONG const n = max(pXOR->bG, max(pXOR->bR, pXOR->bB));//5*pXOR->bG + 3*pXOR->bR + 2*pXOR->bB;
			ULONG const nR = (tColor.bB*n)/220;
			ULONG const nG = (tColor.bG*n)/220;
			ULONG const nB = (tColor.bR*n)/220;
			pXOR->bR = nR > 255 ? 255 : nR;
			pXOR->bG = nG > 255 ? 255 : nG;
			pXOR->bB = nB > 255 ? 255 : nB;
			pXOR->bA = (pXOR->bA*5)>>3;
			++pXOR;
		}
	}
	pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes+sizeof BITMAPINFOHEADER);
	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
	int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	for (int y = 0; y < nSmSizeY; ++y)
	{
		if (y < (nSmSizeY>>1))
		{
			for (int x = 0; x < nSmSizeX; ++x)
			{
				TRasterImagePixel t = pXOR[x];
				pXOR[x] = pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x];
				pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x] = t;
			}
		}
		for (int x = 0; x < nSmSizeX; ++x)
		{
			if (0 == pXOR->bA)
			{
				pAND[x>>3] |= 0x80 >> (x&7);
			}
			++pXOR;
		}
		pAND += nANDLine;
	}
	HICON hTmp = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	DestroyIcon(a_hSource);
	return hTmp;
}
