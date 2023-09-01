
#pragma once

#include <Win32LangEx.h>

MIDL_INTERFACE("BB51B92B-D0DF-4EB4-9D74-B83BA77B4D42")
IStructuredItemLayerItem : public IUnknown
{
public:
	STDMETHOD_(CDocumentLayeredImage*, Doc)() = 0;
	STDMETHOD_(ULONG, ID)(CDocumentLayeredImage* a_pThis) = 0;
	STDMETHOD(Compare)(CDocumentLayeredImage* a_pThis, ULONG a_nID) = 0;
	STDMETHOD(SubItems)(IEnumUnknowns** a_ppSubItems) = 0;
};


class CStructuredItemLayer : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IUIItem,
	public ISubDocumentID,
	public IItemString,
	public IItemBool,
	//public IItemChoice,
	public IStructuredItemLayerItem
{
public:
	CStructuredItemLayer() : m_pDoc(NULL)
	{
	}
	~CStructuredItemLayer()
	{
	}
	void Init(CDocumentLayeredImage* a_pDoc, ULONG a_nLayerID)
	{
		m_nLayerID = a_nLayerID;
		m_pDoc = a_pDoc;
	}


BEGIN_COM_MAP(CStructuredItemLayer)
	COM_INTERFACE_ENTRY2(IComparable, IUIItem)
	COM_INTERFACE_ENTRY(IUIItem)
	COM_INTERFACE_ENTRY(IStructuredItemLayerItem)
	COM_INTERFACE_ENTRY(ISubDocumentID)
	COM_INTERFACE_ENTRY(IItemString)
	COM_INTERFACE_ENTRY(IItemBool)
	//COM_INTERFACE_ENTRY(IItemChoice)
END_COM_MAP()


	// IUIItem methods
public:
	STDMETHOD(NameGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
	{
		try
		{
			*a_pbstrName = NULL;

			CComPtr<ILocalizedString> pName;
			m_pDoc->GetLayerTypeName(m_nLayerID, &pName);
			if (pName)
				return pName->GetLocalized(a_tPreferedLCID, a_pbstrName);

			CMultiLanguageString::GetLocalized(L"[0409]Layer[0405]Vrstva", a_tPreferedLCID, a_pbstrName);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(DescriptionGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(ColorsGet)(DWORD* UNREF(a_prgbPrimary), DWORD* UNREF(a_prgbSecondary))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IconIDGet)(GUID* a_pIconID)
	{
		try
		{
			return m_pDoc->LayerIconIDGet(m_nLayerID, a_pIconID);
		}
		catch (...)
		{
			return E_POINTER;
		}
	}
	STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			return m_pDoc->LayerIconGet(m_nLayerID, a_nSize, a_phIcon);
		}
		catch (...)
		{
			return a_phIcon == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(UseThumbnail)()
	{
		GUID tIconID = GUID_NULL;
		m_pDoc->LayerBuilderIDGet(m_nLayerID, &tIconID);
		return IsEqualGUID(tIconID, __uuidof(DocumentFactoryLayeredImage)) ? S_FALSE : S_OK;
	}
	STDMETHOD(ExpandedByDefault)()
	{
		GUID tIconID = GUID_NULL;
		m_pDoc->LayerBuilderIDGet(m_nLayerID, &tIconID);
		return IsEqualGUID(tIconID, __uuidof(DocumentFactoryLayeredImage)) ? S_OK : S_FALSE;
	}

	// IComparable methods
public:
	STDMETHOD(Compare)(IComparable* a_pOther)
	{
		try
		{
			CComQIPtr<IStructuredItemLayerItem> pLayerItem(a_pOther);
			if (pLayerItem == NULL)
				return S_FALSE;
			return pLayerItem->Compare(m_pDoc, m_nLayerID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		try
		{
			static const GUID tMyID = {0xecca507c, 0x68b5, 0x4730, {0xbb, 0x61, 0x7c, 0x3b, 0x12, 0xa8, 0xa, 0xb8}};
			*a_pCLSID = tMyID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}

	// IStructuredItemLayerItem methods
public:
	STDMETHOD_(CDocumentLayeredImage*, Doc)()
	{
		return m_pDoc;
	}
	STDMETHOD_(ULONG, ID)(CDocumentLayeredImage* a_pThis)
	{
		return a_pThis == m_pDoc ? m_nLayerID : ULONG(-1);
	}
	STDMETHOD(Compare)(CDocumentLayeredImage* a_pThis, ULONG a_nID)
	{
		return a_pThis < m_pDoc ? S_LESS : (a_pThis == m_pDoc ? (a_nID < m_nLayerID ? S_LESS : (a_nID == m_nLayerID ? S_OK : S_MORE)) : S_MORE);
	}
	STDMETHOD(SubItems)(IEnumUnknowns** a_ppSubItems)
	{
		try
		{
			*a_ppSubItems = NULL;
			CReadLock<IBlockOperations> cLock(*m_pDoc);
			CComPtr<IDocument> pDoc;
			m_pDoc->GetSubDocument(m_nLayerID, &pDoc);
			if (pDoc == NULL)
				return S_FALSE;
			CComPtr<IDocumentLayeredImage> pSLI;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pSLI));
			if (pSLI == NULL)
				return S_FALSE;
			return pSLI->ItemsEnum(NULL, a_ppSubItems);
		}
		catch (...)
		{
			return a_ppSubItems == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// ISubDocumentID method
public:
	STDMETHOD(SubDocumentGet)(IDocument** a_ppSubDocument)
	{
		try
		{
			*a_ppSubDocument = NULL;
			CReadLock<IBlockOperations> cLock(*m_pDoc);
			return m_pDoc->GetSubDocument(m_nLayerID, a_ppSubDocument);
		}
		catch (...)
		{
			return a_ppSubDocument == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IItemString method
public:
	STDMETHOD(ValueGet)(BSTR* a_pbstrValue)
	{
		try
		{
			return m_pDoc->LayerNameGet(static_cast<IUIItem*>(this), a_pbstrValue);
		}
		catch (...)
		{
			return a_pbstrValue == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ValueSet)(BSTR a_bstrValue)
	{
		try
		{
			return m_pDoc->LayerNameSet(static_cast<IUIItem*>(this), a_bstrValue);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IItemBool methods
public:
	STDMETHOD(ValueGet)(boolean* a_pbValue)
	{
		try
		{
			BYTE bVisible = TRUE;
			HRESULT hRes = m_pDoc->LayerPropsGet(static_cast<IUIItem*>(this), NULL, &bVisible);
			*a_pbValue = bVisible;
			return hRes;
		}
		catch (...)
		{
			return a_pbValue == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ValueSet)(boolean a_bValue)
	{
		try
		{
			BYTE bVisible = a_bValue ? TRUE : FALSE;
			return m_pDoc->LayerPropsSet(static_cast<IUIItem*>(this), NULL, &bVisible);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

//	// IItemChoice methods
//public:
//	STDMETHOD(ValueGet)(ULONG* a_pnValue)
//	{
//		try
//		{
//			TImageLayer tIL;
//			tIL.eBlend = EBEAlphaBlend;
//			HRESULT hRes = m_pDoc->LayerPropsGet(static_cast<IUIItem*>(this), &tIL);
//			*a_pnValue = FindBlendMode(tIL.eBlend);
//			return hRes;
//		}
//		catch (...)
//		{
//			return a_pnValue == NULL ? E_POINTER : E_UNEXPECTED;
//		}
//	}
//	STDMETHOD(ValueSet)(ULONG a_nValue)
//	{
//		try
//		{
//			TImageLayer tIL;
//			HRESULT hRes = m_pDoc->LayerPropsGet(static_cast<IUIItem*>(this), &tIL);
//			if (FAILED(hRes)) return hRes;
//			ELayerBlend const* begin;
//			ELayerBlend const* end;
//			BlendModes(&begin, &end);
//			if (a_nValue < (end-begin))
//				tIL.eBlend = begin[a_nValue];
//			return m_pDoc->LayerPropsSet(static_cast<IUIItem*>(this), &tIL);
//		}
//		catch (...)
//		{
//			return E_UNEXPECTED;
//		}
//	}
//	STDMETHOD(OptionsEnum)(IEnumUnknowns** a_ppOptionsNames)
//	{
//		if (a_ppOptionsNames == NULL)
//			return E_POINTER;
//		try
//		{
//			CComPtr<IEnumUnknownsInit> pInit;
//			RWCoCreateInstance(pInit, __uuidof(EnumUnknowns));
//			ELayerBlend const* begin;
//			ELayerBlend const* end;
//			BlendModes(&begin, &end);
//			ULONG n = end-begin;
//			for (ULONG i = 0; i < n; ++i)
//			{
//				pInit->Insert(CMultiLanguageString::GetAuto(L"Test"));
//			}
//			*a_ppOptionsNames = pInit;
//			pInit.Detach();
//			return S_OK;
//		}
//		catch (...)
//		{
//			return E_UNEXPECTED;
//		}
//	}
//
//private:
//	void BlendModes(ELayerBlend const** begin, ELayerBlend const** end)
//	{
//		static ELayerBlend const s_aModes[] =
//		{
//			EBEAlphaBlend,
//			EBEModulate,
//			EBEScreen,
//			EBEAdd,
//			EBESubtract,
//			EBEAverage,
//			EBEDifference,
//			EBEMinimum,
//			EBEMaximum,
//			EBEOverlay,
//			ELBHLSReplaceHue,
//			ELBHLSReplaceSaturation,
//			ELBHLSReplaceLuminance,
//			ELBHLSReplaceColor,
//			EBEMultiplyInvAlpha,
//		};
//		*begin = s_aModes;
//		*end = s_aModes+itemsof(s_aModes);
//	}
//	int FindBlendMode(ELayerBlend bm)
//	{
//		ELayerBlend const* begin;
//		ELayerBlend const* end;
//		BlendModes(&begin, &end);
//		int i = 0;
//		while (begin != end)
//		{
//			if (bm == *begin)
//				return i;
//			++i;
//			++begin;
//		}
//		return 0;
//	}

private:
	CDocumentLayeredImage* m_pDoc;
	ULONG m_nLayerID;
};

