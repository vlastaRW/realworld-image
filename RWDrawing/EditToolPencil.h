
#pragma once

#include "EditTool.h"
#include "EditToolPixelBuffer.h"
#include "EditToolWithCtrlDropper.h"
#include "EditToolWithBrush.h"
#include <math.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataPencil
{
	MIDL_INTERFACE("FF9BF2E9-F11B-4F89-9E4C-2F54DEE82527")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataPencil const*, InternalData)() = 0;
	};

	enum EShape
	{
		ESSingle = 0,
		ESSquare3,
		ESSquare5,
		ESCircle3,
		ESCircle5,
		ESDiamond5,
		ESNESW3,
		ESNESW5,
		ESCount
	};

	CEditToolDataPencil() : eShape(ESSingle)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		int i = 0;
		if (a_bstr && 1 == swscanf(a_bstr, L"%i", &i))
		{
			if (i >= 0 && i < ESCount)
				eShape = static_cast<EShape>(i);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[16];
		swprintf(szTmp, L"%i", int(eShape));
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	EShape eShape;
};

struct SPencilShape
{
	BYTE coverage[5*5];
};
extern __declspec(selectany) SPencilShape const g_aPencilShapes[CEditToolDataPencil::ESCount] =
{
	{0, 0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 255, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0,  0, 255, 255, 255, 0,  0, 255, 255, 255, 0,  0, 255, 255, 255, 0,  0, 0, 0, 0, 0},
	{255, 255, 255, 255, 255,  255, 255, 255, 255, 255,  255, 255, 255, 255, 255,  255, 255, 255, 255, 255,  255, 255, 255, 255, 255},
	{0, 0, 0, 0, 0,  0, 0, 255, 0, 0,  0, 255, 255, 255, 0,  0, 0, 255, 0, 0,  0, 0, 0, 0, 0},
	{0, 255, 255, 255, 0,  255, 255, 255, 255, 255,  255, 255, 255, 255, 255,  255, 255, 255, 255, 255,  0, 255, 255, 255, 0},
	{0, 0, 255, 0, 0,  0, 255, 255, 255, 0,  255, 255, 255, 255, 255,  0, 255, 255, 255, 0,  0, 0, 255, 0, 0},
	{0, 0, 0, 0, 0,  0, 255, 255, 0, 0,  0, 255, 255, 255, 0,  0, 0, 255, 255, 0,  0, 0, 0, 0, 0},
	{255, 255, 0, 0, 0,  255, 255, 255, 0, 0,  0, 255, 255, 255, 0,  0, 0, 255, 255, 255,  0, 0, 0, 255, 255},
};
struct SPencilOutline
{
	char n;
	struct {signed char x, y;} pt[20];
};
extern __declspec(selectany) SPencilOutline const g_aPencilOutlines[CEditToolDataPencil::ESCount] =
{
	{4, {{0, 0}, {1, 0}, {1, 1}, {0, 1}}},
	{4, {{-1, -1}, {2, -1}, {2, 2}, {-1, 2}}},
	{4, {{-2, -2}, {3, -2}, {3, 3}, {-2, 3}}},
	{12, {{0, -1}, {1, -1}, {1, 0}, {2, 0}, {2, 1}, {1, 1}, {1, 2}, {0, 2}, {0, 1}, {-1, 1}, {-1, 0}, {0, 0}}},
	{12, {{-1, -2}, {2, -2}, {2, -1}, {3, -1}, {3, 2}, {2, 2}, {2, 3}, {-1, 3}, {-1, 2}, {-2, 2}, {-2, -1}, {-1, -1}}},
	{20, {{0, -2}, {1, -2}, {1, -1}, {2, -1}, {2, 0}, {3, 0}, {3, 1}, {2, 1}, {2, 2}, {1, 2}, {1, 3}, {0, 3}, {0, 2}, {-1, 2}, {-1, 1}, {-2, 1}, {-2, 0}, {-1, 0}, {-1, -1}, {0, -1}}},
	{8, {{-1, -1}, {1, -1}, {1, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 1}, {-1, 1}}},
	{16, {{-2, -2}, {0, -2}, {0, -1}, {1, -1}, {1, 0}, {2, 0}, {2, 1}, {3, 1}, {3, 3}, {1, 3}, {1, 2}, {0, 2}, {0, 1}, {-1, 1}, {-1, 0}, {-2, 0}}},
};

#include "EditToolPencilDlg.h"


class CEditToolPencil :
	public CEditToolPixelCoverageBuffer<CEditToolPencil>, // pixel coverage cache
	public CEditToolMouseInput<CEditToolPencil>, // no direct tablet support
	public CEditToolWithSolidBrush<CEditToolPencil>, // colors handling
	public CEditToolCustomPrecisionCursor<>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolPencil, CEditToolMouseInput<CEditToolPencil>, CEditToolPencil, CEditToolCustomPrecisionCursor<> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolPencil, // T - the top level class for cross casting
		CEditToolPencil, // TResetHandler
		CEditToolPixelCoverageBuffer<CEditToolPencil>, // TDirtyHandler
		CEditToolPixelCoverageBuffer<CEditToolPencil>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlinesHandler
		CEditToolWithSolidBrush<CEditToolPencil>, // TBrushHandler
		CEditToolPencil, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolPencil, CEditToolMouseInput<CEditToolPencil>, CEditToolPencil, CEditToolCustomPrecisionCursor<> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolPencil, CEditToolMouseInput<CEditToolPencil>, CEditToolPencil, CEditToolCustomPrecisionCursor<> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolPencil, CEditToolMouseInput<CEditToolPencil>, CEditToolPencil, CEditToolCustomPrecisionCursor<> >, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CEditToolPencil // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolPencil() : m_eBlendingMode(EBMDrawOver), m_bMouseLeft(true)
	{
	}

	BEGIN_COM_MAP(CEditToolPencil)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
	}
	void PrepareShape()
	{
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		m_bMouseLeft = true;
		RECT const rc = M_DirtyRect();
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataPencil::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			M_Window()->RectangleChanged(&M_DirtyRect());
		}
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		a_pPos->fX = floorf(a_pPos->fX+0.5f*a_pPointerSize->fX);
		a_pPos->fY = floorf(a_pPos->fY+0.5f*a_pPointerSize->fY);

		return S_OK;
	}

	void DrawLine(LONG a_nX1, LONG a_nY1, LONG a_nX2, LONG a_nY2)
	{
		float aDelta[2] = { a_nX2-a_nX1, a_nY2-a_nY1 };
		int aSigns[2];
		ULONG i;
		for (i = 0; i < itemsof(aDelta); i++)
		{
			if (aDelta[i] >= 0.0f)
			{
				aSigns[i] = 1;
			}
			else
			{
				aDelta[i] = -aDelta[i];
				aSigns[i] = -1;
			}
		}
		if (aDelta[0] >= aDelta[1])
		{
			float fStep = aDelta[1]/aDelta[0];
			int nPixels = static_cast<int>(aDelta[0]+1.5f);
			int i;
			for (i = 0; i < nPixels; i++)
			{
				AddPencilPixel(a_nX1+i*aSigns[0], a_nY1+static_cast<int>((i*fStep+0.5f)*aSigns[1]));
			}
		}
		else
		{
			float fStep = aDelta[0]/aDelta[1];
			int nPixels = static_cast<int>(aDelta[1]+1.5f);
			int i;
			for (i = 0; i < nPixels; i++)
			{
				AddPencilPixel(a_nX1+static_cast<int>((i*fStep+0.5f)*aSigns[0]), a_nY1+i*aSigns[1]);
			}
		}
	}
	void AddPencilPixel(LONG a_nX, LONG a_nY)
	{
		if (m_cData.eShape == CEditToolDataPencil::ESSingle)
		{
			AddPixel(a_nX, a_nY, 255);
		}
		else
		{
			BYTE const* p = g_aPencilShapes[m_cData.eShape].coverage;
			for (LONG y = -2; y < 3; ++y)
				for (LONG x = -2; x < 3; ++x, ++p)
					if (*p)
						AddPixel(a_nX+x, a_nY+y, *p);
		}
	}
	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (CEditToolPixelCoverageBuffer<CEditToolPencil>::IsDirty())
		{
			ATLASSERT(0);
			CEditToolPixelCoverageBuffer<CEditToolPencil>::DeleteCachedData();
		}

		DeleteCachedData();
		m_tCursorPos = m_tLastPos = *a_pPos;
		AddPencilPixel(m_tLastPos.fX, m_tLastPos.fY);
		RECT rcChg;
		rcChg.left = m_tLastPos.fX;
		rcChg.top = m_tLastPos.fY;
		rcChg.right = rcChg.left+1;
		rcChg.bottom = rcChg.top+1;
		if (m_cData.eShape != CEditToolDataPencil::ESSingle)
		{
			rcChg.left -= 2;
			rcChg.top -= 2;
			rcChg.right += 2;
			rcChg.bottom += 2;
		}
		M_Window()->RectangleChanged(&rcChg);
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_tCursorPos.fX != m_tLastPos.fX || m_tCursorPos.fY != m_tLastPos.fY)
			OnMouseMove(ECKSNone, a_pPos);
		return ETPAApply;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		LONG nX2 = a_pPos->fX;
		LONG nY2 = a_pPos->fY;
		if (M_Dragging() && a_eKeysState&ECKSShift && !m_bMouseLeft)
		{
			LONG nX1 = m_tCursorPos.fX;
			LONG nY1 = m_tCursorPos.fY;
			if (nX1 != nX2 || nY1 != nY2)
			{
				M_Window()->ControlLinesChanged();
				m_tCursorPos = *a_pPos;
			}
			return S_OK;
		}
		LONG nX1 = m_tLastPos.fX;
		LONG nY1 = m_tLastPos.fY;
		m_tCursorPos = m_tLastPos = *a_pPos;

		if (nX1 != nX2 || nY1 != nY2)
		{
			if (M_Dragging())
			{
				DrawLine(nX1, nY1, nX2, nY2);
				RECT rc = {min(nX1, nX2), min(nY1, nY2), max(nX2, nX1)+1, max(nY2, nY1)+1};
				if (m_cData.eShape != CEditToolDataPencil::ESSingle)
				{
					rc.left -= 2;
					rc.top -= 2;
					rc.right += 2;
					rc.bottom += 2;
				}
				M_Window()->RectangleChanged(&rc);
			}
			M_Window()->ControlLinesChanged();
		}
		else if (m_bMouseLeft)
		{
			M_Window()->ControlLinesChanged();
		}
		m_bMouseLeft = false;
		return S_OK;
	}
	HRESULT OnMouseLeave()
	{
		if (!m_bMouseLeft)
		{
			m_bMouseLeft = true;
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}

	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_bMouseLeft)
			return S_FALSE;
		if (m_tCursorPos.fX != m_tLastPos.fX || m_tCursorPos.fY != m_tLastPos.fY)
		{
			a_pLines->MoveTo(m_tCursorPos.fX+0.5f, m_tCursorPos.fY+0.5f);
			a_pLines->LineTo(m_tLastPos.fX+0.5f, m_tLastPos.fY+0.5f);
		}
		float fHandleSize = 10.0f;
		a_pLines->HandleSize(&fHandleSize);
		if (fHandleSize >= 1.5f)
			return S_FALSE; // only show control lines when sufficiently zoomed in
		SPencilOutline const* p = g_aPencilOutlines+m_cData.eShape;
		a_pLines->MoveTo(m_tCursorPos.fX+p->pt[0].x, m_tCursorPos.fY+p->pt[0].y);
		for (char i = 1; i < p->n; ++i)
			a_pLines->LineTo(m_tCursorPos.fX+p->pt[i].x, m_tCursorPos.fY+p->pt[i].y);
		a_pLines->Close();

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

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CEditToolDataPencil::EShape eShape = CEditToolDataPencil::ESSingle;
			std::vector<TPixelCoords> cPolyLine;
			TPixelCoords t;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(!((str_p(L"SINGLE")[assign_a(eShape, CEditToolDataPencil::ESSingle)]|str_p(L"SQUARE3")[assign_a(eShape, CEditToolDataPencil::ESSquare3)]|str_p(L"SQUARE5")[assign_a(eShape, CEditToolDataPencil::ESSquare5)]|
						str_p(L"ROUND3")[assign_a(eShape, CEditToolDataPencil::ESCircle3)]|str_p(L"ROUND5")[assign_a(eShape, CEditToolDataPencil::ESCircle5)]|str_p(L"DIAMOND5")[assign_a(eShape, CEditToolDataPencil::ESDiamond5)]|
						str_p(L"NESW3")[assign_a(eShape, CEditToolDataPencil::ESNESW3)]|str_p(L"NESW5")[assign_a(eShape, CEditToolDataPencil::ESNESW5)]) >> ch_p(L',')))>>
					(*((real_p[assign_a(t.fX)]>>ch_p(L',')>>real_p[assign_a(t.fY)]>>ch_p(L','))[push_back_a(cPolyLine, t)]))>>
					((real_p[assign_a(t.fX)]>>ch_p(L',')>>real_p[assign_a(t.fY)])[push_back_a(cPolyLine, t)])
					).full;
			if (!bParsed || cPolyLine.size() < 2)
				return E_INVALIDARG;
			m_cData.eShape = eShape;
			LONG nPrevX = 0;
			LONG nPrevY = 0;
			UpdateCache();
			for (std::vector<TPixelCoords>::const_iterator i = cPolyLine.begin(); i != cPolyLine.end(); ++i)
			{
				LONG nX = i->fX+0.5f;
				LONG nY = i->fY+0.5f;
				if (i == cPolyLine.begin())
					AddPencilPixel(nX, nY);
				else
					DrawLine(nPrevX, nPrevY, nX, nY);
				nPrevX = nX;
				nPrevY = nY;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
				//bstr = m_cData.eShape == ESSquare3 ? L"SQUARE3" : (
				//	m_cData.eShape == ESSquare5 ? L"SQUARE5" : (
				//	m_cData.eShape == ESCircle3 ? L"ROUND3" : (
				//	m_cData.eShape == ESCircle5 ? L"ROUND5" : (
				//	m_cData.eShape == ESDiamond5 ? L"DIAMOND5" : (
				//	m_cData.eShape == ESNESW3 ? L"NESW3" : (
				//	m_cData.eShape == ESNESW5 ? L"NESW5" : L"SINGLE"))))));
		return E_NOTIMPL;
	}

private:
	TPixelCoords m_tLastPos;
	TPixelCoords m_tCursorPos;
	EBlendingMode m_eBlendingMode;
	CEditToolDataPencil m_cData;
	bool m_bMouseLeft;
};

