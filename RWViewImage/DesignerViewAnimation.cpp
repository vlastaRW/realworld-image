// DesignerViewAnimation.cpp : Implementation of CDesignerViewAnimation

#include "stdafx.h"
#include "DesignerViewAnimation.h"

#include <htmlhelp.h>


// CDesignerViewAnimation

HRESULT CDesignerViewAnimation::Init(ISharedStateManager* a_pFrame, IConfig* a_pConfig, RWHWND a_hParent, const RECT* a_rcWindow, EDesignerViewWndStyle a_nStyle, IDocument* a_pDoc, LCID a_tLocaleID)
{
	m_pDoc = a_pDoc;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&m_pAnimation));
	m_pConfig = a_pConfig;
	m_tLocaleID = a_tLocaleID;
	if (m_pAnimation == NULL)
		return E_FAIL;
	m_bObserving = SUCCEEDED(m_pAnimation->ObserverIns(CObserverImpl<CDesignerViewAnimation, IAnimationObserver, TAnimationChange>::ObserverGet(), 0));
	//m_pStates = a_pFrame;

	CConfigValue cColor;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_ANIVIEW_BACKGROUNDCOLOR), &cColor);
	m_clrBackground = cColor.operator LONG();
	CConfigValue cFrame;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_ANIVIEW_DRAWFRAME), &cFrame);
	m_bFrame = cFrame;

	m_bUpdating = true;
	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), 0, 0, (a_nStyle&EDVWSBorderMask) != EDVWSNoBorder ? WS_EX_CLIENTEDGE : 0) == NULL)
	{
		// creation failed
		return E_FAIL; // TODO: error code
	}

	m_dwTimeOff = GetTickCount();
	PrepareFrame();
	m_bUpdating = false;
	//if (m_bstrViewportID.Length())
	//{
	//	CComPtr<ISharedStateImageViewport> pIV;
	//	a_pFrame->StateGet(m_bstrViewportID, __uuidof(ISharedStateImageViewport), reinterpret_cast<void**>(&pIV));
	//	if (pIV)
	//	{
	//		float fCenterX = 0.0f;
	//		float fCenterY = 0.0f;
	//		float fZoom = 1.0f;
	//		ULONG nSizeX = 0;
	//		ULONG nSizeY = 0;
	//		boolean bAutoZoom = cAutoZoom.operator bool();
	//		if (SUCCEEDED(pIV->GetEx(&fCenterX, &fCenterY, &fZoom, NULL, NULL, &nSizeX, &nSizeY, &bAutoZoom)) && nSizeX == M_OrigSize().cx && nSizeY == M_OrigSize().cy)
	//		{
	//			m_bUpdating = true;
	//			EnableAutoZoom(bAutoZoom);
	//			if (!bAutoZoom)
	//				SetViewport(fCenterX, fCenterY, fZoom);
	//			m_bUpdating = false;
	//		}
	//	}
	//}
	return S_OK;
}

void CDesignerViewAnimation::OwnerNotify(TCookie, ULONG a_nChangeFlags)
{
	//// TODO: delay refresh (OnIdle???, configurable???)
	//if (IsWindow())
	//{
	//	if (a_nChangeFlags & EICDimesnions)
	//	{
	//		// dimensions changed
	//		CImageControl<CDesignerViewAnimation>::Init(m_pImage.p, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
	//	}
	//	else
	//	{
	//		// dimensions remained the same
	//		RefreshImage(m_pImage.p, m_bCustomBackground ? m_clrBackground : GetSysColor(COLOR_WINDOW));
	//	}
	//}
}

void CDesignerViewAnimation::OwnerNotify(TCookie, TSharedStateChange a_tStateChange)
{
	//if (m_bstrControlledID == a_tStateChange.bstrName)
	//{
	//	CComQIPtr<ISharedStateImageViewport> pIV(a_tStateChange.pState);
	//	if (pIV)
	//	{
	//		float fCenterX = 0.0f;
	//		float fCenterY = 0.0f;
	//		float fZoom = 1.0f;
	//		ULONG nSizeX = 0;
	//		ULONG nSizeY = 0;
	//		if (SUCCEEDED(pIV->Get(&fCenterX, &fCenterY, &fZoom, &nSizeX, &nSizeY)))
	//		{
	//			SetControlledViewport(fCenterX, fCenterY, fZoom, nSizeX, nSizeY);
	//		}
	//	}
	//	return;
	//}

	//if (m_bUpdating)
	//	return;

	//if (m_bstrViewportID == a_tStateChange.bstrName)
	//{
	//	CComQIPtr<ISharedStateImageViewport> pIV(a_tStateChange.pState);
	//	if (pIV)
	//	{
	//		float fCenterX = 0.0f;
	//		float fCenterY = 0.0f;
	//		float fZoom = 1.0f;
	//		if (SUCCEEDED(pIV->Get(&fCenterX, &fCenterY, &fZoom, NULL, NULL)))
	//		{
	//			m_bUpdating = true;
	//			SetViewport(fCenterX, fCenterY, fZoom);
	//			m_bUpdating = false;
	//		}
	//	}
	//}
}

void CDesignerViewAnimation::OwnerNotify(TCookie, TAnimationChange a_eChange)
{
	try
	{
		if (a_eChange.nFlags & EACAnimation)
		{
			PrepareFrame();
		}
		else //if (a_eChange.nFlags & EACImage)
		{
			bool bCur = false;
			for (ULONG i = 0; i < a_eChange.nFrames; ++i)
				if (bCur = a_eChange.apFrames[i] == m_pImgFrame)
					break;
			if (bCur)
				PrepareFrame();
		}
	}
	catch (...)
	{
	}
}

LRESULT CDesignerViewAnimation::OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	AddRef();
	a_bHandled = FALSE;
	return 0;
}

LRESULT CDesignerViewAnimation::OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
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

LRESULT CDesignerViewAnimation::OnContextMenu(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	POINT tPt = {GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam)};
	if (tPt.x == -1 && tPt.y == -1)
		tPt.x = tPt.y = 0;
	else
		ScreenToClient(&tPt);
	RECT rc = {tPt.x, tPt.y, tPt.x, tPt.y};

	CButtonColorPicker cBtn(false, m_tLocaleID);
	cBtn.SetDefaultColor(GetSysColor(COLOR_WINDOW));
	CButtonColorPicker::SColor sColor = m_clrBackground&0xff000000 ? CButtonColorPicker::SColor(CButtonColorPicker::ECTDefault) : CButtonColorPicker::SColor(m_clrBackground);
	cBtn.Create(m_hWnd, &rc);
	cBtn.SetColor(sColor);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	BOOL b;
	cBtn.OnClicked(0, 0, 0, b);
	CButtonColorPicker::SColor sNew = cBtn.GetColor();
	cBtn.DestroyWindow();
	if (sNew != sColor)
	{
		CComBSTR bstrCFGID_ANIVIEW_BACKGROUNDCOLOR(CFGID_ANIVIEW_BACKGROUNDCOLOR);
		m_pConfig->ItemValuesSet(1, &(bstrCFGID_ANIVIEW_BACKGROUNDCOLOR.m_str), CConfigValue(LONG(m_clrBackground = sNew.eCT == CButtonColorPicker::ECTDefault ? 0xff000000 : sNew.ToCOLORREF())));
		RedrawWindow(NULL, NULL, RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW|RDW_NOCHILDREN);
		RefreshImage(m_pSubImg.p, m_bFrame, m_clrBackground&0xff000000 ? GetSysColor(COLOR_WINDOW) : m_clrBackground);
	}
	return 0;
}

LRESULT CDesignerViewAnimation::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_IV_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
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

LRESULT CDesignerViewAnimation::OnPaint(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	BOOL b;
	LRESULT lRes = CImageControl<CDesignerViewAnimation>::OnPaint(a_uMsg, a_wParam, a_lParam, b);

	ULONG dwNow = GetTickCount();
	if (m_dwAniLength && m_dwNextFrame)
	{
		ULONG nPhase = (dwNow-m_dwTimeOff)%m_dwAniLength;
		if (nPhase >= m_dwCurrentFrame && nPhase < m_dwNextFrame)
		{
			// managed to draw the current frame on time, compute the time to the next frame
			SetTimer(14, m_dwNextFrame-nPhase);
		}
		else
		{
			// missed the time window for the current frame (slow computer, hickup, obscured window)
			// resume from the current frame
			m_dwTimeOff = dwNow-m_dwCurrentFrame;
			SetTimer(14, m_dwNextFrame-m_dwCurrentFrame);
		}
	}

	return lRes;
}

void CDesignerViewAnimation::PrepareFrame()
{
	m_pImgFrame = NULL;
	CComPtr<IEnumUnknowns> pFrames;
	m_pAnimation->FramesEnum(&pFrames);
	ULONG nFrames = 0;
	if (pFrames) pFrames->Size(&nFrames);
	m_dwAniLength = 0;
	typedef std::vector<std::pair<ULONG, CComPtr<IUnknown> > > CFrameSeq;
	CFrameSeq cFrameSeq;
	for (ULONG i = 0; i < nFrames; ++i)
	{
		CComPtr<IUnknown> pItem;
		pFrames->Get(i, &pItem);
		float fFrameTime = 0.0f;
		m_pAnimation->FrameGetTime(pItem, &fFrameTime);
		cFrameSeq.push_back(std::make_pair(m_dwAniLength, pItem));
		m_dwAniLength += fFrameTime*1000+0.5f;
	}
	CFrameSeq::const_iterator iActive = cFrameSeq.begin();
	ULONG dwNow = GetTickCount();
	if (m_dwAniLength)
	{
		ULONG nT = (dwNow-m_dwTimeOff)%m_dwAniLength;
		for (CFrameSeq::const_iterator i = cFrameSeq.begin(); i != cFrameSeq.end(); ++i)
		{
			if (i->first > nT)
				break;
			iActive = i;
		}
	}
	if (iActive != cFrameSeq.end())
	{
		CComPtr<IDocument> pSubDoc;
		m_pAnimation->FrameGetDoc(iActive->second, &pSubDoc);
		m_pSubImg = NULL;
		if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pSubImg));
		TImageSize tIS = {0, 0};
		if (m_pSubImg)
		{
			m_pImgFrame = iActive->second;
			m_dwCurrentFrame = iActive->first;
			m_pSubImg->CanvasGet(&tIS, NULL, NULL, NULL, NULL);
			if (tIS.nX == m_tLastIS.nX && tIS.nY == m_tLastIS.nY)
				RefreshImage(m_pSubImg.p, m_bFrame, m_clrBackground&0xff000000 ? GetSysColor(COLOR_WINDOW) : m_clrBackground);
			else
			{
				m_tLastIS = tIS;
				CImageControl<CDesignerViewAnimation>::Init(m_pSubImg.p, m_bFrame, m_clrBackground&0xff000000 ? GetSysColor(COLOR_WINDOW) : m_clrBackground, false);
			}
			++iActive;
			if (m_dwAniLength && cFrameSeq.size() > 1)
			{
				m_dwNextFrame = iActive == cFrameSeq.end() ? m_dwAniLength : iActive->first;
				//ULONG nT = (dwNow-m_dwTimeOff)%m_dwAniLength;
				//SetTimer(14, nNext > nT ? nNext-nT : 1);
			}
		}
	}
}
