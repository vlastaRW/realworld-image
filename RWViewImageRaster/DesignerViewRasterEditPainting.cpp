// DesignerViewRasterEdit.cpp : Implementation of window area rendering of CDesignerViewRasterEdit

#include "stdafx.h"
#include "DesignerViewRasterEdit.h"

#include <math.h>


// CDesignerViewRasterEdit

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_conv_dash.h>

static float hls_value(float n1, float n2, float h)
{
	h += 360.0f;
    float hue = h - 360.0f*(int)(h/360.0f);

	if (hue < 60.0f)
		return n1 + ( n2 - n1 ) * hue / 60.0f;
	else if (hue < 180.0f)
		return n2;
	else if (hue < 240.0f)
		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
	else
		return n1;
}

void CDesignerViewRasterEdit::PostRenderImage(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE szZoomed = M_ZoomedSize();
	SIZE szOrig = M_ImageSize();
	float fZoomX = float(szZoomed.cx)/szOrig.cx;
	float fZoomY = float(szZoomed.cy)/szOrig.cy;

	// draw help lines
	float fXMin = fZoomX*m_fHelpLinesXMin+M_ImagePos().x;
	float fYMin = fZoomY*m_fHelpLinesYMin+M_ImagePos().y;
	float fXMax = fZoomX*m_fHelpLinesXMax+M_ImagePos().x;
	float fYMax = fZoomY*m_fHelpLinesYMax+M_ImagePos().y;
	LONG nXMin = fXMin-2;
	LONG nYMin = fYMin-2;
	LONG nXMax = fXMax+2;
	LONG nYMax = fYMax+2;
	if (fXMax >= fXMin && fYMax >= fYMin &&
		a_rcDirty.left < nXMax && a_rcDirty.top < nYMax &&
		a_rcDirty.right > nXMin && a_rcDirty.bottom > nYMin)
	{
		agg::rendering_buffer rbuf;
		rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, a_nStride*sizeof*a_pBuffer);
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

        agg::path_storage path;
		for (CHelpLines::const_iterator i = m_cHelpLines.begin(); i != m_cHelpLines.end(); ++i)
		{
			if (i->bLineTo)
			{
				path.line_to(fZoomX*i->fX+M_ImagePos().x-a_rcDirty.left, fZoomY*i->fY+M_ImagePos().y-a_rcDirty.top);
				if (i->bClose)
					path.close_polygon();
			}
			else
			{
				path.move_to(fZoomX*i->fX+M_ImagePos().x-a_rcDirty.left, fZoomY*i->fY+M_ImagePos().y-a_rcDirty.top);
			}
		}
		agg::conv_dash<agg::path_storage> dash(path);
		dash.add_dash(5, 5);
		agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke(dash);
		stroke.line_join(agg::bevel_join);
		stroke.line_cap(agg::butt_cap);
		stroke.width(1.0f);
		ren.color(agg::rgba8(255, 255, 255, 255));
		ras.add_path(stroke);
		agg::render_scanlines(ras, sl, ren);

		agg::conv_dash<agg::path_storage> dash2(path);
		dash2.add_dash(5, 5);
		dash2.dash_start(5);
		agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke2(dash2);
		stroke2.line_join(agg::bevel_join);
		stroke2.line_cap(agg::butt_cap);
		stroke2.width(1.0f);
		ren.color(agg::rgba8(0, 0, 0, 255));
		ras.add_path(stroke2);
		agg::render_scanlines(ras, sl, ren);
	}

	// draw control handles
	if (!M_HideHandles() || !m_bMouseOut)
	{
		for (CHandles::const_iterator i = m_cHandles.begin(); i != m_cHandles.end(); ++i)
		{
			TPixelCoords tPos = i->first;
			ULONG nClass = i->second;
			LONG nX = fZoomX*tPos.fX+M_ImagePos().x;
			LONG nY = fZoomY*tPos.fY+M_ImagePos().y;
			RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
			RECT rcIntersection =
			{
				max(rcPt.left, a_rcDirty.left),
				max(rcPt.top, a_rcDirty.top),
				min(rcPt.right, a_rcDirty.right),
				min(rcPt.bottom, a_rcDirty.bottom),
			};
			if (rcIntersection.left >= rcIntersection.right || rcIntersection.top >= rcIntersection.bottom)
				continue;
			bool const hot = m_nHotHandle == i-m_cHandles.begin() || (m_eDragState == EDSHandle && m_nHandleIndex == i-m_cHandles.begin());
			float const l = hot ? 0.9f : 0.8f;
			float const s = hot ? 1.0f : 0.9f;
			float const m2 = l + (l <= 0.5f ? l*s : s - l*s);
			float const m1 = 2.0f * l - m2;
			double hue = double(nClass)*360.0f/4.95f+200.0f;
			hue = 360.0-(hue-360.0f*floor(hue/360.0f));
			float r = hls_value(m1, m2, hue+120.0f);
			float g = hls_value(m1, m2, hue);
			float b = hls_value(m1, m2, hue-120.0f);
			if (r > 1.0f) r = 1.0f;
			if (g > 1.0f) g = 1.0f;
			if (b > 1.0f) b = 1.0f;
			if (r < 0.0f) r = 0.0f;
			if (g < 0.0f) g = 0.0f;
			if (b < 0.0f) b = 0.0f;
			BYTE bClassR = r*255.0f+0.5f;
			BYTE bClassG = g*255.0f+0.5f;
			BYTE bClassB = b*255.0f+0.5f;
			for (LONG y = rcIntersection.top; y < rcIntersection.bottom; ++y)
			{
				BYTE* pO = reinterpret_cast<BYTE*>(a_pBuffer + (y-a_rcDirty.top)*a_nStride + rcIntersection.left-a_rcDirty.left);
				BYTE const* pI = M_HandleImage() + (((y-rcPt.top)*(M_HandleRadius()+M_HandleRadius()+1) + rcIntersection.left-rcPt.left)<<1);
				for (LONG x = rcIntersection.left; x < rcIntersection.right; ++x)
				{
					if (pI[0])
					{
						if (pI[0] == 255)
						{
							pO[0] = (ULONG(pI[1])*bClassB)/255;
							pO[1] = (ULONG(pI[1])*bClassG)/255;
							pO[2] = (ULONG(pI[1])*bClassR)/255;
						}
						else
						{
							ULONG clr = ULONG(pI[1])*pI[0];
							ULONG inv = (255-pI[0])*255;
							pO[0] = (pO[0]*inv + clr*bClassB)/(255*255);
							pO[1] = (pO[1]*inv + clr*bClassG)/(255*255);
							pO[2] = (pO[2]*inv + clr*bClassR)/(255*255);
						}
					}
					pO += 4;
					pI += 2;
				}
			}
		}
	}

	// draw mouse gesture trail
	if (m_eDragState == EDSGesture &&
		m_nGestureXMin <= m_nGestureXMax && m_nGestureYMin <= m_nGestureYMax)
	{
		agg::rendering_buffer rbuf;
		rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, (a_rcDirty.right-a_rcDirty.left)*sizeof*a_pBuffer);
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

        agg::path_storage path;
		path.move_to(m_cGesturePoints.begin()->x-a_rcDirty.left+0.5, m_cGesturePoints.begin()->y-a_rcDirty.top+0.5);
		for (CGesturePoints::const_iterator i = m_cGesturePoints.begin()+1; i != m_cGesturePoints.end(); ++i)
		{
			path.line_to(i->x-a_rcDirty.left+0.5, i->y-a_rcDirty.top+0.5);
		}
		agg::conv_stroke<agg::path_storage> stroke(path);
		stroke.line_join(agg::bevel_join);
		stroke.width(3.0f);
		ren.color(agg::rgba8(255, 50, 50, 160));
		ras.add_path(stroke);
		agg::render_scanlines(ras, sl, ren);
	}
}

/*
LRESULT CDesignerViewRasterEdit::OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	UpdateMarginImgCache(m_eStyle == CFGVAL_2DEDIT_STYLECLASSIC ? M_Background() : 0xff000000);
	UpdateImageSize();
	UpdateImagePos();

	CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
	PAINTSTRUCT ps;
	RECT rcDirty = {0, 0, m_tClientSize.cx, m_tClientSize.cy};
	if (a_wParam == NULL)
	{
		cDC = BeginPaint(&ps);
		rcDirty = ps.rcPaint;
	}
	CAutoVectorPtr<COLORREF> cBB;
	SIZE const szDirty = {rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top};
	int const nDirtyPixels = szDirty.cx*szDirty.cy;
	{
		if (nDirtyPixels == 0 || !cBB.Allocate(nDirtyPixels))
		{
			if (a_wParam == NULL)
				EndPaint(&ps);
			return 0;
		}
	}

	if ((m_eImageQuality&EIQMultithreaded) && ProcessorCount() > 1 && szDirty.cx >= 4 && szDirty.cy >= 4 && 
		(nDirtyPixels >= 128*128 || nDirtyPixels >= 128*128*m_fZoom*m_fZoom))
	{
		CReadLock<IDocument> cLock(m_pDoc);

		// TODO: keep and reuse the threads
		LONG nSplitY = (rcDirty.top+rcDirty.bottom+1)>>1;
		if (ProcessorCount() >= 4)
		{
			// TODO: support up to 32 cpus

			// 4 threads
			LONG nSplitX = (rcDirty.left+rcDirty.right+1)>>1;
			SRenderThreadInfo sInfo1 = {this, rcDirty, cBB.m_p+nSplitX-rcDirty.left, szDirty.cx};
			SRenderThreadInfo sInfo2 = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top), szDirty.cx};
			SRenderThreadInfo sInfo3 = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top)+nSplitX-rcDirty.left, szDirty.cx};
			RECT rc2 = rcDirty;
			sInfo2.rcDirty.top = sInfo3.rcDirty.top = sInfo1.rcDirty.bottom = rc2.bottom = nSplitY;
			sInfo1.rcDirty.left = sInfo3.rcDirty.left = sInfo2.rcDirty.right = rc2.right = nSplitX;
			unsigned uThID;
			HANDLE hFinished[3] =
			{
				(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo1, 0, &uThID),
				(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo2, 0, &uThID),
				(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo3, 0, &uThID),
			};
			try { RenderImage(rc2, cBB, szDirty.cx); } catch (...) {}
			WaitForMultipleObjects(3, hFinished, TRUE, INFINITE);
			CloseHandle(hFinished[0]);
			CloseHandle(hFinished[1]);
			CloseHandle(hFinished[2]);
		}
		else
		{
			// 2 threads
			SRenderThreadInfo sInfo = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top), szDirty.cx};
			RECT rc2 = rcDirty;
			sInfo.rcDirty.top = rc2.bottom = nSplitY;
			unsigned uThID;
			HANDLE hFinished = (HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo, 0, &uThID);
			try { RenderImage(rc2, cBB, szDirty.cx); } catch (...) {}
			//AtlWaitWithMessageLoop(hFinished);
			WaitForSingleObject(hFinished, INFINITE);
			CloseHandle(hFinished);
		}
	}
	else
	{
		// 1 thread
		try { RenderImage(rcDirty, cBB, szDirty.cx); } catch (...) {}
	}

	BITMAPINFO tBI;
	ZeroMemory(&tBI, sizeof tBI);
	tBI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	tBI.bmiHeader.biWidth = rcDirty.right-rcDirty.left;
	tBI.bmiHeader.biHeight = rcDirty.top-rcDirty.bottom;
	tBI.bmiHeader.biPlanes = 1;
	tBI.bmiHeader.biBitCount = 32;
	tBI.bmiHeader.biCompression = BI_RGB;
	cDC.SetDIBitsToDevice(rcDirty.left, rcDirty.top, tBI.bmiHeader.biWidth, -tBI.bmiHeader.biHeight, 0, 0, 0, -tBI.bmiHeader.biHeight, cBB.m_p, &tBI, 0);

	if (a_wParam == NULL)
		EndPaint(&ps);
	return 0;
}

unsigned __stdcall CDesignerViewRasterEdit::RenderImageThreadProc(void* a_p)
{
	SRenderThreadInfo const* pInfo = reinterpret_cast<SRenderThreadInfo*>(a_p);
	try
	{
		pInfo->pThis->RenderImage(pInfo->rcDirty, pInfo->pBuffer, pInfo->nStride);
	}
	catch (...)
	{
	}
	return 0;
}

static int const TRANSPARENCY_SHIFT = 2;
static int const TRANSPARENCY_GRID = 1<<TRANSPARENCY_SHIFT;

struct TRasterImagePixel16
{
	WORD wB, wG, wR, wA;
};

void CDesignerViewRasterEdit::RenderTile100Percent(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};

	RECT rcPP = a_rcTileInImg;
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->PreProcessTile(&rcPP);
	TRasterImagePixel* pSrc;
	ULONG nSrcStride;
	CAutoVectorPtr<TRasterImagePixel> cPPBuffer;
	if (rcPP.left != a_rcTileInImg.left || rcPP.right != a_rcTileInImg.right || 
		rcPP.top != a_rcTileInImg.top || rcPP.bottom != a_rcTileInImg.bottom)
	{
		cPPBuffer.Allocate((rcPP.right-rcPP.left)*(rcPP.bottom-rcPP.top));
		nSrcStride = rcPP.right-rcPP.left;
		pSrc = cPPBuffer.m_p+(a_rcTileInImg.top-rcPP.top)*nSrcStride+a_rcTileInImg.left-rcPP.left;
		m_pActiveTool->GetImageTile(rcPP.left, rcPP.top, rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, m_fGamma, nSrcStride, cPPBuffer);
		if (m_bShowComposed && m_pComposedPreview)
			m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, nSrcStride, cPPBuffer);
		if (m_bShowInverted)
			RenderTileInvert(rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, cPPBuffer, rcPP.right-rcPP.left);
	}
	else
	{
		m_pActiveTool->GetImageTile(a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, m_fGamma, a_nStride, reinterpret_cast<TRasterImagePixel*>(a_pBuffer));
		pSrc = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		nSrcStride = a_nStride;
		if (m_bShowComposed && m_pComposedPreview)
			m_pComposedPreview->ProcessTile(m_eComposedMode, a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, a_nStride, reinterpret_cast<TRasterImagePixel*>(a_pBuffer));
		if (m_bShowInverted)
			RenderTileInvert(szTileInImg.cx, szTileInImg.cy, pSrc, nSrcStride);
	}
	RECT rcSelection = a_rcTileInImg;
	BOOL bEverythingSelected = TRUE;
	m_pActiveTool->GetSelectionInfo(&rcSelection, &bEverythingSelected);
	BOOL const bNothingSelected = rcSelection.left > a_rcTileInImg.right || rcSelection.top > a_rcTileInImg.bottom || rcSelection.right < a_rcTileInImg.left || rcSelection.bottom < a_rcTileInImg.top;
	if (bNothingSelected || rcSelection.left > a_rcTileInImg.left || rcSelection.top > a_rcTileInImg.top || rcSelection.right < a_rcTileInImg.right || rcSelection.bottom < a_rcTileInImg.bottom)
		bEverythingSelected = FALSE;
	CAutoVectorPtr<BYTE> cSelBuf;
	if (!bEverythingSelected && !bNothingSelected)
	{
		if (!cSelBuf.Allocate(szTileInImg.cx*szTileInImg.cy) ||
			FAILED(m_pActiveTool->GetSelectionTile(a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, szTileInImg.cx, cSelBuf.m_p)))
		{
			cSelBuf.Free();
			bEverythingSelected = TRUE;
		}
	}
	ULONG nSelR = GetRValue(m_tSelection);
	ULONG nSelG = GetGValue(m_tSelection);
	ULONG nSelB = GetBValue(m_tSelection);
	ULONG const nGSelR = m_aGammaF[nSelR];
	ULONG const nGSelG = m_aGammaF[nSelG];
	ULONG const nGSelB = m_aGammaF[nSelB];
	TRasterImagePixel aTransparency[2];
	aTransparency[0].bR = GetRValue(M_Square2());
	aTransparency[0].bG = GetGValue(M_Square2());
	aTransparency[0].bB = GetBValue(M_Square2());
	aTransparency[1].bR = GetRValue(M_Square1());
	aTransparency[1].bG = GetGValue(M_Square1());
	aTransparency[1].bB = GetBValue(M_Square1());
	TRasterImagePixel16 aGTransparency[2];
	aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
	aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
	aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
	aGTransparency[1].wR = m_aGammaF[aTransparency[1].bR];
	aGTransparency[1].wG = m_aGammaF[aTransparency[1].bG];
	aGTransparency[1].wB = m_aGammaF[aTransparency[1].bB];
	if (bNothingSelected)
	{
		aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
		aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
		aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
		aGTransparency[1].wR = (aGTransparency[1].wR+nGSelR)>>1;
		aGTransparency[1].wG = (aGTransparency[1].wG+nGSelG)>>1;
		aGTransparency[1].wB = (aGTransparency[1].wB+nGSelB)>>1;
		aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
		aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
		aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
		aTransparency[1].bR = m_aGammaB[aGTransparency[1].wR];
		aTransparency[1].bG = m_aGammaB[aGTransparency[1].wG];
		aTransparency[1].bB = m_aGammaB[aGTransparency[1].wB];
	}
	BYTE* pS = cSelBuf.m_p;
	for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y)
	{
		TRasterImagePixel* pD = reinterpret_cast<TRasterImagePixel*>(a_pBuffer)+(y-a_rcTileInImg.top)*a_nStride;
		TRasterImagePixel const* p = pSrc+(y-a_rcTileInImg.top)*nSrcStride;
		TRasterImagePixel const* const pEnd = p+szTileInImg.cx;
		ULONG nTotalA = 0;
		for (TRasterImagePixel const* pA = p; pA != pEnd; ++pA)
			nTotalA += pA->bA;
		if (nTotalA == 0)
		{
			// everything transparent
			int const yy = y>>TRANSPARENCY_SHIFT;
			if (bEverythingSelected || bNothingSelected)
			{
				for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
				{
					int const xx = x>>TRANSPARENCY_SHIFT;
					*pD = aTransparency[(xx^yy)&1];
					++pD;
				}
			}
			else
			{
				// partially selected empty region
				for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
				{
					int const xx = x>>TRANSPARENCY_SHIFT;
					TRasterImagePixel16 const& t = aGTransparency[(xx^yy)&1];
					ULONG const bDir = 255-*pS;
					ULONG const bInv = 257+*pS;
					pD->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
					pD->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
					pD->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					++pD;
					++pS;
				}
			}
		}
		else if (nTotalA == 255*szTileInImg.cx)
		{
			 // everything opaque
			if (bEverythingSelected)
			{
				while (p != pEnd)
				{
					*pD = *p;
					++pD;
					++p;
				}
			}
			else if (bNothingSelected)
			{
				while (p != pEnd)
				{
					BYTE const bB = p->bB;
					BYTE const bG = p->bG;
					BYTE const bR = p->bR;
					pD->bB = m_aGammaB[(m_aGammaF[bB] + nGSelB)>>1];
					pD->bG = m_aGammaB[(m_aGammaF[bG] + nGSelG)>>1];
					pD->bR = m_aGammaB[(m_aGammaF[bR] + nGSelR)>>1];
					++pD;
					++p;
				}
			}
			else
			{
				while (p != pEnd)
				{
					ULONG const bDir = 255-*pS;
					ULONG const bInv = 257+*pS;
					BYTE const bB = p->bB;
					BYTE const bG = p->bG;
					BYTE const bR = p->bR;
					pD->bB = m_aGammaB[(m_aGammaF[bB]*bInv + nGSelB*bDir)>>9];
					pD->bG = m_aGammaB[(m_aGammaF[bG]*bInv + nGSelG*bDir)>>9];
					pD->bR = m_aGammaB[(m_aGammaF[bR]*bInv + nGSelR*bDir)>>9];
					++pD;
					++p;
					++pS;
				}
			}
		}
		else
		{
			// semitransparent
			int const yy = y>>TRANSPARENCY_SHIFT;
			if (bEverythingSelected)
			{
				for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
				{
					BYTE const bB = p->bB;
					BYTE const bG = p->bG;
					BYTE const bR = p->bR;
					if (p->bA == 255)
					{
						pD->bB = bB;
						pD->bG = bG;
						pD->bR = bR;
					}
					else
					{
						int const xx = x>>TRANSPARENCY_SHIFT;
						TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
						ULONG const bDir = p->bA*0x101+1;
						ULONG const bInv = 0x10000-bDir;
						pD->bB = m_aGammaB[(t1.wB*bInv + m_aGammaF[bB]*bDir)>>16];
						pD->bG = m_aGammaB[(t1.wG*bInv + m_aGammaF[bG]*bDir)>>16];
						pD->bR = m_aGammaB[(t1.wR*bInv + m_aGammaF[bR]*bDir)>>16];
					}
					++pD;
					++p;
				}
			}
			else if (bNothingSelected)
			{
				for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
				{
					BYTE const bB = p->bB;
					BYTE const bG = p->bG;
					BYTE const bR = p->bR;
					if (p->bA == 255)
					{
						pD->bB = m_aGammaB[(m_aGammaF[bB] + nGSelB)>>1];
						pD->bG = m_aGammaB[(m_aGammaF[bG] + nGSelG)>>1];
						pD->bR = m_aGammaB[(m_aGammaF[bR] + nGSelR)>>1];
					}
					else
					{
						int const xx = x>>TRANSPARENCY_SHIFT;
						TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
						ULONG const bInv = 0xffff-p->bA*0x101;
						ULONG const bDir = (0x10000-bInv)>>1;
						pD->bB = m_aGammaB[(t1.wB*bInv + (m_aGammaF[bB] + nGSelB)*bDir)>>16];
						pD->bG = m_aGammaB[(t1.wG*bInv + (m_aGammaF[bG] + nGSelG)*bDir)>>16];
						pD->bR = m_aGammaB[(t1.wR*bInv + (m_aGammaF[bR] + nGSelR)*bDir)>>16];
					}
					++pD;
					++p;
				}
			}
			else
			{
				// partially selected and partially transparent
				for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
				{
					ULONG const bDir = 255-*pS;
					ULONG const bInv = 257+*pS;
					BYTE const bB = p->bB;
					BYTE const bG = p->bG;
					BYTE const bR = p->bR;
					if (p->bA == 255)
					{
						pD->bB = m_aGammaB[(m_aGammaF[bB]*bInv + nGSelB*bDir)>>9];
						pD->bG = m_aGammaB[(m_aGammaF[bG]*bInv + nGSelG*bDir)>>9];
						pD->bR = m_aGammaB[(m_aGammaF[bR]*bInv + nGSelR*bDir)>>9];
					}
					else
					{
						int const xx = x>>TRANSPARENCY_SHIFT;
						TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
						ULONG const bDir2 = p->bA*0x101+1;
						ULONG const bInv2 = 0x10000-bDir2;
						ULONG const nTr = (bInv2*bInv)>>3;
						ULONG const nPx = (bDir2*bInv)>>3;
						ULONG const nSl = bDir<<13;
						pD->bB = m_aGammaB[(t1.wB*nTr + m_aGammaF[bB]*nPx + nGSelB*nSl)>>22];
						pD->bG = m_aGammaB[(t1.wG*nTr + m_aGammaF[bG]*nPx + nGSelG*nSl)>>22];
						pD->bR = m_aGammaB[(t1.wR*nTr + m_aGammaF[bR]*nPx + nGSelR*nSl)>>22];
					}
					++p;
					++pD;
					++pS;
				}
			}
		}
	}
}

struct SFrom
{
	ULONG nFrom;
	int nTransparency;
};

struct SFromTo
{
	ULONG nFrom;
	ULONG nTo;
	int nTransparency;
};

struct SFromToBlend
{
	ULONG nFrom1;
	ULONG nTo;
	ULONG nW1;
	int nTransparency;
};

void CDesignerViewRasterEdit::RenderTileNearest(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
	float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
	float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
	RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
	SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

	RECT rcPP = rcSourceTile;
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->PreProcessTile(&rcPP);
	SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
	CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
	TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
	m_pActiveTool->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, m_fGamma, szAdjustedTile.cx, cSrcBuf);
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

	if (m_bShowInverted)
		RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

	RECT rcSelection = rcSourceTile;
	BOOL bEverythingSelected = TRUE;
	m_pActiveTool->GetSelectionInfo(&rcSelection, &bEverythingSelected);
	BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
	if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
		bEverythingSelected = FALSE;
	CAutoVectorPtr<BYTE> cSelBuf;
	if (!bEverythingSelected && !bNothingSelected)
	{
		if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
			FAILED(m_pActiveTool->GetSelectionTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p)))
		{
			cSelBuf.Free();
			bEverythingSelected = TRUE;
		}
	}
	ULONG const nSelR = GetRValue(m_tSelection);
	ULONG const nSelG = GetGValue(m_tSelection);
	ULONG const nSelB = GetBValue(m_tSelection);
	COLORREF aTransparency[2] = {((M_Square2()&0xff)<<16)|(M_Square2()&0xff00)|((M_Square2()&0xff0000)>>16), ((M_Square1()&0xff)<<16)|(M_Square1()&0xff00)|((M_Square1()&0xff0000)>>16)};
	if (bNothingSelected)
	{
		aTransparency[0] = RGB((GetRValue(aTransparency[0])+nSelR)>>1, (GetGValue(aTransparency[0])+nSelG)>>1, (GetBValue(aTransparency[0])+nSelB)>>1);
		aTransparency[1] = RGB((GetRValue(aTransparency[1])+nSelR)>>1, (GetGValue(aTransparency[1])+nSelG)>>1, (GetBValue(aTransparency[1])+nSelB)>>1);
	}
	TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
	TRasterImagePixel* pI = pSrcBuf;
	BYTE* pS = cSelBuf.m_p;
	CAutoVectorPtr<SFrom> cSimple(new SFrom[szTileInImg.cx]);
	float fTransparency = 0.5f;
	while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
		fTransparency *= 2.0f;
	{
		// prepare guides for x-resizing
		for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
		{
			cSimple[x-a_rcTileInImg.left].nFrom = (x+0.5f)/fZoomX-rcSourceTile.left;
			cSimple[x-a_rcTileInImg.left].nTransparency = int((x+0.5f)/(fTransparency*fZoomX))&1;
		}
	}
	int iLastY = -1;
	int iLastTransparent = -1;
	int iLastAlpha = -1;
	ULONG nLastTotalAlpha = 0;
	for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO+=a_nStride-szTileInImg.cx)
	{
		int nTR1 = int((y+0.5f)/(fTransparency*fZoomY))&1;
		int nY1 = (y+0.5f)/fZoomY-rcSourceTile.top;
		int const nY2 = (y+1)/fZoomY;
		ULONG const nR1 = 256.0f*(nY2*fZoomY-y)+0.5f;
		ULONG const nR2 = 256-nR1;
		if (nY1 == iLastY && nTR1 == iLastTransparent)
		{
			// copy the previous row
			CopyMemory(pO, pO-a_nStride, szTileInImg.cx*sizeof(*pO));
			pO += szTileInImg.cx;
			continue;
		}

		// just a single row
		iLastY = nY1;
		iLastTransparent = nTR1;
		TRasterImagePixel const* pI1 = pI+nY1*szAdjustedTile.cx;
		SFrom const* const pFEnd = cSimple.m_p+szTileInImg.cx;

		ULONG nTotalA = 0;
		if (iLastAlpha == nY1)
		{
			nTotalA = nLastTotalAlpha;
		}
		else
		{
			if (szSourceTile.cx <= szTileInImg.cx)
			{
				TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
					nTotalA += pA->bA;
				iLastAlpha == nY1;
				nLastTotalAlpha = nTotalA;
			}
			else
			{
				for (SFrom* pF = cSimple; pF != pFEnd; ++pF)
					nTotalA += pI1[pF->nFrom].bA;
				iLastAlpha == nY1;
				nLastTotalAlpha = nTotalA;
			}
		}

		if (nTotalA == 0)
		{
			// transparent row
			if (bEverythingSelected || bNothingSelected)
			{
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					*pO = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
			}
			else
			{
				// partially selected empty region
				BYTE const* pS1 = pS+nY1*szSourceTile.cx;
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					ULONG const bDir = 255-pS1[pF->nFrom];
					ULONG const bInv = 512-bDir;
					TRasterImagePixel const t = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
					pO->bB = (t.bB*bInv + nSelB*bDir)>>9;
					pO->bG = (t.bG*bInv + nSelG*bDir)>>9;
					pO->bR = (t.bR*bInv + nSelR*bDir)>>9;
				}
			}
		}
		else if (nTotalA == 255*szSourceTile.cx)
		{
			// opaque row
			if (bEverythingSelected)
			{
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					*pO = pI1[pF->nFrom];
				}
			}
			else if (bNothingSelected)
			{
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					TRasterImagePixel const* const pI = pI1+pF->nFrom;
					pO->bB = (pI->bB + nSelB)>>1;
					pO->bG = (pI->bG + nSelG)>>1;
					pO->bR = (pI->bR + nSelR)>>1;
				}
			}
			else
			{
				BYTE const* pS1 = pS+nY1*szSourceTile.cx;
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					ULONG const bDir = 255-pS1[pF->nFrom];
					ULONG const bInv = 512-bDir;
					TRasterImagePixel const* const pI = pI1+pF->nFrom;
					pO->bB = (pI->bB*bInv + nSelB*bDir)>>9;
					pO->bG = (pI->bG*bInv + nSelG*bDir)>>9;
					pO->bR = (pI->bR*bInv + nSelR*bDir)>>9;
				}
			}
		}
		else
		{
			// semitransparent row
			if (bEverythingSelected)
			{
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
					TRasterImagePixel const t2 = pI1[pF->nFrom];
					ULONG const bDir2 = t2.bA*0x10101+1;
					ULONG const bInv2 = 0x1000000-bDir2;
					pO->bB = (t1.bB*bInv2 + t2.bB*bDir2)>>24;
					pO->bG = (t1.bG*bInv2 + t2.bG*bDir2)>>24;
					pO->bR = (t1.bR*bInv2 + t2.bR*bDir2)>>24;
				}
			}
			else if (bNothingSelected)
			{
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
					TRasterImagePixel const t2 = pI1[pF->nFrom];
					ULONG const bInv2 = 0xffffff-t2.bA*0x10101;
					ULONG const bDir2 = (0x1000000-bInv2)>>1;
					pO->bB = (t1.bB*bInv2 + (t2.bB + nSelB)*bDir2)>>24;
					pO->bG = (t1.bG*bInv2 + (t2.bG + nSelG)*bDir2)>>24;
					pO->bR = (t1.bR*bInv2 + (t2.bR + nSelR)*bDir2)>>24;
				}
			}
			else
			{
				BYTE const* pS1 = pS+nY1*szSourceTile.cx;
				for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
				{
					ULONG const bDir = 255-pS1[pF->nFrom];
					ULONG const bInv = 512-bDir;
					TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
					TRasterImagePixel const t2 = pI1[pF->nFrom];
					ULONG const bDir2 = t2.bA*0x10101+1;
					ULONG const bInv2 = 0x1000000-bDir2;
					BYTE const bB = (t1.bB*bInv2 + t2.bB*bDir2)>>24;
					BYTE const bG = (t1.bG*bInv2 + t2.bG*bDir2)>>24;
					BYTE const bR = (t1.bR*bInv2 + t2.bR*bDir2)>>24;
					pO->bB = (bB*bInv + nSelB*bDir)>>9;
					pO->bG = (bG*bInv + nSelG*bDir)>>9;
					pO->bR = (bR*bInv + nSelR*bDir)>>9;
				}
			}
		}
	}
}

void CDesignerViewRasterEdit::RenderTileInvert(ULONG a_nSizeX, ULONG a_nSizeY, TRasterImagePixel* a_pBuffer, ULONG a_nStride)
{
	for (ULONG y = 0; y < a_nSizeY; ++y, a_pBuffer+=a_nStride-a_nSizeX)
		for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer)
			if (a_pBuffer->bA == 0 && (a_pBuffer->bR || a_pBuffer->bG || a_pBuffer->bB))
				a_pBuffer->bA = 0x80;
}

#define OPAQUEPIXEL_GAMMA(name, source) \
	TRasterImagePixel const t##name = source;\
	WORD const wB##name = m_aGammaF[t##name.bB];\
	WORD const wG##name = m_aGammaF[t##name.bG];\
	WORD const wR##name = m_aGammaF[t##name.bR]
#define TRANSPARENTPIXEL_GAMMA(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bDir##name = t##name.bA*0x4040+1;\
	ULONG const bInv##name = 0x400000-bDir##name;\
	WORD const wB##name = (t1.wB*bInv##name + m_aGammaF[t##name.bB]*bDir##name)>>22;\
	WORD const wG##name = (t1.wG*bInv##name + m_aGammaF[t##name.bG]*bDir##name)>>22;\
	WORD const wR##name = (t1.wR*bInv##name + m_aGammaF[t##name.bR]*bDir##name)>>22
#define TRANSPARENTPIXEL_GAMMA_NOSEL(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bInv##name = 0x3fffff-t##name.bA*0x4040;\
	ULONG const bDir##name = (0x400000-bInv##name)>>1;\
	WORD const wB##name = (t1.wB*bInv##name + (m_aGammaF[t##name.bB] + nGSelB)*bDir##name)>>22;\
	WORD const wG##name = (t1.wG*bInv##name + (m_aGammaF[t##name.bG] + nGSelG)*bDir##name)>>22;\
	WORD const wR##name = (t1.wR*bInv##name + (m_aGammaF[t##name.bR] + nGSelR)*bDir##name)>>22
#define TRANSPARENTPIXELBLEND(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bDir##name = t##name.bA*0x10101+1;\
	ULONG const bInv##name = 0x1000000-bDir##name;\
	BYTE const bB##name = (t1.bB*bInv##name + t##name.bB*bDir##name)>>24;\
	BYTE const bG##name = (t1.bG*bInv##name + t##name.bG*bDir##name)>>24;\
	BYTE const bR##name = (t1.bR*bInv##name + t##name.bR*bDir##name)>>24
#define TRANSPARENTPIXELBLEND_NOSEL(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bInv##name = 0xffffff-t##name.bA*0x10101;\
	ULONG const bDir##name = (0x1000000-bInv##name)>>1;\
	BYTE const bB##name = (t1.bB*bInv##name + (t##name.bB + nSelB)*bDir##name)>>24;\
	BYTE const bG##name = (t1.bG*bInv##name + (t##name.bG + nSelG)*bDir##name)>>24;\
	BYTE const bR##name = (t1.bR*bInv##name + (t##name.bR + nSelR)*bDir##name)>>24
#define WEIGHTEDCOLOR2(output, par1, par2, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2)>>shift
#define WEIGHTEDCOLOR2_GAMMA(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2)>>shift]
#define WEIGHTEDCOLOR2_GAMMA_SELSH(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + (nGSelB<<nSelSh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + (nGSelG<<nSelSh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + (nGSelR<<nSelSh))>>shift]
#define WEIGHTEDCOLOR2_GAMMA_SEL(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + (nGSelB*nSel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + (nGSelG*nSel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + (nGSelR*nSel))>>shift]
#define WEIGHTEDCOLOR4(output, par1, par2, par3, par4, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4)>>shift
#define WEIGHTEDCOLOR4_GAMMA(output, par1, par2, par3, par4, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4)>>shift]
#define WEIGHTEDCOLOR4_GAMMA_SELSH(output, par1, par2, par3, par4, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR4_GAMMA_SEL(output, par1, par2, par3, par4, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR6(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bR = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6)>>shift;\
	output->bB = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6)>>shift
#define WEIGHTEDCOLOR6_GAMMA(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6)>>shift]
#define WEIGHTEDCOLOR6_GAMMA_SELSH(output, par1, par2, par3, par4, par5, par6, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR6_GAMMA_SEL(output, par1, par2, par3, par4, par5, par6, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR9(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + bB##par7*n##par7 + bB##par8*n##par8 + bB##par9*n##par9)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + bG##par7*n##par7 + bG##par8*n##par8 + bG##par9*n##par9)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + bR##par7*n##par7 + bR##par8*n##par8 + bR##par9*n##par9)>>shift
#define WEIGHTEDCOLOR9_GAMMA(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9)>>shift]
#define WEIGHTEDCOLOR9_GAMMA_SELSH(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR9_GAMMA_SEL(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR2_SEL(output, par1, par2, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR4_SEL(output, par1, par2, par3, par4, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR6_SEL(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR9_SEL(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + bB##par7*n##par7 + bB##par8*n##par8 + bB##par9*n##par9 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + bG##par7*n##par7 + bG##par8*n##par8 + bG##par9*n##par9 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + bR##par7*n##par7 + bR##par8*n##par8 + bR##par9*n##par9 + nSelR*nSel)>>shift
#define COMPUTEWEIGHTSR2xC2\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2
#define COMPUTEWEIGHTSR3xC2\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n31 = nR3*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n32 = nR3*nC2
#define COMPUTEWEIGHTSR2xC3\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n13 = nR1*nC3;\
	ULONG const n23 = nR2*nC3
#define COMPUTEWEIGHTSR3xC3\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n31 = nR3*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n32 = nR3*nC2;\
	ULONG const n13 = nR1*nC3;\
	ULONG const n23 = nR2*nC3;\
	ULONG const n33 = nR3*nC3

void CDesignerViewRasterEdit::RenderTileZoomIn(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
	float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
	float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
	RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
	SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

	RECT rcPP = rcSourceTile;
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->PreProcessTile(&rcPP);
	SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
	CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
	TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
	m_pActiveTool->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, m_fGamma, szAdjustedTile.cx, cSrcBuf);
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

	if (m_bShowInverted)
		RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

	RECT rcSelection = rcSourceTile;
	BOOL bEverythingSelected = TRUE;
	m_pActiveTool->GetSelectionInfo(&rcSelection, &bEverythingSelected);
	BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
	if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
		bEverythingSelected = FALSE;
	CAutoVectorPtr<BYTE> cSelBuf;
	if (!bEverythingSelected && !bNothingSelected)
	{
		if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
			FAILED(m_pActiveTool->GetSelectionTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p)))
		{
			cSelBuf.Free();
			bEverythingSelected = TRUE;
		}
	}
	ULONG const nSelR = GetRValue(m_tSelection);
	ULONG const nSelG = GetGValue(m_tSelection);
	ULONG const nSelB = GetBValue(m_tSelection);
	ULONG const nGSelR = m_aGammaF[nSelR];
	ULONG const nGSelG = m_aGammaF[nSelG];
	ULONG const nGSelB = m_aGammaF[nSelB];
	TRasterImagePixel aTransparency[17];
	aTransparency[0].bB = GetBValue(M_Square1());
	aTransparency[0].bG = GetGValue(M_Square1());
	aTransparency[0].bR = GetRValue(M_Square1());
	aTransparency[16].bB = GetBValue(M_Square2());
	aTransparency[16].bG = GetGValue(M_Square2());
	aTransparency[16].bR = GetRValue(M_Square2());
	TRasterImagePixel16 aGTransparency[17];
	aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
	aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
	aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
	aGTransparency[16].wR = m_aGammaF[aTransparency[16].bR];
	aGTransparency[16].wG = m_aGammaF[aTransparency[16].bG];
	aGTransparency[16].wB = m_aGammaF[aTransparency[16].bB];
	if (bNothingSelected)
	{
		aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
		aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
		aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
		aGTransparency[16].wB = (aGTransparency[16].wB+nGSelB)>>1;
		aGTransparency[16].wG = (aGTransparency[16].wG+nGSelG)>>1;
		aGTransparency[16].wR = (aGTransparency[16].wR+nGSelR)>>1;
		aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
		aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
		aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
		aTransparency[16].bR = m_aGammaB[aGTransparency[16].wR];
		aTransparency[16].bG = m_aGammaB[aGTransparency[16].wG];
		aTransparency[16].bB = m_aGammaB[aGTransparency[16].wB];
	}
	for (int i = 1; i < 16; ++i)
	{
		aGTransparency[i].wR = (aGTransparency[0].wR*i + aGTransparency[16].wR*(16-i))>>4;
		aGTransparency[i].wG = (aGTransparency[0].wG*i + aGTransparency[16].wG*(16-i))>>4;
		aGTransparency[i].wB = (aGTransparency[0].wB*i + aGTransparency[16].wB*(16-i))>>4;
		aTransparency[i].bR = m_aGammaB[aGTransparency[i].wR];
		aTransparency[i].bG = m_aGammaB[aGTransparency[i].wG];
		aTransparency[i].bB = m_aGammaB[aGTransparency[i].wB];
	}
	TRasterImagePixel const* const pTransparency = aTransparency+8;
	TRasterImagePixel16 const* const pGTransparency = aGTransparency+8;
	TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
	TRasterImagePixel* pI = pSrcBuf;
	BYTE* pS = cSelBuf.m_p;
	CAutoVectorPtr<SFromTo> cSimple(new SFromTo[szTileInImg.cx]);
	CAutoVectorPtr<SFromToBlend> cBlend(new SFromToBlend[szTileInImg.cx]);
	ULONG nSimple = 0;
	ULONG nCopy = 0;
	ULONG nBlend = 0;
	float fTransparency = 0.5f;
	while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
		fTransparency *= 2.0f;
	{
		// prepare guides for x-resizing
		for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
		{
			int nTX1 = x/(fTransparency*fZoomX);
			int const nTX2 = (x+1)/(fTransparency*fZoomX);
			int nTC1 = 16.0f*(nTX2*fZoomX*fTransparency-x)+0.5f;
			if (nTX1 == nTX2 || nTC1 >= 16)
			{
				nTC1 = (nTX1&1)<<4;
			}
			else
			{
				if (nTC1 <= 0)
				{
					nTX1 = nTX2;
					nTC1 = (nTX1&1)<<4;
				}
				else if (nTX1&1)
					nTC1 = 16-nTC1;
			}
			nTC1 -= 8;

			int nX1 = x/fZoomX;
			int const nX2 = (x+1)/fZoomX;
			ULONG const nC1 = 256.0f*(nX2*fZoomX-x)+0.5f;
			if (nC1 <= 1)
			{
				nX1 = nX2;
			}
			//ULONG const nC2 = 256-nC1;
			if (nX1 == nX2 || nC1 >= 255)
			{
				if (nSimple && cSimple[nSimple-1].nFrom == nX1-rcSourceTile.left && cSimple[nSimple-1].nTransparency == nTC1)
				{
					SFromTo& s = cSimple[szTileInImg.cx-1-(nCopy++)];
					s.nFrom = cSimple[nSimple-1].nTo;
					s.nTo = x-a_rcTileInImg.left;
				}
				else
				{
					SFromTo& s = cSimple[nSimple++];
					s.nFrom = nX1-rcSourceTile.left;
					s.nTo = x-a_rcTileInImg.left;
					s.nTransparency = nTC1;
				}
			}
			else
			{
				SFromToBlend& s = cBlend[nBlend++];
				s.nFrom1 = nX1-rcSourceTile.left;
				s.nTo = x-a_rcTileInImg.left;
				s.nW1 = nC1;
				s.nTransparency = nTC1;
			}
		}
	}
	int iLastY = -1;
	int iLastTransparent = -1;
	int iLastAlpha = -1;
	ULONG nLastTotalAlpha = 0;
	for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride)
	{
		int nTY1 = y/(fTransparency*fZoomY);
		int const nTY2 = (y+1)/(fTransparency*fZoomY);
		int nTR1 = 16.0f*(nTY2*fZoomY*fTransparency-y)+0.5f;
		if (nTY1 == nTY2 || nTR1 >= 16)
		{
			nTR1 = (nTY1&1)<<4;
		}
		else
		{
			if (nTR1 <= 0)
			{
				nTY1 = nTY2;
				nTR1 = (nTY1&1)<<4;
			}
			else if (nTY1&1)
				nTR1 = 16-nTR1;
		}
		nTR1 -= 8;

		int nY1 = y/fZoomY;
		int const nY2 = (y+1)/fZoomY;
		ULONG const nR1 = 256.0f*(nY2*fZoomY-y)+0.5f;
		ULONG const nR2 = 256-nR1;
		if (nR1 <= 1)
		{
			nY1 = nY2;
		}
		if (nY1 == nY2 || nR1 >= 255)
		{
			if (nY1 == iLastY && nTR1 == iLastTransparent)
			{
				// copy the previous row
				CopyMemory(pO, pO-a_nStride, szTileInImg.cx*sizeof(*pO));
				continue;
			}

			// just a single row
			iLastY = nY1;
			iLastTransparent = nTR1;
			TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
			SFromTo const* const pFTEnd = cSimple.m_p+nSimple;
			SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;

			ULONG nTotalA = 0;
			if (iLastAlpha == nY1)
			{
				nTotalA = nLastTotalAlpha;
			}
			else
			{
				TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
					nTotalA += pA->bA;
				iLastAlpha == nY1;
				nLastTotalAlpha = nTotalA;
			}

			if (nTotalA == 0)
			{
				// transparent row
				if (bEverythingSelected || bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						pO[pFT->nTo] = pTransparency[(pFT->nTransparency*nTR1)>>3];
				}
				else
				{
					// partially selected empty region
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-pS1[pFT->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
				}
			}
			else if (nTotalA == 255*szSourceTile.cx)
			{
				// opaque row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1;
						ULONG const n12 = 256-n11;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						pO[pFT->nTo] = pI1[pFT->nFrom];
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1;
						ULONG const n12 = 256-n11;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						int const nSelSh = 8;
						WEIGHTEDCOLOR2_GAMMA_SELSH(pO1, 11, 12, 9);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						TRasterImagePixel const* const pI = pI1+pFT->nFrom;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[(m_aGammaF[pI->bB] + nGSelB)>>1];
						pO1->bG = m_aGammaB[(m_aGammaF[pI->bG] + nGSelG)>>1];
						pO1->bR = m_aGammaB[(m_aGammaF[pI->bR] + nGSelR)>>1];
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1*bInv;
						ULONG const n12 = (bInv<<8)-n11;
						ULONG const nSel = bDir<<8;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 12, 17);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-pS1[pFT->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel const* const pI = pI1+pFT->nFrom;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[(m_aGammaF[pI->bB]*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(m_aGammaF[pI->bG]*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(m_aGammaF[pI->bR]*bInv + nGSelR*bDir)>>9];
					}
				}
			}
			else
			{
				// semitransparent row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1;
						ULONG const n12 = 256-n11;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[wB11];
						pO1->bG = m_aGammaB[wG11];
						pO1->bR = m_aGammaB[wR11];
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1;
						ULONG const n12 = 256-n11;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFT->nFrom]);
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[wB11];
						pO1->bG = m_aGammaB[wG11];
						pO1->bR = m_aGammaB[wR11];
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						ULONG const n11 = pFTB->nW1*bInv;
						ULONG const n12 = (bInv<<8)-n11;
						ULONG const nSel = bDir<<8;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 12, 17);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-pS1[pFT->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[(wB11*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(wG11*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(wR11*bInv + nGSelR*bDir)>>9];
					}
				}
			}
		}
		else
		{
			// blend two rows
			iLastY = -1;
			TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
			TRasterImagePixel const* pI2 = pI1+szAdjustedTile.cx;
			SFromTo const* const pFTEnd = cSimple.m_p+nSimple;
			SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;

			ULONG nTotalA = 0;
			if (iLastAlpha == nY1)
			{
				nTotalA = nLastTotalAlpha;
			}
			else
			{
				TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
					nTotalA += pA->bA;
			}
			{
				iLastAlpha == nY2;
				nLastTotalAlpha = 0;
				TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
					nLastTotalAlpha += pA->bA;
				nTotalA += nLastTotalAlpha;
			}

			if (nTotalA == 0)
			{
				// transparent row
				if (bEverythingSelected || bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						pO[pFT->nTo] = pTransparency[(pFT->nTransparency*nTR1)>>3];
				}
				else
				{
					// partially selected empty region
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
				}
			}
			else if (nTotalA == 510*szSourceTile.cx)
			{
				// opaque row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
						ULONG const n11 = nR1;
						ULONG const n21 = nR2;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SELSH(pO1, 11, 12, 21, 22, 16, 17);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
						ULONG const n11 = nR1;
						ULONG const n21 = nR2;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						int const nSelSh = 8;
						WEIGHTEDCOLOR2_GAMMA_SELSH(pO1, 11, 21, 9);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR2xC2;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
						TRasterImagePixel const* const p1 = pI1+pFT->nFrom;
						TRasterImagePixel const* const p2 = pI2+pFT->nFrom;
						ULONG const n11 = nR1*bInv;
						ULONG const n21 = nR2*bInv;
						ULONG const nSel = bDir<<8;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 21, 17);
					}
				}
			}
			else
			{
				// semitransparent row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFT->nFrom]);
						ULONG const n11 = nR1;
						ULONG const n21 = nR2;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFT->nFrom]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFT->nFrom]);
						ULONG const n11 = nR1;
						ULONG const n21 = nR2;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR2xC2;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
					}
					for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
					{
						ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFT->nFrom]);
						ULONG const n11 = nR1*bInv;
						ULONG const n21 = nR2*bInv;
						ULONG const nSel = bDir<<8;
						TRasterImagePixel* const pO1 = pO+pFT->nTo;
						WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 21, 17);
					}
				}
			}
		}
		SFromTo const* const pFTCEnd = cSimple.m_p+szTileInImg.cx-1-nCopy;
		for (SFromTo const* pFT = cSimple.m_p+szTileInImg.cx-1; pFT != pFTCEnd; --pFT)
			pO[pFT->nTo] = pO[pFT->nFrom];
	}
}

void CDesignerViewRasterEdit::RenderTileZoomOutSimple(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
	float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
	float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
	RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
	SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

	RECT rcPP = rcSourceTile;
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->PreProcessTile(&rcPP);
	SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
	CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
	TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
	m_pActiveTool->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, m_fGamma, szAdjustedTile.cx, cSrcBuf);
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

	if (m_bShowInverted)
		RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

	RECT rcSelection = rcSourceTile;
	BOOL bEverythingSelected = TRUE;
	m_pActiveTool->GetSelectionInfo(&rcSelection, &bEverythingSelected);
	BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
	if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
		bEverythingSelected = FALSE;
	CAutoVectorPtr<BYTE> cSelBuf;
	if (!bEverythingSelected && !bNothingSelected)
	{
		if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
			FAILED(m_pActiveTool->GetSelectionTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p)))
		{
			cSelBuf.Free();
			bEverythingSelected = TRUE;
		}
	}
	ULONG const nSelR = GetRValue(m_tSelection);
	ULONG const nSelG = GetGValue(m_tSelection);
	ULONG const nSelB = GetBValue(m_tSelection);
	ULONG const nGSelR = m_aGammaF[nSelR];
	ULONG const nGSelG = m_aGammaF[nSelG];
	ULONG const nGSelB = m_aGammaF[nSelB];
	TRasterImagePixel aTransparency[17];
	aTransparency[0].bB = GetBValue(M_Square1());
	aTransparency[0].bG = GetGValue(M_Square1());
	aTransparency[0].bR = GetRValue(M_Square1());
	aTransparency[16].bB = GetBValue(M_Square2());
	aTransparency[16].bG = GetGValue(M_Square2());
	aTransparency[16].bR = GetRValue(M_Square2());
	TRasterImagePixel16 aGTransparency[17];
	aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
	aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
	aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
	aGTransparency[16].wR = m_aGammaF[aTransparency[16].bR];
	aGTransparency[16].wG = m_aGammaF[aTransparency[16].bG];
	aGTransparency[16].wB = m_aGammaF[aTransparency[16].bB];
	if (bNothingSelected)
	{
		aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
		aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
		aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
		aGTransparency[16].wB = (aGTransparency[16].wB+nGSelB)>>1;
		aGTransparency[16].wG = (aGTransparency[16].wG+nGSelG)>>1;
		aGTransparency[16].wR = (aGTransparency[16].wR+nGSelR)>>1;
		aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
		aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
		aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
		aTransparency[16].bR = m_aGammaB[aGTransparency[16].wR];
		aTransparency[16].bG = m_aGammaB[aGTransparency[16].wG];
		aTransparency[16].bB = m_aGammaB[aGTransparency[16].wB];
	}
	for (int i = 1; i < 16; ++i)
	{
		aGTransparency[i].wR = (aGTransparency[0].wR*i + aGTransparency[16].wR*(16-i))>>4;
		aGTransparency[i].wG = (aGTransparency[0].wG*i + aGTransparency[16].wG*(16-i))>>4;
		aGTransparency[i].wB = (aGTransparency[0].wB*i + aGTransparency[16].wB*(16-i))>>4;
		aTransparency[i].bR = m_aGammaB[aGTransparency[i].wR];
		aTransparency[i].bG = m_aGammaB[aGTransparency[i].wG];
		aTransparency[i].bB = m_aGammaB[aGTransparency[i].wB];
	}
	TRasterImagePixel const* const pTransparency = aTransparency+8;
	TRasterImagePixel16 const* const pGTransparency = aGTransparency+8;
	TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
	TRasterImagePixel* pI = pSrcBuf;
	BYTE* pS = cSelBuf.m_p;
	CAutoVectorPtr<SFromToBlend> cBlend(new SFromToBlend[szTileInImg.cx]);
	ULONG nBlend = 0;
	ULONG nBlend3 = 0;
	float fTransparency = TRANSPARENCY_GRID;
	while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
		fTransparency *= 2.0f;
	int const nFullX = 256.0f*fZoomX+0.5f;
	int const nMin1X = 256-nFullX;
	{
		// prepare guides for x-resizing
		for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
		{
			int nTX1 = x/(fTransparency*fZoomX);
			int const nTX2 = (x+1)/(fTransparency*fZoomX);
			int nTC1 = 16.0f*(nTX2*fZoomX*fTransparency-x)+0.5f;
			if (nTX1 == nTX2 || nTC1 >= 16)
			{
				nTC1 = (nTX1&1)<<4;
			}
			else
			{
				if (nTC1 <= 0)
				{
					nTX1 = nTX2;
					nTC1 = (nTX1&1)<<4;
				}
				else if (nTX1&1)
					nTC1 = 16-nTC1;
			}
			nTC1 -= 8;

			int nX1 = x/fZoomX;
			int nX2 = (x+1)/fZoomX;
			ULONG nC11 = 256.0f*((nX1+1)*fZoomX-x)+0.5f;
			if (nC11 <= 1)
			{
				// skip the first pixel (weight too small)
				nC11 = nFullX;
				++nX1;
			}
			if (nX1+2 <= nX2 && nC11+nFullX >= 255)
			{
				// skip the last pixel (weight too small)
				--nX2;
			}
			if (nX1 == nX2)
			{
				// zoom is too small - it should be just one pixel, but to keep the algo simple, change it to two with the weight of 0 on the dummy pixel
				if (nX1 == rcSourceTile.left)
					nC11 = 256;
				else
				{
					nC11 = 0;
					--nX1;
				}
			}

			SFromToBlend& s = cBlend[nX1+2 <= nX2 ? szTileInImg.cx-1-(nBlend3++) : nBlend++];
			s.nFrom1 = nX1-rcSourceTile.left;
			s.nTo = x-a_rcTileInImg.left;
			s.nW1 = nC11;
			s.nTransparency = nTC1;
		}
	}
	int iLastAlpha = -1;
	ULONG nLastTotalAlpha = 0;
	int const nFullY = 256.0f*fZoomY+0.5f;
	int const nMin1Y = 256-nFullY;
	for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride)
	{
		int nTY1 = y/(fTransparency*fZoomY);
		int const nTY2 = (y+1)/(fTransparency*fZoomY);
		int nTR1 = 16.0f*(nTY2*fZoomY*fTransparency-y)+0.5f;
		if (nTY1 == nTY2 || nTR1 >= 16)
		{
			nTR1 = (nTY1&1)<<4;
		}
		else
		{
			if (nTR1 <= 0)
			{
				nTY1 = nTY2;
				nTR1 = (nTY1&1)<<4;
			}
			else if (nTY1&1)
				nTR1 = 16-nTR1;
		}
		nTR1 -= 8;

		int nY1 = y/fZoomY;
		int nY2 = (y+1)/fZoomY;
		ULONG nR11 = 256.0f*((nY1+1)*fZoomY-y)+0.5f;
		if (nR11 <= 1)
		{
			// skip the first pixel (weight too small)
			nR11 = nFullY;
			++nY1;
		}
		if (nY1+2 <= nY2 && nR11+nFullY >= 255)
		{
			// skip the last pixel (weight too small)
			--nY2;
		}
		if (nY1 == nY2)
		{
			// damn - zoom is too small - it should be just one row, but to keep the algo simple, change it to two with the weight of 0 on the dummy row
			if (nY1 == rcSourceTile.top)
				nR11 = 256;
			else
			{
				nR11 = 0;
				--nY1;
			}
		}

		TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
		TRasterImagePixel const* pI2 = pI1+szAdjustedTile.cx;
		SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;
		SFromToBlend const* const pFT3End = cBlend.m_p+szTileInImg.cx;

		if (nY1+2 <= nY2)
		{
			// three (or more (but ignored)) rows
			TRasterImagePixel const* pI3 = pI2+szAdjustedTile.cx;
			ULONG const nR1 = nR11;
			ULONG const nR2 = nFullY;
			ULONG const nR3 = nMin1Y-nR11;

			ULONG nTotalA = 0;
			if (iLastAlpha == nY1)
			{
				nTotalA = nLastTotalAlpha;
			}
			else
			{
				TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
					nTotalA += pA->bA;
			}
			{
				TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
					nTotalA += pA->bA;
			}
			{
				iLastAlpha == nY1+2;
				nLastTotalAlpha = 0;
				TRasterImagePixel const* const pEnd = pI3+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI3; pA != pEnd; ++pA)
					nLastTotalAlpha += pA->bA;
				nTotalA += nLastTotalAlpha;
			}

			if (nTotalA == 0)
			{
				// transparent row
				if (bEverythingSelected || bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFT3End; ++pFTB)
						pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
				}
				else
				{
					// partially selected empty region
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					BYTE const* pS3 = pS2+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
				}
			}
			else if (nTotalA == 765*szSourceTile.cx)
			{
				// opaque row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR3xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR3xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR3xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SELSH(pO1, 11, 12, 21, 22, 31, 32, 16, 17);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR3xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA_SELSH(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16, 17);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					BYTE const* pS3 = pS2+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR3xC2;
						ULONG const bDir2 = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 21, 22, 31, 32, bDir2, 22);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = nFullX;
						ULONG nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						nC3 = (nC3*bInv)>>3;
						COMPUTEWEIGHTSR3xC3;
						ULONG const bDir2 = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, bDir2, 22);
					}
				}
			}
			else
			{
				// semitransparent row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR3xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR3xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(32, pI3[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR3xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(23, pI2[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(32, pI3[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(33, pI3[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR3xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					BYTE const* pS3 = pS2+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR3xC2;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 21, 22, 31, 32, nSel, 22);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = nFullX;
						ULONG nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						nC3 = (nC3*bInv)>>3;
						COMPUTEWEIGHTSR3xC3;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR9_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, nSel, 22);
					}
				}
			}
		}
		else
		{
			// two rows
			ULONG const nR1 = nR11;
			ULONG const nR2 = 256-nR11;

			ULONG nTotalA = 0;
			if (iLastAlpha == nY1)
			{
				nTotalA = nLastTotalAlpha;
			}
			else
			{
				TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
					nTotalA += pA->bA;
			}
			{
				iLastAlpha == nY1+1;
				nLastTotalAlpha = 0;
				TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
				for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
					nLastTotalAlpha += pA->bA;
				nTotalA += nLastTotalAlpha;
			}

			if (nTotalA == 0)
			{
				// transparent row
				if (bEverythingSelected || bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFT3End; ++pFTB)
						pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
				}
				else
				{
					// partially selected empty region
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
					}
				}
			}
			else if (nTotalA == 510*szSourceTile.cx)
			{
				// opaque row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR2xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SELSH(pO1, 11, 12, 21, 22, 16, 17);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR2xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SELSH(pO1, 11, 12, 13, 21, 22, 23, 16, 17);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR2xC2;
						ULONG const bDir2 = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, bDir2, 22);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = nFullX;
						ULONG nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
						ULONG const bInv = 512-bDir;
						OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						nC3 = (nC3*bInv)>>3;
						COMPUTEWEIGHTSR2xC3;
						ULONG const bDir2 = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, bDir2, 22);
					}
				}
			}
			else
			{
				// semitransparent row
				if (bEverythingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR2xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
					}
				}
				else if (bNothingSelected)
				{
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = 256-nC1;
						COMPUTEWEIGHTSR2xC2;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA_NOSEL(23, pI2[pFTB->nFrom1+2]);
						ULONG const nC1 = pFTB->nW1;
						ULONG const nC2 = nFullX;
						ULONG const nC3 = nMin1X-nC1;
						COMPUTEWEIGHTSR2xC3;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
					}
				}
				else
				{
					BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
					BYTE const* pS2 = pS1+szSourceTile.cx;
					for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = 256-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						COMPUTEWEIGHTSR2xC2;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
					}
					for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
					{
						ULONG nC1 = pFTB->nW1;
						ULONG nC2 = nFullX;
						ULONG nC3 = nMin1X-nC1;
						ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
						ULONG const bInv = 512-bDir;
						TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
						TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
						TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
						TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
						TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
						nC1 = (nC1*bInv)>>3;
						nC2 = (nC2*bInv)>>3;
						nC3 = (nC3*bInv)>>3;
						COMPUTEWEIGHTSR2xC3;
						ULONG const nSel = bDir<<13;
						TRasterImagePixel* const pO1 = pO+pFTB->nTo;
						WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, nSel, 22);
					}
				}
			}
		}
	}
}

struct SFromToBlendN
{
	ULONG nFrom;
	ULONG nFull;
	ULONG nW0;
	//ULONG nWN;
};

struct TRasterImagePixel32I
{
	ULONG nR;
	ULONG nG;
	ULONG nB;
	ULONG nA;
};

void CDesignerViewRasterEdit::RenderTileZoomOutCoverage(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride)
{
	SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
	float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
	float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
	RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
	SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

	RECT rcPP = rcSourceTile;
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->PreProcessTile(&rcPP);
	SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
	CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
	TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
	m_pActiveTool->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, m_fGamma, szAdjustedTile.cx, cSrcBuf);
	if (m_bShowComposed && m_pComposedPreview)
		m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

	if (m_bShowInverted)
		RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

	RECT rcSelection = rcSourceTile;
	BOOL bEverythingSelected = TRUE;
	m_pActiveTool->GetSelectionInfo(&rcSelection, &bEverythingSelected);
	BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
	if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
		bEverythingSelected = FALSE;
	ULONG const nSelR = GetRValue(m_tSelection);
	ULONG const nSelG = GetGValue(m_tSelection);
	ULONG const nSelB = GetBValue(m_tSelection);
	ULONG const nGSelR = m_aGammaF[nSelR];
	ULONG const nGSelG = m_aGammaF[nSelG];
	ULONG const nGSelB = m_aGammaF[nSelB];
	TRasterImagePixel aTransparency[2];
	aTransparency[0].bB = GetBValue(M_Square2());
	aTransparency[0].bG = GetGValue(M_Square2());
	aTransparency[0].bR = GetRValue(M_Square2());
	aTransparency[1].bB = GetBValue(M_Square1());
	aTransparency[1].bG = GetGValue(M_Square1());
	aTransparency[1].bR = GetRValue(M_Square1());
	TRasterImagePixel16 aGTransparency[2];
	aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
	aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
	aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
	aGTransparency[1].wR = m_aGammaF[aTransparency[1].bR];
	aGTransparency[1].wG = m_aGammaF[aTransparency[1].bG];
	aGTransparency[1].wB = m_aGammaF[aTransparency[1].bB];
	if (bNothingSelected)
	{
		aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
		aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
		aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
		aGTransparency[1].wB = (aGTransparency[1].wB+nGSelB)>>1;
		aGTransparency[1].wG = (aGTransparency[1].wG+nGSelG)>>1;
		aGTransparency[1].wR = (aGTransparency[1].wR+nGSelR)>>1;
		aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
		aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
		aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
		aTransparency[1].bR = m_aGammaB[aGTransparency[1].wR];
		aTransparency[1].bG = m_aGammaB[aGTransparency[1].wG];
		aTransparency[1].bB = m_aGammaB[aGTransparency[1].wB];
	}
	int nTransparencyShift = TRANSPARENCY_SHIFT;
	while ((1<<nTransparencyShift)*m_fZoom < TRANSPARENCY_GRID)
		++nTransparencyShift;
	if (!bEverythingSelected && !bNothingSelected)
	{
		CAutoVectorPtr<BYTE> cSelBuf;
		if (cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) &&
			SUCCEEDED(m_pActiveTool->GetSelectionTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p)))
		{
			TRasterImagePixel *p = pSrcBuf;
			BYTE *pS = cSelBuf;
			for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
			{
				int const yy = y>>nTransparencyShift;
				for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
				{
					if (p->bA == 255)
					{
						ULONG const bDir = 255-*pS;
						ULONG const bInv = 257+*pS;
						p->bB = m_aGammaB[(m_aGammaF[p->bB]*bInv + nGSelB*bDir)>>9];
						p->bG = m_aGammaB[(m_aGammaF[p->bG]*bInv + nGSelG*bDir)>>9];
						p->bR = m_aGammaB[(m_aGammaF[p->bR]*bInv + nGSelR*bDir)>>9];
					}
					else
					{
						int const xx = x>>nTransparencyShift;
						TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
						ULONG const bDir = p->bA*0x4040+1;
						ULONG const bInv = 0x400000-bDir;
						ULONG const bDir2 = 255-*pS;
						ULONG const bInv2 = 257+*pS;
						p->bB = m_aGammaB[(((t1.wB*bInv + m_aGammaF[p->bB]*bDir)>>22)*bInv2 + nGSelB*bDir2)>>9];
						p->bG = m_aGammaB[(((t1.wG*bInv + m_aGammaF[p->bG]*bDir)>>22)*bInv2 + nGSelG*bDir2)>>9];
						p->bR = m_aGammaB[(((t1.wR*bInv + m_aGammaF[p->bR]*bDir)>>22)*bInv2 + nGSelR*bDir2)>>9];
					}
					++p;
					++pS;
				}
				p += szAdjustedTile.cx-szSourceTile.cx;
			}
		}
		else
		{
			bEverythingSelected = TRUE;
		}
	}
	if (bEverythingSelected)
	{
		TRasterImagePixel *p = pSrcBuf;
		for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
		{
			int const yy = y>>nTransparencyShift;
			for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
			{
				if (p->bA != 255)
				{
					int const xx = x>>nTransparencyShift;
					TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
					ULONG const bDir = p->bA*0x4040+1;
					ULONG const bInv = 0x400000-bDir;
					p->bB = m_aGammaB[(t1.wB*bInv + m_aGammaF[p->bB]*bDir)>>22];
					p->bG = m_aGammaB[(t1.wG*bInv + m_aGammaF[p->bG]*bDir)>>22];
					p->bR = m_aGammaB[(t1.wR*bInv + m_aGammaF[p->bR]*bDir)>>22];
				}
				++p;
			}
			p += szAdjustedTile.cx-szSourceTile.cx;
		}
	}
	else if (bNothingSelected)
	{
		TRasterImagePixel *p = pSrcBuf;
		for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
		{
			int const yy = y>>nTransparencyShift;
			for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
			{
				int const xx = x>>nTransparencyShift;
				if (p->bA == 255)
				{
					p->bB = m_aGammaB[(m_aGammaF[p->bB]+nGSelB)>>1];
					p->bG = m_aGammaB[(m_aGammaF[p->bG]+nGSelG)>>1];
					p->bR = m_aGammaB[(m_aGammaF[p->bR]+nGSelR)>>1];
				}
				else
				{
					TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
					ULONG const bInv = 0x3fffff-p->bA*0x4040;
					ULONG const bDir = (0x400000-bInv)>>1;
					p->bB = m_aGammaB[(t1.wB*bInv + (m_aGammaF[p->bB] + nGSelB)*bDir)>>22];
					p->bG = m_aGammaB[(t1.wG*bInv + (m_aGammaF[p->bG] + nGSelG)*bDir)>>22];
					p->bR = m_aGammaB[(t1.wR*bInv + (m_aGammaF[p->bR] + nGSelR)*bDir)>>22];
				}
				++p;
			}
			p += szAdjustedTile.cx-szSourceTile.cx;
		}
	}
	TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
	//TRasterImagePixel* pI = cSrcBuf;
	CAutoVectorPtr<SFromToBlendN> cBlend(new SFromToBlendN[szTileInImg.cx]);
	ULONG const nCoverageX = 256.0f/fZoomX+0.5f;
	ULONG const nInvCovX = 0x1000000/nCoverageX;
	{
		// prepare guides for x-resizing
		for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
		{
			SFromToBlendN& s = cBlend[x-a_rcTileInImg.left];
			int nX1 = 256.0f*x/fZoomX+0.5f;
			if (nX1&0xff == 0xff)
				++nX1; // optimize a bit (make less overlapping pixels)
			s.nW0 = 0x100-(nX1&0xff);
			int nWN = 0xff&(nX1+nCoverageX);
			if (nWN <= 1)
				nWN += 0x100;
			s.nFrom = (nX1>>8)-rcSourceTile.left;
			s.nFull = (nCoverageX-s.nW0-nWN)>>8;
			if (LONG(s.nFrom+s.nFull+2) > szSourceTile.cx)
				s.nFull = szSourceTile.cx-s.nFrom-2;
		}
	}
	//// resize each row
	//for (int y = 0; y < szSourceTile.cy; ++y)
	//{
	//	TRasterImagePixel* const pRow = cSrcBuf.m_p+y*szSourceTile.cx;
	//	TRasterImagePixel* pO = pRow;
	//	SFromToBlendN const* const pEnd = cBlend.m_p+szTileInImg.cx;
	//	for (SFromToBlendN const* pFTB = cBlend.m_p; pFTB != pEnd; ++pFTB, ++pO)
	//	{
	//		TRasterImagePixel* p0 = pRow+pFTB->nFrom;
	//		TRasterImagePixel* p1 = p0+1;
	//		TRasterImagePixel* pN = p1+pFTB->nFull;
	//		ULONG const nWN = nCoverageX-pFTB->nW0-(pFTB->nFull<<8);
	//		int nR = 0;
	//		int nG = 0;
	//		int nB = 0;
	//		while (p1 < pN)
	//		{
	//			nR += p1->bR;
	//			nG += p1->bG;
	//			nB += p1->bB;
	//			++p1;
	//		}
	//		pO->bR = (((nR<<8) + p0->bR*pFTB->nW0 + pN->bR*nWN)*nInvCovX + 0x800000)>>24;
	//		pO->bG = (((nG<<8) + p0->bG*pFTB->nW0 + pN->bG*nWN)*nInvCovX + 0x800000)>>24;
	//		pO->bB = (((nB<<8) + p0->bB*pFTB->nW0 + pN->bB*nWN)*nInvCovX + 0x800000)>>24;
	//	}
	//}
	ULONG const nCoverageY = 256.0f/fZoomY+0.5f;
	ULONG const nInvCovY = 0x1000000/nCoverageY;
	//for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride-szTileInImg.cx)
	//{
	//	int nY1 = 256.0f*y/fZoomY+0.5f;
	//	if (nY1&0xff == 0xff)
	//		++nY1; // optimize a bit (make less overlapping pixels)
	//	ULONG nW0 = 0x100-(nY1&0xff);
	//	ULONG nWN = 0xff&(nY1+nCoverageY);
	//	if (nWN <= 1)
	//		nWN += 0x100;
	//	ULONG nFrom = (nY1>>8)-rcSourceTile.top;
	//	ULONG nFull = (nCoverageY-nW0-nWN)>>8;
	//	if (nFrom+nFull+2 > szSourceTile.cy)
	//		nFull = szSourceTile.cy-nFrom-2;

	//	TRasterImagePixel* p0 = cSrcBuf.m_p+nFrom*szSourceTile.cx;
	//	TRasterImagePixel* p1 = p0+szSourceTile.cx;
	//	TRasterImagePixel* pN = p1+nFull*szSourceTile.cx;
	//	for (int x = 0; x < szTileInImg.cx; ++x, ++pO, ++p0, ++p1, ++pN)
	//	{
	//		int nR = 0;
	//		int nG = 0;
	//		int nB = 0;
	//		TRasterImagePixel* p2 = p1;
	//		while (p2 < pN)
	//		{
	//			nR += p2->bR;
	//			nG += p2->bG;
	//			nB += p2->bB;
	//			p2 += szSourceTile.cx;
	//		}
	//		pO->bR = (((nB<<8) + p0->bB*nW0 + pN->bB*nWN)*nInvCovY + 0x800000)>>24;
	//		pO->bG = (((nG<<8) + p0->bG*nW0 + pN->bG*nWN)*nInvCovY + 0x800000)>>24;
	//		pO->bB = (((nR<<8) + p0->bR*nW0 + pN->bR*nWN)*nInvCovY + 0x800000)>>24;
	//	}
	//}


	ULONG const nInvCov = 0x1000000/(nCoverageY*nCoverageX);
	CAutoVectorPtr<TRasterImagePixel32I> cRow(new TRasterImagePixel32I[szTileInImg.cx]);
	int nRow = -1;
	for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride-szTileInImg.cx)
	{
		int nY1 = 256.0f*y/fZoomY+0.5f;
		if (nY1&0xff == 0xff)
			++nY1; // optimize a bit (make less overlapping pixels)
		ULONG nW0 = 0x100-(nY1&0xff);
		ULONG nWN = 0xff&(nY1+nCoverageY);
		if (nWN <= 1)
			nWN += 0x100;
		ULONG nFrom = (nY1>>8)-rcSourceTile.top;
		ULONG nFull = (nCoverageY-nW0-nWN)>>8;
		if (LONG(nFrom+nFull+2) > szSourceTile.cy)
			nFull = szSourceTile.cy-nFrom-2;
		ULONG const nWY08 = nW0<<8;
		ULONG const nWYN8 = nWN<<8;

		if (nY1 == nRow)
		{
			// use the partially preprocessed row saved in previous step
		}
		else
		{
			TRasterImagePixel* p0 = pSrcBuf+nFrom*szAdjustedTile.cx;
			TRasterImagePixel* p1 = p0+szAdjustedTile.cx;
			TRasterImagePixel* pN = p1+nFull*szAdjustedTile.cx;
			SFromToBlendN const* const pEnd = cBlend.m_p+szTileInImg.cx;
			for (SFromToBlendN const* pFTB = cBlend.m_p; pFTB != pEnd; ++pFTB, ++pO)
			{
				TRasterImagePixel* p00 = p0+pFTB->nFrom;
				TRasterImagePixel* p10 = p00+1;
				TRasterImagePixel* pN0 = p10+pFTB->nFull;
				TRasterImagePixel* p01 = p1+pFTB->nFrom;
				TRasterImagePixel* p11 = p01+1;
				TRasterImagePixel* pN1 = p11+pFTB->nFull;
				TRasterImagePixel* p0N = pN+pFTB->nFrom;
				TRasterImagePixel* p1N = p0N+1;
				TRasterImagePixel* pNN = p1N+pFTB->nFull;
				ULONG const nWNX = nCoverageX-pFTB->nW0-(pFTB->nFull<<8);
				ULONG const nW00 = (pFTB->nW0*nW0)>>8;
				ULONG const nWN0 = (nWNX*nW0)>>8;
				ULONG const nW0N = (pFTB->nW0*nWN)>>8;
				ULONG const nWNN = (nWNX*nWN)>>8;
				ULONG const nWX08 = pFTB->nW0<<8;
				ULONG const nWXN8 = nWNX<<8;
				// first row
				TRasterImagePixel32I tT = {0, 0, 0};
				{
					TRasterImagePixel* p2 = p10;
					while (p2 < pN0)
					{
						tT.nB += m_aGammaF[p2->bB];
						tT.nG += m_aGammaF[p2->bG];
						tT.nR += m_aGammaF[p2->bR];
						++p2;
					}
				}
				// last row
				TRasterImagePixel32I tB = {0, 0, 0};
				{
					TRasterImagePixel* p2 = p1N;
					while (p2 < pNN)
					{
						tB.nB += m_aGammaF[p2->bB];
						tB.nG += m_aGammaF[p2->bG];
						tB.nR += m_aGammaF[p2->bR];
						++p2;
					}
				}
				// central region
				TRasterImagePixel32I tL = {0, 0, 0};
				TRasterImagePixel32I tR = {0, 0, 0};
				TRasterImagePixel32I tC = {0, 0, 0};
				{
					TRasterImagePixel* p2R = p01;
					while (p2R < p0N)
					{
						tL.nB += m_aGammaF[p2R->bB];
						tL.nG += m_aGammaF[p2R->bG];
						tL.nR += m_aGammaF[p2R->bR];
						TRasterImagePixel* p2 = p2R+1;
						TRasterImagePixel* const p2N = p2+pFTB->nFull;
						while (p2 < p2N)
						{
							tC.nB += m_aGammaF[p2->bB];
							tC.nG += m_aGammaF[p2->bG];
							tC.nR += m_aGammaF[p2->bR];
							++p2;
						}
						tR.nB += m_aGammaF[p2->bB];
						tR.nG += m_aGammaF[p2->bG];
						tR.nR += m_aGammaF[p2->bR];
						p2R += szAdjustedTile.cx;
					}
				}
				pO->bB = m_aGammaB[(((tC.nB<<8) + (tT.nB*nW0) + (tB.nB*nWN) + (tL.nB*pFTB->nW0) + (tR.nB*nWNX) + m_aGammaF[p00->bB]*nW00 + m_aGammaF[p0N->bB]*nW0N + m_aGammaF[pN0->bB]*nWN0 + m_aGammaF[pNN->bB]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
				pO->bG = m_aGammaB[(((tC.nG<<8) + (tT.nG*nW0) + (tB.nG*nWN) + (tL.nG*pFTB->nW0) + (tR.nG*nWNX) + m_aGammaF[p00->bG]*nW00 + m_aGammaF[p0N->bG]*nW0N + m_aGammaF[pN0->bG]*nWN0 + m_aGammaF[pNN->bG]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
				pO->bR = m_aGammaB[(((tC.nR<<8) + (tT.nR*nW0) + (tB.nR*nWN) + (tL.nR*pFTB->nW0) + (tR.nR*nWNX) + m_aGammaF[p00->bR]*nW00 + m_aGammaF[p0N->bR]*nW0N + m_aGammaF[pN0->bR]*nWN0 + m_aGammaF[pNN->bR]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
			}
		}
	}
}

void CDesignerViewRasterEdit::RenderImage(RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
{
	RECT const rcCanvas = {m_ptImagePos.x, m_ptImagePos.y, m_ptImagePos.x+m_tZoomedSize.cx, m_ptImagePos.y+m_tZoomedSize.cy};
	float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
	float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
	SIZE const szDirty = {a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top};

	RECT rcImageTile =
	{
		max(rcCanvas.left, a_rcDirty.left),
		max(rcCanvas.top, a_rcDirty.top),
		min(rcCanvas.right, a_rcDirty.right),
		min(rcCanvas.bottom, a_rcDirty.bottom),
	};

	int const nX1 = max(rcCanvas.left-a_rcDirty.left, 0);
	int const nY1 = max(rcCanvas.top-a_rcDirty.top, 0);

	// draw empty space and image border
	{
		int const nX2 = max(a_rcDirty.right-rcCanvas.right, 0);
		int const nY2 = max(a_rcDirty.bottom-rcCanvas.bottom, 0);
		int nYleft = szDirty.cy;
		COLORREF* p = a_pBuffer;
		if (nY1 > 0)
		{
			if (nY1 > MARGIN_TOP)
			{
				int const nL = min(szDirty.cy, nY1-MARGIN_TOP);
				for (int i = 0; i < nL; ++i)
				{
					std::fill_n(p, szDirty.cx, M_Background());
					p += a_nStride;
				}
				nYleft -= nL;
			}
			int const y2 = min(MARGIN_TOP, nYleft);
			for (int y = nY1 < MARGIN_TOP ? MARGIN_TOP-nY1 : 0; y < y2; ++y)
			{
				int nXleft = szDirty.cx;
				if (nX1 > 0)
				{
					if (nX1 > MARGIN_LEFT)
					{
						int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
						std::fill_n(p, nL, M_Background());
						p += nL;
						nXleft -= nL;
					}
					int const x2 = min(MARGIN_LEFT, nXleft);
					for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_LT[x+MARGIN_LEFT*y];
						--nXleft;
					}
				}
				if (rcImageTile.right > rcImageTile.left)
				{
					std::fill_n(p, rcImageTile.right-rcImageTile.left, MARGIN_CACHE_T[y]);
					p += rcImageTile.right-rcImageTile.left;
					nXleft -= rcImageTile.right-rcImageTile.left;
				}
				if (nX2 > 0)
				{
					int const x2 = min(MARGIN_RIGHT, nX2);
					for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_RT[x+MARGIN_RIGHT*y];
						--nXleft;
					}
					if (nX2 > MARGIN_RIGHT && nXleft)
					{
						int const nL = min(nX2-MARGIN_RIGHT, nXleft);
						std::fill_n(p, nL, M_Background());
						p += nL;
						ATLASSERT(nL == nXleft);
					}
				}
				p += a_nStride-szDirty.cx;
				--nYleft;
			}
		}
		if (rcImageTile.bottom > rcImageTile.top)
		{
			for (LONG i = rcImageTile.top; i < rcImageTile.bottom; ++i)
			{
				int nXleft = szDirty.cx;
				if (nX1 > 0)
				{
					if (nX1 > MARGIN_LEFT)
					{
						int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
						std::fill_n(p, nL, M_Background());
						p += nL;
						nXleft -= nL;
					}
					int const x2 = min(MARGIN_LEFT, nXleft);
					for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_L[x];
						--nXleft;
					}
				}
				if (rcImageTile.right > rcImageTile.left)
				{
					p += rcImageTile.right-rcImageTile.left;
					nXleft -= rcImageTile.right-rcImageTile.left;
				}
				if (nX2 > 0)
				{
					int const x2 = min(MARGIN_RIGHT, nX2);
					for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_R[x];
						--nXleft;
					}
					if (nX2 > MARGIN_RIGHT && nXleft)
					{
						int const nL = min(nX2-MARGIN_RIGHT, nXleft);
						std::fill_n(p, nL, M_Background());
						p += nL;
						nXleft += nL;
					}
				}
				p += a_nStride-szDirty.cx;
			}
			nYleft -= rcImageTile.bottom-rcImageTile.top;
		}
		if (nY2 > 0)
		{
			int const y2 = min(MARGIN_BOTTOM, nY2);
			for (int y = max(nY2-szDirty.cy, 0); y < y2; ++y)
			{
				int nXleft = szDirty.cx;
				if (nX1 > 0)
				{
					if (nX1 > MARGIN_LEFT)
					{
						int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
						std::fill_n(p, nL, M_Background());
						p += nL;
						nXleft -= nL;
					}
					int const x2 = min(MARGIN_LEFT, nXleft);
					for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_LB[x+MARGIN_LEFT*y];
						--nXleft;
					}
				}
				if (rcImageTile.right > rcImageTile.left)
				{
					std::fill_n(p, rcImageTile.right-rcImageTile.left, MARGIN_CACHE_B[y]);
					p += rcImageTile.right-rcImageTile.left;
					nXleft -= rcImageTile.right-rcImageTile.left;
				}
				if (nX2 > 0)
				{
					int const x2 = min(MARGIN_RIGHT, nX2);
					for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
					{
						*(p++) = MARGIN_CACHE_RB[x+MARGIN_RIGHT*y];
						--nXleft;
					}
					if (nXleft)
					{
						std::fill_n(p, nXleft, M_Background());
						p += nXleft;
					}
				}
				p += a_nStride-szDirty.cx;
				--nYleft;
			}
			if (nYleft)
			{
				for (int i = 0; i < nYleft; ++i)
				{
					std::fill_n(p, szDirty.cx, M_Background());
					p += a_nStride;
				}
			}
		}
	}

	bool bUsingNearest = false;
	if (rcImageTile.right > rcImageTile.left && rcImageTile.bottom > rcImageTile.top)
	{
		ULONG nImageTilePixels = (rcImageTile.right-rcImageTile.left)*(rcImageTile.bottom-rcImageTile.top);
		RECT const rcTileInImg = {rcImageTile.left-m_ptImagePos.x, rcImageTile.top-m_ptImagePos.y, rcImageTile.right-m_ptImagePos.x, rcImageTile.bottom-m_ptImagePos.y};

		if ((m_eImageQuality&EIQMethodMask) == EIQNearest || // selected method
			(m_tImageSize.cx <= 2 || m_tImageSize.cy <= 2) || // small images not supported by interpolation methods
			(m_tZoomedSize.cx*128 <= m_tImageSize.cx || m_tZoomedSize.cy*128 <= m_tImageSize.cy)) // zoom too small ... it would overflow
		{
			RenderTileNearest(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
			bUsingNearest = true;
		}
		else if (m_tZoomedSize.cx == m_tImageSize.cx && m_tZoomedSize.cy == m_tImageSize.cy)
		{
			RenderTile100Percent(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
		}
		else if (m_tZoomedSize.cx >= m_tImageSize.cx && m_tZoomedSize.cy >= m_tImageSize.cy)
		{
			RenderTileZoomIn(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
		}
		else if (m_tZoomedSize.cx >= (m_tImageSize.cx>>1) && m_tZoomedSize.cy >= (m_tImageSize.cy>>1))
		{
			RenderTileZoomOutSimple(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
		}
		else
		{
			if ((m_eImageQuality&EIQMethodMask) == EIQCoverage)
			{
				RenderTileZoomOutCoverage(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
			}
			else
			{
				RenderTileNearest(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride);
				bUsingNearest = true;
			}
		}

		// draw grid
		if (m_eGridSetting)
		{
			LONG nStep = m_eGridSetting;
			while (nStep*m_fZoom < 16 || nStep*m_fZoom < 8*m_eGridSetting)
				nStep += nStep;

			agg::rendering_buffer rbuf;
			rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer + a_nStride*nY1+nX1), rcTileInImg.right-rcTileInImg.left, rcTileInImg.bottom-rcTileInImg.top, a_nStride*sizeof*a_pBuffer);
			agg::pixfmt_bgra32 pixf(rbuf);
			agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
			agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_p8 sl;

			float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
			float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
			float fX1 = (rcImageTile.left-m_ptImagePos.x)/fZoomX-nStep*0.5f;
			float fY1 = (rcImageTile.top-m_ptImagePos.y)/fZoomY-nStep*0.5f;
			float fX2 = (rcImageTile.right-m_ptImagePos.x)/fZoomX+nStep*0.5f;
			float fY2 = (rcImageTile.bottom-m_ptImagePos.y)/fZoomY+nStep*0.5f;
			int nX1 = nStep*int(fX1/nStep);
			int nY1 = nStep*int(fY1/nStep);
			int nX2 = nStep*int(fX2/nStep);
			int nY2 = nStep*int(fY2/nStep);

			double f1 = 0.6;
			double f2 = nStep*0.2*m_fZoom;
			for (int y = nY1; y <= nY2; y += nStep)
			{
				double const fY = bUsingNearest ? int(y*fZoomY-rcTileInImg.top+0.5f) : (y*fZoomY-rcTileInImg.top);
				for (int x = nX1; x <= nX2; x += nStep)
				{
					double const fX = bUsingNearest ? int(x*fZoomX-rcTileInImg.left+0.5f) : (x*fZoomX-rcTileInImg.left);
					if (m_eStyle == CFGVAL_2DEDIT_STYLECLASSIC)
					{
						ras.move_to_d(fX-f1, fY-f2);
						ras.line_to_d(fX+f1, fY-f2);
						ras.line_to_d(fX+f1, fY-f1);
						ras.line_to_d(fX+f2, fY-f1);
						ras.line_to_d(fX+f2, fY+f1);
						ras.line_to_d(fX+f1, fY+f1);
						ras.line_to_d(fX+f1, fY+f2);
						ras.line_to_d(fX-f1, fY+f2);
						ras.line_to_d(fX-f1, fY+f1);
						ras.line_to_d(fX-f2, fY+f1);
						ras.line_to_d(fX-f2, fY-f1);
						ras.line_to_d(fX-f1, fY-f1);
						ras.close_polygon();
					}
					else
					{
						ras.move_to_d(fX-f1, fY);
						ras.line_to_d(fX, fY-f1);
						ras.line_to_d(fX+f1, fY);
						ras.line_to_d(fX, fY+f1);
						ras.close_polygon();
					}
				}
			}
			ren.color(agg::rgba8(10, 10, 10, 255));
			agg::render_scanlines(ras, sl, ren);
		}

	}

	// draw help lines
	float fXMin = fZoomX*m_fHelpLinesXMin+m_ptImagePos.x;
	float fYMin = fZoomY*m_fHelpLinesYMin+m_ptImagePos.y;
	float fXMax = fZoomX*m_fHelpLinesXMax+m_ptImagePos.x;
	float fYMax = fZoomY*m_fHelpLinesYMax+m_ptImagePos.y;
	LONG nXMin = fXMin-2;
	LONG nYMin = fYMin-2;
	LONG nXMax = fXMax+2;
	LONG nYMax = fYMax+2;
	if (fXMax >= fXMin && fYMax >= fYMin &&
		a_rcDirty.left < nXMax && a_rcDirty.top < nYMax &&
		a_rcDirty.right > nXMin && a_rcDirty.bottom > nYMin)
	{
		agg::rendering_buffer rbuf;
		rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, a_nStride*sizeof*a_pBuffer);
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

        agg::path_storage path;
		for (CHelpLines::const_iterator i = m_cHelpLines.begin(); i != m_cHelpLines.end(); ++i)
		{
			if (i->bLineTo)
			{
				path.line_to(fZoomX*i->fX+m_ptImagePos.x-a_rcDirty.left, fZoomY*i->fY+m_ptImagePos.y-a_rcDirty.top);
				if (i->bClose)
					path.close_polygon();
			}
			else
			{
				path.move_to(fZoomX*i->fX+m_ptImagePos.x-a_rcDirty.left, fZoomY*i->fY+m_ptImagePos.y-a_rcDirty.top);
			}
		}
		agg::conv_dash<agg::path_storage> dash(path);
		dash.add_dash(5, 5);
		agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke(dash);
		stroke.line_join(agg::bevel_join);
		stroke.line_cap(agg::butt_cap);
		stroke.width(1.0f);
		ren.color(agg::rgba8(255, 255, 255, 255));
		ras.add_path(stroke);
		agg::render_scanlines(ras, sl, ren);

		agg::conv_dash<agg::path_storage> dash2(path);
		dash2.add_dash(5, 5);
		dash2.dash_start(5);
		agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke2(dash2);
		stroke2.line_join(agg::bevel_join);
		stroke2.line_cap(agg::butt_cap);
		stroke2.width(1.0f);
		ren.color(agg::rgba8(0, 0, 0, 255));
		ras.add_path(stroke2);
		agg::render_scanlines(ras, sl, ren);
	}

	// draw control handles
	{
		for (CHandles::const_iterator i = m_cHandles.begin(); i != m_cHandles.end(); ++i)
		{
			TPixelCoords tPos = i->first;
			ULONG nClass = i->second;
			LONG nX = fZoomX*tPos.fX+m_ptImagePos.x;
			LONG nY = fZoomY*tPos.fY+m_ptImagePos.y;
			RECT rcPt = {nX-M_HandleRadius(), nY-M_HandleRadius(), nX+M_HandleRadius()+1, nY+M_HandleRadius()+1};
			RECT rcIntersection =
			{
				max(rcPt.left, a_rcDirty.left),
				max(rcPt.top, a_rcDirty.top),
				min(rcPt.right, a_rcDirty.right),
				min(rcPt.bottom, a_rcDirty.bottom),
			};
			if (rcIntersection.left >= rcIntersection.right || rcIntersection.top >= rcIntersection.bottom)
				continue;
			float const l = 0.8f;
			float const s = 1.0f;
			float const m2 = l + (l <= 0.5f ? l*s : s - l*s);
			float const m1 = 2.0f * l - m2;
			double hue = double(nClass)*360.0f/4.95f+200.0f;
			hue = 360.0-(hue-360.0f*floor(hue/360.0f));
			float r = hls_value(m1, m2, hue+120.0f);
			float g = hls_value(m1, m2, hue);
			float b = hls_value(m1, m2, hue-120.0f);
			if (r > 1.0f) r = 1.0f;
			if (g > 1.0f) g = 1.0f;
			if (b > 1.0f) b = 1.0f;
			if (r < 0.0f) r = 0.0f;
			if (g < 0.0f) g = 0.0f;
			if (b < 0.0f) b = 0.0f;
			BYTE bClassR = r*255.0f+0.5f;
			BYTE bClassG = g*255.0f+0.5f;
			BYTE bClassB = b*255.0f+0.5f;
			for (LONG y = rcIntersection.top; y < rcIntersection.bottom; ++y)
			{
				BYTE* pO = reinterpret_cast<BYTE*>(a_pBuffer + (y-a_rcDirty.top)*a_nStride + rcIntersection.left-a_rcDirty.left);
				BYTE const* pI = m_pHandleImage + (((y-rcPt.top)*(M_HandleRadius()+M_HandleRadius()+1) + rcIntersection.left-rcPt.left)<<1);
				for (LONG x = rcIntersection.left; x < rcIntersection.right; ++x)
				{
					if (pI[0])
					{
						if (pI[0] == 255)
						{
							pO[0] = (ULONG(pI[1])*bClassB)/255;
							pO[1] = (ULONG(pI[1])*bClassG)/255;
							pO[2] = (ULONG(pI[1])*bClassR)/255;
						}
						else
						{
							ULONG clr = ULONG(pI[1])*pI[0];
							ULONG inv = (255-pI[0])*255;
							pO[0] = (pO[0]*inv + clr*bClassB)/(255*255);
							pO[1] = (pO[1]*inv + clr*bClassG)/(255*255);
							pO[2] = (pO[2]*inv + clr*bClassR)/(255*255);
						}
					}
					pO += 4;
					pI += 2;
				}
			}
		}
	}

	// draw mouse gesture trail
	if (m_eDragState == EDSGesture &&
		m_nGestureXMin <= m_nGestureXMax && m_nGestureYMin <= m_nGestureYMax)
	{
		agg::rendering_buffer rbuf;
		rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, (a_rcDirty.right-a_rcDirty.left)*sizeof*a_pBuffer);
		agg::pixfmt_bgra32 pixf(rbuf);
		agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;

        agg::path_storage path;
		path.move_to(m_cGesturePoints.begin()->x-a_rcDirty.left+0.5, m_cGesturePoints.begin()->y-a_rcDirty.top+0.5);
		for (CGesturePoints::const_iterator i = m_cGesturePoints.begin()+1; i != m_cGesturePoints.end(); ++i)
		{
			path.line_to(i->x-a_rcDirty.left+0.5, i->y-a_rcDirty.top+0.5);
		}
		agg::conv_stroke<agg::path_storage> stroke(path);
		stroke.line_join(agg::bevel_join);
		stroke.width(3.0f);
		ren.color(agg::rgba8(255, 50, 50, 160));
		ras.add_path(stroke);
		agg::render_scanlines(ras, sl, ren);
	}
}
*/
