// DocumentFactoryLayeredImage.h : Declaration of the CDocumentFactoryLayeredImage

#pragma once
#include "RWDocumentImageRaster.h"

#include "DocumentLayeredImage.h"
#include <RWThumbnailProvider.h>


extern __declspec(selectany) wchar_t const DESWIZ_NEWIMG_NAME[] = L"[0409]New image[0405]Nový obrázek";
extern __declspec(selectany) wchar_t const DESWIZ_NEWIMG_DESC[] = L"[0409]Create a new empty raster image of given size.[0405]Vytvořit nový rastrový obrázek dané velikosti.";
extern __declspec(selectany) wchar_t const DESWIZ_IMAGE_CAT[];// = L"[0409]Images[0405]Obrázky";


// copied from vector image and drawing
// TODO: redesign image creation
enum EOutlineJoinType
{
	EOJTMiter = 0,
	EOJTRound = 2,
	EOJTBevel = 3,
};
enum ERasterizationMode
{
	ERMBinary = 0x1,
	ERMSmooth = 0x2
};
enum ECoordinatesMode
{
	ECMFloatingPoint = 0x1,
	ECMIntegral = 0x2,
	ECMTile2 = 0x4,
	ECMTile4 = 0x8,
	ECMTile8 = 0x10,
};
MIDL_INTERFACE("0DBF0A73-0BFE-4650-AB9C-ABF4D8428184")
IDocumentFactoryVectorImage : public IUnknown
{
public:
	virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Create(
		/* [in] */ BSTR a_bstrPrefix,
		/* [in] */ IDocumentBase * a_pBase,
		/* [in] */ const TImageSize * a_pSize,
		/* [in] */ const TImageResolution * a_pResolution,
		/* [size_is][in] */ const float* a_aBackground) = 0;

	virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddObject(
		/* [in] */ BSTR a_bstrPrefix,
		/* [in] */ IDocumentBase* a_pBase,
		/* [in] */ BSTR a_bstrName,
		/* [in] */ BSTR a_bstrToolID,
		/* [in] */ BSTR a_bstrParams,
		/* [in] */ BSTR a_bstrStyleID,
		/* [in] */ BSTR a_bstrStyleParams,
		/* [in] */ const float* a_pOutlineWidth,
		/* [in] */ const float* a_pOutlinePos,
		/* [in] */ const EOutlineJoinType* a_pOutlineJoins,
		/* [in] */ const TColor* a_pOutlineColor,
		/* [in] */ const ERasterizationMode* a_pRasterizationMode,
		/* [in] */ const ECoordinatesMode* a_pCoordinatesMode,
		/* [in] */ const boolean* a_pEnabled) = 0;
};
class DECLSPEC_UUID("51C87837-B028-4252-A3B3-940F80181770") DocumentFactoryVectorImage;

// CDocumentFactoryLayeredImage

class ATL_NO_VTABLE CDocumentFactoryLayeredImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryLayeredImage, &CLSID_DocumentFactoryLayeredImage>,
	public IDocumentFactoryRasterImage,
	public IDocumentFactoryVectorImage,
	public IDocumentFactoryMetaData,
	public IDocumentFactoryLayeredImage,
	public CDesignerWizardImpl<DESWIZ_NEWIMG_NAME, DESWIZ_NEWIMG_DESC, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryLayeredImage>,
	public IScriptingInterface,
	public IDocumentBuilder,
	public IThumbnailProvider
{
public:
	CDocumentFactoryLayeredImage()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryLayeredImage)

BEGIN_CATEGORY_MAP(CDocumentFactoryLayeredImage)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
	IMPLEMENTED_CATEGORY(CATID_ThumbnailProvider)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryLayeredImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryLayeredImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryMetaData)
	COM_INTERFACE_ENTRY(IDocumentFactoryVectorImage)
	COM_INTERFACE_ENTRY(IDesignerWizard)
	COM_INTERFACE_ENTRY(IScriptingInterface)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY_IID(IID_IThumbnailProvider, IThumbnailProvider)
END_COM_MAP()


	// IDocumentBuilder methods
public:
	HRESULT DBPriority(ULONG* a_pnPriority);
	STDMETHOD(IDocumentBuilder::Priority)(ULONG* a_pnPriority) { return DBPriority(a_pnPriority); }
	STDMETHOD(TypeName)(ILocalizedString** a_ppType);
	HRESULT DBIcon(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(IDocumentBuilder::Icon)(ULONG a_nSize, HICON* a_phIcon) { return DBIcon(a_nSize, a_phIcon); }
	STDMETHOD(FormatInfo)(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon);
	STDMETHOD(HasFeatures)(ULONG a_nCount, IID const* a_aiidRequired);

	// IDocumentFactoryRasterImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, ULONG a_nChannels, TChannelDefault const* a_aChannelDefaults, float a_fGamma, TImageTile const* a_pTile);

	// IDocumentFactoryVectorImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize, TImageResolution const* a_pResolution, float const* a_aBackground);
	STDMETHOD(AddObject)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled);

	// IDocumentFactoryLayeredImage methods
public:
	STDMETHOD(Create)(BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(AddLayer)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BYTE a_bBot, IImageLayerCreator* a_pCreator, BSTR a_bstrName, TImageLayer const* a_pProperties, IConfig* a_pOperation);
	STDMETHOD(SetResolution)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageResolution const* a_pResolution);
	STDMETHOD(SetSize)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize);
	STDMETHOD(RearrangeLayers)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nLayers, TImagePoint const* a_aOffsets);

	// IDesignerWizard methods
public:
	HRESULT DWIcon(ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(IDesignerWizard::Icon)(ULONG a_nSize, HICON* a_phIcon) { return DWIcon(a_nSize, a_phIcon); }
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);

	// IDocumentFactoryMetaData methods
public:
	STDMETHOD(AddBlock)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData);

	// IScriptingInterface methods
public:
	STDMETHOD(GetGlobalObjects)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);

	// IThumbnailProvider methods
public:
	STDMETHOD_(ULONG, IThumbnailProvider::Priority)() { return 80; }
	STDMETHOD(GetThumbnail)(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds));
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryLayeredImage), CDocumentFactoryLayeredImage)
