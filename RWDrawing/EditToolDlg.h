
#pragma once

#include "SharedStateToolData.h"


#include <ObserverImpl.h>

template<class T, class TData, class TDlgBase = Win32LangEx::CLangDialogImpl<T> >
class CEditToolDlgBase :
	public CComObjectRootEx<CComMultiThreadModel>,
	public TDlgBase,
	public CObserverImpl<T, ISharedStateObserver, TSharedStateChange>,
	public CChildWindowImpl<T, IRasterImageEditToolWindow>
{
public:
	CEditToolDlgBase() : m_bEnableUpdates(false)
	{
	}
	~CEditToolDlgBase()
	{
		if (m_pSharedState)
			m_pSharedState->ObserverDel(ObserverGet(), 0);
	}
	HWND Create(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup)
	{
		m_tLocaleID = a_tLocaleID;
		m_pSharedState = a_pSharedState;
		m_bstrSyncToolData = a_bstrSyncGroup;
		m_pSharedState->ObserverIns(ObserverGet(), 0);
		return TDlgBase::Create(a_hParent);
	}
	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }
	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{ return GetParent().SendMessage(a_uMsg, a_wParam, a_lParam); }

	void DataToState()
	{
		if (m_pSharedState != NULL)
		{
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			if (pNew->Init(m_cData))
			{
				m_bEnableUpdates = false;
				m_pSharedState->StateSet(m_bstrSyncToolData, pTmp);
				m_bEnableUpdates = true;
			}
		}
	}
	void AddWindowForPreTranslate(HWND a_hWnd)
	{
		m_aEditControls.push_back(a_hWnd);
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (a_bBeforeAccel && m_hWnd && a_pMsg->hwnd)
		{
			for (std::vector<HWND>::const_iterator i = m_aEditControls.begin(); i != m_aEditControls.end(); ++i)
			{
				if (*i == a_pMsg->hwnd)
				{
					if (IsDialogMessage(const_cast<LPMSG>(a_pMsg)))
						return S_OK;
					TranslateMessage(a_pMsg);
					DispatchMessage(a_pMsg);
					return S_OK;
				}
			}
		}
		return S_FALSE;
	}

protected:
	CComPtr<ISharedStateManager> m_pSharedState;
	CComBSTR m_bstrSyncToolData;
	TData m_cData;
	bool m_bEnableUpdates;

private:
	std::vector<HWND> m_aEditControls;
};

#include <IconRenderer.h>
#include <set>

struct IRAutoCanvas : public IRCanvas
{
	operator IRCanvas const*() { return this; }
	IRAutoCanvas(ULONG n, IRPolyPoint const* p)
	{
		x0 = y0 = 1e6f;
		x1 = y1 = -1e6f;
		countX = countY = 0;
		itemsX = itemsY = NULL;

		std::set<float> gridX;
		std::set<float> gridY;
		for (ULONG i = 0; i < n; ++i)
		{
			if (x0 > p[i].x) x0 = p[i].x;
			if (y0 > p[i].y) y0 = p[i].y;
			if (x1 < p[i].x) x1 = p[i].x;
			if (y1 < p[i].y) y1 = p[i].y;
			char j = i == 0 ? n-1 : i-1;
			if (p[i].x == p[j].x) gridX.insert(p[i].x);
			if (p[i].y == p[j].y) gridY.insert(p[i].y);
		}
		if (gridX.size()+gridY.size() > 0)
		{
			mem.Allocate(gridX.size()+gridY.size());
			itemsX = mem;
			itemsY = itemsX+gridX.size();
		}
		for (std::set<float>::const_iterator i = gridX.begin(); i != gridX.end(); ++i)
		{
			const_cast<IRGridItem*>(itemsX)[countX].flags = 0;
			const_cast<IRGridItem*>(itemsX)[countX].pos = *i;
			++countX;
		}
		for (std::set<float>::const_iterator i = gridY.begin(); i != gridY.end(); ++i)
		{
			const_cast<IRGridItem*>(itemsY)[countY].flags = 0;
			const_cast<IRGridItem*>(itemsY)[countY].pos = *i;
			++countY;
		}
	}

private:
	CAutoVectorPtr<IRGridItem> mem;
};

inline HICON IconFromPolygon(IStockIcons* pSI, IIconRenderer* pIR, int a_nVertices, IRPolyPoint const* a_pVertices, int a_nSize, bool a_bAutoScale, int a_nOverrideWidth = 0)
{
	IRAutoCanvas canvas(a_nVertices, a_pVertices);
	if (!a_bAutoScale)
	{
		canvas.x0 = canvas.y0 = 0;
		canvas.x1 = canvas.y1 = 1;
	}
	IRPolygon const poly = {a_nVertices, a_pVertices};
	if (a_nOverrideWidth == 0 || a_nOverrideWidth == a_nSize)
		return pIR->CreateIcon(a_nSize, &canvas, 1, &poly, pSI->GetMaterial(ESMInterior));
	CAutoVectorPtr<DWORD> buffer(new DWORD[a_nOverrideWidth*a_nSize]);
	ZeroMemory(buffer.m_p, a_nOverrideWidth*a_nSize*sizeof(DWORD));
	pIR->RenderLayer(a_nSize, a_nSize, a_nOverrideWidth, buffer.m_p+((a_nOverrideWidth-a_nSize)>>1), &canvas, 1, 0, &poly, NULL, pSI->GetMaterial(ESMInterior));
	return pIR->CreateIcon(a_nOverrideWidth, a_nSize, a_nOverrideWidth, buffer);
}


//class CEditToolDlg
//{
//public:
//	virtual ~CEditToolDlg() {}
//
//	virtual void UpdateValues() = 0;
//	virtual CWindow& Me() = 0;
//};
//
//typedef CEditToolDlg* (*pfnEditToolPropertiesDlgCreator)(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup);
//
//
//// helpers
//
//template<class T>
//class CEditToolDlgImpl : public CEditToolDlg
//{
//public:
//	void UpdateValues()
//	{
//	}
//
//	CWindow& Me()
//	{
//		return *static_cast<T*>(this); // T must be window!
//	}
//
//	static CEditToolDlg* Creator(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup)
//	{
//		T* pDlg = new T(a_tLocaleID);
//		pDlg->m_pszToolID = a_pszToolID;
//		pDlg->m_pSharedState = a_pSharedState;
//		pDlg->m_bstrSyncGroup = a_bstrSyncGroup;
//		pDlg->Create(a_hParent);
//		return pDlg;
//	}
//	LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
//	{ return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
//	LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
//	{ SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW)); return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
//
//protected:
//	// these are only references - lifetime is controlled by owner
//	LPCOLESTR m_pszToolID;
//	ISharedStateManager* m_pSharedState;
//	BSTR m_bstrSyncGroup;
//};

