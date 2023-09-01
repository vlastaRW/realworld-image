// RasterImageFillStyleBasic.h : Declaration of the CRasterImageFillStyleBasic

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageFillStyleFactoryImpl.h"

#include "FillStyleSolid.h"
#include "FillStylePattern.h"

extern __declspec(selectany) SFillSpec const g_aBasicFills[] =
{
	{
		L"SOLID", IDS_FILLSTYLE_SOLID_NAME, IDS_FILLSTYLE_SOLID_NAME,
		{0x12c96d6b, 0x3e6a, 0x44f7, {0xbb, 0xc8, 0x53, 0xe1, 0xd, 0x7b, 0xca, 0x88}}, IDI_FILLSTYLE_SOLID,
		&CreateFill<CFillStyleSolid>, &CreateFillWindow<CFillStyleSolidDlg>
	},
	{
		L"PATTERN", IDS_FILLSTYLE_PATTERN_NAME, IDS_FILLSTYLE_PATTERN_NAME,
		{0x9db66017, 0x214b, 0x454f, {0x8e, 0x1c, 0xc5, 0x66, 0x4a, 0x2f, 0xcb, 0xcb}}, IDI_FILLSTYLE_PATTERN,
		&CreateFill<CFillStylePattern>, &CreateFillWindow<CFillStylePatternDlg>
	},
};


// CRasterImageFillStyleBasic

class ATL_NO_VTABLE CRasterImageFillStyleBasic :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageFillStyleBasic, &CLSID_RasterImageFillStyleBasic>,
	public CRasterImageFillStyleFactoryImpl<g_aBasicFills, sizeof(g_aBasicFills)/sizeof(g_aBasicFills[0])>
{
public:
	CRasterImageFillStyleBasic()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageFillStyleBasic)

BEGIN_CATEGORY_MAP(CRasterImageFillStyleBasic)
	IMPLEMENTED_CATEGORY(CATID_RasterImageFillStyleFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageFillStyleBasic)
	COM_INTERFACE_ENTRY(IRasterImageFillStyleFactory)
END_COM_MAP()

};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageFillStyleBasic), CRasterImageFillStyleBasic)
