// DocumentEncoderWebP.cpp : Implementation of CDocumentEncoderWebP

#include "stdafx.h"
#include "DocumentEncoderWebP.h"

#include "WebPSaveConfig.h"
#include <RWDocumentAnimation.h>


// CDocumentEncoderWebP

STDMETHODIMP CDocumentEncoderWebP::DocumentType(IDocumentType** a_ppDocumentType)
{
	try
	{
		*a_ppDocumentType = NULL;
		*a_ppDocumentType = CDocumentTypeCreatorWebP::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentEncoderWebP::DefaultConfig(IConfig** a_ppDefCfg)
{
	return InitWebPConfig(a_ppDefCfg);
}

STDMETHODIMP CDocumentEncoderWebP::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA);
		return pDocImg ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "webp/encode.h"
#include "webp/mux.h"
#include "WebPUtils.h"

int WebPToDstStream(const uint8_t* data, size_t data_size, WebPPicture const* const picture)
{
	return SUCCEEDED(reinterpret_cast<IDataDstStream*>(picture->custom_ptr)->Write(data_size, data));
}

STDMETHODIMP CDocumentEncoderWebP::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CConfigValue cQuality;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPQUALITY), &cQuality);
		CConfigValue cLossless;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPLOSSLESS), &cLossless);
		CConfigValue cMethod;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPOPTIMIZATION), &cMethod);

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI))
			return E_FAIL;

		BYTE const* pBuffer = cBuffer.aData;

		CWebPMuxPtr pMux(WebPMuxNew());

		WebPConfig tCfg;
		ZeroMemory(&tCfg, sizeof tCfg);
		WebPConfigPreset(&tCfg, WEBP_PRESET_DEFAULT, cQuality);
		if (cLossless.operator bool())
			tCfg.lossless = 1;
		tCfg.method = cMethod.operator LONG();

		if (cBuffer.tSize.nX <= WEBP_MAX_DIMENSION && cBuffer.tSize.nY <= WEBP_MAX_DIMENSION)
		{
			CWebPMemoryWriter cMem;

			CWebPPicture tPic;
			tPic.colorspace = cBuffer.bAlpha ? WEBP_YUV420A : WEBP_YUV420;
			tPic.width = cBuffer.tSize.nX;
			tPic.height = cBuffer.tSize.nY;
			tPic.writer = WebPMemoryWrite;
			tPic.custom_ptr = &cMem;
			tPic.use_argb = tCfg.lossless;
			int nOK = cBuffer.bAlpha ?
				WebPPictureImportBGRA(&tPic, pBuffer, cBuffer.nStride) :
				WebPPictureImportBGRX(&tPic, pBuffer, cBuffer.nStride);

			if (0 == WebPEncode(&tCfg, &tPic))
				return E_FAIL;

			WebPData tData;
			tData.bytes = cMem.mem;
			tData.size = cMem.size;
			if (WEBP_MUX_OK != WebPMuxSetImage(pMux, &tData, 1))
				return E_FAIL;
		}
		else // needs fragments
			return E_FAIL; // fragments not supported anymore?
		//{
		//	ULONG const nStep = WEBP_MAX_DIMENSION&(~1);
		//	for (ULONG nY = 0; nY < cBuffer.tSize.nY; nY += nStep)
		//	{
		//		ULONG nY1 = min(nY+nStep, cBuffer.tSize.nY);
		//		for (ULONG nX = 0; nX < cBuffer.tSize.nX; nX += nStep)
		//		{
		//			ULONG nX1 = min(nX+nStep, cBuffer.tSize.nX);

		//			CWebPMemoryWriter cMem;

		//			CWebPPicture tPic;
		//			tPic.colorspace = cBuffer.bAlpha ? WEBP_YUV420A : WEBP_YUV420;
		//			tPic.width = nX1-nX;
		//			tPic.height = nY1-nY;
		//			tPic.writer = WebPMemoryWrite;
		//			tPic.custom_ptr = &cMem;
		//			tPic.use_argb = tCfg.lossless;
		//			int nOK = cBuffer.bAlpha ?
		//				WebPPictureImportBGRA(&tPic, pBuffer+nY*cBuffer.nStride+nX*4, cBuffer.nStride) :
		//				WebPPictureImportBGRX(&tPic, pBuffer+nY*cBuffer.nStride+nX*4, cBuffer.nStride);

		//			if (0 == WebPEncode(&tCfg, &tPic))
		//				return E_FAIL;

		//			WebPMuxFrameInfo tFrame;
		//			tFrame.id = WEBP_CHUNK_FRGM;
		//			tFrame.dispose_method = WEBP_MUX_DISPOSE_NONE;
		//			tFrame.bitstream.bytes = cMem.mem;
		//			tFrame.bitstream.size = cMem.size;
		//			tFrame.duration = 0;
		//			tFrame.x_offset = nX;
		//			tFrame.y_offset = nY;

		//			if (WEBP_MUX_OK != WebPMuxPushFrame(pMux, &tFrame, 1))
		//				return E_FAIL;
		//		}
		//	}
		//}

		CComPtr<IImageMetaData> pIMD;
		a_pDoc->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD)
		{
			ULONG nLen = 0;
			pIMD->GetBlockSize(CComBSTR(L"EXIF"), &nLen);
			if (nLen)
			{
				CAutoVectorPtr<BYTE> cData(new BYTE[nLen]);
				pIMD->GetBlock(CComBSTR(L"EXIF"), nLen, cData);
				WebPData tEXIF;
				tEXIF.bytes = cData;
				tEXIF.size = nLen;
				if (WEBP_MUX_OK != WebPMuxSetChunk(pMux, "EXIF", &tEXIF, 1))
					return E_FAIL;
			}
			nLen = 0;
			pIMD->GetBlockSize(CComBSTR(L"XMP"), &nLen);
			if (nLen)
			{
				CAutoVectorPtr<BYTE> cData(new BYTE[nLen]);
				pIMD->GetBlock(CComBSTR(L"XMP"), nLen, cData);
				WebPData tXMP;
				tXMP.bytes = cData;
				tXMP.size = nLen;
				if (WEBP_MUX_OK != WebPMuxSetChunk(pMux, "XMP ", &tXMP, 1))
					return E_FAIL;
			}
		}

		CWebPData cData;
		if (WEBP_MUX_OK != WebPMuxAssemble(pMux, &cData))
			return E_FAIL;
		return a_pDst->Write(cData.size, cData.bytes);
	}
	catch (...)
	{
		return a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentEncoderWebP::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	*a_ppName = NULL;
	if (a_pConfig == NULL)
		return E_FAIL;

	CConfigValue lossless;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WEBPLOSSLESS), &lossless);
	if (lossless)
	{
		*a_ppName = new CMultiLanguageString(L"[0409]Lossless[0405]Bezeztrátově");
		return S_OK;
	}

	CConfigValue quality;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WEBPQUALITY), &quality);
	wchar_t sz[32];
	_swprintf(sz, L"%i%%", quality.operator LONG());
	*a_ppName = new CSimpleLocalizedString(SysAllocString(sz));
	return S_OK;
}

