// OperationPrint.h : Declaration of the COperationPrint

#pragma once
#include "resource.h"       // main symbols

#include <AutoCategories.h>
#include <RWConceptDesignerExtension.h>
#include <RWProcessing.h>
#include <DesignerFrameIconsImpl.h>
#include <IconRenderer.h>

//class DECLSPEC_UUID("3C6C03F8-5A43-4C9C-8448-CD79B18BF7DE")
//OperationPrint;

// {3C6C03F8-5A43-4C9C-8448-CD79B18BF7DE}
extern __declspec(selectany) GUID const CLSID_OperationPrint = { 0x3c6c03f8, 0x5a43, 0x4c9c, { 0x84, 0x48, 0xcd, 0x79, 0xb1, 0x8b, 0xf7, 0xde } };


// COperationPrint

class ATL_NO_VTABLE COperationPrint :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<COperationPrint, &CLSID_OperationPrint>,
	public IDocumentOperation,
	public IAutoOperation,
	public IDesignerFrameIcons,
	public CConfigDescriptorImpl
{
public:
	COperationPrint()
	{
	}

	DECLARE_NO_REGISTRY()
	DECLARE_CLASSFACTORY_SINGLETON(COperationPrint)

	BEGIN_CATEGORY_MAP(COperationPrint)
		IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
		IMPLEMENTED_CATEGORY(CATID_AutoImageFile)
		IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
	END_CATEGORY_MAP()

	BEGIN_COM_MAP(COperationPrint)
		COM_INTERFACE_ENTRY(IDocumentOperation)
		COM_INTERFACE_ENTRY(IAutoOperation)
		COM_INTERFACE_ENTRY(IDesignerFrameIcons)
		COM_INTERFACE_ENTRY(IConfigDescriptor)
	END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

	// IAutoOperation methods
public:
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(IconID)(GUID* a_pIconID);
	STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel);
	STDMETHOD(Configuration)(IConfig* a_pOperationCfg);

public:
	STDMETHOD(TimeStamp)(ULONG* a_pTimeStamp)
	{
		try
		{
			*a_pTimeStamp = 0;
			return S_OK;
		}
		catch (...)
		{
			return a_pTimeStamp == NULL ? E_POINTER: E_UNEXPECTED;
		}
	}
	STDMETHOD(EnumIconIDs)(IEnumGUIDs** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CComPtr<IEnumGUIDsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumGUIDs));
			pTmp->Insert(CLSID_OperationPrint);
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIcon)(REFGUID a_tIconID, ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;

			if (IsEqualGUID(a_tIconID, CLSID_OperationPrint))
			{
				static IRPolyPoint const back[] = { {40, 96}, {216, 96}, {216, 235}, {40, 235} };
				static IRPolyPoint const paperf[] = { {40, 256}, {216, 256}, {196, 208}, {60, 208} };
				static IRPathPoint const holder[] =
				{
					{50, 49, 0, -7.1797, 0, 0},
					{63, 36, 0, 0, -7.1797, 0},
					{193, 36, 7.1797, 0, 0, 0},
					{206, 49, 0, 0, 0, -7.1797},
					{209, 138, 0, 0, 0, 0},
					{47, 139, 0, 0, 0, 0},
				};
				static IRPolyPoint const paperb[] = { {66, 0}, {190, 0}, {194, 140}, {62, 140} };
				static IRPathPoint const top[] =
				{
					{219, 96, 6.62742, 0, 0, 0},
					{234, 107, 0, 0, -3, -6},
					{255, 151, 0, 0, 0, 0},
					{1, 151, 0, 0, 0, 0},
					{22, 107, 3, -6, 0, 0},
					{37, 96, 0, 0, -6.62742, 0},
					{53, 96, 0, 0, 0, 0},
					{42, 128, 0, 0, 0, 0},
					{214, 128, 0, 0, 0, 0},
					{203, 96, 0, 0, 0, 0},
				};
				static IRPathPoint const front[] =
				{
					{12, 235, -6.62742, 0, 0, 0},
					{0, 223, 0, 0, 0, 6.62742},
					{0, 156, 0, -6.62742, 0, 0},
					{12, 144, 0, 0, -6.62742, 0},
					{244, 144, 6.62742, 0, 0, 0},
					{256, 156, 0, 0, 0, -6.62742},
					{256, 223, 0, 6.62742, 0, 0},
					{244, 235, 0, 0, 6.62742, 0},
					{208, 235, 0, 0, 0, 0},
					{208, 208, 0, 0, 0, 0},
					{48, 208, 0, 0, 0, 0},
					{48, 235, 0, 0, 0, 0},
				};
				static IRPathPoint const button[] =
				{
					{49, 174, 0, -5.79899, 0, 5.79899},
					{39, 163, -5.79899, 0, 5.79899, 0},
					{28, 174, 0, 5.79899, 0, -5.79899},
					{39, 184, 5.79899, 0, -5.79899, 0},
				};
				static IRGridItem const gridCaseX[] = { {0, 0}, {0, 48}, {0, 208}, {0, 256} };
				static IRGridItem const gridCaseY[] = { {0, 96}, {0, 144}, {0, 208}, {0, 235} };
				static IRCanvas const canvasCase = { 0, 0, 256, 256, itemsof(gridCaseX), itemsof(gridCaseY), gridCaseX, gridCaseY };
				static IRCanvas const canvas = { 0, 0, 256, 256, 0, 0, NULL, NULL };
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				cRenderer(&canvasCase, itemsof(back), back, pSI->GetMaterial(ESMContrast));
				cRenderer(&canvas, itemsof(paperf), paperf, pSI->GetMaterial(ESMBackground));
				cRenderer(&canvas, itemsof(holder), holder, pSI->GetMaterial(ESMAltBackground));
				cRenderer(&canvas, itemsof(paperb), paperb, pSI->GetMaterial(ESMBackground));
				cRenderer(&canvasCase, itemsof(top), top, pSI->GetMaterial(ESMAltBackground));
				cRenderer(&canvasCase, itemsof(front), front, pSI->GetMaterial(ESMAltBackground));
				cRenderer(&canvasCase, itemsof(button), button, pSI->GetMaterial(ESMBackground));
				*a_phIcon = cRenderer.get();
			}
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);

};

OBJECT_ENTRY_AUTO(CLSID_OperationPrint, COperationPrint)
