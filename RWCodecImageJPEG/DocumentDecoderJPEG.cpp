// DocumentDecoderJPEG.cpp : Implementation of CDocumentDecoderJPEG

#include "stdafx.h"
#include "DocumentDecoderJPEG.h"
#include <MultiLanguageString.h>

extern "C"
{
#include "iccjpeg.h"
}
#include <lcms.h>

#include "JPEGUtils.h"
#include "JPEGConfigIDs.h"

struct SAutoDeleteLCMS
{
	cmsHPROFILE hInProfile;
	cmsHPROFILE hOutProfile;
	cmsHTRANSFORM hTransform;

	SAutoDeleteLCMS() : hInProfile(0), hOutProfile(0), hTransform(0) {}
	~SAutoDeleteLCMS()
	{
		if (hTransform) cmsDeleteTransform(hTransform);
		if (hInProfile) cmsCloseProfile(hInProfile);
		if (hOutProfile) cmsCloseProfile(hOutProfile);
	}
};

static int RWErrorHandler(int ErrorCode, const char *ErrorText)
{
	if (ErrorCode == LCMS_ERRC_ABORTED)
		throw -1;
    return 0;
}

struct CAutoDeleteICCBlock
{
	CAutoDeleteICCBlock() : p(NULL) {}
	~CAutoDeleteICCBlock() { if (p) { free(p); p = NULL; } }
	BYTE* p;
};

// CDocumentDecoderJPEG

HRESULT CDocumentDecoderJPEG::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	if (a_nLen < 2 || a_pData == NULL || a_pData[0] != 0xff || a_pData[1] != 0xd8)
		return E_RW_UNKNOWNINPUTFORMAT;

	static bool bSetErrHandler = true;
	if (bSetErrHandler)
	{
		cmsSetErrorHandler(RWErrorHandler);
		bSetErrHandler = false;
	}

	CJPEGDecompressStruct cinfo;
	CJPEGDataSource cDataSrc(a_pData, a_nLen);
	cinfo.src = &cDataSrc;

	for (int m = 0; m < 16; ++m)
		jpeg_save_markers(&cinfo, JPEG_APP0 + m, 0xFFFF);
	setup_read_icc_profile(&cinfo);
	cinfo.src = &cDataSrc;
	jpeg_read_header(&cinfo, TRUE);

	if (cinfo.num_components != 1 && cinfo.num_components != 3 && cinfo.num_components != 4)
	{
		return E_RW_UNSUPPORTEDINPUTFORMAT;
	}

	int nSamp = cinfo.max_h_samp_factor == 1 ? (cinfo.max_v_samp_factor == 1 ? CFGVAL_CH_1x1 : CFGVAL_CH_1x2) : (cinfo.max_v_samp_factor == 1 ? CFGVAL_CH_2x1 : CFGVAL_CH_2x2);
	bool bArithm = cinfo.arith_code;

	//tIF.dwFormatParameter = (nSamp)<<16|(IsRWPaint() ? (CFGVAL_LL_NEVER<<8) : (CFGVAL_LL_ALL<<8))|(CFGVAL_MD_KEEP<<12)|85; // TODO: determine quality of source
	TImageSize const tSize = {cinfo.image_width, cinfo.image_height};
	TImageResolution tResolution = {1, 0, 1, 0};
	if (cinfo.saw_JFIF_marker && cinfo.density_unit)
	{
		tResolution.nNumeratorX = cinfo.X_density;
		tResolution.nNumeratorY = cinfo.Y_density;
		tResolution.nDenominatorX = tResolution.nDenominatorY = cinfo.density_unit == 1 ? 254 : 100; // dots per inch : dots per centimeter
	}
	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);

//	jpeg_calc_output_dimensions();

	cinfo.dct_method = JDCT_FLOAT;
	jpeg_start_decompress(&cinfo);

	CAutoDeleteICCBlock icc_data_ptr;
	unsigned int icc_data_len = 0;
	boolean bICC = read_icc_profile(&cinfo, &icc_data_ptr.p, &icc_data_len);

	SAutoDeleteLCMS cXform;
	if (icc_data_ptr.p)
	{
		try
		{
			cXform.hInProfile = cmsOpenProfileFromMem(icc_data_ptr.p, icc_data_len);
			if (cXform.hInProfile)
			{
				cXform.hOutProfile = cmsCreate_sRGBProfile();
				if (cXform.hOutProfile)
					cXform.hTransform = cmsCreateTransform(cXform.hInProfile, cinfo.out_color_components == 4 ? (cinfo.saw_Adobe_marker ? TYPE_CMYK_8_REV : TYPE_CMYK_8) : (cinfo.out_color_components == 1 ? TYPE_GRAY_8 : TYPE_RGB_8), cXform.hOutProfile, TYPE_RGB_8, INTENT_PERCEPTUAL, 0);
			}
		}
		catch (...) {}
	}

	JSAMPLE** apRows = reinterpret_cast<JSAMPLE**>(_alloca(sizeof(JSAMPLE*)*tSize.nY));
	if (cinfo.out_color_components == 4) // cmyk
	{
		if (icc_data_ptr.p == NULL) // no embedded profile -> use a generic CMYK profile
		{
			HRSRC hRes = FindResource(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDR_ICC_CMYK), _T("ICC"));
			HGLOBAL hMem = LoadResource(_pModule->get_m_hInst(), hRes);
			LPVOID pMem = LockResource(hMem);
			ULONG nMem = SizeofResource(_pModule->get_m_hInst(), hRes);
			if (nMem && pMem) try
			{
				cXform.hInProfile = cmsOpenProfileFromMem(pMem, nMem);
				if (cXform.hInProfile)
				{
					cXform.hOutProfile = cmsCreate_sRGBProfile();
					if (cXform.hOutProfile)
						cXform.hTransform = cmsCreateTransform(cXform.hInProfile, cinfo.saw_Adobe_marker ? TYPE_CMYK_8_REV : TYPE_CMYK_8, cXform.hOutProfile, TYPE_RGB_8, INTENT_PERCEPTUAL, 0);
				}
			}
			catch (...) {}
		}

		CAutoVectorPtr<BYTE> cLine(new BYTE[tSize.nX*3]);
		ULONG i;
		for (i = 0; i < tSize.nY; i++)
			apRows[i] = reinterpret_cast<JSAMPLE*>(pBuffer.m_p+tSize.nX*i);
		for (i = 0; i < tSize.nY; )
			i += jpeg_read_scanlines(&cinfo, apRows+i, tSize.nY-i);
		TPixelChannel* p = pBuffer;
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			if (cXform.hTransform)
			{
				cmsDoTransform(cXform.hTransform, p, cLine, tSize.nX);
				BYTE const* pL = cLine;
				for (TPixelChannel* const pEnd = p+tSize.nX; p < pEnd; ++p, pL+=3)
				{
					p->bR = pL[0];
					p->bG = pL[1];
					p->bB = pL[2];
					p->bA = 0xff;
				}
			}
			else
			{
				for (TPixelChannel* const pEnd = p+tSize.nX; p < pEnd; ++p)
				{
					BYTE const pS[4] = {reinterpret_cast<BYTE*>(p)[0], reinterpret_cast<BYTE*>(p)[1], reinterpret_cast<BYTE*>(p)[2], reinterpret_cast<BYTE*>(p)[3]};
					if (cinfo.saw_Adobe_marker)
					{
						int const k = pS[3];
						p->bR = k * pS[0] / 255;
						p->bG = k * pS[1] / 255;
						p->bB = k * pS[2] / 255;
					}
					else
					{
						int const k = 255-pS[3];
						p->bR = k * (255-pS[0]) / 255;
						p->bG = k * (255-pS[1]) / 255;
						p->bB = k * (255-pS[2]) / 255;
					}
					p->bA = 0xff;
				}
			}
		}
	}
	else if (cinfo.out_color_components == 3)
	{
		ULONG i;
		for (i = 0; i < tSize.nY; i++)
			apRows[i] = reinterpret_cast<JSAMPLE*>(pBuffer.m_p+tSize.nX*i)+tSize.nX;
		for (i = 0; i < tSize.nY; )
			i += jpeg_read_scanlines(&cinfo, apRows+i, tSize.nY-i);
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			if (/*cinfo.jpeg_color_space != JCS_YCbCr &&*/ cXform.hTransform)
				cmsDoTransform(cXform.hTransform, apRows[y], apRows[y], tSize.nX);
			BYTE const* pS = apRows[y];
			TPixelChannel* pD = pBuffer.m_p+tSize.nX*y;
			for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, pS+=3)
			{
				pD->bR = pS[0];
				pD->bG = pS[1];
				pD->bB = pS[2];
				pD->bA = 0xff;
			}
		}
	}
	else if (cinfo.out_color_components == 1)
	{
		ULONG i;
		for (i = 0; i < tSize.nY; i++)
			apRows[i] = reinterpret_cast<JSAMPLE*>(pBuffer.m_p+tSize.nX*i)+(cXform.hTransform ? 0 : 3*tSize.nX);
		for (i = 0; i < tSize.nY; )
			i += jpeg_read_scanlines(&cinfo, apRows+i, tSize.nY-i);
		if (cXform.hTransform)
		{
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				cmsDoTransform(cXform.hTransform, apRows[y], apRows[y]+tSize.nX, tSize.nX);
				BYTE const* pS = apRows[y]+tSize.nX;
				TPixelChannel* pD = pBuffer.m_p+tSize.nX*y;
				for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, pS+=3)
				{
					pD->bR = pS[0];
					pD->bG = pS[1];
					pD->bB = pS[2];
					pD->bA = 0xff;
				}
			}
		}
		else
		{
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				BYTE* pS = apRows[y];
				TPixelChannel* pD = pBuffer.m_p+tSize.nX*y;
				for (TPixelChannel* const pEnd = pD+tSize.nX; pD < pEnd; ++pD, ++pS)
				{
					pD->bR = pD->bG = pD->bB = *pS;
					pD->bA = 0xff;
				}
			}
		}
	}

	HRESULT hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tResolution, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(tSize.nX, tSize.nY, pBuffer));
	if (SUCCEEDED(hRes))
	{
		CComQIPtr<IDocumentFactoryMetaData> pMDF(a_pBuilder);
		if (pMDF)
		{
			if (cinfo.num_components == 3)
				pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"JPEG"), cDataSrc.DataSizeGet(), cDataSrc.DataGet()); // TODO: strip metadata?
			//if (bICC && icc_data_ptr && icc_data_len && cinfo.jpeg_color_space == JCS_YCbCr && cinfo.num_components == 3) // CMYK jpegs are converted to sRGB
			//	pMeta->SetBlock(CComBSTR(L"ICC"), icc_data_len, icc_data_ptr);
			int aAppIndex[0x100-JPEG_APP0];
			ZeroMemory(aAppIndex, sizeof aAppIndex);
			for (jpeg_saved_marker_ptr marker = cinfo.marker_list; marker != NULL; marker = marker->next)
			{
				if (marker->marker == JPEG_APP0+1 &&
					GETJOCTET(marker->data[0]) == 'E' &&
					GETJOCTET(marker->data[1]) == 'x' &&
					GETJOCTET(marker->data[2]) == 'i' &&
					GETJOCTET(marker->data[3]) == 'f' &&
					GETJOCTET(marker->data[4]) == 0)
				{
					pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(L"EXIF"), marker->data_length, marker->data);
					break;
				}
				if (marker->marker > JPEG_APP0)
				{
					int i = aAppIndex[marker->marker-JPEG_APP0]++;
					wchar_t sz[16];
					swprintf(sz, L"JPEG%i-%i", int(marker->marker-JPEG_APP0), i);
					pMDF->AddBlock(a_bstrPrefix, a_pBase, CComBSTR(sz), marker->data_length, marker->data);
				}
			}
		}

		if (a_pEncoderID) *a_pEncoderID = __uuidof(DocumentEncoderJPEG);
		if (a_ppEncoderCfg)
		{
			CComPtr<IConfig> pConfig;
			pConfig.Attach(CDocumentEncoderJPEG::Config());
			CComBSTR bstrChroma(CFGID_CHROMASUBSAMPLING);
			CComBSTR bstrArithm(CFGID_ARITHMETIC);
			BSTR aIDs[2] = {bstrChroma, bstrArithm};
			TConfigValue aVals[2];
			aVals[0].eTypeID = ECVTInteger;
			aVals[0].iVal = nSamp;
			aVals[1].eTypeID = ECVTBool;
			aVals[1].bVal = bArithm;
			pConfig->ItemValuesSet(2, aIDs, aVals);
			*a_ppEncoderCfg = pConfig.Detach();
		}
	}

	jpeg_finish_decompress(&cinfo);

	return hRes;
}

