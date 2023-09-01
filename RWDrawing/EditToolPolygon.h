
#pragma once

#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithBrush.h"
#include "EditToolPolyLine.h"
#include "EditToolWithCtrlDropper.h"
#include "PolygonUtil.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_primitives.h>
#include <agg_conv_contour.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_rasterizer_compound_aa.h>


struct CEditToolDataPolygon
{
	MIDL_INTERFACE("13ADE81C-448B-499E-8DE1-A79A6BDD77BB")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataPolygon const*, InternalData)() = 0;
	};
	enum EStartWith
	{
		ESWTriangle = 0,
		ESWSquare,
		ESWPentagon,
		ESWHexagon,
		ESWOctagon,
		ESW5Star,
		ESW6Star,
		ESWArrow,
		ESWCount,
		ESWFreeform = ESWCount,
	};

	CEditToolDataPolygon() : eStartWith(ESWTriangle)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (wcscmp(a_bstr, L"SQUARE") == 0)
			eStartWith = ESWSquare;
		else if (wcscmp(a_bstr, L"PENTAGON") == 0)
			eStartWith = ESWPentagon;
		else if (wcscmp(a_bstr, L"HEXAGON") == 0)
			eStartWith = ESWHexagon;
		else if (wcscmp(a_bstr, L"OCTAGON") == 0)
			eStartWith = ESWOctagon;
		else if (wcscmp(a_bstr, L"5STAR") == 0)
			eStartWith = ESW5Star;
		else if (wcscmp(a_bstr, L"6STAR") == 0)
			eStartWith = ESW6Star;
		else if (wcscmp(a_bstr, L"ARROW") == 0)
			eStartWith = ESWArrow;
		else if (wcscmp(a_bstr, L"FREEFORM") == 0)
			eStartWith = ESWFreeform;
		else 
			eStartWith = ESWTriangle;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t const* psz = L"TRIANGLE";
		switch (eStartWith)
		{
		case ESWSquare: psz = L"SQUARE"; break;
		case ESWPentagon: psz = L"PENTAGON"; break;
		case ESWHexagon: psz = L"HEXAGON"; break;
		case ESWOctagon: psz = L"OCTAGON"; break;
		case ESW5Star: psz = L"5STAR"; break;
		case ESW6Star: psz = L"6STAR"; break;
		case ESWArrow: psz = L"ARROW"; break;
		case ESWFreeform: psz = L"FREEFORM"; break;
		}
		*a_pbstr = SysAllocString(psz);
		return S_OK;
	}

	EStartWith eStartWith;
};

struct SStartPoly
{
	char n;
	float scale;
	TPixelCoords const aPoly[12];
};
extern __declspec(selectany) SStartPoly const g_aStartPolys[CEditToolDataPolygon::ESWCount] =
{
	{3, 1.0f, {{0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, -0.86602540378f}}}, // triangle
	{4, 0.8f, {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, -1.0f}, {0.0f, -1.0f}}}, // square
	{5, 0.9f, {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.30901699f, -0.951056516f}, {0.5f, -1.5388417685876f}, {-0.30901699f, -0.951056516f}}}, // pentagon
	{6, 0.9f, {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.5f, -0.86602540378f}, {1.0f, -1.73205080757f}, {0.0f, -1.73205080757f}, {-0.5f, -0.86602540378f}}}, // hexagon
	{8, 0.8f, {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.70710678119f, -0.70710678119f}, {1.70710678119f, -1.70710678119f}, {1.0f, -2.4142135623731f}, {0.0f, -2.4142135623731f}, {-0.70710678119f, -1.70710678119f}, {-0.70710678119f, -0.70710678119f}}}, // octagon
	{10, 1.0f, {{0.0f, 0.0f}, {0.5f, -0.363271264f}, {1.0f, 0.0f}, {0.8090169944f, -0.58778525229f}, {1.30901699f, -0.951056516f}, {0.69098301f, -0.951056516f}, {0.5f, -1.5388417685876f}, {0.30901699f, -0.951056516f}, {-0.30901699f, -0.951056516f}, {0.1909830056f, -0.58778525229f}}}, // 5-pointed star
	{12, 1.0f, {{0.0f, 0.0f}, {0.5f, -0.2886751346f}, {1.0f, 0.0f}, {1.0f, -0.2886751346f*2}, {1.5f, -0.86602540378f}, {1.0f, -1.73205080757f+0.2886751346f*2}, {1.0f, -1.73205080757f}, {0.5f, -1.73205080757f+0.2886751346f}, {0.0f, -1.73205080757f}, {0.0f, -1.73205080757f+0.2886751346f*2}, {-0.5f, -0.86602540378f}, {0.0f, -0.2886751346f*2}}}, // 6-pointed star
	{7, 1.0f, {{0, 0.075f}, {0.75f, 0.075f}, {0.75f, 0.2f}, {1.0f, 0.0f}, {0.75f, -0.2f}, {0.75f, -0.075f}, {0, -0.075f}}}, // arrow
};

#include "EditToolPolygonDlg.h"


HICON GetToolIconPOLYGON(ULONG a_nSize);

class CEditToolPolygon :
	public CEditToolScanlineBuffer<CEditToolPolygon>, // scanline image cache
	public CEditToolPolyLine<CEditToolPolygon>, // polyline-based shape
	public CEditToolMouseInput<CEditToolPolygon>, // no direct tablet support
	public CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, // brush override
	public CEditToolOutline<CEditToolPolygon>, // outline handling
	public CEditToolCustomOrMoveCursor<CEditToolPolygon, GetToolIconPOLYGON>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolPolygon, CEditToolMouseInput<CEditToolPolygon>, CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, CEditToolCustomOrMoveCursor<CEditToolPolygon, GetToolIconPOLYGON> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolPolygon, // T - the top level class for cross casting
		CEditToolPolygon, // TResetHandler
		CEditToolScanlineBuffer<CEditToolPolygon>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolPolygon>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolOutline<CEditToolPolygon>, // TOutlineHandler
		CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, // TBrushHandler
		CEditToolPolygon, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolPolygon, CEditToolMouseInput<CEditToolPolygon>, CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, CEditToolCustomOrMoveCursor<CEditToolPolygon, GetToolIconPOLYGON> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolPolygon, CEditToolMouseInput<CEditToolPolygon>, CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, CEditToolCustomOrMoveCursor<CEditToolPolygon, GetToolIconPOLYGON> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolPolygon, CEditToolMouseInput<CEditToolPolygon>, CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, CEditToolCustomOrMoveCursor<CEditToolPolygon, GetToolIconPOLYGON> >, // TProcessInputHandler
		CEditToolPolyLine<CEditToolPolygon>, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>, // TControlPointsHandler
		CEditToolPolygon // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolPolygon
{
public:
	CEditToolPolygon() : CEditToolPolyLine<CEditToolPolygon>(true), m_bMoving(false),
		m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint)
	{
	}

	BEGIN_COM_MAP(CEditToolPolygon)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
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
	void PrepareShape()
	{
		CPolyLine cOriginal(M_PolyLine());
		if (M_RemoveIndex() < cOriginal.size())
			cOriginal.erase(cOriginal.begin()+M_RemoveIndex());
		else if (M_ExtraIndex())
			cOriginal.insert(cOriginal.begin()+M_ExtraIndex(), M_ExtraPoint());

		if (cOriginal.size() < 3)
			return; // invalid polygon

		std::vector<std::vector<TPixelCoords> > cOuter;
		SplitPolygon(cOriginal, cOuter);

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
			for (std::vector<CPolyLine>::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
			{
				CVertexSource p(*i);
				ras.add_path(p);
			}
			for (std::vector<CPolyLine>::iterator i = cInner.begin(); i != cInner.end(); ++i)
			{
				CVertexSourceHole p(*i);
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
			for (std::vector<CPolyLine>::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
			{
				CVertexSource p(*i);
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
				for (std::vector<CPolyLine>::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
					rasc.add_path(CVertexSource(*i));
				for (std::vector<CPolyLine>::iterator i = cInner.begin(); i != cInner.end(); ++i)
					rasc.add_path(CVertexSourceHole(*i));
				rasc.styles(0, -1);
				for (std::vector<CPolyLine>::iterator i = cInner.begin(); i != cInner.end(); ++i)
					rasc.add_path(CVertexSource(*i));

				agg::span_allocator<CPixelChannel> alloc;
				agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, *this, M_LayerBlender());
			}
			else
			{
				TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
				agg::scanline_p8 sl;
				agg::rasterizer_scanline_aa<> ras;
				for (std::vector<CPolyLine>::iterator i = cOuter.begin(); i != cOuter.end(); ++i)
					ras.add_path(CVertexSource(*i));
				for (std::vector<CPolyLine>::iterator i = cInner.begin(); i != cInner.end(); ++i)
					ras.add_path(CVertexSourceHole(*i));
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);

				ras.reset();
				for (std::vector<CPolyLine>::iterator i = cInner.begin(); i != cInner.end(); ++i)
					ras.add_path(CVertexSource(*i));
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
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		ResetPolyLine();
		ResetDragging();
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		EnableBrush(false);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataPolygon::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			//if (m_cData.eStartWith != pData->InternalData()->eStartWith && !M_PolyLine().empty())
			//{
			//	RECT rc = UpdateCache();
			//	M_Window()->RectangleChanged(&rc);
			//	// TODO: switch shape if it is unmodified
			//}
			m_cData = *(pData->InternalData());
		}
		return S_OK;
	}

	void OutlineChanged(bool a_bWidth, bool a_bColor)
	{
		if (!M_PolyLine().empty())
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
		EnableBrush((!M_Dragging() || m_bMoving) && rcPrev.left < rcPrev.right);
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
		}

		SetPolyLineControlLines(false);
		DeleteCachedData();
		M_PolyLine().clear();
		M_PolyLine().push_back(*a_pPos);
		M_PolyLine().push_back(*a_pPos);
		M_PolyLine().push_back(*a_pPos);
		m_tStartPos = m_tLastPos = *a_pPos;
		if (m_cData.eStartWith == CEditToolDataPolygon::ESWFreeform)
		{
			m_cPathPoints.clear();
			m_cPathPoints.push_back(SPathPoint(*a_pPos, GetTickCount()));
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

		if (M_PolyLine().empty())
			return S_FALSE;

		if (a_pPos->fX == M_PolyLine()[0].fX && a_pPos->fY == M_PolyLine()[0].fY)
		{
			M_PolyLine().clear();
			return S_FALSE;
		}
		if (a_pPos->fX != M_PolyLine()[1].fX || a_pPos->fY != M_PolyLine()[1].fY)
			OnMouseMove(a_eKeysState, a_pPos);
		m_cPathPoints.clear();
		SetPolyLineControlLines(true);
		M_Window()->ControlPointsChanged();
		//M_Window()->ControlLinesChanged();
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
				for (CPolyLine::iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i)
				{
					i->fX += fDX;
					i->fY += fDY;
					ii += 2;
				}
				for (ULONG i = 0; i < ii; ++i)
					M_Window()->ControlPointChanged(i);
				M_Window()->ControlLinesChanged();
				ToolSetBrush();
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
			return S_OK;
		}
		if (!M_Dragging())
			return S_OK;

		TPixelCoords tPos = *a_pPos;
		if (a_eKeysState == ECKSShift)
		{
			// if SHIFT is down, align the first line vertically or horizontally
			if (m_cPathPoints.empty() || M_PolyLine().empty())
			{
				if (fabsf(a_pPos->fX-m_tStartPos.fX) > fabsf(a_pPos->fY-m_tStartPos.fY))
					tPos.fY = m_tStartPos.fY;
				else
					tPos.fX = m_tStartPos.fX;
			}
			else
			{
				CPathPoints::const_iterator iPrev = m_cPathPoints.begin();
				TPixelCoords tPrevPos = iPrev->tPos;
				for (CPathPoints::const_iterator i = iPrev++; i != m_cPathPoints.end(); iPrev = i++)
				{
					if (i->dwTime-iPrev->dwTime > 150UL)
						tPrevPos = i->tPos;
				}
				if (fabsf(a_pPos->fX-tPrevPos.fX) > fabsf(a_pPos->fY-tPrevPos.fY))
					tPos.fY = tPrevPos.fY;
				else
					tPos.fX = tPrevPos.fX;
			}
			a_pPos = &tPos;
		}
		if (m_tLastPos.fX == a_pPos->fX && m_tLastPos.fY == a_pPos->fY)
			return S_FALSE;
		if (!m_cPathPoints.empty())
		{
			m_cPathPoints.push_back(SPathPoint(*a_pPos, GetTickCount()));
			ProcessPath(GetTickCount());
			M_Window()->ControlLinesChanged();
		}
		else
		{
			SStartPoly const* pStart = g_aStartPolys+m_cData.eStartWith;
			M_PolyLine().resize(pStart->n);
			float const fDX = (a_pPos->fX-m_tStartPos.fX);
			float const fDY = (a_pPos->fY-m_tStartPos.fY);
			for (char i = 0; i < pStart->n; ++i)
			{
				M_PolyLine()[i].fX = m_tStartPos.fX + fDX*pStart->aPoly[i].fX + fDY*pStart->aPoly[i].fY;
				M_PolyLine()[i].fY = m_tStartPos.fY + fDY*pStart->aPoly[i].fX - fDX*pStart->aPoly[i].fY;
			}
		}
		m_tLastPos = *a_pPos;
		ToolSetBrush();
		RECT rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_bMoving || (!M_Dragging() && HitTest(a_pPos->fX, a_pPos->fY));
	}

	void ToolSetBrush()
	{
		CVertexSource cVS(this);
		if (M_Brush() && !cVS.m_cPoints.empty())
		{
			TPixelCoords tSum = *cVS.m_cPoints.begin();
			TPixelCoords tMin = tSum;
			TPixelCoords tMax = tSum;
			for (CPolyLine::const_iterator i = cVS.m_cPoints.begin()+1; i != cVS.m_cPoints.end(); ++i)
			{
				tSum.fX += i->fX;
				tSum.fY += i->fY;
				if (tMin.fX > i->fX) tMin.fX = i->fX;
				if (tMax.fX < i->fX) tMax.fX = i->fX;
				if (tMin.fY > i->fY) tMin.fY = i->fY;
				if (tMax.fY < i->fY) tMax.fY = i->fY;
			}
			tSum.fX /= cVS.m_cPoints.size();
			tSum.fY /= cVS.m_cPoints.size();
			M_Brush()->SetShapeBounds(&tSum, (tMax.fX-tMin.fX)*0.5f, (tMax.fY-tMin.fY)*0.5f, 0.0f);
		}
	}

	HRESULT _AdjustCoordinates(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
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
		return CEditToolPolyLine<CEditToolPolygon>::_AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_cPathPoints.size() < 2)
			return CEditToolWithBrush<CEditToolPolygon, CEditToolPolyLine<CEditToolPolygon>, CEditToolPolygon>::_GetControlLines(a_pLines, a_nLineTypes);
		if (M_PolyLine().size() < 2)
		{
			a_pLines->MoveTo(m_cPathPoints[0].tPos.fX, m_cPathPoints[0].tPos.fY);
			a_pLines->LineTo(m_cPathPoints[m_cPathPoints.size()-1].tPos.fX, m_cPathPoints[m_cPathPoints.size()-1].tPos.fY);
			return S_OK;
		}
		CPolyLine::const_iterator i = M_PolyLine().begin();
		a_pLines->MoveTo(i->fX, i->fY);
		for (CPolyLine::const_iterator e = M_PolyLine().end(); i != e; ++i)
		{
			a_pLines->LineTo(i->fX, i->fY);
		}
		if (M_PolyLine().size() > 2)
			a_pLines->Close();
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
		if (M_PolyLine().size() < 3)
		{
			M_PolyLine().clear();
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
			EnableBrush(false);
		}
		ToolSetBrush();
		RECT const rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
	}

	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return a_eKeysState&ECKSControl ? ETPAMissed : HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		TransformPolygon(a_pMatrix);
		TransformBrush(a_pMatrix);
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
			CPolyLine cPolyLine;
			LPOLESTR psz = a_bstrParams;
			TPixelCoords t;
			bool bFirst = true;
			while (*psz)
			{
				LPOLESTR pszNext = psz;
				double d = wcstod(psz, &pszNext);
				if (pszNext <= psz)
					break;
				psz = pszNext;
				while (*psz == L' ' || *psz == L'\t')
					++psz;
				if (*psz == L',')
				{
					++psz;
				}
				if (bFirst)
				{
					t.fX = d;
					bFirst = false;
				}
				else
				{
					t.fY = d;
					cPolyLine.push_back(t);
					bFirst = true;
				}
			}
			if (cPolyLine.size() < 3)
				return E_INVALIDARG;
			SetPolyLine(cPolyLine);
			SetPolyLineControlLines(true);
			m_bMoving = false;
			EnableBrush(true);
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
			OLECHAR sz[48];
			if (M_PolyLine().size() >= 3)
			{
				size_t j = 1;
				for (CPolyLine::const_iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i, ++j)
				{
					if (M_RemoveIndex() == j-1)
						continue;
					swprintf(sz, bstr == NULL ? L"%g, %g" : L", %g, %g", i->fX, i->fY);
					bstr += sz;
					if (M_ExtraIndex() == j)
					{
						swprintf(sz, L", %g, %g", M_ExtraPoint().fX, M_ExtraPoint().fY);
						bstr += sz;
					}
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
		if (a_nCount != 1 || a_pPolygons == NULL)
			return E_NOTIMPL;
		try
		{
			CPolyLine cPolyLine(a_pPolygons->pVertices, a_pPolygons->pVertices+a_pPolygons->nVertices);
			if (cPolyLine.size() < 3)
				return E_INVALIDARG;
			SetPolyLine(cPolyLine);
			SetPolyLineControlLines(true);
			m_bMoving = false;
			EnableBrush(true);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;

		if (M_PolyLine().size() < 3)
			return E_FAIL;

		TRWPolygon tPoly;
		tPoly.nVertices = M_PolyLine().size();
		tPoly.pVertices = &(M_PolyLine()[0]);
		return a_pConsumer->FromPolygon(1, &tPoly);
	}
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;

		if (M_PolyLine().size() < 3)
			return E_FAIL;
		CAutoVectorPtr<TRWPathPoint> cPath(new TRWPathPoint[M_PolyLine().size()]);
		TRWPathPoint* j = cPath;
		for (CPolyLine::const_iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i, ++j)
		{
			j->dwFlags = 0;
			j->tPos = *i;
			j->tTanNext.fX = j->tTanNext.fY = j->tTanPrev.fX = j->tTanPrev.fY = 0.0f;
		}

		TRWPath tPath;
		tPath.nVertices = M_PolyLine().size();
		tPath.pVertices = cPath;
		return a_pConsumer->FromPath(1, &tPath);
	}

private:
	struct SPathPoint
	{
		SPathPoint(TPixelCoords a_t, DWORD a_dw) : tPos(a_t), dwTime(a_dw) {}
		TPixelCoords tPos;
		DWORD dwTime;
	};
	typedef std::vector<SPathPoint> CPathPoints;

	void ProcessPath(DWORD a_dwTime)
	{
		if (m_cPathPoints.empty())
			return;
		CPolyLine cPL;
		CPathPoints::const_iterator iPrev = m_cPathPoints.begin();
		cPL.push_back(iPrev->tPos);
		for (CPathPoints::const_iterator i = iPrev++; i != m_cPathPoints.end(); iPrev = i++)
		{
			if (i->dwTime-iPrev->dwTime > 150UL)
			{
				if (cPL[cPL.size()-1].fX != i->tPos.fX || cPL[cPL.size()-1].fY != i->tPos.fY)
					cPL.push_back(i->tPos);
			}
		}
		if (cPL[cPL.size()-1].fX != m_cPathPoints[m_cPathPoints.size()-1].tPos.fX || cPL[cPL.size()-1].fY != m_cPathPoints[m_cPathPoints.size()-1].tPos.fY)
			cPL.push_back(m_cPathPoints[m_cPathPoints.size()-1].tPos);
		SetPolyLine(cPL);
	}

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;

	bool m_bMoving;
	TPixelCoords m_tStartPos;
	TPixelCoords m_tLastPos;
	CPathPoints m_cPathPoints;

	CEditToolDataPolygon m_cData;
};

