
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
#include "agg_blur.h"



struct CEditToolDataSymmetry
{
	MIDL_INTERFACE("EE6C9397-7171-4B33-8FD0-9D214DC7DCA8")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataSymmetry const*, InternalData)() = 0;
	};

	CEditToolDataRetouch() : nCount(5)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		//int nLen = SysStringLen(a_bstr);
		//SModeList* pList = GetModeList();
		//size_t nSkip = 0;
		//while (pList->second)
		//{
		//	LPCOLESTR psz1 = pList->second;
		//	LPCOLESTR psz2 = a_bstr;
		//	while (*psz1 == *psz2 && *psz1) {++psz1; ++psz2;}
		//	if (*psz1 == L'\0')
		//	{
		//		nSkip = psz2-a_bstr;
		//		eMode = pList->first;
		//		break;
		//	}
		//	++pList;
		//}
		//if (a_bstr[nSkip] == L'(')
		//{
		//	LPOLESTR psz2 = a_bstr+nSkip;
		//	float f = wcstod(a_bstr+nSkip+1, &psz2);
		//	if (*psz2 == L')')
		//	{
		//		nSkip = psz2+1-a_bstr;
		//		fStrength = f;
		//	}
		//}
		//CLSID tID = GUID_NULL;
		//if (nLen < 36+nSkip || !GUIDFromString(a_bstr+nSkip, &tID) || IsEqualGUID(tID, GUID_NULL))
		//	return S_OK;
		//pInternal = NULL;
		//RWCoCreateInstance(pInternal, tID);
		//if (pInternal)
		//{
		//	CComBSTR bstr(a_bstr+36+nSkip);
		//	pInternal->FromText(bstr);
		//}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		CComBSTR bstr;
		//SModeList* pList = GetModeList();
		//while (pList->second)
		//{
		//	if (pList->first == eMode)
		//	{
		//		bstr = pList->second;
		//		break;
		//	}
		//	++pList;
		//}
		//wchar_t szStr[20];
		//swprintf(szStr, L"(%f)", fStrength);
		//bstr += szStr;
		//if (pInternal)
		//{
		//	CLSID tID = GUID_NULL;
		//	pInternal->CLSIDGet(&tID);
		//	if (!IsEqualGUID(tID, GUID_NULL))
		//	{
		//		wchar_t szTmp[64];
		//		StringFromGUID(tID, szTmp);
		//		CComBSTR bstrInt;
		//		pInternal->ToText(&bstrInt);
		//		if (bstrInt.Length())
		//		{
		//			bstr += szTmp;
		//			bstr += bstrInt;
		//		}
		//	}
		//}
		*a_pbstr = bstr;
		bstr.Detach();
		return S_OK;
	}

	int nCount;
	CComPtr<ISharedState> pInternal;
};

//#include "SharedStateEditToolRetouch.h"
//#include "EditToolRetouchDlg.h"


class CEditToolSymmetry :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditWindow,
	public IRasterImageEditToolScripting
{
public:
	CEditToolSymmetry() : m_pCallback(NULL)
	{
	}
	~CEditToolSymmetry()
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

	//static HRESULT WINAPI QICustomToolInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	//{
	//	CEditToolSymmetry* const p = reinterpret_cast<CEditToolSymmetry*>(pv);
	//	if (p->m_pTool)
	//		return p->m_pTool->QueryInterface(riid, ppv);
	//	return E_NOINTERFACE;
	//}

	BEGIN_COM_MAP(CEditToolSymmetry)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		//// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		//COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, EITIBackground, a_pBuffer);
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
		m_pTool->GetImageTile(rcCov.left, rcCov.top, sz.cx, sz.cy, sz.cx, cCov);
		if (m_cData.eMode == CEditToolDataRetouch::EMBrighten ||
			m_cData.eMode == CEditToolDataRetouch::EMDarken)
		{
			int nFactor = m_cData.eMode == CEditToolDataRetouch::EMBrighten ? floorf(m_cData.fStrength*2.56f+0.5f) : -floorf(m_cData.fStrength*2.56f+0.5f);
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					ULONG n = ULONG(pC->bA)<<5;
					int const rgbmax = pD->bR>pD->bG ? (pD->bR>pD->bB ? pD->bR : pD->bB) : (pD->bG>pD->bB ? pD->bG : pD->bB);
					int const rgbmin = pD->bR<pD->bG ? (pD->bR<pD->bB ? pD->bR : pD->bB) : (pD->bG<pD->bB ? pD->bG : pD->bB);
					unsigned int nL = (rgbmax + rgbmin)<<7;
					unsigned int nH = 0;
					unsigned int nS = 0;
					unsigned int const rgbdelta = rgbmax - rgbmin;
					if (rgbdelta)
					{
						if (nL <= 0x7fff)
							nS = ((rgbmax - rgbmin)*(0xffff)) / (rgbmax + rgbmin);
						else
							nS = ((rgbmax - rgbmin)*(0xffff)) / (510 - rgbmax - rgbmin);

						if (pD->bR == rgbmax)
							nH = 0xffff&((pD->bG - pD->bB)*0xffff / int(rgbdelta*6));
						else if (pD->bG == rgbmax)
							nH = 0xffff&(((rgbdelta + rgbdelta + pD->bB - pD->bR)*0xffff) / int(rgbdelta*6));
						else
							nH = 0xffff&((((rgbdelta<<2) + pD->bR - pD->bG)*0xffff) / int(rgbdelta*6));

						// adjust brightness
						int nL2 = ((nFactor*int(n))>>8)+nL;
						nL = nL2 <= 0 ? 0 : (nL2 > 0xffff ? 0xffff : nL2);

						// back to rgb
						unsigned int m2 = nL + (nL <= 0x7fff ? (nL*nS)/0xffff : nS - (nL*nS)/0xffff);
						unsigned int m1 = (nL<<1) - m2;
						pD->bR = IntHLS(m1, m2, nH+0x5555);
						pD->bG = IntHLS(m1, m2, nH);
						pD->bB = IntHLS(m1, m2, nH-0x5555);
					}
					else
					{
						// adjust brightness
						int nL2 = ((nFactor*int(n))>>8)+nL;
						nL = nL2 <= 0 ? 0 : (nL2 > 0xffff ? 0xffff : nL2);

						pD->bR = pD->bG = pD->bB = nL>>8;
					}
				}
				pD += a_nStride-sz.cx;
			}
		}
		else if (m_cData.eMode == CEditToolDataRetouch::EMSoften ||
			m_cData.eMode == CEditToolDataRetouch::EMSharpen)
		{
			RECT rcSrc = rcDirty;
			if (rcSrc.left) --rcSrc.left;
			if (rcSrc.top) --rcSrc.top;
			if (rcSrc.right < nImgSizeX) ++rcSrc.right;
			if (rcSrc.bottom < nImgSizeY) ++rcSrc.bottom;
			CAutoVectorPtr<TRasterImagePixel> cSrc(new TRasterImagePixel[(sz.cx+2)*(sz.cy+2)]);
			m_pWindow->GetImageTile(rcSrc.left, rcSrc.top, rcSrc.right-rcSrc.left, rcSrc.bottom-rcSrc.top, sz.cx+2, EITIBackground, cSrc.m_p+1-rcDirty.left+rcSrc.left+(sz.cx+2)*(1-rcDirty.top+rcSrc.top));
			if (rcDirty.left == rcSrc.left)
			{
				for (LONG y = 0; y < sz.cy+2; ++y)
					cSrc[y*(sz.cx+2)] = cSrc[y*(sz.cx+2)+1];
			}
			if (rcDirty.top == rcSrc.top)
			{
				CopyMemory(cSrc.m_p, cSrc.m_p+(sz.cx+2), (sz.cx+2)*sizeof*cSrc.m_p);
			}
			if (rcDirty.right == rcSrc.right)
			{
				for (LONG y = 0; y < sz.cy+2; ++y)
					cSrc[y*(sz.cx+2)+sz.cx+1] = cSrc[y*(sz.cx+2)+sz.cx];
			}
			if (rcDirty.bottom == rcSrc.bottom)
			{
				CopyMemory(cSrc.m_p+(sz.cx+2)*(sz.cy+1), cSrc.m_p+(sz.cx+2)*(sz.cy), (sz.cx+2)*sizeof*cSrc.m_p);
			}
			TRasterImagePixel* pS = cSrc.m_p+(sz.cx+2)+1;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pS, ++pC, ++pD)
				{
					if (pC->bA == 0)
						continue;
					if (m_cData.eMode == CEditToolDataRetouch::EMSoften)
					{
						int nA = int(pS->bA)+pS[-1].bA+pS[1].bA+
							pS[sz.cx+1].bA+pS[sz.cx+2].bA+pS[sz.cx+3].bA+
							pS[-sz.cx-3].bA+pS[-sz.cx-2].bA+pS[-sz.cx-1].bA;
						int nR = int(pS->bA)*pS->bR+int(pS[-1].bA)*pS[-1].bR+int(pS[1].bA)*pS[1].bR+
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bR+int(pS[sz.cx+2].bA)*pS[sz.cx+2].bR+int(pS[sz.cx+3].bA)*pS[sz.cx+3].bR+
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bR+int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bR+int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bR;
						int nG = int(pS->bA)*pS->bG+int(pS[-1].bA)*pS[-1].bG+int(pS[1].bA)*pS[1].bG+
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bG+int(pS[sz.cx+2].bA)*pS[sz.cx+2].bG+int(pS[sz.cx+3].bA)*pS[sz.cx+3].bG+
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bG+int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bG+int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bG;
						int nB = int(pS->bA)*pS->bB+int(pS[-1].bA)*pS[-1].bB+int(pS[1].bA)*pS[1].bB+
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bB+int(pS[sz.cx+2].bA)*pS[sz.cx+2].bB+int(pS[sz.cx+3].bA)*pS[sz.cx+3].bB+
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bB+int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bB+int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bB;
						TRasterImagePixel t;
						if (nA)
						{
							t.bA = nA/9;
							t.bR = nR/nA;
							t.bG = nG/nA;
							t.bB = nB/nA;
						}
						else
						{
							t.bA = t.bR = t.bG = t.bB = 0;
						}
						Mix(*pD, t, pC->bA*m_cData.fStrength*0.04f);
					}
					else
					{
						int nA = int(pS->bA)*12-pS[-1].bA-pS[1].bA-
							pS[sz.cx+1].bA-pS[sz.cx+2].bA-pS[sz.cx+3].bA-
							pS[-sz.cx-3].bA-pS[-sz.cx-2].bA-pS[-sz.cx-1].bA;
						int nR = (int(pS->bA)*12)*pS->bR-int(pS[-1].bA)*pS[-1].bR-int(pS[1].bA)*pS[1].bR-
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bR-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bR-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bR-
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bR-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bR-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bR;
						int nG = (int(pS->bA)*12)*pS->bG-int(pS[-1].bA)*pS[-1].bG-int(pS[1].bA)*pS[1].bG-
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bG-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bG-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bG-
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bG-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bG-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bG;
						int nB = (int(pS->bA)*12)*pS->bB-int(pS[-1].bA)*pS[-1].bB-int(pS[1].bA)*pS[1].bB-
							int(pS[sz.cx+1].bA)*pS[sz.cx+1].bB-int(pS[sz.cx+2].bA)*pS[sz.cx+2].bB-int(pS[sz.cx+3].bA)*pS[sz.cx+3].bB-
							int(pS[-sz.cx-3].bA)*pS[-sz.cx-3].bB-int(pS[-sz.cx-2].bA)*pS[-sz.cx-2].bB-int(pS[-sz.cx-1].bA)*pS[-sz.cx-1].bB;
						TRasterImagePixel t;
						if (nA > 0)
						{
							nA >>= 2;
							nR >>= 2;
							nG >>= 2;
							nB >>= 2;
							int nAA = nA*255;
							if (nR > nAA) nR = nAA;
							if (nG > nAA) nG = nAA;
							if (nB > nAA) nB = nAA;
							t.bR = nR > 0 ? nR/nA : 0;
							t.bG = nG > 0 ? nG/nA : 0;
							t.bB = nB > 0 ? nB/nA : 0;
							if (nA > 255) nA = 255;
							t.bA = nA;
						}
						else
						{
							t.bA = t.bR = t.bG = t.bB = 0;
						}
						Mix(*pD, t, pC->bA*m_cData.fStrength*0.04f);
					}
				}
				pD += a_nStride-sz.cx;
				pS += 2;
			}
		}
		else if (m_cData.eMode == CEditToolDataRetouch::EMAdd)
		{
			int const nStr = m_cData.fStrength*2.57f+0.5f;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			int nR = m_tColor.fR*255.0f+0.5f;
			int nG = m_tColor.fG*255.0f+0.5f;
			int nB = m_tColor.fB*255.0f+0.5f;
			int nA = m_tColor.fA*255.0f+0.5f;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					int n1 = (pC->bA*nStr)>>8;
					int n = pD->bR + ((nR*n1)>>8); pD->bR = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = pD->bG + ((nG*n1)>>8); pD->bG = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = pD->bB + ((nB*n1)>>8); pD->bB = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = pD->bA + ((nA*n1)>>8); pD->bA = n < 0 ? 0 : (n > 255 ? 255 : n);
				}
				pD += a_nStride-sz.cx;
			}
		}
		else if (m_cData.eMode == CEditToolDataRetouch::EMMul)
		{
			int const nStr = m_cData.fStrength*2.57f+0.5f;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			int nR = m_tColor.fR*255.0f+0.5f;
			int nG = m_tColor.fG*255.0f+0.5f;
			int nB = m_tColor.fB*255.0f+0.5f;
			int nA = m_tColor.fA*255.0f+0.5f;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					int n1 = (pC->bA*nStr)>>8;
					int n2 = 256-n1;
					int n = (pD->bR*n2 + (nR*pD->bR/255)*n1)>>8; pD->bR = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = (pD->bG*n2 + (nG*pD->bG/255)*n1)>>8; pD->bG = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = (pD->bB*n2 + (nB*pD->bB/255)*n1)>>8; pD->bB = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = (pD->bA*n2 + (nA*pD->bA/255)*n1)>>8; pD->bA = n < 0 ? 0 : (n > 255 ? 255 : n);
				}
				pD += a_nStride-sz.cx;
			}
		}
		else if (m_cData.eMode == CEditToolDataRetouch::EMDesaturate)
		{
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					float const fSat1 = m_cData.fStrength*pC->bA*0.000039215686f;
					float const fSat2 = 1.0f-fSat1;
					float const fGray = pD->bR*0.299f + pD->bG*0.587f + pD->bB*0.114f;
					int nR = pD->bR*fSat2 + fGray*fSat1 + 0.5f;
					if (nR < 0) pD->bR = 0; else if (nR > 255) pD->bR = 255; else pD->bR = nR;
					int nG = pD->bG*fSat2 + fGray*fSat1 + 0.5f;
					if (nG < 0) pD->bG = 0; else if (nG > 255) pD->bG = 255; else pD->bG = nG;
					int nB = pD->bB*fSat2 + fGray*fSat1 + 0.5f;
					if (nB < 0) pD->bB = 0; else if (nB > 255) pD->bB = 255; else pD->bB = nB;
				}
				pD += a_nStride-sz.cx;
			}
		}
		else if (m_cData.eMode == CEditToolDataRetouch::EMReplaceHue)
		{
			int const nStr = m_cData.fStrength*2.56f+0.5f;
			TRasterImagePixel const* pC = cCov;
			TRasterImagePixel* pD = a_pBuffer + a_nStride*(rcCov.top-a_nY) + rcCov.left-a_nX;
			int nR = m_tColor.fR < 0.0f ? 0 : (m_tColor.fR > 1.0f ? 255 : int(m_tColor.fR*255.0f+0.5f));
			int nG = m_tColor.fG < 0.0f ? 0 : (m_tColor.fG > 1.0f ? 255 : int(m_tColor.fG*255.0f+0.5f));
			int nB = m_tColor.fB < 0.0f ? 0 : (m_tColor.fB > 1.0f ? 255 : int(m_tColor.fB*255.0f+0.5f));

			int const rgbmax = nR>nG ? (nR>nB ? nR : nB) : (nG>nB ? nG : nB);
			int const rgbmin = nR<nG ? (nR<nB ? nR : nB) : (nG<nB ? nG : nB);
			unsigned int nL = (rgbmax + rgbmin)<<7;
			unsigned int nH = 0;
			unsigned int nS = 0;
			unsigned int const rgbdelta = rgbmax - rgbmin;
			if (rgbdelta)
			{
				if (nL <= 0x7fff)
					nS = (rgbdelta*0xffff) / (rgbmax + rgbmin);
				else
					nS = (rgbdelta*0xffff) / (510 - rgbmax - rgbmin);

				if (nR == rgbmax)
					nH = 0xffff&((nG - nB)*0xffff / int(rgbdelta*6));
				else if (nG == rgbmax)
					nH = 0xffff&(((rgbdelta + rgbdelta + nB - nR)*0xffff) / int(rgbdelta*6));
				else
					nH = 0xffff&((((rgbdelta<<2) + nR - nG)*0xffff) / int(rgbdelta*6));
			}

			for (LONG y = 0; y < sz.cy; ++y)
			{
				for (LONG x = 0; x < sz.cx; ++x, ++pC, ++pD)
				{
					if (pC->bA == 0 || pD->bA == 0)
						continue;
					int const rgbmax = pD->bR>pD->bG ? (pD->bR>pD->bB ? pD->bR : pD->bB) : (pD->bG>pD->bB ? pD->bG : pD->bB);
					int const rgbmin = pD->bR<pD->bG ? (pD->bR<pD->bB ? pD->bR : pD->bB) : (pD->bG<pD->bB ? pD->bG : pD->bB);
					nL = (rgbmax + rgbmin)<<7;
					if (nS)
					{
						// back to rgb
						unsigned int m2 = nL + (nL <= 0x7fff ? (nL*nS)/0xffff : nS - (nL*nS)/0xffff);
						unsigned int m1 = (nL<<1) - m2;
						nR = IntHLS(m1, m2, nH+0x5555);
						nG = IntHLS(m1, m2, nH);
						nB = IntHLS(m1, m2, nH-0x5555);
					}
					else
					{
						nR = nG = nB = nL>>8;
					}

					int n1 = (pC->bA*nStr)>>8;
					int n2 = 256-n1;
					int n = (pD->bR*n2 + nR*n1)>>8; pD->bR = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = (pD->bG*n2 + nG*n1)>>8; pD->bG = n < 0 ? 0 : (n > 255 ? 255 : n);
					n = (pD->bB*n2 + nB*n1)>>8; pD->bB = n < 0 ? 0 : (n > 255 ? 255 : n);
				}
				pD += a_nStride-sz.cx;
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
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataRetouch::ISharedStateToolData> pData(a_pState);
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
	STDMETHOD(IRasterImageEditTool::SetColors)(TColor const* a_pColor1, TColor const* a_pColor2)
	{
		if (a_pColor1 &&
			(a_pColor1->fR != m_tColor.fR || a_pColor1->fG != m_tColor.fG ||
			a_pColor1->fB != m_tColor.fB || a_pColor1->fA != m_tColor.fA))
		{
			m_tColor = *a_pColor1;
			if (m_cData.eMode == CEditToolDataRetouch::EMAdd ||
				m_cData.eMode == CEditToolDataRetouch::EMMul ||
				m_cData.eMode == CEditToolDataRetouch::EMReplaceHue)
			{
				RECT rc = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
				m_pTool->IsDirty(&rc, NULL, NULL);
				m_pWindow->RectangleChanged(&rc);
			}
		}
		return S_OK;//m_pTool->SetColors(a_pColor1, a_pColor2);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return S_FALSE;//m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode, EShapeFillMode a_eShapeFillMode, float a_fGamma, float a_fOutlineWidth)
	{
		return m_pTool->SetGlobals(EBMReplace/*a_eBlendingMode*/, a_eRasterizationMode, a_eCoordinatesMode, ESFMFilled/*a_eShapeFillMode*/, a_fGamma, a_fOutlineWidth);
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
		static HCURSOR hCustom = ::LoadCursor(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDC_EDITTOOL_RETOUCH));
		*a_phCursor = hCustom;
		return S_OK;
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

			CEditToolDataRetouch::SModeList const* pModes = CEditToolDataRetouch::GetModeList();
			LPCWSTR pPar = a_bstrParams;
			while (pModes->second)
			{
				LPCWSTR p = pModes->second;
				for (; *p && *p == *pPar; ++p ,++pPar)
					;
				if (!*p)
				{
					m_cData.eMode = pModes->first;
					break;
				}
				++pModes;
			}
			if (*pPar != L',')
				return S_FALSE;
			LPWSTR pPar2 = a_bstrParams;
			m_cData.fStrength = wcstod(pPar, &pPar2);
			if (*pPar2 != L',')
				return S_FALSE;
			return pInternal->FromText(CComBSTR(pPar2+1));
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
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
	STDMETHOD(IRasterImageEditWindow::GetImageTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
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
				return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_eIntent, a_pBuffer);
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
	STDMETHOD(IRasterImageEditWindow::SetColors)(TColor const* a_pColor1, TColor const* a_pColor2)
	{
		return m_pWindow->SetColors(a_pColor1, a_pColor2);
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
	CEditToolDataSymmetry m_cData;
};

