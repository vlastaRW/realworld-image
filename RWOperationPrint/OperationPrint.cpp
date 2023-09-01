// OperationPrint.cpp : Implementation of COperationPrint

#include "stdafx.h"
#include "OperationPrint.h"

#include <MultiLanguageString.h>
#include <RWDocumentImageRaster.h>
#include <DocumentName.h>
#include <GammaCorrection.h>

void AdjustImageSize(LONG a_nSizingMethod, bool a_bAutoRotate, float a_fSetW, float a_fSetH, float a_fUnit, int a_nPaperW, int a_nPaperH, int a_nPrinterResW, int a_nPrinterResH, int& a_nImgW, int& a_nImgH, float a_fImgResW, float a_fImgResH)
{
	int nImgW = a_nImgW;
	int nImgH = a_nImgH;
	switch (a_nSizingMethod)
	{
	case 0L: // fit paper
		{
			float ratio = (float)a_nPaperH/nImgH;
			if((float)nImgW/nImgH > (float)a_nPaperW/a_nPaperH)
				ratio = (float)a_nPaperW/nImgW;
			if (a_bAutoRotate)
			{
				float ratio2 = (float)a_nPaperH/nImgW;
				if((float)nImgH/nImgW > (float)a_nPaperW/a_nPaperH)
					ratio2 = (float)a_nPaperW/nImgH;
				if (ratio2 > ratio)
					ratio = ratio2;
			}
			nImgW *= ratio;
			nImgH *= ratio;
		}
		break;
	case 1L: // original size
		nImgW = nImgW/a_fImgResW*a_nPrinterResW/254;
		nImgH = nImgH/a_fImgResH*a_nPrinterResH/254;
		break;
	case 2L: // defined width
		{
			float ratio = (float)nImgW/nImgH;
			nImgW = a_fSetW/25.4f*a_fUnit*a_nPrinterResW;
			nImgH = nImgW / ratio;
		}
		break;
	default: // defined height
		{
			float ratio = (float)nImgW/nImgH;
			nImgH = a_fSetH/25.4f*a_fUnit*a_nPrinterResH;
			nImgW = nImgH * ratio;
		}
		break;
	}
	a_nImgH = nImgH;
	a_nImgW = nImgW;
}

int GetPageCount(int a_nPaperW, int a_nPaperH, int a_nImgW, int a_nImgH, LONG a_nCopies, bool a_bCombine)
{
	if (a_nImgW > a_nPaperW || a_nImgH > a_nPaperH)
	{
		// image spans several pages
		return a_nCopies*((a_nImgW+a_nPaperW-1)/a_nPaperW)*((a_nImgH+a_nPaperH-1)/a_nPaperH);
	}

	if (!a_bCombine)
	{
		// no combining -> 1 image per page
		return a_nCopies;
	}

	// possibly several images per page
	int nPerPage = (a_nPaperW/a_nImgW)*(a_nPaperH/a_nImgH);
	return (a_nCopies+nPerPage-1)/nPerPage;
}

void RotateImage(int a_nImgW, int a_nImgH, CAutoVectorPtr<TPixelChannel>& a_pBuffer, bool a_bReverse = false)
{
	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[a_nImgW*a_nImgH]);
	TPixelChannel* pD = pBuffer;
	TPixelChannel const* pS = a_pBuffer;
	for (int y = 0; y < a_nImgH; ++y)
	{
		if (a_bReverse)
		{
			pD += a_nImgH-1;
			for (int x = 0; x < a_nImgW; ++x, ++pS, pD+=a_nImgH)
				*pD = *pS;
			pD -= a_nImgH*a_nImgW+a_nImgH;
		}
		else
		{
			pD += a_nImgH*(a_nImgW-1);
			for (int x = 0; x < a_nImgW; ++x, ++pS, pD-=a_nImgH)
				*pD = *pS;
			pD += a_nImgH+1;
		}
	}
	std::swap(a_pBuffer.m_p, pBuffer.m_p);
}

void BlendWithBackground(ULONG nPix, TPixelChannel* p)
{
	CComPtr<IGammaTableCache> pGTC;
	RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
	CGammaTables const* pGT = pGTC->GetSRGBTable();
	for (TPixelChannel* const e = p+nPix; p != e; ++p)
	{
		if (p->bA == 0)
		{
			p->n = 0xffffffff;
		}
		else if (p->bA != 0xff)
		{
			if (pGT)
			{
				p->bR = pGT->InvGamma(65535-(65535-pGT->m_aGamma[p->bR])*ULONG(p->bA)/255);
				p->bG = pGT->InvGamma(65535-(65535-pGT->m_aGamma[p->bG])*ULONG(p->bA)/255);
				p->bB = pGT->InvGamma(65535-(65535-pGT->m_aGamma[p->bB])*ULONG(p->bA)/255);
			}
			else
			{
				p->bR = 255-(255-p->bR)*ULONG(p->bA)/255;
				p->bG = 255-(255-p->bG)*ULONG(p->bA)/255;
				p->bB = 255-(255-p->bB)*ULONG(p->bA)/255;
			}
			p->bA = 0xff;
		}
	}
}

// COperationPrint

STDMETHODIMP COperationPrint::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Print[0405]Rastrový obrázek - vytisknout");
	return S_OK;
}

#include "ConfigGUIPrint.h"

STDMETHODIMP COperationPrint::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(L"DeviceName"),
			CMultiLanguageString::GetAuto(L"[0409]Printer[0405]Tiskárna"),
			CMultiLanguageString::GetAuto(L"[0409]Selected printer[0405]Vybraná tiskárna"),
			CConfigValue(CComBSTR(L"default")), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(L"DeviceMode"),
			CMultiLanguageString::GetAuto(L"[0409]Printer settings[0405]Nastavení tiskárny"),
			CMultiLanguageString::GetAuto(L"[0409]Configure selected printer[0405]Nastavení vybrané tiskárny"),
			CConfigValue(CComBSTR(L"")), NULL, 0, NULL);

		pCfg->ItemInsSimple(CComBSTR(L"CenterImage"),
			CMultiLanguageString::GetAuto(L"[0409]Center image[0405]Vycentrovat na papír"),
			CMultiLanguageString::GetAuto(L"[0409]Image will be printed in the center of the paper.[0405]Obrázek se vytiskne na střed papíru."),
			CConfigValue(true), NULL, 0, NULL);

		CComBSTR CFGID_IMAGESIZE(L"ImageSize");
		pCfg->ItemIns1ofN(CFGID_IMAGESIZE,
			CMultiLanguageString::GetAuto(L"[0409]Image Size[0405]Velikost obrázku"),
			CMultiLanguageString::GetAuto(L"[0409]Image can be printed over whole paper or it could keep its original size or user can specify image size[0405]Obrázek může být vytisknut přes celou stránku, ponechat si vlastní, nebo uživatelskou velikost"), CConfigValue(0L), NULL);
		pCfg->ItemOptionAdd(CFGID_IMAGESIZE, CConfigValue(0L), CMultiLanguageString::GetAuto(L"[0409]Fit paper[0405]Přes celý papír"), 0, NULL);
		pCfg->ItemOptionAdd(CFGID_IMAGESIZE, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]Original size[0405]Vlastní velikost"), 0, NULL);
		pCfg->ItemOptionAdd(CFGID_IMAGESIZE, CConfigValue(2L), CMultiLanguageString::GetAuto(L"[0409]Specific width[0405]Uživatelská šířka"), 0, NULL);
		pCfg->ItemOptionAdd(CFGID_IMAGESIZE, CConfigValue(3L), CMultiLanguageString::GetAuto(L"[0409]Specific height[0405]Uživatelská výška"), 0, NULL);

		pCfg->ItemInsSimple(CComBSTR("AutoRotate"),
			CMultiLanguageString::GetAuto(L"[0409]Auto-rotate[0405]Samo-natočení"),
			CMultiLanguageString::GetAuto(L"[0409]Rotate the image by 90 degrees if it results in larger prints or saved paper.[0405]Otočit obrázek o 90 stupňů, pokud to umožní vytisknout ho větší nebo ušetřit papír."),
			CConfigValue(true), NULL, 0, NULL);

		TConfigOptionCondition tCond[2];
		tCond[0].bstrID = CFGID_IMAGESIZE;
		tCond[0].eConditionType = ECOCEqual;
		tCond[0].tValue.eTypeID = ECVTInteger;
		tCond[0].tValue.iVal = 2L;
		pCfg->ItemInsRanged(CComBSTR(L"ImageWidth"), CMultiLanguageString::GetAuto(L"[0409]Width[0405]Šířka"), CMultiLanguageString::GetAuto(L"[0409]Specify image width; height will be adjusted to keep aspect ratio.[0405]Zadejte šířku obrázku; výška bude dopočítána, aby byl zachován poměr stran."), CConfigValue(100.0f), NULL, CConfigValue(1.0f), CConfigValue(10000.0f), CConfigValue(1.0f), 1, tCond);
		tCond[0].tValue.iVal = 3L;
		pCfg->ItemInsRanged(CComBSTR(L"ImageHeight"), CMultiLanguageString::GetAuto(L"[0409]Height[0405]Výška"), CMultiLanguageString::GetAuto(L"[0409]Specify image height; width will be adjusted to keep aspect ratio.[0405]Zadejte výšku obrázku; šířka bude dopočítána, aby byl zachován poměr stran."), CConfigValue(100.0f), NULL, CConfigValue(1.0f), CConfigValue(10000.0f), CConfigValue(1.0f), 1, tCond);

		pCfg->ItemIns1ofN(CComBSTR(L"Unit"),
			CMultiLanguageString::GetAuto(L"[0409]Unit[0405]Jednotka"),
			CMultiLanguageString::GetAuto(L"[0409]Image size unit[0405]Jednotka velikosti obrázku"), CConfigValue(1.0f), NULL);
		pCfg->ItemOptionAdd(CComBSTR(L"Unit"), CConfigValue(1.0f), CMultiLanguageString::GetAuto(L"[0409]mm[0405]mm"), 0, NULL);//1, tCond);
		pCfg->ItemOptionAdd(CComBSTR(L"Unit"), CConfigValue(25.4f), CMultiLanguageString::GetAuto(L"[0409]inch[0405]palec"), 0, NULL);//1, tCond);

		CComBSTR CFGID_COPIES(L"Copies");
		pCfg->ItemInsRanged(CFGID_COPIES, CMultiLanguageString::GetAuto(L"[0409]Number of copies[0405]Počet kopií"), CMultiLanguageString::GetAuto(L"[0409]How many times to print the image.[0405]Kolik kopií daného obrázku se vytiskne."), CConfigValue(1L), NULL, CConfigValue(1L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		tCond[0].tValue.iVal = 1L;
		tCond[1].bstrID = CFGID_COPIES;
		tCond[1].eConditionType = ECOCGreater;
		tCond[1].tValue.eTypeID = ECVTInteger;
		tCond[1].tValue.iVal = 1L;

		pCfg->ItemInsSimple(CComBSTR("Combine"),
			CMultiLanguageString::GetAuto(L"[0409]Combine[0405]Zkombinovat"),
			CMultiLanguageString::GetAuto(L"[0409]Place multiple images on a single page if they fit in.[0405]Umístit více obrázků na jednu stránku, pokud se vejdou."),
			CConfigValue(true), NULL, 1, tCond+1);

		CConfigCustomGUI<&CLSID_OperationPrint, CConfigGUIPrint>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CPrintingProfileName :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ILocalizedString
{
public:
	void Init(BSTR printerName, LONG type, float width, float height, float unit, LONG copies)
	{
		m_printerName = printerName;
		m_type = type;
		m_width = width;
		m_height = height;
		m_unit = unit;
		m_copies = copies;
	}

	BEGIN_COM_MAP(CPrintingProfileName)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString) { return GetLocalized(GetThreadLocale(), a_pbstrString); }
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		if (a_pbstrString == NULL)
			return E_POINTER;
		try
		{
			CComPtr<ITranslator> pTr;
			RWCoCreateInstance(pTr, __uuidof(Translator));
			CComBSTR bstr;
			if (m_type == 1L)
			{
				CMultiLanguageString::GetLocalized(L"[0409]Original size[0405]Původní velikost", a_tLCID, &bstr, pTr);
			}
			else if (m_type == 2L || m_type == 3L)
			{
				CMultiLanguageString::GetLocalized(m_type == 2L ? L"[0409]Width[0405]Šířka" : L"[0409]Height[0405]Výška", a_tLCID, &bstr, pTr);
				wchar_t buf[32];
				swprintf(buf, L": %g", int(m_type == 2L ? m_width*m_unit*10.0f : m_height*m_unit*10.0f)/10.0f);
				bstr += buf;
				bstr += m_unit == 25.4f ? L"\"" : L"mm";

			}
			else
			{
				CMultiLanguageString::GetLocalized(L"[0409]Scale to fit paper[0405]Roztáhnout na papír", a_tLCID, &bstr, pTr);
			}
			if (m_copies > 1)
			{
				wchar_t buf[32];
				swprintf(buf, L", %ix", m_copies);
				bstr += buf;
			}
			if (m_printerName.m_str)
			{
				bstr += L", ";
				bstr += m_printerName;
			}
			*a_pbstrString = bstr;
			bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComBSTR m_printerName;
	LONG m_type;
	float m_width;
	float m_height;
	float m_unit;
	LONG m_copies;
};

STDMETHODIMP COperationPrint::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		if (a_pConfig == NULL)
			return NameGet(NULL, a_ppName);

		if (a_ppName == NULL)
			return E_POINTER;

		CComObject<CPrintingProfileName>* p = NULL;
		CComObject<CPrintingProfileName>::CreateInstance(&p);
		CComPtr<ILocalizedString> pTmp = p;
		CConfigValue cName;
		a_pConfig->ItemValueGet(CComBSTR(L"DeviceName"), &cName);
		CConfigValue cSize;
		a_pConfig->ItemValueGet(CComBSTR(L"ImageSize"), &cSize);
		CConfigValue cWidth;
		a_pConfig->ItemValueGet(CComBSTR(L"ImageWidth"), &cWidth);
		CConfigValue cHeight;
		a_pConfig->ItemValueGet(CComBSTR(L"ImageHeight"), &cHeight);
		CConfigValue cUnits;
		a_pConfig->ItemValueGet(CComBSTR(L"Unit"), &cUnits);
		CConfigValue cCopies;
		a_pConfig->ItemValueGet(CComBSTR(L"Copies"), &cCopies);
		p->Init(cName, cSize, cWidth, cHeight, cUnits, cCopies);
		*a_ppName = pTmp;
		pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COperationPrint::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, 1, &__uuidof(IDocumentImage))) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COperationPrint::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<IDocumentImage> pDI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
		if (pDI == NULL)
			return E_FAIL;

		//CPrintDialog cPrintDlg;
		//if(cPrintDlg.DoModal() != IDOK)
		//  return S_OK;
		//LPDEVMODE pDevMode = cPrintDlg.GetDevMode();

		CConfigValue cValDeviceName, cValDeviceMode, cValCenterImage, cValImageSize, cValImageWidth, cValImageHeight, cValUnit, cValCopies, cValCombine, cValAutoRotate;
		a_pConfig->ItemValueGet(CComBSTR(L"DeviceName"), &cValDeviceName);
		a_pConfig->ItemValueGet(CComBSTR(L"DeviceMode"), &cValDeviceMode);
		a_pConfig->ItemValueGet(CComBSTR(L"CenterImage"), &cValCenterImage);
		a_pConfig->ItemValueGet(CComBSTR(L"ImageSize"), &cValImageSize);
		a_pConfig->ItemValueGet(CComBSTR(L"ImageWidth"), &cValImageWidth);
		a_pConfig->ItemValueGet(CComBSTR(L"ImageHeight"), &cValImageHeight);
		a_pConfig->ItemValueGet(CComBSTR(L"Unit"), &cValUnit);
		a_pConfig->ItemValueGet(CComBSTR(L"Copies"), &cValCopies);
		a_pConfig->ItemValueGet(CComBSTR(L"Combine"), &cValCombine);
		a_pConfig->ItemValueGet(CComBSTR(L"AutoRotate"), &cValAutoRotate);
		CDevMode cDevMode;
		DevModeFromConfigValue(cValDeviceMode, cDevMode);

		CPrinter cPrinter;
		if (!cPrinter.OpenPrinter(cValDeviceName, cDevMode.m_pDevMode))
			return E_FAIL; // TODO: error message
		//if (!cPrinter.OpenDefaultPrinter())
		//  throw E_FAIL; // TODO: error message

		if (cDevMode.IsNull())
			cDevMode.CopyFromPrinter(cPrinter.m_hPrinter);

		CDC cDC;
		cDC.Attach(cPrinter.CreatePrinterDC(cDevMode.m_pDevMode));
		if (cDC.IsNull())
			return E_FAIL; // TODO: error message

		TImageSize tSize = {0, 0};
		TImageResolution tRes = {0, 0, 0, 0};
		pDI->CanvasGet(&tSize, &tRes, NULL, NULL, NULL);
		float fImgResW = 100.0f/254.0f;
		float fImgResH = 100.0f/254.0f;
		if (tRes.nNumeratorX && tRes.nDenominatorX && tRes.nNumeratorY && tRes.nDenominatorY)
		{
			fImgResW = float(tRes.nNumeratorX)/tRes.nDenominatorX;
			fImgResH = float(tRes.nNumeratorY)/tRes.nDenominatorY;
		}

		OLECHAR szName[MAX_PATH+32] = L"";
		CDocumentName::GetDocName(a_pDocument, szName, MAX_PATH);
		szName[MAX_PATH-1] = L'\0';
		wcscat(szName, L" - RealWorld");

		// Begin a print job by calling the StartDoc function.
		if (cDC.StartDoc(szName) <= 0)
			return E_FAIL; // TODO: error message

		ULONG const nPix = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pData(new TPixelChannel[nPix]);
		pDI->TileGet(EICIRGBA, NULL, NULL, NULL, nPix, pData.m_p, NULL, EIRIAccurate);
		BlendWithBackground(nPix, pData);

		int nPageW = cDC.GetDeviceCaps(HORZRES);
		int nPageH = cDC.GetDeviceCaps(VERTRES);
		int nPrinterResW = cDC.GetDeviceCaps(LOGPIXELSX);
		int nPrinterResH = cDC.GetDeviceCaps(LOGPIXELSX);
		int nPhysX = cDC.GetDeviceCaps(PHYSICALWIDTH);
		int nPhysY = cDC.GetDeviceCaps(PHYSICALHEIGHT);
		int nMX0 = cDC.GetDeviceCaps(PHYSICALOFFSETX);
		int nMY0 = cDC.GetDeviceCaps(PHYSICALOFFSETY);
		int nMX1 = nPhysX-nPageW-nMX0;
		int nMY1 = nPhysY-nPageH-nMY0;
		int nShiftX = 0;
		int nShiftY = 0;
		if (nPhysX > nPageW)
			nShiftX = (nMX1-nMX0)>>1;
		if (nPhysY > nPageH)
			nShiftY = (nMY1-nMY0)>>1;

		int nImgH = tSize.nY;
		int nImgW = tSize.nX;
		int nImgH2 = nImgH;
		int nImgW2 = nImgW;
		AdjustImageSize(cValImageSize, cValAutoRotate, cValImageWidth, cValImageHeight, cValUnit, nPageW, nPageH, nPrinterResW, nPrinterResH, nImgW2, nImgH2, fImgResW, fImgResH);

		if (cValAutoRotate && GetPageCount(nPageW, nPageH, nImgW2, nImgH2, cValCopies, cValCombine) > GetPageCount(nPageW, nPageH, nImgH2, nImgW2, cValCopies, cValCombine))
		{
			// rotating image will save paper
			RotateImage(nImgW, nImgH, pData);
			std::swap(nImgW, nImgH);
			std::swap(nImgW2, nImgH2);
		}
		BITMAPINFO tBMI;
		ZeroMemory(&tBMI, sizeof(tBMI));
		tBMI.bmiHeader.biSize = sizeof(tBMI.bmiHeader);
		tBMI.bmiHeader.biWidth = nImgW;
		tBMI.bmiHeader.biHeight = -nImgH;
		tBMI.bmiHeader.biPlanes = 1;
		tBMI.bmiHeader.biBitCount = 32;
		tBMI.bmiHeader.biCompression = BI_RGB;


		//cDC.SetStretchBltMode(HALFTONE);

		int nPageX = 0;
		int nPageY = 0;
		if (cValCenterImage)
		{
			nPageY = (nImgH2%nPageH) ? (nPageH-(nImgH2%nPageH))>>1 : 0;
			nPageX = (nImgW2%nPageW) ? (nPageW-(nImgW2%nPageW))>>1 : 0;
		}

		// over more pages
		if (nImgH2 > nPageH || nImgW2 > nPageW)
		{
			int nPagesX = (nImgW2+nPageW-1)/nPageW;
			int nPagesY = (nImgH2+nPageH-1)/nPageH;
			float fImgRatioX = (float)nImgW/nImgW2;
			float fImgRatioY = (float)nImgH/nImgH2;
			for(int iCopy=0; iCopy<cValCopies.operator LONG(); ++iCopy)
			{
				for(int py=0; py<nPagesY; ++py)
				{
					int nPgY1 = py ? 0 : nPageY;
					int nPgY2 = py == nPagesY-1 ? nImgH2-nPageH*py+nPageY : nPageH;
					int nImgY1 = max(0, py*nPageH-nPageY)*fImgRatioY+0.5f;
					int nImgY2 = min(nImgH2, (py+1)*nPageH-nPageY)*fImgRatioY+0.5f;
					if (nImgY2 > nImgH) nImgY2 = nImgH;
					if (nImgY1 == nImgY2) // for very small (1px) source images
						if (nImgY1 > 0) --nImgY1; else ++nImgY2;
					for(int px=0; px<nPagesX; ++px)
					{
						int nPgX1 = px ? 0 : nPageX;
						int nPgX2 = px == nPagesX-1 ? nImgW2-nPageW*px+nPageX : nPageW;
						int nImgX1 = max(0, px*nPageW-nPageX)*fImgRatioX+0.5f;
						int nImgX2 = min(nImgW2, (px+1)*nPageW-nPageX)*fImgRatioX+0.5f;
						if (nImgX2 > nImgW) nImgX2 = nImgW;
						if (nImgX1 == nImgX2) // for very small (1px) source images
							if (nImgX1 > 0) --nImgX1; else ++nImgX2;

						if (cDC.StartPage() <= 0) return E_FAIL;
						//StretchDIBits(cDC, nPgX1, nPgY1, nPgX2-nPgX1, nPgY2-nPgY1,
						//	nImgX1, nImgY1, nImgX2-nImgX1, nImgY2-nImgY1,
						//	pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
						//StretchDIBits(cDC, nPgX1, nPgY2-1/*nPageH-nPgY1+1*/, nPgX2-nPgX1, nPgY1-nPgY2,
						//	nImgX1, nImgH-nImgY1+1, nImgX2-nImgX1, nImgY1-nImgY2,
						//	pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
						StretchDIBits(cDC, nPgX1, nPgY1, nPgX2-nPgX1, nPgY2-nPgY1,
							nImgX1, nImgH-nImgY2, nImgX2-nImgX1, nImgY2-nImgY1,
							pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
						cDC.EndPage();
					}
				}
			}
		}
		else
		{
			int nImagesPerPageX = nPageW / nImgW2;
			int nImagesPerPageY = nPageH / nImgH2;
			if (cValCombine && cValCopies.operator LONG() > 1 && nImagesPerPageX*nImagesPerPageY > 1)
			{
				// multiple images per page
				nPageY = cValCenterImage ? (((nPageH-(nImgH2*nImagesPerPageY))>>1)/nImagesPerPageY) : 0;
				nPageX = cValCenterImage ? (((nPageW-(nImgW2*nImagesPerPageX))>>1)/nImagesPerPageX) : 0;
				int nPages = cValCopies.operator LONG() / (nImagesPerPageX*nImagesPerPageY) + ((cValCopies.operator LONG() % (nImagesPerPageX*nImagesPerPageY)) ? 1 : 0);
				int nCopies = 0;
				for(int iPage=0; iPage<nPages; ++iPage)
				{
					if (cDC.StartPage() <= 0) return E_FAIL; // TODO: error message
					for(int iy=0; iy<nImagesPerPageY && nCopies<cValCopies.operator LONG(); ++iy)
						for(int ix=0; ix<nImagesPerPageX && nCopies<cValCopies.operator LONG(); ++ix)
						{
							StretchDIBits(cDC, ix*nImgW2+(ix+ix+1)*nPageX, iy*nImgH2+(iy+iy+1)*nPageY, nImgW2, nImgH2, 0, 0, nImgW, nImgH,
								pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
							++nCopies;
						}
						cDC.EndPage();
				}
			}
			else
			{
				// one image over one page
				if (cValCenterImage)
				{ // try to compensate margins for more accurate centering
					if (nShiftX < 0) nPageX = max(0, nPageX+nShiftX);
					else if (nShiftX > 0) nPageX = nImgW2+nPageX+nShiftX > nPageW ? nPageW-nImgW2 : nPageX+nShiftX;
					if (nShiftY < 0) nPageY = max(0, nPageY+nShiftY);
					else if (nShiftY > 0) nPageY = nImgH2+nPageY+nShiftY > nPageH ? nPageH-nImgH2 : nPageY+nShiftY;
				}
				for(int iCopy=0; iCopy<cValCopies.operator LONG(); ++iCopy)
				{
					if (cDC.StartPage() <= 0) return E_FAIL; // TODO: error message
					StretchDIBits(cDC, cValCenterImage ? nPageX : 0, cValCenterImage ? nPageY : 0, nImgW2, nImgH2, 0, 0, nImgW, nImgH,
						pData, &tBMI, DIB_RGB_COLORS, SRCCOPY);
					cDC.EndPage();
				}
			}
		}

		cDC.EndDoc(); 

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP COperationPrint::Name(ILocalizedString** a_ppName)
{
	*a_ppName = new CMultiLanguageString(L"[0409]Print...[0405]Vytisknout...");
	return S_OK;
}

STDMETHODIMP COperationPrint::Description(ILocalizedString** a_ppDesc)
{
	*a_ppDesc = new CMultiLanguageString(L"[0409]Select printer and print image.[0405]Vybrat tiskárnu a vytisknout obrázek.");
	return S_OK;
}

STDMETHODIMP COperationPrint::IconID(GUID* a_pIconID)
{
	*a_pIconID = CLSID_OperationPrint;
	return S_OK;
}

STDMETHODIMP COperationPrint::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	a_pAccel->wKeyCode = 'P';
	a_pAccel->fVirtFlags = FCONTROL;
	return S_OK;
}

STDMETHODIMP COperationPrint::Configuration(IConfig* a_pOperationCfg)
{
	// Adjust operation configuration of the menu item.
	// Use GUID of the Display Configuration operation
	// and make the Amount paremeter configurable.

	// "Operation" is a hardcoded ID of the root configuration item
	// and you must set it to the GUID of the main operation.
	CComBSTR bstr1(L"Operation");
	// It is set to __uuidof(DocumentOperationShowConfig), because
	// we want a configuration dialog. If no configuration dialog
	// is necessary, just set it to GUID of your operation.
	CConfigValue cID1(__uuidof(DocumentOperationShowConfig));

	// Since I am using the "Show Configuration" operation
	// http://wiki.rw-designer.com/Operation_Display_Configuration
	// (selected in previous step), I'll have to set its internals.
	// First, the actual operation used (this very "Dissolve" op).
	CComBSTR bstr2(L"Operation\\CfgOperation");
	CConfigValue cID2(CLSID_OperationPrint);

	// dialog window caption
	CComBSTR bstr3(L"Operation\\Caption");
	CConfigValue cID3(L"[0409]Print Image[0405]Vytisknout obrázek");

	CComBSTR bstr4(L"Operation\\IconID");
	CConfigValue cID4(CLSID_OperationPrint);

	// custom help string
	CComBSTR bstr5(L"Operation\\HelpTopic");
	CConfigValue cID5(L"Print image on Dissolve deletes given percentage of pixels from image.<br><a href=\"http://wiki.rw-designer.com/Raster_Image_-_Print\">More information</a>");

	// now, set all those values
	BSTR aIDs[] = {bstr1.m_str, bstr2.m_str, bstr3.m_str, bstr4.m_str/*, bstr5.m_str, bstr6.m_str*/};
	TConfigValue aVals[] = {cID1, cID2, cID3, cID4/*, cID5, cID6*/};
	return a_pOperationCfg->ItemValuesSet(sizeof(aIDs)/sizeof(*aIDs), aIDs, aVals);
}

