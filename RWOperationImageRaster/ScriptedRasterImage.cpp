// ScriptedRasterImage.cpp : Implementation of CScriptedRasterImage

#include "stdafx.h"
#include "ScriptedRasterImage.h"


// CScriptedRasterImage


STDMETHODIMP CScriptedRasterImage::GetPixelColor(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, OLE_COLOR* pixelColor)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*pixelColor = RGB(tPixel.bR, tPixel.bG, tPixel.bB);
		return S_OK;
	}
	catch (...)
	{
		return pixelColor == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelAlpha(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelAlpha)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*pixelAlpha = tPixel.bA;
		return S_OK;
	}
	catch (...)
	{
		return pixelAlpha == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelRed(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelRed)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*pixelRed = tPixel.bR;
		return S_OK;
	}
	catch (...)
	{
		return pixelRed == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelGreen(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelGreen)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*pixelGreen = tPixel.bG;
		return S_OK;
	}
	catch (...)
	{
		return pixelGreen == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelBlue(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelBlue)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*pixelBlue = tPixel.bB;
		return S_OK;
	}
	catch (...)
	{
		return pixelBlue == NULL ? E_POINTER : E_UNEXPECTED;
	}
}


STDMETHODIMP CScriptedRasterImage::GetPixel(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, ULONG* packedPixel)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		*packedPixel = tPixel.n;
		return S_OK;
	}
	catch (...)
	{
		return packedPixel == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetPixelColor(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, OLE_COLOR pixelColor)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		tPixel.bR = GetRValue(pixelColor);
		tPixel.bG = GetGValue(pixelColor);
		tPixel.bB = GetBValue(pixelColor);
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetPixelAlpha(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelAlpha)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		tPixel.bA = pixelAlpha;
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetPixelRed(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelRed)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		tPixel.bR = pixelRed;
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetPixelGreen(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelGreen)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		tPixel.bG = pixelGreen;
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetPixelBlue(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelBlue)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		tPixel.bB = pixelBlue;
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CScriptedRasterImage::SetPixel(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, ULONG packedPixel)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, reinterpret_cast<TPixelChannel*>(&packedPixel), FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_sizeX(ULONG* pVal)
{
	try
	{
		TImageSize tCoord = {0, 0};
		HRESULT hRes = m_pImage->CanvasGet(&tCoord, NULL, NULL, NULL, NULL);
		if (FAILED(hRes))
			return hRes;
		*pVal = tCoord.nX;
		return S_OK;
	}
	catch (...)
	{
		return pVal == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_sizeY(ULONG* pVal)
{
	try
	{
		TImageSize tCoord = {0, 0};
		HRESULT hRes = m_pImage->CanvasGet(&tCoord, NULL, NULL, NULL, NULL);
		if (FAILED(hRes))
			return hRes;
		*pVal = tCoord.nY;
		return S_OK;
	}
	catch (...)
	{
		return pVal == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_sizeZ(ULONG* pVal)
{
	try
	{
		*pVal = 1;
		return S_OK;
	}
	catch (...)
	{
		return pVal == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_sizeW(ULONG* pVal)
{
	try
	{
		*pVal = 1;
		return S_OK;
	}
	catch (...)
	{
		return pVal == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::FillResize(ULONG sizeX, ULONG sizeY, ULONG sizeZ, ULONG sizeW, LONG offsetX, LONG offsetY, LONG offsetZ, LONG offsetW, ULONG fillColor)
{
	try
	{
		CPixelChannel fill(fillColor);
		TImageSize tOldSize = {0, 0};
		m_pImage->CanvasGet(&tOldSize, NULL, NULL, NULL, NULL);
		RECT const rcOld = {offsetX, offsetY, offsetX+tOldSize.nX, offsetY+tOldSize.nY};
		RECT rcNew = {0, 0, sizeX, sizeY};
		TMatrix3x3f tTransform = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  offsetX, offsetY, 1.0f};
		TImageSize tSize = {sizeX, sizeY};
		HRESULT hRes = m_pImage->CanvasSet(&tSize, NULL, &tTransform, NULL);
		TImageTile aTiles[5];
		ULONG nTile = 0;
		if (rcNew.top >= rcOld.bottom || rcNew.left >= rcOld.right || 
			rcOld.top >= rcNew.bottom || rcOld.left >= rcNew.right)
		{
			aTiles[nTile].nChannelIDs = EICIRGBA;
			aTiles[nTile].nPixels = 1;
			aTiles[nTile].pData = &fill;
			aTiles[nTile].nPixels = 1;
			aTiles[nTile].tStride.nX = 0;
			aTiles[nTile].tStride.nY = 0;
			aTiles[nTile].tOrigin.nX = 0;
			aTiles[nTile].tOrigin.nY = 0;
			aTiles[nTile].tSize = tSize;
			++nTile;
		}
		else
		{
			if (rcOld.top > rcNew.top)
			{
				aTiles[nTile].nChannelIDs = EICIRGBA;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].pData = &fill;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].tStride.nX = 0;
				aTiles[nTile].tStride.nY = 0;
				aTiles[nTile].tOrigin.nX = 0;
				aTiles[nTile].tOrigin.nY = 0;
				aTiles[nTile].tSize.nX = tSize.nX;
				aTiles[nTile].tSize.nY = rcOld.top-rcNew.top;
				++nTile;
				rcNew.top = rcOld.top;
			}
			if (rcOld.bottom < rcNew.bottom)
			{
				aTiles[nTile].nChannelIDs = EICIRGBA;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].pData = &fill;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].tStride.nX = 0;
				aTiles[nTile].tStride.nY = 0;
				aTiles[nTile].tOrigin.nX = 0;
				aTiles[nTile].tOrigin.nY = rcOld.bottom;
				aTiles[nTile].tSize.nX = tSize.nX;
				aTiles[nTile].tSize.nY = rcNew.bottom-rcOld.bottom;
				++nTile;
				rcNew.bottom = rcOld.bottom;
			}
			if (rcOld.left > rcNew.left)
			{
				aTiles[nTile].nChannelIDs = EICIRGBA;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].pData = &fill;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].tStride.nX = 0;
				aTiles[nTile].tStride.nY = 0;
				aTiles[nTile].tOrigin.nX = 0;
				aTiles[nTile].tOrigin.nY = rcNew.top;
				aTiles[nTile].tSize.nX = rcOld.left-rcNew.left;
				aTiles[nTile].tSize.nY = rcNew.bottom-rcNew.top;
				++nTile;
			}
			if (rcOld.right < rcNew.right)
			{
				aTiles[nTile].nChannelIDs = EICIRGBA;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].pData = &fill;
				aTiles[nTile].nPixels = 1;
				aTiles[nTile].tStride.nX = 0;
				aTiles[nTile].tStride.nY = 0;
				aTiles[nTile].tOrigin.nX = rcOld.right;
				aTiles[nTile].tOrigin.nY = rcNew.top;
				aTiles[nTile].tSize.nX = rcNew.right-rcOld.right;
				aTiles[nTile].tSize.nY = rcNew.bottom-rcNew.top;
				++nTile;
			}
		}
		for (ULONG n = 0; n < nTile; ++n)
			m_pImage->TileSet(aTiles[n].nChannelIDs, &aTiles[n].tOrigin, &aTiles[n].tSize, &aTiles[n].tStride, aTiles[n].nPixels, aTiles[n].pData, 0);
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static const float f1_255 = 1.0f/255.0f;
STDMETHODIMP CScriptedRasterImage::GetPixelH(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelH)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		float r = tPixel.bR*f1_255;
		float g = tPixel.bG*f1_255;
		float b = tPixel.bB*f1_255;
		float bc, gc, rc, rgbmax, rgbmin, h, l, s;

		// Compute luminosity.
		rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
		rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
		l = (rgbmax + rgbmin) * 0.5f;

		// Compute saturation.
		if (rgbmax == rgbmin)
			s = 0.0f;
		else if (l <= 0.5f)
			s = (rgbmax - rgbmin) / (rgbmax + rgbmin);
		else
			s = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

		// Compute the hue.
		if (rgbmax == rgbmin)
			h = 0.0f;
		else
		{
			rc = (rgbmax - r) / (rgbmax - rgbmin);
			gc = (rgbmax - g) / (rgbmax - rgbmin);
			bc = (rgbmax - b) / (rgbmax - rgbmin);

			if (r == rgbmax)
				h = bc - gc;
			else if (g == rgbmax)
				h = 2.0f + rc - bc;
			else
				h = 4.0f + gc - rc;

			h *= 60.0f;
			h = h - 360.0f*(int)(h/360.0f);
		}
		*pixelH = h;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelL(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelL)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		float r = tPixel.bR*f1_255;
		float g = tPixel.bG*f1_255;
		float b = tPixel.bB*f1_255;
		float rgbmax, rgbmin;

		// Compute luminosity.
		rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
		rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
		*pixelL = (rgbmax + rgbmin) * 0.5f;

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::GetPixelS(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelS)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		float r = tPixel.bR*f1_255;
		float g = tPixel.bG*f1_255;
		float b = tPixel.bB*f1_255;
		float rgbmax, rgbmin, l;

		// Compute luminosity.
		rgbmax = r>g ? (r>b ? r : b) : (g>b ? g : b);
		rgbmin = r<g ? (r<b ? r : b) : (g<b ? g : b);
		l = (rgbmax + rgbmin) * 0.5f;

		// Compute saturation.
		if (rgbmax == rgbmin)
			*pixelS = 0.0f;
		else if (l <= 0.5f)
			*pixelS = (rgbmax - rgbmin) / (rgbmax + rgbmin);
		else
			*pixelS = (rgbmax - rgbmin) / (2.0f - rgbmax - rgbmin);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static float hls_value(float n1, float n2, float h)
{
	h += 360.0f;
	float hue = h - 360.0f*(int)(h/360.0f);

	if (hue < 60.0f)
		return n1 + ( n2 - n1 ) * hue / 60.0f;
	else if (hue < 180.0f)
		return n2;
	else if (hue < 240.0f)
		return n1 + ( n2 - n1 ) * ( 240.0f - hue ) / 60.0f;
	else
		return n1;
}

STDMETHODIMP CScriptedRasterImage::SetPixelHLS(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float h, float l, float s)
{
	try
	{
		TImagePoint const tCoord = {posX, posY};
		static TImageSize const tSize = {1, 1};
		TPixelChannel tPixel;
		HRESULT hRes = m_pImage->TileGet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;

		float m1, m2, r, g, b;
		m2 = l + (l <= 0.5f ? l*s : s - l*s);
		m1 = 2.0f * l - m2;
		if (s == 0.0f)
			r = g = b = l;
		else
		{
			r = hls_value(m1, m2, h+120.0f);
			g = hls_value(m1, m2, h);
			b = hls_value(m1, m2, h-120.0f);
		}
		if (r > 1.0f) r = 1.0f;
		if (g > 1.0f) g = 1.0f;
		if (b > 1.0f) b = 1.0f;
		if (r < 0.0f) r = 0.0f;
		if (g < 0.0f) g = 0.0f;
		if (b < 0.0f) b = 0.0f;

		tPixel.bR = r*255.0f;
		tPixel.bG = g*255.0f;
		tPixel.bB = b*255.0f;
		return m_pImage->TileSet(EICIRGBA, &tCoord, &tSize, NULL, 1, &tPixel, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::HasAlpha(VARIANT_BOOL* alpha)
{
	try
	{
		*alpha = VARIANT_TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetAlpha(VARIANT_BOOL alpha)
{
	try
	{
		return alpha == VARIANT_FALSE ? E_FAIL : S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionXNum(ULONG* pVal)
{
	try
	{
		TImageResolution tRes = {100, 254, 100, 254};
		m_pImage->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
		*pVal = tRes.nNumeratorX;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionXDenom(ULONG* pVal)
{
	try
	{
		TImageResolution tRes = {100, 254, 100, 254};
		m_pImage->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
		*pVal = tRes.nDenominatorX;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionYNum(ULONG* pVal)
{
	try
	{
		TImageResolution tRes = {100, 254, 100, 254};
		m_pImage->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
		*pVal = tRes.nNumeratorY;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionYDenom(ULONG* pVal)
{
	try
	{
		TImageResolution tRes = {100, 254, 100, 254};
		m_pImage->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
		*pVal = tRes.nDenominatorY;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionZNum(ULONG* pVal)
{
	try
	{
		*pVal = 0;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionZDenom(ULONG* pVal)
{
	try
	{
		*pVal = 0;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionWNum(ULONG* pVal)
{
	try
	{
		*pVal = 0;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_resolutionWDenom(ULONG* pVal)
{
	try
	{
		*pVal = 0;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::SetResolution(ULONG a_nResXNum, ULONG a_nResXDenom, ULONG a_nResYNum, ULONG a_nResYDenom, ULONG a_nResZNum, ULONG a_nResZDenom, ULONG a_nResWNum, ULONG a_nResWDenom)
{
	try
	{
		TImageResolution const tRes = {a_nResXNum, a_nResXDenom, a_nResYNum, a_nResYDenom};
		return m_pImage->CanvasSet(NULL, &tRes, NULL, NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedRasterImage::get_previewScale(ULONG* pVal)
{
	try
	{
		*pVal = 1;
		ATLASSERT(FALSE); // not really supported anymore
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

