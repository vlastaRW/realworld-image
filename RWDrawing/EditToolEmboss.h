
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"


class CEditToolEmboss :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditWindow
{
public:
	CEditToolEmboss() : m_pCallback(NULL)
		//m_nExtraIndex(0), m_eBlendingMode(EBMDrawOver),
		//m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint)
	{
		//m_tColor.fR = m_tColor.fG = m_tColor.fB = m_tColor.fA = 0.0f;
	}
	~CEditToolEmboss()
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

	BEGIN_COM_MAP(CEditToolEmboss)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		m_pTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_pBuffer);
		CAutoVectorPtr<TRasterImagePixel> cOrig(new TRasterImagePixel[a_nSizeX*a_nSizeY]);
		m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nSizeX, EITIBackground, cOrig);
		TRasterImagePixel *pO = cOrig;
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				if ((x+a_nX+y+a_nY)&1)
				{
					*a_pBuffer = *pO;
				}
				++a_pBuffer;
				++pO;
			}
			a_pBuffer += a_nStride-a_nSizeX;
		}
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}

	STDMETHOD(IRasterImageEditTool::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
	}

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		m_pWindow = a_pWindow;
		CComObject<CCallbackHelper<IRasterImageEditWindow> >::CreateInstance(&m_pCallback);
		m_pCallback->AddRef();
		m_pCallback->SetOwner(this);
		m_pTool->Init(m_pCallback);
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		return m_pTool->SetState(a_pState);
		//CComQIPtr<CEditToolDataLine::ISharedStateToolData> pData(a_pState);
		//if (pData)
		//{
		//	m_cData = *(pData->InternalData());
		//}
		//return S_OK;
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		return m_pTool->SetGlobals(a_eBlendingMode, a_eRasterizationMode, a_eCoordinatesMode);
		//m_eBlendingMode = a_eBlendingMode;
		//m_eRasterizationMode = a_eRasterizationMode;
		//m_eCoordinatesMode = a_eCoordinatesMode;
		////, EShapeFillMode a_eShapeFillMode
		//return S_OK;
	}

	STDMETHOD(Reset)()
	{
		return m_pTool->Reset();
		//m_bDragging = false;
		//return S_OK;
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return m_pTool->PreTranslateMessage(a_pMsg);
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		return m_pTool->IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect);
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		return m_pTool->AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		return m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		return m_pTool->GetCursor(a_eKeysState, a_pPos, a_phCursor);
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

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return m_pWindow->Size(a_pSizeX, a_pSizeY);
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		return m_pWindow->GetDefaultColor(a_pDefault);
	}
	STDMETHOD(IRasterImageEditWindow::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_eIntent, a_pBuffer);
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
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
		return m_pWindow->RectangleChanged(a_pChanged);
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		return m_pWindow->SetState(a_pState);
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		return m_pWindow->Handle(a_phWnd);
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		return m_pWindow->Document(a_ppDocument);
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	//EBlendingMode m_eBlendingMode;
	//ERasterizationMode m_eRasterizationMode;
	//ECoordinatesMode m_eCoordinatesMode;
	//CEditToolDataLine m_cData;
};

