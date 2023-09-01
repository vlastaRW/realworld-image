// DocumentOperationRasterImageCanvasSize.cpp : Implementation of CDocumentOperationRasterImageCanvasSize

#include "stdafx.h"
#include "DocumentOperationRasterImageCanvasSize.h"
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>

const OLECHAR CFGID_CS_TYPE[] = L"Type";
const LONG CFGVAL_CST_DELTA = 0;
const LONG CFGVAL_CST_SIZE = 1;
const OLECHAR CFGID_CS_DELTA_X[] = L"DeltaX";
const OLECHAR CFGID_CS_DELTA_Y[] = L"DeltaY";
const OLECHAR CFGID_CS_SIZE_X[] = L"SizeX";
const OLECHAR CFGID_CS_SIZE_Y[] = L"SizeY";
const OLECHAR CFGID_CS_LASTSIZE_X[] = L"LastSizeX";
const OLECHAR CFGID_CS_LASTSIZE_Y[] = L"LastSizeY";
const OLECHAR CFGID_CS_POSITION[] = L"Align";

TImageSize GetAnimationSize(IDocumentAnimation* a_pAni, TImageResolution* a_pRes = NULL)
{
	TImageSize tSize = {0, 0};
	CComPtr<IEnumUnknowns> pFrames;
	a_pAni->FramesEnum(&pFrames);
	ULONG nFrames = 0;
	if (pFrames) pFrames->Size(&nFrames);
	for (ULONG i = 0; i < nFrames; ++i)
	{
		CComPtr<IUnknown> pFrame;
		pFrames->Get(i, &pFrame);
		CComPtr<IDocument> pFrameDoc;
		a_pAni->FrameGetDoc(pFrame, &pFrameDoc);
		if (pFrameDoc == NULL) continue;
		CComPtr<IDocumentImage> pFrameImg;
		pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
		if (pFrameImg == NULL) continue;
		TImageSize tIS = {0, 0};
		pFrameImg->CanvasGet(&tIS, a_pRes, NULL, NULL, NULL);
		if (tIS.nX > tSize.nX) tSize.nX = tIS.nX;
		if (tIS.nY > tSize.nY) tSize.nY = tIS.nY;
	}
	return tSize;
}

#include "ConfigGUICanvasSize.h"

// CDocumentOperationRasterImageCanvasSize

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Canvas Size[0405]Rastrový obrázek - velikost plátna");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_CS_TYPE(CFGID_CS_TYPE);
		pCfgInit->ItemIns1ofN(cCFGID_CS_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_CS_TYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_TYPE_DESC), CConfigValue(CFGVAL_CST_DELTA), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CS_TYPE, CConfigValue(CFGVAL_CST_DELTA), _SharedStringTable.GetStringAuto(IDS_CFGVAL_CST_DELTA), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_CS_TYPE, CConfigValue(CFGVAL_CST_SIZE), _SharedStringTable.GetStringAuto(IDS_CFGVAL_CST_SIZE), 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_CS_TYPE;
		tCond.eConditionType = ECOCEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = CFGVAL_CST_DELTA;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_DELTA_X), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_DELTA_X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_DELTA_DESC), CConfigValue(0L), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_DELTA_Y), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_DELTA_Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_DELTA_DESC), CConfigValue(0L), NULL, 1, &tCond);
		tCond.tValue.iVal = CFGVAL_CST_SIZE;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_SIZE_X), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_SIZE_X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_SIZE_DESC), CConfigValue(800L), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_SIZE_Y), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_SIZE_Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_SIZE_DESC), CConfigValue(600L), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_POSITION), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_POSITION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_CS_POSITION_DESC), CConfigValue(0.0f, 0.0f), NULL, 0, NULL);

		CComPtr<ILocalizedString> pDummy;
		RWCoCreateInstance(pDummy, __uuidof(LocalizedString));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_LASTSIZE_X), pDummy, pDummy, CConfigValue(0L), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CS_LASTSIZE_Y), pDummy, pDummy, CConfigValue(0L), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageCanvasSize, CConfigGUICanvasSize>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
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

static bool GetNewCanvasSize(TImageSize const& tSize, bool a_bRelative, LONG a_nSizeX, LONG a_nSizeY, float a_fAlignX, float a_fAlignY, TImageSize& tNewSize, TImagePoint& tOffset)
{
	tNewSize.nX = tSize.nX;
	tNewSize.nY = tSize.nY;
	if (a_bRelative)
	{
		if (LONG(tNewSize.nX + a_nSizeX) <= 0)
			tNewSize.nX = 1;
		else
			tNewSize.nX += a_nSizeX;
		if (LONG(tNewSize.nY + a_nSizeY) <= 0)
			tNewSize.nY = 1;
		else
			tNewSize.nY += a_nSizeY;
	}
	else
	{
		if (a_nSizeX <= 0)
			tNewSize.nX = 1;
		else
			tNewSize.nX = a_nSizeX;
		if (a_nSizeY <= 0)
			tNewSize.nY = 1;
		else
			tNewSize.nY = a_nSizeY;
	}

	tOffset.nX = (a_fAlignX+1.0f)*0.5f*(LONG(tNewSize.nX)-LONG(tSize.nX));
	tOffset.nY = (a_fAlignY+1.0f)*0.5f*(LONG(tNewSize.nY)-LONG(tSize.nY));
	return tSize.nX != tNewSize.nX || tSize.nY != tNewSize.nY;
}

HRESULT ResizeFrame(IDocument* a_pDocument, LONG a_nValType, LONG a_nValSizeX, LONG a_nValSizeY, float a_fAlignX, float a_fAlignY, TImageSize const* a_pNewSize = NULL)
{
	CComPtr<IDocumentEditableImage> pEI;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
	if (pEI == NULL)
		return E_FAIL;

	TImageSize tSize = {1, 1};
	pEI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

	TImageSize tNewSize;
	TImagePoint tOffset;
	if (!GetNewCanvasSize(a_pNewSize ? *a_pNewSize : tSize, a_nValType == CFGVAL_CST_DELTA, a_nValSizeX, a_nValSizeY, a_fAlignX, a_fAlignY, tNewSize, tOffset))
		return S_FALSE;

	TMatrix3x3f tX = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  tOffset.nX, tOffset.nY, 1.0f};
	return pEI->CanvasSet(&tNewSize, NULL, &tX, NULL);
}

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cValType, cValSizeX, cValSizeY, cValPosition;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_TYPE), &cValType);
		a_pConfig->ItemValueGet(CComBSTR(cValType.operator LONG() == CFGVAL_CST_DELTA ? CFGID_CS_DELTA_X : CFGID_CS_SIZE_X), &cValSizeX);
		a_pConfig->ItemValueGet(CComBSTR(cValType.operator LONG() == CFGVAL_CST_DELTA ? CFGID_CS_DELTA_Y : CFGID_CS_SIZE_Y), &cValSizeY);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_POSITION), &cValPosition);

		CComPtr<IDocumentAnimation> pA;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
		{
			// layered image or raster image
			return ResizeFrame(a_pDocument, cValType, cValSizeX, cValSizeY, cValPosition[0], cValPosition[1]);
		}

		// animation
		TImageSize tAniSize = GetAnimationSize(pA);
		if (tAniSize.nX*tAniSize.nY == 0)
			return E_FAIL;
		CComPtr<IEnumUnknowns> pFrames;
		pA->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		pFrames->Size(&nFrames);
		CWriteLock<IDocument> cLock(a_pDocument);
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			pA->FrameGetDoc(pFrame, &pFrameDoc);
			HRESULT hRes = ResizeFrame(pFrameDoc, cValType, cValSizeX, cValSizeY, cValPosition[0], cValPosition[1], &tAniSize);
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

#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Set canvas size[0405]Nastavit velikost plátna");
			return S_OK;
		}

		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pMsg = pStr;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_TYPE), &cVal);
		if (cVal.operator LONG() == CFGVAL_CST_DELTA)
		{
			CConfigValue cX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_DELTA_X), &cX);
			CConfigValue cY;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_DELTA_Y), &cY);
			pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Adjust size by %i, %i[0405]Změnit velikost o %i, %i"), cX.operator LONG(), cY.operator LONG());
		}
		else
		{
			CConfigValue cX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_SIZE_X), &cX);
			CConfigValue cY;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CS_SIZE_Y), &cY);
			pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Set size to %ix%i[0405]Nastavit velikost na %ix%i"), cX.operator LONG(), cY.operator LONG());
		}

		*a_ppName = pMsg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageCanvasSize::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	try
	{
		if (a_pConfig == NULL || a_pContentTransform == NULL)
			return E_RW_INVALIDPARAM;
		float fScaleX = 1.0f;
		float fScaleY = 1.0f;
		Matrix3x3fDecompose(*a_pContentTransform, &fScaleX, &fScaleY, NULL, NULL);
		float const f = sqrtf(fScaleX*fScaleY);
		if (f > 0.9999f && f < 1.0001f)
			return S_FALSE;
		CComBSTR bstrDELTA_X(CFGID_CS_DELTA_X);
		CComBSTR bstrDELTA_Y(CFGID_CS_DELTA_Y);
		CComBSTR bstrSIZE_X(CFGID_CS_SIZE_X);
		CComBSTR bstrSIZE_Y(CFGID_CS_SIZE_Y);
		BSTR aIDs[] = {bstrDELTA_X, bstrDELTA_Y, bstrSIZE_X, bstrSIZE_Y};
		TConfigValue cVal[sizeof(aIDs)/sizeof(aIDs[0])];
		a_pConfig->ItemValueGet(bstrDELTA_X, &cVal[0]);
		a_pConfig->ItemValueGet(bstrDELTA_Y, &cVal[1]);
		a_pConfig->ItemValueGet(bstrSIZE_X, &cVal[2]);
		a_pConfig->ItemValueGet(bstrSIZE_Y, &cVal[3]);
		cVal[0].iVal = LONG(cVal[0].iVal*fScaleX+0.5f);
		cVal[1].iVal = LONG(cVal[1].iVal*fScaleY+0.5f);
		cVal[2].iVal = LONG(cVal[2].iVal*fScaleX+0.5f);
		cVal[3].iVal = LONG(cVal[3].iVal*fScaleY+0.5f);
		return a_pConfig->ItemValuesSet(sizeof(aIDs)/sizeof(aIDs[0]), aIDs, cVal);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
