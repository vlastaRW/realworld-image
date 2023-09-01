
#pragma once


template<class TOwner>
class CBrushCallbackHelper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrushOwner
{
public:
	void SetOwner(TOwner* a_pOwner)
	{
		m_pWindow = a_pOwner;
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
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_pWindow) return m_pWindow->RectangleChanged(a_pChanged);
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		if (m_pWindow) return m_pWindow->ControlLinesChanged();
		return E_UNEXPECTED;
	}
	STDMETHOD(SetBrushState)(ISharedState* a_pState)
	{
		if (m_pWindow) return m_pWindow->SetBrushState(a_pState);
		return E_UNEXPECTED;
	}

private:
	TOwner* m_pWindow;
};

template<class T, class TControlHandler, class TAdjustCoordsHandler>
class CEditToolWithBrush
{
public:
	typedef CEditToolWithBrush<T, TControlHandler, TAdjustCoordsHandler> thisClass;
	CEditToolWithBrush() : m_pHelper(NULL), m_bBrushEnabled(false), m_nCPBase(0)
	{
	}
	~CEditToolWithBrush()
	{
		if (m_pHelper)
		{
			m_pHelper->SetOwner(NULL);
			m_pHelper->Release();
		}
	}

	IRasterImageBrushOwner* M_BrushOwner()
	{
		if (m_pHelper)
			return m_pHelper;
		CComObject<CBrushCallbackHelper<thisClass> >::CreateInstance(&m_pHelper);
		m_pHelper->AddRef();
		m_pHelper->SetOwner(this);
		return m_pHelper;
	}
	IRasterImageBrush* M_Brush() const
	{
		return m_pBrush;
	}
	void EnableBrush(bool a_bEnable)
	{
		if (m_bBrushEnabled != a_bEnable)
		{
			m_bBrushEnabled = a_bEnable;
		}
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}
	template<typename TVertexSource>
	void PrepareBrush(TVertexSource& vs)
	{
		if (m_pBrush == NULL || M_Brush()->NeedsPrepareShape() != S_OK)
			return;

		vs.rewind(0);
		double maxx, maxy;
		if (agg::is_end_poly(vs.vertex(&maxx, &maxy)))
			return;
		double minx = maxx, miny = maxy;
		double x, y;
		while (agg::is_end_poly(vs.vertex(&x, &y)))
		{
			if (x < minx) minx = x;
			else if (x > maxx) maxx = x;
			if (y < miny) miny = y;
			else if (y > maxy) maxy = y;
		}
		if (minx < maxx && miny < maxy)
		{
			RECT const rc = {floorf(minx), floorf(miny), ceilf(maxx), ceilf(maxy)};
			M_Brush()->PrepareShape(&rc, 0, NULL);
		}
	}

	// to be actually implemented by T
	void ToolSetBrush()
	{
	}

	void TransformBrush(TMatrix3x3f const* a_pMatrix)
	{
		if (m_pBrush) m_pBrush->Transform(a_pMatrix);
	}

	HRESULT _SetBrush(IRasterImageBrush* a_pBrush)
	{
		m_pBrush = a_pBrush;
		if (m_pBrush)
		{
			m_pBrush->Init(M_BrushOwner());
		}
		static_cast<T*>(this)->BrushRectangleChanged(NULL);
		if (m_bBrushEnabled)
		{
			static_cast<T*>(this)->M_Window()->ControlPointsChanged();
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		}
		static_cast<T*>(this)->ToolSetBrush();
		return S_OK;
	}

	HRESULT _AdjustCoordinates(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		if (a_pControlPointIndex && *a_pControlPointIndex >= m_nCPBase)
		{
			if (m_pBrush && m_bBrushEnabled)
				return m_pBrush->AdjustCoordinates(a_pPos, *a_pControlPointIndex-m_nCPBase);
			return E_RW_INDEXOUTOFRANGE;
		}
		return static_cast<TAdjustCoordsHandler*>(static_cast<T*>(this))->_AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = 0;
		if (m_pBrush && m_bBrushEnabled)
			m_pBrush->GetControlPointCount(a_pCount);
		static_cast<TControlHandler*>(static_cast<T*>(this))->_GetControlPointCount(&m_nCPBase);
		*a_pCount += m_nCPBase;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (a_nIndex >= m_nCPBase)
		{
			if (m_pBrush && m_bBrushEnabled)
			{
				HRESULT hRes = m_pBrush->GetControlPoint(a_nIndex-m_nCPBase, a_pPos, a_pClass);
				*a_pClass += 0x100;
				return hRes;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		return static_cast<TControlHandler*>(static_cast<T*>(this))->_GetControlPoint(a_nIndex, a_pPos, a_pClass);
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (a_nIndex >= m_nCPBase)
		{
			if (m_pBrush && m_bBrushEnabled)
				return m_pBrush->SetControlPoint(a_nIndex-m_nCPBase, a_pPos, a_bFinished, a_fPointSize);
			return E_RW_INDEXOUTOFRANGE;
		}
		return static_cast<TControlHandler*>(static_cast<T*>(this))->_SetControlPoint(a_nIndex, a_pPos, a_bFinished, a_fPointSize);
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		try { return static_cast<TControlHandler*>(static_cast<T*>(this))->_GetControlPointDesc(a_nIndex, a_ppDescription); } catch (...) { return E_UNEXPECTED; }
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		try
		{
			if (m_pBrush && m_bBrushEnabled && (a_nLineTypes&ECLTHelp))
				m_pBrush->GetControlLines(a_pLines);
			return static_cast<TControlHandler*>(static_cast<T*>(this))->_GetControlLines(a_pLines, a_nLineTypes);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		return static_cast<T*>(this)->M_Window()->RectangleChanged(a_pChanged);
	}

	// IRasterImageBrushOwner methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return static_cast<T*>(this)->M_Window()->Size(a_pSizeX, a_pSizeY);
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return m_bBrushEnabled ? static_cast<T*>(this)->M_Window()->ControlPointsChanged() : S_FALSE;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return m_bBrushEnabled ? static_cast<T*>(this)->M_Window()->ControlPointChanged(a_nIndex+m_nCPBase) : S_FALSE;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		return static_cast<T*>(this)->BrushRectangleChanged(a_pChanged);
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return m_bBrushEnabled ? static_cast<T*>(this)->M_Window()->ControlLinesChanged() : S_FALSE;
	}
	STDMETHOD(SetBrushState)(ISharedState* a_pState)
	{
		return m_bBrushEnabled ? static_cast<T*>(this)->M_Window()->SetBrushState(NULL, a_pState) : S_FALSE;
	}

private:
	CComObject<CBrushCallbackHelper<thisClass> >* m_pHelper;
	CComPtr<IRasterImageBrush> m_pBrush;
	ULONG m_nCPBase;
	bool m_bBrushEnabled;
};

template<class T>
class CEditToolWithSolidBrush
{
public:
	typedef CEditToolWithSolidBrush<T> thisClass;
	CEditToolWithSolidBrush() : m_pHelper(NULL)
	{
	}
	~CEditToolWithSolidBrush()
	{
		if (m_pHelper)
		{
			m_pHelper->SetOwner(NULL);
			m_pHelper->Release();
		}
	}

	IRasterImageBrushOwner* M_BrushOwner()
	{
		if (m_pHelper)
			return m_pHelper;
		CComObject<CBrushCallbackHelper<thisClass> >::CreateInstance(&m_pHelper);
		m_pHelper->AddRef();
		m_pHelper->SetOwner(this);
		return m_pHelper;
	}
	TRasterImagePixel M_Color(float a_fGamma) const
	{
		TRasterImagePixel tClr = {0, 0, 0, 0};
		m_pBrush->GetBrushTile(0, 0, 1, 1, a_fGamma, 1, &tClr);
		return tClr;
	}

	// to be actually implemented by T
	void ToolSetBrush()
	{
	}

	HRESULT _SetBrush(IRasterImageBrush* a_pBrush)
	{
		m_pBrush = a_pBrush;
		if (m_pBrush)
		{
			m_pBrush->Init(M_BrushOwner());
		}
		static_cast<T*>(this)->BrushRectangleChanged(NULL);
		static_cast<T*>(this)->ToolSetBrush();
		return S_OK;
	}

	HRESULT BrushRectangleChanged(RECT const* a_pChanged)
	{
		return static_cast<T*>(this)->M_Window()->RectangleChanged(a_pChanged);
	}

	// IRasterImageBrushOwner methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return static_cast<T*>(this)->M_Window()->Size(a_pSizeX, a_pSizeY);
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return S_FALSE;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return S_FALSE;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		return static_cast<T*>(this)->BrushRectangleChanged(a_pChanged);
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return S_FALSE;
	}
	STDMETHOD(SetBrushState)(ISharedState* a_pState)
	{
		return static_cast<T*>(this)->M_Window()->SetBrushState(NULL, a_pState);
	}

private:
	CComObject<CBrushCallbackHelper<thisClass> >* m_pHelper;
	CComPtr<IRasterImageBrush> m_pBrush;
};

