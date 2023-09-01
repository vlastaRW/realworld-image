// DocumentOperationRasterImageRemoveBorder.cpp : Implementation of CDocumentOperationRasterImageRemoveBorder

#include "stdafx.h"
#include "DocumentOperationRasterImageRemoveBorder.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>

const OLECHAR CFGID_RELSIZE[] = L"RelSize";
const OLECHAR CFGID_ALIGNMENTX[] = L"AlignX";
const OLECHAR CFGID_ALIGNMENTY[] = L"AlignY";
const OLECHAR CFGID_SQUARE[] = L"Square";

#include "ConfigGUIRemoveBorder.h"


// CDocumentOperationRasterImageRemoveBorder

STDMETHODIMP CDocumentOperationRasterImageRemoveBorder::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Remove Empty Border[0405]Rastrový obrázek - odstranit prázdné okraje");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRemoveBorder::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_RELSIZE), _SharedStringTable.GetStringAuto(IDS_CFGID_RELSIZE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RELSIZE_DESC), CConfigValue(100.0f), NULL, CConfigValue(10.0f), CConfigValue(100.0f), CConfigValue(1.0f), 0, NULL);

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_ALIGNMENTX), CMultiLanguageString::GetAuto(L"[0409]Horizontal alignment[0405]Horizontální zarovnání"), CMultiLanguageString::GetAuto(L"[0409]How to place the image on the canvas.[0405]Jak umístit obrázek na plátno."), CConfigValue(0.0f), NULL, CConfigValue(-1.0f), CConfigValue(1.0f), CConfigValue(0.0f), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_ALIGNMENTY), CMultiLanguageString::GetAuto(L"[0409]Vertical alignment[0405]Vertikální zarovnání"), CMultiLanguageString::GetAuto(L"[0409]How to place the image on the canvas.[0405]Jak umístit obrázek na plátno."), CConfigValue(0.0f), NULL, CConfigValue(-1.0f), CConfigValue(1.0f), CConfigValue(0.0f), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SQUARE), _SharedStringTable.GetStringAuto(IDS_CFGID_SQUARE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SQUARE_DESC), CConfigValue(false), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageRemoveBorder, CConfigGUIRemoveBorder>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRemoveBorder::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFtsEdt[] = {__uuidof(IDocumentEditableImage)};
		static IID const aFtsAni[] = {__uuidof(IDocumentAnimation)};
		return (SupportsAllFeatures(a_pDocument, itemsof(aFtsEdt), aFtsEdt) ||
			SupportsAllFeatures(a_pDocument, itemsof(aFtsAni), aFtsAni)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool DetectBounds(IConfig* a_pConfig, TImageSize const& tSize, TImagePoint const& tContentOrigin, TImageSize const& tContentSize, TImagePoint& tOffset, TImageSize& tNewSize)
{
	if (tContentSize.nX == 0 || tContentSize.nY == 0)
		return false;
	LONG nXMin = tContentOrigin.nX;
	LONG nXMax = tContentOrigin.nX+tContentSize.nX-1;
	LONG nYMin = tContentOrigin.nY;
	LONG nYMax = tContentOrigin.nY+tContentSize.nY-1;

	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_RELSIZE), &cVal);
	float const fRelativeSize = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_ALIGNMENTX), &cVal);
	float const fAlignmentX = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_ALIGNMENTY), &cVal);
	float const fAlignmentY = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_SQUARE), &cVal);
	bool const bSquare = cVal;

	LONG nObjectX = nXMax-nXMin+1;
	LONG nObjectY = nYMax-nYMin+1;
	LONG nCanvasX = static_cast<LONG>(0.5f+(nObjectX*100.0f)/fRelativeSize);
	LONG nCanvasY = static_cast<LONG>(0.5f+(nObjectY*100.0f)/fRelativeSize);
	tOffset.nX = -nXMin;
	tOffset.nY = -nYMin;
	if (bSquare)
	{
		if (nCanvasX > nCanvasY)
			nCanvasY = nCanvasX;
		else
			nCanvasX = nCanvasY;
	}
	tNewSize.nX = nCanvasX;
	tNewSize.nY = nCanvasY;

	tOffset.nX += static_cast<LONG>(0.5f+(fAlignmentX*0.5f+0.5f)*(nCanvasX-nObjectX));
	tOffset.nY += static_cast<LONG>(0.5f+(fAlignmentY*0.5f+0.5f)*(nCanvasY-nObjectY));

	if (tNewSize.nX == tSize.nX && tNewSize.nY == tSize.nY && tOffset.nX == 0 && tOffset.nY == 0)
		return false; // no change
	return true;
}

TImageSize GetAnimationSize(IDocumentAnimation* a_pAni, TImageResolution* a_pRes = NULL); // implemented in canvas size op

HRESULT RemoveBorderFrame(IDocument* a_pDocument, IConfig* a_pConfig, TImageSize const* a_pNewSize = NULL)
{
	CComPtr<IDocumentEditableImage> pEI;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
	if (pEI == NULL)
		return E_FAIL;

	TImageSize tSize = {1, 1};
	TImagePoint tContentOrigin = {0, 0};
	TImageSize tContentSize = {0, 0};
	pEI->CanvasGet(&tSize, NULL, &tContentOrigin, &tContentSize, NULL);

	TImagePoint tOffset;
	TImageSize tNewSize;
	if (!DetectBounds(a_pConfig, tSize, tContentOrigin, tContentSize, tOffset, tNewSize))
		return S_FALSE;

	TMatrix3x3f const tX = {1.0f, 0.0f, 0.0f,  0.0f, 1.0, 0.0f,  tOffset.nX, tOffset.nY, 1.0f};
	return pEI->CanvasSet(&tNewSize, NULL, &tX, NULL);
}

HRESULT RemoveBorderFrame(IDocument* a_pDocument, TImageSize const& a_tNewSize, TImagePoint const& a_tOffset)
{
	CComPtr<IDocumentEditableImage> pEI;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));

	TMatrix3x3f tX = {1.0f, 0.0f, 0.0f,  0.0f, 1.0, 0.0f,  a_tOffset.nX, a_tOffset.nY, 1.0f};
	return pEI->CanvasSet(&a_tNewSize, NULL, &tX, NULL);
}

STDMETHODIMP CDocumentOperationRasterImageRemoveBorder::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<IDocumentAnimation> pA;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
		{
			// layered image or raster image
			return RemoveBorderFrame(a_pDocument, a_pConfig);
		}

		// animation
		TImageSize tAniSize = GetAnimationSize(pA);
		if (tAniSize.nX*tAniSize.nY == 0)
			return E_FAIL;
		CComPtr<IEnumUnknowns> pFrames;
		pA->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		if (pFrames) pFrames->Size(&nFrames);
		if (nFrames == 0)
			return S_FALSE;
		CWriteLock<IDocument> cLock(a_pDocument);
		TImagePoint t0 = {LONG_MAX, LONG_MAX};
		TImagePoint t1 = {LONG_MIN, LONG_MIN};
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			pA->FrameGetDoc(pFrame, &pFrameDoc);

			CComPtr<IDocumentImage> pI;
			pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
			if (pI == NULL)
				return E_FAIL;

			TImageSize tSize = {0, 0};
			TImagePoint tContentOrigin = {0, 0};
			TImageSize tContentSize = {0, 0};
			if (FAILED(pI->CanvasGet(&tSize, NULL, &tContentOrigin, &tContentSize, NULL)))
				return E_FAIL;
			tContentOrigin.nX -= tSize.nX>>1;
			tContentOrigin.nY -= tSize.nY>>1;
			if (t0.nX > tContentOrigin.nX) t0.nX = tContentOrigin.nX;
			if (t0.nY > tContentOrigin.nY) t0.nY = tContentOrigin.nY;
			if (t1.nX < LONG(tContentOrigin.nX+tContentSize.nX)) t1.nX = tContentOrigin.nX+tContentSize.nX;
			if (t1.nY < LONG(tContentOrigin.nY+tContentSize.nY)) t1.nY = tContentOrigin.nY+tContentSize.nY;
		}
		if (t1.nX <= t0.nX) t1.nX = t0.nX+1;
		if (t1.nY <= t0.nY) t1.nY = t0.nY+1;
		t0.nX += tAniSize.nX>>1;
		t0.nY += tAniSize.nY>>1;
		t1.nX += tAniSize.nX>>1;
		t1.nY += tAniSize.nY>>1;
		TImageSize t10 = {t1.nX-t0.nX, t1.nY-t0.nY};
		TImagePoint tNewOff = {0, 0};
		TImageSize tNewSize = tAniSize;
		DetectBounds(a_pConfig, tAniSize, t0, t10, tNewOff, tNewSize);
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			pA->FrameGetDoc(pFrame, &pFrameDoc);
			HRESULT hRes = RemoveBorderFrame(pFrameDoc, tNewSize, tNewOff);
			if (FAILED(hRes))
				return hRes;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageRemoveBorder::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Remove border[0405]Odstranit okraj");
			return S_OK;
		}

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_RELSIZE), &cVal);
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pSize = pStr;
		pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Object size %i%%[0405]Velikost objektu %i%%"), cVal.operator LONG());

		a_pConfig->ItemValueGet(CComBSTR(CFGID_SQUARE), &cVal);
		if (cVal.operator bool())
		{
			CComPtr<ILocalizedString> pSquare;
			pSquare.Attach(_SharedStringTable.GetString(IDS_CFGID_SQUARE_NAME));

			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			CComPtr<ILocalizedString> pTmp = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
			pStr->Init(pTempl, pSize, pSquare);
			*a_ppName = pTmp.Detach();
		}
		else
		{
			*a_ppName = pSize.Detach();
		}
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

