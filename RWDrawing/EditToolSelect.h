
#pragma once

#include "EditTool.h"


class CEditToolSelect :
	public CEditToolMouseInput<CEditToolSelect>, // no direct tablet support
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolSelect, // T - the top level class for cross casting
		CEditToolSelect, // TResetHandler
		CEditToolSelect, // TDirtyHandler
		CRasterImageEditToolBase, // TImageTileHandler
		CEditToolSelect, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolSelect, // TAdjustCoordsHandler
		CRasterImageEditToolBase, // TGetCursorHandler
		CEditToolMouseInput<CEditToolSelect>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CRasterImageEditToolBase // TControlLinesHandler
	>
{
public:
	CEditToolSelect() : m_bValid(false)
	{
	}

	BEGIN_COM_MAP(CEditToolSelect)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		m_bValid = false;
		RECT rc;
		if (_IsDirty(NULL, NULL, &rc) == S_OK)
			M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
		{
			if (m_bValid)
			{
				if (m_eKeysState == ECKSNone)
				{
					a_pSelectionRect->left = a_pSelectionRect->top = 0;
					a_pSelectionRect->right = m_nSizeX;
					a_pSelectionRect->bottom = m_nSizeY;
				}
				else
				{
					a_pSelectionRect->left = m_tDragStart.fX < m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX;
					a_pSelectionRect->top = m_tDragStart.fY < m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY;
					a_pSelectionRect->right = 1+(m_tDragStart.fX > m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX);
					a_pSelectionRect->bottom = 1+(m_tDragStart.fY > m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY);
				}
			}
			else
				*a_pSelectionRect = RECT_EMPTY;
		}
		return M_Dragging() ? S_OK : S_FALSE;
	}
	HRESULT _GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (!m_bValid)
			return M_Window()->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
		int nX1 = m_tDragStart.fX < m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX;
		int nX2 = 1+(m_tDragStart.fX > m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX);
		int nY1 = m_tDragStart.fY < m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY;
		int nY2 = 1+(m_tDragStart.fY > m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY);
		if (m_eKeysState == ECKSNone)
		{
			a_pBoundingRectangle->left = nX1;
			a_pBoundingRectangle->right = nX2;
			a_pBoundingRectangle->top = nY1;
			a_pBoundingRectangle->bottom = nY2;
			*a_bEntireRectangle = TRUE;
			return S_OK;
		}
		RECT rcPrev;
		BOOL bEntire;
		M_Window()->GetSelectionInfo(&rcPrev, &bEntire);
		a_pBoundingRectangle->left = min(nX1, rcPrev.left);
		a_pBoundingRectangle->top = min(nY1, rcPrev.top);
		a_pBoundingRectangle->right = max(nX2, rcPrev.right);
		a_pBoundingRectangle->bottom = max(nY2, rcPrev.bottom);
		*a_bEntireRectangle = FALSE;
		return S_OK;
	}
	HRESULT _GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		if (!m_bValid)
			return M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
		int nX1 = m_tDragStart.fX < m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX;
		int nX2 = 1+(m_tDragStart.fX > m_tDragLast.fX ? m_tDragStart.fX : m_tDragLast.fX);
		int nY1 = m_tDragStart.fY < m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY;
		int nY2 = 1+(m_tDragStart.fY > m_tDragLast.fY ? m_tDragStart.fY : m_tDragLast.fY);
		if (nX1 < a_nX) nX1 = a_nX;
		if (nX2 > LONG(a_nX+a_nSizeX)) nX2 = a_nX+a_nSizeX;
		if (nY1 < a_nY) nY1 = a_nY;
		if (nY2 > LONG(a_nY+a_nSizeY)) nY2 = a_nY+a_nSizeY;
		BYTE bInside = 255;
		if (m_eKeysState == ECKSNone)
		{
			if (nX1 >= nX2 || nY1 >= nY2)
			{
				if (a_nStride == a_nSizeX)
				{
					FillMemory(a_pBuffer, a_nSizeX*a_nSizeY, 0);
				}
				else
				{
					for (ULONG y = 0; y < a_nSizeY; ++y)
						FillMemory(a_pBuffer+a_nStride*y, a_nSizeX, 0);
				}
				return S_OK;
			}
		}
		else
		{
			HRESULT hRes = M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
			if (nX1 >= nX2 || nY1 >= nY2)
				return hRes;
			if (FAILED(hRes))
			{
				if (m_eKeysState == ECKSShift)
					return hRes;
				bInside = 0;
			}
			else
			{
				for (LONG y = nY1; y < nY2; ++y)
				{
					BYTE* pLine = a_pBuffer + a_nStride*(y-a_nY) + nX1-a_nX;
					if (m_eKeysState == ECKSShift)
					{
						// add to selection
						FillMemory(pLine, nX2-nX1, 0xff);
					}
					else if (m_eKeysState == ECKSControl)
					{
						// negate selection
						BYTE* pEnd = pLine + nX2-nX1;
						while (pLine < pEnd)
						{
							*pLine = 255-*pLine;
							++pLine;
						}
					}
					else
					{
						// remove from selection
						FillMemory(pLine, nX2-nX1, 0);
					}
				}
				return S_OK;
			}
		}
		BYTE const bOutside = 255-bInside;
		// overwrite entire selection
		if (nY1 > a_nY)
		{
			if (a_nSizeX == a_nStride)
				FillMemory(a_pBuffer, a_nSizeX*(nY1-a_nY), bOutside);
			else
				for (LONG y = a_nY; y < nY1; ++y) FillMemory(a_pBuffer+(y-nY1)*a_nStride, a_nSizeX, bOutside);
		}
		if (nX2-nX1 == a_nSizeX)
		{
			if (a_nSizeX == a_nStride)
				FillMemory(a_pBuffer+a_nStride*(nY1-a_nY), a_nSizeX*(nY2-nY1), bInside);
			else
				for (LONG y = nY1; y < nY2; ++y) FillMemory(a_pBuffer+(y-a_nY)*a_nStride, a_nSizeX, bInside);
		}
		else
		{
			for (LONG y = nY1; y < nY2; ++y)
			{
				BYTE* pLine = a_pBuffer + a_nStride*(y-a_nY);
				if (nX1 > a_nX)
					FillMemory(pLine, nX1-a_nX, bOutside);
				FillMemory(pLine+nX1-a_nX, nX2-nX1, bInside);
				if (nX2 < LONG(a_nX+a_nSizeX))
					FillMemory(pLine+nX2-a_nX, a_nX+a_nSizeX-nX2, bOutside);
			}
		}
		if (nY2 < LONG(a_nY+a_nSizeY))
		{
			if (a_nSizeX == a_nStride)
				FillMemory(a_pBuffer+a_nSizeX*(nY2-a_nY), a_nSizeX*(a_nY+a_nSizeY-nY2), bOutside);
			else
				for (LONG y = nY2; y < LONG(a_nY+a_nSizeY); ++y) FillMemory(a_pBuffer+(y-a_nY)*a_nStride, a_nSizeX, bOutside);
		}
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		if (a_pPos->fX < 0.0f)
			a_pPos->fX = 0.0f;
		else if (int(a_pPos->fX) >= int(m_nSizeX))
			a_pPos->fX = m_nSizeX-1;
		else
			a_pPos->fX = int(a_pPos->fX);

		if (a_pPos->fY < 0.0f)
			a_pPos->fY = 0.0f;
		else if (int(a_pPos->fY) >= int(m_nSizeY))
			a_pPos->fY = m_nSizeY-1;
		else
			a_pPos->fY = int(a_pPos->fY);

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tDragStart = m_tDragLast = *a_pPos;
		m_eKeysState = a_eKeysState;
		m_bValid = true;
		if (m_eKeysState == ECKSNone)
		{
			RECT rc = {0, 0, m_nSizeX, m_nSizeY};
			M_Window()->RectangleChanged(&rc);
		}
		else
		{
			RECT rc = {m_tDragStart.fX, m_tDragStart.fY, m_tDragStart.fX+1, m_tDragStart.fY+1};
			M_Window()->RectangleChanged(&rc);
		}
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tDragLast = *a_pPos;
		return ETPAApply;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (M_Dragging())
		{
			RECT rc =
			{
				m_tDragStart.fX < m_tDragLast.fX ? (a_pPos->fX < m_tDragStart.fX ? a_pPos->fX : m_tDragStart.fX) : (a_pPos->fX < m_tDragLast.fX ? a_pPos->fX : m_tDragLast.fX),
				m_tDragStart.fY < m_tDragLast.fY ? (a_pPos->fY < m_tDragStart.fY ? a_pPos->fY : m_tDragStart.fY) : (a_pPos->fY < m_tDragLast.fY ? a_pPos->fY : m_tDragLast.fY),
				1+(m_tDragStart.fX > m_tDragLast.fX ? (a_pPos->fX > m_tDragStart.fX ? a_pPos->fX : m_tDragStart.fX) : (a_pPos->fX > m_tDragLast.fX ? a_pPos->fX : m_tDragLast.fX)),
				1+(m_tDragStart.fY > m_tDragLast.fY ? (a_pPos->fY > m_tDragStart.fY ? a_pPos->fY : m_tDragStart.fY) : (a_pPos->fY > m_tDragLast.fY ? a_pPos->fY : m_tDragLast.fY)),
			};
			m_tDragLast = *a_pPos;
			M_Window()->RectangleChanged(&rc);
		}
		return S_OK;
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
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	TPixelCoords m_tDragStart;
	TPixelCoords m_tDragLast;
	bool m_bValid;
	EControlKeysState m_eKeysState;
};

