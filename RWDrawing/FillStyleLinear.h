
#pragma once

#include <math.h>

#include "FillStyleGradientDlg.h"

#include <boost/spirit.hpp>
#include <boost/spirit/actor/insert_at_actor.hpp>
using namespace boost::spirit;


class CFillStyleLinear :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush,
	public IRasterImageEditToolScripting
{
public:
	CFillStyleLinear() : m_bAbsValid(false), m_bRelValid(true)
	{
		m_fSizeX = m_tCenter.fX = 0.0f;
		m_fSizeY = m_tCenter.fY = 0.0f;
		m_fAngle = 0.0f;
		m_tPoint1.fX = m_tPoint1.fY = 0.0f;
		m_tPoint2.fX = m_tPoint2.fY = 0.0f;
		m_tRelPt1.fX = m_tRelPt1.fY = 0.1667f;
		m_tRelPt2.fX = m_tRelPt2.fY = 0.8333f;
	}

	BEGIN_COM_MAP(CFillStyleLinear)
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
			m_fSizeX = m_tCenter.fX = nX*0.5f;
			m_fSizeY = m_tCenter.fY = nY*0.5f;
			if (m_cData.bAutoCenter)
				m_bAbsValid = false;
		}
		return S_OK;
	}
	STDMETHOD(SetShapeBounds)(TPixelCoords const* a_pCenter, float a_fSizeX, float a_fSizeY, float a_fAngle)
	{
		m_fSizeX = a_fSizeX;
		m_fSizeY = a_fSizeY;
		m_tCenter = *a_pCenter;
		m_fAngle = a_fAngle;
		if (m_cData.bAutoCenter)
		{
			m_bAbsValid = false;
			if (m_pOwner)
			{
				m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CFillStyleDataGradient::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			bool bPrevAutoCenter = m_cData.bAutoCenter;
			m_cData = *(pData->InternalData());
			Invalidate1DMapCache();
			if (m_pOwner)
				m_pOwner->RectangleChanged(NULL);
			//m_tRelPt1.fX = m_tRelPt1.fY = 0.1667f;
			//m_tRelPt2.fX = m_tRelPt2.fY = 0.8333f;
			if (bPrevAutoCenter != m_cData.bAutoCenter)
				m_bAbsValid = false;
			if (!bPrevAutoCenter && m_cData.bAutoCenter)
				SetShapeBounds(&m_tCenter, m_fSizeX, m_fSizeY, m_fAngle);
			else if (bPrevAutoCenter && !m_cData.bAutoCenter && m_pOwner)
			{
				m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	STDMETHOD(GetState)(ISharedState** a_ppState)
	{
		try
		{
			*a_ppState = NULL;
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			if (pNew->Init(m_cData))
			{
				m_pOwner->SetBrushState(pTmp);
			}
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
		return S_FALSE; // TODO: consider rectangle
	}
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		UpdateAbsolutePositions();
		TPixelCoords const tDelta = {m_tPoint2.fX-m_tPoint1.fX, m_tPoint2.fY-m_tPoint1.fY};
		float const fD2 = tDelta.fX*tDelta.fX + tDelta.fY*tDelta.fY;
		float const fD = sqrtf(fD2);
		int nSteps = 256;
		while (nSteps < 4096 && fD*4 > nSteps)
			nSteps <<= 1;
		float fMul = nSteps-1;
		TRasterImagePixel* cMap = Update1DMap(a_fGamma, nSteps);
		ULONG const nMask = nSteps-1;
		float const fFactor = 1.0f/fD2;
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			float const fDY = fFactor * (a_nY+y-m_tPoint1.fY+0.5f) * tDelta.fY;
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				float fD = fDY + fFactor * (a_nX+x-m_tPoint1.fX+0.5f) * tDelta.fX;
				if (fD > 1.0f) fD = 1.0f;
				else if (fD < 0.0f) fD = 0.0f;
				*a_pBuffer = cMap[nMask&ULONG(fD*fMul+0.5f)];
				++a_pBuffer;
			}
			a_pBuffer += a_nStride-a_nSizeX;
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
			*a_pCount = 2;
			return S_OK;
		}
		catch (...)
		{
			return a_pCount ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		try
		{
			if (a_nIndex >= 2)
				return E_RW_INDEXOUTOFRANGE;
			UpdateAbsolutePositions();
			*a_pPos = a_nIndex ? m_tPoint2 : m_tPoint1;
			*a_pClass = 0;
			return S_OK;
		}
		catch (...)
		{
			return a_pPos ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize)
	{
		try
		{
			if (a_nIndex >= 2)
				return E_RW_INDEXOUTOFRANGE;
			TPixelCoords* pAbsPt = a_nIndex ? &m_tPoint2 : &m_tPoint1;
			TPixelCoords* pRelPt = a_nIndex ? &m_tRelPt2 : &m_tRelPt1;
			*pAbsPt = *a_pPos;
			if (m_cData.bAutoCenter)
			{
				float const fX = pAbsPt->fX-m_tCenter.fX;
				float const fY = pAbsPt->fY-m_tCenter.fY;
				float const fC = cosf(-m_fAngle);
				float const fS = sinf(-m_fAngle);
				pRelPt->fX = ((fC*fX-fS*fY)/m_fSizeX + 1.0f)*0.5f;
				pRelPt->fY = ((fS*fX+fC*fY)/m_fSizeY + 1.0f)*0.5f;
			}
			else
			{
				ULONG nX = 0;
				ULONG nY = 0;
				if (m_pOwner) m_pOwner->Size(&nX, &nY);
				if (nX && nY)
				{
					pRelPt->fX = pAbsPt->fX/nX;
					pRelPt->fY = pAbsPt->fY/nY;
				}
			}
			if (m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				m_pOwner->ControlPointChanged(a_nIndex);
				m_pOwner->ControlLinesChanged();
			}
			return S_OK;
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

	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines)
	{
		UpdateAbsolutePositions();
		a_pLines->MoveTo(m_tPoint1.fX, m_tPoint1.fY);
		a_pLines->LineTo(m_tPoint2.fX, m_tPoint2.fY);
		return S_OK;
	}

	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		if (!m_cData.bAutoCenter && a_pMatrix && memcmp(a_pMatrix, &TMATRIX3X3F_IDENTITY, sizeof*a_pMatrix))
		{
			float const fW1 = 1.0f/(a_pMatrix->_13*m_tPoint1.fX + a_pMatrix->_23*m_tPoint1.fY + a_pMatrix->_33);
			TPixelCoords const t1 =
			{
				fW1*(a_pMatrix->_11*m_tPoint1.fX + a_pMatrix->_21*m_tPoint1.fY + a_pMatrix->_31),
				fW1*(a_pMatrix->_12*m_tPoint1.fX + a_pMatrix->_22*m_tPoint1.fY + a_pMatrix->_32),
			};
			m_tPoint1 = t1;
			float const fW2 = 1.0f/(a_pMatrix->_13*m_tPoint2.fX + a_pMatrix->_23*m_tPoint2.fY + a_pMatrix->_33);
			TPixelCoords const t2 =
			{
				fW1*(a_pMatrix->_11*m_tPoint2.fX + a_pMatrix->_21*m_tPoint2.fY + a_pMatrix->_31),
				fW1*(a_pMatrix->_12*m_tPoint2.fX + a_pMatrix->_22*m_tPoint2.fY + a_pMatrix->_32),
			};
			m_tPoint2 = t2;
			if (m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
			//UpdateAbsolutePositions();
		}
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
			CFillStyleDataGradient cData = m_cData;
			TPixelCoords tPoint1 = {0.1667f, 0.1667f};
			TPixelCoords tPoint2 = {0.8333f, 0.8333f};
			int nMode = 2;
			CGradientColorPicker::CGradient cGradient;
			std::pair<WORD, CButtonColorPicker::SColor> t;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(*((int_p[assign_a(t.first)]>>ch_p(L',')>>real_p[assign_a(t.second.fR)]>>ch_p(L',')>>real_p[assign_a(t.second.fG)]>>ch_p(L',')>>real_p[assign_a(t.second.fB)]>>ch_p(L',')>>real_p[assign_a(t.second.fA)]>>ch_p(L','))[insert_at_a(cGradient, t.first, t.second)]))>>
					(((str_p(L"ABSOLUTE")|str_p(L"\"ABSOLUTE\""))[assign_a(nMode, 0)]|(str_p(L"RELIMAGE")|str_p(L"\"RELIMAGE\""))[assign_a(nMode, 1)]|(str_p(L"RELSHAPE")|str_p(L"\"RELSHAPE\""))[assign_a(nMode, 2)]))>>
					(!(ch_p(L',')>>real_p[assign_a(tPoint1.fX)]>>ch_p(L',')>>real_p[assign_a(tPoint1.fY)]>>ch_p(L',')>>real_p[assign_a(tPoint2.fX)]>>ch_p(L',')>>real_p[assign_a(tPoint2.fY)]))
					).full;
			if (!bParsed)
				return E_INVALIDARG;
			m_cData.bAutoCenter = nMode == 2;
			if (cGradient.size() < 2)
			{
				cGradient.clear();
				cGradient[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
				cGradient[0xffff] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
			}
			std::swap(m_cData.cGradient, cGradient);
			Invalidate1DMapCache();
			if (nMode == 0)
			{
				m_tPoint1 = tPoint1;
				m_tPoint2 = tPoint2;
				m_bAbsValid = true;
			}
			else
			{
				m_tRelPt1.fX = tPoint1.fX;
				m_tRelPt1.fY = tPoint1.fY;
				m_tRelPt2.fX = tPoint2.fX;
				m_tRelPt2.fY = tPoint2.fY;
				m_bAbsValid = false;
			}
			if (cData != m_cData && m_pOwner)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (pNew->Init(m_cData))
				{
					m_pOwner->SetBrushState(pTmp);
				}
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
		CComBSTR bstr;
		for (CGradientColorPicker::CGradient::const_iterator i = m_cData.cGradient.begin(); i != m_cData.cGradient.end(); ++i)
		{
			wchar_t sz[64];
			swprintf(sz, L"%i,%g,%g,%g,%g,", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstr += sz;
		}
		wchar_t sz2[128];
		if (m_cData.bAutoCenter)
			swprintf(sz2, L"RELSHAPE,%g,%g,%g,%g", m_tRelPt1.fX, m_tRelPt1.fY, m_tRelPt2.fX, m_tRelPt2.fY);
		else
			swprintf(sz2, L"ABSOLUTE,%g,%g,%g,%g", m_tPoint1.fX, m_tPoint1.fY, m_tPoint2.fX, m_tPoint2.fY);
		bstr += sz2;
		*a_pbstrParams = bstr.Detach();
		return S_OK;
	}

private:
	void Invalidate1DMapCache()
	{
		m_bCacheInvalid = true;
	}
	TRasterImagePixel* Update1DMap(float a_fGamma, ULONG a_nSteps)
	{
		ObjectLock cLock(this);
		if (m_bCacheInvalid || m_cCacheMap.size() != a_nSteps || m_fCacheGamma != a_fGamma)
		{
			m_cCacheMap.resize(a_nSteps);
			CAutoVectorPtr<CButtonColorPicker::SColor> aVals(new CButtonColorPicker::SColor[a_nSteps]);
			CGradientColorPicker::RenderGradient(m_cData.cGradient, a_nSteps, 0, a_nSteps, aVals.m_p);
			for (ULONG i = 0; i < a_nSteps; ++i)
				*reinterpret_cast<DWORD*>(&(m_cCacheMap[i])) = aVals[i].ToBGRA(a_fGamma);
			m_fCacheGamma = a_fGamma;
			m_bCacheInvalid = false;
		}
		return &(m_cCacheMap[0]);
	}
	void UpdateAbsolutePositions()
	{
		if (!m_bAbsValid)
		{
			if (m_cData.bAutoCenter)
			{
				float const fX1 = m_fSizeX*(m_tRelPt1.fX*2.0f-1.0f);
				float const fY1 = m_fSizeY*(m_tRelPt1.fY*2.0f-1.0f);
				float const fX2 = m_fSizeX*(m_tRelPt2.fX*2.0f-1.0f);
				float const fY2 = m_fSizeY*(m_tRelPt2.fY*2.0f-1.0f);
				float const fC = cosf(m_fAngle);
				float const fS = sinf(m_fAngle);
				m_tPoint1.fX = m_tCenter.fX+fC*fX1-fS*fY1;
				m_tPoint1.fY = m_tCenter.fY+fS*fX1+fC*fY1;
				m_tPoint2.fX = m_tCenter.fX+fC*fX2-fS*fY2;
				m_tPoint2.fY = m_tCenter.fY+fS*fX2+fC*fY2;
			}
			else
			{
				ULONG nX = 0;
				ULONG nY = 0;
				if (m_pOwner) m_pOwner->Size(&nX, &nY);
				m_tPoint1.fX = nX*m_tRelPt1.fX;
				m_tPoint1.fY = nY*m_tRelPt1.fY;
				m_tPoint2.fX = nX*m_tRelPt2.fX;
				m_tPoint2.fY = nY*m_tRelPt2.fY;
			}
			m_bAbsValid = true;
		}
	}

private:
	CComPtr<IRasterImageBrushOwner> m_pOwner;
	TPixelCoords m_tPoint1;
	TPixelCoords m_tPoint2;
	TPixelCoords m_tRelPt1;
	TPixelCoords m_tRelPt2;
	bool m_bAbsValid;
	bool m_bRelValid;
	CFillStyleDataGradient m_cData;

	// cached auto-bounds values
	TPixelCoords m_tCenter;
	float m_fSizeX;
	float m_fSizeY;
	float m_fAngle;

	// cached gradient map
	std::vector<TRasterImagePixel> m_cCacheMap;
	float m_fCacheGamma;
	bool m_bCacheInvalid;
};

