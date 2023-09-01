// SharedStateImageSelection.h : Declaration of the CSharedStateImageSelection

#pragma once
#include "RWDocumentImageRaster.h"


// CSharedStateImageSelection

class ATL_NO_VTABLE CSharedStateImageSelection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateImageSelection, &CLSID_SharedStateImageSelection>,
	public ISharedStateImageSelection
{
public:
	CSharedStateImageSelection()
	{
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateImageSelection)
	COM_INTERFACE_ENTRY(ISharedStateImageSelection)
END_COM_MAP()


	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID *a_pCLSID)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ToText)(BSTR *a_pbstrText)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		return E_NOTIMPL;
	}

	// ISharedStateImageSelection methods
public:
	STDMETHOD(Init)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE const* a_pData);
	STDMETHOD(IsEmpty)();
	STDMETHOD(Bounds)(LONG* a_pX, LONG* a_pY, ULONG* a_pSizeX, ULONG* a_pSizeY);
	STDMETHOD(GetTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pData);

private:
	LONG m_nX;
	LONG m_nY;
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	CAutoVectorPtr<BYTE> m_pData;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateImageSelection), CSharedStateImageSelection)
