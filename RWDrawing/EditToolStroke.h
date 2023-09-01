
#pragma once

#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithBrush.h"
#include "EditToolPolyLine.h"
#include "EditToolWithCtrlDropper.h"
#include "EditToolPolygon.h"
#include "PolygonUtil.h"
#include "BezierDistance.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_curves.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_primitives.h>
#include <agg_conv_contour.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_rasterizer_compound_aa.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataStroke
{
	MIDL_INTERFACE("93BB1A7E-40CD-4AEE-9F52-019FECDFD0DE")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataStroke const*, InternalData)() = 0;
	};

	enum EEditState
	{
		EESPassive = 0,
		EESAddingPoints,
	};
	enum EPathMode
	{
		EPMControlHandles = 0,
		EPMFreeform,
	};

	CEditToolDataStroke() : //bClosed(false),
		eCapMode(agg::square_cap), eJoinMode(agg::bevel_join),
		fWidth(1.0), bUseDash(false), nDashLen(8), dwDash(0xc3),
		eState(EESAddingPoints), eMode(EPMControlHandles), fSmoothing(1.0f)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		eMode = EPMControlHandles;
		fSmoothing = 1.0f;

		swscanf(a_bstr, L"%f", &fWidth);
		if (wcsstr(a_bstr, L"CBUTT"))
			eCapMode = agg::butt_cap;
		else if (wcsstr(a_bstr, L"CROUND"))
			eCapMode = agg::round_cap;
		else if (wcsstr(a_bstr, L"CSQUARE"))
			eCapMode = agg::square_cap;
		if (wcsstr(a_bstr, L"JROUND"))
			eJoinMode = agg::round_join;
		else if (wcsstr(a_bstr, L"JMITER"))
			eJoinMode = agg::miter_join;
		else if (wcsstr(a_bstr, L"JBEVEL"))
			eJoinMode = agg::bevel_join;
		//if (wcsstr(a_bstr, L"CLOSED"))
		//	bClosed = true;
		//else if (wcsstr(a_bstr, L"OPEN"))
		//	bClosed = false;
		wchar_t const* p = wcsstr(a_bstr, L"DASH:");
		if (p)
		{
			p += 5;
			dwDash = 0;
			nDashLen = 0;
			bUseDash = false;
			while (*p)
			{
				if (*p != L' ')
					dwDash |= 1<<nDashLen;
				++nDashLen;
				++p;
			}
		}
		if (wcsstr(a_bstr, L"USEDASH"))
			bUseDash = true;

		p = wcsstr(a_bstr, L"FREEFORM:");
		if (p)
		{
			eMode = EPMFreeform;
			p += 9;
		}
		else
		{
			p = wcsstr(a_bstr, L"HANDLES:");
			if (p)
				p += 8;
			eMode = EPMControlHandles;
		}
		if (p)
			swscanf(p, L"%f", &fSmoothing);
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[300] = L"";
		swprintf(szTmp, L"%g|%s|%s|%s:%g|%s:", fWidth,
			eCapMode == agg::butt_cap ? L"BUTT" : (eCapMode == agg::round_cap ? L"CROUND" : L"CSQUARE"),
			eJoinMode == agg::bevel_join ? L"JBEVEL" : (eJoinMode == agg::round_join ? L"JROUND" : L"JMITER"),
			//bClosed ? L"CLOSED" : L"OPEN",
			eMode == EPMControlHandles ? L"HANDLES" : L"FREEFORM", fSmoothing,
			bUseDash ? L"USEDASH" : L"DASH");
		wchar_t* p = szTmp+wcslen(szTmp);
		for (BYTE b = 0; b < nDashLen; ++b, ++p)
			*p = dwDash&(1<<b) ? L'-' : L' ';
		*p = L'\0';
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	agg::line_cap_e eCapMode;
	//bool bClosed;
	agg::line_join_e eJoinMode;
	float fWidth;
	bool bUseDash;
	BYTE nDashLen;
	DWORD dwDash;

	EEditState eState;
	EPathMode eMode;
	float fSmoothing;
};


HICON GetToolIconSTROKE(ULONG a_nSize);

class CEditToolStroke :
	public CEditToolScanlineBuffer<CEditToolStroke>, // scanline image cache
	public CEditToolMouseInput<CEditToolStroke>, // no direct tablet support
	public CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, // brush override
	public CEditToolOutline<CEditToolStroke>, // outline handling
	public CEditToolCustomOrMoveCursor<CEditToolStroke, GetToolIconSTROKE>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolStroke, CEditToolMouseInput<CEditToolStroke>, CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, CEditToolCustomOrMoveCursor<CEditToolStroke, GetToolIconSTROKE> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolStroke, // T - the top level class for cross casting
		CEditToolStroke, // TResetHandler
		CEditToolScanlineBuffer<CEditToolStroke>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolStroke>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolOutline<CEditToolStroke>, // TOutlineHandler
		CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, // TBrushHandler
		CEditToolStroke, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolStroke, CEditToolMouseInput<CEditToolStroke>, CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, CEditToolCustomOrMoveCursor<CEditToolStroke, GetToolIconSTROKE> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolStroke, CEditToolMouseInput<CEditToolStroke>, CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, CEditToolCustomOrMoveCursor<CEditToolStroke, GetToolIconSTROKE> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolStroke, CEditToolMouseInput<CEditToolStroke>, CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, CEditToolCustomOrMoveCursor<CEditToolStroke, GetToolIconSTROKE> >, // TProcessInputHandler
		CEditToolStroke, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolStroke, CEditToolStroke, CEditToolStroke> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolPolygon,
	public IEditToolLine,
	public IRasterImageEditToolContextMenu
{
public:
	CEditToolStroke() : m_eBlendingMode(EBMDrawOver), m_nSplitting(-1),
		m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		m_bMoving(false), m_bAdding(false)
	{
		m_iInsert = m_cPath.end();
	}

	BEGIN_COM_MAP(CEditToolStroke)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IEditToolLine)
	END_COM_MAP()

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
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
	void GetAsPolygon(std::vector<std::pair<std::vector<TPixelCoords>, bool> >& cOriginal)
	{
		cOriginal.resize(1);
		TRWPathPoint tFirst = m_cPath[0];
		TRWPathPoint tPrev = tFirst;
		std::vector<std::pair<std::vector<TPixelCoords>, bool> >::iterator cOriginal1 = cOriginal.begin();
		cOriginal1->second = false;
		for (CPath::const_iterator i = m_cPath.begin()+1; i < m_cPath.end(); ++i)
		{
			if (tPrev.dwFlags&EPPClose)
			{
				if ((tPrev.dwFlags&EPPRealClose) == EPPRealClose && (tPrev.tPos.fX != tFirst.tPos.fX || tPrev.tPos.fY != tFirst.tPos.fY))
				{
					agg::curve4 c(
						tPrev.tPos.fX, tPrev.tPos.fY,
						tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
						tFirst.tPos.fX+tFirst.tTanPrev.fX, tFirst.tPos.fY+tFirst.tTanPrev.fY,
						tFirst.tPos.fX, tFirst.tPos.fY
					);
					double x = 0.0;
					double y = 0.0;
					while (!agg::is_stop(c.vertex(&x, &y)))
					{
						TPixelCoords t = {x, y};
						cOriginal1->first.push_back(t);
					}
					cOriginal1->second = true;
				}
				tFirst = *i;
				tPrev = *i;
				cOriginal.resize(cOriginal.size()+1);
				cOriginal1 = cOriginal.begin()+cOriginal.size()-1;
				cOriginal1->second = false;
				continue;
			}
			agg::curve4 c(
				tPrev.tPos.fX, tPrev.tPos.fY,
				tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
				i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY,
				i->tPos.fX, i->tPos.fY
			);
			double x = 0.0;
			double y = 0.0;
			while (!agg::is_stop(c.vertex(&x, &y)))
			{
				TPixelCoords t = {x, y};
				cOriginal1->first.push_back(t);
			}
			tPrev = *i;
		}
		if ((tPrev.dwFlags&EPPRealClose) == EPPRealClose && (tPrev.tPos.fX != tFirst.tPos.fX || tPrev.tPos.fY != tFirst.tPos.fY))
		{
			agg::curve4 c(
				tPrev.tPos.fX, tPrev.tPos.fY,
				tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
				tFirst.tPos.fX+tFirst.tTanPrev.fX, tFirst.tPos.fY+tFirst.tTanPrev.fY,
				tFirst.tPos.fX, tFirst.tPos.fY
			);
			double x = 0.0;
			double y = 0.0;
			while (!agg::is_stop(c.vertex(&x, &y)))
			{
				TPixelCoords t = {x, y};
				cOriginal1->first.push_back(t);
			}
			cOriginal1->second = true;
		}
	}
	void GetAsShape(std::vector<std::vector<TPixelCoords> >& dst)
	{
		std::vector<std::pair<std::vector<TPixelCoords>, bool> > cOriginal;
		GetAsPolygon(cOriginal);

		agg::path_storage p1;

		for (std::vector<std::pair<std::vector<TPixelCoords>, bool> >::const_iterator i = cOriginal.begin(); i != cOriginal.end(); ++i)
		{
			CEditToolPolyLine<CEditToolPolygon>::CVertexSource pl(i->first, i->second ? agg::path_flags_close : 0);

			if (m_cData.bUseDash && m_cData.dwDash && m_cData.dwDash != (1<<m_cData.nDashLen)-1)
			{
				agg::conv_dash<CEditToolPolyLine<CEditToolPolygon>::CVertexSource> dash(pl);
				DWORD dwDash = m_cData.dwDash;
				BYTE nLen = m_cData.nDashLen;
				while (dwDash&1 == 0 && nLen)
				{
					dwDash>>=1;
					--nLen;
				}
				BYTE nLastPause = m_cData.nDashLen-nLen;
				int i1 = 0;
				int i2 = 0;
				while (nLen)
				{
					if (i2 == 0)
					{
						if (dwDash&1)
						{
							++i1;
						}
						else
						{
							i2 = 1;
						}
					}
					else
					{
						if (dwDash&1)
						{
							dash.add_dash(m_cData.fWidth*i1, m_cData.fWidth*i2);
							i1 = 1;
							i2 = 0;
						}
						else
						{
							++i2;
						}
					}
					dwDash>>=1;
					--nLen;
				}
				dash.add_dash(m_cData.fWidth*i1, m_cData.fWidth*(i2+nLastPause));
				dash.dash_start(m_cData.fWidth*nLen);
				agg::conv_stroke<agg::conv_dash<CEditToolPolyLine<CEditToolPolygon>::CVertexSource> > pg(dash);
				pg.line_cap(m_cData.eCapMode);
				pg.line_join(m_cData.eJoinMode);
				//pg.miter_limit(m_miter_limit.value());
				pg.width(m_cData.fWidth);
				p1.concat_path(pg);
			}
			else
			{
				agg::conv_stroke<CEditToolPolyLine<CEditToolPolygon>::CVertexSource> pg(pl);
				pg.line_cap(m_cData.eCapMode);
				pg.line_join(m_cData.eJoinMode);
				//pg.miter_limit(m_miter_limit.value());
				pg.width(m_cData.fWidth);
				p1.concat_path(pg);
			}
		}

		std::vector<std::vector<TPixelCoords> > cSecond;
		std::vector<TPixelCoords>* cur = NULL;

		bool first = true;
		double x, y/*, xx, yy*/;
		p1.rewind(0);
        unsigned cmd;
		while (!agg::is_stop(cmd = p1.vertex(&x, &y)))
        {
            if (agg::is_vertex(cmd))
            {
                if (first)
                {
					size_t s = cSecond.size();
					cSecond.resize(s+1);
					cur = &(cSecond[s]);
                    first = false;
                }
				TPixelCoords t = {x, y};
				cur->push_back(t);
            }
			if (agg::is_end_poly(cmd))
				first = true;
        }

		SplitPolygons(cSecond, dst, true);
	}
	void PrepareShape()
	{
		if (m_cPath.size() < 2)
			return; // invalid polygon

		std::vector<std::vector<TPixelCoords> > cOuter;
		GetAsShape(cOuter);

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

//ras.add_path(p1);
//agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
//TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
//ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
//agg::render_scanlines(ras, sl, ren);

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
				agg::render_scanlines_compound_layered(rasc, sl, renb, alloc, *this, M_LayerBlender());
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
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		RECT const rc = M_DirtyRect();
		m_bMoving = false;
		m_bAdding = false;
		m_bLastValid = false;
		m_iInsert = m_cPath.end();
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		m_cPath.clear();
		m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		ResetDragging();
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		EnableBrush(false);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataStroke::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			//if (m_cData.eStartWith != pData->InternalData()->eStartWith && !M_PolyLine().empty())
			//{
			//	RECT rc = UpdateCache();
			//	M_Window()->RectangleChanged(&rc);
			//	// TODO: switch shape if it is unmodified
			//}
			CEditToolDataStroke::EPathMode prev = m_cData.eMode;
			m_cData = *(pData->InternalData());
			if (prev != m_cData.eMode)
			{
				if (m_cPath.size() == 1)
					m_cPath.clear();

				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
			}
			if (m_cPath.size() > 1)
			{
				if (prev != m_cData.eMode && m_cData.eMode == CEditToolDataStroke::EPMControlHandles)
					m_cData.eState = CEditToolDataStroke::EESPassive;
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
		}
		return S_OK;
	}

	void OutlineChanged(bool a_bWidth, bool a_bColor)
	{
		if (!m_cPath.empty())
		{
			InvalidateCachePixels();
			M_Window()->RectangleChanged(&M_DirtyRect());
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
		if (m_cPath.empty())
			return S_OK;
		RECT rcPrev = M_DirtyRect();
		bool bRedrawCache = bRasterizationChange;
		if (bCoordinatesChange && m_eCoordinatesMode == ECMIntegral)
		{
			static TPixelCoords const tPointerSize = {0.0f, 0.0f};
			for (CPath::iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
				_AdjustCoordinates(ECKSNone, &(i->tPos), &tPointerSize, NULL, 0.0f);
			for (ULONG i = 0; i < m_cPath.size()<<1; ++i)
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
		EnableBrush((!M_Dragging() || m_bMoving) && rcPrev.left < rcPrev.right);
		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_bMoving = false; // just in case...
		m_bAdding = false;
		if (m_cData.eState == CEditToolDataStroke::EESPassive)
		{
			if (HitTest(a_pPos->fX, a_pPos->fY))
			{
				m_bMoving = true;
				m_tLastPos = *a_pPos;
				return S_OK;
			}
			if (!m_cPath.empty())
			{
				ATLASSERT(0);
				//M_Window()->ApplyChanges();
			}
			return S_OK;
		}

		if (m_cData.eState == CEditToolDataStroke::EESAddingPoints)
		{
			if (m_cData.eMode == CEditToolDataStroke::EPMFreeform)
			{
				std::vector<CPath> paths;
				SplitPaths(m_cPath, paths);

				float dist = ClosestPoint(paths.begin(), paths.end(), *a_pPos, &m_tPrevCoords, &m_iPrevSegment, &m_iPrevArc, &m_fPrevParam);
				if (dist >= 0.0f && dist <= M_PointSize()*32)
				{
					// possible shape modification
					std::swap(m_cPrevPaths, paths);
				}
				else
				{
					// new shape
					std::swap(m_cPrevPaths, paths); // paths should be actually empty in this case
					m_iPrevSegment = m_cPrevPaths.end();
				}
				m_cPathPoints.clear();
				m_cPathPoints.push_back(SPathPoint(*a_pPos, GetTickCount()));
			}
			else
			{
				m_bAdding = true;
				TRWPathPoint t;
				t.dwFlags = 0;
				t.tPos = *a_pPos;
				t.tTanNext.fX = t.tTanNext.fY = t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
				bool const end = m_iInsert == m_cPath.end();
				m_iInsert = m_cPath.insert(m_iInsert, t);
				if (!end && m_iInsert != m_cPath.begin())
				{
					CPath::iterator tmp = m_iInsert;
					--tmp;
					m_iInsert->dwFlags = tmp->dwFlags;
					tmp->dwFlags = 0;
				}
			}

			ToolSetBrush();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
			EnableBrush(M_Brush());
		}
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

		if (m_bAdding)
		{
			//if (a_pPos->fX != s.tPos.fX || a_pPos->fY != s.tPos.fY)
			{
				m_iInsert->tTanNext.fX = a_pPos->fX-m_iInsert->tPos.fX;
				m_iInsert->tTanNext.fY = a_pPos->fY-m_iInsert->tPos.fY;
				m_iInsert->tTanPrev.fX = m_iInsert->tPos.fX-a_pPos->fX;
				m_iInsert->tTanPrev.fY = m_iInsert->tPos.fY-a_pPos->fY;
				++m_iInsert;
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
			m_bAdding = false;
		}
		m_cPathPoints.clear();
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		EnableBrush(true);
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
				for (CPath::iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
				{
					i->tPos.fX += fDX;
					i->tPos.fY += fDY;
					ii += 3;
				}
				//for (ULONG i = 0; i < ii; ++i)
				//	M_Window()->ControlPointChanged(i);
				M_Window()->ControlPointsChanged(); // uncomment the above and delete this when vector image view is smarther
				M_Window()->ControlLinesChanged();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
			return S_OK;
		}
		if (!m_cPathPoints.empty())
		{
			m_cPathPoints.push_back(SPathPoint(*a_pPos, GetTickCount()));
			ProcessPath(GetTickCount(), M_PointSize());
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
			ToolSetBrush();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
		}
		else if (m_bAdding)
		{
			if (a_pPos->fX != m_iInsert->tPos.fX || a_pPos->fY != m_iInsert->tPos.fY)
			{
				m_iInsert->tTanNext.fX = a_pPos->fX-m_iInsert->tPos.fX;
				m_iInsert->tTanNext.fY = a_pPos->fY-m_iInsert->tPos.fY;
				m_iInsert->tTanPrev.fX = m_iInsert->tPos.fX-a_pPos->fX;
				m_iInsert->tTanPrev.fY = m_iInsert->tPos.fY-a_pPos->fY;
				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
		}
		if (m_cData.eState == CEditToolDataStroke::EESAddingPoints &&
			(m_tLastPos.fX != a_pPos->fX || m_tLastPos.fY != a_pPos->fY || !m_bLastValid))
		{
			m_tLastPos = *a_pPos;
			m_bLastValid = true;
			M_Window()->ControlLinesChanged();
		}

		return S_OK;
	}
	HRESULT OnMouseLeave()
	{
		if (m_bLastValid)
		{
			m_bLastValid = false;
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_bMoving || (!M_Dragging() && m_cData.eState == CEditToolDataStroke::EESPassive && HitTest(a_pPos->fX, a_pPos->fY));
	}

	void ToolSetBrush()
	{
		// TODO: get actual bounding box
		if (M_Brush() && !m_cPath.empty())
		{
			TPixelCoords tSum = m_cPath[0].tPos;
			TPixelCoords tMin = tSum;
			TPixelCoords tMax = tSum;
			for (CPath::const_iterator i = m_cPath.begin()+1; i != m_cPath.end(); ++i)
			{
				tSum.fX += i->tPos.fX;
				tSum.fY += i->tPos.fY;
				if (tMin.fX > i->tPos.fX) tMin.fX = i->tPos.fX;
				if (tMax.fX < i->tPos.fX) tMax.fX = i->tPos.fX;
				if (tMin.fY > i->tPos.fY) tMin.fY = i->tPos.fY;
				if (tMax.fY < i->tPos.fY) tMax.fY = i->tPos.fY;
			}
			tSum.fX /= m_cPath.size();
			tSum.fY /= m_cPath.size();
			M_Brush()->SetShapeBounds(&tSum, (tMax.fX-tMin.fX)*0.5f, (tMax.fY-tMin.fY)*0.5f, 0.0f);
		}
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		if (m_eCoordinatesMode == ECMIntegral)
		{
			a_pPos->fX = floorf(a_pPos->fX + 0.5f);
			a_pPos->fY = floorf(a_pPos->fY + 0.5f);
			//a_pPos->fX = 0.5f + floorf(a_pPos->fX+0.5f*a_pPointerSize->fX);
			//a_pPos->fY = 0.5f + floorf(a_pPos->fY+0.5f*a_pPointerSize->fY);
		}
		//else
		//{
		//	a_pPos->fX += 0.5f*a_pPointerSize->fX;
		//	a_pPos->fY += 0.5f*a_pPointerSize->fY;
		//}
		return S_OK;
	}

	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		InvalidateCachePixels();
		if (a_pChanged == NULL)
			return M_Window()->RectangleChanged(&M_DirtyRect());
		RECT rcShape = M_DirtyRect();
		EnableBrush((!M_Dragging() || m_bMoving) && m_cPath.size() > 1);
		if (rcShape.left < a_pChanged->left) rcShape.left = a_pChanged->left;
		if (rcShape.top < a_pChanged->top) rcShape.top = a_pChanged->top;
		if (rcShape.right > a_pChanged->right) rcShape.right = a_pChanged->right;
		if (rcShape.bottom > a_pChanged->bottom) rcShape.bottom = a_pChanged->bottom;
		return M_Window()->RectangleChanged(&rcShape);
	}
	void PolygonChanged()
	{
		if (m_cPath.size() < 2)
		{
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
			EnableBrush(false);
		}
		ToolSetBrush();
		RECT const rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		if (m_cData.eMode != CEditToolDataStroke::EPMControlHandles)
		{
			*a_pCount = 0;
			return S_OK;
		}

		*a_pCount = m_cPath.size()*CPStep;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (m_cData.eMode != CEditToolDataStroke::EPMControlHandles)
			return E_RW_INDEXOUTOFRANGE;

		if (a_nIndex >= m_cPath.size()*CPStep)
			return E_RW_INDEXOUTOFRANGE;
		ULONG const nPt = a_nIndex/CPStep;
		TRWPathPoint const& s = m_cPath[nPt];
		switch (a_nIndex-nPt*CPStep)
		{
		case CPTanPrev:
			a_pPos->fX = s.tTanPrev.fX+s.tPos.fX;
			a_pPos->fY = s.tTanPrev.fY+s.tPos.fY;
			*a_pClass = 2;//61;39;44
			break;
		case CPTanNext:
			a_pPos->fX = s.tTanNext.fX+s.tPos.fX;
			a_pPos->fY = s.tTanNext.fY+s.tPos.fY;
			*a_pClass = 2;
			break;
		case CPPos:
			*a_pPos = s.tPos;
			*a_pClass = s.dwFlags&EPPSharp ? 39 : 0;
			break;
		case CPSegment:
			if (m_nSplitting == nPt)
			{
				*a_pPos = m_tLastSplit;
			}
			else
			{
				size_t const next = NextPoint(nPt);
				if (next < nPt && (s.dwFlags&EPPRealClose) != EPPRealClose)
				{
					*a_pPos = s.tPos;
				}
				else
				{
					TRWPathPoint const& t = m_cPath[next];
					a_pPos->fX = 0.5f*s.tPos.fX + 0.375f*s.tTanNext.fX + 0.5f*t.tPos.fX + 0.375f*t.tTanPrev.fX;
					a_pPos->fY = 0.5f*s.tPos.fY + 0.375f*s.tTanNext.fY + 0.5f*t.tPos.fY + 0.375f*t.tTanPrev.fY;
				}
			}
			*a_pClass = 1;
			break;
		}
		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (m_cData.eMode != CEditToolDataStroke::EPMControlHandles)
			return E_RW_INDEXOUTOFRANGE;

		if (a_nIndex >= m_cPath.size()*CPStep)
			return E_RW_INDEXOUTOFRANGE;
		m_bLastValid = false;
		ULONG const nPt = a_nIndex/CPStep;
		TRWPathPoint& s = m_cPath[nPt];
		switch (a_nIndex-nPt*CPStep)
		{
		case CPTanPrev:
			s.tTanPrev.fX = a_pPos->fX-s.tPos.fX;
			s.tTanPrev.fY = a_pPos->fY-s.tPos.fY;
			if (a_fPointSize*a_fPointSize >= s.tTanPrev.fX*s.tTanPrev.fX+s.tTanPrev.fY*s.tTanPrev.fY)
			{
				s.tTanPrev.fX = s.tTanPrev.fY = 0.0f;
			}
			M_Window()->ControlPointChanged(a_nIndex);
			if (0 == (s.dwFlags&EPPSharp))
			{
				float const fP = sqrtf(s.tTanPrev.fX*s.tTanPrev.fX+s.tTanPrev.fY*s.tTanPrev.fY);
				float const fN = sqrtf(s.tTanNext.fX*s.tTanNext.fX+s.tTanNext.fY*s.tTanNext.fY);
				if (fP > 1e-6f && fN > 1e-6f)
				{
					s.tTanNext.fX = -s.tTanPrev.fX*fN/fP;
					s.tTanNext.fY = -s.tTanPrev.fY*fN/fP;
					M_Window()->ControlPointChanged(nPt*CPStep+CPTanNext);
					M_Window()->ControlPointChanged(nPt*CPStep+CPSegment);
				}
			}
			M_Window()->ControlPointChanged(PrevPoint(nPt)*CPStep+CPSegment);
			break;
		case CPTanNext:
			s.tTanNext.fX = a_pPos->fX-s.tPos.fX;
			s.tTanNext.fY = a_pPos->fY-s.tPos.fY;
			if (a_fPointSize*a_fPointSize >= s.tTanNext.fX*s.tTanNext.fX+s.tTanNext.fY*s.tTanNext.fY)
			{
				s.tTanNext.fX = s.tTanNext.fY = 0.0f;
			}
			M_Window()->ControlPointChanged(a_nIndex);
			if (0 == (s.dwFlags&EPPSharp))
			{
				float const fP = sqrtf(s.tTanPrev.fX*s.tTanPrev.fX+s.tTanPrev.fY*s.tTanPrev.fY);
				float const fN = sqrtf(s.tTanNext.fX*s.tTanNext.fX+s.tTanNext.fY*s.tTanNext.fY);
				if (fP > 1e-6f && fN > 1e-6f)
				{
					s.tTanPrev.fX = -s.tTanNext.fX*fP/fN;
					s.tTanPrev.fY = -s.tTanNext.fY*fP/fN;
					M_Window()->ControlPointChanged(nPt*CPStep+CPTanPrev);
					M_Window()->ControlPointChanged(PrevPoint(nPt)*CPStep+CPSegment);
				}
			}
			M_Window()->ControlPointChanged(nPt*CPStep+CPSegment);
			break;
		case CPPos:
			s.tPos = *a_pPos;
			M_Window()->ControlPointChanged(PrevPoint(nPt)*CPStep+CPSegment);
			M_Window()->ControlPointChanged(nPt*CPStep+CPSegment);
			M_Window()->ControlPointChanged(nPt*CPStep+CPTanNext);
			M_Window()->ControlPointChanged(nPt*CPStep+CPTanPrev);
			M_Window()->ControlPointChanged(a_nIndex);
			break;
		case CPSegment:
			if (a_bFinished)
			{
				m_nSplitting = -1;
				TRWPathPoint& t = m_cPath[NextPoint(nPt)];
				TRWPathPoint n;
				Bezier b(s.tPos.fX, s.tPos.fY, s.tPos.fX+s.tTanNext.fX, s.tPos.fY+s.tTanNext.fY, t.tPos.fX+t.tTanPrev.fX, t.tPos.fY+t.tTanPrev.fY, t.tPos.fX, t.tPos.fY);
				double fLastSplit = b.NearestPointOnCurve(TVector2d(a_pPos->fX, a_pPos->fY));
				if (fLastSplit <= 0.0 || fLastSplit >= 1.0)
				{
					M_Window()->ControlPointChanged(a_nIndex);
					return E_FAIL;
				}
				Bezier l(3);
				Bezier r(3);
				TVector2d tPt = b.Evaluate(fLastSplit);
				TVector2d tPt2 = b.Evaluate(0.5);
				if (tPt.DistanceSquared(tPt2) < a_fPointSize*a_fPointSize)
					fLastSplit = 0.5;
				b.Evaluate(fLastSplit, &l, &r);
				n.dwFlags = 0;
				t.tTanPrev.fX = r.m_pts[2].x-r.m_pts[3].x;
				t.tTanPrev.fY = r.m_pts[2].y-r.m_pts[3].y;
				s.tTanNext.fX = l.m_pts[1].x-l.m_pts[0].x;
				s.tTanNext.fY = l.m_pts[1].y-l.m_pts[0].y;
				n.tPos.fX = l.m_pts[3].x;
				n.tPos.fY = l.m_pts[3].y;
				n.tTanPrev.fX = l.m_pts[2].x-l.m_pts[3].x;
				n.tTanPrev.fY = l.m_pts[2].y-l.m_pts[3].y;
				n.tTanNext.fX = r.m_pts[1].x-r.m_pts[0].x;
				n.tTanNext.fY = r.m_pts[1].y-r.m_pts[0].y;
				size_t index = m_iInsert-m_cPath.begin();
				size_t next = NextPoint(nPt);
				m_cPath.insert(m_cPath.begin()+next, n);
				m_iInsert = m_cPath.begin()+index;
				if (next < index)
					++m_iInsert;
				M_Window()->ControlPointsChanged();
			}
			else
			{
				m_nSplitting = nPt;
				TRWPathPoint& t = m_cPath[NextPoint(nPt)];
				Bezier b(s.tPos.fX, s.tPos.fY, s.tPos.fX+s.tTanNext.fX, s.tPos.fY+s.tTanNext.fY, t.tPos.fX+t.tTanPrev.fX, t.tPos.fY+t.tTanPrev.fY, t.tPos.fX, t.tPos.fY);
				double fLastSplit = b.NearestPointOnCurve(TVector2d(a_pPos->fX, a_pPos->fY));
				TVector2d tPt = b.Evaluate(fLastSplit);
				TVector2d tPt2 = b.Evaluate(0.5);
				if (tPt.DistanceSquared(tPt2) < a_fPointSize*a_fPointSize)
					tPt = tPt2;
				m_tLastSplit.fX = tPt.x;
				m_tLastSplit.fY = tPt.y;
				M_Window()->ControlPointChanged(a_nIndex);
			}
			break;
		}
		M_Window()->ControlLinesChanged();
		ToolSetBrush();
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		try
		{
			if ((a_nLineTypes&ECLTSelection) && m_cPath.size() >= 2)
			{
				std::vector<std::vector<TPixelCoords> > cOriginal;

				cOriginal.resize(1);
				TRWPathPoint tFirst = m_cPath[0];
				TRWPathPoint tPrev = tFirst;
				std::vector<std::vector<TPixelCoords> >::iterator cOriginal1 = cOriginal.begin();
				bool bFirst = true;
				for (CPath::const_iterator i = m_cPath.begin()+1; i < m_cPath.end(); ++i)
				{
					if (tPrev.dwFlags&EPPClose)
					{
						if ((tPrev.dwFlags&EPPRealClose) == EPPRealClose && (tPrev.tPos.fX != tFirst.tPos.fX || tPrev.tPos.fY != tFirst.tPos.fY))
						{
							agg::curve4 c(
								tPrev.tPos.fX, tPrev.tPos.fY,
								tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
								tFirst.tPos.fX+tFirst.tTanPrev.fX, tFirst.tPos.fY+tFirst.tTanPrev.fY,
								tFirst.tPos.fX, tFirst.tPos.fY
							);
							double x = 0.0;
							double y = 0.0;
							while (!agg::is_stop(c.vertex(&x, &y)))
							{
								if (bFirst)
								{
									a_pLines->MoveTo(x, y);
									bFirst = false;
								}
								else
								{
									a_pLines->LineTo(x, y);
								}
							}
						}
						tFirst = *i;
						tPrev = *i;
						cOriginal.resize(cOriginal.size()+1);
						cOriginal1 = cOriginal.begin()+cOriginal.size()-1;
						bFirst = true;
						continue;
					}
					agg::curve4 c(
						tPrev.tPos.fX, tPrev.tPos.fY,
						tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
						i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY,
						i->tPos.fX, i->tPos.fY
					);
					double x = 0.0;
					double y = 0.0;
					while (!agg::is_stop(c.vertex(&x, &y)))
					{
						if (bFirst)
						{
							a_pLines->MoveTo(x, y);
							bFirst = false;
						}
						else
						{
							a_pLines->LineTo(x, y);
						}
					}
					tPrev = *i;
				}
				if ((tPrev.dwFlags&EPPRealClose) == EPPRealClose && (tPrev.tPos.fX != tFirst.tPos.fX || tPrev.tPos.fY != tFirst.tPos.fY))
				{
					agg::curve4 c(
						tPrev.tPos.fX, tPrev.tPos.fY,
						tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
						tFirst.tPos.fX+tFirst.tTanPrev.fX, tFirst.tPos.fY+tFirst.tTanPrev.fY,
						tFirst.tPos.fX, tFirst.tPos.fY
					);
					double x = 0.0;
					double y = 0.0;
					while (!agg::is_stop(c.vertex(&x, &y)))
					{
						a_pLines->LineTo(x, y);
					}
				}
			}

			if (a_nLineTypes&ECLTHelp)
			{
				if (m_cData.eMode == CEditToolDataStroke::EPMControlHandles)
				{
					for (CPath::const_iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
					{
						if (i->tTanNext.fX != 0.0f || i->tTanNext.fY != 0.0f)
						{
							if (i->tTanPrev.fX != 0.0f || i->tTanPrev.fY != 0.0f)
							{
								a_pLines->MoveTo(i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY);
								a_pLines->LineTo(i->tPos.fX, i->tPos.fY);
								a_pLines->LineTo(i->tPos.fX+i->tTanNext.fX, i->tPos.fY+i->tTanNext.fY);
							}
							else
							{
								a_pLines->MoveTo(i->tPos.fX, i->tPos.fY);
								a_pLines->LineTo(i->tPos.fX+i->tTanNext.fX, i->tPos.fY+i->tTanNext.fY);
							}
						}
						else if (i->tTanPrev.fX != 0.0f || i->tTanPrev.fY != 0.0f)
						{
							a_pLines->MoveTo(i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY);
							a_pLines->LineTo(i->tPos.fX, i->tPos.fY);
						}
					}
					if (m_bLastValid && !M_Dragging() && m_cData.eState == CEditToolDataStroke::EESAddingPoints &&
						!m_cPath.empty() && m_iInsert != m_cPath.begin())
					{
						CPath::iterator i = m_iInsert;
						--i;
						if ((m_iInsert == m_cPath.end() && (i->dwFlags&EPPClose) == 0) ||
							(m_iInsert != m_cPath.end() && i->dwFlags&EPPClose))
						{
							a_pLines->MoveTo(i->tPos.fX, i->tPos.fY);
							a_pLines->LineTo(m_tLastPos.fX, m_tLastPos.fY);
						}
						else if (m_iInsert != m_cPath.end() && (m_iInsert->dwFlags&EPPClose) == 0)
						{
							a_pLines->MoveTo(
								0.5f*i->tPos.fX + 0.375f*i->tTanNext.fX + 0.5f*m_iInsert->tPos.fX + 0.375f*m_iInsert->tTanPrev.fX,
								0.5f*i->tPos.fY + 0.375f*i->tTanNext.fY + 0.5f*m_iInsert->tPos.fY + 0.375f*m_iInsert->tTanPrev.fY);
							a_pLines->LineTo(m_tLastPos.fX, m_tLastPos.fY);
						}
					}
				}
				else if (m_cData.eState == CEditToolDataStroke::EESAddingPoints)
				{
					TPixelCoords p2;
					if (m_bLastValid && ClosestPoint(m_tLastPos, M_PointSize(), &p2))
					{
						a_pLines->MoveTo(m_tLastPos.fX, m_tLastPos.fY);
						a_pLines->LineTo(p2.fX, p2.fY);
					}
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float a_fPointSize)
	{
		if (a_eKeysState&ECKSControl)
			return ETPAMissed;

		if (m_cData.eMode == CEditToolDataStroke::EPMFreeform)
		{
			if (m_cData.eState == CEditToolDataStroke::EESPassive)
				return HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;

			if (ClosestPoint(*a_pPos, a_fPointSize, NULL))
				return ETPAHit|ETPACustomAction;

			return HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPACustomAction : ETPAMissed|ETPAStartNew;
		}

		return HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|(m_cData.eState == CEditToolDataStroke::EESAddingPoints ? ETPACustomAction : ETPATransform) : ETPAMissed|(m_cData.eState == CEditToolDataStroke::EESAddingPoints ? ETPACustomAction : ETPAStartNew);
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
		m_cData.fWidth *= fScale;
		for (CPath::iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
		{
			TPixelCoords tPrev = {i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY};
			TPixelCoords tNext = {i->tPos.fX+i->tTanNext.fX, i->tPos.fY+i->tTanNext.fY};
			i->tPos = TransformPixelCoords(*a_pMatrix, i->tPos);
			i->tTanPrev = TransformPixelCoords(*a_pMatrix, tPrev);
			i->tTanPrev.fX -= i->tPos.fX;
			i->tTanPrev.fY -= i->tPos.fY;
			i->tTanNext = TransformPixelCoords(*a_pMatrix, tNext);
			i->tTanNext.fX -= i->tPos.fX;
			i->tTanNext.fY -= i->tPos.fY;
		}
		TransformBrush(a_pMatrix);
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		ToolSetBrush();
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			float fWidth = 0.0f;
			std::wstring strDash;
			agg::line_cap_e eCapMode = agg::square_cap;
			agg::line_join_e eJoinMode = agg::bevel_join;
			CPath cPath;
			TRWPathPoint t;
			bool bClosed = false;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					str_p(L"\"CWIDTH\"")>>cSep>>real_p[assign_a(fWidth)]>>
					(!(cSep >> ch_p(L'\"')>>(*(ch_p(L' ')|ch_p(L'-')))[assign_a(strDash)]>>ch_p(L'\"')))>>
					(!(cSep >> (str_p(L"\"JROUND\"")[assign_a(eJoinMode, agg::round_join)]|str_p(L"\"JBEVEL\"")[assign_a(eJoinMode, agg::bevel_join)]|str_p(L"\"JMITER\"")[assign_a(eJoinMode, agg::miter_join)])))>>
					(!(cSep >> (str_p(L"\"CROUND\"")[assign_a(eCapMode, agg::round_cap)]|str_p(L"\"CBUTT\"")[assign_a(eCapMode, agg::butt_cap)]|str_p(L"\"CSQUARE\"")[assign_a(eCapMode, agg::square_cap)])))>>
					(*(
						cSep >> (str_p(L"\"C\"")[assign_a(t.dwFlags, 0)]|str_p(L"\"S\"")[assign_a(t.dwFlags, EPPSharp)]|str_p(L"\"E\"")[assign_a(t.dwFlags, EPPClose)]|str_p(L"\"O\"")[assign_a(t.dwFlags, EPPRealClose)])>>
						(cSep >> real_p[assign_a(t.tPos.fX)]>>cSep>>real_p[assign_a(t.tPos.fY)]>>cSep>>real_p[assign_a(t.tTanNext.fX)]>>cSep>>real_p[assign_a(t.tTanNext.fY)]>>cSep>>real_p[assign_a(t.tTanPrev.fX)]>>cSep>>real_p[assign_a(t.tTanPrev.fY)])
					)[push_back_a(cPath, t)])
					>>*space_p).full;
			if (!bParsed || strDash.length() > 32 || fWidth <= 0.0f || cPath.size() < 2)
				return E_INVALIDARG;
			bool bStateChange = m_cData.eCapMode != eCapMode || m_cData.eJoinMode != eJoinMode || m_cData.fWidth != fWidth;//m_cData.bClosed != bClosed || 
			m_cData.eCapMode = eCapMode;
			m_cData.eJoinMode = eJoinMode;
			m_cData.fWidth = fWidth;
			bool bPrevDash = m_cData.bUseDash;
			BYTE nPrevDashLen = m_cData.nDashLen;
			DWORD dwPrevDash = m_cData.dwDash;
			if (m_cData.bUseDash = !strDash.empty())
			{
				m_cData.nDashLen = strDash.length();
				m_cData.dwDash = 0;
				for (BYTE i = 0; i < m_cData.nDashLen; ++i)
					if (strDash[i] == L'-')
						m_cData.dwDash |= 1<<i;
			}
			if (bPrevDash != m_cData.bUseDash || (bPrevDash && (nPrevDashLen != m_cData.nDashLen || dwPrevDash != m_cData.dwDash)))
				bStateChange = true;
			//SetPolyLineClosed(m_cData.bClosed = bClosed);
			m_bMoving = false;
			if (bStateChange)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
			}
			std::swap(cPath, m_cPath);
			m_iInsert = m_cPath.end();
			EnableBrush(true);
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
			ToolSetBrush();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
			m_cData.eState = CEditToolDataStroke::EESPassive;
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
			CComBSTR bstr;
			OLECHAR sz[144];

			swprintf(sz, L"\"CWIDTH\", %g", m_cData.fWidth);
			bstr = sz;
			if (m_cData.bUseDash)
			{
				sz[0] = L',';
				sz[1] = L' ';
				sz[2] = L'"';
				for (BYTE b = 0; b < m_cData.nDashLen; ++b)
					sz[3+b] = m_cData.dwDash&(1<<b) ? L'-' : L' ';
				sz[3+m_cData.nDashLen] = L'"';
				sz[4+m_cData.nDashLen] = L'\0';
				bstr += sz;
			}
			if (m_cData.eJoinMode != agg::bevel_join)
				bstr += m_cData.eJoinMode == agg::round_join ? L", \"JROUND\"" : L", \"JMITER\"";
			if (m_cData.eCapMode != agg::square_cap)
				bstr += m_cData.eCapMode == agg::butt_cap ? L", \"CBUTT\"" : L", \"CROUND\"";

			if (m_cPath.size() >= 2)
			{
				for (CPath::const_iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
				{
					swprintf(sz, bstr == NULL ? L"\"%c\", %g, %g, %g, %g, %g, %g" : L", \"%c\", %g, %g, %g, %g, %g, %g", (i->dwFlags&EPPClose) ? ((i->dwFlags&EPPRealClose) == EPPRealClose ? L'O' : L'E') : ((i->dwFlags&EPPSharp) ? L'S' : L'C'), i->tPos.fX, i->tPos.fY, i->tTanNext.fX, i->tTanNext.fY, i->tTanPrev.fX, i->tTanPrev.fY);
					bstr += sz;
				}
			}
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
		if (a_pConsumer == NULL)
			return E_POINTER;

		if (m_cPath.size() < 2)
			return S_FALSE; // invalid polygon

		try
		{
			std::vector<std::pair<std::vector<TPixelCoords>, bool> > cOriginal;
			GetAsPolygon(cOriginal);
			if (cOriginal.empty())
				return S_FALSE;
			CAutoVectorPtr<TRWPolygon> polys(new TRWPolygon[cOriginal.size()]);
			for (size_t i = 0; i < cOriginal.size(); ++i)
			{
				polys[i].nVertices = cOriginal[i].first.size();
				polys[i].pVertices = &(cOriginal[i].first[0]);
			}
			return a_pConsumer->FromPolygon(cOriginal.size(), polys);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		if (a_nCount == 0)
			return S_FALSE;
		if (a_pPaths == NULL)
			return E_POINTER;
		try
		{
			ULONG n = 0;
			for (ULONG i = 0; i < a_nCount; ++i)
				n += a_pPaths[i].nVertices;
			m_cPath.resize(n);
			m_iInsert = m_cPath.end();
			n = 0;
			for (ULONG i = 0; i < a_nCount; ++i)
			{
				std::copy(a_pPaths[i].pVertices, a_pPaths[i].pVertices+a_pPaths[i].nVertices, m_cPath.begin()+n);
				m_cPath[n+a_pPaths[i].nVertices-1].dwFlags = (a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].dwFlags&1) ?  EPPRealClose : EPPClose;
				n += a_pPaths[i].nVertices;
			}
			EnableBrush(true);
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
			ToolSetBrush();
			RECT rc = UpdateCache();
			M_Window()->RectangleChanged(&rc);
			m_cData.eState = CEditToolDataStroke::EESPassive;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;

		if (m_cPath.size() < 2)
			return S_FALSE; // invalid polygon

		try
		{
			std::vector<std::vector<TPixelCoords> > cOriginal;
			GetAsShape(cOriginal);
			if (cOriginal.empty())
				return S_FALSE;

			CPath cPL;
			for (std::vector<std::vector<TPixelCoords> >::const_iterator i = cOriginal.begin(); i != cOriginal.end(); ++i)
			{
				if (i->empty())
					continue;
				std::vector<TVector2d> pts;
				TPixelCoords t = *i->begin();
				pts.push_back(t);
				for (std::vector<TPixelCoords>::const_iterator j = i->begin()+1; j != i->end(); ++j)
				{
					TPixelCoords s = *j;
					if (s.fX != t.fX || s.fY != t.fY)
					{
						pts.push_back(s);
						t = s;
					}
				}
				if (pts.size() > 1)
					Bezier::FitCurve(pts, 0, pts.size(), 0.5, CFitCurveConsumer(cPL));
			}


			std::vector<TRWPath> aPaths;
			TRWPath t;
			t.nVertices = 0;
			t.pVertices = NULL;
			for (CPath::const_iterator i = cPL.begin(); i != cPL.end(); ++i)
			{
				if (t.pVertices == NULL)
				{
					if ((i->dwFlags&EPPClose) == 0)
					{
						t.pVertices = &(*i);
						t.nVertices = 1;
					}
				}
				else
				{
					++t.nVertices;
					if ((i->dwFlags&EPPClose) == EPPClose)
					{
						if (t.nVertices > 1)
						{
							aPaths.push_back(t);
						}
						t.nVertices = 0;
						t.pVertices = NULL;
					}
				}
			}
			if (t.nVertices > 1)
			{
				aPaths.push_back(t);
			}
			if (aPaths.empty())
				return S_FALSE;
			return a_pConsumer->FromPath(aPaths.size(), &aPaths[0]);

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IEditToolLine methods
public:
	STDMETHOD(IsClosed)() { return E_NOTIMPL; }
	STDMETHOD(DashPattern)(BSTR* a_pbstrPattern)
	{
		*a_pbstrPattern = NULL;
		if (!m_cData.bUseDash || m_cData.nDashLen <= 1)
			return S_FALSE;
		wchar_t sz[33] = L"";
		for (BYTE b = 0; b < m_cData.nDashLen; ++b)
			sz[b] = m_cData.dwDash&(1<<b) ? L'-' : L' ';
		sz[m_cData.nDashLen] = L'\0';
		*a_pbstrPattern = ::SysAllocString(sz);
		return S_OK;
	}
	STDMETHOD(JoinMode)(ELineJoinMode* a_pMode) { *a_pMode = static_cast<ELineJoinMode>(m_cData.eJoinMode); return S_OK; }
	STDMETHOD(CapMode)(ELineCapMode* a_pMode) { *a_pMode = static_cast<ELineCapMode>(m_cData.eCapMode); return S_OK; }
	STDMETHOD(Width)(float* a_pWidth) { *a_pWidth = m_cData.fWidth; return S_OK; }
	STDMETHOD(Polygon)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;

		if (m_cPath.size() < 2)
			return S_FALSE; // invalid polygon

		try
		{
			std::vector<TRWPath> aPaths;
			TRWPath t;
			t.nVertices = 0;
			t.pVertices = NULL;
			for (CPath::const_iterator i = m_cPath.begin(); i != m_cPath.end(); ++i)
			{
				if (t.pVertices == NULL)
				{
					if ((i->dwFlags&EPPClose) == 0)
					{
						t.pVertices = &(*i);
						t.nVertices = 1;
					}
				}
				else
				{
					++t.nVertices;
					if ((i->dwFlags&EPPClose) == EPPClose)
					{
						if (t.nVertices > 1)
						{
							aPaths.push_back(t);
						}
						t.nVertices = 0;
						t.pVertices = NULL;
					}
				}
			}
			if (t.nVertices > 1)
			{
				aPaths.push_back(t);
			}
			if (aPaths.empty())
				return S_FALSE;
			return a_pConsumer->FromPath(aPaths.size(), &aPaths[0]);

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
		//HRESULT hRes = ToPath(a_pConsumer);
		//return hRes == E_NOTIMPL ? ToPolygon(a_pConsumer) : hRes;
	}


	// IRasterImageEditToolContextMenu methods
public:
	typedef HRESULT (CEditToolStroke::*CtxMenuHandler)(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex);

	class ATL_NO_VTABLE CMenuItem :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CMenuItem() : m_pTool(NULL)
		{
		}
		~CMenuItem()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolStroke* a_pTool, wchar_t const* a_pszName, wchar_t const* a_pszDesc, CtxMenuHandler a_pfnHandler, TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
		{
			(m_pTool = a_pTool)->AddRef();
			m_pszName = a_pszName;
			m_pszDesc = a_pszDesc;
			m_pfnHandler = a_pfnHandler;
			m_pPixel = a_pPixel;
			m_nControlPointIndex = a_nControlPointIndex;
		}

	BEGIN_COM_MAP(CMenuItem)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = NULL;
				if (m_pszName == NULL)
					return E_NOTIMPL;
				*a_ppText = new CMultiLanguageString(m_pszName);
				return S_OK;
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = NULL;
				if (m_pszDesc == 0)
					return E_NOTIMPL;
				*a_ppText = new CMultiLanguageString(m_pszDesc);
				return S_OK;
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				//if (t_pIconID == 0)
					return E_NOTIMPL;
				//*a_pIconID = *t_pIconID;
				//return S_OK;
			}
			catch (...)
			{
				return a_pIconID ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				*a_phIcon = NULL;
				//if (t_uIDIcon == 0)
					return E_NOTIMPL;

				//*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(t_uIDIcon), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
				//return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				*a_peState = EMCSNormal;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return (m_pTool->*m_pfnHandler)(m_pPixel, m_nControlPointIndex);
		}

	private:
		CEditToolStroke* m_pTool;
		wchar_t const* m_pszName;
		wchar_t const* m_pszDesc;
		CtxMenuHandler m_pfnHandler;
		TPixelCoords const* m_pPixel;
		ULONG m_nControlPointIndex;
	};

	HRESULT EndPath(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		m_cPath[m_cPath.size()-1].dwFlags = EPPClose;
		m_cData.eState = CEditToolDataStroke::EESPassive;
		M_Window()->ControlLinesChanged();
		return S_OK;
	}
	HRESULT ClosePath(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		m_cPath[m_cPath.size()-1].dwFlags = EPPClose|EPPRealClose;
		m_cData.eState = CEditToolDataStroke::EESPassive;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		return S_OK;
	}
	HRESULT StartNewPath(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		if (!m_cPath.empty())
			m_cPath[m_cPath.size()-1].dwFlags |= EPPClose;
		m_iInsert = m_cPath.end();
		m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		M_Window()->ControlLinesChanged();
		return S_OK;
	}
	HRESULT RemoveVertex(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;
		ULONG nNx = NextPoint(nPt);
		if (nNx != PrevPoint(nPt))
		{
			if (m_cPath[nPt].dwFlags&EPPClose)
				m_cPath[nPt-1].dwFlags |= m_cPath[nPt].dwFlags&EPPRealClose;
			size_t index = m_iInsert-m_cPath.begin();
			m_cPath.erase(m_cPath.begin()+nPt);
			m_iInsert = m_cPath.begin()+(nPt < index ? index-1 : index);
		}
		else
		{
			// erase whole path
			m_cPath.erase(m_cPath.begin()+min(nNx, nPt), m_cPath.begin()+max(nNx, nPt)+1);
			m_iInsert = m_cPath.end();
		}
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		if (m_cPath.empty())
			m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		return S_OK;
	}
	HRESULT SmoothenVertex(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;
		ULONG nNx = NextPoint(nPt);
		ULONG nPr = PrevPoint(nPt);
		float fDir =
			m_cPath[nPr].tPos.fX*m_cPath[nPt].tPos.fY-m_cPath[nPr].tPos.fY*m_cPath[nPt].tPos.fX +
			m_cPath[nPt].tPos.fX*m_cPath[nNx].tPos.fY-m_cPath[nPt].tPos.fY*m_cPath[nNx].tPos.fX +
			m_cPath[nNx].tPos.fX*m_cPath[nPr].tPos.fY-m_cPath[nNx].tPos.fY*m_cPath[nPr].tPos.fX;
		if (fDir < 0)
			fDir = 0.4f;
		else
			fDir = -0.4f;
		float fX = (m_cPath[nNx].tPos.fX+m_cPath[nPr].tPos.fX)*0.5f-m_cPath[nPt].tPos.fX;
		float fY = (m_cPath[nNx].tPos.fY+m_cPath[nPr].tPos.fY)*0.5f-m_cPath[nPt].tPos.fY;
		m_cPath[nPt].tTanNext.fX = -fDir*fY;
		m_cPath[nPt].tTanNext.fY = fDir*fX;
		m_cPath[nPt].tTanPrev.fX = fDir*fY;
		m_cPath[nPt].tTanPrev.fY = -fDir*fX;
		m_cPath[nPt].dwFlags &= ~EPPSharp;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		if (m_cPath.empty())
			m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		return S_OK;
	}
	HRESULT SharpenVertex(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;
		m_cPath[nPt].dwFlags |= EPPSharp;
		M_Window()->ControlPointsChanged();
		return S_OK;
	}
	HRESULT RemoveTangent(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;
		if (a_nControlPointIndex-nPt*CPStep == CPTanNext)
		{
			m_cPath[nPt].tTanNext.fX = m_cPath[nPt].tTanNext.fY = 0.0f;
		}
		else
		{
			m_cPath[nPt].tTanPrev.fX = m_cPath[nPt].tTanPrev.fY = 0.0f;
		}
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		return S_OK;
	}
	bool FindTangents(ULONG a_nPt, TPixelCoords* a_pPrev, TPixelCoords* a_pNext, TPixelCoords* a_pIntersection)
	{
		TRWPathPoint const& s = m_cPath[a_nPt];
		if (s.tTanNext.fX != 0.0f || s.tTanNext.fY != 0.0f)
		{
			*a_pPrev = s.tTanNext;
		}
		else if (s.tTanPrev.fX != 0.0f || s.tTanPrev.fY != 0.0f)
		{
			a_pPrev->fX = -s.tTanNext.fX;
			a_pPrev->fY = -s.tTanNext.fY;
		}
		else
		{
			TRWPathPoint const& e = m_cPath[PrevPoint(a_nPt)];
			if ((e.tPos.fX == s.tPos.fX && e.tPos.fY == s.tPos.fY) ||
				e.tTanNext.fX != 0.0f || e.tTanNext.fY != 0.0f)
				return false;
			a_pPrev->fX = s.tPos.fX-e.tPos.fX;
			a_pPrev->fY = s.tPos.fY-e.tPos.fY;
		}

		TRWPathPoint const& t = m_cPath[NextPoint(a_nPt)];
		if (t.tTanPrev.fX != 0.0f || t.tTanPrev.fY != 0.0f)
		{
			*a_pNext = t.tTanPrev;
		}
		else if (t.tTanNext.fX != 0.0f || t.tTanNext.fY != 0.0f)
		{
			a_pNext->fX = -t.tTanNext.fX;
			a_pNext->fY = -t.tTanNext.fY;
		}
		else
		{
			TRWPathPoint const& e = m_cPath[NextPoint(NextPoint(a_nPt))];
			if ((e.tPos.fX == t.tPos.fX && e.tPos.fY == t.tPos.fY) ||
				e.tTanPrev.fX != 0.0f || e.tTanPrev.fY != 0.0f)
				return false;
			a_pNext->fX = t.tPos.fX-e.tPos.fX;
			a_pNext->fY = t.tPos.fY-e.tPos.fY;
		}

		float denom = (a_pNext->fY*a_pPrev->fX) - (a_pNext->fX*a_pPrev->fY);
		float nume_a = (a_pNext->fX*(s.tPos.fY - t.tPos.fY)) - (a_pNext->fY*(s.tPos.fX - t.tPos.fX));
		float nume_b = (a_pPrev->fX*(s.tPos.fY - t.tPos.fY)) - (a_pPrev->fY*(s.tPos.fX - t.tPos.fX));

		if (denom == 0.0f)
			return false;

		float ua = nume_a / denom;
		float ub = nume_b / denom;

		if (ua <= 0.0f || ub <= 0.0f)
			return false;

		a_pIntersection->fX = s.tPos.fX + ua*a_pPrev->fX;
		a_pIntersection->fY = s.tPos.fY + ua*a_pPrev->fY;

		return true;
	}
	float TangentAngle(TPixelCoords const* a_pPrev, TPixelCoords const* a_pNext)
	{
		float f1 = atan2f(a_pPrev->fY, a_pPrev->fX);
		float f2 = atan2f(-a_pNext->fY, -a_pNext->fX);
		//if (f2 < f1)
		//	f2 += 6.2831853f;
		float f = f2-f1;
		if (f > 3.1415926536f)
			f -= 6.2831853f;
		else if (f < -3.1415926536f)
			f += 6.2831853f;
		return f;
	}
	HRESULT SectionToArc(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;
		TPixelCoords tPrev;
		TPixelCoords tNext;
		TPixelCoords tIntersection;
		float fAngle;
		if (!FindTangents(a_nControlPointIndex/CPStep, &tPrev, &tNext, &tIntersection) || (fAngle = fabsf(TangentAngle(&tPrev, &tNext))) > 2.0f/*1.5707963268f*/)
			return E_FAIL;
		TRWPathPoint& s = m_cPath[nPt];
		TRWPathPoint& t = m_cPath[NextPoint(nPt)];
		float fAx = tIntersection.fX-s.tPos.fX;
		float fBx = tIntersection.fX-t.tPos.fX;
		float fCx = s.tPos.fX+t.tPos.fX-tIntersection.fX;
		float fAy = tIntersection.fY-s.tPos.fY;
		float fBy = tIntersection.fY-t.tPos.fY;
		float fCy = s.tPos.fY+t.tPos.fY-tIntersection.fY;
		s.tTanNext.fX = 0.5522847498f*fAx+fBx+fCx-s.tPos.fX;
		s.tTanNext.fY = 0.5522847498f*fAy+fBy+fCy-s.tPos.fY;
		t.tTanPrev.fX = fAx+0.5522847498f*fBx+fCx-t.tPos.fX;
		t.tTanPrev.fY = fAy+0.5522847498f*fBy+fCy-t.tPos.fY;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		return S_OK;
	}
	HRESULT SectionToLine(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;

		TRWPathPoint& s = m_cPath[nPt];
		TRWPathPoint& t = m_cPath[NextPoint(nPt)];
		s.tTanNext.fX = s.tTanNext.fY = t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		return S_OK;
	}
	HRESULT RemoveSegment(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		ULONG nEnd = nPt+1;
		while (nEnd < m_cPath.size() && (m_cPath[nEnd-1].dwFlags&EPPClose) == 0) ++nEnd;
		ULONG nBegin = nEnd-1;
		while (nBegin > 0 && (m_cPath[nBegin-1].dwFlags&EPPClose) == 0) --nBegin;
		m_cPath[nEnd-1].dwFlags &= ~EPPRealClose;
		if (nPt != nEnd-1)
			std::rotate(m_cPath.begin()+nBegin, m_cPath.begin()+nPt+1, m_cPath.begin()+nEnd);
		m_cPath[nEnd-1].dwFlags |= EPPClose;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		PolygonChanged();
		return S_OK;
	}
	HRESULT AddPointsHere(TPixelCoords const* UNREF(a_pPixel), ULONG a_nControlPointIndex)
	{
		ULONG nPt = a_nControlPointIndex/CPStep;
		if (nPt >= m_cPath.size())
			return E_FAIL;

		if ((a_nControlPointIndex%CPStep) == CPPos)
		{
			if (nPt+1 == m_cPath.size())
			{
				m_iInsert = m_cPath.begin()+nPt+1;
				m_cPath[nPt].dwFlags = 0;
			}
			else if (nPt == 0 || (m_cPath[nPt-1].dwFlags&EPPClose))
			{
				CPath::iterator i1 = m_cPath.begin()+nPt;
				CPath::iterator i2 = i1;
				while (i2 != m_cPath.end() && (i2->dwFlags&EPPClose) == 0) ++i2;
				if (i2 != m_cPath.end()) ++i2;
				std::reverse(i1, i2);
				for (CPath::iterator i = i1; i != i2; ++i)
					std::swap(i->tTanNext, i->tTanPrev);
				m_iInsert = i2;
				i1->dwFlags = 0;
				if (i2 != m_cPath.end())
				{
					--i2;
					i2->dwFlags |= m_cPath[nPt-1].dwFlags&EPPRealClose;
				}
			}
			else if ((m_cPath[nPt].dwFlags&EPPClose))
			{
				m_iInsert = m_cPath.begin()+nPt+1;
			}
		}
		else
		{
			m_iInsert = m_cPath.begin()+nPt+1;
		}
		m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		return S_OK;

		//ULONG nPathBegin = nPt;
		//while (nPathBegin && (m_cPath[nPathBegin-1].dwFlags&EPPClose) == 0)
		//	--nPathBegin;
		//ULONG nPathEnd = nPt+1;
		//while (nPathEnd < m_cPath.size() && (m_cPath[nPathEnd-1].dwFlags&EPPClose) == 0)
		//	++nPathEnd;
		//m_cPath[m_cPath.size()-1].dwFlags |= EPPClose;
		//std::rotate(m_cPath.begin(), m_cPath.begin()+(nPathEnd%m_cPath.size()), m_cPath.end());
		//m_cPath[m_cPath.size()-1].dwFlags &= ~EPPClose;
		//std::rotate(m_cPath.end()-(nPathEnd-nPathBegin), m_cPath.end()-(nPathEnd-nPathBegin)+((nPt-nPathBegin+1)%(nPathEnd-nPathBegin)), m_cPath.end());
		//m_cData.eState = CEditToolDataStroke::EESAddingPoints;
		//M_Window()->ControlLinesChanged();
		//M_Window()->ControlPointsChanged();
		//return S_OK;
	}
	void Reflect(TRWPathPoint& t, TPixelCoords const& v, TPixelCoords const& o)
	{
		float const f = 2.0f/(v.fX*v.fX+v.fY*v.fY);
		float const fN = f*(v.fX*t.tTanNext.fX+v.fY*t.tTanNext.fY);
		t.tTanNext.fX = fN*v.fX-t.tTanNext.fX;
		t.tTanNext.fY = fN*v.fY-t.tTanNext.fY;
		float const fP = f*(v.fX*t.tTanPrev.fX+v.fY*t.tTanPrev.fY);
		t.tTanPrev.fX = fP*v.fX-t.tTanPrev.fX;
		t.tTanPrev.fY = fP*v.fY-t.tTanPrev.fY;
		float const f2 = f*(v.fX*(t.tPos.fX-o.fX)+v.fY*(t.tPos.fY-o.fY));
		t.tPos.fX = f2*v.fX-t.tPos.fX+2*o.fX;
		t.tPos.fY = f2*v.fY-t.tPos.fY+2*o.fY;
		std::swap(t.tTanPrev, t.tTanNext);
	}
	class CClipboardHandler
	{
	public:
		CClipboardHandler(HWND a_hWnd)
		{
			if (!::OpenClipboard(a_hWnd))
				throw E_FAIL;
		}
		~CClipboardHandler()
		{
			::CloseClipboard();
		}
	};
	HRESULT CopySVGPath(TPixelCoords const* UNREF(a_pPixel), ULONG UNREF(a_nControlPointIndex))
	{
		if (m_cPath.size() < 2)
			return E_FAIL;

		CComBSTR bstr;
		CPath::const_iterator first = m_cPath.end();
		CPath::const_iterator i = m_cPath.begin();
		while (i != m_cPath.end())
		{
			if (first == m_cPath.end())
			{
				if (i->dwFlags&EPPClose)
					;
				else
					first = i;
			}
			else
			{
				if (i->dwFlags&EPPClose)
				{
					AddSVGPath(first, i+1, bstr);
					first = m_cPath.end();
				}
			}
			++i;
		}
		AddSVGPath(first, m_cPath.end(), bstr);

		if (bstr.Length() < 3)
			return E_FAIL;
		HWND hWnd = NULL;
		M_Window()->Handle(&hWnd);
		CClipboardHandler cCb(hWnd);
		EmptyClipboard();
		COLE2CT psz(bstr);
		DWORD nLen = _tcslen(psz)+1;
		HANDLE hCopy = GlobalAlloc(GMEM_MOVEABLE, nLen*sizeof TCHAR); 
		LPTSTR pszTmp = reinterpret_cast<LPTSTR>(GlobalLock(hCopy)); 
		memcpy(pszTmp, psz, nLen*sizeof TCHAR);
		GlobalUnlock(pszTmp);
		SetClipboardData(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT, hCopy);

		return S_OK;
	}
	HRESULT PasteSVGPath(TPixelCoords const* UNREF(a_pPixel), ULONG UNREF(a_nControlPointIndex))
	{
		RWHWND hWnd;
		M_Window()->Handle(&hWnd);
		CClipboardHandler cClipboard(hWnd);
		HANDLE hData = GetClipboardData(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT);
		if (hData)
		{
			LPCTSTR psz = reinterpret_cast<LPCTSTR>(GlobalLock(hData));
			CPath path;
			if (CEditToolPath::ParseSVGPath(psz, path) && path.size() > 1)
			{
				std::swap(path, m_cPath);
				m_iInsert = m_cPath.end();
				EnableBrush(true);
				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				m_cData.eState = CEditToolDataStroke::EESPassive;
			}
			GlobalUnlock(hData);
			m_iInsert = m_cPath.end();
		}
		return S_OK;
	}

	STDMETHOD(CommandsEnum)(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			//if (_IsDirty(NULL, NULL, NULL) != S_OK)
			//	return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			if (m_cData.eState == CEditToolDataStroke::EESAddingPoints)
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]End sub-path[0405]Ukončit část cesty", L"[0409]Stop adding vertices to the current path by mouse clicking.[0405]Ukončit přidávání vrcholů do současné cesty klikáním myší.", &CEditToolStroke::EndPath, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			if (m_cData.eState == CEditToolDataStroke::EESAddingPoints)
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]Close sub-path[0405]Uzavřít část cesty", L"[0409]Connect the ends of the current sub-path.[0405]Propojit konce aktuální části cesty.", &CEditToolStroke::ClosePath, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			//if (m_cData.eState == CEditToolDataStroke::EESPassive)
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]Start new sub-path[0405]Začít novou část cesty", L"[0409]Hole is created by positioning a new path inside an existing path.[0405]Díru vytvoříte umístěním nové cesty do existující cesty.", &CEditToolStroke::StartNewPath, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			bool addPointsHere = false;
			if (a_nControlPointIndex < m_cPath.size()*CPStep && (a_nControlPointIndex%CPStep) == CPPos)
			{
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Remove vertex[0405]Odstranit vrchol", L"[0409]Remove clicked vertex from path connecting two neighbour vertices.[0405]Odstranit vrchol z cesty a propojit sousedící vrcholy.", &CEditToolStroke::RemoveVertex, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}
				ULONG nPt = a_nControlPointIndex/CPStep;
				TRWPathPoint const& s = m_cPath[nPt];
				if ((s.tTanPrev.fX == 0.0f && s.tTanPrev.fY == 0.0f) || (s.tTanNext.fX == 0.0f && s.tTanNext.fY == 0.0f) || s.dwFlags&EPPSharp)
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Smoothen vertex[0405]Zahladit vrchol", L"[0409]Generate new, parallel tangents for the vertex.[0405]Nastavit vrcholu nové, paralelní tangenty.", &CEditToolStroke::SmoothenVertex, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}
				else if ((s.dwFlags&EPPSharp) == 0)
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Sharpen vertex[0405]Zostřit vrchol", L"[0409]Allow tangents of this vertex to be moved independently.[0405]Povolit upravovat tangenty vrcholu nezávisle.", &CEditToolStroke::SharpenVertex, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}

				if (nPt == 0 || (m_cPath[nPt-1].dwFlags&EPPClose))
					addPointsHere = true;
				else if (nPt+1 == m_cPath.size() || (m_cPath[nPt].dwFlags&EPPClose))
					addPointsHere = true;
			}
			if (a_nControlPointIndex < m_cPath.size()*CPStep && ((a_nControlPointIndex%CPStep) == CPTanNext || (a_nControlPointIndex%CPStep) == CPTanPrev))
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]Remove tangent[0405]Odstranit tečnu", L"[0409]Make a sharp corner by zeroing the tangent.[0405]Vytvořit ostrý roh vynulováním tečny.", &CEditToolStroke::RemoveTangent, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			if (a_nControlPointIndex < m_cPath.size()*CPStep && (a_nControlPointIndex%CPStep) == CPSegment)
			{
				TPixelCoords tPrev;
				TPixelCoords tNext;
				TPixelCoords tIntersection;
				if (IsSegmentClosed(a_nControlPointIndex/CPStep))
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Remove segment[0405]Odstranit segment", L"[0409]Replace the curve segment with an elliplical arc.[0405]Nahradit segment křivky obloukem elipsy.", &CEditToolStroke::RemoveSegment, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}
				if (FindTangents(a_nControlPointIndex/CPStep, &tPrev, &tNext, &tIntersection) && fabsf(TangentAngle(&tPrev, &tNext)) <= 2.0f/*1.5707963268f*/)
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Replace with arc[0405]Nahradit obloukem", L"[0409]Replace the curve segment with an elliplical arc.[0405]Nahradit segment křivky obloukem elipsy.", &CEditToolStroke::SectionToArc, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}

				TRWPathPoint const& s = m_cPath[a_nControlPointIndex/CPStep];
				TRWPathPoint const& t = m_cPath[NextPoint(a_nControlPointIndex/CPStep)];
				if (s.tTanNext.fX != 0.0f || s.tTanNext.fY != 0.0f || t.tTanPrev.fX != 0.0f || t.tTanPrev.fY != 0.0f)
				{
					CComObject<CMenuItem>* p = NULL;
					CComObject<CMenuItem>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;
					p->Init(this, L"[0409]Replace with line[0405]Nahradit úsečkou", L"[0409]Replace the curve segment with a straight line.[0405]Nahradit segment křivky rovnou linkou.", &CEditToolStroke::SectionToLine, a_pPixel, a_nControlPointIndex);
					pItems->Insert(pTmp);
				}

				if (m_cData.eState == CEditToolDataStroke::EESPassive || a_nControlPointIndex < (m_cPath.size()-1)*CPStep)
					addPointsHere = true;
			}
			if (addPointsHere)
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]Add points here[0405]Přidávat body sem", L"[0409]Start adding new points to the path from here.[0405]Začít přidávat nové body do křivky odtud.", &CEditToolStroke::AddPointsHere, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			if (m_cPath.size() > 1)
			{
				CComObject<CMenuItem>* p = NULL;
				CComObject<CMenuItem>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, L"[0409]Copy as SVG path[0405]Kopírovat jako SVG cestu", L"[0409]Copy the shape to clipboard as text in the SVG path element format.[0405]Kopírovat tvar do schránky ve formátu SVG elementu path.", &CEditToolStroke::CopySVGPath, a_pPixel, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			if (IsClipboardFormatAvailable(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT))
			{
				RWHWND hWnd;
				M_Window()->Handle(&hWnd);
				CClipboardHandler cClipboard(hWnd);
				HANDLE hData = GetClipboardData(sizeof(TCHAR) == sizeof(char) ? CF_TEXT : CF_UNICODETEXT);
				if (hData)
				{
					LPCTSTR psz = reinterpret_cast<LPCTSTR>(GlobalLock(hData));
					CPath path;
					if (CEditToolPath::ParseSVGPath(psz, path) && path.size() > 1)
					{
						CComObject<CMenuItem>* p = NULL;
						CComObject<CMenuItem>::CreateInstance(&p);
						CComPtr<IDocumentMenuCommand> pTmp = p;
						p->Init(this, L"[0409]Paste SVG path[0405]Vložit SVG cestu", L"[0409]Copy the shape to clipboard as text in the SVG path element format.[0405]Kopírovat tvar do schránky ve formátu SVG elementu path.", &CEditToolStroke::PasteSVGPath, a_pPixel, a_nControlPointIndex);
						pItems->Insert(pTmp);
					}
					GlobalUnlock(hData);
				}
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	size_t NextPoint(size_t a_i)
	{
		if ((m_cPath[a_i].dwFlags&EPPClose) == 0 && a_i+1 < m_cPath.size())
			return a_i+1;
		if (m_cPath.size() < 2)
			return 0;
		while (a_i > 0 && (m_cPath[a_i-1].dwFlags&EPPClose) == 0) --a_i;
		return a_i;
	}
	size_t PrevPoint(size_t a_i)
	{
		if (a_i > 0 && (m_cPath[a_i-1].dwFlags&EPPClose) == 0)
			return a_i-1;
		if (m_cPath.size() < 2)
			return 0;
		while (a_i < m_cPath.size()-1 && (m_cPath[++a_i].dwFlags&EPPClose) == 0);
		return a_i;
	}
	bool IsSegmentClosed(size_t a_i)
	{
		do
		{
			if ((m_cPath[a_i].dwFlags&EPPRealClose) == EPPRealClose)
				return true;
		}
		while (++a_i < m_cPath.size() && (m_cPath[a_i].dwFlags&EPPRealClose) != EPPClose);
		return false;
	}

private:
	enum EPathPoint
	{
		EPPClose = 1,
		EPPSharp = 2,
		EPPRealClose = 9,
	};
	typedef std::vector<TRWPathPoint> CPath;

	enum
	{
		CPSegment = 0,
		CPTanPrev = 1,
		CPTanNext = 2,
		CPPos = 3,
		CPStep = 4,
	};

	struct SPathPoint
	{
		SPathPoint(TPixelCoords a_t, DWORD a_dw) : tPos(a_t), dwTime(a_dw) {}
		TPixelCoords tPos;
		DWORD dwTime;
	};
	typedef std::vector<SPathPoint> CPathPoints;

	struct CFitCurveConsumer
	{
		CPath& path;
		bool first;
		TRWPathPoint point;
		CFitCurveConsumer(CPath& path) : path(path), first(true) { point.dwFlags = 0; }
		void add(TVector2d const* curve)
		{
			if (first)
			{
				TRWPathPoint t;
				t.tPos = curve[0];
				t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
				t.dwFlags = 0;
				t.tTanNext = curve[1]-curve[0];
				path.push_back(t);
				first = false;
			}
			else
			{
				point.tTanNext = curve[1]-curve[0];
				path.push_back(point);
			}
			point.tPos = curve[3];
			point.tTanPrev = curve[2]-curve[3];
		}
		~CFitCurveConsumer()
		{
			if (!first)
			{
				point.tTanNext.fX = point.tTanNext.fY = 0.0f;
				path.push_back(point);
			}
		}
	};

	void AddPathPart(CPathPoints::const_iterator first, CPathPoints::const_iterator last, CPath& a_out)
	{
		if (first == last)
			return;
		CPathPoints::const_iterator prev = first;
		++first;
		std::vector<TVector2d> pts;
		pts.push_back(prev->tPos);
		while (first != last)
		{
			if (prev->tPos.fX != first->tPos.fX || prev->tPos.fY != first->tPos.fY)
				pts.push_back(first->tPos);
			prev = first;
			++first;
		}
		if (pts.size() > 1)
			Bezier::FitCurve(pts, 0, pts.size(), M_PointSize()*m_cData.fSmoothing*m_cData.fSmoothing*20, CFitCurveConsumer(a_out));
		//std::vector<TVector2d> pts2;
		//Bezier::Decimate(pts, M_PointSize()*m_cData.fSmoothing, pts2);
		//std::vector<TVector2d> bz;
		//Bezier::ToBeziers(pts2, bz);
		//if (bz.size() < 4)
		//	return;
		//size_t iDst = a_out.size()-1;
		//a_out.resize(iDst+1+(bz.size()-1)/3);
		//for (size_t i = 0; i+3 < bz.size(); i += 3)
		//{
		//	a_out[iDst].tPos = bz[i];
		//	a_out[iDst].tTanNext = bz[i+1]-bz[i];
		//	++iDst;
		//	a_out[iDst].tPos = bz[i+3];
		//	a_out[iDst].tTanPrev = bz[i+2]-bz[i+3];
		//}
	}
	size_t SplitPath(CPath& cPath, size_t nArc, float fSplit)
	{
		if (fSplit <= 0.0f)
			return nArc;
		size_t ret = (nArc+1)%cPath.size();
		if (fSplit >= 1.0f)
			return ret;
		CPath::iterator prev = cPath.begin()+nArc;
		CPath::iterator cur = cPath.begin()+((nArc+1)%cPath.size());
		Bezier b(prev->tPos.fX, prev->tPos.fY, prev->tPos.fX+prev->tTanNext.fX, prev->tPos.fY+prev->tTanNext.fY, cur->tPos.fX+cur->tTanPrev.fX, cur->tPos.fY+cur->tTanPrev.fY, cur->tPos.fX, cur->tPos.fY);
		Bezier nb1(3);
		Bezier nb2(3);
		b.Evaluate(fSplit, &nb1, &nb2);
		TRWPathPoint newPt;
		newPt.dwFlags = 0;
		newPt.tPos.fX = nb2.m_pts[0].x;
		newPt.tPos.fY = nb2.m_pts[0].y;
		newPt.tTanNext.fX = nb2.m_pts[1].x-nb2.m_pts[0].x;
		newPt.tTanNext.fY = nb2.m_pts[1].y-nb2.m_pts[0].y;
		newPt.tTanPrev.fX = nb1.m_pts[2].x-nb1.m_pts[3].x;
		newPt.tTanPrev.fY = nb1.m_pts[2].y-nb1.m_pts[3].y;
		prev->tTanNext.fX = nb1.m_pts[1].x-nb1.m_pts[0].x;
		prev->tTanNext.fY = nb1.m_pts[1].y-nb1.m_pts[0].y;
		cur->tTanPrev.fX = nb2.m_pts[2].x-nb2.m_pts[3].x;
		cur->tTanPrev.fY = nb2.m_pts[2].y-nb2.m_pts[3].y;
		if (cur == cPath.begin())
		{
			cPath.push_back(newPt);
			return nArc+1;
		}
		cPath.insert(cur, newPt);
		return ret;
	}
	template<class VertexSource> 
    double path_length(VertexSource& vs)
    {
		double l = 0.0f;
        double xx;
        double yy;
        double x;
        double y;
        bool first = true;

        vs.rewind(0);
        unsigned cmd;
		while (!agg::is_stop(cmd = vs.vertex(&x, &y)))
        {
            if (agg::is_vertex(cmd))
            {
                if (first)
                {
                    xx = x;
                    yy = y;
                    first = false;
                }
                else
                {
					l += sqrt((xx-x)*(xx-x)+(yy-y)*(yy-y));
                }
            }
        }
        return l;
    }
	double PathLength(CPath::const_iterator first, CPath::const_iterator last)
	{
		double l = 0.0f;
		if (first == last)
			return l;
		CPath::const_iterator prev = first;
		++first;
		while (first != last)
		{
			agg::curve4 c(
				prev->tPos.fX, prev->tPos.fY,
				prev->tPos.fX+prev->tTanNext.fX, prev->tPos.fY+prev->tTanNext.fY,
				first->tPos.fX+first->tTanPrev.fX, first->tPos.fY+first->tTanPrev.fY,
				first->tPos.fX, first->tPos.fY);
			l += path_length(c);
			prev = first;
			++first;
		}
		return l;
	}
	void AppendPath(CPath& p1, CPath const& p2)
	{
		if (p2.size() < 2)
			return;
		if (p1.size() < 2)
		{
			p1 = p2;
			return;
		}
		p1[0].tTanPrev = p2[p2.size()-1].tTanPrev;
		p1[p1.size()-1].tTanNext = p2[0].tTanNext;
		p1.insert(p1.end(), p2.begin()+1, p2.end()-1);
	}
	void ProcessPath(DWORD a_dwTime, float a_fScale)
	{
		if (m_cPathPoints.empty())
			return;

		TPixelCoords endPt2;
		CPath::const_iterator endArc;
		std::vector<CPath>::const_iterator endSeg;
		float endParam;
		TPixelCoords endPt = m_cPathPoints[m_cPathPoints.size()-1].tPos;
		float endDist = !m_cPrevPaths.empty() && m_iPrevSegment != m_cPrevPaths.end() ? ClosestPoint(m_cPrevPaths.begin(), m_cPrevPaths.end(), endPt, &endPt2, &endSeg, &endArc, &endParam) : -1.0f;

		CPathPoints extra;
		CPathPoints::const_iterator iPrev = m_cPathPoints.begin();
		CPathPoints::const_iterator iEnd = m_cPathPoints.end();
		if ((!m_cPrevPaths.empty() && m_iPrevSegment != m_cPrevPaths.end()) || (endDist >= 0.0f && endDist <= M_PointSize()*32))
		{
			if (!m_cPrevPaths.empty() && m_iPrevSegment != m_cPrevPaths.end())
				extra.push_back(SPathPoint(m_tPrevCoords, m_cPathPoints[0].dwTime-50));
			extra.insert(extra.end(), m_cPathPoints.begin(), m_cPathPoints.end());
			if (endDist >= 0.0f && endDist <= M_PointSize()*32)
				extra.push_back(SPathPoint(endPt2, m_cPathPoints[m_cPathPoints.size()-1].dwTime+50));
			iPrev = extra.begin();
			iEnd = extra.end();
		}

		CPath cPL;
		TRWPathPoint t;
		t.tPos = iPrev->tPos;
		t.tTanNext.fX = t.tTanNext.fY = 0.0f;
		t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
		t.dwFlags = 0;
		cPL.push_back(t);
		CPathPoints::const_iterator iStart = iPrev;
		for (CPathPoints::const_iterator i = iPrev++; i != iEnd; iPrev = i++)
		{
			if (i->dwTime-iPrev->dwTime > 150UL)
			{
				if (cPL[cPL.size()-1].tPos.fX != i->tPos.fX || cPL[cPL.size()-1].tPos.fY != i->tPos.fY)
				{
					AddPathPart(iStart, i, cPL);
					iStart = i;
				}
			}
		}
		if (iStart != iEnd)
		{
			AddPathPart(iStart, iEnd, cPL);
		}

		if (cPL.size() > 4)
		{
			// try to smoothen the start/end point
			TRWPathPoint& t1 = cPL[0];
			TRWPathPoint const& t2 = cPL[cPL.size()-1];
			float distSq = (t1.tPos.fX-t2.tPos.fX)*(t1.tPos.fX-t2.tPos.fX) + (t1.tPos.fY-t2.tPos.fY)*(t1.tPos.fY-t2.tPos.fY);
			if (distSq < a_fScale*4096)
			{
				t1.tTanPrev = t2.tTanPrev;

				float f1 = sqrtf(t1.tTanNext.fX*t1.tTanNext.fX + t1.tTanNext.fY*t1.tTanNext.fY);
				float f2 = sqrtf(t2.tTanPrev.fX*t2.tTanPrev.fX + t2.tTanPrev.fY*t2.tTanPrev.fY);
				if (f1 > 0.0f && f2 > 0.0f)
				{
					TPixelCoords v1 = {t1.tTanNext.fX/f1, t1.tTanNext.fY/f1};
					TPixelCoords v2 = {t2.tTanPrev.fX/f2, t2.tTanPrev.fY/f2};
					if (v1.fX*v2.fX+v1.fY*v2.fY < -0.6f)
					{
						t1.tPos.fX = (t1.tPos.fX+t2.tPos.fX)*0.5f;
						t1.tPos.fY = (t1.tPos.fY+t2.tPos.fY)*0.5f;
						t1.tTanPrev.fX = (t2.tTanPrev.fX-t1.tTanNext.fX)*0.5f;
						t1.tTanPrev.fY = (t2.tTanPrev.fY-t1.tTanNext.fY)*0.5f;
						t1.tTanNext.fX = (t1.tTanNext.fX-t2.tTanPrev.fX)*0.5f;
						t1.tTanNext.fY = (t1.tTanNext.fY-t2.tTanPrev.fY)*0.5f;
					}
					//else
					//{
					//	t1.tTanPrev.fX = -f2/f1*t1.tTanNext.fX;
					//	t1.tTanPrev.fY = -f2/f1*t1.tTanNext.fY;
					//}
				}
				cPL.resize(cPL.size()-1);
			}
		}

		m_cPath.clear();
		MergePaths(m_cPrevPaths.begin(), m_cPrevPaths.end(), m_cPath);
		MergePaths(&cPL, (&cPL)+1, m_cPath);
		m_iInsert = m_cPath.end();
	}
	bool ClosestPoint(TPixelCoords p1, float scale, TPixelCoords* p2)
	{
		std::vector<CPath> paths;
		SplitPaths(m_cPath, paths);
		TPixelCoords bestPt;
		std::vector<CPath>::const_iterator part;
		CPath::const_iterator segment;
		float splitPos;
		float best = ClosestPoint(paths.begin(), paths.end(), p1, &bestPt, &part, &segment, &splitPos);
		if (best >= 0.0f && best <= M_PointSize()*32)
		{
			if (p2)
				*p2 = bestPt;
			return true;
		}
		return false;
	}
	static float ClosestPoint(std::vector<CPath>::const_iterator first, std::vector<CPath>::const_iterator last, TPixelCoords p1, TPixelCoords* p2, std::vector<CPath>::const_iterator* part, CPath::const_iterator* segment, float* splitPos)
	{
		if (first == last)
			return -1.0f;

		float r = -1.0f;
		for (std::vector<CPath>::const_iterator i = first; i != last; ++i)
		{
			TPixelCoords tempP2;
			CPath::const_iterator tempSegment;
			float tempSplitPos;
			float f = ClosestPoint(i->begin(), i->end(), p1, &tempP2, &tempSegment, &tempSplitPos);
			if (r < 0.0f || f < r)
			{
				*p2 = tempP2;
				*part = i;
				*segment = tempSegment;
				*splitPos = tempSplitPos;
				r = f;
			}
		}
		return r;
	}
	static float ClosestPoint(CPath::const_iterator first, CPath::const_iterator last, TPixelCoords p1, TPixelCoords* p2, CPath::const_iterator* segment, float* splitPos)
	{
		if (first == last)
			return -1.0f;
		CPath::const_iterator cur = first;
		double r = -1.0f;
		do
		{
			CPath::const_iterator prev = cur;
			++cur;
			if (cur == last)
				cur = first;
			Bezier b(prev->tPos.fX, prev->tPos.fY, prev->tPos.fX+prev->tTanNext.fX, prev->tPos.fY+prev->tTanNext.fY, cur->tPos.fX+cur->tTanPrev.fX, cur->tPos.fY+cur->tTanPrev.fY, cur->tPos.fX, cur->tPos.fY);
			double split = b.NearestPointOnCurve(TVector2d(p1.fX, p1.fY));
			TVector2d tPt = b.Evaluate(split);
			double distSq = tPt.DistanceSquared(p1);
			if (distSq < r || r < 0)
			{
				r = distSq;
				*segment = prev;
				*splitPos = split;
				p2->fX = tPt.x;
				p2->fY = tPt.y;
			}

		}
		while (first != cur);
		return sqrt(r);
	}

	void AddSVGPath(CPath::const_iterator first, CPath::const_iterator last, CComBSTR& bstr)
	{
		if (first == last) return;
		CPath::const_iterator ii = first;
		CPath::const_iterator i = ii;
		++ii;
		if (ii == last) return;

		OLECHAR sz[256];
		swprintf(sz, bstr.Length() == 0 ? L"M %g %g" : L" M %g %g", i->tPos.fX, i->tPos.fY);
		bstr += sz;

		bool end = false;
		do
		{
			ii = i;
			++i;
			if (i == last)
			{
				end = true;
				i = first;
			}
			if (ii->tTanNext.fX == 0.0f && ii->tTanNext.fY == 0.0f && i->tTanPrev.fX == 0.0f && i->tTanPrev.fY == 0.0f)
				swprintf(sz, L" L %g %g", i->tPos.fX, i->tPos.fY);
			else
				swprintf(sz, L" C %g %g %g %g %g %g", ii->tPos.fX+ii->tTanNext.fX, ii->tPos.fY+ii->tTanNext.fY,
													  i->tPos.fX+i->tTanPrev.fX, i->tPos.fY+i->tTanPrev.fY, i->tPos.fX, i->tPos.fY);
			bstr += sz;
		}
		while (!end);

		bstr += L" z";
	}

	static void SplitPaths(CPath const& a_in, std::vector<CPath>& a_out)
	{
		CPath::const_iterator i1 = a_in.begin();
		CPath::const_iterator i2 = i1;
		do
		{
			bool stop = false;
			while (i2 != a_in.end() && !stop)
			{
				stop = i2->dwFlags&EPPClose;
				++i2;
			}
			if (i2-i1 > 1)
			{
				size_t const n = a_out.size();
				a_out.resize(n+1);
				a_out[n].assign(i1, i2);
				a_out[n][a_out[n].size()-1].dwFlags &= ~EPPClose;
			}
			i1 = i2;
		}
		while (i2 != a_in.end());
	}
	template<typename TIt>
	static void MergePaths(TIt first, TIt last, CPath& a_out)
	{
		for (TIt i = first; i != last; ++i)
		{
			if (i->size() > 1)
			{
				a_out.insert(a_out.end(), i->begin(), i->end());
				a_out[a_out.size()-1].dwFlags |= EPPClose;
			}
		}
	}

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;

	CPath m_cPath;
	CPath::iterator m_iInsert;

	std::vector<CPath> m_cPrevPaths;
	std::vector<CPath>::const_iterator m_iPrevSegment;
	CPath::const_iterator m_iPrevArc;
	float m_fPrevParam;
	TPixelCoords m_tPrevCoords;

	bool m_bMoving;
	bool m_bAdding;
	TPixelCoords m_tLastPos;
	bool m_bLastValid;
	int m_nSplitting;
	TPixelCoords m_tLastSplit;


	CPathPoints m_cPathPoints;

	CEditToolDataStroke m_cData;
};

// 0.5522847498 http://www.whizkidtech.redprince.net/bezier/circle/