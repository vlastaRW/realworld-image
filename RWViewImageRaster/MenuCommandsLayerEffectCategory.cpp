﻿
#include "stdafx.h"
#include <RWProcessing.h>
#include <WeakSingleton.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <SharedStateUndo.h>
#include <PlugInCache.h>
#include <StringParsing.h>
#include <RWProcessingTags.h>
#include <ConfigCustomGUIImpl.h>


// CMenuCommandsLayerEffectCategory

// {9EB9AF80-2583-4d12-8E68-8CC835656977}
extern GUID const CLSID_MenuCommandsLayerEffectCategory = {0x9eb9af80, 0x2583, 0x4d12, {0x8e, 0x68, 0x8c, 0xc8, 0x35, 0x65, 0x69, 0x77}};
static const OLECHAR CFGID_CATEGORY[] = L"Category";
static const OLECHAR CFGID_SELECTIONSYNC[] = L"LayerID";
static const OLECHAR CFGID_PRIMARY[] = L"Primary";
static const OLECHAR CFGID_SECONDARY[] = L"Secondary";
static const OLECHAR CFGID_DISABLED[] = L"Disabled";

typedef std::vector<std::pair<GUID, DWORD> > CGUIDAccelVector;

class ATL_NO_VTABLE CMenuCommandsLayerEffectCategory :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerEffectCategory, &CLSID_MenuCommandsLayerEffectCategory>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerEffectCategory()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsLayerEffectCategory)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerEffectCategory)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerEffectCategory)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Effect Category[0405]Vrstvený obrázek - kategorie efektů");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;
			CComPtr<IConfigWithDependencies> pCfg;
			RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

			pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection sync ID[0405]Synchronizace výběru"), CMultiLanguageString::GetAuto(L"[0409]Image selection is synchronized by the given ID.[0405]Vybraná oblast v obrázku je dopstupná a sychronizována přes zadané ID."), CConfigValue(L"LAYER"), NULL, 0, NULL);

			CComBSTR bstrCFGID_CATEGORY(CFGID_CATEGORY);
			pCfg->ItemIns1ofN(bstrCFGID_CATEGORY, CMultiLanguageString::GetAuto(L"[0409]Effect type[0405]Typ efektu"), NULL, CConfigValue(CATID_TagImageUnderlay), NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageRearragement), CMultiLanguageString::GetAuto(L"[0409]Rearrangement[0405]Přeskládání"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImagePhotofix), CMultiLanguageString::GetAuto(L"[0409]Photo fix[0405]Oprava fotky"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageTransformation), CMultiLanguageString::GetAuto(L"[0409]Transformation[0405]Transformace"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageColorAdjustment), CMultiLanguageString::GetAuto(L"[0409]Color adjustment[0405]Doladění barev"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageEmbellishment), CMultiLanguageString::GetAuto(L"[0409]Decoration[0405]Dekorace"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageShape), CMultiLanguageString::GetAuto(L"[0409]Shape adjustment[0405]Změna tvaru"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageUnderlay), CMultiLanguageString::GetAuto(L"[0409]Underlay[0405]Podklad"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageOverlay), CMultiLanguageString::GetAuto(L"[0409]Overlay[0405]Překrytí"), 0, NULL);
			pCfg->ItemOptionAdd(bstrCFGID_CATEGORY, CConfigValue(CATID_TagImageFinalizer), CMultiLanguageString::GetAuto(L"[0409]Framing[0405]Rámování"), 0, NULL);

			pCfg->ItemInsSimple(CComBSTR(CFGID_PRIMARY), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_SECONDARY), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_DISABLED), NULL, NULL, CConfigValue(L""), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_MenuCommandsLayerEffectCategory, CConfigGUIEffectCategoryDlg>::FinalizeConfig(pCfg);

			*a_ppDefaultConfig = pCfg.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return S_FALSE;

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

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));

			CComPtr<IEnumUnknowns> pItems;
			pDLI->StateUnpack(pState, &pItems);
			ULONG nItems = 0;
			if (pItems) pItems->Size(&nItems);
			if (nItems != 1)
				return S_FALSE;

			CComPtr<IComparable> pItem;
			pItems->Get(0, &pItem);
			if (pDLI->IsLayer(pItem, NULL, NULL, NULL) != S_OK)
			{
				CComPtr<IComparable> pItem2;
				pDLI->LayerFromEffect(pItem, &pItem2);
				if (pItem2 == NULL)
					return S_FALSE; // weird
				std::swap(pItem.p, pItem2.p);
			}

			CGUIDAccelVector aPri;
			CGUIDAccelVector aSec;
			CGUIDAccelVector aDis;
			ReadOperations(a_pConfig, aPri, aSec, aDis);

			CComQIPtr<IOperationManager> pMgr(a_pManager);

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			for (CGUIDAccelVector::const_iterator i = aPri.begin(); i != aPri.end(); ++i)
			{
				CComObject<CDocumentMenuCommand>* p = NULL;
				CComObject<CDocumentMenuCommand>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItem, a_pStates, bstrID, i->first, pMgr ? pMgr : M_OperationManager(), i->second, a_pView);

				pCmds->Insert(p);
			}
			if (!aSec.empty())
			{
				CComObject<CSubMenu>* p = NULL;
				CComObject<CSubMenu>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItem, a_pStates, bstrID, aSec, pMgr ? pMgr : M_OperationManager(), a_pView);

				pCmds->Insert(p);
			}

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	IOperationManager* M_OperationManager()
	{
		ObjectLock cLock(this);
		if (m_pOpsMgr)
			return m_pOpsMgr;
		RWCoCreateInstance(m_pOpsMgr, __uuidof(OperationManager));
		if (m_pOpsMgr == NULL)
			throw E_UNEXPECTED;
		return m_pOpsMgr;
	}

	static bool GUIDAndAccelFromString(wchar_t const*& pStr, std::pair<GUID, DWORD>& tVal)
	{
		tVal.second = 0;
		bool bG = GUIDFromString(pStr, &tVal.first);
		if (bG)
			pStr += 36;
		else
			return false;
		if (*pStr == L'(')
		{
			wchar_t const* p = pStr;
			++p;
			while ((*p >= L'0' && *p <= L'9') || (*p >= L'a' && *p <= L'f') || (*p >= L'A' && *p <= L'F'))
			{
				tVal.second = (tVal.second<<4);
				if (*p >= L'0' && *p <= L'9')
					tVal.second += *p-L'0';
				else if (*p >= L'a' && *p <= L'f')
					tVal.second += *p-L'a'+10;
				else if (*p >= L'A' && *p <= L'F')
					tVal.second += *p-L'A'+10;
				++p;
			}
			if (*p != L')')
				return false;
			++p;
			pStr = p;
		}
		return true;
	}
	static void ParseGUIDList(wchar_t const* pStr, CGUIDAccelVector& aOut, std::vector<GUID> const& aAll)
	{
		aOut.clear();
		if (pStr == NULL)
			return;
		std::pair<GUID, DWORD> t;
		while (GUIDAndAccelFromString(pStr, t))
		{
			if (std::binary_search(aAll.begin(), aAll.end(), t.first, CPlugInEnumerator::lessCATID()))
				aOut.push_back(t);
			if (*pStr != L'|')
				break;
			++pStr;
		}
	}

	struct to_vector
	{
		to_vector(std::vector<GUID>& aAll) : aAll(aAll) {}
		void operator()(CLSID const* begin, CLSID const* const end) const
		{
			aAll.resize(end-begin);
			std::copy(begin, end, aAll.begin());
		}
	private:
		std::vector<GUID>& aAll;
	};

	struct SInserter : public std::iterator<std::output_iterator_tag, void, void, void, void>
	{
		SInserter(CGUIDAccelVector& dst) : dst(&dst) {}

		SInserter& operator=(GUID const& t) { dst->push_back(std::make_pair(t, DWORD(0))); return *this; }
		SInserter& operator*() { return *this; }
		SInserter& operator++() { return *this; }
		SInserter& operator++(int) { return *this; }

	private:
		CGUIDAccelVector* dst;
	};
	static void ReadOperations(IConfig* a_pConfig, CGUIDAccelVector& aPri, CGUIDAccelVector& aSec, CGUIDAccelVector& aDis)
	{
		CConfigValue cCatID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_CATEGORY), &cCatID);

		CConfigValue cPri;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PRIMARY), &cPri);
		CConfigValue cSec;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SECONDARY), &cSec);
		CConfigValue cDis;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DISABLED), &cDis);

		std::vector<GUID> aAll;
		CPlugInEnumerator::EnumCategoryCLSIDs(cCatID, to_vector(aAll));
		std::sort(aAll.begin(), aAll.end(), CPlugInEnumerator::lessCATID());

		ParseGUIDList(cPri, aPri, aAll);
		ParseGUIDList(cSec, aSec, aAll);
		ParseGUIDList(cDis, aDis, aAll);
		std::set<GUID, CPlugInEnumerator::lessCATID> used;
		for (CGUIDAccelVector::const_iterator i = aPri.begin(); i != aPri.end(); ++i)
			used.insert(i->first);
		for (CGUIDAccelVector::const_iterator i = aSec.begin(); i != aSec.end(); ++i)
			used.insert(i->first);
		for (CGUIDAccelVector::const_iterator i = aDis.begin(); i != aDis.end(); ++i)
			used.insert(i->first);

		//GetCategoryTimestamp(REFCATID a_tCatID)
		std::set_difference(aAll.begin(), aAll.end(), used.begin(), used.end(), SInserter(aPri), CPlugInEnumerator::lessCATID());
	}
	static wchar_t HexChar(DWORD dw)
	{
		dw &= 0xf;
		if (dw < 10)
			return L'0'+dw;
		return L'a'+dw-10;
	}
	static void SerializeGUIDList(CGUIDAccelVector const& aVals, TConfigValue* pOut)
	{
		int len = aVals.empty() ? 0 : aVals.size()*37-1;
		for (CGUIDAccelVector::const_iterator i = aVals.begin(); i != aVals.end(); ++i)
			if (i->second)
				len += 10;
		pOut->bstrVal = SysAllocStringLen(NULL, len);
		pOut->eTypeID = ECVTString;
		wchar_t* p = pOut->bstrVal;
		for (CGUIDAccelVector::const_iterator i = aVals.begin(); true;)
		{
			if (i != aVals.end())
			{
				StringFromGUID(i->first, p);
				p += 36;
				if (i->second)
				{
					p[0] = L'(';
					p[1] = HexChar(i->second>>28);
					p[2] = HexChar(i->second>>24);
					p[3] = HexChar(i->second>>20);
					p[4] = HexChar(i->second>>16);
					p[5] = HexChar(i->second>>12);
					p[6] = HexChar(i->second>>8);
					p[7] = HexChar(i->second>>4);
					p[8] = HexChar(i->second);
					p[9] = L')';
					p += 10;
				}
				++i;
			}
			if (i != aVals.end())
			{
				*p = L'|';
				++p;
			}
			else
			{
				*p = L'\0';
				break;
			}
		}
	}
	static void SerializeOperations(IConfig* a_pConfig, CGUIDAccelVector const& aPri, CGUIDAccelVector const& aSec, CGUIDAccelVector const& aDis)
	{
		CComBSTR b1(CFGID_PRIMARY);
		CComBSTR b2(CFGID_SECONDARY);
		CComBSTR b3(CFGID_DISABLED);
		BSTR aIDs[] = {b1, b2, b3};
		CConfigValue c1;
		SerializeGUIDList(aPri, &c1);
		CConfigValue c2;
		SerializeGUIDList(aSec, &c2);
		CConfigValue c3;
		SerializeGUIDList(aDis, &c3);
		TConfigValue aVals[] = {c1, c2, c3};
		a_pConfig->ItemValuesSet(3, aIDs, aVals);
	}

private:
	class ATL_NO_VTABLE CConfigGUIEffectCategoryDlg :
		public CCustomConfigResourcelessWndImpl<CConfigGUIEffectCategoryDlg>,
		public CDialogResize<CConfigGUIEffectCategoryDlg>
	{
	public:
		CConfigGUIEffectCategoryDlg() : m_tLastCatID(GUID_NULL), m_hDragged(NULL), m_wndShortcut(this, 1)
		{
		}

		enum
		{
			IDC_LAYERSYNCID = 100,
			IDC_CATEGORY,
			IDC_OPERATIONS,
			IDC_ACCELERATOR,
		};

		BEGIN_DIALOG_EX(0, 0, 180, 111, 0)
			DIALOG_FONT_AUTO()
			DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
			DIALOG_EXSTYLE(0)
		END_DIALOG()

		BEGIN_CONTROLS_MAP()
			CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizace výběru:"), IDC_STATIC, 0, 2, 60, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_LAYERSYNCID, 60, 0, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
			CONTROL_LTEXT(_T("[0409]Category:[0405]Kategorie:"), IDC_STATIC, 0, 18, 60, 8, WS_VISIBLE, 0)
			CONTROL_COMBOBOX(IDC_CATEGORY, 60, 16, 119, 160, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
			CONTROL_CONTROL(_T(""), IDC_OPERATIONS, WC_TREEVIEW, TVS_FULLROWSELECT | TVS_NOHSCROLL | TVS_SHOWSELALWAYS | WS_TABSTOP | WS_VISIBLE, 60, 32, 119, 78, WS_EX_CLIENTEDGE)
			CONTROL_LTEXT(_T("[0409]Accelerator:[0405]Zkratka:"), IDC_STATIC, 0, 32, 60, 8, WS_VISIBLE, 0)
			CONTROL_EDITTEXT(IDC_ACCELERATOR, 0, 42, 53, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_READONLY, 0)
		END_CONTROLS_MAP()

		BEGIN_MSG_MAP(CConfigGUIEffectCategoryDlg)
			CHAIN_MSG_MAP(CDialogResize<CConfigGUIEffectCategoryDlg>)
			CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEffectCategoryDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
			MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
			NOTIFY_HANDLER(IDC_OPERATIONS, TVN_ITEMEXPANDING, OnItemExpanding)
			NOTIFY_HANDLER(IDC_OPERATIONS, TVN_BEGINDRAG, OnBeginDrag)
		ALT_MSG_MAP(1)
			MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
			MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CConfigGUIEffectCategoryDlg)
			DLGRESIZE_CONTROL(IDC_CATEGORY, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_LAYERSYNCID, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_OPERATIONS, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_CONFIGITEM_MAP(CConfigGUIEffectCategoryDlg)
			CONFIGITEM_COMBOBOX(IDC_CATEGORY, CFGID_CATEGORY)
			CONFIGITEM_EDITBOX(IDC_LAYERSYNCID, CFGID_SELECTIONSYNC)
		END_CONFIGITEM_MAP()


		LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
		{
			m_wndShortcut.SubclassWindow(GetDlgItem(IDC_ACCELERATOR));

			DlgResize_Init(false, false, 0);

			return 1;
		}
		LRESULT OnItemExpanding(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
		{
			return TRUE;
		}
		LRESULT OnBeginDrag(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
		{
			LPNMTREEVIEW pNMTV = reinterpret_cast<LPNMTREEVIEW>(a_pNMHDR);
			if (pNMTV->itemNew.hItem == m_hPri || pNMTV->itemNew.hItem == m_hSec || pNMTV->itemNew.hItem == m_hDis)
				return FALSE;

			SetCapture();
			m_hDragged = pNMTV->itemNew.hItem;
			return TRUE;
		}
		LRESULT OnMouseMove(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (m_hDragged == NULL)
				return 0;

			LONG xCur = GET_X_LPARAM(a_lParam);
			LONG yCur = GET_Y_LPARAM(a_lParam);

			POINT point = {xCur, yCur};
			ClientToScreen(&point);
			m_wnd.ScreenToClient(&point);

			TVHITTESTINFO tvht;   // Hit test information.
			tvht.pt.x = point.x;
			tvht.pt.y = point.y;
			HTREEITEM htiTarget;  // Handle to target item.
			if ((htiTarget = m_wnd.HitTest(&tvht)) != NULL)
	        {
				RECT rc;
				m_wnd.GetItemRect(htiTarget, &rc, FALSE);
				bool bOnSection = htiTarget == m_hPri || htiTarget == m_hSec || htiTarget == m_hDis;
				bool bAfter = point.y+point.y > rc.top+rc.bottom || bOnSection;
				m_wnd.SetInsertMark(htiTarget, bAfter);
			}
			return 0;
		}
		LRESULT OnLButtonUp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
		{
			if (m_hDragged == NULL)
				return 0;

			m_wnd.RemoveInsertMark();

			LONG xCur = GET_X_LPARAM(a_lParam);
			LONG yCur = GET_Y_LPARAM(a_lParam);

			POINT point = {xCur, yCur};
			ClientToScreen(&point);
			m_wnd.ScreenToClient(&point);

			TVHITTESTINFO tvht;   // Hit test information.
			tvht.pt.x = point.x;
			tvht.pt.y = point.y;
			HTREEITEM htiTarget;  // Handle to target item.
			if ((htiTarget = m_wnd.HitTest(&tvht)) != NULL)
	        {
				RECT rc;
				m_wnd.GetItemRect(htiTarget, &rc, FALSE);
				bool bOnSection = htiTarget == m_hPri || htiTarget == m_hSec || htiTarget == m_hDis;
				bool bAfter = point.y+point.y > rc.top+rc.bottom || bOnSection;

				LONG srcIndex = m_wnd.GetItemData(m_hDragged);
				CGUIDAccelVector* pSrc = NULL;
				HTREEITEM hSrcPar = m_wnd.GetParentItem(m_hDragged);
				if (hSrcPar == m_hPri)
					pSrc = &m_aPri;
				else if (hSrcPar == m_hSec)
					pSrc = &m_aSec;
				else if (hSrcPar == m_hDis)
					pSrc = &m_aDis;

				LONG dstIndex = bOnSection ? 0 : (bAfter ? 1 : 0)+m_wnd.GetItemData(htiTarget);
				CGUIDAccelVector* pDst = NULL;
				HTREEITEM hDstPar = bOnSection ? htiTarget : m_wnd.GetParentItem(htiTarget);
				if (hDstPar == m_hPri)
					pDst = &m_aPri;
				else if (hDstPar == m_hSec)
					pDst = &m_aSec;
				else if (hDstPar == m_hDis)
					pDst = &m_aDis;

				if (pDst && pSrc && (pDst != pSrc || (dstIndex != srcIndex && dstIndex != srcIndex+1)))
				{
					if (pSrc == pDst)
					{
						if (dstIndex < srcIndex)
							std::rotate(pSrc->begin()+dstIndex, pSrc->begin()+srcIndex, pSrc->begin()+srcIndex+1);
						else
							std::rotate(pSrc->begin()+srcIndex, pSrc->begin()+srcIndex+1, pSrc->begin()+dstIndex);
					}
					else
					{
						pDst->insert(pDst->begin()+dstIndex, (*pSrc)[srcIndex]);
						pSrc->erase(pSrc->begin()+srcIndex);
					}
					m_wnd.SetRedraw(FALSE);
					InsertItems(m_hPri, m_aPri);
					InsertItems(m_hSec, m_aSec);
					InsertItems(m_hDis, m_aDis);
					m_wnd.SetRedraw(TRUE);
					{
						HTREEITEM h = m_wnd.GetChildItem(hDstPar);
						for (int i = 0; i < dstIndex; ++i)
							h = m_wnd.GetNextSiblingItem(h);
						m_wnd.SelectItem(h);
					}
					m_wnd.RedrawWindow();
					SerializeOperations(M_Config(), m_aPri, m_aSec, m_aDis);
				}
			}

			ReleaseCapture();
			m_hDragged = NULL;
			return 0;
		}

		LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			try
			{
				HTREEITEM hSel = m_wnd.GetSelectedItem();
				if (hSel != NULL && hSel != m_hPri && hSel != m_hSec && hSel != m_hDis)
				{
					LONG index = m_wnd.GetItemData(hSel);
					CGUIDAccelVector* pSrc = NULL;
					HTREEITEM hSrcPar = m_wnd.GetParentItem(hSel);
					if (hSrcPar == m_hPri)
						pSrc = &m_aPri;
					else if (hSrcPar == m_hSec)
						pSrc = &m_aSec;
					else if (hSrcPar == m_hDis)
						pSrc = &m_aDis;
					if (pSrc && index >= 0 && size_t(index) < pSrc->size())
					{
						DWORD dwNew = (*pSrc)[index].second;
						if (a_wParam != VK_MENU && a_wParam != VK_SHIFT && a_wParam != VK_CONTROL &&
							a_wParam != VK_LMENU && a_wParam != VK_LSHIFT && a_wParam != VK_LCONTROL &&
							a_wParam != VK_RMENU && a_wParam != VK_RSHIFT && a_wParam != VK_RCONTROL)
							dwNew = a_wParam|(((GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0))<<16);
						else
							dwNew = 0;
						if ((*pSrc)[index].second != dwNew)
						{
							(*pSrc)[index].second = dwNew;
							m_wnd.SetRedraw(FALSE);
							InsertItems(m_hPri, m_aPri);
							InsertItems(m_hSec, m_aSec);
							InsertItems(m_hDis, m_aDis);
							m_wnd.SetRedraw(TRUE);
							{
								HTREEITEM h = m_wnd.GetChildItem(hSrcPar);
								for (int i = 0; i < index; ++i)
									h = m_wnd.GetNextSiblingItem(h);
								m_wnd.SelectItem(h);
							}
							m_wnd.RedrawWindow();
							SerializeOperations(M_Config(), m_aPri, m_aSec, m_aDis);
						}
					}
				}
			}
			catch (...)
			{
			}
			a_bHandled = FALSE;
			return 0;
		}

		void ExtraInitDialog()
		{
			m_wnd = GetDlgItem(IDC_OPERATIONS);
			m_hPri = m_wnd.InsertItem(TVIF_TEXT|TVIF_STATE, L"Visible", 0, 0, TVIS_EXPANDED, TVIS_EXPANDED, 0, TVI_ROOT, TVI_LAST);
			m_hSec = m_wnd.InsertItem(TVIF_TEXT|TVIF_STATE, L"Secondary", 0, 0, TVIS_EXPANDED, TVIS_EXPANDED, 0, TVI_ROOT, TVI_LAST);
			m_hDis = m_wnd.InsertItem(TVIF_TEXT|TVIF_STATE, L"Hidden", 0, 0, TVIS_EXPANDED, TVIS_EXPANDED, 0, TVI_ROOT, TVI_LAST);
		}
		void ExtraConfigNotify()
		{
			CConfigValue cCatID;
			M_Config()->ItemValueGet(CComBSTR(CFGID_CATEGORY), &cCatID);
			if (!IsEqualGUID(cCatID, m_tLastCatID))
			{
				ReadOperations(M_Config(), m_aPri, m_aSec, m_aDis);
				InsertItems(m_hPri, m_aPri);
				InsertItems(m_hSec, m_aSec);
				InsertItems(m_hDis, m_aDis);
				m_tLastCatID = cCatID;
			}
		}

	private:
		void InsertItems(HTREEITEM hParent, CGUIDAccelVector const& aItems)
		{
			while (HTREEITEM hCh = m_wnd.GetChildItem(hParent))
				m_wnd.DeleteItem(hCh);
			for (CGUIDAccelVector::const_iterator i = aItems.begin(); i != aItems.end(); ++i)
			{
				CComPtr<IDocumentOperation> pOp;
				RWCoCreateInstance(pOp, i->first);
				CComQIPtr<IConfigDescriptor> pDesc(pOp);
				CComPtr<ILocalizedString> pName;
				if (pDesc)
					pDesc->Name(NULL, NULL, &pName);
				if (pName == NULL)
					pOp->NameGet(NULL, &pName);
				CComBSTR bstr;
				if (pName)
					pName->GetLocalized(m_tLocaleID, &bstr);
				if (i->second)
				{
					bstr += L" (";
					wchar_t szBuffer[128];
					GetShortcutName(i->second, szBuffer);
					szBuffer[127] = L'\0';
					bstr += szBuffer;
					bstr += L")";
				}
				m_wnd.InsertItem(TVIF_TEXT|TVIF_PARAM, bstr, 0, 0, 0, 0, i-aItems.begin(), hParent, TVI_LAST);
			}
		}
		static void GetShortcutName(DWORD dwCode, wchar_t* pszBuffer)
		{
			if (dwCode == 0)
			{
				pszBuffer[0] = L'-';
				pszBuffer[1] = L'\0';
				//Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NOSHORTCUT, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
			}
			else
			{
				wchar_t* p = pszBuffer;
				if (dwCode&(FALT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_MENU, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (dwCode&(FCONTROL<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_CONTROL, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (dwCode&(FSHIFT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_SHIFT, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				UINT scancode = MapVirtualKey(dwCode&0xffff, 0);
				switch(dwCode&0xffff)
				{
				case VK_INSERT:
				case VK_DELETE:
				case VK_HOME:
				case VK_END:
				case VK_NEXT:  // Page down
				case VK_PRIOR: // Page up
				case VK_LEFT:
				case VK_RIGHT:
				case VK_UP:
				case VK_DOWN:
					scancode |= 0x100; // Add extended bit
				}
				GetKeyNameText((scancode<<16)|0x2000000, p, 31);
			}
		}

	private:
		CTreeViewCtrl m_wnd;
		HTREEITEM m_hPri;
		HTREEITEM m_hSec;
		HTREEITEM m_hDis;
		GUID m_tLastCatID;
		CGUIDAccelVector m_aPri;
		CGUIDAccelVector m_aSec;
		CGUIDAccelVector m_aDis;

		HTREEITEM m_hDragged;

		CContainedWindowT<CEdit> m_wndShortcut;
	};

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CDocumentMenuCommand()
		{
		}
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IComparable* a_pItem, IOperationContext* a_pStates, BSTR a_bstrSyncID, GUID const& a_tOpID, IOperationManager* a_pOpsMgr, DWORD a_tAccel, IDesignerView* a_pView)
		{
			m_pItem = a_pItem;
			m_tOpID = a_tOpID;
			m_pOpsMgr = a_pOpsMgr;
			m_tAccel = a_tAccel;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
			RWCoCreateInstance(m_pOp, a_tOpID);
			m_pDescriptor = m_pOp;
		}

	BEGIN_COM_MAP(CDocumentMenuCommand)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = NULL;
				if (m_pDescriptor == NULL)
					return m_pOp->NameGet(m_pOpsMgr, a_ppText);
				return m_pDescriptor->Name(m_pOpsMgr, NULL, a_ppText);
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]Add effect to the selected layer.[0405]Přidat efekt k vybrané vrstvě.");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (m_pDescriptor && SUCCEEDED(m_pDescriptor->PreviewIconID(m_pOpsMgr, NULL, a_pIconID)))
				return S_OK;
			*a_pIconID = m_tOpID;
			return S_OK;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				if (m_pDescriptor && SUCCEEDED(m_pDescriptor->PreviewIcon(m_pOpsMgr, NULL, a_nSize, a_phIcon)))
					return S_OK;
				CComPtr<IDesignerFrameIcons> pIcons;
				RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
				return pIcons->GetIcon(m_tOpID, a_nSize, a_phIcon);
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			if ((m_tAccel&0xffff) == 0)
				return E_NOTIMPL;

			try
			{
				a_pAccel->wKeyCode = m_tAccel;
				a_pAccel->fVirtFlags = m_tAccel>>16;
				return S_OK;
			}
			catch (...)
			{
				return a_pAccel ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands)) { return E_NOTIMPL; }
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			if (a_peState == NULL)
				return E_POINTER;
			*a_peState = EMCSNormal;
			return S_OK;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView)
					m_pView->DeactivateAll(FALSE);

				CComPtr<IEnumUnknowns> pEffects;
				m_pDLI->LayerEffectsEnum(m_pItem, &pEffects);
				ULONG nEffects = 0;
				if (pEffects) pEffects->Size(&nEffects);
				//CComPtr<IComparable> pToRemove;
				//for (ULONG i = 0; i < nEffects; ++i)
				//{
				//	CComPtr<IComparable> pEffect;
				//	pEffects->Get(i, &pEffect);
				//	GUID tID = GUID_NULL;
				//	m_pDLI->LayerEffectStepGet(pEffect, NULL, &tID, NULL);
				//	if (IsEqualGUID(tID, m_tOpID))
				//	{
				//		pToRemove = pEffect;
				//		break;
				//	}
				//}

				CWriteLock<IDocument> cLock(m_pDocument.p);
				CComPtr<ILocalizedString> pName;
				Name(&pName);
				CUndoBlock cUndo(m_pDocument, pName);

				//if (pToRemove)
				//{
				//	CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrSyncID);
				//	m_pDLI->LayerEffectStepDelete(pToRemove);
				//	CComPtr<ISharedState> pNewState;
				//	m_pDLI->StatePack(1, &(m_pItem.p), &pNewState);
				//	m_pStates->StateSet(m_bstrSyncID, pNewState);
				//	return S_OK;
				//}

				// insert effect
				CComPtr<IComparable> pBefore;
				CComPtr<ILayerStyle> pNew;
				RWCoCreateInstance(pNew, m_tOpID);
				if (pNew)
				{
					BYTE bPrio = pNew->ExecutionPriority();
					//CComPtr<IEnumUnknowns> pEffects;
					//m_pDLI->LayerEffectsEnum(m_pItem, &pEffects);
					//ULONG nEffects = 0;
					//if (pEffects) pEffects->Size(&nEffects);
					for (ULONG i = 0; i < nEffects; ++i)
					{
						CComPtr<IComparable> p;
						pEffects->Get(i, &p);
						GUID tID = GUID_NULL;
						m_pDLI->LayerEffectStepGet(p, NULL, &tID, NULL);
						CComPtr<ILayerStyle> pOld;
						RWCoCreateInstance(pOld, tID);
						if (pOld && pOld->IsPriorityAnchor() == S_OK)
						{
							BYTE bOldPrio = pOld->ExecutionPriority();
							if (bPrio < bOldPrio)
							{
								pBefore = p;
								break;
							}
						}
					}
				}
				CComPtr<IComparable> pNewSel;
				m_pDLI->LayerEffectStepAppend(pBefore.p ? pBefore.p : m_pItem.p, TRUE, m_tOpID, NULL, &pNewSel);
				if (pNewSel)
				{
					CComPtr<ISharedState> pOldState;
					m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldState));
					if (pOldState == NULL)
						m_pDLI->StatePack(1, &(m_pItem.p), &pOldState);
					CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, m_bstrSyncID, pOldState);
					CComPtr<ISharedState> pNewState;
					m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
					m_pStates->StateSet(m_bstrSyncID, pNewState);
				}

				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IComparable> m_pItem;
		CComPtr<IDocumentOperation> m_pOp;
		CComQIPtr<IConfigDescriptor> m_pDescriptor;
		GUID m_tOpID;
		CComPtr<IOperationManager> m_pOpsMgr;
		DWORD m_tAccel;
		CComBSTR m_bstrSyncID;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	class ATL_NO_VTABLE CSubMenu :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CSubMenu()
		{
		}
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IComparable* a_pItem, IOperationContext* a_pStates, BSTR a_bstrSyncID, CGUIDAccelVector& a_aItems, IOperationManager* a_pOpsMgr, IDesignerView* a_pView)
		{
			m_pItem = a_pItem;
			m_pOpsMgr = a_pOpsMgr;
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
			std::swap(m_aItems, a_aItems);
		}

	BEGIN_COM_MAP(CSubMenu)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(L"[0409]More[0405]Další");
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
		{
			if (a_ppSubCommands == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IEnumUnknownsInit> pCmds;
				RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

				for (CGUIDAccelVector::const_iterator i = m_aItems.begin(); i != m_aItems.end(); ++i)
				{
					CComObject<CDocumentMenuCommand>* p = NULL;
					CComObject<CDocumentMenuCommand>::CreateInstance(&p);
					CComPtr<IDocumentMenuCommand> pTmp = p;

					p->Init(m_pDocument, m_pDLI, m_pItem, m_pStates, m_bstrSyncID, i->first, m_pOpsMgr, i->second, m_pView);

					pCmds->Insert(p);
				}
				*a_ppSubCommands = pCmds.Detach();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			if (a_peState == NULL)
				return E_POINTER;
			*a_peState = EMCSSubMenu;
			return S_OK;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return E_NOTIMPL;
		}

	private:
		CComPtr<IComparable> m_pItem;
		CComPtr<IOperationManager> m_pOpsMgr;
		CGUIDAccelVector m_aItems;
		CComBSTR m_bstrSyncID;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	CComPtr<IOperationManager> m_pOpsMgr;
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerEffectCategory, CMenuCommandsLayerEffectCategory)
