// SharedStateToolData.cpp : Implementation of CSharedStateToolData

#include "stdafx.h"
#include "SharedStateToolData.h"


struct SToolDataInfo
{
	IID iid;
	void* (*pfnCreate)(LPWSTR a_psz);
	void* (*pfnCopy)(void const* a_p);
	void (*pfnDelete)(void* a_p);
	HRESULT (*pfnToString)(void* a_p, BSTR* a_pBSTR);
};

template<class T>
inline void* CreateToolData(LPWSTR a_psz)
{
	T* p = new T;
	if (a_psz)
		p->FromString(CComBSTR(a_psz));
	return p;
}

template<class T>
inline void* CopyToolData(void const* a_p)
{
	T* p = new T;
	*p = *reinterpret_cast<T const*>(a_p);
	return p;
}

template<class T>
inline void DeleteToolData(void* a_p)
{
	delete reinterpret_cast<T*>(a_p);
}

template<class T>
inline HRESULT ToolDataToString(void* a_p, BSTR* a_pBSTR)
{
	return reinterpret_cast<T*>(a_p)->ToString(a_pBSTR);
}


#include "EditToolMagicWand.h"
#include "EditToolPencil.h"
#include "EditToolLine.h"
#include "EditToolCurve.h"
#include "EditToolBrush.h"
#include "EditToolStylize.h"
#include "EditToolEllipse.h"
#include "EditToolRectangle.h"
#include "EditToolPolygon.h"
#include "EditToolFloodFill.h"
#include "EditToolShadow.h"
#include "EditToolText.h"
#include "EditToolSimpleSelect.h"
#include "EditToolTransform.h"
#include "EditToolRetouch.h"
#include "EditToolRecolor.h"
#include "EditToolMove.h"
#include "EditToolShape.h"
#include "EditToolStroke.h"
#include "FillStyleGradientDlg.h"
#include "FillStylePattern.h"
#include "FillStyleSolid.h"
//#include "FillStylePolygon.h"
#include <StringParsing.h>

#define TOOLDATA_RECORD(T) {__uuidof(T::ISharedStateToolData), &CreateToolData<T>, &CopyToolData<T>, &DeleteToolData<T>, &ToolDataToString<T>}

static SToolDataInfo const g_aToolDataInfo[] =
{
	TOOLDATA_RECORD(CEditToolDataMagicWand),
	TOOLDATA_RECORD(CEditToolDataPencil),
	TOOLDATA_RECORD(CEditToolDataLine),
	TOOLDATA_RECORD(CEditToolDataCurve),
	TOOLDATA_RECORD(CEditToolDataBrush),
	TOOLDATA_RECORD(CEditToolDataStylize),
	TOOLDATA_RECORD(CEditToolDataEllipse),
	TOOLDATA_RECORD(CEditToolDataRectangle),
	TOOLDATA_RECORD(CEditToolDataPolygon),
	TOOLDATA_RECORD(CEditToolDataFloodFill),
	TOOLDATA_RECORD(CEditToolDataText),
	TOOLDATA_RECORD(CEditToolDataShadow),
	TOOLDATA_RECORD(CEditToolDataSimpleSelect),
	TOOLDATA_RECORD(CEditToolDataTransformation),
	TOOLDATA_RECORD(CEditToolDataRetouch),
	TOOLDATA_RECORD(CEditToolDataRecolor),
	TOOLDATA_RECORD(CEditToolDataMove),
	TOOLDATA_RECORD(CEditToolDataPath),
	TOOLDATA_RECORD(CEditToolDataStroke),
	TOOLDATA_RECORD(CFillStyleDataPattern),
	TOOLDATA_RECORD(CFillStyleDataGradient),
	TOOLDATA_RECORD(CFillStyleDataSolid),
	TOOLDATA_RECORD(CFillStyleDataDiamond),
};

// CSharedStateToolData

CSharedStateToolData::CSharedStateToolData() :
	m_nType(itemsof(g_aToolDataInfo)), m_pData(NULL)
{
}

CSharedStateToolData::~CSharedStateToolData()
{
	if (m_nType < itemsof(g_aToolDataInfo))
		g_aToolDataInfo[m_nType].pfnDelete(m_pData);
}

bool CSharedStateToolData::Init(void const* a_pData, REFIID a_iid)
{
	size_t i = 0;
	while (i < itemsof(g_aToolDataInfo))
	{
		if (IsEqualIID(a_iid, g_aToolDataInfo[i].iid))
		{
			m_nType = i;
			m_pData = g_aToolDataInfo[i].pfnCopy(a_pData);
			return true;
		}
		++i;
	}
	return false;
}

STDMETHODIMP CSharedStateToolData::ToText(BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = NULL;
		if (m_nType >= itemsof(g_aToolDataInfo) || m_pData == NULL)
			return E_FAIL;
		CComBSTR bstr;
		HRESULT hRes = g_aToolDataInfo[m_nType].pfnToString(m_pData, &bstr);
		if (FAILED(hRes))
			return hRes;
		ULONG nLen = bstr.Length();
		if (nLen == 0)
			return E_NOTIMPL;
		*a_pbstrText = SysAllocStringLen(NULL, nLen+38);
		StringFromGUID(g_aToolDataInfo[m_nType].iid, *a_pbstrText);
		(*a_pbstrText)[36] = L':';
		wcscpy((*a_pbstrText)+37, bstr);
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateToolData::FromText(BSTR a_bstrText)
{
	try
	{
		ATLASSERT(m_pData == NULL); // FromText only allowed once
		if (a_bstrText == NULL)
			return E_FAIL;
		ULONG nLen = SysStringLen(a_bstrText);
		if (nLen < 37)
			return E_FAIL;
		GUID iid = GUID_NULL;
		if (!GUIDFromString(a_bstrText, &iid))
			return E_FAIL;
		size_t i = 0;
		while (i < itemsof(g_aToolDataInfo))
		{
			if (IsEqualIID(iid, g_aToolDataInfo[i].iid))
			{
				m_nType = i;
				m_pData = g_aToolDataInfo[i].pfnCreate(a_bstrText+37);
				return S_OK;
			}
			++i;
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

HRESULT WINAPI CSharedStateToolData::QIToolData(void* a_pThis, REFIID a_iid, void** a_ppv, DWORD_PTR a_dw)
{
	CSharedStateToolData* pThis = reinterpret_cast<CSharedStateToolData*>(a_pThis);
	if (pThis->m_nType >= itemsof(g_aToolDataInfo) || !IsEqualIID(a_iid, g_aToolDataInfo[pThis->m_nType].iid))
		return E_NOINTERFACE;
	*a_ppv = reinterpret_cast<void*>(static_cast<ISharedStateToolData*>(pThis));
	pThis->AddRef();
	return S_OK;
}
