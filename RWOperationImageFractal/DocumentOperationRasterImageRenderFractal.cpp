// DocumentOperationRasterImageRenderFractal.cpp : Implementation of CDocumentOperationRasterImageRenderFractal

#include "stdafx.h"
#include "DocumentOperationRasterImageRenderFractal.h"
#include <MultilanguageString.h>
#include <math.h>

// limits on zoom and center
// selection in gradient control (low priority to sides)

const LONG COLORSTEPCOUNT = 1024;

const OLECHAR CFGID_FRACTALTYPE[] = L"Fractal";
const OLECHAR CFGID_CENTERX[] = L"CenterX";
const OLECHAR CFGID_CENTERY[] = L"CenterY";
const OLECHAR CFGID_ZOOM[] = L"Zoom";
const OLECHAR CFGID_ASPECT[] = L"Aspect";
const OLECHAR CFGID_COLORMAP[] = L"ColorMap";
const OLECHAR CFGID_CX[] = L"CX";
const OLECHAR CFGID_CY[] = L"CY";
const OLECHAR CFGID_ITER[] = L"Iter";

#include <WTL_ColorPicker.h>
#include <ConfigCustomGUIImpl.h>


template<typename TPixel>
void RenderFractal(ULONG a_nCanvasSizeX, ULONG a_nCanvasSizeY, TPixel* a_pBuffer, ULONG a_nLinearSize, ULONG a_nMinX, ULONG a_nMinY, ULONG a_nMaxX, ULONG a_nMaxY, DWORD a_dwFractalType, float a_fZoom, double a_fCenterX, double a_fCenterY, LONG a_nAspect, float a_fCX, float a_fCY, LONG a_nIter, CGradientColorPicker::CGradient const& a_cGrad)
{
	float fGamma = 1.0f;
	CComPtr<IGlobalConfigManager> pGCM;
	RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
	if (pGCM)
	{
		CComPtr<IConfig> pConfig;
		class DECLSPEC_UUID("2CBE06C7-4847-4766-AA01-226AF52D5488")
		DesignerViewFactoryColorSwatch;
		pGCM->Config(__uuidof(DesignerViewFactoryColorSwatch), &pConfig);
		if (pConfig)
		{
			CConfigValue cVal;
			pConfig->ItemValueGet(CComBSTR(L"Gamma"), &cVal);
			if (cVal.TypeGet() == ECVTFloat)
			{
				fGamma = cVal;
				if (fGamma < 0.1f) fGamma = 1.0f; else if (fGamma > 10.0f) fGamma = 1.0f;
			}
		}
	}
	TPixel aColorMap[COLORSTEPCOUNT];
	CButtonColorPicker::SColor aColorMapFlt[COLORSTEPCOUNT];
	CGradientColorPicker::RenderGradient(a_cGrad, COLORSTEPCOUNT, 0, COLORSTEPCOUNT, aColorMapFlt);
	for (ULONG i = 0; i < COLORSTEPCOUNT; ++i)
		reinterpret_cast<DWORD*>(aColorMap)[i] = aColorMapFlt[i].ToBGRA(1.0f/fGamma);


	double fAsp = pow(10.0, a_nAspect*0.02);
	double const fScaleX = 3.0/min(a_nCanvasSizeX, a_nCanvasSizeY)*a_fZoom*fAsp;
	double const fScaleY = 3.0/min(a_nCanvasSizeX, a_nCanvasSizeY)*a_fZoom/fAsp;
	double const fOffsetX = a_fCenterX-fScaleX*a_nCanvasSizeX*0.5;
	double const fOffsetY = a_fCenterY-fScaleY*a_nCanvasSizeY*0.5;
	for (ULONG nY = a_nMinY; nY < a_nMaxY; ++nY, a_pBuffer += a_nLinearSize-a_nMaxX+a_nMinX)
	{
		double const b = fOffsetY + nY * fScaleY;
		for (ULONG nX = a_nMinX; nX < a_nMaxX; ++nX, ++a_pBuffer)
		{
			double const a = fOffsetX + nX * fScaleX;
			double tmpx, tmpy, x, y;
			if (a_dwFractalType != 0)
			{
				tmpx = x = a;
				tmpy = y = b;
			}
			else
			{
				x = 0;
				y = 0;
			}
			LONG zaehler;
			for (zaehler = 0; zaehler < a_nIter && ((x*x + y*y) < 4); ++zaehler)
			{
				double oldx=x;
				double oldy=y;
				double xx;
				if (a_dwFractalType == 0)
				{
					// Mandelbrot
					xx = x * x - y * y + a;
					y = 2.0 * x * y + b;
				}
				else if (a_dwFractalType == 1)
				{
					// Julia
					xx = x * x - y * y + a_fCX;
					y = 2.0 * x * y + a_fCY;
				}
				else if (a_dwFractalType == 2)
				{
					// Some code taken from X-Fractint
					// Barnsley M1
					double const foldxinitx = oldx * a_fCX;
					double const foldyinity = oldy * a_fCY;
					double const foldxinity = oldx * a_fCY;
					double const foldyinitx = oldy * a_fCX;
					// orbit calculation
					if (oldx >= 0)
					{
						xx = (foldxinitx - a_fCX - foldyinity);
						y = (foldyinitx - a_fCY + foldxinity);
					}
					else
					{
						xx = (foldxinitx + a_fCX - foldyinity);
						y = (foldyinitx + a_fCY + foldxinity);
					}
				}
				else if (a_dwFractalType == 3)
				{
					// Barnsley Unnamed
					double const foldxinitx = oldx * a_fCX;
					double const foldyinity = oldy * a_fCY;
					double const foldxinity = oldx * a_fCY;
					double const foldyinitx = oldy * a_fCX;
					// orbit calculation
					if (foldxinity + foldyinitx >= 0)
					{
						xx = foldxinitx - a_fCX - foldyinity;
						y = foldyinitx - a_fCY + foldxinity;
					}
					else
					{
						xx = foldxinitx + a_fCX - foldyinity;
						y = foldyinitx + a_fCY + foldxinity;
					}
				}
				else if (a_dwFractalType == 4)
				{
					// Barnsley 1
					double const foldxinitx  = oldx * oldx;
					double const foldyinity  = oldy * oldy;
					double const foldxinity  = oldx * oldy;
					// orbit calculation
					if (oldx > 0)
					{
						xx = foldxinitx - foldyinity - 1.0;
						y = foldxinity * 2;
					}
					else
					{
						xx = foldxinitx - foldyinity -1.0 + a_fCX * oldx;
						y = foldxinity * 2;
						y += a_fCY * oldx;
					}
				}
				else if (a_dwFractalType == 5)
				{
					// Spider(XAXIS) { c=z=pixel: z=z*z+c; c=c/2+z, |z|<=4 }
					xx = x*x - y*y + tmpx + a_fCX;
					y = 2 * oldx * oldy + tmpy +a_fCY;
					tmpx = tmpx/2 + xx;
					tmpy = tmpy/2 + y;
				}
				else if (a_dwFractalType == 6)
				{
					// ManOWarfpFractal()
					xx = x*x - y*y + tmpx + a_fCX;
					y = 2.0 * x * y + tmpy + a_fCY;
					tmpx = oldx;
					tmpy = oldy;
				}
				else if (a_dwFractalType == 7)
				{
					// Lambda
					double tempsqrx=x*x;
					double tempsqry=y*y;
					tempsqrx = oldx - tempsqrx + tempsqry;
					tempsqry = -(oldy * oldx);
					tempsqry += tempsqry + oldy;
					xx = a_fCX * tempsqrx - a_fCY * tempsqry;
					y = a_fCX * tempsqry + a_fCY * tempsqrx;
				}
				else if (a_dwFractalType == 8)
				{
					// Sierpinski
					xx = oldx + oldx;
					y = oldy + oldy;
					if(oldy > .5)
						y = y - 1;
					else if (oldx > .5)
						xx = xx - 1;
				}
				x = xx;
			}
			if (false/*useloglog*/)
			{
				double adjust = log (log (x * x + y * y) / 2.0) / log (2.0);
				int color = (int) (((zaehler - adjust) * (COLORSTEPCOUNT - 1)) / a_nIter);
				*a_pBuffer = aColorMap[color];
			}
			else
			{
				*a_pBuffer = aColorMap[(zaehler*(COLORSTEPCOUNT-1)+(a_nIter>>1))/a_nIter];
			}
		}
	}
}


class CXYCtrl :
	public CWindowImpl<CXYCtrl>,
	public CThemeImpl<CXYCtrl>
{
public:
	CXYCtrl() : m_nSizeX(0), m_nSizeY(0)
	{
		m_tXY.first = m_tXY.second = 0.0f;
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	enum { CXY_CHANGED = 1 };


	DECLARE_WND_CLASS_EX(_T("FraxtalXYCtrl"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CXYCtrl)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocusChange)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnFocusChange)
		CHAIN_MSG_MAP(CThemeImpl<CXYCtrl>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	END_MSG_MAP()

	// operations
public:
	std::pair<float, float> const& GetXY()
	{
		return m_tXY;
	}
	void SetXY(std::pair<float, float> const& a_tXY)
	{
		if (m_tXY != a_tXY && m_hWnd)
		{
			InvalidateThumb(m_tXY);
			InvalidateThumb(a_tXY);
		}
		m_tXY = a_tXY;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		HDC hDC = GetDC();
		m_nHalfSize = (GetDeviceCaps(hDC, LOGPIXELSY)+12)/24;
		ReleaseDC(hDC);
		return 0;
	}

	LRESULT OnFocusChange(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		InvalidateThumb();
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT r;
		GetClientRect(&r);
		SetXY(std::make_pair(float(int(a_lParam<<16)>>16)/float(r.right-1), float(int(a_lParam)>>16)/float(r.bottom-1)));
		SetCapture();
		SendNotification();
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			RECT r;
			GetClientRect(&r);
			SetXY(std::make_pair(float(int(a_lParam<<16)>>16)/float(r.right-1), float(int(a_lParam)>>16)/float(r.bottom-1)));
			SendNotification();
		}
		return 0;
	}

	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		DWORD dwStyle = GetStyle();
		bool bFocus = m_hWnd == GetFocus();

		RECT r;
		PAINTSTRUCT ps;
		HDC hDC = ::BeginPaint(m_hWnd, &ps);
		GetClientRect(&r);

		if (dwStyle&WS_DISABLED)
		{
			FillRect(hDC, &r, (HBRUSH)(COLOR_3DFACE+1));
			COLORREF clrDot = GetSysColor(COLOR_3DSHADOW);
			for (LONG i = 0; i < r.right; i += 2)
				SetPixel(hDC, i, r.bottom>>1, clrDot);
			for (LONG i = 0; i < r.bottom; i += 2)
				SetPixel(hDC, r.right>>1, i, clrDot);
		}
		else
		{
			FillRect(hDC, &r, (HBRUSH)(COLOR_WINDOW+1));
			COLORREF clrBG = GetSysColor(COLOR_WINDOW);
			COLORREF clrFG = GetSysColor(COLOR_WINDOWTEXT);
			COLORREF clrDot = RGB((GetRValue(clrBG)>>1)+(GetRValue(clrFG)>>1), (GetGValue(clrBG)>>1)+(GetGValue(clrFG)>>1), (GetBValue(clrBG)>>1)+(GetBValue(clrFG)>>1));
			for (LONG i = 0; i < r.right; i += 2)
				SetPixel(hDC, i, r.bottom>>1, clrDot);
			for (LONG i = 0; i < r.bottom; i += 2)
				SetPixel(hDC, r.right>>1, i, clrDot);

			float x = m_tXY.first;
			float y = m_tXY.second;
			int xs = (int)(x*(r.right-1)), ys = (int)(y*(r.bottom-1));
			Ellipse(hDC, xs-m_nHalfSize, ys-m_nHalfSize, xs+1+m_nHalfSize, ys+1+m_nHalfSize);
			SetPixel(hDC, xs, ys, 0);
		}

		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT rc;
		GetClientRect(&rc);
		float fDX = 1.0f/(rc.right-1);
		float fDY = 1.0f/(rc.bottom-1);
		float fX = m_tXY.first;
		float fY = m_tXY.second;
		switch (a_wParam)
		{
		case VK_LEFT:
			if (fX <= 0.0f)
				return 0;
			fX = fX > fDX ? fX-fDX : 0.0f;
			break;
		case VK_RIGHT:
			if (fX >= 1.0f)
				return 0;
			fX = fX+fDX < 1.0f ? fX+fDX : 1.0f;
			break;
		case VK_UP:
			if (fY <= 0.0f)
				return 0;
			fY = fY > fDY ? fY-fDY : 0.0f;
			break;
		case VK_DOWN:
			if (fY >= 1.0f)
				return 0;
			fY = fY+fDY < 1.0f ? fY+fDY : 1.0f;
			break;
		}
		SetXY(std::make_pair(fX, fY));
		SendNotification();
		return 0;
	}

	LRESULT OnGetDlgCode(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return DLGC_WANTARROWS;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		SetFocus();
		return MA_ACTIVATE;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

private:
	void InvalidateThumb()
	{
		InvalidateThumb(m_tXY);
	}
	void InvalidateThumb(std::pair<float, float> const& a_tXY)
	{
		RECT r;
		GetClientRect(&r);
		LONG nMaxX = r.right;
		LONG nMaxY = r.bottom;
		float x = a_tXY.first;
		float y = a_tXY.second;
		r.right = (r.right-1)*x;
		r.bottom = (r.bottom-1)*y;
		r.left = r.right-m_nHalfSize-2;
		r.right += m_nHalfSize+3;
		r.top = r.bottom-m_nHalfSize-2;
		r.bottom += m_nHalfSize+3;
		if (r.left < 0) r.left = 0;
		if (r.top < 0) r.top = 0;
		if (r.right > nMaxX) r.right = nMaxX;
		if (r.bottom > nMaxY) r.bottom = nMaxY;
		InvalidateRect(&r, FALSE);
	}

	void SendNotification()
	{
		HWND hPar = GetParent();
		NMHDR nm = {m_hWnd, GetWindowLong(GWL_ID), CXY_CHANGED};
		SendMessage(hPar, WM_NOTIFY, nm.idFrom, (LPARAM)&nm);
	}

private:
	int m_nSizeX;
	int m_nSizeY;
	std::pair<float, float> m_tXY;
	int m_nHalfSize;
};

class CBoundsCtrl :
	public CWindowImpl<CBoundsCtrl>,
	public CThemeImpl<CBoundsCtrl>
{
public:
	CBoundsCtrl() :
		m_dwFractalType(0), m_fZoom(1.0f), m_fCenterX(0.0), m_fCenterY(0.0),
		m_nAspect(0), m_fCX(0.5f), m_fCY(0.5f), m_nIter(50),
		m_nCanvasSizeX(0), m_nCanvasSizeY(0),
		m_hCursor(NULL), m_nLastX(0), m_nLastY(0), m_nWheelDelta(0)
	{
		m_cGrad[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
		m_cGrad[0xffff] = CButtonColorPicker::SColor(1.0f, 1.0f, 1.0f, 1.0f);
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	enum { CBOUNDS_CHANGED = 1 };
	struct SBounds
	{
		double fCenterX;
		double fCenterY;
		float fZoom;
	};
	struct SBoundsNotify
	{
		NMHDR hdr;
		SBounds bounds;
	};


	DECLARE_WND_CLASS_EX(_T("FractalBoundsCtrl"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_WINDOW);

	BEGIN_MSG_MAP(CBoundsCtrl)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		CHAIN_MSG_MAP(CThemeImpl<CBoundsCtrl>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_ENABLE, OnEnable)
		MESSAGE_HANDLER(WM_GETDLGCODE, OnGetDlgCode)
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
	END_MSG_MAP()

	// operations
public:
	float GetZoom() { return m_fZoom; }
	double GetCenterX() { return m_fCenterX; }
	double GetCenterY() { return m_fCenterY; }
	void SetFractalProps(DWORD a_dwFractalType, double a_fCenterX, double a_fCenterY, float a_fZoom, LONG a_nAspect, float a_fCX, float a_fCY, LONG a_nIter, CGradientColorPicker::CGradient& a_cGrad)
	{
		if (m_dwFractalType != a_dwFractalType || a_fCenterX != m_fCenterX || a_fCenterY != m_fCenterY || a_fZoom != m_fZoom ||
			a_nAspect != m_nAspect || a_fCX != m_fCX || a_fCY != m_fCY || a_nIter != m_nIter || a_cGrad != m_cGrad)
		{
			m_dwFractalType = a_dwFractalType;
			m_fCenterX = a_fCenterX;
			m_fCenterY = a_fCenterY;
			m_fZoom = a_fZoom;
			m_nAspect = a_nAspect;
			m_fCX = a_fCX;
			m_fCY = a_fCY;
			m_nIter = a_nIter;
			m_cGrad = a_cGrad;
			Invalidate(FALSE);
		}
	}
	void SetImageSize(ULONG a_nSizeX, ULONG a_nSizeY)
	{
		if (m_hWnd && (a_nSizeX != m_nCanvasSizeX || a_nSizeY != m_nCanvasSizeY))
			Invalidate(FALSE);
		m_nCanvasSizeX = a_nSizeX;
		m_nCanvasSizeY = a_nSizeY;
	}

	// handlers
public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_hCursor = LoadCursor(NULL, IDC_SIZEALL);
		return 0;
	}
	LRESULT OnSetCursor(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		SetCursor(m_hCursor);
		return 0;
	}

	LRESULT OnLButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (GetCapture() == m_hWnd)
		{
			ReleaseCapture();
		}
		return 0;
	}

	LRESULT OnLButtonDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nLastX = GET_X_LPARAM(a_lParam);
		m_nLastY = GET_Y_LPARAM(a_lParam);
		SetCapture();
		return 0;
	}

	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LONG nX = GET_X_LPARAM(a_lParam);
		LONG nY = GET_Y_LPARAM(a_lParam);
		if (GetCapture() == m_hWnd && (nX != m_nLastX || nY != m_nLastY))
		{
			RECT r;
			GetClientRect(&r);
			LONG nSX = r.right;
			LONG nSY = r.bottom;
			if (m_nCanvasSizeX && m_nCanvasSizeY)
				if (m_nCanvasSizeX*nSY > m_nCanvasSizeY*nSX)
					nSY = nSX*m_nCanvasSizeY/m_nCanvasSizeX;
				else
					nSX = nSY*m_nCanvasSizeX/m_nCanvasSizeY;

			LONG nSize = min(nSX, nSY);
			if (nSize <= 0 ) nSize = 1;
			double fAsp = pow(10.0, m_nAspect*0.02);
			SBounds sBounds =
			{
				m_fCenterX - (nX-m_nLastX)*3.0*m_fZoom/nSize*fAsp,
				m_fCenterY - (nY-m_nLastY)*3.0*m_fZoom/nSize/fAsp,
				m_fZoom
			};
			if (sBounds.fCenterX > 3.0) sBounds.fCenterX = 3.0; else if (sBounds.fCenterX < -3.0) sBounds.fCenterX = -3.0;
			if (sBounds.fCenterY > 3.0) sBounds.fCenterY = 3.0; else if (sBounds.fCenterY < -3.0) sBounds.fCenterY = -3.0;
			SendNotification(&sBounds);
		}
		m_nLastX = nX;
		m_nLastY = nY;
		return 0;
	}

	LRESULT OnMouseWheel(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(a_wParam);

		// preserve point under mouse during zoom
		POINT tPos = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
		ScreenToClient(&tPos);

		RECT r;
		GetClientRect(&r);
		LONG nSX = r.right;
		LONG nSY = r.bottom;
		if (m_nCanvasSizeX && m_nCanvasSizeY)
			if (m_nCanvasSizeX*nSY > m_nCanvasSizeY*nSX)
				nSY = nSX*m_nCanvasSizeY/m_nCanvasSizeX;
			else
				nSX = nSY*m_nCanvasSizeX/m_nCanvasSizeY;
		LONG nSize = min(nSX, nSY);
		if (nSize <= 0 ) nSize = 1;
		double fAsp = pow(10.0, m_nAspect*0.02);

		float fZoom = m_fZoom;
		bool bModified = false;
		while (m_nWheelDelta >= WHEEL_DELTA)
		{
			fZoom *= 0.7071067812f; // 1.0f/sqrf(2.0f);
			m_nWheelDelta -= WHEEL_DELTA;
			bModified = true;
		}
		while ((-m_nWheelDelta) >= WHEEL_DELTA)
		{
			fZoom *= 1.41421356f; // sqrf(2.0f);
			if (fZoom > 2.0f) fZoom = 2.0f;
			m_nWheelDelta += WHEEL_DELTA;
			bModified = true;
		}

		if (bModified)
		{
			SBounds sBounds =
			{
				m_fCenterX + (tPos.x-0.5*r.right)*3.0*(m_fZoom-fZoom)/nSize*fAsp,
				m_fCenterY + (tPos.y-0.5*r.bottom)*3.0*(m_fZoom-fZoom)/nSize/fAsp,
				fZoom
			};
			if (sBounds.fCenterX > 3.0) sBounds.fCenterX = 3.0; else if (sBounds.fCenterX < -3.0) sBounds.fCenterX = -3.0;
			if (sBounds.fCenterY > 3.0) sBounds.fCenterY = 3.0; else if (sBounds.fCenterY < -3.0) sBounds.fCenterY = -3.0;
			SendNotification(&sBounds);
		}

		return 0;
	}

	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		RECT r;
		PAINTSTRUCT ps;
		HDC hDC = ::BeginPaint(m_hWnd, &ps);
		GetClientRect(&r);

		CAutoVectorPtr<BYTE> cBuffer(new BYTE[r.right*r.bottom*4]);
		CGradientColorPicker::CGradient cGrad = m_cGrad;
		COLORREF clrWin = GetSysColor(COLOR_WINDOW);
		float fWinR = GetRValue(clrWin)/255.0f;
		float fWinG = GetGValue(clrWin)/255.0f;
		float fWinB = GetBValue(clrWin)/255.0f;
		float fGamma = 1.0f;
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		if (pGCM)
		{
			CComPtr<IConfig> pConfig;
			class DECLSPEC_UUID("2CBE06C7-4847-4766-AA01-226AF52D5488")
			DesignerViewFactoryColorSwatch;
			pGCM->Config(__uuidof(DesignerViewFactoryColorSwatch), &pConfig);
			if (pConfig)
			{
				CConfigValue cVal;
				pConfig->ItemValueGet(CComBSTR(L"Gamma"), &cVal);
				if (cVal.TypeGet() == ECVTFloat)
				{
					fGamma = cVal;
					if (fGamma < 0.1f) fGamma = 1.0f; else if (fGamma > 10.0f) fGamma = 1.0f;
				}
			}
		}
		for (CGradientColorPicker::CGradient::iterator i = cGrad.begin(); i != cGrad.end(); ++i)
			if (i->second.fA < 1.0f)
			{
				i->second.fR = powf(powf(i->second.fR, fGamma)*i->second.fA + powf(fWinR, fGamma)*(1.0f-i->second.fA), 1.0f/fGamma);
				i->second.fG = powf(powf(i->second.fG, fGamma)*i->second.fA + powf(fWinG, fGamma)*(1.0f-i->second.fA), 1.0f/fGamma);
				i->second.fB = powf(powf(i->second.fB, fGamma)*i->second.fA + powf(fWinB, fGamma)*(1.0f-i->second.fA), 1.0f/fGamma);
				i->second.fA = 1.0f;
			}

		if (m_nCanvasSizeX && m_nCanvasSizeY)
		{
			COLORREF clrBG = GetSysColor(COLOR_WINDOW);
			if (m_nCanvasSizeX*r.bottom > m_nCanvasSizeY*r.right)
			{
				ULONG nY = r.right*m_nCanvasSizeY/m_nCanvasSizeX;
				RenderFractal(r.right, nY, reinterpret_cast<TRasterImagePixel*>(cBuffer.m_p)+((r.bottom-nY)>>1)*r.right, r.right, 0, 0, r.right, nY, m_dwFractalType, m_fZoom, m_fCenterX, m_fCenterY, m_nAspect, m_fCX, m_fCY, m_nIter, cGrad);
				std::fill_n(reinterpret_cast<COLORREF*>(cBuffer.m_p), ((r.bottom-nY)>>1)*r.right, clrBG);
				std::fill_n(reinterpret_cast<COLORREF*>(cBuffer.m_p)+(((r.bottom-nY)>>1)+nY)*r.right, (r.bottom-((r.bottom-nY)>>1)-nY)*r.right, clrBG);
			}
			else
			{
				ULONG nX = r.bottom*m_nCanvasSizeX/m_nCanvasSizeY;
				RenderFractal(nX, r.bottom, reinterpret_cast<TRasterImagePixel*>(cBuffer.m_p)+((r.right-nX)>>1), r.right, 0, 0, nX, r.bottom, m_dwFractalType, m_fZoom, m_fCenterX, m_fCenterY, m_nAspect, m_fCX, m_fCY, m_nIter, cGrad);
				COLORREF *p = reinterpret_cast<COLORREF*>(cBuffer.m_p);
				ULONG const n1 = (r.right-nX)>>1;
				ULONG const n2 = r.right-nX-n1;
				for (LONG y = 0; y < r.bottom; ++y)
				{
					std::fill_n(p, n1, clrBG);
					std::fill_n(p+n1+nX, n2, clrBG);
					p += r.right;
				}
			}
		}
		else
		{
			RenderFractal(r.right, r.bottom, reinterpret_cast<TRasterImagePixel*>(cBuffer.m_p), r.right, 0, 0, r.right, r.bottom, m_dwFractalType, m_fZoom, m_fCenterX, m_fCenterY, m_nAspect, m_fCX, m_fCY, m_nIter, cGrad);
		}

		BITMAPINFO bi;
		bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth=r.right;
		bi.bmiHeader.biHeight=-r.bottom;
		bi.bmiHeader.biPlanes=1;
		bi.bmiHeader.biBitCount=32;
		bi.bmiHeader.biCompression=BI_RGB;
		bi.bmiHeader.biSizeImage=0;
		bi.bmiHeader.biXPelsPerMeter=1000;
		bi.bmiHeader.biYPelsPerMeter=1000;
		bi.bmiHeader.biClrUsed=0;
		bi.bmiHeader.biClrImportant=0;
		SetDIBitsToDevice(hDC, 0,0, r.right, r.bottom, 0, 0, 0, r.bottom, cBuffer.m_p, &bi, DIB_RGB_COLORS);

		::EndPaint(m_hWnd, &ps);
		return 0;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		int iDX = 0;
		int iDY = 0;
		float fZoom = m_fZoom;
		switch (a_wParam)
		{
		case VK_LEFT:
			iDX = -1;
			break;
		case VK_RIGHT:
			iDX = 1;
			break;
		case VK_UP:
			iDY = -1;
			break;
		case VK_DOWN:
			iDY = 1;
			break;
		case VK_ADD:
			fZoom *= 0.84089641525f;
			break;
		case VK_SUBTRACT:
			fZoom *= 1.189207115f;
			if (fZoom > 2.0f) fZoom = 2.0f;
			break;
		default:
			a_bHandled = FALSE;
			return 0;
		}
		RECT r;
		GetClientRect(&r);
		LONG nSX = r.right;
		LONG nSY = r.bottom;
		if (m_nCanvasSizeX && m_nCanvasSizeY)
			if (m_nCanvasSizeX*nSY > m_nCanvasSizeY*nSX)
				nSY = nSX*m_nCanvasSizeY/m_nCanvasSizeX;
			else
				nSX = nSY*m_nCanvasSizeX/m_nCanvasSizeY;

		LONG nSize = min(nSX, nSY);
		if (nSize <= 0 ) nSize = 1;
		double fAsp = pow(10.0, m_nAspect*0.02);
		SBounds sBounds =
		{
			m_fCenterX - iDX*3.0*m_fZoom/nSize*fAsp,
			m_fCenterY - iDY*3.0*m_fZoom/nSize/fAsp,
			fZoom
		};
		if (sBounds.fCenterX > 3.0) sBounds.fCenterX = 3.0; else if (sBounds.fCenterX < -3.0) sBounds.fCenterX = -3.0;
		if (sBounds.fCenterY > 3.0) sBounds.fCenterY = 3.0; else if (sBounds.fCenterY < -3.0) sBounds.fCenterY = -3.0;
		SendNotification(&sBounds);
		return 0;
	}

	LRESULT OnGetDlgCode(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return DLGC_WANTALLKEYS;
	}

	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
		if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
			return lRes;
		SetFocus();
		return MA_ACTIVATE;
	}

	LRESULT OnEnable(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		Invalidate(FALSE);
		return 0;
	}

private:
	void SendNotification(SBounds const* a_pBounds)
	{
		HWND hPar = GetParent();
		SBoundsNotify nm = {{m_hWnd, GetWindowLong(GWL_ID), CBOUNDS_CHANGED}, *a_pBounds};
		SendMessage(hPar, WM_NOTIFY, nm.hdr.idFrom, (LPARAM)&nm);
	}

private:
	HCURSOR m_hCursor;
	LONG m_nLastX;
	LONG m_nLastY;
	LONG m_nWheelDelta;

	// size of the current image
	ULONG m_nCanvasSizeX;
	ULONG m_nCanvasSizeY;

	// fractal properties
	DWORD m_dwFractalType;
	float m_fZoom;
	double m_fCenterX;
	double m_fCenterY;
	LONG m_nAspect;
	float m_fCX;
	float m_fCY;
	LONG m_nIter;
	CGradientColorPicker::CGradient m_cGrad;
};

void ParseGradient(LPWSTR psz, CGradientColorPicker::CGradient& cGrad)
{
	while (*psz)
	{
		LPWSTR pNext = psz;
		double fPos = wcstod(psz, &pNext);
		if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
		float fR = wcstod(psz, &pNext);
		if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
		float fG = wcstod(psz, &pNext);
		if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
		float fB = wcstod(psz, &pNext);
		if (pNext == psz || *pNext != L',') break; else psz = pNext+1;
		float fA = wcstod(psz, &pNext);
		if (pNext == psz) break; else if (*pNext == L';') psz = pNext+1; else psz = pNext;
		cGrad[WORD(fPos)] = CButtonColorPicker::SColor(fR, fG, fB, fA);
	}
	if (cGrad.size() < 2)
	{
		cGrad.clear();
		cGrad[WORD(0)] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
		cGrad[WORD(0xffff)] = CButtonColorPicker::SColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

class ATL_NO_VTABLE CConfigGUIRenderFractal :
	public CCustomConfigResourcelessWndImpl<CConfigGUIRenderFractal>,
	public CDialogResize<CConfigGUIRenderFractal>
{
public:
	CConfigGUIRenderFractal()
	{
	}

	enum
	{
		IDC_FRACTALTYPE = 200,
		IDC_FRACTALTYPE_LABEL,
		IDC_COLORMAP,
		IDC_STOPCOLOR,
		IDC_ASPECTRATIO,
		IDC_PARAMETER,
		IDC_PARAMETER_LABEL,
		IDC_PARAMETERX,
		IDC_PARAMETERY,
		IDC_ITERATIONS,
		IDC_ITERATIONS_LABEL,
		IDC_SEPLINE1,
		IDC_SEPLINE2,
		IDC_BOUNDS,
	};

	BEGIN_DIALOG_EX(0, 0, 250, 154, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Fractal type:[0405]Typ fraktálu:"), IDC_FRACTALTYPE_LABEL, 0, 106, 49, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_FRACTALTYPE, 50, 104, 80, 12, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_LTEXT(_T("[0409]Parameter:[0405]Parametr:"), IDC_PARAMETER_LABEL, 140, 106, 49, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_PARAMETERX, 151, 119, 30, 12, WS_VISIBLE | WS_TABSTOP, 0);
		CONTROL_EDITTEXT(IDC_PARAMETERY, 151, 135, 30, 12, WS_VISIBLE | WS_TABSTOP, 0);
		CONTROL_PUSHBUTTON(_T(""), IDC_STOPCOLOR, 82, 139, 35, 12, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Iterations:[0405]Iterace:"), IDC_ITERATIONS_LABEL, 0, 144, 49, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ITERATIONS, 50, 142, 30, 12, WS_VISIBLE | WS_TABSTOP | ES_NUMBER, 0);
		CONTROL_CONTROL(_T(""), IDC_SEPLINE2, WC_STATIC, SS_ETCHEDVERT | WS_GROUP | WS_VISIBLE, 127, 141, 1, 6, 0)
		CONTROL_CONTROL(_T(""), IDC_SEPLINE1, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 123, 145, 6, 1, 0)
		CONTROL_CONTROL(_T(""), IDC_ASPECTRATIO, TRACKBAR_CLASS, TBS_BOTH | TBS_VERT | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 232, 0, 18, 100, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIRenderFractal)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIRenderFractal>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIRenderFractal>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_COLORMAP, CGradientColorPicker::GCPN_ACTIVESTOPCHANGED, OnGradientStopChanged)
		NOTIFY_HANDLER(IDC_STOPCOLOR, CButtonColorPicker::BCPN_SELCHANGE, OnStopColorChanged)
		NOTIFY_HANDLER(IDC_PARAMETER, CXYCtrl::CXY_CHANGED, OnParameterChanged)
		NOTIFY_HANDLER(IDC_BOUNDS, CBoundsCtrl::CBOUNDS_CHANGED, OnBoundsChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIRenderFractal)
		CONFIGITEM_COMBOBOX(IDC_FRACTALTYPE, CFGID_FRACTALTYPE)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_ASPECTRATIO, CFGID_ASPECT)
		CONFIGITEM_EDITBOX(IDC_PARAMETERX, CFGID_CX)
		CONFIGITEM_EDITBOX(IDC_PARAMETERY, CFGID_CY)
		CONFIGITEM_EDITBOX(IDC_ITERATIONS, CFGID_ITER)
		CONFIGITEM_CONTEXTHELP(IDC_BOUNDS, CFGID_ZOOM)
		CONFIGITEM_CONTEXTHELP(IDC_COLORMAP, CFGID_COLORMAP)
		CONFIGITEM_CONTEXTHELP(IDC_PARAMETER, CFGID_CX)
	END_CONFIGITEM_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIRenderFractal)
		DLGRESIZE_CONTROL(IDC_BOUNDS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_ASPECTRATIO, DLSZ_MOVE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_FRACTALTYPE_LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_FRACTALTYPE, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_PARAMETER_LABEL, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_PARAMETERX, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_PARAMETERY, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_PARAMETER, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_SEPLINE1, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_SEPLINE2, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_COLORMAP, DLSZ_SIZE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_STOPCOLOR, DLSZ_MOVE_X|DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_ITERATIONS_LABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_ITERATIONS, DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	void ExtraInitDialog()
	{
		RECT rcGrad = {0, 120, 130, 137};
		MapDialogRect(&rcGrad);
		m_wndGradient.Create(m_hWnd, &rcGrad, 0, 0, 0, IDC_COLORMAP);
		m_wndButton.SubclassWindow(GetDlgItem(IDC_STOPCOLOR));

		RECT rcPar = {190, 104, 250, 154};
		MapDialogRect(&rcPar);
		m_wndParam.Create(m_hWnd, &rcPar, 0, 0, WS_EX_CLIENTEDGE, IDC_PARAMETER);

		RECT rcBounds = {0, 0, 229, 100};
		MapDialogRect(&rcBounds);
		m_wndBounds.Create(m_hWnd, &rcBounds, 0, WS_CHILD|WS_TABSTOP|WS_VISIBLE/*|WS_HSCROLL|WS_VSCROLL*/, WS_EX_CLIENTEDGE, IDC_BOUNDS);
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		GetDocument();

		ExtraConfigNotify();
		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnStopColorChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
		m_wndGradient.SetStop(m_wndGradient.GetStop(), pClr->clr);
		return 0;
	}
	LRESULT OnGradientStopChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		CGradientColorPicker::NMGRADIENT* pGrad = reinterpret_cast<CGradientColorPicker::NMGRADIENT*>(a_pHdr);
		m_wndButton.SetColor(pGrad->clr);

		CComBSTR bstrGrad;
		CGradientColorPicker::CGradient cGradient = m_wndGradient.GetGradient();
		for (CGradientColorPicker::CGradient::const_iterator i = cGradient.begin(); i != cGradient.end(); ++i)
		{
			wchar_t psz[128];
			swprintf(psz, L"%i,%g,%g,%g,%g;", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstrGrad.Append(psz);
		}
		CConfigValue cVal(bstrGrad.m_str);
		CComBSTR bstrID(CFGID_COLORMAP);
		M_Config()->ItemValuesSet(1, &(bstrID.m_str), cVal);
		return 0;
	}
	LRESULT OnParameterChanged(int, LPNMHDR, BOOL&)
	{
		CComBSTR bstrCX(CFGID_CX);
		CComBSTR bstrCY(CFGID_CY);
		BSTR aIDs[2] = {bstrCX, bstrCY};
		TConfigValue aVals[2];
		aVals[0].eTypeID = ECVTFloat;
		aVals[0].fVal = m_wndParam.GetXY().first*2.0f-1.0f;
		aVals[1].eTypeID = ECVTFloat;
		aVals[1].fVal = -m_wndParam.GetXY().second*2.0f+1.0f;
		M_Config()->ItemValuesSet(2, aIDs, aVals);
		return 0;
	}
	LRESULT OnBoundsChanged(int, LPNMHDR a_pHdr, BOOL&)
	{
		CBoundsCtrl::SBoundsNotify const* pBHdr = reinterpret_cast<CBoundsCtrl::SBoundsNotify const*>(a_pHdr);
		CComBSTR bstrCenterX(CFGID_CENTERX);
		CComBSTR bstrCenterY(CFGID_CENTERY);
		CComBSTR bstrZoom(CFGID_ZOOM);
		BSTR aIDs[3] = {bstrCenterX, bstrCenterY, bstrZoom};
		TConfigValue aVals[3];
		aVals[0].eTypeID = ECVTString;
		wchar_t szX[32];
		_swprintf(szX, L"%g", pBHdr->bounds.fCenterX);
		CComBSTR bstrX(szX);
		aVals[0].bstrVal = bstrX;
		aVals[1].eTypeID = ECVTString;
		wchar_t szY[32];
		_swprintf(szY, L"%g", pBHdr->bounds.fCenterY);
		CComBSTR bstrY(szY);
		aVals[1].bstrVal = bstrY;
		aVals[2].eTypeID = ECVTFloat;
		aVals[2].fVal = pBHdr->bounds.fZoom;
		M_Config()->ItemValuesSet(3, aIDs, aVals);
		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_wndGradient.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_COLORMAP), &cVal);
			CGradientColorPicker::CGradient cGrad;
			ParseGradient(cVal, cGrad);
			CGradientColorPicker::CGradient cOld = m_wndGradient.GetGradient();
			if (cOld != cGrad)
			{
				m_wndGradient.SetGradient(cGrad);
				m_wndButton.SetColor(m_wndGradient.GetStopColor(m_wndGradient.GetStop()));
			}
		}
		if (m_wndParam.m_hWnd)
		{
			CConfigValue cX;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CX), &cX);
			CConfigValue cY;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CY), &cY);
			CComPtr<IConfigItemSimple> pInfo;
			M_Config()->ItemGetUIInfo(CComBSTR(CFGID_CX), __uuidof(IConfigItemSimple), reinterpret_cast<void**>(&pInfo));
			m_wndParam.SetXY(std::make_pair(cX.operator float()*0.5f+0.5f, -cY.operator float()*0.5f+0.5f));
			m_wndParam.EnableWindow(pInfo == NULL || pInfo->IsEnabled() == S_OK);
		}
		if (m_wndBounds.m_hWnd)
		{
			CConfigValue cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_FRACTALTYPE), &cVal);
			DWORD dwFractalType = cVal.operator LONG();
			M_Config()->ItemValueGet(CComBSTR(CFGID_CENTERX), &cVal);
			LPWSTR pNext;
			double fCenterX = wcstod(cVal, &pNext);
			M_Config()->ItemValueGet(CComBSTR(CFGID_CENTERY), &cVal);
			double fCenterY = wcstod(cVal, &pNext);
			M_Config()->ItemValueGet(CComBSTR(CFGID_ZOOM), &cVal);
			float fZoom = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_ASPECT), &cVal);
			LONG nAspect = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CX), &cVal);
			float fCX = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CY), &cVal);
			float fCY = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_ITER), &cVal);
			LONG nIter = cVal;
			M_Config()->ItemValueGet(CComBSTR(CFGID_COLORMAP), &cVal);
			BSTR bstr = cVal;
			CGradientColorPicker::CGradient cGrad;
			ParseGradient(bstr, cGrad);
			m_wndBounds.SetFractalProps(dwFractalType, fCenterX, fCenterY, fZoom, nAspect, fCX, fCY, nIter, cGrad);
		}
	}

	void GetDocument()
	{
		try
		{
			CComPtr<IDocument> pDoc;
			GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
			CComPtr<IDocumentImage> pDI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
			if (pDI)
			{
				TImageSize tIS = {0, 0};
				pDI->CanvasGet(&tIS, NULL, NULL, NULL, NULL);
				if (tIS.nX*tIS.nY)
					m_wndBounds.SetImageSize(tIS.nX, tIS.nY);
			}
		}
		catch (...)
		{
		}
	}

private:
	CGradientColorPicker m_wndGradient;
	CButtonColorPicker m_wndButton;
	CXYCtrl m_wndParam;
	CBoundsCtrl m_wndBounds;
};


// CDocumentOperationRasterImageRenderFractal

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Render Fractal[0405]Rastrový obrázek - renderovat fraktál");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Fractal[0405]Fraktál";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FRACTALTYPE), &cType);
		switch (cType.operator LONG())
		{
		case 0L: pszName = L"[0409]Mandelbrot set[0405]Mandelbrotova množnina"; break;
		case 1L: pszName = L"[0409]Julia set[0405]Juliova množnina"; break;
		case 4L: pszName = L"[0409]Barnsley fractal[0405]Barnsleyův fraktál"; break;
		case 5L: pszName = L"[0409]Spider fractal[0405]Spider fraktál"; break;
		case 6L: pszName = L"[0409]Man'o'war fractal[0405]Man'o'war fraktál"; break;
		case 7L: pszName = L"[0409]Lambda fractal[0405]Lambda fraktál"; break;
		}
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_FRACTALTYPE(CFGID_FRACTALTYPE);
		pCfgInit->ItemIns1ofN(cCFGID_FRACTALTYPE, CMultiLanguageString::GetAuto(L"[0409]Fractal type[0405]Typ fraktálu"), CMultiLanguageString::GetAuto(L"[0409]Fractal type controls how the picture is generated. After changing fractal type, zoom out to have a look at the entire image.[0405]Typ fraktálu určuje jak je obrázek generován. Po změně typu fraktálu snižte zvětšení na minumum, abyste viděli celý fraktál."), CConfigValue(0L), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(0L), CMultiLanguageString::GetAuto(L"[0409]Mandelbrot set[0405]Mandelbrotova množnina"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]Julia set[0405]Juliova množnina"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(4L), CMultiLanguageString::GetAuto(L"[0409]Barnsley fractal[0405]Barnsleyův fraktál"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(5L), CMultiLanguageString::GetAuto(L"[0409]Spider fractal[0405]Spider fraktál"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(6L), CMultiLanguageString::GetAuto(L"[0409]Man'o'war fractal[0405]Man'o'war fraktál"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_FRACTALTYPE, CConfigValue(7L), CMultiLanguageString::GetAuto(L"[0409]Lambda fractal[0405]Lambda fraktál"), 0, NULL);
		CComPtr<ILocalizedString> pBounds = CMultiLanguageString::GetAuto(L"[0409]Drag the picture with your mouse to move it, zoom in and out with your mouse wheel. Alternatively use arrows, + and - keys.[0405]Táhněte obrázek myší pro posun, přibližujte nebo oddalujte kolečkem myši. Alternativně použijte šipky a klávesy + a -.");
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CENTERX), CMultiLanguageString::GetAuto(L"[0409]Center X[0405]Střed X"), pBounds, CConfigValue(L"0.0"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CENTERY), CMultiLanguageString::GetAuto(L"[0409]Center Y[0405]Střed Y"), pBounds, CConfigValue(L"0.0"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_ZOOM), CMultiLanguageString::GetAuto(L"[0409]Zoom[0405]Zvětšení"), pBounds, CConfigValue(1.0f), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_ASPECT), CMultiLanguageString::GetAuto(L"[0409]Aspect ratio[0405]Poměr stran"), CMultiLanguageString::GetAuto(L"[0409]Change the scaling in vertical and horizontal directions.[0405]Nastavte škálování ve vertikálním a horizontálním směru."), CConfigValue(0L), NULL, CConfigValue(-50L), CConfigValue(50L), CConfigValue(1L), 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_FRACTALTYPE;
		tCond.eConditionType = ECOCNotEqual;
		tCond.tValue.eTypeID = ECVTInteger;
		tCond.tValue.iVal = 0;
		CComPtr<ILocalizedString> pParam = CMultiLanguageString::GetAuto(L"[0409]A constant used when evaluating the fractals (except Mandelbrot set). Change the value by clicking in the square or by entering numbers directly.[0405]Konstanta použitá při výpočtu fraktálu (mimo Mandelbrotovu množinu). Změňte hodnotu kliknutím ve čtverci nebo zadáním čísel.");
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CX), CMultiLanguageString::GetAuto(L"[0409]CX[0405]CX"), pParam, CConfigValue(0.5f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_CY), CMultiLanguageString::GetAuto(L"[0409]CY[0405]CY"), pParam, CConfigValue(-0.5f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_ITER), CMultiLanguageString::GetAuto(L"[0409]Iterations[0405]Iterace"), CMultiLanguageString::GetAuto(L"[0409]Maximum number of computation steps and hence colors in the final picture.[0405]Maximální počet kroků výpočtu a tedy barev ve výsledném obrázku."), CConfigValue(50L), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_COLORMAP), CMultiLanguageString::GetAuto(L"[0409]Color map[0405]Mapa barev"), CMultiLanguageString::GetAuto(L"[0409]Click empty space to add new stop to the gradient. Delete stop by dragging it outside. Select stop by clicking on it.[0405]Klikněte na prázdném prostoru pro přidání zastavení. Zastavení smažete odtažením ven. Vyberte zastavení kliknutím."), CConfigValue(L""), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageRenderFractal, CConfigGUIRenderFractal>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, 1, aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FRACTALTYPE), &cVal);
		DWORD dwFractalType = cVal.operator LONG();
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CENTERX), &cVal);
		LPWSTR pNext;
		double fCenterX = wcstod(cVal, &pNext);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CENTERY), &cVal);
		double fCenterY = wcstod(cVal, &pNext);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_ZOOM), &cVal);
		float fZoom = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_ASPECT), &cVal);
		LONG nAspect = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CX), &cVal);
		float fCX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CY), &cVal);
		float fCY = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_ITER), &cVal);
		LONG nIter = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_COLORMAP), &cVal);
		BSTR bstr = cVal;
		CGradientColorPicker::CGradient cGrad;
		ParseGradient(bstr, cGrad);

		TImageSize tSize = {64, 64};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		TImageSize tOrigSize = tSize;
		TRasterImageRect tR = {{0, 0}, {tSize.nX, tSize.nY}};

		ULONG nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[nPixels]);

		RenderFractal(tOrigSize.nX, tOrigSize.nY, pPixels.m_p, tSize.nX, tR.tTL.nX, tR.tTL.nY, tR.tBR.nX, tR.tBR.nY, dwFractalType, fZoom, fCenterX, fCenterY, nAspect, fCX, fCY, nIter, cGrad);

		return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, nPixels, pPixels, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Name(ILocalizedString** a_ppName)
{
	*a_ppName = new CMultiLanguageString(L"[0409]Render fractal...[0405]Renderovat fraktál...");
	return S_OK;
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Description(ILocalizedString** a_ppDesc)
{
	*a_ppDesc = new CMultiLanguageString(L"[0409]Generate a natural, organic looking image.[0405]Vygenerovat obrázek s přírodním, organickým charakterem.");
	return S_OK;
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::IconID(GUID* a_pIconID)
{
	*a_pIconID = CLSID_DocumentOperationRasterImageRenderFractal;
	return S_OK;
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentOperationRasterImageRenderFractal::Configuration(IConfig* a_pOperationCfg)
{
	// "Operation" is a hardcoded ID of the root configuration item
	// and you must set it to the GUID of the main operation.
	CComBSTR bstr1(L"Operation");
	// It is set to __uuidof(DocumentOperationShowConfig), because
	// we want a configuration dialog. If no configuration dialog
	// is necessary, just set it to GUID of your operation.
	CConfigValue cID1(__uuidof(DocumentOperationShowConfig));

	// Since I am using the "Show Configuration" operation
	// http://wiki.rw-designer.com/Operation_Display_Configuration
	// (selected in previous step), I'll have to set its internals.
	// First, the actual operation used (this one).
	CComBSTR bstr2(L"Operation\\CfgOperation");
	CConfigValue cID2(__uuidof(DocumentOperationRasterImageRenderFractal));

	// dialog window caption
	CComBSTR bstr3(L"Operation\\Caption");
	CConfigValue cID3(L"[0409]Configure Fractal Rendering[0405]Konfigurovat renderování fraktálu");

	// custom help string
	CComBSTR bstr4(L"Operation\\HelpTopic");
	CConfigValue cID4(L"[0409]Fractal renderer allows you to generate a picture with natural organic look.<br><br><a href=\"http://wiki.rw-designer.com/Raster_Image_-_Render_Fractal\">More information</a>[0405]Fractálový renderer umožní vygenerovat obrázek s přirozeným organickým charakterem.<br><br><a href=\"http://wiki.rw-designer.com/Raster_Image_-_Render_Fractal\">Více informací</a>");

	// now, set all those values
	BSTR aIDs[] = {bstr1.m_str, bstr2.m_str, bstr3.m_str, bstr4.m_str};
	TConfigValue aVals[] = {cID1, cID2, cID3, cID4};
	return a_pOperationCfg->ItemValuesSet(sizeof(aIDs)/sizeof(*aIDs), aIDs, aVals);
}

