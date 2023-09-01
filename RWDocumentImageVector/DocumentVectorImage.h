// DocumentVectorImage.h : Declaration of the CDocumentVectorImage

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentImageVector.h"
#include <RWConceptStructuredData.h>
#include <SubjectImpl.h>
//#include "DocumentVectorImageUndo.h"
#include <DocumentUndoImpl.h>
#include <MultiLanguageString.h>
#include <IconRenderer.h>


extern __declspec(selectany) IID const g_aSupportedVector[] =
{
	__uuidof(IDocumentImage),
	__uuidof(IDocumentVectorImage),
//	__uuidof(IRasterImageControl),
//	__uuidof(IRasterImageComposedPreview),
//	__uuidof(IImageMetaData),
};


// CDocumentVectorImage

class ATL_NO_VTABLE CDocumentVectorImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentVectorImage, &__uuidof(DocumentFactoryVectorImage), EUMMemoryLimited>,
	public CSubjectImpl<IDocumentVectorImage, IVectorImageObserver, TVectorImageChanges>,
	public CSubjectImpl<IDocumentEditableImage, IImageObserver, TImageChange>,
	public CSubjectImpl<CStructuredRootImpl<CDocumentVectorImage, IStructuredRoot>, IStructuredObserver, TStructuredChanges>,
	public IStructuredItemsRichGUI,
	public ILayerType
{
public:
	CDocumentVectorImage() : m_nNextID(1), m_bBuffer(false), m_nBuffer(0),
		m_bChangeSize(false), m_bChangeBackground(false), m_bChangeElements(false),
		m_bContent(false), m_dwTimeStamp(0), m_nClipboardFormat(0)
	{
		m_tCanvas.nX = m_tCanvas.nY = 256;
		m_aBackground[0] = m_aBackground[1] = m_aBackground[2] = m_aBackground[3] = 0.0f;
		m_tResolution.nNumeratorX = m_tResolution.nNumeratorY = 100;
		m_tResolution.nDenominatorX = m_tResolution.nDenominatorY = 254;
		m_rcDirty.left = m_rcDirty.top = LONG_MAX;
		m_rcDirty.right = m_rcDirty.bottom = LONG_MIN;
	}
	~CDocumentVectorImage()
	{
	}
	void Init(ULONG a_nSizeX, ULONG a_nSizeY, TImageResolution const* a_pResolution, float const* a_aBackground)
	{
		m_tCanvas.nX = a_nSizeX;
		m_tCanvas.nY = a_nSizeY;
		if (a_pResolution)
			m_tResolution = *a_pResolution;
		m_aBackground[0] = a_aBackground[0];
		m_aBackground[1] = a_aBackground[1];
		m_aBackground[2] = a_aBackground[2];
		m_aBackground[3] = a_aBackground[3];
	}

BEGIN_COM_MAP(CDocumentVectorImage)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentVectorImage)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IStructuredRoot)
	COM_INTERFACE_ENTRY(IStructuredItemsRichGUI)
	COM_INTERFACE_ENTRY(ILayerType)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		RWCoCreateInstance(m_pToolMgr, __uuidof(RasterImageEditToolsManager));
		RWCoCreateInstance(m_pFillMgr, __uuidof(RasterImageFillStyleManager));
		CComObject<CToolWindow>::CreateInstance(&m_pToolWindow.p);
		if (m_pToolWindow.p == NULL)
			return E_FAIL;
		m_pToolWindow.p->AddRef();
		m_pToolWindow->Init(this);
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pToolWindow.p)
			m_pToolWindow->Init(NULL);
	}


	// IDocumentData methods
public:
	STDMETHOD(DataCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo);
	//EUndoMode DefaultUndoModeHelper() { return m_nPreviewScale ? EUMDisabled : EUMMemoryLimited; }
	STDMETHOD(MaximumUndoSize)(ULONGLONG* a_pnMaximumSize);
	STDMETHOD(ResourcesManage)(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue);
	STDMETHOD(Aspects)(IEnumEncoderAspects* a_pEnumAspects);
	STDMETHOD(WriteFinished)();

	// IDocumentVectorImage methods
public:
	STDMETHOD(ObjectIDs)(IEnum2UInts* a_pIDs);
	STDMETHOD(ObjectGet)(ULONG a_nID, BSTR* a_pToolID, BSTR* a_pToolParams);
	STDMETHOD(ObjectSet)(ULONG* a_pID, BSTR a_bstrToolID, BSTR a_bstrToolParams);
	STDMETHOD(ObjectNameGet)(ULONG a_nID, BSTR* a_pName);
	STDMETHOD(ObjectNameSet)(ULONG a_nID, BSTR a_bstrName);
	STDMETHOD(ObjectStyleGet)(ULONG a_nID, BSTR* a_pStyleID, BSTR* a_pStyleParams);
	STDMETHOD(ObjectStyleSet)(ULONG a_nID, BSTR a_bstrStyleID, BSTR a_bstrStyleParams);
	STDMETHOD(ObjectStateGet)(ULONG a_nID, BOOL* a_pFill, ERasterizationMode* a_pRasterizationMode, ECoordinatesMode* a_pCoordinatesMode, BOOL* a_pOutline, TColor* a_pOutlineColor, float* a_pOutlineWidth, float* a_pOutlinePos, EOutlineJoinType* a_pOutlineJoins);
	STDMETHOD(ObjectStateSet)(ULONG a_nID, BOOL const* a_pFill, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, BOOL const* a_pOutline, TColor const* a_pOutlineColor, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins);
	STDMETHOD(ObjectEffectGet)(ULONG a_nID, IConfig** a_ppOperation);
	STDMETHOD(ObjectEffectSet)(ULONG a_nID, IConfig* a_pOperation);
	STDMETHOD(ObjectIsEnabled)(ULONG a_nID);
	STDMETHOD(ObjectEnable)(ULONG a_nID, BOOL a_bEnabled);
	STDMETHOD(ObjectsMove)(ULONG a_nCount, ULONG const* a_aShapeIDs, ULONG a_nUnder);
	STDMETHOD(ObjectTransform)(ULONG a_nID, TMatrix3x3f const* a_pContentTransform);
	STDMETHOD(ObjectBounds)(ULONG a_nID, TPixelCoords* a_pTL, TPixelCoords* a_pBR);

	STDMETHOD(StatePrefix)(BSTR* a_pbstrPrefix);
	STDMETHOD(StatePack)(ULONG a_nIDs, ULONG const* a_pIDs, ISharedState** a_ppState);
	STDMETHOD(StateUnpack)(ISharedState* a_pState, IEnum2UInts* a_pIDs);

	HRESULT ObjectRestore(ULONG a_nID, ULONG a_nIndex, CComBSTR& a_bstrName, CComBSTR& a_bstrToolID, CComBSTR& a_bstrToolParams,
		CComBSTR& a_bstrStyleID, CComBSTR& a_bstrStyleParams, BOOL a_bFill, BOOL a_bOutline, TColor const& a_tColor, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins,
		ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode);
	HRESULT ObjectsReorder(std::vector<ULONG> const& a_cShapeIDs, RECT a_rcDirty);

	// IStructuredRoot methods
public:
	STDMETHOD(StatePack)(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState);
	STDMETHOD(StateUnpack)(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems);
	//STDMETHOD(StatePrefix)(BSTR *a_pbstrPrefix);
	STDMETHOD(ItemsEnum)(IComparable* a_pItem, IEnumUnknowns** a_ppSubItems);

	// IStructuredItemsRichGUI methods
public:
	STDMETHOD(Begin)(IEnumUnknowns* a_pSelection, IDataObject** a_ppDataObject, IDropSource** a_ppDropSource, DWORD* a_pOKEffects);
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, LCID a_tLocaleID, ISharedState** a_ppNewSel);

	STDMETHOD(ClipboardPriority)(BYTE* a_pPrio) { *a_pPrio = 128; return S_OK; }
	STDMETHOD(ClipboardName)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ILocalizedString** a_ppName);
	STDMETHOD(ClipboardIconID)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, GUID* a_pIconID);
	STDMETHOD(ClipboardIcon)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay);
	STDMETHOD(ClipboardCheck)(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ISharedState* a_pState);
	STDMETHOD(ClipboardRun)(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ISharedState* a_pState, ISharedState** a_ppNewState);

	STDMETHOD(Thumbnail)(IComparable* a_pItem, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, ULONG* a_pTimestamp);

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity);
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults);
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent);
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer);

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper);
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults);

	// ILayerType methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppLayerType)
	{
		try
		{
			*a_ppLayerType = NULL;
			*a_ppLayerType = new CMultiLanguageString(L"[0409]Vector layer[0405]Vektorová vrstva");
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
		// {EBAADA1D-15C1-4021-9519-88B9F575DA29}
		static GUID const tID = {0xebaada1d, 0x15c1, 0x4021, {0x95, 0x19, 0x88, 0xb9, 0xf5, 0x75, 0xda, 0x29}};
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

public:
	IRasterImageEditToolsManager* M_ToolMgr() const { return m_pToolMgr; }
	void Render(TImagePoint const& a_tOrigin, TImageSize const& a_tSize, TPixelChannel* a_pBuffer);
	void UpdateContentSize();
	void UpdateContentCache();
	void InvalidateRectangle(RECT const* a_pChanged);

	IOperationManager* M_OpMgr()
	{
		if (m_pOpMgr) return m_pOpMgr;
		ObjectLock cLock(this);
		if (m_pOpMgr) return m_pOpMgr;
		RWCoCreateInstance(m_pOpMgr, __uuidof(OperationManager));
		return m_pOpMgr;
	}

private:
	class ATL_NO_VTABLE CToolWindow : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IRasterImageEditWindow
	{
	public:
		void Init(CDocumentVectorImage* a_pOwner)
		{
			m_pOwner = a_pOwner;
		}
		void SetROI(TImagePoint const& a_tOrigin, TImageSize const& a_tSize, TPixelChannel* a_pBuffer, TPixelChannel a_tDefault, TImageSize const& a_tContentSize)
		{
			m_tOrigin = a_tOrigin;
			m_tSize = a_tSize;
			m_pBuffer = a_pBuffer;
			m_tDefault = a_tDefault;
			m_tContentSize = a_tContentSize;
		}

	BEGIN_COM_MAP(CToolWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditWindow)
	END_COM_MAP()

		// IRasterImageEditWindow methods
	public:
		STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY) { *a_pSizeX = m_tContentSize.nX; *a_pSizeY = m_tContentSize.nY; return S_OK; }
		STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault) { *a_pDefault = *reinterpret_cast<TRasterImagePixel const*>(&m_tDefault); return S_OK; }
		STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer);
		STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle);
		STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer) { return E_NOTIMPL; }
		STDMETHOD(ControlPointsChanged)() { return S_FALSE; }
		STDMETHOD(ControlPointChanged)(ULONG UNREF(a_nIndex)) { return S_FALSE; }
		STDMETHOD(ControlLinesChanged)() { return S_FALSE; }
		STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
		{
			if (m_pOwner)
				m_pOwner->InvalidateRectangle(a_pChanged);
			return S_OK;
		}
		STDMETHOD(ScrollWindow)(ULONG UNREF(a_nScrollID), TPixelCoords const* UNREF(a_pDelta)) { return E_NOTIMPL; }
		STDMETHOD(ApplyChanges)() { return S_FALSE;} 
		STDMETHOD(SetState)(ISharedState* a_pState) { return E_NOTIMPL; }
		STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState) { return E_NOTIMPL; }
		STDMETHOD(Handle)(RWHWND* a_phWnd) { return E_NOTIMPL; }
		STDMETHOD(Document)(IDocument** a_ppDocument) { ATLASSERT(0); return E_NOTIMPL; }
		STDMETHOD(Checkpoint)() { return E_NOTIMPL; }

	private:
		CDocumentVectorImage* m_pOwner;
		TImagePoint m_tOrigin;
		TImageSize m_tSize;
		TPixelChannel* m_pBuffer;
		TPixelChannel m_tDefault;
		TImageSize m_tContentSize;
	};

	struct SElement
	{
		SElement() : eRasterizationMode(ERMSmooth), eCoordinatesMode(ECMFloatingPoint), bFill(TRUE), bOutline(FALSE), fOutlineWidth(1.0f), fOutlinePos(0.0f), eOutlineJoins(EOJTRound), bEnabled(true),
			bChangeName(false), bChangeToolID(false), bChangeParams(false), bChangeStyle(false), bChangeOutline(false), bChangeModes(false), bChangeEffect(false), bChangeVisibility(0), dwTimeStamp(0)
		{
			tOutlineColor.fR = tOutlineColor.fG = tOutlineColor.fB = 0.0f;
			tOutlineColor.fA = 1.0f;
		}
		CComBSTR bstrName;
		bool bEnabled;

		CComBSTR bstrToolID;
		CComBSTR bstrToolParams;
		CComPtr<IRasterImageEditTool> pTool;

		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;
		CComPtr<IRasterImageBrush> pFill;

		CComPtr<IConfig> pEffect;

		BOOL bFill;
		BOOL bOutline;
		TColor tOutlineColor;
		float fOutlineWidth;
		float fOutlinePos;
		EOutlineJoinType eOutlineJoins;
		ERasterizationMode eRasterizationMode;
		ECoordinatesMode eCoordinatesMode;

		bool bChangeName;
		bool bChangeToolID;
		bool bChangeParams;
		bool bChangeStyle;
		bool bChangeOutline;
		bool bChangeModes;
		bool bChangeEffect;
		bool bChangeVisibility;
		DWORD dwTimeStamp;
	};
	typedef std::map<ULONG, SElement> CElements;
	typedef std::vector<ULONG> CElementOrder;

	HRESULT GetDirtyRect(SElement const& sEl, RECT& rc);

private:
	CComPtr<IRasterImageEditToolsManager> m_pToolMgr;
	CComPtr<IRasterImageFillStyleManager> m_pFillMgr;
	CComPtr<CComObject<CToolWindow> > m_pToolWindow;

	TImageSize m_tCanvas;
	float m_aBackground[4];
	TImageResolution m_tResolution;

	ULONG m_nNextID;
	CElements m_cElements;
	CElementOrder m_cElementOrder;

	bool m_bChangeSize;
	bool m_bChangeBackground;
	bool m_bChangeElements;
	RECT m_rcDirty;
	DWORD m_dwTimeStamp;

	CAutoVectorPtr<TPixelChannel> m_pBuffer;
	ULONG m_nBuffer;
	bool m_bBuffer;
	RECT m_rcContent;
	bool m_bContent;

	CComBSTR m_bstrPreview; // workaround for StatePrefixGet (remove this when states are redesigned)

	UINT m_nClipboardFormat;

	CComPtr<IOperationManager> m_pOpMgr;
};

