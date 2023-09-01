
#pragma once

#include <SharedStringTable.h>
#include <MultiLanguageString.h>

struct SIDorPsz
{
	SIDorPsz(UINT a) : dw(DWORD_PTR(a)) {};
	SIDorPsz(wchar_t const* a) : dw(DWORD_PTR(a)) {};
	DWORD_PTR dw;
};

enum EToolPaintSpec
{
	ETPSNoPaint = 0,
	ETPSSolidFill,
	ETPSPatternFill,
	ETPSSolidOutlineAndPatternFill,
	ETPSPatternOutlineAndPatternFill,
};

struct SToolSpec
{
	wchar_t const* pszID;
	SIDorPsz tName;
	SIDorPsz tDesc;
	GUID tDefIconID;
	UINT uIconResID;
	DWORD dwBlendingModes;
	DWORD dwRasterizationModes;
	DWORD dwCoordinatesModes;
	EToolPaintSpec ePaintSpec;
	BOOL bGammaOverride;
	HRESULT (*pfnCreateTool)(IRasterImageEditTool** a_ppTool);
	HRESULT (*pfnCreateToolWindow)(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolWindow** a_ppWindow);
};

template<class T>
inline HRESULT CreateTool(IRasterImageEditTool** a_ppTool)
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
inline HRESULT CreateToolWindow(LPCOLESTR a_pszToolID, HWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pSharedState, BSTR a_bstrSyncGroup, IRasterImageEditToolWindow** a_ppWindow)
{
	*a_ppWindow = NULL;
	CComObject<T>* p = NULL;
	HRESULT hRes = CComObject<T>::CreateInstance(&p);
	CComPtr<IRasterImageEditToolWindow> pTmp = p;
	if (p)
		if (NULL == p->Create(a_pszToolID, a_hParent, a_tLocaleID, a_pSharedState, a_bstrSyncGroup))
			return E_FAIL;
	*a_ppWindow = pTmp.Detach();
	return hRes;
}


template<SToolSpec const* t_aShapeTools, size_t t_nShapeTools>
class CRasterImageEditToolsFactoryImpl :
	public IRasterImageEditToolsFactory
{
	// IRasterImageEditToolsFactory methods
public:
	STDMETHOD(ToolIDsEnum)(IRasterImageEditToolsManager* UNREF(a_pManager), IEnumStrings** a_ppToolIDs)
	{
		try
		{
			*a_ppToolIDs = NULL;
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				pTmp->Insert(CComBSTR(pTS->pszID));
			*a_ppToolIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppToolIDs ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ToolNameGet)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, ILocalizedString** a_ppName)
	{
		try
		{
			*a_ppName = NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					if (pTS->tName.dw&0xffff0000)
						*a_ppName = new CMultiLanguageString(LPCWSTR(pTS->tName.dw));
					else
						*a_ppName = _SharedStringTable.GetString(pTS->tName.dw);
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ToolDescGet)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, ILocalizedString** a_ppDesc)
	{
		try
		{
			*a_ppDesc = NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					if (pTS->tDesc.dw&0xffff0000)
						*a_ppDesc = new CMultiLanguageString(LPCWSTR(pTS->tDesc.dw));
					else
						*a_ppDesc = _SharedStringTable.GetString(pTS->tDesc.dw);
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppDesc ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(ToolIconIDGet)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, GUID* a_ptDefaultIcon)
	{
		try
		{
			*a_ptDefaultIcon = GUID_NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
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
	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
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
	STDMETHOD(SupportedStates)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
	{
		try
		{
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
				{
					if (a_pBlendingModes) *a_pBlendingModes = pTS->dwBlendingModes;
					if (a_pRasterizationModes) *a_pRasterizationModes = pTS->dwRasterizationModes;
					if (a_pCoordinatesModes) *a_pCoordinatesModes = pTS->dwCoordinatesModes;
					if (a_pPaintSpecs)
					{
						static ULONG const s_aFlags[] =
						{
							ETBTIdentifierInterior|ETBTSingleColor,
							ETBTIdentifierInterior|ETBTRasterImage,
							ETBTIdentifierOutline|ETBTSingleColor|ETBTOutlineWidth,
							ETBTIdentifierInterior|ETBTRasterImage,
							ETBTIdentifierOutline|ETBTRasterImage|ETBTOutlineWidth,
						};
						switch (pTS->ePaintSpec)
						{
						case ETPSSolidFill:
							a_pPaintSpecs->Consume(0, 1, s_aFlags);
							break;
						case ETPSPatternFill:
							a_pPaintSpecs->Consume(0, 1, s_aFlags+1);
							break;
						case ETPSSolidOutlineAndPatternFill:
							a_pPaintSpecs->Consume(0, 2, s_aFlags+1);
							break;
						case ETPSPatternOutlineAndPatternFill:
							a_pPaintSpecs->Consume(0, 2, s_aFlags+3);
							break;
						}
					}
					//if (a_pGammaOverride) *a_pGammaOverride = pTS->bGammaOverride;
					return S_OK;
				}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(EditToolCreate)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, IDocument* UNREF(a_pDocument), IRasterImageEditTool** a_ppTool)
	{
		try
		{
			*a_ppTool = NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
					return pTS->pfnCreateTool ? pTS->pfnCreateTool(a_ppTool) : E_NOTIMPL;
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppTool ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(IRasterImageEditToolsManager* UNREF(a_pManager), BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
	{
		try
		{
			*a_ppWindow = NULL;
			for (SToolSpec const* pTS = t_aShapeTools; pTS != t_aShapeTools+t_nShapeTools; ++pTS)
				if (wcscmp(a_bstrID, pTS->pszID) == 0)
					return pTS->pfnCreateToolWindow ? pTS->pfnCreateToolWindow(pTS->pszID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow) : E_FAIL;
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppWindow ? E_UNEXPECTED : E_POINTER;
		}
	}
};
