
#pragma once

#include <RWImagingDocumentUtils.h>


template<typename TOperator>
class ATL_NO_VTABLE CPixelLevelTask :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IThreadedTask
{
public:
	CPixelLevelTask() : pSource(NULL) {}
	~CPixelLevelTask() { delete pSource; }
	void Init(IDocumentImage* a_pImg, TRasterImageRect a_tArea, TPixelChannel* a_pDst, TOperator& a_tOp)
	{
		pSource = new SLockedImageBuffer(a_pImg);
		a_pImg->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		tArea = a_tArea;
		pDst = a_pDst;
		pOp = &a_tOp;
	}

BEGIN_COM_MAP(CPixelLevelTask<TOperator>)
	COM_INTERFACE_ENTRY(IThreadedTask)
END_COM_MAP()


	STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
	{
		ULONG nY = tArea.tBR.nY-tArea.tTL.nY;
		if (a_nIndex > nY)
			return S_FALSE;
		if (a_nTotal > nY)
			a_nTotal = nY;
		size_t nStride = tArea.tBR.nX-tArea.tTL.nX;
		TRasterImageRect tSub = tArea;
		tSub.tTL.nY = tArea.tTL.nY+nY*a_nIndex/a_nTotal;
		tSub.tBR.nY = tArea.tTL.nY+nY*(a_nIndex+1)/a_nTotal;
		TPixelChannel* pD = pDst + nStride*(tSub.tTL.nY-tArea.tTL.nY);
		CComObjectStackEx<CVisitor> cVis;
		cVis.Init(tSub.tTL, pD, nStride, pOp);

		TImageSize tSubSize = {tSub.tBR.nX-tSub.tTL.nX, tSub.tBR.nY-tSub.tTL.nY};
		return RGBAInspectImpl(pSource->tContentOrigin, pSource->tContentSize, pSource->Content(), pSource->tAllocSize.nX, tDefault, &tSub.tTL, &tSubSize, &cVis, NULL);
		//TImageTile tTile;
		//tTile.nChannelIDs = EICIRGBA;
		//tTile.tOrigin = tSub.tTL;
		//tTile.tSize = tSubSize;
		//tTile.tStride.nX = 1;
		//tTile.tStride.nY = pSource->tAllocSize.nX;
		//tTile.nPixels = tSubSize.nX*tSubSize.nY;
		//tTile.pData = pSource->pData + pSource->tAllocSize.nX*(tSub.tTL.nY-pSource->tAllocOrigin.nY) + (tSub.tTL.nX-pSource->tAllocOrigin.nX);
		//return cVis.Visit(1, &tTile, NULL);
		//return pImg->Inspect(EICIRGBA, &tSub.tTL, &tSubSize, &cVis, NULL, EIRIAccurate);
	}

private:
	class ATL_NO_VTABLE CVisitor :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IImageVisitor
	{
	public:
		void Init(TImagePoint a_tOrig, TPixelChannel* a_pDst, size_t a_nStride, TOperator* a_pOp)
		{
			tOrig = a_tOrig;
			pDst = a_pDst;
			nStride = a_nStride;
			pOp = a_pOp;
		}

	BEGIN_COM_MAP(CVisitor)
		COM_INTERFACE_ENTRY(IImageVisitor)
	END_COM_MAP()

		// IImageVisitor methods
	public:
		STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl*)
		{
			for (TImageTile const* const pE = a_aTiles+a_nTiles; a_aTiles != pE; ++a_aTiles)
			{
				TPixelChannel* pD = pDst + (a_aTiles->tOrigin.nX-tOrig.nX) + nStride*(a_aTiles->tOrigin.nY-tOrig.nY);
				TPixelChannel const* pS = a_aTiles->pData;
				for (ULONG nY = 0; nY < a_aTiles->tSize.nY; ++nY, pD += nStride, pS += a_aTiles->tStride.nY)
					pOp->Process(pD, pS, a_aTiles->tStride.nX, a_aTiles->tSize.nX);
			}
			return S_OK;
		}

	private:
		TImagePoint tOrig;
		TPixelChannel* pDst;
		size_t nStride;
		TOperator* pOp;
	};

private:
	SLockedImageBuffer* pSource;
	TPixelChannel tDefault;
	TRasterImageRect tArea;
	TPixelChannel* pDst;
	TOperator* pOp;
};

#ifndef WIN64
inline LONG64 InterlockedAdd64(LONG64 volatile* Addend, LONG64 Value)
{
	EnterCriticalSection(&_pModule->get_m_csObjMap());
	LONG64 l = *Addend += Value;
	LeaveCriticalSection(&_pModule->get_m_csObjMap());
	return l;
}
#endif
