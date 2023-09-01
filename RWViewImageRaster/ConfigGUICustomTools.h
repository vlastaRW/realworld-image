
#pragma once

#include <ConfigCustomGUIList.h>

extern __declspec(selectany) GUID const TMenuCommandsStorageCtx = {0xcdfc8178, 0x581d, 0x4371, {0xae, 0x61, 0x80, 0x7c, 0x76, 0xeb, 0xf5, 0x5c}};


class ATL_NO_VTABLE CConfigGUICustomToolsDlg :
	public CCustomConfigWndImpl<CConfigGUICustomToolsDlg>,
	public CCustomConfigGUIList<CConfigGUICustomToolsDlg, IDC_CT_LIST, IDC_CT_TOOLBAR, IDC_CT_CONFIG, IDS_LISTBUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, ECWBMNothing>,
	public CDialogResize<CConfigGUICustomToolsDlg>
{
public:
	CConfigGUICustomToolsDlg() :
		CCustomConfigGUIList<CConfigGUICustomToolsDlg, IDC_CT_LIST, IDC_CT_TOOLBAR, IDC_CT_CONFIG, IDS_LISTBUTTONNAMES, 0, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, 0, CCGUILIST_DEFAULTICON, CCGUILIST_DEFAULTICON, ECWBMNothing>(CFGID_CT_COMMANDS)
	{
	}
	enum
	{
		IDD = IDD_CONFIGGUI_CUSTOMTOOLS,
		ID_IMPORT = 3000,
		ID_EXPORT,
	};

	BEGIN_MSG_MAP(CConfigGUICustomToolsDlg)
		COMMAND_ID_HANDLER(ID_IMPORT, OnImport)
		COMMAND_ID_HANDLER(ID_EXPORT, OnExport)
		NOTIFY_HANDLER(IDC_CT_LIST, LVN_ITEMCHANGED, OnListItemChanged)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUICustomToolsDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUICustomToolsDlg>)
		CHAIN_MSG_MAP(ListClass)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUICustomToolsDlg)
		DLGRESIZE_CONTROL(IDC_CT_LIST, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CT_CONFIG, DLSZ_SIZE_X|DLSZ_DIVMOVE_Y(2))
		DLGRESIZE_CONTROL(IDC_CT_HIDEMANAGER, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CT_TOOLLABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_CT_TOOLSYNC, DLSZ_MOVE_Y|DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CT_SEPLINE, DLSZ_SIZE_X|DLSZ_MOVE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICustomToolsDlg)
		CONFIGITEM_CHECKBOX(IDC_CT_HIDEMANAGER, CFGID_CT_HIDEMANAGER)
		CONFIGITEM_EDITBOX(IDC_CT_TOOLSYNC, CFGID_CT_TOOLSYNC)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		static TBBUTTON aButtons[] =
		{
			{ 0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
			{ 5, ID_IMPORT, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX, TBBUTTON_PADDING, 0, 5},
			{ 6, ID_EXPORT, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_NOPREFIX, TBBUTTON_PADDING, 0, 6},
		};
		CToolBarCtrl wndTB(GetDlgItem(IDC_CT_TOOLBAR));
		CImageList cIL(wndTB.GetImageList());
		SIZE sz;
		cIL.GetIconSize(sz);
		HICON h;
		HMODULE hLib = LoadLibraryEx(_T("RWDesignerCore.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hLib)
		{
			cIL.AddIcon(h = (HICON)::LoadImage(hLib, MAKEINTRESOURCE(226), IMAGE_ICON, sz.cx, sz.cy, LR_DEFAULTCOLOR)); DestroyIcon(h);
			cIL.AddIcon(h = (HICON)::LoadImage(hLib, MAKEINTRESOURCE(232), IMAGE_ICON, sz.cx, sz.cy, LR_DEFAULTCOLOR)); DestroyIcon(h);
			FreeLibrary(hLib);
		}
		wndTB.AddButtons(itemsof(aButtons), aButtons);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	void ExtraConfigNotify()
	{
		UpdateList();
	}

	LRESULT OnListItemChanged(int a_idCtrl, LPNMHDR a_pNMHDR, BOOL& a_bHandled)
	{
		int nSel = CListViewCtrl(GetDlgItem(IDC_CT_LIST)).GetSelectedIndex();
		CToolBarCtrl(GetDlgItem(IDC_CT_TOOLBAR)).EnableButton(ID_EXPORT, nSel != -1);
		a_bHandled = FALSE;
		return 0;
	}

	LRESULT OnImport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"rwpreset");
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(IDS_RWTOOLS_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(NULL, EFTOpenExisting, m_hWnd, pDocTypes, NULL, TMenuCommandsStorageCtx, _SharedStringTable.GetStringAuto(IDS_RWTOOLS_OPEN), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;

		CComPtr<IDataSrcDirect> pSrc;
		pStorage->SrcOpen(&pSrc);
		ULONG nSize = 0;
		if (pSrc == NULL || FAILED(pSrc->SizeGet(&nSize)) || nSize == 0)
		{
			// TODO: message
			return 0;
		}
		CDirectInputLock cData(pSrc, nSize);

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		pMemCfg->DataBlockSet(nSize, cData.begin());

		CConfigValue cVal;
		CComBSTR bstrRootCfgID(CFGID_CT_COMMANDS);
		if (SUCCEEDED(M_Config()->ItemValueGet(bstrRootCfgID, &cVal)))
		{
			cVal = cVal.operator LONG()+1;
			M_Config()->ItemValuesSet(1, &(bstrRootCfgID.m_str), cVal);
			OLECHAR szNameID[64];
			_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_CT_COMMANDS, cVal.operator LONG()-1);
			CComBSTR bstrProfileName(szNameID);
			CComPtr<IConfig> pDstCfg;
			M_Config()->SubConfigGet(bstrProfileName, &pDstCfg);
			CopyConfigValues(pDstCfg, pMemCfg);
			CListViewCtrl(GetDlgItem(IDC_CT_LIST)).SetItemState(cVal.operator LONG()-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}

		return 0;
	}
	LRESULT OnExport(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		int nSel = CListViewCtrl(GetDlgItem(IDC_CT_LIST)).GetSelectedIndex();
		if (nSel == -1)
			return 0;

		OLECHAR szNameID[64];
		_snwprintf(szNameID, itemsof(szNameID), L"%s\\%08x", CFGID_CT_COMMANDS, nSel);
		CComBSTR bstrNameID(szNameID);
		CComPtr<IConfig> pSrcCfg;
		M_Config()->SubConfigGet(bstrNameID, &pSrcCfg);
		if (pSrcCfg == NULL)
			return 0;

		CComPtr<IConfigInMemory> pMemCfg;
		RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
		CopyConfigValues(pMemCfg, pSrcCfg);
		ULONG nSize = 0;
		pMemCfg->DataBlockGetSize(&nSize);
		if (nSize == 0)
			return 0;

		CAutoVectorPtr<BYTE> pMem(new BYTE[nSize]);

		pMemCfg->DataBlockGet(nSize, pMem.m_p);

		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"rwpreset");
		CComBSTR bstrFilter(L"*.");
		bstrFilter += bstrExt;
		pDocType->InitEx(_SharedStringTable.GetStringAuto(IDS_RWTOOLS_FILTER), NULL, 1, &(bstrExt.m_str), NULL, NULL, 0, bstrFilter);
		CComPtr<IEnumUnknownsInit> pDocTypes;
		RWCoCreateInstance(pDocTypes, __uuidof(EnumUnknowns));
		pDocTypes->Insert(pDocType);
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pStorage;
		pStMgr->FilterCreateInteractivelyUID(NULL, EFTCreateNew, m_hWnd, pDocTypes, NULL, TMenuCommandsStorageCtx, _SharedStringTable.GetStringAuto(IDS_RWTOOLS_SAVE), NULL, m_tLocaleID, &pStorage);
		if (pStorage == NULL)
			return 0;
		CComPtr<IDataDstStream> pDst;
		pStorage->DstOpen(&pDst);
		if (pDst == NULL || FAILED(pDst->Write(nSize, pMem.m_p)) || FAILED(pDst->Close()))
		{
			// TODO: message
		}

		return 0;
	}
};

#include <ConfigCustomGUIIcons.h>
#include <WTL_ColorPicker.h>

extern __declspec(selectany) GUID const TCUSTOMTOOLCONFIGGUIID = {0xe17e9e29, 0xd23c, 0x4321, {0xb8, 0x8c, 0x56, 0xcf, 0xcf, 0x76, 0x54, 0x64}};

class ATL_NO_VTABLE CConfigGUICustomToolDlg :
	public CCustomConfigWndWithIcons<CConfigGUICustomToolDlg>,
	public CDialogResize<CConfigGUICustomToolDlg>
{
public:
	CConfigGUICustomToolDlg() : m_wndShortcut(this, 1)
	{
	}

	enum { IDD = IDD_CONFIGGUI_CUSTOMTOOL };

	BEGIN_MSG_MAP(CConfigGUICustomToolDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUICustomToolDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUICustomToolDlg>)
		NOTIFY_HANDLER(IDC_CT_OUTCOLOR, CButtonColorPicker::BCPN_SELCHANGE, OnOutColorChanged)
		REFLECT_NOTIFICATIONS()
	ALT_MSG_MAP(1)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SYSKEYDOWN, OnKeyDown)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUICustomToolDlg)
		DLGRESIZE_CONTROL(IDC_CT_NAME, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_SHORTCUTLABEL, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_SHORTCUT, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_ICONIDLABEL, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CT_ICONID, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CT_DESC, DLSZ_SIZE_X)

		//DLGRESIZE_CONTROL(IDC_CT_USEBLENDMODE, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_BLENDMODE, DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_CT_USERASTERMODE, DLSZ_DIVMOVE_X(3))
		DLGRESIZE_CONTROL(IDC_CT_RASTERMODE, DLSZ_DIVMOVE_X(3)|DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_CT_USECOORDSMODE, DLSZ_MULDIVMOVE_X(2, 3))
		DLGRESIZE_CONTROL(IDC_CT_COORDSMODE, DLSZ_MULDIVMOVE_X(2, 3)|DLSZ_DIVSIZE_X(3))

		//DLGRESIZE_CONTROL(IDC_CT_USETOOLID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CT_TOOLID, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_TOOLCFG, DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_CT_USESTYLEID, DLSZ_DIVMOVE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_STYLEID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CT_STYLECFG, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2)|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICustomToolDlg)
		CONFIGITEM_CONTEXTHELP(IDC_CT_SHORTCUT, CFGID_CT_COMMANDSHORTCUT)
		CONFIGITEM_EDITBOX(IDC_CT_NAME, CFGID_CT_COMMANDNAME)
		CONFIGITEM_ICONCOMBO(IDC_CT_ICONID, CFGID_CT_COMMANDICONID)
		CONFIGITEM_EDITBOX(IDC_CT_DESC, CFGID_CT_COMMANDDESC)
		CONFIGITEM_CHECKBOX(IDC_CT_USETOOLID, CFGID_CT_USETOOLID)
		CONFIGITEM_EDITBOX(IDC_CT_TOOLID, CFGID_CT_TOOLID)
		//CONFIGITEM_EDITBOX(IDC_CT_TOOLCONFIG, CFGID_CT_TOOLCONFIG)
		CONFIGITEM_CHECKBOX(IDC_CT_USESTYLEID, CFGID_CT_USESTYLEID)
		CONFIGITEM_EDITBOX(IDC_CT_STYLEID, CFGID_CT_STYLEID)
		//CONFIGITEM_EDITBOX(IDC_CT_STYLECONFIG, CFGID_CT_STYLECONFIG)
		CONFIGITEM_CHECKBOX(IDC_CT_USEBLENDMODE, CFGID_CT_USEBLENDMODE)
		CONFIGITEM_COMBOBOX(IDC_CT_BLENDMODE, CFGID_CT_BLENDMODE)
		CONFIGITEM_CHECKBOX(IDC_CT_USERASTERMODE, CFGID_CT_USERASTERMODE)
		CONFIGITEM_COMBOBOX(IDC_CT_RASTERMODE, CFGID_CT_RASTERMODE)
		CONFIGITEM_CHECKBOX(IDC_CT_USECOORDSMODE, CFGID_CT_USECOORDSMODE)
		CONFIGITEM_COMBOBOX(IDC_CT_COORDSMODE, CFGID_CT_COORDSMODE)
		CONFIGITEM_CHECKBOX(IDC_CT_USEOUTLINE, CFGID_CT_USEOUTLINE)
		CONFIGITEM_CHECKBOX(IDC_CT_OUTENABLED, CFGID_CT_OUTENABLED)
		CONFIGITEM_EDITBOX(IDC_CT_OUTWIDTH, CFGID_CT_OUTWIDTH)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		// initialize color buttons
		m_wndOutColor.m_tLocaleID = m_tLocaleID;
		m_wndOutColor.SubclassWindow(GetDlgItem(IDC_CT_OUTCOLOR));
		m_wndOutColor.SetDefaultText(NULL);
	}
	void ExtraConfigNotify()
	{
		CConfigValue cShortcut;
		M_Config()->ItemValueGet(CComBSTR(CFGID_CT_COMMANDSHORTCUT), &cShortcut);
		if (cShortcut.TypeGet() == ECVTInteger && m_wndShortcut.m_hWnd)
		{
			TCHAR szTmp[128] = _T("");
			if (cShortcut.operator LONG() == 0)
			{
				Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_NOSHORTCUT, szTmp, itemsof(szTmp), LANGIDFROMLCID(m_tLocaleID));
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
		CConfigValue cUseOutline;
		M_Config()->ItemValueGet(CComBSTR(CFGID_CT_USEOUTLINE), &cUseOutline);
		CConfigValue cColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_CT_OUTCOLOR), &cColor);
		if (cUseOutline)
		{
			m_wndOutColor.SetColor(CButtonColorPicker::SColor(cColor[0], cColor[1], cColor[2], cColor[3]));
			m_wndOutColor.EnableWindow(TRUE);
		}
		else
		{
			m_wndOutColor.EnableWindow(FALSE);
		}
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndShortcut.SubclassWindow(GetDlgItem(IDC_CT_SHORTCUT));

		BOOL b;
		CCustomConfigWndWithIcons<CConfigGUICustomToolDlg>::OnInitDialog(a_uMsg, a_wParam, a_lParam, b);

		DlgResize_Init(false, false, 0);

		return 1;
	}

	LRESULT OnKeyDown(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		try
		{
			CComBSTR bstr(CFGID_CT_COMMANDSHORTCUT);
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
	LRESULT OnOutColorChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_CT_COLOR(CFGID_CT_OUTCOLOR);
			CConfigValue cValColor(pClrBtn->clr.fR, pClrBtn->clr.fG, pClrBtn->clr.fB, pClrBtn->clr.fA);
			BSTR aIDs[1];
			aIDs[0] = cCFGID_CT_COLOR;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

private:
	CContainedWindowT<CEdit> m_wndShortcut;
	CButtonColorPicker m_wndOutColor;
	//CComPtr<IRasterImageEditToolsManager> pToolsMgr;
	//CComPtr<IRasterImageFillStyleManager> pStyleMgr;
};
