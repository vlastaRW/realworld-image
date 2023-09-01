// RasterImageEditToolsText.h : Declaration of the CRasterImageEditToolsText

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageEditToolsFactoryImpl.h"

#include "EditToolGDIText.h"
#include "EditToolText.h"
//#include "EditToolArtisticText.h"


extern __declspec(selectany) SToolSpec const g_aTextTools[] =
{
	{
		L"GDITEXT", IDS_TOOLNAME_TEXT, IDS_TOOLDESC_TEXT,
		{0x49b2f4c3, 0x2fb0, 0x4640, {0x9b, 0xaa, 0xca, 0x86, 0xeb, 0x2f, 0x49, 0xb9}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMSmooth|ERMBinary, ECMIntegral, ETPSPatternFill, FALSE,
		&CreateTool<CEditToolGDIText>, &CreateToolWindow<CEditToolTextDlg>
	},
	{
		L"TEXT", IDS_TOOLNAME_TEXT, IDS_TOOLDESC_TEXT,
		{0x49b2f4c3, 0x2fb0, 0x4640, {0x9b, 0xaa, 0xca, 0x86, 0xeb, 0x2f, 0x49, 0xb9}}, 0,
		EBMDrawOver|EBMReplace|EBMDrawUnder, ERMSmooth|ERMBinary, ECMFloatingPoint|ECMIntegral, ETPSSolidOutlineAndPatternFill, FALSE,
		&CreateTool<CEditToolText>, &CreateToolWindow<CEditToolTextDlg>
	},
	//{
	//	L"TEXTART", IDS_TOOLNAME_TEXT, IDS_TOOLNAME_TEXT,
	//	{0x797503e, 0x7f85, 0x4793, {0x82, 0x22, 0x47, 0x7c, 0xeb, 0x9b, 0x7e, 0x34}}, IDI_EDITTOOL_TEXTART,
	//	EBMDrawOver|EBMReplace|EBMDrawUnder, ERMSmooth|ERMBinary, ECMFloatingPoint|ECMIntegral, ETPSPatternFill, FALSE,
	//	&CreateTool<CEditToolArtisticText>, NULL
	//},
};



// CRasterImageEditToolsText

class ATL_NO_VTABLE CRasterImageEditToolsText :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsText, &CLSID_RasterImageEditToolsText>,
	public CRasterImageEditToolsFactoryImpl<g_aTextTools, sizeof(g_aTextTools)/sizeof(g_aTextTools[0])>
{
public:
	CRasterImageEditToolsText()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsText)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsText)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsText)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
END_COM_MAP()

	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageEditToolsText), CRasterImageEditToolsText)
