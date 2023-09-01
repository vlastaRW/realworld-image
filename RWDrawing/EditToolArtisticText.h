
#pragma once


#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithBrush.h"
#include "EditToolPolyLine.h"
#include "EditToolWithCtrlDropper.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_bspline.h>
#include <agg_conv_segmentator.h>
#include <agg_font_win32_tt.h>
#include <agg_trans_single_path.h>


HICON GetToolIconTEXT(ULONG a_nSize);

class CEditToolArtisticText :
	public CEditToolScanlineBuffer<CEditToolArtisticText>, // scanline image cache
	public CEditToolPolyLine<CEditToolArtisticText>, // polyline-based shape
	public CEditToolMouseInput<CEditToolArtisticText>, // no direct tablet support
	public CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, // brush override
	public CEditToolColors<CEditToolArtisticText>, // colors handling
	public CEditToolCustomOrMoveCursor<CEditToolArtisticText, GetToolIconTEXT>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolArtisticText, CEditToolMouseInput<CEditToolArtisticText>, CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, CEditToolCustomOrMoveCursor<CEditToolArtisticText, GetToolIconTEXT> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolArtisticText, // T - the top level class for cross casting
		CEditToolArtisticText, // TResetHandler
		CEditToolScanlineBuffer<CEditToolArtisticText>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolArtisticText>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolColors<CEditToolArtisticText>, // TColorsHandler
		CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, // TBrushHandler
		CEditToolArtisticText, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolArtisticText, CEditToolMouseInput<CEditToolArtisticText>, CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, CEditToolCustomOrMoveCursor<CEditToolArtisticText, GetToolIconTEXT> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolArtisticText, CEditToolMouseInput<CEditToolArtisticText>, CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, CEditToolCustomOrMoveCursor<CEditToolArtisticText, GetToolIconTEXT> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolArtisticText, CEditToolMouseInput<CEditToolArtisticText>, CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, CEditToolCustomOrMoveCursor<CEditToolArtisticText, GetToolIconTEXT> >, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolArtisticText, CEditToolPolyLine<CEditToolArtisticText>, CEditToolArtisticText> // TControlLinesHandler
	>
{
public:
	CEditToolArtisticText() : m_eBlendingMode(EBMDrawOver),// m_eShapeFillMode(ESFMSolidFill),
		m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		m_bMoving(false), m_bUseSecondary(false), m_fGamma(1.0f)
	{
	}

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
	}
	void generate(TPixelChannel* span, int x, int y, unsigned len)
	{
		M_Brush()->GetBrushTile(x, y, len, 1, m_fGamma, m_bUseSecondary, len, reinterpret_cast<TRasterImagePixel*>(span));
	}
	void prepare()
	{
	}
	void PrepareShape()
	{
		agg::renderer_base<CEditToolScanlineBuffer> renb(*this);

		// Scanline renderer for solid filling.
		agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);

		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		CVertexSource pl(this);

		agg::conv_bspline<CVertexSource> bspline(pl);
		bspline.interpolation_step(0.1 / M_PolyLine().size());

		TColor tColor = m_bUseSecondary ? M_Color2() : M_Color1();
		ren.color(CPixelChannel(tColor.fR*255.0f + 0.5f, tColor.fG*255.0f + 0.5f, tColor.fB*255.0f + 0.5f, tColor.fA*255.0f + 0.5f));




		agg::trans_single_path tcurve;
		tcurve.add_path(bspline);
		tcurve.preserve_x_scale(true);
		//if(m_fixed_len.status()) tcurve.base_length(1120);

		typedef agg::font_engine_win32_tt_int16 font_engine_type;
		typedef agg::font_cache_manager<font_engine_type> font_manager_type;
		HDC hDC = GetDC(NULL);
		font_engine_type             m_feng(hDC);
		font_manager_type            m_fman(m_feng);
		typedef agg::conv_curve<font_manager_type::path_adaptor_type>             conv_font_curve_type;
		typedef agg::conv_segmentator<conv_font_curve_type>                      conv_font_segm_type;
		typedef agg::conv_transform<conv_font_segm_type, agg::trans_single_path> conv_font_trans_type;
		conv_font_curve_type fcurves(m_fman.path_adaptor());

		conv_font_segm_type  fsegm(fcurves);
		conv_font_trans_type ftrans(fsegm, tcurve);
		fsegm.approximation_scale(3.0);
		fcurves.approximation_scale(2.0);

		m_feng.height(40.0);
		m_feng.flip_y(true);
		//m_feng.italic(true);

		if(m_feng.create_font(_T("Times New Roman"), agg::glyph_ren_outline))
		{
			double x = 0.0;
			double y = 3.0;
			const char* p = "Testovaci text";

			while(*p)
			{
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if(glyph)
				{
					if(x > tcurve.total_length()) break;

					m_fman.add_kerning(&x, &y);
					m_fman.init_embedded_adaptors(glyph, x, y);

					if(glyph->data_type == agg::glyph_data_outline)
					{
						ras.reset();
						ras.add_path(ftrans);
						//r.color(agg::rgba8(0, 0, 0));
						agg::render_scanlines(ras, sl, ren);
					}

					// increment pen position
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}

		}

		ReleaseDC(NULL, hDC);
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		RECT const rc = M_DirtyRect();
		m_bMoving = false;
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		ResetPolyLine();
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		EnableBrush(false);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return E_NOTIMPL;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		bool bBlendingChange = m_eBlendingMode != a_eBlendingMode;
		bool bRasterizationChange = m_eRasterizationMode != a_eRasterizationMode;
		bool bCoordinatesChange = m_eCoordinatesMode != a_eCoordinatesMode;

		if (!bBlendingChange && !bRasterizationChange && !bCoordinatesChange)
			return S_FALSE;
		m_eBlendingMode = a_eBlendingMode;
		m_eRasterizationMode = a_eRasterizationMode;
		m_eCoordinatesMode = a_eCoordinatesMode;
		if (M_PolyLine().empty())
			return S_OK;
		RECT rcPrev = M_DirtyRect();
		bool bRedrawCache = bRasterizationChange;
		if (bCoordinatesChange && m_eCoordinatesMode == ECMIntegral)
		{
			static TPixelCoords const tPointerSize = {0.0f, 0.0f};
			for (CPolyLine::iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i)
				_AdjustCoordinates(ECKSNone, &*i, &tPointerSize, NULL, 0.0f);
			for (ULONG i = 0; i < M_PolyLine().size()<<1; ++i)
				M_Window()->ControlPointChanged(i);
			M_Window()->ControlLinesChanged();
			bRedrawCache = true;
		}
		if (bRedrawCache)
		{
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
		}
		else if (bBlendingChange)
		{
			M_Window()->RectangleChanged(&rcPrev);
		}
		EnableBrush((!M_Dragging() || m_bMoving) && rcPrev.left < rcPrev.right && m_eShapeFillMode&ESFMFilled);
		return S_OK;
	}
	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (HitTest(a_pPos->fX, a_pPos->fY))
		{
			m_bMoving = true;
			m_tLastPos = *a_pPos;
			return S_OK;
		}
		m_bMoving = false; // just in case...

		if (!M_PolyLine().empty())
		{
			ATLASSERT(0);
			//M_Window()->ApplyChanges();
			return ETPAApply|ETPAStartNew;
		}

		SetPolyLineControlLines(false);
		M_PolyLine().push_back(*a_pPos);
		M_PolyLine().push_back(*a_pPos);
		m_bUseSecondary = a_eKeysState&ECKSShift;
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_bMoving)
		{
			OnMouseMove(a_eKeysState, a_pPos);
			m_bMoving = false;
			return S_OK;
		}

		if (a_pPos->fX == M_PolyLine()[0].fX && a_pPos->fY == M_PolyLine()[0].fY)
		{
			M_PolyLine().clear();
			return S_FALSE;
		}
		if (a_pPos->fX != M_PolyLine()[1].fX || a_pPos->fY != M_PolyLine()[1].fY)
			OnMouseMove(a_eKeysState, a_pPos);
		SetPolyLineControlLines(true);
		M_Window()->ControlPointsChanged();
		//M_Window()->ControlLinesChanged();
		EnableBrush(m_eShapeFillMode&ESFMFilled);
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_bMoving)
		{
			if (m_tLastPos.fX != a_pPos->fX || m_tLastPos.fY != a_pPos->fY)
			{
				float const fDX = a_pPos->fX-m_tLastPos.fX;
				float const fDY = a_pPos->fY-m_tLastPos.fY;
				m_tLastPos = *a_pPos;
				ULONG ii = 0;
				for (CPolyLine::iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i)
				{
					i->fX += fDX;
					i->fY += fDY;
					ii += 2;
				}
				for (ULONG i = 0; i < ii; ++i)
					M_Window()->ControlPointChanged(i);
				M_Window()->ControlLinesChanged();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				ToolSetBrush();
			}
			return S_OK;
		}
		if (!M_Dragging())
			return S_OK;

		if (M_PolyLine()[1].fX == a_pPos->fX && M_PolyLine()[1].fY == a_pPos->fY)
			return S_FALSE;
		M_PolyLine()[1] = *a_pPos;
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		ToolSetBrush();
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_bMoving || (!M_Dragging() && HitTest(a_pPos->fX, a_pPos->fY));
	}

	void ToolSetBrush()
	{
		if (M_Brush())
		{
			RECT const rc = M_DirtyRect();
			if (rc.left < rc.right && rc.top < rc.bottom)
			{
				TPixelCoords const tCenter = {(rc.right+rc.left)*0.5f, (rc.bottom+rc.top)*0.5f};
				M_Brush()->SetShapeBounds(&tCenter, (rc.right-rc.left)*0.5f, (rc.bottom-rc.top)*0.5f, 0.0f);
			}
		}
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		if (m_eCoordinatesMode == ECMIntegral)
		{
			a_pPos->fX = 0.5f + floorf(a_pPos->fX+0.5f*a_pPointerSize->fX);
			a_pPos->fY = 0.5f + floorf(a_pPos->fY+0.5f*a_pPointerSize->fY);
		}
		else
		{
			a_pPos->fX += 0.5f*a_pPointerSize->fX;
			a_pPos->fY += 0.5f*a_pPointerSize->fY;
		}
		return S_OK;
	}

	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		InvalidateCachePixels();
		if (a_pChanged == NULL)
			return M_Window()->RectangleChanged(&M_DirtyRect());
		RECT rcShape = M_DirtyRect();
		if (rcShape.left < a_pChanged->left) rcShape.left = a_pChanged->left;
		if (rcShape.top < a_pChanged->top) rcShape.top = a_pChanged->top;
		if (rcShape.right > a_pChanged->right) rcShape.right = a_pChanged->right;
		if (rcShape.bottom > a_pChanged->bottom) rcShape.bottom = a_pChanged->bottom;
		return M_Window()->RectangleChanged(&rcShape);
	}
	void PolygonChanged()
	{
		if (M_PolyLine().size() < 2)
		{
			M_PolyLine().clear();
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
		}
		RECT const rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		ToolSetBrush();
	}

	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return a_eKeysState&ECKSControl ? ETPAMissed : HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		TransformBrush(a_pMatrix);
		return S_OK;
	}

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	EShapeFillMode m_eShapeFillMode;
	ECoordinatesMode m_eCoordinatesMode;
	float m_fGamma;
	bool m_bUseSecondary;

	bool m_bMoving;
	TPixelCoords m_tLastPos;

	//CEditToolDataCurve m_cData;
};
