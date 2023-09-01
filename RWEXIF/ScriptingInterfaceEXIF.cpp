// ScriptingInterfaceEXIF.cpp : Implementation of CScriptingInterfaceEXIF

#include "stdafx.h"
#include "ScriptingInterfaceEXIF.h"

#include <RWImaging.h>
#include "RWEXIFWrapper.h"
#include <ReturnedData.h>


class ATL_NO_VTABLE CScriptedEXIF : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedEXIF, &IID_IScriptedEXIF, &LIBID_RWEXIFLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedEXIF()
	{
	}
	void Init(IDocument* a_pDoc, IImageMetaData* a_pMetaData)
	{
		m_pDoc = a_pDoc;
		m_pIMD = a_pMetaData;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedEXIF)

BEGIN_COM_MAP(CScriptedEXIF)
	COM_INTERFACE_ENTRY(IScriptedEXIF)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
END_COM_MAP()


// IScriptedEXIF methods
public:
	STDMETHOD(get_Exists)(VARIANT_BOOL* pVal)
	{
		ULONG n = 0;
		m_pIMD->GetBlockSize(CComBSTR(L"EXIF"), &n);
		*pVal = n ? VARIANT_TRUE : VARIANT_FALSE;
		return S_OK;
	}
	STDMETHOD(put_Exists)(VARIANT_BOOL newVal)
	{
		CComBSTR bstr(L"EXIF");
		ULONG n = 0;
		m_pIMD->GetBlockSize(bstr, &n);
		if (n && newVal == VARIANT_FALSE)
		{
			return m_pIMD->DeleteBlock(bstr);
		}
		if (n == 0 && newVal != VARIANT_FALSE)
		{
			 // TODO: implement adding exif data;
		}
		return S_FALSE;
	}
	STDMETHOD(GetValueByName)(BSTR tagName, BSTR/*VARIANT*/* pVal)
	{
		try
		{
			*pVal = NULL;//pVal->vt = VT_EMPTY;
			CComBSTR bstr(L"EXIF");
			ULONG n = 0;
			m_pIMD->GetBlockSize(bstr, &n);
			if (n == 0)
				return S_FALSE;
			CAutoVectorPtr<BYTE> cData(new BYTE[n]);
			if (FAILED(m_pIMD->GetBlock(bstr, n, cData)))
				return S_FALSE;
			CComObject<CRWEXIF>* pExif = NULL;
			CComObject<CRWEXIF>::CreateInstance(&pExif);
			CComPtr<IRWEXIF> pTmp = pExif;
			pExif->Load(n, cData);
			WORD wIFD = 0;
			WORD wTag = 0;
			if (FAILED(pExif->TagFindByName(tagName, &wIFD, &wTag)))
				return E_FAIL;
			return SUCCEEDED(pExif->TagGetAsText(wIFD, wTag, pVal)) ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetValueByName)(BSTR tagName, BSTR val)
	{
		try
		{
			CComBSTR bstr(L"EXIF");
			ULONG n = 0;
			m_pIMD->GetBlockSize(bstr, &n);
			if (n == 0)
				return S_FALSE;
			CAutoVectorPtr<BYTE> cData(new BYTE[n]);
			if (FAILED(m_pIMD->GetBlock(bstr, n, cData)))
				return E_FAIL;
			CComObject<CRWEXIF>* pExif = NULL;
			CComObject<CRWEXIF>::CreateInstance(&pExif);
			CComPtr<IRWEXIF> pTmp = pExif;
			pExif->Load(n, cData);
			WORD wIFD = 0;
			WORD wTag = 0;
			if (FAILED(pExif->TagFindByName(tagName, &wIFD, &wTag)))
				return E_FAIL;
			if (FAILED(pExif->TagSetAsText(wIFD, wTag, val)))
				return E_FAIL;
			CReturnedData cData2;
			if (FAILED(pExif->Save(&cData2)))
				return E_FAIL;
			return SUCCEEDED(m_pIMD->SetBlock(bstr, cData2.size(), cData2.begin())) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IImageMetaData> m_pIMD;
};


// CScriptingInterfaceEXIF

STDMETHODIMP CScriptingInterfaceEXIF::GetGlobalObjects(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IUnknown* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	return S_FALSE;
}

STDMETHODIMP CScriptingInterfaceEXIF::GetInterfaceAdaptors(IScriptingInterfaceManager* a_pScriptingMgr, IScriptingSite* a_pSite, IDocument* a_pDocument)
{
	try
	{
		CComPtr<IImageMetaData> pIMD;
		a_pDocument->QueryFeatureInterface(__uuidof(IImageMetaData), reinterpret_cast<void**>(&pIMD));
		if (pIMD)
		{
			CComObject<CScriptedEXIF>* p = NULL;
			CComObject<CScriptedEXIF>::CreateInstance(&p);
			CComPtr<IDispatch> pTmp = p;
			p->Init(a_pDocument, pIMD);
			a_pSite->AddItem(CComBSTR(L"EXIF"), pTmp);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptingInterfaceEXIF::GetKeywords(IScriptingInterfaceManager* a_pScriptingMgr, IEnumStringsInit* a_pPrimary, IEnumStringsInit* a_pSecondary)
{
	try
	{
		a_pSecondary->Insert(CComBSTR(L"EXIF"));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

