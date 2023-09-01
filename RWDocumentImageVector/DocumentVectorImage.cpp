// DocumentVectorImage.cpp : Implementation of CDocumentVectorImage

#include "stdafx.h"
#include "DocumentVectorImage.h"
#include <math.h>
#include <RWImagingDocumentUtils.h>
#include "DocumentVectorImageUndo.h"
#include <RWBaseEnumUtils.h>
#include <GammaCorrection.h>

//#include "../RWDocumentImageRaster/LayerRasterImage.h"


MIDL_INTERFACE("C76B9E3A-3F4E-46D7-8192-61EDDB181B7A")
IStructuredItemVectorImageElement : public IUnknown
{
public:
	STDMETHOD_(void, ID)(ULONG* a_pItemID) = 0;
};

class CStructuredItemVectorImageElement : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IStructuredItemVectorImageElement,
	public IUIItem,
	public IItemString,
	public IItemBool
{
public:
	CStructuredItemVectorImageElement() : m_pDoc(NULL), m_nItemID(0)
	{
	}
	~CStructuredItemVectorImageElement()
	{
		if (m_pDoc) m_pDoc->Release();
	}
	void Init(CDocumentVectorImage* a_pDoc, ULONG a_nItemID)
	{
		m_nItemID = a_nItemID;
		(m_pDoc = a_pDoc)->AddRef();
	}

BEGIN_COM_MAP(CStructuredItemVectorImageElement)
	COM_INTERFACE_ENTRY(IStructuredItemVectorImageElement)
	COM_INTERFACE_ENTRY(IUIItem)
	COM_INTERFACE_ENTRY2(IComparable, IUIItem)
	COM_INTERFACE_ENTRY(IItemString)
	COM_INTERFACE_ENTRY(IItemBool)
END_COM_MAP()


	// IItemString methods
public:
	STDMETHOD(ValueGet)(BSTR* a_pbstrValue)
	{
		try
		{
			HRESULT hRes = m_pDoc->ObjectNameGet(m_nItemID, a_pbstrValue);
			if (FAILED(hRes) || *a_pbstrValue) return hRes;
			CMultiLanguageString::GetLocalized(L"[0409]unnamed[0405]nepojmenovaný", GetThreadLocale(), a_pbstrValue);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrValue == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ValueSet)(BSTR a_bstrValue)
	{
		try
		{
			m_pDoc->ObjectNameSet(m_nItemID, a_bstrValue);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IItemBool methods
public:
	STDMETHOD(ValueGet)(boolean *a_pbValue)
	{
		try
		{
			*a_pbValue = m_pDoc->ObjectIsEnabled(m_nItemID) == S_OK;
			return S_OK;
		}
		catch (...)
		{
			return a_pbValue == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ValueSet)(boolean a_bValue)
	{
		try
		{
			m_pDoc->ObjectEnable(m_nItemID, a_bValue);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IUIItem methods
public:
	STDMETHOD(NameGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
	{
		try
		{
			*a_pbstrName = NULL;
			CComBSTR bstrID;
			m_pDoc->ObjectGet(m_nItemID, &bstrID, NULL);
			CComPtr<ILocalizedString> pStr;
			m_pDoc->M_ToolMgr()->ToolNameGet(bstrID, &pStr);
			if (pStr)
				return pStr->GetLocalized(a_tPreferedLCID, a_pbstrName);
			*a_pbstrName = bstrID.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(DescriptionGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
	{
		try
		{
			*a_pbstrName = NULL;
			CComBSTR bstrID;
			m_pDoc->ObjectGet(m_nItemID, &bstrID, NULL);
			CComPtr<ILocalizedString> pStr;
			m_pDoc->M_ToolMgr()->ToolDescGet(bstrID, &pStr);
			if (pStr)
				return pStr->GetLocalized(a_tPreferedLCID, a_pbstrName);
			return E_NOTIMPL;
		}
		catch (...)
		{
			return a_pbstrName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ColorsGet)(DWORD* UNREF(a_prgbPrimary), DWORD* UNREF(a_prgbSecondary))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IconIDGet)(GUID* a_pIconID)
	{
		try
		{
			*a_pIconID = GUID_NULL;
			CComBSTR bstrID;
			m_pDoc->ObjectGet(m_nItemID, &bstrID, NULL);
			return m_pDoc->M_ToolMgr()->ToolIconIDGet(bstrID, a_pIconID);
		}
		catch (...)
		{
			return a_pIconID ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			CComBSTR bstrID;
			m_pDoc->ObjectGet(m_nItemID, &bstrID, NULL);
			return m_pDoc->M_ToolMgr()->ToolIconGet(bstrID, a_nSize, a_phIcon);
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(UseThumbnail)() { return S_OK; }
	STDMETHOD(ExpandedByDefault)() { return S_FALSE; }
	STDMETHOD(ContextOperationsEnum)(IEnumUnknowns** a_ppOperations)
	{
		try
		{
			*a_ppOperations = NULL;
			return E_NOTIMPL;
		}
		catch (...)
		{
			return a_ppOperations == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IComparable methods
public:
	STDMETHOD(Compare)(IComparable* a_pOther)
	{
		try
		{
			CComQIPtr<IStructuredItemVectorImageElement> pVectorItem(a_pOther);
			if (pVectorItem == NULL)
				return S_FALSE;
			ULONG nID = 0xffffffff;
			pVectorItem->ID(&nID);
			return  m_nItemID < nID ? S_LESS : (m_nItemID > nID ? S_MORE : S_OK);
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
			static const GUID tMyID = {0x5034a452, 0xba69, 0x48ed, {0x90, 0xf7, 0x9b, 0xbb, 0xb1, 0x80, 0xb1, 0xc4}};
			*a_pCLSID = tMyID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}

	// IStructuredItemVectorImageElement methods
public:
	STDMETHOD_(void, ID)(ULONG* a_pItemID)
	{
		*a_pItemID = m_nItemID;
	}

private:
	CDocumentVectorImage* m_pDoc;
	ULONG m_nItemID;
};

template<class T>
inline void InsertItem(IEnumUnknownsInit* a_pTo, CDocumentVectorImage* a_pThis, ULONG a_nItemID = 0)
{
	CComObject<T>* p = NULL;
	CComObject<T>::CreateInstance(&p);
	CComPtr<IStructuredItemVectorImageElement> pTmp = p;
	p->Init(a_pThis, a_nItemID);
	if (FAILED(a_pTo->Insert(static_cast<IUIItem*>(p))))
		throw E_FAIL;
}

static OLECHAR const CFGID_LAYEREFFECT[] = L"Effect";

bool IsNoOperation(IConfig* pEffect)
{
	if (pEffect == NULL)
		return true;
	CConfigValue cVal;
	pEffect->ItemValueGet(CComBSTR(CFGID_LAYEREFFECT), &cVal);
	if (cVal.TypeGet() != ECVTGUID)
		return true;
	if (IsEqualGUID(cVal.operator const GUID &(), __uuidof(DocumentOperationNULL)))
		return true;
	if (!IsEqualGUID(cVal.operator const GUID &(), __uuidof(DocumentOperationSequence)))
		return false;
	CConfigValue steps;
	pEffect->ItemValueGet(CComBSTR(L"Effect\\SeqSteps"), &steps);
	for (LONG i = 0; i < steps.operator LONG(); ++i)
	{
		wchar_t sz[128];
		swprintf(sz, L"Effect\\SeqSteps\\%08x\\SeqSkipStep", i);
		CConfigValue skip;
		pEffect->ItemValueGet(CComBSTR(sz), &skip);
		if (!skip)
			return false;
	}
	return true;
}

HRESULT CDocumentVectorImage::GetDirtyRect(SElement const& sEl, RECT& rc)
{
	HRESULT hRes = sEl.pTool->IsDirty(&rc, NULL, NULL);
	//if (!IsNoOperation(sEl.pEffect))
	//{
	//	CComBSTR bstrCFGID_LAYEREFFECT(CFGID_LAYEREFFECT);
	//	CConfigValue cEffectID;
	//	sEl.pEffect->ItemValueGet(bstrCFGID_LAYEREFFECT, &cEffectID);
	//	CComPtr<IConfig> pEffect;
	//	sEl.pEffect->SubConfigGet(bstrCFGID_LAYEREFFECT, &pEffect);
	//	CComObjectStackEx<CLayerImageAdjustment> cLIA;
	//	cLIA.Init(EROSAdjustDirty, &rc);
	//	CComObjectStackEx<CLayerOperationContext> cLOC;
	//	if (FAILED(M_OpMgr()->Activate(M_OpMgr(), &cLIA, cEffectID, pEffect, &cLOC, NULL, 0)))
	//	{
	//		// operation does not support regional processing -> better mark the whole canvas as invalid
	//		rc.left = 0;
	//		rc.top = 0;
	//		rc.right = m_nSizeX;
	//		rc.bottom = m_nSizeY;
	//	}
	//}
	return hRes;
}


// CDocumentVectorImage

STDMETHODIMP CDocumentVectorImage::WriteFinished()
{
	std::vector<TStructuredChange> cSub;

	try
	{
		TVectorImageChanges tVIChanges;
		ZeroMemory(&tVIChanges, sizeof tVIChanges);
		CAutoVectorPtr<TVectorImageObjectChange> aObjects(m_cElements.empty() ? NULL : new TVectorImageObjectChange[m_cElements.size()]);
		tVIChanges.aObjects = aObjects;

		for (CElements::iterator i = m_cElements.begin(); i != m_cElements.end(); ++i)
		{
			if (i->second.bChangeName || i->second.bChangeToolID || i->second.bChangeParams ||
				i->second.bChangeOutline || i->second.bChangeModes || i->second.bChangeStyle || i->second.bChangeVisibility)
			{
				aObjects[tVIChanges.nObjects].nID = i->first;
				aObjects[tVIChanges.nObjects].nChangeFlags =
					(i->second.bChangeToolID*EVICObjectToolID) |
					(i->second.bChangeParams*EVICObjectParams) |
					(i->second.bChangeOutline*EVICObjectOutline) |
					(i->second.bChangeModes*EVICObjectModes) |
					(i->second.bChangeStyle*EVICObjectStyle) |
					(i->second.bChangeName*EVICObjectName) |
					(i->second.bChangeVisibility*ECIVObjectVisibility);
				++tVIChanges.nObjects;
				if (i->second.bChangeName || i->second.bChangeToolID || i->second.bChangeParams || i->second.bChangeStyle || i->second.bChangeOutline || i->second.bChangeVisibility)
				{
					TStructuredChange tChange;
					tChange.nChangeFlags = ESCGUIRepresentation|ESCContent;
					CComObject<CStructuredItemVectorImageElement>* p = NULL;
					CComObject<CStructuredItemVectorImageElement>::CreateInstance(&p);
					CComPtr<IUIItem> pTmp = p;
					p->Init(this, i->first);
					tChange.pItem = pTmp;
					cSub.push_back(tChange);
					pTmp.Detach();
				}
				i->second.bChangeName = i->second.bChangeToolID = i->second.bChangeParams =
				i->second.bChangeOutline = i->second.bChangeModes = i->second.bChangeStyle = i->second.bChangeVisibility = false;
			}
		}

		tVIChanges.nGlobalChangeFlags =
			((m_bChangeSize||m_bChangeBackground)*EVICImageProps) |
			(m_bChangeElements*EVICObjects);

		if (tVIChanges.nGlobalChangeFlags || tVIChanges.nObjects)
		{
			CSubjectImpl<IDocumentVectorImage, IVectorImageObserver, TVectorImageChanges>::Fire_Notify(tVIChanges);
		}
		if (m_bChangeElements)
		{
			TStructuredChange tChg;
			tChg.nChangeFlags = ESCChildren;
			tChg.pItem = NULL;
			cSub.push_back(tChg);
		}
		if (!cSub.empty())
		{
			TStructuredChanges tChanges;
			tChanges.aChanges = &(cSub[0]);
			tChanges.nChanges = cSub.size();
			CSubjectImpl<CStructuredRootImpl<CDocumentVectorImage, IStructuredRoot>, IStructuredObserver, TStructuredChanges>::Fire_Notify(tChanges);
		}

		if (m_bChangeSize || m_bChangeBackground || (m_rcDirty.left < m_rcDirty.right && m_rcDirty.top < m_rcDirty.bottom))
		{
			TImageChange tChg;
			tChg.nGlobalFlags = (m_bChangeSize*EICDimensions)|(m_bChangeBackground*EICContent);
			if ((m_rcDirty.left < m_rcDirty.right && m_rcDirty.top < m_rcDirty.bottom))
			{
				tChg.tOrigin.nX = m_rcDirty.left;
				tChg.tOrigin.nY = m_rcDirty.top;
				tChg.tSize.nX = m_rcDirty.right-m_rcDirty.left;
				tChg.tSize.nY = m_rcDirty.bottom-m_rcDirty.top;
			}
			else
			{
				tChg.tOrigin.nX = tChg.tOrigin.nY = 0;
				tChg.tSize.nX = tChg.tSize.nY = 0;
			}
			CSubjectImpl<IDocumentEditableImage, IImageObserver, TImageChange>::Fire_Notify(tChg);
			m_rcDirty.left = m_rcDirty.top = LONG_MAX;
			m_rcDirty.right = m_rcDirty.bottom = LONG_MIN;
		}

		if (tVIChanges.nGlobalChangeFlags || tVIChanges.nObjects || m_bChangeElements)
			M_Base()->SetDirty();
		if (m_bChangeSize)
			M_Base()->UpdateQuickInfo();

		m_bChangeSize = m_bChangeBackground = m_bChangeElements = false;
	}
	catch (...)
	{
	}
	for (std::vector<TStructuredChange>::iterator i = cSub.begin(); i != cSub.end(); ++i)
		if (i->pItem) i->pItem->Release();
	return S_OK;
}

STDMETHODIMP CDocumentVectorImage::DataCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		CComObject<CDocumentVectorImage>* p = NULL;
		CComObject<CDocumentVectorImage>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		CDocumentVectorImage* pDoc = p;

		pDoc->m_nNextID = m_nNextID;
		pDoc->m_tCanvas = m_tCanvas;
		pDoc->m_aBackground[0] = m_aBackground[0];
		pDoc->m_aBackground[1] = m_aBackground[1];
		pDoc->m_aBackground[2] = m_aBackground[2];
		pDoc->m_aBackground[3] = m_aBackground[3];
		for (CElements::const_iterator i = m_cElements.begin(); i != m_cElements.end(); ++i)
		{
			SElement sEl = i->second;
			sEl.pFill = NULL;
			sEl.pTool = NULL;

			pDoc->m_pToolMgr->EditToolCreate(sEl.bstrToolID, NULL, &sEl.pTool);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(sEl.pTool);
			if (pScripting == NULL)
				return E_FAIL;
			sEl.pTool->Init(pDoc->m_pToolWindow);
			if (sEl.bstrToolParams)
				pScripting->FromText(sEl.bstrToolParams);
			sEl.pTool->SetGlobals(EBMDrawOver, sEl.eRasterizationMode, sEl.eCoordinatesMode);
			sEl.pTool->SetOutline(sEl.bOutline, sEl.fOutlineWidth, sEl.fOutlinePos, sEl.eOutlineJoins, &sEl.tOutlineColor);
			//sEl.pTool->SetColors(&sEl.tColor1, &sEl.tColor2);

			if (sEl.bstrStyleID.Length())
			{
				m_pFillMgr->FillStyleCreate(sEl.bstrStyleID, NULL, &sEl.pFill);
				CComQIPtr<IRasterImageEditToolScripting> pScripting(sEl.pFill);
				if (pScripting)
				{
					if (sEl.bstrStyleParams)
						pScripting->FromText(sEl.bstrStyleParams);
				}
				//sEl.pFill->SetColors(&sEl.tColor1, &sEl.tColor2);
			}
			if (sEl.pFill)
				sEl.pTool->SetBrush(sEl.pFill);

			pDoc->m_cElements[i->first] = sEl;
		}
		pDoc->m_cElementOrder = m_cElementOrder;

		if (a_pPreviewEffect)
			pDoc->m_bstrPreview = m_bstrPreview.m_str ? m_bstrPreview.m_str : M_DataID();

		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>
#include <MultiLanguageString.h>

STDMETHODIMP CDocumentVectorImage::QuickInfo(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
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
		pPFStr->Init(pTempl, m_tCanvas.nX, m_tCanvas.nY);
		*a_ppInfo = pStr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::MaximumUndoSize(ULONGLONG* a_pnMaximumSize)
{
	*a_pnMaximumSize = 0x10000000;
	return S_OK;
}

STDMETHODIMP CDocumentVectorImage::ResourcesManage(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue)
{
	if (a_eActions & EDRMGetMemoryUsage && a_pValue)
	{
		*a_pValue = 0;
	}
	return S_OK;
}

STDMETHODIMP CDocumentVectorImage::Aspects(IEnumEncoderAspects* a_pEnumAspects)
{
	try
	{
		static float const aVals = 100.0f;
		CComBSTR bstr(L"[vecimg]");
		a_pEnumAspects->Consume(0, 1, &(bstr.m_str), &aVals);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentVectorImage::UpdateContentSize()
{
	if (!m_bContent)
	{
		m_pToolWindow->SetROI(CImagePoint(0, 0), m_tCanvas, NULL, CPixelChannel(0, 0, 0, 0), m_tCanvas);// TODO: delete when IsDirty is not rendering the item
		RECT rcC = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		for (CElementOrder::const_iterator i = m_cElementOrder.begin(); i != m_cElementOrder.end(); ++i)
		{
			SElement const& sEl = m_cElements[*i];
			if (sEl.pTool == NULL)
				continue;
			RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			GetDirtyRect(sEl, rc);

			if (rcC.left > rc.left) rcC.left = rc.left;
			if (rcC.top > rc.top) rcC.top = rc.top;
			if (rcC.right < rc.right) rcC.right = rc.right;
			if (rcC.bottom < rc.bottom) rcC.bottom = rc.bottom;
		}
		m_rcContent = rcC;
		m_bContent = true;
	}
}

void CDocumentVectorImage::UpdateContentCache()
{
	if (m_bBuffer)
		return;

	ObjectLock lock(this);
	if (m_bBuffer)
		return;

	UpdateContentSize();

	SIZE sz = {m_rcContent.right-m_rcContent.left, m_rcContent.bottom-m_rcContent.top};
	ULONG nPixels = sz.cx*sz.cy;
	if (m_pBuffer == NULL || m_nBuffer > 2*nPixels || m_nBuffer < nPixels)
	{
		m_pBuffer.Free();
		m_pBuffer.Allocate(m_nBuffer = nPixels);
	}
	Render(CImagePoint(m_rcContent.left, m_rcContent.top), CImageSize(sz.cx, sz.cy), m_pBuffer);
	m_bBuffer = true;
}

STDMETHODIMP CDocumentVectorImage::CanvasGet(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
{
	try
	{
		CDocumentReadLock cLock(this);

		if (a_pCanvasSize) *a_pCanvasSize = m_tCanvas;
		if (a_pResolution) *a_pResolution = m_tResolution;
		if (a_pContentOrigin || a_pContentSize)
		{
			{
				ObjectLock lock(this);
				UpdateContentSize();
			}
			if (m_rcContent.left < m_rcContent.right)
			{
				if (a_pContentOrigin)
				{
					a_pContentOrigin->nX = m_rcContent.left;
					a_pContentOrigin->nY = m_rcContent.top;
				}
				if (a_pContentSize)
				{
					a_pContentSize->nX = m_rcContent.right-m_rcContent.left;
					a_pContentSize->nY = m_rcContent.bottom-m_rcContent.top;
				}
			}
			else
			{
				if (a_pContentOrigin) a_pContentOrigin->nX = a_pContentOrigin->nY = 0;
				if (a_pContentSize) a_pContentSize->nX = a_pContentSize->nY = 0;
			}
		}
		if (a_pContentOpacity) *a_pContentOpacity = EIOSemiTransparent;

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ChannelsGet(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
{
	try
	{
		CDocumentReadLock cLock(this);

		if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
		if (a_pGamma) *a_pGamma = 2.2f;
		TPixelChannel tDef;
		tDef.bR = CGammaTables::ToSRGB(m_aBackground[0]);
		tDef.bG = CGammaTables::ToSRGB(m_aBackground[1]);
		tDef.bB = CGammaTables::ToSRGB(m_aBackground[2]);
		tDef.bA = 255.0f*m_aBackground[3] + 0.5f;
		if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, CChannelDefault(EICIRGBA, tDef));

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::TileGet(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* UNREF(a_pControl), EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		CDocumentReadLock cLock(this);
		UpdateContentCache();
		CPixelChannel tBackground(CGammaTables::ToSRGB(m_aBackground[0]), CGammaTables::ToSRGB(m_aBackground[1]), CGammaTables::ToSRGB(m_aBackground[2]), 255.0f*m_aBackground[3] + 0.5f);
		return RGBAGetTileImpl(m_tCanvas, CImagePoint(m_rcContent.left, m_rcContent.top), CImageSize(m_rcContent.right-m_rcContent.left, m_rcContent.bottom-m_rcContent.top), m_pBuffer, m_rcContent.right-m_rcContent.left, tBackground, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::Inspect(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		CDocumentReadLock cLock(this);
		UpdateContentCache();
		CPixelChannel tBackground(CGammaTables::ToSRGB(m_aBackground[0]), CGammaTables::ToSRGB(m_aBackground[1]), CGammaTables::ToSRGB(m_aBackground[2]), 255.0f*m_aBackground[3] + 0.5f);
		return RGBAInspectImpl(CImagePoint(m_rcContent.left, m_rcContent.top), CImageSize(m_rcContent.right-m_rcContent.left, m_rcContent.bottom-m_rcContent.top), m_pBuffer, m_rcContent.right-m_rcContent.left, tBackground, a_pOrigin, a_pSize, a_pVisitor, a_pControl);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::BufferLock(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	if (a_nChannelID != EICIRGBA)
		return E_RW_INVALIDPARAM;
	try
	{
		CDocumentReadLock cLock(this);
		UpdateContentCache();
		UpdateContentSize();
		TImagePoint const tOrg = {m_rcContent.left, m_rcContent.top};
		TImageSize const tSize = {m_rcContent.right > m_rcContent.left ? m_rcContent.right-m_rcContent.left : 0, m_rcContent.bottom > m_rcContent.top ? m_rcContent.bottom-m_rcContent.top : 0};

		if (a_pAllocOrigin) *a_pAllocOrigin = tOrg;
		if (a_pAllocSize) *a_pAllocSize = tSize;
		if (a_pContentOrigin) *a_pContentOrigin = tOrg;
		if (a_pContentSize) *a_pContentSize = tSize;
		if (a_ppBuffer) *a_ppBuffer = m_pBuffer;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::BufferUnlock(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
{
	if (a_nChannelID != EICIRGBA && a_pBuffer == m_pBuffer)
		return E_RW_INVALIDPARAM;
	return S_OK;
}

STDMETHODIMP CDocumentVectorImage::CanvasSet(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* UNREF(a_pHelper))
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (a_pSize && (a_pSize->nX != m_tCanvas.nX || a_pSize->nY != m_tCanvas.nY))
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoCanvasSize::Add(M_Base(), this, m_tCanvas);
			}

			m_tCanvas = *a_pSize;
			m_bChangeSize = true;
			++m_dwTimeStamp;
		}
		if (a_pResolution)
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoResolution::Add(M_Base(), this, m_tResolution);
			}

			m_tResolution = *a_pResolution;
			m_bChangeSize = true;
		}
		if (a_pContentTransform)
		{
			float const fScale = Matrix3x3fDecomposeScale(*a_pContentTransform);
			for (CElements::iterator i = m_cElements.begin(); i != m_cElements.end(); ++i)
			{
				i->second.fOutlineWidth *= fScale;
				i->second.pTool->Transform(a_pContentTransform);
				i->second.pTool->SetGlobals(EBMDrawOver, i->second.eRasterizationMode, i->second.eCoordinatesMode);
				i->second.pTool->SetOutline(i->second.bOutline, i->second.fOutlineWidth, i->second.fOutlinePos, i->second.eOutlineJoins, &i->second.tOutlineColor);
				i->second.bChangeModes = true;
				i->second.bChangeParams = true;
				++i->second.dwTimeStamp;
				CComQIPtr<IRasterImageEditToolScripting> pScripting(i->second.pTool);
				CComBSTR bstrParams;
				pScripting->ToText(&bstrParams);
				std::swap(i->second.bstrToolParams.m_str, bstrParams.m_str);
				pScripting = i->second.pFill;
				CComBSTR bstrParams2;
				pScripting->ToText(&bstrParams2);
				std::swap(i->second.bstrStyleParams.m_str, bstrParams2.m_str);
			}
			m_bContent = false;
			m_bBuffer = false;
			if (M_Base()->UndoEnabled() == S_OK)
			{
				TMatrix3x3f tInv;
				Matrix3x3fInverse(*a_pContentTransform, &tInv);
				CUndoTransform::Add(M_Base(), this, tInv);
			}
			++m_dwTimeStamp;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ChannelsSet(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
{
	try
	{
		CDocumentWriteLock cLock(this);
		for (ULONG i = 0; i < a_nChannels; ++i)
		{
			switch (a_aChannelIDs[i])
			{
			case EICIRGBA:
				{
					if (M_Base()->UndoEnabled() == S_OK)
					{
						CUndoChannels::Add(M_Base(), this, static_cast<TPixelChannel>(CPixelChannel(CGammaTables::ToSRGB(m_aBackground[0]), CGammaTables::ToSRGB(m_aBackground[1]),CGammaTables::ToSRGB(m_aBackground[2]), m_aBackground[3]*255.0f+0.5f)));
					}

					m_aBackground[0] = CGammaTables::FromSRGB(a_aChannelDefaults[i].bR);
					m_aBackground[1] = CGammaTables::FromSRGB(a_aChannelDefaults[i].bG);
					m_aBackground[2] = CGammaTables::FromSRGB(a_aChannelDefaults[i].bB);
					m_aBackground[3] = a_aChannelDefaults[i].bA/255.0f;
					m_bChangeBackground = true;
					m_bBuffer = false;
				}
				break;
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectIDs(IEnum2UInts* a_pIDs)
{
	try
	{
		CDocumentReadLock cLock(this);
		ULONG nStart = 0;
		ULONG nCount = m_cElementOrder.size();
		a_pIDs->Range(&nStart, &nCount);
		if (nCount > 0 && nStart < m_cElementOrder.size())
		{
			ULONG const nEnd = m_cElementOrder.size() < nStart+nCount ? m_cElementOrder.size() : nStart+nCount;
			return a_pIDs->Consume(nStart, nEnd-nStart, &(m_cElementOrder[nStart]));
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectGet(ULONG a_nID, BSTR* a_pToolID, BSTR* a_pToolParams)
{
	try
	{
		if (a_pToolID) *a_pToolID = NULL;
		if (a_pToolParams) *a_pToolParams = NULL;
		CDocumentReadLock cLock(this);
		CElements::const_iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		if (a_pToolID && FAILED(i->second.bstrToolID.CopyTo(a_pToolID))) return E_FAIL;
		if (a_pToolParams && FAILED(i->second.bstrToolParams.CopyTo(a_pToolParams))) return E_FAIL;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

//#include "../../RWProcessing/RWOperationImageRaster/RWOperationImageRaster.h"

//class CRasterImageCallback : public CStackUnknown<IRasterImageCallback>
//{
//public:
//	CRasterImageCallback(IRasterImageEditTool* pTool, RECT rcDirty) : pTool(pTool), rcDirty(rcDirty) {}
//
//	// IRasterImageCallback methods
//public:
//	STDMETHOD(Initalize)(ULONG a_nPixels, TPixelChannel* a_aPixels)
//	{
//		ZeroMemory(a_aPixels, a_nPixels*sizeof*a_aPixels);
//		return pTool->GetImageTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, 2.2f, rcDirty.right-rcDirty.left, reinterpret_cast<TRasterImagePixel*>(a_aPixels));
//	}
//
//private:
//	IRasterImageEditTool* pTool;
//	RECT rcDirty;
//};

//struct CPixelMixerPaintOver
//{
//	static void Mix(TPixelChannel& a_tP1, TPixelChannel const& a_tP2)
//	{
//		if (a_tP2.bA == 255)
//		{
//			a_tP1 = a_tP2;
//		}
//		else
//		{
//			// blend pixels
//			ULONG nNewA = a_tP2.bA*255 + (255-a_tP2.bA)*a_tP1.bA;
//			if (nNewA)
//			{
//				ULONG const bA1 = (255-a_tP2.bA)*a_tP1.bA;
//				ULONG const bA2 = a_tP2.bA*255;
//				a_tP1.bB = (a_tP1.bB*bA1 + a_tP2.bB*bA2)/nNewA;
//				a_tP1.bG = (a_tP1.bG*bA1 + a_tP2.bG*bA2)/nNewA;
//				a_tP1.bR = (a_tP1.bR*bA1 + a_tP2.bR*bA2)/nNewA;
//			}
//			else
//			{
//				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
//			}
//			a_tP1.bA = nNewA/255;
//		}
//	}
//};


STDMETHODIMP CDocumentVectorImage::ObjectSet(ULONG* a_pID, BSTR a_bstrToolID, BSTR a_bstrToolParams)
{
	try
	{
		CDocumentWriteLock cLock(this);
		ULONG nDummy = 0;
		if (a_pID == NULL) a_pID = &nDummy;
		if (*a_pID == 0)
		{
			// insert new element
			if (a_bstrToolID == NULL)
				return S_FALSE;
			*a_pID = m_nNextID++;
			m_cElementOrder.reserve(m_cElementOrder.size()+1);
			SElement s;
			s.bstrToolID = a_bstrToolID;
			s.bstrToolParams = a_bstrToolParams;

			CComPtr<IConfigWithDependencies> pCfg;
			RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
			CComBSTR bstrCFGID_LAYEREFFECT(CFGID_LAYEREFFECT);
			M_OpMgr()->InsertIntoConfigAs(M_OpMgr(), pCfg, CComBSTR(CFGID_LAYEREFFECT), NULL, NULL, 0, NULL);
			pCfg->Finalize(NULL);
			//pCfg->ItemValuesSet(1, &(bstrCFGID_LAYEREFFECT.m_str), CConfigValue(__uuidof(DocumentOperationRasterImageDropShadow)));
			s.pEffect = pCfg;

			m_pToolMgr->EditToolCreate(a_bstrToolID, NULL, &s.pTool);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(s.pTool);
			if (pScripting == NULL)
				return E_FAIL;
			s.pTool->Init(m_pToolWindow);
			if (a_bstrToolParams)
				pScripting->FromText(a_bstrToolParams);

			m_cElements[*a_pID] = s;
			m_cElementOrder.push_back(*a_pID);
			m_bChangeElements = true;
			m_bContent = false;
			m_bBuffer = false;

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoObjectIns::Add(M_Base(), this, *a_pID);
			}
			return S_OK;
		}
		CElements::iterator i = m_cElements.find(*a_pID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		if (a_bstrToolID == NULL)
		{
			m_pToolWindow->SetROI(CImagePoint(0, 0), m_tCanvas, NULL, CPixelChannel(0, 0, 0, 0), m_tCanvas);// TODO: delete when IsDirty is not rendering the item
			ULONG nIndex = 0;
			// delete element
			for (CElementOrder::iterator j = m_cElementOrder.begin(); j != m_cElementOrder.end(); ++j)
				if (*j == *a_pID)
				{
					RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
					GetDirtyRect(i->second, rc);
					if (rc.left < rc.right && rc.top < rc.bottom)
					{
						if (m_rcDirty.left > rc.left) m_rcDirty.left = rc.left;
						if (m_rcDirty.top > rc.top) m_rcDirty.top = rc.top;
						if (m_rcDirty.right < rc.right) m_rcDirty.right = rc.right;
						if (m_rcDirty.bottom < rc.bottom) m_rcDirty.bottom = rc.bottom;
					}
					nIndex = j-m_cElementOrder.begin();
					m_cElementOrder.erase(j);
					m_bChangeElements = true;
					break;
				}
			m_bContent = false;
			m_bBuffer = false;
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoObjectDel::Add(M_Base(), this, *a_pID, nIndex, i->second.bstrName.m_str,
					i->second.bstrToolID.m_str, i->second.bstrToolParams.m_str,
					i->second.bFill, i->second.bstrStyleID.m_str, i->second.bstrStyleParams.m_str,
					i->second.bOutline, i->second.tOutlineColor, i->second.fOutlineWidth,
					i->second.fOutlinePos, i->second.eOutlineJoins,
					i->second.eRasterizationMode, i->second.eCoordinatesMode);
			}
			m_cElements.erase(i);
			return S_OK;
		}
		// change existing element
		if (i->second.bstrToolID != a_bstrToolID)
		{
			m_pToolWindow->SetROI(CImagePoint(0, 0), m_tCanvas, NULL, CPixelChannel(0, 0, 0, 0), m_tCanvas);// TODO: delete when IsDirty is not rendering the item
			RECT rcPrev = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			if (i->second.pTool && S_OK == GetDirtyRect(i->second, rcPrev))
			{
				if (m_rcDirty.left > rcPrev.left) m_rcDirty.left = rcPrev.left;
				if (m_rcDirty.top > rcPrev.top) m_rcDirty.top = rcPrev.top;
				if (m_rcDirty.right < rcPrev.right) m_rcDirty.right = rcPrev.right;
				if (m_rcDirty.bottom < rcPrev.bottom) m_rcDirty.bottom = rcPrev.bottom;
			}
			CComPtr<IRasterImageEditTool> pTool;
			m_pToolMgr->EditToolCreate(a_bstrToolID, NULL, &pTool);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(pTool);
			if (pScripting == NULL)
				return E_FAIL;
			pTool->Init(m_pToolWindow);
			if (a_bstrToolParams)
				pScripting->FromText(a_bstrToolParams);
			if (i->second.pFill)
				pTool->SetBrush(i->second.pFill);
			//pTool->SetColors(&i->second.tColor1, &i->second.tColor2);
			pTool->SetGlobals(EBMDrawOver, i->second.eRasterizationMode, i->second.eCoordinatesMode);
			pTool->SetOutline(i->second.bOutline, i->second.fOutlineWidth, i->second.fOutlinePos, i->second.eOutlineJoins, &i->second.tOutlineColor);

			CComBSTR bstrNewID(a_bstrToolID);
			CComBSTR bstrNewParams(a_bstrToolParams);
			CComBSTR bstrPrevID;
			CComBSTR bstrPrevParams;
			bstrPrevID.Attach(i->second.bstrToolID.Detach());
			bstrPrevParams.Attach(i->second.bstrToolParams.Detach());

			std::swap(pTool.p, i->second.pTool.p);
			i->second.bstrToolID.Attach(bstrNewID.Detach());
			i->second.bstrToolParams.Attach(bstrNewParams.Detach());
			i->second.bChangeParams = i->second.bChangeToolID = true;
			++i->second.dwTimeStamp;
			m_bContent = false;
			m_bBuffer = false;

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoObjectSet::Add(M_Base(), this, *a_pID, bstrPrevID, bstrPrevParams);
			}
		}
		else if (i->second.bstrToolParams != a_bstrToolParams)
		{
			CComBSTR bstrNewParams(a_bstrToolParams);
			CComBSTR bstrPrevID(i->second.bstrToolID);
			CComBSTR bstrPrevParams;
			bstrPrevParams.Attach(i->second.bstrToolParams.Detach());

			CComQIPtr<IRasterImageEditToolScripting> pScripting(i->second.pTool);
			pScripting->FromText(a_bstrToolParams);
			i->second.bstrToolParams.Attach(bstrNewParams.Detach());
			i->second.bChangeParams = i->second.bChangeToolID = true;
			m_bContent = false;
			m_bBuffer = false;

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoObjectSet::Add(M_Base(), this, *a_pID, bstrPrevID, bstrPrevParams);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentVectorImage::ObjectRestore(ULONG a_nID, ULONG a_nIndex, CComBSTR& a_bstrName, CComBSTR& a_bstrToolID, CComBSTR& a_bstrToolParams,
		CComBSTR& a_bstrStyleID, CComBSTR& a_bstrStyleParams, BOOL a_bFill, BOOL a_bOutline, TColor const& a_tColor, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins,
		ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (m_cElements.find(a_nID) != m_cElements.end() || m_cElementOrder.size() < a_nIndex)
			return E_FAIL;

		CComPtr<IRasterImageEditTool> pTool;
		m_pToolMgr->EditToolCreate(a_bstrToolID, NULL, &pTool);
		CComQIPtr<IRasterImageEditToolScripting> pScripting(pTool);
		if (pScripting == NULL)
			return E_FAIL;
		pTool->Init(m_pToolWindow);
		if (a_bstrToolParams)
			pScripting->FromText(a_bstrToolParams);
		pTool->SetGlobals(EBMDrawOver, a_eRasterizationMode, a_eCoordinatesMode);
		pTool->SetOutline(a_bOutline, a_fWidth, a_fPos, a_eJoins, &a_tColor);
		//pTool->SetColors(&a_tColor1, &a_tColor2);

		CComPtr<IRasterImageBrush> pFill;
		if (a_bstrStyleID.Length())
		{
			m_pFillMgr->FillStyleCreate(a_bstrStyleID, NULL, &pFill);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(pFill);
			if (pScripting)
			{
				if (a_bstrStyleParams)
					pScripting->FromText(a_bstrStyleParams);
			}
		}
		if (pFill)
			pTool->SetBrush(pFill);

		m_cElementOrder.reserve(m_cElementOrder.size()+1);
		SElement& sEl = m_cElements[a_nID];
		sEl.bstrName.Attach(a_bstrName.Detach());
		sEl.bstrToolID.Attach(a_bstrToolID.Detach());
		sEl.bstrToolParams.Attach(a_bstrToolParams.Detach());
		sEl.bstrStyleID.Attach(a_bstrStyleID.Detach());
		sEl.bstrStyleParams.Attach(a_bstrStyleParams.Detach());
		sEl.eCoordinatesMode = a_eCoordinatesMode;
		sEl.eRasterizationMode = a_eRasterizationMode;
		sEl.bFill = a_bFill;
		sEl.bOutline = a_bOutline;
		sEl.fOutlineWidth = a_fWidth;
		sEl.tOutlineColor = a_tColor;
		sEl.pTool.Attach(pTool.Detach());
		sEl.pFill.Attach(pFill.Detach());
		sEl.bChangeOutline = sEl.bChangeModes = sEl.bChangeName = sEl.bChangeParams = sEl.bChangeStyle = sEl.bChangeToolID = false;
		++sEl.dwTimeStamp;
		m_cElementOrder.insert(m_cElementOrder.begin()+a_nIndex, a_nID);
		m_bChangeElements = true;
		m_bContent = false;
		m_bBuffer = false;

		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoObjectIns::Add(M_Base(), this, a_nID);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectNameGet(ULONG a_nID, BSTR* a_pName)
{
	try
	{
		*a_pName = NULL;
		CDocumentReadLock cLock(this);
		CElements::const_iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		return i->second.bstrName.CopyTo(a_pName);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectNameSet(ULONG a_nID, BSTR a_bstrName)
{
	try
	{
		CDocumentWriteLock cLock(this);
		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		CComBSTR bstr;
		if (a_bstrName == NULL)
		{
			CMultiLanguageString::GetLocalized(L"[0409]object[0405]objekt", GetThreadLocale(), &bstr);
			bstr += L" ";
			ULONG prefLen = bstr.Length();
			int max = -1;
			for (CElements::const_iterator j = m_cElements.begin(); j != m_cElements.end(); ++j)
			{
				if (i == j) continue;
				if (j->second.bstrName == NULL || j->second.bstrName.Length() <= prefLen ||
					wcsncmp(bstr, j->second.bstrName, prefLen) != 0) continue;
				int n = _wtoi(j->second.bstrName.m_str+prefLen);
				if (n != 0 || j->second.bstrName.m_str[prefLen] == L'0')
				{
					if (n > max) max = n;
				}
			}

			wchar_t sz[64] = L"";
			wsprintf(sz, L"%i", max+1);
			bstr += sz;
			a_bstrName = bstr;
		}
		if (i->second.bstrName == a_bstrName)
			return S_FALSE;
		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoObjectNameSet::Add(M_Base(), this, a_nID, i->second.bstrName);
		}
		i->second.bstrName = a_bstrName;
		i->second.bChangeName = true;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectStyleGet(ULONG a_nID, BSTR* a_pStyleID, BSTR* a_pStyleParams)
{
	try
	{
		CDocumentReadLock cLock(this);
		CElements::const_iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		if (a_pStyleID)
			i->second.bstrStyleID.CopyTo(a_pStyleID);
		if (a_pStyleParams)
			i->second.bstrStyleParams.CopyTo(a_pStyleParams);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectStyleSet(ULONG a_nID, BSTR a_bstrStyleID, BSTR a_bstrStyleParams)
{
	try
	{
		CDocumentWriteLock cLock(this);
		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		if (i->second.bstrStyleID != a_bstrStyleID)
		{
			CComPtr<IRasterImageBrush> pFill;
			m_pFillMgr->FillStyleCreate(a_bstrStyleID, NULL, &pFill);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(pFill);
			if (pScripting)
			{
				CComBSTR bstrNewID(a_bstrStyleID);
				CComBSTR bstrNewParams(a_bstrStyleParams);
				CComBSTR bstrPrevID;
				CComBSTR bstrPrevParams;
				bstrPrevID.Attach(i->second.bstrStyleID.Detach());
				bstrPrevParams.Attach(i->second.bstrStyleParams.Detach());

				if (a_bstrStyleParams)
					pScripting->FromText(a_bstrStyleParams);
				std::swap(i->second.pFill.p, pFill.p);
				i->second.bstrStyleID.Attach(bstrNewID.Detach());
				i->second.bstrStyleParams.Attach(bstrNewParams.Detach());
				i->second.bChangeStyle = true;
				++i->second.dwTimeStamp;
				//i->second.pFill->SetColors(&i->second.tColor1, &i->second.tColor2);
				i->second.pTool->SetBrush(i->second.bFill ? i->second.pFill : NULL);

				m_bBuffer = false;
				if (M_Base()->UndoEnabled() == S_OK)
				{
					CUndoObjectStyleSet::Add(M_Base(), this, a_nID, bstrPrevID, bstrPrevParams);
				}
			}
		}
		else if (i->second.bstrStyleParams != a_bstrStyleParams)
		{
			CComQIPtr<IRasterImageEditToolScripting> pScripting(i->second.pFill);
			if (pScripting)
			{
				CComBSTR bstrNewParams(a_bstrStyleParams);
				CComBSTR bstrPrevID(i->second.bstrStyleID);
				CComBSTR bstrPrevParams;
				bstrPrevParams.Attach(i->second.bstrStyleParams.Detach());

				if (a_bstrStyleParams)
					pScripting->FromText(a_bstrStyleParams);
				i->second.bstrStyleParams.Attach(bstrNewParams.Detach());
				i->second.bChangeStyle = true;
				++i->second.dwTimeStamp;
				//i->second.pFill->SetColors(&i->second.tColor1, &i->second.tColor2);
				i->second.pTool->SetBrush(i->second.bFill ? i->second.pFill : NULL);

				m_bBuffer = false;
				if (M_Base()->UndoEnabled() == S_OK)
				{
					CUndoObjectStyleSet::Add(M_Base(), this, a_nID, bstrPrevID, bstrPrevParams);
				}
			}
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectStateGet(ULONG a_nID, BOOL* a_pFill, ERasterizationMode* a_pRasterizationMode, ECoordinatesMode* a_pCoordinatesMode, BOOL* a_pOutline, TColor* a_pOutlineColor, float* a_pOutlineWidth, float* a_pOutlinePos, EOutlineJoinType* a_pOutlineJoins)
{
	try
	{
		CDocumentReadLock cLock(this);
		CElements::const_iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		if (a_pFill) *a_pFill = i->second.bFill;
		if (a_pRasterizationMode) *a_pRasterizationMode = i->second.eRasterizationMode;
		if (a_pCoordinatesMode) *a_pCoordinatesMode = i->second.eCoordinatesMode;
		if (a_pOutline) *a_pOutline = i->second.bOutline;
		if (a_pOutlineColor) *a_pOutlineColor = i->second.tOutlineColor;
		if (a_pOutlineWidth) *a_pOutlineWidth = i->second.fOutlineWidth;
		if (a_pOutlinePos) *a_pOutlinePos = i->second.fOutlinePos;
		if (a_pOutlineJoins) *a_pOutlineJoins = i->second.eOutlineJoins;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool IsSameColor(TColor const& a_1, TColor const& a_2)
{
	float const f =
		(a_1.fR*a_1.fA-a_2.fR*a_2.fA)*(a_1.fR*a_1.fA-a_2.fR*a_2.fA) +
		(a_1.fG*a_1.fA-a_2.fG*a_2.fA)*(a_1.fG*a_1.fA-a_2.fG*a_2.fA) +
		(a_1.fB*a_1.fA-a_2.fB*a_2.fA)*(a_1.fB*a_1.fA-a_2.fB*a_2.fA) +
		(a_1.fA-a_2.fA)*(a_1.fA-a_2.fA);
	return f < 1.0f/(255*255);
}

STDMETHODIMP CDocumentVectorImage::ObjectStateSet(ULONG a_nID, BOOL const* a_pFill, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, BOOL const* a_pOutline, TColor const* a_pOutlineColor, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins)
{
	try
	{
		CDocumentWriteLock cLock(this);
		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		TColor tOutlineColor = i->second.tOutlineColor;
		float fWidth = i->second.fOutlineWidth;
		float fPos = i->second.fOutlinePos;
		EOutlineJoinType eJoins = i->second.eOutlineJoins;
		ERasterizationMode eRM = i->second.eRasterizationMode;
		ECoordinatesMode eCM = i->second.eCoordinatesMode;
		BOOL bFill = i->second.bFill;
		BOOL bOutline = i->second.bOutline;

		bool bChangeOutline = false;
		if (a_pOutlineColor && !IsSameColor(i->second.tOutlineColor, *a_pOutlineColor))
		{
			bChangeOutline = i->second.bChangeOutline = true;
			i->second.tOutlineColor = *a_pOutlineColor;
			++i->second.dwTimeStamp;
		}
		if (a_pOutline && i->second.bOutline != *a_pOutline)
		{
			bChangeOutline = i->second.bChangeOutline = true;
			i->second.bOutline = *a_pOutline;
			++i->second.dwTimeStamp;
		}
		bool bChangeModes = false;
		if (a_pRasterizationMode)
		{
			if (i->second.eRasterizationMode != *a_pRasterizationMode) bChangeModes = i->second.bChangeModes = true;
			i->second.eRasterizationMode = *a_pRasterizationMode;
		}
		if (a_pCoordinatesMode)
		{
			if (i->second.eCoordinatesMode != *a_pCoordinatesMode) bChangeModes = i->second.bChangeModes = true;
			i->second.eCoordinatesMode = *a_pCoordinatesMode;
		}
		bool bChangeFill = false;
		if (a_pFill)
		{
			if (i->second.bFill != *a_pFill) bChangeFill = i->second.bChangeStyle = true;
			i->second.bFill = *a_pFill;
			++i->second.dwTimeStamp;
		}
		if (a_pOutlineWidth)
		{
			if (i->second.fOutlineWidth != *a_pOutlineWidth) bChangeOutline = i->second.bChangeOutline = true;
			i->second.fOutlineWidth = *a_pOutlineWidth;
			++i->second.dwTimeStamp;
		}
		if (a_pOutlinePos)
		{
			if (i->second.fOutlinePos != *a_pOutlinePos) bChangeOutline = i->second.bChangeOutline = true;
			i->second.fOutlinePos = *a_pOutlinePos;
			++i->second.dwTimeStamp;
		}
		if (a_pOutlineJoins)
		{
			if (i->second.eOutlineJoins != *a_pOutlineJoins) bChangeOutline = i->second.bChangeOutline = true;
			i->second.eOutlineJoins = *a_pOutlineJoins;
			++i->second.dwTimeStamp;
		}

		if (bChangeFill)
		{
			i->second.pTool->SetBrush(i->second.bFill ? i->second.pFill : NULL);
		}
		if (bChangeModes)
		{
			i->second.pTool->SetGlobals(EBMDrawOver, i->second.eRasterizationMode, i->second.eCoordinatesMode);
		}
		if (bChangeOutline)
		{
			i->second.pTool->SetOutline(i->second.bOutline, i->second.fOutlineWidth, i->second.fOutlinePos, i->second.eOutlineJoins, &i->second.tOutlineColor);
		}

		if ((bChangeModes || bChangeOutline || bChangeFill) && M_Base()->UndoEnabled() == S_OK)
		{
			m_bContent = false;
			m_bBuffer = false;
			CUndoObjectStateSet::Add(M_Base(), this, a_nID, bFill, bOutline, tOutlineColor, fWidth, fPos, eJoins, eRM, eCM);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectEffectGet(ULONG a_nID, IConfig** a_ppOperation)
{
	try
	{
		*a_ppOperation = NULL;
		CDocumentReadLock cLock(this);

		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		return i->second.pEffect->DuplicateCreate(a_ppOperation);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectEffectSet(ULONG a_nID, IConfig* a_pOperation)
{
	try
	{
		CDocumentWriteLock cLock(this);

		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		//if (M_Base()->UndoEnabled() == S_OK)
		//{
		//	CUndoLayerEffect::Add(M_Base(), this, a_pItem, pL->pEffect);
		//}
		i->second.bChangeEffect = true;
		//DeleteCacheForLayer(pL->nUID, ECPProcessed);
		return CopyConfigValues(i->second.pEffect, a_pOperation);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectIsEnabled(ULONG a_nID)
{
	try
	{
		CDocumentReadLock cLock(this);

		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		return i->second.bEnabled ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectEnable(ULONG a_nID, BOOL a_bEnabled)
{
	try
	{
		CDocumentWriteLock cLock(this);

		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		bool bEnabled = a_bEnabled;
		if (bEnabled == i->second.bEnabled)
			return S_FALSE;

		m_bContent = false;
		m_bBuffer = false;
		if (M_Base()->UndoEnabled() == S_OK)
		{
			CUndoObjectVisibilitySet::Add(M_Base(), this, a_nID, i->second.bEnabled);
		}
		RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		if (S_OK == i->second.pTool->IsDirty(&rc, NULL, NULL))
		{
			if (m_rcDirty.left > rc.left) m_rcDirty.left = rc.left;
			if (m_rcDirty.top > rc.top) m_rcDirty.top = rc.top;
			if (m_rcDirty.right < rc.right) m_rcDirty.right = rc.right;
			if (m_rcDirty.bottom < rc.bottom) m_rcDirty.bottom = rc.bottom;
		}
		i->second.bEnabled = bEnabled;
		i->second.bChangeVisibility = true;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectsMove(ULONG a_nCount, ULONG const* a_aShapeIDs, ULONG a_nUnder)
{
	try
	{
		CDocumentWriteLock cLock(this);
		std::set<ULONG> aNew;
		RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			CElements::const_iterator iE = m_cElements.find(a_aShapeIDs[i]);
			if (iE == m_cElements.end())
				return E_RW_ITEMNOTFOUND;
			aNew.insert(a_aShapeIDs[i]);
			RECT rc2 = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			GetDirtyRect(iE->second, rc2);
			if (rc.left > rc2.left) rc.left = rc2.left;
			if (rc.top > rc2.top) rc.top = rc2.top;
			if (rc.right < rc2.right) rc.right = rc2.right;
			if (rc.bottom < rc2.bottom) rc.bottom = rc2.bottom;
		}
		if (aNew.size() != a_nCount)
			return E_RW_ITEMNOTFOUND;

		CElementOrder cNew;
		cNew.resize(m_cElementOrder.size());
		CElementOrder::iterator iDst = cNew.begin();
		for (CElementOrder::const_iterator i = m_cElementOrder.begin(); i != m_cElementOrder.end(); ++i)
		{
			if (*i == a_nUnder)
			{
				for (ULONG j = 0; j < a_nCount; ++j, ++iDst)
					*iDst = a_aShapeIDs[j];
			}
			if (aNew.find(*i) == aNew.end())
			{
				*iDst = *i;
				++iDst;
			}
		}
		if (iDst != cNew.end())
			for (ULONG j = 0; j < a_nCount; ++j, ++iDst)
				*iDst = a_aShapeIDs[j];
		if (m_cElementOrder != cNew)
		{
			std::swap(m_cElementOrder, cNew);
			if (m_rcDirty.left > rc.left) m_rcDirty.left = rc.left;
			if (m_rcDirty.top > rc.top) m_rcDirty.top = rc.top;
			if (m_rcDirty.right < rc.right) m_rcDirty.right = rc.right;
			if (m_rcDirty.bottom < rc.bottom) m_rcDirty.bottom = rc.bottom;
			m_bChangeElements = true;

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoReorder::Add(M_Base(), this, cNew, rc);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectTransform(ULONG a_nID, TMatrix3x3f const* a_pContentTransform)
{
	try
	{
		CDocumentWriteLock cLock(this);
		CElements::iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;

		float const fScale = Matrix3x3fDecomposeScale(*a_pContentTransform);

		RECT rc1 = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		GetDirtyRect(i->second, rc1);

		i->second.pTool->Transform(a_pContentTransform);
		i->second.pTool->SetGlobals(EBMDrawOver, i->second.eRasterizationMode, i->second.eCoordinatesMode);
		float const outline = i->second.fOutlineWidth;
		if (fScale != 1.0f)
		{
			i->second.fOutlineWidth *= fScale;
			i->second.pTool->SetOutline(i->second.bOutline, i->second.fOutlineWidth, i->second.fOutlinePos, i->second.eOutlineJoins, &i->second.tOutlineColor);
			i->second.bChangeModes = true;
		}
		i->second.bChangeParams = true;
		++i->second.dwTimeStamp;
		CComQIPtr<IRasterImageEditToolScripting> pScripting(i->second.pTool);
		CComBSTR bstrParams;
		pScripting->ToText(&bstrParams);
		std::swap(i->second.bstrToolParams.m_str, bstrParams.m_str);
		m_bContent = false;
		m_bBuffer = false;

		RECT rc2 = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		GetDirtyRect(i->second, rc2);
		InvalidateRectangle(&rc1);
		InvalidateRectangle(&rc2);
		

		if (M_Base()->UndoEnabled() == S_OK)
		{
			if (fScale != 1.0f)
				CUndoObjectStateSet::Add(M_Base(), this, a_nID, i->second.bFill, i->second.bOutline, i->second.tOutlineColor, outline, i->second.fOutlinePos, i->second.eOutlineJoins, i->second.eRasterizationMode, i->second.eCoordinatesMode);
			CUndoObjectSet::Add(M_Base(), this, a_nID, i->second.bstrToolID, bstrParams);
		}

		++m_dwTimeStamp;

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ObjectBounds(ULONG a_nID, TPixelCoords* a_pTL, TPixelCoords* a_pBR)
{
	try
	{
		CDocumentReadLock cLock(this);
		CElements::const_iterator i = m_cElements.find(a_nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		GetDirtyRect(i->second, rc);
		if (rc.top >= rc.bottom || rc.left >= rc.right)
			return S_FALSE;
		if (a_pTL)
		{
			a_pTL->fX = rc.left;
			a_pTL->fY = rc.top;
		}
		if (a_pBR)
		{
			a_pBR->fX = rc.right;
			a_pBR->fY = rc.bottom;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentVectorImage::ObjectsReorder(std::vector<ULONG> const& a_cShapeIDs, RECT a_rcDirty)
{
	CDocumentWriteLock cLock(this);

	CElementOrder cNew(a_cShapeIDs);
	std::swap(m_cElementOrder, cNew);
	if (m_rcDirty.left > a_rcDirty.left) m_rcDirty.left = a_rcDirty.left;
	if (m_rcDirty.top > a_rcDirty.top) m_rcDirty.top = a_rcDirty.top;
	if (m_rcDirty.right < a_rcDirty.right) m_rcDirty.right = a_rcDirty.right;
	if (m_rcDirty.bottom < a_rcDirty.bottom) m_rcDirty.bottom = a_rcDirty.bottom;
	m_bChangeElements = true;
	m_bBuffer = false;

	if (M_Base()->UndoEnabled() == S_OK)
	{
		CUndoReorder::Add(M_Base(), this, cNew, a_rcDirty);
	}

	return S_OK;
}

//class CNormalBlender : public CStackUnknown<IImageVisitor>
//{
//public:
//	CNormalBlender(TImagePoint const& a_tOrigin, TImageSize const& a_tSize, TPixelChannel* a_pBuffer) :
//		tOrigin(a_tOrigin), tSize(a_tSize), pBuffer(a_pBuffer) {}
//
//	// IImageVisitor methods
//public:
//	STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl *a_pControl)
//	{
//		for (TImageTile const* pTile = a_aTiles; pTile != a_aTiles+a_nTiles; ++pTile)
//		{
//			LONG const y1 = max(tOrigin.nY, pTile->tOrigin.nY);
//			LONG const y2 = min(tOrigin.nY+tSize.nY, pTile->tOrigin.nY+pTile->tSize.nY);
//			for (LONG y = y1; y < y2; ++y)
//			{
//				LONG const x1 = max(tOrigin.nX, pTile->tOrigin.nX);
//				LONG const x2 = min(tOrigin.nX+tSize.nX, pTile->tOrigin.nX+pTile->tSize.nX);
//				TPixelChannel* pD = pBuffer + (y-tOrigin.nY)*tSize.nX + (x1-tOrigin.nX);
//				TPixelChannel const* pS = pTile->pData + (y-pTile->tOrigin.nY)*pTile->tStride.nY + (x1-pTile->tOrigin.nX)*pTile->tStride.nX;
//				TPixelChannel* const pDEnd = pD+(x2-x1);
//				for (; pD < pDEnd; ++pD, pS+=pTile->tStride.nX)
//				{
//					CPixelMixerPaintOver::Mix(*pD, *pS);
//				}
//			}
//		}
//		return S_OK;
//	}
//
//private:
//	TImagePoint const tOrigin;
//	TImageSize const tSize;
//	TPixelChannel* pBuffer;
//};


void CDocumentVectorImage::Render(TImagePoint const& a_tOrigin, TImageSize const& a_tSize, TPixelChannel* a_pBuffer)
{
	CPixelChannel tBackground(CGammaTables::ToSRGB(m_aBackground[0]), CGammaTables::ToSRGB(m_aBackground[1]), CGammaTables::ToSRGB(m_aBackground[2]), 255.0f*m_aBackground[3] + 0.5f);
	std::fill_n(a_pBuffer, a_tSize.nX*a_tSize.nY, static_cast<TPixelChannel>(tBackground));
	m_pToolWindow->SetROI(a_tOrigin, a_tSize, a_pBuffer, tBackground, m_tCanvas);
	for (CElementOrder::const_iterator i = m_cElementOrder.begin(); i != m_cElementOrder.end(); ++i)
	{
		SElement const& sEl = m_cElements[*i];
		if (!sEl.bEnabled)
			continue;
		RECT rcDirty = {0, 0, 0, 0};
		if (S_OK == sEl.pTool->IsDirty(&rcDirty, NULL, NULL))
		{
			if (rcDirty.left < a_tOrigin.nX) rcDirty.left = a_tOrigin.nX;
			if (rcDirty.top < a_tOrigin.nY) rcDirty.top = a_tOrigin.nY;
			if (rcDirty.right > LONG(a_tOrigin.nX+a_tSize.nX)) rcDirty.right = a_tOrigin.nX+a_tSize.nX;
			if (rcDirty.bottom > LONG(a_tOrigin.nY+a_tSize.nY)) rcDirty.bottom = a_tOrigin.nY+a_tSize.nY;

			if (IsNoOperation(sEl.pEffect))
			{
				// no effect
				if (rcDirty.left < rcDirty.right && rcDirty.top < rcDirty.bottom)
					sEl.pTool->GetImageTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, 2.2f, a_tSize.nX, reinterpret_cast<TRasterImagePixel*>(a_pBuffer)+a_tSize.nX*(rcDirty.top-a_tOrigin.nY)+rcDirty.left-a_tOrigin.nX);
			}
			//else
			//{
			//	CComBSTR bstrCFGID_LAYEREFFECT(CFGID_LAYEREFFECT);
			//	CConfigValue cEffectID;
			//	sEl.pEffect->ItemValueGet(bstrCFGID_LAYEREFFECT, &cEffectID);
			//	CComPtr<IConfig> pEffect;
			//	sEl.pEffect->SubConfigGet(bstrCFGID_LAYEREFFECT, &pEffect);
			//	CComObjectStackEx<CLayerImageAdjustment> cLIA;
			//	RECT rcAdjusted = rcDirty;
			//	cLIA.Init(EROSAdjustProcessed, &rcAdjusted);
			//	CComObjectStackEx<CLayerOperationContext> cLOC;
			//	if (FAILED(M_OpMgr()->Activate(M_OpMgr(), &cLIA, cEffectID, pEffect, &cLOC, NULL, 0)))
			//	{
			//		// operation does not support regional processing -> better mark the whole canvas as invalid
			//		rcAdjusted.left = 0;
			//		rcAdjusted.top = 0;
			//		rcAdjusted.right = m_nSizeX;
			//		rcAdjusted.bottom = m_nSizeY;
			//	}
			//	TImagePoint tOff = {rcAdjusted.left, rcAdjusted.top};
			//	TImageSize tSize = {rcAdjusted.right-rcAdjusted.left, rcAdjusted.bottom-rcAdjusted.top};
			//	//CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[tSize.nX*tSize.nY]);
			//	//CComObjectStackEx<CLayerRasterImageRectangle> cLRI;
			//	//TRasterImageRect tRect = { {a_nX, a_nY}, {a_nX+a_nSizeX, a_nY+a_nSizeY} };
			//	TImageSize tCanvas = {m_nSizeX, m_nSizeY};
			//	CPixelChannel chDef(0UL);
			//	//cLRI.Init(tCanvas, 2.2f, reinterpret_cast<TPixelChannel*>(cBuffer.m_p), tOff, tSize, chDef, reinterpret_cast<TPixelChannel*>(a_pData), tRect, a_nStride);

			//	CComPtr<IDocument> pDoc;
			//	RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
			//	CComQIPtr<IDocumentBase> pBase(pDoc);

			//	CComPtr<IDocumentFactoryRasterImageCallback> pFct;
			//	RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
			//	TRasterImageRect tRect = {{rcDirty.left, rcDirty.top}, {rcDirty.right, rcDirty.bottom}};
			//	pFct->Create(NULL, pBase, &tCanvas, NULL, 1, CChannelDefault(EICIRGBA), &tRect, &CRasterImageCallback(sEl.pTool, rcDirty));

			//	M_OpMgr()->Activate(M_OpMgr(), pDoc, cEffectID, pEffect, &cLOC, NULL, 0);

			//	CComPtr<IDocumentImage> pImg;
			//	pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));

			//	TImagePoint tOrig = {0, 0};
			//	TImageSize tCnt = {0, 0};
			//	pImg->CanvasGet(NULL, NULL, &tOrig, &tCnt, NULL);
			//	TImagePoint tEnd = {tOrig.nX+tCnt.nX, tOrig.nY+tCnt.nY};

			//	if (tOrig.nX < a_tOrigin.nX) tOrig.nX = a_tOrigin.nX;
			//	if (tOrig.nY < a_tOrigin.nY) tOrig.nY = a_tOrigin.nY;
			//	if (tEnd.nX > LONG(a_tOrigin.nX+a_tSize.nX)) tEnd.nX = a_tOrigin.nX+a_tSize.nX;
			//	if (tEnd.nY > LONG(a_tOrigin.nY+a_tSize.nY)) tEnd.nY = a_tOrigin.nY+a_tSize.nY;

			//	if (tEnd.nX > tOrig.nX && tEnd.nY > tOrig.nY)
			//	{
			//		tCnt.nX = tEnd.nX-tOrig.nX;
			//		tCnt.nY = tEnd.nY-tOrig.nY;
			//		pImg->Inspect(EICIRGBA, &tOrig, &tCnt, &CNormalBlender(a_tOrigin, a_tSize, a_pBuffer), NULL, EIRIAccurate);
			//	}
			//}
		}
	}
}

STDMETHODIMP CDocumentVectorImage::StatePrefix(BSTR* a_pbstrPrefix)
{
	try
	{
		*a_pbstrPrefix = NULL;
		*a_pbstrPrefix = CComBSTR(m_bstrPreview.m_str ? m_bstrPreview.m_str : M_DataID()).Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrPrefix ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentVectorImage::StatePack(ULONG a_nIDs, ULONG const* a_pIDs, ISharedState** a_ppState)
{
	try
	{
		*a_ppState = NULL;
		CComPtr<ISharedState> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
		CComQIPtr<IEnumUnknownsInit> pInit(pTmp);
		for (ULONG i = 0; i < a_nIDs; ++i)
			InsertItem<CStructuredItemVectorImageElement>(pInit, this, a_pIDs[i]);
		*a_ppState = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppState == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::StateUnpack(ISharedState* a_pState, IEnum2UInts* a_pIDs)
{
	try
	{
		CComQIPtr<IEnumUnknowns> pTmp(a_pState);
		if (pTmp == NULL)
			return S_FALSE;

		ULONG nSize = 0;
		pTmp->Size(&nSize);
		ULONG i = 0;
		ULONG n = nSize;
		a_pIDs->Range(&i, &n);
		n += i;
		if (n > nSize) n = nSize;
		for (; i < nSize; ++i)
		{
			CComPtr<IStructuredItemVectorImageElement> pItem;
			if (SUCCEEDED(pTmp->Get(i, __uuidof(IStructuredItemVectorImageElement), reinterpret_cast<void**>(&pItem))))
			{
				ULONG nID;
				pItem->ID(&nID);
				a_pIDs->Consume(i, 1, &nID);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return a_pIDs ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::StatePack(ULONG a_nItems, IComparable* const* a_paItems, ISharedState** a_ppState)
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

STDMETHODIMP CDocumentVectorImage::StateUnpack(ISharedState* a_pState, IEnumUnknowns** a_ppSelectedItems)
{
	try
	{
		*a_ppSelectedItems = NULL;
		if (a_pState == NULL)
			return S_FALSE;
		return a_pState->QueryInterface(a_ppSelectedItems);
	}
	catch (...)
	{
		return a_ppSelectedItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentVectorImage::ItemsEnum(IComparable* a_pItem, IEnumUnknowns** a_ppSubItems)
{
	try
	{
		*a_ppSubItems = NULL;
		if (a_pItem != NULL)
			return S_FALSE;
		CComPtr<IEnumUnknownsInit> pInit;
		RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));
		for (CElementOrder::const_reverse_iterator i = m_cElementOrder.rbegin(); i != m_cElementOrder.rend(); ++i)
		{
			InsertItem<CStructuredItemVectorImageElement>(pInit, this, *i);
		}
		CDocumentReadLock cLock(this);
		*a_ppSubItems = pInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubItems == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

void CDocumentVectorImage::InvalidateRectangle(RECT const* a_pChanged)
{
	//if (a_pChanged)
	//{
	//	if (!IsNoOperation(sEl.pEffect))
	//	{
	//		CComBSTR bstrCFGID_LAYEREFFECT(CFGID_LAYEREFFECT);
	//		CConfigValue cEffectID;
	//		sEl.pEffect->ItemValueGet(bstrCFGID_LAYEREFFECT, &cEffectID);
	//		CComPtr<IConfig> pEffect;
	//		sEl.pEffect->SubConfigGet(bstrCFGID_LAYEREFFECT, &pEffect);
	//		CComObjectStackEx<CLayerImageAdjustment> cLIA;
	//		cLIA.Init(EROSAdjustDirty, &rc);
	//		CComObjectStackEx<CLayerOperationContext> cLOC;
	//		if (FAILED(M_OpMgr()->Activate(M_OpMgr(), &cLIA, cEffectID, pEffect, &cLOC, NULL, 0)))
	//		{
	//			// operation does not support regional processing -> better mark the whole canvas as invalid
	//			rc.left = 0;
	//			rc.top = 0;
	//			rc.right = m_nSizeX;
	//			rc.bottom = m_nSizeY;
	//		}
	//	}
	//	if (m_rcDirty.left > a_pChanged->left) m_rcDirty.left = a_pChanged->left;
	//	if (m_rcDirty.top > a_pChanged->top) m_rcDirty.top = a_pChanged->top;
	//	if (m_rcDirty.right < a_pChanged->right) m_rcDirty.right = a_pChanged->right;
	//	if (m_rcDirty.bottom < a_pChanged->bottom) m_rcDirty.bottom = a_pChanged->bottom;
	//}
	//else
	{
		if (m_rcDirty.left > 0) m_rcDirty.left = 0;
		if (m_rcDirty.top > 0) m_rcDirty.top = 0;
		if (m_rcDirty.right < LONG(m_tCanvas.nX)) m_rcDirty.right = m_tCanvas.nX;
		if (m_rcDirty.bottom < LONG(m_tCanvas.nY)) m_rcDirty.bottom = m_tCanvas.nY;
	}
}

STDMETHODIMP CDocumentVectorImage::CToolWindow::GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fSize, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
{
	static TImageSize const t = {1, 1};
	//RGBAGetTileImpl(t, m_tOrigin, m_tSize, m_pBuffer, m_tSize.nX, m_tDefault, CImagePoint(a_nX, a_nY), CImageSize(a_nSizeX, a_nSizeY), CImageStride(1, a_nStride), a_nStride*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer));
	return S_OK;
}

STDMETHODIMP CDocumentVectorImage::CToolWindow::GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
{
	if (a_pBoundingRectangle)
	{
		a_pBoundingRectangle->left = a_pBoundingRectangle->top = LONG_MIN;
		a_pBoundingRectangle->right = LONG_MAX;
		a_pBoundingRectangle->bottom = LONG_MAX;
	}
	if (a_bEntireRectangle)
		*a_bEntireRectangle = TRUE;
	return S_OK;
}

#include <DragDropHelper.h>

struct SDraggedShape
{
	CDocumentVectorImage* pThis;
	ULONG nIDs;
	ULONG aIDs[1];
};

STDMETHODIMP CDocumentVectorImage::Begin(IEnumUnknowns* a_pSelection, IDataObject** a_ppDataObject, IDropSource** a_ppDropSource, DWORD* a_pOKEffects)
{
	try
	{
		ULONG nItems = 0;
		a_pSelection->Size(&nItems);
		CAutoVectorPtr<BYTE> pBuf(new BYTE[sizeof(SDraggedShape)+sizeof(ULONG)*LONG(nItems-1)]);
		SDraggedShape* pShape = reinterpret_cast<SDraggedShape*>(pBuf.m_p);
		pShape->pThis = this;
		pShape->nIDs = 0;

		for (ULONG i = 0; i < nItems; ++i)
		{
			CComPtr<IStructuredItemVectorImageElement> pItem;
			a_pSelection->Get(i, __uuidof(IStructuredItemVectorImageElement), reinterpret_cast<void**>(&pItem));
			if (pItem)
				pItem->ID(&pShape->aIDs[pShape->nIDs++]);
		}
		if (pShape->nIDs == 0)
			return E_RW_ITEMNOTFOUND;

		CComObject<CDragDropHelper>* pDropSrc = NULL;
		CComObject<CDragDropHelper>::CreateInstance(&pDropSrc);
		CComPtr<IDropSource> pTmp = pDropSrc;

		if (!pDropSrc->InitStruct(RegisterClipboardFormat(_T("RWVI_SHAPEID")), pBuf, sizeof(SDraggedShape)+sizeof(ULONG)*LONG(pShape->nIDs-1)))
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

STDMETHODIMP CDocumentVectorImage::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
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
				*a_ppFeedback = NULL;
				*a_ppFeedback = new CMultiLanguageString(L"[0409]Insert new shapes[0405]Vložit nové tvary");
			}
			return S_OK;
		}
		FORMATETC tFE = { RegisterClipboardFormat(_T("RWVI_SHAPEID")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM tMed;
		ZeroMemory(&tMed, sizeof tMed);
		if (a_pDataObj && SUCCEEDED(a_pDataObj->GetData(&tFE, &tMed)))
		{
			CDocumentVectorImage* pDVIThis = reinterpret_cast<SDraggedShape*>(GlobalLock(tMed.hGlobal))->pThis;
			GlobalUnlock(tMed.hGlobal);
			ReleaseStgMedium(&tMed);
			if (pDVIThis == this)
			{
				if (a_pdwEffect)
				{
					*a_pdwEffect = DROPEFFECT_MOVE|DND_INSERTMARK_BEFORE;
				}
				if (a_ppFeedback)
				{
					*a_ppFeedback = NULL;
					*a_ppFeedback = new CMultiLanguageString(L"[0409]Move shapes[0405]Přesunout tvary");
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

STDMETHODIMP CDocumentVectorImage::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, IComparable* a_pItem, EDNDPoint a_eDNDPoint, LCID a_tLocaleID, ISharedState** a_ppNewSel)
{
	try
	{
		if (a_ppNewSel) *a_ppNewSel = NULL;
		ULONG nFiles = 0;
		if (a_pFileNames && SUCCEEDED(a_pFileNames->Size(&nFiles)) && nFiles)
		{
			return E_NOTIMPL;
			//CDocumentWriteLock cLock(this);
			//for (ULONG i = 0; i < nFiles; ++i)
			//{
			//	CComBSTR bstr;
			//	a_pFileNames->Get(i, &bstr);
			//	CStorageFilter pLoc(bstr);
			//	CComPtr<IComparable> pItem;
			//	LayerInsert(NULL, &CImageLayerCreatorStorage(pLoc), &pItem);
			//	if (pItem)
			//	{
			//		LPCOLESTR p1 = wcsrchr(bstr, L'\\');
			//		LPCOLESTR p2 = wcsrchr(bstr, L'//');
			//		if (p2 > p1) p1 = p2;
			//		if (p1 == NULL) p1 = bstr; else ++p1;
			//		p2 = wcsrchr(p1, L'.');
			//		if (p2 == NULL) p2 = p1+wcslen(p1);
			//		BSTR bstrName = SysAllocStringLen(p1, p2-p1);
			//		LayerNameSet(pItem, bstrName);
			//		SysFreeString(bstrName);
			//	}
			//}
			//return S_OK;
		}
		FORMATETC tFE = { RegisterClipboardFormat(_T("RWVI_SHAPEID")), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM tMed;
		ZeroMemory(&tMed, sizeof tMed);
		if (a_pDataObj && SUCCEEDED(a_pDataObj->GetData(&tFE, &tMed)))
		{
			SDraggedShape* pDL = reinterpret_cast<SDraggedShape*>(GlobalLock(tMed.hGlobal));
			CAutoVectorPtr<BYTE> pBuf(new BYTE[sizeof(SDraggedShape)+sizeof(ULONG)*LONG(pDL->nIDs-1)]);
			CopyMemory(pBuf.m_p, pDL, sizeof(SDraggedShape)+sizeof(ULONG)*LONG(pDL->nIDs-1));
			pDL = reinterpret_cast<SDraggedShape*>(pBuf.m_p);
			GlobalUnlock(tMed.hGlobal);
			ReleaseStgMedium(&tMed);
			if (pDL->pThis == this)
			{
				CDocumentWriteLock cLock(this);
				if (m_cElementOrder.empty())
					return E_FAIL;
				CComQIPtr<IStructuredItemVectorImageElement> pLI(a_pItem);
				ULONG nShapeID = 0;
				if (pLI) pLI->ID(&nShapeID);
				if (nShapeID == 0)
				{
					nShapeID = m_cElementOrder[0];
				}
				else //if (a_eDNDPoint == EDNDPLower)
				{
					CElementOrder::const_iterator iPos = m_cElementOrder.begin();
					for (CElementOrder::const_iterator i = iPos; i != m_cElementOrder.end(); ++i)
					{
						if (*i == nShapeID)
						{
							++i;
							if (i != m_cElementOrder.end())
								nShapeID = *i;
							else
								nShapeID = 0;
							break;
						}
					}
				}
				return ObjectsMove(pDL->nIDs, pDL->aIDs, nShapeID);
			}
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "Resampling.h"

STDMETHODIMP CDocumentVectorImage::Thumbnail(IComparable* a_pItem, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, ULONG* a_pTimestamp)
{
	try
	{
		CComQIPtr<IStructuredItemVectorImageElement> pVIE(a_pItem);
		if (pVIE == NULL)
			return E_RW_ITEMNOTFOUND;
		CDocumentReadLock cLock(this);
		ULONG nID = m_nNextID+1;
		pVIE->ID(&nID);
		CElements::const_iterator i = m_cElements.find(nID);
		if (i == m_cElements.end())
			return E_RW_ITEMNOTFOUND;
		RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		i->second.pTool->IsDirty(&rc, NULL, NULL);
		//if (rc.left < 0) rc.left = 0;
		//if (rc.top < 0) rc.top = 0;
		//if (rc.right > LONG(m_nSizeX)) rc.right = m_nSizeX;
		//if (rc.bottom > LONG(m_nSizeY)) rc.bottom = m_nSizeY;
		if (rc.left >= rc.right || rc.top >= rc.bottom)
		{
			ZeroMemory(a_pBGRAData, a_nSizeX*a_nSizeY*sizeof*a_pBGRAData);
			return S_OK;
		}
		CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[(rc.right-rc.left)*(rc.bottom-rc.top)]);
		ZeroMemory(cBuffer.m_p, (rc.right-rc.left)*(rc.bottom-rc.top)*sizeof*cBuffer.m_p);
		m_pToolWindow->SetROI(CImagePoint(rc.left, rc.top), CImageSize(rc.right-rc.left, rc.bottom-rc.top), reinterpret_cast<TPixelChannel*>(cBuffer.m_p), CPixelChannel(0, 0, 0, 0), m_tCanvas);
		i->second.pTool->GetImageTile(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, 2.2f, (rc.right-rc.left), cBuffer.m_p);
		LONG nSrcSizeX = rc.right-rc.left;
		LONG nSrcSizeY = rc.bottom-rc.top;
		LONG nDstSizeX;
		LONG nDstSizeY;
		DWORD* pDst;
		if (a_nSizeX*nSrcSizeY < a_nSizeY*nSrcSizeX)
		{
			nDstSizeX = a_nSizeX;
			nDstSizeY = (a_nSizeX*nSrcSizeY+(nSrcSizeX>>1))/nSrcSizeX;
			pDst = a_pBGRAData+((a_nSizeY-nDstSizeY)>>1)*a_nSizeX;
			ZeroMemory(a_pBGRAData, (pDst-a_pBGRAData)*4);
			ZeroMemory(pDst+a_nSizeX*(nDstSizeY), a_nSizeX*(a_nSizeY-nDstSizeY-((a_nSizeY-nDstSizeY)>>1))*4);
			if (a_prcBounds)
			{
				a_prcBounds->left = 0;
				a_prcBounds->top = (a_nSizeY-nDstSizeY)>>1;
				a_prcBounds->right = a_nSizeX;
				a_prcBounds->bottom = a_prcBounds->top+nDstSizeY;
			}
		}
		else
		{
			nDstSizeX = (a_nSizeY*nSrcSizeX+(nSrcSizeY>>1))/nSrcSizeY;
			nDstSizeY = a_nSizeY;
			pDst = a_pBGRAData+((a_nSizeX-nDstSizeX)>>1);
			for (ULONG y = 0; y < a_nSizeY; ++y)
			{
				DWORD* pLine = a_pBGRAData+a_nSizeX*y;
				ZeroMemory(pLine, (pDst-a_pBGRAData)*4);
				ZeroMemory(pLine+(pDst-a_pBGRAData)+nDstSizeX, (a_nSizeX-(pDst-a_pBGRAData)-nDstSizeX)*4);
			}
			if (a_prcBounds)
			{
				a_prcBounds->left = (a_nSizeX-nDstSizeX)>>1;
				a_prcBounds->top = 0;
				a_prcBounds->right = a_prcBounds->left+nDstSizeX;
				a_prcBounds->bottom = a_nSizeY;
			}
		}
		CResampling cRsmp(nDstSizeX, nDstSizeY, nSrcSizeX, nSrcSizeY, reinterpret_cast<TRasterImagePixel*>(pDst), a_nSizeX*4, cBuffer.m_p, nSrcSizeX*4);
		cRsmp.Nearest();
		if (a_pTimestamp)
			*a_pTimestamp = m_dwTimeStamp+i->second.dwTimeStamp;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

//#include <RenderIcon.h>

HICON CDocumentVectorImage::LayerIcon(ULONG a_nSize, IRTarget const* a_pTarget)
{
	static IRPathPoint const section0[] =
	{
		{221.463, 70.3208, -13.8933, -30.6439, 0, 0},
		{145.29, 6.35579, -61.5174, -15.795, 35.1255, 9.01869},
		{5.30408, 89.1435, -15.795, 61.5174, 15.795, -61.5174},
		{88.0918, 229.13, 0, 0, -61.5174, -15.795},
		{116.691, 117.743, 0, 0, 0, 0},
	};
	static IRPathPoint const section1[] =
	{
		{110.31, 250.472, 61.5174, 15.795, 0, 0},
		{250.296, 167.684, 6.77631, -26.3919, -15.795, 61.5174},
		{243.808, 91.2187, 0, 0, 10.4388, 23.0246},
		{138.909, 139.085, 0, 0, 0, 0},
	};
	static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	IRFill shadeFillMat(CGammaTables::BlendSRGBA(pSI->GetSRGBColor(ESMScheme2Color2), 0xff000000, 0.75f));
	IROutlinedFill shadeMat(&shadeFillMat, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(section0), section0, &shadeMat, a_pTarget);
	cRenderer(&canvas, itemsof(section1), section1, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	return cRenderer.get();
}

//STDMETHODIMP CDocumentVectorImage::Icon(ULONG a_nSize, HICON* a_phIcon)
//{
//	if (a_phIcon == NULL)
//		return E_POINTER;
//	static float const f = 1.0f/256.0f;
//	//static TPolyCoords const aVertices[] = {{f*24, f*232}, {f*128, f*24}, {f*232, f*232}};
//	//*a_phIcon = IconFromPolygon(itemsof(aVertices), aVertices, a_nSize, false);
//	static TPolyCoords const aVertices[] =
//	{
//		{f*112.78, f*29.16},
//		{-f*28.91, f*4.47}, {f*15.77, -f*27.31}, {f*41.4, f*78},
//		{-f*27.61, f*47.83}, {-f*47.83, -f*27.61}, {f*78, f*214.6},
//		{f*47.83, f*27.61}, {-f*27.61, f*47.83}, {f*214.6, f*178},
//		{0, 0}, {0, 0}, {f*128, f*128},
//		{0, 0}, {0, 0}, {f*112.78, f*29.16}
//	};
//	TIconPolySpec tPolySpec[1];
//	tPolySpec[0].nVertices = itemsof(aVertices);
//	tPolySpec[0].pVertices = aVertices;
//	tPolySpec[0].interior = GetIconFillColor();
//	tPolySpec[0].outline = agg::rgba8(0, 0, 0, 255);
//	*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
//	return S_OK;
//}
