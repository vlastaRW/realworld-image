
#pragma once

#include "EditTool.h"

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataMove
{
	MIDL_INTERFACE("8564C76B-35F7-4F16-9681-2D3E7FDE3574")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataMove const*, InternalData)() = 0;
	};

	enum EExterior
	{
		EEWrap = 0,
		EEFill,
	};
	CEditToolDataMove() : eExterior(EEWrap)
	{
	}
	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		int nLen = SysStringLen(a_bstr);
		if (nLen < 4)
			return S_FALSE;
		eExterior = wcsncmp(a_bstr, L"WRAP", 4) == 0 ? EEWrap : EEFill;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = SysAllocString(eExterior == EEFill ? L"FILL" : L"WRAP");
		return S_OK;
	}

	EExterior eExterior;
};

#include "EditToolMoveDlg.h"


class CEditToolMove :
	public CEditToolMouseInput<CEditToolMove>, // no direct tablet support
	public CEditToolCustomCursor<UINT(IDC_SIZEALL)>,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolMove, // T - the top level class for cross casting
		CEditToolMove, // TResetHandler
		CEditToolMove, // TDirtyHandler
		CEditToolMove, // TImageTileHandler
		CEditToolMove, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CEditToolMove, // TAdjustCoordsHandler
		CEditToolCustomCursor<UINT(IDC_SIZEALL)>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolMove>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CRasterImageEditToolBase // TControlLinesHandler
	>,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolCustomApply
{
public:
	CEditToolMove()
	{
	}

	BEGIN_COM_MAP(CEditToolMove)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IRasterImageEditToolCustomApply)
	END_COM_MAP()

	// IRasterImageEditToolCustomApply methods
public:
	STDMETHOD(ApplyChanges)(BOOL a_bExplicit)
	{
		try
		{
			if (m_cData.eExterior != CEditToolDataMove::EEFill)
				return E_NOTIMPL;
			CComPtr<IDocument> pDoc;
			M_Window()->Document(&pDoc);
			CComPtr<IDocumentEditableImage> pEI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
			if (pEI)
			{
				CUndoBlock cUndo(pDoc, _SharedStringTable.GetStringAuto(IDS_TOOLNAME_MOVE));
				TMatrix3x3f tMtx = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  m_nOffX, m_nOffY, 1.0f};
				pEI->CanvasSet(NULL, NULL, &tMtx, NULL);
				return S_OK;
			}
			return E_NOTIMPL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		RECT rc;
		if (_IsDirty(&rc, NULL, NULL) == S_OK)
		{
			m_nOffX = m_nOffY = 0;
			M_Window()->RectangleChanged(&rc);
		}
		else
		{
			m_nOffX = m_nOffY = 0;
		}
		return S_OK;
	}

	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
		{
			if (m_nOffX || m_nOffY)
			{
				BOOL b;
				a_pImageRect->top = a_pImageRect->left = 0;
				a_pImageRect->right = m_nSizeX;
				a_pImageRect->bottom = m_nSizeY;
				M_Window()->GetSelectionInfo(a_pImageRect, &b);
			}
			else
			{
				*a_pImageRect = RECT_EMPTY;
			}
		}
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = FALSE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		return m_nOffX || m_nOffY ? S_OK : S_FALSE;
	}
	HRESULT _GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return M_Window()->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	HRESULT _GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
	}
	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		if ((m_nOffX == 0 && m_nOffY == 0) || m_nSizeX == 0 || m_nSizeY == 0)
			return M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
		if (m_cData.eExterior == CEditToolDataMove::EEWrap)
		{
			ULONG nX = ((LONG(a_nX-m_nOffX)%LONG(m_nSizeX))+m_nSizeX)%m_nSizeX;
			ULONG nY = ((LONG(a_nY-m_nOffY)%LONG(m_nSizeY))+m_nSizeY)%m_nSizeY;
			if (nY+a_nSizeY > m_nSizeY)
			{
				ULONG n1 = m_nSizeY-nY;
				if (nX+a_nSizeX > m_nSizeX)
				{
					ULONG n2 = m_nSizeX-nX;
					M_Window()->GetImageTile(nX, nY, n2, n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
					M_Window()->GetImageTile(nX, 0, n2, a_nSizeY-n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer+a_nStride*n1);
					M_Window()->GetImageTile(0, nY, a_nSizeX-n2, n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer+n2);
					M_Window()->GetImageTile(0, 0, a_nSizeX-n2, a_nSizeY-n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer+a_nStride*n1+n2);
				}
				else
				{
					M_Window()->GetImageTile(nX, nY, a_nSizeX, n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
					M_Window()->GetImageTile(nX, 0, a_nSizeX, a_nSizeY-n1, a_fGamma, a_nStride, EITIBoth, a_pBuffer+a_nStride*n1);
				}
			}
			else
			{
				if (nX+a_nSizeX > m_nSizeX)
				{
					ULONG n2 = m_nSizeX-nX;
					M_Window()->GetImageTile(nX, nY, n2, a_nSizeY, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
					M_Window()->GetImageTile(0, nY, a_nSizeX-n2, a_nSizeY, a_fGamma, a_nStride, EITIBoth, a_pBuffer+n2);
				}
				else
				{
					M_Window()->GetImageTile(nX, nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
				}
			}
			return S_OK;
		}
		else // fill exterior
		{
			M_Window()->GetImageTile(a_nX-m_nOffX, a_nY-m_nOffY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBoth, a_pBuffer);
			return S_OK;
		}
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* UNREF(a_pControlPointIndex), float UNREF(a_fPointSize))
	{
		//if (a_pPos->fX < 0.0f)
		//	a_pPos->fX = 0.0f;
		//else if (int(a_pPos->fX) >= m_nSizeX)
		//	a_pPos->fX = m_nSizeX-1;
		//else
		//	a_pPos->fX = int(a_pPos->fX);

		//if (a_pPos->fY < 0.0f)
		//	a_pPos->fY = 0.0f;
		//else if (int(a_pPos->fY) >= m_nSizeY)
		//	a_pPos->fY = m_nSizeY-1;
		//else
		//	a_pPos->fY = int(a_pPos->fY);

		return S_OK;
	}

	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tDragLast = *a_pPos;
		return S_OK;
	}
	STDMETHOD(OnMouseUp)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		m_tDragLast = *a_pPos;
		//M_Window()->ApplyChanges();
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (!M_Dragging())
			return S_FALSE;
		LONG nDX = floorf(a_pPos->fX-m_tDragLast.fX+0.5f);
		LONG nDY = floorf(a_pPos->fY-m_tDragLast.fY+0.5f);
		if (nDX || nDY)
		{
			m_nOffX += nDX;
			m_nOffY += nDY;
			m_tDragLast = *a_pPos;
			RECT rc = {0, 0, m_nSizeX, m_nSizeY};
			M_Window()->RectangleChanged(&rc);
		}
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataMove::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			CEditToolDataMove ePrev = m_cData;
			m_cData = *(pData->InternalData());
			if ((m_nOffX || m_nOffY) && ePrev.eExterior != m_cData.eExterior)
			{
				RECT rc = {0, 0, m_nSizeX, m_nSizeY};
				M_Window()->RectangleChanged(&rc);
			}
		}
		return S_OK;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CEditToolDataMove::EExterior eExt = CEditToolDataMove::EEWrap;
			TColor tClr = {0.0f, 0.0f, 0.0f, 0.0f};
			float fOffX = 0.0f;
			float fOffY = 0.0f;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					real_p[assign_a(fOffX)]>>cSep>>
					real_p[assign_a(fOffY)]>>cSep>>
					((str_p(L"\"WRAP\"")[assign_a(eExt, CEditToolDataMove::EEWrap)])|
					(str_p(L"\"FILL\"")[assign_a(eExt, CEditToolDataMove::EEFill)]>>!(cSep>>real_p[assign_a(tClr.fR)]>>cSep>>real_p[assign_a(tClr.fG)]>>cSep>>real_p[assign_a(tClr.fB)]>>cSep>>real_p[assign_a(tClr.fA)])))
					>>*space_p).full;
			if (!bParsed)
				return E_INVALIDARG;
			bool const bStateChange = m_cData.eExterior != eExt;
			m_cData.eExterior = eExt;
			m_nOffX = fOffX+0.5f;
			m_nOffY = fOffY+0.5f;
			RECT rc = {0, 0, m_nSizeX, m_nSizeY};
			M_Window()->RectangleChanged(&rc);
			if (bStateChange)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		try
		{
			*a_pbstrParams = NULL;
			if (m_nOffX == 0 || m_nOffY == 0)
				return S_FALSE;
			CComBSTR bstr;
			OLECHAR sz[64];
			swprintf(sz, L"%i, %i, \"%s\"", m_nOffX, m_nOffY, m_cData.eExterior == CEditToolDataMove::EEWrap ? L"WRAP" : L"FILL");
			bstr = sz;
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	LONG m_nOffX;
	LONG m_nOffY;
	TPixelCoords m_tDragLast;
	CEditToolDataMove m_cData;
};

