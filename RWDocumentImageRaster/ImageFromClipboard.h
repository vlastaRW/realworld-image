// ImageFromClipboard.h : Declaration of the CImageFromClipboard

#pragma once
#include "DocumentRasterImage.h"

extern __declspec(selectany) wchar_t const DESWIZ_IMGCLIP_NAME[] = L"[0409]Image from clipboard[0405]Obrázek ze schránky";
extern __declspec(selectany) wchar_t const DESWIZ_IMGCLIP_DESC[] = L"[0409]Create new image document using current contents of clipboard.[0405]Vytvořit nový obrázkový dokument z aktuálního obsahu schránky.";


// CImageFromClipboard

class ATL_NO_VTABLE CImageFromClipboard : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CImageFromClipboard, &CLSID_ImageFromClipboard>,
	public CDesignerWizardImpl<DESWIZ_IMGCLIP_NAME, DESWIZ_IMGCLIP_DESC, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryRasterImage, 192>,
	public IDesignerWizardClipboard
{
public:
	CImageFromClipboard()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CImageFromClipboard)

BEGIN_CATEGORY_MAP(CImageFromClipboard)
	IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CImageFromClipboard)
	COM_INTERFACE_ENTRY(IDesignerWizard)
	COM_INTERFACE_ENTRY(IDesignerWizardClipboard)
END_COM_MAP()

	// IDesignerWizard methods
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDesignerWizardClipboard methods
public:
	STDMETHOD(CanActivate)();
};

OBJECT_ENTRY_AUTO(__uuidof(ImageFromClipboard), CImageFromClipboard)
