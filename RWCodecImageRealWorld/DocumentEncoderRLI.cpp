// DocumentEncoderRLI.cpp : Implementation of CDocumentEncoderRLI

#include "stdafx.h"
#include "DocumentEncoderRLI.h"

#include <RWImaging.h>
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimation.h>
#include <ReturnedData.h>
#include <MultiLanguageString.h>
#include <math.h>


// CDocumentEncoderRLI

STDMETHODIMP CDocumentEncoderRLI::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorRLI::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentEncoderRLI::DefaultConfig(IConfig** UNREF(a_ppDefCfg))
{
	return E_NOTIMPL;
}

STDMETHODIMP CDocumentEncoderRLI::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA ENCFEAT_IMAGE_LAYER ENCFEAT_IMAGE_LAYER_EFFECT ENCFEAT_IMAGE_META ENCFEAT_ANIMATION ENCFEAT_IMAGE_CANVAS ENCFEAT_IMAGE_LAYER_SPECIAL);
		//CComPtr<IDocumentImage> pDocImg;
		//a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		//if (pDocImg)
		//	return S_OK;
		CComPtr<IDocumentLayeredImage> pDocLay;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDocLay));
		if (pDocLay)
			return S_OK;
		CComPtr<IDocumentAnimation> pDocAni;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDocAni));
		return pDocAni ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static BYTE const DWORD_PADDING[4] = {0, 0, 0, 0};

HRESULT SaveLayers(IDocumentLayeredImage* pLI, IReturnedData* a_pDst, IInputManager* pIM, IStorageFilter* a_pLocation, ITaskControl* a_pTaskControl)
{
	CComPtr<IEnumUnknowns> pLayers;
	pLI->LayersEnum(NULL, &pLayers);
	ULONG nLayers = 0;
	if (pLayers) pLayers->Size(&nLayers);
	CAutoVectorPtr<CComPtr<IComparable> > aLayerIDs(nLayers ? new CComPtr<IComparable>[nLayers] : NULL);
	for (ULONG i = 0; i < nLayers; ++i)
		pLayers->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&(aLayerIDs[nLayers-1-i])));

	//size_t const nLayers = m_cLayers.size();
	//if (FAILED(pDst->Write(sizeof(nLayers), reinterpret_cast<BYTE const*>(&nLayers))))
	//	return E_FAIL;
	CComBSTR bstrPNGEncID(L"{1CE9642E-BA8C-4110-87AC-DF39D57C9640}");
	CComBSTR bstrRVIEncID(L"{51C87837-B028-4252-A3B3-940F80181770}");
	BSTR aIDs[] = {bstrPNGEncID, bstrRVIEncID};
	static float const aWeights[] = {1.0f, 1.0f};
	for (ULONG i = 0; i < nLayers; ++i)
	{
		IComparable* pItem = aLayerIDs[i];

		ULONG nDataLen = 0;
		CReturnedData cDst;
		CComPtr<ISubDocumentID> pSDID;
		pLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
		CComPtr<IDocument> pSubDoc;
		if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
		if (pSubDoc)
		{
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pCfg;
			pIM->FindBestEncoderEx(pSubDoc, 2, aIDs, aWeights, &tID, &pCfg);
			CComPtr<IDocumentEncoder> pEnc;
			if (!IsEqualGUID(tID, GUID_NULL) && SUCCEEDED(RWCoCreateInstance(pEnc, tID)))
				pEnc->Serialize(pSubDoc, pCfg, &cDst, a_pLocation, a_pTaskControl);
			nDataLen = cDst.size();
		}

		TImageLayer tProps;
		ZeroMemory(&tProps, sizeof tProps);
		tProps.fOpacity = 1.0f;
		pLI->LayerPropsGet(pItem, &tProps.eBlend, &tProps.bVisible);

		ULONG nEffectLen = 0;
		CAutoVectorPtr<BYTE> pEffectData;
		CComPtr<IConfig> pEffect;
		pLI->LayerEffectGet(pItem, &pEffect, &tProps.fOpacity);
		CConfigValue cEffectID;
		if (pEffect)
		{
			pEffect->ItemValueGet(CComBSTR(L"Effect"), &cEffectID);
			if (cEffectID.TypeGet() == ECVTGUID && !IsEqualGUID(cEffectID, __uuidof(DocumentOperationNULL)))
			{
				CComPtr<IConfigInMemory> pMemCfg;
				RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
				CopyConfigValues(pMemCfg, pEffect);
				pMemCfg->DataBlockGetSize(&nEffectLen);
				pEffectData.Allocate(nEffectLen);
				pMemCfg->DataBlockGet(nEffectLen, pEffectData);
			}
		}

		CComBSTR bstrName;
		pLI->LayerNameGet(pItem, &bstrName);
		ULONG nLayerLen = 0;
		if (bstrName.Length())
		{
			nLayerLen += 4+4+sizeof(wchar_t)*bstrName.Length();
			nLayerLen = (nLayerLen+3)&~3; // DWORD alignment
		}
		struct TImageLayerRemainder
		{
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
		bool const bWriteProps = tProps.bVisible == 0 || tProps.fOpacity != 1.0f || tProps.eBlend != EBEAlphaBlend;
		nLayerLen += (bWriteProps ? (4+4+sizeof tProps)*2+sizeof(TImageLayerRemainder) : 0); // layer properties
		nLayerLen = (nLayerLen+3)&~3; // DWORD alignment
		if (nEffectLen)
		{
			nLayerLen += 4+4+nEffectLen;
			nLayerLen = (nLayerLen+3)&~3; // DWORD alignment
		}
		if (nDataLen)
		{
			nLayerLen += 4+4+nDataLen;
			nLayerLen = (nLayerLen+3)&~3; // DWORD alignment
		}

		// write the layer
		if (FAILED(a_pDst->Write(sizeof(MARK_LAYER), reinterpret_cast<BYTE const*>(&MARK_LAYER))))
			return E_FAIL;
		if (FAILED(a_pDst->Write(sizeof(nLayerLen), reinterpret_cast<BYTE const*>(&nLayerLen))))
			return E_FAIL;

		// write layer name
		if (bstrName.Length())
		{
			ULONG nNameLen = sizeof(wchar_t)*bstrName.Length();
			if (FAILED(a_pDst->Write(sizeof(MARK_NAME), reinterpret_cast<BYTE const*>(&MARK_NAME))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof(nNameLen), reinterpret_cast<BYTE const*>(&nNameLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nNameLen, reinterpret_cast<BYTE const*>(bstrName.m_str))))
				return E_FAIL;
			if ((nNameLen&3) && FAILED(a_pDst->Write(4-(nNameLen&3), DWORD_PADDING)))
				return E_FAIL;
		}

		if (bWriteProps)
		{
			// write layer blending properties
			if (FAILED(a_pDst->Write(sizeof(MARK_BLENDING), reinterpret_cast<BYTE const*>(&MARK_BLENDING))))
				return E_FAIL;
			ULONG nPropsLen = sizeof tProps;
			if (FAILED(a_pDst->Write(sizeof(nPropsLen), reinterpret_cast<BYTE const*>(&nPropsLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nPropsLen, reinterpret_cast<BYTE const*>(&tProps))))
				return E_FAIL;
			if ((nPropsLen&3) && FAILED(a_pDst->Write(4-(nPropsLen&3), DWORD_PADDING)))
				return E_FAIL;

			// compatibility with older versions - remove when latest versions of all apps support the new format
			static TImageLayerRemainder const s_tRemainder =
			{
				-1.0f, -1.0f, 0.0f, 0.0f,
				0, 0, 0, 0, 0, 0, 0, 0
			};
			if (FAILED(a_pDst->Write(sizeof(MARK_PROPERTIES), reinterpret_cast<BYTE const*>(&MARK_PROPERTIES))))
				return E_FAIL;
			nPropsLen = sizeof tProps + sizeof s_tRemainder;
			if (FAILED(a_pDst->Write(sizeof(nPropsLen), reinterpret_cast<BYTE const*>(&nPropsLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof tProps, reinterpret_cast<BYTE const*>(&tProps))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof s_tRemainder, reinterpret_cast<BYTE const*>(&s_tRemainder))))
				return E_FAIL;
			if ((nPropsLen&3) && FAILED(a_pDst->Write(4-(nPropsLen&3), DWORD_PADDING)))
				return E_FAIL;
		}

		// write layer effect
		if (nEffectLen)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_EFFECT), reinterpret_cast<BYTE const*>(&MARK_EFFECT))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof(nEffectLen), reinterpret_cast<BYTE const*>(&nEffectLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nEffectLen, pEffectData.m_p)))
				return E_FAIL;
			if ((nEffectLen&3) && FAILED(a_pDst->Write(4-(nEffectLen&3), DWORD_PADDING)))
				return E_FAIL;
		}

		// write layer data
		if (nDataLen)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_DOCUMENT), reinterpret_cast<BYTE const*>(&MARK_DOCUMENT))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof(nDataLen), reinterpret_cast<BYTE const*>(&nDataLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nDataLen, cDst.begin())))
				return E_FAIL;
			if ((nDataLen&3) && FAILED(a_pDst->Write(4-(nDataLen&3), DWORD_PADDING)))
				return E_FAIL;
		}
	}
	return S_OK;
}

HRESULT SaveCanvasAndChannels(IDocument* a_pDoc, IReturnedData* a_pDst)
{
	CComPtr<IDocumentImage> pI;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
	TImageSize tSize = {0, 0};
	TImageResolution tResolution = {100, 254, 100, 254};
	if (pI) pI->CanvasGet(&tSize, &tResolution, NULL, NULL, NULL);
	if (tSize.nX || tSize.nY ||
		tResolution.nNumeratorX != 100 || tResolution.nNumeratorY != 100 ||
		tResolution.nDenominatorX != 254 || tResolution.nDenominatorY != 254)
	{
		if (FAILED(a_pDst->Write(sizeof(MARK_CANVASINFO), reinterpret_cast<BYTE const*>(&MARK_CANVASINFO))))
			return E_FAIL;
		ULONG nLen = sizeof tSize + sizeof tResolution;
		if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
			return E_FAIL;
		if (FAILED(a_pDst->Write(sizeof tSize, reinterpret_cast<BYTE const*>(&tSize))))
			return E_FAIL;
		if (FAILED(a_pDst->Write(sizeof tResolution, reinterpret_cast<BYTE const*>(&tResolution))))
			return E_FAIL;
	}

	CPixelChannel cDefault(0, 0, 0, 0);
	float fGamma = 2.2f;
	if (pI) pI->ChannelsGet(NULL, &fGamma, CImageChannelDefaultGetter(EICIRGBA, &cDefault));
	if (cDefault.n || fabsf(fGamma-2.2f) < 1e-3f)
	{
		if (FAILED(a_pDst->Write(sizeof(MARK_CHANNELS), reinterpret_cast<BYTE const*>(&MARK_CHANNELS))))
			return E_FAIL;
		ULONG nLen = 12;
		ULONG const aData[] = {EICIRGBA, cDefault.n};
		if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
			return E_FAIL;
		if (FAILED(a_pDst->Write(4, reinterpret_cast<BYTE const*>(&fGamma))))
			return E_FAIL;
		if (FAILED(a_pDst->Write(8, reinterpret_cast<BYTE const*>(aData))))
			return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CDocumentEncoderRLI::Serialize(IDocument* a_pDoc, IConfig* UNREF(a_pCfg), IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl)
{
	try
	{
		CReadLock<IDocument> cLock(a_pDoc);

		CComPtr<IDocumentAnimation> pA;
		CComPtr<IDocumentLayeredImage> pLI;
		//CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
		{
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
			//if (pLI == NULL)
			//	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		}
		if (pA == NULL && pLI == NULL)
			return E_NOINTERFACE;

		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));

		if (FAILED(a_pDst->Write(sizeof(RLI_HEADER), RLI_HEADER)))
			return E_FAIL;

		if (pA)
		{
			CComPtr<IEnumUnknowns> pFrames;
			pA->FramesEnum(&pFrames);
			ULONG nFrames = 0;
			if (pFrames) pFrames->Size(&nFrames);
			if (nFrames == 0)
				return E_FAIL; // no frames...
			if (nFrames == 1)
			{
				// single-frame animation - open as layered image
				CComPtr<IUnknown> pFrame;
				pFrames->Get(0, &pFrame);
				CComPtr<IDocument> pDoc;
				pA->FrameGetDoc(pFrame, &pDoc);
				CComPtr<IDocumentLayeredImage> pDLI;
				pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
				if (pDLI == NULL)
					return E_FAIL;
				if (FAILED(SaveCanvasAndChannels(pDoc, a_pDst)))
					return E_FAIL;
				if (FAILED(SaveLayers(pDLI, a_pDst, pIM, a_pLocation, a_pControl)))
					return E_FAIL;
			}
			else
			{
				ULONG nLoopCount = 0;
				pA->LoopCountGet(&nLoopCount);

				for (ULONG i = 0; i < nFrames; ++i)
				{
					CComPtr<IUnknown> pFrame;
					pFrames->Get(i, &pFrame);
					float fTime = 0.0f;
					pA->FrameGetTime(pFrame, &fTime);
					if (FAILED(a_pDst->Write(sizeof(MARK_FRAMEINFO), reinterpret_cast<BYTE const*>(&MARK_FRAMEINFO))))
						return E_FAIL;
					ULONG nLen = sizeof fTime;
					if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
						return E_FAIL;
					if (FAILED(a_pDst->Write(sizeof(fTime), reinterpret_cast<BYTE const*>(&fTime))))
						return E_FAIL;
					CComPtr<IDocument> pDoc;
					pA->FrameGetDoc(pFrame, &pDoc);
					CComPtr<IDocumentLayeredImage> pDLI;
					pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
					if (pDLI == NULL)
						return E_FAIL;
					if (FAILED(SaveCanvasAndChannels(pDoc, a_pDst)))
						return E_FAIL;
					if (FAILED(SaveLayers(pDLI, a_pDst, pIM, a_pLocation, a_pControl)))
						return E_FAIL;
				}
				if (nLoopCount)
				{
					DWORD const b[] = {MARK_LOOPCOUNT, 4, nLoopCount};
					if (FAILED(a_pDst->Write(sizeof(b), reinterpret_cast<BYTE const*>(b))))
						return E_FAIL;
				}
			}
		}
		else
		{
			if (FAILED(SaveCanvasAndChannels(a_pDoc, a_pDst)))
				return E_FAIL;
			if (FAILED(SaveLayers(pLI, a_pDst, pIM, a_pLocation, a_pControl)))
				return E_FAIL;
		}

		CComPtr<IImageMetaData> pMetaData;
		a_pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pMetaData));
		if (pMetaData)
		{
			CComPtr<IEnumStrings> pMetaIDs;
			pMetaData->EnumIDs(&pMetaIDs);
			ULONG nMetaIDs = 0;
			if (pMetaIDs) pMetaIDs->Size(&nMetaIDs);
			for (ULONG i = 0; i < nMetaIDs; ++i)
			{
				CComBSTR bstrMetaID;
				pMetaIDs->Get(i, &bstrMetaID);
				ULONG nMetaLen = 0;
				pMetaData->GetBlockSize(bstrMetaID, &nMetaLen);
				if (nMetaLen == 0)
					continue;
				CAutoVectorPtr<BYTE> cMetaData(new BYTE[nMetaLen]);
				pMetaData->GetBlock(bstrMetaID, nMetaLen, cMetaData);
				if (FAILED(a_pDst->Write(sizeof(MARK_METADATA), reinterpret_cast<BYTE const*>(&MARK_METADATA))))
					return E_FAIL;
				ULONG nLen = nMetaLen+(bstrMetaID.Length()+1)*sizeof(wchar_t);
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(nLen-nMetaLen, reinterpret_cast<BYTE const*>(bstrMetaID.m_str))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(nMetaLen, cMetaData.m_p)))
					return E_FAIL;
				if ((nLen&3) && FAILED(a_pDst->Write(4-(nLen&3), DWORD_PADDING)))
					return E_FAIL;
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

