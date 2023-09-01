// DocumentOperationWatermark.cpp : Implementation of CDocumentOperationWatermark

#include "stdafx.h"
#include "DocumentOperationWatermark.h"
#include <SharedStringTable.h>
#include <RWEXIF_i.h>
#include <RWViewImageRaster.h>
#include <DocumentName.h>
#include <RWDocumentAnimation.h>
#include <RWDocumentImageVector.h>
#include "../RWOperationImageRaster/RWOperationImageRaster.h"


struct CEXIFExtractor
{
	CEXIFExtractor(IImageMetaData* a_pIMD) : pIMD(a_pIMD), bRead(true) {}
	void operator()(wchar_t const* name, size_t length, CComBSTR& value)
	{
		if (bRead)
		{
			bRead = false;
			ULONG nLen = 0;
			if (pIMD)
				pIMD->GetBlockSize(CComBSTR(L"EXIF"), &nLen);
			if (nLen)
			{
				CAutoVectorPtr<BYTE> pExifData(new BYTE[nLen]);
				pIMD->GetBlock(CComBSTR(L"EXIF"), nLen, pExifData);
				RWCoCreateInstance(pExif, __uuidof(RWEXIF));
				if (pExif)
				{
					if (FAILED(pExif->Load(nLen, pExifData)))
						pExif = NULL;
				}
			}
		}
		if (pExif)
		{
			WORD nTag = 0xffff;
			WORD nIFD = 0xffff;
			CComBSTR bstrTag;
			bstrTag.Attach(SysAllocStringLen(name, length));
			pExif->TagFindByName(bstrTag, &nIFD, &nTag);
			pExif->TagGetAsText(nIFD, nTag, &value);
		}
	}

private:
	IImageMetaData* pIMD;
	bool bRead;
	CComPtr<IRWEXIF> pExif;
};

void EXIFCopyName(wchar_t const* name, size_t length, CComBSTR& value)
{
	value.Attach(SysAllocStringLen(NULL, length+2));
	value[0] = L'<';
	value[length+1] = L'>';
	wcsncpy(value.m_str+1, name, length);
}

template<typename TEXIFHandler>
void ProcessTemplate(wchar_t const* pStr, wchar_t const* name, wchar_t const* folder, wchar_t const* path, wchar_t const* ext, TEXIFHandler& exifHandler, std::wstring& str)
{
		while (*pStr)
		{
			if (*pStr == L'%')
			{
				if (pStr[1] == L'%')
				{
					str.append(1, L'%');
					pStr += 2;
					continue;
				}
				else if (wcsncmp(pStr, L"%FILENAME%", 10) == 0)
				{
					pStr += 10;
					wchar_t const* p = name;
					while (*p && p+1 != ext)
					{
						str.append(1, *p);
						++p;
					}
					continue;
				}
				else if (wcsncmp(pStr, L"%FILEFOLDER%", 12) == 0)
				{
					pStr += 12;
					wchar_t const* p = folder;
					while (*p && p != name)
					{
						str.append(1, *p);
						++p;
					}
					continue;
				}
				else if (wcsncmp(pStr, L"%FILEEXT%", 9) == 0)
				{
					pStr += 9;
					str.append(ext);
					continue;
				}
				else if (wcsncmp(pStr, L"%FILEPATH%", 10) == 0)
				{
					pStr += 10;
					str.append(path);
					continue;
				}
				else if (wcsncmp(pStr, L"%EXIF-", 6) == 0)
				{
					wchar_t const* pTag = pStr + 6;
					wchar_t const* pTmp = pStr + 6;
					while ((*pTmp >= L'0' && *pTmp <= L'9') || (*pTmp >= L'a' && *pTmp <= L'z') || (*pTmp >= L'A' && *pTmp <= L'Z'))
						++pTmp;
					if (*pTmp == L'%' && pTmp > pStr+6)
					{
						pStr = pTmp+1;
						CComBSTR bstrVal;
						exifHandler(pTag, pTmp-pTag, bstrVal);
						if (bstrVal.m_str)
							str.append(bstrVal);
						pStr = pTmp+1;
						continue;
					}
				}
			}
			str.append(1, *pStr);
			++pStr;
		}
}

// CDocumentOperationWatermark

STDMETHODIMP CDocumentOperationWatermark::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_DOCOPWATERMARK_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_WM_XPOSITION[] = L"PosX";
static OLECHAR const CFGID_WM_YPOSITION[] = L"PosY";
static OLECHAR const CFGID_WM_MARGINS[] = L"Margins";
static OLECHAR const CFGID_WM_OPACITY[] = L"Opacity";
static OLECHAR const CFGID_WM_TYPE[] = L"Type";
static LONG const CFGID_WMT_TEXT = 0;
static LONG const CFGID_WMT_IMAGE = 1;
static OLECHAR const CFGID_WM_IMAGE[] = L"Image";
static OLECHAR const CFGID_WM_FILTER[] = L"Filter";
static LONG const CFGID_WMF_NONE = 0;
static LONG const CFGID_WMF_GRAYSCALE = 1;
static LONG const CFGID_WMF_EMBOSS = 2;
static OLECHAR const CFGID_WM_IMGSIZETYPE[] = L"ImgSizeType";
static LONG const CFGID_WMIT_ORIGINAL = 0;
static LONG const CFGID_WMIT_RELATIVE = 1;
static OLECHAR const CFGID_WM_IMGRELSIZE[] = L"ImgRelSize";
static OLECHAR const CFGID_WM_TILED[] = L"Tiled";
static OLECHAR const CFGID_WM_TEXT[] = L"Text";
static OLECHAR const CFGID_WM_FONT[] = L"Font";
static OLECHAR const CFGID_WM_COLOR[] = L"Color";
static OLECHAR const CFGID_WM_BOLD[] = L"Bold";
static OLECHAR const CFGID_WM_ITALIC[] = L"Italic";
static OLECHAR const CFGID_WM_SHADOW[] = L"Shadow";
static OLECHAR const CFGID_WM_ANGLE[] = L"Angle";
static OLECHAR const CFGID_WM_ALIGNMENT[] = L"Alignment";
static LONG const CFGID_WMA_LEFT = 0;
static LONG const CFGID_WMA_CENTER = 1;
static LONG const CFGID_WMA_RIGHT = 2;
static OLECHAR const CFGID_WM_SIZETYPE[] = L"SizeType";
static LONG const CFGID_WMH_ABSOLUTE = 0;
static LONG const CFGID_WMH_RELATIVE = 1;
static OLECHAR const CFGID_WM_ABSSIZE[] = L"AbsSize";
static OLECHAR const CFGID_WM_RELSIZE[] = L"RelSize";

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

static bool GetWord(std::wstring& orig, std::wstring& word)
{
	std::wstring::iterator i = orig.begin();
	while (i != orig.end() && (*i == L' ' || *i == L'\n' || *i == L'\r' || *i == L'\t')) ++i;
	std::wstring::iterator i0 = i;
	while (i != orig.end() && *i != L' ' && *i != L'\n' && *i != L'\r' && *i != L'\t') ++i;
	if (i == i0)
		return false;
	std::wstring w(i0, i);
	std::swap(w, word);
	std::wstring o(i, orig.end());
	std::swap(o, orig);
	return true;
}

STDMETHODIMP CDocumentOperationWatermark::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Watermark[0405]Vodotisk";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_TYPE), &cType);
		if (cType.operator LONG() == CFGID_WMT_TEXT)
		{
			CConfigValue cTemplate;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_TEXT), &cTemplate);
			if (cTemplate.operator BSTR() == NULL || cTemplate.operator BSTR()[0] == L'\0')
			{
				*a_ppName = new CMultiLanguageString(L"[0409]Text watermark[0405]Textový vodotisk");
				return S_OK;
			}
			CConfigValue cFont;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_FONT), &cFont);
			std::wstring str;
			ProcessTemplate(cTemplate, L"filename", L"C:\\", L"C:\\filename.ext", L"ext", EXIFCopyName, str);
			std::wstring data(L"\"");
			std::wstring word;
			for (int i = 0; GetWord(str, word); ++i)
			{
				if (word.length() + data.length() > 20)
				{
					if (data.length() < 17)
					{
						if (i)
							data.append(L" ");
						data.append(word.substr(0, 18-data.length()));
					}
					data.append(L"...");
					break;
				}
				else
				{
					if (i)
						data.append(L" ");
					data.append(word);
				}
			}
			data.append(L"\" ");
			data.append(cFont);
			CConfigValue cSizeType;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_SIZETYPE), &cSizeType);
			if (cSizeType.operator LONG() == CFGID_WMH_ABSOLUTE)
			{
				CConfigValue cAbsSize;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_ABSSIZE), &cAbsSize);
		
				data.append(L" %i%s");
				CComBSTR bstr(data.c_str());
				CComPtr<ILocalizedString> pTempl;
				pTempl.Attach(new CSimpleLocalizedString(bstr.Detach()));

				CComObject<CPrintfLocalizedString>* pStr = NULL;
				CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
				CComPtr<ILocalizedString> pTmp = pStr;
				pStr->Init(pTempl, cAbsSize.operator LONG(), CMultiLanguageString::GetAuto(L"[0409]px[0405]px"));
				*a_ppName = pTmp.Detach();
				return S_OK;
			}

			CConfigValue cRelSize;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_RELSIZE), &cRelSize);
			wchar_t szTmp[32];
			_swprintf(szTmp, L" %g%%", cRelSize.operator float());
			data.append(szTmp);

			CComBSTR bstr(data.c_str());
			*a_ppName = new CSimpleLocalizedString(bstr.Detach());
			return S_OK;
		}

		CConfigValue cFile;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_IMAGE), &cFile);
		LPWSTR pFull = cFile;
		LPWSTR p = wcsrchr(pFull, L'\\');
		LPWSTR p2 = wcsrchr(pFull, L'/');
		if (p2 > p) p = p2;
		if (p == NULL) p = pFull; else ++p;
		if (*p)
		{
			CConfigValue cOpacity;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_OPACITY), &cOpacity);
			if (wcslen(p) > 16)
			{
				p[14] = p[15] = p[16] = L'.';
				p[17] = L'\0';
			}
			CComBSTR bstr(p);
			wchar_t sz[32];
			_swprintf(sz, L" - %i%%", cOpacity.operator LONG());
			bstr += sz;
			*a_ppName = new CSimpleLocalizedString(bstr.Detach());
			return S_OK;
		}

		*a_ppName = new CMultiLanguageString(L"[0409]Image watermark[0405]Obrázkový vodotisk");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include "ConfigGUIWatermark.h"

STDMETHODIMP CDocumentOperationWatermark::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsRanged(CComBSTR(CFGID_WM_XPOSITION), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_XPOSITION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_XPOSITION_DESC), CConfigValue(100L), NULL, CConfigValue(0L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_WM_YPOSITION), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_YPOSITION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_YPOSITION_DESC), CConfigValue(100L), NULL, CConfigValue(0L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_MARGINS), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_MARGINS_NAME), CMultiLanguageString::GetAuto(L"[0409]Extra margins around the watermark.[0405]Přidate okraje okolo vodotisku."), CConfigValue(-1L), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_WM_ANGLE), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ANGLE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ANGLE_DESC), CConfigValue(0L), NULL, CConfigValue(-180L), CConfigValue(180L), CConfigValue(1L), 0, NULL);
		CComBSTR cCFGID_WM_TYPE(CFGID_WM_TYPE);
		pCfgInit->ItemIns1ofN(cCFGID_WM_TYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_WM_TYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_TYPE_DESC), CConfigValue(CFGID_WMT_TEXT), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_TYPE, CConfigValue(CFGID_WMT_TEXT), CMultiLanguageString::GetAuto(L"[0409]Text[0405]Text"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_TYPE, CConfigValue(CFGID_WMT_IMAGE), CMultiLanguageString::GetAuto(L"[0409]Image[0405]Obrázek"), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_OPACITY), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_OPACITY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_OPACITY_DESC), CConfigValue(100L), NULL, 0, NULL);

		TConfigOptionCondition aConds[2];
		aConds[0].bstrID = cCFGID_WM_TYPE;
		aConds[0].eConditionType = ECOCEqual;
		aConds[0].tValue.eTypeID = ECVTInteger;
		aConds[0].tValue.iVal = CFGID_WMT_IMAGE;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_IMAGE), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_IMAGE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_IMAGE_DESC), CConfigValue(L""), NULL, 1, aConds);
		CComBSTR cCFGID_WM_FILTER(CFGID_WM_FILTER);
		pCfgInit->ItemIns1ofN(cCFGID_WM_FILTER, CMultiLanguageString::GetAuto(L"[0409]Effect[0405]Efekt"), CMultiLanguageString::GetAuto(L"[0409]Filter applied to the watermark image.[0405]Filtr použitý na obrázek vodotisku."), CConfigValue(CFGID_WMF_NONE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_FILTER, CConfigValue(CFGID_WMF_NONE), CMultiLanguageString::GetAuto(L"[0409]None[0405]Žádný"), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_FILTER, CConfigValue(CFGID_WMF_GRAYSCALE), CMultiLanguageString::GetAuto(L"[0409]Grayscale[0405]Šedotón"), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_FILTER, CConfigValue(CFGID_WMF_EMBOSS), CMultiLanguageString::GetAuto(L"[0409]Emboss[0405]Plastika"), 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_TILED), CMultiLanguageString::GetAuto(L"[0409]Tiled[0405]Dlaždice"), CMultiLanguageString::GetAuto(L"[0409]Repeat the watermark over the whole image.[0405]Opakovat vodotisk po celém obrázku."), CConfigValue(false), NULL, 1, aConds);
		CComBSTR cCFGID_WM_IMGSIZETYPE(CFGID_WM_IMGSIZETYPE);
		pCfgInit->ItemIns1ofN(cCFGID_WM_IMGSIZETYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_WM_SIZETYPE_NAME), CMultiLanguageString::GetAuto(L"[0409]Controls how the size of the watermark is computed.[0405]Určuje, jakým způsobem bude vypočítána velikost vodotisku."), CConfigValue(CFGID_WMIT_ORIGINAL), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_IMGSIZETYPE, CConfigValue(CFGID_WMIT_ORIGINAL), CMultiLanguageString::GetAuto(L"[0409]Original[0405]Původní"), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_IMGSIZETYPE, CConfigValue(CFGID_WMIT_RELATIVE), CMultiLanguageString::GetAuto(L"[0409]Relative[0405]Relativní"), 1, aConds);
		aConds[1].bstrID = cCFGID_WM_IMGSIZETYPE;
		aConds[1].eConditionType = ECOCEqual;
		aConds[1].tValue.eTypeID = ECVTInteger;
		aConds[1].tValue.iVal = CFGID_WMIT_RELATIVE;
		CComPtr<ILocalizedString> pSize;
		pSize.Attach(new CMultiLanguageString(L"[0409]Tiled[0405]Dlaždice"));
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_IMGRELSIZE), pSize, CMultiLanguageString::GetAuto(L"[0409]Watermark size relative to the processed image size.[0405]Velikost vodotisku vztažená k velikosti zpracovávaného obrázku."), CConfigValue(20.0f), NULL, 2, aConds);

		aConds[0].tValue.iVal = CFGID_WMT_TEXT;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_TEXT), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_TEXT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_TEXT_DESC), CConfigValue(L"%FILENAME%"), NULL, 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_FONT), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_FONT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_FONT_DESC), CConfigValue(L"Arial"), NULL, 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_COLOR), _SharedStringTable.GetStringAuto(IDS_CFGID_COLOR_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_COLOR_DESC), CConfigValue((LONG)0x68e0e8), NULL, 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_BOLD), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_BOLD_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_BOLD_DESC), CConfigValue(false), NULL, 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_ITALIC), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ITALIC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ITALIC_DESC), CConfigValue(false), NULL, 1, aConds);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_SHADOW), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_SHADOW_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_SHADOW_DESC), CConfigValue(false), NULL, 1, aConds);
		CComBSTR cCFGID_WM_ALIGNMENT(CFGID_WM_ALIGNMENT);
		pCfgInit->ItemIns1ofN(cCFGID_WM_ALIGNMENT, _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ALIGNMENT_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ALIGNMENT_DESC), CConfigValue(CFGID_WMA_RIGHT), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_ALIGNMENT, CConfigValue(CFGID_WMA_LEFT), _SharedStringTable.GetStringAuto(IDS_CFGID_WMA_LEFT), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_ALIGNMENT, CConfigValue(CFGID_WMA_CENTER), _SharedStringTable.GetStringAuto(IDS_CFGID_WMA_CENTER), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_ALIGNMENT, CConfigValue(CFGID_WMA_RIGHT), _SharedStringTable.GetStringAuto(IDS_CFGID_WMA_RIGHT), 1, aConds);
		CComBSTR cCFGID_WM_SIZETYPE(CFGID_WM_SIZETYPE);
		pCfgInit->ItemIns1ofN(cCFGID_WM_SIZETYPE, _SharedStringTable.GetStringAuto(IDS_CFGID_WM_SIZETYPE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_SIZETYPE_DESC), CConfigValue(CFGID_WMH_ABSOLUTE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_WM_SIZETYPE, CConfigValue(CFGID_WMH_ABSOLUTE), _SharedStringTable.GetStringAuto(IDS_CFGID_WMH_ABSOLUTE), 1, aConds);
		pCfgInit->ItemOptionAdd(cCFGID_WM_SIZETYPE, CConfigValue(CFGID_WMH_RELATIVE), _SharedStringTable.GetStringAuto(IDS_CFGID_WMH_RELATIVE), 1, aConds);
		aConds[1].bstrID = cCFGID_WM_SIZETYPE;
		aConds[1].eConditionType = ECOCEqual;
		aConds[1].tValue.eTypeID = ECVTInteger;
		aConds[1].tValue.iVal = CFGID_WMH_ABSOLUTE;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_ABSSIZE), pSize, _SharedStringTable.GetStringAuto(IDS_CFGID_WM_ABSSIZE_DESC), CConfigValue(16L), NULL, 2, aConds);
		aConds[1].tValue.iVal = CFGID_WMH_RELATIVE;
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_WM_RELSIZE), pSize, CMultiLanguageString::GetAuto(L"[0409]Watermark size relative to the processed image size.[0405]Velikost vodotisku vztažená k velikosti zpracovávaného obrázku."), CConfigValue(10.0f), NULL, 2, aConds);

		//pCfgInit->Finalize(NULL);
		CConfigCustomGUI<&CLSID_DocumentOperationWatermark, CConfigGUIWatermark>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationWatermark::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFtsRas[] = {__uuidof(IDocumentRasterImage)};
		static IID const aFtsLay[] = {__uuidof(IDocumentLayeredImage)};
		static IID const aFtsAni[] = {__uuidof(IDocumentAnimation)};
		return SupportsAllFeatures(a_pDocument, 1, aFtsRas) || SupportsAllFeatures(a_pDocument, 1, aFtsLay) || SupportsAllFeatures(a_pDocument, 1, aFtsAni) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_pixfmt_gray.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_font_win32_tt.h>
#include <agg_blur.h>

static RECT const RECT_EMPTY = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
class CEditToolScanlineBuffer
{
public:
	typedef agg::rgba8 color_type;
	typedef void row_data;
	typedef void span_data;

	CEditToolScanlineBuffer() : m_rcDirty(RECT_EMPTY)
	{
	}
	~CEditToolScanlineBuffer()
	{
		DeleteCachedData();
	}

	RECT const& M_DirtyRect() const
	{
		return m_rcDirty;
	}
	void DeleteCachedData()
	{
		m_rcDirty = RECT_EMPTY;
		m_cSolidLines.clear();
		for (CSolidSpans::iterator i = m_cSolidSpans.begin(); i != m_cSolidSpans.end(); ++i)
			delete[] i->pCovers;
		m_cSolidSpans.clear();
	}
	void GetCoverageData(RECT const& a_rcCoords, TPixelChannel* a_pBuffer) const
	{
		if (a_rcCoords.bottom <= a_rcCoords.top || a_rcCoords.right <= a_rcCoords.left)
			return;

		for (CSolidLines::const_iterator i = m_cSolidLines.begin(); i != m_cSolidLines.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TPixelChannel const tColor = {i->c.r, i->c.g, i->c.b, i->c.a};
			TPixelChannel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*(a_rcCoords.right-a_rcCoords.left);
			TPixelChannel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			int const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TPixelChannel* p = pLine+nOff;
			while (p < pEnd)
			{
				p->bA = i->cover + (((256-i->cover)*p->bA)>>8);
				++p;
			}
		}

		for (CSolidSpans::const_iterator i = m_cSolidSpans.begin(); i != m_cSolidSpans.end(); ++i)
		{
			if (i->y < a_rcCoords.top || i->y >= a_rcCoords.bottom ||
				i->x >= a_rcCoords.right || LONG(i->x+i->n) <= a_rcCoords.left)
				continue;
			TPixelChannel* const pLine = a_pBuffer + (i->y-a_rcCoords.top)*(a_rcCoords.right-a_rcCoords.left);
			TPixelChannel* const pEnd = pLine + min(LONG(i->x+i->n), a_rcCoords.right) - a_rcCoords.left;
			int const nOff = i->x >= a_rcCoords.left ? i->x-a_rcCoords.left : 0;
			TPixelChannel* p = pLine+nOff;
			BYTE* pCover = i->pCovers + a_rcCoords.left-i->x+nOff;
			while (p < pEnd)
			{
				p->bA = *pCover + (((256-*pCover)*p->bA)>>8);
				++p;
				++pCover;
			}
		}
	}

	// AGG image target methods
public:
    //unsigned width() const { return m_nSizeX; }
    //unsigned height() const { return m_nSizeY; }

	void blend_pixel(int x, int y, const color_type& c, agg::int8u cover)
	{
		SSolidLine sLine = {x, y, 1, c, cover};
		m_cSolidLines.push_back(sLine);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right <= x) m_rcDirty.right = x+1;
	}
	// IMPORTANT: watcho out for the x_end vs. unsigned len in base_renderer
	void blend_hline(int x, int y, int x_end, color_type const& c, agg::int8u cover)
	{
		SSolidLine sLine = {x, y, x_end-x+1, c, cover};
		m_cSolidLines.push_back(sLine);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right <= x_end) m_rcDirty.right = x_end+1;
	}
	void blend_solid_hspan(int x, int y, unsigned len, color_type const& c, agg::int8u const* covers)
	{
		SSolidSpan sItem = {x, y, len, c, new BYTE[len]};
		CopyMemory(sItem.pCovers, covers, len*sizeof *sItem.pCovers);
		m_cSolidSpans.push_back(sItem);
		if (m_rcDirty.top > y) m_rcDirty.top = y;
		if (m_rcDirty.bottom <= y) m_rcDirty.bottom = y+1;
		if (m_rcDirty.left > x) m_rcDirty.left = x;
		if (m_rcDirty.right < LONG(x+len)) m_rcDirty.right = x+len;
	}

private:
	struct SSolidLine
	{
		int x;
		int y;
		unsigned n;
		color_type c;
		BYTE cover;
	};
	typedef std::vector<SSolidLine> CSolidLines;

	struct SSolidSpan
	{
		int x;
		int y;
		unsigned n;
		color_type c;
		BYTE* pCovers;
	};
	typedef std::vector<SSolidSpan> CSolidSpans;

private:
	CSolidLines m_cSolidLines;
	CSolidSpans m_cSolidSpans;
	RECT m_rcDirty;
};

static int const nGammaBits = 10;
HRESULT ProcessFrame(IDocument* a_pDocument, IConfig* a_pConfig, LCID a_tLocaleID, BYTE const* pInvGamma, WORD const* pGamma, BSTR bstrPath, CEXIFExtractor& cEXIF)
{
	CComPtr<IDocumentRasterImage> pRI;
	a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
	CComPtr<IDocumentLayeredImage> pLI;
	CComPtr<IDocumentImage> pI;
	if (pRI == NULL)
	{
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pLI));
		if (pLI == NULL)
			return E_FAIL;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		if (pI == NULL)
			return E_FAIL;
	}
	else
	{
		pI = pRI;
	}

	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_XPOSITION), &cVal);
	LONG nPosX = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_YPOSITION), &cVal);
	LONG nPosY = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_MARGINS), &cVal);
	LONG nMargins = cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_TYPE), &cVal);
	LONG nType = cVal;
	CConfigValue cImagePath;
	CConfigValue cTemplate;
	CConfigValue cFont;
	LONG clrText = 0;
	bool bBold = false;
	bool bItalic = false;
	bool bShadow = false;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_ANGLE), &cVal);
	LONG nAngle = cVal;
	LONG nSizeType = CFGID_WMH_ABSOLUTE;
	LONG nAbsSize = 10;
	LONG eAlign = CFGID_WMA_RIGHT;
	float fRelSize = 20.0f;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_OPACITY), &cVal);
	LONG nOpacity = cVal;
	LONG nEffect = CFGID_WMF_NONE;
	bool bTiled = false;
	if (nType == CFGID_WMT_IMAGE)
	{
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_FILTER), &cVal);
		nEffect = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_TILED), &cVal);
		bTiled = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_IMGSIZETYPE), &cVal);
		nSizeType = cVal;
		if (nSizeType == CFGID_WMIT_RELATIVE)
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_IMGRELSIZE), &cVal);
			fRelSize = cVal;
		}
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_IMAGE), &cImagePath);
	}
	else
	{
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_TEXT), &cTemplate);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_FONT), &cFont);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_COLOR), &cVal);
		clrText = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_BOLD), &cVal);
		bBold = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_ITALIC), &cVal);
		bItalic = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_SHADOW), &cVal);
		bShadow = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_ALIGNMENT), &cVal);
		eAlign = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_SIZETYPE), &cVal);
		nSizeType = cVal;
		if (nSizeType == CFGID_WMH_ABSOLUTE)
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_ABSSIZE), &cVal);
			nAbsSize = cVal;
		}
		else
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_WM_RELSIZE), &cVal);
			fRelSize = cVal;
		}
	}

	CWriteLock<IDocument> cLock(a_pDocument);

	TImageSize tSize = {1, 1};
	pI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

	ULONG nPatSizeX = 0;
	ULONG nPatSizeY = 0;
	CAutoVectorPtr<TPixelChannel> cPattern;
	if (nType == CFGID_WMT_IMAGE)
	{
		// image watermark
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		CComPtr<IEnumUnknowns> pBuilders;
		pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
		CComPtr<IDocument> pImgDoc;
		RWCoCreateInstance(pImgDoc, __uuidof(DocumentBase));
		CComPtr<IDocumentImage> pPatImg;
		if (SUCCEEDED(pIM->DocumentCreateData(pBuilders, CStorageFilter(cImagePath.operator BSTR()), NULL, CComQIPtr<IDocumentBase>(pImgDoc))))
			pImgDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pPatImg));
		if (pPatImg == NULL)
		{
			// invalid file - use default copyright sign
			//DrawTool.SHAPE(Document, "C", 220, 110, 0, 60.7513, 0, -60.7513, "C", 110, 220, -60.7513, 0, 60.7513, 0, "C", 0, 110, 0, -60.7513, 0, 60.7513, "E", 110, 0, 60.7513, 0, -60.7513, 0, "C", 110, 21, 49.1533, 0, -49.1533, 0, "C", 199, 110, 0, 49.1533, 0, -49.1533, "C", 110, 199, -49.1533, 0, 49.1533, 0, "E", 21, 110, 0, -49.1533, 0, 49.1533, "C", 171, 94, 0, 0, -7.07901, -27.0472, "C", 141, 94, -5.81067, -11.2801, 0, 0, "C", 110, 75, -19.33, 0, 13.5626, 0, "C", 75, 110, 0, 19.33, 0, -19.33, "C", 110, 145, 13.5626, 0, -19.33, 0, "C", 141, 126, 0, 0, -5.81067, 11.2801, "C", 171, 126, -7.07901, 27.0472, 0, 0, "C", 110, 173, -34.7939, 0, 29.2667, 0, "C", 47, 110, 0, -34.7939, 0, 34.7939, "E", 110, 47, 29.2667, 0, -34.7939, 0);
			pImgDoc = NULL;
			RWCoCreateInstance(pImgDoc, __uuidof(DocumentBase));
			CComPtr<IDocumentFactoryVectorImage> pDFVI;
			RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryVectorImage));
			if (pDFVI)
			{
				static float const aBackground[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				pDFVI->Create(NULL, CComQIPtr<IDocumentBase>(pImgDoc), CImageSize(220, 220), NULL, aBackground);
				CComBSTR bstrParams(L"\"C\", 220, 110, 0, 60.7513, 0, -60.7513, \"C\", 110, 220, -60.7513, 0, 60.7513, 0, \"C\", 0, 110, 0, -60.7513, 0, 60.7513, \"E\", 110, 0, 60.7513, 0, -60.7513, 0, \"C\", 110, 21, 49.1533, 0, -49.1533, 0, \"C\", 199, 110, 0, 49.1533, 0, -49.1533, \"C\", 110, 199, -49.1533, 0, 49.1533, 0, \"E\", 21, 110, 0, -49.1533, 0, 49.1533, \"C\", 171, 94, 0, 0, -7.07901, -27.0472, \"C\", 141, 94, -5.81067, -11.2801, 0, 0, \"C\", 110, 75, -19.33, 0, 13.5626, 0, \"C\", 75, 110, 0, 19.33, 0, -19.33, \"C\", 110, 145, 13.5626, 0, -19.33, 0, \"C\", 141, 126, 0, 0, -5.81067, 11.2801, \"C\", 171, 126, -7.07901, 27.0472, 0, 0, \"C\", 110, 173, -34.7939, 0, 29.2667, 0, \"C\", 47, 110, 0, -34.7939, 0, 34.7939, \"E\", 110, 47, 29.2667, 0, -34.7939, 0");
				pDFVI->AddObject(NULL, CComQIPtr<IDocumentBase>(pImgDoc), NULL, CComBSTR(L"SHAPE"), bstrParams, CComBSTR(L"SOLID"), CComBSTR(L"0, 0, 0, 1"), NULL, NULL, NULL, NULL, NULL, NULL, NULL);
				pImgDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pPatImg));
			}
		}
		if (pPatImg)
		{
			TImageSize tOrigSize = {1, 1};
			pPatImg->CanvasGet(&tOrigSize, NULL, NULL, NULL, NULL);
			if (nAngle)
			{
				CComPtr<IDocumentOperation> pRot;
				RWCoCreateInstance(pRot, __uuidof(DocumentOperationRasterImageRotate));
				CComPtr<IConfig> pRotCfg;
				pRot->ConfigCreate(NULL, &pRotCfg);
				CComBSTR bstrAngle(L"Angle");
				CConfigValue cAngle(static_cast<float>(nAngle));
				pRotCfg->ItemValuesSet(1, &(bstrAngle.m_str), cAngle);
				pRot->Activate(NULL, pImgDoc, pRotCfg, NULL, NULL, 0);
			}
			TImageSize tPatSize = {1, 1};
			pPatImg->CanvasGet(&tPatSize, NULL, NULL, NULL, NULL);
			if (nSizeType == CFGID_WMIT_RELATIVE && fRelSize > 0.0f && fRelSize <= 100.0f)
			{
				double orig = sqrt(double(tSize.nX)*tSize.nY);
				double pat = sqrt(double(tOrigSize.nX)*tOrigSize.nY);
				double ratio = 100.0*pat/orig;
				nPatSizeX = tPatSize.nX*fRelSize/ratio+0.5;
				nPatSizeY = tPatSize.nY*fRelSize/ratio+0.5;
			}
			if (nPatSizeX == 0) nPatSizeX = 1;
			if (nPatSizeY == 0) nPatSizeY = 1;
			cPattern.Allocate(nPatSizeX*nPatSizeY);
			if (nPatSizeX != tPatSize.nX && nPatSizeY != tPatSize.nY)
			{
				CComPtr<IDocumentOperation> rescale;
				RWCoCreateInstance(rescale, __uuidof(DocumentOperationRasterImageResample));
				CComPtr<IConfig> cfg;
				rescale->ConfigCreate(NULL, &cfg);
				CComBSTR strMode(L"Mode");
				CComBSTR strMethod(L"Method");
				CComBSTR strSizeX(L"SizeX");
				CComBSTR strSizeY(L"SizeY");
				BSTR aIDs[4] = {strMode, strSizeX, strSizeY, strMethod};
				TConfigValue aVals[4];
				aVals[0] = CConfigValue(10L);
				aVals[1] = CConfigValue(LONG(nPatSizeX));
				aVals[2] = CConfigValue(LONG(nPatSizeY));
				aVals[3] = CConfigValue(1L);
				cfg->ItemValuesSet(4, aIDs, aVals);
				rescale->Activate(NULL, pImgDoc, cfg, NULL, NULL, 0);
			}
			pPatImg->TileGet(EICIRGBA, NULL, NULL, NULL, nPatSizeX*nPatSizeY, reinterpret_cast<TPixelChannel*>(cPattern.m_p), NULL, EIRIAccurate);

			if (nEffect == CFGID_WMF_GRAYSCALE)
			{
				TPixelChannel* p = cPattern;
				for (TPixelChannel* pEnd = p+nPatSizeX*nPatSizeY; p != pEnd; ++p)
					p->bR = p->bG = p->bB = (ULONG(p->bR)+ULONG(p->bG)+ULONG(p->bB)+1)/3;
			}
			else if (nEffect == CFGID_WMF_EMBOSS)
			{
				CComPtr<IDocumentOperation> pBev;
				RWCoCreateInstance(pBev, __uuidof(RasterImageBevel));
				if (pBev)
				{
					CComPtr<IConfig> pCfg;
					pBev->ConfigCreate(NULL, &pCfg);
					CComBSTR id1(L"Method");
					CComBSTR id2(L"Amount");
					CComBSTR id3(L"Blur");
					CComBSTR id4(L"Output");
					BSTR aIDs[4] = {id1, id2, id3, id4};
					TConfigValue aVals[4];
					aVals[0].eTypeID = aVals[1].eTypeID = aVals[2].eTypeID = aVals[3].eTypeID = ECVTInteger;
					aVals[0].iVal = -1;
					aVals[1].iVal = sqrtf(nPatSizeX*nPatSizeY)*0.05f;
					if (aVals[1].iVal < 3) aVals[1].iVal = 3;
					aVals[2].iVal = aVals[1].iVal/5;
					aVals[3].iVal = 1;
					pCfg->ItemValuesSet(4, aIDs, aVals);
					CComPtr<IDocument> pDoc;
					RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
					CComQIPtr<IDocumentFactoryRasterImage> pRIF;
					RWCoCreateInstance(pRIF, __uuidof(DocumentFactoryRasterImage));
					pRIF->Create(NULL, CComQIPtr<IDocumentBase>(pDoc), CImageSize(nPatSizeX, nPatSizeY), NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 2.2f, CImageTile(nPatSizeX, nPatSizeY, cPattern));
					pBev->Activate(NULL, pDoc, pCfg, NULL, NULL, 0);
					CComPtr<IDocumentImage> pI;
					pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
					pI->TileGet(EICIRGBA, NULL, NULL, NULL, nPatSizeX*nPatSizeY, reinterpret_cast<TPixelChannel*>(cPattern.m_p), NULL, EIRIAccurate);
					// convert normal map to light and dark areas
					TPixelChannel* p = cPattern;
					for (TPixelChannel* pEnd = p+nPatSizeX*nPatSizeY; p != pEnd; ++p)
					{
						if (p->bA == 0)
							continue;
						LONG nX = p->bR-128;
						LONG nY = p->bG-128;
						LONG nZ = p->bB-128;
						LONG nVal = nX*(-73) + nY*(73) + nZ*(73);
						if (p->bB > 250)
							p->bA = 0;
						else if (p->bB < 184)
							p->bA = 255;
						else
							p->bA = (250-p->bB)<<2;
						p->bR = p->bG = p->bB = nVal < 0 ? 0 : nVal/(63);
					}
				}
			}
		}

		if (nMargins == -1)
			nMargins = sqrt(sqrt(double(nPatSizeX)*nPatSizeY)*0.03 * sqrt(double(tSize.nX)*tSize.nY)*0.03);
	}
	else
	{
		// text watermark
		wchar_t const* pName = NULL;
		wchar_t const* pDot = NULL;
		if (bstrPath)
		{
			pName = wcsrchr(bstrPath, L'\\');
			wchar_t const* pName2 = wcsrchr(bstrPath, L'/');
			if (pName < pName2) pName = pName2;
			if (pName)
				++pName;
			else
				pName = bstrPath;
			pDot = wcsrchr(pName, L'.');
		}
		
		// replace variables in input string
		std::wstring str;
		ProcessTemplate(cTemplate, pName ? pName : L"filename", bstrPath ? bstrPath : L"folder\\", bstrPath ? bstrPath : L"fullpath", pDot ? pDot+1 : L"ext", cEXIF, str);

		float fHeight = nSizeType == CFGID_WMH_ABSOLUTE ? nAbsSize : fRelSize*sqrtf(tSize.nX*tSize.nY)*0.002f;
		if (fHeight < 3.0f) fHeight = 3.0f;
		if (fHeight > sqrtf(tSize.nX*tSize.nY)) fHeight = sqrtf(tSize.nX*tSize.nY);

		CEditToolScanlineBuffer cBuffer;

		//agg::renderer_base<CEditToolScanlineBuffer> renb(cBuffer);
		//agg::renderer_scanline_aa_solid<agg::renderer_base<CEditToolScanlineBuffer> > ren(renb);
		agg::renderer_scanline_aa_solid<CEditToolScanlineBuffer> ren(cBuffer);
		agg::rasterizer_scanline_aa<agg::rasterizer_sl_no_clip> ras;
		agg::scanline_p8 sl;

		typedef agg::font_engine_win32_tt_int32 font_engine_type;
		typedef agg::font_cache_manager<font_engine_type> font_manager_type;
		typedef agg::conv_curve<font_manager_type::path_adaptor_type> conv_curve_type;
		//typedef agg::conv_contour<conv_curve_type> conv_contour_type;

		HDC hDC = GetDC(NULL);
		font_engine_type m_feng(hDC);
		font_manager_type m_fman(m_feng);
		conv_curve_type m_curves(m_fman.path_adaptor());
		//conv_contour_type m_contour;

		m_feng.hinting(true);
		m_feng.height(fHeight);
		m_feng.flip_y(true);
		//m_feng.width((m_width.value() == m_height.value()) ? 0.0 : m_width.value() / 2.4);
		m_feng.italic(bItalic);
		m_feng.weight(bBold ? FW_BOLD : FW_NORMAL);

		if (m_feng.create_font(COLE2T(cFont), agg::glyph_ren_outline))
		{
			if (nMargins == -1)
				nMargins = sqrt(fHeight*0.5 * sqrt(double(tSize.nX)*tSize.nY)*0.03);

			double x = 0.0;
			double y = 0.0;
			//double y0 = height() - m_height.value() - 10.0;
			CW2T strAnsi(str.c_str());

			std::vector<double> cLineLens;
			double fMaxLen = 0.0;

			const TCHAR* p = strAnsi;
			while (*p)
			{
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[1] == *p^(_T('\r')^_T('\n')))
						++p; 
					cLineLens.push_back(x);
					if (x > fMaxLen)
						fMaxLen = x;
					x = 0.0;
					continue;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
					x += glyph->advance_x;
				++p;
			}
			cLineLens.push_back(x);
			if (x > fMaxLen)
				fMaxLen = x;
			x = 0.0;

			agg::trans_affine mtx;
			mtx *= agg::trans_affine_rotation(agg::deg2rad(nAngle));
			m_feng.transform(mtx);
			m_fman.reset_cache();

			double xx = 1.0;
			double xy = 0.0;
			mtx.transform(&xx, &xy);
			double yx = 0.0;
			double yy = 1.3;
			mtx.transform(&yx, &yy);
			p = strAnsi;
			bool bLineSet = false;
			std::vector<double>::const_iterator iLine = cLineLens.begin();
			while (*p)
			{
				if (*p == _T('\r') || *p == _T('\n'))
				{
					++p;
					if (p[1] == *p^(_T('\r')^_T('\n')))
						++p;
					bLineSet = false;
					continue;
				}
				if (!bLineSet)
				{
					x = (iLine-cLineLens.begin())*yx*fHeight;
					y = (iLine-cLineLens.begin())*yy*fHeight;
					switch (eAlign)
					{
					case CFGID_WMA_CENTER:
						x += (fMaxLen-*iLine)*xx*0.5;
						y += (fMaxLen-*iLine)*xy*0.5;
						break;
					case CFGID_WMA_RIGHT:
						x += (fMaxLen-*iLine)*xx;
						y += (fMaxLen-*iLine)*xy;
						break;
					//case CFGID_WMA_LEFT:
					}
					bLineSet = true;
					++iLine;
				}
				const agg::glyph_cache* glyph = m_fman.glyph(*p);
				if (glyph)
				{
					m_fman.add_kerning(&x, &y);
					m_fman.init_embedded_adaptors(glyph, x, y);

					if(glyph->data_type == agg::glyph_data_outline)
					{
						ras.reset();
						ras.add_path(m_curves);
						//r.color(agg::rgba8(0, 0, 0));
						agg::render_scanlines(ras, sl, ren);
					}

					// increment pen position
					x += glyph->advance_x;
					y += glyph->advance_y;
				}
				++p;
			}
		}

		ReleaseDC(NULL, hDC);

		RECT rc = cBuffer.M_DirtyRect();
		if (rc.left >= rc.right && rc.top >= rc.bottom)
			return S_FALSE;
		int nShadow = bShadow ? 1+powf(fHeight*0.2f, 0.7f) : 0;
		nPatSizeX = rc.right-rc.left + nShadow*2;
		nPatSizeY = rc.bottom-rc.top + nShadow*2;
		rc.left -= nShadow;
		rc.top -= nShadow;
		rc.right += nShadow;
		rc.bottom += nShadow;
		cPattern.Allocate(nPatSizeX*nPatSizeY);
		std::fill_n((DWORD*)(cPattern.m_p), nPatSizeX*nPatSizeY, (0xff00&clrText)|((0xff&clrText)<<16)|((0xff0000&clrText)>>16));
		cBuffer.GetCoverageData(rc, cPattern);

		if (nShadow)
		{
			CAutoVectorPtr<BYTE> cShadow(new BYTE[nPatSizeX*nPatSizeY]);

			TPixelChannel* pS = cPattern.m_p;
			BYTE* pD = cShadow.m_p;
			for (TPixelChannel* const pEnd = cPattern.m_p+nPatSizeX*nPatSizeY; pS != pEnd; ++pS, ++pD)
			{
				*pD = pS->bA;
			}

			agg::rendering_buffer buf(cShadow.m_p, nPatSizeX, nPatSizeY, nPatSizeX);
			agg::pixfmt_gray8 img(buf);
			agg::stack_blur_gray8(img, nShadow, nShadow);

			pS = cPattern.m_p;
			pD = cShadow.m_p;
			ULONG nShadowColor = 0;
			float fR = GetRValue(clrText)/255.0f;
			float fG = GetGValue(clrText)/255.0f;
			float fB = GetBValue(clrText)/255.0f;
			if (powf(fR, 0.45f)*0.3f+powf(fG, 0.45f)*0.58f+powf(fB, 0.45f)*0.12f < 0.575f)
				nShadowColor = pGamma[0xff];
			for (TPixelChannel* const pEnd = cPattern.m_p+nPatSizeX*nPatSizeY; pS != pEnd; ++pS, ++pD)
			{
				if (pS->bA != 255)
				{
					ULONG const n = min(255, *pD*5);
					// blend pixels
					ULONG nNewA = pS->bA*255 + (255-pS->bA)*n;
					if (nNewA)
					{
						ULONG const bA1 = (255-pS->bA)*n;
						ULONG const bA2 = pS->bA*255;
						pS->bR = pInvGamma[(nShadowColor*bA1 + pGamma[pS->bR]*bA2)/nNewA];
						pS->bG = pInvGamma[(nShadowColor*bA1 + pGamma[pS->bG]*bA2)/nNewA];
						pS->bB = pInvGamma[(nShadowColor*bA1 + pGamma[pS->bB]*bA2)/nNewA];
					}
					else
					{
						pS->bR = 0;
						pS->bG = 0;
						pS->bB = 0;
					}
					pS->bA = nNewA/255;
				}
			}
		}
	}

	if (nOpacity != 100)
	{
		if (nOpacity > 100) nOpacity = 100;
		else if (nOpacity < 0) nOpacity = 0;
		ULONG n = (0x1000000*nOpacity)/100;
		TPixelChannel* p = cPattern.m_p;
		for (TPixelChannel* const pEnd = cPattern.m_p+nPatSizeX*nPatSizeY; p != pEnd; ++p)
		{
			p->bA = (p->bA*n)>>24;
		}
	}

	CAutoVectorPtr<TPixelChannel> pData(new TPixelChannel[tSize.nX*tSize.nY]);
	HRESULT hRes = S_OK;
	if (pRI)
	{
		hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pData.m_p), NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
	}
	else
	{
		ZeroMemory(pData.m_p, tSize.nX*tSize.nY*sizeof*pData.m_p);
	}
	LONG const nXExtra = LONG(tSize.nX)-LONG(nPatSizeX+2*nMargins);
	LONG const nYExtra = LONG(tSize.nY)-LONG(nPatSizeY+2*nMargins);
	LONG const nXOffset = (nXExtra*nPosX+50)/100+nMargins;
	LONG const nYOffset = (nYExtra*nPosY+50)/100+nMargins;
	RECT const rcPattern = {nXOffset, nYOffset, nXOffset+nPatSizeX, nYOffset+nPatSizeY};
	if (bTiled)
	{
		LONG const nBiasX = (nXOffset/nPatSizeX+1)*nPatSizeX-nXOffset;
		LONG const nBiasY = (nYOffset/nPatSizeY+1)*nPatSizeY-nYOffset;
		TPixelChannel* pD = pData.m_p;
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			TPixelChannel const* pL = cPattern.m_p + nPatSizeX*((y+nBiasY)%nPatSizeY);
			for (ULONG x = 0; x < tSize.nX; ++x)
			{
				TPixelChannel const* pS = pL + ((x+nBiasX)%nPatSizeX);
				if (pS->bA == 255)
				{
					*pD = *pS;
				}
				else
				{
					// blend pixels
					ULONG nNewA = pS->bA*255 + (255-pS->bA)*pD->bA;
					if (nNewA)
					{
						ULONG const bA1 = (255-pS->bA)*pD->bA;
						ULONG const bA2 = pS->bA*255;
						pD->bR = pInvGamma[(pGamma[pD->bR]*bA1 + pGamma[pS->bR]*bA2)/nNewA];
						pD->bG = pInvGamma[(pGamma[pD->bG]*bA1 + pGamma[pS->bG]*bA2)/nNewA];
						pD->bB = pInvGamma[(pGamma[pD->bB]*bA1 + pGamma[pS->bB]*bA2)/nNewA];
					}
					else
					{
						pD->bR = 0;
						pD->bG = 0;
						pD->bB = 0;
					}
					pD->bA = nNewA/255;
				}
				++pD;
			}
		}
	}
	else
	{
		RECT const rcClipped =
		{
			max(rcPattern.left, 0),
			max(rcPattern.top, 0),
			min(rcPattern.right, LONG(tSize.nX)),
			min(rcPattern.bottom, LONG(tSize.nY)),
		};
		TPixelChannel* const pPat = cPattern.m_p + nPatSizeX*(rcClipped.top-rcPattern.top) + rcClipped.left-rcPattern.left;
		TPixelChannel* const pPlane = pData.m_p;
		TPixelChannel* pD = pPlane + tSize.nX*rcClipped.top + rcClipped.left;
		TPixelChannel const* pS = pPat;
		for (LONG y = rcClipped.top; y < rcClipped.bottom; ++y)
		{
			for (LONG x = rcClipped.left; x < rcClipped.right; ++x)
			{
				if (pS->bA == 255)
				{
					*pD = *pS;
				}
				else
				{
					// blend pixels
					ULONG nNewA = pS->bA*255 + (255-pS->bA)*pD->bA;
					if (nNewA)
					{
						ULONG const bA1 = (255-pS->bA)*pD->bA;
						ULONG const bA2 = pS->bA*255;
						pD->bR = pInvGamma[(pGamma[pD->bR]*bA1 + pGamma[pS->bR]*bA2)/nNewA];
						pD->bG = pInvGamma[(pGamma[pD->bG]*bA1 + pGamma[pS->bG]*bA2)/nNewA];
						pD->bB = pInvGamma[(pGamma[pD->bB]*bA1 + pGamma[pS->bB]*bA2)/nNewA];
					}
					else
					{
						pD->bR = 0;
						pD->bG = 0;
						pD->bB = 0;
					}
					pD->bA = nNewA/255;
				}
				++pD;
				++pS;
			}
			pD += tSize.nX-rcClipped.right+rcClipped.left;
			pS += nPatSizeX-rcClipped.right+rcClipped.left;
		}
	}
	if (pRI)
	{
		return pRI->TileSet(EICIRGBA, NULL, &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel const*>(pData.m_p), FALSE);
	}
	else
	{
		CComPtr<IComparable> pLayer;
		HRESULT hRes = pLI->LayerInsert(NULL, ELIPDefault, &CImageLayerCreatorRasterImage(tSize, pData.m_p), &pLayer);
		if (pLayer)
		{
			wchar_t szName[128] = L"";
			Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_WATERMARK_LAYERNAME, szName, itemsof(szName), LANGIDFROMLCID(a_tLocaleID));
			pLI->LayerNameSet(pLayer, CComBSTR(szName));
		}
		return hRes;
	}
}

STDMETHODIMP CDocumentOperationWatermark::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID a_tLocaleID)
{
	try
	{
		float fGamma = 2.2f;
		CAutoVectorPtr<BYTE> pInvGamma(new BYTE[1<<nGammaBits]);
		CAutoVectorPtr<WORD> pGamma(new WORD[256]);
		float const fMul = (1<<nGammaBits)-1;
		for (ULONG i = 0; i < 256; ++i)
			pGamma[i] = powf(i/255.0f, fGamma)*fMul+0.5f;
		for (ULONG i = 0; i < (1<<nGammaBits); ++i)
			pInvGamma[i] = powf(i/fMul, 1.0f/fGamma)*255.0f+0.5f;

		CComPtr<IStorageFilter> pLoc;
		a_pDocument->LocationGet(&pLoc);
		CComBSTR bstrPath;
		if (pLoc) pLoc->ToText(NULL, &bstrPath);
		if (bstrPath == NULL)
		{
			CComQIPtr<IDocumentName> pName(pLoc);
			if (pName)
				pName->GetName(&bstrPath);
		}

		CComPtr<IImageMetaData> pIMD;
		a_pDocument->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		CEXIFExtractor cEXIFExtractor(pIMD);

		CComPtr<IDocumentAnimation> pAni;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAni));
		if (pAni)
		{
			CComPtr<IEnumUnknowns> pFrames;
			pAni->FramesEnum(&pFrames);
			ULONG nFrames = 0;
			if (pFrames) pFrames->Size(&nFrames);
			for (ULONG i = 0; i < nFrames; ++i)
			{
				CComPtr<IUnknown> pFrame;
				pFrames->Get(i, &pFrame);
				CComPtr<IDocument> pFrameDoc;
				if (pFrame) pAni->FrameGetDoc(pFrame, &pFrameDoc);
				if (pFrameDoc)
					if (FAILED(ProcessFrame(pFrameDoc, a_pConfig, a_tLocaleID, pInvGamma, pGamma, bstrPath, cEXIFExtractor)))
						return E_FAIL;
			}
			return S_OK;
		}

		return ProcessFrame(a_pDocument, a_pConfig, a_tLocaleID, pInvGamma, pGamma, bstrPath, cEXIFExtractor);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationWatermark::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrMARGINS(CFGID_WM_MARGINS);
	CConfigValue cVal;
	a_pConfig->ItemValueGet(bstrMARGINS, &cVal);
	if (cVal.TypeGet() == ECVTInteger && cVal.operator LONG() > 0)
	{
		LONG l = cVal.operator LONG()*f+0.5f;
		if (l < 1) l = 1;
		cVal = l;
		a_pConfig->ItemValuesSet(1, &(bstrMARGINS.m_str), cVal);
	}
	CComBSTR bstrABSSIZE(CFGID_WM_ABSSIZE);
	a_pConfig->ItemValueGet(bstrABSSIZE, &cVal);
	if (cVal.TypeGet() == ECVTInteger && cVal.operator LONG() > 0)
	{
		LONG l = cVal.operator LONG()*f+0.5f;
		if (l < 1) l = 1;
		cVal = l;
		a_pConfig->ItemValuesSet(1, &(bstrABSSIZE.m_str), cVal);
	}
	return S_OK;
}

#include <IconRenderer.h>

HICON CDocumentOperationWatermark::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const handle[] =
	{
		{172, 164, 0, -22, 0, 0},
		{157, 114, 0, -28, 0, 23.5372},
		{185, 52, 0, -30, 0, 32},
		{128, 0, -34, 0, 34, 0},
		{71, 52, 0, 32, 0, -30},
		{99, 114, 0, 23.5372, 0, -28},
		{84, 164, 0, 0, 0, -22},
		//{172, 176, 0, -22, 0, 0},
		//{157, 118, 0, -28, 0, 23.5372},
		//{185, 52, 0, -30, 0, 34},
		//{128, 0, -34, 0, 34, 0},
		//{71, 52, 0, 34, 0, -30},
		//{99, 118, 0, 23.5372, 0, -28},
		//{84, 176, 0, 0, 0, -22},
	};
	static IRGridItem handleY[] = { {0, 0}, {0, 176} };
	static IRCanvas canvasHandle = {0, 0, 256, 256, 0, itemsof(handleY), NULL, handleY};
	static IRPolyPoint const body[] =
	{
		{164, 152}, {164, 180}, {224, 180}, {240, 200}, {244, 244}, {12, 244}, {16, 200}, {32, 180}, {92, 180}, {92, 152},
		//{164, 160}, {164, 192}, {224, 192}, {240, 208}, {244, 244}, {12, 244}, {16, 208}, {32, 192}, {92, 192}, {92, 160},
	};
	static IRGridItem bodyX[] = { {0, 92}, {0, 164} };
	static IRGridItem bodyY[] = { {0, 192}, {0, 244} };
	static IRCanvas canvasBody = {0, 0, 256, 256, itemsof(bodyX), itemsof(bodyY), bodyX, bodyY};
	static IRPolyPoint const rubber[] =
	{
		{224, 256}, {224, 224}, {32, 224}, {32, 256},
	};
	static IRGridItem bot = {0, 256};
	static IRCanvas canvasRubber = {0, 0, 256, 256, 0, 1, NULL, &bot};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvasRubber, itemsof(rubber), rubber, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvasBody, itemsof(body), body, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvasHandle, itemsof(handle), handle, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}
