
#pragma once

#include <list>
#include <algorithm>

#include "RasterImageOperationStep.h"


class CComAutoCriticalSectionLock
{
public:
	CComAutoCriticalSectionLock(CComAutoCriticalSection& p) : m_p(p)
	{
		m_p.Lock();
	}

	~CComAutoCriticalSectionLock()
	{
		m_p.Unlock();
	}

private:
	CComAutoCriticalSection& m_p;
};

void CDocumentLayeredImage::AgeCache()
{
	{
		CComAutoCriticalSectionLock lock(m_cMergedCS);

		CMergedCache::iterator i = m_cMergedCache.begin();
		while (i != m_cMergedCache.end())
		{
			++i->second->nAge;
			if (i->second->nAge >= 9)
			{
				CMergedCache::iterator ii = i;
				++i;
				m_cMergedCache.erase(ii);
			}
			else
			{
				++i;
			}
		}
	}

	{
		CComAutoCriticalSectionLock lock(m_cLayerCS);

		CLayerCache::iterator i = m_cLayerCache.begin();
		while (i != m_cLayerCache.end())
		{
			++i->second->nAge;
			if (i->second->nAge >= 9)
			{
				CLayerCache::iterator ii = i;
				++i;
				m_cLayerCache.erase(ii);
			}
			else
			{
				++i;
			}
		}
	}
}

void CDocumentLayeredImage::DeleteMergedCacheForLayer(ULONG a_nLayerID)
{
	CComAutoCriticalSectionLock lock(m_cMergedCS);

	CMergedCache::iterator i = m_cMergedCache.begin();
	while (i != m_cMergedCache.end())
	{
		if (std::find(i->first.begin(), i->first.end(), a_nLayerID) == i->first.end())
		{
			++i;
			continue;
		}
		CMergedCache::iterator ii = i;
		++i;
		m_cMergedCache.erase(ii);
	}
}

void CDocumentLayeredImage::DeleteCacheForLayer(ULONG a_nLayerID)
{
	DeleteMergedCacheForLayer(a_nLayerID);

	CComAutoCriticalSectionLock lock(m_cLayerCS);

	CLayerCache::iterator i = m_cLayerCache.begin();
	while (i != m_cLayerCache.end())
	{
		if (i->first != a_nLayerID)
		{
			++i;
			continue;
		}
		CLayerCache::iterator ii = i;
		++i;
		m_cLayerCache.erase(ii);
	}
}

void CDocumentLayeredImage::DeleteCacheForMerges()
{
	CComAutoCriticalSectionLock lock(m_cMergedCS);

	m_cMergedCache.clear();
}

class ATL_NO_VTABLE CLayerOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
BEGIN_COM_MAP(CLayerOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
		if (i == m_cStates.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->QueryInterface(a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (a_pState)
			m_cStates[a_bstrCategoryName] = a_pState;
		else
			m_cStates.erase(a_bstrCategoryName);
		return S_OK;
	}
	STDMETHOD(IsCancelled)()
	{
		return S_FALSE;
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		if (a_pItemIndex) *a_pItemIndex = 0;
		if (a_pItemsRemaining) *a_pItemsRemaining = 0;
		if (a_pStepIndex) *a_pStepIndex = 0;
		if (a_pStepsRemaining) *a_pStepsRemaining = 0;
		return S_OK;
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		return S_FALSE;
	}

private:
	typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

private:
	CStates m_cStates;
};

CDocumentLayeredImage::SCacheEntryPtr CDocumentLayeredImage::QueryLayerCache(ULONG a_nLayerID)
{
	CLayers::const_iterator iL;
	for (iL = m_cLayers.begin(); iL != m_cLayers.end(); ++iL)
	{
		if (iL->nUID == a_nLayerID)
			break;
	}
	if (iL == m_cLayers.end())
		return SCacheEntryPtr(); // should not happen

	SCacheEntryPtr pItem(FindOrCreateItemInCache(m_cLayerCache, a_nLayerID, m_cLayerCS));

	// if it is valid...
	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
		return pItem;

	CComAutoCriticalSectionLock lock(pItem->tCS);

	// testing again, item could have been updated by another thread
	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
		return pItem;

	// is the item partially valid?
	TRasterImageRect tInvalid;
	TRasterImageRect* pInvalid = NULL;
	if (pItem->pImage)
	{
		tInvalid = pItem->tInvalid;
		pInvalid = &tInvalid;
	}

	// check if there are effects and eventually adjust the invalid rectangle
	bool bHasEffect = false;
	for (CLayerEffects::const_iterator i = iL->pEffects->begin(); i != iL->pEffects->end(); ++i)
	{
		if (!i->bEnabled || IsEqualGUID(i->tOpID, GUID_NULL) || IsEqualGUID(i->tOpID, __uuidof(DocumentOperationNULL)) || i->pOp == NULL)
			continue;
		bHasEffect = true;
		if (pInvalid == NULL)
			break;
		CComQIPtr<IRasterImageFilter> pRIF(i->pOp);
		if (pRIF)
			pRIF->AdjustDirtyRect(i->pOpCfg, NULL, pInvalid);
	}

	CComPtr<IDocument> pCurRaw;
	CComBSTR bstrLID;
	iL->GetLayerID(this, bstrLID);
	M_Base()->DataBlockDoc(bstrLID, &pCurRaw);

	// no effects -> just place reference into the cache and read basic values
	if (!bHasEffect)
	{
		pItem->Init(iL->eBlendingMode, pCurRaw, iL->pDocImg);
		return pItem;
	}

	// read basic info about the image
	CComPtr<IDocumentImage> pCur(iL->pDocImg);

	TRasterImageRect tBounds = INVALIDRECT;
	TImageSize t = {0, 0};
	pCur->CanvasGet(NULL, NULL, &tBounds.tTL, &t, NULL);
	tBounds.tBR.nX = tBounds.tTL.nX + t.nX;
	tBounds.tBR.nY = tBounds.tTL.nY + t.nY;

	// decide whether to recompute the whole image or just the invalid rectangle
	bool bRecomputeAll = pInvalid == NULL ||
		pInvalid->tTL.nX < tBounds.tTL.nX || pInvalid->tTL.nY < tBounds.tTL.nY ||
		pInvalid->tBR.nX > tBounds.tBR.nX || pInvalid->tBR.nY < tBounds.tBR.nY;
	if (!bRecomputeAll)
	{
		ULONGLONG nInvalidPixels = ULONGLONG(pInvalid->tBR.nX-pInvalid->tTL.nX)*(pInvalid->tBR.nY-pInvalid->tTL.nY);
		ULONGLONG nBoundsPixels = ULONGLONG(tBounds.tBR.nX-tBounds.tTL.nX)*(tBounds.tBR.nY-tBounds.tTL.nY);
		if (nInvalidPixels > nBoundsPixels*2/3)
			bRecomputeAll = true; // dirty region quite large, probably not worth it to recompute just the dirty region since there is an extra step required to combine the old and new buffers
	}

	if (!bRecomputeAll)
	{
		// TODO: support subregion updates
		// return pItem;
		pItem->pDoc = NULL;
		pItem->pImage = NULL;
	}

	// recompute the buffer

	CComObjectStackEx<CLayerOperationContext> cLOC;
	for (CLayerEffects::const_iterator i = iL->pEffects->begin(); i != iL->pEffects->end(); ++i)
	{
		if (!i->bEnabled || IsEqualGUID(i->tOpID, GUID_NULL) || IsEqualGUID(i->tOpID, __uuidof(DocumentOperationNULL)) || i->pOp == NULL)
			continue;
		CComQIPtr<IRasterImageFilter> pRIF(i->pOp);
		if (pRIF && pCurRaw)
		{
			// try processing via non-destructive IRasterImageFilter::Process method
			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			HRESULT hRes = pRIF->Process(pCurRaw, i->pOpCfg, pBase, NULL);
			if (SUCCEEDED(hRes))
			{
				pCurRaw = NULL;
				pBase->QueryInterface(&pCurRaw);
				pCur = NULL;
				pCurRaw->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pCur));
				continue;
			}
			else if (hRes != E_NOTIMPL)
				continue; // skip step if it failed ...or should we cancel the rest and break?
		}
		CComObjectStackEx<CRasterImageOperationStep> cStepSrc;
		cStepSrc.Init(pCur, NULL, NULL);
		i->pOp->Activate(M_OpMgr(), &cStepSrc, i->pOpCfg, &cLOC, NULL, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));
		cStepSrc.SwapRes(pCurRaw, pCur);
	}

	pItem->Init(iL->eBlendingMode, pCurRaw, pCur);
	return pItem;
}

CDocumentLayeredImage::SCacheEntryPtr CDocumentLayeredImage::QueryMergedCache(std::vector<ULONG> const& a_aLayerIDs, std::vector<SCacheEntryPtr>* a_pChunks)
{
	if (a_aLayerIDs.size() == 1)
		return QueryLayerCache(a_aLayerIDs[0]);

	if (a_pChunks && a_pChunks->size() == 1)
		return (*a_pChunks)[0]; // simplifies client code a bit

	SCacheEntryPtr pItem(FindOrCreateItemInCache(m_cMergedCache, a_aLayerIDs, m_cMergedCS));

	// if it is valid...
	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
		return pItem;

	CComAutoCriticalSectionLock lock(pItem->tCS);

	// testing again, item could have been updated by another thread
	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
		return pItem;

	// need to create the valid cache item

	std::vector<SCacheEntryPtr> aLayers;
	if (a_pChunks == NULL)
	{
		// no chunks given, combine merged item from individual layers (may not be optimal, there may be usable block in merged cache anyway, but it will be hard to find them)
		aLayers.resize(a_aLayerIDs.size());
		for (size_t i = 0; i < a_aLayerIDs.size(); ++i)
		{
			aLayers[i] = QueryLayerCache(a_aLayerIDs[i]);
			if (aLayers[i] == NULL)
				return SCacheEntryPtr();
		}
		a_pChunks = &aLayers;
	}

	// merge layers
	std::vector<SCacheEntryPtr>& chunks = *a_pChunks; // just for [] convenience
	ATLASSERT(chunks.size() > 1);
	// compute bounding rectangle

	CComPtr<IDocument> pDoc;
	CComPtr<IDocumentImage> pDst;
	TRasterImageRect tBounds;
	TPixelChannel tDefault;

	if (pItem->pImage)
	{
		tBounds = pItem->tInvalid;
		std::swap(pDst.p, pItem->pImage.p);
		std::swap(pDoc.p, pItem->pDoc.p);
	}
	else
	{
		tBounds = chunks[0]->tBounds;
		for (std::vector<SCacheEntryPtr>::const_iterator i = chunks.begin()+1; i != chunks.end(); ++i)
		{
			tBounds.tTL.nX = min(tBounds.tTL.nX, (*i)->tBounds.tTL.nX);
			tBounds.tTL.nY = min(tBounds.tTL.nY, (*i)->tBounds.tTL.nY);
			tBounds.tBR.nX = max(tBounds.tBR.nX, (*i)->tBounds.tBR.nX);
			tBounds.tBR.nY = max(tBounds.tBR.nY, (*i)->tBounds.tBR.nY);
		}
		TImageSize tSize = {{tBounds.tBR.nX-tBounds.tTL.nX}, {tBounds.tBR.nY-tBounds.tTL.nY}};
		ULONG nPixels = tSize.nX*tSize.nY;

		RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
		CComObject<CDocumentRasterImage>* pImg = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&pImg);
		CComPtr<IDocumentData> pData(pImg);
		CAutoVectorPtr<TPixelChannel> pPixels(nPixels ? new TPixelChannel[nPixels] : NULL);
		pImg->Init(m_tSize, NULL, NULL, tBounds.tTL, tSize, pPixels, 2.2f, NULL);
		CComQIPtr<IDocumentBase>(pDoc)->DataBlockSet(NULL, pData);
		//CComPtr<IDocumentImage> pFinal;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDst));
		//pItem->pImage = pFinal;
	}

	TPixelChannel const* pData = NULL;
	TImagePoint tAllocOrigin = {0, 0};
	TImageSize tAllocSize = {0, 0};
	pDst->BufferLock(EICIRGBA, &tAllocOrigin, &tAllocSize, NULL, NULL, &pData, NULL, EIRIAccurate);
	try
	{
		TImageStride tStride = {1, tAllocSize.nX};
		TPixelChannel* pPixels = const_cast<TPixelChannel*>(pData) + tStride.nY*(tBounds.tTL.nY-tAllocOrigin.nY) + (tBounds.tTL.nX-tAllocOrigin.nX);
		TImageSize tSize = {{tBounds.tBR.nX-tBounds.tTL.nX}, {tBounds.tBR.nY-tBounds.tTL.nY}};

		// it may be possible to save some copying by combining the two first layers and writing into another buffer
		// instead of copying the first and then blending the others

		if (tSize.nX*tSize.nY)
		{
			CComAutoCriticalSectionLock lock(chunks[0]->tCS);
			chunks[0]->pImage->TileGet(EICIRGBA, &tBounds.tTL, &tSize, &tStride, tStride.nY*tSize.nY, pPixels, NULL, EIRIAccurate);
		}
		tDefault = chunks[0]->tDefault;

		for (std::vector<SCacheEntryPtr>::const_iterator i = chunks.begin()+1; i != chunks.end(); ++i)
		{
			CComAutoCriticalSectionLock lock((*i)->tCS);
			MergeRectangle(tBounds, tStride.nY, pPixels, tDefault, (*i)->eBlend, (*i)->pImage);
		}
	}
	catch (...)
	{
	}
	pDst->BufferUnlock(EICIRGBA, pData);

	CDocumentRasterImage* pRI = static_cast<CDocumentRasterImage*>(pDst.p);
	pRI->InitFinish(tDefault);

	pItem->Init(chunks[0]->eBlend, pDoc, pDst);
	return pItem;
}

	//EBEAlphaBlend	= 0,
	//EBEModulate	= ( EBEAlphaBlend + 1 ) ,
	//EBEScreen	= ( EBEModulate + 1 ) ,
	//EBEAdd	= ( EBEScreen + 1 ) ,
	//EBESubtract	= ( EBEAdd + 1 ) ,
	//EBEAverage	= ( EBESubtract + 1 ) ,
	//EBEDifference	= ( EBEAverage + 1 ) ,
	//EBEMinimum	= ( EBEDifference + 1 ) ,
	//EBEMaximum	= ( EBEMinimum + 1 ) ,
	//EBEOverlay	= ( EBEMaximum + 1 ) ,
	//EBEHLSReplace	= ( EBEOverlay + 1 ) ,
	//EBEMultiplyInvAlpha	= ( EBEHLSReplace + 1 ) ,
	//EBEGlass	= ( EBEMultiplyInvAlpha + 1 ) ,
	//ELBHLSReplaceHue	= ( EBEHLSReplace + 0x100 ) ,
	//ELBHLSReplaceSaturation	= ( EBEHLSReplace + 0x200 ) ,
	//ELBHLSReplaceLuminance	= ( EBEHLSReplace + 0x400 ) ,
	//ELBHLSReplaceColor	= ( EBEHLSReplace + 0x300 ) ,


bool CanCombine(ELayerBlend eBlendingMode)
{
	return
		eBlendingMode == EBEAlphaBlend || 
		eBlendingMode == EBEModulate ||
		eBlendingMode == EBEScreen ||
		eBlendingMode == EBEAdd ||
		eBlendingMode == EBEMinimum ||
		eBlendingMode == EBEMaximum ||
		eBlendingMode == EBEMultiplyInvAlpha;
}

bool CanCommute(ELayerBlend eBlendingMode)
{
	return
		eBlendingMode == EBEModulate ||
		eBlendingMode == EBEScreen ||
		eBlendingMode == EBEAdd ||
		eBlendingMode == EBEMultiplyInvAlpha;
}

void EndBlock(CDocumentLayeredImage::CLayers::const_iterator first, CDocumentLayeredImage::CLayers::const_iterator end, std::list<std::vector<ULONG> >& chunks)
{
	std::vector<ULONG> t;
	ELayerBlend eLB = first->eBlendingMode;
	while (first != end)
	{
		if (first->bVisible)
			t.push_back(first->nUID);
		++first;
	}
	if (CanCommute(eLB))
		std::sort(t.begin(), t.end());
	chunks.push_back(t);
}

void CDocumentLayeredImage::ToChunks(CDocumentLayeredImage::CLayers::const_iterator begin, CDocumentLayeredImage::CLayers::const_iterator end, std::vector<ULONG>& ids, std::vector<CDocumentLayeredImage::SCacheEntryPtr>& dst)
{
	std::list<std::vector<ULONG> > chunks;
	CDocumentLayeredImage::CLayers::const_iterator first = end;
	for (CDocumentLayeredImage::CLayers::const_iterator i = begin; i != end; ++i)
	{
		if (!i->bVisible)
			continue;
		ids.push_back(i->nUID);
		if (first == end)
		{
			first = i;
			continue;
		}
		if (i->eBlendingMode == first->eBlendingMode && CanCombine(first->eBlendingMode))
		{
			continue;
		}
		EndBlock(first, i, chunks);
		first = i;
	}
	if (first != end)
		EndBlock(first, end, chunks);

	for (std::list<std::vector<ULONG> >::const_iterator i = chunks.begin(); i != chunks.end(); ++i)
	{
		// TODO: check if it makes sense to combine the layers and eventually don't combine
		// if rectangles do not overlap
		dst.push_back(QueryMergedCache(*i, NULL));
	}
}

CDocumentLayeredImage::SCacheEntryPtr CDocumentLayeredImage::QueryMergedCache()
{
	std::vector<SCacheEntryPtr> blocks;
	std::vector<ULONG> ids;
	ToChunks(m_cLayers.begin(), m_cLayers.end(), ids, blocks);
	if (ids.empty())
		return SCacheEntryPtr();
	return QueryMergedCache(ids, &blocks);
}

#include "LayerBlending.h"

void MergeRectangleInner(TImagePoint tP0, TImagePoint tP1, TImagePoint tDstOffset, TImageSize tDstSize, TImagePoint tBufferOffset, TImageSize tBufferSize, TImagePoint tBufferEnd, ELayerBlend a_eBlend, TPixelChannel* pDst, TPixelChannel const* pSrc, ULONG const nBufDiff, TPixelChannel const* pLaySrc, ULONG tSrcSizeX, TPixelChannel tSrcDefault, TPixelChannel tOldDefault, TPixelChannel tBufferDefault, CGammaTables const& m_cGamma)
{
	for (LONG y = tDstOffset.nY; y < LONG(tDstOffset.nY+tDstSize.nY); ++y)
	{
		if (y >= tBufferOffset.nY && y < tBufferOffset.nY+LONG(tBufferSize.nY))
		{
			if (y >= tP0.nY && y < tP1.nY)
			{
				LONG x = tDstOffset.nX;
				// old and new content
				if (tBufferOffset.nX < tP0.nX)
				{
					ULONG n = tBufferSize.nX < ULONG(tP0.nX-tBufferOffset.nX) ? tBufferSize.nX : tP0.nX-tBufferOffset.nX;
					MergeLayerLine(a_eBlend, n, pDst, pSrc, tSrcDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					x += n;
					pDst += n;
					pSrc += n;
				}
				else if (tBufferOffset.nX > tP0.nX)
				{
					ULONG n = tP1.nX-tP0.nX < tBufferOffset.nX-tP0.nX ? tP1.nX-tP0.nX : tBufferOffset.nX-tP0.nX;
					MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					x += n;
					pDst += n;
					pLaySrc += n;
				}
				if (tBufferOffset.nX >= tP1.nX || tBufferEnd.nX <= tP0.nX)
				{
					ULONG n = tBufferOffset.nX >= tP1.nX ? tBufferOffset.nX-tP1.nX : tP0.nX-tBufferEnd.nX;
					std::fill_n(pDst, n, tBufferDefault);
					x += n;
					pDst += n;
				}
				else
				{
					ULONG n = (tBufferEnd.nX < tP1.nX ? tBufferEnd.nX : tP1.nX)-(tBufferOffset.nX > tP0.nX ? tBufferOffset.nX : tP0.nX);
					MergeLayerLine(a_eBlend, n, pDst, pSrc, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					x += n;
					pDst += n;
					pSrc += n;
					pLaySrc += n;
				}
				if (tBufferEnd.nX > tP1.nX)
				{
					ULONG n = tBufferEnd.nX-x;
					MergeLayerLine(a_eBlend, n, pDst, pSrc, tSrcDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					x += n;
					pDst += n;
					pSrc += n;
				}
				else if (tBufferEnd.nX < tP1.nX)
				{
					ULONG n = tP1.nX-x;
					MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					x += n;
					pDst += n;
					pLaySrc += n;
				}
				pLaySrc += tSrcSizeX-(tP1.nX-tP0.nX);
			}
			else
			{
				// just the old content
				if (tDstOffset.nX < tBufferOffset.nX)
				{
					std::fill_n(pDst, tBufferOffset.nX-tDstOffset.nX, tBufferDefault);
					pDst += tBufferOffset.nX-tDstOffset.nX;
				}
				MergeLayerLine(a_eBlend, tBufferSize.nX, pDst, pSrc, tSrcDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
				pSrc += tBufferSize.nX;
				pDst += tBufferSize.nX;
				if (tDstOffset.nX+LONG(tDstSize.nX) > tBufferOffset.nX+LONG(tBufferSize.nX))
				{
					std::fill_n(pDst, (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX), tBufferDefault);
					pDst += (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX);
				}
			}
		}
		else if (y >= tP0.nY && y < tP1.nY)
		{
			// just the new content
			if (tDstOffset.nX < tP0.nX)
			{
				std::fill_n(pDst, tP0.nX-tDstOffset.nX, tBufferDefault);
				pDst += tP0.nX-tDstOffset.nX;
			}
			MergeLayerLine(a_eBlend, tP1.nX-tP0.nX, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
			pLaySrc += tSrcSizeX;
			pDst += tP1.nX-tP0.nX;
			if (tDstOffset.nX+LONG(tDstSize.nX) > tP1.nX)
			{
				std::fill_n(pDst, tDstOffset.nX+tDstSize.nX-tP1.nX, tBufferDefault);
				pDst += tDstOffset.nX+tDstSize.nX-tP1.nX;
			}
		}
		else
		{
			// empty space
			std::fill_n(pDst, tDstSize.nX, tBufferDefault);
			pDst += tDstSize.nX;
		}
		pDst += nBufDiff;
		pSrc += nBufDiff;
	}
}

class ATL_NO_VTABLE CBlendTask :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IThreadedTask
{
public:
	void Init(TImagePoint a_tP0, TImagePoint a_tP1, TImagePoint a_tDstOffset, TImageSize a_tDstSize, TImagePoint a_tBufferOffset, TImageSize a_tBufferSize, TImagePoint a_tBufferEnd, ELayerBlend a_eBlend, TPixelChannel* a_pDst, TPixelChannel const* a_pSrc, ULONG const a_nBufDiff, TPixelChannel const* a_pLaySrc, ULONG a_tSrcSizeX, TPixelChannel a_tSrcDefault, TPixelChannel a_tOldDefault, TPixelChannel a_tBufferDefault, CGammaTables const* a_pGamma)
	{
		tP0 = a_tP0;
		tP1 = a_tP1;
		tDstOffset = a_tDstOffset;
		tDstSize = a_tDstSize;
		tBufferOffset = a_tBufferOffset;
		tBufferSize = a_tBufferSize;
		tBufferEnd = a_tBufferEnd;
		eBlend = a_eBlend;
		pDst = a_pDst;
		pSrc = a_pSrc;
		nBufDiff = a_nBufDiff;
		pLaySrc = a_pLaySrc;
		tSrcSizeX = a_tSrcSizeX;
		tSrcDefault = a_tSrcDefault;
		tOldDefault = a_tOldDefault;
		tBufferDefault = a_tBufferDefault;
		pGamma = a_pGamma;
	}

BEGIN_COM_MAP(CBlendTask)
	COM_INTERFACE_ENTRY(IThreadedTask)
END_COM_MAP()

	STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
	{
		if (a_nIndex > tDstSize.nY)
			return S_FALSE;
		if (a_nTotal > tDstSize.nY)
			a_nTotal = tDstSize.nY;
		ULONG nY0 = a_nIndex*tDstSize.nY/a_nTotal;
		ULONG nY1 = (a_nIndex+1)*tDstSize.nY/a_nTotal;
		TImageSize tSubDstSize = tDstSize;
		tSubDstSize.nY = nY1-nY0;
		TImagePoint tSubDstOffset = tDstOffset;
		tSubDstOffset.nY += nY0;
		TPixelChannel* pSubDst = pDst+(tDstSize.nX+nBufDiff)*nY0;
		TPixelChannel const* pSubSrc = pSrc;
		if (tSubDstOffset.nY > tBufferOffset.nY)
			pSubSrc += (tBufferSize.nX+nBufDiff)*(tSubDstOffset.nY-tBufferOffset.nY);
		TPixelChannel const* pSubLaySrc = pLaySrc;
		if (tSubDstOffset.nY > tP0.nY)
			pSubLaySrc += tSrcSizeX*(tSubDstOffset.nY-tP0.nY);
		MergeRectangleInner(tP0, tP1, tSubDstOffset, tSubDstSize, tBufferOffset, tBufferSize, tBufferEnd, eBlend, pSubDst, pSubSrc, nBufDiff, pSubLaySrc, tSrcSizeX, tSrcDefault, tOldDefault, tBufferDefault, *pGamma);
		return S_OK;
	}

private:
	TImagePoint tP0;
	TImagePoint tP1;
	TImagePoint tDstOffset;
	TImageSize tDstSize;
	TImagePoint tBufferOffset;
	TImageSize tBufferSize;
	TImagePoint tBufferEnd;
	ELayerBlend eBlend;
	TPixelChannel* pDst;
	TPixelChannel const* pSrc;
	ULONG nBufDiff;
	TPixelChannel const* pLaySrc;
	ULONG tSrcSizeX;
	TPixelChannel tSrcDefault;
	TPixelChannel tOldDefault;
	TPixelChannel tBufferDefault;
	CGammaTables const* pGamma;
};

void CDocumentLayeredImage::MergeRectangle(TRasterImageRect a_tBounds, ULONG a_nStride, TPixelChannel* a_pPixels, TPixelChannel& a_tDefault, ELayerBlend a_eBlend, IDocumentImage* a_pImage)
{
	ULONG const nBufDiff = 0;//a_nBufferStride == 0 ? 0 : a_nBufferStride-tBufferSize.nX;

	try
	{
		// update default color
		TPixelChannel tOverlayDefault;
		a_pImage->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tOverlayDefault));
		TPixelChannel const tOldDefault = a_tDefault;
		MergeLayerLine(a_eBlend, 1, &a_tDefault, &tOldDefault, &tOverlayDefault, m_cGamma);

		TImagePoint tBufferOffset = a_tBounds.tTL;
		TImagePoint tBufferEnd = a_tBounds.tBR;//{tBufferOffset.nX+tBufferSize.nX, tBufferOffset.nY+tBufferSize.nY};
		TImageSize tBufferSize = {a_tBounds.tBR.nX-a_tBounds.tTL.nX, a_tBounds.tBR.nY-a_tBounds.tTL.nY};

		SLockedImageBuffer const cOverlay(a_pImage);
		TImagePoint tP0 = cOverlay.tContentOrigin;
		TImagePoint tP1 = {cOverlay.tContentOrigin.nX+cOverlay.tContentSize.nX, cOverlay.tContentOrigin.nY+cOverlay.tContentSize.nY};

		if (tP0.nX < a_tBounds.tTL.nX) tP0.nX = a_tBounds.tTL.nX;
		if (tP0.nY < a_tBounds.tTL.nY) tP0.nY = a_tBounds.tTL.nY;
		if (tP1.nX > a_tBounds.tBR.nX) tP1.nX = a_tBounds.tBR.nX;
		if (tP1.nY > a_tBounds.tBR.nY) tP1.nY = a_tBounds.tBR.nY;
		TPixelChannel const* pLaySrc = cOverlay.pData + (tP0.nY-cOverlay.tAllocOrigin.nY)*cOverlay.tAllocSize.nX + (tP0.nX-cOverlay.tAllocOrigin.nX);
		bool const bLayerEmpty = tP0.nX >= tP1.nX || tP0.nY >= tP1.nY;
		if (bLayerEmpty && tOverlayDefault.bA == 0)
			return; // layer does not influence the outcome
		//if (bLayerEmpty && tBufferSize.nX*tBufferSize.nY == 0)
		//	return;

		TPixelChannel* pDst = a_pPixels;
		TPixelChannel const* pSrc = a_pPixels;
		TImagePoint tDstOffset = a_tBounds.tTL;
		TImageSize tDstSize = {a_tBounds.tBR.nX-a_tBounds.tTL.nX, a_tBounds.tBR.nY-a_tBounds.tTL.nY};

		//CAutoVectorPtr<TPixelChannel> pBuffer2;
		//TPixelChannel* pDst = pBuffer1;
		//TPixelChannel const* pSrc = pBuffer1;
		//TImagePoint tDstOffset = tBufferOffset;
		//TImageSize tDstSize = tBufferSize;
		//if (!bLayerEmpty)
		//{
		//	if (tBufferSize.nX*tBufferSize.nY == 0)
		//	{
		//		ATLASSERT(a_nBufferStride == 0);
		//		tDstOffset = tP0;
		//		tDstSize.nX = tP1.nX-tP0.nX;
		//		tDstSize.nY = tP1.nY-tP0.nY;
		//		pBuffer2.Attach(new TPixelChannel[tDstSize.nX*tDstSize.nY]);
		//		pDst = pBuffer2;
		//	}
		//	else if (tP0.nX < tBufferOffset.nX || tP0.nY < tBufferOffset.nY ||
		//		tP1.nX > tBufferOffset.nX+LONG(tBufferSize.nX) ||
		//		tP1.nY > tBufferOffset.nY+LONG(tBufferSize.nY))
		//	{
		//		tDstOffset.nX = tP0.nX < tBufferOffset.nX ? tP0.nX : tBufferOffset.nX;
		//		tDstOffset.nY = tP0.nY < tBufferOffset.nY ? tP0.nY : tBufferOffset.nY;
		//		tDstSize.nX = tP1.nX > tBufferOffset.nX+LONG(tBufferSize.nX) ? tP1.nX-tDstOffset.nX : tBufferOffset.nX+LONG(tBufferSize.nX)-tDstOffset.nX;
		//		tDstSize.nY = tP1.nY > tBufferOffset.nY+LONG(tBufferSize.nY) ? tP1.nY-tDstOffset.nY : tBufferOffset.nY+LONG(tBufferSize.nY)-tDstOffset.nY;
		//		pBuffer2.Attach(new TPixelChannel[tDstSize.nX*tDstSize.nY]);
		//		pDst = pBuffer2;
		//	}
		//}
		TImagePoint tDstEnd = {tDstOffset.nX+tDstSize.nX, tDstOffset.nY+tDstSize.nY};

		if (tDstSize.nX*tDstSize.nY >= 256*256 && tDstSize.nY >= 32 && m_pThPool)
		{
			CComObjectStackEx<CBlendTask> cBlendTask;
			cBlendTask.Init(tP0, tP1, tDstOffset, tDstSize, tBufferOffset, tBufferSize, tBufferEnd, a_eBlend, pDst, pSrc, nBufDiff, pLaySrc, cOverlay.tAllocSize.nX, tOverlayDefault, tOldDefault, a_tDefault, &m_cGamma);
			m_pThPool->Execute(0, &cBlendTask);
		}
		else
		{
			MergeRectangleInner(tP0, tP1, tDstOffset, tDstSize, tBufferOffset, tBufferSize, tBufferEnd, a_eBlend, pDst, pSrc, nBufDiff, pLaySrc, cOverlay.tAllocSize.nX, tOverlayDefault, tOldDefault, a_tDefault, m_cGamma);
		}
		//for (LONG y = tDstOffset.nY; y < LONG(tDstOffset.nY+tDstSize.nY); ++y)
		//{
		//	if (y >= tBufferOffset.nY && y < tBufferOffset.nY+LONG(tBufferSize.nY))
		//	{
		//		if (y >= tP0.nY && y < tP1.nY)
		//		{
		//			LONG x = tDstOffset.nX;
		//			// old and new content
		//			if (tBufferOffset.nX < tP0.nX)
		//			{
		//				ULONG n = tBufferSize.nX < ULONG(tP0.nX-tBufferOffset.nX) ? tBufferSize.nX : tP0.nX-tBufferOffset.nX;
		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//				x += n;
		//				pDst += n;
		//				pSrc += n;
		//			}
		//			else if (tBufferOffset.nX > tP0.nX)
		//			{
		//				ULONG n = tP1.nX-tP0.nX < tBufferOffset.nX-tP0.nX ? tP1.nX-tP0.nX : tBufferOffset.nX-tP0.nX;
		//				MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//				x += n;
		//				pDst += n;
		//				pLaySrc += n;
		//			}
		//			if (tBufferOffset.nX >= tP1.nX || tBufferEnd.nX <= tP0.nX)
		//			{
		//				ULONG n = tBufferOffset.nX >= tP1.nX ? tBufferOffset.nX-tP1.nX : tP0.nX-tBufferEnd.nX;
		//				std::fill_n(pDst, n, tBufferDefault);
		//				x += n;
		//				pDst += n;
		//			}
		//			else
		//			{
		//				ULONG n = (tBufferEnd.nX < tP1.nX ? tBufferEnd.nX : tP1.nX)-(tBufferOffset.nX > tP0.nX ? tBufferOffset.nX : tP0.nX);
		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//				x += n;
		//				pDst += n;
		//				pSrc += n;
		//				pLaySrc += n;
		//			}
		//			if (tBufferEnd.nX > tP1.nX)
		//			{
		//				ULONG n = tBufferEnd.nX-x;
		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//				x += n;
		//				pDst += n;
		//				pSrc += n;
		//			}
		//			else if (tBufferEnd.nX < tP1.nX)
		//			{
		//				ULONG n = tP1.nX-x;
		//				MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//				x += n;
		//				pDst += n;
		//				pLaySrc += n;
		//			}
		//			pLaySrc += pCachedLayer->tSize.nX-(tP1.nX-tP0.nX);
		//		}
		//		else
		//		{
		//			// just the old content
		//			if (tDstOffset.nX < tBufferOffset.nX)
		//			{
		//				std::fill_n(pDst, tBufferOffset.nX-tDstOffset.nX, tBufferDefault);
		//				pDst += tBufferOffset.nX-tDstOffset.nX;
		//			}
		//			MergeLayerLine(a_eBlend, tBufferSize.nX, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//			pSrc += tBufferSize.nX;
		//			pDst += tBufferSize.nX;
		//			if (tDstOffset.nX+LONG(tDstSize.nX) > tBufferOffset.nX+LONG(tBufferSize.nX))
		//			{
		//				std::fill_n(pDst, (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX), tBufferDefault);
		//				pDst += (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX);
		//			}
		//		}
		//	}
		//	else if (y >= tP0.nY && y < tP1.nY)
		//	{
		//		// just the new content
		//		if (tDstOffset.nX < tP0.nX)
		//		{
		//			std::fill_n(pDst, tP0.nX-tDstOffset.nX, tBufferDefault);
		//			pDst += tP0.nX-tDstOffset.nX;
		//		}
		//		MergeLayerLine(a_eBlend, tP1.nX-tP0.nX, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//		pLaySrc += pCachedLayer->tSize.nX;
		//		pDst += tP1.nX-tP0.nX;
		//		if (tDstOffset.nX+LONG(tDstSize.nX) > tP1.nX)
		//		{
		//			std::fill_n(pDst, tDstOffset.nX+tDstSize.nX-tP1.nX, tBufferDefault);
		//			pDst += tDstOffset.nX+tDstSize.nX-tP1.nX;
		//		}
		//	}
		//	else
		//	{
		//		// empty space
		//		std::fill_n(pDst, tDstSize.nX, tBufferDefault);
		//		pDst += tDstSize.nX;
		//	}
		//	pDst += nBufDiff;
		//	pSrc += nBufDiff;
		//}

		//CAutoVectorPtr<TRasterImagePixel> pTmp;
		//if ((i->tData.eBlend&EBEOperationMask) == EBEGlass)
		//{
		//	pTmp.Allocate(pCachedLayer->tSize.nX*pCachedLayer->tSize.nY);
		//	BoxBlur(pCachedLayer->tSize.nX, pCachedLayer->tSize.nY, 17, 17, reinterpret_cast<TRasterImagePixel*>(pDst), pTmp);
		//	CComPtr<SCacheEntry> pRawLayer;
		//	pRawLayer.Attach(QueryCache(ECPRaw, i->nUID));
		//	ULONG const n = pCachedLayer->tSize.nX*pCachedLayer->tSize.nY;
		//	TRasterImagePixel *pD = reinterpret_cast<TRasterImagePixel*>(pDst);
		//	TPixelChannel const* pR = pRawLayer->pPixels;
		//	TRasterImagePixel const* pB = pTmp;
		//	for (TRasterImagePixel* pEnd = pD+n; pD < pEnd; ++pD, ++pR, ++pB)
		//	{
		//		if (pR->bA != 0 && pR->bA != 255)
		//			*pD = *pB;
		//	}
		//}

		//for (ULONG nY = 0; nY < pCachedLayer->tSize.nY; pDst+=nDeltaY, ++nY)
		//{
		//	MergeLayerLine(i->tData.eBlend, pCachedLayer->tSize.nX, reinterpret_cast<TRasterImagePixel*>(pDst), reinterpret_cast<TRasterImagePixel const*>(pDst), reinterpret_cast<TRasterImagePixel const*>(pSrc), m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
		//	pDst += pCachedLayer->tSize.nX<<2;
		//	pSrc += pCachedLayer->tSize.nX<<2;
		//}

		//if (pBuffer2.m_p)
		//{
		//	std::swap(pBuffer1, pBuffer2.m_p);
		//	tBufferSize = tDstSize;
		//	tBufferOffset = tDstOffset;
		//	tBufferEnd = tDstEnd;
		//}
	}
	catch (...)
	{
	}
}

