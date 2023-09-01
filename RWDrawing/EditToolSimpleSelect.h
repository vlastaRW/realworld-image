
#pragma once

#include "EditTool.h"
#include "EditToolPixelMixer.h"

#include <math.h>
#include <Resampling.h>
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>


struct CEditToolDataSimpleSelect
{
	MIDL_INTERFACE("E5A9CD4E-F104-4BA6-9A17-D3EF05E4900B")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataSimpleSelect const*, InternalData)() = 0;
	};

	enum EFilteringType
	{
		EFTNearest = 0,
		EFTLinear,
		EFTCubic
	};
	CEditToolDataSimpleSelect() : eFilteringType(EFTLinear), bOriginal(0), bEntireArea(0)
	{
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (wcscmp(a_bstr, L"NEAREST") == 0)
			eFilteringType = EFTNearest;
		else if (wcscmp(a_bstr, L"CUBIC") == 0)
			eFilteringType = EFTCubic;
		else
			eFilteringType = EFTLinear;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = SysAllocString(eFilteringType == EFTNearest ? L"NEAREST" : (eFilteringType == EFTCubic ? L"CUBIC" : L"LINEAR"));
		return S_OK;
	}

	EFilteringType eFilteringType;
	BYTE bOriginal;
	BYTE bEntireArea;
};

#include "EditToolSimpleSelectDlg.h"


class CEditToolSimpleSelect :
	public CEditToolMouseInput<CEditToolSimpleSelect>, // no direct tablet support
	public CEditToolCustomOrMoveCursor<CEditToolSimpleSelect>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolSimpleSelect, // T - the top level class for cross casting
		CEditToolSimpleSelect, // TResetHandler
		CEditToolSimpleSelect, // TDirtyHandler
		CEditToolSimpleSelect, // TImageTileHandler
		CEditToolSimpleSelect, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CEditToolSimpleSelect, // TGlobalsHandler
		CEditToolSimpleSelect, // TAdjustCoordsHandler
		CEditToolCustomOrMoveCursor<CEditToolSimpleSelect>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolSimpleSelect>, // TProcessInputHandler
		CEditToolSimpleSelect, // TPreTranslateMessageHandler
		CEditToolSimpleSelect, // TControlPointsHandler
		CEditToolSimpleSelect // TControlLinesHandler
	>,
	public IRasterImageEditToolContextMenu,
	public IRasterImageEditToolFloatingSelection,
	public IDesignerViewStatusBar
{
public:
	CEditToolSimpleSelect() : m_eBlendingMode(EBMDrawOver),
		m_eDragState(EDSNothing), m_bModified(false), m_rcDelete(RECT_EMPTY), m_rcSelection(RECT_EMPTY)
	{
		m_szOriginal.cx = m_szOriginal.cy = 0;
		m_szResized.cx = m_szResized.cy = 0;
	}

	BEGIN_COM_MAP(CEditToolSimpleSelect)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IRasterImageEditToolFloatingSelection)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	END_COM_MAP()

	RECT M_DirtyRect()
	{
		RECT rc =
		{
			min(m_rcDelete.left, m_rcSelection.left),
			min(m_rcDelete.top, m_rcSelection.top),
			max(m_rcDelete.right, m_rcSelection.right),
			max(m_rcDelete.bottom, m_rcSelection.bottom),
		};
		return rc;
	}

	// IDesignerViewStatusBar
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		OLECHAR szTmp[64];
		if (m_eDragState >= EDSSelection)
		{
			swprintf(szTmp, L"%ix%i", m_rcSelection.right-m_rcSelection.left, m_rcSelection.bottom-m_rcSelection.top);
		}
		else if (m_eDragState == EDSDefining && m_tOtherPos.fX != m_tStartPos.fX && m_tOtherPos.fY != m_tStartPos.fY)
		{
			int const fX1 = min(m_tOtherPos.fX, m_tStartPos.fX);
			int const fY1 = min(m_tOtherPos.fY, m_tStartPos.fY);
			int const fX2 = max(m_tOtherPos.fX, m_tStartPos.fX);
			int const fY2 = max(m_tOtherPos.fY, m_tStartPos.fY);
			if (fX2-fX1 || fY2-fY1)
				swprintf(szTmp, L"%ix%i", fX2-fX1, fY2-fY1);
			else
				return S_FALSE;
		}
		else
			return S_FALSE;
		return a_pStatusBar->PaneSet(CComBSTR("SIMPSELSIZE"), NULL, CComBSTR(szTmp), 60, -200);
	}

	// IRasterImageEditToolContextMenu methods
public:
	class ATL_NO_VTABLE CRestoreSelection :
		public CDocumentMenuCommandImpl<CRestoreSelection, IDS_MENU_RESTORESELECTION_NAME, IDS_MENU_RESTORESELECTION_DESC, NULL, 0>
	{
	public:
		CRestoreSelection() : m_pTool(NULL)
		{
		}
		~CRestoreSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolSimpleSelect* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->RestoreSelection();
		}

	private:
		CEditToolSimpleSelect* m_pTool;
	};
	class ATL_NO_VTABLE CStretchSelection :
		public CDocumentMenuCommandImpl<CStretchSelection, IDS_MENU_STRETCHSELECTION_NAME, IDS_MENU_STRETCHSELECTION_DESC, NULL, 0>
	{
	public:
		CStretchSelection() : m_pTool(NULL)
		{
		}
		~CStretchSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolSimpleSelect* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->StretchSelection();
		}

	private:
		CEditToolSimpleSelect* m_pTool;
	};
	//class ATL_NO_VTABLE CCancelSelection :
	//	public CDocumentMenuCommandImpl<CCancelSelection, IDS_MENU_CANCELSELECTION_NAME, IDS_MENU_CANCELSELECTION_DESC, NULL, 0>
	//{
	//public:
	//	CCancelSelection() : m_pTool(NULL)
	//	{
	//	}
	//	~CCancelSelection()
	//	{
	//		if (m_pTool) m_pTool->Release();
	//	}
	//	void Init(CEditToolSimpleSelect* a_pTool)
	//	{
	//		(m_pTool = a_pTool)->AddRef();
	//	}

	//	// IDocumentMenuCommand
	//public:
	//	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	//	{
	//		return m_pTool->CancelSelection();
	//	}

	//private:
	//	CEditToolSimpleSelect* m_pTool;
	//};

	HRESULT RestoreSelection()
	{
		if (M_Window() == NULL || _IsDirty(NULL, NULL, NULL) != S_OK)
			return S_FALSE;
		RECT rcPrev = M_DirtyRect();
		m_rcSelection.left = ((m_rcSelection.left+m_rcSelection.right)>>1)-(m_szOriginal.cx>>1);
		m_rcSelection.right = m_rcSelection.left+m_szOriginal.cx;
		m_rcSelection.top = ((m_rcSelection.top+m_rcSelection.bottom)>>1)-(m_szOriginal.cy>>1);
		m_rcSelection.bottom = m_rcSelection.top+m_szOriginal.cy;
		RECT rcNew = M_DirtyRect();
		if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
		if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
		if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
		if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
		M_Window()->RectangleChanged(&rcNew);
		M_Window()->ControlLinesChanged();
		for (LONG i = 0; i < 8; ++i)
			M_Window()->ControlPointChanged(i);
		return S_OK;
	}
	HRESULT StretchSelection()
	{
		if (M_Window() == NULL || _IsDirty(NULL, NULL, NULL) != S_OK)
			return S_FALSE;
		RECT rcPrev = M_DirtyRect();
		m_rcSelection.left = 0;
		m_rcSelection.right = m_nSizeX;
		m_rcSelection.top = 0;
		m_rcSelection.bottom = m_nSizeY;
		RECT rcNew = M_DirtyRect();
		if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
		if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
		if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
		if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
		M_Window()->RectangleChanged(&rcNew);
		M_Window()->ControlLinesChanged();
		for (LONG i = 0; i < 8; ++i)
			M_Window()->ControlPointChanged(i);
		return S_OK;
	}
	//HRESULT CancelSelection()
	//{
	//	if (M_Window() == NULL || _IsDirty(NULL, NULL, NULL) != S_OK)
	//		return S_FALSE;
	//	return _Reset();
	//}

	STDMETHOD(CommandsEnum)(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			if (_IsDirty(NULL, NULL, NULL) != S_OK)
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CRestoreSelection>* p = NULL;
				CComObject<CRestoreSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CStretchSelection>* p = NULL;
				CComObject<CStretchSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			//{
			//	CComObject<CCancelSelection>* p = NULL;
			//	CComObject<CCancelSelection>::CreateInstance(&p);
			//	CComPtr<IDocumentMenuCommand> pTmp = p;
			//	p->Init(this);
			//	pItems->Insert(pTmp);
			//}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		RECT const rc = M_DirtyRect();
		m_eDragState = EDSNothing;
		m_bModified = false;
		m_rcDelete = RECT_EMPTY;
		m_rcSelection = RECT_EMPTY;
		m_szOriginal.cx = m_szOriginal.cy = 0;
		m_szResized.cx = m_szResized.cy = 0;
		m_pOriginal.Free();
		m_pResized.Free();
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		RECT const rcDirty = M_DirtyRect();
		if (rcDirty.left < rcDirty.right && rcDirty.top < rcDirty.bottom)
		{
			if (a_pOptimizeImageRect)
				*a_pOptimizeImageRect = (m_rcSelection.right-m_rcSelection.left)*(m_rcSelection.bottom-m_rcSelection.top)*3 < (rcDirty.right-rcDirty.left)*(rcDirty.bottom-rcDirty.top);
			if (a_pImageRect)
				*a_pImageRect = rcDirty;
			return S_OK;
		}
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pImageRect)
			*a_pImageRect = RECT_EMPTY;
		return S_FALSE;
	}

	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		RECT const rcRequired = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		RECT const rcDirty = M_DirtyRect();
		if (rcRequired.left >= rcDirty.right || rcRequired.right <= rcDirty.left ||
			rcRequired.top >= rcDirty.bottom || rcRequired.bottom <= rcDirty.top ||
			(!m_bModified && m_rcDelete.left == m_rcSelection.left && m_rcDelete.right == m_rcSelection.right &&
			m_rcDelete.top == m_rcSelection.top && m_rcDelete.bottom == m_rcSelection.bottom))
			return M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		RECT rcDelete =
		{
			max(m_rcDelete.left, rcRequired.left),
			max(m_rcDelete.top, rcRequired.top),
			min(m_rcDelete.right, rcRequired.right),
			min(m_rcDelete.bottom, rcRequired.bottom),
		};
		if (rcDelete.left < rcDelete.right && rcDelete.top < rcDelete.bottom)
		{
			// top stripe
			if (rcRequired.top < rcDelete.top)
				M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, rcDelete.top-rcRequired.top, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
			// bottom stripe
			if (rcRequired.bottom > rcDelete.bottom)
				M_Window()->GetImageTile(a_nX, rcDelete.bottom, a_nSizeX, rcRequired.bottom-rcDelete.bottom, a_fGamma, a_nStride, EITIBackground, a_pBuffer+a_nStride*(rcDelete.bottom-rcRequired.top));
			// left stripe
			if (rcRequired.left < rcDelete.left)
				M_Window()->GetImageTile(a_nX, rcDelete.top, rcDelete.left-rcRequired.left, rcDelete.bottom-rcDelete.top, a_fGamma, a_nStride, EITIBackground, a_pBuffer+a_nStride*(rcDelete.top-rcRequired.top));
			// right stripe
			if (rcRequired.right > rcDelete.right)
				M_Window()->GetImageTile(rcDelete.right, rcDelete.top, rcRequired.right-rcDelete.right, rcDelete.bottom-rcDelete.top, a_fGamma, a_nStride, EITIBackground, a_pBuffer+a_nStride*(rcDelete.top-rcRequired.top)+rcDelete.right-rcRequired.left);
			// the actual deleted area
			for (LONG y = rcDelete.top; y < rcDelete.bottom; ++y)
				ZeroMemory(a_pBuffer+a_nStride*(y-rcRequired.top)+rcDelete.left-rcRequired.left, (rcDelete.right-rcDelete.left)*sizeof*a_pBuffer);
		}
		else
		{
			M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		}
		if (m_pOriginal == NULL)
			return S_OK;
		{
			// ensure cache is valid
			ObjectLock cLock(this);
			if (m_pResized.m_p == NULL || m_szResized.cx != m_rcSelection.right-m_rcSelection.left ||
				m_szResized.cy != m_rcSelection.bottom-m_rcSelection.top)
			{
				m_szResized.cx = m_rcSelection.right-m_rcSelection.left;
				m_szResized.cy = m_rcSelection.bottom-m_rcSelection.top;
				m_pResized.Free();
				m_pResized.Allocate(m_szResized.cx*m_szResized.cy);
				CResampling cRsmp(m_szResized.cx, m_szResized.cy, m_szOriginal.cx, m_szOriginal.cy, m_pResized, m_pOriginal);
				switch (m_cData.eFilteringType)
				{
				case CEditToolDataSimpleSelect::EFTCubic:
					cRsmp.Cubic();
					break;
				case CEditToolDataSimpleSelect::EFTLinear:
					cRsmp.Linear();
					break;
				default:
					cRsmp.Nearest();
					break;
				}
			}
		}
		RECT const rcSelection =
		{
			max(m_rcSelection.left, rcRequired.left),
			max(m_rcSelection.top, rcRequired.top),
			min(m_rcSelection.right, rcRequired.right),
			min(m_rcSelection.bottom, rcRequired.bottom),
		};
		for (LONG y = rcSelection.top; y < rcSelection.bottom; ++y)
		{
			TRasterImagePixel const* pS = m_pResized.m_p + (y-m_rcSelection.top)*(m_rcSelection.right-m_rcSelection.left) + rcSelection.left-m_rcSelection.left;
			TRasterImagePixel* pD = a_pBuffer+a_nStride*(y-a_nY)+rcSelection.left-a_nX;
			switch (m_eBlendingMode)
			{
			case EBMReplace:
				for (LONG x = rcSelection.left; x < rcSelection.right; ++x, ++pS, ++pD)
					CPixelMixerReplace::Mix(*pD, *pS);
				break;
			case EBMDrawOver:
				for (LONG x = rcSelection.left; x < rcSelection.right; ++x, ++pS, ++pD)
					CPixelMixerPaintOver::Mix(*pD, *pS);
				break;
			case EBMDrawUnder:
				for (LONG x = rcSelection.left; x < rcSelection.right; ++x, ++pS, ++pD)
					CPixelMixerPaintUnder::Mix(*pD, *pS);
				break;
			}
		}
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataSimpleSelect::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			CEditToolDataSimpleSelect::EFilteringType eFT = m_cData.eFilteringType;
			m_cData = *(pData->InternalData());
			if (eFT != m_cData.eFilteringType && m_pResized.m_p)
			{
				{
					ObjectLock cLock(this);
					m_pResized.Free();
				}
				M_Window()->RectangleChanged(&M_DirtyRect());
			}
		}
		return S_OK;
	}

	//void ColorChanged(bool a_bColor1, bool a_bColor2)
	//{
	//	if (!M_PolyLine().empty())
	//	{
	//		if (((a_bColor1 && !m_bUseSecondary) || (a_bColor2 && m_bUseSecondary)) && m_eShapeFillMode == ESFMSolidFill)
	//		{
	//			InvalidateCachePixels();
	//			M_Window()->RectangleChanged(&M_DirtyRect());
	//		}
	//	}
	//}

	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		bool bBlendingChange = m_eBlendingMode != a_eBlendingMode;

		if (!bBlendingChange)
			return S_FALSE;
		m_eBlendingMode = a_eBlendingMode;
		M_Window()->RectangleChanged(&M_DirtyRect());
		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tStartPos = m_tOtherPos = *a_pPos;
		if (HitTest(a_pPos->fX, a_pPos->fY))
		{
			m_eDragState = EDSMoving;
			return S_OK;
		}

		if (m_eDragState != EDSNothing)
		{
			// apply previous
			ATLASSERT(0);
			//M_Window()->ApplyChanges();
		}

		m_eDragState = EDSDefining;
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();

		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		OnMouseMove(a_eKeysState, a_pPos);
		if (m_eDragState == EDSDefining)
		{
			m_rcDelete.left = min(a_pPos->fX, m_tStartPos.fX);
			m_rcDelete.top = min(a_pPos->fY, m_tStartPos.fY);
			m_rcDelete.right = max(a_pPos->fX, m_tStartPos.fX);
			m_rcDelete.bottom = max(a_pPos->fY, m_tStartPos.fY);
			if (m_rcDelete.left < 0) m_rcDelete.left = 0;
			if (m_rcDelete.top < 0) m_rcDelete.top = 0;
			if (m_rcDelete.right > LONG(m_nSizeX)) m_rcDelete.right = m_nSizeX;
			if (m_rcDelete.bottom > LONG(m_nSizeY)) m_rcDelete.bottom = m_nSizeY;
			if (m_rcDelete.left < m_rcDelete.right && m_rcDelete.top < m_rcDelete.bottom)
			{
				m_rcSelection = m_rcDelete;
				m_szOriginal.cx = m_rcSelection.right-m_rcSelection.left;
				m_szOriginal.cy = m_rcSelection.bottom-m_rcSelection.top;
				ATLASSERT(m_pOriginal.m_p == NULL);
				m_pOriginal.Free();
				m_pOriginal.Allocate(m_szOriginal.cx*m_szOriginal.cy);
				M_Window()->GetImageTile(m_rcSelection.left, m_rcSelection.top, m_szOriginal.cx, m_szOriginal.cy, 0.0f, m_szOriginal.cx, EITIContent, m_pOriginal);
				m_bModified = false;
				{
					ObjectLock cLock(this);
					m_pResized.Free();
				}
				m_eDragState = EDSSelection;
				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
				M_Window()->RectangleChanged(&M_DirtyRect());
			}
			else
			{
				m_eDragState = EDSNothing;
				M_Window()->ControlLinesChanged();
			}
		}
		else
		{
			m_eDragState = EDSSelection;
		}
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eDragState == EDSDefining)
		{
			if (m_tOtherPos.fX != a_pPos->fX || m_tOtherPos.fY != a_pPos->fY)
			{
				m_tOtherPos = *a_pPos;
				M_Window()->ControlLinesChanged();
			}
			return S_OK;
		}
		else if (m_eDragState == EDSMoving)
		{
			if (m_tOtherPos.fX != a_pPos->fX || m_tOtherPos.fY != a_pPos->fY)
			{
				LONG nDX = a_pPos->fX-m_tOtherPos.fX;
				LONG nDY = a_pPos->fY-m_tOtherPos.fY;
				m_tOtherPos = *a_pPos;
				RECT rcPrev = M_DirtyRect();
				m_rcSelection.left += nDX;
				m_rcSelection.right += nDX;
				m_rcSelection.top += nDY;
				m_rcSelection.bottom += nDY;
				RECT rcNew = M_DirtyRect();
				if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
				if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
				if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
				if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
				M_Window()->RectangleChanged(&rcNew);
				M_Window()->ControlLinesChanged();
				for (LONG i = 0; i < 8; ++i)
					M_Window()->ControlPointChanged(i);
			}
			return S_OK;
		}
		return S_OK;
	}

	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_eDragState == EDSMoving || (m_eDragState == EDSSelection && HitTest(a_pPos->fX, a_pPos->fY));
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		TPixelCoords const t = *a_pPos;
		a_pPos->fX = floorf(a_pPos->fX+0.5f);
		a_pPos->fY = floorf(a_pPos->fY+0.5f);
		if (a_pControlPointIndex && *a_pControlPointIndex < 8)
		{
			if ((*a_pControlPointIndex&3) == 1)
			{
				a_pPos->fX = (m_rcSelection.left+m_rcSelection.right)*0.5f;
			}
			else// if ((*a_pControlPointIndex-2) <= 2)
			{
				a_pPos->fX = floorf(t.fX+0.5f);
				//a_pPos->fX = ceilf(t.fX);
			}
			if ((*a_pControlPointIndex&3) == 3)
			{
				a_pPos->fY = (m_rcSelection.top+m_rcSelection.bottom)*0.5f;
			}
			else// if (*a_pControlPointIndex > 2)
			{
				a_pPos->fY = floorf(t.fY+0.5f);
				//a_pPos->fY = ceilf(t.fY);
			}
		}

		return S_OK;
	}

	bool HitTest(LONG a_nX, LONG a_nY) const
	{
		return a_nX >= m_rcSelection.left && a_nX < m_rcSelection.right && a_nY >= m_rcSelection.top && a_nY < m_rcSelection.bottom;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = m_eDragState >= EDSSelection ? 8 : 0;
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		try
		{
			if (m_eDragState < EDSSelection || a_nIndex >= 8)
				return E_RW_INDEXOUTOFRANGE;
			*a_pClass = a_nIndex&1;
			a_pPos->fX = (a_nIndex&3) == 1 ? (m_rcSelection.left+m_rcSelection.right)*0.5f : (a_nIndex-2) <= 2 ? m_rcSelection.right : m_rcSelection.left;
			a_pPos->fY = (a_nIndex&3) == 3 ? (m_rcSelection.top+m_rcSelection.bottom)*0.5f : a_nIndex <= 2 ? m_rcSelection.top : m_rcSelection.bottom;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		try
		{
			if (m_eDragState < EDSSelection || a_nIndex >= 8)
				return E_RW_INDEXOUTOFRANGE;
			RECT rcPrev = M_DirtyRect();
			BYTE bChange = 0;
			// 0 1 2
			// 7   3
			// 6 5 4
			if ((a_nIndex&3) != 1)
			{
				if ((a_nIndex-2) <= 2)
				{
					m_rcSelection.right = a_pPos->fX;
					if (m_rcSelection.left >= m_rcSelection.right)
					{
						m_rcSelection.left = m_rcSelection.right-1;
						bChange = 0xff;
					}
					else
						bChange = 0x3e;
				}
				else
				{
					m_rcSelection.left = a_pPos->fX;
					if (m_rcSelection.left >= m_rcSelection.right)
					{
						m_rcSelection.right = m_rcSelection.left+1;
						bChange = 0xff;
					}
					else
						bChange = 0xe3;
				}
			}
			if ((a_nIndex&3) != 3)
			{
				if (a_nIndex <= 2)
				{
					m_rcSelection.top = a_pPos->fY;
					if (m_rcSelection.top >= m_rcSelection.bottom)
					{
						m_rcSelection.bottom = m_rcSelection.top+1;
						bChange = 0xff;
					}
					else
						bChange |= 0x8f;
				}
				else
				{
					m_rcSelection.bottom = a_pPos->fY;
					if (m_rcSelection.top >= m_rcSelection.bottom)
					{
						m_rcSelection.top = m_rcSelection.bottom-1;
						bChange = 0xff;
					}
					else
						bChange |= 0xf8;
				}
			}
			for (int i = 0; bChange; bChange>>=1, ++i)
				if (bChange&1)
					M_Window()->ControlPointChanged(i);
			M_Window()->ControlLinesChanged();
			RECT rcNew = M_DirtyRect();
			if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
			if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
			if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
			if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
			M_Window()->RectangleChanged(&rcNew);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_eDragState >= EDSSelection)
		{
			a_pLines->MoveTo(m_rcSelection.left, m_rcSelection.top);
			a_pLines->LineTo(m_rcSelection.right, m_rcSelection.top);
			a_pLines->MoveTo(m_rcSelection.right, m_rcSelection.bottom);
			a_pLines->LineTo(m_rcSelection.right, m_rcSelection.top);
			a_pLines->MoveTo(m_rcSelection.right, m_rcSelection.bottom);
			a_pLines->LineTo(m_rcSelection.left, m_rcSelection.bottom);
			a_pLines->MoveTo(m_rcSelection.left, m_rcSelection.top);
			a_pLines->LineTo(m_rcSelection.left, m_rcSelection.bottom);
			return S_OK;
		}
		else if (m_eDragState == EDSDefining && m_tOtherPos.fX != m_tStartPos.fX && m_tOtherPos.fY != m_tStartPos.fY)
		{
			float const fX1 = min(m_tOtherPos.fX, m_tStartPos.fX);
			float const fY1 = min(m_tOtherPos.fY, m_tStartPos.fY);
			float const fX2 = max(m_tOtherPos.fX, m_tStartPos.fX);
			float const fY2 = max(m_tOtherPos.fY, m_tStartPos.fY);

			a_pLines->MoveTo(fX1, fY1);
			a_pLines->LineTo(fX2, fY1);
			a_pLines->MoveTo(fX2, fY2);
			a_pLines->LineTo(fX2, fY1);
			a_pLines->MoveTo(fX2, fY2);
			a_pLines->LineTo(fX1, fY2);
			a_pLines->MoveTo(fX1, fY1);
			a_pLines->LineTo(fX1, fY2);
			return S_OK;
		}
		else
		{
			return S_FALSE;
		}
	}

	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (m_pOriginal.m_p == NULL)
			return S_FALSE;

		if (a_pMsg->message == WM_KEYDOWN)
		{
			if (a_pMsg->wParam == VK_DELETE && m_rcSelection.right > (m_rcSelection.left+1))
			{
				m_bModified = true;
				m_rcSelection = m_rcDelete;
				m_pOriginal.Free();
				return ETPAApply;
			}
			else if (0x8000&GetAsyncKeyState(VK_SHIFT))
			{
				if (a_pMsg->wParam == VK_LEFT && m_rcSelection.right > (m_rcSelection.left+1))
				{
					TPixelCoords t = {m_rcSelection.right-1, 0};
					_SetControlPoint(3, &t, TRUE, 0);
					return S_OK;
				}
				else if (a_pMsg->wParam == VK_RIGHT)
				{
					TPixelCoords t = {m_rcSelection.right+1, 0};
					_SetControlPoint(3, &t, TRUE, 0);
					return S_OK;
				}
				else if (a_pMsg->wParam == VK_UP && m_rcSelection.bottom > (m_rcSelection.top+1))
				{
					TPixelCoords t = {0, m_rcSelection.bottom-1};
					_SetControlPoint(5, &t, TRUE, 0);
					return S_OK;
				}
				else if (a_pMsg->wParam == VK_DOWN)
				{
					TPixelCoords t = {0, m_rcSelection.bottom+1};
					_SetControlPoint(5, &t, TRUE, 0);
					return S_OK;
				}
				else
					return S_FALSE;
			}
			else
			{
				LONG nDX = 0;
				LONG nDY = 0;
				if (a_pMsg->wParam == VK_LEFT)
					nDX = -1;
				else if (a_pMsg->wParam == VK_RIGHT)
					nDX = 1;
				else if (a_pMsg->wParam == VK_UP)
					nDY = -1;
				else if (a_pMsg->wParam == VK_DOWN)
					nDY = 1;
				if (nDX|| nDY)
				{
					RECT rcPrev = M_DirtyRect();
					m_rcSelection.left += nDX;
					m_rcSelection.right += nDX;
					m_rcSelection.top += nDY;
					m_rcSelection.bottom += nDY;
					RECT rcNew = M_DirtyRect();
					if (rcNew.left > rcPrev.left) rcNew.left = rcPrev.left;
					if (rcNew.top > rcPrev.top) rcNew.top = rcPrev.top;
					if (rcNew.right < rcPrev.right) rcNew.right = rcPrev.right;
					if (rcNew.bottom < rcPrev.bottom) rcNew.bottom = rcPrev.bottom;
					M_Window()->RectangleChanged(&rcNew);
					M_Window()->ControlLinesChanged();
					for (LONG i = 0; i < 8; ++i)
						M_Window()->ControlPointChanged(i);
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPACustomAction : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

	// IRasterImageEditToolFloatingSelection methods
public:
	STDMETHOD(Set)(float a_fX, float a_fY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel const* a_pBuffer, BOOL a_bModifyExisting)
	{
		//if (_IsDirty(NULL, NULL, NULL) == S_OK)
		//	M_Window()->ApplyChanges();
		m_pOriginal.Free();
		m_pResized.Free();
		m_szResized.cx = m_szResized.cy = 0;
		int nPrevSizeX = m_rcSelection.right-m_rcSelection.left;
		int nPrevSizeY = m_rcSelection.bottom-m_rcSelection.top;
		RECT rcPrev = m_rcSelection;
		m_rcSelection.left = a_fX+0.5f;
		m_rcSelection.top = a_fY+0.5f;
		m_rcSelection.right = m_szOriginal.cx ? m_rcSelection.left+a_nSizeX*nPrevSizeX/m_szOriginal.cx : m_rcSelection.left+a_nSizeX;
		m_rcSelection.bottom = m_szOriginal.cy ? m_rcSelection.top+a_nSizeY*nPrevSizeY/m_szOriginal.cy : m_rcSelection.top+a_nSizeY;
		m_szOriginal.cx = a_nSizeX;
		m_szOriginal.cy = a_nSizeY;
		if (a_nSizeX && a_nSizeY && a_pBuffer)
		{
			m_pOriginal.Allocate(a_nSizeX*a_nSizeY);
			for (ULONG y = 0; y < a_nSizeY; ++y)
				CopyMemory(m_pOriginal.m_p+y*a_nSizeX, a_pBuffer+y*a_nStride, a_nSizeX*sizeof *a_pBuffer);
		}
		m_eDragState = EDSSelection;
		m_bModified = true;
		if (rcPrev.left > m_rcSelection.left) rcPrev.left = m_rcSelection.left;
		if (rcPrev.top > m_rcSelection.top) rcPrev.top = m_rcSelection.top;
		if (rcPrev.right < m_rcSelection.right) rcPrev.right = m_rcSelection.right;
		if (rcPrev.bottom < m_rcSelection.bottom) rcPrev.bottom = m_rcSelection.bottom;
		M_Window()->RectangleChanged(&rcPrev);
		M_Window()->ControlLinesChanged();
		M_Window()->ControlPointsChanged();
		return S_OK;
	}
	STDMETHOD(Position)(float* a_pX, float* a_pY)
	{
		*a_pX = m_rcSelection.left;
		*a_pY = m_rcSelection.top;
		return S_OK;
	}
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		*a_pSizeX = m_szOriginal.cx;
		*a_pSizeY = m_szOriginal.cy;
		return S_OK;
	}
	STDMETHOD(Data)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			if (LONG(a_nX+a_nSizeX) > m_szOriginal.cx || LONG(a_nY+a_nSizeY) > m_szOriginal.cy)
				return E_RW_INVALIDPARAM;
			for (ULONG y = 0; y < a_nSizeY; ++y)
				CopyMemory(a_pBuffer+y*a_nStride, m_pOriginal.m_p+(y+a_nY)*m_szOriginal.cx+a_nX, a_nSizeX*sizeof *m_pOriginal.m_p);
			return S_OK;
		}
		catch (...)
		{
			return a_pBuffer ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Delete)()
	{
		m_rcSelection.left = m_rcSelection.right = m_rcSelection.top = m_rcSelection.bottom = 0;
		m_pOriginal.Free();
		return ETPAApply;
	}
	STDMETHOD(SelectAll)()
	{
		TPixelCoords t = {0, 0};
		OnMouseDown(ECKSNone, &t);
		t.fX = m_nSizeX; t.fY = m_nSizeY;
		OnMouseUp(ECKSNone, &t);
		return S_OK;
	}

	//bool CanUndo()
	//{
	//	return SelectionExists();
	//}
	//void Undo()
	//{
	//	if (m_pSavedSelection)
	//	{
	//		delete[] m_pSelection;
	//		m_pSelection = m_pSavedSelection;
	//		m_tOrigSize = m_tSavedSize;
	//		m_pSavedSelection = NULL;
	//		m_bModified = false;
	//		DrawSelection();
	//	}
	//	else
	//	{
	//		Reset();
	//	}
	//}
	//void ModifySelection(LONG a_nX, LONG a_nY, TRasterImagePixel const* a_pData)
	//{
	//	if (m_tOrigSize.nX == (m_tSelTo.nX-m_tSelFrom.nX) &&
	//		m_tOrigSize.nY == (m_tSelTo.nY-m_tSelFrom.nY) &&
	//		(m_tOrigSize.nX != a_nX || m_tOrigSize.nY != a_nY) && !m_bModified)
	//	{
	//		m_tSelFrom.nX -= (a_nX-m_tOrigSize.nX)>>1;
	//		m_tSelFrom.nY -= (a_nY-m_tOrigSize.nY)>>1;
	//		m_tSelTo.nX = m_tSelFrom.nX+a_nX;
	//		m_tSelTo.nY = m_tSelFrom.nY+a_nY;
	//	}
	//	m_tSavedSize = m_tOrigSize;
	//	delete[] m_pSavedSelection;
	//	m_pSavedSelection = m_pSelection;
	//	m_tOrigSize.nX = a_nX;
	//	m_tOrigSize.nY = a_nY;
	//	m_pSelection = NULL;
	//	if (a_nX*a_nY)
	//	{
	//		m_pSelection = new TRasterImagePixel[a_nX*a_nY];
	//		std::copy(a_pData, a_pData+a_nX*a_nY, m_pSelection);
	//		DrawSelection();
	//		m_bModified = true;
	//	}
	//}

private:
	enum EDragState {EDSNothing, EDSDefining, EDSSelection, EDSMoving};

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	TPixelCoords m_tStartPos;
	TPixelCoords m_tOtherPos;

	CAutoVectorPtr<TRasterImagePixel> m_pOriginal;
	SIZE m_szOriginal;
	CAutoVectorPtr<TRasterImagePixel> m_pResized;
	SIZE m_szResized;
	RECT m_rcDelete;
	RECT m_rcSelection;
	EDragState m_eDragState;
	bool m_bModified;

	EBlendingMode m_eBlendingMode;

	CEditToolDataSimpleSelect m_cData;

	//TDrawCoord m_tSavedSize;
	//TRasterImagePixel* m_pSavedSelection;
};
