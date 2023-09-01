// DesignerViewAnimationFrames.h : Declaration of the CDesignerViewAnimationFrames

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"
#include <ObserverImpl.h>
#include <RWDocumentAnimation.h>


// CDesignerViewAnimationFrames

class ATL_NO_VTABLE CDesignerViewAnimationFrames :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CWindowImpl<CDesignerViewAnimationFrames>,
	public CDesignerViewWndImpl<CDesignerViewAnimationFrames, IDesignerView>,
	public CObserverImpl<CDesignerViewAnimationFrames, IAnimationObserver, TAnimationChange>,
	public CObserverImpl<CDesignerViewAnimationFrames, ISharedStateObserver, TSharedStateChange>,
	public IDesignerViewClipboardHandler,
	public IDragAndDropHandler
{
public:
	CDesignerViewAnimationFrames() : m_bEnableUpdates(true), m_bSelectionUpdatePosted(false), m_bDragging(false),
		m_hFramesMove(NULL), m_hFramesCopy(NULL), m_hFramesBad(NULL), m_bValidDND(false), m_nIconSize(48)
	{
		ZeroMemory(m_aImageCache, sizeof(m_aImageCache));
	}
	~CDesignerViewAnimationFrames()
	{
		if (m_hFramesMove) DestroyCursor(m_hFramesMove);
		if (m_hFramesCopy) DestroyCursor(m_hFramesCopy);
	}

	void Init(ISharedStateManager* a_pFrame, CConfigValue& a_cVal, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc);

BEGIN_COM_MAP(CDesignerViewAnimationFrames)
	COM_INTERFACE_ENTRY(IDesignerView)
	COM_INTERFACE_ENTRY(IDesignerViewClipboardHandler)
	COM_INTERFACE_ENTRY(IDragAndDropHandler)
END_COM_MAP()

	enum {
		WM_UPDATESELECTION = WM_APP+893,
		IMAGECACHESIZE = 64,
	};
BEGIN_MSG_MAP(CDesignerViewAnimationFrames)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnButtonUp)
	MESSAGE_HANDLER(WM_RBUTTONUP, OnButtonUp)
	NOTIFY_CODE_HANDLER(NM_CUSTOMDRAW, OnCustomDraw)
	NOTIFY_CODE_HANDLER(LVN_BEGINLABELEDIT, OnBeginLabelEdit)
	NOTIFY_CODE_HANDLER(LVN_ENDLABELEDIT, OnEndLabelEdit)
	NOTIFY_CODE_HANDLER(LVN_BEGINDRAG, OnBeginDrag)
	NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
	MESSAGE_HANDLER(WM_UPDATESELECTION, OnUpdateSelection)
	NOTIFY_CODE_HANDLER(LVN_KEYDOWN, OnKeyDown)
	//COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
	MESSAGE_HANDLER(WM_HELP, OnHelp)
END_MSG_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		FreeAllCachedImages();
		if (m_pCursor)
			m_pCursor->ObserverDel(CObserverImpl<CDesignerViewAnimationFrames, IAnimationObserver, TAnimationChange>::ObserverGet(), 0);
		if (m_pFrame)
			m_pFrame->ObserverDel(CObserverImpl<CDesignerViewAnimationFrames, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
	}

	void OnFinalMessage(HWND)
	{
		Release();
	}

	// method called by CObserverImpl
public:
	void OwnerNotify(TCookie a_tCookie, TAnimationChange a_tChange)
	{
		if (a_tChange.nFlags & EACAnimation)
		{
			Data2GUI();
		}
		for (ULONG i = 0; i < a_tChange.nFrames; ++i)
		{
			ObjectLock cLock(this);
			CFrames::const_iterator j = m_cFrames.find(a_tChange.apFrames[i]);
			if (j != m_cFrames.end())
			{
				RECT rc;
				if (m_wndList.GetItemRect(j->second, &rc, LVIR_BOUNDS))
				{
					InvalidateRect(&rc, FALSE);
				}
			}
			for (SImageCache* p = m_aImageCache; p < m_aImageCache+IMAGECACHESIZE; ++p)
			{
				if (p->pFrame == a_tChange.apFrames[i])
				{
					FreeCachedImage(p, 0);
					break;
				}
			}
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tChange)
	{
		if (a_tChange.bstrName && m_strSelGrp == a_tChange.bstrName)
		{
			Selection2GUI(a_tChange.pState);
		}
	}
	//void OwnerNotify(TCookie a_tCookie, TIconFormat a_tFormat)
	//{
	//	OwnerNotify(a_tCookie, ULONG(0));
	//}

	// IDesignerViewClipboardHandler methods
public:
	STDMETHOD(Priority)(BYTE* a_pPrio) { return E_NOTIMPL; }
	STDMETHOD(ObjectName)(EDesignerViewClipboardAction a_eAction, ILocalizedString** a_ppName);
	STDMETHOD(ObjectIconID)(EDesignerViewClipboardAction a_eAction, GUID* a_pIconID);
	STDMETHOD(ObjectIcon)(EDesignerViewClipboardAction a_eAction, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay);
	STDMETHOD(Check)(EDesignerViewClipboardAction a_eAction);
	STDMETHOD(Exec)(EDesignerViewClipboardAction a_eAction);

	// IDragAndDropHandler methods
public:
	STDMETHOD(Drag)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt, DWORD* a_pdwEffect, ILocalizedString** a_ppFeedback);
	STDMETHOD(Drop)(IDataObject* a_pDataObj, IEnumStrings* a_pFileNames, DWORD a_grfKeyState, POINT a_pt);

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		if (m_bValidDND && a_bBeforeAccel && a_pMsg->hwnd && a_pMsg->hwnd == m_wndList.m_hWnd && GetCapture() == m_hWnd)
		{
			if (a_pMsg->message == WM_KEYDOWN && a_pMsg->wParam == VK_CONTROL)
			{
				if (m_hFramesCopy == NULL)
					m_hFramesCopy = CreateFrameManipulationCursor(true);
				SetCursor(m_hFramesCopy);
			}
			else if (a_pMsg->message == WM_KEYUP && a_pMsg->wParam == VK_CONTROL)
			{
				if (m_hFramesMove == NULL)
					m_hFramesMove = CreateFrameManipulationCursor(false);
				SetCursor(m_hFramesMove);
			}
		}
		if (a_bBeforeAccel && m_wndList.m_hWnd)
		{
			HWND hEdit = m_wndList.GetEditControl();
			if (hEdit != NULL && hEdit == a_pMsg->hwnd)
			{
				TranslateMessage(a_pMsg);
				DispatchMessage(a_pMsg);
				return S_OK;
			}
		}
		return S_FALSE;
	}

public:
	LRESULT OnCreate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnMouseMove(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnButtonUp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnSize(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndList.SetWindowPos(NULL, 0, 0, LOWORD(a_lParam), HIWORD(a_lParam), SWP_NOMOVE|SWP_NOOWNERZORDER);

		return 0;
	}
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndList.SetFocus();
		return 0;
	}
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndList.GetEditControl().m_hWnd)
			return MA_NOACTIVATE;

		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);

		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			m_wndList.SetFocus();
		}

		return lRet;
	}
	LRESULT OnUpdateSelection(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);
	LRESULT OnCustomDraw(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnBeginLabelEdit(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnEndLabelEdit(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnBeginDrag(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnItemChanged(int a_idCtrl, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnKeyDown(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled);
	LRESULT OnHelp(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled);

private:
	typedef std::map<CComPtr<IUnknown>, int> CFrames;
	struct SImageCache
	{
		IUnknown* pFrame;
		BYTE* pData;
		int nSizeX;
		int nSizeY;
		int nAge;
	};

private:
	void Data2GUI();
	void Selection2GUI(ISharedState* a_pState, bool a_bForceFocus = false);
	bool GetImage(IUnknown* a_pFrame, BYTE** a_ppData, int* a_pSizeX, int* a_pSizeY);
	void FreeCachedImage(SImageCache* a_p, int a_nCookie);
	void FreeAllCachedImages();
	void AgeCachedImages();
	static HCURSOR CreateFrameManipulationCursor(bool copy);

private:
	CListViewCtrl m_wndList;
	LCID m_tLocaleID;

	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentAnimation> m_pCursor;
	CComPtr<IDocumentAnimationClipboard> m_pClipboard;
	CComPtr<ISharedStateManager> m_pFrame;
	CComBSTR m_strSelGrp;
	bool m_bEnableUpdates;
	bool m_bSelectionUpdatePosted;
	bool m_bDragging;

	CFrames m_cFrames;

	SImageCache m_aImageCache[IMAGECACHESIZE];
	ULONG m_nIconSize;

	HCURSOR m_hFramesMove;
	HCURSOR m_hFramesCopy;
	HCURSOR m_hFramesBad;
	bool m_bValidDND;
};

