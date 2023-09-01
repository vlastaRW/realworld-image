// DocumentFactoryVectorImage.cpp : Implementation of CDocumentFactoryVectorImage

#include "stdafx.h"
#include "DocumentFactoryVectorImage.h"
#include "DocumentVectorImage.h"

#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>


// CDocumentFactoryVectorImage

STDMETHODIMP CDocumentFactoryVectorImage::Create(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground)
{
	try
	{
		CComObject<CDocumentVectorImage>* pDoc = NULL;
		CComObject<CDocumentVectorImage>::CreateInstance(&pDoc);
		CComPtr<IDocumentData> pTmp = pDoc;
		pDoc->Init(a_pSize->nX, a_pSize->nY, a_pResolution, a_aBackground);

		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::AddObject(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled)
{
	try
	{
		CComPtr<IDocumentVectorImage> pVI;
		a_pBase->DataBlockGet(a_bstrPrefix, __uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pVI));
		if (pVI == NULL)
			return E_RW_ITEMNOTFOUND;
		ULONG nID = 0;
		pVI->ObjectSet(&nID, a_bstrToolID, a_bstrParams);
		if (nID)
		{
			if (a_bstrName)
				pVI->ObjectNameSet(nID, a_bstrName);
			BOOL const bFill = a_bstrStyleID != NULL && *a_bstrStyleID != L'\0';
			if (bFill)
				pVI->ObjectStyleSet(nID, a_bstrStyleID, a_bstrStyleParams);
			else
				pVI->ObjectStyleSet(nID, CComBSTR(L"SOLID"), NULL);
			BOOL const bOutline = a_pOutlineWidth ? TRUE : FALSE;
			pVI->ObjectStateSet(nID, &bFill, a_pRasterizationMode, a_pCoordinatesMode, &bOutline, a_pOutlineColor, a_pOutlineWidth, a_pOutlinePos, a_pOutlineJoins);
			if (a_pEnabled && 0 == *a_pEnabled)
				pVI->ObjectEnable(nID, 0);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

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

		cRenderer(&canvasWhole, itemsof(sky), sky, &skyMat);
		cRenderer(&canvasWhole, itemsof(ground), ground, &groundMat);
		cRenderer(&canvasWhole, itemsof(sun), sun, &sunMat);
		cRenderer(&canvasWhole, itemsof(succulent), succulent, &plantMat);
		cRenderer(&canvasCactus, itemsof(cactus), cactus, &plantMat);

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
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
#define COLORPICKER_NOGRADIENT
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUINewVectorImage : 
	public CCustomConfigResourcelessWndImpl<CConfigGUINewVectorImage>
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
		CONTROL_LTEXT(_T("[0409]A vector image consists of geometric shapes like ellipses, lines, rectangles or glyphs. Individual shapes can be modified at any time and can be filled with solid colors, patterns or color gradients.[0405]Vektorový obrázek se skládá z geometrických tvarů jako jsou elipsy, čáry, obdélníky nebo písmena. Jednotlivé tvary lze kdykoli upravit a mohou být vyplněny barvami, vzory nebo barevnými gradienty."), IDC_STATIC, 0, 24, 156, 40, WS_VISIBLE, 0)
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

	BEGIN_MSG_MAP(CConfigGUINewVectorImage)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUINewVectorImage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_BACKGROUND, CButtonColorPicker::BCPN_SELCHANGE, OnColorChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUINewVectorImage)
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
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/vector-image"));
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

STDMETHODIMP CDocumentFactoryVectorImage::Config(IConfig** a_ppConfig)
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

		CConfigCustomGUI<&CLSID_DocumentFactoryVectorImage, CConfigGUINewVectorImage>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryVectorImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
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
		TImageResolution const tRes = {nResolution, 254, nResolution, 254};
		return a_pBuilder->Create(a_bstrPrefix, a_pBase, CImageSize(nSizeX, nSizeY), &tRes, aBackground);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(L"[vecimg]");
		return pDVI ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static BYTE const RVI_HEADER[] = {'R', 'W', 'V', 'e', 'c', 't', 'o', 'r', 'I', 'm', 'a', 'g', 'e', '_', '_', '1'};
static DWORD const MARK_SIZE = mmioFOURCC('S', 'I', 'Z', 'E');
static DWORD const MARK_RESOLUTION = mmioFOURCC('R', 'S', 'L', 'T');
static DWORD const MARK_BACKGROUND = mmioFOURCC('B', 'G', 'N', 'D');
static DWORD const MARK_ELEMENT_MODES = mmioFOURCC('M', 'O', 'D', 'E');
static DWORD const MARK_ELEMENT_FILLPAINT = mmioFOURCC('P', 'N', 'T', '0');
static DWORD const MARK_ELEMENT_OUTLINEPAINT = mmioFOURCC('P', 'N', 'T', '1');
static DWORD const MARK_ELEMENT_OUTLINEWIDTH = mmioFOURCC('L', 'I', 'N', 'E');
static DWORD const MARK_ELEMENT_OUTLINEJOINS = mmioFOURCC('J', 'O', 'I', 'N');
static DWORD const MARK_ELEMENT_VISIBILITY = mmioFOURCC('V', 'S', 'B', 'L');
static DWORD const MARK_ELEMENT_NAME = mmioFOURCC('N', 'A', 'M', 'E');
static DWORD const MARK_ELEMENT_TOOL = mmioFOURCC('T', 'O', 'O', 'L');
// legacy
static DWORD const MARK_ELEMENT = mmioFOURCC('E', 'L', 'E', 'M');
static DWORD const MARK_ELEMENT_STYLE = mmioFOURCC('S', 'T', 'Y', 'L');
static DWORD const MARK_ELEMENT_STATE = mmioFOURCC('S', 'T', 'A', 'T');

static BYTE DWORD_PADDING[4] = {0, 0, 0, 0};

class CImageChannelWriter : public IEnumImageChannels
{
public:
	CImageChannelWriter(std::vector<TChannelDefault>& a_cVals) : m_cVals(a_cVals) {}

	operator IEnumImageChannels*() { return this; }

	// IUnknown methods
public:
	STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
	{
		if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IEnumImageChannels)))
		{
			*a_ppvObject = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }

	// IEnumImageChannels methods
public:
	STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, TChannelDefault const* a_aChannelDefaults)
	{
		for (; a_nCount > 0; ++a_aChannelDefaults, --a_nCount)
			m_cVals.push_back(*a_aChannelDefaults);
		return S_OK;
	}

private:
	std::vector<TChannelDefault>& m_cVals;
};

HRESULT WriteIDAndParams(IReturnedData* a_pDst, DWORD a_dwMark, ULONG a_nIDLength, wchar_t const* a_pszID, ULONG a_nParamsLength, wchar_t const* a_pszParams)
{
	ULONG nLenID = WideCharToMultiByte(CP_UTF8, 0, a_pszID, a_nIDLength, NULL, 0, NULL, NULL);
	CAutoVectorPtr<BYTE> aID(nLenID ? new BYTE[nLenID] : 0);
	WideCharToMultiByte(CP_UTF8, 0, a_pszID, a_nIDLength, reinterpret_cast<char*>(aID.m_p), nLenID, NULL, NULL);
	ULONG nLenParams = WideCharToMultiByte(CP_UTF8, 0, a_pszParams, a_nParamsLength, NULL, 0, NULL, NULL);
	CAutoVectorPtr<BYTE> aParams(nLenParams ? new BYTE[nLenParams] : 0);
	WideCharToMultiByte(CP_UTF8, 0, a_pszParams, a_nParamsLength, reinterpret_cast<char*>(aParams.m_p), nLenParams, NULL, NULL);

	BYTE aLenID[6] = {0};
	ULONG nLenIDLen = 0;
	for (ULONG n = nLenID; n; n>>=7, ++nLenIDLen)
	{
		aLenID[nLenIDLen] = (n&0x7f)|((n&0xffffff80) ? 0x80 : 0);
	}
	if (nLenIDLen == 0) nLenIDLen = 1;
	BYTE aLenParams[6] = {0};
	ULONG nLenParamsLen = 0;
	for (ULONG n = nLenParams; n; n>>=7, ++nLenParamsLen)
	{
		aLenParams[nLenParamsLen] = (n&0x7f)|((n&0xffffff80) ? 0x80 : 0);
	}
	if (nLenParamsLen == 0) nLenParamsLen = 1;
	ULONG nLen = nLenIDLen+nLenID+nLenParamsLen+nLenParams;

	if (FAILED(a_pDst->Write(sizeof(a_dwMark), reinterpret_cast<BYTE const*>(&a_dwMark))))
		return E_FAIL;
	if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
		return E_FAIL;
	if (FAILED(a_pDst->Write(nLenIDLen, aLenID)))
		return E_FAIL;
	if (nLenID && FAILED(a_pDst->Write(nLenID, aID)))
		return E_FAIL;
	if (FAILED(a_pDst->Write(nLenParamsLen, aLenParams)))
		return E_FAIL;
	if (nLenParams && FAILED(a_pDst->Write(nLenParams, aParams)))
		return E_FAIL;
	if (nLen&3)
		if (FAILED(a_pDst->Write(4-(nLen&3), DWORD_PADDING)))
			return E_FAIL;
	return S_OK;
}

STDMETHODIMP CDocumentFactoryVectorImage::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl)
{
	try
	{
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI == NULL) return E_NOINTERFACE;

		if (FAILED(a_pDst->Write(sizeof(RVI_HEADER), RVI_HEADER)))
			return E_FAIL;

		TImageSize tSize = {256, 256};
		TImageResolution tRes = {100, 254, 100, 254};
		ULONG nChIDs = 0;
		float fGamma = 2.2f;
		std::vector<TChannelDefault> cChannelDefs;
		CComPtr<IDocumentImage> pDI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
		if (pDI)
		{
			pDI->CanvasGet(&tSize, &tRes, NULL, NULL, NULL);
			pDI->ChannelsGet(&nChIDs, &fGamma, CImageChannelWriter(cChannelDefs));
		}

		// image size
		if (tSize.nX != 256 || tSize.nY != 256)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_SIZE), reinterpret_cast<BYTE const*>(&MARK_SIZE))))
				return E_FAIL;
			ULONG nLen = sizeof tSize;
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&tSize))))
				return E_FAIL;
		}

		// image resolution
		if (tRes.nNumeratorX != 100 || tRes.nDenominatorX != 254 || tRes.nNumeratorY != 100 || tRes.nDenominatorY != 254)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_RESOLUTION), reinterpret_cast<BYTE const*>(&MARK_RESOLUTION))))
				return E_FAIL;
			ULONG nLen = sizeof tRes;
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&tRes))))
				return E_FAIL;
		}

		// background color
		if (nChIDs != EICIRGBA || fGamma != 2.2f || cChannelDefs.size() != 1 || cChannelDefs[0].eID != EICIRGBA || cChannelDefs[0].tValue.n != 0)
		{
			if (FAILED(a_pDst->Write(sizeof(MARK_BACKGROUND), reinterpret_cast<BYTE const*>(&MARK_BACKGROUND))))
				return E_FAIL;
			ULONG nItems = cChannelDefs.size();
			ULONG nLen = sizeof nChIDs+sizeof fGamma+sizeof nItems+sizeof(TChannelDefault)*cChannelDefs.size();
			if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof nChIDs, reinterpret_cast<BYTE const*>(&nChIDs))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof fGamma, reinterpret_cast<BYTE const*>(&fGamma))))
				return E_FAIL;
			if (FAILED(a_pDst->Write(sizeof nItems, reinterpret_cast<BYTE const*>(&nItems))))
				return E_FAIL;
			if (nItems)
				if (FAILED(a_pDst->Write(sizeof(TChannelDefault)*cChannelDefs.size(), reinterpret_cast<BYTE const*>(&(cChannelDefs[0])))))
					return E_FAIL;
		}

		boolean bEnabled = 1;
		TColor tOutClr = {0.0f, 0.0f, 0.0f, 1.0f};
		ERasterizationMode eRM = ERMSmooth;
		ECoordinatesMode eCM = ECMFloatingPoint;
		BOOL bFill = TRUE;
		BOOL bOutline = FALSE;
		float fWidth = 1.0f;
		float fPos = -1.0f;
		EOutlineJoinType eJoins = EOJTRound;
		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;

		// elements
		std::vector<ULONG> cIDs;
		pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
		for (std::vector<ULONG>::const_iterator i = cIDs.begin(); i != cIDs.end(); ++i)
		{
			boolean bEnabled2 = bEnabled;
			TColor tOutClr2 = tOutClr;
			BOOL bFill2 = bFill;
			BOOL bOutline2 = bOutline;
			ERasterizationMode eRM2 = eRM;
			ECoordinatesMode eCM2 = eCM;
			float fWidth2 = fWidth;
			float fPos2 = fPos;
			EOutlineJoinType eJoins2 = eJoins;
			bEnabled = pDVI->ObjectIsEnabled(*i) != S_FALSE ? 1 : 0;
			pDVI->ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tOutClr, &fWidth, &fPos, &eJoins);
			if (eRM != eRM2 || eCM != eCM2)
			{
				ULONG nLen = 4+4;
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_MODES), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_MODES))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(4, reinterpret_cast<BYTE const*>(&eRM))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(4, reinterpret_cast<BYTE const*>(&eCM))))
					return E_FAIL;
			}
			if (bEnabled2 != bEnabled)
			{
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_VISIBILITY), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_VISIBILITY))))
					return E_FAIL;
				DWORD dwVal = bEnabled;
				ULONG nLen = 4;
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&dwVal))))
					return E_FAIL;
			}
			if (bOutline != bOutline2 || (bOutline && (tOutClr.fR != tOutClr2.fR || tOutClr.fG != tOutClr2.fG || tOutClr.fB != tOutClr2.fB || tOutClr.fA != tOutClr2.fA)))
			{
				if (bOutline)
				{
					wchar_t sz[64] = L"";
					swprintf(sz, L"%g,%g,%g,%g", tOutClr.fR, tOutClr.fG, tOutClr.fB, tOutClr.fA);
					if (FAILED(WriteIDAndParams(a_pDst, MARK_ELEMENT_OUTLINEPAINT, 5, L"SOLID", wcslen(sz), sz)))
						return E_FAIL;
				}
				else
				{
					if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_OUTLINEPAINT), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_OUTLINEPAINT))))
						return E_FAIL;
					ULONG nLen = 0;
					if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
						return E_FAIL;
					tOutClr = tOutClr2; // save later if needed
				}
			}
			if (bOutline && (fWidth != fWidth2 || fPos != fPos2))
			{
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_OUTLINEWIDTH), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_OUTLINEWIDTH))))
					return E_FAIL;
				float aOutPos[2] = {-fWidth*(fPos*0.5f-0.5f), fWidth*(fPos*0.5f+0.5f)};
				ULONG nLen = 8;//sizeof(aOutPos);
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(aOutPos))))
					return E_FAIL;
			}
			else
			{
				fWidth = fWidth2; // save later if needed
				fPos = fPos2;
			}
			if (bOutline && eJoins != eJoins2)
			{
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_OUTLINEJOINS), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_OUTLINEJOINS))))
					return E_FAIL;
				ULONG nLen = 4;
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				UINT jt = eJoins;
				if (FAILED(a_pDst->Write(nLen, reinterpret_cast<BYTE const*>(&jt))))
					return E_FAIL;
			}
			else
			{
				eJoins = eJoins2; // save later if needed
			}

			if (bFill)
			{
				CComBSTR bstrStyleID2;
				CComBSTR bstrStyleParams2;
				pDVI->ObjectStyleGet(*i, &bstrStyleID2, &bstrStyleParams2);
				if (bFill != bFill2 || bstrStyleID2 != bstrStyleID || bstrStyleParams2 != bstrStyleParams)
				{
					std::swap(bstrStyleID2.m_str, bstrStyleID.m_str);
					std::swap(bstrStyleParams2.m_str, bstrStyleParams.m_str);
					if (FAILED(WriteIDAndParams(a_pDst, MARK_ELEMENT_FILLPAINT, bstrStyleID.Length(), bstrStyleID, bstrStyleParams.Length(), bstrStyleParams)))
						return E_FAIL;
				}
			}
			else
			{
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_FILLPAINT), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_FILLPAINT))))
					return E_FAIL;
				ULONG nLen = 0;
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
			}

			CComBSTR bstrName;
			pDVI->ObjectNameGet(*i, &bstrName);
			if (bstrName.m_str)
			{
				ULONG nLen = bstrName.Length()*sizeof(wchar_t);
				if (FAILED(a_pDst->Write(sizeof(MARK_ELEMENT_NAME), reinterpret_cast<BYTE const*>(&MARK_ELEMENT_NAME))))
					return E_FAIL;
				if (FAILED(a_pDst->Write(sizeof(nLen), reinterpret_cast<BYTE const*>(&nLen))))
					return E_FAIL;
				if (nLen && FAILED(a_pDst->Write(((nLen+3)&0xfffffffc), reinterpret_cast<BYTE const*>(bstrName.m_str))))
					return E_FAIL;
			}

			CComBSTR bstrToolID;
			CComBSTR bstrParams;
			pDVI->ObjectGet(*i, &bstrToolID, &bstrParams);
			if (FAILED(WriteIDAndParams(a_pDst, MARK_ELEMENT_TOOL, bstrToolID.Length(), bstrToolID, bstrParams.Length(), bstrParams)))
				return E_FAIL;
		}

		return S_OK;
	}
	catch (...)
	{
		return E_NOTIMPL;
	}
}

BOOL ReadIDAndParams(ULONG a_nLen, BYTE const* a_pData, BSTR* a_pID, BSTR* a_pParams)
{
	if (a_nLen < 1)
		return FALSE;
	ULONG nLenID = 0;
	for (int i = 0; true; i+=7)
	{
		nLenID |= ULONG(0x7f&*a_pData)<<i;
		bool bLast = 0 == (0x80&*a_pData);
		--a_nLen;
		++a_pData;
		if (bLast)
			break;
		if (a_nLen == 0)
			return FALSE;
	}
	if (a_nLen < nLenID)
		return FALSE;
	if (nLenID)
	{
		LONG nWLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLenID, NULL, 0);
		if (nWLen <= 0)
			return FALSE;
		*a_pID = ::SysAllocStringLen(NULL, nWLen);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLenID, *a_pID, nWLen);
		a_nLen -= nLenID;
		a_pData += nLenID;
	}
	else
	{
		*a_pID = ::SysAllocStringLen(NULL, 0);
	}

	if (a_nLen < 1)
		return FALSE;
	ULONG nLenParams = 0;
	for (int i = 0; true; i+=7)
	{
		nLenParams |= ULONG(0x7f&*a_pData)<<i;
		bool bLast = 0 == (0x80&*a_pData);
		--a_nLen;
		++a_pData;
		if (bLast)
			break;
		if (a_nLen == 0)
			return FALSE;
	}
	if (a_nLen < nLenParams)
		return FALSE;
	if (nLenParams)
	{
		LONG nWLen = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLenParams, NULL, 0);
		if (nWLen <= 0)
			return FALSE;
		*a_pParams = ::SysAllocStringLen(NULL, nWLen);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char const*>(a_pData), nLenParams, *a_pParams, nWLen);
		a_nLen -= nLenParams;
		a_pData += nLenParams;
	}
	else
	{
		*a_pParams = ::SysAllocStringLen(NULL, 0);
	}

	return TRUE;
}

HRESULT CDocumentFactoryVectorImage::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryVectorImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
{
	try
	{
		if (a_nLen < 16 || memcmp(a_pData, RVI_HEADER, sizeof RVI_HEADER))
			return E_RW_UNKNOWNINPUTFORMAT;

		bool bCreated = false;
		TImageSize tSize = {256, 256};
		TImageResolution tRes = {100, 254, 100, 254};
		//ULONG tChannels = 1<<EICIRGBA;
		//float fGamma = 2.2f;
		//CChannelDefault cDefBG(EICIRGBA, 0);
		float aBackground[4] = {0.0f, 0.0f, 0.0f, 0.0f};

		boolean bEnabled = 1;
		TColor tClr1 = {0.0f, 0.0f, 0.0f, 1.0f};
		TColor tClr2 = {0.0f, 0.0f, 0.0f, 0.0f};
		TColor tOutClr = {0.0f, 0.0f, 0.0f, 1.0f};
		ERasterizationMode eRM = ERMSmooth;
		ECoordinatesMode eCM = ECMFloatingPoint;
		BOOL bOutline = FALSE;
		BOOL bFill = TRUE;
		float fWidth = 1.0f;
		float fPos = -1.0f;
		EOutlineJoinType eJoins = EOJTRound;
		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;
		CComBSTR bstrName;

		BYTE const* const pBegin = a_pData;
		BYTE const* const pEnd = a_pData+a_nLen;
		BYTE const* pData = pBegin+sizeof(RVI_HEADER);
		while (pEnd-pData > 8)
		{
			if (pData+8+*reinterpret_cast<DWORD const*>(pData+4) > pEnd)
				break; // incomplete block
			if (MARK_SIZE == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= sizeof tSize)
			{
				tSize = *reinterpret_cast<TImageSize const*>(pData+8);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_RESOLUTION == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) >= sizeof tRes)
			{
				tRes = *reinterpret_cast<TImageResolution const*>(pData+8);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_BACKGROUND == *reinterpret_cast<DWORD const*>(pData) && *reinterpret_cast<DWORD const*>(pData+4) == 0x14)
			{
				TChannelDefault const* pChDf = reinterpret_cast<TChannelDefault const*>(pData+8+0xc); // skip 3 dwords
				if (pChDf->eID == EICIRGBA)
				{
					aBackground[0] = powf(pChDf->tValue.bR/255.0f, 2.2f);
					aBackground[1] = powf(pChDf->tValue.bG/255.0f, 2.2f);
					aBackground[2] = powf(pChDf->tValue.bB/255.0f, 2.2f);
					aBackground[3] = pChDf->tValue.bA/255.0f;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}

			// object properties
			if (MARK_ELEMENT_MODES == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) >= 4+4)
				{
					eRM = *reinterpret_cast<ERasterizationMode const*>(pData+8);
					eCM = *reinterpret_cast<ECoordinatesMode const*>(pData+8+4);
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_FILLPAINT == *reinterpret_cast<DWORD const*>(pData))
			{
				bstrStyleID.Empty();
				bstrStyleParams.Empty();
				if (*reinterpret_cast<DWORD const*>(pData+4) > 0)
				{
					bFill = ReadIDAndParams(*reinterpret_cast<DWORD const*>(pData+4), pData+8, &bstrStyleID, &bstrStyleParams);
					if (bFill && bstrStyleID == L"CONICAL")
						bstrStyleID = L"ANGULAR";
				}
				else
				{
					bFill = FALSE;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_OUTLINEPAINT == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) > 0)
				{
					CComBSTR bstrID;
					CComBSTR bstrParams;
					bOutline = ReadIDAndParams(*reinterpret_cast<DWORD const*>(pData+4), pData+8, &bstrID, &bstrParams);
					if (bOutline)
					{
						tOutClr.fR = tOutClr.fG = tOutClr.fB = 0.0f;
						tOutClr.fA = 1.0f;
						if (bstrID == L"SOLID" && bstrParams.m_str)
						{
							swscanf(bstrParams, L"%f,%f,%f,%f", &tOutClr.fR, &tOutClr.fG, &tOutClr.fB, &tOutClr.fA);
						}
					}
				}
				else
				{
					bOutline = FALSE;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_OUTLINEWIDTH == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) >= 4+4)
				{
					float const fIn = *reinterpret_cast<float const*>(pData+8);
					float const fOut = *reinterpret_cast<float const*>(pData+8+4);
					fWidth = fIn + fOut;
					fPos = (fOut - fIn)/fWidth;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_OUTLINEJOINS == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) >= 4)
				{
					eJoins = *reinterpret_cast<EOutlineJoinType const*>(pData+8);
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_NAME == *reinterpret_cast<DWORD const*>(pData))
			{
				bstrName.Empty();
				if (*reinterpret_cast<DWORD const*>(pData+4) >= 4)
				{
					DWORD dwLen = *reinterpret_cast<DWORD const*>(pData+4)>>1;
					if (dwLen)
					{
						bstrName.Attach(SysAllocStringLen(reinterpret_cast<wchar_t const*>(pData+8), dwLen));
					}
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_VISIBILITY == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) >= 1)
				{
					bEnabled = pData[8] != 0;
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_TOOL == *reinterpret_cast<DWORD const*>(pData))
			{
				CComBSTR bstrID;
				CComBSTR bstrParams;
				if (ReadIDAndParams(*reinterpret_cast<DWORD const*>(pData+4), pData+8, &bstrID, &bstrParams))
				{
					if (!bCreated)
					{
						if (FAILED(a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tRes, aBackground)))
							return E_FAIL;
						bCreated = true;
					}
					a_pBuilder->AddObject(a_bstrPrefix, a_pBase, bstrName, bstrID, bstrParams, bFill ? bstrStyleID.m_str : NULL, bFill ? bstrStyleParams.m_str : NULL, bOutline ? &fWidth : NULL, bOutline ? &fPos : NULL, bOutline ? &eJoins : NULL, bOutline ? &tOutClr : NULL, &eRM, &eCM, &bEnabled);
					bstrName.Empty();
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}

			// legacy chunks (supported only in 2012.1 beta)
			if (MARK_ELEMENT_STYLE == *reinterpret_cast<DWORD const*>(pData))
			{
				BYTE const* pElementData = pData+8;
				BYTE const* const pElementEnd = pData+8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				CComBSTR bstr[2];
				for (ULONG i = 0; i < 2; ++i)
				{
					if (pElementEnd-pElementData < 4)
						break;
					DWORD dwLen = *reinterpret_cast<DWORD const*>(pElementData);
					pElementData += 4;
					if (pElementEnd < pElementData+dwLen*sizeof(wchar_t))
						break;
					bstr[i].Attach(SysAllocStringLen(reinterpret_cast<wchar_t const*>(pElementData), dwLen));
					pElementData += dwLen*sizeof(wchar_t);
				}
				std::swap(bstrStyleID.m_str, bstr[0].m_str);
				std::swap(bstrStyleParams.m_str, bstr[1].m_str);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT_STATE == *reinterpret_cast<DWORD const*>(pData))
			{
				if (*reinterpret_cast<DWORD const*>(pData+4) == 16+16+4+4+4+4)
				{
					tClr1 = *reinterpret_cast<TColor const*>(pData+8);
					tClr1.fR = powf(tClr1.fR, 2.2f);
					tClr1.fG = powf(tClr1.fG, 2.2f);
					tClr1.fB = powf(tClr1.fB, 2.2f);
					tOutClr = tClr1;
					tClr2 = *reinterpret_cast<TColor const*>(pData+8+16);
					tClr2.fR = powf(tClr2.fR, 2.2f);
					tClr2.fG = powf(tClr2.fG, 2.2f);
					tClr2.fB = powf(tClr2.fB, 2.2f);
					eRM = *reinterpret_cast<ERasterizationMode const*>(pData+8+16+16);
					eCM = *reinterpret_cast<ECoordinatesMode const*>(pData+8+16+16+4);
					ULONG nSFM = *reinterpret_cast<ULONG const*>(pData+8+16+16+4+4);
					bOutline = nSFM&1;
					bFill = (nSFM&2)>>1;
					fWidth = *reinterpret_cast<float const*>(pData+8+16+16+4+4+4);
				}
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}
			if (MARK_ELEMENT == *reinterpret_cast<DWORD const*>(pData))
			{
				if (!bCreated)
				{
					if (FAILED(a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tRes, aBackground)))
						return E_FAIL;
					bCreated = true;
				}
				BYTE const* pElementData = pData+8;
				BYTE const* const pElementEnd = pData+8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				CComBSTR bstr[3];
				for (ULONG i = 0; i < 3; ++i)
				{
					if (pElementEnd-pElementData < 4)
						break;
					DWORD dwLen = *reinterpret_cast<DWORD const*>(pElementData);
					pElementData += 4;
					if (pElementEnd < pElementData+dwLen*sizeof(wchar_t))
						break;
					bstr[i].Attach(SysAllocStringLen(reinterpret_cast<wchar_t const*>(pElementData), dwLen));
					pElementData += dwLen*sizeof(wchar_t);
				}
				if (bstr[0] && bstr[0][0] == L'\0')
					bstr[0].Empty();
				if (bstr[1] == L"PATH")
					bstr[1] = L"SHAPE"; // TODO: eventually remove
				CComBSTR bstrStyleParams2 = bstrStyleParams;
				if (bstrStyleID == L"SOLID")
				{
					if (wcsncmp(bstrStyleParams, L"CUSTOM", 6) == 0)
					{
						TColor t = {0.5f, 0.5f, 0.5f, 1.0f};
						swscanf(bstrStyleParams+7, L"%f,%f,%f,%f", &t.fR, &t.fG, &t.fB, &t.fA);
						t.fR = powf(t.fR, 2.2f);
						t.fG = powf(t.fG, 2.2f);
						t.fB = powf(t.fB, 2.2f);
						wchar_t sz[64] = L"";
						swprintf(sz, 64, L"%g,%g,%g,%g", t.fR, t.fG, t.fB, t.fA);
						bstrStyleParams2 = sz;
					}
					else if (wcsncmp(bstrStyleParams, L"PRIMARY", 7) == 0)
					{
						wchar_t sz[64] = L"";
						swprintf(sz, 64, L"%g,%g,%g,%g", tClr1.fR, tClr1.fG, tClr1.fB, tClr1.fA);
						bstrStyleParams2 = sz;
					}
					else if (wcsncmp(bstrStyleParams, L"SECONDARY", 9) == 0)
					{
						wchar_t sz[64] = L"";
						swprintf(sz, 64, L"%g,%g,%g,%g", tClr2.fR, tClr2.fG, tClr2.fB, tClr2.fA);
						bstrStyleParams2 = sz;
					}
				}
				else if (bstrStyleID == L"PATTERN")
				{
					wchar_t sz[128] = L"";
					swprintf(sz, 64, L"|%g,%g,%g,%g,%g,%g,%g,%g", tClr1.fR, tClr1.fG, tClr1.fB, tClr1.fA, tClr2.fR, tClr2.fG, tClr2.fB, tClr2.fA);
					bstrStyleParams2 += sz;
				}
				else if (bstrStyleID == L"LINEAR" || bstrStyleID == L"RADIAL" || bstrStyleID == L"CONICAL")
				{
					std::vector<std::pair<int, TColor> > cG;
					wchar_t const* psz = bstrStyleParams;
					while (*psz)
					{
						std::pair<int, TColor> t;
						int n = swscanf(psz, L"%i,%f,%f,%f,%f", &t.first, &t.second.fR, &t.second.fG, &t.second.fB, &t.second.fA);
						if (n != 5)
							break;
						for (int i = 0; *psz && i < 5; ++psz)
							if (*psz == L',')
								++i;
						t.second.fR = powf(t.second.fR, 2.2f);
						t.second.fG = powf(t.second.fG, 2.2f);
						t.second.fB = powf(t.second.fB, 2.2f);
						cG.push_back(t);
					}
					if (cG.size() < 2)
					{
						wchar_t sz[160] = L"";
						swprintf(sz, 160, L"0,%g,%g,%g,%g,65535,%g,%g,%g,%g,", tClr1.fR, tClr1.fG, tClr1.fB, tClr1.fA, tClr2.fR, tClr2.fG, tClr2.fB, tClr2.fA);
						bstrStyleParams2 = sz;
					}
					else
					{
						bstrStyleParams2.Empty();
						wchar_t sz[80] = L"";
						for (std::vector<std::pair<int, TColor> >::const_iterator i = cG.begin(); i != cG.end(); ++i)
						{
							swprintf(sz, 80, L"%i,%g,%g,%g,%g,", i->first, i->second.fR, i->second.fG, i->second.fB, i->second.fA);
							bstrStyleParams2 += sz;
						}
					}
					bstrStyleParams2 += psz;
				}
				a_pBuilder->AddObject(a_bstrPrefix, a_pBase, bstr[0], bstr[1], bstr[2], bFill ? bstrStyleID.m_str : NULL, bFill ? bstrStyleParams2.m_str : NULL, bOutline ? &fWidth : NULL, bOutline ? &fPos : NULL, bOutline ? &eJoins : NULL, bOutline ? &tOutClr : NULL, &eRM, &eCM, &bEnabled);
				pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
				continue;
			}

			// skip unknown block
			pData += 8+((*reinterpret_cast<DWORD const*>(pData+4)+3)&~3);
		}
		if (!bCreated)
		{
			HRESULT hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, &tRes, aBackground);
			if (FAILED(hRes)) return hRes;
		}
		if (a_pEncoderID)
			*a_pEncoderID = __uuidof(DocumentFactoryVectorImage);

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	return S_FALSE;
}

#include "ScriptedVectorImage.h"

STDMETHODIMP CDocumentFactoryVectorImage::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI)
		{
			CComObject<CScriptedVectorImage>* p = NULL;
			CComObject<CScriptedVectorImage>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pScriptingMgr, a_pDocument, pDVI);
			a_pSite->AddItem(CComBSTR(L"VectorImage"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pSecondary->Insert(CComBSTR(L"VectorImage"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::LayerName(BYTE a_bCreateNew, ILocalizedString** a_ppLayerType)
{
	try
	{
		*a_ppLayerType = NULL;
		*a_ppLayerType = new CMultiLanguageString(a_bCreateNew ? L"[0409]Create vector layer[0405]Vytvořit vektorovou vrstvu" : L"[0409]Vector layer[0405]Vektorová vrstva");
		return S_OK;
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CDocumentFactoryVectorImage::LayerIconID(GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	// {EBAADA1D-15C1-4021-9519-88B9F575DA29}
	static GUID const tID = {0xebaada1d, 0x15c1, 0x4021, {0x95, 0x19, 0x88, 0xb9, 0xf5, 0x75, 0xda, 0x29}};
	*a_pIconID = tID;
	return S_OK;
}

//#include <RenderIcon.h>

STDMETHODIMP CDocumentFactoryVectorImage::LayerIcon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	*a_phIcon = CDocumentVectorImage::LayerIcon(a_nSize);

	//if (a_phIcon == NULL)
	//	return E_POINTER;
	//static float const f = 1.0f/256.0f;
	////static TPolyCoords const aVertices[] = {{f*24, f*232}, {f*128, f*24}, {f*232, f*232}};
	////*a_phIcon = IconFromPolygon(itemsof(aVertices), aVertices, a_nSize, false);
	//static TPolyCoords const aVertices[] =
	//{
	//	{f*112.78, f*29.16},
	//	{-f*28.91, f*4.47}, {f*15.77, -f*27.31}, {f*41.4, f*78},
	//	{-f*27.61, f*47.83}, {-f*47.83, -f*27.61}, {f*78, f*214.6},
	//	{f*47.83, f*27.61}, {-f*27.61, f*47.83}, {f*214.6, f*178},
	//	{0, 0}, {0, 0}, {f*128, f*128},
	//	{0, 0}, {0, 0}, {f*112.78, f*29.16}
	//};
	//TIconPolySpec tPolySpec[1];
	//tPolySpec[0].nVertices = itemsof(aVertices);
	//tPolySpec[0].pVertices = aVertices;
	//tPolySpec[0].interior = GetIconFillColor();
	//tPolySpec[0].outline = agg::rgba8(0, 0, 0, 255);
	//*a_phIcon = IconFromPath(itemsof(tPolySpec), tPolySpec, a_nSize, false);
	return S_OK;
}

STDMETHODIMP CDocumentFactoryVectorImage::LayerAccelerator(TCmdAccel* a_pAccel)
{
	if (a_pAccel == NULL)
		return E_POINTER;
	a_pAccel->wKeyCode = 'V';
	a_pAccel->fVirtFlags = FCONTROL|FSHIFT;
	return S_OK;
}

class ATL_NO_VTABLE CVectorLayerCreator :
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public IImageLayerCreator
{
public:
	void Init(IDocumentFactoryVectorImage* a_pFactory, TImageSize a_tSize, float const* a_aBg)
	{
		m_pFactory = a_pFactory;
		m_tSize = a_tSize;
		if (a_aBg)
		{
			m_aBg[0] = a_aBg[0];
			m_aBg[1] = a_aBg[1];
			m_aBg[2] = a_aBg[2];
			m_aBg[3] = a_aBg[3];
		}
		else
		{
			m_aBg[0] = m_aBg[1] = m_aBg[2] = m_aBg[3] = 0.0f;
		}
	}

BEGIN_COM_MAP(CVectorLayerCreator)
	COM_INTERFACE_ENTRY(IImageLayerCreator)
END_COM_MAP()

	// IImageLayerCreator methods
public:
	STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
	{
		return m_pFactory->Create(a_bstrID, a_pBase, &m_tSize, NULL, m_aBg);
	}

private:
	CComPtr<IDocumentFactoryVectorImage> m_pFactory;
	TImageSize m_tSize;
	float m_aBg[4];
};

STDMETHODIMP CDocumentFactoryVectorImage::LayerCreatorGet(TImageSize a_tSize, float const* a_aBackgroundRGBA, IImageLayerCreator** a_ppCreator)
{
	if (a_ppCreator == NULL)
		return E_POINTER;

	try
	{
		CComObject<CVectorLayerCreator>* p = NULL;
		CComObject<CVectorLayerCreator>::CreateInstance(&p);
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

STDMETHODIMP CDocumentFactoryVectorImage::LayerDescription(ILocalizedString** a_ppImageType)
{
	if (a_ppImageType == NULL)
		return E_POINTER;
	try
	{
		*a_ppImageType = new CMultiLanguageString(L"[0409]A vector image consists of geometric shapes like ellipses, lines, rectangles or glyphs. Individual shapes can be modified at any time and can be filled with solid colors, patterns or color gradients.[0405]Vektorový obrázek se skládá z geometrických tvarů jako jsou elipsy, čáry, obdélníky nebo písmena. Jednotlivé tvary lze kdykoli upravit a mohou být vyplněny barvami, vzory nebo barevnými gradienty.");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

