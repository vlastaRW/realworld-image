
#pragma once

#include "EditToolPixelMixer.h"


template<class T>
class CEditToolPixelCoverageBuffer
{
public:
	CEditToolPixelCoverageBuffer() : m_rcDirty(RECT_EMPTY), m_eCacheState(ECSInvalid), m_fCachedGamma(1.0f), m_fGamma(2.2f), m_pGT(NULL)
	{
	}
	~CEditToolPixelCoverageBuffer()
	{
		delete m_pGT;
	}

	void InvalidateCacheAndRectangle() { m_eCacheState = ECSInvalid; }
	void InvalidateCachePixels() { m_eCacheState = ECSRectangleValid; }
	RECT UpdateCache()
	{
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
	bool HitTest(LONG a_nX, LONG a_nY) const
	{
		CCoveredPixels::const_iterator iY = m_cPixels.find(nY);
		if (iY != m_cPixels.end())
		{
			std::map<LONG, BYTE>::const_iterator iX = iY->second.find(nY);
			if (iX != iY->second.end())
				return iX->second != 0;
		}
		CCoveredPixels2::const_iterator iY2 = m_cPixels2.find(nY);
		if (iY != m_cPixels2.end())
		{
			std::map<LONG, TRasterImagePixel>::const_iterator iX = iY2->second.find(nY);
			if (iX != iY->second.end())
				return iX->second.bA != 0;
		}
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
		RECT rcSel = {0, 0, a_nSizeX, a_nSizeY};
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

		RenderTile(static_cast<T*>(this)->M_BlendingMode(), rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask);
		return S_OK;
	}
	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = m_cPixels.size()*2 < size_t((m_rcDirty.right-m_rcDirty.left)*(m_rcDirty.bottom-m_rcDirty.top));
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = m_rcDirty;
		return m_cPixels.empty() && m_cPixels2.empty() ? S_FALSE : S_OK;
	}

	void InitImageTarget(unsigned a_nSizeX, unsigned a_nSizeY, bool a_bCacheValid = false)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		DeleteCachedData();
		if (a_bCacheValid)
			m_eCacheState = ECSValid;
	}
	RECT const& M_DirtyRect() const
	{
		return m_rcDirty;
	}
	bool IsDirty() const
	{
		return !m_cPixels.empty() || !m_cPixels2.empty();
	}
	void DeleteCachedData()
	{
		m_cPixels.clear();
		m_cPixels2.clear();
		m_rcDirty = RECT_EMPTY;
	}

	void AddPixel(LONG a_nX, LONG a_nY, BYTE a_bCoverage)
	{
		//if (a_nX >= 0 && a_nY >= 0 && ULONG(a_nX) < m_nSizeX && ULONG(a_nY) < m_nSizeY)
		{
			CCoveredPixels::iterator i = m_cPixels.find(a_nY);
			if (i == m_cPixels.end())
			{
				m_cPixels[a_nY][a_nX] = a_bCoverage;
				if (m_rcDirty.top > a_nY)
					m_rcDirty.top = a_nY;
				if (m_rcDirty.bottom <= a_nY)
					m_rcDirty.bottom = a_nY+1;
				if (m_rcDirty.left > a_nX)
					m_rcDirty.left = a_nX;
				if (m_rcDirty.right <= a_nX)
					m_rcDirty.right = a_nX+1;
			}
			else
			{
				std::map<LONG, BYTE>::iterator j = i->second.find(a_nX);
				if (j == i->second.end())
				{
					i->second[a_nX] = a_bCoverage;
					if (m_rcDirty.left > a_nX)
						m_rcDirty.left = a_nX;
					if (m_rcDirty.right <= a_nX)
						m_rcDirty.right = a_nX+1;
				}
				else
				{
					BYTE const bPrev = j->second;
					j->second = a_bCoverage + (((255-a_bCoverage)*bPrev)>>8);
					if (bPrev != j->second)
					{
						if (m_rcDirty.left > a_nX)
							m_rcDirty.left = a_nX;
						if (m_rcDirty.right <= a_nX)
							m_rcDirty.right = a_nX+1;
					}
				}
			}
		}
	}

	void MergePixel(LONG a_nX, LONG a_nY, BYTE a_bCoverage)
	{
		//if (a_nX >= 0 && a_nY >= 0 && ULONG(a_nX) < m_nSizeX && ULONG(a_nY) < m_nSizeY)
		{
			CCoveredPixels::iterator i = m_cPixels.find(a_nY);
			if (i == m_cPixels.end())
			{
				m_cPixels[a_nY][a_nX] = a_bCoverage;
				if (m_rcDirty.top > a_nY)
					m_rcDirty.top = a_nY;
				if (m_rcDirty.bottom <= a_nY)
					m_rcDirty.bottom = a_nY+1;
				if (m_rcDirty.left > a_nX)
					m_rcDirty.left = a_nX;
				if (m_rcDirty.right <= a_nX)
					m_rcDirty.right = a_nX+1;
			}
			else
			{
				std::map<LONG, BYTE>::iterator j = i->second.find(a_nX);
				if (j == i->second.end())
				{
					i->second[a_nX] = a_bCoverage;
					if (m_rcDirty.left > a_nX)
						m_rcDirty.left = a_nX;
					if (m_rcDirty.right <= a_nX)
						m_rcDirty.right = a_nX+1;
				}
				else
				{
					j->second = 255-(255-a_bCoverage)*(255-j->second)/255;
					if (m_rcDirty.left > a_nX)
						m_rcDirty.left = a_nX;
					if (m_rcDirty.right <= a_nX)
						m_rcDirty.right = a_nX+1;
				}
			}
		}
	}

	void MergePixel(LONG a_nX, LONG a_nY, TRasterImagePixel a_tPixel)
	{
		//if (a_nX >= 0 && a_nY >= 0 && ULONG(a_nX) < m_nSizeX && ULONG(a_nY) < m_nSizeY)
		{
			CCoveredPixels2::iterator i = m_cPixels2.find(a_nY);
			if (i == m_cPixels2.end())
			{
				m_cPixels2[a_nY][a_nX] = a_tPixel;
				if (m_rcDirty.top > a_nY)
					m_rcDirty.top = a_nY;
				if (m_rcDirty.bottom <= a_nY)
					m_rcDirty.bottom = a_nY+1;
				if (m_rcDirty.left > a_nX)
					m_rcDirty.left = a_nX;
				if (m_rcDirty.right <= a_nX)
					m_rcDirty.right = a_nX+1;
			}
			else
			{
				std::map<LONG, TRasterImagePixel>::iterator j = i->second.find(a_nX);
				if (j == i->second.end())
				{
					i->second[a_nX] = a_tPixel;
					if (m_rcDirty.left > a_nX)
						m_rcDirty.left = a_nX;
					if (m_rcDirty.right <= a_nX)
						m_rcDirty.right = a_nX+1;
				}
				else
				{
					CPixelMixerPaintOver::Mix(j->second, a_tPixel);
					if (m_rcDirty.left > a_nX)
						m_rcDirty.left = a_nX;
					if (m_rcDirty.right <= a_nX)
						m_rcDirty.right = a_nX+1;
				}
			}
		}
	}

	void RenderTile(EBlendingMode a_eBlendingMode, RECT const& a_rc, float a_fGamma, ULONG a_nBufferStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask) const
	{
		UpdateGammaTables();
		TRasterImagePixel tPixel = static_cast<T const*>(this)->M_Color(a_fGamma);
		switch (tPixel.bA ? a_eBlendingMode : EBMReplace)
		{
		case EBMReplace:
			RenderPixels<CPixelMixerReplace>(tPixel, a_rc, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		case EBMDrawOver:
			RenderPixels<CPixelMixerPaintOver>(tPixel, a_rc, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
			break;
		case EBMDrawUnder:
			RenderPixels<CPixelMixerPaintUnder>(tPixel, a_rc, a_nBufferStride, a_pBuffer, a_nMaskStride, a_pMask);
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
		}
	}
	template<class TPixelMixer>
	void RenderPixels(TRasterImagePixel const tColor, RECT const& a_rc, ULONG a_nBufferStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask) const
	{
		bool const bUseGamma = m_fCachedGamma != 1.0f;
		if (a_rc.left >= a_rc.right || a_rc.top >= a_rc.bottom)
			return ;
		{
			CCoveredPixels::const_iterator iY = m_cPixels.lower_bound(a_rc.top);
			CCoveredPixels::const_iterator const iYEnd = m_cPixels.lower_bound(a_rc.bottom);
			if (iY != m_cPixels.end())
			{
				while (iY != iYEnd)
				{
					std::map<LONG, BYTE>::const_iterator iX = iY->second.lower_bound(a_rc.left);
					std::map<LONG, BYTE>::const_iterator iXEnd = iY->second.lower_bound(a_rc.right);
					if (iX != iY->second.end())
					{
						TRasterImagePixel* pBuffer = a_pBuffer+(iY->first-a_rc.top)*a_nBufferStride-a_rc.left;
						BYTE* pMask = a_pMask+(iY->first-a_rc.top)*a_nMaskStride-a_rc.left;
						if (bUseGamma)
						{
							while (iX != iXEnd)
							{
								TPixelMixer::Mix(pBuffer[iX->first], tColor, a_pMask ? (pMask[iX->first])*ULONG(iX->second)/255 : iX->second, m_pGT);
								++iX;
							}
						}
						else
						{
							while (iX != iXEnd)
							{
								TPixelMixer::Mix(pBuffer[iX->first], tColor, a_pMask ? (pMask[iX->first])*ULONG(iX->second)/255 : iX->second);
								++iX;
							}
						}
					}
					++iY;
				}
			}
		}

		{
			CCoveredPixels2::const_iterator iY = m_cPixels2.lower_bound(a_rc.top);
			CCoveredPixels2::const_iterator const iYEnd = m_cPixels2.lower_bound(a_rc.bottom);
			if (iY != m_cPixels2.end())
			{
				while (iY != iYEnd)
				{
					std::map<LONG, TRasterImagePixel>::const_iterator iX = iY->second.lower_bound(a_rc.left);
					std::map<LONG, TRasterImagePixel>::const_iterator iXEnd = iY->second.lower_bound(a_rc.right);
					if (iX != iY->second.end())
					{
						TRasterImagePixel* pBuffer = a_pBuffer+(iY->first-a_rc.top)*a_nBufferStride-a_rc.left;
						BYTE* pMask = a_pMask+(iY->first-a_rc.top)*a_nMaskStride-a_rc.left;
						if (bUseGamma)
						{
							while (iX != iXEnd)
							{
								TRasterImagePixel t = iX->second;
								t.bA = 0xff;
								TPixelMixer::Mix(pBuffer[iX->first], t, a_pMask ? (pMask[iX->first])*ULONG(iX->second.bA)/255 : iX->second.bA, m_pGT);
								++iX;
							}
						}
						else
						{
							while (iX != iXEnd)
							{
								TRasterImagePixel t = iX->second;
								t.bA = 0xff;
								TPixelMixer::Mix(pBuffer[iX->first], t, a_pMask ? (pMask[iX->first])*ULONG(iX->second.bA)/255 : iX->second.bA);
								++iX;
							}
						}
					}
					++iY;
				}
			}
		}
	}

private:
	typedef std::map<LONG, std::map<LONG, BYTE> > CCoveredPixels;
	typedef std::map<LONG, std::map<LONG, TRasterImagePixel> > CCoveredPixels2;
	enum ECacheState
	{
		ECSValid = 0,
		ECSRectangleValid,
		ECSInvalid,
	};

private:
	CCoveredPixels m_cPixels;
	CCoveredPixels2 m_cPixels2;
	RECT m_rcDirty;
	unsigned m_nSizeX;
	unsigned m_nSizeY;
	ECacheState m_eCacheState;

	float m_fGamma;
	mutable float m_fCachedGamma;
	mutable CGammaTables* m_pGT;
};

