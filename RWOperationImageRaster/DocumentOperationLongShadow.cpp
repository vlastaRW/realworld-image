
#include "stdafx.h"
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include <RWDocumentImageVector.h>
#include <RWProcessingTags.h>
#include <algorithm>
#include <deque>

#include "AGGBuffer.h"
#include <agg_trans_perspective.h>
#include <agg_span_interpolator_trans.h>

#include <GammaCorrection.h>
#include <IconRenderer.h>


static OLECHAR const CFGID_LENGTH[] = L"Length";
static OLECHAR const CFGID_ORIENTATION[] = L"Orientation";
static OLECHAR const CFGID_COLOR[] = L"Color";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUILongShadowDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILongShadowDlg>,
	public CDialogResize<CConfigGUILongShadowDlg>,
	public CObserverImpl<CConfigGUILongShadowDlg, IColorPickerObserver, ULONG>,
	public IResizableConfigWindow
{
public:
	enum
	{
		IDC_LENGTH_EDIT = 100,
		IDC_ORIENTATION_EDIT,
		IDC_ORIENTATION_SLIDER,
		IDC_COLOR,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 40, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Length:[0405]Délka:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_LENGTH_EDIT, 50, 0, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Orientation:[0405]Orientace:"), IDC_STATIC, 0, 18, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ORIENTATION_EDIT, 50, 16, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_ORIENTATION_SLIDER, TRACKBAR_CLASS, TBS_NOTICKS | TBS_TOP | WS_TABSTOP | WS_VISIBLE, 0, 28, 100, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILongShadowDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILongShadowDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILongShadowDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILongShadowDlg)
		DLGRESIZE_CONTROL(IDC_LENGTH_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORIENTATION_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORIENTATION_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_COLOR, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILongShadowDlg)
		CONFIGITEM_EDITBOX(IDC_LENGTH_EDIT, CFGID_LENGTH)
		CONFIGITEM_EDITBOX(IDC_ORIENTATION_EDIT, CFGID_ORIENTATION)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_ORIENTATION_SLIDER, CFGID_ORIENTATION)
	END_CONFIGITEM_MAP()

	BEGIN_COM_MAP(CConfigGUILongShadowDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IResizableConfigWindow)
	END_COM_MAP()

	// IResizableConfigWindow methods
public:
	STDMETHOD(OptimumSize)(SIZE *a_pSize)
	{
		if (m_hWnd == NULL || m_pColor == NULL || a_pSize == NULL)
			return E_UNEXPECTED;
		RECT rc = {0, 0, 100, 44};
		MapDialogRect(&rc);
		SIZE sz = {0, 0};
		m_pColor->OptimumSize(&sz);
		a_pSize->cx = rc.right;
		a_pSize->cy = rc.bottom+sz.cy;
		return S_OK;
	}

	void ExtraInitDialog()
	{
		RECT rcColor = {0, 44, 100, 44};
		MapDialogRect(&rcColor);
		RWCoCreateInstance(m_pColor, __uuidof(ColorPicker));
		m_pColor->Create(m_hWnd, &rcColor, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Shadow color and intensity.[0405]Barva a intenzita stínu."), FALSE, CComBSTR(L"LONGSHADOW"), NULL);
		RWHWND hColor;
		m_pColor->Handle(&hColor);
		::SetWindowLong(hColor, GWLP_ID, IDC_COLOR);
		SIZE tSize = {rcColor.right-rcColor.left, 0};
		m_pColor->OptimumSize(&tSize);
		rcColor.bottom = rcColor.top+tSize.cy;
		m_pColor->Move(&rcColor);
		m_pColor->ObserverIns(CObserverImpl<CConfigGUILongShadowDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);
	}

	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		//if (!m_bEnableUpdates)
		//	return;

		if (a_nFlags&ECPCColor)
		{
			TColor clr;
			m_pColor->ColorGet(&clr);
			CConfigValue cVal(clr.fR, clr.fG, clr.fB, clr.fA);
			CComBSTR bstr(CFGID_COLOR);
			M_Config()->ItemValuesSet(1, &(bstr.m_str), cVal);
		}
		if (a_nFlags&(ECPCLayout|ECPCWindowHeight))
		{
			RWHWND h = NULL;
			m_pColor->Handle(&h);
			RECT rcWindow;
			::GetWindowRect(h, &rcWindow);
			ScreenToClient(&rcWindow);
			SIZE sz = {rcWindow.right-rcWindow.left, rcWindow.bottom-rcWindow.top};
			m_pColor->OptimumSize(&sz);
			rcWindow.bottom = rcWindow.top+sz.cy;
			m_pColor->Move(&rcWindow);
		}
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (m_pColor)
			m_pColor->ObserverDel(CObserverImpl<CConfigGUILongShadowDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}

	void ExtraConfigNotify()
	{
		if (m_hWnd == NULL)
			return;

		if (m_pColor)
		{
			CConfigValue cColor;
			M_Config()->ItemValueGet(CComBSTR(CFGID_COLOR), &cColor);
			TColor clr = {cColor[0], cColor[1], cColor[2], cColor[3]};
			m_pColor->ColorSet(&clr);
		}
	}

private:
	CComPtr<IColorPicker> m_pColor;
};


// CDocumentOperationLongShadow

// {6C85431B-87F2-4d07-8A1D-FDEA836165BA}
extern GUID const CLSID_DocumentOperationLongShadow = {0x6c85431b, 0x87f2, 0x4d07, {0x8a, 0x1d, 0xfd, 0xea, 0x83, 0x61, 0x65, 0xba}};

class ATL_NO_VTABLE CDocumentOperationLongShadow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationLongShadow, &CLSID_DocumentOperationLongShadow>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl,
	public ICanvasInteractingOperation,
	public ILayerStyle
{
public:
	CDocumentOperationLongShadow()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationLongShadow)

BEGIN_CATEGORY_MAP(CDocumentOperationLongShadow)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageUnderlay)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationLongShadow)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(ICanvasInteractingOperation)
	COM_INTERFACE_ENTRY(ILayerStyle)
END_COM_MAP()


	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
	{
		if (a_pConfig == NULL)
			return E_RW_INVALIDPARAM;
		try
		{
			CComBSTR bstrCFGID_LENGTH(CFGID_LENGTH);
			CComBSTR bstrCFGID_ORIENTATION(CFGID_ORIENTATION);
			CConfigValue cLength;
			a_pConfig->ItemValueGet(bstrCFGID_LENGTH, &cLength);
			CConfigValue cOrientation;
			a_pConfig->ItemValueGet(bstrCFGID_ORIENTATION, &cOrientation);
			if (a_pContentTransform == NULL)
				return S_FALSE;
			float scaleX = 1.0f;
			float scaleY = 1.0f;
			float rotation = 0.0f;
			TVector2f translation = {0.0f, 0.0f};
			Matrix3x3fDecompose(*a_pContentTransform, &scaleX, &scaleY, &rotation, &translation);
			cLength = cLength.operator float()*sqrtf(scaleX*scaleY);
			float a = cOrientation;
			a += rotation*(180*3.14159265358979);
			if (a > 360) a -= 360;
			else if (a < 0) a += 360;
			cOrientation = a;
			BSTR aIDs[2] = {bstrCFGID_LENGTH, bstrCFGID_ORIENTATION};
			TConfigValue aVals[2] = {cLength, cOrientation};
			a_pConfig->ItemValuesSet(2, aIDs, aVals);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		ATLASSERT(a_pConfig != NULL);

		CComBSTR bstrCFGID_LENGTH(CFGID_LENGTH);
		CComBSTR bstrCFGID_ORIENTATION(CFGID_ORIENTATION);
		CConfigValue cLength;
		a_pConfig->ItemValueGet(bstrCFGID_LENGTH, &cLength);
		CConfigValue cOrientation;
		a_pConfig->ItemValueGet(bstrCFGID_ORIENTATION, &cOrientation);

		float dx = cLength.operator float()*sinf(cOrientation.operator float()*3.14159265359f/180.0f);
		float dy = -cLength.operator float()*cosf(cOrientation.operator float()*3.14159265359f/180.0f);
		if (a_pRect)
		{
			if (dx > 0.0f)
				a_pRect->tBR.nX += ceilf(dx);
			else
				a_pRect->tTL.nX -= ceilf(-dx);

			if (dy > 0.0f)
				a_pRect->tBR.nY += ceilf(dy);
			else
				a_pRect->tTL.nY -= ceilf(-dy);
		}
		return S_OK;
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		ATLASSERT(a_pConfig != NULL);

		CComBSTR bstrCFGID_LENGTH(CFGID_LENGTH);
		CComBSTR bstrCFGID_ORIENTATION(CFGID_ORIENTATION);
		CConfigValue cLength;
		a_pConfig->ItemValueGet(bstrCFGID_LENGTH, &cLength);
		CConfigValue cOrientation;
		a_pConfig->ItemValueGet(bstrCFGID_ORIENTATION, &cOrientation);

		float dx = cLength.operator float()*sinf(cOrientation.operator float()*3.14159265359f/180.0f);
		float dy = -cLength.operator float()*cosf(cOrientation.operator float()*3.14159265359f/180.0f);
		if (a_pRect)
		{
			if (dx > 0.0f)
				a_pRect->tTL.nX -= ceilf(dx);
			else
				a_pRect->tBR.nX += ceilf(-dx);

			if (dy > 0.0f)
				a_pRect->tTL.nY -= ceilf(dy);
			else
				a_pRect->tBR.nY += ceilf(-dy);
		}
		return S_OK;
	}

	// ILayerStyle methods
public:
	STDMETHOD_(BYTE, ExecutionPriority)() { return ELSEPShadow; }
	STDMETHOD(IsPriorityAnchor)() { return S_FALSE; }

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			//CConfigValue cType;
			//CConfigValue cOrder;
			//CConfigValue cOrientation;
			//if (a_pConfig)
			//{
			//	a_pConfig->ItemValueGet(CComBSTR(CFGID_TYPE), &cType);
			//	a_pConfig->ItemValueGet(CComBSTR(CFGID_ORDER), &cOrder);
			//	a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cOrientation);
			//}
			//if (cType.TypeGet() == ECVTInteger && cType.operator LONG() == CFGVAL_TP_LINEREFLECTION)
			//{
			//	float fOri = cOrientation;
			//	if (fOri == -180 || fOri == 180 || fOri == 0)
			//		*a_ppName = new CMultiLanguageString(L"[0409]Vertical reflection[0405]Vertikální zrcadlení");
			//	else if (fOri == -90 || fOri == 90)
			//		*a_ppName = new CMultiLanguageString(L"[0409]Horizontal reflection[0405]Horizontální zrcadlení");
			//	else
			//		*a_ppName = new CMultiLanguageString(L"[0409]Line symmetry[0405]Osová symetrie");
			//	return S_OK;
			//}
			//if (cType.TypeGet() == ECVTInteger && cType.operator LONG() == CFGVAL_TP_ROTATIONAL)
			//{
			//	CComObject<CPrintfLocalizedString>* p = NULL;
			//	CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			//	CComPtr<ILocalizedString> pStr = p;
			//	p->Init(CMultiLanguageString::GetAuto(L"[0409]Rotation symmetry (%i°)[0405]Rotační symetrie (%i°)"), LONG(360.0f/cOrder.operator LONG()+0.5f));
			//	*a_ppName = pStr;
			//	pStr.Detach();
			//	return S_OK;
			//}
			*a_ppName = new CMultiLanguageString(L"[0409]Long shadow[0405]Dlouhý stín");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(PreviewIconID)(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
	{
		if (a_pIconID == NULL)
			return E_POINTER;
		//// {0642BA6C-79A5-4b17-A006-2BBDE1B86465}
		//static GUID const tIDHorizontalReflection =  {0x642ba6c, 0x79a5, 0x4b17, {0xa0, 0x6, 0x2b, 0xbd, 0xe1, 0xb8, 0x64, 0x65}};
		*a_pIconID = CLSID_DocumentOperationLongShadow;//tIDHorizontalReflection;
		return S_OK;
	}

	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
				IRPathPoint const sphere[] =
				{
					{192, 96, 0, -53.0193, 0, 53.0193},
					{96, 0, -53.0193, 0, 53.0193, 0},
					{0, 96, 0, 53.0193, 0, -53.0193},
					{96, 192, 53.0193, 0, -53.0193, 0},
				};
				IRPathPoint const highlight[] =
				{
					{73, 50, -4.79583, -5.71544, 4.79583, 5.71544},
					{51, 50, -6.90661, 5.79534, 6.90661, -5.79534},
					{47, 70, 4.79583, 5.71544, -4.79583, -5.71544},
					{69, 70, 6.90661, -5.79534, -6.90661, 5.79534},
				};
				IRPathPoint const shade[] =
				{
					{178, 60, 13.8071, 30.3757, -9, 70},
					{159, 159, -23.7482, 23.7482, 23.7482, -23.7482},
					{60, 178, 70, -9, 30.3757, 13.8071},
				};
				IRPathPoint const shadow[] =
				{
					{92, 228, 37.4903, 37.4903, 0, 0},
					{228, 228, 37.4903, -37.4903, -37.4903, 37.4903},
					{228, 92, 0, 0, 37.4903, 37.4903},

					{164, 28, 0, 0, 0, 0},
					{28, 164, 0, 0, 0, 0},

					//{120, 256, 0, 0, 0, 0},
					//{256, 256, 0, 0, 0, 0},
					//{256, 120, 0, 0, 0, 0},
				};

				IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
				IRFill matWhite(0x7fffffff);
				IRFill matBlack(0x7f000000);
				IRFill matShadow(0x41000000);
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&canvas, itemsof(shadow), shadow, &matShadow);
				cRenderer(&canvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme1Color2));
				cRenderer(&canvas, itemsof(highlight), highlight, &matWhite);
				cRenderer(&canvas, itemsof(shade), shade, &matBlack);
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// ICanvasInteractingOperation
public:
	STDMETHOD(CreateWrapper)(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY, ICanvasInteractingWrapper** a_ppWrapper)
	{
		if (a_ppWrapper == NULL)
			return E_POINTER;
		try
		{
			CComObject<CWrapper>* p = NULL;
			CComObject<CWrapper>::CreateInstance(&p);
			CComPtr<ICanvasInteractingWrapper> pNew = p;
			p->Init(a_nSizeX, a_nSizeY, a_pConfig);
			*a_ppWrapper = pNew.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Long shadow[0405]Rastrový obrázek - dlouhý stín");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_LENGTH), CMultiLanguageString::GetAuto(L"[0409]Length[0405]Délka"), NULL, CConfigValue(64.0f), NULL, 0, NULL);

			pCfgInit->ItemInsRanged(CComBSTR(CFGID_ORIENTATION), CMultiLanguageString::GetAuto(L"[0409]Orientation[0405]Orientace"), NULL, CConfigValue(135.0f), NULL, CConfigValue(0.0f), CConfigValue(360.0f), CConfigValue(0.0f), 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_COLOR), CMultiLanguageString::GetAuto(L"[0409]Color[0405]Barva"), NULL, CConfigValue(0.0f, 0.0f, 0.0f, 0.5f), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_DocumentOperationLongShadow, CConfigGUILongShadowDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (a_pDocument == NULL)
				return S_FALSE;
			//CComPtr<IDocumentVectorImage> pDVI;
			//a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			//if (pDVI != NULL)
			//	return S_OK;
			CComPtr<IDocumentRasterImage> pDRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));
			if (pDRI != NULL)
				return S_OK;
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IDocumentRasterImage> pDRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));
			if (pDRI == NULL)
				return E_FAIL;
			TImageSize tCanvas = {0, 0};
            //TImageResolution *a_pResolution;
			TImagePoint tOrigin = {0, 0};
			TImageSize tContent = {0, 0};
            //EImageOpacity *a_pContentOpacity;
			pDRI->CanvasGet(&tCanvas, NULL, &tOrigin, &tContent, NULL);

			TPixelChannel tDefault;
			float fGamma = 2.2f;
			pDRI->ChannelsGet(NULL, &fGamma, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

			CConfigValue cLength;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_LENGTH), &cLength);
			float const length = cLength;
			CConfigValue cAngle;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cAngle);
			float dx = length*sinf(cAngle.operator float()*3.14159265359f/180.0f);
			float dy = -length*cosf(cAngle.operator float()*3.14159265359f/180.0f);
			LONG nx = dx >= 0 ? ceilf(dx) : -ceilf(-dx);
			LONG ny = dy >= 0 ? ceilf(dy) : -ceilf(-dy);

			TImageSize tDstSize = {nx < 0 ? tContent.nX-nx : tContent.nX+nx, ny < 0 ? tContent.nY-ny : tContent.nY+ny};
			TImagePoint tDstOrigin = {nx < 0 ? tOrigin.nX+nx : tOrigin.nX, ny < 0 ? tOrigin.nY+ny : tOrigin.nY};

			CAutoPixelBuffer cDst(pDRI, tDstSize);
			ULONGLONG totalA = 0;

			CComPtr<IGammaTableCache> pGTC;
			RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
			CGammaTables const* pGT = pGTC->GetSRGBTable();
			CConfigValue cColor;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_COLOR), &cColor);
			TPixelChannel shadowColor = {CGammaTables::ToSRGB(cColor[2]), CGammaTables::ToSRGB(cColor[1]), CGammaTables::ToSRGB(cColor[0]), cColor[3]*255.0f+0.5f};
			struct TGammaPixel {WORD wR, wG, wB;};
			TGammaPixel const tShadowGamma = {pGT->m_aGamma[shadowColor.bR], pGT->m_aGamma[shadowColor.bG], pGT->m_aGamma[shadowColor.bB]};

			{
				SLockedImageBuffer cSrc(pDRI);
				TPixelChannel const* pSrc = cSrc.Content();
				TPixelChannel* pDst = cDst.Buffer();
				int dSrcY;
				int dDstY;
				int dSrcX;
				int dDstX;
				float dSkew;
				ULONG eX0;
				ULONG eX1;
				ULONG eY0;
				ULONG eY1;
				float adjLen;
				if (fabsf(dy) >= fabsf(dx))
				{
					if (ny >= 0)
					{
						dSrcY = cSrc.tAllocSize.nX;
						dDstY = tDstSize.nX;
					}
					else
					{
						dSrcY = -cSrc.tAllocSize.nX;
						pSrc += (cSrc.tContentSize.nY-1)*cSrc.tAllocSize.nX;
						dDstY = -tDstSize.nX;
						pDst += (tDstSize.nY-1)*tDstSize.nX;
					}
					if (nx >= 0)
					{
						dSrcX = 1;
						dDstX = 1;
					}
					else
					{
						dSrcX = -1;
						pSrc += cSrc.tContentSize.nX-1;
						dDstX = -1;
						pDst += tDstSize.nX-1;
					}
					dSkew = fabsf(dx/dy);
					adjLen = fabsf(dy);
					eX0 = cSrc.tContentSize.nX;
					eX1 = tDstSize.nX;
					eY0 = cSrc.tContentSize.nY;
					eY1 = tDstSize.nY;
				}
				else
				{
					if (ny >= 0)
					{
						dSrcX = cSrc.tAllocSize.nX;
						dDstX = tDstSize.nX;
					}
					else
					{
						dSrcX = -cSrc.tAllocSize.nX;
						pSrc += (cSrc.tContentSize.nY-1)*cSrc.tAllocSize.nX;
						dDstX = -tDstSize.nX;
						pDst += (tDstSize.nY-1)*tDstSize.nX;
					}
					if (nx >= 0)
					{
						dSrcY = 1;
						dDstY = 1;
					}
					else
					{
						dSrcY = -1;
						pSrc += cSrc.tContentSize.nX-1;
						dDstY = -1;
						pDst += tDstSize.nX-1;
					}
					dSkew = fabsf(dy/dx);
					adjLen = fabsf(dx);
					eX0 = cSrc.tContentSize.nY;
					eX1 = tDstSize.nY;
					eY0 = cSrc.tContentSize.nX;
					eY1 = tDstSize.nX;
				}
				TPixelChannel const* pSrcY = pSrc;
				TPixelChannel* pDstY = pDst;
				CAutoVectorPtr<float> dist(new float[eX1+eY1+1]);
				std::fill_n(dist.m_p, eX1+eY1+1, - 2*255*adjLen); // distance buffer starts with "far away"
				for (ULONG iY = 0; iY < eY0; ++iY)
				{
					int off = eY1-(iY*dSkew+0.5f);
					TPixelChannel const* pSrcX = pSrcY;
					TPixelChannel* pDstX = pDstY;
					for (ULONG iX = 0; iX < eX0; ++iX)
					{
						if (pSrcX->bA == 0)
						{
							if (dist[iX+off] > (iY-adjLen)*255)
								*pDstX = shadowColor;
							else
								*pDstX = tDefault;
						}
						else
						{
							float f = iY*255+pSrcX->bA;
							if (dist[iX+off] < f)
								dist[iX+off] = f;
							if (pSrcX->bA == 255 || dist[iX+off] <= (iY-adjLen)*255)
							{
								*pDstX = *pSrcX;
							}
							else
							{
								ULONG a = pSrcX->bA;
								DWORD r = pGT->m_aGamma[pSrcX->bR]*a;
								DWORD g = pGT->m_aGamma[pSrcX->bG]*a;
								DWORD b = pGT->m_aGamma[pSrcX->bB]*a;
								ULONG ia = shadowColor.bA*(255-a);
								ULONG totA = a*255+ia;
								pDstX->bA = totA/255;
								pDstX->bR = pGT->InvGamma((r*255+tShadowGamma.wR*ia)/totA);
								pDstX->bG = pGT->InvGamma((g*255+tShadowGamma.wG*ia)/totA);
								pDstX->bB = pGT->InvGamma((b*255+tShadowGamma.wB*ia)/totA);
							}
						}
						pSrcX += dSrcX;
						pDstX += dDstX;
					}
					for (ULONG iX = eX0; iX < eX1; ++iX)
					{
						if (dist[iX+off] > (iY-adjLen)*255)
							*pDstX = shadowColor;
						else
							*pDstX = tDefault;
						//*pDstX = tDefault;
						pDstX += dDstX;
					}
					pSrcY += dSrcY;
					pDstY += dDstY;
				}
				for (ULONG iY = eY0; iY < eY1; ++iY)
				{
					int off = eY1-(iY*dSkew+0.5f);
					TPixelChannel* pDstX = pDstY;
					for (ULONG iX = 0; iX < eX1; ++iX)
					{
						if (dist[iX+off] > (iY-adjLen)*255)
							*pDstX = shadowColor;
						else
							*pDstX = tDefault;
						//*pDstX = shadowColor;
						pDstX += dDstX;
					}
					pDstY += dDstY;
				}
			}

			return cDst.Replace(tDstOrigin, &tDstOrigin, &tDstSize, &totalA, tDefault);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CWrapper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ICanvasInteractingWrapper
	{
	public:
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, float a_fLength, float a_fAngle)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			m_tOrigin.fX = 0.5f*m_nSizeX;
			m_tOrigin.fY = 0.5f*m_nSizeY;

			m_fLength = a_fLength;
			m_fAngle = a_fAngle;
			m_tDirection.fX = m_tOrigin.fX + m_fLength*sinf(m_fAngle*3.14159265359f/180.0f);
			m_tDirection.fY = m_tOrigin.fY - m_fLength*cosf(m_fAngle*3.14159265359f/180.0f);
		}
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			m_tOrigin.fX = 0.5f*m_nSizeX;
			m_tOrigin.fY = 0.5f*m_nSizeY;

			CConfigValue cLength;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_LENGTH), &cLength);
			m_fLength = cLength;

			CConfigValue cOrientation;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cOrientation);
			m_fAngle = cOrientation;
			m_tDirection.fX = m_tOrigin.fX + m_fLength*sinf(m_fAngle*3.14159265359f/180.0f);
			m_tDirection.fY = m_tOrigin.fY - m_fLength*cosf(m_fAngle*3.14159265359f/180.0f);
		}

	BEGIN_COM_MAP(CWrapper)
		COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
	END_COM_MAP()

		// ICanvasInteractingWrapper methods
	public:
		STDMETHOD_(ULONG, GetControlPointCount)()
		{
			return 1;
		}
		STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
		{
			if (a_nIndex == 0)
			{
			//	if (a_pPos)
			//		*a_pPos = m_tOrigin;
			//	if (a_pClass)
			//		*a_pClass = 0;
			//	return S_OK;
			//}
			//if (a_nIndex == 1)
			//{
				if (a_pPos)
					*a_pPos = m_tDirection;
				if (a_pClass)
					*a_pClass = 1;
				return S_OK;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel)
		{
			if (a_ppNew == NULL)
				return E_POINTER;
			try
			{
				if (a_pNewSel)
					*a_pNewSel = a_nIndex;
				if (a_nIndex == 0)
				{
				//	if (a_pPos->fX != m_tOrigin.fX || a_pPos->fY != m_tOrigin.fY) // or use epsilon? ...not that important
				//	{
				//		CComObject<CWrapper>* p = NULL;
				//		CComObject<CWrapper>::CreateInstance(&p);
				//		CComPtr<ICanvasInteractingWrapper> pNew = p;
				//		p->Init(m_nSizeX, m_nSizeY, m_nType, m_nOrder, *a_pPos, m_fAngle);
				//		*a_ppNew = pNew.Detach();
				//		return S_OK;
				//	}
				//	(*a_ppNew = this)->AddRef();
				//	return S_FALSE;
				//}
				//if (a_nIndex == 1)
				//{
					float fDX = a_pPos->fX-m_tOrigin.fX;
					float fDY = a_pPos->fY-m_tOrigin.fY;
					if (fabsf(fDX) < 0.01f && fabsf(fDY) < 0.01)
					{
						(*a_ppNew = this)->AddRef();
						return S_FALSE;
					}
					float fAngle = atan2(fDX, -fDY)*180.0f/3.14159265359f;
					if (fAngle < 0.0f) fAngle += 360.0f;
					float fLength = sqrtf(fDX*fDX+fDY*fDY);
					if (fabsf(m_fAngle-fAngle) > 1e-5f || fabsf(m_fLength-fLength) > 1e-5f)
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, fLength, fAngle);
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				return E_RW_INDEXOUTOFRANGE;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
		{
			a_pLines->MoveTo(m_tOrigin.fX, m_tOrigin.fY);
			a_pLines->LineTo(m_tDirection.fX, m_tDirection.fY);
			return S_OK;
		}

		STDMETHOD(ToConfig)(IConfig* a_pConfig)
		{
			CComBSTR cLength(CFGID_LENGTH);
			CComBSTR cOrientation(CFGID_ORIENTATION);
			BSTR aIDs[2] = {cLength, cOrientation};
			TConfigValue aVals[2];
			aVals[0] = CConfigValue(m_fLength);
			aVals[1] = CConfigValue(m_fAngle);
			return a_pConfig->ItemValuesSet(2, aIDs, aVals);
		}

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		float m_fLength;
		TPixelCoords m_tOrigin;
		float m_fAngle;
		TPixelCoords m_tDirection;
	};
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationLongShadow, CDocumentOperationLongShadow)
