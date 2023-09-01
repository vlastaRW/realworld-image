// SharedStateToolData.h : Declaration of the CSharedStateToolData

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"

struct ISharedStateToolData : public ISharedState
{
	STDMETHOD_(void const*, InternalData)() = 0;
};

// CSharedStateToolData

class ATL_NO_VTABLE CSharedStateToolData :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateToolData, &CLSID_SharedStateToolData>,
	public ISharedStateToolData
{
public:
	CSharedStateToolData();
	~CSharedStateToolData();

	bool Init(void const* a_pData, REFIID a_iid);

	template<class TData>
	bool Init(TData const& a_cData)
	{
		return Init(&a_cData, __uuidof(TData::ISharedStateToolData));
	}

DECLARE_NO_REGISTRY()


	static HRESULT WINAPI QIToolData(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw);

BEGIN_COM_MAP(CSharedStateToolData)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY_FUNC_BLIND(0, QIToolData)
END_COM_MAP()


	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		*a_pCLSID = CLSID_SharedStateToolData;
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText);
	STDMETHOD(FromText)(BSTR a_bstrText);

	// ISharedStateToolData methods
public:
    STDMETHOD_(void const*, InternalData)()
	{
		return m_pData;
	}

private:
	ULONG m_nType;
	void* m_pData;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateToolData), CSharedStateToolData)
