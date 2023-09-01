// MenuCommandsFillStyle.cpp : Implementation of CMenuCommandsFillStyle

#include "stdafx.h"
#include "MenuCommandsFillStyle.h"
#include <SharedStringTable.h>
#include <RWBaseEnumUtils.h>
#include "SharedStateToolMode.h"


static const OLECHAR CFGID_FILLSTYLE_STATESYNC[] = L"FillStyleSyncGroup";
static const OLECHAR CFGID_FILLSTYLE_FILTER[] = L"ToolListFilter";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIFillStyleListDlg :
	public CCustomConfigWndImpl<CConfigGUIFillStyleListDlg>,
	public CDialogResize<CConfigGUIFillStyleListDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_DRAWINGTOOLLIST };

	BEGIN_MSG_MAP(CConfigGUIFillStyleListDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIFillStyleListDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIFillStyleListDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIFillStyleListDlg)
		DLGRESIZE_CONTROL(IDC_CGTL_SYNCGRP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGTL_FILTER, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIFillStyleListDlg)
		CONFIGITEM_EDITBOX(IDC_CGTL_SYNCGRP, CFGID_FILLSTYLE_STATESYNC)
		CONFIGITEM_EDITBOX(IDC_CGTL_FILTER, CFGID_FILLSTYLE_FILTER)
		//CONFIGITEM_COMBOBOX(IDC_CGTL_FILTER, CFGID_FILLSTYLE_FILTER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};


// CMenuCommandsFillStyle

STDMETHODIMP CMenuCommandsFillStyle::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_FILLSTYLE);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_FILLSTYLE_STATESYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_DESC), CConfigValue(L"RASTEREDITSTATE"), NULL, 0, NULL);

		CComBSTR cCFGID_FILLSTYLE_FILTER(CFGID_FILLSTYLE_FILTER);
		pCfg->ItemInsSimple(cCFGID_FILLSTYLE_FILTER, _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLLIST_FILTER_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_TOOLLIST_FILTER_DESC), CConfigValue(L"PATTERN LINEAR RADIAL CONICAL"), NULL, 0, NULL);
		//pCfg->ItemIns1ofN(cCFGID_FILLSTYLE_FILTER, _SharedStringTable.GetStringAuto(IDS_CFGID_FILLSTYLE_FILTER_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_FILLSTYLE_FILTER_DESC), CConfigValue(CFGVAL_TL_RAW), NULL);
		//pCfg->ItemOptionAdd(cCFGID_FILLSTYLE_FILTER, CConfigValue(CFGVAL_TL_RAW), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TL_RAW), 0, NULL);
		//pCfg->ItemOptionAdd(cCFGID_FILLSTYLE_FILTER, CConfigValue(CFGVAL_TL_HOTSPOT), _SharedStringTable.GetStringAuto(IDS_CFGVAL_TL_HOTSPOT), 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_MenuCommandsFillStyle, CConfigGUIFillStyleListDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

struct SToolEntry
{
	std::vector<CComBSTR> aOptions;
	size_t iActive;
	wchar_t cAccel;
};
typedef std::vector<SToolEntry> CToolEntries;

STDMETHODIMP CMenuCommandsFillStyle::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* UNREF(a_pView), IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FILLSTYLE_STATESYNC), &cSyncID);
		if (cSyncID.operator BSTR() == NULL || cSyncID.operator BSTR()[0] == L'\0')
			return S_FALSE;
		CConfigValue cFilter;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FILLSTYLE_FILTER), &cFilter);

		CToolEntries cEntries;

		bool bRasterImage = false;
		bool bOthers = false;
		bool bEnabled = true;
		//bool bSolidColor = false;
		CComPtr<ISharedStateToolMode> pMode;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pMode));
		if (pMode)
		{
			CComBSTR bstrToolID;
			CComBSTR bstrStyleID;
			BOOL bFill = TRUE;
			pMode->Get(&bstrToolID, &bFill, &bstrStyleID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			std::vector<ULONG> aPaints;
			M_ToolManager()->SupportedStates(bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
			for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
			{
				if ((*i&ETBTIdentifierMask) != ETBTIdentifierInterior)
					continue;
				//if ((*i&ETBTTypeMask) == ETBTSingleColor)
				//	bSolidColor = true;
				if ((*i&ETBTTypeMask) == ETBTRasterImage)
					bRasterImage = true;
			}
			bOthers = aPaints.size() > 1;
			bEnabled = bFill;
		}
		if (!bRasterImage)
			return S_FALSE;


		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuNoFillStyle>* p = NULL;
			CComObject<CDocumentMenuNoFillStyle>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOthers, !bEnabled);
			pItems->Insert(pTmp);
		}
		//CComPtr<IDocumentMenuCommand> pSep;
		//RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
		//pItems->Insert(pSep);

		// format:
		//   | - separator
		//   PENCIL - simple entry
		//   [*TEXT STYLIZE_TEXT BUBBLE_TEXT] (B) - complex entry with submenu and accelerator; * indicates active tool (not implemented)
		LPWSTR p1 = cFilter;
		while (true)
		{
			while (*p1 == L' ') ++p1;

			if (*p1 == L'[')
			{
				LPWSTR p2 = wcschr(p1, L']');
				if (p2 == NULL)
					break;
				// complex entry
				*p2 = L'\0';
				++p1;
				SToolEntry sEntry;
				sEntry.cAccel = L'\0';
				sEntry.iActive = 0;
				while (true)
				{
					while (*p1 == L' ') ++p1;
					LPWSTR p3 = wcschr(p1, L' ');
					if (p3)
						*p3 = L'\0';
					if (wcscmp(p1, L"|") == 0)
					{
						sEntry.aOptions.resize(sEntry.aOptions.size()+1);
					}
					else
					{
						if (*p1 == L'*')
						{
							++p1;
							sEntry.iActive = sEntry.aOptions.size();
						}
						sEntry.aOptions.push_back(p1);
					}
					if (p3)
					{
						p1 = p3+1;
					}
					else
					{
						break;
					}
				}
				if (!sEntry.aOptions.empty())
					cEntries.push_back(sEntry);
				p1 = p2+1;
				continue;
			}
			if (*p1 == L'(')
			{
				if (p1[1] == L'\0' || p1[2] != L')')
					break;
				if (!cEntries.empty())
					cEntries[cEntries.size()-1].cAccel = p1[1];
				p1 += 3;
				continue;
			}
			LPWSTR p2 = wcschr(p1, L' ');
			bool const bStop = p2 == NULL;
			if (p2)
				*p2 = L'\0';
			else
				p2 = p1 + wcslen(p1);
			SToolEntry sEntry;
			sEntry.cAccel = L'\0';
			sEntry.iActive = 0;
			if (wcscmp(p1, L"|") == 0)
			{
				sEntry.aOptions.resize(1);
				cEntries.push_back(sEntry);
			}
			else
			{
				if (p2-p1 > 3 && p2[-1] == L')' && p2[-3] == L'(')
				{
					sEntry.cAccel = p2[-2];
					p2[-3] = L'\0';
				}
				sEntry.aOptions.push_back(p1);
				cEntries.push_back(sEntry);
			}
			if (p2)
			{
				p1 = p2+1;
			}
			if (bStop)
				break;
		}

		for (CToolEntries::const_iterator i = cEntries.begin(); i != cEntries.end(); ++i)
		{
			if (i->aOptions.size() == 1 && i->aOptions[0].Length() == 0)
			{
				CComPtr<IDocumentMenuCommand> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
				pItems->Insert(pTmp);
			}
			else
			{
				ULONG iActive = i->iActive;
				if (i->aOptions.size() > 1 && pMode)
				{
					CAutoVectorPtr<BSTR> aIDs(new BSTR[i->aOptions.size()]);
					for (ULONG j = 0; j < i->aOptions.size(); ++j)
						aIDs[j] = i->aOptions[j];
					pMode->PickRecentFill(i->aOptions.size(), aIDs, &iActive);
				}
				CComObject<CDocumentMenuSwitchStyle>* p = NULL;
				CComObject<CDocumentMenuSwitchStyle>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(M_Manager(), a_pStates, cSyncID, i->aOptions, iActive, i->cAccel, !bOthers);
				pItems->Insert(pTmp);
			}
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

#include <SimpleLocalizedString.h>

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Name(ILocalizedString** a_ppText)
{
	if (m_cAccel)
	{
		*a_ppText = this;
		AddRef();
		return S_OK;
	}
	return m_pManager->StyleNameGet(m_aIDs[m_iActive], a_ppText);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::GetLocalized(LCID a_tLCID, BSTR* a_pbstrString)
{
	try
	{
		*a_pbstrString = NULL;
		CComPtr<ILocalizedString> pStr;
		m_pManager->StyleNameGet(m_aIDs[m_iActive], &pStr);
		CComBSTR bstr;
		if (pStr)
			pStr->GetLocalized(a_tLCID, &bstr);
		if (bstr.Length() == 0)
			bstr = m_aIDs[m_iActive];
		wchar_t sz[5] = L" (x)";
		sz[2] = m_cAccel;
		bstr += sz;
		*a_pbstrString = bstr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrString ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Description(ILocalizedString** a_ppText)
{
	return m_pManager->StyleDescGet(m_aIDs[m_iActive], a_ppText);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::IconID(GUID* a_pIconID)
{
	return m_pManager->StyleIconIDGet(m_aIDs[m_iActive], a_pIconID);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	return m_pManager->StyleIconGet(m_aIDs[m_iActive], a_nSize, a_phIcon);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	if (m_cAccel)
	{
		a_pAccel->fVirtFlags = 0;
		a_pAccel->wKeyCode = m_cAccel;
		return S_OK;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	if (m_aIDs.size() > 1)
	{
		try
		{
			*a_ppSubCommands = NULL;
			(*a_ppSubCommands = this)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Size(ULONG *a_pnSize)
{
	*a_pnSize = m_aIDs.size();
	return S_OK;
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Get(ULONG a_nIndex, REFIID a_iid, void** a_ppItem)
{
	try
	{
		*a_ppItem = NULL;
		if (a_nIndex >= m_aIDs.size())
			return E_RW_INDEXOUTOFRANGE;
		if (IsEqualIID(a_iid, IID_IUnknown) || IsEqualIID(a_iid, __uuidof(IDocumentMenuCommand)))
		{
			if (m_aIDs[a_nIndex].Length())
			{
				CComObject<CDocumentMenuSwitchSubStyle>* p = NULL;
				CComObject<CDocumentMenuSwitchSubStyle>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pManager, m_pStates, m_bstrSyncID, m_aIDs[a_nIndex], m_aIDs);
				*a_ppItem = pTmp.Detach();
			}
			else
			{
				CComPtr<IDocumentMenuCommand> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
				*a_ppItem = pTmp.Detach();
			}
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	catch (...)
	{
		return a_ppItem ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::GetMultiple(ULONG a_nIndexFirst, ULONG a_nCount, REFIID a_iid, void** a_apItems)
{
	try
	{
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			HRESULT hRes = Get(a_nIndexFirst+i, a_iid, a_apItems+i);
			if (FAILED(hRes))
			{
				for (ULONG j = 0; j < i; ++i)
				{
					reinterpret_cast<IUnknown*>(a_apItems[i])->Release();
					a_apItems[i] = NULL;
				}
				return hRes;
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return a_apItems ? E_UNEXPECTED : E_POINTER;
	}
}


STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSDisabled;
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComBSTR bstrStyleID;
		BOOL bFill = TRUE;
		if (pPrev)
			pPrev->Get(NULL, &bFill, &bstrStyleID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		else
			bstrStyleID = L"SOLID";
		*a_peState = static_cast<EMenuCommandState>((m_aIDs.size() <= 1 ? EMCSRadio : EMCSRadio|EMCSExecuteSubMenu)|(bstrStyleID && bstrStyleID == m_aIDs[m_iActive] && (m_bForceEnabled || bFill) ? EMCSChecked : EMCSNormal));
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchStyle::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		CComBSTR bstrFillID;
		BOOL bFill = TRUE;
		if (pPrev)
			pPrev->Get(NULL, NULL, &bstrFillID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		BSTR bstrNew = bstrFillID == m_aIDs[m_iActive] ? m_aIDs[(m_iActive+1)%m_aIDs.size()] : m_aIDs[m_iActive];
		if (S_OK == pNew->Set(NULL, &bFill, bstrNew, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
		{
			for (std::vector<CComBSTR>::const_iterator i = m_aIDs.begin(); i != m_aIDs.end(); ++i)
				if (*i != bstrNew)
					pNew->RemoveRecentFill(*i);
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::Name(ILocalizedString** a_ppText)
{
	return m_pManager->StyleNameGet(m_bstrStyleID, a_ppText);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::Description(ILocalizedString** a_ppText)
{
	return m_pManager->StyleDescGet(m_bstrStyleID, a_ppText);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::IconID(GUID* a_pIconID)
{
	return m_pManager->StyleIconIDGet(m_bstrStyleID, a_pIconID);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	return m_pManager->StyleIconGet(m_bstrStyleID, a_nSize, a_phIcon);
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::Accelerator(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	return E_NOTIMPL;
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSDisabled;
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		if (pPrev == NULL || m_aIDs.size() < 2)
		{
			*a_peState = EMCSRadio;
			return S_OK;
		}
		CAutoVectorPtr<BSTR> aIDs(new BSTR[m_aIDs.size()]);
		std::copy(m_aIDs.begin(), m_aIDs.end(), aIDs.m_p);
		ULONG nIndex = m_aIDs.size();
		pPrev->PickRecentFill(m_aIDs.size(), aIDs, &nIndex);
		*a_peState = nIndex < m_aIDs.size() && m_aIDs[nIndex] == m_bstrStyleID ? EMCSRadioChecked : EMCSRadio;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuSwitchSubStyle::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		BOOL bFill = TRUE;
		if (S_OK == pNew->Set(NULL, &bFill, m_bstrStyleID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
		{
			for (std::vector<CComBSTR>::const_iterator i = m_aIDs.begin(); i != m_aIDs.end(); ++i)
				if (*i != m_bstrStyleID)
					pNew->RemoveRecentFill(*i);
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsFillStyle::CDocumentMenuNoFillStyle::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		BOOL bOutline = TRUE;
		BOOL bFill = FALSE;
		if (S_OK == pNew->Set(NULL, &bFill, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
