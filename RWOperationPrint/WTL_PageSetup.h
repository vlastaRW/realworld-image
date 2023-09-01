#pragma once

#include <ContextHelpDlg.h>

void BlendWithBackground(ULONG nPix, TPixelChannel* p);
void RotateImage(int a_nImgW, int a_nImgH, CAutoVectorPtr<TPixelChannel>& a_pBuffer, bool a_bReverse);

class CPageSetupControl :
	public CWindowImpl<CPageSetupControl>,
	public CThemeImpl<CPageSetupControl>
{
public:
	CPageSetupControl() : m_nPaperX(2100), m_nPaperY(2970), m_nImgX(900), m_nImgY(600), m_nCopies(1), m_bCombine(1), m_bCenter(true), m_bAutoRotate(true), m_nBufX(0), m_nBufY(0), m_bRotated(false)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}
	~CPageSetupControl()
	{
	}

	void Init(int a_nPaperX, int a_nPaperY, RECT const& a_rcParperMargins, int a_nImgX, int a_nImgY, int a_nCopies, bool a_bCombine, bool a_bCenter, bool a_bAutoRotate, int a_nBufX, int a_nBufY, CAutoVectorPtr<TPixelChannel>& a_cBuffer)
	{
		m_nPaperX=a_nPaperX;
		m_nPaperY=a_nPaperY;
		m_nImgX=a_nImgX;
		m_nImgY=a_nImgY;
		m_nCopies=a_nCopies;
		m_rcParperMargins = a_rcParperMargins;
		m_bCombine=a_bCombine;
		m_bCenter=a_bCenter;
		m_bAutoRotate=a_bAutoRotate;
		if (a_cBuffer.m_p)
		{
			m_nBufX = a_nBufX;
			m_nBufY = a_nBufY;
			std::swap(m_cBuf.m_p, a_cBuffer.m_p);
			BlendWithBackground(m_nBufX*m_nBufY, reinterpret_cast<TPixelChannel*>(m_cBuf.m_p));
		}
		bool bRotate = false;
		if (m_bAutoRotate && GetPageCount(m_nPaperX, m_nPaperY, m_nImgX, m_nImgY, m_nCopies, m_bCombine) > GetPageCount(m_nPaperX, m_nPaperY, m_nImgY, m_nImgX, m_nCopies, m_bCombine))
		{
			// rotating image will save paper
			std::swap(m_nImgX, m_nImgY);
			bRotate = true;
		}
		if (m_cBuf.m_p)
		{
			if (m_bRotated != bRotate)
			{
				RotateImage(m_nBufX, m_nBufY, m_cBuf, m_bRotated);
				std::swap(m_nBufX, m_nBufY);
				m_bRotated = !m_bRotated;
			}
		}
		if(IsWindow())
			Invalidate();
	}

	DECLARE_WND_CLASS_EX(_T("PageSetupWndClass"), CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, COLOR_APPWORKSPACE);

	BEGIN_MSG_MAP(CPageSetupControl)
		CHAIN_MSG_MAP(CThemeImpl<CPageSetupControl>)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()


	// handlers
public:
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(&ps);
		RECT rcWin;
		GetClientRect(&rcWin);

		int width = rcWin.right - rcWin.left;
		int height = rcWin.bottom - rcWin.top;

		::FillRect(hDC, &rcWin, HBRUSH(COLOR_APPWORKSPACE+1));

		try
		{
			int nImgW = m_nImgX;
			int nImgH = m_nImgY;
			if (nImgW == 0) nImgW = 256;
			if (nImgH == 0) nImgH = 256;
			int nPageW = m_nPaperX;
			int nPageH = m_nPaperY;
			if (nPageW == 0) nPageW = nImgW;
			if (nPageH == 0) nPageH = nImgH;

			int nPageX = 0;
			int nPageY = 0;

			if(m_bCenter)
			{
				nPageY = (nImgH%nPageH) ? (nPageH-(nImgH%nPageH))>>1 : 0;
				nPageX = (nImgW%nPageW) ? (nPageW-(nImgW%nPageW))>>1 : 0;
			}

			// one image over more pages
			if(nImgH > nPageH || nImgW > nPageW)
			{
				int nPagesX = nImgW/nPageW + (nImgW%nPageW ? 1 : 0);
				int nPagesY = nImgH/nPageH + (nImgH%nPageH ? 1 : 0);
				float fImgRatioX = (float)m_nBufX/nImgW;
				float fImgRatioY = (float)m_nBufY/nImgH;
				for(int py=0; py<nPagesY; ++py)
				{
					int nImgH3 = nPageH;
					if(py == 0) nImgH3 -= nPageY;
					int nImgY = py*nPageH;
					if(nImgY > 0) nImgY -= nPageY;
					if(nImgY + nImgH3 > nImgH)
						nImgH3 = nImgH - nImgY;
					int nImgY1 = max(0, py*nPageH-nPageY)*fImgRatioY+0.5f;
					int nImgY2 = min(nImgH, (py+1)*nPageH-nPageY)*fImgRatioY+0.5f;
					if (nImgY1 == nImgY2) // for very small (1px) source images
						if (nImgY1 > 0) --nImgY1; else ++nImgY2;
					for(int px=0; px<nPagesX; ++px)
					{
						int nImgW3 = nPageW;
						if(px == 0) nImgW3 -= nPageX;
						int nImgX = px*nPageW;
						if(nImgX > 0) nImgX -= nPageX;
						if(nImgX + nImgW3 > nImgW)
							nImgW3 = nImgW - nImgX;
						int nImgX1 = max(0, px*nPageW-nPageX)*fImgRatioX+0.5f;
						int nImgX2 = min(nImgW, (px+1)*nPageW-nPageX)*fImgRatioX+0.5f;
						if (nImgX1 == nImgX2) // for very small (1px) source images
							if (nImgX1 > 0) --nImgX1; else ++nImgX2;

						RECT rcBox;
						rcBox.left = rcWin.left + px * (rcWin.right - rcWin.left) / nPagesX;
						rcBox.right = rcBox.left + (rcWin.right - rcWin.left) / nPagesX;
						rcBox.top = rcWin.top + py * (rcWin.bottom - rcWin.top) / nPagesY;
						rcBox.bottom = rcBox.top + (rcWin.bottom - rcWin.top) / nPagesY;
						DrawPage(hDC, rcBox);
						RECT rcSrc = {nImgX1, m_nBufY-nImgY2, nImgX2, m_nBufY-nImgY1};
						DrawImage(hDC, rcBox, px == 0 ? nPageX : 0, py == 0 ? nPageY : 0, nImgW3, nImgH3, rcSrc);
					}
				}
				if(m_nCopies > 1)
				{
					TCHAR psz[10];
					int nLen = _stprintf(psz, _T("%dx"), m_nCopies);
					SetBkMode(hDC, TRANSPARENT);
					DrawText(hDC, psz, nLen, &rcWin, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
				}
			}
			else
			{
				int nImagesPerPageX = nPageW / nImgW;
				int nImagesPerPageY = nPageH / nImgH;
				if (m_bCombine && m_nCopies > 1 && nImagesPerPageX*nImagesPerPageY > 1)
				{
					// multiple images per page
					nPageY = m_bCenter ? (((nPageH-(nImgH*nImagesPerPageY))>>1)/nImagesPerPageY) : 0;
					nPageX = m_bCenter ? (((nPageW-(nImgW*nImagesPerPageX))>>1)/nImagesPerPageX) : 0;
					int nPages = m_nCopies / (nImagesPerPageX*nImagesPerPageY) + ((m_nCopies % (nImagesPerPageX*nImagesPerPageY)) ? 1 : 0);
					int nCopies = 0;
					bool bLastDifferent = nPages*nImagesPerPageX*nImagesPerPageY != m_nCopies;
					int nPreviewPages = bLastDifferent && nPages > 1 ? 2 : 1;
					for(int iPage=0; iPage<nPreviewPages; ++iPage)
					{
						RECT rcBox;
						SetRect(&rcBox, rcWin.left + iPage * (float)(rcWin.right - rcWin.left)/nPreviewPages, rcWin.top, rcWin.left + (iPage+1) * (float)(rcWin.right - rcWin.left)/nPreviewPages, rcWin.bottom);
						DrawPage(hDC, rcBox);
						for(int iy=0; iy<nImagesPerPageY && nCopies<m_nCopies; ++iy)
							for(int ix=0; ix<nImagesPerPageX && nCopies<m_nCopies; ++ix)
							{
								RECT rcSrc = {0, 0, m_nBufX, m_nBufY};
								DrawImage(hDC, rcBox, ix*nImgW+(ix+ix+1)*nPageX, iy*nImgH+(iy+iy+1)*nPageY, nImgW, nImgH, rcSrc);
								++nCopies;
							}
						if (iPage == 0 && nPages != nPreviewPages)
						{
							TCHAR psz[10];
							int nLen = _stprintf(psz, _T("%dx"), nPages+1-nPreviewPages);
							SetBkMode(hDC, TRANSPARENT);
							DrawText(hDC, psz, nLen, &rcBox, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
							nCopies = (nPages+1-nPreviewPages)*nImagesPerPageX*nImagesPerPageY;
						}
					}
				}
				else
				{
					// one image over one page
					RECT rcBox = rcWin;
					DrawPage(hDC, rcBox);
					RECT rcSrc = {0, 0, m_nBufX, m_nBufY};
					DrawImage(hDC, rcBox, m_bCenter ? nPageX : 0, m_bCenter ? nPageY : 0, nImgW, nImgH, rcSrc);
					if(m_nCopies > 1)
					{
						TCHAR psz[10];
						int nLen = _stprintf(psz, _T("%dx"), m_nCopies);
						SetBkMode(hDC, TRANSPARENT);
						DrawText(hDC, psz, nLen, &rcBox, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
					}
				}
			}
		}
		catch (...)
		{
		}

		EndPaint(&ps);
		return 0;
	}
	LRESULT OnEraseBkgnd(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}

private:
	void DrawPage(HDC hDC, RECT& rc)
	{
		rc.left += 5; rc.right -= 5; rc.top += 5; rc.bottom -= 5;
		if((float)(rc.right - rc.left) / (rc.bottom - rc.top) > (float)m_nPaperX/m_nPaperY)
		{
			int dw = ((rc.right - rc.left) - (rc.bottom - rc.top) * (float)m_nPaperX/m_nPaperY) / 2;
			rc.left += dw;
			rc.right -= dw;
		}
		else
		{
			int dh = ((rc.bottom - rc.top) - (rc.right - rc.left) * (float)m_nPaperY/m_nPaperX) / 2;
			rc.top += dh;
			rc.bottom -= dh;
		}
		HBRUSH hBrPage = CreateSolidBrush(0xffffff);
		HBRUSH hBrShadow = CreateSolidBrush(0x555555);
		rc.left += 3; rc.right += 3; rc.top += 3; rc.bottom += 3;
		::FillRect(hDC, &rc, hBrShadow);
		rc.left -= 3; rc.right -= 3; rc.top -= 3; rc.bottom -= 3;
		::FillRect(hDC, &rc, hBrPage);
		DeleteObject(hBrPage);
		DeleteObject(hBrShadow);
	}
	void DrawImage(HDC hDC, RECT rc, int x, int y, int w, int h, RECT rcSrc)
	{
		int pw = rc.right - rc.left;
		int ph = rc.bottom - rc.top;
		rc.left += (float)x/m_nPaperX * pw;
		rc.top += (float)y/m_nPaperY * ph;
		rc.right = rc.left + (float)w/m_nPaperX * pw;
		rc.bottom = rc.top + (float)h/m_nPaperY * ph;
		if (m_cBuf.m_p)
		{
			BITMAPINFO tBI;
			ZeroMemory(&tBI, sizeof tBI);
			tBI.bmiHeader.biSize = sizeof tBI.bmiHeader;
			tBI.bmiHeader.biWidth = m_nBufX;
			tBI.bmiHeader.biHeight = -m_nBufY;
			tBI.bmiHeader.biPlanes = 1;
			tBI.bmiHeader.biBitCount = 32;
			tBI.bmiHeader.biCompression = BI_RGB;
			//RECT rcSrc = {x*m_nBufX/m_nImgX, y*m_nBufY/m_nImgY, (x+w)*m_nBufX/m_nImgX, (y+h)*m_nBufY/m_nImgY};
			StretchDIBits(hDC, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, rcSrc.left, rcSrc.top, rcSrc.right-rcSrc.left, rcSrc.bottom-rcSrc.top, m_cBuf.m_p, &tBI, DIB_RGB_COLORS, SRCCOPY);
		}
		else
		{
			HBRUSH hBrImage = CreateSolidBrush(0xcccccc);
			HPEN hPen = CreatePen(PS_DOT, 1, 0x555555);
			SelectObject(hDC, hPen);
			SelectObject(hDC, hBrImage);
			Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
			DeleteObject(hBrImage);
		}
	}

private:
	int m_nPaperX, m_nPaperY, m_nImgX, m_nImgY, m_nCopies;
	RECT m_rcParperMargins;
	bool m_bCombine;
	bool m_bCenter;
	bool m_bAutoRotate;

	int m_nBufX;
	int m_nBufY;
	CAutoVectorPtr<TPixelChannel> m_cBuf;
	bool m_bRotated;
};
