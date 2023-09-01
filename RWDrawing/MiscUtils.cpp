
#include "stdafx.h"
#include "RWDrawing.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_gamma_lut.h>
#include <GammaCorrection.h>


//HICON IconFromPolygon(int a_nVertices, TPixelCoords const* a_pVertices, int a_nSize, bool a_bAutoScale)
//{
//	float fScale = a_nSize-1;
//	TPixelCoords tCenter = {0.5f, 0.5f};
//	if (a_bAutoScale)
//	{
//		TPixelCoords tMin = a_pVertices[0];
//		TPixelCoords tMax = tMin;
//		for (int i = 1; i < a_nVertices; ++i)
//		{
//			if (tMin.fX > a_pVertices[i].fX) tMin.fX = a_pVertices[i].fX;
//			if (tMin.fY > a_pVertices[i].fY) tMin.fY = a_pVertices[i].fY;
//			if (tMax.fX < a_pVertices[i].fX) tMax.fX = a_pVertices[i].fX;
//			if (tMax.fY < a_pVertices[i].fY) tMax.fY = a_pVertices[i].fY;
//		}
//		float fArea = 0.0f;
//		for (int i = 0; i < a_nVertices; ++i)
//		{
//			int j = (i+1)%a_nVertices;
//			fArea += (a_pVertices[i].fY + a_pVertices[j].fY) * (a_pVertices[j].fX - a_pVertices[i].fX);
//		}
//		fArea *= 0.5f;
//		tCenter.fX = 0.5f*(tMax.fX+tMin.fX);
//		tCenter.fY = 0.5f*(tMax.fY+tMin.fY);
//		fScale = (a_nSize*7.0f/8.0f)/max(tMax.fX-tMin.fX, tMax.fY-tMin.fY);
//
//		float fSide = sqrtf(fArea);
//		if (fScale > 0.5*a_nSize/fSide) fScale = sqrtf(fScale*0.5*a_nSize/fSide);
//	}
//
//	DWORD nXOR = a_nSize*a_nSize<<2;
//	DWORD nAND = a_nSize*((((a_nSize+7)>>3)+3)&0xfffffffc);
//	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
//	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
//	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
//	pHead->biSize = sizeof(BITMAPINFOHEADER);
//	pHead->biWidth = a_nSize;
//	pHead->biHeight = a_nSize<<1;
//	pHead->biPlanes = 1;
//	pHead->biBitCount = 32;
//	pHead->biCompression = BI_RGB;
//	pHead->biSizeImage = nXOR+nAND;
//	pHead->biXPelsPerMeter = 0;
//	pHead->biYPelsPerMeter = 0;
//	DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
//
//	agg::rendering_buffer rbuf;
//	rbuf.attach(reinterpret_cast<agg::int8u*>(pXOR), a_nSize, a_nSize, -a_nSize*4); // Use negative stride in order to keep Y-axis consistent with WinGDI, i.e., going down.
//	// Pixel format and basic primitives renderer
//	agg::pixfmt_bgra32 pixf(rbuf);
//	agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
//	renb.clear(agg::rgba8(0, 0, 0, 0));
//	// Scanline renderer for solid filling.
//	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
//	// Rasterizer & scanline
//	agg::rasterizer_scanline_aa<> ras;
//	agg::scanline_p8 sl;
//
//    agg::path_storage path;
//	for (int i = 0; i < a_nVertices; ++i)
//	{
//		float fX = a_pVertices[i].fX*fScale-tCenter.fX*fScale+a_nSize*0.5f;
//		float fY = a_pVertices[i].fY*fScale-tCenter.fY*fScale+a_nSize*0.5f;
//		if (a_pVertices[i].fX == a_pVertices[(i+a_nVertices-1)%a_nVertices].fX ||
//			a_pVertices[i].fX == a_pVertices[(i+1)%a_nVertices].fX)
//			fX = int(fX)+0.5f;
//		if (a_pVertices[i].fY == a_pVertices[(i+a_nVertices-1)%a_nVertices].fY ||
//			a_pVertices[i].fY == a_pVertices[(i+1)%a_nVertices].fY)
//			fY = int(fY)+0.5f;
//		if (i)
//			path.line_to(fX, fY);
//		else
//			path.move_to(fX, fY);
//	}
//    path.close_polygon();
//
//	COLORREF clr3D = GetSysColor(COLOR_3DFACE);
//	float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
//	float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
//	float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
//	float const f3DA = 0.64f;
//	float const fA = 0.36f;
//	ren.color(agg::rgba8(255.0f*(f3DR*f3DA+fA)+0.5f, 255.0f*(f3DG*f3DA+fA)+0.5f, 255.0f*(f3DB*f3DA+fA)+0.5f, 255));
//	ras.add_path(path);
//	agg::render_scanlines(ras, sl, ren);
//
//	agg::conv_stroke<agg::path_storage> stroke(path);
//	stroke.line_join(agg::miter_join);
//	stroke.width(1.0f);
//	ren.color(agg::rgba8(0, 0, 0, 255));
//	ras.add_path(stroke);
//	agg::render_scanlines(ras, sl, ren);
//
//	pixf.demultiply();
//
//	// gamma correction
//	agg::gamma_lut<agg::int8u, agg::int8u, 8, 8> gp(2.2);
//	pixf.apply_gamma_inv(gp);
//
//	pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
//	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSize*a_nSize));
//	int nANDLine = ((((a_nSize+7)>>3)+3)&0xfffffffc);
//	for (int y = 0; y < a_nSize; ++y)
//	{
//		for (int x = 0; x < a_nSize; ++x)
//		{
//			if (0 == (0xff000000&*pXOR))
//			{
//				pAND[x>>3] |= 0x80 >> (x&7);
//			}
//			++pXOR;
//		}
//		pAND += nANDLine;
//	}
//	return CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSize, a_nSize, LR_DEFAULTCOLOR);
//}

TRasterImagePixel TColorToTRasterImagePixel(TColor const& a_tColor, float a_fGamma)
{
	TRasterImagePixel t;
	if (a_fGamma <= 0.0f || (a_fGamma >= 2.1f && a_fGamma <= 2.3f))
	{
		t.bR = CGammaTables::ToSRGB(a_tColor.fR);
		t.bG = CGammaTables::ToSRGB(a_tColor.fG);
		t.bB = CGammaTables::ToSRGB(a_tColor.fB);
	}
	else
	{
		if (a_fGamma < 0.1f || a_fGamma > 10.0f) a_fGamma = 1.0f/2.2f; else a_fGamma = 1.0f/a_fGamma;
		t.bR = a_tColor.fR >= 1.0f ? 255 : (a_tColor.fR <= 0.0f ? 0 : BYTE(powf(a_tColor.fR, a_fGamma)*255.0f+0.5f));
		t.bG = a_tColor.fG >= 1.0f ? 255 : (a_tColor.fG <= 0.0f ? 0 : BYTE(powf(a_tColor.fG, a_fGamma)*255.0f+0.5f));
		t.bB = a_tColor.fB >= 1.0f ? 255 : (a_tColor.fB <= 0.0f ? 0 : BYTE(powf(a_tColor.fB, a_fGamma)*255.0f+0.5f));
	}
	t.bA = a_tColor.fA >= 1.0f ? 255 : (a_tColor.fA <= 0.0f ? 0 : BYTE(a_tColor.fA*255.0f+0.5f));
	return t;
}

#include "FillStyleSolid.h"

void ActivateSolidFill(IRasterImageEditWindow* a_pWindow, TColor const& a_tColor)
{
	CFillStyleDataSolid cSolid;
	cSolid.tColor = a_tColor;
	CComObject<CSharedStateToolData>* pData = NULL;
	CComObject<CSharedStateToolData>::CreateInstance(&pData);
	CComPtr<ISharedState> pState = pData;
	pData->Init(cSolid);
	a_pWindow->SetBrushState(CComBSTR(L"SOLID"), pState);
}
