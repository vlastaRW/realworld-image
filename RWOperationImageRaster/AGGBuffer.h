
#pragma once

#include <agg_basics.h>
#include <agg_color_rgba.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_renderer_scanline.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_span_subdiv_adaptor.h>
#include <agg_span_allocator.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_image_accessors.h>
#include <GammaCorrection.h>


class CRasterImageTarget
{
public:
	typedef agg::rgba16 color_type;
	typedef void row_data;
	typedef void span_data;

    CRasterImageTarget(TPixelChannel* a_pBuffer, TImageSize const* a_pSize, TImageStride const* a_pStride, CGammaTables* a_pGamma) :
		m_pBuffer(a_pBuffer), m_tSize(*a_pSize), m_tStride(*a_pStride), m_pGamma(a_pGamma)
    {
    }

    unsigned width() const
	{
		return m_tSize.nX;
	}
    unsigned height() const
	{
		return m_tSize.nY;
	}

	//void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
	//void blend_hline(int x, int y, unsigned len, color_type const& c, agg::int8u cover)
	//void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)

	void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
	{
		if (m_pGamma)
		{
			for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
			{
				color_type c = *colors;
				c.demultiply();
				p->bB = m_pGamma->InvGamma(c.b);
				p->bG = m_pGamma->InvGamma(c.g);
				p->bR = m_pGamma->InvGamma(c.r);
				p->bA = c.a>>8;
			}
		}
		else
		{
			for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
			{
				color_type c = *colors;
				c.demultiply();
				p->bB = c.b>>8;
				p->bG = c.g>>8;
				p->bR = c.r>>8;
				p->bA = c.a>>8;
			}
		}
	}

protected:
	TPixelChannel* m_pBuffer;
	TImageSize const m_tSize;
	TImageStride const m_tStride;
	CGammaTables* m_pGamma;
};

class CRasterImageTargetNonOverlappingBlend : public CRasterImageTarget
{
public:
	typedef agg::rgba16 color_type;
	typedef void row_data;
	typedef void span_data;

    CRasterImageTargetNonOverlappingBlend(TPixelChannel* a_pBuffer, TImageSize const* a_pSize, TImageStride const* a_pStride, CGammaTables* a_pGamma) :
		CRasterImageTarget(a_pBuffer, a_pSize, a_pStride, a_pGamma)
    {
    }

	void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
	{
		if (covers == NULL/* && cover == 255*/)
		{
			CRasterImageTarget::blend_color_hspan(x, y, len, colors, covers, cover);
			return;
		}
		if (m_pGamma)
		{
			for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
			{
				color_type c = *colors;
				if (*covers == 255/* || p->bA == 0*/)
				{
					c.demultiply();
					p->bB = m_pGamma->InvGamma(c.b);
					p->bG = m_pGamma->InvGamma(c.g);
					p->bR = m_pGamma->InvGamma(c.r);
					p->bA = c.a>>8;
				}
				else
				{
					c.b = c.b*ULONG(*covers)/255;
					c.g = c.g*ULONG(*covers)/255;
					c.r = c.r*ULONG(*covers)/255;
					c.a = c.a*ULONG(*covers)/255;
					ULONG nA2 = (ULONG(p->bA)<<8)|p->bA;
					ULONG nC2 = (ULONG(*covers)<<8)|*covers;
					nA2 = min(nA2, ULONG(65535-nC2));
					ULONG nBS = m_pGamma->m_aGamma[p->bB]*nA2/65535;
					ULONG nGS = m_pGamma->m_aGamma[p->bG]*nA2/65535;
					ULONG nRS = m_pGamma->m_aGamma[p->bR]*nA2/65535;
					c.b += nBS;
					c.g += nGS;
					c.r += nRS;
					c.a += nA2;
					c.demultiply();
					p->bB = m_pGamma->InvGamma(c.b);
					p->bG = m_pGamma->InvGamma(c.g);
					p->bR = m_pGamma->InvGamma(c.r);
					p->bA = c.a>>8;
				}
			}
		}
		else
		{
			// TODO: handle covers for cases where gamma is not used
			for (TPixelChannel* p = m_pBuffer + y*m_tStride.nY+x*m_tStride.nX; len; p+=m_tStride.nX, --len, ++covers, ++colors)
			{
				color_type c = *colors;
				if (*covers == 255)
				{
					c.demultiply();
					p->bB = c.b>>8;
					p->bG = c.g>>8;
					p->bR = c.r>>8;
					p->bA = c.a>>8;
				}
			}
		}
	}
};

class CRasterImagePreMpSrc
{
public:
	typedef agg::pixfmt_rgba64::color_type color_type;
	typedef agg::pixfmt_rgba64::order_type order_type;

	CRasterImagePreMpSrc(TPixelChannel const* a_pData, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, TPixelChannel a_tDefault, CGammaTables* a_pGamma) :
		m_pData(a_pData), m_tOrigin(*a_pOrigin), m_tSize(*a_pSize), m_tStride(*a_pStride),
		m_tEnd(CImagePoint(a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY)), m_x(0), m_x0(0), m_y(0), m_pBuffer(NULL), m_pGamma(a_pGamma)
	{
		if (m_pGamma)
		{
			m_aDefault[0] = m_pGamma->m_aGamma[a_tDefault.bR]*ULONG(a_tDefault.bA)/255;
			m_aDefault[1] = m_pGamma->m_aGamma[a_tDefault.bG]*ULONG(a_tDefault.bA)/255;
			m_aDefault[2] = m_pGamma->m_aGamma[a_tDefault.bB]*ULONG(a_tDefault.bA)/255;
		}
		else
		{
			m_aDefault[0] = ((ULONG(a_tDefault.bR)<<8)|a_tDefault.bR)*ULONG(a_tDefault.bA)/255;
			m_aDefault[1] = ((ULONG(a_tDefault.bG)<<8)|a_tDefault.bG)*ULONG(a_tDefault.bA)/255;
			m_aDefault[2] = ((ULONG(a_tDefault.bB)<<8)|a_tDefault.bB)*ULONG(a_tDefault.bA)/255;
		}
		m_aDefault[3] = (ULONG(a_tDefault.bA)<<8)|a_tDefault.bA;
	}

public:
	AGG_INLINE const agg::int16u* span(int x, int y, unsigned UNREF(len))
	{
		m_x = m_x0 = x;
		m_y = y;
		if (y < m_tOrigin.nY || y >= m_tEnd.nY || x < m_tOrigin.nX || x >= m_tEnd.nX)
		{
			m_pBuffer = NULL;
			return m_aDefault;
		}
		else
		{
			m_pBuffer = m_pData + (x-m_tOrigin.nX)*m_tStride.nX+(y-m_tOrigin.nY)*m_tStride.nY;
			if (m_pGamma)
			{
				m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
			}
			else
			{
				m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
			}
			m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
			return m_aBuffer;
		}
	}

	AGG_INLINE const agg::int16u* next_x()
	{
		++m_x;
		if (m_pBuffer)
		{
			if (m_x < m_tEnd.nX)
			{
				m_pBuffer += m_tStride.nX;
				if (m_pGamma)
				{
					m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
				}
				else
				{
					m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
					m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
				}
				m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
				return m_aBuffer;
			}
			else
			{
				m_pBuffer = NULL;
				return m_aDefault;
			}
		}
		if (m_x >= m_tOrigin.nX && m_x < m_tEnd.nX && m_y >= m_tOrigin.nY && m_y < m_tEnd.nY)
		{
			m_pBuffer = m_pData + (m_x-m_tOrigin.nX)*m_tStride.nX+(m_y-m_tOrigin.nY)*m_tStride.nY;
			if (m_pGamma)
			{
				m_aBuffer[0] = m_pGamma->m_aGamma[m_pBuffer->bR]*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[1] = m_pGamma->m_aGamma[m_pBuffer->bG]*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[2] = m_pGamma->m_aGamma[m_pBuffer->bB]*ULONG(m_pBuffer->bA)/255;
			}
			else
			{
				m_aBuffer[0] = ((ULONG(m_pBuffer->bR)<<8)|m_pBuffer->bR)*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[1] = ((ULONG(m_pBuffer->bG)<<8)|m_pBuffer->bG)*ULONG(m_pBuffer->bA)/255;
				m_aBuffer[2] = ((ULONG(m_pBuffer->bB)<<8)|m_pBuffer->bB)*ULONG(m_pBuffer->bA)/255;
			}
			m_aBuffer[3] = (ULONG(m_pBuffer->bA)<<8)|m_pBuffer->bA;
			return m_aBuffer;
		}

		return m_aDefault;
	}

	AGG_INLINE const agg::int16u* next_y()
	{
		return span(m_x0, m_y+1, 0);
	}

	int dx;
	int dy;
	void prepare() {}
    void generate(color_type* span_out, int x, int y, unsigned len)
    {
		x += dx;
		y += dy;
        do
        {
			agg::int16u const* p = span(x, y, 1);
			span_out->r = p[0];
			span_out->g = p[1];
			span_out->b = p[2];
			span_out->a = p[3];
			++span_out;
			++x;
        } while(--len);
    }

private:
	TPixelChannel const* m_pData;
	TImagePoint const m_tOrigin;
	TImagePoint const m_tEnd;
	TImageSize const m_tSize;
	TImageStride const m_tStride;
	agg::int16u m_aDefault[4];
	TPixelChannel const* m_pBuffer;
	agg::int16u m_aBuffer[4];
	CGammaTables* m_pGamma;
	int m_x, m_x0, m_y;
};

