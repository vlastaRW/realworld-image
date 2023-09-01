
#include "stdafx.h"
#include <RWDrawing.h>
#include <MultiLanguageString.h>
#include "RWDocumentImageVector.h"
#include <RWBaseEnumUtils.h>
#include "PathUtils.h"
#include <SharedStateUndo.h>


// CDocumentOperationCombineShapes

extern GUID const CLSID_DocumentOperationCombineShapes = {0x658eddd9, 0x3ec3, 0x439f, {0x85, 0x83, 0xe, 0x5c, 0x4d, 0xc, 0x8e, 0x75}};
static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_OPERATION[] = L"Operation";
static LONG const CFGVAL_COP_SMART = 0;
static LONG const CFGVAL_COP_UNION = 1;
static LONG const CFGVAL_COP_INTERSECTION = 2;
static LONG const CFGVAL_COP_DIFFERENCE = 3;

class ATL_NO_VTABLE CDocumentOperationCombineShapes :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationCombineShapes, &CLSID_DocumentOperationCombineShapes>,
	public IDocumentOperation
{
public:
	CDocumentOperationCombineShapes()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationCombineShapes)

BEGIN_CATEGORY_MAP(CDocumentOperationCombineShapes)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationCombineShapes)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Combine Shapes[0405]Vektorový obrázek - sloučit Tvary");
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

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);
			CComBSTR cCFGID_OPERATION(CFGID_OPERATION);
			pCfgInit->ItemIns1ofN(cCFGID_OPERATION, CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Defines how to combine selected shapes.[0405]Určuje, jakým způsobem zkombinovat vybrané tvary."), CConfigValue(CFGVAL_COP_SMART), NULL);
			pCfgInit->ItemOptionAdd(cCFGID_OPERATION, CConfigValue(CFGVAL_COP_SMART), CMultiLanguageString::GetAuto(L"[0409]Default[0405]Výchozí"), 0, NULL);
			pCfgInit->ItemOptionAdd(cCFGID_OPERATION, CConfigValue(CFGVAL_COP_UNION), CMultiLanguageString::GetAuto(L"[0409]Union[0405]Sjednocení"), 0, NULL);
			pCfgInit->ItemOptionAdd(cCFGID_OPERATION, CConfigValue(CFGVAL_COP_INTERSECTION), CMultiLanguageString::GetAuto(L"[0409]Intersection[0405]Průnik"), 0, NULL);
			pCfgInit->ItemOptionAdd(cCFGID_OPERATION, CConfigValue(CFGVAL_COP_DIFFERENCE), CMultiLanguageString::GetAuto(L"[0409]Difference[0405]Rozdíl"), 0, NULL);

			// finalize the initialization of the config
			pCfgInit->Finalize(NULL);
			//CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

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
			std::vector<ULONG> aSelected;
			pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
			if (aSelected.size() < 2)
				return S_FALSE;
			bool none = true;
			for (std::vector<ULONG>::const_iterator i = aSelected.begin(); i != aSelected.end(); ++i)
			{
				CComBSTR bstrToolID;
				pDVI->ObjectGet(*i, &bstrToolID, NULL);
				// TODO: use actual tool properties when tools are redesigned
				if (bstrToolID == L"SHAPE" || bstrToolID == L"TEXT" || bstrToolID == L"POLYGON" ||
					bstrToolID == L"ELLIPSE" || bstrToolID == L"RECTANGLE")
				{
					if (none)
						none = false;
					else
						return S_OK;
				}
			}
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
			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return E_FAIL;
			CConfigValue cOpType;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION), &cOpType);
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);

			CWriteLock<IBlockOperations> cLock(a_pDocument);

			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> aSelected;
			pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
			if (aSelected.size() < 2)
				return E_FAIL;

			std::vector<ULONG> aAll;
			pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aAll));

			{
				// reorder selection bottom to top
				std::set<ULONG> old(aSelected.begin(), aSelected.end());
				aSelected.clear();
				for (std::vector<ULONG>::const_iterator i = aAll.begin(); i != aAll.end(); ++i)
					if (old.find(*i) != old.end())
						aSelected.push_back(*i);
			}

			CComBSTR bstrPathID(L"SHAPE");
			CComBSTR bstrPathParams;
			CComObjectStackEx<CToolWindow> cWnd;

			CComPtr<IRasterImageEditToolsManager> pToolMgr;
			RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));

			CBezierPaths srcBeziers;
			ClipperLib::Paths cDst;

			ULONG mainObjectID = 0;
			std::vector<ULONG> toDelete;

			for (std::vector<ULONG>::const_iterator sel = aSelected.begin(); sel != aSelected.end(); ++sel)
			{
				ULONG objectID = *sel;

				ClipperLib::Paths srcPolys;

				{
					CComBSTR bstrID;
					CComBSTR bstrParams;
					HRESULT hRes = pDVI->ObjectGet(objectID, &bstrID, &bstrParams);
					if (FAILED(hRes)) continue;
					CComPtr<IRasterImageEditTool> pTool;
					pToolMgr->EditToolCreate(bstrID, NULL, &pTool);
					if (pTool == NULL) continue;
					CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
					if (pToolScript == NULL) continue;
					CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
					if (pToolPoly == NULL) continue;
					pTool->Init(&cWnd);
					pToolScript->FromText(bstrParams);

					CPolygonReceiver cReceiver(srcBeziers, srcPolys);
					pToolPoly->ToPath(&cReceiver);
				}

				if (sel == aSelected.begin())
				{
					std::swap(cDst, srcPolys);
					mainObjectID = objectID;
					continue;
				}

				toDelete.push_back(objectID);

				ClipperLib::ClipType clipType = ClipperLib::ctUnion;
				switch (cOpType.operator LONG())
				{
				case CFGVAL_COP_UNION:
					break;
				case CFGVAL_COP_INTERSECTION:
					clipType = ClipperLib::ctIntersection;
					break;
				case CFGVAL_COP_DIFFERENCE:
					clipType = ClipperLib::ctXor;
					break;
				default:
					{
						CComBSTR objStyle;
						CComBSTR objStyleParams;
						pDVI->ObjectStyleGet(objectID, &objStyle, &objStyleParams);
						float f[4] = {1.0f, 1.0f, 1.0f, 1.0f};
						if (objStyle == L"SOLID" && objStyleParams.Length() &&
							4 == swscanf(objStyleParams, L"%f,%f,%f,%f", f, f+1, f+2, f+3) && f[3] == 0)
							clipType = ClipperLib::ctDifference;
					}
					break;
				}

				ClipperLib::Clipper c;
				c.ZFillFunction(&CoordZFill);
				c.StrictlySimple(true);
				c.AddPaths(cDst, ClipperLib::ptSubject, true);
				c.AddPaths(srcPolys, ClipperLib::ptClip, true);
				c.Execute(clipType, cDst, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
			}

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
				CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrState, pState);
				a_pStates->StateSet(bstrState, NULL);

				for (std::vector<ULONG>::const_iterator i = aSelected.begin(); i != aSelected.end(); ++i)
				{
					ULONG n = *i;
					pDVI->ObjectSet(&n, NULL, NULL);
				}
			}
			else
			{
				CComBSTR toolID;
				CComBSTR toolParams;
				GetToolParams(cWnd, pToolMgr, buffer, &toolID, &toolParams);

				pDVI->ObjectSet(&mainObjectID, toolID, toolParams);

				CComPtr<ISharedState> pNewState;
				pDVI->StatePack(1, &mainObjectID, &pNewState);
				if (pNewState)
				{
					CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrState, pState);
					a_pStates->StateSet(bstrState, pNewState);
				}

				for (std::vector<ULONG>::const_iterator i = toDelete.begin(); i != toDelete.end(); ++i)
				{
					ULONG n = *i;
					pDVI->ObjectSet(&n, NULL, NULL);
				}
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationCombineShapes, CDocumentOperationCombineShapes)

#include <IconRenderer.h>
HICON GetIconCombine(ULONG a_nSize, LONG a_nType)
{
	static IRPathPoint const uni[] =
	{
		{176, 67, 36.9302, 10.4507, 0, 0},
		{240, 152, 0, 48.6011, 0, -40.2767},
		{152, 240, -40.2767, 0, 48.6011, 0},
		{67, 176, 0, 0, 10.4507, 36.9302},
		{16, 176, 0, 0, 0, 0},
		{16, 16, 0, 0, 0, 0},
		{176, 16, 0, 0, 0, 0},
	};
	static IRPathPoint const intr[] =
	{
		{176, 67, 0, 0, -7.63272, -2.15994},
		{176, 176, 0, 0, 0, 0},
		{67, 176, -2.15994, -7.63272, 0, 0},
		{64, 152, 0, -48.6011, 0, 8.32437},
		{152, 64, 8.32437, 0, -48.6011, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRGridItem const grid[] = { {0, 16.0f}, {0, 176.0f} };
	IRCanvas canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(uni), uni, pSI->GetMaterial(a_nType == CFGVAL_COP_INTERSECTION ? ESMInterior : ESMScheme1Color2));
	if (a_nType != CFGVAL_COP_UNION)
		cRenderer(&canvas, itemsof(intr), intr, pSI->GetMaterial(a_nType == CFGVAL_COP_INTERSECTION ? ESMScheme1Color2 : ESMInterior));
	return cRenderer.get();
}

HICON GetIconCombineUnion(ULONG a_nSize) { return GetIconCombine(a_nSize, CFGVAL_COP_UNION); }
HICON GetIconCombineIntersection(ULONG a_nSize) { return GetIconCombine(a_nSize, CFGVAL_COP_INTERSECTION); }
HICON GetIconCombineDifference(ULONG a_nSize) { return GetIconCombine(a_nSize, CFGVAL_COP_DIFFERENCE); }
