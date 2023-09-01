// RasterImageEditToolsDrawing.h : Declaration of the CRasterImageEditToolsDrawing

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageEditToolsFactoryImpl.h"

#include "EditToolPencil.h"
#include "EditToolBrush.h"
#include "EditToolLasso.h"


extern __declspec(selectany) SToolSpec const g_aDrawingTools[] =
{
	{
		L"PENCIL", IDS_TOOLNAME_PENCIL, IDS_TOOLDESC_PENCIL,
		{0x14af4b9e, 0x6aaa, 0x4938, {0x83, 0xe7, 0xb0, 0x61, 0xe6, 0x5e, 0x60, 0x9d}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMBinary, ECMIntegral, ETPSSolidFill, FALSE,
		&CreateTool<CEditToolPencil>, &CreateToolWindow<CEditToolPencilDlg>
	},
	{
		L"BRUSH", IDS_TOOLNAME_BRUSH, IDS_TOOLDESC_BRUSH,
		{0x97edc533, 0x90df, 0x496c, {0xaf, 0x8f, 0xa0, 0xab, 0xba, 0x81, 0xdd, 0x1e}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMSmooth, ECMIntegral|ECMFloatingPoint, ETPSSolidFill, FALSE,
		&CreateTool<CEditToolBrush>, &CreateToolWindow<CEditToolBrushDlg>
	},
	{
		L"LASSO", IDS_TOOLNAME_LASSO, IDS_TOOLDESC_LASSO,
		{0xdaf3f35c, 0x5ded, 0x4dea, {0xb6, 0xb4, 0x33, 0x32, 0x06, 0x0a, 0x8b, 0x6e}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMSmooth|ERMBinary, ECMIntegral|ECMFloatingPoint, ETPSPatternFill, TRUE,
		&CreateTool<CEditToolLasso>, NULL//&CreateToolWindow<CEditToolBrushDlg>
	},
};


// CRasterImageEditToolsDrawing

class ATL_NO_VTABLE CRasterImageEditToolsDrawing :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsDrawing, &CLSID_RasterImageEditToolsDrawing>,
	public CRasterImageEditToolsFactoryImpl<g_aDrawingTools, sizeof(g_aDrawingTools)/sizeof(g_aDrawingTools[0])>
{
public:
	CRasterImageEditToolsDrawing()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsDrawing)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsDrawing)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsDrawing)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
END_COM_MAP()

	// IRasterImageEditToolsFactory methods
public:
	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsDrawing), CRasterImageEditToolsDrawing)
