
#pragma once

#include <GammaCorrection.h>

// CFadeDocument

class ATL_NO_VTABLE CFadeDocument : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocument,
	public IDocumentRasterImage
{
public:
	CFadeDocument()
	{
	}
	void Init(IDocument* a_pDoc, IDocumentRasterImage* a_pRI, float a_fStrength)
	{
		m_pDoc = a_pDoc;
		m_pRI = a_pRI;
		m_nStrength = a_fStrength*1024;
		RWCoCreateInstance(m_pGamma, __uuidof(GammaTableCache));
	}


BEGIN_COM_MAP(CFadeDocument)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IDocumentRasterImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IBlockOperations)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)() { return m_pDoc->WriteLock(); }
	STDMETHOD(WriteUnlock)() { return m_pDoc->WriteUnlock(); }
	STDMETHOD(ReadLock)() { return m_pDoc->ReadLock(); }
	STDMETHOD(ReadUnlock)() { return m_pDoc->ReadUnlock(); }

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder) { return m_pDoc->BuilderID(a_pguidBuilder); }
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
	{
		if (IsEqualIID(a_iid, __uuidof(IDocumentRasterImage)) || IsEqualIID(a_iid, __uuidof(IDocumentEditableImage)) || IsEqualIID(a_iid, __uuidof(IDocumentImage)))
		{
			*a_ppFeatureInterface = static_cast<IDocumentRasterImage*>(this);
			AddRef();
			return S_OK;
		}
		return m_pDoc->QueryFeatureInterface(a_iid, a_ppFeatureInterface);
	}
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation) { return m_pDoc->LocationGet(a_ppLocation); }
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation) { return m_pDoc->LocationSet(a_pLocation); }
	STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig) { return E_NOTIMPL; }
	STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig) { return E_NOTIMPL; }
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects) { return m_pDoc->EncoderAspects(a_pEnumAspects);	}
	STDMETHOD(IsDirty)() { return m_pDoc->IsDirty(); }
	STDMETHOD(SetDirty)() { return m_pDoc->SetDirty(); }
	STDMETHOD(ClearDirty)() { return m_pDoc->ClearDirty(); }
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
	{
		try
		{
			// TODO: implement preview hint
			CComObject<CFadeDocument>* pNew = NULL;
			CComObject<CFadeDocument>::CreateInstance(&pNew);
			CComPtr<IDocument> pTmp = pNew;
			CFadeDocument* p = pNew;
			if (FAILED(m_pDoc->DocumentCopy(a_bstrPrefix, a_pBase, a_tPreviewEffectID, a_pPreviewEffect))) // <--
				return E_FAIL;
			CComQIPtr<IDocument> pDocCopy(a_pBase);
			CComPtr<IDocumentRasterImage> pRICopy;
			pDocCopy->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRICopy));
			p->Init(pDocCopy, pRICopy, m_nStrength/1024.0f);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(QuickInfo)(ULONG /*a_nInfoIndex*/, ILocalizedString** /*a_ppInfo*/)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ObserverIns)(IDocumentObserver* a_pObserver, TCookie a_tCookie) { return m_pDoc->ObserverIns(a_pObserver, a_tCookie); }
	STDMETHOD(ObserverDel)(IDocumentObserver* a_pObserver, TCookie a_tCookie) { return m_pDoc->ObserverDel(a_pObserver, a_tCookie); }

	// helpers
private:
	static void Mix(TPixelChannel& a_tP1, TPixelChannel const& a_tP2, int a_nCoverage)
	{
		int const nSrcR = a_tP1.bR*int(a_tP1.bA);
		int const nSrcG = a_tP1.bG*int(a_tP1.bA);
		int const nSrcB = a_tP1.bB*int(a_tP1.bA);
		int const nSrcA = a_tP1.bA;
		int const nDstR = a_tP2.bR*int(a_tP1.bA);
		int const nDstG = a_tP2.bG*int(a_tP1.bA);
		int const nDstB = a_tP2.bB*int(a_tP1.bA);
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

	static void Mix(TPixelChannel& a_tP1, TPixelChannel const& a_tP2, int a_nCoverage, CGammaTables const* a_pGamma)
	{
		int const nSrcR = a_pGamma->m_aGamma[a_tP1.bR];//*int(a_tP1.bA);
		int const nSrcG = a_pGamma->m_aGamma[a_tP1.bG];//*int(a_tP1.bA);
		int const nSrcB = a_pGamma->m_aGamma[a_tP1.bB];//*int(a_tP1.bA);
		int const nSrcA = a_tP1.bA;
		int const nDstR = a_pGamma->m_aGamma[a_tP2.bR];//*int(a_tP2.bA);
		int const nDstG = a_pGamma->m_aGamma[a_tP2.bG];//*int(a_tP2.bA);
		int const nDstB = a_pGamma->m_aGamma[a_tP2.bB];//*int(a_tP2.bA);
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
			int const nDmlR = nFinR;///nFinA;
			int const nDmlG = nFinG;///nFinA;
			int const nDmlB = nFinB;///nFinA;
			a_tP1.bR = nDmlR < 0 ? 0 : (nDmlR > 65535 ? 255 : a_pGamma->InvGamma(nDmlR));
			a_tP1.bG = nDmlG < 0 ? 0 : (nDmlG > 65535 ? 255 : a_pGamma->InvGamma(nDmlG));
			a_tP1.bB = nDmlB < 0 ? 0 : (nDmlB > 65535 ? 255 : a_pGamma->InvGamma(nDmlB));
			a_tP1.bA = nFinA > 255 ? 255 : nFinA;
		}
	}

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
	{ return m_pRI->CanvasGet(a_pCanvasSize, a_pResolution, a_pContentOrigin, a_pContentSize, a_pContentOpacity); }
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
	{ return m_pRI->ChannelsGet(a_pChannelIDs, a_pGamma, a_pChannelDefaults); }
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pRI->TileGet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData, a_pControl, a_eIntent); }
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pRI->Inspect(a_nChannelIDs, a_pOrigin, a_pSize, a_pVisitor, a_pControl, a_eIntent); }
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pRI->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent); }
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
	{ return m_pRI->BufferUnlock(a_nChannelID, a_pBuffer); }

	STDMETHOD(ObserverIns)(IImageObserver* a_pObserver, TCookie a_tCookie)
	{ return m_pRI->ObserverIns(a_pObserver, a_tCookie); }
	STDMETHOD(ObserverDel)(IImageObserver* a_pObserver, TCookie a_tCookie)
	{ return m_pRI->ObserverDel(a_pObserver, a_tCookie); }

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
	{ return m_pRI->CanvasSet(a_pSize, a_pResolution, a_pContentTransform, a_pHelper); }
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
	{ return m_pRI->ChannelsSet(a_nChannels, a_aChannelIDs, a_aChannelDefaults); }

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
	{
		try
		{
			if (a_nChannelIDs != EICIRGBA)
				return E_RW_INVALIDPARAM;
			TImageSize tOrigSize = {1, 1};
			m_pRI->CanvasGet(&tOrigSize, NULL, NULL, NULL, NULL);
			if (a_pSize == NULL) a_pSize = &tOrigSize;
			ULONG const nPixels = a_pSize->nX*a_pSize->nY;
			if (nPixels == 0)
				return S_FALSE;
			CAutoVectorPtr<TPixelChannel> cPix(new TPixelChannel[nPixels]);
			if (FAILED(m_pRI->TileGet(EICIRGBA, a_pOrigin, a_pSize, NULL, nPixels, cPix, NULL, EIRIAccurate)))
				return E_FAIL;
			TPixelChannel const* pS = a_pPixels;
			TPixelChannel* pD = cPix;
			ULONG const nStrideX = a_pStride ? a_pStride->nX : 1;
			ULONG const nStrideYDelta = a_pStride ? a_pStride->nY-a_pSize->nX*nStrideX : 0;
			CGammaTables const* pGT = m_pGamma->GetSRGBTable();
			for (ULONG y = 0; y < a_pSize->nY; ++y)
			{
				for (ULONG x = 0; x < a_pSize->nX; ++x, ++pD, pS+=nStrideX)
					Mix(*pD, *pS, m_nStrength, pGT);
				pS += nStrideYDelta;
			}
			return m_pRI->TileSet(EICIRGBA, a_pOrigin, a_pSize, NULL, nPixels, cPix, a_bDeleteOldContent);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
	{
		// TODO: optimize?
		TImageStride tStride = {1, a_tAllocSize.nX};
		TImagePoint tOrigin = a_tAllocOrigin;
		TImageSize tSize = a_tAllocSize;
		TPixelChannel* pPixels = a_pPixels;
		if (a_pContentOrigin && a_pContentSize)
		{
			pPixels += (a_pContentOrigin->nX-a_tAllocOrigin.nX) + (a_pContentOrigin->nY-a_tAllocOrigin.nY)*tStride.nY;
			tOrigin = *a_pContentOrigin;
			tSize = *a_pContentSize;
		}
		HRESULT hRes = TileSet(EICIRGBA, &tOrigin, &tSize, &tStride, tSize.nX*tSize.nY, pPixels, TRUE);
		(*a_pDeleter)(a_pPixels);
		return hRes;
	}
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
	{
		return m_pRI->BufferAllocate(a_tSize, a_ppPixels, a_ppDeleter);
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentRasterImage> m_pRI;
	CComPtr<IGammaTableCache> m_pGamma;
	int m_nStrength;
};

