// RasterImageFillStyleManager.h : Declaration of the CRasterImageFillStyleManager

#pragma once
#include "resource.h"       // main symbols
#include "RWDrawing.h"
#include <WeakSingleton.h>



// CRasterImageFillStyleManager

class ATL_NO_VTABLE CRasterImageFillStyleManager :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageFillStyleManager, &CLSID_RasterImageFillStyleManager>,
	public IRasterImageFillStyleManager
{
public:
	CRasterImageFillStyleManager() : m_nTimeStamp(0)
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CRasterImageFillStyleManager)

BEGIN_COM_MAP(CRasterImageFillStyleManager)
	COM_INTERFACE_ENTRY(IRasterImageFillStyleManager)
END_COM_MAP()


public:
	STDMETHOD(StyleIDsEnum)(IEnumStrings** a_ppIDs);
	STDMETHOD(StyleNameGet)(BSTR a_bstrID, ILocalizedString** a_ppName);
	STDMETHOD(StyleDescGet)(BSTR a_bstrID, ILocalizedString** a_ppDesc);
	STDMETHOD(StyleIconIDGet)(BSTR a_bstrID, GUID* a_ptDefaultIcon);
	STDMETHOD(StyleIconGet)(BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon);
	STDMETHOD(FillStyleCreate)(BSTR a_bstrID, IDocument* a_pDocument, IRasterImageBrush** a_ppFillStyle);
	STDMETHOD(WindowCreate)(BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow);

private:
	struct SPrefixLess
	{
		bool operator()(CComBSTR const& a_1, CComBSTR const& a_2) const
		{
			LPCWSTR p1 = a_1;
			LPCWSTR p2 = a_2;
			while (*p1 == *p2 && *p1) {++p1; ++p2;}
			if (*p1 == L'\0' || *p2 == L'\0')
				return false;
			return *p1 < *p2;
		}
	};

	typedef std::map<CComBSTR, CComPtr<IRasterImageFillStyleFactory>, SPrefixLess> CFactoryMap;

private:
	void CheckState();

private:
	CComPtr<IEnumStringsInit> m_pIDs;
	CFactoryMap m_cFactoryMap;
	ULONG m_nTimeStamp;
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageFillStyleManager), CRasterImageFillStyleManager)
