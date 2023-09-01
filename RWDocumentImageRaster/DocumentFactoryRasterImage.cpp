// DocumentFactoryRasterImage.cpp : Implementation of CDocumentFactoryRasterImage

#include "stdafx.h"
#include "DocumentFactoryRasterImage.h"
#include "DocumentLayeredImage.h"

#include <algorithm>
#include <MultiLanguageString.h>
#include <IconRenderer.h>


// CDocumentFactoryRasterImage

void GetPictureIcon(IStockIcons* pSI, CIconRendererReceiver& cRenderer, IRTarget const* target)
{
	static IRPolyPoint const sky[] =
	{
		{0, 0}, {256, 0}, {256, 154}, {0, 154},
	};
	static IRPolyPoint const ground[] =
	{
		{0, 125}, {256, 125}, {256, 256}, {0, 256},
	};
	static IRPathPoint const sun[] =
	{
		{127, 62, 0, -27.6142, 0, 27.6142},
		{77, 12, -27.6142, 0, 27.6142, 0},
		{27, 62, 0, 27.6142, 0, -27.6142},
		{77, 112, 27.6142, 0, -27.6142, 0},
	};
	static IRPathPoint const succulent[] =
	{
		{52.6465, 214.049, -13.5096, -10.1648, 5.25772, 2.34796},
		{20.5678, 159.276, 16.111, -1.50352, 1.54575, 15.0425},
		{50.0195, 172.211, 0.295563, -7.47905, -8.25191, -7.81683},
		{63.6429, 152.418, 10.5155, 4.69597, -6.66395, 6.15726},
		{79.8934, 174.462, 7.81683, -8.25191, 0.295563, -7.47905},
		{109.332, 160.644, 1.58796, 13.9741, -11.7528, -0.464462},
		{79.3574, 215.105, -5.3844, 0.857315, 13.2434, -10.1777},
	};
	static IRPathPoint const cactus[] =
	{
		{195, 39, 0, 0, 0, -11.2666},
		{195, 153, 0, 0, 0, 0},
		{205, 147, 0, 0, 0, 0},
		{205, 119, 0, -11.2666, 0, 0},
		{225, 99, 11.2666, 0, -11.2666, 0},
		{245, 119, 0, 0, 0, -11.2666},
		{245, 159, 0, 8.53818, 0, 0},
		{232, 178, 0, 0, 7.44406, -3.04126},
		{195, 200, 0, 0, 0.77684, -0.327835},
		{194, 243, 0, 0, 0, 0},
		{154, 243, 0, 0, 0, 0},
		{154, 184, -0.77684, -0.32782, 0, 0},
		{117, 163, -7.44408, -3.04128, 0, 0},
		{104, 143, 0, 0, 0, 8.53818},
		{104, 77, 0, -11.2666, 0, 0},
		{124, 57, 11.2666, 0, -11.2666, 0},
		{145, 77, 0, 0, 0, -11.2666},
		{145, 131, 0, 0, 0, 0},
		{154, 137, 0, 0, 0, 0},
		{154, 39, 0, -11.2666, 0, 0},
		{175, 19, 11.2666, 0, -11.2666, 0},
	};
	static IRGridItem const gridWhole[] = {{0, 0}, {0, 256}};
	static IRCanvas const canvasWhole = {0, 0, 256, 256, itemsof(gridWhole), itemsof(gridWhole), gridWhole, gridWhole};
	static IRGridItem const gridCactusX[] = {{0, 104}, {0, 145}, {0, 154}, {0, 195}, {0, 205}, {0, 245}};
	static IRCanvas const canvasCactus = {0, 0, 256, 256, itemsof(gridCactusX), 0, gridCactusX, NULL};
	IRMaterial const* pOutline = pSI->GetMaterial(ESMContrast);
	IRFill skyFill(0xff94e7ff);
	IROutlinedFill skyMat(&skyFill, pOutline, 1.0f/80.0f, 0.4f);
	IRFill groundFill(0xffa58442);
	IROutlinedFill groundMat(&groundFill, pOutline, 1.0f/80.0f, 0.4f);
	IRFill sunFill(0xffebe867);
	IROutlinedFill sunMat(&sunFill, pOutline, 1.0f/80.0f, 0.4f);
	IRFill plantFill(0xff73c610);
	IROutlinedFill plantMat(&plantFill, pOutline, 1.0f/80.0f, 0.4f);

	cRenderer(&canvasWhole, itemsof(sky), sky, &skyMat, target);
	cRenderer(&canvasWhole, itemsof(ground), ground, &groundMat, target);
	cRenderer(&canvasWhole, itemsof(sun), sun, &sunMat, target);
	cRenderer(&canvasWhole, itemsof(succulent), succulent, &plantMat, target);
	cRenderer(&canvasCactus, itemsof(cactus), cactus, &plantMat, target);


	static IRPathPoint const border1[] =
	{
		{256, 256, 0, 0, 0, 0},
		{0, 256, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		{256, 0, 0, 0, 0, 0},
	};
	static IRPathPoint const border2[] =
	{
		{220, 15, 11.598, 0, -8.14452, 0},
		{241, 36, 0, 7.97921, 0, -11.598},
		{230, 55, 6.55381, 3.55388, 6.55378, -3.55387},
		{241, 73, -3.14932e-005, 7.97918, 3.14935e-005, -7.97924},
		{230, 92, 6.55381, 3.55388, 6.55376, -3.55386},
		{241, 110, -3.10798e-005, 7.87443, 3.14935e-005, -7.97924},
		{230, 128, 5.83972, 3.72968, 6.41281, -3.59454},
		{240, 146, -3.14932e-005, 7.97918, 2.93769e-005, -7.44299},
		{229, 165, 6.55381, 3.55388, 6.55376, -3.55386},
		{240, 183, -3.08366e-005, 7.81281, 3.14935e-005, -7.97924},
		{229, 201, 6.87295, 3.45091, 6.33021, -3.61701},
		{241, 220, -4.19846e-005, 11.598, 3.24229e-005, -8.21472},
		{220, 241, -7.97918, -3.14932e-005, 11.598, 4.19846e-005},
		{202, 230, -3.55388, 6.5538, 3.55386, 6.55376},
		{183, 241, -7.97919, -2.09955e-005, 7.97923, 2.09956e-005},
		{165, 230, -3.55386, 6.55377, 3.55386, 6.55377},
		{146, 241, -7.87417, 2.07192e-005, 7.97919, -2.09955e-005},
		{128, 230, -3.72973, 5.83929, 3.59467, 6.41244},
		{110, 240, -7.97923, 2.09956e-005, 7.44265, -1.95837e-005},
		{92, 229, -3.55386, 6.55376, 3.55388, 6.5538},
		{73, 240, -7.81316, 2.05586e-005, 7.9792, -2.09955e-005},
		{55, 229, -3.45076, 6.87336, 3.61693, 6.33066},
		{36, 241, -11.598, -2.9403e-005, 8.21503, 2.16161e-005},
		{15, 220, 2.03394e-005, -7.97919, -2.9403e-005, 11.598},
		{26, 202, -6.55382, -3.55389, -6.55377, 3.55386},
		{15, 183, 4.98642e-005, -7.97916, -4.98648e-005, 7.97926},
		{26, 165, -6.55382, -3.55389, -6.55374, 3.55385},
		{15, 146, 4.92095e-005, -7.87441, -4.98648e-005, 7.97926},
		{26, 128, -5.83973, -3.72969, -6.41279, 3.59453},
		{16, 110, 4.98642e-005, -7.97917, -4.65136e-005, 7.44301},
		{27, 92, -6.55382, -3.55389, -6.55374, 3.55385},
		{16, 73, 4.88245e-005, -7.81279, -4.98648e-005, 7.97926},
		{27, 55, -6.87293, -3.4509, -6.33019, 3.617},
		{15, 36, 0, -11.598, 0, 8.2147},
		{36, 15, 7.91392, 0, -11.598, 0},
		{54, 26, 3.57965, -6.46567, -3.57959, -6.46585},
		{73, 15, 7.91388, 0, -7.91379, 0},
		{91, 26, 3.57963, -6.46572, -3.57961, -6.46581},
		{110, 15, 8.14415, 0, -7.91382, 0},
		{128, 26, 3.64409, -6.22544, -3.48344, -6.777},
		{146, 16, 7.91382, 0, -7.7344, 0},
		{165, 27, 3.57961, -6.46581, -3.57963, -6.46572},
		{183, 16, 7.73463, 0, -7.91388, 0},
		{201, 26, 3.48327, -6.7775, -3.64401, -6.22575},
	};
	static IRPath const border[] = { {itemsof(border1), border1}, {itemsof(border2), border2} };
	IROutlinedFill borderMat(pSI->GetMaterial(ESMBackground, true), pOutline, 1.0f/80.0f, 0.4f);

	cRenderer(&canvasWhole, itemsof(border), border, /*&borderMat*/pSI->GetMaterial(ESMBackground, true), target);
}

STDMETHODIMP CDocumentFactoryRasterImage::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

		IRTarget target;
		GetPictureIcon(pSI, cRenderer, target);

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvořit");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_SIZEX[] = L"SizeX";
static OLECHAR const CFGID_SIZEY[] = L"SizeY";
static OLECHAR const CFGID_RESOLUTION[] = L"Resolution";
static OLECHAR const CFGID_BACKGROUND[] = L"Background";

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUINewRasterImage : 
	public CCustomConfigResourcelessWndImpl<CConfigGUINewRasterImage>
{
public:
	BEGIN_DIALOG_EX(0, 0, 156, 20, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
		IDC_WIDTH_EDIT,
		IDC_WIDTH_SPIN,
		IDC_HEIGHT_EDIT,
		IDC_HEIGHT_SPIN,
		IDC_RESOLUTION_EDIT,
		IDC_RESOLUTION_SPIN,
		IDC_DOCUMENTATION,
		IDC_BACKGROUND,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Create image[0405]Vytvořit obrázek"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]A raster image is a rectangular area of dots called pixels. A picture is created by assigning each dot a color using various drawing tools. All photographs and most pictures found on the web are in fact raster images.[0405]Rastrový obrázek je obdélníková oblast bodů (nazývaných pixely). Obrázek je vytvořen obarvením každého pixelu pomocí kreslicích nástrojů. Všechny fotografie a většina obrázků na Internetu jsou rastrovými obrázky."), IDC_STATIC, 0, 24, 156, 40, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]More information[0405]Více informací"), IDC_DOCUMENTATION, 0, 68, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_LTEXT(_T("[0409]Canvas width:[0405]Šířka plátna:"), IDC_STATIC, 0, 85, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_WIDTH_EDIT, 75, 83, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_WIDTH_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 83, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STATIC, 123, 85, 23, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Canvas height:[0405]Výška plátna:"), IDC_STATIC, 0, 101, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_HEIGHT_EDIT, 75, 99, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_HEIGHT_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 99, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixely"), IDC_STATIC, 123, 101, 23, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Resolution:[0405]Rozlišení:"), IDC_STATIC, 0, 117, 73, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_RESOLUTION_EDIT, 75, 115, 42, 12, ES_RIGHT | ES_AUTOHSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_RESOLUTION_SPIN, UPDOWN_CLASS, UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 99, 115, 14, 12, 0)
		CONTROL_LTEXT(_T("[0409]DPI[0405]DPI"), IDC_STATIC, 123, 117, 23, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Background color:[0405]Barva pozadí:"), IDC_STATIC, 0, 133, 73, 8, WS_VISIBLE, 0)
		CONTROL_PUSHBUTTON(_T(""), IDC_BACKGROUND, 75, 131, 42, 12, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUINewRasterImage)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUINewRasterImage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_BACKGROUND, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUINewRasterImage)
		CONFIGITEM_EDITBOX(IDC_WIDTH_EDIT, CFGID_SIZEX)
		CONFIGITEM_EDITBOX(IDC_HEIGHT_EDIT, CFGID_SIZEY)
		CONFIGITEM_EDITBOX(IDC_RESOLUTION_EDIT, CFGID_RESOLUTION)
		CONFIGITEM_CONTEXTHELP(IDC_BACKGROUND, CFGID_BACKGROUND)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		m_wndColor.SubclassWindow(GetDlgItem(IDC_BACKGROUND));
		m_wndColor.SetDefaultColor(CButtonColorPicker::SColor(0, true));
		CUpDownCtrl wnd;
		wnd = GetDlgItem(IDC_WIDTH_SPIN);
		wnd.SetRange(1, 30000);
		wnd = GetDlgItem(IDC_HEIGHT_SPIN);
		wnd.SetRange(1, 30000);
		wnd = GetDlgItem(IDC_RESOLUTION_SPIN);
		wnd.SetRange(10, 1000);
	}
	void ExtraConfigNotify()
	{
		CConfigValue cColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cColor);
		m_wndColor.SetColor(CButtonColorPicker::SColor(cColor[0], cColor[1], cColor[2], cColor[3]), true);
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfUnderline = 1;
		lf.lfHeight *= 2;
		GetDlgItem(IDC_TITLE).SetFont(m_font.CreateFontIndirect(&lf));

		//m_wndRootFCC = GetDlgItem(IDC_ROOTFCC);

		m_wndDocumentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/raster-image"));
		return 1;  // Let the system set the focus
	}

	LRESULT OnColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
		CComBSTR cCFGID_BACKGROUND(CFGID_BACKGROUND);
		CConfigValue cColor(pClrBtn->clr.fR, pClrBtn->clr.fG, pClrBtn->clr.fB, pClrBtn->clr.fA);
		M_Config()->ItemValuesSet(1, &(cCFGID_BACKGROUND.m_str), cColor);

		return 0;
	}

private:
	CFont m_font;
	CHyperLink m_wndDocumentation;
	CButtonColorPicker m_wndColor;
};


STDMETHODIMP CDocumentFactoryRasterImage::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEX), NULL, NULL, CConfigValue(256L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_SIZEY), NULL, NULL, CConfigValue(256L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_RESOLUTION), NULL, NULL, CConfigValue(100L), NULL, 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_BACKGROUND), NULL, NULL, CConfigValue(0.0f, 0.0f, 0.0f, 0.0f), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentFactoryRasterImage, CConfigGUINewRasterImage>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		LONG nSizeX = 256;
		LONG nSizeY = 256;
		LONG nResolution = 100;
		float aBackground[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		CConfigValue cVal;
		if (a_pConfig)
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cVal);
			nSizeX = cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cVal);
			nSizeY = cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_RESOLUTION), &cVal);
			nResolution = cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cVal);
			aBackground[0] = cVal[0];
			aBackground[1] = cVal[1];
			aBackground[2] = cVal[2];
			aBackground[3] = cVal[3];
		}
		if (nSizeX < 1) nSizeX = 1;
		if (nSizeY < 1) nSizeY = 1;
		if (nResolution < 1) nResolution = 1;

		TImageSize const tSize = {nSizeX, nSizeY};
		CAutoVectorPtr<TRasterImagePixel> pData(new TRasterImagePixel[nSizeX*nSizeY]);
		CPixelChannel const tFill
		(
			aBackground[0] <= 0.0f ? 0 : (aBackground[0] >= 1.0f ? 255 : 255.0f*aBackground[0]+0.5f),
			aBackground[1] <= 0.0f ? 0 : (aBackground[1] >= 1.0f ? 255 : 255.0f*aBackground[1]+0.5f),
			aBackground[2] <= 0.0f ? 0 : (aBackground[2] >= 1.0f ? 255 : 255.0f*aBackground[2]+0.5f),
			aBackground[3] <= 0.0f ? 0 : (aBackground[3] >= 1.0f ? 255 : 255.0f*aBackground[3]+0.5f)
		);
		TImageResolution tRes = {nResolution, 254, nResolution, 254};
		return a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tRes, 1, CChannelDefault(EICIRGBA, tFill), 0.0f, NULL);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
{
	try
	{
		CComPtr<IDocumentData> pTmp;
		HRESULT hRes = CreateData(pTmp, a_pSize, a_pResolution, a_nChannels, a_aChannelDefaults, a_fGamma, a_pTile);
		if (FAILED(hRes)) return hRes;
		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, TRasterImageRect const* a_pRect, IRasterImageCallback* a_pProducer)
{
	try
	{
		if (a_nChannels != 1 || a_aChannelDefaults[0].eID != EICIRGBA)
			return E_RW_INVALIDPARAM;
		if (a_pRect == NULL || a_pProducer == NULL)
			return E_POINTER;
		LONG nPixels = LONG(a_pRect->tBR.nX-a_pRect->tTL.nX)*LONG(a_pRect->tBR.nY-a_pRect->tTL.nY);
		if (nPixels <= 0)
			return E_RW_INVALIDPARAM;
		CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[nPixels]);
		a_pProducer->Initalize(nPixels, cBuffer);
		CComObject<CDocumentRasterImage>* pDoc = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		TImageResolution tResolution = {100, 254, 100, 254};
		if (a_pResolution) tResolution = *a_pResolution;

		pDoc->Init(*a_pSize, &tResolution, &a_aChannelDefaults[0].tValue, a_pRect->tTL, CImageSize(a_pRect->tBR.nX-a_pRect->tTL.nX, a_pRect->tBR.nY-a_pRect->tTL.nY), cBuffer, 2.2f, NULL);
		ATLASSERT(FALSE); // TODO: remove the whole interface, scenario not supported anymore

		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT CDocumentFactoryRasterImage::CreateData(CComPtr<IDocumentData>& a_pDocData, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile)
{
	if (a_nChannels != 1 || a_aChannelDefaults[0].eID != EICIRGBA)
		return E_RW_INVALIDPARAM;

	CComObject<CDocumentRasterImage>* pDoc = NULL;
	CComObject<CDocumentRasterImage>::CreateInstance(&pDoc);
	a_pDocData = pDoc;
	TImageResolution tResolution = {100, 254, 100, 254};
	if (a_pResolution) tResolution = *a_pResolution;
	if (a_pTile)
	{
		CAutoVectorPtr<TPixelChannel> cPixels(new TPixelChannel[a_pTile->tSize.nX*a_pTile->tSize.nY]);
		if (a_pTile->tStride.nX == 1)
		{
			if (a_pTile->tStride.nY == a_pTile->tSize.nX)
				CopyMemory(cPixels.m_p, a_pTile->pData, a_pTile->tSize.nX*a_pTile->tSize.nY*sizeof*a_pTile->pData);
			else
				for (ULONG y = 0; y < a_pTile->tSize.nY; ++y)
					CopyMemory(cPixels.m_p+y*a_pTile->tSize.nX, a_pTile->pData+y*a_pTile->tStride.nY, a_pTile->tSize.nX*sizeof*a_pTile->pData);
		}
		else
		{
			TPixelChannel* pD = cPixels.m_p;
			for (ULONG y = 0; y < a_pTile->tSize.nY; ++y)
			{
				TPixelChannel const* pS = a_pTile->pData+y*a_pTile->tStride.nY;
				for (TPixelChannel* const pEnd = pD+a_pTile->tSize.nX; pD < pEnd; ++pD, pS+=a_pTile->tStride.nX)
					*pD = *pS;
			}
		}
		pDoc->Init(*a_pSize, &tResolution, &a_aChannelDefaults[0].tValue, a_pTile->tOrigin, a_pTile->tSize, cPixels, a_fGamma != 0.0f ? a_fGamma : 2.2f, NULL);
		if (a_pTile->tOrigin.nX == 0 && a_pTile->tOrigin.nY == 0 &&
			a_pTile->tSize.nX == a_pSize->nX && a_pTile->tSize.nY == a_pSize->nY)
			pDoc->OptimizeContent();
	}
	else
	{
		pDoc->Init(*a_pSize, &tResolution, &a_aChannelDefaults[0].tValue, CImagePoint(0, 0), CImageSize(0, 0), NULL, a_fGamma != 0.0f ? a_fGamma : 2.2f, NULL);
	}
	return S_OK;
}

STDMETHODIMP CDocumentFactoryRasterImage::AddBlock(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
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

STDMETHODIMP CDocumentFactoryRasterImage::LayerName(BYTE a_bCreateNew, ILocalizedString** a_ppLayerType)
{
	try
	{
		*a_ppLayerType = NULL;
		*a_ppLayerType = new CMultiLanguageString(a_bCreateNew ? L"[0409]Create raster layer[0405]Vytvořit rastrovou vrstvu" : L"[0409]Raster layer[0405]Rastrová vrstva");
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::LayerIconID(GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	// {A376A369-5BB8-4f60-8274-FE577FDE2B0D}
	static GUID const tID = {0xa376a369, 0x5bb8, 0x4f60, {0x82, 0x74, 0xfe, 0x57, 0x7f, 0xde, 0x2b, 0xd}};
	*a_pIconID = tID;
	return S_OK;
}

//#include <RenderIcon.h>

STDMETHODIMP CDocumentFactoryRasterImage::LayerIcon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	*a_phIcon = CDocumentRasterImage::LayerIcon(a_nSize);

	//static TPolyCoords const aVertices1[] = {{24, 232}, {24, 160}, {56, 160}, {56, 232}};
	//static TPolyCoords const aVertices2[] = {{68, 232}, {68, 96}, {100, 96}, {100, 232}};
	//static TPolyCoords const aVertices3[] = {{112, 232}, {112, 32}, {144, 32}, {144, 232}};
	//static TPolyCoords const aVertices4[] = {{156, 232}, {156, 95}, {188, 95}, {188, 232}};
	//static TPolyCoords const aVertices5[] = {{200, 232}, {200, 160}, {232, 160}, {232, 232}};
	//agg::rgba8 inter = GetIconFillColor();
	//agg::rgba8 outer(0, 0, 0, 255);
	//TIconPolySpec aPolys[] =
	//{
	//	{itemsof(aVertices1), aVertices1, inter, outer},
	//	{itemsof(aVertices2), aVertices2, inter, outer},
	//	{itemsof(aVertices3), aVertices3, inter, outer},
	//	{itemsof(aVertices4), aVertices4, inter, outer},
	//	{itemsof(aVertices5), aVertices5, inter, outer},
	//};
	//*a_phIcon = IconFromPolygon(itemsof(aPolys), aPolys, a_nSize, true);

	//static float const f = 1.0f/256.0f;
	//static TPolyCoords const aVertices1[] = {{f*232, f*232}, {f*232, f*88}, {f*184, f*88}, {f*184, f*184}, {f*136, f*184}, {f*136, f*232}};
	//static TPolyCoords const aVertices2[] = {{f*40, f*48}, {f*88, f*48}, {f*88, f*96}, {f*136, f*96}, {f*136, f*144}, {f*88, f*144}, {f*88, f*192}, {f*40, f*192}};
	//*a_phIcon = IconFromPolygon(itemsof(aVertices1), aVertices1, itemsof(aVertices2), aVertices2, a_nSize, false);
	return S_OK;
}

STDMETHODIMP CDocumentFactoryRasterImage::LayerAccelerator(TCmdAccel* a_pAccel)
{
	if (a_pAccel == NULL)
		return E_POINTER;
	a_pAccel->wKeyCode = 'R';
	a_pAccel->fVirtFlags = FCONTROL|FSHIFT;
	return S_OK;
}

class ATL_NO_VTABLE CRasterLayerCreator :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public IImageLayerCreator
{
public:
	void Init(IDocumentFactoryRasterImage* a_pFactory, TImageSize a_tSize, float const* a_aBackgroundRGBA)
	{
		m_pFactory = a_pFactory;
		m_tSize = a_tSize;
		if (a_aBackgroundRGBA)
		{
			m_tDefault.bR = CGammaTables::ToSRGB(a_aBackgroundRGBA[0]);
			m_tDefault.bG = CGammaTables::ToSRGB(a_aBackgroundRGBA[1]);
			m_tDefault.bB = CGammaTables::ToSRGB(a_aBackgroundRGBA[2]);
			m_tDefault.bA = min(max(0, int(a_aBackgroundRGBA[3]*255.0f+0.5f)), 255);
		}
		else
		{
			m_tDefault.n = 0;
		}
	}

BEGIN_COM_MAP(CRasterLayerCreator)
	COM_INTERFACE_ENTRY(IImageLayerCreator)
END_COM_MAP()

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		return m_pFactory->Create(a_bstrID, a_pBase, &m_tSize, NULL, 1, CChannelDefault(EICIRGBA, m_tDefault), 0.0f, NULL);
	}

private:
	CComPtr<IDocumentFactoryRasterImage> m_pFactory;
	TImageSize m_tSize;
	TPixelChannel m_tDefault;
};

STDMETHODIMP CDocumentFactoryRasterImage::LayerCreatorGet(TImageSize a_tSize, float const* a_aBackgroundRGBA, IImageLayerCreator** a_ppCreator)
{
	if (a_ppCreator == NULL)
		return E_POINTER;

	try
	{
		CComObject<CRasterLayerCreator>* p = NULL;
		CComObject<CRasterLayerCreator>::CreateInstance(&p);
		CComPtr<IImageLayerCreator> pTmp = p;
		p->Init(this, a_tSize, a_aBackgroundRGBA);
		*a_ppCreator = p;
		pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryRasterImage::LayerDescription(ILocalizedString** a_ppImageType)
{
	if (a_ppImageType == NULL)
		return E_POINTER;
	try
	{
		*a_ppImageType = new CMultiLanguageString(L"[0409]A raster image is a rectangular area of dots called pixels. A picture is created by assigning each dot a color using various drawing tools. All photographs and most pictures found on the web are in fact raster images.[0405]Rastrový obrázek je obdélníková oblast bodů (nazývaných pixely). Obrázek je vytvořen obarvením každého pixelu pomocí kreslicích nástrojů. Všechny fotografie a většina obrázků na Internetu jsou rastrovými obrázky.");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

