
#pragma once

#include <SubjectNotImpl.h>


class ATL_NO_VTABLE CRasterImageOperationStep : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CSubjectNotImpl<IDocument, IDocumentObserver, ULONG>,
	public CSubjectNotImpl<IDocumentRasterImage, IImageObserver, TImageChange>
{
public:
	void Init(IDocumentImage* a_pDocImg, TImagePoint const* a_pReadOffset, TImageSize const* a_pReadSize)
	{
		m_pDocImg = a_pDocImg;
		m_pDocImg->CanvasGet(&m_tCanvas, &m_tRes, &m_tOrigin, &m_tSize, &m_eOpacity);

		if (a_pReadOffset && a_pReadSize)
		{
			m_tReadOrigin = *a_pReadOffset;
			m_tReadSize = *a_pReadSize;
		}
		else
		{
			m_tReadOrigin = m_tOrigin;
			m_tReadSize = m_tSize;
		}

		RWCoCreateInstance(m_pDst, __uuidof(DocumentBase));
		CComObject<CDocumentRasterImage>* pImg = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&pImg);
		CComPtr<IDocumentData> pData(pImg);
		pImg->Init(m_tCanvas, &m_tRes, NULL);
		CComQIPtr<IDocumentBase>(m_pDst)->DataBlockSet(NULL, pData);
		m_pDst->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&m_pDstImg));
		m_bTouched = false;
	}
	void SwapRes(CComPtr<IDocument>& pRhs, CComPtr<IDocumentImage>& pRhsImg)
	{
		if (m_bTouched)
		{
			if (m_pDstImg.p != pRhsImg.p)
				pRhsImg.Attach(m_pDstImg.Detach());
			if (m_pDst.p != pRhs.p)
				pRhs.Attach(m_pDst.Detach());
		}
	}

BEGIN_COM_MAP(CRasterImageOperationStep)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IBlockOperations)
	COM_INTERFACE_ENTRY(IDocumentRasterImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IDocumentImage)
END_COM_MAP()

	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)()
	{
		return S_FALSE;
	}
	STDMETHOD(WriteUnlock)()
	{
		return S_FALSE;
	}
	STDMETHOD(ReadLock)()
	{
		return S_FALSE;
	}
	STDMETHOD(ReadUnlock)()
	{
		return S_FALSE;
	}

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
	{
		*a_pguidBuilder = CLSID_DocumentFactoryRasterImage;
		return S_OK;
	}
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
	{
		return QueryInterface(a_iid, a_ppFeatureInterface);
	}
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ClearDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
	{
		try
		{
			CComPtr<IDocumentFactoryRasterImage> pFct;
			RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
			CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[m_tCanvas.nX*m_tCanvas.nY]);
			TileGet(EICIRGBA, NULL, NULL, NULL, m_tCanvas.nX*m_tCanvas.nY, pBuffer, NULL, EIRIPreview);
			return pFct->Create(a_bstrPrefix, a_pBase, &m_tCanvas, NULL, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(m_tCanvas.nX, m_tCanvas.nY, pBuffer));
		}
		catch (...)
		{
			return a_pBase ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(QuickInfo)(ULONG UNREF(a_nInfoIndex), ILocalizedString** UNREF(a_ppInfo))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(DocumentTypeGet)(IDocumentType** a_ppDocumentType)
	{
		return E_NOTIMPL;
	}

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
	{
		try
		{
			if (a_pCanvasSize) *a_pCanvasSize = m_tCanvas;
			if (a_pResolution) *a_pResolution = m_tRes;
			if (a_pContentOrigin) *a_pContentOrigin = m_tReadOrigin;
			if (a_pContentSize) *a_pContentSize = m_tReadSize;
			if (a_pContentOpacity) *a_pContentOpacity = m_eOpacity;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
	{
		return m_pDocImg->ChannelsGet(a_pChannelIDs, a_pGamma, a_pChannelDefaults);
	}
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		return m_pDocImg->TileGet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData, a_pControl, a_eIntent);
	}
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		return m_pDocImg->Inspect(a_nChannelIDs, a_pOrigin, a_pSize, a_pVisitor, a_pControl, a_eIntent);
	}
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		return m_pDocImg->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent);
	}
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
	{
		return m_pDocImg->BufferUnlock(a_nChannelID, a_pBuffer);
	}

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* UNREF(a_pSize), TImageResolution const* UNREF(a_pResolution), TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
	{
		if (a_pContentTransform && a_pHelper)
		{
			m_bTouched = true;
			TPixelChannel tDefault;
			m_pDocImg->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tDefault));

			SLockedImageBuffer cBuf(m_pDocImg);
			
			TVector2f t00 = {cBuf.tContentOrigin.nX, cBuf.tContentOrigin.nY};
			TVector2f t11 = {cBuf.tContentOrigin.nX+cBuf.tContentSize.nX, cBuf.tContentOrigin.nY+cBuf.tContentSize.nY};
			TVector2f t01 = {t00.x, t11.y};
			TVector2f t10 = {t11.x, t00.y};
			t00 = TransformVector2(*a_pContentTransform, t00);
			t01 = TransformVector2(*a_pContentTransform, t01);
			t10 = TransformVector2(*a_pContentTransform, t10);
			t11 = TransformVector2(*a_pContentTransform, t11);
			TImagePoint tNewOrigin = {floor(t00.x), floor(t00.y)};
			if (floor(t01.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t01.x);
			if (floor(t01.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t01.y);
			if (floor(t10.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t10.x);
			if (floor(t10.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t10.y);
			if (floor(t11.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t11.x);
			if (floor(t11.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t11.y);
			TImagePoint tNewEnd = {ceil(t00.x), ceil(t00.y)};
			if (ceil(t01.x) > tNewEnd.nX) tNewEnd.nX = ceil(t01.x);
			if (ceil(t01.y) > tNewEnd.nY) tNewEnd.nY = ceil(t01.y);
			if (ceil(t10.x) > tNewEnd.nX) tNewEnd.nX = ceil(t10.x);
			if (ceil(t10.y) > tNewEnd.nY) tNewEnd.nY = ceil(t10.y);
			if (ceil(t11.x) > tNewEnd.nX) tNewEnd.nX = ceil(t11.x);
			if (ceil(t11.y) > tNewEnd.nY) tNewEnd.nY = ceil(t11.y);
			TImageSize const tNewSize = {tNewEnd.nX-tNewOrigin.nX, tNewEnd.nY-tNewOrigin.nY};
			ULONG const nPixels = tNewSize.nX*tNewSize.nY;
			CAutoPixelBuffer cOutBuf(m_pDstImg, tNewSize);
			if (nPixels)
				a_pHelper->ProcessTile(EICIRGBA, 2.2f, &tDefault, a_pContentTransform, cBuf.tAllocSize.nX*cBuf.tContentSize.nY, cBuf.pData+cBuf.tAllocSize.nX*(cBuf.tContentOrigin.nY-cBuf.tAllocOrigin.nY)+cBuf.tContentOrigin.nX-cBuf.tAllocOrigin.nX, &cBuf.tContentOrigin, &cBuf.tContentSize, CImageStride(1, cBuf.tAllocSize.nX), tNewSize.nX*tNewSize.nY, cOutBuf.Buffer(), &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));

			return cOutBuf.Replace(tNewOrigin, NULL, NULL, NULL, tDefault);
		}
		return E_NOTIMPL;
	}
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
	{
		m_bTouched = true;
		return m_pDstImg->ChannelsSet(a_nChannels, a_aChannelIDs, a_aChannelDefaults);
	}

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
	{
		m_bTouched = true;
		if (a_bDeleteOldContent) 
			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, a_bDeleteOldContent);
		static TImagePoint const tPt0 = TIMGPOINT_NULL;
		if (a_pOrigin == NULL)
		{
			a_pOrigin = &tPt0;
			if (a_pSize == NULL)
				a_pSize = &m_tCanvas;
		}
		else if (a_pSize == NULL)
			return E_RW_INVALIDPARAM;
		if (a_pOrigin->nX <= m_tReadOrigin.nX && a_pOrigin->nX <= m_tReadOrigin.nX &&
			a_pOrigin->nX+a_pSize->nX >= m_tReadOrigin.nX+m_tReadSize.nX &&
			a_pOrigin->nY+a_pSize->nY >= m_tReadOrigin.nY+m_tReadSize.nY)
			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, TRUE);

		//ATLASSERT(FALSE); // have to? combine old and new content
		try
		{
			CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[m_tReadSize.nX*m_tReadSize.nY]);
			m_pDocImg->TileGet(EICIRGBA, &m_tReadOrigin, &m_tReadSize, NULL, m_tReadSize.nX*m_tReadSize.nY, cBuffer, NULL, EIRIAccurate);

			HRESULT hRes = m_pDstImg->BufferReplace(m_tReadOrigin, m_tReadSize, &m_tReadOrigin, &m_tReadSize, NULL, cBuffer, &DefDeleter);
			if (SUCCEEDED(hRes)) cBuffer.Detach();

			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, FALSE);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
	{
		return m_pDstImg->BufferAllocate(a_tSize, a_ppPixels, a_ppDeleter);
	}
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
	{
		m_bTouched = true;
		return m_pDstImg->BufferReplace(a_tAllocOrigin, a_tAllocSize, a_pContentOrigin, a_pContentSize, a_pContentAlphaSum, a_pPixels, a_pDeleter);
	}


private:
	static void DefDeleter(TPixelChannel* p) { delete[] p; }

private:
	CComPtr<IDocumentImage> m_pDocImg;

	// original doc cached values
	TImageSize m_tCanvas;
	TImageResolution m_tRes;
	TImagePoint m_tOrigin;
	TImageSize m_tSize;
	EImageOpacity m_eOpacity;

	TImagePoint m_tReadOrigin;
	TImageSize m_tReadSize;

	CComPtr<IDocument> m_pDst;
	CComPtr<IDocumentRasterImage> m_pDstImg;
	bool m_bTouched;
};


class ATL_NO_VTABLE CRasterImageOperationStepRectangle : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CSubjectNotImpl<IDocument, IDocumentObserver, ULONG>,
	public CSubjectNotImpl<IDocumentRasterImage, IImageObserver, TImageChange>
{
public:
	void Init(TImageSize a_tCanvas, TPixelChannel* a_pBuffer, TImagePoint a_tOrigin, TImageSize a_tSize, ULONG a_nStride, TPixelChannel a_tDefault)
	{
		m_tCanvas = a_tCanvas;
		m_pOrigBuffer = a_pBuffer;
		m_tOrigOrigin = a_tOrigin;
		m_tOrigSize = a_tSize;
		m_nOrigStride = a_nStride;
		m_tDefault = a_tDefault;

		RWCoCreateInstance(m_pDst, __uuidof(DocumentBase));
		CComObject<CDocumentRasterImage>* pImg = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&pImg);
		CComPtr<IDocumentData> pData(pImg);
		pImg->Init(m_tCanvas, NULL, &a_tDefault);
		CComQIPtr<IDocumentBase>(m_pDst)->DataBlockSet(NULL, pData);
		m_pDst->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&m_pDstImg));
		m_bTouched = false;
	}
	void SwapRes(CComPtr<IDocumentImage>& pRhs)
	{
		if (m_bTouched)
		{
			if (m_pDstImg.p != pRhs.p)
				pRhs.Attach(m_pDstImg.Detach());
		}
	}

BEGIN_COM_MAP(CRasterImageOperationStepRectangle)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IBlockOperations)
	COM_INTERFACE_ENTRY(IDocumentRasterImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IDocumentImage)
END_COM_MAP()

	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)()
	{
		return S_FALSE;
	}
	STDMETHOD(WriteUnlock)()
	{
		return S_FALSE;
	}
	STDMETHOD(ReadLock)()
	{
		return S_FALSE;
	}
	STDMETHOD(ReadUnlock)()
	{
		return S_FALSE;
	}

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
	{
		*a_pguidBuilder = CLSID_DocumentFactoryRasterImage;
		return S_OK;
	}
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
	{
		return QueryInterface(a_iid, a_ppFeatureInterface);
	}
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SetDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ClearDirty)()
	{
		return E_NOTIMPL;
	}
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
	{
		try
		{
			CComPtr<IDocumentFactoryRasterImage> pFct;
			RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
			CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[m_tCanvas.nX*m_tCanvas.nY]);
			TileGet(EICIRGBA, NULL, NULL, NULL, m_tCanvas.nX*m_tCanvas.nY, pBuffer, NULL, EIRIPreview);
			return pFct->Create(a_bstrPrefix, a_pBase, &m_tCanvas, NULL, 1, CChannelDefault(EICIRGBA, m_tDefault), 2.2f, CImageTile(m_tCanvas.nX, m_tCanvas.nY, pBuffer));
		}
		catch (...)
		{
			return a_pBase ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(QuickInfo)(ULONG UNREF(a_nInfoIndex), ILocalizedString** UNREF(a_ppInfo))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(DocumentTypeGet)(IDocumentType** a_ppDocumentType)
	{
		return E_NOTIMPL;
	}

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
	{
		try
		{
			if (a_pCanvasSize) *a_pCanvasSize = m_tCanvas;
			if (a_pResolution) { a_pResolution->nNumeratorX = a_pResolution->nNumeratorY = 100; a_pResolution->nDenominatorX = a_pResolution->nDenominatorY = 254; }
			if (a_pContentOrigin) *a_pContentOrigin = m_tOrigOrigin;
			if (a_pContentSize) *a_pContentSize = m_tOrigSize;
			if (a_pContentOpacity) *a_pContentOpacity = EIOUnknown;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
	{
		if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
		if (a_pGamma) *a_pGamma = 2.2f;//m_fCachedGamma;
		if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, CChannelDefault(EICIRGBA, m_tDefault));
		return S_OK;
	}
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		return RGBAGetTileImpl(m_tCanvas, m_tOrigOrigin, m_tOrigSize, m_pOrigBuffer, m_nOrigStride, CPixelChannel(0, 0, 0, 0), a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
	}
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		return RGBAInspectImpl(m_tOrigOrigin, m_tOrigSize, m_pOrigBuffer, m_nOrigStride, CPixelChannel(0, 0, 0, 0), a_pOrigin, a_pSize, a_pVisitor, a_pControl);
	}
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		if (a_nChannelID != EICIRGBA)
			return E_RW_INVALIDPARAM;
		try
		{
			if (a_pAllocOrigin) *a_pAllocOrigin = m_tOrigOrigin;
			if (a_pAllocSize) { a_pAllocSize->nX = m_nOrigStride; a_pAllocSize->nY = m_tOrigSize.nY; }
			if (a_pContentOrigin) *a_pContentOrigin = m_tOrigOrigin;
			if (a_pContentSize) *a_pContentSize = m_tOrigSize;
			if (a_ppBuffer) *a_ppBuffer = m_pOrigBuffer;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
	{
		if (a_nChannelID != EICIRGBA || a_pBuffer != m_pOrigBuffer)
			return E_RW_INVALIDPARAM;
		return S_OK;
	}

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* UNREF(a_pSize), TImageResolution const* UNREF(a_pResolution), TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
	{
		if (a_pContentTransform && a_pHelper)
		{
			m_bTouched = true;
			TPixelChannel tDefault;
			tDefault.n = 0;

			TVector2f t00 = {m_tOrigOrigin.nX, m_tOrigOrigin.nY};
			TVector2f t11 = {m_tOrigOrigin.nX+m_tOrigSize.nX, m_tOrigOrigin.nY+m_tOrigSize.nY};
			TVector2f t01 = {t00.x, t11.y};
			TVector2f t10 = {t11.x, t00.y};
			t00 = TransformVector2(*a_pContentTransform, t00);
			t01 = TransformVector2(*a_pContentTransform, t01);
			t10 = TransformVector2(*a_pContentTransform, t10);
			t11 = TransformVector2(*a_pContentTransform, t11);
			TImagePoint tNewOrigin = {floor(t00.x), floor(t00.y)};
			if (floor(t01.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t01.x);
			if (floor(t01.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t01.y);
			if (floor(t10.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t10.x);
			if (floor(t10.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t10.y);
			if (floor(t11.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t11.x);
			if (floor(t11.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t11.y);
			TImagePoint tNewEnd = {ceil(t00.x), ceil(t00.y)};
			if (ceil(t01.x) > tNewEnd.nX) tNewEnd.nX = ceil(t01.x);
			if (ceil(t01.y) > tNewEnd.nY) tNewEnd.nY = ceil(t01.y);
			if (ceil(t10.x) > tNewEnd.nX) tNewEnd.nX = ceil(t10.x);
			if (ceil(t10.y) > tNewEnd.nY) tNewEnd.nY = ceil(t10.y);
			if (ceil(t11.x) > tNewEnd.nX) tNewEnd.nX = ceil(t11.x);
			if (ceil(t11.y) > tNewEnd.nY) tNewEnd.nY = ceil(t11.y);
			TImageSize const tNewSize = {tNewEnd.nX-tNewOrigin.nX, tNewEnd.nY-tNewOrigin.nY};
			ULONG const nPixels = tNewSize.nX*tNewSize.nY;
			CAutoPixelBuffer cOutBuf(m_pDstImg, tNewSize);
			if (nPixels)
				a_pHelper->ProcessTile(EICIRGBA, 2.2f, &tDefault, a_pContentTransform, m_nOrigStride*m_tOrigSize.nY, m_pOrigBuffer, &m_tOrigOrigin, &m_tOrigSize, CImageStride(1, m_nOrigStride), tNewSize.nX*tNewSize.nY, cOutBuf.Buffer(), &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));

			return cOutBuf.Replace(tNewOrigin, NULL, NULL, NULL, tDefault);
		}
		return E_NOTIMPL;
	}
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
	{
		m_bTouched = true;
		return m_pDstImg->ChannelsSet(a_nChannels, a_aChannelIDs, a_aChannelDefaults);
	}

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
	{
		m_bTouched = true;
		if (a_bDeleteOldContent) 
			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, a_bDeleteOldContent);
		static TImagePoint const tPt0 = TIMGPOINT_NULL;
		if (a_pOrigin == NULL)
		{
			a_pOrigin = &tPt0;
			if (a_pSize == NULL)
				a_pSize = &m_tCanvas;
		}
		else if (a_pSize == NULL)
			return E_RW_INVALIDPARAM;
		if (a_pOrigin->nX <= m_tOrigOrigin.nX && a_pOrigin->nX <= m_tOrigOrigin.nX &&
			a_pOrigin->nX+a_pSize->nX >= m_tOrigOrigin.nX+m_tOrigSize.nX &&
			a_pOrigin->nY+a_pSize->nY >= m_tOrigOrigin.nY+m_tOrigSize.nY)
			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, TRUE);

		ATLASSERT(FALSE); // have to? combine old and new content
		try
		{
			CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[m_tOrigSize.nX*m_tOrigSize.nY]);
			TileGet(EICIRGBA, &m_tOrigOrigin, &m_tOrigSize, NULL, m_tOrigSize.nX*m_tOrigSize.nY, cBuffer, NULL, EIRIAccurate);

			HRESULT hRes = m_pDstImg->BufferReplace(m_tOrigOrigin, m_tOrigSize, &m_tOrigOrigin, &m_tOrigSize, NULL, cBuffer, &DefDeleter);
			if (SUCCEEDED(hRes)) cBuffer.Detach();

			return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, FALSE);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
	{
		return m_pDstImg->BufferAllocate(a_tSize, a_ppPixels, a_ppDeleter);
	}
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
	{
		m_bTouched = true;
		return m_pDstImg->BufferReplace(a_tAllocOrigin, a_tAllocSize, a_pContentOrigin, a_pContentSize, a_pContentAlphaSum, a_pPixels, a_pDeleter);
	}


private:
	static void DefDeleter(TPixelChannel* p) { delete[] p; }

private:
	TImageSize m_tCanvas;
	TImagePoint m_tOrigOrigin;
	TImageSize m_tOrigSize;
	TPixelChannel* m_pOrigBuffer;
	ULONG m_nOrigStride;
	TPixelChannel m_tDefault;

	CComPtr<IDocument> m_pDst;
	CComPtr<IDocumentRasterImage> m_pDstImg;
	bool m_bTouched;
};
