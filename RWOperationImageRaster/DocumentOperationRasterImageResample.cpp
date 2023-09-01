// DocumentOperationRasterImageResample.cpp : Implementation of CDocumentOperationRasterImageResample

#include "stdafx.h"
#include "DocumentOperationRasterImageResample.h"

#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>
#include <SharedStringTable.h>
#include "Resampling.h"

const OLECHAR CFGID_SIZETYPE[] = L"Mode";
const LONG CFGVAL_SIZETYPE_REL = 0;
const LONG CFGVAL_SIZETYPE_X = 2;
const LONG CFGVAL_SIZETYPE_Y = 3;
const LONG CFGVAL_SIZETYPE_LONGER = 4;
const LONG CFGVAL_SIZETYPE_SHORTER = 5;
const LONG CFGVAL_SIZETYPE_ABS = 10;
const LONG CFGVAL_SIZETYPE_FRAME = 11;
const LONG CFGVAL_SIZETYPE_CROP = 13;
const LONG CFGVAL_SIZETYPE_EXTEND = 14;
const OLECHAR CFGID_SIZEX[] = L"SizeX";
const OLECHAR CFGID_SIZEY[] = L"SizeY";
const OLECHAR CFGID_SIZEREL[] = L"SizeRel";
const OLECHAR CFGID_SIZE[] = L"Size";
const OLECHAR CFGID_CANVAS[] = L"Canvas";
const OLECHAR CFGID_CANVASCOLOR[] = L"CanvasColor";
const OLECHAR CFGID_ALIGNMODE[] = L"AlignMode";
const OLECHAR CFGID_ALIGNPOS[] = L"AlignPos";
const OLECHAR CFGID_RESAMPLEMETHOD[] = L"Method";
const LONG CFGVAL_RESAMPLE_NEAREST = 0;
const LONG CFGVAL_RESAMPLE_LINEAR = 1;
const LONG CFGVAL_RESAMPLE_CUBIC = 2;
const OLECHAR CFGID_RESOLUTIONMODE[] = L"ResolutionMode";
const LONG CFGVAL_RESOLUTION_AUTO = 0;
const LONG CFGVAL_RESOLUTION_KEEP = 1;
const LONG CFGVAL_RESOLUTION_SET = 2;
const OLECHAR CFGID_RESOLUTION[] = L"Resolution";

TImageSize GetAnimationSize(IDocumentAnimation* a_pAni, TImageResolution* a_pRes = NULL); // implemented in canvas size op
void GetResampledSize(IConfig* a_pConfig, TImageSize const& tSize, LONG& nSizeX, LONG& nSizeY, LONG& nFrameX, LONG& nFrameY);
TImageResolution GetResolution(TImageResolution a_tOrigRes, IConfig* a_pConfig, TImageSize a_tOrigSize, TImageSize a_tNewSize);

#include "ConfigGUIResample.h"


// CDocumentOperationRasterImageResample

STDMETHODIMP CDocumentOperationRasterImageResample::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGERESAMPLE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageResample::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Rescale image[0405]Přeškálovat obrázek");
			return S_OK;
		}
		CConfigValue cSizeType;
		CComBSTR cCFGID_SIZETYPE(CFGID_SIZETYPE);
		a_pConfig->ItemValueGet(cCFGID_SIZETYPE, &cSizeType);
		CComPtr<IConfigItemOptions> pUIInfo;
		a_pConfig->ItemGetUIInfo(cCFGID_SIZETYPE, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pUIInfo));
		CComPtr<ILocalizedString> pType;
		pUIInfo->ValueGetName(cSizeType, &pType);
		CConfigValue cVal1;
		CConfigValue cVal2;
		OLECHAR szSize[64] = L"";
		bool px = true;
		switch (cSizeType.operator LONG())
		{
		case CFGVAL_SIZETYPE_REL:
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEREL), &cVal1);
			_swprintf(szSize, L"%g%%", cVal1.operator float());
			px = false;
			break;
		case CFGVAL_SIZETYPE_X:
		case CFGVAL_SIZETYPE_Y:
		case CFGVAL_SIZETYPE_LONGER:
		case CFGVAL_SIZETYPE_SHORTER:
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal1);
			_swprintf(szSize, L"%i", cVal1.operator LONG());
			break;
		case CFGVAL_SIZETYPE_ABS:
		case CFGVAL_SIZETYPE_FRAME:
		case CFGVAL_SIZETYPE_CROP:
		case CFGVAL_SIZETYPE_EXTEND:
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal1);
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal2);
			_swprintf(szSize, L"%ix%i", cVal1.operator LONG(), cVal2.operator LONG());
			break;
		}
		CComPtr<ILocalizedString> pSize;
		if (px)
		{
			CComPtr<ILocalizedString> pConcatPx;
			pConcatPx.Attach(new CSimpleLocalizedString(SysAllocString(L"%s%s")));
			CComPtr<ILocalizedString> pNumber;
			pNumber.Attach(new CSimpleLocalizedString(SysAllocString(szSize)));
			CComObject<CPrintfLocalizedString>* pS = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pS);
			pSize = pS;
			pS->Init(pConcatPx, pNumber, CMultiLanguageString::GetAuto(L"[0409]px[0405]px"));
		}
		else
		{
			pSize.Attach(new CSimpleLocalizedString(SysAllocString(szSize)));
		}
		CComPtr<ILocalizedString> pConcat;
		pConcat.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		pStr->Init(pConcat, pType, pSize);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageResample::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR bstrCFGID_SIZETYPE(CFGID_SIZETYPE);
		pCfgInit->ItemIns1ofN(bstrCFGID_SIZETYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_SIZETYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZETYPE_DESC), CConfigValue(CFGVAL_SIZETYPE_REL), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_REL), CMultiLanguageString::GetAuto(L"[0409]Relative size[0405]Relativní velikost"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_X), CMultiLanguageString::GetAuto(L"[0409]Width[0405]Šířka"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_Y), CMultiLanguageString::GetAuto(L"[0409]Height[0405]Výška"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_LONGER), CMultiLanguageString::GetAuto(L"[0409]Longer side[0405]Delší strana"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_SHORTER), CMultiLanguageString::GetAuto(L"[0409]Shorter side[0405]Kratší strana"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_ABS), CMultiLanguageString::GetAuto(L"[0409]Both sides[0405]Obě strany"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_FRAME), CMultiLanguageString::GetAuto(L"[0409]Maximum frame[0405]Maximální rámeček"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_CROP), CMultiLanguageString::GetAuto(L"[0409]Crop canvas[0405]Oříznout plátno"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_SIZETYPE, CConfigValue(CFGVAL_SIZETYPE_EXTEND), CMultiLanguageString::GetAuto(L"[0409]Extend canvas[0405]Rozšířit plátno"), 0, NULL);

		TConfigOptionCondition aCond[2];
		aCond[0].bstrID = bstrCFGID_SIZETYPE;
		aCond[0].eConditionType = ECOCEqual;
		aCond[0].tValue.eTypeID = ECVTInteger;
		aCond[0].tValue.iVal = CFGVAL_SIZETYPE_REL;

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SIZEREL), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEREL_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEREL_DESC), CConfigValue(50.0f), NULL, 1, aCond);

		aCond[0].eConditionType = ECOCGreaterEqual;
		aCond[0].tValue.iVal = CFGVAL_SIZETYPE_X;
		aCond[1] = aCond[0];
		aCond[1].eConditionType = ECOCLessEqual;
		aCond[1].tValue.iVal = CFGVAL_SIZETYPE_SHORTER;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SIZE), CMultiLanguageString::GetAuto(L"[0409]Size[0405]Velikost"), CMultiLanguageString::GetAuto(L"[0409]Size of the selected image dimension in pixels.[0405]Velikost vybraného rozměru obrázku v pixelech."), CConfigValue(256L), NULL, 2, aCond);

		aCond[0].tValue.iVal = CFGVAL_SIZETYPE_ABS;
		aCond[1].tValue.iVal = CFGVAL_SIZETYPE_EXTEND;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEXABS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEXABS_DESC), CConfigValue(256L), NULL, 2, aCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEYABS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SIZEYABS_DESC), CConfigValue(256L), NULL, 2, aCond);

		CComBSTR bstrCFGID_RESAMPLEMETHOD(CFGID_RESAMPLEMETHOD);
		pCfgInit->ItemIns1ofN(bstrCFGID_RESAMPLEMETHOD, _SharedStringTable.GetStringAuto(IDS_CFGID_RESAMPLEMETHOD_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_RESAMPLEMETHOD_DESC), CConfigValue(CFGVAL_RESAMPLE_LINEAR), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESAMPLEMETHOD, CConfigValue(CFGVAL_RESAMPLE_NEAREST), _SharedStringTable.GetStringAuto(IDS_CFGVAL_RESAMPLE_NEAREST), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESAMPLEMETHOD, CConfigValue(CFGVAL_RESAMPLE_LINEAR), _SharedStringTable.GetStringAuto(IDS_CFGVAL_RESAMPLE_LINEAR), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESAMPLEMETHOD, CConfigValue(CFGVAL_RESAMPLE_CUBIC), _SharedStringTable.GetStringAuto(IDS_CFGVAL_RESAMPLE_CUBIC), 0, NULL);

		CComBSTR bstrCFGID_RESOLUTIONMODE(CFGID_RESOLUTIONMODE);
		pCfgInit->ItemIns1ofN(bstrCFGID_RESOLUTIONMODE, CMultiLanguageString::GetAuto(L"[0409]Resolution[0405]Rozlišení"), CMultiLanguageString::GetAuto(L"[0409]Resolution relates number of pixels to physical dimensions of an image. Select how resolution is adjusted during pixel resampling.[0405]Rozlišení dává do vztahu počet pixelů a fyzickou velikost obrázku. Vyberte, jak nastavit rozlišení po převzorkování pixelů."), CConfigValue(CFGVAL_RESOLUTION_KEEP), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESOLUTIONMODE, CConfigValue(CFGVAL_RESOLUTION_AUTO), CMultiLanguageString::GetAuto(L"[0409]Recompute[0405]Přepočítat"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESOLUTIONMODE, CConfigValue(CFGVAL_RESOLUTION_KEEP), CMultiLanguageString::GetAuto(L"[0409]Keep original[0405]Zachovat původní"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_RESOLUTIONMODE, CConfigValue(CFGVAL_RESOLUTION_SET), CMultiLanguageString::GetAuto(L"[0409]Set to[0405]Nastavit na"), 0, NULL);

		aCond[0].bstrID = bstrCFGID_RESOLUTIONMODE;
		aCond[0].eConditionType = ECOCEqual;
		aCond[0].tValue.eTypeID = ECVTInteger;
		aCond[0].tValue.iVal = CFGVAL_RESOLUTION_SET;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_RESOLUTION), CMultiLanguageString::GetAuto(L"[0409]Resolution[0405]Rozlišení"), CMultiLanguageString::GetAuto(L"[0409]Resolution in pixels per inch (DPI) units.[0405]Rozlišení v jednotkách pixelů na palec (DPI)."), CConfigValue(100.0f), NULL, 1, aCond);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageResample, CConfigGUIResample>::FinalizeConfig(pCfgInit);
		//pCfgInit->Finalize(NULL);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageResample::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFtsImg[] = {__uuidof(IDocumentEditableImage)};
		static IID const aFtsAni[] = {__uuidof(IDocumentAnimation)};
		return (SupportsAllFeatures(a_pDocument, itemsof(aFtsImg), aFtsImg) ||
			SupportsAllFeatures(a_pDocument, itemsof(aFtsAni), aFtsAni)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static void GetResampledSize(IConfig* a_pConfig, TImageSize const& tSize, LONG& nSizeX, LONG& nSizeY, LONG& nFrameX, LONG& nFrameY)
{
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZETYPE), &cVal);
	switch (cVal.operator LONG())
	{
	case CFGVAL_SIZETYPE_REL:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEREL), &cVal);
		nSizeX = nFrameX = cVal.operator float()*tSize.nX*0.01f + 0.5f;
		nSizeY = nFrameY = cVal.operator float()*tSize.nY*0.01f + 0.5f;
		break;
	case CFGVAL_SIZETYPE_X:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
		nSizeX = nFrameX = cVal;
		nSizeY = nFrameY = nSizeX*tSize.nY/float(tSize.nX) + 0.5f;
		break;
	case CFGVAL_SIZETYPE_Y:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
		nSizeY = nFrameY = cVal;
		nSizeX = nFrameX = nSizeY*tSize.nX/float(tSize.nY) + 0.5f;
		break;
	case CFGVAL_SIZETYPE_LONGER:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
		if (tSize.nX >= tSize.nY)
		{
			nSizeX = nFrameX = cVal;
			nSizeY = nFrameY = nSizeX*tSize.nY/float(tSize.nX) + 0.5f;
		}
		else
		{
			nSizeY = nFrameY = cVal;
			nSizeX = nFrameX = nSizeY*tSize.nX/float(tSize.nY) + 0.5f;
		}
		break;
	case CFGVAL_SIZETYPE_SHORTER:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZE), &cVal);
		if (tSize.nX <= tSize.nY)
		{
			nSizeX = nFrameX = cVal;
			nSizeY = nFrameY = nSizeX*tSize.nY/float(tSize.nX) + 0.5f;
		}
		else
		{
			nSizeY = nFrameY = cVal;
			nSizeX = nFrameX = nSizeY*tSize.nX/float(tSize.nY) + 0.5f;
		}
		break;
	case CFGVAL_SIZETYPE_ABS:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
		nSizeX = nFrameX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
		nSizeY = nFrameY = cVal;
		break;
	case CFGVAL_SIZETYPE_FRAME:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
		nFrameX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
		nFrameY = cVal;
		if (nFrameX*tSize.nY >= nFrameY*tSize.nX)
		{
			nSizeX = nFrameX = nFrameY*tSize.nX/float(tSize.nY) + 0.5f;
			nSizeY = nFrameY;
		}
		else
		{
			nSizeY = nFrameY = nFrameX*tSize.nY/float(tSize.nX) + 0.5f;
			nSizeX = nFrameX;
		}
		break;
	case CFGVAL_SIZETYPE_CROP:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
		nFrameX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
		nFrameY = cVal;
		if (nFrameX*tSize.nY <= nFrameY*tSize.nX)
		{
			nSizeX = nFrameY*tSize.nX/float(tSize.nY) + 0.5f;
			nSizeY = nFrameY;
		}
		else
		{
			nSizeY = nFrameX*tSize.nY/float(tSize.nX) + 0.5f;
			nSizeX = nFrameX;
		}
		break;
	case CFGVAL_SIZETYPE_EXTEND:
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
		nFrameX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
		nFrameY = cVal;
		if (nFrameX*tSize.nY >= nFrameY*tSize.nX)
		{
			nSizeX = nFrameY*tSize.nX/float(tSize.nY) + 0.5f;
			nSizeY = nFrameY;
		}
		else
		{
			nSizeY = nFrameX*tSize.nY/float(tSize.nX) + 0.5f;
			nSizeX = nFrameX;
		}
		break;
	default:
		return;
	}
	if (nSizeX < 1) nSizeX = 1;
	if (nSizeY < 1) nSizeY = 1;
	if (nFrameX < 1) nFrameX = 1;
	if (nFrameY < 1) nFrameY = 1;
}

static void Resample(LONG a_nMethod, float a_fGamma, LONG a_nDstSizeX, LONG a_nDstSizeY, LONG a_nSrcSizeX, LONG a_nSrcSizeY, TRasterImagePixel* a_pDst, TRasterImagePixel const* a_pSrc)
{
	if (a_fGamma == 1.0f || a_nMethod == CFGVAL_RESAMPLE_NEAREST)
	{
		CResampling cRsmp(a_nDstSizeX, a_nDstSizeY, a_nSrcSizeX, a_nSrcSizeY, a_pDst, a_pSrc);
		switch (a_nMethod)
		{
		case CFGVAL_RESAMPLE_NEAREST:
			cRsmp.Nearest();
			break;
		case CFGVAL_RESAMPLE_LINEAR:
			cRsmp.Linear();
			break;
		case CFGVAL_RESAMPLE_CUBIC:
			cRsmp.Cubic();
			break;
		}
	}
	else
	{
		CGammaResampling cRsmp(a_fGamma, a_nDstSizeX, a_nDstSizeY, a_nSrcSizeX, a_nSrcSizeY, a_pDst, a_pSrc);
		switch (a_nMethod)
		{
		case CFGVAL_RESAMPLE_LINEAR:
			cRsmp.Linear();
			break;
		case CFGVAL_RESAMPLE_CUBIC:
			cRsmp.Cubic();
			break;
		}
	}
}

#include <RWViewImageRaster.h> // TODO: move color-related stuff to RWImaging


class ATL_NO_VTABLE CResamplingHelperNearest : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageTransformer
{
public:
	CResamplingHelperNearest()
	{
	}

BEGIN_COM_MAP(CResamplingHelperNearest)
	COM_INTERFACE_ENTRY(IRasterImageTransformer)
END_COM_MAP()

	// IRasterImageTransformer methods
public:
	STDMETHOD(ProcessTile)(EImageChannelID a_eChannelID, float a_fGamma, TPixelChannel const* a_pDefault, TMatrix3x3f const* a_pContentTransform,
						   ULONG a_nSrcPixels, TPixelChannel const* a_pSrcData, TImagePoint const* a_pSrcOrigin, TImageSize const* a_pSrcSize, TImageStride const* a_pSrcStride,
						   ULONG a_nDstPixels, TPixelChannel* a_pDstData, TImagePoint const* a_pDstOrigin, TImageSize const* a_pDstSize, TImageStride const* a_pDstStride)
	{
		try
		{
			float fScaleX = 1.0f;
			float fScaleY = 1.0f;
			Matrix3x3fDecompose(*a_pContentTransform, &fScaleX, &fScaleY, NULL, NULL);
			LONGLONG const nFactorX = 0x10000000/fScaleX+0.5;
			LONGLONG const nFactorY = 0x10000000/fScaleY+0.5;
			LONG const nDstY1 = ((LONGLONG(a_pSrcOrigin->nY)<<28)+nFactorY-1)/nFactorY;
			LONG const nDstY2 = ((LONGLONG(a_pSrcOrigin->nY+a_pSrcSize->nY)<<28)+nFactorY-1)/nFactorY;
			LONG const nDstX1 = ((LONGLONG(a_pSrcOrigin->nX)<<28)+nFactorX-1)/nFactorX;
			LONG const nDstX2 = ((LONGLONG(a_pSrcOrigin->nX+a_pSrcSize->nX)<<28)+nFactorX-1)/nFactorX;
			LONG y = a_pDstOrigin->nY;
			LONG const nYEnd = a_pDstOrigin->nY+a_pDstSize->nY;
			LONG const nXEnd = a_pDstOrigin->nX+a_pDstSize->nX;
			LONG const nDstDiff = a_pDstStride->nY-a_pDstSize->nX*a_pDstStride->nX;
			while (y < nDstY1)
			{
				for (TPixelChannel* const pEnd = a_pDstData+a_pDstSize->nX*a_pDstStride->nX; a_pDstData < pEnd; a_pDstData += a_pDstStride->nX)
					*a_pDstData = *a_pDefault;
				a_pDstData += nDstDiff;
				++y;
			}
			LONG const nYMid = nDstY2 < nYEnd ? nDstY2 : nYEnd;
			LONG const nXMid = nDstX2 < nXEnd ? nDstX2 : nXEnd;
			while (y < nYMid)
			{
				LONG x = a_pDstOrigin->nX;
				while (x < nDstX1)
				{
					*a_pDstData = *a_pDefault;
					++x;
					a_pDstData += a_pDstStride->nX;
				}
				TPixelChannel const* pSrc = a_pSrcData + (((y*nFactorY+(nFactorY>>1))>>28)-a_pSrcOrigin->nY)*a_pSrcStride->nY;
				while (x < nXMid)
				{
					*a_pDstData = pSrc[LONG(((x*nFactorX+(nFactorX>>1))>>28)-a_pSrcOrigin->nX)*a_pSrcStride->nX];
					++x;
					a_pDstData += a_pDstStride->nX;
				}
				while (x < nXEnd)
				{
					*a_pDstData = *a_pDefault;
					++x;
					a_pDstData += a_pDstStride->nX;
				}
				a_pDstData += nDstDiff;
				++y;
			}
			while (y < nYEnd)
			{
				for (TPixelChannel* const pEnd = a_pDstData+a_pDstSize->nX*a_pDstStride->nX; a_pDstData < pEnd; a_pDstData += a_pDstStride->nX)
					*a_pDstData = *a_pDefault;
				a_pDstData += nDstDiff;
				++y;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

#include <RWImagingDocumentUtils.h>

class ATL_NO_VTABLE CResamplingHelperCoverage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageTransformer
{
public:
	CResamplingHelperCoverage() : m_bCubic(false)
	{
	}

	void Init(TImageSize const& a_tSize, TImageSize const& a_tSizeNew, bool a_bCubic)
	{
		m_tSize = a_tSize;
		m_tSizeNew = a_tSizeNew;
		m_bCubic = a_bCubic;
	}

BEGIN_COM_MAP(CResamplingHelperCoverage)
	COM_INTERFACE_ENTRY(IRasterImageTransformer)
END_COM_MAP()

	// IRasterImageTransformer methods
public:
	STDMETHOD(ProcessTile)(EImageChannelID a_eChannelID, float a_fGamma, TPixelChannel const* a_pDefault, TMatrix3x3f const* a_pContentTransform,
						   ULONG a_nSrcPixels, TPixelChannel const* a_pSrcData, TImagePoint const* a_pSrcOrigin, TImageSize const* a_pSrcSize, TImageStride const* a_pSrcStride,
						   ULONG a_nDstPixels, TPixelChannel* a_pDstData, TImagePoint const* a_pDstOrigin, TImageSize const* a_pDstSize, TImageStride const* a_pDstStride)
	{
		try
		{
			if (a_eChannelID != EICIRGBA)
				return E_NOTIMPL;
			if (a_pSrcOrigin->nX == 0 && a_pSrcOrigin->nY == 0 && a_pSrcSize->nX == m_tSize.nX && a_pSrcSize->nY == m_tSize.nY && a_pSrcStride->nX == 1 &&
				a_pDstOrigin->nX == 0 && a_pDstOrigin->nY == 0 && a_pDstSize->nX == LONG(m_tSize.nX*a_pContentTransform->_11+0.5f) && a_pDstSize->nY == LONG(m_tSize.nY*a_pContentTransform->_22+0.5f) && a_pDstStride->nX == 1)
			{
				if (a_fGamma != 1.0f && a_fGamma > 0.1f && a_fGamma < 10.0f)
				{
					CGammaResampling cRsmp(a_fGamma, a_pDstSize->nX, a_pDstSize->nY, a_pSrcSize->nX, a_pSrcSize->nY, reinterpret_cast<TRasterImagePixel*>(a_pDstData), a_pDstStride->nY<<2, reinterpret_cast<TRasterImagePixel const*>(a_pSrcData), a_pSrcStride->nY<<2);
					if (m_bCubic)
						cRsmp.Cubic();
					else
						cRsmp.Linear();
				}
				else
				{
					CResampling cRsmp(a_pDstSize->nX, a_pDstSize->nY, a_pSrcSize->nX, a_pSrcSize->nY, reinterpret_cast<TRasterImagePixel*>(a_pDstData), a_pDstStride->nY<<2, reinterpret_cast<TRasterImagePixel const*>(a_pSrcData), a_pSrcStride->nY<<2);
					if (m_bCubic)
						cRsmp.Cubic();
					else
						cRsmp.Linear();
				}
				return S_OK;
			}

			CAutoVectorPtr<TPixelChannel> cSrc(new TPixelChannel[m_tSize.nX*m_tSize.nY]);
			RGBAGetTileImpl(m_tSize, *a_pSrcOrigin, *a_pSrcSize, a_pSrcData, a_pSrcStride->nY, *a_pDefault, NULL, &m_tSize, NULL, m_tSize.nX*m_tSize.nY, cSrc);
			CAutoVectorPtr<TPixelChannel> cDst(new TPixelChannel[m_tSizeNew.nX*m_tSizeNew.nY]);

			if (a_fGamma != 1.0f && a_fGamma > 0.1f && a_fGamma < 10.0f)
			{
				CGammaResampling cRsmp(a_fGamma, m_tSizeNew.nX, m_tSizeNew.nY, m_tSize.nX, m_tSize.nY, reinterpret_cast<TRasterImagePixel*>(cDst.m_p), reinterpret_cast<TRasterImagePixel const*>(cSrc.m_p));
				if (m_bCubic)
					cRsmp.Cubic();
				else
					cRsmp.Linear();
			}
			else
			{
				CResampling cRsmp(m_tSizeNew.nX, m_tSizeNew.nY, m_tSize.nX, m_tSize.nY, reinterpret_cast<TRasterImagePixel*>(cDst.m_p), reinterpret_cast<TRasterImagePixel const*>(cSrc.m_p));
				if (m_bCubic)
					cRsmp.Cubic();
				else
					cRsmp.Linear();
			}

			RGBAGetTileImpl(m_tSize, CImagePoint(a_pContentTransform->_31+0.5f, a_pContentTransform->_32+0.5f), m_tSizeNew, cDst, m_tSizeNew.nX, *a_pDefault, a_pDstOrigin, a_pDstSize, a_pDstStride, a_pDstSize->nX*a_pDstSize->nY, a_pDstData);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	TImageSize m_tSize;
	TImageSize m_tSizeNew;
	bool m_bCubic;
};

TImageResolution GetResolution(TImageResolution a_tOrigRes, IConfig* a_pConfig, TImageSize a_tOrigSize, TImageSize a_tNewSize)
{
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_RESOLUTIONMODE), &cVal);

	if (cVal.operator LONG() == CFGVAL_RESOLUTION_AUTO)
	{
		a_tOrigRes.nNumeratorY = 
		a_tOrigRes.nNumeratorX = double(a_tOrigRes.nNumeratorX)*a_tNewSize.nX/a_tOrigSize.nX;
		//a_tOrigRes.nNumeratorY = double(a_tOrigRes.nNumeratorY)*a_tNewSize.nY/a_tOrigSize.nY;
		return a_tOrigRes;
	}

	if (cVal.operator LONG() == CFGVAL_RESOLUTION_KEEP)
		return a_tOrigRes;

	// CFGVAL_RESOLUTION_SET
	a_pConfig->ItemValueGet(CComBSTR(CFGID_RESOLUTION), &cVal);
	TImageResolution const t = {cVal.operator float()+0.5f, 254, cVal.operator float()+0.5f, 254};
	return t;
}

HRESULT ResampleFrame(IDocument* a_pDocument, float const fGamma, IConfig* a_pConfig, TImageSize const* a_pNewSize = NULL)
{
	CComPtr<IDocumentEditableImage> pEI;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
	TImageResolution tRes = {0, 0, 0, 0};
	TImageSize tSize;
	pEI->CanvasGet(&tSize, &tRes, NULL, NULL, NULL);

	LONG nSizeX = 1;
	LONG nSizeY = 1;
	LONG nFrameX = 1;
	LONG nFrameY = 1;
	GetResampledSize(a_pConfig, tSize, nSizeX, nSizeY, nFrameX, nFrameY);
	TImageResolution tNewRes = GetResolution(tRes, a_pConfig, tSize, CImageSize(nSizeX, nSizeY));
	bool const bChangeRes = tRes.nNumeratorX != tNewRes.nNumeratorX || tRes.nNumeratorY != tNewRes.nNumeratorY || tRes.nDenominatorX != tNewRes.nDenominatorX || tRes.nDenominatorY != tNewRes.nDenominatorY;
	if (nSizeX == tSize.nX && nSizeY == tSize.nY && nFrameX == tSize.nX && nFrameY == tSize.nY)
		return bChangeRes ? pEI->CanvasSet(NULL, &tNewRes, NULL, NULL) : S_FALSE;
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_RESAMPLEMETHOD), &cVal);
	LONG nMethod = cVal;

	TImageSize tSizeNew = {nSizeX, nSizeY};
	TImageSize tFrameNew = {nFrameX, nFrameY};
	TMatrix3x3f tTransform =
	{
		double(tSizeNew.nX)/tSize.nX, 0.0f, 0.0f,
		0.0f, double(tSizeNew.nY)/tSize.nY, 0.0f,
		(double(tFrameNew.nX)-double(tSizeNew.nX))*0.5, (double(tFrameNew.nY)-double(tSizeNew.nY))*0.5, 1.0f
	};
	CComPtr<IRasterImageTransformer> pHelper;
	if (nMethod == CFGVAL_RESAMPLE_NEAREST)
	{
		CComObject<CResamplingHelperNearest>* p = NULL;
		CComObject<CResamplingHelperNearest>::CreateInstance(&p);
		pHelper = p;
	}
	else
	{
		CComObject<CResamplingHelperCoverage>* p = NULL;
		CComObject<CResamplingHelperCoverage>::CreateInstance(&p);
		pHelper = p;
		p->Init(tSize, tSizeNew, nMethod == CFGVAL_RESAMPLE_CUBIC);
	}
	return pEI->CanvasSet(&tFrameNew, bChangeRes ? &tNewRes : NULL, &tTransform, pHelper);
}

STDMETHODIMP CDocumentOperationRasterImageResample::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		float fGamma = 1.0f;
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		if (pGCM)
		{
			CComPtr<IConfig> pConfig;
			pGCM->Config(__uuidof(ColorWindow), &pConfig);
			if (pConfig)
			{
				CConfigValue cVal;
				pConfig->ItemValueGet(CComBSTR(L"Gamma"), &cVal);
				if (cVal.TypeGet() == ECVTFloat)
				{
					fGamma = cVal;
					if (fGamma < 0.1f) fGamma = 1.0f; else if (fGamma > 10.0f) fGamma = 1.0f;
				}
			}
		}

		CComPtr<IDocumentAnimation> pA;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
		{
			// layered image or raster image
			return ResampleFrame(a_pDocument, fGamma, a_pConfig);
		}

		// animation
		TImageResolution tAniRes = {0, 0, 0, 0};
		TImageSize tAniSize = GetAnimationSize(pA, &tAniRes);
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
			HRESULT hRes = ResampleFrame(pFrameDoc, fGamma, a_pConfig, &tAniSize);
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

