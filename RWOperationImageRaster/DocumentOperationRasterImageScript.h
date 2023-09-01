// DocumentOperationRasterImageScript.h : Declaration of the CDocumentOperationRasterImageScript

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"
#include <DesignerFrameIconsImpl.h>

extern __declspec(selectany) UINT s_aScriptIconResIDs[] =
{
	IDI_SCRIPTALPHA,
	IDI_SCRIPTBRIGHTNESS,
	IDI_SCRIPTCONTRAST,
	IDI_SCRIPTCONTURES,
	0,//IDI_SCRIPTFLIP,
	0,//IDI_SCRIPTMIRROR,
	0,//IDI_SCRIPTRESIZE,
	0,//IDI_SCRIPTBORDERS,
	IDI_SCRIPTMIXALPHA,
	0,//IDI_SCRIPTCOLORIZE,
	0,//IDI_SCRIPTGRAYSCALE,
	IDI_SCRIPTSHADOW,
	0,//IDI_IMAGEPROPERTIES,
	0,//IDI_RASTERIMAGE,
	0,//IDI_IMAGEROTATE,
	IDI_IMAGEHUETORUS,
	//IDI_IMAGESHARPEN,
	0,//IDI_COLORADJUETMENTS,
	0,//IDI_IMAGE_BEVEL,
	0,//IDI_IMAGE_AUTOCONTRAST,
	0,//IDI_EXPORTIMAGE,
	0,//IDI_IMAGE_OUTLINE,
	0,//IDI_IMAGE_FILL,
	0,//IDI_IMAGE_SHADOW,
	0,//IDI_IMAGE_POLARXFORM,
	0,//IDI_IMAGE_EXPOSURE,
	0,//IDI_IMAGE_TRANSFORM,
	0,//IDI_IMAGE_BLUR,
	0,//IDI_IMAGE_CONVOLUTION,
	IDI_IMAGE_FADE,
	IDI_IMAGE_BACKGROUND,
	0,//IDI_IMAGE_SATIN,
	0,//IDI_IMAGE_CONTOURGRADIENT,
};
extern __declspec(selectany) GUID s_aScriptIconUIDs[] =
{
	{0x574e1f93, 0x8e46, 0x40c5, {0xa7, 0xc4, 0x02, 0x26, 0x58, 0x35, 0x4b, 0x19}},
	{0xd79a831b, 0x7af7, 0x46e0, {0xa7, 0xcd, 0xcc, 0x3d, 0x22, 0xa8, 0x73, 0xaf}},
	{0x9a92568d, 0xa1bd, 0x45e5, {0x81, 0x58, 0x33, 0x26, 0xfe, 0x44, 0x95, 0x0f}},
	{0x4666580f, 0x2397, 0x42c0, {0xbe, 0x45, 0x11, 0x7b, 0x33, 0xd7, 0x41, 0xb1}},
	{0x8ffce732, 0xaac5, 0x4192, {0xa0, 0xaa, 0x42, 0x5a, 0xe7, 0x5f, 0x12, 0x00}},
	{0xe44c597e, 0x6f47, 0x47be, {0xa5, 0xf9, 0x87, 0x4c, 0xdd, 0x0f, 0x27, 0xa7}},
	{0x86d01424, 0x28c6, 0x4bb5, {0x84, 0x3a, 0xf8, 0x1e, 0xac, 0xe2, 0x9a, 0xd4}},
	{0xbfee8e99, 0xdbaa, 0x4ff6, {0x9a, 0x61, 0x67, 0x27, 0xc9, 0x4b, 0x9f, 0x32}},
	{0x02725d55, 0x5508, 0x44b9, {0xb7, 0x7f, 0x13, 0x3c, 0xcc, 0xd4, 0x53, 0x9a}},
	CLSID_DocumentOperationRasterImageColorize,//{0x71fb12f4, 0x663d, 0x40d7, {0x84, 0x02, 0xf6, 0xb2, 0xa9, 0x12, 0xe6, 0x4d}},
	{0x8f1fc6b6, 0x41a4, 0x4a7b, {0xb2, 0xaa, 0xe8, 0x43, 0x23, 0x0d, 0x2b, 0xf5}},
	//{0xbaf69596, 0x10fb, 0x4c25, {0x96, 0x2a, 0xc1, 0x70, 0x2c, 0x3c, 0xc5, 0xcf}},
	{0x6cbad117, 0x2471, 0x492e, {0xbb, 0x3f, 0xa3, 0x10, 0xcb, 0x58, 0x05, 0x27}},
	CLSID_DocumentOperationRasterImageProps,
	{0x8b16a678, 0x97f5, 0x4ed5, {0x81, 0x6d, 0xbb, 0xe5, 0x70, 0x68, 0x76, 0x89}},
	CLSID_DocumentOperationRasterImageRotate,//{0x6d0b9ab0, 0xf727, 0x414e, {0x8f, 0x8c, 0x63, 0x49, 0x0c, 0x8b, 0x7d, 0x07}},
	{0x259e5e88, 0x5584, 0x44d3, {0x8f, 0x0a, 0xda, 0x05, 0x30, 0x0a, 0x21, 0x19}},
	//{0x715f55fb, 0xc711, 0x4e36, {0xb3, 0x9b, 0x20, 0x8e, 0xe2, 0x31, 0x61, 0x28}}, // CLSID_DocumentOperationRasterImageUnsharpMask
	//CLSID_DocumentOperationRasterImageConvolution,
	CLSID_DocumentOperationRasterImageShiftHue,
	CLSID_RasterImageBevel,
	{0x7b852ca0, 0xdbec, 0x4233, {0x94, 0x40, 0x30, 0x89, 0xc8, 0xd5, 0xec, 0x1c}}, // CLSID_DocumentOperationRasterImageAutoContrast
	//{0x0ef8a25b, 0xff07, 0x4a9e, {0x9b, 0x7a, 0x08, 0x2e, 0xaf, 0xc9, 0x4d, 0x92}},
	{0xca4e1501, 0x6831, 0x450c, {0xa2, 0xc8, 0x58, 0x50, 0x54, 0x1c, 0xf6, 0x5e}},
	CLSID_DocumentOperationRasterImageOutlines,
	CLSID_RasterImageFill,
	CLSID_DocumentOperationRasterImageDropShadow,
	CLSID_RasterImagePolarTransformation,
	CLSID_DocumentOperationRasterImageColorTransformations,
	CLSID_DocumentOperationRasterImagePerspective,
	CLSID_DocumentOperationRasterImageBlur,
	CLSID_DocumentOperationRasterImageConvolution,
	CLSID_DocumentOperationRasterImageFade,
	CLSID_DocumentOperationRasterImageBackgroundBlend,
	CLSID_DocumentOperationRasterImageSatin,
	{0x20f5be48, 0x8d2, 0x4b8f, {0x8d, 0xfc, 0x5d, 0xe8, 0x57, 0x6e, 0x1a, 0x3f}},//CLSID_RasterImageContourGradient
};

#include "DocumentOperationRasterImagePerspective.h" //class ATL_NO_VTABLE CDocumentOperationRasterImagePerspective { public: static HICON GetDefaultIcon(ULONG a_nSize); };
#include "DocumentOperationRasterImageBlur.h"
#include "DocumentOperationRasterImageOutlines.h"
#include "DocumentOperationRasterImageDropShadow.h"
#include "DocumentOperationRasterImageSatin.h"
#include "DocumentOperationRasterImageConvolution.h"
#include "DocumentOperationRasterImageColorize.h"
#include "DocumentOperationRasterImageShiftHue.h"
#include "DocumentOperationRasterImageColorTransformations.h"
#include "DocumentOperationRasterImageRotate.h"
#include "RasterImagePolarTransformation.h"
#include "RasterImageBevel.h"
#include "RasterImageFill.h"

HICON GetDefaultIconContourGradient(ULONG a_nSize);
HICON GetIconPicture(ULONG a_nSize);
HICON GetIconSaveImage(ULONG a_nSize);
HICON GetIconCanvas(ULONG a_nSize);
HICON GetIconResample(ULONG a_nSize);
HICON GetIconBorders(ULONG a_nSize);
HICON GetIconAutoContrast(ULONG a_nSize);
HICON GetIconCheckmark(ULONG a_nSize);
HICON GetIconMirror(ULONG a_nSize);
HICON GetIconFlip(ULONG a_nSize);

extern __declspec(selectany) pfnGetDFIcon s_aGetIcon[] =
{
	NULL,//IDI_SCRIPTALPHA,
	NULL,//IDI_SCRIPTBRIGHTNESS,
	NULL,//IDI_SCRIPTCONTRAST,
	NULL,//IDI_SCRIPTCONTURES,
	GetIconFlip,//IDI_SCRIPTFLIP,
	GetIconMirror,//IDI_SCRIPTMIRROR,
	GetIconResample,//IDI_SCRIPTRESIZE,
	GetIconBorders,//IDI_SCRIPTBORDERS,
	NULL,//IDI_SCRIPTMIXALPHA,
	CDocumentOperationRasterImageColorize::GetDefaultIcon,//IDI_SCRIPTCOLORIZE,
	GetIconCanvas,
	NULL,//IDI_SCRIPTSHADOW,
	GetIconCheckmark,//IDI_IMAGEPROPERTIES,
	GetIconPicture,//IDI_RASTERIMAGE,
	CDocumentOperationRasterImageRotate::GetDefaultIcon,//IDI_IMAGEROTATE,
	NULL,//IDI_IMAGEHUETORUS,
	//NULL,//IDI_IMAGESHARPEN,
	CDocumentOperationRasterImageShiftHue::GetDefaultIcon,//IDI_COLORADJUETMENTS,
	CRasterImageBevel::GetDefaultIcon,//IDI_IMAGE_BEVEL,
	GetIconAutoContrast,//IDI_IMAGE_AUTOCONTRAST,
	GetIconSaveImage,//IDI_EXPORTIMAGE,
	CDocumentOperationRasterImageOutlines::GetDefaultIcon,//IDI_IMAGE_OUTLINE,
	CRasterImageFill::GetDefaultIcon,//IDI_IMAGE_FILL,
	CDocumentOperationRasterImageDropShadow::GetDefaultIcon,//IDI_IMAGE_SHADOW,
	CRasterImagePolarTransformation::GetDefaultIcon,//IDI_IMAGE_POLARXFORM,
	CDocumentOperationRasterImageColorTransformations::GetDefaultIcon,//IDI_IMAGE_EXPOSURE,
	CDocumentOperationRasterImagePerspective::GetDefaultIcon,//IDI_IMAGE_TRANSFORM,
	CDocumentOperationRasterImageBlur::GetDefaultIcon,//IDI_IMAGE_BLUR,
	CDocumentOperationRasterImageConvolution::GetDefaultIcon,//IDI_IMAGE_CONVOLUTION,
	NULL,//IDI_IMAGE_FADE,
	NULL,//IDI_IMAGE_BACKGROUND,
	CDocumentOperationRasterImageSatin::GetDefaultIcon,//IDI_IMAGE_SATIN,
	GetDefaultIconContourGradient,//IDI_IMAGE_CONTOURGRADIENT,
};

// CDocumentOperationRasterImageScript

class ATL_NO_VTABLE CDocumentOperationRasterImageScript : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationRasterImageScript, &CLSID_DocumentOperationRasterImageScript>,
	public IScriptingInterface,
	public CDesignerFrameIconsImpl<sizeof(s_aScriptIconUIDs)/sizeof(s_aScriptIconUIDs[0]), s_aScriptIconUIDs, s_aScriptIconResIDs, s_aGetIcon>
{
public:
	CDocumentOperationRasterImageScript()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationRasterImageScript)

BEGIN_CATEGORY_MAP(CDocumentOperationRasterImageScript)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
	IMPLEMENTED_CATEGORY(CATID_ScriptingInterface)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationRasterImageScript)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
	COM_INTERFACE_ENTRY(IScriptingInterface)
END_COM_MAP()


	// IScriptingInterface methods
public:
	STDMETHOD(GetGlobalObjects)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);
	STDMETHOD(GetInterfaceAdaptors)(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument);
	STDMETHOD(GetKeywords)(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary);
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationRasterImageScript), CDocumentOperationRasterImageScript)
