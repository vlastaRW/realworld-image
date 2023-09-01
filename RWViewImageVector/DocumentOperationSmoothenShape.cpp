
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"
#include "PathUtils.h"
#include <RWProcessingTags.h>
#include <IconRenderer.h>
#include "../RWDrawing/BezierDistance.h"


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_TOLERANCE[] = L"Tolerance";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISmoothenShapeDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUISmoothenShapeDlg>,
	public CDialogResize<CConfigGUISmoothenShapeDlg>
{
public:
	CConfigGUISmoothenShapeDlg()
	{
	}

	enum { IDC_EDIT = 100, IDC_SLIDER, IDC_SPIN, IDC_UNIT };

	BEGIN_DIALOG_EX(0, 0, 120, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Tolerance:[0405]Tolerance:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_EDIT, 86, 0, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 109, 0, 11, 12, 0)
		CONTROL_CONTROL(_T(""), IDC_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 0, 41, 12, 0)
		//CONTROL_EDITTEXT(IDC_EDIT, 78, 0, 34, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		//CONTROL_CONTROL(_T(""), IDC_SPIN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 101, 0, 11, 12, 0)
		//CONTROL_CONTROL(_T(""), IDC_SLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 45, 0, 33, 12, 0)
		//CONTROL_RTEXT(_T("px"), IDC_UNIT, 112, 2, 8, 8, WS_VISIBLE, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUISmoothenShapeDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISmoothenShapeDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUISmoothenShapeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		//MESSAGE_HANDLER(WM_RW_CFGSPLIT, OnRWCfgSplit)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISmoothenShapeDlg)
		DLGRESIZE_CONTROL(IDC_SLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_EDIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_SPIN, DLSZ_MOVE_X)
		//DLGRESIZE_CONTROL(IDC_UNIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISmoothenShapeDlg)
		//CONFIGITEM_SLIDER_TRACKUPDATE(IDC_SLIDER, CFGID_TOLERANCE)
		CONFIGITEM_EDITBOX(IDC_EDIT, CFGID_TOLERANCE)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
	//LRESULT OnRWCfgSplit(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
	//{
	//	if (a_lParam)
	//		*reinterpret_cast<float*>(a_lParam) = 1.0f;
	//	return 0;
	//}

	void ExtraConfigNotify()
	{
		if (m_scrollbar.m_hWnd)
		{
			CConfigValue tolerance;
			M_Config()->ItemValueGet(CComBSTR(CFGID_TOLERANCE), &tolerance);
			float val = tolerance.operator float() <= 0.5f ? 0.0f : (tolerance.operator float() >= 50.0f ? 100.0f : (logf(tolerance.operator float()*2)*100.0f/logf(100.0f)));
			m_scrollbar.SetPos(val);
		}
	}
	void ExtraInitDialog()
	{
		m_scrollbar = GetDlgItem(IDC_SLIDER);
		m_scrollbar.SetRange(0, 100);
	}

	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (reinterpret_cast<HWND>(a_lParam) == m_scrollbar.m_hWnd)
		{
			CComBSTR bstr(CFGID_TOLERANCE);
			CConfigValue tolerance;
			M_Config()->ItemValueGet(bstr, &tolerance);
			float n = exp(m_scrollbar.GetPos()/100.0f*logf(100.0f))*0.5f;
			if (n < 1)
				n = int(n*1000.0f+0.5f)/1000.0f;
			else if (n < 10)
				n = int(n*100.0f+0.5f)/100.0f;
			else
				n = int(n*10.0f+0.5f)/10.0f;
			M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(n));
		}
		else
		{
			a_bHandled = FALSE;
		}
		return 0;
	}

public:
	CTrackBarCtrl m_scrollbar;
};

#include <IconRenderer.h>
HICON GetIconSmoothenShape(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const aOffset[] =
	{
		{65.219, 4.82858, 35.781, -8.82858, -42.4643, 10.4776},
		{149, 50, 9.65461, 31.1093, -9, -29},
		{216.219, 123.829, 61, 22, -58.0262, -20.9275},
		{204, 247, -56.4358, 16.1245, 56, -16},
		{14.219, 165.829, -33.2804, -82.1609, 32, 79},
	};
	static IRPath const tOffset = {itemsof(aOffset), aOffset};
	static IRPathPoint const aOriginal[] =
	{
		{118.219, 91.8286, 13, 65, -8.73763, -43.6882},
		{212, 190, -4.13442, 31.3035, 7, -53},
		{83.219, 183.829, -40, -30, 40.6428, 30.4821},
		{57.219, 64.8286, 18.6364, -33.1313, -18, 32},
	};
	static IRPath const tOriginal = {itemsof(aOriginal), aOriginal};
	static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&tCanvas, 1, &tOffset, pSI->GetMaterial(ESMAltBackground));//ESMScheme1Color3));
	cRenderer(&tCanvas, 1, &tOriginal, pSI->GetMaterial(ESMScheme1Color2));//ESMInterior));
	return cRenderer.get();
}


// CVectorImageSmoothenShape

// {93692B3B-E1A7-44f1-ADD8-E6FD8335D973}
extern GUID const CLSID_VectorImageSmoothenShape = {0x93692b3b, 0xe1a7, 0x44f1, {0xad, 0xd8, 0xe6, 0xfd, 0x83, 0x35, 0xd9, 0x73}};

class ATL_NO_VTABLE CVectorImageSmoothenShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CVectorImageSmoothenShape, &CLSID_VectorImageSmoothenShape>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter
{
public:
	CVectorImageSmoothenShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CVectorImageSmoothenShape)

BEGIN_CATEGORY_MAP(CVectorImageSmoothenShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageRearragement)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CVectorImageSmoothenShape)
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
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Approximate object[0405]Vektorový obrázek - aproximovat objekt");
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

			CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
			pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_TOLERANCE), CMultiLanguageString::GetAuto(L"[0409]Tolerance[0405]Tolerance"), NULL, CConfigValue(5.0f), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_VectorImageSmoothenShape, CConfigGUISmoothenShapeDlg>::FinalizeConfig(pCfgInit);

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
			if (pDVI == NULL)
				return S_FALSE;
			if (a_pConfig == NULL)
				return S_OK;
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			ULONG nSelected = 0;
			pDVI->StateUnpack(pState, &CEnumItemCounter<IEnum2UInts, ULONG>(&nSelected));
			return nSelected ? S_OK : S_FALSE;
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
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return E_FAIL;
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			std::vector<ULONG> cIDs;
			if (a_pStates)
			{
				a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			}
			if (cIDs.empty())
				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				return S_FALSE;

			CConfigValue cSmoothing;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_TOLERANCE), &cSmoothing);
			float fSmoothing = cSmoothing;

			CComObjectStackEx<CToolWindow> cWnd;

			CComPtr<IRasterImageEditToolsManager> pToolMgr;
			RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));

			CWriteLock<IBlockOperations> cLock(a_pDocument);

			for (std::vector<ULONG>::const_iterator i = cIDs.begin(); i != cIDs.end(); ++i)
			{
				ULONG objectID = *i;

				{
					CComBSTR bstrID;
					CComBSTR bstrParams;
					HRESULT hRes = pDVI->ObjectGet(objectID, &bstrID, &bstrParams);

					std::vector<std::vector<TPixelCoords> > cOriginal;
					if (FAILED(hRes)) continue;
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

						CPolyReceiver cReceiver(cOriginal);
						pToolPoly->ToPolygon(&cReceiver);
					}
					if (cOriginal.empty())
						continue;

					std::vector<TRWPath> aRes;
					aRes.reserve(cOriginal.size());
					std::vector<std::vector<TRWPathPoint> > cache;
					cache.resize(cOriginal.size());

					for (std::vector<std::vector<TPixelCoords> >::const_iterator j = cOriginal.begin(); j != cOriginal.end(); ++j)
					{
						std::vector<TVector2d> pts;
						for (std::vector<TPixelCoords>::const_iterator k = j->begin(); k != j->end(); ++k)
						{
							TVector2d t(k->fX, k->fY);
							if (!pts.empty())
							{
								TVector2d const& t2 = pts[pts.size()-1];
								if (t2.x == t.x && t2.y == t.y)
									continue;
							}
							pts.push_back(t);
						}
						std::vector<TRWPathPoint>& dst = cache[aRes.size()];
						Bezier::FitCurve(pts, 0, pts.size(), fSmoothing, CFitCurveConsumer(dst));
						if (dst.empty())
							continue;
						aRes.resize(aRes.size()+1);
						aRes[aRes.size()-1].nVertices = dst.size();
						aRes[aRes.size()-1].pVertices = &(dst[0]);
					}

					if (aRes.empty())
						continue;

					CComBSTR bstrNewID(L"SHAPE");
					CComBSTR bstrNewParams;
					{
						CComPtr<IRasterImageEditTool> pTool;
						pToolMgr->EditToolCreate(bstrNewID, NULL, &pTool);
						CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
						CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
						pTool->Init(&cWnd);
						pToolPoly->FromPath(aRes.size(), &(aRes[0]));
						pToolScript->ToText(&bstrNewParams);
					}

					ECoordinatesMode eCM = ECMFloatingPoint;
					pDVI->ObjectStateSet(objectID, NULL, NULL, &eCM, NULL, NULL, NULL, NULL, NULL); // usually needed, otherwise points are moved
					hRes = pDVI->ObjectSet(&objectID, bstrNewID, bstrNewParams);
				}
			}
			return S_OK;
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
		if (a_ppName == NULL) return E_POINTER;
		try
		{
			//if (a_pConfig)
			//{
			//	CConfigValue cOffset;
			//	a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSET), &cOffset);
			//	float fOffset = cOffset;
			//	if (fOffset > 0.0f)
			//	{
			//		CComObject<CPrintfLocalizedString>* pPF = NULL;
			//		CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
			//		CComPtr<ILocalizedString> pOut = pPF;
			//		int off = int(fOffset+0.5f);
			//		if (off < 1) off = 1;
			//		pPF->Init(CMultiLanguageString::GetAuto(L"[0409]Grow object - %ipx[0405]Zvětšit objekt - %ipx"), off);
			//		*a_ppName = pOut.Detach();
			//		return S_OK;
			//	}
			//	else if (fOffset < 0.0f)
			//	{
			//		CComObject<CPrintfLocalizedString>* pPF = NULL;
			//		CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
			//		CComPtr<ILocalizedString> pOut = pPF;
			//		int off = int(0.5f-fOffset);
			//		if (off < 1) off = 1;
			//		pPF->Init(CMultiLanguageString::GetAuto(L"[0409]Shrink object - %ipx[0405]Zmenšit objekt - %ipx"), off);
			//		*a_ppName = pOut.Detach();
			//		return S_OK;
			//	}
			//}
			*a_ppName = new CMultiLanguageString(L"[0409]Approximate object[0405]Aproximovat objekt");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	//STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon)
	//{
	//	if (a_phIcon == NULL)
	//		return E_POINTER;
	//	try
	//	{
	//		*a_phIcon = GetIconSmoothenShape(a_nSize);
	//		return S_OK;
	//	}
	//	catch (...)
	//	{
	//		return E_UNEXPECTED;
	//	}
	//}

	// IRasterImageFilter methods
public:
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		return S_OK;
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		return AdjustDirtyRect(a_pConfig, a_pCanvas, a_pRect);
	}
	STDMETHOD(Process)(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix)
	{
		//CComPtr<IDocumentFactoryVectorImage> pVF;
		//RWCoCreateInstance(pVF, __uuidof(DocumentFactoryVectorImage));
		a_pSrc->DocumentCopy(a_bstrPrefix, a_pDst, NULL, NULL);
		CComPtr<IDocument> pDoc;
		a_pDst->DataBlockDoc(a_bstrPrefix, &pDoc);
		return Activate(NULL, pDoc, a_pConfig, NULL, NULL, MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), SORT_DEFAULT));
	}

private:
	struct CFitCurveConsumer
	{
		std::vector<TRWPathPoint>& path;
		bool first;
		TRWPathPoint point;
		CFitCurveConsumer(std::vector<TRWPathPoint>& path) : path(path), first(true) { point.dwFlags = 0; }
		void add(TVector2d const* curve)
		{
			if (first)
			{
				TRWPathPoint t;
				t.tPos = curve[0];
				t.tTanPrev.fX = t.tTanPrev.fY = 0.0f;
				t.dwFlags = 0;
				t.tTanNext = curve[1]-curve[0];
				path.push_back(t);
				first = false;
			}
			else
			{
				point.tTanNext = curve[1]-curve[0];
				path.push_back(point);
			}
			point.tPos = curve[3];
			point.tTanPrev = curve[2]-curve[3];
		}
		~CFitCurveConsumer()
		{
			if (!first)
			{
				point.tTanNext.fX = point.tTanNext.fY = 0.0f;
				path.push_back(point);
			}
		}
	};

	class CPolyReceiver : public CStackUnknown<IRasterImageEditToolPolygon>
	{
	public:
		CPolyReceiver(std::vector<std::vector<TPixelCoords> >& polyPaths) : polyPaths(polyPaths)
		{
		}

		// IRasterImageEditToolPolygon methods
	public:
		STDMETHOD(FromPolygon)(ULONG a_nCount, TRWPolygon const* a_pPolygons)
		{
			try
			{
				if (a_nCount == 0)
					return S_FALSE;

				size_t i = polyPaths.size();
				polyPaths.resize(i+a_nCount);
				for (TRWPolygon const* pE = a_pPolygons+a_nCount; a_pPolygons != pE; ++a_pPolygons)
					polyPaths[i].assign(a_pPolygons->pVertices, a_pPolygons->pVertices+a_pPolygons->nVertices);

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }
		STDMETHOD(FromPath)(ULONG UNREF(a_nCount), TRWPath const* UNREF(a_pPaths)) { return E_NOTIMPL; }
		STDMETHOD(ToPath)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }

	private:
		std::vector<std::vector<TPixelCoords> >& polyPaths;
	};

};

OBJECT_ENTRY_AUTO(CLSID_VectorImageSmoothenShape, CVectorImageSmoothenShape)
