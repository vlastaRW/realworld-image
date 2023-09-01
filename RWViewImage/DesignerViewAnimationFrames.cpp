// DesignerViewAnimationFrames.cpp : Implementation of CDesignerViewAnimationFrames

#include "stdafx.h"
#include "DesignerViewFactoryAnimationFrames.h"
#include "DesignerViewAnimationFrames.h"
#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include <htmlhelp.h>
#include <Win32LangEx.h>
#include <XPGUI.h>


// CDesignerViewAnimationFrames

void CDesignerViewAnimationFrames::Init(ISharedStateManager* a_pFrame, CConfigValue& a_cVal, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc)
{
	m_pDoc = a_pDoc;
	m_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&m_pCursor));
	if (m_pCursor == NULL)
		throw E_FAIL;
	m_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimationClipboard), reinterpret_cast<void**>(&m_pClipboard));

	static INITCOMMONCONTROLSEX tICCE = {sizeof(tICCE), ICC_LISTVIEW_CLASSES};
	static BOOL bDummy = InitCommonControlsEx(&tICCE);

	m_pCursor->ObserverIns(CObserverImpl<CDesignerViewAnimationFrames, IAnimationObserver, TAnimationChange>::ObserverGet(), 0);
	m_tLocaleID = a_tLocaleID;

	CComBSTR bstrPrefix;
	m_pCursor->StatePrefix(&bstrPrefix);
	if (bstrPrefix.Length())
	{
		m_strSelGrp.Attach(bstrPrefix.Detach());
		m_strSelGrp += a_cVal;
	}
	else
	{
		m_strSelGrp.Attach(a_cVal.Detach().bstrVal);
	}

	a_pFrame->ObserverIns(CObserverImpl<CDesignerViewAnimationFrames, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	m_pFrame = a_pFrame;

	// create self
	if (Create(a_hParent, const_cast<LPRECT>(a_rcWindow), _T("AnimatedCursor"), WS_CHILDWINDOW|WS_CLIPSIBLINGS, 0, 1, reinterpret_cast<void*>(a_nStyle&EDVWSBorderMask)) == NULL)
	{
		// creation failed
		throw E_FAIL; // TODO: error code
	}

	MoveWindow(const_cast<LPRECT>(a_rcWindow));
	ShowWindow(SW_SHOW);
}

LRESULT CDesignerViewAnimationFrames::OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	RECT rc;
	GetClientRect(&rc);
	m_wndList.Create(m_hWnd, rc, 0, LVS_AUTOARRANGE|LVS_EDITLABELS|LVS_ICON|LVS_SHOWSELALWAYS|WS_CHILD|WS_VISIBLE);
	if (XPGUI::IsVista() && CTheme::IsThemingSupported())
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER/*|LVS_EX_INFOTIP*/);
		::SetWindowTheme(m_wndList, L"explorer", NULL);
	}
	else
	{
		m_wndList.SetExtendedListViewStyle(LVS_EX_BORDERSELECT|LVS_EX_DOUBLEBUFFER/*|LVS_EX_INFOTIP*/);
	}
	HDC hDC = GetDC();
	m_nIconSize = GetDeviceCaps(hDC, LOGPIXELSX)/2;
	ReleaseDC(hDC);
	m_wndList.SetImageList(ImageList_Create(m_nIconSize, m_nIconSize, ILC_COLOR, 1, 1), LVSIL_NORMAL);
	m_wndList.SetIconSpacing(m_nIconSize+(m_nIconSize+1)/3, m_nIconSize+(m_nIconSize*2+1)/3);

	Data2GUI();
	if (m_wndList.GetItemCount() && -1 == m_wndList.GetNextItem(-1, LVNI_ALL|LVNI_SELECTED))
	{
		// select first item
		m_wndList.SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}

	AddRef();

	return 0;
}

LRESULT CDesignerViewAnimationFrames::OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bDragging)
	{
		if ((a_wParam&MK_LBUTTON) == 0)
		{
			LVINSERTMARK tMark;
			tMark.cbSize = sizeof tMark;
			tMark.dwFlags = 0;
			tMark.iItem = -1;
			tMark.dwReserved = 0;
			m_wndList.SetInsertMark(&tMark);
			ReleaseCapture();
			m_bDragging = false;
			m_bValidDND = false;
			return 0;
		}
		POINT tPt = { GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam) };
		RECT rcWnd;
		GetClientRect(&rcWnd);
		LVINSERTMARK tMark;
		tMark.cbSize = sizeof tMark;
		tMark.dwReserved = 0;
		if (::PtInRect(&rcWnd, tPt))
		{
			int nItems = m_wndList.GetItemCount();
			tMark.dwFlags = LVIM_AFTER;
			tMark.iItem = nItems-1;
			for (int i = 0; i < nItems; ++i)
			{
				RECT rc;
				m_wndList.GetItemRect(i, &rc, LVIR_BOUNDS);
				if (::PtInRect(&rc, tPt))
				{
					tMark.dwFlags = tPt.x+tPt.x > rc.left+rc.right ? LVIM_AFTER : 0;
					tMark.iItem = i;
					break;
				}
			}
		}
		else
		{
			tMark.dwFlags = 0;
			tMark.iItem = -1;
		}
		m_wndList.SetInsertMark(&tMark);
		if (tMark.iItem == -1)
		{
			if (m_hFramesBad == NULL)
				m_hFramesBad = ::LoadCursor(NULL, IDC_NO);
			SetCursor(m_hFramesBad);
			m_bValidDND = false;
		}
		else if (a_wParam & MK_CONTROL)
		{
			if (m_hFramesCopy == NULL)
				m_hFramesCopy = CreateFrameManipulationCursor(true);
			SetCursor(m_hFramesCopy);
			m_bValidDND = true;
		}
		else
		{
			if (m_hFramesMove == NULL)
				m_hFramesMove = CreateFrameManipulationCursor(false);
			SetCursor(m_hFramesMove);
			m_bValidDND = true;
		}
	}
	return 0;
}

HCURSOR CDesignerViewAnimationFrames::CreateFrameManipulationCursor(bool copy)
{
	ULONG nSX = GetSystemMetrics(SM_CXCURSOR);
	ULONG nSY = GetSystemMetrics(SM_CYCURSOR);
	HKEY hKey = NULL;
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, _T("Control Panel\\Cursors"), &hKey))
	{
		DWORD dwValue = 0;
		DWORD dwSize = sizeof dwValue;
		RegQueryValueEx(hKey, _T("CursorBaseSize"), NULL, NULL, reinterpret_cast<BYTE*>(&dwValue), &dwSize);
		if (dwValue > nSX && dwValue <= 256) nSX = dwValue;
		if (dwValue > nSY && dwValue <= 256) nSY = dwValue;
		RegCloseKey(hKey);
	}
	CIconRendererReceiver cRenderer(nSX, nSY);
	IRCanvas canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	IRFill white(0xffffffff);
	IRFill black(0xff000000);
	IROutlinedFill defMat(&white, &black, 1.0f/32.0f, 0.0f);
	IRFill gray0(0x80bcbcbc);
	IRFill gray1(0xffbcbcbc);
	IROutlinedFill gray(&gray0, &gray1, 1.0f/32.0f, 0.0f);

	IRGridItem gridX2[] = { {0, 16}, {0, 120} };
	IRGridItem gridY2[] = { {0, 128}, {0, 200} };
	IRCanvas canvas2 = {0, 0, 256, 256, itemsof(gridX2), itemsof(gridY2), gridX2, gridY2};
	IRPolyPoint const arrow[] = { {0, 0}, {0, 144}, {30, 114}, {49, 159}, {79, 147}, {60, 102}, {102, 102} };
	IRPolyPoint const items[] = { {16, 128}, {120, 128}, {120, 200}, {16, 200} };
	cRenderer(&canvas, itemsof(items), items, &gray);
	cRenderer(&canvas2, itemsof(arrow), arrow, &defMat);
	if (copy)
	{
		IRGridItem gridX1[] = { {0, 80}, {0, 168} };
		IRGridItem gridY1[] = { {0, 160}, {0, 248} };
		IRCanvas canvas1 = {0, 0, 256, 256, itemsof(gridX1), itemsof(gridY1), gridX1, gridY1};
		IRPolyPoint const copy[] = { {80, 160}, {168, 160}, {168, 248}, {80, 248} };
		cRenderer(&canvas1, itemsof(copy), copy, &defMat);
		IRPolyPoint const plus1[] = { {124, 184}, {124, 224} };
		IRPolyPoint const plus2[] = { {144, 204}, {104, 204} };
		IRStroke stroke(0xff000000, 1.0f/32.0f);
		cRenderer(&canvas1, itemsof(plus1), plus1, &stroke);
		cRenderer(&canvas1, itemsof(plus2), plus2, &stroke);
	}
	return cRenderer.getCursor(0, 0);
}

#include <RWDocumentAnimationUtils.h>

LRESULT CDesignerViewAnimationFrames::OnButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	if (m_bDragging)
	{
		LVINSERTMARK tMark;
		tMark.cbSize = sizeof tMark;
		tMark.dwFlags = 0;
		tMark.iItem = -1;
		tMark.dwReserved = 0;
		m_wndList.SetInsertMark(&tMark);
		ReleaseCapture();
		m_bDragging = false;
		m_bValidDND = false;

		POINT tPt = { GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam) };
		RECT rcWnd;
		GetClientRect(&rcWnd);
		if (!::PtInRect(&rcWnd, tPt))
			return 0;

		CComPtr<IUnknown> pBefore;
		int nItems = m_wndList.GetItemCount();
		for (int i = 0; i < nItems; ++i)
		{
			RECT rc;
			m_wndList.GetItemRect(i, &rc, LVIR_BOUNDS);
			if (::PtInRect(&rc, tPt))
			{
				if (tPt.x+tPt.x > rc.left+rc.right)
				{
					if (i+1 < nItems)
						pBefore = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(i+1));
				}
				else
				{
					pBefore = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(i));
				}
				break;
			}
		}
		std::vector<CComPtr<IUnknown> > cSel;
		for (int i = 0; i < nItems; ++i)
		{
			if (m_wndList.GetItemState(i, LVIS_SELECTED))
			{
				cSel.push_back(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(i)));
			}
		}
		if (a_wParam & MK_CONTROL)
		{
			CWriteLock<IBlockOperations> cLock(m_pDoc);
			for (std::vector<CComPtr<IUnknown> >::const_reverse_iterator i = cSel.rbegin(); i != cSel.rend(); ++i)
			{
				CComPtr<IUnknown> pNew;
				{

					CComPtr<IDocument> pSubDoc;
					if (*i) m_pCursor->FrameGetDoc(*i, &pSubDoc);
					m_pCursor->FrameIns(pBefore, pSubDoc ? &CAnimationFrameCreatorDocument(pSubDoc, true) : NULL, &pNew);
				}
				float fTime = 0.1f;
				m_pCursor->FrameGetTime(*i, &fTime);
				m_pCursor->FrameSetTime(pNew, fTime);
				pBefore = pNew;
			}
		}
		else
		{
			CWriteLock<IBlockOperations> cLock(m_pDoc);
			for (std::vector<CComPtr<IUnknown> >::const_reverse_iterator i = cSel.rbegin(); i != cSel.rend(); ++i)
			{
				m_pCursor->FrameMove(pBefore, *i);
				pBefore = *i;
			}
		}
	}
	return 0;
}

LRESULT CDesignerViewAnimationFrames::OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMLVCUSTOMDRAW* p = reinterpret_cast<NMLVCUSTOMDRAW*>(a_pNMHeader);
	switch (p->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		AgeCachedImages();
	case CDDS_PREERASE:
		return CDRF_NOTIFYITEMDRAW;//CDRF_NOTIFYITEMERASE;
	case CDDS_ITEMPREPAINT://CDDS_ITEMPREERASE:
		return CDRF_NOTIFYPOSTPAINT;
		//{
		//	RECT rc;
		//	m_wndList.GetItemRect(p->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);
		//	//FillRect(p->nmcd.hdc, &/*p->nmcd.*/rc, (HBRUSH)(COLOR_3DFACE+1));
		//}
		//return CDRF_NOTIFYPOSTPAINT;//CDRF_DODEFAULT;//CDRF_SKIPDEFAULT;
	case CDDS_ITEMPOSTPAINT:
		{
			ObjectLock cLock(this);
			IUnknown* pFrame = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(p->nmcd.dwItemSpec));
			BYTE* pData = NULL;
			int nSizeX = 1;
			int nSizeY = 1;
			RECT rc = {0, 0, 0, 0};
			if (m_wndList.GetItemRect(p->nmcd.dwItemSpec, &rc, LVIR_ICON) &&
				rc.right > rc.left && rc.bottom > rc.top &&
				GetImage(pFrame, &pData, &nSizeX, &nSizeY))
			{
				ULONG nMaskLineSize = (((nSizeX+7)>>3)+3)&~3;
				CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+nSizeX*nSizeY*4+nMaskLineSize*nSizeY]);
				BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
				pBIH->biSize = sizeof*pBIH;
				pBIH->biWidth = nSizeX;
				pBIH->biHeight = nSizeY<<1;
				pBIH->biPlanes = 1;
				pBIH->biBitCount = 32;
				pBIH->biCompression = BI_RGB;
				pBIH->biSizeImage = nSizeX*nSizeY*4+nMaskLineSize*nSizeY;
				pBIH->biXPelsPerMeter = 0x8000;
				pBIH->biYPelsPerMeter = 0x8000;
				pBIH->biClrUsed = 0;
				pBIH->biClrImportant = 0;
				BYTE* pXOR = reinterpret_cast<BYTE*>(pBIH+1);
				BYTE* pAND = pXOR+nSizeX*nSizeY*4;
				for (int y = 0; y < nSizeY; ++y)
					CopyMemory(pXOR+(nSizeY-1-y)*nSizeX*4, pData+y*nSizeX*4, nSizeX*4);
				// create mask
				for (int y = 0; y < nSizeY; ++y)
				{
					BYTE* pA = pAND+nMaskLineSize*y;
					BYTE* pC = pXOR+nSizeX*4*y;
					for (int x = 0; x < nSizeX; ++x, pC+=4)
					{
						BYTE* p = pA+(x>>3);
						if (pC[3])
							*p &= ~(0x80 >> (x&7));
						else
							*p |= 0x80 >> (x&7);
					}
				}

				HICON h = CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+nSizeX*nSizeY*4+nMaskLineSize*nSizeY, TRUE, 0x00030000, nSizeX, nSizeY, LR_DEFAULTCOLOR);
				DrawIconEx(p->nmcd.hdc, (rc.left+rc.right-nSizeX)>>1, (rc.top+rc.bottom-nSizeY)>>1, h, nSizeX, nSizeY, 0, NULL, DI_NORMAL);
				DestroyIcon(h);
			}
		}
		return CDRF_DODEFAULT;
	default:
		return CDRF_DODEFAULT;
	}
}

LRESULT CDesignerViewAnimationFrames::OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	CEdit wnd = m_wndList.GetEditControl();
	TCHAR szTmp[64] = _T("");
	wnd.GetWindowText(szTmp, itemsof(szTmp));
	szTmp[63] = _T('\0');
	LPTSTR p = szTmp;
	while (*p && *p != _T(' ')) ++p;
	*p = _T('\0');
	wnd.SetWindowText(szTmp);
	return FALSE;
}

LRESULT CDesignerViewAnimationFrames::OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	NMLVDISPINFO* p = reinterpret_cast<NMLVDISPINFO*>(a_pNMHeader);
	if (p->item.pszText == NULL)
		return FALSE;
	LPTSTR pEnd = NULL;
	double dNew = _tcstod(p->item.pszText, &pEnd);
	if (dNew <= 0.0 /*&& p->item.pszText[0] != _T('0')*/)
		return FALSE;
	std::vector<IUnknown*> cFrames;
	for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
		cFrames.push_back(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));
	CComPtr<IUnknown> pFrame;
	if (cFrames.empty())
	{
		CComPtr<IEnumUnknowns> pFrames;
		m_pCursor->FramesEnum(&pFrames);
		pFrames->Get(p->item.iItem, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame));
		if (pFrame == NULL)
			return FALSE;
		cFrames.push_back(pFrame.p);
	}
	float fUnit = 0.01f;
	m_pCursor->TimeUnitInfo(&fUnit, 0, NULL);
	CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(L"[0409]Change frame delay[0405]Změnit čas snímku"));
	{
		CWriteLock<IDocument> cLock(m_pDoc);
		for (std::vector<IUnknown*>::const_iterator i = cFrames.begin(); i != cFrames.end(); ++i)
			m_pCursor->FrameSetTime(*i, dNew*fUnit);
	}
	return FALSE;
}

LRESULT CDesignerViewAnimationFrames::OnKeyDown(int UNREF(a_nCtrlID), LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	int nFocused = m_wndList.GetNextItem(-1, LVNI_ALL|LVNI_FOCUSED);
	if (nFocused >= 0)
	{
		LPNMLVKEYDOWN pNMTVKeyDown = reinterpret_cast<LPNMLVKEYDOWN>(a_pNMHeader);

		if (pNMTVKeyDown->wVKey == VK_F2)
		{
			m_wndList.EditLabel(nFocused);
			return 1;
		}
		if (pNMTVKeyDown->wVKey == VK_RIGHT)
		{
			RECT rc = {0, 0, 0, 0};
			m_wndList.GetItemRect(nFocused, &rc, LVIR_ICON);
			RECT rc2 = {rc.left+10, 0, 0, 0};
			if (m_wndList.GetItemRect(nFocused+1, &rc2, LVIR_ICON) && rc2.left <= rc.left)
			{
				m_wndList.SetItemState(nFocused+1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
					if (nLast != nFocused+1) m_wndList.SetItemState(nLast, 0, LVIS_SELECTED | LVIS_FOCUSED);
				m_wndList.EnsureVisible(nFocused+1, FALSE);
				return 1;
			}
			return 0;
		}
		if (pNMTVKeyDown->wVKey == VK_LEFT && nFocused > 0)
		{
			RECT rc = {0, 0, 0, 0};
			m_wndList.GetItemRect(nFocused, &rc, LVIR_ICON);
			RECT rc2 = {rc.left-10, 0, 0, 0};
			if (m_wndList.GetItemRect(nFocused-1, &rc2, LVIR_ICON) && rc2.left >= rc.left)
			{
				m_wndList.SetItemState(nFocused-1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
					if (nLast != nFocused-1) m_wndList.SetItemState(nLast, 0, LVIS_SELECTED | LVIS_FOCUSED);
				m_wndList.EnsureVisible(nFocused-1, FALSE);
				return 1;
			}
			return 0;
		}
		TCHAR szBuf[10];
		int nCount = 0;
		BYTE szKeyboardState[256];
		GetKeyboardState(szKeyboardState);
#ifdef _UNICODE
		nCount = ToUnicode(pNMTVKeyDown->wVKey,MapVirtualKey(pNMTVKeyDown->wVKey, 2), szKeyboardState, szBuf, itemsof(szBuf), 0);
#else
		nCount = ToAscii(pNMTVKeyDown->wVKey, MapVirtualKey(pNMTVKeyDown->wVKey, 2), szKeyboardState, reinterpret_cast<LPWORD>(szBuf), 0);
#endif
		if (nCount > 0 && szBuf[0] >= _T('0') && szBuf[0] <= _T('9'))
		{
			szBuf[nCount] = _T('\0');
			CEdit wndEdit = m_wndList.EditLabel(nFocused);
			wndEdit.SetWindowText(szBuf);
			wndEdit.SetSel(nCount, nCount);
			return 1;
		}
	}
	return 0;
}

LRESULT CDesignerViewAnimationFrames::OnBeginDrag(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	m_bDragging = true;
	m_bValidDND = true;
	SetCapture();
	return 0;
}

LRESULT CDesignerViewAnimationFrames::OnItemChanged(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
{
	if (m_strSelGrp.Length() == 0 || !m_bEnableUpdates)
		return 0;

	if (!m_bSelectionUpdatePosted)
	{
		m_bSelectionUpdatePosted = true;
		PostMessage(WM_UPDATESELECTION);
	}
	return 0;
}

LRESULT CDesignerViewAnimationFrames::OnUpdateSelection(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
{
	m_bSelectionUpdatePosted = false;
	CComPtr<IEnumUnknownsInit> pFrames;
	RWCoCreateInstance(pFrames, __uuidof(EnumUnknowns));
	int nItems = m_wndList.GetItemCount();
	for (int i = 0; i < nItems; ++i)
	{
		if (m_wndList.GetItemState(i, LVIS_SELECTED))
		{
			pFrames->Insert(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(i)));
		}
	}
	CComPtr<ISharedState> pSS;
	m_pCursor->StatePack(pFrames.p, &pSS);
	m_bEnableUpdates = false;
	m_pFrame->StateSet(m_strSelGrp, pSS);
	m_bEnableUpdates = true;
	return 0;
}


void CDesignerViewAnimationFrames::Data2GUI()
{
	ObjectLock cLock(this);
	m_cFrames.clear();
	CComPtr<IEnumUnknowns> pFrames;
	m_pCursor->FramesEnum(&pFrames);
	CComPtr<IUnknown> pDocument;
	m_wndList.SetRedraw(FALSE);
	ULONG nItems = m_wndList.GetItemCount();

	int nFocused = m_wndList.GetNextItem(-1, LVNI_ALL|LVNI_FOCUSED);
	CComPtr<IUnknown> pFocused = nFocused >= 0 ? reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nFocused)) : NULL;

	bool bForceFocus = true;
	for (ULONG i = 0; SUCCEEDED(pFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pDocument))); ++i, pDocument = NULL)
	{
		float fTime = 0.0f;
		m_pCursor->FrameGetTime(pDocument, &fTime);
		TCHAR szTmp[64];
		CComBSTR bstrUnit;
		float fScale = 1.0f;
		m_pCursor->TimeUnitInfo(&fScale, m_tLocaleID, &bstrUnit);
		if (bstrUnit.Length())
		{
			COLE2T str(bstrUnit.m_str);
			_stprintf(szTmp, _T("%i [%s]"), ULONG(fTime/fScale+0.5f), (LPCTSTR)str);
		}
		else
		{
			_stprintf(szTmp, _T("%g [s]"), fTime);
		}
		m_cFrames[pDocument] = i;
		if (i < nItems)
		{
			m_wndList.SetItem(i, 0, LVIF_TEXT|LVIF_PARAM|LVIF_STATE, szTmp, 0, pDocument.p == pFocused.p ? LVIS_FOCUSED : 0, LVIS_FOCUSED, reinterpret_cast<DWORD_PTR>(pDocument.p));
		}
		else
		{
			m_wndList.InsertItem(LVIF_TEXT|LVIF_PARAM|LVIF_STATE, i, szTmp, 0, pDocument.p == pFocused.p ? LVIS_FOCUSED : 0, LVIS_FOCUSED, reinterpret_cast<DWORD_PTR>(pDocument.p));
		}
		if (pDocument.p == pFocused.p)
			bForceFocus = false;
	}
	ULONG nNewItems = m_cFrames.size();
	for (ULONG i = nItems; i > nNewItems; --i)
	{
		m_wndList.DeleteItem(i-1);
	}
	// delete cached images that are not in the cursor anymore
	std::set<IUnknown*> cCached;
	for (size_t i = 0; i != IMAGECACHESIZE; ++i)
		if (m_aImageCache[i].pFrame)
			cCached.insert(m_aImageCache[i].pFrame);
	std::set<IUnknown*> cPresent;
	for (CFrames::const_iterator i = m_cFrames.begin(); i != m_cFrames.end(); ++i)
		cPresent.insert(i->first);
	std::vector<IUnknown*> cToDelete;
	std::set_difference(cCached.begin(), cCached.end(), cPresent.begin(), cPresent.end(), std::back_inserter(cToDelete));
	for (size_t i = 0; i != IMAGECACHESIZE; ++i)
		if (m_aImageCache[i].pFrame)
			if (std::binary_search(cToDelete.begin(), cToDelete.end(), m_aImageCache[i].pFrame))
				FreeCachedImage(m_aImageCache+i, i+1);
	// select items
	if (m_strSelGrp)
	{
		CComPtr<ISharedState> pSS;
		m_pFrame->StateGet(m_strSelGrp, __uuidof(ISharedState), reinterpret_cast<void**>(&pSS));
		if (pSS)
			Selection2GUI(pSS, bForceFocus);
	}
	m_wndList.SetRedraw(TRUE);
}
void CDesignerViewAnimationFrames::Selection2GUI(ISharedState* a_pState, bool a_bForceFocus)
{
	m_bEnableUpdates = false;
	bool bSetFocus = a_bForceFocus || m_wndList.GetNextItem(-1, LVNI_ALL|LVNI_FOCUSED) == -1;
	CComPtr<IEnumUnknowns> pItems;
	m_pCursor->StateUnpack(a_pState, &pItems);
	std::set<int> cSel;
	if (pItems)
	{
		ULONG nSize = 0;
		pItems->Size(&nSize);
		if (nSize)
		{
			IUnknown** p = reinterpret_cast<IUnknown**>(_alloca(sizeof(IUnknown*)*nSize));
			pItems->GetMultiple(0, nSize, __uuidof(IUnknown), reinterpret_cast<void**>(p));
			for (IUnknown** pp = p; pp < p+nSize; ++pp)
			{
				ObjectLock cLock(this);
				CFrames::const_iterator i = m_cFrames.find(*pp);
				if (i != m_cFrames.end())
				{
					if (bSetFocus)
					{
						m_wndList.SetItemState(i->second, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
						bSetFocus = false;
					}
					else
					{
						m_wndList.SetItemState(i->second, LVIS_SELECTED, LVIS_SELECTED);
					}
					cSel.insert(i->second);
				}
				(*pp)->Release();
			}
		}
	}
	int nItems = m_wndList.GetItemCount();
	for (int i = 0; i < nItems; ++i)
	{
		if (m_wndList.GetItemState(i, LVIS_SELECTED) && cSel.find(i) == cSel.end())
		{
			m_wndList.SetItemState(i, 0, LVIS_SELECTED);
		}
	}
	m_bEnableUpdates = true;
}

void CDesignerViewAnimationFrames::FreeCachedImage(SImageCache* a_p, int a_nCookie)
{
	if (a_p->pFrame)
	{
		a_p->pFrame->Release();
		a_p->pFrame = NULL;
		delete[] a_p->pData;
		a_p->pData = NULL;
	}
}

void CDesignerViewAnimationFrames::FreeAllCachedImages()
{
	ObjectLock cLock(this);
	SImageCache* const pEnd = m_aImageCache+IMAGECACHESIZE;
	int nCookie = 0;
	for (SImageCache* p = m_aImageCache; p != pEnd; ++p)
	{
		FreeCachedImage(p, ++nCookie);
	}
}

void CDesignerViewAnimationFrames::AgeCachedImages()
{
	ObjectLock cLock(this);
	SImageCache* const pEnd = m_aImageCache+IMAGECACHESIZE;
	for (SImageCache* p = m_aImageCache; p != pEnd; ++p)
	{
		p->nAge++;
	}
}

bool CDesignerViewAnimationFrames::GetImage(IUnknown* a_pFrame, BYTE** a_ppData, int* a_pSizeX, int* a_pSizeY)
{
	// 1. try to find it in the cache
	SImageCache* const pEnd = m_aImageCache+IMAGECACHESIZE;
	for (SImageCache* p = m_aImageCache; p != pEnd; ++p)
	{
		if (a_pFrame == p->pFrame)
		{
			*a_ppData = p->pData;
			*a_pSizeX = p->nSizeX;
			*a_pSizeY = p->nSizeY;
			return true;
		}
	}

	// 2. not in cache -> find a free place or something to delete
	SImageCache* pReplace = NULL;
	for (SImageCache* p = m_aImageCache; p != pEnd; ++p)
	{
		if (p->pFrame == NULL)
		{
			pReplace = p;
			break;
		}
		if (pReplace == NULL || pReplace->nAge < p->nAge)
		{
			pReplace = p;
		}
	}
	int nCookie = 1+(pReplace-m_aImageCache);
	FreeCachedImage(pReplace, nCookie);

	CAutoVectorPtr<BYTE> cSrcData;
	try
	{
		CComPtr<IDocument> pDoc;
		m_pCursor->FrameGetDoc(a_pFrame, &pDoc);
		if (pDoc == NULL) return CDRF_DODEFAULT;
		CComPtr<IDocumentImage> pImg;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
		if (pImg == NULL) return CDRF_DODEFAULT;
		TImageSize tSize = {0, 0};
		pImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		if (tSize.nX*tSize.nY == 0) return CDRF_DODEFAULT;
		TImageSize const tSizeOrig = tSize;
		if (tSize.nX > m_nIconSize)
		{
			if (tSize.nY > m_nIconSize)
			{
				if (tSize.nX > tSize.nY)
				{
					tSize.nY = (tSize.nY*m_nIconSize+(tSize.nX>>1))/tSize.nX;
					tSize.nX = m_nIconSize;
				}
				else
				{
					tSize.nX = (tSize.nX*m_nIconSize+(tSize.nY>>1))/tSize.nY;
					tSize.nY = m_nIconSize;
				}
			}
			else
			{
				tSize.nY = (tSize.nY*m_nIconSize+(tSize.nX>>1))/tSize.nX;
				tSize.nX = m_nIconSize;
			}
		}
		else
		{
			if (tSize.nY > m_nIconSize)
			{
				tSize.nX = (tSize.nX*m_nIconSize+(tSize.nY>>1))/tSize.nY;
				tSize.nY = m_nIconSize;
			}
		}
		if (tSize.nX == 0)
			tSize.nX = 1;
		if (tSize.nY == 0)
			tSize.nY = 1;
		cSrcData.Attach(new BYTE[tSize.nX*tSize.nY<<2]);
		pReplace->nSizeX = tSize.nX;
		pReplace->nSizeY = tSize.nY;
		if (tSize.nX != tSizeOrig.nX || tSize.nY != tSizeOrig.nY)
		{
			CNearestImageResizer::GetResizedImage(pImg, EICIRGBA, tSize.nX, tSize.nY, 1, tSize.nX, reinterpret_cast<TPixelChannel*>(cSrcData.m_p));
		}
		else
		{
			pImg->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(cSrcData.m_p), NULL, EIRIPreview);
		}
	}
	catch (...)
	{
		return false;
	}
	pReplace->nAge = 0;
	pReplace->pData = cSrcData.Detach();
	(pReplace->pFrame = a_pFrame)->AddRef();
	*a_ppData = pReplace->pData;
	*a_pSizeX = pReplace->nSizeX;
	*a_pSizeY = pReplace->nSizeY;
	return true;
}

STDMETHODIMP CDesignerViewAnimationFrames::ObjectName(EDesignerViewClipboardAction a_eAction, ILocalizedString **a_ppName)
{
	if (a_ppName == NULL)
		return E_POINTER;
	try
	{
		*a_ppName = new CMultiLanguageString(L"[0409]frames[0405]snímmky");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewAnimationFrames::ObjectIconID(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID)
{
	try
	{
		GUID const tID = {0x87375b61, 0x294b, 0x4d64, {0x85, 0x23, 0x01, 0xc2, 0x63, 0x68, 0xfc, 0x9d}};
		*a_pIconID = tID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewAnimationFrames::ObjectIcon(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	try
	{
		if (a_phIcon) *a_phIcon = NULL;
		if (a_pOverlay) *a_pOverlay = TRUE;
		if (a_phIcon)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cRenderer(a_nSize);
			CDesignerViewFactoryAnimationFrames::GetFrameIconLayers(pSI, cRenderer);
			*a_phIcon = cRenderer.get();
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		return S_OK;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDesignerViewAnimationFrames::Check(EDesignerViewClipboardAction a_eAction)
{
	if (m_hWnd == NULL)
		return S_FALSE;
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:
			if (m_pClipboard.p == NULL)
				return S_FALSE;
		case EDVCADelete:
			{
				int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
				if (nSel == -1)
					return S_FALSE;
				int nCount = 1;
				while (-1 != (nSel = m_wndList.GetNextItem(nSel, LVNI_SELECTED)))
					++nCount;
				return m_wndList.GetItemCount() > nCount ? S_OK : S_FALSE;
			}
		case EDVCACopy:
			if (m_pClipboard.p == NULL)
				return S_FALSE;
			return m_wndList.GetNextItem(-1, LVNI_SELECTED) >= 0 ? S_OK : S_FALSE;
		case EDVCAPaste:
			return m_pClipboard.p ? m_pClipboard->CanPaste() : S_FALSE;
		case EDVCASelectAll:
		case EDVCAInvertSelection:
			return m_strSelGrp.Length() ? S_OK : S_FALSE;
		case EDVCADuplicate:
			{
				int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
				if (nSel == -1)
					return S_FALSE;
				nSel = m_wndList.GetNextItem(nSel, LVNI_SELECTED);
				return nSel == -1 ? S_OK : S_FALSE;
			}
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <SharedStateUndo.h>

STDMETHODIMP CDesignerViewAnimationFrames::Exec(EDesignerViewClipboardAction a_eAction)
{
	try
	{
		switch (a_eAction)
		{
		case EDVCACut:
			if (m_hWnd && m_pClipboard.p)
			{
				CUndoBlock cUndo(m_pDoc);
				{
					CWriteLock<IDocument> cLock(m_pDoc);

					std::vector<IUnknown*> cFrames;
					for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
						cFrames.push_back(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));
					if (cFrames.empty())
						return S_FALSE;

					// find an item to select after deletion
					int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
					int nPrev = m_wndList.GetNextItem(nSel, LVNI_PREVIOUS);
					int nNext = m_wndList.GetNextItem(nSel, LVNI_ALL);
					while (nNext >= 0 && m_wndList.GetItemState(nNext, LVIS_SELECTED) == LVIS_SELECTED)
						nNext = m_wndList.GetNextItem(nNext, LVNI_ALL);
					if (nNext < 0) nNext = nPrev;
					IUnknown* pNewSel = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nNext));

					HRESULT hRes = m_pClipboard->Copy(m_hWnd, cFrames.size(), &cFrames[0]);

					if (SUCCEEDED(hRes))
					{
						for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
							m_pCursor->FrameDel(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));

						CComPtr<IEnumUnknownsInit> pAll;
						RWCoCreateInstance(pAll, __uuidof(EnumUnknowns));
						if (pNewSel)
							pAll->Insert(pNewSel);
						CComPtr<ISharedState> pSel;
						m_pCursor->StatePack(pAll, &pSel);
						if (pSel)
						{
							if (cUndo)
								CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pFrame, m_strSelGrp);
							m_pFrame->StateSet(m_strSelGrp, pSel);
						}
					}

					return hRes;
				}
			}
			return E_NOTIMPL;

		case EDVCACopy:
			if (m_hWnd && m_pClipboard.p)
			{
				CWriteLock<IDocument> cLock(m_pDoc); // due to broken self-copies
				//CReadLock<IDocument> cLock(m_pDoc);

				std::vector<IUnknown*> cFrames;
				for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
					cFrames.push_back(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));

				return m_pClipboard->Copy(m_hWnd, cFrames.size(), &cFrames[0]);
			}
			return E_NOTIMPL;

		case EDVCADelete:
			if (m_hWnd)
			{
				CUndoBlock cUndo(m_pDoc);
				{
					CWriteLock<IDocument> cLock(m_pDoc);

					std::vector<IUnknown*> cFrames;
					for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
						cFrames.push_back(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));
					if (cFrames.empty())
						return S_FALSE;

					// find an item to select after deletion
					int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
					int nPrev = m_wndList.GetNextItem(nSel, LVNI_PREVIOUS);
					int nNext = m_wndList.GetNextItem(nSel, LVNI_ALL);
					while (nNext >= 0 && m_wndList.GetItemState(nNext, LVIS_SELECTED) == LVIS_SELECTED)
						nNext = m_wndList.GetNextItem(nNext, LVNI_ALL);
					if (nNext < 0) nNext = nPrev;
					IUnknown* pNewSel = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nNext));

					HRESULT hRes = m_pClipboard->Copy(m_hWnd, cFrames.size(), &cFrames[0]);

					for (int nLast = -1; -1 != (nLast = m_wndList.GetNextItem(nLast, LVNI_SELECTED)); )
						m_pCursor->FrameDel(reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nLast)));

					CComPtr<IEnumUnknownsInit> pAll;
					RWCoCreateInstance(pAll, __uuidof(EnumUnknowns));
					if (pNewSel)
						pAll->Insert(pNewSel);
					CComPtr<ISharedState> pSel;
					m_pCursor->StatePack(pAll, &pSel);
					if (pSel)
					{
						if (cUndo)
							CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pFrame, m_strSelGrp);
						m_pFrame->StateSet(m_strSelGrp, pSel);
					}

					return S_OK;
				}
			}
			return E_UNEXPECTED;

		case EDVCADuplicate:
			{
				CUndoBlock cUndo(m_pDoc);
				CWriteLock<IDocument> cLock(m_pDoc);

				std::vector<IUnknown*> cFrames;
				int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
				if (nSel == -1)
					return S_FALSE; // can only duplicate one selected frame
				IUnknown* pSource = reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nSel));
				int nNext = m_wndList.GetNextItem(nSel, LVNI_ALL);
				IUnknown* pBefore = nNext != -1 ? reinterpret_cast<IUnknown*>(m_wndList.GetItemData(nNext)) : NULL;

				CComPtr<IDocument> pSubDoc;
				m_pCursor->FrameGetDoc(pSource, &pSubDoc);
				CComPtr<IUnknown> pNewSel;
				HRESULT hRes = m_pCursor->FrameIns(pBefore, &CAnimationFrameCreatorDocument(pSubDoc, true), &pNewSel);
				if (FAILED(hRes))
					return hRes;

				CComPtr<IEnumUnknownsInit> pAll;
				RWCoCreateInstance(pAll, __uuidof(EnumUnknowns));
				if (pNewSel)
					pAll->Insert(pNewSel);
				CComPtr<ISharedState> pSel;
				m_pCursor->StatePack(pAll, &pSel);
				if (pSel)
				{
					if (cUndo)
						CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pFrame, m_strSelGrp);
					m_pFrame->StateSet(m_strSelGrp, pSel);
				}
				return S_OK;
			}
			return E_NOTIMPL;

		case EDVCAPaste:
			if (m_pClipboard.p)
			{
				CUndoBlock cUndo(m_pDoc);
				CComPtr<IEnumUnknowns> pNewSel;
				m_bEnableUpdates = false;
				HRESULT hRes = m_pClipboard->Paste(m_hWnd, NULL, &pNewSel);
				m_bEnableUpdates = true;
				if (FAILED(hRes))
					return hRes;
				CComPtr<ISharedState> pSel;
				m_pCursor->StatePack(pNewSel, &pSel);
				if (pSel)
				{
					if (cUndo)
						CSharedStateUndo<ISharedStateManager>::SaveState(cUndo, m_pFrame, m_strSelGrp);
					m_pFrame->StateSet(m_strSelGrp, pSel);
				}
				return hRes;
			}
			return E_FAIL;

		case EDVCASelectAll:
			{
				if (m_strSelGrp.Length() == 0)
					return S_FALSE;
				CComPtr<IEnumUnknowns> pFrames;
				m_pCursor->FramesEnum(&pFrames);
				if (pFrames == NULL)
					return S_FALSE;
				ULONG nSize = 0;
				pFrames->Size(&nSize);
				if (nSize == 0)
					return S_FALSE;
				CComPtr<ISharedState> pState;
				m_pCursor->StatePack(pFrames, &pState);
				if (pState == NULL)
					return S_FALSE;
				return m_pFrame->StateSet(m_strSelGrp, pState);
			}
		case EDVCAInvertSelection:
			{
				if (m_strSelGrp.Length() == 0)
					return S_FALSE;
				CComPtr<ISharedState> pOldState;
				m_pFrame->StateGet(m_strSelGrp, __uuidof(ISharedState), reinterpret_cast<void**>(&pOldState));
				CComPtr<IEnumUnknowns> pOldFrames;
				if (pOldState)
					m_pCursor->StateUnpack(pOldState, &pOldFrames);
				ULONG nOldFrames = 0;
				std::set<IUnknown*> cOld; // HACK: holding pointers without addrefing them (for comparison only)
				CComPtr<IUnknown> p;
				if (pOldFrames)
				{
					for (ULONG i = 0; SUCCEEDED(pOldFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&p))); ++i, p = NULL)
						cOld.insert(p);
				}
				CComPtr<IEnumUnknowns> pFrames;
				m_pCursor->FramesEnum(&pFrames);
				if (pFrames == NULL)
					return S_FALSE;
				CComPtr<IEnumUnknownsInit> pNewFrames;
				RWCoCreateInstance(pNewFrames, __uuidof(EnumUnknowns));
				for (ULONG i = 0; SUCCEEDED(pFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&p))); ++i, p = NULL)
				{
					if (cOld.find(p) == cOld.end())
						pNewFrames->Insert(p);
				}
				CComPtr<ISharedState> pState;
				m_pCursor->StatePack(pNewFrames, &pState);
				if (pState == NULL)
					return S_FALSE;
				return m_pFrame->StateSet(m_strSelGrp, pState);
			}
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewAnimationFrames::Drag(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback)
{
	try
	{
		if (!IsWindow() || m_pClipboard.p == NULL || a_pFileNames == NULL || a_pDataObj == NULL)
			return E_FAIL;
		RECT rc;
		GetWindowRect(&rc);
		if (a_pt.x < rc.left || a_pt.x >= rc.right || a_pt.y < rc.top || a_pt.y >= rc.bottom)
			return E_FAIL;

		if (m_pClipboard->CanDrop(a_pFileNames) != S_OK)
			return E_FAIL;

		if (a_pdwEffect) *a_pdwEffect = DROPEFFECT_COPY;
		if (a_ppFeedback) *a_ppFeedback = _SharedStringTable.GetString(IDS_DROPMSG_APPENDFRAMES);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDesignerViewAnimationFrames::Drop(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt)
{
	try
	{
		if (a_pDataObj == NULL && a_pFileNames == NULL)
			return E_RW_CANCELLEDBYUSER; // cancelled d'n'd operation

		if (!IsWindow() || m_pClipboard.p == NULL || a_pFileNames == NULL)
			return E_FAIL;
		RECT rc;
		GetWindowRect(&rc);
		if (a_pt.x < rc.left || a_pt.x >= rc.right || a_pt.y < rc.top || a_pt.y >= rc.bottom)
			return E_FAIL;

		CWaitCursor cWait;

		CUndoBlock cBlock(m_pDoc, CMultiLanguageString::GetAuto(L"[0409]Drag and drop frames[0405]Přetáhnout snímky"));
		CComPtr<IEnumUnknowns> pNewSel;
		m_bEnableUpdates = false;
		HRESULT hRes = m_pClipboard->Drop(a_pFileNames, NULL, &pNewSel);
		m_bEnableUpdates = true;
		if (FAILED(hRes))
			return hRes;
		CComPtr<ISharedState> pSel;
		m_pCursor->StatePack(pNewSel, &pSel);
		if (pSel)
		{
			if (cBlock)
				CSharedStateUndo<ISharedStateManager>::SaveState(cBlock, m_pFrame, m_strSelGrp);
			m_pFrame->StateSet(m_strSelGrp, pSel);
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

LRESULT CDesignerViewAnimationFrames::OnHelp(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM a_lParam, BOOL& a_bHandled)
{
	HELPINFO const* const pHelpInfo = reinterpret_cast<HELPINFO const* const>(a_lParam);
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		TCHAR szBuffer[512] = _T("");
		Win32LangEx::LoadString(_pModule->get_m_hInst(), IDS_AF_CONTEXTHELP, szBuffer, itemsof(szBuffer), LANGIDFROMLCID(m_tLocaleID));
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


/*
STDMETHODIMP CDesignerViewAnimationFrames::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	LVINSERTMARK tMark;
	tMark.cbSize = sizeof tMark;
	tMark.dwFlags = 0;
	tMark.iItem = 1;
	tMark.dwReserved = 0;
	m_wndList.SetInsertMark(&tMark);
	return S_OK;
}

STDMETHODIMP CDesignerViewAnimationFrames::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	LVINSERTMARK tMark;
	tMark.cbSize = sizeof tMark;
	tMark.dwFlags = 0;
	tMark.iItem = 1;
	tMark.dwReserved = 0;
	m_wndList.SetInsertMark(&tMark);
	return S_OK;
}

STDMETHODIMP CDesignerViewAnimationFrames::DragLeave()
{
	LVINSERTMARK tMark;
	tMark.cbSize = sizeof tMark;
	tMark.dwFlags = 0;
	tMark.iItem = -1;
	tMark.dwReserved = 0;
	m_wndList.SetInsertMark(&tMark);
	return S_OK;
}

STDMETHODIMP CDesignerViewAnimationFrames::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	return S_OK;
}

*/