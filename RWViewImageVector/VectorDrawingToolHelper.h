
#pragma once


template<class TOwner>
class CCallbackHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditWindow
{
public:
	void SetOwner(TOwner* a_pWindow, ULONG a_nShapeID)
	{
		m_pWindow = a_pWindow;
		m_nShapeID = a_nShapeID;
	}

BEGIN_COM_MAP(CCallbackHelper)
	COM_INTERFACE_ENTRY(IRasterImageEditWindow)
END_COM_MAP()

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		if (m_pWindow) return m_pWindow->Size(m_nShapeID, a_pSizeX, a_pSizeY);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		if (m_pWindow) return m_pWindow->GetDefaultColor(a_pDefault);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		if (m_pWindow) return m_pWindow->GetImageTile(m_nShapeID, a_nX, a_nY, a_nSizeX, a_fGamma, a_nSizeY, a_nStride, a_eIntent, a_pBuffer);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_pBoundingRectangle)
		{
			a_pBoundingRectangle->left = a_pBoundingRectangle->top = LONG_MIN;
			a_pBoundingRectangle->right = LONG_MAX;
			a_pBoundingRectangle->bottom = LONG_MAX;
		}
		if (a_bEntireRectangle)
			*a_bEntireRectangle = TRUE;
		return S_OK;
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlPointsChanged(m_nShapeID);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		if (m_pWindow) return m_pWindow->ControlPointChanged(m_nShapeID, a_nIndex);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlLinesChanged(m_nShapeID);
		return E_UNEXPECTED;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_pWindow) return m_pWindow->RectangleChanged(m_nShapeID, a_pChanged);
		return E_UNEXPECTED;
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetToolState(m_nShapeID, a_pState);
		return E_UNEXPECTED;
	}
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetBrushState(m_nShapeID, a_bstrStyleID, a_pState);
		return E_UNEXPECTED;
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		if (m_pWindow) return m_pWindow->Handle(a_phWnd);
		return E_NOTIMPL;
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		if (m_pWindow) return m_pWindow->Document(a_ppDocument);
		return E_NOTIMPL;
	}

private:
	TOwner* m_pWindow;
	ULONG m_nShapeID;
};

