// RasterImageEditToolsShapes.h : Declaration of the CRasterImageEditToolsShapes

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageEditToolsFactoryImpl.h"

#include "EditToolLine.h"
#include "EditToolCurve.h"
#include "EditToolPolygon.h"
#include "EditToolEllipse.h"
#include "EditToolRectangle.h"
#include "EditToolShape.h"
#include "EditToolShapeDlg.h"
#include "EditToolStroke.h"
#include "EditToolStrokeDlg.h"


extern __declspec(selectany) SToolSpec const g_aShapeTools[] =
{
	{
		L"LINE", IDS_TOOLNAME_LINE, IDS_TOOLDESC_LINE,
		{0xa9b160ef, 0x9d13, 0x4f31, {0xb2, 0xb7, 0x1f, 0x8a, 0xd8, 0xc9, 0xbb, 0x46}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSPatternFill, TRUE,
		&CreateTool<CEditToolLine>, &CreateToolWindow<CEditToolLineDlg>
	},
	{
		L"CURVE", IDS_TOOLNAME_CURVE, IDS_TOOLDESC_CURVE,
		{0xb3e48357, 0x41d1, 0x4e0c, {0xb6, 0x5b, 0x3e, 0x9, 0x3f, 0x2b, 0x3e, 0x4f}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSPatternFill, TRUE,
		&CreateTool<CEditToolCurve>, &CreateToolWindow<CEditToolCurveDlg>
	},
	{
		L"POLYGON", IDS_TOOLNAME_POLYGON, IDS_TOOLDESC_POLYGON,
		{0x2391bacb, 0x1d6d, 0x481b, {0xb2, 0xf3, 0x58, 0x13, 0x56, 0x72, 0xae, 0xf9}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, TRUE,
		&CreateTool<CEditToolPolygon>, &CreateToolWindow<CEditToolPolygonDlg>
	},
	{
		L"ELLIPSE", IDS_TOOLNAME_ELLIPSE, IDS_TOOLDESC_ELLIPSE,
		{0xf3789d1d, 0x3be2, 0x4ab6, {0xb7, 0x9d, 0x7a, 0x8, 0x71, 0x65, 0x4c, 0x6}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, TRUE,
		&CreateTool<CEditToolEllipse>, &CreateToolWindow<CEditToolEllipseDlg>
	},
	{
		L"RECTANGLE", IDS_TOOLNAME_RECTANGLE, IDS_TOOLDESC_RECTANGLE,
		{0xdcb3185c, 0xdb6a, 0x42d0, {0x80, 0x91, 0x2b, 0x25, 0x38, 0xd2, 0x79, 0x2d}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, TRUE,
		&CreateTool<CEditToolRectangle>, &CreateToolWindow<CEditToolRectangleDlg>
	},
	{
		L"SHAPE", L"[0409]Shape[0405]Tvar", L"[0409]Draw an arbitrary shape controlled by points with tangents.[0405]Nakreslit libovolný tvar určený pomocí bodů a tangent.",
		{0x30ce9e6f, 0x1689, 0x4dd8, {0xb3, 0x19, 0x9b, 0x7a, 0x19, 0x01, 0xd5, 0xdd}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, TRUE,
		&CreateTool<CEditToolPath>, &CreateToolWindow<CEditToolShapeDlg>
	},
	{
		L"STROKE", L"[0409]Stroke[0405]Tah", L"[0409]Draw an arbitrary shape controlled by points with tangents.[0405]Nakreslit libovolný tvar určený pomocí bodů a tangent.",
		{0xaa4e8806, 0x48bf, 0x492f, {0xb4, 0x57, 0x26, 0x1f, 0xe6, 0x35, 0xf3, 0x5c}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder|EBMAdd, ERMBinary|ERMSmooth, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, TRUE,
		&CreateTool<CEditToolStroke>, &CreateToolWindow<CEditToolStrokeDlg>
	},
};


// CRasterImageEditToolsShapes

class ATL_NO_VTABLE CRasterImageEditToolsShapes :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsShapes, &CLSID_RasterImageEditToolsShapes>,
	public CRasterImageEditToolsFactoryImpl<g_aShapeTools, sizeof(g_aShapeTools)/sizeof(g_aShapeTools[0])>,
	public IBezierPathUtils
{
public:
	CRasterImageEditToolsShapes()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsShapes)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsShapes)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsShapes)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
	COM_INTERFACE_ENTRY(IBezierPathUtils)
END_COM_MAP()


	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);

	// IBezierPathUtils methods
public:
	STDMETHOD(SVGToPolygon)(BSTR a_bstrSVG, IRasterImageEditToolPolygon* a_pConsumer);
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsShapes), CRasterImageEditToolsShapes)
