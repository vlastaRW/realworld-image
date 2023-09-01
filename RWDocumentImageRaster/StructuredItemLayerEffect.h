
#pragma once

#include <Win32LangEx.h>

MIDL_INTERFACE("8DC9ACA7-E1C8-4E61-8A58-DB1DDE949794")
IStructuredItemLayerEffect : public IUnknown
{
public:
	STDMETHOD_(CDocumentLayeredImage*, Doc)() = 0;
	STDMETHOD(Compare)(CDocumentLayeredImage* a_pThis, ULONG a_nEffectID) = 0;
	STDMETHOD_(ULONG, LayerID)() = 0;
	STDMETHOD_(ULONG, EffectID)() = 0;
};

class CStructuredItemLayerEffect : 
	public IUIItem,
	public ISubDocumentID,
	public IItemBool,
	public IStructuredItemLayerEffect
{
public:
	CStructuredItemLayerEffect(CDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, ULONG a_nEffectID) :
		m_pDoc(a_pDoc), m_nLayerID(a_nLayerID), m_nEffectID(a_nEffectID), m_nRefCount(1)
	{
	}
	void ReleaseAndUnlink()
	{
		m_pDoc = NULL;
		Release();
	}


	// IUnknown methods
public:
	STDMETHOD_(ULONG, AddRef)()
	{
		LONG nNew = InterlockedIncrement(&m_nRefCount);
		//if (nNew == 2 && m_pDoc)
		//	m_pDoc->AddRef();
		return nNew;
	}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG n = InterlockedDecrement(&m_nRefCount);
		if (n == 0)
			delete this;
		return n;
	}
	STDMETHOD(QueryInterface)(REFIID a_guidIID, void** a_ppInterface)
	{
		if (IsEqualIID(a_guidIID, IID_IUnknown) ||
			IsEqualIID(a_guidIID, __uuidof(IComparable)) ||
			IsEqualIID(a_guidIID, __uuidof(IUIItem)))
		{
			*a_ppInterface = static_cast<IUIItem*>(this);
			AddRef();
			return S_OK;
		}
		if (IsEqualIID(a_guidIID, __uuidof(IStructuredItemLayerEffect)))
		{
			*a_ppInterface = static_cast<IStructuredItemLayerEffect*>(this);
			AddRef();
			return S_OK;
		}
		if (IsEqualIID(a_guidIID, __uuidof(ISubDocumentID)))
		{
			*a_ppInterface = static_cast<ISubDocumentID*>(this);
			AddRef();
			return S_OK;
		}
		if (IsEqualIID(a_guidIID, __uuidof(IItemBool)))
		{
			*a_ppInterface = static_cast<IItemBool*>(this);
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	// IUIItem methods
public:
	STDMETHOD(NameGet)(LCID a_tPreferedLCID, BSTR* a_pbstrName)
	{
		try
		{
			bool bEnabled = false;
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pOpCfg;
			if (m_pDoc->LayerEffectStepGet(m_nLayerID, m_nEffectID, &bEnabled, &tID, &pOpCfg))
			{
				CComPtr<IDocumentOperation> pOp;
				RWCoCreateInstance(pOp, tID);
				CComQIPtr<IConfigDescriptor> pOA(pOp);
				if (pOA)
				{
					CComPtr<ILocalizedString> pDisplayName;
					pOA->Name(/*m_pOM*/NULL, pOpCfg, &pDisplayName);
					if (pDisplayName)
						return pDisplayName->GetLocalized(a_tPreferedLCID, a_pbstrName);
				}
				CComPtr<ILocalizedString> pStr;
				if (pOp) pOp->NameGet(NULL, &pStr);
				if (pStr) return pStr->GetLocalized(a_tPreferedLCID, a_pbstrName);
			}
			return E_FAIL;
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
		if (a_pIconID == NULL)
			return E_POINTER;

		try
		{
			bool bEnabled = false;
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pOpCfg;
			if (m_pDoc->LayerEffectStepGet(m_nLayerID, m_nEffectID, &bEnabled, &tID, &pOpCfg))
			{
				CComPtr<IDocumentOperation> pOp;
				RWCoCreateInstance(pOp, tID);
				CComQIPtr<IConfigDescriptor> pOA(pOp);
				if (pOA)
				{
					// TODO: add icon id method to IConfigDescriptor
					*a_pIconID = tID;
					pOA->PreviewIconID(NULL, pOpCfg, a_pIconID);
					return S_OK;
				}
			}
			return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(IconGet)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			bool bEnabled = false;
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pOpCfg;
			if (m_pDoc->LayerEffectStepGet(m_nLayerID, m_nEffectID, &bEnabled, &tID, &pOpCfg))
			{
				CComPtr<IDocumentOperation> pOp;
				RWCoCreateInstance(pOp, tID);
				CComQIPtr<IConfigDescriptor> pOA(pOp);
				if (pOA)
				{
					if (SUCCEEDED(pOA->PreviewIcon(NULL, pOpCfg, a_nSize, a_phIcon)))
						return S_OK;
				}
				// TODO: use IConfigDescriptor
				CComPtr<IDesignerFrameIcons> pDFI;
				RWCoCreateInstance(pDFI, __uuidof(DesignerFrameIconsManager));
				return pDFI ? pDFI->GetIcon(tID, a_nSize, a_phIcon) : E_FAIL;
			}
			return E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(UseThumbnail)() { return S_FALSE; }
	STDMETHOD(ExpandedByDefault)() { return S_FALSE; }

	// IComparable methods
public:
	STDMETHOD(Compare)(IComparable* a_pOther)
	{
		try
		{
			CComQIPtr<IStructuredItemLayerEffect> pLayerEffect(a_pOther);
			if (pLayerEffect == NULL)
				return S_FALSE;
			return pLayerEffect->Compare(m_pDoc, m_nEffectID);
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
			static const GUID tMyID = {0x265110c9, 0xe26f, 0x4585, {0x92, 0xb0, 0x82, 0x9, 0x20, 0x1e, 0x26, 0x14}};
			*a_pCLSID = tMyID;
			return S_OK;
		}
		catch (...)
		{
			return E_POINTER;
		}
	}

	// IStructuredItemLayerEffect methods
public:
	STDMETHOD_(CDocumentLayeredImage*, Doc)()
	{
		return m_pDoc;
	}
	STDMETHOD(Compare)(CDocumentLayeredImage* a_pThis, ULONG a_nEffectID)
	{
		return a_pThis < m_pDoc ? S_LESS : (a_pThis == m_pDoc ? (a_nEffectID < m_nEffectID ? S_LESS : (a_nEffectID == m_nEffectID ? S_OK : S_MORE)) : S_MORE);
	}
	STDMETHOD_(ULONG, LayerID)()
	{
		return m_nLayerID;
	}
	STDMETHOD_(ULONG, EffectID)()
	{
		return m_nEffectID;
	}

	// ISubDocumentID method
public:
	STDMETHOD(SubDocumentGet)(IDocument** a_ppSubDocument)
	{
		try
		{
			*a_ppSubDocument = NULL;
			CReadLock<IBlockOperations> cLock(*m_pDoc);
			return m_pDoc->GetEffectSubDocument(m_nLayerID, m_nEffectID, a_ppSubDocument);
		}
		catch (...)
		{
			return a_ppSubDocument == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

	// IItemBool methods
public:
	STDMETHOD(ValueGet)(boolean* a_pbValue)
	{
		try
		{
			bool bEnabled = false;
			if (m_pDoc->LayerEffectStepGet(m_nLayerID, m_nEffectID, &bEnabled, NULL, NULL))
				*a_pbValue = bEnabled;
			else
				*a_pbValue = false;
			return S_OK;
		}
		catch (...)
		{
			return a_pbValue == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ValueSet)(boolean a_bValue)
	{
		BYTE b = a_bValue;
		if (m_pDoc)
			return m_pDoc->LayerEffectStepSet(static_cast<IUIItem*>(this), &b, NULL, NULL);
		return E_UNEXPECTED;
	}

private:
	CDocumentLayeredImage* m_pDoc;
	ULONG m_nLayerID;
	ULONG m_nEffectID;

	LONG m_nRefCount;
};

