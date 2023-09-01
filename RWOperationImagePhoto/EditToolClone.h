
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
#include "agg_blur.h"
#include "../RWDrawing/ConstantRasterImageBrush.h"


struct CEditToolDataClone
{
	MIDL_INTERFACE("30DE75A3-4DB6-40FF-94A4-E15921AC5BA7")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataClone const*, InternalData)() = 0;
	};

	enum EMode
	{
		EMStandard,
		EMHighFrequency,
		EMLowFrequency,
	};

	CEditToolDataClone() : eMode(EMStandard), bRelative(true)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		int nLen = SysStringLen(a_bstr);
		if (nLen < 3)
			return S_FALSE;
		if (wcsncmp(a_bstr, L"HF", 2) == 0)
			eMode = EMHighFrequency;
		else if (wcsncmp(a_bstr, L"LF", 2) == 0)
			eMode = EMLowFrequency;
		else if (wcsncmp(a_bstr, L"CL", 2) == 0)
			eMode = EMStandard;
		if (a_bstr[2] == L'R')
			bRelative = true;
		else if (a_bstr[2] == L'A')
			bRelative = false;
		CLSID tID = GUID_NULL;
		if (nLen < 39 || !GUIDFromString(a_bstr+3, &tID) || IsEqualGUID(tID, GUID_NULL))
			return S_OK;
		pInternal = NULL;
		RWCoCreateInstance(pInternal, tID);
		if (pInternal)
		{
			CComBSTR bstr(a_bstr+39);
			pInternal->FromText(bstr);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		CComBSTR bstr(eMode == EMHighFrequency ? (bRelative ? L"HFR" : L"HFA") : (eMode == EMLowFrequency ? (bRelative ? L"LFR" : L"LFA") : (bRelative ? L"CLR" : L"CLA")));
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

	EMode eMode;
	bool bRelative;
	CComPtr<ISharedState> pInternal;
};

#include "SharedStateEditToolClone.h"
#include "EditToolCloneDlg.h"


HICON GetToolIconCLONE(ULONG a_nSize);

class CEditToolClone :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CEditToolCustomPrecisionCursor<GetToolIconCLONE>,
	public IRasterImageEditTool,
	public IRasterImageEditWindow
{
public:
	CEditToolClone() : m_pCallback(NULL), m_nOffsetX(40), m_nOffsetY(40), m_bDragging(false),
		m_bDefining(false), m_bShowPoint(false), m_nLastPosX(-10000), m_nLastPosY(-10000),
		m_nAbsoluteRefX(0), m_nAbsoluteRefY(0), m_bAbsLock(false), m_bCtrlDragging(false),
		m_bMouseLeft(false)
	{
	}
	~CEditToolClone()
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
		CEditToolClone* const p = reinterpret_cast<CEditToolClone*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolClone)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		RECT rcDirty;
		if (S_OK != m_pTool->IsDirty(&rcDirty, NULL, NULL))
			return S_OK;

		ULONG nImgSizeX = 0;
		ULONG nImgSizeY = 0;
		m_pWindow->Size(&nImgSizeX, &nImgSizeY);

		RECT rcUpdate = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		if (rcDirty.left < rcUpdate.left) rcDirty.left = rcUpdate.left;
		if (rcDirty.top < rcUpdate.top) rcDirty.top = rcUpdate.top;
		if (rcDirty.right > rcUpdate.right) rcDirty.right = rcUpdate.right;
		if (rcDirty.bottom > rcUpdate.bottom) rcDirty.bottom = rcUpdate.bottom;
		RECT rcSrc =
		{
			max(0, rcDirty.left+m_nOffsetX),
			max(0, rcDirty.top+m_nOffsetY),
			min(LONG(nImgSizeX), rcDirty.right+m_nOffsetX),
			min(LONG(nImgSizeY), rcDirty.bottom+m_nOffsetY)
		};
		if (rcSrc.left >= rcSrc.right || rcSrc.top >= rcSrc.bottom)
			return S_OK; // nothing to draw
		RECT rcCov = {rcSrc.left-m_nOffsetX, rcSrc.top-m_nOffsetY, rcSrc.right-m_nOffsetX, rcSrc.bottom-m_nOffsetY};
		SIZE sz = {rcSrc.right-rcSrc.left, rcSrc.bottom-rcSrc.top};
		CAutoVectorPtr<TRasterImagePixel> cCov(new TRasterImagePixel[sz.cx*sz.cy]);
		m_pTool->GetImageTile(rcCov.left, rcCov.top, sz.cx, sz.cy, a_fGamma, sz.cx, cCov);
		if (m_cData.eMode == CEditToolDataClone::EMStandard)
		{
			// standard cloning
			CAutoVectorPtr<TRasterImagePixel> cSrc(new TRasterImagePixel[sz.cx*sz.cy]);
			m_pWindow->GetImageTile(rcSrc.left, rcSrc.top, sz.cx, sz.cy, a_fGamma, sz.cx, EITIContent, cSrc);
			TRasterImagePixel const* pS = cSrc;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pS, ++pC, ++pD)
				{
					if (pC->bA == 0)
						continue;
					CPixelMixerPaintOver::Mix(*pD, *pS, pC->bA);
				}
				pD += a_nStride-sz.cx;
			}
		}
		else
		{
			// copy high or low frequencies only (needs 5 pixels on each side of source and destination)
			static int const BLURRADIUS = 5;
			RECT rcSrcEx =
			{
				rcSrc.left-BLURRADIUS,
				rcSrc.top-BLURRADIUS,
				rcSrc.right+BLURRADIUS,
				rcSrc.bottom+BLURRADIUS
			};
			RECT rcSrcExClipped =
			{
				max(rcSrcEx.left, 0),
				max(rcSrcEx.top, 0),
				min(rcSrcEx.right, LONG(nImgSizeX)),
				min(rcSrcEx.bottom, LONG(nImgSizeY))
			};
			SIZE szEx = {sz.cx+BLURRADIUS+BLURRADIUS, sz.cy+BLURRADIUS+BLURRADIUS};
			CAutoVectorPtr<TRasterImagePixel> cSrc(new TRasterImagePixel[szEx.cx*szEx.cy]);
			ZeroMemory(cSrc.m_p, szEx.cx*szEx.cy*sizeof*cSrc.m_p);
			m_pWindow->GetImageTile(rcSrcExClipped.left, rcSrcExClipped.top, rcSrcExClipped.right-rcSrcExClipped.left, rcSrcExClipped.bottom-rcSrcExClipped.top, a_fGamma, szEx.cx, EITIContent, cSrc.m_p+(rcSrcExClipped.top-rcSrcEx.top)*szEx.cx+rcSrcExClipped.left-rcSrcEx.left);
			// premultiply
			TRasterImagePixel* pEnd = cSrc.m_p+szEx.cx*szEx.cy;
			for (TRasterImagePixel* p = cSrc.m_p; p != pEnd; ++p)
			{
				p->bR = ((ULONG(p->bR)+(p->bR>>7))*p->bA)>>8;
				p->bG = ((ULONG(p->bG)+(p->bG>>7))*p->bA)>>8;
				p->bB = ((ULONG(p->bB)+(p->bB>>7))*p->bA)>>8;
			}
			// blur
			CAutoVectorPtr<TRasterImagePixel> cSrcB(new TRasterImagePixel[szEx.cx*szEx.cy]);
			CopyMemory(cSrcB.m_p, cSrc.m_p, szEx.cx*szEx.cy*sizeof*cSrc.m_p);
			agg::rendering_buffer rbuf(reinterpret_cast<agg::int8u*>(cSrcB.m_p), szEx.cx, szEx.cy, szEx.cx*sizeof(TRasterImagePixel));
			agg::pixfmt_rgba32 pixf(rbuf);
			agg::stack_blur_rgba32(pixf, BLURRADIUS, BLURRADIUS);

			RECT rcDstEx =
			{
				rcCov.left-BLURRADIUS,
				rcCov.top-BLURRADIUS,
				rcCov.right+BLURRADIUS,
				rcCov.bottom+BLURRADIUS
			};
			RECT rcDstExClipped =
			{
				max(rcDstEx.left, 0),
				max(rcDstEx.top, 0),
				min(rcDstEx.right, LONG(nImgSizeX)),
				min(rcDstEx.bottom, LONG(nImgSizeY))
			};
			CAutoVectorPtr<TRasterImagePixel> cDst(new TRasterImagePixel[szEx.cx*szEx.cy]); // need destination for blurring
			ZeroMemory(cDst.m_p, szEx.cx*szEx.cy*sizeof*cDst.m_p);
			m_pWindow->GetImageTile(rcDstExClipped.left, rcDstExClipped.top, rcDstExClipped.right-rcDstExClipped.left, rcDstExClipped.bottom-rcDstExClipped.top, a_fGamma, szEx.cx, EITIContent, cDst.m_p+(rcDstExClipped.top-rcDstEx.top)*szEx.cx+rcDstExClipped.left-rcDstEx.left);
			// premultiply
			pEnd = cDst.m_p+szEx.cx*szEx.cy;
			for (TRasterImagePixel* p = cDst.m_p; p != pEnd; ++p)
			{
				p->bR = ((ULONG(p->bR)+(p->bR>>7))*p->bA)>>8;
				p->bG = ((ULONG(p->bG)+(p->bG>>7))*p->bA)>>8;
				p->bB = ((ULONG(p->bB)+(p->bB>>7))*p->bA)>>8;
			}
			// blur
			CAutoVectorPtr<TRasterImagePixel> cDstB(new TRasterImagePixel[szEx.cx*szEx.cy]); // need destination for blurring
			CopyMemory(cDstB.m_p, cDst.m_p, szEx.cx*szEx.cy*sizeof*cSrc.m_p);
			rbuf.attach(reinterpret_cast<agg::int8u*>(cDstB.m_p), szEx.cx, szEx.cy, szEx.cx*sizeof(TRasterImagePixel));
			pixf.attach(rbuf);
			agg::stack_blur_rgba32(pixf, BLURRADIUS, BLURRADIUS);

			TRasterImagePixel const* pSO = cSrc.m_p+BLURRADIUS+szEx.cx*BLURRADIUS;
			TRasterImagePixel const* pSB = cSrcB.m_p+BLURRADIUS+szEx.cx*BLURRADIUS;
			TRasterImagePixel const* pDO = cDst.m_p+BLURRADIUS+szEx.cx*BLURRADIUS;
			TRasterImagePixel const* pDB = cDstB.m_p+BLURRADIUS+szEx.cx*BLURRADIUS;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				if (m_cData.eMode == CEditToolDataClone::EMHighFrequency)
				{
					// copy high frequencies
					for (LONG x = 0; x < sz.cx; ++x, ++pSO, ++pDO, ++pSB, ++pDB, ++pC, ++pD)
					{
						if (pC->bA == 0)
							continue;
						int nR = pDB->bR + ((int(pSO->bR)-int(pSB->bR))*pC->bA + (int(pDO->bR)-int(pDB->bR))*(255-pC->bA))/255;
						int nG = pDB->bG + ((int(pSO->bG)-int(pSB->bG))*pC->bA + (int(pDO->bG)-int(pDB->bG))*(255-pC->bA))/255;
						int nB = pDB->bB + ((int(pSO->bB)-int(pSB->bB))*pC->bA + (int(pDO->bB)-int(pDB->bB))*(255-pC->bA))/255;
						int nA = pDB->bA + ((int(pSO->bA)-int(pSB->bA))*pC->bA + (int(pDO->bA)-int(pDB->bA))*(255-pC->bA))/255;
						if (nA > 0)
						{
							nR = nR*255/nA;
							nG = nG*255/nA;
							nB = nB*255/nA;
							pD->bR = min(255, max(nR, 0));
							pD->bG = min(255, max(nG, 0));
							pD->bB = min(255, max(nB, 0));
							pD->bA = min(255, nA);
						}
						else
						{
							pD->bR = pD->bG = pD->bB = pD->bA = 0;
						}
					}
				}
				else
				{
					// copy low frequencies
					for (LONG x = 0; x < sz.cx; ++x, ++pSO, ++pDO, ++pSB, ++pDB, ++pC, ++pD)
					{
						if (pC->bA == 0)
							continue;
						int nR = int(pDO->bR)-int(pDB->bR) + (int(pSB->bR)*pC->bA + int(pDB->bR)*(255-pC->bA))/255;
						int nG = int(pDO->bG)-int(pDB->bG) + (int(pSB->bG)*pC->bA + int(pDB->bG)*(255-pC->bA))/255;
						int nB = int(pDO->bB)-int(pDB->bB) + (int(pSB->bB)*pC->bA + int(pDB->bB)*(255-pC->bA))/255;
						int nA = int(pDO->bA)-int(pDB->bA) + (int(pSB->bA)*pC->bA + int(pDB->bA)*(255-pC->bA))/255;
						if (nA > 0)
						{
							nR = nR*255/nA;
							nG = nG*255/nA;
							nB = nB*255/nA;
							pD->bR = min(255, max(nR, 0));
							pD->bG = min(255, max(nG, 0));
							pD->bB = min(255, max(nB, 0));
							pD->bA = min(255, nA);
						}
						else
						{
							pD->bR = pD->bG = pD->bB = pD->bA = 0;
						}
					}
				}
				pD += a_nStride-sz.cx;
				pSO += BLURRADIUS+BLURRADIUS;
				pDO += BLURRADIUS+BLURRADIUS;
				pSB += BLURRADIUS+BLURRADIUS;
				pDB += BLURRADIUS+BLURRADIUS;
			}
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
		CComObject<CConstantRasterImageBrush>* p = NULL;
		CComObject<CConstantRasterImageBrush>::CreateInstance(&p);
		CComPtr<IRasterImageBrush> pBr = p;
		static TColor const tClr = {0.0f, 0.0f, 0.0f, 1.0f};
		p->Init(tClr);
		m_pTool->SetBrush(pBr);
		ULONG nX = 0;
		ULONG nY = 0;
		m_pWindow->Size(&nX, &nY);
		m_nAbsoluteRefX = nX>>1;
		m_nAbsoluteRefY = nY>>1;
		m_nOffsetX = nX>>3;
		m_nOffsetY = nY>>3;
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataClone::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			// TODO: update window if internal tool dirty
			m_cData = *(pData->InternalData());
			m_pTool->SetState(m_cData.pInternal);
		}
		return S_OK;
	}
	STDMETHOD(IRasterImageEditTool::SetOutline)(BYTE, float, float, EOutlineJoinType, TColor const*)
	{
		return S_FALSE;
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return S_FALSE;//m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		return m_pTool->SetGlobals(EBMReplace/*a_eBlendingMode*/, a_eRasterizationMode, a_eCoordinatesMode);
		//m_eBlendingMode = a_eBlendingMode;
		//m_eRasterizationMode = a_eRasterizationMode;
		//m_eCoordinatesMode = a_eCoordinatesMode;
		////, EShapeFillMode a_eShapeFillMode
		//return S_OK;
	}

	STDMETHOD(Reset)()
	{
		m_bAbsLock = false;
		m_pWindow->ControlLinesChanged();
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
		if (a_pPos == NULL)
		{
			if (!m_bMouseLeft)
			{
				m_bMouseLeft = true;
				m_pWindow->ControlLinesChanged();
			}
			return m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
		}
		if (m_bMouseLeft)
		{
			m_bMouseLeft = false;
			m_pWindow->ControlLinesChanged();
		}

		if (m_bDragging)
		{
			if (a_fNormalPressure < 0.333f)
				m_bCtrlDragging = m_bDragging = false;
		}
		else
		{
			if (a_fNormalPressure > 0.667f)
			{
				m_bCtrlDragging = a_eKeysState&ECKSControl;
				m_bDragging = true;
			}
		}
		if (m_bCtrlDragging)
		{
			if (!m_bDefining)
			{
				m_bDefining = true;
				m_pWindow->ControlPointsChanged();
			}
			m_nOffsetX = a_pPos->fX-m_nLastPosX;
			m_nOffsetY = a_pPos->fY-m_nLastPosY;
			m_nAbsoluteRefX = a_pPos->fX;
			m_nAbsoluteRefY = a_pPos->fY;
			if (m_bShowPoint)
				m_pWindow->ControlPointChanged(0);
			m_pWindow->ControlLinesChanged();
			RECT rc;
			if (S_OK == m_pTool->IsDirty(&rc, NULL, NULL))
				m_pWindow->RectangleChanged(&rc);
			return S_OK;
		}
		if (!m_bDragging && a_eKeysState&ECKSControl)
		{
			if (!m_bDefining)
			{
				m_bDefining = true;
				m_pWindow->ControlPointsChanged();
			}
			return S_OK;
		}
		else
		{
			m_nLastPosX = a_pPos->fX;
			m_nLastPosY = a_pPos->fY;
			if (!m_cData.bRelative && !m_bDragging && !m_bAbsLock)
			{
				m_nOffsetX = m_nAbsoluteRefX-a_pPos->fX;
				m_nOffsetY = m_nAbsoluteRefY-a_pPos->fY;
				if (m_bShowPoint)
					m_pWindow->ControlPointChanged(0);
				m_pWindow->ControlLinesChanged();
			}
			if (m_cData.bRelative)
			{
				m_nAbsoluteRefX = a_pPos->fX+m_nOffsetX;
				m_nAbsoluteRefY = a_pPos->fY+m_nOffsetY;
			}
			if (m_bDefining)
			{
				m_bDefining = false;
				m_pWindow->ControlPointsChanged();
			}
			m_pWindow->ControlLinesChanged();
			return m_pTool->ProcessInputEvent(static_cast<EControlKeysState>(a_eKeysState&(~ECKSControl)), a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
		}
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		if (m_bDefining)
			return CEditToolCustomPrecisionCursor<GetToolIconCLONE>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
		return m_pTool->GetCursor(static_cast<EControlKeysState>(a_eKeysState&(~ECKSControl)), a_pPos, a_phCursor);
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		HRESULT hRes = m_pTool->GetControlPointCount(a_pCount);
		if (m_bDefining)
		{
			if (a_pCount)
				++*a_pCount;
		}
		m_bShowPoint = m_bDefining;
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
					a_pPos->fX = m_cData.bRelative ? m_nLastPosX+m_nOffsetX+0.5f : m_nAbsoluteRefX+0.5f;
					a_pPos->fY = m_cData.bRelative ? m_nLastPosY+m_nOffsetY+0.5f : m_nAbsoluteRefY+0.5f;
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
				m_nOffsetX = a_pPos->fX-m_nLastPosX;
				m_nOffsetY = a_pPos->fY-m_nLastPosY;
				m_nAbsoluteRefX = a_pPos->fX;
				m_nAbsoluteRefY = a_pPos->fY;
				m_pWindow->ControlPointChanged(0);
				m_pWindow->ControlLinesChanged();
				RECT rc;
				if (S_OK == m_pTool->IsDirty(&rc, NULL, NULL))
					m_pWindow->RectangleChanged(&rc);
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
		if (m_bMouseLeft)
			return m_pTool->GetControlLines(a_pLines, a_nLineTypes);

		int const nSteps = 12;
		float const fStep = 3.14159265f*2.0f/nSteps;
		TPixelCoords tCenter = {m_nLastPosX+m_nOffsetX+0.5f, m_nLastPosY+m_nOffsetY+0.5f};
		float fRadius = 3.0f;
		a_pLines->HandleSize(&fRadius);
		fRadius *= 2.0f;
		a_pLines->MoveTo(tCenter.fX, tCenter.fY-fRadius);
		for (int i = 1; i < nSteps; ++i)
		{
			float fAngle = i*fStep;
			a_pLines->LineTo(tCenter.fX+fRadius*sinf(fAngle), tCenter.fY-fRadius*cosf(fAngle));
		}
		a_pLines->Close();

		float fDist = sqrtf(m_nOffsetX*m_nOffsetX + m_nOffsetY*m_nOffsetY);
		if (fDist > fRadius)
		{
			float f = (fDist-fRadius)/fDist;
			a_pLines->MoveTo(m_nLastPosX+0.5f, m_nLastPosY+0.5f);
			a_pLines->LineTo(m_nLastPosX+m_nOffsetX*f+0.5f, m_nLastPosY+m_nOffsetY*f+0.5f);
		}
		return m_pTool->GetControlLines(a_pLines, a_nLineTypes);
	}
	STDMETHOD(PointTest)(EControlKeysState (a_eKeysState), TPixelCoords const* (a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
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
		m_bAbsLock = m_pTool ? S_OK == m_pTool->IsDirty(NULL, NULL, NULL) : false;
		return m_pWindow->RectangleChanged(a_pChanged);
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		m_cData.pInternal = a_pState;
		CComObject<CSharedStateEditToolClone>* pNew = NULL;
		CComObject<CSharedStateEditToolClone>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		pNew->Init(m_cData);
		return m_pWindow->SetState(pTmp);
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

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	CEditToolDataClone m_cData;
	LONG m_nOffsetX;
	LONG m_nOffsetY;
	LONG m_nAbsoluteRefX;
	LONG m_nAbsoluteRefY;
	LONG m_nLastPosX;
	LONG m_nLastPosY;
	bool m_bDefining;
	bool m_bShowPoint;
	bool m_bDragging;
	bool m_bAbsLock;
	bool m_bCtrlDragging;
	bool m_bMouseLeft;
};

