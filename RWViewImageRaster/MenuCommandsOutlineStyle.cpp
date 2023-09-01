// MenuCommandsOutlineStyle.cpp : Implementation of CMenuCommandsOutlineStyle

#include "stdafx.h"
#include "MenuCommandsOutlineStyle.h"

#include <SharedStringTable.h>
#include <SimpleLocalizedString.h>
#include "ConfigIDsEditToolProperties.h"
#include "ConfigGUIEditToolProperties.h"
#include <RWBaseEnumUtils.h>
#include "SharedStateToolMode.h"


// CMenuCommandsOutlineStyle

STDMETHODIMP CMenuCommandsOutlineStyle::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Image Editor - Outline Style[0405]Obrázkový editor - styl obrysu");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_TOOLCOLORSYNC_DESC), CConfigValue(L"FILLSTYLE"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DesignerViewFactoryEditToolProperties, CConfigGUIEditToolPropertiesDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

extern const GUID IconIDShapeOutline = {0xc4517858, 0x480a, 0x4099, {0x91, 0x57, 0xee, 0x55, 0x4c, 0x9f, 0x17, 0xe2}};
extern const GUID IconIDShapeFilled = {0x82e17235, 0x2b6e, 0x4b61, {0x8a, 0xed, 0xb0, 0xc1, 0x21, 0x7d, 0xb9, 0x4}};
extern const GUID IconIDShapeCombined = {0x1411aa22, 0x1fa6, 0x4ce1, {0x9c, 0xbb, 0x39, 0x10, 0xc8, 0x85, 0xb8, 0xe2}};
extern const GUID IconIDOutlineInc = {0x97363f10, 0x44d, 0x4fe1, {0xbf, 0x8b, 0x8d, 0xf2, 0x8, 0xcd, 0x16, 0x37}};
extern const GUID IconIDOutlineDec = {0x16db42fe, 0x2f2a, 0x47fa, {0xb4, 0xfa, 0xe5, 0xec, 0x51, 0x6b, 0xcf, 0x97}};

STDMETHODIMP CMenuCommandsOutlineStyle::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* UNREF(a_pView), IDocument* UNREF(a_pDocument), IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TOOLPROPS_TOOLSTATESYNC), &cSyncID);
		if (cSyncID.operator BSTR() == NULL || cSyncID.operator BSTR()[0] == L'\0')
			return S_FALSE;

		bool bOutline = false;
		bool bOthers = false;
		CComPtr<ISharedStateToolMode> pMode;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pMode));
		BOOL bEnabled = FALSE;
		if (pMode)
		{
			CComBSTR bstrToolID;
			pMode->Get(&bstrToolID, NULL, NULL, NULL, NULL, NULL, &bEnabled, NULL, NULL, NULL, NULL);
			std::vector<ULONG> aPaints;
			M_Manager()->SupportedStates(bstrToolID, NULL, NULL, NULL, &CEnumToVector<IEnum2UInts, ULONG>(aPaints));
			for (std::vector<ULONG>::const_iterator i = aPaints.begin(); i != aPaints.end(); ++i)
			{
				if ((*i&ETBTIdentifierMask) != ETBTIdentifierOutline)
					continue;
				bOutline = true;
				//if ((*i&ETBTTypeMask) == ETBTRasterImage)
				//	bRasterImage = true;
			}
			bOthers = aPaints.size() > 1;
		}
		if (!bOutline)
			return S_FALSE;

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		if (bOthers)
		{
			{
				CComObject<CDocumentMenuNoOutlineStyle>* p = NULL;
				CComObject<CDocumentMenuNoOutlineStyle>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, !bEnabled);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuSolidOutlineStyle>* p = NULL;
				CComObject<CDocumentMenuSolidOutlineStyle>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pStates, cSyncID, bEnabled, M_StyleManager());
				pItems->Insert(pTmp);
			}
			CComPtr<IDocumentMenuCommand> pSep;
			RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
			pItems->Insert(pSep);
		}

		{
			CComObject<CDocumentMenuOutlineWidth<IDS_MENU_OUTLINEDEC_NAME, IDS_MENU_OUTLINEDEC_DESC, &IconIDOutlineDec> >* p = NULL;
			CComObject<CDocumentMenuOutlineWidth<IDS_MENU_OUTLINEDEC_NAME, IDS_MENU_OUTLINEDEC_DESC, &IconIDOutlineDec> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, -1.0f, bOutline);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuOutlineWidthPopup>* p = NULL;
			CComObject<CDocumentMenuOutlineWidthPopup>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuOutlineWidth<IDS_MENU_OUTLINEINC_NAME, IDS_MENU_OUTLINEINC_DESC, &IconIDOutlineInc> >* p = NULL;
			CComObject<CDocumentMenuOutlineWidth<IDS_MENU_OUTLINEINC_NAME, IDS_MENU_OUTLINEINC_DESC, &IconIDOutlineInc> >::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, 1.0f, bOutline);
			pItems->Insert(pTmp);
		}

		CComPtr<IDocumentMenuCommand> pSep;
		RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
		pItems->Insert(pSep);

		{
			CComObject<CDocumentMenuConstantOutlinePosition>* p = NULL;
			CComObject<CDocumentMenuConstantOutlinePosition>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, -1.0f);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuConstantOutlinePosition>* p = NULL;
			CComObject<CDocumentMenuConstantOutlinePosition>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, 0.0f);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuConstantOutlinePosition>* p = NULL;
			CComObject<CDocumentMenuConstantOutlinePosition>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, 1.0f);
			pItems->Insert(pTmp);
		}

		pItems->Insert(pSep);

		{
			CComObject<CDocumentMenuOutlineJoins>* p = NULL;
			CComObject<CDocumentMenuOutlineJoins>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, EOJTMiter);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuOutlineJoins>* p = NULL;
			CComObject<CDocumentMenuOutlineJoins>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, EOJTRound);
			pItems->Insert(pTmp);
		}
		{
			CComObject<CDocumentMenuOutlineJoins>* p = NULL;
			CComObject<CDocumentMenuOutlineJoins>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, cSyncID, bOutline, EOJTBevel);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuNoOutlineStyle::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		BOOL bOutline = FALSE;
		BOOL bFill = TRUE;
		if (S_OK == pNew->Set(NULL, &bFill, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuSolidOutlineStyle::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		CComPtr<ISharedState> pTmp = pNew;
		BOOL bOutline = TRUE;
		if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuSolidOutlineStyle::IconID(GUID* a_pIconID)
{
	return m_pManager->StyleIconIDGet(m_bstrStyle, a_pIconID);
}
STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuSolidOutlineStyle::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	return m_pManager->StyleIconGet(m_bstrStyle, a_nSize, a_phIcon);
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineWidthPopup::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		float fWidth = 0.0f;
		if (pPrev)
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fWidth, NULL, NULL);
		wchar_t szTmp[16];
		if (fWidth >= 100.0f)
			swprintf(szTmp, L"%i px", static_cast<int>(fWidth+0.5f));
		else if (fWidth >= 10.f)
			swprintf(szTmp, L"%g px", 0.1f*static_cast<int>(fWidth*10.0f+0.5f));
		else
			swprintf(szTmp, L"%g px", 0.01f*static_cast<int>(fWidth*100.0f+0.5f));
		*a_ppText = new CSimpleLocalizedString(SysAllocString(szTmp));
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineWidthPopup::Description(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		*a_ppText = _SharedStringTable.GetString(IDS_MENU_OWPOPUP_DESC);
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineWidthPopup::SubCommands(IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;
		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		static float aConstVals[] = {/*0.5f, */1.0f, 2.0f, 5.0f, 10.0f, 20.0f};
		for (float* pVal = aConstVals; pVal != aConstVals+itemsof(aConstVals); ++pVal)
		{
			CComObject<CDocumentMenuConstantOutlineWidth>* p = NULL;
			CComObject<CDocumentMenuConstantOutlineWidth>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pCmd = p;
			p->Init(m_pStates, m_bstrSyncID, m_bEnabled, *pVal);
			pItems->Insert(pCmd);
		}
		//{
		//	CComPtr<IDocumentMenuCommand> pTmp;
		//	RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
		//	pItems->Insert(pTmp);
		//}
		//{
		//	CComObject<CDocumentMenuCustomZoom>* p = NULL;
		//	CComObject<CDocumentMenuCustomZoom>::CreateInstance(&p);
		//	CComPtr<IDocumentMenuCommand> pTmp = p;
		//	p->Init(m_pView);
		//	pItems->Insert(pTmp);
		//}
		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlineWidth::Name(ILocalizedString** a_ppText)
{
	try
	{
		*a_ppText = NULL;
		wchar_t szTmp[16];
		swprintf(szTmp, L"%g px", m_fWidth);
		*a_ppText = new CSimpleLocalizedString(SysAllocString(szTmp));
		return S_OK;
	}
	catch (...)
	{
		return a_ppText ? E_UNEXPECTED : E_POINTER;
	}
}

EMenuCommandState CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlineWidth::IntState()
{
	if (!m_bEnabled)
		return EMCSDisabled;
	CComPtr<ISharedStateToolMode> pPrev;
	m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
	float fWidth = 0.0f;
	if (pPrev == NULL || FAILED(pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fWidth, NULL, NULL)))
		return EMCSDisabled;
	return fabsf(fWidth-m_fWidth)<1e-4 ? EMCSRadioChecked : EMCSNormal;
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlineWidth::Execute(RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		// enable outline if it was not enabled prior to selection of outline width
		BOOL bOutline = 1;
		BOOL* pOutline = NULL;
		if (pPrev)
		{
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL);
			if (bOutline == 0)
			{
				bOutline = 1;
				pOutline = &bOutline;
			}
		}
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, &m_fWidth, NULL, NULL, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlinePosition::IconID(GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	if (m_fPos > 0.0f)
	{
		// {40983109-50FE-4829-B2DA-B90FC68F8E2A}
		static const GUID tID = {0x40983109, 0x50fe, 0x4829, {0xb2, 0xda, 0xb9, 0xf, 0xc6, 0x8f, 0x8e, 0x2a}};
		*a_pIconID = tID;
	}
	else if (m_fPos < 0.0f)
	{
		// {40983109-50FE-4829-B2DA-B90FC68F8E28}
		static const GUID tID = {0x40983109, 0x50fe, 0x4829, {0xb2, 0xda, 0xb9, 0xf, 0xc6, 0x8f, 0x8e, 0x28}};
		*a_pIconID = tID;
	}
	else
	{
		// {40983109-50FE-4829-B2DA-B90FC68F8E29}
		static const GUID tID = {0x40983109, 0x50fe, 0x4829, {0xb2, 0xda, 0xb9, 0xf, 0xc6, 0x8f, 0x8e, 0x29}};
		*a_pIconID = tID;
	}
	return S_OK;
}

#define AUTOCANVASSUPPORT
#include <IconRenderer.h>

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlinePosition::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	if (a_nSize == 0)
		return E_RW_INVALIDPARAM;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
		if (m_fPos > 0.0f)
		{
			static IRPathPoint const shape[] =
			{
				{248, 101, 0, 0, 0, 0},
				{217, 93, 0, 0, 0, 0},
				{203, 143, 18.8799, 7.7466, 0, 0},
				{255, 174, 0, 0, -15.5096, -12.9472},
				{225, 211, -26.1961, -21.8679, 0, 0},
				{128, 176, -36.7922, 0, 36.7922, 0},
				{31, 211, 0, 0, 26.1961, -21.8679},
				{1, 174, 15.5095, -12.9471, 0, 0},
				{53, 143, 0, 0, -18.88, 7.74662},
				{39, 93, 0, 0, 0, 0},
				{8, 101, 0, 0, 0, 0},
				{48, 32, 0, 0, 0, 0},
				{116, 72, 0, 0, 0, 0},
				{86, 80, 0, 0, 0, 0},
				{99, 130, 9.45613, -1.37728, 0, 0},
				{128, 128, 9.87097, 0, -9.87095, 0},
				{157, 130, 0, 0, -9.45611, -1.37725},
				{170, 80, 0, 0, 0, 0},
				{140, 72, 0, 0, 0, 0},
				{208, 32, 0, 0, 0, 0},
			};
			cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior));
		}
		else if (m_fPos < 0.0f)
		{
			static IRPathPoint const shape[] =
			{
				{255, 114, 0, 0, -34.5234, -28.8196},
				{225, 151, 0, 0, 0, 0},
				{217, 145, 0, 0, 0, 0},
				{198, 179, 0, 0, 0, 0},
				{225, 195, 0, 0, 0, 0},
				{149, 215, 0, 0, 0, 0},
				{129, 139, 0, 0, 0, 0},
				{157, 155, 0, 0, 0, 0},
				{175, 123, -14.7303, -4.74389, 0, 0},
				{128, 116, -16.3145, 0, 16.2487, 0},
				{81, 123, 0, 0, 14.782, -4.78084},
				{99, 155, 0, 0, 0, 0},
				{127, 139, 0, 0, 0, 0},
				{107, 215, 0, 0, 0, 0},
				{31, 195, 0, 0, 0, 0},
				{58, 179, 0, 0, 0, 0},
				{39, 145, 0, 0, 0, 0},
				{31, 151, 0, 0, 0, 0},
				{1, 114, 34.5233, -28.8194, 0, 0},
				{128, 68, 48.4882, 0, -48.4879, 0},
			};
			cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior));
		}
		else
		{
			static IRPathPoint const shape[] =
			{
				{184, 60, 0, 0, 0, 0},
				{152, 60, 0, 0, 0, 0},
				{152, 101.438, 38.9803, 4.66782, 0, 0},
				{255, 146, 0, 0, -28.7313, -23.9845},
				{225, 183, -20.3872, -17.0188, 0, 0},
				{152, 149.956, 0, 0, 27.4813, 4.36431},
				{152, 196, 0, 0, 0, 0},
				{184, 196, 0, 0, 0, 0},
				{128, 252, 0, 0, 0, 0},
				{72, 196, 0, 0, 0, 0},
				{104, 196, 0, 0, 0, 0},
				{104, 149.956, -27.4813, 4.36431, 0, 0},
				{31, 183, 0, 0, 20.3872, -17.0188},
				{1, 146, 28.7312, -23.9843, 0, 0},
				{104, 101.438, 0, 0, -38.9802, 4.66788},
				{104, 60, 0, 0, 0, 0},
				{72, 60, 0, 0, 0, 0},
				{128, 4, 0, 0, 0, 0},
			};
			cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior));
		}
		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlinePosition::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSDisabled;
		if (!m_bEnabled)
			return S_OK;
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		float fPos = 0.0f;
		if (pPrev == NULL || FAILED(pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fPos, NULL)))
			return EMCSDisabled;
		*a_peState = fabsf(fPos-m_fPos)<1e-4 ? EMCSRadioChecked : EMCSNormal;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuConstantOutlinePosition::Execute(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		// enable outline if it was not enabled prior to selection of outline position (or remove this and keep it only for width?)
		BOOL bOutline = 1;
		BOOL* pOutline = NULL;
		if (pPrev)
		{
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL);
			if (bOutline == 0)
			{
				bOutline = 1;
				pOutline = &bOutline;
			}
		}
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, &m_fPos, NULL, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineJoins::IconID(GUID* a_pIconID)
{
	if (a_pIconID == NULL)
		return E_POINTER;
	if (m_eJoins == EOJTBevel)
	{
		// {F808CA1A-68BE-4cb2-8FB1-4C2C2533BC34}
		static const GUID tID = {0xf808ca1a, 0x68be, 0x4cb2, {0x8f, 0xb1, 0x4c, 0x2c, 0x25, 0x33, 0xbc, 0x34}};
		*a_pIconID = tID;
	}
	else if (m_eJoins == EOJTRound)
	{
		// {F808CA1A-68BE-4cb2-8FB1-4C2C2533BC33}
		static const GUID tID = {0xf808ca1a, 0x68be, 0x4cb2, {0x8f, 0xb1, 0x4c, 0x2c, 0x25, 0x33, 0xbc, 0x33}};
		*a_pIconID = tID;
	}
	else
	{
		// {F808CA1A-68BE-4cb2-8FB1-4C2C2533BC32}
		static const GUID tID = {0xf808ca1a, 0x68be, 0x4cb2, {0x8f, 0xb1, 0x4c, 0x2c, 0x25, 0x33, 0xbc, 0x32}};
		*a_pIconID = tID;
	}
	return S_OK;
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineJoins::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	if (a_nSize == 0)
		return E_RW_INVALIDPARAM;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CComPtr<IIconRenderer> pIR;
		RWCoCreateInstance(pIR, __uuidof(IconRenderer));
		if (m_eJoins == EOJTBevel)
		{
			static IRPolyPoint const s_aBevelJoins[] =
			{ {0.1f, 0.2f}, {0.4f, 0.2f}, {0.8f, 0.6f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
			*a_phIcon = IconFromPolygon(pSI, pIR, itemsof(s_aBevelJoins), s_aBevelJoins, a_nSize, false);
		}
		else if (m_eJoins == EOJTRound)
		{
			static IRPolyPoint const s_aRoundJoins[] =
			{ {0.1f, 0.2f}, {0.2f, 0.2f}, {0.4296f, 0.2457f}, {0.6243f, 0.3757f}, {0.7543f, 0.5704f}, {0.8f, 0.8f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
			*a_phIcon = IconFromPolygon(pSI, pIR, itemsof(s_aRoundJoins), s_aRoundJoins, a_nSize, false);
		}
		else
		{
			static IRPolyPoint const s_aMiterJoins[] =
			{ {0.1f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.9f}, {0.3f, 0.9f}, {0.3f, 0.7f}, {0.1f, 0.7f} };
			*a_phIcon = IconFromPolygon(pSI, pIR, itemsof(s_aMiterJoins), s_aMiterJoins, a_nSize, false);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineJoins::State(EMenuCommandState* a_peState)
{
	try
	{
		*a_peState = EMCSDisabled;
		if (!m_bEnabled)
			return S_OK;
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		EOutlineJoinType eJoins = EOJTRound;
		if (pPrev == NULL || FAILED(pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &eJoins)))
			return EMCSDisabled;
		*a_peState = eJoins == m_eJoins ? EMCSRadioChecked : EMCSNormal;
		return S_OK;
	}
	catch (...)
	{
		return a_peState ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsOutlineStyle::CDocumentMenuOutlineJoins::Execute(RWHWND UNREF(a_hParent), LCID UNREF(a_tLocaleID))
{
	try
	{
		CComPtr<ISharedStateToolMode> pPrev;
		m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
		CComObject<CSharedStateToolMode>* pNew = NULL;
		CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
		// enable outline if it was not enabled prior to selection of outline position (or remove this and keep it only for width?)
		BOOL bOutline = 1;
		BOOL* pOutline = NULL;
		if (pPrev)
		{
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL);
			if (bOutline == 0)
			{
				bOutline = 1;
				pOutline = &bOutline;
			}
		}
		CComPtr<ISharedState> pTmp = pNew;
		if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, &m_eJoins, pPrev))
			m_pStates->StateSet(m_bstrSyncID, pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

