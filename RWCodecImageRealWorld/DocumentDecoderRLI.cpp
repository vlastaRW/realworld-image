// DocumentDecoderRLI.cpp : Implementation of CDocumentDecoderRLI

#include "stdafx.h"
#include "DocumentDecoderRLI.h"

#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>
#include <MultiLanguageString.h>

struct TImageLayerOld
{
	BYTE bVisible;
	float fOpacity;
	ELayerBlend eBlend;
	float fPositionX;
	float fPositionY;
	float fPositionZ;
	float fPositionW;
	ULONG nMarginXPrev;
	ULONG nMarginXNext;
	ULONG nMarginYPrev;
	ULONG nMarginYNext;
	ULONG nMarginZPrev;
	ULONG nMarginZNext;
	ULONG nMarginWPrev;
	ULONG nMarginWNext;
};


// CDocumentDecoderRLI

STDMETHODIMP CDocumentDecoderRLI::Priority(ULONG* a_pnPriority)
{
	try
	{
		*a_pnPriority = EDPAverage;
		return S_OK;
	}
	catch (...)
	{
		return a_pnPriority ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderRLI::DocumentType(IDocumentType** a_ppDocumentType)
{
	try
	{
		*a_ppDocumentType = NULL;
		*a_ppDocumentType = CDocumentTypeCreatorRLI::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderRLI::IsCompatible(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
{
	try
	{
		for (ULONG i = 0; i < a_nBuilders; ++i)
		{
			CComPtr<IDocumentFactoryLayeredImage> pI;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryLayeredImage), reinterpret_cast<void**>(&pI));
			if (pI) return S_OK;
			CComPtr<IDocumentFactoryAnimation> pA;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryAnimation), reinterpret_cast<void**>(&pA));
			if (pA) return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class CImageLayerCreatorDataBlock :
	public IImageLayerCreator
{
public:
	CImageLayerCreatorDataBlock(BYTE const* a_pData, ULONG a_nSize, IStorageFilter* a_pLocation = NULL, TImageSize* a_pSize = NULL) :
		m_pData(a_pData), m_nSize(a_nSize), m_pLocation(a_pLocation), m_pSize(a_pSize)
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
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		HRESULT hRes = pIM->DocumentCreateDataEx(pBuilders, m_nSize, m_pData, m_pLocation, a_bstrID, a_pBase, NULL, NULL, NULL);
		if (m_pSize)
		{ // workaround for old .rli images
			CComPtr<IDocumentImage> pImg;
			a_pBase->DataBlockGet(a_bstrID, __uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
			if (pImg) pImg->CanvasGet(m_pSize, NULL, NULL, NULL, NULL);
		}
		return hRes;
	}

private:
	BYTE const* m_pData;
	ULONG m_nSize;
	IStorageFilter* m_pLocation;
	TImageSize* m_pSize;
};

class CAnimationFrameCreatorRLI :
	public IAnimationFrameCreator
{
public:
	CAnimationFrameCreatorRLI(BYTE const*& a_pData, BYTE const* a_pEnd, IStorageFilter* a_pLocation, TImageSize const* a_pSize, TImageResolution const* a_pRes, float a_fGamma, TPixelChannel a_tBackground) :
		pData(a_pData), pEnd(a_pEnd), m_pLocation(a_pLocation), m_pSize(a_pSize), m_pRes(a_pRes), m_fGamma(a_fGamma), m_tBackground(a_tBackground)
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
		CComQIPtr<IDocumentFactoryLayeredImage> pLIF(a_pDefaultBuilder);
		pLIF->Create(a_bstrID, a_pBase);
		bool bSizeSet = false;
		TImageSize tSize = {0, 0};
		std::vector<std::pair<TImageSize, TImageLayerOld> > aOldPos;
		ULONG nLayers = 0;
		while (pEnd-pData > 8)
		{
			if (MARK_LAYER == *reinterpret_cast<DWORD const*>(pData))
			{
				BYTE const* const pLayerBegin = pData;
				BYTE const* const pLayerEnd = pData+8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				if (pLayerEnd > pEnd)
					return E_FAIL;
				pData += 8;
				CComBSTR bstrName;
				TImageLayer tLayer;
				ZeroMemory(&tLayer, sizeof tLayer);
				tLayer.bVisible = 1;
				tLayer.fOpacity = 1.0f;
				tLayer.eBlend = EBEAlphaBlend;
				TImageLayerOld tLayerOld;
				ZeroMemory(&tLayerOld, sizeof tLayerOld);
				tLayerOld.bVisible = 1;
				tLayerOld.fOpacity = 1.0f;
				tLayerOld.eBlend = EBEAlphaBlend;
				bool bNew = false;
				bool bOld = false;
				CComPtr<IConfigInMemory> pEffect;
				BYTE const* pDoc = NULL;
				ULONG nDocLen = 0;
				while (pLayerEnd > pData)
				{
					ULONG nLen = *reinterpret_cast<DWORD const*>(pData+4);
					switch (*reinterpret_cast<DWORD const*>(pData))
					{
					case MARK_NAME:
						bstrName.Attach(SysAllocStringLen(reinterpret_cast<wchar_t const*>(pData+8), nLen>>1));
						break;
					case MARK_BLENDING:
						memcpy(&tLayer, pData+8, sizeof tLayer);
						bNew = true;
						break;
					case MARK_PROPERTIES:
						memcpy(&tLayerOld, pData+8, sizeof tLayerOld);
						tLayer.bVisible = tLayerOld.bVisible;
						tLayer.fOpacity = tLayerOld.fOpacity;
						tLayer.eBlend = tLayerOld.eBlend;
						bOld = true;
						break;
					case MARK_EFFECT:
						if (pEffect == NULL)
						{
							RWCoCreateInstance(pEffect, __uuidof(ConfigInMemory));
							pEffect->DataBlockSet(nLen, pData+8);
						}
						break;
					case MARK_DOCUMENT:
						{
							pDoc = pData+8;
							nDocLen = nLen;
						}
						break;
					}
					pData += 8+((nLen+3)&~3);
				}
				pData = pLayerEnd;
				TImageSize tLaySize = {0, 0};
				if (SUCCEEDED(pLIF->AddLayer(a_bstrID, a_pBase, 0, &CImageLayerCreatorDataBlock(pDoc, nDocLen, m_pLocation, !bSizeSet ? &tLaySize : NULL), bstrName, &tLayer, pEffect)))
				{
					++nLayers;
					tLaySize.nX += tLayerOld.nMarginXPrev+tLayerOld.nMarginXNext;
					tLaySize.nY += tLayerOld.nMarginYPrev+tLayerOld.nMarginYNext;
					if (tSize.nX < tLaySize.nX) tSize.nX = tLaySize.nX;
					if (tSize.nY < tLaySize.nY) tSize.nY = tLaySize.nY;
					if (!bNew && bOld)
						aOldPos.push_back(std::make_pair(tLaySize, tLayerOld));
				}
				continue;
			}
			else
				break;

			//// skip unknown block
			//pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
		}
		if (!bSizeSet)
			pLIF->SetSize(a_bstrID, a_pBase, &tSize);
		if (nLayers && aOldPos.size() == nLayers)
		{
			CAutoVectorPtr<TImagePoint> aOffsets(new TImagePoint[nLayers]);
			bool bChange = false;
			for (ULONG i = 0; i < nLayers; ++i)
			{
				ULONG nExtraX = tSize.nX-aOldPos[i].first.nX;
				ULONG nExtraY = tSize.nY-aOldPos[i].first.nY;
				float fPosX = aOldPos[i].second.fPositionX < -1.0f ? 0.0f : (aOldPos[i].second.fPositionX > 1.0f ? 1.0f : (aOldPos[i].second.fPositionX*0.5f+0.5f));
				float fPosY = aOldPos[i].second.fPositionY < -1.0f ? 0.0f : (aOldPos[i].second.fPositionY > 1.0f ? 1.0f : (aOldPos[i].second.fPositionY*0.5f+0.5f));
				aOffsets[nLayers-1-i].nX = int(nExtraX*fPosX+0.5f)+aOldPos[i].second.nMarginXPrev;
				aOffsets[nLayers-1-i].nY = int(nExtraY*fPosY+0.5f)+aOldPos[i].second.nMarginYPrev;
				if (aOffsets[nLayers-1-i].nX || aOffsets[nLayers-1-i].nY)
					bChange = true;
			}
			if (bChange)
				pLIF->RearrangeLayers(a_bstrID, a_pBase, nLayers, aOffsets);
		}
		return S_OK;
	}

private:
	BYTE const*& pData;
	BYTE const* const pEnd;
	IStorageFilter* m_pLocation;
	TImageSize const* m_pSize;
	TImageResolution const* m_pRes;
	float m_fGamma;
	TPixelChannel m_tBackground;
};

STDMETHODIMP CDocumentDecoderRLI::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentFactoryAnimation> pAni;
		CComPtr<IDocumentFactoryLayeredImage> pLIF;
		for (ULONG i = 0; i < a_nBuilders && pLIF == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pLIF);
		for (ULONG i = 0; i < a_nBuilders && pAni == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pAni);
		if (pAni == NULL && pLIF == NULL)
			return E_FAIL;

		if (a_nLen <= sizeof(RLI_HEADER) || a_pData == NULL || memcmp(RLI_HEADER, a_pData, sizeof(RLI_HEADER)))
			return E_RW_UNKNOWNINPUTFORMAT;

		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));

		CComQIPtr<IDocumentFactoryMetaData> pMD;

		BYTE const* pFrameInfo = NULL;
		TImageResolution tResolution = {100, 254, 100, 254};
		TImageSize tSize = {256, 256};
		float fGamma = 0.0f;
		TPixelChannel tDefault;
		tDefault.n = 0;
		bool bDocCreated = false;
		bool bFrameRead = false;
		bool bResolutionSet = false;
		bool bSizeSet = false;
		ULONG nLoopCount = 0;

		BYTE const* const pBegin = a_pData;
		BYTE const* const pEnd = a_pData+a_nLen;
		BYTE const* pData = a_pData+sizeof(RLI_HEADER);
		while (pEnd-pData > 8)
		{
			if (MARK_FRAMEINFO == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= 4)
			{
				pFrameInfo = pData+8;
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_RESOLUTION == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= 4*2*4)
			{
				tResolution.nNumeratorX = reinterpret_cast<ULONG const*>(pData+8)[0];
				tResolution.nDenominatorX = reinterpret_cast<ULONG const*>(pData+8)[1];
				tResolution.nNumeratorY = reinterpret_cast<ULONG const*>(pData+8)[2];
				tResolution.nDenominatorY = reinterpret_cast<ULONG const*>(pData+8)[3];
				bResolutionSet = true;
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_CANVASINFO == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= (sizeof tSize + sizeof tResolution))
			{
				tSize = *reinterpret_cast<TImageSize const*>(pData+8);
				bSizeSet = true;
				tResolution = *reinterpret_cast<TImageResolution const*>(pData+16);
				bResolutionSet = true;
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_CHANNELS == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= 12)
			{
				fGamma = *reinterpret_cast<float const*>(pData+8);
				if (EICIRGBA == *reinterpret_cast<ULONG const*>(pData+12))
					tDefault.n = *reinterpret_cast<ULONG const*>(pData+16);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}

			if (!bDocCreated)
			{
				if (pAni && pLIF)
				{
					if (pFrameInfo)
						pLIF = NULL;
					else
						pAni = NULL;
				}
				if (pAni)
				{
					pMD = pAni;
					pAni->Init(a_bstrPrefix, a_pBase);
				}
				else
				{
					pMD = pLIF;
				}
				bDocCreated = true;
			}
			if (MARK_LAYER == *reinterpret_cast<DWORD const*>(pData))
			{
				CAnimationFrameCreatorRLI cCreator(pData, pEnd, a_pLocation, bSizeSet ? &tSize : NULL, bResolutionSet ? &tResolution : NULL, fGamma, tDefault);
				if (pAni)
				{
					pAni->AppendFrame(&cCreator, *reinterpret_cast<float const*>(pFrameInfo), a_bstrPrefix, a_pBase);
				}
				else if (!bFrameRead)
				{
					cCreator.Create(CComQIPtr<IDocumentBuilder>(pLIF), a_bstrPrefix, a_pBase);
					bFrameRead = true;
				}
				else
				{
					// skip other frames...
					pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				}
				tDefault.n = 0;
				fGamma = 0.0f;
				bSizeSet = false;
				continue;
			}
			if (MARK_METADATA == *reinterpret_cast<DWORD const*>(pData))
			{
				BYTE const* pMetaBegin = pData+8;
				BYTE const* const pRealEnd = pData+8+*reinterpret_cast<DWORD const*>(pData+4);
				BYTE const* const pMetaEnd = pData+8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				if (pRealEnd > pEnd) break; // or throw?
				CComBSTR bstr(reinterpret_cast<wchar_t const*>(pMetaBegin));
				pMetaBegin += (bstr.Length()+1)*sizeof(wchar_t);
				if (pMD) pMD->AddBlock(a_bstrPrefix, a_pBase, bstr, pRealEnd-pMetaBegin, pMetaBegin);
				pData = pMetaEnd;
				continue;
			}
			if (MARK_LOOPCOUNT == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= 4)
			{
				nLoopCount = *reinterpret_cast<DWORD const*>(pData+8);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
			}

			// skip unknown block
			pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
		}
		if (bResolutionSet && bDocCreated)
			if (pAni)
				pAni->SetResolution(a_bstrPrefix, a_pBase, &tResolution);
			else
				pLIF->SetResolution(a_bstrPrefix, a_pBase, &tResolution);
		if (pAni && bDocCreated && nLoopCount)
			pAni->SetLoopCount(a_bstrPrefix, a_pBase, nLoopCount);
		if (a_pEncoderID) *a_pEncoderID = CLSID_DocumentEncoderRLI;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

