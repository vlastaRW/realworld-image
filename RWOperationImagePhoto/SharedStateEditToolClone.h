// SharedStateEditToolClone.h : Declaration of the CSharedStateEditToolClone

#pragma once
#include "resource.h"       // main symbols

#include "RWOperationImagePhoto.h"



// CSharedStateEditToolClone

class ATL_NO_VTABLE CSharedStateEditToolClone :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateEditToolClone, &CLSID_SharedStateEditToolClone>,
	public CEditToolDataClone::ISharedStateToolData
{
public:
	CSharedStateEditToolClone()
	{
	}

	void Init(CEditToolDataClone const& a_cData)
	{
		m_cData = a_cData;
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateEditToolClone)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(CEditToolDataClone::ISharedStateToolData)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		*a_pCLSID = CLSID_SharedStateEditToolClone;
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText)
	{
		try
		{
			*a_pbstrText = NULL;
			return m_cData.ToString(a_pbstrText);
		}
		catch (...)
		{
			return a_pbstrText ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		try
		{
			return m_cData.FromString(a_bstrText);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// ISharedStateToolData methods
public:
	STDMETHOD_(CEditToolDataClone const*, InternalData)()
	{
		return &m_cData;
	}

private:
	CEditToolDataClone m_cData;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateEditToolClone), CSharedStateEditToolClone)
