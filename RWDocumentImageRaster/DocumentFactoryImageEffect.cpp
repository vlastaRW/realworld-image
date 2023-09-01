
#include "stdafx.h"
#include <RWInput.h>
#include <MultiLanguageString.h>

// {051A4612-0DA7-49de-875F-A19CE60627FD}
extern GUID const CLSID_DocumentFactoryImageEffect = {0x51a4612, 0xda7, 0x49de, { 0x87, 0x5f, 0xa1, 0x9c, 0xe6, 0x6, 0x27, 0xfd}};

class ATL_NO_VTABLE CDocumentFactoryImageEffect : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentFactoryImageEffect>,
	public IDocumentBuilder
{
public:
	CDocumentFactoryImageEffect()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentFactoryImageEffect)

BEGIN_CATEGORY_MAP(CDocumentFactoryImageEffect)
	IMPLEMENTED_CATEGORY(CATID_DocumentBuilder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentFactoryImageEffect)
	COM_INTERFACE_ENTRY(IDocumentBuilder)
END_COM_MAP()


	// IDocumentBuilder methods
public:
	STDMETHOD(Priority)(ULONG* a_pnPriority)
	{
		if (a_pnPriority)
			*a_pnPriority = EDPMinimum;
		return S_OK;
	}
	STDMETHOD(TypeName)(ILocalizedString** a_ppType)
	{
		if (a_ppType)
			*a_ppType = new CMultiLanguageString(L"[0409]Image Effect[0405]Obrázkový efekt");
		return S_OK;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(FormatInfo)(ILocalizedString** a_ppFormat, BSTR* a_pbstrShellIcon)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(HasFeatures)(ULONG a_nCount, IID const* a_aiidRequired)
	{
		return E_NOTIMPL;
	}
};

OBJECT_ENTRY_AUTO(CLSID_DocumentFactoryImageEffect, CDocumentFactoryImageEffect)
