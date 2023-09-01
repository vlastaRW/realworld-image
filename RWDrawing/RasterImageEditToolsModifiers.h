// RasterImageEditToolsModifiers.h : Declaration of the CRasterImageEditToolsModifiers

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include <WeakSingleton.h>



// CRasterImageEditToolsModifiers

class ATL_NO_VTABLE CRasterImageEditToolsModifiers :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsModifiers, &CLSID_RasterImageEditToolsModifiers>,
	public IRasterImageEditToolsFactory
{
public:
	CRasterImageEditToolsModifiers()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CRasterImageEditToolsModifiers)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsModifiers)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsModifiers)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
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

	static HICON CreateEraserIcon(HICON a_hSource);
	static HICON CreateStylizeIcon(HICON a_hSource);

private:
	CIconIds m_cIconIds;
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsModifiers), CRasterImageEditToolsModifiers)
