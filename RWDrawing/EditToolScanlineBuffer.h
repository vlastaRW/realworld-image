
#pragma once

#include <string.h>
#include <agg_basics.h>
#include <agg_color_rgba.h>
#include <agg_rendering_buffer.h>

#include "EditToolPixelMixer.h"



struct compound_layered_add
{
	void inline operator()(TPixelChannel* t, unsigned char* this_cover, TPixelChannel const& c, unsigned cover) const
	{
		if (*this_cover == 0 || cover == 255)
		{
			*t = c;
			*this_cover = cover;
			return;
		}
		unsigned const total_cover = *this_cover + cover;
		ULONG const ac = ULONG(t->bA)**this_cover;
		ULONG const cac = ULONG(c.bA)*cover;
		t->bA = (ac + cac + (total_cover>>1))/total_cover;
		ULONG const div = ac + cac;
		if (div)
		{
			if (pGT)
			{
				t->bR = pGT->InvGamma((pGT->m_aGamma[t->bR]*ac + pGT->m_aGamma[c.bR]*cac)/div); // TODO: +(div>>1) ?
				t->bG = pGT->InvGamma((pGT->m_aGamma[t->bG]*ac + pGT->m_aGamma[c.bG]*cac)/div);
				t->bB = pGT->InvGamma((pGT->m_aGamma[t->bB]*ac + pGT->m_aGamma[c.bB]*cac)/div);
			}
			else
			{
				t->bR = (t->bR*ac + c.bR*cac)/div; // TODO: +(div>>1) ?
				t->bG = (t->bG*ac + c.bG*cac)/div;
				t->bB = (t->bB*ac + c.bB*cac)/div;
			}
		}
		else
		{
			t->bR = t->bG = t->bB = 0;
		}
		*this_cover = total_cover;
	}

	CGammaTables const* pGT;
};

template<class T>
class CEditToolScanlineBuffer
{
public:
	typedef TPixelChannel color_type;
	typedef void row_data;
	typedef void span_data;

	CEditToolScanlineBuffer() : m_eCacheState(ECSInvalid), m_rcDirty(RECT_EMPTY), m_nTotalAlpha(0), m_fCachedGamma(1.0f), m_fGamma(2.2f), m_pGT(NULL)
	{
		m_cLayerBlender.pGT = NULL;
	}
	~CEditToolScanlineBuffer()
	{
		delete m_pGT;
		DeleteCachedData();
	}

	void InvalidateCacheAndRectangle() { m_eCacheState = ECSInvalid; }
	void InvalidateCachePixels() { m_eCacheState = ECSRectangleValid; }
	RECT UpdateCache()
	{
		T::ObjectLock cLock(static_cast<T*>(this));
		RECT rcPrev = m_rcDirty;
		DeleteCachedData();
		static_cast<T*>(this)->PrepareShape();
		if (rcPrev.left > m_rcDirty.left) rcPrev.left = m_rcDirty.left;
		if (rcPrev.top > m_rcDirty.top) rcPrev.top = m_rcDirty.top;
		if (rcPrev.right < m_rcDirty.right) rcPrev.right = m_rcDirty.right;
		if (rcPrev.bottom < m_rcDirty.bottom) rcPrev.bottom = m_rcDirty.bottom;
		m_eCacheState = ECSValid;
		return rcPrev;
	}
	bool HitTest(LONG a_nX, LONG a_nY)
	{
		if (m_eCacheState != ECSValid)
		{
			DeleteCachedData();
			static_cast<T*>(this)->PrepareShape();
			m_eCacheState = ECSValid;
		}
		return static_cast<CEditToolScanlineBuffer<T> const*>(this)->HitTest(a_nX, a_nY);
	}
	bool HitTest(LONG a_nX, LONG a_nY) const
	{
		for (CSolidLines::const_iterator i = m_cSolidLines.begin(); i != m_cSolidLines.end(); ++i)
			if (i->y == a_nY && i->x <= a_nX && LONG(i->x+i->n) > a_nX)
				return true;
		for (CSolidSpans::const_iterator i = m_cSolidSpans.begin(); i != m_cSolidSpans.end(); ++i)
			if (i->y == a_nY && i->x <= a_nX && LONG(i->x+i->n) > a_nX && i->pCovers[a_nX-i->x])
				return true;
		for (CColorSpans::const_iterator i = m_cColorSpans.begin(); i != m_cColorSpans.end(); ++i)
			if (i->y == a_nY && i->x <= a_nX && LONG(i->x+i->n) > a_nX)
				return true;
		for (CFullSpans::const_iterator i = m_cFullSpans.begin(); i != m_cFullSpans.end(); ++i)
			if (i->y == a_nY && i->x <= a_nX && LONG(i->x+i->n) > a_nX && i->pCovers[a_nX-i->x])
				return true;
		return false;
	}

	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		if (a_fGamma > 0.1f && a_fGamma < 10.0f)
			m_fGamma = a_fGamma;
		{
			T::ObjectLock cLock(static_cast<T*>(this));
			if (m_eCacheState != ECSValid)
			{
				DeleteCachedData();
				static_cast<T*>(this)->PrepareShape();
				m_eCacheState = ECSValid;
			}
		}
		HRESULT hRes = static_cast<T*>(this)->M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes))
			return hRes;
		CAutoVectorPtr<BYTE> cMask;
		RECT rcSel = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		BOOL bSolidSel = TRUE;
		static_cast<T*>(this)->M_Window()->GetSelectionInfo(&rcSel, &bSolidSel);
		RECT rc =
		{
			max(LONG(a_nX), rcSel.left),
			max(LONG(a_nY), rcSel.top),
			min(LONG(a_nX+a_nSizeX), rcSel.right),
			min(LONG(a_nY+a_nSizeY), rcSel.bottom)
		};
		if (rc.left >= rc.right || rc.top >= rc.bottom)
			return S_OK; // nothing to draw

		SIZE sz = {rc.right-rc.left, rc.bottom-rc.top};

		if (!bSolidSel)
		{
			cMask.Allocate(sz.cx*sz.cy);
			if (FAILED(static_cast<T*>(this)->M_Window()->GetSelectionTile(rc.left, rc.top, sz.cx, sz.cy, sz.cx, cMask)))
				cMask.Free();
		}

		RenderTile(static_cast<T*>(this)->M_BlendingMode(), rc, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask);
		return S_OK;
	}
	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		{
			T::ObjectLock cLock(static_cast<T*>(this));
			if (m_eCacheState != ECSValid)
			{
				DeleteCachedData();
				static_cast<T*>(this)->PrepareShape();
				m_eCacheState = ECSValid;
			}
		}
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE; // TODO: compare number of pixels in rectangle with number of pixels in scanline buffers
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = M_DirtyRect();
		return IsDirty() ? S_OK : S_FALSE;
	}

	void InitImageTarget(unsigned a_nSizeX, unsigned a_nSizeY)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		DeleteCachedData();
	}
	RECT const& M_DirtyRect() const
	{
		return m_rcDirty;
	}
	bool IsDirty() const
	{
		return m_rcDirty.left<m_rcDirty.right && m_rcDirty.top<m_rcDirty.bottom;
	}
	void DeleteCachedData()
	{
		m_rcDirty = RECT_EMPTY;
		m_eCacheState = ECSInvalid;
		m_nTotalAlpha = 0;
		m_cSolidLines.clear();
		for (CSolidSpans::iterator i = m_cSolidSpans.begin(); i != m_cSolidSpans.end(); ++i)
			delete[] i->pCovers;
		m_cSolidSpans.clear();
		for (CColorSpans::iterator i = m_cColorSpans.begin(); i != m_cColorSpans.end(); ++i)
			delete[] i->pC;
		m_cColorSpans.clear();
		for (CFullSpans::iterator i = m_cFullSpans.begin(); i != m_cFullSpans.end(); ++i)
		{
			delete[] i->pCovers;
			delete[] i->pC;
		}
		m_cFullSpans.clear();
	}
	void RenderTile(EBlendingMode a_eBlendingMode, RECT const& a_rcCoords, ULONG a_nBufferStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask) const
	{
		switch (m_nTotalAlpha ? a_eBlendingMode : EBMReplace) // when drawing with transparent color, switch to replace mode
		{
		case EBMReplace:
			RenderTile<CPixelMixerReplace>(a_rcCoords, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		case EBMDrawOver:
			RenderTile<CPixelMixerPaintOver>(a_rcCoords, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		case EBMDrawUnder:
			RenderTile<CPixelMixerPaintUnder>(a_rcCoords, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		case EBMAdd:
			RenderTile<CPixelMixerAdd>(a_rcCoords, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		}
	}
	float M_Gamma() const { return m_fGamma; }
	void UpdateGammaTables() const
	{
		float const fGamma = static_cast<T const*>(this)->M_Gamma();
		if (m_fCachedGamma != fGamma)
		{
			delete m_pGT;
			if (fGamma >= 2.1f && fGamma <= 2.3f)
			{
				m_pGT = new CGammaTables();
				m_fCachedGamma = 2.2f;
			}
			else if (fGamma != 1.0f)
			{
				m_pGT = new CGammaTables(fGamma);
				m_fCachedGamma = fGamma;
			}
			else
			{
				m_pGT = NULL;
				m_fCachedGamma = 1.0f;
			}
			m_cLayerBlender.pGT = m_pGT;
		}
	}
	compound_layered_add const& M_LayerBlender()
	{
		return m_cLayerBlender;
	}
	template<class TPixelMixer>
	void RenderTile(RECT const& a_rcCoords, ULONG a_nBufferStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask) const
	{
		//if (a_rcCoords.bottom <= 0 || a_rcCoords.right <= 0 ||
		//	a_rcCoords.top >= m_nSizeY || a_rcCoords.left >= m_nSizeX)
		//	return;
		UpdateGammaTables();
		bool const bUseGamma = m_fCachedGamma != 1.0f;
		for (CSolidLines::const_iterator i = m_cSolidLines.begin(); i != m_cSolidLines.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TRasterImagePixel const tColor = {i->c.bB, i->c.bG, i->c.bR, i->c.bA};
			TRasterImagePixel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*a_nBufferStride;
			TRasterImagePixel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			ULONG const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TRasterImagePixel* p = pLine+nOff;
			if (a_pMask)
			{
				BYTE* pMask = a_pMask + (i->y-a_rcCoords.top)*a_nMaskStride + nOff;
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, (i->cover*ULONG(*pMask))/255, m_pGT);
						++p;
						++pMask;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, (i->cover*ULONG(*pMask))/255);
						++p;
						++pMask;
					}
				}
			}
			else
			{
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, i->cover, m_pGT);
						++p;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, i->cover);
						++p;
					}
				}
			}
		}
		for (CSolidSpans::const_iterator i = m_cSolidSpans.begin(); i != m_cSolidSpans.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TRasterImagePixel const tColor = {i->c.bB, i->c.bG, i->c.bR, i->c.bA};
			TRasterImagePixel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*a_nBufferStride;
			TRasterImagePixel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			ULONG const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TRasterImagePixel* p = pLine+nOff;
			BYTE* pCover = i->pCovers + a_rcCoords.left-i->x+nOff;
			if (a_pMask)
			{
				BYTE* pMask = a_pMask + (i->y-a_rcCoords.top)*a_nMaskStride + nOff;
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, (*pCover*ULONG(*pMask))/255, m_pGT);
						++p;
						++pCover;
						++pMask;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, (*pCover*ULONG(*pMask))/255);
						++p;
						++pCover;
						++pMask;
					}
				}
			}
			else
			{
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, *pCover, m_pGT);
						++p;
						++pCover;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, tColor, *pCover);
						++p;
						++pCover;
					}
				}
			}
		}
		for (CColorSpans::const_iterator i = m_cColorSpans.begin(); i != m_cColorSpans.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TRasterImagePixel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*a_nBufferStride;
			TRasterImagePixel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			ULONG const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TRasterImagePixel* p = pLine+nOff;
			// TODO: remove hack
			TRasterImagePixel const* pColor = reinterpret_cast<TRasterImagePixel const*>(i->pC) + a_rcCoords.left-i->x+nOff;//agg::rgba8
			if (a_pMask)
			{
				BYTE* pMask = a_pMask + (i->y-a_rcCoords.top)*a_nMaskStride + nOff;
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, (i->cover*ULONG(*pMask))/255, m_pGT);
						++p;
						++pColor;
						++pMask;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, (i->cover*ULONG(*pMask))/255);
						++p;
						++pColor;
						++pMask;
					}
				}
			}
			else
			{
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, i->cover, m_pGT);
						++p;
						++pColor;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, i->cover);
						++p;
						++pColor;
					}
				}
			}
		}
		for (CFullSpans::const_iterator i = m_cFullSpans.begin(); i != m_cFullSpans.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TRasterImagePixel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*a_nBufferStride;
			TRasterImagePixel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			ULONG const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TRasterImagePixel* p = pLine+nOff;
			// TODO: remove hack
			TRasterImagePixel const* pColor = reinterpret_cast<TRasterImagePixel const*>(i->pC) + a_rcCoords.left-i->x+nOff;//agg::rgba8
			BYTE* pCover = i->pCovers + a_rcCoords.left-i->x+nOff;
			if (a_pMask)
			{
				BYTE* pMask = a_pMask + (i->y-a_rcCoords.top)*a_nMaskStride + nOff;
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, (*pCover*ULONG(*pMask))/255, m_pGT);
						++p;
						++pCover;
						++pColor;
						++pMask;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, (*pCover*ULONG(*pMask))/255);
						++p;
						++pCover;
						++pColor;
						++pMask;
					}
				}
			}
			else
			{
				if (bUseGamma)
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, *pCover, m_pGT);
						++p;
						++pCover;
						++pColor;
					}
				}
				else
				{
					while (p < pEnd)
					{
						TPixelMixer::Mix(*p, *pColor, *pCover);
						++p;
						++pCover;
						++pColor;
					}
				}
			}
		}
	}

	// AGG image target methods
public:
    unsigned width() const { return LONG_MAX/*m_nSizeX*/; }
    unsigned height() const { return LONG_MAX/*m_nSizeY*/; }

	void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
	{
		SSolidLine sLine = {x, y, 1, c, cover};
		m_nTotalAlpha += c.a;
		m_cSolidLines.push_back(sLine);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right <= x) m_rcDirty.right = x+1;
	}
	void blend_hline(int x, int y, unsigned len, color_type const& c, agg::int8u cover)
    {
		SSolidLine sLine = {x, y, len, c, cover};
		m_nTotalAlpha += c.bA;
		m_cSolidLines.push_back(sLine);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right < LONG(x+len)) m_rcDirty.right = x+len;
	}
	void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)
	{
		SSolidSpan sItem = {x, y, len, c, new BYTE[len]};
		m_nTotalAlpha += c.bA;
		CopyMemory(sItem.pCovers, covers, len*sizeof *sItem.pCovers);
		m_cSolidSpans.push_back(sItem);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right < LONG(x+len)) m_rcDirty.right = x+len;
	}
	void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
	{
		if (covers)
		{
			// TODO: handle exceptions correctly
			SFullSpan sItem = {x, y, len, new color_type[len], new BYTE[len]};
			CopyMemory(sItem.pCovers, covers, len*sizeof *sItem.pCovers);
			CopyMemory(sItem.pC, colors, len*sizeof *sItem.pC);
			m_cFullSpans.push_back(sItem);
		}
		else
		{
			SColorSpan sItem = {x, y, len, new color_type[len], cover};
			CopyMemory(sItem.pC, colors, len*sizeof *sItem.pC);
			m_cColorSpans.push_back(sItem);
		}
		for (unsigned i = 0; i < len; ++i)
			m_nTotalAlpha += colors[i].bA;
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right < LONG(x+len)) m_rcDirty.right = x+len;
	}

private:
	struct SSolidLine
	{
		int x;
		int y;
		unsigned n;
		color_type c;
		BYTE cover;
	};
	typedef std::vector<SSolidLine> CSolidLines;

	struct SSolidSpan
	{
		int x;
		int y;
		unsigned n;
		color_type c;
		BYTE* pCovers;
	};
	typedef std::vector<SSolidSpan> CSolidSpans;

	struct SColorSpan
	{
		int x;
		int y;
		unsigned n;
		color_type* pC;
		BYTE cover;
	};
	typedef std::vector<SColorSpan> CColorSpans;

	struct SFullSpan
	{
		int x;
		int y;
		unsigned n;
		color_type* pC;
		BYTE* pCovers;
	};
	typedef std::vector<SFullSpan> CFullSpans;

	enum ECacheState
	{
		ECSValid = 0,
		ECSRectangleValid,
		ECSInvalid,
	};

private:
	unsigned m_nSizeX;
	unsigned m_nSizeY;
	CSolidLines m_cSolidLines;
	CSolidSpans m_cSolidSpans;
	CColorSpans m_cColorSpans;
	CFullSpans m_cFullSpans;
	ULONG m_nTotalAlpha;
	RECT m_rcDirty;
	ECacheState m_eCacheState;
	float m_fGamma;
	mutable float m_fCachedGamma;
	mutable compound_layered_add m_cLayerBlender;
	mutable CGammaTables* m_pGT;
};

