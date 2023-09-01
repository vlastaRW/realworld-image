// SharedStateToolMode.h : Declaration of the CSharedStateToolMode

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif



// CSharedStateToolMode

class ATL_NO_VTABLE CSharedStateToolMode :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateToolMode, &CLSID_SharedStateToolMode>,
	public ISharedStateToolMode
{
public:
	CSharedStateToolMode() : m_bstrToolID(L"SELECT"), m_bstrStyleID(L"SOLID"),
		m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth),
		m_eCoordinatesMode(ECMFloatingPoint),
		m_bOutline(1.0f), m_fOutlineWidth(1.0f), m_fOutlinePos(-1.0f), m_eOutlineJoins(EOJTRound)
	{
		m_tOutlineColor.fR = m_tOutlineColor.fG = m_tOutlineColor.fB = 0.0f;
		m_tOutlineColor.fA = 1.0f;
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CSharedStateToolMode)
	COM_INTERFACE_ENTRY(ISharedStateToolMode)
	COM_INTERFACE_ENTRY(ISharedState)
END_COM_MAP()


	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		*a_pCLSID = CLSID_SharedStateToolMode;
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText);
	STDMETHOD(FromText)(BSTR a_bstrText);

	// ISharedStateToolMode
public:
	STDMETHOD(Get)(BSTR* a_pbstrToolID, BOOL* a_pFill, BSTR* a_pbstrStyleID, EBlendingMode* a_pBlendingMode, ERasterizationMode* a_pRasterizationMode, ECoordinatesMode* a_pCoordinatesMode, BOOL* a_pOutline, TColor* a_pOutlineColor, float* a_pOutlineWidth, float* a_pOutlinePos, EOutlineJoinType* a_pOutlineJoins);
	STDMETHOD(Set)(BSTR a_bstrToolID, BOOL const* a_pFill, BSTR a_bstrStyleID, EBlendingMode const* a_pBlendingMode, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, BOOL const* a_pOutline, TColor const* a_pOutlineColor, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, ISharedStateToolMode* a_pOldState);
	STDMETHOD(RemoveRecentTool)(BSTR a_bstrToolID);
	STDMETHOD(RemoveRecentFill)(BSTR a_bstrFillID);
	STDMETHOD(PickRecentTool)(ULONG a_nTools, BSTR* a_pbstrToolIDs, ULONG* a_pIndex);
	STDMETHOD(PickRecentFill)(ULONG a_nFills, BSTR* a_pbstrFillIDs, ULONG* a_pIndex);
	STDMETHOD(GetRecentIDs)(IEnumStrings** a_ppToolIDs, IEnumStrings** a_ppFillIDs);

private:
	typedef std::vector<CComBSTR> CStrings;

private:
	CComBSTR m_bstrToolID;
	BOOL m_bFill;
	CComBSTR m_bstrStyleID;
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
	BOOL m_bOutline;
	TColor m_tOutlineColor;
	float m_fOutlineWidth;
	float m_fOutlinePos;
	EOutlineJoinType m_eOutlineJoins;
	CStrings m_aRecentTools;
	CStrings m_aRecentFills;
};

OBJECT_ENTRY_AUTO(__uuidof(SharedStateToolMode), CSharedStateToolMode)
