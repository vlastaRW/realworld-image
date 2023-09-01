
#include "stdafx.h"

#include "RWOperationImageRaster.h"
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>

#include <IconRenderer.h>
//#include <RenderIcon.h>

#include <RWDrawing.h>

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


const OLECHAR CFGID_PT1X[] = L"Pt1X";
const OLECHAR CFGID_PT1Y[] = L"Pt1Y";
const OLECHAR CFGID_PT2X[] = L"Pt2X";
const OLECHAR CFGID_PT2Y[] = L"Pt2Y";
const OLECHAR CFGID_PT3X[] = L"Pt3X";
const OLECHAR CFGID_PT3Y[] = L"Pt3Y";
const OLECHAR CFGID_PT4X[] = L"Pt4X";
const OLECHAR CFGID_PT4Y[] = L"Pt4Y";
const OLECHAR CFGID_BLUR1[] = L"Blur1";
const OLECHAR CFGID_BLUR2[] = L"Blur2";
const OLECHAR CFGID_DENSITY1[] = L"Density1";
const OLECHAR CFGID_DENSITY2[] = L"Density2";

#include "ConfigGUIProjectedShadow.h"

//{ 0x3c18a690, 0x5565, 0x41d6, { 0x90, 0xaf, 0x4d, 0xa7, 0x5d, 0xed, 0xd2, 0xf3 } };
class DECLSPEC_UUID("3C18A690-5565-41D6-90AF-4DA75DEDD2F3")
RasterImageProjectedShadow;

class ATL_NO_VTABLE CRasterImageProjectedShadow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageProjectedShadow, &__uuidof(RasterImageProjectedShadow)>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl,
	public ICanvasInteractingOperation
{
public:
	CRasterImageProjectedShadow()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageProjectedShadow)

BEGIN_CATEGORY_MAP(CRasterImageProjectedShadow)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	IMPLEMENTED_CATEGORY(CATID_TagImageUnderlay)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageProjectedShadow)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(ICanvasInteractingOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL) return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Projected Shadow[0405]Rastrový obrázek - vržený stín");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_BLUR1), NULL, NULL, CConfigValue(15.0f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_BLUR2), NULL, NULL, CConfigValue(40.0f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_DENSITY1), NULL, NULL, CConfigValue(60.0f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_DENSITY2), NULL, NULL, CConfigValue(20.0f), NULL, 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT1X), NULL, NULL, CConfigValue(0.2f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT1Y), NULL, NULL, CConfigValue(0.5f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT2X), NULL, NULL, CConfigValue(0.8f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT2Y), NULL, NULL, CConfigValue(0.5f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT3X), NULL, NULL, CConfigValue(1.1f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT3Y), NULL, NULL, CConfigValue(0.9f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT4X), NULL, NULL, CConfigValue(-0.1f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT4Y), NULL, NULL, CConfigValue(0.9f), NULL, 0, NULL);

			CConfigCustomGUI<&__uuidof(RasterImageProjectedShadow), CConfigGUIProjectedShadow>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (a_pDocument == NULL)
				return S_FALSE;

			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
			if (pRI == NULL)
				return S_FALSE;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	static int GetInterpolatedBlur(int nBlur1, int nBlur2, int y, int nRange)
	{
		if (y <= 0) return nBlur1;
		if (y >= nRange) return nBlur2;
		return (nBlur1*y + nBlur2*(nRange-y) + (nRange>>1))/nRange;
	}
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
			if (pRI == NULL)
				return E_FAIL;

			TImageSize tSize = {1, 1};
			pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_BLUR1), &cVal);
			LONG nBlur1 = cVal.operator float()*sqrtf(tSize.nX*tSize.nY)/300.0f+0.5f;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_BLUR2), &cVal);
			LONG nBlur2 = cVal.operator float()*sqrtf(tSize.nX*tSize.nY)/300.0f+0.5f;
			if (nBlur1 > LONG(tSize.nX)) nBlur1 = tSize.nX;
			if (nBlur1 > LONG(tSize.nY)) nBlur1 = tSize.nY;
			if (nBlur2 > LONG(tSize.nX)) nBlur2 = tSize.nX;
			if (nBlur2 > LONG(tSize.nY)) nBlur2 = tSize.nY;
			LONG nBlur = max(nBlur1, nBlur2);
			a_pConfig->ItemValueGet(CComBSTR(CFGID_DENSITY1), &cVal);
			LONG nNearDensity = cVal.operator float()+0.5f;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_DENSITY2), &cVal);
			LONG nFarDensity = cVal.operator float()+0.5f;
			CAutoVectorPtr<BYTE> m_pShadow;
			m_pShadow.Allocate((tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
			ZeroMemory(m_pShadow.m_p, sizeof(*m_pShadow.m_p)*(tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
			CAutoVectorPtr<TRasterImagePixel> pImage;
			pImage.Allocate(tSize.nX*tSize.nY);
			pRI->TileGet(EICIRGBA, CImagePoint(0, 0), &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pImage.m_p), NULL, EIRIAccurate);
			for (LONG y = 0; y < LONG(tSize.nY); ++y)
			{
				ULONG const nDensity = LONG((nFarDensity + float(nNearDensity-nFarDensity)*y/LONG(tSize.nY))*256/100+0.5f);
				BYTE* pD = m_pShadow.m_p + (y+nBlur)*(tSize.nX+nBlur*2) + nBlur;
				TRasterImagePixel* pS = pImage.m_p + y*tSize.nX;
				for (ULONG x = 0; x < tSize.nX; ++x)
				{
					*pD = (pS->bA*nDensity)>>8;
					++pD; ++pS;
				}
			}
			SGray sBuffer(m_pShadow, sizeof(*m_pShadow.m_p)*(tSize.nX+nBlur*2), tSize.nX+nBlur*2, tSize.nY+nBlur*2);
			if (nBlur1 != nBlur2 && (sBuffer.m_nSizeY-2*nBlur) > 1)
			{
				BYTE* pBuffer = sBuffer.m_pBuffer;
				int nSizeY = sBuffer.m_nSizeY;
				int y = 0;
				while (y < nSizeY)
				{
					sBuffer.m_pBuffer = pBuffer+sBuffer.m_nStride*y;
					LONG nBl = GetInterpolatedBlur(nBlur1, nBlur2, y-nBlur, nSizeY-2*nBlur-1);
					int y2 = y+1;
					while (y2 < nSizeY && nBl == GetInterpolatedBlur(nBlur1, nBlur2, y2-nBlur, nSizeY-2*nBlur-1))
						++y2;
					sBuffer.m_nSizeY = y2-y;
					agg::stack_blur_gray8(sBuffer, nBl, 0);
					y = y2;
				}
				sBuffer.m_nSizeY = nSizeY;
				CAutoVectorPtr<BYTE> pShadow1;
				pShadow1.Allocate((tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
				CopyMemory(pShadow1.m_p, pBuffer, (tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
				CAutoVectorPtr<BYTE> pShadow2;
				pShadow2.Allocate((tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
				CopyMemory(pShadow2.m_p, pBuffer, (tSize.nX+nBlur*2)*(tSize.nY+nBlur*2));
				sBuffer.m_pBuffer = pShadow1;
				agg::stack_blur_gray8(sBuffer, 0, LONG((nBlur1+nBlur2)*0.5f+0.5f));
				sBuffer.m_pBuffer = pShadow2;
				agg::stack_blur_gray8(sBuffer, 0, nBlur1);
				sBuffer.m_pBuffer = pBuffer;
				agg::stack_blur_gray8(sBuffer, 0, nBlur2);
				y = nBlur;
				int ym = nSizeY>>1;
				while (y < ym)
				{
					BYTE* pD = pBuffer+sBuffer.m_nStride*y;
					BYTE const* pS = pShadow1.m_p+sBuffer.m_nStride*y;
					ULONG n1 = (256*(y-nBlur)+128)/(ym-nBlur);
					ULONG n2 = 256-n1;
					for (BYTE* const pEnd = pD+sBuffer.m_nSizeX; pD != pEnd; ++pD, ++pS)
						*pD = (*pD*n2 + *pS*n1)>>8;
					++y;
				}
				while (y < nSizeY-nBlur)
				{
					BYTE* pD = pBuffer+sBuffer.m_nStride*y;
					BYTE const* pS1 = pShadow1.m_p+sBuffer.m_nStride*y;
					BYTE const* pS2 = pShadow2.m_p+sBuffer.m_nStride*y;
					ULONG n1 = (256*(y-ym)+128)/(nSizeY-nBlur-ym);
					ULONG n2 = 256-n1;
					for (BYTE* const pEnd = pD+sBuffer.m_nSizeX; pD != pEnd; ++pD, ++pS1, ++pS2)
						*pD = (*pS1*n2 + *pS2*n1)>>8;
					++y;
				}
				CopyMemory(pBuffer+sBuffer.m_nStride*y, pShadow2.m_p+sBuffer.m_nStride*y, sBuffer.m_nStride*(nSizeY-y));
			}
			else
			{
				agg::stack_blur_gray8(sBuffer, nBlur, nBlur);
			}

			double dest[8];
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1X), &cVal); dest[0] = cVal.operator float()*tSize.nX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1Y), &cVal); dest[1] = cVal.operator float()*tSize.nY;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2X), &cVal); dest[2] = cVal.operator float()*tSize.nX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2Y), &cVal); dest[3] = cVal.operator float()*tSize.nY;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3X), &cVal); dest[4] = cVal.operator float()*tSize.nX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3Y), &cVal); dest[5] = cVal.operator float()*tSize.nY;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4X), &cVal); dest[6] = cVal.operator float()*tSize.nX;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4Y), &cVal); dest[7] = cVal.operator float()*tSize.nY;

			agg::trans_perspective tr(dest, nBlur, nBlur, tSize.nX+nBlur, tSize.nY+nBlur);

			if (tr.is_valid())
			{
				agg::trans_perspective tr_inv(dest, 0.0, 0.0, tSize.nX, tSize.nY);
				tr_inv.invert();
				double src[8] =
				{
					-nBlur, -nBlur,
					tSize.nX+nBlur, -nBlur,
					tSize.nX+nBlur, tSize.nY+nBlur,
					-nBlur, tSize.nY+nBlur,
				};
				tr_inv.transform(src+0, src+1);
				tr_inv.transform(src+2, src+3);
				tr_inv.transform(src+4, src+5);
				tr_inv.transform(src+6, src+7);

				agg::rasterizer_scanline_aa<> ras;
				ras.clip_box(0, 0, tSize.nX, tSize.nY);
				ras.move_to_d(src[0], src[1]);
				ras.line_to_d(src[2], src[3]);
				ras.line_to_d(src[4], src[5]);
				ras.line_to_d(src[6], src[7]);

				agg::span_allocator<SGray::SDummy> sa;
				agg::image_filter_bilinear filter_kernel;
				agg::image_filter_lut filter(filter_kernel, false);

				//agg::rendering_buffer buffer(reinterpret_cast<agg::int8u*>(m_pShadow.m_p), m_nSizeX+m_cData.nBlur*2, m_nSizeY+m_cData.nBlur*2, sizeof(*m_pShadow.m_p)*(m_nSizeX+m_cData.nBlur*2));
				//agg::pixfmt_gray8 pixf_img(buffer);
				SGray sGray(m_pShadow.m_p, sizeof(*m_pShadow.m_p)*(tSize.nX+nBlur*2), tSize.nX+nBlur*2, tSize.nY+nBlur*2);

				typedef agg::image_accessor_clip<SGray> img_accessor_type;
				SGray::SDummy s = { 0 };
				img_accessor_type ia(sGray, s);

				typedef agg::span_interpolator_trans<agg::trans_perspective> interpolator_type;
				interpolator_type interpolator(tr);
				typedef span_image_filter_SGray_2x2<img_accessor_type,
													interpolator_type> span_gen_type;
				span_gen_type sg(ia, interpolator, filter);

				agg::scanline_u8 sl;

				RECT rcClip = {0, 0, tSize.nX, tSize.nY};
				STargetBuffer sBuffer(tSize.nX, tSize.nY, tSize.nX, rcClip, pImage.m_p);
				agg::renderer_base<STargetBuffer> rb(sBuffer);

				agg::render_scanlines_aa(ras, sl, rb, sa, sg);

				pRI->TileSet(EICIRGBA, CImagePoint(0, 0), &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pImage.m_p), FALSE);
			}
			return S_OK;

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}


	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		if (a_pCanvas == NULL)
			return E_NOTIMPL;
		// TODO: implement correctly
		if (a_pRect)
		{
			a_pRect->tTL.nX = a_pRect->tTL.nY = 0;
			a_pRect->tBR.nX = a_pCanvas->nX;
			a_pRect->tBR.nY = a_pCanvas->nY;
			//a_pRect->tTL.nX = a_pRect->tTL.nY = LONG_MIN;
			//a_pRect->tBR.nX = a_pRect->tBR.nY = LONG_MAX;
		}
		return S_OK;
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{ return AdjustDirtyRect(a_pConfig, a_pCanvas, a_pRect); }

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Projected shadow[0405]Vržený stín");
			return S_OK;
		}
		catch (...)
		{
			return E_NOTIMPL;
		}
	}

	STDMETHOD(PreviewIconID)(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
	{
		if (a_pIconID == NULL)
			return E_POINTER;
		*a_pIconID = __uuidof(RasterImageProjectedShadow);
		return S_OK;
	}
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
				IRPathPoint const sphere[] =
				{
					{224, 96, 0, -53.0193, 0, 53.0193},
					{128, 0, -53.0193, 0, 53.0193, 0},
					{32, 96, 0, 53.0193, 0, -53.0193},
					{128, 192, 53.0193, 0, -53.0193, 0},
				};
				IRPathPoint const highlight[] =
				{
					{105, 50, -4.79583, -5.71544, 4.79583, 5.71544},
					{83, 50, -6.90661, 5.79534, 6.90661, -5.79534},
					{79, 70, 4.79583, 5.71544, -4.79583, -5.71544},
					{101, 70, 6.90661, -5.79534, -6.90661, 5.79534},
				};
				IRPathPoint const shade[] =
				{
					{210, 60, 13.8071, 30.3757, -9, 70},
					{191, 159, -23.7482, 23.7482, 23.7482, -23.7482},
					{92, 178, 70, -9, 30.3757, 13.8071},
				};
				IRPathPoint const shadow1[] =
				{
					{128, 255, 75.25, 0, -75.25, 0},
					{254, 213, -5.83151, -22.743, 7.00002, 27.3},
					{128, 177, -52.5, 0, 52.5, 0},
					{2, 213, -7, 27.3, 5.83153, -22.743},
				};
				IRPathPoint const shadow2[] =
				{
					{128, 252, 69.23, 0, -69.23, 0},
					{244, 213, -5.36499, -20.9236, 6.44, 25.116},
					{128, 180, -48.3, 0, 48.3, 0},
					{12, 213, -6.44, 25.116, 5.36501, -20.9236},
				};
				IRPathPoint const shadow3[] =
				{
					{128, 248, 63.6916, 0, -63.6916, 0},
					{235, 212, -4.93579, -19.2497, 5.9248, 23.1067},
					{128, 182, -44.436, 0, 44.436, 0},
					{21, 212, -5.9248, 23.1067, 4.93581, -19.2497},
				};

				IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
				IRFill matWhite(0x7fffffff);
				IRFill matBlack(0x7f000000);
				IRFill matShadow(0x18000000);
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&canvas, itemsof(shadow1), shadow1, &matShadow);
				cRenderer(&canvas, itemsof(shadow2), shadow2, &matShadow);
				cRenderer(&canvas, itemsof(shadow3), shadow3, &matShadow);
				cRenderer(&canvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme1Color2));
				cRenderer(&canvas, itemsof(highlight), highlight, &matWhite);
				cRenderer(&canvas, itemsof(shade), shade, &matBlack);
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	//STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	//{
	//	if (a_phIcon == NULL)
	//		return E_POINTER;
	//	try
	//	{
	//		static float const f = 1.0f/256.0f;
	//		//static TPolyCoords const aCoords1[] =
	//		//{
	//		//	{f*121, f*58}, {f*141, f*58}, {f*141, f*123}, {f*161, f*123}, {f*161, f*237}, {f*41, f*237}, {f*41, f*123}, {f*61, f*123}, {f*61, f*58}, {f*81, f*58}, {f*81, f*25}, {f*121, f*25}
	//		//};
	//		//static TPolyCoords const aCoords2[] =
	//		//{
	//		//	{f*174, f*247}, {f*225, f*189}, {f*197, f*189}, {f*221, f*160}, {f*196, f*160}, {f*205, f*148}, {f*182, f*148}, {f*173, f*160}, {f*140, f*160}, {f*69, f*247}
	//		//};
	//		COLORREF clrOut = GetSysColor(COLOR_WINDOWTEXT);
	//		//COLORREF clrSh = GetSysColor(COLOR_BTNFACE);
	//		//TIconPolySpec tPolySpec[2];
	//		//tPolySpec[0].nVertices = itemsof(aCoords2);
	//		//tPolySpec[0].pVertices = aCoords2;
	//		//tPolySpec[0].interior = agg::rgba8(GetRValue(clrSh), GetGValue(clrSh), GetBValue(clrSh), 255);
	//		//tPolySpec[0].outline = agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
	//		//tPolySpec[1].nVertices = itemsof(aCoords1);
	//		//tPolySpec[1].pVertices = aCoords1;
	//		//tPolySpec[1].interior = GetIconFillColor();
	//		//tPolySpec[1].outline = tPolySpec[0].outline;
	//		//*a_phIcon = IconFromPolygon(itemsof(tPolySpec), tPolySpec, a_nSize, false);
	//		static TPolyCoords const aCoords1[] =
	//		{
	//			{f*214, f*97},
	//			{0, -f*47.4965}, {f*47.4965, 0}, {f*128, f*11},
	//			{-f*47.4965, 0}, {0, -f*47.4965}, {f*42, f*97},
	//			{0, f*47.4965}, {-f*47.4965, 0}, {f*128, f*183},
	//			{f*47.4965, 0}, {0, f*47.4965}, {f*214, f*97}
	//		};
	//		static TPolyCoords const aCoords2[] =
	//		{
	//			{f*128, f*244}, 
	//			{f*53.75, 0}, {f*5, f*19.5}, {f*218, f*214},
	//			{-f*4.16538, -f*16.245}, {-f*37.5, 0}, {f*128, f*188},
	//			{f*37.5, 0}, {f*4.16538, -f*16.245}, {f*38, f*214},
	//			{-f*5, f*19.5}, {-f*53.75, 0}, {f*128, f*244}
	//		};
	//		TIconPolySpec tPolySpec[2];
	//		tPolySpec[0].nVertices = itemsof(aCoords2);
	//		tPolySpec[0].pVertices = aCoords2;
	//		tPolySpec[0].interior = GetIconShadowColor();
	//		tPolySpec[0].outline = tPolySpec[0].interior;//agg::rgba8(0, 0, 0, 0);
	//		tPolySpec[0].outline.a = 128;
	//		tPolySpec[1].nVertices = itemsof(aCoords1);
	//		tPolySpec[1].pVertices = aCoords1;
	//		tPolySpec[1].interior = GetIconFillColor();
	//		tPolySpec[1].outline = agg::rgba8(GetRValue(clrOut), GetGValue(clrOut), GetBValue(clrOut), 255);
	//		*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
	//		return S_OK;
	//	}
	//	catch (...)
	//	{
	//		return E_UNEXPECTED;
	//	}
	//}

	inline agg::rgba8 GetIconShadowColor()
	{
		COLORREF clr3D = GetSysColor(COLOR_WINDOWTEXT);
		float const f3DR = powf(GetRValue(clr3D)/255.0f, 2.2f);
		float const f3DG = powf(GetGValue(clr3D)/255.0f, 2.2f);
		float const f3DB = powf(GetBValue(clr3D)/255.0f, 2.2f);
		float const f3DA = 0.64f;
		float const fA = 0.36f;
		return agg::rgba8(255.0f*(f3DR*f3DA+fA)+0.5f, 255.0f*(f3DG*f3DA+fA)+0.5f, 255.0f*(f3DB*f3DA+fA)+0.5f, 255);
	}
	// ICanvasInteractingOperation
public:
	STDMETHOD(CreateWrapper)(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY, ICanvasInteractingWrapper** a_ppWrapper)
	{
		if (a_ppWrapper == NULL)
			return E_POINTER;
		try
		{
			CComObject<CWrapper>* p = NULL;
			CComObject<CWrapper>::CreateInstance(&p);
			CComPtr<ICanvasInteractingWrapper> pNew = p;
			p->Init(a_nSizeX, a_nSizeY, a_pConfig);
			*a_ppWrapper = pNew.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CWrapper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ICanvasInteractingWrapper
	{
	public:
		float* Init(ULONG a_nSizeX, ULONG a_nSizeY, float const* a_aCoords)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			for (int i = 0; i < 8; ++i)
				m_aCoords[i] = a_aCoords[i];
			return m_aCoords;
		}
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			CConfigValue c1X;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1X), &c1X);
			m_aCoords[0] = a_nSizeX*c1X.operator float();
			CConfigValue c1Y;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1Y), &c1Y);
			m_aCoords[1] = a_nSizeY*c1Y.operator float();

			CConfigValue c2X;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2X), &c2X);
			m_aCoords[2] = a_nSizeX*c2X.operator float();
			CConfigValue c2Y;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2Y), &c2Y);
			m_aCoords[3] = a_nSizeY*c2Y.operator float();

			CConfigValue c3X;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3X), &c3X);
			m_aCoords[4] = a_nSizeX*c3X.operator float();
			CConfigValue c3Y;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3Y), &c3Y);
			m_aCoords[5] = a_nSizeY*c3Y.operator float();

			CConfigValue c4X;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4X), &c4X);
			m_aCoords[6] = a_nSizeX*c4X.operator float();
			CConfigValue c4Y;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4Y), &c4Y);
			m_aCoords[7] = a_nSizeY*c4Y.operator float();
		}

	BEGIN_COM_MAP(CWrapper)
		COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
	END_COM_MAP()

		// ICanvasInteractingWrapper methods
	public:
		STDMETHOD_(ULONG, GetControlPointCount)()
		{
			return 8;
		}
		STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
		{
			if (a_nIndex < 4)
			{
				if (a_pPos)
				{
					a_pPos->fX = m_aCoords[a_nIndex+a_nIndex];
					a_pPos->fY = m_aCoords[a_nIndex+a_nIndex+1];
				}
				if (a_pClass)
					*a_pClass = 0;
				return S_OK;
			}
			if (a_nIndex < 8)
			{
				if (a_pPos)
				{
					ULONG i1 = a_nIndex&3;
					ULONG i2 = (a_nIndex+1)&3;
					a_pPos->fX = 0.5f*(m_aCoords[i1+i1]+m_aCoords[i2+i2]);
					a_pPos->fY = 0.5f*(m_aCoords[i1+i1+1]+m_aCoords[i2+i2+1]);
				}
				if (a_pClass)
					*a_pClass = 1;
				return S_OK;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel)
		{
			if (a_ppNew == NULL)
				return E_POINTER;
			try
			{
				if (a_pNewSel)
					*a_pNewSel = a_nIndex;
				if (a_nIndex < 4)
				{
					if (a_pPos->fX != m_aCoords[a_nIndex+a_nIndex] ||
						a_pPos->fY != m_aCoords[a_nIndex+a_nIndex+1]) // or use epsilon? ...not that important
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						float* pCoords = p->Init(m_nSizeX, m_nSizeY, m_aCoords);
						pCoords[a_nIndex+a_nIndex] = a_pPos->fX;
						pCoords[a_nIndex+a_nIndex+1] = a_pPos->fY;
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				if (a_nIndex < 8)
				{
					ULONG i1 = a_nIndex&3;
					ULONG i2 = (a_nIndex+1)&3;
					float fX = 0.5f*(m_aCoords[i1+i1]+m_aCoords[i2+i2]);
					float fY = 0.5f*(m_aCoords[i1+i1+1]+m_aCoords[i2+i2+1]);
					if (a_pPos->fX != fX || a_pPos->fY != fY) // or use epsilon? ...not that important
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						float* pCoords = p->Init(m_nSizeX, m_nSizeY, m_aCoords);
						pCoords[i1+i1] += a_pPos->fX-fX;
						pCoords[i1+i1+1] += a_pPos->fY-fY;
						pCoords[i2+i2] += a_pPos->fX-fX;
						pCoords[i2+i2+1] += a_pPos->fY-fY;
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				return E_RW_INDEXOUTOFRANGE;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
		{
			a_pLines->MoveTo(m_aCoords[0], m_aCoords[1]);
			a_pLines->LineTo(m_aCoords[2], m_aCoords[3]);
			a_pLines->LineTo(m_aCoords[4], m_aCoords[5]);
			a_pLines->LineTo(m_aCoords[6], m_aCoords[7]);
			a_pLines->Close();
			return S_OK;
		}

		STDMETHOD(ToConfig)(IConfig* a_pConfig)
		{
			CComBSTR c1X(CFGID_PT1X);
			CComBSTR c1Y(CFGID_PT1Y);
			CComBSTR c2X(CFGID_PT2X);
			CComBSTR c2Y(CFGID_PT2Y);
			CComBSTR c3X(CFGID_PT3X);
			CComBSTR c3Y(CFGID_PT3Y);
			CComBSTR c4X(CFGID_PT4X);
			CComBSTR c4Y(CFGID_PT4Y);
			BSTR aIDs[8] = {c1X, c1Y, c2X, c2Y, c3X, c3Y, c4X, c4Y};
			TConfigValue aVals[8];
			for (int i = 0; i < 8; i+=2)
			{
				aVals[i] = CConfigValue(m_aCoords[i]/m_nSizeX);
				aVals[i+1] = CConfigValue(m_aCoords[i+1]/m_nSizeY);
			}
			return a_pConfig->ItemValuesSet(8, aIDs, aVals);
		}

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		float m_aCoords[8];
	};
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageProjectedShadow), CRasterImageProjectedShadow)
