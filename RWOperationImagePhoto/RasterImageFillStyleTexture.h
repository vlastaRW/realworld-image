// RasterImageFillStyleTexture.h : Declaration of the CRasterImageFillStyleTexture

#pragma once
#include "resource.h"       // main symbols

#include "RWOperationImagePhoto.h"
#include <RasterImageFillStyleFactoryImpl.h>

#include "FillStyleWood.h"
#include "FillStyleClouds.h"

extern __declspec(selectany) SFillSpec const g_aTextureFills[] =
{
	{
		L"WOOD", IDS_FILLSTYLE_WOOD_NAME, IDS_FILLSTYLE_WOOD_NAME,
		{0xd7d68157, 0xda1e, 0x4a53, {0x84, 0x17, 0xf7, 0xad, 0x84, 0x57, 0x80, 0xd7}}, IDI_FILLSTYLE_WOOD,
		&CreateFill<CFillStyleWood>, &CreateFillWindow<CFillStyleWoodDlg>
	},
	{
		L"CLOUDS", IDS_FILLSTYLE_CLOUDS_NAME, IDS_FILLSTYLE_CLOUDS_NAME,
		{0x29bddec3, 0xc802, 0x48de, {0xb7, 0x24, 0x2d, 0xa7, 0x68, 0xd3, 0x39, 0x69}}, IDI_FILLSTYLE_CLOUDS,
		&CreateFill<CFillStyleClouds>, &CreateFillWindow<CFillStyleCloudsDlg>
	},
};



// CRasterImageFillStyleTexture

class ATL_NO_VTABLE CRasterImageFillStyleTexture :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageFillStyleTexture, &CLSID_RasterImageFillStyleTexture>,
	public CRasterImageFillStyleFactoryImpl<g_aTextureFills, sizeof(g_aTextureFills)/sizeof(g_aTextureFills[0])>
{
public:
	CRasterImageFillStyleTexture()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageFillStyleTexture)

BEGIN_CATEGORY_MAP(CRasterImageFillStyleTexture)
	IMPLEMENTED_CATEGORY(CATID_RasterImageFillStyleFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageFillStyleTexture)
	COM_INTERFACE_ENTRY(IRasterImageFillStyleFactory)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:

};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageFillStyleTexture), CRasterImageFillStyleTexture)
