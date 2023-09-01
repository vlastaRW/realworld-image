// RasterImageEditToolsPhoto.h : Declaration of the CRasterImageEditToolsPhoto

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImagePhoto.h"
#include <RasterImageEditToolsFactoryImpl.h>
#include <DesignerFrameIconsImpl.h>

#include "EditToolShapeShift.h"
#include "EditToolRedEye.h"
#include "EditToolCrop.h"


extern __declspec(selectany) SToolSpec const g_aPhotoTools[] =
{
	{
		L"REDEYE", L"[0409]Red eye remover[0405]Odstranit červené oči", L"[0409]Define area for red eye removing by clicking and dragging with mouse.[0405]Oblast pro odstranění červených očí nastavte kliknutím a tažením myší.",
		{0x426605fe, 0x8c3b, 0x49e0, {0x81, 0x26, 0x26, 0x91, 0xe3, 0x76, 0x17, 0xa}}, 0,
		0, 0, ECMFloatingPoint, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolRedEye>, NULL
	},
	{
		L"CROP", L"[0409]Crop[0405]Oříznout", L"[0409]Cut a defined quadrangle from an image.[0405]Vyříznout z obrázku definovaný čtyřúhelník.",
		{0x6e26d2eb, 0xc85d, 0x4a4b, {0xbb, 0x75, 0xda, 0x2a, 0x62, 0x73, 0x9f, 0xaa}}, 0,
		0, 0, 0, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolCrop>, &CreateToolWindow<CEditToolCropDlg>
	},
	{
		L"SHAPESHIFT", L"[0409]Shapeshifter[0405]Změna tvaru", L"[0409]Modify shapes in picture by pushing, collapsing or expanding specified areas.[0405]Pozměnit tvary v obrázku pomocí posunů, smršťování nebo roztahování jednotlivých oblastí.",
		{0xd2e89c7d, 0xe663, 0x4f73, {0x95, 0xd1, 0x79, 0x66, 0xae, 0x6f, 0xd7, 0x84}}, 0,
		0, ERMBinary|ERMSmooth, 0, ETPSNoPaint, TRUE,
		&CreateTool<CEditToolShapeShift>, &CreateToolWindow<CEditToolShapeShiftDlg>
	},
};

extern __declspec(selectany) UINT s_aPhotoIconResIDs[] =
{
	0,//IDI_EDITTOOL_REDEYE,
	0,//IDI_EDITTOOL_CROP,
	0,//IDI_EDITTOOL_CLONE,
	0,//IDI_EDITTOOL_BUBBLE,
	IDI_FILLSTYLE_WOOD,
	IDI_FILLSTYLE_CLOUDS,
	0,//IDI_WATERMARK,
	0,//IDI_LEVELS,
	0,//IDI_EDITTOOL_SHAPESHIFT,
	0,//IDI_IMAGE_GLOW,
	0,//IDI_VIGNETTING,
	0,//IDI_IMAGE_DENOISE,
	0,//IDI_CURVES,
	0,
	0,
};
extern __declspec(selectany) GUID s_aPhotoIconUIDs[] =
{
	{0x426605fe, 0x8c3b, 0x49e0, {0x81, 0x26, 0x26, 0x91, 0xe3, 0x76, 0x17, 0x0a}},
	{0x6e26d2eb, 0xc85d, 0x4a4b, {0xbb, 0x75, 0xda, 0x2a, 0x62, 0x73, 0x9f, 0xaa}},
	{0x91999163, 0xa4c4, 0x4e4d, {0x8c, 0xff, 0xab, 0xff, 0x82, 0x8c, 0x28, 0xe4}},
	{0xcd50417d, 0x1771, 0x426b, {0x80, 0xaf, 0x4a, 0x0a, 0x78, 0x60, 0xfe, 0x3d}},
	{0xd7d68157, 0xda1e, 0x4a53, {0x84, 0x17, 0xf7, 0xad, 0x84, 0x57, 0x80, 0xd7}},
	{0x29bddec3, 0xc802, 0x48de, {0xb7, 0x24, 0x2d, 0xa7, 0x68, 0xd3, 0x39, 0x69}},
	CLSID_DocumentOperationWatermark,//{0x75921ba0, 0xa27e, 0x4bc5, {0x97, 0x0e, 0x6f, 0xd5, 0xab, 0x08, 0xea, 0x42}},
	CLSID_DocumentOperationRasterImageLevels,//{0x539cacf5, 0x722f, 0x47e1, {0xbd, 0xd9, 0xfa, 0x8d, 0x23, 0x09, 0x65, 0xbe}},
	{0xd2e89c7d, 0xe663, 0x4f73, {0x95, 0xd1, 0x79, 0x66, 0xae, 0x6f, 0xd7, 0x84}},
	CLSID_DocumentOperationRasterImageGlow,
	CLSID_DocumentOperationRasterImageVignetting,
	CLSID_DocumentOperationRemoveNoise,
	{0x2107e6ea, 0xfa97, 0x42d5, {0xaa, 0x2a, 0xce, 0xe2, 0x13, 0x54, 0x6, 0x21}}, // CLSID_DocumentOperationRasterImageCurves
	CLSID_DocumentOperationRasterImageGreyscale,
	CLSID_DocumentOperationRasterImageUnsharpMask,
};

#include "DocumentOperationWatermark.h"
#include "DocumentOperationRasterImageGlow.h"
#include "DocumentOperationRasterImageGreyscale.h"
#include "DocumentOperationRasterImageLevels.h"
#include "DocumentOperationRasterImageUnsharpMask.h"
#include "DocumentOperationRemoveNoise.h"
#include "DocumentOperationRasterImageVignetting.h"
HICON GetToolIconREDEYE(ULONG a_nSize);
HICON GetToolIconCROP(ULONG a_nSize);
HICON GetToolIconCLONE(ULONG a_nSize);
HICON GetToolIconBUBBLE(ULONG a_nSize);
HICON GetToolIconSHAPESHIFT(ULONG a_nSize);
HICON GetIconCurves(ULONG a_nSize);


extern __declspec(selectany) pfnGetDFIcon s_aPhotoGetIcons[] =
{
	GetToolIconREDEYE,//IDI_EDITTOOL_REDEYE,
	GetToolIconCROP,//IDI_EDITTOOL_CROP,
	GetToolIconCLONE,//IDI_EDITTOOL_CLONE,
	GetToolIconBUBBLE,//IDI_EDITTOOL_BUBBLE,
	NULL,//IDI_FILLSTYLE_WOOD,
	NULL,//IDI_FILLSTYLE_CLOUDS,
	CDocumentOperationWatermark::GetDefaultIcon,//IDI_WATERMARK,
	CDocumentOperationRasterImageLevels::GetDefaultIcon,//IDI_LEVELS,
	GetToolIconSHAPESHIFT,//IDI_EDITTOOL_SHAPESHIFT,
	CDocumentOperationRasterImageGlow::GetDefaultIcon,//IDI_IMAGE_GLOW,
	CDocumentOperationRasterImageVignetting::GetDefaultIcon,
	CDocumentOperationRemoveNoise::GetDefaultIcon,
	GetIconCurves,
	CDocumentOperationRasterImageGreyscale::GetDefaultIcon,
	CDocumentOperationRasterImageUnsharpMask::GetDefaultIcon,
};


// CRasterImageEditToolsPhoto

class ATL_NO_VTABLE CRasterImageEditToolsPhoto :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsPhoto, &CLSID_RasterImageEditToolsPhoto>,
	public CRasterImageEditToolsFactoryImpl<g_aPhotoTools, sizeof(g_aPhotoTools)/sizeof(g_aPhotoTools[0])>,
	public CDesignerFrameIconsImpl<sizeof(s_aPhotoIconUIDs)/sizeof(s_aPhotoIconUIDs[0]), s_aPhotoIconUIDs, s_aPhotoIconResIDs, s_aPhotoGetIcons>
{
public:
	typedef CRasterImageEditToolsFactoryImpl<g_aPhotoTools, sizeof(g_aPhotoTools)/sizeof(g_aPhotoTools[0])> CImpl;

	CRasterImageEditToolsPhoto()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsPhoto)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsPhoto)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsPhoto)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IRasterImageEditToolsFactory methods
public:
	STDMETHOD(ToolIDsEnum)(IRasterImageEditToolsManager* a_pManager, IEnumStrings** a_ppToolIDs);
	STDMETHOD(ToolNameGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppName);
	STDMETHOD(ToolDescGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppDesc);
	STDMETHOD(ToolIconIDGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, GUID* a_ptDefaultIcon);
	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(SupportedStates)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs);
	STDMETHOD(EditToolCreate)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool);
	STDMETHOD(WindowCreate)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow);

private:
	struct SKey
	{
		SKey(GUID a_tID, int a_nType) : tID(a_tID), nType(a_nType) {}
		bool operator<(SKey const& a_rhs) const
		{
			if (nType < a_rhs.nType)
				return true;
			if (nType > a_rhs.nType)
				return false;
			return memcmp(&tID, &a_rhs.tID, sizeof tID) < 0;
		}
		GUID tID;
		int nType;
	};
	typedef std::map<SKey, GUID> CIconIds;

	static HICON CreateBubbleIcon(HICON a_hSource);

private:
	CIconIds m_cIconIds;
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsPhoto), CRasterImageEditToolsPhoto)
