
#pragma once

class CConstantRasterImageBrush :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush
{
public:
	CConstantRasterImageBrush()
	{
		m_tColor.fR = m_tColor.fG = m_tColor.fB = 0.0f; m_tColor.fA = 1.0f;
	}
	void Init(TColor const& a_tColor)
	{
		m_tColor = a_tColor;
	}

	BEGIN_COM_MAP(CConstantRasterImageBrush)
		COM_INTERFACE_ENTRY(IRasterImageBrush)
	END_COM_MAP()

	// IRasterImageBrush methods
public:
	STDMETHOD(Init)(IRasterImageBrushOwner* UNREF(a_pOwner)) { return S_OK; }
	STDMETHOD(SetShapeBounds)(TPixelCoords const* UNREF(a_pCenter), float UNREF(a_fSizeX), float UNREF(a_fSizeY), float UNREF(a_fAngle)) { return E_NOTIMPL; }
	STDMETHOD(SetState)(ISharedState* UNREF(a_pState)) { return S_OK; }
	STDMETHOD(GetState)(ISharedState** UNREF(a_ppState)) { return E_NOTIMPL; }
	STDMETHOD(NeedsPrepareShape)() { return S_FALSE; }
	STDMETHOD(PrepareShape)(RECT const* UNREF(a_pBounds), ULONG UNREF(a_nPaths), TRWPolygon const* UNREF(a_pPaths)) { return E_NOTIMPL; }
	STDMETHOD(IsSolid)(RECT const* UNREF(a_pRect)) { return S_OK; }
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		TRasterImagePixel const t = TColorToTRasterImagePixel(m_tColor, a_fGamma);
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
	STDMETHOD(AdjustCoordinates)(TPixelCoords* UNREF(a_pPos), ULONG UNREF(a_nControlPointIndex)) { return S_OK; }
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
	STDMETHOD(GetControlPoint)(ULONG UNREF(a_nIndex), TPixelCoords* UNREF(a_pPos), ULONG* UNREF(a_pClass)) { return E_RW_INDEXOUTOFRANGE; }
	STDMETHOD(SetControlPoint)(ULONG UNREF(a_nIndex), TPixelCoords const* UNREF(a_pPos), boolean UNREF(a_bReleased), float UNREF(a_fPointSize)) { return E_RW_INDEXOUTOFRANGE; }
	STDMETHOD(GetControlPointDesc)(ULONG UNREF(a_nIndex), ILocalizedString** UNREF(a_ppDescription)) { return E_NOTIMPL; }
	STDMETHOD(GetControlLines)(IEditToolControlLines* UNREF(a_pLines)) { return S_FALSE; }

	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix) { return S_FALSE; }

private:
	TColor m_tColor;
};