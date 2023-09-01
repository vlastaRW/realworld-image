
#pragma once


class CSelectionBlockingOperationContext :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IOperationContext
{
public:
	void Init(BSTR a_bstrSyncID, IOperationContext* a_pStates)
	{
		m_bstrID = a_bstrSyncID;
		m_pStates = a_pStates;
	}

BEGIN_COM_MAP(CSelectionBlockingOperationContext)
	COM_INTERFACE_ENTRY(IOperationContext)
END_COM_MAP()

	// IOperationContext methods
public:
	STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
	{
		if (m_bstrID == a_bstrCategoryName)
			return E_RW_ITEMNOTFOUND;
		return m_pStates->StateGet(a_bstrCategoryName, a_iid, a_ppState);
	}
	STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
	{
		if (m_bstrID == a_bstrCategoryName)
			return E_FAIL;
		return m_pStates->StateSet(a_bstrCategoryName, a_pState);
	}
	STDMETHOD(IsCancelled)()
	{
		return m_pStates->IsCancelled();
	}
	STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
	{
		return m_pStates->GetOperationInfo(a_pItemIndex, a_pItemsRemaining, a_pStepIndex, a_pStepsRemaining);
	}
	STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
	{
		return m_pStates->SetErrorMessage(a_pMessage);
	}

private:
	CComBSTR m_bstrID;
	CComPtr<IOperationContext> m_pStates;
};

