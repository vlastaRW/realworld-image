
#pragma once

#include "EditTool.h"
#include "EditToolPixelMixer.h"

#include <agg_trans_perspective.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_gray.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_span_allocator.h>
#include <agg_span_interpolator_trans.h>
#include <agg_span_image_filter_gray.h>
#include <agg_blur.h>
#include <agg_image_accessors.h>

struct CEditToolDataShadow
{
	MIDL_INTERFACE("90BA1964-761F-4E95-B5DE-48FCA08F6B80")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataShadow const*, InternalData)() = 0;
	};

	CEditToolDataShadow() : nBlur(3), nNearDensity(60), nFarDensity(20)
	{
	}
	HRESULT FromString(BSTR a_bstr)
	{
		swscanf(a_bstr, L"%i|%i|%i", &nNearDensity, &nFarDensity, &nBlur);
		if (nNearDensity < 0) nNearDensity = 0; else if (nNearDensity > 100) nNearDensity = 100;
		if (nFarDensity < 0) nFarDensity = 0; else if (nFarDensity > 100) nFarDensity = 100;
		if (nFarDensity < 0) nBlur = 0; else if (nBlur > 30) nBlur = 30;
		return E_NOTIMPL;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[64] = L"";
		swprintf(szTmp, L"%i|%i|%i", nNearDensity, nFarDensity, nBlur);
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	int nBlur;
	int nNearDensity;
	int nFarDensity;
};

#include "EditToolShadowDlg.h"


HICON GetToolIconPSHADOW(ULONG a_nSize);

class CEditToolShadow :
	public CEditToolMouseInput<CEditToolShadow>, // no direct tablet support
	public CEditToolCustomOrMoveCursor<CEditToolShadow, GetToolIconPSHADOW>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolShadow, // T - the top level class for cross casting
		CEditToolShadow, // TResetHandler
		CEditToolShadow, // TDirtyHandler
		CEditToolShadow, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolShadow, // TAdjustCoordsHandler
		CEditToolCustomOrMoveCursor<CEditToolShadow, GetToolIconPSHADOW>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolShadow>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CEditToolShadow, // TControlPointsHandler
		CEditToolShadow // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolShadow() : m_bModifying(false), m_bDragging(false)
	{
	}

	BEGIN_COM_MAP(CEditToolShadow)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	struct SGray
	{
		struct SDummy
		{
			typedef BYTE value_type;
			typedef ULONG calc_type;
	        enum base_scale_e
			{
				base_shift = 0,
				base_mask  = 0xff
			};

			BYTE b;
		};
		typedef SDummy color_type;
		typedef void order_type;
		typedef void value_type;
        enum pix_width_e
		{
			pix_width = 1,
			pix_step = 1
		};

		SGray(BYTE* a_pBuffer, int a_nStride, int a_nSizeX, int a_nSizeY) :
			m_pBuffer(a_pBuffer), m_nStride(a_nStride),
			m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY)
		{
		}
		int stride() const { return m_nStride; }
		BYTE* pix_ptr(int x, int y) const { return m_pBuffer + y*m_nStride + x; }
		static void make_pix(agg::int8u* p, SDummy s) { *p = s.b; }
		int width() const { return m_nSizeX; }
		int height() const { return m_nSizeY; }
		BYTE* m_pBuffer;
		int m_nStride;
		int m_nSizeX;
		int m_nSizeY;
	};
	struct STargetBuffer
	{
		typedef SGray::SDummy color_type;
		typedef void row_data;
		typedef void span_data;

		STargetBuffer(ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, RECT const& a_rcClip, TRasterImagePixel* a_pBuffer) :
			m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_nStride(a_nStride), m_rcClip(a_rcClip), m_pBuffer(a_pBuffer)
		{
		}

		unsigned width() const
		{
			return m_nSizeX;
		}
		unsigned height() const
		{
			return m_nSizeY;
		}

		void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
		{
			if (y < m_rcClip.top || y >= m_rcClip.bottom)
				return;
			ATLASSERT(x >= m_rcClip.left && int(x+len) <= m_rcClip.right);
			TRasterImagePixel* pLine = m_pBuffer + (y-m_rcClip.top)*m_nStride + x-m_rcClip.left;
			while (len--)
			{
				if (pLine->bA != 255)
				{
					ULONG const nNewA = pLine->bA*255 + (255-pLine->bA)*colors->b;
					if (nNewA)
					{
						ULONG const bA2 = pLine->bA*255;
						pLine->bR = pLine->bR*bA2/nNewA;
						pLine->bG = pLine->bG*bA2/nNewA;
						pLine->bB = pLine->bB*bA2/nNewA;
					}
					pLine->bA = nNewA/255;
				}
				++colors;
				++pLine;
			}
			if (covers)
			{
				// TODO: handle exceptions correctly
				//SFullSpan sItem = {x, y, len, new color_type[len], new BYTE[len]};
				//CopyMemory(sItem.pCovers, covers, len*sizeof *sItem.pCovers);
				//CopyMemory(sItem.pC, colors, len*sizeof *sItem.pC);
				//m_cFullSpans.push_back(sItem);
			}
			else
			{
				//SColorSpan sItem = {x, y, len, new color_type[len], cover};
				//CopyMemory(sItem.pC, colors, len*sizeof *sItem.pC);
				//m_cColorSpans.push_back(sItem);
			}
		}

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		ULONG m_nStride;
		RECT const m_rcClip;
		TRasterImagePixel* m_pBuffer;
	};
    template<class Source, class Interpolator> 
    class span_image_filter_SGray_2x2 : 
		public agg::span_image_filter<Source, Interpolator>
    {
    public:
        typedef Source source_type;
        typedef typename source_type::color_type color_type;
        typedef Interpolator interpolator_type;
        typedef span_image_filter<source_type, interpolator_type> base_type;
        typedef typename color_type::value_type value_type;
        typedef typename color_type::calc_type calc_type;
        enum base_scale_e
        {
            base_shift = color_type::base_shift,
            base_mask  = color_type::base_mask
        };

        //--------------------------------------------------------------------
        span_image_filter_SGray_2x2() {}
        span_image_filter_SGray_2x2(source_type& src, 
                                    interpolator_type& inter,
									const agg::image_filter_lut& filter) :
            base_type(src, inter, &filter) 
        {}


        //--------------------------------------------------------------------
        void generate(color_type* span, int x, int y, unsigned len)
        {
            base_type::interpolator().begin(x + base_type::filter_dx_dbl(), 
                                            y + base_type::filter_dy_dbl(), len);

            calc_type fg;

            const value_type *fg_ptr;
			const agg::int16* weight_array = base_type::filter().weight_array() + 
                                        ((base_type::filter().diameter()/2 - 1) << 
										agg::image_subpixel_shift);
            do
            {
                int x_hr;
                int y_hr;

                base_type::interpolator().coordinates(&x_hr, &y_hr);

                x_hr -= base_type::filter_dx_int();
                y_hr -= base_type::filter_dy_int();

                int x_lr = x_hr >> agg::image_subpixel_shift;
                int y_lr = y_hr >> agg::image_subpixel_shift;

                unsigned weight;
				fg = agg::image_filter_scale / 2;

				x_hr &= agg::image_subpixel_mask;
				y_hr &= agg::image_subpixel_mask;

                fg_ptr = (const value_type*)base_type::source().span(x_lr, y_lr, 2);
				weight = (weight_array[x_hr + agg::image_subpixel_scale] * 
                          weight_array[y_hr + agg::image_subpixel_scale] + 
                          agg::image_filter_scale / 2) >> 
                          agg::image_filter_shift;
                fg += weight * *fg_ptr;

                fg_ptr = (const value_type*)base_type::source().next_x();
                weight = (weight_array[x_hr] * 
                          weight_array[y_hr + agg::image_subpixel_scale] + 
                          agg::image_filter_scale / 2) >> 
                          agg::image_filter_shift;
                fg += weight * *fg_ptr;

                fg_ptr = (const value_type*)base_type::source().next_y();
                weight = (weight_array[x_hr + agg::image_subpixel_scale] * 
                          weight_array[y_hr] + 
                          agg::image_filter_scale / 2) >> 
                          agg::image_filter_shift;
                fg += weight * *fg_ptr;

                fg_ptr = (const value_type*)base_type::source().next_x();
                weight = (weight_array[x_hr] * 
                          weight_array[y_hr] + 
                          agg::image_filter_scale / 2) >> 
                          agg::image_filter_shift;
                fg += weight * *fg_ptr;

                fg >>= agg::image_filter_shift;
                if(fg > base_mask) fg = base_mask;

                span->b = (value_type)fg;
                //span->a = base_mask;
                ++span;
                ++base_type::interpolator();
            } while(--len);
        }
    };

	// IRasterImageEditTool methods
public:
	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		if ((!m_bModifying && !m_bDragging) ||
		   (m_aPerspectivePolygon[0].fX == 0.0f && m_aPerspectivePolygon[0].fY == 0.0f &&
			m_aPerspectivePolygon[1].fX == 0.0f && m_aPerspectivePolygon[1].fY == 0.0f &&
			m_aPerspectivePolygon[2].fX == 0.0f && m_aPerspectivePolygon[2].fY == 0.0f &&
			m_aPerspectivePolygon[3].fX == 0.0f && m_aPerspectivePolygon[3].fY == 0.0f))
			return M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);

		HRESULT hRes = M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes))
			return hRes;
		PrepareShadow();

		double dest[] =
		{
			m_aPerspectivePolygon[0].fX, m_aPerspectivePolygon[0].fY,
			m_aPerspectivePolygon[1].fX, m_aPerspectivePolygon[1].fY,
			m_aPerspectivePolygon[2].fX, m_aPerspectivePolygon[2].fY,
			m_aPerspectivePolygon[3].fX, m_aPerspectivePolygon[3].fY,
		};
		agg::trans_perspective tr(dest, m_cData.nBlur, m_cData.nBlur, m_nSizeX+m_cData.nBlur, m_nSizeY+m_cData.nBlur);

        if (tr.is_valid())
        {
			agg::trans_perspective tr_inv(dest, 0.0, 0.0, m_nSizeX, m_nSizeY);
			tr_inv.invert();
			double src[8] =
			{
				-m_cData.nBlur, -m_cData.nBlur,
				m_nSizeX+m_cData.nBlur, -m_cData.nBlur,
				m_nSizeX+m_cData.nBlur, m_nSizeY+m_cData.nBlur,
				-m_cData.nBlur, m_nSizeY+m_cData.nBlur,
			};
			tr_inv.transform(src+0, src+1);
			tr_inv.transform(src+2, src+3);
			tr_inv.transform(src+4, src+5);
			tr_inv.transform(src+6, src+7);

			agg::rasterizer_scanline_aa<> ras;
			ras.clip_box(a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY);
			ras.move_to_d(src[0], src[1]);
			ras.line_to_d(src[2], src[3]);
			ras.line_to_d(src[4], src[5]);
			ras.line_to_d(src[6], src[7]);

			agg::span_allocator<SGray::SDummy> sa;
			agg::image_filter_bilinear filter_kernel;
			agg::image_filter_lut filter(filter_kernel, false);

			//agg::rendering_buffer buffer(reinterpret_cast<agg::int8u*>(m_pShadow.m_p), m_nSizeX+m_cData.nBlur*2, m_nSizeY+m_cData.nBlur*2, sizeof(*m_pShadow.m_p)*(m_nSizeX+m_cData.nBlur*2));
			//agg::pixfmt_gray8 pixf_img(buffer);
			SGray sGray(m_pShadow.m_p, sizeof(*m_pShadow.m_p)*(m_nSizeX+m_cData.nBlur*2), m_nSizeX+m_cData.nBlur*2, m_nSizeY+m_cData.nBlur*2);

			typedef agg::image_accessor_clip<SGray> img_accessor_type;
			SGray::SDummy s = { 0 };
			img_accessor_type ia(sGray, s);

            typedef agg::span_interpolator_trans<agg::trans_perspective> interpolator_type;
            interpolator_type interpolator(tr);
            typedef span_image_filter_SGray_2x2<img_accessor_type,
                                                interpolator_type> span_gen_type;
            span_gen_type sg(ia, interpolator, filter);

			agg::scanline_u8 sl;

			RECT rcClip = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
			STargetBuffer sBuffer(m_nSizeX, m_nSizeY, a_nStride, rcClip, a_pBuffer);
			agg::renderer_base<STargetBuffer> rb(sBuffer);

			agg::render_scanlines_aa(ras, sl, rb, sa, sg);
        }



//        if(tr.is_valid())
//        {
//			typedef agg::span_allocator<agg::rgba8> span_alloc_type;
//			span_alloc_type sa;
//			agg::image_filter_hermite filter_kernel;
//			agg::image_filter_lut filter(filter_kernel, false);
//
//            typedef agg::span_interpolator_linear<agg::trans_perspective> interpolator_type;
//            typedef agg::span_subdiv_adaptor<interpolator_type> subdiv_adaptor_type;
//			interpolator_type interpolator(tr);
//            subdiv_adaptor_type subdiv_adaptor(interpolator);
//
//			typedef agg::span_image_filter_rgba_2x2<agg::pixfmt_rgba32, interpolator_type> span_gen_type;
//            //typedef agg::span_image_filter_rgba_2x2<agg::rgba8,
//            //                                        agg::order_bgra, 
//            //                                        subdiv_adaptor_type> span_gen_type;
//
//            span_gen_type sg(   buffer, 
////                                agg::rgba_pre(0, 0, 0, 0),
////                                subdiv_adaptor,
//								interpolator,
//                                filter);
//
//			CAGGRasterImageTarget<TPixelMixer, TRasterImageCache> cTarget(a_pImage);
//			agg::renderer_base<CAGGRasterImageTarget<TPixelMixer, TRasterImageCache> > renb(cTarget);
//
//			agg::renderer_scanline_aa<agg::renderer_base<CAGGRasterImageTarget<TPixelMixer, TRasterImageCache> >, span_alloc_type, span_gen_type > ren(renb, sa, sg);
//			agg::rasterizer_scanline_aa<> ras;
//			agg::scanline_u8 sl;
//			//ras.clip_box(m_tSelFrom.nX, m_tSelFrom.nY, m_tSelTo.nX, m_tSelTo.nY);
//			ras.reset();
//			ras.move_to_d(m_tSelFrom.nX, m_tSelFrom.nY);
//			ras.line_to_d(m_tSelTo.nX, m_tSelFrom.nY);
//			ras.line_to_d(m_tSelTo.nX, m_tSelTo.nY);
//			ras.line_to_d(m_tSelFrom.nX, m_tSelTo.nY);
//
//			agg::render_scanlines(ras, sl, ren);

		//CAutoVectorPtr<BYTE> cMask(new BYTE[a_nSizeX*a_nSizeY]);
		//if (FAILED(M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nSizeX, cMask)))
		//	cMask.Free();
		RECT rc = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		//RenderTile(m_eBlendingMode, rc, a_nStride, a_pBuffer, 0, NULL);
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		try
		{
			if (a_pOptimizeImageRect)
				*a_pOptimizeImageRect = FALSE;
			if (a_pSelectionRect)
				*a_pSelectionRect = RECT_EMPTY;
			if (a_pImageRect)
				*a_pImageRect = DirtyRect();
			return m_bDragging || m_bModifying ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataShadow::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			RECT rcPrev = DirtyRect();
			m_cData = *(pData->InternalData());
			InvalidateShadow();
			if (m_bModifying || m_bDragging)
			{
				RECT rcNew = DirtyRect();
				if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
				if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
				if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
				if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
				M_Window()->RectangleChanged(&rcNew);
			}
		}
		return S_OK;
	}

	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		m_bDragging = false;
		m_bModifying = false;
		m_aPerspectivePolygon[0].fX = m_aPerspectivePolygon[0].fY =
		m_aPerspectivePolygon[1].fX = m_aPerspectivePolygon[1].fY =
		m_aPerspectivePolygon[2].fX = m_aPerspectivePolygon[2].fY =
		m_aPerspectivePolygon[3].fX = m_aPerspectivePolygon[3].fY = 0.0f;
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		InvalidateShadow();
		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_bModifying)
		{
			if (UseMoveCursor(a_pPos))
			{
				m_tStartPos = *a_pPos;
				m_bDragging = true;
				return S_OK;
			}
			ATLASSERT(0);
			return ETPAStartNew|ETPAApply;
		}
		m_tStartPos = *a_pPos;
		ZeroMemory(m_aPerspectivePolygon, sizeof m_aPerspectivePolygon);
		m_bDragging = true;
		m_bModifying = false;
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_bDragging)
		{
			m_bDragging = false;
			m_bModifying =
				m_aPerspectivePolygon[0].fX != 0.0f || m_aPerspectivePolygon[0].fY != 0.0f ||
				m_aPerspectivePolygon[1].fX != 0.0f || m_aPerspectivePolygon[1].fY != 0.0f ||
				m_aPerspectivePolygon[2].fX != 0.0f || m_aPerspectivePolygon[2].fY != 0.0f ||
				m_aPerspectivePolygon[3].fX != 0.0f || m_aPerspectivePolygon[3].fY != 0.0f;
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_bDragging)
		{
			RECT rcPrev = DirtyRect();

			if (m_bModifying)
			{
				m_aPerspectivePolygon[0].fX += a_pPos->fX-m_tStartPos.fX;
				m_aPerspectivePolygon[0].fY += a_pPos->fY-m_tStartPos.fY;
				m_aPerspectivePolygon[1].fX += a_pPos->fX-m_tStartPos.fX;
				m_aPerspectivePolygon[1].fY += a_pPos->fY-m_tStartPos.fY;
				m_aPerspectivePolygon[2].fX += a_pPos->fX-m_tStartPos.fX;
				m_aPerspectivePolygon[2].fY += a_pPos->fY-m_tStartPos.fY;
				m_aPerspectivePolygon[3].fX += a_pPos->fX-m_tStartPos.fX;
				m_aPerspectivePolygon[3].fY += a_pPos->fY-m_tStartPos.fY;
				M_Window()->ControlPointChanged(0);
				M_Window()->ControlPointChanged(1);
				M_Window()->ControlPointChanged(2);
				M_Window()->ControlPointChanged(3);
				M_Window()->ControlPointChanged(4);
				m_tStartPos = *a_pPos;
			}
			else if (a_pPos->fY != m_tStartPos.fY)
			{
				//double angleY = (a_pPos->fY-m_tStartPos.fY)*3.141528/(m_nSizeY*2.0);
				//int deltaX = a_pPos->fX-m_tStartPos.fX;
				double dest[] =
				{
					a_pPos->fX, a_pPos->fY,
					(m_nSizeX-m_tStartPos.fX)*0.75+a_pPos->fX, a_pPos->fY,
					m_nSizeX, m_tStartPos.fY,
					m_tStartPos.fX, m_tStartPos.fY
				};
				agg::trans_perspective tr(dest, m_tStartPos.fX, 0, m_nSizeX, m_tStartPos.fY);
				double dX, dY;
				dX = 0.0f; dY = 0.0f;
				tr.inverse_transform(&dX, &dY);
				m_aPerspectivePolygon[0].fX = dX; m_aPerspectivePolygon[0].fY = dY;
				dX = m_nSizeX; dY = 0.0f;
				tr.inverse_transform(&dX, &dY);
				m_aPerspectivePolygon[1].fX = dX; m_aPerspectivePolygon[1].fY = dY;
				dX = m_nSizeX; dY = m_nSizeY;
				tr.inverse_transform(&dX, &dY);
				m_aPerspectivePolygon[2].fX = dX; m_aPerspectivePolygon[2].fY = dY;
				dX = 0.0f; dY = m_nSizeY;
				tr.inverse_transform(&dX, &dY);
				m_aPerspectivePolygon[3].fX = dX; m_aPerspectivePolygon[3].fY = dY;
			}
			else
			{
				ZeroMemory(m_aPerspectivePolygon, sizeof m_aPerspectivePolygon);
			}

			M_Window()->ControlLinesChanged();
			RECT rcNew = DirtyRect();
			if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
			if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
			if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
			if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
			M_Window()->RectangleChanged(&rcNew);
		}
		return S_OK;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_bModifying ? 5 : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (!m_bModifying || a_nIndex >= 5)
			return E_RW_INDEXOUTOFRANGE;
		if (a_nIndex == 4)
		{
			a_pPos->fX = 0.5f*(m_aPerspectivePolygon[0].fX+m_aPerspectivePolygon[1].fX);
			a_pPos->fY = 0.5f*(m_aPerspectivePolygon[0].fY+m_aPerspectivePolygon[1].fY);
			*a_pClass = 1;
		}
		else
		{
			*a_pPos = m_aPerspectivePolygon[a_nIndex];
			*a_pClass = 0;
		}
		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (!m_bModifying || a_nIndex >= 5)
			return E_RW_INDEXOUTOFRANGE;
		RECT rcPrev = DirtyRect();
		if (a_nIndex == 4)
		{
			TPixelCoords const tCenter =
			{
				0.5f*(m_aPerspectivePolygon[0].fX+m_aPerspectivePolygon[1].fX),
				0.5f*(m_aPerspectivePolygon[0].fY+m_aPerspectivePolygon[1].fY),
			};
			m_aPerspectivePolygon[0].fX += a_pPos->fX-tCenter.fX;
			m_aPerspectivePolygon[0].fY += a_pPos->fY-tCenter.fY;
			m_aPerspectivePolygon[1].fX += a_pPos->fX-tCenter.fX;
			m_aPerspectivePolygon[1].fY += a_pPos->fY-tCenter.fY;
			M_Window()->ControlPointChanged(0);
			M_Window()->ControlPointChanged(1);
			M_Window()->ControlPointChanged(4);
			M_Window()->ControlLinesChanged();
		}
		else
		{
			m_aPerspectivePolygon[a_nIndex] = *a_pPos;
			M_Window()->ControlPointChanged(a_nIndex);
			if (a_nIndex < 2)
				M_Window()->ControlPointChanged(4);
			M_Window()->ControlLinesChanged();
		}
		RECT rcNew = DirtyRect();
		if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
		if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
		if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
		if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
		M_Window()->RectangleChanged(&rcNew);
		return S_OK;
	}

	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if ((m_bModifying || m_bDragging) &&
		   (m_aPerspectivePolygon[0].fX != 0.0f || m_aPerspectivePolygon[0].fY != 0.0f ||
			m_aPerspectivePolygon[1].fX != 0.0f || m_aPerspectivePolygon[1].fY != 0.0f ||
			m_aPerspectivePolygon[2].fX != 0.0f || m_aPerspectivePolygon[2].fY != 0.0f ||
			m_aPerspectivePolygon[3].fX != 0.0f || m_aPerspectivePolygon[3].fY != 0.0f))
		{
			RECT rc = DirtyRect();
			a_pLines->MoveTo(m_aPerspectivePolygon[0].fX, m_aPerspectivePolygon[0].fY);
			a_pLines->LineTo(m_aPerspectivePolygon[1].fX, m_aPerspectivePolygon[1].fY);
			a_pLines->LineTo(m_aPerspectivePolygon[2].fX, m_aPerspectivePolygon[2].fY);
			a_pLines->LineTo(m_aPerspectivePolygon[3].fX, m_aPerspectivePolygon[3].fY);
			a_pLines->Close();
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		if (!m_bModifying)
			return false;
		if (m_bDragging)
			return true;
		double angles[5] =
		{
			atan2(m_aPerspectivePolygon[0].fX-a_pPos->fX, m_aPerspectivePolygon[0].fY-a_pPos->fY),
			atan2(m_aPerspectivePolygon[1].fX-a_pPos->fX, m_aPerspectivePolygon[1].fY-a_pPos->fY),
			atan2(m_aPerspectivePolygon[2].fX-a_pPos->fX, m_aPerspectivePolygon[2].fY-a_pPos->fY),
			atan2(m_aPerspectivePolygon[3].fX-a_pPos->fX, m_aPerspectivePolygon[3].fY-a_pPos->fY),
			atan2(m_aPerspectivePolygon[0].fX-a_pPos->fX, m_aPerspectivePolygon[0].fY-a_pPos->fY),
		};
		double fi = 0.0;
		for (int i = 0; i < 4; ++i)
		{
			double delta = angles[i+1]-angles[i];
			if (delta < 3.1415926535897932 && delta > -3.1415926535897932)
				fi += delta;
			else
				fi -= delta;
		}
		return fi > 3.1415926535897932 || fi < -3.1415926535897932;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return UseMoveCursor(a_pPos) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
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
			float fBlur = 0.0f;
			float fNear = 0.0f;
			float fFar = 0.0f;
			TPixelCoords aCoords[4];
			if (11 == swscanf(a_bstrParams, L"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &fBlur, &fNear, &fFar, &aCoords[0].fX, &aCoords[0].fY, &aCoords[1].fX, &aCoords[1].fY, &aCoords[2].fX, &aCoords[2].fY, &aCoords[3].fX, &aCoords[3].fY))
			{
				bool bChange = m_cData.nBlur != int(fBlur+0.5f) || m_cData.nFarDensity != int(fFar+0.5f) || m_cData.nNearDensity != int(fNear+0.5f);
				m_cData.nBlur = fBlur+0.5f;
				m_cData.nFarDensity = fFar+0.5f;
				m_cData.nNearDensity = fNear+0.5f;
				m_aPerspectivePolygon[0] = aCoords[0];
				m_aPerspectivePolygon[1] = aCoords[1];
				m_aPerspectivePolygon[2] = aCoords[2];
				m_aPerspectivePolygon[3] = aCoords[3];
				m_bDragging = false;
				m_bModifying = true;
				InvalidateShadow();
				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
				if (bChange)
				{
					CComObject<CSharedStateToolData>* pNew = NULL;
					CComObject<CSharedStateToolData>::CreateInstance(&pNew);
					CComPtr<ISharedState> pTmp = pNew;
					pNew->Init(m_cData);
					M_Window()->SetState(pTmp);
				}
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
			if (!m_bModifying && !m_bDragging)
				return S_FALSE;
			CComBSTR bstr;
			OLECHAR sz[256];
			swprintf(sz, L"%i, %i, %i, %g, %g, %g, %g, %g, %g, %g, %g", m_cData.nBlur, m_cData.nNearDensity, m_cData.nFarDensity,
				m_aPerspectivePolygon[0].fX, m_aPerspectivePolygon[0].fY,
				m_aPerspectivePolygon[1].fX, m_aPerspectivePolygon[1].fY,
				m_aPerspectivePolygon[2].fX, m_aPerspectivePolygon[2].fY,
				m_aPerspectivePolygon[3].fX, m_aPerspectivePolygon[3].fY);
			bstr = sz;
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	void PrepareShadow()
	{
		ObjectLock cLock(this);
		if (m_pShadow.m_p)
			return;
		m_pShadow.Free();
		m_pShadow.Allocate((m_nSizeX+m_cData.nBlur*2)*(m_nSizeY+m_cData.nBlur*2));
		ZeroMemory(m_pShadow.m_p, sizeof(*m_pShadow.m_p)*(m_nSizeX+m_cData.nBlur*2)*(m_nSizeY+m_cData.nBlur*2));
		CAutoVectorPtr<TRasterImagePixel> pImage;
		pImage.Allocate(m_nSizeX*m_nSizeY);
		M_Window()->GetImageTile(0, 0, m_nSizeX, m_nSizeY, 0.0f, m_nSizeX, EITIContent, pImage.m_p);
		BOOL bEntire = TRUE;
		RECT rcBounds = {0, 0, m_nSizeX, m_nSizeY};
		M_Window()->GetSelectionInfo(&rcBounds, &bEntire);
		if ((!bEntire || rcBounds.left > 0 || rcBounds.top > 0 || rcBounds.right < LONG(m_nSizeX) || rcBounds.bottom < LONG(m_nSizeY)) &&
			SUCCEEDED(M_Window()->GetSelectionTile(0, 0, m_nSizeX, m_nSizeY, m_nSizeX+m_cData.nBlur*2, m_pShadow.m_p + (m_nSizeX+m_cData.nBlur*2)*m_cData.nBlur + m_cData.nBlur)))
		{
			// with selection
			for (LONG y = 0; y < LONG(m_nSizeY); ++y)
			{
				ULONG const nDensity = (m_cData.nFarDensity + (m_cData.nNearDensity-m_cData.nFarDensity)*y/LONG(m_nSizeY))*256/100;
				BYTE* pD = m_pShadow.m_p + (y+m_cData.nBlur)*(m_nSizeX+m_cData.nBlur*2) + m_cData.nBlur;
				TRasterImagePixel* pS = pImage.m_p + y*m_nSizeX;
				for (ULONG x = 0; x < m_nSizeX; ++x)
				{
					*pD = (ULONG(*pD)*pS->bA*nDensity)>>16;
					++pD; ++pS;
				}
			}
		}
		else
		{
			// no selection
			for (LONG y = 0; y < LONG(m_nSizeY); ++y)
			{
				ULONG const nDensity = (m_cData.nFarDensity + (m_cData.nNearDensity-m_cData.nFarDensity)*y/LONG(m_nSizeY))*256/100;
				BYTE* pD = m_pShadow.m_p + (y+m_cData.nBlur)*(m_nSizeX+m_cData.nBlur*2) + m_cData.nBlur;
				TRasterImagePixel* pS = pImage.m_p + y*m_nSizeX;
				for (ULONG x = 0; x < m_nSizeX; ++x)
				{
					*pD = (pS->bA*nDensity)>>8;
					++pD; ++pS;
				}
			}
		}
		SGray sBuffer(m_pShadow, sizeof(*m_pShadow.m_p)*(m_nSizeX+m_cData.nBlur*2), m_nSizeX+m_cData.nBlur*2, m_nSizeY+m_cData.nBlur*2);
		agg::stack_blur_gray8(sBuffer, m_cData.nBlur, m_cData.nBlur);
	}
	void InvalidateShadow()
	{
		ObjectLock cLock(this);
		m_pShadow.Free();
	}
	RECT DirtyRect() const
	{
		if (m_aPerspectivePolygon[0].fX == 0.0f && m_aPerspectivePolygon[0].fY == 0.0f &&
			m_aPerspectivePolygon[1].fX == 0.0f && m_aPerspectivePolygon[1].fY == 0.0f &&
			m_aPerspectivePolygon[2].fX == 0.0f && m_aPerspectivePolygon[2].fY == 0.0f &&
			m_aPerspectivePolygon[3].fX == 0.0f && m_aPerspectivePolygon[3].fY == 0.0f)
			return RECT_EMPTY;

		double dest[] =
		{
			m_aPerspectivePolygon[0].fX, m_aPerspectivePolygon[0].fY,
			m_aPerspectivePolygon[1].fX, m_aPerspectivePolygon[1].fY,
			m_aPerspectivePolygon[2].fX, m_aPerspectivePolygon[2].fY,
			m_aPerspectivePolygon[3].fX, m_aPerspectivePolygon[3].fY,
		};
		agg::trans_perspective tr_inv(dest, 0.0, 0.0, m_nSizeX, m_nSizeY);
		if (!tr_inv.is_valid())
			return RECT_EMPTY;
		tr_inv.invert();
		double src[8] =
		{
			-m_cData.nBlur, -m_cData.nBlur,
			m_nSizeX+m_cData.nBlur, -m_cData.nBlur,
			m_nSizeX+m_cData.nBlur, m_nSizeY+m_cData.nBlur,
			-m_cData.nBlur, m_nSizeY+m_cData.nBlur,
		};
		tr_inv.transform(src+0, src+1);
		tr_inv.transform(src+2, src+3);
		tr_inv.transform(src+4, src+5);
		tr_inv.transform(src+6, src+7);

		float fMinX = src[0];
		float fMaxX = src[1];
		float fMinY = src[0];
		float fMaxY = src[1];
		for (int i = 1; i < 4; ++i)
		{
			if (fMinX > src[i+i]) fMinX = src[i+i];
			if (fMaxX < src[i+i]) fMaxX = src[i+i];
			if (fMinY > src[i+i+1]) fMinY = src[i+i+1];
			if (fMaxY < src[i+i+1]) fMaxY = src[i+i+1];
		}
		RECT const rc =
		{
			fMinX-1,
			fMinY-1,
			fMaxX+2,
			fMaxY+2
		};
		return rc;
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	TPixelCoords m_tStartPos;
	TPixelCoords m_aPerspectivePolygon[4];
	bool m_bDragging;
	bool m_bModifying;
	CEditToolDataShadow m_cData;
	CAutoVectorPtr<BYTE> m_pShadow;
};
