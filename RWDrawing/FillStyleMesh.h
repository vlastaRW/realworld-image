
#pragma once

#include <math.h>

#include "FillStyleGradientDlg.h"

#include <boost/spirit.hpp>
using namespace boost::spirit;


class CFillStyleShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush,
	public IRasterImageEditToolScripting
{
public:
	CFillStyleShape()
	{
		m_tRelCenter.fX = m_tRelCenter.fY = 0.5f;
		m_fRelRadius = 0.5f;
		m_bAbsValid = false;
		m_tCenterAuto.fX = m_tCenterAuto.fY = m_tCenter.fX = m_tCenter.fY = 0.0f;
		m_fSizeXAuto = m_fSizeYAuto = m_fRadius = 0.0f;
	}

	BEGIN_COM_MAP(CFillStyleShape)
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
			m_fSizeXAuto = m_tCenterAuto.fX = nX*0.5f;
			m_fSizeYAuto = m_tCenterAuto.fY = nY*0.5f;
			if (m_cData.bAutoCenter)
				m_bAbsValid = false;
		}
		return S_OK;
	}
	STDMETHOD(SetShapeBounds)(TPixelCoords const* a_pCenter, float a_fSizeX, float a_fSizeY, float UNREF(a_fAngle))
	{
		m_tCenterAuto = *a_pCenter;
		m_fSizeXAuto = a_fSizeX;
		m_fSizeYAuto = a_fSizeY;
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
			m_bAbsValid = false;
			//m_tRelCenter.fX = m_tRelCenter.fY = 0.5f;
			//m_fRelRadius = 0.5f;
			if (!bPrevAutoCenter && m_cData.bAutoCenter)
				SetShapeBounds(&m_tCenterAuto, m_fSizeXAuto, m_fSizeYAuto, 0.0f);
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
		if (m_rcBounds.left != a_pBounds->left || m_rcBounds.top != a_pBounds->top || m_rcBounds.right != a_pBounds->right || m_rcBounds.bottom != a_pBounds->bottom)
		{
			m_rcBounds = *a_pBounds;
			m_pCache.Free();
		}
		return E_NOTIMPL;
	}
	STDMETHOD(IsSolid)(RECT const* a_pRect)
	{
		return S_FALSE; // TODO: consider rectangle
	}
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		ATLASSERT(m_pCache.m_p && LONG(a_nX) >= m_rcBounds.left && LONG(a_nY) >= m_rcBounds.top && LONG(a_nX+a_nSizeX) <= m_rcBounds.right && LONG(a_nY+a_nSizeY) <= m_rcBounds.bottom);

		TPixelChannel const* pS = m_pCache.m_p+(m_rcBounds.right-m_rcBounds.left)*(LONG(a_nY)-m_rcBounds.top)+LONG(a_nX)-m_rcBounds.left;

		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			CopyMemory(a_pBuffer, pS, a_nSizeX*sizeof*a_pBuffer);
			a_pBuffer += a_nStride-a_nSizeX;
			pS += (m_rcBounds.right-m_rcBounds.left)-a_nSizeX;
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
			if (!m_bAbsValid)
			{
				if (m_cData.bAutoCenter)
				{
					m_tCenter.fX = m_tCenterAuto.fX+m_fSizeXAuto*(m_tRelCenter.fX*2.0f-1.0f);
					m_tCenter.fY = m_tCenterAuto.fY+m_fSizeYAuto*(m_tRelCenter.fY*2.0f-1.0f);
					m_fRadius = (m_fSizeXAuto+m_fSizeYAuto)*m_fRelRadius;
				}
				else
				{
					ULONG nX = 0;
					ULONG nY = 0;
					if (m_pOwner) m_pOwner->Size(&nX, &nY);
					m_tCenter.fX = nX*m_tRelCenter.fX;
					m_tCenter.fY = nY*m_tRelCenter.fY;
					m_fRadius = (nX+nY)*0.5f*m_fRelRadius;
				}
				m_bAbsValid = true;
			}
			*a_pPos = m_tCenter;
			if (a_nIndex == 1)
				a_pPos->fX += m_fRadius;
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
			if (a_nIndex == 0)
			{
				m_tCenter = *a_pPos;
				if (m_cData.bAutoCenter)
				{
					m_tRelCenter.fX = ((m_tCenter.fX-m_tCenterAuto.fX)/m_fSizeXAuto + 1.0f)*0.5f;
					m_tRelCenter.fY = ((m_tCenter.fY-m_tCenterAuto.fY)/m_fSizeYAuto + 1.0f)*0.5f;
				}
				else
				{
					ULONG nX = 0;
					ULONG nY = 0;
					if (m_pOwner) m_pOwner->Size(&nX, &nY);
					if (nX && nY)
					{
						m_tRelCenter.fX = m_tCenter.fX/nX;
						m_tRelCenter.fY = m_tCenter.fY/nY;
					}
				}
			}
			else
			{
				m_fRadius = sqrtf((a_pPos->fX-m_tCenter.fX)*(a_pPos->fX-m_tCenter.fX) +
								  (a_pPos->fY-m_tCenter.fY)*(a_pPos->fY-m_tCenter.fY));
				if (m_cData.bAutoCenter)
				{
					m_fRelRadius = m_fRadius/(m_fSizeXAuto+m_fSizeYAuto);
				}
				else
				{
					ULONG nX = 0;
					ULONG nY = 0;
					if (m_pOwner) m_pOwner->Size(&nX, &nY);
					if (nX && nY)
					{
						m_fRelRadius = m_fRadius/((nX+nY)*0.5f);
					}
				}
			}
			if (m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				if (a_nIndex == 0)
					m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
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
		if (!m_bAbsValid)
		{
			if (m_cData.bAutoCenter)
			{
				m_tCenter.fX = m_tCenterAuto.fX+m_fSizeXAuto*(m_tRelCenter.fX*2.0f-1.0f);
				m_tCenter.fY = m_tCenterAuto.fY+m_fSizeYAuto*(m_tRelCenter.fY*2.0f-1.0f);
				m_fRadius = (m_fSizeXAuto+m_fSizeYAuto)*m_fRelRadius;
			}
			else
			{
				ULONG nX = 0;
				ULONG nY = 0;
				if (m_pOwner) m_pOwner->Size(&nX, &nY);
				m_tCenter.fX = nX*m_tRelCenter.fX;
				m_tCenter.fY = nY*m_tRelCenter.fY;
				m_fRadius = (nX+nY)*0.5f*m_fRelRadius;
			}
			m_bAbsValid = true;
		}
		a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY);
		a_pLines->LineTo(m_tCenter.fX+m_fRadius, m_tCenter.fY);
		return S_OK;
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
			CFillStyleDataGradient cData = m_cData;
			TPixelCoords tCenter = {0.5f, 0.5f};
			float fRadius = 0.5f;
			int nMode = 2;
			CGradientColorPicker::CGradient cGradient;
			std::pair<WORD, CButtonColorPicker::SColor> t;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(*((int_p[assign_a(t.first)]>>ch_p(L',')>>real_p[assign_a(t.second.fR)]>>ch_p(L',')>>real_p[assign_a(t.second.fG)]>>ch_p(L',')>>real_p[assign_a(t.second.fB)]>>ch_p(L',')>>real_p[assign_a(t.second.fA)]>>ch_p(L','))[insert_at_a(cGradient, t.first, t.second)]))>>
					((str_p(L"ABSOLUTE")[assign_a(nMode, 0)]|str_p(L"RELIMAGE")[assign_a(nMode, 1)]|str_p(L"RELSHAPE")[assign_a(nMode, 2)]))>>
					(!(ch_p(L',')>>real_p[assign_a(tCenter.fX)]>>ch_p(L',')>>real_p[assign_a(tCenter.fY)]>>ch_p(L',')>>real_p[assign_a(fRadius)]))
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
				m_tCenter = tCenter;
				m_fRadius = fRadius;
				m_bAbsValid = true;
			}
			else
			{
				m_tRelCenter = tCenter;
				m_fRelRadius = fRadius;
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
			swprintf(sz2, L"RELSHAPE,%g,%g,%g", m_tRelCenter.fX, m_tRelCenter.fY, m_fRelRadius);
		else
			swprintf(sz2, L"ABSOLUTE,%g,%g,%g", m_tCenter.fX, m_tCenter.fY, m_fRadius);
		bstr += sz2;
		*a_pbstrParams = bstr.Detach();
		return S_OK;
	}

private:
	void Invalidate1DMapCache()
	{
		m_bCacheInvalid = true;
	}
	void Update1DMap(TRasterImagePixel* a_pMap, float a_fGamma)
	{
		ObjectLock cLock(this);
		if (m_bCacheInvalid || m_cCacheMap.empty() || m_fCacheGamma != a_fGamma)
		{
			m_cCacheMap.resize(256);
			CButtonColorPicker::SColor aVals[256];
			CGradientColorPicker::RenderGradient(m_cData.cGradient, 256, 0, 256, aVals);
			for (ULONG i = 0; i < 256; ++i)
				*reinterpret_cast<DWORD*>(&(m_cCacheMap[i])) = aVals[i].ToBGRA(a_fGamma);
			m_fCacheGamma = a_fGamma;
			m_bCacheInvalid = false;
		}
		std::copy(m_cCacheMap.begin(), m_cCacheMap.end(), a_pMap);
	}

private:
	CComPtr<IRasterImageBrushOwner> m_pOwner;

	RECT m_rcBounds;
	CAutoVectorPtr<TPixelChannel> m_pCache;

	TPixelCoords m_tCenter;
	float m_fRadius;
	TPixelCoords m_tRelCenter;
	float m_fRelRadius;
	bool m_bAbsValid;
	TPixelCoords m_tCenterAuto;
	float m_fSizeXAuto;
	float m_fSizeYAuto;
	CFillStyleDataGradient m_cData;

	// cached gradient map
	std::vector<TRasterImagePixel> m_cCacheMap;
	float m_fCacheGamma;
	bool m_bCacheSwapColors;
	bool m_bCacheCustomGradient;
	bool m_bCacheInvalid;
};

