
#pragma once

#include <EditTool.h>
#include <math.h>


#include "SharedStateEditToolCrop.h"
#include "EditToolCropDlg.h"


#include <agg_basics.h>
#include <agg_color_rgba.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_trans_perspective.h>
#include <agg_renderer_scanline.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_span_subdiv_adaptor.h>
#include <agg_span_allocator.h>
//#include <agg_span_image_resample_rgba.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_interpolator_trans.h>
#include <agg_image_accessors.h>

#include <RWEXIF_i.h>

HICON GetToolIconCROP(ULONG a_nSize);

class CEditToolCrop :
	public CEditToolMouseInput<CEditToolCrop>, // no direct tablet support
	public CEditToolCustomOrMoveCursor<CEditToolCrop, GetToolIconCROP>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolCrop, // T - the top level class for cross casting
		CEditToolCrop, // TResetHandler
		CEditToolCrop, // TDirtyHandler
		CRasterImageEditToolBase, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolCrop, // TAdjustCoordsHandler
		CEditToolCustomOrMoveCursor<CEditToolCrop, GetToolIconCROP>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolCrop>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolCrop, // TControlPointsHandler
		CEditToolCrop // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolCustomApply
{
public:
	CEditToolCrop() : m_bApply(false), m_bMoving(false)
	{
	}

BEGIN_COM_MAP(CEditToolCrop)
	COM_INTERFACE_ENTRY(IRasterImageEditTool)
	COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	COM_INTERFACE_ENTRY(IRasterImageEditToolCustomApply)
END_COM_MAP()

	// IRasterImageEditToolCustomApply methods
public:
	STDMETHOD(ApplyChanges)(BOOL a_bExplicit)
	{
		if (!m_bApply || !a_bExplicit)
			return S_FALSE;

		try
		{
			m_bApply = false;
			CComPtr<IDocument> pDoc;
			M_Window()->Document(&pDoc);
			CComPtr<IDocumentEditableImage> pRI;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pRI));
			if (pRI)
			{
				CUndoBlock cUndo(pDoc, _SharedStringTable.GetStringAuto(IDS_EDITTOOL_CROP_NAME));
				CWriteLock<IDocument> cLock(pDoc);
				TImageSize tSize = {1, 1};
				pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				if (m_cData.eCropMode == CEditToolDataCrop::ECMPerspective)
				{
					// crop with perspective
					TPixelCoords p1 = m_tPixelTL;
					TPixelCoords p2 = m_tPixelTL;
					if (p1.fX > m_tPixelTR.fX) p1.fX = m_tPixelTR.fX;
					if (p1.fY > m_tPixelTR.fY) p1.fY = m_tPixelTR.fY;
					if (p2.fX < m_tPixelTR.fX) p2.fX = m_tPixelTR.fX;
					if (p2.fY < m_tPixelTR.fY) p2.fY = m_tPixelTR.fY;
					if (p1.fX > m_tPixelBR.fX) p1.fX = m_tPixelBR.fX;
					if (p1.fY > m_tPixelBR.fY) p1.fY = m_tPixelBR.fY;
					if (p2.fX < m_tPixelBR.fX) p2.fX = m_tPixelBR.fX;
					if (p2.fY < m_tPixelBR.fY) p2.fY = m_tPixelBR.fY;
					if (p1.fX > m_tPixelBL.fX) p1.fX = m_tPixelBL.fX;
					if (p1.fY > m_tPixelBL.fY) p1.fY = m_tPixelBL.fY;
					if (p2.fX < m_tPixelBL.fX) p2.fX = m_tPixelBL.fX;
					if (p2.fY < m_tPixelBL.fY) p2.fY = m_tPixelBL.fY;

					float fRelativeFocalLen = 1.5f;
					// attempt to read lens properties from EXIF (assume that the image was not croped already)
					CComPtr<IRWEXIF> pEXIF;
					RWCoCreateInstance(pEXIF, __uuidof(RWEXIF));
					CComPtr<IImageMetaData> pIMD;
					pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
					if (pIMD && pEXIF)
					{
						CComBSTR bstrEXIF(L"EXIF");
						ULONG nSize = 0;
						pIMD->GetBlockSize(bstrEXIF, &nSize);
						if (nSize)
						{
							CAutoVectorPtr<BYTE> cData(new BYTE[nSize]);
							pIMD->GetBlock(bstrEXIF, nSize, cData);
							if (SUCCEEDED(pEXIF->Load(nSize, cData)))
							{
								LONG nVal = 0;
								if (SUCCEEDED(pEXIF->TagGetInt(2, 0xa405, 0, &nVal)) && nVal) // focal lenght in 35 mm film
								{
									fRelativeFocalLen = nVal/35.0f;
								}
								else
								{
									nVal = 0;
									ULONG nDenom = 0;
									if (SUCCEEDED(pEXIF->TagGetRat(2, 0x920a, 0, &nVal, &nDenom)) && nVal && nDenom) // focal lenght
									{
										CComBSTR bstrModel;
										if (SUCCEEDED(pEXIF->TagGetStr(0, 0x0110, &bstrModel)) && nVal && nDenom) // focal lenght
										if (bstrModel == L"DSC-S2000")
											fRelativeFocalLen = float(nVal)/nDenom/6.2f;
									}
								}
								nVal = 0;
								ULONG nDenom = 0;
								if (SUCCEEDED(pEXIF->TagGetRat(2, 0xa404, 0, &nVal, &nDenom)) && nVal && nDenom) // digital zoom
								{
									fRelativeFocalLen *= float(nVal)/nDenom;
								}
							}
						}
					}
					// compute output dimensions; TODO: do it correctly from EXIF
					float fDT = sqrtf((m_tPixelTL.fX-m_tPixelTR.fX)*(m_tPixelTL.fX-m_tPixelTR.fX) + (m_tPixelTL.fY-m_tPixelTR.fY)*(m_tPixelTL.fY-m_tPixelTR.fY));
					float fDB = sqrtf((m_tPixelBL.fX-m_tPixelBR.fX)*(m_tPixelBL.fX-m_tPixelBR.fX) + (m_tPixelBL.fY-m_tPixelBR.fY)*(m_tPixelBL.fY-m_tPixelBR.fY));
					float fDL = sqrtf((m_tPixelTL.fX-m_tPixelBL.fX)*(m_tPixelTL.fX-m_tPixelBL.fX) + (m_tPixelTL.fY-m_tPixelBL.fY)*(m_tPixelTL.fY-m_tPixelBL.fY));
					float fDR = sqrtf((m_tPixelBR.fX-m_tPixelTR.fX)*(m_tPixelBR.fX-m_tPixelTR.fX) + (m_tPixelBR.fY-m_tPixelTR.fY)*(m_tPixelBR.fY-m_tPixelTR.fY));
					float fZTL = max(tSize.nX, tSize.nY)*fRelativeFocalLen;
					float fZTR = fZTL*fDL/fDR;
					float fZBL = fZTL*fDT/fDB;
					//float fZBR = fZTR*fZBL;
					TPixelCoords fCenter = {tSize.nX*0.5f, tSize.nY*0.5f};
					float fXTL = (m_tPixelTL.fX-fCenter.fX);
					float fYTL = (m_tPixelTL.fY-fCenter.fY);
					float fXTR = (m_tPixelTR.fX-fCenter.fX)*fZTR/fZTL;
					float fYTR = (m_tPixelTR.fY-fCenter.fY)*fZTR/fZTL;
					float fXBL = (m_tPixelBL.fX-fCenter.fX)*fZBL/fZTL;
					float fYBL = (m_tPixelBL.fY-fCenter.fY)*fZBL/fZTL;
					float fDTP = sqrtf((fXTL-fXTR)*(fXTL-fXTR) + (fYTL-fYTR)*(fYTL-fYTR) + (fZTL-fZTR)*(fZTL-fZTR));
					float fDLP = sqrtf((fXTL-fXBL)*(fXTL-fXBL) + (fYTL-fYBL)*(fYTL-fYBL) + (fZTL-fZBL)*(fZTL-fZBL));
					float fAsp = fDLP/fDTP;
					float fPixels = 0.5f*sqrtf(fDT*fDT*fDL*fDL-((m_tPixelTR.fX-m_tPixelTL.fX)*(m_tPixelBL.fX-m_tPixelTL.fX)+(m_tPixelTR.fY-m_tPixelTL.fY)*(m_tPixelBL.fY-m_tPixelTL.fY))*((m_tPixelTR.fX-m_tPixelTL.fX)*(m_tPixelBL.fX-m_tPixelTL.fX)+(m_tPixelTR.fY-m_tPixelTL.fY)*(m_tPixelBL.fY-m_tPixelTL.fY)))+
									0.5f*sqrtf(fDB*fDB*fDR*fDR-((m_tPixelBL.fX-m_tPixelBR.fX)*(m_tPixelTR.fX-m_tPixelBR.fX)+(m_tPixelBL.fY-m_tPixelBR.fY)*(m_tPixelTR.fY-m_tPixelBL.fY))*((m_tPixelBL.fX-m_tPixelBR.fX)*(m_tPixelTR.fX-m_tPixelBR.fX)+(m_tPixelBL.fY-m_tPixelBR.fY)*(m_tPixelTR.fY-m_tPixelBL.fY)));
					float fDDX = sqrtf(fPixels/fAsp);
					float fDDY = fDDX*fAsp;
					//TPixelCoords tPixelT = {0.5f*(m_tPixelTL.fX+m_tPixelTR.fX), 0.5f*(m_tPixelTL.fY+m_tPixelTR.fY)};
					//TPixelCoords tPixelB = {0.5f*(m_tPixelBL.fX+m_tPixelBR.fX), 0.5f*(m_tPixelBL.fY+m_tPixelBR.fY)};
					//TPixelCoords tPixelL = {0.5f*(m_tPixelTL.fX+m_tPixelBL.fX), 0.5f*(m_tPixelTL.fY+m_tPixelBL.fY)};
					//TPixelCoords tPixelR = {0.5f*(m_tPixelTR.fX+m_tPixelBR.fX), 0.5f*(m_tPixelTR.fY+m_tPixelBR.fY)};
					//float fDDX = sqrtf((tPixelR.fX-tPixelL.fX)*(tPixelR.fX-tPixelL.fX) + (tPixelR.fY-tPixelL.fY)*(tPixelR.fY-tPixelL.fY));
					//float fDDY = sqrtf((tPixelB.fX-tPixelT.fX)*(tPixelB.fX-tPixelT.fX) + (tPixelB.fY-tPixelT.fY)*(tPixelB.fY-tPixelT.fY));
					int nDDX = fDDX+0.5f;
					int nDDY = fDDY+0.5f;
					if (nDDX <= 0 || nDDY <= 0)
						return E_FAIL;
					double dest[8] =
					{
						m_tPixelTL.fX, m_tPixelTL.fY,
						m_tPixelTR.fX, m_tPixelTR.fY,
						m_tPixelBR.fX, m_tPixelBR.fY,
						m_tPixelBL.fX, m_tPixelBL.fY,
					};
					agg::trans_perspective tr(dest, 0, 0, nDDX, nDDY);
					if (!tr.is_valid())
						return E_FAIL;
					//tr.invert();

					// crop the image
					tSize.nX = nDDX;
					tSize.nY = nDDY;

					CComObjectStackEx<CTransformHelperTransform> cTrn;

					TMatrix3x3f tMtx =
					{
						tr.sx, tr.shy, tr.w0,
						tr.shx, tr.sy, tr.w1,
						tr.tx, tr.ty, tr.w2
					};

					pRI->CanvasSet(&tSize, NULL, &tMtx, &cTrn);
				}
				else
				{
					// simple rectangular crop
					tSize.nX = m_tPixelBR.fX-m_tPixelTL.fX;
					tSize.nY = m_tPixelBR.fY-m_tPixelTL.fY;
					TMatrix3x3f tTransform = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  -m_tPixelTL.fX, -m_tPixelTL.fY, 1.0f};
					pRI->CanvasSet(&tSize, NULL, &tTransform, NULL);
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		m_bApply = false;
		static TPixelCoords const tZeroPixel = {0, 0};
		m_tPixelTL = m_tPixelTR = m_tPixelBL = m_tPixelBR = tZeroPixel;
		m_nPixels = 0;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataCrop::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			CEditToolDataCrop cPrev = m_cData;
			m_cData = *(pData->InternalData());
			if (m_bApply && m_cData.eCropMode != CEditToolDataCrop::ECMPerspective)
			{
				// fix points - make it rectangular
				TPixelCoords p1 = m_tPixelTL;
				TPixelCoords p2 = m_tPixelTL;
				if (p1.fX > m_tPixelTR.fX) p1.fX = m_tPixelTR.fX;
				if (p1.fY > m_tPixelTR.fY) p1.fY = m_tPixelTR.fY;
				if (p2.fX < m_tPixelTR.fX) p2.fX = m_tPixelTR.fX;
				if (p2.fY < m_tPixelTR.fY) p2.fY = m_tPixelTR.fY;
				if (p1.fX > m_tPixelBR.fX) p1.fX = m_tPixelBR.fX;
				if (p1.fY > m_tPixelBR.fY) p1.fY = m_tPixelBR.fY;
				if (p2.fX < m_tPixelBR.fX) p2.fX = m_tPixelBR.fX;
				if (p2.fY < m_tPixelBR.fY) p2.fY = m_tPixelBR.fY;
				if (p1.fX > m_tPixelBL.fX) p1.fX = m_tPixelBL.fX;
				if (p1.fY > m_tPixelBL.fY) p1.fY = m_tPixelBL.fY;
				if (p2.fX < m_tPixelBL.fX) p2.fX = m_tPixelBL.fX;
				if (p2.fY < m_tPixelBL.fY) p2.fY = m_tPixelBL.fY;
				if (m_tPixelTL.fX != m_tPixelBL.fX || m_tPixelTR.fX != m_tPixelBR.fX ||
					m_tPixelTL.fY != m_tPixelTR.fY || m_tPixelBL.fY != m_tPixelBR.fY)
				{
					m_nPixels = (p2.fX-p1.fX)*(p2.fY-p1.fY);
				}
				m_tPixelTL = p1;
				m_tPixelTR.fX = p2.fX;
				m_tPixelTR.fY = p1.fY;
				m_tPixelBL.fX = p1.fX;
				m_tPixelBL.fY = p2.fY;
				m_tPixelBR = p2;
				if (m_cData.eProportionsMode && m_nPixels)
				{
					float fAspect = fabsf(p2.fX-p1.fX) >= fabsf(p2.fY-p1.fY) ? float(m_cData.eProportionsMode&0xffff)/float(m_cData.eProportionsMode>>16) : float(m_cData.eProportionsMode>>16)/float(m_cData.eProportionsMode&0xffff);
					float fSizeX = sqrtf(m_nPixels*fAspect);
					float fSizeY = fSizeX/fAspect;
					int nSizeX = fSizeX + 0.5f;
					int nSizeY = fSizeY + 0.5f;
					int nDeltaX = p2.fX-p1.fX-nSizeX;
					int nDeltaY = p2.fY-p1.fY-nSizeY;
					int nD1X = nDeltaX>>1;
					int nD2X = nDeltaX-nD1X;
					int nD1Y = nDeltaY>>1;
					int nD2Y = nDeltaY-nD1Y;
					m_tPixelTL.fX += nD1X;
					m_tPixelTL.fY += nD1Y;
					m_tPixelTR.fX -= nD2X;
					m_tPixelTR.fY += nD1Y;
					m_tPixelBL.fX += nD1X;
					m_tPixelBL.fY -= nD2Y;
					m_tPixelBR.fX -= nD2X;
					m_tPixelBR.fY -= nD2Y;
				}
				if (m_cData.eCropMode == CEditToolDataCrop::ECMLosslessJPEG)
				{
					static TPixelCoords const tPointerSize = {0.0f, 0.0f};
					_AdjustCoordinates(ECKSNone, &m_tPixelTL, &tPointerSize, NULL, 0.0f);
					_AdjustCoordinates(ECKSNone, &m_tPixelTR, &tPointerSize, NULL, 0.0f);
					_AdjustCoordinates(ECKSNone, &m_tPixelBR, &tPointerSize, NULL, 0.0f);
					_AdjustCoordinates(ECKSNone, &m_tPixelBL, &tPointerSize, NULL, 0.0f);
				}
				for (int i = 0; i < 4; ++i)
					M_Window()->ControlPointChanged(i);
				M_Window()->ControlLinesChanged();
			}
		}
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		return m_bApply ? S_OK : S_FALSE;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		switch (m_cData.eCropMode)
		{
		case CEditToolDataCrop::ECMPerspective:
			break;
		case CEditToolDataCrop::ECMLosslessJPEG:
			a_pPos->fX = 16*floorf(a_pPos->fX*0.0625f+0.5f);
			a_pPos->fY = 16*floorf(a_pPos->fY*0.0625f+0.5f);
			break;
		case CEditToolDataCrop::ECMStandard:
			a_pPos->fX = floorf(a_pPos->fX+0.5f);
			a_pPos->fY = floorf(a_pPos->fY+0.5f);
			break;
		}

		return S_OK;
	}

	HRESULT OnMouseDown(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (UseMoveCursor(a_pPos))
		{
			m_bMoving = true;
			m_tLastPixel = *a_pPos;
			return S_OK;
		}
		m_bMoving = false;
		m_bApply = false; // trying it again
		m_tPixelTL = m_tPixelTR = m_tPixelBL = m_tPixelBR = m_tLastPixel = *a_pPos;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		return S_OK;
	}
	HRESULT OnMouseUp(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		OnMouseMove(a_eKeysState, a_pPos);
		m_bMoving = false;
		M_Window()->ControlPointsChanged();
		return S_OK;
	}
	HRESULT OnMouseMove(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (!M_Dragging())
			return S_OK;
		if (m_bMoving)
		{
			if (a_pPos->fX != m_tLastPixel.fX || a_pPos->fY != m_tLastPixel.fY)
			{
				m_tPixelTL.fX += a_pPos->fX-m_tLastPixel.fX;
				m_tPixelTL.fY += a_pPos->fY-m_tLastPixel.fY;
				m_tPixelTR.fX += a_pPos->fX-m_tLastPixel.fX;
				m_tPixelTR.fY += a_pPos->fY-m_tLastPixel.fY;
				m_tPixelBL.fX += a_pPos->fX-m_tLastPixel.fX;
				m_tPixelBL.fY += a_pPos->fY-m_tLastPixel.fY;
				m_tPixelBR.fX += a_pPos->fX-m_tLastPixel.fX;
				m_tPixelBR.fY += a_pPos->fY-m_tLastPixel.fY;
				m_tLastPixel = *a_pPos;
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointsChanged();
			}
			return S_OK;
		}

		m_tPixelTL.fX = m_tPixelBL.fX = min(m_tLastPixel.fX, a_pPos->fX);
		m_tPixelTR.fX = m_tPixelBR.fX = max(m_tLastPixel.fX, a_pPos->fX);
		m_tPixelTL.fY = m_tPixelTR.fY = min(m_tLastPixel.fY, a_pPos->fY);
		m_tPixelBL.fY = m_tPixelBR.fY = max(m_tLastPixel.fY, a_pPos->fY);
		if (m_cData.eCropMode != CEditToolDataCrop::ECMPerspective && m_cData.eProportionsMode != CEditToolDataCrop::EPMArbitrary)
		{
			float dX = m_tPixelBR.fX-m_tPixelTL.fX;
			float dY = m_tPixelBR.fY-m_tPixelTL.fY;
			float fAspect = fabsf(dX) >= fabsf(dY) ? float(m_cData.eProportionsMode&0xffff)/float(m_cData.eProportionsMode>>16) : float(m_cData.eProportionsMode>>16)/float(m_cData.eProportionsMode&0xffff);
			float dX2 = (dY*fAspect+dX)*0.5f;
			float dY2 = dX2/fAspect;
			//float dX2 = min(dX, dY*fAspect);
			//float dY2 = min(dY, dX/fAspect);
			int nX = m_cData.eCropMode == CEditToolDataCrop::ECMStandard ? (dX2+0.5f) : 16*floorf(dX2*0.0625f+0.5f);
			int nY = m_cData.eCropMode == CEditToolDataCrop::ECMStandard ? (dY2+0.5f) : 16*floorf(dY2*0.0625f+0.5f);
			if (m_tLastPixel.fY >= a_pPos->fY)
			{
				if (m_tLastPixel.fX >= a_pPos->fX)
				{
					m_tPixelTL.fX = m_tPixelBR.fX-nX;
					m_tPixelTL.fY = m_tPixelBR.fY-nY;
				}
				else
				{
					m_tPixelBR.fX = m_tPixelTL.fX+nX;
					m_tPixelTL.fY = m_tPixelBR.fY-nY;
				}
			}
			else
			{
				if (m_tLastPixel.fX >= a_pPos->fX)
				{
					m_tPixelTL.fX = m_tPixelBR.fX-nX;
					m_tPixelBR.fY = m_tPixelTL.fY+nY;
				}
				else
				{
					m_tPixelBR.fX = m_tPixelTL.fX+nX;
					m_tPixelBR.fY = m_tPixelTL.fY+nY;
				}
			}
			m_tPixelTR.fX = m_tPixelBR.fX;
			m_tPixelTR.fY = m_tPixelTL.fY;
			m_tPixelBL.fX = m_tPixelTL.fX;
			m_tPixelBL.fY = m_tPixelBR.fY;
		}
		m_nPixels = (m_tPixelBR.fX-m_tPixelTL.fX)*(m_tPixelBR.fY-m_tPixelTL.fY);
		m_bApply = m_tLastPixel.fX != a_pPos->fX && m_tLastPixel.fY != a_pPos->fY;
		M_Window()->ControlLinesChanged();
		//M_Window()->ControlPointsChanged();
		return S_OK;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_bApply ? 4 : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (!m_bApply || a_nIndex >= 4)
			return E_RW_INDEXOUTOFRANGE;

		switch (a_nIndex)
		{
		case 0: *a_pPos = m_tPixelTL; break;
		case 1: *a_pPos = m_tPixelTR; break;
		case 2: *a_pPos = m_tPixelBR; break;
		case 3: *a_pPos = m_tPixelBL; break;
		}
		*a_pClass = 0;
		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (!m_bApply || a_nIndex >= 4)
			return E_RW_INDEXOUTOFRANGE;
		if (m_cData.eCropMode == CEditToolDataCrop::ECMPerspective)
		{
			switch (a_nIndex)
			{
			case 0: m_tPixelTL = *a_pPos; break;
			case 1: m_tPixelTR = *a_pPos; break;
			case 2: m_tPixelBR = *a_pPos; break;
			case 3: m_tPixelBL = *a_pPos; break;
			}
			M_Window()->ControlPointChanged(a_nIndex);
			M_Window()->ControlLinesChanged();
		}
		else
		{
			switch (a_nIndex)
			{
			case 0: m_tPixelTL = *a_pPos; break;
			case 1: m_tPixelTL.fY = a_pPos->fY; m_tPixelBR.fX = a_pPos->fX; break;
			case 2: m_tPixelBR = *a_pPos; break;
			case 3: m_tPixelTL.fX = a_pPos->fX; m_tPixelBR.fY = a_pPos->fY; break;
			}
			if (m_cData.eProportionsMode != CEditToolDataCrop::EPMArbitrary)
			{
				float dX = m_tPixelBR.fX-m_tPixelTL.fX;
				float dY = m_tPixelBR.fY-m_tPixelTL.fY;
				float fAspect = fabsf(dX) >= fabsf(dY) ? float(m_cData.eProportionsMode&0xffff)/float(m_cData.eProportionsMode>>16) : float(m_cData.eProportionsMode>>16)/float(m_cData.eProportionsMode&0xffff);
				float dX2 = (dY*fAspect+dX)*0.5f;
				float dY2 = dX2/fAspect;
				int nX = m_cData.eCropMode == CEditToolDataCrop::ECMStandard ? (dX2+0.5f) : 16*floorf(dX2*0.0625f+0.5f);
				int nY = m_cData.eCropMode == CEditToolDataCrop::ECMStandard ? (dY2+0.5f) : 16*floorf(dY2*0.0625f+0.5f);
				switch (a_nIndex)
				{
				case 0: m_tPixelTL.fX = m_tPixelBR.fX-nX; m_tPixelTL.fY = m_tPixelBR.fY-nY; break;
				case 1: m_tPixelBR.fX = m_tPixelTL.fX+nX; m_tPixelTL.fY = m_tPixelBR.fY-nY; break;
				case 2: m_tPixelBR.fX = m_tPixelTL.fX+nX; m_tPixelBR.fY = m_tPixelTL.fY+nY; break;
				case 3: m_tPixelTL.fX = m_tPixelBR.fX-nX; m_tPixelBR.fY = m_tPixelTL.fY+nY; break;
				}
			}
			m_nPixels = (m_tPixelBR.fX-m_tPixelTL.fX)*(m_tPixelBR.fY-m_tPixelTL.fY);
			m_tPixelTR.fX = m_tPixelBR.fX;
			m_tPixelTR.fY = m_tPixelTL.fY;
			m_tPixelBL.fX = m_tPixelTL.fX;
			m_tPixelBL.fY = m_tPixelBR.fY;
			M_Window()->ControlPointChanged(0);
			M_Window()->ControlPointChanged(1);
			M_Window()->ControlPointChanged(2);
			M_Window()->ControlPointChanged(3);
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if ((M_Dragging() || m_bApply) && (
			m_tPixelTL.fX != m_tPixelTR.fX || m_tPixelTL.fY != m_tPixelTR.fY ||
			m_tPixelTR.fX != m_tPixelBR.fX || m_tPixelTR.fY != m_tPixelBR.fY ||
			m_tPixelBR.fX != m_tPixelBL.fX || m_tPixelBR.fY != m_tPixelBL.fY ||
			m_tPixelBL.fX != m_tPixelTL.fX || m_tPixelBL.fY != m_tPixelTL.fY))
		{
			a_pLines->MoveTo(m_tPixelTL.fX, m_tPixelTL.fY);
			a_pLines->LineTo(m_tPixelTR.fX, m_tPixelTR.fY);
			a_pLines->MoveTo(m_tPixelTL.fX, m_tPixelTL.fY);
			a_pLines->LineTo(m_tPixelBL.fX, m_tPixelBL.fY);
			a_pLines->MoveTo(m_tPixelBR.fX, m_tPixelBR.fY);
			a_pLines->LineTo(m_tPixelTR.fX, m_tPixelTR.fY);
			a_pLines->MoveTo(m_tPixelBR.fX, m_tPixelBR.fY);
			a_pLines->LineTo(m_tPixelBL.fX, m_tPixelBL.fY);
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		if (!m_bApply || m_cData.eCropMode == CEditToolDataCrop::ECMPerspective)
			return false;
		return a_pPos->fX >= m_tPixelTL.fX && a_pPos->fX < m_tPixelBR.fX && a_pPos->fY >= m_tPixelTL.fY && a_pPos->fY < m_tPixelBR.fY; 
	}
	STDMETHOD(PointTest)(EControlKeysState (a_eKeysState), TPixelCoords const* (a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;

			float f1 = 0.0f;
			float f2 = 0.0f;
			float f3 = 0.0f;
			float f4 = 0.0f;
			float f5 = 0.0f;
			float f6 = 0.0f;
			float f7 = 0.0f;
			float f8 = 0.0f;
			int n = swscanf(a_bstrParams, L"%f,%f,%f,%f,%f,%f,%f,%f", &f1, &f2, &f3, &f4, &f5, &f6, &f7, &f8);
			if (n == 8)
			{
				m_tPixelTL.fX = f1;
				m_tPixelTL.fY = f2;
				m_tPixelTR.fX = f3;
				m_tPixelTR.fY = f4;
				m_tPixelBL.fX = f5;
				m_tPixelBL.fY = f6;
				m_tPixelBR.fX = f7;
				m_tPixelBR.fY = f8;
				m_cData.eCropMode = CEditToolDataCrop::ECMPerspective;
			}
			else if (n == 4)
			{
				m_tPixelTL.fX = f1;
				m_tPixelTL.fY = f2;
				m_tPixelTR.fX = f3;
				m_tPixelTR.fY = f2;
				m_tPixelBL.fX = f1;
				m_tPixelBL.fY = f4;
				m_tPixelBR.fX = f3;
				m_tPixelBR.fY = f4;
				m_cData.eCropMode = CEditToolDataCrop::ECMStandard;
			}
			else
				return S_FALSE;
			m_bMoving = false;
			m_nPixels = (m_tPixelBR.fX-m_tPixelTL.fX)*(m_tPixelBR.fY-m_tPixelTL.fY);
			m_bApply = m_nPixels != 0;
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		try
		{
			*a_pbstrParams = NULL;
			OLECHAR sz[256];
			if (m_cData.eCropMode == CEditToolDataCrop::ECMPerspective)
				swprintf(sz, L"%g, %g, %g, %g, %g, %g, %g, %g", m_tPixelTL.fX, m_tPixelTL.fY, m_tPixelTR.fX, m_tPixelTR.fY, m_tPixelBL.fX, m_tPixelBL.fY, m_tPixelBR.fX, m_tPixelBR.fY);
			else
				swprintf(sz, L"%g, %g, %g, %g", m_tPixelTL.fX, m_tPixelTL.fY, m_tPixelBR.fX, m_tPixelBR.fY);
			CComBSTR bstr(sz);
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class CRasterImageTarget
	{
	public:
		typedef agg::rgba16 color_type;
		typedef void row_data;
		typedef void span_data;

		CRasterImageTarget(TPixelChannel* a_pBuffer, TImageSize const* a_pSize, TImageStride const* a_pStride, CGammaTables* a_pGamma) :
			m_pBuffer(a_pBuffer), m_tSize(*a_pSize), m_tStride(*a_pStride), m_pGamma(a_pGamma)
		{
		}

		unsigned width() const
		{
			return m_tSize.nX;
		}
		unsigned height() const
		{
			return m_tSize.nY;
		}

		//void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
		//void blend_hline(int x, int y, unsigned len, color_type const& c, agg::int8u cover)
		//void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)

		void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
		{
			if (m_pGamma)
			{
				for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
				{
					color_type c = *colors;
					c.demultiply();
					p->bB = m_pGamma->InvGamma(c.b);
					p->bG = m_pGamma->InvGamma(c.g);
					p->bR = m_pGamma->InvGamma(c.r);
					p->bA = c.a>>8;
				}
			}
			else
			{
				for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
				{
					color_type c = *colors;
					c.demultiply();
					p->bB = c.b>>8;
					p->bG = c.g>>8;
					p->bR = c.r>>8;
					p->bA = c.a>>8;
				}
			}
			for (color_type* p = reinterpret_cast<color_type*>(m_pBuffer + y*m_tStride.nY+x*m_tStride.nX); len; p+=m_tStride.nX, --len, ++covers, ++colors)
			{
				*p = *colors;
				p->demultiply();
			}
		}

	private:
		TPixelChannel* m_pBuffer;
		TImageSize const m_tSize;
		TImageStride const m_tStride;
		CGammaTables* m_pGamma;
	};

	class CRasterImagePreMpSrc
	{
	public:
		typedef agg::pixfmt_rgba64::color_type color_type;
		typedef agg::pixfmt_rgba64::order_type order_type;

		CRasterImagePreMpSrc(TPixelChannel const* a_pData, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, TPixelChannel a_tDefault, CGammaTables* a_pGamma) :
			m_pData(a_pData), m_tOrigin(*a_pOrigin), m_tSize(*a_pSize), m_tStride(*a_pStride),
			m_tEnd(CImagePoint(a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY)), m_x(0), m_x0(0), m_y(0), m_pBuffer(NULL), m_pGamma(a_pGamma)
		{
			if (m_pGamma)
			{
				m_aDefault[0] = m_pGamma->m_aGamma[a_tDefault.bR]*ULONG(a_tDefault.bA)/255;
				m_aDefault[1] = m_pGamma->m_aGamma[a_tDefault.bG]*ULONG(a_tDefault.bA)/255;
				m_aDefault[2] = m_pGamma->m_aGamma[a_tDefault.bB]*ULONG(a_tDefault.bA)/255;
			}
			else
			{
				m_aDefault[0] = ((ULONG(a_tDefault.bR)<<8)|a_tDefault.bR)*ULONG(a_tDefault.bA)/255;
				m_aDefault[1] = ((ULONG(a_tDefault.bG)<<8)|a_tDefault.bG)*ULONG(a_tDefault.bA)/255;
				m_aDefault[2] = ((ULONG(a_tDefault.bB)<<8)|a_tDefault.bB)*ULONG(a_tDefault.bA)/255;
			}
			m_aDefault[3] = (ULONG(a_tDefault.bA)<<8)|a_tDefault.bA;
		}

	public:
		AGG_INLINE const agg::int16u* span(int x, int y, unsigned UNREF(len))
		{
			m_x = m_x0 = x;
			m_y = y;
			if (y < m_tOrigin.nY || y >= m_tEnd.nY || x < m_tOrigin.nX || x >= m_tEnd.nX)
			{
				m_pBuffer = NULL;
				return m_aDefault;
			}
			else
			{
				m_pBuffer = m_pData + (x-m_tOrigin.nX)*m_tStride.nX+(y-m_tOrigin.nY)*m_tStride.nY;
				if (m_pGamma)
				{
					m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
				}
				else
				{
					m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
				}
				m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
				return m_aBuffer;
			}
		}

		AGG_INLINE const agg::int16u* next_x()
		{
			++m_x;
			if (m_pBuffer)
			{
				if (m_x < m_tEnd.nX)
				{
					m_pBuffer += m_tStride.nX;
					if (m_pGamma)
					{
						m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
						m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
						m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
					}
					else
					{
						m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
						m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
						m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
					}
					m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
					return m_aBuffer;
				}
				else
				{
					m_pBuffer = NULL;
					return m_aDefault;
				}
			}
			if (m_x >= m_tOrigin.nX && m_x < m_tEnd.nX && m_y >= m_tOrigin.nY && m_y < m_tEnd.nY)
			{
				m_pBuffer = m_pData + (m_x-m_tOrigin.nX)*m_tStride.nX+(m_y-m_tOrigin.nY)*m_tStride.nY;
				if (m_pGamma)
				{
					m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
				}
				else
				{
					m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
				}
				m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
				return m_aBuffer;
			}

			return m_aDefault;
		}

		AGG_INLINE const agg::int16u* next_y()
		{
			return span(m_x0, m_y+1, 0);
		}

	private:
		TPixelChannel const* m_pData;
		TImagePoint const m_tOrigin;
		TImagePoint const m_tEnd;
		TImageSize const m_tSize;
		TImageStride const m_tStride;
		agg::int16u m_aDefault[4];
		TPixelChannel const* m_pBuffer;
		agg::int16u m_aBuffer[4];
		CGammaTables* m_pGamma;
		int m_x, m_x0, m_y;
	};

	class ATL_NO_VTABLE CTransformHelperTransform : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IRasterImageTransformer
	{
	public:
		CTransformHelperTransform()
		{
		}

	BEGIN_COM_MAP(CTransformHelperTransform)
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

				TMatrix3x3f tInv;
				Matrix3x3fInverse(*a_pContentTransform, &tInv);
				TMatrix3x3f tMul;
				TMatrix3x3f tShift = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, a_pDstOrigin->nX, a_pDstOrigin->nY, 1.0f};
				Matrix3x3fMultiply(tShift, tInv, &tMul);
				agg::trans_perspective tr(
					tMul._11, tMul._12, tMul._13,
					tMul._21, tMul._22, tMul._23,
					tMul._31, tMul._32, tMul._33);

				CAutoPtr<CGammaTables> pGT;
				if (a_fGamma != 1.0f && a_fGamma >= 0.1f && a_fGamma <= 10.f)
					pGT.Attach(new CGammaTables(a_fGamma));

				typedef agg::span_allocator<agg::rgba16> span_alloc_type;
				span_alloc_type sa;
				agg::image_filter_hermite filter_kernel;
				agg::image_filter_lut filter(filter_kernel, false);

				typedef agg::span_interpolator_trans<agg::trans_perspective> interpolator_type;
				interpolator_type interpolator(tr);

				CRasterImagePreMpSrc img_accessor(a_pSrcData, a_pSrcOrigin, a_pSrcSize, a_pSrcStride, *a_pDefault, pGT);

				typedef agg::span_image_filter_rgba_2x2<CRasterImagePreMpSrc, interpolator_type> span_gen_type;
				span_gen_type sg(img_accessor, interpolator, filter);

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
				return S_OK;

			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
	};

private:
	TPixelCoords m_tPixelTL;
	TPixelCoords m_tPixelTR;
	TPixelCoords m_tPixelBL;
	TPixelCoords m_tPixelBR;
	TPixelCoords m_tLastPixel;
	ULONG m_nPixels;
	bool m_bApply;
	bool m_bMoving;
	CEditToolDataCrop m_cData;
};

