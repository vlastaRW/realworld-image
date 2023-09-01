
#pragma once

__declspec(selectany) RECT RECT_EMPTY = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};

template<class TBase = IRasterImageEditTool>
class CEditToolBase : public TBase
{
public:
	IRasterImageEditWindow* M_Window() const { return m_pWindow; }

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		m_pWindow = a_pWindow;
		Reset();
		return S_OK;
	}
	STDMETHOD(Reset)()
	{
		return E_NOTIMPL;
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		return S_FALSE;
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float m_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, m_fGamma, a_nStride, EITIBackground, a_pBuffer);
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
	}

	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return S_FALSE;
	}
	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_tPos, HCURSOR* a_phCursor)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetOutline)(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState UNREF(a_eKeysState), TPixelCoords* UNREF(a_pPos), TPixelCoords const* UNREF(a_pPointerSize), ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		return S_FALSE;
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* UNREF(a_pLines), ULONG UNREF(a_nLineTypes))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

private:
	CComPtr<IRasterImageEditWindow> m_pWindow;
};

class CRasterImageEditToolBase
{
public:
	IRasterImageEditWindow* M_Window() const { return m_pWindow; }
	void InitWindow(IRasterImageEditWindow* a_pWindow) { m_pWindow = a_pWindow; }

	HRESULT _Reset()
	{
		return E_NOTIMPL;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		return S_FALSE;
	}
	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		return m_pWindow ? m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer) : E_UNEXPECTED;
	}
	HRESULT _GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pWindow ? m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle) : E_UNEXPECTED;
	}
	HRESULT _GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pWindow ? m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer) : E_UNEXPECTED;
	}

	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		return S_FALSE;
	}
	HRESULT _GetCursor(EControlKeysState a_eKeysState, TPixelCoords const* a_tPos, HCURSOR* a_phCursor)
	{
		return E_NOTIMPL;
	}

	HRESULT _SetState(ISharedState* a_pState)
	{
		return E_NOTIMPL;
	}
	HRESULT _SetOutline(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		return E_NOTIMPL;
	}
	HRESULT _SetBrush(IRasterImageBrush* a_pBrush)
	{
		return E_NOTIMPL;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		return E_NOTIMPL;
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* UNREF(a_pPos), TPixelCoords const* UNREF(a_pPointerSize), ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		return S_FALSE;
	}
	HRESULT _ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		return E_NOTIMPL;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return E_NOTIMPL;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlLines(IEditToolControlLines* UNREF(a_pLines), ULONG UNREF(a_nLineTypes))
	{
		return E_NOTIMPL;
	}

private:
	CComPtr<IRasterImageEditWindow> m_pWindow;
};

template<
	class T,
	class TResetHandler = CRasterImageEditToolBase,
	class TDirtyHandler = CRasterImageEditToolBase,
	class TImageTileHandler = CRasterImageEditToolBase,
	class TSelectionTileHandler = CRasterImageEditToolBase,
	class TOutlineHandler = CRasterImageEditToolBase,
	class TBrushHandler = CRasterImageEditToolBase,
	class TGlobalsHandler = CRasterImageEditToolBase,
	class TAdjustCoordsHandler = CRasterImageEditToolBase,
	class TGetCursorHandler = CRasterImageEditToolBase,
	class TProcessInputHandler = CRasterImageEditToolBase,
	class TPreTranslateMessageHandler = CRasterImageEditToolBase,
	class TControlPointsHandler = CRasterImageEditToolBase,
	class TControlLinesHandler = CRasterImageEditToolBase
>
class CRasterImageEditToolImpl :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CRasterImageEditToolBase,
	public IRasterImageEditTool
{
public:
	typedef CRasterImageEditToolImpl<T, TResetHandler, TDirtyHandler, TImageTileHandler, TSelectionTileHandler, TOutlineHandler, TBrushHandler, TGlobalsHandler, TAdjustCoordsHandler, TGetCursorHandler, TProcessInputHandler, TPreTranslateMessageHandler, TControlPointsHandler, TControlLinesHandler> thisClass;

	BEGIN_COM_MAP(thisClass)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
	END_COM_MAP()

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		if (a_pWindow == NULL)
			return E_RW_INVALIDPARAM;
		InitWindow(a_pWindow);
		return Reset();
	}
	STDMETHOD(Reset)()
	{
		try { return static_cast<TResetHandler*>(static_cast<T*>(this))->_Reset(); } catch (...) { return E_UNEXPECTED; }
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		try { return static_cast<TDirtyHandler*>(static_cast<T*>(this))->_IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try { return static_cast<TImageTileHandler*>(static_cast<T*>(this))->_GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_pBuffer); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		try { return static_cast<TSelectionTileHandler*>(static_cast<T*>(this))->_GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		try { return static_cast<TSelectionTileHandler*>(static_cast<T*>(this))->_GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer); } catch (...) { return E_UNEXPECTED; }
	}

	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		try { return static_cast<TPreTranslateMessageHandler*>(static_cast<T*>(this))->_PreTranslateMessage(a_pMsg); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		try { return static_cast<TGetCursorHandler*>(static_cast<T*>(this))->_GetCursor(a_eKeysState, a_pPos, a_phCursor); } catch (...) { return E_UNEXPECTED; }
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetOutline)(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		try { return static_cast<TOutlineHandler*>(static_cast<T*>(this))->_SetOutline(a_bEnabled != 0, a_fWidth, a_fPos, a_eJoins, a_pColor); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		try { return static_cast<TBrushHandler*>(static_cast<T*>(this))->_SetBrush(a_pBrush); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		try { return static_cast<TGlobalsHandler*>(static_cast<T*>(this))->_SetGlobals(a_eBlendingMode, a_eRasterizationMode, a_eCoordinatesMode); } catch (...) { return E_UNEXPECTED; }
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		try { return static_cast<TAdjustCoordsHandler*>(static_cast<T*>(this))->_AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		try { return static_cast<TProcessInputHandler*>(static_cast<T*>(this))->_ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime); } catch (...) { return E_UNEXPECTED; }
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		try { return static_cast<TControlPointsHandler*>(static_cast<T*>(this))->_GetControlPointCount(a_pCount); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		try { return static_cast<TControlPointsHandler*>(static_cast<T*>(this))->_GetControlPoint(a_nIndex, a_pPos, a_pClass); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		try { return static_cast<TControlPointsHandler*>(static_cast<T*>(this))->_SetControlPoint(a_nIndex, a_pPos, a_bFinished, a_fPointSize); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		try { return static_cast<TControlPointsHandler*>(static_cast<T*>(this))->_GetControlPointDesc(a_nIndex, a_ppDescription); } catch (...) { return E_UNEXPECTED; }
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		try { return static_cast<TControlLinesHandler*>(static_cast<T*>(this))->_GetControlLines(a_pLines, a_nLineTypes); } catch (...) { return E_UNEXPECTED; }
	}
};


template<class T, class TUpDownMoveHandler = T>
class CEditToolMouseInput
{
public:
	CEditToolMouseInput() : m_bDragging(false), m_fPointSize(1.0f)
	{
	}

	bool M_Dragging() const { return m_bDragging; }
	float M_PointSize() const { return sqrtf(m_fPointSize); }
	void ResetDragging() { m_bDragging = false; }

	HRESULT _ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		if (a_pPos == NULL)
			return static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->OnMouseLeave();

		if (a_pPointerSize)
			m_fPointSize = a_pPointerSize->fX*a_pPointerSize->fY;

		if (a_pMaxIdleTime) *a_pMaxIdleTime = static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->MaxIdleTime();
		HRESULT hRes = S_OK;
		if (m_bDragging)
		{
			if (a_fNormalPressure < 0.333f)
			{
				hRes = static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->OnMouseUp(a_eKeysState, a_pPos);
				m_bDragging = false;
			}
			else
			{
				hRes = static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->OnMouseMove(a_eKeysState, a_pPos);
			}
		}
		else
		{
			if (a_fNormalPressure > 0.667f)
			{
				m_bDragging = true;
				hRes = static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->OnMouseDown(a_eKeysState, a_pPos);
				m_bDragging = true; // just in case it was reseted during application of previous shape
			}
			else
			{
				hRes = static_cast<TUpDownMoveHandler*>(static_cast<T*>(this))->OnMouseMove(a_eKeysState, a_pPos);
			}
		}
		return hRes;
	}

	// to be overridden...
	DWORD MaxIdleTime()
	{
		return 0;
	}
	HRESULT OnMouseDown(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		return E_NOTIMPL;
	}
	HRESULT OnMouseUp(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		return E_NOTIMPL;
	}
	HRESULT OnMouseMove(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		return E_NOTIMPL;
	}
	HRESULT OnMouseLeave()
	{
		return E_NOTIMPL;
	}

private:
	bool m_bDragging;
	TPixelCoords m_tLastPos;
	float m_fPointSize;
};

template<class T>
class CEditToolOutline
{
public:
	CEditToolOutline() : m_fWidth(1.0f), m_fPos(0.0f), m_eJoins(EOJTRound), m_bEnabled(false)
	{
		m_tColor.fR = m_tColor.fG = m_tColor.fB = 0.0f;
		m_tColor.fA = 1.0f;
	}

	TColor const& M_OutlineColor() const { return m_tColor; }
	float M_OutlineWidth() const { return m_bEnabled ? m_fWidth : 0.0f; }
	float M_OutlineIn() const { return m_bEnabled ? m_fWidth*(m_fPos*0.5f-0.5f) : 0.0f; }
	float M_OutlineOut() const { return m_bEnabled ? m_fWidth*(m_fPos*0.5f+0.5f) : 0.0f; }
	EOutlineJoinType M_OutlineJoins() const { return m_eJoins; }

	// to be overriden
	void OutlineChanged(bool a_bWidth, bool a_bColor)
	{
	}

	HRESULT _SetOutline(bool a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		bool bOutline = false;
		if (a_bEnabled != m_bEnabled)
		{
			m_bEnabled = a_bEnabled;
			bOutline = true;
		}
		if (a_fWidth > 0.0f && m_fWidth != a_fWidth)
		{
			m_fWidth = a_fWidth;
			if (m_bEnabled)
				bOutline = true;
		}
		bool bColor = false;
		if (a_pColor && (m_tColor.fR != a_pColor->fR || m_tColor.fG != a_pColor->fG ||
			m_tColor.fB != a_pColor->fB || m_tColor.fA != a_pColor->fA))
		{
			m_tColor = *a_pColor;
			if (m_bEnabled)
				bColor = true;
		}
		if (m_fPos != a_fPos)
		{
			m_fPos = a_fPos;
			if (m_bEnabled)
				bOutline = true;
		}
		if (m_eJoins != a_eJoins)
		{
			m_eJoins = a_eJoins;
			if (m_bEnabled)
				bOutline = true;
		}

		if (bOutline || bColor)
			static_cast<T*>(this)->OutlineChanged(bOutline, bColor);
		return S_OK;
	}

private:
	bool m_bEnabled;
	float m_fWidth;
	float m_fPos;
	EOutlineJoinType m_eJoins;
	TColor m_tColor;
};
TRasterImagePixel TColorToTRasterImagePixel(TColor const& a_tColor, float a_fGamma);

template<UINT t_uIDC = UINT(IDC_ARROW)>
class CEditToolCustomCursor
{
public:
	HRESULT _GetCursor(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), HCURSOR* a_phCursor)
	{
		static HCURSOR hCustom = ::LoadCursor(t_uIDC < UINT(IDC_ARROW) || t_uIDC > UINT(IDC_HELP) ? _pModule->get_m_hInst() : NULL, MAKEINTRESOURCE(t_uIDC));
		*a_phCursor = hCustom;
		return S_OK;
	}
};

typedef HICON (*pfnGetToolIcon)(ULONG size);

template<pfnGetToolIcon t_fnGetIcon = NULL>
class CEditToolCustomPrecisionCursor
{
public:
	CEditToolCustomPrecisionCursor() : m_hCursor(NULL) {}
	~CEditToolCustomPrecisionCursor() { if (m_hCursor) DestroyCursor(m_hCursor); }

	HRESULT _GetCursor(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), HCURSOR* a_phCursor)
	{
		if (m_hCursor == NULL)
		{
			ULONG nSX = GetSystemMetrics(SM_CXCURSOR);
			ULONG nSY = GetSystemMetrics(SM_CYCURSOR);
			HKEY hKey = NULL;
			if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, _T("Control Panel\\Cursors"), &hKey))
			{
				DWORD dwValue = 0;
				DWORD dwSize = sizeof dwValue;
				RegQueryValueEx(hKey, _T("CursorBaseSize"), NULL, NULL, reinterpret_cast<BYTE*>(&dwValue), &dwSize);
				if (dwValue > nSX && dwValue <= 256) nSX = dwValue;
				if (dwValue > nSY && dwValue <= 256) nSY = dwValue;
				RegCloseKey(hKey);
			}
			ULONG nHX = 9.0f*nSX/32-0.5f;//((nSX+3)>>2);
			ULONG nHY = 9.0f*nSY/32-0.5f;//((nSY+3)>>2);
			CIconRendererReceiver cRenderer(nSX, nSY);
			IRCanvas canvas = {0, 0, nSX, nSY, 0, 0, NULL, NULL};
			IRFill white(0xffffffff);
			IRFill black(0xff000000);
			float r10 = min(nSX, nSY)/32.0f;
			float r15 = r10*1.5f;
			float r05 = r10*0.5f;
			float len = r10*8.5f;
			float cx = nHX+0.5f;
			float cy = nHY+0.5f;
			IRPolyPoint const topo[] = { {cx, cy-r10}, {cx-r15, cy-r10-r15}, {cx-r15, cy-len+r05}, {cx-r10, cy-len}, {cx+r10, cy-len}, {cx+r15, cy-len+r05}, {cx+r15, cy-r10-r15} };
			IRPolyPoint const topi[] = { {cx, cy-r10*2}, {cx-r10, cy-r10*3}, {cx, cy-r10*4}, {cx-r10, cy-r10*5}, {cx, cy-r10*6}, {cx-r10, cy-r10*7}, {cx, cy-r10*8}, {cx+r10, cy-r10*7}, {cx, cy-r10*6}, {cx+r10, cy-r10*5}, {cx, cy-r10*4}, {cx+r10, cy-r10*3} };
			cRenderer(&canvas, itemsof(topo), topo, &black);
			cRenderer(&canvas, itemsof(topi), topi, &white);
			IRPolyPoint const boto[] = { {cx, cy+r10}, {cx+r15, cy+r10+r15}, {cx+r15, cy+len-r05}, {cx+r10, cy+len}, {cx-r10, cy+len}, {cx-r15, cy+len-r05}, {cx-r15, cy+r10+r15} };
			IRPolyPoint const boti[] = { {cx, cy+r10*2}, {cx+r10, cy+r10*3}, {cx, cy+r10*4}, {cx+r10, cy+r10*5}, {cx, cy+r10*6}, {cx+r10, cy+r10*7}, {cx, cy+r10*8}, {cx-r10, cy+r10*7}, {cx, cy+r10*6}, {cx-r10, cy+r10*5}, {cx, cy+r10*4}, {cx-r10, cy+r10*3} };
			cRenderer(&canvas, itemsof(boto), boto, &black);
			cRenderer(&canvas, itemsof(boti), boti, &white);
			IRPolyPoint const lfto[] = { {cx-r10, cy}, {cx-r10-r15, cy-r15}, {cx-len+r05, cy-r15}, {cx-len, cy-r10}, {cx-len, cy+r10}, {cx-len+r05, cy+r15}, {cx-r10-r15, cy+r15} };
			IRPolyPoint const lfti[] = { {cx-r10*2, cy}, {cx-r10*3, cy-r10}, {cx-r10*4, cy}, {cx-r10*5, cy-r10}, {cx-r10*6, cy}, {cx-r10*7, cy-r10}, {cx-r10*8, cy}, {cx-r10*7, cy+r10}, {cx-r10*6, cy}, {cx-r10*5, cy+r10}, {cx-r10*4, cy}, {cx-r10*3, cy+r10} };
			cRenderer(&canvas, itemsof(lfto), lfto, &black);
			cRenderer(&canvas, itemsof(lfti), lfti, &white);
			IRPolyPoint const rgto[] = { {cx+r10, cy}, {cx+r10+r15, cy-r15}, {cx+len-r05, cy-r15}, {cx+len, cy-r10}, {cx+len, cy+r10}, {cx+len-r05, cy+r15}, {cx+r10+r15, cy+r15} };
			IRPolyPoint const rgti[] = { {cx+r10*2, cy}, {cx+r10*3, cy-r10}, {cx+r10*4, cy}, {cx+r10*5, cy-r10}, {cx+r10*6, cy}, {cx+r10*7, cy-r10}, {cx+r10*8, cy}, {cx+r10*7, cy+r10}, {cx+r10*6, cy}, {cx+r10*5, cy+r10}, {cx+r10*4, cy}, {cx+r10*3, cy+r10} };
			cRenderer(&canvas, itemsof(rgto), rgto, &black);
			cRenderer(&canvas, itemsof(rgti), rgti, &white);

			cRenderer.pixelRow(nHY)[nHX] = 0x80000000;

			if (t_fnGetIcon)
			{
				HICON hIcon = t_fnGetIcon((min(nSX, nSY)+1)/2);
				ICONINFO tInfo;
				ZeroMemory(&tInfo, sizeof tInfo);
				GetIconInfo(hIcon, &tInfo);
				if (tInfo.hbmMask)
					DeleteObject(tInfo.hbmMask);
				if (tInfo.hbmColor)
				{
					BITMAP tBmp;
					ZeroMemory(&tBmp, sizeof tBmp);
					GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
					if (tBmp.bmBitsPixel == 32)
					{
						int nSmSizeX = tBmp.bmWidth;
						int nSmSizeY = tBmp.bmHeight;
						DWORD nXOR = nSmSizeY*nSmSizeX<<2;
						DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
						CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
						ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

						if (GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pIconRes.m_p+sizeof BITMAPINFOHEADER))
						{
							ULONG offX = (nSX-nSmSizeX)*7/8;
							ULONG offY = (nSY-nSmSizeY)*7/8;
							DWORD const* pS = reinterpret_cast<DWORD*>(pIconRes.m_p+sizeof BITMAPINFOHEADER);
							for (int y = 0; y < nSmSizeY; ++y)
							{
								DWORD* pLD = cRenderer.pixelRow(offY+y)+offX;
								DWORD const* pLS = pS+nSmSizeX*y;
								for (int x = 0; x < nSmSizeX; ++x, ++pLD, ++pLS)
									*pLD = ((*pLS)&0xffffff)|(((*pLS)&0xfe000000)>>1);
							}
						}
					}

					DeleteObject(tInfo.hbmColor);
				}
				DestroyIcon(hIcon);
			}
			m_hCursor = cRenderer.getCursor(nHX, nHY);
		}
		*a_phCursor = m_hCursor;
		return S_OK;
	}

private:
	HCURSOR m_hCursor;
};

template<class T, pfnGetToolIcon t_fnGetIcon = NULL>
class CEditToolCustomOrMoveCursor : public CEditToolCustomPrecisionCursor<t_fnGetIcon>
{
public:
	// to be overriden
	bool UseMoveCursor(TPixelCoords const* UNREF(a_pPos)) const { return false; }

	HRESULT _GetCursor(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		if (static_cast<T*>(this)->UseMoveCursor(a_pPos))
		{
			static HCURSOR hMove = ::LoadCursor(NULL, IDC_SIZEALL);
			*a_phCursor = hMove;
			return S_OK;
		}
		else if (t_fnGetIcon == NULL)
		{
			static HCURSOR hCustom = ::LoadCursor(NULL, IDC_ARROW);
			*a_phCursor = hCustom;
			return S_OK;
		}
		else
		{
			return CEditToolCustomPrecisionCursor<t_fnGetIcon>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
		}
	}
};

