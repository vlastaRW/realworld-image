
#include "stdafx.h"
#include "RWDocumentImageRaster.h"
#include <RWProcessingTags.h>

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>
#include <ConfigCustomGUIImpl.h>
#include <IconRenderer.h>
#include <RWImagingDocumentUtils.h>

#ifndef WIN64
LONG64 InterlockedAdd64(LONG64 volatile* Addend, LONG64 Value)
{
	EnterCriticalSection(&_pModule->get_m_csObjMap());
	LONG64 l = *Addend += Value;
	LeaveCriticalSection(&_pModule->get_m_csObjMap());
	return l;
}
#endif

static OLECHAR const CFGID_OPACITY[] = L"Opacity";

// {95FDD82E-E7C5-48d1-A43F-A2A99AF8D5A6}
extern const GUID CLSID_ImageFilterOpacity = {0x95fdd82e, 0xe7c5, 0x48d1, {0xa4, 0x3f, 0xa2, 0xa9, 0x9a, 0xf8, 0xd5, 0xa6}};


class ATL_NO_VTABLE CImageFilterOpacity :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageFilterOpacity>,
	//public CDesignerFrameIconsImpl<1, &g_tIconIDLayerAddRaster, &ARICONID>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter,
	public ILayerStyle
{
public:
	CImageFilterOpacity()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CImageFilterOpacity)

BEGIN_CATEGORY_MAP(CImageFilterOpacity)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	//IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
	IMPLEMENTED_CATEGORY(CATID_TagImageColorAdjustment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CImageFilterOpacity)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
	COM_INTERFACE_ENTRY(ILayerStyle)
	//COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"Raster Image - Opacity");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_OPACITY), CMultiLanguageString::GetAuto(L"[0409]Opacity[0405]Krytí"), CMultiLanguageString::GetAuto(L"[0409]Controls how much does the current layer influence the final image (0-100%).[0405]Určuje, jak moc bude aktuální vrstva ovlivňovat výsledný obrázek (0-100%)."), CConfigValue(1.0f), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_ImageFilterOpacity, CConfigGUI>::FinalizeConfig(pCfgInit);

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
			if (pRI == NULL)
				return E_NOINTERFACE;

			CConfigValue cOpacity;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPACITY), &cOpacity);
			float const fOpacity = cOpacity;

			if (fOpacity == 1.0f)
			{
				// no change
				return S_FALSE;
			}

			TImagePoint tOrigin = {0, 0};
			TImageSize tSize = {0, 0};
			pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL);
			TPixelChannel  tDefault;
			pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));

			// compute new default
			TPixelChannel tNewDefault = tDefault;
			{
				ULONG nA = tDefault.bA;
				if (fOpacity <= 0.0f)
					tNewDefault.bA = 0;
				else if (fOpacity < 1.0f)
					tNewDefault.bA = (tDefault.bA*ULONG(fOpacity*65536.0f)+32768)>>16;
				else if (fOpacity > 1.0f)
					tNewDefault.bA = min(255, int(tDefault.bA*fOpacity+0.5f));
			}
			if (tSize.nX*tSize.nY == 0)
			{
				// empty canvas - just adjust the default (if needed)
				if (tDefault.n != tNewDefault.n)
				{
					EImageChannelID e = EICIRGBA;
					pRI->ChannelsSet(1, &e, &tNewDefault);
				}
				return S_OK;
			}
			if (fOpacity <= 0.0f)
			{
				// erase everything
				tOrigin.nX = tSize.nY = 0;
				tSize.nX = tSize.nY = 0;
				if (tDefault.n != tNewDefault.n)
				{
					EImageChannelID e = EICIRGBA;
					pRI->ChannelsSet(1, &e, &tNewDefault);
				}
				return pRI->TileSet(EICIRGBA, &tOrigin, &tSize, NULL, 0, NULL, TRUE);
			}

			CAutoPixelBuffer cBuf(pRI, tSize);

			CComPtr<IThreadPool> pThPool;
			if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
				RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

			ULONGLONG totalA;
			{
				CComObjectStackEx<COpacityTask> cXform;
				cXform.Init(pRI, tOrigin, tSize, cBuf.Buffer(), fOpacity);

				if (pThPool)
					pThPool->Execute(0, &cXform);
				else
					cXform.Execute(0, 1);
				totalA = cXform.TotalAlpha();
			}

			return cBuf.Replace(tOrigin, &tOrigin, &tSize, &totalA, tNewDefault);
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
			CComPtr<ILocalizedString> pName;
			pName.Attach(new CMultiLanguageString(L"[0409]Opacity[0405]Krytí"));
			if (a_pConfig == NULL)
			{
				*a_ppName = pName.Detach();
				return S_OK;
			}
			CConfigValue cOpacity;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPACITY), &cOpacity);
			//if (cOpacity.operator float() == 100.0f)
			//{
			//	*a_ppName = pName.Detach();
			//	return S_OK;
			//}

			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			CComPtr<ILocalizedString> pTmp = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %i%%")));
			pStr->Init(pTempl, pName, static_cast<int>(cOpacity.operator float()*100.0f+0.5f));
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(PreviewIconID)(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID)
	{
		if (a_pIconID == NULL)
			return E_POINTER;
		*a_pIconID = CLSID_ImageFilterOpacity;
		return S_OK;
	}
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			static IRPolyPoint const sq0[] = {{0, 0}, {80, 0}, {80, 80}, {0, 80}};
			static IRPolyPoint const sq1[] = {{104, 8}, {168, 8}, {168, 72}, {104, 72}};
			static IRPolyPoint const sq2[] = {{192, 16}, {240, 16}, {240, 64}, {192, 64}};
			static IRPolyPoint const sq3[] = {{8, 104}, {72, 104}, {72, 168}, {8, 168}};
			static IRPolyPoint const sq4[] = {{112, 112}, {160, 112}, {160, 160}, {112, 160}};
			static IRPolyPoint const sq5[] = {{200, 120}, {232, 120}, {232, 152}, {200, 152}};
			static IRPolyPoint const sq6[] = {{16, 192}, {64, 192}, {64, 240}, {16, 240}};
			static IRPolyPoint const sq7[] = {{120, 200}, {152, 200}, {152, 232}, {120, 232}};
			static IRPolyPoint const sq8[] = {{208, 208}, {224, 208}, {224, 224}, {208, 224}};
			static IRPolygon const squares[] =
			{
				{itemsof(sq0), sq0}, {itemsof(sq1), sq1}, {itemsof(sq2), sq2},
				{itemsof(sq3), sq3}, {itemsof(sq4), sq4}, {itemsof(sq5), sq5},
				{itemsof(sq6), sq6}, {itemsof(sq7), sq7}, {itemsof(sq8), sq8},
			};
			static IRGridItem const grid[] = {{0, 0}, {0, 80}};
			static IRCanvas const tCanvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
			CIconRendererReceiver cRenderer(a_nSize);
			cRenderer(&tCanvas, itemsof(squares), squares, pSI->GetMaterial(ESMScheme1Color1));
			*a_phIcon = cRenderer.get();

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// ILayerStyle methods
public:
	STDMETHOD_(BYTE, ExecutionPriority)() { return ELSEPOpacity + 5; }
	STDMETHOD(IsPriorityAnchor)() { return S_OK; }

private:
	class ATL_NO_VTABLE CConfigGUI :
		public CCustomConfigResourcelessWndImpl<CConfigGUI>,
		public CDialogResize<CConfigGUI>
	{
	public:
		CConfigGUI()
		{
		}

		enum { IDC_OPACITY_EDIT = 100, IDC_OPACITY_SLIDER, IDC_OPACITY_SPIN, IDC_OPACITY_UNIT };

		BEGIN_DIALOG_EX(0, 0, 120, 12, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_LTEXT(_T("[0409]Opacity:[0405]Krytí:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_OPACITY_EDIT, 78, 0, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
			CONTROL_CONTROL(_T(""), IDC_OPACITY_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 0, 11, 12, 0)
			CONTROL_CONTROL(_T(""), IDC_OPACITY_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 0, 33, 12, 0)
			CONTROL_RTEXT(_T("%"), IDC_OPACITY_UNIT, 112, 2, 8, 8, WS_VISIBLE, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUI)
			CHAIN_MSG_MAP(CDialogResize<CConfigGUI>)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUI>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigGUI)
			DLGRESIZE_CONTROL(IDC_OPACITY_SLIDER, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_OPACITY_EDIT, DLSZ_MOVE_X)
			DLGRESIZE_CONTROL(IDC_OPACITY_SPIN, DLSZ_MOVE_X)
			DLGRESIZE_CONTROL(IDC_OPACITY_UNIT, DLSZ_MOVE_X)
		END_DLGRESIZE_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUI)
			CONFIGITEM_SLIDER_TRACKUPDATE(IDC_OPACITY_SLIDER, CFGID_OPACITY)
			CONFIGITEM_EDITBOX(IDC_OPACITY_EDIT, CFGID_OPACITY)
		END_CONFIGITEM_MAP()

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			DlgResize_Init(false, false, 0);

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
			a_pTo->fVal = 100;
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

	class ATL_NO_VTABLE COpacityTask :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IThreadedTask
	{
	public:
		COpacityTask() : m_pSource(NULL) {}
		~COpacityTask() { delete m_pSource; }
		void Init(IDocumentImage* a_pImage, TImagePoint a_tOrigin, TImageSize a_tSize, TPixelChannel* a_pDst, float a_fOpacity)
		{
			m_pSource = new SLockedImageBuffer(a_pImage);
			a_pImage->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
			tArea.tTL = a_tOrigin;
			tArea.tBR.nX = a_tOrigin.nX+a_tSize.nX;
			tArea.tBR.nY = a_tOrigin.nY+a_tSize.nY;
			pDst = a_pDst;
			m_fOpacity = a_fOpacity;
			m_nTotalA = 0;
		}
		ULONGLONG TotalAlpha() const { return m_nTotalA; }

	BEGIN_COM_MAP(COpacityTask)
		COM_INTERFACE_ENTRY(IThreadedTask)
	END_COM_MAP()


		STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
		{
			ULONG nY = tArea.tBR.nY-tArea.tTL.nY;
			if (a_nIndex > nY)
				return S_FALSE;
			if (a_nTotal > nY)
				a_nTotal = nY;
			size_t nStride = tArea.tBR.nX-tArea.tTL.nX;
			TRasterImageRect tSub = tArea;
			tSub.tTL.nY = tArea.tTL.nY+nY*a_nIndex/a_nTotal;
			tSub.tBR.nY = tArea.tTL.nY+nY*(a_nIndex+1)/a_nTotal;
			TPixelChannel* pD = pDst + nStride*(tSub.tTL.nY-tArea.tTL.nY);
			CComObjectStackEx<CVisitor> cVis;
			cVis.Init(tSub.tTL, pD, nStride, m_fOpacity, &m_nTotalA);

			TImageSize tSubSize = {tSub.tBR.nX-tSub.tTL.nX, tSub.tBR.nY-tSub.tTL.nY};
			return RGBAInspectImpl(m_pSource->tContentOrigin, m_pSource->tContentSize, m_pSource->Content(), m_pSource->tAllocSize.nX, tDefault, &tSub.tTL, &tSubSize, &cVis, NULL);
			//TImageTile tTile;
			//tTile.nChannelIDs = EICIRGBA;
			//tTile.tOrigin = tSub.tTL;
			//tTile.tSize = tSubSize;
			//tTile.tStride.nX = 1;
			//tTile.tStride.nY = m_pSource->tAllocSize.nX;
			//tTile.nPixels = tSubSize.nX*tSubSize.nY;
			//tTile.pData = m_pSource->pData + m_pSource->tAllocSize.nX*(tSub.tTL.nY-m_pSource->tAllocOrigin.nY) + (tSub.tTL.nX-m_pSource->tAllocOrigin.nX);
			//return cVis.Visit(1, &tTile, NULL);
			//return pImg->Inspect(EICIRGBA, &tSub.tTL, &tSubSize, &cVis, NULL, EIRIAccurate);
		}

	private:
		class ATL_NO_VTABLE CVisitor :
			public CComObjectRootEx<CComMultiThreadModel>,
			public IImageVisitor
		{
		public:
			void Init(TImagePoint a_tOrig, TPixelChannel* a_pDst, size_t a_nStride, float a_fOpacity, LONGLONG* a_pTotalA)
			{
				tOrig = a_tOrig;
				pDst = a_pDst;
				nStride = a_nStride;
				m_fOpacity = a_fOpacity;
				m_pTotalA = a_pTotalA;
			}

		BEGIN_COM_MAP(CVisitor)
			COM_INTERFACE_ENTRY(IImageVisitor)
		END_COM_MAP()

			// IImageVisitor methods
		public:
			STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl*)
			{
				ULONGLONG nTA = 0;
				ULONG const n = m_fOpacity*65536.0f;
				if (m_fOpacity < 1.0f)
				{
					for (TImageTile const* const pE = a_aTiles+a_nTiles; a_aTiles != pE; ++a_aTiles)
					{
						TPixelChannel* pD = pDst + (a_aTiles->tOrigin.nX-tOrig.nX) + nStride*(a_aTiles->tOrigin.nY-tOrig.nY);
						TPixelChannel const* pS = a_aTiles->pData;
						for (ULONG nY = 0; nY < a_aTiles->tSize.nY; ++nY, pD += nStride, pS += a_aTiles->tStride.nY)
						{
							TPixelChannel* pD1 = pD;
							TPixelChannel const* pS1 = pS;
							for (TPixelChannel* pDE = pD+a_aTiles->tSize.nX; pD1 != pDE; ++pD1, pS1 += a_aTiles->tStride.nX)
							{
								ULONG nA = (((pS1->n>>24)*n+32768)<<8)&0xff000000;
								nTA += nA;
								pD1->n = (pS1->n&0xffffff)|nA;
							}
						}
					}
				}
				else
				{
					for (TImageTile const* const pE = a_aTiles+a_nTiles; a_aTiles != pE; ++a_aTiles)
					{
						TPixelChannel* pD = pDst + (a_aTiles->tOrigin.nX-tOrig.nX) + nStride*(a_aTiles->tOrigin.nY-tOrig.nY);
						TPixelChannel const* pS = a_aTiles->pData;
						ULONGLONG nTotalA = 0;
						for (ULONG nY = 0; nY < a_aTiles->tSize.nY; ++nY, pD += nStride, pS += a_aTiles->tStride.nY)
						{
							TPixelChannel* pD1 = pD;
							TPixelChannel const* pS1 = pS;
							for (TPixelChannel* pDE = pD+a_aTiles->tSize.nX; pD1 != pDE; ++pD1, pS1 += a_aTiles->tStride.nX)
							{
								ULONG nA = (pS1->n>>24)*n+32768;
								nA = min(nA, 0xff0000);
								nA = (nA<<8)&0xff000000;
								nTA += nA;
								pD1->n = ((pS1->n)&0xffffff)|nA;
							}
						}
					}
				}
				nTA >>= 24;
				InterlockedAdd64(m_pTotalA, nTA);
				return S_OK;
			}

		private:
			TImagePoint tOrig;
			TPixelChannel* pDst;
			size_t nStride;
			float m_fOpacity;
			LONGLONG* m_pTotalA;
		};

	private:
		SLockedImageBuffer* m_pSource;
		TPixelChannel tDefault;
		TRasterImageRect tArea;
		TPixelChannel* pDst;
		float m_fOpacity;
		LONGLONG m_nTotalA;
	};
};

OBJECT_ENTRY_AUTO(CLSID_ImageFilterOpacity, CImageFilterOpacity)
