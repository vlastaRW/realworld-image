// RasterImageEditToolsSpecial.h : Declaration of the CRasterImageEditToolsSpecial

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageEditToolsFactoryImpl.h"

#include "EditToolShadow.h"
#include "EditToolDropper.h"
#include "EditToolFloodFill.h"
#include "EditToolFill.h"


extern __declspec(selectany) SToolSpec const g_aSpecialTools[] =
{
	{
		L"PSHADOW", IDS_TOOLNAME_PSHADOW, IDS_TOOLDESC_PSHADOW,
		{0xbef1d7e1, 0x2c08, 0x4641, {0xa3, 0x3a, 0xeb, 0x1e, 0xb1, 0xb3, 0x44, 0xb3}}, 0,
		0, 0, 0, ETPSNoPaint/*ETPSSolidFill*/, FALSE,
		&CreateTool<CEditToolShadow>, &CreateToolWindow<CEditToolShadowDlg>
	},
	{
		L"DROPPER", IDS_TOOLNAME_DROPPER, IDS_TOOLDESC_DROPPER,
		{0x9cc3272b, 0xaa0b, 0x460e, {0xb1, 0x89, 0xa6, 0xfb, 0xea, 0xd8, 0xab, 0x3c}}, 0,
		0, 0, 0, ETPSPatternFill, FALSE,
		&CreateTool<CEditToolDropper>, NULL
	},
	{
		L"FLOODFILL", IDS_TOOLNAME_FLOODFILL, IDS_TOOLDESC_FLOODFILL,
		{0xafdf1173, 0x43de, 0x4a86, {0xb0, 0x27, 0x90, 0xc4, 0xc5, 0x89, 0x5b, 0x54}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, 0, 0, ETPSPatternFill, FALSE,
		&CreateTool<CEditToolFloodFill>, &CreateToolWindow<CEditToolFloodFillDlg>
	},
	{
		L"FILL", IDS_TOOLNAME_FILL, IDS_TOOLDESC_FILL,
		/*{0x76f44ed2, 0xd0b4, 0x4047, {0x98, 0x5d, 0x2f, 0xa6, 0x35, 0x6a, 0x1b, 0xf9}}*/{0x20a1b32e, 0x9440, 0x428a, {0x85, 0x2e, 0x3a, 0xac, 0x55, 0x07, 0x24, 0xd2}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, 0, 0, ETPSPatternFill, FALSE,
		&CreateTool<CEditToolFill>, NULL
	},
};


// CRasterImageEditToolsSpecial

class ATL_NO_VTABLE CRasterImageEditToolsSpecial :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsSpecial, &CLSID_RasterImageEditToolsSpecial>,
	public CRasterImageEditToolsFactoryImpl<g_aSpecialTools, sizeof(g_aSpecialTools)/sizeof(g_aSpecialTools[0])>
{
public:
	CRasterImageEditToolsSpecial()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsSpecial)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsSpecial)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsSpecial)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
END_COM_MAP()

	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsSpecial), CRasterImageEditToolsSpecial)
