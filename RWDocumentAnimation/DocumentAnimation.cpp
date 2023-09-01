// DocumentAnimation.cpp : Implementation of CDocumentAnimation

#include "stdafx.h"
#include "DocumentAnimation.h"
#include <DocumentName.h>
#include "AnimationUndo.h"


// CDocumentAnimation

STDMETHODIMP CDocumentAnimation::DataCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		CDocumentWriteLock cLock(this);
		//CDocumentReadLock cLock(this); // prevent locking problems when duplicating frames

		CComObject<CDocumentAnimation>* p = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		CDocumentAnimation* pDoc = p;
		a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
		pDoc->m_nSizeX = m_nSizeX;
		pDoc->m_nSizeY = m_nSizeY;
		pDoc->m_nLoopCount = m_nLoopCount;
		pDoc->m_eSaveMode = m_eSaveMode;
		pDoc->m_nNextID = m_nNextID;

		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			SFrame sFrame(pDoc, i->nID, i->bstrID, i->fTime, NULL);
			pDoc->m_cFrames.push_back(sFrame);
			CComPtr<IDocumentData> pDD;
			M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pDD));
			pDD->DataCopy(sFrame.bstrID, a_pBase, a_tPreviewEffectID, i == m_cFrames.begin() ? a_pPreviewEffect : NULL);

			{ // add observer
				CComPtr<IDocumentImage> pDocImg;
				a_pBase->DataBlockGet(sFrame.bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
				if (pDocImg) pDocImg->ObserverIns(pDoc->CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), sFrame.nID);
			}
		}

		//if (M_MetaData())
		//	pDoc->CopyMetaData(M_MetaData(), a_eHint);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>

STDMETHODIMP CDocumentAnimation::QuickInfo(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
{
	try
	{
		if (a_nInfoIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		*a_ppInfo = NULL;
		CComObject<CPrintfLocalizedString>* pPFStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pPFStr);
		CComPtr<ILocalizedString> pStr = pPFStr;
		CComPtr<ILocalizedString> pTempl;
		if (m_cFrames.size() > 1)
		{
			pTempl.Attach(new CMultiLanguageString(L"[0409]%ix%i pixels, %i frames[0405]%ix%i pixelů, %i snímků"));
			pPFStr->Init(pTempl, m_nSizeX, m_nSizeY, m_cFrames.size());
		}
		else
		{
			pTempl.Attach(new CMultiLanguageString(L"[0409]%ix%i pixels[0405]%ix%i pixelů"));
			pPFStr->Init(pTempl, m_nSizeX, m_nSizeY);
		}
		*a_ppInfo = pStr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
	return E_NOTIMPL;
}

class ATL_NO_VTABLE CAnimationComposedPreview : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageComposedPreview
{
public:
	CAnimationComposedPreview() : m_pDoc(NULL)
	{
	}
	~CAnimationComposedPreview()
	{
		if (m_pDoc) m_pDoc->Release();
	}
	void Init(CDocumentAnimation* a_pDoc, ULONG a_nFrameID, IRasterImageComposedPreview* a_pOrig)
	{
		(m_pDoc = a_pDoc)->AddRef();
		m_nFrameID = a_nFrameID;
		m_pOrig.Attach(a_pOrig);
	}

BEGIN_COM_MAP(CAnimationComposedPreview)
	COM_INTERFACE_ENTRY(IRasterImageComposedPreview)
END_COM_MAP()

	// IRasterImageComposedPreview methods
public:
	STDMETHOD(AdjustDirtyRect)(RECT* a_prc)
	{
		return m_pOrig ? m_pOrig->AdjustDirtyRect(a_prc) : E_NOTIMPL;
	}
	STDMETHOD(PreProcessTile)(RECT* a_prc)
	{
		return m_pOrig ? m_pOrig->PreProcessTile(a_prc) : E_NOTIMPL;
	}
	STDMETHOD(ProcessTile)(EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData)
	{
		if (m_pOrig) m_pOrig->ProcessTile(a_eMode == ECPMTransparent ? ECPMFinal : a_eMode, a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pData);
		return m_pDoc->ComposedPreviewProcessTile(m_nFrameID, a_eMode, a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pData);
	}
	STDMETHOD(InputTransform)(TMatrix3x3f* a_pTransform)
	{
		return m_pOrig ? m_pOrig->InputTransform(a_pTransform) : E_NOTIMPL;
	}
	STDMETHOD(ObserverIns)(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie)
	{
		if (m_pOrig) m_pOrig->ObserverIns(a_pObserver, a_tCookie);
		return S_OK;//m_pDoc->ComposedObserverIns(a_pObserver, a_tCookie, m_nLayerID);
	}
	STDMETHOD(ObserverDel)(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie)
	{
		if (m_pOrig) m_pOrig->ObserverDel(a_pObserver, a_tCookie);
		return S_OK;//m_pDoc->ComposedObserverDel(a_pObserver, a_tCookie, m_nLayerID);
	}

private:
	CDocumentAnimation* m_pDoc;
	ULONG m_nFrameID;
	CComPtr<IRasterImageComposedPreview> m_pOrig;
};

class ATL_NO_VTABLE CFrameBlender :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IImageVisitor
{
public:
	void Init(TImagePoint a_tOrig, TImageSize a_tSize, ULONG a_nStride, TPixelChannel* a_pBuffer, BYTE a_bBias)
	{
		m_tOrig = a_tOrig;
		m_tSize = a_tSize;
		m_nStride = a_nStride;
		m_pBuffer = a_pBuffer;
		m_bBias = a_bBias;
	}

	BEGIN_COM_MAP(CFrameBlender)
		COM_INTERFACE_ENTRY(IImageVisitor)
	END_COM_MAP()

	// IImageVisitor methods
public:
	STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl* UNREF(a_pControl))
	{
		for (TImageTile const* p = a_aTiles; p != a_aTiles+a_nTiles; ++p)
		{
			TPixelChannel* pD = m_pBuffer+(p->tOrigin.nY-m_tOrig.nY)*m_nStride+p->tOrigin.nX-m_tOrig.nX;
			TPixelChannel const* pS = p->pData;
			for (ULONG y = 0; y < p->tSize.nY; ++y)
			{
				for (TPixelChannel* const pEnd = pD+p->tSize.nX; pD != pEnd; ++pD, pS+=p->tStride.nX)
				{
					if (pS->bA)
					{
						// blend pixels
						ULONG const nSA = pS->bA>>1;
						ULONG const nNewA = nSA*255 + (255-nSA)*pD->bA;
						if (nNewA)
						{
							ULONG const bA1 = (255-nSA)*pD->bA;
							ULONG const bA2 = nSA*255;
							pD->bB = (pD->bB*bA1 + ((pS->bB>>2)+m_bBias)*bA2)/nNewA;
							pD->bG = (pD->bG*bA1 + ((pS->bG>>2)+m_bBias)*bA2)/nNewA;
							pD->bR = (pD->bR*bA1 + ((pS->bR>>2)+m_bBias)*bA2)/nNewA;
						}
						else
						{
							pD->bB = pD->bG = pD->bR = 0;
						}
						pD->bA = nNewA/255;
					}
				}
				pD += m_nStride - p->tSize.nX;
				pS += p->tStride.nY - p->tStride.nX*p->tSize.nX;
			}
		}
		return S_OK;
	}

private:
	TImagePoint m_tOrig;
	TImageSize m_tSize;
	ULONG m_nStride;
	TPixelChannel* m_pBuffer;
	BYTE m_bBias;
};

HRESULT CDocumentAnimation::ComposedPreviewProcessTile(ULONG a_nFrameID, EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData)
{
	if (a_eMode != ECPMTransparent || m_cFrames.size() == 0)
		return S_OK;

	try
	{
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (a_nFrameID == i->nID)
			{
				TImagePoint tOrig = {a_nX, a_nY};
				TImageSize tSize = {a_nSizeX, a_nSizeY};
				CComObjectStackEx<CFrameBlender> cFrameBlender;
				cFrameBlender.Init(tOrig, tSize, a_nStride, reinterpret_cast<TPixelChannel*>(a_pData), 0x20);
				{
					CComPtr<IDocument> pDoc2;
					M_Base()->DataBlockDoc(i != m_cFrames.begin() ? (i-1)->bstrID.m_str : m_cFrames.rbegin()->bstrID.m_str, &pDoc2);
					CComPtr<IDocumentImage> pImg2;
					if (pDoc2) pDoc2->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg2));
					if (pImg2) pImg2->Inspect(EICIRGBA, &tOrig, &tSize, &cFrameBlender, NULL, EIRIPreview);
				}
				//cFrameBlender.Init(tOrig, tSize, a_nStride, reinterpret_cast<TPixelChannel*>(a_pData), 0xa0);
				//if (i+1 != m_cFrames.end())
				//{
				//	CComPtr<IDocument> pDoc2;
				//	M_Base()->DataBlockDoc((i+1)->bstrID, &pDoc2);
				//	CComPtr<IDocumentImage> pImg2;
				//	if (pDoc2) pDoc2->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg2));
				//	if (pImg2) pImg2->Inspect(EICIRGBA, &tOrig, &tSize, &cFrameBlender, NULL, EIRIPreview);
				//}
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::ComponentFeatureOverride(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface)
{
	try
	{
		ULONG nIDLen = SysStringLen(a_bstrID);
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			ULONG nImageIDLen = i->bstrID.Length();
			if (nIDLen >= nImageIDLen && 0 == wcsncmp(i->bstrID, a_bstrID, nImageIDLen))
			{
				if (nIDLen == nImageIDLen)
				{
//					if (IsEqualIID(a_iid, __uuidof(IDocumentEditableImage))) // not actually that much effective
//					{
//ATLASSERT(0);
//						if (*a_ppFeatureInterface)
//						{
//							reinterpret_cast<IDocumentEditableImage*>(*a_ppFeatureInterface)->Release();
//							*a_ppFeatureInterface = NULL;
//						}
//						return E_NOINTERFACE;
//					}
				}
				else
				{
					// forward call to child
					CComPtr<IDocumentData> pChild;
					M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pChild));
					if (pChild)
						pChild->ComponentFeatureOverride(a_bstrID, a_iid, a_ppFeatureInterface);
					if (IsEqualIID(a_iid, __uuidof(IRasterImageComposedPreview)))
					{
						CComObject<CAnimationComposedPreview>* pWrapper = NULL;
						CComObject<CAnimationComposedPreview>::CreateInstance(&pWrapper);
						CComPtr<IRasterImageComposedPreview> pDoc = pWrapper;
						pWrapper->Init(this, i->nID, reinterpret_cast<IRasterImageComposedPreview*>(*a_ppFeatureInterface));
						*a_ppFeatureInterface = pDoc.Detach();
					}
				}
				break;
			}
		}
		return *a_ppFeatureInterface ? S_OK : E_NOINTERFACE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::ComponentLocationGet(BSTR a_bstrID, IStorageFilter* a_pThisLoc, IStorageFilter** a_ppComponentLoc)
{
	try
	{
		*a_ppComponentLoc = NULL;
		ULONG nIDLen = SysStringLen(a_bstrID);
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			ULONG nFrameIDLen = i->bstrID.Length();
			if (nIDLen >= nFrameIDLen && 0 == wcsncmp(i->bstrID, a_bstrID, nFrameIDLen))
			{
				wchar_t szName[MAX_PATH+35] = L"";
				CDocumentName::GetDocNameW(a_pThisLoc, szName, itemsof(szName)-5);
				wchar_t* pLastDot = wcsrchr(szName, L'.');
				wchar_t sz2[32] = L"";
				swprintf(sz2, L"-F%i", 1+(i-m_cFrames.begin()));
				if (pLastDot)
					wcscpy(pLastDot, sz2);
				else
					wcscat(szName, sz2);
				CComObject<CDocumentName>* pName = NULL;
				CComObject<CDocumentName>::CreateInstance(&pName);
				CComPtr<IStorageFilter> pTmp = pName;
				pName->Init(szName);
				if (nIDLen > nFrameIDLen)
				{
					// forward call to child
					CComPtr<IDocumentData> pChild;
					M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pChild));
					if (pChild)
					{
						CComPtr<IStorageFilter> pTmp2;
						pChild->ComponentLocationGet(a_bstrID, pTmp, &pTmp2);
						std::swap(pTmp.p, pTmp2.p);
					}
				}
				*a_ppComponentLoc = pTmp.Detach();
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::RemovingBlock()
{
	try
	{
		CDocumentWriteLock cLock(this);
		CComPtr<IEnumUnknowns> pItems;
		ItemsEnum(NULL, &pItems);
		CComPtr<IComparable> pItem;
		for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); ++i, pItem = NULL)
		{
			FrameDel(pItem);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::LocationChanged(IStorageFilter* a_pOldLoc)
{
	// TODO: automatic extension switching
	//ATLASSERT(0);
	return E_NOTIMPL;
}

class CFrameEncoderChecker :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumEncoderAspects
{
public:
	CFrameEncoderChecker() : m_bAlpha(false), m_bLayers(false), m_bEffect(false) {}
	bool Alpha() const { return m_bAlpha; }
	bool Layers() const { return m_bLayers; }
	bool Effect() const { return m_bEffect; }

	BEGIN_COM_MAP(CFrameEncoderChecker)
		COM_INTERFACE_ENTRY(IEnumEncoderAspects)
	END_COM_MAP()

	// IEnumEncoderAspects methods
public:
	STDMETHOD(Range)(ULONG* a_pBegin, ULONG* a_nCount) { return S_OK; }
	STDMETHOD(Consume)(ULONG a_nBegin, ULONG a_nCount, BSTR const* a_abstrID, float const* a_afWeight)
	{
		for (; a_nCount > 0; --a_nCount, ++a_abstrID)
		{
			if (wcscmp(ENCFEAT_IMAGE_ALPHA, *a_abstrID) == 0)
				m_bAlpha = true;
			if (wcscmp(ENCFEAT_IMAGE_LAYER, *a_abstrID) == 0)
				m_bLayers = true;
			if (wcscmp(ENCFEAT_IMAGE_LAYER_EFFECT, *a_abstrID) == 0)
				m_bLayers = m_bEffect = true;
		}
		return S_OK;
	}

private:
	bool m_bAlpha;
	bool m_bLayers;
	bool m_bEffect;
};


STDMETHODIMP CDocumentAnimation::Aspects(IEnumEncoderAspects* a_pEnumAspects)
{
	try
	{
		CDocumentReadLock cLock(this);
		if (m_cFrames.size() == 1)
		{
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(m_cFrames[0].bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			if (pData)
				return pData->Aspects(a_pEnumAspects);
		}
		bool bLayers = false;
		bool bAlpha = false;
		bool bEffect = false;
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end() && !(bLayers && bAlpha && bEffect); ++i)
		{
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			if (pData)
			{
				CComObjectStackEx<CFrameEncoderChecker> cChecker;
				pData->Aspects(&cChecker);
				bLayers |= cChecker.Layers();
				bAlpha |= cChecker.Alpha();
				bEffect |= cChecker.Effect();
			}
		}
		float aVals[6] = {100.0f, 10.0f, 0.1f, 0.1f, 0.1f};
		BSTR aIDs[6];
		ULONG n = 0;
		aIDs[n++] = m_ENCFEAT_ANIMATION;
		aIDs[n++] = m_ENCFEAT_IMAGE;
		if (bLayers)
		{
			aVals[n] = 5.0f;
			aIDs[n++] = m_ENCFEAT_IMAGE_LAYER;
		}
		if (bAlpha)
			aIDs[n++] = m_ENCFEAT_IMAGE_ALPHA;
		if (bEffect)
			aIDs[n++] = m_ENCFEAT_IMAGE_LAYER_EFFECT;
		if (/*MetaDataPresent()*/false)
			aIDs[n++] = m_ENCFEAT_IMAGE_META;
		a_pEnumAspects->Consume(0, n, aIDs, aVals);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


STDMETHODIMP CDocumentAnimation::StatePack(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState)
{
	try
	{
		*a_ppState = NULL;
		CComPtr<ISharedState> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
		if (FAILED(pInit->InsertMultiple(a_nItems, reinterpret_cast<IUnknown* const*>(a_paItems)))) // ugly but working ( <= no multiple inheritance in COM interfaces ... )
			return E_FAIL;
		*a_ppState = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppState == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HRESULT CDocumentAnimation::ISubDocumentsMgr_StateUnpack(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems)
{
	try
	{
		*a_ppSelectedItems = NULL;
		if (a_pState)
			return a_pState->QueryInterface(a_ppSelectedItems);

		// select first frame by default
		CComPtr<IEnumUnknownsInit> pInit;
		RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));

		CDocumentReadLock cLock(this);
		if (!m_cFrames.empty())
			pInit->Insert(static_cast<IUIItem*>(m_cFrames.begin()->pFrame));

		*a_ppSelectedItems = pInit.Detach();
		return S_FALSE;
	}
	catch (...)
	{
		return a_ppSelectedItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::ItemsEnum(IComparable* a_pRoot, IEnumUnknowns** a_ppSubItems)
{
	try
	{
		*a_ppSubItems = NULL;
		CComPtr<IEnumUnknownsInit> pInit;
		RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));

		CDocumentReadLock cLock(this);
		if (a_pRoot == NULL)
		{
			for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
			{
				HRESULT hRes = pInit->Insert(static_cast<IUIItem*>(i->pFrame));
				if (FAILED(hRes)) return hRes;
			}
		}

		*a_ppSubItems = pInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::FindByName(BSTR a_bstrName, ISubDocumentID** a_ppItem)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentAnimation::FramesEnum(IEnumUnknowns** a_ppFrames)
{
	try
	{
		*a_ppFrames = NULL;
		CComPtr<IEnumUnknownsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));

		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (FAILED(pTmp->Insert(static_cast<IUIItem*>(i->pFrame))))
				return E_FAIL;
		}

		*a_ppFrames = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppFrames ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentAnimation::IsFramePresent(IUnknown* a_pFrame)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->bstrID == pFrame->ID())
				return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::FrameDel(IUnknown* a_pFrame)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->bstrID == pFrame->ID())
			{
				{ // remove observer
					CComPtr<IDocumentImage> pDocImg;
					M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
					if (pDocImg) pDocImg->ObserverDel(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
				}

				CComPtr<IDocumentData> pData;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
				M_Base()->DataBlockSet(i->bstrID, NULL);
				if (UndoEnabled() == S_OK)
				{
					CUndoFrameDel::Add(M_Base(), this, i->nID, i->bstrID, pData, i->pFrame, i->fTime, i-m_cFrames.begin());
				}
				m_cFrames.erase(i);
				m_bFramesChange = true;

				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentAnimation::FrameDel(ULONG a_nID)
{
	for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
	{
		if (i->nID == a_nID)
		{
			{ // remove observer
				CComPtr<IDocumentImage> pDocImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
				if (pDocImg) pDocImg->ObserverDel(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
			}

			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			M_Base()->DataBlockSet(i->bstrID, NULL);
			if (UndoEnabled() == S_OK)
			{
				CUndoFrameDel::Add(M_Base(), this, i->nID, i->bstrID, pData, i->pFrame, i->fTime, i-m_cFrames.begin());
			}
			m_cFrames.erase(i);
			m_bFramesChange = true;

			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CDocumentAnimation::FrameIns(IUnknown* a_pBefore, IAnimationFrameCreator* a_pCreator, IUnknown** a_ppNewFrame)
{
	try
	{
		if (a_ppNewFrame) *a_ppNewFrame = NULL;
		CDocumentWriteLock cLock(this);

		GUID tDefBuilder = __uuidof(DocumentFactoryRasterImage);
		if (!m_cFrames.empty() && a_pCreator == NULL)
		{
			CComPtr<IDocument> pSubDoc;
			CComBSTR bstrID;
			GetFrameID(m_cFrames.begin()->nID, bstrID);
			GetSubDoc(bstrID, &pSubDoc);
			CComPtr<IDocumentLayeredImage> pDLI;
			if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			CComPtr<IEnumUnknowns> pLayers;
			if (pDLI) pDLI->LayersEnum(NULL, &pLayers);
			CComPtr<IComparable> pLayer;
			if (pLayers) pLayers->Get(0, &pLayer);
			CComQIPtr<ISubDocumentID> pSDID(pLayer);
			CComPtr<IDocument> pLayerDoc;
			if (pSDID) pSDID->SubDocumentGet(&pLayerDoc);
			if (pLayerDoc) pLayerDoc->BuilderID(&tDefBuilder);
			if (IsEqualGUID(tDefBuilder, __uuidof(DocumentFactoryLayeredImage)))
				tDefBuilder = __uuidof(DocumentFactoryRasterImage);
		}

		SFrame sFrame(this, 0.1f);
		if (UndoEnabled() == S_OK)
		{
			CUndoFrameIns::Add(M_Base(), this, sFrame.nID);
		}
		m_bFramesChange = true;

		size_t nBefore = m_cFrames.size();
		if (a_pBefore)
		{
			CComQIPtr<IFrame> pBefore(a_pBefore);
			for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
			{
				if (i->bstrID == pBefore->ID())
				{
					nBefore = i-m_cFrames.begin();
					m_cFrames.insert(i, sFrame);
					break;
				}
			}
		}
		if (nBefore == m_cFrames.size())
			m_cFrames.push_back(sFrame);

		CComPtr<IDocumentBuilder> pBuilder;
		RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryLayeredImage));
		if (a_pCreator)
		{
			if (FAILED(a_pCreator->Create(pBuilder, sFrame.bstrID, M_Base())))
			{
				m_cFrames.erase(m_cFrames.begin()+nBefore);
				return E_FAIL;
			}
		}
		else
		{
			CComQIPtr<IDocumentFactoryLayeredImage> pFct2(pBuilder);
			if (SUCCEEDED(pFct2->Create(sFrame.bstrID, M_Base())))
			{
				CComPtr<IImageLayerFactory> pILF;
				RWCoCreateInstance(pILF, tDefBuilder);
				CComPtr<IImageLayerCreator> pLayerCreator;
				pILF->LayerCreatorGet(CImageSize(m_nSizeX, m_nSizeY), NULL, &pLayerCreator);
				CComBSTR bstrName;
				CMultiLanguageString::GetLocalized(L"[0409]layer[0405]vrstva", /*a_tLocaleID*/GetThreadLocale(), &bstrName);
				bstrName += L" 0";
				pFct2->AddLayer(sFrame.bstrID, M_Base(), TRUE, pLayerCreator, NULL, NULL, NULL);
				pFct2->SetSize(sFrame.bstrID, M_Base(), CImageSize(m_nSizeX, m_nSizeY));
			}
			else
			//CComPtr<IDocumentFactoryRasterImage> pImgFct;
			//RWCoCreateInstance(pImgFct, __uuidof(DocumentFactoryLayeredImage/*DocumentFactoryRasterImage*/));
			//if (FAILED(pImgFct->Create(sFrame.bstrID, M_Base(), CImageSize(m_nSizeX, m_nSizeY), NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 2.2f, NULL)))
			{
				m_cFrames.erase(m_cFrames.begin()+nBefore);
				return E_FAIL;
			}
		}
		if (a_ppNewFrame)
			(*a_ppNewFrame = static_cast<IUIItem*>(sFrame.pFrame))->AddRef();

		{ // add observer
			CComPtr<IDocumentImage> pDocImg;
			M_Base()->DataBlockGet(sFrame.bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
			if (pDocImg) pDocImg->ObserverIns(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), sFrame.nID);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentAnimation::FrameInsIntern(ULONG a_nID, BSTR a_bstrID, IDocumentData* a_pData, CComObject<CFrame>* a_pFrame, float a_fTime, size_t a_nBefore)
{
	SFrame sFrame(this, a_nID, a_bstrID, a_fTime, a_pFrame);
	if (UndoEnabled() == S_OK)
	{
		CUndoFrameIns::Add(M_Base(), this, sFrame.nID);
	}
	m_bFramesChange = true;
	M_Base()->DataBlockSet(a_bstrID, a_pData);

	{ // add observer
		CComPtr<IDocumentImage> pDocImg;
		M_Base()->DataBlockGet(sFrame.bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg) pDocImg->ObserverIns(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), sFrame.nID);
	}

	if (a_nBefore < m_cFrames.size())
		m_cFrames.insert(m_cFrames.begin()+a_nBefore, sFrame);
	else
		m_cFrames.push_back(sFrame);
}

STDMETHODIMP CDocumentAnimation::FrameMove(IUnknown* a_pBefore, IUnknown* a_pSrcFrame)
{
	try
	{
		CComQIPtr<IFrame> pSrcFrame(a_pSrcFrame);
		if (pSrcFrame == NULL)
			return E_RW_ITEMNOTFOUND;

		CDocumentWriteLock cLock(this);
		size_t iSrc = m_cFrames.size();
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pSrcFrame)
			{
				iSrc = i-m_cFrames.begin();
				break;
			}
		}
		if (iSrc == m_cFrames.size())
			return E_RW_ITEMNOTFOUND;

		size_t iDst = m_cFrames.size();
		CComQIPtr<IFrame> pFrame(a_pBefore);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pFrame)
			{
				iDst = i-m_cFrames.begin();
				break;
			}
		}

		if (iDst > iSrc)
			--iDst;

		FrameMoveIntern(iSrc, iDst);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentAnimation::FrameMoveIntern(size_t a_iFrom, size_t a_iTo)
{
	if (a_iFrom == a_iTo)
		return;

	if (UndoEnabled() == S_OK)
	{
		CUndoFrameMove::Add(M_Base(), this, a_iFrom, a_iTo);
	}
	m_bFramesChange = true;

	if (a_iFrom > a_iTo)
	{
		for (size_t i = a_iFrom; i > a_iTo; --i)
		{
			std::swap(m_cFrames[i].nID, m_cFrames[i-1].nID);
			std::swap(m_cFrames[i].bstrID.m_str, m_cFrames[i-1].bstrID.m_str);
			std::swap(m_cFrames[i].fTime, m_cFrames[i-1].fTime);
			std::swap(m_cFrames[i].pFrame, m_cFrames[i-1].pFrame);
		}
	}
	else
	{
		for (size_t i = a_iFrom; i < a_iTo; ++i)
		{
			std::swap(m_cFrames[i].nID, m_cFrames[i+1].nID);
			std::swap(m_cFrames[i].bstrID.m_str, m_cFrames[i+1].bstrID.m_str);
			std::swap(m_cFrames[i].fTime, m_cFrames[i+1].fTime);
			std::swap(m_cFrames[i].pFrame, m_cFrames[i+1].pFrame);
		}
	}
}

STDMETHODIMP CDocumentAnimation::FrameGetTime(IUnknown* a_pFrame, float* a_pSeconds)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pFrame)
			{
				*a_pSeconds = i->fTime;
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::FrameGetDoc(IUnknown* a_pFrame, IDocument** a_ppFrameDoc)
{
	try
	{
		*a_ppFrameDoc = NULL;
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL || pFrame->Doc() != this)
			return S_FALSE;
		CDocumentReadLock cLock(this);
		for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->bstrID == pFrame->ID())
			{
				return M_Base()->DataBlockDoc(i->bstrID, a_ppFrameDoc);
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::FrameSetDoc(IUnknown* a_pFrame, IDocument* a_pFrameDoc)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL || pFrame->Doc() != this)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->bstrID == pFrame->ID())
			{
				{ // remove observer
					CComPtr<IDocumentImage> pDocImg;
					M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
					if (pDocImg) pDocImg->ObserverDel(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
				}

				HRESULT hRes = S_OK;
				CComPtr<IDocumentData> pData;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
				M_Base()->DataBlockSet(i->bstrID, NULL);
				if (UndoEnabled() == S_OK)
				{
					CUndoSetDoc::Add(M_Base(), this, i->nID, pData);
				}
				if (FAILED(a_pFrameDoc->DocumentCopy(i->bstrID, M_Base(), NULL, NULL)))
				{
					// put it back
					hRes = E_FAIL;
					M_Base()->DataBlockSet(i->bstrID, pData);
				}
				else
				{
					i->bChanged = true;
				}

				{ // add observer
					CComPtr<IDocumentImage> pDocImg;
					M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
					if (pDocImg) pDocImg->ObserverIns(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
				}
				return hRes;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentAnimation::FrameSetDocIntern(ULONG m_nID, IDocumentData* a_pData)
{
	for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
	{
		if (i->nID == m_nID)
		{
			{ // remove observer
				CComPtr<IDocumentImage> pDocImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
				if (pDocImg) pDocImg->ObserverDel(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
			}

			HRESULT hRes = S_OK;
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			M_Base()->DataBlockSet(i->bstrID, NULL);
			if (UndoEnabled() == S_OK)
			{
				CUndoSetDoc::Add(M_Base(), this, i->nID, pData);
			}
			if (FAILED(M_Base()->DataBlockSet(i->bstrID, a_pData)))
			{
				// put it back
				M_Base()->DataBlockSet(i->bstrID, pData);
			}
			else
			{
				i->bChanged = true;
			}

			{ // add observer
				CComPtr<IDocumentImage> pDocImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
				if (pDocImg) pDocImg->ObserverIns(CObserverImpl<CDocumentAnimation, IImageObserver, TImageChange>::ObserverGet(), i->nID);
			}
			return;
		}
	}
}

STDMETHODIMP CDocumentAnimation::FrameSetTime(IUnknown* a_pFrame, float a_fSeconds)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentWriteLock cLock(this);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pFrame)
			{
				if (UndoEnabled() == S_OK)
				{
					CUndoSetTime::Add(M_Base(), this, a_pFrame, i->fTime);
				}
				m_bTimingChange = true;
				i->fTime = a_fSeconds;
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::TimeUnitInfo(float* a_pSecondsPerUnit, LCID a_tLocaleHint, BSTR* a_pbstrUnitName)
{
	try
	{
		if (a_pSecondsPerUnit)
			*a_pSecondsPerUnit = 0.001f;
		if (a_pbstrUnitName)
		{
			*a_pbstrUnitName = NULL;
			*a_pbstrUnitName = SysAllocString(L"ms");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::LoopCountGet(ULONG* a_pCount)
{
	try
	{
		CDocumentReadLock cLock(this);
		*a_pCount = m_nLoopCount;
		return S_OK;
	}
	catch (...)
	{
		return a_pCount ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentAnimation::LoopCountSet(ULONG a_nCount)
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (a_nCount != m_nLoopCount)
		{
			if (UndoEnabled() == S_OK)
			{
				CUndoLoopCount::Add(M_Base(), this, m_nLoopCount);
			}
			m_bTimingChange = true;
			m_nLoopCount = a_nCount;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
/*
STDMETHODIMP CDocumentAnimation::FrameGetDefaultFormat(IUnknown* a_pFrame, TPixelFormat* a_ptImageFormat)
{
	try
	{
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentReadLock cLock(this);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pFrame)
			{
				CComPtr<IDocumentImage> pImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
				if (pImg)
				{
					TImageSize tSize = {0, 0};
					HRESULT hRes = pImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
					if (FAILED(hRes))
						return hRes;
					a_ptImageFormat->tRed.nWidth = a_ptImageFormat->tGreen.nWidth = a_ptImageFormat->tBlue.nWidth = a_ptImageFormat->tAlpha.nWidth = 8;
					a_ptImageFormat->tRed.nOffset = 0;
					a_ptImageFormat->tGreen.nOffset = 8;
					a_ptImageFormat->tBlue.nOffset = 16;
					a_ptImageFormat->tAlpha.nOffset = 24;
					a_ptImageFormat->nSizeX = tSize.nX;
					a_ptImageFormat->nSizeY = tSize.nY;
					a_ptImageFormat->nStrideX = 4;
					a_ptImageFormat->nStrideY = tSize.nX<<2;
					return S_OK;
				}
				return E_UNEXPECTED;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ptImageFormat ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentAnimation::FrameGetImage(IUnknown* a_pFrame, TPixelFormat const* a_ptImageFormat, ULONG a_nData, BYTE* a_pData)
{
	try
	{
		*a_ppImageSource = NULL;
		CComQIPtr<IFrame> pFrame(a_pFrame);
		if (pFrame == NULL)
			return S_FALSE;
		CDocumentReadLock cLock(this);
		for (CFrames::iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		{
			if (i->pFrame == pFrame)
			{
				CComPtr<IDocumentImage> pImg;
				M_Base()->DataBlockGet(i->bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
				if (pImg)
					return pImg->ImageSourceGet(a_ptImageFormat, a_ppImageSource);
				return E_UNEXPECTED;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ppImageSource ? E_UNEXPECTED : E_POINTER;
	}
}
*/
STDMETHODIMP CDocumentAnimation::StatePack(IEnumUnknowns* a_pFrames, ISharedState** a_ppState)
{
	try
	{
		*a_ppState = NULL;
		CComPtr<ISharedState> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
		if (FAILED(pInit->InsertFromEnum(a_pFrames)))
			return E_FAIL;
		*a_ppState = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppState == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::CanPaste()
{
	static UINT nClipboardFormat = 0;
	if (nClipboardFormat == 0)
		nClipboardFormat = RegisterClipboardFormat(_T("RWLayers"));
	return IsClipboardFormatAvailable(nClipboardFormat) ||
		IsClipboardFormatAvailable(CF_DIBV5) ? S_OK : S_FALSE;
}

#include "RWDocumentAnimationUtils.h"
#include <ImageClipboardUtils.h>

STDMETHODIMP CDocumentAnimation::Paste(RWHWND a_hWindow, IUnknown* a_pBefore, IEnumUnknowns** a_ppNewFrameIDs)
{
	try
	{
		if (a_ppNewFrameIDs) *a_ppNewFrameIDs = NULL;

		CComPtr<IEnumUnknownsInit> pNewFrames;
		RWCoCreateInstance(pNewFrames, __uuidof(EnumUnknowns));

		UINT m_eClipboardFormat = RegisterClipboardFormat(_T("RWLayers"));
		if (m_eClipboardFormat == 0)
			return E_FAIL;

		CClipboardHandler cClipboard(a_hWindow);

		bool bDIBV5 = false;
		bool bCustom = false;
		ULONG iFmt = 0;
		while (true)
		{
			iFmt = EnumClipboardFormats(iFmt);
			if (iFmt == 0)
				break;
			if (iFmt == m_eClipboardFormat)
				bCustom = true;
			if (iFmt == CF_DIBV5)
				bDIBV5 = true;
		}

		if (bCustom)
		{
			HANDLE hMem = GetClipboardData(m_eClipboardFormat);
			if (hMem)
			{
				BYTE const* pData = reinterpret_cast<BYTE const*>(GlobalLock(hMem));
				SIZE_T nSize = GlobalSize(hMem);

				CComObject<CDocumentName>* pName = NULL;
				CComObject<CDocumentName>::CreateInstance(&pName);
				CComPtr<IStorageFilter> pName2 = pName;
				pName->Init(L"pasted.rli");
				CComPtr<IInputManager> pIM;
				RWCoCreateInstance(pIM, __uuidof(InputManager));
				CComPtr<IDocumentBase> pDocBase;
				RWCoCreateInstance(pDocBase, __uuidof(DocumentBase));
				CComQIPtr<IDocument> pDoc;
				CComPtr<IDocumentBuilder> pBuilder;
				RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryAnimation));
				if (SUCCEEDED(pIM->DocumentCreateDataEx(pBuilder, nSize, pData, pName2, NULL, pDocBase, NULL, NULL, NULL)))
					pDoc = pDocBase;
				GlobalUnlock(hMem);
				CComPtr<IDocumentAnimation> pAni;
				if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAni));
				if (pAni)
				{
					CDocumentWriteLock cLock(this);
					CComPtr<IEnumUnknowns> pItems;
					pAni->FramesEnum(&pItems);
					ULONG nItems = 0;
					pItems->Size(&nItems);
					HRESULT hRes = E_FAIL;
					for (ULONG i = 0; i < nItems; ++i)
					{
						CComPtr<IUnknown> pFrame;
						pItems->Get(i, &pFrame);
						CComPtr<IDocument> pDoc;
						pAni->FrameGetDoc(pFrame, &pDoc);
						float fTime = 0.1f;
						pAni->FrameGetTime(pFrame, &fTime);
						CComPtr<IUnknown> pNew;
						hRes = FrameIns(a_pBefore, &CAnimationFrameCreatorDocument(pDoc), &pNew);
						if (FAILED(hRes))
							return hRes;
						FrameSetTime(pNew, fTime);
						pNewFrames->Insert(pNew);
					}

					if (a_ppNewFrameIDs) *a_ppNewFrameIDs = pNewFrames.Detach();

					return hRes;
				}
			}
		}

		if (bDIBV5)
		{
			CAutoVectorPtr<TPixelChannel> cBuffer;
			TImageSize tSize = {0, 0};
			if (GetClipboardImage(tSize, cBuffer))
			{
				CDocumentWriteLock cLock(this);
				CComPtr<IUnknown> pNew;
				CChannelDefault cChDef(EICIRGBA, 0, 0, 0, 0);
				CImageTile cTile(tSize.nX, tSize.nY, cBuffer);
				HRESULT hRes = FrameIns(a_pBefore, &CAnimationFrameCreatorRasterImage(&tSize, NULL, 1, cChDef, 0.0f, cTile), &pNew);
				pNewFrames->Insert(pNew);

				//CComPtr<IComparable> pItem;
				//HRESULT hRes = LayerInsert(NULL, &CImageLayerCreatorRasterImage(tSize, nSizeX*nSizeY, reinterpret_cast<TRasterImagePixel const*>(cBuffer.m_p)), &pItem);
				//if (SUCCEEDED(hRes) && pItem)
				//{
				//	if (a_ppNewState)
				//	{
				//		CComPtr<ISharedState> pTmp;
				//		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
				//		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
				//		pInit->Insert(pItem);
				//		*a_ppNewState = pTmp.Detach();
				//	}
				//	wchar_t szName[64] = L"";
				//	Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_LAYER_PASTED, szName, itemsof(szName), LANGIDFROMLCID(a_tLocaleID));
				//	LayerNameSet(pItem, CComBSTR(szName));
				//}

				if (a_ppNewFrameIDs) *a_ppNewFrameIDs = pNewFrames.Detach();

				return hRes;
			}
		}

		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ReturnedData.h>

STDMETHODIMP CDocumentAnimation::Copy(RWHWND a_hWindow, ULONG a_nFrames, IUnknown** a_apFrames)
{
	try
	{
		UINT m_eClipboardFormat = RegisterClipboardFormat(_T("RWLayers"));
		if (m_eClipboardFormat == 0)
			return E_FAIL;

		CClipboardHandler cClipboard(a_hWindow);
		EmptyClipboard();

		CComPtr<IEnumUnknownsInit> pNewItems;
		RWCoCreateInstance(pNewItems, __uuidof(EnumUnknowns));

		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		CComObject<CDocumentAnimation>* pDoc = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		pBase->DataBlockSet(NULL, pDoc);
		for (ULONG i = 0; i < a_nFrames; ++i)
		{
			CComPtr<IDocument> pFrmDoc;
			FrameGetDoc(a_apFrames[i], &pFrmDoc);
			if (pFrmDoc == NULL) continue;
			CComPtr<IUnknown> pNew;
			pDoc->FrameIns(NULL, &CAnimationFrameCreatorDocument(pFrmDoc), &pNew);
			float fTime = 0.1f;
			FrameGetTime(a_apFrames[i], &fTime);
			pDoc->FrameSetTime(pNew, fTime);
		}
		
		CComQIPtr<IDocument> pDoc2(pBase);
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComBSTR bstrRLI(L"{72D10CF5-E9DF-43D2-9B3D-5433B70CE98B}");
		float const fWeight = 1.0f;
		GUID tID = GUID_NULL;
		CComPtr<IConfig> pCfg;
		pIM->FindBestEncoderEx(pDoc2, 1, &(bstrRLI.m_str), &fWeight, &tID, &pCfg);
		CComPtr<IDocumentEncoder> pEnc;
		if (IsEqualGUID(tID, GUID_NULL) || FAILED(RWCoCreateInstance(pEnc, tID)))
			return E_FAIL;
		CReturnedData cData;
		pEnc->Serialize(pDoc2, pCfg, &cData, NULL, NULL);

		HANDLE hMem = GlobalAlloc(GHND, cData.size());
		BYTE* pData = reinterpret_cast<BYTE*>(GlobalLock(hMem));
		CopyMemory(pData, cData.begin(), cData.size());
		GlobalUnlock(hMem);
		SetClipboardData(m_eClipboardFormat, hMem);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentAnimation::CanDrop(IEnumStrings* a_pFiles)
{
	if (a_pFiles == NULL)
		return S_FALSE;
	try
	{
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders1;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentAnimation), &pBuilders1);
		CComPtr<IEnumUnknowns> pDocTypes1;
		pIM->DocumentTypesEnumEx(pBuilders1, &pDocTypes1);
		ULONG nTypes1 = 0;
		pDocTypes1->Size(&nTypes1);
		CComPtr<IEnumUnknowns> pBuilders2;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders2);
		CComPtr<IEnumUnknowns> pDocTypes2;
		pIM->DocumentTypesEnumEx(pBuilders2, &pDocTypes2);
		ULONG nTypes2 = 0;
		pDocTypes2->Size(&nTypes2);

		ULONG nItems = 0;
		a_pFiles->Size(&nItems);
		for (ULONG j = 0; j < nItems; ++j)
		{
			CComBSTR bstrName;
			a_pFiles->Get(0, &bstrName);
			if (bstrName.Length() == 0)
				continue;
			for (ULONG i = 0; i < nTypes1; ++i)
			{
				CComPtr<IDocumentType> pType;
				pDocTypes1->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pType));
				if (pType->MatchFilename(bstrName) == S_OK)
					return S_OK;
			}
			for (ULONG i = 0; i < nTypes2; ++i)
			{
				CComPtr<IDocumentType> pType;
				pDocTypes2->Get(i, __uuidof(IDocumentType), reinterpret_cast<void**>(&pType));
				if (pType->MatchFilename(bstrName) == S_OK)
					return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool CDocumentAnimation::AppendFrame(IDocument* a_pFrame, float a_fSeconds, IEnumUnknownsInit* a_pNewFrames)
{
	CComPtr<IDocumentLayeredImage> pDLI;
	a_pFrame->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
	if (pDLI == NULL)
		return false;
	CComPtr<IUnknown> pNew;
	FrameIns(NULL, NULL, &pNew);
	if (pNew == NULL)
		return false;
	if (FAILED(FrameSetDoc(pNew, a_pFrame)))
		return false;
	if (a_fSeconds >= 0.0f)
		FrameSetTime(pNew, a_fSeconds);
	if (a_pNewFrames)
		a_pNewFrames->Insert(pNew);
	return true;
}

STDMETHODIMP CDocumentAnimation::Drop(IEnumStrings* a_pFiles, IUnknown* a_pBefore, IEnumUnknowns** a_ppNewFrameIDs)
{
	try
	{
		if (a_ppNewFrameIDs) *a_ppNewFrameIDs = NULL;

		CComPtr<IEnumUnknownsInit> pNewFrames;
		RWCoCreateInstance(pNewFrames, __uuidof(EnumUnknowns));

		ULONG nItems = 0;
		a_pFiles->Size(&nItems);
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CDocumentWriteLock cLock(this);
		HRESULT hRes = S_FALSE;
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComBSTR bstr;
			a_pFiles->Get(i, &bstr);
			CStorageFilter cInPath(bstr);
			// is it animation
			CComPtr<IDocumentBuilder> pBuilder;
			RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryAnimation));
			CComPtr<IDocument> pDoc;
			pIM->DocumentCreateEx(pBuilder, cInPath, NULL, &pDoc);
			if (pDoc)
			{
				// add frames from animation
				CComPtr<IDocumentAnimation> pAni;
				pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAni));
				CComPtr<IEnumUnknowns> pFrames;
				pAni->FramesEnum(&pFrames);
				CComPtr<IUnknown> pFrame;
				if (pFrames) for (ULONG i = 0; SUCCEEDED(pFrames->Get(i, &pFrame)); ++i, pFrame = NULL)
				{
					float fTime = -1.0f;
					pAni->FrameGetTime(pFrame, &fTime);
					CComPtr<IDocument> pDoc;
					pAni->FrameGetDoc(pFrame, &pDoc);
					if (AppendFrame(pDoc, fTime, pNewFrames))
					{
						hRes = S_OK;
					}
				}
			}
			//else
			//{
			//	CComPtr<IEnumUnknowns> pBuilders2;
			//	pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders2);
			//	pIM->DocumentCreateEx(pBuilders2, cInPath, NULL, &pDoc);
			//	if (pDoc && AppendFrame(pDoc))
			//		hRes = S_OK;
			//}
		}

		if (a_ppNewFrameIDs) *a_ppNewFrameIDs = pNewFrames.Detach();

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

