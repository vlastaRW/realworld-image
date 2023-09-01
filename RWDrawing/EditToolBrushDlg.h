// EditToolBrushDlg.h : Declaration of the CEditToolBrushDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"


// CEditToolLineDlg

class CEditToolBrushDlg : 
	public CEditToolDlgBase<CEditToolBrushDlg, CEditToolDataBrush>,
	public CDialogResize<CEditToolBrushDlg>,
	public CContextHelpDlg<CEditToolBrushDlg>
{
public:
	CEditToolBrushDlg() : m_bBlurDisabled(false), m_bScatterDisabled(false)
	{
	}
	~CEditToolBrushDlg()
	{
		m_cIcons.Destroy();
	}

	enum { IDD = IDD_EDITTOOL_BRUSH, ID_BRUSH_ROUND = 200, ID_BRUSH_SQUARE, ID_BRUSH_DIAGONAL, ID_BRUSH_CUSTOM };

	BEGIN_MSG_MAP(CEditToolBrushDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolBrushDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDC_ET_BRUSHSHAPE, CBN_SELCHANGE, OnChange)
		COMMAND_HANDLER(IDC_ET_SIZECHECK, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_BLURCHECK, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_FLOWCHECK, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_SIZEEDIT, EN_CHANGE, OnChangeSize)
		COMMAND_HANDLER(IDC_ET_BLUREDIT, EN_CHANGE, OnChangeBlur)
		COMMAND_HANDLER(IDC_ET_FLOWEDIT, EN_CHANGE, OnChangeFlow)
		COMMAND_HANDLER(IDC_ET_SCATTER, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_ROTATE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(IDC_ET_RESIZE, BN_CLICKED, OnChange)
		COMMAND_HANDLER(ID_BRUSH_ROUND, BN_CLICKED, OnClickedRoundBrush)
		COMMAND_HANDLER(ID_BRUSH_SQUARE, BN_CLICKED, OnClickedSquareBrush)
		COMMAND_HANDLER(ID_BRUSH_DIAGONAL, BN_CLICKED, OnClickedDiagonalBrush)
		COMMAND_HANDLER(ID_BRUSH_CUSTOM, BN_CLICKED, OnClickedImageBrush)
		//COMMAND_HANDLER(IDC_ET_SMOOTHING_CHECK, BN_CLICKED, OnChange)//OnClickedSmoothing)
		NOTIFY_HANDLER(IDC_ET_SIZESPIN, UDN_DELTAPOS, OnUpDownChangeSize)
		NOTIFY_HANDLER(IDC_ET_BLURSPIN, UDN_DELTAPOS, OnUpDownChangeBlur)
		NOTIFY_HANDLER(IDC_ET_FLOWSPIN, UDN_DELTAPOS, OnUpDownChangeFlow)
		CHAIN_MSG_MAP(CDialogResize<CEditToolBrushDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolBrushDlg)
		//DLGRESIZE_CONTROL(IDC_ET_BRUSHSHAPE, DLSZ_SIZE_X)
		//DLGRESIZE_CONTROL(IDC_ET_BRUSHIMAGE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZEEDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZESPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SIZEUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_BLUREDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_BLURSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_BLURUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_FLOWEDIT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_ET_FLOWSPIN, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_FLOWUNIT, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_ET_SCATTER, DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_ET_ROTATE, DLSZ_DIVMOVE_X(3)|DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_ET_RESIZE, DLSZ_MULDIVMOVE_X(2, 3)|DLSZ_DIVSIZE_X(3))
		DLGRESIZE_CONTROL(IDC_ET_SMOOTHING_SLIDER, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolBrushDlg)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_BRUSHSHAPE, IDS_HELP_BRUSHSHAPE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SIZEEDIT, IDS_HELP_BRUSHSIZE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SIZESPIN, IDS_HELP_BRUSHSIZE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BLUREDIT, IDS_HELP_BRUSHBLUR)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BLURSPIN, IDS_HELP_BRUSHBLUR)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FLOWEDIT, IDS_HELP_BRUSHFLOW)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FLOWSPIN, IDS_HELP_BRUSHFLOW)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_SIZECHECK, IDS_HELP_BRUSHPRESSURE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_BLURCHECK, IDS_HELP_BRUSHPRESSURE)
		CTXHELP_CONTROL_RESOURCE(IDC_ET_FLOWCHECK, IDS_HELP_BRUSHPRESSURE)
		//CTXHELP_CONTROL_STRING(IDC_ET_BRUSHIMAGE, L"[0409]Select a custom image brush.[0405]Vybrat vlastní obrázkový štětec.")
		CTXHELP_CONTROL_STRING(IDC_ET_SCATTER, L"[0409]Randomly move the painting position.[0405]Náhodně posunovat místo vykreslování.")
		CTXHELP_CONTROL_STRING(IDC_ET_ROTATE, L"[0409]Randomly rotate the brush shape.[0405]Náhodně otáčet obrázkový štětec.")
		CTXHELP_CONTROL_STRING(IDC_ET_RESIZE, L"[0409]Randomly change the brush size.[0405]Náhodně měnit velikost štětce.")
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolBrushDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataBrush::ISharedStateToolData> pData(a_tParams.pState);
			if (pData)
			{
				m_cData = *(pData->InternalData());
				DataToGUI();
			}
		}
	}

	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			return Win32LangEx::GetDialogSize(_pModule->get_m_hInst(), IDD, a_pSize, m_tLocaleID) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* UNREF(a_pState))
	{
		return S_OK;
	}

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndSize = GetDlgItem(IDC_ET_SIZEEDIT);
		m_wndBlur = GetDlgItem(IDC_ET_BLUREDIT);
		m_wndFlow = GetDlgItem(IDC_ET_FLOWEDIT);
		AddWindowForPreTranslate(m_wndSize);
		AddWindowForPreTranslate(m_wndBlur);
		AddWindowForPreTranslate(m_wndFlow);

		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataBrush::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataBrush::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		m_wndToolBar = GetDlgItem(IDC_ET_BRUSHSHAPE);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CComPtr<IIconRenderer> pIR;
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));

		int nIconSize = XPGUI::GetSmallIconSize();
		ULONG nIconDelta = (nIconSize>>1)-8;
		ULONG nGapLeft = nIconDelta>>1;
		ULONG nGapRight = nIconDelta-nGapLeft;
		RECT padding = {nGapLeft, 0, nGapRight, 0};
		m_cIcons.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), 4, 0);
		HICON hTmp;

		static IRPolyPoint const s_aRound[] =
		{
			{0.5f, 0.0f}, {0.6913f, 0.0381f}, {0.8536f, 0.1464f}, {0.9619f, 0.3087f}, //0.1913f 0.3536f, 0.4619f
			{1.0f, 0.5f}, {0.9619f, 0.6913f}, {0.8536f, 0.8536f}, {0.6913f, 0.9619f},
			{0.5f, 1.0f}, {0.3087f, 0.9619f}, {0.1464f, 0.8536f}, {0.0381f, 0.6913f},
			{0.0f, 0.5f}, {0.0381f, 0.3087f}, {0.1464f, 0.1464f}, {0.3087f, 0.0381f},
		};
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aRound), s_aRound, nIconSize, false, nIconSize+nIconDelta);
		m_cIcons.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aSquare[] =
		{ {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aSquare), s_aSquare, nIconSize, false, nIconSize+nIconDelta);
		m_cIcons.AddIcon(hTmp); DestroyIcon(hTmp);

		static IRPolyPoint const s_aDiagonal[] =
		{ {1.0f, 0.0f}, {1.0f, 0.2f}, {0.2f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.8f}, {0.8f, 0.0f} };
		hTmp = IconFromPolygon(pSI, pIR, itemsof(s_aDiagonal), s_aDiagonal, nIconSize, false, nIconSize+nIconDelta);
		m_cIcons.AddIcon(hTmp); DestroyIcon(hTmp);

		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(nIconSize);
			pSI->GetLayers(ESIPicture, cRenderer, IRTarget(1, 0, 1));
			pSI->GetLayers(ESIFolderSimple, cRenderer, IRTarget(0.65f, 1, -1));
			hTmp = cRenderer.get(padding);
			m_cIcons.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}

		wchar_t szTooltipStrings[1024] = L"";
		wchar_t* pD = szTooltipStrings;
		static wchar_t const* const s_aNames[] =
		{
			L"[0409]Circle[0405]Kruh",
			L"[0409]Rectangle[0405]Čtverec",
			L"[0409]Diagonal[0405]Úhlopříčka",
			L"[0409]Custom image[0405]Vlastní obrázek",
		};
		for (wchar_t const* const* p = s_aNames; p != s_aNames+sizeof(s_aNames)/sizeof(*s_aNames); ++p)
		{
			CComBSTR bstr;
			CMultiLanguageString::GetLocalized(*p, m_tLocaleID, &bstr);
			wcscpy(pD, bstr.m_str == NULL ? L"" : bstr);
			pD += bstr.Length()+1;
			*pD = L'\0';
		}

		m_wndToolBar.SetImageList(m_cIcons);
		m_wndToolBar.AddStrings(szTooltipStrings);
		TBBUTTON aButtons[] =
		{
			{0, ID_BRUSH_ROUND, TBSTATE_ENABLED|(m_cData.eShape == CEditToolDataBrush::ESRound ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 0},
			{1, ID_BRUSH_SQUARE, TBSTATE_ENABLED|(m_cData.eShape == CEditToolDataBrush::ESSquare ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 1},
			{2, ID_BRUSH_DIAGONAL, TBSTATE_ENABLED|(m_cData.eShape == CEditToolDataBrush::ESDiagonal ? TBSTATE_CHECKED : 0), BTNS_BUTTON|BTNS_CHECKGROUP, TBBUTTON_PADDING, 0, 2},
			{3, ID_BRUSH_CUSTOM, TBSTATE_ENABLED, BTNS_BUTTON, TBBUTTON_PADDING, 0, 3},
		};
		m_wndToolBar.AddButtons(itemsof(aButtons), aButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
			m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);

		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		int iDelta = (rcToolbar.bottom-rcToolbar.top)-(rcActual.bottom-rcActual.top);
		rcActual.top = ((rcActual.bottom+rcActual.top)>>1)-(rcToolbar.bottom>>1);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
		rcActual.right = rcActual.left+rcToolbar.right;
		m_wndToolBar.MoveWindow(&rcActual, FALSE);

		DataToGUI();

		DlgResize_Init(false, false, 0);

		return 1;  // Let the system set the focus
	}
	LRESULT OnClickedSomething(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		return 0;
	}
	LRESULT OnChange(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		GUIToData();
		DataToGUI();
		DataToState();
		return 0;
	}
	LRESULT OnChangeSize(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fSize))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnChangeBlur(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndBlur.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fBlur))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnChangeFlow(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		TCHAR szVal[32] = _T("");
		m_wndFlow.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		if (1 != _stscanf(szVal, _T("%f"), &m_cData.fFlow))
			return 0;
		GUIToData();
		DataToState();
		return 0;
	}
	LRESULT OnUpDownChangeSize(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fSize > 0.0f)
			{
				m_cData.fSize = max(0.0f, m_cData.fSize-1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fSize);
				m_wndSize.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fSize < 500.0f)
			{
				m_cData.fSize = min(500.0f, m_cData.fSize+1.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fSize);
				m_wndSize.SetWindowText(szTmp);
			}
		}

		return 0;
	}
	LRESULT OnUpDownChangeBlur(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fBlur > 0.0f)
			{
				m_cData.fBlur = max(0.0f, m_cData.fBlur-10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fBlur);
				m_wndBlur.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fBlur < 100.0f)
			{
				m_cData.fBlur = min(100.0f, m_cData.fBlur+10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fBlur);
				m_wndBlur.SetWindowText(szTmp);
			}
		}

		return 0;
	}
	LRESULT OnUpDownChangeFlow(int UNREF(a_idCtrl), LPNMHDR a_pNMHDR, BOOL& UNREF(a_bHandled))
	{
		LPNMUPDOWN pNMUD = reinterpret_cast<LPNMUPDOWN>(a_pNMHDR);

		if (pNMUD->iDelta > 0)
		{
			if (m_cData.fFlow > 0.0f)
			{
				m_cData.fFlow = max(0.0f, m_cData.fFlow-10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fFlow);
				m_wndFlow.SetWindowText(szTmp);
			}
		}
		else
		{
			if (m_cData.fFlow < 100.0f)
			{
				m_cData.fFlow = min(100.0f, m_cData.fFlow+10.0f);
				TCHAR szTmp[32] = _T("");
				_stprintf(szTmp, _T("%g"), m_cData.fFlow);
				m_wndFlow.SetWindowText(szTmp);
			}
		}

		return 0;
	}
	LRESULT OnClickedRoundBrush(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_cData.eShape != CEditToolDataBrush::ESRound)
		{
			m_cData.eShape = CEditToolDataBrush::ESRound;
			DataToGUI();
			DataToState();
		}
		return 0;
	}
	LRESULT OnClickedSquareBrush(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_cData.eShape != CEditToolDataBrush::ESSquare)
		{
			m_cData.eShape = CEditToolDataBrush::ESSquare;
			DataToGUI();
			DataToState();
		}
		return 0;
	}
	LRESULT OnClickedDiagonalBrush(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_cData.eShape != CEditToolDataBrush::ESDiagonal)
		{
			m_cData.eShape = CEditToolDataBrush::ESDiagonal;
			DataToGUI();
			DataToState();
		}
		return 0;
	}
	LRESULT OnClickedImageBrush(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		CComBSTR bstr(m_cData.strBrushPath.c_str());
		CComPtr<IStorageManager> pStMgr;
		RWCoCreateInstance(pStMgr, __uuidof(StorageManager));
		CComPtr<IStorageFilter> pLoc;
		static const GUID tID = {0x9f046363, 0x06a1, 0x4852, {0xa1, 0xe3, 0x47, 0xa0, 0x6d, 0x6d, 0x14, 0xac}};
		CComPtr<IInputManager> pInMgr;
		RWCoCreateInstance(pInMgr, __uuidof(InputManager));
		CComPtr<IEnumUnknownsInit> pBuilders;
		RWCoCreateInstance(pBuilders, __uuidof(EnumUnknowns));
		{
			CComPtr<IEnumUnknowns> pB1;
			pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentAnimation), &pB1);
			pBuilders->InsertFromEnum(pB1);
			CComPtr<IEnumUnknowns> pB2;
			pInMgr->GetCompatibleBuilders(1, &__uuidof(IDocumentImage), &pB2);
			pBuilders->InsertFromEnum(pB2);
		}
		CComPtr<IEnumUnknowns> pTypes;
		pInMgr->DocumentTypesEnumEx(pBuilders, &pTypes);
		pStMgr->FilterCreateInteractivelyUID(bstr, EFTOpenExisting, m_hWnd, pTypes, NULL, tID, CMultiLanguageString::GetAuto(L"[0409]Select Brush Pattern[0405]Vybrat vzor pro štětec"), NULL, m_tLocaleID, &pLoc);
		if (pLoc)
		{
			CComBSTR bstr2;
			pLoc->ToText(NULL, &bstr2);
			if (bstr2.Length())
			{
				if (wcscmp(m_cData.strBrushPath.c_str(), bstr2.m_str))
				{
					m_cData.strBrushPath = bstr2.m_str;
					m_cData.eShape = CEditToolDataBrush::ESCustom;
					DataToGUI();
					DataToState();
				}
			}
		}
		return 0;
	}

	void GUIToData()
	{
		TCHAR szVal[32] = _T("");
		m_wndSize.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fSize);
		m_wndBlur.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fBlur);
		m_wndFlow.GetWindowText(szVal, itemsof(szVal));
		szVal[itemsof(szVal)-1] = _T('\0');
		_stscanf(szVal, _T("%f"), &m_cData.fFlow);
		m_cData.bSize = IsDlgButtonChecked(IDC_ET_SIZECHECK);
		m_cData.bBlur = IsDlgButtonChecked(IDC_ET_BLURCHECK);
		m_cData.bFlow = IsDlgButtonChecked(IDC_ET_FLOWCHECK);

		m_cData.bScatter = IsDlgButtonChecked(IDC_ET_SCATTER);
		m_cData.bRotate = IsDlgButtonChecked(IDC_ET_ROTATE);
		m_cData.bResize = IsDlgButtonChecked(IDC_ET_RESIZE);
	}
	void DataToGUI()
	{
		m_bEnableUpdates = false;
		m_wndToolBar.CheckButton(ID_BRUSH_ROUND, m_cData.eShape == CEditToolDataBrush::ESRound);
		m_wndToolBar.CheckButton(ID_BRUSH_SQUARE, m_cData.eShape == CEditToolDataBrush::ESSquare);
		m_wndToolBar.CheckButton(ID_BRUSH_DIAGONAL, m_cData.eShape == CEditToolDataBrush::ESDiagonal);
		TCHAR szPrev[33] = _T("");
		m_wndSize.GetWindowText(szPrev, itemsof(szPrev));
		float f = m_cData.fSize+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fSize)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fSize);
			m_wndSize.SetWindowText(szTmp);
		}
		m_wndBlur.GetWindowText(szPrev, itemsof(szPrev));
		f = m_cData.fBlur+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fBlur)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fBlur);
			m_wndBlur.SetWindowText(szTmp);
		}
		bool bBlurDisabled = m_cData.eShape == CEditToolDataBrush::ESCustom;
		if (bBlurDisabled != m_bBlurDisabled)
		{
			GetDlgItem(IDC_ET_BLURCHECK).EnableWindow(m_bBlurDisabled);
			GetDlgItem(IDC_ET_BLURLABEL).EnableWindow(m_bBlurDisabled);
			m_wndBlur.EnableWindow(m_bBlurDisabled);
			GetDlgItem(IDC_ET_BLURSPIN).EnableWindow(m_bBlurDisabled);
			GetDlgItem(IDC_ET_BLURUNIT).EnableWindow(m_bBlurDisabled);
			GetDlgItem(IDC_ET_FLOWCHECK).EnableWindow(m_bBlurDisabled);
			m_bBlurDisabled = bBlurDisabled;
		}
		m_wndFlow.GetWindowText(szPrev, itemsof(szPrev));
		f = m_cData.fFlow+1;
		_stscanf(szPrev, _T("%f"), &f);
		if (f != m_cData.fFlow)
		{
			TCHAR szTmp[32] = _T("");
			_stprintf(szTmp, _T("%g"), m_cData.fFlow);
			m_wndFlow.SetWindowText(szTmp);
		}
		CheckDlgButton(IDC_ET_SIZECHECK, m_cData.bSize ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_BLURCHECK, m_cData.bBlur ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_FLOWCHECK, m_cData.bFlow ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(IDC_ET_SCATTER, m_cData.bScatter ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_ROTATE, m_cData.bRotate ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(IDC_ET_RESIZE, m_cData.bResize ? BST_CHECKED : BST_UNCHECKED);
		bool bScatterDisabled = m_cData.eShape != CEditToolDataBrush::ESCustom;
		if (bScatterDisabled != m_bScatterDisabled)
		{
			GetDlgItem(IDC_ET_SCATTER).EnableWindow(m_bScatterDisabled);
			GetDlgItem(IDC_ET_ROTATE).EnableWindow(m_bScatterDisabled);
			GetDlgItem(IDC_ET_RESIZE).EnableWindow(m_bScatterDisabled);
			m_bScatterDisabled = bScatterDisabled;
		}

		m_bEnableUpdates = true;
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cIcons;
	CEdit m_wndSize;
	CEdit m_wndBlur;
	CEdit m_wndFlow;
	bool m_bBlurDisabled;
	bool m_bScatterDisabled;
};


