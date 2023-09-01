// RasterImageContourGradient.h : Declaration of the CRasterImageContourGradient

#include "stdafx.h"

#include "RWOperationImageRaster.h"
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <WTL_ColorPicker.h>
#include <ConfigCustomGUIImpl.h>
#include <IconRenderer.h>


// {20F5BE48-08D2-4b8f-8DFC-5DE8576E1A3F}
extern GUID const CLSID_RasterImageContourGradient = {0x20f5be48, 0x8d2, 0x4b8f, {0x8d, 0xfc, 0x5d, 0xe8, 0x57, 0x6e, 0x1a, 0x3f}};
static wchar_t const CFGID_GRADIENT[] = L"Gradient";
static wchar_t const CFGID_SCALE[] = L"Scale";
HICON GetDefaultIconContourGradient(ULONG a_nSize)
{
	CComPtr<IIconRenderer> pIR;
	RWCoCreateInstance(pIR, __uuidof(IconRenderer));
	IRPolyPoint const poly0[] = {{207, 247}, {128, 189}, {49, 247}, {79, 154}, {0, 96}, {97, 96}, {128, 3}, {159, 96}, {256, 96}, {177, 154}};
	IRPolyPoint const poly1[] = {{170, 195}, {128, 165}, {86, 195}, {102, 146}, {60, 115}, {112, 115}, {128, 66}, {144, 115}, {196, 115}, {154, 146}};
	IRPolyPoint const poly2[] = {{153, 172}, {128, 154}, {103, 172}, {112, 142}, {87, 124}, {118, 124}, {128, 94}, {138, 124}, {169, 124}, {144, 142}};
	IRPolygon const shape0 = {itemsof(poly0), poly0};
	IRPolygon const shape1 = {itemsof(poly1), poly1};
	IRPolygon const shape2 = {itemsof(poly2), poly2};
	IRCanvas const canvas0 = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRFill inner(pSI->GetSRGBColor(ESMInterior));
	IRFill middle(CGammaTables::BlendSRGBA(pSI->GetSRGBColor(ESMScheme1Color2), inner.color, 0.5f));
	IRLayer const layers[] =
	{
		{&canvas0, 1, 0, &shape0, NULL, pSI->GetMaterial(ESMScheme1Color2)},
		{&canvas0, 1, 0, &shape1, NULL, &middle},
		{&canvas0, 1, 0, &shape2, NULL, &inner},
	};
	return pIR->CreateIcon(a_nSize, itemsof(layers), layers);
}

const DWORD one = 256;
DWORD distanceMap(DWORD* map, int width, int height, DWORD ceil, DWORD def = one);

// CRasterImageContourGradient

class ATL_NO_VTABLE CRasterImageContourGradient :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageContourGradient, &CLSID_RasterImageContourGradient>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl
{
public:
	CRasterImageContourGradient()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageContourGradient)

BEGIN_CATEGORY_MAP(CRasterImageContourGradient)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	IMPLEMENTED_CATEGORY(CATID_TagImageEmbellishment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageContourGradient)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Shape gradient[0405]Rastrový obrázek - tvarový gradient");
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

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_GRADIENT), CMultiLanguageString::GetAuto(L"[0409]Gradient[0405]Gradient"), CMultiLanguageString::GetAuto(L"[0409]Gradient[0405]Gradient"), CConfigValue(L"0,0,0,0,1;65535,1,1,1,1;"), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SCALE), CMultiLanguageString::GetAuto(L"[0409]Scale[0405]Škálování"), CMultiLanguageString::GetAuto(L"[0409]Size of the gradient in pixels.[0405]Velikost gradientu v pixelech."), CConfigValue(50.0f), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_RasterImageContourGradient, CConfigGUIDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			static IID const aFts[] = {__uuidof(IDocumentRasterImage)/*, __uuidof(IRasterImageControl)*/};
			return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
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
			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SCALE), &cVal);
			float fScale = cVal.operator float();
			if (fScale <= 1.0f) fScale = 1.0f;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_GRADIENT), &cVal);

			int nExtraSpace = ceilf(fScale)+1;

			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

			ULONG const steps = ceilf(fScale)*4;
			CAutoVectorPtr<TPixelChannel> aGrad(new TPixelChannel[steps+1]);
			{
				CGradientColorPicker::CGradient cGradient;
				CGradientColorPicker::ParseGradient(cVal, cGradient);
				CGradientColorPicker::FixGradient(cGradient);
				CAutoVectorPtr<CButtonColorPicker::SColor> tmp(new CButtonColorPicker::SColor[steps]);
				CGradientColorPicker::RenderGradient(cGradient, steps, 0, steps, tmp.m_p);
				for (ULONG i = 0; i < steps; ++i)
				{
					*reinterpret_cast<DWORD*>(aGrad.m_p+i) = tmp[i].ToBGRA(2.2f);
				}
				aGrad[steps] = aGrad[steps-1];
			}

			bool bHasAlpha = true;

			TImagePoint tOffset = {0, 0};
			TImageSize tSize = {0, 0};
			if (FAILED(pRI->CanvasGet(NULL, NULL, &tOffset, &tSize, NULL)))
				return E_FAIL;

			TRasterImageRect tRect = {tOffset, {tOffset.nX+tSize.nX, tOffset.nY+tSize.nY}};
			if (tRect.tBR.nX <= tRect.tTL.nX || tRect.tBR.nY <= tRect.tTL.nY)
				return S_FALSE; // not a valid rectangle

			ULONG nPixels = tSize.nX*tSize.nY;
			CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
			CAutoVectorPtr<DWORD> pHeight(new DWORD[(tSize.nX+2)*(tSize.nY+2)]);
			HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
			if (FAILED(hRes))
				return hRes;

			DWORD const dwMaxHeight = one*fScale;

			ZeroMemory(pHeight.m_p, (tSize.nX+2)*sizeof*pHeight.m_p);
			ZeroMemory(pHeight.m_p+(tSize.nX+2)*(tSize.nY+1), (tSize.nX+2)*sizeof*pHeight.m_p);
			TPixelChannel* p = pSrc.m_p;
			DWORD* pH = pHeight.m_p+tSize.nX+2;
			DWORD nMax = min(tSize.nX, tSize.nY);
			for (ULONG nY = 0; nY < tSize.nY; ++nY)
			{
				*(pH++) = 0;
				for (ULONG nX = 0; nX < tSize.nX; ++nX, ++pH, ++p)
				{
					if (p->bA == 0)
					{
						*pH = 0;
					}
					else if (p->bA == 255)
					{
						*pH = dwMaxHeight;
					}
					else
					{
						*pH = (p->bA*one)>>8;
					}
					//*pH = p->bA*one*nMax;
				}
				*(pH++) = 0;
			}
			distanceMap(pHeight, tSize.nX+2, tSize.nY+2, dwMaxHeight);

			ATLASSERT(one == 1<<8); // should be static assert
			TPixelChannel* pD = pSrc.m_p;
			DWORD const* map = pHeight+tSize.nX+3;
			for (ULONG y = 0; y < tSize.nY; ++y)
			{
				for (ULONG x = 0; x < tSize.nX; ++x, ++map, ++pD)
				{
					if (pD->bA == 255)
					{
						*pD = aGrad[(*map+0x20)>>6];
					}
					else if (pD->bA)
					{
						ULONG const bA = pD->bA;
						*pD = aGrad[(*map+0x20)>>6];
						pD->bA = (pD->bA*bA)/255;
					}
				}
				map += 2;
			}

			return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, FALSE);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
	{
		if (a_pConfig == NULL || a_pContentTransform == NULL)
			return E_RW_INVALIDPARAM;
		float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
		if (f > 0.9999f && f < 1.0001f)
			return S_FALSE;
		CComBSTR bstrSCALE(CFGID_SCALE);
		TConfigValue tVal;
		a_pConfig->ItemValueGet(bstrSCALE, &tVal);
		tVal.fVal *= f;
		return a_pConfig->ItemValuesSet(1, &(bstrSCALE.m_str), &tVal);
	}
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		if (a_pConfig && a_pRect)
		{
			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SCALE), &cVal);
			float fScale = cVal.operator float();
			if (fScale <= 1.0f) fScale = 1.0f;
			int nExtraSpace = ceilf(fScale)+1;

			a_pRect->tTL.nX -= nExtraSpace;
			a_pRect->tTL.nY -= nExtraSpace;
			a_pRect->tBR.nX += nExtraSpace;
			a_pRect->tBR.nY += nExtraSpace;
		}
		return S_OK;
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{ return AdjustDirtyRect(a_pConfig, a_pCanvas, a_pRect); }

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			*a_ppName = new CMultiLanguageString(L"[0409]Shape gradient[0405]Tvarový gradient");
			return S_OK;
		}
		catch (...)
		{
			return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			*a_phIcon = GetDefaultIconContourGradient(a_nSize);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CConfigGUIDlg :
		public CCustomConfigResourcelessWndImpl<CConfigGUIDlg>,
		public CObserverImpl<CConfigGUIDlg, IColorPickerObserver, ULONG>,
		public CDialogResize<CConfigGUIDlg>,
		public IResizableConfigWindow
	{
	public:
		CConfigGUIDlg() : m_wndGradient(true), m_nOffSetY(0), m_nDefaultX(100)
		{
		}

		enum
		{
			IDC_SCALE_LABEL = 200,
			IDC_SCALE,
			IDC_SCALE_SLIDER,
			IDC_GRADIENT,
			IDC_STOPCOLOR,
		};

		BEGIN_DIALOG_EX(0, 0, 120, 154, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_LTEXT(_T("[0409]Scale:[0405]Škála:"), IDC_SCALE_LABEL, 0, 2, 44, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_SCALE, 45, 0, 30, 12, WS_VISIBLE | WS_TABSTOP, 0)
			CONTROL_CONTROL(_T(""), IDC_SCALE_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_HORZ | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 75, 0, 45, 12, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUIDlg)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			CHAIN_MSG_MAP(CDialogResize<CConfigGUIDlg>)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
			NOTIFY_HANDLER(IDC_GRADIENT, CGradientColorPicker::GCPN_ACTIVESTOPCHANGED, OnGradientStopChanged)
		END_MSG_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUIDlg)
			CONFIGITEM_EDITBOX(IDC_SCALE, CFGID_SCALE)
		END_CONFIGITEM_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigGUIDlg)
			DLGRESIZE_CONTROL(IDC_SCALE_SLIDER, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_GRADIENT, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_STOPCOLOR, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_COM_MAP(CConfigGUIDlg)
			COM_INTERFACE_ENTRY(IChildWindow)
			COM_INTERFACE_ENTRY(IResizableConfigWindow)
		END_COM_MAP()

		void ExtraInitDialog()
		{
			HDC hdc = GetDC();
			float fScale = GetDeviceCaps(hdc, LOGPIXELSX) / 96.0f;
			ReleaseDC(hdc);

			RECT rcGrad = {4, 16, 120, 33};
			MapDialogRect(&rcGrad);
			m_wndGradient.SetMargin(rcGrad.left);
			rcGrad.left = 0;
			rcGrad.bottom = rcGrad.top + 28*fScale;
			m_wndGradient.Create(m_hWnd, rcGrad, _T("Gradient"), WS_CHILD|WS_TABSTOP|WS_VISIBLE, 0, IDC_GRADIENT);
			RECT rcColor = {0, 4, 120, 154};
			MapDialogRect(&rcColor);
			m_nOffSetY = rcColor.top += rcGrad.bottom;
			m_nDefaultX = rcColor.right;
			RWCoCreateInstance(m_pColor, __uuidof(ColorPicker));
			m_pColor->Create(m_hWnd, &rcColor, TRUE, m_tLocaleID, CMultiLanguageString::GetAuto(L"[0409]Selected stop color[0405]Barva vybraného zastavení"), FALSE, CComBSTR(L"GRADIENT"), NULL);
			RWHWND h = NULL;
			m_pColor->Handle(&h);
			::SetWindowLong(h, GWLP_ID, IDC_STOPCOLOR);
			m_pColor->ObserverIns(CObserverImpl<CConfigGUIDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);

			m_wndScale = GetDlgItem(IDC_SCALE_SLIDER);
			m_wndScale.SetRange(1, 256);
		}

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			//ExtraConfigNotify();
			DlgResize_Init(false, false, 0);

			return 1;
		}
		LRESULT OnDestroy(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			m_pColor->ObserverDel(CObserverImpl<CConfigGUIDlg, IColorPickerObserver, ULONG>::ObserverGet(), 0);
			a_bHandled = FALSE;
			return 0;
		}
		LRESULT OnStopColorChanged(int, LPNMHDR a_pHdr, BOOL&)
		{
			CButtonColorPicker::NMCOLORBUTTON* pClr = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON*>(a_pHdr);
			m_wndGradient.SetStop(m_wndGradient.GetStop(), pClr->clr);
			return 0;
		}
		LRESULT OnGradientStopChanged(int, LPNMHDR a_pHdr, BOOL&)
		{
			CGradientColorPicker::NMGRADIENT* pGrad = reinterpret_cast<CGradientColorPicker::NMGRADIENT*>(a_pHdr);
			{
				TColor t = {pGrad->clr.fR, pGrad->clr.fG, pGrad->clr.fB, pGrad->clr.fA};
				m_pColor->ColorSet(&t);
			}

			CComBSTR bstrGrad;
			CGradientColorPicker::SerializeGradient(m_wndGradient.GetGradient(), bstrGrad);
			CConfigValue cVal(bstrGrad.m_str);
			CComBSTR bstrID(CFGID_GRADIENT);
			M_Config()->ItemValuesSet(1, &(bstrID.m_str), cVal);
			return 0;
		}
		void OwnerNotify(TCookie, ULONG a_nFlags)
		{
			if (a_nFlags&ECPCColor)
			{
				TColor tClr = {0.0f, 0.0f, 0.0f, 0.0f};
				m_pColor->ColorGet(&tClr);
				m_wndGradient.SetStop(m_wndGradient.GetStop(), CButtonColorPicker::SColor(tClr.fR, tClr.fG, tClr.fB, tClr.fA));
			}
		}
		void OwnerNotify(TCookie t, IUnknown* p)
		{
			CCustomConfigResourcelessWndImpl<CConfigGUIDlg>::OwnerNotify(t, p);
		}

		void ExtraConfigNotify()
		{
			if (m_wndGradient.m_hWnd)
			{
				CConfigValue cVal;
				M_Config()->ItemValueGet(CComBSTR(CFGID_GRADIENT), &cVal);
				CGradientColorPicker::CGradient cGrad;
				CGradientColorPicker::ParseGradient(cVal, cGrad);
				CGradientColorPicker::FixGradient(cGrad);
				CGradientColorPicker::CGradient cOld = m_wndGradient.GetGradient();
				if (cOld != cGrad)
				{
					m_wndGradient.SetGradient(cGrad);
					CButtonColorPicker::SColor clr = m_wndGradient.GetStopColor(m_wndGradient.GetStop());
					{
						TColor t = {clr.fR, clr.fG, clr.fB, clr.fA};
						m_pColor->ColorSet(&t);
					}
				}
			}
			if (m_wndScale.m_hWnd)
			{
				CConfigValue cVal;
				M_Config()->ItemValueGet(CComBSTR(CFGID_SCALE), &cVal);
				m_wndScale.SetPos(cVal.operator float());
			}
		}

		LRESULT OnHScroll(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (a_lParam && reinterpret_cast<HWND>(a_lParam) != m_wndScale)
			{
				a_bHandled = FALSE;
				return 0;
			}

			{
				TCHAR sz[32] = _T("");
				_stprintf(sz, _T("%i"), m_wndScale.GetPos());
				SetDlgItemText(IDC_SCALE, sz);
				BOOL b;
				OnEnKillFocus(0, IDC_SCALE, GetDlgItem(IDC_SCALE), b);
				return 1;
			}
		}

		STDMETHOD(OptimumSize)(SIZE *a_pSize)
		{
			SIZE sz = {100, 100};
			if (SUCCEEDED(m_pColor->OptimumSize(&sz)))
			{
				a_pSize->cx = m_nDefaultX;
				a_pSize->cy = m_nOffSetY+sz.cy;
				return S_OK;
			}
			else
				return E_FAIL;
		}

	private:
		CGradientColorPicker m_wndGradient;
		CComPtr<IColorPicker> m_pColor;
		CTrackBarCtrl m_wndScale;
		int m_nOffSetY;
		int m_nDefaultX;
	};

};

OBJECT_ENTRY_AUTO(CLSID_RasterImageContourGradient, CRasterImageContourGradient)
