
#include "stdafx.h"
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include <RWDocumentImageVector.h>
#include <RWProcessingTags.h>
#include <algorithm>
#include <deque>
#include <SharedStateUndo.h>

#include "PathUtils.h"
#include "../RWOperationImageRaster/AGGBuffer.h"
#include <agg_trans_perspective.h>
#include <agg_span_interpolator_trans.h>

#include <IconRenderer.h>


static OLECHAR const CFGID_TYPE[] = L"Type";
static LONG const CFGVAL_TP_LINEREFLECTION = 0;
static LONG const CFGVAL_TP_ROTATIONAL = 1;
static OLECHAR const CFGID_ORDER[] = L"Order";
static OLECHAR const CFGID_ORIGIN[] = L"Origin";
static OLECHAR const CFGID_ORIENTATION[] = L"Orientation";
static OLECHAR const CFGID_LASTSIZE[] = L"LastSize";
static OLECHAR const CFGID_CLIP[] = L"Clip";
static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISymmetryDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUISymmetryDlg>,
	public CDialogResize<CConfigGUISymmetryDlg>
{
public:
	enum
	{
		IDC_SYMTYPE = 100,
		IDC_ORIGIN,
		IDC_ORDER_LABEL,
		IDC_ORDER,
		IDC_ORIENTATION_EDIT,
		IDC_ORIENTATION_SLIDER,
		IDC_CLIP,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 72, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Type:[0405]Typ:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_SYMTYPE, 50, 0, 50, 30, CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Order:[0405]Řád:"), IDC_ORDER_LABEL, 0, 18, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ORDER, 50, 16, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Center:[0405]Střed:"), IDC_STATIC, 0, 34, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ORIGIN, 50, 32, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Orientation:[0405]Orientace:"), IDC_STATIC, 0, 50, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_ORIENTATION_EDIT, 50, 48, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_ORIENTATION_SLIDER, TRACKBAR_CLASS, TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 0, 60, 100, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUISymmetryDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISymmetryDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUISymmetryDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISymmetryDlg)
		DLGRESIZE_CONTROL(IDC_SYMTYPE, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORIGIN, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORIENTATION_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ORIENTATION_SLIDER, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISymmetryDlg)
		CONFIGITEM_COMBOBOX(IDC_SYMTYPE, CFGID_TYPE)
		CONFIGITEM_EDITBOX(IDC_ORDER, CFGID_ORDER)
		CONFIGITEM_EDITBOX(IDC_ORIGIN, CFGID_ORIGIN)
		CONFIGITEM_EDITBOX(IDC_ORIENTATION_EDIT, CFGID_ORIENTATION)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_ORIENTATION_SLIDER, CFGID_ORIENTATION)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};


// CDocumentOperationSymmetry

// {6AB86929-DFF5-41f4-BEBA-8321E180CAF8}
extern GUID const CLSID_DocumentOperationSymmetry = {0x6ab86929, 0xdff5, 0x41f4, {0xbe, 0xba, 0x83, 0x21, 0xe1, 0x80, 0xca, 0xf8}};

class ATL_NO_VTABLE CDocumentOperationSymmetry :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSymmetry, &CLSID_DocumentOperationSymmetry>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl,
	public ICanvasInteractingOperation,
	public ILayerStyle
{
public:
	CDocumentOperationSymmetry()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationSymmetry)

BEGIN_CATEGORY_MAP(CDocumentOperationSymmetry)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageRearragement)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSymmetry)
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
			CComBSTR bstrCFGID_LASTSIZE(CFGID_LASTSIZE);
			CComBSTR bstrCFGID_ORIGIN(CFGID_ORIGIN);
			CConfigValue cLastSize;
			a_pConfig->ItemValueGet(bstrCFGID_LASTSIZE, &cLastSize);
			CConfigValue cOrigin;
			a_pConfig->ItemValueGet(bstrCFGID_ORIGIN, &cOrigin);
			if (a_pContentTransform)
			{
				TVector2f t = {cOrigin[0], cOrigin[1]};
				t = TransformVector2(*a_pContentTransform, t);
				cOrigin = CConfigValue(t.x, t.y);
			}
			if (a_pCanvas && (a_pCanvas->nX != cLastSize[0] || a_pCanvas->nY != cLastSize[1]))
			{
				if (a_pContentTransform == NULL && cLastSize[0] != 0.0f && cLastSize[0] != 0.0f)
				{
					TVector2f t = {cOrigin[0]*a_pCanvas->nX/cLastSize[0], cOrigin[1]*a_pCanvas->nY/cLastSize[1]};
					cOrigin = CConfigValue(t.x, t.y);
				}
				cLastSize = CConfigValue(a_pCanvas->nX, a_pCanvas->nY);
			}
			BSTR aIDs[2] = {bstrCFGID_LASTSIZE, bstrCFGID_ORIGIN};
			TConfigValue aVals[2] = {cLastSize, cOrigin};
			a_pConfig->ItemValuesSet(2, aIDs, aVals);
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		if (a_pCanvas && a_pRect)
		{
			if (a_pRect->tTL.nX > 0) a_pRect->tTL.nX = 0;
			if (a_pRect->tTL.nY > 0) a_pRect->tTL.nY = 0;
			if (a_pRect->tBR.nX < LONG(a_pCanvas->nX)) a_pRect->tBR.nX = a_pCanvas->nX;
			if (a_pRect->tBR.nY < LONG(a_pCanvas->nY)) a_pRect->tBR.nY = a_pCanvas->nY;
		}
		return S_OK;
		// TODO: implement correctly
		if (a_pRect)
		{
			a_pRect->tTL.nX = a_pRect->tTL.nY = LONG_MIN;
			a_pRect->tBR.nX = a_pRect->tBR.nY = LONG_MAX;
		}
		return S_OK;
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{ return AdjustDirtyRect(a_pConfig, a_pCanvas, a_pRect); }
	STDMETHOD(Process)(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix)
	{
		try
		{
			//CComPtr<IDocumentEditableImage> pEI;
			//a_pSrc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
			//if (pEI == NULL)
			//	return E_FAIL;

			a_pSrc->DocumentCopy(a_bstrPrefix, a_pDst, NULL, NULL);
			CComPtr<IDocument> pDstDoc;
			a_pDst->DataBlockDoc(a_bstrPrefix, &pDstDoc);
			return Activate(NULL, pDstDoc, a_pConfig, NULL, NULL, 0);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// ILayerStyle methods
public:
	STDMETHOD_(BYTE, ExecutionPriority)() { return ELSEPRearrange - 3; }
	STDMETHOD(IsPriorityAnchor)() { return S_FALSE; }

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			CConfigValue cType;
			CConfigValue cOrder;
			CConfigValue cOrientation;
			if (a_pConfig)
			{
				a_pConfig->ItemValueGet(CComBSTR(CFGID_TYPE), &cType);
				a_pConfig->ItemValueGet(CComBSTR(CFGID_ORDER), &cOrder);
				a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cOrientation);
			}
			if (cType.TypeGet() == ECVTInteger && cType.operator LONG() == CFGVAL_TP_LINEREFLECTION)
			{
				float fOri = cOrientation;
				if (fOri == -180 || fOri == 180 || fOri == 0)
					*a_ppName = new CMultiLanguageString(L"[0409]Vertical reflection[0405]Vertikální zrcadlení");
				else if (fOri == -90 || fOri == 90)
					*a_ppName = new CMultiLanguageString(L"[0409]Horizontal reflection[0405]Horizontální zrcadlení");
				else
					*a_ppName = new CMultiLanguageString(L"[0409]Line symmetry[0405]Osová symetrie");
				return S_OK;
			}
			if (cType.TypeGet() == ECVTInteger && cType.operator LONG() == CFGVAL_TP_ROTATIONAL)
			{
				CComObject<CPrintfLocalizedString>* p = NULL;
				CComObject<CPrintfLocalizedString>::CreateInstance(&p);
				CComPtr<ILocalizedString> pStr = p;
				p->Init(CMultiLanguageString::GetAuto(L"[0409]Rotation symmetry (%i°)[0405]Rotační symetrie (%i°)"), LONG(360.0f/cOrder.operator LONG()+0.5f));
				*a_ppName = pStr;
				pStr.Detach();
				return S_OK;
			}
			*a_ppName = new CMultiLanguageString(L"[0409]Symmetry[0405]Symetrie");
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
		// {0642BA6C-79A5-4b17-A006-2BBDE1B86465}
		static GUID const tIDHorizontalReflection =  {0x642ba6c, 0x79a5, 0x4b17, {0xa0, 0x6, 0x2b, 0xbd, 0xe1, 0xb8, 0x64, 0x65}};
		*a_pIconID = tIDHorizontalReflection;
		return S_OK;
	}
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetDefaultIcon(a_nSize);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	static HICON GetDefaultIcon(ULONG a_nSize)
	{
		IRPathPoint const petals[] =
		{
			{188.14, 25.18, 2.43732, 7.04113, -8.73, -25.22},
			{180.507, 55.7302, 9.77226, -9.98082, 6.47249, -12.3782},
			{207.203, 39.0303, 26.6833, 0.509312, -7.44969, -0.142197},
			{238.703, 92.0305, 8.9924, 27.6758, -8.99239, -27.6758},
			{244.372, 153.424, -5.94357, 4.49402, 21.2879, -16.0961},
			{212.956, 155.604, 12.5126, 6.20988, 13.7732, 2.33096},
			{237.09, 175.834, 7.76123, 25.5348, -2.16692, -7.12924},
			{196.418, 222.17, -23.5424, 17.1045, 23.5424, -17.1046},
			{139.782, 246.532, -6.11075, -4.26395, 21.8867, 15.272},
			{128, 217.328, -2.03929, 13.8193, 2.03928, 13.8193},
			{116.218, 246.532, -21.8867, 15.272, 6.11076, -4.26395},
			{59.5818, 222.17, -23.5424, -17.1045, 23.5424, 17.1046},
			{18.9096, 175.834, 2.16692, -7.12929, -7.76121, 25.5348},
			{43.0435, 155.604, -13.7732, 2.33096, -12.5128, 6.20992},
			{11.6281, 153.424, -21.2879, -16.0961, 5.94358, 4.49402},
			{17.297, 92.0305, 8.9924, -27.6758, -8.99239, 27.6758},
			{48.7967, 39.0303, 7.45015, -0.142204, -26.6833, 0.509319},
			{75.495, 55.7322, -6.47324, -12.379, -9.77294, -9.98178},
			{67.86, 25.18, 8.73, -25.22, -2.43747, 7.04156},
			{128, 11.6, 29.1, 0, -29.1, 0},
		};
		IRPathPoint const center[] =
		{
			{168, 128, 0, -22.0914, 0, 22.0914},
			{128, 88, -22.0914, 0, 22.0914, 0},
			{88, 128, 0, 22.0914, 0, -22.0914},
			{128, 168, 22.0914, 0, -22.0914, 0},
		};
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
		CIconRendererReceiver cRenderer(a_nSize);
		cRenderer(&canvas, itemsof(petals), petals, pSI->GetMaterial(ESMScheme1Color1));
		cRenderer(&canvas, itemsof(center), center, pSI->GetMaterial(ESMScheme1Color3));
		return cRenderer.get();
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
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Symmetry[0405]Vektorový obrázek - symetrie");
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

			CComBSTR cCFGID_TYPE(CFGID_TYPE);
			pCfgInit->ItemIns1ofN(cCFGID_TYPE, CMultiLanguageString::GetAuto(L"[0409]Type[0405]Typ"), NULL, CConfigValue(CFGVAL_TP_LINEREFLECTION), NULL);
			pCfgInit->ItemOptionAdd(cCFGID_TYPE, CConfigValue(CFGVAL_TP_LINEREFLECTION), CMultiLanguageString::GetAuto(L"[0409]Line symmetry[0405]Osová symetrie"), 0, NULL);
			pCfgInit->ItemOptionAdd(cCFGID_TYPE, CConfigValue(CFGVAL_TP_ROTATIONAL), CMultiLanguageString::GetAuto(L"[0409]Rotation symmetry[0405]Rotační symetrie"), 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_ORDER), CMultiLanguageString::GetAuto(L"[0409]Order[0405]Řád"), NULL, CConfigValue(2L), NULL, 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_ORIGIN), CMultiLanguageString::GetAuto(L"[0409]Origin[0405]Počátek"), NULL, CConfigValue(0.0f, 0.0f), NULL, 0, NULL);
			pCfgInit->ItemInsRanged(CComBSTR(CFGID_ORIENTATION), CMultiLanguageString::GetAuto(L"[0409]Orientation[0405]Orientace"), NULL, CConfigValue(0.0f), NULL, CConfigValue(-180.0f), CConfigValue(180.0f), CConfigValue(0.0f), 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_CLIP), CMultiLanguageString::GetAuto(L"[0409]Clip overflowing content[0405]Oříznout přesahující obsah"), NULL, CConfigValue(false), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_LASTSIZE), NULL, NULL, CConfigValue(0.0f, 0.0f), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_DocumentOperationSymmetry, CConfigGUISymmetryDlg>::FinalizeConfig(pCfgInit);

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
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI != NULL)
				return S_OK;
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
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
			{
				a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));
				if (pDRI == NULL)
					return E_FAIL;
			}
			CComPtr<IDocumentImage> pDI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
			TImageSize tCanvas = {0, 0};
            //TImageResolution *a_pResolution;
			TImagePoint tOrigin = {0, 0};
			TImageSize tContent = {0, 0};
            //EImageOpacity *a_pContentOpacity;
			pDI->CanvasGet(&tCanvas, NULL, &tOrigin, &tContent, NULL);

			TPixelChannel tDefault;
			float fGamma = 2.2f;
			pDI->ChannelsGet(NULL, &fGamma, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

			std::vector<TMatrix3x3f> xforms;
			CConfigValue cType;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_TYPE), &cType);
			CConfigValue cOrigin;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIGIN), &cOrigin);
			CConfigValue cAngle;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cAngle);
			float fAngle = cAngle;
			if (fAngle < -90)
				fAngle += 270;
			else
				fAngle -= 90;
			CConfigValue cLastSize;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_LASTSIZE), &cLastSize);
			if (cLastSize[0] != tCanvas.nX || cLastSize[1] != tCanvas.nY)
				cOrigin = CConfigValue(tCanvas.nX*0.5f, tCanvas.nY*0.5f);

			Point t00(tOrigin.nX, tOrigin.nY);
			Point t10(tOrigin.nX+tContent.nX, tOrigin.nY);
			Point t11(tOrigin.nX+tContent.nX, tOrigin.nY+tContent.nY);
			Point t01(tOrigin.nX, tOrigin.nY+tContent.nY);
			std::vector<Halfplane> srcPlanes;
			srcPlanes.push_back(Halfplane(t00, t10));
			srcPlanes.push_back(Halfplane(t10, t11));
			srcPlanes.push_back(Halfplane(t11, t01));
			srcPlanes.push_back(Halfplane(t01, t00));
			if (cType.operator LONG() == CFGVAL_TP_LINEREFLECTION)
			{
				float const fSin = sinf(fAngle*3.14159265359f/90.0f);
				float const fCos = cosf(fAngle*3.14159265359f/90.0f);
				TMatrix3x3f const tRotation = {fCos, fSin, 0.0f, fSin, -fCos, 0.0f, 0.0f, 0.0f, 1.0f};
				TMatrix3x3f const tTransPre = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -cOrigin[0], -cOrigin[1], 1.0f};
				TMatrix3x3f const tTransPost = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, cOrigin[0], cOrigin[1], 1.0f};
				TMatrix3x3f tmp1;
				Matrix3x3fMultiply(tTransPre, tRotation, &tmp1);
				TMatrix3x3f tmp2;
				Matrix3x3fMultiply(tmp1, tTransPost, &tmp2);
				xforms.push_back(tmp2);
				float fA = fAngle;
				float const fS = sinf((fAngle+90)*3.14159265359f/180.0f);
				float const fC = cosf((fAngle+90)*3.14159265359f/180.0f);
				srcPlanes.push_back(Halfplane(Point(cOrigin[0]+0.00390625f*fC, cOrigin[1]+0.00390625f*fS), (fA > 0 ? fA-180 : fA+180)*3.14159265359f/180.0f));
			}
			else if (cType.operator LONG() == CFGVAL_TP_ROTATIONAL)
			{
				CConfigValue cOrder;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_ORDER), &cOrder);
				LONG nOrder = cOrder;
				if (nOrder < 2)
					nOrder = 2;
				else if (nOrder > 18)
					nOrder = 18;
				TMatrix3x3f const tTransPre = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -cOrigin[0], -cOrigin[1], 1.0f};
				TMatrix3x3f const tTransPost = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, cOrigin[0], cOrigin[1], 1.0f};
				for (LONG o = 1; o < nOrder; ++o)
				{
					float const fSin = sinf(3.14159265359f*2*o/nOrder);
					float const fCos = cosf(3.14159265359f*2*o/nOrder);
					TMatrix3x3f const tRotation = {fCos, fSin, 0.0f, -fSin, fCos, 0.0f, 0.0f, 0.0f, 1.0f};
					TMatrix3x3f tmp1;
					Matrix3x3fMultiply(tTransPre, tRotation, &tmp1);
					TMatrix3x3f tmp2;
					Matrix3x3fMultiply(tmp1, tTransPost, &tmp2);
					xforms.push_back(tmp2);
				}
				float fA = fAngle/180.0f;
				float fS = sinf((fAngle+90)*3.14159265359f/180.0f);
				float fC = cosf((fAngle+90)*3.14159265359f/180.0f);
				srcPlanes.push_back(Halfplane(Point(cOrigin[0]+0.00390625f*fC, cOrigin[1]+0.00390625f*fS), (fA > 0 ? fA-1 : fA+1)*3.14159265359f));
				if (nOrder > 2)
				{
					fS = sinf((fAngle+90-360.0f/nOrder)*3.14159265359f/180.0f);
					fC = cosf((fAngle+90-360.0f/nOrder)*3.14159265359f/180.0f);
					srcPlanes.push_back(Halfplane(Point(cOrigin[0]-0.00390625f*fC, cOrigin[1]-0.00390625f*fS), (fA-2.0f/nOrder < -1 ? fA-2.0f/nOrder+2 : fA-2.0f/nOrder)*3.14159265359f));
				}
			}
			std::vector<Point> srcPoly;
			hp_intersect(srcPlanes, srcPoly);
			if (srcPoly.size() < 3)
			{
				// invalid symmetry source (should never happen)
				if (pDVI)
				{
					std::vector<ULONG> aIDs;
					pDVI->ObjectIDs(CEnumToVector<IEnum2UInts, ULONG>(aIDs));

					CConfigValue cSyncID;
					a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
					CComBSTR bstrState;
					pDVI->StatePrefix(&bstrState);
					bstrState.Append(cSyncID.operator BSTR());
					if (a_pStates)
						CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrState);
					for (std::vector<ULONG>::const_iterator i = aIDs.begin(); i != aIDs.end(); ++i)
					{
						ULONG id = *i;
						pDVI->ObjectSet(&id, NULL, NULL);
					}
					if (a_pStates)
					{
						CComPtr<ISharedState> pNewState;
						pDVI->StatePack(0, NULL, &pNewState);
						a_pStates->StateSet(bstrState, pNewState);
					}
					return S_OK;
				}
				return pDRI->TileSet(EICIRGBA, NULL, NULL, NULL, 0, NULL, TRUE);
			}

			if (pDVI)
			{
				std::vector<ULONG> aIDs;
				pDVI->ObjectIDs(CEnumToVector<IEnum2UInts, ULONG>(aIDs));

				CConfigValue cSyncID;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
				CComBSTR bstrState;
				pDVI->StatePrefix(&bstrState);
				bstrState.Append(cSyncID.operator BSTR());

				std::vector<ULONG> aSelIDs;

				if (a_pStates)
				{
					CComPtr<ISharedState> pState;
					a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
					if (pState)
						pDVI->StateUnpack(pState, CEnumToVector<IEnum2UInts, ULONG>(aSelIDs));
					CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrState);
				}

				CComBSTR bstrPathID(L"SHAPE");
				CComBSTR bstrPathParams;
				CComObjectStackEx<CToolWindow> cWnd;

				CComPtr<IRasterImageEditToolsManager> pToolMgr;
				RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));

				for (std::vector<ULONG>::const_iterator i = aIDs.begin(); i != aIDs.end(); ++i)
				{
					ULONG objectID = *i;

					CBezierPaths srcBeziers;

					CComBSTR bstrID;
					CComBSTR bstrParams;
					HRESULT hRes = pDVI->ObjectGet(objectID, &bstrID, &bstrParams);
					if (FAILED(hRes)) continue;
					xforms.insert(xforms.begin(), TMATRIX3X3F_IDENTITY);

					ClipperLib::Clipper c2;
					c2.ZFillFunction(&CoordZFill);
					c2.StrictlySimple(true);

					for (std::vector<TMatrix3x3f>::const_iterator j = xforms.begin(); j != xforms.end(); ++j)
					{
						CComPtr<IRasterImageEditTool> pTool;
						pToolMgr->EditToolCreate(bstrID, NULL, &pTool);
						if (pTool == NULL) continue;
						CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
						if (pToolScript == NULL) continue;
						CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
						if (pToolPoly == NULL) continue;
						pTool->Init(&cWnd);
						pToolScript->FromText(bstrParams);

						if (j != xforms.begin())
							pTool->Transform(&(*j));

						ClipperLib::Paths srcPolys;
						CPolygonReceiver cReceiver(srcBeziers, srcPolys);
						pToolPoly->ToPath(&cReceiver);

						ClipperLib::Paths clip;
						ClipperLib::Path cp;
						for (std::vector<Point>::const_iterator i = srcPoly.begin(); i != srcPoly.end(); ++i)
						{
							double const fW = 1.0/(j->_13*i->x + j->_23*i->y + j->_33);
							double x = fW*(j->_11*i->x + j->_21*i->y + j->_31);
							double y = fW*(j->_12*i->x + j->_22*i->y + j->_32);
							cp.push_back(ClipperLib::IntPoint(F2I(x), F2I(y)));
						}
						clip.push_back(cp);

						ClipperLib::Paths cMid;

						ClipperLib::Clipper c;
						c.ZFillFunction(&CoordZFill);
						c.StrictlySimple(true);
						c.AddPaths(srcPolys, ClipperLib::ptSubject, true);
						c.AddPaths(clip, ClipperLib::ptClip, true);
						c.Execute(ClipperLib::ctIntersection, cMid, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

						c2.AddPaths(cMid, j != xforms.begin() ? ClipperLib::ptClip : ClipperLib::ptSubject, true);
					}

					ClipperLib::Paths cDst;

					c2.Execute(ClipperLib::ctUnion, cDst, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

					CComBSTR toolID;
					CComBSTR toolParams;
					EncodeResult(cWnd, pToolMgr, cDst, srcBeziers, toolID, toolParams);

					pDVI->ObjectSet(&objectID, toolID, toolParams);
				}

				std::vector<ULONG> aNewSelIDs;

				if (a_pStates)
				{
					CComPtr<ISharedState> pNewState;
					pDVI->StatePack(aNewSelIDs.size(), !aNewSelIDs.empty() ? &(aNewSelIDs[0]) : NULL, &pNewState);
					a_pStates->StateSet(bstrState, pNewState);
				}
				return S_OK;
			}

			TImagePoint src00 = {floor(srcPoly[0].x), floor(srcPoly[0].y)};
			TImagePoint src11 = {ceil(srcPoly[0].x), ceil(srcPoly[0].y)};
			for (std::vector<Point>::const_iterator i = srcPoly.begin()+1; i != srcPoly.end(); ++i)
			{
				LONG  x0 = floor(i->x);
				LONG  y0 = floor(i->y);
				LONG  x1 = ceil(i->x);
				LONG  y1 = ceil(i->y);
				if (src00.nX > x0) src00.nX = x0;
				if (src00.nY > y0) src00.nY = y0;
				if (src11.nX < x1) src11.nX = x1;
				if (src11.nY < y1) src11.nY = y1;
			}
			TImagePoint dst00 = src00;
			TImagePoint dst11 = src11;
			// extra margins for easier filtering
			--src00.nX; --src00.nY;
			++src11.nX; ++src11.nY;
			TImageSize srcSize = {src11.nX-src00.nX, src11.nY-src00.nY};
			CAutoVectorPtr<TPixelChannel> srcBuffer(new TPixelChannel[srcSize.nX*srcSize.nY]);
			pDRI->TileGet(EICIRGBA, &src00, &srcSize, NULL, srcSize.nX*srcSize.nY, srcBuffer, NULL, EIRIAccurate);

			for (std::vector<TMatrix3x3f>::const_iterator j = xforms.begin(); j != xforms.end(); ++j)
			{
				for (std::vector<Point>::const_iterator i = srcPoly.begin(); i != srcPoly.end(); ++i)
				{
					double const fW = 1.0/(j->_13*i->x + j->_23*i->y + j->_33);
					Point const t(fW*(j->_11*i->x + j->_21*i->y + j->_31), fW*(j->_12*i->x + j->_22*i->y + j->_32));
					LONG x0 = floor(t.x);
					LONG y0 = floor(t.y);
					LONG x1 = ceil(t.x);
					LONG y1 = ceil(t.y);
					if (dst00.nX > x0) dst00.nX = x0;
					if (dst00.nY > y0) dst00.nY = y0;
					if (dst11.nX < x1) dst11.nX = x1;
					if (dst11.nY < y1) dst11.nY = y1;
				}
			}
			TImageSize dstSize = {dst11.nX-dst00.nX, dst11.nY-dst00.nY};
			CAutoVectorPtr<TPixelChannel> dstBuffer(new TPixelChannel[dstSize.nX*dstSize.nY]);
			std::fill_n(dstBuffer.m_p, dstSize.nX*dstSize.nY, tDefault);

			CAutoPtr<CGammaTables> pGT(new CGammaTables());

			CRasterImageTargetNonOverlappingBlend cTarget(dstBuffer, &dstSize, CImageStride(1, dstSize.nX), pGT);
			agg::renderer_base<CRasterImageTargetNonOverlappingBlend> renb(cTarget);

			CRasterImagePreMpSrc img_accessor(srcBuffer, &src00, &srcSize, CImageStride(1, srcSize.nX), tDefault, pGT);

			{
				typedef agg::span_allocator<agg::rgba16> span_alloc_type;
				span_alloc_type sa;
				agg::renderer_scanline_aa<agg::renderer_base<CRasterImageTargetNonOverlappingBlend>, span_alloc_type, CRasterImagePreMpSrc> ren(renb, sa, img_accessor);
				agg::rasterizer_scanline_aa<> ras;
				agg::scanline_u8 sl;
				ras.reset();
				img_accessor.dx = dst00.nX;
				img_accessor.dy = dst00.nY;
				for (std::vector<Point>::const_iterator i = srcPoly.begin(); i != srcPoly.end(); ++i)
				{
					if (i == srcPoly.begin())
						ras.move_to_d(i->x-dst00.nX, i->y-dst00.nY);
					else
						ras.line_to_d(i->x-dst00.nX, i->y-dst00.nY);
				}
				agg::render_scanlines(ras, sl, ren);
			}

			TMatrix3x3f const tOff = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -dst00.nX, -dst00.nY, 1.0f};
			//TMatrix3x3f tMul;
			//Matrix3x3fMultiply(tOff, xforms[0], &tMul);
			agg::trans_perspective tr;
				//tMul._11, tMul._12, tMul._13,
				//tMul._21, tMul._22, tMul._23,
				//tMul._31, tMul._32, tMul._33);

			typedef agg::span_allocator<agg::rgba16> span_alloc_type;
			span_alloc_type sa;
			agg::image_filter_hermite filter_kernel;
			agg::image_filter_lut filter(filter_kernel, false);

			typedef agg::span_interpolator_trans<agg::trans_perspective> interpolator_type;
			interpolator_type interpolator(tr);

			typedef agg::span_image_filter_rgba_2x2<CRasterImagePreMpSrc, interpolator_type> span_gen_type;
			span_gen_type sg(img_accessor, interpolator, filter);

			agg::renderer_scanline_aa<agg::renderer_base<CRasterImageTargetNonOverlappingBlend>, span_alloc_type, span_gen_type> ren(renb, sa, sg);
			agg::rasterizer_scanline_aa<> ras;
			agg::scanline_u8 sl;
			for (std::vector<TMatrix3x3f>::const_iterator j = xforms.begin(); j != xforms.end(); ++j)
			{
				TMatrix3x3f tTmp;
				Matrix3x3fMultiply(*j, tOff, &tTmp);
				TMatrix3x3f tMul;
				Matrix3x3fInverse(tTmp, &tMul);
				double m[9] =
				{
					tMul._11, tMul._12, tMul._13,
					tMul._21, tMul._22, tMul._23,
					tMul._31, tMul._32, tMul._33
				};
				tr.load_from(m);
				ras.reset();
				for (std::vector<Point>::const_iterator i = srcPoly.begin(); i != srcPoly.end(); ++i)
				{
					double const fW = 1.0/(j->_13*i->x + j->_23*i->y + j->_33);
					Point const t(fW*(j->_11*i->x + j->_21*i->y + j->_31), fW*(j->_12*i->x + j->_22*i->y + j->_32));
					if (i == srcPoly.begin())
						ras.move_to_d(t.x-dst00.nX, t.y-dst00.nY);
					else
						ras.line_to_d(t.x-dst00.nX, t.y-dst00.nY);
				}
				agg::render_scanlines(ras, sl, ren);
			}


			return pDRI->TileSet(EICIRGBA, &dst00, &dstSize, NULL, dstSize.nX*dstSize.nY, dstBuffer, TRUE);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	void EncodeResult(CComObjectStackEx<CToolWindow>& cWnd, IRasterImageEditToolsManager* pToolMgr,
		ClipperLib::Paths& cDst, CBezierPaths& srcBeziers, CComBSTR& toolID, CComBSTR& toolParams)
	{
		std::vector<int> holes(cDst.size());
		for (size_t i = 0; i < cDst.size(); ++i)
			holes[i] = ClipperLib::Orientation(cDst[i]) ? -1 : -2;
		for (size_t i = 0; i < cDst.size(); ++i)
		{
			if (holes[i] == -2)
			{
				for (size_t j = 0; j < cDst.size(); ++j)
					if (holes[j] == -1 && ClipperLib::PointInPolygon(cDst[i][0], cDst[j]))
					{
						holes[i] = j;
						break;
					}
				std::reverse(cDst[i].begin(), cDst[i].end());
			}
			
		}

		CBezierPaths restored(cDst.size());
		for (size_t i = 0; i < cDst.size(); ++i)
		{
			ClipperLib::Path const& p = cDst[i];
			CBezierPath& r = restored[i];
			r.resize(1);

			if (WalkSegments(srcBeziers, p, addSegment(r)) && r.size() > 1)
			{
				r[0].tTanPrev = r[r.size()-1].tTanPrev;
				r.resize(r.size()-1);
				r[r.size()-1].dwFlags = 1;
			}
		}

		std::vector<TRWPath> buffer;
		for (size_t i = 0; i < restored.size(); ++i)
		{
			TRWPath cur;
			cur.nVertices = restored[i].size();
			cur.pVertices = &(restored[i][0]);
			buffer.push_back(cur);
		}

		if (buffer.empty())
		{
			toolID.Empty();
			toolParams.Empty();
		}
		else
		{
			GetToolParams(cWnd, pToolMgr, buffer, &toolID, &toolParams);
		}
	}

private:
	// Basic point/vector struct.
	struct Point
	{
		double x, y;
		explicit Point(double x = 0, double y = 0) : x(x), y(y) {}

		friend Point operator+(const Point& p, const Point& q) { return Point(p.x + q.x, p.y + q.y); }
		friend Point operator-(const Point& p, const Point& q) { return Point(p.x - q.x, p.y - q.y); }
		friend Point operator*(const Point& p, const double k) { return Point(p.x * k, p.y * k); } 
		friend double dot(const Point& p, const Point& q) { return p.x * q.x + p.y * q.y; }
		friend double cross(const Point& p, const Point& q) { return p.x * q.y - p.y * q.x; }
	};

	// Basic half-plane struct.
	struct Halfplane
	{ 
		// 'p' is a passing point of the line and 'pq' is the direction vector of the line.
		Point p, pq; 
		double angle;

		Halfplane() {}
		Halfplane(const Point& a, const Point& b) : p(a), pq(b - a) { angle = atan2(pq.y, pq.x); }
		Halfplane(const Point& p, double a) : p(p), pq(cos(a), sin(a)), angle(a) {}

		// Check if point 'r' is outside this half-plane.
		// Every half-plane allows the region to the LEFT of its line.
		bool out(const Point& r, double eps) { return cross(pq, r - p) < -eps; }

		// Comparator for sorting.
		bool operator<(const Halfplane& e) const { return angle < e.angle; } 

		// Intersection point of the lines of two half-planes. It is assumed they're never parallel.
		friend Point inter(const Halfplane& s, const Halfplane& t)
		{
			double alpha = cross((t.p - s.p), t.pq) / cross(s.pq, t.pq);
			return s.p + (s.pq * alpha);
		}
	};

	// Actual algorithm
	void hp_intersect(std::vector<Halfplane>& H, std::vector<Point>& ret, double eps = 1e-9)
	{
		ret.clear();

		// Sort by angle and start algorithm
		std::sort(H.begin(), H.end());
		std::deque<Halfplane> dq;
		int len = 0;
		for(int i = 0; i < int(H.size()); i++)
		{
			// Remove from the back of the deque while last half-plane is redundant
			while (len > 1 && H[i].out(inter(dq[len-1], dq[len-2]), eps))
			{
				dq.pop_back();
				--len;
			}

			// Remove from the front of the deque while first half-plane is redundant
			while (len > 1 && H[i].out(inter(dq[0], dq[1]), eps))
			{
				dq.pop_front();
				--len;
			}

			// Special case check: Parallel half-planes
			if (len > 0 && fabsl(cross(H[i].pq, dq[len-1].pq)) < eps)
			{
				// Opposite parallel half-planes that ended up checked against each other.
				if (dot(H[i].pq, dq[len-1].pq) < 0.0)
					return;

				// Same direction half-plane: keep only the leftmost half-plane.
				if (H[i].out(dq[len-1].p, eps))
				{
					dq.pop_back();
					--len;
				}
				else
					continue;
			}

			// Add new half-plane
			dq.push_back(H[i]);
			++len;
		}

		// Final cleanup: Check half-planes at the front against the back and vice-versa
		while (len > 2 && dq[0].out(inter(dq[len-1], dq[len-2]), eps))
		{
			dq.pop_back();
			--len;
		}

		while (len > 2 && dq[len-1].out(inter(dq[0], dq[1]), eps))
		{
			dq.pop_front();
			--len;
		}

		// Report empty intersection if necessary
		if (len < 3)
			return;

		// Reconstruct the convex polygon from the remaining half-planes.
		ret.resize(len);
		for(int i = 0; i < len; ++i)
			ret[i] = inter(dq[i], dq[(i+1)%len]);
	}

private:
	class ATL_NO_VTABLE CWrapper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ICanvasInteractingWrapper
	{
	public:
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, LONG a_nType, LONG a_nOrder, TPixelCoords a_tOrigin, float a_fAngle)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;
			m_fRadius = 0.35f*min(m_nSizeX, m_nSizeY);
			m_nType = a_nType;
			m_nOrder = a_nOrder;

			m_tOrigin = a_tOrigin;

			m_fAngle = a_fAngle;
			m_tDirection.fX = m_tOrigin.fX + m_fRadius*sinf(m_fAngle*3.14159265359f/180.0f);
			m_tDirection.fY = m_tOrigin.fY - m_fRadius*cosf(m_fAngle*3.14159265359f/180.0f);
		}
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;
			m_fRadius = 0.35f*min(m_nSizeX, m_nSizeY);

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_TYPE), &cVal);
			m_nType = cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORDER), &cVal);
			m_nOrder = cVal;

			CConfigValue cOrigin;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIGIN), &cOrigin);
			m_tOrigin.fX = cOrigin[0];
			m_tOrigin.fY = cOrigin[1];

			CConfigValue cLastSize;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_LASTSIZE), &cLastSize);
			if (cLastSize[0] != m_nSizeX || cLastSize[1] != m_nSizeY)
			{
				m_tOrigin.fX = 0.5f*m_nSizeX;
				m_tOrigin.fY = 0.5f*m_nSizeY;
			}

			CConfigValue cOrientation;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ORIENTATION), &cOrientation);
			m_fAngle = cOrientation;
			m_tDirection.fX = m_tOrigin.fX + m_fRadius*sinf(m_fAngle*3.14159265359f/180.0f);
			m_tDirection.fY = m_tOrigin.fY - m_fRadius*cosf(m_fAngle*3.14159265359f/180.0f);
		}

	BEGIN_COM_MAP(CWrapper)
		COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
	END_COM_MAP()

		// ICanvasInteractingWrapper methods
	public:
		STDMETHOD_(ULONG, GetControlPointCount)()
		{
			return 2;
		}
		STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
		{
			if (a_nIndex == 0)
			{
				if (a_pPos)
					*a_pPos = m_tOrigin;
				if (a_pClass)
					*a_pClass = 0;
				return S_OK;
			}
			if (a_nIndex == 1)
			{
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
					if (a_pPos->fX != m_tOrigin.fX || a_pPos->fY != m_tOrigin.fY) // or use epsilon? ...not that important
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, m_nType, m_nOrder, *a_pPos, m_fAngle);
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				if (a_nIndex == 1)
				{
					float fDX = a_pPos->fX-m_tOrigin.fX;
					float fDY = a_pPos->fY-m_tOrigin.fY;
					if (fabsf(fDX) < 0.01f && fabsf(fDY) < 0.01)
					{
						(*a_ppNew = this)->AddRef();
						return S_FALSE;
					}
					float fAngle = atan2(fDX, -fDY)*180.0f/3.14159265359f;
					if (fabsf(m_fAngle-fAngle) > 1e-5f)
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, m_nType, m_nOrder, m_tOrigin, fAngle);
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

		static bool BoxIntersect(float fOX, float fOY, float fAngle, float fB0X, float fB0Y, float fB1X, float fB1Y, float* pIX, float* pIY)
		{
			float fDX = sinf(fAngle*3.14159265359f/180.0f);
			float fDY = -cosf(fAngle*3.14159265359f/180.0f);
			if (fDX < -1e-5f && fOX > fB0X)
			{
				float fY = fOY - (fOX-fB0X)/fDX*fDY;
				if (fY >= fB0Y && fY < fB1Y)
				{
					*pIX = fB0X;
					*pIY = fY;
					return true;
				}
			}
			if (fDX > 1e-5f && fOX < fB1X)
			{
				float fY = fOY + (fB1X-fOX)/fDX*fDY;
				if (fY >= fB0Y && fY < fB1Y)
				{
					*pIX = fB1X;
					*pIY = fY;
					return true;
				}
			}
			if (fDY < -1e-5f && fOY > fB0Y)
			{
				float fX = fOX - (fOY-fB0Y)/fDY*fDX;
				if (fX >= fB0X && fX < fB1X)
				{
					*pIX = fX;
					*pIY = fB0Y;
					return true;
				}
			}
			if (fDY > 1e-5f && fOY < fB1Y)
			{
				float fX = fOX + (fB1Y-fOY)/fDY*fDX;
				if (fX >= fB0X && fX < fB1X)
				{
					*pIX = fX;
					*pIY = fB1Y;
					return true;
				}
			}
			return false;
		}
		STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
		{
			float fTX = m_tDirection.fX;
			float fTY = m_tDirection.fY;
			if (BoxIntersect(m_tOrigin.fX, m_tOrigin.fY, m_fAngle, 0.0f, 0.0f, m_nSizeX, m_nSizeY, &fTX, &fTY))
			{
				a_pLines->MoveTo(m_tOrigin.fX, m_tOrigin.fY);
				a_pLines->LineTo(fTX, fTY);
			}
			if (BoxIntersect(m_tOrigin.fX, m_tOrigin.fY, m_fAngle+(m_nType == CFGVAL_TP_LINEREFLECTION ? 180.0f : -360.0f/m_nOrder), 0.0f, 0.0f, m_nSizeX, m_nSizeY, &fTX, &fTY))
			{
				a_pLines->MoveTo(m_tOrigin.fX, m_tOrigin.fY);
				a_pLines->LineTo(fTX, fTY);
			}
			if (m_nType == CFGVAL_TP_LINEREFLECTION)
			{
				float fDX = m_tDirection.fX-m_tOrigin.fX;
				float fDY = m_tDirection.fY-m_tOrigin.fY;
				float f1 = 0.15f;
				float f2 = 0.05f;
				float fOX = m_tOrigin.fX;
				float fOY = m_tOrigin.fY;
				a_pLines->MoveTo(fOX, fOY);
				a_pLines->LineTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY + f2*fDX, fOY + (f1-f2)*fDX + f2*fDY);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY - f2*fDX, fOY + (f1-f2)*fDX - f2*fDY);

				fOX = m_tDirection.fX;
				fOY = m_tDirection.fY;
				a_pLines->MoveTo(fOX, fOY);
				a_pLines->LineTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY + f2*fDX, fOY + (f1-f2)*fDX + f2*fDY);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY - f2*fDX, fOY + (f1-f2)*fDX - f2*fDY);

				fOX = m_tOrigin.fX-fDX;
				fOY = m_tOrigin.fY-fDY;
				a_pLines->MoveTo(fOX, fOY);
				a_pLines->LineTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY + f2*fDX, fOY + (f1-f2)*fDX + f2*fDY);
				a_pLines->MoveTo(fOX - f1*fDY, fOY + f1*fDX);
				a_pLines->LineTo(fOX - (f1-f2)*fDY - f2*fDX, fOY + (f1-f2)*fDX - f2*fDY);
			}
			else
			{
				a_pLines->MoveTo(m_tDirection.fX, m_tDirection.fY);
				float fTotalAngle = 360.0f/m_nOrder;
				int nSteps = (90+(m_nOrder>>1))/m_nOrder;
				if (nSteps < 1)
					nSteps = 1;
				for (int i = 0; i < nSteps; ++i)
				{
					float fAngle = (m_fAngle+(i+1)*fTotalAngle/nSteps);
					float fX = m_tOrigin.fX + m_fRadius*sinf(fAngle*3.14159265359f/180.0f);
					float fY = m_tOrigin.fY - m_fRadius*cosf(fAngle*3.14159265359f/180.0f);
					a_pLines->LineTo(fX, fY);
				}
				float fAngle = m_fAngle+fTotalAngle;
				float fDX = m_fRadius*sinf(fAngle*3.14159265359f/180.0f);
				float fDY = -m_fRadius*cosf(fAngle*3.14159265359f/180.0f);
				float fOX = m_tOrigin.fX+fDX;
				float fOY = m_tOrigin.fY+fDY;
				float f2 = 0.05f;
				a_pLines->MoveTo(fOX, fOY);
				a_pLines->LineTo(fOX + f2*fDY + f2*fDX, fOY - f2*fDX + f2*fDY);
				a_pLines->MoveTo(fOX, fOY);
				a_pLines->LineTo(fOX + f2*fDY - f2*fDX, fOY - f2*fDX - f2*fDY);
			}
			//float fDX = m_tDirection.fX-m_tOrigin.fX;
			//float fDY = m_tDirection.fY-m_tOrigin.fY;
			//if (fDX < 1e-5f && m_tOrigin.fX > 0)
			//{
			//	float fY = m_tOrigin.fY - m_tOrigin.fX/fDX*fDY;
			//	if (fY >= 0 && fY < m_nSizeY)
			//	{
			//		fTX = 0;
			//		fTY = fY;
			//	}
			//}
			//a_pLines->LineTo(m_aCoords[4], m_aCoords[5]);
			//a_pLines->LineTo(m_aCoords[6], m_aCoords[7]);
			//a_pLines->Close();
			return S_OK;
		}

		STDMETHOD(ToConfig)(IConfig* a_pConfig)
		{
			CComBSTR cOrigin(CFGID_ORIGIN);
			CComBSTR cOrientation(CFGID_ORIENTATION);
			CComBSTR cLastSize(CFGID_LASTSIZE);
			BSTR aIDs[3] = {cOrigin, cOrientation, cLastSize};
			TConfigValue aVals[3];
			aVals[0] = CConfigValue(m_tOrigin.fX, m_tOrigin.fY);
			aVals[1] = CConfigValue(m_fAngle);
			aVals[2] = CConfigValue(m_nSizeX, m_nSizeY);
			return a_pConfig->ItemValuesSet(3, aIDs, aVals);
		}

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		float m_fRadius;
		TPixelCoords m_tOrigin;
		float m_fAngle;
		TPixelCoords m_tDirection;
		LONG m_nType;
		LONG m_nOrder;
	};
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationSymmetry, CDocumentOperationSymmetry)
