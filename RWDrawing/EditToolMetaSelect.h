
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "ConstantRasterImageBrush.h"


class CEditToolMetaSelect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditWindow
{
public:
	CEditToolMetaSelect() : m_pCallback(NULL), m_eKeysState(ECKSNone)
	{
	}
	~CEditToolMetaSelect()
	{
		m_pTool = NULL;
		if (m_pCallback)
		{
			m_pCallback->SetOwner(NULL);
			m_pCallback->Release();
		}
	}
	void Init(IRasterImageEditTool* a_pInternalTool)
	{
		m_pTool = a_pInternalTool;
	}

	static HRESULT WINAPI QICustomToolInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CEditToolMetaSelect* const p = reinterpret_cast<CEditToolMetaSelect*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolMetaSelect)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
	}

	STDMETHOD(IRasterImageEditTool::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		RECT rcTool = RECT_EMPTY;
		if (m_pTool->IsDirty(&rcTool, NULL, NULL) != S_OK)
			return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);

		if (a_bEntireRectangle)
			*a_bEntireRectangle = FALSE;
		if (m_eKeysState == ECKSNone)
		{
			if (a_pBoundingRectangle)
				*a_pBoundingRectangle = rcTool;
		}
		else
		{
			BOOL b;
			RECT rc;
			m_pWindow->GetSelectionInfo(&rc, &b);
			a_pBoundingRectangle->left = min(rc.left, rcTool.left);
			a_pBoundingRectangle->right = max(rc.right, rcTool.right);
			a_pBoundingRectangle->top = min(rc.top, rcTool.top);
			a_pBoundingRectangle->bottom = max(rc.bottom, rcTool.bottom);
		}
		return S_OK;
	}
	STDMETHOD(IRasterImageEditTool::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		if (m_pTool->IsDirty(NULL, NULL, NULL) != S_OK)
			return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);

		if (m_eKeysState != ECKSNone)
			m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);

		CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[a_nSizeX*a_nSizeY]);
		m_pTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, 0.0f, a_nSizeX, cBuffer);
		TRasterImagePixel const* pS = cBuffer;
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			switch (m_eKeysState)
			{
			case ECKSNone:
				for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer, ++pS)
					*a_pBuffer = pS->bA;
				break;
			case ECKSShift:
				for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer, ++pS)
					*a_pBuffer = 255-(255-*a_pBuffer)*(255-pS->bA)/255;
				break;
			case ECKSControl:
				for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer, ++pS)
					*a_pBuffer = abs(int(*a_pBuffer)-int(pS->bA));
				break;
			case ECKSShiftControl:
				for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer, ++pS)
					*a_pBuffer = ULONG(*a_pBuffer)*(255-pS->bA)/255;
				break;
			}
			pS += a_nStride-a_nSizeX;
		}
		return S_OK;
	}

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		m_pWindow = a_pWindow;
		CComObject<CCallbackHelper<IRasterImageEditWindow> >::CreateInstance(&m_pCallback);
		m_pCallback->AddRef();
		m_pCallback->SetOwner(this);
		HRESULT hRes = m_pTool->Init(m_pCallback);
		CComObject<CConstantRasterImageBrush>* p = NULL;
		CComObject<CConstantRasterImageBrush>::CreateInstance(&p);
		CComPtr<IRasterImageBrush> pBr = p;
		static TColor const tClr = {0.0f, 0.0f, 0.0f, 1.0f};
		p->Init(tClr);
		m_pTool->SetBrush(pBr);
		return hRes;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		m_pTool->SetState(a_pState);
		return S_OK;
	}
	STDMETHOD(SetOutline)(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		return S_FALSE;//m_pTool->SetOutline(a_bEnabled, a_fWidth, a_pColor);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return S_FALSE;//m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		return m_pTool->SetGlobals(EBMReplace/*a_eBlendingMode*/, a_eRasterizationMode, a_eCoordinatesMode);
	}

	STDMETHOD(Reset)()
	{
		return m_pTool->Reset();
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return m_pTool->PreTranslateMessage(a_pMsg);
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect == NULL)
			return m_pTool->IsDirty(NULL, NULL, NULL);
		RECT rc = RECT_EMPTY;
		HRESULT hRes = m_pTool->IsDirty(&rc, NULL, NULL);
		if (hRes != S_OK)
			return hRes;
		if (m_eKeysState == ECKSNone)
		{
			ULONG nX = 0, nY = 0;
			m_pWindow->Size(&nX, &nY);
			RECT rcOldSel = {0, 0, nX, nY};
			BOOL b;
			m_pWindow->GetSelectionInfo(&rcOldSel, &b);
			a_pSelectionRect->left = min(rc.left, rcOldSel.left);
			a_pSelectionRect->right = max(rc.right, rcOldSel.right);
			a_pSelectionRect->top = min(rc.top, rcOldSel.top);
			a_pSelectionRect->bottom = max(rc.bottom, rcOldSel.bottom);
			return S_OK;
		}
		else
		{
			*a_pSelectionRect = rc;
			return S_OK;
		}
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		return m_pTool->AdjustCoordinates(ECKSNone, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		if (a_pPos && S_OK != m_pTool->IsDirty(NULL, NULL, NULL))
			m_eKeysState = static_cast<EControlKeysState>(a_eKeysState&ECKSShiftControl);
		return m_pTool->ProcessInputEvent(ECKSNone, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		return m_pTool->GetCursor(ECKSNone, a_pPos, a_phCursor);
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		return m_pTool->GetControlPointCount(a_pCount);
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return m_pTool->GetControlPoint(a_nIndex, a_pPos, a_pClass);
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		return m_pTool->SetControlPoint(a_nIndex, a_pPos, a_bFinished, a_fPointSize);
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return m_pTool->GetControlPointDesc(a_nIndex, a_ppDescription);
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		return m_pTool->GetControlLines(a_pLines, a_nLineTypes);
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* a_pPos, BYTE a_bAccurate, float a_fPointSize)
	{
		return m_pTool->PointTest(ECKSNone, a_pPos, a_bAccurate, a_fPointSize);
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return m_pTool->Transform(a_pMatrix);
	}

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return m_pWindow->Size(a_pSizeX, a_pSizeY);
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		a_pDefault->bR = a_pDefault->bG = a_pDefault->bB = a_pDefault->bA = 0;
		return S_OK;
	}
	STDMETHOD(IRasterImageEditWindow::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			switch (a_eIntent)
			{
			case EITIBackground:
				for (ULONG y = 0; y < a_nSizeY; ++y)
					ZeroMemory(a_pBuffer+a_nStride*y, a_nSizeX*sizeof*a_pBuffer);
				return S_OK;
			case EITIContent:
				return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_eIntent, a_pBuffer);
			case EITIBoth:
				// this is a bit hacky
				{
					CAutoVectorPtr<BYTE> cBuf(new BYTE[a_nSizeX*a_nSizeY+1]);
					if (FAILED(m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nSizeX, cBuf)))
						return E_FAIL;
					BYTE const* p = cBuf;
					for (ULONG nY = 0; nY < a_nSizeY; ++nY)
					{
						for (ULONG nX = 0; nX < a_nSizeX; ++nX, ++p, ++a_pBuffer)
						{
							a_pBuffer->bR = a_pBuffer->bG = a_pBuffer->bB = 0;
							a_pBuffer->bA = *p;
						}
						a_pBuffer += a_nStride-a_nSizeX;
					}
					return S_OK;
				}
			default:
				return E_FAIL;
			}
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_bEntireRectangle)
			*a_bEntireRectangle = TRUE;
		if (a_pBoundingRectangle)
		{
			ULONG nX = 0, nY = 0;
			m_pWindow->Size(&nX, &nY);
			a_pBoundingRectangle->left = a_pBoundingRectangle->top = 0;
			a_pBoundingRectangle->right = nX;
			a_pBoundingRectangle->bottom = nY;
		}
		return S_OK;
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		try
		{
			for (ULONG y = 0; y < a_nSizeY; ++y)
				FillMemory(a_pBuffer+a_nStride*y, a_nSizeX*sizeof*a_pBuffer, 0xff);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return m_pWindow->ControlPointsChanged();
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return m_pWindow->ControlPointChanged(a_nIndex);
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return m_pWindow->ControlLinesChanged();
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_eKeysState == ECKSNone)
		{
			ULONG nX = 0, nY = 0;
			m_pWindow->Size(&nX, &nY);
			RECT rcOldSel = {0, 0, nX, nY};
			BOOL b;
			m_pWindow->GetSelectionInfo(&rcOldSel, &b);
			RECT rc;
			rc.left = min(a_pChanged->left, rcOldSel.left);
			rc.right = max(a_pChanged->right, rcOldSel.right);
			rc.top = min(a_pChanged->top, rcOldSel.top);
			rc.bottom = max(a_pChanged->bottom, rcOldSel.bottom);
			return m_pWindow->RectangleChanged(&rc);
		}
		else
		{
			return m_pWindow->RectangleChanged(a_pChanged);
		}
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		return m_pWindow->SetState(a_pState);
	}
	STDMETHOD(IRasterImageEditWindow::SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		return m_pWindow->SetBrushState(a_bstrStyleID, a_pState);
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		return m_pWindow->Handle(a_phWnd);
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		return E_NOTIMPL;//m_pWindow->Document(a_ppDocument);
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	EControlKeysState m_eKeysState;
};

