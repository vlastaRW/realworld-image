// DocumentRasterImage.h : Declaration of the CDocumentRasterImage

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentImageRaster.h"
#include <SubjectImpl.h>
#include "DocumentRasterImageUndo.h"
#include <DocumentUndoImpl.h>
#include "ImageMetaDataImpl.h"
#include <MultiLanguageString.h>
#include <IconRenderer.h>


extern __declspec(selectany) IID const g_aSupportedRaster[] =
{
	__uuidof(IDocumentImage),
	__uuidof(IDocumentEditableImage),
	__uuidof(IDocumentRasterImage),
	__uuidof(IRasterImageComposedPreview),
	__uuidof(IImageMetaData),
};
extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsRaster[] = L"rri";
extern __declspec(selectany) OLECHAR const g_pszShellIconPathRaster[] = L"%MODULE%,0";

extern __declspec(selectany) wchar_t const DESWIZ_IMAGE_CAT[] = L"[0409]Images[0405]Obrázky";


TImageSize const TIMGSIZE_NULL = {0, 0};
TImagePoint const TIMGPOINT_NULL = {0, 0};
//TRasterImageRect const TRECT_NULL = {{0, 0, 0, 0}, {0, 0, 0, 0}};


// CDocumentRasterImage

class ATL_NO_VTABLE CDocumentRasterImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentRasterImage, &CLSID_DocumentFactoryRasterImage, EUMMemoryLimited>,
	public CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>,
	public CSubjectImpl<IRasterImageComposedPreview, IComposedPreviewObserver, ULONG>,
	public CImageMetaDataImpl<CDocumentRasterImage>,
	public ILayerType
{
public:
	CDocumentRasterImage() : m_pData(NULL), m_pfnDeleter(DefDeleter), m_tSize(TIMGSIZE_NULL), m_qwAlphaTotal(0),
		m_bResized(false), m_bChannelsChanged(false), m_tDirtyTL(TIMGPOINT_NULL), m_tDirtyBR(TIMGPOINT_NULL), m_bPrevAlphaState(false),
		m_bResolutionChange(false), m_bDefaultChange(false), m_bNoUndo(false),
		m_ENCFEAT_IMAGE(ENCFEAT_IMAGE), m_ENCFEAT_IMAGE_META(ENCFEAT_IMAGE_META), m_ENCFEAT_IMAGE_ALPHA(ENCFEAT_IMAGE_ALPHA)
	{
		m_tResolution.nNumeratorX = m_tResolution.nNumeratorY = 100;
		m_tResolution.nDenominatorX = m_tResolution.nDenominatorY = 254;
		m_tDefault.n = 0;
		RWCoCreateInstance(m_pThPool, __uuidof(ThreadPool));
	}
	~CDocumentRasterImage()
	{
		m_pfnDeleter(m_pData);
	}
	void Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault);
	void Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault, TImagePoint const& a_tContentOrigin, TImageSize const& a_tContentSize, CAutoVectorPtr<TPixelChannel>& a_pPixels, float a_fGamma, IImageMetaData* a_pMetaData, ULONGLONG const* a_pTotalAlpha = NULL, ULONG a_nPreviewScale = 0);
	void Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault, TImagePoint const& a_tContentOrigin, TImageSize const& a_tContentSize, TPixelChannel const* a_pPixels, float a_fGamma, IImageMetaData* a_pMetaData, ULONG const* a_pStride = NULL);
	void InitFinish(TPixelChannel a_tDefault) { m_tDefault = a_tDefault; }

	TImagePoint M_ContentOrigin() { return m_tAllocOrigin; }
	TImageSize M_ContentSize() const { return m_tAllocSize; }
	TPixelChannel M_Default() const { return m_tDefault; }
	TPixelChannel* Detach() { TPixelChannel* p = m_pData; m_pData = NULL; return p; }
	void OptimizeContent(bool a_bForce = false);

BEGIN_COM_MAP(CDocumentRasterImage)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentRasterImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IRasterImageComposedPreview)
	COM_INTERFACE_ENTRY(IImageMetaData)
	COM_INTERFACE_ENTRY(ILayerType)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		InitMetaData();
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	STDMETHOD(WriteFinished)()
	{
		bool bPrevAlphaState = m_bPrevAlphaState;
		bool bChannelsChanged = m_bChannelsChanged;
		m_bChannelsChanged = false;
		bool bDefaultChange = m_bDefaultChange;
		m_bDefaultChange = false;

		m_bPrevAlphaState = m_qwAlphaTotal != ULONGLONG(m_tContentSize.nX*m_tContentSize.nY)*ULONGLONG(255);
		if (bPrevAlphaState != m_bPrevAlphaState)
			bChannelsChanged = true;

		HRESULT hRes = S_FALSE;
		TImageChange tChg;
		tChg.nGlobalFlags = GetMetaDataChanges();
		tChg.tOrigin = m_tDirtyTL;
		tChg.tSize.nX = m_tDirtyBR.nX-m_tDirtyTL.nX;
		tChg.tSize.nY = m_tDirtyBR.nY-m_tDirtyTL.nY;
		if (bDefaultChange)
			tChg.nGlobalFlags |= EICContent;
		if (m_bResized)
		{
			tChg.nGlobalFlags |= (bChannelsChanged ? EICDimensions|EICContent|EICChannels : EICDimensions|EICContent);
			CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
			hRes = S_OK;
			M_Base()->SetDirty();
			M_Base()->UpdateQuickInfo();
		}
		else if (m_tDirtyTL.nX < m_tDirtyBR.nX && m_tDirtyTL.nY < m_tDirtyBR.nY)
		{
			tChg.nGlobalFlags |= (bChannelsChanged ? EICContent|EICChannels : EICContent);
			CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
			hRes = S_OK;
			M_Base()->SetDirty();
		}
		else if (bChannelsChanged)
		{
			tChg.nGlobalFlags |= EICContent|EICChannels;
			CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
			hRes = S_OK;
			M_Base()->SetDirty();
		}
		else if (tChg.nGlobalFlags)
		{
			CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
			hRes = S_OK;
			M_Base()->SetDirty();
		}
		else if (m_bResolutionChange)
		{
			tChg.nGlobalFlags |= EICResolution;
			CSubjectImpl<IDocumentRasterImage, IImageObserver, TImageChange>::Fire_Notify(tChg); // TODO: flag for resolution change?
			hRes = S_OK;
			M_Base()->SetDirty();
		}
		m_bResolutionChange = false;
		m_bResized = false;
		m_tDirtyTL = m_tDirtyBR = TIMGPOINT_NULL;
		if (bChannelsChanged/* || dwMetaDataChanges*/)
			M_Base()->RefreshEncoder();

		return hRes;
	}

	// IDocumentData methods
public:
	STDMETHOD(DataCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo);
	EUndoMode DefaultUndoModeHelper() { return m_bNoUndo ? EUMDisabled : EUMMemoryLimited; }
	STDMETHOD(MaximumUndoSize)(ULONGLONG* a_pnMaximumSize);
	STDMETHOD(ResourcesManage)(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue);
	STDMETHOD(Aspects)(IEnumEncoderAspects* a_pEnumAspects);

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity);
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults);
	STDMETHOD(TileGet)(ULONG a_nChannelID, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer);

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper);
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults);

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent);
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter);
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter);

	// IRasterImageComposedPreview methods
public:
	STDMETHOD(AdjustDirtyRect)(RECT* a_prc) { return S_FALSE; }
	STDMETHOD(PreProcessTile)(RECT* a_prc) { return S_FALSE; }
	STDMETHOD(ProcessTile)(EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData);
	STDMETHOD(InputTransform)(TMatrix3x3f*) { return S_FALSE; }

	// ILayerType methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppLayerType)
	{
		try
		{
			*a_ppLayerType = NULL;
			*a_ppLayerType = new CMultiLanguageString(L"[0409]Raster layer[0405]Rastrová vrstva");
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(IconID)(GUID* a_pIconID)
	{
		if (a_pIconID == NULL)
			return E_POINTER;
		// {A376A369-5BB8-4f60-8274-FE577FDE2B0D}
		static GUID const tID = {0xa376a369, 0x5bb8, 0x4f60, {0x82, 0x74, 0xfe, 0x57, 0x7f, 0xde, 0x2b, 0xd}};
		*a_pIconID = tID;
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		*a_phIcon = LayerIcon(a_nSize);
		return S_OK;
	}

	static HICON LayerIcon(ULONG a_nSize, IRTarget const* a_pTarget = NULL);

private:
	void AddDirtyRect(TImagePoint const& a_tOrigin, TImageSize const& a_tSize);
	static void CopyRectangle(TPixelChannel* a_pDst, ULONG const a_nDstStrideY, TPixelChannel const* a_pSrc, ULONG const a_nSrcSizeX, ULONG const a_nSrcSizeY, ULONG const a_nSrcStrideX, ULONG const a_nSrcStrideY);
	static void DefDeleter(TPixelChannel* p) { delete[] p; }

private:
	TImageSize m_tSize;
	TImagePoint m_tAllocOrigin;
	TImageSize m_tAllocSize;
	TImagePoint m_tContentOrigin;
	TImageSize m_tContentSize;
	TPixelChannel m_tDefault;
	TPixelChannel* m_pData;
	fnDeleteBuffer m_pfnDeleter;
	float m_fGamma;
	bool m_bPrevAlphaState;
	ULONGLONG m_qwAlphaTotal;
	TImageResolution m_tResolution;

	bool m_bResized;
	bool m_bChannelsChanged;
	TImagePoint m_tDirtyTL;
	TImagePoint m_tDirtyBR;
	bool m_bResolutionChange;
	bool m_bDefaultChange;

	bool m_bNoUndo;

	CComBSTR m_ENCFEAT_IMAGE;
	CComBSTR m_ENCFEAT_IMAGE_META;
	CComBSTR m_ENCFEAT_IMAGE_ALPHA;

	CComPtr<IThreadPool> m_pThPool;
};

