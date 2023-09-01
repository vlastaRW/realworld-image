// EditToolBrushDlg.h : Declaration of the CEditToolPencilDlg

#pragma once

#include "resource.h"       // main symbols
#include <MultiLanguageString.h>
#include <ContextHelpDlg.h>
#include <Win32LangEx.h>
#include "EditToolDlg.h"
#include <XPGUI.h>


// CEditToolPencilDlg

class CEditToolPencilDlg : 
	public CEditToolDlgBase<CEditToolPencilDlg, CEditToolDataPencil>,
	public CDialogResize<CEditToolPencilDlg>,
	public CContextHelpDlg<CEditToolPencilDlg>
{
public:
	CEditToolPencilDlg()
	{
		m_tOptimumSize.cx = m_tOptimumSize.cy = 0;
	}
	~CEditToolPencilDlg()
	{
		m_cImageList.Destroy();
	}


	enum { IDD = IDD_EDITTOOL_PENCIL };

	BEGIN_MSG_MAP(CEditToolPencilDlg)
		CHAIN_MSG_MAP(CContextHelpDlg<CEditToolPencilDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
		MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorDlg)
		COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedSomething)
		COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedSomething)
		COMMAND_RANGE_CODE_HANDLER(IDC_ET_PENCILSHAPE+1, IDC_ET_PENCILSHAPE+CEditToolDataPencil::ESCount, BN_CLICKED, OnChange)
		CHAIN_MSG_MAP(CDialogResize<CEditToolPencilDlg>)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CEditToolPencilDlg)
		DLGRESIZE_CONTROL(IDC_ET_PENCILSHAPE, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CEditToolPencilDlg)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_WIDTH, IDS_HELP_WIDTH)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_WIDTHSPIN, IDS_HELP_WIDTH)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_CLOSED, IDS_HELP_CLOSED)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_MITERJOIN, IDS_HELP_MITERJOIN)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_ROUNDJOIN, IDS_HELP_ROUNDJOIN)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_BEVELJOIN, IDS_HELP_BEVELJOIN)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_SQUARECAP, IDS_HELP_SQUARECAP)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_ROUNDEDCAP, IDS_HELP_ROUNDEDCAP)
		//CTXHELP_CONTROL_RESOURCE(IDC_ET_BUTTCAP, IDS_HELP_BUTTCAP)
	END_CTXHELP_MAP()

	BEGIN_COM_MAP(CEditToolPencilDlg)
		COM_INTERFACE_ENTRY(IChildWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditToolWindow)
	END_COM_MAP()

	void OwnerNotify(TCookie, TSharedStateChange a_tParams)
	{
		if (wcscmp(a_tParams.bstrName, m_bstrSyncToolData) == 0)
		{
			CComQIPtr<CEditToolDataPencil::ISharedStateToolData> pData(a_tParams.pState);
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
			*a_pSize = m_tOptimumSize;
			return m_tOptimumSize.cx ? S_OK : E_FAIL;
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
		if (m_pSharedState != NULL)
		{
			CComPtr<CEditToolDataPencil::ISharedStateToolData> pState;
			m_pSharedState->StateGet(m_bstrSyncToolData, __uuidof(CEditToolDataPencil::ISharedStateToolData), reinterpret_cast<void**>(&pState));
			if (pState != NULL)
			{
				m_cData = *(pState->InternalData());
			}
		}

		int nIconSize = XPGUI::GetSmallIconSize();
		ULONG nIconDelta = (nIconSize>>1)-8;
		m_cImageList.Create(nIconSize+nIconDelta, nIconSize, XPGUI::GetImageListColorFlags(), CEditToolDataPencil::ESCount, 0);
		ATLASSERT(CEditToolDataPencil::ESCount == itemsof(g_aPencilShapes));
		for (SPencilShape const* p = g_aPencilShapes; p < g_aPencilShapes+itemsof(g_aPencilShapes); ++p)
		{
			HICON hTmp = PrepareIcon(p, nIconSize+nIconDelta, nIconSize);
			m_cImageList.AddIcon(hTmp);
			DestroyIcon(hTmp);
		}

		TCHAR szTooltipStrings[1024] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_PENCILSHAPES, szTooltipStrings, itemsof(szTooltipStrings), LANGIDFROMLCID(m_tLocaleID));
		for (LPTSTR p = szTooltipStrings; *p; ++p)
			if (*p == _T('|')) *p = _T('\0');

		m_wndToolBar = GetDlgItem(IDC_ET_PENCILSHAPE);
		m_wndToolBar.SetButtonStructSize(sizeof(TBBUTTON));
		m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
		m_wndToolBar.SetImageList(m_cImageList);
		m_wndToolBar.AddStrings(szTooltipStrings);
		CAutoVectorPtr<TBBUTTON> pButtons(new TBBUTTON[CEditToolDataPencil::ESCount]);
		for (int i = 0; i < CEditToolDataPencil::ESCount; ++i)
		{
			pButtons[i].iBitmap = i;
			pButtons[i].idCommand = i+1+IDC_ET_PENCILSHAPE;
			pButtons[i].fsState = TBSTATE_ENABLED|(m_cData.eShape == i ? TBSTATE_CHECKED : 0);
			pButtons[i].fsStyle = BTNS_BUTTON|BTNS_CHECKGROUP;
			//pButtons[i].bReserved[2 or 6];          // padding for alignment
			pButtons[i].dwData = 0;
			pButtons[i].iString = i;
		}
		m_wndToolBar.AddButtons(CEditToolDataPencil::ESCount, pButtons);
		if (CTheme::IsThemingSupported() && IsAppThemed())
			m_wndToolBar.SetButtonSize(nIconSize+8, nIconSize+(nIconSize>>1)+1);
		RECT rcToolbar;
		m_wndToolBar.GetItemRect(m_wndToolBar.GetButtonCount()-1, &rcToolbar);
		RECT rcMargins = {0, 0, 10, 10};
		MapDialogRect(&rcMargins);
		m_tOptimumSize.cx = rcToolbar.right+rcMargins.right;
		m_tOptimumSize.cy = rcToolbar.bottom+rcMargins.bottom;
		RECT rcActual;
		m_wndToolBar.GetWindowRect(&rcActual);
		ScreenToClient(&rcActual);
		rcActual.bottom = rcActual.top+rcToolbar.bottom;
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

		m_cData.eShape = static_cast<CEditToolDataPencil::EShape>(a_wID-IDC_ET_PENCILSHAPE-1);
		DataToState();
		return 0;
	}

	void DataToGUI()
	{
		m_bEnableUpdates = false;
		if (!m_wndToolBar.IsButtonChecked(IDC_ET_PENCILSHAPE+1+m_cData.eShape))
		{
			for (int i = 0; i < CEditToolDataPencil::ESCount; ++i)
				m_wndToolBar.SetState(IDC_ET_PENCILSHAPE+1+i, m_cData.eShape == i ? TBSTATE_CHECKED|TBSTATE_ENABLED : TBSTATE_ENABLED);
		}
		m_bEnableUpdates = true;
	}

private:
	HICON PrepareIcon(SPencilShape const* a_pShape, int a_nSizeX, int a_nSizeY)
	{
		int nSize = min(a_nSizeX, a_nSizeY);
		int nZoom = nSize*0.14f;
		int nOffsetX = (a_nSizeX-nZoom*5)>>1;
		int nOffsetY = (a_nSizeY-nZoom*5)>>1;

		DWORD nXOR = a_nSizeX*a_nSizeY<<2;
		DWORD nAND = a_nSizeY*((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
		ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
		BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pHead->biSize = sizeof(BITMAPINFOHEADER);
		pHead->biWidth = a_nSizeX;
		pHead->biHeight = a_nSizeY<<1;
		pHead->biPlanes = 1;
		pHead->biBitCount = 32;
		pHead->biCompression = BI_RGB;
		pHead->biSizeImage = nXOR+nAND;
		pHead->biXPelsPerMeter = 0;
		pHead->biYPelsPerMeter = 0;
		DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);

		BYTE const* p = a_pShape->coverage;
		for (LONG y = 4; y >= 0; --y)
		{
			for (LONG x = 0; x < 5; ++x, ++p)
			{
				if (*p)
				{
					DWORD* pPix = pXOR + x*nZoom+nOffsetX + (y*nZoom+nOffsetY)*a_nSizeX;
					for (LONG y1 = 0; y1 < nZoom; ++y1)
						for (LONG x1 = 0; x1 < nZoom; ++x1)
							reinterpret_cast<BYTE*>(pPix+x1+y1*a_nSizeX)[3] = *p;
				}
			}
		}

		pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
		BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSizeX*a_nSizeY));
		int nANDLine = ((((a_nSizeX+7)>>3)+3)&0xfffffffc);
		for (int y = 0; y < a_nSizeY; ++y)
		{
			for (int x = 0; x < a_nSizeX; ++x)
			{
				if (0 == (0xff000000&*pXOR))
				{
					pAND[x>>3] |= 0x80 >> (x&7);
				}
				++pXOR;
			}
			pAND += nANDLine;
		}
		return CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSizeX, a_nSizeY, LR_DEFAULTCOLOR);
	}

private:
	CToolBarCtrl m_wndToolBar;
	CImageList m_cImageList;
	SIZE m_tOptimumSize;
};


