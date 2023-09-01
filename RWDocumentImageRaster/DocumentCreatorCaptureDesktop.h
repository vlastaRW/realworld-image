// DocumentCreatorCaptureDesktop.h : Declaration of the CDocumentCreatorCaptureDesktop

#pragma once
#include "RWDocumentImageRaster.h"

#include "DocumentRasterImage.h"

extern __declspec(selectany) wchar_t const DESWIZ_CAPDESK_NAME[] = L"[0409]Capture desktop[0405]Zachytit plochu";
extern __declspec(selectany) wchar_t const DESWIZ_CAPDESK_DESC[] = L"[0409]Create new raster image from the current content of Windows desktop.[0405]Vytvořit nový rastrový obrázek z aktuálního obsahu pracovní plochy.";


// CDocumentCreatorCaptureDesktop

class ATL_NO_VTABLE CDocumentCreatorCaptureDesktop :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCreatorCaptureDesktop, &CLSID_DocumentCreatorCaptureDesktop>,
	public CDesignerWizardImpl<DESWIZ_CAPDESK_NAME, DESWIZ_CAPDESK_DESC, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryRasterImage, 191>
{
public:
	CDocumentCreatorCaptureDesktop()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCreatorCaptureDesktop)

BEGIN_CATEGORY_MAP(CDocumentCreatorCaptureDesktop)
	IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCreatorCaptureDesktop)
	COM_INTERFACE_ENTRY(IDesignerWizard)
END_COM_MAP()

	// IDesignerWizard methods
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentCreatorCaptureDesktop), CDocumentCreatorCaptureDesktop)
