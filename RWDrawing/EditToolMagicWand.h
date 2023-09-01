
#pragma once

#include <math.h>
#include "EditTool.h"

struct CEditToolDataMagicWand
{
	MIDL_INTERFACE("3D4D4A35-FCF7-4EA6-9011-90D2FCD7FD05")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataMagicWand const*, InternalData)() = 0;
	};

	enum EMatchMode
	{
		EMMRGBA = 0,
		EMMHue,
		EMMAlpha,
		EMMBrightness,
	};

	CEditToolDataMagicWand() : bMultipoint(false), bEnergy(false), bSmooth(false),
		eMode(EMMRGBA), nTolerance(8)
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
		bMultipoint = wcsstr(a_bstr, L"MULTIPOINT");
		bEnergy = wcsstr(a_bstr, L"ENERGY");
		bSmooth = wcsstr(a_bstr, L"SMOOTH");
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
		}
		if (bMultipoint) wcscat(szTmp, L"MULTIPOINT");
		if (bEnergy) wcscat(szTmp, L"ENERGY");
		if (bSmooth) wcscat(szTmp, L"SMOOTH");
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	bool bMultipoint;
	bool bEnergy;
	bool bSmooth;
	EMatchMode eMode;
	int nTolerance;
};

#include "EditToolMagicWandDlg.h"
#include "EditToolShadow.h"


HICON GetToolIconMAGICWAND(ULONG a_nSize);

class CEditToolMagicWand :
	public CEditToolMouseInput<CEditToolMagicWand>, // no direct tablet support
	public CEditToolCustomPrecisionCursor<GetToolIconMAGICWAND>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolMagicWand, // T - the top level class for cross casting
		CEditToolMagicWand, // TResetHandler
		CEditToolMagicWand, // TDirtyHandler
		CRasterImageEditToolBase, // TImageTileHandler
		CEditToolMagicWand, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolMagicWand, // TAdjustCoordsHandler
		CEditToolMagicWand, // TGetCursorHandler
		CEditToolMouseInput<CEditToolMagicWand>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolMagicWand, // TControlPointsHandler
		CRasterImageEditToolBase // TControlLinesHandler
	>
{
public:
	CEditToolMagicWand() : m_rcDirty(RECT_EMPTY), m_nIgnoredIndex(-1), m_eKeysState(ECKSNone)
	{
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		m_cSeeds.clear();
		m_cIntervals.clear();
		m_nDirtyPixels = 0;
		m_rcDirty = RECT_EMPTY;
		m_nIgnoredIndex = -1;
		RECT rc = {0, 0, m_nSizeX, m_nSizeY};
		if (rc.right > rc.left)
			M_Window()->RectangleChanged(&rc);
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
		{
			if (m_cIntervals.empty())
			{
				*a_pSelectionRect = RECT_EMPTY;
			}
			else
			{
				if (m_eKeysState == ECKSNone)
				{
					a_pSelectionRect->left = a_pSelectionRect->top = 0;
					a_pSelectionRect->right = m_nSizeX;
					a_pSelectionRect->bottom = m_nSizeY;
				}
				else
				{
					*a_pSelectionRect = m_rcDirty;
				}
			}
		}
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		return m_cIntervals.empty() ? S_FALSE : S_OK;
	}

	HRESULT _GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (m_cIntervals.empty())
			return M_Window()->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
		*a_bEntireRectangle = FALSE;
		BOOL bDummy;
		switch (m_eKeysState & ECKSShiftControl)
		{
		case ECKSNone:
			*a_pBoundingRectangle = m_rcDirty;
			return S_OK;
		case ECKSControl:
		case ECKSShift:
			{
				RECT rcPrev;
				M_Window()->GetSelectionInfo(&rcPrev, &bDummy);
				a_pBoundingRectangle->left = min(rcPrev.left, m_rcDirty.left);
				a_pBoundingRectangle->right = max(rcPrev.right, m_rcDirty.right);
				a_pBoundingRectangle->top = min(rcPrev.top, m_rcDirty.top);
				a_pBoundingRectangle->bottom = max(rcPrev.bottom, m_rcDirty.bottom);
			}
			return S_OK;
		case ECKSShiftControl:
			return M_Window()->GetSelectionInfo(a_pBoundingRectangle, &bDummy);
		}

		*a_pBoundingRectangle = m_rcDirty;
		return S_OK;
	}
	HRESULT _GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		if (m_cIntervals.empty())
			return M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);

		LONG const nY2 = a_nY+a_nSizeY;
		LONG const nX2 = a_nX+a_nSizeX;

		switch (m_eKeysState & ECKSShiftControl)
		{
		case ECKSNone:
			for (ULONG y = 0; y < a_nSizeY; ++y)
				FillMemory(a_pBuffer+y*a_nStride, a_nSizeX, 0);
			for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
			{
				if (i->nY < a_nY || i->nY >= nY2 || i->nX2 < a_nX || i->nX1 >= nX2)
					continue;
				ULONG const nXA = max(a_nX, i->nX1);
				ULONG const nXB = min(nX2, i->nX2);
				FillMemory(a_pBuffer+(i->nY-a_nY)*a_nStride+nXA-a_nX, nXB-nXA, 0xff);
			}
			return S_OK;
		case ECKSControl:
			M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
			for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
			{
				if (i->nY < a_nY || i->nY >= nY2 || i->nX2 < a_nX || i->nX1 >= nX2)
					continue;
				ULONG const nXA = max(a_nX, i->nX1);
				BYTE* p = a_pBuffer+(i->nY-a_nY)*a_nStride+nXA-a_nX;
				BYTE* const pEnd = p + min(nX2, i->nX2)-nXA;
				while (p != pEnd)
				{
					*p = 0xff-*p;
					++p;
				}
			}
			return S_OK;
		case ECKSShift:
			M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
			for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
			{
				if (i->nY < a_nY || i->nY >= nY2 || i->nX2 < a_nX || i->nX1 >= nX2)
					continue;
				ULONG const nXA = max(a_nX, i->nX1);
				ULONG const nXB = min(nX2, i->nX2);
				FillMemory(a_pBuffer+(i->nY-a_nY)*a_nStride+nXA-a_nX, nXB-nXA, 0xff);
			}
			return S_OK;
		case ECKSShiftControl:
			M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
			for (CIntervals::const_iterator i = m_cIntervals.begin(); i != m_cIntervals.end(); ++i)
			{
				if (i->nY < a_nY || i->nY >= nY2 || i->nX2 < a_nX || i->nX1 >= nX2)
					continue;
				ULONG const nXA = max(a_nX, i->nX1);
				ULONG const nXB = min(nX2, i->nX2);
				FillMemory(a_pBuffer+(i->nY-a_nY)*a_nStride+nXA-a_nX, nXB-nXA, 0);
			}
			return S_OK;
		default:
			return E_FAIL;
		}
	}


	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataMagicWand::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			CEditToolDataMagicWand cData = m_cData;
			m_cData = *(pData->InternalData());
			if ((m_cData.bMultipoint == cData.bMultipoint &&
				m_cData.eMode == cData.eMode && m_cData.nTolerance == cData.nTolerance &&
				m_cData.bEnergy == cData.bEnergy && m_cData.bSmooth == cData.bSmooth) ||
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

		if (m_cSeeds.empty())
			m_eKeysState = a_eKeysState;

		POINT pt = {a_pPos->fX, a_pPos->fY};
		m_cSeeds.push_back(pt);
		PrepareFloodFill();

		if (!m_cIntervals.empty())
		{
			if (m_eKeysState == ECKSNone)
			{
				RECT rc = {0, 0, m_nSizeX, m_nSizeY};
				M_Window()->RectangleChanged(&rc);
			}
			else
			{
				M_Window()->RectangleChanged(&m_rcDirty);
			}
		}

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
		CSeeds::reverse_iterator i = m_cSeeds.rbegin();
		if (i->x != pt.x || i->y != pt.y)
		{
			*i = pt;
			RefreshImage();
		}
		if (!m_cData.bMultipoint)
			return ETPAApply;
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
			return CEditToolCustomPrecisionCursor<GetToolIconMAGICWAND>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
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
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		return S_OK;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return m_cData.bMultipoint ? ETPAMissed|ETPACustomAction : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
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
		virtual int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const = 0;
		// 0 = 100% match, 0xffff = 0%
	};

	struct FillAllComparer : public PixelComparer
	{
		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			return 0;
		}
	};

	struct ZeroToleranceRGBAComparer : public PixelComparer
	{
		ZeroToleranceRGBAComparer(TRasterImagePixel a_tReference) : m_tReference(a_tReference) {}

		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			return *reinterpret_cast<DWORD const*>(&m_tReference) == *reinterpret_cast<DWORD const*>(&a_tPixel) ? 0 : 0xffff;
		}

	private:
		TRasterImagePixel const m_tReference;
	};

	struct RGBAComparer : public PixelComparer
	{
		RGBAComparer(TRasterImagePixel a_tReference) :
			m_nRefR(int(a_tReference.bR)*a_tReference.bA), m_nRefG(int(a_tReference.bG)*a_tReference.bA),
			m_nRefB(int(a_tReference.bB)*a_tReference.bA), m_nRefA(a_tReference.bA)
		{
		}

		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			int n1 = abs(int(a_tPixel.bR)*a_tPixel.bA-m_nRefR);
			int n2 = abs(int(a_tPixel.bG)*a_tPixel.bA-m_nRefG);
			int n3 = abs(int(a_tPixel.bB)*a_tPixel.bA-m_nRefB);
			int n4 = abs(int(a_tPixel.bA)-m_nRefA)*255;
			return (n1 > n2 ? (n3 > n4 ? (n1 > n3 ? n1 : n3) : (n1 > n4 ? n1 : n4)) : (n3 > n4 ? (n2 > n3 ? n2 : n3) : (n2 > n4 ? n2 : n4)));
		}

	private:
		int const m_nRefR;
		int const m_nRefG;
		int const m_nRefB;
		int const m_nRefA;
	};

	struct BrightnessComparer : public PixelComparer
	{
		BrightnessComparer(TRasterImagePixel a_tReference) :
			m_nReferenceL(int(a_tReference.bR)*a_tReference.bA+int(a_tReference.bG)*a_tReference.bA+int(a_tReference.bB)*a_tReference.bA),
			m_nReferenceA(a_tReference.bA)
		{
		}

		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			int n1 = abs((int(a_tPixel.bR)*a_tPixel.bA + int(a_tPixel.bG)*a_tPixel.bA + int(a_tPixel.bB)*a_tPixel.bA) - m_nReferenceL);
			int n2 = abs(int(a_tPixel.bA)-m_nReferenceA)*255;
			return (n1 > n2 ? n1 : n2);
		}

	private:
		int const m_nReferenceL;
		int const m_nReferenceA;
	};

	struct HueComparer : public PixelComparer
	{
		HueComparer(TRasterImagePixel a_tReference) :
			m_nReferenceH(GetHue(a_tReference)),
			m_nReferenceA(a_tReference.bA) {}

		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			int n1 = abs(int(a_tPixel.bA)-m_nReferenceA)*255;
			int n2 = (GetHue(a_tPixel) - m_nReferenceH)&0xffff;
			if (n2 >= 0x8000) n2 = (0x10000-n2)<<1; else n2 <<= 1;
			return (n1 > n2 ? n1 : n2);
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
	};

	struct AlphaComparer : public PixelComparer
	{
		AlphaComparer(TRasterImagePixel a_tReference) :
			m_nReferenceA(a_tReference.bA) {}

		int operator()(TRasterImagePixel const a_tPixel, LONG const UNREF(a_nX), LONG const UNREF(a_nY)) const
		{
			return abs(int(a_tPixel.bA)-m_nReferenceA)*255;
		}

	private:
		int const m_nReferenceA;
	};

	struct EnergyComparer : public PixelComparer
	{
		EnergyComparer(PixelComparer* a_pComp, TRasterImagePixel const* a_pData, LONG a_nSizeX, LONG a_nSizeY, int a_nTolerance) :
			m_pComp(a_pComp), m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_pData(a_pData),
			m_nMaxTolerance(a_nTolerance+(a_nTolerance>>1)), m_nMinTolerance(a_nTolerance-(a_nTolerance>>1)), m_nFactor(a_nTolerance)
		{
			m_cBuf.Allocate(m_nSizeX*m_nSizeY);
		}
		~EnergyComparer()
		{
			delete m_pComp;
		}

		int operator()(TRasterImagePixel const a_tPixel, LONG const a_nX, LONG const a_nY) const
		{
			int nBase = (*m_pComp)(a_tPixel, a_nX, a_nY);
			if (nBase < m_nMinTolerance || nBase > m_nMaxTolerance)
				return nBase;

			ULONG iDiffTotal = 0;
			ULONG iDiffCount = 0;
			ULONG const nYMax = min((a_nY+4), m_nSizeY);
			for (ULONG iYSample = a_nY < 3 ? 0 : (a_nY-3); iYSample <= nYMax; ++iYSample)
			{
				ULONG const nXMax = min((a_nX+4), m_nSizeX);
				TRasterImagePixel const* p = m_pData+iYSample*m_nSizeX;
				for(ULONG iXSample = a_nX < 3 ? 0 : (a_nX-3); iXSample < nXMax; ++iXSample)
				{
					iDiffTotal += abs(int(a_tPixel.bR) - int(p[iXSample].bR));
					iDiffTotal += abs(int(a_tPixel.bG) - int(p[iXSample].bG));
					iDiffTotal += abs(int(a_tPixel.bB) - int(p[iXSample].bB));
					iDiffTotal += abs(int(a_tPixel.bA) - int(p[iXSample].bA));
					++iDiffCount;
				}
			}

			return m_nMinTolerance + (nBase-m_nMinTolerance)*iDiffTotal/(iDiffCount*0x30);
		}

	private:
		PixelComparer* const m_pComp;
		TRasterImagePixel const* const m_pData;
		CAutoVectorPtr<BYTE> m_cBuf;
		LONG const m_nSizeX;
		LONG const m_nSizeY;
		int const m_nMaxTolerance;
		int const m_nMinTolerance;
		int const m_nFactor;
	};

	struct MultiPointComparer : public PixelComparer
	{
		MultiPointComparer(int a_nComparers) :
			m_aComparers(new PixelComparer*[a_nComparers]),
			m_nComparers(a_nComparers)
		{
			ZeroMemory(m_aComparers, a_nComparers*sizeof*m_aComparers);
		}
		~MultiPointComparer()
		{
			for (int i = 0; i < m_nComparers; ++i)
				delete m_aComparers[i];
			delete[] m_aComparers;
		}

		int operator()(TRasterImagePixel const a_tPixel, LONG const a_nX, LONG const a_nY) const
		{
			int nVal = (*(m_aComparers[0]))(a_tPixel, a_nX, a_nY);
			for (int i = 1; i < m_nComparers; ++i)
			{
				int n = (*(m_aComparers[i]))(a_tPixel, a_nX, a_nY);
				if (n < nVal)
					nVal = n;
			}
			return nVal;
		}
		void SetComparer(int a_nIndex, PixelComparer* a_pComparer)
		{
			m_aComparers[a_nIndex] = a_pComparer;
		}

	private:
		PixelComparer** m_aComparers;
		int const m_nComparers;
	};

	typedef std::vector<POINT> CSeeds;

private:
	template<class TIntervals>
	inline static void LinearFill(TRasterImagePixel const* const a_pC, BYTE* const a_pF, ULONG const a_nSizeX, ULONG const a_nX, ULONG const a_nY, PixelComparer const& a_tComparer, int const a_nTolerance, TIntervals& a_cQueue)
	{
		SInterval sI;
		sI.nY = a_nY;
		sI.nX1 = a_nX;
        do
		{
            a_pF[sI.nX1] = 255;
			--sI.nX1;
		}
		while (sI.nX1 != ULONG(-1) && a_pF[sI.nX1] == 0 && a_tComparer(a_pC[sI.nX1], sI.nX1, a_nY) <= a_nTolerance);
		++sI.nX1;

		sI.nX2 = a_nX+1;
		while (sI.nX2 < LONG(a_nSizeX) && a_pF[sI.nX2] == 0 && a_tComparer(a_pC[sI.nX2], sI.nX2, a_nY) <= a_nTolerance)
        {
            a_pF[sI.nX2] = 255;
			++sI.nX2;
		}
		a_cQueue.push_back(sI);
	}
	void PrepareFloodFill()
	{
		m_rcDirty = RECT_EMPTY;
		m_nDirtyPixels = 0;
		m_cIntervals.clear();
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
		int const nTolerance = m_cData.nTolerance*0xffff/100;
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
			case CEditToolDataMagicWand::EMMBrightness:
				pComparer.Attach(new BrightnessComparer(tMatch));
				break;
			case CEditToolDataMagicWand::EMMAlpha:
				pComparer.Attach(new AlphaComparer(tMatch));
				break;
			case CEditToolDataMagicWand::EMMHue:
				pComparer.Attach(new HueComparer(tMatch));
				break;
			default:
				pComparer.Attach(m_cData.nTolerance ?
					static_cast<PixelComparer*>(new RGBAComparer(tMatch)) :
					static_cast<PixelComparer*>(new ZeroToleranceRGBAComparer(tMatch))
					);
				break;
			}
		}
		else
		{
			MultiPointComparer* pMPC = new MultiPointComparer(cSeeds.size());
			pComparer.Attach(pMPC);
			for (CSeeds::const_iterator i = cSeeds.begin(); i != cSeeds.end(); ++i)
			{
				TRasterImagePixel tMatch = cBuffer[i->x+nSizeX*i->y];
				switch (m_cData.eMode)
				{
				case CEditToolDataMagicWand::EMMBrightness:
					pMPC->SetComparer(i-cSeeds.begin(), new BrightnessComparer(tMatch));
					break;
				case CEditToolDataMagicWand::EMMAlpha:
					pMPC->SetComparer(i-cSeeds.begin(), new AlphaComparer(tMatch));
					break;
				case CEditToolDataMagicWand::EMMHue:
					pMPC->SetComparer(i-cSeeds.begin(), new HueComparer(tMatch));
					break;
				default:
					pMPC->SetComparer(i-cSeeds.begin(), m_cData.nTolerance ?
						static_cast<PixelComparer*>(new RGBAComparer(tMatch)) :
						static_cast<PixelComparer*>(new ZeroToleranceRGBAComparer(tMatch))
						);
					break;
				}
			}
		}
		if (m_cData.bEnergy && m_cData.nTolerance > 0)
		{
			EnergyComparer* p = new EnergyComparer(pComparer.m_p, cBuffer.m_p, nSizeX, nSizeY, nTolerance);
			pComparer.Detach();
			pComparer.Attach(p);
		}
		for (CSeeds::const_iterator i = cSeeds.begin(); i != cSeeds.end(); ++i)
		{
			ULONG nX = i->x;
			ULONG nY = i->y;
			if (cFilled[nY*nSizeX+nX])
				continue;
			LinearFill(cBuffer.m_p+nY*nSizeX, cFilled.m_p+nY*nSizeX, nSizeX, nX, nY, *pComparer.m_p, nTolerance, m_cIntervals);
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
						if (pF[x] == 0 && (*pComparer.m_p)(pC[x], x, sInt.nY) < nTolerance)
							LinearFill(pC, pF, nSizeX, x, sInt.nY, *pComparer.m_p, nTolerance, m_cIntervals);
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
						if (pF[x] == 0 && (*pComparer.m_p)(pC[x], x, sInt.nY) < nTolerance)
							LinearFill(pC, pF, nSizeX, x, sInt.nY, *pComparer.m_p, nTolerance, m_cIntervals);
					}
				}
			}
		}
		if (m_cData.bSmooth && m_rcDirty.left < m_rcDirty.right-3 && m_rcDirty.top < m_rcDirty.bottom-3)
		{
			m_cIntervals.clear();
			RECT rc = m_rcDirty;
			m_rcDirty = RECT_EMPTY;
			m_nDirtyPixels = 0;
			rc.top = rc.top > 3 ? rc.top-3 : 0;
			rc.left = rc.left > 3 ? rc.left-3 : 0;
			rc.bottom = rc.bottom < LONG(nSizeY-3) ? rc.bottom+3 : nSizeY;
			rc.right = rc.right < LONG(nSizeX-3) ? rc.right+3 : nSizeX;
			CEditToolShadow::SGray sBuffer(cFilled.m_p+rc.top*nSizeX+rc.left, sizeof(*cFilled.m_p)*nSizeX, rc.right-rc.left, rc.bottom-rc.top);
			agg::stack_blur_gray8(sBuffer, 3, 3);
			SInterval sI;
			for (LONG y = rc.top; y < rc.bottom; ++y)
			{
				bool bInInterval = false;
				BYTE const* p = cFilled.m_p+y*nSizeX+rc.left;
				sI.nY = y;
				for (LONG x = rc.left; x < rc.right; ++x, ++p)
				{
					if (bInInterval)
					{
						if (*p <= 0x6f)
						{
							sI.nX2 = x;
							if (m_rcDirty.left > LONG(sI.nX1)) m_rcDirty.left = sI.nX1;
							if (m_rcDirty.top > LONG(sI.nY)) m_rcDirty.top = sI.nY;
							if (m_rcDirty.right < LONG(sI.nX2)) m_rcDirty.right = sI.nX2;
							if (m_rcDirty.bottom <= LONG(sI.nY)) m_rcDirty.bottom = sI.nY+1;
							m_nDirtyPixels += sI.nX2-sI.nX1;
							m_cIntervals.push_back(sI);
							bInInterval = false;
						}
					}
					else
					{
						if (*p > 0x6f)
						{
							sI.nX1 = x;
							bInInterval = true;
						}
					}
				}
				if (bInInterval)
				{
					sI.nX2 = rc.right;
					if (m_rcDirty.left > LONG(sI.nX1)) m_rcDirty.left = sI.nX1;
					if (m_rcDirty.top > LONG(sI.nY)) m_rcDirty.top = sI.nY;
					if (m_rcDirty.right < LONG(sI.nX2)) m_rcDirty.right = sI.nX2;
					if (m_rcDirty.bottom <= LONG(sI.nY)) m_rcDirty.bottom = sI.nY+1;
					m_nDirtyPixels += sI.nX2-sI.nX1;
					m_cIntervals.push_back(sI);
				}
			}
			if (m_cIntervals.empty())
			{
				// smoothing deleted the intervals -> add at least the seeds
				for (CSeeds::const_iterator i = cSeeds.begin(); i != cSeeds.end(); ++i)
				{
					SInterval sI = {i->y, i->x, i->x+1};
					if (m_rcDirty.left > LONG(sI.nX1)) m_rcDirty.left = sI.nX1;
					if (m_rcDirty.top > LONG(sI.nY)) m_rcDirty.top = sI.nY;
					if (m_rcDirty.right < LONG(sI.nX2)) m_rcDirty.right = sI.nX2;
					if (m_rcDirty.bottom <= LONG(sI.nY)) m_rcDirty.bottom = sI.nY+1;
					m_nDirtyPixels += sI.nX2-sI.nX1;
					m_cIntervals.push_back(sI);
				}
			}
		}
	}

	void RefreshImage()
	{
		RECT rcPrev = m_rcDirty;
		bool bPrevEmpty = m_cIntervals.empty();
		PrepareFloodFill();
		if ((m_cIntervals.empty() || bPrevEmpty) && m_eKeysState == ECKSNone)
		{
			rcPrev.left = rcPrev.top = 0;
			rcPrev.right = m_nSizeX;
			rcPrev.bottom = m_nSizeY;
		}
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
	CIntervals m_cIntervals;
	RECT m_rcDirty;
	ULONG m_nDirtyPixels;
	CEditToolDataMagicWand m_cData;
	EControlKeysState m_eKeysState;
};
