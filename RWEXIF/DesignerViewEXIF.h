// DesignerViewEXIF.h : Declaration of the CDesignerViewEXIF

#pragma once
#include "RWEXIF_i.h"
#include <Win32LangEx.h>
#include <ObserverImpl.h>
#include "PropertyList.h"
#include <SimpleLocalizedString.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>

#include "config.h"

#include "libexif/exif-data.h"
#include "libexif/exif-ifd.h"
#include "libexif/exif-loader.h"


// CDesignerViewEXIF

class ATL_NO_VTABLE CDesignerViewEXIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public Win32LangEx::CLangIndirectDialogImpl<CDesignerViewEXIF>,
	public CThemeImpl<CDesignerViewEXIF>,
	public CDialogResize<CDesignerViewEXIF>,
	public CDesignerViewWndImpl<CDesignerViewEXIF, IDesignerView>,
	public CObserverImpl<CDesignerViewEXIF, ISharedStateObserver, TSharedStateChange>,
	public CObserverImpl<CDesignerViewEXIF, IImageObserver, TImageChange>
{
public:
	CDesignerViewEXIF() : m_bBorder(false), m_bUpdating(false)
	{
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE);
		SetThemeClassList(L"ListView");
	}

	void Init(IConfig* a_pConfig, ISharedStateManager* a_pFrame, RWHWND a_hParent, RECT const* a_rcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDocument* a_pDoc, IImageMetaData* a_pIMD)
	{
		m_bBorder = (a_nStyle & EDVWSBorderMask) == EDVWSBorder;

		m_tLocaleID = a_tLocaleID;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cVal);

		m_bstrSelectionID = cVal;
		if (m_bstrSelectionID.Length())
		{
			a_pFrame->ObserverIns(CObserverImpl<CDesignerViewEXIF, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
			m_pFrame = a_pFrame;
		}
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&m_pImage));
		if (m_pImage)
			m_pImage->ObserverIns(CObserverImpl<CDesignerViewEXIF, IImageObserver, TImageChange>::ObserverGet(), 0);
		m_pIMD = a_pIMD;

		m_pDoc = a_pDoc;

		// create self
		if (Create(a_hParent) == NULL)
		{
			// creation failed
			throw E_FAIL; // TODO: error code
		}

		MoveWindow(const_cast<LPRECT>(a_rcWindow));
		ShowWindow(SW_SHOW);
	}

	enum
	{
		IDC_EXIF_PROPERTIES = 100,
	};

BEGIN_DIALOG_EX(0, 0, m_bBorder ? 185 : 187, 94, 0)
	DIALOG_FONT_AUTO()
	DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
	DIALOG_EXSTYLE(m_bBorder ? WS_EX_CONTROLPARENT | WS_EX_CLIENTEDGE : WS_EX_CONTROLPARENT)
END_DIALOG()

BEGIN_CONTROLS_MAP()
	//CONTROL_LISTBOX(IDC_EXIF_PROPERTIES, 0, 0, m_bBorder ? 186 : 187, 94, LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0); // cannot use (forces WS_BORDER)
	m_Template.AddStdControl(m_Template.CTRL_LISTBOX, (WORD)IDC_EXIF_PROPERTIES, 0, 0, m_bBorder ? 186 : 187, 94, LBS_NOTIFY | LBS_OWNERDRAWVARIABLE | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP | WS_VISIBLE, 0, (LPCTSTR)NULL, NULL, 0);
END_CONTROLS_MAP()

BEGIN_COM_MAP(CDesignerViewEXIF)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

BEGIN_MSG_MAP(CDesignerViewEXIF)
	CHAIN_MSG_MAP(CThemeImpl<CDesignerViewEXIF>)
	//MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	//MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate)
	COMMAND_HANDLER(IDC_EXIF_PROPERTIES, LBN_SELCHANGE, OnSelChange)
	//COMMAND_HANDLER(IDOK, BN_CLICKED, OnApplyFramesTime)
	NOTIFY_HANDLER(IDC_EXIF_PROPERTIES, PIN_ITEMCHANGED, OnItemChanged)
	CHAIN_MSG_MAP(CDialogResize<CDesignerViewEXIF>)
	REFLECT_NOTIFICATIONS()
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CDesignerViewAnimatedCursorInfo)
	DLGRESIZE_CONTROL(IDC_EXIF_PROPERTIES, DLSZ_SIZE_X|DLSZ_SIZE_Y)
END_DLGRESIZE_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		if (m_pFrame)
			m_pFrame->ObserverDel(CObserverImpl<CDesignerViewEXIF, ISharedStateObserver, TSharedStateChange>::ObserverGet(), 0);
		if (m_pImage)
			m_pImage->ObserverDel(CObserverImpl<CDesignerViewEXIF, IImageObserver, TImageChange>::ObserverGet(), 0);
	}

	// IChildWindow methods
public:
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{
		//if (a_bBeforeAccel && a_pMsg->hwnd && m_wndEditTime.m_hWnd == a_pMsg->hwnd)
		//{
		//	if ((a_pMsg->message == WM_KEYDOWN || a_pMsg->message == WM_SYSKEYDOWN) &&
		//		(a_pMsg->wParam == 'C' || a_pMsg->wParam == 'X' || a_pMsg->wParam == 'V' ||
		//		a_pMsg->wParam == VK_INSERT || a_pMsg->wParam == VK_DELETE || a_pMsg->wParam == 'Z'))
		//	{
		//		TranslateMessage(a_pMsg);
		//		DispatchMessage(a_pMsg);
		//		return S_OK;
		//	}
		//}
		return S_FALSE;
	}

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{
		try
		{
			SIZE sz;
			if (!GetDialogSize(&sz, LANGIDFROMLCID(m_tLocaleID)))
				return E_FAIL;
			if (a_pSize->cx < sz.cx) a_pSize->cx = sz.cx;
			if (a_pSize->cy < sz.cy) a_pSize->cy = sz.cy;
			return S_OK;
		}
		catch (...)
		{
			return a_pSize == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges)
	{
		//if (!a_bCancelChanges && m_hWnd && GetFocus() == m_wndEditTime)
		//{
		//	BOOL b;
		//	OnApplyFramesTime(0, 0, 0, b);
		//}
		return S_OK;
	}

	// message handlers
public:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndList.SubclassWindow(GetDlgItem(IDC_EXIF_PROPERTIES));

		m_wndList.SetExtendedListStyle(PLS_EX_CATEGORIZED|PLS_EX_XPLOOK|PLS_EX_SHOWSELALWAYS);

		DlgResize_Init(false, false, 0);

		Data2GUI();

		return 0;
	}
	//LRESULT OnCtlColorDlg(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{ return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
	//LRESULT OnCtlColorStatic(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	//{ SetBkColor((HDC)a_wParam, GetSysColor(COLOR_WINDOW)); return (LRESULT)GetSysColorBrush(COLOR_WINDOW); }
	LRESULT OnSetFocus(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndList.SetFocus();
		return 0;
	}
	LRESULT OnMouseActivate(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (m_wndList.m_hwndInplace)
			return MA_NOACTIVATE;

		LRESULT lRet = GetParent().SendMessage(a_uMsg, a_wParam, a_lParam);

		if (lRet == MA_ACTIVATE || lRet == MA_ACTIVATEANDEAT)
		{
			m_wndList.SetFocus();
		}

		return lRet;
	}
	LRESULT OnSelChange(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		if (m_pFrame == NULL)
			return 0;

		HPROPERTY hProp = m_wndList.GetProperty(m_wndList.GetCurSel());
		CComBSTR bstr;
		if (hProp)
		{
			LPARAM lPar = hProp->GetItemData();
			if ((lPar&0xffff) != 0xffff)
			{
				OLECHAR szTmp[32];
				_swprintf(szTmp, L"%i/%i", (lPar>>16)&0xffff, lPar&0xffff);
				bstr = szTmp;
			}
		}
		CComPtr<ISharedState> pState;
		RWCoCreateInstance(pState, __uuidof(SharedStateString));
		pState->FromText(bstr);
		m_bUpdating = true;
		m_pFrame->StateSet(m_bstrSelectionID, pState);
		m_bUpdating = false;
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnItemChanged(int a_nCtrlID, LPNMHDR a_pNMHeader, BOOL& a_bHandled)
	{
		if (m_bUpdating)
			return 0;
		NMPROPERTYITEM* pItem = reinterpret_cast<NMPROPERTYITEM*>(a_pNMHeader);
		if (pItem->prop == NULL)
			return 0;
		LPARAM lParam = pItem->prop->GetItemData();
		ExifTag eTag = static_cast<ExifTag>(lParam&0xffff);
		ExifIfd eIFD = static_cast<ExifIfd>((lParam>>16)&0xffff);
		CComBSTR bstrEXIF(L"EXIF");
		ULONG nSize = 0;
		m_pIMD->GetBlockSize(bstrEXIF, &nSize);
		if (nSize == 0)
			return 0;
		CAutoVectorPtr<BYTE> cEXIF(new BYTE[nSize]);
		m_pIMD->GetBlock(bstrEXIF, nSize, cEXIF);
		ExifData* pData = exif_data_new_from_data(cEXIF, nSize);
		if (pData == NULL)
			return 0;
		ExifByteOrder o = exif_data_get_byte_order(pData);
		ExifEnumeratedValues const* pEVals = exif_get_enumeratedvalues_table();

		ExifContent* pContent = pData->ifd[eIFD];
		ExifEntry* pEntry = exif_content_get_entry(pContent, eTag);
		if (pItem->prop->GetKind() == PROPKIND_LIST)
		{
			CComVariant cVal;
			pItem->prop->GetValue(&cVal);
			int value = cVal.intVal;
			int iVal = 0;
			while (pEVals[iVal].tag && pEVals[iVal].tag != eTag)
				++iVal;
			if (pEVals[iVal].tag)
			{
				ExifEnumeratedValues::TElem const* pElems = pEVals[iVal].elem;
				while (*pElems->values)
				{
					if (pElems-pEVals[iVal].elem == value)
					{
						value = pElems->index;
						break;
					}
					++pElems;
				}
			}
			exif_set_short(pEntry->data, o, value);
		}
		else if (pItem->prop->GetKind() == PROPKIND_EDIT)
		{
			CComVariant cVal;
			pItem->prop->GetValue(&cVal);
			CComBSTR bstr(cVal.bstrVal);
			if (bstr == NULL) bstr = L"";
			std::vector<LPOLESTR> aComponents;
			aComponents.push_back(bstr.m_str);

			if (pEntry->components > 1)
			{
				for (LPOLESTR p = bstr.m_str; *p; ++p)
				{
					if (*p == L',')
					{
						*p = L'\0';
						aComponents.push_back(p+1);
					}
				}
			}
			if (pEntry->components < aComponents.size())
				aComponents.resize(pEntry->components);

			for (ULONG  i = 0; i < aComponents.size(); ++i)
			{
				switch (pEntry->format)
				{
				case EXIF_FORMAT_BYTE:
				case EXIF_FORMAT_SBYTE:
					pEntry->data[i] = _wtoi(aComponents[i]);
					break;
				case EXIF_FORMAT_ASCII:
					{
						int nSrcLen = wcslen(aComponents[i]);
						CAutoVectorPtr<char> psz(new char[nSrcLen*4+4]);
						exif_convert_utf16_to_utf8(psz, reinterpret_cast<unsigned short*>(aComponents[i]), nSrcLen*4+4);
						pEntry->data = reinterpret_cast<BYTE*>(realloc(pEntry->data, strlen(psz)+1));
						strcpy(reinterpret_cast<char*>(pEntry->data), psz);
					}
					break;
				case EXIF_FORMAT_SHORT:
				case EXIF_FORMAT_SSHORT:
					exif_set_short(pEntry->data+i*2, o, _wtoi(aComponents[i]));
					break;
				case EXIF_FORMAT_LONG:
				case EXIF_FORMAT_SLONG:
					exif_set_long(pEntry->data+i*4, o, _wtoi(aComponents[i]));
					break;
				case EXIF_FORMAT_RATIONAL:
				case EXIF_FORMAT_SRATIONAL:
					{
						LPOLESTR pDiv = wcschr(aComponents[i], L'/');
						if (pDiv)
						{
							ExifRational eRat;
							eRat.numerator = _wtoi(aComponents[i]);
							eRat.denominator = _wtoi(pDiv+1);
							exif_set_rational(pEntry->data+i*8, o, eRat);
						}
						else
						{
							OLECHAR* p2;
							double d = wcstod(aComponents[i], &p2);
							ExifRational eRat = exif_get_rational(pEntry->data+i*8, o);
							if (eRat.denominator == 0) eRat.denominator = 100;
							eRat.numerator = d*eRat.denominator;
							exif_set_rational(pEntry->data+i*8, o, eRat);
						}
					}
					break;
				case EXIF_FORMAT_UNDEFINED:
					break;
				case EXIF_FORMAT_FLOAT:
					break;
				case EXIF_FORMAT_DOUBLE:
					break;
				}
			}
		}
		else
		{
			exif_data_unref(pData);
			m_bUpdating = false;
			ATLASSERT(FALSE);
			return 0;
		}
		BYTE* pNewData = NULL;
		unsigned int nNewSize = 0;
		exif_data_save_data(pData, &pNewData, &nNewSize);
		exif_data_unref(pData);
		m_bUpdating = true;
		{
			const char* pName = exif_tag_get_title_in_ifd(eTag, eIFD);
			if (pName == NULL) pName = "unknown";
			CComPtr<ILocalizedString> pTagName;
			pTagName.Attach(new CSimpleLocalizedString(::SysAllocString(CA2W(pName))));
			CComObject<CPrintfLocalizedString>* p = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&p);
			CComPtr<ILocalizedString> pStr = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]Modify \"%s\" EXIF tag[0405]Změna EXIF tagu \"%s\""), pTagName);
			CUndoBlock cBlock(m_pDoc, pStr);
			m_pIMD->SetBlock(bstrEXIF, nNewSize, pNewData);
		}
		m_bUpdating = false;
		return 0;
	}

public:
	void OwnerNotify(TCookie, TImageChange a_tChange)
	{
		if (a_tChange.nGlobalFlags&EICEXIF && !m_bUpdating && m_hWnd)
		{
			Data2GUI();
		}
	}
	void OwnerNotify(TCookie, TSharedStateChange a_tChange)
	{
		if (m_hWnd && !m_bUpdating && m_bstrSelectionID == a_tChange.bstrName)
		{
			HPROPERTY hProp = m_wndList.GetProperty(m_wndList.GetCurSel());
			CComBSTR bstr;
			if (hProp)
			{
				LPARAM lPar = hProp->GetItemData();
				if ((lPar&0xffff) != 0xffff)
				{
					OLECHAR szTmp[32];
					_swprintf(szTmp, L"%i/%i", (lPar>>16)&0xffff, lPar&0xffff);
					bstr = szTmp;
				}
			}
			CComBSTR bstrNew;
			a_tChange.pState->ToText(&bstrNew);
			if (bstrNew != bstr)
			{
				Data2GUI();
			}
		}
	}

	void Data2GUI()
	{
		m_bUpdating = true;
		m_wndList.SetRedraw(FALSE);

		int iTopIndex = m_wndList.GetTopIndex();
		std::vector<bool> aExpanded;
		int iCount = m_wndList.GetCount();
		int j = 0;
		for (int i = 0; i < iCount; ++i)
		{
			HPROPERTY hProp = m_wndList.GetProperty(i);
			if (hProp->GetKind() == PROPKIND_CATEGORY)
				aExpanded.push_back(static_cast<CCategoryProperty*>(hProp)->IsExpanded());
		}
		while (aExpanded.size() < EXIF_IFD_COUNT)
			aExpanded.push_back(true);
		int iSelected = m_wndList.GetCaretIndex();
		int iSelIFD = -2;
		int iSelTag = -1;
		if (iSelected > 0)
		{
			HPROPERTY hProp = m_wndList.GetProperty(iSelected);
			LPARAM lPar = hProp->GetItemData();
			iSelIFD = (lPar>>16)&0xffff;
			if ((lPar&0xffff) != 0xffff)
			{
				iSelTag = lPar&0xffff;
			}
		}

		m_wndList.ResetContent();

		CComBSTR bstrEXIF(L"EXIF");
		ULONG nSize = 0;
		m_pIMD->GetBlockSize(bstrEXIF, &nSize);
		ExifData* pData = NULL;
		if (nSize > 0)
		{
			CAutoVectorPtr<BYTE> cEXIF(new BYTE[nSize]);
			m_pIMD->GetBlock(bstrEXIF, nSize, cEXIF);
			pData = exif_data_new_from_data(cEXIF, nSize);
		}
		ExifIndexedValues const* pIVals = exif_get_indexedvalues_table();
		ExifEnumeratedValues const* pEVals = exif_get_enumeratedvalues_table();
		ExifByteOrder o = exif_data_get_byte_order(pData);
		if (pData) for (int i = EXIF_IFD_0; i < EXIF_IFD_COUNT; ++i)
		{
			CA2T strName(exif_ifd_get_name(static_cast<ExifIfd>(i)));
			HPROPERTY hCat = m_wndList.AddItem(PropCreateCategory(strName, (i<<16)|0xffff));
			if (iSelIFD == i && iSelTag == -1)
				m_wndList.SetCurSel(m_wndList.FindProperty(hCat));
			ExifContent* pContent = pData->ifd[i];
			for (unsigned int j = 0; j < pContent->count; ++j)
			{
				ExifEntry* pEntry = pContent->entries[j];
				CA2T strTagName(exif_tag_get_name_in_ifd(pEntry->tag, static_cast<ExifIfd>(i)));

				bool bSelect = iSelIFD == i && iSelTag == pEntry->tag;

				// check if the tag is 1-of-N type
				int iVal = 0;
				while (pIVals[iVal].tag && pIVals[iVal].tag != pEntry->tag)
					++iVal;
				if (pIVals[iVal].tag)
				{
					CPropertyListItem* p = new CPropertyListItem(strTagName, (i<<16)|pEntry->tag);
					char const* const* pStrs = pIVals[iVal].strings;
					while (*pStrs)
					{
						p->AddListItem(CA2CT(*pStrs));
						++pStrs;
					}
					p->SetValue(CComVariant(int(exif_get_short(pEntry->data, o))));
					HPROPERTY hItem = m_wndList.AddItem(p);
					if (bSelect) m_wndList.SetCurSel(m_wndList.FindProperty(hItem));
					continue;
				}
				iVal = 0;
				while (pEVals[iVal].tag && pEVals[iVal].tag != pEntry->tag)
					++iVal;
				if (pEVals[iVal].tag)
				{
					int value = int(exif_get_short(pEntry->data, o));
					CPropertyListItem* p = new CPropertyListItem(strTagName, (i<<16)|pEntry->tag);
					ExifEnumeratedValues::TElem const* pElems = pEVals[iVal].elem;
					int iSel = 0;
					while (*pElems->values)
					{
						p->AddListItem(CA2CT(*pElems->values));
						if (pElems->index == value)
							iSel = pElems-pEVals[iVal].elem;
						++pElems;
					}
					p->SetValue(CComVariant(iSel));
					HPROPERTY hItem = m_wndList.AddItem(p);
					if (bSelect) m_wndList.SetCurSel(m_wndList.FindProperty(hItem));
					continue;
				}

				char szTmp[1024] = "";
				exif_entry_get_value(pEntry, szTmp, sizeof(szTmp));
				CA2T strTagVal(szTmp);
				HPROPERTY hItem = m_wndList.AddItem(PropCreateSimple(strTagName, strTagVal, (i<<16)|pEntry->tag));
				if (bSelect) m_wndList.SetCurSel(m_wndList.FindProperty(hItem));
			}
			if (!aExpanded[i])
			{
				m_wndList.CollapseItem(hCat);
			}
		}

		exif_data_unref(pData);

		m_wndList.SetTopIndex(iTopIndex);

		m_wndList.SetRedraw();
		m_bUpdating = false;
	}

private:
	bool m_bBorder;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IImageMetaData> m_pIMD;
	CComPtr<IDocumentImage> m_pImage;
	CComPtr<ISharedStateManager> m_pFrame;
	bool m_bUpdating;
	CComBSTR m_bstrSelectionID;

	CPropertyListCtrl m_wndList;
};

