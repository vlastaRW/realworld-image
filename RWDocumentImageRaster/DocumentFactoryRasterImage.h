// DocumentFactoryRasterImage.h : Declaration of the CDocumentFactoryRasterImage

#pragma once
#include "RWDocumentImageRaster.h"
#include "DocumentRasterImage.h"
#include <WeakSingleton.h>

extern __declspec(selectany) wchar_t const DESWIZ_NEWRASIMG_NAME[] = L"[0409]New raster image[0405]Nový rastrový obrázek";
extern __declspec(selectany) wchar_t const DESWIZ_NEWRASIMG_DESC[] = L"[0409]Create a new empty raster image of given size.[0405]Vytvořit nový rastrový obrázek dané velikosti.";
extern __declspec(selectany) wchar_t const TYPENAME_RASTER[] = L"[0409]Raster Image[0405]Rastrový obrázek";
extern __declspec(selectany) wchar_t const FORMATNAME_RASTER[] = L"[0409]Image files[0405]Soubory obrázků";


// CDocumentFactoryRasterImage

class ATL_NO_VTABLE CDocumentFactoryRasterImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryRasterImage, &CLSID_DocumentFactoryRasterImage>,
	public IDocumentFactoryRasterImage,
	public IDocumentFactoryRasterImageCallback,
	public IDocumentFactoryMetaData,
	public CDocumentBuilderImpl<CDocumentFactoryRasterImage, FEATURES(g_aSupportedRaster), TYPENAME_RASTER, 0, FORMATNAME_RASTER, g_pszShellIconPathRaster>,
	public CDesignerWizardImpl<DESWIZ_NEWRASIMG_NAME, DESWIZ_NEWRASIMG_DESC, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryRasterImage>,
	public IImageLayerFactory
{
public:
	CDocumentFactoryRasterImage()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CDocumentFactoryRasterImage)

BEGIN_CATEGORY_MAP(CDocumentFactoryRasterImage)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	//IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDesignerWizard)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryMetaData)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImageCallback)
	COM_INTERFACE_ENTRY(IImageLayerFactory)
END_COM_MAP()


	// IDocumentBuilder methods
public:
	STDMETHOD(IDocumentBuilder::Priority)(ULONG* a_pnPriority)
	{
		try
		{
			*a_pnPriority = EDPAverage-1;
			CComPtr<IGlobalConfigManager> pMgr;
			RWCoCreateInstance(pMgr, __uuidof(GlobalConfigManager));
			if (pMgr == NULL) return S_OK;
			CComPtr<IConfig> pGlbCfg;
			pMgr->Config(CLSID_GlobalConfigDefaultImageFormat, &pGlbCfg);
			if (pGlbCfg == NULL) return S_OK;
			CConfigValue cVal;
			pGlbCfg->ItemValueGet(CComBSTR(CFGID_DIF_BUILDER), &cVal);
			if (cVal.TypeGet() == ECVTGUID && IsEqualGUID(cVal, __uuidof(DocumentFactoryRasterImage)))
				*a_pnPriority = EDPAverage;
			return S_OK;
		}
		catch (...)
		{
			return a_pnPriority == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IDesignerWizard methods
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDocumentFactoryRasterImage methods
public:
	static HRESULT CreateData(CComPtr<IDocumentData>& a_pDocData, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile);
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile);

	// IDocumentFactoryRasterImageCallback methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, TRasterImageRect const* a_pRect, IRasterImageCallback* a_pProducer);

	// IDocumentFactoryMetaData methods
public:
	STDMETHOD(AddBlock)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData);

	// IImageLayerFactory methods
public:
	STDMETHOD(LayerName)(BYTE a_bCreateNew, ILocalizedString** a_ppLayerType);
	STDMETHOD(LayerIconID)(GUID* a_pIconID);
	STDMETHOD(LayerIcon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(LayerAccelerator)(TCmdAccel* a_pAccel);
	STDMETHOD(LayerCreatorGet)(TImageSize a_tSize, float const* a_aBackgroundRGBA, IImageLayerCreator** a_ppCreator);
	STDMETHOD(LayerDescription)(ILocalizedString** a_ppImageType);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryRasterImage), CDocumentFactoryRasterImage)
