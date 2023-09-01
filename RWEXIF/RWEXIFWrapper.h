// RWEXIFWrapper.h : Declaration of the CRWEXIF

#pragma once
#include "RWEXIF_i.h"
#include "libexif/exif-data.h"
#include "libexif/exif-ifd.h"
#include "libexif/exif-loader.h"



// CRWEXIF

class ATL_NO_VTABLE CRWEXIF :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRWEXIF, &CLSID_RWEXIF>,
	public IRWEXIF
{
public:
	CRWEXIF() : m_pData(NULL)
	{
	}
	~CRWEXIF()
	{
		if (m_pData)
			exif_data_unref(m_pData);
	}

DECLARE_NO_REGISTRY()


BEGIN_COM_MAP(CRWEXIF)
	COM_INTERFACE_ENTRY(IRWEXIF)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IRWEXIF methods
public:
	STDMETHOD(Load)(ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(Save)(IReturnedData* a_pData);
	STDMETHOD(ThumbnailGet)(IReturnedData* a_pData);
	STDMETHOD(ThumbnailSet)(ULONG a_nSize, BYTE const* a_pData);
	STDMETHOD(TagGetInt)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG* a_n);
	STDMETHOD(TagSetInt)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG a_n);
	STDMETHOD(TagGetFlt)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, double* a_f);
	STDMETHOD(TagSetFlt)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, double a_f);
	STDMETHOD(TagGetRat)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG* a_nNom, ULONG* a_nDenom);
	STDMETHOD(TagSetRat)(WORD a_nIFD, WORD a_nTag, ULONG a_nComponent, LONG a_nNom, ULONG a_nDenom);
	STDMETHOD(TagGetStr)(WORD a_nIFD, WORD a_nTag, BSTR* a_bstrText);
	STDMETHOD(TagSetStr)(WORD a_nIFD, WORD a_nTag, BSTR a_bstrText);
	STDMETHOD(TagDel)(WORD a_nIFD, WORD a_nTag);
	STDMETHOD(TagGetAsText)(WORD a_nIFD, WORD a_nTag, BSTR* a_pbstrText);
	STDMETHOD(TagSetAsText)(WORD a_nIFD, WORD a_nTag, BSTR a_bstrText);
	STDMETHOD(TagGetName)(WORD a_nIFD, WORD a_nTag, BSTR* a_pbstrName);
	STDMETHOD(TagFindByName)(BSTR a_bstrName, WORD* a_pIFD, WORD* a_pTag);

private:
	ExifEntry* FindEntry(WORD a_nIFD, WORD a_nTag);
private:
	ExifData* m_pData;
};

OBJECT_ENTRY_AUTO(__uuidof(RWEXIF), CRWEXIF)
