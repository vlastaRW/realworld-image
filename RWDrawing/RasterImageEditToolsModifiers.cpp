// RasterImageEditToolsModifiers.cpp : Implementation of CRasterImageEditToolsModifiers

#include "stdafx.h"
#include "RasterImageEditToolsModifiers.h"

#include <SharedStringTable.h>
#include <MultiLanguageString.h>
#include "EditToolEmboss.h"
#include "EditToolStylize.h"
#include "EditToolEraser.h"
#include "EditToolRetouch.h"
#include "EditToolAlpha.h"
#include "EditToolChannelMask.h"
#include "EditToolRecolor.h"


// CRasterImageEditToolsModifiers

STDMETHODIMP CRasterImageEditToolsModifiers::ToolIDsEnum(IRasterImageEditToolsManager* a_pManager, IEnumStrings** a_ppToolIDs)
{
	try
	{
		*a_ppToolIDs = NULL;
		CComPtr<IEnumStringsInit> pTmp;
		RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
		pTmp->Insert(CComBSTR(L"EMBOSSED_"));
		pTmp->Insert(CComBSTR(L"STYLIZE_"));
		pTmp->Insert(CComBSTR(L"ERASER_"));
		pTmp->Insert(CComBSTR(L"RETOUCH_"));
		pTmp->Insert(CComBSTR(L"ALPHA_"));
		pTmp->Insert(CComBSTR(L"MASK_"));
		pTmp->Insert(CComBSTR(L"RECOLOR_"));
		*a_ppToolIDs = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppToolIDs ? E_UNEXPECTED : E_POINTER;
	}
}

class CLocalizedStringPrintf :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ILocalizedString
{
public:
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_nInt = a_nInt;
		m_bUseInt = true;
		m_bStringFirst = true;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_nInt = a_nInt;
		m_bUseInt = true;
		m_bStringFirst = false;
	}
	void Init(ILocalizedString* a_pTemplate, ILocalizedString* a_pString)
	{
		m_pTemplate = a_pTemplate;
		m_pString = a_pString;
		m_bUseInt = false;
	}
	void Init(ILocalizedString* a_pTemplate, int a_nInt)
	{
		m_pTemplate = a_pTemplate;
		m_nInt = a_nInt;
		m_bUseInt = true;
	}

	BEGIN_COM_MAP(CLocalizedStringPrintf)
		COM_INTERFACE_ENTRY(ILocalizedString)
	END_COM_MAP()

	// ILocalizedString methods
public:
	STDMETHOD(Get)(BSTR* a_pbstrString)
	{
		return GetLocalized(GetThreadLocale(), a_pbstrString);
	}
	STDMETHOD(GetLocalized)(LCID a_tLCID, BSTR* a_pbstrString)
	{
		try
		{
			*a_pbstrString = NULL;

			CComBSTR bstrTemplate;
			m_pTemplate->GetLocalized(a_tLCID, &bstrTemplate);
			CComBSTR bstrString;
			if (m_pString)
				m_pString->GetLocalized(a_tLCID, &bstrString);
			CAutoVectorPtr<wchar_t> szBuffer(new wchar_t[bstrTemplate.Length()+bstrString.Length()+128]);
			if (m_bUseInt)
			{
				if (m_pString)
					if (m_bStringFirst)
						swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str, m_nInt);
					else
						swprintf(szBuffer.m_p, bstrTemplate, m_nInt, bstrString.m_str);
				else
					swprintf(szBuffer.m_p, bstrTemplate, m_nInt);
			}
			else
			{
				if (m_pString)
					swprintf(szBuffer.m_p, bstrTemplate, bstrString.m_str);
				else
					wcscpy(szBuffer.m_p, bstrTemplate);
			}
			*a_pbstrString = SysAllocString(szBuffer.m_p);
			return S_OK;
		}
		catch (...)
		{
			return a_pbstrString == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}

private:
	CComPtr<ILocalizedString> m_pTemplate;
	CComPtr<ILocalizedString> m_pString;
	int m_nInt;
	bool m_bUseInt;
	bool m_bStringFirst;
};

STDMETHODIMP CRasterImageEditToolsModifiers::ToolNameGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_EDITTOOL_EMBOSS), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else*/ if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_STYLIZE), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_ERASER), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_RETOUCH), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]Recolor (%s)[0405]Přebarvit (%s)"), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLNAME_ALPHA), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(CMultiLanguageString::GetAuto(L"[0409]%s with mask[0405]%s s maskou"), pInternal);
			*a_ppName = pTmp.Detach();
			return S_OK;
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsModifiers::ToolDescGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolNameGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLDESC_RETOUCH), pInternal);
			*a_ppDesc = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<ILocalizedString> pInternal;
			HRESULT hRes = a_pManager->ToolDescGet(bstrInternal, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CLocalizedStringPrintf>* p = NULL;
			CComObject<CLocalizedStringPrintf>::CreateInstance(&p);
			CComPtr<ILocalizedString> pTmp = p;
			p->Init(_SharedStringTable.GetStringAuto(IDS_TOOLDESC_STYLIZE), pInternal);
			*a_ppDesc = pTmp.Detach();
			return S_OK;
		}
		else
			return ToolNameGet(a_pManager, a_bstrID, a_ppDesc); // TODO: implement correctly
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsModifiers::ToolIconIDGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, GUID* a_ptDefaultIcon)
{
	try
	{
		*a_ptDefaultIcon = GUID_NULL;
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
		}
		else*/ if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			GUID tIcon = GUID_NULL;
			HRESULT hRes = a_pManager->ToolIconIDGet(bstrInternal, &tIcon);
			if (FAILED(hRes) || IsEqualGUID(tIcon, GUID_NULL))
				return hRes;
			SKey sKey(tIcon, 1);
			CIconIds::const_iterator i = m_cIconIds.find(sKey);
			if (i != m_cIconIds.end())
			{
				*a_ptDefaultIcon = i->second;
				return S_OK;
			}
			CoCreateGuid(a_ptDefaultIcon);
			m_cIconIds[sKey] = *a_ptDefaultIcon;
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			GUID tIcon = GUID_NULL;
			HRESULT hRes = a_pManager->ToolIconIDGet(bstrInternal, &tIcon);
			if (FAILED(hRes) || IsEqualGUID(tIcon, GUID_NULL))
				return hRes;
			SKey sKey(tIcon, 0);
			CIconIds::const_iterator i = m_cIconIds.find(sKey);
			if (i != m_cIconIds.end())
			{
				*a_ptDefaultIcon = i->second;
				return S_OK;
			}
			CoCreateGuid(a_ptDefaultIcon);
			m_cIconIds[sKey] = *a_ptDefaultIcon;
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			static const GUID tID = {0x1aef148, 0xc1f1, 0x45c1, {0x93, 0x99, 0x70, 0xb3, 0x8f, 0xe8, 0x43, 0xfb}};
			*a_ptDefaultIcon = tID;
			return S_OK;
			//CComBSTR bstrInternal(a_bstrID+8);
			//return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			// {B8CC1643-F843-42f8-BC88-1AE3D77B5D7B}
			static const GUID tID = {0xb8cc1643, 0xf843, 0x42f8, {0xbc, 0x88, 0x1a, 0xe3, 0xd7, 0x7b, 0x5d, 0x7b}};
			*a_ptDefaultIcon = tID;
			return S_OK;
			//CComBSTR bstrInternal(a_bstrID+8);
			//return a_pManager->ToolIconIDGet(bstrInternal, a_ptDefaultIcon);
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
	}
}

HICON GetToolIconRETOUCH(ULONG a_nSize)
{
	IRPathPoint const hand[] =
	{
		{218.164, 153, -11.2715, 50.5041, 11.8588, -52.565},
		{177.189, 245.296, -3.47375, 17.5834, 7.82928, -39.6314},
		{96.5888, 229.393, -0.884949, -30.9402, 0.784851, 27.434},
		{35.2739, 133.509, -18.3973, -26.3281, 15.1306, 21.9433},
		{89.9835, 131.105, -1.83124, -45.0869, -35.5508, -44.8714},
		{95.357, 19.3456, 3.992, -26.3622, -5.40466, 19.7365},
		{132.273, 25.7294, 2.71356, 24.2522, -3.56903, -33.0319},
		{128.947, 105.163, 3.7057, -32.5972, 2.43494, -28.1589},
		{151.31, 58.0935, 18.564, 4.84558, -18.9901, -6.40155},
		{159.019, 109.741, 5.72041, -27.1066, 3.23911, -34.2172},
		{183.618, 70.0808, 18.6501, 6.88672, -17.7802, -6.63904},
		{189.446, 118.767, 7.10577, -31.5438, 3.45309, -26.6057},
		{213.992, 83.9396, 14.8607, 4.59717, -13.1721, -3.56964},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(hand), hand, pSI->GetMaterial(ESMScheme2Color1));
	return cRenderer.get();
}

HICON GetToolIconRECOLOR(ULONG a_nSize)
{
	IRPathPoint const hair[] =
	{
		{55, 195, 0, 0, 0, 0},
		{83, 163, -19.0063, 18.1042, -13.1801, 17.8685},
		{47, 191, 0, 0, 0, 0},
		{3, 167, 27.3027, -12.4323, 0, 0},
		{78, 95, 0, 0, -8.79219, 20.2657},
		{224, 174, -15.7398, 26.7256, 0, 0},
		{166, 256, 0, 0, 22.281, -22.0127},
		{130, 236, 0, 0, 0, 0},
		{144, 216, -11.5793, 10.7666, -7.11011, 10.9291},
		{123, 232, 0, 0, 0, 0},
	};
	IRPolyPoint const mid[] =
	{
		{84, 77}, {234, 164}, {217, 194}, {67, 107},
	};
	IRPathPoint const handle[] =
	{
		{96, 66, 16.2923, -17.3078, -10.6406, 11.3039},
		{161, 66, 18.6976, -34.2257, -8.64299, 15.8209},
		{221, 6, 39.4912, 21.5742, -39.4912, -21.5742},
		{203, 89, -8.64299, 15.8209, 18.6976, -34.2257},
		{238, 144, -3.76274, 15.0613, 5.76129, -23.0609},
		{225, 173, 0, 0, 0, 0},
		{79, 93, 0, 0, 0, 0},
	};

	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(hair), hair, pSI->GetMaterial(ESMScheme2Color1));
	cRenderer(&canvas, itemsof(handle), handle, pSI->GetMaterial(ESMScheme2Color2));
	cRenderer(&canvas, itemsof(mid), mid, pSI->GetMaterial(ESMAltBackground));
	return cRenderer.get();
}

STDMETHODIMP CRasterImageEditToolsModifiers::ToolIconGet(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
		}
		else*/ if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			HICON hIcon = NULL;
			HRESULT hRes = a_pManager->ToolIconGet(bstrInternal, a_nSize, &hIcon);
			if (FAILED(hRes) || hIcon == NULL)
				return hRes;
			*a_phIcon = CreateStylizeIcon(hIcon);
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			HICON hIcon = NULL;
			HRESULT hRes = a_pManager->ToolIconGet(bstrInternal, a_nSize, &hIcon);
			if (FAILED(hRes) || hIcon == NULL)
				return hRes;
			*a_phIcon = CreateEraserIcon(hIcon);
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			*a_phIcon = GetToolIconRETOUCH(a_nSize);//(HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_EDITTOOL_RETOUCH), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return S_OK;
			//CComBSTR bstrInternal(a_bstrID+8);
			//return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			*a_phIcon = GetToolIconRECOLOR(a_nSize);//(HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_EDITTOOL_RETOUCH), IMAGE_ICON, a_nSize, a_nSize, LR_DEFAULTCOLOR);
			return S_OK;
			//CComBSTR bstrInternal(a_bstrID+8);
			//return a_pManager->ToolIconGet(bstrInternal, a_nSize, a_phIcon);
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsModifiers::SupportedStates(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
{
	try
	{
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pShapeFillModes, a_pOutline, a_pGammaOverride);
			return hRes;
		}
		else*/ if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
			return hRes;
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
			return hRes;
		}
		else if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
			if (a_pBlendingModes) *a_pBlendingModes = EBMDrawOver|EBMReplace;
			return hRes;
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, NULL);
			if (a_pBlendingModes) *a_pBlendingModes = EBMReplace;
			return hRes;
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, NULL);
			if (a_pPaintSpecs)
			{
				static ULONG const n = ETBTIdentifierInterior|ETBTSingleColor;
				a_pPaintSpecs->Consume(0, 1, &n);
			}
			if (a_pBlendingModes) *a_pBlendingModes = 0;
			return hRes;
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			HRESULT hRes = a_pManager->SupportedStates(bstrInternal, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, NULL);
			if (a_pPaintSpecs)
			{
				static ULONG const n = ETBTIdentifierInterior|ETBTSingleColor;
				a_pPaintSpecs->Consume(0, 1, &n);
			}
			if (a_pBlendingModes) *a_pBlendingModes = 0;
			return hRes;
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageEditToolsModifiers::EditToolCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool)
{
	try
	{
		*a_ppTool = NULL;
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolEmboss>* p = NULL;
			CComObject<CEditToolEmboss>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else*/ if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolStylize>* p = NULL;
			CComObject<CEditToolStylize>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolEraser>* p = NULL;
			CComObject<CEditToolEraser>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolAlpha>* p = NULL;
			CComObject<CEditToolAlpha>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolChannelMask>* p = NULL;
			CComObject<CEditToolChannelMask>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolRetouch>* p = NULL;
			CComObject<CEditToolRetouch>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+8);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolRecolor>* p = NULL;
			CComObject<CEditToolRecolor>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ppTool ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsModifiers::WindowCreate(IRasterImageEditToolsManager* a_pManager, BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		/*if (wcsncmp(a_bstrID, L"EMBOSSED_", 9) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+9);
			CComPtr<IRasterImageEditTool> pInternal;
			HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			if (pInternal == NULL)
				return hRes;
			CComObject<CEditToolEmboss>* p = NULL;
			CComObject<CEditToolEmboss>::CreateInstance(&p);
			CComPtr<IRasterImageEditTool> pTmp = p;
			p->Init(pInternal);
			*a_ppTool = pTmp.Detach();
			return S_OK;
		}
		else*/ if (wcsncmp(a_bstrID, L"STYLIZE_", 8) == 0)
		{
			CComObject<CEditToolStylizeDlg>* p = NULL;
			HRESULT hRes = CComObject<CEditToolStylizeDlg>::CreateInstance(&p);
			CComPtr<IRasterImageEditToolWindow> pTmp = p;
			if (p)
				if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+8))
					return E_FAIL;
			*a_ppWindow = pTmp.Detach();
			return S_OK;

			//CComBSTR bstrInternal(a_bstrID+6);
			//CComPtr<IRasterImageEditTool> pInternal;
			//HRESULT hRes = a_pManager->EditToolCreate(bstrInternal, a_pDocument, &pInternal);
			//if (pInternal == NULL)
			//	return hRes;
			//CComObject<CEditToolStylize>* p = NULL;
			//CComObject<CEditToolStylize>::CreateInstance(&p);
			//CComPtr<IRasterImageEditTool> pTmp = p;
			//p->Init(pInternal);
			//*a_ppTool = pTmp.Detach();
			//return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"ERASER_", 7) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+7);
			return a_pManager->WindowCreate(bstrInternal, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
		}
		else if (wcsncmp(a_bstrID, L"ALPHA_", 6) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+6);
			return a_pManager->WindowCreate(bstrInternal, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
		}
		else if (wcsncmp(a_bstrID, L"MASK_", 5) == 0)
		{
			CComBSTR bstrInternal(a_bstrID+5);
			return a_pManager->WindowCreate(bstrInternal, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
		}
		else if (wcsncmp(a_bstrID, L"RETOUCH_", 8) == 0)
		{
			CComObject<CEditToolRetouchDlg>* p = NULL;
			HRESULT hRes = CComObject<CEditToolRetouchDlg>::CreateInstance(&p);
			CComPtr<IRasterImageEditToolWindow> pTmp = p;
			if (p)
				if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+8))
					return E_FAIL;
			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		else if (wcsncmp(a_bstrID, L"RECOLOR_", 8) == 0)
		{
			CComObject<CEditToolRecolorDlg>* p = NULL;
			HRESULT hRes = CComObject<CEditToolRecolorDlg>::CreateInstance(&p);
			CComPtr<IRasterImageEditToolWindow> pTmp = p;
			if (p)
				if (NULL == p->Create(a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_pManager, a_bstrID+8))
					return E_FAIL;
			*a_ppWindow = pTmp.Detach();
			return S_OK;
		}
		else
			return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return a_ppWindow ? E_UNEXPECTED : E_POINTER;
	}
}

HICON CRasterImageEditToolsModifiers::CreateEraserIcon(HICON a_hSource)
{
	if (a_hSource == NULL)
		return NULL;
	ICONINFO tInfo;
	ZeroMemory(&tInfo, sizeof tInfo);
	GetIconInfo(a_hSource, &tInfo);
	if (tInfo.hbmMask)
		DeleteObject(tInfo.hbmMask);
	if (tInfo.hbmColor == NULL)
		return a_hSource;
	BITMAP tBmp;
	ZeroMemory(&tBmp, sizeof tBmp);
	GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
	if (tBmp.bmBitsPixel != 32)
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // ignore low quality icons
	}
	int nSmSizeX = tBmp.bmWidth;
	int nSmSizeY = tBmp.bmHeight;
	DWORD nXOR = nSmSizeY*nSmSizeX<<2;
	DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pIconRes.m_p+sizeof BITMAPINFOHEADER))
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // failure
	}

	DeleteObject(tInfo.hbmColor);

	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pHead->biSize = sizeof(BITMAPINFOHEADER);
	pHead->biWidth = nSmSizeX;
	pHead->biHeight = nSmSizeY<<1;
	pHead->biPlanes = 1;
	pHead->biBitCount = 32;
	pHead->biCompression = BI_RGB;
	pHead->biSizeImage = nXOR+nAND;
	pHead->biXPelsPerMeter = 0;
	pHead->biYPelsPerMeter = 0;
	TRasterImagePixel *pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes.m_p+sizeof BITMAPINFOHEADER);
	COLORREF clr = GetSysColor(COLOR_3DLIGHT);
	TRasterImagePixel t1 = *reinterpret_cast<TRasterImagePixel*>(&clr);
	clr = GetSysColor(COLOR_3DSHADOW);
	TRasterImagePixel t2 = *reinterpret_cast<TRasterImagePixel*>(&clr);
	for (int y = 0; y < tBmp.bmHeight; ++y)
	{
		for (int x = 0; x < tBmp.bmWidth; ++x)
		{
			TRasterImagePixel const tColor = ((x%(nSmSizeX>>1))>=(nSmSizeX>>2)) == ((y%(nSmSizeY>>1))>=(nSmSizeY>>2)) ? t1 : t2;
			ULONG const n = max(pXOR->bG, max(pXOR->bR, pXOR->bB));//5*pXOR->bG + 3*pXOR->bR + 2*pXOR->bB;
			ULONG const nR = (tColor.bB*n)/220;
			ULONG const nG = (tColor.bG*n)/220;
			ULONG const nB = (tColor.bR*n)/220;
			pXOR->bR = nR > 255 ? 255 : nR;
			pXOR->bG = nG > 255 ? 255 : nG;
			pXOR->bB = nB > 255 ? 255 : nB;
			++pXOR;
		}
	}
	pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes+sizeof BITMAPINFOHEADER);
	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
	int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	for (int y = 0; y < nSmSizeY; ++y)
	{
		if (y < (nSmSizeY>>1))
		{
			for (int x = 0; x < nSmSizeX; ++x)
			{
				TRasterImagePixel t = pXOR[x];
				pXOR[x] = pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x];
				pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x] = t;
			}
		}
		for (int x = 0; x < nSmSizeX; ++x)
		{
			if (0 == pXOR->bA)
			{
				pAND[x>>3] |= 0x80 >> (x&7);
			}
			++pXOR;
		}
		pAND += nANDLine;
	}
	HICON hTmp = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	DestroyIcon(a_hSource);
	return hTmp;
}

#include "EditToolPixelMixer.h"

HICON CRasterImageEditToolsModifiers::CreateStylizeIcon(HICON a_hSource)
{
	// get source icon image
	if (a_hSource == NULL)
		return NULL;
	ICONINFO tInfo;
	ZeroMemory(&tInfo, sizeof tInfo);
	GetIconInfo(a_hSource, &tInfo);
	if (tInfo.hbmMask)
		DeleteObject(tInfo.hbmMask);
	if (tInfo.hbmColor == NULL)
		return a_hSource;
	BITMAP tBmp;
	ZeroMemory(&tBmp, sizeof tBmp);
	GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
	if (tBmp.bmBitsPixel != 32)
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // ignore low quality icons
	}
	int nSmSizeX = tBmp.bmWidth;
	int nSmSizeY = tBmp.bmHeight;
	DWORD nXOR = nSmSizeY*nSmSizeX<<2;
	DWORD nAND = nSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pIconRes.m_p+sizeof BITMAPINFOHEADER))
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // failure
	}

	DeleteObject(tInfo.hbmColor);

	// prepare overlay
	HICON hSparkle = (HICON)LoadImage(_pModule->get_m_hInst(), MAKEINTRESOURCE(IDI_EDITTOOL_STYLIZE), IMAGE_ICON, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	if (hSparkle == NULL)
		return a_hSource;
	ZeroMemory(&tInfo, sizeof tInfo);
	GetIconInfo(hSparkle, &tInfo);
	if (tInfo.hbmMask)
		DeleteObject(tInfo.hbmMask);
	if (tInfo.hbmColor == NULL)
		return a_hSource;
	ZeroMemory(&tBmp, sizeof tBmp);
	GetObject(tInfo.hbmColor, sizeof tBmp, &tBmp);
	if (tBmp.bmBitsPixel != 32)
	{
		DeleteObject(tInfo.hbmColor);
		return hSparkle; // ignore low quality icons
	}
	int nPatSmSizeX = tBmp.bmWidth;
	int nPatSmSizeY = tBmp.bmHeight;
	DWORD nPatXOR = nPatSmSizeY*nSmSizeX<<2;
	DWORD nPatAND = nPatSmSizeY*((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	CAutoVectorPtr<BYTE> pPatIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
	ZeroMemory(pPatIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);

	if (0 == GetBitmapBits(tInfo.hbmColor, tBmp.bmWidth*tBmp.bmHeight<<2, pPatIconRes.m_p+sizeof BITMAPINFOHEADER))
	{
		DeleteObject(tInfo.hbmColor);
		return a_hSource; // failure
	}

	DeleteObject(tInfo.hbmColor);

	if (nPatSmSizeX != nSmSizeX || nPatSmSizeY != nSmSizeY)
		return a_hSource;

	BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
	pHead->biSize = sizeof(BITMAPINFOHEADER);
	pHead->biWidth = nSmSizeX;
	pHead->biHeight = nSmSizeY<<1;
	pHead->biPlanes = 1;
	pHead->biBitCount = 32;
	pHead->biCompression = BI_RGB;
	pHead->biSizeImage = nXOR+nAND;
	pHead->biXPelsPerMeter = 0;
	pHead->biYPelsPerMeter = 0;
	TRasterImagePixel *pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes.m_p+sizeof BITMAPINFOHEADER);
	TRasterImagePixel *pPatXOR = reinterpret_cast<TRasterImagePixel *>(pPatIconRes.m_p+sizeof BITMAPINFOHEADER);
	for (int y = 0; y < tBmp.bmHeight; ++y)
	{
		for (int x = 0; x < tBmp.bmWidth; ++x)
		{
			CPixelMixerPaintOver::Mix(*pXOR, *pPatXOR);
			++pXOR;
			++pPatXOR;
		}
	}
	pXOR = reinterpret_cast<TRasterImagePixel *>(pIconRes+sizeof BITMAPINFOHEADER);
	BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(nSmSizeY*nSmSizeX));
	int nANDLine = ((((nSmSizeX+7)>>3)+3)&0xfffffffc);
	for (int y = 0; y < nSmSizeY; ++y)
	{
		if (y < (nSmSizeY>>1))
		{
			for (int x = 0; x < nSmSizeX; ++x)
			{
				TRasterImagePixel t = pXOR[x];
				pXOR[x] = pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x];
				pXOR[(nSmSizeY-y-y-1)*nSmSizeX+x] = t;
			}
		}
		for (int x = 0; x < nSmSizeX; ++x)
		{
			if (0 == pXOR->bA)
			{
				pAND[x>>3] |= 0x80 >> (x&7);
			}
			++pXOR;
		}
		pAND += nANDLine;
	}
	HICON hTmp = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, nSmSizeX, nSmSizeY, LR_DEFAULTCOLOR);
	DestroyIcon(a_hSource);
	return hTmp;
}
