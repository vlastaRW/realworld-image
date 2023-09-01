// DocumentLayeredImage.cpp : Implementation of CDocumentLayeredImage

#include "stdafx.h"
#include "DocumentLayeredImage.h"
#include "StructuredItemLayer.h"
#include "StructuredItemLayerEffect.h"
#include "DocumentLayeredImageUndo.h"
#include "DocumentRasterImage.h"
#include <DocumentName.h>
#include <MultiLanguageString.h>
#include <RWImagingDocumentUtils.h>
#include "DocumentLayeredImageCache.h"

static OLECHAR const CFGID_LAYEREFFECT[] = L"Effect";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUILayoutEffectDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILayoutEffectDlg>,
	public CDialogResize<CConfigGUILayoutEffectDlg>
{
public:
	BEGIN_DIALOG_EX(0, 0, 160, 80, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
		DIALOG_EXSTYLE(WS_EX_CONTEXTHELP | WS_EX_CONTROLPARENT)
	END_DIALOG()

	enum
	{
		IDC_LE_OPERATION = 100,
		IDC_LE_CONFIG,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Operation:[0405]Operace:"), IDC_STATIC, 0, 2, 51, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_LE_OPERATION, 53, 0, 106, 170, WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, 0);
		CONTROL_CONTROL(_T(""), IDC_LE_CONFIG, WC_STATIC, SS_BLACKRECT | WS_VISIBLE, 0, 16, 159, 64, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILayoutEffectDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayoutEffectDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILayoutEffectDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayoutEffectDlg)
		DLGRESIZE_CONTROL(IDC_LE_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LE_CONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayoutEffectDlg)
		CONFIGITEM_COMBOBOX(IDC_LE_OPERATION, CFGID_LAYEREFFECT)
		CONFIGITEM_SUBCONFIG_NOMARGINS(IDC_LE_CONFIG, CFGID_LAYEREFFECT)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

void CDocumentLayeredImage::SLayer::Init(CDocumentLayeredImage* a_pThis, ULONG a_nUID, IOperationManager* a_pMgr)
{
	nUID = a_nUID;
	nTimestamp = 0;
	pEffects = NULL;
	pDocImg = NULL;
	pDocGrp = NULL;
	bVisible = true;
	eBlendingMode = EBEAlphaBlend;
	pEffects = new CLayerEffects;

	CComObject<CStructuredItemLayer>* p = NULL;
	CComObject<CStructuredItemLayer>::CreateInstance(&p);
	pItem = p;
	p->Init(a_pThis, nUID);
}

void CDocumentLayeredImage::SLayer::Release(IImageObserver* a_pImageObserver, IStructuredObserver* a_pStructuredObserver)
{
	if (pDocImg)
	{
		pDocImg->ObserverDel(a_pImageObserver, nUID);
		pDocImg->Release();
	}
	if (pDocGrp)
	{
		pDocGrp->ObserverDel(a_pStructuredObserver, nUID);
		pDocGrp->Release();
	}
	if (pItem)
		static_cast<CComObject<CStructuredItemLayer>*>(pItem.p)->Init(NULL, 0);
	delete pEffects;
}

void CDocumentLayeredImage::SLayerEffect::CleanUp()
{
	if (pOp)
		pOp->Release();
	if (pOpCfg)
		pOpCfg->Release();
	if (pItem)
		pItem->ReleaseAndUnlink();
}

void CDocumentLayeredImage::SLayerEffect::swap(SLayerEffect& rhs)
{
	std::swap(tOpID, rhs.tOpID);
	std::swap(pOp, rhs.pOp);
	std::swap(pOpCfg, rhs.pOpCfg);
	std::swap(bEnabled, rhs.bEnabled);
	std::swap(pItem, rhs.pItem);
}

CDocumentLayeredImage::CLayerEffects::~CLayerEffects()
{
	for (iterator i = begin(); i != end(); ++i)
		i->CleanUp();
}


// CDocumentLayeredImage

CDocumentLayeredImage::~CDocumentLayeredImage()
{
	for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		i->Release(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet());

	ATLASSERT(m_aComposedObservers.empty());
	for (std::vector<SComposedObserver>::iterator i = m_aComposedObservers.begin(); i != m_aComposedObservers.end(); ++i)
		i->pObserver->Release();
}

STDMETHODIMP CDocumentLayeredImage::WriteFinished()
{
	std::map<ULONG, ULONG> cAccumChg;
	bool bBel = false;
	CStructuredChanges cSubDocChg;
	ULONG nImageChanges = 0;
	if (m_bLayersChange)
	{
		cSubDocChg.Add(this, ESCChildren);
		nImageChanges |= EICContent;
	}
	DWORD dwMetaDataChange = GetMetaDataChanges();
	if (m_bLayersChange || !m_cLayerChanges.empty() || dwMetaDataChange)
		M_Base()->SetDirty(); // set dirty flag
	for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
	{
		CComBSTR bstr;
		i->GetLayerID(this, bstr);
		CComPtr<IDocumentData> pData;
		M_Base()->DataBlockGet(bstr, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
		if (pData)
			pData->WriteFinished();
		ULONG& nChg = cAccumChg[i->nUID];
		nChg = m_bLayersChange ? ECPCUnder|ECPCOver : (bBel ? ECPCUnder : 0); // TODO: optimize layer move and delete change notifications
		CLayerChanges::const_iterator iChg = m_cLayerChanges.find(i->nUID);
		if (iChg != m_cLayerChanges.end())
		{
			if (iChg->second.nChanges&ECSLayerMeta)
			{
				cSubDocChg.Add<CStructuredItemLayer>(this, iChg->second.nChanges&(~ECSLayerMeta)|ESCChildren, iChg->first);
				bBel = true;
				nChg |= ECPCCurrent;
				nImageChanges |= EICContent;
				for (std::map<ULONG, ULONG>::iterator ii = cAccumChg.begin(); ii != cAccumChg.end(); ++ii)
					if (ii->first != i->nUID) ii->second |= ECPCOver;
				//ULONG const nSteps = GetStyleStepCount(i->pEffect);
				//for (ULONG j = 0; j < nSteps; ++j)
					//cSubDocChg.Add<CStructuredItemLayerEffect>(this, ESCContent|ESCGUIRepresentation, iChg->first, j);
				//for (ULONG j = 0; j < nSteps; ++j)
				// TODO: only send changes to layer effect that were actualy changed
				for (CLayerEffects::const_iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
				{
					cAccumChg[j->pItem->EffectID()] = ELECOpCfg|ELECOpID|ELECState;
					cSubDocChg.AddItem(ESCContent|ESCGUIRepresentation, static_cast<IUIItem*>(j->pItem));
				}
			}
			else
			{
				cSubDocChg.Add<CStructuredItemLayer>(this, iChg->second.nChanges, iChg->first);
			}
		}
	}
	if (m_bSizeChange)
	{
		m_bSizeChange = false;
		nImageChanges |= EICDimensions;
	}

	HRESULT hRes = S_FALSE; // no changes

	{
	CComCritSecLock<CComAutoCriticalSection> cLock(m_cComposedCS); // is it needed ?
	// prevent iterator invalidation during Notify calls
	ATLASSERT(0 > (int)m_iComposedActNotifying);// Fire_Notify is not reentrant (the behavior is affected while in execution)
	m_iComposedMaxNotifying = m_aComposedObservers.size();
	for (m_iComposedActNotifying = 0; m_iComposedActNotifying < m_iComposedMaxNotifying; ++m_iComposedActNotifying)
	{
		std::map<ULONG, ULONG>::const_iterator j = cAccumChg.find(m_aComposedObservers[m_iComposedActNotifying].nLayerID);
		if (j != cAccumChg.end() && j->second)
		{
			m_aComposedObservers[m_iComposedActNotifying].pObserver->Notify(m_aComposedObservers[m_iComposedActNotifying].tCookie, j->second);
			hRes = S_OK;
		}
	}
#ifdef _DEBUG
	m_iComposedActNotifying = static_cast<size_t>(-1);
#endif
	}

	{
	CComCritSecLock<CComAutoCriticalSection> cLock(m_cEffectCS); // is it needed ?
	// prevent iterator invalidation during Notify calls
	ATLASSERT(0 > (int)m_iEffectActNotifying);// Fire_Notify is not reentrant (the behavior is affected while in execution)
	m_iEffectMaxNotifying = m_aEffectObservers.size();
	for (m_iEffectActNotifying = 0; m_iEffectActNotifying < m_iEffectMaxNotifying; ++m_iEffectActNotifying)
	{
		std::map<ULONG, ULONG>::const_iterator j = cAccumChg.find(m_aEffectObservers[m_iEffectActNotifying].nEffectID);
		if (j != cAccumChg.end() && j->second)
		{
			m_aEffectObservers[m_iEffectActNotifying].pObserver->Notify(m_aEffectObservers[m_iEffectActNotifying].tCookie, j->second);
			hRes = S_OK;
		}
	}
#ifdef _DEBUG
	m_iEffectActNotifying = static_cast<size_t>(-1);
#endif
	}

	m_cLayerChanges.clear();
	m_bLayersChange = false;

	cSubDocChg.insert(cSubDocChg.end(), m_cSubDocChanges.begin(), m_cSubDocChanges.end());
	m_cSubDocChanges.clear();

	TStructuredChanges tChanges;
	tChanges.nChanges = cSubDocChg.size();
	if (tChanges.nChanges != 0)
	{
		tChanges.aChanges = &(cSubDocChg[0]);
		CSubjectImpl<CStructuredRootImpl<CDocumentLayeredImage, IDocumentLayeredImage>, IStructuredObserver, TStructuredChanges>::Fire_Notify(tChanges);
		hRes = S_OK;
	}

	if (nImageChanges || dwMetaDataChange)
	{
		TImageChange tChg;
		tChg.nGlobalFlags = nImageChanges|dwMetaDataChange;
		tChg.tOrigin.nX = tChg.tOrigin.nY = 0;
		tChg.tSize = m_tSize;
		CSubjectImpl<IDocumentEditableImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
		hRes = S_OK;
	}

	M_Base()->RefreshEncoder();
	//if ((M_DataID() == NULL || *M_DataID() == L'\0') && m_dwFormatID != EImgFmtRAW)
	//{
	//	CComPtr<IStorageFilter> pFlt;
	//	M_Base()->LocationGet(&pFlt);
	//	CComQIPtr<IDocumentName> pName(pFlt);
	//	if (pName && (m_bRLI != (m_cLayers.size() > 1)))
	//	{
	//		m_bRLI = (m_cLayers.size() > 1);
	//		CComPtr<IStorageFilter> pNew;
	//		if (m_bRLI)
	//		{
	//			CDocumentName::ChangeExtension(pFlt, L"rli", &pNew);
	//		}
	//		else
	//		{
	//			CComPtr<IDocumentType> pInternal;
	//			CComPtr<IImageEncoderManager> pEncMgr;
	//			RWCoCreateInstance(pEncMgr, __uuidof(ImageEncoderManager));
	//			pEncMgr->GetFormatProperties(m_dwFormatID, &pInternal);
	//			CComBSTR bstrDefExt;
	//			pInternal->DefaultExtensionGet(&bstrDefExt);
	//			CDocumentName::ChangeExtension(pFlt, bstrDefExt.m_str, &pNew);
	//		}
	//		M_Base()->LocationSet(pNew);
	//	}
	//}

	return hRes;
}

void CDocumentLayeredImage::OwnerNotify(TCookie a_tCookie, TImageChange a_tChange)
{
	try
	{
		if (a_tCookie == EDocumentCookie)
		{
			m_bNonImageChange = true;
		}
		else
		{
			for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			{
				if (i->nUID == a_tCookie)
				{
					++i->nTimestamp;
					DeleteCacheForLayer(a_tCookie);
					break;
				}
			}
			//DeleteCachedData();
			//CSubjectImpl<IDocumentImage, IImageObserver, ULONG>::Fire_Notify(a_nParam);
			m_cLayerChanges[a_tCookie].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta;
		}
	}
	catch (...)
	{
		// TODO: log?
	}
}

void CDocumentLayeredImage::OwnerNotify(TCookie a_tCookie, TStructuredChanges a_tParam)
{
	try
	{
		for (ULONG i = 0; i < a_tParam.nChanges; ++i)
		{
			if (a_tParam.aChanges[i].pItem)
			{
				m_cSubDocChanges.push_back(a_tParam.aChanges[i]);
				a_tParam.aChanges[i].pItem->AddRef();
			}
			else
			{
				// TODO: mark layer changed - children?
			}
		}
	}
	catch (...)
	{
		// TODO: log?
	}
}


class CImageLayerCreatorRescaledImage :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorRescaledImage(IDocument* a_pDoc, ULONG a_nFactor, ULONG a_nSizeX, ULONG a_nSizeY) :
		m_pDoc(a_pDoc), m_nFactor(a_nFactor), m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY)
	{
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		try
		{
			CComPtr<IDocumentImage> pI;
			m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
			CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[m_nSizeX*m_nSizeY]);
			CNearestImageResizer::GetResizedImage(pI, EICIRGBA, m_nSizeX, m_nSizeY, 1, m_nSizeX, cBuffer);

			CComObject<CDocumentRasterImage>* p = NULL;
			CComObject<CDocumentRasterImage>::CreateInstance(&p);
			CComPtr<IDocumentData> pTmp = p;
			TImageSize tSize = {m_nSizeX, m_nSizeY};
			TImagePoint t0 = {0, 0};
			p->Init(tSize, NULL, NULL, t0, tSize, cBuffer, 2.2f, NULL, NULL, m_nFactor);
			return a_pBase->DataBlockSet(a_bstrID, pTmp);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	ULONG m_nFactor;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
};

STDMETHODIMP CDocumentLayeredImage::DataCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		//CDocumentWriteLock cLock(this); // hack - preventing lock problems with part-of-self copies

		CComObject<CDocumentLayeredImage>* p = NULL;
		CComObject<CDocumentLayeredImage>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		CDocumentLayeredImage* pDoc = p;

		pDoc->m_nClipboardLayers = m_nClipboardLayers;
		pDoc->m_nClipboardEffects = m_nClipboardEffects;
		pDoc->m_pOpMgr = m_pOpMgr;
		pDoc->m_tSize = m_tSize;

		ULONG nMulSize = m_tSize.nX*m_tSize.nY;
		int nFactor = 1;
		if (a_tPreviewEffectID)
		{
			TImageSize t = {m_tSize.nX, m_tSize.nY};
			while (t.nX*t.nY > 0x50000)
			{
				t.nX = (t.nX+1)>>1;
				t.nY = (t.nY+1)>>1;
				nFactor <<= 1;
			}
			pDoc->m_tSize = t;
		}
		TMatrix3x3f trans =
		{
			1.0f/nFactor, 0, 0,
			0, 1.0f/nFactor, 0,
			0, 0, 1
		};
		if (nFactor > 1)
		{
			if (a_pPreviewEffect)
			{
				CComPtr<IRasterImageFilter> pRIF;
				RWCoCreateInstance(pRIF, *a_tPreviewEffectID);
				if (pRIF)
					pRIF->Transform(a_pPreviewEffect, &pDoc->m_tSize, &trans);
			}
		}

		a_pBase->DataBlockSet(a_bstrPrefix, pTmp);

		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			CComBSTR bstrID;
			i->GetLayerID(this, bstrID);
			CComPtr<IDocument> pLayDoc;
			M_Base()->DataBlockDoc(bstrID, &pLayDoc);
			CComPtr<IComparable> pLayer;
			if (a_tPreviewEffectID == NULL || nFactor == 1)
			{
				pDoc->LayerInsert(NULL, ELIPDefault, &CImageLayerCreatorDocument(pLayDoc), &pLayer);
				CComQIPtr<IStructuredItemLayerItem> pSILI(pLayer);
				BYTE b = i->bVisible;
				pDoc->LayerPropsSet(pLayer, &(i->eBlendingMode), &b);
				if (i->pEffects) pDoc->LayerEffectSet(pSILI->ID(pDoc), *i->pEffects, false);
				pDoc->LayerNameSet(pLayer, CComBSTR(i->strName.c_str()));
			}
			else
			{
				// change layer to raster image and rescale
				pDoc->LayerInsert(NULL, ELIPDefault, &CImageLayerCreatorRescaledImage(pLayDoc, nFactor, (m_tSize.nX+nFactor-1)/nFactor, (m_tSize.nY+nFactor-1)/nFactor), &pLayer);
				CComQIPtr<IStructuredItemLayerItem> pSILI(pLayer);
				BYTE b = i->bVisible;
				pDoc->LayerPropsSet(pLayer, &(i->eBlendingMode), &b);
				CLayerEffects cEffects;
				cEffects.resize(i->pEffects->size());
				for (size_t j = 0; j != cEffects.size(); ++j)
				{
					SLayerEffect const& s = (*i->pEffects)[j];
					SLayerEffect& d = cEffects[j];
					d.bEnabled = s.bEnabled;
					d.tOpID = s.tOpID;
					RWCoCreateInstance(d.tOpID, NULL, CLSCTX_ALL, __uuidof(IDocumentOperation), reinterpret_cast<void**>(&d.pOp));
					if (d.pOp)
						d.pOp->ConfigCreate(M_OpMgr(), &d.pOpCfg);
					CopyConfigValues(d.pOpCfg, s.pOpCfg);
					CComQIPtr<IRasterImageFilter> pRIF(d.pOp);
					if (pRIF)
						pRIF->Transform(a_pPreviewEffect, &pDoc->m_tSize, &trans);
					d.pItem = new CStructuredItemLayerEffect(pDoc, pDoc->m_nNextID, InterlockedIncrement(&pDoc->m_nNextID));
				}

				if (i->pEffects) pDoc->LayerEffectSet(pSILI->ID(pDoc), cEffects, true);
				pDoc->LayerNameSet(pLayer, CComBSTR(i->strName.c_str()));
			}
		}

		if (M_MetaData())
			pDoc->CopyMetaData(M_MetaData());

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>

STDMETHODIMP CDocumentLayeredImage::QuickInfo(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
{
	try
	{
		if (a_nInfoIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		*a_ppInfo = NULL;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CMultiLanguageString(L"[0409]%ix%i pixels[0405]%ix%i pixelů"));
		CComObject<CPrintfLocalizedString>* pPFStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pPFStr);
		CComPtr<ILocalizedString> pStr = pPFStr;
		pPFStr->Init(pTempl, m_tSize.nX, m_tSize.nY);
		*a_ppInfo = pStr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
	return E_NOTIMPL;
}


class ATL_NO_VTABLE CLayerComposedPreview : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageComposedPreview
{
public:
	CLayerComposedPreview() : m_pDoc(NULL)
	{
	}
	~CLayerComposedPreview()
	{
		if (m_pDoc) m_pDoc->Release();
	}
	void Init(CDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, IRasterImageComposedPreview* a_pOrig)
	{
		(m_pDoc = a_pDoc)->AddRef();
		m_nLayerID = a_nLayerID;
		m_pOrig.Attach(a_pOrig);
	}

BEGIN_COM_MAP(CLayerComposedPreview)
	COM_INTERFACE_ENTRY(IRasterImageComposedPreview)
END_COM_MAP()

	// IRasterImageComposedPreview methods
public:
	STDMETHOD(AdjustDirtyRect)(RECT* a_prc)
	{
		if (m_pOrig) m_pOrig->AdjustDirtyRect(a_prc);
		return m_pDoc->ComposedPreviewAdjustTile(m_nLayerID, true, a_prc);
	}
	STDMETHOD(PreProcessTile)(RECT* a_prc)
	{
		if (m_pOrig) m_pOrig->PreProcessTile(a_prc);
		return m_pDoc->ComposedPreviewAdjustTile(m_nLayerID, false, a_prc);
	}
	STDMETHOD(ProcessTile)(EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData)
	{
		if (m_pOrig) m_pOrig->ProcessTile(a_eMode, a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pData);
		return m_pDoc->ComposedPreviewProcessTile(m_nLayerID, a_eMode, a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pData);
	}
	STDMETHOD(InputTransform)(TMatrix3x3f* a_pTransform)
	{
		if (m_pOrig) m_pOrig->InputTransform(a_pTransform);
		return m_pDoc->ComposedPreviewInputTransform(m_nLayerID, a_pTransform);
	}
	STDMETHOD(ObserverIns)(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie)
	{
		if (m_pOrig) m_pOrig->ObserverIns(a_pObserver, a_tCookie);
		return m_pDoc->ComposedObserverIns(a_pObserver, a_tCookie, m_nLayerID);
	}
	STDMETHOD(ObserverDel)(IComposedPreviewObserver* a_pObserver, TCookie a_tCookie)
	{
		if (m_pOrig) m_pOrig->ObserverDel(a_pObserver, a_tCookie);
		return m_pDoc->ComposedObserverDel(a_pObserver, a_tCookie, m_nLayerID);
	}

private:
	CDocumentLayeredImage* m_pDoc;
	ULONG m_nLayerID;
	CComPtr<IRasterImageComposedPreview> m_pOrig;
};

class ATL_NO_VTABLE CLayerCanvasSize : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDocumentRasterImage
{
public:
	CLayerCanvasSize() : m_pDoc(NULL)
	{
	}
	~CLayerCanvasSize()
	{
		if (m_pDoc) m_pDoc->Release();
	}
	void Init(CDocumentLayeredImage* a_pDoc, TImageSize* a_pSize, IDocumentRasterImage* a_pOrig)
	{
		(m_pDoc = a_pDoc)->AddRef();
		m_pSize = a_pSize;
		m_pOrig.Attach(a_pOrig);
	}

BEGIN_COM_MAP(CLayerCanvasSize)
	COM_INTERFACE_ENTRY(IDocumentImage)
	COM_INTERFACE_ENTRY(IDocumentEditableImage)
	COM_INTERFACE_ENTRY(IDocumentRasterImage)
END_COM_MAP()

	// IDocumentImage methods
public:
	STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
	{
		if (a_pCanvasSize)
			*a_pCanvasSize = *m_pSize;
		return m_pOrig->CanvasGet(NULL, a_pResolution, a_pContentOrigin, a_pContentSize, a_pContentOpacity);
	}
	STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
	{ return m_pOrig->ChannelsGet(a_pChannelIDs, a_pGamma, a_pChannelDefaults); }
	STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pPixels, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pOrig->TileGet(a_nChannelIDs, a_pOrigin, a_pSize ? a_pSize : m_pSize, a_pStride, a_nPixels, a_pPixels, a_pControl, a_eIntent); }
	STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pOrig->Inspect(a_nChannelIDs, a_pOrigin, a_pSize ? a_pSize : m_pSize, a_pVisitor, a_pControl, a_eIntent); }
	STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
	{ return m_pOrig->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent); }
	STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
	{ return m_pOrig->BufferUnlock(a_nChannelID, a_pBuffer); }

	STDMETHOD(ObserverIns)(IImageObserver* a_pObserver, TCookie a_tCookie)
	{ return m_pOrig->ObserverIns(a_pObserver, a_tCookie); }
	STDMETHOD(ObserverDel)(IImageObserver* a_pObserver, TCookie a_tCookie)
	{ return m_pOrig->ObserverDel(a_pObserver, a_tCookie); }

	// IDocumentEditableImage methods
public:
	STDMETHOD(CanvasSet)(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
	{
		if (a_pSize)
			return m_pDoc->CanvasSet(a_pSize, a_pResolution, a_pContentTransform, a_pHelper);
		return m_pOrig->CanvasSet(a_pSize, a_pResolution, a_pContentTransform, a_pHelper);
	}
	STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
	{ return m_pOrig->ChannelsSet(a_nChannels, a_aChannelIDs, a_aChannelDefaults); }

	// IDocumentRasterImage methods
public:
	STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
	{ return m_pOrig->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, a_bDeleteOldContent); }
	STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
	{ return m_pOrig->BufferReplace(a_tAllocOrigin, a_tAllocSize, a_pContentOrigin, a_pContentSize, a_pContentAlphaSum, a_pPixels, a_pDeleter); }
	STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
	{ return m_pOrig->BufferAllocate(a_tSize, a_ppPixels, a_ppDeleter); }

private:
	CDocumentLayeredImage* m_pDoc;
	TImageSize* m_pSize; // HACK: points to m_pDoc's member (must keep m_pDoc alive)
	CComPtr<IDocumentRasterImage> m_pOrig;
};

STDMETHODIMP CDocumentLayeredImage::ComponentFeatureOverride(BSTR a_bstrID, REFIID a_iid, void** a_ppFeatureInterface)
{
	try
	{
		BSTR bstrBase = M_DataID();
		int nBaseLen = bstrBase ? wcslen(bstrBase) : 0;
		int nCompLen = a_bstrID ? wcslen(a_bstrID) : 0;
		LPWSTR pszEnd;
		if (nCompLen > nBaseLen && a_bstrID[nBaseLen] == L'L' && (pszEnd=wcschr(a_bstrID+nBaseLen+1, L';')))
		{
			ULONG nID = _wtoi(a_bstrID+nBaseLen+1);
			if (nID)
			{
				if (pszEnd[1])
				{
					// quering an interface for subitem
					CComBSTR bstrLayerID;
					bstrLayerID.Attach(SysAllocStringLen(a_bstrID, pszEnd-a_bstrID+1));
					CComPtr<IDocumentData> pSubDoc;
					M_Base()->DataBlockGet(bstrLayerID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pSubDoc));
					if (pSubDoc)
						pSubDoc->ComponentFeatureOverride(a_bstrID, a_iid, a_ppFeatureInterface);
				}
				if (IsEqualIID(a_iid, __uuidof(IRasterImageComposedPreview)))
				{
					CComObject<CLayerComposedPreview>* pWrapper = NULL;
					CComObject<CLayerComposedPreview>::CreateInstance(&pWrapper);
					CComPtr<IRasterImageComposedPreview> pDoc = pWrapper;
					pWrapper->Init(this, nID, reinterpret_cast<IRasterImageComposedPreview*>(*a_ppFeatureInterface));
					*a_ppFeatureInterface = pDoc.Detach();
				}
				else if (*a_ppFeatureInterface && (IsEqualIID(a_iid, __uuidof(IDocumentImage)) || IsEqualIID(a_iid, __uuidof(IDocumentEditableImage)) || IsEqualIID(a_iid, __uuidof(IDocumentRasterImage))))
				{
					CComObject<CLayerCanvasSize>* pWrapper = NULL;
					CComObject<CLayerCanvasSize>::CreateInstance(&pWrapper);
					CComPtr<IDocumentRasterImage> pDoc = pWrapper;
					pWrapper->Init(this, &m_tSize, reinterpret_cast<IDocumentRasterImage*>(*a_ppFeatureInterface));
					*a_ppFeatureInterface = pDoc.Detach();
				}
				else if (IsEqualIID(a_iid, __uuidof(IImageMetaData)))
				{
					if (*a_ppFeatureInterface)
						reinterpret_cast<IUnknown*>(*a_ppFeatureInterface)->Release();
					*a_ppFeatureInterface = static_cast<IImageMetaData*>(this);
					AddRef();
				}
			}
		}
		return *a_ppFeatureInterface ? S_OK : E_NOINTERFACE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::RemovingBlock()
{
	try
	{
		CDocumentWriteLock cLock(this);
		CComPtr<IEnumUnknowns> pItems;
		ItemsEnum(NULL, &pItems);
		CComPtr<IComparable> pItem;
		for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); ++i, pItem = NULL)
		{
			LayerDelete(pItem);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::MaximumUndoSize(ULONGLONG* a_pnMaximumSize)
{
	ULONGLONG n = m_tSize.nX*m_tSize.nY;
	n *= 4*8;
	*a_pnMaximumSize = min(0x10000000, n);
	return S_OK;
}

STDMETHODIMP CDocumentLayeredImage::ResourcesManage(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue)
{
	try
	{
		ULONGLONG n = 256; // some base value
		if (a_eActions & EDRMMinimizeUsage)
		{
			m_cLayerCS.Lock();
			for (CLayerCache::const_iterator i = m_cLayerCache.begin(); i != m_cLayerCache.end(); ++i)
			{
				if (i->second->pImage)
					n += (i->second->tBounds.tBR.nX-i->second->tBounds.tTL.nX)*(i->second->tBounds.tBR.nY-i->second->tBounds.tTL.nY)*4;
			}
			m_cLayerCache.clear();
			m_cLayerCS.Unlock();

			m_cMergedCS.Lock();
			for (CMergedCache::const_iterator i = m_cMergedCache.begin(); i != m_cMergedCache.end(); ++i)
			{
				if (i->second->pImage)
					n += (i->second->tBounds.tBR.nX-i->second->tBounds.tTL.nX)*(i->second->tBounds.tBR.nY-i->second->tBounds.tTL.nY)*4;
			}
			m_cMergedCache.clear();
			m_cMergedCS.Unlock();
		}
		if (a_eActions & EDRMGetMemoryUsage && a_pValue)
		{
			*a_pValue = n;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CLayerEncoderChecker :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumEncoderAspects
{
public:
	CLayerEncoderChecker() : m_bIncompatible(false) {}
	bool Compatible() const { return !m_bIncompatible; }

	BEGIN_COM_MAP(CLayerEncoderChecker)
		COM_INTERFACE_ENTRY(IEnumEncoderAspects)
	END_COM_MAP()

	// IEnumEncoderAspects methods
public:
	STDMETHOD(Range)(ULONG* a_pBegin, ULONG* a_nCount) { return S_OK; }
	STDMETHOD(Consume)(ULONG a_nBegin, ULONG a_nCount, BSTR const* a_abstrID, float const* a_afWeight)
	{
		for (; a_nCount > 0; --a_nCount, ++a_abstrID)
			if (wcscmp(ENCFEAT_IMAGE, *a_abstrID) && wcscmp(ENCFEAT_IMAGE_META, *a_abstrID) && wcscmp(ENCFEAT_IMAGE_ALPHA, *a_abstrID))
				m_bIncompatible = true;
		return S_OK;
	}

private:
	bool m_bIncompatible;
};

extern const GUID CLSID_ImageFilterOpacity;

STDMETHODIMP CDocumentLayeredImage::Aspects(IEnumEncoderAspects* a_pEnumAspects)
{
	try
	{
		CDocumentReadLock cLock(this);
		bool bLayers = true;
		bool bLayerEffects = false;
		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->pEffects && i->pEffects->size() > 0 && (i->pEffects->size() > 1 || !IsEqualGUID((*i->pEffects)[0].tOpID, CLSID_ImageFilterOpacity)))
			{
				bLayerEffects = true;
				break;
			}
		}
		bool bNonRasterLayer = false;
		if (m_cLayers.size() == 1)
		{
			CComBSTR bstrID;
			m_cLayers[0].GetLayerID(this, bstrID);
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			if (pData)
			{
				CComObjectStackEx<CLayerEncoderChecker> cChecker;
				pData->Aspects(&cChecker);
				if (cChecker.Compatible())
					bLayers = false;
				else
					bNonRasterLayer = true;
			}
		}
		else
		{
			for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			{
				CComBSTR bstrID;
				i->GetLayerID(this, bstrID);
				CComPtr<IDocumentData> pData;
				M_Base()->DataBlockGet(bstrID, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
				if (pData)
				{
					CComObjectStackEx<CLayerEncoderChecker> cChecker;
					pData->Aspects(&cChecker);
					if (!cChecker.Compatible())
					{
						bNonRasterLayer = true;
						break;
					}
				}
			}
		}
		float aVals[7] = {100.0f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
		BSTR aIDs[7];
		ULONG n = 0;
		aIDs[n++] = m_ENCFEAT_IMAGE;
		if (bLayers || bLayerEffects)
		{
			aVals[n] = 8.0f;
			aIDs[n++] = m_ENCFEAT_IMAGE_LAYER;
			if (bLayerEffects)
				aIDs[n++] = m_ENCFEAT_IMAGE_LAYER_EFFECT;
			if (bNonRasterLayer)
				aIDs[n++] = m_ENCFEAT_IMAGE_LAYER_SPECIAL;
		}
		//if (m_qwAlphaTotal != ULONGLONG(m_tMulSize.n3)*ULONGLONG(255))
			aIDs[n++] = m_ENCFEAT_IMAGE_ALPHA;
		if (MetaDataPresent())
			aIDs[n++] = m_ENCFEAT_IMAGE_META;
		TImagePoint tOrg = {LONG_MAX, LONG_MAX};
		TImagePoint tEnd = {LONG_MIN, LONG_MIN};
		QueryCacheContent(&tOrg, &tEnd);
		if (tOrg.nX < tEnd.nX && (tOrg.nX < 0 || tOrg.nY < 0 || tEnd.nX > LONG(m_tSize.nX) || tEnd.nY > LONG(m_tSize.nY)))
		{
			aVals[n] = 2.0f;
			aIDs[n++] = m_ENCFEAT_IMAGE_CANVAS;
		}
		a_pEnumAspects->Consume(0, n, aIDs, aVals);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "LayerRasterImage.h"

STDMETHODIMP CDocumentLayeredImage::CanvasGet(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
{
	try
	{
		if (a_pCanvasSize) *a_pCanvasSize = m_tSize;
		if (a_pResolution) *a_pResolution = m_tResolution;
		if (a_pContentOrigin || a_pContentSize)
		{
			TImagePoint tOrg = {LONG_MAX, LONG_MAX};
			TImagePoint tEnd = {LONG_MIN, LONG_MIN};
			QueryCacheContent(&tOrg, &tEnd);
			if (tOrg.nX < tEnd.nX)
			{
				if (a_pContentOrigin) *a_pContentOrigin = tOrg;
				if (a_pContentSize) { a_pContentSize->nX = tEnd.nX-tOrg.nX; a_pContentSize->nY = tEnd.nY-tOrg.nY; }
			}
			else
			{
				if (a_pContentOrigin) a_pContentOrigin->nX = a_pContentOrigin->nY = 0;
				if (a_pContentSize) a_pContentSize->nX = a_pContentSize->nY = 0;
			}
		}
		if (a_pContentOpacity) *a_pContentOpacity = EIOUnknown;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::ChannelsGet(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
{
	try
	{
		if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
		if (a_pGamma) *a_pGamma = 2.2f;//m_fCachedGamma;
		if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::TileGet(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;

		SCacheEntryPtr pEntry;
		{
			CDocumentReadLock cLock(this);
			if (m_cLayers.empty())
				return E_FAIL;
			pEntry = QueryMergedCache();
			if (pEntry == NULL)
			{
				TPixelChannel t0 = {0, 0, 0, 0};
				for (ULONG y = 0; y < a_pSize->nY; ++y, a_pData+=a_pStride->nY)
					std::fill_n(a_pData, a_pSize->nX, t0);
				return S_FALSE;
			}
			AgeCache();
		}
		return pEntry->pImage->TileGet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData, a_pControl, a_eIntent);//RGBAGetTileImpl(m_pThPool, m_tSize, pEntry->tOffset, pEntry->tSize, pEntry->pPixels, pEntry->tSize.nX, pEntry->tDefault, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::Inspect(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;

		CDocumentReadLock cLock(this);
		SCacheEntryPtr pEntry = QueryMergedCache();
		if (pEntry == NULL)
			return S_FALSE;
		HRESULT hRes = pEntry->pImage->Inspect(a_nChannelIDs, a_pOrigin, a_pSize, a_pVisitor, a_pControl, a_eIntent);//RGBAInspectImpl(pEntry->tOffset, pEntry->tSize, pEntry->pPixels, pEntry->tSize.nX, pEntry->tDefault, a_pOrigin, a_pSize, a_pVisitor, a_pControl);
		AgeCache();
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::BufferLock(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	try
	{
		if (a_nChannelID != EICIRGBA)
			return E_RW_INVALIDPARAM;

		CDocumentReadLock cLock(this);
		SCacheEntryPtr pEntry = QueryMergedCache();
		if (pEntry == NULL)
		{
			if (a_pAllocOrigin) a_pAllocOrigin->nX = a_pAllocOrigin->nY = 0;
			if (a_pAllocSize) a_pAllocSize->nX = a_pAllocSize->nY = 0;
			if (a_pContentOrigin) a_pContentOrigin->nX = a_pContentOrigin->nY = 0;
			if (a_pContentSize) a_pContentSize->nX = a_pContentSize->nY = 0;
			if (a_ppBuffer) *a_ppBuffer = NULL;
			return S_FALSE;
		}
		M_Base()->ReadLock();
		return pEntry->pImage->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::BufferUnlock(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
{
	try
	{
		if (a_nChannelID != EICIRGBA)
			return E_RW_INVALIDPARAM;

		CDocumentReadLock cLock(this);
		SCacheEntryPtr pEntry = QueryMergedCache();
		if (pEntry == NULL)
			return E_FAIL;
		HRESULT hRes = pEntry->pImage->BufferUnlock(a_nChannelID, a_pBuffer);
		AgeCache();
		M_Base()->ReadUnlock();
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

inline HRESULT CompareConfigValues(IConfig* a_p1, IConfig* a_p2)
{
	if (a_p1 == NULL || a_p2 == NULL)
		return E_POINTER;

	CComPtr<IEnumStrings> pES1;
	a_p1->ItemIDsEnum(&pES1);
	ULONG nItems1 = 0;
	pES1->Size(&nItems1);
	CComPtr<IEnumStrings> pES2;
	a_p2->ItemIDsEnum(&pES2);
	ULONG nItems2 = 0;
	pES2->Size(&nItems2);
	if (nItems1 != nItems2)
		return S_FALSE;

	HRESULT hr = S_OK;
	CAutoVectorPtr<BSTR> aIDs(new BSTR[nItems1]);
	pES1->GetMultiple(0, nItems1, aIDs);
	for (ULONG i = 0; i < nItems1; i++)
	{
		CConfigValue c1;
		CConfigValue c2;
		a_p1->ItemValueGet(aIDs[i], &c1);
		a_p2->ItemValueGet(aIDs[i], &c2);
		if (c1 != c2)
		{
			hr = S_FALSE;
			break;
		}
	}

	for (ULONG i = 0; i < nItems1; i++)
	{
		SysFreeString(aIDs[i]);
	}

	return hr;
}

STDMETHODIMP CDocumentLayeredImage::CanvasSet(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (a_pResolution &&
			(m_tResolution.nNumeratorX != a_pResolution->nNumeratorX ||
			 m_tResolution.nDenominatorX != a_pResolution->nDenominatorX ||
			 m_tResolution.nNumeratorY != a_pResolution->nNumeratorY ||
			 m_tResolution.nDenominatorY != a_pResolution->nDenominatorY))
		{
			m_tResolution = *a_pResolution;
			//m_bResolutionChange = true;
		}
		bool const bChangeCanvas = a_pSize && (a_pSize->nX != m_tSize.nX || a_pSize->nY != m_tSize.nY);
		if (a_pContentTransform || bChangeCanvas)
		{
			//CRasterImageOperationTransform cTransform(a_pSize ? a_pSize : &m_tSize, a_pContentTransform);
			CComBSTR bstrCFGID_LAYEREFFECT(CFGID_LAYEREFFECT);
			for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			{
				CComBSTR bstrID;
				i->GetLayerID(this, bstrID);
				CComPtr<IDocumentEditableImage> pDEI;
				M_Base()->DataBlockGet(bstrID, __uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
				if (pDEI)
					pDEI->CanvasSet(a_pSize ? a_pSize : &m_tSize, NULL, a_pContentTransform, a_pHelper);

				if (i->pEffects == NULL || i->pEffects->empty())
					continue;

				bool bChange = false;
				for (CLayerEffects::iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
				{
					CComQIPtr<IRasterImageFilter> pFilter(j->pOp);
					if (pFilter == NULL || j->pOpCfg == NULL)
						continue;
					CComPtr<IConfig> pOldCfg;
					j->pOpCfg->DuplicateCreate(&pOldCfg);
					pFilter->Transform(j->pOpCfg, a_pSize ? a_pSize : &m_tSize, a_pContentTransform);
					if (CompareConfigValues(j->pOpCfg, pOldCfg) == S_FALSE)
					{
						bChange = true;
						if (M_Base()->UndoEnabled() == S_OK)
						{
							CUndoLayerEffectStepSet::Add(M_Base(), this, i->nUID, j->pItem->EffectID(), static_cast<bool const*>(NULL), static_cast<GUID const*>(NULL), pOldCfg.p);
						}
					}
				}
				if (bChange)
				{
					m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
					DeleteCacheForLayer(i->nUID);
				}
			}
		}
		if (bChangeCanvas)
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoCanvasSize::Add(M_Base(), this, m_tSize);
			}

			m_tSize = *a_pSize;
			DeleteCacheForMerges();
			m_bSizeChange = true;
		}
		//if (a_pSize/* && !a_pContentTransform*/)
		//{
		//	// forward canvas change to force update notifications
		//	for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		//	{
		//		CComBSTR bstrID;
		//		i->GetLayerID(this, bstrID);
		//		CComPtr<IDocumentEditableImage> pDEI;
		//		M_Base()->DataBlockGet(bstrID, __uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
		//		if (pDEI)
		//			pDEI->CanvasSet(&m_tSize, NULL, NULL, NULL);
		//	}
		//}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::ChannelsSet(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
{
	try
	{
		if (a_nChannels != 1 || a_aChannelIDs[0] != EICIRGBA)
			return E_FAIL;

		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::StatePack(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState)
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

STDMETHODIMP CDocumentLayeredImage::StateUnpack(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems)
{
	try
	{
		*a_ppSelectedItems = NULL;
		if (a_pState)
			return a_pState->QueryInterface(a_ppSelectedItems);

		// select top frame by default
		CComPtr<IEnumUnknownsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));

		CDocumentReadLock cLock(this);

		if (!m_cLayers.empty())
			pTmp->Insert(m_cLayers.rbegin()->pItem);

		*a_ppSelectedItems = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSelectedItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

void CDocumentLayeredImage::DecodeStyle(IConfig* pOperation, CLayerEffects& aEffects)
{
	if (pOperation == NULL)
	{
		aEffects.clear();
		return;
	}
	CComBSTR bstrEffect(CFGID_LAYEREFFECT);
	CConfigValue cEffect;
	pOperation->ItemValueGet(bstrEffect, &cEffect);
	if (IsEqualGUID(cEffect, __uuidof(DocumentOperationNULL)))
	{
		aEffects.clear();
		return;
	}
	if (!IsEqualGUID(cEffect, __uuidof(DocumentOperationSequence)))
	{
		aEffects.resize(1);
		SLayerEffect& sLE = aEffects[0];
		sLE.bEnabled = true;
		sLE.tOpID = cEffect;
		pOperation->SubConfigGet(bstrEffect, &sLE.pOpCfg);
		return;
	}
	CComPtr<IConfig> pSeq;
	pOperation->SubConfigGet(bstrEffect, &pSeq);
	CConfigValue cSteps;
	CComBSTR bstrSteps(L"SeqSteps");
	pSeq->ItemValueGet(bstrSteps, &cSteps);
	ULONG const nSteps = cSteps.operator LONG();
	CComPtr<IConfig> pSteps;
	pSeq->SubConfigGet(bstrSteps, &pSteps);
	aEffects.resize(nSteps);
	OLECHAR sz[32];
	for (ULONG i = 0; i < nSteps; ++i)
	{
		SLayerEffect& sLE = aEffects[i];

		swprintf(sz, L"%08x\\SeqOperation", i);
		CComBSTR bstrSubID(sz);
		CConfigValue cStep;
		pSteps->ItemValueGet(bstrSubID, &cStep);
		sLE.tOpID = cStep;
		pSteps->SubConfigGet(bstrSubID, &sLE.pOpCfg);

		swprintf(sz, L"%08x\\SeqSkipStep", i);
		CConfigValue cSkip;
		pSteps->ItemValueGet(CComBSTR(sz), &cSkip);
		sLE.bEnabled = cSkip.TypeGet() != ECVTBool || !cSkip.operator bool();
	}
}

void CDocumentLayeredImage::EncodeStyle(CLayerEffects::const_iterator first, CLayerEffects::const_iterator last, IConfig* pOperation)
{
	CComBSTR bstrEffect(CFGID_LAYEREFFECT);
	if (first == last)
	{
		pOperation->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(__uuidof(DocumentOperationNULL)));
		return;
	}
	{
		CLayerEffects::const_iterator next = first;
		++next;
		if (next == last && first->bEnabled)
		{
			pOperation->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(first->tOpID));
			CComPtr<IConfig> pSub;
			pOperation->SubConfigGet(bstrEffect, &pSub);
			CopyConfigValues(pSub, first->pOpCfg);
			return;
		}
	}
	pOperation->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(__uuidof(DocumentOperationSequence)));
	CComPtr<IConfig> pSeq;
	pOperation->SubConfigGet(bstrEffect, &pSeq);
	CComBSTR bstrSteps(L"SeqSteps");
	pSeq->ItemValuesSet(1, &(bstrSteps.m_str), CConfigValue(LONG(last-first)));
	CComPtr<IConfig> pSteps;
	pSeq->SubConfigGet(bstrSteps, &pSteps);
	OLECHAR sz[32];
	for (LONG i = 0; first != last; ++first, ++i)
	{
		SLayerEffect const& sLE = *first;

		swprintf(sz, L"%08x\\SeqOperation", i);
		CComBSTR bstrSubID(sz);
		pSteps->ItemValuesSet(1, &(bstrSubID.m_str), CConfigValue(sLE.tOpID));
		CComPtr<IConfig> pSub;
		pSteps->SubConfigGet(bstrSubID, &pSub);
		CopyConfigValues(pSub, sLE.pOpCfg);

		swprintf(sz, L"%08x\\SeqSkipStep", i);
		CComBSTR bstrSkip(sz);
		pSteps->ItemValuesSet(1, &(bstrSkip.m_str), CConfigValue(!sLE.bEnabled));
	}
}

//void ConvertToSequence(IConfig* a_pEffect, GUID const& a_tPrevID, BSTR a_bstrEffect, bool a_bSkip, GUID const* pID, IConfig** ppConfig)
//{
//	// convert to sequence to be able to disable op
//	CConfigValue cPrev = a_tPrevID;
//	CComPtr<IConfig> pPrev2;
//	a_pEffect->SubConfigGet(a_bstrEffect, &pPrev2);
//	CComPtr<IConfig> pPrev;
//	pPrev2->DuplicateCreate(&pPrev);
//
//	a_pEffect->ItemValuesSet(1, &a_bstrEffect, CConfigValue(__uuidof(DocumentOperationSequence)));
//	CComPtr<IConfig> pSeq;
//	a_pEffect->SubConfigGet(a_bstrEffect, &pSeq);
//	CComBSTR bstrSteps(L"SeqSteps");
//	pSeq->ItemValuesSet(1, &(bstrSteps.m_str), CConfigValue(1L));
//	CComBSTR bstrSubID(L"SeqSteps\\00000000\\SeqOperation");
//	pSeq->ItemValuesSet(1, &(bstrSubID.m_str), pID ? CConfigValue(*pID) : cPrev);
//	CComBSTR bstrSkip(L"SeqSteps\\00000000\\SeqSkipStep");
//	CConfigValue cSkip;
//	pSeq->ItemValuesSet(1, &(bstrSkip.m_str), CConfigValue(a_bSkip));
//	CComPtr<IConfig> pSub;
//	pSeq->SubConfigGet(bstrSubID, &pSub);
//	CopyConfigValues(pSub, ppConfig ? *ppConfig : pPrev);
//}

STDMETHODIMP CDocumentLayeredImage::ItemsEnum(IComparable* a_pRoot, IEnumUnknowns** a_ppSubItems)
{
	try
	{
		*a_ppSubItems = NULL;

		if (a_pRoot != NULL)
		//	return E_RW_ITEMNOTFOUND;
		{
			CComQIPtr<IStructuredItemLayerItem> pSILI(a_pRoot);
			if (pSILI == NULL)
				return E_RW_ITEMNOTFOUND;
			if (pSILI->Doc() != this)
			{
				return pSILI->Doc()->ItemsEnum(a_pRoot, a_ppSubItems);
			}
			ULONG nLayerUID = -1;
			nLayerUID = pSILI->ID(this);

			CDocumentReadLock cLock(this);

			for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			{
				if (i->nUID == nLayerUID)
				{
					CComPtr<IEnumUnknownsInit> pInit;
					RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));

					if (i->pEffects)
					{
						for (CLayerEffects::const_reverse_iterator j = i->pEffects->rbegin(); j != i->pEffects->rend(); ++j)
							pInit->Insert(static_cast<IUIItem*>(j->pItem));
					}

					CComBSTR bstr;
					i->GetLayerID(this, bstr);
					CComPtr<IDocument> pDoc;
					M_Base()->DataBlockDoc(bstr, &pDoc);
					if (pDoc)
					{
						CComPtr<IDocumentLayeredImage> pDLI;
						pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
						if (pDLI)
						{
							CComPtr<IEnumUnknowns> pSub;
							pDLI->ItemsEnum(NULL, &pSub);
							if (pSub)
								pInit->InsertFromEnum(pSub);
						}
					}

					//CComPtr<IEnumUnknowns> pSub;
					//pSILI->SubItems(&pSub);
					//if (pSub)
					//	pInit->InsertFromEnum(pSub);

					*a_ppSubItems = pInit.Detach();
					return S_OK;
				}
			}

			//CComQIPtr<ISubDocumentID> pSD(a_pRoot);
			//if (pSD)
			//{
			//	CComPtr<IDocument> pDoc;
			//	pSD->SubDocumentGet(&pDoc);
			//	if (pDoc)
			//	{
			//		CComPtr<IStructuredDocument> pStr;
			//		pDoc->QueryFeatureInterface(__uuidof(IStructuredDocument), reinterpret_cast<void**>(&pStr));
			//		if (pStr)
			//		{
			//			return pStr->ItemsEnum(NULL, a_ppSubItems);
			//		}
			//	}
			//}
			return E_RW_ITEMNOTFOUND;
		//	CComQIPtr<IStructuredItemLayerItem> pLI(a_pRoot);
		//	if (pLI == NULL)
		//		return E_RW_ITEMNOTFOUND;
		//	return pLI->SubItems(a_ppSubItems);
		}

		CComPtr<IEnumUnknownsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));

		CDocumentReadLock cLock(this);

		for (CLayers::const_reverse_iterator i = m_cLayers.rbegin(); i != m_cLayers.rend(); ++i)
		{
			pTmp->Insert(i->pItem);
		}

		*a_ppSubItems = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectsEnum(IComparable* a_pLayer, IEnumUnknowns** a_ppEffects)
{
	try
	{
		*a_ppEffects = NULL;
		if (a_pLayer == NULL)
			return S_FALSE;

		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pLayer);
		if (pSILI == NULL || pSILI->Doc() == NULL)
			return E_RW_ITEMNOTFOUND;
		if (pSILI->Doc() != this)
			return pSILI->Doc()->LayerEffectsEnum(a_pLayer, a_ppEffects);

		ULONG nLayerUID = -1;
		nLayerUID = pSILI->ID(this);

		CDocumentReadLock cLock(this);

		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nLayerUID)
			{
				CComPtr<IEnumUnknownsInit> pInit;
				RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));

				if (i->pEffects)
				{
					for (CLayerEffects::const_iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
						pInit->Insert(static_cast<IUIItem*>(j->pItem));
				}

				*a_ppEffects = pInit.Detach();
				return S_OK;
			}
		}

		return S_OK;
	}
	catch (...)
	{
		return a_ppEffects == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerFromEffect(IComparable* a_pEffect, IComparable** a_ppLayer)
{
	if (a_ppLayer == NULL)
		return E_POINTER;
	try
	{
		CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pEffect);
		if (pSILE == NULL || pSILE->Doc() == NULL)
			return E_RW_ITEMNOTFOUND;
		if (pSILE->Doc() != this)
			return pSILE->Doc()->LayerFromEffect(a_pEffect, a_ppLayer);

		ULONG nLayerUID = -1;
		nLayerUID = pSILE->LayerID();

		CDocumentReadLock cLock(this);

		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nLayerUID)
			{
				*a_ppLayer = i->pItem;
				i->pItem.p->AddRef();
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

LONG CDocumentLayeredImage::FindItemLevel(IComparable* a_pRoot, IComparable* a_pItem, LONG a_nRootLevel)
{
	CComPtr<IEnumUnknowns> pItems;
	ItemsEnum(a_pRoot, &pItems);
	if (pItems == NULL)
		return -1;
	CComPtr<IComparable> pItem;
	for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); ++i, pItem = NULL)
	{
		if (pItem->Compare(a_pItem) == S_OK)
			return a_nRootLevel;
		LONG nLevel = FindItemLevel(pItem, a_pItem, a_nRootLevel);
		if (nLevel != -1)
			return nLevel+1;
	}
	return -1;
}

STDMETHODIMP CDocumentLayeredImage::IsLayer(IComparable* a_pItem, ULONG* a_pLevel, INT_PTR* a_pParentHandle, GUID* a_pBuilder)
{
	if (a_pItem == NULL)
		return E_POINTER;
	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
	if (pSILI)
	{
		if (a_pLevel)
			*a_pLevel = FindItemLevel(NULL, a_pItem, 0);
		if (a_pParentHandle)
			*a_pParentHandle = reinterpret_cast<INT_PTR>(pSILI->Doc());
		if (a_pBuilder)
			pSILI->Doc()->LayerBuilderIDGet(pSILI->ID(pSILI->Doc()), a_pBuilder);
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CDocumentLayeredImage::FindByName(BSTR a_bstrName, ISubDocumentID** a_ppItem)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentLayeredImage::LayersEnum(IComparable* a_pRoot, IEnumUnknowns** a_ppLayers)
{
	if (a_pRoot == NULL)
		return ItemsEnum(NULL, a_ppLayers);

	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pRoot);
	if (pSILI == NULL) return E_RW_INVALIDPARAM;

	CComQIPtr<ISubDocumentID> pSD(a_pRoot);
	if (pSD)
	{
		CComPtr<IDocument> pDoc;
		pSD->SubDocumentGet(&pDoc);
		if (pDoc)
		{
			CComPtr<IDocumentLayeredImage> pDLI;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI)
			{
				return pDLI->ItemsEnum(NULL, a_ppLayers);
			}
		}
	}
	return E_RW_INVALIDPARAM;
}

STDMETHODIMP CDocumentLayeredImage::LayerInsert(IComparable* a_pWhere, ELayerInsertPosition a_ePosition, IImageLayerCreator* a_pCreator, IComparable** a_ppNew)
{
	try
	{
		if (a_ppNew)
			*a_ppNew = NULL;
		CDocumentWriteLock cLock(this);

		if (a_pWhere != NULL)
		{
			CComPtr<IStructuredItemLayerItem> pSILI;
			ItemFeatureGet(a_pWhere, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pSILI));
			if (pSILI != NULL && pSILI->Doc() != this)
				return pSILI->Doc()->LayerInsert(a_pWhere, a_ePosition, a_pCreator, a_ppNew);
		}

		SLayer sLayer;
		sLayer.Init(this, InterlockedIncrement(&m_nNextID), M_OpMgr());
		CComBSTR bstr;
		sLayer.GetLayerID(this, bstr);

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoLayerInsert::Add(M_Base(), this, sLayer.nUID);
		}

		HRESULT hRes = a_pCreator->Create(bstr, M_Base());
		M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&sLayer.pDocImg));
		if (sLayer.pDocImg)
		{
			sLayer.pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), sLayer.nUID);
			M_Base()->DataBlockGet(bstr, __uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&sLayer.pDocGrp));
			if (sLayer.pDocGrp)
				sLayer.pDocGrp->ObserverIns(CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet(), sLayer.nUID);
		}
		else
		{
			M_Base()->DataBlockSet(bstr, NULL);
			return hRes;
		}
		CComQIPtr<IStructuredItemLayerItem> pLI(a_pWhere);
		ULONG nBefore = -1;
		if (pLI) nBefore = pLI->ID(this);
		CLayers::iterator iBefore = a_ePosition != ELIPBelow ? m_cLayers.end() : m_cLayers.begin();
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nBefore)
			{
				iBefore = i;
				if (a_ePosition != ELIPBelow)
					++iBefore;
				break;
			}
		}
		bool bLast = iBefore == m_cLayers.end();
		if (bLast)
			m_cLayers.push_back(sLayer);
		else
			m_cLayers.insert(iBefore, sLayer);

		LayersChanged();
		if (!bLast)
			DeleteCacheForMerges(); // TODO: only delete affected (after inserted)
		if (a_ppNew)
		{
			(*a_ppNew = sLayer.pItem)->AddRef();
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::InitLayer(IDocumentData* a_pBlock, LCID a_tLocaleID, TImageSize const& a_tCanvasSize)
{
	try
	{
		CDocumentWriteLock cLock(this);
		m_tSize = a_tCanvasSize;
		SLayer sLayer;
		sLayer.Init(this, InterlockedIncrement(&m_nNextID), M_OpMgr());
		CComBSTR bstr;
		sLayer.GetLayerID(this, bstr);
		if (a_pBlock)
			M_Base()->DataBlockSet(bstr, a_pBlock);
		M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&sLayer.pDocImg));
		if (sLayer.pDocImg)
		{
			sLayer.pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), sLayer.nUID);
		}
		CComBSTR bstrName;
		CMultiLanguageString::GetLocalized(L"[0409]Background[0405]Pozadí", a_tLocaleID, &bstrName);
		sLayer.strName = bstrName.m_str;
		m_cLayers.push_back(sLayer);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerMove(IComparable* a_pItem, IComparable* a_pWhere, ELayerInsertPosition a_ePosition)
{
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
		//CComPtr<IStructuredItemLayerItem> pSILI;
		//ItemFeatureGet(a_pItem, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pSILI));
		if (pSILI == NULL)
			return E_RW_ITEMNOTFOUND;
		CDocumentLayeredImage* pFrom = pSILI->Doc();

		CComQIPtr<IStructuredItemLayerItem> pSILI2(a_pWhere);
		if (pSILI2 == NULL && a_pWhere != NULL)
			return E_RW_INVALIDPARAM; // invalid target
		CDocumentLayeredImage* pTo = pSILI2.p ? pSILI2->Doc() : this;

		if (pFrom != pTo)
			return E_RW_INVALIDPARAM; // can only move item in the same doc

		if (pTo != this)
			return pTo->LayerMove(a_pItem, a_pWhere, a_ePosition);

		ULONG nLayerUID = pSILI->ID(this);
		ULONG nBeforeUID = -1;
		if (pSILI2)
			nBeforeUID = pSILI2->ID(this);
		CDocumentWriteLock cLock(this);
		CLayers::iterator iBefore = a_ePosition != ELIPBelow ? m_cLayers.end() : m_cLayers.begin();
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nBeforeUID)
			{
				iBefore = i;
				if (a_ePosition != ELIPBelow)
					++iBefore;
				break;
			}
		}
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nLayerUID)
			{
				if (UndoEnabled() == S_OK)
				{
					CUndoLayerMove::Add(M_Base(), this, nLayerUID, (i+1) == m_cLayers.end() ? 0xffffffff : (i+1)->nUID);
				}
				if (i < iBefore)
				{
					SLayer sL = *i;
					while (++i < iBefore)
					{
						*(i-1) = *i;
					}
					*(i-1) = sL;
				}
				else if (i > iBefore)
				{
					SLayer sL = *i;
					while (--i > iBefore)
					{
						*(i+1) = *i;
					}
					*(i+1) = *i;
					*iBefore = sL;
				}
				LayersChanged();
				DeleteCacheForMerges(); // TODO: only delete affected (ex: none if layer is appended)
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

STDMETHODIMP CDocumentLayeredImage::LayerDelete(IComparable* a_pItem)
{
	try
	{
		CComPtr<IStructuredItemLayerItem> pSILI;
		ULONG nLayerUID = -1;
		ItemFeatureGet(a_pItem, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pSILI));
		if (pSILI == NULL)
			return E_RW_ITEMNOTFOUND;
		if (pSILI->Doc() != this)
			return pSILI->Doc()->LayerDelete(a_pItem);
		nLayerUID = pSILI->ID(this);
		CDocumentWriteLock cLock(this);
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nLayerUID)
			{
				CComBSTR bstr;
				i->GetLayerID(this, bstr);
				if (UndoEnabled() == S_OK)
				{
					if (i->pDocImg)
					{
						i->pDocImg->ObserverDel(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), i->nUID);
						i->pDocImg->Release();
						i->pDocImg = NULL;
					}
					if (i->pDocGrp)
					{
						i->pDocGrp->ObserverDel(CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet(), i->nUID);
						i->pDocGrp->Release();
						i->pDocGrp = NULL;
					}
					SLayer s = *i;
					ULONG nUnder = (i+1) == m_cLayers.end() ? 0xffffffff : (i+1)->nUID;
					m_cLayers.erase(i);
					CComPtr<IDocumentData> pData;
					M_Base()->DataBlockGet(bstr, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
					ULONGLONG nSize = 1024;
					pData->ResourcesManage(EDRMMinimizeAndGetMemoryUsage, &nSize);
					M_Base()->DataBlockSet(bstr, NULL);
					CUndoLayerDelete::Add(M_Base(), this, &s, pData, nSize, nUnder);
				}
				else
				{
					i->Release(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet());
					m_cLayers.erase(i);
					M_Base()->DataBlockSet(bstr, NULL);
				}
				LayersChanged();
				DeleteCacheForLayer(nLayerUID);
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

STDMETHODIMP CDocumentLayeredImage::LayerReplace(IComparable* a_pItem, IImageLayerCreator* a_pCreator)
{
	try
	{
		CComPtr<IStructuredItemLayerItem> pSILI;
		ULONG nLayerUID = -1;
		ItemFeatureGet(a_pItem, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pSILI));
		if (pSILI == NULL)
			return E_RW_ITEMNOTFOUND;
		nLayerUID = pSILI->ID(this);
		CDocumentWriteLock cLock(this);
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nLayerUID)
			{
				CComBSTR bstr;
				i->GetLayerID(this, bstr);
				//if (UndoEnabled() == S_OK)
				//{
				//	if (i->pDocImg)
				//	{
				//		i->pDocImg->ObserverDel(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), i->nUID);
				//		i->pDocImg->Release();
				//		i->pDocImg = NULL;
				//	}
				//	SLayer s = *i;

				//	CComPtr<IDocumentData> pData;
				//	M_Base()->DataBlockGet(bstr, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
				//	ULONGLONG nSize = 1024;
				//	pData->ResourcesManage(EDRMMinimizeAndGetMemoryUsage, &nSize);
				//	M_Base()->DataBlockSet(bstr, NULL);
				//	CUndoLayerReplace::Add(M_Base(), this, &s, pData, nSize);

				//	a_pCreator->Create(bstr, M_Base());
				//	M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&sLayer.pDocImg));
				//	if (sLayer.pDocImg)
				//	{
				//		sLayer.pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), sLayer.nUID);
				//	}
				//	else
				//	{
				//		M_Base()->DataBlockSet(bstr, NULL);
				//		return E_FAIL;
				//	}
				//}
				//else
				{
					if (i->pDocImg)
					{
						i->pDocImg->ObserverDel(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), i->nUID);
						i->pDocImg->Release();
						i->pDocImg = NULL;
					}
					if (i->pDocGrp)
					{
						i->pDocGrp->ObserverDel(CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet(), i->nUID);
						i->pDocGrp->Release();
						i->pDocGrp = NULL;
					}
					a_pCreator->Create(bstr, M_Base());
					M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&i->pDocImg));
					if (i->pDocImg)
					{
						i->pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), i->nUID);
					}
					else
					{
						M_Base()->DataBlockSet(bstr, NULL);
						return E_FAIL; // inconsistent state
					}
				}
				++i->nTimestamp;
				m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation;
				DeleteCacheForLayer(nLayerUID);
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

HRESULT CDocumentLayeredImage::LayerRestore(SLayer& a_sLayer, IDocumentData* a_pData, ULONG a_nUnder)
{
	CDocumentWriteLock cLock(this);
	CComBSTR bstr;
	a_sLayer.GetLayerID(this, bstr);
	M_Base()->DataBlockSet(bstr, a_pData);
	M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&a_sLayer.pDocImg));
	if (a_sLayer.pDocImg)
	{
		a_sLayer.pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), a_sLayer.nUID);
	}
	CLayers::iterator iBefore = m_cLayers.end();
	for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
	{
		if (i->nUID == a_nUnder)
		{
			iBefore = i;
			break;
		}
	}
	bool bLast = iBefore == m_cLayers.end();
	if (bLast)
		m_cLayers.push_back(a_sLayer);
	else
		m_cLayers.insert(iBefore, a_sLayer);
	if (a_sLayer.pEffects)
		a_sLayer.pEffects = NULL;
	if (M_Base()->UndoEnabled() == S_OK)
	{
		CUndoLayerInsert::Add(M_Base(), this, a_sLayer.nUID);
	}
	LayersChanged();
	if (!bLast)
		DeleteCacheForMerges(); // TODO: only delete affected (after inserted)
	return S_OK;
}

STDMETHODIMP CDocumentLayeredImage::LayerNameGet(IComparable* a_pItem, BSTR* a_pbstrName)
{
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
		if (pSILI != NULL && pSILI->Doc() != this)
			return pSILI->Doc()->LayerNameGet(a_pItem, a_pbstrName);

		*a_pbstrName = NULL;
		CDocumentReadLock cLock(this);
		SLayer* pL = FindLayer(a_pItem);
		if (pL == NULL) return E_RW_ITEMNOTFOUND;
		*a_pbstrName = CComBSTR(pL->strName.c_str()).Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerNameSet(IComparable* a_pItem, BSTR a_bstrName)
{
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
		if (pSILI != NULL && pSILI->Doc() != this)
			return pSILI->Doc()->LayerNameSet(a_pItem, a_bstrName);

		CDocumentWriteLock cLock(this);
		SLayer* pL = FindLayer(a_pItem);
		if (pL == NULL) return E_RW_ITEMNOTFOUND;
		std::wstring str(a_bstrName ? a_bstrName : L"");
		if (str == pL->strName)
			return S_FALSE;
		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoLayerName::Add(M_Base(), this, a_pItem, pL->strName.c_str());
		}
		std::swap(str, pL->strName);
		m_cLayerChanges[pL->nUID].nChanges |= ESCContent|ESCGUIRepresentation;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerPropsGet(IComparable* a_pItem, ELayerBlend* a_pBlendingMode, BYTE* a_pVisible)
{
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
		if (pSILI != NULL && pSILI->Doc() != this)
			return pSILI->Doc()->LayerPropsGet(a_pItem, a_pBlendingMode, a_pVisible);

		CDocumentReadLock cLock(this);
		SLayer* pL = FindLayer(a_pItem);
		if (pL == NULL) return E_RW_ITEMNOTFOUND;
		if (a_pBlendingMode)
			*a_pBlendingMode = pL->eBlendingMode;
		if (a_pVisible)
			*a_pVisible = pL->bVisible;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerPropsSet(IComparable* a_pItem, ELayerBlend const* a_pBlendingMode, BYTE const* a_pVisible)
{
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
		if (pSILI != NULL && pSILI->Doc() != this)
			return pSILI->Doc()->LayerPropsSet(a_pItem, a_pBlendingMode, a_pVisible);

		CDocumentWriteLock cLock(this);
		SLayer* pL = FindLayer(a_pItem);
		if (pL == NULL) return E_RW_ITEMNOTFOUND;
		if (a_pBlendingMode && *a_pBlendingMode == pL->eBlendingMode)
			a_pBlendingMode = NULL;
		if (a_pVisible && (*a_pVisible != 0) == pL->bVisible)
			a_pVisible = NULL;
		if (a_pBlendingMode || a_pVisible)
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoLayerProperties::Add(M_Base(), this, a_pItem, pL->eBlendingMode, pL->bVisible);
			}
			if (a_pBlendingMode)
				pL->eBlendingMode = *a_pBlendingMode;
			if (a_pVisible)
				pL->bVisible = *a_pVisible;
			m_cLayerChanges[pL->nUID].nChanges |= ESCContent|ECSLayerMeta;
			DeleteMergedCacheForLayer(pL->nUID);
			if (a_pBlendingMode)
			{
				// update cached blending mode
				CComAutoCriticalSectionLock lock(m_cLayerCS);
				CLayerCache::iterator i = m_cLayerCache.find(pL->nUID);
				if (i != m_cLayerCache.end())
					i->second->eBlend = *a_pBlendingMode;
			}

		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectGet(IComparable* a_pItem, IConfig** a_ppOperation, float* a_pOpacity)
{
	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
	if (pSILI == NULL || pSILI->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILI->Doc() != this)
		return pSILI->Doc()->LayerEffectGet(a_pItem, a_ppOperation, a_pOpacity);

	return LayerEffectGet(pSILI->ID(this), a_ppOperation, a_pOpacity);
}

HRESULT CDocumentLayeredImage::LayerEffectGet(ULONG a_nLayerID, IConfig** a_ppOperation, float* a_pOpacity)
{
	try
	{
		CDocumentReadLock cLock(this);
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == a_nLayerID)
			{
				CLayerEffects::const_iterator first = i->pEffects->begin();
				CLayerEffects::const_iterator last = i->pEffects->end();
				if (a_pOpacity)
				{
					*a_pOpacity = 1.0f;
					if (first != last)
					{
						CLayerEffects::const_iterator opac = i->pEffects->begin()+(i->pEffects->size()-1);
						if (IsEqualGUID(opac->tOpID, CLSID_ImageFilterOpacity))
						{
							CConfigValue cVal;
							if (opac->pOpCfg)
								opac->pOpCfg->ItemValueGet(CComBSTR("Opacity"), &cVal);
							if (cVal.TypeGet() == ECVTFloat)
								*a_pOpacity = cVal;
							last = opac;
						}
					}
				}
				if (a_ppOperation)
				{
					CComPtr<IConfigWithDependencies> pCfg;
					RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
					M_OpMgr()->InsertIntoConfigAs(M_OpMgr(), pCfg, CComBSTR(CFGID_LAYEREFFECT), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Operation applied on the selected image layer.[0405]Operace aplikovaná na vybranou vrstvu obrázku."), 0, NULL);
					CConfigCustomGUI<&CLSID_DocumentFactoryLayeredImage, CConfigGUILayoutEffectDlg>::FinalizeConfig(pCfg);
					EncodeStyle(first, last, pCfg);
					*a_ppOperation = pCfg.Detach();
				}
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

STDMETHODIMP CDocumentLayeredImage::LayerEffectSet(IComparable* a_pItem, IConfig* a_pOperation)
{
	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
	if (pSILI == NULL || pSILI->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILI->Doc() != this)
		return pSILI->Doc()->LayerEffectSet(a_pItem, a_pOperation);

	try
	{
		CLayerEffects cEffects;
		DecodeStyle(a_pOperation, cEffects);
		return LayerEffectSet(pSILI->ID(this), cEffects, cEffects.empty()); // can swap if empty ;-)
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::LayerEffectSet(ULONG a_nLayerID, CLayerEffects& a_cEffects, bool a_bSwap)
{
	try
	{
		CDocumentWriteLock cLock(this);
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == a_nLayerID)
			{
				// TODO: compare styles and skip op if same
				CLayerEffects cEffects;
				if (a_bSwap)
				{
					cEffects.swap(a_cEffects);
				}
				else
				{
					cEffects.resize(a_cEffects.size());
					for (size_t j = 0; j != cEffects.size(); ++j)
					{
						SLayerEffect const& s = a_cEffects[j];
						SLayerEffect& d = cEffects[j];
						d.bEnabled = s.bEnabled;
						d.tOpID = s.tOpID;
						RWCoCreateInstance(d.tOpID, NULL, CLSCTX_ALL, __uuidof(IDocumentOperation), reinterpret_cast<void**>(&d.pOp));
						if (d.pOp)
							d.pOp->ConfigCreate(M_OpMgr(), &d.pOpCfg);
						CopyConfigValues(d.pOpCfg, s.pOpCfg);
						d.pItem = new CStructuredItemLayerEffect(this, i->nUID, InterlockedIncrement(&m_nNextID));
					}
				}
				if (M_Base()->UndoEnabled() == S_OK)
				{
					CUndoLayerEffect::Add(M_Base(), this, i->nUID, i->pEffects);
				}
				m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
				DeleteCacheForLayer(i->nUID);
				cEffects.swap(*i->pEffects);
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

CDocumentLayeredImage::SLayerEffect* CDocumentLayeredImage::LayerEffectStepGet(ULONG a_nLayerID, ULONG a_nEffectID)
{
	CLayers::iterator i = m_cLayers.begin();
	while (i != m_cLayers.end())
	{
		if (i->nUID == a_nLayerID)
			break;
		++i;
	}
	if (i == m_cLayers.end() || i->pEffects == NULL)
		return NULL;
	CLayerEffects& cLE = *i->pEffects;
	for (CLayerEffects::iterator j = cLE.begin(); j != cLE.end(); ++j)
	{
		if (j->pItem->EffectID() == a_nEffectID)
			return &*j;
	}
	return NULL;
}

bool CDocumentLayeredImage::LayerEffectStepGet(ULONG a_nLayerID, ULONG a_nEffectID, bool* a_pEnabled, CLSID* a_pID, IConfig** a_ppConfig)
{
	CDocumentReadLock cLock(this);
	SLayerEffect const* p = LayerEffectStepGet(a_nLayerID, a_nEffectID);
	if (p == NULL)
		return false;
	if (a_pEnabled)
		*a_pEnabled = p->bEnabled;
	if (a_pID)
		*a_pID = p->tOpID;
	if (a_ppConfig && p->pOpCfg)
		p->pOpCfg->DuplicateCreate(a_ppConfig);
	return true;
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectStepGet(IComparable* a_pItem, BYTE* a_pEnabled, GUID* a_pOpID, IConfig** a_ppOpCfg)
{
	CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
	if (pSILE == NULL || pSILE->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILE->Doc() != this)
		return pSILE->Doc()->LayerEffectStepGet(a_pItem, a_pEnabled, a_pOpID, a_ppOpCfg);

	try
	{
		CDocumentReadLock cLock(this);

		SLayerEffect const* p = LayerEffectStepGet(pSILE->LayerID(), pSILE->EffectID());
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;

		if (a_pEnabled)
			*a_pEnabled = p->bEnabled;
		if (a_pOpID)
			*a_pOpID = p->tOpID;
		if (a_ppOpCfg && p->pOpCfg)
			p->pOpCfg->DuplicateCreate(a_ppOpCfg);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::LayerEffectStepSet(ULONG a_nLayerID, ULONG a_nEffectID, bool const* a_pEnabled, GUID const* a_pOpID, IConfig* a_pOpCfg)
{
	try
	{
		CDocumentWriteLock cLock(this);

		SLayerEffect* p = LayerEffectStepGet(a_nLayerID, a_nEffectID);
		if (p == NULL)
			return E_RW_ITEMNOTFOUND;
		if (a_pEnabled && *a_pEnabled == p->bEnabled)
			a_pEnabled = NULL;
		if (a_pOpID && IsEqualGUID(*a_pOpID, p->tOpID))
		{
			a_pOpID = NULL;
			if (a_pOpCfg && CompareConfigValues(a_pOpCfg, p->pOpCfg) == S_OK)
				a_pOpCfg = NULL;
		}
		if (a_pEnabled == NULL && a_pOpID == NULL && a_pOpCfg == NULL)
			return S_FALSE; // no change

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CComPtr<IConfig> pCopy;
			if (a_pOpCfg && p->pOpCfg)
				p->pOpCfg->DuplicateCreate(&pCopy);
			CUndoLayerEffectStepSet::Add(M_Base(), this, a_nLayerID, a_nEffectID, a_pEnabled ? &p->bEnabled : NULL, a_pOpID ? &p->tOpID : NULL, pCopy.p);
		}

		if (a_pEnabled)
			p->bEnabled = *a_pEnabled;
		if (a_pOpID)
		{
			p->tOpID = *a_pOpID;
			CComPtr<IDocumentOperation> pOp;
			RWCoCreateInstance(pOp, *a_pOpID);
			CComPtr<IConfig> pOpCfg;
			if (pOp)
				pOp->ConfigCreate(M_OpMgr(), &pOpCfg);
			std::swap(p->pOp, pOp.p);
			std::swap(p->pOpCfg, pOpCfg.p);
		}
		if (a_pOpCfg && p->pOpCfg)
			CopyConfigValues(p->pOpCfg, a_pOpCfg);
		m_cLayerChanges[a_nLayerID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(a_nLayerID);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectStepSet(IComparable* a_pItem, BYTE* a_pEnabled, GUID* a_pOpID, IConfig* a_pOpCfg)
{
	CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
	if (pSILE == NULL || pSILE->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILE->Doc() != this)
		return pSILE->Doc()->LayerEffectStepSet(a_pItem, a_pEnabled, a_pOpID, a_pOpCfg);

	if (!a_pEnabled)
		return LayerEffectStepSet(pSILE->LayerID(), pSILE->EffectID(), NULL, a_pOpID, a_pOpCfg);
	bool b = *a_pEnabled;
	return LayerEffectStepSet(pSILE->LayerID(), pSILE->EffectID(), &b, a_pOpID, a_pOpCfg);
}

HRESULT CDocumentLayeredImage::LayerEffectStepInsert(ULONG a_nLayerID, ULONG a_nBeforeEffectID, SLayerEffect* a_pLE, IComparable** a_ppNew)
{
	try
	{
		CDocumentWriteLock cLock(this);

		CLayers::iterator i = m_cLayers.begin();
		while (i != m_cLayers.end())
		{
			if (i->nUID == a_nLayerID)
				break;
			++i;
		}
		if (i == m_cLayers.end())
			return E_RW_ITEMNOTFOUND;

		if (i->pEffects == NULL)
			return E_UNEXPECTED;

		i->pEffects->reserve(i->pEffects->size()+1);

		CLayerEffects::iterator iBefore = i->pEffects->end();
		for (CLayerEffects::iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
		{
			if (j->pItem->EffectID() == a_nBeforeEffectID)
			{
				iBefore = j;
				break;
			}
		}

		i->pEffects->insert(iBefore, *a_pLE);

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoLayerEffectStepInsert::Add(M_Base(), this, a_nLayerID, a_pLE->pItem->EffectID());
		}

		SLayerEffect dummy;
		a_pLE->swap(dummy); // so that it does not get cleaned up in undo (since we are stealing the internals)

		m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(i->nUID);

		if (a_ppNew)
			a_pLE->pItem->QueryInterface(__uuidof(IComparable), reinterpret_cast<void**>(a_ppNew));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::LayerEffectStepInsert(ULONG a_nLayerID, ULONG a_nBeforeEffectID, BYTE a_bEnabled, REFGUID a_tOpID, IConfig* a_pOpCfg, IComparable** a_ppNew)
{
	try
	{
		CDocumentWriteLock cLock(this);

		CLayers::iterator i = m_cLayers.begin();
		while (i != m_cLayers.end())
		{
			if (i->nUID == a_nLayerID)
				break;
			++i;
		}
		if (i == m_cLayers.end())
			return E_RW_ITEMNOTFOUND;

		if (i->pEffects == NULL)
			return E_UNEXPECTED;

		CLayerEffects cNew;
		cNew.resize(1);
		SLayerEffect& sLE = cNew[0];
		sLE.bEnabled = a_bEnabled;
		sLE.tOpID = a_tOpID;
		CComPtr<IDocumentOperation> pOp;
		RWCoCreateInstance(pOp, a_tOpID);
		CComPtr<IConfig> pOpCfg;
		if (pOp)
			pOp->ConfigCreate(M_OpMgr(), &pOpCfg);
		if (pOpCfg && a_pOpCfg)
			CopyConfigValues(pOpCfg, a_pOpCfg);
		std::swap(sLE.pOp, pOp.p);
		std::swap(sLE.pOpCfg, pOpCfg.p);
		ULONG const nNewID = InterlockedIncrement(&m_nNextID);
		sLE.pItem = new CStructuredItemLayerEffect(this, i->nUID, nNewID);
		CComPtr<IUIItem> pNewItem(sLE.pItem);

		i->pEffects->reserve(i->pEffects->size()+1);

		CLayerEffects::iterator iBefore = i->pEffects->end();
		for (CLayerEffects::iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
		{
			if (j->pItem->EffectID() == a_nBeforeEffectID)
			{
				iBefore = j;
				break;
			}
		}

		i->pEffects->insert(iBefore, sLE);
		cNew.clear();

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoLayerEffectStepInsert::Add(M_Base(), this, a_nLayerID, nNewID);
		}

		m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(i->nUID);

		if (a_ppNew)
			*a_ppNew = pNewItem.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectStepAppend(IComparable* a_pItem, BYTE a_bEnabled, REFGUID a_tOpID, IConfig* a_pOpCfg, IComparable** a_ppNew)
{
	CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
	if (pSILE != NULL)
	{
		if (pSILE->Doc() == NULL)
			return E_RW_ITEMNOTFOUND;
		if (pSILE->Doc() != this)
			return pSILE->Doc()->LayerEffectStepAppend(a_pItem, a_bEnabled, a_tOpID, a_pOpCfg, a_ppNew);
		return LayerEffectStepInsert(pSILE->LayerID(), pSILE->EffectID(), a_bEnabled, a_tOpID, a_pOpCfg, a_ppNew);
	}
	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
	if (pSILI == NULL || pSILI->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILI->Doc() != this)
		return pSILI->Doc()->LayerEffectStepAppend(a_pItem, a_bEnabled, a_tOpID, a_pOpCfg, a_ppNew);
	return LayerEffectStepInsert(pSILI->ID(this), 0xffffffff, a_bEnabled, a_tOpID, a_pOpCfg, a_ppNew);
}

HRESULT CDocumentLayeredImage::LayerEffectStepDelete(ULONG a_nLayerID, ULONG a_nEffectID)
{
	try
	{
		CDocumentWriteLock cLock(this);

		CLayers::iterator i = m_cLayers.begin();
		while (i != m_cLayers.end())
		{
			if (i->nUID == a_nLayerID)
				break;
			++i;
		}
		if (i == m_cLayers.end())
			return E_RW_ITEMNOTFOUND;

		CLayerEffects::iterator iToDelete = i->pEffects->end();
		for (CLayerEffects::iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
		{
			if (j->pItem->EffectID() == a_nEffectID)
			{
				iToDelete = j;
				break;
			}
		}
		if (iToDelete == i->pEffects->end())
			return E_RW_ITEMNOTFOUND;

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CLayerEffects::iterator iNext = iToDelete; ++iNext;
			ULONG nBefore = iNext == i->pEffects->end() ? 0xffffffff : iNext->pItem->EffectID();
			CUndoLayerEffectStepDelete::Add(M_Base(), this, a_nLayerID, nBefore, &*iToDelete);
		}
		else
		{
			iToDelete->CleanUp();
		}
		i->pEffects->erase(iToDelete);

		m_cLayerChanges[i->nUID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(a_nLayerID);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::LayerEffectStepDelete(IComparable* a_pItem)
{
	CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
	if (pSILE == NULL || pSILE->Doc() == NULL)
		return E_RW_ITEMNOTFOUND;
	if (pSILE->Doc() != this)
		return pSILE->Doc()->LayerEffectStepDelete(a_pItem);

	return LayerEffectStepDelete(pSILE->LayerID(), pSILE->EffectID());
}

STDMETHODIMP CDocumentLayeredImage::LayerRender(IComparable* a_pItem, BSTR a_bstrLayerWithEffectID, IDocumentBase* a_pLayerWithEffectBase)
{
	try
	{
		CDocumentReadLock cLock(this);
		SLayer* pL = FindLayer(a_pItem);
		if (pL == NULL) return E_RW_ITEMNOTFOUND;
		SCacheEntryPtr pCached = QueryLayerCache(pL->nUID);
		CComObject<CDocumentRasterImage>* pRI = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&pRI);
		CComPtr<IDocumentData> pRIData = pRI;
		TImagePoint tAllocOrigin;
		TImageSize  tAllocSize;
		TImagePoint tContentOrigin;
		TImageSize tContentSize;
		TPixelChannel const* pData;
		pCached->pImage->BufferLock(EICIRGBA, &tAllocOrigin, &tAllocSize, &tContentOrigin, &tContentSize, &pData, NULL, EIRIAccurate);
		try {
			pRI->Init(m_tSize, &m_tResolution, &pCached->tDefault, tContentOrigin, tContentSize, pData+(tContentOrigin.nY-tAllocOrigin.nY)*tAllocSize.nX+(tContentOrigin.nX-tAllocOrigin.nX), 2.2f, NULL);
		} catch (...) {}
		pCached->pImage->BufferUnlock(EICIRGBA, pData);
		return a_pLayerWithEffectBase->DataBlockSet(a_bstrLayerWithEffectID, pRIData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentLayeredImage::LayersChanged()
{
	m_bLayersChange = true;
}

HRESULT CDocumentLayeredImage::GetSubDocument(ULONG a_nID, IDocument** a_ppSubDocument) const
{
	SLayer const& sLayer = FindLayer(a_nID);
	CComBSTR bstr;
	sLayer.GetLayerID(this, bstr);
	return M_Base()->DataBlockDoc(bstr, a_ppSubDocument);
	//CComPtr<IDocument> pDoc;
	//HRESULT hRes = M_Base()->DataBlockDoc(bstr, &pDoc);
	//CComPtr<IDocumentLayeredImage> pDLI;
	//if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
	//if (pDLI == NULL)
	//{
	//	*a_ppSubDocument = pDoc.Detach();
	//	return hRes;
	//}
	//CComPtr<IEnumUnknowns> pSubItems;
	//pDLI->ItemsEnum(NULL, &pSubItems);
	//ULONG nSubItems = 0;
	//if (pSubItems) pSubItems->Size(&nSubItems);
	//for (ULONG i = 0; i < nSubItems; ++i)
	//{
	//	CComPtr<IComparable> pSubItem;
	//	pSubItems->Get(i, &pSubItem);
	//	CComQIPtr<IStructuredItemLayerItem> pSILI(pSubItem);
	//	if (pSILI == NULL) continue;
	//	CComQIPtr<ISubDocumentID> pSDID(pSubItem);
	//	if (pSDID) return pSDID->SubDocumentGet(a_ppSubDocument);
	//}
	//*a_ppSubDocument = pDoc.Detach();
	//return hRes;
}

#include "LayerEffectSubDocument.h"

HRESULT CDocumentLayeredImage::GetEffectSubDocument(ULONG a_nID, ULONG a_nStep, IDocument** a_ppSubDocument)
{
	if (a_ppSubDocument == NULL)
		return E_POINTER;

	try
	{
		CComObject<CLayerEffectSubDocument>* p = NULL;
		CComObject<CLayerEffectSubDocument>::CreateInstance(&p);
		CComPtr<IDocument> pDoc = p;
		p->Init(M_Base(), this, a_nID, a_nStep);
		*a_ppSubDocument = pDoc;
		pDoc.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentLayeredImage::GetLayerTypeName(ULONG a_nID, ILocalizedString** a_ppName)
{
	CDocumentReadLock cLock(this);
	SLayer const& sLayer = FindLayer(a_nID);
	CComBSTR bstr;
	sLayer.GetLayerID(this, bstr);
	CComPtr<ILayerType> pLT;
	M_Base()->DataBlockGet(bstr, __uuidof(ILayerType), reinterpret_cast<void**>(&pLT));
	if (pLT) pLT->Name(a_ppName);
}

CDocumentLayeredImage::SLayer* CDocumentLayeredImage::FindLayer(IComparable* a_pItem)
{
	CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
	if (pSILI == NULL || this != pSILI->Doc())
		return NULL;
	ULONG nLayerUID = -1;
	//ItemFeatureGet(a_pItem, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pSILI));
	//if (pSILI == NULL)
	//	return NULL;
	nLayerUID = pSILI->ID(this);
	for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
	{
		if (i->nUID == nLayerUID)
		{
			return &(*i);
		}
	}
	return NULL;
}


#include <DragDropHelper.h>

struct SDraggedLayer
{
	CDocumentLayeredImage* pRoot;
	CDocumentLayeredImage* pThis;
	ULONG nLayerID;
	ULONG nIDs;
	ULONG aIDs[1];
};

STDMETHODIMP CDocumentLayeredImage::Begin(IEnumUnknowns* a_pSelection, IDataObject** a_ppDataObject, IDropSource** a_ppDropSource, DWORD* a_pOKEffects)
{
	try
	{
		ULONG nItems = 0;
		a_pSelection->Size(&nItems);
		if (nItems == 0)
			return S_FALSE;

		CAutoVectorPtr<BYTE> pBuf(new BYTE[sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(nItems-1)]);
		SDraggedLayer* pLayer = reinterpret_cast<SDraggedLayer*>(pBuf.m_p);
		pLayer->nLayerID = 0xffffffff;
		pLayer->nIDs = 0;
		pLayer->pThis = NULL;
		pLayer->pRoot = this;

		for (ULONG i = 0; i < nItems; ++i)
		{
			CComPtr<IStructuredItemLayerItem> pItem;
			a_pSelection->Get(i, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&pItem));
			if (pItem)
			{
				if (pLayer->nLayerID != 0xffffffff)
					return E_RW_INVALIDPARAM; // items must be all layers or all effects
				if (pLayer->pThis == NULL)
					pLayer->pThis = pItem->Doc();
				else if (pLayer->pThis != pItem->Doc())
					return E_RW_INVALIDPARAM; // items must have same parent
				pLayer->aIDs[pLayer->nIDs++] = pItem->ID(pLayer->pThis);
				continue;
			}
			CComPtr<IStructuredItemLayerEffect> pEffect;
			a_pSelection->Get(i, __uuidof(IStructuredItemLayerEffect), reinterpret_cast<void**>(&pEffect));
			if (pEffect)
			{
				if (pLayer->nLayerID == 0xffffffff)
				{
					if (pLayer->nIDs > 0)
						return E_RW_INVALIDPARAM; // items must be all layers or all effects
					pLayer->nLayerID = pEffect->LayerID();
				}
				else if (pLayer->nLayerID != pEffect->LayerID())
					return E_RW_INVALIDPARAM; // effects must belong to the same layer

				if (pLayer->pThis == NULL)
					pLayer->pThis = pEffect->Doc();
				else if (pLayer->pThis != pEffect->Doc())
					return E_RW_INVALIDPARAM; // items must have same parent
				pLayer->aIDs[pLayer->nIDs++] = pEffect->EffectID();
				continue;
			}
			return E_RW_INVALIDPARAM; // unknown item
		}

		CComObject<CDragDropHelper>* pDropSrc = NULL;
		CComObject<CDragDropHelper>::CreateInstance(&pDropSrc);
		CComPtr<IDropSource> pTmp = pDropSrc;

		if (!pDropSrc->InitStruct(RegisterClipboardFormat(_T("RWLI_LAYERID")), pBuf, sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(pLayer->nIDs-1)))
	        return E_FAIL;

		//CComPtr<IDragSourceHelper> pdsh;
		//HRESULT hr = pdsh.CoCreateInstance ( CLSID_DragDropHelper );
		//if (pdsh)
		//{
		//	SHDRAGIMAGE tImage;
		//	tImage.sizeDragImage;
		//	tImage.ptOffset;
		//	tImagehbmpDragImage;
		//	tImage.crColorKey = 0;
		//	pdsh->InitializeFromBitmap(&tImage, pDropSrc);
		//	//pdsh->InitializeFromWindow(m_wndList, &pNMLV->ptAction, pDropSrc);
		//}

		if (a_ppDataObject)
		{
			*a_ppDataObject = pDropSrc;
			pDropSrc->AddRef();
		}
		if (a_ppDropSource)
		{
			*a_ppDropSource = pDropSrc;
			pDropSrc->AddRef();
		}
		if (a_pOKEffects)
			*a_pOKEffects = DROPEFFECT_MOVE|DROPEFFECT_COPY;

	    return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	try
	{
		ULONG nFiles = 0;
		if (a_pFileNames && SUCCEEDED(a_pFileNames->Size(&nFiles)) && nFiles)
		{
			if (a_pdwEffect)
			{
				*a_pdwEffect = DROPEFFECT_COPY;
			}
			if (a_ppFeedback)
			{
				bool effect = false;
				if (nFiles == 1)
				{
					CComBSTR bstrName;
					a_pFileNames->Get(0, &bstrName);
					ULONG len = bstrName.Length();
					effect = len > 9 && _wcsicmp(bstrName.m_str+len-9, L".rweffect") == 0;
				}
				if (effect)
					*a_ppFeedback = new CMultiLanguageString(L"[0409]Replace layer style.[0405]Nahradit styl vrstvy.");
				else
					*a_ppFeedback = new CMultiLanguageString(L"[0409]Add the dragged file(s) as new layer(s).[0405]Přidat tažené soubory jako nové vrstvy.");
			}
			return S_OK;
		}
		FORMATETC tFE = { RegisterClipboardFormat(_T("RWLI_LAYERID")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM tMed;
		ZeroMemory(&tMed, sizeof tMed);
		if (a_pDataObj && SUCCEEDED(a_pDataObj->GetData(&tFE, &tMed)))
		{
			SDraggedLayer* pDL = reinterpret_cast<SDraggedLayer*>(GlobalLock(tMed.hGlobal));
			CAutoVectorPtr<BYTE> pBuf(new BYTE[sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(pDL->nIDs-1)]);
			CopyMemory(pBuf.m_p, pDL, sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(pDL->nIDs-1));
			pDL = reinterpret_cast<SDraggedLayer*>(pBuf.m_p);
			bool bEffects = pDL->nLayerID != 0xffffffff;
			GlobalUnlock(tMed.hGlobal);
			ReleaseStgMedium(&tMed);
			if (pDL->pRoot == this)
			{
				if (!bEffects)
				{
					// check if target is a layer
					CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
					if (pSILI == NULL && a_pItem)
					{
						if (a_pdwEffect)
							*a_pdwEffect = DROPEFFECT_SCROLL;
						return E_FAIL; // invalid target (probably effect)
					}

					CDocumentLayeredImage* pTargetDoc = pSILI ? pSILI->Doc() : this;
					if (pTargetDoc != pDL->pThis) // dragging layers between groups
					{
						// check if we are not dragging to our own child, which would result in invalid structure
						for (ULONG i = 0; i < pDL->nIDs; ++i)
						{
							CLayers::const_iterator iR = pDL->pThis->m_cLayers.begin();
							while (iR != pDL->pThis->m_cLayers.end() && iR->nUID != pDL->aIDs[i])
								++iR;
							if (iR == pDL->pThis->m_cLayers.end())
								continue;
							if (HasChildLayer(pDL->pThis, iR->pItem, a_pItem))
							{
								if (a_pdwEffect)
									*a_pdwEffect = DROPEFFECT_SCROLL;
								return E_FAIL;
							}
						}
					}
				}
				else
				{
					CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
					if (pSILE == NULL || pSILE->Doc() != pDL->pThis || pSILE->LayerID() != pDL->nLayerID)
					{
						if (a_pdwEffect)
							*a_pdwEffect = DROPEFFECT_SCROLL;
						return E_FAIL; // only can drag within one layer now
					}
				}
				if (a_pdwEffect)
				{
					if (a_grfKeyState&MK_CONTROL)
						*a_pdwEffect = DROPEFFECT_COPY;
					else
						*a_pdwEffect = DROPEFFECT_MOVE;
					if (a_eDNDPoint == EDNDPLower)
						*a_pdwEffect |= DND_INSERTMARK_AFTER;
					else
						*a_pdwEffect |= DND_INSERTMARK_BEFORE;
				}
				if (a_ppFeedback)
				{
					*a_ppFeedback = NULL;
					wchar_t const* psz = NULL;
					if (bEffects)
					{
						if (a_grfKeyState&MK_CONTROL)
							psz = L"[0409]Copy the dragged effects.[0405]Zkopírovat tažené efekty.";
						else
							psz = L"[0409]Move the dragged effects up or down.[0405]Posunout tažený efekt nahoru nebo dolů.";
					}
					else
					{
						if (a_grfKeyState&MK_CONTROL)
							psz = L"[0409]Copy the dragged layer.[0405]Zkopírovat taženou vrstvu.";
						else
							psz = L"[0409]Move the dragged layer up or down.[0405]Posunout taženou vrstvu nahoru nebo dolů.";
					}
					*a_ppFeedback = new CMultiLanguageString(psz);
				}
				return S_OK;
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct CIsSel
{
	CIsSel(ULONG nItemIDs, ULONG const* aItemIDs, bool polarity) : nItemIDs(nItemIDs), aItemIDs(aItemIDs), polarity(polarity) {}

	bool operator()(CDocumentLayeredImage::SLayerEffect const& tSL) const
	{
		for (ULONG k = 0; k < nItemIDs; ++k)
			if (aItemIDs[k] == tSL.pItem->EffectID())
				return polarity;
		return !polarity;
	}
	bool operator()(CDocumentLayeredImage::SLayer const& tS) const
	{
		for (ULONG k = 0; k < nItemIDs; ++k)
			if (aItemIDs[k] == tS.nUID)
				return polarity;
		return !polarity;
	}

	ULONG nItemIDs;
	ULONG const* aItemIDs;
	bool polarity;
};

#include <algorithm>

HRESULT CDocumentLayeredImage::MoveEffects(ULONG nLayerID, ULONG nEffectIDs, ULONG const* aEffectIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel)
{
	CLayers::const_iterator i = m_cLayers.begin();
	while (i != m_cLayers.end() && i->nUID != nLayerID)
		++i;
	if (i == m_cLayers.end())
		return E_RW_ITEMNOTFOUND;
	CLayerEffects& cEffects = *(i->pEffects);
	CLayerEffects::iterator iTarget = cEffects.begin();
	while (iTarget != cEffects.end() && iTarget->pItem->EffectID() != nTarget)
		++iTarget;
	if (iTarget != cEffects.end() && a_eDNDPoint == EDNDPUpper)
		++iTarget;

	// for sending changes and undoing
	std::vector<ULONG> cOld;
	for (CLayerEffects::const_iterator j = cEffects.begin(); j != cEffects.end(); ++j)
		cOld.push_back(j->pItem->EffectID());

	std::stable_partition(cEffects.begin(), iTarget, CIsSel(nEffectIDs, aEffectIDs, false));
	std::stable_partition(iTarget, cEffects.end(), CIsSel(nEffectIDs, aEffectIDs, true));

	bool bChange = false;
	{
		std::vector<ULONG>::const_iterator k = cOld.begin();
		for (CLayerEffects::const_iterator j = cEffects.begin(); j != cEffects.end(); ++j, ++k)
			if (j->pItem->EffectID() != *k)
			{
				bChange = true;
				break;
			}
	}

	if (bChange)
	{
		if (UndoEnabled() == S_OK)
		{
			CUndoLayerEffectsReorder::Add(M_Base(), this, nLayerID, &cOld);
		}
		m_cLayerChanges[nLayerID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(nLayerID);
	}
	if (a_ppNewSel)
	{
		CComPtr<ISharedState> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
		for (CLayerEffects::const_iterator j = cEffects.begin(); j != cEffects.end(); ++j)
		{
			for (ULONG k = 0; k < nEffectIDs; ++k)
				if (aEffectIDs[k] == j->pItem->EffectID())
				{
					pInit->Insert(static_cast<IUIItem*>(j->pItem));
					break;
				}
		}
		*a_ppNewSel = pTmp.Detach();
	}
	return S_OK;
}

HRESULT CDocumentLayeredImage::MoveLayers(ULONG nLayerIDs, ULONG const* aLayerIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel)
{
	CLayers::iterator iTarget = m_cLayers.begin();
	while (iTarget != m_cLayers.end() && iTarget->nUID != nTarget)
		++iTarget;
	if (iTarget != m_cLayers.end())
	{
		if (a_eDNDPoint == EDNDPUpper)
			++iTarget;
	}
	else
	{
		if (a_eDNDPoint == EDNDPLower)
			iTarget = m_cLayers.begin();
	}

	// for sending changes and undoing
	std::vector<ULONG> cOld;
	for (CLayers::const_iterator j = m_cLayers.begin(); j != m_cLayers.end(); ++j)
		cOld.push_back(j->nUID);

	std::stable_partition(m_cLayers.begin(), iTarget, CIsSel(nLayerIDs, aLayerIDs, false));
	std::stable_partition(iTarget, m_cLayers.end(), CIsSel(nLayerIDs, aLayerIDs, true));

	bool bChange = false;
	{
		std::vector<ULONG>::const_iterator k = cOld.begin();
		for (CLayers::const_iterator j = m_cLayers.begin(); j != m_cLayers.end(); ++j, ++k)
			if (j->nUID != *k)
			{
				bChange = true;
				break;
			}
	}

	if (bChange)
	{
		if (UndoEnabled() == S_OK)
		{
			CUndoLayersReorder::Add(M_Base(), this, &cOld);
		}
		LayersChanged();
		DeleteCacheForMerges(); // TODO: dont delete merged cache below first change
	}

	if (a_ppNewSel)
	{
		CComPtr<ISharedState> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
		for (CLayers::const_iterator j = m_cLayers.begin(); j != m_cLayers.end(); ++j)
		{
			for (ULONG k = 0; k < nLayerIDs; ++k)
				if (aLayerIDs[k] == j->nUID)
				{
					pInit->Insert(static_cast<IUIItem*>(j->pItem));
					break;
				}
		}
		*a_ppNewSel = pTmp.Detach();
	}
	return S_OK;
}

HRESULT CDocumentLayeredImage::InsertLayers(CDocumentLayeredImage* pSrc, ULONG nLayerIDs, ULONG const* aLayerIDs, ULONG nTarget, EDNDPoint a_eDNDPoint, ISharedState** a_ppNewSel)
{
	size_t iTarget = 0;
	while (iTarget != m_cLayers.size() && m_cLayers[iTarget].nUID != nTarget)
		++iTarget;
	if (iTarget != m_cLayers.size())
	{
		if (a_eDNDPoint == EDNDPUpper)
			++iTarget;
	}
	else
	{
		if (a_eDNDPoint == EDNDPLower)
			iTarget = 0;
	}

	CComPtr<ISharedState> pTmp;
	RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
	CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
	for (CLayers::iterator iSource = pSrc->m_cLayers.begin(); iSource != pSrc->m_cLayers.end(); ++iSource)
	{
		bool bFound = false;
		for (ULONG j = 0; j < nLayerIDs; ++j)
		{
			if (iSource->nUID == aLayerIDs[j])
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
			continue;

		SLayer sLayer;
		sLayer.Init(this, InterlockedIncrement(&m_nNextID), M_OpMgr());
		CComBSTR bstr;
		sLayer.GetLayerID(this, bstr);

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoLayerInsert::Add(M_Base(), this, sLayer.nUID);
		}

		sLayer.bVisible = iSource->bVisible;
		sLayer.eBlendingMode = iSource->eBlendingMode;
		sLayer.strName = iSource->strName;

		CComBSTR bstrSrcID;
		iSource->GetLayerID(pSrc, bstrSrcID);
		CComPtr<IDocument> pSrcDoc;
		M_Base()->DataBlockDoc(bstrSrcID, &pSrcDoc);
		HRESULT hRes = pSrcDoc->DocumentCopy(bstr, M_Base(), NULL, NULL);
		M_Base()->DataBlockGet(bstr, __uuidof(IDocumentImage), reinterpret_cast<void**>(&sLayer.pDocImg));
		if (sLayer.pDocImg)
		{
			sLayer.pDocImg->ObserverIns(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), sLayer.nUID);
			M_Base()->DataBlockGet(bstr, __uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&sLayer.pDocGrp));
			if (sLayer.pDocGrp)
				sLayer.pDocGrp->ObserverIns(CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet(), sLayer.nUID);
		}
		else
		{
			M_Base()->DataBlockSet(bstr, NULL);
			return hRes;
		}

		// copy effects
		sLayer.pEffects->resize(iSource->pEffects->size());
		for (size_t j = 0; j != iSource->pEffects->size(); ++j)
		{
			SLayerEffect const& s = (*iSource->pEffects)[j];
			SLayerEffect& d = (*sLayer.pEffects)[j];
			d.bEnabled = s.bEnabled;
			d.tOpID = s.tOpID;
			RWCoCreateInstance(d.tOpID, NULL, CLSCTX_ALL, __uuidof(IDocumentOperation), reinterpret_cast<void**>(&d.pOp));
			if (d.pOp)
				d.pOp->ConfigCreate(M_OpMgr(), &d.pOpCfg);
			CopyConfigValues(d.pOpCfg, s.pOpCfg);
			d.pItem = new CStructuredItemLayerEffect(this, sLayer.nUID, InterlockedIncrement(&m_nNextID));
		}

		if (this == pSrc) // when copying from self, iSource is invalidated
		{
			size_t pos = iSource-pSrc->m_cLayers.begin();
			m_cLayers.insert(m_cLayers.begin()+iTarget, sLayer);
			if (iTarget <= pos)
				++pos;
			iSource = pSrc->m_cLayers.begin()+pos;
		}
		else
		{
			m_cLayers.insert(m_cLayers.begin()+iTarget, sLayer);
		}
		++iTarget;

		LayersChanged();
		if (a_ppNewSel)
		{
			pInit->Insert(sLayer.pItem);
		}
	}
	if (iTarget != m_cLayers.size())
		DeleteCacheForMerges(); // TODO: only delete affected (after inserted)
	if (a_ppNewSel)
		*a_ppNewSel = pTmp.Detach();
	return S_OK;
}

HRESULT CDocumentLayeredImage::DeleteLayers(ULONG nLayerIDs, ULONG const* aLayerIDs)
{
	for (ULONG j = 0; j < nLayerIDs; ++j)
	{
		CLayers::iterator i = m_cLayers.begin();
		while (i != m_cLayers.end() && i->nUID != aLayerIDs[j])
			++i;
		if (i == m_cLayers.end())
			continue; // should not really happen

		CComBSTR bstr;
		i->GetLayerID(this, bstr);
		if (UndoEnabled() == S_OK)
		{
			if (i->pDocImg)
			{
				i->pDocImg->ObserverDel(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), i->nUID);
				i->pDocImg->Release();
				i->pDocImg = NULL;
			}
			if (i->pDocGrp)
			{
				i->pDocGrp->ObserverDel(CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet(), i->nUID);
				i->pDocGrp->Release();
				i->pDocGrp = NULL;
			}
			SLayer s = *i;
			ULONG nUnder = (i+1) == m_cLayers.end() ? 0xffffffff : (i+1)->nUID;
			m_cLayers.erase(i);
			CComPtr<IDocumentData> pData;
			M_Base()->DataBlockGet(bstr, __uuidof(IDocumentData), reinterpret_cast<void**>(&pData));
			ULONGLONG nSize = 1024;
			pData->ResourcesManage(EDRMMinimizeAndGetMemoryUsage, &nSize);
			M_Base()->DataBlockSet(bstr, NULL);
			CUndoLayerDelete::Add(M_Base(), this, &s, pData, nSize, nUnder);
		}
		else
		{
			i->Release(CObserverImpl<CDocumentLayeredImage, IImageObserver, TImageChange>::ObserverGet(), CObserverImpl<CDocumentLayeredImage, IStructuredObserver, TStructuredChanges>::ObserverGet());
			m_cLayers.erase(i);
			M_Base()->DataBlockSet(bstr, NULL);
		}
		LayersChanged();
		DeleteCacheForLayer(aLayerIDs[j]);
	}
	return S_OK;
}

ULONG CDocumentLayeredImage::DeleteEmptyGroups(IDocumentLayeredImage* pDoc)
{
	CComPtr<IEnumUnknowns> pLayers;
	pDoc->LayersEnum(NULL, &pLayers);
	ULONG nLayers = 0;
	if (pLayers) pLayers->Size(&nLayers);
	ULONG nRet = nLayers;
	for (ULONG i = 0; i < nLayers; ++i)
	{
		CComPtr<IComparable> pItem;
		pLayers->Get(i, &pItem);
		CComQIPtr<ISubDocumentID> pSDID(pItem);
		CComPtr<IDocument> pSubDoc;
		if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
		CComPtr<IDocumentLayeredImage> pSubLI;
		if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pSubLI));
		if (pSubLI)
		{
			if (0 == DeleteEmptyGroups(pSubLI))
			{
				--nRet;
				pDoc->LayerDelete(pItem);
			}
		}
	}
	return nRet;
}

bool CDocumentLayeredImage::HasChildLayer(IDocumentLayeredImage* pDoc, IComparable* pRoot, IComparable* pItem)
{
	CComPtr<IEnumUnknowns> pLayers;
	pDoc->LayersEnum(pRoot, &pLayers);
	ULONG nLayers = 0;
	if (pLayers) pLayers->Size(&nLayers);
	for (ULONG i = 0; i < nLayers; ++i)
	{
		CComPtr<IComparable> p;
		pLayers->Get(i, &p);
		if (p->Compare(pItem) == S_OK)
			return true;
		if (HasChildLayer(pDoc, p, pItem))
			return true;
	}
	return false;
}

HRESULT CDocumentLayeredImage::LayerEffectsReorder(ULONG a_nLayerID, std::vector<ULONG>& a_aOrder)
{
	try
	{
		CDocumentWriteLock cLock(this);
		CLayers::const_iterator i = m_cLayers.begin();
		while (i != m_cLayers.end() && i->nUID != a_nLayerID)
			++i;
		if (i == m_cLayers.end())
			return E_RW_ITEMNOTFOUND;

		CLayerEffects& cEffects = *(i->pEffects);

		std::vector<ULONG> cOld;
		for (CLayerEffects::const_iterator j = cEffects.begin(); j != cEffects.end(); ++j)
			cOld.push_back(j->pItem->EffectID());
		if (cOld == a_aOrder)
			return S_FALSE; // no change

		ATLASSERT(cOld.size() == a_aOrder.size());

		std::vector<ULONG>::const_iterator k = a_aOrder.begin();
		CLayerEffects::iterator j = cEffects.begin();
		while (k != a_aOrder.end())
		{
			CLayerEffects::iterator next = j;
			while (next != cEffects.end() && next->pItem->EffectID() != *k)
				++next;
			if (next == cEffects.end())
			{
				ATLASSERT(FALSE);
				return E_FAIL; // inconsistent state
			}
			if (next != j)
				next->swap(*j);
			++j;
			++k;
		}
		m_cLayerChanges[a_nLayerID].nChanges |= ESCContent|ESCGUIRepresentation|ECSLayerMeta; // TODO: only set ESCGUIRepresentation if effect actually changes from none to something or back
		DeleteCacheForLayer(a_nLayerID);
		if (UndoEnabled() == S_OK)
		{
			CUndoLayerEffectsReorder::Add(M_Base(), this, a_nLayerID, &cOld);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::LayersReorder(std::vector<ULONG>& a_aOrder)
{
	try
	{
		CDocumentWriteLock cLock(this);

		std::vector<ULONG> cOld;
		for (CLayers::const_iterator j = m_cLayers.begin(); j != m_cLayers.end(); ++j)
			cOld.push_back(j->nUID);
		if (cOld == a_aOrder)
			return S_FALSE; // no change

		ATLASSERT(cOld.size() == a_aOrder.size());

		std::vector<ULONG>::const_iterator k = a_aOrder.begin();
		CLayers::iterator j = m_cLayers.begin();
		while (k != a_aOrder.end())
		{
			CLayers::iterator next = j;
			while (next != m_cLayers.end() && next->nUID != *k)
				++next;
			if (next == m_cLayers.end())
			{
				ATLASSERT(FALSE);
				return E_FAIL; // inconsistent state
			}
			if (next != j)
				next->swap(*j);
			++j;
			++k;
		}
		LayersChanged();
		DeleteCacheForMerges(); // TODO: dont delete merged cache below first change
		if (UndoEnabled() == S_OK)
		{
			CUndoLayersReorder::Add(M_Base(), this, &cOld);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, LCID a_tLocaleID, ISharedState** a_ppNewSel)
{
	try
	{
		if (a_ppNewSel) *a_ppNewSel = NULL;
		ULONG nFiles = 0;
		if (a_pFileNames && SUCCEEDED(a_pFileNames->Size(&nFiles)) && nFiles)
		{
			CDocumentWriteLock cLock(this);
			if (nFiles == 1 && a_pItem)
			{
				CComBSTR bstrName;
				a_pFileNames->Get(0, &bstrName);
				ULONG len = bstrName.Length();
				if (len > 9 && _wcsicmp(bstrName.m_str+len-9, L".rweffect") == 0)
				{
					CStorageFilter pLoc(bstrName);
					CDirectInputLock cDataLock(pLoc);
					//CComPtr<IDataSrcDirect> pSrc;
					//pLoc.operator IStorageFilter *()->SrcOpen(&pSrc);
					//pSrc->SrcLock(
					CComPtr<IConfigInMemory> pMemCfg;
					RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
					pMemCfg->DataBlockSet(cDataLock.size(), cDataLock.begin());
					if (S_OK == IsLayer(a_pItem, NULL, NULL, NULL))
						return LayerEffectSet(a_pItem, pMemCfg);
					CComPtr<IComparable> pLayer;
					HRESULT hRes = LayerFromEffect(a_pItem, &pLayer);
					if (FAILED(hRes)) return hRes;
					hRes = LayerEffectSet(pLayer, pMemCfg);
					if (FAILED(hRes)) return hRes;
					StatePack(1, &(pLayer.p), a_ppNewSel);
					return hRes;
				}
			}
			for (ULONG i = 0; i < nFiles; ++i)
			{
				CComBSTR bstr;
				a_pFileNames->Get(i, &bstr);
				CStorageFilter pLoc(bstr);
				CComPtr<IComparable> pItem;
				LayerInsert(NULL, ELIPDefault, &CImageLayerCreatorStorage(pLoc), &pItem);
				if (pItem)
				{
					LPCOLESTR p1 = wcsrchr(bstr, L'\\');
					LPCOLESTR p2 = wcsrchr(bstr, L'//');
					if (p2 > p1) p1 = p2;
					if (p1 == NULL) p1 = bstr; else ++p1;
					p2 = wcsrchr(p1, L'.');
					if (p2 == NULL) p2 = p1+wcslen(p1);
					BSTR bstrName = SysAllocStringLen(p1, p2-p1);
					LayerNameSet(pItem, bstrName);
					SysFreeString(bstrName);
				}
			}
			return S_OK;
		}
		FORMATETC tFE = { RegisterClipboardFormat(_T("RWLI_LAYERID")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM tMed;
		ZeroMemory(&tMed, sizeof tMed);
		if (a_pDataObj == NULL || FAILED(a_pDataObj->GetData(&tFE, &tMed)))
			return E_FAIL;
		SDraggedLayer* pDL = reinterpret_cast<SDraggedLayer*>(GlobalLock(tMed.hGlobal));
		CAutoVectorPtr<BYTE> pBuf(new BYTE[sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(pDL->nIDs-1)]);
		CopyMemory(pBuf.m_p, pDL, sizeof(SDraggedLayer)+sizeof(ULONG)*LONG(pDL->nIDs-1));
		pDL = reinterpret_cast<SDraggedLayer*>(pBuf.m_p);
		GlobalUnlock(tMed.hGlobal);
		ReleaseStgMedium(&tMed);
		if (pDL->pRoot != this)
			return E_FAIL; // do not support dragging form another document

		CDocumentWriteLock cLock(this);
		if (pDL->nLayerID == 0xffffffff)
		{
			// moving/copying layers

			CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
			if (pSILI == NULL && a_pItem)
				return E_RW_INVALIDPARAM; // invalid target
			CDocumentLayeredImage* pTargetDoc = pSILI ? pSILI->Doc() : this;
			if (pTargetDoc == pDL->pThis) // dragging layers within a single group
			{
				if (0 == (a_grfKeyState&MK_CONTROL)) // if not copying do optimized move (reorder)
					return pTargetDoc->MoveLayers(pDL->nIDs, pDL->aIDs, pSILI ? pSILI->ID(pDL->pThis) : 0xffffffff, a_eDNDPoint, a_ppNewSel);
				return pTargetDoc->InsertLayers(pDL->pThis, pDL->nIDs, pDL->aIDs, pSILI ? pSILI->ID(pSILI->Doc()) : 0xffffffff, a_eDNDPoint, a_ppNewSel);
			}

			// check if we are not dragging to our own child, which would result in invalid structure
			for (ULONG i = 0; i < pDL->nIDs; ++i)
			{
				CLayers::const_iterator iR = pDL->pThis->m_cLayers.begin();
				while (iR != pDL->pThis->m_cLayers.end() && iR->nUID != pDL->aIDs[i])
					++iR;
				if (iR == pDL->pThis->m_cLayers.end())
					continue;
				if (HasChildLayer(pDL->pThis, iR->pItem, a_pItem))
					return E_FAIL;
			}
			
			HRESULT hRes = pTargetDoc->InsertLayers(pDL->pThis, pDL->nIDs, pDL->aIDs, pSILI ? pSILI->ID(pSILI->Doc()) : 0xffffffff, a_eDNDPoint, a_ppNewSel);

			if (0 == (a_grfKeyState&MK_CONTROL)) // if not copying, delete source
			{
				if (FAILED(hRes)) return hRes;
				pDL->pThis->DeleteLayers(pDL->nIDs, pDL->aIDs);

				DeleteEmptyGroups(this);
			}

			return hRes; // unsupported right now
		}
		else
		{
			// moving/copying effects
			CComQIPtr<IStructuredItemLayerEffect> pSILE(a_pItem);
			if (pSILE)
			{
				if (pSILE->Doc() == pDL->pThis) // dragging effect within one layer
					return pDL->pThis->MoveEffects(pDL->nLayerID, pDL->nIDs, pDL->aIDs, pSILE->EffectID(), a_eDNDPoint, a_ppNewSel);
				return E_NOTIMPL;
			}
			CComQIPtr<IStructuredItemLayerItem> pSILI(a_pItem);
			if (pSILI)
			{
				return E_NOTIMPL;
			}
			return E_FAIL; // invalid target
		}

		if (pDL->pThis != this)
		{
			// moving subitems
			return E_FAIL;
		}
		CComQIPtr<IStructuredItemLayerItem> pLI(a_pItem);
		ULONG nLayerID = -1;
		if (pLI) nLayerID = pLI->ID(this);
		CComPtr<IUIItem> pUnder;
		if (nLayerID == -1)
		{
			if (!m_cLayers.empty()) nLayerID = m_cLayers.begin()->nUID;
		}
		else
		{
			for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			{
				if (i->nUID == nLayerID)
				{
					nLayerID = i+1 != m_cLayers.end() ? (++i)->nUID : -1;
					break;
				}
			}
		}
		if (nLayerID != -1)
		{
			CComObject<CStructuredItemLayer>* p = NULL;
			CComObject<CStructuredItemLayer>::CreateInstance(&p);
			pUnder = p;
			p->Init(this, nLayerID);
		}
		for (ULONG i = 0; i < pDL->nIDs; ++i)
		{
			CComObject<CStructuredItemLayer>* p = NULL;
			CComObject<CStructuredItemLayer>::CreateInstance(&p);
			CComPtr<IUIItem> p2 = p;
			p->Init(this, pDL->aIDs[i]);
			if (FAILED(LayerMove(p2, pUnder, ELIPBelow)))
				return E_FAIL;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool CDocumentLayeredImage::ClipboardTypeIsLayer(ERichGUIClipboardAction a_eAction, ISharedState* a_pState)
{
	if (a_eAction == ERGCAPaste)
	{
		if (m_nClipboardLayers == 0) m_nClipboardLayers = RegisterClipboardFormat(_T("RWLayers"));
		if (m_nClipboardEffects == 0) m_nClipboardEffects = RegisterClipboardFormat(_T("RWCONFIG"));
		if (IsClipboardFormatAvailable(m_nClipboardEffects))
			return false;
		return true;
	}
	CComPtr<IEnumUnknowns> pList;
	if (a_pState) a_pState->QueryInterface(__uuidof(IEnumUnknowns), reinterpret_cast<void**>(&pList));
	if (pList)
	{
		CComPtr<IStructuredItemLayerEffect> pEffect;
		pList->Get(0, &pEffect);
		if (pEffect)
			return false;
	}
	return true;
}

STDMETHODIMP CDocumentLayeredImage::ClipboardName(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ILocalizedString** a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	try
	{
		*a_ppName = ClipboardTypeIsLayer(a_eAction, a_pState) ?
			new CMultiLanguageString(L"[0409]layers[0405]vrstvy") :
			new CMultiLanguageString(L"[0409]effects[0405]efekty");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::ClipboardIconID(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, GUID* a_pIconID)
{
	try
	{
		GUID const tLayerID = {0xbb09dd95, 0xe3e9, 0x41e9, {0xbb, 0x77, 0x89, 0x8f, 0xc6, 0xb0, 0x6e, 0x7c}};
		GUID const tEffectID = {0xf21f051c, 0xb1b9, 0x4c35, {0xaf, 0x19, 0xed, 0x32, 0xfe, 0x34, 0x73, 0x1f}};
		*a_pIconID = ClipboardTypeIsLayer(a_eAction, a_pState) ? tLayerID : tEffectID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

#include <IconRenderer.h>

STDMETHODIMP CDocumentLayeredImage::ClipboardIcon(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	try
	{
		if (a_phIcon) *a_phIcon = NULL;
		if (a_pOverlay) *a_pOverlay = TRUE;
		if (a_phIcon)
		{
			if (ClipboardTypeIsLayer(a_eAction, a_pState))
			{
				static IRPolyPoint const aBack[] = { {0, 36}, {220, 36}, {220, 256}, {0, 256}, };
				static IRPolyPoint const aFront[] = { {36, 161}, {36, 0}, {256, 0}, {256, 220}, {95, 220}, };
				static IRPolyPoint const aCorner[] = { {44, 155}, {101, 155}, {101, 212}, };
				static IRPolygon const tBack = {itemsof(aBack), aBack};
				static IRPolygon const tFront = {itemsof(aFront), aFront};
				static IRPolygon const tCorner = {itemsof(aCorner), aCorner};
				static IRGridItem const tGridBackX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 220.0f}};
				static IRGridItem const tGridBackY[] = { {EGIFInteger, 36.0f}, {EGIFInteger, 256.0f}};
				static IRCanvas const tCanvasBack = {0, 0, 256, 256, itemsof(tGridBackX), itemsof(tGridBackY), tGridBackX, tGridBackY};
				static IRGridItem const tGridFrontX[] = { {EGIFInteger, 36.0f}, {EGIFMidPixel, 101.0f}, {EGIFInteger, 256.0f}};
				static IRGridItem const tGridFrontY[] = { {EGIFInteger, 0.0f}, {EGIFMidPixel, 155.0f}, {EGIFInteger, 220.0f}};
				static IRCanvas const tCanvasFront = {0, 0, 256, 256, itemsof(tGridFrontX), itemsof(tGridFrontY), tGridFrontX, tGridFrontY};
				static IRTarget const tTarget(0.8f, -1, 1);

				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&tCanvasBack, 1, &tBack, pSI->GetMaterial(ESMAltBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tFront, pSI->GetMaterial(ESMBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tCorner, pSI->GetMaterial(ESMOutlineSoft), &tTarget);
				*a_phIcon = cRenderer.get();

				return S_OK;
			}
			else
			{
				static int const arc = 7;
				static int const extra = 5;
				static float const cx = 0.42f;
				static float const cy = 0.58f;
				static float const scale = 1.0f;
				IRPolyPoint aInner[5*4];
				IRPolyPoint aOuter[10];
				IRPolyPoint* p = aInner;
				for (int i = 0; i < 5; ++i)
				{
					float const a1 = (i*0.4f+0.05f)*3.14159265359f;
					float const a2 = (i*0.4f+0.13f)*3.14159265359f;
					float const a3 = (i*0.4f+0.27f)*3.14159265359f;
					float const a4 = (i*0.4f+0.35f)*3.14159265359f;
					p->x = cx + cosf(a1)*0.27f*scale;
					p->y = cy + sinf(a1)*0.27f*scale;
					++p;
					p->x = cx + cosf(a2)*0.42f*scale;
					p->y = cy + sinf(a2)*0.42f*scale;
					++p;
					p->x = cx + cosf(a3)*0.42f*scale;
					p->y = cy + sinf(a3)*0.42f*scale;
					++p;
					p->x = cx + cosf(a4)*0.27f*scale;
					p->y = cy + sinf(a4)*0.27f*scale;
					++p;
				}
				p = aOuter;
				for (int i = 0; i < 10; ++i)
				{
					float const a1 = (i*0.2f+0.05f)*3.14159265359f;
					p->x = cx + cosf(a1)*0.1f*scale;
					p->y = cy + sinf(a1)*0.1f*scale;
					++p;
				}
				IRPolygon const aPoly[] = { {itemsof(aInner), aInner}, {itemsof(aOuter), aOuter} };
				static IRCanvas const canvas = {0, 0, 1, 1, 0, 0, NULL, NULL};
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CComPtr<IIconRenderer> pIR;
				RWCoCreateInstance(pIR, __uuidof(IconRenderer));
				*a_phIcon = pIR->CreateIcon(a_nSize, &canvas, itemsof(aPoly), aPoly, pSI->GetMaterial(ESMInterior));
				return S_OK;
			}
			return E_FAIL;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentLayeredImage::ClipboardCheck(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ISharedState* a_pState)
{
	if (a_eAction == ERGCAPaste)
	{
		if (m_nClipboardLayers == 0) m_nClipboardLayers = RegisterClipboardFormat(_T("RWLayers"));
		if (m_nClipboardEffects == 0) m_nClipboardEffects = RegisterClipboardFormat(_T("RWCONFIG"));
		bool const bHaveEffect = IsClipboardFormatAvailable(m_nClipboardEffects);
		bool const bHaveImage = IsClipboardFormatAvailable(m_nClipboardLayers) || IsClipboardFormatAvailable(CF_BITMAP);
		return (bHaveEffect || bHaveImage) ? S_OK : S_FALSE;
	}

	try
	{
		CComQIPtr<IEnumUnknowns> pSel(a_pState);
		ULONG nLayer = 0xffffffff;
		std::set<ULONG> cIDs;
		CDocumentLayeredImage* pDoc = NULL;
		if (a_pState == NULL)
		{
			CDocumentReadLock cLock(this);
			if (!m_cLayers.empty())
				cIDs.insert(m_cLayers.rbegin()->nUID);
			pDoc = this;
		}
		else
		{
			ULONG nSel = 0;
			if (pSel) pSel->Size(&nSel);
			for (ULONG i = 0; i < nSel; ++i)
			{
				if (pDoc == NULL)
				{
					CComPtr<IStructuredItemLayerItem> p;
					if (SUCCEEDED(pSel->Get(i, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&p))))
					{
						pDoc = p->Doc();
						nLayer = 0xffffffff;
						cIDs.insert(p->ID(pDoc));
					}
					else
					{
						CComPtr<IStructuredItemLayerEffect> p;
						if (SUCCEEDED(pSel->Get(i, __uuidof(IStructuredItemLayerEffect), reinterpret_cast<void**>(&p))))
						{
							pDoc = p->Doc();
							nLayer = p->LayerID();
							cIDs.insert(p->EffectID());
						}
						else
							return E_FAIL; // invalid item in selection
					}
				}
				else if (nLayer == 0xffffffff)
				{
					CComPtr<IStructuredItemLayerItem> p;
					if (FAILED(pSel->Get(i, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&p))))
						return E_FAIL; // invalid item in selection
					if (p->Doc() != pDoc)
						return E_FAIL; // items with different parents in selection
					cIDs.insert(p->ID(pDoc));
				}
				else
				{
					CComPtr<IStructuredItemLayerEffect> p;
					if (FAILED(pSel->Get(i, __uuidof(IStructuredItemLayerEffect), reinterpret_cast<void**>(&p))))
						return E_FAIL; // invalid item in selection
					if (p->Doc() != pDoc || p->LayerID() != nLayer)
						return E_FAIL; // items with different parents in selection
					cIDs.insert(p->EffectID());
				}
			}
			if (pDoc == NULL)
				pDoc = this;
		}
		return pDoc->ClipboardCheck(a_eAction, a_hWnd, nLayer, cIDs);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::ClipboardCheck(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ULONG a_nLayerID, std::set<ULONG> a_cSelIDs)
{
	CDocumentReadLock cLock(this);

	std::set<ULONG> cAll;
	std::set<ULONG> cSel;
	ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);

	if (a_eAction == ERGCASelectAll || a_eAction == ERGCAInvertSelection)
	{
		return !cSel.empty() && cSel.size() < cAll.size() ? S_OK : S_FALSE;
	}

	if (a_eAction == ERGCADuplicate)
	{
		return (cSel.size() != 1 || a_nLayerID != 0xffffffff) ? S_FALSE : S_OK;
	}

	if (a_eAction == ERGCACopy)
	{
		return cSel.empty() ? S_FALSE : S_OK;
	}

	if (a_eAction == ERGCACut || a_eAction == ERGCADelete)
	{
		return cSel.empty() || (a_nLayerID == 0xffffffff && cAll.size() == cSel.size()) ? S_FALSE : S_OK;
	}

	return E_NOTIMPL;
}


HRESULT CDocumentLayeredImage::LayersCopy(IDocumentLayeredImage* a_pDst, IDocumentLayeredImage* a_pSrc, IEnumUnknowns* a_pItems, IEnumUnknownsInit* a_pNewItems, IComparable* a_pBefore)
{
	ULONG nItems = 0;
	if (a_pItems) a_pItems->Size(&nItems);
	CComPtr<IComparable> pBefore(a_pBefore);
	for (ULONG i = 0; i < nItems; ++i)
	{
		CComPtr<IComparable> pItem;
		a_pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		CComPtr<ISubDocumentID> pSDID;
		if (pItem) a_pSrc->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
		CComPtr<IDocument> pDoc;
		if (pSDID) pSDID->SubDocumentGet(&pDoc);
		CComPtr<IComparable> pNewItem;
		if (pDoc) a_pDst->LayerInsert(a_pBefore, ELIPAbove, &CImageLayerCreatorDocument(pDoc), &pNewItem);
		if (pNewItem) a_pNewItems->Insert(pNewItem);
		if (pItem && pNewItem)
		{
			CComPtr<IConfig> pEffect;
			a_pSrc->LayerEffectGet(pItem, &pEffect, NULL);
			a_pDst->LayerEffectSet(pNewItem, pEffect);
			CComBSTR bstrName;
			a_pSrc->LayerNameGet(pItem, &bstrName);
			a_pDst->LayerNameSet(pNewItem, bstrName);
			TImageLayer tProps;
			ELayerBlend eBlend = EBEAlphaBlend;
			BYTE bVisible = TRUE;
			a_pSrc->LayerPropsGet(pItem, &eBlend, &bVisible);
			a_pDst->LayerPropsSet(pNewItem, &eBlend, &bVisible);
			pBefore = pNewItem;
		}
	}
	return S_OK;
}

void Split(IEnumUnknowns* a_pAll, IEnumUnknowns* a_pSel, std::vector<CComPtr<IComparable> >& a_cToDelete, CComPtr<IComparable>& a_pToSelect)
{
	ULONG nAll = 0;
	if (a_pAll) a_pAll->Size(&nAll);
	ULONG nSel = 0;
	if (a_pSel) a_pSel->Size(&nSel);
	std::vector<std::pair<CComPtr<IComparable>, bool> > cAll;
	for (ULONG i = 0; i < nAll; ++i)
	{
		std::pair<CComPtr<IComparable>, bool> pItem;
		a_pAll->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem.first));
		pItem.second = false;
		cAll.push_back(pItem);
	}
	for (ULONG i = 0; i < nSel; ++i)
	{
		CComPtr<IComparable> pItem;
		a_pSel->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
		for (std::vector<std::pair<CComPtr<IComparable>, bool> >::iterator j = cAll.begin(); j != cAll.end(); ++j)
		{
			if (j->first->Compare(pItem) == S_OK)
			{
				a_cToDelete.push_back(pItem);
				j->second = true;
				break;
			}
		}
	}
	bool b = false;
	for (LONG j = cAll.size()-1; j > 0; --j)
	{
		if (!cAll[j].second && cAll[j-1].second)
		{
			a_pToSelect = cAll[j].first;
			return;
		}
		else if (cAll[j].second && !cAll[j-1].second)
		{
			a_pToSelect = cAll[j-1].first;
			return;
		}
	}
}

#include <ReturnedData.h>
#include <ImageClipboardUtils.h>

STDMETHODIMP CDocumentLayeredImage::ClipboardRun(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ISharedState* a_pState, ISharedState** a_ppNewState)
{
	try
	{
		CComQIPtr<IEnumUnknowns> pSel(a_pState);
		CDocumentLayeredImage* pDoc = NULL;
		ULONG nLayer = 0xffffffff;
		std::set<ULONG> cIDs;
		if (a_pState == NULL)
		{
			CDocumentReadLock cLock(this);
			if (!m_cLayers.empty())
				cIDs.insert(m_cLayers.rbegin()->nUID);
			pDoc = this;
		}
		else
		{
			ULONG nSel = 0;
			if (pSel) pSel->Size(&nSel);
			for (ULONG i = 0; i < nSel; ++i)
			{
				if (pDoc == NULL)
				{
					CComPtr<IStructuredItemLayerItem> p;
					if (SUCCEEDED(pSel->Get(i, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&p))))
					{
						pDoc = p->Doc();
						nLayer = 0xffffffff;
						cIDs.insert(p->ID(pDoc));
					}
					else
					{
						CComPtr<IStructuredItemLayerEffect> p;
						if (SUCCEEDED(pSel->Get(i, __uuidof(IStructuredItemLayerEffect), reinterpret_cast<void**>(&p))))
						{
							pDoc = p->Doc();
							nLayer = p->LayerID();
							cIDs.insert(p->EffectID());
						}
						else
							return E_FAIL; // invalid item in selection
					}
				}
				else if (nLayer == 0xffffffff)
				{
					CComPtr<IStructuredItemLayerItem> p;
					if (FAILED(pSel->Get(i, __uuidof(IStructuredItemLayerItem), reinterpret_cast<void**>(&p))))
						return E_FAIL; // invalid item in selection
					if (p->Doc() != pDoc)
						return E_FAIL; // items with different parents in selection
					cIDs.insert(p->ID(pDoc));
				}
				else
				{
					CComPtr<IStructuredItemLayerEffect> p;
					if (FAILED(pSel->Get(i, __uuidof(IStructuredItemLayerEffect), reinterpret_cast<void**>(&p))))
						return E_FAIL; // invalid item in selection
					if (p->Doc() != pDoc || p->LayerID() != nLayer)
						return E_FAIL; // items with different parents in selection
					cIDs.insert(p->EffectID());
				}
			}
			if (pDoc == NULL)
				pDoc = this;
		}
		return pDoc->ClipboardRun(a_eAction, a_hWnd, a_tLocaleID, nLayer, cIDs, a_ppNewState);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentLayeredImage::ClipboardSelectionHelper(ULONG a_nLayerID, std::set<ULONG> a_cSelIDs, std::set<ULONG>& a_cAll, std::set<ULONG>& a_cSel)
{
	if (a_nLayerID == 0xffffffff)
	{
		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
			a_cAll.insert(i->nUID);
	}
	else
	{
		for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == a_nLayerID)
			{
				for (CLayerEffects::const_iterator j = i->pEffects->begin(); j != i->pEffects->end(); ++j)
					a_cAll.insert(j->pItem->EffectID());
				break;
			}
		}
	}
	std::set_intersection(a_cAll.begin(), a_cAll.end(), a_cSelIDs.begin(), a_cSelIDs.end(), std::inserter(a_cSel, a_cSel.end()));
}

HRESULT CDocumentLayeredImage::ClipboardRun(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ULONG a_nLayerID, std::set<ULONG> a_cSelIDs, ISharedState** a_ppNewState)
{
	switch (a_eAction)
	{
	case ERGCAPaste:
		{
			if (a_ppNewState)
				*a_ppNewState = NULL;

			if (m_nClipboardLayers == 0)
			{
				m_nClipboardLayers = RegisterClipboardFormat(_T("RWLayers"));
				if (m_nClipboardLayers == 0)
					return E_FAIL;
			}
			if (m_nClipboardEffects == 0)
			{
				m_nClipboardEffects = RegisterClipboardFormat(_T("RWCONFIG"));
				if (m_nClipboardEffects == 0)
					return E_FAIL;
			}

			CClipboardHandler cClipboard(a_hWnd);

			bool bBitmap = false;
			bool bDIBV5 = false;
			bool bCustom = false;
			bool bEffect = false;
			ULONG iFmt = 0;
			while (true)
			{
				iFmt = EnumClipboardFormats(iFmt);
				if (iFmt == 0)
					break;
				if (iFmt == m_nClipboardLayers)
					bCustom = true;
				if (iFmt == CF_DIBV5)
					bDIBV5 = true;
				if (iFmt == CF_BITMAP)
					bBitmap = true;
				if (iFmt == m_nClipboardEffects)
					bEffect = true;
			}

			if (bCustom)
			{
				HANDLE hMem = GetClipboardData(m_nClipboardLayers);
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
					RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryLayeredImage));
					if (SUCCEEDED(pIM->DocumentCreateDataEx(pBuilder, nSize, pData, pName2, NULL, pDocBase, NULL, NULL, NULL)))
						pDoc = pDocBase;
					GlobalUnlock(hMem);
					CComPtr<IDocumentLayeredImage> pDLI;
					if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
					if (pDLI)
					{
						CComPtr<IEnumUnknowns> pItems;
						pDLI->ItemsEnum(NULL, &pItems);
						CComPtr<ISharedState> pState;
						RWCoCreateInstance(pState, __uuidof(SharedStateEnum));
						CComQIPtr<IEnumUnknownsInit> pInit(pState);
						CDocumentWriteLock cLock(this);

						std::set<ULONG> cAll;
						std::set<ULONG> cSel;
						ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);

						if (a_nLayerID == 0xffffffff && !cSel.empty())
							a_nLayerID = *cSel.begin();
						IComparable* pBefore = NULL;
						for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
							if (l->nUID == a_nLayerID)
							{
								pBefore = l->pItem;
								break;
							}

						HRESULT hRes = LayersCopy(this, pDLI, pItems, pInit, pBefore);
						if (a_ppNewState && SUCCEEDED(hRes))
							*a_ppNewState = pState.Detach();
						return hRes;
					}
				}
			}

			if (bDIBV5 || bBitmap)
			{
				CAutoVectorPtr<TPixelChannel> cBuffer;
				TImageSize tSize = {0, 0};
				if (GetClipboardImage(tSize, cBuffer))
				{
					CComPtr<IComparable> pItem;
					CDocumentWriteLock cLock(this);

					std::set<ULONG> cAll;
					std::set<ULONG> cSel;
					ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);

					if (a_nLayerID == 0xffffffff && !cSel.empty())
						a_nLayerID = *cSel.begin();
					IComparable* pBefore = NULL;
					for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
						if (l->nUID == a_nLayerID)
						{
							pBefore = l->pItem;
							break;
						}

					HRESULT hRes = LayerInsert(pBefore, ELIPAbove, &CImageLayerCreatorRasterImage(tSize, cBuffer.m_p), &pItem);
					if (SUCCEEDED(hRes) && pItem)
					{
						if (a_ppNewState)
						{
							CComPtr<ISharedState> pTmp;
							RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
							CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
							pInit->Insert(pItem);
							*a_ppNewState = pTmp.Detach();
						}
						CComBSTR bstrName;
						CMultiLanguageString::GetLocalized(L"[0409]Pasted[0405]Vložený", a_tLocaleID, &bstrName);
						LayerNameSet(pItem, bstrName);
					}
					return hRes;
				}
			}

			if (bEffect)
			{
				HANDLE hMem = GetClipboardData(m_nClipboardEffects);
				if (hMem)
				{
					ULONG const* pMem = reinterpret_cast<ULONG const*>(GlobalLock(hMem));
					CComPtr<IConfigInMemory> pMemCfg;
					RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
					pMemCfg->DataBlockSet(*pMem, reinterpret_cast<BYTE const*>(pMem+1));

					GlobalUnlock(hMem);

					CDocumentWriteLock cLock(this);

					std::set<ULONG> cAll;
					std::set<ULONG> cSel;
					ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);

					CLayerEffects cEffects;
					{
						CComPtr<IConfigWithDependencies> pCfg;
						RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
						M_OpMgr()->InsertIntoConfigAs(M_OpMgr(), pCfg, CComBSTR(CFGID_LAYEREFFECT), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Operation applied on the selected image layer.[0405]Operace aplikovaná na vybranou vrstvu obrázku."), 0, NULL);
						CConfigCustomGUI<&CLSID_DocumentFactoryLayeredImage, CConfigGUILayoutEffectDlg>::FinalizeConfig(pCfg);
						CopyConfigValues(pCfg, pMemCfg);
						DecodeStyle(pMemCfg, cEffects);
					}

					if (a_nLayerID == 0xffffffff)
					{
						// pasted effects on layer -> replace whole style
						for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
						{
							if (cSel.find(l->nUID) != cSel.end())
							{
								LayerEffectSet(l->nUID, cEffects, false);
							}
						}
						// TODO: select the inserted effects?
					}
					else
					{
						for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
						{
							if (l->nUID == a_nLayerID)
							{
								// replace style if we are pasting equal number of effect and they have the same types
								bool replace = cSel.size() == cEffects.size();
								if (replace)
								{
									CLayerEffects::const_iterator j = l->pEffects->begin();
									size_t i = 0;
									while (j != l->pEffects->end())
									{
										if (cSel.find(j->pItem->EffectID()) != cSel.end())
										{
											if (IsEqualGUID(j->tOpID, cEffects[i].tOpID))
											{
												++i;
												if (i == cSel.size())
													break;
											}
											else
											{
												replace = false;
												break;
											}
										}
										++j;
									}
								}
								if (replace)
								{
									std::set<ULONG>::const_iterator s = cSel.begin();
									for (size_t i = 0; i < cSel.size(); ++i, ++s)
									{
										LayerEffectStepSet(a_nLayerID, *s, &cEffects[i].bEnabled, &cEffects[i].tOpID, cEffects[i].pOpCfg);
									}
								}
								else
								{
									CComPtr<ISharedState> pTmp;
									RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
									CComQIPtr<IEnumUnknownsInit> pInit(pTmp);

									ULONG nBeforeID = 0xffffffff;
									bool after = false;
									for (CLayerEffects::const_iterator j = l->pEffects->begin(); j != l->pEffects->end(); ++j)
									{
										if (cSel.find(j->pItem->EffectID()) != cSel.end())
										{
											after = true;
										}
										else if (after)
										{
											nBeforeID = j->pItem->EffectID();
											after = false;
										}
									}
									for (CLayerEffects::const_iterator i = cEffects.begin(); i != cEffects.end(); ++i)
									{
										CComPtr<IComparable> pStep;
										LayerEffectStepInsert(a_nLayerID, nBeforeID, i->bEnabled, i->tOpID, i->pOpCfg, &pStep);
										pInit->Insert(pStep);
									}

									if (a_ppNewState)
									{
										*a_ppNewState = pTmp;
										pTmp.Detach();
									}
								}
								break;
							}
						}
					}
					return S_OK;

					//HRESULT hRes = E_FAIL;
					//for (ULONG i = 0; i < nItems; ++i)
					//{
					//	CComPtr<IComparable> pItem;
					//	m_pSel->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
					//	CComPtr<IConfig> pConfig;
					//	m_pDLI->LayerEffectGet(pItem, &pConfig, NULL);
					//	if (pConfig == NULL) continue;
					//	CopyConfigValues(pConfig, pMemCfg);
					//	if (pItem && SUCCEEDED(m_pDLI->LayerEffectSet(pItem, pConfig)))
					//		hRes = S_OK;
					//}
					//return hRes;
				}
			}

			return E_FAIL;
		}
	case ERGCACopy:
	case ERGCACut:
		{
			CAutoPtr<CDocumentReadLock> pRead;
			CAutoPtr<CDocumentWriteLock> pWrite;
			if (a_eAction == ERGCACut)
				pWrite.Attach(new CDocumentWriteLock(this));
			else
				pRead.Attach(new CDocumentReadLock(this));

			std::set<ULONG> cAll;
			std::set<ULONG> cSel;

			ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);
			if (cSel.empty())
				return S_FALSE;

			if (a_nLayerID == 0xffffffff)
			{
				if (m_nClipboardLayers == 0)
				{
					m_nClipboardLayers = RegisterClipboardFormat(_T("RWLayers"));
					if (m_nClipboardLayers == 0)
						return E_FAIL;
				}

				if (cSel.empty())
					return S_FALSE; // nothing to copy
				if (a_eAction == ERGCACut && cSel.size() == cAll.size())
					return E_FAIL; // cutting all is not allowed
				CComPtr<IComparable> p1stItem;
				for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
					if (cSel.find(l->nUID) != cSel.end())
					{
						p1stItem = l->pItem;
						break;
					}

				CComQIPtr<ISubDocumentID> pSDID(p1stItem);
				if (pSDID == NULL)
					return S_FALSE; // weird

				CClipboardHandler cClipboard(a_hWnd);
				EmptyClipboard();

				CComPtr<IDocument> p1stDoc;
				pSDID->SubDocumentGet(&p1stDoc);
				CComPtr<IDocumentImage> pDocImage;
				p1stDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImage));
				if (pDocImage)
				{
					TImageSize tSize = {0, 0};
					pDocImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

					HANDLE hMem = GlobalAlloc(GHND, sizeof(BITMAPV5HEADER)+(tSize.nX*tSize.nY)*sizeof(TRasterImagePixel)+3*sizeof(RGBQUAD));
					if (hMem)
					{
						BITMAPV5HEADER* pHeader = reinterpret_cast<BITMAPV5HEADER*>(GlobalLock(hMem));
						ZeroMemory(pHeader, sizeof *pHeader);
						pHeader->bV5Size = sizeof *pHeader;
						pHeader->bV5Width = tSize.nX;
						pHeader->bV5Height = -tSize.nY;
						pHeader->bV5Planes = 1;
						pHeader->bV5BitCount = 32;
						pHeader->bV5Compression = BI_BITFIELDS;
						pHeader->bV5SizeImage = 4*tSize.nX*tSize.nY;
						//pHeader->bV5XPelsPerMeter = 0;
						//pHeader->bV5YPelsPerMeter = 0;
						//pHeader->bV5ClrUsed = 0;
						//pHeader->bV5ClrImportant = 0;
						pHeader->bV5RedMask = 0x00ff0000;
						pHeader->bV5GreenMask = 0x0000ff00;
						pHeader->bV5BlueMask = 0x000000ff;
						pHeader->bV5AlphaMask = 0xff000000;
						pHeader->bV5CSType = LCS_sRGB;
						pHeader->bV5Intent = LCS_GM_IMAGES;

						pDocImage->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pHeader+1), NULL, EIRIPreview);
						SetClipboardData(CF_DIBV5, hMem);

						GlobalUnlock(hMem);
					}
				}

				CComPtr<IEnumUnknownsInit> pNewItems;
				RWCoCreateInstance(pNewItems, __uuidof(EnumUnknowns));
				CComPtr<IDocumentBase> pBase;
				RWCoCreateInstance(pBase, __uuidof(DocumentBase));
				CComObject<CDocumentLayeredImage>* pDoc = NULL;
				CComObject<CDocumentLayeredImage>::CreateInstance(&pDoc);
				CComPtr<IDocumentData> pTmp = pDoc;
				pBase->DataBlockSet(NULL, pDoc);
				pDoc->CanvasSet(&m_tSize, &m_tResolution, NULL, NULL);
				{
					CComPtr<IEnumUnknownsInit> pItems;
					RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
					for (CLayers::const_iterator l = m_cLayers.begin(); l != m_cLayers.end(); ++l)
						if (cSel.find(l->nUID) != cSel.end())
						{
							pItems->Insert(l->pItem);
						}
					LayersCopy(pDoc, this, pItems, pNewItems);
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
				SetClipboardData(m_nClipboardLayers, hMem);

				if (a_eAction == ERGCACut)
				{
					IComparable* pNew = NULL;
					bool found = false;
					for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
					{
						if (cSel.find(i->nUID) != cSel.end())
						{
							if (pNew)
								break;
							found = true;
							continue;
						}
						pNew = static_cast<IUIItem*>(i->pItem);
						if (found)
							break;
					}

					CAutoVectorPtr<ULONG> pIDs(new ULONG[cSel.size()]);
					std::copy(cSel.begin(), cSel.end(), pIDs.m_p);
					DeleteLayers(cSel.size(), pIDs);

					if (a_ppNewState && pNew)
					{
						CComPtr<ISharedState> pStateNew;
						StatePack(1, &pNew, &pStateNew);
						*a_ppNewState = pStateNew.Detach();
					}
				}
			}
			else
			{
				if (m_nClipboardEffects == 0)
				{
					m_nClipboardEffects = RegisterClipboardFormat(_T("RWCONFIG"));
					if (m_nClipboardEffects == 0)
						return E_FAIL;
				}

				CLayers::const_iterator l = m_cLayers.begin();
				while (l != m_cLayers.end() && l->nUID != a_nLayerID)
					++l;
				if (l == m_cLayers.end())
					return E_FAIL;

				CLayerEffects cToCopy;
				for (CLayerEffects::const_iterator i = l->pEffects->begin(); i != l->pEffects->end(); ++i)
				{
					if (cSel.find(i->pItem->EffectID()) == cSel.end())
						continue;
					SLayerEffect sLE;
					sLE.bEnabled = i->bEnabled;
					sLE.tOpID = i->tOpID;
					sLE.pOpCfg = i->pOpCfg;
					cToCopy.push_back(sLE);
					sLE.pOpCfg->AddRef();
				}

				CComPtr<IConfigWithDependencies> pCfg;
				RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
				M_OpMgr()->InsertIntoConfigAs(M_OpMgr(), pCfg, CComBSTR(CFGID_LAYEREFFECT), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Operation applied on the selected image layer.[0405]Operace aplikovaná na vybranou vrstvu obrázku."), 0, NULL);
				CConfigCustomGUI<&CLSID_DocumentFactoryLayeredImage, CConfigGUILayoutEffectDlg>::FinalizeConfig(pCfg);
				EncodeStyle(cToCopy.begin(), cToCopy.end(), pCfg);

				CComPtr<IConfigInMemory> pMemCfg;
				RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
				CopyConfigValues(pMemCfg, pCfg);
				ULONG nSize = 0;
				pMemCfg->DataBlockGetSize(&nSize);
				if (nSize == 0)
					return 0;

				CClipboardHandler cClipboard(a_hWnd);
				EmptyClipboard();

				HANDLE hMem = GlobalAlloc(GHND, nSize+sizeof nSize);
				if (hMem == NULL)
					return 0;

				BYTE* pMem = reinterpret_cast<BYTE*>(GlobalLock(hMem));
				*reinterpret_cast<ULONG*>(pMem) = nSize;
				pMemCfg->DataBlockGet(nSize, pMem+sizeof nSize);

				GlobalUnlock(hMem);

				SetClipboardData(m_nClipboardEffects, hMem);

				if (a_eAction == ERGCACut)
				{
					IComparable* pNew = NULL;
					bool found = false;
					for (CLayerEffects::const_iterator i = l->pEffects->begin(); i != l->pEffects->end(); ++i)
					{
						if (cSel.find(i->pItem->EffectID()) != cSel.end())
						{
							if (pNew)
								break;
							found = true;
							continue;
						}
						pNew = static_cast<IUIItem*>(i->pItem);
						if (found)
							break;
					}

					for (std::set<ULONG>::const_iterator i = cSel.begin(); i != cSel.end(); ++i)
						LayerEffectStepDelete(a_nLayerID, *i);

					if (pNew == NULL)
						pNew = l->pItem;

					if (a_ppNewState && pNew)
					{
						CComPtr<ISharedState> pStateNew;
						StatePack(1, &pNew, &pStateNew);
						*a_ppNewState = pStateNew.Detach();
					}
				}
			}
		}
		return S_OK;
	case ERGCADelete:
		{
			CDocumentWriteLock cLock(this);

			std::set<ULONG> cAll;
			std::set<ULONG> cSel;

			ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);
			if (cSel.empty())
				return S_FALSE;

			if (a_nLayerID == 0xffffffff)
			{
				if (cSel.empty())
					return S_FALSE; // nothing to copy
				if (cSel.size() == cAll.size())
					return E_FAIL; // cutting all is not allowed

				IComparable* pNew = NULL;
				bool found = false;
				for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
				{
					if (cSel.find(i->nUID) != cSel.end())
					{
						if (pNew)
							break;
						found = true;
						continue;
					}
					pNew = static_cast<IUIItem*>(i->pItem);
					if (found)
						break;
				}

				CAutoVectorPtr<ULONG> pIDs(new ULONG[cSel.size()]);
				std::copy(cSel.begin(), cSel.end(), pIDs.m_p);
				DeleteLayers(cSel.size(), pIDs);

				if (a_ppNewState && pNew)
				{
					CComPtr<ISharedState> pStateNew;
					StatePack(1, &pNew, &pStateNew);
					*a_ppNewState = pStateNew.Detach();
				}
			}
			else
			{
				CLayers::const_iterator l = m_cLayers.begin();
				while (l != m_cLayers.end() && l->nUID != a_nLayerID)
					++l;
				if (l == m_cLayers.end())
					return E_FAIL;

				IComparable* pNew = NULL;
				bool found = false;
				for (CLayerEffects::const_iterator i = l->pEffects->begin(); i != l->pEffects->end(); ++i)
				{
					if (cSel.find(i->pItem->EffectID()) != cSel.end())
					{
						if (pNew)
							break;
						found = true;
						continue;
					}
					pNew = static_cast<IUIItem*>(i->pItem);
					if (found)
						break;
				}

				for (std::set<ULONG>::const_iterator i = cSel.begin(); i != cSel.end(); ++i)
					LayerEffectStepDelete(a_nLayerID, *i);

				if (pNew == NULL)
					pNew = l->pItem;

				if (a_ppNewState && pNew)
				{
					CComPtr<ISharedState> pStateNew;
					StatePack(1, &pNew, &pStateNew);
					*a_ppNewState = pStateNew.Detach();
				}
			}
		}
		return S_OK;
	case ERGCADuplicate:
		{
			*a_ppNewState = NULL;
			CDocumentWriteLock cLock(this);

			std::set<ULONG> cAll;
			std::set<ULONG> cSel;

			ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);
			if (cSel.empty())
				return S_FALSE;

			CComPtr<ISharedState> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
			CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
			if (a_nLayerID == 0xffffffff)
			{
				for (std::set<ULONG>::const_iterator iS = cSel.begin(); iS != cSel.end(); ++iS)
				{
					CLayers::const_iterator i = m_cLayers.begin();
					while (i != m_cLayers.end() && i->nUID != *iS) ++i;
					CComPtr<IComparable> pItem(i->pItem);
					CComPtr<IComparable> pNew;
					HRESULT hRes = LayerInsert(pItem, ELIPAbove, &CImageLayerCreatorDocument(this, pItem), &pNew);
					if (SUCCEEDED(hRes) && pNew)
					{
						CComBSTR bstrName;
						LayerNameGet(pItem, &bstrName);
						CComBSTR bstrTempl;
						CMultiLanguageString::GetLocalized(L"[0409]Copy of %s[0405]Kopie - %s", a_tLocaleID, &bstrTempl);
						wchar_t sz[1024] = L"";
						swprintf(sz, itemsof(sz), bstrTempl.m_str, bstrName == NULL ? L"" : bstrName.m_str);
						sz[itemsof(sz)-1] = L'\0';
						LayerNameSet(pNew, CComBSTR(sz));
						ELayerBlend eBlend = EBEAlphaBlend;
						BYTE bVisible = 1;
						LayerPropsGet(pItem, &eBlend, &bVisible);
						LayerPropsSet(pNew, &eBlend, &bVisible);
						CComPtr<IConfig> pEffect;
						LayerEffectGet(pItem, &pEffect, NULL);
						LayerEffectSet(pNew, pEffect);
						pInit->Insert(pNew);
					}
				}
				//CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID, pState);
			}
			*a_ppNewState = pTmp.Detach();
			return S_OK;
		}
	case ERGCASelectAll:
		{
			*a_ppNewState = NULL;
			CDocumentReadLock cLock(this);
			CComPtr<ISharedState> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
			CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
			if (a_nLayerID != 0xffffffff)
			{
				for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
				{
					if (i->nUID == a_nLayerID)
					{
						for (CLayerEffects::const_reverse_iterator e = i->pEffects->rbegin(); e != i->pEffects->rend(); ++e)
						{
							pInit->Insert(static_cast<IUIItem*>(e->pItem));
						}
					}
				}
			}
			else
			{
				for (CLayers::const_reverse_iterator i = m_cLayers.rbegin(); i != m_cLayers.rend(); ++i)
				{
					pInit->Insert(i->pItem);
				}
			}
			*a_ppNewState = pTmp.Detach();
			return S_OK;
		}
	case ERGCAInvertSelection:
		{
			*a_ppNewState = NULL;
			CDocumentReadLock cLock(this);

			std::set<ULONG> cAll;
			std::set<ULONG> cSel;

			ClipboardSelectionHelper(a_nLayerID, a_cSelIDs, cAll, cSel);

			CComPtr<ISharedState> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
			CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
			if (a_nLayerID != 0xffffffff)
			{
				for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
				{
					if (i->nUID == a_nLayerID)
					{
						for (CLayerEffects::const_reverse_iterator e = i->pEffects->rbegin(); e != i->pEffects->rend(); ++e)
						{
							if (cSel.find(e->pItem->EffectID()) != cSel.end())
								continue;
							pInit->Insert(static_cast<IUIItem*>(e->pItem));
						}
					}
				}
			}
			else
			{
				for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
				{
					if (cSel.find(i->nUID) != cSel.end())
						continue;
					pInit->Insert(i->pItem);
				}
			}
			*a_ppNewState = pTmp.Detach();
			return S_OK;
		}
	}
	return E_RW_INVALIDPARAM;
}

//#include "..\..\RWProcessing\RWOperationImageRaster\Resampling.h"

STDMETHODIMP CDocumentLayeredImage::Thumbnail(IComparable* a_pItem, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, ULONG* a_pTimestamp)
{
	return E_NOTIMPL;
	try
	{
		CComQIPtr<IStructuredItemLayerItem> pLI(a_pItem);
		ULONG nID = -1;
		if (pLI != NULL)
			nID = pLI->ID(this);
		if (nID == -1)
			return E_RW_ITEMNOTFOUND;
		CDocumentReadLock cLock(this);
		for (CLayers::iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
		{
			if (i->nUID == nID)
			{
				if (a_pTimestamp)
					*a_pTimestamp = i->nTimestamp;

				if (a_pBGRAData)
				{
					SCacheEntryPtr pCached = QueryLayerCache(i->nUID);
					//pCached.Attach(QueryCache(static_cast<ECachePhase>(ECPThumbnail|((a_nSizeX-1)<<16)|((a_nSizeY-1)<<24)), i->nUID));
					//if (pCached == NULL || pCached->tSize.nX > a_nSizeX || pCached->tSize.nY > a_nSizeY)
					//	return E_FAIL;
					//ULONG nIcoOffX = (a_nSizeX-pCached->tSize.nX)>>1;
					//ULONG nIcoOffY = (a_nSizeY-pCached->tSize.nY)>>1;
					//if (a_prcBounds)
					//{
					//	a_prcBounds->left = nIcoOffX;
					//	a_prcBounds->right = nIcoOffX+pCached->tSize.nX;
					//	a_prcBounds->top = nIcoOffY;
					//	a_prcBounds->bottom = nIcoOffY+pCached->tSize.nY;
					//}
					//ZeroMemory(a_pBGRAData, sizeof(*a_pBGRAData)*a_nSizeX*a_nSizeY);
					//TPixelChannel const* pS = pCached->pPixels;
					//for (ULONG y = 0; y < pCached->tSize.nY; ++y)
					//{
					//	CopyMemory(a_pBGRAData+(y+nIcoOffY)*a_nSizeX+nIcoOffX, pS, pCached->tSize.nX*sizeof*a_pBGRAData);
					//	pS += pCached->tSize.nX;
					//}
				}
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

HRESULT CDocumentLayeredImage::LayerBuilderIDGet(ULONG a_nLayerID, GUID* a_pBuilderID)
{
	CDocumentReadLock cLock(this);
	CComBSTR bstr;
	SLayer::GetLayerID(this, a_nLayerID, bstr);
	CComPtr<IDocument> pDoc;
	M_Base()->DataBlockDoc(bstr, &pDoc);
	if (pDoc == NULL)
		return E_FAIL;
	return pDoc->BuilderID(a_pBuilderID);
}

HRESULT CDocumentLayeredImage::LayerIconIDGet(ULONG a_nLayerID, GUID* a_pIconID)
{
	CDocumentReadLock cLock(this);
	SLayer const& sLayer = FindLayer(a_nLayerID);
	CComBSTR bstr;
	sLayer.GetLayerID(this, bstr);
	CComPtr<ILayerType> pLT;
	M_Base()->DataBlockGet(bstr, __uuidof(ILayerType), reinterpret_cast<void**>(&pLT));
	if (pLT) return pLT->IconID(a_pIconID);
	return E_RW_ITEMNOTFOUND;
}

HRESULT CDocumentLayeredImage::LayerIconGet(ULONG a_nLayerID, ULONG a_nSize, HICON* a_phIcon)
{
	CDocumentReadLock cLock(this);
	SLayer const& sLayer = FindLayer(a_nLayerID);
	CComBSTR bstr;
	sLayer.GetLayerID(this, bstr);
	CComPtr<ILayerType> pLT;
	M_Base()->DataBlockGet(bstr, __uuidof(ILayerType), reinterpret_cast<void**>(&pLT));
	if (pLT) return pLT->Icon(a_nSize, a_phIcon);
	return E_RW_ITEMNOTFOUND;
	//CDocumentReadLock cLock(this);
	//CComBSTR bstr;
	//SLayer::GetLayerID(this, a_nLayerID, bstr);
	//CComPtr<IDocument> pDoc;
	//M_Base()->DataBlockDoc(bstr, &pDoc);
	//GUID tBuilderID = GUID_NULL;
	//if (pDoc == NULL || FAILED(pDoc->BuilderID(&tBuilderID)))
	//	return E_FAIL;
	//CComPtr<IDocumentBuilder> pBuilder;
	//RWCoCreateInstance(pBuilder, tBuilderID);
	//return pBuilder ? pBuilder->Icon(a_nSize, a_phIcon) : E_FAIL;
}

HRESULT CDocumentLayeredImage::ComposedPreviewAdjustTile(ULONG a_nLayerID, bool a_bAdjustDirty, RECT* a_prc)
{
	try
	{
		CDocumentReadLock cLock(this);
		for (CLayers::const_iterator iL = m_cLayers.begin(); iL != m_cLayers.end(); ++iL)
		{
			if (iL->nUID == a_nLayerID)
			{
				if (iL->pDocImg == NULL)
					return E_FAIL;

				TRasterImageRect tR = {{a_prc->left, a_prc->top}, {a_prc->right, a_prc->bottom}};
				for (CLayerEffects::const_iterator iE = iL->pEffects->begin(); iE != iL->pEffects->end(); ++iE)
				{
					if (!iE->bEnabled || IsEqualGUID(iE->tOpID, GUID_NULL) || IsEqualGUID(iE->tOpID, __uuidof(DocumentOperationNULL)) || iE->pOp == NULL)
						continue;
					CComQIPtr<IRasterImageFilter> pRIF(iE->pOp);
					if (pRIF)
					{
						if (a_bAdjustDirty)
							pRIF->AdjustDirtyRect(iE->pOpCfg, &m_tSize, &tR);
						else
							pRIF->NeededToCompute(iE->pOpCfg, &m_tSize, &tR);
					}
					else
					{
						 // TODO: remove this when all filters implement IRasterImageFilter
						ATLASSERT(0/* no IRasterImageFilter support in layer effect*/);
						if (tR.tTL.nX > 0) tR.tTL.nX = 0;
						if (tR.tTL.nY > 0) tR.tTL.nY = 0;
						if (tR.tBR.nX < LONG(m_tSize.nX)) tR.tBR.nX = m_tSize.nX;
						if (tR.tBR.nY < LONG(m_tSize.nY)) tR.tBR.nY = m_tSize.nY;
					}
				}
				a_prc->left = tR.tTL.nX;
				a_prc->top = tR.tTL.nY;
				a_prc->right = tR.tBR.nX;
				a_prc->bottom = tR.tBR.nY;
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

inline BYTE IntHLS(unsigned const a_nMin, unsigned const a_nMax, unsigned const a_nAngle)
{
	unsigned a = (a_nAngle+0x2aaa)&0xffff;
	if (a >= 0x8000) a = 0xffff-a;
	if (a < 0x2aaa) a = 0;
	else if (a >= 0x5555) a = 0x2aaa;
	else a -= 0x2aaa;
	return (a_nMin + (a_nMax-a_nMin)*a/0x2aaa)>>8;
}

HRESULT CDocumentLayeredImage::ComposedPreviewProcessTile(ULONG a_nLayerID, EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData)
{
	try
	{
		// TODO: optimize case when tile is same as the saved layer data (-> return fully merged data)

		CDocumentReadLock cLock(this);
		bool bTrivial = true;
		for (CLayers::const_iterator iL = m_cLayers.begin(); iL != m_cLayers.end(); ++iL)
		{
			if (iL->nUID == a_nLayerID)
			{
				for (CLayerEffects::const_iterator iE = iL->pEffects->begin(); iE != iL->pEffects->end(); ++iE)
				{
					if (!iE->bEnabled || IsEqualGUID(iE->tOpID, GUID_NULL) || IsEqualGUID(iE->tOpID, __uuidof(DocumentOperationNULL)) || iE->pOp == NULL)
						continue;
					bTrivial = false;
					break;
				}
				if (!bTrivial)
					break;
			}
			else
			{
				if (iL->bVisible)
				{
					bTrivial = false;
					break;
				}
			}
		}
		if (bTrivial)
			return S_FALSE; // only a single normal layer -> nothing to do

		CLayers::const_iterator i = m_cLayers.begin();
		while (i != m_cLayers.end() && i->nUID != a_nLayerID)
			++i;
		if (i == m_cLayers.end())
			return E_RW_ITEMNOTFOUND; // layer not found

		if (i->bVisible)
		{
			// apply layer effect

			CComPtr<IDocumentImage> pCur;

			CComObjectStackEx<CLayerOperationContext> cLOC;
			for (CLayerEffects::const_iterator iE = i->pEffects->begin(); iE != i->pEffects->end(); ++iE)
			{
				if (!iE->bEnabled || IsEqualGUID(iE->tOpID, GUID_NULL) || IsEqualGUID(iE->tOpID, __uuidof(DocumentOperationNULL)) || iE->pOp == NULL)
					continue;
				if (pCur)
				{
					CComObjectStackEx<CRasterImageOperationStep> cStepSrc;
					cStepSrc.Init(pCur, NULL, NULL);
					iE->pOp->Activate(M_OpMgr(), &cStepSrc, iE->pOpCfg, &cLOC, NULL, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));
					CComPtr<IDocument> pDoc;
					cStepSrc.SwapRes(pDoc, pCur);
				}
				else
				{
					TPixelChannel tDef = {0, 0, 0, 0};
					i->pDocImg->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tDef));
					CComObjectStackEx<CRasterImageOperationStepRectangle> cStepSrc;
					cStepSrc.Init(m_tSize, reinterpret_cast<TPixelChannel*>(a_pData), CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), a_nStride, tDef);
					iE->pOp->Activate(M_OpMgr(), &cStepSrc, iE->pOpCfg, &cLOC, NULL, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));
					cStepSrc.SwapRes(pCur);
				}
			}

			if (pCur)
				pCur->TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pData), NULL, EIRIAccurate);

			//if (cEffectID.TypeGet() == ECVTGUID && !IsEqualGUID(__uuidof(DocumentOperationNULL), cEffectID))
			//{
			//	CComPtr<SCacheEntry> pBg;
			//	pBg.Attach(QueryCache(ECPRaw, i->nUID));
			//	CComPtr<IConfig> pEffect;
			//	pStyle->SubConfigGet(bstrCFGID_LAYEREFFECT, &pEffect);
			//	CComObjectStackEx<CLayerRasterImageRectangle> cLRI;
			//	TRasterImageRect tRect = { {a_nX, a_nY}, {a_nX+a_nSizeX, a_nY+a_nSizeY} };
			//	cLRI.Init(m_tSize, 2.2f, pBg->pPixels, pBg->tOffset, pBg->tSize, pBg->tDefault, reinterpret_cast<TPixelChannel*>(a_pData), tRect, a_nStride);
			//	CComObjectStackEx<CLayerOperationContext> cLOC;
			//	M_OpMgr()->Activate(M_OpMgr(), &cLRI, cEffectID, pEffect, &cLOC, NULL, 0);
			//}
		}
		else
		{
			// current layer is not visible -> return complete image
			return TileGet(EICIRGBA, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pData), NULL, EIRIAccurate);
			//SCacheEntryPtr pWhole = ;
			//pWhole.Attach(QueryCache(ECPMerged, m_cLayers[m_cLayers.size()-1].nUID));
			//return RGBAGetTileImpl(m_tSize, pWhole->tOffset, pWhole->tSize, pWhole->pPixels, pWhole->tSize.nX, pWhole->tDefault,
			//					   CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pData));
		}

		std::vector<ULONG> belowIds;
		std::vector<SCacheEntryPtr> belowChunks;
		ToChunks(m_cLayers.begin(), i, belowIds, belowChunks);
		SCacheEntryPtr pBg;
		if (!belowIds.empty())
			pBg = QueryMergedCache(belowIds, &belowChunks);
		if (pBg == NULL)
		{
			// is it OK?
			//if (i->tData.eBlend != EBEAlphaBlend)
			//{
			//	CAutoVectorPtr<TRasterImagePixel> cEmpty(new TRasterImagePixel[a_nSizeX]);
			//	ZeroMemory(cEmpty.m_p, a_nSizeX*sizeof*cEmpty.m_p);
			//	for (ULONG y = 0; y < a_nSizeY; ++y)
			//		MergeLayerLine(i->tData.eBlend, a_nSizeX, a_pData+y*a_nStride, cEmpty.m_p, a_pData+y*a_nStride, m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
			//}
		}
		else
		{
			TPixelChannel tDef;
			tDef.n = a_eMode == ECPMTransparent ? (pBg->tDefault.n&0xffffff)|((pBg->tDefault.n>>1)&0x7f000000) : pBg->tDefault.n;
			CAutoVectorPtr<TPixelChannel> cLine(a_eMode == ECPMTransparent ? new TPixelChannel[a_nSizeX] : NULL);
			TPixelChannel* pDst = reinterpret_cast<TPixelChannel*>(a_pData);

			TPixelChannel const* pData = NULL;
			TImagePoint tAllocOrigin = {0, 0};
			TImageSize tAllocSize = {0, 0};
			pBg->pImage->BufferLock(EICIRGBA, &tAllocOrigin, &tAllocSize, NULL, NULL, &pData, NULL, EIRIAccurate);

			LONG const nX0 = pBg->tBounds.tBR.nX > pBg->tBounds.tTL.nX && a_nX > pBg->tBounds.tTL.nX ? a_nX : pBg->tBounds.tTL.nX;
			LONG const nX1 = pBg->tBounds.tBR.nX > pBg->tBounds.tTL.nX && a_nX+LONG(a_nSizeX) < pBg->tBounds.tBR.nX ? a_nX+LONG(a_nSizeX) : pBg->tBounds.tBR.nX;
			for (LONG y = a_nY; y < a_nY+LONG(a_nSizeY); ++y)
			{
				if (nX0 < nX1 && y >= pBg->tBounds.tTL.nY && y < pBg->tBounds.tBR.nY)
				{
					if (nX0 > a_nX)
					{
						MergeLayerLine(i->eBlendingMode, nX0-a_nX, pDst, tDef, pDst, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
						pDst += nX0-a_nX;
					}
					TPixelChannel const* pS = pData+(y-tAllocOrigin.nY)*tAllocSize.nX+nX0-tAllocOrigin.nX;
					if (a_eMode == ECPMTransparent)
					{
						TPixelChannel* pD = cLine;
						for (TPixelChannel const* const pEnd = pS+(nX1-nX0); pS < pEnd; ++pS, ++pD)
							pD->n = (pS->n&0xffffff)|((pS->n>>1)&0x7f000000);
						pS = cLine;
					}
					MergeLayerLine(i->eBlendingMode, nX1-nX0, pDst, pS, pDst, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					pDst += nX1-nX0;
					if (a_nX+LONG(a_nSizeX) > nX1)
					{
						MergeLayerLine(i->eBlendingMode, a_nX+LONG(a_nSizeX)-nX1, pDst, tDef, pDst, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
						pDst += a_nX+LONG(a_nSizeX)-nX1;
					}
				}
				else
				{
					MergeLayerLine(i->eBlendingMode, a_nSizeX, pDst, tDef, pDst, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
					pDst += a_nSizeX;
				}
				pDst += a_nStride-a_nSizeX;
			}
			pBg->pImage->BufferUnlock(EICIRGBA, pData);
		}

		//CAutoVectorPtr<TRasterImagePixel> pBackup;

		std::vector<ULONG> aboveIds;
		std::vector<SCacheEntryPtr> aboveChunks;
		ToChunks(i+1, m_cLayers.end(), aboveIds, aboveChunks);

		for (std::vector<SCacheEntryPtr>::const_iterator iA = aboveChunks.begin(); iA != aboveChunks.end(); ++iA)
		{
			TRasterImageRect tBounds = {{a_nX, a_nY}, {a_nX+a_nSizeX, a_nY+a_nSizeY}};
			TPixelChannel* p = reinterpret_cast<TPixelChannel*>(a_pData);
			if (a_eMode == ECPMTransparent)
			{
				// TODO: only copy and adjust the needed rectangle
				TImageSize tSrcSize = {(*iA)->tBounds.tBR.nX-(*iA)->tBounds.tTL.nX, (*iA)->tBounds.tBR.nY-(*iA)->tBounds.tTL.nY};
				CAutoVectorPtr<TPixelChannel> cBuffer(tSrcSize.nX*tSrcSize.nY ? new TPixelChannel[tSrcSize.nX*tSrcSize.nY] : NULL);
				if (cBuffer.m_p)
				{
					(*iA)->pImage->TileGet(EICIRGBA, &(*iA)->tBounds.tTL, &tSrcSize, NULL, tSrcSize.nX*tSrcSize.nY, cBuffer, NULL, EIRIAccurate);
					TPixelChannel* pD = cBuffer;
					for (TPixelChannel* const pEnd = pD+tSrcSize.nX*tSrcSize.nY; pD < pEnd; ++pD)
						pD->n = (pD->n&0xffffff)|((pD->n>>1)&0x7f000000);
				}
				CComPtr<IDocument> pDoc;
				RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
				CComObject<CDocumentRasterImage>* pImg = NULL;
				CComObject<CDocumentRasterImage>::CreateInstance(&pImg);
				CComPtr<IDocumentData> pData(pImg);
				pImg->Init(m_tSize, NULL, NULL, (*iA)->tBounds.tTL, tSrcSize, cBuffer, 2.2f, NULL);
				CComQIPtr<IDocumentBase>(pDoc)->DataBlockSet(NULL, pData);
				CComPtr<IDocumentImage> pFinal;
				pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFinal));
				MergeRectangle(tBounds, a_nSizeX, p, CPixelChannel(0, 0, 0, 0), (*iA)->eBlend, pFinal);
			}
			else
			{
				MergeRectangle(tBounds, a_nSizeX, p, CPixelChannel(0, 0, 0, 0), (*iA)->eBlend, (*iA)->pImage);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentLayeredImage::ComposedPreviewInputTransform(ULONG a_nLayerID, TMatrix3x3f* a_pTransform)
{
	try
	{
		CDocumentReadLock cLock(this);
		for (CLayers::const_iterator iL = m_cLayers.begin(); iL != m_cLayers.end(); ++iL)
		{
			if (iL->nUID == a_nLayerID)
			{
				if (iL->pDocImg == NULL)
					return E_FAIL;

				for (CLayerEffects::const_iterator iE = iL->pEffects->begin(); iE != iL->pEffects->end(); ++iE)
				{
					if (!iE->bEnabled || IsEqualGUID(iE->tOpID, GUID_NULL) || IsEqualGUID(iE->tOpID, __uuidof(DocumentOperationNULL)) || iE->pOp == NULL)
						continue;
					CComQIPtr<IRasterImageFilter> pRIF(iE->pOp);
					if (pRIF)
						pRIF->AdjustTransform(iE->pOpCfg, &m_tSize, a_pTransform);
				}
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

void BoxBlur(ULONG const a_nSizeX, ULONG const a_nSizeY, ULONG a_nRadiusX, ULONG a_nRadiusY, TRasterImagePixel const* a_pData, TRasterImagePixel* a_pDst)
{
	// assumes premultiplied values
	if (a_nSizeX < a_nRadiusX+a_nRadiusX+1)
		a_nRadiusX = (a_nSizeX-1)>>1;
	if (a_nSizeY < a_nRadiusY+a_nRadiusY+1)
		a_nRadiusY = (a_nSizeY-1)>>1;
	ULONG const nExSizeX = a_nSizeX+a_nRadiusX+a_nRadiusX;
	ULONG const nExSizeY = a_nSizeY+a_nRadiusY+a_nRadiusY;
	CAutoVectorPtr<TRasterImagePixel> cLine(new TRasterImagePixel[nExSizeX > nExSizeY ? nExSizeX : nExSizeY]);
	if (a_nRadiusX)
	{
		ULONG const nMul = 0x1000000/(a_nRadiusX+a_nRadiusX+1);
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			CopyMemory(cLine.m_p+a_nRadiusX, a_pData, a_nSizeX*sizeof*a_pData);
			std::fill_n(cLine.m_p, a_nRadiusX, *a_pData);
			std::fill_n(cLine.m_p+a_nSizeX+a_nRadiusX, a_nRadiusX, a_pData[a_nSizeX-1]);

			ULONG nR = 0;
			ULONG nG = 0;
			ULONG nB = 0;
			ULONG nA = 0;
			TRasterImagePixel* pSub = cLine;
			for (ULONG x = 0; x < a_nRadiusX+a_nRadiusX; ++x, ++pSub)
			{
				nR += pSub->bR;
				nG += pSub->bG;
				nB += pSub->bB;
				nA += pSub->bA;
			}
			TRasterImagePixel* pD = cLine;
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				nR += pSub->bR;
				nG += pSub->bG;
				nB += pSub->bB;
				nA += pSub->bA;
				a_pDst->bR = (nR*nMul+0x800000)>>24;
				a_pDst->bG = (nG*nMul+0x800000)>>24;
				a_pDst->bB = (nB*nMul+0x800000)>>24;
				a_pDst->bA = (nA*nMul+0x800000)>>24;
				nR -= pD->bR;
				nG -= pD->bG;
				nB -= pD->bB;
				nA -= pD->bA;
				++a_pDst;
				++pD;
				++pSub;
			}
			a_pData += a_nSizeX;
		}
		a_pDst -= a_nSizeX*a_nSizeY;
		a_pData = a_pDst;
	}
	if (a_nRadiusY)
	{
		ULONG const nMul = 0x1000000/(a_nRadiusY+a_nRadiusY+1);
		for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pData)
		{
			TRasterImagePixel* pD = cLine.m_p+a_nRadiusY;
			TRasterImagePixel const* pS = a_pData;
			for (ULONG y = 0; y < a_nSizeY; ++y)
			{
				*pD = *pS;
				++pD;
				pS += a_nSizeX;
			}
			std::fill_n(cLine.m_p, a_nRadiusY, *a_pData);
			std::fill_n(cLine.m_p+a_nSizeY+a_nRadiusY, a_nRadiusY, a_pData[a_nSizeX*(a_nSizeY-1)]);

			ULONG nR = 0;
			ULONG nG = 0;
			ULONG nB = 0;
			ULONG nA = 0;
			TRasterImagePixel* pSub = cLine;
			for (ULONG y = 0; y < a_nRadiusY+a_nRadiusY; ++y, ++pSub)
			{
				nR += pSub->bR;
				nG += pSub->bG;
				nB += pSub->bB;
				nA += pSub->bA;
			}
			pD = a_pDst+x;
			pS = cLine.m_p;
			for (ULONG y = 0; y < a_nSizeY; ++y)
			{
				nR += pSub->bR;
				nG += pSub->bG;
				nB += pSub->bB;
				nA += pSub->bA;
				pD->bR = (nR*nMul+0x800000)>>24;
				pD->bG = (nG*nMul+0x800000)>>24;
				pD->bB = (nB*nMul+0x800000)>>24;
				pD->bA = (nA*nMul+0x800000)>>24;
				nR -= pS->bR;
				nG -= pS->bG;
				nB -= pS->bB;
				nA -= pS->bA;
				pD += a_nSizeX;
				++pSub;
				++pS;
			}
		}
	}
}


//void CDocumentLayeredImage::MergeRectangle(ELayerBlend a_eBlend, TPixelChannel*& pBuffer1, TImagePoint& tBufferOffset, TImageSize& tBufferSize, TPixelChannel& tBufferDefault, TImagePoint const& a_tROITL, TImagePoint const& a_tROIBR, SCacheEntry* pCachedLayer, ULONG a_nBufferStride)
//{
//	ULONG const nBufDiff = a_nBufferStride == 0 ? 0 : a_nBufferStride-tBufferSize.nX;
//
//	try
//	{
//		TImagePoint tBufferEnd = {tBufferOffset.nX+tBufferSize.nX, tBufferOffset.nY+tBufferSize.nY};
//		TImagePoint tP0 = pCachedLayer->tOffset;
//		TImagePoint tP1 = {pCachedLayer->tOffset.nX+pCachedLayer->tSize.nX, pCachedLayer->tOffset.nY+pCachedLayer->tSize.nY};
//		//BYTE* pDst = reinterpret_cast<BYTE*>(pBuffer1.m_p)+((nOffsetX+m_tSize.nX*(nOffsetY+m_tSize.nY))<<2);
//		//BYTE const* pSrc = reinterpret_cast<BYTE const*>(pCachedLayer->pPixels);
//		//ULONG nDeltaY = (m_tSize.nX-pCachedLayer->tSize.nX)<<2;
//
//		if (tP0.nX < a_tROITL.nX) tP0.nX = a_tROITL.nX;
//		if (tP0.nY < a_tROITL.nY) tP0.nY = a_tROITL.nY;
//		if (tP1.nX > a_tROIBR.nX) tP1.nX = a_tROIBR.nX;
//		if (tP1.nY > a_tROIBR.nY) tP1.nY = a_tROIBR.nY;
//		TPixelChannel const* pLaySrc = pCachedLayer->pPixels+(tP0.nY-pCachedLayer->tOffset.nY)*pCachedLayer->tSize.nX+tP0.nX-pCachedLayer->tOffset.nX;
//		bool const bLayerEmpty = tP0.nX >= tP1.nX || tP0.nY >= tP1.nY;
//		if (bLayerEmpty && pCachedLayer->tDefault.bA == 0)
//			return; // layer does not influence the outcome
//		// update default color
//		TPixelChannel const tOldDefault = tBufferDefault;
//		MergeLayerLine(a_eBlend, 1, &tBufferDefault, &tBufferDefault, &pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		if (bLayerEmpty && tBufferSize.nX*tBufferSize.nY == 0)
//			return;
//
//		CAutoVectorPtr<TPixelChannel> pBuffer2;
//		TPixelChannel* pDst = pBuffer1;
//		TPixelChannel const* pSrc = pBuffer1;
//		TImagePoint tDstOffset = tBufferOffset;
//		TImageSize tDstSize = tBufferSize;
//		if (!bLayerEmpty)
//		{
//			if (tBufferSize.nX*tBufferSize.nY == 0)
//			{
//				ATLASSERT(a_nBufferStride == 0);
//				tDstOffset = tP0;
//				tDstSize.nX = tP1.nX-tP0.nX;
//				tDstSize.nY = tP1.nY-tP0.nY;
//				pBuffer2.Attach(new TPixelChannel[tDstSize.nX*tDstSize.nY]);
//				pDst = pBuffer2;
//			}
//			else if (tP0.nX < tBufferOffset.nX || tP0.nY < tBufferOffset.nY ||
//				tP1.nX > tBufferOffset.nX+LONG(tBufferSize.nX) ||
//				tP1.nY > tBufferOffset.nY+LONG(tBufferSize.nY))
//			{
//				tDstOffset.nX = tP0.nX < tBufferOffset.nX ? tP0.nX : tBufferOffset.nX;
//				tDstOffset.nY = tP0.nY < tBufferOffset.nY ? tP0.nY : tBufferOffset.nY;
//				tDstSize.nX = tP1.nX > tBufferOffset.nX+LONG(tBufferSize.nX) ? tP1.nX-tDstOffset.nX : tBufferOffset.nX+LONG(tBufferSize.nX)-tDstOffset.nX;
//				tDstSize.nY = tP1.nY > tBufferOffset.nY+LONG(tBufferSize.nY) ? tP1.nY-tDstOffset.nY : tBufferOffset.nY+LONG(tBufferSize.nY)-tDstOffset.nY;
//				pBuffer2.Attach(new TPixelChannel[tDstSize.nX*tDstSize.nY]);
//				pDst = pBuffer2;
//			}
//		}
//		TImagePoint tDstEnd = {tDstOffset.nX+tDstSize.nX, tDstOffset.nY+tDstSize.nY};
//
//		if (tDstSize.nX*tDstSize.nY >= 256*256 && tDstSize.nY >= 32 && m_pThPool)
//		{
//			CComObjectStackEx<CBlendTask> cBlendTask;
//			cBlendTask.Init(tP0, tP1, tDstOffset, tDstSize, tBufferOffset, tBufferSize, tBufferEnd, a_eBlend, pDst, pSrc, nBufDiff, pLaySrc, pCachedLayer->tSize.nX, pCachedLayer->tDefault, tOldDefault, tBufferDefault, &m_cGamma);
//			m_pThPool->Execute(0, &cBlendTask);
//		}
//		else
//		{
//			MergeRectangleInner(tP0, tP1, tDstOffset, tDstSize, tBufferOffset, tBufferSize, tBufferEnd, a_eBlend, pDst, pSrc, nBufDiff, pLaySrc, pCachedLayer->tSize.nX, pCachedLayer->tDefault, tOldDefault, tBufferDefault, m_cGamma);
//		}
//		//for (LONG y = tDstOffset.nY; y < LONG(tDstOffset.nY+tDstSize.nY); ++y)
//		//{
//		//	if (y >= tBufferOffset.nY && y < tBufferOffset.nY+LONG(tBufferSize.nY))
//		//	{
//		//		if (y >= tP0.nY && y < tP1.nY)
//		//		{
//		//			LONG x = tDstOffset.nX;
//		//			// old and new content
//		//			if (tBufferOffset.nX < tP0.nX)
//		//			{
//		//				ULONG n = tBufferSize.nX < ULONG(tP0.nX-tBufferOffset.nX) ? tBufferSize.nX : tP0.nX-tBufferOffset.nX;
//		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//				x += n;
//		//				pDst += n;
//		//				pSrc += n;
//		//			}
//		//			else if (tBufferOffset.nX > tP0.nX)
//		//			{
//		//				ULONG n = tP1.nX-tP0.nX < tBufferOffset.nX-tP0.nX ? tP1.nX-tP0.nX : tBufferOffset.nX-tP0.nX;
//		//				MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//				x += n;
//		//				pDst += n;
//		//				pLaySrc += n;
//		//			}
//		//			if (tBufferOffset.nX >= tP1.nX || tBufferEnd.nX <= tP0.nX)
//		//			{
//		//				ULONG n = tBufferOffset.nX >= tP1.nX ? tBufferOffset.nX-tP1.nX : tP0.nX-tBufferEnd.nX;
//		//				std::fill_n(pDst, n, tBufferDefault);
//		//				x += n;
//		//				pDst += n;
//		//			}
//		//			else
//		//			{
//		//				ULONG n = (tBufferEnd.nX < tP1.nX ? tBufferEnd.nX : tP1.nX)-(tBufferOffset.nX > tP0.nX ? tBufferOffset.nX : tP0.nX);
//		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//				x += n;
//		//				pDst += n;
//		//				pSrc += n;
//		//				pLaySrc += n;
//		//			}
//		//			if (tBufferEnd.nX > tP1.nX)
//		//			{
//		//				ULONG n = tBufferEnd.nX-x;
//		//				MergeLayerLine(a_eBlend, n, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//				x += n;
//		//				pDst += n;
//		//				pSrc += n;
//		//			}
//		//			else if (tBufferEnd.nX < tP1.nX)
//		//			{
//		//				ULONG n = tP1.nX-x;
//		//				MergeLayerLine(a_eBlend, n, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//				x += n;
//		//				pDst += n;
//		//				pLaySrc += n;
//		//			}
//		//			pLaySrc += pCachedLayer->tSize.nX-(tP1.nX-tP0.nX);
//		//		}
//		//		else
//		//		{
//		//			// just the old content
//		//			if (tDstOffset.nX < tBufferOffset.nX)
//		//			{
//		//				std::fill_n(pDst, tBufferOffset.nX-tDstOffset.nX, tBufferDefault);
//		//				pDst += tBufferOffset.nX-tDstOffset.nX;
//		//			}
//		//			MergeLayerLine(a_eBlend, tBufferSize.nX, pDst, pSrc, pCachedLayer->tDefault, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//			pSrc += tBufferSize.nX;
//		//			pDst += tBufferSize.nX;
//		//			if (tDstOffset.nX+LONG(tDstSize.nX) > tBufferOffset.nX+LONG(tBufferSize.nX))
//		//			{
//		//				std::fill_n(pDst, (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX), tBufferDefault);
//		//				pDst += (tDstOffset.nX+tDstSize.nX)-(tBufferOffset.nX+tBufferSize.nX);
//		//			}
//		//		}
//		//	}
//		//	else if (y >= tP0.nY && y < tP1.nY)
//		//	{
//		//		// just the new content
//		//		if (tDstOffset.nX < tP0.nX)
//		//		{
//		//			std::fill_n(pDst, tP0.nX-tDstOffset.nX, tBufferDefault);
//		//			pDst += tP0.nX-tDstOffset.nX;
//		//		}
//		//		MergeLayerLine(a_eBlend, tP1.nX-tP0.nX, pDst, tOldDefault, pLaySrc, m_cGamma);//m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//		pLaySrc += pCachedLayer->tSize.nX;
//		//		pDst += tP1.nX-tP0.nX;
//		//		if (tDstOffset.nX+LONG(tDstSize.nX) > tP1.nX)
//		//		{
//		//			std::fill_n(pDst, tDstOffset.nX+tDstSize.nX-tP1.nX, tBufferDefault);
//		//			pDst += tDstOffset.nX+tDstSize.nX-tP1.nX;
//		//		}
//		//	}
//		//	else
//		//	{
//		//		// empty space
//		//		std::fill_n(pDst, tDstSize.nX, tBufferDefault);
//		//		pDst += tDstSize.nX;
//		//	}
//		//	pDst += nBufDiff;
//		//	pSrc += nBufDiff;
//		//}
//
//		//CAutoVectorPtr<TRasterImagePixel> pTmp;
//		//if ((i->tData.eBlend&EBEOperationMask) == EBEGlass)
//		//{
//		//	pTmp.Allocate(pCachedLayer->tSize.nX*pCachedLayer->tSize.nY);
//		//	BoxBlur(pCachedLayer->tSize.nX, pCachedLayer->tSize.nY, 17, 17, reinterpret_cast<TRasterImagePixel*>(pDst), pTmp);
//		//	CComPtr<SCacheEntry> pRawLayer;
//		//	pRawLayer.Attach(QueryCache(ECPRaw, i->nUID));
//		//	ULONG const n = pCachedLayer->tSize.nX*pCachedLayer->tSize.nY;
//		//	TRasterImagePixel *pD = reinterpret_cast<TRasterImagePixel*>(pDst);
//		//	TPixelChannel const* pR = pRawLayer->pPixels;
//		//	TRasterImagePixel const* pB = pTmp;
//		//	for (TRasterImagePixel* pEnd = pD+n; pD < pEnd; ++pD, ++pR, ++pB)
//		//	{
//		//		if (pR->bA != 0 && pR->bA != 255)
//		//			*pD = *pB;
//		//	}
//		//}
//
//		//for (ULONG nY = 0; nY < pCachedLayer->tSize.nY; pDst+=nDeltaY, ++nY)
//		//{
//		//	MergeLayerLine(i->tData.eBlend, pCachedLayer->tSize.nX, reinterpret_cast<TRasterImagePixel*>(pDst), reinterpret_cast<TRasterImagePixel const*>(pDst), reinterpret_cast<TRasterImagePixel const*>(pSrc), m_fCachedGamma != 1.0f, m_aGammaF, m_aGammaB);
//		//	pDst += pCachedLayer->tSize.nX<<2;
//		//	pSrc += pCachedLayer->tSize.nX<<2;
//		//}
//		if (pBuffer2.m_p)
//		{
//			std::swap(pBuffer1, pBuffer2.m_p);
//			tBufferSize = tDstSize;
//			tBufferOffset = tDstOffset;
//			tBufferEnd = tDstEnd;
//		}
//	}
//	catch (...)
//	{
//	}
//}

#include "RasterImageOperationStep.h"

//CDocumentLayeredImage::SCacheEntry* CDocumentLayeredImage::QueryCache(ECachePhase a_ePhase, ULONG a_nCount, ULONG* a_aLayerIDs)
//{
//	CComPtr<SCacheEntry> pItem;
//
//	{
//		ObjectLock lock(m_cCacheCS);
//
//		CLayerCache::iterator i = m_cCache.find(SCacheKey(a_nLayerID, a_ePhase));
//		if (i != m_cCache.end())
//		{
//			i->first.nAge = 0;
//			pItem = i->second;
//		}
//		else
//		{
//			// create empty cache entry
//			try
//			{
//				SCacheEntry* pNew = new SCacheEntry;
//				pItem = pNew;
//				m_cCache[SCacheKey(a_nLayerID, a_ePhase)] = pNew;
//			}
//			catch (...)
//			{
//				m_cCacheCS.Unlock();
//				return NULL; // very bad
//			}
//		}
//	}
//
//	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
//		return pItem.Detach();
//
//	ObjectLock lock(pItem->tCS);
//
//	if (pItem->pImage && pItem->tInvalid.tTL.nX >= pItem->tInvalid.tBR.nX && pItem->tInvalid.tTL.nY >= pItem->tInvalid.tBR.nY)
//		return pItem.Detach();
//
//	pItem->pImage = NULL; // TODO: eventually, only update invalid subrectangle
//
//	try
//	{
//		CLayers::const_iterator iL;
//		for (iL = m_cLayers.begin(); iL != m_cLayers.end(); ++iL)
//		{
//			if (iL->nUID == a_nLayerID)
//				break;
//		}
//		if (iL == m_cLayers.end())
//			return NULL;
//
//		switch (a_ePhase)
//		{
//		case ECPRaw:
//			{
//				(pItem->pImage = iL->pDocImg)->AddRef();
//				pItem->tInvalid = INVALIDRECT;
//				pItem->nMergedIDs = 0;
//				break;
//			}
//		case ECPProcessed:
//			{
//				CComPtr<IDocumentImage> pProcessed;
//				if (iL->pEffects && !iL->pEffects->empty())
//					AppplyLayerStyle(iL->pDocImg, *iL->pEffects, &pProcessed);
//				if (pProcessed)
//					pItem->pImage = pProcessed.Detach();
//				else
//					(pItem->pImage = iL->pDocImg)->AddRef();
//
//				pItem->tInvalid = INVALIDRECT;
//				pItem->nMergedIDs = 0;
//				break;
//			}
//		case ECPMerged:
//			{
//				if (m_cLayers.size() == 1 && iL->bVisible && iL->eBlendingMode == EBEAlphaBlend)
//				{
//					pItem->tCS.Unlock();
//					return QueryCache(ECPProcessed, a_nLayerID); // optimization for single-layer with no special effects
//				}
//
//				CAutoVectorPtr<TPixelChannel> pBuffer1;//(new TPixelChannel[m_tSize.nX*m_tSize.nY]);
//				TImageSize tBufferSize = {0, 0};
//				TImagePoint tBufferOffset = {0, 0};
//				TPixelChannel tBufferDefault;
//				tBufferDefault.n = 0;
//				for (CLayers::const_iterator i = m_cLayers.begin(); i <= iL; i++)
//				{
//					if (i->pDocImg == NULL || i->bVisible == 0)
//						continue;
//
//					CComPtr<SCacheEntry> pCachedLayer;
//					pCachedLayer.Attach(QueryCache(ECPProcessed, i->nUID));
//					MergeRectangle(i->eBlendingMode, pBuffer1.m_p, tBufferOffset, tBufferSize, tBufferDefault, CImagePoint(0, 0), CImagePoint(m_tSize.nX, m_tSize.nY), pCachedLayer);
//				}
//				CAutoVectorPtr<ULONG> pMergedIDs;
//				if (pItem->nMergedIDs = iL-m_cLayers.begin())
//				{
//					pMergedIDs.Allocate(pItem->nMergedIDs);
//					for (ULONG j = 0; j < pItem->nMergedIDs; ++j)
//						pMergedIDs[j] = m_cLayers[j].nUID;
//				}
//				pItem->pMergedIDs = pMergedIDs.Detach();
//				pItem->tSize = tBufferSize;
//				pItem->tOffset = tBufferOffset;
//				pItem->pPixels = pBuffer1.Detach();
//				pItem->tDefault = tBufferDefault;
//				break;
//			}
//		default:
//			if ((a_ePhase&0xffff) == ECPThumbnail)
//			{
//				ULONG nSizeX = 1+((a_ePhase>>16)&0xff);
//				ULONG nSizeY = 1+((a_ePhase>>24)&0xff);
//				CComPtr<SCacheEntry> pRaw;
//				pRaw.Attach(QueryCache(ECPRaw, a_nLayerID));
//				if (pRaw == NULL) throw E_UNEXPECTED;
//
//				ULONG nIcoSizeX;
//				ULONG nIcoSizeY;
//				if (m_tSize.nX*nSizeY > m_tSize.nY*nSizeX)
//				{
//					nIcoSizeX = nSizeX;
//					nIcoSizeY = (m_tSize.nY*nSizeX+(m_tSize.nX>>1))/m_tSize.nX;
//				}
//				else
//				{
//					nIcoSizeX = (m_tSize.nX*nSizeY+(m_tSize.nY>>1))/m_tSize.nY;
//					nIcoSizeY = nSizeY;
//				}
//				if (nIcoSizeX == 0) nIcoSizeX = 1;
//				if (nIcoSizeY == 0) nIcoSizeY = 1;
//				bool const bResize = nIcoSizeX < m_tSize.nX || nIcoSizeY < m_tSize.nY;
//				CAutoVectorPtr<TPixelChannel> pBuffer2(new TPixelChannel[bResize ? nIcoSizeX*nIcoSizeY : m_tSize.nX*m_tSize.nY]);
//
//				if (pRaw->pPixels == NULL || pRaw->tOffset.nX >= LONG(m_tSize.nX) || pRaw->tOffset.nY >= LONG(m_tSize.nY) ||
//					pRaw->tOffset.nX+LONG(pRaw->tSize.nX) <= 0 || pRaw->tOffset.nY+LONG(pRaw->tSize.nY) <= 0)
//				{
//					std::fill_n(pBuffer2.m_p, bResize ? nIcoSizeX*nIcoSizeY : m_tSize.nX*m_tSize.nY, pRaw->tDefault);
//					pItem->tSize.nX = bResize ? nIcoSizeX : m_tSize.nX;
//					pItem->tSize.nY = bResize ? nIcoSizeY : m_tSize.nY;
//				}
//				else
//				{
//					TPixelChannel* pD = pBuffer2.m_p;
//					if (nIcoSizeX < m_tSize.nX || nIcoSizeY < m_tSize.nY)
//					{
//						for (ULONG y = 0; y < nIcoSizeY; ++y)
//						{
//							LONG yS = (y*m_tSize.nY+(nIcoSizeY>>1))/nIcoSizeY;
//							if (yS < pRaw->tOffset.nY || yS >= pRaw->tOffset.nY+LONG(pRaw->tSize.nY))
//							{
//								std::fill_n(pD, nIcoSizeX, pRaw->tDefault);
//								pD += nIcoSizeX;
//								continue;
//							}
//							TPixelChannel const* pS = pRaw->pPixels+(yS-pRaw->tOffset.nY)*pRaw->tSize.nX;
//							for (ULONG x = 0; x < nIcoSizeX; ++x)
//							{
//								LONG xS = (x*m_tSize.nX+(nIcoSizeX>>1))/nIcoSizeX;
//								if (xS < pRaw->tOffset.nX || xS >= pRaw->tOffset.nX+LONG(pRaw->tSize.nX))
//									*pD = pRaw->tDefault;
//								else
//									*pD = pS[xS-pRaw->tOffset.nX];
//								++pD;
//							}
//						}
//						pItem->tSize.nX = nIcoSizeX;
//						pItem->tSize.nY = nIcoSizeY;
//					}
//					else
//					{
//						for (LONG y = 0; y < LONG(m_tSize.nY); ++y)
//						{
//							if (y < pRaw->tOffset.nY || y >= pRaw->tOffset.nY+LONG(pRaw->tSize.nY))
//							{
//								std::fill_n(pD, m_tSize.nX, pRaw->tDefault);
//								pD += m_tSize.nX;
//								continue;
//							}
//							TPixelChannel const* pS = pRaw->pPixels+(y-pRaw->tOffset.nY)*pRaw->tSize.nX;
//							for (LONG x = 0; x < LONG(m_tSize.nX); ++x)
//							{
//								if (x < pRaw->tOffset.nX || x >= pRaw->tOffset.nX+LONG(pRaw->tSize.nX))
//									*pD = pRaw->tDefault;
//								else
//									*pD = pS[x-pRaw->tOffset.nX];
//								++pD;
//							}
//						}
//						pItem->tSize = m_tSize;
//					}
//				}
//				pItem->nMergedIDs = 0;
//				pItem->pPixels = pBuffer2.Detach();
//				break;
//			}
//		//default:
//		//	pItem->tCS.Unlock();
//		//	return NULL;
//		}
//	}
//	catch (...)
//	{
//		return NULL;
//	}
//
//	if (pItem->bUseRaw)
//		return QueryCache(ECPRaw, a_nLayerID);
//
//	return pItem.Detach();
//}

void CDocumentLayeredImage::QueryCacheContent(TImagePoint* a_pOrigin, TImagePoint* a_pEnd)
{
	CDocumentReadLock cLock(this);
	for (CLayers::const_iterator i = m_cLayers.begin(); i != m_cLayers.end(); ++i)
	{
		SCacheEntryPtr pEntry = QueryLayerCache(i->nUID);
		if (pEntry != NULL)
		{
			if (a_pOrigin->nX > pEntry->tBounds.tTL.nX) a_pOrigin->nX = pEntry->tBounds.tTL.nX;
			if (a_pOrigin->nY > pEntry->tBounds.tTL.nY) a_pOrigin->nY = pEntry->tBounds.tTL.nY;
			if (a_pEnd->nX < pEntry->tBounds.tBR.nX) a_pEnd->nX = pEntry->tBounds.tBR.nX;
			if (a_pEnd->nY < pEntry->tBounds.tBR.nY) a_pEnd->nY = pEntry->tBounds.tBR.nY;
		}
	}
}

HICON CDocumentLayeredImage::LayerIcon(ULONG a_nSize, IRTarget const* a_pTarget)
{
	static IRPolyPoint const aVertices1[] = {{16, 16}, {96, 16}, {96, 64}, {64, 64}, {64, 192}, {96, 192}, {96, 240}, {16, 240}};
	static IRPolyPoint const aVertices2[] = {{160, 64}, {160, 16}, {240, 16}, {240, 240}, {160, 240}, {160, 192}, {192, 192}, {192, 64}};
	static IRPolygon const aPolys[] = {{itemsof(aVertices1), aVertices1}, {itemsof(aVertices2), aVertices2}};

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));

	static IRGridItem const gridX[] = { {EGIFInteger, 16.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 96.0f}, {EGIFInteger, 160.0f}, {EGIFInteger, 192.0f}, {EGIFInteger, 240.0f}};
	static IRGridItem const gridY[] = { {EGIFInteger, 16.0f}, {EGIFInteger, 64.0f}, {EGIFInteger, 192.0f}, {EGIFInteger, 240.0f}};
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(aPolys), aPolys, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	return cRenderer.get();
}

