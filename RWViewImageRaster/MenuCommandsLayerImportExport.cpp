// MenuCommandsLayerImportExport.cpp : Implementation of CMenuCommandsLayerImportExport

#include "stdafx.h"
#include "resource.h"
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <SharedStateUndo.h>
#include <RWDocumentImageVector.h>
#include <RWBaseEnumUtils.h>
#include "ConfigGUILayerID.h"
#include <ConfigCustomGUIImpl.h>
#include <IconRenderer.h>

static IRPolyPoint const aBack[] = { {0, 36}, {220, 36}, {220, 256}, {0, 256}, };
static IRPolyPoint const aFront[] = { {36, 161}, {36, 0}, {256, 0}, {256, 220}, {95, 220}, };
static IRPolyPoint const aCorner[] = { {44, 155}, {101, 155}, {101, 212}, };
static IRPolygon const tBack = {itemsof(aBack), aBack};
static IRPolygon const tFront = {itemsof(aFront), aFront};
static IRPolygon const tCorner = {itemsof(aCorner), aCorner};
//static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
static IRGridItem const tGridBackX[] = { {EGIFInteger, 0.0f}, {EGIFInteger, 220.0f}};
static IRGridItem const tGridBackY[] = { {EGIFInteger, 36.0f}, {EGIFInteger, 256.0f}};
static IRCanvas const tCanvasBack = {0, 0, 256, 256, itemsof(tGridBackX), itemsof(tGridBackY), tGridBackX, tGridBackY};
static IRGridItem const tGridFrontX[] = { {EGIFInteger, 36.0f}, {EGIFMidPixel, 101.0f}, {EGIFInteger, 256.0f}};
static IRGridItem const tGridFrontY[] = { {EGIFInteger, 0.0f}, {EGIFMidPixel, 155.0f}, {EGIFInteger, 220.0f}};
static IRCanvas const tCanvasFront = {0, 0, 256, 256, itemsof(tGridFrontX), itemsof(tGridFrontY), tGridFrontX, tGridFrontY};
static IRTarget const tTarget(0.8f, -1, 1);

static OLECHAR const CFGID_ADJUST[] = L"Adjust";


extern __declspec(selectany) GUID const g_tIconIDLayerAddRaster = {0x4d7f0e85, 0x3b46, 0x4b47, {0xa0, 0x71, 0xd, 0x71, 0x57, 0x58, 0x63, 0x8a}};
extern __declspec(selectany) GUID const g_tIconIDLayerClone = {0xeee63704, 0xbe08, 0x4d7e, {0xa4, 0xa1, 0x83, 0x9a, 0x4e, 0x59, 0x83, 0x93}};
extern __declspec(selectany) GUID const g_tIconIDLayerDelete = {0x89882344, 0xd793, 0x458a, {0xab, 0xcc, 0x6, 0xbe, 0xff, 0xd5, 0x9b, 0x66}};
extern __declspec(selectany) GUID const g_tIconIDLayerImport = {0x77117f93, 0x366, 0x458d, {0xaf, 0xeb, 0x9a, 0x2f, 0x54, 0x85, 0x43, 0x7d}};
extern __declspec(selectany) GUID const g_tIconIDLayerExport = {0xf61440e, 0x631d, 0x43b1, {0xb8, 0x3e, 0x86, 0xb3, 0xb7, 0xc1, 0x8d, 0x43}};
extern __declspec(selectany) GUID const g_tIconIDLayerMerge = {0x9566f427, 0x4116, 0x4028, {0x90, 0xc, 0xf6, 0x6, 0x3a, 0x10, 0xa3, 0x66}};


// D1D4952E-4B88-466A-814B-EB700B5A9235
extern GUID const CLSID_MenuCommandsLayerImportExport = {0xd1d4952e, 0x4b88, 0x466a, {0x81, 0x4b, 0xeb, 0x70, 0x0b, 0x5a, 0x92, 0x35}};


// CMenuCommandsLayerImportExport

class ATL_NO_VTABLE CMenuCommandsLayerImportExport :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerImportExport, &CLSID_MenuCommandsLayerImportExport>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerImportExport()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerImportExport)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerImportExport)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerImportExport)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Import/Export Layer[0405]Vrstvený obrázek - import/export vrstvy");
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
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return E_FAIL;

			CConfigValue cVal;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);
			CComBSTR bstrID;
			pDLI->StatePrefix(&bstrID);
			if (bstrID.Length())
			{
				bstrID += cVal;
			}
			else
			{
				bstrID.Attach(cVal.Detach().bstrVal);
			}

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CImport>* p = NULL;
				CComObject<CImport>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, a_pDocument, pDLI, a_pStates, bstrID);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CExport>* p = NULL;
				CComObject<CExport>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, a_pDocument, pDLI, a_pStates, bstrID);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	class ATL_NO_VTABLE CConfigGUIImportLayerDlg :
		public CCustomConfigResourcelessWndImpl<CConfigGUIImportLayerDlg>
	{
	public:
		enum { IDC_CGIMP_ADJUST = 100 };

		BEGIN_DIALOG_EX(0, 0, 100, 10, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_CHECKBOX(_T("[0409]Adjust canvas size[0405]Přizpůsobit velikost plátna"), IDC_CGIMP_ADJUST, 0, 0, 100, 10, WS_VISIBLE | WS_TABSTOP, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUIImportLayerDlg)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIImportLayerDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		END_MSG_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUIImportLayerDlg)
			CONFIGITEM_CHECKBOX(IDC_CGIMP_ADJUST, CFGID_ADJUST)
		END_CONFIGITEM_MAP()

		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			return 1;
		}

	};

	class ATL_NO_VTABLE CImport :
		public CDocumentMenuCommandImpl<CImport, IDS_MN_IMPORTLAYER, IDS_MD_IMPORTLAYER, &g_tIconIDLayerImport, 0>
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
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&tCanvasBack, 1, &tBack, pSI->GetMaterial(ESMAltBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tFront, pSI->GetMaterial(ESMBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tCorner, pSI->GetMaterial(ESMOutlineSoft), &tTarget);
				pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IInputManager> pIM;
				RWCoCreateInstance(pIM, __uuidof(InputManager));
				CComPtr<IEnumUnknowns> pBuilders;
				pIM->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pBuilders);
				CComPtr<IEnumUnknowns> pTypes;
				pIM->DocumentTypesEnumEx(pBuilders, &pTypes);
				CComPtr<IStorageManager> pSM;
				RWCoCreateInstance(pSM, __uuidof(StorageManager));
				static const GUID tCtxID = {0xb85db35e, 0x70d4, 0x4b37, {0x81, 0xcb, 0x1f, 0x5e, 0xc3, 0x78, 0x87, 0xc6}};
				CComPtr<IStorageFilter> pLoc;
				static bool bExtend = false;
				CComPtr<IConfigWithDependencies> pCfg;
				RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
				CComBSTR cCFGID_ADJUST(CFGID_ADJUST);
				pCfg->ItemInsSimple(cCFGID_ADJUST, CMultiLanguageString::GetAuto(L"[0409]Adjust canvas size[0405]Přizpůsobit velikost plátna"), CMultiLanguageString::GetAuto(L"[0409]If the imported image is larger than the current canvas, the canvas will be extended.[0405]Pokud bude importovaný obrázek větší než současné plátno, plátno bude rozšířeno."), CConfigValue(bExtend), NULL, 0, NULL);
				CConfigCustomGUI<&g_tIconIDLayerImport, CConfigGUIImportLayerDlg>::FinalizeConfig(pCfg);
				pSM->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, a_hParent, pTypes, pCfg, tCtxID, _SharedStringTable.GetStringAuto(IDS_MN_IMPORTLAYER), NULL, a_tLocaleID, &pLoc);
				if (pLoc == NULL)
					return S_FALSE;
				CConfigValue cVal;
				pCfg->ItemValueGet(cCFGID_ADJUST, &cVal);
				bExtend = cVal;

				if (m_pView) m_pView->DeactivateAll(FALSE);

				CWriteLock<IDocument> cLock(m_pDoc.p);
				CUndoBlock cUndo(m_pDoc, _SharedStringTable.GetStringAuto(IDS_MN_IMPORTLAYER));

				CComPtr<IComparable> pWhere;
				{
					CComPtr<ISharedState> pState;
					m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
					CComPtr<IEnumUnknowns> pSel;
					m_pDLI->StateUnpack(pState, &pSel);
					if (pSel)
					{
						ULONG n = 0;
						pSel->Size(&n);
						if (n == 1)
						{
							pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pWhere));
							if (m_pDLI->IsLayer(pWhere, NULL, NULL, NULL) != S_OK)
								pWhere = NULL;
						}
					}
				}

				CComPtr<IComparable> pItem;
				HRESULT hRes = m_pDLI->LayerInsert(pWhere, ELIPDefault, &CImageLayerCreatorStorage(pLoc), &pItem);
				if (SUCCEEDED(hRes) && pItem)
				{
					CComBSTR bstrPath;
					pLoc->ToText(NULL, &bstrPath);
					if (bstrPath.Length())
					{
						LPCOLESTR p1 = wcsrchr(bstrPath, L'\\');
						LPCOLESTR p2 = wcsrchr(bstrPath, L'//');
						if (p2 > p1) p1 = p2;
						if (p1 == NULL) p1 = bstrPath; else ++p1;
						p2 = wcsrchr(p1, L'.');
						if (p2 == NULL) p2 = p1+wcslen(p1);
						BSTR bstr = SysAllocStringLen(p1, p2-p1);
						m_pDLI->LayerNameSet(pItem, bstr);
						SysFreeString(bstr);
					}
					CSharedStateUndo<IOperationContext>::SaveState(m_pDoc.p, m_pSSM, m_bstrSyncID);
					CComPtr<ISharedState> pState;
					m_pDLI->StatePack(1, &(pItem.p), &pState);
					m_pSSM->StateSet(m_bstrSyncID, pState);
				}
				if (bExtend)
				{
					CComPtr<IDocumentEditableImage> pEI;
					m_pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
					TImageSize tCanvasSize = {1, 1};
					if (pEI) pEI->CanvasGet(&tCanvasSize, NULL, NULL, NULL, NULL);
					CComPtr<ISubDocumentID> pSDID;
					m_pDLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
					CComPtr<IDocument> pSubDoc;
					if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
					CComPtr<IDocumentImage> pI;
					if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
					TImageSize tImgSize = {1, 1};
					TImagePoint tImgOrig = {0, 0};
					if (pI) pI->CanvasGet(NULL, NULL, &tImgOrig, &tImgSize, NULL);
					tImgOrig.nX += tImgSize.nX;
					tImgOrig.nY += tImgSize.nY;
					bool bChange = false;
					if (LONG(tCanvasSize.nX) < tImgOrig.nX)
					{
						bChange = true;
						tCanvasSize.nX = tImgOrig.nX;
					}
					if (LONG(tCanvasSize.nY) < tImgOrig.nY)
					{
						bChange = true;
						tCanvasSize.nY = tImgOrig.nY;
					}
					if (bChange)
						pEI->CanvasSet(&tCanvasSize, NULL, NULL, NULL);
				}
				return hRes;
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

	class ATL_NO_VTABLE CExport :
		public CDocumentMenuCommandImpl<CExport, IDS_MN_EXPORTLAYER, IDS_MD_EXPORTLAYER, &g_tIconIDLayerExport, 0>
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
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&tCanvasBack, 1, &tBack, pSI->GetMaterial(ESMAltBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tFront, pSI->GetMaterial(ESMBackground), &tTarget);
				cRenderer(&tCanvasFront, 1, &tCorner, pSI->GetMaterial(ESMOutlineSoft), &tTarget);
				pSI->GetLayers(ESIFloppySimple, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState()
		{
			CComPtr<ISharedState> pState;
			m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pSel;
			m_pDLI->StateUnpack(pState, &pSel);
			if (pSel == NULL) return EMCSDisabled;
			ULONG nItems = 0;
			if (pSel) pSel->Size(&nItems);
			if (nItems != 1)
				return EMCSDisabled;
			CComPtr<IComparable> pItem;
			pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			return m_pDLI->IsLayer(pItem, NULL, NULL, NULL) == S_OK ? EMCSNormal : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView) m_pView->DeactivateAll(FALSE);

				CComPtr<ISharedState> pState;
				m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				CComPtr<IEnumUnknowns> pSel;
				m_pDLI->StateUnpack(pState, &pSel);
				if (pSel == NULL) return E_FAIL;
				CComPtr<IComparable> pItem;
				if (pSel) pSel->Get(0, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
				if (m_pDLI->IsLayer(pItem, NULL, NULL, NULL) != S_OK)
					return E_FAIL;
				CComPtr<ISubDocumentID> pSDID;
				m_pDLI->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID));
				CComPtr<IDocument> pSubDoc;
				if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
				CComPtr<IDocument> pDoc;
				RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
				pSubDoc->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pDoc), NULL, NULL);

				CComPtr<IInputManager> pIM;
				RWCoCreateInstance(pIM, __uuidof(InputManager));

				CComBSTR bstrName;
				m_pDLI->LayerNameGet(pItem, &bstrName);

				CComPtr<IConfig> pOpts;
				CComPtr<IEnumUnknowns> pFlts;
				CComPtr<IStorageFilterWindowListener> pLst;
				pIM->SaveOptionsGet(pDoc, &pOpts, &pFlts, &pLst);

				CComPtr<IStorageManager> pSM;
				RWCoCreateInstance(pSM, __uuidof(StorageManager));
				static const GUID tCtxID = {0xb85db35e, 0x70d4, 0x4b37, {0x81, 0xcb, 0x1f, 0x5e, 0xc3, 0x78, 0x87, 0xc6}};
				CComPtr<IStorageFilter> pLoc;
				pSM->FilterCreateInteractivelyUID(bstrName, EFTCreateNew, a_hParent, pFlts, pOpts, tCtxID, _SharedStringTable.GetStringAuto(IDS_MN_EXPORTLAYER), pLst, a_tLocaleID, &pLoc);
				if (pLoc == NULL)
					return S_FALSE;

				return pIM->Save(pDoc, pOpts, pLoc);
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

};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerImportExport, CMenuCommandsLayerImportExport)

