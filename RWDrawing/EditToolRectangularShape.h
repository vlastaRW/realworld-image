
#pragma once


template<class T>
class CEditToolRectangularShape
{
public:
	enum ERectangleMode
	{
		ERMNonSquare = 1,
		ERMRotated = 2,
		ERMCoaxialSquare = 0,
		ERMCoaxialRectangle = ERMNonSquare,
		ERMRotatedSquare = ERMRotated,
		ERMRotatedRectangle = ERMRotated|ERMNonSquare,
		ERMLockAspectRectangle = 4,
		ERMRotatedLockAspectRectangle = ERMRotated|ERMLockAspectRectangle,
		ERMQuadrangle = 8,
	};
	enum EState
	{
		ESClean = 0,
		ESDragging,
		ESFloating,
		ESMoving
	};

	enum
	{
		eStep = 5 // rotation step in integral mode
	};

public:
	CEditToolRectangularShape(ERectangleMode a_eMode = ERMRotatedRectangle, bool a_bLinesWhenDefining = false, bool a_bCornersOnly = true) :
		m_eMode(a_eMode), m_bControlLines(true), m_bLinesWhenDefining(a_bLinesWhenDefining),
		m_fAspectLock(1.0f), m_bCornersOnly(a_bCornersOnly)
	{
		ResetRectangle();
	}

	void ShapeChanged(bool a_bInitFinish) // override in T
	{
	}
	EState M_State() const
	{
		return m_eState;
	}
	bool RectangleDefined() const
	{
		if (m_eState == ESClean)
			return false;
		return m_aPixels[0].fX != m_aPixels[1].fX || m_aPixels[0].fY != m_aPixels[1].fY ||
			   m_aPixels[0].fX != m_aPixels[2].fX || m_aPixels[0].fY != m_aPixels[2].fY ||
			   m_aPixels[0].fX != m_aPixels[3].fX || m_aPixels[0].fY != m_aPixels[3].fY;
	}
	bool RectangleMoving() const
	{
		return m_eState == ESMoving;
	}
	void ResetRectangle()
	{
		m_fAspectLock = 1.0f;
		m_eState = ESClean;
		m_fSizeX = m_fSizeY = m_fAngle = m_tCenter.fX = m_tCenter.fY = 0.0f;
		m_aPixels[3] = m_aPixels[2] = m_aPixels[1] = m_aPixels[0] = m_tCenter;
	}
	bool GetRectangle(TPixelCoords* a_pCenter, float* a_pSizeX, float* a_pSizeY, float* a_pAngle) const
	{
		if (m_eMode == ERMQuadrangle)
			return false;
		*a_pCenter = m_tCenter;
		*a_pSizeX = 0.5f*m_fSizeX;
		*a_pSizeY = 0.5f*m_fSizeY;
		*a_pAngle = m_fAngle;
		return RectangleDefined();
	}
	TPixelCoords const* GetRectanglePoints() const
	{
		return m_aPixels;
	}
	void SetRectangle(float a_fCenterX, float a_fCenterY, float a_fSizeX, float a_fSizeY, float a_fAngle)
	{
		if (m_eMode != ERMRotatedRectangle && m_eMode != ERMRotatedLockAspectRectangle && m_eMode != ERMQuadrangle)
		{
			if (a_fSizeX != a_fSizeY)
				SetRectangleMode(ERMRotatedRectangle);
		}
		m_tCenter.fX = a_fCenterX;
		m_tCenter.fY = a_fCenterY;
		m_fSizeX = a_fSizeX*2.0f;
		m_fSizeY = a_fSizeY*2.0f;
		m_fAngle = a_fAngle;
		PointsFromParams();
		m_eState = ESFloating;
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		static_cast<T*>(this)->ShapeChanged(false);
	}
	void SetRectanglePoints(float a_fX1, float a_fY1, float a_fX2, float a_fY2, float a_fX3, float a_fY3, float a_fX4, float a_fY4)
	{
		if (m_eMode != ERMQuadrangle)
			SetRectangleMode(ERMQuadrangle);
		m_aPixels[0].fX = a_fX1;
		m_aPixels[0].fY = a_fY1;
		m_aPixels[1].fX = a_fX2;
		m_aPixels[1].fY = a_fY2;
		m_aPixels[2].fX = a_fX3;
		m_aPixels[2].fY = a_fY3;
		m_aPixels[3].fX = a_fX4;
		m_aPixels[3].fY = a_fY4;
		m_eState = ESFloating;
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		static_cast<T*>(this)->ShapeChanged(false);
	}
	RECT GetBoundingBox() const
	{
		RECT rc = RECT_EMPTY;
		if (RectangleDefined())
		{
			TPixelCoords tMin = m_aPixels[0];
			TPixelCoords tMax = m_aPixels[0];
			for (int i = 1; i < 4; ++i)
			{
				if (tMin.fX > m_aPixels[i].fX) tMin.fX = m_aPixels[i].fX;
				if (tMin.fY > m_aPixels[i].fY) tMin.fY = m_aPixels[i].fY;
				if (tMax.fX < m_aPixels[i].fX) tMax.fX = m_aPixels[i].fX;
				if (tMax.fY < m_aPixels[i].fY) tMax.fY = m_aPixels[i].fY;
			}
			rc.left = floorf(tMin.fX);
			rc.top = floorf(tMin.fY);
			rc.right = ceilf(tMax.fX);
			rc.bottom = ceilf(tMax.fY);
		}
		return rc;
	}
	bool RectangleHitTest(float a_fX, float a_fY) const
	{
		double angles[5] =
		{
			atan2(m_aPixels[0].fX-a_fX, m_aPixels[0].fY-a_fY),
			atan2(m_aPixels[1].fX-a_fX, m_aPixels[1].fY-a_fY),
			atan2(m_aPixels[2].fX-a_fX, m_aPixels[2].fY-a_fY),
			atan2(m_aPixels[3].fX-a_fX, m_aPixels[3].fY-a_fY),
			atan2(m_aPixels[0].fX-a_fX, m_aPixels[0].fY-a_fY),
		};
		double fi = 0.0;
		for (int i = 0; i < 4; ++i)
		{
			double delta = angles[i+1]-angles[i];
			if (delta < 3.1415926535897932 && delta > -3.1415926535897932)
				fi += delta;
			else
				fi -= delta;
		}
		return fi > 3.1415926535897932 || fi < -3.1415926535897932;
	}

	//TPixelCoords const& M_Pixel1()
	//{
	//	return m_tPixel1;
	//}
	//TPixelCoords const& M_Pixel2()
	//{
	//	return m_tPixel2;
	//}
	float M_Angle()
	{
		return m_fAngle;
	}
	bool SetAspectLock(float a_fAspectLock)
	{
		if (m_fAspectLock == a_fAspectLock)
			return false;
		m_fAspectLock = a_fAspectLock;
		return true;
	}
	bool SetRectangleMode(ERectangleMode a_eMode)
	{
		if (m_eMode == a_eMode)
			return false;
		ERectangleMode const ePrev = m_eMode;
		m_eMode = a_eMode;
		if (!RectangleDefined())
			return false;
		bool bReturn = false;
		if (ePrev == ERMQuadrangle)
		{
			RECT rc = GetBoundingBox();
			m_tCenter.fX = 0.5f*(rc.left+rc.right);
			m_tCenter.fY = 0.5f*(rc.top+rc.bottom);
			m_fAngle = 0.0f;
			if ((m_eMode&ERMNonSquare) == 0)
			{
				float f = sqrtf((rc.right-rc.left)*(rc.right-rc.left) + (rc.bottom-rc.top)*(rc.bottom-rc.top));
				m_fSizeX = f/sqrtf(m_fAspectLock);
				m_fSizeY = f*sqrtf(m_fAspectLock);
				if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
				{
					m_fSizeX = floorf(m_fSizeX+0.5f);
					m_fSizeY = floorf(m_fSizeY+0.5f);
				}
			}
			else
			{
				m_fSizeX = rc.right-rc.left;
				m_fSizeY = rc.bottom-rc.top;
			}
			PointsFromParams();
			static_cast<T*>(this)->M_Window()->ControlPointsChanged();
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			return true;
		}
		else if (m_eMode != ERMQuadrangle)
		{
			if ((m_eMode&ERMNonSquare) == 0 && (ePrev&ERMNonSquare))
			{
				// rectangle to square
				float fSizeX = 0.5f*(m_fSizeX+m_fSizeY/m_fAspectLock);
				float fSizeY = fSizeX*m_fAspectLock;
				if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
				{
					fSizeX = floorf(fSizeX+0.5f);
					fSizeY = floorf(fSizeY+0.5f);
				}
				if (fSizeX != m_fSizeX || fSizeY != m_fSizeY)
				{
					m_fSizeX = fSizeX;
					m_fSizeY = fSizeY;
					if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
					{
						if (int(fSizeX+0.5f)&1)
							m_tCenter.fX = 0.5f + floorf(m_tCenter.fX);
						else
							m_tCenter.fX = floorf(m_tCenter.fX + 0.5f);
						if (int(fSizeY+0.5f)&1)
							m_tCenter.fY = 0.5f + floorf(m_tCenter.fY);
						else
							m_tCenter.fY = floorf(m_tCenter.fY + 0.5f);
					}
					PointsFromParams();
					bReturn = true;
				}
			}
			if ((m_eMode&ERMRotated) == 0 && (ePrev&ERMRotated) && m_fAngle != 0.0f)
			{
				m_fAngle = 0.0f;
				PointsFromParams();
				bReturn = true;
				static_cast<T*>(this)->M_Window()->ControlPointsChanged();
			}
			else if (bReturn)
			{
				for (ULONG i = 0; i < ((m_eMode&ERMRotated) ? 9UL : 8UL); ++i)
					static_cast<T*>(this)->M_Window()->ControlPointChanged(i);
			}
			if (bReturn)
				static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			return bReturn;
		}
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		return true;
	}
	void SetRectangleControlLines(bool a_bEnabled)
	{
		if (m_bControlLines == a_bEnabled)
			return;
		m_bControlLines = a_bEnabled;
		if (!RectangleDefined())
			return;
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}
	bool AdjustRectangleCoordinates()
	{
		if (static_cast<T*>(this)->M_CoordinatesMode() != ECMIntegral)
			return false;
		float const fAngle = eStep*floorf(m_fAngle*180.0f/3.14159265f/eStep+0.5f)/180.0f*3.14159265f; // eStep degrees steps in integral mode
		bool bChange = fabsf(fAngle-m_fAngle) > 1e-4f;
		if (bChange)
		{
			m_fAngle = fAngle;
			PointsFromParams();
		}
		for (int i = 0; i < 4; ++i)
		{
			if (m_aPixels[i].fX != floorf(m_aPixels[i].fX+0.5f))
				bChange = true;
			m_aPixels[i].fX = floorf(m_aPixels[i].fX+0.5f);
			if (m_aPixels[i].fY != floorf(m_aPixels[i].fY+0.5f))
				bChange = true;
			m_aPixels[i].fY = floorf(m_aPixels[i].fY+0.5f);
		}
		m_fSizeX = sqrtf((m_aPixels[1].fX-m_aPixels[0].fX)*(m_aPixels[1].fX-m_aPixels[0].fX) + (m_aPixels[1].fY-m_aPixels[0].fY)*(m_aPixels[1].fY-m_aPixels[0].fY));
		m_fSizeY = sqrtf((m_aPixels[3].fX-m_aPixels[0].fX)*(m_aPixels[3].fX-m_aPixels[0].fX) + (m_aPixels[3].fY-m_aPixels[0].fY)*(m_aPixels[3].fY-m_aPixels[0].fY));
		m_tCenter.fX = 0.25f*(m_aPixels[0].fX + m_aPixels[1].fX + m_aPixels[2].fX + m_aPixels[3].fX);
		m_tCenter.fY = 0.25f*(m_aPixels[0].fY + m_aPixels[1].fY + m_aPixels[2].fY + m_aPixels[3].fY);
		return bChange;
	}


	DWORD MaxIdleTime()
	{
		return 0;
	}
	HRESULT OnMouseDown(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState == ESFloating)
		{
			if (static_cast<T*>(this)->HitTest(a_pPos->fX, a_pPos->fY))
			{
				m_eState = ESMoving;
				m_tDragStart = *a_pPos;
				return S_OK;
			}
			else
			{
				if (RectangleDefined())
				{
					m_eState = ESDragging;
					ATLASSERT(0);
					//static_cast<T*>(this)->M_Window()->ApplyChanges();
				}
			}
		}
		ResetRectangle();
		static_cast<T*>(this)->DeleteCachedData();
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		m_tDragStart = *a_pPos;
		m_eState = ESDragging;
		return S_OK;
	}
	HRESULT OnMouseUp(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		OnMouseMove(a_eKeysState, a_pPos);
		if (m_eState == ESDragging && m_fSizeX == 0.0f && m_fSizeY == 0.0f)
		{
			m_eState = ESClean;
		}
		else
		{
			if (m_eMode&ERMLockAspectRectangle && m_eState == ESDragging)
				m_fAspectLock = m_fSizeY/m_fSizeX;
			m_eState = ESFloating;
		}
		static_cast<T*>(this)->ShapeChanged(true);
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines && !m_bLinesWhenDefining)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		return S_OK;
	}
	void InitializeShape(TPixelCoords const* a_pFrom, TPixelCoords const* a_pTo, TPixelCoords* a_pCenter, float* a_pSizeX, float* a_pSizeY, float* a_pAngle, TPixelCoords* a_aPixels)
	{
		if (m_eMode == ERMRotatedSquare || m_eMode == ERMCoaxialSquare)
		{
			TPixelCoords t1 = {min(a_pFrom->fX, a_pTo->fX), min(a_pFrom->fY, a_pTo->fY)};
			TPixelCoords t2 = {max(a_pFrom->fX, a_pTo->fX), max(a_pFrom->fY, a_pTo->fY)};
			float const fSize = min((t2.fX-t1.fX)*m_fAspectLock, t2.fY-t1.fY);//sqrtf((m_tDragStart.fX-a_pPos->fX)*(m_tDragStart.fX-a_pPos->fX)+(m_tDragStart.fY-a_pPos->fY)*(m_tDragStart.fY-a_pPos->fY))/sqrtf(2.0f);
			*a_pSizeX = fSize/m_fAspectLock;
			*a_pSizeY = fSize;
			a_aPixels[0].fX = a_aPixels[3].fX = a_pFrom->fX < a_pTo->fX ? a_pTo->fX : a_pFrom->fX-*a_pSizeX;
			a_aPixels[0].fY = a_aPixels[1].fY = a_pFrom->fY < a_pTo->fY ? a_pTo->fY : a_pFrom->fY-*a_pSizeY;
			a_aPixels[1].fX = a_aPixels[2].fX = a_aPixels[0].fX + *a_pSizeX;
			a_aPixels[2].fY = a_aPixels[3].fY = a_aPixels[0].fY + *a_pSizeY;
			a_pCenter->fX = a_aPixels[0].fX + 0.5f**a_pSizeX;
			a_pCenter->fY = a_aPixels[0].fY + 0.5f**a_pSizeY;
		}
		else
		{
			a_aPixels[0].fX = a_aPixels[3].fX = min(a_pFrom->fX, a_pTo->fX);
			a_aPixels[0].fY = a_aPixels[1].fY = min(a_pFrom->fY, a_pTo->fY);
			a_aPixels[1].fX = a_aPixels[2].fX = max(a_pFrom->fX, a_pTo->fX);
			a_aPixels[2].fY = a_aPixels[3].fY = max(a_pFrom->fY, a_pTo->fY);
			a_pCenter->fX = 0.5f*(a_aPixels[0].fX + a_aPixels[2].fX);
			a_pCenter->fY = 0.5f*(a_aPixels[0].fY + a_aPixels[2].fY);
			*a_pSizeX = a_aPixels[2].fX - a_aPixels[0].fX;
			*a_pSizeY = a_aPixels[2].fY - a_aPixels[0].fY;
		}
		*a_pAngle = 0.0f;
	}
	HRESULT OnMouseMove(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (a_pPos->fX == m_tDragStart.fX && a_pPos->fY == m_tDragStart.fY)
			return S_FALSE;
		if (m_eState == ESDragging)
		{
			static_cast<T*>(this)->InitializeShape(&m_tDragStart, a_pPos, &m_tCenter, &m_fSizeX, &m_fSizeY, &m_fAngle, m_aPixels);
			static_cast<T*>(this)->ShapeChanged(false);
			if (m_bLinesWhenDefining)
				static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			return S_OK;
		}
		else if (m_eState == ESMoving)
		{
			m_aPixels[0].fX += a_pPos->fX-m_tDragStart.fX;
			m_aPixels[0].fY += a_pPos->fY-m_tDragStart.fY;
			m_aPixels[1].fX += a_pPos->fX-m_tDragStart.fX;
			m_aPixels[1].fY += a_pPos->fY-m_tDragStart.fY;
			m_aPixels[2].fX += a_pPos->fX-m_tDragStart.fX;
			m_aPixels[2].fY += a_pPos->fY-m_tDragStart.fY;
			m_aPixels[3].fX += a_pPos->fX-m_tDragStart.fX;
			m_aPixels[3].fY += a_pPos->fY-m_tDragStart.fY;
			m_tCenter.fX += a_pPos->fX-m_tDragStart.fX;
			m_tCenter.fY += a_pPos->fY-m_tDragStart.fY;
			m_tDragStart = *a_pPos;
			for (ULONG i = 0; i < 9; ++i) static_cast<T*>(this)->M_Window()->ControlPointChanged(i);
			if (m_bControlLines)
				static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			static_cast<T*>(this)->ShapeChanged(false);
			return S_OK;
		}
		return S_OK;
	}
	HRESULT OnMouseLeave()
	{
		return E_NOTIMPL;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_bControlLines && (m_eState > ESDragging || (RectangleDefined() && m_eState == ESDragging && m_bLinesWhenDefining)) ? (m_eMode&ERMRotated ? 9 : 8) : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (a_nIndex < 8)
		{
			if (!m_bControlLines || (!RectangleDefined() && m_eState <= ESDragging))
				return E_RW_INDEXOUTOFRANGE;
			if (a_nIndex&1)
			{
				a_pPos->fX = 0.5f*(m_aPixels[a_nIndex>>1].fX+m_aPixels[((a_nIndex>>1)+1)&3].fX);
				a_pPos->fY = 0.5f*(m_aPixels[a_nIndex>>1].fY+m_aPixels[((a_nIndex>>1)+1)&3].fY);
				*a_pClass = 1;
			}
			else
			{
				a_pPos->fX = m_aPixels[a_nIndex>>1].fX;
				a_pPos->fY = m_aPixels[a_nIndex>>1].fY;
				*a_pClass = 0;
			}
			return S_OK;
		}
		if (a_nIndex > 8 || !m_bControlLines || (!RectangleDefined() && m_eState <= ESDragging) || (m_eMode != ERMRotatedSquare && m_eMode != ERMRotatedRectangle && m_eMode != ERMRotatedLockAspectRectangle))
			return E_RW_INDEXOUTOFRANGE;
		a_pPos->fX = m_tCenter.fX + 0.35f*m_fSizeX*cosf(m_fAngle);
		a_pPos->fY = m_tCenter.fY + 0.35f*m_fSizeX*sinf(m_fAngle);
		*a_pClass = 2;
		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (a_nIndex >= 9)
			return E_RW_INDEXOUTOFRANGE;
		float const fX = a_pPos->fX-m_tCenter.fX;
		float const fY = a_pPos->fY-m_tCenter.fY;
		if (a_nIndex == 8)
		{
			if ((m_eMode&ERMRotated) == 0)
				return E_RW_INDEXOUTOFRANGE;
			float fAngle = atan2(fY, fX);
			if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
				fAngle = eStep*floorf(fAngle*180.0f/3.14159265f/eStep+0.5f)/180.0f*3.14159265f; // eStep degrees steps in integral mode
			if (fabsf(fAngle-m_fAngle) > 1e-4f)
			{
				m_fAngle = fAngle;
				PointsFromParams();
				for (ULONG i = 0; i < 9; ++i)
					static_cast<T*>(this)->M_Window()->ControlPointChanged(i);
				if (m_bControlLines)
					static_cast<T*>(this)->M_Window()->ControlLinesChanged();
				static_cast<T*>(this)->ShapeChanged(false);
			}
		}
		else
		{
			BYTE bChange = 0;
			// 0 1 2
			// 7   3
			// 6 5 4
			if (m_eMode == ERMQuadrangle)
			{
				if (a_nIndex&1)
				{
					ULONG nPt1 = a_nIndex>>1;
					ULONG nPt2 = ((a_nIndex+1)>>1)&3;
					float fPX = 0.5f*(m_aPixels[nPt1].fX+m_aPixels[nPt2].fX);
					float fPY = 0.5f*(m_aPixels[nPt1].fY+m_aPixels[nPt2].fY);
					m_aPixels[nPt1].fX += a_pPos->fX-fPX;
					m_aPixels[nPt2].fX += a_pPos->fX-fPX;
					m_aPixels[nPt1].fY += a_pPos->fY-fPY;
					m_aPixels[nPt2].fY += a_pPos->fY-fPY;
				}
				else
				{
					m_aPixels[a_nIndex>>1] = *a_pPos;
				}
				bChange = 0xff; // TODO: optimize
			}
			else
			{
				TPixelCoords tDirX = { cosf(m_fAngle),  sinf(m_fAngle)};
				TPixelCoords tDirY = {-sinf(m_fAngle),  cosf(m_fAngle)};
				if (a_nIndex&1)
				{
					//if (m_eMode == ERMCoaxialSquare || m_eMode == ERMRotatedSquare || m_eMode == ERMRotatedLockAspectRectangle)
					//{
					//	float const fDir = (float(a_nIndex&4)-2)*0.5f;
					//	float fCDX = a_pPos->fX-m_tCenter.fX;
					//	float fCDY = a_pPos->fY-m_tCenter.fY;
					//	if (a_nIndex&2)
					//	{
					//		// x
					//		float fPDX = -fCDX*tDirX.fX*fDir;
					//		float fPDY = -fCDY*tDirX.fY*fDir;
					//		if (fPDX >= 0 && fPDY >= 0)
					//		{
					//			float fD = sqrtf(fPDX*fPDX + fPDY*fPDY);
					//			m_tCenter.fX += tDirX.fX*fDir*(fD-m_fSizeX*0.5f);
					//			m_tCenter.fY += tDirX.fY*fDir*(fD-m_fSizeX*0.5f);
					//			m_fSizeX = 2*fD;
					//		}
					//	}
					//	else
					//	{
					//		// y
					//		float fPDX = fCDX*tDirY.fX*fDir;
					//		float fPDY = fCDY*tDirY.fY*fDir;
					//		if (fPDX >= 0 && fPDY >= 0)
					//		{
					//			float fD = sqrtf(fPDX*fPDX + fPDY*fPDY);
					//			m_tCenter.fX += tDirY.fX*fDir*(fD-m_fSizeY*0.5f);
					//			m_tCenter.fY += tDirY.fY*fDir*(fD-m_fSizeY*0.5f);
					//			m_fSizeY = 2*fD;
					//		}
					//	}
					//	PointsFromParams();
					////if (m_eMode == ERMCoaxialSquare || m_eMode == ERMRotatedSquare || m_eMode == ERMRotatedLockAspectRectangle)
					////{
					////	if (nPtR1&1)
					////	{
					////		m_fSizeX = m_fSizeY/m_fAspectLock;
					////		if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
					////		{
					////			m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock+0.5f);
					////			m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock+0.5f);
					////			m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock+0.5f);
					////			m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock+0.5f);
					////		}
					////		else
					////		{
					////			m_aPixels[nPtR1].fX = m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock;
					////			m_aPixels[nPtR1].fY = m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock;
					////			m_aPixels[nPtR2].fX = m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock;
					////			m_aPixels[nPtR2].fY = m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock;
					////		}
					////	}
					////	else
					////	{
					////		m_fSizeY = m_fSizeX*m_fAspectLock;
					////		if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
					////		{
					////			m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY*m_fAspectLock+0.5f);
					////			m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX*m_fAspectLock+0.5f);
					////			m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY*m_fAspectLock+0.5f);
					////			m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX*m_fAspectLock+0.5f);
					////		}
					////		else
					////		{
					////			m_aPixels[nPtR1].fX = m_aPixels[nPtL1].fX - tD.fY*m_fAspectLock;
					////			m_aPixels[nPtR1].fY = m_aPixels[nPtL1].fY + tD.fX*m_fAspectLock;
					////			m_aPixels[nPtR2].fX = m_aPixels[nPtL2].fX - tD.fY*m_fAspectLock;
					////			m_aPixels[nPtR2].fY = m_aPixels[nPtL2].fY + tD.fX*m_fAspectLock;
					////		}
					////	}
					////	m_tCenter.fX = 0.25f*(m_aPixels[0].fX + m_aPixels[1].fX + m_aPixels[2].fX + m_aPixels[3].fX);
					////	m_tCenter.fY = 0.25f*(m_aPixels[0].fY + m_aPixels[1].fY + m_aPixels[2].fY + m_aPixels[3].fY);
					////}
					//}
					//else
					//{
					//	ULONG nPtL1 = ((a_nIndex-1)>>1)&3;
					//	ULONG nPtR1 = (nPtL1+1)&3;
					//	ULONG nPtL2 = (nPtL1-1)&3;
					//	ULONG nPtR2 = (nPtR1+1)&3;
					//	TPixelCoords tRef = {0.5f*(m_aPixels[nPtL2].fX+m_aPixels[nPtR2].fX), 0.5f*(m_aPixels[nPtL2].fY+m_aPixels[nPtR2].fY)};
					//	TPixelCoords tD = {a_pPos->fX-tRef.fX, a_pPos->fY-tRef.fY};
					//	if (nPtR1&1)
					//	{
					//		float fD = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
					//		tD.fX = tDirY.fX*fD;
					//		tD.fY = tDirY.fY*fD;
					//		m_fSizeY = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
					//	}
					//	else
					//	{
					//		float fD = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
					//		tD.fX = tDirX.fX*fD;
					//		tD.fY = tDirX.fY*fD;
					//		m_fSizeX = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
					//	}
					//	m_aPixels[nPtL1].fX = m_aPixels[nPtL2].fX + tD.fX;
					//	m_aPixels[nPtL1].fY = m_aPixels[nPtL2].fY + tD.fY;
					//	m_aPixels[nPtR1].fX = m_aPixels[nPtR2].fX + tD.fX;
					//	m_aPixels[nPtR1].fY = m_aPixels[nPtR2].fY + tD.fY;
					//	m_tCenter.fX = tRef.fX+tD.fX*0.5f;
					//	m_tCenter.fY = tRef.fY+tD.fY*0.5f;
					//}


					ULONG nPtL1 = ((a_nIndex-1)>>1)&3;
					ULONG nPtR1 = (nPtL1+1)&3;
					ULONG nPtL2 = (nPtL1-1)&3;
					ULONG nPtR2 = (nPtR1+1)&3;
					TPixelCoords tRef = {0.5f*(m_aPixels[nPtL2].fX+m_aPixels[nPtR2].fX), 0.5f*(m_aPixels[nPtL2].fY+m_aPixels[nPtR2].fY)};
					TPixelCoords tD = {a_pPos->fX-tRef.fX, a_pPos->fY-tRef.fY};
					if (nPtR1&1)
					{
						float fD = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
						tD.fX = tDirY.fX*fD;
						tD.fY = tDirY.fY*fD;
						m_fSizeY = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
					}
					else
					{
						float fD = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
						tD.fX = tDirX.fX*fD;
						tD.fY = tDirX.fY*fD;
						m_fSizeX = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
					}
					m_aPixels[nPtL1].fX = m_aPixels[nPtL2].fX + tD.fX;
					m_aPixels[nPtL1].fY = m_aPixels[nPtL2].fY + tD.fY;
					m_aPixels[nPtR1].fX = m_aPixels[nPtR2].fX + tD.fX;
					m_aPixels[nPtR1].fY = m_aPixels[nPtR2].fY + tD.fY;
					m_tCenter.fX = tRef.fX+tD.fX*0.5f;
					m_tCenter.fY = tRef.fY+tD.fY*0.5f;
					if (m_eMode == ERMRotatedLockAspectRectangle)
					{
						if (nPtR1&1)
						{
							m_fSizeX = m_fSizeY/m_fAspectLock;
							if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
							{
								m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock+0.5f);
								m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock+0.5f);
							}
							else
							{
								m_aPixels[nPtR1].fX = m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock;
								m_aPixels[nPtR1].fY = m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock;
								m_aPixels[nPtR2].fX = m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock;
								m_aPixels[nPtR2].fY = m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock;
							}
						}
						else
						{
							m_fSizeY = m_fSizeX*m_fAspectLock;
							if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
							{
								m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY*m_fAspectLock+0.5f);
								m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX*m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY*m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX*m_fAspectLock+0.5f);
							}
							else
							{
								m_aPixels[nPtR1].fX = m_aPixels[nPtL1].fX - tD.fY*m_fAspectLock;
								m_aPixels[nPtR1].fY = m_aPixels[nPtL1].fY + tD.fX*m_fAspectLock;
								m_aPixels[nPtR2].fX = m_aPixels[nPtL2].fX - tD.fY*m_fAspectLock;
								m_aPixels[nPtR2].fY = m_aPixels[nPtL2].fY + tD.fX*m_fAspectLock;
							}
						}
						m_tCenter.fX = 0.25f*(m_aPixels[0].fX + m_aPixels[1].fX + m_aPixels[2].fX + m_aPixels[3].fX);
						m_tCenter.fY = 0.25f*(m_aPixels[0].fY + m_aPixels[1].fY + m_aPixels[2].fY + m_aPixels[3].fY);
					}
					else if (m_eMode == ERMCoaxialSquare || m_eMode == ERMRotatedSquare)
					{
						if (nPtR1&1)
						{
							m_fSizeX = m_fSizeY/m_fAspectLock;
							if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
							{
								m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock+0.5f);
								m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock+0.5f);
							}
							else
							{
								//m_aPixels[nPtR1].fX = m_aPixels[nPtL1].fX - tD.fY/m_fAspectLock;
								//m_aPixels[nPtR1].fY = m_aPixels[nPtL1].fY + tD.fX/m_fAspectLock;
								//m_aPixels[nPtR2].fX = m_aPixels[nPtL2].fX - tD.fY/m_fAspectLock;
								//m_aPixels[nPtR2].fY = m_aPixels[nPtL2].fY + tD.fX/m_fAspectLock;
								TPixelCoords const tN = {-0.5f*tD.fY/m_fAspectLock, 0.5f*tD.fX/m_fAspectLock};
								TPixelCoords const tC1 = {0.5f*(m_aPixels[nPtR1].fX+m_aPixels[nPtL1].fX), 0.5f*(m_aPixels[nPtR1].fY+m_aPixels[nPtL1].fY)};
								TPixelCoords const tC2 = {0.5f*(m_aPixels[nPtR2].fX+m_aPixels[nPtL2].fX), 0.5f*(m_aPixels[nPtR2].fY+m_aPixels[nPtL2].fY)};
								m_aPixels[nPtR1].fX = tC1.fX+tN.fX;
								m_aPixels[nPtR1].fY = tC1.fY+tN.fY;
								m_aPixels[nPtR2].fX = tC2.fX+tN.fX;
								m_aPixels[nPtR2].fY = tC2.fY+tN.fY;
								m_aPixels[nPtL1].fX = tC1.fX-tN.fX;
								m_aPixels[nPtL1].fY = tC1.fY-tN.fY;
								m_aPixels[nPtL2].fX = tC2.fX-tN.fX;
								m_aPixels[nPtL2].fY = tC2.fY-tN.fY;
							}
						}
						else
						{
							m_fSizeY = m_fSizeX*m_fAspectLock;
							if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
							{
								m_aPixels[nPtR1].fX = floorf(m_aPixels[nPtL1].fX - tD.fY*m_fAspectLock+0.5f);
								m_aPixels[nPtR1].fY = floorf(m_aPixels[nPtL1].fY + tD.fX*m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fX = floorf(m_aPixels[nPtL2].fX - tD.fY*m_fAspectLock+0.5f);
								m_aPixels[nPtR2].fY = floorf(m_aPixels[nPtL2].fY + tD.fX*m_fAspectLock+0.5f);
							}
							else
							{
								TPixelCoords const tN = {-0.5f*tD.fY/m_fAspectLock, 0.5f*tD.fX/m_fAspectLock};
								TPixelCoords const tC1 = {0.5f*(m_aPixels[nPtR1].fX+m_aPixels[nPtL1].fX), 0.5f*(m_aPixels[nPtR1].fY+m_aPixels[nPtL1].fY)};
								TPixelCoords const tC2 = {0.5f*(m_aPixels[nPtR2].fX+m_aPixels[nPtL2].fX), 0.5f*(m_aPixels[nPtR2].fY+m_aPixels[nPtL2].fY)};
								m_aPixels[nPtR1].fX = tC1.fX+tN.fX;
								m_aPixels[nPtR1].fY = tC1.fY+tN.fY;
								m_aPixels[nPtR2].fX = tC2.fX+tN.fX;
								m_aPixels[nPtR2].fY = tC2.fY+tN.fY;
								m_aPixels[nPtL1].fX = tC1.fX-tN.fX;
								m_aPixels[nPtL1].fY = tC1.fY-tN.fY;
								m_aPixels[nPtL2].fX = tC2.fX-tN.fX;
								m_aPixels[nPtL2].fY = tC2.fY-tN.fY;
							}
						}
						m_tCenter.fX = 0.25f*(m_aPixels[0].fX + m_aPixels[1].fX + m_aPixels[2].fX + m_aPixels[3].fX);
						m_tCenter.fY = 0.25f*(m_aPixels[0].fY + m_aPixels[1].fY + m_aPixels[2].fY + m_aPixels[3].fY);
					}
				}
				else
				{
					ULONG nPt1 = a_nIndex>>1;
					ULONG nPt2 = (nPt1+2)&3;
					m_aPixels[nPt1] = *a_pPos;
					TPixelCoords tD = {m_aPixels[nPt2].fX-m_aPixels[nPt1].fX, m_aPixels[nPt2].fY-m_aPixels[nPt1].fY};
					float fDX = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
					float fDY = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
					if (m_eMode == ERMCoaxialSquare || m_eMode == ERMRotatedSquare || m_eMode == ERMRotatedLockAspectRectangle)
					{
						if (fabsf(fabsf(fDX)*m_fAspectLock-fabsf(fDY)) > 1e-4f)
						{
							float fSize = min(fabsf(fDX)*m_fAspectLock, fabsf(fDY));//sqrtf(tD.fX*tD.fX+tD.fY*tD.fY)/sqrtf(2.0f);
							if (static_cast<T*>(this)->M_CoordinatesMode() == ECMIntegral)
							{
								fDX = fDX < 0.0f ? floorf(-fSize/m_fAspectLock+0.5f) : floorf(fSize/m_fAspectLock+0.5f);
								fDY = fDY < 0.0f ? floorf(-fSize+0.5f) : floorf(fSize+0.5f);
							}
							else
							{
								fDX = fDX < 0.0f ? -fSize/m_fAspectLock : fSize/m_fAspectLock;
								fDY = fDY < 0.0f ? -fSize : fSize;
							}
							m_aPixels[nPt1].fX = m_aPixels[nPt2].fX - tDirX.fX*fDX - tDirY.fX*fDY;
							m_aPixels[nPt1].fY = m_aPixels[nPt2].fY - tDirX.fY*fDX - tDirY.fY*fDY;
						}
						//tD.fX = m_aPixels[nPt2].fX-m_aPixels[nPt1].fX;
						//tD.fY = m_aPixels[nPt2].fY-m_aPixels[nPt1].fY;
					}
					TPixelCoords tDX = {tDirX.fX*fDX, tDirX.fY*fDX};
					TPixelCoords tDY = {tDirY.fX*fDY, tDirY.fY*fDY};
					m_aPixels[nPt1^3].fX = m_aPixels[nPt2].fX - tDX.fX;
					m_aPixels[nPt1^3].fY = m_aPixels[nPt2].fY - tDX.fY;
					m_aPixels[nPt1^1].fX = m_aPixels[nPt2].fX - tDY.fX;
					m_aPixels[nPt1^1].fY = m_aPixels[nPt2].fY - tDY.fY;
					m_tCenter.fX = 0.5f*(m_aPixels[nPt1].fX + m_aPixels[nPt2].fX);
					m_tCenter.fY = 0.5f*(m_aPixels[nPt1].fY + m_aPixels[nPt2].fY);
					m_fSizeX = fabsf(fDX);
					m_fSizeY = fabsf(fDY);
				}
				// if the rectangle would have negative size, move it instead
				{
					float fSizeX = (m_aPixels[1].fX-m_aPixels[0].fX)*tDirX.fX + (m_aPixels[1].fY-m_aPixels[0].fY)*tDirX.fY;
					if (fSizeX < 0.0f)
					{
						ULONG n = ((a_nIndex-2)>>2)&1;
						m_aPixels[0^n] = m_aPixels[1^n];
						m_aPixels[3^n] = m_aPixels[2^n];
						m_tCenter.fX = 0.5f*(m_aPixels[1].fX + m_aPixels[2].fX);
						m_tCenter.fY = 0.5f*(m_aPixels[1].fY + m_aPixels[2].fY);
						m_fSizeX = 0.0f;
					}
					float fSizeY = (m_aPixels[3].fX-m_aPixels[0].fX)*tDirY.fX + (m_aPixels[3].fY-m_aPixels[0].fY)*tDirY.fY;
					if (fSizeY < 0.0f)
					{
						ULONG n = ((a_nIndex>>2)&1)*3;
						m_aPixels[3^n] = m_aPixels[0^n];
						m_aPixels[2^n] = m_aPixels[1^n];
						m_tCenter.fX = 0.5f*(m_aPixels[1].fX + m_aPixels[0].fX);
						m_tCenter.fY = 0.5f*(m_aPixels[1].fY + m_aPixels[0].fY);
						m_fSizeY = 0.0f;
					}
				}
				bChange = 0xff; // TODO: optimize
			}
			for (int i = 0; bChange; bChange>>=1, ++i)
				if (bChange&1)
					static_cast<T*>(this)->M_Window()->ControlPointChanged(i);
			if (m_eMode&ERMRotated)
				static_cast<T*>(this)->M_Window()->ControlPointChanged(8);
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			static_cast<T*>(this)->ShapeChanged(false);
			return S_OK;
		//	float fX2 = fX*cos(m_fAngle)+fY*sin(m_fAngle);
		//	float fY2 = fY*cos(m_fAngle)-fX*sin(m_fAngle);
		//	if (m_cData.eMode == CEditToolDataRectangle::EMSquare)
		//	{
		//		if (
		//		if (s_aWeights[a_nIndex].fX == 0.0f)
		//		{
		//			//m_tCenter.fY += s_aWeights[a_nIndex].fX*fD1*sinf(m_fAngle);
		//		}
		//		else if (s_aWeights[a_nIndex].fY == 0.0f)
		//		{
		//		}
		//		else
		//		{
		//			if (fX2 < 0.0f)
		//			{
		//				if (fY2 < 0.0f)
		//					fX2 = fY2 = 0.5f*(fX2+fY2);
		//				else
		//					fX2 = -(fY2 = 0.5f*(fY2-fX2));
		//			}
		//			else
		//			{
		//				if (fY2 < 0.0f)
		//					fY2 = -(fX2 = 0.5f*(fX2-fY2));
		//				else
		//					fX2 = fY2 = 0.5f*(fX2+fY2);
		//			}
		//			float const fD1 = 0.5f*(fX2/s_aWeights[a_nIndex].fX-m_fSizeX);
		//			m_tCenter.fX += s_aWeights[a_nIndex].fX*fD1*cosf(m_fAngle);
		//			m_tCenter.fY += s_aWeights[a_nIndex].fX*fD1*sinf(m_fAngle);
		//			m_fSizeX = fabsf(0.5f*(m_fSizeX+fX2/s_aWeights[a_nIndex].fX));
		//			float const fD2 = 0.5f*(fY2/s_aWeights[a_nIndex].fY-m_fSizeY);
		//			m_tCenter.fX -= s_aWeights[a_nIndex].fY*fD2*sinf(m_fAngle);
		//			m_tCenter.fY += s_aWeights[a_nIndex].fY*fD2*cosf(m_fAngle);
		//			m_fSizeY = fabsf(0.5f*(m_fSizeY+fY2/s_aWeights[a_nIndex].fY));
		//		}
		//	}
		//	else
		//	{
		//		if (s_aWeights[a_nIndex].fX != 0.0f)
		//		{
		//			float const fD = 0.5f*(fX2/s_aWeights[a_nIndex].fX-m_fSizeX);
		//			m_tCenter.fX += s_aWeights[a_nIndex].fX*fD*cosf(m_fAngle);
		//			m_tCenter.fY += s_aWeights[a_nIndex].fX*fD*sinf(m_fAngle);
		//			m_fSizeX = fabsf(0.5f*(m_fSizeX+fX2/s_aWeights[a_nIndex].fX));
		//		}
		//		if (s_aWeights[a_nIndex].fY != 0.0f)
		//		{
		//			float const fD = 0.5f*(fY2/s_aWeights[a_nIndex].fY-m_fSizeY);
		//			m_tCenter.fX -= s_aWeights[a_nIndex].fY*fD*sinf(m_fAngle);
		//			m_tCenter.fY += s_aWeights[a_nIndex].fY*fD*cosf(m_fAngle);
		//			m_fSizeY = fabsf(0.5f*(m_fSizeY+fY2/s_aWeights[a_nIndex].fY));
		//		}
		//	}
		//}
		//if (m_eCoordinatesMode == ECMIntegral)
		//{
		//	// keep size and center on boundaries of integer values
		//	int nCenterX = m_tCenter.fX*2.0f+0.5f;
		//	int nSizeX = m_fSizeX*2.0f+0.5f;
		//	if (nCenterX&1)
		//		nSizeX |= 1;
		//	else
		//		nSizeX &= ~1;
		//	m_tCenter.fX = nCenterX*0.5f;
		//	m_fSizeX = nSizeX*0.5f;

		//	int nCenterY = m_tCenter.fY*2.0f+0.5f;
		//	int nSizeY = m_fSizeY*2.0f+0.5f;
		//	if (nCenterY&1)
		//		nSizeY |= 1;
		//	else
		//		nSizeY &= ~1;
		//	m_tCenter.fY = nCenterY*0.5f;
		//	m_fSizeY = nSizeY*0.5f;
		}
		//for (ULONG i = 0; i < 9; ++i) M_Window()->ControlPointChanged(i);
		//if (M_Brush())
		//	M_Brush()->SetShapeBounds(&m_tCenter, m_fSizeX, m_fSizeY, m_fAngle);
		//RECT rcPrev = M_DirtyRect();
		//PrepareShape();
		//RECT rcNew = M_DirtyRect();
		//if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
		//if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
		//if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
		//if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
		//M_Window()->RectangleChanged(&rcNew);
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetSelectionControlLines(IEditToolControlLines* a_pLines)
	{
		return S_FALSE;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (!m_bControlLines || !(m_eState > ESDragging || (RectangleDefined() && m_eState == ESDragging && m_bLinesWhenDefining)))
			return S_FALSE;
		if (a_nLineTypes&ECLTHelp)
		{
			float fLineLenX = 1.0f;
			a_pLines->HandleSize(&fLineLenX);
			fLineLenX *= 4.0f;
			float fLineLenY = fLineLenX;
			if (m_fSizeX < fLineLenX) fLineLenX = m_fSizeX;
			if (m_fSizeY < fLineLenY) fLineLenY = m_fSizeY;
			float const fCos = cosf(m_fAngle);
			float const fSin = sinf(m_fAngle);
			if (m_bCornersOnly)
			{
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[0].fX+fLineLenX*fCos, m_aPixels[0].fY+fLineLenX*fSin);
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[0].fX-fLineLenY*fSin, m_aPixels[0].fY+fLineLenY*fCos);
				
				a_pLines->MoveTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->LineTo(m_aPixels[1].fX-fLineLenX*fCos, m_aPixels[1].fY-fLineLenX*fSin);
				a_pLines->MoveTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->LineTo(m_aPixels[1].fX-fLineLenY*fSin, m_aPixels[1].fY+fLineLenY*fCos);

				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[2].fX-fLineLenX*fCos, m_aPixels[2].fY-fLineLenX*fSin);
				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[2].fX+fLineLenY*fSin, m_aPixels[2].fY-fLineLenY*fCos);

				a_pLines->MoveTo(m_aPixels[3].fX, m_aPixels[3].fY);
				a_pLines->LineTo(m_aPixels[3].fX+fLineLenX*fCos, m_aPixels[3].fY+fLineLenX*fSin);
				a_pLines->MoveTo(m_aPixels[3].fX, m_aPixels[3].fY);
				a_pLines->LineTo(m_aPixels[3].fX+fLineLenY*fSin, m_aPixels[3].fY-fLineLenY*fCos);

				if (m_eMode&ERMRotated)
				{
					TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle), m_fSizeX*sinf(m_fAngle)};
					a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY);
					a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					for (int i = 0; i < 5; ++i)
					{
						TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle+i*0.04f), m_fSizeX*sinf(m_fAngle+i*0.04f)};
						a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					}
					a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					for (int i = 0; i < 5; ++i)
					{
						TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle-i*0.04f), m_fSizeX*sinf(m_fAngle-i*0.04f)};
						a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					}
				}
			}
			else
			{
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[3].fX, m_aPixels[3].fY);
				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[3].fX, m_aPixels[3].fY);
				if (m_eMode&ERMRotated)
				{
					TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle), m_fSizeX*sinf(m_fAngle)};
					a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY);
					a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					for (int i = 0; i < 5; ++i)
 					{
						TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle+i*0.04f), m_fSizeX*sinf(m_fAngle+i*0.04f)};
 						a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					}
					a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					for (int i = 0; i < 5; ++i)
					{
						TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle-i*0.04f), m_fSizeX*sinf(m_fAngle-i*0.04f)};
						a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
					}
				}
			}
		}
		return (a_nLineTypes&ECLTSelection) ? static_cast<T*>(this)->_GetSelectionControlLines(a_pLines) : S_OK;
	}

	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (!RectangleDefined())
			return S_FALSE;

		if (a_pMsg->message != WM_KEYDOWN ||
			(a_pMsg->wParam != VK_LEFT && a_pMsg->wParam != VK_RIGHT &&
			 a_pMsg->wParam != VK_UP && a_pMsg->wParam != VK_DOWN))
			 return S_FALSE;
		LONG nDX = 0;
		LONG nDY = 0;
		if (a_pMsg->wParam == VK_LEFT)
			nDX = -1;
		else if (a_pMsg->wParam == VK_RIGHT)
			nDX = 1;
		else if (a_pMsg->wParam == VK_UP)
			nDY = -1;
		else if (a_pMsg->wParam == VK_DOWN)
			nDY = 1;
		if (m_eMode == ERMQuadrangle)
		{
			SetRectanglePoints(m_aPixels[0].fX+nDX, m_aPixels[0].fY+nDY, m_aPixels[1].fX+nDX, m_aPixels[1].fY+nDY, m_aPixels[2].fX+nDX, m_aPixels[2].fY+nDY, m_aPixels[3].fX+nDX, m_aPixels[3].fY+nDY);
		}
		else if (0x8000&GetAsyncKeyState(VK_SHIFT))
		{
			if ((m_eMode&ERMNonSquare) == 0)
			{
				if (nDX != 0)
					nDY = nDX*m_fAspectLock+0.5f;
				else
					nDX = nDY/m_fAspectLock+0.5f;
			}
			float const fCos = cosf(m_fAngle);
			float const fSin = sinf(m_fAngle);
			SetRectangle(m_tCenter.fX+nDX*0.5f*fCos-nDY*0.5f*fSin, m_tCenter.fY+nDY*0.5f*fCos+nDX*0.5f*fSin, (m_fSizeX+nDX)*0.5f, (m_fSizeY+nDY)*0.5f, m_fAngle);
		}
		else
		{
			SetRectangle(m_tCenter.fX+nDX, m_tCenter.fY+nDY, m_fSizeX*0.5f, m_fSizeY*0.5f, m_fAngle);
		}
		return S_OK;
	}
	TPixelCoords TransformPixelCoords(TMatrix3x3f const& a_tMatrix, TPixelCoords const& a_tVector)
	{
		float const fW = 1.0f/(a_tMatrix._13*a_tVector.fX + a_tMatrix._23*a_tVector.fY + a_tMatrix._33);
		TPixelCoords const t =
		{
			fW*(a_tMatrix._11*a_tVector.fX + a_tMatrix._21*a_tVector.fY + a_tMatrix._31),
			fW*(a_tMatrix._12*a_tVector.fX + a_tMatrix._22*a_tVector.fY + a_tMatrix._32),
		};
		return t;
	}
	void TransformRectangle(TMatrix3x3f const* a_pMatrix)
	{
		float const fCos = cosf(m_fAngle);
		float const fSin = sinf(m_fAngle);
		TPixelCoords tCenter = TransformPixelCoords(*a_pMatrix, m_tCenter);
		TPixelCoords tX = {m_tCenter.fX+m_fSizeX*fCos, m_tCenter.fY+m_fSizeX*fSin};
		tX = TransformPixelCoords(*a_pMatrix, tX);
		float fSizeX = sqrtf((tCenter.fX-tX.fX)*(tCenter.fX-tX.fX)+(tCenter.fY-tX.fY)*(tCenter.fY-tX.fY));
		float fAX = atan2f(tX.fY-tCenter.fY, tX.fX-tCenter.fX);
		TPixelCoords tY = {m_tCenter.fX-m_fSizeY*fSin, m_tCenter.fY+m_fSizeY*fCos};
		tY = TransformPixelCoords(*a_pMatrix, tY);
		float fSizeY = sqrtf((tCenter.fX-tY.fX)*(tCenter.fX-tY.fX)+(tCenter.fY-tY.fY)*(tCenter.fY-tY.fY));
		float fAY = atan2f(tY.fX-tCenter.fX, tY.fY-tCenter.fY);
		m_tCenter = tCenter;
		m_fSizeX = fSizeX;
		m_fSizeY = fSizeY;
		m_fAngle = fSizeX >= fSizeY ? fAX : -fAY/*-1.57079632679489661923f*/;
		PointsFromParams();
		m_eState = ESFloating;
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		static_cast<T*>(this)->ShapeChanged(false);
	}

private:
	void PointsFromParams()
	{
		float const fCos = cosf(m_fAngle);
		float const fSin = sinf(m_fAngle);
		float const fXCos = m_fSizeX*0.5f*fCos;
		float const fXSin = m_fSizeX*0.5f*fSin;
		float const fYCos = m_fSizeY*0.5f*fCos;
		float const fYSin = m_fSizeY*0.5f*fSin;
		m_aPixels[0].fX = m_tCenter.fX - fXCos + fYSin;
		m_aPixels[0].fY = m_tCenter.fY - fXSin - fYCos;
		m_aPixels[1].fX = m_tCenter.fX + fXCos + fYSin;
		m_aPixels[1].fY = m_tCenter.fY + fXSin - fYCos;
		m_aPixels[2].fX = m_tCenter.fX + fXCos - fYSin;
		m_aPixels[2].fY = m_tCenter.fY + fXSin + fYCos;
		m_aPixels[3].fX = m_tCenter.fX - fXCos - fYSin;
		m_aPixels[3].fY = m_tCenter.fY - fXSin + fYCos;
	}

private:
	TPixelCoords m_tDragStart;
	EState m_eState;
	TPixelCoords m_aPixels[4];
	TPixelCoords m_tCenter;
	float m_fSizeX;
	float m_fSizeY;
	float m_fAngle;
	bool m_bControlLines;
	ERectangleMode m_eMode;
	float m_fAspectLock;
	bool m_bLinesWhenDefining;
	bool const m_bCornersOnly;
};
