// DocumentLayeredImage.h : Declaration of the CDocumentLayeredImage

#pragma once
#include "RWDocumentImageRaster.h"
#include <SubjectImpl.h>
#include <ObserverImpl.h>
#include "ImageMetaDataImpl.h"
#include <MultiLanguageString.h>
#include <GammaCorrection.h>
#include <IconRenderer.h>


extern __declspec(selectany) IID const g_aSupportedLayered[] =
{
	__uuidof(IDocumentImage),
	__uuidof(IDocumentLayeredImage),
	__uuidof(IDocumentEditableImage),
	__uuidof(ISubDocumentsMgr),
	__uuidof(IImageMetaData),
};

static TRasterImageRect const INVALIDRECT = {{LONG_MAX, LONG_MAX}, {LONG_MIN, LONG_MIN}};
class CStructuredItemLayerEffect;

// CDocumentLayeredImage

class ATL_NO_VTABLE CDocumentLayeredImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentLayeredImage, &CLSID_DocumentFactoryLayeredImage, EUMMemoryLimited>,
	public CSubjectImpl<CStructuredRootImpl<CDocumentLayeredImage, IDocumentLayeredImage>, IStructuredObserver, TStructuredChanges>,
	public CSubjectImpl<IDocumentEditableImage, IImageObserver, TImageChange>,
	public IStructuredItemsRichGUI,
	public CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>,
	public CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>,
	public CImageMetaDataImpl<CDocumentLayeredImage>,
	public ILayerType
{
public:
	CDocumentLayeredImage() : m_nNextID(0), 
		m_bSizeChange(false), m_bNonImageChange(false), m_bLayersChange(false),
		m_nClipboardLayers(0), m_nClipboardEffects(0),
#ifdef _DEBUG
		m_iComposedActNotifying(static_cast<size_t>(-1)), m_iComposedMaxNotifying(0),
#endif
		/*m_fCachedGamma(1.0f),*/ m_bPrevLayers(false), m_bPrevLayerEffects(false), m_bPrevMetaData(false),
		m_ENCFEAT_IMAGE(ENCFEAT_IMAGE), m_ENCFEAT_IMAGE_META(ENCFEAT_IMAGE_META), m_ENCFEAT_IMAGE_ALPHA(ENCFEAT_IMAGE_ALPHA),
		m_ENCFEAT_IMAGE_LAYER(ENCFEAT_IMAGE_LAYER), m_ENCFEAT_IMAGE_LAYER_EFFECT(ENCFEAT_IMAGE_LAYER_EFFECT),
		m_ENCFEAT_IMAGE_CANVAS(ENCFEAT_IMAGE_CANVAS), m_ENCFEAT_IMAGE_LAYER_SPECIAL(ENCFEAT_IMAGE_LAYER_SPECIAL)
	{
		m_tSize.nX = m_tSize.nY = 1;
		m_tResolution.nNumeratorX = m_tResolution.nNumeratorY = 100;
		m_tResolution.nDenominatorX = m_tResolution.nDenominatorY = 254;
		//UpdateGammaTables();
		//RWCoCreateInstance(m_pOpMgr, __uuidof(OperationManager));
		RWCoCreateInstance(m_pThPool, __uuidof(ThreadPool));

	}
	~CDocumentLayeredImage();
	void Init(TImageResolution const* a_pRes)
	{
		if (a_pRes) m_tResolution = *a_pRes;
	}
	HRESULT InitLayer(IDocumentData* a_pBlock, LCID a_tLocaleID, TImageSize const& a_tCanvasSize);


	static HRESULT WINAPI QISingleLayer(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CDocumentLayeredImage* pThis = reinterpret_cast<CDocumentLayeredImage*>(a_pThis);
		CDocumentReadLock cLock(pThis);
		if (pThis->m_cLayers.size() == 1)
		{
			CComBSTR bstr;
			pThis->m_cLayers[0].GetLayerID(pThis, bstr);
			return pThis->M_Base()->DataBlockGet(bstr, a_iid, a_ppv);
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}

	enum { ECSLayerMeta = 0x10000 };

BEGIN_COM_MAP(CDocumentLayeredImage)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentLayeredImage)
	COM_INTERFACE_ENTRY(IStructuredRoot)
	COM_INTERFACE_ENTRY(ISubDocumentsMgr)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IStructuredItemsRichGUI)
	COM_INTERFACE_ENTRY(IImageMetaData)
	COM_INTERFACE_ENTRY(ILayerType)
	COM_INTERFACE_ENTRY_FUNC_BLIND(0, QISingleLayer)
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

	//void WriteFinished();
	void OwnerNotify(TCookie a_tCookie, TImageChange a_nParam);
	void OwnerNotify(TCookie a_tCookie, TStructuredChanges a_tParam);

	// IDocumentData methods
public:
	STDMETHOD(WriteFinished)();

	STDMETHOD(DataCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo);
	//STDMETHOD(LocationChanged)(IStorageFilter* a_pOldLoc);
	STDMETHOD(ComponentFeatureOverride)(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface);
	STDMETHOD(RemovingBlock)();
	EUndoMode DefaultUndoModeHelper() { return /*m_nPreviewScale ? EUMDisabled : */EUMMemoryLimited; }
	STDMETHOD(MaximumUndoSize)(ULONGLONG* a_pnMaximumSize);
	STDMETHOD(ResourcesManage)(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue);
	STDMETHOD(Aspects)(IEnumEncoderAspects* a_pEnumAspects);

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

	// IStructuredRoot mehods
public:
	STDMETHOD(StatePack)(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState);
	STDMETHOD(StateUnpack)(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems);
	STDMETHOD(ItemsEnum)(IComparable* a_pRoot, IEnumUnknowns** a_ppSubItems);

	// ISubDocumentsMgr methods
public:
	STDMETHOD(FindByName)(BSTR a_bstrName, ISubDocumentID** a_ppItem);

	// IDocumentLayeredImage methods
public:
	STDMETHOD(LayersEnum)(IComparable* a_pRoot, IEnumUnknowns** a_ppLayers);
	STDMETHOD(LayerInsert)(IComparable* a_pWhere, ELayerInsertPosition a_ePosition, IImageLayerCreator* a_pCreator, IComparable** a_ppNew);
	STDMETHOD(LayerMove)(IComparable* a_pItem, IComparable* a_pWhere, ELayerInsertPosition a_ePosition);
	STDMETHOD(LayerDelete)(IComparable* a_pItem);
	STDMETHOD(LayerReplace)(IComparable* a_pItem, IImageLayerCreator* a_pCreator);
	STDMETHOD(LayerNameGet)(IComparable* a_pItem, BSTR* a_pbstrName);
	STDMETHOD(LayerNameSet)(IComparable* a_pItem, BSTR a_bstrName);
	STDMETHOD(LayerPropsGet)(IComparable* a_pItem, ELayerBlend* a_pBlendingMode, BYTE* a_pVisible);
	STDMETHOD(LayerPropsSet)(IComparable* a_pItem, ELayerBlend const* a_pBlendingMode, BYTE const* a_pVisible);
	STDMETHOD(LayerEffectGet)(IComparable* a_pItem, IConfig** a_ppOperation, float* a_pOpacity);
	STDMETHOD(LayerEffectSet)(IComparable* a_pItem, IConfig* a_pOperation);
	STDMETHOD(LayerRender)(IComparable* a_pItem, BSTR a_bstrLayerWithEffectID, IDocumentBase* a_pLayerWithEffectBase);
	STDMETHOD(LayerEffectStepGet)(IComparable* a_pItem, BYTE* a_pEnabled, GUID* a_pOpID, IConfig** a_ppOpCfg);
	STDMETHOD(LayerEffectStepSet)(IComparable* a_pItem, BYTE* a_pEnabled, GUID* a_pOpID, IConfig* a_pOpCfg);
	STDMETHOD(LayerEffectStepAppend)(IComparable* a_pItem, BYTE a_bEnabled, REFGUID a_tOpID, IConfig* a_pOpCfg, IComparable** a_ppNew);
	STDMETHOD(LayerEffectStepDelete)(IComparable* a_pItem);
	STDMETHOD(IsLayer)(IComparable* a_pItem, ULONG* a_pLevel, INT_PTR* a_pParentHandle, GUID* a_pBuilder);
	STDMETHOD(LayerEffectsEnum)(IComparable* a_pLayer, IEnumUnknowns** a_ppEffects);
	STDMETHOD(LayerFromEffect)(IComparable* a_pEffect, IComparable** a_ppLayer);

	// IStructuredItemsRichGUI methods
public:
	STDMETHOD(Begin)(IEnumUnknowns* a_pSelection, IDataObject** a_ppDataObject, IDropSource** a_ppDropSource, DWORD* a_pOKEffects);
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, LCID a_tLocaleID, ISharedState** a_ppNewSel);

	STDMETHOD(ClipboardPriority)(BYTE* a_pPrio) { *a_pPrio = 128; return S_OK; }
	STDMETHOD(ClipboardName)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ILocalizedString** a_ppName);
	STDMETHOD(ClipboardIconID)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, GUID* a_pIconID);
	STDMETHOD(ClipboardIcon)(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay);
	HRESULT ClipboardCheck(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ULONG a_nLayerID, std::set<ULONG> a_cSelIDs);
	STDMETHOD(ClipboardCheck)(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ISharedState* a_pState);
	HRESULT ClipboardRun(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ULONG a_nLayerID, std::set<ULONG> a_cSelIDs, ISharedState** a_ppNewState);
	STDMETHOD(ClipboardRun)(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ISharedState* a_pState, ISharedState** a_ppNewState);
	void ClipboardSelectionHelper(ULONG a_nLayerID, std::set<ULONG> a_cSelIDs, std::set<ULONG>& a_cAll, std::set<ULONG>& a_cSel);
	bool ClipboardTypeIsLayer(ERichGUIClipboardAction a_eAction, ISharedState* a_pState);

	STDMETHOD(Thumbnail)(IComparable* a_pItem, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, ULONG* a_pTimestamp);

	// IRasterImageComposedPreview helper methods
public:
	HRESULT ComposedPreviewAdjustTile(ULONG a_nLayerID, bool a_bAdjustDirty, RECT* a_prc);
	HRESULT ComposedPreviewProcessTile(ULONG a_nLayerID, EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData);
	HRESULT ComposedPreviewInputTransform(ULONG a_nLayerID, TMatrix3x3f* a_pTransform);

	// ILayerType methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppLayerType)
	{
		try
		{
			*a_ppLayerType = NULL;
			*a_ppLayerType = new CMultiLanguageString(L"[0409]Layer group[0405]Skupina vrstev");
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
		// {7C8B0433-8B15-47F1-9669-211BF4A15115}
		static GUID const tID = {0x7c8b0433, 0x8b15, 0x47f1, {0x96, 0x69, 0x21, 0x1b, 0xf4, 0xa1, 0x51, 0x15}};
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

	// internal methods
public:
	struct SLayerEffect
	{
		SLayerEffect() : pItem(NULL), pOpCfg(NULL), bEnabled(true), tOpID(GUID_NULL), pOp(NULL) {};
		void CleanUp();
		void swap(SLayerEffect& rhs);
		GUID tOpID;
		IDocumentOperation* pOp;
		IConfig* pOpCfg;
		bool bEnabled;
		CStructuredItemLayerEffect* pItem;
	};
	struct CLayerEffects : public std::vector<SLayerEffect>
	{
		~CLayerEffects();
		void swap(CLayerEffects& rhs)
		{
			std::vector<SLayerEffect>::swap(rhs);
		}
	};

	struct SLayer
	{
		void Init(CDocumentLayeredImage* a_pThis, ULONG a_nUID, IOperationManager* a_pMgr);
		void Release(IImageObserver* a_pImageObserver, IStructuredObserver* a_pStructuredObserver);
		static void GetLayerID(CDocumentLayeredImage const* a_pDoc, ULONG a_nUID, CComBSTR& a_bstrLayerID)
		{
			OLECHAR szID[32];
			swprintf(szID, L"L%i;", a_nUID);
			a_bstrLayerID = a_pDoc->M_DataID();
			a_bstrLayerID += szID;
		}
		void GetLayerID(CDocumentLayeredImage const* a_pDoc, CComBSTR& a_bstrLayerID) const
		{
			GetLayerID(a_pDoc, nUID, a_bstrLayerID);
		}
		void swap(SLayer& rhs)
		{
			std::swap(nUID, rhs.nUID);
			std::swap(nTimestamp, rhs.nTimestamp);
			std::swap(pDocImg, rhs.pDocImg);
			std::swap(pDocGrp, rhs.pDocGrp);
			std::swap(strName, rhs.strName);
			std::swap(eBlendingMode, rhs.eBlendingMode);
			std::swap(bVisible, rhs.bVisible);
			std::swap(pEffects, rhs.pEffects);
			std::swap(pItem.p, rhs.pItem.p);
		}

	public:
		ULONG nUID;
		ULONG nTimestamp;
		IDocumentImage* pDocImg;
		IDocumentLayeredImage* pDocGrp;
		std::wstring strName;
		ELayerBlend eBlendingMode;
		bool bVisible;
		CLayerEffects* pEffects;
		CComPtr<IUIItem> pItem;
	};
	typedef std::vector<SLayer> CLayers;

	HRESULT LayerBuilderIDGet(ULONG a_nLayerID, GUID* a_pBuilderID);
	HRESULT LayerIconIDGet(ULONG a_nLayerID, GUID* a_pIconID);
	HRESULT LayerIconGet(ULONG a_nLayerID, ULONG a_nSize, HICON* a_phIcon);
	std::wstring const& M_LayerName(ULONG a_nID) const {return FindLayer(a_nID).strName;}
	HRESULT GetSubDocument(ULONG a_nID, IDocument** a_ppSubDocument) const;
	void GetLayerTypeName(ULONG a_nID, ILocalizedString** a_ppName);
	//HRESULT OpenFile(ULONG a_nID, CMemoryStorageObj* a_pDocData, IDocument* a_pDoc);
	static void DecodeStyle(IConfig* pOperation, CLayerEffects& aEffects);
	static void EncodeStyle(CLayerEffects::const_iterator first, CLayerEffects::const_iterator last, IConfig* pOperation);
	HRESULT LayerEffectGet(ULONG a_nLayerID, IConfig** a_ppOperation, float* a_pOpacity);
	HRESULT LayerEffectSet(ULONG a_nLayerID, CLayerEffects& a_cEffects, bool a_bSwap);
	SLayerEffect* LayerEffectStepGet(ULONG a_nLayerID, ULONG a_nEffectID);
	HRESULT LayerEffectStepSet(ULONG a_nLayerID, ULONG a_nEffectID, bool const* a_pEnabled, GUID const* a_pOpID, IConfig* a_pOpCfg);
	HRESULT LayerEffectStepInsert(ULONG a_nLayerID, ULONG a_nBeforeEffectID, BYTE a_bEnabled, REFGUID a_tOpID, IConfig* a_pOpCfg, IComparable** a_ppNew);
	HRESULT LayerEffectStepInsert(ULONG a_nLayerID, ULONG a_nBeforeEffectID, SLayerEffect* a_pLE, IComparable** a_ppNew);
	HRESULT LayerEffectStepDelete(ULONG a_nLayerID, ULONG a_nEffectID);
	HRESULT LayerEffectsReorder(ULONG a_nLayerID, std::vector<ULONG>& a_aOrder);
	HRESULT LayersReorder(std::vector<ULONG>& a_aOrder);
	static ULONG DeleteEmptyGroups(IDocumentLayeredImage* pDoc);
	static bool HasChildLayer(IDocumentLayeredImage* pDoc, IComparable* pRoot, IComparable* pItem);
	bool LayerEffectStepGet(ULONG a_nLayerID, ULONG a_nEffectID, bool* a_pEnabled, CLSID* a_pID, IConfig** a_ppConfig);
	HRESULT GetEffectSubDocument(ULONG a_nID, ULONG a_nStep, IDocument** a_ppSubDocument);
	LONG FindItemLevel(IComparable* a_pRoot, IComparable* a_pItem, LONG a_nRootLevel);

	HRESULT MoveEffects(ULONG nLayerID, ULONG nEffectIDs, ULONG const* aEffectIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel);
	HRESULT MoveLayers(ULONG nLayerIDs, ULONG const* aLayerIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel);
	HRESULT InsertLayers(CDocumentLayeredImage* pSrc, ULONG nLayerIDs, ULONG const* aLayerIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel);
	HRESULT DeleteLayers(ULONG nLayerIDs, ULONG const* aLayerIDs);

	HRESULT ComposedObserverIns(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie, ULONG a_nLayerID)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cComposedCS);
		m_aComposedObservers.push_back(SComposedObserver(a_pObserver, a_tCookie, a_nLayerID));

		return S_OK;
	}
	HRESULT ComposedObserverDel(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie, ULONG a_nLayerID)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cComposedCS);
		for (std::vector<SComposedObserver>::iterator i = m_aComposedObservers.begin(); i != m_aComposedObservers.end(); ++i)
		{
			// comparison via IUnknown omitted
			if (i->pObserver == a_pObserver && i->tCookie == a_tCookie && i->nLayerID == a_nLayerID)
			{
				if (static_cast<int>(m_iComposedActNotifying) >= static_cast<int>(i-m_aComposedObservers.begin())) // force signed comparison
				{
					--m_iComposedActNotifying;
				}
				m_aComposedObservers.erase(i);
				a_pObserver->Release();
				--m_iComposedMaxNotifying;
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}

	HRESULT EffectObserverIns(ILayerEffectObserver* a_pObserver, TCookie a_tCookie, ULONG a_nEffectID)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cEffectCS);
		m_aEffectObservers.push_back(SEffectObserver(a_pObserver, a_tCookie, a_nEffectID));

		return S_OK;
	}
	HRESULT EffectObserverDel(ILayerEffectObserver* a_pObserver, TCookie a_tCookie, ULONG a_nEffectID)
	{
		CComCritSecLock<CComAutoCriticalSection> cLock(m_cEffectCS);
		for (std::vector<SEffectObserver>::iterator i = m_aEffectObservers.begin(); i != m_aEffectObservers.end(); ++i)
		{
			// comparison via IUnknown omitted
			if (i->pObserver == a_pObserver && i->tCookie == a_tCookie && i->nEffectID == a_nEffectID)
			{
				if (static_cast<int>(m_iEffectActNotifying) >= static_cast<int>(i-m_aEffectObservers.begin())) // force signed comparison
				{
					--m_iEffectActNotifying;
				}
				m_aEffectObservers.erase(i);
				a_pObserver->Release();
				--m_iEffectMaxNotifying;
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}


private:
	struct SLayerChangeFlags
	{
		SLayerChangeFlags() : nChanges(0) {}
		ULONG nChanges;
	};
	typedef std::map<ULONG, SLayerChangeFlags> CLayerChanges;

	struct SCacheEntryPtr;
	struct SCacheEntry
	{
		void Init(ELayerBlend a_eBlend, IDocument* a_pDoc, IDocumentImage* a_pImage)
		{
			pDoc = a_pDoc;
			pImage = a_pImage;
			tInvalid = INVALIDRECT;
			eBlend = a_eBlend;
			nAge = 0;
			TImageSize tS = {0, 0};
			pImage->CanvasGet(NULL, NULL, &tBounds.tTL, &tS, NULL);
			tBounds.tBR.nX = tBounds.tTL.nX + tS.nX;
			tBounds.tBR.nY = tBounds.tTL.nY + tS.nY;
			pImage->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		}
		void Init(ELayerBlend a_eBlend, IDocument* a_pDoc)
		{
			CComPtr<IDocumentImage> pImage;
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
			Init(a_eBlend, a_pDoc, pImage);
		}
		CComPtr<IDocument> pDoc;
		CComPtr<IDocumentImage> pImage;
		TRasterImageRect tBounds;
		TRasterImageRect tInvalid;
		TPixelChannel tDefault;
		ELayerBlend eBlend;

		CComAutoCriticalSection tCS;
		mutable ULONG nAge;

		friend struct SCacheEntryPtr;

	private:
		LONG nRef;

		SCacheEntry() : nRef(1), tBounds(INVALIDRECT), tInvalid(INVALIDRECT) { }
		ULONG AddRef() { return InterlockedIncrement(&nRef); }
		ULONG Release() { LONG const n = InterlockedDecrement(&nRef); if (n == 0) delete this; return n; }
	};
	struct SCacheEntryPtr
	{
		SCacheEntryPtr() : p(NULL) {}
		~SCacheEntryPtr() { if (p) p->Release(); }
		SCacheEntryPtr(SCacheEntryPtr const& a_p) : p(a_p.p) { if (p) p->AddRef(); }
		SCacheEntryPtr& operator=(SCacheEntryPtr const& a_p)
		{
			if (a_p.p != p)
			{
				if (a_p.p) a_p.p->AddRef();
				if (p) p->Release();
				p = a_p.p;
			}
			return *this;
		}
		static SCacheEntryPtr Make()
		{
			SCacheEntry* p = new SCacheEntry;
			return SCacheEntryPtr(p);
		}
		SCacheEntry* operator->() const throw()
		{
			return p;
		}
		bool operator!() const throw()
		{
			return p == NULL;
		}
		bool operator!=(SCacheEntry* pT) const throw()
		{
			return !operator==(pT);
		}
		bool operator==(SCacheEntry* pT) const throw()
		{
			return p == pT;
		}
		//operator bool() const throw()
		//{
		//	return p != NULL;
		//}

	private:
		SCacheEntryPtr(SCacheEntry* a_p) : p(a_p) {}
	private:
		SCacheEntry* p;
	};
	typedef std::map<ULONG, SCacheEntryPtr> CLayerCache;
	typedef std::map<std::vector<ULONG>, SCacheEntryPtr> CMergedCache;

	struct SComposedObserver
	{
		SComposedObserver(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie, ULONG a_nLayerID) :
			pObserver(a_pObserver), tCookie(a_tCookie), nLayerID(a_nLayerID) {pObserver->AddRef();}
		IComposedPreviewObserver* pObserver;
		TCookie tCookie;
		ULONG nLayerID;
	};

	struct SEffectObserver
	{
		SEffectObserver(ILayerEffectObserver* a_pObserver, TCookie a_tCookie, ULONG a_nEffectID) :
			pObserver(a_pObserver), tCookie(a_tCookie), nEffectID(a_nEffectID) {pObserver->AddRef();}
		ILayerEffectObserver* pObserver;
		TCookie tCookie;
		ULONG nEffectID;
	};

	typedef std::map<CComBSTR, GUID> CLayerIconIDs;

private:
	enum { EDocumentCookie = 0xffffffff };
	SLayer const& FindLayer(ULONG a_nID) const
	{
		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == a_nID)
				return *i;
		}
		throw E_RW_ITEMNOTFOUND;
	}
	SLayer& FindLayer(ULONG a_nID)
	{
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == a_nID)
				return *i;
		}
		throw E_RW_ITEMNOTFOUND;
	}
	void LayersChanged();
	SLayer* FindLayer(IComparable* a_pItem);
	IOperationManager* M_OpMgr()
	{
		if (m_pOpMgr) return m_pOpMgr;
		ObjectLock cLock(this);
		if (m_pOpMgr) return m_pOpMgr;
		RWCoCreateInstance(m_pOpMgr, __uuidof(OperationManager));
		return m_pOpMgr;
	}
	void AgeCache();
	void DeleteCacheForLayer(ULONG a_nLayerID);
	void DeleteMergedCacheForLayer(ULONG a_nLayerID);
	void DeleteCacheForMerges();
	template<typename TCache>
	static SCacheEntryPtr FindOrCreateItemInCache(TCache& tCache, typename TCache::key_type const& tKey, CComAutoCriticalSection& tCritSec)
	{
		CComAutoCriticalSectionLock lock(tCritSec);

		typename TCache::iterator i = tCache.find(tKey);
		if (i != tCache.end())
		{
			i->second->nAge = 0;
			return i->second;
		}

		// create empty cache entry
		try
		{
			SCacheEntryPtr pItem = SCacheEntryPtr::Make();
			tCache[tKey] = pItem;
			return pItem;
		}
		catch (...)
		{
			return SCacheEntryPtr(); // very bad
		}
	}
	void ToChunks(CDocumentLayeredImage::CLayers::const_iterator begin, CDocumentLayeredImage::CLayers::const_iterator end, std::vector<ULONG>& ids, std::vector<CDocumentLayeredImage::SCacheEntryPtr>& dst);
	SCacheEntryPtr QueryLayerCache(ULONG a_nLayerID);
	SCacheEntryPtr QueryMergedCache(std::vector<ULONG> const& a_aLayerIDs, std::vector<SCacheEntryPtr>* a_pChunks);
	SCacheEntryPtr QueryMergedCache();
	void QueryCacheContent(TImagePoint* a_pOrigin, TImagePoint* a_pEnd);

	void MergeRectangle(TRasterImageRect a_tBounds, ULONG a_nStride, TPixelChannel* a_pPixels, TPixelChannel& a_tDefault, ELayerBlend a_eBlend, IDocumentImage* a_pImage);

	friend class CUndoStepLayerDelete;
	HRESULT LayerRestore(SLayer& a_sLayer, IDocumentData* a_pData, ULONG a_nUnder);
	static HRESULT LayersCopy(IDocumentLayeredImage* a_pDst, IDocumentLayeredImage* a_pSrc, IEnumUnknowns* a_pItems, IEnumUnknownsInit* a_pNewItems, IComparable* a_pBefore = NULL);

	class CStructuredChanges : public std::vector<TStructuredChange>
	{
	public:
		~CStructuredChanges()
		{
			for (std::vector<TStructuredChange>::iterator i = begin(); i != end(); ++i)
			{
				if (i->pItem)
					i->pItem->Release();
			}
		}
		template<class T, typename TParam1, typename TParam2>
		void Add(CDocumentLayeredImage* a_pDoc, ULONG a_nChangeFlags, TParam1 const& a_tParam1, TParam2 const& a_tParam2)
		{
			CComObject<T>* p = NULL;
			CComObject<T>::CreateInstance(&p);
			CComPtr<IUIItem> p2 = p;
			p->Init(a_pDoc, a_tParam1, a_tParam2);
			TStructuredChange tChange;
			tChange.nChangeFlags = a_nChangeFlags;
			tChange.pItem = p2;
			push_back(tChange);
			p2.Detach();
		}
		template<class T, typename TParam>
		void Add(CDocumentLayeredImage* a_pDoc, ULONG a_nChangeFlags, TParam const& a_tParam)
		{
			CComObject<T>* p = NULL;
			CComObject<T>::CreateInstance(&p);
			CComPtr<IUIItem> p2 = p;
			p->Init(a_pDoc, a_tParam);
			TStructuredChange tChange;
			tChange.nChangeFlags = a_nChangeFlags;
			tChange.pItem = p2;
			push_back(tChange);
			p2.Detach();
		}
		template<class T>
		void Add(CDocumentLayeredImage* a_pDoc, ULONG a_nChangeFlags)
		{
			CComObject<T>* p = NULL;
			CComObject<T>::CreateInstance(&p);
			CComPtr<IComparable> p2 = p;
			p->Init(a_pDoc);
			TStructuredChange tChange;
			tChange.nChangeFlags = a_nChangeFlags;
			tChange.pItem = p2;
			push_back(tChange);
			p2.Detach();
		}
		void Add(CDocumentLayeredImage* a_pDoc, ULONG a_nChangeFlags)
		{
			TStructuredChange tChange;
			tChange.nChangeFlags = a_nChangeFlags;
			tChange.pItem = NULL;
			push_back(tChange);
		}
		void AddItem(ULONG a_nChangeFlags, IComparable* a_pItem)
		{
			TStructuredChange tChange;
			tChange.nChangeFlags = a_nChangeFlags;
			tChange.pItem = a_pItem;
			push_back(tChange);
			if (a_pItem)
				a_pItem->AddRef();
		}
	};

private:
	CLayers m_cLayers;
	LONG m_nNextID;
	TImageSize m_tSize;

	CLayerChanges m_cLayerChanges;
	CStructuredChanges m_cSubDocChanges;
	bool m_bLayersChange;
	bool m_bSizeChange;
	bool m_bNonImageChange;

	bool m_bPrevLayers;
	bool m_bPrevLayerEffects;
	bool m_bPrevMetaData;

	CComPtr<IOperationManager> m_pOpMgr;
	CComPtr<IThreadPool> m_pThPool;

	TImageResolution m_tResolution;

	CLayerCache m_cLayerCache;
	CComAutoCriticalSection m_cLayerCS;
	CMergedCache m_cMergedCache;
	CComAutoCriticalSection m_cMergedCS;

	UINT m_nClipboardLayers;
	UINT m_nClipboardEffects;

	// composed observer implementation
	std::vector<SComposedObserver> m_aComposedObservers;
	size_t m_iComposedActNotifying;
	size_t m_iComposedMaxNotifying;
	CComAutoCriticalSection mutable m_cComposedCS;

	// layer effect observer implementation
	std::vector<SEffectObserver> m_aEffectObservers;
	size_t m_iEffectActNotifying;
	size_t m_iEffectMaxNotifying;
	CComAutoCriticalSection mutable m_cEffectCS;

	// cached shared gamma table (hack)
	CGammaTables m_cGamma;

	CComBSTR m_ENCFEAT_IMAGE;
	CComBSTR m_ENCFEAT_IMAGE_META;
	CComBSTR m_ENCFEAT_IMAGE_ALPHA;
	CComBSTR m_ENCFEAT_IMAGE_LAYER;
	CComBSTR m_ENCFEAT_IMAGE_LAYER_EFFECT;
	CComBSTR m_ENCFEAT_IMAGE_CANVAS;
	CComBSTR m_ENCFEAT_IMAGE_LAYER_SPECIAL;
};

