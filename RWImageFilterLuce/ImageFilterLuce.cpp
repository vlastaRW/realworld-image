
#include "stdafx.h"

#include <RWBase.h>
#include <RWConfig.h>
#include <RWProcessing.h>
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <ConfigCustomGUIImpl.h>
#include <GammaCorrection.h>
#include <IconRenderer.h>
#include <RWDrawing.h>
#include "luce.h"

struct TRasterImagePixel16
{
	WORD wB, wG, wR, wA;
};


static OLECHAR const CFGID_INTENSITY[] = L"Intensity";
static OLECHAR const CFGID_ATTENUATION[] = L"Attenuation";
static LONG const CFGVAL_ATT_LINEAR = 1;
static LONG const CFGVAL_ATT_QUADRATIC = 2;
static OLECHAR const CFGID_SOURCE[] = L"Source";

// {0x3293f03b, 0x4e1f, 0x4017, {0xbf, 0x86, 0xe, 0xc3, 0x1a, 0x7e, 0xf5, 0x8d}};
class DECLSPEC_UUID("3293F03B-4E1F-4017-BF86-0EC31A7EF58D")
ImageFilterLuce;

class ATL_NO_VTABLE CImageFilterLuce :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageFilterLuce>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl,
	public ICanvasInteractingOperation
{
public:
	CImageFilterLuce()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CImageFilterLuce)

BEGIN_CATEGORY_MAP(CImageFilterLuce)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	IMPLEMENTED_CATEGORY(CATID_TagImageOverlay)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CImageFilterLuce)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(ICanvasInteractingOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL) return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Luce[0405]Rastrový obrázek - paprsky");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_INTENSITY), CMultiLanguageString::GetAuto(L"[0409]Intensity[0405]Intenzita"), NULL, CConfigValue(1.0f), NULL, 0, NULL);
			CComBSTR bstrCFGID_ATTENUATION(CFGID_ATTENUATION);
			pCfgInit->ItemIns1ofN(bstrCFGID_ATTENUATION, CMultiLanguageString::GetAuto(L"[0409]Attenuation[0405]Útlum"), CMultiLanguageString::GetAuto(L"[0409]This setting controls how the light rays intensity falls off.[0405]Toto nastavení určuje, jak intenzita světelných paprsků klesá."), CConfigValue(CFGVAL_ATT_LINEAR), NULL);
			pCfgInit->ItemOptionAdd(bstrCFGID_ATTENUATION, CConfigValue(CFGVAL_ATT_LINEAR), CMultiLanguageString::GetAuto(L"[0409]Linear[0405]Lineární"), 0, NULL);
			pCfgInit->ItemOptionAdd(bstrCFGID_ATTENUATION, CConfigValue(CFGVAL_ATT_QUADRATIC), CMultiLanguageString::GetAuto(L"[0409]Quadratic[0405]Kvadratický"), 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SOURCE), CMultiLanguageString::GetAuto(L"[0409]Source[0405]Zdroj"), NULL, CConfigValue(0.0f, 0.0f), NULL, 0, NULL);
 
			CConfigCustomGUI<&__uuidof(ImageFilterLuce), CConfigGUI>::FinalizeConfig(pCfgInit);

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

			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
			if (pRI == NULL)
				return S_FALSE;
			return S_OK;
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
			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
			if (pRI == NULL)
				return E_FAIL;

			TImageSize tSize = {1, 1};
			pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

			CConfigValue cInt;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_INTENSITY), &cInt);
			CConfigValue cAtt;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ATTENUATION), &cAtt);
			CConfigValue cSrc;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SOURCE), &cSrc);
			CAutoVectorPtr<TRasterImagePixel> pImage;
			pImage.Allocate(tSize.nX*tSize.nY);
			pRI->TileGet(EICIRGBA, CImagePoint(0, 0), &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pImage.m_p), NULL, EIRIAccurate);

			CAutoVectorPtr<TRasterImagePixel16> pBuf;
			pBuf.Allocate(tSize.nX*tSize.nY*2);

			CAutoPtr<CGammaTables> pGamma(new CGammaTables());
			CGammaTables* pG = pGamma;

			int const mat = 32768;

			TRasterImagePixel* pS = pImage;
			TRasterImagePixel16* pD = pBuf;
			for (ULONG n = tSize.nX*tSize.nY; n > 0; --n, ++pS, ++pD)
			{
				unsigned const a = pS->bA;
				unsigned const ia = mat*(255-a);
				pD->wB = (pG->m_aGamma[pS->bB]*a + ia + 128)/255;
				pD->wG = (pG->m_aGamma[pS->bG]*a + ia + 128)/255;
				pD->wR = (pG->m_aGamma[pS->bR]*a + ia + 128)/255;
				pD->wA = a|(a<<8);
			}

			//CComPtr<IGammaCacheTable> pGCT;
			//RWCoCreateInstance(pGCT, __uuidof(GammaCacheTable));
			
			CLuce cLuce;
			LuceOptions cOpt;
			cOpt.InitDefault();
			cOpt.SetCenter(cSrc[0], cSrc[1]);//m_tLight.fX/m_nSizeX, m_tLight.fY/m_nSizeY);
			//if (m_eBM == EBMDrawOver)
				cOpt.SetAddSource();
			//else
			//	cOpt.SetNotAddSource();
			cOpt.SetNegativeIntensity(cInt);
			cOpt.SetPositiveIntensity(cInt);
			if (cAtt.operator LONG() == CFGVAL_ATT_QUADRATIC)
				cOpt.SetQuadraticAttenuation();
			else
				cOpt.SetLinearAttenuation();
			cOpt.SetNumThreadsToUse(4);//ProcessorCount());
			//cOpt.SetDirection((m_tLight.fX-m_nSizeX*0.5)/(m_nSizeX*0.5f), (m_tLight.fY-m_nSizeY*0.5)/(m_nSizeY*0.5f));
			cLuce.pOpt = &cOpt;
			cLuce.width = tSize.nX;
			cLuce.height = tSize.nY;
			if (/*m_cData.bColored*/true)
			{
				cLuce.input.iPixelSize = 8;
				cLuce.input.iStride = 8*tSize.nX;
				cLuce.input.pBits = reinterpret_cast<BYTE*>(pBuf.m_p);
				cLuce.output.iPixelSize = 8;
				cLuce.output.iStride = 8*tSize.nX;
				cLuce.output.pBits = reinterpret_cast<BYTE*>(pBuf.m_p+tSize.nX*tSize.nY);
				cLuce.alpha = cLuce.input;
				cLuce.alpha.pBits += 6;
				for (int i = 0; i < 4; ++i)
				{
					if (i == 4-1)
					{
						cLuce.alpha.pBits = NULL;
					}
					cLuce.currentZero = i;
					cLuce.Do16Bit();
					cLuce.input.pBits += 2;
					cLuce.output.pBits += 2;
				}
			}
			else
			{
				cLuce.numchannels = 4;
				cLuce.alphaInInputChannel = 3;
				cLuce.input.iPixelSize = 8;
				cLuce.input.iStride = 8*tSize.nX;
				cLuce.input.pBits = reinterpret_cast<BYTE*>(pBuf.m_p);
				cLuce.output.iPixelSize = 8;
				cLuce.output.iStride = 8*tSize.nX;
				cLuce.output.pBits = reinterpret_cast<BYTE*>(pBuf.m_p+tSize.nX*tSize.nY);
				cLuce.alpha.iPixelSize = cLuce.alpha.iStride = 0;
				cLuce.alpha.pBits = NULL;
				cLuce.Do16Bit();
			}

			{
				TRasterImagePixel* pD = pImage;
				TRasterImagePixel16 const* pS = pBuf.m_p+tSize.nX*tSize.nY;
				for (TRasterImagePixel16 const* pSEnd = pS+(tSize.nX*tSize.nY); pS != pSEnd; ++pD, ++pS)
				{
					int const nA = pS->wA>>8;
					if (nA)
					{
						int nR = ((pS->wR-32768)*255)/nA+32768;
						int nG = ((pS->wG-32768)*255)/nA+32768;
						int nB = ((pS->wB-32768)*255)/nA+32768;
						pD->bB = pG->InvGamma(nB < 0 ? 0 : (nB > 65535 ? 65535 : nB));
						pD->bG = pG->InvGamma(nG < 0 ? 0 : (nG > 65535 ? 65535 : nG));
						pD->bR = pG->InvGamma(nR < 0 ? 0 : (nR > 65535 ? 65535 : nR));
						pD->bA = nA;
					}
					else
					{
						static TRasterImagePixel const t0 = {0, 0, 0, 0};
						*pD = t0;
					}
				}
			}


			pRI->TileSet(EICIRGBA, CImagePoint(0, 0), &tSize, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(pImage.m_p), FALSE);
			return S_OK;

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}


	// IRasterImageFilter methods
public:
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
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		if (a_pCanvas && a_pRect)
		{
			if (a_pRect->tTL.nX > 0) a_pRect->tTL.nX = 0;
			if (a_pRect->tTL.nY > 0) a_pRect->tTL.nY = 0;
			if (a_pRect->tBR.nX < LONG(a_pCanvas->nX)) a_pRect->tBR.nX = a_pCanvas->nX;
			if (a_pRect->tBR.nY < LONG(a_pCanvas->nY)) a_pRect->tBR.nY = a_pCanvas->nY;
		}
		return S_OK;
	}

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL) return E_POINTER;
		try
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Light rays[0405]Paprsky světla");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			//static IRPolyPoint const rays[] = {{193, 249}, {122, 144}, {33, 160}, {15, 66}, {115, 113}, {171, 8}, {252, 37}, {147, 121}, {239, 177}};
			static IRPathPoint const sphere[] =
			{
				{192, 128, 0, -35.3462, 0, 35.3462},
				{128, 64, -35.3462, 0, 35.3462, 0},
				{64, 128, 0, 35.3462, 0, -35.3462},
				{128, 192, 35.3462, 0, -35.3462, 0},
			};
			static IRPathPoint const hilight[] =
			{
				{116, 91, -3.27295, -3.90055, 3.27295, 3.90055},
				{102, 91, -4.49733, 3.77371, 4.49733, -3.77371},
				{100, 105, 3.27295, 3.90055, -3.27295, -3.90055},
				{114, 105, 4.49733, -3.77371, -4.49733, 3.77371},
			};
			static IRPathPoint const shadow[] =
			{
				{181, 105, 6, 19, -6, 42},
				{167, 171, -23, 21, 20.8875, -19.0712},
				{98, 179, 45, -5, 16, 7},
			};
			static IRPathPoint const spot1[] =
			{
				{170, 10, -2.60162, 9.70924, 2.60162, -9.70924},
				{208, 39, 23.4194, 6.27529, -23.4194, -6.27529},
				{255, 33, 2.60162, -9.70924, -2.60162, 9.70924},
				{217, 4, -23.4194, -6.27529, 23.4194, 6.27529},
			};
			static IRPathPoint const ray1[] =
			{
				{163, 102, -12, 2, 0, 0},
				{133, 88, 0, 0, 1, 11},
				{170, 9, 0, 0, 0, 0},
				{251, 37, 0, 0, 0, 0},
			};
			static IRPathPoint const spot2[] =
			{
				{238, 177, -8.89999, -5.13838, 8.89999, 5.13838},
				{201, 205, -11.9365, 20.6748, 11.9365, -20.6748},
				{195, 251, 8.89999, 5.13838, -8.89999, -5.13838},
				{233, 223, 11.9365, -20.6748, -11.9365, 20.6748},
			};
			static IRPathPoint const ray2[] =
			{
				{143, 165, -1, -10, 0, 0},
				{163, 136, 0, 0, -13, 1},
				{237, 176, 0, 0, 0, 0},
				{192, 249, 0, 0, 0, 0},
			};
			static IRPathPoint const spot3[] =
			{
				{9, 65, -9.96594, 2.67041, 9.96594, -2.67041},
				{4, 118, 7.09016, 26.4604, -7.09016, -26.4604},
				{35, 161, 9.96594, -2.67041, -9.96594, 2.67041},
				{40, 108, -7.09016, -26.4604, 7.09016, 26.4604},
			};
			static IRPathPoint const ray3[] =
			{
				{96, 110, 6, 9, 0, 0},
				{97, 144, 0, 0, 9, -6},
				{37, 160, 0, 0, 0, 0},
				{15, 66, 0, 0, 0, 0},
			};
			static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
			//IRFill lightFill(0xffebe867);
			//IROutlinedFill light(&lightFill, pSI->GetMaterial(ESMContrast));
			CIconRendererReceiver cRenderer(a_nSize);
			//cRenderer(&tCanvas, itemsof(rays), rays, &light);
			IRFill hilightFill(0x7fffffff);
			IRFill shadowFill(0x7f000000);
			cRenderer(&tCanvas, itemsof(sphere), sphere, pSI->GetMaterial(ESMScheme1Color2));
			cRenderer(&tCanvas, itemsof(hilight), hilight, &hilightFill);
			cRenderer(&tCanvas, itemsof(shadow), shadow, &shadowFill);
			cRenderer(&tCanvas, itemsof(ray1), ray1, pSI->GetMaterial(ESMScheme1Color3));//ESMBrightLight));
			cRenderer(&tCanvas, itemsof(spot1), spot1, pSI->GetMaterial(ESMScheme1Color3));//ESMBackground));
			cRenderer(&tCanvas, itemsof(ray2), ray2, pSI->GetMaterial(ESMScheme1Color3));//ESMBrightLight));
			cRenderer(&tCanvas, itemsof(spot2), spot2, pSI->GetMaterial(ESMScheme1Color3));//ESMBackground));
			cRenderer(&tCanvas, itemsof(ray3), ray3, pSI->GetMaterial(ESMScheme1Color3));//ESMBrightLight));
			cRenderer(&tCanvas, itemsof(spot3), spot3, pSI->GetMaterial(ESMScheme1Color3));//ESMBackground));
			*a_phIcon = cRenderer.get();

			return S_OK;
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

private:
	class ATL_NO_VTABLE CConfigGUI :
		public CCustomConfigResourcelessWndImpl<CConfigGUI>,
		public CDialogResize<CConfigGUI>
	{
	public:
		CConfigGUI()
		{
		}

		enum { IDC_INTENSITY_EDIT = 100, IDC_INTENSITY_SLIDER, IDC_INTENSITY_SPIN, IDC_INTENSITY_UNIT, IDC_ATTENUATION };

		BEGIN_DIALOG_EX(0, 0, 120, 28, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_LTEXT(_T("[0409]Attenuation:[0405]Útlum:"), IDC_STATIC, 0, 2, 45, 8, WS_VISIBLE, 0)
			CONTROL_COMBOBOX(IDC_ATTENUATION, 45, 0, 75, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
			CONTROL_LTEXT(_T("[0409]Intensity:[0405]Intenzita:"), IDC_STATIC, 0, 18, 45, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_INTENSITY_EDIT, 78, 16, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
			CONTROL_CONTROL(_T(""), IDC_INTENSITY_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 16, 11, 12, 0)
			CONTROL_CONTROL(_T(""), IDC_INTENSITY_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 16, 33, 12, 0)
			CONTROL_RTEXT(_T("%"), IDC_INTENSITY_UNIT, 112, 18, 8, 8, WS_VISIBLE, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUI)
			CHAIN_MSG_MAP(CDialogResize<CConfigGUI>)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUI>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigGUI)
			DLGRESIZE_CONTROL(IDC_ATTENUATION, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_INTENSITY_SLIDER, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_INTENSITY_EDIT, DLSZ_MOVE_X)
			DLGRESIZE_CONTROL(IDC_INTENSITY_SPIN, DLSZ_MOVE_X)
			DLGRESIZE_CONTROL(IDC_INTENSITY_UNIT, DLSZ_MOVE_X)
		END_DLGRESIZE_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUI)
			CONFIGITEM_COMBOBOX(IDC_ATTENUATION, CFGID_ATTENUATION)
			CONFIGITEM_SLIDER_TRACKUPDATE(IDC_INTENSITY_SLIDER, CFGID_INTENSITY)
			CONFIGITEM_EDITBOX(IDC_INTENSITY_EDIT, CFGID_INTENSITY)
		END_CONFIGITEM_MAP()

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			DlgResize_Init(false, false, 0);

			CUpDownCtrl wndUD = GetDlgItem(IDC_INTENSITY_SPIN);
			wndUD.SetRange(0, 200);

			return 1;
		}
		LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (a_lParam)
				*reinterpret_cast<float*>(a_lParam) = 1.0f;
			return 0;
		}

		bool GetSliderRange(wchar_t const* UNREF(a_pszName), TConfigValue* a_pFrom, TConfigValue* a_pTo, TConfigValue* a_pStep)
		{
			a_pFrom->eTypeID = a_pTo->eTypeID = a_pStep->eTypeID = ECVTFloat;
			a_pFrom->fVal = 0;
			a_pTo->fVal = 200;
			a_pStep->fVal = 1.0f;
			return true;
		}
		// scaling for CFGID_OPACITY: 0-100% on screen <-> 0-1 internally
		void FloatToValue(WCHAR const* id, float f, CConfigValue& val)
		{
			val = f/100.0f;
		}
		float ValueToFloat(WCHAR const* id, TConfigValue const& val)
		{
			return val.fVal*100.0f;
		}
	};

	class ATL_NO_VTABLE CWrapper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ICanvasInteractingWrapper
	{
	public:
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, float a_fSourceX, float a_fSourceY)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			m_fSourceX = a_fSourceX;
			m_fSourceY = a_fSourceY;
		}
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
		{
			m_nSizeX = a_nSizeX;
			m_nSizeY = a_nSizeY;

			CConfigValue cSrc;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SOURCE), &cSrc);
			m_fSourceX = a_nSizeX*cSrc[0];
			m_fSourceY = a_nSizeY*cSrc[1];
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
				if (a_pPos)
				{
					a_pPos->fX = m_fSourceX;
					a_pPos->fY = m_fSourceY;
				}
				if (a_pClass)
					*a_pClass = 0;
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
					if (a_pPos->fX != m_fSourceX || a_pPos->fY != m_fSourceY) // or use epsilon? ...not that important
					{
						CComObject<CWrapper>* p = NULL;
						CComObject<CWrapper>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, a_pPos->fX, a_pPos->fY);
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
			return S_OK;
		}

		STDMETHOD(ToConfig)(IConfig* a_pConfig)
		{
			CComBSTR cSource(CFGID_SOURCE);
			BSTR aIDs[1] = {cSource};
			TConfigValue aVals[1];
			aVals[0] = CConfigValue(m_fSourceX/m_nSizeX, m_fSourceY/m_nSizeY);
			return a_pConfig->ItemValuesSet(1, aIDs, aVals);
		}

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		float m_fSourceX;
		float m_fSourceY;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(ImageFilterLuce), CImageFilterLuce)
