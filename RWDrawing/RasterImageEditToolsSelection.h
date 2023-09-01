// RasterImageEditToolsSelection.h : Declaration of the CRasterImageEditToolsSelection

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageEditToolsFactoryImpl.h"

#include "EditToolSimpleSelect.h"
#include "EditToolSelect.h"
#include "EditToolMagicWand.h"
#include "EditToolTransform.h"
#include "EditToolMove.h"

extern __declspec(selectany) SToolSpec const g_aSelectionTools[] =
{
	{
		L"SIMPLESELECT", IDS_TOOLNAME_SIMPLESELECT, IDS_TOOLDESC_SIMPLESELECT,
		{0xb33b0053, 0x7a01, 0x4504, {0x92, 0x45, 0x1, 0xf5, 0xe3, 0x63, 0xae, 0x6b}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, 0, 0, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolSimpleSelect>, &CreateToolWindow<CEditToolSimpleSelectDlg>
	},
	{
		L"SELECT", IDS_TOOLNAME_SELECT, IDS_TOOLDESC_SELECT,
		{0xb33b0053, 0x7a01, 0x4504, {0x92, 0x45, 0x1, 0xf5, 0xe3, 0x63, 0xae, 0x6b}}, 0,
		0, 0, ECMIntegral, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolSelect>, NULL
	},
	{
		L"MAGICWAND", IDS_TOOLNAME_MAGICWAND, IDS_TOOLDESC_MAGICWAND,
		{0xcf09b5ae, 0x1a89, 0x46d6, {0x80, 0xa2, 0x20, 0xf1, 0xa4, 0xd4, 0x60, 0x33}}, 0,
		0, 0, ECMIntegral, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolMagicWand>, &CreateToolWindow<CEditToolMagicWandDlg>
	},
	{
		L"TRANSFORM", IDS_TOOLNAME_TRANSFORMATION, IDS_TOOLDESC_TRANSFORMATION,
		{0x79135c40, 0x71cf, 0x4d2d, {0x9f, 0x5d, 0xbc, 0xde, 0x58, 0x54, 0xa7, 0xbb}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSNoPaint, TRUE,
		&CreateTool<CEditToolTransformation>, &CreateToolWindow<CEditToolTransformationDlg>
	},
	{
		L"MOVE", IDS_TOOLNAME_MOVE, IDS_TOOLDESC_MOVE,
		{0xa177c595, 0xaad4, 0x4b76, {0x96, 0x23, 0x7e, 0xb1, 0x89, 0xfc, 0xa5, 0x78}}, 0,
		0, 0, ECMIntegral, ETPSNoPaint, FALSE,
		&CreateTool<CEditToolMove>, &CreateToolWindow<CEditToolMoveDlg>
	},
};


// CRasterImageEditToolsSelection

class ATL_NO_VTABLE CRasterImageEditToolsSelection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsSelection, &CLSID_RasterImageEditToolsSelection>,
	public CRasterImageEditToolsFactoryImpl<g_aSelectionTools, sizeof(g_aSelectionTools)/sizeof(g_aSelectionTools[0])>
{
public:
	typedef CRasterImageEditToolsFactoryImpl<g_aSelectionTools, sizeof(g_aSelectionTools)/sizeof(g_aSelectionTools[0])> CImpl;

	CRasterImageEditToolsSelection()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsSelection)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsSelection)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsSelection)
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

	static HICON CreateSelectIcon(HICON a_hSource);

private:
	CIconIds m_cIconIds;
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsSelection), CRasterImageEditToolsSelection)
