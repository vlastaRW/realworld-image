// SharedStateImageViewport.h : Declaration of the CSharedStateImageViewport

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImage.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CSharedStateImageViewport

class ATL_NO_VTABLE CSharedStateImageViewport :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateImageViewport, &CLSID_SharedStateImageViewport>,
	public ISharedStateImageViewport
{
public:
	CSharedStateImageViewport() :
		m_fCenterX(0.0f), m_fCenterY(0.0f), m_fZoom(1.0f), m_nSizeX(0), m_nSizeY(0),
		m_nImageX(0), m_nImageY(0), m_bAutoZoom(true)
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateImageViewport)
	COM_INTERFACE_ENTRY(ISharedStateImageViewport)
	COM_INTERFACE_ENTRY(ISharedState)
END_COM_MAP()


	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		*a_pCLSID = GetObjectCLSID();
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText)
	{
		try
		{
			*a_pbstrText = NULL;
			wchar_t szTmp[128];
			swprintf(szTmp, L"%g,%g,%g,%i,%i", m_fCenterX, m_fCenterY, m_fZoom, m_nSizeX, m_nSizeY);
			*a_pbstrText = SysAllocString(szTmp);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		if (a_bstrText == NULL)
			return E_RW_INVALIDPARAM;
		swscanf(a_bstrText, L"%g,%g,%g,%i,%i", &m_fCenterX, &m_fCenterY, &m_fZoom, &m_nSizeX, &m_nSizeY);
		return S_OK;
	}

	// ISharedStateImageViewport methods
public:
	STDMETHOD(Init)(float a_fCenterX, float a_fCenterY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY)
	{
		m_fCenterX = a_fCenterX;
		m_fCenterY = a_fCenterY;
		m_fZoom = a_fZoom;
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		m_nImageX = m_nImageY = 0;
		m_bAutoZoom = false;
		return S_OK;
	}
	STDMETHOD(InitEx)(float a_fCenterX, float a_fCenterY, float a_fZoom, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nImageX, ULONG a_nImageY, boolean a_bAutoZoom)
	{
		m_fCenterX = a_fCenterX;
		m_fCenterY = a_fCenterY;
		m_fZoom = a_fZoom;
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		m_nImageX = a_nImageX;
		m_nImageY = a_nImageY;
		m_bAutoZoom = a_bAutoZoom;
		return S_OK;
	}
	STDMETHOD(Get)(float* a_pCenterX, float* a_pCenterY, float* a_pZoom, ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		try
		{
			if (a_pCenterX)
				*a_pCenterX = m_fCenterX;
			if (a_pCenterY)
				*a_pCenterY = m_fCenterY;
			if (a_pZoom)
				*a_pZoom = m_fZoom;
			if (a_pSizeX)
				*a_pSizeX = m_nSizeX;
			if (a_pSizeY)
				*a_pSizeY = m_nSizeY;
			return S_OK; 
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetEx)(float* a_pCenterX, float* a_pCenterY, float* a_pZoom, ULONG* a_pSizeX, ULONG* a_pSizeY, ULONG* a_pImageX, ULONG* a_pImageY, boolean* a_pAutoZoom)
	{
		try
		{
			if (a_pCenterX)
				*a_pCenterX = m_fCenterX;
			if (a_pCenterY)
				*a_pCenterY = m_fCenterY;
			if (a_pZoom)
				*a_pZoom = m_fZoom;
			if (a_pSizeX)
				*a_pSizeX = m_nSizeX;
			if (a_pSizeY)
				*a_pSizeY = m_nSizeY;
			if (a_pImageX)
				*a_pImageX = m_nImageX;
			if (a_pImageY)
				*a_pImageY = m_nImageY;
			if (a_pAutoZoom)
				*a_pAutoZoom = m_bAutoZoom;
			return S_OK; 
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	float m_fCenterX;
	float m_fCenterY;
	float m_fZoom;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	ULONG m_nImageX;
	ULONG m_nImageY;
	boolean m_bAutoZoom;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateImageViewport), CSharedStateImageViewport)
