// MenuCommandsComposedView.h : Declaration of the CMenuCommandsComposedView

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include <IConRenderer.h>


// CMenuCommandsComposedView

class ATL_NO_VTABLE CMenuCommandsComposedView :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsComposedView, &CLSID_MenuCommandsComposedView>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsComposedView()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsComposedView)

BEGIN_CATEGORY_MAP(CMenuCommandsComposedView)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsComposedView)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CDocumentMenuCommandImpl<CDocumentMenuCommand<t_uIDName, t_uIDDesc, t_pIconID>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IDesignerView* a_pView, EImageCompositon a_eMode)
		{
			m_pView = a_pView;
			m_eMode = a_eMode;
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
			EImageCompositon eMode;
			return pRIG && pRIG->CanComposeView() == S_OK && SUCCEEDED(pRIG->GetCompositionMode(&eMode)) ? (eMode == m_eMode ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				static IRPolyPoint const box1[] = { {76, 124}, {256, 124}, {194, 256}, {0, 236}, };
				static IRPolyPoint const box2[] = { {76, 62}, {256, 62}, {194, 194}, {0, 174}, };
				static IRPolyPoint const box3[] = { {76, 0}, {256, 0}, {194, 132}, {0, 112}, };
				//static IRPolyPoint const box1[] = { {0, 64}, {192, 64}, {192, 256}, {0, 256}, };
				//static IRPolyPoint const box2[] = { {32, 32}, {224, 32}, {224, 224}, {32, 224}, };
				//static IRPolyPoint const box3[] = { {64, 0}, {256, 0}, {256, 192}, {64, 192}, };
				static IRGridItem const grid[] = { {0, 0}, {0, 32}, {0, 64}, {0, 192}, {0, 224}, {0, 256} };
				static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
				if (m_eMode == EICFinalImage)
				{
					cRenderer(&canvas, itemsof(box1), box1, pSI->GetMaterial(ESMAltBackground));
					cRenderer(&canvas, itemsof(box2), box2, pSI->GetMaterial(ESMBackground));
					cRenderer(&canvas, itemsof(box3), box3, pSI->GetMaterial(ESMAltBackground));
				}
				else if (m_eMode != EICActiveLayer)
				{
					IRFill fill(0x7fffffff&pSI->GetSRGBColor(ESMAltBackground));
					IRFill outline(0x7fffffff&pSI->GetSRGBColor(ESMContrast));
					IROutlinedFill mat(&fill, &outline);
					cRenderer(&canvas, itemsof(box1), box1, &mat);
					cRenderer(&canvas, itemsof(box2), box2, pSI->GetMaterial(ESMBackground));
					cRenderer(&canvas, itemsof(box3), box3, &mat);
				}
				else
				{
					cRenderer(&canvas, itemsof(box2), box2, pSI->GetMaterial(ESMBackground));
				}
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
				return pRIG ? pRIG->SetCompositionMode(m_eMode) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
		EImageCompositon m_eMode;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsComposedView), CMenuCommandsComposedView)
