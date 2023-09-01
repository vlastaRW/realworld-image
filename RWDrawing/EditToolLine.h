
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
#include <agg_conv_dash.h>
#include <agg_span_allocator.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataLine
{
	MIDL_INTERFACE("BF461DD4-4331-451D-8764-57E8AB312179")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataLine const*, InternalData)() = 0;
	};

	CEditToolDataLine() : bClosed(false),
		eCapMode(agg::square_cap), eJoinMode(agg::bevel_join),
		fWidth(1.0), bUseDash(false), nDashLen(8), dwDash(0xc3)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
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
		if (wcsstr(a_bstr, L"CLOSED"))
			bClosed = true;
		else if (wcsstr(a_bstr, L"OPEN"))
			bClosed = false;
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
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[256] = L"";
		swprintf(szTmp, L"%g|%s|%s|%s|%s:", fWidth,
			eCapMode == agg::butt_cap ? L"BUTT" : (eCapMode == agg::round_cap ? L"CROUND" : L"CSQUARE"),
			eJoinMode == agg::bevel_join ? L"JBEVEL" : (eJoinMode == agg::round_join ? L"JROUND" : L"JMITER"),
			bClosed ? L"CLOSED" : L"OPEN",
			bUseDash ? L"USEDASH" : L"DASH");
		wchar_t* p = szTmp+wcslen(szTmp);
		for (BYTE b = 0; b < nDashLen; ++b, ++p)
			*p = dwDash&(1<<b) ? L'-' : L' ';
		*p = L'\0';
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	agg::line_cap_e eCapMode;
	bool bClosed;
	agg::line_join_e eJoinMode;
	float fWidth;
	bool bUseDash;
	BYTE nDashLen;
	DWORD dwDash;
};

#include "EditToolLineDlg.h"


HICON GetToolIconLINE(ULONG a_nSize);

class CEditToolLine :
	public CEditToolScanlineBuffer<CEditToolLine>, // scanline image cache
	public CEditToolPolyLine<CEditToolLine>, // polyline-based shape
	public CEditToolMouseInput<CEditToolLine>, // no direct tablet support
	public CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, // brush override
	public CEditToolCustomOrMoveCursor<CEditToolLine, GetToolIconLINE>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolLine, CEditToolMouseInput<CEditToolLine>, CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, CEditToolCustomOrMoveCursor<CEditToolLine, GetToolIconLINE> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolLine, // T - the top level class for cross casting
		CEditToolLine, // TResetHandler
		CEditToolScanlineBuffer<CEditToolLine>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolLine>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, // TBrushHandler
		CEditToolLine, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolLine, CEditToolMouseInput<CEditToolLine>, CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, CEditToolCustomOrMoveCursor<CEditToolLine, GetToolIconLINE> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolLine, CEditToolMouseInput<CEditToolLine>, CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, CEditToolCustomOrMoveCursor<CEditToolLine, GetToolIconLINE> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolLine, CEditToolMouseInput<CEditToolLine>, CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, CEditToolCustomOrMoveCursor<CEditToolLine, GetToolIconLINE> >, // TProcessInputHandler
		CEditToolPolyLine<CEditToolLine>, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolLine, CEditToolPolyLine<CEditToolLine>, CEditToolLine> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IEditToolLine
{
public:
	CEditToolLine() : m_eBlendingMode(EBMDrawOver),
		m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		m_bMoving(false)
	{
	}

	BEGIN_COM_MAP(CEditToolLine)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IEditToolLine)
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
		if (M_PolyLine().size() < 2)
			return;

		agg::renderer_base<CEditToolScanlineBuffer> renb(*this);

		// Rasterizer & scanline
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

		CVertexSource pl(this);

		if (m_cData.bUseDash && m_cData.dwDash && m_cData.dwDash != (1<<m_cData.nDashLen)-1)
		{
			agg::conv_dash<CVertexSource/*, agg::vcgen_markers_term*/> dash(pl);
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
			agg::conv_stroke<agg::conv_dash<CVertexSource/*, agg::vcgen_markers_term*/> > pg(dash);
			pg.line_cap(m_cData.eCapMode);
			pg.line_join(m_cData.eJoinMode);
			//pg.miter_limit(m_miter_limit.value());
			pg.width(m_cData.fWidth);
			ras.add_path(pg);
		}
		else
		{
			agg::conv_stroke<CVertexSource> pg(pl);
			pg.line_cap(m_cData.eCapMode);
			pg.line_join(m_cData.eJoinMode);
			//pg.miter_limit(m_miter_limit.value());
			pg.width(m_cData.fWidth);
			ras.add_path(pg);
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
		CComQIPtr<CEditToolDataLine::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			SetPolyLineClosed(m_cData.bClosed);
			if (!M_PolyLine().empty())
			{
				RECT rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
			}
		}
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

		if (!M_PolyLine().empty())
		{
			ATLASSERT(0);
		}

		SetPolyLineControlLines(false);
		DeleteCachedData();
		M_PolyLine().clear();
		M_PolyLine().push_back(*a_pPos);
		M_PolyLine().push_back(*a_pPos);
		if (a_eKeysState&ECKSControl)
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

		if (a_pPos->fX == M_PolyLine()[0].fX && a_pPos->fY == M_PolyLine()[0].fY)
		{
			M_PolyLine().clear();
			return S_FALSE;
		}
		if (a_pPos->fX != M_PolyLine()[M_PolyLine().size()-1].fX || a_pPos->fY != M_PolyLine()[M_PolyLine().size()-1].fY)
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

		if (M_PolyLine()[M_PolyLine().size()-1].fX == a_pPos->fX && M_PolyLine()[M_PolyLine().size()-1].fY == a_pPos->fY)
			return S_FALSE;
		if (!m_cPathPoints.empty())
		{
			m_cPathPoints.push_back(SPathPoint(*a_pPos, GetTickCount()));
			ProcessPath(GetTickCount());
		}
		else
		{
			M_PolyLine()[1] = *a_pPos;
		}
		//M_PolyLine()[1] = *a_pPos;
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
			M_Brush()->SetShapeBounds(&tSum, (tMax.fX-tMin.fX)*0.5f+m_cData.fWidth, (tMax.fY-tMin.fY)*0.5f+m_cData.fWidth, 0.0f);
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
		float fScale = sqrtf(sqrtf(a_pMatrix->_11*a_pMatrix->_11+a_pMatrix->_12*a_pMatrix->_12)*sqrtf(a_pMatrix->_21*a_pMatrix->_21+a_pMatrix->_22*a_pMatrix->_22));
		m_cData.fWidth *= fScale;
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
			float fWidth = 0.0f;
			std::wstring strDash;
			agg::line_cap_e eCapMode = agg::square_cap;
			agg::line_join_e eJoinMode = agg::bevel_join;
			CPolyLine cPolyLine;
			TPixelCoords t;
			bool bClosed = false;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					real_p[assign_a(fWidth)]>>cSep>>
					(!(ch_p(L'\"')>>(*(ch_p(L' ')|ch_p(L'-')))[assign_a(strDash)]>>ch_p(L'\"')>>cSep))>>
					(!((str_p(L"\"JROUND\"")[assign_a(eJoinMode, agg::round_join)]|str_p(L"\"JBEVEL\"")[assign_a(eJoinMode, agg::bevel_join)]|str_p(L"\"JMITER\"")[assign_a(eJoinMode, agg::miter_join)]) >> cSep))>>
					(!((str_p(L"\"CROUND\"")[assign_a(eCapMode, agg::round_cap)]|str_p(L"\"CBUTT\"")[assign_a(eCapMode, agg::butt_cap)]|str_p(L"\"CSQUARE\"")[assign_a(eCapMode, agg::square_cap)]) >> cSep))>>
					(*((real_p[assign_a(t.fX)]>>cSep>>real_p[assign_a(t.fY)]>>cSep)[push_back_a(cPolyLine, t)]))>>
					((str_p(L"\"CLOSE\"")[assign_a(bClosed, true)]|str_p(L"\"OPEN\"")[assign_a(bClosed, false)])|((real_p[assign_a(t.fX)]>>cSep>>real_p[assign_a(t.fY)])[push_back_a(cPolyLine, t)]))
					>>*space_p).full;
			if (!bParsed || strDash.length() > 32 || fWidth <= 0.0f || cPolyLine.size() < 2)
				return E_INVALIDARG;
			bool bStateChange = m_cData.bClosed != bClosed || m_cData.eCapMode != eCapMode || m_cData.eJoinMode != eJoinMode || m_cData.fWidth != fWidth;
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
			SetPolyLineControlLines(true);
			SetPolyLineClosed(m_cData.bClosed = bClosed);
			SetPolyLine(cPolyLine);
			m_bMoving = false;
			if (bStateChange)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
			}
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
			if (!M_PolyLine().empty())
			{
				swprintf(sz, L"%g", m_cData.fWidth);
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
				size_t j = 1;
				for (CPolyLine::const_iterator i = M_PolyLine().begin(); i != M_PolyLine().end(); ++i, ++j)
				{
					if (M_RemoveIndex() == j-1)
						continue;
					swprintf(sz, L", %g, %g", i->fX, i->fY);
					bstr += sz;
					if (M_ExtraIndex() == j)
					{
						swprintf(sz, L", %g, %g", M_ExtraPoint().fX, M_ExtraPoint().fY);
						bstr += sz;
					}
				}
				if (m_cData.bClosed) bstr += L", \"CLOSE\"";
			}
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IEditToolLine methods
public:
	STDMETHOD(IsClosed)() { return m_cData.bClosed ? S_OK : S_FALSE; }
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
		TRWPolygon tPoly;
		tPoly.nVertices = M_PolyLine().size();
		tPoly.pVertices = &M_PolyLine()[0];
		return a_pConsumer->FromPolygon(1, &tPoly);
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
	TPixelCoords m_tLastPos;
	CPathPoints m_cPathPoints;

	CEditToolDataLine m_cData;
};

