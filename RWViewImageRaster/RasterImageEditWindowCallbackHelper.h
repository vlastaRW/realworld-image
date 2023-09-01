
#pragma once


template<class TOwner>
class CCallbackHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditWindow
{
public:
	void SetOwner(TOwner* a_pWindow)
	{
		m_pWindow = a_pWindow;
	}

BEGIN_COM_MAP(CCallbackHelper)
	COM_INTERFACE_ENTRY(IRasterImageEditWindow)
END_COM_MAP()

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		if (m_pWindow) return m_pWindow->Size(a_pSizeX, a_pSizeY);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		if (m_pWindow) return m_pWindow->GetDefaultColor(a_pDefault);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		if (m_pWindow) return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_eIntent, a_pBuffer);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (m_pWindow) return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
		return E_UNEXPECTED;
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		if (m_pWindow) return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlPointsChanged();
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		if (m_pWindow) return m_pWindow->ControlPointChanged(a_nIndex);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlLinesChanged();
		return E_UNEXPECTED;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_pWindow) return m_pWindow->RectangleChanged(a_pChanged);
		return E_UNEXPECTED;
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		if (m_pWindow) return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
		return E_UNEXPECTED;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetState(a_pState);
		return E_UNEXPECTED;
	}
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetBrushState(a_bstrStyleID, a_pState);
		return E_UNEXPECTED;
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		if (m_pWindow) return m_pWindow->Handle(a_phWnd);
		return E_UNEXPECTED;
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		if (m_pWindow) return m_pWindow->Document(a_ppDocument);
		return E_UNEXPECTED;
	}

private:
	TOwner* m_pWindow;
};

template<class TOwner>
class CBrushCallbackHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrushOwner
{
public:
	void SetOwner(TOwner* a_pWindow)
	{
		m_pWindow = a_pWindow;
	}

BEGIN_COM_MAP(CBrushCallbackHelper)
	COM_INTERFACE_ENTRY(IRasterImageBrushOwner)
END_COM_MAP()

	// IRasterImageBrushOwner methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		if (m_pWindow) return m_pWindow->Size(a_pSizeX, a_pSizeY);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlPointsChanged();
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		if (m_pWindow) return m_pWindow->ControlPointChanged(a_nIndex);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlLinesChanged();
		return E_UNEXPECTED;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_pWindow) return m_pWindow->RectangleChanged(a_pChanged);
		return E_UNEXPECTED;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetState(a_pState);
		return E_UNEXPECTED;
	}

private:
	TOwner* m_pWindow;
};

