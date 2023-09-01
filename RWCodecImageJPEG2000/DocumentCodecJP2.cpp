// DocumentCodecJP2.cpp : Implementation of CDocumentCodecJP2

#include "stdafx.h"
#include "DocumentCodecJP2.h"

#include <MultiLanguageString.h>
#include <jasper/jasper.h>
#include "JapPerImageSink.h"
#include "JasPerImageSource.h"


// CDocumentCodecJP2

STDMETHODIMP CDocumentCodecJP2::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		return S_FALSE;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentCodecJP2::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
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

STDMETHODIMP CDocumentCodecJP2::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI))
			return E_FAIL;

		HRESULT hRes = S_OK;

		jas_image_cmptparm_t component[4];
		component[0].height = cBuffer.tSize.nY;
		component[0].width = cBuffer.tSize.nX;
		component[0].hstep = 1;
		component[0].vstep = 1;
		component[0].tlx = 0;
		component[0].tly = 0;
		component[0].prec = 8;
		component[0].sgnd = false;
		component[3] = component[2] = component[1] = component[0];
		int nComps = cBuffer.bAlpha ? 4 : 3;
		int const nCompStep = 4;
		int nROffset = 2;
		int nGOffset = 1;
		int nBOffset = 0;
		int nAOffset = 3;
		//component.smpltype;
		jas_image_t* image = jas_image_create(nComps, component, JAS_CLRSPC_SRGB);
		jas_matrix_t* data = jas_matrix_create(cBuffer.tSize.nY, cBuffer.tSize.nX);

		for (int y = 0; y < data->numrows_; ++y)
		{
			BYTE const* p = cBuffer.aData + cBuffer.nStride*y + nROffset;
			for (int x = 0; x < data->numcols_; ++x)
			{
				jas_matrix_set(data, y, x, *p);
				p += nCompStep;
			}
		}
		jas_image_writecmpt(image, 0, 0, 0, cBuffer.tSize.nX, cBuffer.tSize.nY, data);
		image->cmpts_[0]->type_ = JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_R);

		for (int y = 0; y < data->numrows_; ++y)
		{
			BYTE const* p = cBuffer.aData + cBuffer.nStride*y + nGOffset;
			for (int x = 0; x < data->numcols_; ++x)
			{
				jas_matrix_set(data, y, x, *p);
				p += nCompStep;
			}
		}
		jas_image_writecmpt(image, 1, 0, 0, cBuffer.tSize.nX, cBuffer.tSize.nY, data);
		image->cmpts_[1]->type_ = JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_G);

		for (int y = 0; y < data->numrows_; ++y)
		{
			BYTE const* p = cBuffer.aData + cBuffer.nStride*y + nBOffset;
			for (int x = 0; x < data->numcols_; ++x)
			{
				jas_matrix_set(data, y, x, *p);
				p += nCompStep;
			}
		}
		jas_image_writecmpt(image, 2, 0, 0, cBuffer.tSize.nX, cBuffer.tSize.nY, data);
		image->cmpts_[2]->type_ = JAS_IMAGE_CT_COLOR(JAS_IMAGE_CT_RGB_B);

		if (nComps == 4)
		{
			for (int y = 0; y < data->numrows_; ++y)
			{
				BYTE const* p = cBuffer.aData + cBuffer.nStride*y + nAOffset;
				for (int x = 0; x < data->numcols_; ++x)
				{
					jas_matrix_set(data, y, x, *p);
					p += nCompStep;
				}
			}
			jas_image_writecmpt(image, 3, 0, 0, cBuffer.tSize.nX, cBuffer.tSize.nY, data);
			image->cmpts_[3]->type_ = JAS_IMAGE_CT_OPACITY;
		}

		jas_matrix_destroy(data);

		SJasPerDstStream sStream(a_pDst);
		jp2_encode(image, &sStream, "");
		jas_image_destroy(image);

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct SAutoDeleteJasImage
{
	SAutoDeleteJasImage(jas_image_t* a_p) : p(a_p) {}
	~SAutoDeleteJasImage() { if (p) jas_image_destroy(p); }
	jas_image_t* operator->() { return p; }

	jas_image_t* p;
};

HRESULT CDocumentCodecJP2::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	SJasPerImageSource sStream(a_nLen, a_pData);
	if (jp2_validate(&sStream))
		return E_RW_UNKNOWNINPUTFORMAT;

	SAutoDeleteJasImage image(jp2_decode(&sStream, ""));
	if (image.p == NULL)
		return E_FAIL;

	if ((image->numcmpts_ != 3 ||
		image->brx_ != image->cmpts_[0]->width_ || image->brx_ != image->cmpts_[1]->width_ || image->brx_ != image->cmpts_[2]->width_ ||
		image->bry_ != image->cmpts_[0]->height_ || image->bry_ != image->cmpts_[1]->height_ || image->bry_ != image->cmpts_[2]->height_) &&
		(image->numcmpts_ != 4 ||
		image->brx_ != image->cmpts_[0]->width_ || image->brx_ != image->cmpts_[1]->width_ || image->brx_ != image->cmpts_[2]->width_ || image->brx_ != image->cmpts_[3]->width_ ||
		image->bry_ != image->cmpts_[0]->height_ || image->bry_ != image->cmpts_[1]->height_ || image->bry_ != image->cmpts_[2]->height_ || image->bry_ != image->cmpts_[3]->height_))
		return E_RW_UNSUPPORTEDINPUTFORMAT;

	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[image->brx_*image->bry_]);

	jas_stream_rewind(image->cmpts_[0]->stream_);
	jas_stream_rewind(image->cmpts_[1]->stream_);
	jas_stream_rewind(image->cmpts_[2]->stream_);

	TPixelChannel* pD = pBuffer;
	if (image->numcmpts_ == 3)
	{
		int i = 0;
		for (jas_image_coord_t iY = 0; iY < image->bry_; ++iY)
		{
			for (jas_image_coord_t iX = 0; iX < image->brx_; ++iX, ++pD)
			{
				pD->bB = jas_stream_getc(image->cmpts_[2]->stream_);
				pD->bG = jas_stream_getc(image->cmpts_[1]->stream_);
				pD->bR = jas_stream_getc(image->cmpts_[0]->stream_);
				pD->bA = 0xff;
			}
		}
	}
	else
	{
		jas_stream_rewind(image->cmpts_[3]->stream_);

		int i = 0;
		for (jas_image_coord_t iY = 0; iY < image->bry_; ++iY)
		{
			for (jas_image_coord_t iX = 0; iX < image->brx_; ++iX, ++pD)
			{
				pD->bB = jas_stream_getc(image->cmpts_[2]->stream_);
				pD->bG = jas_stream_getc(image->cmpts_[1]->stream_);
				pD->bR = jas_stream_getc(image->cmpts_[0]->stream_);
				pD->bA = jas_stream_getc(image->cmpts_[3]->stream_);
			}
		}
	}

	HRESULT hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, CImageSize(image->brx_, image->bry_), NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 2.2f, CImageTile(image->brx_, image->bry_, pBuffer));
	if (FAILED(hRes)) return hRes;

	if (a_pBase && a_bstrPrefix == NULL)
	{
		a_pBase->EncoderSet(__uuidof(DocumentCodecJP2), NULL);
	}

	return hRes;
}

