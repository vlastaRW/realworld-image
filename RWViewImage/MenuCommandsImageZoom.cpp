// MenuCommandsImageZoom.cpp : Implementation of CMenuCommandsImageZoom

#include "stdafx.h"
#include "MenuCommandsImageZoom.h"
#include <SharedStringTable.h>
#include <SimpleLocalizedString.h>


// CMenuCommandsImageZoom

STDMETHODIMP CMenuCommandsImageZoom::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENU_IMAGEZOOM);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		//CComPtr<IConfigWithDependencies> pCfg;
		//RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		//pCfg->Finalize(NULL);
		//*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates), IDesignerView* a_pView, IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		{
			CComObject<CDocumentMenuZoomOut>* p = NULL;
			CComObject<CDocumentMenuZoomOut>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuZoomPopup>* p = NULL;
			CComObject<CDocumentMenuZoomPopup>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuZoomIn>* p = NULL;
			CComObject<CDocumentMenuZoomIn>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuAutoZoom>* p = NULL;
			CComObject<CDocumentMenuAutoZoom>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pView);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

EMenuCommandState CMenuCommandsImageZoom::CDocumentMenuZoomIn::IntState()
{
	CComPtr<IEnumUnknownsInit> p;
	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
	CComPtr<IImageZoomControl> pIZ;
	p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
	float fZoom = 1.0f;
	if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
		return EMCSDisabled;
	fZoom *= 1.41421356f;
	return pIZ->CanSetZoom(&fZoom) == S_OK ? EMCSNormal : EMCSDisabled;
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuZoomIn::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		float fZoom = 1.0f;
		if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
			return E_FAIL;
		return pIZ->SetZoom(fZoom*1.41421356f);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

EMenuCommandState CMenuCommandsImageZoom::CDocumentMenuZoomOut::IntState()
{
	CComPtr<IEnumUnknownsInit> p;
	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
	CComPtr<IImageZoomControl> pIZ;
	p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
	float fZoom = 1.0f;
	if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
		return EMCSDisabled;
	fZoom *= 0.7071067812f;
	return pIZ->CanSetZoom(&fZoom) == S_OK ? EMCSNormal : EMCSDisabled;
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuZoomOut::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		float fZoom = 1.0f;
		if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
			return E_FAIL;
		return pIZ->SetZoom(fZoom*0.7071067812f);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

EMenuCommandState CMenuCommandsImageZoom::CDocumentMenuAutoZoom::IntState()
{
	CComPtr<IEnumUnknownsInit> p;
	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
	CComPtr<IImageZoomControl> pIZ;
	p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
	return pIZ && pIZ->SupportsAutoZoom() == S_OK ? (pIZ->IsAutoZoomEnabled() == S_OK ? EMCSChecked : EMCSNormal) : EMCSDisabled;
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuAutoZoom::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		return pIZ ? pIZ->EnableAutoZoom(pIZ->IsAutoZoomEnabled() != S_OK) : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuZoomPopup::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		float fZoom = 1.0f;
		if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
			return E_FAIL;
		wchar_t szTmp[16];
		if (fZoom >= 1.0f)
			swprintf(szTmp, L"%i%%", static_cast<int>(fZoom*100.0f+0.5f));
		else if (fZoom >= 0.1f)
			swprintf(szTmp, L"%g%%", 0.1f*static_cast<int>(fZoom*1000.0f+0.5f));
		else
			swprintf(szTmp, L"%g%%", 0.01f*static_cast<int>(fZoom*10000.0f+0.5f));
		*a_ppText = new CSimpleLocalizedString(SysAllocString(szTmp));
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuZoomPopup::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(IDS_MENU_ZOOMPOPUP_DESC);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuZoomPopup::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		static float aConstZooms[] = {0.0625f, 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 6.0f, 8.0f, 12.0f, 16.0f, 24.0f, 32.0f};
		for (float* pZoom = aConstZooms; pZoom != aConstZooms+itemsof(aConstZooms); ++pZoom)
		{
			CComObject<CDocumentMenuConstantZoom>* p = NULL;
			CComObject<CDocumentMenuConstantZoom>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pCmd = p;
			p->Init(m_pView, *pZoom);
			pItems->Insert(pCmd);
		}
		{
			CComPtr<IDocumentMenuCommand> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuCustomZoom>* p = NULL;
			CComObject<CDocumentMenuCustomZoom>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(m_pView);
			pItems->Insert(pTmp);
		}
		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuConstantZoom::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		wchar_t szTmp[16];
		swprintf(szTmp, L"%g%%", m_fZoom*100.0f);
		*a_ppText = new CSimpleLocalizedString(SysAllocString(szTmp));
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

EMenuCommandState CMenuCommandsImageZoom::CDocumentMenuConstantZoom::IntState()
{
	CComPtr<IEnumUnknownsInit> p;
	RWCoCreateInstance(p, __uuidof(EnumUnknowns));
	m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
	CComPtr<IImageZoomControl> pIZ;
	p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
	float fZoom = 1.0f;
	if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
		return EMCSDisabled;
	float fMyZoom = m_fZoom;
	return pIZ->CanSetZoom(&fMyZoom) == S_OK && fMyZoom == m_fZoom ? (fZoom == m_fZoom ? EMCSRadioChecked : EMCSNormal) : EMCSDisabled;
}

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuConstantZoom::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		return pIZ ? pIZ->SetZoom(m_fZoom) : E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <Win32LangEx.h>

class CCustomZoomDlg : public Win32LangEx::CLangDialogImpl<CCustomZoomDlg>
{
public:
	CCustomZoomDlg(LCID a_tLocaleID, float* a_pZoom) : m_pZoom(a_pZoom),
		Win32LangEx::CLangDialogImpl<CCustomZoomDlg>(a_tLocaleID)
	{
	}

	enum { IDD = IDD_CUSTOMZOOM };

	BEGIN_MSG_MAP(CCustomZoomDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

protected:
	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		CenterWindow(GetParent());
		m_wndZoom = GetDlgItem(IDC_ZOOM);
		TCHAR szTmp[32];
		_stprintf(szTmp, _T("%g"), 100.0f**m_pZoom);
		m_wndZoom.SetWindowText(szTmp);
		return TRUE;
	}
	LRESULT OnCancel(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		EndDialog(a_wID);
		return 0;
	}
	LRESULT OnOK(WORD a_wNotifyCode, WORD a_wID, HWND a_hWndCtl, BOOL& a_bHandled)
	{
		TCHAR szTmp[32];
		m_wndZoom.GetWindowText(szTmp, 31);
		szTmp[31] = _T('\0');
		TCHAR* pEnd;
		double d = _tcstod(szTmp, &pEnd);
		if (d > 0.0)
		{
			*m_pZoom = d/100.0;
			EndDialog(a_wID);
		}
		else
		{
			MessageBeep(MB_ICONHAND);
			m_wndZoom.SetFocus();
		}
		return 0;
	}

private:
	CEdit m_wndZoom;
	float* m_pZoom;
};

STDMETHODIMP CMenuCommandsImageZoom::CDocumentMenuCustomZoom::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IEnumUnknownsInit> p;
		RWCoCreateInstance(p, __uuidof(EnumUnknowns));
		m_pView->QueryInterfaces(__uuidof(IImageZoomControl), EQIFVisible, p);
		CComPtr<IImageZoomControl> pIZ;
		p->Get(0, __uuidof(IImageZoomControl), reinterpret_cast<void**>(&pIZ));
		float fZoom = 1.0f;
		if (pIZ == NULL || FAILED(pIZ->GetZoom(&fZoom)))
			return E_FAIL;

		CCustomZoomDlg cDlg(a_tLocaleID, &fZoom);
		if (cDlg.DoModal(a_hParent) != IDOK)
			return S_FALSE;

		return pIZ->SetZoom(fZoom);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

