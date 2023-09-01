
#include "stdafx.h"
#include "RWOperationImageRaster.h"
#include <RWDrawing.h>
#include "EditToolEffect.h"

// {308C3732-CC24-4729-B6E8-736C4160CC61}
extern const GUID CLSID_RasterImageEditToolsEffect = {0x308c3732, 0xcc24, 0x4729, {0xb6, 0xe8, 0x73, 0x6c, 0x41, 0x60, 0xcc, 0x61}};


class ATL_NO_VTABLE CRasterImageEditToolsEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageEditToolsEffect, &CLSID_RasterImageEditToolsEffect>,
	public IRasterImageEditToolsFactory
{
public:
	CRasterImageEditToolsEffect()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageEditToolsEffect)

BEGIN_CATEGORY_MAP(CRasterImageEditToolsEffect)
	IMPLEMENTED_CATEGORY(CATID_RasterImageEditToolsFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageEditToolsEffect)
	COM_INTERFACE_ENTRY(IRasterImageEditToolsFactory)
END_COM_MAP()


	// IRasterImageEditToolsFactory methods
public:
	STDMETHOD(ToolIDsEnum)(IRasterImageEditToolsManager* a_pManager, IEnumStrings** a_ppToolIDs)
	{
		if (a_ppToolIDs == NULL)
			return E_POINTER;
		try
		{
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			pTmp->Insert(CComBSTR(L"EFFECT_"));
			*a_ppToolIDs = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToolNameGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppName)
	{
		if (a_ppName == NULL)
			return E_POINTER;
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				return a_pManager->ToolNameGet(bstrInternal, a_ppName);
				//CComPtr<ILocalizedString> pInternal;
				//HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
				//if (pInternal == NULL)
				//	return hRes;
				//CComObject<CLocalizedStringPrintf>* p = NULL;
				//CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
				//CComPtr<ILocalizedString> pTmp = p;
				//p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_CLONE), pInternal);
				//*a_ppName = pTmp.Detach();
				//return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToolDescGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppDesc)
	{
		if (a_ppDesc == NULL)
			return E_POINTER;
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				return a_pManager->ToolDescGet(bstrInternal, a_ppDesc);
				//CComPtr<ILocalizedString> pInternal;
				//HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
				//if (pInternal == NULL)
				//	return hRes;
				//CComObject<CLocalizedStringPrintf>* p = NULL;
				//CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
				//CComPtr<ILocalizedString> pTmp = p;
				//p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_CLONE), pInternal);
				//*a_ppName = pTmp.Detach();
				//return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToolIconIDGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, GUID* a_ptDefaultIcon)
	{
		if (a_ptDefaultIcon == NULL)
			return E_POINTER;
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
				//CComPtr<ILocalizedString> pInternal;
				//HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
				//if (pInternal == NULL)
				//	return hRes;
				//CComObject<CLocalizedStringPrintf>* p = NULL;
				//CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
				//CComPtr<ILocalizedString> pTmp = p;
				//p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_CLONE), pInternal);
				//*a_ppName = pTmp.Detach();
				//return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToolIconGet)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
	{
		if (a_phIcon == NULL)
			return E_POINTER;
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
				//CComPtr<ILocalizedString> pInternal;
				//HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
				//if (pInternal == NULL)
				//	return hRes;
				//CComObject<CLocalizedStringPrintf>* p = NULL;
				//CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
				//CComPtr<ILocalizedString> pTmp = p;
				//p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_CLONE), pInternal);
				//*a_ppName = pTmp.Detach();
				//return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SupportedStates)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
	{
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				if (a_pBlendingModes) *a_pBlendingModes = EBMDrawOver;
				return a_pManager->SupportedStates(bstrInternal, NULL, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(EditToolCreate)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool)
	{
		try
		{
			*a_ppTool = NULL;
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComBSTR bstrInternal(a_bstrID+7);
				CComPtr<IRasterImageEditTool> pInternal;
				HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
				if (pInternal == NULL)
					return hRes;
				CComObject<CEditToolEffect>* p = NULL;
				CComObject<CEditToolEffect>::CreateInstance(&p);
				CComPtr<IRasterImageEditTool> pTmp = p;
				p->Init(pInternal);
				*a_ppTool = pTmp.Detach();
				return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return a_ppTool ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(WindowCreate)(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
	{
		if (a_ppWindow == NULL)
			return E_POINTER;
		try
		{
			if (wcsncmp(a_bstrID, L"EFFECT_", 7) == 0)
			{
				CComObject<CEditToolEffectDlg>* p = NULL;
				HRESULT hRes = CComObject<CEditToolEffectDlg>::CreateInstance(&p);
				CComPtr<IRasterImageEditToolWindow> pTmp = p;
				if (p)
					if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+7))
						return E_FAIL;
				*a_ppWindow = pTmp.Detach();
				return S_OK;
			}
			return E_RW_ITEMNOTFOUND;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

//private:
//	struct SKey
//	{
//		SKey(GUID a_tID, int a_nType) : tID(a_tID), nType(a_nType) {}
//		bool operator<(SKey const& a_rhs) const
//		{
//			if (nType < a_rhs.nType)
//				return true;
//			if (nType > a_rhs.nType)
//				return false;
//			return memcmp(&tID, &a_rhs.tID, sizeof tID) < 0;
//		}
//		GUID tID;
//		int nType;
//	};
//	typedef std::map<SKey, GUID> CIconIds;
//
//	static HICON CreateBubbleIcon(HICON a_hSource);
//
//private:
//	CIconIds m_cIconIds;
};

OBJECT_ENTRY_AUTO(CLSID_RasterImageEditToolsEffect, CRasterImageEditToolsEffect)
