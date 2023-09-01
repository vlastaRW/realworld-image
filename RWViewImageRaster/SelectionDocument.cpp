// SelectionDocument.cpp : Implementation of CSelectionDocument

#include "stdafx.h"
#include "SelectionDocument.h"

#include <algorithm>


// CSelectionDocument

STDMETHODIMP CSelectionDocument::WriteLock()
{
	try
	{
		InitInternDoc();
		return m_pDoc->WriteLock();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::WriteUnlock()
{
	try
	{
		InitInternDoc();
		return m_pDoc->WriteUnlock();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::ReadLock()
{
	try
	{
		InitInternDoc();
		return m_pDoc->ReadLock();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::ReadUnlock()
{
	try
	{
		InitInternDoc();
		return m_pDoc->ReadUnlock();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::QueryFeatureInterface(REFIID a_iid, void** a_ppFeatureInterface)
{
	try
	{
		InitInternDoc();
		return m_pDoc->QueryFeatureInterface(a_iid, a_ppFeatureInterface);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::LocationGet(IStorageFilter** a_ppLocation)
{
	try
	{
		InitInternDoc();
		return m_pDoc->LocationGet(a_ppLocation);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::LocationSet(IStorageFilter* a_pLocation)
{
	try
	{
		InitInternDoc();
		return m_pDoc->LocationSet(a_pLocation);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::IsDirty()
{
	try
	{
		InitInternDoc();
		return m_pDoc->IsDirty();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::SetDirty()
{
	try
	{
		InitInternDoc();
		return m_pDoc->SetDirty();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::ClearDirty()
{
	try
	{
		InitInternDoc();
		return m_pDoc->ClearDirty();
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::DocumentCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		InitInternDoc();
		return m_pDoc->DocumentCopy(a_bstrPrefix, a_pBase, a_tPreviewEffectID, a_pPreviewEffect);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::ObserverIns(IDocumentObserver* a_pObserver, TCookie a_tCookie)
{
	try
	{
		InitInternDoc();
		return m_pDoc->ObserverIns(a_pObserver, a_tCookie);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

STDMETHODIMP CSelectionDocument::ObserverDel(IDocumentObserver* a_pObserver, TCookie a_tCookie)
{
	try
	{
		InitInternDoc();
		return m_pDoc->ObserverDel(a_pObserver, a_tCookie);
	}
	catch (...)
	{
		return E_POINTER;
	}
}

