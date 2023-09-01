
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
#include <agg_conv_gpc.h>
#include <agg_renderer_primitives.h>
#include <agg_conv_contour.h>
#include <agg_span_allocator.h>


HICON GetToolIconLASSO(ULONG a_nSize);

class CEditToolLasso :
	public CEditToolScanlineBuffer<CEditToolLasso>, // scanline image cache
	public CEditToolPolyLine<CEditToolLasso>, // polyline-based shape
	public CEditToolMouseInput<CEditToolLasso>, // no direct tablet support
	public CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, // brush override
	public CEditToolCustomOrMoveCursor<CEditToolLasso, GetToolIconLASSO>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolLasso, CEditToolMouseInput<CEditToolLasso>, CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, CEditToolCustomOrMoveCursor<CEditToolLasso, GetToolIconLASSO> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolLasso, // T - the top level class for cross casting
		CEditToolLasso, // TResetHandler
		CEditToolScanlineBuffer<CEditToolLasso>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolLasso>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, // TBrushHandler
		CEditToolLasso, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolLasso, CEditToolMouseInput<CEditToolLasso>, CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, CEditToolCustomOrMoveCursor<CEditToolLasso, GetToolIconLASSO> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolLasso, CEditToolMouseInput<CEditToolLasso>, CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, CEditToolCustomOrMoveCursor<CEditToolLasso, GetToolIconLASSO> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolLasso, CEditToolMouseInput<CEditToolLasso>, CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, CEditToolCustomOrMoveCursor<CEditToolLasso, GetToolIconLASSO> >, // TProcessInputHandler
		CEditToolPolyLine<CEditToolLasso>, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolLasso, CRasterImageEditToolBase, CEditToolLasso> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolLasso() : CEditToolPolyLine<CEditToolLasso>(true),
		m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth),
		m_eCoordinatesMode(ECMFloatingPoint), m_bMoving(false)
	{
	}

	BEGIN_COM_MAP(CEditToolLasso)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
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
		if (M_PolyLine().size() < 3)
			return; // invalid polygon

		agg::renderer_base<CEditToolScanlineBuffer> renb(*this);

		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		CVertexSource pl(this);

		ras.add_path(pl);

		PrepareBrush(CVertexSource(this));

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

		if (M_PolyLine().size() >= 3)
		{
			ATLASSERT(0);
			return ETPAApply|ETPAStartNew;
		}
		EnableBrush(false);

		SetPolyLineControlLines(false);
		M_PolyLine().clear();
		M_PolyLine().push_back(*a_pPos);
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

		CPolyLine& cPL = M_PolyLine();
		if (a_pPos->fX != cPL[cPL.size()-1].fX || a_pPos->fY != cPL[cPL.size()-1].fY)
			OnMouseMove(a_eKeysState, a_pPos);
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

		CPolyLine& cPL = M_PolyLine();
		if (cPL[cPL.size()-1].fX == a_pPos->fX && cPL[cPL.size()-1].fY == a_pPos->fY)
			return S_FALSE;
		cPL.push_back(*a_pPos);
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
		return E_NOTIMPL;
	}

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;

	bool m_bMoving;
	TPixelCoords m_tLastPos;
};

