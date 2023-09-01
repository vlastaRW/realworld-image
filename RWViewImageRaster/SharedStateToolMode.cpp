// SharedStateToolMode.cpp : Implementation of CSharedStateToolMode

#include "stdafx.h"
#include "SharedStateToolMode.h"


// CSharedStateToolMode

STDMETHODIMP CSharedStateToolMode::ToText(BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = NULL;
		size_t n = 512;
		for (CStrings::const_iterator i = m_aRecentTools.begin(); i != m_aRecentTools.end(); ++i)
			n += 1+(*i).Length();
		for (CStrings::const_iterator i = m_aRecentFills.begin(); i != m_aRecentFills.end(); ++i)
			n += 1+(*i).Length();
		CAutoVectorPtr<wchar_t> szTmp(new wchar_t[n]);
		swprintf(szTmp, L"%s|%sFILL:%s|%s|%s|%s|%s|WIDTH:%g|COLOR:%g,%g,%g,%g", m_bstrToolID.m_str, m_bFill ? L"" : L"NO", m_bstrStyleID.m_str,
			m_eBlendingMode == EBMReplace ? L"REPLACE" : (m_eBlendingMode == EBMDrawOver ? L"PAINTOVER" : L"PAINTUNDER"),
			m_eRasterizationMode == ERMSmooth ? L"SMOOTH" : L"BINARY",
			m_eCoordinatesMode == ECMFloatingPoint ? L"SUBPIXEL" : L"INTEGRAL",
			m_bOutline ? L"OUTLINE" : L"",
			m_fOutlineWidth,
			m_tOutlineColor.fR, m_tOutlineColor.fG, m_tOutlineColor.fB, m_tOutlineColor.fA);
		LPWSTR psz = szTmp.m_p+wcslen(szTmp.m_p);
		if (!m_aRecentTools.empty())
		{
			wcscpy(psz, L"|REC_TOOLS:"); psz += 11;
			for (CStrings::iterator i = m_aRecentTools.begin(); i != m_aRecentTools.end(); ++i)
			{
				if (i != m_aRecentTools.begin())
					*(psz++) = L' ';
				wcscpy(psz, *i);
				psz += (*i).Length();
			}
		}
		if (!m_aRecentFills.empty())
		{
			wcscpy(psz, L"|REC_FILLS:"); psz += 11;
			for (CStrings::iterator i = m_aRecentFills.begin(); i != m_aRecentFills.end(); ++i)
			{
				if (i != m_aRecentFills.begin())
					*(psz++) = L' ';
				wcscpy(psz, *i);
				psz += (*i).Length();
			}
		}
		*a_pbstrText = SysAllocString(szTmp);
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CSharedStateToolMode::FromText(BSTR a_bstrText)
{
	try
	{
		if (a_bstrText == NULL || *a_bstrText == L'\0')
			return S_FALSE;
		LPCOLESTR pszRest = a_bstrText;
		while (*pszRest && *pszRest != L'|') ++pszRest;
		m_bstrToolID.Attach(SysAllocStringLen(a_bstrText, pszRest-a_bstrText));
		LPCOLESTR pszFill = wcsstr(pszRest, L"FILL:");
		if (pszFill)
		{
			LPCOLESTR pszFillEnd = pszFill+5;
			while (*pszFillEnd && *pszFillEnd != L'|') ++pszFillEnd;
			m_bstrStyleID.Attach(SysAllocStringLen(pszFill+5, pszFillEnd-pszFill-5));
		}
		m_bOutline = FALSE;
		m_bFill = TRUE;
		if (wcsstr(pszRest, L"NOFILL")) m_bFill = FALSE;
		if (wcsstr(pszRest, L"REPLACE")) m_eBlendingMode = EBMReplace;
		else if (wcsstr(pszRest, L"PAINTOVER")) m_eBlendingMode = EBMDrawOver;
		else if (wcsstr(pszRest, L"PAINTUNDER")) m_eBlendingMode = EBMDrawUnder;
		if (wcsstr(pszRest, L"SMOOTH")) m_eRasterizationMode = ERMSmooth;
		else if (wcsstr(pszRest, L"BINARY")) m_eRasterizationMode = ERMBinary;
		if (wcsstr(pszRest, L"SUBPIXEL")) m_eCoordinatesMode = ECMFloatingPoint;
		else if (wcsstr(pszRest, L"INTEGRAL")) m_eCoordinatesMode = ECMIntegral;
		if (wcsstr(pszRest, L"OUTLINE")) m_bOutline = TRUE;
		LPCOLESTR pszWidth = wcsstr(pszRest, L"WIDTH:");
		if (pszWidth)
		{
			swscanf(pszWidth+6, L"%f", &m_fOutlineWidth);
			pszRest = pszWidth;
		}
		LPCOLESTR pszColor = wcsstr(pszRest, L"COLOR:");
		if (pszColor)
		{
			swscanf(pszColor+6, L"%f,%f,%f,%f", &m_tOutlineColor.fR, &m_tOutlineColor.fG, &m_tOutlineColor.fB, &m_tOutlineColor.fA);
			pszRest = pszColor;
		}
		LPCOLESTR pszRecTools = wcsstr(pszRest, L"REC_TOOLS:");
		if (pszRecTools)
		{
			LPCOLESTR pszEnd = pszRecTools+10;
			while (*pszEnd && *pszEnd != L'|') ++pszEnd;
			LPCOLESTR psz = pszRecTools+10;
			while (psz < pszEnd)
			{
				LPCOLESTR pszSep = wcschr(psz, L' ');
				if (pszSep == NULL) pszSep = pszEnd;
				if (psz < pszSep)
				{
					CComBSTR bstr;
					bstr.Attach(SysAllocStringLen(psz, pszSep-psz));
					m_aRecentTools.push_back(bstr);
					if (m_aRecentTools.size() > 16)
						break;
					psz = pszSep+1;
				}
				else
				{
					++psz;
				}
			}
			pszRest = pszRecTools;
		}
		pszRest = wcsstr(pszRest, L"REC_FILLS:");
		if (pszRest)
		{
			LPCOLESTR pszEnd = pszRest+10;
			while (*pszEnd && *pszEnd != L'|') ++pszEnd;
			LPCOLESTR psz = pszRest+10;
			while (psz < pszEnd)
			{
				LPCOLESTR pszSep = wcschr(psz, L' ');
				if (pszSep == NULL) pszSep = pszEnd;
				if (psz < pszSep)
				{
					CComBSTR bstr;
					bstr.Attach(SysAllocStringLen(psz, pszSep-psz));
					m_aRecentFills.push_back(bstr);
					if (m_aRecentFills.size() > 16)
						break;
					psz = pszSep+1;
				}
				else
				{
					++psz;
				}
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::Get(BSTR* a_pbstrToolID, BOOL* a_pFill, BSTR* a_pbstrStyleID, EBlendingMode* a_pBlendingMode, ERasterizationMode* a_pRasterizationMode, ECoordinatesMode* a_pCoordinatesMode, BOOL* a_pOutline, TColor* a_pOutlineColor, float* a_pOutlineWidth, float* a_pOutlinePos, EOutlineJoinType* a_pOutlineJoins)
{
	try
	{
		if (a_pbstrToolID)
			m_bstrToolID.CopyTo(a_pbstrToolID);
		if (a_pFill)
			*a_pFill = m_bFill;
		if (a_pbstrStyleID)
			m_bstrStyleID.CopyTo(a_pbstrStyleID);
		if (a_pBlendingMode)
			*a_pBlendingMode = m_eBlendingMode;
		if (a_pRasterizationMode)
			*a_pRasterizationMode = m_eRasterizationMode;
		if (a_pCoordinatesMode)
			*a_pCoordinatesMode = m_eCoordinatesMode;
		if (a_pOutline)
			*a_pOutline = m_bOutline;
		if (a_pOutlineColor)
			*a_pOutlineColor = m_tOutlineColor;
		if (a_pOutlineWidth)
			*a_pOutlineWidth = m_fOutlineWidth;
		if (a_pOutlinePos)
			*a_pOutlinePos = m_fOutlinePos;
		if (a_pOutlineJoins)
			*a_pOutlineJoins = m_eOutlineJoins;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::Set(BSTR a_bstrToolID, BOOL const* a_pFill, BSTR a_bstrStyleID, EBlendingMode const* a_pBlendingMode, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, BOOL const* a_pOutline, TColor const* a_pOutlineColor, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, ISharedStateToolMode* a_pOldState)
{
	try
	{
		if (a_pOldState)
		{
			m_bstrToolID.Empty();
			m_bstrStyleID.Empty();
			a_pOldState->Get(&m_bstrToolID, &m_bFill, &m_bstrStyleID, &m_eBlendingMode, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins);
			m_aRecentTools.clear();
			m_aRecentFills.clear();
			CComPtr<IEnumStrings> pToolIDs;
			CComPtr<IEnumStrings> pFillIDs;
			a_pOldState->GetRecentIDs(&pToolIDs, &pFillIDs);
			ULONG n = 0;
			if (pToolIDs) pToolIDs->Size(&n);
			for (ULONG i = 0; i < n; ++i)
			{
				CComBSTR bstr;
				pToolIDs->Get(i, &bstr);
				if (bstr.Length())
					m_aRecentTools.push_back(bstr);
			}
			n = 0;
			if (pFillIDs) pFillIDs->Size(&n);
			for (ULONG i = 0; i < n; ++i)
			{
				CComBSTR bstr;
				pFillIDs->Get(i, &bstr);
				if (bstr.Length())
					m_aRecentFills.push_back(bstr);
			}
		}

		bool bChange = false;
		if (a_bstrToolID && m_bstrToolID != a_bstrToolID)
		{
			RemoveRecentTool(a_bstrToolID);
			m_aRecentTools.insert(m_aRecentTools.begin(), a_bstrToolID);
			if (m_aRecentTools.size() > 16) m_aRecentTools.resize(16);
			m_bstrToolID = a_bstrToolID;
			bChange = true;
		}
		if (a_pFill && m_bFill != *a_pFill)
		{
			m_bFill = *a_pFill;
			bChange = true;
		}
		if (a_bstrStyleID && m_bstrStyleID != a_bstrStyleID)
		{
			RemoveRecentFill(a_bstrStyleID);
			m_aRecentFills.insert(m_aRecentFills.begin(), a_bstrStyleID);
			if (m_aRecentFills.size() > 16) m_aRecentFills.resize(16);
			m_bstrStyleID = a_bstrStyleID;
			bChange = true;
		}
		if (a_pBlendingMode && m_eBlendingMode != *a_pBlendingMode)
		{
			m_eBlendingMode = *a_pBlendingMode;
			bChange = true;
		}
		if (a_pRasterizationMode && m_eRasterizationMode != *a_pRasterizationMode)
		{
			m_eRasterizationMode = *a_pRasterizationMode;
			bChange = true;
		}
		if (a_pCoordinatesMode && m_eCoordinatesMode != *a_pCoordinatesMode)
		{
			m_eCoordinatesMode = *a_pCoordinatesMode;
			bChange = true;
		}
		if (a_pOutline && m_bOutline != *a_pOutline)
		{
			m_bOutline = *a_pOutline;
			bChange = true;
		}
		if (a_pOutlineColor && (m_tOutlineColor.fR != a_pOutlineColor->fR || m_tOutlineColor.fG != a_pOutlineColor->fG ||
			m_tOutlineColor.fB != a_pOutlineColor->fB || m_tOutlineColor.fA != a_pOutlineColor->fA))
		{
			m_tOutlineColor = *a_pOutlineColor;
			bChange = true;
		}
		if (a_pOutlineWidth && m_fOutlineWidth != *a_pOutlineWidth)
		{
			m_fOutlineWidth = *a_pOutlineWidth;
			bChange = true;
		}
		if (a_pOutlinePos && m_fOutlinePos != *a_pOutlinePos)
		{
			m_fOutlinePos = *a_pOutlinePos;
			bChange = true;
		}
		if (a_pOutlineJoins && m_eOutlineJoins != *a_pOutlineJoins)
		{
			m_eOutlineJoins = *a_pOutlineJoins;
			bChange = true;
		}
		return bChange ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::RemoveRecentTool(BSTR a_bstrToolID)
{
	try
	{
		for (CStrings::iterator i = m_aRecentTools.begin(); i != m_aRecentTools.end(); ++i)
		{
			if (*i == a_bstrToolID)
			{
				m_aRecentTools.erase(i);
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::RemoveRecentFill(BSTR a_bstrFillID)
{
	try
	{
		for (CStrings::iterator i = m_aRecentFills.begin(); i != m_aRecentFills.end(); ++i)
		{
			if (*i == a_bstrFillID)
			{
				m_aRecentFills.erase(i);
				return S_OK;
			}
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::PickRecentTool(ULONG a_nTools, BSTR* a_pbstrToolIDs, ULONG* a_pIndex)
{
	try
	{
		for (CStrings::iterator i = m_aRecentTools.begin(); i != m_aRecentTools.end(); ++i)
		{
			for (ULONG j = 0; j < a_nTools; ++j)
			{
				if (*i == a_pbstrToolIDs[j])
				{
					*a_pIndex = j;
					return S_OK;
				}
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::PickRecentFill(ULONG a_nFills, BSTR* a_pbstrFillIDs, ULONG* a_pIndex)
{
	try
	{
		for (CStrings::iterator i = m_aRecentFills.begin(); i != m_aRecentFills.end(); ++i)
		{
			for (ULONG j = 0; j < a_nFills; ++j)
			{
				if (*i == a_pbstrFillIDs[j])
				{
					*a_pIndex = j;
					return S_OK;
				}
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateToolMode::GetRecentIDs(IEnumStrings** a_ppToolIDs, IEnumStrings** a_ppFillIDs)
{
	try
	{
		if (a_ppToolIDs)
			*a_ppToolIDs = NULL;
		if (a_ppFillIDs)
			*a_ppFillIDs = NULL;

		if (a_ppToolIDs)
		{
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			for (CStrings::iterator i = m_aRecentTools.begin(); i != m_aRecentTools.end(); ++i)
				pTmp->Insert(*i);
			*a_ppToolIDs = pTmp.Detach();
		}

		if (a_ppFillIDs)
		{
			CComPtr<IEnumStringsInit> pTmp;
			RWCoCreateInstance(pTmp, __uuidof(EnumStrings));
			for (CStrings::iterator i = m_aRecentFills.begin(); i != m_aRecentFills.end(); ++i)
				pTmp->Insert(*i);
			*a_ppFillIDs = pTmp.Detach();
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
