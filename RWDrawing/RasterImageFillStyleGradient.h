// RasterImageFillStyleGradient.h : Declaration of the CRasterImageFillStyleGradient

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include "RasterImageFillStyleFactoryImpl.h"

#include "FillStyleLinear.h"
#include "FillStyleRadial.h"
#include "FillStyleConical.h"
//#include "FillStyleShape.h"
#include "FillStyleDiamond.h"
#include "FillStyleSphere.h"

extern __declspec(selectany) SFillSpec const g_aGradientFills[] =
{
	{
		L"LINEAR", IDS_FILLSTYLE_LINEAR_NAME, IDS_FILLSTYLE_LINEAR_NAME,
		{0xf4c5d79f, 0xd3, 0x4f1d, {0xad, 0x38, 0x1c, 0xa5, 0xb8, 0x7a, 0xea, 0x60}}, IDI_FILLSTYLE_LINEAR,
		&CreateFill<CFillStyleLinear>, &CreateFillWindow<CFillStyleGradientDlg>
	},
	{
		L"RADIAL", IDS_FILLSTYLE_RADIAL_NAME, IDS_FILLSTYLE_RADIAL_NAME,
		{0x25814671, 0xffe, 0x41c9, {0x98, 0x28, 0x1d, 0x11, 0x85, 0xca, 0x8f, 0xf1}}, IDI_FILLSTYLE_RADIAL,
		&CreateFill<CFillStyleRadial>, &CreateFillWindow<CFillStyleGradientDlg>
	},
	{
		L"ANGULAR", IDS_FILLSTYLE_CONICAL_NAME, IDS_FILLSTYLE_CONICAL_NAME,
		{0xb8bf1f2c, 0x3aa0, 0x4c26, {0xbe, 0xa2, 0x72, 0x60, 0x91, 0xa1, 0x55, 0x31}}, IDI_FILLSTYLE_CONICAL,
		&CreateFill<CFillStyleConical>, &CreateFillWindow<CFillStyleGradientDlg>
	},
	//{
	//	L"SHAPE", IDS_FILLSTYLE_CONICAL_NAME, IDS_FILLSTYLE_CONICAL_NAME,
	//	{0xc7396adb, 0x1f5c, 0x404c, {0xbd, 0x75, 0x7e, 0x17, 0x7c, 0x06, 0xd6, 0xd5}}, IDI_FILLSTYLE_CONICAL,
	//	&CreateFill<CFillStyleShape>, &CreateFillWindow<CFillStyleGradientDlg>
	//},
	{
		L"DIAMOND", IDS_FILLSTYLE_DIAMOND_NAME, IDS_FILLSTYLE_DIAMOND_NAME,
		{0xc9903297, 0xfd3f, 0x4cc4, {0xb7, 0x7c, 0x48, 0xd1, 0xd2, 0xb7, 0x7b, 0x72}}, IDI_FILLSTYLE_DIAMOND,
		&CreateFill<CFillStyleDiamond>, &CreateFillWindow<CFillStyleDiamondDlg>
	},
	{
		L"SPHERE", IDS_FILLSTYLE_SPHERE_NAME, IDS_FILLSTYLE_SPHERE_NAME,
		{0xfa8c023e, 0x827d, 0x45f0, {0xb1, 0xba, 0xfc, 0x0b, 0x94, 0x19, 0x70, 0x21}}, IDI_FILLSTYLE_SPHERE,
		&CreateFill<CFillStyleSphere>, &CreateFillWindow<CFillStyleGradientDlg>
	},
};


// CRasterImageFillStyleGradient

class ATL_NO_VTABLE CRasterImageFillStyleGradient :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageFillStyleGradient, &CLSID_RasterImageFillStyleGradient>,
	public CRasterImageFillStyleFactoryImpl<g_aGradientFills, sizeof(g_aGradientFills)/sizeof(g_aGradientFills[0])>
{
public:
	CRasterImageFillStyleGradient()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageFillStyleGradient)

BEGIN_CATEGORY_MAP(CRasterImageFillStyleGradient)
	IMPLEMENTED_CATEGORY(CATID_RasterImageFillStyleFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageFillStyleGradient)
	COM_INTERFACE_ENTRY(IRasterImageFillStyleFactory)
END_COM_MAP()

};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageFillStyleGradient), CRasterImageFillStyleGradient)
