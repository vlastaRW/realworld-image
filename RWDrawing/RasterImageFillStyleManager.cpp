// RasterImageFillStyleManager.cpp : Implementation of CRasterImageFillStyleManager

#include "stdafx.h"
#include "RasterImageFillStyleManager.h"


// CRasterImageFillStyleManager

STDMETHODIMP CRasterImageFillStyleManager::StyleIDsEnum(IEnumStrings** a_ppIDs)
{
	try
	{
		*a_ppIDs = NULL;
		ObjectLock cLock(this);
		CheckState();
		(*a_ppIDs = m_pIDs)->AddRef();
		return S_OK;
	}
	catch (...)
	{
		return a_ppIDs ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::StyleNameGet(BSTR a_bstrID, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->StyleNameGet(this, a_bstrID, a_ppName);
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::StyleDescGet(BSTR a_bstrID, ILocalizedString** a_ppDesc)
{
	try
	{
		*a_ppDesc = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->StyleDescGet(this, a_bstrID, a_ppDesc);
	}
	catch (...)
	{
		return a_ppDesc ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::StyleIconIDGet(BSTR a_bstrID, GUID* a_ptDefaultIcon)
{
	try
	{
		*a_ptDefaultIcon = GUID_NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->StyleIconIDGet(this, a_bstrID, a_ptDefaultIcon);
	}
	catch (...)
	{
		return a_ptDefaultIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::StyleIconGet(BSTR a_bstrID, ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->StyleIconGet(this, a_bstrID, a_nSize, a_phIcon);
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::FillStyleCreate(BSTR a_bstrID, IDocument* a_pDocument, IRasterImageBrush** a_ppFillStyle)
{
	try
	{
		if (a_bstrID == NULL)
			return E_RW_ITEMNOTFOUND;
		*a_ppFillStyle = NULL;
		ObjectLock cLock(this);
		CheckState();
		CFactoryMap::const_iterator i = m_cFactoryMap.find(a_bstrID);
		if (i == m_cFactoryMap.end())
			return E_RW_ITEMNOTFOUND;
		return i->second->FillStyleCreate(this, a_bstrID, a_pDocument, a_ppFillStyle);
	}
	catch (...)
	{
		return a_ppFillStyle ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRasterImageFillStyleManager::WindowCreate(BSTR a_bstrID, RWHWND a_hParent, LCID a_tLocaleID, ISharedStateManager* a_pStates, BSTR a_bstrSyncID, IRasterImageEditToolWindow** a_ppWindow)
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

void CRasterImageFillStyleManager::CheckState()
{
	ULONG const nTimeStamp = CPlugInEnumerator::GetCategoryTimestamp(CATID_RasterImageFillStyleFactory);
	if (m_pIDs && m_nTimeStamp == nTimeStamp)
		return;

	CComPtr<IEnumStringsInit> pToolIDs;
	CFactoryMap cFactoryMap;
	RWCoCreateInstance(pToolIDs, __uuidof(EnumStrings));

	std::map<CATID, CComPtr<IRasterImageFillStyleFactory>, CPlugInEnumerator::lessCATID> plugins;
	CPlugInEnumerator::GetCategoryPlugInMap(CATID_RasterImageFillStyleFactory, plugins);
	for (std::map<CATID, CComPtr<IRasterImageFillStyleFactory>, CPlugInEnumerator::lessCATID>::const_iterator i = plugins.begin(); i != plugins.end(); ++i)
	{
		CComPtr<IEnumStrings> pIDs;
		i->second->StyleIDsEnum(this, &pIDs);
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
	std::swap(pToolIDs.p, m_pIDs.p);
	std::swap(cFactoryMap, m_cFactoryMap);
	m_nTimeStamp = nTimeStamp;
}
