
#pragma once

#include <DocumentUndoImpl.h>


class CUndoStepDeleteBlock : public CDocumentUndoStep
{
public:
	void Init(IImageMetaData* a_pImage, BSTR a_bstrID, ULONG a_nSize, CAutoVectorPtr<BYTE>* a_pData)
	{
		m_pImage = a_pImage;
		m_bstrID = a_bstrID;
		m_nSize = a_nSize;
		m_cData.Attach(a_pData->Detach());
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pImage->SetBlock(m_bstrID, m_nSize, m_cData);
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = m_bstrID.Length()*2+32+m_nSize;
		return S_OK;
	}

private:
	CComPtr<IImageMetaData> m_pImage;
	CComBSTR m_bstrID;
	ULONG m_nSize;
	CAutoVectorPtr<BYTE> m_cData;
};

typedef CUndoStepImpl<CUndoStepDeleteBlock> CUndoDeleteBlock;


class CUndoStepAddBlock : public CDocumentUndoStep
{
public:
	void Init(IImageMetaData* a_pImage, BSTR a_bstrID)
	{
		m_pImage = a_pImage;
		m_bstrID = a_bstrID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pImage->DeleteBlock(m_bstrID);
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = m_bstrID.Length()*2+32;
		return S_OK;
	}

private:
	CComPtr<IImageMetaData> m_pImage;
	CComBSTR m_bstrID;
};

typedef CUndoStepImpl<CUndoStepAddBlock> CUndoAddBlock;


template<typename T>
class ATL_NO_VTABLE CImageMetaDataImpl :
	public IImageMetaData
{
public:
	CImageMetaDataImpl() : m_dwMetaDataChange(0)
	{
	}

	bool MetaDataPresent()
	{
		if (m_pMetaData == NULL)
			return false;
		CComPtr<IEnumStrings> pIDs;
		m_pMetaData->EnumIDs(&pIDs);
		ULONG nIDs = 0;
		if (pIDs) pIDs->Size(&nIDs);
		return nIDs;
	}
	DWORD GetMetaDataChanges()
	{
		DWORD dw = m_dwMetaDataChange;
		m_dwMetaDataChange = 0;
		return dw;
	}
	void InitMetaData()
	{
		RWCoCreateInstance(m_pMetaData, __uuidof(ImageMetaData));
	}
	void CopyMetaData(IImageMetaData* a_pMetaData)
	{
		CComPtr<IEnumStrings> pIDs;
		a_pMetaData->EnumIDs(&pIDs);
		if (pIDs == NULL)
			return;
		CComBSTR bstr;
		for (ULONG i = 0; SUCCEEDED(pIDs->Get(i, &bstr)); ++i, bstr.Empty())
		{
			ULONG nSize = 0;
			a_pMetaData->GetBlockSize(bstr, &nSize);
			if (nSize == 0)
				continue;
			CAutoVectorPtr<BYTE> pData(new BYTE[nSize]);
			if (SUCCEEDED(a_pMetaData->GetBlock(bstr, nSize, pData)))
				m_pMetaData->SetBlock(bstr, nSize, pData);
		}
	}
	IImageMetaData* M_MetaData()
	{
		return m_pMetaData;
	}

	// IImageMetaData methods
public:
	STDMETHOD(EnumIDs)(IEnumStrings** a_ppBlockIDs)
	{
		try
		{
			T::CDocumentReadLock cLock(static_cast<T*>(this));
			if (m_pMetaData == NULL)
				return E_NOTIMPL;
			return m_pMetaData->EnumIDs(a_ppBlockIDs);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE const* a_pData)
	{
		try
		{
			if (a_bstrID == NULL)
				return E_RW_INVALIDPARAM;
			T::CDocumentWriteLock cLock(static_cast<T*>(this));
			if (m_pMetaData == NULL)
				return E_NOTIMPL;
			if (static_cast<T*>(this)->M_Base()->UndoEnabled() == S_OK)
			{
				ULONG nSize = 0;
				if (SUCCEEDED(m_pMetaData->GetBlockSize(a_bstrID, &nSize)))
				{
					CAutoVectorPtr<BYTE> cData(nSize ? new BYTE[nSize] : NULL);
					m_pMetaData->GetBlock(a_bstrID, nSize, cData);
					CUndoDeleteBlock::Add(static_cast<T*>(this)->M_Base(), this, a_bstrID, nSize, &cData);
				}
				else
				{
					CUndoAddBlock::Add(static_cast<T*>(this)->M_Base(), this, a_bstrID);
				}
			}
			m_dwMetaDataChange |= wcscmp(a_bstrID, L"EXIF") == 0 ? EICEXIF : (wcscmp(a_bstrID, L"ICC") == 0 ? EICColorProfile : EICMetadata);
			return m_pMetaData->SetBlock(a_bstrID, a_nSize, a_pData);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetBlockSize)(BSTR a_bstrID, ULONG* a_pSize)
	{
		try
		{
			T::CDocumentReadLock cLock(static_cast<T*>(this));
			if (m_pMetaData == NULL)
				return E_NOTIMPL;
			return m_pMetaData->GetBlockSize(a_bstrID, a_pSize);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetBlock)(BSTR a_bstrID, ULONG a_nSize, BYTE* a_pData)
	{
		try
		{
			T::CDocumentReadLock cLock(static_cast<T*>(this));
			if (m_pMetaData == NULL)
				return E_NOTIMPL;
			return m_pMetaData->GetBlock(a_bstrID, a_nSize, a_pData);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DeleteBlock)(BSTR a_bstrID)
	{
		try
		{
			if (a_bstrID == NULL)
				return E_RW_INVALIDPARAM;
			T::CDocumentWriteLock cLock(static_cast<T*>(this));
			if (m_pMetaData == NULL)
				return E_NOTIMPL;
			if (static_cast<T*>(this)->M_Base()->UndoEnabled() == S_OK)
			{
				ULONG nSize = 0;
				if (SUCCEEDED(m_pMetaData->GetBlockSize(a_bstrID, &nSize)))
				{
					CAutoVectorPtr<BYTE> cData(nSize ? new BYTE[nSize] : NULL);
					m_pMetaData->GetBlock(a_bstrID, nSize, cData);
					CUndoDeleteBlock::Add(static_cast<T*>(this)->M_Base(), this, a_bstrID, nSize, &cData);
				}
			}
			m_dwMetaDataChange |= wcscmp(a_bstrID, L"EXIF") == 0 ? EICEXIF : (wcscmp(a_bstrID, L"ICC") == 0 ? EICColorProfile : EICMetadata);
			return m_pMetaData->DeleteBlock(a_bstrID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IImageMetaData> m_pMetaData;
	DWORD m_dwMetaDataChange;
};

