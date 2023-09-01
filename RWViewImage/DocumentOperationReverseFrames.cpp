// DocumentOperationReverseFrames.cpp : Implementation of CDocumentOperationReverseFrames

#include "stdafx.h"
#include "DocumentOperationReverseFrames.h"
#include <SharedStringTable.h>
#include <RWDocumentAnimation.h>


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
#include "ConfigGUISyncID.h"


// CDocumentOperationReverseFrames

STDMETHODIMP CDocumentOperationReverseFrames::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_OPREVERSEFRAMES_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationReverseFrames::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_SELSYNCGROUP(CFGID_SELSYNCGROUP);
		pCfgInit->ItemInsSimple(cCFGID_SELSYNCGROUP, _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_HELP), CConfigValue(L"SELECTION"), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&tCGSyncID, CConfigGUISyncIDDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationReverseFrames::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;
		CComPtr<IDocumentAnimation> pAC;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAC));
		if (pAC == NULL)
			return S_FALSE;
		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pFrames;
		pAC->StateUnpack(pState, &pFrames);
		if (pFrames == NULL)
			return S_FALSE;
		ULONG nFrames = 0;
		pFrames->Size(&nFrames);
		return nFrames ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationReverseFrames::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentAnimation> pAC;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAC));
		if (pAC == NULL)
			return E_FAIL;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pFrames;
		pAC->StateUnpack(pState, &pFrames);
		if (pFrames == NULL)
			return S_FALSE;

		std::set<CComPtr<IUnknown> > cFrames;
		CComPtr<IUnknown> pFrame;
		for (ULONG i = 0; SUCCEEDED(pFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame))); ++i, pFrame = NULL)
			cFrames.insert(pFrame);

		CWriteLock<IBlockOperations> cLock(a_pDocument);
		std::vector<CComPtr<IUnknown> > cAllFrames;
		CComPtr<IEnumUnknowns> pAllFrames;
		pAC->FramesEnum(&pAllFrames);
		for (ULONG i = 0; SUCCEEDED(pAllFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame))); ++i, pFrame = NULL)
			cAllFrames.push_back(pFrame);

		if (cAllFrames.size() <= 1)
			return S_FALSE;
		if (cFrames.size() <= 1)
		{
			cFrames.clear();
			for (std::vector<CComPtr<IUnknown> >::const_iterator i = cAllFrames.begin(); i != cAllFrames.end(); ++i)
				cFrames.insert(*i);
		}
		std::vector<CComPtr<IUnknown> >::iterator i1 = cAllFrames.begin();
		std::vector<CComPtr<IUnknown> >::iterator i2 = cAllFrames.end()-1;
		while (i1 < i2)
		{
			if (cFrames.find(*i1) == cFrames.end())
			{
				++i1;
				continue;
			}
			if (cFrames.find(*i2) == cFrames.end())
			{
				--i2;
				continue;
			}
			pAC->FrameMove(*i2, *i1);
			pAC->FrameMove(i1+1 == i2 ? *i1 : *(i1+1), *i2);
			std::swap((*i1).p, (*i2).p);
			++i1;
			--i2;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

