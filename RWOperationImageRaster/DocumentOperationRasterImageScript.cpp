// DocumentOperationRasterImageScript.cpp : Implementation of CDocumentOperationRasterImageScript

#include "stdafx.h"
#include "DocumentOperationRasterImageScript.h"

#include <SharedStringTable.h>
#include "ScriptedRasterImage.h"
#include "ScriptedTool.h"
#include "ScriptedBlender.h"


// CDocumentOperationRasterImageScript

STDMETHODIMP CDocumentOperationRasterImageScript::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		{
			CComObject<CScriptedTool>* p = NULL;
			CComObject<CScriptedTool>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			//p->Init(, );
			a_pSite->AddItem(CComBSTR(L"DrawTool"), pTmp);
		}
		{
			CComObject<CScriptedBlender>* p = NULL;
			CComObject<CScriptedBlender>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			//p->Init(, );
			a_pSite->AddItem(CComBSTR(L"Blender"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageScript::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IDocumentRasterImage> pDRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));
		if (pDRI)
		{
			CComObject<CScriptedRasterImage>* p = NULL;
			CComObject<CScriptedRasterImage>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pDocument, pDRI);
			a_pSite->AddItem(CComBSTR(L"RasterImage"), pTmp);
		}
		//else
		//{
		//	CComPtr<IDocumentImage> pInDoc;
		//	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pInDoc));
		//	if (pInDoc)
		//	{
		//		CComPtr<IDocumentFactoryRasterImage> pImageFact;
		//		RWCoCreateInstance(pImageFact, __uuidof(DocumentFactoryRasterImage));

		//		CImageFormat cFmt;
		//		pInDoc->FormatGet(cFmt);
		//		bool bHasAlphaChannel =
		//			(cFmt->atPSps[0].eCCHint == ECCAlpha && cFmt->atPSps[0].nWidth != 0) ||
		//			(cFmt->atPSps[1].eCCHint == ECCAlpha && cFmt->atPSps[1].nWidth != 0) ||
		//			(cFmt->atPSps[2].eCCHint == ECCAlpha && cFmt->atPSps[2].nWidth != 0) ||
		//			(cFmt->atPSps[3].eCCHint == ECCAlpha && cFmt->atPSps[3].nWidth != 0);
		//		cFmt->atDims[0].nAlignment = 4;
		//		cFmt->atDims[1].nAlignment = 4;
		//		cFmt->atDims[2].nAlignment = 4;
		//		cFmt->atDims[3].nAlignment = 4;
		//		cFmt->atPSps[0].eCCHint = ECCBlue;
		//		cFmt->atPSps[0].nOffset = 16;
		//		cFmt->atPSps[0].nWidth = 8;
		//		cFmt->atPSps[1].eCCHint = ECCGreen;
		//		cFmt->atPSps[1].nOffset = 8;
		//		cFmt->atPSps[1].nWidth = 8;
		//		cFmt->atPSps[2].eCCHint = ECCRed;
		//		cFmt->atPSps[2].nOffset = 0;
		//		cFmt->atPSps[2].nWidth = 8;
		//		cFmt->atPSps[3].eCCHint = bHasAlphaChannel ? ECCAlpha : ECCEmpty;
		//		cFmt->atPSps[3].nOffset = bHasAlphaChannel ? 24 : 0;
		//		cFmt->atPSps[3].nWidth = bHasAlphaChannel ? 8 : 0;
		//		cFmt->dwFormatID = EImgFmtRAW;
		//		cFmt->nPixelSize = 32;
		//		size_t nPixels = cFmt->atDims[0].nItems*cFmt->atDims[1].nItems*cFmt->atDims[2].nItems*cFmt->atDims[3].nItems;
		//		cFmt->nDataSize = nPixels<<2;
		//		CComPtr<IImageSource> pImgSrc;
		//		pInDoc->ImageSourceGet(cFmt, &pImgSrc);
		//		CAutoVectorPtr<TRasterImagePixel> pData(new TRasterImagePixel[nPixels]);
		//		pImgSrc->DataGet(CImageSinkOnMemory(reinterpret_cast<BYTE*>(pData.m_p), cFmt->nDataSize));
		//		if (!bHasAlphaChannel)
		//		{
		//			for (size_t i = 0; i < nPixels; i++)
		//			{
		//				pData[i].bA = 0xff;
		//			}
		//		}
		//		TRasterImageCoord const tSize = {cFmt->atDims[0].nItems, cFmt->atDims[1].nItems, cFmt->atDims[2].nItems, cFmt->atDims[3].nItems};

		//		CComPtr<IDocumentBase> pBase;
		//		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		//		if (FAILED(pImageFact->Create(&tSize, nPixels, pData, ERIAMAuto, NULL, pBase)))
		//			return E_FAIL;
		//		CComQIPtr<IDocument> pOutDoc(pBase);
		//		CComPtr<IDocumentRasterImage> pDRI;
		//		pOutDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));

		//		CComPtr<IRasterImageControl> pRIC;
		//		pOutDoc->QueryFeatureInterface(__uuidof(IRasterImageControl), reinterpret_cast<void**>(&pRIC));
		//		CComPtr<IRasterImageScaledPreview> pSP;
		//		pOutDoc->QueryFeatureInterface(__uuidof(IRasterImageScaledPreview), reinterpret_cast<void**>(&pSP));

		//		CComObject<CScriptedRasterImage>* p = NULL;
		//		CComObject<CScriptedRasterImage>::CreateInstance(&p);
		//		CComPtr<IDispatch> pTmp = p;
		//		p->Init(pOutDoc, pDRI, pRIC, pSP);
		//		a_pSite->AddItem(CComBSTR(L"RasterImage"), pTmp);
		//	}
		//}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageScript::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pPrimary->Insert(CComBSTR(L"DrawTool"));
		a_pPrimary->Insert(CComBSTR(L"Blender"));
		a_pSecondary->Insert(CComBSTR(L"RasterImage"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON GetIconPicture(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	pSI->GetLayers(ESIPicture, cRenderer);
	return cRenderer.get();
}

HICON GetIconSaveImage(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	pSI->GetLayers(ESIPicture, cRenderer, IRTarget(1.0f, -1, 1));
	pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.65f, 1, -1));
	return cRenderer.get();
}

HICON GetIconCanvas(ULONG a_nSize)
{
	static IRPolyPoint const plank1[] = { {62, 204}, {194, 204}, {194, 228}, {62, 228} };
	static IRPolyPoint const plank2[] = { {221, 256}, {189, 256}, {128, 27}, {67, 256}, {35, 256}, {109, 0}, {147, 0} };
	static IRPolyPoint const canvas[] = { {23, 26}, {233, 26}, {233, 184}, {23, 184} };
	static IRPolyPoint const plank3[] = { {12, 170}, {244, 170}, {244, 198}, {12, 198} };
	static IRGridItem const grid1[] = { {0, 204}, {0, 228} };
	static IRCanvas const canvas1 = {0, 0, 256, 256, 0, itemsof(grid1), NULL, grid1};
	static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	static IRGridItem const grid3x[] = { {0, 23}, {0, 233} };
	static IRGridItem const grid3y[] = { {0, 26}, {0, 184} };
	static IRCanvas const canvas3 = {0, 0, 256, 256, itemsof(grid3x), itemsof(grid3y), grid3x, grid3y};
	static IRGridItem const grid4x[] = { {0, 12}, {0, 244} };
	static IRGridItem const grid4y[] = { {0, 170}, {0, 198} };
	static IRCanvas const canvas4 = {0, 0, 256, 256, itemsof(grid4x), itemsof(grid4y), grid4x, grid4y};
	static IRPathPoint const stroke[] =
	{
		{36, 136, -0.741158, -20.0113, 1, 27},
		{81, 54, 20, -15, -12.4964, 9.3723},
		{112, 72, 20, -16, 10, -26},
		{145, 78, 15, -19, 3, -16},
		{181, 80, 23, -29, 2, -15},
		{217, 88, -5, 15, 6.40312, -19.2094},
		{185, 136, -13.375, 9.25, 13.375, -9.25},
		{158, 127, -16, 25, -1, 10},
		{118, 132, -17, 22, 0, 15},
		{81, 133, -19, 31, -1, 22},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas1, itemsof(plank1), plank1, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas2, itemsof(plank2), plank2, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas3, itemsof(canvas), canvas, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas4, itemsof(plank3), plank3, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas2, itemsof(stroke), stroke, pSI->GetMaterial(ESMScheme1Color1), IRTarget(0.9f));
	return cRenderer.get();
}

HICON GetIconResample(ULONG a_nSize)
{
	static IRPolyPoint const smooth[] = { {224, 42}, {224, 224}, {42, 224} };
	static IRPolyPoint const jagged[] = {
		{157, 44}, {134, 21}, {112, 44}, {89, 21}, {66, 44}, {44, 21}, {21, 44}, {44, 66}, {21, 89}, {44, 112}, {21, 134}, {44, 157}, {21, 180}, {44, 202}, {202, 44}, {180, 21}
	};
	static IRGridItem const grid[] = { {0, 32}, {0, 224} };
	static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
	static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas1, itemsof(smooth), smooth, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas2, itemsof(jagged), jagged, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}

HICON GetIconBorders(ULONG a_nSize)
{
	static IRPolyPoint const top[] = { {64, 64}, {16, 16}, {240, 16}, {192, 64} };
	static IRPolyPoint const bot[] = { {192, 192}, {240, 240}, {16, 240}, {64, 192} };
	static IRPolyPoint const lft[] = { {48, 176}, {0, 224}, {0, 32}, {48, 80} };
	static IRPolyPoint const rgt[] = { {208, 80}, {256, 32}, {256, 224}, {208, 176} };
	static IRGridItem const gridX[] = { {0, 0}, {0, 48}, {0, 208}, {0, 256} };
	static IRGridItem const gridY[] = { {0, 16}, {0, 64}, {0, 224}, {0, 240} };
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(top), top, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(bot), bot, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(lft), lft, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(rgt), rgt, pSI->GetMaterial(ESMInterior));
	pSI->GetLayers(ESIPicture|ESISimplified, cRenderer, IRTarget(0.5f));
	return cRenderer.get();
}

HICON GetIconAutoContrast(ULONG a_nSize)
{
	IRPathPoint const white[] =
	{
		{164, 128, 0, -39.7645, 0, 39.7645},
		{128, 56, -39.7645, 0, 39.7645, 0},
		{56, 128, 0, 39.7645, 0, -39.7645},
		{128, 200, 39.7645, 0, -39.7645, 0},
	};
	IRPathPoint const black[] =
	{
		{200, 128, 0, -39.7645, 0, 39.7645},
		{128, 56, 0, 0, 39.7645, 0},
		{128, 200, 39.7645, 0, 0, 0},
	};

	IRCanvas const canvas1 = {56, 56, 200, 200, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	IRTarget target(0.5f, -1, 1);
	cRenderer(&canvas1, itemsof(white), white, pSI->GetMaterial(ESMBackground), target);
	cRenderer(&canvas1, itemsof(black), black, pSI->GetMaterial(ESMContrast), target);

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
	};
	static IRPolyPoint const mid[] =
	{
		{72, 35}, {221, 184}, {184, 221}, {35, 72},
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
	};
	static IRCanvas canvas2 = {7, 7, 249, 249, 0, 0, NULL, NULL};
	IRTarget wand(0.9f, 1, -1);
	cRenderer(&canvas2, itemsof(mid), mid, pSI->GetMaterial(ESMScheme1Color2), wand);
	cRenderer(&canvas2, itemsof(top), top, pSI->GetMaterial(ESMScheme1Color1), wand);
	cRenderer(&canvas2, itemsof(bot), bot, pSI->GetMaterial(ESMScheme1Color1), wand);
	return cRenderer.get();
}

HICON GetIconCheckmark(ULONG a_nSize)
{
	static IRPathPoint const area[] =
	{
		{0, 56, 0, -13.2548, 0, 0},
		{24, 32, 0, 0, -13.2548, 0},
		{184, 32, 13.2548, 0, 0, 0},
		{208, 56, 0, 0, 0, -13.2548},
		{208, 216, 0, 13.2548, 0, 0},
		{184, 240, 0, 0, 13.2548, 0},
		{24, 240, -13.2548, 0, 0, 0},
		{0, 216, 0, 0, 0, 13.2548},
	};
	static IRPolyPoint const check[] =
	{
		{24, 112}, {76, 60}, {120, 104}, {204, 20}, {256, 72}, {120, 208}
	};
	static IRGridItem const gridX[] = { {0, 0}, {0, 208} };
	static IRGridItem const gridY[] = { {0, 32}, {0, 240} };
	static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	static IRCanvas const canvas2 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas1, itemsof(area), area, pSI->GetMaterial(ESMAltBackground/*ESMScheme2Color2*/), IRTarget(0.875f));
	cRenderer(&canvas2, itemsof(check), check, pSI->GetMaterial(ESMScheme2Color1), IRTarget(0.875f));
	return cRenderer.get();
}

HICON GetIconMirror(ULONG a_nSize)
{
	static IRPathPoint const arrow[] =
	{
		{213, 59, 0, 0, -60, -31},
		{226, 33, 0, 0, 0, 0},
		{255, 100, 0, 0, 0, 0},
		{183, 114, 0, 0, 0, 0},
		{197, 88, -48, -24, 0, 0},
		{59, 88, 0, 0, 48, -24},
		{73, 114, 0, 0, 0, 0},
		{1, 100, 0, 0, 0, 0},
		{30, 33, 0, 0, 0, 0},
		{43, 59, 60, -31, 0, 0},
	};
	static IRPolyPoint const axis[] =
	{
		{128, 100}, {128, 256}
	};
	static IRGridItem const grid[] = { {0, 1}, {1|EGIFMidPixel, 128}, {1, 255} };
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), 0, grid, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(axis), axis, pSI->GetMaterial(ESMOutlineSoft));
	return cRenderer.get();
}

HICON GetIconFlip(ULONG a_nSize)
{
	static IRPathPoint const arrow[] =
	{
		{59, 213, 0, 0, -31, -60},
		{33, 226, 0, 0, 0, 0},
		{100, 255, 0, 0, 0, 0},
		{114, 183, 0, 0, 0, 0},
		{88, 197, -24, -48, 0, 0},
		{88, 59, 0, 0, -24, 48},
		{114, 73, 0, 0, 0, 0},
		{100, 1, 0, 0, 0, 0},
		{33, 30, 0, 0, 0, 0},
		{59, 43, -31, 60, 0, 0},
	};
	static IRPolyPoint const axis[] =
	{
		{100, 128}, {256, 128}
	};
	static IRGridItem const grid[] = { {0, 1}, {1|EGIFMidPixel, 128}, {1, 255} };
	static IRCanvas const canvas = {0, 0, 256, 256, 0, itemsof(grid), NULL, grid};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(arrow), arrow, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(axis), axis, pSI->GetMaterial(ESMOutlineSoft));
	return cRenderer.get();
}

