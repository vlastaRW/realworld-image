// ScriptedRasterImage.h : Declaration of the CScriptedRasterImage

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"

#include <RWDocumentImageRaster.h>


// CScriptedRasterImage

class ATL_NO_VTABLE CScriptedRasterImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedRasterImage, &IID_IScriptedRasterImage, &LIBID_RWOperationImageRasterLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedRasterImage()
	{
	}
	void Init(IDocument* a_pDoc, IDocumentRasterImage* a_pImage)
	{
		m_pDoc = a_pDoc;
		m_pImage = a_pImage;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedRasterImage)

BEGIN_COM_MAP(CScriptedRasterImage)
	COM_INTERFACE_ENTRY(IScriptedRasterImage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IScriptedRasterImage methods
public:
	STDMETHOD(GetPixelColor)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, OLE_COLOR* pixelColor);
	STDMETHOD(GetPixelAlpha)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelAlpha);
	STDMETHOD(GetPixelRed)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelRed);
	STDMETHOD(GetPixelGreen)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* pixelGreen);
	STDMETHOD(GetPixelBlue)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE* packedBlue);
	STDMETHOD(GetPixel)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, ULONG* packedPixel);
	STDMETHOD(SetPixelColor)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, OLE_COLOR pixelColor);
	STDMETHOD(SetPixelAlpha)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelAlpha);
	STDMETHOD(SetPixelRed)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelRed);
	STDMETHOD(SetPixelGreen)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelGreen);
	STDMETHOD(SetPixelBlue)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, BYTE pixelBlue);
	STDMETHOD(SetPixel)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, ULONG packedPixel);
	STDMETHOD(get_sizeX)(ULONG* pVal);
	STDMETHOD(get_sizeY)(ULONG* pVal);
	STDMETHOD(get_sizeZ)(ULONG* pVal);
	STDMETHOD(get_sizeW)(ULONG* pVal);
	STDMETHOD(Resize)(ULONG sizeX, ULONG sizeY, ULONG sizeZ, ULONG sizeW, LONG offsetX, LONG offsetY, LONG offsetZ, LONG offsetW)
	{ return FillResize(sizeX, sizeY, sizeZ, sizeW, offsetX, offsetY, offsetZ, offsetW, 0); }
	STDMETHOD(GetPixelH)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelH);
	STDMETHOD(GetPixelL)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelL);
	STDMETHOD(GetPixelS)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float* pixelS);
	STDMETHOD(SetPixelHLS)(ULONG posX, ULONG posY, ULONG posZ, ULONG posW, float pixelH, float pixelL, float pixelS);
	STDMETHOD(HasAlpha)(VARIANT_BOOL* alpha);
	STDMETHOD(SetAlpha)(VARIANT_BOOL alpha);
	STDMETHOD(FillResize)(ULONG sizeX, ULONG sizeY, ULONG sizeZ, ULONG sizeW, LONG offsetX, LONG offsetY, LONG offsetZ, LONG offsetW, ULONG fillColor);
	STDMETHOD(get_resolutionXNum)(ULONG* pVal);
	STDMETHOD(get_resolutionXDenom)(ULONG* pVal);
	STDMETHOD(get_resolutionYNum)(ULONG* pVal);
	STDMETHOD(get_resolutionYDenom)(ULONG* pVal);
	STDMETHOD(get_resolutionZNum)(ULONG* pVal);
	STDMETHOD(get_resolutionZDenom)(ULONG* pVal);
	STDMETHOD(get_resolutionWNum)(ULONG* pVal);
	STDMETHOD(get_resolutionWDenom)(ULONG* pVal);
	STDMETHOD(SetResolution)(ULONG a_nResXNum, ULONG a_nResXDenom, ULONG a_nResYNum, ULONG a_nResYDenom, ULONG a_nResZNum, ULONG a_nResZDenom, ULONG a_nResWNum, ULONG a_nResWDenom);
	STDMETHOD(get_previewScale)(ULONG* pVal);

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentRasterImage> m_pImage;
};

