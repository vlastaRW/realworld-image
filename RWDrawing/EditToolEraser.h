
#pragma once

#include "ConstantRasterImageBrush.h"


class CEditToolEraser :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool
{
public:
	void Init(IRasterImageEditTool* a_pInternalTool)
	{
		m_pTool = a_pInternalTool;
	}

	static HRESULT WINAPI QICustomToolInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CEditToolEraser* const p = reinterpret_cast<CEditToolEraser*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolEraser)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		return m_pTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_pBuffer);
	}

	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pTool->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pTool->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
	}

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		HRESULT hRes = m_pTool->Init(a_pWindow);
		CComObject<CConstantRasterImageBrush>* p = NULL;
		CComObject<CConstantRasterImageBrush>::CreateInstance(&p);
		CComPtr<IRasterImageBrush> pBr = p;
		static TColor const tClr = {0.0f, 0.0f, 0.0f, 0.0f};
		p->Init(tClr);
		m_pTool->SetBrush(pBr);
		return hRes;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return m_pTool->SetState(a_pState);
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
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* a_pPos, BYTE a_bAccurate, float a_fPointSize)
	{
		return m_pTool->PointTest(ECKSNone, a_pPos, a_bAccurate, a_fPointSize);
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return m_pTool->Transform(a_pMatrix);
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
};

