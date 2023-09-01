
#pragma once

#include <EditTool.h>
#include <math.h>

struct CEditToolDataRedEye
{
	MIDL_INTERFACE("A0BF7080-D0B5-4C7E-86D6-25AE619AB85A")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataRedEye const*, InternalData)() = 0;
	};

	CEditToolDataRedEye() : fThreshold(30.0f), fBrightness(100.0f), fSlope(0.0f)
	{
	}
	HRESULT FromString(BSTR a_bstr)
	{
		return E_NOTIMPL;
		//int x = 0;
		//int y = 0;
		//swscanf(a_bstr, L"%i:%i", &x, &y);
		//if (x && y)
		//	eProportionsMode = static_cast<EProportionsMode>(x|(y<<16));
		//if (wcsstr(a_bstr, L"PERSPECTIVE"))
		//	eCropMode = ECMPerspective;
		//else if (wcsstr(a_bstr, L"LOSSLESSJPEG"))
		//	eCropMode = ECMLosslessJPEG;
		//return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		return E_NOTIMPL;
		//wchar_t szTmp[64] = L"";
		//swprintf(szTmp, L"%i:%i|%s", int(eProportionsMode&0xffff), int(eProportionsMode>>16), eCropMode == ECMPerspective ? L"PERSPECTIVE" : (eCropMode == ECMLosslessJPEG ? L"LOSSLESSJPEG" : L"STANDARD"));
		//*a_pbstr = SysAllocString(szTmp);
		//return S_OK;
	}

	float fThreshold;
	float fBrightness;
	float fSlope;
};

HICON GetToolIconREDEYE(ULONG a_nSize);

class CEditToolRedEye :
	public CEditToolMouseInput<CEditToolRedEye>, // no direct tablet support
	public CEditToolCustomPrecisionCursor<GetToolIconREDEYE>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolRedEye, // T - the top level class for cross casting
		CEditToolRedEye, // TResetHandler
		CEditToolRedEye, // TDirtyHandler
		CEditToolRedEye, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolRedEye, // TAdjustCoordsHandler
		CEditToolCustomPrecisionCursor<GetToolIconREDEYE>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolRedEye>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CEditToolRedEye // TControlLinesHandler
	>
{
public:
	CEditToolRedEye() : m_fRadius(0.0f)
	{
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		if (m_fRadius > 0.0f)
		{
			m_fRadius = 0.0f;
			M_Window()->RectangleChanged(&M_Dirty());
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}

	HRESULT _GetImageTile(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		HRESULT hRes = M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes))
			return hRes;
		if (m_fRadius >= 0.5f)
		{
			RECT rcDirty = M_Dirty();
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			M_Window()->Size(&nSizeX, &nSizeY);
			if (rcDirty.left < 0) rcDirty.left = 0;
			if (rcDirty.top < 0) rcDirty.top = 0;
			if (rcDirty.right > LONG(nSizeX)) rcDirty.right = nSizeX;
			if (rcDirty.bottom > LONG(nSizeY)) rcDirty.bottom = nSizeY;
			RECT rcBounds = rcDirty;
			BOOL bSolidSel = TRUE;
			M_Window()->GetSelectionInfo(&rcBounds, &bSolidSel);
			if (rcDirty.left < rcBounds.left) rcDirty.left = rcBounds.left;
			if (rcDirty.top < rcBounds.top) rcDirty.top = rcBounds.top;
			if (rcDirty.right > rcBounds.right) rcDirty.right = rcBounds.right;
			if (rcDirty.bottom > rcDirty.bottom) rcDirty.bottom = rcDirty.bottom;
			RECT rcUpdating = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
			if (rcDirty.left >= rcUpdating.right || rcDirty.right <= rcUpdating.left ||
				rcDirty.top >= rcUpdating.bottom || rcDirty.bottom <= rcUpdating.top ||
				rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom)
				return S_OK; // updated is not dirty
			CAutoVectorPtr<TRasterImagePixel> cSource(new TRasterImagePixel[(rcDirty.right-rcDirty.left)*(rcDirty.bottom-rcDirty.top)]);
			M_Window()->GetImageTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, a_fGamma, rcDirty.right-rcDirty.left, EITIBoth, cSource);
			CAutoVectorPtr<BYTE> cMask(new BYTE[(rcDirty.right-rcDirty.left)*(rcDirty.bottom-rcDirty.top)]);
			if (!bSolidSel)
			{
				if (FAILED(M_Window()->GetSelectionTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, rcDirty.right-rcDirty.left, cMask)))
					bSolidSel = TRUE;
			}

			static double const RED_FACTOR = 0.5133333;
			static double const GREEN_FACTOR = 1.0;
			static double const BLUE_FACTOR = 0.1933333;
			static double const MAX_BRIGHTNESS = 100.0;

			double fMaxAdjustedRed = 0.0;
			TRasterImagePixel* p = cSource;
			BYTE* pM = cMask;
			float const fRadNone = m_fRadius+1.0f;
			float const fRadSq = fRadNone*fRadNone;
			float const fRadFull = m_fRadius <= 1.5f ? m_fRadius-1.0f : (m_fRadius-powf((m_fRadius-1.5f)*0.1f, 0.75f)-1.0f);
			float const fRadSqFull = fRadFull*fRadFull;
			float const fFact = 1.0f/(fRadNone-fRadFull-1.0);
			float const fRadMin = m_fRadius-(fRadNone-fRadFull-1.0f);
			for (LONG y = rcDirty.top; y < rcDirty.bottom; ++y)
			{
				for (LONG x = rcDirty.left; x < rcDirty.right; ++x, ++p, ++pM)
				{
					float const fDistSq = (m_tCenter.fX-x-0.5f)*(m_tCenter.fX-x-0.5f)+(m_tCenter.fY-y-0.5f)*(m_tCenter.fY-y-0.5f);
					if (fDistSq > fRadSq)
					{
						*pM = 0;
						continue;
					}
					if (bSolidSel)
						*pM = 255;
					if (fDistSq > fRadSqFull)
					{
						float const fDist = sqrtf(fDistSq);
						float const fDistM = fDist-0.5f;
						float const fDistP = fDist+0.5f;
						if (fDistM >= fRadMin && fDistP <= m_fRadius)
						{
							ATLASSERT(*pM*(m_fRadius-fDist)*fFact < 256);
							*pM = *pM*(m_fRadius-fDist)*fFact;
						}
						else
						{
							float f = 0.0f;
							if (fDist <= m_fRadius)
								f += fDist >= fRadMin ? (m_fRadius-fDist)*fFact : 1.0f;
							if (fDistM <= m_fRadius)
								f += fDistM >= fRadMin ? (m_fRadius-fDistM)*fFact : 1.0f;
							if (fDistP <= m_fRadius)
								f += fDistP >= fRadMin ? (m_fRadius-fDistP)*fFact : 1.0f;
							ATLASSERT(*pM*f*0.3333333f < 256);
							*pM = *pM*f*0.3333333f;
						}
					}
					double fAdjustedRed = p->bR * RED_FACTOR;
					if (fAdjustedRed > fMaxAdjustedRed)
						fMaxAdjustedRed = fAdjustedRed;
				}
			}

			pM = cMask.m_p;
			ULONG nDelta = rcDirty.right-rcDirty.left;
			if (rcDirty.left < rcUpdating.left) { pM += rcUpdating.left-rcDirty.left; rcDirty.left = rcUpdating.left; }
			if (rcDirty.top < rcUpdating.top) { pM += (rcUpdating.top-rcDirty.top)*nDelta; rcDirty.top = rcUpdating.top; }
			if (rcDirty.right > rcUpdating.right) rcDirty.right = rcUpdating.right;
			if (rcDirty.bottom > rcUpdating.bottom) rcDirty.bottom = rcUpdating.bottom;
			nDelta -= rcDirty.right-rcDirty.left;
			for (LONG y = rcDirty.top; y < rcDirty.bottom; ++y)
			{
				p = a_pBuffer + a_nStride*(y-a_nY) - a_nX;
				for (LONG x = rcDirty.left; x < rcDirty.right; ++x, ++pM)
				{
					if (*pM == 0)
						continue;
					double fAdjustedRed   = p[x].bR * RED_FACTOR;
					double fAdjustedGreen = p[x].bG * GREEN_FACTOR;
					double fAdjustedBlue  = p[x].bB * BLUE_FACTOR;

					double fThred = ( fAdjustedGreen + fAdjustedBlue ) / 2;
					double att = (double)(m_cData.fBrightness)/MAX_BRIGHTNESS;
					double slopecorr =  pow(10, (double)(m_cData.fSlope)/255);
					double const nom = (fMaxAdjustedRed-fThred)*(fAdjustedRed-fThred);
					double k = nom == 0.0 ? 1.0 : 1.0 + slopecorr*(att-1)/nom;
				  
					if (k<att) k = att;
					if (k>1) k = 1;
				    
					if (fAdjustedRed >= fAdjustedGreen - m_cData.fThreshold &&
						fAdjustedRed >= fAdjustedBlue - m_cData.fThreshold)
					{
						if (*pM == 255)
						{
							p[x].bR = (int) (k * fThred / RED_FACTOR);
							p[x].bG = (int) (k * fAdjustedGreen / GREEN_FACTOR);
							p[x].bB = (int) (k * fAdjustedBlue / BLUE_FACTOR);
						}
						else
						{
							ULONG const nW = 255-*pM;
							ULONG const nR = (int) (k * fThred / RED_FACTOR);
							ULONG const nG = (int) (k * fAdjustedGreen / GREEN_FACTOR);
							ULONG const nB = (int) (k * fAdjustedBlue / BLUE_FACTOR);
							p[x].bR = (nR**pM + p[x].bR*nW)/255;
							p[x].bG = (nG**pM + p[x].bG*nW)/255;
							p[x].bB = (nB**pM + p[x].bB*nW)/255;
						}
					}
				}
				pM += nDelta;
			}
		}
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = m_fRadius >= 0.5f ? M_Dirty() : RECT_EMPTY;
		return m_fRadius >= 0.5f ? S_OK : S_FALSE;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		//a_pPos->fX = floorf(a_pPos->fX);
		//a_pPos->fY = floorf(a_pPos->fY);

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tCenter.fX = a_pPos->fX;//+0.5f;
		m_tCenter.fY = a_pPos->fY;//+0.5f;
		m_fRadius = 0.0f;
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		OnMouseMove(a_eKeysState, a_pPos);
		if (m_fRadius > 0.5f)
			return ETPAApply;

		M_Window()->ControlLinesChanged();
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (!M_Dragging())
			return S_OK;

		float const fRadius = sqrtf((a_pPos->fX-m_tCenter.fX)*(a_pPos->fX-m_tCenter.fX)+(a_pPos->fY-m_tCenter.fY)*(a_pPos->fY-m_tCenter.fY));
		if (fabsf(fRadius-m_fRadius) > 1e-3f)
		{
			RECT rcPrev = M_Dirty();
			m_fRadius = fRadius;
			M_Window()->ControlLinesChanged();
			RECT rcDirty = M_Dirty();
			if (rcDirty.left > rcPrev.left) rcDirty.left = rcPrev.left;
			if (rcDirty.top > rcPrev.top) rcDirty.top = rcPrev.top;
			if (rcDirty.right < rcPrev.right) rcDirty.right = rcPrev.right;
			if (rcDirty.bottom < rcPrev.bottom) rcDirty.bottom = rcPrev.bottom;
			M_Window()->RectangleChanged(&rcDirty);
		}
		return S_OK;
	}

	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (M_Dragging() && m_fRadius != 0.0f)
		{
			float fHandleSize = 1.0f;
			a_pLines->HandleSize(&fHandleSize);
			int nSteps = m_fRadius/fHandleSize;
			if (nSteps < 12) nSteps = 12;
			else if (nSteps > 120) nSteps = 120;
			float const fStep = 3.14159265f*2.0f/nSteps;
			a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY-m_fRadius);
			for (int i = 1; i < nSteps; ++i)
			{
				float fAngle = i*fStep;
				a_pLines->LineTo(m_tCenter.fX+m_fRadius*sinf(fAngle), m_tCenter.fY-m_fRadius*cosf(fAngle));
			}
			a_pLines->Close();
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}
	RECT M_Dirty()
	{
		RECT rc = {floorf(m_tCenter.fX-m_fRadius)-1, floorf(m_tCenter.fY-m_fRadius)-1, ceilf(m_tCenter.fX+m_fRadius)+2, ceilf(m_tCenter.fY+m_fRadius)+2};
		return rc;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

private:
	TPixelCoords m_tCenter;
	float m_fRadius;
	CEditToolDataRedEye m_cData;
};

