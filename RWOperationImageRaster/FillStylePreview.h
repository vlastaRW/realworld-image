
#pragma once


class CGradientPreview :
	public CWindowImpl<CGradientPreview>,
	public CThemeImpl<CGradientPreview>,
	public IRasterImageBrushOwner
{
public:
	CGradientPreview() : m_bEnableUpdates(true), m_nDragging(-1), m_fGamma(2.2f)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CGradientPreview()
	{
	}

	enum { GPN_STATECHANGED = 1 };


	DECLARE_WND_CLASS_EX(_T("GradientPreview"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_BTNFACE);

	BEGIN_MSG_MAP(CGradientPreview)
		CHAIN_MSG_MAP(CThemeImpl<CGradientPreview>)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		//MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		//MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		//MESSAGE_HANDLER(WM_CREATE, OnCreate)
		//MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	// operations
public:
	void UpdateStyle(IRasterImageFillStyleManager* a_pFSMgr, BSTR a_bstrID, BSTR a_bstrParams)
	{
		if (!m_bEnableUpdates)
			return;
		bool bUpdate = false;
		if (m_bstrID != a_bstrID)
		{
			m_bstrID = a_bstrID;
			m_pBrush = NULL;
			a_pFSMgr->FillStyleCreate(a_bstrID, NULL, &m_pBrush);
			m_pBrush->Init(this);
			m_bstrParams.Empty();
			bUpdate = true;
		}
		if (m_bstrParams != a_bstrParams)
		{
			m_bstrParams = a_bstrParams;
			CComQIPtr<IRasterImageEditToolScripting> pScript(m_pBrush);
			if (pScript)
			{
				m_bEnableUpdates = false;
				pScript->FromText(m_bstrParams);
				m_bEnableUpdates = true;
				bUpdate = true;
			}
		}
		if (bUpdate)
			Invalidate(FALSE);
	}
	void Serialize(CComBSTR& a_bstr)
	{
		CComQIPtr<IRasterImageEditToolScripting> pScript(m_pBrush);
		if (pScript)
			pScript->ToText(&a_bstr);
	}
	void State(ISharedState* a_pState)
	{
		if (m_pBrush)
			m_pBrush->SetState(a_pState);
	}
	void State(ISharedState** a_ppState)
	{
		if (m_pBrush)
			m_pBrush->GetState(a_ppState);
	}


	// IRasterImageBrushOwner methods
public:
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject)
	{
		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, __uuidof(IRasterImageBrushOwner)))
		{
			*ppvObject = static_cast<IRasterImageBrushOwner*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)()
	{
		return 2;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 1;
	}
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		if (m_hWnd)
		{
			RECT rc;
			GetClientRect(&rc);
			*a_pSizeX = rc.right;
			*a_pSizeY = rc.bottom;
			return S_OK;
		}
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		Invalidate(FALSE);
		return S_FALSE;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		Invalidate(FALSE);
		return S_FALSE;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		InvalidateRect(a_pChanged, FALSE);
		if (m_bEnableUpdates)
		{
			m_bEnableUpdates = false;
			NMHDR nm = {m_hWnd, GetWindowLong(GWLP_ID), GPN_STATECHANGED};
			::SendMessage(GetParent(), WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
			m_bEnableUpdates = true;
		}
		return S_FALSE;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return S_FALSE;
	}
	STDMETHOD(SetBrushState)(ISharedState* a_pState)
	{
		return S_FALSE;
	}

	// handlers
public:
	//LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{
	//	return 0;
	//}

	//LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{
	//	return 0;
	//}

	//LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{
	//	return 0;
	//}

	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pBrush && m_nDragging < 0)
		{
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(&p);
			ULONG nPts = 0;
			m_pBrush->GetControlPointCount(&nPts);
			static ULONG const nPointRadius = 3;
			for (ULONG i = 0; i < nPts; ++i)
			{
				ULONG n;
				TPixelCoords tPos;
				if (SUCCEEDED(m_pBrush->GetControlPoint(i, &tPos, &n)))
				{
					POINT tPt = {tPos.fX+0.5f, tPos.fY+0.5f};
					if (abs(p.x-tPt.x) <= nPointRadius && abs(p.y-tPt.y) <= nPointRadius)
					{
						static HCURSOR hHand = ::LoadCursor(NULL, IDC_HAND);
						SetCursor(hHand);
						return 0;
					}
				}
			}
		}

		static HCURSOR hArrow = ::LoadCursor(NULL, IDC_ARROW);
		SetCursor(hArrow);
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nDragging = -1;
		ReleaseCapture();
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		POINT p = {(short)LOWORD(a_lParam), (short)HIWORD(a_lParam)};

		if (m_pBrush)
		{
			ULONG nPts = 0;
			m_pBrush->GetControlPointCount(&nPts);
			static ULONG const nPointRadius = 3;
			for (ULONG i = 0; i < nPts; ++i)
			{
				ULONG n;
				TPixelCoords tPos;
				if (SUCCEEDED(m_pBrush->GetControlPoint(i, &tPos, &n)))
				{
					POINT tPt = {tPos.fX+0.5f, tPos.fY+0.5f};
					if (abs(p.x-tPt.x) <= nPointRadius && abs(p.y-tPt.y) <= nPointRadius)
					{
						m_nDragging = i;
						m_tLastPos = p;
						SetCapture();
						break;
					}
				}
			}
		}

		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_nDragging >= 0 && m_pBrush && GetCapture() == m_hWnd)
		{
			POINT p = {(short)LOWORD(a_lParam), (short)HIWORD(a_lParam)};
			if (m_tLastPos.x != p.x || m_tLastPos.y != p.y)
			{
				m_tLastPos = p;
				TPixelCoords tPos = {p.x, p.y};
				m_pBrush->AdjustCoordinates(&tPos, m_nDragging);
				m_pBrush->SetControlPoint(m_nDragging, &tPos, false, 1.0f);
			}
		}
		return 0;
	}

	//LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{
	//	Invalidate(FALSE);
	//	return 0;
	//}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
//		HWND hPar = GetParent(hWnd);

		// get window style
		DWORD dwStyle = GetWindowLong(GWL_STYLE);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(&ps);
		HGDIOBJ hBmp;
		RECT rcWin;
		GetClientRect(&rcWin);

		// get client rectangle and rectangle to update
		RECT rUpd;
		if (ps.rcPaint.bottom == 0 || ps.rcPaint.right == 0)
			rUpd = rcWin;
		else
			rUpd = ps.rcPaint;

		if (m_pBrush == NULL)
		{
			FillRect(hDC, &rUpd, reinterpret_cast<HBRUSH>(COLOR_3DSHADOW+1));
		}
		else
		{
			COLORREF clr1 = GetSysColor(COLOR_3DSHADOW);
			COLORREF clr2 = GetSysColor(COLOR_3DLIGHT);
			ULONG aClr[] = {GetRValue(clr1), GetGValue(clr1), GetBValue(clr1), GetRValue(clr2), GetGValue(clr2), GetBValue(clr2)};
			CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[(rUpd.right-rUpd.left)*(rUpd.bottom-rUpd.top)]);
			m_pBrush->GetBrushTile(rUpd.left, rUpd.top, rUpd.right-rUpd.left, rUpd.bottom-rUpd.top, m_fGamma, rUpd.right-rUpd.left, cBuffer);
			BITMAPINFO tBMI;
			ZeroMemory(&tBMI, sizeof tBMI);
			tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
			tBMI.bmiHeader.biWidth = rUpd.right-rUpd.left;
			tBMI.bmiHeader.biHeight = rUpd.top-rUpd.bottom;
			tBMI.bmiHeader.biPlanes = 1;
			tBMI.bmiHeader.biBitCount = 32;
			tBMI.bmiHeader.biCompression = BI_RGB;
			TRasterImagePixel* p = cBuffer;
			for (LONG y = rUpd.top; y < rUpd.bottom; ++y)
			{
				for (LONG x = rUpd.left; x < rUpd.right; ++x, ++p)
				{
					if (p->bA == 255)
					{
					}
					else
					{
						ULONG const* pClr = aClr + ((0x8&(x^y))>>3)*3;
						ULONG const nIA = 255-p->bA;
						p->bB = (p->bA*ULONG(p->bB) + nIA*pClr[2])/255;
						p->bG = (p->bA*ULONG(p->bG) + nIA*pClr[1])/255;
						p->bR = (p->bA*ULONG(p->bR) + nIA*pClr[0])/255;
					}
				}
			}
			ULONG nPts = 0;
			m_pBrush->GetControlPointCount(&nPts);
			static ULONG const nPointRadius = 3;
			static BYTE const aPointPattern[(nPointRadius+nPointRadius+1)*(nPointRadius+nPointRadius+1)] =
			{
				0, 0, 1, 1, 1, 0, 0,
				0, 1, 2, 2, 2, 1, 0,
				1, 2, 2, 2, 2, 2, 1,
				1, 2, 2, 2, 2, 2, 1,
				1, 2, 2, 2, 2, 2, 1,
				0, 1, 2, 2, 2, 1, 0,
				0, 0, 1, 1, 1, 0, 0,
			};
			static TRasterImagePixel const tOutline = {0x40, 0xff, 0xff, 0xff}; // R<->B
			static TRasterImagePixel const tFill = {0xff, 0x40, 0x40, 0xff};
			for (ULONG i = 0; i < nPts; ++i)
			{
				ULONG n;
				TPixelCoords tPos;
				if (SUCCEEDED(m_pBrush->GetControlPoint(i, &tPos, &n)))
				{
					POINT tPt = {tPos.fX+0.5f, tPos.fY+0.5f};
					BYTE const* pPat = aPointPattern;
					for (LONG y = tPt.y-nPointRadius; y <= LONG(tPt.y+nPointRadius); ++y)
					{
						if (y >= rUpd.top && y < rUpd.bottom)
						{
							for (LONG x = tPt.x-nPointRadius; x <= LONG(tPt.x+nPointRadius); ++x, ++pPat)
							{
								if (x >= rUpd.left && x < rUpd.right && *pPat)
								{
									cBuffer[(y-rUpd.top)*(rUpd.right-rUpd.left)+x-rUpd.left] = *pPat == 1 ? tOutline : tFill;
								}
							}
						}
						else
						{
							pPat += nPointRadius+nPointRadius+1;
						}
					}
				}
			}
			SetDIBitsToDevice(hDC, rUpd.left, rUpd.top, rUpd.right-rUpd.left, rUpd.bottom-rUpd.top, 0, 0, 0, rUpd.bottom-rUpd.top, cBuffer.m_p, &tBMI, DIB_RGB_COLORS);
		}

		EndPaint(&ps);

		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_pBrush && m_bEnableUpdates)
		{
			m_bEnableUpdates = false;
			m_pBrush->Init(this);
			m_bEnableUpdates = true;
		}
		return 0;
	}

private:
	CComBSTR m_bstrID;
	CComBSTR m_bstrParams;
	CComPtr<IRasterImageBrush> m_pBrush;
	bool m_bEnableUpdates;
	LONG m_nDragging;
	POINT m_tLastPos;
	float m_fGamma;
};
