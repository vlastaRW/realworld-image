
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
#include "agg_blur.h"
#include "ConstantRasterImageBrush.h"


struct CEditToolDataRecolor
{
	MIDL_INTERFACE("9EB1DB29-A522-418E-A3F8-79509C4E8CC2")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataRecolor const*, InternalData)() = 0;
	};

	CEditToolDataRecolor() : fTolerance(20.0f)
	{
		tToReplace.fR = tToReplace.fG = tToReplace.fB = tToReplace.fA = 0.0f;
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		ULONG nLen = SysStringLen(a_bstr);
		wchar_t const* p = a_bstr;
		wchar_t const* pGuid = NULL;
		int n = 0;
		while (*p)
		{
			if (*p == L',')
			{
				++n;
				if (n == 5)
					pGuid = p+1;
				else if (n == 6)
					break;

			}
			++p;
		}
		if (n != 6)
			return S_FALSE;
		if (5 != swscanf(a_bstr, L"%f,%f,%f,%f,%f", &tToReplace.fR, &tToReplace.fG, &tToReplace.fB, &tToReplace.fA, &fTolerance))
			return S_FALSE;
		CLSID tID = GUID_NULL;
		if (!GUIDFromString(pGuid, &tID) || IsEqualGUID(tID, GUID_NULL))
			return S_OK;
		pInternal = NULL;
		RWCoCreateInstance(pInternal, tID);
		if (pInternal)
		{
			CComBSTR bstr(p+1);
			pInternal->FromText(bstr);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szGuid[64];
		CLSID tID = GUID_NULL;
		if (pInternal)
			pInternal->CLSIDGet(&tID);
		StringFromGUID(tID, szGuid);
		wchar_t szStr[256];
		swprintf(szStr, L"%g,%g,%g,%g,%g,%s,", tToReplace.fR, tToReplace.fG, tToReplace.fB, tToReplace.fA, fTolerance, szGuid);
		CComBSTR bstr = szStr;
		if (pInternal)
		{
			CComBSTR bstrInt;
			pInternal->ToText(&bstrInt);
			if (bstrInt.Length())
				bstr += bstrInt;
		}
		*a_pbstr = bstr;
		bstr.Detach();
		return S_OK;
	}

	TColor tToReplace;
	float fTolerance;
	CComPtr<ISharedState> pInternal;
};

#include "EditToolRecolorDlg.h"


//HICON GetToolIconRETOUCH(ULONG a_nSize);

class CEditToolRecolor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CEditToolCustomPrecisionCursor<GetToolIconRETOUCH>,
	public IRasterImageEditTool,
	public IRasterImageEditWindow,
	public IRasterImageEditToolScripting
{
public:
	CEditToolRecolor() : m_pCallback(NULL), m_pBrushCallback(NULL), m_bPosBefore(false), m_bPosAfter(false), m_fLastPressure(0.0f)
	{
	}
	~CEditToolRecolor()
	{
		m_pTool = NULL;
		if (m_pCallback)
		{
			m_pCallback->SetOwner(NULL);
			m_pCallback->Release();
		}
		if (m_pBrushCallback)
		{
			m_pBrushCallback->SetOwner(NULL);
			m_pBrushCallback->Release();
		}
	}
	void Init(IRasterImageEditTool* a_pInternalTool)
	{
		m_pTool = a_pInternalTool;
	}

	static HRESULT WINAPI QICustomToolInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CEditToolRecolor* const p = reinterpret_cast<CEditToolRecolor*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolRecolor)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	static int IntHLS(unsigned a_nMin, unsigned a_nMax, unsigned a_nAngle)
	{
		unsigned long long a = (a_nAngle+0x2aaa)&0xffff;
		if (a >= 0x8000) a = 0xffff-a;
		if (a < 0x2aaa) a = 0;
		else if (a >= 0x5555) a = 0x2aaa;
		else a -= 0x2aaa;
		return (a_nMin + (a_nMax-a_nMin)*a/0x2aaa);
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, int a_nCoverage)
	{
		int const nSrcR = a_tP1.bR*int(a_tP1.bA);
		int const nSrcG = a_tP1.bG*int(a_tP1.bA);
		int const nSrcB = a_tP1.bB*int(a_tP1.bA);
		int const nSrcA = a_tP1.bA;
		int const nDstR = a_tP2.bR*int(a_tP2.bA);
		int const nDstG = a_tP2.bG*int(a_tP2.bA);
		int const nDstB = a_tP2.bB*int(a_tP2.bA);
		int const nDstA = a_tP2.bA;
		int const nDltR = ((nDstR-nSrcR)*a_nCoverage)>>10;
		int const nDltG = ((nDstG-nSrcG)*a_nCoverage)>>10;
		int const nDltB = ((nDstB-nSrcB)*a_nCoverage)>>10;
		int const nDltA = ((nDstA-nSrcA)*a_nCoverage)>>10;
		int const nFinR = nSrcR+nDltR;
		int const nFinG = nSrcG+nDltG;
		int const nFinB = nSrcB+nDltB;
		int const nFinA = nSrcA+nDltA;
		if (nFinA < 1)
		{
			a_tP1.bR = a_tP1.bG = a_tP1.bB = a_tP1.bA = 0;
		}
		else
		{
			int const nDmlR = nFinR/nFinA;
			int const nDmlG = nFinG/nFinA;
			int const nDmlB = nFinB/nFinA;
			a_tP1.bR = nDmlR < 0 ? 0 : (nDmlR > 255 ? 255 : nDmlR);
			a_tP1.bG = nDmlG < 0 ? 0 : (nDmlG > 255 ? 255 : nDmlG);
			a_tP1.bB = nDmlB < 0 ? 0 : (nDmlB > 255 ? 255 : nDmlB);
			a_tP1.bA = nFinA > 255 ? 255 : nFinA;
		}
	}
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

		if (rcDirty.left >= rcDirty.right || rcDirty.top >= rcDirty.bottom)
			return S_OK;

		RECT rcCov = rcDirty;
		SIZE sz = {rcCov.right-rcCov.left, rcCov.bottom-rcCov.top};
		CAutoVectorPtr<TRasterImagePixel> cCov(new TRasterImagePixel[sz.cx*sz.cy]);
		m_pTool->GetImageTile(rcCov.left, rcCov.top, sz.cx, sz.cy, a_fGamma, sz.cx, cCov);
		//if (m_cData.eMode == CEditToolDataRecolor::EMSharpen)
		//{
		//	RECT rcSrc = rcDirty;
		//	if (rcSrc.left) --rcSrc.left;
		//	if (rcSrc.top) --rcSrc.top;
		//	if (rcSrc.right < LONG(nImgSizeX)) ++rcSrc.right;
		//	if (rcSrc.bottom < LONG(nImgSizeY)) ++rcSrc.bottom;
		//	CAutoVectorPtr<TRasterImagePixel> cSrc(new TRasterImagePixel[(sz.cx+2)*(sz.cy+2)]);
		//	m_pWindow->GetImageTile(rcSrc.left, rcSrc.top, rcSrc.right-rcSrc.left, rcSrc.bottom-rcSrc.top, a_fGamma, sz.cx+2, EITIBackground, cSrc.m_p+1-rcDirty.left+rcSrc.left+(sz.cx+2)*(1-rcDirty.top+rcSrc.top));
		//	if (rcDirty.left == rcSrc.left)
		//	{
		//		for (LONG y = 0; y < sz.cy+2; ++y)
		//			cSrc[y*(sz.cx+2)] = cSrc[y*(sz.cx+2)+1];
		//	}
		//	if (rcDirty.top == rcSrc.top)
		//	{
		//		CopyMemory(cSrc.m_p, cSrc.m_p+(sz.cx+2), (sz.cx+2)*sizeof*cSrc.m_p);
		//	}
		//	if (rcDirty.right == rcSrc.right)
		//	{
		//		for (LONG y = 0; y < sz.cy+2; ++y)
		//			cSrc[y*(sz.cx+2)+sz.cx+1] = cSrc[y*(sz.cx+2)+sz.cx];
		//	}
		//	if (rcDirty.bottom == rcSrc.bottom)
		//	{
		//		CopyMemory(cSrc.m_p+(sz.cx+2)*(sz.cy+1), cSrc.m_p+(sz.cx+2)*(sz.cy), (sz.cx+2)*sizeof*cSrc.m_p);
		//	}
		//	TRasterImagePixel* pS = cSrc.m_p+(sz.cx+2)+1;
		//	TRasterImagePixel const* pC = cCov;
		//	TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
		//	for (LONG y = 0; y < sz.cy; ++y)
		//	{
		//		for (LONG x = 0; x < sz.cx; ++x, ++pS, ++pC, ++pD)
		//		{
		//			if (pC->bA == 0)
		//				continue;
		//			int nA = int(pS->bA)*12-pS[-1].bA-pS[1].bA-
		//				pS[sz.cx+1].bA-pS[sz.cx+2].bA-pS[sz.cx+3].bA-
		//				pS[-sz.cx-3].bA-pS[-sz.cx-2].bA-pS[-sz.cx-1].bA;
		//			int nR = (int(pS->bA)*12)*pS->bR-int(pS[-1].bA)*pS[-1].bR-int(pS[1].bA)*pS[1].bR-
		//				int(pS[sz.cx+1].bA)*pS[sz.cx+1].bR-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bR-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bR-
		//				int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bR-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bR-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bR;
		//			int nG = (int(pS->bA)*12)*pS->bG-int(pS[-1].bA)*pS[-1].bG-int(pS[1].bA)*pS[1].bG-
		//				int(pS[sz.cx+1].bA)*pS[sz.cx+1].bG-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bG-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bG-
		//				int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bG-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bG-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bG;
		//			int nB = (int(pS->bA)*12)*pS->bB-int(pS[-1].bA)*pS[-1].bB-int(pS[1].bA)*pS[1].bB-
		//				int(pS[sz.cx+1].bA)*pS[sz.cx+1].bB-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bB-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bB-
		//				int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bB-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bB-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bB;
		//			TRasterImagePixel t;
		//			if (nA > 0)
		//			{
		//				nA >>= 2;
		//				nR >>= 2;
		//				nG >>= 2;
		//				nB >>= 2;
		//				int nAA = nA*255;
		//				if (nR > nAA) nR = nAA;
		//				if (nG > nAA) nG = nAA;
		//				if (nB > nAA) nB = nAA;
		//				t.bR = nR > 0 ? nR/nA : 0;
		//				t.bG = nG > 0 ? nG/nA : 0;
		//				t.bB = nB > 0 ? nB/nA : 0;
		//				if (nA > 255) nA = 255;
		//				t.bA = nA;
		//			}
		//			else
		//			{
		//				t.bA = t.bR = t.bG = t.bB = 0;
		//			}
		//			Mix(*pD, t, pC->bA*m_cData.fStrength*0.04f);
		//		}
		//		pD += a_nStride-sz.cx;
		//		pS += 2;
		//	}
		//}
		{
			CComPtr<IGammaTableCache> pGTC;
			RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
			CGammaTables const* pGT = pGTC->GetSRGBTable();

			//int const nStr = m_cData.fStrength*2.56f+0.5f;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			TRasterImagePixel tRGBA = {0, 0, 0, 255};
			HLS target = {0, 0, 0};
			{
				if (m_pBrush) m_pBrush->GetBrushTile(0, 0, 1, 1, a_fGamma, 1, &tRGBA);
				int nR = pGT->m_aGamma[tRGBA.bR];
				int nG = pGT->m_aGamma[tRGBA.bG];
				int nB = pGT->m_aGamma[tRGBA.bB];
				target = ToHLS(nR, nG, nB);
			}
			TColor tToReplaceColor = m_cData.tToReplace;
			TRasterImagePixel tToReplace = {CGammaTables::ToSRGB(tToReplaceColor.fB), CGammaTables::ToSRGB(tToReplaceColor.fG), CGammaTables::ToSRGB(tToReplaceColor.fR), 255};//m_cData.tToReplace.fA*255.0f+0.5f};
			if (m_cData.tToReplace.fA != 1.0f)
			{
				if (m_bPosAfter)
				{
					m_pWindow->GetImageTile(m_tPosAfter.fX, m_tPosAfter.fY, 1, 1, 2.2f, 1, EITIBackground, &tToReplace);
					tToReplaceColor.fR = CGammaTables::FromSRGB(tToReplace.bR);
					tToReplaceColor.fG = CGammaTables::FromSRGB(tToReplace.bG);
					tToReplaceColor.fB = CGammaTables::FromSRGB(tToReplace.bB);
				}
				else if (m_bPosBefore)
				{
					m_pWindow->GetImageTile(m_tPosBefore.fX, m_tPosBefore.fY, 1, 1, 2.2f, 1, EITIBackground, &tToReplace);
					tToReplaceColor.fR = CGammaTables::FromSRGB(tToReplace.bR);
					tToReplaceColor.fG = CGammaTables::FromSRGB(tToReplace.bG);
					tToReplaceColor.fB = CGammaTables::FromSRGB(tToReplace.bB);
				}
			}

			HLS source = ToHLS(tToReplaceColor.fR*0xffff, tToReplaceColor.fG*0xffff, tToReplaceColor.fB*0xffff);

			float fByHue = min(source.s/4369.0f, 1.0f);
			if (source.l < 1000 || source.l > 64535)
				fByHue = 0.0f;
			else if (source.l < 4000)
			{
				fByHue *= (source.l-1000)/3000.0f;
			}
			else if (source.l > 61535)
			{
				fByHue *= (64535-source.l)/3000.0f;
			}

			//int delta = m_cData.fTolerance*255/100*3;
			int delta = m_cData.fTolerance*655350/2;

			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					//int const rgbmax = pD->bR>pD->bG ? (pD->bR>pD->bB ? pD->bR : pD->bB) : (pD->bG>pD->bB ? pD->bG : pD->bB);
					//int const rgbmin = pD->bR<pD->bG ? (pD->bR<pD->bB ? pD->bR : pD->bB) : (pD->bG<pD->bB ? pD->bG : pD->bB);
					//nL = (rgbmax + rgbmin)<<7;
					//if (nS)
					//{
					//	// back to rgb
					//	unsigned int m2 = nL + (nL <= 0x7fff ? (nL*nS)/0xffff : nS - (nL*nS)/0xffff);
					//	unsigned int m1 = (nL<<1) - m2;
					//	nR = IntHLS(m1, m2, nH+0x5555);
					//	nG = IntHLS(m1, m2, nH);
					//	nB = IntHLS(m1, m2, nH-0x5555);
					//}
					//else
					//{
					//	nR = nG = nB = nL>>8;
					//}

					//int n1 = (pC->bA*nStr)>>8;
					//int n2 = 256-n1;
					//int n = (pD->bR*n2 + nR*n1)>>8; pD->bR = n < 0 ? 0 : (n > 255 ? 255 : n);
					//n = (pD->bG*n2 + nG*n1)>>8; pD->bG = n < 0 ? 0 : (n > 255 ? 255 : n);
					//n = (pD->bB*n2 + nB*n1)>>8; pD->bB = n < 0 ? 0 : (n > 255 ? 255 : n);
					//if (source.
					HLS current = ToHLS(pGT->m_aGamma[pD->bR], pGT->m_aGamma[pD->bG], pGT->m_aGamma[pD->bB]);
					int wH = fByHue*900;
					int wLS = 1000-wH;
					int d = abs(current.h-source.h)*wH + abs(current.l-source.l)*wLS;

					//int d = abs(int(pD->bR)-int(tToReplace.bR))+
					//		abs(int(pD->bG)-int(tToReplace.bG))+
					//		abs(int(pD->bB)-int(tToReplace.bB));
					if (d < delta)
					{
						BYTE bBl = d < (delta>>1) ? pC->bA : pC->bA*((d-(delta>>1))/(delta>>1));
						HLS res = ReplaceColor(current, current, source, target, fByHue);//pC->bA*(delta-d)/delta);
						//int nR = res.h, nG = res.l, nB = res.s;
						int nR, nG, nB;
						if (res.s)
						{
							// back to rgb
							unsigned int m = ((unsigned int)(res.l*res.s))/0xffff;
							unsigned int m2 = res.l + (res.l <= 0x7fff ? m : res.s - m);
							unsigned int m1 = (res.l<<1) - m2;
							nR = IntHLS(m1, m2, res.h+0x5555);
							nG = IntHLS(m1, m2, res.h);
							nB = IntHLS(m1, m2, res.h-0x5555);
						}
						else
						{
							nR = nG = nB = res.l;
						}
						if (bBl < 255)
						{
							ULONG nIA = 255-bBl;
							nR = (nR*bBl + pGT->m_aGamma[pD->bR]*nIA)/255;
							nG = (nG*bBl + pGT->m_aGamma[pD->bG]*nIA)/255;
							nB = (nB*bBl + pGT->m_aGamma[pD->bB]*nIA)/255;
						}
						pD->bR = pGT->InvGamma(nR);
						pD->bG = pGT->InvGamma(nG);
						pD->bB = pGT->InvGamma(nB);
					}
				}
				pD += a_nStride-sz.cx;
			}
		}

		return S_OK;
	}
	struct HLS
	{
		int h;
		int l;
		int s;
	};
	HLS ToHLS(int nR, int nG, int nB)
	{
		HLS ret;

		//ret.h = nR;
		//ret.l = nG;
		//ret.s = nB;
		//return ret;

		int const rgbmax = nR>nG ? (nR>nB ? nR : nB) : (nG>nB ? nG : nB);
		int const rgbmin = nR<nG ? (nR<nB ? nR : nB) : (nG<nB ? nG : nB);
		unsigned int nL = (rgbmax + rgbmin)>>1;
		unsigned int nH = 0;
		unsigned int nS = 0;
		unsigned int const rgbdelta = rgbmax - rgbmin;
		if (rgbdelta)
		{
			if (nL <= 0x7fff)
				nS = (rgbdelta*0xffff) / (rgbmax + rgbmin);
			else
				nS = (rgbdelta*0xffff) / (0x1fffe - rgbmax - rgbmin);

			int div = int(rgbdelta*6)>>2;
			if (nR == rgbmax)
				nH = 0xffff&((nG - nB)*0x3fff / div);
			else if (nG == rgbmax)
				nH = 0xffff&(((rgbdelta + rgbdelta + nB - nR)*0x3fff) / div);
			else
				nH = 0xffff&((((rgbdelta<<2) + nR - nG)*0x3fff) / div);
		}
		ret.h = nH;
		ret.l = nL;
		ret.s = nS;
		return ret;
	}
	HLS ReplaceColor(HLS base, HLS current, HLS source, HLS target, float byHue)
	{
		HLS ret;
		//if (target.h-source.h > 32768 || target.h-source.h < -32768)
		//{
		//	ret.h = current.h - (source.h-target.h)*strength/255;
		//	if (ret.h < 0) ret.h += 0xffff;
		//	if (ret.h > 0xffff) ret.h -= 0xffff;
		//}
		//else
		//{
		//	ret.h = current.h + (target.h-source.h)*strength/255;
		//	if (ret.h < 0) ret.h += 0xffff;
		//	if (ret.h > 0xffff) ret.h -= 0xffff;
		//}
		ret.h = target.h;
		ret.l = current.l + (target.l-source.l);
		if (ret.l < 0) ret.l = 0;
		ret.s = current.s + (target.s-source.s);
		if (ret.s < 0) ret.s = 0;
		if (ret.s > 0xffff) ret.s = 0xffff;
		if (ret.l >= 0xffff) {ret.l = 0xffff; ret.s = 0;}
		return ret;
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
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataRecolor::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			if (m_cData.pInternal)
				m_pTool->SetState(m_cData.pInternal);
			RECT rc = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
			m_pTool->IsDirty(&rc, NULL, NULL);
			m_pWindow->RectangleChanged(&rc);
		}
		return S_OK;
	}
	STDMETHOD(SetOutline)(BYTE a_bEnabled, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, TColor const* a_pColor)
	{
		return S_FALSE;//m_pTool->SetOutline(a_bEnabled, a_fWidth, a_pColor);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		m_pBrush = a_pBrush;
		if (m_pBrush)
			m_pBrush->Init(M_BrushOwner());
		RECT rc = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
		if (m_pTool && S_OK == m_pTool->IsDirty(&rc, NULL, NULL))
		{
			m_pWindow->RectangleChanged(&rc);
		}
		return S_OK;
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
		float fPrevPressure = m_fLastPressure;
		m_fLastPressure = a_pPos ? a_fNormalPressure : 0.0f;

		if (m_cData.tToReplace.fA == 1.0f || a_pPos == NULL)
			return (~ETPAApply)&m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);

		bool bMouseDown = fPrevPressure < 0.05f && m_fLastPressure > 0.05f;
		if (bMouseDown && (m_bPosBefore = m_pTool->IsDirty(NULL, NULL, NULL) == S_OK))
			m_tPosBefore = *a_pPos;
		HRESULT res = m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
		if (bMouseDown && (m_bPosAfter = m_pTool->IsDirty(NULL, NULL, NULL) == S_OK))
			m_tPosAfter = *a_pPos;
		return (~ETPAApply)&res;
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		return CEditToolCustomPrecisionCursor<GetToolIconRETOUCH>::_GetCursor(a_eKeysState, a_pPos, a_phCursor);
		//static HCURSOR hCustom = ::LoadCursor(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDC_EDITTOOL_RETOUCH));
		//*a_phCursor = hCustom;
		//return S_OK;
		//return m_pTool->GetCursor(a_eKeysState, a_pPos, a_phCursor);
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

			wchar_t const* p = a_bstrParams;
			int n = 0;
			while (*p)
			{
				if (*p == L',')
				{
					++n;
					if (n == 5)
						break;

				}
				++p;
			}
			if (n != 5)
				return S_FALSE;
			if (5 != swscanf(a_bstrParams, L"%f,%f,%f,%f,%f", &m_cData.tToReplace.fR, &m_cData.tToReplace.fG, &m_cData.tToReplace.fB, &m_cData.tToReplace.fA, &m_cData.fTolerance))
				return S_FALSE;

			return pInternal->FromText(CComBSTR(p+1));
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
			wchar_t sz[256];
			swprintf(sz, L"%g, %g, %g, %g, %g, ", m_cData.tToReplace.fR, m_cData.tToReplace.fG, m_cData.tToReplace.fB, m_cData.tToReplace.fA, m_cData.fTolerance);
			CComBSTR bstr(sz);
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
		return m_pWindow->RectangleChanged(a_pChanged);
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

	STDMETHOD(BrushRectangleChanged)(RECT const* a_pChanged)
	{
		RECT rc = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
		if (m_pTool && S_OK == m_pTool->IsDirty(&rc, NULL, NULL))
		{
			m_pWindow->RectangleChanged(&rc);
		}
		return S_OK;
	}

private:
	class CBrushCallbackHelper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IRasterImageBrushOwner
	{
	public:
		void SetOwner(CEditToolRecolor* a_pOwner)
		{
			m_pWindow = a_pOwner;
		}

	BEGIN_COM_MAP(CBrushCallbackHelper)
		COM_INTERFACE_ENTRY(IRasterImageBrushOwner)
	END_COM_MAP()

		// IRasterImageBrushOwner methods
	public:
		STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
		{
			*a_pSizeX = *a_pSizeY = 1;
			return S_OK;
		}
		STDMETHOD(ControlPointsChanged)()
		{
			return S_FALSE;
		}
		STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
		{
			return S_FALSE;
		}
		STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
		{
			if (m_pWindow) return m_pWindow->BrushRectangleChanged(a_pChanged);
			return E_UNEXPECTED;
		}
		STDMETHOD(ControlLinesChanged)()
		{
			return S_FALSE;
		}
		STDMETHOD(SetBrushState)(ISharedState* a_pState)
		{
			return S_FALSE;
		}

	private:
		CEditToolRecolor* m_pWindow;
	};

	IRasterImageBrushOwner* M_BrushOwner()
	{
		if (m_pBrushCallback)
			return m_pBrushCallback;
		CComObject<CBrushCallbackHelper>::CreateInstance(&m_pBrushCallback);
		m_pBrushCallback->AddRef();
		m_pBrushCallback->SetOwner(this);
		return m_pBrushCallback;
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComObject<CBrushCallbackHelper>* m_pBrushCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	CEditToolDataRecolor m_cData;
	CComPtr<IRasterImageBrush> m_pBrush;
	bool m_bPosBefore;
	TPixelCoords m_tPosBefore;
	bool m_bPosAfter;
	TPixelCoords m_tPosAfter;
	float m_fLastPressure;
};

