// GlobalConfigDefaultImageFormat.cpp : Implementation of CGlobalConfigDefaultImageFormat

#include "stdafx.h"
#include "GlobalConfigDefaultImageFormat.h"
#include <MultiLanguageString.h>

// CGlobalConfigDefaultImageFormat

STDMETHODIMP CGlobalConfigDefaultImageFormat::Interactive(BYTE* a_pPriority)
{
	if (a_pPriority)
		*a_pPriority = 200;
	return S_OK;
}

STDMETHODIMP CGlobalConfigDefaultImageFormat::Name(ILocalizedString** a_ppName)
{
	if (a_ppName == nullptr)
		return E_POINTER;
	try
	{
		*a_ppName = new CMultiLanguageString(L"[0409]Image[0405]Obrázek");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CGlobalConfigDefaultImageFormat::Description(ILocalizedString** a_ppDesc)
{
	if (a_ppDesc == nullptr)
		return E_POINTER;
	try
	{
		*a_ppDesc = new CMultiLanguageString(L"[0409]Choose, how to open images in standard formats. Window layout for the selected document type must be installed.[0405]Rozhodněte, jak otevírat obrázky ve standardních formátech. Pro zvolený typ dokumentu musí existovat layout okna.");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIDefaultImageFormatDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIDefaultImageFormatDlg>,
	public CDialogResize<CConfigGUIDefaultImageFormatDlg>
{
public:
	BEGIN_DIALOG_EX(0, 0, 219, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_CGRI_BUILDER = 100,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Open images as:[0405]Otevírat obrázky jako:"), IDC_STATIC, 0, 2, 77, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGRI_BUILDER, 79, 0, 139, 85, WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT | WS_VSCROLL | WS_TABSTOP, 0);
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIDefaultImageFormatDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIDefaultImageFormatDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIDefaultImageFormatDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIDefaultImageFormatDlg)
		DLGRESIZE_CONTROL(IDC_CGRI_BUILDER, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIDefaultImageFormatDlg)
		CONFIGITEM_COMBOBOX(IDC_CGRI_BUILDER, CFGID_DIF_BUILDER)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

class ATL_NO_VTABLE CStdImageBuilderOptions :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IConfigItemCustomOptions
{
public:
	BEGIN_COM_MAP(CStdImageBuilderOptions)
		COM_INTERFACE_ENTRY(IConfigItemCustomOptions)
		COM_INTERFACE_ENTRY(IEnumConfigItemOptions)
	END_COM_MAP()

	// IEnumConfigItemOptions methods
public:
	STDMETHOD(Size)(ULONG* a_pnSize)
	{
		try
		{
			*a_pnSize = 0;
			ObjectLock cLock(this);
			CComPtr<IPlugInCache> pPIC;
			RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
			m_pGUIDs = NULL;
			CComPtr<IEnumUnknowns> pBuilders;
			pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentFactoryRasterImage), 0, &pBuilders, &m_pGUIDs);
			if (m_pGUIDs)
			{
				ULONG nSize = 0;
				m_pGUIDs->Size(&nSize);
				*a_pnSize = nSize;
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pnSize ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Get)(ULONG a_nIndex, TConfigValue* a_ptItem)
	{
		try
		{
			a_ptItem->eTypeID = ECVTGUID;
			ObjectLock cLock(this);
			if (m_pGUIDs && SUCCEEDED(m_pGUIDs->Get(a_nIndex, &a_ptItem->guidVal)))
				return S_OK;
			return E_FAIL;
		}
		catch (...)
		{
			return a_ptItem ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetMultiple)(ULONG a_nIndexFirst, ULONG a_nCount, TConfigValue* a_atItems)
	{
		try
		{
			ObjectLock cLock(this);
			for (ULONG i = 0; i < a_nCount; ++i)
			{
				HRESULT hRes = Get(a_nIndexFirst+i, a_atItems+i);
				if (FAILED(hRes)) return hRes;
			}
			return S_OK;
		}
		catch (...)
		{
			return a_atItems ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IConfigItemCustomOptions methods
public:
	STDMETHOD(GetValueName)(TConfigValue const* a_pValue, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			if (a_pValue->eTypeID != ECVTGUID)
				return E_FAIL;
			CComPtr<IDocumentBuilder> pDB;
			RWCoCreateInstance(pDB, a_pValue->guidVal);
			return pDB ? pDB->TypeName(a_ppName) : E_FAIL;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CComPtr<IEnumGUIDs> m_pGUIDs;
};


STDMETHODIMP CGlobalConfigDefaultImageFormat::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		CComObject<CStdImageBuilderOptions>* p = NULL;
		CComObject<CStdImageBuilderOptions>::CreateInstance(&p);
		CComPtr<IConfigItemCustomOptions> pDocTypesOpetions = p;
		pCfg->ItemIns1ofNWithCustomOptions(CComBSTR(CFGID_DIF_BUILDER), CMultiLanguageString::GetAuto(L"[0409]Open image as[0405]Otevřít obrázek jako"), CMultiLanguageString::GetAuto(L"[0409]Controls how are images in standard formats like .jpg or .png opened.[0405]Určuje, jako co se budou obrázky ve standardních formátech jako .jpg nebo .png otevírat."), CConfigValue(__uuidof(DocumentFactoryLayeredImage)), pDocTypesOpetions, NULL, 0, NULL);
		CConfigCustomGUI<&CLSID_GlobalConfigDefaultImageFormat, CConfigGUIDefaultImageFormatDlg>::FinalizeConfig(pCfg);
		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

