// RasterImageEditToolsManager.h : Declaration of the CRasterImageEditToolsManager

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include <WeakSingleton.h>
#include <DesignerFrameIconsImpl.h>


extern __declspec(selectany) UINT s_aToolIconResIDs[] =
{
	0,//IDI_EDITTOOL_TEXT,
	//IDI_EDITTOOL_TEXTART,
	0,//IDI_EDITTOOL_SHADOW,
	0,//IDI_EDITTOOL_DROPPER,
	0,//IDI_EDITTOOL_FLOODFILL,
	0,//IDI_EDITTOOL_LINE,
	0,//IDI_EDITTOOL_CURVE,
	0,//IDI_EDITTOOL_POLYGON,
	0,//IDI_EDITTOOL_ELLIPSE,
	0,//IDI_EDITTOOL_RECTANGLE,
	0,//IDI_EDITTOOL_MOVESELECT,
	0,//IDI_EDITTOOL_MAGICWAND,
	0,//IDI_EDITTOOL_TRANSFORMATION,
	0,//IDI_EDITTOOL_FREEHAND,
	0,//IDI_EDITTOOL_BRUSH,
	0,//IDI_EDITTOOL_RETOUCH,
	0,//IDI_EDITTOOL_FILL,
	IDI_EDITTOOL_ERASER,
	0,//IDI_EDITTOOL_MOVE,
	0,//IDI_EDITTOOL_SHAPE,
};
extern __declspec(selectany) GUID s_aToolIconUIDs[] =
{
	{0x49b2f4c3, 0x2fb0, 0x4640, {0x9b, 0xaa, 0xca, 0x86, 0xeb, 0x2f, 0x49, 0xb9}},
	//{0x797503e, 0x7f85, 0x4793, {0x82, 0x22, 0x47, 0x7c, 0xeb, 0x9b, 0x7e, 0x34}},
	{0xbef1d7e1, 0x2c08, 0x4641, {0xa3, 0x3a, 0xeb, 0x1e, 0xb1, 0xb3, 0x44, 0xb3}},
	{0x9cc3272b, 0xaa0b, 0x460e, {0xb1, 0x89, 0xa6, 0xfb, 0xea, 0xd8, 0xab, 0x3c}},
	{0xafdf1173, 0x43de, 0x4a86, {0xb0, 0x27, 0x90, 0xc4, 0xc5, 0x89, 0x5b, 0x54}},
	{0xa9b160ef, 0x9d13, 0x4f31, {0xb2, 0xb7, 0x1f, 0x8a, 0xd8, 0xc9, 0xbb, 0x46}},
	{0xb3e48357, 0x41d1, 0x4e0c, {0xb6, 0x5b, 0x3e, 0x09, 0x3f, 0x2b, 0x3e, 0x4f}},
	{0x2391bacb, 0x1d6d, 0x481b, {0xb2, 0xf3, 0x58, 0x13, 0x56, 0x72, 0xae, 0xf9}},
	{0xf3789d1d, 0x3be2, 0x4ab6, {0xb7, 0x9d, 0x7a, 0x08, 0x71, 0x65, 0x4c, 0x06}},
	{0xdcb3185c, 0xdb6a, 0x42d0, {0x80, 0x91, 0x2b, 0x25, 0x38, 0xd2, 0x79, 0x2d}},
	{0xb33b0053, 0x7a01, 0x4504, {0x92, 0x45, 0x01, 0xf5, 0xe3, 0x63, 0xae, 0x6b}},
	{0xcf09b5ae, 0x1a89, 0x46d6, {0x80, 0xa2, 0x20, 0xf1, 0xa4, 0xd4, 0x60, 0x33}},
	{0x79135c40, 0x71cf, 0x4d2d, {0x9f, 0x5d, 0xbc, 0xde, 0x58, 0x54, 0xa7, 0xbb}},
	{0x14af4b9e, 0x6aaa, 0x4938, {0x83, 0xe7, 0xb0, 0x61, 0xe6, 0x5e, 0x60, 0x9d}},
	{0x97edc533, 0x90df, 0x496c, {0xaf, 0x8f, 0xa0, 0xab, 0xba, 0x81, 0xdd, 0x1e}},
	{0x01aef148, 0xc1f1, 0x45c1, {0x93, 0x99, 0x70, 0xb3, 0x8f, 0xe8, 0x43, 0xfb}},
	{0x20a1b32e, 0x9440, 0x428a, {0x85, 0x2e, 0x3a, 0xac, 0x55, 0x07, 0x24, 0xd2}},
	{0xdf356912, 0x8111, 0x44b0, {0x81, 0xa8, 0xa4, 0xf2, 0x30, 0x0a, 0x50, 0xf4}},
	{0xa177c595, 0xaad4, 0x4b76, {0x96, 0x23, 0x7e, 0xb1, 0x89, 0xfc, 0xa5, 0x78}},
	{0x30ce9e6f, 0x1689, 0x4dd8, {0xb3, 0x19, 0x9b, 0x7a, 0x19, 0x01, 0xd5, 0xdd}},
};
HICON GetToolIconTEXT(ULONG a_nSize);
HICON GetToolIconPSHADOW(ULONG a_nSize);
HICON GetToolIconDROPPER(ULONG a_nSize);
HICON GetToolIconFLOODFILL(ULONG a_nSize);
HICON GetToolIconELLIPSE(ULONG a_nSize);
HICON GetToolIconRECTANGLE(ULONG a_nSize);
HICON GetToolIconPOLYGON(ULONG a_nSize);
HICON GetToolIconSHAPE(ULONG a_nSize);
HICON GetToolIconLINE(ULONG a_nSize);
HICON GetToolIconCURVE(ULONG a_nSize);
HICON GetToolIconSTROKE(ULONG a_nSize);
HICON GetToolIconMAGICWAND(ULONG a_nSize);
HICON GetToolIconTRANSFORM(ULONG a_nSize);
HICON GetToolIconPENCIL(ULONG a_nSize);
HICON GetToolIconBRUSH(ULONG a_nSize);
HICON GetToolIconRETOUCH(ULONG a_nSize);
HICON GetToolIconFILL(ULONG a_nSize);
HICON GetToolIconMOVE(ULONG a_nSize);
HICON GetToolIconSIMPLESELECT(ULONG a_nSize);
extern __declspec(selectany) pfnGetDFIcon s_aToolGetIconFuncs[] =
{
	GetToolIconTEXT,//IDI_EDITTOOL_TEXT,
	//IDI_EDITTOOL_TEXTART,
	GetToolIconPSHADOW,//IDI_EDITTOOL_SHADOW,
	GetToolIconDROPPER,//IDI_EDITTOOL_DROPPER,
	GetToolIconFLOODFILL,//IDI_EDITTOOL_FLOODFILL,
	GetToolIconLINE,//IDI_EDITTOOL_LINE,
	GetToolIconCURVE,//IDI_EDITTOOL_CURVE,
	GetToolIconPOLYGON,//IDI_EDITTOOL_POLYGON,
	GetToolIconELLIPSE,//IDI_EDITTOOL_ELLIPSE,
	GetToolIconRECTANGLE,//IDI_EDITTOOL_RECTANGLE,
	GetToolIconSIMPLESELECT,//IDI_EDITTOOL_MOVESELECT,
	GetToolIconMAGICWAND,//IDI_EDITTOOL_MAGICWAND,
	GetToolIconTRANSFORM,//IDI_EDITTOOL_TRANSFORMATION,
	GetToolIconPENCIL,//IDI_EDITTOOL_FREEHAND,
	GetToolIconBRUSH,//IDI_EDITTOOL_BRUSH,
	GetToolIconRETOUCH,//IDI_EDITTOOL_RETOUCH,
	GetToolIconFILL,//IDI_EDITTOOL_FILL,
	NULL,//IDI_EDITTOOL_ERASER,
	GetToolIconMOVE,//IDI_EDITTOOL_MOVE,
	GetToolIconSHAPE,//IDI_EDITTOOL_SHAPE,
};


// CRasterImageEditToolsManager

class ATL_NO_VTABLE CRasterImageEditToolsManager :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsManager, &CLSID_RasterImageEditToolsManager>,
	public IRasterImageEditToolsManager,
	public CDesignerFrameIconsImpl<sizeof(s_aToolIconUIDs)/sizeof(s_aToolIconUIDs[0]), s_aToolIconUIDs, s_aToolIconResIDs, s_aToolGetIconFuncs>
{
public:
	CRasterImageEditToolsManager() : m_nTimeStamp(0)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CRasterImageEditToolsManager)

BEGIN_CATEGORY_MAP(CMenuCommandsDrawingTools)
	IMPLEMENTED_CATEGORY(CATID_DesignerFrameIcons)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsManager)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsManager)
	COM_INTERFACE_ENTRY(IDesignerFrameIcons)
END_COM_MAP()


	// IRasterImageEditToolsManager methods
public:
	STDMETHOD(ToolIDsEnum)(IEnumStrings** a_ppToolIDs);
	STDMETHOD(ToolNameGet)(BSTR a_bstrID, ILocalizedString** a_ppName);
	STDMETHOD(ToolDescGet)(BSTR a_bstrID, ILocalizedString** a_ppDesc);
	STDMETHOD(ToolIconIDGet)(BSTR a_bstrID, GUID* a_ptDefaultIcon);
	STDMETHOD(ToolIconGet)(BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(SupportedStates)(BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs);
	STDMETHOD(EditToolCreate)(BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool);
	STDMETHOD(WindowCreate)(BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow);

private:
	struct SPrefixLess
	{
		bool operator()(CComBSTR const& a_1, CComBSTR const& a_2) const
		{
			LPCWSTR p1 = a_1;
			LPCWSTR p2 = a_2;
			if (p1 == NULL)
				return p2;
			if (p2 == NULL)
				return false;
			while (*p1 == *p2 && *p1) {++p1; ++p2;}
			if (p1 > a_1.m_str && p1[-1] == L'_')
				if (*p1 == L'\0' || *p2 == L'\0')
					return false;
			return *p1 < *p2;
		}
	};

	typedef std::map<CComBSTR, CComPtr<IRasterImageEditToolsFactory>, SPrefixLess> CFactoryMap;

private:
	void CheckState();

private:
	CComPtr<IEnumStringsInit> m_pToolIDs;
	CFactoryMap m_cFactoryMap;
	ULONG m_nTimeStamp;
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsManager), CRasterImageEditToolsManager)
