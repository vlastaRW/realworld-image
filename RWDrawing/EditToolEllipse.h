
#pragma once


#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolRectangularShape.h"
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_ellipse.h>
#include <agg_trans_affine.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_span_gradient.h>
#include <agg_conv_stroke.h>
#include <agg_conv_transform.h>
#include <XPGUI.h>


struct CEditToolDataEllipse
{
	MIDL_INTERFACE("5649946D-AA96-4E74-B082-F2AC20519EDF")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataEllipse const*, InternalData)() = 0;
	};
	enum EMode
	{
		EMCircle = 0,
		EMEllipse,
		EMEllipseSector,
	};

	CEditToolDataEllipse() : eMode(EMEllipse)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (wcsstr(a_bstr, L"CIRCLE"))
			eMode = EMCircle;
		else if (wcsstr(a_bstr, L"ELLIPSE"))
			eMode = EMEllipse;
		else if (wcsstr(a_bstr, L"SECTOR"))
			eMode = EMEllipseSector;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = SysAllocString(eMode == EMCircle ? L"CIRCLE" : (eMode == EMEllipse ? L"ELLIPSE" : L"SECTOR"));
		return S_OK;
	}

	EMode eMode;
};

#include "EditToolEllipseDlg.h"


HICON GetToolIconELLIPSE(ULONG a_nSize);

class CEditToolEllipse :
	public CEditToolScanlineBuffer<CEditToolEllipse>, // scanline image cache
	public CEditToolRectangularShape<CEditToolEllipse>, // creating and modifying the rectangle
	public CEditToolMouseInput<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse> >, // no direct tablet support
	public CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, // brush override
	public CEditToolOutline<CEditToolEllipse>, // outine handling
	public CEditToolCustomOrMoveCursor<CEditToolEllipse, GetToolIconELLIPSE>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolEllipse, CEditToolMouseInput<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse> >, CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, CEditToolCustomOrMoveCursor<CEditToolEllipse, GetToolIconELLIPSE> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolEllipse, // T - the top level class for cross casting
		CEditToolEllipse, // TResetHandler
		CEditToolScanlineBuffer<CEditToolEllipse>, // TDirtyHandler
		CEditToolScanlineBuffer<CEditToolEllipse>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CEditToolOutline<CEditToolEllipse>, // TOutlineHandler
		CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, // TBrushHandler
		CEditToolEllipse, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolEllipse, CEditToolMouseInput<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse> >, CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, CEditToolCustomOrMoveCursor<CEditToolEllipse, GetToolIconELLIPSE> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolEllipse, CEditToolMouseInput<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse> >, CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, CEditToolCustomOrMoveCursor<CEditToolEllipse, GetToolIconELLIPSE> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolEllipse, CEditToolMouseInput<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse> >, CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, CEditToolCustomOrMoveCursor<CEditToolEllipse, GetToolIconELLIPSE> >, // TProcessInputHandler
		CEditToolRectangularShape<CEditToolEllipse>, // TPreTranslateMessageHandler
		CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse>, // TControlPointsHandler
		CEditToolWithBrush<CEditToolEllipse, CEditToolRectangularShape<CEditToolEllipse>, CEditToolEllipse> // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IDesignerViewStatusBar,
	public IRasterImageEditToolPolygon
{
public:
	CEditToolEllipse() : m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		CEditToolRectangularShape<CEditToolEllipse>(CEditToolRectangularShape::ERMRotatedRectangle)
	{
	}

	BEGIN_COM_MAP(CEditToolEllipse)
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
		return a_nLayer ? true : M_Brush() == NULL || S_OK == M_Brush()->IsSolid(NULL);
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
		agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
		agg::trans_affine mtx(agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));
		agg::trans_affine mtx_i(agg::trans_affine_scaling(-1.0, 1.0)*agg::trans_affine_rotation(fAngle)*agg::trans_affine_translation(tCenter.fX, tCenter.fY));

		float const fWidthIn = M_OutlineIn();
		float const fWidthOut = M_OutlineOut();
		float const fWidth = fWidthOut-fWidthIn;
		if (fWidth <= 0.0f && M_Brush())
		{
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::ellipse e(0.0, 0.0, fSizeX+fWidthOut, fSizeY+fWidthOut, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
			agg::conv_transform<agg::ellipse> tr(e, mtx);
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
		else if (fWidth > 0.0f && M_Brush() == NULL && fSizeX+fWidthOut > fWidth && fSizeY+fWidthOut > fWidth)
		{
			TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::ellipse e(0.0, 0.0, fSizeX+fWidthOut-fWidth*0.5f, fSizeY+fWidthOut-fWidth*0.5f, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
			agg::conv_transform<agg::ellipse> tr(e, mtx);
			agg::conv_stroke<agg::conv_transform<agg::ellipse>> pg(tr);
			pg.width(fWidth);
			ras.add_path(pg);

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
		else if (fWidth > 0.0f && M_Brush() && fSizeX+fWidthOut > fWidth && fSizeY+fWidthOut > fWidth)
		{
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::scanline_u8 sl;
				agg::ellipse e(0.0, 0.0, fSizeX+fWidthOut-fWidth*0.5f, fSizeY+fWidthOut-fWidth*0.5f, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
				agg::conv_transform<agg::ellipse> tr(e, mtx);
				agg::conv_stroke<agg::conv_transform<agg::ellipse> > str_ell(tr);
				str_ell.width(fWidth);
				agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;
				rasc.styles(1, -1);
				rasc.add_path(str_ell);
				rasc.styles(0, -1);
				rasc.add_path(tr);

				agg::span_allocator<TPixelChannel> alloc;
				agg::render_scanlines_compound_layered(rasc, sl, renb/*_pre*/, alloc, *this, M_LayerBlender());
			}
			else
			{
				TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
				agg::scanline_p8 sl;
				agg::rasterizer_scanline_aa<> ras;
				agg::ellipse e(0.0, 0.0, fSizeX+fWidthOut-fWidth*0.5f, fSizeY+fWidthOut-fWidth*0.5f, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
				agg::conv_transform<agg::ellipse> tr(e, mtx);
				agg::conv_stroke<agg::conv_transform<agg::ellipse>> pg(tr);
				pg.width(fWidth);
				ras.add_path(pg);
				agg::renderer_scanline_bin_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
				ren.color(CPixelChannel(tColor.bR, tColor.bG, tColor.bB, tColor.bA));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);

				ras.reset();
				agg::ellipse e2(0.0, 0.0, fSizeX-fWidth, fSizeY-fWidth, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
				agg::conv_transform<agg::ellipse> tr2(e2, mtx);
				ras.add_path(tr2);
				agg::span_allocator<TPixelChannel> span_alloc;
				ras.gamma(agg::gamma_threshold(0.502));
				agg::render_scanlines_bin(ras, sl, renb, span_alloc, *this);
			}
		}
		else
		{
			TRasterImagePixel tColor = TColorToTRasterImagePixel(M_OutlineColor(), M_Gamma());
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::ellipse e(0.0, 0.0, fSizeX+fWidthOut, fSizeY+fWidthOut, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
			agg::conv_transform<agg::ellipse> tr(e, mtx);
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

	void InitializeShape(TPixelCoords const* a_pFrom, TPixelCoords const* a_pTo, TPixelCoords* a_pCenter, float* a_pSizeX, float* a_pSizeY, float* a_pAngle, TPixelCoords* a_aPixels)
	{
		if (m_cData.eMode == CEditToolDataEllipse::EMCircle)
		{
			float fSize = sqrtf((a_pFrom->fX-a_pTo->fX)*(a_pFrom->fX-a_pTo->fX) + (a_pFrom->fY-a_pTo->fY)*(a_pFrom->fY-a_pTo->fY));
			a_pCenter->fX = (a_pFrom->fX+a_pTo->fX)*0.5f;
			a_pCenter->fY = (a_pFrom->fY+a_pTo->fY)*0.5f;
			if (m_eCoordinatesMode == ECMIntegral)
			{
				fSize = floorf(fSize+0.5f);
				a_pCenter->fX = floorf(a_pCenter->fX+0.5f);
				a_pCenter->fY = floorf(a_pCenter->fY+0.5f);
			}
			*a_pSizeX = fSize;
			*a_pSizeY = fSize;
			*a_pAngle = 0.0f;
			a_aPixels[0].fX = a_pCenter->fX-0.5f*fSize;
			a_aPixels[0].fY = a_pCenter->fY-0.5f*fSize;
			a_aPixels[1].fX = a_pCenter->fX+0.5f*fSize;
			a_aPixels[1].fY = a_pCenter->fY-0.5f*fSize;
			a_aPixels[2].fX = a_pCenter->fX+0.5f*fSize;
			a_aPixels[2].fY = a_pCenter->fY+0.5f*fSize;
			a_aPixels[3].fX = a_pCenter->fX-0.5f*fSize;
			a_aPixels[3].fY = a_pCenter->fY+0.5f*fSize;
		}
		else
		{
			CEditToolRectangularShape<CEditToolEllipse>::InitializeShape(a_pFrom, a_pTo, a_pCenter, a_pSizeX, a_pSizeY, a_pAngle, a_aPixels);
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
		CComQIPtr<CEditToolDataEllipse::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			if (SetRectangleMode(m_cData.eMode == CEditToolDataEllipse::EMCircle ? CEditToolRectangularShape::ERMRotatedSquare : CEditToolRectangularShape::ERMRotatedRectangle))
			{
				RECT const rc = UpdateCache();
				M_Window()->RectangleChanged(&rc);
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

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return RectangleMoving() || (!M_Dragging() && HitTest(a_pPos->fX, a_pPos->fY));
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
		EnableBrush(RectangleDefined() && M_State() >= ESFloating);
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

		agg::ellipse e(0.0, 0.0, fSizeX, fSizeY, max(8, static_cast<int>(max(fSizeX*2+1, fSizeY*2+1)*2)&~3));
		agg::conv_transform<agg::ellipse> tr(e, mtx);
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
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fSizeX = 0.0f;
			float fSizeY = 0.0f;
			float fAngle = 0.0f;
			float fSection1 = 0.0f;
			float fSection2 = 0.0f;
			CEditToolDataEllipse::EMode eMode = CEditToolDataEllipse::EMEllipse;
			int nParams = swscanf(a_bstrParams, L"%f,%f,%f,%f,%f,%f,%f", &fCenterX, &fCenterY, &fSizeX, &fSizeY, &fAngle, &fSection1, &fSection2);
			switch (nParams)
			{
			case 3:
				fSizeY = fSizeX;
				eMode = CEditToolDataEllipse::EMCircle;
			case 4:
			case 5:
			//case 7:
				if (fAngle != 0.0f && fSizeY == fSizeX)
					eMode = CEditToolDataEllipse::EMCircle;
				if (m_cData.eMode == eMode)
				{
					m_cData.eMode == eMode;
					SetRectangleMode(m_cData.eMode != CEditToolDataEllipse::EMCircle ? CEditToolRectangularShape::ERMRotatedRectangle : CEditToolRectangularShape::ERMRotatedSquare);
					SetRectangle(fCenterX, fCenterY, fSizeX, fSizeY, fAngle);
					CComObject<CSharedStateToolData>* pNew = NULL;
					CComObject<CSharedStateToolData>::CreateInstance(&pNew);
					CComPtr<ISharedState> pTmp = pNew;
					pNew->Init(m_cData);
					M_Window()->SetState(pTmp);
				}
				else
				{
					SetRectangle(fCenterX, fCenterY, fSizeX, fSizeY, fAngle);
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
				swprintf(sz, L"%g, %g, %g", tCenter.fX, tCenter.fY, fSizeX);
				bstr = sz;
				if (m_cData.eMode == CEditToolDataEllipse::EMEllipse || fSizeX != fSizeY || fAngle != 0.0f)
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

		float const fCos = cosf(fAngle);
		float const fSin = sinf(fAngle);
		float const fXCos = fSizeX*fCos;
		float const fXSin = fSizeX*fSin;
		float const fYCos = fSizeY*fCos;
		float const fYSin = fSizeY*fSin;

		TRWPathPoint aPoints[4];
		aPoints[0].dwFlags = 0;
		aPoints[0].tPos.fX = tCenter.fX+fXCos;
		aPoints[0].tPos.fY = tCenter.fY+fXSin;
		aPoints[0].tTanNext.fX = 0.5522847498f*fYSin;
		aPoints[0].tTanNext.fY = -0.5522847498f*fYCos;
		aPoints[0].tTanPrev.fX = -0.5522847498f*fYSin;
		aPoints[0].tTanPrev.fY = 0.5522847498f*fYCos;
		aPoints[1].dwFlags = 0;
		aPoints[1].tPos.fX = tCenter.fX+fYSin;
		aPoints[1].tPos.fY = tCenter.fY-fYCos;
		aPoints[1].tTanNext.fX = -0.5522847498f*fXCos;
		aPoints[1].tTanNext.fY = -0.5522847498f*fXSin;
		aPoints[1].tTanPrev.fX = 0.5522847498f*fXCos;
		aPoints[1].tTanPrev.fY = 0.5522847498f*fXSin;
		aPoints[2].dwFlags = 0;
		aPoints[2].tPos.fX = tCenter.fX-fXCos;
		aPoints[2].tPos.fY = tCenter.fY-fXSin;
		aPoints[2].tTanNext.fX = -0.5522847498f*fYSin;
		aPoints[2].tTanNext.fY = 0.5522847498f*fYCos;
		aPoints[2].tTanPrev.fX = 0.5522847498f*fYSin;
		aPoints[2].tTanPrev.fY = -0.5522847498f*fYCos;
		aPoints[3].dwFlags = 0;
		aPoints[3].tPos.fX = tCenter.fX-fYSin;
		aPoints[3].tPos.fY = tCenter.fY+fYCos;
		aPoints[3].tTanNext.fX = 0.5522847498f*fXCos;
		aPoints[3].tTanNext.fY = 0.5522847498f*fXSin;
		aPoints[3].tTanPrev.fX = -0.5522847498f*fXCos;
		aPoints[3].tTanPrev.fY = -0.5522847498f*fXSin;
		TRWPath tPath;
		tPath.nVertices = 4;
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
	CEditToolDataEllipse m_cData;
};

