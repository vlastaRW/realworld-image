// DocumentEncoderPNG.cpp : Implementation of CDocumentEncoderPNG

#include "stdafx.h"
#include "DocumentEncoderPNG.h"

#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>


// CDocumentEncoderPNG

STDMETHODIMP CDocumentEncoderPNG::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorPNG::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderPNGDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderPNGDlg>,
	public CDialogResize<CConfigGUIEncoderPNGDlg>
{
public:
	enum { IDC_CGPNG_EXTRAATTRS = 100, IDC_CGPNG_INTERLACE, IDC_CGPNG_OPTIMIZATION };

	BEGIN_DIALOG_EX(0, 0, 154, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_CHECKBOX(_T("[0409]Maximum compression[0405]Maximální komprese"), IDC_CGPNG_OPTIMIZATION, 0, 0, 50, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Extra attributes[0405]Další atributy"), IDC_CGPNG_EXTRAATTRS, 57, 0, 50, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Interlaced[0405]Prokládat"), IDC_CGPNG_INTERLACE, 114, 0, 50, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderPNGDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderPNGDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderPNGDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderPNGDlg)
		DLGRESIZE_CONTROL(IDC_CGPNG_OPTIMIZATION, DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_CGPNG_EXTRAATTRS, DLSZ_DIVMOVE_X(3)|DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_CGPNG_INTERLACE, DLSZ_MULDIVMOVE_X(2,3)|DLSZ_DIVSIZE_X(3))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderPNGDlg)
		CONFIGITEM_CHECKBOX_FLAG(IDC_CGPNG_OPTIMIZATION, CFGID_OPTIMIZE, 1)
		CONFIGITEM_CHECKBOX(IDC_CGPNG_EXTRAATTRS, CFGID_EXTRAATTRS)
		CONFIGITEM_CHECKBOX(IDC_CGPNG_INTERLACE, CFGID_INTERLACING)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

IConfig* CDocumentEncoderPNG::Config()
{
	CComPtr<IConfigWithDependencies> pCfgInit;
	RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

	pCfgInit->ItemInsSimple(CComBSTR(CFGID_EXTRAATTRS), CMultiLanguageString::GetAuto(L"[0409]Extra attributes[0405]Další atributy"), CMultiLanguageString::GetAuto(L"[0409]Save optional attributes like background color, resolution, and gamma.[0405]Uložit nepovinné atributy jako je barva pozadí, rozlišení a gama."), CConfigValue(true), NULL, 0, NULL);
	pCfgInit->ItemInsSimple(CComBSTR(CFGID_INTERLACING), CMultiLanguageString::GetAuto(L"[0409]Interlacing[0405]Prokládání"), CMultiLanguageString::GetAuto(L"[0409]Enable interlacing.[0405]Povolit prokládání."), CConfigValue(false), NULL, 0, NULL);
	pCfgInit->ItemInsSimple(CComBSTR(CFGID_OPTIMIZE), CMultiLanguageString::GetAuto(L"[0409]Compression[0405]Komprese"), CMultiLanguageString::GetAuto(L"[0409]Create smaller files, but saving can take significant amount of time.[0405]Vytvořit menší soubory, ale ukládání může trvat dlouho."), CConfigValue(0L), NULL, 0, NULL);

	CConfigCustomGUI<&CLSID_DocumentEncoderPNG, CConfigGUIEncoderPNGDlg>::FinalizeConfig(pCfgInit);
	return pCfgInit.Detach();
}

STDMETHODIMP CDocumentEncoderPNG::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		*a_ppDefCfg = Config();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderPNG::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
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

void PNGAPI PNGErrorFnc(png_structp, png_const_charp)
{
	throw E_FAIL;
}
void PNGAPI PNGWarningFnc(png_structp, png_const_charp)
{
}

class CPNGStreamWriter
{
public:
	CPNGStreamWriter(IReturnedData* a_pDst) : m_pDst(a_pDst), m_nWritten(0), m_bFailed(false)
	{
	}

	operator png_voidp()
	{
		return reinterpret_cast<png_voidp>(this);
	}

	static void PNGAPI PngRWFnc(png_structp a_pPNGStruct, png_bytep a_pBuffer, png_size_t a_nBufferLen)
	{
		CPNGStreamWriter* pThis = reinterpret_cast<CPNGStreamWriter*>(png_get_io_ptr(a_pPNGStruct));
		BYTE* pOut = NULL;
		if (!pThis->m_bFailed)
		{
			pThis->m_bFailed = FAILED(pThis->m_pDst->Write(a_nBufferLen, a_pBuffer));
			pThis->m_nWritten += a_nBufferLen;
		}
	}
	static void PNGAPI PngFlushFnc(png_structp)
	{
	}

private:
	IReturnedData* m_pDst;
	ULONG m_nWritten;
	bool m_bFailed;
};

extern "C"
{
#include "zopfli/zopfli.h"
#include "zopfli/zlib_container.h"
}

void zopfli_free(png_bytep ptr)
{
	free(ptr);
}

int zopfli_deflate(png_bytep raw_data, png_size_t raw_size, png_bytepp comp_data, png_size_tp comp_size, void (**comp_free)(png_bytep))
{
	ZopfliOptions opts;
	ZopfliInitOptions(&opts);
	*comp_free = zopfli_free;
	//ZopfliCompress(&opts, ZOPFLI_FORMAT_DEFLATE, raw_data, raw_size, comp_data, comp_size);
    //unsigned char bp = 0;
    //ZopfliDeflate(&opts, 2 /* Dynamic block */, 1,
    //              raw_data, raw_size, &bp, comp_data, comp_size);
	ZopfliZlibCompress(&opts, raw_data, raw_size, comp_data, comp_size);
	return 0;
}

#include <set>

bool ShouldUsePalette(ULONG nSizeX, ULONG nSizeY, std::map<DWORD, BYTE> const& clrs)
{
	if (clrs.size() <= 4 || nSizeX*nSizeY > 2048)
		return true;
	std::set<DWORD> pal;
	for (std::map<DWORD, BYTE>::const_iterator i = clrs.begin(); i != clrs.end(); ++i)
		pal.insert(0xf8f8f8f8&i->first);
	return pal.size()*2 <= clrs.size();
}

STDMETHODIMP CDocumentEncoderPNG::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CConfigValue cInterlacing;
		CConfigValue cExtraAttr;
		CConfigValue cOptimize;
		if (a_pCfg)
		{
			a_pCfg->ItemValueGet(CComBSTR(CFGID_INTERLACING), &cInterlacing);
			a_pCfg->ItemValueGet(CComBSTR(CFGID_EXTRAATTRS), &cExtraAttr);
			a_pCfg->ItemValueGet(CComBSTR(CFGID_OPTIMIZE), &cOptimize);
		}
		if (cInterlacing.TypeGet() != ECVTBool) cInterlacing = false;
		if (cExtraAttr.TypeGet() != ECVTBool) cExtraAttr = true;
		if (cOptimize.TypeGet() != ECVTInteger) cOptimize = 0L;

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI, true, true))
			return E_FAIL;
		ULONG const nSizeX = cBuffer.tSize.nX;
		ULONG const nSizeY = cBuffer.tSize.nY;
		CPixelChannel cDef;
		pI->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &cDef));

		png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, PNGErrorFnc, PNGWarningFnc);
		if (png_ptr == NULL)
			return E_FAIL;

		if (cOptimize.operator LONG()&1)
			png_set_cust_comp(png_ptr, zopfli_deflate);

		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			png_destroy_write_struct(&png_ptr, NULL);
			return E_FAIL;
		}

		try
		{
			CPNGStreamWriter cSink(a_pDst);
			png_set_write_fn(png_ptr, &cSink, CPNGStreamWriter::PngRWFnc, CPNGStreamWriter::PngFlushFnc);

			CAutoVectorPtr<BYTE> cBuffer2;
			png_bytep pB = reinterpret_cast<png_bytep>(cBuffer.aData.m_p);

			bool bBW = !cBuffer.bColor;
			bool bHasAlpha = cBuffer.bAlpha;
			std::map<DWORD, BYTE> clrs;
			{
				DWORD const* p = reinterpret_cast<DWORD const*>(cBuffer.aData.m_p);
				for (DWORD const* const e = p+cBuffer.tSize.nX*cBuffer.tSize.nY; p != e; ++p)
				{
					std::map<DWORD, BYTE>::iterator i = clrs.find(*p);
					if (i == clrs.end())
					{
						size_t const s = clrs.size();
						clrs[*p] = static_cast<BYTE>(s);
						if (clrs.size() > 256)
							break; // too many colors for a paletted image
					}
				}
			}

			png_bytep* apRows = reinterpret_cast<png_bytep*>(_alloca(sizeof(png_bytep*)*nSizeY));

			if (((!bBW || bHasAlpha) && clrs.size() <= 256 || clrs.size() <= 16) && ShouldUsePalette(nSizeX, nSizeY, clrs))
			{
				int bits = clrs.size() <= 2 ? 1 : (
						   clrs.size() <= 4 ? 2 : (
						   clrs.size() <= 16 ? 4 : 8));
				CAutoVectorPtr<png_color> pal(new png_color[clrs.size()]);
				for (std::map<DWORD, BYTE>::const_iterator i = clrs.begin(), e = clrs.end(); i != e; ++i)
				{
					png_color* p = pal.m_p+i->second;
					p->red = GetBValue(i->first);
					p->green = GetGValue(i->first);
					p->blue = GetRValue(i->first);
				}
				png_set_PLTE(png_ptr, info_ptr, pal, clrs.size());

				if (bHasAlpha)
				{
					CAutoVectorPtr<png_byte> alp(new png_byte[clrs.size()]);
					for (std::map<DWORD, BYTE>::const_iterator i = clrs.begin(), e = clrs.end(); i != e; ++i)
						alp[i->second] = ((i->first)>>24)&0xff;
					png_set_tRNS(png_ptr, info_ptr, alp, clrs.size(), NULL);
				}
				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, bits,
					PNG_COLOR_TYPE_PALETTE,
					cInterlacing.operator bool() ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
				BYTE* pD = cBuffer.aData.m_p;
				DWORD const* p = reinterpret_cast<DWORD const*>(cBuffer.aData.m_p);
				for (DWORD const* const e = p+cBuffer.tSize.nX*cBuffer.tSize.nY; p != e; ++p, ++pD)
					*pD = clrs[*p];
				cBuffer.nStride = cBuffer.tSize.nX;
				bBW = false; // palette is not technically grayscale
			}
			else if (bBW)
			{
				if (bHasAlpha)
					cBuffer.ToLA();
				else
					cBuffer.ToL();

				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, 8,
					bHasAlpha ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY,
					cInterlacing.operator bool() ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
			}
			else
			{
				png_set_IHDR(png_ptr, info_ptr, nSizeX, nSizeY, 8,
					bHasAlpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
					cInterlacing.operator bool() ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
			}

			if (cExtraAttr.operator bool())
			{
				//if (cBuffer.fGamma >= 0.1f && cBuffer.fGamma <= 10.0f && fabsf(cBuffer.fGamma-2.2f) > 1e-3f)
				//{
				//	png_set_gAMA(png_ptr, info_ptr, 1.0/cBuffer.fGamma);
				//}
				if (cBuffer.tResolution.nNumeratorX && cBuffer.tResolution.nDenominatorX && cBuffer.tResolution.nNumeratorY && cBuffer.tResolution.nDenominatorY)
				{
					png_set_pHYs(png_ptr, info_ptr, cBuffer.tResolution.nNumeratorX*10000ULL/cBuffer.tResolution.nDenominatorX, cBuffer.tResolution.nNumeratorY*10000ULL/cBuffer.tResolution.nDenominatorY, 1);
				}
				if (cDef.bA == 255 && !bHasAlpha)
				{
					png_color_16 tBG;
					tBG.index = 0;
					tBG.red = cDef.bR;
					tBG.green = cDef.bG;
					tBG.blue = cDef.bB;
					tBG.gray = (ULONG(cDef.bR)+ULONG(cDef.bG)+ULONG(cDef.bB))/3;
					png_set_bKGD(png_ptr, info_ptr, &tBG);
				}
			}

			png_write_info(png_ptr, info_ptr);
			if (!bBW)
			{
				if (!bHasAlpha)
					png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
				png_set_bgr(png_ptr);
			}
			if (clrs.size() <= 16)
				png_set_packing(png_ptr); // ignored if not paletted image

			size_t nB = cBuffer.nStride;
			for (ULONG i = 0; i < nSizeY; ++i)
				apRows[i] = pB+nB*i;

			png_write_image(png_ptr, apRows);
			png_write_end(png_ptr, info_ptr);
		}
		catch (...)
		{
			hRes = E_UNEXPECTED;
		}

		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
	catch (...)
	{
		hRes = a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}

	return hRes;
}

