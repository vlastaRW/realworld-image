// SharedStateEditToolCrop.h : Declaration of the CSharedStateEditToolCrop

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImagePhoto.h"


struct CEditToolDataCrop
{
	MIDL_INTERFACE("FB4CDB9D-55D2-4CE6-B677-70567B8C80B1")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataCrop const*, InternalData)() = 0;
	};

	enum ECropMode
	{
		ECMPerspective = 0,
		ECMStandard,
		ECMLosslessJPEG
	};
	enum EProportionsMode
	{
		EPMArbitrary = 0,
		EPM1_1 = 1|(1<<16),
		EPM3_2 = 3|(2<<16),
		EPM4_3 = 4|(3<<16),
		EPM5_4 = 5|(4<<16),
		EPM16_10 = 16|(10<<16),
		EPM16_9 = 16|(9<<16),
	};

	CEditToolDataCrop() : eCropMode(ECMStandard), eProportionsMode(EPMArbitrary)
	{
	}
	HRESULT FromString(BSTR a_bstr)
	{
		int x = 0;
		int y = 0;
		swscanf(a_bstr, L"%i:%i", &x, &y);
		if (x && y)
			eProportionsMode = static_cast<EProportionsMode>(x|(y<<16));
		if (wcsstr(a_bstr, L"PERSPECTIVE"))
			eCropMode = ECMPerspective;
		else if (wcsstr(a_bstr, L"LOSSLESSJPEG"))
			eCropMode = ECMLosslessJPEG;
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		wchar_t szTmp[64] = L"";
		swprintf(szTmp, L"%i:%i|%s", int(eProportionsMode&0xffff), int(eProportionsMode>>16), eCropMode == ECMPerspective ? L"PERSPECTIVE" : (eCropMode == ECMLosslessJPEG ? L"LOSSLESSJPEG" : L"STANDARD"));
		*a_pbstr = SysAllocString(szTmp);
		return S_OK;
	}

	ECropMode eCropMode;
	EProportionsMode eProportionsMode;
};


// CSharedStateEditToolCrop

class ATL_NO_VTABLE CSharedStateEditToolCrop :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateEditToolCrop, &CLSID_SharedStateEditToolCrop>,
	public CEditToolDataCrop::ISharedStateToolData
{
public:
	CSharedStateEditToolCrop()
	{
	}

	void Init(CEditToolDataCrop const& a_cData)
	{
		m_cData = a_cData;
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateEditToolCrop)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(CEditToolDataCrop::ISharedStateToolData)
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
		*a_pCLSID = CLSID_SharedStateEditToolCrop;
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
	STDMETHOD_(CEditToolDataCrop const*, InternalData)()
	{
		return &m_cData;
	}

private:
	CEditToolDataCrop m_cData;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateEditToolCrop), CSharedStateEditToolCrop)
