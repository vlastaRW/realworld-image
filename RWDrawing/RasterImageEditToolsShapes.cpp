// RasterImageEditToolsShapes.cpp : Implementation of CRasterImageEditToolsShapes

#include "stdafx.h"
#include "RasterImageEditToolsShapes.h"

#include <IconRenderer.h>

HICON GetToolIconELLIPSE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const outer[] =
	{
		{256, 128, 0, -50.8102, 0, 50.8102},
		{128, 36, -70.6925, 0, 70.6925, 0},
		{0, 128, 0, 50.8102, 0, -50.8102},
		{128, 220, 70.6925, 0, -70.6925, 0},
	};
	static IRPathPoint const inner[] =
	{
		{220, 128, 0, -30.9279, 0, 30.9279},
		{128, 72, -50.8102, 0, 50.8102, 0},
		{36, 128, 0, 30.9279, 0, -30.9279},
		{128, 184, 50.8102, 0, -50.8102, 0},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(outer), outer, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(inner), inner, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetToolIconRECTANGLE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const outer[] =
	{
		{0, 96, 0, -33.1371, 0, 0},
		{60, 36, 0, 0, -33.1371, 0},
		{196, 36, 33.1371, 0, 0, 0},
		{256, 96, 0, 0, 0, -33.1371},
		{256, 160, 0, 33.1371, 0, 0},
		{196, 220, 0, 0, 33.1371, 0},
		{60, 220, -33.1371, 0, 0, 0},
		{0, 160, 0, 0, 0, 33.1371},
	};
	static IRPathPoint const inner[] =
	{
		{36, 96, 0, -13.2548, 0, 0},
		{60, 72, 0, 0, -13.2548, 0},
		{196, 72, 13.2548, 0, 0, 0},
		{220, 96, 0, 0, 0, -13.2548},
		{220, 160, 0, 13.2548, 0, 0},
		{196, 184, 0, 0, 13.2548, 0},
		{60, 184, -13.2548, 0, 0, 0},
		{36, 160, 0, 0, 0, 13.2548},
	};
	static IRGridItem gridX[] = { {0, 0}, {0, 256} };
	static IRGridItem gridY[] = { {0, 36}, {0, 220} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	//static IRGridItem innerX[] = { {0, 36}, {0, 220} };
	//static IRGridItem innerY[] = { {0, 72}, {0, 184} };
	//static IRCanvas canvasInner = {0, 0, 256, 256, itemsof(innerX), itemsof(innerY), innerX, innerY};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(outer), outer, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(inner), inner, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetToolIconPOLYGON(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const outer[] =
	{
		{206, 243}, {50, 243}, {2, 95}, {128, 3}, {254, 95},
	};
	static IRPolyPoint const inner[] =
	{
		{212, 109}, {180, 207}, {76, 207}, {44, 109}, {128, 48},
	};
	static IRGridItem gridY[] = { {0, 243} };
	static IRCanvas canvas = {0, 0, 256, 256, 0, itemsof(gridY), NULL, gridY};
	//static IRGridItem innerY[] = { {0, 207} };
	//static IRCanvas canvasInner = {0, 0, 256, 256, 0, itemsof(innerY), NULL, innerY};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(outer), outer, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(inner), inner, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetToolIconSHAPE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const outer[] =
	{
		{65, 5, 41.1356, -9.4269, -48, 11},
		{150, 56, 13, 40, -13, -40},
		{210, 123, 68, 22, -49.3469, -15.9652},
		{197, 250, -60.733, 13.2853, 64, -14},
		{17, 168, -38, -83, 36.4926, 79.7075},
		//{256, 256, 0, 0, 0, 0},
		//{256, 0, -119.294, 0, 0, 0},
		//{0, 256, 0, 0, -1.52588e-005, -119.294},
	};
	static IRPathPoint const inner[] =
	{
		{121, 89, 15, 80, -12.3967, -66.116},
		{216, 191, -5.59629, 36.065, 9, -58},
		{78, 190, -44, -36, 45.2086, 36.9889},
		{47, 64, 17, -32, -17, 32},
		//{220, 220, 0, 0, 0, 0},
		//{220, 41, -78.6107, 20.2482, 0, 0},
		//{41, 220, 0, 0, 20.2482, -78.6107},
	};
	//static IRGridItem outerGrid[] = { {0, 256} };
	//static IRCanvas canvas = {0, 0, 256, 256, itemsof(outerGrid), itemsof(outerGrid), outerGrid, outerGrid};
	//static IRGridItem innerGrid[] = { {0, 220} };
	//static IRCanvas canvasInner = {0, 0, 256, 256, itemsof(innerGrid), itemsof(innerGrid), innerGrid, innerGrid};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(outer), outer, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(inner), inner, pSI->GetMaterial(ESMScheme2Color2));
	return cRenderer.get();
}

HICON GetToolIconLINE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPolyPoint const line[] =
	{
		{0, 143}, {235, 57}, {256, 113}, {21, 199},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(line), line, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconCURVE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const line[] =
	{
		{0, 93, 83.5, -40.5, 0, 0},
		{140, 97, 25, 28, -19.3145, -21.6322},
		{235, 107, 0, 0, -64, 24},
		{256, 163, -85, 40, 0, 0},
		{116, 159, -25, -28, 19.3145, 21.6322},
		{21, 149, 0, 0, 64, -24},
		//{0, 93, 130, -48, 0, 0},
		//{235, 107, 0, 0, -96, 36},
		//{256, 163, -130, 48, 0, 0},
		//{21, 149, 0, 0, 96, -36},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(line), line, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconSTROKE(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const line[] =
	{
		{0, 181, 57, -35, 0, 0},
		{130, 120, 27.1277, 3.01419, -27, -3},
		{194, 104, 6.58218, -18.8062, -7, 20},
		{164, 65, 0, 0, 16, 2},
		{174, 6, 31, 6, 0, 0},
		{251, 74, 12, 59, -7.54485, -37.0955},
		{195, 176, -35.6737, 12.1813, 41, -14},
		{111, 185, -11.6619, 5.83095, 28, -14},
		{32, 231, 0, 0, 40, -25},
	};
	static IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(line), line, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}


// CRasterImageEditToolsShapes

STDMETHODIMP CRasterImageEditToolsShapes::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		if (wcscmp(a_bstrID, L"ELLIPSE") == 0)
		{
			*a_phIcon = GetToolIconELLIPSE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"RECTANGLE") == 0)
		{
			*a_phIcon = GetToolIconRECTANGLE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"POLYGON") == 0)
		{
			*a_phIcon = GetToolIconPOLYGON(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"SHAPE") == 0)
		{
			*a_phIcon = GetToolIconSHAPE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"LINE") == 0)
		{
			*a_phIcon = GetToolIconLINE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"CURVE") == 0)
		{
			*a_phIcon = GetToolIconCURVE(a_nSize);
			return S_OK;
		}
		if (wcscmp(a_bstrID, L"STROKE") == 0)
		{
			*a_phIcon = GetToolIconSTROKE(a_nSize);
			return S_OK;
		}

		return CRasterImageEditToolsFactoryImpl<g_aShapeTools, sizeof(g_aShapeTools)/sizeof(g_aShapeTools[0])>::ToolIconGet(a_pManager, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

static ULONG const PF_CLOSE = 1;
static ULONG const PF_ENDSUB = 4;

static void ClosePathPart(std::vector<TRWPathPoint>& path, ULONG extra)
{
	size_t size = path.size();
	if (size < 2)
		return;
	size_t first = size-1;
	if (path[first].dwFlags&PF_ENDSUB)
		return;
	while (first > 0 && (path[first].dwFlags&PF_ENDSUB) == 0) --first;
	if (path[first].dwFlags&PF_ENDSUB) ++first;
	size_t last = size-1;
	if (last-first > 2)
	{
		if (path[first].tPos.fX == path[last].tPos.fX && path[first].tPos.fY == path[last].tPos.fY)
		{
			path[first].tTanPrev = path[last].tTanPrev;
			path[last-1].dwFlags |= PF_ENDSUB|extra;
			path.resize(size-1);
			return;
		}
	}
	path[size-1].dwFlags |= PF_ENDSUB|extra;
}

// Generate the control points and endpoints for a set of bezier curves that match
// a circular arc starting from angle 'angleStart' and sweep the angle 'angleExtent'.
// The circle the arc follows will be centred on (0,0) and have a radius of 1.0.
//
// Each bezier can cover no more than 90 degrees, so the arc will be divided evenly
// into a maximum of four curves.
//
// The resulting control points will later be scaled and rotated to match the final
// arc required.
//
// The returned array has the format [x0,y0, x1,y1,...] and excludes the start point
// of the arc.
//
static int arcToBeziers(double angleStart, double angleExtent, TVector2f* aCoords) // must have space for 4*3 points
{
	int numSegments = (int) ceil(fabs(angleExtent) * 2.0 / 3.14159265358979);  // (angleExtent / 90deg)

	double angleIncrement = angleExtent / numSegments;

	// The length of each control point vector is given by the following formula.
	double controlLength = 4.0 / 3.0 * sin(angleIncrement / 2.0) / (1.0 + cos(angleIncrement / 2.0));

	int pos = 0;

	for (int i=0; i<numSegments; i++)
	{
		double  angle = angleStart + i * angleIncrement;
		// Calculate the control vector at this angle
		double  dx = cos(angle);
		double  dy = sin(angle);
		// First control point
		aCoords[pos].x = (float) (dx - controlLength * dy);
		aCoords[pos].y = (float) (dy + controlLength * dx);
		++pos;
		// Second control point
		angle += angleIncrement;
		dx = cos(angle);
		dy = sin(angle);
		aCoords[pos].x = (float) (dx + controlLength * dy);
		aCoords[pos].y = (float) (dy - controlLength * dx);
		++pos;
		// Endpoint of bezier
		aCoords[pos].x = (float) dx;
		aCoords[pos].y = (float) dy;
		++pos;
	}
	return numSegments;
}

// Check input to Math.acos() in case rounding or other errors result in a val < -1 or > +1.
// For example, see the possible KitKat JIT error described in issue #62.
static double checkedArcCos(double val)
{
	return (val < -1.0) ? 3.14159265358979 : (val > 1.0) ? 0 : acos(val);
}

// SVG arc representation uses "endpoint parameterisation" where we specify the start and endpoint of the arc.
// This is to be consistent with the other path commands.  However we need to convert this to "centre point
// parameterisation" in order to calculate the arc. Handily, the SVG spec provides all the required maths
// in section "F.6 Elliptical arc implementation notes".
//
// Some of this code has been borrowed from the Batik library (Apache-2 license).
//
// Previously, to work around issue #62, we converted this function to use floats. However in issue #155,
// we discovered that there are some arcs that fail due of a lack of precision. So we have switched back to doubles.
// https://github.com/BigBadaboom/androidsvg/blob/5db71ef0007b41644258c1f139f941017aef7de3/androidsvg/src/main/java/com/caverock/androidsvg/utils/SVGAndroidRenderer.java#L2889
//
static void arcTo(float lastX, float lastY, float rx, float ry, float angle, bool largeArcFlag, bool sweepFlag, float x, float y, std::vector<TRWPathPoint>& path)
{
    // If the endpoints (x, y) and (x0, y0) are identical, then this
    // is equivalent to omitting the elliptical arc segment entirely.
    // (behaviour specified by the spec)
	if (lastX == x && lastY == y)
		return;

	// Handle degenerate case (behaviour specified by the spec)
	if (rx == 0 || ry == 0)
	{
		TRWPathPoint t;
		t.dwFlags = 0;
		t.tPos.fX = x;
		t.tPos.fY = y;
		t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
		t.tTanNext.fX = t.tTanNext.fY = 0.0f;
		path.push_back(t);
		return;
	}

	// Sign of the radii is ignored (behaviour specified by the spec)
	rx = fabs(rx);
	ry = fabs(ry);

	// Convert angle from degrees to radians
	double angleRad = fmod(angle, 360)/180.0*3.14159265358979;
	double cosAngle = cos(angleRad);
	double sinAngle = sin(angleRad);

	// We simplify the calculations by transforming the arc so that the origin is at the
	// midpoint calculated above followed by a rotation to line up the coordinate axes
	// with the axes of the ellipse.

	// Compute the midpoint of the line between the current and the end point
	double dx2 = (lastX - x) / 2.0;
	double dy2 = (lastY - y) / 2.0;

	// Step 1 : Compute (x1', y1')
	// x1,y1 is the midpoint vector rotated to take the arc's angle out of consideration
	double x1 = (cosAngle * dx2 + sinAngle * dy2);
	double y1 = (-sinAngle * dx2 + cosAngle * dy2);

	double rx_sq = rx * rx;
	double ry_sq = ry * ry;
	double x1_sq = x1 * x1;
	double y1_sq = y1 * y1;

	// Check that radii are large enough.
	// If they are not, the spec says to scale them up so they are.
	// This is to compensate for potential rounding errors/differences between SVG implementations.
	double radiiCheck = x1_sq / rx_sq + y1_sq / ry_sq;
	if (radiiCheck > 0.99999)
	{
		double radiiScale = sqrt(radiiCheck) * 1.00001;
		rx = (float) (radiiScale * rx);
		ry = (float) (radiiScale * ry);
		rx_sq = rx * rx;
		ry_sq = ry * ry;
	}

	// Step 2 : Compute (cx1, cy1) - the transformed centre point
	double sign = (largeArcFlag == sweepFlag) ? -1 : 1;
	double sq = ((rx_sq * ry_sq) - (rx_sq * y1_sq) - (ry_sq * x1_sq)) / ((rx_sq * y1_sq) + (ry_sq * x1_sq));
	sq = (sq < 0) ? 0 : sq;
	double coef = (sign * sqrt(sq));
	double cx1 = coef * ((rx * y1) / ry);
	double cy1 = coef * -((ry * x1) / rx);

	// Step 3 : Compute (cx, cy) from (cx1, cy1)
	double sx2 = (lastX + x) / 2.0;
	double sy2 = (lastY + y) / 2.0;
	double cx = sx2 + (cosAngle * cx1 - sinAngle * cy1);
	double cy = sy2 + (sinAngle * cx1 + cosAngle * cy1);

	// Step 4 : Compute the angleStart (angle1) and the angleExtent (dangle)
	double ux = (x1 - cx1) / rx;
	double uy = (y1 - cy1) / ry;
	double vx = (-x1 - cx1) / rx;
	double vy = (-y1 - cy1) / ry;
	double p, n;

	// Angle betwen two vectors is +/- acos( u.v / len(u) * len(v))
	// Where '.' is the dot product. And +/- is calculated from the sign of the cross product (u x v)

	double const TWO_PI = 3.14159265358979 * 2.0;

	// Compute the start angle
	// The angle between (ux,uy) and the 0deg angle (1,0)
	n = sqrt((ux * ux) + (uy * uy));		// len(u) * len(1,0) == len(u)
	p = ux;									// u.v == (ux,uy).(1,0) == (1 * ux) + (0 * uy) == ux
	sign = (uy < 0) ? -1.0 : 1.0;			// u x v == (1 * uy - ux * 0) == uy
	double angleStart = sign * acos(p / n); // No need for checkedArcCos() here. (p >= n) should always be true.

	// Compute the angle extent
	n = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
	p = ux * vx + uy * vy;
	sign = (ux * vy - uy * vx < 0) ? -1.0f : 1.0f;
	double angleExtent = sign * checkedArcCos(p / n);

	// Catch angleExtents of 0, which will cause problems later in arcToBeziers
	if (angleExtent == 0.0f)
	{
		TRWPathPoint t;
		t.dwFlags = 0;
		t.tPos.fX = x;
		t.tPos.fY = y;
		t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
		t.tTanNext.fX = t.tTanNext.fY = 0.0f;
		path.push_back(t);
		return;
	}

	if (!sweepFlag && angleExtent > 0)
		angleExtent -= TWO_PI;
	else if (sweepFlag && angleExtent < 0)
		angleExtent += TWO_PI;

	angleExtent = fmod(angleExtent, TWO_PI);
	angleStart = fmod(angleStart, TWO_PI);

	// Many elliptical arc implementations including the Java2D and Android ones, only
	// support arcs that are axis aligned.  Therefore we need to substitute the arc
	// with bezier curves.  The following method call will generate the beziers for
	// a unit circle that covers the arc angles we want.
	TVector2f bezierPoints[4*3];
	int bezierArcs = arcToBeziers(angleStart, angleExtent, bezierPoints);

	// Calculate a transformation matrix that will move and scale these bezier points to the correct location.
	for (TVector2f* p = bezierPoints; p != bezierPoints+3*bezierArcs; ++p)
	{
		float const x = p->x*rx*cosAngle - p->y*ry*sinAngle + cx;
		float const y = p->x*rx*sinAngle + p->y*ry*cosAngle + cy;
		p->x = x;
		p->y = y;
	}

	// The last point in the bezier set should match exactly the last coord pair in the arc (ie: x,y). But
	// considering all the mathematical manipulation we have been doing, it is bound to be off by a tiny
	// fraction. Experiments show that it can be up to around 0.00002.  So why don't we just set it to
	// exactly what it ought to be.
	bezierPoints[bezierArcs*3-1].x = x;
	bezierPoints[bezierArcs*3-1].y = y;

	// Final step is to add the bezier curves to the path
	for (int i=0; i<bezierArcs; ++i)
	{
		TVector2f const* p = bezierPoints+3*i;

		TRWPathPoint t;
		t.dwFlags = 0;

		TRWPathPoint& tp = path[path.size()-1];
		tp.tTanNext.fX = p[0].x-tp.tPos.fX;
		tp.tTanNext.fY = p[0].y-tp.tPos.fY;
		t.tTanPrev.fX = p[1].x-p[2].x;
		t.tTanPrev.fY = p[1].y-p[2].y;
		t.tPos.fX = p[2].x;
		t.tPos.fY = p[2].y;
		t.tTanNext.fX = t.tTanNext.fY = 0.0f;
		path.push_back(t);
	}
}


static void AddPoint(std::vector<TRWPathPoint>& path, wchar_t state, std::vector<float> const& nums, bool close, TPixelCoords& lastPos)
{
	TRWPathPoint t;
	t.dwFlags = 0;
	size_t step = 0;
	switch (state)
	{
	case L'M':
		if (nums.size() >= 2)
		{
			ClosePathPart(path, 0);
			t.tPos.fX = nums[0];
			t.tPos.fY = nums[1];
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			step = 2; // more points -> L
			for (size_t base = 2; base+step <= nums.size(); base += step)
			{
				t.tPos.fX = nums[base+0];
				t.tPos.fY = nums[base+1];
				t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
				t.tTanNext.fX = t.tTanNext.fY = 0.0f;
				path.push_back(t);
				lastPos = t.tPos;
				if (close && base+step+step > nums.size())
					ClosePathPart(path, PF_CLOSE);
			}
		}
		break;
	case L'm':
		if (nums.size() >= 2)
		{
			ClosePathPart(path, 0);
			t.tPos.fX = lastPos.fX+nums[0];
			t.tPos.fY = lastPos.fY+nums[1];
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			step = 2;
			for (size_t base = 2; base+step <= nums.size(); base += step)
			{
				t.tPos.fX = lastPos.fX+nums[base+0];
				t.tPos.fY = lastPos.fY+nums[base+1];
				t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
				t.tTanNext.fX = t.tTanNext.fY = 0.0f;
				path.push_back(t);
				lastPos = t.tPos;
				if (close && base+step+step > nums.size())
					ClosePathPart(path, PF_CLOSE);
			}
		}
		break;
	case L'H':
	case L'h':
		if (path.empty())
			break; // missing M
		step = 1;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			t.tPos.fX = state == L'h' ? lastPos.fX+nums[base+0] : nums[base+0];
			t.tPos.fY = lastPos.fY;
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'V':
	case L'v':
		if (path.empty())
			break; // missing M
		step = 1;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			t.tPos.fX = lastPos.fX;
			t.tPos.fY = state == L'v' ? lastPos.fY+nums[base+0] : nums[base+0];
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'L':
		if (path.empty())
			break; // missing M
		step = 2;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			t.tPos.fX = nums[base+0];
			t.tPos.fY = nums[base+1];
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'l':
		if (path.empty())
			break; // missing M
		step = 2;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			t.tPos.fX = lastPos.fX+nums[base+0];
			t.tPos.fY = lastPos.fY+nums[base+1];
			t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'C':
		if (path.empty())
			break; // missing M
		step = 6;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = nums[base+0]-tp.tPos.fX;
			tp.tTanNext.fY = nums[base+1]-tp.tPos.fY;
			t.tTanPrev.fX = nums[base+2]-nums[base+4];
			t.tTanPrev.fY = nums[base+3]-nums[base+5];
			t.tPos.fX = nums[base+4];
			t.tPos.fY = nums[base+5];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'c':
		if (path.empty())
			break; // missing M
		step = 6;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = nums[base+0];
			tp.tTanNext.fY = nums[base+1];
			t.tTanPrev.fX = nums[base+2]-nums[base+4];
			t.tTanPrev.fY = nums[base+3]-nums[base+5];
			t.tPos.fX = tp.tPos.fX+nums[base+4];
			t.tPos.fY = tp.tPos.fY+nums[base+5];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'S':
		if (path.empty())
			break; // missing M
		step = 4;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = -tp.tTanPrev.fX;
			tp.tTanNext.fY = -tp.tTanPrev.fY;
			t.tTanPrev.fX = nums[base+0]-nums[base+2];
			t.tTanPrev.fY = nums[base+1]-nums[base+3];
			t.tPos.fX = nums[base+2];
			t.tPos.fY = nums[base+3];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L's':
		if (path.empty())
			break; // missing M
		step = 4;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = -tp.tTanPrev.fX;
			tp.tTanNext.fY = -tp.tTanPrev.fY;
			t.tTanPrev.fX = nums[base+0]-nums[base+2];
			t.tTanPrev.fY = nums[base+1]-nums[base+3];
			t.tPos.fX = tp.tPos.fX+nums[base+2];
			t.tPos.fY = tp.tPos.fY+nums[base+3];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'Q':
		if (path.empty())
			break; // missing M
		step = 4;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = (nums[base+0]*2+tp.tPos.fX)/3-tp.tPos.fX;
			tp.tTanNext.fY = (nums[base+1]*2+tp.tPos.fY)/3-tp.tPos.fY;
			t.tTanPrev.fX = (nums[base+0]*2+nums[base+2])/3-nums[base+2];
			t.tTanPrev.fY = (nums[base+1]*2+nums[base+3])/3-nums[base+3];
			t.tPos.fX = nums[base+2];
			t.tPos.fY = nums[base+3];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'q':
		if (path.empty())
			break; // missing M
		step = 4;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			TRWPathPoint& tp = path[path.size()-1];
			tp.tTanNext.fX = (nums[base+0]*2)/3;
			tp.tTanNext.fY = (nums[base+1]*2)/3;
			t.tTanPrev.fX = (nums[base+0]*2+nums[base+2])/3-nums[base+2];
			t.tTanPrev.fY = (nums[base+1]*2+nums[base+3])/3-nums[base+3];
			t.tPos.fX = tp.tPos.fX+nums[base+2];
			t.tPos.fY = tp.tPos.fY+nums[base+3];
			t.tTanNext.fX = t.tTanNext.fY = 0.0f;
			path.push_back(t);
			lastPos = t.tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'A':
		if (path.empty())
			break; // missing M
		step = 7;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			arcTo(lastPos.fX, lastPos.fY, nums[base+0], nums[base+1], nums[base+2], nums[base+3] != 0.0f, nums[base+4] != 0.0f, nums[base+5], nums[base+6], path);
			lastPos = path[path.size()-1].tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	case L'a':
		if (path.empty())
			break; // missing M
		step = 7;
		for (size_t base = 0; base+step <= nums.size(); base += step)
		{
			//TRWPathPoint& tp = path[path.size()-1];
			arcTo(lastPos.fX, lastPos.fY, nums[base+0], nums[base+1], nums[base+2], nums[base+3] != 0.0f, nums[base+4] != 0.0f, lastPos.fX+nums[base+5], lastPos.fY+nums[base+6], path);
			lastPos = path[path.size()-1].tPos;
			if (close && base+step+step > nums.size())
				ClosePathPart(path, PF_CLOSE);
		}
		break;
	}
}

static bool ParseSVGPath(LPCWSTR psz, std::vector<TRWPathPoint>& cPath)
{
	if (psz == NULL)
		return false;
	TPixelCoords tLastPos = {0, 0};
	std::vector<float> nums;
	wchar_t state = L'\0';
	while (*psz)
	{
		while (*psz == L' ' || *psz == L'\t' || *psz == L',' || *psz == L'\n' || *psz == L'\r')
			++psz;
		if (!*psz)
			break;
		switch (*psz)
		{
		case L'M':
		case L'L':
		case L'C':
		case L'S':
		case L'H':
		case L'V':
		case L'Q':
//		case L'T':
		case L'A'://https://www.w3.org/TR/SVG/paths.html#PathDataEllipticalArcCommands
		case L'm':
		case L'l':
		case L'c':
		case L's':
		case L'h':
		case L'v':
		case L'q':
//		case L't':
		case L'a':
			AddPoint(cPath, state, nums, false, tLastPos);
			nums.clear();
			state = *psz;
			++psz;
			break;
		case L'Z':
		case L'z':
			AddPoint(cPath, state, nums, true, tLastPos);
			nums.clear();
			state = L'\0';
			++psz;
			break;
		default:
			{
				LPCOLESTR pszNext = psz;
				double d = wcstod(psz, const_cast<LPOLESTR*>(&pszNext));
				if (pszNext <= psz)
					return false;
				nums.push_back(d);
				psz = pszNext;
			}
		}
	}
	AddPoint(cPath, state, nums, false, tLastPos);
	return true;
}

STDMETHODIMP CRasterImageEditToolsShapes::SVGToPolygon(BSTR a_bstrSVG, IRasterImageEditToolPolygon* a_pConsumer)
{
	try
	{
		std::vector<TRWPathPoint> path;
		if (!ParseSVGPath(a_bstrSVG, path))
			return E_NOTIMPL;
		int parts = 0;
		bool ended = true;
		for (std::vector<TRWPathPoint>::const_iterator i = path.begin(); i != path.end(); ++i)
		{
			if (ended)
				++parts;
			ended = (i->dwFlags&PF_ENDSUB) == PF_ENDSUB;
		}
		if (parts == 0)
			return S_FALSE;
		CAutoVectorPtr<TRWPath> aPaths(new TRWPath[parts]);
		parts = 0;
		ended = true;
		for (std::vector<TRWPathPoint>::iterator i = path.begin(); i != path.end(); ++i)
		{
			if (ended)
			{
				aPaths[parts].pVertices = &*i;
				aPaths[parts].nVertices = 0;
				++parts;
			}
			ended = (i->dwFlags&PF_ENDSUB) == PF_ENDSUB;
			++aPaths[parts-1].nVertices;
			i->dwFlags &= PF_CLOSE;
		}
		return a_pConsumer->FromPath(parts, aPaths);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

