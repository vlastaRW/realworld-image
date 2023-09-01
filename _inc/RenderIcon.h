
#pragma once


#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_gamma_lut.h>
#include <agg_curves.h>


struct TPolyCoords
{
	float fX;
	float fY;
};

struct TIconPolySpec
{
	int nVertices;
	TPolyCoords const* pVertices;
	agg::rgba8 interior;
	agg::rgba8 outline;
};

inline HICON IconFromPolygon(int a_nPolys, TIconPolySpec const* a_pPolys, int a_nSize, bool a_bAutoScale)
{
	float fScale = a_nSize-1;
	TPolyCoords tCenter = {0.5f, 0.5f};
	ATLASSERT(a_nPolys && a_pPolys && a_pPolys->nVertices && a_pPolys->pVertices);
	if (a_bAutoScale)
	{
		TPolyCoords tMin = a_pPolys->pVertices[0];
		TPolyCoords tMax = tMin;
		float fArea = 0.0f;
		for (TIconPolySpec const* p = a_pPolys; p != a_pPolys+a_nPolys; ++p)
		{
			for (int i = 0; i < p->nVertices; ++i)
			{
				if (tMin.fX > p->pVertices[i].fX) tMin.fX = p->pVertices[i].fX;
				if (tMin.fY > p->pVertices[i].fY) tMin.fY = p->pVertices[i].fY;
				if (tMax.fX < p->pVertices[i].fX) tMax.fX = p->pVertices[i].fX;
				if (tMax.fY < p->pVertices[i].fY) tMax.fY = p->pVertices[i].fY;
			}
			for (int i = 0; i < p->nVertices; ++i)
			{
				int j = (i+1)%p->nVertices;
				fArea += (p->pVertices[i].fY + p->pVertices[j].fY) * (p->pVertices[j].fX - p->pVertices[i].fX);
			}
		}
		fArea *= 0.5f/a_nPolys;
		tCenter.fX = 0.5f*(tMax.fX+tMin.fX);
		tCenter.fY = 0.5f*(tMax.fY+tMin.fY);
		fScale = (a_nSize*7.0f/8.0f)/max(tMax.fX-tMin.fX, tMax.fY-tMin.fY);

		float fSide = sqrtf(fArea);
		if (fScale > 0.5*a_nSize/fSide) fScale = sqrtf(fScale*0.5*a_nSize/fSide);
	}

	DWORD nXOR = a_nSize*a_nSize<<2;
	DWORD nAND = a_nSize*((((a_nSize+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pHead->biSize = sizeof(BITMAPINFOHEADER);
	pHead->biWidth = a_nSize;
	pHead->biHeight = a_nSize<<1;
	pHead->biPlanes = 1;
	pHead->biBitCount = 32;
	pHead->biCompression = BI_RGB;
	pHead->biSizeImage = nXOR+nAND;
	pHead->biXPelsPerMeter = 0;
	pHead->biYPelsPerMeter = 0;
	DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);

	agg::rendering_buffer rbuf;
	rbuf.attach(reinterpret_cast<agg::int8u*>(pXOR), a_nSize, a_nSize, -a_nSize*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
	// Pixel format and basic primitives renderer
	agg::pixfmt_bgra32 pixf(rbuf);
	agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
	renb.clear(agg::rgba8(0, 0, 0, 0));
	// Scanline renderer for solid filling.
	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
	// Rasterizer & scanline
	agg::rasterizer_scanline_aa<> ras;
	agg::scanline_p8 sl;

	for (TIconPolySpec const* p = a_pPolys; p != a_pPolys+a_nPolys; ++p)
	{
		agg::path_storage path;
		int start = 0;
		int last = -1;
		bool simple = p->interior.a == 0;
		if (p->nVertices == 2)
		{
			float fX0 = p->pVertices[0].fX*fScale-tCenter.fX*fScale+a_nSize*0.5f;
			float fY0 = p->pVertices[0].fY*fScale-tCenter.fY*fScale+a_nSize*0.5f;
			float fX1 = p->pVertices[1].fX*fScale-tCenter.fX*fScale+a_nSize*0.5f;
			float fY1 = p->pVertices[1].fY*fScale-tCenter.fY*fScale+a_nSize*0.5f;
			if (fX0 == fX1)
				fX0 = fX1 = int(fX0)+0.5f;
			else if (fY0 == fY1)
				fY0 = fY1 = int(fY0)+0.5f;
			path.move_to(fX0, fY0);
			path.line_to(fX1, fY1);
		}
		else for (int i = 0; i < p->nVertices; ++i)
		{
			if (last == -1)
			{
				for (last = i+1; last < p->nVertices; ++last)
					if (p->pVertices[i].fX == p->pVertices[last].fX && p->pVertices[i].fY == p->pVertices[last].fY)
						break;
			}
			float fX = p->pVertices[i].fX*fScale-tCenter.fX*fScale+a_nSize*0.5f;
			float fY = p->pVertices[i].fY*fScale-tCenter.fY*fScale+a_nSize*0.5f;
			int prev = i == start ? last-1 : i-1;
			int next = i == last-1 ? start : i+1;
			if (!simple || prev < next)
			{
				if (p->pVertices[i].fX == p->pVertices[prev].fX ||
					p->pVertices[i].fX == p->pVertices[next].fX)
					fX = int(fX)+0.5f;
				if (p->pVertices[i].fY == p->pVertices[prev].fY ||
					p->pVertices[i].fY == p->pVertices[next].fY)
					fY = int(fY)+0.5f;
			}
			if (i > start && p->pVertices[i].fX == p->pVertices[start].fX && p->pVertices[i].fY == p->pVertices[start].fY)
			{
				path.close_polygon();
				start = i+1;
				last = -1;
				simple  = false;
			}
			else if (i == start)
			{
				path.move_to(fX, fY);
			}
			else
				path.line_to(fX, fY);
		}
		if (!simple)
		{
			path.close_polygon();
		}

		if (p->interior.a != 0)
		{
			ren.color(p->interior);
			ras.add_path(path);
			ras.filling_rule(agg::fill_even_odd);
			agg::render_scanlines(ras, sl, ren);
		}

		if (p->outline.a != 0)
		{
			agg::conv_stroke<agg::path_storage> stroke(path);
			stroke.line_join(agg::miter_join);
			stroke.width(1.0f);
			ren.color(p->outline);
			ras.add_path(stroke);
			agg::render_scanlines(ras, sl, ren);
		}
	}

	pixf.demultiply();

	// gamma correction
	agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
	pixf.apply_gamma_inv(gp);

	pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSize*a_nSize));
	int nANDLine = ((((a_nSize+7)>>3)+3)&0xfffffffc);
	for (int y = 0; y < a_nSize; ++y)
	{
		for (int x = 0; x < a_nSize; ++x)
		{
			if (0 == (0xff000000&*pXOR))
			{
				pAND[x>>3] |= 0x80 >> (x&7);
			}
			++pXOR;
		}
		pAND += nANDLine;
	}
	return CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSize, a_nSize, LR_DEFAULTCOLOR);
}

inline HICON IconFromPolygon(int a_nVertices, TPolyCoords const* a_pVertices, agg::rgba8 interior, agg::rgba8 outline, int a_nSize, bool a_bAutoScale)
{
	TIconPolySpec const t = {a_nVertices, a_pVertices, interior, outline};
	return IconFromPolygon(1, &t, a_nSize, a_bAutoScale);
}

inline agg::rgba8 GetIconFillColor()
{
	COLORREF clr3D = GetSysColor(COLOR_3DFACE);
	float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
	float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
	float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
	float const f3DA = 0.64f;
	float const fA = 0.36f;
	return agg::rgba8(255.0f*(f3DR*f3DA+fA)+0.5f, 255.0f*(f3DG*f3DA+fA)+0.5f, 255.0f*(f3DB*f3DA+fA)+0.5f, 255);
}

inline HICON IconFromPolygon(int a_nVertices, TPolyCoords const* a_pVertices, int a_nSize, bool a_bAutoScale)
{
	TIconPolySpec const t = {a_nVertices, a_pVertices, GetIconFillColor(), agg::rgba8(0, 0, 0, 255)};
	return IconFromPolygon(1, &t, a_nSize, a_bAutoScale);
}

inline HICON IconFromPolygon(int a_nVertices1, TPolyCoords const* a_pVertices1, int a_nVertices2, TPolyCoords const* a_pVertices2, int a_nSize, bool a_bAutoScale)
{
	agg::rgba8 const fill = GetIconFillColor();
	TIconPolySpec const t[2] = 
	{
		{a_nVertices1, a_pVertices1, fill, agg::rgba8(0, 0, 0, 255)},
		{a_nVertices2, a_pVertices2, fill, agg::rgba8(0, 0, 0, 255)},
	};
	return IconFromPolygon(2, t, a_nSize, a_bAutoScale);
}

struct CIconPolyFromCurves
{
	CIconPolyFromCurves() : pPolys(NULL), nPolys(0) {}
	CIconPolyFromCurves(int n) : pPolys(NULL), nPolys(n)
	{
		pPolys = new TIconPolySpec[n];
		ZeroMemory(pPolys, sizeof(TIconPolySpec)*n);
	}
	void SetPoly(int i, std::vector<TPolyCoords> const& vertices, agg::rgba8 interior, agg::rgba8 outline)
	{
		pPolys[i].nVertices = vertices.size();
		TPolyCoords* pCoords = new TPolyCoords[vertices.size()];
		pPolys[i].pVertices = pCoords;
		CopyMemory(pCoords, &(vertices[0]), pPolys[i].nVertices*sizeof*pPolys[i].pVertices);
		//std::copy(vertices.begin(), vertices.end(), pPolys[i].pVertices);
		pPolys[i].interior = interior;
		pPolys[i].outline = outline;
	}

	~CIconPolyFromCurves()
	{
		if (pPolys)
		{
			for (TIconPolySpec* p = pPolys; p != pPolys+nPolys; ++p)
				delete[] p->pVertices;
			delete[] pPolys;
			pPolys = NULL;
			nPolys = 0;
		}
	}

	TIconPolySpec* pPolys;
	int nPolys;
};

inline HICON IconFromPath(int a_nPolys, TIconPolySpec const* a_pPolys, int a_nSize, bool a_bAutoScale)
{
	CIconPolyFromCurves poly(a_nPolys);
	std::vector<TPolyCoords> tmp;

	float const fMul = a_bAutoScale ? 1.0f : a_nSize;
	float const fDiv = 1.0f/fMul;
	for (TIconPolySpec const* p = a_pPolys; p != a_pPolys+a_nPolys; ++p)
	{
		if (p->nVertices < 4)
			continue;
		tmp.push_back(p->pVertices[0]);
		for (int v = 1; v+2 < p->nVertices; v+=3)
		{
			if (p->pVertices[v].fX == 0 && p->pVertices[v].fY == 0 && p->pVertices[v+1].fX == 0 && p->pVertices[v+1].fY == 0)
			{
				TPolyCoords t = p->pVertices[v+2];
				if (t.fX != tmp[tmp.size()-1].fX || t.fY != tmp[tmp.size()-1].fY)
					tmp.push_back(t);
			}
			else
			{
				agg::curve4 c(
					fMul*p->pVertices[v-1].fX, fMul*p->pVertices[v-1].fY,
					fMul*(p->pVertices[v-1].fX+p->pVertices[v].fX), fMul*(p->pVertices[v-1].fY+p->pVertices[v].fY),
					fMul*(p->pVertices[v+2].fX+p->pVertices[v+1].fX), fMul*(p->pVertices[v+2].fY+p->pVertices[v+1].fY),
					fMul*p->pVertices[v+2].fX, fMul*p->pVertices[v+2].fY
				);
				double x = 0.0;
				double y = 0.0;
				while (!agg::is_stop(c.vertex(&x, &y)))
				{
					TPolyCoords t = {x*fDiv, y*fDiv};
					if (t.fX != tmp[tmp.size()-1].fX || t.fY != tmp[tmp.size()-1].fY)
						tmp.push_back(t);
				}
			}
		}
		poly.SetPoly(p-a_pPolys, tmp, p->interior, p->outline);
		tmp.clear();
	}
	return IconFromPolygon(poly.nPolys, poly.pPolys, a_nSize, a_bAutoScale);
}
