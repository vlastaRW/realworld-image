
#pragma once

#include "EditTool.h"
#include "EditToolPixelMixer.h"
#include "EditToolWithBrush.h"
#include "EditToolWithCtrlDropper.h"


struct CEditToolDataFloodFill
{
	MIDL_INTERFACE("1649C80F-C085-415C-9502-6A52181AC162")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataFloodFill const*, InternalData)() = 0;
	};

	enum EMatchMode
	{
		EMMRGBA = 0,
		EMMHue,
		EMMAlpha,
		EMMBrightness,
		EMMSelection,
	};

	CEditToolDataFloodFill() : bMultipoint(false), eMode(EMMRGBA), nTolerance(8)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return E_FAIL;
		int n = _wtoi(a_bstr);
		if (n || a_bstr[0] == L'0')
			nTolerance = n;
		if (wcsstr(a_bstr, L"RGBA"))
			eMode = EMMRGBA;
		else if (wcsstr(a_bstr, L"HLSA"))
			eMode = EMMHue;
		else if (wcsstr(a_bstr, L"ALPHA"))
			eMode = EMMAlpha;
		else if (wcsstr(a_bstr, L"BRIGHTNESS"))
			eMode = EMMBrightness;
		else if (wcsstr(a_bstr, L"ALL"))
			eMode = EMMSelection;
		bMultipoint = wcsstr(a_bstr, L"MULTIPOINT");
		//bEnergy = wcsstr(a_bstr, L"ENERGY");
		//bSmooth = wcsstr(a_bstr, L"SMOOTH");
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		if (a_pbstr == NULL)
			return E_POINTER;

		wchar_t szTmp[128];
		switch (eMode)
		{
		case EMMRGBA:		swprintf(szTmp, L"%i,RGBA", nTolerance); break;
		case EMMHue:		swprintf(szTmp, L"%i,HLSA", nTolerance); break;
		case EMMAlpha:		swprintf(szTmp, L"%i,ALPHA", nTolerance); break;
		case EMMBrightness:	swprintf(szTmp, L"%i,BRIGHTNESS", nTolerance); break;
		case EMMSelection:	swprintf(szTmp, L"%i,ALL", nTolerance); break;
		}
		if (bMultipoint) wcscat(szTmp, L"MULTIPOINT");
		//if (bEnergy) wcscat(szTmp, L"ENERGY");
		//if (bSmooth) wcscat(szTmp, L"SMOOTH");
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	bool bMultipoint;
	EMatchMode eMode;
	int nTolerance;
};

#include "EditToolFloodFillDlg.h"


HICON GetToolIconFLOODFILL(ULONG a_nSize);

class CEditToolFloodFill :
	public CEditToolMouseInput<CEditToolFloodFill>, // no direct tablet support
	public CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>,
	public CEditToolWithCtrlDropper<CEditToolFloodFill, CEditToolMouseInput<CEditToolFloodFill>, CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, CEditToolFloodFill>,
	public CEditToolCustomPrecisionCursor<GetToolIconFLOODFILL>,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolFloodFill, // T - the top level class for cross casting
		CEditToolFloodFill, // TResetHandler
		CEditToolFloodFill, // TDirtyHandler
		CEditToolFloodFill, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, // TBrushHandler
		CEditToolFloodFill, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolFloodFill, CEditToolMouseInput<CEditToolFloodFill>, CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, CEditToolFloodFill>, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolFloodFill, CEditToolMouseInput<CEditToolFloodFill>, CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, CEditToolFloodFill>, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolFloodFill, CEditToolMouseInput<CEditToolFloodFill>, CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, CEditToolFloodFill>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolFloodFill, CEditToolFloodFill, CEditToolFloodFill> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolFloodFill() : m_eBlendingMode(EBMDrawOver),
		m_rcDirty(RECT_EMPTY), m_nIgnoredIndex(-1), m_pGamma(NULL)
	{
		RWCoCreateInstance(m_pGammaCache, __uuidof(GammaTableCache));
		if (m_pGammaCache)
			m_pGamma = m_pGammaCache->GetSRGBTable();
	}

	BEGIN_COM_MAP(CEditToolFloodFill)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		m_cSeeds.clear();
		m_cIntervals.clear();
		m_nDirtyPixels = 0;
		RECT rc = m_rcDirty;
		m_rcDirty = RECT_EMPTY;
		m_nIgnoredIndex = -1;
		EnableBrush(false);
		if (rc.right > rc.left)
			M_Window()->RectangleChanged(&rc);
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = m_nDirtyPixels*3 < ULONG((m_rcDirty.right-m_rcDirty.left)*(m_rcDirty.bottom-m_rcDirty.top));
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		if (a_pImageRect)
			*a_pImageRect = m_rcDirty;
		return m_cIntervals.empty() ? S_FALSE : S_OK;
	}

	template<class TPixelMixer, bool t_bCorrect>
	void RenderPixels(RECT const& a_rc, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer, ULONG a_nMaskStride, BYTE* a_pMask)
	{
		RECT rcBrush =
		{
			max(a_rc.left, m_rcDirty.left),
			max(a_rc.top, m_rcDirty.top),
			min(a_rc.right, m_rcDirty.right),
			min(a_rc.bottom, m_rcDirty.bottom),
		};
		if (rcBrush.left >= rcBrush.right || rcBrush.top >= rcBrush.bottom)
			return; // no fille pixels in the requested tile

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
				M_Brush()->GetBrushTile(rcBrush.left, rcBrush.top, rcBrush.right-rcBrush.left, rcBrush.bottom-rcBrush.top, a_fGamma, a_rc.right-a_rc.left, cBrush.m_p + rcBrush.left-a_rc.left+(rcBrush.top-a_rc.top)*(a_rc.right-a_rc.left));

				for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
				{
					if (i->nY < a_rc.top || i->nY >= a_rc.bottom || i->nX2 < a_rc.left || i->nX1 >= a_rc.right)
						continue;
					ULONG nX1 = max(i->nX1, a_rc.left);
					ULONG nX2 = min(i->nX2, a_rc.right);
					TRasterImagePixel* p = a_pBuffer+(i->nY-a_rc.top)*a_nStride-a_rc.left;
					TRasterImagePixel* pB = cBrush.m_p+(i->nY-a_rc.top)*(a_rc.right-a_rc.left)-a_rc.left;
					if (a_pMask)
					{
						BYTE* pM = a_pMask+(i->nY-a_rc.top)*a_nMaskStride-a_rc.left;
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
		for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
		{
			if (i->nY < a_rc.top || i->nY >= a_rc.bottom || i->nX2 < a_rc.left || i->nX1 >= a_rc.right)
				continue;
			ULONG nX1 = max(i->nX1, a_rc.left);
			ULONG nX2 = min(i->nX2, a_rc.right);
			TRasterImagePixel* p = a_pBuffer+(i->nY-a_rc.top)*a_nStride-a_rc.left;
			if (a_pMask)
			{
				BYTE* pM = a_pMask+(i->nY-a_rc.top)*a_nMaskStride-a_rc.left;
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
		if (FAILED(hRes) || m_cIntervals.empty())
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


	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataFloodFill::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			CEditToolDataFloodFill cData = m_cData;
			m_cData = *(pData->InternalData());
			if ((m_cData.bMultipoint == cData.bMultipoint &&
				m_cData.eMode == cData.eMode && m_cData.nTolerance == cData.nTolerance) ||
				m_cIntervals.empty())
				return S_OK;
			if (!m_cData.bMultipoint && cData.bMultipoint && !m_cSeeds.empty())
			{
				m_cSeeds.resize(1);
				M_Window()->ControlPointsChanged();
			}
			RefreshImage();
		}
		return S_OK;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			EnableBrush(!m_cSeeds.empty());
			M_Window()->RectangleChanged(&m_rcDirty);
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
		if (!m_cData.bMultipoint && !m_cIntervals.empty())
		{
			ATLASSERT(0);
			m_cIntervals.clear();
		}

		POINT pt = {a_pPos->fX, a_pPos->fY};
		m_cSeeds.push_back(pt);
		PrepareFloodFill();
		if (!m_cIntervals.empty())
			M_Window()->RectangleChanged(&m_rcDirty);
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_cSeeds.empty())
			return S_OK;

		POINT pt = {a_pPos->fX, a_pPos->fY};
		if (pt.x < 0 || pt.y < 0 || pt.x >= LONG(m_nSizeX) || pt.y >= LONG(m_nSizeY))
		{
			m_cSeeds.resize(m_cSeeds.size()-1);
			return S_OK;
		}
		M_Window()->ControlPointsChanged();
		EnableBrush(!m_cSeeds.empty());
		CSeeds::reverse_iterator i = m_cSeeds.rbegin();
		if (i->x != pt.x || i->y != pt.y)
		{
			*i = pt;
			RefreshImage();
		}
		if (!m_cData.bMultipoint)
		{
			ULONG n = 0;
			if (M_Brush()) M_Brush()->GetControlPointCount(&n);
			if (n == 0)
				return ETPAApply;
		}
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (!M_Dragging())
			return S_OK;
		if (m_cSeeds.empty())
			return S_OK;
		CSeeds::reverse_iterator i = m_cSeeds.rbegin();
		POINT pt = {a_pPos->fX, a_pPos->fY};
		if (i->x != pt.x || i->y != pt.y)
		{
			*i = pt;
			RefreshImage();
		}
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
			return CEditToolCustomPrecisionCursor<GetToolIconFLOODFILL>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
		}
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_cData.bMultipoint ? m_cSeeds.size() : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (a_nIndex >= m_cSeeds.size())
			return E_RW_INDEXOUTOFRANGE;

		a_pPos->fX = m_cSeeds[a_nIndex].x+0.5f;
		a_pPos->fY = m_cSeeds[a_nIndex].y+0.5f;
		*a_pClass = 0;
		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (a_nIndex >= m_cSeeds.size())
			return E_RW_INDEXOUTOFRANGE;

		ULONG i;
		for (i = 0; i < m_cSeeds.size(); ++i)
		{
			if (i != a_nIndex)
			{
				float const fDX = a_pPos->fX-m_cSeeds[i].x-0.5f;
				float const fDY = a_pPos->fY-m_cSeeds[i].y-0.5f;
				if ((fDX*fDX + fDY*fDY) < a_fPointSize*a_fPointSize)
				{
					if (a_bFinished)
					{
						m_cSeeds.erase(m_cSeeds.begin()+a_nIndex);
						EnableBrush(!m_cSeeds.empty());
						M_Window()->ControlPointsChanged();
						if (m_nIgnoredIndex != a_nIndex)
						{
							RefreshImage();
						}
						m_nIgnoredIndex = -1;
						return S_OK;
					}
					else
					{
						if (m_nIgnoredIndex != a_nIndex)
						{
							m_nIgnoredIndex = a_nIndex;
							RefreshImage();
						}
					}
					break;
				}
			}
		}
		if (i == m_cSeeds.size())
			m_nIgnoredIndex = -1;
		POINT pt = {a_pPos->fX, a_pPos->fY};
		if (a_bFinished && (pt.x < 0 || pt.y < 0 || pt.x >= LONG(m_nSizeX) || pt.y >= LONG(m_nSizeY)))
		{
			m_cSeeds.erase(m_cSeeds.begin()+a_nIndex);
			EnableBrush(!m_cSeeds.empty());
			M_Window()->ControlPointsChanged();
			RefreshImage();
			m_nIgnoredIndex = -1;
			return S_OK;
		}
		if (pt.x == m_cSeeds[a_nIndex].x && pt.y == m_cSeeds[a_nIndex].y)
			return S_FALSE;
		m_cSeeds[a_nIndex] = pt;
		M_Window()->ControlPointChanged(a_nIndex);
		RefreshImage();
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlLines(IEditToolControlLines* UNREF(a_pLines), ULONG UNREF(a_nLineTypes))
	{
		return S_OK;
	}
	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return a_eKeysState&ECKSControl ? ETPAMissed : m_cData.bMultipoint ? ETPAMissed|ETPACustomAction : ETPAMissed|ETPAStartNew;
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
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			float fX;
			float fY;
			float fTolerance;
			switch (swscanf(a_bstrParams, L"%f,%f,%f", &fX, &fY, &fTolerance))
			{
			case 3:
				m_cData.nTolerance = fTolerance+0.5f;
			case 2:
				{
					if (wcsstr(a_bstrParams, L"\"RGBA\""))
						m_cData.eMode = CEditToolDataFloodFill::EMMRGBA;
					else if (wcsstr(a_bstrParams, L"\"HLSA\""))
						m_cData.eMode = CEditToolDataFloodFill::EMMHue;
					else if (wcsstr(a_bstrParams, L"\"ALPHA\""))
						m_cData.eMode = CEditToolDataFloodFill::EMMAlpha;
					else if (wcsstr(a_bstrParams, L"\"BRIGHTNESS\""))
						m_cData.eMode = CEditToolDataFloodFill::EMMBrightness;
					else if (wcsstr(a_bstrParams, L"\"ALL\""))
						m_cData.eMode = CEditToolDataFloodFill::EMMSelection;
					//m_cData.bMultipoint = wcsstr(a_bstrParams, L"MULTIPOINT");
					int nX = fX+0.5f;
					int nY = fY+0.5f;
					if (nX < 0 || nY < 0 || nX >= LONG(m_nSizeX) || nY >= LONG(m_nSizeY))
						return S_FALSE;
					POINT pt = {nX, nY};
					m_cSeeds.push_back(pt);
					PrepareFloodFill();
					if (!m_cIntervals.empty())
						M_Window()->RectangleChanged(&m_rcDirty);
					return S_OK;
				}
			default:
				return S_FALSE;
			}
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		try
		{
			CComBSTR bstr;
			if (m_cSeeds.size() == 1)
			{
				OLECHAR sz[64];
				swprintf(sz, L"%i, %i, %i, \"%s\"", m_cSeeds[0].x, m_cSeeds[0].y, m_cData.nTolerance, m_cData.eMode == CEditToolDataFloodFill::EMMRGBA ? L"RGBA" : (m_cData.eMode == CEditToolDataFloodFill::EMMHue ? L"HLSA" : (m_cData.eMode == CEditToolDataFloodFill::EMMAlpha ? L"ALPHA" : (m_cData.eMode == CEditToolDataFloodFill::EMMBrightness ? L"BRIGHTNESS" : L"ALL"))));
				bstr = sz;
			}
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	struct SInterval
	{
		LONG nY;
		LONG nX1;
		LONG nX2;
	};
	typedef std::vector<SInterval> CIntervals;

	struct PixelComparer
	{
		virtual ~PixelComparer() {}
		virtual bool operator()(TRasterImagePixel const& a_tPixel) const = 0;
	};

	struct FillAllComparer : public PixelComparer
	{
		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			return true;
		}
	};

	struct ZeroToleranceRGBAComparer : public PixelComparer
	{
		ZeroToleranceRGBAComparer(TRasterImagePixel a_tReference) : m_tReference(a_tReference) {}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			return *reinterpret_cast<DWORD const*>(&m_tReference) == *reinterpret_cast<DWORD const*>(&a_tPixel);
		}

	private:
		TRasterImagePixel const m_tReference;
	};

	struct RGBAComparer : public PixelComparer
	{
		RGBAComparer(TRasterImagePixel a_tReference, int a_nTolerance) : m_tReference(a_tReference), m_nTolerance(a_nTolerance) {}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			return abs(int(a_tPixel.bR)*a_tPixel.bA-int(m_tReference.bR)*m_tReference.bA) < m_nTolerance*650 &&
				   abs(int(a_tPixel.bG)*a_tPixel.bA-int(m_tReference.bG)*m_tReference.bA) < m_nTolerance*650 &&
				   abs(int(a_tPixel.bB)*a_tPixel.bA-int(m_tReference.bB)*m_tReference.bA) < m_nTolerance*650 &&
				   abs(int(a_tPixel.bA)-int(m_tReference.bA))*100 < m_nTolerance*255;
		}

	private:
		TRasterImagePixel const m_tReference;
		int const m_nTolerance;
	};

	struct BrightnessComparer : public PixelComparer
	{
		BrightnessComparer(TRasterImagePixel a_tReference, int a_nTolerance) :
			m_nReferenceL(int(a_tReference.bR)*a_tReference.bA+int(a_tReference.bG)*a_tReference.bA+int(a_tReference.bB)*a_tReference.bA),
			m_nReferenceA(a_tReference.bA), m_nToleranceL(a_nTolerance*255*255*3/100), m_nToleranceA(a_nTolerance*255) {}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			return abs((int(a_tPixel.bR)*a_tPixel.bA + int(a_tPixel.bG)*a_tPixel.bA + int(a_tPixel.bB)*a_tPixel.bA) - m_nReferenceL) < m_nToleranceL &&
				   abs(int(a_tPixel.bA)-m_nReferenceA)*100 < m_nToleranceA;
		}

	private:
		int const m_nReferenceL;
		int const m_nReferenceA;
		int const m_nToleranceL;
		int const m_nToleranceA;
	};

	struct HueComparer : public PixelComparer
	{
		HueComparer(TRasterImagePixel a_tReference, int a_nTolerance) :
			m_nReferenceH(GetHue(a_tReference)), m_nToleranceH(a_nTolerance*0xffff/199),
			m_nReferenceA(a_tReference.bA), m_nToleranceA(a_nTolerance*255) {}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			if (abs(int(a_tPixel.bA)-m_nReferenceA)*100 >= m_nToleranceA)
				return false;
			int const n = (GetHue(a_tPixel) - m_nReferenceH)&0xffff;
			return n < m_nToleranceH || n > (0xffff-m_nToleranceH);
		}

		static int GetHue(TRasterImagePixel const a_t)
		{
			int const rgbmax = a_t.bR>a_t.bG ? (a_t.bR>a_t.bB ? a_t.bR : a_t.bB) : (a_t.bG>a_t.bB ? a_t.bG : a_t.bB);
			int const rgbmin = a_t.bR<a_t.bG ? (a_t.bR<a_t.bB ? a_t.bR : a_t.bB) : (a_t.bG<a_t.bB ? a_t.bG : a_t.bB);

			int const rgbdelta = rgbmax - rgbmin;
			if (rgbdelta == 0)
				return 0;

			if (a_t.bR == rgbmax)
				return 0xffff&((a_t.bG - a_t.bB)*0xffff / (rgbdelta*6));
			if (a_t.bG == rgbmax)
				return 0xffff&(((rgbdelta + rgbdelta + a_t.bB - a_t.bR)*0xffff) / (rgbdelta*6));
			return 0xffff&((((rgbdelta<<2) + a_t.bR - a_t.bG)*0xffff) / (rgbdelta*6));
		}

	private:
		int const m_nReferenceH;
		int const m_nReferenceA;
		int const m_nToleranceH;
		int const m_nToleranceA;
	};

	struct AlphaComparer : public PixelComparer
	{
		AlphaComparer(TRasterImagePixel a_tReference, int a_nTolerance) :
			m_nReferenceA(a_tReference.bA), m_nToleranceA(a_nTolerance*255) {}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			return abs(int(a_tPixel.bA)-m_nReferenceA)*100 < m_nToleranceA;
		}

	private:
		int const m_nReferenceA;
		int const m_nToleranceA;
	};

	struct MultiPointComparer : public PixelComparer
	{
		MultiPointComparer(int a_nComparers, int a_nTolerance) :
			m_aComparers(new PixelComparer*[a_nComparers]),
			m_nComparers(a_nComparers), m_nTolerance(a_nTolerance)
		{
			ZeroMemory(m_aComparers, a_nComparers*sizeof*m_aComparers);
		}
		~MultiPointComparer()
		{
			for (int i = 0; i < m_nComparers; ++i)
			{
				delete m_aComparers[i];
			}
			delete[] m_aComparers;
		}

		bool operator()(TRasterImagePixel const& a_tPixel) const
		{
			for (int i = 0; i < m_nComparers; ++i)
				if ((*(m_aComparers[i]))(a_tPixel))
					return true;
		return false;
		}
		void SetComparer(int a_nIndex, PixelComparer* a_pComparer)
		{
			m_aComparers[a_nIndex] = a_pComparer;
		}

	private:
		PixelComparer** m_aComparers;
		int const m_nComparers;
		int const m_nTolerance;
	};

	typedef std::vector<POINT> CSeeds;

private:
	inline static void LinearFill(TRasterImagePixel const* const a_pC, BYTE* const a_pF, ULONG const a_nSizeX, ULONG const a_nX, ULONG const a_nY, PixelComparer const& a_tComparer, CIntervals& a_cQueue)
	{
		SInterval sI;
		sI.nY = a_nY;
		sI.nX1 = a_nX;
        do
		{
            a_pF[sI.nX1] = 1;
			--sI.nX1;
		}
		while (sI.nX1 != ULONG(-1) && a_pF[sI.nX1] == 0 && a_tComparer(a_pC[sI.nX1]));
		++sI.nX1;

		sI.nX2 = a_nX+1;
		while (sI.nX2 < LONG(a_nSizeX) && a_pF[sI.nX2] == 0 && a_tComparer(a_pC[sI.nX2]))
        {
            a_pF[sI.nX2] = 1;
			++sI.nX2;
		}
		a_cQueue.push_back(sI);
	}
	void PrepareFloodFill()
	{
		m_rcDirty = RECT_EMPTY;
		m_nDirtyPixels = 0;
		m_cIntervals.clear();
		if (m_cSeeds.empty())
			return;
		CSeeds cSeeds;
		for (CSeeds::const_iterator i = m_cSeeds.begin(); i != m_cSeeds.end(); ++i)
		{
			if (i->x < 0 || i->y < 0 || i->x >= LONG(m_nSizeX) || i->y >= LONG(m_nSizeY) || i-m_cSeeds.begin() == m_nIgnoredIndex)
				continue;
			cSeeds.push_back(*i);
		}
		if (cSeeds.empty())
			return;
		ULONG nSizeX = m_nSizeX;
		ULONG nSizeY = m_nSizeY;
		CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nSizeX*nSizeY]);
		M_Window()->GetImageTile(0, 0, nSizeX, nSizeY, 0.0f, nSizeX, EITIContent, cBuffer);
		CAutoVectorPtr<BYTE> cFilled(new BYTE[nSizeX*nSizeY]);
		ZeroMemory(cFilled.m_p, nSizeX*nSizeY);
		CAutoPtr<PixelComparer> pComparer;
		if (cSeeds.size() == 1)
		{
			TRasterImagePixel tMatch = cBuffer[cSeeds[0].x+nSizeX*cSeeds[0].y];
			switch (m_cData.eMode)
			{
			case CEditToolDataFloodFill::EMMBrightness:
				pComparer.Attach(new BrightnessComparer(tMatch, m_cData.nTolerance));
				break;
			case CEditToolDataFloodFill::EMMAlpha:
				pComparer.Attach(new AlphaComparer(tMatch, m_cData.nTolerance));
				break;
			case CEditToolDataFloodFill::EMMHue:
				pComparer.Attach(new HueComparer(tMatch, m_cData.nTolerance));
				break;
			case CEditToolDataFloodFill::EMMSelection:
				pComparer.Attach(new FillAllComparer());
				break;
			default:
				pComparer.Attach(m_cData.nTolerance ?
					static_cast<PixelComparer*>(new RGBAComparer(tMatch, m_cData.nTolerance)) :
					static_cast<PixelComparer*>(new ZeroToleranceRGBAComparer(tMatch))
					);
				break;
			}
		}
		else
		{
			MultiPointComparer* pMPC = new MultiPointComparer(cSeeds.size(), m_cData.nTolerance);
			pComparer.Attach(pMPC);
			for (CSeeds::const_iterator i = cSeeds.begin(); i != cSeeds.end(); ++i)
			{
				TRasterImagePixel tMatch = cBuffer[i->x+nSizeX*i->y];
				switch (m_cData.eMode)
				{
				case CEditToolDataFloodFill::EMMBrightness:
					pMPC->SetComparer(i-cSeeds.begin(), new BrightnessComparer(tMatch, m_cData.nTolerance));
					break;
				case CEditToolDataFloodFill::EMMAlpha:
					pMPC->SetComparer(i-cSeeds.begin(), new AlphaComparer(tMatch, m_cData.nTolerance));
					break;
				case CEditToolDataFloodFill::EMMHue:
					pMPC->SetComparer(i-cSeeds.begin(), new HueComparer(tMatch, m_cData.nTolerance));
					break;
				case CEditToolDataFloodFill::EMMSelection:
					pMPC->SetComparer(i-cSeeds.begin(), new FillAllComparer());
					break;
				default:
					pMPC->SetComparer(i-cSeeds.begin(), m_cData.nTolerance ?
						static_cast<PixelComparer*>(new RGBAComparer(tMatch, m_cData.nTolerance)) :
						static_cast<PixelComparer*>(new ZeroToleranceRGBAComparer(tMatch))
						);
					break;
				}
			}
		}
		for (CSeeds::const_iterator i = cSeeds.begin(); i != cSeeds.end(); ++i)
		{
			ULONG nX = i->x;
			ULONG nY = i->y;
			if (cFilled[nY*nSizeX+nX])
				continue;
			LinearFill(cBuffer.m_p+nY*nSizeX, cFilled.m_p+nY*nSizeX, nSizeX, nX, nY, *pComparer.m_p, m_cIntervals);
			for (size_t iNext = 0; iNext < m_cIntervals.size(); ++iNext)
			{
				SInterval sInt = m_cIntervals[iNext];
				if (m_rcDirty.left > LONG(sInt.nX1)) m_rcDirty.left = sInt.nX1;
				if (m_rcDirty.top > LONG(sInt.nY)) m_rcDirty.top = sInt.nY;
				if (m_rcDirty.right < LONG(sInt.nX2)) m_rcDirty.right = sInt.nX2;
				if (m_rcDirty.bottom <= LONG(sInt.nY)) m_rcDirty.bottom = sInt.nY+1;
				m_nDirtyPixels += sInt.nX2-sInt.nX1;

				if (sInt.nY > 0)
				{
					--sInt.nY;
					TRasterImagePixel const* pC = cBuffer.m_p+sInt.nY*nSizeX;
					BYTE* pF = cFilled.m_p+sInt.nY*nSizeX;
					// fill above
					for (LONG x = sInt.nX1; x < sInt.nX2; ++x)
					{
						if (pF[x] == 0 && (*pComparer.m_p)(pC[x]))
							LinearFill(pC, pF, nSizeX, x, sInt.nY, *pComparer.m_p, m_cIntervals);
					}
					++sInt.nY;
				}
				if (sInt.nY < LONG(nSizeY-1))
				{
					++sInt.nY;
					TRasterImagePixel const* pC = cBuffer.m_p+sInt.nY*nSizeX;
					BYTE* pF = cFilled.m_p+sInt.nY*nSizeX;
					// fill below
					for (LONG x = sInt.nX1; x < sInt.nX2; ++x)
					{
						if (pF[x] == 0 && (*pComparer.m_p)(pC[x]))
							LinearFill(pC, pF, nSizeX, x, sInt.nY, *pComparer.m_p, m_cIntervals);
					}
				}
			}
		}
		if (M_Brush() && m_nDirtyPixels)
		{
			TPixelCoords tCenter = {(m_rcDirty.right+m_rcDirty.left)*0.5f, (m_rcDirty.bottom+m_rcDirty.top)*0.5f};
			M_Brush()->SetShapeBounds(&tCenter, (m_rcDirty.right-m_rcDirty.left)*0.5f, (m_rcDirty.bottom-m_rcDirty.top)*0.5f, 0.0f);
		}
	}

	void RefreshImage()
	{
		RECT rcPrev = m_rcDirty;
		PrepareFloodFill();
		if (rcPrev.left > m_rcDirty.left) rcPrev.left = m_rcDirty.left;
		if (rcPrev.top > m_rcDirty.top) rcPrev.top = m_rcDirty.top;
		if (rcPrev.right < m_rcDirty.right) rcPrev.right = m_rcDirty.right;
		if (rcPrev.bottom < m_rcDirty.bottom) rcPrev.bottom = m_rcDirty.bottom;
		if (rcPrev.left < rcPrev.right)
			M_Window()->RectangleChanged(&rcPrev);
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	CSeeds m_cSeeds;
	int m_nIgnoredIndex;
	EBlendingMode m_eBlendingMode;
	CIntervals m_cIntervals;
	RECT m_rcDirty;
	ULONG m_nDirtyPixels;
	CEditToolDataFloodFill m_cData;

	CComPtr<IGammaTableCache> m_pGammaCache;
	CGammaTables const* m_pGamma;
};
