
#pragma once

#include <math.h>
#include "Noise.h"

class CFractalBrownianMotion : public CNoise
{
public:
	CFractalBrownianMotion(float a_H, float a_lacunarity, float a_octaves)
	{
		H = a_H;
		lacunarity = a_lacunarity;
		octaves = a_octaves;

		exponents.Attach(new float[(int)octaves+1]);
		float frequency = 1.0f;
		for (int i = 0; i <= (int)octaves; i++)
		{
			exponents[i] = powf(frequency, -H);
			frequency *= lacunarity;
		}
	}

	float evaluate(float x, float y)
	{
		float value = 0.0f;
		float remainder;
		int i;
		
		// to prevent "cascading" effects
		x += 371;
		y += 529;
		
		for (i = 0; i < (int)octaves; i++)
		{
			value += noise2(x, y) * exponents[i];
			x *= lacunarity;
			y *= lacunarity;
		}

		remainder = octaves - (int)octaves;
		if (remainder != 0)
			value += remainder * noise2(x, y) * exponents[i];

		return value;
	}

private:
	CAutoVectorPtr<float> exponents;
	float H;
	float lacunarity;
	float octaves;
};


struct CFillStyleDataClouds
{
	CFillStyleDataClouds()
	{
		bAutoCenter = true;
		tColor1 = DefaultColor1();
		tColor2 = DefaultColor2();
		nSeed = 123456789;
		scale = 30.0f;
		stretch = 3.0f;
		angle = 90.0f;
		bias = -0.25f;
		amount = 1.5f;
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		int nLen = SysStringLen(a_bstr);
		if (nLen < 9)
			return S_FALSE;
		wchar_t cAC = L'C';
		int nCommas = 0;
		for (wchar_t const* p = a_bstr; *p; ++p) if (*p == L',') ++nCommas;
		if (nCommas == 14)
		{
			swscanf(a_bstr, L"%c,%f,%f,%f,%f,%f,%f,%f,%f,%i,%f,%f,%f,%f,%f", &cAC, &tColor1.fR, &tColor1.fG, &tColor1.fB, &tColor1.fA, &tColor2.fR, &tColor2.fG, &tColor2.fB, &tColor2.fA, &nSeed, &angle, &scale, &stretch, &bias, &amount);
		}
		else
		{
			int colorset;
			swscanf(a_bstr, L"%c,%i,%i,%f,%f,%f,%f,%f", &cAC, &colorset, &nSeed, &angle, &scale, &stretch, &bias, &amount);
		}
		bAutoCenter = cAC == 'C';
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t sz[300];
		swprintf(sz, L"%c,%g,%g,%g,%g,%g,%g,%g,%g,%i,%g,%g,%g,%g,%g", bAutoCenter ? L'C' : L'G', tColor1.fR, tColor1.fG, tColor1.fB, tColor1.fA, tColor2.fR, tColor2.fG, tColor2.fB, tColor2.fA, nSeed, angle, scale, stretch, bias, amount);
		CComBSTR bstr(sz);
		*a_pbstr = bstr;
		bstr.Detach();
		return S_OK;
	}
	bool operator!=(CFillStyleDataClouds const& a_cData) const
	{
		return bAutoCenter != a_cData.bAutoCenter || a_cData.nSeed != nSeed ||
			fabsf(a_cData.tColor1.fR-tColor1.fR) > 1e-4f || fabsf(a_cData.tColor1.fG-tColor1.fG) > 1e-4f ||
			fabsf(a_cData.tColor1.fB-tColor1.fB) > 1e-4f || fabsf(a_cData.tColor1.fA-tColor1.fA) > 1e-4f ||
			fabsf(a_cData.tColor2.fR-tColor2.fR) > 1e-4f || fabsf(a_cData.tColor2.fG-tColor2.fG) > 1e-4f ||
			fabsf(a_cData.tColor2.fB-tColor2.fB) > 1e-4f || fabsf(a_cData.tColor2.fA-tColor2.fA) > 1e-4f ||
			a_cData.scale != scale || a_cData.stretch != stretch ||
			a_cData.angle != angle || a_cData.bias != bias || a_cData.amount != amount;
	}

	static TColor DefaultColor1()
	{
		TColor const t = {1.0f, 1.0f, 1.0f, 1.0f};
		return t;
	}
	static TColor DefaultColor2()
	{
		TColor const t = {0.0385f, 0.187f, 0.612f, 1.0f}; // sRGB: 58, 119, 204
		return t;
	}

	bool bAutoCenter;
	TColor tColor1;
	TColor tColor2;
	int nSeed;
	float scale;
	float stretch;
	float angle;
	float bias;
	float amount;
};

#include "FillStyleCloudsDlg.h"


class CFillStyleClouds :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush,
	public IRasterImageEditToolScripting
{
public:
	CFillStyleClouds()
	{
		m_tAbsCenter.fX = m_tAbsCenter.fY = m_tRelCenter.fX = m_tRelCenter.fY = m_fAngle = 0.0f;
	}

	BEGIN_COM_MAP(CFillStyleClouds)
		COM_INTERFACE_ENTRY(IRasterImageBrush)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageBrush methods
public:
	STDMETHOD(Init)(IRasterImageBrushOwner* a_pOwner)
	{
		m_pOwner = a_pOwner;
		if (a_pOwner)
		{
			ULONG nX = 0;
			ULONG nY = 0;
			a_pOwner->Size(&nX, &nY);
			m_tRelCenter.fX = m_tAbsCenter.fX = nX*0.5f;
			m_tRelCenter.fY = m_tAbsCenter.fY = nY*0.5f;
		}
		return S_OK;
	}
	STDMETHOD(SetShapeBounds)(TPixelCoords const* a_pCenter, float UNREF(a_fSizeX), float UNREF(a_fSizeY), float a_fAngle)
	{
		m_tRelCenter = *a_pCenter;
		m_fAngle = a_fAngle;
		//if (m_cData.bAutoCenter)
		//{
		//	m_bAbsValid = false;
		//	if (m_pOwner)
		//	{
		//		//m_pOwner->ControlPointChanged(0);
		//		//m_pOwner->ControlPointChanged(1);
		//		//m_pOwner->ControlLinesChanged();
		//	}
		//}
		return S_OK;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComBSTR bstr;
		if (FAILED(a_pState->ToText(&bstr)))
			return E_FAIL;
		if (SUCCEEDED(m_cData.FromString(bstr)))
		{
			if (m_pOwner) m_pOwner->RectangleChanged(NULL);
		}
		return S_OK;
	}
	STDMETHOD(GetState)(ISharedState** a_ppState)
	{
		try
		{
			*a_ppState = NULL;
			CComPtr<ISharedState> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateString));
			CComBSTR bstr;
			m_cData.ToString(&bstr);
			pTmp->FromText(bstr);
			*a_ppState = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(NeedsPrepareShape)()
	{
		return S_FALSE;
	}
	STDMETHOD(PrepareShape)(RECT const* a_pBounds, ULONG a_nPaths, TRWPolygon const* a_pPaths)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsSolid)(RECT const* a_pRect)
	{
		return S_FALSE;
	}
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			if (a_fGamma < 0.1f || a_fGamma > 10.0f) a_fGamma = 1.0f;
			TRasterImagePixel aMap[256];
			float const fGammaInv = 1.0f/a_fGamma;
			for (ULONG i = 0; i < 256; ++i)
			{
				TColor tC1;
				tC1.fR = m_cData.tColor1.fR*m_cData.tColor1.fA;
				tC1.fG = m_cData.tColor1.fG*m_cData.tColor1.fA;
				tC1.fB = m_cData.tColor1.fB*m_cData.tColor1.fA;
				tC1.fA = m_cData.tColor1.fA;
				TColor tC2;
				tC2.fR = m_cData.tColor2.fR*m_cData.tColor2.fA;
				tC2.fG = m_cData.tColor2.fG*m_cData.tColor2.fA;
				tC2.fB = m_cData.tColor2.fB*m_cData.tColor2.fA;
				tC2.fA = m_cData.tColor2.fA;
				ULONG const i2 = 255-i;
				float const fA = i*tC1.fA + i2*tC2.fA;
				BYTE bA = fA;
				if (bA)
				{
					float const fIA = 1.0f/fA;
					aMap[i].bR = powf((tC1.fR*i + tC2.fR*i2)*fIA, fGammaInv)*255.0f+0.5f;
					aMap[i].bG = powf((tC1.fG*i + tC2.fG*i2)*fIA, fGammaInv)*255.0f+0.5f;
					aMap[i].bB = powf((tC1.fB*i + tC2.fB*i2)*fIA, fGammaInv)*255.0f+0.5f;
					aMap[i].bA = bA;
				}
				else
				{
					static TRasterImagePixel const t0 = {0, 0, 0, 0};
					aMap[i] = t0;
				}
			}

			if (m_pOwner)
			{
				ULONG nX = 0;
				ULONG nY = 0;
				m_pOwner->Size(&nX, &nY);
				m_tAbsCenter.fX = nX*0.5f;
				m_tAbsCenter.fY = nY*0.5f;
			}
			float const fDX = m_cData.bAutoCenter ? m_tRelCenter.fX : m_tAbsCenter.fX;
			float const fDY = m_cData.bAutoCenter ? m_tRelCenter.fY : m_tAbsCenter.fY;
			float const a = m_cData.angle*3.14159265f/180.0f + (m_cData.bAutoCenter ? m_fAngle : 0.0f);
			float const fC = cosf(a);
			float const fS = sinf(a);
			CFractalBrownianMotion m_cFBM(1.0f, 2.0f, 4.0f);
			m_cFBM.SetSeed(m_cData.nSeed);
			float min = 0.0f;
			float max = 1.0f;
			CNoise::findRange(m_cFBM, min, max);
			float const bias = -0.5f*m_cData.amount+0.5f+m_cData.bias;
			float fScaleX = fabsf(m_cData.scale);
			if (fScaleX < 1e-4f) fScaleX = 1e-4f;
			float fScaleY = fScaleX*fabsf(m_cData.stretch);
			if (fScaleY < 1e-4f) fScaleY = 1e-4f;
			fScaleX = 1.0f/fScaleX;
			fScaleY = 1.0f/fScaleY;
			for (ULONG y = 0; y < a_nSizeY; ++y)
			{
				for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer)
				{
					float nx = fC*(x+a_nX-fDX) + fS*(y+a_nY-fDY);
					float ny =-fS*(x+a_nX-fDX) + fC*(y+a_nY-fDY);
					nx *= fScaleX;
					ny *= fScaleY;

					float f = m_cFBM.evaluate(nx, ny);
					f = (f-min)/(max-min);
					f = f*m_cData.amount+bias;
					if (f <= 0.0f) f = 0.0f; else if (f > 1.0f) f = 1.0f;

					*a_pBuffer = aMap[BYTE(f*255.0f+0.5f)];
				}
				a_pBuffer += a_nStride-a_nSizeX;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
		return S_OK;
	}

	STDMETHOD(AdjustCoordinates)(TPixelCoords* a_pPos, ULONG a_nControlPointIndex)
	{
		return S_OK;
	}
	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		try
		{
			*a_pCount = 0;
			return S_OK;
		}
		catch (...)
		{
			return a_pCount ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize)
	{
		return E_RW_INDEXOUTOFRANGE;
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines)
	{
		return S_FALSE;
	}

	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return S_FALSE;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CFillStyleDataClouds cData = m_cData;
			HRESULT hRes = m_cData.FromString(a_bstrParams);
			if (SUCCEEDED(hRes) && m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				if (cData != m_cData)
				{
					CComPtr<ISharedState> pTmp;
					RWCoCreateInstance(pTmp, __uuidof(SharedStateString));
					CComBSTR bstr;
					m_cData.ToString(&bstr);
					pTmp->FromText(bstr);
					m_pOwner->SetBrushState(pTmp);
				}
			}
			return hRes;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		return m_cData.ToString(a_pbstrParams);
	}

private:

private:
	CComPtr<IRasterImageBrushOwner> m_pOwner;
	TPixelCoords m_tAbsCenter;
	float m_fAngle;
	TPixelCoords m_tRelCenter;
	CFillStyleDataClouds m_cData;
};

