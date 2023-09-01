// DocumentAnimation.h : Declaration of the CDocumentAnimation

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentAnimation.h"
#include <RWDocumentImageRaster.h>
#include <SubjectImpl.h>
#include <ObserverImpl.h>
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include "GIFComment.h"


extern __declspec(selectany) GUID const g_aSupported[] =
{
	__uuidof(IDocumentAnimation),
	__uuidof(ISubDocumentsMgr),
	__uuidof(IDocumentAnimationClipboard),
	__uuidof(IDocumentGIF),
};


class ATL_NO_VTABLE CDocumentAnimation;

MIDL_INTERFACE("C9AC73D0-B5F6-49A9-A1B4-41F6ACD86916")
IFrame : public IUnknown
{
public:
	virtual CDocumentAnimation* Doc() = 0;
	virtual CComBSTR const& ID() = 0;
	virtual bool WriteFinished() = 0;
};

// CDocumentAnimation

class ATL_NO_VTABLE CDocumentAnimation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CDocumentDataImpl<CDocumentAnimation, &CLSID_DocumentFactoryAnimation, EUMMemoryLimited>,
	public CSubjectImpl<CStructuredRootImpl<CDocumentAnimation, ISubDocumentsMgr>, IStructuredObserver, TStructuredChanges>,
	public CSubjectImpl<IDocumentGIF, IAnimationObserver, TAnimationChange>,
	public CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>,
	public IDocumentAnimationClipboard
{
public:
	CDocumentAnimation() : m_nNextID(0), m_bFramesChange(false), m_bTimingChange(false),
		m_nSizeX(32), m_nSizeY(32), m_nLoopCount(0), m_eSaveMode(ESMBestQuality),
		m_ENCFEAT_ANIMATION(ENCFEAT_ANIMATION), m_ENCFEAT_IMAGE(ENCFEAT_IMAGE),
		m_ENCFEAT_IMAGE_META(ENCFEAT_IMAGE_META), m_ENCFEAT_IMAGE_ALPHA(ENCFEAT_IMAGE_ALPHA),
		m_ENCFEAT_IMAGE_LAYER(ENCFEAT_IMAGE_LAYER), m_ENCFEAT_IMAGE_LAYER_EFFECT(ENCFEAT_IMAGE_LAYER_EFFECT)
	{
		//m_tBackground.bR = m_tBackground.bG = m_tBackground.bB = 0;
		//m_tBackground.bA = 255;
	}


	static HRESULT WINAPI QISingleFrame(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
	{
		CDocumentAnimation* pThis = reinterpret_cast<CDocumentAnimation*>(a_pThis);
		CDocumentReadLock cLock(pThis);
		if (pThis->m_cFrames.size() == 1)
		{
			return pThis->M_Base()->DataBlockGet(pThis->m_cFrames[0].bstrID, a_iid, a_ppv);
		}
		else
		{
			*a_ppv = NULL;
			return E_NOINTERFACE;
		}
	}

BEGIN_COM_MAP(CDocumentAnimation)
	COM_INTERFACE_ENTRY(IDocumentData)
	COM_INTERFACE_ENTRY(IDocumentAnimation)
	COM_INTERFACE_ENTRY(IDocumentGIF)
	COM_INTERFACE_ENTRY(ISubDocumentsMgr)
	COM_INTERFACE_ENTRY(IStructuredRoot)
	COM_INTERFACE_ENTRY(IDocumentAnimationClipboard)
	COM_INTERFACE_ENTRY_FUNC_BLIND(0, QISingleFrame)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentData mehods
public:
	STDMETHOD(DataCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect);
	STDMETHOD(QuickInfo)(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo);
	STDMETHOD(WriteFinished)()
	{
		ULONG nChangeFlags = m_bFramesChange || m_bTimingChange ? EACAnimation : 0;
		bool bChange = false;
		std::vector<IUnknown*> cChangedFrames;
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			if (pData && S_OK == pData->WriteFinished())
				bChange = true;
			if (i->pFrame->WriteFinished())
				bChange = true;
			if (i->bChanged)
			{
				cChangedFrames.push_back(static_cast<IUIItem*>(i->pFrame));
				i->bChanged = false;
			}
		}

		std::vector<TStructuredChange> cStructuredChanges;
		if (m_bFramesChange)
		{
			TStructuredChange tChange;
			tChange.pItem = NULL;
			tChange.nChangeFlags = ESCChildren;
			cStructuredChanges.push_back(tChange);
		}
		if (!cStructuredChanges.empty())
		{
			bChange = true;
			TStructuredChange tChange;
			tChange.pItem = NULL;
			tChange.nChangeFlags = ESCChildren;
			TStructuredChanges tChanges;
			tChanges.nChanges = cStructuredChanges.size();
			tChanges.aChanges = &(cStructuredChanges[0]);
			CSubjectImpl<CStructuredRootImpl<CDocumentAnimation, ISubDocumentsMgr>, IStructuredObserver, TStructuredChanges>::Fire_Notify(tChanges);
			cStructuredChanges.clear();
		}
		if (m_bFramesChange || m_bTimingChange)
		{
			M_Base()->SetDirty();
			if (m_bFramesChange)
				M_Base()->UpdateQuickInfo();
		}
		m_bFramesChange = false;
		m_bTimingChange = false;

		if (nChangeFlags || cChangedFrames.size())
		{
			TAnimationChange tChg;
			tChg.nFlags = nChangeFlags;
			tChg.nFrames = cChangedFrames.size();
			tChg.apFrames = tChg.nFrames ? &cChangedFrames[0] : NULL;
			CSubjectImpl<IDocumentGIF, IAnimationObserver, TAnimationChange>::Fire_Notify(tChg);

			// update animation size
			TImageSize tSize = {0, 0};
			for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
			{
				CComPtr<IDocumentImage> pImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
				if (pImg)
				{
					TImageSize tIS = {0, 0};
					pImg->CanvasGet(&tIS, NULL, NULL, NULL, NULL);
					if (tIS.nX > tSize.nX) tSize.nX = tIS.nX;
					if (tIS.nY > tSize.nY) tSize.nY = tIS.nY;
				}
			}
			if (tSize.nX*tSize.nY)
			{
				m_nSizeX = tSize.nX;
				m_nSizeY = tSize.nY;
			}
		}
		return bChange ? S_OK : S_FALSE;
	}
	STDMETHOD(MaximumUndoSize)(ULONGLONG* a_pMaximumSize)
	{
		*a_pMaximumSize = 20000000; // 20MB seems to be enough for animated image undo history
		return S_OK;
	}
	STDMETHOD(ComponentFeatureOverride)(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface);
	STDMETHOD(ComponentLocationGet)(BSTR a_bstrID, IStorageFilter* a_pThisLoc, IStorageFilter** a_ppComponentLoc);
	STDMETHOD(RemovingBlock)();
	STDMETHOD(LocationChanged)(IStorageFilter* a_pOldLoc);
	STDMETHOD(Aspects)(IEnumEncoderAspects* a_pEnumAspects);

	void OwnerNotify(TCookie a_tCookie, TImageChange a_nParam)
	{
		try
		{
			for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
			{
				if (i->nID == a_tCookie)
				{
					i->bChanged = true;
					break;
				}
			}
		}
		catch (...)
		{
			// TODO: log?
		}
	}

	// IStructuredRoot(ISubDocumentsMgr) methods
public:
	STDMETHOD(StatePack)(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState);
	HRESULT ISubDocumentsMgr_StateUnpack(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems);
	STDMETHOD(ISubDocumentsMgr::StateUnpack)(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems)
	{ return ISubDocumentsMgr_StateUnpack(a_pState, a_ppSelectedItems); }
	STDMETHOD(ItemsEnum)(IComparable* a_pRoot, IEnumUnknowns** a_ppSubItems);

	// ISubDocumentsMgr methods
public:
	STDMETHOD(FindByName)(BSTR a_bstrName, ISubDocumentID** a_ppItem);

	// IDocumentAnimation methods
public:
	STDMETHOD(FramesEnum)(IEnumUnknowns** a_ppFrames);
	STDMETHOD(IsFramePresent)(IUnknown* a_pFrame);
	STDMETHOD(FrameDel)(IUnknown* a_pFrame);
	STDMETHOD(FrameIns)(IUnknown* a_pBefore, IAnimationFrameCreator* a_pCreator, IUnknown** a_ppNewFrame);
	STDMETHOD(FrameMove)(IUnknown* a_pBefore, IUnknown* a_pSrcFrame);
	STDMETHOD(FrameGetDoc)(IUnknown* a_pFrame, IDocument** a_ppFrameDoc);
	STDMETHOD(FrameSetDoc)(IUnknown* a_pFrame, IDocument* a_pFrameDoc);
	STDMETHOD(FrameGetTime)(IUnknown* a_pFrame, float* a_pSeconds);
	STDMETHOD(FrameSetTime)(IUnknown* a_pFrame, float a_fSeconds);
	STDMETHOD(TimeUnitInfo)(float* a_pSecondsPerUnit, LCID a_tLocaleHint, BSTR* a_pbstrUnitName);
	STDMETHOD(LoopCountGet)(ULONG* a_pCount);
	STDMETHOD(LoopCountSet)(ULONG a_nCount);
	STDMETHOD(StatePack)(IEnumUnknowns* a_pFrames, ISharedState** a_ppState);
	STDMETHOD(IDocumentAnimation::StateUnpack)(ISharedState* a_pState, IEnumUnknowns** a_ppFrames)
	{ return ISubDocumentsMgr_StateUnpack(a_pState, a_ppFrames); } // TODO: update if semantic changes
	STDMETHOD(StatePrefix)(BSTR* a_pbstrPrefix)
	{
		try
		{
			*a_pbstrPrefix = NULL;
			*a_pbstrPrefix = CComBSTR(M_DataID()).Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrPrefix ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IDocumentAnimationClipboard methods
public:
	STDMETHOD(CanPaste)();
	STDMETHOD(Paste)(RWHWND a_hWindow, IUnknown* a_pBefore, IEnumUnknowns** a_ppNewFrameIDs);
	STDMETHOD(Copy)(RWHWND a_hWindow, ULONG a_nFrames, IUnknown** a_apFrames);
	STDMETHOD(CanDrop)(IEnumStrings* a_pFiles);
	STDMETHOD(Drop)(IEnumStrings* a_pFiles, IUnknown* a_pBefore, IEnumUnknowns** a_ppNewFrameIDs);

	// IDocumentAnimation methods
public:
	STDMETHOD(FrameGetWaitFlag)(IUnknown* a_pFrame, boolean* a_pWaitForInput)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(FrameSetWaitFlag)(IUnknown* a_pFrame, boolean a_bWaitForInput)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(SizeGet)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		try
		{
			CDocumentReadLock cLock(this);
			*a_pSizeX = m_nSizeX;
			*a_pSizeY = m_nSizeY;
			return S_OK;
		}
		catch (...)
		{
			return a_pSizeX && a_pSizeY ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SizeSet)(ULONG a_nSizeX, ULONG a_nSizeY)
	{
		try
		{
			if (a_nSizeX == 0 || a_nSizeY == 0)
				return E_RW_INVALIDPARAM;
			CDocumentWriteLock cLock(this);
			if (a_nSizeX == m_nSizeX && a_nSizeY == m_nSizeY)
				return S_FALSE;

			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;
			for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
			{
				CComPtr<IDocumentEditableImage> pDEI;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
				if (pDEI)
				{
					TImageSize tNewSize = {m_nSizeX, m_nSizeY};
					pDEI->CanvasSet(&tNewSize, NULL, NULL, NULL);
				}
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}


	bool AppendFrame(IDocument* a_pFrame, float a_fSeconds = -1.0f, IEnumUnknownsInit* a_pNewFrames = NULL);

	HRESULT GetSubDoc(BSTR a_bstrID, IDocument** a_ppSubDocument)
	{
		return M_Base()->DataBlockDoc(a_bstrID, a_ppSubDocument);
	}
	void GetFrameID(ULONG a_nID, CComBSTR& a_bstrID)
	{
		a_bstrID = M_DataID();
		OLECHAR sz[20];
		swprintf(sz, L"F%i;", a_nID);
		a_bstrID += sz;
	}
	void GetNewFrameID(CComBSTR& a_bstrID, ULONG& a_nID)
	{
		GetFrameID(a_nID = InterlockedIncrement(&m_nNextID), a_bstrID);
	}

public:
	class ATL_NO_VTABLE CFrame :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ISubDocumentID,
		public IUIItem,
		public IFrame
	{
	public:
		CFrame() : m_pDoc(NULL)
		{
		}
		void Init(CDocumentAnimation* a_pDoc, BSTR a_bstrID)
		{
			m_pDoc = a_pDoc;
			m_bstrID = a_bstrID;
		}

	BEGIN_COM_MAP(CFrame)
		COM_INTERFACE_ENTRY(IUIItem)
		COM_INTERFACE_ENTRY(IFrame)
		COM_INTERFACE_ENTRY(ISubDocumentID)
		COM_INTERFACE_ENTRY2(IComparable, IUIItem)
	END_COM_MAP()

		// IComparable methods
	public:
		STDMETHOD(Compare)(IComparable* a_pOther)
		{
			try
			{
				CComQIPtr<IComparable> pOther(static_cast<IUnknown*>(a_pOther));
				if (pOther.p == static_cast<IUIItem*>(this))
					return S_OK;
				else
					return pOther.p < static_cast<IUIItem*>(this) ? S_LESS : S_MORE;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
		{
			try
			{
				static const GUID tID = {0x20c8f3e9, 0xde6d, 0x4bcd, {0xb3, 0xac, 0xb8, 0x67, 0x4c, 0xf9, 0x9a, 0xec}};
				*a_pCLSID = tID;
				return S_OK;
			}
			catch (...)
			{
				return E_POINTER;
			}
		}

		// IUIItem methods
	public:
		STDMETHOD(NameGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
		{
			try
			{
				*a_pbstrName = NULL;
				OLECHAR szTmp[32] = L"";
				//swprintf(szTmp, L"#%d", 0); // TODO: frame index
				CMultiLanguageString::GetLocalized(L"[0409]Frame[0405]Snímek", a_tPreferedLCID, a_pbstrName);
				return S_OK;
			}
			catch (...)
			{
				return a_pbstrName == NULL ? E_POINTER : E_UNEXPECTED;
			}
		}
		STDMETHOD(DescriptionGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(ColorsGet)(DWORD* UNREF(a_prgbPrimary), DWORD* UNREF(a_prgbSecondary))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IconIDGet)(GUID* a_pIconID)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(UseThumbnail)() { return S_OK; }
		STDMETHOD(ExpandedByDefault)() { return S_FALSE; }

		// ISubDocumentID methods
	public:
		STDMETHOD(SubDocumentGet)(IDocument** a_ppSubDocument)
		{
			try
			{
				*a_ppSubDocument = NULL;
				if (NULL == m_pDoc)
					return E_FAIL;
				return m_pDoc->GetSubDoc(m_bstrID, a_ppSubDocument);
			}
			catch (...)
			{
				return E_POINTER;
			}
		}

		// IIconImage methods
	public:
		CDocumentAnimation* Doc() { return m_pDoc; }
		CComBSTR const& ID() { return m_bstrID; }
		bool WriteFinished()
		{
			return false;
		}

	private:
		CDocumentAnimation* m_pDoc;
		CComBSTR m_bstrID;
	};

	void FrameMoveIntern(size_t a_iFrom, size_t a_iTo);
	void FrameInsIntern(ULONG a_nID, BSTR a_bstrID, IDocumentData* a_pData, CComObject<CFrame>* a_pFrame, float a_fTime, size_t a_nBefore);
	HRESULT FrameDel(ULONG a_nID);
	void FrameSetDocIntern(ULONG m_nID, IDocumentData* a_pData);

	HRESULT ComposedPreviewProcessTile(ULONG a_nLayerID, EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData);

private:
	struct SFrame
	{
		SFrame() : pFrame(NULL), bChanged(false)
		{
		}
		SFrame(SFrame const& a_rhs)
		{
			nID = a_rhs.nID;
			bstrID = a_rhs.bstrID;
			fTime = a_rhs.fTime;
			pFrame = a_rhs.pFrame;
			if (pFrame) pFrame->AddRef();
			bChanged = a_rhs.bChanged;
		}
		SFrame(CDocumentAnimation* a_pDoc, ULONG a_nID, BSTR a_bstrID, float a_fTime, CComObject<CFrame>* a_pItem) :
			nID(a_nID), bstrID(a_bstrID), fTime(a_fTime), pFrame(NULL), bChanged(false)
		{
			if (a_pItem == NULL)
			{
				CComObject<CFrame>::CreateInstance(&pFrame);
				pFrame->Init(a_pDoc, a_bstrID);
			}
			else
			{
				pFrame = a_pItem;
			}
			pFrame->AddRef();
		}
		SFrame(CDocumentAnimation* a_pDoc, float a_fTime) : fTime(a_fTime), pFrame(NULL), bChanged(false)
		{
			a_pDoc->GetNewFrameID(bstrID, nID);
			CComObject<CFrame>::CreateInstance(&pFrame);
			pFrame->Init(a_pDoc, bstrID);
			pFrame->AddRef();
		}
		~SFrame()
		{
			if (pFrame) pFrame->Release();
		}
		SFrame& operator=(SFrame const& a_rhs)
		{
			if (&a_rhs != this)
			{
				nID = a_rhs.nID;
				bstrID = a_rhs.bstrID;
				fTime = a_rhs.fTime;
				bChanged = a_rhs.bChanged;
				if (a_rhs.pFrame) a_rhs.pFrame->AddRef();
				if (pFrame) pFrame->Release();
				pFrame = a_rhs.pFrame;
			}
			return *this;
		}
		ULONG nID;
		CComBSTR bstrID;
		float fTime; // [s]
		CComObject<CFrame>* pFrame;
		bool bChanged;
	};
	typedef std::vector<SFrame> CFrames;
	typedef std::vector<CComPtr<IGIFComment> > CComments;
	enum ESaveMode
	{
		ESMDithered = 0x1,
		ESMGlobalPalette = 0x2,

		ESMBestQuality = ESMDithered,
		ESMGoodQuality = 0,
		ESMGoodCompression = ESMDithered|ESMGlobalPalette,
		ESMBestCompression = ESMGlobalPalette,
	};

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	ULONG m_nLoopCount;
	ESaveMode m_eSaveMode;
	//TRasterImagePixel m_tBackground;
	CFrames m_cFrames;
	LONG m_nNextID;
	CComments m_cComments;
	bool m_bFramesChange;
	bool m_bTimingChange;

	CComBSTR m_ENCFEAT_ANIMATION;
	CComBSTR m_ENCFEAT_IMAGE;
	CComBSTR m_ENCFEAT_IMAGE_META;
	CComBSTR m_ENCFEAT_IMAGE_ALPHA;
	CComBSTR m_ENCFEAT_IMAGE_LAYER;
	CComBSTR m_ENCFEAT_IMAGE_LAYER_EFFECT;
};

