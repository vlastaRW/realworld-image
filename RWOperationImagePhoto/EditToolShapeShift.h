
#pragma once

#include "EditTool.h"
#include <math.h>
#include <agg_blur.h>
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <GammaCorrection.h>

//#include <boost/spirit.hpp>
//using namespace boost::spirit;


struct CEditToolDataShapeShift
{
	enum EShapeTool
	{
		ESTTangentPush = 0,
		ESTNormalPush,
		ESTRestore,
		ESTCollapse,
		ESTExpand,
		//ESTTwirl,
		//ESTTurbulence,
		ESTCount
	};

	CEditToolDataShapeShift() : eTool(ESTTangentPush), fSize(100.0f), fIntensity(50.0f), bIntPressure(false)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		int i = 0;
		float fSize_ = 100.0f;
		float fIntensity_ = 50.0f;
		if (a_bstr && 1 <= swscanf(a_bstr, L"%i,%f,%f", &i, &fSize_, &fIntensity_))
		{
			if (i >= 0 && i < ESTCount)
				eTool = static_cast<EShapeTool>(i);
			fSize = fSize_;
			fIntensity = fIntensity_;
			int nLen = wcslen(a_bstr);
			bIntPressure = nLen > 3 && wcscmp(a_bstr+nLen-3, L",NP")==0;
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[32];
		swprintf(szTmp, L"%i,%g,%g", int(eTool), fSize, fIntensity);
		if (bIntPressure)
			wcscat(szTmp, L",NP");
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	EShapeTool eTool;
	float fSize;
	float fIntensity;
	bool bIntPressure;
};


#include "EditToolShapeShiftDlg.h"


class CEditToolShapeShift :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CEditToolBase<>,
	public CEditToolCustomPrecisionCursor<GetShapeShiftIconForward>,
	public CEditToolCustomPrecisionCursor<GetShapeShiftIconSideways>,
	public CEditToolCustomPrecisionCursor<GetShapeShiftIconRestore>,
	public CEditToolCustomPrecisionCursor<GetShapeShiftIconCollapse>,
	public CEditToolCustomPrecisionCursor<GetShapeShiftIconExpand>
{
public:
	CEditToolShapeShift() : m_eRasterMode(ERMSmooth), m_bMouseLeft(true), m_rcDirty(RECT_EMPTY), m_fLastPressure(0.0f),
		m_bCTRL(false), m_bSHIFT(false)
	{
	}

	BEGIN_COM_MAP(CEditToolShapeShift)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
	END_COM_MAP()


	// IRasterImageEditTool methods
public:
	STDMETHOD(Reset)()
	{
		try
		{
			RECT rc = m_rcDirty;
			m_rcDirty = RECT_EMPTY;
			ULONG nX = 0;
			ULONG nY = 0;
			M_Window()->Size(&nX, &nY);
			m_fLastPressure = 0.0f;
			if (nX && nY)
			{
				m_aOffsets.Free();
				m_aSource.Free();
				//m_aCached.Free();
				m_aOffsets.Allocate(nX*nY);
				ZeroMemory(m_aOffsets.m_p, nX*nY*sizeof*m_aOffsets.m_p);
				m_aSource.Allocate(nX*nY);
				//m_aCached.Allocate(nX*nY);
				m_nSizeX = nX;
				m_nSizeY = nY;
				//m_rcCacheValid = RECT_EMPTY;
				M_Window()->GetImageTile(0, 0, nX, nY, 0.0f, nX, EITIContent, m_aSource);
			}
			if (rc.left < rc.right && rc.top < rc.bottom)
				M_Window()->RectangleChanged(&rc);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pImageRect)
			*a_pImageRect = m_rcDirty;
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE;
		if (a_pSelectionRect)
			*a_pSelectionRect = RECT_EMPTY;
		return m_rcDirty.left < m_rcDirty.right && m_rcDirty.top < m_rcDirty.bottom ? S_OK : S_FALSE;
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		if (m_aOffsets.m_p == NULL || m_aSource.m_p == NULL || LONG(a_nX+a_nSizeX) < 0 || a_nX >= LONG(m_nSizeX) || LONG(a_nY+a_nSizeY) < 0 || a_nY >= LONG(m_nSizeY))
			return M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);

		RECT const rc = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};

		try
		{
			BOOL bEntire = TRUE;
			RECT rcBounds = {0, 0, m_nSizeX, m_nSizeY};
			M_Window()->GetSelectionInfo(&rcBounds, &bEntire);
			if (rc.left >= rcBounds.right || rc.top >= rcBounds.bottom || 
				rc.right < rcBounds.left || rc.bottom < rcBounds.top) // rectangle outside of selection
				return M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
			CAutoVectorPtr<BYTE> cSel;
			if (!bEntire || rcBounds.left > rc.left || rcBounds.top > rc.top || rcBounds.right < rc.right || rcBounds.bottom < rc.bottom)
			{
				cSel.Attach(new BYTE[a_nSizeX*a_nSizeY]);
				if (FAILED(M_Window()->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nSizeX, cSel)))
					cSel.Free();
			}

			CAutoPtr<CGammaTables> pGT;
			if (a_fGamma != 1.0f)
				pGT.Attach(new CGammaTables(a_fGamma));
			SOffset const* pOff = m_aOffsets.m_p+LONG(m_nSizeX)*a_nY+a_nX;
			TRasterImagePixel tEmpty = {0, 0, 0, 0};
			M_Window()->GetDefaultColor(&tEmpty);
			BYTE *pS = cSel;
			for (LONG nY = rc.top; nY < rc.bottom; ++nY, a_pBuffer += LONG(a_nStride-a_nSizeX), pOff += LONG(m_nSizeX-a_nSizeX))
			{
				if (nY < 0 || nY >= LONG(m_nSizeY))
				{
					for (LONG nX = rc.left; nX < rc.right; ++nX, ++a_pBuffer, ++pOff, ++pS)
						*a_pBuffer = tEmpty;
					continue;
				}
				if (m_eRasterMode == ERMSmooth)
				{
					for (LONG nX = rc.left; nX < rc.right; ++nX, ++a_pBuffer, ++pOff, ++pS)
					{
						int const nOffX = nX < 0 || nX >= LONG(m_nSizeX) ? 0 : (cSel.m_p ? pOff->nDX**pS/255 : pOff->nDX);
						int const nOffY = nX < 0 || nX >= LONG(m_nSizeX) ? 0 : (cSel.m_p ? pOff->nDY**pS/255 : pOff->nDY);
						if (nOffX&0xff)
						{
							int nDX = nOffX>>8;
							int x1 = nDX+nX;
							if (x1 < -1 || x1 >= int(m_nSizeX))
							{
								*a_pBuffer = tEmpty;
							}
							else if (nOffY&0xff)
							{
								int nDY = nOffY>>8;
								int y1 = nDY+nY;
								if (y1 < -1 || y1 >= int(m_nSizeY))
								{
									*a_pBuffer = tEmpty;
								}
								else
								{
									int yo1 = y1*m_nSizeX;
									TRasterImagePixel const t1 = y1 == -1 || x1 == -1 ? tEmpty : m_aSource[yo1+x1];
									TRasterImagePixel const t2 = y1 == m_nSizeY-1 || x1 == -1 ? tEmpty : m_aSource[yo1+m_nSizeX+x1];
									TRasterImagePixel const t3 = y1 == -1 || x1 == m_nSizeX-1 ? tEmpty : m_aSource[yo1+x1+1];
									TRasterImagePixel const t4 = y1 == m_nSizeY-1 || x1 == m_nSizeX-1 ? tEmpty : m_aSource[yo1+m_nSizeX+x1+1];
									ULONG const nIY = nOffY&0xff;
									ULONG const nFY = 0x100-nIY;
									ULONG const nIX = nOffX&0xff;
									ULONG const nFX = 0x100-nIX;
									int nA = t1.bA*nFY*nFX + t2.bA*nIY*nFX + t3.bA*nFY*nIX + t4.bA*nIY*nIX;
									if (nA&0xffffff00)
									{
										if (pGT.m_p)
										{
											ULONG const nFF = (nFX*nFY)>>8;
											ULONG const nFI = (nFX*nIY)>>8;
											ULONG const nIF = (nIX*nFY)>>8;
											ULONG const nII = (nIX*nIY)>>8;
											ULONG const nAA = nA>>8;
											a_pBuffer->bR = pGT->InvGamma((pGT->m_aGamma[t1.bR]*nFF*t1.bA + pGT->m_aGamma[t2.bR]*nFI*t2.bA + pGT->m_aGamma[t3.bR]*nIF*t3.bA + pGT->m_aGamma[t4.bR]*nII*t4.bA)/nAA);
											a_pBuffer->bG = pGT->InvGamma((pGT->m_aGamma[t1.bG]*nFF*t1.bA + pGT->m_aGamma[t2.bG]*nFI*t2.bA + pGT->m_aGamma[t3.bG]*nIF*t3.bA + pGT->m_aGamma[t4.bG]*nII*t4.bA)/nAA);
											a_pBuffer->bB = pGT->InvGamma((pGT->m_aGamma[t1.bB]*nFF*t1.bA + pGT->m_aGamma[t2.bB]*nFI*t2.bA + pGT->m_aGamma[t3.bB]*nIF*t3.bA + pGT->m_aGamma[t4.bB]*nII*t4.bA)/nAA);
										}
										else
										{
											a_pBuffer->bR = (t1.bR*nFY*nFX*t1.bA + t2.bR*nIY*nFX*t2.bA + t3.bR*nFY*nIX*t3.bA + t4.bR*nIY*nIX*t4.bA)/nA;
											a_pBuffer->bG = (t1.bG*nFY*nFX*t1.bA + t2.bG*nIY*nFX*t2.bA + t3.bG*nFY*nIX*t3.bA + t4.bG*nIY*nIX*t4.bA)/nA;
											a_pBuffer->bB = (t1.bB*nFY*nFX*t1.bA + t2.bB*nIY*nFX*t2.bA + t3.bB*nFY*nIX*t3.bA + t4.bB*nIY*nIX*t4.bA)/nA;
										}
										a_pBuffer->bA = nA>>16;
									}
									else
									{
										*a_pBuffer = tEmpty;
									}
								}
							}
							else
							{
								int y = nY+(nOffY>>8);
								if (y < 0 || y >= int(m_nSizeY))
								{
									*a_pBuffer = tEmpty;
								}
								else
								{
									int yo = y*m_nSizeX;
									TRasterImagePixel const t1 = x1 == -1 ? tEmpty : m_aSource[yo+x1];
									TRasterImagePixel const t2 = x1 == m_nSizeX-1 ? tEmpty : m_aSource[yo+1+x1];
									ULONG const nIX = nOffX&0xff;
									ULONG const nFX = 0x100-nIX;
									int nA = t1.bA*nFX + t2.bA*nIX;
									if (nA)
									{
										if (pGT.m_p)
										{
											a_pBuffer->bR = pGT->InvGamma((pGT->m_aGamma[t1.bR]*nFX*t1.bA + pGT->m_aGamma[t2.bR]*nIX*t2.bA)/nA);
											a_pBuffer->bG = pGT->InvGamma((pGT->m_aGamma[t1.bG]*nFX*t1.bA + pGT->m_aGamma[t2.bG]*nIX*t2.bA)/nA);
											a_pBuffer->bB = pGT->InvGamma((pGT->m_aGamma[t1.bB]*nFX*t1.bA + pGT->m_aGamma[t2.bB]*nIX*t2.bA)/nA);
										}
										else
										{
											a_pBuffer->bR = (t1.bR*nFX*t1.bA + t2.bR*nIX*t2.bA)/nA;
											a_pBuffer->bG = (t1.bG*nFX*t1.bA + t2.bG*nIX*t2.bA)/nA;
											a_pBuffer->bB = (t1.bB*nFX*t1.bA + t2.bB*nIX*t2.bA)/nA;
										}
										a_pBuffer->bA = nA>>8;
									}
									else
									{
										*a_pBuffer = tEmpty;
									}
								}
							}
						}
						else
						{
							int x = nX+(nOffX>>8);
							if (x < 0 || x >= int(m_nSizeX))
							{
								*a_pBuffer = tEmpty;
							}
							else if (nOffY&0xff)
							{
								int nDY = nOffY>>8;
								int y1 = nDY+nY;
								if (y1 < -1 || y1 >= int(m_nSizeY))
								{
									*a_pBuffer = tEmpty;
								}
								else
								{
									int yo1 = y1*m_nSizeX;
									TRasterImagePixel const t1 = y1 == -1 ? tEmpty : m_aSource[yo1+x];
									TRasterImagePixel const t2 = y1 == m_nSizeY-1 ? tEmpty : m_aSource[yo1+m_nSizeX+x];
									ULONG const nIY = nOffY&0xff;
									ULONG const nFY = 0x100-nIY;
									int nA = t1.bA*nFY + t2.bA*nIY;
									if (nA)
									{
										if (pGT.m_p)
										{
											a_pBuffer->bR = pGT->InvGamma((pGT->m_aGamma[t1.bR]*nFY*t1.bA + pGT->m_aGamma[t2.bR]*nIY*t2.bA)/nA);
											a_pBuffer->bG = pGT->InvGamma((pGT->m_aGamma[t1.bG]*nFY*t1.bA + pGT->m_aGamma[t2.bG]*nIY*t2.bA)/nA);
											a_pBuffer->bB = pGT->InvGamma((pGT->m_aGamma[t1.bB]*nFY*t1.bA + pGT->m_aGamma[t2.bB]*nIY*t2.bA)/nA);
										}
										else
										{
											a_pBuffer->bR = (t1.bR*nFY*t1.bA + t2.bR*nIY*t2.bA)/nA;
											a_pBuffer->bG = (t1.bG*nFY*t1.bA + t2.bG*nIY*t2.bA)/nA;
											a_pBuffer->bB = (t1.bB*nFY*t1.bA + t2.bB*nIY*t2.bA)/nA;
										}
										a_pBuffer->bA = nA>>8;
									}
									else
									{
										*a_pBuffer = tEmpty;
									}
								}
							}
							else
							{
								int y = nY+(nOffY>>8);
								if (y < 0 || y >= int(m_nSizeY))
									*a_pBuffer = tEmpty;
								else
									*a_pBuffer = m_aSource[m_nSizeX*y+x];
							}
						}
					}
				}
				else
				{
					//for (ULONG nX = rc.left; nX < rc.right; ++nX, ++a_pBuffer, ++pOff)
					//{
					//	a_pBuffer->bA = 255;
					//	a_pBuffer->bR = a_pBuffer->bG = a_pBuffer->bB = sqrtf(pOff->nDX/256.0*pOff->nDX/256.0+pOff->nDY/256.0*pOff->nDY/256.0)*64;
					//}
					if (cSel.m_p)
					{
						for (LONG nX = rc.left; nX < rc.right; ++nX, ++a_pBuffer, ++pOff, ++pS)
						{
							int const x = nX < 0 || nX >= LONG(m_nSizeX) ? nX : (nX+((pOff->nDX**pS/255+0x80)>>8));
							int const y = nX < 0 || nX >= LONG(m_nSizeX) ? nY : (nY+((pOff->nDY**pS/255+0x80)>>8));
							if (x < 0 || x >= int(m_nSizeX) || y < 0 || y >= int(m_nSizeY))
								*a_pBuffer = tEmpty;
							else
								*a_pBuffer = m_aSource[m_nSizeX*y+x];
						}
					}
					else
					{
						for (LONG nX = rc.left; nX < rc.right; ++nX, ++a_pBuffer, ++pOff)
						{
							int const x = nX < 0 || nX >= LONG(m_nSizeX) ? nX : (nX+((pOff->nDX+0x80)>>8));
							int const y = nX < 0 || nX >= LONG(m_nSizeX) ? nY : (nY+((pOff->nDY+0x80)>>8));
							if (x < 0 || x >= int(m_nSizeX) || y < 0 || y >= int(m_nSizeY))
								*a_pBuffer = tEmpty;
							else
								*a_pBuffer = m_aSource[m_nSizeX*y+x];
						}
					}
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return S_FALSE;
	}
	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_tPos, HCURSOR* a_phCursor)
	{
		ULONG n = m_cData.eTool;
		if (m_fLastPressure > 0.0f && m_bSHIFT || m_fLastPressure == 0.0 && a_eKeysState&ECKSShift)
			n = CEditToolDataShapeShift::ESTRestore;
		switch (n)
		{
		case CEditToolDataShapeShift::ESTTangentPush:
			return CEditToolCustomPrecisionCursor<GetShapeShiftIconForward>::_GetCursor(a_eKeysState, a_tPos, a_phCursor);
		case CEditToolDataShapeShift::ESTNormalPush:
			return CEditToolCustomPrecisionCursor<GetShapeShiftIconSideways>::_GetCursor(a_eKeysState, a_tPos, a_phCursor);
		case CEditToolDataShapeShift::ESTCollapse:
			return CEditToolCustomPrecisionCursor<GetShapeShiftIconCollapse>::_GetCursor(a_eKeysState, a_tPos, a_phCursor);
		case CEditToolDataShapeShift::ESTExpand:
			return CEditToolCustomPrecisionCursor<GetShapeShiftIconExpand>::_GetCursor(a_eKeysState, a_tPos, a_phCursor);
		default:
			return CEditToolCustomPrecisionCursor<GetShapeShiftIconRestore>::_GetCursor(a_eKeysState, a_tPos, a_phCursor);
		}
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComBSTR bstrText;
		a_pState->ToText(&bstrText);
		m_cData.FromString(bstrText);
		M_Window()->ControlLinesChanged();
		return S_OK;
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (a_eRasterizationMode != m_eRasterMode)
		{
			m_eRasterMode = a_eRasterizationMode;
			if (m_rcDirty.left < m_rcDirty.right)
				M_Window()->RectangleChanged(&m_rcDirty);
		}
		return S_OK;
	}

	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		if (a_pPos == NULL)
		{
			if (!m_bMouseLeft)
			{
				m_bMouseLeft = true;
				M_Window()->ControlLinesChanged();
			}
			return S_OK;
		}

		bool bPosChange = m_tLastPos.fX != a_pPos->fX || m_tLastPos.fY != a_pPos->fY || m_bMouseLeft;
		float const fSize = m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize;
		m_bMouseLeft = false;
		if (m_fLastPressure == 0.0f)
		{
			if (a_fNormalPressure > 0.05f)
			{
				// start drawing
				m_fLastPressure = a_fNormalPressure;
				m_tLastPos = *a_pPos;
				m_bSHIFT = a_eKeysState&ECKSShift;
				m_bCTRL = a_eKeysState&ECKSControl;
				//static TPixelCoords const tDir = {0.0f, 0.0f};
				//PushPixels(*a_pPos, a_fNormalPressure, fSize, tDir);
			}
			else
			{
				m_tLastPos = *a_pPos;
			}
		}
		else
		{
			if (a_fNormalPressure < 0.05f)
			{
				// stop drawing
				//M_Window()->ApplyChanges();
				m_fLastPressure = 0.0f;
				m_tLastPos = *a_pPos;
				m_bSHIFT = m_bCTRL = false;
			}
			else
			{
				// continue drawing
				float const fDX = a_pPos->fX-m_tLastPos.fX;
				float const fDY = a_pPos->fY-m_tLastPos.fY;
				float const fDist = sqrtf(fDX*fDX + fDY*fDY);
				if (fDist > 0.25f)
				{
					float const fPressure = m_cData.bIntPressure ? 0.3f+0.35f*(m_fLastPressure+a_fNormalPressure) : 1.0f;
					if (m_cData.eTool == CEditToolDataShapeShift::ESTRestore || m_bSHIFT)
						RestorePixels(m_tLastPos.fX, m_tLastPos.fY, a_pPos->fX, a_pPos->fY, fPressure);
					else if (m_cData.eTool == CEditToolDataShapeShift::ESTTangentPush)
						PushPixels(m_tLastPos.fX, m_tLastPos.fY, a_pPos->fX, a_pPos->fY, fDX/fDist*fPressure, fDY/fDist*fPressure);
					else if (m_cData.eTool == CEditToolDataShapeShift::ESTNormalPush)
						PushPixels(m_tLastPos.fX, m_tLastPos.fY, a_pPos->fX, a_pPos->fY, -fDY/fDist*fPressure, fDX/fDist*fPressure);
					else if (m_cData.eTool == CEditToolDataShapeShift::ESTCollapse)
						CollapsePixels(m_tLastPos.fX, m_tLastPos.fY, a_pPos->fX, a_pPos->fY, -1.0f*fPressure);
					else if (m_cData.eTool == CEditToolDataShapeShift::ESTExpand)
						CollapsePixels(m_tLastPos.fX, m_tLastPos.fY, a_pPos->fX, a_pPos->fY, 1.0f*fPressure);
				}
				m_tLastPos = *a_pPos;
				m_fLastPressure = a_fNormalPressure;
			}
		}
		if (bPosChange)
			M_Window()->ControlLinesChanged();
		return S_OK;
	}

	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (!m_bMouseLeft && (a_nLineTypes&ECLTHelp))
		{
			float fRadius = m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize*0.5f;
			float fScale = 1.0f;
			a_pLines->HandleSize(&fScale);
			int nSteps = 12*sqrtf(fRadius/fScale*0.1f);
			if (nSteps > 120) nSteps = 120;
			else if (nSteps < 8)
				nSteps = 8;
			float const fStep = 3.14159265f*2.0f/nSteps;
			TPixelCoords tCenter = m_tLastPos;
			a_pLines->MoveTo(tCenter.fX, tCenter.fY-fRadius);
			for (int i = 1; i < nSteps; ++i)
			{
				float fAngle = i*fStep;
				a_pLines->LineTo(tCenter.fX+fRadius*sinf(fAngle), tCenter.fY-fRadius*cosf(fAngle));
			}
			a_pLines->Close();
		}

		return S_OK;
	}

private:
	struct SOffset
	{
		int nDX; // fixed point 24:8 bits 
		int nDY;
		typedef int calc_type;
		typedef int value_type;
	};
	struct SOffsetImg
	{
		typedef SOffset color_type;
		typedef SOffset row_data;

		SOffsetImg(SOffset* a_pBuffer, int a_nSizeX, int a_nSizeY) :
			m_pBuffer(a_pBuffer), m_nStride(a_nSizeX),
			m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY)
		{
		}
		int stride() const { return m_nStride; }
		SOffset const& pixel(int x, int y) {return m_pBuffer[y*m_nStride+x];}
		void copy_color_hspan(int x, int y, unsigned len, SOffset const* colors)
        {
            SOffset* p = m_pBuffer+m_nStride*y+x;
            while (len--)
            {
				*p = *colors;
                ++colors;
                ++p;
            }
        }
		void copy_color_vspan(int x, int y, unsigned len, SOffset const* colors)
        {
            SOffset* p = m_pBuffer+m_nStride*y+x;
            while (len--)
            {
				*p = *colors;
                ++colors;
				p+=m_nStride;
            }
        }
		int width() const { return m_nSizeX; }
		int height() const { return m_nSizeY; }
		SOffset* m_pBuffer;
		int m_nStride;
		int m_nSizeX;
		int m_nSizeY;
	};
    template<class T=double> struct recursive_blur_calc_offset
    {
        typedef T value_type;
        typedef recursive_blur_calc_offset<T> self_type;

        value_type x;
        value_type y;

        AGG_INLINE void from_pix(const SOffset& c)
        {
			x = c.nDX;
			y = c.nDY;
        }

        AGG_INLINE void calc(value_type b1, 
                             value_type b2, 
                             value_type b3, 
                             value_type b4,
                             const self_type& c1, 
                             const self_type& c2, 
                             const self_type& c3, 
                             const self_type& c4)
        {
            x = b1*c1.x + b2*c2.x + b3*c3.x + b4*c4.x;
            y = b1*c1.y + b2*c2.y + b3*c3.y + b4*c4.y;
        }

        AGG_INLINE void to_pix(SOffset& c) const
        {
			c.nDX = int(x);
            c.nDY = int(y);
        }
    };

	void PushPixels(float a_fX1, float a_fY1, float a_fX2, float a_fY2, float a_fDX, float a_fDY)
	{
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;
		float fTX = a_fX2-a_fX1;
		float fTY = a_fY2-a_fY1;
		float fT = 1.0f/sqrtf(fTX*fTX+fTY*fTY);
		ras.move_to_d(a_fX1-fTY*fT*0.5, a_fY1+fTX*fT*0.5);
		ras.line_to_d(a_fX1+fTY*fT*0.5, a_fY1-fTX*fT*0.5);
		ras.line_to_d(a_fX2+fTY*fT*0.5, a_fY2-fTX*fT*0.5);
		ras.line_to_d(a_fX2-fTY*fT*0.5, a_fY2+fTX*fT*0.5);
		ras.close_polygon();
		LONG nR = ceilf(m_cData.fSize*0.5f);
		RECT rc = {ras.min_x()-nR-1, ras.min_y()-nR-1, ras.max_x()+2+nR+nR, ras.max_y()+2+nR+nR};
		LONG nW = rc.right-rc.left;
		//if (
		//m_nSizeX
		CAutoVectorPtr<SOffset> cBuffer(new SOffset[(rc.bottom-rc.top)*nW]);
		ZeroMemory(cBuffer.m_p, sizeof*cBuffer.m_p*(rc.bottom-rc.top)*nW);
		SOffset const t = {-a_fDX*m_cData.fIntensity*m_cData.fSize*m_cData.fSize*0.25f, -a_fDY*m_cData.fIntensity*m_cData.fSize*m_cData.fSize*0.25f};
		if (ras.rewind_scanlines())
		{
			sl.reset(ras.min_x(), ras.max_x());
            while(ras.sweep_scanline(sl))
			{
				int y = sl.y();

				agg::scanline_p8::const_iterator span = sl.begin();
				for(unsigned num_spans = sl.num_spans(); num_spans > 0; --num_spans, ++span)
				{
					int x = span->x;
					int len = span->len;
					const agg::scanline_p8::cover_type* covers = span->covers;

					if(len < 0) len = -len;
					SOffset* p = cBuffer.m_p+(y-rc.top)*nW+x-rc.left;
					while (len > 0)
					{
						p->nDX = t.nDX**covers/255;
						p->nDY = t.nDY**covers/255;
						++p;
						++covers;
						--len;
					}
				}
			}
			SOffsetImg sImg(cBuffer, rc.right-rc.left, rc.bottom-rc.top);
			agg::recursive_blur<SOffset, recursive_blur_calc_offset<double> > blur;
			blur.blur(sImg, m_cData.fSize*0.70710678f*0.5f);
			LONG const nY2 = min(LONG(m_nSizeY), rc.bottom-nR);
			for (LONG nY = max(0, rc.top); nY < nY2; ++nY)
			{
				LONG const nX1 = max(0, rc.left);
				LONG const nX2 = min(LONG(m_nSizeX), rc.right-nR);
				SOffset* pD = m_aOffsets.m_p+nY*m_nSizeX+nX1;
				SOffset* pS = cBuffer.m_p+nW*(nY-rc.top)+nX1-rc.left;
				for (LONG nX = nX1; nX < nX2; ++nX, ++pD, ++pS)
				{
					ULONG const nSrcX = nX+(pS->nDX>>8);
					ULONG const nSrcY = nY+(pS->nDY>>8);
					if (nSrcX < m_nSizeX-1 && nSrcY < m_nSizeY-1)
					{
						SOffset const* const p = m_aOffsets.m_p+nSrcY*m_nSizeX+nSrcX;
						ULONG const nX2 = pS->nDX&0xff;
						ULONG const nX1 = 256-nX2;
						ULONG const nY2 = pS->nDY&0xff;
						ULONG const nY1 = 256-nY2;
						ULONG const n11 = (nX1*nY1)>>8;
						ULONG const n12 = (nX2*nY1)>>8;
						ULONG const n21 = (nX1*nY2)>>8;
						ULONG const n22 = (nX2*nY2)>>8;
						SOffset const* const p11 = p;
						SOffset const* const p12 = p+1;
						SOffset const* const p21 = p+m_nSizeX;
						SOffset const* const p22 = p+m_nSizeX+1;
						SOffset const s = {LONG(p11->nDX*n11+p12->nDX*n12+p21->nDX*n21+p22->nDX*n22)>>8, LONG(p11->nDY*n11+p12->nDY*n12+p21->nDY*n21+p22->nDY*n22)>>8};
						pS->nDX += s.nDX;
						pS->nDY += s.nDY;
					}
					else
					{
						ULONG nSX = nSrcX;
						ULONG nSY = nSrcY;
						if (nSX >= m_nSizeX-1)
							nSX = nSX < 0x7fffffff ? m_nSizeX-1 : 0;
						if (nSY >= m_nSizeY-1)
							nSY = nSY < 0x7fffffff ? m_nSizeY-1 : 0;
						SOffset const* const p = m_aOffsets.m_p+nSY*m_nSizeX+nSX;
						pS->nDX += p->nDX;
						pS->nDY += p->nDY;
					}
				}
			}
			for (LONG nY = max(0, rc.top); nY < nY2; ++nY)
			{
				LONG const nX1 = max(0, rc.left);
				LONG const nX2 = min(LONG(m_nSizeX), rc.right-nR);
				SOffset* pD = m_aOffsets.m_p+nY*m_nSizeX+nX1;
				SOffset* pS = cBuffer.m_p+nW*(nY-rc.top)+nX1-rc.left;
				for (LONG nX = nX1; nX < nX2; ++nX, ++pD, ++pS)
				{
					pD->nDX = pS->nDX;
					pD->nDY = pS->nDY;
				}
			}
			AddDirtyRect(max(rc.left, 0), max(rc.top, 0), min(LONG(m_nSizeX), rc.right), min(LONG(m_nSizeY), rc.bottom));
			M_Window()->RectangleChanged(&rc);
		}
	}

	struct SWeight
	{
		int n;
		typedef int calc_type;
		typedef int value_type;
	};
	struct SWeightImg
	{
		typedef SWeight color_type;
		typedef SWeight row_data;

		SWeightImg(SWeight* a_pBuffer, int a_nSizeX, int a_nSizeY) :
			m_pBuffer(a_pBuffer), m_nStride(a_nSizeX),
			m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY)
		{
		}
		int stride() const { return m_nStride; }
		SWeight const& pixel(int x, int y) {return m_pBuffer[y*m_nStride+x];}
		void copy_color_hspan(int x, int y, unsigned len, SWeight const* colors)
        {
            SWeight* p = m_pBuffer+m_nStride*y+x;
            while (len--)
            {
				*p = *colors;
                ++colors;
                ++p;
            }
        }
		void copy_color_vspan(int x, int y, unsigned len, SWeight const* colors)
        {
            SWeight* p = m_pBuffer+m_nStride*y+x;
            while (len--)
            {
				*p = *colors;
                ++colors;
				p+=m_nStride;
            }
        }
		int width() const { return m_nSizeX; }
		int height() const { return m_nSizeY; }
		SWeight* m_pBuffer;
		int m_nStride;
		int m_nSizeX;
		int m_nSizeY;
	};
    template<class T=double> struct recursive_blur_calc_weight
    {
        typedef T value_type;
        typedef recursive_blur_calc_weight<T> self_type;

        value_type n;

        AGG_INLINE void from_pix(const SWeight& c)
        {
			n = c.n;
        }

        AGG_INLINE void calc(value_type b1, 
                             value_type b2, 
                             value_type b3, 
                             value_type b4,
                             const self_type& c1, 
                             const self_type& c2, 
                             const self_type& c3, 
                             const self_type& c4)
        {
            n = b1*c1.n + b2*c2.n + b3*c3.n + b4*c4.n;
        }

        AGG_INLINE void to_pix(SWeight& c) const
        {
			c.n = int(n);
        }
    };
	void RestorePixels(float a_fX1, float a_fY1, float a_fX2, float a_fY2, float a_fPressure)
	{
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;
		float fTX = a_fX2-a_fX1;
		float fTY = a_fY2-a_fY1;
		float fT = 1.0f/sqrtf(fTX*fTX+fTY*fTY);
		ras.move_to_d(a_fX1-fTY*fT*0.5, a_fY1+fTX*fT*0.5);
		ras.line_to_d(a_fX1+fTY*fT*0.5, a_fY1-fTX*fT*0.5);
		ras.line_to_d(a_fX2+fTY*fT*0.5, a_fY2-fTX*fT*0.5);
		ras.line_to_d(a_fX2-fTY*fT*0.5, a_fY2+fTX*fT*0.5);
		ras.close_polygon();
		LONG nR = ceilf(m_cData.fSize*0.5f);
		RECT rc = {ras.min_x()-nR-1, ras.min_y()-nR-1, ras.max_x()+2+nR+nR, ras.max_y()+2+nR+nR};
		LONG nW = rc.right-rc.left;
		LONG const nY2 = min(LONG(m_nSizeY), rc.bottom-nR);
		LONG const nX1 = max(0, rc.left);
		LONG const nX2 = min(LONG(m_nSizeX), rc.right-nR);
		LONG const nY1 = max(0, rc.top);
		if (nX1 >= nX2 || nY1 >= nY2)
			return;
		CAutoVectorPtr<SWeight> cBuffer(new SWeight[(rc.bottom-rc.top)*nW]);
		ZeroMemory(cBuffer.m_p, sizeof*cBuffer.m_p*(rc.bottom-rc.top)*nW);
		int const nAmount = m_cData.fSize*m_cData.fSize*0.25f*m_cData.fIntensity*a_fPressure;
		if (ras.rewind_scanlines())
		{
			sl.reset(ras.min_x(), ras.max_x());
            while(ras.sweep_scanline(sl))
			{
				int y = sl.y();

				agg::scanline_p8::const_iterator span = sl.begin();
				for(unsigned num_spans = sl.num_spans(); num_spans > 0; --num_spans, ++span)
				{
					int x = span->x;
					int len = span->len;
					const agg::scanline_p8::cover_type* covers = span->covers;

					if(len < 0) len = -len;
					SWeight* p = cBuffer.m_p+(y-rc.top)*nW+x-rc.left;
					while (len > 0)
					{
						p->n = (nAmount**covers)>>8;
						++p;
						++covers;
						--len;
					}
				}
			}
			SWeightImg sImg(cBuffer, rc.right-rc.left, rc.bottom-rc.top);
			agg::recursive_blur<SWeight, recursive_blur_calc_weight<double> > blur;
			blur.blur(sImg, m_cData.fSize*0.5f*0.70710678f);
			for (LONG nY = nY1; nY < nY2; ++nY)
			{
				SOffset* pD = m_aOffsets.m_p+nY*m_nSizeX+nX1;
				SWeight* pS = cBuffer.m_p+nW*(nY-rc.top)+nX1-rc.left;
				for (LONG nX = nX1; nX < nX2; ++nX, ++pD, ++pS)
				{
					if (pD->nDX >= 0)
					{
						pD->nDX = pS->n > pD->nDX ? 0 : pD->nDX-pS->n;
					}
					else
					{
						pD->nDX = pS->n > -pD->nDX ? 0 : pD->nDX+pS->n;
					}
					if (pD->nDY >= 0)
					{
						pD->nDY = pS->n > pD->nDY ? 0 : pD->nDY-pS->n;
					}
					else
					{
						pD->nDY = pS->n > -pD->nDY ? 0 : pD->nDY+pS->n;
					}
				}
			}
			AddDirtyRect(nX1, nY1, nX2, nY2);
			M_Window()->RectangleChanged(&rc);
		}
	}
	void CollapsePixels(float a_fX1, float a_fY1, float a_fX2, float a_fY2, float a_fDir)
	{
		agg::rasterizer_scanline_aa<> ras;
		agg::scanline_p8 sl;
		float fTX = a_fX2-a_fX1;
		float fTY = a_fY2-a_fY1;
		float fT = 1.0f/sqrtf(fTX*fTX+fTY*fTY);
		ras.move_to_d(a_fX1-fTY*fT*0.5, a_fY1+fTX*fT*0.5);
		ras.line_to_d(a_fX1+fTY*fT*0.5, a_fY1-fTX*fT*0.5);
		ras.line_to_d(a_fX2+fTY*fT*0.5, a_fY2-fTX*fT*0.5);
		ras.line_to_d(a_fX2-fTY*fT*0.5, a_fY2+fTX*fT*0.5);
		ras.close_polygon();
		LONG nR = ceilf(m_cData.fSize*0.5);
		RECT rc = {ras.min_x()-nR-1, ras.min_y()-nR-1, ras.max_x()+2+nR+nR, ras.max_y()+2+nR+nR};
		LONG nW = rc.right-rc.left;
		LONG const nY2 = min(LONG(m_nSizeY), rc.bottom-nR-1);
		LONG const nX1 = max(0, rc.left+1);
		LONG const nX2 = min(LONG(m_nSizeX), rc.right-nR-1);
		LONG const nY1 = max(0, rc.top+1);
		if (nX1 >= nX2 || nY1 >= nY2)
			return;
		CAutoVectorPtr<SWeight> cBuffer(new SWeight[(rc.bottom-rc.top)*nW]);
		ZeroMemory(cBuffer.m_p, sizeof*cBuffer.m_p*(rc.bottom-rc.top)*nW);
		int const nAmount = m_cData.fSize*m_cData.fSize*0.25f*m_cData.fIntensity;
		if (ras.rewind_scanlines())
		{
			sl.reset(ras.min_x(), ras.max_x());
            while(ras.sweep_scanline(sl))
			{
				int y = sl.y();

				agg::scanline_p8::const_iterator span = sl.begin();
				for(unsigned num_spans = sl.num_spans(); num_spans > 0; --num_spans, ++span)
				{
					int x = span->x;
					int len = span->len;
					const agg::scanline_p8::cover_type* covers = span->covers;

					if(len < 0) len = -len;
					SWeight* p = cBuffer.m_p+(y-rc.top)*nW+x-rc.left;
					while (len > 0)
					{
						p->n = (nAmount**covers)>>8;
						++p;
						++covers;
						--len;
					}
				}
			}
			SWeightImg sImg(cBuffer, rc.right-rc.left, rc.bottom-rc.top);
			agg::recursive_blur<SWeight, recursive_blur_calc_weight<double> > blur;
			blur.blur(sImg, m_cData.fSize*0.5f*0.70710678f);
			CAutoVectorPtr<SOffset> cTmp(new SOffset[(nX2-nX1)*(nY2-nY1)]);
			SOffset* pT = cTmp;
			float const fFactor = a_fDir*m_cData.fSize*0.055f;
			for (LONG nY = nY1; nY < nY2; ++nY)
			{
				SOffset* pD = m_aOffsets.m_p+nY*m_nSizeX+nX1;
				SWeight* pS = cBuffer.m_p+nW*(nY-rc.top)+nX1-rc.left;
				for (LONG nX = nX1; nX < nX2; ++nX, ++pD, ++pS)
				{
					int nDX = (pS[1].n-pS[-1].n)*fFactor;
					int nDY = (pS[nW].n-pS[-nW].n)*fFactor;
					ULONG const nSrcX = nX+(nDX>>8);
					ULONG const nSrcY = nY+(nDY>>8);
					if (nSrcX < m_nSizeX-1 && nSrcY < m_nSizeY-1)
					{
						SOffset const* const p = m_aOffsets.m_p+nSrcY*m_nSizeX+nSrcX;
						ULONG const nX2 = nDX&0xff;
						ULONG const nX1 = 256-nX2;
						ULONG const nY2 = nDY&0xff;
						ULONG const nY1 = 256-nY2;
						ULONG const n11 = (nX1*nY1)>>8;
						ULONG const n12 = (nX2*nY1)>>8;
						ULONG const n21 = (nX1*nY2)>>8;
						ULONG const n22 = (nX2*nY2)>>8;
						SOffset const* const p11 = p;
						SOffset const* const p12 = p+1;
						SOffset const* const p21 = p+m_nSizeX;
						SOffset const* const p22 = p+m_nSizeX+1;
						SOffset const s = {LONG(p11->nDX*n11+p12->nDX*n12+p21->nDX*n21+p22->nDX*n22)>>8, LONG(p11->nDY*n11+p12->nDY*n12+p21->nDY*n21+p22->nDY*n22)>>8};
						pT->nDX = nDX+s.nDX;
						pT->nDY = nDY+s.nDY;
					}
					else
					{
						ULONG nSX = nSrcX;
						ULONG nSY = nSrcY;
						if (nSX >= m_nSizeX-1)
							nSX = nSX < 0x7fffffff ? m_nSizeX-1 : 0;
						if (nSY >= m_nSizeY-1)
							nSY = nSY < 0x7fffffff ? m_nSizeY-1 : 0;
						SOffset const* const p = m_aOffsets.m_p+nSY*m_nSizeX+nSX;
						pT->nDX = nDX+p->nDX;
						pT->nDY = nDY+p->nDY;
					}
					++pT;
				}
			}
			pT = cTmp;
			for (LONG nY = nY1; nY < nY2; ++nY)
			{
				SOffset* pD = m_aOffsets.m_p+nY*m_nSizeX+nX1;
				for (LONG nX = nX1; nX < nX2; ++nX, ++pD, ++pT)
				{
					pD->nDX = pT->nDX;
					pD->nDY = pT->nDY;
				}
			}
			AddDirtyRect(nX1, nY1, nX2, nY2);
			M_Window()->RectangleChanged(&rc);
		}
	}

	void AddDirtyRect(int a_nX1, int a_nY1, int a_nX2, int a_nY2)
	{
		if (m_rcDirty.left > a_nX1) m_rcDirty.left = a_nX1;
		if (m_rcDirty.top > a_nY1) m_rcDirty.top = a_nY1;
		if (m_rcDirty.right < a_nX2) m_rcDirty.right = a_nX2;
		if (m_rcDirty.bottom < a_nY2) m_rcDirty.bottom = a_nY2;
	}

private:
	ERasterizationMode m_eRasterMode;
	CEditToolDataShapeShift m_cData;

	RECT m_rcDirty;
	CAutoVectorPtr<SOffset> m_aOffsets;
	CAutoVectorPtr<TRasterImagePixel> m_aSource;
	//CAutoVectorPtr<TRasterImagePixel> m_aCached;
	//RECT m_rcCacheValid;
	ULONG m_nSizeX;
	ULONG m_nSizeY;

	TPixelCoords m_tLastPos;
	bool m_bMouseLeft;
	float m_fLastPressure;
	bool m_bCTRL;
	bool m_bSHIFT;
};

