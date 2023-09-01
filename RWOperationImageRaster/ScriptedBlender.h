
#pragma once

#include <agg_pixfmt_rgba.h>


// CScriptedBlender

class ATL_NO_VTABLE CScriptedBlender : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedBlender, &IID_IScriptedBlender, &LIBID_RWOperationImageRasterLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedBlender() : m_fGamma(1.0f)
	{
	}

DECLARE_NOT_AGGREGATABLE(CScriptedBlender)

BEGIN_COM_MAP(CScriptedBlender)
	COM_INTERFACE_ENTRY(IScriptedBlender)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		CComPtr<IGlobalConfigManager> pGCM;
		RWCoCreateInstance(pGCM, __uuidof(GlobalConfigManager));
		if (pGCM)
		{
			CComPtr<IConfig> pConfig;
			pGCM->Config(__uuidof(ColorWindow), &pConfig);
			if (pConfig)
			{
				CConfigValue cVal;
				pConfig->ItemValueGet(CComBSTR(L"Gamma"), &cVal);
				if (cVal.TypeGet() == ECVTFloat)
				{
					m_fGamma = cVal;
				}
			}
		}
		if (m_fGamma < 0.1f) m_fGamma = 1.0f; else if (m_fGamma > 10.0f) m_fGamma = 1.0f;
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IScriptedBlender methods
public:
	STDMETHOD(get_OpClear)(ULONG* pVal) { *pVal = EBOClear; return S_OK; }
	STDMETHOD(get_OpColorBurn)(ULONG* pVal) { *pVal = EBOColorBurn; return S_OK; }
	STDMETHOD(get_OpColorDodge)(ULONG* pVal) { *pVal = EBOColorDodge; return S_OK; }
	STDMETHOD(get_OpContrast)(ULONG* pVal) { *pVal = EBOContrast; return S_OK; }
	STDMETHOD(get_OpDarken)(ULONG* pVal) { *pVal = EBODarken; return S_OK; }
	STDMETHOD(get_OpDifference)(ULONG* pVal) { *pVal = EBODifference; return S_OK; }
	STDMETHOD(get_OpDst)(ULONG* pVal) { *pVal = EBODst; return S_OK; }
	STDMETHOD(get_OpDstAtop)(ULONG* pVal) { *pVal = EBODstAtop; return S_OK; }
	STDMETHOD(get_OpDstIn)(ULONG* pVal) { *pVal = EBODstIn; return S_OK; }
	STDMETHOD(get_OpDstOut)(ULONG* pVal) { *pVal = EBODstOut; return S_OK; }
	STDMETHOD(get_OpDstOver)(ULONG* pVal) { *pVal = EBODstOver; return S_OK; }
	STDMETHOD(get_OpExclusion)(ULONG* pVal) { *pVal = EBOExclusion; return S_OK; }
	STDMETHOD(get_OpHardLight)(ULONG* pVal) { *pVal = EBOHardLight; return S_OK; }
	STDMETHOD(get_OpInvert)(ULONG* pVal) { *pVal = EBOInvert; return S_OK; }
	STDMETHOD(get_OpInvertRGB)(ULONG* pVal) { *pVal = EBOInvertRGB; return S_OK; }
	STDMETHOD(get_OpLighten)(ULONG* pVal) { *pVal = EBOLighten; return S_OK; }
	STDMETHOD(get_OpMinus)(ULONG* pVal) { *pVal = EBOMinus; return S_OK; }
	STDMETHOD(get_OpMultiply)(ULONG* pVal) { *pVal = EBOMultiply; return S_OK; }
	STDMETHOD(get_OpOverlay)(ULONG* pVal) { *pVal = EBOOverlay; return S_OK; }
	STDMETHOD(get_OpPlus)(ULONG* pVal) { *pVal = EBOPlus; return S_OK; }
	STDMETHOD(get_OpScreen)(ULONG* pVal) { *pVal = EBOScreen; return S_OK; }
	STDMETHOD(get_OpSoftLight)(ULONG* pVal) { *pVal = EBOSoftLight; return S_OK; }
	STDMETHOD(get_OpSrc)(ULONG* pVal) { *pVal = EBOSrc; return S_OK; }
	STDMETHOD(get_OpSrcAtop)(ULONG* pVal) { *pVal = EBOSrcAtop; return S_OK; }
	STDMETHOD(get_OpSrcIn)(ULONG* pVal) { *pVal = EBOSrcIn; return S_OK; }
	STDMETHOD(get_OpSrcOut)(ULONG* pVal) { *pVal = EBOSrcOut; return S_OK; }
	STDMETHOD(get_OpSrcOver)(ULONG* pVal) { *pVal = EBOSrcOver; return S_OK; }
	STDMETHOD(get_OpXor)(ULONG* pVal) { *pVal = EBOXor; return S_OK; }
	STDMETHOD(get_OpRGBAPlus)(ULONG* pVal) { *pVal = EBORGBAPlus; return S_OK; }
	STDMETHOD(get_OpRGBAMinus)(ULONG* pVal) { *pVal = EBORGBAMinus; return S_OK; }
	STDMETHOD(get_OpRGBAMultiply)(ULONG* pVal) { *pVal = EBORGBAMultiply; return S_OK; }
	STDMETHOD(get_OpRGBAMultiply2x)(ULONG* pVal) { *pVal = EBORGBAMultiply2x; return S_OK; }
	STDMETHOD(get_OpNormalBumpmap)(ULONG* pVal) { *pVal = EBONormalBumpmap; return S_OK; }
	STDMETHOD(get_OpHeightBumpmap)(ULONG* pVal) { *pVal = EBOHeightBumpmap; return S_OK; }
	STDMETHOD(get_OpNormalDisplace)(ULONG* pVal) { *pVal = EBONormalDisplace; return S_OK; }
	STDMETHOD(get_OpHeightDisplace)(ULONG* pVal) { *pVal = EBOHeightDisplace; return S_OK; }
	STDMETHOD(get_OpHeightToNormal)(ULONG* pVal) { *pVal = EBOHeightToNormal; return S_OK; }
	STDMETHOD(get_OpRGBAMaximum)(ULONG* pVal) { *pVal = EBORGBAMaximum; return S_OK; }
	STDMETHOD(get_OpRGBAMinimum)(ULONG* pVal) { *pVal = EBORGBAMinimum; return S_OK; }
	STDMETHOD(get_OpRGBADifference)(ULONG* pVal) { *pVal = EBORGBADifference; return S_OK; }
	STDMETHOD(get_OpNormalReflection)(ULONG* pVal) { *pVal = EBONormalReflection; return S_OK; }
	STDMETHOD(get_OpMapChannels)(ULONG* pVal) { *pVal = EBOMapChannels; return S_OK; }
	STDMETHOD(get_ChEmpty)(ULONG* pVal) { *pVal = EChEmpty; return S_OK; }
	STDMETHOD(get_ChR)(ULONG* pVal) { *pVal = EChR; return S_OK; }
	STDMETHOD(get_ChG)(ULONG* pVal) { *pVal = EChG; return S_OK; }
	STDMETHOD(get_ChB)(ULONG* pVal) { *pVal = EChB; return S_OK; }
	STDMETHOD(get_ChA)(ULONG* pVal) { *pVal = EChA; return S_OK; }
	STDMETHOD(get_ChY)(ULONG* pVal) { *pVal = EChY; return S_OK; }
	STDMETHOD(get_ChCb)(ULONG* pVal) { *pVal = EChCb; return S_OK; }
	STDMETHOD(get_ChCr)(ULONG* pVal) { *pVal = EChCr; return S_OK; }
	STDMETHOD(get_ChH)(ULONG* pVal) { *pVal = EChH; return S_OK; }
	STDMETHOD(get_ChL)(ULONG* pVal) { *pVal = EChL; return S_OK; }
	STDMETHOD(get_ChS)(ULONG* pVal) { *pVal = EChS; return S_OK; }
	STDMETHOD(get_DesktopSizeX)(ULONG* pVal){ *pVal = GetSystemMetrics(SM_CXSCREEN); return S_OK; }
	STDMETHOD(get_DesktopSizeY)(ULONG* pVal) { *pVal = GetSystemMetrics(SM_CYSCREEN); return S_OK; }
	STDMETHOD(get_GammaCorrection)(float* pVal) { *pVal = m_fGamma; return S_OK; }
	STDMETHOD(put_GammaCorrection)(float newVal) { m_fGamma = newVal; return S_OK; }

	STDMETHOD(CreateCanvas)(ULONG sizeX, ULONG sizeY, ULONG fill, IScriptedRasterImage** ppCanvas)
	{
		try
		{
			*ppCanvas = NULL;
			CComPtr<IDocumentFactoryRasterImage> pFact;
			RWCoCreateInstance(pFact, __uuidof(DocumentFactoryRasterImage));
			TImageSize tSize = {sizeX, sizeY};
			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			if (FAILED(pFact->Create(NULL, pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, (fill&0xff00ff00)|((fill>>16)&0xff)|((fill<<16)&0xff0000)), m_fGamma, NULL)))
				return E_FAIL;
			CComQIPtr<IDocument> pDoc(pBase);
			CComPtr<IDocumentRasterImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pImage));
			CComObject<CScriptedRasterImage>* p = NULL;
			CComObject<CScriptedRasterImage>::CreateInstance(&p);
			CComPtr<IScriptedRasterImage> pTmp = p;
			p->Init(pDoc, pImage);
			*ppCanvas = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppCanvas ? E_UNEXPECTED : E_POINTER;
		}
	}
	template<class TBlender>
	static void BlendPixels(TPixelChannel* a_pD, TPixelChannel* const a_pDEnd, TPixelChannel const* a_pS)
	{
		while (a_pD != a_pDEnd)
		{
			a_pD->bR = (UINT(a_pD->bR)*a_pD->bA*0x10101+0x800000)>>24;
			a_pD->bG = (UINT(a_pD->bG)*a_pD->bA*0x10101+0x800000)>>24;
			a_pD->bB = (UINT(a_pD->bB)*a_pD->bA*0x10101+0x800000)>>24;
			TBlender::blend_pix(reinterpret_cast<TBlender::value_type*>(a_pD),
				(UINT(a_pS->bR)*a_pS->bA*0x10101+0x800000)>>24,
				(UINT(a_pS->bG)*a_pS->bA*0x10101+0x800000)>>24,
				(UINT(a_pS->bB)*a_pS->bA*0x10101+0x800000)>>24,
				a_pS->bA, 255);
			if (a_pD->bA)
			{
				// fix agg errors
				if (a_pD->bR > a_pD->bA) a_pD->bR = a_pD->bA;
				if (a_pD->bG > a_pD->bA) a_pD->bG = a_pD->bA;
				if (a_pD->bB > a_pD->bA) a_pD->bB = a_pD->bA;

				UINT i = 0xff0000/a_pD->bA;
				a_pD->bR = (a_pD->bR*i+0x8000)>>16;
				a_pD->bG = (a_pD->bG*i+0x8000)>>16;
				a_pD->bB = (a_pD->bB*i+0x8000)>>16;
			}
			else
			{
				a_pD->bR = a_pD->bG = a_pD->bB = 0;
			}
			++a_pD;
			++a_pS;
		}
	}
	struct BlendOpPlus
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			ULONG const nR = ULONG(a_pD->bR) + a_pS->bR; a_pD->bR = min(255, nR);
			ULONG const nG = ULONG(a_pD->bG) + a_pS->bG; a_pD->bG = min(255, nG);
			ULONG const nB = ULONG(a_pD->bB) + a_pS->bB; a_pD->bB = min(255, nB);
			ULONG const nA = ULONG(a_pD->bA) + a_pS->bA; a_pD->bA = min(255, nA);
		}
	};
	struct BlendOpMinus
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			int const nR = int(a_pD->bR) - int(a_pS->bR); a_pD->bR = max(0, nR);
			int const nG = int(a_pD->bG) - int(a_pS->bG); a_pD->bG = max(0, nG);
			int const nB = int(a_pD->bB) - int(a_pS->bB); a_pD->bB = max(0, nB);
			int const nA = int(a_pD->bA) - int(a_pS->bA); a_pD->bA = max(0, nA);
		}
	};
	struct BlendOpMultiply
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			a_pD->bR = (ULONG(a_pD->bR) * a_pS->bR * 0x10101 + 0x8000)>>24;
			a_pD->bG = (ULONG(a_pD->bG) * a_pS->bG * 0x10101 + 0x8000)>>24;
			a_pD->bB = (ULONG(a_pD->bB) * a_pS->bB * 0x10101 + 0x8000)>>24;
			a_pD->bA = (ULONG(a_pD->bA) * a_pS->bA * 0x10101 + 0x8000)>>24;
		}
	};
	struct BlendOpMultiply2x
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			ULONG const nR = (ULONG(a_pD->bR) * a_pS->bR * 0x8102 + 0x4000)>>22; a_pD->bR = min(255, nR);
			ULONG const nG = (ULONG(a_pD->bG) * a_pS->bG * 0x8102 + 0x4000)>>22; a_pD->bG = min(255, nG);
			ULONG const nB = (ULONG(a_pD->bB) * a_pS->bB * 0x8102 + 0x4000)>>22; a_pD->bB = min(255, nB);
			ULONG const nA = (ULONG(a_pD->bA) * a_pS->bA * 0x8102 + 0x4000)>>22; a_pD->bA = min(255, nA);
		}
	};
	struct BlendOpMaximum
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			a_pD->bR = max(a_pD->bR, a_pS->bR);
			a_pD->bG = max(a_pD->bG, a_pS->bG);
			a_pD->bB = max(a_pD->bB, a_pS->bB);
			a_pD->bA = max(a_pD->bA, a_pS->bA);
		}
	};
	struct BlendOpMinimum
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			a_pD->bR = min(a_pD->bR, a_pS->bR);
			a_pD->bG = min(a_pD->bG, a_pS->bG);
			a_pD->bB = min(a_pD->bB, a_pS->bB);
			a_pD->bA = min(a_pD->bA, a_pS->bA);
		}
	};
	struct BlendOpDifference
	{
		static void blend_pix(TPixelChannel* a_pD, TPixelChannel const* a_pS)
		{
			a_pD->bR = a_pD->bR > a_pS->bR ? a_pD->bR-a_pS->bR : a_pS->bR-a_pD->bR;
			a_pD->bG = a_pD->bG > a_pS->bG ? a_pD->bG-a_pS->bG : a_pS->bG-a_pD->bG;
			a_pD->bB = a_pD->bB > a_pS->bB ? a_pD->bB-a_pS->bB : a_pS->bB-a_pD->bB;
			a_pD->bA = a_pD->bA > a_pS->bA ? a_pD->bA-a_pS->bA : a_pS->bA-a_pD->bA;
		}
	};
	template<class TBlender>
	static void SimpleBlend(TPixelChannel* a_pD, TPixelChannel* const a_pDEnd, TPixelChannel const* a_pS)
	{
		while (a_pD != a_pDEnd)
		{
			TBlender::blend_pix(a_pD, a_pS);
			++a_pD;
			++a_pS;
		}
	}
	static void BumpMap(TPixelChannel* a_pD, TPixelChannel* const a_pDEnd, TPixelChannel const* a_pS, VARIANT& a_par)
	{
		CComVariant cPar;
		float fX = -0.4f;
		float fY = 0.4f;
		float fZ = sqrtf(1.0f-fX*fX-fY*fY);
		if (a_par.vt == VT_BSTR)
		{
			swscanf(a_par.bstrVal, L"%f,%f,%f", &fX, &fY, &fZ);
		}
		else if (SUCCEEDED(cPar.ChangeType(VT_I4, &a_par)))
		{
			fX = (cPar.intVal&0xff)/127.0f-1.0f;
			fY = ((cPar.intVal>>8)&0xff)/127.0f-1.0f;
			fZ = ((cPar.intVal>>16)&0xff)/127.0f-1.0f;
		}

		float fLevel = (fZ*200.0f);

		while (a_pD != a_pDEnd)
		{
			if (a_pD->bA)
			{
				float fX2 = a_pS->bR-127;
				float fY2 = a_pS->bG-127;
				float fZ2 = a_pS->bB-127;
				float const f = sqrtf(fX2*fX2+fY2*fY2+fZ2*fZ2)*0.005f;
				if (f > 1e-4f)
				{
					int fL = (fX*fX2 + fY*fY2 + fZ*fZ2)/f-fLevel;
					int iR = fL+a_pD->bR;
					a_pD->bR = iR < 0 ? 0 : (iR > 255 ? 255 : iR);
					int iG = fL+a_pD->bG;
					a_pD->bG = iG < 0 ? 0 : (iG > 255 ? 255 : iG);
					int iB = fL+a_pD->bB;
					a_pD->bB = iB < 0 ? 0 : (iB > 255 ? 255 : iB);
				}
			}
			++a_pD;
			++a_pS;
		}
	}
	static void DisplaceMap(TPixelChannel* a_pD, ULONG const a_nSizeX, ULONG const a_nSizeY, TPixelChannel const* a_pS, VARIANT& a_par)
	{
		CComVariant cPar;
		float fAmount = 1.25f;
		if (a_par.vt == VT_R8)
			fAmount = a_par.dblVal*0.025;
		else if (a_par.vt == VT_R4)
			fAmount = a_par.fltVal*0.025f;
		else if (SUCCEEDED(cPar.ChangeType(VT_R4, &a_par)))
			fAmount = cPar.fltVal*0.025f;

		CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[a_nSizeX*a_nSizeY]);
		CopyMemory(cBuffer.m_p, a_pD, a_nSizeX*a_nSizeY*sizeof*a_pD);
		TPixelChannel* pD = a_pD;
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pS, ++pD)
			{
				LONG x2 = LONG(x) - (LONG(a_pS->bR)-127)*fAmount + 0.5f + a_nSizeX; // +a_nSizeX to keep it positive (in normal cases)
				LONG y2 = LONG(y) + (LONG(a_pS->bG)-127)*fAmount + 0.5f + a_nSizeY;
				x2 %= a_nSizeX;
				y2 %= a_nSizeY;
				*pD = cBuffer[y2*a_nSizeX+x2];
			}
		}
	}
	static void ReflectionMap(TPixelChannel* a_pD, ULONG const a_nSizeX, ULONG const a_nSizeY, TPixelChannel const* a_pS, VARIANT& a_par)
	{
		CComQIPtr<IDocument> pEnvDoc(a_par.vt == VT_UNKNOWN ? a_par.punkVal : a_par.vt == VT_DISPATCH ? a_par.pdispVal : NULL);
		if (pEnvDoc == NULL)
			return;
		CComPtr<IDocumentImage> pEnvImg;
		pEnvDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pEnvImg));
		if (pEnvImg == NULL)
			return;
		TImageSize tEnvSize;
		pEnvImg->CanvasGet(&tEnvSize, NULL, NULL, NULL, NULL);
		ULONG const nEnvX = tEnvSize.nX;
		ULONG const nEnvY = tEnvSize.nY;
		size_t nPixels = nEnvX*nEnvY;
		CAutoVectorPtr<TPixelChannel> pEnvData(new TPixelChannel[nPixels]);
		pEnvImg->TileGet(EICIRGBA, NULL, &tEnvSize, NULL, nPixels, pEnvData, NULL, EIRIAccurate);

		CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[a_nSizeX*a_nSizeY]);
		CopyMemory(cBuffer.m_p, a_pD, a_nSizeX*a_nSizeY*sizeof*a_pD);
		TPixelChannel* pD = a_pD;
		float const fAbsSize = a_nSizeX > a_nSizeY ? a_nSizeX : a_nSizeY;
		float const fRelSize = 0.5f;
		float const fF = fRelSize*2.0f/fAbsSize;
		float const fYH = -fRelSize+fF*(0.5f+0.5f*(fAbsSize-a_nSizeY)/*+a_nTileOffY*/);
		float const fXH = fRelSize-fF*(0.5f+0.5f*(fAbsSize-a_nSizeX)/*+a_nTileOffX*/);
		TPixelChannel const* pO = cBuffer;
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			float const fVY = y*fF+fYH;
			for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pS, ++pD, ++pO)
			{
				if (pD->bA == 0)
					continue;
				float fNX = (a_pS->bR-127)*0.007874015748f;
				float fNY = (a_pS->bG-127)*0.007874015748f;
				float fNZ = (a_pS->bB-127)*0.007874015748f;
				float fN = 1.0f/sqrtf(fNX*fNX+fNY*fNY+fNZ*fNZ);
				fNX *= fN;
				fNY *= fN;
				fNZ *= fN;
				float const fVX = fXH-x*fF;
				float const fVZ = sqrtf(1.0f-fVX*fVX-fVY*fVY);
				float const fVN = fVX*fNX + fVY*fNY + fVZ*fNZ;
				float const fRX = fVX-(fNX+fNX)*fVN;
				float const fRY = fVY-(fNY+fNY)*fVN;
				float const fRZ = fVZ-(fNZ+fNZ)*fVN;
				float fTh = asin(fRY);
				float fRo = atan2f(fRZ, fRX);
				LONG xf = (-fRo/3.14159265359f*0.5+0.25f)*nEnvX*256.0f + 0.5f;
				LONG yf = fTh/3.14159265359f*nEnvY*256.0f + nEnvY*0.5f*256.0f + 0.5f;
				LONG x2 = xf>>8;
				LONG y2 = yf>>8;
				xf &= 0xff;
				yf &= 0xff;
				x2 %= nEnvX;
				y2 %= nEnvY;
				LONG x3 = (x2+1) % nEnvX;
				LONG y3 = (y2+1) % nEnvY;
				ULONG const n11 = (255-xf)*(255-yf);
				ULONG const n12 = xf*(255-yf);
				ULONG const n21 = (255-xf)*yf;
				ULONG const n22 = xf*yf;
				TPixelChannel tR11 = pEnvData[y2*nEnvX+x2];
				TPixelChannel tR12 = pEnvData[y2*nEnvX+x3];
				TPixelChannel tR21 = pEnvData[y3*nEnvX+x2];
				TPixelChannel tR22 = pEnvData[y3*nEnvX+x3];
				ULONG bA = n11*tR11.bA + n12*tR12.bA + n21*tR21.bA + n22*tR22.bA;
				if (bA == 255*255*255)
				{
					ULONG bR = tR11.bR*n11*tR11.bA + tR12.bR*n12*tR12.bA + tR21.bR*n21*tR21.bA + tR22.bR*n22*tR22.bA;
					ULONG bG = tR11.bG*n11*tR11.bA + tR12.bG*n12*tR12.bA + tR21.bG*n21*tR21.bA + tR22.bG*n22*tR22.bA;
					ULONG bB = tR11.bB*n11*tR11.bA + tR12.bB*n12*tR12.bA + tR21.bB*n21*tR21.bA + tR22.bB*n22*tR22.bA;
					pD->bR = bR/bA;
					pD->bG = bG/bA;
					pD->bB = bB/bA;
				}
				else
				{
					ULONG nA2 = 255*255*255-bA;
					ULONG bR = tR11.bR*n11*tR11.bA + tR12.bR*n12*tR12.bA + tR21.bR*n21*tR21.bA + tR22.bR*n22*tR22.bA + pD->bR*nA2;
					ULONG bG = tR11.bG*n11*tR11.bA + tR12.bG*n12*tR12.bA + tR21.bG*n21*tR21.bA + tR22.bG*n22*tR22.bA + pD->bG*nA2;
					ULONG bB = tR11.bB*n11*tR11.bA + tR12.bB*n12*tR12.bA + tR21.bB*n21*tR21.bA + tR22.bB*n22*tR22.bA + pD->bB*nA2;
					pD->bR = bR/(255*255*255);
					pD->bG = bG/(255*255*255);
					pD->bB = bB/(255*255*255);
				}
				//int nR = int(tR.bR)-127 + int(pO->bR>>1);
				//pD->bR = nR < 0 ? 0 : (nR > 255 ? 255 : nR);
				//int nG = int(tR.bG)-127 + int(pO->bG>>1);
				//pD->bG = nG < 0 ? 0 : (nG > 255 ? 255 : nG);
				//int nB = int(tR.bB)-127 + int(pO->bB>>1);
				//pD->bB = nB < 0 ? 0 : (nB > 255 ? 255 : nB);
			}
		}
	}
	void HeightToNormal(ULONG a_nSizeX, ULONG a_nSizeY, TPixelChannel* a_pData, TPixelChannel const* a_pDataSrc = NULL, int a_nStrength = 0x100)
	{
		if (a_pDataSrc == NULL) a_pDataSrc = a_pData;
		CAutoVectorPtr<BYTE> cHeights(new BYTE[a_nSizeX*a_nSizeY]);
		BYTE* pH = cHeights;
		BYTE* pHEnd = pH+a_nSizeX*a_nSizeY;
		TPixelChannel const* pI = a_pDataSrc;
		while (pH < pHEnd)
		{
			*pH = (8*pI->bR + 5*pI->bG + 3*pI->bB)>>4;
			++pH;
			++pI;
		}
		for (int y = 0; y < int(a_nSizeY); ++y)
		{
			int nRow1 = -a_nSizeX;
			int nRow2 = a_nSizeX;
			int nRowSh = 10;
			if (y == 0)
			{
				if (a_nSizeY == 1)
				{
					// single row image
					nRow1 = nRow2 = 0;
				}
				else
				{
					// first row
					nRow1 = 0;
					nRowSh = 9;
				}
			}
			else if (y+1 == int(a_nSizeY))
			{
				// last row
				nRow2 = 0;
				nRowSh = 9;
			}
			TPixelChannel* pR = a_pData+y*a_nSizeX;
			pH = cHeights.m_p+y*a_nSizeX;
			for (int x = 0; x < int(a_nSizeX); ++x)
			{
				int nDX;
				if (x == 0)
				{
					if (a_nSizeX == 1)
					{
						// single column image
						nDX = 0;
					}
					else
					{
						// first column
						nDX = ((int(pH[x+1])-int(pH[x]))*a_nStrength)>>9;
					}
				}
				else if (x+1 == int(a_nSizeX))
				{
					// last column
					nDX = ((int(pH[x])-int(pH[x-1]))*a_nStrength)>>9;
				}
				else
				{
					// ordinary interior pixel
					nDX = ((int(pH[x+1])-int(pH[x-1]))*a_nStrength)>>10;
				}
				int nDY = ((int(pH[x+nRow2])-int(pH[x+nRow1]))*a_nStrength)>>nRowSh;
				int const nPartLen = nDX*nDX + nDY*nDY;
				if (nPartLen > 254*254)
				{
					float const f = 254/sqrtf(nPartLen);
					pR->bR = - nDX*f + 127;
					pR->bG = nDY*f + 127;
					pR->bB = 127;
				}
				else
				{
					pR->bR = - nDX + 127;
					pR->bG = nDY + 127;
					pR->bB = 254-sqrtf(nPartLen);
				}
				++pR;
			}
		}
	}
	static void MapChannels(TPixelChannel* a_pD, TPixelChannel* const a_pDEnd, TPixelChannel const* a_pS, VARIANT& a_par)
	{
		CComVariant cPar;
		if (FAILED(cPar.ChangeType(VT_I4, &a_par)))
			return;
		EChannel eSrc1 = static_cast<EChannel>(cPar.intVal&0xf);
		EChannel eSrc2 = static_cast<EChannel>((cPar.intVal>>4)&0xf);
		EChannel eSrc3 = static_cast<EChannel>((cPar.intVal>>8)&0xf);
		EChannel eSrc4 = static_cast<EChannel>((cPar.intVal>>12)&0xf);
		EChannel eDst1 = static_cast<EChannel>((cPar.intVal>>16)&0xf);
		EChannel eDst2 = static_cast<EChannel>((cPar.intVal>>20)&0xf);
		EChannel eDst3 = static_cast<EChannel>((cPar.intVal>>24)&0xf);
		EChannel eDst4 = static_cast<EChannel>((cPar.intVal>>28)&0xf);
		if ((eDst1 == EChEmpty || eDst1 == eSrc1 || eDst1 == eSrc2 || eDst1 == eSrc3 || eDst1 == eSrc4) &&
			(eDst2 == EChEmpty || eDst2 == eSrc1 || eDst2 == eSrc2 || eDst2 == eSrc3 || eDst2 == eSrc4) &&
			(eDst3 == EChEmpty || eDst3 == eSrc1 || eDst3 == eSrc2 || eDst3 == eSrc3 || eDst3 == eSrc4) &&
			(eDst4 == EChEmpty || eDst4 == eSrc1 || eDst4 == eSrc2 || eDst4 == eSrc3 || eDst4 == eSrc4))
		{
			// channel swapping/duplication only
			int aDst[4];
			int aSrc[4];
			int nValid = 0;
			if (eDst1 != EChEmpty)
			{
				aDst[nValid] = 0;
				aSrc[nValid] = eDst1 == eSrc1 ? 0 : (eDst1 == eSrc2 ? 1 : (eDst1 == eSrc3 ? 2 : 3));
				++nValid;
			}
			if (eDst2 != EChEmpty)
			{
				aDst[nValid] = 1;
				aSrc[nValid] = eDst2 == eSrc1 ? 0 : (eDst2 == eSrc2 ? 1 : (eDst2 == eSrc3 ? 2 : 3));
				++nValid;
			}
			if (eDst3 != EChEmpty)
			{
				aDst[nValid] = 2;
				aSrc[nValid] = eDst3 == eSrc1 ? 0 : (eDst3 == eSrc2 ? 1 : (eDst3 == eSrc3 ? 2 : 3));
				++nValid;
			}
			if (eDst4 != EChEmpty)
			{
				aDst[nValid] = 3;
				aSrc[nValid] = eDst4 == eSrc1 ? 0 : (eDst4 == eSrc2 ? 1 : (eDst4 == eSrc3 ? 2 : 3));
				++nValid;
			}
			if (nValid) while (a_pD != a_pDEnd)
			{
				for (int i = 0; i < nValid; ++i)
					reinterpret_cast<BYTE*>(a_pD)[aDst[i]] = reinterpret_cast<BYTE const*>(a_pS)[aSrc[i]];
				++a_pD;
				++a_pS;
			}
		}
		else
		{
			int nSrc[/*EChCOUNT*/16];
			for (int i = 0; i < EChCOUNT; ++i) nSrc[i] = -1;
			nSrc[eSrc1] = 0;
			nSrc[eSrc2] = 1;
			nSrc[eSrc3] = 2;
			nSrc[eSrc4] = 3;
			if (eDst1 == EChA || eDst2 == EChA || eDst3 == EChA || eDst4 == EChA && nSrc[EChA] == -1)
				return; // A channel missing
			if ((eDst1 >= EChR && eDst1 <= EChB) || (eDst2 >= EChR && eDst2 <= EChB) ||
				(eDst3 >= EChR && eDst3 <= EChB) || (eDst4 >= EChR && eDst4 <= EChB))
			{
				if (nSrc[EChY] >= 0 && nSrc[EChCb] >= 0 && nSrc[EChCr] >= 0)
				{
					// YCbCr->RGB
					while (a_pD != a_pDEnd)
					{
						int nY = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChY]];
						int nCr = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChCr]];
						int nCb = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChCb]];
						int nR = nY + 1.402*(nCr-128);
						int nG = nY - 0.34414*(nCb-128) - 0.71414*(nCr-128);
						int nB = nY + 1.772*(nCb-128);
						switch (eDst1)
						{
						case EChR: a_pD->bR = nR < 0 ? 0 : (nR > 255 ? 255 : nR); break;
						case EChG: a_pD->bR = nG < 0 ? 0 : (nG > 255 ? 255 : nG); break;
						case EChB: a_pD->bR = nB < 0 ? 0 : (nB > 255 ? 255 : nB); break;
						case EChA: a_pD->bR = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst2)
						{
						case EChR: a_pD->bG = nR < 0 ? 0 : (nR > 255 ? 255 : nR); break;
						case EChG: a_pD->bG = nG < 0 ? 0 : (nG > 255 ? 255 : nG); break;
						case EChB: a_pD->bG = nB < 0 ? 0 : (nB > 255 ? 255 : nB); break;
						case EChA: a_pD->bG = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst3)
						{
						case EChR: a_pD->bB = nR < 0 ? 0 : (nR > 255 ? 255 : nR); break;
						case EChG: a_pD->bB = nG < 0 ? 0 : (nG > 255 ? 255 : nG); break;
						case EChB: a_pD->bB = nB < 0 ? 0 : (nB > 255 ? 255 : nB); break;
						case EChA: a_pD->bB = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst4)
						{
						case EChR: a_pD->bA = nR < 0 ? 0 : (nR > 255 ? 255 : nR); break;
						case EChG: a_pD->bA = nG < 0 ? 0 : (nG > 255 ? 255 : nG); break;
						case EChB: a_pD->bA = nB < 0 ? 0 : (nB > 255 ? 255 : nB); break;
						case EChA: a_pD->bA = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						++a_pD;
						++a_pS;
					}
				}
				else if (nSrc[EChH] >= 0 && nSrc[EChL] >= 0 && nSrc[EChS] >= 0)
				{
					// HLS->RGB
				}
			}
			else if ((eDst1 >= EChY && eDst1 <= EChCr) || (eDst2 >= EChY && eDst2 <= EChCr) ||
					 (eDst3 >= EChY && eDst3 <= EChCr) || (eDst4 >= EChY && eDst4 <= EChCr))
			{
				if (nSrc[EChR] >= 0 && nSrc[EChG] >= 0 && nSrc[EChB] >= 0)
				{
					// RGB->YCbCr
					while (a_pD != a_pDEnd)
					{
						int nR = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChR]];
						int nG = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChG]];
						int nB = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChB]];
						int nY = 0.299*nR + 0.587*nG + 0.114*nB;
						int nCr = (nR-nY)*0.713+128;
						int nCb = (nB-nY)*0.565+128;
						switch (eDst1)
						{
						case EChY: a_pD->bR = nY < 0 ? 0 : (nY > 255 ? 255 : nY); break;
						case EChCb: a_pD->bR = nCb < 0 ? 0 : (nCb > 255 ? 255 : nCb); break;
						case EChCr: a_pD->bR = nCr < 0 ? 0 : (nCr > 255 ? 255 : nCr); break;
						case EChA: a_pD->bR = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst2)
						{
						case EChY: a_pD->bG = nY < 0 ? 0 : (nY > 255 ? 255 : nY); break;
						case EChCb: a_pD->bG = nCb < 0 ? 0 : (nCb > 255 ? 255 : nCb); break;
						case EChCr: a_pD->bG = nCr < 0 ? 0 : (nCr > 255 ? 255 : nCr); break;
						case EChA: a_pD->bG = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst3)
						{
						case EChY: a_pD->bB = nY < 0 ? 0 : (nY > 255 ? 255 : nY); break;
						case EChCb: a_pD->bB = nCb < 0 ? 0 : (nCb > 255 ? 255 : nCb); break;
						case EChCr: a_pD->bB = nCr < 0 ? 0 : (nCr > 255 ? 255 : nCr); break;
						case EChA: a_pD->bB = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						switch (eDst4)
						{
						case EChY: a_pD->bA = nY < 0 ? 0 : (nY > 255 ? 255 : nY); break;
						case EChCb: a_pD->bA = nCb < 0 ? 0 : (nCb > 255 ? 255 : nCb); break;
						case EChCr: a_pD->bA = nCr < 0 ? 0 : (nCr > 255 ? 255 : nCr); break;
						case EChA: a_pD->bA = reinterpret_cast<BYTE const*>(a_pS)[nSrc[EChA]]; break;
						}
						++a_pD;
						++a_pS;
					}
				}
				else if (nSrc[EChH] >= 0 && nSrc[EChL] >= 0 && nSrc[EChS] >= 0)
				{
					// HLS->YCbCr
				}
			}
			else if ((eDst1 >= EChH && eDst1 <= EChS) || (eDst2 >= EChH && eDst2 <= EChS) ||
					 (eDst3 >= EChH && eDst3 <= EChS) || (eDst4 >= EChH && eDst4 <= EChS))
			{
				if (nSrc[EChY] >= 0 && nSrc[EChCb] >= 0 && nSrc[EChCr] >= 0)
				{
					// YCbCr->HLS
				}
				else if (nSrc[EChR] >= 0 && nSrc[EChG] >= 0 && nSrc[EChB] >= 0)
				{
					// RGB->HLS
				}
			}
		}
	}

	STDMETHOD(Compose)(IUnknown* dst, LONG dstX1, LONG dstY1, LONG dstX2, LONG dstY2, VARIANT src, LONG srcX1, LONG srcY1, VARIANT outFill, ULONG blendingMode, VARIANT parameter)
	{
		try
		{
			CComQIPtr<IDocument> pDst(dst);
			if (pDst == NULL)
				return E_INVALIDARG;
			CComPtr<IDocumentRasterImage> pDstImage;
			pDst->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDstImage));
			if (pDstImage == NULL)
				return E_INVALIDARG;
			TImageSize tDstSize = {0, 0};
			pDstImage->CanvasGet(&tDstSize, NULL, NULL, NULL, NULL);
			RECT rcDst =
			{
				dstX1 < 0 ? 0 : (dstX1 > LONG(tDstSize.nX) ? tDstSize.nX : dstX1),
				dstY1 < 0 ? 0 : (dstY1 > LONG(tDstSize.nY) ? tDstSize.nY : dstY1),
				dstX2 < 0 ? 0 : (dstX2 > LONG(tDstSize.nX) ? tDstSize.nX : dstX2),
				dstY2 < 0 ? 0 : (dstY2 > LONG(tDstSize.nY) ? tDstSize.nY : dstY2),
			};
			if (rcDst.left >= rcDst.right || rcDst.top >= rcDst.bottom)
				return S_FALSE; // no pixels to blend
			TImagePoint const t0 = {rcDst.left, rcDst.top};
			TImageSize const t10 = {rcDst.right-rcDst.left, rcDst.bottom-rcDst.top};
			CAutoVectorPtr<TPixelChannel> cDst(new TPixelChannel[t10.nX*t10.nY]);
			pDstImage->TileGet(EICIRGBA, &t0, &t10, NULL, t10.nX*t10.nY, cDst, NULL, EIRIAccurate);
			CAutoVectorPtr<TPixelChannel> cSrc(new TPixelChannel[t10.nX*t10.nY]);
			CComVariant cOutFill;
			bool bOutFill = SUCCEEDED(cOutFill.ChangeType(VT_UINT, &outFill));
			if (bOutFill)
			{
				ULONG const nFill = (cOutFill.uintVal&0xff00ff00)|((cOutFill.uintVal>>16)&0xff)|((cOutFill.uintVal<<16)&0xff0000);
				std::fill_n(cSrc.m_p, (rcDst.right-rcDst.left)*(rcDst.bottom-rcDst.top), *reinterpret_cast<TPixelChannel const*>(&nFill));
			}
			CComQIPtr<IDocument> pSrc(src.vt == VT_UNKNOWN ? src.punkVal : src.vt == VT_DISPATCH ? src.pdispVal : NULL);
			CComPtr<IDocumentImage> pSrcImage;
			if (pSrc) pSrc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pSrcImage));
			TImageSize tSrcSize = {0, 0};
			if (pSrcImage) pSrcImage->CanvasGet(&tSrcSize, NULL, NULL, NULL, NULL);
			if ((tSrcSize.nX == 0 || tSrcSize.nY == 0) && !bOutFill)
				return E_RW_INVALIDPARAM; // either outFill or src must be valid
			if (bOutFill)
			{
				LONG srcX = srcX1-dstX1+rcDst.left;
				LONG srcY = srcY1-dstY1+rcDst.top;
				LONG srcX2 = srcX+ULONG(rcDst.right-rcDst.left);
				LONG srcY2 = srcY+ULONG(rcDst.bottom-rcDst.top);
				RECT rcSrc =
				{
					srcX < 0 ? 0 : min(srcX, LONG(tSrcSize.nX)),
					srcY < 0 ? 0 : min(srcY, LONG(tSrcSize.nY)),
					srcX2 < 0 ? 0 : min(srcX2, LONG(tSrcSize.nX)),
					srcY2 < 0 ? 0 : min(srcY2, LONG(tSrcSize.nY)),
				};
				if (rcSrc.left < rcSrc.right && rcSrc.top < rcSrc.bottom)
				{
					TImagePoint const tS0 = {rcSrc.left, rcSrc.top};
					TImageSize const tS10 = {rcSrc.right-rcSrc.left, rcSrc.bottom-rcSrc.top};
					TImageStride const tSS = {1, t10.nX};
					pSrcImage->TileGet(EICIRGBA, &tS0, &tS10, &tSS, tSS.nY*tS10.nY, cSrc.m_p+tSS.nY*(rcSrc.top-srcY)+rcSrc.left-srcX, NULL, EIRIAccurate);
				}
			}
			else
			{
				TImageStride const tSS = {1, t10.nX};
				for (LONG nY = rcDst.top; nY < rcDst.bottom; )
				{
					LONG nY1 = (nY-dstY1+srcY1)%tSrcSize.nY;
					LONG nY2 = ((rcDst.bottom-nY) < LONG(tSrcSize.nY)-nY1) ? rcDst.bottom-nY+nY1 : tSrcSize.nY;
					for (LONG nX = rcDst.left; nX < rcDst.right; )
					{
						LONG nX1 = (nX-dstX1+srcX1)%tSrcSize.nX;
						LONG nX2 = ((rcDst.right-nX) < LONG(tSrcSize.nX)-nX1) ? rcDst.right-nX+nX1 : tSrcSize.nX;
						TImagePoint const tS0 = {nX1, nY1};
						TImageSize const tS10 = {nX2-nX1, nY2-nY1};
						pSrcImage->TileGet(EICIRGBA, &tS0, &tS10, &tSS, tSS.nY*tS10.nY, cSrc.m_p+tSS.nY*(nY-rcDst.top)+nX-rcDst.left, NULL, EIRIAccurate);
						nX += nX2-nX1;
					}
					nY += nY2-nY1;
				}
			}
			TPixelChannel* const pDEnd = cDst.m_p+(rcDst.right-rcDst.left)*(rcDst.bottom-rcDst.top);
			switch (blendingMode)
			{
			case EBOClear:		BlendPixels<agg::comp_op_rgba_clear<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOColorBurn:	BlendPixels<agg::comp_op_rgba_color_burn<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOColorDodge:	BlendPixels<agg::comp_op_rgba_color_dodge<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOContrast:	BlendPixels<agg::comp_op_rgba_contrast<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODarken:		BlendPixels<agg::comp_op_rgba_darken<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODifference:	BlendPixels<agg::comp_op_rgba_difference<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODst:		BlendPixels<agg::comp_op_rgba_dst<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODstAtop:	BlendPixels<agg::comp_op_rgba_dst_atop<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODstIn:		BlendPixels<agg::comp_op_rgba_dst_in<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODstOut:		BlendPixels<agg::comp_op_rgba_dst_out<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBODstOver:	BlendPixels<agg::comp_op_rgba_dst_over<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOExclusion:	BlendPixels<agg::comp_op_rgba_exclusion<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOHardLight:	BlendPixels<agg::comp_op_rgba_hard_light<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOInvert:		BlendPixels<agg::comp_op_rgba_invert<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOInvertRGB:	BlendPixels<agg::comp_op_rgba_invert_rgb<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOLighten:	BlendPixels<agg::comp_op_rgba_lighten<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOMinus:		BlendPixels<agg::comp_op_rgba_minus<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOMultiply:	BlendPixels<agg::comp_op_rgba_multiply<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOOverlay:	BlendPixels<agg::comp_op_rgba_overlay<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOPlus:		BlendPixels<agg::comp_op_rgba_plus<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOScreen:		BlendPixels<agg::comp_op_rgba_screen<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSoftLight:	BlendPixels<agg::comp_op_rgba_soft_light<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSrc:		BlendPixels<agg::comp_op_rgba_src<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSrcAtop:	BlendPixels<agg::comp_op_rgba_src_atop<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSrcIn:		BlendPixels<agg::comp_op_rgba_src_in<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSrcOut:		BlendPixels<agg::comp_op_rgba_src_out<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOSrcOver:	BlendPixels<agg::comp_op_rgba_src_over<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBOXor:		BlendPixels<agg::comp_op_rgba_xor<agg::rgba8, agg::order_bgra> >(cDst, pDEnd, cSrc); break;
			case EBORGBAPlus:		SimpleBlend<BlendOpPlus>(cDst, pDEnd, cSrc); break;
			case EBORGBAMinus:		SimpleBlend<BlendOpMinus>(cDst, pDEnd, cSrc); break;
			case EBORGBAMultiply:	SimpleBlend<BlendOpMultiply>(cDst, pDEnd, cSrc); break;
			case EBORGBAMultiply2x:	SimpleBlend<BlendOpMultiply2x>(cDst, pDEnd, cSrc); break;
			case EBORGBAMaximum:	SimpleBlend<BlendOpMaximum>(cDst, pDEnd, cSrc); break;
			case EBORGBAMinimum:	SimpleBlend<BlendOpMinimum>(cDst, pDEnd, cSrc); break;
			case EBORGBADifference:	SimpleBlend<BlendOpDifference>(cDst, pDEnd, cSrc); break;
			case EBOHeightBumpmap:	HeightToNormal(rcDst.right-rcDst.left, rcDst.bottom-rcDst.top, cSrc);
			case EBONormalBumpmap:	BumpMap(cDst, pDEnd, cSrc, parameter); break;
			case EBOMapChannels:	MapChannels(cDst, pDEnd, cSrc, parameter); break;
			case EBOHeightDisplace:	HeightToNormal(rcDst.right-rcDst.left, rcDst.bottom-rcDst.top, cSrc);
			case EBONormalDisplace:	DisplaceMap(cDst, rcDst.right-rcDst.left, rcDst.bottom-rcDst.top, cSrc, parameter); break;
			case EBOHeightToNormal:
				{
					ULONG nStr = 0x100;
					CComVariant cPar;
					if (parameter.vt == VT_R8)
						nStr = parameter.dblVal*0x100;
					else if (parameter.vt == VT_R4)
						nStr = parameter.fltVal*0x100;
					else if (SUCCEEDED(cPar.ChangeType(VT_R4, &parameter)))
						nStr = cPar.fltVal*0x100;
					HeightToNormal(rcDst.right-rcDst.left, rcDst.bottom-rcDst.top, cDst, cSrc, nStr); break;
				}
			case EBONormalReflection:
				ReflectionMap(cDst, rcDst.right-rcDst.left, rcDst.bottom-rcDst.top, cSrc, parameter); break;
			default:
				return E_INVALIDARG;
			}
			return pDstImage->TileSet(EICIRGBA, &t0, &t10, NULL, t10.nX*t10.nY, cDst, FALSE);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanvasFromMask)(IUnknown* context, BSTR maskID, IScriptedRasterImage** ppCanvas)
	{
		try
		{
			*ppCanvas = NULL;
			CComQIPtr<IOperationContext> pContext(context);
			if (pContext == NULL)
				return E_INVALIDARG;
			CComPtr<ISharedStateImageSelection> pSel;
			pContext->StateGet(maskID, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pSel));
			if (pSel == NULL)
				return S_FALSE;
			LONG nX = 0;
			LONG nY = 0;
			ULONG nDX = 0;
			ULONG nDY = 0;
			pSel->Bounds(&nX, &nY, &nDX, &nDY);
			if (nDX == 0 || nDY == 0)
				return S_FALSE;
			CAutoVectorPtr<BYTE> cMask(new BYTE[nDX*nDY]);
			pSel->GetTile(nX, nY, nDX, nDY, nDX, cMask);
			CAutoVectorPtr<TPixelChannel> cImage(new TPixelChannel[nDX*nDY]);
			BYTE const* pS = cMask;
			TPixelChannel* pD = cImage;
			TPixelChannel* pDEnd = pD+nDX*nDY;
			while (pD != pDEnd)
			{
				pD->bR = pD->bG = pD->bB = 0;
				pD->bA = *pS;
				++pD;
				++pS;
			}
			CComPtr<IDocumentFactoryRasterImage> pFact;
			RWCoCreateInstance(pFact, __uuidof(DocumentFactoryRasterImage));
			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			TImageTile t = {EICIRGBA, {nX, nY}, {nDX, nDY}, {1, nDX}, nDX*nDY, cImage};
			if (FAILED(pFact->Create(NULL, pBase, CImageSize(nX+nDX, nY+nDY), NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0xff), 1.0f, &t)))
				return E_FAIL;
			CComQIPtr<IDocument> pDoc(pBase);
			CComPtr<IDocumentRasterImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pImage));
			CComObject<CScriptedRasterImage>* p = NULL;
			CComObject<CScriptedRasterImage>::CreateInstance(&p);
			CComPtr<IScriptedRasterImage> pTmp = p;
			p->Init(pDoc, pImage);
			*ppCanvas = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanvasToMask)(IUnknown* context, BSTR maskID, IUnknown* canvas)
	{
		try
		{
			CComQIPtr<IOperationContext> pContext(context);
			if (pContext == NULL || maskID == NULL || maskID[0] == L'\0') return E_INVALIDARG;
			CComQIPtr<IDocument> pDoc(canvas);
			if (pDoc == NULL) return E_INVALIDARG;
			CComPtr<IDocumentImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
			if (pImage == NULL) return E_INVALIDARG;
			TImagePoint tOrig = {0, 0};
			TImageSize tSize = {1, 1};
			if (FAILED(pImage->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL))) return E_FAIL;
			if (tSize.nX == 0 || tSize.nY == 0)
				tSize.nX = tSize.nY = 1;
			ULONG const nPixels = tSize.nX*tSize.nY;
			CAutoVectorPtr<TPixelChannel> aBuffer(new TPixelChannel[nPixels]);
			pImage->TileGet(EICIRGBA, &tOrig, &tSize, NULL, nPixels, aBuffer, NULL, EIRIAccurate);
			for (ULONG i = 0; i < nPixels; ++i)
				reinterpret_cast<BYTE*>(aBuffer.m_p)[i] = aBuffer[i].bA;
			// TODO: optimize selection
			CComPtr<ISharedStateImageSelection> pState;
			RWCoCreateInstance(pState, __uuidof(SharedStateImageSelection));
			pState->Init(tOrig.nX, tOrig.nY, tSize.nX, tSize.nY, tSize.nX, reinterpret_cast<BYTE*>(aBuffer.m_p));
			return pContext->StateSet(maskID, pState);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(MapChannels)(ULONG src1, ULONG src2, ULONG src3, ULONG src4, ULONG dst1, ULONG dst2, ULONG dst3, ULONG dst4, ULONG* mapCode)
	{
		*mapCode = (src1)|(src2<<4)|(src3<<8)|(src4<<12)|(dst1<<16)|(dst2<<20)|(dst3<<24)|(dst4<<28);
		return S_OK;
	}
	STDMETHOD(GetActiveColor)(IUnknown* context, BSTR colorID, ULONG* pColor)
	{
		try
		{
			*pColor = 0;
			CComQIPtr<IOperationContext> pContext(context);
			if (pContext == NULL)
				return S_FALSE;
			CComPtr<ISharedStateColor> pClr;
			pContext->StateGet(colorID, __uuidof(ISharedStateColor), reinterpret_cast<void**>(&pClr));
			if (pClr == NULL)
				return S_FALSE;
			float f[4] = {0.0f, 0.0f, 0.0f, 0.0f};
			if (FAILED(pClr->RGBAGet(f)))
				return S_FALSE;
			for (int i = 0; i < 4; ++i)
			{
				if (f[i] < 0.0f) f[i] = 0.0f; else if (f[i] > 1.0f) f[i] = 1.0f;
				ULONG n = f[i]*255.0f+0.5f;
				*pColor |= n<<(i*8);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	enum EBlendOp
	{
		EBOClear = 0,
		EBOColorBurn,
		EBOColorDodge,
		EBOContrast,
		EBODarken,
		EBODifference,
		EBODst,
		EBODstAtop,
		EBODstIn,
		EBODstOut,
		EBODstOver,
		EBOExclusion,
		EBOHardLight,
		EBOInvert,
		EBOInvertRGB,
		EBOLighten,
		EBOMinus,
		EBOMultiply,
		EBOOverlay,
		EBOPlus,
		EBOScreen,
		EBOSoftLight,
		EBOSrc,
		EBOSrcAtop,
		EBOSrcIn,
		EBOSrcOut,
		EBOSrcOver,
		EBOXor,
		EBORGBAPlus,
		EBORGBAMinus,
		EBORGBAMultiply,
		EBORGBAMultiply2x,
		EBONormalBumpmap,
		EBOHeightBumpmap,
		EBOMapChannels,
		EBONormalDisplace,
		EBOHeightDisplace,
		EBOHeightToNormal,
		EBORGBAMaximum,
		EBORGBAMinimum,
		EBORGBADifference,
		EBONormalReflection,
		EBO_COUNT
	};
	enum EChannel
	{
		EChEmpty = 0,
		EChR,
		EChG,
		EChB,
		EChA,
		EChY,
		EChCb,
		EChCr,
		EChH,
		EChL,
		EChS,
		EChCOUNT
	};

private:
	float m_fGamma;
};

