// DocumentEncoderAPNG.cpp : Implementation of CDocumentEncoderAPNG

#include "stdafx.h"
#include "DocumentEncoderAPNG.h"
#include "DocumentEncoderPNG.h"

#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>


// CDocumentEncoderAPNG

STDMETHODIMP CDocumentEncoderAPNG::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorAPNG::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentEncoderAPNG::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		*a_ppDefCfg = CDocumentEncoderPNG::Config();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderAPNG::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg)
			return S_FALSE; // the standard PNG encoder should take care of this
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_ANIMATION ENCFEAT_IMAGE_ALPHA);
		CComPtr<IDocumentAnimation> pDocAni;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDocAni));
		return pDocAni ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void PNGAPI PNGErrorFnc(png_structp, png_const_charp);
void PNGAPI PNGWarningFnc(png_structp, png_const_charp);

class CPNGStreamWriter
{
public:
	CPNGStreamWriter(IReturnedData* a_pDst) : m_pDst(a_pDst), m_nWritten(0), m_bFailed(false)
	{
	}

	operator png_voidp()
	{
		return reinterpret_cast<png_voidp>(this);
	}

	static void PNGAPI PngRWFnc(png_structp a_pPNGStruct, png_bytep a_pBuffer, png_size_t a_nBufferLen)
	{
		CPNGStreamWriter* pThis = reinterpret_cast<CPNGStreamWriter*>(png_get_io_ptr(a_pPNGStruct));
		BYTE* pOut = NULL;
		if (!pThis->m_bFailed)
		{
			pThis->m_bFailed = FAILED(pThis->m_pDst->Write(a_nBufferLen, a_pBuffer));
			pThis->m_nWritten += a_nBufferLen;
		}
	}
	static void PNGAPI PngFlushFnc(png_structp)
	{
	}

private:
	IReturnedData* m_pDst;
	ULONG m_nWritten;
	bool m_bFailed;
};

struct SPNGFrame
{
	enum { ECompareMask = 0xfffefefe };
	SPNGFrame() : nSizeX(0), nSizeY(0), fDelay(0.1f) {}
	HRESULT Init(IDocumentAnimation* a_pAni, IUnknown* a_pFrame, ULONG a_nAniSizeX, ULONG a_nAniSizeY)
	{
		CComPtr<IDocument> pFrameDoc;
		if (FAILED(a_pAni->FrameGetDoc(a_pFrame, &pFrameDoc)) || pFrameDoc == NULL)
			return E_FAIL;
		CComPtr<IDocumentImage> pFrameImg;
		pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
		if (pFrameImg == NULL)
			return E_FAIL;
		TImageSize tSize = {1, 1};
		pFrameImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		nSizeX = a_nAniSizeX;
		nSizeY = a_nAniSizeY;
		nX0 = (a_nAniSizeX-tSize.nX)>>1;
		nY0 = (a_nAniSizeY-tSize.nY)>>1;
		nX1 = nX0+tSize.nX;
		nY1 = nY0+tSize.nY;
		bDispose = PNG_DISPOSE_OP_BACKGROUND;//PNG_DISPOSE_OP_NONE
		bBlend = PNG_BLEND_OP_SOURCE;
		pData.Attach(new BYTE[nSizeX*nSizeY<<2]);
		if (nSizeX > tSize.nX || nSizeY > tSize.nY)
			ZeroMemory(pData.m_p, nSizeX*nSizeY<<2);
		TImageStride const tStride = {1, nSizeX};
		if (FAILED(pFrameImg->TileGet(EICIRGBA, NULL, &tSize, &tStride, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pData.m_p+((nSizeX*nY0+nX0)<<2)), NULL, EIRIAccurate)))
			return E_FAIL;
		Trim();
		nFX0 = nX0;
		nFX1 = nX1;
		nFY0 = nY0;
		nFY1 = nY1;
		a_pAni->FrameGetTime(a_pFrame, &fDelay);
		return S_OK;
	}
	static bool RowEmpty(BYTE const* p, ULONG const n)
	{
		for (BYTE const* const pEnd = p+(n<<2); p < pEnd; p+=4)
			if (p[3])
				return false;
		return true;
	}
	static bool ColEmpty(BYTE const* p, ULONG const n, ULONG const line)
	{
		for (BYTE const* const pEnd = p+n*line; p < pEnd; p+=line)
			if (p[3])
				return false;
		return true;
	}
	bool Trim()
	{
		// check if there is an empty border and if yes, reduce the frame dimensions
		bool bChange = false;
		ULONG n;
		BYTE const* p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nY0; n < nY1; ++n, p+=nSizeX<<2)
			if (!RowEmpty(p, nX1-nX0))
				break;
		if (n == nY1)
		{
			// image is completely transparent, reduce it to single pixel
			nX0 = nY0 = 0;
			nX1 = nY1 = 1;
			return true;
		}
		if (n > nY0)
		{
			bChange = true;
			nY0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY1+(nX0<<2);
		for (n = nY1; n > nY0; --n, p-=nSizeX<<2)
			if (!RowEmpty(p-(nSizeX<<2), nX1-nX0))
				break;
		if (n < nY1)
		{
			bChange = true;
			nY1 = n;
		}

		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nX0; n < nX1; ++n, p+=4)
			if (!ColEmpty(p, nY1-nY0, nSizeX<<2))
				break;
		if (n > nX0)
		{
			bChange = true;
			nX0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY0+(nX1<<2);
		for (n = nX1; n > nX0; --n, p-=4)
			if (!ColEmpty(p-4, nY1-nY0, nSizeX<<2))
				break;
		if (n < nX1)
		{
			bChange = true;
			nX1 = n;
		}
		if (!bChange)
			return true;//bAlpha;

		bool bNewAlpha = false;
		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (ULONG nY = nY0; nY < nY1; ++nY)
		{
			for (ULONG nX = nX0; nX < nX1; ++nX, p+=4)
			{
				if (p[3] == 0)
					bNewAlpha = true;
			}
			p += (nSizeX-nX1+nX0)<<2;
		}
		return bNewAlpha;
	}

	template<typename TIterator>
	static void OptimizeRectangles(TIterator a_tBegin, TIterator a_tEnd) // optimize logical screen sizes and disposal methods
	{
		if (a_tBegin == a_tEnd)
			return;
		TIterator p = a_tBegin;
		TIterator c = a_tBegin;
		TIterator b = a_tEnd;
		for (++c; c != a_tEnd; ++c, ++p)
		{
			DWORD const* pP = reinterpret_cast<DWORD const*>(p->pData.m_p);
			DWORD const* pC = reinterpret_cast<DWORD const*>(c->pData.m_p);
			LONG nX0 = min(p->nFX0, c->nX0);
			LONG nX1 = max(p->nFX1, c->nX1);
			LONG nY0 = min(p->nFY0, c->nY0);
			LONG nY1 = max(p->nFY1, c->nY1);
			LONG nChgX0 = nX1+1;
			LONG nChgX1 = nX0-1;
			LONG nChgY0 = nY1+1;
			LONG nChgY1 = nY0-1;
			ULONG nChg = 0;
			ULONG n = nY0*c->nSizeX+nX0;
			bool bOpaque = true;
			for (LONG nY = nY0; nY < nY1; ++nY)
			{
				for (LONG nX = nX0; nX < nX1; ++nX, ++n)
				{
					if ((pP[n]^pC[n])&ECompareMask)
					{
						if (nChgX0 > nX) nChgX0 = nX;
						if (nChgY0 > nY) nChgY0 = nY;
						if (nChgX1 <= nX) nChgX1 = nX+1;
						if (nChgY1 <= nY) nChgY1 = nY+1;
						++nChg;
						if (bOpaque && (pC[n]&0xff000000) != 0xff000000)
							bOpaque = false;
					}
				}
				n += c->nSizeX - (nX1-nX0);
			}
			ULONG nCostNoneOver = (nChgY1-nChgY0)*(nChgX1-nChgX0)-1;
			ULONG nCostNoneBlend = bOpaque ? nChg : 0xfffffffe;
			if (nChgX0 >= nChgX1)
			{
				// frames are the same - reduce to 1 pixel and keep the previous one
				c->nX0 = c->nY0 = 0;
				c->nX1 = c->nY1 = 1;
				p->bDispose = PNG_DISPOSE_OP_NONE;
				c->bBlend = PNG_BLEND_OP_SOURCE;
				b = p;
			}
			else
			{
				if (b != a_tEnd)
				{
					DWORD const* pB = reinterpret_cast<DWORD const*>(b->pData.m_p);
					LONG nX0 = min(b->nFX0, c->nX0);
					LONG nX1 = max(b->nFX1, c->nX1);
					LONG nY0 = min(b->nFY0, c->nY0);
					LONG nY1 = max(b->nFY1, c->nY1);
					LONG nBgrX0 = nX1+1;
					LONG nBgrX1 = nX0-1;
					LONG nBgrY0 = nY1+1;
					LONG nBgrY1 = nY0-1;
					ULONG nBgr = 0;
					bool bBgrOpaque = true;
					ULONG n = nY0*c->nSizeX+nX0;
					for (LONG nY = nY0; nY < nY1; ++nY)
					{
						for (LONG nX = nX0; nX < nX1; ++nX, ++n)
						{
							if ((pB[n]^pC[n])&ECompareMask)
							{
								if (nBgrX0 > nX) nBgrX0 = nX;
								if (nBgrY0 > nY) nBgrY0 = nY;
								if (nBgrX1 <= nX) nBgrX1 = nX+1;
								if (nBgrY1 <= nY) nBgrY1 = nY+1;
								++nBgr;
								if (bBgrOpaque && (pC[n]&0xff000000) != 0xff000000)
									bBgrOpaque = false;
							}
						}
						n += c->nSizeX - (nX1-nX0);
					}
					ULONG nCostPrevOver = nBgrY1 > nBgrY0 ? (nBgrY1-nBgrY0)*(nBgrX1-nBgrX0)-1 : 0;
					ULONG nCostPrevBlend = bBgrOpaque ? nBgr : 0xfffffffe;
					if (min(nCostPrevOver, nCostPrevBlend) < min(nCostNoneOver, nCostNoneBlend))
					{
						if (nBgrY0 >= nBgrY1)
						{
							// frames are the same - reduce to 1 pixel and keep the previous one
							c->nX0 = c->nY0 = 0;
							c->nX1 = c->nY1 = 1;
							p->bDispose = PNG_DISPOSE_OP_PREVIOUS;
							c->bBlend = PNG_BLEND_OP_SOURCE;
						}
						else if (min(nCostPrevOver, nCostPrevBlend)+1 < (c->nX1-c->nX0)*(c->nY1-c->nY0)) // for transparent images, the default just may work better
						{
							p->bDispose = PNG_DISPOSE_OP_PREVIOUS;
							c->bBlend = nCostPrevOver < nCostPrevBlend ? PNG_BLEND_OP_SOURCE : PNG_BLEND_OP_OVER;
							c->nX0 = nBgrX0;
							c->nX1 = nBgrX1;
							c->nY0 = nBgrY0;
							c->nY1 = nBgrY1;
						}
						else
						{
							// current frame may need to be expanded to delete old content
							if (LONG(c->nX0) > nChgX0) c->nX0 = nChgX0;
							if (LONG(c->nX1) < nChgX1) c->nX1 = nChgX1;
							if (LONG(c->nY0) > nChgY0) c->nY0 = nChgY0;
							if (LONG(c->nY1) < nChgY1) c->nY1 = nChgY1;
							if (c->nX0 >= c->nX1 || c->nY0 >= c->nY1) // empty frame
							{
								c->nX0 = c->nY0 = 0;
								c->nX1 = c->nY1 = 1;
							}
							b = a_tEnd;
						}
						continue;
					}
				}
				if (min(nCostNoneOver, nCostNoneBlend)+1 < (c->nX1-c->nX0)*(c->nY1-c->nY0)) // for transparent images, the default just may work better
				{
					p->bDispose = PNG_DISPOSE_OP_NONE;
					c->bBlend = nCostNoneOver < nCostNoneBlend ? PNG_BLEND_OP_SOURCE : PNG_BLEND_OP_OVER;
					c->nX0 = nChgX0;
					c->nX1 = nChgX1;
					c->nY0 = nChgY0;
					c->nY1 = nChgY1;
					//if (b == a_tEnd)
						b = p;
				}
				else
				{
					// current frame may need to be expanded to delete old content
					if (LONG(c->nX0) > nChgX0) c->nX0 = nChgX0;
					if (LONG(c->nX1) < nChgX1) c->nX1 = nChgX1;
					if (LONG(c->nY0) > nChgY0) c->nY0 = nChgY0;
					if (LONG(c->nY1) < nChgY1) c->nY1 = nChgY1;
					if (c->nX0 >= c->nX1 || c->nY0 >= c->nY1) // empty frame
					{
						c->nX0 = c->nY0 = 0;
						c->nX1 = c->nY1 = 1;
					}
					b = a_tEnd;
				}
			}
		}
	}
	template<typename TIterator>
	static void OptimizePixels(TIterator a_tBegin, TIterator a_tEnd) // delete duplicate pixels
	{
		if (a_tBegin == a_tEnd)
			return;
		--a_tEnd;
		TIterator p = a_tEnd;
		TIterator c = a_tEnd;
		for (--p; c != a_tBegin; --c, --p)
		{
			if (c->bBlend == PNG_BLEND_OP_SOURCE)
				continue;
			TIterator p2 = p;
			while (p2->bDispose == PNG_DISPOSE_OP_PREVIOUS) --p2;
			if (p2->bDispose != PNG_DISPOSE_OP_NONE)
				continue;
			DWORD const* pP = reinterpret_cast<DWORD const*>(p2->pData.m_p);
			DWORD* pC = reinterpret_cast<DWORD*>(c->pData.m_p);
			ULONG n = c->nY0*c->nSizeX+c->nX0;
			for (ULONG nY = c->nY0; nY < c->nY1; ++nY)
			{
				for (ULONG nX = c->nX0; nX < c->nX1; ++nX, ++n)
				{
					if (((pP[n]^pC[n])&ECompareMask) == 0)
					{
						pC[n] = 0;
					}
				}
				n += c->nSizeX - (c->nX1-c->nX0);
			}
		}
	}

	ULONG nSizeX;
	ULONG nSizeY;
	CAutoVectorPtr<BYTE> pData;
	float fDelay;
	ULONG nX0;
	ULONG nY0;
	ULONG nX1;
	ULONG nY1;
	ULONG nFX0;
	ULONG nFY0;
	ULONG nFX1;
	ULONG nFY1;
	BYTE bDispose;
	BYTE bBlend;
};

int zopfli_deflate(png_bytep raw_data, png_size_t raw_size, png_bytepp comp_data, png_size_tp comp_size, void (**comp_free)(png_bytep));

STDMETHODIMP CDocumentEncoderAPNG::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CComPtr<IDocumentAnimation> pA;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
			return E_NOINTERFACE;
		CComPtr<IEnumUnknowns> pFrames;
		pA->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		if (pFrames) pFrames->Size(&nFrames);
		if (nFrames == 0)
			return E_FAIL; // no frames...

		int interlacing = PNG_INTERLACE_NONE;
		bool optimizing = false;
		bool extraAttr = false;
		if (a_pCfg)
		{
			CConfigValue val;
			a_pCfg->ItemValueGet(CComBSTR(CFGID_INTERLACING), &val);
			if (val)
				interlacing = PNG_INTERLACE_ADAM7;
			a_pCfg->ItemValueGet(CComBSTR(CFGID_EXTRAATTRS), &val);
			extraAttr = val;
			a_pCfg->ItemValueGet(CComBSTR(CFGID_OPTIMIZE), &val);
			if (val.operator LONG()&1)
				optimizing = true;
		}

		ULONG nLoopCount = 0;
		pA->LoopCountGet(&nLoopCount);
		CAutoVectorPtr<SPNGFrame> aPNGFrames(new SPNGFrame[nFrames]);
		// get animation size (bounding rectangle)
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		TImageResolution tResolution = {0, 0, 0, 0};
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			if (pFrame) pA->FrameGetDoc(pFrame, &pFrameDoc);
			CComPtr<IDocumentImage> pFrameImg;
			if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
			TImageSize tSize = {0, 0};
			if (pFrameImg) pFrameImg->CanvasGet(&tSize, i == 0 ? &tResolution : NULL, NULL, NULL, NULL);
			if (tSize.nX > nSizeX) nSizeX = tSize.nX;
			if (tSize.nY > nSizeY) nSizeY = tSize.nY;
		}
		if (nSizeX*nSizeY == 0)
			return E_FAIL;
		// read individual frames
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			SPNGFrame& sF = aPNGFrames.m_p[i];
			if (FAILED(sF.Init(pA, pFrame, nSizeX, nSizeY)))
				return E_FAIL;
		}

		// find rectangles that change
		SPNGFrame::OptimizeRectangles(aPNGFrames.m_p, aPNGFrames.m_p+nFrames);
		// delete same pixels
		SPNGFrame::OptimizePixels(aPNGFrames.m_p, aPNGFrames.m_p+nFrames);

		std::map<DWORD, BYTE> clrs;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			SPNGFrame const& sF = aPNGFrames.m_p[i];

			ULONG nFOffX = i ? sF.nX0 : 0;
			ULONG nFOffY = i ? sF.nY0 : 0;
			ULONG nFSizeX = i ? sF.nX1-sF.nX0 : sF.nSizeX;
			ULONG nFSizeY = i ? sF.nY1-sF.nY0 : sF.nSizeY;
			for (ULONG j = 0; j < nFSizeY; ++j)
			{
				DWORD const* p = reinterpret_cast<DWORD const*>(sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2));
				for (DWORD const* const e = p+nFSizeX; p != e; ++p)
				{
					std::map<DWORD, BYTE>::iterator i = clrs.find(*p);
					if (i == clrs.end())
					{
						size_t const s = clrs.size();
						clrs[*p] = static_cast<BYTE>(s);
						if (clrs.size() > 256)
							break; // too many colors for a paletted image
					}
				}
			}
			if (clrs.size() > 256)
				break; // too many colors for a paletted image
		}
		bool bBW = true;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			SPNGFrame const& sF = aPNGFrames.m_p[i];

			ULONG nFOffX = i ? sF.nX0 : 0;
			ULONG nFOffY = i ? sF.nY0 : 0;
			ULONG nFSizeX = i ? sF.nX1-sF.nX0 : sF.nSizeX;
			ULONG nFSizeY = i ? sF.nY1-sF.nY0 : sF.nSizeY;
			for (ULONG j = 0; j < nFSizeY; ++j)
			{
				DWORD const* p = reinterpret_cast<DWORD const*>(sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2));
				for (DWORD const* const e = p+nFSizeX; p != e; ++p)
				{
					if (GetRValue(*p) != GetGValue(*p) || GetRValue(*p) != GetBValue(*p))
					{
						bBW = false;
						break;
					}
				}
			}
			if (!bBW)
				break;
		}
		bool bHasAlpha = false;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			SPNGFrame const& sF = aPNGFrames.m_p[i];

			ULONG nFOffX = i ? sF.nX0 : 0;
			ULONG nFOffY = i ? sF.nY0 : 0;
			ULONG nFSizeX = i ? sF.nX1-sF.nX0 : sF.nSizeX;
			ULONG nFSizeY = i ? sF.nY1-sF.nY0 : sF.nSizeY;
			for (ULONG j = 0; j < nFSizeY; ++j)
			{
				DWORD const* p = reinterpret_cast<DWORD const*>(sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2));
				for (DWORD const* const e = p+nFSizeX; p != e; ++p)
				{
					if ((0xff000000&*p) != 0xff000000)
					{
						bHasAlpha = true;
						break;
					}
				}
			}
			if (bHasAlpha)
				break;
		}

		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, PNGErrorFnc, PNGWarningFnc);
		if (png_ptr == NULL)
			return E_FAIL;

		if (optimizing)
			png_set_cust_comp(png_ptr, zopfli_deflate);

		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			png_destroy_write_struct(&png_ptr, NULL);
			return E_FAIL;
		}

		png_bytep* apRows = reinterpret_cast<png_bytep*>(_alloca(sizeof(png_bytep*)*nSizeY));

		try
		{
			CPNGStreamWriter cSink(a_pDst);
			png_set_write_fn(png_ptr, &cSink, CPNGStreamWriter::PngRWFnc, CPNGStreamWriter::PngFlushFnc);

			if ((!bBW || bHasAlpha) && clrs.size() <= 256 || clrs.size() <= 16)
			{
				int bits = clrs.size() <= 2 ? 1 : (
						   clrs.size() <= 4 ? 2 : (
						   clrs.size() <= 16 ? 4 : 8));
				CAutoVectorPtr<png_color> pal(new png_color[clrs.size()]);
				for (std::map<DWORD, BYTE>::const_iterator i = clrs.begin(), e = clrs.end(); i != e; ++i)
				{
					png_color* p = pal.m_p+i->second;
					p->red = GetBValue(i->first);
					p->green = GetGValue(i->first);
					p->blue = GetRValue(i->first);
				}
				png_set_PLTE(png_ptr, info_ptr, pal, clrs.size());

				if (bHasAlpha)
				{
					CAutoVectorPtr<png_byte> alp(new png_byte[clrs.size()]);
					for (std::map<DWORD, BYTE>::const_iterator i = clrs.begin(), e = clrs.end(); i != e; ++i)
						alp[i->second] = ((i->first)>>24)&0xff;
					png_set_tRNS(png_ptr, info_ptr, alp, clrs.size(), NULL);
				}
				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, bits,
					PNG_COLOR_TYPE_PALETTE,
					interlacing, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				bBW = false; // palette is not technically grayscale
			}
			else if (bBW)
			{
				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, 8,
					bHasAlpha ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY,
					interlacing, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				clrs.clear();
			}
			else
			{
				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, 8,
					bHasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
					interlacing, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				clrs.clear();
			}

			png_set_acTL(png_ptr, info_ptr, nFrames, nLoopCount);

			if (extraAttr)
			{
				//if (cBuffer.fGamma >= 0.1f && cBuffer.fGamma <= 10.0f && fabsf(cBuffer.fGamma-2.2f) > 1e-3f)
				//{
				//	png_set_gAMA(png_ptr, info_ptr, 1.0/cBuffer.fGamma);
				//}
				if (tResolution.nNumeratorX && tResolution.nDenominatorX && tResolution.nNumeratorY && tResolution.nDenominatorY)
				{
					png_set_pHYs(png_ptr, info_ptr, tResolution.nNumeratorX*10000ULL/tResolution.nDenominatorX, tResolution.nNumeratorY*10000ULL/tResolution.nDenominatorY, 1);
				}
				//if (cDef.bA == 255 && !bHasAlpha)
				//{
				//	png_color_16 tBG;
				//	tBG.index = 0;
				//	tBG.red = cDef.bR;
				//	tBG.green = cDef.bG;
				//	tBG.blue = cDef.bB;
				//	tBG.gray = (ULONG(cDef.bR)+ULONG(cDef.bG)+ULONG(cDef.bB))/3;
				//	png_set_bKGD(png_ptr, info_ptr, &tBG);
				//}
			}

			png_write_info(png_ptr, info_ptr);

			if (!bBW)
			{
				if (!bHasAlpha)
					png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
				png_set_bgr(png_ptr);
			}
			if (!clrs.empty() && clrs.size() <= 16)
				png_set_packing(png_ptr); // ignored if not paletted image

			for (ULONG i = 0; i < nFrames; ++i)
			{
				SPNGFrame& sF = aPNGFrames.m_p[i];

				ULONG nNum = sF.fDelay*1000.0f+0.5f;
				ULONG nDen = 1000;
				while (nNum > 32768 && nDen > 1) { nNum >>= 1; nDen >>= 1; }

				ULONG nFOffX = i ? sF.nX0 : 0;
				ULONG nFOffY = i ? sF.nY0 : 0;
				ULONG nFSizeX = i ? sF.nX1-sF.nX0 : sF.nSizeX;
				ULONG nFSizeY = i ? sF.nY1-sF.nY0 : sF.nSizeY;

				if (!clrs.empty())
				{
					for (ULONG j = 0; j < nFSizeY; ++j)
					{
						BYTE* pD = sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2);
						DWORD const* p = reinterpret_cast<DWORD const*>(pD);
						for (DWORD const* const e = p+nFSizeX; p != e; ++p, ++pD)
							*pD = clrs[*p];
					}
				}
				else if (bBW)
				{
					if (bHasAlpha)
						for (ULONG j = 0; j < nFSizeY; ++j)
						{
							BYTE* pD = sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2);
							BYTE const* p = pD;
							for (BYTE const* const e = p+nFSizeX*4; p != e; p+=4, pD+=2)
							{
								pD[0] = p[0];
								pD[1] = p[3];
							}
						}
					else
						for (ULONG j = 0; j < nFSizeY; ++j)
						{
							BYTE* pD = sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2);
							BYTE const* p = pD;
							for (BYTE const* const e = p+nFSizeX*4; p != e; p+=4, ++pD)
								*pD = *p;
						}
				}

				for (ULONG j = 0; j < nFSizeY; ++j)
					apRows[j] = sF.pData.m_p+((sF.nSizeX*(j+nFOffY)+nFOffX)<<2);

				png_write_frame_head(png_ptr, info_ptr, apRows, nFSizeX, nFSizeY, nFOffX, nFOffY, min(0xffff, nNum), nDen, sF.bDispose, sF.bBlend);
				png_write_image(png_ptr, apRows);
				png_write_frame_tail(png_ptr, info_ptr);
			}
			png_write_end(png_ptr, info_ptr);
		}
		catch (...)
		{
			hRes = E_UNEXPECTED;
		}

		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
	catch (...)
	{
		hRes = a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}

	return hRes;
}

