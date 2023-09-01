// RWEXIFWrapper.cpp : Implementation of CRWEXIF

#include "stdafx.h"
#include "RWEXIFWrapper.h"


// CRWEXIF

STDMETHODIMP CRWEXIF::Load(ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		m_pData = a_nSize && a_pData ? exif_data_new_from_data(a_pData, a_nSize) : exif_data_new();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::Save(IReturnedData* a_pData)
{
	try
	{
		if (m_pData == NULL)
			return E_FAIL;
		BYTE* p = NULL;
		unsigned int n = 0;
		exif_data_save_data(m_pData, &p, &n);
		HRESULT hRes = p ? a_pData->Write(n, p) : E_FAIL;
		free(p);
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}


// libexif problems...
struct _ExifDataPrivate
{
	ExifByteOrder order;

	ExifMnoteData *md;

	ExifLog *log;
	ExifMem *mem;

	unsigned int ref_count;

	/* Temporarily used while loading data */
	unsigned int offset_mnote;

	ExifDataOption options;
	ExifDataType data_type;
};
static void *
exif_data_alloc (ExifData *data, unsigned int i)
{
	void *d;

	if (!data || !i) 
		return NULL;

	d = exif_mem_alloc (data->priv->mem, i);
	if (d) 
		return d;

	EXIF_LOG_NO_MEMORY (data->priv->log, "ExifData", i);
	return NULL;
}

STDMETHODIMP CRWEXIF::ThumbnailGet(IReturnedData* a_pData)
{
	try
	{
		if (m_pData == NULL)
			return E_FAIL;
		return a_pData->Write(m_pData->size, m_pData->data);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::ThumbnailSet(ULONG a_nSize, BYTE const* a_pData)
{
	try
	{
		if (m_pData == NULL)
			return E_FAIL;
		void* pTmp = NULL;
		if (a_nSize)
		{
			pTmp = exif_data_alloc(m_pData, a_nSize);
			if (!pTmp)
				return E_FAIL;
			memcpy(pTmp, a_pData, a_nSize);
		}
		if (m_pData->data)
			exif_mem_free(m_pData->priv->mem, m_pData->data);
		m_pData->size = a_nSize;
		m_pData->data = reinterpret_cast<BYTE*>(pTmp);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

ExifEntry* CRWEXIF::FindEntry(WORD a_nIFD, WORD a_nTag)
{
	if (m_pData == NULL)
		return NULL;
	if (a_nIFD >= EXIF_IFD_COUNT)
		return NULL;
	ExifContent* pContent = m_pData->ifd[a_nIFD];
	for (unsigned int j = 0; j < pContent->count; ++j)
	{
		ExifEntry* pEntry = pContent->entries[j];
		if (pEntry->tag == a_nTag)
			return pEntry;
	}
	return NULL;
}

STDMETHODIMP CRWEXIF::TagGetInt(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG* a_n)
{
	try
	{
		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
			return E_RW_ITEMNOTFOUND;
		if (a_nComponent >= pEntry->components)
			return E_RW_INDEXOUTOFRANGE;
		switch (pEntry->format)
		{
		case EXIF_FORMAT_BYTE:
		case EXIF_FORMAT_SBYTE:
		case EXIF_FORMAT_UNDEFINED:
			*a_n = pEntry->data[a_nComponent];
			return S_OK;
		case EXIF_FORMAT_SHORT:
			*a_n = exif_get_short(pEntry->data+2*a_nComponent, m_pData->priv->order);
			return S_OK;
		case EXIF_FORMAT_SSHORT:
			*a_n = exif_get_sshort(pEntry->data+2*a_nComponent, m_pData->priv->order);
			return S_OK;
		case EXIF_FORMAT_LONG:
		case EXIF_FORMAT_SLONG:
			*a_n = exif_get_slong(pEntry->data+2*a_nComponent, m_pData->priv->order);
			return S_OK;
		//case EXIF_FORMAT_RATIONAL:
		//case EXIF_FORMAT_SRATIONAL:
		//case EXIF_FORMAT_FLOAT:
		//case EXIF_FORMAT_DOUBLE:
		default:
			return E_RW_INVALIDPARAM;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagSetInt(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG a_n)
{
	try
	{
		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
		{
			ExifContent* pContent = m_pData->ifd[a_nIFD];
			pEntry = exif_entry_new_mem(m_pData->priv->mem);
			pEntry->tag = static_cast<ExifTag>(a_nTag);
			exif_content_add_entry(pContent, pEntry);
			exif_entry_initialize(pEntry, static_cast<ExifTag>(a_nTag));
		}
		if (a_nComponent >= pEntry->components)
			return E_RW_INDEXOUTOFRANGE;
		switch (pEntry->format)
		{
		case EXIF_FORMAT_BYTE:
		case EXIF_FORMAT_SBYTE:
		case EXIF_FORMAT_UNDEFINED:
			pEntry->data[a_nComponent] = a_n;
			return S_OK;
		case EXIF_FORMAT_SHORT:
			exif_set_short(pEntry->data+2*a_nComponent, m_pData->priv->order, a_n);
			return S_OK;
		case EXIF_FORMAT_SSHORT:
			exif_set_sshort(pEntry->data+2*a_nComponent, m_pData->priv->order, a_n);
			return S_OK;
		case EXIF_FORMAT_LONG:
		case EXIF_FORMAT_SLONG:
			exif_set_slong(pEntry->data+4*a_nComponent, m_pData->priv->order, a_n);
			return S_OK;
		//case EXIF_FORMAT_RATIONAL:
		//case EXIF_FORMAT_SRATIONAL:
		//case EXIF_FORMAT_FLOAT:
		//case EXIF_FORMAT_DOUBLE:
		default:
			return E_RW_INVALIDPARAM;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagGetFlt(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, double* a_f)
{
	try
	{
		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
			return E_RW_ITEMNOTFOUND;
		if (a_nComponent >= pEntry->components)
			return E_RW_INDEXOUTOFRANGE;
		switch (pEntry->format)
		{
		case EXIF_FORMAT_RATIONAL:
			{
				ExifRational tRat = exif_get_rational(pEntry->data+2*a_nComponent, m_pData->priv->order);
				if (tRat.denominator == 0)
					return E_FAIL;
				*a_f = float(tRat.numerator)/tRat.denominator;
			}
			return S_OK;
		case EXIF_FORMAT_SRATIONAL:
			{
				ExifSRational tRat = exif_get_srational(pEntry->data+2*a_nComponent, m_pData->priv->order);
				if (tRat.denominator == 0)
					return E_FAIL;
				*a_f = float(tRat.numerator)/tRat.denominator;
			}
			return S_OK;
		//case EXIF_FORMAT_FLOAT:
		//case EXIF_FORMAT_DOUBLE:
		//	{
		//		ExifRational tRat = exif_get_dorational(pEntry->data+2*a_nComponent, m_pData->priv->order);
		//		a_pNum = tRat.numerator;
		//		a_pDenom = tRat.denominator;
		//	}
		default:
			return E_RW_INVALIDPARAM;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagSetFlt(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, double a_f)
{
	try
	{
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagGetRat(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG* a_pNum, ULONG* a_pDenom)
{
	try
	{
		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
			return E_RW_ITEMNOTFOUND;
		if (a_nComponent >= pEntry->components)
			return E_RW_INDEXOUTOFRANGE;
		switch (pEntry->format)
		{
		case EXIF_FORMAT_RATIONAL:
			{
				ExifRational tRat = exif_get_rational(pEntry->data+2*a_nComponent, m_pData->priv->order);
				*a_pNum = tRat.numerator;
				*a_pDenom = tRat.denominator;
			}
			return S_OK;
		case EXIF_FORMAT_SRATIONAL:
			{
				ExifSRational tRat = exif_get_srational(pEntry->data+2*a_nComponent, m_pData->priv->order);
				*a_pNum = tRat.numerator;
				*a_pDenom = tRat.denominator;
			}
			return S_OK;
		//case EXIF_FORMAT_FLOAT:
		//case EXIF_FORMAT_DOUBLE:
		default:
			return E_RW_INVALIDPARAM;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagSetRat(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG a_nNom, ULONG a_nDenom)
{
	try
	{
		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
		{
			ExifContent* pContent = m_pData->ifd[a_nIFD];
			pEntry = exif_entry_new_mem(m_pData->priv->mem);
			pEntry->tag = static_cast<ExifTag>(a_nTag);
			exif_content_add_entry(pContent, pEntry);
			exif_entry_initialize(pEntry, static_cast<ExifTag>(a_nTag));
		}
		if (a_nComponent >= pEntry->components)
			return E_RW_INDEXOUTOFRANGE;
		switch (pEntry->format)
		{
		case EXIF_FORMAT_RATIONAL:
		case EXIF_FORMAT_SRATIONAL:
			{
				ExifRational val = {a_nNom, a_nDenom};
				exif_set_rational(pEntry->data+8*a_nComponent, m_pData->priv->order, val);
			}
			return S_OK;
		default:
			return E_RW_INVALIDPARAM;
		}
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagGetStr(WORD a_nIFD, WORD a_nTag, BSTR* a_bstrText)
{
	try
	{
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagSetStr(WORD a_nIFD, WORD a_nTag, BSTR a_bstrText)
{
	try
	{
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagDel(WORD a_nIFD, WORD a_nTag)
{
	try
	{
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagGetAsText(WORD a_nIFD, WORD a_nTag, BSTR* a_pbstrText)
{
	try
	{
		*a_pbstrText = NULL;

		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
			return E_RW_ITEMNOTFOUND;
		char szTmp[1024];
		if (NULL == exif_entry_get_value(pEntry, szTmp, sizeof(szTmp)))
			return E_RW_ITEMNOTFOUND;
		CComBSTR bstr(szTmp);
		*a_pbstrText = bstr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrText ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRWEXIF::TagSetAsText(WORD a_nIFD, WORD a_nTag, BSTR a_bstrText)
{
	try
	{
		if (a_bstrText == NULL)
			return E_FAIL; // TODO: remove tag

		ExifEntry* pEntry = FindEntry(a_nIFD, a_nTag);
		if (pEntry == NULL)
			return E_RW_ITEMNOTFOUND;

		CW2A strText(a_bstrText); // TODO: utf8
		ExifIndexedValues const* pIVals = exif_get_indexedvalues_table();
		int iVal = 0;
		while (pIVals[iVal].tag && pIVals[iVal].tag != a_nTag)
			++iVal;
		short index = -1;
		if (pIVals[iVal].tag)
		{
			char const* const* pStrs = pIVals[iVal].strings;
			while (*pStrs)
			{
				if (strcmp(strText, *pStrs) == 0)
				{
					index = pStrs-pIVals[iVal].strings;
					break;
				}
				++pStrs;
			}
		}
		else
		{
			iVal = 0;
			ExifEnumeratedValues const* pEVals = exif_get_enumeratedvalues_table();
			while (pEVals[iVal].tag && pEVals[iVal].tag != pEntry->tag)
				++iVal;
			if (pEVals[iVal].tag)
			{
				ExifEnumeratedValues::TElem const* pElems = pEVals[iVal].elem;
				while (*pElems->values)
				{
					if (strcmp(strText, *pElems->values) == 0)
					{
						index = pElems->index;
						break;
					}
					++pElems;
				}
			}
		}
		if (index > -1)
		{
			exif_set_short(pEntry->data, exif_data_get_byte_order(m_pData), index);
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CRWEXIF::TagGetName(WORD a_nIFD, WORD a_nTag, BSTR* a_pbstrName)
{
	try
	{
		*a_pbstrName = NULL;
		char const* psz = exif_tag_get_name_in_ifd(static_cast<ExifTag>(a_nTag), static_cast<ExifIfd>(a_nIFD));
		if (psz == NULL)
			return E_RW_ITEMNOTFOUND;
		CComBSTR bstr(psz);
		*a_pbstrName = bstr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CRWEXIF::TagFindByName(BSTR a_bstrName, WORD* a_pIFD, WORD* a_pTag)
{
	try
	{
		ExifTag tag = exif_tag_from_name(CW2A(a_bstrName));
		if (tag == 0)
			return E_RW_ITEMNOTFOUND;
		*a_pTag = tag;
		for (int i = 0; i < EXIF_IFD_COUNT; ++i)
		{
			ExifEntry* pEntry = FindEntry(i, tag);
			if (pEntry)
			{
				*a_pIFD = i;
				return S_OK;
			}
		}
		return E_RW_ITEMNOTFOUND;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

