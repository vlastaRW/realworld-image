
#pragma once


#include <ContextHelpDlg.h>
#include <ObserverImpl.h>
#include <math.h>
#include <SubjectNotImpl.h>
#include <RWProcessingTags.h>
#include "..\RWViewStructure\RWViewStructure.h"
#include "..\RWDesignerServices\RWDesignerServices.h"


class CConfigureLayerEffectDlg :
	public Win32LangEx::CLangDialogImpl<CConfigureLayerEffectDlg>,
	public CDialogResize<CConfigureLayerEffectDlg>,
	public CContextHelpDlg<CConfigureLayerEffectDlg>,
	public CObserverImpl<CConfigureLayerEffectDlg, IConfigObserver, IUnknown*>
{
public:
	CConfigureLayerEffectDlg(IConfig* a_pConfig, LCID a_tLocaleID, IConfig* a_pSizeConfig, IDocumentImage* a_pImage) :
		Win32LangEx::CLangDialogImpl<CConfigureLayerEffectDlg>(a_tLocaleID),
		CContextHelpDlg<CConfigureLayerEffectDlg>(_T("http://www.rw-designer.com/configure-layer-effect")),
		m_pConfig(a_pConfig), m_hIcon(NULL), m_pSizeConfig(a_pSizeConfig), m_pImage(a_pImage),
		m_bUpdating(false), m_nActiveStep(-1), m_nPreviewSize(0), m_nHotTracking(-1)
	{
		RWCoCreateInstance(m_pOM, __uuidof(OperationManager));
	}
	~CConfigureLayerEffectDlg()
	{
		m_cImageList.Destroy();
		m_cOperationIcons.Destroy();
		if (m_hIcon) DestroyIcon(m_hIcon);
	}

	void OwnerNotify(TCookie, IUnknown*)
	{
		if (m_wndSequence.m_hWnd && m_nActiveStep >= 0 && m_nActiveStep < m_wndSequence.GetItemCount())
		{
			CComBSTR bstrName;
			GetStepName(m_cSteps.begin()+m_nActiveStep, bstrName);
			m_wndSequence.SetItemText(m_nActiveStep, 0, bstrName.m_str);
		}
		RefreshPreview();
	}

	bool DoModalPreTranslate(MSG const* a_pMsg)
	{
		if (!m_pCfgWnd)
			return false;
		return m_pCfgWnd->PreTranslateMessage(a_pMsg, TRUE) == S_OK || m_pCfgWnd->PreTranslateMessage(a_pMsg, FALSE) == S_OK;
	}

	enum {IDD = IDD_CONFIGURELAYEREFFECT, ID_LE_REMOVE = 500, ID_LE_MOVEUP, ID_LE_MOVEDOWN/*ID_LE_COPY = 500, ID_LE_PASTE, ID_LE_CLEAR, ID_LE_LOAD, ID_LE_SAVE*/};

	BEGIN_MSG_MAP(CConfigureLayerEffectDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CConfigureLayerEffectDlg>)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawPreview)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnOK)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnCancel)
		NOTIFY_HANDLER(IDC_LE_OPERATIONS, LVN_ITEMACTIVATE, OnOperationActivate)
		NOTIFY_HANDLER(IDC_LE_OPERATIONS, LVN_HOTTRACK/*LVN_ITEMCHANGED*/, OnOperationChanged)
		NOTIFY_HANDLER(IDC_LE_SEQUENCE, LVN_ITEMCHANGED, OnSequenceChanged)
		COMMAND_HANDLER(ID_LE_REMOVE, BN_CLICKED, OnStepRemove)
		COMMAND_HANDLER(ID_LE_MOVEUP, BN_CLICKED, OnStepMoveUp)
		COMMAND_HANDLER(ID_LE_MOVEDOWN, BN_CLICKED, OnStepMoveDown)
		COMMAND_HANDLER(IDC_LE_PREVIEW, STN_CLICKED, OnPreviewClicked)
		CHAIN_MSG_MAP(CDialogResize<CConfigureLayerEffectDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigureLayerEffectDlg)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LE_SEQUENCE, DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_LE_PREVIEWLABEL, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LE_PREVIEW, DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_LE_CONFIG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_LE_OPERATIONS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_LE_TOOLBAR, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CConfigureLayerEffectDlg)
		CTXHELP_CONTROL_STRING(IDC_LE_PREVIEW, L"[0409]Click to change preview image.[0405]Kliněte pro změnu náhledu.")
		//CTXHELP_CONTROL_RESOURCE(IDC_MG_OPERATION, IDS_MG_OPERATION_HELP)
	END_CTXHELP_MAP()

	bool GroupExpanded(int a_nIndex, bool a_bToggle = false)
	{
		if (a_nIndex >= 0 && a_nIndex < 3)
		{
			if (a_bToggle && a_nIndex)
				m_nExpand ^= (1<<a_nIndex);
			return (1|m_nExpand) & (1<<a_nIndex);
		}
		return false;
	}

	// message handlers
public:
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		m_wndToolbar = GetDlgItem(IDC_LE_TOOLBAR);
		m_wndSequence = GetDlgItem(IDC_LE_SEQUENCE);
		m_wndSequence.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP);
		m_wndOperations = GetDlgItem(IDC_LE_OPERATIONS);
		m_wndOperations.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER|LVS_EX_ONECLICKACTIVATE|LVS_EX_TRACKSELECT);
		m_wndOperations.SetHoverTime(1);
		if (XPGUI::IsVista() && CTheme::IsThemingSupported())
		{
			::SetWindowTheme(m_wndOperations, L"explorer", NULL);
		}

		try
		{
			m_nPreviewSize = 128;
			m_pPreview.Attach(new TRasterImagePixel[m_nPreviewSize*m_nPreviewSize]);
			ZeroMemory(m_pPreview.m_p, sizeof*m_pPreview.m_p*m_nPreviewSize*m_nPreviewSize);

			ReadOperations();

			m_hIcon = DPIUtils::PrepareIconForCaption((HICON)::LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_LAYER_OPERATION), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR));
			SetIcon(m_hIcon, 0);

			RECT rcSeq;
			m_wndSequence.GetClientRect(&rcSeq);
			LVCOLUMN tLVC;
			ZeroMemory(&tLVC, sizeof tLVC);
			tLVC.mask = LVCF_WIDTH;
			tLVC.cx = rcSeq.right-GetSystemMetrics(SM_CXVSCROLL);
			m_wndSequence.InsertColumn(0, &tLVC);
			m_cImageList.Create(XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), XPGUI::GetImageListColorFlags(), 5, 0);
			HMODULE hLib = LoadLibraryEx(_T("RWDesignerServices.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
			if (hLib)
			{
				UINT aIcIDs[] = {219, 222, 223};
				for (size_t i = 0; i < itemsof(aIcIDs); ++i)
				{
					HICON hIcon = (HICON)LoadImage(hLib, MAKEINTRESOURCE(aIcIDs[i]), IMAGE_ICON, XPGUI::GetSmallIconSize(), XPGUI::GetSmallIconSize(), LR_DEFAULTCOLOR);
					m_cImageList.AddIcon(hIcon);
					DestroyIcon(hIcon);
				}
			}
			m_wndToolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
			m_wndToolbar.SetButtonStructSize(sizeof(TBBUTTON));
			m_wndToolbar.SetImageList(m_cImageList);
			CComBSTR bstr1;
			CMultiLanguageString::GetLocalized(L"[0409]Remove this step[0405]Odstranit tento krok", m_tLocaleID, &bstr1);
			CComBSTR bstr2;
			CMultiLanguageString::GetLocalized(L"[0409]Move effect up[0405]Posunout efekt nahoru", m_tLocaleID, &bstr2);
			CComBSTR bstr3;
			CMultiLanguageString::GetLocalized(L"[0409]Move effect down[0405]Posunout efekt dolů", m_tLocaleID, &bstr3);
			CAutoVectorPtr<wchar_t> pszCmds(new wchar_t[bstr1.Length()+bstr2.Length()+bstr3.Length()+4]);
			wcscpy(pszCmds.m_p, bstr1);
			wcscpy(pszCmds.m_p+bstr1.Length()+1, bstr2);
			wcscpy(pszCmds.m_p+bstr1.Length()+bstr2.Length()+2, bstr3);
			pszCmds[bstr1.Length()+bstr2.Length()+bstr3.Length()+3] = L'\0';
			m_wndToolbar.AddStrings(pszCmds.m_p);
			TBBUTTON atButtons[] =
			{
				{0, ID_LE_REMOVE, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_AUTOSIZE, TBBUTTON_PADDING, 0, 0},
				{0, 0, 0, BTNS_SEP, TBBUTTON_PADDING, 0, 0},
				{1, ID_LE_MOVEUP, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_AUTOSIZE, TBBUTTON_PADDING, 0, 1},
				{2, ID_LE_MOVEDOWN, TBSTATE_ENABLED, BTNS_BUTTON|BTNS_SHOWTEXT|BTNS_AUTOSIZE, TBBUTTON_PADDING, 0, 2},
			};
			m_wndToolbar.AddButtons(itemsof(atButtons), atButtons);
			RECT rcTB;
			m_wndToolbar.GetItemRect(itemsof(atButtons)-1, &rcTB);
			ULONG nB = rcTB.bottom;
			m_wndToolbar.GetWindowRect(&rcTB);
			ScreenToClient(&rcTB);
			rcTB.bottom = rcTB.top+nB;
			m_wndToolbar.MoveWindow(&rcTB);
			m_wndToolbar.ShowWindow(SW_SHOW);

			CWindow wnd = GetDlgItem(IDC_LE_CONFIG);
			RECT rc;
			wnd.GetWindowRect(&rc);
			ScreenToClient(&rc);
			//rc.top = rcTB.bottom;
			RWCoCreateInstance(m_pCfgWnd, __uuidof(AutoConfigWnd));
			m_pCfgWnd->Create(m_hWnd, &rc, IDC_LE_CONFIG, m_tLocaleID, FALSE, ECWBMMargin);
			//m_pCfgWnd->ConfigSet(m_pConfig);
			wnd.DestroyWindow();

			DlgResize_Init();

			if (m_pSizeConfig != 0)
			{
				CConfigValue cExpand;
				m_pSizeConfig->ItemValueGet(CComBSTR(CFGID_EXPAND), &cExpand);
				m_nExpand = cExpand;
				CConfigValue cPreview;
				m_pSizeConfig->ItemValueGet(CComBSTR(CFGID_PREVIEW), &cPreview);
				m_bPreview = cPreview;

				CConfigValue cSizeX;
				m_pSizeConfig->ItemValueGet(CComBSTR(CFGID_SIZEX), &cSizeX);
				CConfigValue cSizeY;
				m_pSizeConfig->ItemValueGet(CComBSTR(CFGID_SIZEY), &cSizeY);
				if (cSizeX.operator LONG() != CW_USEDEFAULT && cSizeX.operator LONG() != CW_USEDEFAULT)
				{
					RECT rc;
					GetWindowRect(&rc);
					rc.right = rc.left + cSizeX.operator LONG();
					rc.bottom = rc.top + cSizeY.operator LONG();
					MoveWindow(&rc);
				}
			}

			CComPtr<IPlugInCache> pPIC;
			RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
			CClassSet cBannedOps;
			cBannedOps.insert(__uuidof(DocumentOperationNULL));
			cBannedOps.insert(__uuidof(DocumentOperationSequence));
			cBannedOps.insert(__uuidof(DocumentOperationSetFileFormat));
			cBannedOps.insert(__uuidof(DocumentOperationExtractSubDocument));
			GetCategoryPlugIns(pPIC, CATID_TagImageFile, cBannedOps);
			GetCategoryPlugIns(pPIC, CATID_TagInteractive, cBannedOps);
			GetCategoryPlugIns(pPIC, CATID_TagWindowState, cBannedOps);
			GetCategoryPlugIns(pPIC, CATID_TagLayeredImage, cBannedOps);
			CClassSet cLayerStyles;
			GetCategoryPlugIns(pPIC, CATID_TagLayerStyle, cLayerStyles);
			CClassSet cMetaOps;
			GetCategoryPlugIns(pPIC, CATID_TagMetaOp, cMetaOps);

			CComObjectStackEx<CNonResizableRasterImage> cNROC;

			CComPtr<IDesignerFrameIcons> pDFI;
			RWCoCreateInstance(pDFI, __uuidof(DesignerFrameIconsManager));

			m_cOperationIcons.Create(48, 48, XPGUI::GetImageListColorFlags(), 16, 8);
			m_wndOperations.SetImageList(m_cOperationIcons, LVSIL_NORMAL);

			// insert groups
			LVGROUP tGroup;
			ZeroMemory(&tGroup, sizeof(tGroup));
			tGroup.cbSize = XPGUI::IsVista() ? sizeof(tGroup) : LVGROUP_V5_SIZE;
			tGroup.mask = XPGUI::IsVista() ? LVGF_HEADER|LVGF_ALIGN|LVGF_GROUPID|LVGF_STATE : LVGF_HEADER|LVGF_ALIGN|LVGF_GROUPID;
			tGroup.iGroupId = 1;
			tGroup.stateMask = LVGS_COLLAPSED|LVGS_COLLAPSIBLE;
			tGroup.uAlign = LVGA_HEADER_LEFT;

			static LPCWSTR s_aCatNames[] =
			{
				L"[0409]Recommended for layer styles[0405]Doporučeno pro styly vrstev",
				L"[0409]Additional image effects[0405]Další obrázkové efekty",
				L"[0409]Meta-filters[0405]Meta-filtry",
			};
			for (tGroup.iGroupId = 1; tGroup.iGroupId < 4; ++tGroup.iGroupId)
			{
				CComBSTR bstr;
				CMultiLanguageString::GetLocalized(s_aCatNames[tGroup.iGroupId-1], m_tLocaleID, &bstr);
				tGroup.pszHeader = bstr;
				tGroup.cchHeader = wcslen(tGroup.pszHeader);
				tGroup.state = GroupExpanded(tGroup.iGroupId-1) ? LVGS_NORMAL|LVGS_COLLAPSIBLE : LVGS_COLLAPSED|LVGS_COLLAPSIBLE;//tGroup.iGroupId = 1 ? LVGS_NORMAL|LVGS_COLLAPSIBLE : LVGS_NORMAL|LVGS_COLLAPSIBLE|LVGS_COLLAPSED;
				m_wndOperations.InsertGroup(-1, &tGroup);
			}

			m_wndOperations.EnableGroupView(TRUE);

			ULONG nOps = 0;
			m_pOM->ItemGetCount(&nOps);
			for (ULONG i = 0; i < nOps; ++i)
			{
				CConfigValue cVal;
				CComPtr<ILocalizedString> pName;
				m_pOM->ItemIDGet(NULL, i, &cVal, &pName);
				if (cBannedOps.find(cVal.operator const GUID &()) != cBannedOps.end() ||
					m_pOM->CanActivate(m_pOM, &cNROC, cVal, NULL, NULL) != S_OK)
					continue;
				CComPtr<IConfigDescriptor> pOA;
				RWCoCreateInstance(pOA, cVal.operator const GUID &());
				CComBSTR bstrName;
				if (pOA)
				{
					CComPtr<ILocalizedString> pDisplayName;
					pOA->Name(m_pOM, NULL, &pDisplayName);
					if (pDisplayName)
						pDisplayName->GetLocalized(m_tLocaleID, &bstrName);
				}
				HICON hIcon = NULL;
				pDFI->GetIcon(cVal, 48, &hIcon);
				int nIcon = -1;
				if (hIcon == NULL)
				{
					static GUID const tOpGreenID = {0x91A7D922, 0x8B41, 0x4953, {0x9F, 0x80, 0x64, 0x26, 0xB6, 0x41, 0xAF, 0x47}};
					pDFI->GetIcon(tOpGreenID, 48, &hIcon);
				}
				if (hIcon)
				{
					nIcon = m_cOperationIcons.AddIcon(hIcon);
					DestroyIcon(hIcon);
				}
				if (bstrName.Length() == 0)
					pName->GetLocalized(m_tLocaleID, &bstrName);
				int iItem = m_wndOperations.InsertItem(i, bstrName, nIcon);
				GUID* g = new GUID;
				*g = cVal;
				m_wndOperations.SetItemData(iItem, reinterpret_cast<DWORD_PTR>(g));
				LVITEM t;
				ZeroMemory(&t, sizeof t);
				t.mask = LVIF_GROUPID;
				t.iItem = iItem;
				t.iGroupId = cLayerStyles.find(cVal.operator const GUID &()) != cLayerStyles.end() ? 1 : (cMetaOps.find(cVal.operator const GUID &()) != cMetaOps.end() ? 3 : 2);
				m_wndOperations.SetItem(&t);
			}

			UpdateOpList();
			RefreshPreview();

			// center the dialog on the screen
			CenterWindow();
		}
		catch (...)
		{
		}

		return TRUE;
	}
	LRESULT OnDestroy(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		if (m_pWatched)
			m_pWatched->ObserverDel(CObserverImpl<CConfigureLayerEffectDlg, IConfigObserver, IUnknown*>::ObserverGet(), 0);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnDrawPreview(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam != IDC_LE_PREVIEW)
		{
			a_bHandled = FALSE;
			return 0;
		}
		DRAWITEMSTRUCT* pDIS = reinterpret_cast<DRAWITEMSTRUCT*>(a_lParam);
		FillRect(pDIS->hDC, &pDIS->rcItem, (HBRUSH)(COLOR_3DFACE+1));
		BITMAPINFO tBMI;
		ZeroMemory(&tBMI, sizeof tBMI);
		tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
		tBMI.bmiHeader.biWidth = m_nPreviewSize;
		tBMI.bmiHeader.biHeight = -m_nPreviewSize;
		tBMI.bmiHeader.biPlanes = 1;
		tBMI.bmiHeader.biBitCount = 32;
		tBMI.bmiHeader.biCompression = BI_RGB;
		SetDIBitsToDevice(pDIS->hDC, (pDIS->rcItem.right+pDIS->rcItem.left-m_nPreviewSize)>>1, (pDIS->rcItem.bottom+pDIS->rcItem.top-m_nPreviewSize)>>1, m_nPreviewSize, m_nPreviewSize, 0, 0, 0, m_nPreviewSize, m_pPreview.m_p, &tBMI, DIB_RGB_COLORS);
		return 0;
	}

	LRESULT OnOK(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND a_hWndCtl, BOOL& a_bHandled)
	{
		WriteOperations(m_cSteps, m_pConfig);
		OnCancel(BN_CLICKED, IDOK, a_hWndCtl, a_bHandled);
		return 0;
	}
	LRESULT OnCancel(WORD UNREF(a_wNotifyCode), WORD a_wID, HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_wndOperations.m_hWnd)
		{
			LVGROUP tGroup;
			ZeroMemory(&tGroup, sizeof(tGroup));
			tGroup.cbSize = sizeof(tGroup);
			tGroup.mask = LVGF_STATE;
			tGroup.iGroupId = 1;
			tGroup.stateMask = LVGS_COLLAPSED;
			for (tGroup.iGroupId = 1; tGroup.iGroupId < 4; ++tGroup.iGroupId)
			{
				tGroup.state = GroupExpanded(tGroup.iGroupId-1) ? LVGS_COLLAPSED : LVGS_NORMAL;
				m_wndOperations.GetGroupInfo(tGroup.iGroupId, &tGroup);
				if (((tGroup.state&LVGS_COLLAPSED) && GroupExpanded(tGroup.iGroupId-1)) ||
					((tGroup.state&LVGS_COLLAPSED) == 0 && !GroupExpanded(tGroup.iGroupId-1)))
					GroupExpanded(tGroup.iGroupId-1, true);
			}

			int n = m_wndOperations.GetItemCount();
			for (int i = 0; i < n; ++i)
				delete reinterpret_cast<GUID*>(m_wndOperations.GetItemData(i));
			m_wndOperations.DeleteAllItems();
		}
		if (m_pSizeConfig != 0)
		{
			RECT rc;
			GetWindowRect(&rc);
			BSTR aIDs[4];
			CComBSTR bstrIDX(CFGID_SIZEX);
			CComBSTR bstrIDY(CFGID_SIZEY);
			CComBSTR bstrIDE(CFGID_EXPAND);
			CComBSTR bstrIDP(CFGID_PREVIEW);
			aIDs[0] = bstrIDX;
			aIDs[1] = bstrIDY;
			aIDs[2] = bstrIDE;
			aIDs[3] = bstrIDP;
			TConfigValue aVals[4];
			aVals[0] = CConfigValue(rc.right-rc.left);
			aVals[1] = CConfigValue(rc.bottom-rc.top);
			aVals[2] = CConfigValue(m_nExpand);
			aVals[3] = CConfigValue(m_bPreview);
			m_pSizeConfig->ItemValuesSet(4, aIDs, aVals);
		}
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnPreviewClicked(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		m_bPreview = !m_bPreview;
		RefreshPreview();
		return 0;
	}

	LRESULT OnOperationActivate(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		if (m_nActiveStep < 0)
			return 0;
		NMITEMACTIVATE* pNMIA = reinterpret_cast<NMITEMACTIVATE*>(a_pNMHDR);
		if (ULONG(m_nActiveStep) >= m_cSteps.size())
		{
			m_nActiveStep = m_cSteps.size();
			m_cSteps.resize(m_cSteps.size()+1);
		}
		m_cSteps[m_nActiveStep].first = *reinterpret_cast<GUID*>(m_wndOperations.GetItemData(pNMIA->iItem));
		m_cSteps[m_nActiveStep].second = NULL;
		m_pOM->CreateConfig(CConfigValue(m_cSteps[m_nActiveStep].first), &m_cSteps[m_nActiveStep].second);
		UpdateOpList(m_nActiveStep);
		RefreshPreview();
		return 0;
	}
	LRESULT OnOperationChanged(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		NMLISTVIEW* pNMLV = reinterpret_cast<NMLISTVIEW*>(a_pNMHDR);
		if (m_nHotTracking != pNMLV->iItem)
		{
			m_nHotTracking = pNMLV->iItem;
			RefreshPreview();
		}
		//if (pNMLV->uNewState & LVIS_H)
		return S_OK;
	}
	LRESULT OnSequenceChanged(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		NMLISTVIEW* pNMLV = reinterpret_cast<NMLISTVIEW*>(a_pNMHDR);
		if (pNMLV->uNewState & LVIS_SELECTED)
		{
			if (ULONG(pNMLV->iItem) >= m_cSteps.size())
			{
				m_pCfgWnd->Show(FALSE);
				m_wndToolbar.ShowWindow(FALSE);
				m_wndOperations.ShowWindow(TRUE);
				GetDlgItem(IDC_LE_OPERATIONS_LABEL).ShowWindow(TRUE);
			}
			else
			{
				m_wndOperations.ShowWindow(FALSE);
				GetDlgItem(IDC_LE_OPERATIONS_LABEL).ShowWindow(FALSE);
				m_pCfgWnd->Show(TRUE);
				m_pCfgWnd->ConfigSet(m_cSteps[pNMLV->iItem].second, ECPMFull);
				if (m_pWatched)
					m_pWatched->ObserverDel(CObserverImpl<CConfigureLayerEffectDlg, IConfigObserver, IUnknown*>::ObserverGet(), 0);
				m_pWatched = m_cSteps[pNMLV->iItem].second;
				if (m_pWatched)
					m_pWatched->ObserverIns(CObserverImpl<CConfigureLayerEffectDlg, IConfigObserver, IUnknown*>::ObserverGet(), 0);

				m_wndToolbar.ShowWindow(TRUE);
				m_wndToolbar.EnableButton(ID_LE_MOVEUP, pNMLV->iItem > 0);
				m_wndToolbar.EnableButton(ID_LE_MOVEDOWN, ULONG(pNMLV->iItem+1) < m_cSteps.size());
			}
			m_nActiveStep = pNMLV->iItem;
		}
		return 0;
	}
	LRESULT OnStepRemove(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_nActiveStep >= 0 && ULONG(m_nActiveStep) < m_cSteps.size())
		{
			m_cSteps.erase(m_cSteps.begin()+m_nActiveStep);
			UpdateOpList();
			RefreshPreview();
		}
		return 0;
	}
	LRESULT OnStepMoveUp(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_nActiveStep > 0 && ULONG(m_nActiveStep) < m_cSteps.size())
		{
			std::pair<GUID, CComPtr<IConfig> > t = m_cSteps[m_nActiveStep];
			m_cSteps[m_nActiveStep] = m_cSteps[m_nActiveStep-1];
			m_cSteps[m_nActiveStep-1] = t;
			UpdateOpList(m_nActiveStep-1);
			RefreshPreview();
		}
		return 0;
	}
	LRESULT OnStepMoveDown(WORD UNREF(a_wNotifyCode), WORD UNREF(a_wID), HWND UNREF(a_hWndCtl), BOOL& UNREF(a_bHandled))
	{
		if (m_nActiveStep >= 0 && ULONG(m_nActiveStep+1) < m_cSteps.size())
		{
			std::pair<GUID, CComPtr<IConfig> > t = m_cSteps[m_nActiveStep];
			m_cSteps[m_nActiveStep] = m_cSteps[m_nActiveStep+1];
			m_cSteps[m_nActiveStep+1] = t;
			UpdateOpList(m_nActiveStep+1);
			RefreshPreview();
		}
		return 0;
	}

private:
	typedef std::vector<std::pair<GUID, CComPtr<IConfig> > > CSteps;
	template<class T>
	struct lessBinary
	{
		bool operator()(const T& a_1, const T& a_2) const
		{
			return memcmp(&a_1, &a_2, sizeof(T)) < 0;
		}
	};
	typedef std::set<GUID, lessBinary<GUID> > CClassSet;

	class ATL_NO_VTABLE CNonResizableRasterImage : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public CSubjectNotImpl<IDocument, IDocumentObserver, ULONG>,
		public CSubjectNotImpl<IDocumentRasterImage, IImageObserver, TImageChange>
	{
	public:

	BEGIN_COM_MAP(CNonResizableRasterImage)
		COM_INTERFACE_ENTRY(IDocument)
		COM_INTERFACE_ENTRY(IBlockOperations)
		COM_INTERFACE_ENTRY(IDocumentRasterImage)
		COM_INTERFACE_ENTRY(IDocumentImage)
	END_COM_MAP()

		// IBlockOperations methods
	public:
		STDMETHOD(WriteLock)() { return S_FALSE; }
		STDMETHOD(WriteUnlock)() { return S_FALSE; }
		STDMETHOD(ReadLock)() { return S_FALSE; }
		STDMETHOD(ReadUnlock)() { return S_FALSE; }

		// IDocument methods
	public:
		STDMETHOD(BuilderID)(CLSID* a_pguidBuilder) { return E_NOTIMPL; }
		STDMETHOD(QueryFeatureInterface)(REFIID a_iid, void** a_ppFeatureInterface)
		{
			return QueryInterface(a_iid, a_ppFeatureInterface);
		}
		STDMETHOD(LocationGet)(IStorageFilter** a_ppLocation) { return E_NOTIMPL; }
		STDMETHOD(LocationSet)(IStorageFilter* a_pLocation) { return E_NOTIMPL; }
		STDMETHOD(EncoderGet)(CLSID* a_pEncoderID, IConfig** a_ppConfig) { return E_NOTIMPL; }
		STDMETHOD(EncoderSet)(REFCLSID a_tEncoderID, IConfig* a_pConfig) { return E_NOTIMPL; }
		STDMETHOD(EncoderAspects)(IEnumEncoderAspects* a_pEnumAspects) { return E_NOTIMPL; }
		STDMETHOD(IsDirty)() { return E_NOTIMPL; }
		STDMETHOD(SetDirty)() { return E_NOTIMPL; }
		STDMETHOD(ClearDirty)() { return E_NOTIMPL; }
		STDMETHOD(DocumentCopy)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect) { return E_NOTIMPL; }
		STDMETHOD(QuickInfo)(ULONG /*a_nInfoIndex*/, ILocalizedString** /*a_ppInfo*/) { return E_NOTIMPL; }
		STDMETHOD(DocumentTypeGet)(IDocumentType** a_ppDocumentType) { return E_NOTIMPL; }

		// IDocumentImage methods
	public:
		STDMETHOD(CanvasGet)(TImageSize* a_pCanvasSize, TImageResolution*, TImagePoint*, TImageSize*, EImageOpacity*)
		{
			try
			{
				a_pCanvasSize->nX = a_pCanvasSize->nY = 1;
				return S_OK;
			}
			catch (...)
			{
				return a_pCanvasSize ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(ChannelsGet)(ULONG*, float*, IEnumImageChannels*) { return E_NOTIMPL; }
		STDMETHOD(TileGet)(ULONG, TImagePoint const*, TImageSize const*, TImageStride const*, ULONG, TPixelChannel*, ITaskControl*, EImageRenderingIntent) { return E_NOTIMPL; }
		STDMETHOD(Inspect)(ULONG, TImagePoint const*, TImageSize const*, IImageVisitor*, ITaskControl*, EImageRenderingIntent) { return E_NOTIMPL; }
		STDMETHOD(BufferLock)(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent) { return E_NOTIMPL; }
		STDMETHOD(BufferUnlock)(ULONG a_nChannelID, TPixelChannel const* a_pBuffer) { return E_NOTIMPL; }

		// IDocumentEditableImage methods
	public:
		STDMETHOD(CanvasSet)(TImageSize const*, TImageResolution const*, TMatrix3x3f const*, IRasterImageTransformer*) { return E_NOTIMPL; }
		STDMETHOD(ChannelsSet)(ULONG, EImageChannelID const*, TPixelChannel const*) { return E_NOTIMPL; }

		// IDocumentRasterImage methods
	public:
		STDMETHOD(TileSet)(ULONG, TImagePoint const*, TImageSize const*, TImageStride const*, ULONG, TPixelChannel const*, BYTE) { return E_NOTIMPL; }
		STDMETHOD(BufferReplace)(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter) { return E_NOTIMPL; }
		STDMETHOD(BufferAllocate)(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter) { return E_NOTIMPL; }
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
			return S_OK;
		}

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
	};

private:
	void GetCategoryPlugIns(IPlugInCache* a_pPIC, GUID const& a_tCatID, CClassSet& a_cSet)
	{
		CComPtr<IEnumGUIDs> pCLSIDs;
		a_pPIC->CLSIDsEnum(a_tCatID, 0xffffffff, &pCLSIDs);
		ULONG nCLSIDs = 0;
		if (pCLSIDs) pCLSIDs->Size(&nCLSIDs);
		for (ULONG i = 0; i < nCLSIDs; ++i)
		{
			GUID t = GUID_NULL;
			pCLSIDs->Get(i, &t);
			a_cSet.insert(t);
		}
	}

	void ReadOperations()
	{
		CComPtr<IConfig> pDup;
		m_pConfig->DuplicateCreate(&pDup);
		CComBSTR bstrEffect(L"Effect");
		CConfigValue cEffect;
		pDup->ItemValueGet(bstrEffect, &cEffect);
		if (IsEqualGUID(cEffect, __uuidof(DocumentOperationNULL)))
		{
		}
		else if (IsEqualGUID(cEffect, __uuidof(DocumentOperationSequence)))
		{
			CComPtr<IConfig> pSeq;
			pDup->SubConfigGet(bstrEffect, &pSeq);
			CConfigValue cSteps;
			pSeq->ItemValueGet(CComBSTR(L"SeqSteps"), &cSteps);
			CComPtr<IConfig> pSteps;
			pSeq->SubConfigGet(CComBSTR(L"SeqSteps"), &pSteps);
			for (LONG i = 0; i < cSteps.operator LONG(); ++i)
			{
				OLECHAR sz[32];
				swprintf(sz, L"%08x\\SeqOperation", i);
				CComBSTR bstrSubID(sz);
				CConfigValue cStep;
				pSteps->ItemValueGet(bstrSubID, &cStep);
				std::pair<GUID, CComPtr<IConfig> > tOp;
				tOp.first = cStep.operator const GUID &();
				pSteps->SubConfigGet(bstrSubID, &tOp.second);
				m_cSteps.push_back(tOp);
			}
		}
		else
		{
			std::pair<GUID, CComPtr<IConfig> > tOp;
			tOp.first = cEffect.operator const GUID &();
			pDup->SubConfigGet(bstrEffect, &tOp.second);
			m_cSteps.push_back(tOp);
		}
	}
	static void WriteOperations(CSteps const& a_cSteps, IConfig* a_pConfig)
	{
		CComBSTR bstrEffect(L"Effect");
		if (a_cSteps.size() == 0)
		{
			a_pConfig->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(__uuidof(DocumentOperationNULL)));
		}
		else if (a_cSteps.size() == 1)
		{
			a_pConfig->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(a_cSteps[0].first));
			CComPtr<IConfig> p;
			a_pConfig->SubConfigGet(bstrEffect, &p);
			if (p && a_cSteps[0].second)
				CopyConfigValues(p, a_cSteps[0].second);
		}
		else
		{
			a_pConfig->ItemValuesSet(1, &(bstrEffect.m_str), CConfigValue(__uuidof(DocumentOperationSequence)));
			CComPtr<IConfig> pSeq;
			a_pConfig->SubConfigGet(bstrEffect, &pSeq);
			CComBSTR bstrSteps(L"SeqSteps");
			pSeq->ItemValuesSet(1, &(bstrSteps.m_str), CConfigValue(LONG(a_cSteps.size())));
			CComPtr<IConfig> pSteps;
			pSeq->SubConfigGet(bstrSteps, &pSteps);
			for (ULONG i = 0; i < a_cSteps.size(); ++i)
			{
				OLECHAR sz[32];
				swprintf(sz, L"%08x\\SeqType", i);
				CComBSTR bstrSubType(sz);
				pSteps->ItemValuesSet(1, &(bstrSubType.m_str), CConfigValue(0L));
				swprintf(sz, L"%08x\\SeqOperation", i);
				CComBSTR bstrSubID(sz);
				pSteps->ItemValuesSet(1, &(bstrSubID.m_str), CConfigValue(a_cSteps[i].first));
				CComPtr<IConfig> p;
				pSteps->SubConfigGet(bstrSubID, &p);
				if (p && a_cSteps[i].second)
					CopyConfigValues(p, a_cSteps[i].second);
			}
		}
	}
	void GetStepName(CSteps::const_iterator i, CComBSTR& bstrName)
	{
		CComPtr<IDocumentOperation> pOp;
		RWCoCreateInstance(pOp, i->first);
		CComQIPtr<IConfigDescriptor> pOA(pOp);
		if (pOA)
		{
			CComPtr<ILocalizedString> pDisplayName;
			pOA->Name(m_pOM, i->second, &pDisplayName);
			if (pDisplayName)
				pDisplayName->GetLocalized(m_tLocaleID, &bstrName);
		}
		if (bstrName.Length() == 0)
		{
			CComPtr<ILocalizedString> pStr;
			if (pOp) pOp->NameGet(NULL, &pStr);
			if (pStr) pStr->GetLocalized(m_tLocaleID, &bstrName);
		}
		if (bstrName.Length() == 0)
			bstrName = L"Unknown effect";
	}
	void UpdateOpList(int a_iSel = -1)
	{
		int iSel = a_iSel == -1 ? m_wndSequence.GetSelectedIndex() : a_iSel;
		if (iSel < 0) iSel = 0;
		m_wndSequence.DeleteAllItems();

		for (CSteps::const_iterator i = m_cSteps.begin(); i != m_cSteps.end(); ++i)
		{
			CComBSTR bstrName;
			GetStepName(i, bstrName);
			m_wndSequence.InsertItem(m_wndSequence.GetItemCount(), bstrName.m_str);
		}
		CComBSTR bstrAdd;
		CMultiLanguageString::GetLocalized(L"[0409]< add new effect >[0405]< přidat nový efekt >", m_tLocaleID, &bstrAdd);
		m_wndSequence.InsertItem(m_wndSequence.GetItemCount(), bstrAdd);
		if (m_wndSequence.GetItemCount() > iSel)
			m_wndSequence.SelectItem(iSel);
		else
			m_wndSequence.SelectItem(m_wndSequence.GetItemCount()-1);
	}

	void RefreshPreview()
	{
		CSteps cSteps = m_cSteps;
		CComPtr<IConfig> pConfig;
		m_pConfig->DuplicateCreate(&pConfig);
		if (m_nActiveStep == m_cSteps.size() && m_nHotTracking >= 0 && m_nHotTracking < m_wndOperations.GetItemCount())
		{
			int const n = cSteps.size();
			cSteps.resize(n+1);
			cSteps[n].first = *reinterpret_cast<GUID*>(m_wndOperations.GetItemData(m_nHotTracking));
			cSteps[n].second = NULL;
			m_pOM->CreateConfig(CConfigValue(cSteps[n].first), &cSteps[n].second);
		}
		WriteOperations(cSteps, pConfig);
		UpdatePreview(pConfig);
		GetDlgItem(IDC_LE_PREVIEW).Invalidate(FALSE);
		GetDlgItem(IDC_LE_PREVIEW).RedrawWindow();
	}
	void UpdatePreview(IConfig* a_pConfig)
	{
		CComPtr<IDocumentFactoryRasterImage> pFact;
		RWCoCreateInstance(pFact, __uuidof(DocumentFactoryRasterImage));
		TImageSize tSize = {m_nPreviewSize, m_nPreviewSize};
		CAutoVectorPtr<TPixelChannel> pPixels(new TPixelChannel[tSize.nX*tSize.nY]);
		bool bValid = false;
		if (m_bPreview && m_pImage)
		{
			TImagePoint tOrig = {0, 0};
			TImageSize tSize = {0, 0};
			m_pImage->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
			if (tSize.nX > 16 && tSize.nY > 16)
			{
				tOrig.nX -= m_nPreviewSize>>4;//LONG(m_nPreviewSize-tSize.nX)>>1;
				tOrig.nY -= m_nPreviewSize>>4;//LONG(m_nPreviewSize-tSize.nY)>>1;
				tSize.nX = tSize.nY = m_nPreviewSize;
				m_pImage->TileGet(EICIRGBA, &tOrig, &tSize, NULL, tSize.nX*tSize.nY, pPixels, NULL, EIRIPreview);
				ULONG const n = pPixels.m_p->n;
				TPixelChannel const* p = pPixels.m_p+1;
				for (TPixelChannel const* pEnd = pPixels.m_p+tSize.nX*tSize.nY; p != pEnd; ++p)
					if (p->n != n)
					{
						bValid = true;
						break;
					}
			}
		}
		if (!bValid)
		{
			static TPixelChannel const tEmpty = {0, 0, 0, 0};
			static TPixelChannel const tFill = {50, 50, 200, 255};
			for (ULONG nY = 0; nY < m_nPreviewSize; ++nY)
			{
				TPixelChannel* p = pPixels.m_p+(nY*m_nPreviewSize);
				float const f = fabsf(nY-m_nPreviewSize*0.5f-0.5f);
				float const f2 = f*f;
				float const f3 = ((m_nPreviewSize*m_nPreviewSize*9)>>6)-f2;
				if (f3 > 0.25f)
				{
					LONG const n = sqrtf(f3)+0.5f;
					LONG m = 0;
					if (nY > (m_nPreviewSize>>1))
					{
						float const g = fabsf(nY-m_nPreviewSize*0.65f-0.5f);
						float const g2 = g*g;
						float const g3 = ((m_nPreviewSize*m_nPreviewSize)>>7)-g2;
						if (g3 > 0.25f)
							m = sqrtf(g3)+0.5f;
					}
					std::fill_n(p, (m_nPreviewSize>>1)-n, tEmpty);
					std::fill_n(p+(m_nPreviewSize>>1)-n, n+n, tFill);
					if (m > 0)
						std::fill_n(p+int(m_nPreviewSize*0.35f)-m, m+m, tEmpty);
					std::fill_n(p+(m_nPreviewSize>>1)+n, m_nPreviewSize-((m_nPreviewSize>>1)+n), tEmpty);
				}
				else
				{
					std::fill_n(p, m_nPreviewSize, tEmpty);
				}
			}
		}
		CComPtr<IDocumentBase> pBase;
		RWCoCreateInstance(pBase, __uuidof(DocumentBase));
		if (FAILED(pFact->Create(NULL, pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA), 2.2f, CImageTile(tSize.nX, tSize.nY, pPixels))))
			return;
		CComQIPtr<IDocument> pDoc(pBase);
		CComBSTR bstrEffect(L"Effect");
		CConfigValue cEffect;
		a_pConfig->ItemValueGet(bstrEffect, &cEffect);
		CComPtr<IConfig> pEffect;
		a_pConfig->SubConfigGet(bstrEffect, &pEffect);
		CComObject<CPreviewOperationContext>* pPOC = NULL;
		CComObject<CPreviewOperationContext>::CreateInstance(&pPOC);
		CComPtr<IOperationContext> pOC = pPOC;
		m_pOM->Activate(m_pOM, pDoc, cEffect, pEffect, pOC, NULL, m_tLocaleID);
		CComPtr<IDocumentRasterImage> pDRI;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI));
		if (m_pPreview)
		{
			pDRI->TileGet(EICIRGBA, NULL, CImageSize(m_nPreviewSize, m_nPreviewSize), NULL, m_nPreviewSize*m_nPreviewSize, reinterpret_cast<TPixelChannel*>(m_pPreview.m_p), NULL, EIRIPreview);
			COLORREF clrBG = GetSysColor(COLOR_3DFACE);
			ULONG const nBgR = GetRValue(clrBG);
			ULONG const nBgG = GetGValue(clrBG);
			ULONG const nBgB = GetBValue(clrBG);
			TRasterImagePixel* p = m_pPreview;
			for (TRasterImagePixel* pEnd = p+m_nPreviewSize*m_nPreviewSize; p < pEnd; ++p)
			{
				p->bR = (int(p->bA)*p->bR+(255-p->bA)*nBgR)/255;
				p->bG = (int(p->bA)*p->bG+(255-p->bA)*nBgG)/255;
				p->bB = (int(p->bA)*p->bB+(255-p->bA)*nBgB)/255;
				p->bA = 255;
			}
		}
	}

private:
	CComPtr<IOperationManager> m_pOM;
	CComPtr<IConfig> m_pConfig;
	CComPtr<IConfig> m_pWatched;
	CComPtr<IDocumentImage> m_pImage;
	CSteps m_cSteps;
	CComPtr<IConfig> m_pSizeConfig;
	bool m_bPreview;
	LONG m_nExpand;
	CComPtr<IConfigWnd> m_pCfgWnd;
	CToolBarCtrl m_wndToolbar;
	CImageList m_cImageList;
	HICON m_hIcon;
	CListViewCtrl m_wndSequence;
	int m_nActiveStep;
	CListViewCtrl m_wndOperations;
	CImageList m_cOperationIcons;
	bool m_bUpdating;
	CAutoVectorPtr<TRasterImagePixel> m_pPreview;
	ULONG m_nPreviewSize;
	LONG m_nHotTracking;
};
