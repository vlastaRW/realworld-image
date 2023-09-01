// DocumentFactoryVectorImage.h : Declaration of the CDocumentFactoryVectorImage

#pragma once
#include "resource.h"       // main symbols
#include "DocumentVectorImage.h"

extern __declspec(selectany) wchar_t const g_pszFormatNameRVI[] = L"[0409]RealWorld vector image files[0405]Soubory vektorových obrázků RealWorldu";
extern __declspec(selectany) wchar_t const g_pszTypeNameRVI[] = L"[0409]RealWorld Vector Image[0405]Vektorový obrázek RealWorldu";
extern __declspec(selectany) wchar_t const g_pszShellIconPathRVI[] = L"%MODULE%,0";
extern __declspec(selectany) wchar_t const g_pszSupportedExtensionsRVI[] = L"rvi";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameRVI, g_pszTypeNameRVI, g_pszSupportedExtensionsRVI, 0, NULL/*IDI_VECTORIMAGE, g_pszShellIconPathRVI*/> CDocumentTypeCreatorRVI;

extern __declspec(selectany) wchar_t const g_pszNewVectorImageName[] = L"[0409]New vector image[0405]Nový vektorový obrázek";
extern __declspec(selectany) wchar_t const g_pszNewVectorImageDesc[] = L"[0409]Create a new empty vector image of given size.[0405]Vytvořit nový vektorový obrázek dané velikosti.";
extern __declspec(selectany) wchar_t const DESWIZ_IMAGE_CAT[] = L"[0409]Images[0405]Obrázky";

extern __declspec(selectany) wchar_t const TYPENAME_VECTOR[] = L"[0409]Vector Image[0405]Vektorový obrázek";
extern __declspec(selectany) wchar_t const FORMATNAME_VECTOR[] = L"[0409]Vector image files[0405]Soubory vektorových obrázků";


// CDocumentFactoryVectorImage

class ATL_NO_VTABLE CDocumentFactoryVectorImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryVectorImage, &CLSID_DocumentFactoryVectorImage>,
	public IDocumentFactoryVectorImage,
	public CDocumentBuilderImpl<CDocumentFactoryVectorImage, FEATURES(g_aSupportedVector), TYPENAME_VECTOR, IDI_VECTORIMAGE, FORMATNAME_VECTOR, g_pszShellIconPathRVI>,
	public CDocumentDecoderImpl<CDocumentFactoryVectorImage, CDocumentTypeCreatorRVI, IDocumentFactoryVectorImage, EDPAverage, IDocumentCodec>,
	public CDesignerWizardImpl<g_pszNewVectorImageName, g_pszNewVectorImageDesc, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryVectorImage>,
	public IScriptingInterface,
	public IImageLayerFactory
{
public:
	CDocumentFactoryVectorImage()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryVectorImage)

BEGIN_CATEGORY_MAP(CDocumentFactoryVectorImage)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	//IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryVectorImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryVectorImage)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
	COM_INTERFACE_ENTRY(IDesignerWizard)
	COM_INTERFACE_ENTRY(IScriptingInterface)
	COM_INTERFACE_ENTRY(IImageLayerFactory)
END_COM_MAP()


	// IDesignerWizard methods
public:
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryVectorImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDocumentBuilder methods
public:

	// IDocumentEncoder methods
public:
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg) { return E_NOTIMPL; }
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

	// IDocumentDecoder methods
public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryVectorImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl);

	// IDocumentFactoryVectorImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground);
	STDMETHOD(AddObject)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled);

	// IScriptingInterface methods
public:
	STDMETHOD(GetGlobalObjects)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);

	// IImageLayerFactory methods
public:
	STDMETHOD(LayerName)(BYTE a_bCreateNew, ILocalizedString** a_ppLayerType);
//extern __declspec(selectany) wchar_t const g_tCmdDescLayerAddVector[] = L"[0409]Shapes placed in a vector layer can be easily transformed and modified.[0405]Tvary umístěné ve vektorové vrstvě lze snadno transformovat a upravovat.";
	STDMETHOD(LayerIconID)(GUID* a_pIconID);
	STDMETHOD(LayerIcon)(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(LayerAccelerator)(TCmdAccel* a_pAccel);
	STDMETHOD(LayerCreatorGet)(TImageSize a_tSize, float const* a_aBackgroundRGBA, IImageLayerCreator** a_ppCreator);
	STDMETHOD(LayerDescription)(ILocalizedString** a_ppImageType);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryVectorImage), CDocumentFactoryVectorImage)
