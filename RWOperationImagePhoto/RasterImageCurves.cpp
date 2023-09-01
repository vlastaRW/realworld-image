
#include "stdafx.h"

#include "RWOperationImagePhoto.h"
#include <ConfigCustomGUIImpl.h>
#include <WTL_Curve.h>
#include <RWProcessingTags.h>
#include <MultiLanguageString.h>
#include <IconRenderer.h>
#include "../RWOperationImageRaster/PixelLevelTask.h"


extern GUID const CLSID_DocumentOperationRasterImageCurves = {0x2107e6ea, 0xfa97, 0x42d5, {0xaa, 0x2a, 0xce, 0xe2, 0x13, 0x54, 0x6, 0x21}};
static wchar_t const CFGID_CURVE[] = L"Curve";
static wchar_t const CFGID_CHANNELS[] = L"Channels";
static LONG const CFGVAL_CH_RED = 1;
static LONG const CFGVAL_CH_GREEN = 2;
static LONG const CFGVAL_CH_BLUE = 4;

HICON GetIconCurves(ULONG a_nSize)
{
	static IRPathPoint const curve[] =
	{
		{224, 56, -5, 0, 0, 0},
		{208, 58, -12.2108, 3.53866, 3.95718, -1.14678},
		{177, 80, -25.1834, 29.6968, 8.22254, -9.6962},
		{118, 181, -12.6404, 20.6653, 20.3174, -33.2161},
		{67, 233, -6.186, 2.93456, 21.8867, -10.3828},
		{32, 240, 0, 0, 11, 0},
		{32, 200, 7, 0, 0, 0},
		{54, 194, 12.709, -8.08789, -4.16664, 2.65162},
		{84, 160, 23.0557, -37.7724, -7.84845, 12.8582},
		{153, 47, 9.52979, -9.78232, -30.8797, 31.698},
		{187, 23, 9.98193, -4.27904, -12.5522, 5.38085},
		{224, 16, 0, 0, -11, 0},
	};
	static IRPathPoint const point1[] =
	{
		{64, 220, 0, -17.6731, 0, 17.6731},
		{32, 188, -17.6731, 0, 17.6731, 0},
		{0, 220, 0, 17.6731, 0, -17.6731},
		{32, 252, 17.6731, 0, -17.6731, 0},
	};
	static IRPathPoint const point2[] =
	{
		{138, 164, 0, -17.6731, 0, 17.6731},
		{106, 132, -17.6731, 0, 17.6731, 0},
		{74, 164, 0, 17.6731, 0, -17.6731},
		{106, 196, 17.6731, 0, -17.6731, 0},
	};
	static IRPathPoint const point3[] =
	{
		{181, 84, 0, -17.6731, 0, 17.6731},
		{149, 52, -17.6731, 0, 17.6731, 0},
		{117, 84, 0, 17.6731, 0, -17.6731},
		{149, 116, 17.6731, 0, -17.6731, 0},
	};
	static IRPathPoint const point4[] =
	{
		{256, 36, 0, -17.6731, 0, 17.6731},
		{224, 4, -17.6731, 0, 17.6731, 0},
		{192, 36, 0, 17.6731, 0, -17.6731},
		{224, 68, 17.6731, 0, -17.6731, 0},
	};
	static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(curve), curve, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(point1), point1, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas, itemsof(point2), point2, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas, itemsof(point3), point3, pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&canvas, itemsof(point4), point4, pSI->GetMaterial(ESMScheme1Color1));
	return cRenderer.get();
}


// CDocumentOperationRasterImageCurves

class ATL_NO_VTABLE CDocumentOperationRasterImageCurves :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageCurves, &CLSID_DocumentOperationRasterImageCurves>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationRasterImageCurves()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageCurves)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageCurves)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	IMPLEMENTED_CATEGORY(CATID_TagImageColorAdjustment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageCurves)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Raster Image - Curves[0405]Rastrový obrázek - křivky");
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

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_CHANNELS), CMultiLanguageString::GetAuto(L"[0409]Channels[0405]Kanály"), CMultiLanguageString::GetAuto(L"[0409]Curve for all color channels.[0405]Křivka pro všechny barevné kanály."), CConfigValue(CFGVAL_CH_RED|CFGVAL_CH_GREEN|CFGVAL_CH_BLUE), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_CURVE), CMultiLanguageString::GetAuto(L"[0409]Curve[0405]Křivka"), CMultiLanguageString::GetAuto(L"[0409]Curve for all color channels.[0405]Křivka pro všechny barevné kanály."), CConfigValue(L"0,0;255,255;"), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_DocumentOperationRasterImageCurves, CConfigGUIDlg>::FinalizeConfig(pCfgInit);

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
			CComPtr<IDocumentRasterImage> pRI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

			CConfigValue cCurve;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CURVE), &cCurve);
			TCurvePoints aPoints;
			CCurveControl::ParseCurvePoints(cCurve.operator BSTR(), aPoints);

			static BYTE dummyLUT[256];
			static bool initialized = false;
			if (!initialized)
			{
				for (int i = 0; i < 256; ++i)
					dummyLUT[i] = i;
				initialized = true;
			}
			BYTE realLUT[256];
			for (ULONG i = 0; i < 256; ++i)
			{
				float f = GetCurveValue(powf(i/255.0f, 2.2f)*255.0f, aPoints)/255.0f;
				if (f < 0.0f) f = 0.0f;
				else if (f > 1.0f) f = 1.0f;
				else f = powf(f, 1.0f/2.2f);
				realLUT[i] = f*255.0f+0.5f;
			}

			CConfigValue cCh;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_CHANNELS), &cCh);
			BYTE const* aLUTR = cCh.operator LONG()&CFGVAL_CH_RED ? realLUT : dummyLUT;
			BYTE const* aLUTG = cCh.operator LONG()&CFGVAL_CH_GREEN ? realLUT : dummyLUT;
			BYTE const* aLUTB = cCh.operator LONG()&CFGVAL_CH_BLUE ? realLUT : dummyLUT;

			TImagePoint tOrigin = {0, 0};
			TImageSize tSize = {0, 0};
			pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL);
			TRasterImageRect tR = {tOrigin, {tOrigin.nX+tSize.nX, tOrigin.nY+tSize.nY}};
			TPixelChannel  tDefault;
			pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

			CLUTOp cOp;
			cOp.pLUTR = aLUTR;
			cOp.pLUTG = aLUTG;
			cOp.pLUTB = aLUTB;
			ULONGLONG totalA = 0;
			cOp.pTotalA = reinterpret_cast<LONGLONG*>(&totalA);
			tDefault = CLUTOp::ProcessPixel(tDefault, aLUTR, aLUTG, aLUTB);

			CAutoPixelBuffer cBuf(pRI, tSize);

			{
				CComPtr<IThreadPool> pThPool;
				if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
					RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

				CComObjectStackEx<CPixelLevelTask<CLUTOp> > cXform;
				cXform.Init(pRI, tR, cBuf.Buffer(), cOp);

				if (pThPool)
					pThPool->Execute(0, &cXform);
				else
					cXform.Execute(0, 1);
			}

			return cBuf.Replace(tR.tTL, &tR.tTL, &tSize, &totalA, tDefault);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			*a_ppName = new CMultiLanguageString(L"[0409]Curves[0405]Křivky");
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
			*a_phIcon = GetIconCurves(a_nSize);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CConfigGUIDlg :
		public CCustomConfigResourcelessWndImpl<CConfigGUIDlg>,
		public CDialogResize<CConfigGUIDlg>
	{
	public:
		CConfigGUIDlg()
		{
		}

		enum
		{
			IDC_CHANNEL_RED = 200,
			IDC_CHANNEL_GREEN,
			IDC_CHANNEL_BLUE,
			IDC_CURVE,
		};

		BEGIN_DIALOG_EX(0, 0, 120, 130, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_CHECKBOX(_T("[0409]Red[0405]Červená"), IDC_CHANNEL_RED, 0, 0, 40, 10, WS_VISIBLE | WS_TABSTOP, 0)
			CONTROL_CHECKBOX(_T("[0409]Green[0405]Zelená"), IDC_CHANNEL_GREEN, 40, 0, 40, 10, WS_VISIBLE | WS_TABSTOP, 0)
			CONTROL_CHECKBOX(_T("[0409]Blue[0405]Modrá"), IDC_CHANNEL_BLUE, 80, 0, 40, 10, WS_VISIBLE | WS_TABSTOP, 0)
			//CONTROL_LTEXT(_T("[0409]Scale:[0405]Škála:"), IDC_SCALE_LABEL, 0, 2, 44, 8, WS_VISIBLE, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUIDlg)
			CHAIN_MSG_MAP(CDialogResize<CConfigGUIDlg>)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			NOTIFY_HANDLER(IDC_CURVE, CCurveControl::CPC_POINT_MOVED, OnCurveChanged)
			NOTIFY_HANDLER(IDC_CURVE, CCurveControl::CPC_DRAG_FINISHED, OnCurveChanged)
			NOTIFY_HANDLER(IDC_CURVE, CCurveControl::CPC_DRAG_START, OnCurveChanged)
		END_MSG_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUIDlg)
			CONFIGITEM_CHECKBOX_FLAG(IDC_CHANNEL_RED, CFGID_CHANNELS, CFGVAL_CH_RED)
			CONFIGITEM_CHECKBOX_FLAG(IDC_CHANNEL_GREEN, CFGID_CHANNELS, CFGVAL_CH_GREEN)
			CONFIGITEM_CHECKBOX_FLAG(IDC_CHANNEL_BLUE, CFGID_CHANNELS, CFGVAL_CH_BLUE)
		END_CONFIGITEM_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigGUIDlg)
			DLGRESIZE_CONTROL(IDC_CURVE, DLSZ_SIZE_X)
		END_DLGRESIZE_MAP()

		void ExtraInitDialog()
		{
			RECT rc = {0, 14, 120, 130};
			MapDialogRect(&rc);

			m_wndCurve.Create(m_hWnd, &rc, 0, 0, WS_EX_CLIENTEDGE, IDC_CURVE);
		}

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			DlgResize_Init(false, false, 0);

			return 1;
		}

		LRESULT OnCurveChanged(int, LPNMHDR, BOOL&)
		{
			try
			{
				TCurvePoints aPoints;
				m_wndCurve.GetPoints(&aPoints);
				CComBSTR cPoints;
				m_wndCurve.SerializeCurvePoints(aPoints, cPoints);

				CComBSTR cCFGID_CURVE(CFGID_CURVE);
				CConfigValue cValCurve(cPoints);
				BSTR aIDs[1];
				aIDs[0] = cCFGID_CURVE;
				TConfigValue aVals[1];
				aVals[0] = cValCurve;
				M_Config()->ItemValuesSet(1, aIDs, aVals);
			}
			catch (...)
			{
			}

			return 0;
		}

		void ExtraConfigNotify()
		{
			if (m_wndCurve.m_hWnd)
			{
				CConfigValue cValCurve;
				M_Config()->ItemValueGet(CComBSTR(CFGID_CURVE), &cValCurve);
				TCurvePoints aPoints;
				CCurveControl::ParseCurvePoints(cValCurve.operator BSTR(), aPoints);
				m_wndCurve.SetPoints(aPoints);
			}
		}

	private:
		CCurveControl m_wndCurve;
	};

	struct CLUTOp
	{
		BYTE const* pLUTR;
		BYTE const* pLUTG;
		BYTE const* pLUTB;
		LONGLONG* pTotalA;
		static inline TPixelChannel ProcessPixel(TPixelChannel tS, BYTE const* pLUTR, BYTE const* pLUTG, BYTE const* pLUTB)
		{
			TPixelChannel t;
			t.n = pLUTR[tS.n&0xff]|(ULONG(pLUTG[(tS.n>>8)&0xff])<<8)|(ULONG(pLUTB[(tS.n>>16)&0xff])<<16)|(tS.n&0xff000000);
			return t;
		}
		void Process(TPixelChannel* pD, TPixelChannel const* pS, size_t const s, size_t const n)
		{
			BYTE const* pLUTR = this->pLUTR;
			BYTE const* pLUTG = this->pLUTG;
			BYTE const* pLUTB = this->pLUTB;
			ULONGLONG totalA = 0;
			for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
			{
				TPixelChannel tS = *pS;
				DWORD nA = tS.n&0xff000000;
				totalA += nA;

				*pD = ProcessPixel(tS, pLUTR, pLUTG, pLUTB);
			}
			totalA>>=24;
			InterlockedAdd64(pTotalA, totalA);
		}
	};
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationRasterImageCurves, CDocumentOperationRasterImageCurves)
