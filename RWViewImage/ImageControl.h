#pragma once

#include "RWViewImage.h"

template<typename T, typename TBase = CWindow, typename TWinTraits = CControlWinTraits>
class CImageControl :
	public CWindowImpl<T, TBase, TWinTraits>
{
public:
	CImageControl() : m_fZoom(1.0f), m_pData(NULL), m_nWheelDelta(0), m_bControlling(false),
		m_fOffsetX(0.0f), m_fOffsetY(0.0f), m_bAutoZoom(false), m_bUpdating(false),
		m_fControlledOffsetX(0.0f), m_fControlledOffsetY(0.0f), m_fControlledZoom(0.0f),
		m_nControlledSizeX(0), m_nControlledSizeY(0), m_bFrame(false)
	{
		::InitializeCriticalSection(&m_tImageData);
		m_tZoomedSize.cx = 1;
		m_tZoomedSize.cy = 1;
	}
	~CImageControl()
	{
		::DeleteCriticalSection(&m_tImageData);
		delete[] m_pData;
	}

BEGIN_MSG_MAP(CImageControl)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
	MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClick)
	MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
END_MSG_MAP()

	void Init(IDocumentImage* a_pDI, bool a_bFrame, COLORREF a_tBackground)
	{
		CCritSecLock cLock(m_tImageData);
		m_pDI = a_pDI;
		m_fZoom = 1.0f;
		m_nWheelDelta = 0;
		m_fOffsetX = 0.0f;
		m_fOffsetY = 0.0f;
		
		m_tBackground = a_tBackground;
		m_bFrame = a_bFrame;
		delete[] m_pData;
		m_pData = NULL;
		TImageSize tSize;
		a_pDI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		m_tOrigSize.cx = tSize.nX;
		m_tOrigSize.cy = tSize.nY;

		UpdateState();
	}
	void Init(IDocumentImage* a_pDI, bool a_bFrame,  COLORREF a_tBackground, bool a_bAutoZoom)
	{
		m_bAutoZoom = a_bAutoZoom | m_bControlling;
		Init(a_pDI, a_bFrame, a_tBackground);
	}
	void RefreshImage(IDocumentImage* a_pDI, bool a_bFrame, COLORREF a_tBackground)
	{
		CCritSecLock cLock(m_tImageData);
		m_pDI = a_pDI;
		m_tBackground = a_tBackground;
		m_bFrame = a_bFrame;
		Invalidate(FALSE);
	}
	void SetViewport(float a_fOffsetX, float a_fOffsetY, float a_fZoom)
	{
		if (m_bControlling)
			return;
		if (a_fZoom != 0.0f)
			m_fZoom = a_fZoom;
		m_fOffsetX = a_fOffsetX;
		m_fOffsetY = a_fOffsetY;
		UpdateState();
		if (m_hWnd)
			UpdateWindow();
	}
	void EnableControlledMode()
	{
		m_bControlling = true;
		m_bAutoZoom = true;
		UpdateState();
	}
	void SetControlledViewport(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY)
	{
		m_fControlledOffsetX = a_fOffsetX;
		m_fControlledOffsetY = a_fOffsetY;
		m_fControlledZoom = a_fZoom;
		m_nControlledSizeX = a_nSizeX;
		m_nControlledSizeY = a_nSizeY;
		if (m_hWnd)
		{
			Invalidate(FALSE);
			UpdateWindow();
		}
	}

	static void InitializeImageData(COLORREF a_tBackground, SIZE a_tOrigSize, BYTE* a_pData, IDocumentImage* a_pDI)
	{
		TImageSize tSize;
		a_pDI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		if (tSize.nX != a_tOrigSize.cx || tSize.nY != a_tOrigSize.cy)
		{
			CNearestImageResizer::GetResizedImage(a_pDI, EICIRGBA, a_tOrigSize.cx, a_tOrigSize.cy, 1, a_tOrigSize.cx, reinterpret_cast<TPixelChannel*>(a_pData));
		}
		else
		{
			a_pDI->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(a_pData), NULL, EIRIPreview);
		}
		BYTE* p = a_pData;
		BYTE const bR = GetRValue(a_tBackground);
		BYTE const bG = GetGValue(a_tBackground);
		BYTE const bB = GetBValue(a_tBackground);
		for (LONG nY = 0; nY < a_tOrigSize.cy; nY++)
		{
			for (LONG nX = 0; nX < a_tOrigSize.cx; nX++)
			{
				ULONG const nA = p[3];
				ULONG const nIA = 256-nA;
				p[0] = (p[0]*nA + bB*nIA)>>8;
				p[1] = (p[1]*nA + bG*nIA)>>8;
				p[2] = (p[2]*nA + bR*nIA)>>8;
				p += 4;
			}
		}
	}

	// handlers
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bControlling)
		{
		}
		else if (!GetParent().SendMessage(a_uMsg, a_wParam, a_lParam))
		{
			if (reinterpret_cast<HWND>(a_wParam) == m_hWnd && LOWORD(a_lParam) == HTCLIENT)
			{
				SetCursor(LoadCursor(NULL,
					m_tZoomedSize.cx > m_tClientSize.cx ?
					(m_tZoomedSize.cy > m_tClientSize.cy ? IDC_SIZEALL: IDC_SIZEWE) :
					(m_tZoomedSize.cy > m_tClientSize.cy ? IDC_SIZENS : IDC_ARROW)));
				return TRUE;
			}
		}
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int nX = 0;
		if (m_tZoomedSize.cx > m_tClientSize.cx)
		{
			float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
			nX = static_cast<int>((m_fOffsetX+fOffsetXBound)*m_tZoomedSize.cx+0.5f);
		}
		int nY = 0;
		if (m_tZoomedSize.cy > m_tClientSize.cy)
		{
			float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
			nY = static_cast<int>((m_fOffsetY+fOffsetYBound)*m_tZoomedSize.cy+0.5f);
		}

		if (a_wParam != NULL)
		{
			CDCHandle dc = (HDC)a_wParam;
			dc.SetViewportOrg(-nX, -nY);
			DoPaint(dc);
		}
		else
		{
			CPaintDC dc(m_hWnd);
			dc.SetViewportOrg(-nX, -nY);
			DoPaint(dc.m_hDC);
		}
		{
			CCritSecLock cLock(m_tImageData);
			if (m_pDI != NULL)
				Invalidate();
		}
		return 0;
	}

	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		UpdateState();
		if (!m_bControlling)
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));

		return 0;
	}

	LRESULT OnScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bControlling) // should not be necessary
			return 0;

		LONG& nZoomedSize = a_uMsg == WM_HSCROLL ? m_tZoomedSize.cx : m_tZoomedSize.cy;
		LONG& nClientSize = a_uMsg == WM_HSCROLL ? m_tClientSize.cx : m_tClientSize.cy;
		float& fOffset = a_uMsg == WM_HSCROLL ? m_fOffsetX : m_fOffsetY;

		if (nZoomedSize <= nClientSize)
			return 0;

		switch (LOWORD(a_wParam))
		{
		case SB_TOP:		// top or all left
			fOffset = -1.0f;
			break;
		case SB_BOTTOM:		// bottom or all right
			fOffset = 1.0f;
			break;
		case SB_LINEUP:		// line up or line left
			fOffset -= 16.0f/nZoomedSize;
			break;
		case SB_LINEDOWN:	// line down or line right
			fOffset += 16.0f/nZoomedSize;
			break;
		case SB_PAGEUP:		// page up or page left
			fOffset -= static_cast<float>(nClientSize)/nZoomedSize;
			break;
		case SB_PAGEDOWN:	// page down or page right
			fOffset += static_cast<float>(nClientSize)/nZoomedSize;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			{
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				if (GetScrollInfo(a_uMsg == WM_HSCROLL ? SB_HORZ : SB_VERT, &si))
				{
					float fOffsetBound = static_cast<float>(nZoomedSize-nClientSize)/static_cast<float>(nZoomedSize);
					fOffset = (static_cast<float>(si.nTrackPos)/(nZoomedSize-nClientSize)-0.5f)*fOffsetBound;
				}
			}
			break;
		case SB_ENDSCROLL:
		default:
			return 0;
		}

		UpdatePosition();
		static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
		return 0;
	}

	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		UpdateState();

		return 0;
	}

	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bControlling)
			return 1;

		m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);

		// preserve point under mouse during zoom
		POINT tPos = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		ScreenToClient(&tPos);

		float fX = 0;
		if (m_tZoomedSize.cx > m_tClientSize.cx)
		{
			float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
			fX = (m_fOffsetX+fOffsetXBound)*m_tZoomedSize.cx;
		}
		else
		{
			fX = -0.5f*(m_tClientSize.cx-m_tZoomedSize.cx);
		}
		float fY = 0;
		if (m_tZoomedSize.cy > m_tClientSize.cy)
		{
			float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
			fY = (m_fOffsetY+fOffsetYBound)*m_tZoomedSize.cy;
		}
		else
		{
			fY = -0.5f*(m_tClientSize.cy-m_tZoomedSize.cy);
		}
		float fPixelX = (tPos.x + fX - m_tZoomedSize.cx*0.5f) / m_fZoom;
		float fPixelY = (tPos.y + fY - m_tZoomedSize.cy*0.5f) / m_fZoom;

		bool bModified = false;
		while (m_nWheelDelta >= WHEEL_DELTA)
		{
			m_fZoom *= 1.41421356f; // sqrf(2.0f);
			m_nWheelDelta -= WHEEL_DELTA;
			bModified = true;
		}
		while ((-m_nWheelDelta) >= WHEEL_DELTA)
		{
			m_fZoom *= 0.7071067812f; // 1.0f/sqrf(2.0f);
			m_nWheelDelta += WHEEL_DELTA;
			bModified = true;
		}

		if (bModified)
		{
			SetAutoZoom(false, true);
			UpdateState();
			float fX = fPixelX*m_fZoom - tPos.x + m_tZoomedSize.cx*0.5f;
			float fY = fPixelY*m_fZoom - tPos.y + m_tZoomedSize.cy*0.5f;
			if (m_tZoomedSize.cx > m_tClientSize.cx)
			{
				float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
				m_fOffsetX = fX/m_tZoomedSize.cx-fOffsetXBound;
			}
			if (m_tZoomedSize.cy > m_tClientSize.cy)
			{
				float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
				m_fOffsetY = fY/m_tZoomedSize.cy-fOffsetYBound;
			}
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
		}

		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			if (m_bControlling)
			{
				POINT tCenter = {m_tClientSize.cx>>1, m_tClientSize.cy>>1};
				m_fControlledOffsetX = (GET_X_LPARAM(a_lParam)-tCenter.x)/static_cast<float>(m_tZoomedSize.cx);
				m_fControlledOffsetY = (GET_Y_LPARAM(a_lParam)-tCenter.y)/static_cast<float>(m_tZoomedSize.cy);
				if (m_fControlledOffsetX > 1.0f) m_fControlledOffsetX = 1.0f;
				else if (m_fControlledOffsetX < -1.0f) m_fControlledOffsetX = -1.0f;
				if (m_fControlledOffsetY > 1.0f) m_fControlledOffsetY = 1.0f;
				else if (m_fControlledOffsetY < -1.0f) m_fControlledOffsetY = -1.0f;
				static_cast<T*>(this)->ControlledVieportChanged(m_fControlledOffsetX, m_fControlledOffsetY, m_fControlledZoom, m_nControlledSizeX, m_nControlledSizeY);
			}
			else
			{
				if (m_tZoomedSize.cx > m_tClientSize.cx)
				{
					float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
					m_fOffsetX = m_fMouseDownX - GET_X_LPARAM(a_lParam)/static_cast<float>(m_tZoomedSize.cx);
				}
				if (m_tZoomedSize.cy > m_tClientSize.cy)
				{
					float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
					m_fOffsetY = m_fMouseDownY - GET_Y_LPARAM(a_lParam)/static_cast<float>(m_tZoomedSize.cy);
				}
				UpdatePosition();
				static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
				UpdateWindow();
			}
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCapture();
		POINT pt;

		if (m_bControlling)
		{
			POINT tCenter = {m_tClientSize.cx>>1, m_tClientSize.cy>>1};
			m_fControlledOffsetX = (GET_X_LPARAM(a_lParam)-tCenter.x)/static_cast<float>(m_tZoomedSize.cx);
			m_fControlledOffsetY = (GET_Y_LPARAM(a_lParam)-tCenter.y)/static_cast<float>(m_tZoomedSize.cy);
			static_cast<T*>(this)->ControlledVieportChanged(m_fControlledOffsetX, m_fControlledOffsetY, m_fControlledZoom, m_nControlledSizeX, m_nControlledSizeY);
		}
		else
		{
			if (m_tZoomedSize.cx > m_tClientSize.cx)
			{
				float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
				m_fMouseDownX = m_fOffsetX + GET_X_LPARAM(a_lParam)/static_cast<float>(m_tZoomedSize.cx);
			}
			if (m_tZoomedSize.cy > m_tClientSize.cy)
			{
				float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
				m_fMouseDownY = m_fOffsetY + GET_Y_LPARAM(a_lParam)/static_cast<float>(m_tZoomedSize.cy);
			}
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		ReleaseCapture();
		return 0;
	}

	LRESULT OnLButtonDblClick(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bControlling)
			SetAutoZoom(true);
		return 0;
	}

	LRESULT OnKeyDown(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		if (m_bControlling)
			return 1;

		switch (a_wParam)
		{
		case VK_DOWN:
			m_fOffsetY += 16.0f/m_tZoomedSize.cy;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_UP:
			m_fOffsetY -= 16.0f/m_tZoomedSize.cy;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_LEFT:
			m_fOffsetX -= 16.0f/m_tZoomedSize.cx;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_RIGHT:
			m_fOffsetX += 16.0f/m_tZoomedSize.cx;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_HOME:
			m_fOffsetX = -1.0f;
			m_fOffsetY = -1.0f;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_END:
			m_fOffsetX = 1.0f;
			m_fOffsetY = 1.0f;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_PRIOR:
			m_fOffsetY -= static_cast<float>(m_tClientSize.cy)/m_tZoomedSize.cy;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_NEXT: // 100% zoom
			m_fOffsetY += static_cast<float>(m_tClientSize.cy)/m_tZoomedSize.cy;
			UpdatePosition();
			static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
			break;
		case VK_MULTIPLY: // fit page
			SetAutoZoom(true);
			break;
		case VK_DIVIDE:
			SetZoom(1.0f);
			break;
		case VK_ADD:
			SetZoom(m_fZoom * 1.41421356f); // sqrf(2.0f);
			break;
		case VK_SUBTRACT:
			SetZoom(m_fZoom * 0.7071067812f); // 1.0f/sqrf(2.0f);
			break;
		default:
			return 1;
		}
		return 0;
	}

	void ZoomFactorChanged(float) {}
	void AutoZoomChanged(bool) {}

	float M_Zoom() const { return m_fZoom; }
	bool M_AutoZoom() const { return m_bAutoZoom; }
	void SetZoom(float a_fZoom)
	{
		SetAutoZoom(false, true);
		m_fZoom = a_fZoom;
		UpdateState();
		static_cast<T*>(this)->VieportChanged(m_fOffsetX, m_fOffsetY, m_fZoom, min(m_tClientSize.cx, m_tOrigSize.cx), min(m_tClientSize.cy, m_tOrigSize.cy));
	}
	void SetAutoZoom(bool a_bEnable, bool a_bNoInternalUpdate = false)
	{
		if (m_bAutoZoom != a_bEnable)
		{
			static_cast<T*>(this)->AutoZoomChanged(m_bAutoZoom = a_bEnable);
			if (!a_bNoInternalUpdate)
				UpdateState();
		}
	}
	SIZE M_OrigSize() const { return m_tOrigSize; }

private:
	void DoPaint(CDCHandle a_cDC)
	{
		CComPtr<IDocumentImage> pDI;
		SIZE tSize;
		SIZE tZoomedSize;
		BYTE* pData = NULL;
		{
			CCritSecLock cLock(m_tImageData);
			tSize = m_tOrigSize;
			tZoomedSize = m_tZoomedSize;
			pData = m_pData;
			if (m_pDI != NULL)
			{
				pDI.Attach(m_pDI.Detach());
			}
			m_pData = NULL;
		}

		RECT rcClient = {0, 0, m_tClientSize.cx, m_tClientSize.cy};
		RECT rc = {0, 0, tZoomedSize.cx, tZoomedSize.cy};
		if (rc.right < rcClient.right)
		{
			rc.left = (rcClient.right - rc.right)>>1;
			rc.right += rc.left;
			RECT rcFill = rcClient;
			rcFill.bottom = rc.bottom > rcClient.bottom ? rc.bottom : rcClient.bottom;
			rcFill.right = rc.left;
			a_cDC.FillSolidRect(&rcFill, m_tBackground);
			rcFill.left = rc.right;
			rcFill.right = rcClient.right;
			a_cDC.FillSolidRect(&rcFill, m_tBackground);
		}
		if (rc.bottom < rcClient.bottom)
		{
			rc.top = (rcClient.bottom - rc.bottom)>>1;
			rc.bottom += rc.top;
			RECT rcFill = rc;
			rcFill.top = rcClient.top;
			rcFill.bottom = rc.top;
			a_cDC.FillSolidRect(&rcFill, m_tBackground);
			rcFill.top = rc.bottom;
			rcFill.bottom = rcClient.bottom;
			a_cDC.FillSolidRect(&rcFill, m_tBackground);
		}
		if (m_bFrame)
		{
			COLORREF clrPen = (
				0.299f * powf(GetRValue(m_tBackground)/255.0f, 2.2f)+
				0.587f * powf(GetGValue(m_tBackground)/255.0f, 2.2f)+
				0.114f * powf(GetBValue(m_tBackground)/255.0f, 2.2f)) > 0.5f ? 0 : 0xffffff;
			CPen cPen;
			LOGBRUSH tLB;
			tLB.lbColor = clrPen;
			tLB.lbStyle = PS_SOLID;
			tLB.lbHatch = NULL;
			cPen.CreatePen(PS_COSMETIC | PS_DOT, 1, &tLB, 0, NULL);
			//cPen.CreatePen(PS_DOT, 1, clrPen);
			HPEN hPen = a_cDC.SelectPen(cPen);
			a_cDC.MoveTo(rc.left-1, rc.top-1);
			a_cDC.LineTo(rc.right, rc.top-1);
			a_cDC.LineTo(rc.right, rc.bottom);
			a_cDC.LineTo(rc.left-1, rc.bottom);
			a_cDC.LineTo(rc.left-1, rc.top-1);
			a_cDC.SelectPen(hPen);
		}

		if (m_tOrigSize.cx*m_tOrigSize.cy == 0)
		{
			a_cDC.FillSolidRect(&rc, m_tBackground);
		}
		else
		{
			if (pData == NULL && m_tOrigSize.cx*m_tOrigSize.cy)
				try { pData = new BYTE[m_tOrigSize.cx*m_tOrigSize.cy<<2]; } catch (...) {}
			if (pData == NULL)
				return;

			if (pDI != NULL)
				InitializeImageData(m_tBackground, tSize, pData, pDI);

			BITMAPINFO tBMI;
			ZeroMemory(&tBMI, sizeof(tBMI));
			tBMI.bmiHeader.biSize = sizeof(tBMI.bmiHeader);
			tBMI.bmiHeader.biWidth = tSize.cx;
			tBMI.bmiHeader.biHeight = -tSize.cy;
			tBMI.bmiHeader.biPlanes = 1;
			tBMI.bmiHeader.biBitCount = 32;
			tBMI.bmiHeader.biCompression = BI_RGB;
			int iPrevStretchMode = a_cDC.SetStretchBltMode(/*m_fZoom > 1.0f ? HALFTONE : */COLORONCOLOR);
			//if (iPrevStretchMode == 0) iPrevStretchMode = a_cDC.SetStretchBltMode(COLORONCOLOR);
			int ret = StretchDIBits(a_cDC, rc.left, rc.top, tZoomedSize.cx, tZoomedSize.cy,
								0, 0, tSize.cx, tSize.cy,
								pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
			a_cDC.SetStretchBltMode(iPrevStretchMode);

			{
				CCritSecLock cLock(m_tImageData);
				if (m_pData == NULL)
					m_pData = pData;
				else
					delete[] pData;
			}
			if (m_bControlling)
			{
				float fCenterX = (m_tClientSize.cx>>1)+m_fControlledOffsetX*m_tZoomedSize.cx;
				float fCenterY = (m_tClientSize.cy>>1)+m_fControlledOffsetY*m_tZoomedSize.cy;
				if (m_nControlledSizeX && m_nControlledSizeY && m_fControlledZoom != 0.0f)
				{
					RECT rcSel =
					{
						fCenterX - m_nControlledSizeX*0.5f*m_fZoom/m_fControlledZoom + 0.5f,
						fCenterY - m_nControlledSizeY*0.5f*m_fZoom/m_fControlledZoom + 0.5f,
						fCenterX + m_nControlledSizeX*0.5f*m_fZoom/m_fControlledZoom + 0.5f,
						fCenterY + m_nControlledSizeY*0.5f*m_fZoom/m_fControlledZoom + 0.5f,
					};
					if (rcSel.right-rcSel.left < rc.right-rc.left)
					{
						if (rcSel.left < rc.left)
						{
							rcSel.right += rc.left-rcSel.left;
							rcSel.left = rc.left;
						}
						else if (rcSel.right > rc.right)
						{
							rcSel.left += rc.right-rcSel.right;
							rcSel.right = rc.right;
						}
					}
					else
					{
						rcSel.left = rc.left;
						rcSel.right = rc.right;
					}
					if (rcSel.bottom-rcSel.top < rc.bottom-rc.top)
					{
						if (rcSel.top < rc.top)
						{
							rcSel.bottom += rc.top-rcSel.top;
							rcSel.top = rc.top;
						}
						else if (rcSel.bottom > rc.bottom)
						{
							rcSel.top += rc.bottom-rcSel.bottom;
							rcSel.bottom = rc.bottom;
						}
					}
					else
					{
						rcSel.top = rc.top;
						rcSel.bottom = rc.bottom;
					}
					RECT rc1 = {rcSel.left, rcSel.top, rcSel.right, rcSel.top+2};
					a_cDC.FillRect(&rc1, COLOR_MENUHILIGHT);
					RECT rc2 = {rcSel.left, rcSel.bottom-2, rcSel.right, rcSel.bottom};
					a_cDC.FillRect(&rc2, COLOR_MENUHILIGHT);
					RECT rc3 = {rcSel.left, rcSel.top+2, rcSel.left+2, rcSel.bottom-2};
					a_cDC.FillRect(&rc3, COLOR_MENUHILIGHT);
					RECT rc4 = {rcSel.right-2, rcSel.top+2, rcSel.right, rcSel.bottom-2};
					a_cDC.FillRect(&rc4, COLOR_MENUHILIGHT);
				}
				else
				{
					RECT rc1 = {fCenterX+0.5f, fCenterY-2.5f, fCenterX+1.5f, fCenterY+4.5f};
					a_cDC.FillRect(&rc1, COLOR_MENUHILIGHT);
					RECT rc2 = {fCenterX-2.5f, fCenterY, fCenterX+4.5f, fCenterY+1.5f};
					a_cDC.FillRect(&rc2, COLOR_MENUHILIGHT);
				}
			}
		}
	}

	float GetFitToSizeZoom(int a_nSizeX, int a_nSizeY) const
	{
		float const fZoomX = a_nSizeX/static_cast<float>(m_tOrigSize.cx);
		float const fZoomY = a_nSizeY/static_cast<float>(m_tOrigSize.cy);
		return fZoomX < fZoomY ? fZoomX : fZoomY;
	}

	void UpdateState()
	{
		if (!IsWindow() || m_bUpdating)
			return;

		bool bIsVisible = (GetWindowLong(GWL_STYLE) & WS_VISIBLE) != 0;
		if (bIsVisible)
		{
			SetRedraw(FALSE);
		}

		m_bUpdating = true; // prevent recursion due to WM_SIZE caused by ShowScrollBar

		RECT rcClient;
		ShowScrollBar(SB_HORZ, FALSE);
		ShowScrollBar(SB_VERT, FALSE);
		GetClientRect(&rcClient);

		if (m_bAutoZoom)
		{
			m_fZoom = GetFitToSizeZoom(rcClient.right, rcClient.bottom);
		}
		else
		{
			float fMaxZoom = GetFitToSizeZoom(rcClient.right, rcClient.bottom);
			if (fMaxZoom < 32.0f)
				fMaxZoom = 32.0f;
			if (m_fZoom > fMaxZoom)
				m_fZoom = fMaxZoom;
			float fMinZoom = GetFitToSizeZoom(rcClient.right, rcClient.bottom);
			if (fMinZoom > (1.0f/max(m_tOrigSize.cx, m_tOrigSize.cy)))
				fMinZoom = (1.0f/max(m_tOrigSize.cx, m_tOrigSize.cy));
			if (m_fZoom < fMinZoom)
				m_fZoom = fMinZoom;
		}
		m_tZoomedSize.cx = static_cast<LONG>(m_tOrigSize.cx*m_fZoom+0.5f);
		m_tZoomedSize.cy = static_cast<LONG>(m_tOrigSize.cy*m_fZoom+0.5f);

		if (rcClient.right < m_tZoomedSize.cx)
		{
			ShowScrollBar(SB_HORZ, TRUE);
			GetClientRect(&rcClient);
		}
		if (rcClient.bottom < m_tZoomedSize.cy)
		{
			ShowScrollBar(SB_VERT, TRUE);
			GetClientRect(&rcClient);
		}
		if (rcClient.right < m_tZoomedSize.cx) // horizontal size must be re-checked again
		{
			ShowScrollBar(SB_HORZ, TRUE);
			GetClientRect(&rcClient);
		}
		m_tClientSize.cx = rcClient.right;
		m_tClientSize.cy = rcClient.bottom;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = m_tZoomedSize.cx - 1;
		si.nPage = m_tClientSize.cx;
		si.nPos = 0;
		SetScrollInfo(SB_HORZ, &si, FALSE);
		si.nMax = m_tZoomedSize.cy - 1;
		si.nPage = m_tClientSize.cy;
		si.nPos = 0;
		SetScrollInfo(SB_VERT, &si, FALSE);
		UpdatePosition();

		m_bUpdating = false;

		if (bIsVisible)
		{
			SetRedraw();
		}

		Invalidate(FALSE);
	}

	void UpdatePosition()
	{
		if (m_tZoomedSize.cx > m_tClientSize.cx)
		{
			float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
			if (m_fOffsetX < -fOffsetXBound)
				m_fOffsetX = -fOffsetXBound;
			else
			if (m_fOffsetX > fOffsetXBound)
				m_fOffsetX = fOffsetXBound;
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = static_cast<int>((m_fOffsetX+fOffsetXBound)*m_tZoomedSize.cx+0.5f);
			SetScrollInfo(SB_HORZ, &si, TRUE);
		}
		else
		{
			m_fOffsetX = 0.0f;
		}

		if (m_tZoomedSize.cy > m_tClientSize.cy)
		{
			float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
			if (m_fOffsetY < -fOffsetYBound)
				m_fOffsetY = -fOffsetYBound;
			else
			if (m_fOffsetY > fOffsetYBound)
				m_fOffsetY = fOffsetYBound;
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = static_cast<int>((m_fOffsetY+fOffsetYBound)*m_tZoomedSize.cy);
			SetScrollInfo(SB_VERT, &si, TRUE);
		}
		else
		{
			m_fOffsetY = 0.0f;
		}

		Invalidate(FALSE);
	}

private:
	// original data
	CComPtr<IDocumentImage> m_pDI;
	BYTE* m_pData;
	SIZE m_tOrigSize;
	COLORREF m_tBackground;
	bool m_bFrame;
	CRITICAL_SECTION m_tImageData;

	bool m_bAutoZoom;

	// current viewpoint
	bool m_bUpdating;
	float m_fZoom;
	float m_fOffsetX; // when 0, center of image in is center of screen
	float m_fOffsetY; // when 1, right/bottom of image is in center of screen

	// cached values
	SIZE m_tZoomedSize;
	SIZE m_tClientSize;

	// temporary values
	int m_nWheelDelta;
	float m_fMouseDownX;
	float m_fMouseDownY;

	// controlled viewport
	bool m_bControlling;
	float m_fControlledOffsetX;
	float m_fControlledOffsetY;
	float m_fControlledZoom;
	ULONG m_nControlledSizeX;
	ULONG m_nControlledSizeY;
};
