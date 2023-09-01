
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"
#include <RWDocumentImageRasterUtils.h>
#include "PathUtils.h"
#include <RWProcessingTags.h>
#include <IconRenderer.h>


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_OFFSET[] = L"Offset";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIOffsetShapeDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIOffsetShapeDlg>,
	public CDialogResize<CConfigGUIOffsetShapeDlg>
{
public:
	enum
	{
		IDC_OFFSET_EDIT = 100,
		IDC_OFFSET_UPDOWN,
	};

	BEGIN_DIALOG_EX(0, 0, 100, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Offset:[0405]Odsazení:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_OFFSET_EDIT, 50, 0, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_OFFSET_UPDOWN, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 90, 0, 10, 12, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIOffsetShapeDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIOffsetShapeDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIOffsetShapeDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIOffsetShapeDlg)
		DLGRESIZE_CONTROL(IDC_OFFSET_EDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_OFFSET_UPDOWN, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIOffsetShapeDlg)
		CONFIGITEM_EDITBOX(IDC_OFFSET_EDIT, CFGID_OFFSET)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		CUpDownCtrl wnd(GetDlgItem(IDC_OFFSET_UPDOWN));
		wnd.SetRange(-100, 100);

		return 1;
	}

};

#include <IconRenderer.h>
HICON GetIconOffsetShape(ULONG a_nSize)
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


// CVectorImageOffsetShape

// {2B0D9F5B-7DF6-4c53-A0A2-08B90D8BD995}
extern GUID const CLSID_VectorImageOffsetShape = {0x2b0d9f5b, 0x7df6, 0x4c53, {0xa0, 0xa2, 0x8, 0xb9, 0xd, 0x8b, 0xd9, 0x95}};

class ATL_NO_VTABLE CVectorImageOffsetShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CVectorImageOffsetShape, &CLSID_VectorImageOffsetShape>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter
{
public:
	CVectorImageOffsetShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CVectorImageOffsetShape)

BEGIN_CATEGORY_MAP(CVectorImageOffsetShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagImageRearragement)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CVectorImageOffsetShape)
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
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Offset object[0405]Vektorový obrázek - odsadit objekt");
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

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_OFFSET), CMultiLanguageString::GetAuto(L"[0409]Offset[0405]Odsazení"), CMultiLanguageString::GetAuto(L"[0409]Positive value moves object edge out.[0405]Pozitivní hodnota posune hranu objektu ven."), CConfigValue(0.0f), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_VectorImageOffsetShape, CConfigGUIOffsetShapeDlg>::FinalizeConfig(pCfgInit);

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

			CConfigValue cOffset;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSET), &cOffset);
			float fOffset = cOffset;
			if (fOffset == 0.0f)
				return S_FALSE;

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

					std::vector<std::vector<TPixelCoords> > cResult;
					ShrinkPolygon(cOriginal, -fOffset, cResult);

					CComBSTR bstrNewID(L"SHAPE");
					CComBSTR bstrNewParams;
					if (cResult.empty())
					{
						hRes = pDVI->ObjectSet(&objectID, NULL, NULL);
					}
					else
					{
						CComPtr<IRasterImageEditTool> pTool;
						pToolMgr->EditToolCreate(bstrNewID, NULL, &pTool);
						CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
						CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
						pTool->Init(&cWnd);
						std::vector<TRWPolygon> aRes;
						aRes.resize(cResult.size());
						for (size_t i = 0; i < cResult.size(); ++i)
						{
							aRes[i].nVertices = cResult[i].size();
							aRes[i].pVertices = &(cResult[i][0]);
						}
						pToolPoly->FromPolygon(aRes.size(), &(aRes[0]));
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
			if (a_pConfig)
			{
				CConfigValue cOffset;
				a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSET), &cOffset);
				float fOffset = cOffset;
				if (fOffset > 0.0f)
				{
					CComObject<CPrintfLocalizedString>* pPF = NULL;
					CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
					CComPtr<ILocalizedString> pOut = pPF;
					int off = int(fOffset+0.5f);
					if (off < 1) off = 1;
					pPF->Init(CMultiLanguageString::GetAuto(L"[0409]Grow object - %ipx[0405]Zvětšit objekt - %ipx"), off);
					*a_ppName = pOut.Detach();
					return S_OK;
				}
				else if (fOffset < 0.0f)
				{
					CComObject<CPrintfLocalizedString>* pPF = NULL;
					CComObject<CPrintfLocalizedString>::CreateInstance(&pPF);
					CComPtr<ILocalizedString> pOut = pPF;
					int off = int(0.5f-fOffset);
					if (off < 1) off = 1;
					pPF->Init(CMultiLanguageString::GetAuto(L"[0409]Shrink object - %ipx[0405]Zmenšit objekt - %ipx"), off);
					*a_ppName = pOut.Detach();
					return S_OK;
				}
			}
			*a_ppName = new CMultiLanguageString(L"[0409]Offset object[0405]Odsadit objekt");
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
			*a_phIcon = GetIconOffsetShape(a_nSize);
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
		if (a_pConfig && a_pRect)
		{
			CConfigValue cOffset;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OFFSET), &cOffset);
			float fOffset = cOffset;
			if (fOffset == 0.0f)
				return S_FALSE;

			int nOff = ceilf(fabsf(fOffset));

			a_pRect->tTL.nX -= nOff;
			a_pRect->tTL.nY -= nOff;
			a_pRect->tBR.nX += nOff;
			a_pRect->tBR.nY += nOff;
		}
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

	static bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, EOutlineJoinType a_eJoinType, std::vector<std::vector<TPixelCoords> >& a_cOutput)
	{
		ClipperLib::Paths cSrc;
		cSrc.resize(a_cInput.size());
		for (std::vector<std::vector<TPixelCoords> >::const_iterator i = a_cInput.begin(); i != a_cInput.end(); ++i)
		{
			ClipperLib::Path& cSrc1 = cSrc[i-a_cInput.begin()];
			cSrc1.reserve(i->size());
			for (std::vector<TPixelCoords>::const_iterator j = i->begin(); j != i->end(); ++j)
			{
				ClipperLib::IntPoint pt(j->fX*256.0+0.5, j->fY*256.0+0.5);
				cSrc1.push_back(pt);
			}
		}

		ClipperLib::JoinType jt = a_eJoinType == EOJTMiter ? ClipperLib::jtMiter : (a_eJoinType == EOJTBevel ? ClipperLib::jtSquare : ClipperLib::jtRound);
		ClipperLib::Paths cDst;
		ClipperLib::ClipperOffset cOff(4.0f);
		cOff.AddPaths(cSrc, jt, ClipperLib::etClosedPolygon);
		cOff.Execute(cDst, -256.0*a_fAmount);
		//ClipperLib::OffsetPolygons(cSrc, cDst, -1000.0*a_fAmount, ClipperLib::jtRound);

		a_cOutput.resize(cDst.size());
		for (ClipperLib::Paths::const_iterator i = cDst.begin(); i != cDst.end(); ++i)
		{
			std::vector<TPixelCoords>& cOut1 = a_cOutput[i-cDst.begin()];
			cOut1.reserve(i->size());
			for (ClipperLib::Path::const_iterator j = i->begin(); j != i->end(); ++j)
			{
				TPixelCoords pt = {j->X*0.00390625f, j->Y*0.00390625f};
				cOut1.push_back(pt);
			}
		}
		return true;
	}

	static bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, std::vector<std::vector<TPixelCoords> >& a_cOutput)
	{
		return ShrinkPolygon(a_cInput, a_fAmount, EOJTRound, a_cOutput);
	}

};

OBJECT_ENTRY_AUTO(CLSID_VectorImageOffsetShape, CVectorImageOffsetShape)
