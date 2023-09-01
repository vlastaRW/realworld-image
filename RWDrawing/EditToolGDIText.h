
#pragma once

#include "EditTool.h"
#include "EditToolPixelMixer.h"
#include "EditToolWithCtrlDropper.h"
#include "EditToolWithBrush.h"
#include "SharedStateToolData.h"
#include <math.h>
#include <boost/spirit.hpp>
using namespace boost::spirit;


#include "EditToolTextDlg.h"


HICON GetToolIconTEXT(ULONG a_nSize);

class CEditToolGDIText :
	public CEditToolMouseInput<CEditToolGDIText>, // no direct tablet support
	public CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>,
	public CEditToolCustomOrMoveCursor<CEditToolGDIText, GetToolIconTEXT>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolGDIText, CEditToolMouseInput<CEditToolGDIText>, CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, CEditToolCustomOrMoveCursor<CEditToolGDIText, GetToolIconTEXT> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolGDIText, // T - the top level class for cross casting
		CEditToolGDIText, // TResetHandler
		CEditToolGDIText, // TDirtyHandler
		CEditToolGDIText, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, // TBrushHandler
		CEditToolGDIText, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolGDIText, CEditToolMouseInput<CEditToolGDIText>, CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, CEditToolCustomOrMoveCursor<CEditToolGDIText, GetToolIconTEXT> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolGDIText, CEditToolMouseInput<CEditToolGDIText>, CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, CEditToolCustomOrMoveCursor<CEditToolGDIText, GetToolIconTEXT> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolGDIText, CEditToolMouseInput<CEditToolGDIText>, CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, CEditToolCustomOrMoveCursor<CEditToolGDIText, GetToolIconTEXT> >, // TProcessInputHandler
		CEditToolGDIText, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolGDIText, CEditToolGDIText, CEditToolGDIText> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolGDIText() : m_bCacheValid(false), m_eState(ESClean), m_bUseSecondary(false),
		m_eRasterizationMode(ERMSmooth), m_eBlendingMode(EBMDrawOver),// m_eShapeFillMode(ESFMSolidFill),
		m_fGamma(1.0f)
	{
	}

	BEGIN_COM_MAP(CEditToolGDIText)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		EnableBrush(false);
		if (m_eState != ESClean)
		{
			m_eState = ESClean;
			M_Window()->RectangleChanged(&DirtyRect());
			M_Window()->ControlLinesChanged();
		}
		m_cTextBitmap.Free();
		m_nTextSizeX = m_nTextSizeY = 0;
		ResetDragging();
		m_bCacheValid = false;
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = m_eState != ESClean ? DirtyRect() : RECT_EMPTY;
		return m_eState != ESClean ? S_OK : S_FALSE;
	}

	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		HRESULT hRes = M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes) || m_eState == ESClean)
			return hRes;
		CAutoVectorPtr<BYTE> cMask(new BYTE[a_nSizeX*a_nSizeY]);
		if (FAILED(M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nSizeX, cMask)))
			cMask.Free();
		RECT rc = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		switch (m_eBlendingMode)
		{
		case EBMDrawOver:	RenderPixels<CPixelMixerPaintOver> (rc, a_fGamma, a_nStride, a_pBuffer, a_nSizeX, cMask); break;
		case EBMReplace:	RenderPixels<CPixelMixerReplace>   (rc, a_fGamma, a_nStride, a_pBuffer, a_nSizeX, cMask); break;
		case EBMDrawUnder:	RenderPixels<CPixelMixerPaintUnder>(rc, a_fGamma, a_nStride, a_pBuffer, a_nSizeX, cMask); break;
		}
		return S_OK;
	}


	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataText::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			if (m_eState != ESClean)
			{
				RECT rcPrev = DirtyRect();
				m_bCacheValid = false;
				RECT rcNew = DirtyRect();
				ToolSetBrush();
				if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
				if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
				if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
				if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
				M_Window()->RectangleChanged(&rcNew);
				M_Window()->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	void ColorChanged(bool a_bColor1, bool a_bColor2)
	{
		if (((a_bColor1 && !m_bUseSecondary) || (a_bColor2 && m_bUseSecondary)) && m_eState != ESClean)
		{
			M_Window()->RectangleChanged(&DirtyRect());
		}
	}
	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		if (m_eState == ESClean)
			return S_FALSE;
		m_bCacheValid = false;
		if (a_pChanged == NULL)
			return M_Window()->RectangleChanged(&DirtyRect());
		RECT rcShape = DirtyRect();
		if (rcShape.left < a_pChanged->left) rcShape.left = a_pChanged->left;
		if (rcShape.top < a_pChanged->top) rcShape.top = a_pChanged->top;
		if (rcShape.right > a_pChanged->right) rcShape.right = a_pChanged->right;
		if (rcShape.bottom > a_pChanged->bottom) rcShape.bottom = a_pChanged->bottom;
		return M_Window()->RectangleChanged(&rcShape);
	}
	void ToolSetBrush()
	{
		if (M_Brush() && m_eState != ESClean)
		{
			RECT rcShape = DirtyRect();
			TPixelCoords tCenter = {0.5f*(rcShape.left+rcShape.right), 0.5f*(rcShape.top+rcShape.bottom)};
			M_Brush()->SetShapeBounds(&tCenter, 0.5f*(rcShape.right-rcShape.left), 0.5f*(rcShape.bottom-rcShape.top), 0.0f);
		}
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (a_eRasterizationMode != m_eRasterizationMode)
		{
			m_eRasterizationMode = a_eRasterizationMode;
			m_eBlendingMode = a_eBlendingMode;
			EnableBrush(m_eState != ESClean);
			if (m_eState != ESClean)
			{
				RECT rcPrev = DirtyRect();
				m_bCacheValid = false;
				RECT rcNew = DirtyRect();
				if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
				if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
				if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
				if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
				M_Window()->RectangleChanged(&rcNew);
				M_Window()->ControlLinesChanged();
			}
		}
		else if (m_eBlendingMode != a_eBlendingMode/* || m_eShapeFillMode != a_eShapeFillMode*/)
		{
			m_eBlendingMode = a_eBlendingMode;
			//m_eShapeFillMode = a_eShapeFillMode;
			EnableBrush(m_eState != ESClean);
			if (m_eState != ESClean)
				M_Window()->RectangleChanged(&DirtyRect());
		}
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		a_pPos->fX = floorf(a_pPos->fX);
		a_pPos->fY = floorf(a_pPos->fY);

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState == ESFloating &&
			a_pPos->fX >= m_tLastPos.fX && a_pPos->fY >= m_tLastPos.fY &&
			a_pPos->fX < m_tLastPos.fX+m_nTextSizeX && a_pPos->fY < m_tLastPos.fY+m_nTextSizeY)
		{
			m_eState = ESDragging;
			m_tDelta.fX = a_pPos->fX-m_tLastPos.fX;
			m_tDelta.fY = a_pPos->fY-m_tLastPos.fY;
			return S_OK;
		}

		if (m_eState != ESClean)
		{
			ATLASSERT(0);
			//M_Window()->ApplyChanges();
			return ETPAApply|ETPAStartNew;
		}
		m_eState = ESDragging;
		m_tLastPos = *a_pPos;
		m_tDelta.fX = m_tDelta.fY = 0.0f;
		M_Window()->RectangleChanged(&DirtyRect());
		ToolSetBrush();
		M_Window()->ControlLinesChanged();
		m_bUseSecondary = a_eKeysState&ECKSShift;
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_eState = ESFloating;
		ToolSetBrush();
		EnableBrush(true);
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState != ESDragging)
			return S_FALSE;
		if (a_pPos->fX == m_tLastPos.fX && a_pPos->fY == m_tLastPos.fY)
			return S_FALSE;
		RECT rcPrev = DirtyRect();
		m_tLastPos.fX = a_pPos->fX-m_tDelta.fX;
		m_tLastPos.fY = a_pPos->fY-m_tDelta.fY;
		m_bCacheValid = false;
		RECT rcNew = DirtyRect();
		if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
		if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
		if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
		if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
		M_Window()->RectangleChanged(&rcNew);
		M_Window()->ControlLinesChanged();
		ToolSetBrush();
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_eState == ESFloating &&
			a_pPos->fX >= m_tLastPos.fX && a_pPos->fY >= m_tLastPos.fY &&
			a_pPos->fX < m_tLastPos.fX+m_nTextSizeX && a_pPos->fY < m_tLastPos.fY+m_nTextSizeY;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_eState != ESClean)
		{
			RECT rc = DirtyRect();
			a_pLines->MoveTo(rc.left, rc.top);
			a_pLines->LineTo(rc.right, rc.top);
			a_pLines->LineTo(rc.right, rc.bottom);
			a_pLines->LineTo(rc.left, rc.bottom);
			a_pLines->Close();
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}
	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (a_pMsg->message == WM_CHAR)
		{
			RECT rcPrev = DirtyRect();
			if (a_pMsg->wParam >= L' ')
			{
				m_cData.strText.push_back(wchar_t(a_pMsg->wParam));
			}
			else if (a_pMsg->wParam == L'\r')
			{
				m_cData.strText.push_back(L'\r');
				m_cData.strText.push_back(L'\n');
			}
			else if (a_pMsg->wParam == L'\b' && m_cData.strText.size() > 0)
			{
				if (m_cData.strText.size() > 1 && m_cData.strText[m_cData.strText.size()-1] == L'\n' && m_cData.strText[m_cData.strText.size()-2] == L'\r')
					m_cData.strText.resize(m_cData.strText.size()-2);
				else
					m_cData.strText.resize(m_cData.strText.size()-1);
			}
			else
			{
				return S_FALSE;
			}
			m_bCacheValid = false;
			RECT rcNew = DirtyRect();
			if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
			if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
			if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
			if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
			M_Window()->RectangleChanged(&rcNew);
			M_Window()->ControlLinesChanged();
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			M_Window()->SetState(pTmp);
			ToolSetBrush();
			return S_OK;
		}
		else if (a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_KEYUP)
		{
			if (a_pMsg->wParam == VK_RETURN && a_pMsg->hwnd)
			{
				RWHWND h = NULL;
				if (M_Window()) M_Window()->Handle(&h);
				if (a_pMsg->hwnd == h)
				{
					TranslateMessage(a_pMsg);
					//DispatchMessage(a_pMsg);
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}

private:
	enum EState
	{
		ESClean = 0,
		ESDragging,
		ESFloating
	};

	// internal methods
private:
	void PrepareCache()
	{
		CDC cDCMain;
		cDCMain.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		CDC cDC;
		cDC.CreateCompatibleDC(cDCMain);
		CFont cFont;
		cFont.CreateFont(
			-m_cData.fSize,
			0,
			0,
			0,
			m_cData.bBold ? FW_BOLD : FW_NORMAL,
			m_cData.bItalic,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			m_eRasterizationMode == ERMSmooth ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY,
			DEFAULT_PITCH|FF_DONTCARE,
			CW2CT(m_cData.lfFaceName)
			);
		cFont.Attach(cDC.SelectFont(cFont.Detach()));
		COLE2T str(m_cData.strText.c_str());
		RECT rc = {0, 0, 2048, 0};
		cDC.DrawText(str, -1, &rc, DT_LEFT|DT_TOP|DT_CALCRECT);
		TEXTMETRIC tm;
		cDC.GetTextMetrics(&tm);
		rc.right += tm.tmHeight; // try to compensate for bold italic fonts
		if (rc.right == 0 || rc.bottom == 0)
		{
			m_cTextBitmap.Free();
			m_nTextSizeX = rc.right;
			m_nTextSizeY = rc.bottom;
			m_bCacheValid = true;
			return;
		}

		CBitmap cBmp;
		cBmp.CreateCompatibleBitmap(cDCMain, rc.right, rc.bottom);
		cBmp.Attach(cDC.SelectBitmap(cBmp.Detach()));
		cDC.SetTextColor(0xffffff);
		cDC.SetBkColor(0);
		cDC.DrawText(str, -1, &rc, DT_LEFT|DT_TOP);

		cFont.Attach(cDC.SelectFont(cFont.Detach()));

		size_t nLinearSize = (rc.right*3+3)&~3;
		CAutoVectorPtr<BYTE> pData(new BYTE[nLinearSize*rc.bottom]);
		BITMAPINFO tBMPInfo;
		ZeroMemory(&tBMPInfo, sizeof(tBMPInfo));
		tBMPInfo.bmiHeader.biSize = sizeof(tBMPInfo.bmiHeader);
		tBMPInfo.bmiHeader.biWidth = rc.right;
		tBMPInfo.bmiHeader.biHeight = -rc.bottom;
		tBMPInfo.bmiHeader.biCompression = BI_RGB;
		tBMPInfo.bmiHeader.biPlanes = 1;
		tBMPInfo.bmiHeader.biBitCount = 24;
		cBmp.Attach(cDC.SelectBitmap(cBmp.Detach()));
		cBmp.GetDIBits(cDC, 0, rc.bottom, pData.m_p, &tBMPInfo, DIB_RGB_COLORS);

		m_cTextBitmap.Free();
		m_cTextBitmap.Attach(new BYTE[rc.right*rc.bottom]);
		m_nTextSizeX = rc.right;
		m_nTextSizeY = rc.bottom;
		BYTE const* pSrc = pData.m_p;
		BYTE* pDst = m_cTextBitmap;
		for (LONG y = 0; y < rc.bottom; ++y)
		{
			BYTE const* p = pSrc;
			for (LONG x = 0; x < rc.right; ++x)
			{
				*pDst = *p;
				p += 3;
				++pDst;
			}
			pSrc += nLinearSize;
		}
		m_bCacheValid = true;
	}
	template<class TPixelMixer>
	void RenderPixels(RECT const& a_rc, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE const* a_pMask)
	{
		if (!m_bCacheValid)
			PrepareCache();
		LONG nX1 = m_tLastPos.fX;
		LONG nY1 = m_tLastPos.fY;
		LONG nX2 = nX1+m_nTextSizeX;
		LONG nY2 = nY1+m_nTextSizeY;
		RECT rcI =
		{
			max(a_rc.left, nX1),
			max(a_rc.top, nY1),
			min(a_rc.right, nX2),
			min(a_rc.bottom, nY2),
		};
		if (rcI.left >= rcI.right || rcI.top >= rcI.bottom)
			return;
		TRasterImagePixel t = {0, 0, 0, 0};
		BYTE const* pSrc = m_cTextBitmap.m_p+(rcI.top-nY1)*m_nTextSizeX+rcI.left-nX1;
		TRasterImagePixel* pDst = a_pBuffer+(rcI.top-a_rc.top)*a_nStride+rcI.left-a_rc.left;
		BYTE const* pMask = a_pMask+(rcI.top-a_rc.top)*a_nMaskStride+rcI.left-a_rc.left;
		if (/*m_eShapeFillMode == ESFMBrushFill && */M_Brush())
		{
			if (M_Brush()->IsSolid(&rcI) == S_OK)
			{
				M_Brush()->GetBrushTile(rcI.left, rcI.top, 1, 1, a_fGamma, 1, &t);
			}
			else
			{
				CAutoVectorPtr<TRasterImagePixel> cBrush(new TRasterImagePixel[(rcI.right-rcI.left)*(rcI.bottom-rcI.top)]);
				M_Brush()->GetBrushTile(rcI.left, rcI.top, rcI.right-rcI.left, rcI.bottom-rcI.top, a_fGamma, rcI.right-rcI.left, cBrush);
				TRasterImagePixel* pBrush = cBrush;
				for (LONG y = rcI.top; y < rcI.bottom; ++y)
				{
					BYTE const* pS = pSrc;
					TRasterImagePixel* pD = pDst;
					BYTE const* pM = pMask;
					if (a_pMask)
					{
						for (LONG x = rcI.left; x < rcI.right; ++x)
						{
							TPixelMixer::Mix(*pD, *pBrush, ULONG(*pM)**pS/255);
							++pS;
							++pD;
							++pM;
							++pBrush;
						}
					}
					else
					{
						for (LONG x = rcI.left; x < rcI.right; ++x)
						{
							TPixelMixer::Mix(*pD, *pBrush, *pS);
							++pS;
							++pD;
							++pBrush;
						}
					}
					pSrc += m_nTextSizeX;
					pDst += a_nStride;
					pMask += a_nMaskStride;
				}
				return;
			}
		}
		for (LONG y = rcI.top; y < rcI.bottom; ++y)
		{
			BYTE const* pS = pSrc;
			TRasterImagePixel* pD = pDst;
			BYTE const* pM = pMask;
			if (a_pMask)
			{
				if (t.bA)
				{
					for (LONG x = rcI.left; x < rcI.right; ++x)
					{
						TPixelMixer::Mix(*pD, t, ULONG(*pM)**pS/255);
						++pS;
						++pD;
						++pM;
					}
				}
				else
				{
					for (LONG x = rcI.left; x < rcI.right; ++x)
					{
						CPixelMixerReplace::Mix(*pD, t, ULONG(*pM)**pS/255);
						++pS;
						++pD;
						++pM;
					}
				}
			}
			else
			{
				if (t.bA)
				{
					for (LONG x = rcI.left; x < rcI.right; ++x)
					{
						TPixelMixer::Mix(*pD, t, *pS);
						++pS;
						++pD;
					}
				}
				else
				{
					for (LONG x = rcI.left; x < rcI.right; ++x)
					{
						CPixelMixerReplace::Mix(*pD, t, *pS);
						++pS;
						++pD;
					}
				}
			}
			pSrc += m_nTextSizeX;
			pDst += a_nStride;
			pMask += a_nMaskStride;
		}
	}
	RECT DirtyRect()
	{
		if (!m_bCacheValid)
			PrepareCache();
		RECT rc;
		rc.left = m_tLastPos.fX;
		rc.top = m_tLastPos.fY;
		rc.right = rc.left+m_nTextSizeX;
		rc.bottom = rc.top+m_nTextSizeY;
		return rc;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		TransformBrush(a_pMatrix);
		return E_NOTIMPL;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			std::wstring strFont;
			bool bBold = false;
			bool bItalic = false;
			float fSize = 0.0f;
			float fPosX = 0.0f;
			float fPosY = 0.0f;
			std::wstring strText;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(*negated_char_parser<chlit<wchar_t> >(L',')/*~ch_p(L'\,')*/)[assign_a(strFont)]>>ch_p(L',')>>
					(!((str_p(L"BOLD")[assign_a(bBold, true)]|str_p(L"ITALIC")[assign_a(bItalic, true)]|str_p(L"BOLDITALIC")[assign_a(bBold, true)][assign_a(bItalic, true)]|str_p(L"NORMAL")) >> ch_p(L',')))>>
					real_p[assign_a(fSize)]>>ch_p(L',')>>
					real_p[assign_a(fPosX)]>>ch_p(L',')>>
					real_p[assign_a(fPosY)]>>ch_p(L',')>>
					(*anychar_p)[assign_a(strText)]
					).full;
			if (!bParsed || strFont.empty() || strFont.length() >= LF_FACESIZE || fSize <= 0.0f)
				return E_INVALIDARG;
			m_cData.bBold = bBold;
			m_cData.bItalic = bItalic;
			m_cData.fSize = fSize;
			wcscpy(m_cData.lfFaceName, strFont.c_str());
			m_cData.strText = strText;
			m_eState = ESDragging;
			m_tLastPos.fX = fPosX;
			m_tLastPos.fY = fPosY;
			m_tDelta.fX = m_tDelta.fY = 0.0f;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		return E_NOTIMPL;
	}

private:
	EState m_eState;
	bool m_bCacheValid;
	CAutoVectorPtr<BYTE> m_cTextBitmap;
	ULONG m_nTextSizeX;
	ULONG m_nTextSizeY;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	TPixelCoords m_tLastPos;
	TPixelCoords m_tDelta;
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	//EShapeFillMode m_eShapeFillMode;
	float m_fGamma;
	bool m_bUseSecondary;
	CEditToolDataText m_cData;
};
