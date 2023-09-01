
#include "stdafx.h"

#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"
#include <SharedStateUndo.h>
#include <IconRenderer.h>
#include <RWDocumentImageVector.h>
#include <RWBaseEnumUtils.h>
#include <SubjectNotImpl.h>

// {786E5C37-7F1E-4c2d-92FE-1DE5F12DDDB7}
extern GUID const CLSID_MenuCommandsLayerMerge = {0x786e5c37, 0x7f1e, 0x4c2d, {0x92, 0xfe, 0x1d, 0xe5, 0xf1, 0x2d, 0xdd, 0xb7}};

extern wchar_t const MERGE_NAME[] = L"[0409]Merge layers[0405]Sloučit vrstvy";
extern wchar_t const MERGE_DESC[] = L"[0409]Combine content of all layers flattening the image.[0405]Zkombinovat obsah všech vrstev do jediné vrstvy.";


class ATL_NO_VTABLE CMenuCommandsLayerMerge :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerMerge>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerMerge()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerMerge)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerMerge)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerMerge)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Merge Layers[0405]Vrstvený obrázek - sloučit vrstvy");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return CConfigGUILayerIDDlg::CreateConfig(a_ppDefaultConfig);
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		if (a_ppSubCommands == NULL)
			return E_POINTER;

		try
		{
			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return E_NOINTERFACE;

			CConfigValue cSelID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSelID);
			CComBSTR bstrID;
			pDLI->StatePrefix(&bstrID);
			if (bstrID.Length())
			{
				bstrID += cSelID.operator BSTR();
			}
			else
			{
				bstrID = cSelID.operator BSTR();
			}

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			{
				CComObject<CMerge>* p = NULL;
				CComObject<CMerge>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, a_pDocument, pDLI, a_pStates, bstrID);
				pCmds->Insert(pTmp);
			}

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CMerge :
		public CDocumentMenuCommandMLImpl<CMerge, MERGE_NAME, MERGE_DESC, &CLSID_MenuCommandsLayerMerge, 0>
	{
	public:
		void Init(IDesignerView* a_pView, IDocument* a_pDoc, IDocumentLayeredImage* a_pDLI, IOperationContext* a_pSSM, BSTR a_bstrSyncID)
		{
			m_pView = a_pView;
			m_pDoc = a_pDoc;
			m_pDLI = a_pDLI;
			m_pSSM = a_pSSM;
			m_bstrSyncID = a_bstrSyncID;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				static IRPolyPoint const aCoords1[] =
				{
					{24, 24}, {232, 24}, {232, 120}, {163, 80}, {93, 120}, {24, 80}
				};
				static IRPolyPoint const aCoords2[] =
				{
					{24, 136}, {93, 176}, {163, 136}, {232, 176}, {232, 232}, {24, 232}
				};

				static IRGridItem const tGrid[] = { {EGIFInteger, 24.0f}, {EGIFInteger, 232.0f}};
				static IRCanvas const tCanvas = {24, 24, 232, 232, itemsof(tGrid), itemsof(tGrid), tGrid, tGrid};
				static IRTarget const tTarget(0.92f);

				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&tCanvas, itemsof(aCoords1), aCoords1, pSI->GetMaterial(ESMBackground), &tTarget);
				cRenderer(&tCanvas, itemsof(aCoords2), aCoords2, pSI->GetMaterial(ESMAltBackground), &tTarget);
				*a_phIcon = cRenderer.get();

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		//EMenuCommandState IntState()
		//{
		//	CComPtr<IEnumUnknowns> pItems;
		//	m_pDLI->ItemsEnum(NULL, &pItems);
		//	ULONG nItems = 0;
		//	if (pItems) pItems->Size(&nItems);
		//	return nItems > 1 ? EMCSNormal : EMCSDisabled;
		//}

		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView) m_pView->DeactivateAll(FALSE);

				CWriteLock<IDocument> cLock(m_pDoc.p);
				CUndoBlock cUndo(m_pDoc, CMultiLanguageString::GetAuto(MERGE_NAME));

				CComPtr<IEnumUnknowns> pItems; // will be used and eventually deleted afterwards
				m_pDLI->ItemsEnum(NULL, &pItems);
				ULONG nItems = 0;
				if (pItems) pItems->Size(&nItems);

				CComPtr<ISharedState> pState;
				m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				CComPtr<IEnumUnknowns> pSelected;
				m_pDLI->StateUnpack(pState, &pSelected);
				ULONG nSelected = 0;
				if (pSelected) pSelected->Size(&nSelected);

				if (nSelected == 1)
				{
					CComPtr<IComparable> pEffect;
					pSelected->Get(0, &pEffect);
					CComPtr<IComparable> pLayer;
					m_pDLI->LayerFromEffect(pEffect, &pLayer);
					if (pLayer != NULL)
					{
						CComPtr<ISubDocumentID> pSDID;
						m_pDLI->ItemFeatureGet(pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
						CComPtr<IDocument> pCurRaw;
						if (pSDID) pSDID->SubDocumentGet(&pCurRaw);
						{
							CComObjectStackEx<CLayerOperationContext> cLOC;
							CComPtr<IOperationManager> pOpMgr;
							RWCoCreateInstance(pOpMgr, __uuidof(OperationManager));

							CComPtr<IEnumUnknowns> pEffects;
							m_pDLI->LayerEffectsEnum(pLayer, &pEffects);
							ULONG nEffects = 0;
							pEffects->Size(&nEffects);
							ULONG i;
							for (i = 0; i < nEffects; ++i)
							{
								CComPtr<IComparable> pE;
								pEffects->Get(i, &pE);
								BYTE bEnabled = 1;
								GUID tOpID = GUID_NULL;
								CComPtr<IConfig> pOpCfg;
								m_pDLI->LayerEffectStepGet(pE, &bEnabled, &tOpID, &pOpCfg);
								if (bEnabled)
								{
									CComPtr<IDocumentOperation> pOp;
									RWCoCreateInstance(pOp, tOpID);
									if (pOp)
									{
										bool bProcessed = false;
										CComQIPtr<IRasterImageFilter> pRIF(pOp);
										if (pRIF && pCurRaw)
										{
											// try processing via non-destructive IRasterImageFilter::Process method
											CComPtr<IDocumentBase> pBase;
											RWCoCreateInstance(pBase, __uuidof(DocumentBase));
											HRESULT hRes = pRIF->Process(pCurRaw, pOpCfg, pBase, NULL);
											if (SUCCEEDED(hRes))
											{
												pCurRaw = NULL;
												pBase->QueryInterface(&pCurRaw);
												//pCur = NULL;
												//pCurRaw->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pCur));
												bProcessed = true;
											}
											else if (hRes != E_NOTIMPL)
												bProcessed = true; // skip step if it failed ...or should we cancel the rest and break?
										}
										if (!bProcessed)
										{
											CComObjectStackEx<CRasterImageOperationStep> cStepSrc;
											cStepSrc.Init(pCurRaw);
											pOp->Activate(pOpMgr, &cStepSrc, pOpCfg, &cLOC, NULL, MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));
											cStepSrc.SwapRes(pCurRaw);
										}
									}
								}
								if (pEffect->Compare(pE) == S_OK)
								{
									++i;
									break;
								}
							}

							CComPtr<IComparable> pItem;
							m_pDLI->LayerInsert(pLayer, ELIPBelow, &CImageLayerCreatorDocument(pCurRaw), &pItem);

							for (; i < nEffects; ++i)
							{
								CComPtr<IComparable> pE;
								pEffects->Get(i, &pE);
								BYTE bEnabled = 1;
								GUID tOpID = GUID_NULL;
								CComPtr<IConfig> pOpCfg;
								m_pDLI->LayerEffectStepGet(pE, &bEnabled, &tOpID, &pOpCfg);
								m_pDLI->LayerEffectStepAppend(pItem, bEnabled, tOpID, pOpCfg, NULL);
							}

							CComBSTR bstr;
							m_pDLI->LayerNameGet(pLayer, &bstr);
							m_pDLI->LayerNameSet(pItem, bstr);

							ELayerBlend eBlendingMode;
							BYTE bVisible;
							m_pDLI->LayerPropsGet(pLayer, &eBlendingMode, &bVisible);
							m_pDLI->LayerPropsSet(pItem, &eBlendingMode, &bVisible);

							CComPtr<ISharedState> pState;
							m_pDLI->StatePack(1, &(pItem.p), &pState);
							CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID);
							m_pSSM->StateSet(m_bstrSyncID, pState);

							m_pDLI->LayerDelete(pLayer);
						}
						return S_OK;
					}
				}

				if (nSelected <= 1 || nSelected >= nItems)
				{
					// replace all layers with a single layer
					CComPtr<IDocumentImage> pDI;
					m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
					CBGRABuffer cBuffer;
					if (!cBuffer.Init(pDI, false))
						return E_FAIL;

					CComPtr<IComparable> pItem;
					m_pDLI->LayerInsert(NULL, ELIPDefault, &CImageLayerCreatorRasterImage(cBuffer.tSize, reinterpret_cast<TPixelChannel const*>(cBuffer.aData.m_p)), &pItem);

					wchar_t sz[64] = L"";
					Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_LAYER_MERGED, sz, itemsof(sz), LANGIDFROMLCID(a_tLocaleID));
					m_pDLI->LayerNameSet(pItem, CComBSTR(sz));
					CComPtr<ISharedState> pState;
					m_pDLI->StatePack(1, &(pItem.p), &pState);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID);
					m_pSSM->StateSet(m_bstrSyncID, pState);

					CComPtr<IComparable> pTmp;
					for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pTmp))); ++i, pTmp = NULL)
					{
						m_pDLI->LayerDelete(pTmp);
					}
					return S_OK;
				}

				// more sophisticated merge
				// TODO: - if consecutive sequence of layers is selected and all of them use normal blending mode and all are vector layers with no layer style (or same style?), merge them into 1 vector layer
				// - if consecutive sequence of layers is selected and all of them use normal blending mode, merge only them
				// - if consecutive sequence of layers from some layer to the lowest layer is selected, do "merge down"
				// - else report cannot merge error

				ULONG nSelFirst = nItems;
				ULONG nSelLast = 0;
				bool bSequence = true;
				for (ULONG i = 0; bSequence && i < nItems; ++i)
				{
					CComPtr<IComparable> pLayer;
					pItems->Get(i, &pLayer);
					if (pLayer == NULL) continue;
					bool bFound = false;
					for (ULONG j = 0; j < nSelected; ++j)
					{
						CComPtr<IComparable> pSelItem;
						pSelected->Get(j, &pSelItem);
						if (pSelItem == NULL) continue;
						if (pSelItem->Compare(pLayer) == S_OK)
						{
							bFound = true;
							break;
						}
					}
					if (bFound)
					{
						if (nSelFirst > i)
						{
							nSelFirst = nSelLast = i;
						}
						else if (nSelLast != i-1)
						{
							bSequence = false;
						}
						else
						{
							nSelLast = i;
						}
					}
				}
				if (nSelFirst > nSelLast)
					bSequence = false;
				bool bVectorSequence = bSequence;
				for (ULONG i = nSelFirst; bSequence && i <= nSelLast; ++i)
				{
					CComPtr<IComparable> pLayer;
					pItems->Get(i, &pLayer);
					ELayerBlend eBlend = EBEAlphaBlend;
					BYTE bVisible = 1;
					m_pDLI->LayerPropsGet(pLayer, &eBlend, &bVisible);
					if (bVisible && eBlend != EBEAlphaBlend)
					{
						bVectorSequence = bSequence = false;
					}
					else if (bVectorSequence)
					{
						CComPtr<IConfig> pEffect;
						m_pDLI->LayerEffectGet(pLayer, &pEffect, NULL);
						CConfigValue cVal;
						bool const bEffect = pEffect && SUCCEEDED(pEffect->ItemValueGet(CComBSTR(L"Effect"), &cVal)) &&
											 cVal.TypeGet() == ECVTGUID && !IsEqualGUID(cVal, __uuidof(DocumentOperationNULL));
						if (bEffect)
						{
							bVectorSequence = false;
						}
						else
						{
							CComPtr<ISubDocumentID> pSDID;
							m_pDLI->ItemFeatureGet(pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
							CComPtr<IDocument> pSubDoc;
							if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
							CComPtr<IDocumentVectorImage> pSubVI;
							if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pSubVI));
							bVectorSequence = pSubVI;
						}
					}
				}

				if (bVectorSequence)
				{
					CComPtr<IDocument> pDoc;
					RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
					CComQIPtr<IDocumentBase> pBase(pDoc);
					CComPtr<IDocumentFactoryVectorImage> pDF;
					RWCoCreateInstance(pDF, __uuidof(DocumentFactoryVectorImage));
					CComPtr<IDocumentImage> pDIOrig;
					m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDIOrig));
					TImageSize tCanvas = {0, 0};
					TImageResolution tRes = {100, 254, 100, 254};
					pDIOrig->CanvasGet(&tCanvas, &tRes, NULL, NULL, NULL);
					static float const aBg[] = {0.0f, 0.0f, 0.0f, 0.0f};
					pDF->Create(NULL, pBase, &tCanvas, &tRes, aBg);
					for (LONG i = nSelLast; i >= LONG(nSelFirst); --i)
					{
						CComPtr<IComparable> pLayer;
						pItems->Get(i, &pLayer);
						BYTE bVisible = 1;
						m_pDLI->LayerPropsGet(pLayer, NULL, &bVisible);
						if (bVisible == 0)
							continue;
						CComPtr<ISubDocumentID> pSDID;
						m_pDLI->ItemFeatureGet(pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
						CComPtr<IDocument> pSubDoc;
						if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
						CComPtr<IDocumentVectorImage> pSubVI;
						if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pSubVI));
						std::vector<ULONG> aObjects;
						if (pSubVI) pSubVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aObjects));
						CComBSTR bstrLayerName;
						m_pDLI->LayerNameGet(pLayer, &bstrLayerName);
						for (std::vector<ULONG>::const_iterator j = aObjects.begin(); j != aObjects.end(); ++j)
						{
							CComBSTR bstrToolID;
							CComBSTR bstrToolParams;
							CComBSTR bstrStyleID;
							CComBSTR bstrStyleParams;
							CComBSTR bstrName;
							float fWidth = 0.0f;
							float fPos = 0.0f;
							EOutlineJoinType eJoins = EOJTRound;
							TColor tOutClr = {0.0f, 0.0f, 0.0f, 0.0f};
							ERasterizationMode eRM = ERMSmooth;
							ECoordinatesMode eCM = ECMFloatingPoint;
							pSubVI->ObjectGet(*j, &bstrToolID, &bstrToolParams);
							pSubVI->ObjectStyleGet(*j, &bstrStyleID, &bstrStyleParams);
							pSubVI->ObjectNameGet(*j, &bstrName);
							BOOL bFill = FALSE;
							BOOL bOutline = FALSE;
							pSubVI->ObjectStateGet(*j, &bFill, &eRM, &eCM, &bOutline, &tOutClr, &fWidth, &fPos, &eJoins);
							pDF->AddObject(NULL, pBase, bstrName.m_str ? bstrName.m_str : bstrLayerName.m_str, bstrToolID, bstrToolParams, bFill ? bstrStyleID.m_str : NULL, bFill ? bstrStyleParams.m_str : NULL, bOutline ? &fWidth : NULL, bOutline ? &fPos : NULL, bOutline ? &eJoins : NULL, bOutline ? &tOutClr : NULL, &eRM, &eCM, NULL);
						}
					}
					CComPtr<IComparable> pUnder;
					pItems->Get(nSelFirst, &pUnder);

					CComPtr<IComparable> pItem;
					m_pDLI->LayerInsert(pUnder, ELIPBelow, &CImageLayerCreatorDocument(pDoc), &pItem);

					wchar_t sz[64] = L"";
					Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_LAYER_MERGED, sz, itemsof(sz), LANGIDFROMLCID(a_tLocaleID));
					m_pDLI->LayerNameSet(pItem, CComBSTR(sz));
					CComPtr<ISharedState> pState2;
					m_pDLI->StatePack(1, &(pItem.p), &pState2);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID, pState);
					m_pSSM->StateSet(m_bstrSyncID, pState2);

					for (ULONG i = nSelFirst; i <= nSelLast; ++i)
					{
						CComPtr<IComparable> pLayer;
						pItems->Get(i, &pLayer);
						m_pDLI->LayerDelete(pLayer);
					}
					return S_OK;
				}

				if (bSequence || nSelLast+1 == nItems)
				{
					CComPtr<IDocument> pDoc;
					RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
					CComQIPtr<IDocumentBase> pBase(pDoc);
					CComPtr<IDocumentFactoryLayeredImage> pDF;
					RWCoCreateInstance(pDF, __uuidof(DocumentFactoryLayeredImage));
					pDF->Create(NULL, pBase);
					CComPtr<IDocumentImage> pDIOrig;
					m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDIOrig));
					TImageSize tCanvas = {0, 0};
					TImageResolution tRes = {100, 254, 100, 254};
					pDIOrig->CanvasGet(&tCanvas, &tRes, NULL, NULL, NULL);
					pDF->SetSize(NULL, pBase, &tCanvas);
					//pDF->SetResolution(NULL, pBase, &tRes);
					for (ULONG i = nSelFirst; i <= nSelLast; ++i)
					{
						CComPtr<IComparable> pLayer;
						pItems->Get(i, &pLayer);
						TImageLayer tProps;
						ZeroMemory(&tProps, sizeof tProps);
						tProps.fOpacity = 1.0f;
						m_pDLI->LayerPropsGet(pLayer, &tProps.eBlend, &tProps.bVisible);
						CComPtr<IConfig> pEffect;
						m_pDLI->LayerEffectGet(pLayer, &pEffect, &tProps.fOpacity);
						CComPtr<ISubDocumentID> pSDID;
						m_pDLI->ItemFeatureGet(pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
						CComPtr<IDocument> pSubDoc;
						if (pSDID) pSDID->SubDocumentGet(&pSubDoc);

						pDF->AddLayer(NULL, pBase, 1, &CImageLayerCreatorDocument(pSubDoc), NULL, &tProps, pEffect);
					}
					CComPtr<IComparable> pUnder;
					pItems->Get(nSelFirst, &pUnder);

					CComPtr<IDocumentImage> pDI;
					pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDI));
					CBGRABuffer cBuffer;
					if (!cBuffer.Init(pDI, false))
						return E_FAIL;

					CComPtr<IComparable> pItem;
					m_pDLI->LayerInsert(pUnder, ELIPBelow, &CImageLayerCreatorRasterImage(cBuffer.tSize, reinterpret_cast<TPixelChannel const*>(cBuffer.aData.m_p)), &pItem);

					wchar_t sz[64] = L"";
					Win32LangEx::LoadStringW(_pModule->get_m_hInst(), IDS_LAYER_MERGED, sz, itemsof(sz), LANGIDFROMLCID(a_tLocaleID));
					m_pDLI->LayerNameSet(pItem, CComBSTR(sz));
					CComPtr<ISharedState> pState2;
					m_pDLI->StatePack(1, &(pItem.p), &pState2);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID, pState);
					m_pSSM->StateSet(m_bstrSyncID, pState2);

					for (ULONG i = nSelFirst; i <= nSelLast; ++i)
					{
						CComPtr<IComparable> pLayer;
						pItems->Get(i, &pLayer);
						m_pDLI->LayerDelete(pLayer);
					}
					return S_OK;
				}

				m_pSSM->SetErrorMessage(CMultiLanguageString::GetAuto(L"[0409]Selected layers cannot be merged without changing the image.[0405]Vybrané vrstvy nelze sloučit beze změny obrázku."));

				return E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IOperationContext> m_pSSM;
		CComBSTR m_bstrSyncID;
	};

	class ATL_NO_VTABLE CRasterImageOperationStep : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public CSubjectNotImpl<IDocument, IDocumentObserver, ULONG>,
		public CSubjectNotImpl<IDocumentRasterImage, IImageObserver, TImageChange>
	{
	public:
		void Init(IDocument* a_pDoc)
		{
			m_pDoc = a_pDoc;
			m_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pDocImg));

			m_pDocImg->CanvasGet(&m_tCanvas, &m_tRes, &m_tOrigin, &m_tSize, &m_eOpacity);

			m_tReadOrigin = m_tOrigin;
			m_tReadSize = m_tSize;

			RWCoCreateInstance(m_pDst, __uuidof(DocumentBase));
			CComPtr<IDocumentFactoryRasterImage> pFct;
			RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
			pFct->Create(NULL, CComQIPtr<IDocumentBase>(m_pDst), &m_tCanvas, &m_tRes, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 2.2f, NULL);
			//CComObject<CDocumentRasterImage>* pImg = NULL;
			//CComObject<CDocumentRasterImage>::CreateInstance(&pImg);
			//CComPtr<IDocumentData> pData(pImg);
			//pImg->Init(m_tCanvas, &m_tRes, NULL);
			//CComQIPtr<IDocumentBase>(m_pDst)->DataBlockSet(NULL, pData);
			m_pDst->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&m_pDstImg));
			m_bTouched = false;
		}
		void SwapRes(CComPtr<IDocument>& pRhs)
		{
			if (m_bTouched)
			{
				if (m_pDst.p != pRhs.p)
					pRhs.Attach(m_pDst.Detach());
			}
		}

	BEGIN_COM_MAP(CRasterImageOperationStep)
		COM_INTERFACE_ENTRY(IDocument)
		COM_INTERFACE_ENTRY(IBlockOperations)
		COM_INTERFACE_ENTRY(IDocumentRasterImage)
		COM_INTERFACE_ENTRY(IDocumentEditableImage)
		COM_INTERFACE_ENTRY(IDocumentImage)
	END_COM_MAP()

		// IBlockOperations methods
	public:
		STDMETHOD(WriteLock)()
		{
			return S_FALSE;
		}
		STDMETHOD(WriteUnlock)()
		{
			return S_FALSE;
		}
		STDMETHOD(ReadLock)()
		{
			return S_FALSE;
		}
		STDMETHOD(ReadUnlock)()
		{
			return S_FALSE;
		}

		// IDocument methods
	public:
		STDMETHOD(BuilderID)(CLSID* a_pguidBuilder)
		{
			*a_pguidBuilder = __uuidof(DocumentFactoryRasterImage);
			return S_OK;
		}
		STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
		{
			return QueryInterface(a_iid, a_ppFeatureInterface);
		}
		STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(LocationSet)(IStorageFilter* a_pLocation)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IsDirty)()
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SetDirty)()
		{
			return E_NOTIMPL;
		}
		STDMETHOD(ClearDirty)()
		{
			return E_NOTIMPL;
		}
		STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
		{
			try
			{
				CComPtr<IDocumentFactoryRasterImage> pFct;
				RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
				CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[m_tCanvas.nX*m_tCanvas.nY]);
				TileGet(EICIRGBA, NULL, NULL, NULL, m_tCanvas.nX*m_tCanvas.nY, pBuffer, NULL, EIRIPreview);
				return pFct->Create(a_bstrPrefix, a_pBase, &m_tCanvas, NULL, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(m_tCanvas.nX, m_tCanvas.nY, pBuffer));
			}
			catch (...)
			{
				return a_pBase ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(QuickInfo)(ULONG UNREF(a_nInfoIndex), ILocalizedString** UNREF(a_ppInfo))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(DocumentTypeGet)(IDocumentType** a_ppDocumentType)
		{
			return E_NOTIMPL;
		}

		// IDocumentImage methods
	public:
		STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
		{
			try
			{
				if (a_pCanvasSize) *a_pCanvasSize = m_tCanvas;
				if (a_pResolution) *a_pResolution = m_tRes;
				if (a_pContentOrigin) *a_pContentOrigin = m_tReadOrigin;
				if (a_pContentSize) *a_pContentSize = m_tReadSize;
				if (a_pContentOpacity) *a_pContentOpacity = m_eOpacity;
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(ChannelsGet)(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
		{
			return m_pDocImg->ChannelsGet(a_pChannelIDs, a_pGamma, a_pChannelDefaults);
		}
		STDMETHOD(TileGet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
		{
			return m_pDocImg->TileGet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData, a_pControl, a_eIntent);
		}
		STDMETHOD(Inspect)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
		{
			return m_pDocImg->Inspect(a_nChannelIDs, a_pOrigin, a_pSize, a_pVisitor, a_pControl, a_eIntent);
		}
		STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
		{
			return m_pDocImg->BufferLock(a_nChannelID, a_pAllocOrigin, a_pAllocSize, a_pContentOrigin, a_pContentSize, a_ppBuffer, a_pControl, a_eIntent);
		}
		STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
		{
			return m_pDocImg->BufferUnlock(a_nChannelID, a_pBuffer);
		}

		// IDocumentEditableImage methods
	public:
		STDMETHOD(CanvasSet)(TImageSize const* UNREF(a_pSize), TImageResolution const* UNREF(a_pResolution), TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
		{
			if (a_pContentTransform && a_pHelper)
			{
				m_bTouched = true;
				TPixelChannel tDefault;
				m_pDocImg->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tDefault));

				SLockedImageBuffer cBuf(m_pDocImg);
				
				TVector2f t00 = {cBuf.tContentOrigin.nX, cBuf.tContentOrigin.nY};
				TVector2f t11 = {cBuf.tContentOrigin.nX+cBuf.tContentSize.nX, cBuf.tContentOrigin.nY+cBuf.tContentSize.nY};
				TVector2f t01 = {t00.x, t11.y};
				TVector2f t10 = {t11.x, t00.y};
				t00 = TransformVector2(*a_pContentTransform, t00);
				t01 = TransformVector2(*a_pContentTransform, t01);
				t10 = TransformVector2(*a_pContentTransform, t10);
				t11 = TransformVector2(*a_pContentTransform, t11);
				TImagePoint tNewOrigin = {floor(t00.x), floor(t00.y)};
				if (floor(t01.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t01.x);
				if (floor(t01.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t01.y);
				if (floor(t10.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t10.x);
				if (floor(t10.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t10.y);
				if (floor(t11.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t11.x);
				if (floor(t11.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t11.y);
				TImagePoint tNewEnd = {ceil(t00.x), ceil(t00.y)};
				if (ceil(t01.x) > tNewEnd.nX) tNewEnd.nX = ceil(t01.x);
				if (ceil(t01.y) > tNewEnd.nY) tNewEnd.nY = ceil(t01.y);
				if (ceil(t10.x) > tNewEnd.nX) tNewEnd.nX = ceil(t10.x);
				if (ceil(t10.y) > tNewEnd.nY) tNewEnd.nY = ceil(t10.y);
				if (ceil(t11.x) > tNewEnd.nX) tNewEnd.nX = ceil(t11.x);
				if (ceil(t11.y) > tNewEnd.nY) tNewEnd.nY = ceil(t11.y);
				TImageSize const tNewSize = {tNewEnd.nX-tNewOrigin.nX, tNewEnd.nY-tNewOrigin.nY};
				ULONG const nPixels = tNewSize.nX*tNewSize.nY;
				CAutoPixelBuffer cOutBuf(m_pDstImg, tNewSize);
				if (nPixels)
					a_pHelper->ProcessTile(EICIRGBA, 2.2f, &tDefault, a_pContentTransform, cBuf.tAllocSize.nX*cBuf.tContentSize.nY, cBuf.pData+cBuf.tAllocSize.nX*(cBuf.tContentOrigin.nY-cBuf.tAllocOrigin.nY)+cBuf.tContentOrigin.nX-cBuf.tAllocOrigin.nX, &cBuf.tContentOrigin, &cBuf.tContentSize, CImageStride(1, cBuf.tAllocSize.nX), tNewSize.nX*tNewSize.nY, cOutBuf.Buffer(), &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));

				return cOutBuf.Replace(tNewOrigin, NULL, NULL, NULL, tDefault);
			}
			return E_NOTIMPL;
		}
		STDMETHOD(ChannelsSet)(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
		{
			m_bTouched = true;
			return m_pDstImg->ChannelsSet(a_nChannels, a_aChannelIDs, a_aChannelDefaults);
		}

		// IDocumentRasterImage methods
	public:
		STDMETHOD(TileSet)(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
		{
			m_bTouched = true;
			if (a_bDeleteOldContent) 
				return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, a_bDeleteOldContent);
			static TImagePoint const tPt0 = {0, 0};
			if (a_pOrigin == NULL)
			{
				a_pOrigin = &tPt0;
				if (a_pSize == NULL)
					a_pSize = &m_tCanvas;
			}
			else if (a_pSize == NULL)
				return E_RW_INVALIDPARAM;
			if (a_pOrigin->nX <= m_tReadOrigin.nX && a_pOrigin->nX <= m_tReadOrigin.nX &&
				a_pOrigin->nX+a_pSize->nX >= m_tReadOrigin.nX+m_tReadSize.nX &&
				a_pOrigin->nY+a_pSize->nY >= m_tReadOrigin.nY+m_tReadSize.nY)
				return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, TRUE);

			//ATLASSERT(FALSE); // have to? combine old and new content
			try
			{
				CAutoVectorPtr<TPixelChannel> cBuffer(new TPixelChannel[m_tReadSize.nX*m_tReadSize.nY]);
				m_pDocImg->TileGet(EICIRGBA, &m_tReadOrigin, &m_tReadSize, NULL, m_tReadSize.nX*m_tReadSize.nY, cBuffer, NULL, EIRIAccurate);

				HRESULT hRes = m_pDstImg->BufferReplace(m_tReadOrigin, m_tReadSize, &m_tReadOrigin, &m_tReadSize, NULL, cBuffer, &DefDeleter);
				if (SUCCEEDED(hRes)) cBuffer.Detach();

				return m_pDstImg->TileSet(a_nChannelIDs, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pPixels, FALSE);
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
		{
			return m_pDstImg->BufferAllocate(a_tSize, a_ppPixels, a_ppDeleter);
		}
		STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
		{
			m_bTouched = true;
			return m_pDstImg->BufferReplace(a_tAllocOrigin, a_tAllocSize, a_pContentOrigin, a_pContentSize, a_pContentAlphaSum, a_pPixels, a_pDeleter);
		}


	private:
		static void DefDeleter(TPixelChannel* p) { delete[] p; }

	private:
		CComPtr<IDocument> m_pDoc;
		CComPtr<IDocumentImage> m_pDocImg;

		// original doc cached values
		TImageSize m_tCanvas;
		TImageResolution m_tRes;
		TImagePoint m_tOrigin;
		TImageSize m_tSize;
		EImageOpacity m_eOpacity;

		TImagePoint m_tReadOrigin;
		TImageSize m_tReadSize;

		CComPtr<IDocument> m_pDst;
		CComPtr<IDocumentRasterImage> m_pDstImg;
		bool m_bTouched;
	};

	class ATL_NO_VTABLE CLayerOperationContext :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
	BEGIN_COM_MAP(CLayerOperationContext)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// IOperationContext methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
			if (i == m_cStates.end())
				return E_RW_ITEMNOTFOUND;
			return i->second->QueryInterface(a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
		{
			if (a_pState)
				m_cStates[a_bstrCategoryName] = a_pState;
			else
				m_cStates.erase(a_bstrCategoryName);
			return S_OK;
		}
		STDMETHOD(IsCancelled)()
		{
			return S_FALSE;
		}
		STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
		{
			if (a_pItemIndex) *a_pItemIndex = 0;
			if (a_pItemsRemaining) *a_pItemsRemaining = 0;
			if (a_pStepIndex) *a_pStepIndex = 0;
			if (a_pStepsRemaining) *a_pStepsRemaining = 0;
			return S_OK;
		}
		STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
		{
			return S_FALSE;
		}

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
	};

};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerMerge, CMenuCommandsLayerMerge)
