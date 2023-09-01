// DocumentCodecRRI.cpp : Implementation of CDocumentCodecRRI

#include "stdafx.h"
#include "DocumentCodecRRI.h"

#include <MultiLanguageString.h>


// CDocumentCodecRRI

STDMETHODIMP CDocumentCodecRRI::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA ENCFEAT_IMAGE_CANVAS);
		return pDI ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static BYTE const RRI_HEADER[] = {'R', 'W', 'R', 'a', 's', 't', 'e', 'r', 'I', 'm', 'a', 'g', 'e', '_', '_', '1'};
static DWORD const MARK_SIZE = mmioFOURCC('S', 'I', 'Z', 'E');
static DWORD const MARK_RESOLUTION = mmioFOURCC('R', 'S', 'L', 'T');
static DWORD const MARK_BACKGROUND = mmioFOURCC('B', 'G', 'N', 'D');
static DWORD const MARK_OFFSET = mmioFOURCC('O', 'F', 'S', 'T');
static DWORD const MARK_PIXELS = mmioFOURCC('P', 'X', 'L', 'S');
static BYTE const DWORD_PADDING[4] = {0, 0, 0, 0};

class CImageChannelWriter : public IEnumImageChannels
{
public:
	CImageChannelWriter(std::vector<TChannelDefault>& a_cVals) : m_cVals(a_cVals) {}

	operator IEnumImageChannels*() { return this; }

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IEnumImageChannels)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IEnumImageChannels methods
public:
	STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, TChannelDefault const* a_aChannelDefaults)
	{
		for (; a_nCount > 0; ++a_aChannelDefaults, --a_nCount)
			m_cVals.push_back(*a_aChannelDefaults);
		return S_OK;
	}

private:
	std::vector<TChannelDefault>& m_cVals;
};

class ATL_NO_VTABLE COffsetedImageDocument : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocument,
	public IDocumentImage
{
public:
	void Init(IDocumentImage* a_pOrigImg, TImagePoint a_tOffset, TImageSize a_tSize)
	{
		m_pImage = a_pOrigImg;
		m_tOffset = a_tOffset;
		m_tSize = a_tSize;
	}

BEGIN_COM_MAP(COffsetedImageDocument)
	COM_INTERFACE_ENTRY(IDocument)
	COM_INTERFACE_ENTRY(IDocumentImage)
END_COM_MAP()


	// IBlockOperations methods
public:
	STDMETHOD(WriteLock)() { return S_OK; }
	STDMETHOD(WriteUnlock)() { return S_OK; }
	STDMETHOD(ReadLock)() { return S_OK; }
	STDMETHOD(ReadUnlock)() { return S_OK; }

	// IDocument methods
public:
	STDMETHOD(BuilderID)(CLSID* UNREF(a_pguidBuilder)) { return E_NOTIMPL; }
	STDMETHOD(EncoderGet)(CLSID*, IConfig**) { return E_NOTIMPL; }
	STDMETHOD(EncoderSet)(REFCLSID, IConfig*) { return S_OK; }
	STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface) { return QueryInterface(a_iid, a_ppFeatureInterface); }
	STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation) { return E_NOTIMPL; }
	STDMETHOD(LocationSet)(IStorageFilter* a_pLocation) { return E_NOTIMPL; }
	STDMETHOD(IsDirty)() { return S_FALSE; }
	STDMETHOD(SetDirty)() { return S_OK; }
	STDMETHOD(ClearDirty)() { return S_OK; }
	STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)
	{
		static float const aVals[2] = {100.0f, 0.1f};
		CComBSTR bstrENCFEAT_IMAGE(ENCFEAT_IMAGE);
		CComBSTR bstrENCFEAT_IMAGE_ALPHA(ENCFEAT_IMAGE_ALPHA);
		BSTR aIDs[2] = {bstrENCFEAT_IMAGE, bstrENCFEAT_IMAGE_ALPHA};
		a_pEnumAspects->Consume(0, 2, aIDs, aVals);
		return S_OK;
	}
	STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
	{
		ATLASSERT(0);
		return E_NOTIMPL;//a_pBase->DataBlockSet(a_bstrPrefix, this);
	}
	STDMETHOD(ObserverIns)(IDocumentObserver*, TCookie) { return S_FALSE; }
	STDMETHOD(ObserverDel)(IDocumentObserver*, TCookie) { return S_FALSE; }
	STDMETHOD(QuickInfo)(ULONG, ILocalizedString**) { return E_NOTIMPL; }

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
	{
		if (a_pCanvasSize) *a_pCanvasSize = m_tSize;
		if (a_pResolution) ZeroMemory(a_pResolution, sizeof*a_pResolution);
		if (a_pContentOrigin) *a_pContentOrigin = m_tOffset;
		if (a_pContentSize) *a_pContentSize = m_tSize;
		if (a_pContentOpacity) *a_pContentOpacity = EIOSemiTransparent;
		return S_OK;
	}
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
	{
		return m_pImage->ChannelsGet(a_pChannelIDs, a_pGamma, a_pChannelDefaults);
	}
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		return m_pImage->TileGet(EICIRGBA,
			CImagePoint(a_pOrigin ? a_pOrigin->nX+m_tOffset.nX : m_tOffset.nX, a_pOrigin ? a_pOrigin->nY+m_tOffset.nY : m_tOffset.nY),
			a_pSize ? a_pSize : &m_tSize, a_pStride, a_nPixels, a_pData, a_pControl, a_eIntent);
	}
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		CComObjectStackEx<CVisitor> cVisitor;
		cVisitor.Init(a_pVisitor, m_tOffset);
		return m_pImage->Inspect(EICIRGBA,
			CImagePoint(a_pOrigin ? a_pOrigin->nX+m_tOffset.nX : m_tOffset.nX, a_pOrigin ? a_pOrigin->nY+m_tOffset.nY : m_tOffset.nY),
			a_pSize ? a_pSize : &m_tSize, &cVisitor, a_pControl, a_eIntent);
	}
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{
		HRESULT hRes = m_pImage->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent);
		if (SUCCEEDED(hRes))
		{
			if (a_pAllocOrigin) { a_pAllocOrigin->nX += m_tOffset.nX; a_pAllocOrigin->nY += m_tOffset.nY; }
			if (a_pContentOrigin) { a_pContentOrigin->nX += m_tOffset.nX; a_pContentOrigin->nY += m_tOffset.nY; }
		}
		return hRes;
	}
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
	{
		return m_pImage->BufferUnlock(a_nChannelID, a_pBuffer);
	}

	STDMETHOD(ObserverIns)(IImageObserver*, TCookie) { return S_FALSE; }
	STDMETHOD(ObserverDel)(IImageObserver*, TCookie) { return S_FALSE; }

private:
	class ATL_NO_VTABLE CVisitor : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IImageVisitor
	{
	public:
		void Init(IImageVisitor* a_pOrigVisitor, TImagePoint a_tOffset)
		{
			m_pVisitor = a_pOrigVisitor;
			m_tOffset = a_tOffset;
		}

	BEGIN_COM_MAP(CVisitor)
		COM_INTERFACE_ENTRY(IImageVisitor)
	END_COM_MAP()


		// IImageVisitor methods
	public:
		STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* a_pControl)
		{
			CAutoVectorPtr<TImageTile> aTiles(new TImageTile[a_nTiles]);
			for (ULONG i = 0; i < a_nTiles; ++i)
			{
				aTiles[i] = a_aTiles[i];
				aTiles[i].tOrigin.nX -= m_tOffset.nX;
				aTiles[i].tOrigin.nY -= m_tOffset.nY;
			}
			return m_pVisitor->Visit(a_nTiles, aTiles, a_pControl);
		}

	private:
		CComPtr<IImageVisitor> m_pVisitor;
		TImagePoint m_tOffset;
	};

private:
	CComPtr<IDocumentImage> m_pImage;
	TImagePoint m_tOffset;
	TImageSize m_tSize;
};

#include <ReturnedData.h>

STDMETHODIMP CDocumentCodecRRI::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl)
{
	try
	{
		CComPtr<IDocumentImage> pDI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
		if (pDI == NULL) return E_NOINTERFACE;

		if (FAILED(a_pDst->Write(sizeof(RRI_HEADER), RRI_HEADER)))
			return E_FAIL;

		TImageSize tSize = {256, 256};
		TImageResolution tRes = {100, 254, 100, 254};
		TImagePoint tContentOffset = {0, 0};
		TImageSize tContentSize = {0, 0};
		ULONG nChIDs = 0;
		float fGamma = 2.2f;
		std::vector<TChannelDefault> cChannelDefs;
		pDI->CanvasGet(&tSize, &tRes, &tContentOffset, &tContentSize, NULL);
		pDI->ChannelsGet(&nChIDs, &fGamma, CImageChannelWriter(cChannelDefs));

		// image size
		if (tSize.nX != 256 || tSize.nY != 256)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_SIZE), reinterpret_cast<BYTE const*>(&MARK_SIZE))))
				return E_FAIL;
			ULONG nLen = sizeof tSize;
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&tSize))))
				return E_FAIL;
		}

		// image resolution
		if (tRes.nNumeratorX != 100 || tRes.nDenominatorX != 254 || tRes.nNumeratorY != 100 || tRes.nDenominatorY != 254)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_RESOLUTION), reinterpret_cast<BYTE const*>(&MARK_RESOLUTION))))
				return E_FAIL;
			ULONG nLen = sizeof tRes;
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&tRes))))
				return E_FAIL;
		}

		// background color
		if (nChIDs != EICIRGBA || fGamma != 2.2f || cChannelDefs.size() != 1 || cChannelDefs[0].eID != EICIRGBA || cChannelDefs[0].tValue.n != 0)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_BACKGROUND), reinterpret_cast<BYTE const*>(&MARK_BACKGROUND))))
				return E_FAIL;
			ULONG nItems = cChannelDefs.size();
			ULONG nLen = sizeof nChIDs+sizeof fGamma+sizeof nItems+sizeof(TChannelDefault)*cChannelDefs.size();
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof nChIDs, reinterpret_cast<BYTE const*>(&nChIDs))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof fGamma, reinterpret_cast<BYTE const*>(&fGamma))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof nItems, reinterpret_cast<BYTE const*>(&nItems))))
				return E_FAIL;
			if (nItems)
				if (FAILED(a_pDst->Write(sizeof(TChannelDefault)*cChannelDefs.size(), reinterpret_cast<BYTE const*>(&(cChannelDefs[0])))))
					return E_FAIL;
		}

		// data
		if (tContentSize.nX && tContentSize.nY)
		{
			CComObjectStackEx<COffsetedImageDocument> cOff;
			cOff.Init(pDI, tContentOffset, tContentSize);

			CComBSTR bstrPNGEncID(L"{1CE9642E-BA8C-4110-87AC-DF39D57C9640}");
			static float const aWeights[2] = {1.0f, -10.0f};
			CComBSTR bstrRRIEncID(L"{BAC1AFF2-F20B-493C-BAB5-DCFD0D0A1BF1}");
			BSTR aIDs[2] = {bstrPNGEncID, bstrRRIEncID};

			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pCfg;
			pIM->FindBestEncoderEx(&cOff, 2, aIDs, aWeights, &tID, &pCfg);

			CReturnedData cDst;
			CComPtr<IDocumentEncoder> pEnc;
			if (!IsEqualGUID(tID, GUID_NULL) && !IsEqualGUID(tID, CLSID_DocumentCodecRRI) && SUCCEEDED(RWCoCreateInstance(pEnc, tID)))
				pEnc->Serialize(&cOff, pCfg, &cDst, a_pLocation, a_pControl);

			if (cDst.size())
			{
				if (tContentOffset.nX || tContentOffset.nY)
				{
					if (FAILED(a_pDst->Write(sizeof(MARK_OFFSET), reinterpret_cast<BYTE const*>(&MARK_OFFSET))))
						return E_FAIL;
					ULONG nLen = sizeof tContentOffset;
					if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
						return E_FAIL;
					if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&tContentOffset))))
						return E_FAIL;
				}
				if (FAILED(a_pDst->Write(sizeof(MARK_PIXELS), reinterpret_cast<BYTE const*>(&MARK_PIXELS))))
					return E_FAIL;
				ULONG nLen = cDst.size();
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(cDst.size(), cDst.begin())))
					return E_FAIL;
				if ((nLen&3) && FAILED(a_pDst->Write(4-(nLen&3), DWORD_PADDING)))
					return E_FAIL;
			}
		}

		return S_OK;
	}
	catch (...)
	{
		return E_NOTIMPL;
	}
}


class ATL_NO_VTABLE COffsetedImageBuilder : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentFactoryRasterImage,
	public IDocumentBuilder
{
public:
	void Init(IDocumentFactoryRasterImage* a_pOrig, TImageSize a_tSize, TImageResolution a_tResolution, TImagePoint a_tOffset, float a_fGamma, TPixelChannel a_tDefault)
	{
		m_pOrig = a_pOrig;
		m_tSize = a_tSize;
		m_tResolution = a_tResolution;
		m_tOffset = a_tOffset;
		m_fGamma = a_fGamma;
		m_tDefault = a_tDefault;
	}


BEGIN_COM_MAP(COffsetedImageBuilder)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
END_COM_MAP()

	// IDocumentFactoryRasterImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
	{
		TImageTile tTile;
		if (a_pTile)
		{
			tTile = *a_pTile;
			tTile.tOrigin = m_tOffset;
		}
		return m_pOrig->Create(a_bstrPrefix, a_pBase, &m_tSize, &m_tResolution, 1, CChannelDefault(EICIRGBA, m_tDefault), m_fGamma, a_pTile ? &tTile : NULL);
	}

	// IDocumentBuilder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority) { *a_pnPriority = EDPAverage; return S_OK; }
	STDMETHOD(TypeName)(ILocalizedString** a_ppType) { return E_NOTIMPL; }
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon) { return E_NOTIMPL; }
	STDMETHOD(FormatInfo)(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon) { return E_NOTIMPL; }
	STDMETHOD(HasFeatures)(ULONG a_nCount, IID const* a_aiidRequired) { return E_NOTIMPL; }

private:
	CComPtr<IDocumentFactoryRasterImage> m_pOrig;
	TImageSize m_tSize;
	TImageResolution m_tResolution;
	TImagePoint m_tOffset;
	float m_fGamma;
	TPixelChannel m_tDefault;
};

// old format
BYTE const RRI_HEADER_TRANSPARENT[] = "RWRasterImage_1";
BYTE const RRI_HEADER_OPAQUE[] = "RWRasterImage_O";
struct TRasterImageCoord { ULONG n0, n1, n2, n3; };

HRESULT CDocumentCodecRRI::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
{
	try
	{
		if (a_nLen >= (sizeof(RRI_HEADER_TRANSPARENT)+sizeof(TRasterImageCoord)))
		{
			bool bHasAlpha = memcmp(RRI_HEADER_TRANSPARENT, a_pData, sizeof(RRI_HEADER_TRANSPARENT)) == 0;
			bool bWithoutAlpha = memcmp(RRI_HEADER_OPAQUE, a_pData, sizeof(RRI_HEADER_OPAQUE)) == 0;
			if (bHasAlpha || bWithoutAlpha)
			{
				TRasterImageCoord tSize = *reinterpret_cast<TRasterImageCoord const*>(a_pData+sizeof(RRI_HEADER_TRANSPARENT));
				ULONG nPixels = tSize.n0*tSize.n1*tSize.n2*tSize.n3;
				if (nPixels*sizeof(TRasterImagePixel) != a_nLen-(sizeof(RRI_HEADER_TRANSPARENT)+sizeof(TRasterImageCoord)))
					return E_RW_LOADFILEFAILED;
				CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[nPixels]);
				TPixelChannel const* pS = reinterpret_cast<TPixelChannel const*>(a_pData+sizeof(RRI_HEADER_TRANSPARENT)+sizeof(TRasterImageCoord));
				TPixelChannel* pD = cBuffer;
				for (TPixelChannel const* pEnd = pS+nPixels; pS < pEnd; ++pS, ++pD)
				{
					pD->bB = pS->bR;
					pD->bG = pS->bG;
					pD->bR = pS->bB;
					pD->bA = pS->bA;
				}
				return a_pBuilder->Create(a_bstrPrefix, a_pBase, CImageSize(tSize.n0, tSize.n1), NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(tSize.n0, tSize.n1, cBuffer));
			}
		}

		if (a_nLen < 16 || memcmp(a_pData, RRI_HEADER, sizeof RRI_HEADER))
			return E_RW_UNKNOWNINPUTFORMAT;

		bool bCreated = false;
		TImageSize tSize = {256, 256};
		TImageResolution tRes = {100, 254, 100, 254};
		TImagePoint tOffset = {0, 0};
		ULONG nChannels = EICIRGBA;
		float fGamma = 0.0f;
		TPixelChannel tDefault;
		tDefault.n = 0;
		BYTE const* pDoc = NULL;
		ULONG nDocLen = 0;

		BYTE const* const pBegin = a_pData;
		BYTE const* const pEnd = a_pData+a_nLen;
		BYTE const* pData = pBegin+sizeof(RRI_HEADER);
		while (pEnd-pData > 8)
		{
			if (pData+8+*reinterpret_cast<DWORD const*>(pData+4) > pEnd)
				break; // incomplete block
			if (MARK_SIZE == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= sizeof tSize)
			{
				tSize = *reinterpret_cast<TImageSize const*>(pData+8);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_RESOLUTION == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= sizeof tRes)
			{
				tRes = *reinterpret_cast<TImageResolution const*>(pData+8);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_OFFSET == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= sizeof tOffset)
			{
				CopyMemory(&tOffset, pData+8, sizeof tOffset);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_BACKGROUND == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= 12)
			{
				fGamma = *reinterpret_cast<float const*>(pData+12);
				if (*reinterpret_cast<ULONG const*>(pData+8) == EICIRGBA &&
					*reinterpret_cast<ULONG const*>(pData+16) >= 1 && *reinterpret_cast<DWORD const*>(pData+4) >= 12+*reinterpret_cast<ULONG const*>(pData+16)*sizeof(TChannelDefault))
				{
					nChannels = *reinterpret_cast<ULONG const*>(pData+8);
					TChannelDefault const* pChDef = reinterpret_cast<TChannelDefault const*>(pData+20);
					for (ULONG i = 0; i < *reinterpret_cast<ULONG const*>(pData+16); ++i)
						if (pChDef[i].eID == EICIRGBA)
							tDefault = pChDef[i].tValue;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_PIXELS == *reinterpret_cast<DWORD const*>(pData))
			{
				pDoc = pData+8;
				nDocLen = *reinterpret_cast<DWORD const*>(pData+4);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}

			// skip unknown block
			pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
		}
		HRESULT hRes = E_FAIL;
		if (pDoc)
		{
			CComObjectStackEx<COffsetedImageBuilder> cBuilder;
			cBuilder.Init(a_pBuilder, tSize, tRes, tOffset, fGamma, tDefault);
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			hRes = pIM->DocumentCreateDataEx(static_cast<IDocumentBuilder*>(&cBuilder), nDocLen, pDoc, a_pLocation, a_bstrPrefix, a_pBase, NULL, NULL, a_pControl);
		}
		else
		{
			hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tRes, 1, CChannelDefault(EICIRGBA, tDefault), fGamma, NULL);
		}
		if (FAILED(hRes)) return hRes;
		if (a_pEncoderID)
			*a_pEncoderID = __uuidof(DocumentCodecRRI);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
