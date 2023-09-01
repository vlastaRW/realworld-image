
#pragma once

#include <SubjectNotImpl.h>


class ATL_NO_VTABLE CLayerRasterImageRectangle : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CSubjectNotImpl<IDocument, IDocumentObserver, ULONG>,
	public CSubjectNotImpl<IDocumentRasterImage, IImageObserver, TImageChange>
{
public:
	void Init(TImageSize const& a_tCanvasSize, float a_fGamma, TPixelChannel const* a_pImgData, TImagePoint const& a_tImgOffset, TImageSize const& a_tImgSize, TPixelChannel a_tDefault, TPixelChannel* a_pRectData, TRasterImageRect const& a_tRect, ULONG a_nStride1)
	{
		m_tCanvasSize = a_tCanvasSize;
		m_fGamma = a_fGamma;
		m_pImgData = a_pImgData;
		m_tOffset = a_tImgOffset;
		m_tSize = a_tImgSize;
		m_tDefault = a_tDefault;
		m_pRectData = a_pRectData;
		m_tRect = a_tRect;
		m_nStride1 = a_nStride1;
	}

BEGIN_COM_MAP(CLayerRasterImageRectangle)
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
			CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[m_tCanvasSize.nX*m_tCanvasSize.nY]);
			TileGet(EICIRGBA, NULL, NULL, NULL, m_tCanvasSize.nX*m_tCanvasSize.nY, pBuffer, NULL, EIRIPreview);
			return pFct->Create(a_bstrPrefix, a_pBase, &m_tCanvasSize, NULL, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(m_tCanvasSize.nX, m_tCanvasSize.nY, pBuffer));
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
			if (a_pCanvasSize) *a_pCanvasSize = m_tCanvasSize;
			if (a_pResolution) { a_pResolution->nNumeratorX = a_pResolution->nNumeratorY = 100; a_pResolution->nDenominatorX = a_pResolution->nDenominatorY = 254; }
			if (a_pContentOrigin)
			{
				a_pContentOrigin->nX = m_tOffset.nX < m_tRect.tTL.nX ? m_tOffset.nX : m_tRect.tTL.nX;
				a_pContentOrigin->nY = m_tOffset.nY < m_tRect.tTL.nY ? m_tOffset.nY : m_tRect.tTL.nY;
			}
			if (a_pContentSize)
			{
				LONG const nX1 = m_tOffset.nX < m_tRect.tTL.nX ? m_tOffset.nX : m_tRect.tTL.nX;
				LONG const nY1 = m_tOffset.nY < m_tRect.tTL.nY ? m_tOffset.nY : m_tRect.tTL.nY;
				LONG const nX2 = m_tOffset.nX+LONG(m_tSize.nX) > m_tRect.tBR.nX ? m_tOffset.nX+LONG(m_tSize.nX) : m_tRect.tBR.nX;
				LONG const nY2 = m_tOffset.nY+LONG(m_tSize.nY) > m_tRect.tBR.nY ? m_tOffset.nY+LONG(m_tSize.nY) : m_tRect.tBR.nY;
				a_pContentSize->nX = nX2-nX1;
				a_pContentSize->nY = nY2-nY1;
			}
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
		try
		{
			if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
			if (a_pGamma) *a_pGamma = m_fGamma;
			static EImageChannelID const tChID = EICIRGBA;
			if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, CChannelDefault(EICIRGBA, m_tDefault));
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		try
		{
			if (a_nChannelIDs != EICIRGBA)
				return E_RW_INVALIDPARAM;
			static TImagePoint const t0 = {0, 0};
			if (a_pOrigin == NULL)
				a_pOrigin = &t0;
			if (a_pSize == NULL)
				a_pSize = &m_tCanvasSize;
			TImageStride tStr = {1, a_pSize->nX};
			if (a_pStride == NULL)
				a_pStride = &tStr;
			TImagePoint const t1 = {a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY};
			if (a_pOrigin->nX >= m_tRect.tBR.nX || a_pOrigin->nY >= m_tRect.tBR.nY || t1.nX <= m_tRect.tTL.nX || t1.nY <= m_tRect.tTL.nY)
				return RGBAGetTileImpl(m_tCanvasSize, m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
			TImagePoint const tTL = {a_pOrigin->nX > m_tRect.tTL.nX ? a_pOrigin->nX : m_tRect.tTL.nX, a_pOrigin->nY > m_tRect.tTL.nY ? a_pOrigin->nY : m_tRect.tTL.nY};
			TImagePoint const tBR = {t1.nX < m_tRect.tBR.nX ? t1.nX : m_tRect.tBR.nX, t1.nY < m_tRect.tBR.nY ? t1.nY : m_tRect.tBR.nY};
			if (a_pOrigin->nY < tTL.nY)
				RGBAGetTileImpl(m_tCanvasSize, m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, a_pOrigin, CImageSize(a_pSize->nX, m_tRect.tTL.nY-a_pOrigin->nY), a_pStride, a_nPixels, a_pData);
			if (a_pOrigin->nX < tTL.nX)
				RGBAGetTileImpl(m_tCanvasSize, m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(a_pOrigin->nX, tTL.nY), CImageSize(tTL.nX-a_pOrigin->nX, tBR.nY-tTL.nY), a_pStride, a_nPixels, a_pData+(tTL.nY-a_pOrigin->nY)*a_pStride->nY);
			RGBAGetTileImpl(m_tCanvasSize, m_tRect.tTL, CImageSize(m_tRect.tBR.nX-m_tRect.tTL.nX, m_tRect.tBR.nY-m_tRect.tTL.nY), m_pRectData, m_nStride1, CPixelChannel(0, 0, 0, 0), &tTL, CImageSize(tBR.nX-tTL.nX, tBR.nY-tTL.nY), a_pStride, a_nPixels, a_pData+(tTL.nY-a_pOrigin->nY)*a_pStride->nY+(tTL.nX-a_pOrigin->nX)*a_pStride->nX);
			if (t1.nX > tBR.nX)
				RGBAGetTileImpl(m_tCanvasSize, m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(tBR.nX, tTL.nY), CImageSize(t1.nX-tBR.nX, tBR.nY-tTL.nY), a_pStride, a_nPixels, a_pData+(tTL.nY-a_pOrigin->nY)*a_pStride->nY+(tBR.nX-a_pOrigin->nX)*a_pStride->nX);
			if (t1.nY > tBR.nY)
				RGBAGetTileImpl(m_tCanvasSize, m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(a_pOrigin->nX, tBR.nY), CImageSize(a_pSize->nX, t1.nY-tBR.nY), a_pStride, a_nPixels, a_pData+(tBR.nY-a_pOrigin->nY)*a_pStride->nY);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		try
		{
			if (a_nChannelIDs != EICIRGBA)
				return E_RW_INVALIDPARAM;
			static TImagePoint const t0 = {0, 0};
			if (a_pOrigin == NULL)
				a_pOrigin = &t0;
			if (a_pSize == NULL)
				a_pSize = &m_tCanvasSize;
			TImagePoint const t1 = {a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY};
			if (a_pOrigin->nX >= m_tRect.tBR.nX || a_pOrigin->nY >= m_tRect.tBR.nY || t1.nX <= m_tRect.tTL.nX || t1.nY <= m_tRect.tTL.nY)
				return RGBAInspectImpl(m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pVisitor, a_pControl);

			TImageTile aTiles[25];
			ULONG nTiles = 0;
			CComObjectStackEx<CImageVisitor> cVisitor;
			cVisitor.Init(aTiles, &nTiles);

			TImagePoint const tTL = {a_pOrigin->nX > m_tRect.tTL.nX ? a_pOrigin->nX : m_tRect.tTL.nX, a_pOrigin->nY > m_tRect.tTL.nY ? a_pOrigin->nY : m_tRect.tTL.nY};
			TImagePoint const tBR = {t1.nX < m_tRect.tBR.nX ? t1.nX : m_tRect.tBR.nX, t1.nY < m_tRect.tBR.nY ? t1.nY : m_tRect.tBR.nY};
			if (a_pOrigin->nY < tTL.nY)
				RGBAInspectImpl(m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, a_pOrigin, CImageSize(a_pSize->nX, m_tRect.tTL.nY-a_pOrigin->nY), &cVisitor, NULL);
			if (a_pOrigin->nX < tTL.nX)
				RGBAInspectImpl(m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(a_pOrigin->nX, tTL.nY), CImageSize(tTL.nX-a_pOrigin->nX, tBR.nY-tTL.nY), &cVisitor, NULL);
			RGBAInspectImpl(m_tRect.tTL, CImageSize(m_tRect.tBR.nX-m_tRect.tTL.nX, m_tRect.tBR.nY-m_tRect.tTL.nY), m_pRectData, m_nStride1, CPixelChannel(0, 0, 0, 0), &tTL, CImageSize(tBR.nX-tTL.nX, tBR.nY-tTL.nY), &cVisitor, NULL);
			if (t1.nX > tBR.nX)
				RGBAInspectImpl(m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(tBR.nX, tTL.nY), CImageSize(t1.nX-tBR.nX, tBR.nY-tTL.nY), &cVisitor, NULL);
			if (t1.nY > tBR.nY)
				RGBAInspectImpl(m_tOffset, m_tSize, m_pImgData, m_tSize.nX, m_tDefault, CImagePoint(a_pOrigin->nX, tBR.nY), CImageSize(a_pSize->nX, t1.nY-tBR.nY), &cVisitor, NULL);

			return a_pVisitor->Visit(nTiles, aTiles, a_pControl);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* UNREF(a_pSize), TImageResolution const* UNREF(a_pResolution), TMatrix3x3f const* UNREF(a_pContentTransform), IRasterImageTransformer* UNREF(a_pHelper))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
	{
		return E_NOTIMPL;
	}

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
	{
		try
		{
			if (a_nChannelIDs != EICIRGBA)
				return E_RW_INVALIDPARAM;
			static TImagePoint const t0 = {0, 0};
			if (a_pOrigin == NULL)
				a_pOrigin = &t0;
			TImagePoint const t1 = {a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY};
			if (a_pOrigin->nX >= m_tRect.tBR.nX || a_pOrigin->nY >= m_tRect.tBR.nY ||
				t1.nX <= m_tRect.tTL.nX || t1.nY <= m_tRect.tTL.nY)
			{
				if (a_bDeleteOldContent)
					if (m_nStride1 == (m_tRect.tBR.nX-m_tRect.tTL.nX))
						std::fill_n(m_pRectData, (m_tRect.tBR.nX-m_tRect.tTL.nX)*(m_tRect.tBR.nY-m_tRect.tTL.nY), m_tDefault);
					else
						for (LONG y = m_tRect.tTL.nY; y < m_tRect.tBR.nY; ++y)
							std::fill_n(m_pRectData+m_nStride1*(y-m_tRect.tTL.nY), m_tRect.tBR.nX-m_tRect.tTL.nX, m_tDefault);
				return S_OK;
			}
			if (a_pStride && a_pStride->nX != 1)
			{
				ATLASSERT(0);
				return E_FAIL;
			}
			if (a_bDeleteOldContent)
			{
				return RGBAGetTileImpl(m_tCanvasSize, *a_pOrigin, *a_pSize, a_pPixels, a_pStride ? a_pStride->nY : a_pSize->nX, m_tDefault, &m_tRect.tTL, CImageSize(m_tRect.tBR.nX-m_tRect.tTL.nX, m_tRect.tBR.nY-m_tRect.tTL.nY), CImageStride(1, m_nStride1), m_nStride1*(m_tRect.tBR.nY-m_tRect.tTL.nY), m_pRectData);
			}
			else
			{
				TImagePoint const tTL = {a_pOrigin->nX > m_tRect.tTL.nX ? a_pOrigin->nX : m_tRect.tTL.nX, a_pOrigin->nY > m_tRect.tTL.nY ? a_pOrigin->nY : m_tRect.tTL.nY};
				TImagePoint const tBR = {t1.nX < m_tRect.tBR.nX ? t1.nX : m_tRect.tBR.nX, t1.nY < m_tRect.tBR.nY ? t1.nY : m_tRect.tBR.nY};
				TImageSize const tSz = {tBR.nX-tTL.nX, tBR.nY-tTL.nY};
				return RGBAGetTileImpl(m_tCanvasSize, *a_pOrigin, *a_pSize, a_pPixels, a_pStride ? a_pStride->nY : a_pSize->nX, m_tDefault, &tTL, &tSz, CImageStride(1, m_nStride1), m_nStride1*tSz.nY, m_pRectData+(tTL.nY-m_tRect.tTL.nY)*m_nStride1+(tTL.nX-m_tRect.tTL.nX));
			}
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter) { return E_NOTIMPL; }
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter) { return E_NOTIMPL; }

private:
	class ATL_NO_VTABLE CImageVisitor : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IImageVisitor
	{
	public:
		void Init(TImageTile* a_aTiles, ULONG* a_nTiles)
		{
			m_aTiles = a_aTiles;
			m_nTiles = a_nTiles;
		}

	BEGIN_COM_MAP(CImageVisitor)
		COM_INTERFACE_ENTRY(IImageVisitor)
	END_COM_MAP()

		// IImageVisitor methods
	public:
		STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* a_pControl)
		{
			while (a_nTiles)
			{
				*m_aTiles = *a_aTiles;
				++m_aTiles;
				++a_aTiles;
				--a_nTiles;
				++*m_nTiles;
			}
			return S_OK;
		}

	private:
		TImageTile* m_aTiles;
		ULONG* m_nTiles;
	};

private:
	TImageSize m_tCanvasSize;
	float m_fGamma;
	TPixelChannel const* m_pImgData;
	TImagePoint m_tOffset;
	TImageSize m_tSize;
	TPixelChannel m_tDefault;
	TPixelChannel* m_pRectData;
	TRasterImageRect m_tRect;
	ULONG m_nStride1;
};

