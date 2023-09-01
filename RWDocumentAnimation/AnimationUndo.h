
#pragma once

#include <DocumentUndoImpl.h>


class CUndoStepAnimation : public CDocumentUndoStep
{
protected:
	CUndoStepAnimation() : m_pDoc(NULL)
	{
	}
	~CUndoStepAnimation()
	{
		if (m_pDoc) m_pDoc->Release();
	}
	void InitD(CDocumentAnimation* a_pDoc)
	{
		(m_pDoc = a_pDoc)->AddRef();
	}
	CDocumentAnimation* M_Doc() const
	{
		return m_pDoc;
	}

private:
	CDocumentAnimation* m_pDoc;
};


//class CUndoStepName : public CUndoStepAnimation
//{
//public:
//	void Init(CDocumentAnimation* a_pDoc, std::string const& a_strOld)
//	{
//		InitD(a_pDoc);
//		m_strOld = a_strOld;
//	}
//
//	// IDocumentUndoStep methods
//public:
//	STDMETHOD(Execute)()
//	{
//		M_Doc()->SetName(m_strOld);
//		return S_OK;
//	}
//
//private:
//	std::string m_strOld;
//};
//
//typedef CUndoStepImpl<CUndoStepName> CUndoName;


//class CUndoStepArtist : public CUndoStepAnimation
//{
//public:
//	void Init(CDocumentAnimation* a_pDoc, std::string const& a_strOld)
//	{
//		InitD(a_pDoc);
//		m_strOld = a_strOld;
//	}
//
//	// IDocumentUndoStep methods
//public:
//	STDMETHOD(Execute)()
//	{
//		M_Doc()->SetArtist(m_strOld);
//		return S_OK;
//	}
//
//private:
//	std::string m_strOld;
//};
//
//typedef CUndoStepImpl<CUndoStepArtist> CUndoArtist;


class CUndoStepSetTime : public CUndoStepAnimation
{
public:
	void Init(CDocumentAnimation* a_pDoc, IUnknown* a_pFrame, float a_fTime)
	{
		InitD(a_pDoc);
		m_pFrame = a_pFrame;
		m_fTime = a_fTime;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return M_Doc()->FrameSetTime(m_pFrame, m_fTime);
	}

private:
	CComPtr<IUnknown> m_pFrame;
	float m_fTime;
};

typedef CUndoStepImpl<CUndoStepSetTime> CUndoSetTime;


class CUndoStepLoopCount : public CUndoStepAnimation
{
public:
	void Init(CDocumentAnimation* a_pDoc, ULONG a_nCount)
	{
		InitD(a_pDoc);
		m_nCount = a_nCount;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return M_Doc()->LoopCountSet(m_nCount);
	}

private:
	ULONG m_nCount;
};

typedef CUndoStepImpl<CUndoStepLoopCount> CUndoLoopCount;


class CUndoStepSetDoc : public CUndoStepAnimation
{
public:
	void Init(CDocumentAnimation* a_pDoc, ULONG a_nID, IDocumentData* a_pFrame)
	{
		InitD(a_pDoc);
		m_nID = a_nID;
		m_pFrame = a_pFrame;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		M_Doc()->FrameSetDocIntern(m_nID, m_pFrame);
		return S_OK;
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = 256; // guess
		return S_OK;
	}

private:
	ULONG m_nID;
	CComPtr<IDocumentData> m_pFrame;
};

typedef CUndoStepImpl<CUndoStepSetDoc> CUndoSetDoc;


class CUndoStepFrameDel : public CUndoStepAnimation
{
public:
	CUndoStepFrameDel() : m_pItem(NULL) {}
	~CUndoStepFrameDel() { if (m_pItem) m_pItem->Release(); }
	void Init(CDocumentAnimation* a_pDoc, ULONG a_nID, BSTR a_bstrID, IDocumentData* a_pFrame, CComObject<CDocumentAnimation::CFrame>* a_pItem, float a_fTime, size_t a_nBefore)
	{
		InitD(a_pDoc);
		m_nID = a_nID;
		m_bstrID = a_bstrID;
		m_pFrame = a_pFrame;
		(m_pItem = a_pItem)->AddRef();
		m_fTime = a_fTime;
		m_nBefore = a_nBefore;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		M_Doc()->FrameInsIntern(m_nID, m_bstrID, m_pFrame, m_pItem, m_fTime, m_nBefore);
		return S_OK;
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = 256; // guess
		return S_OK;
	}

private:
	ULONG m_nID;
	CComBSTR m_bstrID;
	CComPtr<IDocumentData> m_pFrame;
	CComObject<CDocumentAnimation::CFrame>* m_pItem;
	float m_fTime;
	size_t m_nBefore;
};

typedef CUndoStepImpl<CUndoStepFrameDel> CUndoFrameDel;


class CUndoStepFrameIns : public CUndoStepAnimation
{
public:
	void Init(CDocumentAnimation* a_pDoc, ULONG a_nFrameID)
	{
		InitD(a_pDoc);
		m_nFrameID = a_nFrameID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		try
		{
			return M_Doc()->FrameDel(m_nFrameID);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	ULONG m_nFrameID;
};

typedef CUndoStepImpl<CUndoStepFrameIns> CUndoFrameIns;


class CUndoStepFrameMove : public CUndoStepAnimation
{
public:
	void Init(CDocumentAnimation* a_pDoc, size_t a_nFrom, size_t a_nTo)
	{
		InitD(a_pDoc);
		m_nFrom = a_nFrom;
		m_nTo = a_nTo;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		M_Doc()->FrameMoveIntern(m_nTo, m_nFrom);
		return S_OK;
	}

private:
	size_t m_nFrom;
	size_t m_nTo;
};

typedef CUndoStepImpl<CUndoStepFrameMove> CUndoFrameMove;

