
#pragma once

TRasterImagePixel TColorToTRasterImagePixel(TColor const& a_tColor, float a_fGamma);


struct CFillStyleDataSolid
{
	MIDL_INTERFACE("F2F9400F-BA55-4BE2-A6A4-7359C100D0C3")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CFillStyleDataSolid const*, InternalData)() = 0;
	};

	CFillStyleDataSolid()
	{
		tColor.fR = tColor.fG = tColor.fB = 0.5f;
		tColor.fA = 1.0f;
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr)
		{
			wchar_t const* pNum = a_bstr;
			if (wcsncmp(a_bstr, L"PRIMARY", 7) == 0)
				pNum = a_bstr[7] == L',' ? a_bstr+8 : a_bstr+7;
			else if (wcsncmp(a_bstr, L"SECONDARY", 9) == 0)
				pNum = a_bstr[9] == L',' ? a_bstr+10 : a_bstr+9;
			else if (wcsncmp(a_bstr, L"CUSTOM", 6) == 0)
				pNum = a_bstr[6] == L',' ? a_bstr+7 : a_bstr+6;
			swscanf(pNum, L"%f,%f,%f,%f", &tColor.fR, &tColor.fG, &tColor.fB, &tColor.fA);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[64];
		swprintf(szTmp, L"%g,%g,%g,%g", tColor.fR, tColor.fG, tColor.fB, tColor.fA);
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}
	bool operator!=(CFillStyleDataSolid const& a_cData) const
	{
		return
			tColor.fA != a_cData.tColor.fA || tColor.fR != a_cData.tColor.fR ||
			tColor.fG != a_cData.tColor.fG || tColor.fB != a_cData.tColor.fB;
	}

	TColor tColor;
};

#include "FillStyleSolidDlg.h"


class CFillStyleSolid :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush,
	public IRasterImageEditToolScripting
{
public:
	CFillStyleSolid()
	{
		m_tColor1.fR = m_tColor1.fG = m_tColor1.fB = 0.0f; m_tColor1.fA = 1.0f;
		m_tColor2.fR = m_tColor2.fG = m_tColor2.fB = m_tColor2.fA = 0.0f;
	}

	BEGIN_COM_MAP(CFillStyleSolid)
		COM_INTERFACE_ENTRY(IRasterImageBrush)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageBrush methods
public:
	STDMETHOD(Init)(IRasterImageBrushOwner* a_pOwner)
	{
		m_pOwner = a_pOwner;
		return S_OK;
	}
	STDMETHOD(SetShapeBounds)(TPixelCoords const* UNREF(a_pCenter), float UNREF(a_fSizeX), float UNREF(a_fSizeY), float UNREF(a_fAngle))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CFillStyleDataSolid::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			if (m_pOwner)
				m_pOwner->RectangleChanged(NULL);
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
	STDMETHOD(IsSolid)(RECT const* UNREF(a_pRect))
	{
		return S_OK;
	}
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		TRasterImagePixel const t = TColorToTRasterImagePixel(m_cData.tColor, a_fGamma);
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				*(a_pBuffer++) = t;
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
			CFillStyleDataSolid cData = m_cData;
			HRESULT hRes = m_cData.FromString(a_bstrParams);
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
	CComPtr<IRasterImageBrushOwner> m_pOwner;
	TColor m_tColor1;
	TColor m_tColor2;
	CFillStyleDataSolid m_cData;
};

