// MenuCommandsEXIF.h : Declaration of the CMenuCommandsEXIF

#pragma once
#include "RWEXIF_i.h"
#include <DesignerFrameIconsImpl.h>
#include <IconRenderer.h>


extern __declspec(selectany) GUID const g_aEXIFIconGUIDs[] =
{
	CLSID_DesignerViewFactoryEXIF,
	//{0xe2a9293c, 0x3d39, 0x4840, {0xbb, 0xa8, 0x4c, 0x59, 0xac, 0xb1, 0x6c, 0xf1}},
};
inline HICON GetIconProperties(ULONG size)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(size);
	pSI->GetLayers(ESIProperties, cRenderer);
	return cRenderer.get();
}
extern __declspec(selectany) pfnGetDFIcon const g_aEXIFGetIconFncs[] =
{
	GetIconProperties,
};


// CMenuCommandsEXIF

class ATL_NO_VTABLE CMenuCommandsEXIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsEXIF, &CLSID_MenuCommandsEXIF>,
	public IDocumentMenuCommands,
	public CDesignerFrameIconsImpl<sizeof(g_aEXIFIconGUIDs)/sizeof(*g_aEXIFIconGUIDs), g_aEXIFIconGUIDs, nullptr, g_aEXIFGetIconFncs>
{
public:
	CMenuCommandsEXIF()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsEXIF)

BEGIN_CATEGORY_MAP(CMenuCommandsEXIF)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsEXIF)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsEXIF), CMenuCommandsEXIF)
