
#pragma once

#include <MultiLanguageString.h>

#include <GammaCorrection.h>
#include <IconRenderer.h>

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_stroke.h>
#include <agg_path_storage.h>

extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_STARTWITHAUTOZOOM[] = L"StartWithAutoZoom";
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_RENDERINGMODE[] = L"RenderingMode";
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_COMPOSED[] = L"Composed";
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_INVERTED[] = L"Inverted";
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_GRID[] = L"Grid";
static const LONG CFGVAL_2DEDIT_GRID1x1 = 1;
static const LONG CFGVAL_2DEDIT_GRID8x8 = 8;
static const LONG CFGVAL_2DEDIT_GRIDOFF = 0;
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_STYLE[] = L"Style";
static const LONG CFGVAL_2DEDIT_STYLECLASSIC = 0;
static const LONG CFGVAL_2DEDIT_STYLECLEAN = 1;
static const LONG CFGVAL_2DEDIT_STYLESUBTLE = 3;
static const LONG CFGVAL_2DEDIT_STYLEGLASS = 2;
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_HANDLESIZE[] = L"HandleSize";
extern __declspec(selectany) OLECHAR const CFGID_IMGEDIT_HIDEHANDLES[] = L"HideHandles";

struct TRasterImagePixel16
{
	WORD wB, wG, wR, wA;
};

template<class T>
class CRasterImagePainting :
	public CObserverImpl<CRasterImagePainting<T>, IComposedPreviewObserver, ULONG>,
	public IImageZoomControl,
	public IRasterEditView
{
public:
	CRasterImagePainting() :
		MARGIN_CACHE_LASTMODE(-1), m_fZoom(1.0f), m_bAutoZoom(false),
		m_fOffsetX(0.0f), m_fOffsetY(0.0f), m_bUpdating(false), m_bstrIES(L"IMAGEEDITORVIEW"),
		m_bShowComposed(true), m_eComposedMode(ECPMFinal), m_bShowInverted(false),
		m_eImageQuality(EIQCoverage), m_fBaseHandleSize(4.0f), m_bHideOnLeave(true),
		m_eGridSetting(CFGVAL_2DEDIT_GRID8x8), m_eStyle(CFGVAL_2DEDIT_STYLESUBTLE), m_bUpdateWindow(false),
		m_bComposedPreviewUpdates(true), m_bScrollBarH(false), m_bScrollBarV(false),
		m_tInputTrans(TMATRIX3X3F_IDENTITY), m_tInputInvTrans(TMATRIX3X3F_IDENTITY)
	{
#ifdef PERFMON
		RWCoCreateInstance(m_pPerfMon, __uuidof(PerformanceManager));
		if (m_pPerfMon)
		{
			m_pPerfMon->RegisterCounter(CComBSTR(L"EditorRedraw"), CMultiLanguageString::GetAuto(L"[0409]Image editor redraw canvas[0405]Překreslení plátna v editoru obrázků"), &m_nCounterRedraw);
			m_pPerfMon->RegisterCounter(CComBSTR(L"EditorObtain"), CMultiLanguageString::GetAuto(L"[0409]Image editor read pixels[0405]Načtení pixelů v editoru obrázků"), &m_nCounterObtain);
		}
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);
		freq = f.QuadPart;
#endif
		RWCoCreateInstance(m_pThPool, __uuidof(ThreadPool));
	}
	~CRasterImagePainting()
	{
		if (m_pComposedPreview)
			m_pComposedPreview->ObserverDel(ObserverGet(), 0);
	}
	static void InitPaintingConfig(IConfigWithDependencies* a_pConfig)
	{
		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_STARTWITHAUTOZOOM), CMultiLanguageString::GetAuto(L"[0409]Start with AutoZoom[0405]Zapnout automatický zoom"), CMultiLanguageString::GetAuto(L"[0409]When turned on, the AutoZoom feature will be enabled when new window opens.[0405]Je-li povoleno, bude funkce automatického nastavení zvětšení nastavena pro nová okna."), CConfigValue(true), NULL, 0, NULL);

		CComBSTR cCFGID_2DEDIT_GRID(CFGID_IMGEDIT_GRID);
		a_pConfig->ItemIns1ofN(cCFGID_2DEDIT_GRID, CMultiLanguageString::GetAuto(L"[0409]Grid[0405]Mřížka"), CMultiLanguageString::GetAuto(L"[0409]Grid size[0405]Velikost mřížky"), CConfigValue(CFGVAL_2DEDIT_GRID8x8), NULL);
		a_pConfig->ItemOptionAdd(cCFGID_2DEDIT_GRID, CConfigValue(CFGVAL_2DEDIT_GRID1x1), CMultiLanguageString::GetAuto(L"[0409]Small[0405]Malá mřížka"), 0, NULL);
		a_pConfig->ItemOptionAdd(cCFGID_2DEDIT_GRID, CConfigValue(CFGVAL_2DEDIT_GRID8x8), CMultiLanguageString::GetAuto(L"[0409]Large[0405]Velká mřížka"), 0, NULL);
		a_pConfig->ItemOptionAdd(cCFGID_2DEDIT_GRID, CConfigValue(CFGVAL_2DEDIT_GRIDOFF), CMultiLanguageString::GetAuto(L"[0409]Disabled[0405]Vypnuto"), 0, NULL);

		CComBSTR cCFGID_IMGEDIT_STYLE(CFGID_IMGEDIT_STYLE);
		a_pConfig->ItemIns1ofN(cCFGID_IMGEDIT_STYLE, NULL, NULL, CConfigValue(CFGVAL_2DEDIT_STYLESUBTLE), NULL);
		a_pConfig->ItemOptionAdd(cCFGID_IMGEDIT_STYLE, CConfigValue(CFGVAL_2DEDIT_STYLECLASSIC), NULL, 0, NULL);
		a_pConfig->ItemOptionAdd(cCFGID_IMGEDIT_STYLE, CConfigValue(CFGVAL_2DEDIT_STYLESUBTLE), NULL, 0, NULL);
		a_pConfig->ItemOptionAdd(cCFGID_IMGEDIT_STYLE, CConfigValue(CFGVAL_2DEDIT_STYLECLEAN), NULL, 0, NULL);
		//a_pConfig->ItemOptionAdd(cCFGID_IMGEDIT_STYLE, CConfigValue(CFGVAL_2DEDIT_STYLEGLASS), NULL, 0, NULL);

		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_RENDERINGMODE), NULL, NULL, CConfigValue(LONG(EIQCoverage)), NULL, 0, NULL);
		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_COMPOSED), NULL, NULL, CConfigValue(LONG(EICFinalImage)), NULL, 0, NULL);
		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_INVERTED), NULL, NULL, CConfigValue(false), NULL, 0, NULL);
		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_HANDLESIZE), NULL, NULL, CConfigValue(4.0f), NULL, 0, NULL);
		a_pConfig->ItemInsSimple(CComBSTR(CFGID_IMGEDIT_HIDEHANDLES), NULL, NULL, CConfigValue(true), NULL, 0, NULL);
	}
	void InitVisualization(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY)
	{
		m_pConfig = a_pConfig;
		CConfigValue cAutoZoom;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_STARTWITHAUTOZOOM), &cAutoZoom);
		m_bAutoZoom = cAutoZoom;
		CConfigValue cQuality;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_RENDERINGMODE), &cQuality);
		m_eImageQuality = static_cast<EImageQuality>(cQuality.operator LONG());
		CConfigValue cComposed;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_COMPOSED), &cComposed);
		m_bShowComposed = cComposed.operator LONG() != EICActiveLayer;
		m_eComposedMode = cComposed.operator LONG() == EICEmphasizedLayer ? ECPMTransparent : ECPMFinal;
		CConfigValue cInverted;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_INVERTED), &cInverted);
		m_bShowInverted = cInverted;
		CConfigValue cGrid;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_GRID), &cGrid);
		m_eGridSetting = cGrid;
		CConfigValue cStyle;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_STYLE), &cStyle);
		m_eStyle = cStyle;
		CConfigValue cHandleSize;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_HANDLESIZE), &cHandleSize);
		m_fBaseHandleSize = cHandleSize;
		CConfigValue cHideHandles;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGEDIT_HIDEHANDLES), &cHideHandles);
		m_bHideOnLeave = cHideHandles;

		UpdateMarginImgCache();

		UpdateGammaTables();

		m_tImageSize.cx = a_nSizeX;
		m_tImageSize.cy = a_nSizeY;

		static_cast<T*>(this)->M_Doc()->QueryFeatureInterface(__uuidof(IRasterImageComposedPreview), reinterpret_cast<void**>(&m_pComposedPreview));
		if (m_pComposedPreview)
		{
			m_pComposedPreview->ObserverIns(ObserverGet(), 0);
			m_pComposedPreview->InputTransform(&m_tInputTrans);
			Matrix3x3fInverse(m_tInputTrans, &m_tInputInvTrans);
		}
	}
	void InitVisualizationPostCreate()
	{
		InitPaintConstants();

		UpdateState();
	}
	void InitImageEditorState(ISharedStateManager* a_pStateMgr, IConfig* a_pConfig)
	{
		m_pSSM = a_pStateMgr;
		CComPtr<ISharedStateImageEditor> pIES;
		a_pStateMgr->StateGet(m_bstrIES, __uuidof(ISharedStateImageEditor), reinterpret_cast<void**>(&pIES));
		if (pIES)
		{
			if (m_eStyle != pIES->GetStyle())
			{
				m_eStyle = pIES->GetStyle();
				UpdateMarginImgCache();
			}
			m_eGridSetting = pIES->GetGrid();
			m_bShowComposed = pIES->GetCompositionMode() != EICActiveLayer;
			m_eComposedMode = pIES->GetCompositionMode() == EICEmphasizedLayer ? ECPMTransparent : ECPMFinal;
			m_eImageQuality = pIES->GetImageQuality();
			m_bShowInverted = pIES->GetInvertedPixels();
			m_fBaseHandleSize = pIES->GetHandleSize();
			m_bHideOnLeave = pIES->GetHideOnLeave();
		}
		else
		{
			SendImageEditorState();
		}
	}
	void SendImageEditorState()
	{
		CComPtr<ISharedStateImageEditor> pIES;
		RWCoCreateInstance(pIES, __uuidof(SharedStateImageEditor));
		pIES->SetStyle(m_eStyle);
		pIES->SetGrid(m_eGridSetting);
		pIES->SetCompositionMode(m_bShowComposed ? (m_eComposedMode == ECPMFinal ? EICFinalImage : EICEmphasizedLayer) : EICActiveLayer);
		pIES->SetImageQuality(m_eImageQuality);
		pIES->SetInvertedPixels(m_bShowInverted);
		pIES->SetHandleSize(m_fBaseHandleSize);
		pIES->SetHideOnLeave(m_bHideOnLeave);
		m_pSSM->StateSet(m_bstrIES, pIES);
	}


	ULONG M_HandleRadius() const { return m_nHandleRadius; }
	BYTE const* M_HandleImage() const { return m_pHandleImage; }
	TMatrix3x3f const& M_InputTransform() const { return m_tInputTrans; }
	TMatrix3x3f const& M_InputInvTransform() const { return m_tInputInvTrans; }
	SIZE const& M_WindowSize() const { return m_tClientSize; }
	bool M_HideHandles() const { return m_bHideOnLeave; }
	float M_ImageZoom() const { return m_fZoom; }
	bool M_AutoZoom() const { return m_bAutoZoom; }
	float M_OffsetX() const { return m_fOffsetX; }
	float M_OffsetY() const { return m_fOffsetY; }
	SIZE const& M_ImageSize() const { return m_tImageSize; }
	SIZE const& M_ZoomedSize() const { return m_tZoomedSize; }
	POINT const& M_ImagePos() const { return m_ptImagePos; }
	bool M_UpdateWindow() const { return m_bUpdateWindow; }
	void SetUpdateWindowFlag() { m_bUpdateWindow = true; }
	void ProcessUpdateWindow() { if (m_bUpdateWindow) static_cast<T*>(this)->UpdateWindow(); }
	bool MoveViewport(float a_fOffsetX, float a_fOffsetY, float a_fZoom)
	{
		m_bAutoZoom = false;
		m_fOffsetX = a_fOffsetX;
		m_fOffsetY = a_fOffsetY;
		m_fZoom = a_fZoom;
		UpdateState();
		static_cast<T*>(this)->UpdateScalableContent();
		static_cast<T*>(this)->SendViewportUpdate();
		return true;
	}
	bool MoveViewport()
	{
		if (m_bAutoZoom)
			return false;
		m_bAutoZoom = true;
		UpdateState();
		static_cast<T*>(this)->UpdateScalableContent();
		static_cast<T*>(this)->SendViewportUpdate();
		return true;
	}
	void ResizeImage(TImageSize a_tSize)
	{
		m_tImageSize.cx = a_tSize.nX;
		m_tImageSize.cy = a_tSize.nY;
		UpdateState();
	}


	void OwnerNotify(TCookie, ULONG a_nFlags)
	{
		if (static_cast<T*>(this)->m_hWnd && m_bComposedPreviewUpdates/* && a_nFlags&(ECPCUnder|ECPCOver)*/)
		{
			m_tInputTrans = TMATRIX3X3F_IDENTITY;
			m_pComposedPreview->InputTransform(&m_tInputTrans);
			Matrix3x3fInverse(m_tInputTrans, &m_tInputInvTrans);
			static_cast<T*>(this)->Invalidate(FALSE);
		}
	}
	void EnableComposedPreviewUpdates(bool enable = true) { m_bComposedPreviewUpdates = enable; }

	enum {
		MARGIN_TOP = 3, MARGIN_BOTTOM = 3, MARGIN_LEFT = 3, MARGIN_RIGHT = 3,
		TRANSPARENCY_SHIFT = 2, TRANSPARENCY_GRID = 1<<TRANSPARENCY_SHIFT,
	};

BEGIN_MSG_MAP(CRasterImagePainting<T>)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_HSCROLL, OnScroll)
	MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
END_MSG_MAP()

	// IImageZoomControl methods
public:
	STDMETHOD(CanSetZoom)(float* a_pVal)
	{
		LONG const nSize = min(m_tImageSize.cx, m_tImageSize.cy);
		if (nSize <= 0)
			return S_FALSE;
		float const fMinZoom = 1.0f/nSize;
		if (*a_pVal < fMinZoom)
		{
			*a_pVal = fMinZoom;
			return m_fZoom > fMinZoom ? S_OK : S_FALSE;
		}
		if (*a_pVal > 32.0f)
		{
			*a_pVal = 32.0f;
			return m_fZoom < 32.0f ? S_OK : S_FALSE;
		}
		return S_OK;
	}
	STDMETHOD(SetZoom)(float a_fVal)
	{
		// TODO: turn off auto-zoom in config?
		if (a_fVal > m_fZoom)
		{
			m_bAutoZoom = false;
			if (m_fZoom >= 32.0f)
				return S_FALSE;
			m_fZoom = a_fVal;
			if (m_fZoom >= 32.0f)
				m_fZoom = 32.0f;
			static_cast<T*>(this)->UpdateScalableContent();
			UpdateState();
			static_cast<T*>(this)->SendViewportUpdate();
			static_cast<T*>(this)->UpdateWindow();
			return S_OK;
		}
		else if (a_fVal < m_fZoom)
		{
			m_bAutoZoom = false;
			LONG nSize = min(m_tImageSize.cx, m_tImageSize.cy);
			if (nSize <= 0)
				return E_FAIL;
			if (m_fZoom <= 1.0f/nSize)
				return S_FALSE;
			m_fZoom = a_fVal;
			if (m_fZoom*nSize <= 1.0f)
				m_fZoom = 1.0f/nSize;
			static_cast<T*>(this)->UpdateScalableContent();
			UpdateState();
			static_cast<T*>(this)->SendViewportUpdate();
			static_cast<T*>(this)->UpdateWindow();
			return S_OK;
		}
		return S_OK;
	}
	STDMETHOD(GetZoom)(float* a_pVal)
	{
		try
		{
			*a_pVal = m_fZoom;
			return S_OK;
		}
		catch (...)
		{
			return a_pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SupportsAutoZoom)()
	{
		return S_OK;
	}
	STDMETHOD(IsAutoZoomEnabled)()
	{
		return m_bAutoZoom ? S_OK : S_FALSE;
	}
	STDMETHOD(EnableAutoZoom)(BOOL a_bEnable)
	{
		bool b = a_bEnable != 0;
		if (m_bAutoZoom != b)
		{
			m_bAutoZoom = b;
			if (m_bAutoZoom)
			{
				static_cast<T*>(this)->UpdateScalableContent();
				UpdateState();
				static_cast<T*>(this)->SendViewportUpdate();
				static_cast<T*>(this)->UpdateWindow();
			}
			CComBSTR cCFGID_IMGEDIT_STARTWITHAUTOZOOM(CFGID_IMGEDIT_STARTWITHAUTOZOOM);
			m_pConfig->ItemValuesSet(1, &(cCFGID_IMGEDIT_STARTWITHAUTOZOOM.m_str), CConfigValue(m_bAutoZoom));
		}
		return S_OK;
	}

	void ZoomIn(POINT const& a_tCenter)
	{
		// TODO: turn off auto-zoom in config?
		if (m_fZoom >= 32.0f)
			return;
		float fZoom = m_fZoom * 1.41421356f;
		if (fZoom >= 32.0f)
			fZoom = 32.0f;
		LONG fDX = a_tCenter.x-(m_tClientSize.cx>>1);
		LONG fDY = a_tCenter.y-(m_tClientSize.cy>>1);
		MoveViewport(
			static_cast<T*>(this)->M_OffsetX() + (fDX/m_fZoom-fDX/fZoom)/m_tImageSize.cx,
			static_cast<T*>(this)->M_OffsetY() +  (fDY/m_fZoom-fDY/fZoom)/m_tImageSize.cy,
			fZoom);
		static_cast<T*>(this)->UpdateWindow();
	}
	void ZoomOut(POINT const& a_tCenter)
	{
		// TODO: turn off auto-zoom in config?
		LONG nSize = min(m_tClientSize.cx, m_tClientSize.cy);
		if (nSize <= 0)
			return;
		if (m_fZoom*nSize <= 1.0f)
			return;
		float fZoom = m_fZoom*0.707106781f;
		float fMinZoom = 1.0f/nSize;
		if (fZoom <= fMinZoom)
			fZoom = fMinZoom;
		LONG fDX = a_tCenter.x-(m_tClientSize.cx>>1);
		LONG fDY = a_tCenter.y-(m_tClientSize.cy>>1);
		MoveViewport(
			static_cast<T*>(this)->M_OffsetX() + (fDX/m_fZoom-fDX/fZoom)/m_tImageSize.cx,
			static_cast<T*>(this)->M_OffsetY() +  (fDY/m_fZoom-fDY/fZoom)/m_tImageSize.cy,
			fZoom);
		static_cast<T*>(this)->UpdateWindow();
	}

	// IRasterEditView methods
public:
	STDMETHOD(GetGrid)(ULONG* a_pVal)
	{
		try
		{
			*a_pVal = m_eGridSetting;
			return S_OK;
		}
		catch (...)
		{
			return a_pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetGrid)(ULONG a_nVal)
	{
		if (m_eGridSetting != a_nVal)
		{
			m_eGridSetting = a_nVal;
			CComBSTR cCFGID_GRID(CFGID_IMGEDIT_GRID);
			m_pConfig->ItemValuesSet(1, &(cCFGID_GRID.m_str), CConfigValue(m_eGridSetting));
			static_cast<T*>(this)->InvalidateCanvas();
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(CanSetGrid)(ULONG a_nVal) { return S_OK; }
	STDMETHOD(CanSetStyle)(ULONG a_nVal) { return S_OK; }
	STDMETHOD(GetStyle)(ULONG* a_pVal)
	{
		if (a_pVal == NULL) return E_POINTER;
		*a_pVal = m_eStyle;
		return S_OK;
	}
	STDMETHOD(SetStyle)(ULONG a_nVal)
	{
		if (m_eStyle != a_nVal)
		{
			m_eStyle = a_nVal;
			UpdateMarginImgCache();
			CComBSTR cCFGID(CFGID_IMGEDIT_STYLE);
			m_pConfig->ItemValuesSet(1, &(cCFGID.m_str), CConfigValue(m_eStyle));
			static_cast<T*>(this)->Invalidate(FALSE);
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(CanApplyChanges)() { return E_NOTIMPL; }
	STDMETHOD(ApplyChanges)() { return E_NOTIMPL; }
	STDMETHOD(CanComposeView)() { return m_pComposedPreview ? S_OK : S_FALSE; }
	STDMETHOD(GetCompositionMode)(EImageCompositon* a_pMode)
	{
		*a_pMode = m_bShowComposed ? (m_eComposedMode == ECPMFinal ? EICFinalImage : EICEmphasizedLayer) : EICActiveLayer;
		return S_OK;
	}
	STDMETHOD(SetCompositionMode)(EImageCompositon a_eMode)
	{
		bool bNew = a_eMode != EICActiveLayer;
		EComposedPreviewMode eNew = a_eMode == EICActiveLayer ? m_eComposedMode : (a_eMode == EICEmphasizedLayer ? ECPMTransparent : ECPMFinal);
		if (m_bShowComposed != bNew || m_eComposedMode != eNew)
		{
			m_bShowComposed = bNew;
			m_eComposedMode = eNew;
			CComBSTR cCFGID_COMPOSED(CFGID_IMGEDIT_COMPOSED);
			m_pConfig->ItemValuesSet(1, &(cCFGID_COMPOSED.m_str), CConfigValue(LONG(a_eMode)));
			if (m_pComposedPreview)
			{
				RECT rc = { m_ptImagePos.x, m_ptImagePos.y, m_tZoomedSize.cx+m_ptImagePos.x, m_tZoomedSize.cy+m_ptImagePos.y };
				static_cast<T*>(this)->InvalidateCanvas();
			}
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(GetImageQuality)(EImageQuality* a_pQuality)
	{
		*a_pQuality = m_eImageQuality;
		return S_OK;
	}
	STDMETHOD(SetImageQuality)(EImageQuality a_eQuality)
	{
		if (m_eImageQuality != a_eQuality)
		{
			m_eImageQuality = a_eQuality;
			CComBSTR cCFGID_RENDERINGMODE(CFGID_IMGEDIT_RENDERINGMODE);
			m_pConfig->ItemValuesSet(1, &(cCFGID_RENDERINGMODE.m_str), CConfigValue(LONG(m_eImageQuality)));

			static_cast<T*>(this)->InvalidateCanvas();
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(GetInvertedPixels)(BYTE* a_pInverting)
	{
		*a_pInverting = m_bShowInverted;
		return S_OK;
	}
	STDMETHOD(SetInvertedPixels)(BYTE a_bInverting)
	{
		bool bNew = a_bInverting;
		if (m_bShowInverted != bNew)
		{
			m_bShowInverted = bNew;
			CComBSTR cCFGID_INVERTED(CFGID_IMGEDIT_INVERTED);
			m_pConfig->ItemValuesSet(1, &(cCFGID_INVERTED.m_str), CConfigValue(bNew));

			static_cast<T*>(this)->InvalidateCanvas();
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(ConfigureGestures)(RWHWND a_hParent, LCID a_tLocaleID) { return E_NOTIMPL; }
	STDMETHOD(CanSetHandleSize)() { return S_OK; }
	STDMETHOD(GetHandleSize)(float* a_pVal)
	{
		*a_pVal = m_fBaseHandleSize;
		return S_OK;
	}
	STDMETHOD(SetHandleSize)(float a_fVal)
	{
		if (m_fBaseHandleSize != a_fVal)
		{
			m_fBaseHandleSize = a_fVal;
			CComBSTR cCFGID(CFGID_IMGEDIT_HANDLESIZE);
			m_pConfig->ItemValuesSet(1, &(cCFGID.m_str), CConfigValue(m_fBaseHandleSize));

			static_cast<T*>(this)->BaseHandleSizeChanged();
			static_cast<T*>(this)->Invalidate(FALSE);
			//static_cast<T*>(this)->InvalidateCanvas();
			SendImageEditorState();
		}
		return S_OK;
	}
	STDMETHOD(CanHideHandles)() { return S_OK; }
	STDMETHOD(GetHideHandles)(BYTE* a_pAutoHide)
	{
		*a_pAutoHide = m_bHideOnLeave;
		return S_OK;
	}
	STDMETHOD(SetHideHandles)(BYTE a_bAutoHide)
	{
		bool bNew = a_bAutoHide;
		if (m_bHideOnLeave != bNew)
		{
			m_bHideOnLeave = bNew;
			CComBSTR cCFGID_HIDEHANDLES(CFGID_IMGEDIT_HIDEHANDLES);
			m_pConfig->ItemValuesSet(1, &(cCFGID_HIDEHANDLES.m_str), CConfigValue(bNew));

			static_cast<T*>(this)->InvalidateCanvas();
			SendImageEditorState();
		}
		return S_OK;
	}

	float M_BaseHandleSize() const { return m_fBaseHandleSize; }
	void BaseHandleSizeChanged()
	{
		InitPaintConstants();
		//Invalidate(FALSE);
	}

	// message handlers
public:
	LRESULT OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
#ifdef PERFMON
		LARGE_INTEGER cntBeg;
		QueryPerformanceCounter(&cntBeg);
#endif

		UpdateImageSize();
		UpdateImagePos();
		m_bUpdateWindow = false;

		CDCHandle cDC = reinterpret_cast<HDC>(a_wParam);
		PAINTSTRUCT ps;
		RECT rcDirty = {0, 0, m_tClientSize.cx, m_tClientSize.cy};
		if (a_wParam == NULL)
		{
			cDC = static_cast<T*>(this)->BeginPaint(&ps);
			rcDirty = ps.rcPaint;
		}
		CAutoVectorPtr<COLORREF> cBB;
		SIZE const szDirty = {rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top};
		int const nDirtyPixels = szDirty.cx*szDirty.cy;
		{
			if (nDirtyPixels == 0 || !cBB.Allocate(nDirtyPixels))
			{
				if (a_wParam == NULL)
					static_cast<T*>(this)->EndPaint(&ps);
				return 0;
			}
		}

		CComObjectStackEx<CRenderTask> cTask;

		// full image in client coordinates
		RECT const rcCanvas = {m_ptImagePos.x, m_ptImagePos.y, m_ptImagePos.x+m_tZoomedSize.cx, m_ptImagePos.y+m_tZoomedSize.cy};
		// part of the image that needs to be updated in client coordinates
		RECT const rcImageTile =
		{
			max(rcCanvas.left, rcDirty.left),
			max(rcCanvas.top, rcDirty.top),
			min(rcCanvas.right, rcDirty.right),
			min(rcCanvas.bottom, rcDirty.bottom),
		};
		CAutoVectorPtr<TRasterImagePixel> cSrcBuf;
		if (rcImageTile.left < rcImageTile.right && rcImageTile.top < rcImageTile.bottom)
		{
			SIZE const szImageTile = {rcImageTile.right-rcImageTile.left, rcImageTile.bottom-rcImageTile.top};

			float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
			float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
			RECT const rcTileInImg = {rcImageTile.left-m_ptImagePos.x, rcImageTile.top-m_ptImagePos.y, rcImageTile.right-m_ptImagePos.x, rcImageTile.bottom-m_ptImagePos.y};

			// rectangle in image coordinates that would be needed to update the window (if there are no effects)
			RECT const rcSourceTile = {rcTileInImg.left/fZoomX, rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(rcTileInImg.bottom/fZoomY)))};
			SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};
			// rectangle in image coordinates that would be needed to update the window (including effects)
			RECT rcAdjustedTile;
			// image to draw
			TRasterImagePixel* pSrcBuf;

			// get the source image
			CReadLock<IDocument> cLock(static_cast<T*>(this)->M_Doc());

			rcAdjustedTile = rcSourceTile;
			if (m_bShowComposed && m_pComposedPreview)
				m_pComposedPreview->PreProcessTile(&rcAdjustedTile);
			if (rcAdjustedTile.left > rcSourceTile.left) rcAdjustedTile.left = rcSourceTile.left;
			if (rcAdjustedTile.top > rcSourceTile.top) rcAdjustedTile.top = rcSourceTile.top;
			if (rcAdjustedTile.right < rcSourceTile.right) rcAdjustedTile.right = rcSourceTile.right;
			if (rcAdjustedTile.bottom < rcSourceTile.bottom) rcAdjustedTile.bottom = rcSourceTile.bottom;
			if (rcAdjustedTile.left < 0 && rcSourceTile.left >= 0) rcAdjustedTile.left = 0;
			if (rcAdjustedTile.top < 0 && rcSourceTile.top >= 0) rcAdjustedTile.top = 0;
			if (rcAdjustedTile.right > m_tImageSize.cx && rcSourceTile.right <= m_tImageSize.cx) rcAdjustedTile.right = m_tImageSize.cx;
			if (rcAdjustedTile.bottom > m_tImageSize.cy && rcSourceTile.bottom <= m_tImageSize.cy) rcAdjustedTile.bottom = m_tImageSize.cy;
			SIZE szAdjustedTile = {rcAdjustedTile.right-rcAdjustedTile.left, rcAdjustedTile.bottom-rcAdjustedTile.top};
			cSrcBuf.Attach(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
			pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcAdjustedTile.top)*szAdjustedTile.cx+rcSourceTile.left-rcAdjustedTile.left;
			static_cast<T*>(this)->GetImageTile(rcAdjustedTile.left, rcAdjustedTile.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);
			//TRasterImagePixel t = {255, 255, 255, 255};
			//std::fill_n(cSrcBuf.m_p, szAdjustedTile.cx*szAdjustedTile.cy, t);
			if (m_bShowComposed && m_pComposedPreview)
				m_pComposedPreview->ProcessTile(m_eComposedMode, rcAdjustedTile.left, rcAdjustedTile.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

			cTask.Init(this, rcDirty, cBB, szDirty.cx, rcCanvas, rcImageTile, rcSourceTile, rcAdjustedTile, cSrcBuf);
		}
		else
		{
			RECT const rcSourceTile = {0, 0, 0, 0};
			RECT const rcImageTile = {rcCanvas.left, rcCanvas.top, rcCanvas.left, rcCanvas.top};
			cTask.Init(this, rcDirty, cBB, szDirty.cx, rcCanvas, rcImageTile, rcSourceTile, rcSourceTile, cSrcBuf);
		}

#ifdef PERFMON
		LARGE_INTEGER cntMid1;
		QueryPerformanceCounter(&cntMid1);
#endif

		if (m_pThPool && (nDirtyPixels >= 32*32 || nDirtyPixels >= 32*32*m_fZoom*m_fZoom))
			m_pThPool->Execute(0, &cTask);
		else
			cTask.Execute(0, 1);

#ifdef PERFMON
		LARGE_INTEGER cntMid2;
		QueryPerformanceCounter(&cntMid2);
#endif

		//if ((m_eImageQuality&EIQMultithreaded) && ProcessorCount() > 1 && szDirty.cx >= 4 && szDirty.cy >= 4 && 
		//	(nDirtyPixels >= 128*128 || nDirtyPixels >= 128*128*m_fZoom*m_fZoom))
		//{
		//	CReadLock<IDocument> cLock(static_cast<T*>(this)->M_Doc());

		//	// TODO: keep and reuse the threads
		//	LONG nSplitY = (rcDirty.top+rcDirty.bottom+1)>>1;
		//	if (ProcessorCount() >= 4)
		//	{
		//		// TODO: support up to 32 cores

		//		// 4 threads
		//		LONG nSplitX = (rcDirty.left+rcDirty.right+1)>>1;
		//		SRenderThreadInfo sInfo1 = {this, rcDirty, cBB.m_p+nSplitX-rcDirty.left, szDirty.cx};
		//		SRenderThreadInfo sInfo2 = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top), szDirty.cx};
		//		SRenderThreadInfo sInfo3 = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top)+nSplitX-rcDirty.left, szDirty.cx};
		//		RECT rc2 = rcDirty;
		//		sInfo2.rcDirty.top = sInfo3.rcDirty.top = sInfo1.rcDirty.bottom = rc2.bottom = nSplitY;
		//		sInfo1.rcDirty.left = sInfo3.rcDirty.left = sInfo2.rcDirty.right = rc2.right = nSplitX;
		//		unsigned uThID;
		//		HANDLE hFinished[3] =
		//		{
		//			(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo1, 0, &uThID),
		//			(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo2, 0, &uThID),
		//			(HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo3, 0, &uThID),
		//		};
		//		try { RenderImage(rc2, cBB, szDirty.cx); } catch (...) {}
		//		WaitForMultipleObjects(3, hFinished, TRUE, INFINITE);
		//		CloseHandle(hFinished[0]);
		//		CloseHandle(hFinished[1]);
		//		CloseHandle(hFinished[2]);
		//	}
		//	else
		//	{
		//		// 2 threads
		//		SRenderThreadInfo sInfo = {this, rcDirty, cBB.m_p+szDirty.cx*(nSplitY-rcDirty.top), szDirty.cx};
		//		RECT rc2 = rcDirty;
		//		sInfo.rcDirty.top = rc2.bottom = nSplitY;
		//		unsigned uThID;
		//		HANDLE hFinished = (HANDLE)_beginthreadex(NULL, 0, RenderImageThreadProc, &sInfo, 0, &uThID);
		//		try { RenderImage(rc2, cBB, szDirty.cx); } catch (...) {}
		//		//AtlWaitWithMessageLoop(hFinished);
		//		WaitForSingleObject(hFinished, INFINITE);
		//		CloseHandle(hFinished);
		//	}
		//}
		//else
		//{
		//	// 1 thread
		//	try { RenderImage(rcDirty, cBB, szDirty.cx, rcCanvas, rcImageTile, rcSourceTile, rcAdjustedTile, pSrcBuf); } catch (...) {}
		//}

		RECT const rcImage = {m_ptImagePos.x, m_ptImagePos.y, m_ptImagePos.x+m_tZoomedSize.cx, m_ptImagePos.y+m_tZoomedSize.cy};
		static_cast<T*>(this)->PostRenderImage(rcImage, m_tClientSize.cx, m_tClientSize.cy, rcDirty, cBB, szDirty.cx);

		BITMAPINFO tBI;
		ZeroMemory(&tBI, sizeof tBI);
		tBI.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		tBI.bmiHeader.biWidth = rcDirty.right-rcDirty.left;
		tBI.bmiHeader.biHeight = rcDirty.top-rcDirty.bottom;
		tBI.bmiHeader.biPlanes = 1;
		tBI.bmiHeader.biBitCount = 32;
		tBI.bmiHeader.biCompression = BI_RGB;
		cDC.SetDIBitsToDevice(rcDirty.left, rcDirty.top, tBI.bmiHeader.biWidth, -tBI.bmiHeader.biHeight, 0, 0, 0, -tBI.bmiHeader.biHeight, cBB.m_p, &tBI, 0);

		if (a_wParam == NULL)
			static_cast<T*>(this)->EndPaint(&ps);

#ifdef PERFMON
		LARGE_INTEGER cntEnd;
		QueryPerformanceCounter(&cntEnd);
		//readPart = cntMid1.QuadPart-cntBeg.QuadPart;
		//parPart = cntMid2.QuadPart-cntMid1.QuadPart;
		if (m_pPerfMon)
		{
			m_pPerfMon->AddMeasurement(m_nCounterRedraw, 1000.0*(cntEnd.QuadPart-cntBeg.QuadPart)/freq);
			m_pPerfMon->AddMeasurement(m_nCounterObtain, 1000.0*(cntMid1.QuadPart-cntBeg.QuadPart)/freq);
		}
#endif
		return 0;
	}

	LRESULT OnSize(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(a_bHandled))
	{
		UpdateState();
		static_cast<T*>(this)->SendViewportUpdate();

		return 0;
	}
	LRESULT OnScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		LONG nZoomedSize = a_uMsg == WM_HSCROLL ? m_tZoomedSize.cx : m_tZoomedSize.cy;
		LONG nClientSize = a_uMsg == WM_HSCROLL ? m_tClientSize.cx : m_tClientSize.cy;
		float& fOffset = a_uMsg == WM_HSCROLL ? m_fOffsetX : m_fOffsetY;

		if (!(a_uMsg == WM_HSCROLL ? m_bScrollBarH : m_bScrollBarV))
			return 0;

		switch (LOWORD(a_wParam))
		{
		case SB_TOP:		// top or all left
			fOffset = -1.0f;
			break;
		case SB_BOTTOM:		// bottom or all right
			fOffset = 1.0f;
			break;
		case SB_LINEUP:		// line up or line left
			fOffset -= 16.0f/nZoomedSize;
			break;
		case SB_LINEDOWN:	// line down or line right
			fOffset += 16.0f/nZoomedSize;
			break;
		case SB_PAGEUP:		// page up or page left
			fOffset -= static_cast<float>(nClientSize)/nZoomedSize;
			break;
		case SB_PAGEDOWN:	// page down or page right
			fOffset += static_cast<float>(nClientSize)/nZoomedSize;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			{
				SCROLLINFO si;
				si.cbSize = sizeof(SCROLLINFO);
				si.fMask = SIF_TRACKPOS;
				if (static_cast<T*>(this)->GetScrollInfo(a_uMsg == WM_HSCROLL ? SB_HORZ : SB_VERT, &si))
				{
					float const fOffsetBound = 0.5f*static_cast<float>(nZoomedSize-nClientSize+static_cast<T*>(this)->CanvasPadding()*2)/static_cast<float>(nZoomedSize);
					fOffset = static_cast<float>(si.nTrackPos)*(fOffsetBound*2)/(nZoomedSize-1+(static_cast<T*>(this)->CanvasPadding()*2)-nClientSize)-fOffsetBound;
				}
			}
			break;
		case SB_ENDSCROLL:
		default:
			return 0;
		}

		UpdatePosition();
		static_cast<T*>(this)->SendViewportUpdate();
		static_cast<T*>(this)->UpdateWindow();
		return 0;
	}
	LRESULT OnEraseBackground(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		return 1;
	}
	LRESULT OnSettingChange(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& UNREF(bHandled))
	{
		UpdateMarginImgCache();
		static_cast<T*>(this)->Invalidate(FALSE);
		//UpdateSystemColors();
		return 0;
	}

	void InvalidateImageRectangle(RECT rcChanged)
	{
		if (m_pComposedPreview)
			m_pComposedPreview->AdjustDirtyRect(&rcChanged);
		RECT const rcSrc =
		{
			rcChanged.left > 0 ? rcChanged.left : 0,
			rcChanged.top > 0 ? rcChanged.top : 0,
			rcChanged.right < m_tImageSize.cx ? rcChanged.right : m_tImageSize.cx,
			rcChanged.bottom < m_tImageSize.cy ? rcChanged.bottom : m_tImageSize.cy,
		};
		if (rcSrc.left < rcSrc.right && rcSrc.top < rcSrc.bottom)
		{
			RECT rc =
			{
				float(rcSrc.left)*float(m_tZoomedSize.cx)/float(m_tImageSize.cx)+m_ptImagePos.x,
				float(rcSrc.top)*float(m_tZoomedSize.cy)/float(m_tImageSize.cy)+m_ptImagePos.y,
				float(rcSrc.right)*float(m_tZoomedSize.cx)/float(m_tImageSize.cx)+m_ptImagePos.x+0.9999f,
				float(rcSrc.bottom)*float(m_tZoomedSize.cy)/float(m_tImageSize.cy)+m_ptImagePos.y+0.9999f,
			};
			static_cast<T*>(this)->InvalidateRect(&rc, FALSE);
			m_bUpdateWindow = true;
		}
	}
	void InvalidateCanvas()
	{
		RECT rc = { m_ptImagePos.x, m_ptImagePos.y, m_tZoomedSize.cx+m_ptImagePos.x, m_tZoomedSize.cy+m_ptImagePos.y };
		static_cast<T*>(this)->InvalidateRect(&rc, FALSE);
		m_bUpdateWindow = true;
	}

private:
	class ATL_NO_VTABLE CRenderTask :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IThreadedTask
	{
	public:
		void Init(CRasterImagePainting<T>* a_pThis, RECT const a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride, RECT const a_rcCanvas, RECT const a_rcImageTile, RECT const a_rcSourceTile, RECT const a_rcAdjustedTile, TRasterImagePixel* a_pSrcBuf)
		{
			pThis = a_pThis;
			rcDirty = a_rcDirty;
			pBuffer = a_pBuffer;
			nStride = a_nStride;
			rcCanvas = a_rcCanvas;
			rcImageTile = a_rcImageTile;
			rcSourceTile = a_rcSourceTile;
			rcAdjustedTile = a_rcAdjustedTile;
			pSrcBuf = a_pSrcBuf;
		}

	BEGIN_COM_MAP(CRenderTask)
		COM_INTERFACE_ENTRY(IThreadedTask)
	END_COM_MAP()

		STDMETHOD(Execute)(ULONG a_nIndex, ULONG a_nTotal)
		{
			try
			{
				ULONG nY = rcDirty.bottom-rcDirty.top;
				if (a_nTotal > nY)
					a_nTotal = nY;
				if (a_nIndex >= nY)
					return S_OK;
				RECT rcSubDirty = rcDirty;
				rcSubDirty.top = rcDirty.top+nY*a_nIndex/a_nTotal;
				rcSubDirty.bottom = rcDirty.top+nY*(a_nIndex+1)/a_nTotal;
				RECT const rcImageTile =
				{
					max(rcCanvas.left, rcSubDirty.left),
					max(rcCanvas.top, rcSubDirty.top),
					min(rcCanvas.right, rcSubDirty.right),
					min(rcCanvas.bottom, rcSubDirty.bottom),
				};
				RECT const rcTileInImg = {rcImageTile.left-rcCanvas.left, rcImageTile.top-rcCanvas.top, rcImageTile.right-rcCanvas.left, rcImageTile.bottom-rcCanvas.top};
				float const fZoomX = float(pThis->m_tZoomedSize.cx)/float(pThis->m_tImageSize.cx);
				float const fZoomY = float(pThis->m_tZoomedSize.cy)/float(pThis->m_tImageSize.cy);
				RECT const rcSourceTile = {rcTileInImg.left/fZoomX, rcTileInImg.top/fZoomY, min(pThis->m_tImageSize.cx, LONG(ceilf(rcTileInImg.right/fZoomX))), min(pThis->m_tImageSize.cy, LONG(ceilf(rcTileInImg.bottom/fZoomY)))};
				SIZE szAdjustedTile = {rcAdjustedTile.right-rcAdjustedTile.left, rcAdjustedTile.bottom-rcAdjustedTile.top};
				TRasterImagePixel* pSrcBuf = this->pSrcBuf+(rcSourceTile.top-rcAdjustedTile.top)*szAdjustedTile.cx+rcSourceTile.left-rcAdjustedTile.left;
				pThis->RenderImage(rcSubDirty, pBuffer+nStride*(rcSubDirty.top-rcDirty.top), nStride, rcCanvas, rcImageTile, rcSourceTile, rcAdjustedTile, pSrcBuf);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CRasterImagePainting<T>* pThis;
		RECT rcDirty;
		COLORREF* pBuffer;
		ULONG nStride;
		RECT rcCanvas;
		RECT rcImageTile;
		RECT rcSourceTile;
		RECT rcAdjustedTile;
		TRasterImagePixel* pSrcBuf;
	};

	//struct SRenderThreadInfo
	//{
	//	CRasterImagePainting<T>* pThis;
	//	RECT rcDirty;
	//	COLORREF* pBuffer;
	//	ULONG nStride;
	//};

private:
	void UpdateImageSize()
	{
		m_tZoomedSize.cx = static_cast<LONG>(m_tImageSize.cx*m_fZoom + 0.5f);
		m_tZoomedSize.cy = static_cast<LONG>(m_tImageSize.cy*m_fZoom + 0.5f);
		m_tSizeWithBorder.cx = m_tZoomedSize.cx + MARGIN_LEFT+MARGIN_RIGHT;
		m_tSizeWithBorder.cy = m_tZoomedSize.cy + MARGIN_TOP+MARGIN_BOTTOM;
	}

	void UpdateImagePos()
	{
		m_ptImagePos.x = MARGIN_LEFT;
		if (m_bScrollBarH)
		{
			float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
			m_ptImagePos.x = MARGIN_LEFT-MARGIN_RIGHT-static_cast<int>((m_fOffsetX+fOffsetXBound)*m_tZoomedSize.cx+0.5f);
		}
		else
		{
			m_ptImagePos.x = MARGIN_LEFT-MARGIN_RIGHT+((m_tClientSize.cx-m_tZoomedSize.cx)>>1);
		}

		m_ptImagePos.y = MARGIN_TOP;
		if (m_bScrollBarV)
		{
			float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
			m_ptImagePos.y = MARGIN_TOP-MARGIN_BOTTOM-static_cast<int>((m_fOffsetY+fOffsetYBound)*m_tZoomedSize.cy+0.5f);
		}
		else
		{
			m_ptImagePos.y = MARGIN_TOP-MARGIN_BOTTOM+((m_tClientSize.cy-m_tZoomedSize.cy)>>1);
		}
	}

	void UpdateMarginImgCache()
	{
		static BYTE const MARGIN_IMG_LT[MARGIN_TOP*MARGIN_LEFT] =
		{
			4, 11, 22,
			11, 31, 87,
			22, 87, 209,
		};
		static BYTE const MARGIN_IMG_RT[MARGIN_TOP*MARGIN_RIGHT] =
		{
			22, 11, 4,
			87, 31, 11,
			209, 87, 22,
		};
		static BYTE const MARGIN_IMG_LB[MARGIN_BOTTOM*MARGIN_LEFT] =
		{
			22, 87, 209,
			11, 31, 87,
			4, 11, 22,
		};
		static BYTE const MARGIN_IMG_RB[MARGIN_BOTTOM*MARGIN_RIGHT] =
		{
			209, 87, 22,
			87, 31, 11,
			22, 11, 4,
		};
		static BYTE const MARGIN_IMG_L[MARGIN_LEFT] = { 30, 116, 255 };
		static BYTE const MARGIN_IMG_T[MARGIN_TOP] = { 30, 116, 255 };
		static BYTE const MARGIN_IMG_R[MARGIN_RIGHT] = { 255, 116, 30 };
		static BYTE const MARGIN_IMG_B[MARGIN_BOTTOM] = { 255, 116, 30 };
		struct SImgCache {BYTE const* pF; COLORREF* pT; size_t n;} const aImgs[] =
		{
			{ MARGIN_IMG_LT, MARGIN_CACHE_LT, MARGIN_TOP*MARGIN_LEFT },
			{ MARGIN_IMG_RT, MARGIN_CACHE_RT, MARGIN_TOP*MARGIN_RIGHT },
			{ MARGIN_IMG_LB, MARGIN_CACHE_LB, MARGIN_BOTTOM*MARGIN_LEFT },
			{ MARGIN_IMG_RB, MARGIN_CACHE_RB, MARGIN_BOTTOM*MARGIN_RIGHT },
			{ MARGIN_IMG_L, MARGIN_CACHE_L, MARGIN_LEFT },
			{ MARGIN_IMG_T, MARGIN_CACHE_T, MARGIN_TOP },
			{ MARGIN_IMG_R, MARGIN_CACHE_R, MARGIN_RIGHT },
			{ MARGIN_IMG_B, MARGIN_CACHE_B, MARGIN_BOTTOM },
		};
		switch (m_eStyle)
		{
		case CFGVAL_2DEDIT_STYLECLASSIC:
			m_tBackground = GetSysColor(COLOR_APPWORKSPACE);
			m_tSelection = RGB(128, 128, 128);//GetSysColor(COLOR_MENUHILIGHT);
			m_tSquare1 = GetSysColor(COLOR_3DLIGHT);//RGB(255,0,0);
			m_tSquare2 = GetSysColor(COLOR_3DSHADOW);//RGB(0,255,0);
			m_tGrid = 0xff0a0a0a;
			for (SImgCache const* p = aImgs; p != aImgs+sizeof(aImgs)/sizeof(aImgs[0]); ++p)
				for (ULONG i = 0; i < p->n; ++i)
					p->pT[i] = RGB(GetRValue(m_tBackground)*(255-p->pF[i])/255, GetGValue(m_tBackground)*(255-p->pF[i])/255, GetBValue(m_tBackground)*(255-p->pF[i])/255);
			break;
		case CFGVAL_2DEDIT_STYLECLEAN:
			m_tBackground = 0xffffff;
			m_tSelection = RGB(128, 128, 128);//GetSysColor(COLOR_MENUHILIGHT);
			m_tSquare1 = 0xffffff;
			m_tSquare2 = 0xffffff;
			for (SImgCache const* p = aImgs; p != aImgs+sizeof(aImgs)/sizeof(aImgs[0]); ++p)
				for (ULONG i = 0; i < p->n; ++i)
					p->pT[i] = 0xffffff;
			MARGIN_CACHE_L[1] = MARGIN_CACHE_R[1] = MARGIN_CACHE_T[1] = MARGIN_CACHE_B[1] =
			MARGIN_CACHE_LT[4] = MARGIN_CACHE_LT[5] = MARGIN_CACHE_LT[7] =
			MARGIN_CACHE_RT[3] = MARGIN_CACHE_RT[4] = MARGIN_CACHE_RT[7] =
			MARGIN_CACHE_LB[4] = MARGIN_CACHE_LB[5] = MARGIN_CACHE_LB[1] =
			MARGIN_CACHE_RB[3] = MARGIN_CACHE_RB[4] = MARGIN_CACHE_RB[1] =
			0xdddddd;
			m_tGrid = 0xff0a0a0a;
			break;
		case CFGVAL_2DEDIT_STYLESUBTLE:
		{
			m_tBackground = GetSysColor(COLOR_WINDOW);
			COLORREF fore = GetSysColor(COLOR_WINDOWTEXT);
			float br = CGammaTables::FromSRGB(GetRValue(m_tBackground));
			float bg = CGammaTables::FromSRGB(GetGValue(m_tBackground));
			float bb = CGammaTables::FromSRGB(GetBValue(m_tBackground));
			float fr = CGammaTables::FromSRGB(GetRValue(fore));
			float fg = CGammaTables::FromSRGB(GetGValue(fore));
			float fb = CGammaTables::FromSRGB(GetBValue(fore));
			m_tSelection = RGB(128, 128, 128);//GetSysColor(COLOR_MENUHILIGHT);
			m_tSquare1 = RGB(CGammaTables::ToSRGB(br*0.9f+fr*0.1f), CGammaTables::ToSRGB(bg*0.9f+fg*0.1f), CGammaTables::ToSRGB(bb*0.9f+fb*0.1f));
			m_tSquare2 = RGB(CGammaTables::ToSRGB(br*0.8f+fr*0.2f), CGammaTables::ToSRGB(bg*0.8f+fg*0.2f), CGammaTables::ToSRGB(bb*0.8f+fb*0.2f));
			for (SImgCache const* p = aImgs; p != aImgs+sizeof(aImgs)/sizeof(aImgs[0]); ++p)
			{
				for (ULONG i = 0; i < p->n; ++i)
				{
					float f2 = p->pF[i]/300.0f;
					float f1 = 1.0f-f2;
					p->pT[i] = RGB(CGammaTables::ToSRGB(br*f1+fr*f2), CGammaTables::ToSRGB(bg*f1+fg*f2), CGammaTables::ToSRGB(bb*f1+fb*f2));
				}
			}
			MARGIN_CACHE_L[2] = MARGIN_CACHE_R[0] = MARGIN_CACHE_T[2] = MARGIN_CACHE_B[0] =
			MARGIN_CACHE_LT[8] = MARGIN_CACHE_RT[6] = MARGIN_CACHE_LB[2] = MARGIN_CACHE_RB[0] = m_tBackground;
			m_tGrid = 0x5f000000|fore;//0xff000000|RGB(CGammaTables::ToSRGB(br*0.6f+fr*0.4f), CGammaTables::ToSRGB(bg*0.6f+fg*0.4f), CGammaTables::ToSRGB(bb*0.6f+fb*0.4f));
		}
			break;
		}
	}

	void UpdateScalableContent() {}
	void UpdateState()
	{
		if (!static_cast<T*>(this)->m_hWnd || m_bUpdating)
			return;

		static_cast<T*>(this)->SetRedraw(FALSE);

		m_bUpdating = true; // prevent recursion due to WM_SIZE caused by ShowScrollBar

		RECT rcClient;
		//ShowScrollBar(SB_HORZ, FALSE);
		//ShowScrollBar(SB_VERT, FALSE);
		//GetClientRect(&rcClient);
		static_cast<T*>(this)->GetWindowRect(&rcClient);
		static_cast<T*>(this)->ScreenToClient(&rcClient);

		if (static_cast<T*>(this)->GetExStyle()&WS_EX_CLIENTEDGE)
		{
			int const cxEdge = GetSystemMetrics(SM_CXEDGE);
			int const cyEdge = GetSystemMetrics(SM_CYEDGE);
			rcClient.left += cxEdge;
			rcClient.right -= cxEdge;
			if (rcClient.right < rcClient.left) rcClient.right = rcClient.left;
			rcClient.top += cyEdge;
			rcClient.bottom -= cyEdge;
			if (rcClient.bottom < rcClient.top) rcClient.bottom = rcClient.top;
		}

		if (m_bAutoZoom)
		{
			static_cast<T*>(this)->ShowScrollBar(SB_HORZ, FALSE);
			static_cast<T*>(this)->ShowScrollBar(SB_VERT, FALSE);
			m_bScrollBarH = m_bScrollBarV = false;
			UpdateImageSize();
			int nX = rcClient.right - (MARGIN_LEFT+MARGIN_RIGHT) - 4;
			int nY = rcClient.bottom - (MARGIN_TOP+MARGIN_BOTTOM) - 4;
			if (nX <= 0) nX = 1;
			if (nY <= 0) nY = 1;
			float fZoomX = nX/float(m_tImageSize.cx);
			float fZoomY = nY/float(m_tImageSize.cy);
			m_fZoom = min(fZoomX, fZoomY);
			if (m_fZoom > 32.0f)
				m_fZoom = 32.0f;
			static_cast<T*>(this)->UpdateScalableContent();
		}

		UpdateImageSize();

		if (rcClient.right < m_tZoomedSize.cx || rcClient.bottom < m_tZoomedSize.cy)
		{
			int const pad = static_cast<T*>(this)->CanvasPadding()<<1;
			if (rcClient.right < m_tZoomedSize.cx+pad)
			{
				static_cast<T*>(this)->ShowScrollBar(SB_HORZ, TRUE);
				static_cast<T*>(this)->GetClientRect(&rcClient);
				m_bScrollBarH = true;
			}
			else
			{
				static_cast<T*>(this)->ShowScrollBar(SB_HORZ, FALSE);
				m_bScrollBarH = false;
			}
			if (rcClient.bottom < m_tZoomedSize.cy+pad)
			{
				static_cast<T*>(this)->ShowScrollBar(SB_VERT, TRUE);
				static_cast<T*>(this)->GetClientRect(&rcClient);
				m_bScrollBarV = true;
			}
			else
			{
				static_cast<T*>(this)->ShowScrollBar(SB_VERT, FALSE);
				m_bScrollBarV = false;
			}
			if (rcClient.right < m_tZoomedSize.cx+pad) // horizontal size must be re-checked again
			{
				static_cast<T*>(this)->ShowScrollBar(SB_HORZ, TRUE);
				static_cast<T*>(this)->GetClientRect(&rcClient);
				m_bScrollBarH = true;
			}
		}
		else
		{
			static_cast<T*>(this)->ShowScrollBar(SB_HORZ, FALSE);
			static_cast<T*>(this)->ShowScrollBar(SB_VERT, FALSE);
			m_bScrollBarH = m_bScrollBarV = false;
		}

		m_tClientSize.cx = rcClient.right;
		m_tClientSize.cy = rcClient.bottom;

		SCROLLINFO si;
		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = m_tZoomedSize.cx - 1 + (m_bScrollBarH ? static_cast<T*>(this)->CanvasPadding()*2 : 0);
		si.nPage = m_tClientSize.cx;
		si.nPos = 0;
		static_cast<T*>(this)->SetScrollInfo(SB_HORZ, &si, FALSE);
		si.nMax = m_tZoomedSize.cy - 1 + (m_bScrollBarV ? static_cast<T*>(this)->CanvasPadding()*2 : 0);
		si.nPage = m_tClientSize.cy;
		si.nPos = 0;
		static_cast<T*>(this)->SetScrollInfo(SB_VERT, &si, FALSE);
		UpdatePosition();

		m_bUpdating = false;

		static_cast<T*>(this)->SetRedraw();

		static_cast<T*>(this)->Invalidate(FALSE);
		m_bUpdateWindow = true;
	}

	void UpdatePosition()
	{
		if (m_bScrollBarH)
		{
			float fOffsetXBound = 0.5f*static_cast<float>(m_tZoomedSize.cx+static_cast<T*>(this)->CanvasPadding()*2-m_tClientSize.cx)/static_cast<float>(m_tZoomedSize.cx);
			if (m_fOffsetX < -fOffsetXBound)
				m_fOffsetX = -fOffsetXBound;
			else
			if (m_fOffsetX > fOffsetXBound)
				m_fOffsetX = fOffsetXBound;
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = static_cast<int>((m_fOffsetX+fOffsetXBound)/(fOffsetXBound*2)*(m_tZoomedSize.cx-1+(static_cast<T*>(this)->CanvasPadding()*2)-m_tClientSize.cx)+0.5f);
			static_cast<T*>(this)->SetScrollInfo(SB_HORZ, &si, TRUE);
		}
		else
		{
			m_fOffsetX = 0.0f;
		}

		if (m_bScrollBarV)
		{
			float fOffsetYBound = 0.5f*static_cast<float>(m_tZoomedSize.cy+static_cast<T*>(this)->CanvasPadding()*2-m_tClientSize.cy)/static_cast<float>(m_tZoomedSize.cy);
			if (m_fOffsetY < -fOffsetYBound)
				m_fOffsetY = -fOffsetYBound;
			else
			if (m_fOffsetY > fOffsetYBound)
				m_fOffsetY = fOffsetYBound;
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = static_cast<int>((m_fOffsetY+fOffsetYBound)/(fOffsetYBound*2)*(m_tZoomedSize.cy-1+(static_cast<T*>(this)->CanvasPadding()*2)-m_tClientSize.cy)+0.5f);
			static_cast<T*>(this)->SetScrollInfo(SB_VERT, &si, TRUE);
		}
		else
		{
			m_fOffsetY = 0.0f;
		}

		if (!m_bUpdating)
		{
			static_cast<T*>(this)->Invalidate(FALSE);
			m_bUpdateWindow = true;
		}
	}

	void RenderImage(RECT const a_rcDirty, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcCanvas, RECT const rcImageTile, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel* pSrcBuf)
	{
		float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
		float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
		SIZE const szDirty = {a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top};

		int const nX1 = max(rcCanvas.left-a_rcDirty.left, 0);
		int const nY1 = max(rcCanvas.top-a_rcDirty.top, 0);

		// draw empty space and image border
		{
			int const nX2 = max(a_rcDirty.right-rcCanvas.right, 0);
			int const nY2 = max(a_rcDirty.bottom-rcCanvas.bottom, 0);
			int nYleft = szDirty.cy;
			COLORREF* p = a_pBuffer;
			if (nY1 > 0)
			{
				if (nY1 > MARGIN_TOP)
				{
					int const nL = min(szDirty.cy, nY1-MARGIN_TOP);
					for (int i = 0; i < nL; ++i)
					{
						std::fill_n(p, szDirty.cx, M_Background());
						p += a_nStride;
					}
					nYleft -= nL;
				}
				int const y2 = min(MARGIN_TOP, nYleft);
				for (int y = nY1 < MARGIN_TOP ? MARGIN_TOP-nY1 : 0; y < y2; ++y)
				{
					int nXleft = szDirty.cx;
					if (nX1 > 0)
					{
						if (nX1 > MARGIN_LEFT)
						{
							int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
							std::fill_n(p, nL, M_Background());
							p += nL;
							nXleft -= nL;
						}
						int const x2 = min(MARGIN_LEFT, nXleft);
						for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_LT[x+MARGIN_LEFT*y];
							--nXleft;
						}
					}
					if (rcImageTile.right > rcImageTile.left)
					{
						std::fill_n(p, rcImageTile.right-rcImageTile.left, MARGIN_CACHE_T[y]);
						p += rcImageTile.right-rcImageTile.left;
						nXleft -= rcImageTile.right-rcImageTile.left;
					}
					if (nX2 > 0)
					{
						int const x2 = min(MARGIN_RIGHT, nX2);
						for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_RT[x+MARGIN_RIGHT*y];
							--nXleft;
						}
						if (nX2 > MARGIN_RIGHT && nXleft)
						{
							int const nL = min(nX2-MARGIN_RIGHT, nXleft);
							std::fill_n(p, nL, M_Background());
							p += nL;
							ATLASSERT(nL == nXleft);
						}
					}
					p += a_nStride-szDirty.cx;
					--nYleft;
				}
			}
			if (rcImageTile.bottom > rcImageTile.top)
			{
				for (LONG i = rcImageTile.top; i < rcImageTile.bottom; ++i)
				{
					int nXleft = szDirty.cx;
					if (nX1 > 0)
					{
						if (nX1 > MARGIN_LEFT)
						{
							int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
							std::fill_n(p, nL, M_Background());
							p += nL;
							nXleft -= nL;
						}
						int const x2 = min(MARGIN_LEFT, nXleft);
						for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_L[x];
							--nXleft;
						}
					}
					if (rcImageTile.right > rcImageTile.left)
					{
						p += rcImageTile.right-rcImageTile.left;
						nXleft -= rcImageTile.right-rcImageTile.left;
					}
					if (nX2 > 0)
					{
						int const x2 = min(MARGIN_RIGHT, nX2);
						for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_R[x];
							--nXleft;
						}
						if (nX2 > MARGIN_RIGHT && nXleft)
						{
							int const nL = min(nX2-MARGIN_RIGHT, nXleft);
							std::fill_n(p, nL, M_Background());
							p += nL;
							nXleft += nL;
						}
					}
					p += a_nStride-szDirty.cx;
				}
				nYleft -= rcImageTile.bottom-rcImageTile.top;
			}
			if (nY2 > 0)
			{
				int const y2 = min(MARGIN_BOTTOM, nY2);
				for (int y = max(nY2-szDirty.cy, 0); y < y2; ++y)
				{
					int nXleft = szDirty.cx;
					if (nX1 > 0)
					{
						if (nX1 > MARGIN_LEFT)
						{
							int const nL = min(szDirty.cx, nX1-MARGIN_LEFT);
							std::fill_n(p, nL, M_Background());
							p += nL;
							nXleft -= nL;
						}
						int const x2 = min(MARGIN_LEFT, nXleft);
						for (int x = max(MARGIN_LEFT-nX1, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_LB[x+MARGIN_LEFT*y];
							--nXleft;
						}
					}
					if (rcImageTile.right > rcImageTile.left)
					{
						std::fill_n(p, rcImageTile.right-rcImageTile.left, MARGIN_CACHE_B[y]);
						p += rcImageTile.right-rcImageTile.left;
						nXleft -= rcImageTile.right-rcImageTile.left;
					}
					if (nX2 > 0)
					{
						int const x2 = min(MARGIN_RIGHT, nX2);
						for (int x = max(nX2-szDirty.cx, 0); x < x2; ++x)
						{
							*(p++) = MARGIN_CACHE_RB[x+MARGIN_RIGHT*y];
							--nXleft;
						}
						if (nXleft)
						{
							std::fill_n(p, nXleft, M_Background());
							p += nXleft;
						}
					}
					p += a_nStride-szDirty.cx;
					--nYleft;
				}
				if (nYleft)
				{
					for (int i = 0; i < nYleft; ++i)
					{
						std::fill_n(p, szDirty.cx, M_Background());
						p += a_nStride;
					}
				}
			}
		}

		bool bUsingNearest = false;
		if (rcImageTile.right > rcImageTile.left && rcImageTile.bottom > rcImageTile.top)
		{
			if (m_bShowInverted)
				RenderTileInvert(rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top, pSrcBuf, rcPP.right-rcPP.left);

			//ULONG nImageTilePixels = (rcImageTile.right-rcImageTile.left)*(rcImageTile.bottom-rcImageTile.top);
			RECT const rcTileInImg = {rcImageTile.left-m_ptImagePos.x, rcImageTile.top-m_ptImagePos.y, rcImageTile.right-m_ptImagePos.x, rcImageTile.bottom-m_ptImagePos.y};

			if ((m_eImageQuality&EIQMethodMask) == EIQNearest || // selected method
				(m_tImageSize.cx <= 2 || m_tImageSize.cy <= 2) || // small images not supported by interpolation methods
				(m_tZoomedSize.cx*128 <= m_tImageSize.cx || m_tZoomedSize.cy*128 <= m_tImageSize.cy)) // zoom too small ... it would overflow
			{
				RenderTileNearest(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
				bUsingNearest = true;
			}
			else if (m_tZoomedSize.cx == m_tImageSize.cx && m_tZoomedSize.cy == m_tImageSize.cy)
			{
				RenderTile100Percent(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
			}
			else if (m_tZoomedSize.cx >= m_tImageSize.cx && m_tZoomedSize.cy >= m_tImageSize.cy)
			{
				RenderTileZoomIn(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
			}
			else if (m_tZoomedSize.cx >= (m_tImageSize.cx>>1) && m_tZoomedSize.cy >= (m_tImageSize.cy>>1))
			{
				RenderTileZoomOutSimple(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
			}
			else
			{
				if ((m_eImageQuality&EIQMethodMask) == EIQCoverage)
				{
					RenderTileZoomOutCoverage(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
				}
				else
				{
					RenderTileNearest(rcTileInImg, a_pBuffer + a_nStride*nY1+nX1, a_nStride, rcSourceTile, rcPP, pSrcBuf);
					bUsingNearest = true;
				}
			}

			// draw grid
			if (m_eGridSetting)
			{
				LONG nStep = m_eGridSetting;
				while (nStep*m_fZoom < 16 || nStep*m_fZoom < 8*m_eGridSetting)
					nStep += nStep;

				agg::rendering_buffer rbuf;
				rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer + a_nStride*nY1+nX1), rcTileInImg.right-rcTileInImg.left, rcTileInImg.bottom-rcTileInImg.top, a_nStride*sizeof*a_pBuffer);
				agg::pixfmt_bgra32 pixf(rbuf);
				agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
				agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
				agg::rasterizer_scanline_aa<> ras;
				agg::scanline_p8 sl;

				float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
				float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
				float fX1 = (rcImageTile.left-m_ptImagePos.x)/fZoomX-nStep*0.5f;
				float fY1 = (rcImageTile.top-m_ptImagePos.y)/fZoomY-nStep*0.5f;
				float fX2 = (rcImageTile.right-m_ptImagePos.x)/fZoomX+nStep*0.5f;
				float fY2 = (rcImageTile.bottom-m_ptImagePos.y)/fZoomY+nStep*0.5f;
				int nX1 = nStep*int(fX1/nStep);
				int nY1 = nStep*int(fY1/nStep);
				int nX2 = nStep*int(fX2/nStep);
				int nY2 = nStep*int(fY2/nStep);

				double f1 = 0.6;
				double f2 = nStep*0.2*m_fZoom;
				for (int y = nY1; y <= nY2; y += nStep)
				{
					double const fY = bUsingNearest ? int(y*fZoomY-rcTileInImg.top+0.5f) : (y*fZoomY-rcTileInImg.top);
					for (int x = nX1; x <= nX2; x += nStep)
					{
						double const fX = bUsingNearest ? int(x*fZoomX-rcTileInImg.left+0.5f) : (x*fZoomX-rcTileInImg.left);
						if (m_eStyle != CFGVAL_2DEDIT_STYLECLEAN)
						{
							ras.move_to_d(fX-f1, fY-f2);
							ras.line_to_d(fX+f1, fY-f2);
							ras.line_to_d(fX+f1, fY-f1);
							ras.line_to_d(fX+f2, fY-f1);
							ras.line_to_d(fX+f2, fY+f1);
							ras.line_to_d(fX+f1, fY+f1);
							ras.line_to_d(fX+f1, fY+f2);
							ras.line_to_d(fX-f1, fY+f2);
							ras.line_to_d(fX-f1, fY+f1);
							ras.line_to_d(fX-f2, fY+f1);
							ras.line_to_d(fX-f2, fY-f1);
							ras.line_to_d(fX-f1, fY-f1);
							ras.close_polygon();
						}
						else
						{
							float size = m_eGridSetting == 1 ? 1.5f : 2.0f;
							ras.move_to_d(fX-f1*size, fY);
							ras.line_to_d(fX, fY-f1*size);
							ras.line_to_d(fX+f1*size, fY);
							ras.line_to_d(fX, fY+f1*size);
							ras.close_polygon();
						}
					}
				}
				ren.color(agg::rgba8(GetRValue(m_tGrid), GetGValue(m_tGrid), GetBValue(m_tGrid), m_tGrid>>24));
				agg::render_scanlines(ras, sl, ren);
			}

		}

		//// draw help lines
		//float fXMin = fZoomX*m_fHelpLinesXMin+m_ptImagePos.x;
		//float fYMin = fZoomY*m_fHelpLinesYMin+m_ptImagePos.y;
		//float fXMax = fZoomX*m_fHelpLinesXMax+m_ptImagePos.x;
		//float fYMax = fZoomY*m_fHelpLinesYMax+m_ptImagePos.y;
		//LONG nXMin = fXMin-2;
		//LONG nYMin = fYMin-2;
		//LONG nXMax = fXMax+2;
		//LONG nYMax = fYMax+2;
		//if (fXMax >= fXMin && fYMax >= fYMin &&
		//	a_rcDirty.left < nXMax && a_rcDirty.top < nYMax &&
		//	a_rcDirty.right > nXMin && a_rcDirty.bottom > nYMin)
		//{
		//	agg::rendering_buffer rbuf;
		//	rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, a_nStride*sizeof*a_pBuffer);
		//	agg::pixfmt_bgra32 pixf(rbuf);
		//	agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		//	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		//	agg::rasterizer_scanline_aa<> ras;
		//	agg::scanline_p8 sl;

	 //       agg::path_storage path;
		//	for (CHelpLines::const_iterator i = m_cHelpLines.begin(); i != m_cHelpLines.end(); ++i)
		//	{
		//		if (i->bLineTo)
		//		{
		//			path.line_to(fZoomX*i->fX+m_ptImagePos.x-a_rcDirty.left, fZoomY*i->fY+m_ptImagePos.y-a_rcDirty.top);
		//			if (i->bClose)
		//				path.close_polygon();
		//		}
		//		else
		//		{
		//			path.move_to(fZoomX*i->fX+m_ptImagePos.x-a_rcDirty.left, fZoomY*i->fY+m_ptImagePos.y-a_rcDirty.top);
		//		}
		//	}
		//	agg::conv_dash<agg::path_storage> dash(path);
		//	dash.add_dash(5, 5);
		//	agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke(dash);
		//	stroke.line_join(agg::bevel_join);
		//	stroke.line_cap(agg::butt_cap);
		//	stroke.width(1.0f);
		//	ren.color(agg::rgba8(255, 255, 255, 255));
		//	ras.add_path(stroke);
		//	agg::render_scanlines(ras, sl, ren);

		//	agg::conv_dash<agg::path_storage> dash2(path);
		//	dash2.add_dash(5, 5);
		//	dash2.dash_start(5);
		//	agg::conv_stroke<agg::conv_dash<agg::path_storage> > stroke2(dash2);
		//	stroke2.line_join(agg::bevel_join);
		//	stroke2.line_cap(agg::butt_cap);
		//	stroke2.width(1.0f);
		//	ren.color(agg::rgba8(0, 0, 0, 255));
		//	ras.add_path(stroke2);
		//	agg::render_scanlines(ras, sl, ren);
		//}

		//// draw control handles
		//{
		//	for (CHandles::const_iterator i = m_cHandles.begin(); i != m_cHandles.end(); ++i)
		//	{
		//		TPixelCoords tPos = i->first;
		//		ULONG nClass = i->second;
		//		LONG nX = fZoomX*tPos.fX+m_ptImagePos.x;
		//		LONG nY = fZoomY*tPos.fY+m_ptImagePos.y;
		//		RECT rcPt = {nX-m_nHandleRadius, nY-m_nHandleRadius, nX+m_nHandleRadius+1, nY+m_nHandleRadius+1};
		//		RECT rcIntersection =
		//		{
		//			max(rcPt.left, a_rcDirty.left),
		//			max(rcPt.top, a_rcDirty.top),
		//			min(rcPt.right, a_rcDirty.right),
		//			min(rcPt.bottom, a_rcDirty.bottom),
		//		};
		//		if (rcIntersection.left >= rcIntersection.right || rcIntersection.top >= rcIntersection.bottom)
		//			continue;
		//		float const l = 0.8f;
		//		float const s = 1.0f;
		//		float const m2 = l + (l <= 0.5f ? l*s : s - l*s);
		//		float const m1 = 2.0f * l - m2;
		//		double hue = double(nClass)*360.0f/4.95f+200.0f;
		//		hue = 360.0-(hue-360.0f*floor(hue/360.0f));
		//		float r = hls_value(m1, m2, hue+120.0f);
		//		float g = hls_value(m1, m2, hue);
		//		float b = hls_value(m1, m2, hue-120.0f);
		//		if (r > 1.0f) r = 1.0f;
		//		if (g > 1.0f) g = 1.0f;
		//		if (b > 1.0f) b = 1.0f;
		//		if (r < 0.0f) r = 0.0f;
		//		if (g < 0.0f) g = 0.0f;
		//		if (b < 0.0f) b = 0.0f;
		//		BYTE bClassR = r*255.0f+0.5f;
		//		BYTE bClassG = g*255.0f+0.5f;
		//		BYTE bClassB = b*255.0f+0.5f;
		//		for (LONG y = rcIntersection.top; y < rcIntersection.bottom; ++y)
		//		{
		//			BYTE* pO = reinterpret_cast<BYTE*>(a_pBuffer + (y-a_rcDirty.top)*a_nStride + rcIntersection.left-a_rcDirty.left);
		//			BYTE const* pI = m_pHandleImage + (((y-rcPt.top)*(m_nHandleRadius+m_nHandleRadius+1) + rcIntersection.left-rcPt.left)<<1);
		//			for (LONG x = rcIntersection.left; x < rcIntersection.right; ++x)
		//			{
		//				if (pI[0])
		//				{
		//					if (pI[0] == 255)
		//					{
		//						pO[0] = (ULONG(pI[1])*bClassB)/255;
		//						pO[1] = (ULONG(pI[1])*bClassG)/255;
		//						pO[2] = (ULONG(pI[1])*bClassR)/255;
		//					}
		//					else
		//					{
		//						ULONG clr = ULONG(pI[1])*pI[0];
		//						ULONG inv = (255-pI[0])*255;
		//						pO[0] = (pO[0]*inv + clr*bClassB)/(255*255);
		//						pO[1] = (pO[1]*inv + clr*bClassG)/(255*255);
		//						pO[2] = (pO[2]*inv + clr*bClassR)/(255*255);
		//					}
		//				}
		//				pO += 4;
		//				pI += 2;
		//			}
		//		}
		//	}
		//}

		//// draw mouse gesture trail
		//if (m_eDragState == EDSGesture &&
		//	m_nGestureXMin <= m_nGestureXMax && m_nGestureYMin <= m_nGestureYMax)
		//{
		//	agg::rendering_buffer rbuf;
		//	rbuf.attach(reinterpret_cast<agg::int8u*>(a_pBuffer), a_rcDirty.right-a_rcDirty.left, a_rcDirty.bottom-a_rcDirty.top, (a_rcDirty.right-a_rcDirty.left)*sizeof*a_pBuffer);
		//	agg::pixfmt_bgra32 pixf(rbuf);
		//	agg::renderer_base<agg::pixfmt_bgra32> renb(pixf);
		//	agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_bgra32> > ren(renb);
		//	agg::rasterizer_scanline_aa<> ras;
		//	agg::scanline_p8 sl;

		//	agg::path_storage path;
		//	path.move_to(m_cGesturePoints.begin()->x-a_rcDirty.left+0.5, m_cGesturePoints.begin()->y-a_rcDirty.top+0.5);
		//	for (CGesturePoints::const_iterator i = m_cGesturePoints.begin()+1; i != m_cGesturePoints.end(); ++i)
		//	{
		//		path.line_to(i->x-a_rcDirty.left+0.5, i->y-a_rcDirty.top+0.5);
		//	}
		//	agg::conv_stroke<agg::path_storage> stroke(path);
		//	stroke.line_join(agg::bevel_join);
		//	stroke.width(3.0f);
		//	ren.color(agg::rgba8(255, 50, 50, 160));
		//	ras.add_path(stroke);
		//	agg::render_scanlines(ras, sl, ren);
		//}
	}

	void GetSelInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle) {}
	bool GetSelTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer) { return false; }

	void RenderTile100Percent(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel const* pSrcBuf)
	{
		SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};

		//RECT rcPP = a_rcTileInImg;
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->PreProcessTile(&rcPP);
		//TRasterImagePixel* pSrc;
		ULONG nSrcStride;
		//CAutoVectorPtr<TRasterImagePixel> cPPBuffer;
		//if (rcPP.left != a_rcTileInImg.left || rcPP.right != a_rcTileInImg.right || 
		//	rcPP.top != a_rcTileInImg.top || rcPP.bottom != a_rcTileInImg.bottom)
		{
			//cPPBuffer.Allocate((rcPP.right-rcPP.left)*(rcPP.bottom-rcPP.top));
			nSrcStride = rcPP.right-rcPP.left;
			//pSrc = cPPBuffer.m_p+(a_rcTileInImg.top-rcPP.top)*nSrcStride+a_rcTileInImg.left-rcPP.left;
			//static_cast<T*>(this)->GetImageTile(rcPP.left, rcPP.top, rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, nSrcStride, cPPBuffer);
			//if (m_bShowComposed && m_pComposedPreview)
			//	m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, nSrcStride, cPPBuffer);
			//if (m_bShowInverted)
			//	RenderTileInvert(rcPP.right-rcPP.left, rcPP.bottom-rcPP.top, cPPBuffer, rcPP.right-rcPP.left);
		}
		//else
		//{
		//	static_cast<T*>(this)->GetImageTile(a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, a_nStride, reinterpret_cast<TRasterImagePixel*>(a_pBuffer));
		//	pSrc = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		//	nSrcStride = a_nStride;
		//	if (m_bShowComposed && m_pComposedPreview)
		//		m_pComposedPreview->ProcessTile(m_eComposedMode, a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, a_nStride, reinterpret_cast<TRasterImagePixel*>(a_pBuffer));
		//	if (m_bShowInverted)
		//		RenderTileInvert(szTileInImg.cx, szTileInImg.cy, pSrc, nSrcStride);
		//}
		RECT rcSelection = a_rcTileInImg;
		BOOL bEverythingSelected = TRUE;
		static_cast<T*>(this)->GetSelInfo(&rcSelection, &bEverythingSelected);
		BOOL const bNothingSelected = rcSelection.left > a_rcTileInImg.right || rcSelection.top > a_rcTileInImg.bottom || rcSelection.right < a_rcTileInImg.left || rcSelection.bottom < a_rcTileInImg.top;
		if (bNothingSelected || rcSelection.left > a_rcTileInImg.left || rcSelection.top > a_rcTileInImg.top || rcSelection.right < a_rcTileInImg.right || rcSelection.bottom < a_rcTileInImg.bottom)
			bEverythingSelected = FALSE;
		CAutoVectorPtr<BYTE> cSelBuf;
		if (!bEverythingSelected && !bNothingSelected)
		{
			if (!cSelBuf.Allocate(szTileInImg.cx*szTileInImg.cy) ||
				!static_cast<T*>(this)->GetSelTile(a_rcTileInImg.left, a_rcTileInImg.top, szTileInImg.cx, szTileInImg.cy, szTileInImg.cx, cSelBuf.m_p))
			{
				cSelBuf.Free();
				bEverythingSelected = TRUE;
			}
		}
		ULONG nSelR = GetRValue(m_tSelection);
		ULONG nSelG = GetGValue(m_tSelection);
		ULONG nSelB = GetBValue(m_tSelection);
		ULONG const nGSelR = m_aGammaF[nSelR];
		ULONG const nGSelG = m_aGammaF[nSelG];
		ULONG const nGSelB = m_aGammaF[nSelB];
		TRasterImagePixel aTransparency[2];
		aTransparency[0].bB = GetBValue(M_Square2());
		aTransparency[0].bG = GetGValue(M_Square2());
		aTransparency[0].bR = GetRValue(M_Square2());
		aTransparency[1].bB = GetBValue(M_Square1());
		aTransparency[1].bG = GetGValue(M_Square1());
		aTransparency[1].bR = GetRValue(M_Square1());
		TRasterImagePixel16 aGTransparency[2];
		aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
		aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
		aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
		aGTransparency[1].wR = m_aGammaF[aTransparency[1].bR];
		aGTransparency[1].wG = m_aGammaF[aTransparency[1].bG];
		aGTransparency[1].wB = m_aGammaF[aTransparency[1].bB];
		if (bNothingSelected)
		{
			aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
			aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
			aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
			aGTransparency[1].wB = (aGTransparency[1].wB+nGSelB)>>1;
			aGTransparency[1].wG = (aGTransparency[1].wG+nGSelG)>>1;
			aGTransparency[1].wR = (aGTransparency[1].wR+nGSelR)>>1;
			aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
			aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
			aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
			aTransparency[1].bR = m_aGammaB[aGTransparency[1].wR];
			aTransparency[1].bG = m_aGammaB[aGTransparency[1].wG];
			aTransparency[1].bB = m_aGammaB[aGTransparency[1].wB];
		}
		BYTE* pS = cSelBuf.m_p;
		for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y)
		{
			TRasterImagePixel* pD = reinterpret_cast<TRasterImagePixel*>(a_pBuffer)+(y-a_rcTileInImg.top)*a_nStride;
			TRasterImagePixel const* p = pSrcBuf+(y-a_rcTileInImg.top)*nSrcStride;
			TRasterImagePixel const* const pEnd = p+szTileInImg.cx;
			ULONG nTotalA = 0;
			for (TRasterImagePixel const* pA = p; pA != pEnd; ++pA)
				nTotalA += pA->bA;
			if (nTotalA == 0)
			{
				// everything transparent
				int const yy = y>>TRANSPARENCY_SHIFT;
				if (bEverythingSelected || bNothingSelected)
				{
					for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
					{
						int const xx = x>>TRANSPARENCY_SHIFT;
						*pD = aTransparency[(xx^yy)&1];
						++pD;
					}
				}
				else
				{
					// partially selected empty region
					for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
					{
						int const xx = x>>TRANSPARENCY_SHIFT;
						TRasterImagePixel16 const& t = aGTransparency[(xx^yy)&1];
						ULONG const bDir = 255-*pS;
						ULONG const bInv = 257+*pS;
						pD->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
						pD->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
						pD->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						++pD;
						++pS;
					}
				}
			}
			else if (nTotalA == 255*szTileInImg.cx)
			{
				 // everything opaque
				if (bEverythingSelected)
				{
					while (p != pEnd)
					{
						*pD = *p;
						++pD;
						++p;
					}
				}
				else if (bNothingSelected)
				{
					while (p != pEnd)
					{
						BYTE const bR = p->bR;
						BYTE const bG = p->bG;
						BYTE const bB = p->bB;
						pD->bB = m_aGammaB[(m_aGammaF[bB] + nGSelB)>>1];
						pD->bG = m_aGammaB[(m_aGammaF[bG] + nGSelG)>>1];
						pD->bR = m_aGammaB[(m_aGammaF[bR] + nGSelR)>>1];
						++pD;
						++p;
					}
				}
				else
				{
					while (p != pEnd)
					{
						ULONG const bDir = 255-*pS;
						ULONG const bInv = 257+*pS;
						BYTE const bR = p->bR;
						BYTE const bG = p->bG;
						BYTE const bB = p->bB;
						pD->bB = m_aGammaB[(m_aGammaF[bB]*bInv + nGSelB*bDir)>>9];
						pD->bG = m_aGammaB[(m_aGammaF[bG]*bInv + nGSelG*bDir)>>9];
						pD->bR = m_aGammaB[(m_aGammaF[bR]*bInv + nGSelR*bDir)>>9];
						++pD;
						++p;
						++pS;
					}
				}
			}
			else
			{
				// semitransparent
				int const yy = y>>TRANSPARENCY_SHIFT;
				if (bEverythingSelected)
				{
					for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
					{
						BYTE const bR = p->bR;
						BYTE const bG = p->bG;
						BYTE const bB = p->bB;
						if (p->bA == 255)
						{
							pD->bB = bB;
							pD->bG = bG;
							pD->bR = bR;
						}
						else
						{
							int const xx = x>>TRANSPARENCY_SHIFT;
							TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
							ULONG const bDir = p->bA*0x101+1;
							ULONG const bInv = 0x10000-bDir;
							pD->bB = m_aGammaB[(t1.wB*bInv + m_aGammaF[bB]*bDir)>>16];
							pD->bG = m_aGammaB[(t1.wG*bInv + m_aGammaF[bG]*bDir)>>16];
							pD->bR = m_aGammaB[(t1.wR*bInv + m_aGammaF[bR]*bDir)>>16];
						}
						++pD;
						++p;
					}
				}
				else if (bNothingSelected)
				{
					for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
					{
						BYTE const bR = p->bR;
						BYTE const bG = p->bG;
						BYTE const bB = p->bB;
						if (p->bA == 255)
						{
							pD->bB = m_aGammaB[(m_aGammaF[bB] + nGSelB)>>1];
							pD->bG = m_aGammaB[(m_aGammaF[bG] + nGSelG)>>1];
							pD->bR = m_aGammaB[(m_aGammaF[bR] + nGSelR)>>1];
						}
						else
						{
							int const xx = x>>TRANSPARENCY_SHIFT;
							TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
							ULONG const bInv = 0xffff-p->bA*0x101;
							ULONG const bDir = (0x10000-bInv)>>1;
							pD->bB = m_aGammaB[(t1.wB*bInv + (m_aGammaF[bB] + nGSelB)*bDir)>>16];
							pD->bG = m_aGammaB[(t1.wG*bInv + (m_aGammaF[bG] + nGSelG)*bDir)>>16];
							pD->bR = m_aGammaB[(t1.wR*bInv + (m_aGammaF[bR] + nGSelR)*bDir)>>16];
						}
						++pD;
						++p;
					}
				}
				else
				{
					// partially selected and partially transparent
					for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
					{
						ULONG const bDir = 255-*pS;
						ULONG const bInv = 257+*pS;
						BYTE const bR = p->bR;
						BYTE const bG = p->bG;
						BYTE const bB = p->bB;
						if (p->bA == 255)
						{
							pD->bB = m_aGammaB[(m_aGammaF[bB]*bInv + nGSelB*bDir)>>9];
							pD->bG = m_aGammaB[(m_aGammaF[bG]*bInv + nGSelG*bDir)>>9];
							pD->bR = m_aGammaB[(m_aGammaF[bR]*bInv + nGSelR*bDir)>>9];
						}
						else
						{
							int const xx = x>>TRANSPARENCY_SHIFT;
							TRasterImagePixel16 const& t1 = aGTransparency[(xx^yy)&1];
							ULONG const bDir2 = p->bA*0x101+1;
							ULONG const bInv2 = 0x10000-bDir2;
							ULONG const nTr = (bInv2*bInv)>>3;
							ULONG const nPx = (bDir2*bInv)>>3;
							ULONG const nSl = bDir<<13;
							pD->bB = m_aGammaB[(t1.wB*nTr + m_aGammaF[bB]*nPx + nGSelB*nSl)>>22];
							pD->bG = m_aGammaB[(t1.wG*nTr + m_aGammaF[bG]*nPx + nGSelG*nSl)>>22];
							pD->bR = m_aGammaB[(t1.wR*nTr + m_aGammaF[bR]*nPx + nGSelR*nSl)>>22];
						}
						++p;
						++pD;
						++pS;
					}
				}
			}
		}
	}

	struct SFrom
	{
		ULONG nFrom;
		int nTransparency;
	};

	struct SFromTo
	{
		ULONG nFrom;
		ULONG nTo;
		int nTransparency;
	};

	struct SFromToBlend
	{
		ULONG nFrom1;
		ULONG nTo;
		ULONG nW1;
		int nTransparency;
	};

	void RenderTileNearest(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel const* pSrcBuf)
	{
		SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
		float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
		float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
		//RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
		SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

		//RECT rcPP = rcSourceTile;
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->PreProcessTile(&rcPP);
		SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
		//CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
		//TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
		//static_cast<T*>(this)->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

		//if (m_bShowInverted)
		//	RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

		RECT rcSelection = rcSourceTile;
		BOOL bEverythingSelected = TRUE;
		static_cast<T*>(this)->GetSelInfo(&rcSelection, &bEverythingSelected);
		BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
		if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
			bEverythingSelected = FALSE;
		CAutoVectorPtr<BYTE> cSelBuf;
		if (!bEverythingSelected && !bNothingSelected)
		{
			if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
				!static_cast<T*>(this)->GetSelTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p))
			{
				cSelBuf.Free();
				bEverythingSelected = TRUE;
			}
		}
		ULONG const nSelR = GetRValue(m_tSelection);
		ULONG const nSelG = GetGValue(m_tSelection);
		ULONG const nSelB = GetBValue(m_tSelection);
		COLORREF aTransparency[2] = {((M_Square2()&0xff)<<16)|(M_Square2()&0xff00)|((M_Square2()&0xff0000)>>16), ((M_Square1()&0xff)<<16)|(M_Square1()&0xff00)|((M_Square1()&0xff0000)>>16)};
		if (bNothingSelected)
		{
			aTransparency[0] = RGB((GetRValue(aTransparency[0])+nSelR)>>1, (GetGValue(aTransparency[0])+nSelG)>>1, (GetBValue(aTransparency[0])+nSelB)>>1);
			aTransparency[1] = RGB((GetRValue(aTransparency[1])+nSelR)>>1, (GetGValue(aTransparency[1])+nSelG)>>1, (GetBValue(aTransparency[1])+nSelB)>>1);
		}
		TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		TRasterImagePixel const* pI = pSrcBuf;
		BYTE* pS = cSelBuf.m_p;
		CAutoVectorPtr<SFrom> cSimple(new SFrom[szTileInImg.cx]);
		float fTransparency = 0.5f;
		while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
			fTransparency *= 2.0f;
		{
			// prepare guides for x-resizing
			for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
			{
				cSimple[x-a_rcTileInImg.left].nFrom = (x+0.5f)/fZoomX-rcSourceTile.left;
				cSimple[x-a_rcTileInImg.left].nTransparency = int((x+0.5f)/(fTransparency*fZoomX))&1;
			}
		}
		int iLastY = -1;
		int iLastTransparent = -1;
		int iLastAlpha = -1;
		ULONG nLastTotalAlpha = 0;
		for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO+=a_nStride-szTileInImg.cx)
		{
			int nTR1 = int((y+0.5f)/(fTransparency*fZoomY))&1;
			int nY1 = (y+0.5f)/fZoomY-rcSourceTile.top;
			int const nY2 = (y+1)/fZoomY;
			ULONG const nR1 = 256.0f*(nY2*fZoomY-y)+0.5f;
			ULONG const nR2 = 256-nR1;
			if (nY1 == iLastY && nTR1 == iLastTransparent)
			{
				// copy the previous row
				CopyMemory(pO, pO-a_nStride, szTileInImg.cx*sizeof(*pO));
				pO += szTileInImg.cx;
				continue;
			}

			// just a single row
			iLastY = nY1;
			iLastTransparent = nTR1;
			TRasterImagePixel const* pI1 = pI+nY1*szAdjustedTile.cx;
			SFrom const* const pFEnd = cSimple.m_p+szTileInImg.cx;

			ULONG nTotalA = 0;
			if (iLastAlpha == nY1)
			{
				nTotalA = nLastTotalAlpha;
			}
			else
			{
				if (szSourceTile.cx <= szTileInImg.cx)
				{
					TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
						nTotalA += pA->bA;
					iLastAlpha == nY1;
					nLastTotalAlpha = nTotalA;
				}
				else
				{
					for (SFrom* pF = cSimple; pF != pFEnd; ++pF)
						nTotalA += pI1[pF->nFrom].bA;
					iLastAlpha == nY1;
					nLastTotalAlpha = nTotalA;
				}
			}

			if (nTotalA == 0)
			{
				// transparent row
				if (bEverythingSelected || bNothingSelected)
				{
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
						*pO = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
				}
				else
				{
					// partially selected empty region
					BYTE const* pS1 = pS+nY1*szSourceTile.cx;
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						ULONG const bDir = 255-pS1[pF->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel const t = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
						pO->bB = (t.bB*bInv + nSelB*bDir)>>9;
						pO->bG = (t.bG*bInv + nSelG*bDir)>>9;
						pO->bR = (t.bR*bInv + nSelR*bDir)>>9;
					}
				}
			}
			else if (nTotalA == 255*szSourceTile.cx)
			{
				// opaque row
				if (bEverythingSelected)
				{
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						*pO = pI1[pF->nFrom];
					}
				}
				else if (bNothingSelected)
				{
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						TRasterImagePixel const* const pI = pI1+pF->nFrom;
						pO->bB = (pI->bB + nSelB)>>1;
						pO->bG = (pI->bG + nSelG)>>1;
						pO->bR = (pI->bR + nSelR)>>1;
					}
				}
				else
				{
					BYTE const* pS1 = pS+nY1*szSourceTile.cx;
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						ULONG const bDir = 255-pS1[pF->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel const* const pI = pI1+pF->nFrom;
						pO->bB = (pI->bB*bInv + nSelB*bDir)>>9;
						pO->bG = (pI->bG*bInv + nSelG*bDir)>>9;
						pO->bR = (pI->bR*bInv + nSelR*bDir)>>9;
					}
				}
			}
			else
			{
				// semitransparent row
				if (bEverythingSelected)
				{
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
						TRasterImagePixel const t2 = pI1[pF->nFrom];
						ULONG const bDir2 = t2.bA*0x10101+1;
						ULONG const bInv2 = 0x1000000-bDir2;
						pO->bB = (t1.bB*bInv2 + t2.bB*bDir2)>>24;
						pO->bG = (t1.bG*bInv2 + t2.bG*bDir2)>>24;
						pO->bR = (t1.bR*bInv2 + t2.bR*bDir2)>>24;
					}
				}
				else if (bNothingSelected)
				{
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
						TRasterImagePixel const t2 = pI1[pF->nFrom];
						ULONG const bInv2 = 0xffffff-t2.bA*0x10101;
						ULONG const bDir2 = (0x1000000-bInv2)>>1;
						pO->bB = (t1.bB*bInv2 + (t2.bB + nSelB)*bDir2)>>24;
						pO->bG = (t1.bG*bInv2 + (t2.bG + nSelG)*bDir2)>>24;
						pO->bR = (t1.bR*bInv2 + (t2.bR + nSelR)*bDir2)>>24;
					}
				}
				else
				{
					BYTE const* pS1 = pS+nY1*szSourceTile.cx;
					for (SFrom const* pF = cSimple.m_p; pF != pFEnd; ++pF, ++pO)
					{
						ULONG const bDir = 255-pS1[pF->nFrom];
						ULONG const bInv = 512-bDir;
						TRasterImagePixel const t1 = *reinterpret_cast<TRasterImagePixel*>(aTransparency+(pF->nTransparency^nTR1));
						TRasterImagePixel const t2 = pI1[pF->nFrom];
						ULONG const bDir2 = t2.bA*0x10101+1;
						ULONG const bInv2 = 0x1000000-bDir2;
						BYTE const bB = (t1.bB*bInv2 + t2.bB*bDir2)>>24;
						BYTE const bG = (t1.bG*bInv2 + t2.bG*bDir2)>>24;
						BYTE const bR = (t1.bR*bInv2 + t2.bR*bDir2)>>24;
						pO->bB = (bB*bInv + nSelB*bDir)>>9;
						pO->bG = (bG*bInv + nSelG*bDir)>>9;
						pO->bR = (bR*bInv + nSelR*bDir)>>9;
					}
				}
			}
		}
	}

#define OPAQUEPIXEL_GAMMA(name, source) \
	TRasterImagePixel const t##name = source;\
	WORD const wB##name = m_aGammaF[t##name.bB];\
	WORD const wG##name = m_aGammaF[t##name.bG];\
	WORD const wR##name = m_aGammaF[t##name.bR]
#define TRANSPARENTPIXEL_GAMMA(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bDir##name = t##name.bA*0x4040+1;\
	ULONG const bInv##name = 0x400000-bDir##name;\
	WORD const wB##name = (t1.wB*bInv##name + m_aGammaF[t##name.bB]*bDir##name)>>22;\
	WORD const wG##name = (t1.wG*bInv##name + m_aGammaF[t##name.bG]*bDir##name)>>22;\
	WORD const wR##name = (t1.wR*bInv##name + m_aGammaF[t##name.bR]*bDir##name)>>22
#define TRANSPARENTPIXEL_GAMMA_NOSEL(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bInv##name = 0x3fffff-t##name.bA*0x4040;\
	ULONG const bDir##name = (0x400000-bInv##name)>>1;\
	WORD const wB##name = (t1.wB*bInv##name + (m_aGammaF[t##name.bB] + nGSelB)*bDir##name)>>22;\
	WORD const wG##name = (t1.wG*bInv##name + (m_aGammaF[t##name.bG] + nGSelG)*bDir##name)>>22;\
	WORD const wR##name = (t1.wR*bInv##name + (m_aGammaF[t##name.bR] + nGSelR)*bDir##name)>>22
#define TRANSPARENTPIXELBLEND(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bDir##name = t##name.bA*0x10101+1;\
	ULONG const bInv##name = 0x1000000-bDir##name;\
	BYTE const bB##name = (t1.bB*bInv##name + t##name.bB*bDir##name)>>24;\
	BYTE const bG##name = (t1.bG*bInv##name + t##name.bG*bDir##name)>>24;\
	BYTE const bR##name = (t1.bR*bInv##name + t##name.bR*bDir##name)>>24
#define TRANSPARENTPIXELBLEND_NOSEL(name, source) \
	TRasterImagePixel const t##name = source;\
	ULONG const bInv##name = 0xffffff-t##name.bA*0x10101;\
	ULONG const bDir##name = (0x1000000-bInv##name)>>1;\
	BYTE const bB##name = (t1.bB*bInv##name + (t##name.bB + nSelB)*bDir##name)>>24;\
	BYTE const bG##name = (t1.bG*bInv##name + (t##name.bG + nSelG)*bDir##name)>>24;\
	BYTE const bR##name = (t1.bR*bInv##name + (t##name.bR + nSelR)*bDir##name)>>24
#define WEIGHTEDCOLOR2(output, par1, par2, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2)>>shift
#define WEIGHTEDCOLOR2_GAMMA(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2)>>shift]
#define WEIGHTEDCOLOR2_GAMMA_SELSH(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + (nGSelB<<nSelSh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + (nGSelG<<nSelSh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + (nGSelR<<nSelSh))>>shift]
#define WEIGHTEDCOLOR2_GAMMA_SEL(output, par1, par2, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + (nGSelB*nSel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + (nGSelG*nSel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + (nGSelR*nSel))>>shift]
#define WEIGHTEDCOLOR4(output, par1, par2, par3, par4, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4)>>shift
#define WEIGHTEDCOLOR4_GAMMA(output, par1, par2, par3, par4, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4)>>shift]
#define WEIGHTEDCOLOR4_GAMMA_SELSH(output, par1, par2, par3, par4, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR4_GAMMA_SEL(output, par1, par2, par3, par4, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR6(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6)>>shift
#define WEIGHTEDCOLOR6_GAMMA(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6)>>shift]
#define WEIGHTEDCOLOR6_GAMMA_SELSH(output, par1, par2, par3, par4, par5, par6, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR6_GAMMA_SEL(output, par1, par2, par3, par4, par5, par6, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR9(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + bB##par7*n##par7 + bB##par8*n##par8 + bB##par9*n##par9)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + bG##par7*n##par7 + bG##par8*n##par8 + bG##par9*n##par9)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + bR##par7*n##par7 + bR##par8*n##par8 + bR##par9*n##par9)>>shift
#define WEIGHTEDCOLOR9_GAMMA(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9)>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9)>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9)>>shift]
#define WEIGHTEDCOLOR9_GAMMA_SELSH(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, selsh, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9 + (nGSelB<<selsh))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9 + (nGSelG<<selsh))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9 + (nGSelR<<selsh))>>shift]
#define WEIGHTEDCOLOR9_GAMMA_SEL(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, sel, shift)\
	output->bB = m_aGammaB[(wB##par1*n##par1 + wB##par2*n##par2 + wB##par3*n##par3 + wB##par4*n##par4 + wB##par5*n##par5 + wB##par6*n##par6 + wB##par7*n##par7 + wB##par8*n##par8 + wB##par9*n##par9 + (nGSelB*sel))>>shift];\
	output->bG = m_aGammaB[(wG##par1*n##par1 + wG##par2*n##par2 + wG##par3*n##par3 + wG##par4*n##par4 + wG##par5*n##par5 + wG##par6*n##par6 + wG##par7*n##par7 + wG##par8*n##par8 + wG##par9*n##par9 + (nGSelG*sel))>>shift];\
	output->bR = m_aGammaB[(wR##par1*n##par1 + wR##par2*n##par2 + wR##par3*n##par3 + wR##par4*n##par4 + wR##par5*n##par5 + wR##par6*n##par6 + wR##par7*n##par7 + wR##par8*n##par8 + wR##par9*n##par9 + (nGSelR*sel))>>shift]
#define WEIGHTEDCOLOR2_SEL(output, par1, par2, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR4_SEL(output, par1, par2, par3, par4, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR6_SEL(output, par1, par2, par3, par4, par5, par6, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + nSelR*nSel)>>shift
#define WEIGHTEDCOLOR9_SEL(output, par1, par2, par3, par4, par5, par6, par7, par8, par9, shift)\
	output->bB = (bB##par1*n##par1 + bB##par2*n##par2 + bB##par3*n##par3 + bB##par4*n##par4 + bB##par5*n##par5 + bB##par6*n##par6 + bB##par7*n##par7 + bB##par8*n##par8 + bB##par9*n##par9 + nSelB*nSel)>>shift;\
	output->bG = (bG##par1*n##par1 + bG##par2*n##par2 + bG##par3*n##par3 + bG##par4*n##par4 + bG##par5*n##par5 + bG##par6*n##par6 + bG##par7*n##par7 + bG##par8*n##par8 + bG##par9*n##par9 + nSelG*nSel)>>shift;\
	output->bR = (bR##par1*n##par1 + bR##par2*n##par2 + bR##par3*n##par3 + bR##par4*n##par4 + bR##par5*n##par5 + bR##par6*n##par6 + bR##par7*n##par7 + bR##par8*n##par8 + bR##par9*n##par9 + nSelR*nSel)>>shift
#define COMPUTEWEIGHTSR2xC2\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2
#define COMPUTEWEIGHTSR3xC2\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n31 = nR3*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n32 = nR3*nC2
#define COMPUTEWEIGHTSR2xC3\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n13 = nR1*nC3;\
	ULONG const n23 = nR2*nC3
#define COMPUTEWEIGHTSR3xC3\
	ULONG const n11 = nR1*nC1;\
	ULONG const n21 = nR2*nC1;\
	ULONG const n31 = nR3*nC1;\
	ULONG const n12 = nR1*nC2;\
	ULONG const n22 = nR2*nC2;\
	ULONG const n32 = nR3*nC2;\
	ULONG const n13 = nR1*nC3;\
	ULONG const n23 = nR2*nC3;\
	ULONG const n33 = nR3*nC3

	void RenderTileZoomIn(RECT const a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel const* pSrcBuf)
	{
		SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
		float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
		float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
		//RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
		SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

		//RECT rcPP = rcSourceTile;
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->PreProcessTile(&rcPP);
		SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
		//CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
		//TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
		//static_cast<T*>(this)->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

		//if (m_bShowInverted)
		//	RenderTileInvert(szSourceTile.cx, szSourceTile.cy, pSrcBuf, szAdjustedTile.cx);

		RECT rcSelection = rcSourceTile;
		BOOL bEverythingSelected = TRUE;
		static_cast<T*>(this)->GetSelInfo(&rcSelection, &bEverythingSelected);
		BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
		if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
			bEverythingSelected = FALSE;
		CAutoVectorPtr<BYTE> cSelBuf;
		if (!bEverythingSelected && !bNothingSelected)
		{
			if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
				!static_cast<T*>(this)->GetSelTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p))
			{
				cSelBuf.Free();
				bEverythingSelected = TRUE;
			}
		}
		ULONG const nSelR = GetRValue(m_tSelection);
		ULONG const nSelG = GetGValue(m_tSelection);
		ULONG const nSelB = GetBValue(m_tSelection);
		ULONG const nGSelR = m_aGammaF[nSelR];
		ULONG const nGSelG = m_aGammaF[nSelG];
		ULONG const nGSelB = m_aGammaF[nSelB];
		TRasterImagePixel aTransparency[17];
		aTransparency[0].bB = GetBValue(M_Square1());
		aTransparency[0].bG = GetGValue(M_Square1());
		aTransparency[0].bR = GetRValue(M_Square1());
		aTransparency[16].bB = GetBValue(M_Square2());
		aTransparency[16].bG = GetGValue(M_Square2());
		aTransparency[16].bR = GetRValue(M_Square2());
		TRasterImagePixel16 aGTransparency[17];
		aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
		aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
		aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
		aGTransparency[16].wR = m_aGammaF[aTransparency[16].bR];
		aGTransparency[16].wG = m_aGammaF[aTransparency[16].bG];
		aGTransparency[16].wB = m_aGammaF[aTransparency[16].bB];
		if (bNothingSelected)
		{
			aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
			aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
			aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
			aGTransparency[16].wB = (aGTransparency[16].wB+nGSelB)>>1;
			aGTransparency[16].wG = (aGTransparency[16].wG+nGSelG)>>1;
			aGTransparency[16].wR = (aGTransparency[16].wR+nGSelR)>>1;
			aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
			aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
			aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
			aTransparency[16].bR = m_aGammaB[aGTransparency[16].wR];
			aTransparency[16].bG = m_aGammaB[aGTransparency[16].wG];
			aTransparency[16].bB = m_aGammaB[aGTransparency[16].wB];
		}
		for (int i = 1; i < 16; ++i)
		{
			aGTransparency[i].wR = (aGTransparency[0].wR*i + aGTransparency[16].wR*(16-i))>>4;
			aGTransparency[i].wG = (aGTransparency[0].wG*i + aGTransparency[16].wG*(16-i))>>4;
			aGTransparency[i].wB = (aGTransparency[0].wB*i + aGTransparency[16].wB*(16-i))>>4;
			aTransparency[i].bR = m_aGammaB[aGTransparency[i].wR];
			aTransparency[i].bG = m_aGammaB[aGTransparency[i].wG];
			aTransparency[i].bB = m_aGammaB[aGTransparency[i].wB];
		}
		TRasterImagePixel const* const pTransparency = aTransparency+8;
		TRasterImagePixel16 const* const pGTransparency = aGTransparency+8;
		TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		TRasterImagePixel const* pI = pSrcBuf;
		BYTE* pS = cSelBuf.m_p;
		CAutoVectorPtr<SFromTo> cSimple(new SFromTo[szTileInImg.cx]);
		CAutoVectorPtr<SFromToBlend> cBlend(new SFromToBlend[szTileInImg.cx]);
		ULONG nSimple = 0;
		ULONG nCopy = 0;
		ULONG nBlend = 0;
		float fTransparency = 0.5f;
		while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
			fTransparency *= 2.0f;
		{
			// prepare guides for x-resizing
			for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
			{
				int nTX1 = x/(fTransparency*fZoomX);
				int const nTX2 = (x+1)/(fTransparency*fZoomX);
				int nTC1 = 16.0f*(nTX2*fZoomX*fTransparency-x)+0.5f;
				if (nTX1 == nTX2 || nTC1 >= 16)
				{
					nTC1 = (nTX1&1)<<4;
				}
				else
				{
					if (nTC1 <= 0)
					{
						nTX1 = nTX2;
						nTC1 = (nTX1&1)<<4;
					}
					else if (nTX1&1)
						nTC1 = 16-nTC1;
				}
				nTC1 -= 8;

				int nX1 = x/fZoomX;
				int const nX2 = (x+1)/fZoomX;
				ULONG const nC1 = 256.0f*(nX2*fZoomX-x)+0.5f;
				if (nC1 <= 1)
				{
					nX1 = nX2;
				}
				//ULONG const nC2 = 256-nC1;
				if (nX1 == nX2 || nC1 >= 255)
				{
					if (nSimple && cSimple[nSimple-1].nFrom == nX1-rcSourceTile.left && cSimple[nSimple-1].nTransparency == nTC1)
					{
						SFromTo& s = cSimple[szTileInImg.cx-1-(nCopy++)];
						s.nFrom = cSimple[nSimple-1].nTo;
						s.nTo = x-a_rcTileInImg.left;
					}
					else
					{
						SFromTo& s = cSimple[nSimple++];
						s.nFrom = nX1-rcSourceTile.left;
						s.nTo = x-a_rcTileInImg.left;
						s.nTransparency = nTC1;
					}
				}
				else
				{
					SFromToBlend& s = cBlend[nBlend++];
					s.nFrom1 = nX1-rcSourceTile.left;
					s.nTo = x-a_rcTileInImg.left;
					s.nW1 = nC1;
					s.nTransparency = nTC1;
				}
			}
		}
		int iLastY = -1;
		int iLastTransparent = -1;
		int iLastAlpha = -1;
		ULONG nLastTotalAlpha = 0;
		for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride)
		{
			int nTY1 = y/(fTransparency*fZoomY);
			int const nTY2 = (y+1)/(fTransparency*fZoomY);
			int nTR1 = 16.0f*(nTY2*fZoomY*fTransparency-y)+0.5f;
			if (nTY1 == nTY2 || nTR1 >= 16)
			{
				nTR1 = (nTY1&1)<<4;
			}
			else
			{
				if (nTR1 <= 0)
				{
					nTY1 = nTY2;
					nTR1 = (nTY1&1)<<4;
				}
				else if (nTY1&1)
					nTR1 = 16-nTR1;
			}
			nTR1 -= 8;

			int nY1 = y/fZoomY;
			int const nY2 = (y+1)/fZoomY;
			ULONG const nR1 = 256.0f*(nY2*fZoomY-y)+0.5f;
			ULONG const nR2 = 256-nR1;
			if (nR1 <= 1)
			{
				nY1 = nY2;
			}
			if (nY1 == nY2 || nR1 >= 255)
			{
				if (nY1 == iLastY && nTR1 == iLastTransparent)
				{
					// copy the previous row
					CopyMemory(pO, pO-a_nStride, szTileInImg.cx*sizeof(*pO));
					continue;
				}

				// just a single row
				iLastY = nY1;
				iLastTransparent = nTR1;
				TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
				SFromTo const* const pFTEnd = cSimple.m_p+nSimple;
				SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;

				ULONG nTotalA = 0;
				if (iLastAlpha == nY1)
				{
					nTotalA = nLastTotalAlpha;
				}
				else
				{
					TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
						nTotalA += pA->bA;
					iLastAlpha == nY1;
					nLastTotalAlpha = nTotalA;
				}

				if (nTotalA == 0)
				{
					// transparent row
					if (bEverythingSelected || bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
							pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
							pO[pFT->nTo] = pTransparency[(pFT->nTransparency*nTR1)>>3];
					}
					else
					{
						// partially selected empty region
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-pS1[pFT->nFrom];
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
					}
				}
				else if (nTotalA == 255*szSourceTile.cx)
				{
					// opaque row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1;
							ULONG const n12 = 256-n11;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							pO[pFT->nTo] = pI1[pFT->nFrom];
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1;
							ULONG const n12 = 256-n11;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							int const nSelSh = 8;
							WEIGHTEDCOLOR2_GAMMA_SELSH(pO1, 11, 12, 9);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							TRasterImagePixel const* const pI = pI1+pFT->nFrom;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[(m_aGammaF[pI->bB] + nGSelB)>>1];
							pO1->bG = m_aGammaB[(m_aGammaF[pI->bG] + nGSelG)>>1];
							pO1->bR = m_aGammaB[(m_aGammaF[pI->bR] + nGSelR)>>1];
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1*bInv;
							ULONG const n12 = (bInv<<8)-n11;
							ULONG const nSel = bDir<<8;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 12, 17);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-pS1[pFT->nFrom];
							ULONG const bInv = 512-bDir;
							TRasterImagePixel const* const pI = pI1+pFT->nFrom;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[(m_aGammaF[pI->bB]*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(m_aGammaF[pI->bG]*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(m_aGammaF[pI->bR]*bInv + nGSelR*bDir)>>9];
						}
					}
				}
				else
				{
					// semitransparent row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1;
							ULONG const n12 = 256-n11;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[wB11];
							pO1->bG = m_aGammaB[wG11];
							pO1->bR = m_aGammaB[wR11];
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1;
							ULONG const n12 = 256-n11;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 12, 8);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFT->nFrom]);
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[wB11];
							pO1->bG = m_aGammaB[wG11];
							pO1->bR = m_aGammaB[wR11];
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*pFTB->nW1+pS1[pFTB->nFrom1+1]*(256-pFTB->nW1))>>8);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							ULONG const n11 = pFTB->nW1*bInv;
							ULONG const n12 = (bInv<<8)-n11;
							ULONG const nSel = bDir<<8;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 12, 17);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-pS1[pFT->nFrom];
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[(wB11*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(wG11*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(wR11*bInv + nGSelR*bDir)>>9];
						}
					}
				}
			}
			else
			{
				// blend two rows
				iLastY = -1;
				TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
				TRasterImagePixel const* pI2 = pI1+szAdjustedTile.cx;
				SFromTo const* const pFTEnd = cSimple.m_p+nSimple;
				SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;

				ULONG nTotalA = 0;
				if (iLastAlpha == nY1)
				{
					nTotalA = nLastTotalAlpha;
				}
				else
				{
					TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
						nTotalA += pA->bA;
				}
				{
					iLastAlpha == nY2;
					nLastTotalAlpha = 0;
					TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
						nLastTotalAlpha += pA->bA;
					nTotalA += nLastTotalAlpha;
				}

				if (nTotalA == 0)
				{
					// transparent row
					if (bEverythingSelected || bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
							pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
							pO[pFT->nTo] = pTransparency[(pFT->nTransparency*nTR1)>>3];
					}
					else
					{
						// partially selected empty region
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
					}
				}
				else if (nTotalA == 510*szSourceTile.cx)
				{
					// opaque row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
							ULONG const n11 = nR1;
							ULONG const n21 = nR2;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SELSH(pO1, 11, 12, 21, 22, 16, 17);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
							ULONG const n11 = nR1;
							ULONG const n21 = nR2;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							int const nSelSh = 8;
							WEIGHTEDCOLOR2_GAMMA_SELSH(pO1, 11, 21, 9);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR2xC2;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFT->nFrom]);
							TRasterImagePixel const* const p1 = pI1+pFT->nFrom;
							TRasterImagePixel const* const p2 = pI2+pFT->nFrom;
							ULONG const n11 = nR1*bInv;
							ULONG const n21 = nR2*bInv;
							ULONG const nSel = bDir<<8;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 21, 17);
						}
					}
				}
				else
				{
					// semitransparent row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFT->nFrom]);
							ULONG const n11 = nR1;
							ULONG const n21 = nR2;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFT->nFrom]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFT->nFrom]);
							ULONG const n11 = nR1;
							ULONG const n21 = nR2;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							WEIGHTEDCOLOR2_GAMMA(pO1, 11, 21, 8);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nC1*nR1+pS1[pFTB->nFrom1+1]*nC2*nR1+pS2[pFTB->nFrom1]*nC1*nR2+pS2[pFTB->nFrom1+1]*nC2*nR2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR2xC2;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
						}
						for (SFromTo const* pFT = cSimple.m_p; pFT != pFTEnd; ++pFT)
						{
							ULONG const bDir = 255-((pS1[pFT->nFrom]*nR1+pS2[pFT->nFrom]*nR2)>>8);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFT->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFT->nFrom]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFT->nFrom]);
							ULONG const n11 = nR1*bInv;
							ULONG const n21 = nR2*bInv;
							ULONG const nSel = bDir<<8;
							TRasterImagePixel* const pO1 = pO+pFT->nTo;
							WEIGHTEDCOLOR2_GAMMA_SEL(pO1, 11, 21, 17);
						}
					}
				}
			}
			SFromTo const* const pFTCEnd = cSimple.m_p+szTileInImg.cx-1-nCopy;
			for (SFromTo const* pFT = cSimple.m_p+szTileInImg.cx-1; pFT != pFTCEnd; --pFT)
				pO[pFT->nTo] = pO[pFT->nFrom];
		}
	}

	void RenderTileZoomOutSimple(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel const* pSrcBuf)
	{
		SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
		float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
		float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
		//RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
		SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

		//RECT rcPP = rcSourceTile;
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->PreProcessTile(&rcPP);
		SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
		//CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
		//TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
		//static_cast<T*>(this)->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

		//if (m_bShowInverted)
		//	RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

		RECT rcSelection = rcSourceTile;
		BOOL bEverythingSelected = TRUE;
		static_cast<T*>(this)->GetSelInfo(&rcSelection, &bEverythingSelected);
		BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
		if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
			bEverythingSelected = FALSE;
		CAutoVectorPtr<BYTE> cSelBuf;
		if (!bEverythingSelected && !bNothingSelected)
		{
			if (!cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) ||
				!static_cast<T*>(this)->GetSelTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p))
			{
				cSelBuf.Free();
				bEverythingSelected = TRUE;
			}
		}
		ULONG const nSelR = GetRValue(m_tSelection);
		ULONG const nSelG = GetGValue(m_tSelection);
		ULONG const nSelB = GetBValue(m_tSelection);
		ULONG const nGSelR = m_aGammaF[nSelR];
		ULONG const nGSelG = m_aGammaF[nSelG];
		ULONG const nGSelB = m_aGammaF[nSelB];
		TRasterImagePixel aTransparency[17];
		aTransparency[0].bB = GetBValue(M_Square1());
		aTransparency[0].bG = GetGValue(M_Square1());
		aTransparency[0].bR = GetRValue(M_Square1());
		aTransparency[16].bB = GetBValue(M_Square2());
		aTransparency[16].bG = GetGValue(M_Square2());
		aTransparency[16].bR = GetRValue(M_Square2());
		TRasterImagePixel16 aGTransparency[17];
		aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
		aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
		aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
		aGTransparency[16].wR = m_aGammaF[aTransparency[16].bR];
		aGTransparency[16].wG = m_aGammaF[aTransparency[16].bG];
		aGTransparency[16].wB = m_aGammaF[aTransparency[16].bB];
		if (bNothingSelected)
		{
			aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
			aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
			aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
			aGTransparency[16].wB = (aGTransparency[16].wB+nGSelB)>>1;
			aGTransparency[16].wG = (aGTransparency[16].wG+nGSelG)>>1;
			aGTransparency[16].wR = (aGTransparency[16].wR+nGSelR)>>1;
			aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
			aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
			aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
			aTransparency[16].bR = m_aGammaB[aGTransparency[16].wR];
			aTransparency[16].bG = m_aGammaB[aGTransparency[16].wG];
			aTransparency[16].bB = m_aGammaB[aGTransparency[16].wB];
		}
		for (int i = 1; i < 16; ++i)
		{
			aGTransparency[i].wR = (aGTransparency[0].wR*i + aGTransparency[16].wR*(16-i))>>4;
			aGTransparency[i].wG = (aGTransparency[0].wG*i + aGTransparency[16].wG*(16-i))>>4;
			aGTransparency[i].wB = (aGTransparency[0].wB*i + aGTransparency[16].wB*(16-i))>>4;
			aTransparency[i].bR = m_aGammaB[aGTransparency[i].wR];
			aTransparency[i].bG = m_aGammaB[aGTransparency[i].wG];
			aTransparency[i].bB = m_aGammaB[aGTransparency[i].wB];
		}
		TRasterImagePixel const* const pTransparency = aTransparency+8;
		TRasterImagePixel16 const* const pGTransparency = aGTransparency+8;
		TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		TRasterImagePixel const* pI = pSrcBuf;
		BYTE* pS = cSelBuf.m_p;
		CAutoVectorPtr<SFromToBlend> cBlend(new SFromToBlend[szTileInImg.cx]);
		ULONG nBlend = 0;
		ULONG nBlend3 = 0;
		float fTransparency = TRANSPARENCY_GRID;
		while (fTransparency*m_fZoom < TRANSPARENCY_GRID)
			fTransparency *= 2.0f;
		int const nFullX = 256.0f*fZoomX+0.5f;
		int const nMin1X = 256-nFullX;
		{
			// prepare guides for x-resizing
			for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
			{
				int nTX1 = x/(fTransparency*fZoomX);
				int const nTX2 = (x+1)/(fTransparency*fZoomX);
				int nTC1 = 16.0f*(nTX2*fZoomX*fTransparency-x)+0.5f;
				if (nTX1 == nTX2 || nTC1 >= 16)
				{
					nTC1 = (nTX1&1)<<4;
				}
				else
				{
					if (nTC1 <= 0)
					{
						nTX1 = nTX2;
						nTC1 = (nTX1&1)<<4;
					}
					else if (nTX1&1)
						nTC1 = 16-nTC1;
				}
				nTC1 -= 8;

				int nX1 = x/fZoomX;
				int nX2 = (x+1)/fZoomX;
				ULONG nC11 = 256.0f*((nX1+1)*fZoomX-x)+0.5f;
				if (nC11 <= 1)
				{
					// skip the first pixel (weight too small)
					nC11 = nFullX;
					++nX1;
				}
				if (nX1+2 <= nX2 && nC11+nFullX >= 255)
				{
					// skip the last pixel (weight too small)
					--nX2;
				}
				if (nX1 == nX2)
				{
					// zoom is too small - it should be just one pixel, but to keep the algo simple, change it to two with the weight of 0 on the dummy pixel
					if (nX1 == rcSourceTile.left)
						nC11 = 256;
					else
					{
						nC11 = 0;
						--nX1;
					}
				}

				SFromToBlend& s = cBlend[nX1+2 <= nX2 ? szTileInImg.cx-1-(nBlend3++) : nBlend++];
				s.nFrom1 = nX1-rcSourceTile.left;
				s.nTo = x-a_rcTileInImg.left;
				s.nW1 = nC11;
				s.nTransparency = nTC1;
			}
		}
		int iLastAlpha = -1;
		ULONG nLastTotalAlpha = 0;
		int const nFullY = 256.0f*fZoomY+0.5f;
		int const nMin1Y = 256-nFullY;
		for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride)
		{
			int nTY1 = y/(fTransparency*fZoomY);
			int const nTY2 = (y+1)/(fTransparency*fZoomY);
			int nTR1 = 16.0f*(nTY2*fZoomY*fTransparency-y)+0.5f;
			if (nTY1 == nTY2 || nTR1 >= 16)
			{
				nTR1 = (nTY1&1)<<4;
			}
			else
			{
				if (nTR1 <= 0)
				{
					nTY1 = nTY2;
					nTR1 = (nTY1&1)<<4;
				}
				else if (nTY1&1)
					nTR1 = 16-nTR1;
			}
			nTR1 -= 8;

			int nY1 = y/fZoomY;
			int nY2 = (y+1)/fZoomY;
			ULONG nR11 = 256.0f*((nY1+1)*fZoomY-y)+0.5f;
			if (nR11 <= 1)
			{
				// skip the first pixel (weight too small)
				nR11 = nFullY;
				++nY1;
			}
			if (nY1+2 <= nY2 && nR11+nFullY >= 255)
			{
				// skip the last pixel (weight too small)
				--nY2;
			}
			if (nY1 == nY2)
			{
				// damn - zoom is too small - it should be just one row, but to keep the algo simple, change it to two with the weight of 0 on the dummy row
				if (nY1 == rcSourceTile.top)
					nR11 = 256;
				else
				{
					nR11 = 0;
					--nY1;
				}
			}

			TRasterImagePixel const* pI1 = pI+(nY1-rcSourceTile.top)*szAdjustedTile.cx;
			TRasterImagePixel const* pI2 = pI1+szAdjustedTile.cx;
			SFromToBlend const* const pFTBEnd = cBlend.m_p+nBlend;
			SFromToBlend const* const pFT3End = cBlend.m_p+szTileInImg.cx;

			if (nY1+2 <= nY2)
			{
				// three (or more (but ignored)) rows
				TRasterImagePixel const* pI3 = pI2+szAdjustedTile.cx;
				ULONG const nR1 = nR11;
				ULONG const nR2 = nFullY;
				ULONG const nR3 = nMin1Y-nR11;

				ULONG nTotalA = 0;
				if (iLastAlpha == nY1)
				{
					nTotalA = nLastTotalAlpha;
				}
				else
				{
					TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
						nTotalA += pA->bA;
				}
				{
					TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
						nTotalA += pA->bA;
				}
				{
					iLastAlpha == nY1+2;
					nLastTotalAlpha = 0;
					TRasterImagePixel const* const pEnd = pI3+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI3; pA != pEnd; ++pA)
						nLastTotalAlpha += pA->bA;
					nTotalA += nLastTotalAlpha;
				}

				if (nTotalA == 0)
				{
					// transparent row
					if (bEverythingSelected || bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFT3End; ++pFTB)
							pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
					}
					else
					{
						// partially selected empty region
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						BYTE const* pS3 = pS2+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
					}
				}
				else if (nTotalA == 765*szSourceTile.cx)
				{
					// opaque row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR3xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR3xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR3xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SELSH(pO1, 11, 12, 21, 22, 31, 32, 16, 17);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR3xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA_SELSH(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16, 17);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						BYTE const* pS3 = pS2+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR3xC2;
							ULONG const bDir2 = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 21, 22, 31, 32, bDir2, 22);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = nFullX;
							ULONG nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							nC3 = (nC3*bInv)>>3;
							COMPUTEWEIGHTSR3xC3;
							ULONG const bDir2 = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, bDir2, 22);
						}
					}
				}
				else
				{
					// semitransparent row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR3xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR3xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(32, pI3[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR3xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 21, 22, 31, 32, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(23, pI2[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(32, pI3[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(33, pI3[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR3xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, 16);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						BYTE const* pS3 = pS2+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR3xC2;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 21, 22, 31, 32, nSel, 22);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = nFullX;
							ULONG nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3 + pS3[pFTB->nFrom1]*nR3*nC1 + pS3[pFTB->nFrom1+1]*nR3*nC2 + pS3[pFTB->nFrom1+2]*nR3*nC3)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(31, pI3[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(32, pI3[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(33, pI3[pFTB->nFrom1+2]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							nC3 = (nC3*bInv)>>3;
							COMPUTEWEIGHTSR3xC3;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR9_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, 31, 32, 33, nSel, 22);
						}
					}
				}
			}
			else
			{
				// two rows
				ULONG const nR1 = nR11;
				ULONG const nR2 = 256-nR11;

				ULONG nTotalA = 0;
				if (iLastAlpha == nY1)
				{
					nTotalA = nLastTotalAlpha;
				}
				else
				{
					TRasterImagePixel const* const pEnd = pI1+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI1; pA != pEnd; ++pA)
						nTotalA += pA->bA;
				}
				{
					iLastAlpha == nY1+1;
					nLastTotalAlpha = 0;
					TRasterImagePixel const* const pEnd = pI2+szSourceTile.cx;
					for (TRasterImagePixel const* pA = pI2; pA != pEnd; ++pA)
						nLastTotalAlpha += pA->bA;
					nTotalA += nLastTotalAlpha;
				}

				if (nTotalA == 0)
				{
					// transparent row
					if (bEverythingSelected || bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFT3End; ++pFTB)
							pO[pFTB->nTo] = pTransparency[(pFTB->nTransparency*nTR1)>>3];
					}
					else
					{
						// partially selected empty region
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							pO1->bB = m_aGammaB[(t.wB*bInv + nGSelB*bDir)>>9];
							pO1->bG = m_aGammaB[(t.wG*bInv + nGSelG*bDir)>>9];
							pO1->bR = m_aGammaB[(t.wR*bInv + nGSelR*bDir)>>9];
						}
					}
				}
				else if (nTotalA == 510*szSourceTile.cx)
				{
					// opaque row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR2xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SELSH(pO1, 11, 12, 21, 22, 16, 17);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR2xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SELSH(pO1, 11, 12, 13, 21, 22, 23, 16, 17);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR2xC2;
							ULONG const bDir2 = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, bDir2, 22);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = nFullX;
							ULONG nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
							ULONG const bInv = 512-bDir;
							OPAQUEPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							OPAQUEPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							OPAQUEPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							OPAQUEPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							nC3 = (nC3*bInv)>>3;
							COMPUTEWEIGHTSR2xC3;
							ULONG const bDir2 = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, bDir2, 22);
						}
					}
				}
				else
				{
					// semitransparent row
					if (bEverythingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR2xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
						}
					}
					else if (bNothingSelected)
					{
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = 256-nC1;
							COMPUTEWEIGHTSR2xC2;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA(pO1, 11, 12, 21, 22, 16);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA_NOSEL(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA_NOSEL(23, pI2[pFTB->nFrom1+2]);
							ULONG const nC1 = pFTB->nW1;
							ULONG const nC2 = nFullX;
							ULONG const nC3 = nMin1X-nC1;
							COMPUTEWEIGHTSR2xC3;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA(pO1, 11, 12, 13, 21, 22, 23, 16);
						}
					}
					else
					{
						BYTE const* pS1 = pS+(nY1-rcSourceTile.top)*szSourceTile.cx;
						BYTE const* pS2 = pS1+szSourceTile.cx;
						for (SFromToBlend const* pFTB = cBlend.m_p; pFTB != pFTBEnd; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = 256-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							COMPUTEWEIGHTSR2xC2;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR4_GAMMA_SEL(pO1, 11, 12, 21, 22, nSel, 22);
						}
						for (SFromToBlend const* pFTB = pFTBEnd; pFTB != pFT3End; ++pFTB)
						{
							ULONG nC1 = pFTB->nW1;
							ULONG nC2 = nFullX;
							ULONG nC3 = nMin1X-nC1;
							ULONG const bDir = 255-((pS1[pFTB->nFrom1]*nR1*nC1 + pS1[pFTB->nFrom1+1]*nR1*nC2 + pS1[pFTB->nFrom1+2]*nR1*nC3 + pS2[pFTB->nFrom1]*nR2*nC1 + pS2[pFTB->nFrom1+1]*nR2*nC2 + pS2[pFTB->nFrom1+2]*nR2*nC3)>>16);
							ULONG const bInv = 512-bDir;
							TRasterImagePixel16 const& t1 = pGTransparency[(pFTB->nTransparency*nTR1)>>3];
							TRANSPARENTPIXEL_GAMMA(11, pI1[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(12, pI1[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(13, pI1[pFTB->nFrom1+2]);
							TRANSPARENTPIXEL_GAMMA(21, pI2[pFTB->nFrom1]);
							TRANSPARENTPIXEL_GAMMA(22, pI2[pFTB->nFrom1+1]);
							TRANSPARENTPIXEL_GAMMA(23, pI2[pFTB->nFrom1+2]);
							nC1 = (nC1*bInv)>>3;
							nC2 = (nC2*bInv)>>3;
							nC3 = (nC3*bInv)>>3;
							COMPUTEWEIGHTSR2xC3;
							ULONG const nSel = bDir<<13;
							TRasterImagePixel* const pO1 = pO+pFTB->nTo;
							WEIGHTEDCOLOR6_GAMMA_SEL(pO1, 11, 12, 13, 21, 22, 23, nSel, 22);
						}
					}
				}
			}
		}
	}

	struct SFromToBlendN
	{
		ULONG nFrom;
		ULONG nFull;
		ULONG nW0;
		//ULONG nWN;
	};

	struct TRasterImagePixel32I
	{
		ULONG nR;
		ULONG nG;
		ULONG nB;
		ULONG nA;
	};

	void RenderTileZoomOutCoverage(RECT const& a_rcTileInImg, COLORREF* a_pBuffer, ULONG a_nStride, RECT const rcSourceTile, RECT const rcPP, TRasterImagePixel const* pSrcBuf)
	{
		SIZE const szTileInImg = {a_rcTileInImg.right-a_rcTileInImg.left, a_rcTileInImg.bottom-a_rcTileInImg.top};
		float const fZoomX = float(m_tZoomedSize.cx)/float(m_tImageSize.cx);
		float const fZoomY = float(m_tZoomedSize.cy)/float(m_tImageSize.cy);
		//RECT const rcSourceTile = {a_rcTileInImg.left/fZoomX, a_rcTileInImg.top/fZoomY, min(m_tImageSize.cx, LONG(ceilf(a_rcTileInImg.right/fZoomX))), min(m_tImageSize.cy, LONG(ceilf(a_rcTileInImg.bottom/fZoomY)))};
		SIZE const szSourceTile = {rcSourceTile.right-rcSourceTile.left, rcSourceTile.bottom-rcSourceTile.top};

		//RECT rcPP = rcSourceTile;
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->PreProcessTile(&rcPP);
		SIZE const szAdjustedTile = {rcPP.right-rcPP.left, rcPP.bottom-rcPP.top};
		//CAutoVectorPtr<TRasterImagePixel> cSrcBuf(new TRasterImagePixel[szAdjustedTile.cx*szAdjustedTile.cy]);
		//TRasterImagePixel* const pSrcBuf = cSrcBuf.m_p+(rcSourceTile.top-rcPP.top)*szAdjustedTile.cx+rcSourceTile.left-rcPP.left;
		//static_cast<T*>(this)->GetImageTile(rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);
		//if (m_bShowComposed && m_pComposedPreview)
		//	m_pComposedPreview->ProcessTile(m_eComposedMode, rcPP.left, rcPP.top, szAdjustedTile.cx, szAdjustedTile.cy, szAdjustedTile.cx, cSrcBuf);

		//if (m_bShowInverted)
		//	RenderTileInvert(szAdjustedTile.cx, szAdjustedTile.cy, cSrcBuf, szAdjustedTile.cx);

		RECT rcSelection = rcSourceTile;
		BOOL bEverythingSelected = TRUE;
		static_cast<T*>(this)->GetSelInfo(&rcSelection, &bEverythingSelected);
		BOOL const bNothingSelected = rcSelection.left > rcSourceTile.right || rcSelection.top > rcSourceTile.bottom || rcSelection.right < rcSourceTile.left || rcSelection.bottom < rcSourceTile.top;
		if (bNothingSelected || rcSelection.left > rcSourceTile.left || rcSelection.top > rcSourceTile.top || rcSelection.right < rcSourceTile.right || rcSelection.bottom < rcSourceTile.bottom)
			bEverythingSelected = FALSE;
		ULONG const nSelR = GetRValue(m_tSelection);
		ULONG const nSelG = GetGValue(m_tSelection);
		ULONG const nSelB = GetBValue(m_tSelection);
		ULONG const nGSelR = m_aGammaF[nSelR];
		ULONG const nGSelG = m_aGammaF[nSelG];
		ULONG const nGSelB = m_aGammaF[nSelB];
		TRasterImagePixel aTransparency[2];
		aTransparency[0].bB = GetBValue(M_Square2());
		aTransparency[0].bG = GetGValue(M_Square2());
		aTransparency[0].bR = GetRValue(M_Square2());
		aTransparency[1].bB = GetBValue(M_Square1());
		aTransparency[1].bG = GetGValue(M_Square1());
		aTransparency[1].bR = GetRValue(M_Square1());
		TRasterImagePixel16 aGTransparency[2];
		aGTransparency[0].wR = m_aGammaF[aTransparency[0].bR];
		aGTransparency[0].wG = m_aGammaF[aTransparency[0].bG];
		aGTransparency[0].wB = m_aGammaF[aTransparency[0].bB];
		aGTransparency[1].wR = m_aGammaF[aTransparency[1].bR];
		aGTransparency[1].wG = m_aGammaF[aTransparency[1].bG];
		aGTransparency[1].wB = m_aGammaF[aTransparency[1].bB];
		if (bNothingSelected)
		{
			aGTransparency[0].wB = (aGTransparency[0].wB+nGSelB)>>1;
			aGTransparency[0].wG = (aGTransparency[0].wG+nGSelG)>>1;
			aGTransparency[0].wR = (aGTransparency[0].wR+nGSelR)>>1;
			aGTransparency[1].wB = (aGTransparency[1].wB+nGSelB)>>1;
			aGTransparency[1].wG = (aGTransparency[1].wG+nGSelG)>>1;
			aGTransparency[1].wR = (aGTransparency[1].wR+nGSelR)>>1;
			aTransparency[0].bR = m_aGammaB[aGTransparency[0].wR];
			aTransparency[0].bG = m_aGammaB[aGTransparency[0].wG];
			aTransparency[0].bB = m_aGammaB[aGTransparency[0].wB];
			aTransparency[1].bR = m_aGammaB[aGTransparency[1].wR];
			aTransparency[1].bG = m_aGammaB[aGTransparency[1].wG];
			aTransparency[1].bB = m_aGammaB[aGTransparency[1].wB];
		}
		int nTransparencyShift = TRANSPARENCY_SHIFT;
		while ((1<<nTransparencyShift)*m_fZoom < TRANSPARENCY_GRID)
			++nTransparencyShift;
		if (!bEverythingSelected && !bNothingSelected)
		{
			CAutoVectorPtr<BYTE> cSelBuf;
			if (cSelBuf.Allocate(szSourceTile.cx*szSourceTile.cy) &&
				static_cast<T*>(this)->GetSelTile(rcSourceTile.left, rcSourceTile.top, szSourceTile.cx, szSourceTile.cy, szSourceTile.cx, cSelBuf.m_p))
			{
				TRasterImagePixel *p = const_cast<TRasterImagePixel*>(pSrcBuf);
				BYTE *pS = cSelBuf;
				for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
				{
					int const yy = y>>nTransparencyShift;
					for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
					{
						if (p->bA == 255)
						{
							ULONG const bDir = 255-*pS;
							ULONG const bInv = 257+*pS;
							p->bR = m_aGammaB[(m_aGammaF[p->bR]*bInv + nGSelR*bDir)>>9];
							p->bG = m_aGammaB[(m_aGammaF[p->bG]*bInv + nGSelG*bDir)>>9];
							p->bB = m_aGammaB[(m_aGammaF[p->bB]*bInv + nGSelB*bDir)>>9];
						}
						else
						{
							int const xx = x>>nTransparencyShift;
							TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
							ULONG const bDir = p->bA*0x4040+1;
							ULONG const bInv = 0x400000-bDir;
							ULONG const bDir2 = 255-*pS;
							ULONG const bInv2 = 257+*pS;
							p->bR = m_aGammaB[(((t1.wB*bInv + m_aGammaF[p->bR]*bDir)>>22)*bInv2 + nGSelR*bDir2)>>9];
							p->bG = m_aGammaB[(((t1.wG*bInv + m_aGammaF[p->bG]*bDir)>>22)*bInv2 + nGSelG*bDir2)>>9];
							p->bB = m_aGammaB[(((t1.wR*bInv + m_aGammaF[p->bB]*bDir)>>22)*bInv2 + nGSelB*bDir2)>>9];
						}
						++p;
						++pS;
					}
					p += szAdjustedTile.cx-szSourceTile.cx;
				}
			}
			else
			{
				bEverythingSelected = TRUE;
			}
		}
		if (bEverythingSelected)
		{
			TRasterImagePixel* p = const_cast<TRasterImagePixel*>(pSrcBuf);
			for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
			{
				int const yy = y>>nTransparencyShift;
				for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
				{
					if (p->bA != 255)
					{
						int const xx = x>>nTransparencyShift;
						TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
						ULONG const bDir = p->bA*0x4040+1;
						ULONG const bInv = 0x400000-bDir;
						p->bB = m_aGammaB[(t1.wB*bInv + m_aGammaF[p->bB]*bDir)>>22];
						p->bG = m_aGammaB[(t1.wG*bInv + m_aGammaF[p->bG]*bDir)>>22];
						p->bR = m_aGammaB[(t1.wR*bInv + m_aGammaF[p->bR]*bDir)>>22];
					}
					++p;
				}
				p += szAdjustedTile.cx-szSourceTile.cx;
			}
		}
		else if (bNothingSelected)
		{
			TRasterImagePixel* p = const_cast<TRasterImagePixel*>(pSrcBuf);
			for (int y = rcSourceTile.top; y < rcSourceTile.bottom; ++y)
			{
				int const yy = y>>nTransparencyShift;
				for (int x = rcSourceTile.left; x < rcSourceTile.right; ++x)
				{
					int const xx = x>>nTransparencyShift;
					if (p->bA == 255)
					{
						p->bB = m_aGammaB[(m_aGammaF[p->bB]+nGSelB)>>1];
						p->bG = m_aGammaB[(m_aGammaF[p->bG]+nGSelG)>>1];
						p->bR = m_aGammaB[(m_aGammaF[p->bR]+nGSelR)>>1];
					}
					else
					{
						TRasterImagePixel16 const t1 = aGTransparency[(xx^yy)&1];
						ULONG const bInv = 0x3fffff-p->bA*0x4040;
						ULONG const bDir = (0x400000-bInv)>>1;
						p->bB = m_aGammaB[(t1.wB*bInv + (m_aGammaF[p->bB] + nGSelB)*bDir)>>22];
						p->bG = m_aGammaB[(t1.wG*bInv + (m_aGammaF[p->bG] + nGSelG)*bDir)>>22];
						p->bR = m_aGammaB[(t1.wR*bInv + (m_aGammaF[p->bR] + nGSelR)*bDir)>>22];
					}
					++p;
				}
				p += szAdjustedTile.cx-szSourceTile.cx;
			}
		}
		TRasterImagePixel* pO = reinterpret_cast<TRasterImagePixel*>(a_pBuffer);
		//TRasterImagePixel* pI = cSrcBuf;
		CAutoVectorPtr<SFromToBlendN> cBlend(new SFromToBlendN[szTileInImg.cx]);
		ULONG const nCoverageX = 256.0f/fZoomX+0.5f;
		ULONG const nInvCovX = 0x1000000/nCoverageX;
		{
			// prepare guides for x-resizing
			for (int x = a_rcTileInImg.left; x < a_rcTileInImg.right; ++x)
			{
				SFromToBlendN& s = cBlend[x-a_rcTileInImg.left];
				int nX1 = 256.0f*x/fZoomX+0.5f;
				if (nX1&0xff == 0xff)
					++nX1; // optimize a bit (make less overlapping pixels)
				s.nW0 = 0x100-(nX1&0xff);
				int nWN = 0xff&(nX1+nCoverageX);
				if (nWN <= 1)
					nWN += 0x100;
				s.nFrom = (nX1>>8)-rcSourceTile.left;
				s.nFull = (nCoverageX-s.nW0-nWN)>>8;
				if (LONG(s.nFrom+s.nFull+2) > szSourceTile.cx)
					s.nFull = szSourceTile.cx-s.nFrom-2;
			}
		}
		//// resize each row
		//for (int y = 0; y < szSourceTile.cy; ++y)
		//{
		//	TRasterImagePixel* const pRow = cSrcBuf.m_p+y*szSourceTile.cx;
		//	TRasterImagePixel* pO = pRow;
		//	SFromToBlendN const* const pEnd = cBlend.m_p+szTileInImg.cx;
		//	for (SFromToBlendN const* pFTB = cBlend.m_p; pFTB != pEnd; ++pFTB, ++pO)
		//	{
		//		TRasterImagePixel* p0 = pRow+pFTB->nFrom;
		//		TRasterImagePixel* p1 = p0+1;
		//		TRasterImagePixel* pN = p1+pFTB->nFull;
		//		ULONG const nWN = nCoverageX-pFTB->nW0-(pFTB->nFull<<8);
		//		int nR = 0;
		//		int nG = 0;
		//		int nB = 0;
		//		while (p1 < pN)
		//		{
		//			nR += p1->bR;
		//			nG += p1->bG;
		//			nB += p1->bB;
		//			++p1;
		//		}
		//		pO->bR = (((nR<<8) + p0->bR*pFTB->nW0 + pN->bR*nWN)*nInvCovX + 0x800000)>>24;
		//		pO->bG = (((nG<<8) + p0->bG*pFTB->nW0 + pN->bG*nWN)*nInvCovX + 0x800000)>>24;
		//		pO->bB = (((nB<<8) + p0->bB*pFTB->nW0 + pN->bB*nWN)*nInvCovX + 0x800000)>>24;
		//	}
		//}
		ULONG const nCoverageY = 256.0f/fZoomY+0.5f;
		ULONG const nInvCovY = 0x1000000/nCoverageY;
		//for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride-szTileInImg.cx)
		//{
		//	int nY1 = 256.0f*y/fZoomY+0.5f;
		//	if (nY1&0xff == 0xff)
		//		++nY1; // optimize a bit (make less overlapping pixels)
		//	ULONG nW0 = 0x100-(nY1&0xff);
		//	ULONG nWN = 0xff&(nY1+nCoverageY);
		//	if (nWN <= 1)
		//		nWN += 0x100;
		//	ULONG nFrom = (nY1>>8)-rcSourceTile.top;
		//	ULONG nFull = (nCoverageY-nW0-nWN)>>8;
		//	if (nFrom+nFull+2 > szSourceTile.cy)
		//		nFull = szSourceTile.cy-nFrom-2;

		//	TRasterImagePixel* p0 = cSrcBuf.m_p+nFrom*szSourceTile.cx;
		//	TRasterImagePixel* p1 = p0+szSourceTile.cx;
		//	TRasterImagePixel* pN = p1+nFull*szSourceTile.cx;
		//	for (int x = 0; x < szTileInImg.cx; ++x, ++pO, ++p0, ++p1, ++pN)
		//	{
		//		int nR = 0;
		//		int nG = 0;
		//		int nB = 0;
		//		TRasterImagePixel* p2 = p1;
		//		while (p2 < pN)
		//		{
		//			nR += p2->bR;
		//			nG += p2->bG;
		//			nB += p2->bB;
		//			p2 += szSourceTile.cx;
		//		}
		//		pO->bR = (((nB<<8) + p0->bB*nW0 + pN->bB*nWN)*nInvCovY + 0x800000)>>24;
		//		pO->bG = (((nG<<8) + p0->bG*nW0 + pN->bG*nWN)*nInvCovY + 0x800000)>>24;
		//		pO->bB = (((nR<<8) + p0->bR*nW0 + pN->bR*nWN)*nInvCovY + 0x800000)>>24;
		//	}
		//}


		ULONG const nInvCov = 0x1000000/(nCoverageY*nCoverageX);
		CAutoVectorPtr<TRasterImagePixel32I> cRow(new TRasterImagePixel32I[szTileInImg.cx]);
		int nRow = -1;
		for (int y = a_rcTileInImg.top; y < a_rcTileInImg.bottom; ++y, pO += a_nStride-szTileInImg.cx)
		{
			int nY1 = 256.0f*y/fZoomY+0.5f;
			if (nY1&0xff == 0xff)
				++nY1; // optimize a bit (make less overlapping pixels)
			ULONG nW0 = 0x100-(nY1&0xff);
			ULONG nWN = 0xff&(nY1+nCoverageY);
			if (nWN <= 1)
				nWN += 0x100;
			ULONG nFrom = (nY1>>8)-rcSourceTile.top;
			ULONG nFull = (nCoverageY-nW0-nWN)>>8;
			if (LONG(nFrom+nFull+2) > szSourceTile.cy)
				nFull = szSourceTile.cy-nFrom-2;
			ULONG const nWY08 = nW0<<8;
			ULONG const nWYN8 = nWN<<8;

			if (nY1 == nRow)
			{
				// use the partially preprocessed row saved in previous step
			}
			else
			{
				TRasterImagePixel const* p0 = pSrcBuf+nFrom*szAdjustedTile.cx;
				TRasterImagePixel const* p1 = p0+szAdjustedTile.cx;
				TRasterImagePixel const* pN = p1+nFull*szAdjustedTile.cx;
				SFromToBlendN const* const pEnd = cBlend.m_p+szTileInImg.cx;
				for (SFromToBlendN const* pFTB = cBlend.m_p; pFTB != pEnd; ++pFTB, ++pO)
				{
					TRasterImagePixel const* p00 = p0+pFTB->nFrom;
					TRasterImagePixel const* p10 = p00+1;
					TRasterImagePixel const* pN0 = p10+pFTB->nFull;
					TRasterImagePixel const* p01 = p1+pFTB->nFrom;
					TRasterImagePixel const* p11 = p01+1;
					TRasterImagePixel const* pN1 = p11+pFTB->nFull;
					TRasterImagePixel const* p0N = pN+pFTB->nFrom;
					TRasterImagePixel const* p1N = p0N+1;
					TRasterImagePixel const* pNN = p1N+pFTB->nFull;
					ULONG const nWNX = nCoverageX-pFTB->nW0-(pFTB->nFull<<8);
					ULONG const nW00 = (pFTB->nW0*nW0)>>8;
					ULONG const nWN0 = (nWNX*nW0)>>8;
					ULONG const nW0N = (pFTB->nW0*nWN)>>8;
					ULONG const nWNN = (nWNX*nWN)>>8;
					ULONG const nWX08 = pFTB->nW0<<8;
					ULONG const nWXN8 = nWNX<<8;
					// first row
					TRasterImagePixel32I tT = {0, 0, 0};
					{
						TRasterImagePixel const* p2 = p10;
						while (p2 < pN0)
						{
							tT.nB += m_aGammaF[p2->bB];
							tT.nG += m_aGammaF[p2->bG];
							tT.nR += m_aGammaF[p2->bR];
							++p2;
						}
					}
					// last row
					TRasterImagePixel32I tB = {0, 0, 0};
					{
						TRasterImagePixel const* p2 = p1N;
						while (p2 < pNN)
						{
							tB.nB += m_aGammaF[p2->bB];
							tB.nG += m_aGammaF[p2->bG];
							tB.nR += m_aGammaF[p2->bR];
							++p2;
						}
					}
					// central region
					TRasterImagePixel32I tL = {0, 0, 0};
					TRasterImagePixel32I tR = {0, 0, 0};
					TRasterImagePixel32I tC = {0, 0, 0};
					{
						TRasterImagePixel const* p2R = p01;
						while (p2R < p0N)
						{
							tL.nB += m_aGammaF[p2R->bB];
							tL.nG += m_aGammaF[p2R->bG];
							tL.nR += m_aGammaF[p2R->bR];
							TRasterImagePixel const* p2 = p2R+1;
							TRasterImagePixel const* const p2N = p2+pFTB->nFull;
							while (p2 < p2N)
							{
								tC.nB += m_aGammaF[p2->bB];
								tC.nG += m_aGammaF[p2->bG];
								tC.nR += m_aGammaF[p2->bR];
								++p2;
							}
							tR.nB += m_aGammaF[p2->bB];
							tR.nG += m_aGammaF[p2->bG];
							tR.nR += m_aGammaF[p2->bR];
							p2R += szAdjustedTile.cx;
						}
					}
					pO->bB = m_aGammaB[(((tC.nB<<8) + (tT.nB*nW0) + (tB.nB*nWN) + (tL.nB*pFTB->nW0) + (tR.nB*nWNX) + m_aGammaF[p00->bB]*nW00 + m_aGammaF[p0N->bB]*nW0N + m_aGammaF[pN0->bB]*nWN0 + m_aGammaF[pNN->bB]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
					pO->bG = m_aGammaB[(((tC.nG<<8) + (tT.nG*nW0) + (tB.nG*nWN) + (tL.nG*pFTB->nW0) + (tR.nG*nWNX) + m_aGammaF[p00->bG]*nW00 + m_aGammaF[p0N->bG]*nW0N + m_aGammaF[pN0->bG]*nWN0 + m_aGammaF[pNN->bG]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
					pO->bR = m_aGammaB[(((tC.nR<<8) + (tT.nR*nW0) + (tB.nR*nWN) + (tL.nR*pFTB->nW0) + (tR.nR*nWNX) + m_aGammaF[p00->bR]*nW00 + m_aGammaF[p0N->bR]*nW0N + m_aGammaF[pN0->bR]*nWN0 + m_aGammaF[pNN->bR]*nWNN)/((nCoverageY*nCoverageX)>>8))];//*nInvCov + 0x800000)>>24;
				}
			}
		}
	}
	static void RenderTileInvert(ULONG a_nSizeX, ULONG a_nSizeY, TRasterImagePixel* a_pBuffer, ULONG a_nStride)
	{
		for (ULONG y = 0; y < a_nSizeY; ++y, a_pBuffer+=a_nStride-a_nSizeX)
			for (ULONG x = 0; x < a_nSizeX; ++x, ++a_pBuffer)
				if (a_pBuffer->bA == 0 && (a_pBuffer->bR || a_pBuffer->bG || a_pBuffer->bB))
					a_pBuffer->bA = 0x80;
	}

	void UpdateGammaTables()
	{
		float const ff = itemsof(m_aGammaF)-1;
		float const fb = itemsof(m_aGammaB)-1;
		for (ULONG i = 0; i < itemsof(m_aGammaF); ++i)
		{
			float const f = float(i)/ff;
			m_aGammaF[i] = fb * (f <= 0.04045f ? f/12.92f : powf((f+0.055f)/1.055f, 2.4f)) + 0.5f;
		}
		for (ULONG i = 0; i < itemsof(m_aGammaB); ++i)
		{
			float const f = float(i)/fb;
			m_aGammaB[i] = ff * (f <= 0.0031308f ? f*12.92f : powf(f, 1.0f/2.4f)*1.055f-0.055f) + 0.5f;
		}
	}

private:
	COLORREF M_Background() { return m_tBackground; }
	COLORREF M_Square1() { return m_tSquare1; }
	COLORREF M_Square2() { return m_tSquare2; }

	void InitPaintConstants()
	{
		HDC hDC = static_cast<T*>(this)->GetDC();
		int nDPI = GetDeviceCaps(hDC, LOGPIXELSX);
		static_cast<T*>(this)->ReleaseDC(hDC);
		float baseSize = M_BaseHandleSize();
		m_nHandleRadius = floorf(nDPI*(baseSize+0.5f)/96);
		float effectiveBaseSize = float(m_nHandleRadius)/nDPI*96-0.5f;
		if (m_nHandleRadius < 2) m_nHandleRadius = 2;
		ULONG size = m_nHandleRadius+m_nHandleRadius+1;
		m_pHandleImage.Free();
		m_pHandleImage.Attach(new BYTE[size*size*2]);
		CIconRendererReceiver cRenderer(size);
		static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
		static IRPathPoint const circle[] =
		{
			{256, 128, 0, -70.6925, 0, 70.6925},
			{128, 0, -70.6925, 0, 70.6925, 0},
			{0, 128, 0, 70.6925, 0, -70.6925},
			{128, 256, 70.6925, 0, -70.6925, 0},
		};
		static IRFill const white(0xffdddddd);
		static IRFill const black(0xff000000);
		IROutlinedFill const circleMat(&white, &black, 0.4f/(effectiveBaseSize+effectiveBaseSize+1), 0.4f);
		cRenderer(&canvas, itemsof(circle), circle, &circleMat);
		static IRPathPoint const shadow1[] =
		{
			{209.317, 46.6827, 44.9103, 44.9103, -9.34491, -9.34491},
			{209.317, 209.317, -44.9102, 44.9102, 44.9103, -44.9103},
			{46.6827, 209.317, -9.34491, -9.34491, 44.9102, 44.9103},
			{24.0553, 178.205, 36.262, 24.4362, 5.45592, 11.2584},
			{154.87, 154.87, 38.7582, -38.7582, -38.7582, 38.7582},
			{178.205, 24.0553, 11.2584, 5.45592, 24.4362, 36.262},
		};
		static IRFill const shadowMat(0x3f000000);
		cRenderer(&canvas, itemsof(shadow1), shadow1, &shadowMat);
		static IRPathPoint const shadow2[] =
		{
			{212.853, 43.1472, 46.863, 46.8629, 0, 0},
			{212.853, 212.853, -46.8629, 46.8629, 46.863, -46.8629},
			{43.1472, 212.853, 0, 0, 46.8629, 46.8629},
			{42.4401, 212.146, 47.3213, 22.9391, 0, 0},
			{187.397, 187.397, 39.2802, -39.2802, -39.2802, 39.2802},
			{212.146, 42.4401, 0, 0, 22.9391, 47.3213},
		};
		cRenderer(&canvas, itemsof(shadow2), shadow2, &shadowMat);
		static IRPathPoint const hilight1[] =
		{
			{110, 40, -10.9347, -10.9347, 10.9347, 10.9347},
			{55, 55, -19.1357, 19.1357, 19.1357, -19.1357},
			{40, 110, 10.9347, 10.9347, -10.9347, -10.9347},
			{95, 95, 19.1357, -19.1357, -19.1357, 19.1357},
		};
		static IRFill const hilightMat(0x3fffffff);
		cRenderer(&canvas, itemsof(hilight1), hilight1, &hilightMat);
		static IRPathPoint const hilight2[] =
		{
			{87, 56, -4.68629, -4.68629, 4.68629, 4.68629},
			{63, 63, -8.59154, 8.59153, 8.59154, -8.59153},
			{56, 87, 4.68629, 4.68629, -4.68629, -4.68629},
			{80, 80, 8.59154, -8.59153, -8.59154, 8.59153},
		};
		cRenderer(&canvas, itemsof(hilight2), hilight2, &hilightMat);
		for (ULONG y = 0; y < size; ++y)
		{
			DWORD const* pS = cRenderer.pixelRow(y);
			BYTE* pD = m_pHandleImage.m_p+2*size*y;
			for (ULONG x = 0; x < size; ++x, ++pS, pD+=2)
			{
				pD[0] = (*pS)>>24;
				pD[1] = (*pS)&0xff;
			}
		}
	}

private:
	CComPtr<IRasterImageComposedPreview> m_pComposedPreview;
	bool m_bComposedPreviewUpdates;
	CComPtr<IThreadPool> m_pThPool;

	COLORREF m_tBackground; // color of the window; outside the canvas
COLORREF m_tSelection;
	COLORREF m_tSquare1;
	COLORREF m_tSquare2;
	COLORREF m_tGrid;
	WORD m_aGammaF[256]; // 8bits->10bits
	BYTE m_aGammaB[1024]; // 10bits->8bits

	// zooming and scrolling
	bool m_bUpdating;
	float m_fOffsetX; // when 0, center of image in is center of screen
	float m_fOffsetY; // when 1, right/bottom of image is in center of screen
	SIZE m_tImageSize;
	SIZE m_tZoomedSize;
	SIZE m_tSizeWithBorder;
	POINT m_ptImagePos;
	SIZE m_tClientSize; // size of the window without borders (updated via WM_SIZE)
	bool m_bScrollBarH;
	bool m_bScrollBarV;

	// image rendering options
	bool m_bShowComposed;
	EComposedPreviewMode m_eComposedMode;
	bool m_bShowInverted;
	EImageQuality m_eImageQuality;
	float m_fZoom;
	bool m_bAutoZoom;
	LONG m_eGridSetting;
	LONG m_eStyle;
	float m_fBaseHandleSize;
	bool m_bHideOnLeave;
	TMatrix3x3f m_tInputTrans;
	TMatrix3x3f m_tInputInvTrans;

	// cached visualization values
	ULONG m_nHandleRadius;
	CAutoVectorPtr<BYTE> m_pHandleImage;

	bool m_bUpdateWindow;

	COLORREF MARGIN_CACHE_LT[MARGIN_TOP*MARGIN_LEFT];
	COLORREF MARGIN_CACHE_RT[MARGIN_TOP*MARGIN_RIGHT];
	COLORREF MARGIN_CACHE_LB[MARGIN_BOTTOM*MARGIN_LEFT];
	COLORREF MARGIN_CACHE_RB[MARGIN_BOTTOM*MARGIN_RIGHT];
	COLORREF MARGIN_CACHE_L[MARGIN_LEFT];
	COLORREF MARGIN_CACHE_T[MARGIN_TOP];
	COLORREF MARGIN_CACHE_R[MARGIN_RIGHT];
	COLORREF MARGIN_CACHE_B[MARGIN_BOTTOM];
	LONG MARGIN_CACHE_LASTMODE;
	//CComPtr<IRasterImageRendering> m_pRendering;

	CComPtr<IConfig> m_pConfig;

	CComPtr<ISharedStateManager> m_pSSM;
	CComBSTR m_bstrIES;

#ifdef PERFMON
	CComPtr<IPerformanceMonitor> m_pPerfMon;
	ULONG m_nCounterRedraw;
	ULONG m_nCounterObtain;
	LONGLONG freq;
#endif
};

