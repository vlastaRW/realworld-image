
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
//#include "agg_blur.h"
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rounded_rect.h>
#include <agg_conv_stroke.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataBubble
{
	//MIDL_INTERFACE("0CB2EEB0-B5D4-40A1-A723-0B290ED0CDFA")
	//ISharedStateToolData : public ISharedState
	//{
	//public:
	//	STDMETHOD_(CEditToolDataBubble const*, InternalData)() = 0;
	//};

	CEditToolDataBubble()
	{
		tFill.fR = tFill.fG = tFill.fB = tFill.fA = 1.0f;
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		int nLen = SysStringLen(a_bstr);
		if (nLen < 12)
			return S_FALSE;
		LPCOLESTR psz = a_bstr;
		if (wcsncmp(a_bstr, L"BCx:", 4) == 0)
		{
			wchar_t sz[9];
			wcsncpy(sz, a_bstr+4, 8);
			sz[8] = L'\0';
			DWORD dw = 0;
			swscanf(sz, L"%x", &dw);
			tFill.fR = (dw&0xff)/255.0f;
			tFill.fG = ((dw>>8)&0xff)/255.0f;
			tFill.fB = ((dw>>16)&0xff)/255.0f;
			tFill.fA = ((dw>>24)&0xff)/255.0f;
			psz = a_bstr+12;
			nLen -= 12;
		}
		CLSID tID = GUID_NULL;
		if (nLen < 36 || !GUIDFromString(psz, &tID) || IsEqualGUID(tID, GUID_NULL))
			return S_OK;
		pInternal = NULL;
		RWCoCreateInstance(pInternal, tID);
		if (pInternal)
		{
			CComBSTR bstr(psz+36);
			pInternal->FromText(bstr);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t sz[13];
		DWORD dw = DWORD(tFill.fR*255.0f+0.5f)|(DWORD(tFill.fG*255.0f+0.5f)<<8)|
			(DWORD(tFill.fB*255.0f+0.5f)<<16)|(DWORD(tFill.fA*255.0f+0.5f)<<24);
		swprintf(sz, L"BCx:%08x", dw);
		CComBSTR bstr(sz);
		if (pInternal)
		{
			CLSID tID = GUID_NULL;
			pInternal->CLSIDGet(&tID);
			if (!IsEqualGUID(tID, GUID_NULL))
			{
				wchar_t szTmp[64];
				StringFromGUID(tID, szTmp);
				CComBSTR bstrInt;
				pInternal->ToText(&bstrInt);
				if (bstrInt.Length())
				{
					bstr += szTmp;
					bstr += bstrInt;
				}
			}
		}
		*a_pbstr = bstr;
		bstr.Detach();
		return S_OK;
	}

	TColor tFill;
	CComPtr<ISharedState> pInternal;
};

MIDL_INTERFACE("F7B33716-97EE-46A4-B53C-D32DE9A22242")
ISharedStateBubble : public ISharedState
{
	STDMETHOD_(CEditToolDataBubble const*, InternalData)() = 0;
};

// {3369238C-99E0-496a-84BE-C185BB26DE0D}
extern __declspec(selectany) GUID const CLSID_SharedStateBubble = {0x3369238c, 0x99e0, 0x496a, {0x84, 0xbe, 0xc1, 0x85, 0xbb, 0x26, 0xde, 0xd}};

// CSharedStateBubble

class ATL_NO_VTABLE CSharedStateBubble :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateBubble, &CLSID_SharedStateBubble>,
	public ISharedStateBubble
{
public:
	void Init(CEditToolDataBubble const& a_cData) { m_cData = a_cData; }

	DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CSharedStateBubble)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(ISharedStateBubble)
END_COM_MAP()

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID) { *a_pCLSID = CLSID_SharedStateBubble; return S_OK; }
	STDMETHOD(ToText)(BSTR* a_pbstrText) { return m_cData.ToString(a_pbstrText); }
	STDMETHOD(FromText)(BSTR a_bstrText) { return m_cData.FromString(a_bstrText); }

	// ISharedStateBubble methods
public:
    STDMETHOD_(CEditToolDataBubble const*, InternalData)() { return &m_cData; }

private:
	CEditToolDataBubble m_cData;
};

OBJECT_ENTRY_AUTO(CLSID_SharedStateBubble, CSharedStateBubble)


#include "EditToolBubbleDlg.h"


class CEditToolBubble :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditToolScripting,
	public IRasterImageEditWindow
{
public:
	CEditToolBubble() : m_pCallback(NULL), m_bShowPoint(false), m_bPointMoved(false),
		m_rcShape(RECT_EMPTY), m_rcBubble(RECT_EMPTY), m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth)
	{
	}
	~CEditToolBubble()
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
		CEditToolBubble* const p = reinterpret_cast<CEditToolBubble*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolBubble)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	class CRasterImageTarget
	{
	public:
		typedef agg::rgba8 color_type;
		typedef void row_data;
		typedef void span_data;

		CRasterImageTarget(TRasterImagePixel* a_pBuffer, unsigned a_nSizeX, unsigned a_nSizeY, unsigned a_nStride, TRasterImagePixel const* a_pShape, EBlendingMode a_eBlendingMode, BYTE const* a_pMask) :
			m_pBuffer(a_pBuffer), m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY), m_nStride(a_nStride), m_pShape(a_pShape), m_eBlendingMode(a_eBlendingMode), m_pMask(a_pMask)
		{
		}

		unsigned width() const
		{
			return m_nSizeX;
		}
		unsigned height() const
		{
			return m_nSizeY;
		}

		void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
		{
			if (m_pMask)
			{
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					CPixelMixerReplace::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], ULONG(cover)*m_pMask[y*m_nSizeX+x]/255);
					break;
				case EBMDrawOver:
					CPixelMixerPaintOver::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], ULONG(cover)*m_pMask[y*m_nSizeX+x]/255);
					break;
				case EBMDrawUnder:
					CPixelMixerPaintUnder::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], ULONG(cover)*m_pMask[y*m_nSizeX+x]/255);
					break;
				}
			}
			else
			{
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					CPixelMixerReplace::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], cover);
					break;
				case EBMDrawOver:
					CPixelMixerPaintOver::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], cover);
					break;
				case EBMDrawUnder:
					CPixelMixerPaintUnder::Mix(m_pBuffer[y*m_nStride+x], m_pShape[y*m_nSizeX+x], cover);
					break;
				}
			}
		}

		void blend_hline(int x, int y, unsigned len, color_type const& c, agg::int8u cover)
		{
			TRasterImagePixel const* pColor = m_pShape+y*m_nSizeX+x;
			TRasterImagePixel* p = m_pBuffer+y*m_nStride+x;
			if (m_pMask)
			{
				BYTE const* pMask = m_pMask+y*m_nSizeX+x;
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					for (; len; ++p, ++pColor, ++pMask, --len)
						CPixelMixerReplace::Mix(*p, *pColor, ULONG(cover)**pMask/255);
					break;
				case EBMDrawOver:
					for (; len; ++p, ++pColor, ++pMask, --len)
						CPixelMixerPaintOver::Mix(*p, *pColor, ULONG(cover)**pMask/255);
					break;
				case EBMDrawUnder:
					for (; len; ++p, ++pColor, ++pMask, --len)
						CPixelMixerPaintUnder::Mix(*p, *pColor, ULONG(cover)**pMask/255);
					break;
				}
			}
			else
			{
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					for (; len; ++p, ++pColor, --len)
						CPixelMixerReplace::Mix(*p, *pColor, cover);
					break;
				case EBMDrawOver:
					for (; len; ++p, ++pColor, --len)
						CPixelMixerPaintOver::Mix(*p, *pColor, cover);
					break;
				case EBMDrawUnder:
					for (; len; ++p, ++pColor, --len)
						CPixelMixerPaintUnder::Mix(*p, *pColor, cover);
					break;
				}
			}
		}

		void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)
		{
			TRasterImagePixel const* pColor = m_pShape+y*m_nSizeX+x;
			TRasterImagePixel* p = m_pBuffer+y*m_nStride+x;
			if (m_pMask)
			{
				BYTE const* pMask = m_pMask+y*m_nSizeX+x;
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					for (; len; ++p, ++pColor, ++covers, ++pMask, --len)
						CPixelMixerReplace::Mix(*p, *pColor, ULONG(*covers)**pMask/255);
					break;
				case EBMDrawOver:
					for (; len; ++p, ++pColor, ++covers, ++pMask, --len)
						CPixelMixerPaintOver::Mix(*p, *pColor, ULONG(*covers)**pMask/255);
					break;
				case EBMDrawUnder:
					for (; len; ++p, ++pColor, ++covers, ++pMask, --len)
						CPixelMixerPaintUnder::Mix(*p, *pColor, ULONG(*covers)**pMask/255);
					break;
				}
			}
			else
			{
				switch (m_eBlendingMode)
				{
				case EBMReplace:
					for (; len; ++p, ++pColor, ++covers, --len)
						CPixelMixerReplace::Mix(*p, *pColor, *covers);
					break;
				case EBMDrawOver:
					for (; len; ++p, ++pColor, ++covers, --len)
						CPixelMixerPaintOver::Mix(*p, *pColor, *covers);
					break;
				case EBMDrawUnder:
					for (; len; ++p, ++pColor, ++covers, --len)
						CPixelMixerPaintUnder::Mix(*p, *pColor, *covers);
					break;
				}
			}
		}

	private:
		TRasterImagePixel* m_pBuffer;
		unsigned m_nSizeX;
		unsigned m_nSizeY;
		unsigned m_nStride;
		TRasterImagePixel const* m_pShape;
		EBlendingMode m_eBlendingMode;
		BYTE const* m_pMask;
	};

	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			HRESULT hRes = m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
			if (FAILED(hRes)) return hRes;

			if (m_rcShape.left >= m_rcShape.right || m_rcShape.top >= m_rcShape.bottom)
				return hRes; // internal tool has not shape to draw

			RECT const rcRedraw = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};

			RECT rcFinal =
			{
				max(rcRedraw.left, m_rcDirty.left),
				max(rcRedraw.top, m_rcDirty.top),
				min(rcRedraw.right, m_rcDirty.right),
				min(rcRedraw.bottom, m_rcDirty.bottom),
			};

			if (rcFinal.left >= rcFinal.right || rcFinal.top >= rcFinal.bottom)
				return hRes; // not redrawing region affected by the bubble

			RECT rcSel = rcFinal;
			BOOL bSelSolid = FALSE;
			m_pWindow->GetSelectionInfo(&rcSel, &bSelSolid);

			if (rcFinal.left < rcSel.left) rcFinal.left = rcSel.left;
			if (rcFinal.top < rcSel.top) rcFinal.top = rcSel.top;
			if (rcFinal.right > rcSel.right) rcFinal.right = rcSel.right;
			if (rcFinal.bottom > rcSel.bottom) rcFinal.bottom = rcSel.bottom;

			if (rcFinal.left >= rcFinal.right || rcFinal.top >= rcFinal.bottom)
				return hRes; // not redrawing region affected by the bubble (due to selection)

			SIZE const szFinal = {rcFinal.right-rcFinal.left ,rcFinal.bottom-rcFinal.top};
			ULONG const nFinal = szFinal.cx*szFinal.cy;
			CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nFinal]);
			hRes = m_pTool->GetImageTile(rcFinal.left, rcFinal.top, szFinal.cx, szFinal.cy, a_fGamma, szFinal.cx, cBuffer);
			if (FAILED(hRes)) return hRes;

			CAutoVectorPtr<BYTE> cMask;
			if (bSelSolid == FALSE)
			{
				cMask.Allocate(nFinal);
				m_pWindow->GetSelectionTile(rcFinal.left, rcFinal.top, szFinal.cx, szFinal.cy, szFinal.cx, cMask);
			}

			// draw the bubble shape and combine the buffers
			CRasterImageTarget cDst(a_pBuffer+rcFinal.left-a_nX+(rcFinal.top-a_nY)*a_nStride, szFinal.cx, szFinal.cy, a_nStride, cBuffer, m_eBlendingMode, cMask);
			agg::renderer_base<CRasterImageTarget> renb(cDst);
			agg::scanline_p8 sl;
			agg::rasterizer_scanline_aa<> ras;
			agg::rounded_rect rect(m_rcBubble.left-rcFinal.left, m_rcBubble.top-rcFinal.top, m_rcBubble.right-rcFinal.left, m_rcBubble.bottom-rcFinal.top, m_nRadius);
			ras.add_path(rect);
			RECT rcShape = m_rcShape;
			TPixelCoords tCenter = {(rcShape.right+rcShape.left)*0.5f, (rcShape.bottom+rcShape.top)*0.5f};
			TPixelCoords tDelta = {tCenter.fY-m_tPoint.fY, m_tPoint.fX-tCenter.fX};
			float fFact = min(rcShape.right-rcShape.left, rcShape.bottom-rcShape.top)*0.4/*m_nRadius*1.5f*//sqrtf(tDelta.fX*tDelta.fX + tDelta.fY*tDelta.fY);
			tDelta.fX *= fFact; tDelta.fY *= fFact;
			ras.move_to_d(m_tPoint.fX-rcFinal.left, m_tPoint.fY-rcFinal.top);
			ras.line_to_d(tCenter.fX-rcFinal.left+tDelta.fX, tCenter.fY-rcFinal.top+tDelta.fY);
			ras.line_to_d(tCenter.fX-rcFinal.left-tDelta.fX, tCenter.fY-rcFinal.top-tDelta.fY);
			ras.close_polygon();
			if (m_eRasterizationMode == ERMSmooth)
			{
				agg::renderer_scanline_aa_solid<agg::renderer_base<CRasterImageTarget> > ren(renb);
				//ren.color(agg::rgba8(m_cData.tFill.fR*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fG*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fB*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fA*255.0f + 0.5f));
				agg::render_scanlines(ras, sl, ren);
			}
			else
			{
				agg::renderer_scanline_bin_solid<agg::renderer_base<CRasterImageTarget> > ren(renb);
				//ren.color(agg::rgba8(m_cData.tFill.fR*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fG*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fB*m_cData.tFill.fA*255.0f + 0.5f, m_cData.tFill.fA*255.0f + 0.5f));
				ras.gamma(agg::gamma_threshold(0.5));
				agg::render_scanlines(ras, sl, ren);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
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
		CComQIPtr<ISharedStateBubble> pSSB(a_pState);
		if (pSSB == NULL)
			return E_FAIL;
		TColor tFill = m_cData.tFill;
		m_cData = *pSSB->InternalData();
		if ((tFill.fR != m_cData.tFill.fR || tFill.fG != m_cData.tFill.fG ||
			tFill.fB != m_cData.tFill.fB || tFill.fA != m_cData.tFill.fA) &&
			m_pTool->IsDirty(NULL, NULL, NULL) == S_OK)
		{
			m_pWindow->RectangleChanged(&M_DirtyRect());
		}
		if (m_cData.pInternal)
			m_pTool->SetState(m_cData.pInternal);
		return S_OK;
	}
	STDMETHOD(SetOutline)(BYTE a_bOutline, float a_fOutlineWidth, float a_fOutlinePos, EOutlineJoinType a_eOutlineJoins, TColor const* a_pOutlineColor)
	{
		return m_pTool->SetOutline(a_bOutline, a_fOutlineWidth, a_fOutlinePos, a_eOutlineJoins, a_pOutlineColor);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode || m_eRasterizationMode != a_eRasterizationMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			m_eRasterizationMode = a_eRasterizationMode;
			if (m_rcDirty.left < m_rcDirty.right && m_rcDirty.top < m_rcDirty.bottom)
				m_pWindow->RectangleChanged(&m_rcDirty);
		}
		return m_pTool->SetGlobals(EBMReplace, a_eRasterizationMode, a_eCoordinatesMode);
	}

	STDMETHOD(Reset)()
	{
		m_bShowPoint = false;
		m_pWindow->ControlPointsChanged();
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
		HRESULT hRes = m_pTool->IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect);
		if (a_pImageRect)
		{
			UpdateBubbleRect(*a_pImageRect);
			*a_pImageRect = m_rcDirty;
		}
		return hRes;
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		if (a_pControlPointIndex && m_bShowPoint)
		{
			if (*a_pControlPointIndex == 0)
			{
				return S_OK;
			}
			ULONG n = *a_pControlPointIndex-1;
			return m_pTool->AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, &n, a_fPointSize);
		}

		return m_pTool->AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		return m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		return m_pTool->GetCursor(static_cast<EControlKeysState>(a_eKeysState&(~ECKSControl)), a_pPos, a_phCursor);
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		HRESULT hRes = m_pTool->GetControlPointCount(a_pCount);
		if (m_bShowPoint)
		{
			if (a_pCount)
				++*a_pCount;
		}
		return hRes;
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (m_bShowPoint)
		{
			if (a_nIndex == 0)
			{
				if (a_pPos)
				{
					*a_pPos = m_tPoint;
				}
				if (a_pClass)
					*a_pClass = 0x11;
				return S_OK;
			}
			return m_pTool->GetControlPoint(a_nIndex-1, a_pPos, a_pClass);
		}

		return m_pTool->GetControlPoint(a_nIndex, a_pPos, a_pClass);
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		if (m_bShowPoint)
		{
			if (a_nIndex == 0)
			{
				m_tPoint = *a_pPos;
				m_pWindow->ControlPointChanged(0);
				RECT rcDirty = m_rcDirty;
				RECT rcPt = {floorf(m_tPoint.fX), floorf(m_tPoint.fY), ceilf(m_tPoint.fX), ceilf(m_tPoint.fY)};
				if (rcDirty.left > rcPt.left) rcDirty.left = rcPt.left;
				if (rcDirty.top > rcPt.top) rcDirty.top = rcPt.top;
				if (rcDirty.right < rcPt.right) rcDirty.right = rcPt.right;
				if (rcDirty.bottom < rcPt.bottom) rcDirty.bottom = rcPt.bottom;
				m_rcDirty = rcDirty;
				return m_pWindow->RectangleChanged(&rcDirty);
			}
			return m_pTool->SetControlPoint(a_nIndex-1, a_pPos, a_bFinished, a_fPointSize);
		}

		return m_pTool->SetControlPoint(a_nIndex, a_pPos, a_bFinished, a_fPointSize);
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		if (m_bShowPoint)
		{
			if (a_nIndex == 0)
				return E_NOTIMPL;
			return m_pTool->GetControlPointDesc(a_nIndex-1, a_ppDescription);
		}
		return m_pTool->GetControlPointDesc(a_nIndex, a_ppDescription);
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		return m_pTool->GetControlLines(a_pLines, a_nLineTypes);
	}
	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE a_bAccurate, float a_fPointSize)
	{
		return m_pTool->PointTest(a_eKeysState, a_pPos, a_bAccurate, a_fPointSize);
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		if (a_pMatrix)
		{
			TVector2f a = {m_tPoint.fX, m_tPoint.fY};
			TVector2f b = TransformVector2(*a_pMatrix, a);
			m_tPoint.fX = b.x;
			m_tPoint.fY = b.y;
		}
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
		return m_pWindow->GetDefaultColor(a_pDefault);
	}
	STDMETHOD(IRasterImageEditWindow::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		switch (a_eIntent)
		{
		case EITIBackground:
			{
				TRasterImagePixel t;
				t.bR = m_cData.tFill.fR < 0.0f ? 0 : (m_cData.tFill.fR > 1.0f ? 255 : m_cData.tFill.fR*255.0f+0.5f);
				t.bG = m_cData.tFill.fG < 0.0f ? 0 : (m_cData.tFill.fG > 1.0f ? 255 : m_cData.tFill.fG*255.0f+0.5f);
				t.bB = m_cData.tFill.fB < 0.0f ? 0 : (m_cData.tFill.fB > 1.0f ? 255 : m_cData.tFill.fB*255.0f+0.5f);
				t.bA = m_cData.tFill.fA < 0.0f ? 0 : (m_cData.tFill.fA > 1.0f ? 255 : m_cData.tFill.fA*255.0f+0.5f);
				for (ULONG y = 0; y < a_nSizeY; ++y)
					std::fill_n(a_pBuffer+a_nStride*y, a_nSizeX, t);
			}
			return S_OK;
		case EITIContent:
			return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_eIntent, a_pBuffer);
		case EITIBoth:
		default:
			return E_FAIL;
		}
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_pBoundingRectangle)
		{
			a_pBoundingRectangle->left = 0;
			a_pBoundingRectangle->top = 0;
			m_pWindow->Size(reinterpret_cast<ULONG*>(&(a_pBoundingRectangle->right)), reinterpret_cast<ULONG*>(&(a_pBoundingRectangle->bottom)));
		}
		if (a_bEntireRectangle)
			*a_bEntireRectangle = TRUE;
		return S_OK;
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		for (ULONG y = 0; y < a_nSizeY; ++y)
			FillMemory(a_pBuffer+y*a_nStride, a_nSizeX, 0xff);
		return S_OK;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return m_pWindow->ControlPointsChanged();
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return m_pWindow->ControlPointChanged(m_bShowPoint ? a_nIndex+1 : a_nIndex);
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return m_pWindow->ControlLinesChanged();
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		RECT rcPrev = M_DirtyRect();
		RECT rc = RECT_EMPTY;
		if (m_pTool->IsDirty(&rc, NULL, NULL) == S_OK)
		{
			if (!m_bShowPoint)
			{
				m_bShowPoint = true;
				m_tPoint.fX = rc.right*1.2f-rc.left*0.2;
				m_tPoint.fY = rc.bottom*1.4f-rc.top*0.4;
				ULONG nX = 0;
				ULONG nY = 0;
				m_pWindow->Size(&nX, &nY);
				if (m_tPoint.fX < 0.0f) m_tPoint.fX = 0.0f;
				else if (m_tPoint.fX > nX) m_tPoint.fX = nX;
				if (m_tPoint.fY < 0.0f) m_tPoint.fY = 0.0f;
				else if (m_tPoint.fY > nY) m_tPoint.fY = nY;
				m_pWindow->ControlPointsChanged();
			}
		}
		else
		{
			if (m_bShowPoint)
			{
				m_bShowPoint = false;
				m_pWindow->ControlPointsChanged();
			}
		}
		RECT rcPrev2 = m_rcBubble;
		UpdateBubbleRect(rc);
		if (rcPrev2.left == m_rcBubble.left && rcPrev2.right == m_rcBubble.right &&
			rcPrev2.top == m_rcBubble.top && rcPrev2.bottom == m_rcBubble.bottom)
		{
			return m_pWindow->RectangleChanged(a_pChanged);
		}
		rc = M_DirtyRect();
		if (rc.left > rcPrev.left) rc.left = rcPrev.left;
		if (rc.top > rcPrev.top) rc.top = rcPrev.top;
		if (rc.right < rcPrev.right) rc.right = rcPrev.right;
		if (rc.bottom < rcPrev.bottom) rc.bottom = rcPrev.bottom;
		return m_pWindow->RectangleChanged(&rc);
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		m_cData.pInternal = a_pState;
		CComObject<CSharedStateBubble>* p = NULL;
		CComObject<CSharedStateBubble>::CreateInstance(&p);
		CComPtr<ISharedState> pState = p;
		p->Init(m_cData);
		return m_pWindow->SetState(pState);
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
		return m_pWindow->Document(a_ppDocument);
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CComQIPtr<IRasterImageEditToolScripting> pInternal(m_pTool);
			if (pInternal == NULL)
				return S_FALSE;

			TColor tClr = {0.0f, 0.0f, 0.0f, 0.0f};
			float fPtX = 0.0f;
			float fPtY = 0.0f;
			std::wstring strCmd;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					real_p[assign_a(fPtX)]>>cSep>>
					real_p[assign_a(fPtY)]>>cSep>>
					real_p[assign_a(tClr.fR)]>>cSep>>
					real_p[assign_a(tClr.fG)]>>cSep>>
					real_p[assign_a(tClr.fB)]>>cSep>>
					real_p[assign_a(tClr.fA)]>>
					(!(cSep>>(*anychar_p)[assign_a(strCmd)]))
					).full;
			if (!bParsed)
				return E_INVALIDARG;
			bool bClrChange = fabsf(m_cData.tFill.fR-tClr.fR) > 0.0001f || fabsf(m_cData.tFill.fG-tClr.fG) > 0.0001f ||
				fabsf(m_cData.tFill.fB-tClr.fB) > 0.0001f || fabsf(m_cData.tFill.fA-tClr.fA) > 0.0001f;
			m_cData.tFill = tClr;
			bool bPtChange = fabsf(m_tPoint.fX-fPtX) > 0.0001f || fabsf(m_tPoint.fY-fPtY) > 0.0001f;
			m_tPoint.fX = fPtX;
			m_tPoint.fY = fPtY;
			if (bPtChange || !m_bShowPoint)
			{
				if (m_bShowPoint)
					m_pWindow->ControlPointChanged(0);
				else
				{
					m_bShowPoint = true;
					m_pWindow->ControlPointsChanged();
				}
			}
			if (bClrChange)
			{
				CComPtr<ISharedState> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(SharedStateString));
				CComBSTR bstr;
				m_cData.ToString(&bstr);
				pTmp->FromText(bstr);
				m_pWindow->SetState(pTmp);
			}
			if (bClrChange || bPtChange)
			{
				RECT rcDirty = m_rcDirty;
				RECT rcPt = {floorf(m_tPoint.fX), floorf(m_tPoint.fY), ceilf(m_tPoint.fX), ceilf(m_tPoint.fY)};
				if (rcDirty.left > rcPt.left) rcDirty.left = rcPt.left;
				if (rcDirty.top > rcPt.top) rcDirty.top = rcPt.top;
				if (rcDirty.right < rcPt.right) rcDirty.right = rcPt.right;
				if (rcDirty.bottom < rcPt.bottom) rcDirty.bottom = rcPt.bottom;
				m_rcDirty = rcDirty;
				m_pWindow->RectangleChanged(&rcDirty);
			}
			return pInternal->FromText(CComBSTR(strCmd.c_str()));
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
			CComQIPtr<IRasterImageEditToolScripting> pInternal(m_pTool);
			if (pInternal == NULL)
				return S_FALSE;
			CComBSTR bstrInt;
			if (S_OK != pInternal->ToText(&bstrInt))
				return S_FALSE;
			CComBSTR bstr;
			OLECHAR sz[128];
			swprintf(sz, L"%g, %g, %g, %g, %g, %g", m_tPoint.fX, m_tPoint.fY, m_cData.tFill.fR, m_cData.tFill.fG, m_cData.tFill.fB, m_cData.tFill.fA);
			bstr = sz;
			if (bstrInt.Length())
			{
				bstr += L", ";
				bstr += bstrInt;
			}
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	void UpdateBubbleRect(RECT const& a_rcShape)
	{
		if (a_rcShape.left >= a_rcShape.right || a_rcShape.top >= a_rcShape.bottom)
		{
			m_rcShape = m_rcBubble = RECT_EMPTY;
		}
		else
		{
			m_rcShape = a_rcShape;
			LONG nDX = a_rcShape.right-a_rcShape.left;
			LONG nDY = a_rcShape.bottom-a_rcShape.top;
			LONG nD = min(nDX, nDY);
			m_nRadius = 2 + sqrtf(nD);
			m_rcBubble.left = a_rcShape.left-m_nRadius;
			m_rcBubble.top = a_rcShape.top-m_nRadius;
			m_rcBubble.right = a_rcShape.right+m_nRadius;
			m_rcBubble.bottom = a_rcShape.bottom+m_nRadius;
			RECT rcPt = {floorf(m_tPoint.fX), floorf(m_tPoint.fY), ceilf(m_tPoint.fX), ceilf(m_tPoint.fY)};
			m_rcDirty = m_rcBubble;
			if (m_rcDirty.left > rcPt.left) m_rcDirty.left = rcPt.left;
			if (m_rcDirty.top > rcPt.top) m_rcDirty.top = rcPt.top;
			if (m_rcDirty.right < rcPt.right) m_rcDirty.right = rcPt.right;
			if (m_rcDirty.bottom < rcPt.bottom) m_rcDirty.bottom = rcPt.bottom;
		}
	}
	RECT M_DirtyRect()
	{
		return m_rcDirty;
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	CEditToolDataBubble m_cData;
	RECT m_rcShape;
	RECT m_rcBubble;
	RECT m_rcDirty;
	int m_nRadius;
	TPixelCoords m_tPoint;
	bool m_bShowPoint;
	bool m_bPointMoved;
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
};

