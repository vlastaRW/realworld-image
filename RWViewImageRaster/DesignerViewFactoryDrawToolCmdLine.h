// DesignerViewFactoryDrawToolCmdLine.h : Declaration of the CDesignerViewFactoryDrawToolCmdLine

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <DesignerFrameIconsImpl.h>
#include <IconRenderer.h>


inline HICON GetDFIconCmdLine(ULONG size)
{
	static IRPathPoint const greater[] =
	{
		{153, 25, 0, 0, -12.1503, -12.1503},
		{256, 128, 0, 0, 0, 0},
		{153, 231, -11.6889, 11.6889, 0, 0},
		{109, 231, -12.6117, -11.6889, 12.6117, 11.6889},
		{109, 187, 0, 0, -12.6117, 12.6117},
		{168, 128, 0, 0, 0, 0},
		{109, 69, -12.1503, -12.1503, 0, 0},
		{109, 25, 12.1503, -12.1503, -12.1503, 12.1503},
	};
	static IRPathPoint const top[] =
	{
		{84, 76, 0, -23.196, 0, 23.196},
		{42, 34, -23.196, 0, 23.196, 0},
		{0, 76, 0, 23.196, 0, -23.196},
		{42, 118, 23.196, 0, -23.196, 0},
		//{76, 76, 0, -20.9868, 0, 20.9868},
		//{38, 38, -20.9868, 0, 20.9868, 0},
		//{0, 76, 0, 20.9868, 0, -20.9868},
		//{38, 114, 20.9868, 0, -20.9868, 0},
	};
	static IRPathPoint const bot[] =
	{
		{84, 180, 0, -23.196, 0, 23.196},
		{42, 138, -23.196, 0, 23.196, 0},
		{0, 180, 0, 23.196, 0, -23.196},
		{42, 222, 23.196, 0, -23.196, 0},
		//{76, 180, 0, -20.9868, 0, 20.9868},
		//{38, 142, -20.9868, 0, 20.9868, 0},
		//{0, 180, 0, 20.9868, 0, -20.9868},
		//{38, 218, 20.9868, 0, -20.9868, 0},
	};
	static IRPath const shapes[] = { {itemsof(greater), greater}, {itemsof(top), top}, {itemsof(bot), bot} };
	IRCanvas canvas = {0, 12, 256, 244, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);

	cRenderer(&canvas, itemsof(shapes), shapes, pSI->GetMaterial(ESMInterior), IRTarget(0.92f));

	return cRenderer.get();
}

extern __declspec(selectany) GUID const g_tIconIDToolCmdLine = {0xb2669851, 0x913a, 0x461f, {0xbd, 0xff, 0xea, 0x3f, 0x4c, 0x4b, 0x59, 0x19}};
extern __declspec(selectany) pfnGetDFIcon const g_tToolCmdLineGetIcon = GetDFIconCmdLine;


// CDesignerViewFactoryDrawToolCmdLine

class ATL_NO_VTABLE CDesignerViewFactoryDrawToolCmdLine :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDesignerViewFactoryDrawToolCmdLine, &CLSID_DesignerViewFactoryDrawToolCmdLine>,
	public IDesignerViewFactory,
	public CDesignerFrameIconsImpl<1, &g_tIconIDToolCmdLine, NULL, &g_tToolCmdLineGetIcon>
{
public:
	CDesignerViewFactoryDrawToolCmdLine()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDesignerViewFactoryDrawToolCmdLine)

BEGIN_CATEGORY_MAP(CDesignerViewFactoryDrawToolCmdLine)
	IMPLEMENTED_CATEGORY(CATID_DesignerViewFactory)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDesignerViewFactoryDrawToolCmdLine)
	COM_INTERFACE_ENTRY(IDesignerViewFactory)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IDesignerViewFactory methods
public:
	STDMETHOD(NameGet)(IViewManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IViewManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CreateWnd)(IViewManager* a_pManager, IConfig* a_pConfig, ISharedStateManager* a_pFrame, IStatusBarObserver* a_pStatusBar, IDocument* a_pDoc, RWHWND a_hParent, RECT const* a_prcWindow, EDesignerViewWndStyle a_nStyle, LCID a_tLocaleID, IDesignerView** a_ppDVWnd);
	STDMETHOD(CheckSuitability)(IViewManager* a_pManager, IConfig* a_pConfig, IDocument* a_pDocument, ICheckSuitabilityCallback* a_pCallback);

};

OBJECT_ENTRY_AUTO(__uuidof(DesignerViewFactoryDrawToolCmdLine), CDesignerViewFactoryDrawToolCmdLine)
