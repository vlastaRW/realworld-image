// DocumentOperationRasterImageRotate.cpp : Implementation of CDocumentOperationRasterImageRotate

#include "stdafx.h"
#include "DocumentOperationRasterImageRotate.h"
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>

const OLECHAR CFGID_ANGLE[] = L"Angle";
const OLECHAR CFGID_EXTEND[] = L"Extend";
const LONG CFGVAL_EXTEND_NEVER = 0;
const LONG CFGVAL_EXTEND_AUTO = 1;
const LONG CFGVAL_EXTEND_ALWAYS = 2;

#include "ConfigGUIRotate.h"


// CDocumentOperationRasterImageRotate

STDMETHODIMP CDocumentOperationRasterImageRotate::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Rotate[0405]Rastrový obrázek - otočit");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRotate::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Rotate[0405]Otočit";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cAngle;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_ANGLE), &cAngle);
		if (cAngle.operator float() == 90.0f)
		{
			pszName = L"[0409]Rotate right[0405]Otočit doprava";
		}
		else if (cAngle.operator float() == 270.0f)
		{
			pszName = L"[0409]Rotate left[0405]Otočit doleva";
		}
		else
		{
			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			CComPtr<ILocalizedString> pTmp = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CMultiLanguageString(L"[0409]Rotate by %i°[0405]Otočit o %i°"));
			pStr->Init(pTempl, int(cAngle.operator float()+0.5f));
			*a_ppName = pTmp.Detach();
			return S_OK;
		}

		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRotate::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_ANGLE), CMultiLanguageString::GetAuto(L"[0409]Angle[0405]Úhel"), CMultiLanguageString::GetAuto(L"[0409]Value in degrees specifying how much is the image rotated clockwise.[0405]Hodnota ve stupních určující jak moc bude obrázek otočen ve směru hodinových ručiček."), CConfigValue(45.0f), NULL, 0, NULL);
		CComBSTR bstrEXTEND(CFGID_EXTEND);
		pCfgInit->ItemIns1ofN(bstrEXTEND, CMultiLanguageString::GetAuto(L"[0409]Adjust canvas[0405]Přizpůsobit plátno"), CMultiLanguageString::GetAuto(L"[0409]Make the canvas large enough for the rotated image.[0405]Změnit velikost plátna, aby se na něj otočený obrázek vešel."), CConfigValue(CFGVAL_EXTEND_AUTO), NULL);
		pCfgInit->ItemOptionAdd(bstrEXTEND, CConfigValue(CFGVAL_EXTEND_NEVER), CMultiLanguageString::GetAuto(L"[0409]Never[0405]Nikdy"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrEXTEND, CConfigValue(CFGVAL_EXTEND_AUTO), CMultiLanguageString::GetAuto(L"[0409]If needed[0405]Je-li potřeba"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrEXTEND, CConfigValue(CFGVAL_EXTEND_ALWAYS), CMultiLanguageString::GetAuto(L"[0409]Always[0405]Vždycky"), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageRotate, CConfigGUIRotate>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRotate::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentEditableImage)};
		return SupportsAllFeatures(a_pDocument, 1, aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "AGGBuffer.h"

#include <agg_trans_affine.h>

class ATL_NO_VTABLE CTransformHelperRotate : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageTransformer
{
public:
	CTransformHelperRotate()
	{
	}

BEGIN_COM_MAP(CTransformHelperRotate)
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
			TImagePoint const tDstEnd = {a_pDstOrigin->nX+a_pDstSize->nX, a_pDstOrigin->nY+a_pDstSize->nY};
			TImagePoint const tSrcEnd = {a_pSrcOrigin->nX+a_pSrcSize->nX, a_pSrcOrigin->nY+a_pSrcSize->nY};

			// optimized processing for 90, 180, and 270 degrees
			if (a_pContentTransform->_11 == 0.0f && a_pContentTransform->_12 == 1.0f &&
				a_pContentTransform->_21 == -1.0f && a_pContentTransform->_22 == 0.0f &&
				a_pContentTransform->_31 == LONG(a_pContentTransform->_31) && a_pContentTransform->_32 == LONG(a_pContentTransform->_32))
			{
				LONG const nDY = a_pContentTransform->_32;
				LONG const nDX = a_pContentTransform->_31-1;
				for (LONG nY = a_pDstOrigin->nY; nY < tDstEnd.nY; ++nY)
				{
					LONG nSX = nY-nDY;
					if (nSX < a_pSrcOrigin->nX || nSX >= tSrcEnd.nX)
					{
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*a_pDstSize->nX; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					else
					{
						TPixelChannel const* pSRow = a_pSrcData+(nSX-a_pSrcOrigin->nX)*a_pSrcStride->nX;
						for (LONG nX = a_pDstOrigin->nX; nX < tDstEnd.nX; ++nX)
						{
							LONG nSY = nDX-nX;
							if (nSY < a_pSrcOrigin->nY || nSY >= tSrcEnd.nY)
								*a_pDstData = *a_pDefault;
							else
								*a_pDstData = pSRow[(nSY-a_pSrcOrigin->nY)*a_pSrcStride->nY];
							a_pDstData+=a_pDstStride->nX;
						}
					}
					a_pDstData += a_pDstStride->nY-a_pDstStride->nX*a_pDstSize->nX;
				}
			}
			else if (a_pContentTransform->_11 == 0.0f && a_pContentTransform->_12 == -1.0f &&
				a_pContentTransform->_21 == 1.0f && a_pContentTransform->_22 == 0.0f &&
				a_pContentTransform->_31 == LONG(a_pContentTransform->_31) && a_pContentTransform->_32 == LONG(a_pContentTransform->_32))
			{
				LONG const nDY = a_pContentTransform->_32-1;
				LONG const nDX = a_pContentTransform->_31;
				for (LONG nY = a_pDstOrigin->nY; nY < tDstEnd.nY; ++nY)
				{
					LONG nSX = nDY-nY;
					if (nSX < a_pSrcOrigin->nX || nSX >= tSrcEnd.nX)
					{
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*a_pDstSize->nX; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					else
					{
						TPixelChannel const* pSRow = a_pSrcData+(nSX-a_pSrcOrigin->nX)*a_pSrcStride->nX;
						for (LONG nX = a_pDstOrigin->nX; nX < tDstEnd.nX; ++nX)
						{
							LONG nSY = nX-nDX;
							if (nSY < a_pSrcOrigin->nY || nSY >= tSrcEnd.nY)
								*a_pDstData = *a_pDefault;
							else
								*a_pDstData = pSRow[(nSY-a_pSrcOrigin->nY)*a_pSrcStride->nY];
							a_pDstData+=a_pDstStride->nX;
						}
					}
					a_pDstData += a_pDstStride->nY-a_pDstStride->nX*a_pDstSize->nX;
				}
			}
			else if (a_pContentTransform->_11 == -1.0f && a_pContentTransform->_12 == 0.0f &&
				a_pContentTransform->_21 == 0.0f && a_pContentTransform->_22 == -1.0f &&
				a_pDstSize->nX == a_pSrcSize->nX && a_pDstSize->nY == a_pSrcSize->nY)
			{
				a_pDstData += a_pDstStride->nY*(a_pDstSize->nY-1)+a_pDstStride->nX*(a_pDstSize->nX-1);
				for (LONG nY = a_pDstOrigin->nY; nY < tDstEnd.nY; ++nY)
				{
					TPixelChannel const* pSEnd = a_pSrcData+a_pSrcSize->nX*a_pSrcStride->nX;
					while (a_pSrcData != pSEnd)
					{
						*a_pDstData = *a_pSrcData;
						a_pSrcData += a_pSrcStride->nX;
						a_pDstData -= a_pDstStride->nX;
					}
					a_pSrcData += a_pSrcStride->nY-a_pSrcSize->nX*a_pSrcStride->nX;
					a_pDstData -= a_pDstStride->nY-a_pDstSize->nX*a_pDstStride->nX;
				}
			}
			else
			{
				agg::trans_affine tr;
				tr *= agg::trans_affine_translation(a_pDstOrigin->nX, a_pDstOrigin->nY);
				agg::trans_affine tr2(a_pContentTransform->_11, a_pContentTransform->_12, a_pContentTransform->_21, a_pContentTransform->_22, a_pContentTransform->_31/*-a_pDstOrigin->nX*/, a_pContentTransform->_32/*-a_pDstOrigin->nY*/);
				tr2.invert();
				tr *= tr2;

				CAutoPtr<CGammaTables> pGT;
				if (a_fGamma != 1.0f && a_fGamma >= 0.1f && a_fGamma <= 10.f)
					pGT.Attach(new CGammaTables(a_fGamma));

				typedef agg::span_allocator<agg::rgba16> span_alloc_type;
				span_alloc_type sa;
				agg::image_filter_hermite filter_kernel;
				agg::image_filter_lut filter(filter_kernel, false);

				typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
				typedef agg::span_subdiv_adaptor<interpolator_type> subdiv_adaptor_type;
				interpolator_type interpolator(tr);
				subdiv_adaptor_type subdiv_adaptor(interpolator);

				CRasterImagePreMpSrc img_accessor(a_pSrcData, a_pSrcOrigin, a_pSrcSize, a_pSrcStride, *a_pDefault, pGT);
				typedef agg::span_image_filter_rgba_2x2<CRasterImagePreMpSrc, subdiv_adaptor_type> span_gen_type;

				span_gen_type sg(img_accessor, subdiv_adaptor, filter);

				CRasterImageTarget cTarget(a_pDstData, a_pDstSize, a_pDstStride, pGT);
				agg::renderer_base<CRasterImageTarget> renb(cTarget);

				agg::renderer_scanline_aa<agg::renderer_base<CRasterImageTarget>, span_alloc_type, span_gen_type> ren(renb, sa, sg);
				agg::rasterizer_scanline_aa<> ras;
				agg::scanline_u8 sl;
				ras.reset();
				ras.move_to_d(0, 0);
				ras.line_to_d(a_pDstSize->nX, 0);
				ras.line_to_d(a_pDstSize->nX, a_pDstSize->nY);
				ras.line_to_d(0, a_pDstSize->nY);

				agg::render_scanlines(ras, sl, ren);
			}
			return S_OK;

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

STDMETHODIMP CDocumentOperationRasterImageRotate::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentEditableImage> pEI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
		if (pEI == NULL)
			return E_FAIL;

		TImageSize tSize = {1, 1};
		TImagePoint tContOrig = {0, 0};
		TImageSize tContSize = {0, 0};
		pEI->CanvasGet(&tSize, NULL, &tContOrig, &tContSize, NULL);

		CComObjectStackEx<CTransformHelperRotate> cRot;

		TMatrix3x3f tMtx =
		{
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f,
		};

		CConfigValue cAngle;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_ANGLE), &cAngle);
		float const fAngle = cAngle;
		float const fRad = fAngle * agg::pi / -180.0;

		if (fAngle == 0.0f)
			return S_FALSE;

		if (fAngle == 90.0f)
		{
			tMtx._21 = -1.0f;
			tMtx._12 = 1.0f;
		}
		else if (fAngle == 270.0f)
		{
			tMtx._21 = 1.0f;
			tMtx._12 = -1.0f;
		}
		else if (fAngle == 180.0f)
		{
			tMtx._11 = -1.0f;
			tMtx._22 = -1.0f;
		}
		else
		{
			tMtx._11 = cosf(fRad);
			tMtx._21 = sinf(fRad);
			tMtx._12 = -tMtx._21;
			tMtx._22 = tMtx._11;
		}
		CConfigValue cExtend;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_EXTEND), &cExtend);

		TImageSize tOutSize = tSize;
		TVector2f const tCenter1 = {tSize.nX*0.5f, tSize.nY*0.5f};
		TVector2f const tCenter2 = TransformVector2(tMtx, tCenter1);

		if (cExtend.operator LONG() == CFGVAL_EXTEND_AUTO)
		{
			cExtend = CFGVAL_EXTEND_NEVER;
			if (tContOrig.nX >= 0 && tContOrig.nY >= 0 &&
				LONG(tContSize.nX+tContOrig.nX) <= LONG(tSize.nX) && LONG(tContSize.nY+tContOrig.nY) <= LONG(tSize.nY))
			{
				float fX1 = tContOrig.nX-tSize.nX*0.5f;
				float fY1 = tContOrig.nY-tSize.nY*0.5f;
				float fX2 = LONG(tContSize.nX+tContOrig.nX)-tSize.nX*0.5f;
				float fY2 = LONG(tContSize.nY+tContOrig.nY)-tSize.nY*0.5f;
				float fCos = cosf(fRad);
				float fSin = sinf(fRad);
				float fTX1 = fCos*fX1 - fSin*fY1 + tSize.nX*0.5f;
				float fTY1 = fSin*fX1 + fCos*fY1 + tSize.nY*0.5f;
				float fTX2 = fCos*fX1 - fSin*fY2 + tSize.nX*0.5f;
				float fTY2 = fSin*fX1 + fCos*fY2 + tSize.nY*0.5f;
				float fTX3 = fCos*fX1 - fSin*fY2 + tSize.nX*0.5f;
				float fTY3 = fSin*fX1 + fCos*fY2 + tSize.nY*0.5f;
				float fTX4 = fCos*fX2 - fSin*fY2 + tSize.nX*0.5f;
				float fTY4 = fSin*fX2 + fCos*fY2 + tSize.nY*0.5f;
				float fMaxX = fTX1 > fTX2 ? fTX1 : fTX2;
				if (fMaxX < fTX3) fMaxX = fTX3;
				if (fMaxX < fTX4) fMaxX = fTX4;
				float fMaxY = fTY1 > fTY2 ? fTY1 : fTY2;
				if (fMaxY < fTY3) fMaxY = fTY3;
				if (fMaxY < fTY4) fMaxY = fTY4;
				float fMinX = fTX1 < fTX2 ? fTX1 : fTX2;
				if (fMinX > fTX3) fMinX = fTX3;
				if (fMinX > fTX4) fMinX = fTX4;
				float fMinY = fTY1 < fTY2 ? fTY1 : fTY2;
				if (fMinY > fTY3) fMinY = fTY3;
				if (fMinY > fTY4) fMinY = fTY4;
				LONG nMaxX = ceilf(fMaxX);
				LONG nMaxY = ceilf(fMaxY);
				LONG nMinX = floorf(fMinX);
				LONG nMinY = floorf(fMinY);
				if (nMinX < 0 || nMinY < 0 || nMaxX > LONG(tSize.nX) || nMaxY > LONG(tSize.nY))
					cExtend = CFGVAL_EXTEND_ALWAYS;
			}
		}

		if (cExtend.operator LONG() == CFGVAL_EXTEND_NEVER)
		{

			tMtx._31 = tCenter1.x-tCenter2.x;
			tMtx._32 = tCenter1.y-tCenter2.y;
			return pEI->CanvasSet(NULL, NULL, &tMtx, &cRot);
		}

		float fX1 = tSize.nX*0.5f;
		float fY1 = tSize.nY*0.5f;
		float fX2 = tSize.nX*-0.5f;
		float fY2 = tSize.nY*0.5f;
		float fCos = cosf(fRad);
		float fSin = sinf(fRad);
		float fTX1 = fabsf(fCos*fX1 - fSin*fY1);
		float fTY1 = fabsf(fSin*fX1 + fCos*fY1);
		float fTX2 = fabsf(fCos*fX2 - fSin*fY2);
		float fTY2 = fabsf(fSin*fX2 + fCos*fY2);
		tOutSize.nX = (fTX1 > fTX2 ? fTX1 : fTX2)*2.0f + 0.5f;
		tOutSize.nY = (fTY1 > fTY2 ? fTY1 : fTY2)*2.0f + 0.5f;
		if (tOutSize.nX == 0) tOutSize.nX = 1;
		if (tOutSize.nY == 0) tOutSize.nY = 1;

		TVector2f const tOutCenter = {tOutSize.nX*0.5f, tOutSize.nY*0.5f};

		tMtx._31 = tOutCenter.x-tCenter2.x;
		tMtx._32 = tOutCenter.y-tCenter2.y;

		// TODO: split into two calls to extend the whole canvas, but rotate only this layer?
		// or do it in DocumentLayeredImage.cpp in CLayerCanvasSize?
		return pEI->CanvasSet(&tOutSize, NULL, &tMtx, &cRot);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImageRotate::GetDefaultIcon(ULONG a_nSize)
{
	static IRPathPoint const shape[] =
	{
		{200, 58, 0, 0, 0, 0},
		{128, 116, 0, 0, 0, 0},
		{128, 84, -33.1371, 0, 0, 0},
		{68, 144, 0, 33.1371, 0, -33.1371},
		{128, 204, 33.1371, 0, -33.1371, 0},
		{188, 144, 0, 0, 0, 33.1371},
		{240, 144, 0, 61.8559, 0, 0},
		{128, 256, -61.8559, 0, 61.8559, 0},
		{16, 144, 0, -61.8559, 0, 61.8559},
		{128, 32, 0, 0, -61.8559, 0},
		{128, 0, 0, 0, 0, 0},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}

