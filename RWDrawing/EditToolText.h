
#pragma once

#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithCtrlDropper.h"
#include "EditToolWithBrush.h"
#include "EditToolPolyLine.h"
#include "SharedStateToolData.h"
#include "EditToolPolygon.h"
#include <math.h>
#include <boost/spirit.hpp>
using namespace boost::spirit;
#include "PolygonUtil.h"

#include "EditToolTextDlg.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
//#include <agg_conv_bspline.h>
#include <agg_conv_segmentator.h>
#include <agg_font_win32_tt.h>
//#include <agg_trans_single_path.h>
#include <agg_span_allocator.h>


HICON GetToolIconTEXT(ULONG a_nSize);

class CEditToolText :
	public CEditToolScanlineBuffer<CEditToolText>, // scanline image cache
	public CEditToolMouseInput<CEditToolText>, // no direct tablet support
	public CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>,
	public CEditToolOutline<CEditToolText>,
	public CEditToolCustomOrMoveCursor<CEditToolText, GetToolIconTEXT>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolText, CEditToolMouseInput<CEditToolText>, CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, CEditToolCustomOrMoveCursor<CEditToolText, GetToolIconTEXT> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolText, // T - the top level class for cross casting
		CEditToolText, // TResetHandler
		CEditToolText, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolText>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolOutline<CEditToolText>, // TOutlineHandler
		CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, // TBrushHandler
		CEditToolText, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolText, CEditToolMouseInput<CEditToolText>, CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, CEditToolCustomOrMoveCursor<CEditToolText, GetToolIconTEXT> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolText, CEditToolMouseInput<CEditToolText>, CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, CEditToolCustomOrMoveCursor<CEditToolText, GetToolIconTEXT> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolText, CEditToolMouseInput<CEditToolText>, CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, CEditToolCustomOrMoveCursor<CEditToolText, GetToolIconTEXT> >, // TProcessInputHandler
		CEditToolText, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolText, CEditToolText, CEditToolText> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolPolygon
{
public:
	CEditToolText() : m_eState(ESClean),
		m_eRasterizationMode(ERMSmooth), m_eBlendingMode(EBMDrawOver),
		m_fWidth(1.0f), m_eCoordinatesMode(ECMIntegral), m_fTextSizeX(0.0f), m_fTextSizeY(0.0f),
		m_fDeltaTop(0.0f), m_fDeltaBottom(0.0f), m_fLineSize(1.0f)
	{
		m_cData.dwTextID = 1;
	}

	BEGIN_COM_MAP(CEditToolText)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
	END_COM_MAP()

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
	}
	ECoordinatesMode M_CoordinatesMode() const
	{
		return m_eCoordinatesMode;
	}
	bool is_solid(unsigned a_nLayer)
	{
		return a_nLayer ? true : S_OK == M_Brush()->IsSolid(NULL);
	}
	TPixelChannel color(unsigned a_nLayer) const
	{
		if (a_nLayer)
		{
			TRasterImagePixel t = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			return CPixelChannel(t.bR, t.bG, t.bB, t.bA);
		}
		else
		{
			TRasterImagePixel tSolid;
			M_Brush()->GetBrushTile(0, 0, 1, 1, M_Gamma(), 1, &tSolid);
			return CPixelChannel(tSolid.bR, tSolid.bG, tSolid.bB, tSolid.bA);
		}
	}
    void generate_span(TPixelChannel* span, int x, int y, unsigned len, unsigned style)
    {
		M_Brush()->GetBrushTile(x, y, len, 1, M_Gamma(), len, reinterpret_cast<TRasterImagePixel*>(span));
    }
	void generate(TPixelChannel* span, int x, int y, unsigned len)
	{
		M_Brush()->GetBrushTile(x, y, len, 1, M_Gamma(), len, reinterpret_cast<TRasterImagePixel*>(span));
	}
	void prepare()
	{
	}
    class trans_circular
    {
    public:
		trans_circular() : m_r(1.0), m_x(0.0), m_y(0.0), m_phi(0.0), m_b(0.0)
		{
		}

        void radius(double r) { m_r = r; }
        bool radius() const { return m_r; }
        void center_x(double x) { m_x = x; }
        bool center_x() const { return m_x; }
        void center_y(double y) { m_y = y; }
        bool center_y() const { return m_y; }
        void base_y(double y) { m_b = y; }
        bool base_y() const { return m_b; }
        void angle(double phi) { m_phi = phi; }
        bool angle() const { return m_phi; }

        void transform(double *x, double *y) const
		{
			double const rx = *x-m_x;
			double const ry = *y-m_y;
			if (abs(ry) > 1e-6)
			{
				double const angle = rx/m_r-m_phi;//6.28318530718
				*x = m_x+fabsf(ry)*sin(angle)+fabsf(m_b-m_y)*sin(m_phi)+m_b*sin(0.0);
				*y = m_y+ry*cos(angle)-(m_b-m_y)*cos(m_phi)+(m_b-m_y)*cos(0.0);
			}
			else
			{
				*x = m_x;
				*y = m_y;
			}
		}

    private:
        double m_r;
		double m_x;
		double m_y;
		double m_phi;
		double m_b;
    };
	void PrepareText()
	{
		m_fTextSizeX = m_fTextSizeY = 0.0f;
		m_cLineLens.clear();
		m_cGeometry.free_all();
		m_fGeoX1 = m_fGeoY1 = 1e6;
		m_fGeoX2 = m_fGeoY2 = -1e6;
		m_fLineSize = 0.0f;
		m_fDeltaTop = m_fDeltaBottom = 0.0f;

		if (m_eState == ESClean)
			return;

		typedef agg::font_engine_win32_tt_int32 font_engine_type;
		typedef agg::font_cache_manager<font_engine_type> font_manager_type;
		typedef agg::conv_curve<font_manager_type::path_adaptor_type> conv_curve_type;
		//typedef agg::conv_contour<conv_curve_type> conv_contour_type;

		HDC hDC = GetDC(NULL);
		font_engine_type m_feng(hDC);
		font_manager_type m_fman(m_feng);
		conv_curve_type m_curves(m_fman.path_adaptor());
		//conv_contour_type m_contour;

		typedef agg::conv_segmentator<conv_curve_type> conv_font_segm_type;
		typedef agg::conv_transform<conv_font_segm_type, trans_circular> conv_font_trans_type;

		conv_font_segm_type  fsegm(m_curves);
		trans_circular circ;
		conv_font_trans_type ftrans(fsegm, circ);
		fsegm.approximation_scale(3.0);

		//m_feng.hinting(true);
		m_feng.flip_y(true);

		if (m_feng.create_font(COLE2T(m_cData.lfFaceName), agg::glyph_ren_outline, m_cData.fSize, 0, m_cData.bBold ? FW_BOLD : FW_NORMAL, m_cData.bItalic))
		{
			double x = 0.0;
			double y = 0.0;
			//double y0 = height() - m_height.value() - 10.0;
			CW2CT strAnsi(m_cData.strText.c_str());

			double fMaxLen = 0.0;

			const TCHAR* p = strAnsi;
			while (*p)
			{
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[-1] == (*p^(_T('\r')^_T('\n'))))
						++p; 
					m_cLineLens.push_back(x);
					if (x > fMaxLen)
						fMaxLen = x;
					x = 0.0;
					m_fman.reset_last_glyph();
					continue;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
				{
					m_fman.add_kerning(&x, &y);
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}
			m_cLineLens.push_back(x);
			if (x > fMaxLen)
				fMaxLen = x;
			x = 0.0;
			m_fTextSizeX = fMaxLen;
			TEXTMETRIC tTM;
			GetTextMetrics(hDC, &tTM);
			m_fLineSize = tTM.tmAscent+tTM.tmDescent;
			//m_fLineSize = (tTM.tmAscent+tTM.tmDescent)*-tTM.tmExternalLeading;
			m_fTextSizeY = (tTM.tmAscent+tTM.tmDescent)*(m_cLineLens.empty() ? 1 : m_cLineLens.size())-tTM.tmExternalLeading;
			float fExtraSpace = max(tTM.tmDescent, (tTM.tmInternalLeading+tTM.tmExternalLeading)*0.5f);
			m_fDeltaTop = tTM.tmInternalLeading-fExtraSpace;
			m_fDeltaBottom = fExtraSpace-tTM.tmDescent;

			agg::trans_affine mtx;
			if (fabsf(m_cData.fBend) < 1e-6f)
				mtx *= agg::trans_affine_rotation(agg::deg2rad(m_cData.fAngle));
			else
				circ.angle(m_cData.fBend < 0.0 ? -agg::deg2rad(m_cData.fAngle) : agg::deg2rad(m_cData.fAngle));
			m_feng.transform(mtx);
			m_fman.reset_cache();

			double xx = 1.0;
			double xy = 0.0;
			mtx.transform(&xx, &xy);
			double yx = 0.0;
			double yy = (tTM.tmAscent+tTM.tmDescent)/m_cData.fSize;
			mtx.transform(&yx, &yy);
			double ascx = 0.0;
			double ascy = tTM.tmAscent;
			mtx.transform(&ascx, &ascy);
			p = strAnsi;
			bool bLineSet = false;
			std::vector<double>::const_iterator iLine = m_cLineLens.begin();
			while (*p)
			{
				if (!bLineSet)
				{
					x = (iLine-m_cLineLens.begin())*yx*m_cData.fSize+ascx;
					y = (iLine-m_cLineLens.begin())*yy*m_cData.fSize+ascy;
					switch (m_cData.eAlign)
					{
					case CEditToolDataText::ETACenter:
						x -= *iLine*xx*0.5;
						y -= *iLine*xy*0.5;
						break;
					case CEditToolDataText::ETARight:
						x -= *iLine*xx;
						y -= *iLine*xy;
						break;
					//case CFGID_WMA_LEFT:
					}
					bLineSet = true;
					m_fman.reset_last_glyph();
					++iLine;
				}
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[-1] == (*p^(_T('\r')^_T('\n'))))
						++p;
					bLineSet = false;
					continue;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
				{
					m_fman.add_kerning(&x, &y);
					m_fman.init_embedded_adaptors(glyph, x+m_tLastPos.fX, y+m_tLastPos.fY);

					if(glyph->data_type == agg::glyph_data_outline)
					{
						if (fabsf(m_cData.fBend) < 1e-6f)
						{
							m_cGeometry.concat_path(m_curves);
						}
						else
						{
							double refX = m_tLastPos.fX;
							double refY = m_tLastPos.fY+(tTM.tmAscent+tTM.tmDescent)*0.5f;//+tTM.tmAscent;
							double lineR = (tTM.tmAscent+tTM.tmDescent)*(iLine-m_cLineLens.begin()-1);
							double baseR = (tTM.tmAscent+tTM.tmDescent)*4000.0f/powf(10.0f+fabsf(m_cData.fBend), 1.7f);
							double direction = m_cData.fBend < 0.0f ? -1.0 : 1.0;
							double r = baseR+lineR*direction;
							circ.radius(r);
							circ.center_x(refX);
							circ.center_y(refY-baseR*direction);
							circ.base_y(m_tLastPos.fY);
							m_cGeometry.concat_path(ftrans);
						}
					}

					// increment pen position
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}
		}

		ReleaseDC(NULL, hDC);

		agg::vertex_block_storage<double>& cVtx = m_cGeometry.vertices();
		unsigned int const n = cVtx.total_vertices();
		for (unsigned int i = 0; i < n; ++i)
		{
			double x, y;
			if (agg::path_flags_close != (agg::path_flags_mask&cVtx.vertex(i, &x, &y)))
			{
				if (x < m_fGeoX1) m_fGeoX1 = x;
				if (y < m_fGeoY1) m_fGeoY1 = y;
				if (x > m_fGeoX2) m_fGeoX2 = x;
				if (y > m_fGeoY2) m_fGeoY2 = y;
			}
		}
		m_cOriginal.clear();
		GetAsPolygon(m_cOriginal);
	}
	void GetAsPolygon(std::vector<std::vector<TPixelCoords> >& cOriginal)
	{
		cOriginal.resize(1);
		std::vector<std::vector<TPixelCoords> >::iterator cOriginal1 = cOriginal.begin();

		double x;
        double y;
        unsigned cmd;
        m_cGeometry.rewind(0);
		while(!agg::is_stop(cmd = m_cGeometry.vertex(&x, &y)))
        {
			TPixelCoords t = {x, y};
			if (agg::is_closed(cmd))
			{
				cOriginal.resize(cOriginal.size()+1);
				cOriginal1 = cOriginal.begin()+cOriginal.size()-1;
			}
			else
				cOriginal1->push_back(t);
        }
	}
	void PrepareShape()
	{
		if (m_eState == ESClean)
			return;

		std::vector<std::vector<TPixelCoords> > cOuter;
		SplitPolygons(m_cOriginal, cOuter);

		if (cOuter.empty())
			return;

		float const fWidthIn = M_OutlineIn();
		float const fWidthOut = M_OutlineOut();

		std::vector<std::vector<TPixelCoords> > cInner;
		if (fWidthIn != fWidthOut)
		{
			if (fWidthIn != 0.0f)
				ShrinkPolygon(cOuter, -fWidthIn, M_OutlineJoins(), cInner);
			else
				cInner = cOuter;
			if (fWidthOut != 0.0f)
			{
				std::vector<std::vector<TPixelCoords> > cExtra;
				ShrinkPolygon(cOuter, -fWidthOut, M_OutlineJoins(), cExtra);
				std::swap(cExtra, cOuter);
			}
		}

		agg::renderer_base<CEditToolScanlineBuffer> renb(*this);

		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		if (fWidthIn != fWidthOut && (M_Brush() == NULL || cInner.empty()))
		{
			TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
			{
				CEditToolPolyLine<CEditToolPolygon>::CVertexSource p(*i);
				ras.add_path(p);
			}
			for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
			{
				CEditToolPolyLine<CEditToolPolygon>::CVertexSourceHole p(*i);
				ras.add_path(p);
			}
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				agg::render_scanlines(ras, sl, ren);
			}
			else
			{
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);
			}
		}
		else if (fWidthIn == fWidthOut)
		{
			for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
			{
				CEditToolPolyLine<CEditToolPolygon>::CVertexSource p(*i);
				ras.add_path(p);
			}
			if (M_Brush() && S_OK != M_Brush()->IsSolid(NULL))
			{
				agg::span_allocator<TPixelChannel> span_alloc;
				if (m_eRasterizationMode == ERMSmooth)
				{
					agg::render_scanlines_aa(ras, sl, renb, span_alloc, *this);
				}
				else
				{
					ras.gamma(agg::gamma_threshold(0.5));
					agg::render_scanlines_bin(ras, sl, renb, span_alloc, *this);
				}
			}
			else
			{
				TRasterImagePixel tColor = {0, 0, 0, 0};
				if (M_Brush()) M_Brush()->GetBrushTile(0, 0, 1, 1, M_Gamma(), 1, &tColor);
				if (m_eRasterizationMode == ERMSmooth)
				{
					agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
					ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
					agg::render_scanlines(ras, sl, ren);
				}
				else
				{
					agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
					ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
					ras.gamma(agg::gamma_threshold(0.5));
					agg::render_scanlines(ras, sl, ren);
				}
			}
		}
		else
		{
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::scanline_u8 sl;

				agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;
				rasc.styles(1, -1);
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
					rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
					rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSourceHole(*i));
				rasc.styles(0, -1);
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
					rasc.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));

				agg::span_allocator<TPixelChannel> alloc;
				agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, *this, M_LayerBlender());
			}
			else
			{
				TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
				agg::scanline_p8 sl;
				agg::rasterizer_scanline_aa<> ras;
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
					ras.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
					ras.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSourceHole(*i));
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);

				ras.reset();
				for (std::vector<std::vector<TPixelCoords> >::iterator i = cInner.begin(); i != cInner.end(); ++i)
					ras.add_path(CEditToolPolyLine<CEditToolPolygon>::CVertexSource(*i));
				agg::span_allocator<TPixelChannel> span_alloc;
				ras.gamma(agg::gamma_threshold(0.502));
				agg::render_scanlines_bin(ras, sl, renb, span_alloc, *this);
			}
		}
		return;
		//agg::renderer_base<CEditToolScanlineBuffer> renb(*this);

		//agg::rasterizer_scanline_aa</*agg::rasterizer_sl_no_clip*/> ras;
		//agg::scanline_p8 sl;

		//agg::span_allocator<TPixelChannel> span_alloc;
		//ras.reset();
		//ras.add_path(m_cGeometry);
		//if (M_Brush() && S_OK != M_Brush()->IsSolid(NULL))
		//{
		//	agg::render_scanlines_aa(ras, sl, renb, span_alloc, *this);
		//}
		//else
		//{
		//	TRasterImagePixel tSolid;
		//	if (M_Brush())
		//		M_Brush()->GetBrushTile(0, 0, 1, 1, M_Gamma(), 1, &tSolid);
		//	else
		//	{
		//		tSolid.bR = tSolid.bG = tSolid.bB = 0;
		//		tSolid.bA = 255;
		//	}
		//	agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
		//	ren.color(CPixelChannel(tSolid.bR, tSolid.bG, tSolid.bB, tSolid.bA));
		//	agg::render_scanlines(ras, sl, ren);
		//}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		RECT const rc = M_DirtyRect();
		m_eState = ESClean;
		m_fTextSizeX = 0.0f;
		m_fTextSizeY = 0.0f;
		m_fDeltaTop = 0.0f;
		m_fDeltaBottom = 0.0f;
		m_cLineLens.clear();
		m_cGeometry.free_all();
		m_fGeoX1 = m_fGeoY1 = 1e6;
		m_fGeoX2 = m_fGeoY2 = -1e6;
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		ResetDragging();
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		//M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		EnableBrush(false);
		return S_OK;
	}
	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (m_fGeoX1 >= m_fGeoX2 || m_fGeoY1 >= m_fGeoY2)
		{
			if (a_pImageRect)
				*a_pImageRect = M_DirtyRect();
			return S_FALSE;
		}
		if (a_pImageRect)
		{
			float const fWidthOut = M_OutlineOut();
			a_pImageRect->left = floor(m_fGeoX1-fWidthOut);
			a_pImageRect->top = floor(m_fGeoY1-fWidthOut);
			a_pImageRect->right = ceil(m_fGeoX2+fWidthOut);
			a_pImageRect->bottom = ceil(m_fGeoY2+fWidthOut);
		}
		return S_OK;
		//HRESULT hRes = CEditToolScanlineBuffer<CEditToolText>::_IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect);
		//if (a_pImageRect && m_eState != ESClean && (m_fTextSizeX > 0.0f || m_fTextSizeY > 0.0f))
		//{
		//	RECT const rc = {m_tLastPos.fX, m_tLastPos.fY+m_fDeltaTop, m_tLastPos.fX+m_fTextSizeX, m_tLastPos.fY+m_fTextSizeY+m_fDeltaBottom};
		//	if (a_pImageRect->left > rc.left) a_pImageRect->left = rc.left;
		//	if (a_pImageRect->top > rc.top) a_pImageRect->top = rc.top;
		//	if (a_pImageRect->right < rc.right) a_pImageRect->right = rc.right;
		//	if (a_pImageRect->bottom < rc.bottom) a_pImageRect->bottom = rc.bottom;
		//}
		//return hRes;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataText::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			if (m_cData.dwTextID == pData->InternalData()->dwTextID)
			{
				m_cData = *(pData->InternalData());
			}
			else
			{
				std::wstring str;
				std::swap(str, m_cData.strText);
				DWORD dw = m_cData.dwTextID;
				m_cData = *(pData->InternalData());
				std::swap(str, m_cData.strText);
				m_cData.dwTextID = dw;
			}
			if (m_cData.fSize < 1.0f)
				m_cData.fSize = 1.0f;
			if (m_eState == ESClean && !m_cData.strText.empty())
				m_eState = ESFloating;
			if (m_eState != ESClean)
			{
				PrepareText();
				ToolSetBrush();
				RECT const rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointsChanged();
			}
		}
		return S_OK;
	}
	void ColorChanged(bool a_bColor1, bool a_bColor2)
	{
		if (m_eState != ESClean)
		{
			InvalidateCachePixels();
			M_Window()->RectangleChanged(&M_DirtyRect());
		}
	}
	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		if (m_eState == ESClean)
			return S_FALSE;
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
	void ToolSetBrush()
	{
		if (M_Brush() && m_eState != ESClean)
		{
			if (m_fGeoX1 < m_fGeoX2 && m_fGeoY1 < m_fGeoY2)
			{
				TPixelCoords const tCenter = {(m_fGeoX1+m_fGeoX2)*0.5, (m_fGeoY1+m_fGeoY2)*0.5};
				M_Brush()->SetShapeBounds(&tCenter, (m_fGeoX2-m_fGeoX1)*0.5, (m_fGeoY2-m_fGeoY1)*0.5, 0.0f);
			}
		}
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
		if (m_eState == ESClean)
			return S_OK;
		RECT rcPrev = M_DirtyRect();
		bool bRedrawCache = bRasterizationChange;
		if (bCoordinatesChange && m_eCoordinatesMode != ECMFloatingPoint)
		{
			_AdjustCoordinates(ECKSNone, &m_tLastPos, NULL, NULL, 0.0f);
			M_Window()->ControlLinesChanged();
			bRedrawCache = true;
		}
		if (bRedrawCache)
		{
			PrepareText();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
		}
		else if (bBlendingChange)
		{
			M_Window()->RectangleChanged(&rcPrev);
		}
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* UNREF(a_pPointerSize), ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		if (m_eCoordinatesMode == ECMIntegral)
		{
			a_pPos->fX = floorf(a_pPos->fX+0.5f);
			a_pPos->fY = floorf(a_pPos->fY+0.5f);
		}

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState == ESFloating && HitTestPoint(a_pPos))
		{
			m_eState = ESDragging;
			m_tDelta.fX = a_pPos->fX-m_tLastPos.fX;
			m_tDelta.fY = a_pPos->fY-m_tLastPos.fY;
			M_Window()->ControlPointsChanged();
			return S_OK;
		}

		if (!m_cData.strText.empty())
		{
			if (m_eState != ESClean)
			{
				ATLASSERT(0);
				return ETPAApply|ETPAStartNew;
			}
		}

		m_cData.strText.clear();
		m_cData.dwTextID = 1|((GetTickCount()>>3)&0xffffe)|((DWORD(this)<<20)&0xfff00000);

		m_eState = ESDragging;
		m_tLastPos = *a_pPos;
		m_tDelta.fX = m_tDelta.fY = 0.0f;
		PrepareText();
		ToolSetBrush();
		RECT const rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
//		M_Window()->RectangleChanged(&DirtyRect());
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();

		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_eState = ESFloating;
		PrepareText();
		ToolSetBrush();
		EnableBrush(true);
		M_Window()->ControlPointsChanged();

		{
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			M_Window()->SetState(pTmp);
		}

		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState != ESDragging)
			return S_FALSE;
		if (a_pPos->fX == m_tLastPos.fX && a_pPos->fY == m_tLastPos.fY)
			return S_FALSE;
		m_tLastPos.fX = a_pPos->fX-m_tDelta.fX;
		m_tLastPos.fY = a_pPos->fY-m_tDelta.fY;
		PrepareText();
		ToolSetBrush();
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_eState == ESFloating && HitTestPoint(a_pPos);
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_eState != ESClean ? 4 : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (m_eState != ESClean)
		{
			if (a_nIndex == 0)
			{
				*a_pPos = m_tLastPos;
				*a_pClass = 0;
				return S_OK;
			}
			if (a_nIndex == 1)
			{
				a_pPos->fX = m_tLastPos.fX-sinf(agg::deg2rad(m_cData.fAngle))*m_cData.fSize;
				a_pPos->fY = m_tLastPos.fY+cosf(agg::deg2rad(m_cData.fAngle))*m_cData.fSize;
				*a_pClass = 1;
				return S_OK;
			}
			if (a_nIndex == 2)
			{
				a_pPos->fX = m_tLastPos.fX-sinf(agg::deg2rad(m_cData.fAngle))*2.0f*m_cData.fSize;
				a_pPos->fY = m_tLastPos.fY+cosf(agg::deg2rad(m_cData.fAngle))*2.0f*m_cData.fSize;
				*a_pClass = 2;
				return S_OK;
			}
			if (a_nIndex == 3)
			{
				float const fCos = cosf(agg::deg2rad(m_cData.fAngle));
				float const fSin = sinf(agg::deg2rad(m_cData.fAngle));
				if (fabsf(m_cData.fBend) < 1e-3f)
				{
					a_pPos->fX = m_tLastPos.fX+m_fLineSize*2.0f*fCos;
					a_pPos->fY = m_tLastPos.fY+m_fLineSize*2.0f*fSin;
				}
				else
				{
					double baseR = m_fLineSize*4000.0f/powf(10.0f+fabsf(m_cData.fBend), 1.7f);
					double direction = m_cData.fBend < 0.0f ? -1.0 : 1.0;
					double r = baseR;
					double centerY = 0.5f*m_fLineSize-baseR*direction;
					double rx = m_fLineSize*2.0f;
					double ry = -centerY;
					double angle = 0;
					if (abs(r) > 1e-6)
						angle = rx/r;
					a_pPos->fX = m_tLastPos.fX+(fabsf(ry)*sin(angle))*fCos-(centerY+ry*cos(angle))*fSin;
					a_pPos->fY = m_tLastPos.fY+(fabsf(ry)*sin(angle))*fSin+(centerY+ry*cos(angle))*fCos;
				}
				*a_pClass = 9;
				return S_OK;
			}
		}
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (m_eState != ESClean)
		{
			if (a_nIndex == 0)
			{
				m_tLastPos = *a_pPos;
				PrepareText();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointChanged(0);
				M_Window()->ControlPointChanged(1);
				M_Window()->ControlPointChanged(2);
				M_Window()->ControlPointChanged(3);
				return S_OK;
			}
			if (a_nIndex == 1)
			{
				float f = sqrtf((a_pPos->fX-m_tLastPos.fX)*(a_pPos->fX-m_tLastPos.fX)+(a_pPos->fY-m_tLastPos.fY)*(a_pPos->fY-m_tLastPos.fY));
				m_cData.fSize = LONG(f/**100.0f*/+0.5f)/**0.01f*/;
				if (m_cData.fSize < 1.0f)
					m_cData.fSize = 1.0f;
				PrepareText();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointChanged(1);
				M_Window()->ControlPointChanged(2);
				M_Window()->ControlPointChanged(3);
				if (a_bFinished)
				{
					CComObject<CSharedStateToolData>* pNew = NULL;
					CComObject<CSharedStateToolData>::CreateInstance(&pNew);
					CComPtr<ISharedState> pTmp = pNew;
					pNew->Init(m_cData);
					M_Window()->SetState(pTmp);
				}
				return S_OK;
			}
			if (a_nIndex == 2)
			{
				float f = atan2f(m_tLastPos.fX-a_pPos->fX, a_pPos->fY-m_tLastPos.fY);
				m_cData.fAngle = agg::rad2deg(f);
				PrepareText();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointChanged(1);
				M_Window()->ControlPointChanged(2);
				M_Window()->ControlPointChanged(3);
				if (a_bFinished)
				{
					CComObject<CSharedStateToolData>* pNew = NULL;
					CComObject<CSharedStateToolData>::CreateInstance(&pNew);
					CComPtr<ISharedState> pTmp = pNew;
					pNew->Init(m_cData);
					M_Window()->SetState(pTmp);
				}
				return S_OK;
			}
			if (a_nIndex == 3)
			{
				TPixelCoords const tPos = {a_pPos->fX-m_tLastPos.fX, a_pPos->fY-m_tLastPos.fY};
				float fAngle = agg::deg2rad(m_cData.fAngle);
				TPixelCoords tRot =
				{
					tPos.fX*cosf(fAngle) + tPos.fY*sinf(fAngle),
					-tPos.fX*sinf(fAngle) + tPos.fY*cosf(fAngle)
				};
				float fBend = 0.0f;
				if (tRot.fX < 0.0f)
					tRot.fX = 0.0f;
				if (fabs(tRot.fY) > 0.1f)
				{
					if (tRot.fY < 0.0f)
					{
						double angle = 2.0f*atan2(-tRot.fY, tRot.fX);
						double r = m_fLineSize*2.0f/angle;
						fBend = 1.0f*(powf(m_fLineSize*4000.0f/r, 1.0/1.7)-10.0f);
					}
					else
					{
						double angle = 2.0f*atan2(tRot.fY, tRot.fX);
						double r = m_fLineSize*2.0f/angle;
						fBend = -1.0f*(powf(m_fLineSize*4000.0f/r, 1.0/1.7)-10.0f);
					}
				}
				if (fabs(m_cData.fBend-fBend) > 1e-6f)
				{
					m_cData.fBend = fBend;
					PrepareText();
					ToolSetBrush();
					RECT rc = UpdateCache();
					M_Window()->RectangleChanged(&rc);
					M_Window()->ControlLinesChanged();
					M_Window()->ControlPointChanged(3);
					if (a_bFinished)
					{
						CComObject<CSharedStateToolData>* pNew = NULL;
						CComObject<CSharedStateToolData>::CreateInstance(&pNew);
						CComPtr<ISharedState> pTmp = pNew;
						pNew->Init(m_cData);
						M_Window()->SetState(pTmp);
					}
				}
				return S_OK;
			}
		}
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_eState != ESClean && !m_cLineLens.empty())
		{
			float const fCos = cosf(agg::deg2rad(m_cData.fAngle));
			float const fSin = sinf(agg::deg2rad(m_cData.fAngle));
			float fXMin = 0.0f;
			float fXMax = 1.0f;
			if (m_cData.eAlign == CEditToolDataText::ETACenter)
			{
				fXMin = -0.5f;
				fXMax = 0.5;
			}
			else if (m_cData.eAlign == CEditToolDataText::ETARight)
			{
				fXMin = -1.0f;
				fXMax = 0.0f;
			}
			if (fabsf(m_cData.fBend) < 1e-6)
			{
				a_pLines->MoveTo(m_tLastPos.fX, m_tLastPos.fY);
				int j = 0;
				for (std::vector<double>::const_iterator i = m_cLineLens.begin(); i != m_cLineLens.end(); ++i, ++j)
				{
					a_pLines->LineTo(m_tLastPos.fX+fXMax**i*fCos-m_fLineSize*j*fSin, m_tLastPos.fY+fXMax**i*fSin+m_fLineSize*j*fCos);
					a_pLines->LineTo(m_tLastPos.fX+fXMax**i*fCos-m_fLineSize*(j+1)*fSin, m_tLastPos.fY+fXMax**i*fSin+m_fLineSize*(j+1)*fCos);
				}
				for (std::vector<double>::const_reverse_iterator i = m_cLineLens.rbegin(); i != m_cLineLens.rend(); ++i, --j)
				{
					a_pLines->LineTo(m_tLastPos.fX+fXMin**i*fCos-m_fLineSize*j*fSin, m_tLastPos.fY+fXMin**i*fSin+m_fLineSize*j*fCos);
					a_pLines->LineTo(m_tLastPos.fX+fXMin**i*fCos-m_fLineSize*(j-1)*fSin, m_tLastPos.fY+fXMin**i*fSin+m_fLineSize*(j-1)*fCos);
				}
				a_pLines->Close();
			}
			else
			{
				double baseR = m_fLineSize*4000.0f/powf(10.0f+fabsf(m_cData.fBend), 1.7f);
				double direction = m_cData.fBend < 0.0f ? -1.0 : 1.0;
				int j = 0;
				double prev = m_cLineLens[0]*fXMin/baseR;
				{
					double centerY = 0.5f*m_fLineSize-baseR*direction;
					double rx = m_cLineLens[0];
					double ry = -centerY;
					double angle = 0;
					if (abs(baseR) > 1e-6)
						angle = rx/baseR;
					a_pLines->MoveTo(
						m_tLastPos.fX+(fabsf(ry)*sin(fXMin*angle))*fCos-(centerY+ry*cos(fXMin*angle))*fSin,
						m_tLastPos.fY+(fabsf(ry)*sin(fXMin*angle))*fSin+(centerY+ry*cos(fXMin*angle))*fCos);
				}
				for (std::vector<double>::const_iterator i = m_cLineLens.begin(); i != m_cLineLens.end(); ++i, ++j)
				{
					double lineR = m_fLineSize*j;
					double r = baseR+lineR*direction;
					double centerY = 0.5f*m_fLineSize-baseR*direction;
					double rx = *i;
					double ry = j*m_fLineSize-centerY;
					double angle = 0;
					double a2 = 0;
					if (abs(r) > 1e-6)
					{
						angle = fXMax*rx/r;
						a2 = prev;
					}
					int steps = ceil(fabs((a2-angle)*10.0));
					if (steps < 1) steps = 1;
					for (int k = 1; k <= steps; ++k)
					{
						double a = (a2*(steps-k)+angle*k)/steps;
						a_pLines->LineTo(
							m_tLastPos.fX+(fabsf(ry)*sin(a))*fCos-(centerY+ry*cos(a))*fSin,
							m_tLastPos.fY+(fabsf(ry)*sin(a))*fSin+(centerY+ry*cos(a))*fCos);
					}
					ry = (j+1)*m_fLineSize-centerY;
					a_pLines->LineTo(
						m_tLastPos.fX+(fabsf(ry)*sin(angle))*fCos-(centerY+ry*cos(angle))*fSin,
						m_tLastPos.fY+(fabsf(ry)*sin(angle))*fSin+(centerY+ry*cos(angle))*fCos);
					prev = *i*fXMax/r;
				}
				for (std::vector<double>::const_reverse_iterator i = m_cLineLens.rbegin(); i != m_cLineLens.rend(); ++i, --j)
				{
					double lineR = m_fLineSize*(j-1);
					double r = baseR+lineR*direction;
					double centerY = 0.5f*m_fLineSize-baseR*direction;
					double rx = *i;
					double ry = j*m_fLineSize-centerY;
					double angle = 0;
					double a2 = 0;
					if (abs(r) > 1e-6)
					{
						angle = fXMin*rx/r;
						a2 = prev;
					}
					int steps = ceil(fabs((a2-angle)*10.0));
					if (steps < 1) steps = 1;
					for (int k = 1; k <= steps; ++k)
					{
						double a = (a2*(steps-k)+angle*k)/steps;
						a_pLines->LineTo(
							m_tLastPos.fX+(fabsf(ry)*sin(a))*fCos-(centerY+ry*cos(a))*fSin,
							m_tLastPos.fY+(fabsf(ry)*sin(a))*fSin+(centerY+ry*cos(a))*fCos);
					}
					ry = (j-1)*m_fLineSize-centerY;
					a_pLines->LineTo(
						m_tLastPos.fX+(fabsf(ry)*sin(angle))*fCos-(centerY+ry*cos(angle))*fSin,
						m_tLastPos.fY+(fabsf(ry)*sin(angle))*fSin+(centerY+ry*cos(angle))*fCos);
					prev = *i*fXMin/r;
				}
				//a_pLines->Close();
			}
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}
	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (a_pMsg->message == WM_CHAR)
		{
			if (a_pMsg->wParam >= L' ')
			{
				m_cData.strText.push_back(wchar_t(a_pMsg->wParam));
			}
			else if (a_pMsg->wParam == L'\r')
			{
				m_cData.strText.push_back(L'\r');
				m_cData.strText.push_back(L'\n');
			}
			else if (a_pMsg->wParam == L'\b' && m_cData.strText.size() > 0)
			{
				if (m_cData.strText.size() > 1 && m_cData.strText[m_cData.strText.size()-1] == L'\n' && m_cData.strText[m_cData.strText.size()-2] == L'\r')
					m_cData.strText.resize(m_cData.strText.size()-2);
				else
					m_cData.strText.resize(m_cData.strText.size()-1);
			}
			else
			{
				return S_FALSE;
			}
			PrepareText();
			ToolSetBrush();
			RECT rcNew = UpdateCache();
			M_Window()->RectangleChanged(&rcNew);
			M_Window()->ControlLinesChanged();
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			M_Window()->SetState(pTmp);
			return S_OK;
		}
		else if ((a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_KEYUP) && a_pMsg->hwnd)
		{
			if (a_pMsg->wParam == VK_RETURN)
			{
				RWHWND h = NULL;
				if (M_Window()) M_Window()->Handle(&h);
				if (a_pMsg->hwnd == h)
				{
					TranslateMessage(a_pMsg);
					//DispatchMessage(a_pMsg);
					return S_OK;
				}
			}
			else if (m_eState != ESClean && a_pMsg->message == WM_KEYDOWN &&
					 (a_pMsg->wParam == VK_LEFT || a_pMsg->wParam == VK_RIGHT ||
					  a_pMsg->wParam == VK_UP || a_pMsg->wParam == VK_DOWN))
			{
				m_tLastPos.fX += a_pMsg->wParam == VK_LEFT ? -1 : (a_pMsg->wParam == VK_RIGHT ? 1 : 0);
				m_tLastPos.fY += a_pMsg->wParam == VK_UP ? -1 : (a_pMsg->wParam == VK_DOWN ? 1 : 0);
				PrepareText();
				ToolSetBrush();
				RECT rcNew = UpdateCache();
				M_Window()->RectangleChanged(&rcNew);
				M_Window()->ControlLinesChanged();
				M_Window()->ControlPointsChanged();
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
				return S_OK;
			}
			else if (m_eState != ESClean)
			{
				if ((GetKeyState(VK_CONTROL)&0x8000) == 0x8000 || (GetKeyState(VK_MENU)&0x8000) == 0x8000)
					return S_FALSE;
				TranslateMessage(a_pMsg);
				DispatchMessage(a_pMsg);
				return S_OK;
			}
		}
		return S_FALSE;
	}
	bool HitTestPoint(TPixelCoords const* a_pPos) const
	{
		if (m_eState == ESClean || m_cLineLens.empty())
			return false;

		TPixelCoords const tPos = {a_pPos->fX-m_tLastPos.fX, a_pPos->fY-m_tLastPos.fY};
		float fAngle = agg::deg2rad(m_cData.fAngle);
		TPixelCoords tRot =
		{
			tPos.fX*cosf(fAngle) + tPos.fY*sinf(fAngle),
			-tPos.fX*sinf(fAngle) + tPos.fY*cosf(fAngle)
		};

		float const fCos = cosf(agg::deg2rad(m_cData.fAngle));
		float const fSin = sinf(agg::deg2rad(m_cData.fAngle));
		float fXMin = 0.0f;
		float fXMax = 1.0f;
		if (m_cData.eAlign == CEditToolDataText::ETACenter)
		{
			fXMin = -0.5f;
			fXMax = 0.5;
		}
		else if (m_cData.eAlign == CEditToolDataText::ETARight)
		{
			fXMin = -1.0f;
			fXMax = 0.0f;
		}
		if (fabsf(m_cData.fBend) < 1e-6)
		{
			int j = 0;
			for (std::vector<double>::const_iterator i = m_cLineLens.begin(); i != m_cLineLens.end(); ++i, ++j)
				if (tRot.fX >= fXMin**i && tRot.fY >= m_fLineSize*j && tRot.fX < fXMax**i && tRot.fY < m_fLineSize*(j+1))
					return true;
			return false;
		}
		else
		{
			double baseR = m_fLineSize*4000.0f/powf(10.0f+fabsf(m_cData.fBend), 1.7f);
			double direction = m_cData.fBend < 0.0f ? -1.0 : 1.0;
			int j = 0;
			for (std::vector<double>::const_iterator i = m_cLineLens.begin(); i != m_cLineLens.end(); ++i, ++j)
			{
				double lineR = m_fLineSize*j;
				double r = baseR+lineR*direction;
				double centerY = 0.5f*m_fLineSize-baseR*direction;
				float fRotY = tRot.fY - centerY;
				double pointA = atan2(tRot.fX, fRotY*float(direction));
				double pointR = sqrt(tRot.fX*tRot.fX + fRotY*fRotY);
				double rx = *i;
				double ry = j*m_fLineSize-centerY;
				double angle = 0;
				if (abs(r) > 1e-6)
					angle = rx/r;
				if (pointR >= (fabs(r-0.5*m_fLineSize)) && pointR < (fabs(r+0.5*m_fLineSize)) && (
					(pointA >= fXMin*angle && pointA < fXMax*angle) ||
					((pointA-6.28318530718) >= fXMin*angle && (pointA-6.28318530718) < fXMax*angle) ||
					((pointA+6.28318530718) >= fXMin*angle && (pointA+6.28318530718) < fXMax*angle)))
					return true;
			}
			return false;
		}
	}

	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return a_eKeysState&ECKSControl ? ETPAMissed : HitTestPoint(a_pPos) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
	}
	TPixelCoords TransformPixelCoords(TMatrix3x3f const& a_tMatrix, TPixelCoords const& a_tVector)
	{
		float const fW = 1.0f/(a_tMatrix._13*a_tVector.fX + a_tMatrix._23*a_tVector.fY + a_tMatrix._33);
		TPixelCoords const t =
		{
			fW*(a_tMatrix._11*a_tVector.fX + a_tMatrix._21*a_tVector.fY + a_tMatrix._31),
			fW*(a_tMatrix._12*a_tVector.fX + a_tMatrix._22*a_tVector.fY + a_tMatrix._32),
		};
		return t;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		float fScale = sqrtf(sqrtf(a_pMatrix->_11*a_pMatrix->_11+a_pMatrix->_12*a_pMatrix->_12)*sqrtf(a_pMatrix->_21*a_pMatrix->_21+a_pMatrix->_22*a_pMatrix->_22));
		m_cData.fSize *= fScale;
		m_tLastPos = TransformPixelCoords(*a_pMatrix, m_tLastPos);
		if (m_cData.fSize < 1.0f)
			m_cData.fSize = 1.0f;
		PrepareText();
		TransformBrush(a_pMatrix);
		ToolSetBrush();
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointChanged(0);
		M_Window()->ControlPointChanged(1);
		M_Window()->ControlPointChanged(2);
		M_Window()->ControlPointChanged(3);
		CComObject<CSharedStateToolData>* pNew = NULL;
		CComObject<CSharedStateToolData>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		pNew->Init(m_cData);
		M_Window()->SetState(pTmp);
		return S_OK;
	}

private:
	enum EState
	{
		ESClean = 0,
		ESDragging,
		ESFloating
	};

	// IRasterImageEditToolScripting
public:
	void Unescape(std::wstring& a_str)
	{
		size_t nLen = a_str.length();
		size_t iD = 0;
		size_t iS = 0;
		while (iS < nLen)
		{
			if (a_str[iS] == L'\\')
				++iS;
			if (iS == nLen)
				break;
			a_str[iD] = a_str[iS];
			++iS;
			++iD;
		}
		if (iD != iS)
			a_str.resize(iD);
	}
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			std::wstring strFont;
			bool bBold = false;
			bool bItalic = false;
			bool bDeg = false;
			float fSize = 0.0f;
			float fAngle = 0.0f;
			float fBend = 0.0f;
			CEditToolDataText::ETextAlign eAlign = CEditToolDataText::ETALeft;
			float fPosX = 0.0f;
			float fPosY = 0.0f;
			std::wstring strText;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					confix_p(L'"', (*c_escape_ch_p)[assign_a(strFont)], L'"')>>cSep>>
					//(*negated_char_parser<chlit<wchar_t> >(L',')/*~ch_p(L'\,')*/)[assign_a(strFont)]>>cSep>>
					(!((str_p(L"\"BOLD\"")[assign_a(bBold, true)]|str_p(L"\"ITALIC\"")[assign_a(bItalic, true)]|str_p(L"\"BOLDITALIC\"")[assign_a(bBold, true)][assign_a(bItalic, true)]|str_p(L"\"NORMAL\"")) >> cSep))>>
					real_p[assign_a(fSize)]>>cSep>>
					real_p[assign_a(fPosX)]>>cSep>>
					real_p[assign_a(fPosY)]>>cSep>>
					(!(real_p[assign_a(fAngle)]>>cSep>>(!(real_p[assign_a(fBend)]>>cSep))))>>
					confix_p(L'"', (*c_escape_ch_p)[assign_a(strText)], L'"')>>
					(!(cSep >> (str_p(L"\"LEFT\"")[assign_a(eAlign, CEditToolDataText::ETALeft)]|str_p(L"\"CENTER\"")[assign_a(eAlign, CEditToolDataText::ETACenter)]|str_p(L"\"RIGHT\"")[assign_a(eAlign, CEditToolDataText::ETARight)]|str_p(L"\"JUSTIFY\"")[assign_a(eAlign, CEditToolDataText::ETAJustify)])))
					>>*space_p).full;
			Unescape(strFont);
			Unescape(strText);
			if (!bParsed || strFont.empty() || strFont.length() >= LF_FACESIZE || fSize <= 0.0f)
				return E_INVALIDARG;
			bool bChange = m_cData.bBold != bBold || m_cData.bItalic != bItalic || m_cData.fSize != fSize ||
				m_cData.fAngle != fAngle || m_cData.fBend != fBend || wcscmp(m_cData.lfFaceName, strFont.c_str()) ||
				m_cData.eAlign != eAlign || wcscmp(m_cData.strText.c_str(), strText.c_str());
			m_cData.bBold = bBold;
			m_cData.bItalic = bItalic;
			m_cData.fSize = fSize;
			m_cData.fAngle = fAngle;
			m_cData.fBend = fBend;
			wcscpy(m_cData.lfFaceName, strFont.c_str());
			m_cData.eAlign = eAlign;
			m_cData.strText = strText;
			m_cData.dwTextID = 1|((GetTickCount()>>3)&0xffffe)|((DWORD(this)<<20)&0xfff00000);
			m_eState = ESFloating;
			m_tLastPos.fX = fPosX;
			m_tLastPos.fY = fPosY;
			m_tDelta.fX = m_tDelta.fY = 0.0f;
			PrepareText();
			EnableBrush(true);
			ToolSetBrush();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
			if (bChange)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
			}
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
			if (m_eState == ESClean)
				return S_FALSE;
			CComBSTR bstr(L"\"");
			bstr += m_cData.lfFaceName;
			bstr += L"\"";
			if (m_cData.bBold || m_cData.bItalic)
				bstr += m_cData.bBold ? (m_cData.bItalic ? L", \"BOLDITALIC\"" : L", \"BOLD\"") : L", \"ITALIC\"";
			OLECHAR sz[64];
			swprintf(sz, L", %g, %g, %g", m_cData.fSize, m_tLastPos.fX, m_tLastPos.fY);
			bstr += sz;
			if (m_cData.fAngle != 0.0f || fabsf(m_cData.fBend) >= 1e-6f)
			{
				swprintf(sz, L", %g", m_cData.fAngle);
				bstr += sz;
			}
			if (fabsf(m_cData.fBend) >= 1e-6f)
			{
				swprintf(sz, L", %g", m_cData.fBend);
				bstr += sz;
			}
			bstr += L", \"";
			ULONG nExtra = 0;
			if (m_cData.strText.c_str()) for (LPCWSTR psz = m_cData.strText.c_str(); *psz; ++psz)
				if (*psz == L'\"' || *psz == L'\\')
					++nExtra;
			if (nExtra)
			{
				CAutoVectorPtr<OLECHAR> sz(new OLECHAR[nExtra+m_cData.strText.length()+1]);
				LPOLESTR pszD = sz;
				for (LPCWSTR psz = m_cData.strText.c_str(); *psz; ++psz, ++pszD)
				{
					if (*psz == L'\"' || *psz == L'\\')
						*(pszD++) = L'\\';
					*pszD = *psz;
				}
				*pszD = L'\0';
				bstr += sz.m_p;
			}
			else
			{
				bstr += m_cData.strText.c_str();
			}
			bstr += L"\"";
			if (m_cData.eAlign != CEditToolDataText::ETALeft)
				bstr += m_cData.eAlign == CEditToolDataText::ETACenter ? L", \"CENTER\"" : (m_cData.eAlign == CEditToolDataText::ETARight ? L", \"RIGHT\"" : L", \"JUSTIFY\"");
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IRasterImageEditToolPolygon methods
public:
	STDMETHOD(FromPolygon)(ULONG a_nCount, TRWPolygon const* a_pPolygons)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;

		std::vector<TRWPathPoint> cPath;
		std::vector<TRWPath> cPaths;


		float fTextSizeX = 0.0f;
		float fTextSizeY = 0.0f;
		std::vector<double> cLineLens;
		float fLineSize = 0.0f;
		float fDeltaTop = 0.0f;
		float fDeltaBottom = 0.0f;

		if (m_eState == ESClean)
			return E_FAIL;
		if (m_cData.fBend != 0.0)
			return E_FAIL;

		typedef agg::font_engine_win32_tt_int32 font_engine_type;
		typedef agg::font_cache_manager<font_engine_type> font_manager_type;

		HDC hDC = GetDC(NULL);
		font_engine_type m_feng(hDC);
		font_manager_type m_fman(m_feng);
		font_manager_type::path_adaptor_type& m_curves(m_fman.path_adaptor());

		//m_feng.hinting(true);
		m_feng.flip_y(true);

		if (m_feng.create_font(COLE2T(m_cData.lfFaceName), agg::glyph_ren_outline, m_cData.fSize, 0, m_cData.bBold ? FW_BOLD : FW_NORMAL, m_cData.bItalic))
		{
			double x = 0.0;
			double y = 0.0;
			//double y0 = height() - m_height.value() - 10.0;
			CW2CT strAnsi(m_cData.strText.c_str());

			double fMaxLen = 0.0;

			const TCHAR* p = strAnsi;
			while (*p)
			{
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[-1] == (*p^(_T('\r')^_T('\n'))))
						++p; 
					cLineLens.push_back(x);
					if (x > fMaxLen)
						fMaxLen = x;
					x = 0.0;
					m_fman.reset_last_glyph();
					continue;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
				{
					m_fman.add_kerning(&x, &y);
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}
			cLineLens.push_back(x);
			if (x > fMaxLen)
				fMaxLen = x;
			x = 0.0;
			fTextSizeX = fMaxLen;
			TEXTMETRIC tTM;
			GetTextMetrics(hDC, &tTM);
			fLineSize = tTM.tmAscent+tTM.tmDescent;
			//m_fLineSize = (tTM.tmAscent+tTM.tmDescent)*-tTM.tmExternalLeading;
			fTextSizeY = (tTM.tmAscent+tTM.tmDescent)*(m_cLineLens.empty() ? 1 : m_cLineLens.size())-tTM.tmExternalLeading;
			float fExtraSpace = max(tTM.tmDescent, (tTM.tmInternalLeading+tTM.tmExternalLeading)*0.5f);
			fDeltaTop = tTM.tmInternalLeading-fExtraSpace;
			fDeltaBottom = fExtraSpace-tTM.tmDescent;

			agg::trans_affine mtx;
			mtx *= agg::trans_affine_rotation(agg::deg2rad(m_cData.fAngle));
			m_feng.transform(mtx);
			m_fman.reset_cache();

			double xx = 1.0;
			double xy = 0.0;
			mtx.transform(&xx, &xy);
			double yx = 0.0;
			double yy = (tTM.tmAscent+tTM.tmDescent)/m_cData.fSize;
			mtx.transform(&yx, &yy);
			double ascx = 0.0;
			double ascy = tTM.tmAscent;
			mtx.transform(&ascx, &ascy);
			p = strAnsi;
			bool bLineSet = false;
			std::vector<double>::const_iterator iLine = cLineLens.begin();
			while (*p)
			{
				if (!bLineSet)
				{
					x = (iLine-cLineLens.begin())*yx*m_cData.fSize+ascx;
					y = (iLine-cLineLens.begin())*yy*m_cData.fSize+ascy;
					switch (m_cData.eAlign)
					{
					case CEditToolDataText::ETACenter:
						x -= *iLine*xx*0.5;
						y -= *iLine*xy*0.5;
						break;
					case CEditToolDataText::ETARight:
						x -= *iLine*xx;
						y -= *iLine*xy;
						break;
					//case CFGID_WMA_LEFT:
					}
					bLineSet = true;
					m_fman.reset_last_glyph();
					++iLine;
				}
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[-1] == (*p^(_T('\r')^_T('\n'))))
						++p;
					bLineSet = false;
					continue;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
				{
					m_fman.add_kerning(&x, &y);
					m_fman.init_embedded_adaptors(glyph, x+m_tLastPos.fX, y+m_tLastPos.fY);

					if(glyph->data_type == agg::glyph_data_outline)
					{
						TRWPathPoint t;
						double x, y;
						unsigned cmd;
						m_curves.rewind(0);
						size_t iStart = cPath.size();
						while (!agg::is_stop(cmd = m_curves.vertex(&x, &y)))
						{
							if (agg::is_close(cmd))
							{
								continue;
							}
							if (agg::is_move_to(cmd))
							{
								if (cPath.size()-iStart >= 2)
								{
									if (cPath.size()-iStart > 2)
									{
										if (cPath[iStart].tPos.fX == cPath[cPath.size()-1].tPos.fX &&
											cPath[iStart].tPos.fY == cPath[cPath.size()-1].tPos.fY &&
											cPath[iStart].tTanPrev.fX == 0.0f && cPath[iStart].tTanPrev.fY == 0.0f &&
											cPath[cPath.size()-1].tTanNext.fX == 0.0f && cPath[cPath.size()-1].tTanNext.fY == 0.0f)
										{
											cPath[iStart].tTanPrev = cPath[cPath.size()-1].tTanPrev;
											cPath.erase(cPath.begin()+cPath.size()-1);
										}
									}
									TRWPath tPath;
									tPath.nVertices = cPath.size()-iStart;
									tPath.pVertices = NULL;
									cPaths.push_back(tPath);
								}
								iStart = cPath.size();
							}


							if (cmd == agg::path_cmd_curve3)
							{
								double end_x;
								double end_y;
								m_curves.vertex(&end_x, &end_y);
								if (!cPath.empty())
								{
									TRWPathPoint& p = cPath[cPath.size()-1];
									p.tTanNext.fX = 2.0*(x-p.tPos.fX)/3.0;
									p.tTanNext.fY = 2.0*(y-p.tPos.fY)/3.0;
									t.dwFlags = 0;
									t.tPos.fX = end_x;
									t.tPos.fY = end_y;
									t.tTanPrev.fX = 2.0*(x-end_x)/3.0;
									t.tTanPrev.fY = 2.0*(y-end_y)/3.0;
									t.tTanNext.fX = t.tTanNext.fY = 0.0f;
									cPath.push_back(t);
								}
								continue;
							}
							else if (cmd == agg::path_cmd_curve4)
							{
								double ct2_x;
								double ct2_y;
								m_curves.vertex(&ct2_x, &ct2_y);
								double end_x;
								double end_y;
								m_curves.vertex(&end_x, &end_y);
								if (!cPath.empty())
								{
									TRWPathPoint& p = cPath[cPath.size()-1];
									p.tTanNext.fX = 2.0*(x-p.tPos.fX)/3.0;
									p.tTanNext.fY = 2.0*(y-p.tPos.fY)/3.0;
									t.dwFlags = 0;
									t.tPos.fX = end_x;
									t.tPos.fY = end_y;
									t.tTanPrev.fX = 2.0*(ct2_x-end_x)/3.0;
									t.tTanPrev.fY = 2.0*(ct2_y-end_y)/3.0;
									t.tTanNext.fX = t.tTanNext.fY = 0.0f;
									cPath.push_back(t);
								}
								continue;
							}

							t.dwFlags = 0;
							t.tPos.fX = x;
							t.tPos.fY = y;
							t.tTanNext.fX = t.tTanNext.fY = t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
							cPath.push_back(t);
						}
						if (cPath.size()-iStart >= 2)
						{
							if (cPath.size()-iStart > 2)
							{
								if (cPath[iStart].tPos.fX == cPath[cPath.size()-1].tPos.fX &&
									cPath[iStart].tPos.fY == cPath[cPath.size()-1].tPos.fY &&
									cPath[iStart].tTanPrev.fX == 0.0f && cPath[iStart].tTanPrev.fY == 0.0f &&
									cPath[cPath.size()-1].tTanNext.fX == 0.0f && cPath[cPath.size()-1].tTanNext.fY == 0.0f)
								{
									cPath[iStart].tTanPrev = cPath[cPath.size()-1].tTanPrev;
									cPath.erase(cPath.begin()+cPath.size()-1);
								}
							}
							TRWPath tPath;
							tPath.nVertices = cPath.size()-iStart;
							tPath.pVertices = NULL;
							cPaths.push_back(tPath);
						}
					}

					// increment pen position
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}
		}

		ReleaseDC(NULL, hDC);

		if (cPaths.empty())
			return E_FAIL;
		size_t j = 0;
		for (std::vector<TRWPath>::iterator i = cPaths.begin(); i != cPaths.end(); ++i)
		{
			i->pVertices = &(cPath[j]);
			j += i->nVertices;
		}

		return a_pConsumer->FromPath(cPaths.size(), &(cPaths[0]));
	}

private:
	EState m_eState;
	TPixelCoords m_tLastPos;
	float m_fTextSizeX;
	float m_fTextSizeY;
	std::vector<double> m_cLineLens;
	agg::path_storage m_cGeometry;
	std::vector<std::vector<TPixelCoords> > m_cOriginal;
	double m_fGeoX1;
	double m_fGeoY1;
	double m_fGeoX2;
	double m_fGeoY2;
	float m_fLineSize;
	float m_fDeltaTop;
	float m_fDeltaBottom;
	TPixelCoords m_tDelta;
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
	float m_fWidth;
	CEditToolDataText m_cData;
};
