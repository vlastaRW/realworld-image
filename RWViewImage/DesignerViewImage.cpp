// DesignerViewImage.cpp : Implementation of CDesignerViewImage

#include "stdafx.h"
#include "DesignerViewImage.h"

#include "ConfigIDsImageView.h"
#include <htmlhelp.h>


// CDesignerViewImage

HRESULT CDesignerViewImage::Init(ISharedStateManager* a_pFrame, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID)
{
	m_pDoc = a_pDoc;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pImage));
	m_pConfig = a_pConfig;
	m_tLocaleID = a_tLocaleID;
	if (m_pImage == NULL)
		return E_FAIL;
	m_bObserving = SUCCEEDED(m_pImage->ObserverIns(CObserverImpl<CDesignerViewImage, IImageObserver, TImageChange>::ObserverGet(), 0));
	m_pStates = a_pFrame;

	CConfigValue cCustom;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_CUSTOMBACKGROUND), &cCustom);
	m_bCustomBackground = cCustom.operator bool();
	CConfigValue cColor;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_BACKGROUNDCOLOR), &cColor);
	m_clrBackground = cColor.operator LONG();
	CConfigValue cFrame;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_DRAWFRAME), &cFrame);
	m_bFrame = cFrame;
	CConfigValue cViewID;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_MYVIEWPORT), &cViewID);
	m_bstrViewportID = cViewID;
	CConfigValue cCtrlID;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_CONTROLLEDVIEPORT), &cCtrlID);
	m_bstrControlledID = cCtrlID;
	if (m_bstrViewportID.Length() || m_bstrControlledID.Length())
	{
		if (a_pFrame)
		{
			//if (m_bstrViewportID.Length())
			//{
			//	CComPtr<ISharedStateImageViewport> pMyState;
			//	m_pStateMgr->StateGet(m_bstrViewportStateSync, __uuidof(ISharedStateImageViewport), reinterpret_cast<void**>(&pMyState));
			//	if (pMyState)
			//	{
			//		float fOffsetX = 0.0f;
			//		float fOffsetY = 0.0f;
			//		float fZoom = 1.0f;
			//		boolean bAutoZoom = false;
			//		ULONG nSizeX = 0;
			//		ULONG nSizeY = 0;
			//		pMyState->GetEx(&fOffsetX, &fOffsetY, &fZoom, NULL, NULL, &nSizeX, &nSizeY, &bAutoZoom);
			//		if (m_tImageSize.cx == nSizeX && m_tImageSize.cy == nSizeY)
			//		{
			//			m_fOffsetX = fOffsetX;
			//			m_fOffsetY = fOffsetY;
			//			m_fZoom = fZoom;
			//			m_bAutoZoom = bAutoZoom;
			//			//UpdatePosition();
			//		}
			//	}
			//}
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewImage, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		}
		else
		{
			m_bstrViewportID.Empty();
			m_bstrControlledID.Empty();
		}
	}

	CConfigValue cAutoZoom;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGVIEW_AUTOZOOM), &cAutoZoom);

	m_bUpdating = true;
	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), 0, 0, (a_nStyle&EDVWSBorderMask) != EDVWSNoBorder ? WS_EX_CLIENTEDGE : 0) == NULL)
	{
		// creation failed
		return E_FAIL; // TODO: error code
	}

	CImageControl<CDesignerViewImage>::Init(m_pImage.p, m_bFrame, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW), cAutoZoom);
	m_bUpdating = false;
	if (m_bstrControlledID.Length())
	{
		EnableControlledMode();
		CComPtr<ISharedStateImageViewport> pIV;
		a_pFrame->StateGet(m_bstrControlledID, __uuidof(ISharedStateImageViewport), reinterpret_cast<void**>(&pIV));
		if (pIV)
		{
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fZoom = 1.0f;
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			if (SUCCEEDED(pIV->Get(&fCenterX, &fCenterY, &fZoom, &nSizeX, &nSizeY)))
			{
				SetControlledViewport(fCenterX, fCenterY, fZoom, nSizeX, nSizeY);
			}
		}
	}
	else if (m_bstrViewportID.Length())
	{
		CComPtr<ISharedStateImageViewport> pIV;
		a_pFrame->StateGet(m_bstrViewportID, __uuidof(ISharedStateImageViewport), reinterpret_cast<void**>(&pIV));
		if (pIV)
		{
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fZoom = 1.0f;
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			boolean bAutoZoom = cAutoZoom.operator bool();
			if (SUCCEEDED(pIV->GetEx(&fCenterX, &fCenterY, &fZoom, NULL, NULL, &nSizeX, &nSizeY, &bAutoZoom)) && nSizeX == M_OrigSize().cx && nSizeY == M_OrigSize().cy)
			{
				m_bUpdating = true;
				EnableAutoZoom(bAutoZoom);
				if (!bAutoZoom)
					SetViewport(fCenterX, fCenterY, fZoom);
				m_bUpdating = false;
			}
		}
	}
	return S_OK;
}

void CDesignerViewImage::OwnerNotify(TCookie, TImageChange a_tChange)
{
	// TODO: delay refresh (OnIdle???, configurable???)
	if (IsWindow())
	{
		if (a_tChange.nGlobalFlags & EICDimensions)
		{
			// dimensions changed
			CImageControl<CDesignerViewImage>::Init(m_pImage.p, m_bFrame, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
		}
		else
		{
			// dimensions remained the same
			RefreshImage(m_pImage.p, m_bFrame, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
		}
	}
}

void CDesignerViewImage::OwnerNotify(TCookie, TSharedStateChange a_tStateChange)
{
	if (m_bstrControlledID == a_tStateChange.bstrName)
	{
		CComQIPtr<ISharedStateImageViewport> pIV(a_tStateChange.pState);
		if (pIV)
		{
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fZoom = 1.0f;
			ULONG nSizeX = 0;
			ULONG nSizeY = 0;
			if (SUCCEEDED(pIV->Get(&fCenterX, &fCenterY, &fZoom, &nSizeX, &nSizeY)))
			{
				SetControlledViewport(fCenterX, fCenterY, fZoom, nSizeX, nSizeY);
			}
		}
		return;
	}

	if (m_bUpdating)
		return;

	if (m_bstrViewportID == a_tStateChange.bstrName)
	{
		CComQIPtr<ISharedStateImageViewport> pIV(a_tStateChange.pState);
		if (pIV)
		{
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fZoom = 1.0f;
			if (SUCCEEDED(pIV->Get(&fCenterX, &fCenterY, &fZoom, NULL, NULL)))
			{
				m_bUpdating = true;
				SetViewport(fCenterX, fCenterY, fZoom);
				m_bUpdating = false;
			}
		}
	}
}

LRESULT CDesignerViewImage::OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	LRESULT lRes = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);
	if (lRes == MA_NOACTIVATE || lRes == MA_NOACTIVATEANDEAT)
		return lRes;
	
	SetFocus();

	return MA_ACTIVATE;
}

#define COLORPICKER_NOGRADIENT
#include <WTL_ColorPicker.h>
#include <Win32LangEx.h>

LRESULT CDesignerViewImage::OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	if (tPt.x == -1 && tPt.y == -1)
		tPt.x = tPt.y = 0;
	else
		ScreenToClient(&tPt);
	RECT rc = {tPt.x, tPt.y, tPt.x, tPt.y};

	CButtonColorPicker cBtn(false, m_tLocaleID);
	cBtn.SetDefaultColor(GetSysColor(COLOR_WINDOW));
	CButtonColorPicker::SColor sColor = m_bCustomBackground ? CButtonColorPicker::SColor(m_clrBackground) : CButtonColorPicker::SColor(CButtonColorPicker::ECTDefault);
	cBtn.Create(m_hWnd, &rc);
	cBtn.SetColor(sColor);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	BOOL b;
	cBtn.OnClicked(0, 0, 0, b);
	CButtonColorPicker::SColor sNew = cBtn.GetColor();
	cBtn.DestroyWindow();
	if (sNew != sColor)
	{
		CComBSTR bstrCFGID_IMGVIEW_CUSTOMBACKGROUND(CFGID_IMGVIEW_CUSTOMBACKGROUND);
		CComBSTR bstrCFGID_IMGVIEW_BACKGROUNDCOLOR(CFGID_IMGVIEW_BACKGROUNDCOLOR);
		if (sNew.eCT == CButtonColorPicker::ECTDefault)
		{
			m_bCustomBackground = false;
			m_pConfig->ItemValuesSet(1, &(bstrCFGID_IMGVIEW_CUSTOMBACKGROUND.m_str), CConfigValue(false));
		}
		else
		{
			m_bCustomBackground = true;
			m_clrBackground = sNew.ToCOLORREF();
			m_pConfig->ItemValuesSet(1, &(bstrCFGID_IMGVIEW_CUSTOMBACKGROUND.m_str), CConfigValue(true));
			m_pConfig->ItemValuesSet(1, &(bstrCFGID_IMGVIEW_BACKGROUNDCOLOR.m_str), CConfigValue(LONG(m_clrBackground)));
		}
		RedrawWindow(NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOCHILDREN);
		RefreshImage(m_pImage.p, m_bFrame, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
	}
	return 0;
}

LRESULT CDesignerViewImage::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), m_bstrControlledID.Length() ? IDS_IV_CONTEXTHELP_CONTROLLED : IDS_IV_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
		RECT rcItem;
		::GetWindowRect(m_hWnd, &rcItem);
		HH_POPUP hhp;
		hhp.cbStruct = sizeof(hhp);
		hhp.hinst = _pModule->get_m_hInst();
		hhp.idString = 0;
		hhp.pszText = szBuffer;
		hhp.pt.x = (rcItem.right+rcItem.left)>>1;
		hhp.pt.y = (rcItem.bottom+rcItem.top)>>1;
		hhp.clrForeground = 0xffffffff;
		hhp.clrBackground = 0xffffffff;
		hhp.rcMargins.left = -1;
		hhp.rcMargins.top = -1;
		hhp.rcMargins.right = -1;
		hhp.rcMargins.bottom = -1;
		hhp.pszFont = _T("Lucida Sans Unicode, 10");//MS Sans Serif, 10, charset , BOLD
		HtmlHelp(m_hWnd, NULL, HH_DISPLAY_TEXT_POPUP, reinterpret_cast<DWORD>(&hhp));
		return 0;
	}
	a_bHandled = FALSE;
	return 0;
}

STDMETHODIMP CDesignerViewImage::CanSetZoom(float* UNREF(a_pVal))
{
	return S_OK;
}

STDMETHODIMP CDesignerViewImage::SetZoom(float a_fVal)
{
	CImageControl::SetZoom(a_fVal);
	return S_OK;
}

STDMETHODIMP CDesignerViewImage::GetZoom(float* a_pVal)
{
	try
	{
		*a_pVal = M_Zoom();
		return S_OK;
	}
	catch (...)
	{
		return a_pVal ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewImage::SupportsAutoZoom()
{
	return S_OK;
}

STDMETHODIMP CDesignerViewImage::IsAutoZoomEnabled()
{
	return M_AutoZoom() ? S_OK : S_FALSE;
}

STDMETHODIMP CDesignerViewImage::EnableAutoZoom(BOOL a_bEnable)
{
	try
	{
		SetAutoZoom(a_bEnable != 0);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDesignerViewImage::AutoZoomChanged(bool a_bEnabled)
{
	CComBSTR bstrCFGID_IMGVIEW_AUTOZOOM(CFGID_IMGVIEW_AUTOZOOM);
	m_pConfig->ItemValuesSet(1, &(bstrCFGID_IMGVIEW_AUTOZOOM.m_str), CConfigValue(a_bEnabled));
}

void CDesignerViewImage::VieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY)
{
	if (m_bUpdating)
		return;

	if (m_bstrViewportID.Length())
	{
		CComPtr<ISharedStateImageViewport> pIV;
		RWCoCreateInstance(pIV, __uuidof(SharedStateImageViewport));
		pIV->InitEx(a_fOffsetX, a_fOffsetY, a_fZoom, a_nSizeX, a_nSizeY, M_OrigSize().cx, M_OrigSize().cy, M_AutoZoom());
		m_bUpdating = true;
		m_pStates->StateSet(m_bstrViewportID, pIV.p);
		m_bUpdating = false;
	}
}

void CDesignerViewImage::ControlledVieportChanged(float a_fOffsetX, float a_fOffsetY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY)
{
	if (m_bstrControlledID.Length())
	{
		CComPtr<ISharedStateImageViewport> pIV;
		RWCoCreateInstance(pIV, __uuidof(SharedStateImageViewport));
		pIV->InitEx(a_fOffsetX, a_fOffsetY, a_fZoom, a_nSizeX, a_nSizeY, M_OrigSize().cx, M_OrigSize().cy, false);
		m_bUpdating = true;
		m_pStates->StateSet(m_bstrControlledID, pIV.p);
		m_bUpdating = false;
	}
}
