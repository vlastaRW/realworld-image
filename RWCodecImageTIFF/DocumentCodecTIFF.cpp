// DocumentCodecTIFF.cpp : Implementation of CDocumentCodecTIFF

#include "stdafx.h"
#include "DocumentCodecTIFF.h"
#include <tiffio.h>
#include <MultiLanguageString.h>


// CDocumentCodecTIFF

STDMETHODIMP CDocumentCodecTIFF::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
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

class CTIFFDataSink
{
public:
	CTIFFDataSink() : m_nPos(0)
	{
	}

	static tsize_t Read(thandle_t a_hContext, tdata_t a_pBuf, tsize_t a_nSize)
	{
		return 0;
	}

	static toff_t Seek(thandle_t a_hContext, toff_t a_nOffset, int whence)
	{
		CTIFFDataSink* const pThis = reinterpret_cast<CTIFFDataSink*>(a_hContext);
		switch (whence)
		{
		case 0:
			pThis->m_nPos = a_nOffset;
			break;
		case 1:
			pThis->m_nPos += a_nOffset;
			break;
		case 2:
			pThis->m_nPos = pThis->m_cData.size() - a_nOffset;
			break;
		}
		return pThis->m_nPos;
	}

	static tsize_t Write(thandle_t a_hContext, tdata_t buf, tsize_t size)
	{
		try
		{
			CTIFFDataSink* const pThis = reinterpret_cast<CTIFFDataSink*>(a_hContext);
			if (pThis->m_cData.size() < pThis->m_nPos+size)
				pThis->m_cData.resize(pThis->m_nPos+size);
			std::copy(reinterpret_cast<BYTE const*>(buf), reinterpret_cast<BYTE const*>(buf)+size, pThis->m_cData.begin()+pThis->m_nPos);
			pThis->m_nPos += size;
			return size;
		}
		catch (...)
		{
			return 0;
		}
	}

	static int Close(thandle_t fd)
	{
		return 0;
	}

	static toff_t Size(thandle_t a_hContext)
	{
		return reinterpret_cast<CTIFFDataSink*>(a_hContext)->m_nPos;
	}

	static int MapFile(thandle_t a_hContext, tdata_t* a_ppBase, toff_t* a_pSize)
	{
		return 0;
	}

	static void UnmapFile(thandle_t a_hContext, tdata_t a_pBase, toff_t a_tSize)
	{
	}

	HRESULT GetData(IReturnedData* a_pDst)
	{
		return m_cData.empty() ? E_FAIL : a_pDst->Write(m_cData.size(), &(m_cData[0]));
	}

private:
	std::vector<BYTE> m_cData;
	ULONG m_nPos;
};

STDMETHODIMP CDocumentCodecTIFF::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		//CConfigValue cQuality;
		//a_pCfg->ItemValueGet(CComBSTR(CFGID_WEBPQUALITY), &cQuality);

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI))
			return E_FAIL;

		if (!cBuffer.bAlpha)
			cBuffer.ToRGB();
		else
		{
			TPixelChannel* p = reinterpret_cast<TPixelChannel*>(cBuffer.aData.m_p);
			for (TPixelChannel* const pEnd = p+cBuffer.tSize.nX*cBuffer.tSize.nY; p != pEnd; ++p)
				p->n = (p->n&0xff00ff00)|((p->n&0xff)<<16)|((p->n&0xff0000)>>16);
		}

		BYTE const* pBuffer = cBuffer.aData;
		ULONG const nSizeX = cBuffer.tSize.nX;
		ULONG const nSizeY = cBuffer.tSize.nY;

		CTIFFDataSink cTIFFDst;
		TIFF* pTiff = TIFFClientOpen("unavailable", "w", reinterpret_cast<thandle_t>(&cTIFFDst),
			CTIFFDataSink::Read, CTIFFDataSink::Write, CTIFFDataSink::Seek,
			CTIFFDataSink::Close, CTIFFDataSink::Size,
			NULL, NULL);
		if (pTiff == NULL)
			return E_FAIL;

		TIFFSetField(pTiff, TIFFTAG_IMAGEWIDTH, nSizeX);
		TIFFSetField(pTiff, TIFFTAG_IMAGELENGTH, nSizeY);
		TIFFSetField(pTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(pTiff, TIFFTAG_BITSPERSAMPLE, 8);
		TIFFSetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, cBuffer.bAlpha ? 4 : 3);
		TIFFSetField(pTiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);//COMPRESSION_DEFLATE);
		TIFFSetField(pTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		if (cBuffer.bAlpha)
		{
			uint16 ext = EXTRASAMPLE_UNASSALPHA;
			TIFFSetField(pTiff, TIFFTAG_EXTRASAMPLES, 1, &ext);
		}

		if (size_t(-1) == TIFFWriteEncodedStrip(pTiff, 0, const_cast<BYTE*>(pBuffer), nSizeX*nSizeY * (cBuffer.bAlpha ? 4 : 3)))
			hRes = E_FAIL;

		TIFFClose(pTiff);

		if (SUCCEEDED(hRes))
			hRes = cTIFFDst.GetData(a_pDst);
	}
	catch (...)
	{
		hRes = a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}

	return hRes;
}

class CTIFFDataSource
{
public:
	CTIFFDataSource(BYTE const* a_pData, ULONG a_nSize) :
		m_pData(a_pData), m_nSize(a_nSize), m_nOffset(0)
	{
	}

	static tsize_t Read(thandle_t a_hContext, tdata_t a_pBuf, tsize_t a_nSize)
	{
		CTIFFDataSource* const pThis = reinterpret_cast<CTIFFDataSource*>(a_hContext);
		if (pThis->m_nSize < (pThis->m_nOffset + a_nSize))
		{
			if (pThis->m_nSize < pThis->m_nOffset)
				return 0;
			else
				a_nSize = pThis->m_nSize-pThis->m_nOffset;
		}

		CopyMemory(a_pBuf, pThis->m_pData+pThis->m_nOffset, a_nSize);

		pThis->m_nOffset += a_nSize;

		return a_nSize;
	}

	static toff_t Seek(thandle_t a_hContext, toff_t a_nOffset, int whence)
	{
		CTIFFDataSource* const pThis = reinterpret_cast<CTIFFDataSource*>(a_hContext);
		switch (whence)
		{
		case 0:
			pThis->m_nOffset = a_nOffset;
			break;
		case 1:
			pThis->m_nOffset += a_nOffset;
			break;
		case 2:
			pThis->m_nOffset = pThis->m_nSize - a_nOffset;
			break;
		}
		return pThis->m_nOffset;
	}

	static tsize_t Write(thandle_t fd, tdata_t buf, tsize_t size)
	{
		ATLASSERT(0);
		return 0;
	}

	static int Close(thandle_t fd)
	{
		return 0;
	}

	static toff_t Size(thandle_t a_hContext)
	{
		return reinterpret_cast<CTIFFDataSource*>(a_hContext)->m_nSize;
	}

	static int MapFile(thandle_t a_hContext, tdata_t* a_ppBase, toff_t* a_pSize)
	{
		CTIFFDataSource const* const pThis = reinterpret_cast<CTIFFDataSource const*>(a_hContext);
		*a_pSize = pThis->m_nSize;
		*a_ppBase = reinterpret_cast<tdata_t>(const_cast<BYTE*>(pThis->m_pData));
		return 1;
	}

	static void UnmapFile(thandle_t a_hContext, tdata_t a_pBase, toff_t a_tSize)
	{
	}

private:
	BYTE const* m_pData;
	ULONG m_nSize;
	ULONG m_nOffset;
};

HRESULT CDocumentCodecTIFF::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		CTIFFDataSource cTIFFSrc(a_pData, a_nLen);
		TIFF* pTiff = TIFFClientOpen("unavailable", "rM", reinterpret_cast<thandle_t>(&cTIFFSrc),
			CTIFFDataSource::Read, CTIFFDataSource::Write, CTIFFDataSource::Seek,
			CTIFFDataSource::Close, CTIFFDataSource::Size,
			CTIFFDataSource::MapFile, CTIFFDataSource::UnmapFile);
		if (pTiff == NULL)
			return E_RW_UNKNOWNINPUTFORMAT;

		HRESULT hRes = S_OK;
		try
		{
			TImageSize tSize = {1, 1};
			TIFFGetField(pTiff, TIFFTAG_IMAGEWIDTH, &tSize.nX);
			TIFFGetField(pTiff, TIFFTAG_IMAGELENGTH, &tSize.nY);
			TIFFSetField(pTiff, TIFFTAG_ORIENTATION, ORIENTATION_LEFTBOT);
			CAutoVectorPtr<TPixelChannel> pDst(new TPixelChannel[tSize.nX*tSize.nY]);
			if (0 == TIFFReadRGBAImage(pTiff, tSize.nX, tSize.nY, reinterpret_cast<uint32*>(pDst.m_p)))
				hRes = E_FAIL;
			else
			{
				TPixelChannel* p = pDst;
				for (TPixelChannel* const pEnd = p+tSize.nX*tSize.nY; p != pEnd; ++p)
					p->n = (p->n&0xff00ff00)|((p->n&0xff)<<16)|((p->n&0xff0000)>>16);
				hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(tSize.nX, tSize.nY, pDst));
			}
		}
		catch (...)
		{
			hRes = E_UNEXPECTED;
		}

		TIFFClose(pTiff);
		if (a_pEncoderID)
			*a_pEncoderID = __uuidof(DocumentCodecTIFF);
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

