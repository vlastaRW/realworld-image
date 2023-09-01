#pragma once

template<class T>
class CRasterImageMouseInput
{
public:
	CRasterImageMouseInput() :
		m_eDragState(EDSNothing), m_bAutoScroll(false), m_bTrackMouse(false), m_nWheelDelta(0)
	{
	}
	~CRasterImageMouseInput()
	{
	}
	typedef std::vector<POINT> CGesturePoints;

	// block mouse messages when tablet is active
	bool IsMouseDisabled() { return false; } // (GetTickCount()-m_dwLastMessage) <= TABLETBLOCKTIME

BEGIN_MSG_MAP(CRasterImageMouseInput<T>)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
	MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
	MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
	MESSAGE_HANDLER(WM_MBUTTONDOWN, OnMButtonDown)
	MESSAGE_HANDLER(WM_MBUTTONUP, OnMButtonUp)
	MESSAGE_HANDLER(WM_XBUTTONDOWN, OnXButtonDown)
	MESSAGE_HANDLER(WM_XBUTTONUP, OnXButtonUp)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
END_MSG_MAP()

	// message handlers
public:
	void MouseMoved(TPixelCoords const& a_tPos) {}
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
		POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		TPixelCoords tLastPos;
		static_cast<T*>(this)->GetPixelFromPoint(pt, &tLastPos);//, NULL, m_eDragState == EDSHandle ? &m_nHandleIndex : NULL, eKeysState);
		static_cast<T*>(this)->MouseMoved(tLastPos);

		switch (m_eDragState)
		{
		case EDSNothing:
			if (static_cast<T*>(this)->IsMouseDisabled())
				break;
			{
				DWORD dwDummy = 0;
				TPixelCoords tPointerSize = {float(static_cast<T*>(this)->M_ImageSize().cx)/float(static_cast<T*>(this)->M_ZoomedSize().cx), float(static_cast<T*>(this)->M_ImageSize().cy)/float(static_cast<T*>(this)->M_ZoomedSize().cy)};
				static_cast<T*>(this)->ProcessInputEvent(eKeysState, &tLastPos, &tPointerSize, /*m_fLastPressure = */0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			}
			if (!m_bTrackMouse)
			{
				m_bTrackMouse = true;
				TRACKMOUSEEVENT tME;
				tME.cbSize = sizeof tME;
				tME.hwndTrack = static_cast<T*>(this)->m_hWnd;
				tME.dwFlags = TME_LEAVE;
				TrackMouseEvent(&tME);
			}
			break;
		case EDSPanning:
			static_cast<T*>(this)->MoveViewport(
				m_fOrigOffsetX-float(pt.x-m_tDragStartClient.x)/static_cast<T*>(this)->M_ZoomedSize().cx,
				m_fOrigOffsetY-float(pt.y-m_tDragStartClient.y)/static_cast<T*>(this)->M_ZoomedSize().cy,
				static_cast<T*>(this)->M_ImageZoom());
			static_cast<T*>(this)->UpdateWindow();
			break;
		case EDSZooming:
			{
				float fZoom = m_fOrigZoom*powf(1.01f, m_tDragStartClient.y-pt.y);
				LONG fDX = m_tDragStartClient.x-(static_cast<T*>(this)->M_WindowSize().cx>>1);
				LONG fDY = m_tDragStartClient.y-(static_cast<T*>(this)->M_WindowSize().cy>>1);
				if (fZoom > 32.0f)
				{
					fZoom = 32.0f;
				}
				else
				{
					LONG nSize = min(static_cast<T*>(this)->M_ImageSize().cx, static_cast<T*>(this)->M_ImageSize().cy);
					float fMinZoom = 1.0f/nSize;
					if (fZoom <= fMinZoom)
						fZoom = fMinZoom;
				}
				static_cast<T*>(this)->MoveViewport(
					m_fOrigOffsetX + (fDX/m_fOrigZoom-fDX/fZoom)/static_cast<T*>(this)->M_ImageSize().cx,
					m_fOrigOffsetY + (fDY/m_fOrigZoom-fDY/fZoom)/static_cast<T*>(this)->M_ImageSize().cy,
					fZoom);
				static_cast<T*>(this)->UpdateWindow();
			}
			break;
		case EDSGesture:
			{
				POINT const tLast = *m_cGesturePoints.rbegin();
				m_cGesturePoints.push_back(pt);
				if (m_nGestureXMin > pt.x) m_nGestureXMin = pt.x;
				if (m_nGestureXMax < pt.x) m_nGestureXMax = pt.x;
				if (m_nGestureYMin > pt.y) m_nGestureYMin = pt.y;
				if (m_nGestureYMax < pt.y) m_nGestureYMax = pt.y;
				RECT rc = {min(tLast.x, pt.x)-2, min(tLast.y, pt.y)-2, max(tLast.x, pt.x)+3, max(tLast.y, pt.y)+3};
				static_cast<T*>(this)->InvalidateRect(&rc, FALSE);
			}
			break;
		case EDSUser:
			// scroll window if dragged outside of window
			if (pt.x < -2 || pt.y < -2 || pt.x >= static_cast<T*>(this)->M_WindowSize().cx+2 || pt.y >= static_cast<T*>(this)->M_WindowSize().cy+2)
			{
				m_tScrollMousePos = pt;
				if (!m_bAutoScroll)
				{
					m_bAutoScroll = true;
					static_cast<T*>(this)->SetTimer(SCROLL_TIMERID, SCROLL_TICK);
					m_dwScrollStart = GetTickCount();
				}
				else if ((GetTickCount()-m_dwScrollStart) > SCROLL_TICK)
				{
					BOOL b;
					OnTimer(0, 0, 0, b);
				}
			}
			else
			{
				if (m_bAutoScroll)
				{
					m_bAutoScroll = false;
					static_cast<T*>(this)->KillTimer(SCROLL_TIMERID);
				}
			}
			DWORD dwDummy = 0;
			TPixelCoords tPointerSize = {float(static_cast<T*>(this)->M_ImageSize().cx)/float(static_cast<T*>(this)->M_ZoomedSize().cx), float(static_cast<T*>(this)->M_ImageSize().cy)/float(static_cast<T*>(this)->M_ZoomedSize().cy)};
			static_cast<T*>(this)->ProcessInputEvent(eKeysState, &tLastPos, &tPointerSize, /*m_fLastPressure = */1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			break;
		}

		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSNothing:
			{
				if (m_bTrackMouse)
				{
					TRACKMOUSEEVENT tME;
					tME.cbSize = sizeof tME;
					tME.hwndTrack = static_cast<T*>(this)->m_hWnd;
					tME.dwFlags = TME_LEAVE|TME_CANCEL;
					TrackMouseEvent(&tME);
					m_bTrackMouse = false;
				}
				if (static_cast<T*>(this)->IsMouseDisabled())
					break;
				static_cast<T*>(this)->SetCapture();
				POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
				
				m_eDragState = EDSUser;
				EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
				TPixelCoords tPos;
				TPixelCoords tPointerSize;
				static_cast<T*>(this)->GetPixelFromPoint(pt, &tPos, &tPointerSize, NULL, eKeysState);
				DWORD dwDummy = 0;
				static_cast<T*>(this)->ProcessInputEvent(eKeysState, &tPos, &tPointerSize, /*m_fLastPressure = */1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			}
			break;
		case EDSPanning:
		case EDSZooming:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSUser:
			{
				ReleaseCapture();
				m_eDragState = EDSNothing;
				POINT pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
				TPixelCoords* pPos = NULL;

				EControlKeysState eKeysState = (a_wParam&MK_CONTROL) ? (a_wParam&MK_SHIFT ? ECKSShiftControl : ECKSControl) : (a_wParam&MK_SHIFT ? ECKSShift : ECKSNone);
				TPixelCoords tPos;
				TPixelCoords tPointerSize;
				static_cast<T*>(this)->GetPixelFromPoint(pt, &tPos, &tPointerSize, NULL, eKeysState);
				if (pt.x >= 0 && pt.y >= 0 && pt.x < static_cast<T*>(this)->M_WindowSize().cx && pt.y < static_cast<T*>(this)->M_WindowSize().cy)
					pPos = &tPos;
				DWORD dwDummy = 0;
				static_cast<T*>(this)->ProcessInputEvent(eKeysState, pPos, &tPointerSize, /*m_fLastPressure = */0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			}
			break;
		case EDSNothing:
		case EDSPanning:
		case EDSZooming:
		case EDSGesture:
			break;
		}

		if (m_bAutoScroll)
		{
			m_bAutoScroll = false;
			static_cast<T*>(this)->KillTimer(SCROLL_TIMERID);
		}

		return 0;
	}

	LRESULT OnRButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSNothing:
			{
				if (m_bTrackMouse)
				{
					TRACKMOUSEEVENT tME;
					tME.cbSize = sizeof tME;
					tME.hwndTrack = static_cast<T*>(this)->m_hWnd;
					tME.dwFlags = TME_LEAVE|TME_CANCEL;
					TrackMouseEvent(&tME);
					m_bTrackMouse = false;
				}
				static_cast<T*>(this)->SetCapture();
				m_eDragState = EDSGesture;
				POINT const pt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
				m_cGesturePoints.push_back(pt);
				m_nGestureXMin = m_nGestureXMax = pt.x;
				m_nGestureYMin = m_nGestureYMax = pt.y;
			}
			break;
		case EDSPanning:
		case EDSZooming:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	void ProcessGesture(ULONG UNREF(a_nPoints), POINT const* UNREF(a_pPoints)) {}

	LRESULT OnRButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bAutoScroll)
		{
			m_bAutoScroll = false;
			static_cast<T*>(this)->KillTimer(SCROLL_TIMERID);
		}

		switch (m_eDragState)
		{
		case EDSGesture:
			{
				ReleaseCapture();
				m_eDragState = EDSNothing;
				if (m_cGesturePoints.empty())
					break;
				POINT ptCenter = m_cGesturePoints[0];
				int nDragX = GetSystemMetrics(SM_CXDRAG);
				int nDragY = GetSystemMetrics(SM_CYDRAG);
				if (m_nGestureXMin < (ptCenter.x-nDragX) || m_nGestureXMax > (ptCenter.x+nDragX) ||
					m_nGestureYMin < (ptCenter.y-nDragY) || m_nGestureYMax > (ptCenter.y+nDragY))
				{
					CGesturePoints cGesturePoints;
					std::swap(cGesturePoints, m_cGesturePoints);
					RECT rc = {m_nGestureXMin-2, m_nGestureYMin-2, m_nGestureXMax+3, m_nGestureYMax+3};
					static_cast<T*>(this)->InvalidateRect(&rc, FALSE);
					static_cast<T*>(this)->ProcessGesture(cGesturePoints.size(), &(cGesturePoints[0]));
				}
				else
				{
					RECT rc = {m_nGestureXMin-2, m_nGestureYMin-2, m_nGestureXMax+3, m_nGestureYMax+3};
					static_cast<T*>(this)->InvalidateRect(&rc, FALSE);

					static_cast<T*>(this)->OnContextMenu(&(m_cGesturePoints[0]));
					m_cGesturePoints.clear();
				}
			}
			break;
		case EDSNothing:
		case EDSPanning:
		case EDSZooming:
			break;
		}

		return 0;
	}
	void OnContextMenu(POINT const*)
	{
	}

	LRESULT OnMButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSNothing:
			{
				if (m_bTrackMouse)
				{
					TRACKMOUSEEVENT tME;
					tME.cbSize = sizeof tME;
					tME.hwndTrack = static_cast<T*>(this)->m_hWnd;
					tME.dwFlags = TME_LEAVE|TME_CANCEL;
					TrackMouseEvent(&tME);
					m_bTrackMouse = false;
				}
				static_cast<T*>(this)->SetCapture();
				m_eDragState = a_wParam&MK_CONTROL ? EDSZooming : EDSPanning;
				m_tDragStartClient.x = GET_X_LPARAM(a_lParam);
				m_tDragStartClient.y = GET_Y_LPARAM(a_lParam);
				static_cast<T*>(this)->GetPixelFromPoint(m_tDragStartClient, &m_tDragStartImage);
				m_fOrigOffsetX = static_cast<T*>(this)->M_OffsetX();
				m_fOrigOffsetY = static_cast<T*>(this)->M_OffsetY();
				m_fOrigZoom = static_cast<T*>(this)->M_ImageZoom();
			}
			break;
		case EDSPanning:
		case EDSZooming:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	LRESULT OnMButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSPanning:
		case EDSZooming:
			{
				ReleaseCapture();
				m_eDragState = EDSNothing;
			}
			break;
		case EDSNothing:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	LRESULT OnXButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSNothing:
			{
				static_cast<T*>(this)->SetCapture();
				m_eDragState = HIWORD(a_wParam) == XBUTTON1 ? EDSZooming : EDSPanning;
				m_tDragStartClient.x = GET_X_LPARAM(a_lParam);
				m_tDragStartClient.y = GET_Y_LPARAM(a_lParam);
				static_cast<T*>(this)->GetPixelFromPoint(m_tDragStartClient, &m_tDragStartImage);
				m_fOrigOffsetX = static_cast<T*>(this)->M_OffsetX();
				m_fOrigOffsetY = static_cast<T*>(this)->M_OffsetY();
				m_fOrigZoom = static_cast<T*>(this)->M_ImageZoom();
			}
			break;
		case EDSPanning:
		case EDSZooming:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	LRESULT OnXButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		switch (m_eDragState)
		{
		case EDSPanning:
		case EDSZooming:
			{
				ReleaseCapture();
				m_eDragState = EDSNothing;
			}
			break;
		case EDSNothing:
		case EDSGesture:
		case EDSUser:
			break;
		}

		return 0;
	}

	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);
		POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		static_cast<T*>(this)->ScreenToClient(&tPt);

		while (m_nWheelDelta >= WHEEL_DELTA)
		{
			m_nWheelDelta -= WHEEL_DELTA;
			static_cast<T*>(this)->ZoomIn(tPt);
		}
		while ((-m_nWheelDelta) >= WHEEL_DELTA)
		{
			m_nWheelDelta += WHEEL_DELTA;
			static_cast<T*>(this)->ZoomOut(tPt);
		}

		return 0;
	}

	LRESULT OnTimer(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (m_eDragState != EDSUser)
		{
			m_bAutoScroll = false;
			return 0;
		}

		int nSteps = (GetTickCount()-m_dwScrollStart)/SCROLL_TICK;
		if (nSteps < 1)
		{
			static_cast<T*>(this)->SetTimer(SCROLL_TIMERID, SCROLL_TICK);
			return 0;
		}

		ULONG nDX = 0;
		if (m_tScrollMousePos.x < -2)
		{
			nDX = m_tScrollMousePos.x < -15 ? -30 : -10;
		}
		else if (m_tScrollMousePos.x >= static_cast<T*>(this)->M_WindowSize().cx+2)
		{
			nDX = m_tScrollMousePos.x >= static_cast<T*>(this)->M_WindowSize().cx+15 ? 30 : 10;
		}

		ULONG nDY = 0;
		if (m_tScrollMousePos.y < -2)
		{
			nDY = m_tScrollMousePos.y < -20 ? -40 : -15;
		}
		else if (m_tScrollMousePos.y >= static_cast<T*>(this)->M_WindowSize().cy+2)
		{
			nDY = m_tScrollMousePos.y >= static_cast<T*>(this)->M_WindowSize().cy+20 ? 40 : 15;
		}

		nDX *= nSteps;
		nDY *= nSteps;
		m_dwScrollStart += nSteps*SCROLL_TICK;

		if (nDX || nDY)
		{
			bool bMoved = false;
			float fOffsetX = static_cast<T*>(this)->M_OffsetX();
			float fOffsetY = static_cast<T*>(this)->M_OffsetY();
			if (nDX && static_cast<T*>(this)->M_ZoomedSize().cx != static_cast<T*>(this)->M_WindowSize().cx)
			{
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;
				if (static_cast<T*>(this)->GetScrollInfo(SB_HORZ, &si))
				{
					int nNew = si.nPos + nDX;
					if (nNew < si.nMin) nNew = si.nMin;
					else if (nNew > int(si.nMax-si.nPage)) nNew = si.nMax-si.nPage;
					if (nNew != si.nPos)
					{
						si.nPos = nNew;
						si.fMask = SIF_POS;
						float const fOffsetBound = 0.5f*static_cast<float>(static_cast<T*>(this)->M_ZoomedSize().cx-static_cast<T*>(this)->M_WindowSize().cx+static_cast<T*>(this)->CanvasPadding()*2)/static_cast<float>(static_cast<T*>(this)->M_ZoomedSize().cx);
						fOffsetX = static_cast<float>(nNew)*(fOffsetBound*2)/(static_cast<T*>(this)->M_ZoomedSize().cx-1+(static_cast<T*>(this)->CanvasPadding()*2)-static_cast<T*>(this)->M_WindowSize().cx)-fOffsetBound;
						static_cast<T*>(this)->SetScrollInfo(SB_HORZ, &si);
						bMoved = true;
					}
				}
			}
			if (nDY && static_cast<T*>(this)->M_ZoomedSize().cy != static_cast<T*>(this)->M_WindowSize().cy)
			{
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_POS|SIF_RANGE|SIF_PAGE;
				if (static_cast<T*>(this)->GetScrollInfo(SB_VERT, &si))
				{
					int nNew = si.nPos + nDY;
					if (nNew < si.nMin) nNew = si.nMin;
					else if (nNew > int(si.nMax-si.nPage)) nNew = si.nMax-si.nPage;
					if (nNew != si.nPos)
					{
						si.nPos = nNew;
						si.fMask = SIF_POS;
						float const fOffsetBound = 0.5f*static_cast<float>(static_cast<T*>(this)->M_ZoomedSize().cy-static_cast<T*>(this)->M_WindowSize().cy+static_cast<T*>(this)->CanvasPadding()*2)/static_cast<float>(static_cast<T*>(this)->M_ZoomedSize().cy);
						fOffsetY = static_cast<float>(nNew)*(fOffsetBound*2)/(static_cast<T*>(this)->M_ZoomedSize().cy-1+(static_cast<T*>(this)->CanvasPadding()*2)-static_cast<T*>(this)->M_WindowSize().cy)-fOffsetBound;
						static_cast<T*>(this)->SetScrollInfo(SB_VERT, &si);
						bMoved = true;
					}
				}
			}
			if (bMoved)
			{
				EControlKeysState eKeysState = (GetAsyncKeyState(VK_CONTROL)&0x8000) ? ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShiftControl : ECKSControl) : ((GetAsyncKeyState(VK_SHIFT)&0x8000) ? ECKSShift : ECKSNone);
				TPixelCoords tPointerSize = {float(static_cast<T*>(this)->M_ImageSize().cx)/float(static_cast<T*>(this)->M_ZoomedSize().cx), float(static_cast<T*>(this)->M_ImageSize().cy)/float(static_cast<T*>(this)->M_ZoomedSize().cy)};
				TPixelCoords tLastPos;
				static_cast<T*>(this)->GetPixelFromPoint(m_tScrollMousePos, &tLastPos);//, NULL, m_eDragState == EDSHandle ? &m_nHandleIndex : NULL, eKeysState);
				static_cast<T*>(this)->MouseMoved(tLastPos);
				DWORD dwDummy = 0;
				static_cast<T*>(this)->ProcessInputEvent(eKeysState, &tLastPos, &tPointerSize, /*m_fLastPressure = */1.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
				static_cast<T*>(this)->MoveViewport(fOffsetX, fOffsetY, static_cast<T*>(this)->M_ImageZoom());
				//static_cast<T*>(this)->Invalidate(FALSE);
			}
		}

		static_cast<T*>(this)->SetTimer(SCROLL_TIMERID, SCROLL_TICK);

		return 0;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		LRESULT lRes = static_cast<T*>(this)->GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		
		static_cast<T*>(this)->SetFocus();

		return MA_ACTIVATE;
	}

	LRESULT OnMouseLeave(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_bTrackMouse)
		{
			DWORD dwDummy;
			static_cast<T*>(this)->ProcessInputEvent(ECKSNone, NULL, NULL, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, &dwDummy);
			m_bTrackMouse = false;
		}
		return 0;
	}

	void RenderMouseTrail(RECT const& a_rcImage, ULONG a_nWindowX, ULONG a_nWindowY, RECT const& a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride)
	{
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

private:
	enum EDragState
	{
		EDSNothing = 0,
		EDSPanning,
		EDSZooming,
		EDSGesture,
		EDSUser,
	};
	enum { SCROLL_TIMERID = 82, SCROLL_TICK = 100, TABLETBLOCKTIME = 500 };

private:
	EDragState m_eDragState;

	POINT m_tDragStartClient;
	TPixelCoords m_tDragStartImage;
	float m_fOrigOffsetX;
	float m_fOrigOffsetY;
	float m_fOrigZoom;

	bool m_bAutoScroll;
	bool m_bTrackMouse;
	POINT m_tScrollMousePos;
	DWORD m_dwScrollStart;

	int m_nWheelDelta;

	// mouse gestures
	CGesturePoints m_cGesturePoints;
	LONG m_nGestureXMin;
	LONG m_nGestureYMin;
	LONG m_nGestureXMax;
	LONG m_nGestureYMax;
};

