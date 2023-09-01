// MenuCommandsImageZoom.h : Declaration of the CMenuCommandsImageZoom

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImage.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>


extern __declspec(selectany) GUID const IconIDZoomIn = {0x96ef8a35, 0x696d, 0x43fc, {0x8a, 0xe0, 0xcb, 0x13, 0xc, 0xa8, 0x4a, 0x37}};
extern __declspec(selectany) GUID const IconIDZoomOut = {0x145a9160, 0x47af, 0x4a7b, {0xa9, 0x7b, 0xc0, 0xb7, 0xe7, 0x2, 0xf9, 0xb2}};
extern __declspec(selectany) GUID const IconIDZoomAuto = {0x3adee759, 0xc783, 0x4243, {0xb5, 0x3f, 0x4e, 0x1b, 0xce, 0x86, 0xfd, 0x14}};

// CMenuCommandsImageZoom

class ATL_NO_VTABLE CMenuCommandsImageZoom :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsImageZoom, &CLSID_MenuCommandsImageZoom>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsImageZoom()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsImageZoom)

BEGIN_CATEGORY_MAP(CMenuCommandsDrawingTools)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsImageZoom)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuZoomIn :
		public CDocumentMenuCommandImpl<CDocumentMenuZoomIn, IDS_MENU_ZOOMIN_NAME, IDS_MENU_ZOOMIN_DESC, &IconIDZoomIn, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cReceiver(a_nSize);
			pSI->GetLayers(ESIMagnifier, cReceiver, IRTarget(0.85f, -1, 1));
			pSI->GetLayers(ESIPlus, cReceiver, IRTarget(0.65f, 1, -1));
			*a_phIcon = cReceiver.get();
			return *a_phIcon ? S_OK : E_FAIL;

			//CComPtr<IIconRenderer> pIR;
			//RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			//IRPathPoint const glass0[] =
			//{
			//	{174.5, 92.5, 0, 15.1974, 0, -45.2874},
			//	{163.15, 134.126, 0, 0, 7.20521, -12.2028},
			//	{241, 199, 5.07682, 5.07682, 0, 0},
			//	{241, 217, 0, 0, 5.07682, -5.07682},
			//	{217, 241, -5.07682, 5.07682, 0, 0},
			//	{199, 241, 0, 0, 5.07682, 5.07682},
			//	{134.126, 163.15, -12.2028, 7.20521, 0, 0},
			//	{92.5, 174.5, -45.2874, 0, 15.1974, 0},
			//	{10.5, 92.5, 0, -45.2874, 0, 45.2874},
			//	{92.5, 10.5, 45.2874, 0, -45.2874, 0},
			//};
			//IRPathPoint const glass1[] =
			//{
			//	{92.5, 30.5, 34.2417, 0, -34.2417, 0},
			//	{154.5, 92.5, 0, 34.2417, 0, -34.2417},
			//	{92.5, 154.5, -34.2417, 0, 34.2417, 0},
			//	{30.5, 92.5, 0, -34.2417, 0, 34.2417},
			//};
			//IRPathPoint const glass2[] =
			//{
			//	{107, 43, -32.2602, 2.18306, -4.68001, -1.38319},
			//	{50, 105, 0, 8.06891, 0, -32.8138},
			//	{54, 127, -8.32019, -9.21885, -2.8044, -7.0705},
			//	{41, 93, 0, -28.7188, 0, 13.3961},
			//	{93, 41, 5.12841, 0, -28.7188, 0},
			//};
			//IRPolyPoint const plus[] =
			//{
			//	{80, 0}, {176, 0}, {176, 80}, {256, 80}, {256, 176}, {176, 176}, {176, 256}, {80, 256}, {80, 176}, {0, 176}, {0, 80}, {80, 80}
			//};
			//IRPath const shape0[] =
			//{
			//	{itemsof(glass0), glass0},
			//	{itemsof(glass1), glass1},
			//	//{itemsof(glass2), glass2},
			//};
			//IRPolygon const shape1 = {itemsof(plus), plus};
			//IRCanvas const canvasSign = {-96, 0, 256, 256+96, 0, 0, NULL, NULL};
			//IRCanvas const canvasGlass = {10, -42, 256+42, 256-10, 0, 0, NULL, NULL};
			//IRFillWithInternalOutline material0(0xff6782eb, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 352/20.0f);
			//IRFillWithInternalOutline material1(0xff000000|GetSysColor(COLOR_WINDOW), 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 288/20.0f);
			//IRLayer const layers[] =
			//{
			//	{&canvasGlass, 0, itemsof(shape0), NULL, shape0, &material1},
			//	{&canvasSign, 1, 0, &shape1, NULL, &material0},
			//};
			//*a_phIcon = pIR->CreateIcon(a_nSize, itemsof(layers), layers);
			//return *a_phIcon ? S_OK : E_FAIL;
		}
		EMenuCommandState IntState();
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* UNREF(a_pAuxAccel))
		{
			a_pAccel->wKeyCode = VK_ADD;
			a_pAccel->fVirtFlags = FCONTROL;
			return S_OK;
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CDocumentMenuZoomOut :
		public CDocumentMenuCommandImpl<CDocumentMenuZoomOut, IDS_MENU_ZOOMOUT_NAME, IDS_MENU_ZOOMOUT_DESC, &IconIDZoomOut, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CIconRendererReceiver cReceiver(a_nSize);
			pSI->GetLayers(ESIMagnifier, cReceiver, IRTarget(0.85f, -1, 1));
			pSI->GetLayers(ESIMinus, cReceiver, IRTarget(0.65f, 1, -1));
			*a_phIcon = cReceiver.get();
			return *a_phIcon ? S_OK : E_FAIL;

			//CComPtr<IIconRenderer> pIR;
			//RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			//IRPathPoint const glass0[] =
			//{
			//	{174.5, 92.5, 0, 15.1974, 0, -45.2874},
			//	{163.15, 134.126, 0, 0, 7.20521, -12.2028},
			//	{241, 199, 5.07682, 5.07682, 0, 0},
			//	{241, 217, 0, 0, 5.07682, -5.07682},
			//	{217, 241, -5.07682, 5.07682, 0, 0},
			//	{199, 241, 0, 0, 5.07682, 5.07682},
			//	{134.126, 163.15, -12.2028, 7.20521, 0, 0},
			//	{92.5, 174.5, -45.2874, 0, 15.1974, 0},
			//	{10.5, 92.5, 0, -45.2874, 0, 45.2874},
			//	{92.5, 10.5, 45.2874, 0, -45.2874, 0},
			//};
			//IRPathPoint const glass1[] =
			//{
			//	{92.5, 30.5, 34.2417, 0, -34.2417, 0},
			//	{154.5, 92.5, 0, 34.2417, 0, -34.2417},
			//	{92.5, 154.5, -34.2417, 0, 34.2417, 0},
			//	{30.5, 92.5, 0, -34.2417, 0, 34.2417},
			//};
			//IRPathPoint const glass2[] =
			//{
			//	{107, 43, -32.2602, 2.18306, -4.68001, -1.38319},
			//	{50, 105, 0, 8.06891, 0, -32.8138},
			//	{54, 127, -8.32019, -9.21885, -2.8044, -7.0705},
			//	{41, 93, 0, -28.7188, 0, 13.3961},
			//	{93, 41, 5.12841, 0, -28.7188, 0},
			//};
			//IRPolyPoint const minus[] =
			//{
			//	{256, 40}, {256, 136}, {0, 136}, {0, 40}
			//};
			//IRPath const shape0[] =
			//{
			//	{itemsof(glass0), glass0},
			//	{itemsof(glass1), glass1},
			//	//{itemsof(glass2), glass2},
			//};
			//IRPolygon const shape1 = {itemsof(minus), minus};
			//IRCanvas const canvasSign = {-96, 0, 256, 256+96, 0, 0, NULL, NULL};
			//IRCanvas const canvasGlass = {10, -42, 256+42, 256-10, 0, 0, NULL, NULL};
			//IRFillWithInternalOutline material0(0xff6782eb, 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 352/20.0f);
			//IRFillWithInternalOutline material1(0xff000000|GetSysColor(COLOR_WINDOW), 0xff000000|GetSysColor(COLOR_WINDOWTEXT), 288/20.0f);
			//IRLayer const layers[] =
			//{
			//	{&canvasGlass, 0, itemsof(shape0), NULL, shape0, &material1},
			//	{&canvasSign, 1, 0, &shape1, NULL, &material0},
			//};
			//*a_phIcon = pIR->CreateIcon(a_nSize, itemsof(layers), layers);
			//return *a_phIcon ? S_OK : E_FAIL;
		}
		EMenuCommandState IntState();
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* UNREF(a_pAuxAccel))
		{
			a_pAccel->wKeyCode = VK_SUBTRACT;
			a_pAccel->fVirtFlags = FCONTROL;
			return S_OK;
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CDocumentMenuAutoZoom :
		public CDocumentMenuCommandImpl<CDocumentMenuAutoZoom, IDS_MENU_ZOOMAUTO_NAME, IDS_MENU_ZOOMAUTO_DESC, &IconIDZoomAuto, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			IRPolyPoint const corner0[] = {{0, 32}, {80, 32}, {0, 112}};
			IRPolyPoint const corner1[] = {{256, 32}, {256, 112}, {176, 32}};
			IRPolyPoint const corner2[] = {{256, 224}, {176, 224}, {256, 144}};
			IRPolyPoint const corner3[] = {{0, 224}, {0, 144}, {80, 224}};
			IRPolygon const shape0[] =
			{
				{itemsof(corner0), corner0},
				{itemsof(corner1), corner1},
				{itemsof(corner2), corner2},
				{itemsof(corner3), corner3},
			};
			static IRGridItem const tGridX[] = {{EGIFInteger, 0.0f}, {EGIFInteger, 256.0f}};
			static IRGridItem const tGridY[] = {{EGIFInteger, 32.0f}, {EGIFInteger, 224.0f}};
			static IRCanvas const canvas = {0, 0, 256, 256, itemsof(tGridX), itemsof(tGridY), tGridX, tGridY};
			CComPtr<IIconRenderer> pIR;
			RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			*a_phIcon = pIR->CreateIcon(a_nSize, &canvas, itemsof(shape0), shape0, pSI->GetMaterial(ESMManipulate));
			return *a_phIcon ? S_OK : E_FAIL;
		}
		EMenuCommandState IntState();
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CDocumentMenuZoomPopup :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CDocumentMenuZoomPopup)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* UNREF(a_pIconID))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Icon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Accelerator)(TCmdAccel* UNREF(a_pAccel), TCmdAccel* UNREF(a_pAuxAccel))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands);
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				*a_peState = static_cast<EMenuCommandState>((4<<24)|EMCSShowButtonText|EMCSSubMenu);
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return E_NOTIMPL;
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CDocumentMenuConstantZoom :
		public CDocumentMenuCommandImpl<CDocumentMenuConstantZoom, IDS_MENU_ZOOMCONST_NAME, IDS_MENU_ZOOMCONST_DESC, NULL, 0>
	{
	public:
		void Init(IDesignerView* a_pView, float a_fZoom)
		{
			m_pView = a_pView;
			m_fZoom = a_fZoom;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		EMenuCommandState IntState();
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IDesignerView> m_pView;
		float m_fZoom;
	};

	class ATL_NO_VTABLE CDocumentMenuCustomZoom :
		public CDocumentMenuCommandImpl<CDocumentMenuCustomZoom, IDS_MENU_ZOOMCUSTOM_NAME, IDS_MENU_ZOOMCUSTOM_DESC, NULL, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsImageZoom), CMenuCommandsImageZoom)
