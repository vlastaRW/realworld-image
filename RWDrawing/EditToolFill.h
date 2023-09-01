
#pragma once

#include "EditTool.h"
#include "EditToolPixelMixer.h"
#include "EditToolWithBrush.h"
#include "EditToolWithCtrlDropper.h"


HICON GetToolIconFILL(ULONG a_nSize);

class CEditToolFill :
	public CEditToolMouseInput<CEditToolFill>, // no direct tablet support
	public CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>,
	public CEditToolWithCtrlDropper<CEditToolFill, CEditToolMouseInput<CEditToolFill>, CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, CEditToolFill>,
	public CEditToolCustomPrecisionCursor<GetToolIconFILL>,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolFill, // T - the top level class for cross casting
		CEditToolFill, // TResetHandler
		CEditToolFill, // TDirtyHandler
		CEditToolFill, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, // TBrushHandler
		CEditToolFill, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolFill, CEditToolMouseInput<CEditToolFill>, CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, CEditToolFill>, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolFill, CEditToolMouseInput<CEditToolFill>, CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, CEditToolFill>, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolFill, CEditToolMouseInput<CEditToolFill>, CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, CEditToolFill>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolFill, CRasterImageEditToolBase, CEditToolFill> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolFill() : m_eBlendingMode(EBMDrawOver), m_bFill(false),
		m_pGamma(NULL), m_bJustCreated(true)
	{
		RWCoCreateInstance(m_pGammaCache, __uuidof(GammaTableCache));
		if (m_pGammaCache)
			m_pGamma = m_pGammaCache->GetSRGBTable();
	}

	BEGIN_COM_MAP(CEditToolFill)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		bool bFill = m_bFill;
		m_bFill = m_bJustCreated;
		m_bJustCreated = false;
		EnableBrush(m_bFill);
		if (bFill != m_bFill)
			M_Window()->RectangleChanged(NULL);
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
		{
			if (m_bFill)
			{
				a_pImageRect->left = a_pImageRect->top = 0;
				a_pImageRect->right = m_nSizeX;
				a_pImageRect->bottom = m_nSizeY;
			}
			else
			{
				*a_pImageRect = RECT_EMPTY;
			}

		}
		return m_bFill ? S_OK : S_FALSE;
	}

	template<class TPixelMixer, bool t_bCorrect>
	void RenderPixels(RECT const& a_rc, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask)
	{
		TRasterImagePixel t = {0, 0, 0, 0};
		if (M_Brush())
		{
			if (S_OK == M_Brush()->IsSolid(&a_rc))
			{
				M_Brush()->GetBrushTile(a_rc.left, a_rc.top, 1, 1, a_fGamma, 1, &t);
			}
			else
			{
				CAutoVectorPtr<TRasterImagePixel> cBrush(new TRasterImagePixel[(a_rc.right-a_rc.left)*(a_rc.bottom-a_rc.top)]);
				M_Brush()->GetBrushTile(a_rc.left, a_rc.top, a_rc.right-a_rc.left, a_rc.bottom-a_rc.top, a_fGamma, a_rc.right-a_rc.left, cBrush.m_p);

				for (LONG nY = a_rc.top; nY < a_rc.bottom; ++nY)
				{
					ULONG nX1 = a_rc.left;
					ULONG const nX2 = a_rc.right;
					TRasterImagePixel* p = a_pBuffer+(nY-a_rc.top)*a_nStride-a_rc.left;
					TRasterImagePixel* pB = cBrush.m_p+(nY-a_rc.top)*(a_rc.right-a_rc.left)-a_rc.left;
					if (a_pMask)
					{
						BYTE* pM = a_pMask+(nY-a_rc.top)*a_nMaskStride-a_rc.left;
						for (; nX1 < nX2; ++nX1)
						{
							if (t_bCorrect)
								TPixelMixer::Mix(p[nX1], pB[nX1], pM[nX1], m_pGamma);
							else
								TPixelMixer::Mix(p[nX1], pB[nX1], pM[nX1]);
						}
					}
					else
					{
						for (; nX1 < nX2; ++nX1)
						{
							if (t_bCorrect)
								TPixelMixer::Mix(p[nX1], pB[nX1], m_pGamma);
							else
								TPixelMixer::Mix(p[nX1], pB[nX1]);
						}
					}
				}
				return;
			}
		}
		for (LONG nY = a_rc.top; nY < a_rc.bottom; ++nY)
		{
			ULONG nX1 = a_rc.left;
			ULONG const nX2 = a_rc.right;
			TRasterImagePixel* p = a_pBuffer+(nY-a_rc.top)*a_nStride-a_rc.left;
			if (a_pMask)
			{
				BYTE* pM = a_pMask+(nY-a_rc.top)*a_nMaskStride-a_rc.left;
				if (t.bA)
					for (; nX1 < nX2; ++nX1)
						if (t_bCorrect)
							TPixelMixer::Mix(p[nX1], t, pM[nX1], m_pGamma);
						else
							TPixelMixer::Mix(p[nX1], t, pM[nX1]);
				else
					for (; nX1 < nX2; ++nX1)
						if (t_bCorrect)
							CPixelMixerReplace::Mix(p[nX1], t, pM[nX1], m_pGamma);
						else
							CPixelMixerReplace::Mix(p[nX1], t, pM[nX1]);
			}
			else
			{
				if (t.bA)
					for (; nX1 < nX2; ++nX1)
						if (t_bCorrect)
							TPixelMixer::Mix(p[nX1], t, m_pGamma);
						else
							TPixelMixer::Mix(p[nX1], t);
				else
					for (; nX1 < nX2; ++nX1)
						if (t_bCorrect)
							CPixelMixerReplace::Mix(p[nX1], t, m_pGamma);
						else
							CPixelMixerReplace::Mix(p[nX1], t);
			}
		}
	}
	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		HRESULT hRes = M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes) || !m_bFill)
			return hRes;

		CAutoVectorPtr<BYTE> cMask;
		RECT rcSel = {0, 0, a_nSizeX, a_nSizeY};
		BOOL bSolidSel = TRUE;
		M_Window()->GetSelectionInfo(&rcSel, &bSolidSel);
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
			if (FAILED(M_Window()->GetSelectionTile(rc.left, rc.top, sz.cx, sz.cy, sz.cx, cMask)))
				cMask.Free();
		}

		if (a_fGamma == 1.0f || m_pGamma == NULL)
		{
			switch (m_eBlendingMode)
			{
			case EBMDrawOver:	RenderPixels<CPixelMixerPaintOver, false>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			case EBMDrawUnder:	RenderPixels<CPixelMixerPaintUnder, false>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			case EBMReplace:	RenderPixels<CPixelMixerReplace, false>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			}
		}
		else
		{
			switch (m_eBlendingMode)
			{
			case EBMDrawOver:	RenderPixels<CPixelMixerPaintOver, true>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			case EBMDrawUnder:	RenderPixels<CPixelMixerPaintUnder, true>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			case EBMReplace:	RenderPixels<CPixelMixerReplace, true>(rc, a_fGamma, a_nStride, a_pBuffer + (rc.top-a_nY)*a_nStride + rc.left-a_nX, sz.cx, cMask); break;
			}
		}
		return S_OK;
	}

	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			if (m_bFill)
			{
				//EnableBrush(m_bFill);
				RECT rc = {0, 0, m_nSizeX, m_nSizeY};
				M_Window()->RectangleChanged(&rc);
			}
		}
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		a_pPos->fX = floorf(a_pPos->fX+0.5f*a_pPointerSize->fX);
		a_pPos->fY = floorf(a_pPos->fY+0.5f*a_pPointerSize->fY);

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (!m_bFill)
		{
			m_bFill = true;
			if (M_Brush())
			{
				TPixelCoords tCenter = {m_nSizeX*0.5f, m_nSizeY*0.5f};
				M_Brush()->SetShapeBounds(&tCenter, tCenter.fX, tCenter.fY, 0.0f);
				EnableBrush(true);
			}
			RECT rc = {0, 0, m_nSizeX, m_nSizeY};
			M_Window()->RectangleChanged(&rc);
		}
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		return S_OK;
	}

	HRESULT _GetCursor(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		M_Window()->Size(&nSizeX, &nSizeY);
		if (a_pPos->fX < 0.0f || a_pPos->fY < 0.0f || a_pPos->fX >= nSizeX || a_pPos->fY >= nSizeY)
		{
			static HCURSOR h = ::LoadCursor(NULL, IDC_NO);
			*a_phCursor = h;
			return S_OK;
		}
		else
		{
			return CEditToolCustomPrecisionCursor<GetToolIconFILL>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
		}
	}

	HRESULT _GetControlLines(IEditToolControlLines* UNREF(a_pLines), ULONG UNREF(a_nLineTypes))
	{
		return S_OK;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		TransformBrush(a_pMatrix);
		return S_OK;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		return S_FALSE;
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	EBlendingMode m_eBlendingMode;
	bool m_bFill;
	bool m_bJustCreated;

	CComPtr<IGammaTableCache> m_pGammaCache;
	CGammaTables const* m_pGamma;
};
