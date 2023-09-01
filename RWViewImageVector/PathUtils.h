
#pragma once

#define use_xyz
#include <clipper.hpp>
#include <agg_curves.h>
#include "../RWDrawing/BezierDistance.h"


typedef std::vector<TRWPathPoint> CBezierPath;
typedef std::vector<CBezierPath> CBezierPaths;


inline ClipperLib::cInt F2I(float x) { return x*1024.0+0.5; }
inline float I2F(ClipperLib::cInt x) { return x*0.0009765625; }
inline bool operator==(TPixelCoords l, ClipperLib::IntPoint r) { return F2I(l.fX) == r.X && F2I(l.fY) == r.Y; }
inline bool IsOldVertex(ClipperLib::cInt v) { return v&0x80000000; }
inline bool IsNewVertex(ClipperLib::cInt v) { return v==0; }
inline bool IsSameSegment(ClipperLib::cInt v1, ClipperLib::cInt v2) { return (((v1)&0x7ffffff)==((v2)&0x7ffffff)); }

inline bool GetBezier(CBezierPaths const& orig, ClipperLib::cInt marker, TPixelCoords* bezierPts)
{
	size_t const i1 = (marker&0x7ffff0000)>>16;
	if (i1 == 0 || i1 > orig.size())
		return false;
	CBezierPath const& path = orig[i1-1];
	size_t const i2 = marker&0xffff;
	if (i2 >= path.size())
		return false;
	size_t const i3 = (i2+1)%path.size();
	bezierPts[0] = path[i2].tPos;
	bezierPts[1] = path[i2].tTanNext;
	bezierPts[2] = path[i3].tTanPrev;
	bezierPts[3] = path[i3].tPos;
	return true;
}

template<typename TOut>
inline bool WalkSegments(CBezierPaths const& orig, ClipperLib::Path const& poly, TOut& t_out)
{
	if (poly.size() < 3)
		return false;

	size_t i = 0;
	while (i < poly.size() && !IsOldVertex(poly[i].Z) && !IsNewVertex(poly[i].Z)) ++i;

	if (i == poly.size())
	{
		// no explicit vertex, try segment break (this should not happen if the originals were OK)
		ClipperLib::cInt const first = poly[0].Z;
		i = 1;
		while (i < poly.size() && first == poly[i].Z) ++i;
		if (i == poly.size())
			return false; // errorneous segment

		TPixelCoords b[4];
		if (!GetBezier(orig, poly[i].Z, b))
			return false;
		if (b[0] == poly[i-1])
		{
			//segmentMarker = poly[i].Z;
			--i;
		}
		else if (b[0] == poly[i])
		{
			//segmentMarker = poly[i].Z;
		}
		else
			return false; // weird
	}

	size_t const last = i;
	while (true)
	{
		ClipperLib::cInt segmentMarker = 0;

		size_t const j = i;
		i = (i+1)%poly.size();
		while (i != last && !IsOldVertex(poly[i].Z) && !IsNewVertex(poly[i].Z))
		{
			if (segmentMarker == 0)
				segmentMarker = poly[i].Z;
			else if (segmentMarker != poly[i].Z)
			{
				// missing boundary point
				TPixelCoords b[4];
				if (!GetBezier(orig, segmentMarker, b))
					return false;
				if (b[3] == poly[i] || b[0] == poly[i])
					break;
				i = (i+poly.size()-1)%poly.size();
				break;
			}
			i = (i+1)%poly.size();
		}

		TPixelCoords b[4];
		if (segmentMarker == 0 /*&& (!IsOldVertex(poly[i].Z) || !IsOldVertex(poly[j].Z))*/)
		{
			// no intermediary point -> straight line
			b[0].fX = I2F(poly[j].X);
			b[0].fY = I2F(poly[j].Y);
			b[1].fX = b[1].fY = b[2].fX = b[2].fY = 0.0f;
			b[3].fX = I2F(poly[i].X);
			b[3].fY = I2F(poly[i].Y);
		}
		else
		{
			if (!GetBezier(orig, segmentMarker, b))
				return false;

			bool reverse = false;
			if (IsOldVertex(poly[j].Z) || IsOldVertex(poly[i].Z))
			{
				reverse = IsOldVertex(poly[i].Z) ? IsSameSegment(poly[i].Z, segmentMarker) : !IsSameSegment(poly[j].Z, segmentMarker);
			}
			else
			{
				Bezier segment(b[0].fX, b[0].fY, b[0].fX+b[1].fX, b[0].fY+b[1].fY, b[3].fX+b[2].fX, b[3].fY+b[2].fY, b[3].fX, b[3].fY);
				reverse = segment.NearestPointOnCurve(TVector2d(I2F(poly[j].X), I2F(poly[j].Y))) > segment.NearestPointOnCurve(TVector2d(I2F(poly[i].X), I2F(poly[i].Y)));
			}

			if (reverse)
			{
				std::swap(b[0], b[3]);
				std::swap(b[1], b[2]);
			}

			//ATLASSERT(IsOldVertex(poly[j].Z) ? b[0] == poly[j] : TRUE);
			//ATLASSERT(IsOldVertex(poly[i].Z) ? b[3] == poly[i] : TRUE);

			bool const bj = IsOldVertex(poly[j].Z) && b[0] == poly[j];
			bool const bi = IsOldVertex(poly[i].Z) && b[3] == poly[i];
			if (!bj && !bi && (IsOldVertex(poly[j].Z) || IsOldVertex(poly[i].Z)))
			{
				// recheck orientation
				Bezier segment(b[0].fX, b[0].fY, b[0].fX+b[1].fX, b[0].fY+b[1].fY, b[3].fX+b[2].fX, b[3].fY+b[2].fY, b[3].fX, b[3].fY);
				reverse = segment.NearestPointOnCurve(TVector2d(I2F(poly[j].X), I2F(poly[j].Y))) > segment.NearestPointOnCurve(TVector2d(I2F(poly[i].X), I2F(poly[i].Y)));
				if (reverse)
				{
					std::swap(b[0], b[3]);
					std::swap(b[1], b[2]);
				}
			}

			if (!bj || !bi)
			{
				Bezier segment(b[0].fX, b[0].fY, b[0].fX+b[1].fX, b[0].fY+b[1].fY, b[3].fX+b[2].fX, b[3].fY+b[2].fY, b[3].fX, b[3].fY);
				if (!bj) // first point is intersection
				{
					double split = segment.NearestPointOnCurve(TVector2d(I2F(poly[j].X), I2F(poly[j].Y)));
					Bezier seg(3);
					segment.Evaluate(split, NULL, &seg);
					b[0].fX = I2F(poly[j].X);
					b[0].fY = I2F(poly[j].Y);
					b[1].fX = seg.m_pts[1].x-seg.m_pts[0].x;
					b[1].fY = seg.m_pts[1].y-seg.m_pts[0].y;
					b[2].fX = seg.m_pts[2].x-seg.m_pts[3].x;
					b[2].fY = seg.m_pts[2].y-seg.m_pts[3].y;
					segment.m_pts[0].x = seg.m_pts[0].x;
					segment.m_pts[0].y = seg.m_pts[0].y;
					segment.m_pts[1].x = seg.m_pts[1].x;
					segment.m_pts[1].y = seg.m_pts[1].y;
					segment.m_pts[2].x = seg.m_pts[2].x;
					segment.m_pts[2].y = seg.m_pts[2].y;
					segment.m_pts[3].x = seg.m_pts[3].x;
					segment.m_pts[3].y = seg.m_pts[3].y;
				}
				if (!bi) // last point is intersection
				{
					double split = segment.NearestPointOnCurve(TVector2d(I2F(poly[i].X), I2F(poly[i].Y)));
					Bezier seg(3);
					segment.Evaluate(split, &seg, NULL);
					b[1].fX = seg.m_pts[1].x-seg.m_pts[0].x;
					b[1].fY = seg.m_pts[1].y-seg.m_pts[0].y;
					b[2].fX = seg.m_pts[2].x-seg.m_pts[3].x;
					b[2].fY = seg.m_pts[2].y-seg.m_pts[3].y;
					b[3].fX = I2F(poly[i].X);
					b[3].fY = I2F(poly[i].Y);
				}
			}
		}

		t_out(b);
		if (i == last)
			break;
	}
	return true;
}

struct addSegment
{
	addSegment(CBezierPath& path) : path(path) {}
	void operator()(TPixelCoords const* p)
	{
		size_t const i = path.size();
		path.resize(i+1);
		path[i].dwFlags = 0;
		path[i].tPos = p[3];
		path[i].tTanPrev = p[2];
		path[i-1].tPos = p[0];
		path[i-1].tTanNext = p[1];
	}

private:
	CBezierPath& path;
};

inline void CoordZFill(ClipperLib::IntPoint& e1bot, ClipperLib::IntPoint& e1top, ClipperLib::IntPoint& e2bot, ClipperLib::IntPoint& e2top, ClipperLib::IntPoint& pt)
{
	pt.Z = 0;
}


class ATL_NO_VTABLE CToolWindow : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditWindow
{
public:

BEGIN_COM_MAP(CToolWindow)
	COM_INTERFACE_ENTRY(IRasterImageEditWindow)
END_COM_MAP()

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY) { *a_pSizeX = 1; *a_pSizeY = 1; return S_OK; }
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault) { a_pDefault->bR = a_pDefault->bG = a_pDefault->bB = a_pDefault->bA = 0; return S_OK; }
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer) { return S_FALSE; }
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_pBoundingRectangle)
		{
			a_pBoundingRectangle->left = a_pBoundingRectangle->top = LONG_MIN;
			a_pBoundingRectangle->right = LONG_MAX;
			a_pBoundingRectangle->bottom = LONG_MAX;
		}
		if (a_bEntireRectangle)
			*a_bEntireRectangle = TRUE;
		return S_OK;
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer) { return E_NOTIMPL; }
	STDMETHOD(ControlPointsChanged)() { return S_FALSE; }
	STDMETHOD(ControlPointChanged)(ULONG UNREF(a_nIndex)) { return S_FALSE; }
	STDMETHOD(ControlLinesChanged)() { return S_FALSE; }
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged) { return S_FALSE; }
	STDMETHOD(ScrollWindow)(ULONG UNREF(a_nScrollID), TPixelCoords const* UNREF(a_pDelta)) { return E_NOTIMPL; }
	STDMETHOD(ApplyChanges)() { return S_FALSE;} 
	STDMETHOD(SetState)(ISharedState* a_pState) { return E_NOTIMPL; }
	STDMETHOD(SetColors)(TColor const* a_pColor1, TColor const* a_pColor2) { return E_NOTIMPL; }
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState) { return E_NOTIMPL; }
	STDMETHOD(Handle)(RWHWND* a_phWnd) { return E_NOTIMPL; }
	STDMETHOD(Document)(IDocument** a_ppDocument) { ATLASSERT(0); return E_NOTIMPL; }
	STDMETHOD(Checkpoint)() { return E_NOTIMPL; }
};

class CPolygonReceiver : public CStackUnknown<IRasterImageEditToolPolygon>
{
public:
	CPolygonReceiver(CBezierPaths& bezierPaths, ClipperLib::Paths& polyPaths) :
		bezierPaths(bezierPaths), polyPaths(polyPaths)
	{
	}

	// IRasterImageEditToolPolygon methods
public:
	STDMETHOD(FromPolygon)(ULONG UNREF(a_nCount), TRWPolygon const* UNREF(a_pPolygons)) { return E_NOTIMPL; }
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		try
		{
			if (a_nCount == 0)
				return S_FALSE;

			size_t j = bezierPaths.size()+1;
			//if (j != polyPaths.size()+1)
			//{
			//	ATLASSERT(FALSE);
			//	return E_UNEXPECTED;
			//}

			bezierPaths.reserve(bezierPaths.size()+a_nCount);
			polyPaths.reserve(polyPaths.size()+a_nCount);

			for (ULONG i = 0; i < a_nCount; ++i)
			{
				if (a_pPaths[i].nVertices < 2)
					continue;

				CBezierPath bezierPath(a_pPaths[i].pVertices, a_pPaths[i].pVertices+a_pPaths[i].nVertices);
				ClipperLib::Path polyPath;

				BezierToPolygon(bezierPath, j, polyPath);

				if (polyPath.size() < 3 || ClipperLib::Area(polyPath) == 0.0) // invalid polygon?
					continue;

				if (ClipperLib::Orientation(polyPath))
				{
					// reverse orientation to save ourselves complications
					ReverseBezier(bezierPath);
					polyPath.clear();
					BezierToPolygon(bezierPath, j, polyPath);
				}

				size_t next = bezierPaths.size();
				bezierPaths.resize(next+1);
				std::swap(bezierPaths[next], bezierPath);
				next = polyPaths.size();
				polyPaths.resize(next+1);
				std::swap(polyPaths[next], polyPath);

				++j;
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }

private:
	static void BezierToPolygon(CBezierPath const& bezierPath, size_t index, ClipperLib::Path& polyPath)
	{
		TRWPathPoint tFirst = bezierPath[0];
		TRWPathPoint tPrev = tFirst;
		ClipperLib::Path::iterator prev = polyPath.end();
		index <<= 16;
		for (size_t ii = 0; ii < bezierPath.size(); ++ii)
		{
			TRWPathPoint tThis = bezierPath[(ii+1)%bezierPath.size()];
			agg::curve4 c(
				tPrev.tPos.fX, tPrev.tPos.fY,
				tPrev.tPos.fX+tPrev.tTanNext.fX, tPrev.tPos.fY+tPrev.tTanNext.fY,
				tThis.tPos.fX+tThis.tTanPrev.fX, tThis.tPos.fY+tThis.tTanPrev.fY,
				tThis.tPos.fX, tThis.tPos.fY
			);
			double x = 0.0;
			double y = 0.0;
			size_t frst = 0x80000000;
			while (!agg::is_stop(c.vertex(&x, &y)))
			{
				ClipperLib::IntPoint pt(x*1024.0+0.5, y*1024.0+0.5, index|frst);
				if (prev != polyPath.end() && pt.X == prev->X && pt.Y == prev->Y)
				{
					if ((prev->Z&0x80000000) == 0)
						prev->Z = index|frst;
				}
				else
				{
					polyPath.push_back(pt);
					prev = polyPath.end()-1;
				}
				frst = 0;
			}
			tPrev = tThis;
			++index;
		}
		if (polyPath.size() > 1)
		{
			if (polyPath[0].X == polyPath[polyPath.size()-1].X && polyPath[0].Y == polyPath[polyPath.size()-1].Y)
				polyPath.resize(polyPath.size()-1);
		}
	}
	static void ReverseBezier(CBezierPath& bezierPath)
	{
		std::reverse(bezierPath.begin(), bezierPath.end());
		for (CBezierPath::iterator i = bezierPath.begin(); i != bezierPath.end(); ++i)
			std::swap(i->tTanPrev, i->tTanNext);
	}

private:
	CBezierPaths& bezierPaths;
	ClipperLib::Paths& polyPaths;
};

inline void GetToolParams(CComObjectStackEx<CToolWindow>& cWnd, IRasterImageEditToolsManager* pToolMgr, std::vector<TRWPath> const& buffer, BSTR* toolID, BSTR* toolParams)
{
	bool simple = true;
	ULONG vtxs = 0;
	ULONG parts = 0;
	for (std::vector<TRWPath>::const_iterator j = buffer.begin(); j != buffer.end(); ++j)
	{
		TRWPathPoint const* i = j->pVertices;
		for (TRWPathPoint const* const e = i+j->nVertices; i != e; ++i)
		{
			if (i->tTanNext.fX != 0.0f || i->tTanNext.fY != 0.0f || i->tTanPrev.fX != 0.0f || i->tTanPrev.fY != 0.0f)
			{
				simple = false;
				break;
			}
			++vtxs;
		}
		if (!simple)
			break;
		++parts;
	}

	CComBSTR bstrID(simple && parts == 1 ? L"POLYGON" : L"SHAPE"); // POLYGON does not support holes now
	CComPtr<IRasterImageEditTool> pPath;
	pToolMgr->EditToolCreate(bstrID, NULL, &pPath);
	CComQIPtr<IRasterImageEditToolScripting> pPathScript(pPath);
	CComQIPtr<IRasterImageEditToolPolygon> pPathPoly(pPath);
	pPath->Init(&cWnd);

	if (simple) // POLYGON does not support holes now
	{
		CAutoVectorPtr<TPixelCoords> vtx(new TPixelCoords[vtxs]);
		CAutoVectorPtr<TRWPolygon> part(new TRWPolygon[parts]);
		vtxs = 0;
		parts = 0;
		for (std::vector<TRWPath>::const_iterator j = buffer.begin(); j != buffer.end(); ++j)
		{
			part[parts].nVertices = j->nVertices;
			part[parts].pVertices = vtx+vtxs;
			TRWPathPoint const* i = j->pVertices;
			for (TRWPathPoint const* const e = i+j->nVertices; i != e; ++i)
				vtx[vtxs++] = i->tPos;
			++parts;
		}
		pPathPoly->FromPolygon(parts, part);
	}
	else
	{
		pPathPoly->FromPath(buffer.size(), &(buffer[0]));
	}

	pPathScript->ToText(toolParams);
	*toolID = bstrID.Detach();
}

