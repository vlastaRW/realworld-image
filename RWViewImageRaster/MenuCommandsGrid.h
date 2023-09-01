// MenuCommandsGrid.h : Declaration of the CMenuCommandsGrid

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>


// CMenuCommandsGrid

class ATL_NO_VTABLE CMenuCommandsGrid :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsGrid, &CLSID_MenuCommandsGrid>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsGrid()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsGrid)

BEGIN_CATEGORY_MAP(CMenuCommandsGrid)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsGrid)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID, ULONG t_nGrid>
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CDocumentMenuCommandImpl<CDocumentMenuCommand<t_uIDName, t_uIDDesc, t_pIconID, t_nGrid>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			ULONG nGrid = 0xffffffff;
			return pRIG && pRIG->CanSetGrid(t_nGrid) == S_OK ? (SUCCEEDED(pRIG->GetGrid(&nGrid)) && nGrid == t_nGrid ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				static IRPolyPoint const box[] = { {0, 0}, {0, 256}, {256, 256}, {256, 0}, };
				static float const pts[] = {60, 105, 151, 196};
				static IRPolyPoint const vline1[] = { {pts[0], 16}, {pts[0], 240}, };
				static IRPolyPoint const vline2[] = { {pts[3], 16}, {pts[3], 240}, };
				static IRPolyPoint const vline3[] = { {pts[1], 16}, {pts[1], 240}, };
				static IRPolyPoint const vline4[] = { {pts[2], 16}, {pts[2], 240}, };
				static IRPolyPoint const hline1[] = { {16, pts[0]}, {240, pts[0]}, };
				static IRPolyPoint const hline2[] = { {16, pts[3]}, {240, pts[3]}, };
				static IRPolyPoint const hline3[] = { {16, pts[1]}, {240, pts[1]}, };
				static IRPolyPoint const hline4[] = { {16, pts[2]}, {240, pts[2]}, };
				static IRPolygon const lines[] =
				{
					{itemsof(vline1), vline1},
					{itemsof(vline2), vline2},
					{itemsof(hline1), hline1},
					{itemsof(hline2), hline2},
					{itemsof(vline3), vline3},
					{itemsof(vline4), vline4},
					{itemsof(hline3), hline3},
					{itemsof(hline4), hline4},
				};
				static IRGridItem const grid[] = { {0, 0}, {EGIFMidPixel, pts[0]}, {EGIFMidPixel, pts[1]}, {EGIFMidPixel, pts[2]}, {EGIFMidPixel, pts[3]}, {0, 256} };
				static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
				cRenderer(&canvas, itemsof(box), box, pSI->GetMaterial(ESMAltBackground));
				if (t_nGrid == 8)
					cRenderer(&canvas, 4, lines, pSI->GetMaterial(ESMOutlineSoft));
				else if (t_nGrid == 1)
					cRenderer(&canvas, 8, lines, pSI->GetMaterial(ESMOutlineSoft));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IEnumUnknownsInit> p;
				RWCoCreateInstance(p, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
				CComPtr<IRasterEditView> pRIG;
				p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
				return pRIG ? pRIG->SetGrid(t_nGrid) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsGrid), CMenuCommandsGrid)
