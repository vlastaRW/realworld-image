// DesignerViewFactoryVectorImageEditor.h : Declaration of the CDesignerViewFactoryVectorImageEditor

#pragma once
#include <RWInput.h>
#include <RWConceptDesignerView.h>
#include <RWProcessing.h>
#include <RWDrawing.h>
#include "RasterImageEditToolsFactoryImpl.h"
#include "DesignerFrameIconsImpl.h"


HICON GetIconSplitShape(ULONG a_nSize);
HICON GetIconCombineShape(ULONG a_nSize);
HICON GetIconCombineUnion(ULONG a_nSize);
HICON GetIconCombineIntersection(ULONG a_nSize);
HICON GetIconCombineDifference(ULONG a_nSize);
HICON GetIconConvertToShape(ULONG a_nSize);
HICON GetIconConvertToPoly(ULONG a_nSize);
HICON GetIconOffsetShape(ULONG a_nSize);
extern __declspec(selectany) GUID const g_aUIDs[] =
{
	{0x290e6e87, 0x3096, 0x4a70, {0xbd, 0xdc, 0x24, 0xf9, 0x82, 0x53, 0x94, 0xa8}}, // CLSID_DocumentOperationSplitShape,
	{0x290e6e87, 0x3096, 0x4a70, {0xbd, 0xdc, 0x24, 0xf9, 0x82, 0x53, 0x94, 0xa9}},
	{0x658eddd9, 0x3ec3, 0x439f, {0x85, 0x83, 0x0e, 0x5c, 0x4d, 0x0c, 0x8e, 0x75}}, // CLSID_DocumentOperationCombineShapes
	{0x658eddd9, 0x3ec3, 0x439f, {0x85, 0x83, 0x0e, 0x5c, 0x4d, 0x0c, 0x8e, 0x76}},
	{0x658eddd9, 0x3ec3, 0x439f, {0x85, 0x83, 0x0e, 0x5c, 0x4d, 0x0c, 0x8e, 0x77}},
	{0xcef65495, 0x18f6, 0x4347, {0xad, 0x92, 0x62, 0xdf, 0x75, 0xa2, 0xff, 0x77}}, // CLSID_DocumentOperationObjectToShape
	{0xcef65495, 0x18f6, 0x4347, {0xad, 0x92, 0x62, 0xdf, 0x75, 0xa2, 0xff, 0x78}},
	{0x2b0d9f5b, 0x7df6, 0x4c53, {0xa0, 0xa2, 0x08, 0xb9, 0x0d, 0x8b, 0xd9, 0x95}}, // CLSID_VectorImageOffsetShape,
};
extern __declspec(selectany) pfnGetDFIcon const g_aGetIcons[] =
{
	GetIconSplitShape,
	GetIconCombineShape,
	GetIconCombineUnion,
	GetIconCombineIntersection,
	GetIconCombineDifference,
	GetIconConvertToShape,
	GetIconConvertToPoly,
	GetIconOffsetShape,
};


extern __declspec(selectany) SToolSpec const g_aVectorTools[] =
{
	{
		L"VECTORSELECT", L"[0409]Select[0405]Výběr", L"[0409]Click to select and edit existing shapes[0405]Klikněte pro výběr a úpravy existujících tvarů.",
		{0xa570b128, 0xde70, 0x41c4, {0x8f, 0xc3, 0x65, 0xc5, 0xfb, 0x35, 0x01, 0x11}}, 0,
		0, 0, 0, ETPSNoPaint, FALSE,
		NULL, NULL
	},
};

extern __declspec(selectany) GUID const CLSID_DesignerViewFactoryVectorImageEditor = { 0xf9d96fd7, 0x288b, 0x402c, { 0xb6, 0x25, 0x4c, 0xab, 0x10, 0x43, 0x13, 0x8f } };
class DECLSPEC_UUID("F9D96FD7-288B-402C-B625-4CAB1043138F") DesignerViewFactoryVectorImageEditor;


// CDesignerViewFactoryVectorImageEditor

class ATL_NO_VTABLE CDesignerViewFactoryVectorImageEditor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryVectorImageEditor, &CLSID_DesignerViewFactoryVectorImageEditor>,
	public IDesignerViewFactory,
	public CDesignerFrameIconsImpl<sizeof(g_aUIDs)/sizeof(g_aUIDs[0]), g_aUIDs, NULL, g_aGetIcons>,
	public CRasterImageEditToolsFactoryImpl<g_aVectorTools, sizeof(g_aVectorTools)/sizeof(g_aVectorTools[0])>
{
public:
	CDesignerViewFactoryVectorImageEditor()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryVectorImageEditor)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryVectorImageEditor)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryVectorImageEditor)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryVectorImageEditor), CDesignerViewFactoryVectorImageEditor)
