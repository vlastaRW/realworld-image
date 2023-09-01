// DocumentFactoryAnimation.h : Declaration of the CDocumentFactoryAnimation

#pragma once
#include "resource.h"       // main symbols
#include "RWDocumentAnimation.h"
#include <DesignerFrameIconsImpl.h>

#include "DocumentAnimation.h"
//#include "RWDocumentImageVector.h"
#include <RWThumbnailProvider.h>

extern __declspec(selectany) wchar_t const FILETYPE_ANI[] = L"[0409]Animated Image[0405]Animovaný obrázek";


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


// CDocumentFactoryAnimation

class ATL_NO_VTABLE CDocumentFactoryAnimation :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryAnimation, &CLSID_DocumentFactoryAnimation>,
	public CDocumentBuilderImpl<CDocumentFactoryAnimation, FEATURES(g_aSupported), FILETYPE_ANI, 0>,
	public IDocumentFactoryRasterImage,
	public IDocumentFactoryVectorImage,
	public IDocumentFactoryMetaData,
	public IDocumentFactoryLayeredImage,
	public IDocumentFactoryAnimation,
	public IDesignerFrameIcons,
	public IThumbnailProvider
{
public:
	CDocumentFactoryAnimation()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryAnimation)

BEGIN_CATEGORY_MAP(CDocumentFactoryAnimation)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
	IMPLEMENTED_CATEGORY(CATID_ThumbnailProvider)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryAnimation)
	COM_INTERFACE_ENTRY(IDocumentFactoryRasterImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryLayeredImage)
	COM_INTERFACE_ENTRY(IDocumentFactoryAnimation)
	COM_INTERFACE_ENTRY(IDocumentFactoryMetaData)
	COM_INTERFACE_ENTRY(IDocumentFactoryVectorImage)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
	COM_INTERFACE_ENTRY_IID(IID_IThumbnailProvider, IThumbnailProvider)
END_COM_MAP()


	// IDocumentBuilder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority);
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);

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
	STDMETHOD(AddLayer)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BYTE a_bTop, IImageLayerCreator* a_pCreator, BSTR a_bstrName, TImageLayer const* a_pProperties, IConfig* a_pOperation);
	STDMETHOD(SetResolution)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageResolution const* a_pResolution);
	STDMETHOD(SetSize)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const* a_pSize);
	STDMETHOD(RearrangeLayers)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nLayers, TImagePoint const* a_aOffsets);

	// IDocumentFactoryAnimation methods
public:
	STDMETHOD(Init)(BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(AppendFrame)(IAnimationFrameCreator* a_pCreator, float a_fDelay, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(SetLoopCount)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, ULONG a_nCount);
	//STDMETHOD(SetResolution)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageResolution const* a_pResolution); // shared

	// IDocumentFactoryMetaData methods
public:
	STDMETHOD(AddBlock)(BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData);

	// IDesignerFrameIcons methods
public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp)
	{
		if (a_pTimeStamp == NULL) return E_POINTER;
		*a_pTimeStamp = 0;
		return S_OK;
	}
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs)
	{
		try
		{
			CComPtr<IEnumGUIDsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
			pTmp->Insert(CLSID_DocumentFactoryAnimation);
			*a_ppIDs = pTmp;
			pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon);

	// IThumbnailProvider methods
public:
	STDMETHOD_(ULONG, IThumbnailProvider::Priority)() { return 50; }
	STDMETHOD(GetThumbnail)(IDocument* a_pDoc, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds, LCID a_tLocaleID, BSTR* a_pbstrInfo, HRESULT(fnRescaleImage)(ULONG a_nSrcSizeX, ULONG a_nSrcSizeY, DWORD const* a_pSrcData, bool a_bSrcAlpha, ULONG a_nSizeX, ULONG a_nSizeY, DWORD* a_pBGRAData, RECT* a_prcBounds));
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentFactoryAnimation), CDocumentFactoryAnimation)
