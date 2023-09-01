// DocumentFactoryAnimation.cpp : Implementation of CDocumentFactoryAnimation

#include "stdafx.h"
#include "DocumentFactoryAnimation.h"

#include <MultiLanguageString.h>
#include "RWDocumentAnimationUtils.h"
#include <IconRenderer.h>


// CDocumentFactoryAnimation

STDMETHODIMP CDocumentFactoryAnimation::Priority(ULONG* a_pnPriority)
{
	try
	{
		*a_pnPriority = EDPAverage-1;
		CComPtr<IGlobalConfigManager> pMgr;
		RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
		if (pMgr == NULL) return S_OK;
		CComPtr<IConfig> pGlbCfg;
		pMgr->Config(__uuidof(GlobalConfigDefaultImageFormat), &pGlbCfg);
		if (pGlbCfg == NULL) return S_OK;
		CConfigValue cVal;
		pGlbCfg->ItemValueGet(CComBSTR(CFGID_DIF_BUILDER), &cVal);
		if (cVal.TypeGet() == ECVTGUID && IsEqualGUID(cVal, __uuidof(DocumentFactoryAnimation)))
			*a_pnPriority = EDPAverage;
		return S_OK;
	}
	catch (...)
	{
		return a_pnPriority == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::Init(BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDocumentAnimation>* p = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::AppendFrame(IAnimationFrameCreator* a_pCreator, float a_fDelay, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComPtr<IDocument> pDoc;
		a_pBase->DataBlockDoc(a_bstrPrefix, &pDoc);
		CComPtr<IDocumentAnimation> pAni;
		if (FAILED(pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAni))))
			return E_FAIL;
		CComPtr<IUnknown> pNew;
		if (FAILED(pAni->FrameIns(NULL, a_pCreator, &pNew)))
			return E_FAIL;
		if (FAILED(pAni->FrameSetTime(pNew, a_fDelay)))
			return E_FAIL;

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::SetLoopCount(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nCount)
{
	try
	{
		CComPtr<IDocument> pDoc;
		a_pBase->DataBlockDoc(a_bstrPrefix, &pDoc);
		CComPtr<IDocumentAnimation> pAni;
		if (pDoc == NULL || FAILED(pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAni))))
			return E_FAIL;
		pAni->LoopCountSet(a_nCount);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
{
	try
	{
		CComObject<CDocumentAnimation>* p = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
		return p->FrameIns(NULL, &CAnimationFrameCreatorRasterImage(a_pSize, a_pResolution, a_nChannels, a_aChannelDefaults, a_fGamma, a_pTile), NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CAnimationFrameCreatorVector :
	public IAnimationFrameCreator
{
public:
	CAnimationFrameCreatorVector(TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground) :
		m_pSize(a_pSize), m_pResolution(a_pResolution), m_aBackground(a_aBackground)
	{
	}

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IAnimationFrameCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IAnimationFrameCreator methods
public:
	STDMETHOD(Create)(IDocumentBuilder* a_pDefaultBuilder, BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		CComQIPtr<IDocumentFactoryVectorImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		return pFct->Create(a_bstrID, a_pBase, m_pSize, m_pResolution, m_aBackground);
	}

private:
	TImageSize const* m_pSize;
	TImageResolution const* m_pResolution;
	float const* m_aBackground;
};

STDMETHODIMP CDocumentFactoryAnimation::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground)
{
	try
	{
		CComObject<CDocumentAnimation>* p = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
		return p->FrameIns(NULL, &CAnimationFrameCreatorVector(a_pSize, a_pResolution, a_aBackground), NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::AddObject(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled)
{
	try
	{
		CComPtr<IDocumentFactoryVectorImage> pDFVI;
		RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryLayeredImage));
		if (pDFVI == NULL)
			return E_FAIL;
		CComBSTR bstrSubID(a_bstrPrefix);
		bstrSubID += L"F1;";
		return pDFVI->AddObject(bstrSubID, a_pBase, a_bstrName, a_bstrToolID, a_bstrParams, a_bstrStyleID, a_bstrStyleParams, a_pOutlineWidth, a_pOutlinePos, a_pOutlineJoins, a_pOutlineColor, a_pRasterizationMode, a_pCoordinatesMode, a_pEnabled);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CAnimationFrameCreatorEmpty :
	public IAnimationFrameCreator
{
public:

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IAnimationFrameCreator)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IAnimationFrameCreator methods
public:
	STDMETHOD(Create)(IDocumentBuilder* a_pDefaultBuilder, BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		CComQIPtr<IDocumentFactoryLayeredImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		return pFct->Create(a_bstrID, a_pBase);
	}
};

STDMETHODIMP CDocumentFactoryAnimation::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDocumentAnimation>* p = NULL;
		CComObject<CDocumentAnimation>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
		return p->FrameIns(NULL, &CAnimationFrameCreatorEmpty(), NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::AddLayer(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BYTE a_bTop, IImageLayerCreator* a_pCreator, BSTR a_bstrName, TImageLayer const* a_pProperties, IConfig* a_pOperation)
{
	try
	{
		CComQIPtr<IDocumentFactoryLayeredImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		CComBSTR bstr(a_bstrPrefix);
		bstr += L"F1;";
		return pFct->AddLayer(bstr, a_pBase, a_bTop, a_pCreator, a_bstrName, a_pProperties, a_pOperation);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::SetResolution(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageResolution const* a_pResolution)
{
	try
	{
		CComQIPtr<IDocumentFactoryLayeredImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		CComBSTR bstr(a_bstrPrefix);
		bstr += L"F1;";
		return pFct->SetResolution(bstr, a_pBase, a_pResolution);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::SetSize(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize)
{
	try
	{
		CComQIPtr<IDocumentFactoryLayeredImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		CComBSTR bstr(a_bstrPrefix);
		bstr += L"F1;";
		return pFct->SetSize(bstr, a_pBase, a_pSize);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::RearrangeLayers(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nLayers, TImagePoint const* a_aOffsets)
{
	try
	{
		CComQIPtr<IDocumentFactoryLayeredImage> pFct;
		RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
		CComBSTR bstr(a_bstrPrefix);
		bstr += L"F1;";
		return pFct->RearrangeLayers(bstr, a_pBase, a_nLayers, a_aOffsets);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::AddBlock(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		CComPtr<IImageMetaData> pIMD;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD == NULL)
			return E_RW_ITEMNOTFOUND;
		return pIMD->SetBlock(a_bstrID, a_nSize, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		//pSI->GetLayers(ESIPicture, cRenderer, IRTarget(0.8f));
		static IRPathPoint const film[] =
		{
			{60, 60, 0, 0, 0, 0},
			{196, 60, 0, 0, 0, 0},
			{196, 40, 0, -10.8758, 0, 0},
			{216, 20, 0, 0, -10.8758, 0},
			{236, 20, 10.8758, 0, 0, 0},
			{256, 40, 0, 0, 0, -10.8758},
			{256, 216, 0, 10.8758, 0, 0},
			{236, 236, 0, 0, 10.8758, 0},
			{216, 236, -10.8758, 0, 0, 0},
			{196, 216, 0, 0, 0, 10.8758},
			{196, 196, 0, 0, 0, 0},
			{60, 196, 0, 0, 0, 0},
			{60, 216, 0, 10.8758, 0, 0},
			{40, 236, 0, 0, 10.8758, 0},
			{20, 236, -10.8758, 0, 0, 0},
			{0, 216, 0, 0, 0, 10.8758},
			{0, 40, 0, -10.8758, 0, 0},
			{20, 20, 0, 0, -10.8758, 0},
			{40, 20, 10.8758, 0, 0, 0},
			{60, 40, 0, 0, 0, -10.8758},
		};
		static IRPolyPoint const hole0[] = { {20, 40}, {40, 40}, {40, 60}, {20, 60} };
		static IRPolyPoint const hole1[] = { {20, 79}, {40, 79}, {40, 99}, {20, 99} };
		static IRPolyPoint const hole2[] = { {20, 118}, {40, 118}, {40, 138}, {20, 138} };
		static IRPolyPoint const hole3[] = { {20, 157}, {40, 157}, {40, 177}, {20, 177} };
		static IRPolyPoint const hole4[] = { {20, 196}, {40, 196}, {40, 216}, {20, 216} };
		static IRPolyPoint const hole5[] = { {216, 40}, {236, 40}, {236, 60}, {216, 60} };
		static IRPolyPoint const hole6[] = { {216, 79}, {236, 79}, {236, 99}, {216, 99} };
		static IRPolyPoint const hole7[] = { {216, 118}, {236, 118}, {236, 138}, {216, 138} };
		static IRPolyPoint const hole8[] = { {216, 157}, {236, 157}, {236, 177}, {216, 177} };
		static IRPolyPoint const hole9[] = { {216, 196}, {236, 196}, {236, 216}, {216, 216} };
		static IRPolygon const holes[] =
		{
			{itemsof(hole0), hole0}, {itemsof(hole1), hole1}, {itemsof(hole2), hole2}, {itemsof(hole3), hole3}, {itemsof(hole4), hole4},
			{itemsof(hole5), hole5}, {itemsof(hole6), hole6}, {itemsof(hole7), hole7}, {itemsof(hole8), hole8}, {itemsof(hole9), hole9},
		};
		static IRGridItem gridX[] = { {0, 0}, {0, 60}, {0, 196}, {0, 256} };
		static IRGridItem gridY[] = { {0, 20}, {0, 60}, {0, 196}, {0, 236} };
		static IRCanvas canvas = {0, 20, 256, 236, itemsof(gridX), itemsof(gridY), gridX, gridY};
		cRenderer(&canvas, itemsof(film), film, pSI->GetMaterial(ESMContrast));
		cRenderer(&canvas, itemsof(holes), holes, pSI->GetMaterial(ESMBackground, true));
		pSI->GetLayers(ESIPicture, cRenderer, IRTarget(0.6f));
		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryAnimation::GetIcon(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
{
	if (IsEqualGUID(a_tIconID, CLSID_DocumentFactoryAnimation))
		return Icon(a_nSize, a_phIcon);
	return E_RW_ITEMNOTFOUND;
}

STDMETHODIMP CDocumentFactoryAnimation::GetThumbnail(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds))
{
	try
	{
		CComPtr<IDocumentAnimation> pDocAni;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDocAni));
		if (pDocAni == nullptr)
			return E_RW_UNKNOWNINPUTFORMAT;

		CComPtr<IEnumUnknowns> pFrames;
		pDocAni->FramesEnum(&pFrames);
		CComPtr<IUnknown> pFrm;
		if (pFrames) pFrames->Get(0, IID_IUnknown, reinterpret_cast<void**>(&pFrm));
		if (pFrm == NULL) return E_FAIL;
		ULONG nFrames = 0;
		pFrames->Size(&nFrames);
		CComPtr<IDocument> pFrameDoc;
		pDocAni->FrameGetDoc(pFrm, &pFrameDoc);
		CComPtr<IDocumentImage> pFrameImg;
		if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
		if (pFrameImg)
		{
			CBGRABuffer cBuffer;
			if (!cBuffer.Init(pFrameImg, false))
				return E_FAIL;
			HRESULT hRes = fnRescaleImage(cBuffer.tSize.nX, cBuffer.tSize.nY, reinterpret_cast<DWORD const*>(cBuffer.aData.m_p), true, a_nSizeX, a_nSizeY, a_pBGRAData, a_prcBounds);
			if (a_pbstrInfo)
			{
				wchar_t sz[256] = L"";
				CComBSTR bstrTempl;
				CMultiLanguageString::GetLocalized(L"[0409]Dimensions: %ix%i pixels[0405]Rozměry: %ix%i pixelů", a_tLocaleID, &bstrTempl);
				swprintf(sz, bstrTempl, cBuffer.tSize.nX, cBuffer.tSize.nY);
				wcscat(sz, L"\n");
				bstrTempl.Empty();
				CMultiLanguageString::GetLocalized(L"[0409]Frames: %i[0405]Snímků: %i", a_tLocaleID, &bstrTempl);
				swprintf(sz + wcslen(sz), 128, bstrTempl, nFrames);//cFmt->atDims[0].nItems, cFmt->atDims[1].nItems);
				*a_pbstrInfo = SysAllocString(sz);
			}
			return hRes;
		}
		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

