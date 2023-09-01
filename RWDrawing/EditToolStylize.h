
#pragma once

#include "EditTool.h"
#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
#include <agg_pixfmt_gray.h>
#include <agg_blur.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataStylize
{
	MIDL_INTERFACE("30DE75A3-4DB6-40FF-94A4-E15921AC5BA7")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataStylize const*, InternalData)() = 0;
	};

	CEditToolDataStylize() : bBlur(true), bShadow(true), bReflection(true),
		nShadowSize(19), nShadowDensity(100)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		int nLen = SysStringLen(a_bstr);
		if (nLen < 6)
			return S_FALSE;
		LPCOLESTR psz = a_bstr;
		while (*psz && psz[1] == L':')
		{
			if (*psz == L'B')
			{
				bBlur = wcsncmp(psz+2, L"ON", 2) == 0;
			}
			else if (*psz == L'S')
			{
				bShadow = wcsncmp(psz+2, L"ON", 2) == 0;
				LPCOLESTR pExtra = psz;
				while (*pExtra && *pExtra != L',' && *pExtra != L'|') ++pExtra;
				if (*pExtra == L',')
					swscanf(pExtra+1, L"%i,%i", &nShadowSize, &nShadowDensity);
			}
			else if (*psz == L'R')
			{
				bReflection = wcsncmp(psz+2, L"ON", 2) == 0;
			}
			while (*psz && *psz != L'|') { ++psz; --nLen; }
			if (*psz == L'|') { ++psz; --nLen; }
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
		wchar_t sz[64];
		swprintf(sz, L"B:%s|S:%s,%i,%i|R:%s|", bBlur ? L"ON" : L"OFF", bShadow ? L"ON" : L"OFF", nShadowSize, nShadowDensity, bReflection ? L"ON" : L"OFF");
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

	bool bBlur;
	bool bShadow;
	bool bReflection;
	int nShadowSize;
	int nShadowDensity;
	CComPtr<ISharedState> pInternal;
};

#include "EditToolStylizeDlg.h"


class CEditToolStylize :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditToolScripting,
	public IRasterImageEditWindow
{
public:
	CEditToolStylize() : m_pCallback(NULL), m_rcLastDirty(RECT_EMPTY),
		m_eBlendingMode(EBMDrawOver)
	{
		//m_tColor.fR = m_tColor.fG = m_tColor.fB = m_tColor.fA = 0.0f;
	}
	~CEditToolStylize()
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
		CEditToolStylize* const p = reinterpret_cast<CEditToolStylize*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolStylize)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	static void BoxBlur(ULONG const a_nSizeX, ULONG const a_nSizeY, ULONG const a_nRadius, TRasterImagePixel* a_pData)
	{
		// also premultiply the values
		if (a_nSizeX < a_nRadius+a_nRadius+1 || a_nSizeY < a_nRadius+a_nRadius+1)
			return; // invalid args
		CAutoVectorPtr<TRasterImagePixel> cLine(new TRasterImagePixel[a_nSizeX > a_nSizeY ? a_nSizeX : a_nSizeY]);
		ULONG const nMul = 0x1000000/((a_nRadius+a_nRadius+1)*0xff);
		ULONG const nMulA = 0x1000000/(a_nRadius+a_nRadius+1);
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			ULONG nR = 0;
			ULONG nG = 0;
			ULONG nB = 0;
			ULONG nA = 0;
			TRasterImagePixel* pSub = a_pData;
			for (ULONG x = 0; x < a_nRadius+a_nRadius; ++x, ++a_pData)
			{
				nR += a_pData->bR*ULONG(a_pData->bA);
				nG += a_pData->bG*ULONG(a_pData->bA);
				nB += a_pData->bB*ULONG(a_pData->bA);
				nA += a_pData->bA;
			}
			TRasterImagePixel* pD = cLine;
			TRasterImagePixel* pD2 = a_pData-a_nRadius;
			for (ULONG x = a_nRadius+a_nRadius; x < a_nSizeX; ++x)
			{
				nR += a_pData->bR*ULONG(a_pData->bA);
				nG += a_pData->bG*ULONG(a_pData->bA);
				nB += a_pData->bB*ULONG(a_pData->bA);
				nA += a_pData->bA;
				pD->bR = (nR*nMul+0x800000)>>24;
				pD->bG = (nG*nMul+0x800000)>>24;
				pD->bB = (nB*nMul+0x800000)>>24;
				pD->bA = (nA*nMulA+0x800000)>>24;
				nR -= pSub->bR*ULONG(pSub->bA);
				nG -= pSub->bG*ULONG(pSub->bA);
				nB -= pSub->bB*ULONG(pSub->bA);
				nA -= pSub->bA;
				++a_pData;
				++pD;
				++pSub;
			}
			CopyMemory(pD2, cLine.m_p, (a_nSizeX-a_nRadius-a_nRadius)*sizeof*pD2);
		}
		a_pData -= a_nSizeX*a_nSizeY;
		for (ULONG x = 0; x < a_nSizeX; ++x)
		{
			ULONG nR = 0;
			ULONG nG = 0;
			ULONG nB = 0;
			ULONG nA = 0;
			TRasterImagePixel* pSub = a_pData;
			for (ULONG y = 0; y < a_nRadius+a_nRadius; ++y, a_pData+=a_nSizeX)
			{
				nR += a_pData->bR;
				nG += a_pData->bG;
				nB += a_pData->bB;
				nA += a_pData->bA;
			}
			TRasterImagePixel* pD = cLine.m_p;
			TRasterImagePixel* pD2 = a_pData-a_nRadius*a_nSizeX;
			for (ULONG y = a_nRadius+a_nRadius; y < a_nSizeY; ++y)
			{
				nR += a_pData->bR;
				nG += a_pData->bG;
				nB += a_pData->bB;
				nA += a_pData->bA;
				pD->bR = (nR*nMulA+0x800000)>>24;
				pD->bG = (nG*nMulA+0x800000)>>24;
				pD->bB = (nB*nMulA+0x800000)>>24;
				pD->bA = (nA*nMulA+0x800000)>>24;
				nR -= pSub->bR;
				nG -= pSub->bG;
				nB -= pSub->bB;
				nA -= pSub->bA;
				a_pData += a_nSizeX;
				++pD;
				pSub += a_nSizeX;
			}
			a_pData -= a_nSizeX*a_nSizeY-1;
			pD = cLine.m_p;
			for (ULONG y = a_nRadius+a_nRadius; y < a_nSizeY; ++y)
			{
				*pD2 = *pD;
				++pD;
				pD2 += a_nSizeX;
			}
		}
	}

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		// blur: +-5 pixels
		// drop shadow: +-19 pixels
		bool const bBlur = m_eBlendingMode != EBMReplace && m_cData.bBlur;
		int const nExtraBlur = bBlur ? 5 : 0;
		int const nExtraShadow = m_cData.bShadow ? m_cData.nShadowSize : 0;

		// glass tool only supports paint-over mode
		HRESULT hRes = m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes)) return hRes;

		RECT const rcInternalDirty = m_rcLastDirty;//RECT_EMPTY;
		//m_pTool->IsDirty(&rcInternalDirty, NULL, NULL);
		if (rcInternalDirty.left >= rcInternalDirty.right || rcInternalDirty.top >= rcInternalDirty.bottom)
			return hRes; // if internal tool is not dirty, skip rendering

		float const fReflectionOffsetX = (rcInternalDirty.right+rcInternalDirty.left)*0.05f;
		float const fReflectionOffsetY = (rcInternalDirty.bottom+rcInternalDirty.top)*0.05f;

		ULONG nImgSizeX = 0;
		ULONG nImgSizeY = 0;
		m_pWindow->Size(&nImgSizeX, &nImgSizeY);

		// rectangle affected by this tool
		RECT rcDirty = rcInternalDirty;
		rcDirty.left = rcDirty.left < nExtraShadow ? 0 : rcDirty.left-nExtraShadow;
		rcDirty.top = rcDirty.top < nExtraShadow ? 0 : rcDirty.top-nExtraShadow;
		rcDirty.right = rcDirty.right > LONG(nImgSizeX-nExtraShadow) ? nImgSizeX : rcDirty.right+nExtraShadow;
		rcDirty.bottom = rcDirty.bottom > LONG(nImgSizeY-nExtraShadow) ? nImgSizeY : rcDirty.bottom+nExtraShadow;

		// rectangle to redraw
		RECT const rcRedraw = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};

		if (rcRedraw.left >= rcDirty.right || rcRedraw.top >= rcDirty.bottom ||
			rcRedraw.right <= rcDirty.left || rcRedraw.bottom <= rcDirty.top)
			return hRes; // not redrawing region affected by this tool

		RECT rcDirtyShape =
		{
			max(rcInternalDirty.left, rcRedraw.left),
			max(rcInternalDirty.top, rcRedraw.top),
			min(rcInternalDirty.right, rcRedraw.right),
			min(rcInternalDirty.bottom, rcRedraw.bottom)
		};

		RECT rcSource =
		{
			rcRedraw.left-nExtraShadow,
			rcRedraw.top-nExtraShadow,
			rcRedraw.right+nExtraShadow,
			rcRedraw.bottom+nExtraShadow
		};
		if (rcSource.left < rcDirty.left) rcSource.left = rcDirty.left;
		if (rcSource.top < rcDirty.top) rcSource.top = rcDirty.top;
		if (rcSource.right > rcDirty.right) rcSource.right = rcDirty.right;
		if (rcSource.bottom > rcDirty.bottom) rcSource.bottom = rcDirty.bottom;
		SIZE szSource = {rcSource.right-rcSource.left, rcSource.bottom-rcSource.top};
		ULONG nSource = szSource.cx*szSource.cy;

		CAutoVectorPtr<TRasterImagePixel> cOnBlack(new TRasterImagePixel[nSource]);
		static TRasterImagePixel const tBlack = {0, 0, 0, 0xff};
		std::fill_n(cOnBlack.m_p, nSource, tBlack);
		m_pTool->GetImageTile(rcSource.left, rcSource.top, szSource.cx, szSource.cy, a_fGamma, szSource.cx, cOnBlack);
		CAutoVectorPtr<TRasterImagePixel> cInternal(new TRasterImagePixel[nSource]);
		static TRasterImagePixel const tTransparent = {0, 0, 0, 0};
		std::fill_n(cInternal.m_p, nSource, tTransparent);
		m_pTool->GetImageTile(rcSource.left, rcSource.top, szSource.cx, szSource.cy, a_fGamma, szSource.cx, cInternal);

		for (ULONG i = 0; i < nSource; ++i)
			cOnBlack[i].bA -= cInternal[i].bA;
		// cOnBlack[x].bA -> 0xff=not covered, 0=fully covered

		CAutoVectorPtr<TRasterImagePixel> cBlurred;
		RECT rcBlurred;
		SIZE szBlurred;
		TRasterImagePixel* pBlurred = NULL;
		if (bBlur && rcDirtyShape.left < rcDirtyShape.right && rcDirtyShape.top < rcDirtyShape.bottom)
		{
			rcBlurred.left = rcDirtyShape.left-nExtraBlur;
			rcBlurred.top = rcDirtyShape.top-nExtraBlur;
			rcBlurred.right = rcDirtyShape.right+nExtraBlur;
			rcBlurred.bottom = rcDirtyShape.bottom+nExtraBlur;
			szBlurred.cx = rcBlurred.right-rcBlurred.left;
			szBlurred.cy = rcBlurred.bottom-rcBlurred.top;
			cBlurred.Allocate(szBlurred.cx*szBlurred.cy);
			RECT const rcBlurredClipped = {max(0, rcBlurred.left), max(0, rcBlurred.top), min(LONG(nImgSizeX), rcBlurred.right), min(LONG(nImgSizeY), rcBlurred.bottom)};
			m_pWindow->GetImageTile(rcBlurredClipped.left, rcBlurredClipped.top, rcBlurredClipped.right-rcBlurredClipped.left, rcBlurredClipped.bottom-rcBlurredClipped.top, a_fGamma, rcBlurred.right-rcBlurred.left, EITIBackground, cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(rcBlurredClipped.top-rcBlurred.top)+rcBlurredClipped.left-rcBlurred.left);
			if (rcBlurred.left < rcBlurredClipped.left)
			{
				TRasterImagePixel* p = cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(rcBlurredClipped.top-rcBlurred.top);
				for (LONG y = rcBlurredClipped.top; y < rcBlurredClipped.bottom; ++y)
				{
					for (LONG x = 0; x < -rcBlurred.left; ++x)
						p[x] = p[-rcBlurred.left];
					p += rcBlurred.right-rcBlurred.left;
				}
			}
			if (rcBlurred.right > rcBlurredClipped.right)
			{
				TRasterImagePixel* p = cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(rcBlurredClipped.top-rcBlurred.top)-rcBlurred.left;
				for (LONG y = rcBlurredClipped.top; y < rcBlurredClipped.bottom; ++y)
				{
					for (LONG x = rcBlurredClipped.right; x < rcBlurred.right; ++x)
						p[x] = p[rcBlurredClipped.right-1];
					p += rcBlurred.right-rcBlurred.left;
				}
			}
			for (LONG y = rcBlurred.top; y < rcBlurredClipped.top; ++y)
				CopyMemory(cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(y-rcBlurred.top), cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(-rcBlurred.top), (rcBlurred.right-rcBlurred.left)*sizeof*cBlurred.m_p);
			for (LONG y = rcBlurredClipped.bottom; y < rcBlurred.bottom; ++y)
				CopyMemory(cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(y-rcBlurred.top), cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*(rcBlurredClipped.bottom-1-rcBlurred.top), (rcBlurred.right-rcBlurred.left)*sizeof*cBlurred.m_p);
			pBlurred = cBlurred.m_p+(rcBlurred.right-rcBlurred.left)*nExtraBlur+nExtraBlur;
			BoxBlur(szBlurred.cx, szBlurred.cy, nExtraBlur, cBlurred.m_p);
		}

		CAutoVectorPtr<BYTE> cShadow;
		if (m_cData.bShadow)
		{
			cShadow.Allocate(nSource);
			for (ULONG i = 0; i < nSource; ++i)
				cShadow[i] = cOnBlack[i].bA;
			agg::rendering_buffer rbuf(cShadow, szSource.cx, szSource.cy, szSource.cx);
			agg::pixfmt_gray8 img(rbuf);
			agg::stack_blur_gray8(img, nExtraShadow, nExtraShadow);
			if (m_cData.nShadowDensity != 100)
			{
				ULONG const n = (m_cData.nShadowDensity*256)/100;
				for (ULONG i = 0; i < nSource; ++i)
				{
					ULONG nVal = ((255-cShadow[i])*n)>>8;
					cShadow[i] = nVal > 255 ? 0 : 255-nVal;
				}
			}
			//for (ULONG i = 0; i < nSource; ++i)
			//	cOnBlack[i].bA = cShadow[i];
		}

		// reflection constants
		double fCenterX;
		double fCenterY;
		double fScale = 1.0/3.0;
		if (nImgSizeX > nImgSizeY)
		{
			fCenterX = -4.5*nImgSizeY+nImgSizeX*0.5;
			fCenterY = nImgSizeY*2.0;
		}
		else
		{
			fCenterX = -4.0*nImgSizeX;
			fCenterY = nImgSizeY*0.5+nImgSizeX*1.5;
		}

		RECT const rcFinal =
		{
			max(rcRedraw.left, rcSource.left),
			max(rcRedraw.top, rcSource.top),
			min(rcRedraw.right, rcSource.right),
			min(rcRedraw.bottom, rcSource.bottom),
		};
		for (LONG y = rcFinal.top; y < rcFinal.bottom; ++y)
		{
			if (m_cData.bShadow)
			{
				if (bBlur && y >= rcBlurred.top && y < rcBlurred.bottom)
				{
					// shadow & blur
					BYTE* pSL = cShadow.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pBL = cBlurred.m_p+(y-rcBlurred.top)*szBlurred.cx+rcFinal.left-rcBlurred.left;
					TRasterImagePixel* pCL = cOnBlack.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pIL = cInternal.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pDL = a_pBuffer+(y-rcRedraw.top)*a_nStride;
					for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pIL, ++pSL, ++pCL, ++pBL)
					{
						if (x >= rcBlurred.left && x < rcBlurred.right)
						{
							TRasterImagePixel* pD = pDL+x-rcRedraw.left;
							CPixelMixerReplace::MixPM(*pD, *pBL, 255-pCL->bA);
							ULONG nS = ULONG(255-*pSL)*pCL->bA/255;
							TRasterImagePixel t;
							t.bR = pIL->bR*ULONG(pIL->bA)/255;
							t.bG = pIL->bG*ULONG(pIL->bA)/255;
							t.bB = pIL->bB*ULONG(pIL->bA)/255;
							t.bA = pIL->bA+nS > 255 ? 255 : pIL->bA+nS;
							CPixelMixerPaintOver::MixPM(*pD, t);
						}
						else
						{
							ULONG nS = ULONG(255-*pSL)*pCL->bA/255;
							TRasterImagePixel t;
							t.bR = pIL->bR*ULONG(pIL->bA)/255;
							t.bG = pIL->bG*ULONG(pIL->bA)/255;
							t.bB = pIL->bB*ULONG(pIL->bA)/255;
							t.bA = pIL->bA+nS > 255 ? 255 : pIL->bA+nS;
							CPixelMixerPaintOver::MixPM(pDL[x-rcRedraw.left], t);
						}
					}
				}
				else
				{
					// shadow
					BYTE* pSL = cShadow.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pCL = cOnBlack.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pIL = cInternal.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pDL = a_pBuffer+(y-rcRedraw.top)*a_nStride;
					if (m_eBlendingMode == EBMReplace)
					{
						for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pIL, ++pSL, ++pCL)
						{
							if (pCL->bA == 0)
							{
								CPixelMixerReplace::Mix(pDL[x-rcRedraw.left], *pIL);
								continue;
							}
							static TRasterImagePixel const tShd = {0, 0, 0, 255};
							CPixelMixerReplace::Mix(pDL[x-rcRedraw.left], tShd, 255-*pSL);
							if (pCL->bA != 255)
							{
								ULONG nCov = 255-pCL->bA;
								TRasterImagePixel tS = *pIL;
								tS.bA = nCov < tS.bA ? 255 : tS.bA*255/nCov;
								CPixelMixerReplace::Mix(pDL[x-rcRedraw.left], tS, nCov);
							}
						}
					}
					else
					{
						for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pIL, ++pSL, ++pCL)
						{
							ULONG nS = ULONG(255-*pSL)*pCL->bA/255;
							TRasterImagePixel t;
							t.bR = pIL->bR*ULONG(pIL->bA)/255;
							t.bG = pIL->bG*ULONG(pIL->bA)/255;
							t.bB = pIL->bB*ULONG(pIL->bA)/255;
							t.bA = pIL->bA+nS > 255 ? 255 : pIL->bA+nS;
							CPixelMixerPaintOver::MixPM(pDL[x-rcRedraw.left], t);
						}
					}
				}
			}
			else
			{
				if (bBlur)
				{
					// blur
					TRasterImagePixel* pBL = cBlurred.m_p+(y-rcBlurred.top)*szBlurred.cx+rcFinal.left-rcBlurred.left;
					TRasterImagePixel* pCL = cOnBlack.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pIL = cInternal.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pDL = a_pBuffer+(y-rcRedraw.top)*a_nStride;
					for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pBL, ++pIL, ++pCL)
					{
						TRasterImagePixel* pD = pDL+x-rcRedraw.left;
						CPixelMixerReplace::MixPM(*pD, *pBL, 255-pCL->bA);
						CPixelMixerPaintOver::Mix(*pD, *pIL);
					}
				}
				else
				{
					// no effect
					TRasterImagePixel* pIL = cInternal.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
					TRasterImagePixel* pDL = a_pBuffer+(y-rcRedraw.top)*a_nStride;
					if (m_eBlendingMode == EBMReplace)
					{
						TRasterImagePixel* pCL = cOnBlack.m_p+(y-rcSource.top)*szSource.cx+rcFinal.left-rcSource.left;
						for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pIL, ++pCL)
						{
							if (pCL->bA == 0)
								pDL[x-rcRedraw.left] = *pIL;
							else if (pCL->bA != 255)
							{
								ULONG nCov = 255-pCL->bA;
								TRasterImagePixel tS = *pIL;
								tS.bA = nCov < tS.bA ? 255 : tS.bA*255/nCov;
								CPixelMixerReplace::Mix(pDL[x-rcRedraw.left], tS, nCov);
							}
						}
					}
					else
					{
						for (LONG x = rcFinal.left; x < rcFinal.right; ++x, ++pIL)
						{
							CPixelMixerPaintOver::Mix(pDL[x-rcRedraw.left], *pIL);
						}
					}
				}
			}
			if (m_cData.bReflection && y >= rcDirtyShape.top && y < rcDirtyShape.bottom)
			{
				TRasterImagePixel* pCL = cOnBlack.m_p+(y-rcSource.top)*szSource.cx+rcDirtyShape.left-rcSource.left;
				TRasterImagePixel* pDL = a_pBuffer+(y-rcRedraw.top)*a_nStride;
				for (LONG x = rcDirtyShape.left; x < rcDirtyShape.right; ++x, ++pCL)
				{
					if (pCL->bA != 255)
					{
						double fDistance = sqrt((x-fCenterX-fReflectionOffsetX)*(x-fCenterX-fReflectionOffsetX)+(y-fCenterY-fReflectionOffsetY)*(y-fCenterY-fReflectionOffsetY))*fScale;
						int nDistance = fDistance;
						static struct SReflectionItem { float fBase, fDelta;} const aReflectionMap[128] =
						{
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 

							{0.0f,   0.25f}, {0.25f,  0.25f}, {0.5f,   0.25f}, {0.75f,  0.25f}, 
							{1.0f,   0.0f }, {0.95f, -0.05f}, {0.9f,  -0.05f}, {0.85f, -0.05f}, 
							{0.8f,  -0.05f}, {0.75f, -0.05f}, {0.7f,  -0.05f}, {0.65f, -0.05f}, 
							{0.6f,  -0.05f}, {0.55f, -0.05f}, {0.5f,  -0.05f}, {0.45f, -0.05f}, 
							{0.4f,  -0.05f}, {0.35f, -0.05f}, {0.3f,  -0.05f}, {0.25f, -0.05f}, 
							{0.2f,  -0.05f}, {0.15f, -0.05f}, {0.1f,  -0.05f}, {0.05f, -0.05f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.25f}, {0.25f, 0.25f}, {0.5f, 0.25f}, {0.75f, 0.25f}, 

							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, 
							{1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, -0.25f}, 

							{0.75f, -0.25f}, {0.5f, -0.25f}, {0.25f, -0.25f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
							{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, 
						};
						SReflectionItem const& sReflection = aReflectionMap[nDistance&0x7f];
						ULONG const nAmount = (sReflection.fBase+(fDistance-nDistance)*sReflection.fDelta)*0.125f*(255-pCL->bA);
						TRasterImagePixel t = {255, 255, 255, nAmount};
						CPixelMixerPaintOver::Mix(pDL[x-rcRedraw.left], t);
					}
				}
			}
		}
		return hRes;
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
		CComQIPtr<CEditToolDataStylize::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			int nShadowDensity = m_cData.nShadowDensity;
			int nShadowSize = m_cData.nShadowSize;
			bool bShadow = m_cData.bShadow;
			m_cData = *(pData->InternalData());
			if (m_cData.pInternal)
				m_pTool->SetState(m_cData.pInternal);
			if (bShadow != m_cData.bShadow || (m_cData.bShadow && (nShadowSize != m_cData.nShadowSize || nShadowDensity != m_cData.nShadowDensity)))
			{
				ULONG nX = 0;
				ULONG nY = 0;
				m_pWindow->Size(&nX, &nY);
				int const nShadow = max(nShadowSize, m_cData.nShadowSize);
				RECT rc;
				rc.left = m_rcLastDirty.left < nShadow ? 0 : m_rcLastDirty.left-nShadow;
				rc.top = m_rcLastDirty.top < nShadow ? 0 : m_rcLastDirty.top-nShadow;
				rc.right = m_rcLastDirty.right+nShadow > LONG(nX) ? nX : m_rcLastDirty.right+nShadow;
				rc.bottom = m_rcLastDirty.bottom+nShadow > LONG(nY) ? nY : m_rcLastDirty.bottom+nShadow;
				m_pWindow->RectangleChanged(&rc);
			}
			else
			{
				m_pWindow->RectangleChanged(&m_rcLastDirty);
			}
		}
		return S_OK;
	}
	STDMETHOD(SetOutline)(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		return m_pTool->SetOutline(a_bEnabled, a_fWidth, a_fPos, a_eJoins, a_pColor);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (a_eBlendingMode == EBMDrawUnder)
			a_eBlendingMode = EBMDrawOver;
		HRESULT hRes = m_pTool->SetGlobals(EBMReplace/*a_eBlendingMode*/, a_eRasterizationMode, a_eCoordinatesMode);
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			RECT rc;
			if (S_OK == IsDirty(&rc, NULL, NULL))
				m_pWindow->RectangleChanged(&rc);
		}
		return hRes;
	}

	STDMETHOD(Reset)()
	{
		m_rcLastDirty = RECT_EMPTY;
		return m_pTool->Reset();
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return m_pTool->PreTranslateMessage(a_pMsg);
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (!m_cData.bShadow || a_pImageRect == NULL)
			return m_pTool->IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect);
		RECT rc = *a_pImageRect;
		HRESULT hRes = m_pTool->IsDirty(&rc, a_pOptimizeImageRect, a_pSelectionRect);
		if (rc.left < rc.right && rc.top < rc.bottom)
		{
			ULONG nX = 0;
			ULONG nY = 0;
			m_pWindow->Size(&nX, &nY);
			int const nShadow = m_cData.nShadowSize;
			rc.left = rc.left < nShadow ? 0 : rc.left-nShadow;
			rc.top = rc.top < nShadow ? 0 : rc.top-nShadow;
			rc.right = rc.right+nShadow > LONG(nX) ? nX : rc.right+nShadow;
			rc.bottom = rc.bottom+nShadow > LONG(nY) ? nY : rc.bottom+nShadow;
		}
		*a_pImageRect = rc;
		return hRes;
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
	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE a_bAccurate, float a_fPointSize)
	{
		return m_pTool->PointTest(a_eKeysState, a_pPos, a_bAccurate, a_fPointSize);
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return m_pTool->Transform(a_pMatrix);
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
			bool bBlur = false;
			bool bShadow = false;
			float fShadowSize = 19.0f;
			float fShadowDensity = 100.0f;
			bool bReflection = false;
			std::wstring strCmd;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams), *space_p>>
					(!(str_p(L"\"BLUR\"")[assign_a(bBlur, true)]>>cSep))>>
					(!(str_p(L"\"SHADOW\"")[assign_a(bShadow, true)]>>cSep>>real_p[assign_a(fShadowSize)]>>cSep>>real_p[assign_a(fShadowDensity)]>>cSep))>>
					(!(str_p(L"\"REFLECTION\"")[assign_a(bReflection, true)]>>cSep))>>
					(*anychar_p)[assign_a(strCmd)]
					).full;
			if (!bParsed)
				return E_INVALIDARG;
			int const nShadowSize = m_cData.nShadowSize;
			bool bChange = m_cData.bBlur != bBlur || m_cData.bShadow != bShadow || m_cData.bReflection != bReflection ||
				m_cData.nShadowSize != int(fShadowSize+0.5f) || m_cData.nShadowDensity != int(fShadowDensity+0.5f);
			m_cData.bBlur = bBlur;
			m_cData.bShadow = bShadow;
			m_cData.nShadowSize = fShadowSize+0.5f;
			m_cData.nShadowDensity = fShadowDensity+0.5f;
			m_cData.bReflection = bReflection;
			if (bChange)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				m_pWindow->SetState(pTmp);

				ULONG nX = 0;
				ULONG nY = 0;
				m_pWindow->Size(&nX, &nY);
				int const nShadow = max(nShadowSize, m_cData.nShadowSize);
				RECT rc;
				rc.left = m_rcLastDirty.left < nShadow ? 0 : m_rcLastDirty.left-nShadow;
				rc.top = m_rcLastDirty.top < nShadow ? 0 : m_rcLastDirty.top-nShadow;
				rc.right = m_rcLastDirty.right+nShadow > LONG(nX) ? nX : m_rcLastDirty.right+nShadow;
				rc.bottom = m_rcLastDirty.bottom+nShadow > LONG(nY) ? nY : m_rcLastDirty.bottom+nShadow;
				m_pWindow->RectangleChanged(&rc);
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
			if (!m_cData.bBlur && !m_cData.bShadow && !m_cData.bReflection)
				return pInternal->ToText(a_pbstrParams);
			CComBSTR bstrInt;
			if (S_OK != pInternal->ToText(&bstrInt) || bstrInt == NULL)
				return S_FALSE;
			CComBSTR bstr;
			if (m_cData.bBlur)
				bstr = L"\"BLUR\", ";
			if (m_cData.bShadow)
			{
				WCHAR sz[64];
				swprintf(sz, L"\"SHADOW\", %i, %i, ", m_cData.nShadowSize, m_cData.nShadowDensity);
				bstr += sz;
			}
			if (m_cData.bReflection)
				bstr += L"\"REFLECTION\", ";
			bstr += bstrInt;
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
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
				// TODO: hacked - filled in GetImageTile - handle this scenario correctly

				// deleted in GetImageTile prior to calling internal tool
				//for (ULONG y = 0; y < a_nSizeY; ++y)
				//	ZeroMemory(a_pBuffer+a_nStride*y, a_nSizeX*sizeof*a_pBuffer);
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
		m_pTool->IsDirty(&m_rcLastDirty, NULL, NULL);
		if (!m_cData.bShadow || a_pChanged == NULL)
			return m_pWindow->RectangleChanged(a_pChanged);
		RECT rc = *a_pChanged;
		ULONG nX = 0;
		ULONG nY = 0;
		m_pWindow->Size(&nX, &nY);
		int const nShadow = m_cData.nShadowSize;
		rc.left = a_pChanged->left < nShadow ? 0 : a_pChanged->left-nShadow;
		rc.top = a_pChanged->top < nShadow ? 0 : a_pChanged->top-nShadow;
		rc.right = a_pChanged->right+nShadow > LONG(nX) ? nX : a_pChanged->right+nShadow;
		rc.bottom = a_pChanged->bottom+nShadow > LONG(nY) ? nY : a_pChanged->bottom+nShadow;
		return m_pWindow->RectangleChanged(&rc);
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		m_cData.pInternal = a_pState;
		CComObject<CSharedStateToolData>* pNew = NULL;
		CComObject<CSharedStateToolData>::CreateInstance(&pNew);
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
	CEditToolDataStylize m_cData;
	RECT m_rcLastDirty;
	EBlendingMode m_eBlendingMode;
};

