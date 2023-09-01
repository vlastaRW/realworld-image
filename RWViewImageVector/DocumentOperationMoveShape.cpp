// DocumentOperationMoveShape.cpp : Implementation of CDocumentOperationMoveShape

#include "stdafx.h"
#include "DocumentOperationMoveShape.h"
#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_FOCSYNCGROUP[] = L"FocusSyncGroup";
static OLECHAR const CFGID_DESTINATION[] = L"Destination";
static LONG const CFGVAL_DEST_TOP = -2;
static LONG const CFGVAL_DEST_BOTTOM = 2;

// CDocumentOperationMoveShape

STDMETHODIMP CDocumentOperationMoveShape::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Move Object[0405]Vektorový obrázek - přesunout objekt");
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationMoveShape::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
		pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_FOCSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CConfigValue(L"FOCUSEDSHAPE"), NULL, 0, NULL);
		CComBSTR cCFGID_DESTINATION(CFGID_DESTINATION);
		pCfgInit->ItemIns1ofN(cCFGID_DESTINATION, CMultiLanguageString::GetAuto(L"[0409]Type[0405]Typ"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(CFGVAL_DEST_TOP), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_DESTINATION, CConfigValue(CFGVAL_DEST_TOP), CMultiLanguageString::GetAuto(L"[0409]Bring to front[0405]Posunout dopředu"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_DESTINATION, CConfigValue(CFGVAL_DEST_BOTTOM), CMultiLanguageString::GetAuto(L"[0409]Send to back[0405]Poslat dozadu"), 0, NULL);

		// finalize the initialization of the config
		pCfgInit->Finalize(NULL);
		//CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationMoveShape::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI == NULL)
			return S_FALSE;
		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cFocID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FOCSYNCGROUP), &cFocID);
		CComBSTR bstrState;
		pDVI->StatePrefix(&bstrState);
		CComBSTR bstrFocus = bstrState;
		bstrFocus.Append(cFocID.operator BSTR());
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrFocus, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState == NULL)
		{
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			bstrState.Append(cSyncID.operator BSTR());
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		}
		std::vector<ULONG> aSelected;
		pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		if (aSelected.size() != 1)
			return S_FALSE;
		std::vector<ULONG> aItems;
		pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aItems));
		if (aItems.size() < 2)
			return S_FALSE;
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DESTINATION), &cType);
		if (cType.operator LONG() == CFGVAL_DEST_TOP)
			return aItems[aItems.size()-1] != aSelected[0] ? S_OK : S_FALSE;
		else
			return aItems[0] != aSelected[0] ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationMoveShape::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentVectorImage> pDVI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
		if (pDVI == NULL)
			return E_FAIL;
		CComBSTR bstrState;
		pDVI->StatePrefix(&bstrState);
		CComBSTR bstrFocus = bstrState;
		CConfigValue cFocID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_FOCSYNCGROUP), &cFocID);
		bstrFocus.Append(cFocID.operator BSTR());
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(bstrFocus, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (pState == NULL)
		{
			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			bstrState.Append(cSyncID.operator BSTR());
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		}
		std::vector<ULONG> aSelected;
		pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		if (aSelected.size() != 1)
			return E_FAIL;
		std::vector<ULONG> aItems;
		pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aItems));
		if (aItems.size() < 2)
			return E_FAIL;
		CConfigValue cType;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_DESTINATION), &cType);
		if (cType.operator LONG() == CFGVAL_DEST_TOP)
			return pDVI->ObjectsMove(1, &(aSelected[0]), 0);
		else
			return pDVI->ObjectsMove(1, &(aSelected[0]), aItems[0]);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

