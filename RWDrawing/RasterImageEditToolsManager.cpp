// RasterImageEditToolsManager.cpp : Implementation of CRasterImageEditToolsManager

#include "stdafx.h"
#include "RasterImageEditToolsManager.h"

#include <SharedStringTable.h>


// CRasterImageEditToolsManager

STDMETHODIMP CRasterImageEditToolsManager::ToolIDsEnum(IEnumStrings** a_ppToolIDs)
{
	try
	{
		*a_ppToolIDs = NULL;
		ObjectLock cLock(this);
		CheckState();
		(*a_ppToolIDs = m_pToolIDs)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppToolIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::ToolNameGet(BSTR a_bstrID, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->ToolNameGet(this, a_bstrID, a_ppName);
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::ToolDescGet(BSTR a_bstrID, ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->ToolDescGet(this, a_bstrID, a_ppDesc);
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::ToolIconIDGet(BSTR a_bstrID, GUID* a_ptDefaultIcon)
{
	try
	{
		*a_ptDefaultIcon = GUID_NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->ToolIconIDGet(this, a_bstrID, a_ptDefaultIcon);
	}
	catch (...)
	{
		return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::ToolIconGet(BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->ToolIconGet(this, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::SupportedStates(BSTR a_bstrID, DWORD* a_pBlendingModes, DWORD* a_pRasterizationModes, DWORD* a_pCoordinatesModes, IEnum2UInts* a_pPaintSpecs)
{
	try
	{
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->SupportedStates(this, a_bstrID, a_pBlendingModes, a_pRasterizationModes, a_pCoordinatesModes, a_pPaintSpecs);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::EditToolCreate(BSTR a_bstrID, IDocument* a_pDocument, IRasterImageEditTool** a_ppTool)
{
	try
	{
		*a_ppTool = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->EditToolCreate(this, a_bstrID, a_pDocument, a_ppTool);
	}
	catch (...)
	{
		return a_ppTool ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageEditToolsManager::WindowCreate(BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
{
	try
	{
		*a_ppWindow = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->WindowCreate(this, a_bstrID, a_hParent, a_tLocaleID, a_pStates, a_bstrSyncID, a_ppWindow);
	}
	catch (...)
	{
		return a_ppWindow ? E_UNEXPECTED : E_POINTER;
	}
}

#include <PlugInCache.h>

void CRasterImageEditToolsManager::CheckState()
{
	ULONG const nTimeStamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_RasterImageEditToolsFactory);
	if (m_pToolIDs && m_nTimeStamp == nTimeStamp)
		return;

	CComPtr<IEnumStringsInit> pToolIDs;
	CFactoryMap cFactoryMap;
	RWCoCreateInstance(pToolIDs, __uuidof(EnumStrings));

	std::map<CATID, CComPtr<IRasterImageEditToolsFactory>, CPlugInEnumerator::lessCATID> plugins;
	CPlugInEnumerator::GetCategoryPlugInMap(CATID_RasterImageEditToolsFactory, plugins);
	for (std::map<CATID, CComPtr<IRasterImageEditToolsFactory>, CPlugInEnumerator::lessCATID>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
	{
		CComPtr<IEnumStrings> pIDs;
		i->second->ToolIDsEnum(this, &pIDs);
		ULONG nIDs = 0;
		pIDs->Size(&nIDs);
		for (ULONG j = 0; j < nIDs; ++j)
		{
			CComBSTR bstrID;
			pIDs->Get(j, &bstrID);
			cFactoryMap[bstrID] = i->second;
			pToolIDs->Insert(bstrID);
		}
	}
	std::swap(pToolIDs.p, m_pToolIDs.p);
	std::swap(cFactoryMap, m_cFactoryMap);
	m_nTimeStamp = nTimeStamp;
}
