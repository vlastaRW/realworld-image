// ScriptingInterfaceAnimation.cpp : Implementation of CScriptingInterfaceAnimation

#include "stdafx.h"
#include "ScriptingInterfaceAnimation.h"
#include "RWDocumentAnimationUtils.h"


// CScriptingInterfaceAnimation

STDMETHODIMP CScriptingInterfaceAnimation::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	return S_FALSE;
}

class ATL_NO_VTABLE CScriptedAnimation : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedAnimation, &IID_IScriptedAnimation, &LIBID_RWDocumentAnimationLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedAnimation()
	{
	}
	void Init(IScriptingInterfaceManager* a_pMgr, IDocument* a_pDoc, IDocumentAnimation* a_pDA)
	{
		m_pMgr = a_pMgr;
		m_pDoc = a_pDoc;
		m_pDA = a_pDA;
	}


DECLARE_NOT_AGGREGATABLE(CScriptedAnimation)

BEGIN_COM_MAP(CScriptedAnimation)
	COM_INTERFACE_ENTRY(IScriptedAnimation)
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

	// IScriptedLayeredImage methods
public:
	STDMETHOD(get_Frames)(ULONG* pnCount)
	{
		try
		{
			*pnCount = 0;
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			if (pItems)
				pItems->Size(pnCount);
			return S_OK;
		}
		catch (...)
		{
			return pnCount/*ppArray*/ ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetFrame)(ULONG index, IScriptedDocument** document)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			CComPtr<IUnknown> pItem;
			pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
			CComPtr<IDocument> pDoc;
			m_pDA->FrameGetDoc(pItem, &pDoc);
			return m_pMgr->WrapDocument(m_pMgr, pDoc, document);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(MoveFrame)(ULONG index, LONG before)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			CComPtr<IUnknown> pItem;
			pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
			CComPtr<IUnknown> pBefore;
			pItems->Get(before, __uuidof(IUnknown), reinterpret_cast<void**>(&pBefore));
			return m_pDA->FrameMove(pBefore, pItem);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DeleteFrame)(ULONG index)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			CComPtr<IUnknown> pItem;
			pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
			return m_pDA->FrameDel(pItem);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CopyToFrame)(LONG index, IUnknown* document)
	{
		try
		{
			CComQIPtr<IDocument> pDoc(document);
			CComPtr<IDocument> pDoc2;
			RWCoCreateInstance(pDoc2, __uuidof(DocumentBase));
			pDoc->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pDoc2), NULL, NULL); // copying due to possible read-write lock problem
			if (pDoc == NULL)
				return E_INVALIDARG;
			if (index >= 0)
			{
				CComPtr<IEnumUnknowns> pItems;
				m_pDA->FramesEnum(&pItems);
				CComPtr<IUnknown> pBefore;
				pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pBefore));
				return m_pDA->FrameSetDoc(pBefore, pDoc2);
			}
			else
			{
				return m_pDA->FrameIns(NULL, &CAnimationFrameCreatorDocument(pDoc2), NULL);
			}
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetFrameTime)(LONG index, double* ms)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			CComPtr<IUnknown> pFrame;
			pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame));
			float fTime = 0;
			if (pFrame == NULL || FAILED(m_pDA->FrameGetTime(pFrame, &fTime)))
				return E_FAIL;
			*ms = double(fTime)*1000.0;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetFrameTime)(LONG index, double ms)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			CComPtr<IUnknown> pFrame;
			pItems->Get(index, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame));
			ULONG nTime = 0;
			if (pFrame == NULL)
				return E_FAIL;
			return m_pDA->FrameSetTime(pFrame, ms >= 1.0 ? float(ms/1000.0) : 0.001f);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(get_LoopCount)(ULONG* pnCount)
	{
		return m_pDA->LoopCountGet(pnCount);
	}
	STDMETHOD(put_LoopCount)(ULONG count)
	{
		return m_pDA->LoopCountSet(count);
	}
	STDMETHOD(get_sizeX)(ULONG* pnVal)
	{
		try
		{
			*pnVal = GetAnimationSize(m_pDA).nX;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(get_sizeY)(ULONG* pnVal)
	{
		try
		{
			*pnVal = GetAnimationSize(m_pDA).nY;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIndicesFromState)(IScriptedContext* context, BSTR state, IDispatch** ppArray)
	{
		try
		{
			*ppArray = NULL;

			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);
			ULONG nItems = 0;
			pItems->Size(&nItems);

			CComBSTR bstrState;
			m_pDA->StatePrefix(&bstrState);
			bstrState += state;

			CComQIPtr<IOperationContext> pContext(context);
			CComPtr<ISharedState> pState;
			pContext->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pIDs;
			if (pState)
				m_pDA->StateUnpack(pState, &pIDs);

			CComPtr<IJScriptArrayInit> p;
			m_pMgr->CreateJScriptArray(&p);

			ULONG nSel = 0;
			pIDs->Size(&nSel);
			for (ULONG i = 0; i < nSel; ++i)
			{
				CComPtr<IComparable> pSel;
				pIDs->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pSel));
				for (ULONG j = 0; j < nItems; ++j)
				{
					CComPtr<IComparable> pItem;
					pItems->Get(j, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
					if (pSel && pItem && S_OK == pSel->Compare(pItem))
					{
						p->AddNumber(j);
						break;
					}
				}
			}
			*ppArray = p.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppArray ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(PutIndicesToState)(IScriptedContext* context, BSTR state, VARIANT* pSelectedItems)
	{
		try
		{
			CComPtr<ISharedState> pState;

			CComPtr<IEnumUnknowns> pItems;
			m_pDA->FramesEnum(&pItems);

			if (pSelectedItems->vt == VT_DISPATCH)
			{
				DISPPARAMS params;
				ZeroMemory(&params, sizeof params);
				CComVariant res;
				DISPID dl = 0;
				LPOLESTR ln = L"length";
				if (SUCCEEDED(pSelectedItems->pdispVal->GetIDsOfNames(IID_NULL, &ln, 1, LOCALE_USER_DEFAULT, &dl)) &&
					SUCCEEDED(pSelectedItems->pdispVal->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
				{
					res.ChangeType(VT_I4);
					LONG const len = res.lVal;

					CComPtr<IEnumUnknownsInit> pSel;
					RWCoCreateInstance(pSel, __uuidof(EnumUnknowns));

					for (int j = 0; j < len; ++j)
					{
						OLECHAR szIndex[16];
						swprintf(szIndex, L"%i", j);
						LPOLESTR psz = szIndex;
						DISPID id = 0;
						res.Clear();
						if (SUCCEEDED(pSelectedItems->pdispVal->GetIDsOfNames(IID_NULL, &psz, 1, LOCALE_USER_DEFAULT, &id)) &&
							SUCCEEDED(pSelectedItems->pdispVal->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
						{
							res.ChangeType(VT_I4);
							CComPtr<IUnknown> pItem;
							pItems->Get(res.intVal, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
							if (pItem)
								pSel->Insert(pItem);
						}
					}

					m_pDA->StatePack(pSel, &pState);
				}
			}
			else if (pSelectedItems->vt == VT_I4)
			{
				CComPtr<IUnknown> pItem;
				pItems->Get(pSelectedItems->intVal, __uuidof(IUnknown), reinterpret_cast<void**>(&pItem));
				if (pItem)
				{
					CComPtr<IEnumUnknownsInit> pItems;
					RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
					pItems->Insert(pItem);
					m_pDA->StatePack(pItems, &pState);
				}
			}
			CComBSTR bstrPrefix;
			m_pDA->StatePrefix(&bstrPrefix);
			if (bstrPrefix.Length())
			{
				bstrPrefix += state;
				state = bstrPrefix;
			}
			CComQIPtr<IOperationContext> pContext(context);
			return pState ? pContext->StateSet(state, pState) : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	static TImageSize GetAnimationSize(IDocumentAnimation* a_pAni)
	{
		TImageSize tSize = {0, 0};
		CComPtr<IEnumUnknowns> pFrames;
		a_pAni->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		if (pFrames) pFrames->Size(&nFrames);
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pDoc;
			a_pAni->FrameGetDoc(pFrame, &pDoc);
			CComPtr<IDocumentImage> pImg;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
			TImageSize tFrameSize = {1, 1};
			if (pImg) pImg->CanvasGet(&tFrameSize, NULL, NULL, NULL, NULL);
			if (tFrameSize.nX > tSize.nX) tSize.nX = tFrameSize.nX;
			if (tFrameSize.nY > tSize.nY) tSize.nY = tFrameSize.nY;
		}
		return tSize;
	}
private:
	CComPtr<IScriptingInterfaceManager> m_pMgr;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentAnimation> m_pDA;
};


STDMETHODIMP CScriptingInterfaceAnimation::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IDocumentAnimation> pDA;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDA));
		if (pDA)
		{
			CComObject<CScriptedAnimation>* p = NULL;
			CComObject<CScriptedAnimation>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pScriptingMgr, a_pDocument, pDA);
			a_pSite->AddItem(CComBSTR(L"Animation"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceAnimation::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pSecondary->Insert(CComBSTR(L"Animation"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

