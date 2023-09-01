
#pragma once

#include <SharedStringTable.h>


struct SFillSpec
{
	wchar_t const* pszID;
	UINT uIDName;
	UINT uIDDesc;
	GUID tDefIconID;
	UINT uIconResID;
	HRESULT (*pfnCreateFill)(IRasterImageBrush** a_ppFill);
	HRESULT (*pfnCreateFillWindow)(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolWindow** a_ppWindow);
};

template<class T>
inline HRESULT CreateFill(IRasterImageBrush** a_ppTool)
{
	*a_ppTool = NULL;
	CComObject<T>* p = NULL;
	HRESULT hRes = CComObject<T>::CreateInstance(&p);
	if (p)
		p->AddRef();
	*a_ppTool = p;
	return hRes;
}

template<class T>
inline HRESULT CreateFillWindow(LPCOLESTR a_pszStyleID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolWindow** a_ppWindow)
{
	*a_ppWindow = NULL;
	CComObject<T>* p = NULL;
	HRESULT hRes = CComObject<T>::CreateInstance(&p);
	CComPtr<IRasterImageEditToolWindow> pTmp = p;
	if (p)
		if (NULL == p->Create(a_pszStyleID, a_hParent, a_tLocaleID, a_pSharedState, a_bstrSyncGroup))
			return E_FAIL;
	*a_ppWindow = pTmp.Detach();
	return hRes;
}


template<SFillSpec const* t_aFillStyles, size_t t_nFillStyles>
class CRasterImageFillStyleFactoryImpl :
	public IRasterImageFillStyleFactory
{
	// IRasterImageFillStyleFactory methods
public:
	STDMETHOD(StyleIDsEnum)(IRasterImageFillStyleManager* UNREF(a_pManager), IEnumStrings** a_ppIDs)
	{
		try
		{
			*a_ppIDs = NULL;
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				pTmp->Insert(CComBSTR(pTS->pszID));
			*a_ppIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppIDs ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(StyleNameGet)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					*a_ppName = _SharedStringTable.GetString(pTS->uIDName);
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(StyleDescGet)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, ILocalizedString** a_ppDesc)
	{
		try
		{
			*a_ppDesc = NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					*a_ppDesc = _SharedStringTable.GetString(pTS->uIDDesc);
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppDesc ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(StyleIconIDGet)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, GUID* a_ptDefaultIcon)
	{
		try
		{
			*a_ptDefaultIcon = GUID_NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					*a_ptDefaultIcon = pTS->tDefIconID;
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(StyleIconGet)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					*a_phIcon = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(pTS->uIconResID), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(FillStyleCreate)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, IDocument* a_pDocument, IRasterImageBrush** a_ppFillStyle)
	{
		try
		{
			*a_ppFillStyle = NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
					return pTS->pfnCreateFill(a_ppFillStyle);
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppFillStyle ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(IRasterImageFillStyleManager* UNREF(a_pManager), BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
	{
		try
		{
			*a_ppWindow = NULL;
			for (SFillSpec const* pTS = t_aFillStyles; pTS != t_aFillStyles+t_nFillStyles; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
					return pTS->pfnCreateFillWindow ? pTS->pfnCreateFillWindow(pTS->pszID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow) : E_FAIL;
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppWindow ? E_UNEXPECTED : E_POINTER;
		}
	}
};
