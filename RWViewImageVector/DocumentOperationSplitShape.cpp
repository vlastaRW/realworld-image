
#include "stdafx.h"
#include <RWDrawing.h>
#include <MultiLanguageString.h>
#include "RWDocumentImageVector.h"
#include <RWBaseEnumUtils.h>
#include "PathUtils.h"
#include <SharedStateUndo.h>


// CDocumentOperationSplitShape

extern GUID const CLSID_DocumentOperationSplitShape = {0x290e6e87, 0x3096, 0x4a70, {0xbd, 0xdc, 0x24, 0xf9, 0x82, 0x53, 0x94, 0xa8}};
static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_SEPARATEHOLES[] = L"SeparateHoles";

class ATL_NO_VTABLE CDocumentOperationSplitShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationSplitShape, &CLSID_DocumentOperationSplitShape>,
	public IDocumentOperation
{
public:
	CDocumentOperationSplitShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationSplitShape)

BEGIN_CATEGORY_MAP(CDocumentOperationSplitShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationSplitShape)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Split Shape[0405]Vektorový obrázek - rozdělit Tvar");
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
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SEPARATEHOLES), CMultiLanguageString::GetAuto(L"[0409]Separate holes[0405]Oddělit díry"), CMultiLanguageString::GetAuto(L"[0409]Represent holes in shapes with separate shapes using transparent fill.[0405]Reprezentovat díry v tvarech samostatnými tvary s průhlednou výplní."), CConfigValue(false), NULL, 0, NULL);

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
			if (aSelected.empty())
				return S_FALSE;
			// at lease 1 should be decomposable
			for (std::vector<ULONG>::const_iterator i = aSelected.begin(); i != aSelected.end(); ++i)
			{
				CComBSTR bstrToolID;
				pDVI->ObjectGet(*i, &bstrToolID, NULL);
				// TODO: use actual tool properties when tools are redesigned
				if (bstrToolID == L"SHAPE" || bstrToolID == L"TEXT" || bstrToolID == L"POLYGON")
					return S_OK;
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
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CConfigValue cSepHoles;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SEPARATEHOLES), &cSepHoles);
			
			CWriteLock<IBlockOperations> cLock(a_pDocument);
			
			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			bstrState.Append(cSyncID.operator BSTR());
			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> aSelected;
			pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
			if (aSelected.empty())
				return S_FALSE;

			std::vector<ULONG> aAll;
			pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aAll));

			std::vector<ULONG> aNewSel;
			bool noOp = true;

			for (std::vector<ULONG>::const_iterator sel = aSelected.begin(); sel != aSelected.end(); ++sel)
			{
				ULONG objectID = *sel;

				ULONG under = 0;
				{
					std::vector<ULONG>::iterator pos = std::find(aAll.begin(), aAll.end(), objectID);
					if (pos != aAll.end() && pos+1 != aAll.end())
						under = *++pos;
				}

				CComObjectStackEx<CToolWindow> cWnd;

				CBezierPaths srcBeziers;
				ClipperLib::Paths srcPolys;

				CComPtr<IRasterImageEditToolsManager> pToolMgr;
				RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));
				{
					CComBSTR bstrID;
					CComBSTR bstrParams;
					HRESULT hRes = pDVI->ObjectGet(objectID, &bstrID, &bstrParams);
					if (FAILED(hRes))
						continue;
					CComPtr<IRasterImageEditTool> pTool;
					pToolMgr->EditToolCreate(bstrID, NULL, &pTool);
					if (pTool == NULL)
						continue;
					CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
					if (pToolScript == NULL)
					{
						aNewSel.push_back(objectID);
						continue;
					}
					CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
					if (pToolPoly == NULL)
					{
						aNewSel.push_back(objectID);
						continue;
					}
					pTool->Init(&cWnd);
					pToolScript->FromText(bstrParams);

					CPolygonReceiver cReceiver(srcBeziers, srcPolys);
					pToolPoly->ToPath(&cReceiver);
					if (srcBeziers.empty() || srcPolys.empty())
					{
						aNewSel.push_back(objectID);
						continue;
					}
				}

				ClipperLib::Paths cDst;
				{
					ClipperLib::Clipper c;
					c.ZFillFunction(&CoordZFill);
					c.StrictlySimple(true);
					c.AddPaths(srcPolys, ClipperLib::ptSubject, true);
					c.Execute(ClipperLib::ctUnion, cDst, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
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

				if (srcPolys.size() <= 1 && cDst.size() <= 1)
				{
					aNewSel.push_back(objectID);
					continue; // nothing to do
				}
				else
				{
					noOp = false;
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

				CComBSTR objName;
				pDVI->ObjectNameGet(objectID, &objName);
				if (objName.Length())
					 objName += L"-";
				BOOL objFill = TRUE;
				ERasterizationMode objRastMode = ERMSmooth;
				ECoordinatesMode objCoordMode = ECMFloatingPoint;
				BOOL objOutline = FALSE;
				TColor objOutColor = {0.0f, 0.0f, 0.0f, 1.0f};
				float objOutWidth = 1.0f;
				float objOutPos = 0.0f;
				EOutlineJoinType objOutJoins = EOJTRound;
				pDVI->ObjectStateGet(objectID, &objFill, &objRastMode, &objCoordMode, &objOutline, &objOutColor, &objOutWidth, &objOutPos, &objOutJoins);
				CComBSTR objStyle;
				CComBSTR objStyleParams;
				pDVI->ObjectStyleGet(objectID, &objStyle, &objStyleParams);
				for (size_t i = 0; i < restored.size(); ++i)
				{
					if (!cSepHoles && holes[i] != -1)
						continue;

					int h = 0;
					std::vector<TRWPath> buffer;
					TRWPath cur;
					cur.nVertices = restored[i].size();
					cur.pVertices = &(restored[i][0]);
					buffer.push_back(cur);
					if (!cSepHoles)
					{
						for (size_t j = 0; j < restored.size(); ++j)
						{
							if (holes[j] == i)
							{
								++h;
								cur.nVertices = restored[j].size();
								cur.pVertices = &(restored[j][0]);
								buffer.push_back(cur);
							}
						}
					}

					CComBSTR toolID;
					CComBSTR toolParams;
					GetToolParams(cWnd, pToolMgr, buffer, &toolID, &toolParams);
					wchar_t szSuff[32] = L"";
					_itow(i+1, szSuff, 10);
					CComBSTR name(objName);
					name += szSuff;
					if (i == 0)
					{
						pDVI->ObjectSet(&objectID, toolID, toolParams);
						pDVI->ObjectNameSet(objectID, name);
						aNewSel.push_back(objectID);
					}
					else
					{
						ULONG newObjID = 0;
						pDVI->ObjectSet(&newObjID, toolID, toolParams);
						pDVI->ObjectNameSet(newObjID, name);
						pDVI->ObjectStateSet(newObjID, &objFill, &objRastMode, &objCoordMode, &objOutline, &objOutColor, &objOutWidth, &objOutPos, &objOutJoins);
						if (holes[i] == -1)
						{
							pDVI->ObjectStyleSet(newObjID, objStyle, objStyleParams);
							if (under)
								pDVI->ObjectsMove(1, &newObjID, under);
						}
						else
						{
							pDVI->ObjectStyleSet(newObjID, CComBSTR(L"SOLID"), CComBSTR(L"0,0,0,0"));
							ULONG u2 = under;
							int aboveOwner = holes[i]+1;
							if (aboveOwner < int(i) && holes[aboveOwner] == -1)
								u2 = aNewSel[aNewSel.size()+aboveOwner-i];
							if (u2)
								pDVI->ObjectsMove(1, &newObjID, u2);
						}
						aNewSel.push_back(newObjID);
					}
				}
			}

			if (noOp)
				return S_FALSE;

			CComPtr<ISharedState> pNewState;
			pDVI->StatePack(aNewSel.size(), &(aNewSel[0]), &pNewState);
			if (pNewState)
			{
				CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, bstrState, pState);
				a_pStates->StateSet(bstrState, pNewState);
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

#include <IconRenderer.h>
HICON GetIconSplitShape(ULONG a_nSize)
{
	static IRPathPoint const left[] =
	{
		{112, 80, -26.5097, 0, 0, 0},
		{64, 128, 0, 26.5097, 0, -26.5097},
		{112, 176, 0, 0, -26.5097, 0},
		{112, 208, 0, 0, 0, 0},
		{0, 208, 0, 0, 0, 0},
		{0, 48, 0, 0, 0, 0},
		{112, 48, 0, 0, 0, 0},
	};
	static IRPathPoint const right[] =
	{
		{256, 208, 0, 0, 0, 0},
		{176, 208, 0, 0, 0, 0},
		{176, 176, -26.5097, 0, 0, 0},
		{128, 128, 0, -26.5097, 0, 26.5097},
		{176, 80, 0, 0, -26.5097, 0},
		{176, 48, 0, 0, 0, 0},
		{256, 48, 0, 0, 0, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRGridItem const hor[] = { {0, 48.0f}, {0, 208.0f} };
	static IRGridItem const lft[] = { {0, 0.0f}, {0, 112.0f} };
	static IRGridItem const rgt[] = { {0, 176.0f}, {0, 256.0f} };
	IRCanvas canvasL = {0, 0, 256, 256, itemsof(lft), itemsof(hor), lft, hor};
	IRCanvas canvasR = {0, 0, 256, 256, itemsof(rgt), itemsof(hor), rgt, hor};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvasL, itemsof(left), left, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvasR, itemsof(right), right, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}

HICON GetIconCombineShape(ULONG a_nSize)
{
	static IRPolyPoint const left[] =
	{
		{32, 48}, {168, 48}, {168, 208}, {32, 208},
	};
	static IRPathPoint const right[] =
	{
		{224, 208, 0, 0, 0, 0},
		{144, 208, 0, 0, 0, 0},
		{144, 176, -26.5097, 0, 0, 0},
		{96, 128, 0, -26.5097, 0, 26.5097},
		{144, 80, 0, 0, -26.5097, 0},
		{144, 48, 0, 0, 0, 0},
		{224, 48, 0, 0, 0, 0},
	};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRGridItem const hor[] = { {0, 48.0f}, {0, 208.0f} };
	static IRGridItem const ver[] = { {0, 32.0f}, {0, 168.0f}, {0, 224.0f} };
	IRCanvas canvas = {0, 0, 256, 256, itemsof(ver), itemsof(hor), ver, hor};

	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(left), left, pSI->GetMaterial(ESMInterior));
	cRenderer(&canvas, itemsof(right), right, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}

OBJECT_ENTRY_AUTO(CLSID_DocumentOperationSplitShape, CDocumentOperationSplitShape)
