
#include "stdafx.h"
#include <RWProcessing.h>
#include <WeakSingleton.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include "../ShowConfigWithPreviewDlg.h"
#include <RWDocumentImageRaster.h>
#include <SharedStateUndo.h>


// CMenuCommandsInsertOperator

extern GUID const CLSID_MenuCommandsInsertOperator = {0x390bacaa, 0xf8e2, 0x487e, {0xa8, 0x45, 0xc1, 0x53, 0x3b, 0x22, 0x88, 0x70}}; // {390BACAA-F8E2-487e-A845-C1533B228870}
static const OLECHAR CFGID_OPERATION_ID[] = L"Operation";
static const OLECHAR CFGID_OPERATION_NAME[] = L"Name";
static const OLECHAR CFGID_OPERATION_DESCRITPION[] = L"Description";
static const OLECHAR CFGID_OPERATION_ICONID[] = L"IconID";
static const OLECHAR CFGID_OPERATION_SHORTCUT[] = L"Shortcut";
static const OLECHAR CFGID_INSOP_SYNCID[] = L"LayerID";

#include <ConfigCustomGUIIcons.h>
#include <ConfigCustomGUIML.h>

class ATL_NO_VTABLE CConfigGUIInsertOperatorDlg :
	public CCustomConfigWndMultiLang<CConfigGUIInsertOperatorDlg, CCustomConfigWndWithIcons<CConfigGUIInsertOperatorDlg, CCustomConfigResourcelessWndImpl<CConfigGUIInsertOperatorDlg> > >,
	public CDialogResize<CConfigGUIInsertOperatorDlg>
{
	typedef CCustomConfigWndWithIcons<CConfigGUIInsertOperatorDlg, CCustomConfigResourcelessWndImpl<CConfigGUIInsertOperatorDlg> > base1;
public:
	CConfigGUIInsertOperatorDlg() : m_wndShortcut(this, 1),
		CCustomConfigWndMultiLang<CConfigGUIInsertOperatorDlg, CCustomConfigWndWithIcons<CConfigGUIInsertOperatorDlg, CCustomConfigResourcelessWndImpl<CConfigGUIInsertOperatorDlg> > >(CFGID_CFG_CAPTION, CFGID_CFG_HELPTOPIC)
	{
	}

	enum
	{
		IDC_CG_NAME = 100,
		IDC_CG_SHORTCUTLABEL, IDC_CG_SHORTCUT,
		IDC_CG_DESCRIPTION,
		IDC_CG_ICONLABEL, IDC_CG_ICON,
		IDC_CG_CAPTION,
		IDC_CG_OPERATION,
		IDC_CG_HELPTOPIC_LABEL, IDC_CG_HELPTOPIC,
		IDC_CG_LAYERSYNCID,
	};

	BEGIN_DIALOG_EX(0, 0, 180, 81, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Name:[0405]&Název:"), IDC_STATIC, 0, 2, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_NAME, 60, 0, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]&Shortcut:[0405]&Zkratka:"), IDC_CG_SHORTCUTLABEL, 121, 2, 40, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_SHORTCUT, 161, 0, 18, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_READONLY, 0)
		CONTROL_LTEXT(_T("[0409]&Description:[0405]&Popis:"), IDC_STATIC, 0, 18, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_DESCRIPTION, 60, 16, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Dialog caption:[0405]Záhlaví dialogu:"), IDC_STATIC, 0, 34, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_CAPTION, 60, 32, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Icon:[0405]Ikona:"), IDC_CG_ICONLABEL, 121, 34, 27, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_CG_ICON, WC_COMBOBOXEX, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 150, 32, 29, 197, 0)
		CONTROL_LTEXT(_T("[0409]Operation:[0405]Operace:"), IDC_STATIC, 0, 50, 60, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CG_OPERATION, 60, 48, 119, 160, CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0)
    //CONTROL         "",IDC_CG_OPCONFIG,"Static",SS_GRAYRECT,0,32,88,43
		CONTROL_LTEXT(_T("[0409]Help text:[0405]Nápověda:"), IDC_CG_HELPTOPIC_LABEL, 0, 66, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_HELPTOPIC, 60, 64, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]&Selection sync ID::[0405]Synchronizace výběru:"), IDC_CG_HELPTOPIC_LABEL, 0, 82, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_LAYERSYNCID, 60, 80, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIInsertOperatorDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIInsertOperatorDlg>)
		CHAIN_MSG_MAP(base1)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIInsertOperatorDlg)
		DLGRESIZE_CONTROL(IDC_CG_NAME, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_SHORTCUTLABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_SHORTCUT, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CG_DESCRIPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_CAPTION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICONLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_ICON, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_HELPTOPIC, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_LAYERSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIInsertOperatorDlg)
		CONFIGITEM_EDITBOX(IDC_CG_NAME, CFGID_OPERATION_NAME)
		CONFIGITEM_EDITBOX(IDC_CG_DESCRIPTION, CFGID_OPERATION_DESCRITPION)
		CONFIGITEM_EDITBOX(IDC_CG_CAPTION, CFGID_CFG_CAPTION)
		CONFIGITEM_ICONCOMBO(IDC_CG_ICON, CFGID_CFG_ICONID)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_OPERATION_ID)
		CONFIGITEM_EDITBOX(IDC_CG_HELPTOPIC, CFGID_CFG_HELPTOPIC)
		CONFIGITEM_EDITBOX(IDC_CG_LAYERSYNCID, CFGID_INSOP_SYNCID)
	END_CONFIGITEM_MAP()

	void ExtraConfigNotify()
	{
		CConfigValue cShortcut;
		M_Config()->ItemValueGet(CComBSTR(CFGID_OPERATION_SHORTCUT), &cShortcut);
		if (cShortcut.TypeGet() == ECVTInteger && m_wndShortcut.m_hWnd)
		{
			TCHAR szTmp[128] = _T("");
			if (cShortcut.operator LONG() == 0)
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(L"[0409]None[0405]žádná", m_tLocaleID, &bstr);
				_tcsncpy(szTmp, bstr, 128);
				szTmp[127] = _T('\0');
			}
			else
			{
				TCHAR* p = szTmp;
				if (cShortcut.operator LONG()&(FALT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_MENU, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (cShortcut.operator LONG()&(FCONTROL<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_CONTROL, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				if (cShortcut.operator LONG()&(FSHIFT<<16))
				{
					p += GetKeyNameText((MapVirtualKey(VK_SHIFT, 0)<<16)|0x2000000, p, 31);
					_tcscpy(p, _T("+"));
					p += 1;
				}
				UINT scancode = MapVirtualKey(cShortcut.operator LONG()&0xffff, 0);
				switch(cShortcut.operator LONG()&0xffff)
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
				szTmp[itemsof(szTmp)-1] = _T('\0');
			}
			m_wndShortcut.SetWindowText(szTmp);
		}
	}

	void ExtraInitDialog()
	{
		m_wndShortcut.SubclassWindow(GetDlgItem(IDC_CG_SHORTCUT));
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR bstr(CFGID_OPERATION_SHORTCUT);
			if (a_wParam != VK_MENU && a_wParam != VK_SHIFT && a_wParam != VK_CONTROL &&
				a_wParam != VK_LMENU && a_wParam != VK_LSHIFT && a_wParam != VK_LCONTROL &&
				a_wParam != VK_RMENU && a_wParam != VK_RSHIFT && a_wParam != VK_RCONTROL)
				M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(LONG(a_wParam|
					((GetKeyState(VK_MENU)&0x8000 ? FALT : 0) | (GetKeyState(VK_CONTROL)&0x8000 ? FCONTROL : 0) | (GetKeyState(VK_SHIFT)&0x8000 ? FSHIFT : 0))<<16)));
			else
				M_Config()->ItemValuesSet(1, &(bstr.m_str), CConfigValue(0L));
		}
		catch (...)
		{
		}
		a_bHandled = FALSE;
		return 0;
	}

private:
	CContainedWindowT<CEdit> m_wndShortcut;
};

class ATL_NO_VTABLE CMenuCommandsInsertOperator :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsInsertOperator, &CLSID_MenuCommandsInsertOperator>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsInsertOperator()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsInsertOperator)

BEGIN_CATEGORY_MAP(CMenuCommandsInsertOperator)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsInsertOperator)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Append Effect[0405]Vrstvený obrázek - přidat efekt");
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

			pCfg->ItemInsSimple(CComBSTR(CFGID_INSOP_SYNCID), CMultiLanguageString::GetAuto(L"[0409]Selection sync ID[0405]Synchronizace výběru"), CMultiLanguageString::GetAuto(L"[0409]Image selection is synchronized by the given ID.[0405]Vybraná oblast v obrázku je dopstupná a sychronizována přes zadané ID."), CConfigValue(L"LAYER"), NULL, 0, NULL);

			pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_NAME), CMultiLanguageString::GetAuto(L"[0409]Name[0405]Jméno"), CMultiLanguageString::GetAuto(L"[0409]Text describing this command.[0405]Text popisující tento příkaz."), CConfigValue(L"Operation"), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_DESCRITPION), CMultiLanguageString::GetAuto(L"[0409]Description[0405]Popis"), CMultiLanguageString::GetAuto(L"[0409]Explanation of this command's function.[0405]Vysvětlení funkce tohoto příkazu."), CConfigValue(L""), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_OPERATION_SHORTCUT), CMultiLanguageString::GetAuto(L"[0409]Shortcut[0405]Zkratka"), CMultiLanguageString::GetAuto(L"[0409]Key combination used to activating this operation (only valid if used in main menu).[0405]Klávesová zkratka pro aktivaci této operace (platí pouze pro použití z hlavního menu)."), CConfigValue(0L), NULL, 0, NULL);

			// icon
			CComBSTR cCFGID_ICONID(CFGID_OPERATION_ICONID);
			CComPtr<IConfigItemCustomOptions> pCustIconIDs;
			RWCoCreateInstance(pCustIconIDs, __uuidof(DesignerFrameIconsManager));
			if (pCustIconIDs != NULL)
				pCfg->ItemIns1ofNWithCustomOptions(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon associated with this command.[0405]Ikona přiřazené tomuto příkazu."), CConfigValue(GUID_NULL), pCustIconIDs, NULL, 0, NULL);
			else
				pCfg->ItemInsSimple(cCFGID_ICONID, CMultiLanguageString::GetAuto(L"[0409]Icon[0405]Ikona"), CMultiLanguageString::GetAuto(L"[0409]Icon associated with this command.[0405]Ikona přiřazené tomuto příkazu."), CConfigValue(GUID_NULL), NULL, 0, NULL);

			CComQIPtr<IOperationManager> pMgr(a_pManager);

			M_OperationManager()->InsertIntoConfigAs(pMgr ? pMgr : M_OperationManager(), pCfg, CComBSTR(CFGID_OPERATION_ID), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Document operation associated with this command.[0405]Dokumentová operace spojená s tímto příkazem."), 0, NULL);

			pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEX), NULL, NULL, CConfigValue(-1.0f), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_DLGSIZEY), NULL, NULL, CConfigValue(-1.0f), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_CAPTION), CMultiLanguageString::GetAuto(L"[0409]Window caption[0405]Titulek okna"), CMultiLanguageString::GetAuto(L"[0409]Text in the caption of the window displayed during execution.[0405]Text v titulku okna konfiguračního dialogu."), CConfigValue(L"[0409]Configure Operation[0405]Konfigurovat operaci"), NULL, 0, NULL);
			pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_HELPTOPIC), CMultiLanguageString::GetAuto(L"[0409]Help topic[0405]Téma nápovědy"), CMultiLanguageString::GetAuto(L"[0409]Path to a html help topic that will be displayed if user clicks on a Help button. If this field is left blank, the Help button will be hidden.[0405]Cesta k tématu v html nápovědě, který bude zobrazen po kliknutí na tlačítko Nápověda. Pokud je toto pole prázdné, tlačítko bude skryté."), CConfigValue(L""), NULL, 0, NULL);

			CComPtr<ISubConfig> pHistory;
			RWCoCreateInstance(pHistory, __uuidof(ConfigInMemory));
			pCfg->ItemInsSimple(CComBSTR(CFGID_CFG_HISTORY), CMultiLanguageString::GetAuto(L"[0409]History[0405]Historie"), CMultiLanguageString::GetAuto(L"[0409]If enabled, previous selected values will be remembered.[0405]Je-li povoleno, budou se zaznamenávat dříve vybraná nastavení."), CConfigValue(true), pHistory, 0, NULL);

			CConfigCustomGUI<&CLSID_MenuCommandsInsertOperator, CConfigGUIInsertOperatorDlg>::FinalizeConfig(pCfg);

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

			CConfigValue cName;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_NAME), &cName);
			CConfigValue cDesc;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_DESCRITPION), &cDesc);
			CConfigValue cShortcut;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_SHORTCUT), &cShortcut);
			CConfigValue cIconID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_OPERATION_ICONID), &cIconID);
			CComBSTR bstrCFGID_OPERATION_ID(CFGID_OPERATION_ID);
			CConfigValue cSubItem;
			a_pConfig->ItemValueGet(bstrCFGID_OPERATION_ID, &cSubItem);
			CComPtr<IConfig> pSubCfg;
			a_pConfig->SubConfigGet(bstrCFGID_OPERATION_ID, &pSubCfg);

			CComObject<CDocumentMenuCommand>* p = NULL;
			CComObject<CDocumentMenuCommand>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;

			CComPtr<ILocalizedString> pName;
			pName.Attach(new CMultiLanguageString(cName.Detach().bstrVal));
			CComPtr<ILocalizedString> pDesc;
			pDesc.Attach(new CMultiLanguageString(cDesc.Detach().bstrVal));

			CComQIPtr<IOperationManager> pMgr(a_pManager);

			CComPtr<IStructuredRoot> pSR;
			a_pDocument->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSR));
			if (pSR == NULL)
				a_pDocument->QueryFeatureInterface(__uuidof(IStructuredRoot), reinterpret_cast<void**>(&pSR));
			CComBSTR bstrPrefix;
			if (pSR)
				pSR->StatePrefix(&bstrPrefix);

			p->Init(pName, pDesc, cShortcut, cIconID, pMgr ? pMgr : M_OperationManager(), cSubItem, pSubCfg, a_pStates, a_pConfig, a_pDocument, pDLI, bstrPrefix, a_pView);

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
			pItems->Insert(p);

			*a_ppSubCommands = pItems.Detach();
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

private:
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		CDocumentMenuCommand()
		{
		}
		void Init(ILocalizedString* a_pName, ILocalizedString* a_pDesc, LONG a_nShortcut, GUID const& a_tIconID, IOperationManager* a_pOpsMgr, TConfigValue const& a_cItemID, IConfig* a_pItemCfg, IOperationContext* a_pStates, IConfig* a_pMainCfg, IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, CComBSTR& a_bstrPrefix, IDesignerView* a_pView)
		{
			m_pName = a_pName;
			m_pDesc = a_pDesc;
			m_nShortcut = a_nShortcut;
			m_tIconID = a_tIconID;
			m_pOpsMgr = a_pOpsMgr;
			m_cItemID = a_cItemID;
			m_pItemCfg = a_pItemCfg;
			m_pStates = a_pStates;
			m_pMainCfg = a_pMainCfg;
			m_pDocument = a_pDocument;
			m_bstrPrefix.Attach(a_bstrPrefix.Detach());
			m_pDLI = a_pDLI;
			m_pView = a_pView;
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
				(*a_ppText = m_pName)->AddRef();
				return S_OK;
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			try
			{
				*a_ppText = NULL;
				(*a_ppText = m_pDesc)->AddRef();
				return S_OK;
			}
			catch (...)
			{
				return a_ppText ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			try
			{
				*a_pIconID = m_tIconID;
				return S_OK;
			}
			catch (...)
			{
				return a_pIconID ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				CComPtr<IDesignerFrameIcons> pIcons;
				RWCoCreateInstance(pIcons, __uuidof(DesignerFrameIconsManager));
				return pIcons->GetIcon(m_tIconID, a_nSize, a_phIcon);
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			if ((m_nShortcut&0xffff) == 0)
				return E_NOTIMPL;

			try
			{
				a_pAccel->wKeyCode = m_nShortcut;
				a_pAccel->fVirtFlags = m_nShortcut>>16;
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
			try
			{
				CConfigValue cSel;
				m_pMainCfg->ItemValueGet(CComBSTR(CFGID_INSOP_SYNCID), &cSel);
				CComPtr<ISharedState> pState;
				if (cSel.TypeGet() == ECVTString)
				{
					CComBSTR bstrSel(m_bstrPrefix);
					bstrSel += cSel;
					m_pStates->StateGet(bstrSel, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				}
				CComPtr<IEnumUnknowns> pLayers;
				m_pDLI->StateUnpack(pState, &pLayers);
				ULONG n = 0;
				if (pLayers) pLayers->Size(&n);
				*a_peState = n > 0 ? EMCSNormal : EMCSDisabled;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				if (m_pView)
					m_pView->DeactivateAll(FALSE);

				CConfigValue cSel;
				m_pMainCfg->ItemValueGet(CComBSTR(CFGID_INSOP_SYNCID), &cSel);
				CComPtr<ISharedState> pState;
				CComBSTR bstrSel(m_bstrPrefix);
				if (cSel.TypeGet() == ECVTString)
				{
					bstrSel += cSel;
					m_pStates->StateGet(bstrSel, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
				}
				CComPtr<IEnumUnknowns> pLayers;
				m_pDLI->StateUnpack(pState, &pLayers);
				ULONG n = 0;
				if (pLayers) pLayers->Size(&n);
				if (n == 0)
					return S_FALSE;

				CComPtr<IConfig> pCopy;
				m_pItemCfg->DuplicateCreate(&pCopy);

				if (a_hParent)
				{
					CComPtr<IComparable> pFirst;
					pLayers->Get(0, &pFirst);
					CComPtr<IDocumentBase> pDB;
					RWCoCreateInstance(pDB, __uuidof(DocumentBase));
					m_pDLI->LayerRender(pFirst, NULL, pDB);
					CComQIPtr<IDocument> pD(pDB);
					CComPtr<IDocumentBase> pDBP;
					RWCoCreateInstance(pDBP, __uuidof(DocumentBase));
					pD->DocumentCopy(NULL, pDBP, const_cast<GUID*>(&m_cItemID.operator const GUID &()), pCopy);
					pDB = NULL;
					pD = pDBP;

					CComObjectStackEx<CPreviewOperationContext> cOpCtx;
					CComPtr<IOperationManager> pOpMgr;
					RWCoCreateInstance(pOpMgr, __uuidof(OperationManager));
					COperationPreviewMaker cPreviewMaker(pOpMgr, pD, m_cItemID, pCopy, &cOpCtx);
					CComPtr<IConfigDescriptor> pOA;
					RWCoCreateInstance(pOA, m_cItemID.operator const GUID &());
					CComPtr<IConfigInMemory> pMainCfg;
					RWCoCreateInstance(pMainCfg, __uuidof(ConfigInMemory));
					CComBSTR bstrDLGSIZEX(CFGID_CFG_DLGSIZEX);
					CComBSTR bstrDLGSIZEY(CFGID_CFG_DLGSIZEY);
					CComBSTR bstrCAPTION(CFGID_CFG_CAPTION);
					CComBSTR bstrICONID(CFGID_CFG_ICONID);
					CComBSTR bstrPREVIEW(CFGID_CFG_PREVIEW);
//					CComBSTR bstrPREVIEW_CUSTOMBACKGROUND(L"Preview\\CustomBackground");
//static const OLECHAR CFGID_IMGVIEW_CUSTOMBACKGROUND[] = L;
//static const OLECHAR CFGID_IMGVIEW_BACKGROUNDCOLOR[] = L"BackgroundColor";
//static const OLECHAR CFGID_IMGVIEW_DRAWFRAME[] = L"Frame";
//static const OLECHAR CFGID_IMGVIEW_AUTOZOOM[] = L"AutomaticZoom";
//static const OLECHAR CFGID_IMGVIEW_MYVIEWPORT[] = L"MyViewport";
//static const OLECHAR CFGID_IMGVIEW_CONTROLLEDVIEPORT[] = L"ControlledViewport";
					CComBSTR bstrHELPTOPIC(CFGID_CFG_HELPTOPIC);
					CComBSTR bstrPREVIEWMODE(CFGID_CFG_PREVIEWMODE);
					CComBSTR bstrSPLITPOS(CFGID_CFG_SPLITPOS);
					CComBSTR bstrHISTORY(CFGID_CFG_HISTORY);
					CConfigValue cDlgSizeX;
					m_pMainCfg->ItemValueGet(bstrDLGSIZEX, &cDlgSizeX);
					CConfigValue cDlgSizeY;
					m_pMainCfg->ItemValueGet(bstrDLGSIZEY, &cDlgSizeY);
					CConfigValue cCaption;
					m_pMainCfg->ItemValueGet(bstrCAPTION, &cCaption);
					CConfigValue cHelp;
					m_pMainCfg->ItemValueGet(bstrHELPTOPIC, &cHelp);
					CConfigValue cStr(L"");
					CConfigValue cIconID;
					m_pMainCfg->ItemValueGet(CComBSTR(CFGID_OPERATION_ICONID), &cIconID);
					static GUID const GUID_DesignerViewFactoryImage = {0x9432886A, 0xD01D, 0x40F1, {0x99, 0x7C, 0x0B, 0x8F, 0x7A, 0x72, 0x2B, 0xFA}};
					CConfigValue cPreview(GUID_DesignerViewFactoryImage);
					CConfigValue cPreviewMode(CFGVAL_PM_AUTOSELECT);
					CConfigValue cTrue(true);
					CConfigValue cDefSplit(-1.0f);
					BSTR aBSTRs[] = {bstrDLGSIZEX, bstrDLGSIZEY, bstrSPLITPOS, bstrCAPTION, bstrICONID, bstrPREVIEW, bstrHELPTOPIC, bstrPREVIEWMODE, bstrHISTORY};
					TConfigValue aVals[] = {cDlgSizeX, cDlgSizeY, cDefSplit, cCaption, cIconID, cPreview, cHelp, cPreviewMode, cTrue};
					pMainCfg->ItemValuesSet(itemsof(aBSTRs), aBSTRs, aVals);
					CComPtr<IDesignerViewFactory> pDVFImage;
					RWCoCreateInstance(pDVFImage, GUID_DesignerViewFactoryImage);
					CComPtr<IConfig> pDVICfg;
					pDVFImage->ConfigCreate(NULL, &pDVICfg);
					CComPtr<IConfig> pSubCfg;
					pMainCfg->SubConfigGet(bstrPREVIEW, &pSubCfg);
					CopyConfigValues(pSubCfg, pDVICfg);
					CComPtr<IConfig> pHistOld;
					m_pMainCfg->SubConfigGet(bstrHISTORY, &pHistOld);
					CComPtr<IConfig> pHistNew;
					pMainCfg->SubConfigGet(bstrHISTORY, &pHistNew);
					pHistNew->CopyFrom(pHistOld, NULL);

					UINT const nResult = CShowConfigWithPreviewDlg<>(a_tLocaleID, pCopy, pMainCfg, pOpMgr, pOA, NULL, pD, &cPreviewMaker).DoModal(a_hParent);
					pMainCfg->ItemValueGet(bstrDLGSIZEX, &cDlgSizeX);
					pMainCfg->ItemValueGet(bstrDLGSIZEY, &cDlgSizeY);
					aVals[0] = cDlgSizeX;
					aVals[1] = cDlgSizeY;
					m_pMainCfg->ItemValuesSet(2, aBSTRs, aVals);
					pHistOld->CopyFrom(pHistNew, NULL);

					switch (nResult)
					{
					case IDOK:
						break;
					case IDCONTINUE:
						return S_FALSE;
					default:
						return E_RW_CANCELLEDBYUSER;
					}

					CopyConfigValues(m_pItemCfg, pCopy);
				}

				//CWriteLock<IDocument> cLock(m_pDocument.p);
				//CUndoBlock cUndo(m_pDocument, m_pName);

				//CComPtr<IComparable> pItem;
				//pLayers->Get(0, &pItem);
				//CComPtr<IComparable> pNewSel;
				//m_pDLI->LayerEffectStepAppend(pItem, TRUE, m_cItemID, NULL, &pNewSel);
				//if (pNewSel)
				//{
				//	CComPtr<ISharedState> pNewState;
				//	m_pDLI->StatePack(1, &(pNewSel.p), &pNewState);
				//	m_pStates->StateSet(bstrSel, pNewState);
				//	CSharedStateUndo<IOperationContext>::SaveState(m_pDocument.p, m_pStates, bstrSel, pState);
				//}

				//CUndoBlock cUndo(m_pDocument, m_pName);
				for (ULONG i = 0; i < n; ++i)
				{
					CComPtr<IComparable> pItem;
					pLayers->Get(i, &pItem);
					CComPtr<IConfig> pOldEffect;
					m_pDLI->LayerEffectGet(pItem, &pOldEffect, NULL);
					if (pOldEffect == NULL) continue;
					CComBSTR bstrEffect(L"Effect");
					CComBSTR bstrCount(L"Effect\\SeqSteps");
					CConfigValue cVal;
					pOldEffect->ItemValueGet(bstrEffect, &cVal);
					if (IsEqualGUID(cVal, __uuidof(DocumentOperationNULL)) || IsEqualGUID(cVal, GUID_NULL))
					{
						pOldEffect->ItemValuesSet(1, &(bstrEffect.m_str), m_cItemID);
						CComPtr<IConfig> pSubCfg;
						pOldEffect->SubConfigGet(bstrEffect, &pSubCfg);
						CopyConfigValues(pSubCfg, m_pItemCfg);
						m_pDLI->LayerEffectSet(pItem, pOldEffect);
						continue;
					}
					CConfigValue cSeq(__uuidof(DocumentOperationSequence));
					CConfigValue c0(0L);
					if (!IsEqualGUID(cVal, __uuidof(DocumentOperationSequence)))
					{
						CComPtr<IConfig> pNewEffect;
						pOldEffect->DuplicateCreate(&pNewEffect);
						CComBSTR bstrType(L"Effect\\SeqSteps\\00000000\\SeqType");
						CComBSTR bstrID(L"Effect\\SeqSteps\\00000000\\SeqOperation");
						BSTR aIDs[] = {bstrEffect, bstrCount, bstrType, bstrID};
						CConfigValue c1(1L);
						TConfigValue aVals[] = {cSeq, c1, c0, cVal};
						pNewEffect->ItemValuesSet(4, aIDs, aVals);
						CComPtr<IConfig> pOld;
						pOldEffect->SubConfigGet(bstrEffect, &pOld);
						CComPtr<IConfig> pNew;
						pNewEffect->SubConfigGet(bstrID, &pNew);
						CopyConfigValues(pNew, pOld);
						pOldEffect = pNewEffect;
					}
					CConfigValue cCount;
					pOldEffect->ItemValueGet(bstrCount, &cCount);
					wchar_t szTmp[64];
					_swprintf(szTmp, L"Effect\\SeqSteps\\%08x\\SeqType", cCount.operator LONG());
					CComBSTR bstrType = szTmp;
					_swprintf(szTmp, L"Effect\\SeqSteps\\%08x\\SeqOperation", cCount.operator LONG());
					CComBSTR bstrOpID = szTmp;
					cCount = cCount.operator LONG()+1L;
					BSTR aIDs[] = {bstrCount, bstrType, bstrOpID};
					CConfigValue c1(1L);
					TConfigValue aVals[] = {cCount, c0, m_cItemID};
					pOldEffect->ItemValuesSet(3, aIDs, aVals);
					CComPtr<IConfig> pNew;
					pOldEffect->SubConfigGet(bstrOpID, &pNew);
					CopyConfigValues(pNew, m_pItemCfg);
					m_pDLI->LayerEffectSet(pItem, pOldEffect);
				}
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<ILocalizedString> m_pName;
		CComPtr<ILocalizedString> m_pDesc;
		LONG m_nShortcut;
		GUID m_tIconID;
		CComPtr<IOperationManager> m_pOpsMgr;
		CConfigValue m_cItemID;
		CComPtr<IConfig> m_pItemCfg;
		CComPtr<IOperationContext> m_pStates;
		CComPtr<IConfig> m_pMainCfg;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComBSTR m_bstrPrefix;
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CPreviewOperationContext :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:

	BEGIN_COM_MAP(CPreviewOperationContext)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// IOperationContext methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
			if (i == m_cStates.end()) return E_RW_ITEMNOTFOUND;
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
		STDMETHOD(IsCancelled)() { return S_FALSE; }
		STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining) { return E_NOTIMPL; }
		STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage) { return E_NOTIMPL; }

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
	};

	struct COperationPreviewMaker : public IPreviewMaker
	{
		COperationPreviewMaker(IOperationManager* a_pManager, IDocument* a_pDocument, TConfigValue const& a_tVal, IConfig* a_pCfg, IOperationContext* a_pStates) :
			m_pManager(a_pManager), m_pDocument(a_pDocument), m_tVal(a_tVal), m_pCfg(a_pCfg), m_pStates(a_pStates)
		{
		}

		HRESULT MakePreview(RWHWND a_hParent, LCID a_tLocaleID, IDocument** a_ppPreviewDoc)
		{
			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			if (FAILED(m_pDocument->DocumentCopy(NULL, pBase, &m_tVal.guidVal, m_pCfg)))
				return E_NOTIMPL;
			pBase->QueryInterface(a_ppPreviewDoc);
			if (NULL == *a_ppPreviewDoc)
				return E_NOTIMPL;
			return m_pManager->Activate(m_pManager, *a_ppPreviewDoc, &m_tVal, m_pCfg, m_pStates, a_hParent, a_tLocaleID);
		}

	private:
		IOperationManager* m_pManager;
		IDocument* m_pDocument;
		TConfigValue m_tVal;
		IConfig* m_pCfg;
		IOperationContext* m_pStates;
	};

private:
	CComPtr<IOperationManager> m_pOpsMgr;
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsInsertOperator, CMenuCommandsInsertOperator)
