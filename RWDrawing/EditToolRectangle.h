
#pragma once


#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithBrush.h"
#include "EditToolRectangularShape.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rounded_rect.h>
#include <agg_trans_affine.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_span_gradient.h>
#include <agg_conv_stroke.h>
#include <agg_conv_transform.h>

#include <XPGUI.h>


struct CEditToolDataRectangle
{
	MIDL_INTERFACE("055E3C6A-32F6-41FF-9CAA-1E82E6B1746E")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataRectangle const*, InternalData)() = 0;
	};
	enum EMode
	{
		EMSquare = 0,
		EMRectangle,
		//EMTiltedRectangle,
	};

	CEditToolDataRectangle() : eMode(EMRectangle), fCornerRadius(0.0f)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		swscanf(a_bstr, L"%f", &fCornerRadius);
		if (wcsstr(a_bstr, L"SQUARE"))
			eMode = EMSquare;
		else if (wcsstr(a_bstr, L"RECTANGLE"))
			eMode = EMRectangle;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[64] = L"";
		swprintf(szTmp, L"%g|%s", fCornerRadius, eMode == EMSquare ? L"SQUARE" : L"RECTANGLE");
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	EMode eMode;
	float fCornerRadius;
};

#include "EditToolRectangleDlg.h"


HICON GetToolIconRECTANGLE(ULONG a_nSize);

class CEditToolRectangle :
	public CEditToolScanlineBuffer<CEditToolRectangle>, // scanline image cache
	public CEditToolRectangularShape<CEditToolRectangle>, // creating and modifying the rectangle
	public CEditToolMouseInput<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle> >, // no direct tablet support
	public CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, // brush override
	public CEditToolOutline<CEditToolRectangle>, // outline handling
	public CEditToolCustomOrMoveCursor<CEditToolRectangle, GetToolIconRECTANGLE>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolRectangle, CEditToolMouseInput<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle> >, CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, CEditToolCustomOrMoveCursor<CEditToolRectangle, GetToolIconRECTANGLE> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolRectangle, // T - the top level class for cross casting
		CEditToolRectangle, // TResetHandler
		CEditToolScanlineBuffer<CEditToolRectangle>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolRectangle>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolOutline<CEditToolRectangle>, // TOutlineHandler
		CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, // TBrushHandler
		CEditToolRectangle, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolRectangle, CEditToolMouseInput<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle> >, CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, CEditToolCustomOrMoveCursor<CEditToolRectangle, GetToolIconRECTANGLE> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolRectangle, CEditToolMouseInput<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle> >, CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, CEditToolCustomOrMoveCursor<CEditToolRectangle, GetToolIconRECTANGLE> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolRectangle, CEditToolMouseInput<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle> >, CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, CEditToolCustomOrMoveCursor<CEditToolRectangle, GetToolIconRECTANGLE> >, // TProcessInputHandler
		CEditToolRectangularShape<CEditToolRectangle>, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolRectangle, CEditToolRectangularShape<CEditToolRectangle>, CEditToolRectangle> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IDesignerViewStatusBar,
	public IRasterImageEditToolPolygon
{
public:
	CEditToolRectangle() : m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		CEditToolRectangularShape<CEditToolRectangle>(CEditToolRectangularShape::ERMRotatedRectangle)
	{
	}

	BEGIN_COM_MAP(CEditToolRectangle)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
		COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
	END_COM_MAP()

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
	}
	ECoordinatesMode M_CoordinatesMode() const
	{
		return m_eCoordinatesMode;
	}
	bool is_solid(unsigned a_nLayer)
	{
		return a_nLayer ? true : S_OK == M_Brush()->IsSolid(NULL);
	}
	TPixelChannel color(unsigned a_nLayer) const
	{
		if (a_nLayer)
		{
			TRasterImagePixel t = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			return CPixelChannel(t.bR, t.bG, t.bB, t.bA);
		}
		else
		{
			TRasterImagePixel tSolid;
			M_Brush()->GetBrushTile(0, 0, 1, 1, M_Gamma(), 1, &tSolid);
			return CPixelChannel(tSolid.bR, tSolid.bG, tSolid.bB, tSolid.bA);
		}
	}
    void generate_span(TPixelChannel* span, int x, int y, unsigned len, unsigned style)
    {
		M_Brush()->GetBrushTile(x, y, len, 1, M_Gamma(), len, reinterpret_cast<TRasterImagePixel*>(span));
    }
	void generate(TPixelChannel* span, int x, int y, unsigned len)
	{
		M_Brush()->GetBrushTile(x, y, len, 1, M_Gamma(), len, reinterpret_cast<TRasterImagePixel*>(span));
	}
	void prepare()
	{
	}
	void PrepareShape()
	{
		TPixelCoords tCenter;
		float fSizeX;
		float fSizeY;
		float fAngle;
		if (!GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
			return;

		agg::renderer_base<CEditToolScanlineBuffer> renb(*this);
		agg::trans_affine mtx(agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));
		agg::trans_affine mtx_i(agg::trans_affine_scaling(-1.0, 1.0)*agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));

		float const fWidthIn = M_OutlineIn();
		float const fWidthOut = M_OutlineOut();
		//M_OutlineJoins()
		float const fWidth = fWidthOut-fWidthIn;
		fSizeX += fWidthOut;
		fSizeY += fWidthOut;
		float const fRadius = m_cData.fCornerRadius > 0.0f || M_OutlineJoins() == EOJTRound ? m_cData.fCornerRadius + fWidthOut : m_cData.fCornerRadius;
		if (fWidth <= 0.0f && M_Brush())
		{
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::rounded_rect outline(-fSizeX, -fSizeY, fSizeX, fSizeY, fRadius);
			outline.normalize_radius();

			agg::conv_transform<agg::rounded_rect> tr(outline, mtx);
			ras.add_path(tr);
			agg::span_allocator<TPixelChannel> span_alloc;
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::render_scanlines_aa(ras, sl, renb, span_alloc, *this);
			}
			else
			{
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines_bin(ras, sl, renb, span_alloc, *this);
			}
		}
		else if (fWidth > 0.0f && M_Brush() == NULL && fSizeX > fWidth && fSizeY > fWidth)
		{
			TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::rounded_rect ext_line(-fSizeX, -fSizeY, fSizeX, fSizeY, fRadius);
			agg::rounded_rect int_line(-fSizeX+fWidth, -fSizeY+fWidth, fSizeX-fWidth, fSizeY-fWidth, max(0.0f, fRadius-fWidth));
			ext_line.normalize_radius();
			int_line.normalize_radius();
			agg::conv_transform<agg::rounded_rect> ext_tr(ext_line, mtx);
			agg::conv_transform<agg::rounded_rect> int_tr(int_line, mtx_i);
			ras.add_path(ext_tr);
			ras.add_path(int_tr);
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				agg::render_scanlines(ras, sl, ren);
			}
			else
			{
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);
			}
		}
		else if (fWidth > 0.0f && M_Brush() && fSizeX > fWidth && fSizeY > fWidth)
		{
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::scanline_u8 sl;
				agg::rounded_rect ext_line(-fSizeX, -fSizeY, fSizeX, fSizeY, fRadius);
				agg::rounded_rect int_line(-fSizeX+fWidth, -fSizeY+fWidth, fSizeX-fWidth, fSizeY-fWidth, max(0.0f, fRadius-fWidth));
				ext_line.normalize_radius();
				int_line.normalize_radius();
				agg::conv_transform<agg::rounded_rect> ext_tr(ext_line, mtx);
				agg::conv_transform<agg::rounded_rect> int_tr(int_line, mtx_i);

				agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;
				rasc.styles(1, -1);
				rasc.add_path(ext_tr);
				rasc.add_path(int_tr);
				rasc.styles(0, -1);
				rasc.add_path(int_tr);

				agg::span_allocator<TPixelChannel> alloc;
				agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, *this, M_LayerBlender());
			}
			else
			{
				TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
				agg::scanline_p8 sl;
				agg::rasterizer_scanline_aa<> ras;
				agg::rounded_rect ext_line(-fSizeX, -fSizeY, fSizeX, fSizeY, fRadius);
				agg::rounded_rect int_line(-fSizeX+fWidth, -fSizeY+fWidth, fSizeX-fWidth, fSizeY-fWidth, max(0.0f, fRadius-fWidth));
				ext_line.normalize_radius();
				int_line.normalize_radius();
				agg::conv_transform<agg::rounded_rect> ext_tr(ext_line, mtx);
				agg::conv_transform<agg::rounded_rect> int_tr(int_line, mtx_i);
				ras.add_path(ext_tr);
				ras.add_path(int_tr);
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);

				ras.reset();
				agg::conv_transform<agg::rounded_rect> int_fill(int_line, mtx);
				ras.add_path(int_fill);
				agg::span_allocator<TPixelChannel> span_alloc;
				ras.gamma(agg::gamma_threshold(0.502));
				agg::render_scanlines_bin(ras, sl, renb, span_alloc, *this);
			}
		}
		else // solid fill with outline color
		{
			TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::rounded_rect outline(-fSizeX, -fSizeY, fSizeX, fSizeY, fRadius);
			outline.normalize_radius();

			agg::conv_transform<agg::rounded_rect> tr(outline, mtx);
			ras.add_path(tr);
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				agg::render_scanlines(ras, sl, ren);
			}
			else
			{
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);
			}
		}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		RECT const rc = M_DirtyRect();
		ResetRectangle();
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY);
		ResetDragging();
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		EnableBrush(false);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataRectangle::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			float const fPrevRadius = m_cData.fCornerRadius;
			m_cData = *(pData->InternalData());
			if (SetRectangleMode(m_cData.eMode == CEditToolDataRectangle::EMRectangle ? CEditToolRectangularShape::ERMRotatedRectangle : CEditToolRectangularShape::ERMRotatedSquare) ||
				(RectangleDefined() && fPrevRadius != m_cData.fCornerRadius))
			{
				RECT const rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
				M_Window()->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	void OutlineChanged(bool a_bWidth, bool a_bColor)
	{
		if (RectangleDefined())
		{
			InvalidateCachePixels();
			M_Window()->RectangleChanged(&M_DirtyRect());
		}
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		bool bBlendingChange = m_eBlendingMode != a_eBlendingMode;
		bool bRasterizationChange = m_eRasterizationMode != a_eRasterizationMode;
		bool bCoordinatesChange = m_eCoordinatesMode != a_eCoordinatesMode;

		if (!bBlendingChange && !bRasterizationChange && !bCoordinatesChange)
			return S_FALSE;
		m_eBlendingMode = a_eBlendingMode;
		m_eRasterizationMode = a_eRasterizationMode;
		m_eCoordinatesMode = a_eCoordinatesMode;
		if (!RectangleDefined())
			return S_OK;
		RECT rcPrev = M_DirtyRect();
		bool bRedrawCache = bRasterizationChange;
		if (bCoordinatesChange && AdjustRectangleCoordinates())
		{
			for (ULONG i = 0; i < 9; ++i)
				M_Window()->ControlPointChanged(i);
			M_Window()->ControlLinesChanged();
			bRedrawCache = true;
		}
		if (bRedrawCache)
		{
			ShapeChanged(false);
		}
		else if (bBlendingChange)
		{
			M_Window()->RectangleChanged(&rcPrev);
		}
		return S_OK;
	}

	void ToolSetBrush()
	{
		if (M_Brush())
		{
			TPixelCoords tCenter;
			float fSizeX, fSizeY, fAngle;
			if (GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
				M_Brush()->SetShapeBounds(&tCenter, fSizeX, fSizeY, fAngle);
		}
	}

	void ShapeChanged(bool)
	{
		ToolSetBrush();
		RECT const rc = UpdateCache();
		M_Window()->RectangleChanged(&rc);
		EnableBrush(RectangleDefined() && M_State() >= ESFloating && M_Brush());
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		if (m_eCoordinatesMode == ECMIntegral)
		{
			a_pPos->fX = floorf(a_pPos->fX+0.5f);
			a_pPos->fY = floorf(a_pPos->fY+0.5f);
		}
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return RectangleMoving() || (!M_Dragging() && HitTest(a_pPos->fX, a_pPos->fY));
	}
	//void PolygonChanged()
	//{
	//	if (M_PolyLine().size() < 2)
	//	{
	//		M_PolyLine().clear();
	//		M_Window()->ControlLinesChanged();
	//		M_Window()->ControlPointsChanged();
	//	}
	//	RECT const rc = UpdateCache();
	//	M_Window()->RectangleChanged(&rc);
	//	ToolSetBrush();
	//}

	//bool ShapeHitTest(TPixelCoords const* a_pPos)
	//{
	//	TPixelCoords const t1 = M_Pixel1();
	//	TPixelCoords const t2 = {t1.fX*cosf(M_Angle())+t1.fY*sinf(M_Angle()), t1.fY*cosf(M_Angle())-t1.fX*sinf(M_Angle())};
	//	float fSizeX = t1.fX-M_Pixel2().fX;
	//	float fSizeY = t1.fY-M_Pixel2().fY;
	//	if (fabsf(t2.fX) > fSizeX || fabsf(t2.fY) > fSizeY)
	//		return false;
	//	float const fRad = (m_cData.fCornerRadius < m_fSizeX) ? (m_cData.fCornerRadius < m_fSizeY ? m_cData.fCornerRadius : m_fSizeY) : (m_fSizeX < m_fSizeY ? m_fSizeX : m_fSizeY);
	//	if (fabsf(t2.fX) <= (m_fSizeX-fRad) || fabsf(t2.fY) <= (m_fSizeY-fRad))
	//		return true;
	//	return ((fabsf(t2.fX)-(m_fSizeX-fRad))*(fabsf(t2.fX)-(m_fSizeX-fRad)) +
	//			(fabsf(t2.fY)-(m_fSizeY-fRad))*(fabsf(t2.fY)-(m_fSizeY-fRad))) <= (fRad*fRad);
	//}

	//STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	//{
	//	if (m_eState == ESMoving || (m_eState == ESFloating && ShapeHitTest(a_pPos)))
	//	{
	//		static HCURSOR hMove = ::LoadCursor(NULL, IDC_SIZEALL);
	//		*a_phCursor = hMove;
	//		return S_OK;
	//	}
	//	else
	//	{
	//		static HCURSOR hText = ::LoadCursor(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDC_EDITTOOL_RECTANGLE));
	//		*a_phCursor = hText;
	//		return S_OK;
	//	}
	//}

	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		InvalidateCachePixels();
		if (a_pChanged == NULL)
			return M_Window()->RectangleChanged(&M_DirtyRect());
		RECT rcShape = M_DirtyRect();
		if (rcShape.left < a_pChanged->left) rcShape.left = a_pChanged->left;
		if (rcShape.top < a_pChanged->top) rcShape.top = a_pChanged->top;
		if (rcShape.right > a_pChanged->right) rcShape.right = a_pChanged->right;
		if (rcShape.bottom > a_pChanged->bottom) rcShape.bottom = a_pChanged->bottom;
		return M_Window()->RectangleChanged(&rcShape);
	}
	HRESULT _GetSelectionControlLines(IEditToolControlLines* a_pLines)
	{
		TPixelCoords tCenter;
		float fSizeX;
		float fSizeY;
		float fAngle;
		if (!GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
			return S_FALSE;

		agg::trans_affine mtx(agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));
		agg::trans_affine mtx_i(agg::trans_affine_scaling(-1.0, 1.0)*agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));

		agg::rounded_rect outline(-fSizeX, -fSizeY, fSizeX, fSizeY, m_cData.fCornerRadius);
		outline.normalize_radius();
		agg::conv_transform<agg::rounded_rect> tr(outline, mtx);
		double x;
		double y;
		unsigned cmd;
		tr.rewind(0);
		while (!agg::is_stop(cmd = tr.vertex(&x, &y)))
			if (agg::is_move_to(cmd))
				a_pLines->MoveTo(x, y);
			else if (agg::is_vertex(cmd))
				a_pLines->LineTo(x, y);
		a_pLines->Close();
		return S_OK;
	}

	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return a_eKeysState&ECKSControl ? ETPAMissed : HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		float fScale = sqrtf(sqrtf(a_pMatrix->_11*a_pMatrix->_11+a_pMatrix->_12*a_pMatrix->_12)*sqrtf(a_pMatrix->_21*a_pMatrix->_21+a_pMatrix->_22*a_pMatrix->_22));
		m_cData.fCornerRadius *= fScale;
		TransformRectangle(a_pMatrix);
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
			float fRadius = 0.0f;
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fSizeX = 0.0f;
			float fSizeY = 0.0f;
			float fAngle = 0.0f;
			CEditToolDataRectangle::EMode eMode = CEditToolDataRectangle::EMRectangle;
			int nParams = swscanf(a_bstrParams, L"%f,%f,%f,%f,%f,%f", &fRadius, &fCenterX, &fCenterY, &fSizeX, &fSizeY, &fAngle);
			switch (nParams)
			{
			case 4:
				fSizeY = fSizeX;
				eMode = CEditToolDataRectangle::EMSquare;
			case 5:
			case 6:
				{
					if (fAngle != 0.0f && fSizeY == fSizeX)
						eMode = CEditToolDataRectangle::EMSquare;
					bool const bChange = m_cData.fCornerRadius != fRadius || m_cData.eMode != eMode;
					m_cData.fCornerRadius = fRadius;
					if (m_cData.eMode == eMode)
					{
						m_cData.eMode = eMode;
						SetRectangleMode(m_cData.eMode == CEditToolDataRectangle::EMRectangle ? CEditToolRectangularShape::ERMRotatedRectangle : CEditToolRectangularShape::ERMRotatedSquare);
					}
					SetRectangle(fCenterX, fCenterY, fSizeX, fSizeY, fAngle);
					if (bChange)
					{
						CComObject<CSharedStateToolData>* pNew = NULL;
						CComObject<CSharedStateToolData>::CreateInstance(&pNew);
						CComPtr<ISharedState> pTmp = pNew;
						pNew->Init(m_cData);
						M_Window()->SetState(pTmp);
					}
				}
				EnableBrush(true);
				return S_OK;
			}
			return E_FAIL;
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
			TPixelCoords tCenter;
			float fSizeX;
			float fSizeY;
			float fAngle;
			if (GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
			{
				OLECHAR sz[64];
				swprintf(sz, L"%g, %g, %g, %g", m_cData.fCornerRadius, tCenter.fX, tCenter.fY, fSizeX);
				bstr = sz;
				if (m_cData.eMode == CEditToolDataRectangle::EMRectangle || fSizeX != fSizeY || fAngle != 0.0f)
				{
					swprintf(sz, L", %g", fSizeY);
					bstr += sz;
					if (fAngle != 0.0f)
					{
						swprintf(sz, L", %g", fAngle);
						bstr += sz;
					}
				}
			}
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IRasterImageEditToolPolygon methods
public:
	STDMETHOD(FromPolygon)(ULONG a_nCount, TRWPolygon const* a_pPolygons)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* a_pConsumer)
	{
		if (a_pConsumer == NULL)
			return E_POINTER;
		CComBSTR bstr;
		TPixelCoords tCenter;
		float fSizeX;
		float fSizeY;
		float fAngle;
		if (!GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
			return S_FALSE;
		float fRad = m_cData.fCornerRadius;
		bool bHLines = true;
		bool bVLines = true;
		if (fRad >= fSizeX)
		{
			fRad = fSizeX;
			bHLines = false;
		}
		if (fRad >= fSizeY)
		{
			fRad = fSizeY;
			bVLines = false;
			if (fRad > fSizeX)
				bHLines = true;
		}

		float const fCos = cosf(fAngle);
		float const fSin = sinf(fAngle);
		float const fXCos = fSizeX*fCos;
		float const fXSin = fSizeX*fSin;
		float const fYCos = fSizeY*fCos;
		float const fYSin = fSizeY*fSin;
		float const fRCos = fRad*fCos;
		float const fRSin = fRad*fSin;

		TRWPathPoint aPoints[8];
		TRWPathPoint* p = aPoints;

		if (fRad <= 0.0f)
		{
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX-fXCos+fYSin;
			p->tPos.fY = tCenter.fY-fXSin-fYCos;
			p->tTanNext.fX = p->tTanNext.fY = p->tTanPrev.fX = p->tTanPrev.fY = 0.0f;
			++p;
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX+fYSin+fXCos;
			p->tPos.fY = tCenter.fY-fYCos+fXSin;
			p->tTanNext.fX = p->tTanNext.fY = p->tTanPrev.fX = p->tTanPrev.fY = 0.0f;
			++p;
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX+fXCos-fYSin;
			p->tPos.fY = tCenter.fY+fXSin+fYCos;
			p->tTanNext.fX = p->tTanNext.fY = p->tTanPrev.fX = p->tTanPrev.fY = 0.0f;
			++p;
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX-fYSin-fXCos;
			p->tPos.fY = tCenter.fY+fYCos-fXSin;
			p->tTanNext.fX = p->tTanNext.fY = p->tTanPrev.fX = p->tTanPrev.fY = 0.0f;
			++p;
		}
		else
		{
			if (bVLines)
			{
				p->dwFlags = 0;
				p->tPos.fX = tCenter.fX-fXCos+fYSin-fRSin;
				p->tPos.fY = tCenter.fY-fXSin-fYCos+fRCos;
				p->tTanNext.fX = 0.5522847498f*fRSin;
				p->tTanNext.fY = -0.5522847498f*fRCos;
				p->tTanPrev.fX = 0.0f;
				p->tTanPrev.fY = 0.0f;
				++p;
			}
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX-fXCos+fYSin+fRCos;
			p->tPos.fY = tCenter.fY-fXSin-fYCos+fRSin;
			p->tTanNext.fX = bHLines ? 0.0f : 0.5522847498f*fRCos;
			p->tTanNext.fY = bHLines ? 0.0f : 0.5522847498f*fRSin;
			p->tTanPrev.fX = -0.5522847498f*fRCos;
			p->tTanPrev.fY = -0.5522847498f*fRSin;
			++p;
			if (bHLines)
			{
				p->dwFlags = 0;
				p->tPos.fX = tCenter.fX+fYSin+fXCos-fRCos;
				p->tPos.fY = tCenter.fY-fYCos+fXSin-fRSin;
				p->tTanNext.fX = 0.5522847498f*fRCos;
				p->tTanNext.fY = 0.5522847498f*fRSin;
				p->tTanPrev.fX = 0.0f;
				p->tTanPrev.fY = 0.0f;
				++p;
			}
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX+fYSin+fXCos-fRSin;
			p->tPos.fY = tCenter.fY-fYCos+fXSin+fRCos;
			p->tTanNext.fX = bVLines ? 0.0f : -0.5522847498f*fRSin;
			p->tTanNext.fY = bVLines ? 0.0f : 0.5522847498f*fRCos;
			p->tTanPrev.fX = 0.5522847498f*fRSin;
			p->tTanPrev.fY = -0.5522847498f*fRCos;
			++p;
			if (bVLines)
			{
				p->dwFlags = 0;
				p->tPos.fX = tCenter.fX+fXCos-fYSin+fRSin;
				p->tPos.fY = tCenter.fY+fXSin+fYCos-fRCos;
				p->tTanNext.fX = -0.5522847498f*fRSin;
				p->tTanNext.fY = 0.5522847498f*fRCos;
				p->tTanPrev.fX = 0.0f;
				p->tTanPrev.fY = 0.0f;
				++p;
			}
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX+fXCos-fYSin-fRCos;
			p->tPos.fY = tCenter.fY+fXSin+fYCos-fRSin;
			p->tTanNext.fX = bHLines ? 0.0f : -0.5522847498f*fRCos;
			p->tTanNext.fY = bHLines ? 0.0f : -0.5522847498f*fRSin;
			p->tTanPrev.fX = 0.5522847498f*fRCos;
			p->tTanPrev.fY = 0.5522847498f*fRSin;
			++p;
			if (bHLines)
			{
				p->dwFlags = 0;
				p->tPos.fX = tCenter.fX-fYSin-fXCos+fRCos;
				p->tPos.fY = tCenter.fY+fYCos-fXSin+fRSin;
				p->tTanNext.fX = -0.5522847498f*fRCos;
				p->tTanNext.fY = -0.5522847498f*fRSin;
				p->tTanPrev.fX = 0.0f;
				p->tTanPrev.fY = 0.0f;
				++p;
			}
			p->dwFlags = 0;
			p->tPos.fX = tCenter.fX-fYSin-fXCos+fRSin;
			p->tPos.fY = tCenter.fY+fYCos-fXSin-fRCos;
			p->tTanNext.fX = bVLines ? 0.0f : 0.5522847498f*fRSin;
			p->tTanNext.fY = bVLines ? 0.0f : -0.5522847498f*fRCos;
			p->tTanPrev.fX = -0.5522847498f*fRSin;
			p->tTanPrev.fY = 0.5522847498f*fRCos;
			++p;
		}

		TRWPath tPath;
		tPath.nVertices = p-aPoints;
		tPath.pVertices = aPoints;
		return a_pConsumer->FromPath(1, &tPath);
	}

	// IDesignerViewStatusBar methods
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		TPixelCoords tCenter;
		float fSizeX;
		float fSizeY;
		float fAngle;
		if (GetRectangle(&tCenter, &fSizeX, &fSizeY, &fAngle))
		{
			static HICON hIcon = (HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_STATUS_RECTSIZE), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR|LR_SHARED);
			wchar_t sz[128];
			swprintf(sz, L"%g x %g", int(fSizeX*20.0f+0.5f)*0.1f, int(fSizeY*20.0f+0.5f)*0.1f);
			a_pStatusBar->PaneSet(CComBSTR(L"RECTSIZE"), hIcon, CComBSTR(sz), 100, -200);
		}
		return S_OK;
	}

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
	CEditToolDataRectangle m_cData;
};

