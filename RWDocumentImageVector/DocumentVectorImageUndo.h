
#pragma once

#include <DocumentUndoImpl.h>


class CUndoStepObjectDel : public CDocumentUndoStep
{
public:
	CUndoStepObjectDel() : m_pDoc(NULL) {}
	~CUndoStepObjectDel() { if (m_pDoc) m_pDoc->Release(); }
	void Init(CDocumentVectorImage* a_pDoc, ULONG a_nID, ULONG a_nIndex, BSTR a_bstrName, BSTR a_bstrToolID, BSTR a_bstrToolParams,
		BOOL a_bFill, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, BOOL a_bOutline, TColor a_tColor, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins,
		ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		(m_pDoc = a_pDoc)->AddRef();
		m_nID = a_nID;
		m_nIndex = a_nIndex;
		m_bstrName = a_bstrName;
		m_bstrToolID = a_bstrToolID;
		m_bstrToolParams = a_bstrToolParams;
		m_bFill = a_bFill;
		m_bstrStyleID = a_bstrStyleID;
		m_bstrStyleParams = a_bstrStyleParams;
		m_bOutline = a_bOutline;
		m_tColor = a_tColor;
		m_fWidth = a_fWidth;
		m_fPos = a_fPos;
		m_eJoins = a_eJoins;
		m_eRasterizationMode = a_eRasterizationMode;
		m_eCoordinatesMode = a_eCoordinatesMode;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectRestore(m_nID, m_nIndex, m_bstrName, m_bstrToolID, m_bstrToolParams,
			m_bstrStyleID, m_bstrStyleParams, m_bFill, m_bOutline, m_tColor, m_fWidth, m_fPos, m_eJoins,
			m_eRasterizationMode, m_eCoordinatesMode);
	}

private:
	CDocumentVectorImage* m_pDoc;
	ULONG m_nID;
	ULONG m_nIndex;
	CComBSTR m_bstrName;
	CComBSTR m_bstrToolID;
	CComBSTR m_bstrToolParams;
	BOOL m_bFill;
	CComBSTR m_bstrStyleID;
	CComBSTR m_bstrStyleParams;
	BOOL m_bOutline;
	TColor m_tColor;
	float m_fWidth;
	float m_fPos;
	EOutlineJoinType m_eJoins;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
};

typedef CUndoStepImpl<CUndoStepObjectDel> CUndoObjectDel;


class CUndoStepObjectIns : public CDocumentUndoStep
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectSet(&m_nID, NULL, NULL);
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
};

typedef CUndoStepImpl<CUndoStepObjectIns> CUndoObjectIns;


MIDL_INTERFACE("CF885200-0EAA-46F9-80CF-8CC453B9EDB6")
IUndoStepObjectSet : public IUnknown
{
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID, BSTR a_bstrToolID) = 0;
};

class CUndoStepObjectSet : public CDocumentUndoStep, public IUndoStepObjectSet
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID, CComBSTR& a_bstrToolID, CComBSTR& a_bstrToolParams)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
		m_bstrToolID.Attach(a_bstrToolID.Detach());
		m_bstrToolParams.Attach(a_bstrToolParams.Detach());
	}

BEGIN_COM_MAP(CUndoStepObjectSet)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
	COM_INTERFACE_ENTRY(IUndoStepObjectSet)
END_COM_MAP()

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectSet(&m_nID, m_bstrToolID, m_bstrToolParams);
	}
	STDMETHOD(Merge)(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta)
	{
		if (a_dwTimeDelta > 200)
			return E_FAIL;
		CComQIPtr<IUndoStepObjectSet> pPrev(a_pNextStep);
		return pPrev && pPrev->IsCompatible(m_pDoc, m_nID, m_bstrToolID) ? S_OK : E_FAIL;
	}

	// IUndoStepObjectSet
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID, BSTR a_bstrToolID)
	{
		return a_pDoc == m_pDoc && a_nID == m_nID && m_bstrToolID == a_bstrToolID;
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
	CComBSTR m_bstrToolID;
	CComBSTR m_bstrToolParams;
};

typedef CUndoStepImpl<CUndoStepObjectSet> CUndoObjectSet;


class CUndoStepObjectNameSet : public CDocumentUndoStep
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID, BSTR a_bstrName)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
		m_bstrName = a_bstrName;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectNameSet(m_nID, m_bstrName);
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
	CComBSTR m_bstrName;
};

typedef CUndoStepImpl<CUndoStepObjectNameSet> CUndoObjectNameSet;


class CUndoStepObjectVisibilitySet : public CDocumentUndoStep
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID, boolean a_bVisible)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
		m_bVisible = a_bVisible;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectEnable(m_nID, m_bVisible);
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
	boolean m_bVisible;
};

typedef CUndoStepImpl<CUndoStepObjectVisibilitySet> CUndoObjectVisibilitySet;


MIDL_INTERFACE("1C412D21-FB4A-4DBA-84BD-181FC1FADEF7")
IUndoStepObjectStyleSet : public IUnknown
{
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID, BSTR a_bstrStyleID) = 0;
};

class CUndoStepObjectStyleSet : public CDocumentUndoStep, public IUndoStepObjectStyleSet
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID, CComBSTR& a_bstrStyleID, CComBSTR& a_bstrStyleParams)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
		m_bstrStyleID.Attach(a_bstrStyleID.Detach());
		m_bstrStyleParams.Attach(a_bstrStyleParams.Detach());
	}

BEGIN_COM_MAP(CUndoStepObjectStyleSet)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
	COM_INTERFACE_ENTRY(IUndoStepObjectStyleSet)
END_COM_MAP()

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectStyleSet(m_nID, m_bstrStyleID, m_bstrStyleParams);
	}
	STDMETHOD(Merge)(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta)
	{
		if (a_dwTimeDelta > 200)
			return E_FAIL;
		CComQIPtr<IUndoStepObjectStyleSet> pPrev(a_pNextStep);
		return pPrev && pPrev->IsCompatible(m_pDoc, m_nID, m_bstrStyleID) ? S_OK : E_FAIL;
	}

	// IUndoStepObjectStyleSet
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID, BSTR a_bstrStyleID)
	{
		return a_pDoc == m_pDoc && a_nID == m_nID && m_bstrStyleID == a_bstrStyleID;
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
	CComBSTR m_bstrStyleID;
	CComBSTR m_bstrStyleParams;
};

typedef CUndoStepImpl<CUndoStepObjectStyleSet> CUndoObjectStyleSet;


MIDL_INTERFACE("68E435D6-732C-4A14-ADBB-0D9912C3569C")
IUndoStepObjectStateSet : public IUnknown
{
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID) = 0;
};

class CUndoStepObjectStateSet : public CDocumentUndoStep, public IUndoStepObjectStateSet
{
public:
	void Init(IDocumentVectorImage* a_pDoc, ULONG a_nID, BOOL a_bFill, BOOL a_bOutline, TColor const a_tOutlineColor, float a_fWidth, float a_fPos, EOutlineJoinType a_eJoins, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		m_pDoc = a_pDoc;
		m_nID = a_nID;
		m_bFill = a_bFill;
		m_bOutline = a_bOutline;
		m_tOutlineColor = a_tOutlineColor;
		m_fWidth = a_fWidth;
		m_fPos = a_fPos;
		m_eJoins = a_eJoins;
		m_eRasterizationMode = a_eRasterizationMode;
		m_eCoordinatesMode = a_eCoordinatesMode;
	}

BEGIN_COM_MAP(CUndoStepObjectStateSet)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
	COM_INTERFACE_ENTRY(IUndoStepObjectStateSet)
END_COM_MAP()

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectStateSet(m_nID, &m_bFill, &m_eRasterizationMode, &m_eCoordinatesMode, &m_bOutline, &m_tOutlineColor, &m_fWidth, &m_fPos, &m_eJoins);
	}
	STDMETHOD(Merge)(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta)
	{
		if (a_dwTimeDelta > 200)
			return E_FAIL;
		CComQIPtr<IUndoStepObjectStateSet> pPrev(a_pNextStep);
		return pPrev && pPrev->IsCompatible(m_pDoc, m_nID) ? S_OK : E_FAIL;
	}

	// IUndoStepObjectStateSet
public:
	STDMETHOD_(bool, IsCompatible)(IDocumentVectorImage* a_pDoc, ULONG a_nID)
	{
		return a_pDoc == m_pDoc && a_nID == m_nID;
	}

private:
	CComPtr<IDocumentVectorImage> m_pDoc;
	ULONG m_nID;
	BOOL m_bFill;
	BOOL m_bOutline;
	TColor m_tOutlineColor;
	float m_fWidth;
	float m_fPos;
	EOutlineJoinType m_eJoins;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
};

typedef CUndoStepImpl<CUndoStepObjectStateSet> CUndoObjectStateSet;


class CUndoStepCanvasSize : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TImageSize a_tSize)
	{
		m_pDoc = a_pDoc;
		m_tSize = a_tSize;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->CanvasSet(&m_tSize, NULL, NULL, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TImageSize m_tSize;
};

typedef CUndoStepImpl<CUndoStepCanvasSize> CUndoCanvasSize;


class CUndoStepResolution : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TImageResolution a_tResolution)
	{
		m_pDoc = a_pDoc;
		m_tResolution = a_tResolution;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->CanvasSet(NULL, &m_tResolution, NULL, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TImageResolution m_tResolution;
};

typedef CUndoStepImpl<CUndoStepResolution> CUndoResolution;


class CUndoStepTransform : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TMatrix3x3f a_tTransform)
	{
		m_pDoc = a_pDoc;
		m_tTransform = a_tTransform;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->CanvasSet(NULL, NULL, &m_tTransform, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TMatrix3x3f m_tTransform;
};

typedef CUndoStepImpl<CUndoStepTransform> CUndoTransform;


class CUndoStepChannels : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TPixelChannel a_tDefault)
	{
		m_pDoc = a_pDoc;
		m_tDefault = a_tDefault;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		static EImageChannelID const tChID = EICIRGBA;
		return m_pDoc->ChannelsSet(EICIRGBA, &tChID, &m_tDefault);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TPixelChannel m_tDefault;
};

typedef CUndoStepImpl<CUndoStepChannels> CUndoChannels;


class CUndoStepReorder : public CDocumentUndoStep
{
public:
	CUndoStepReorder() : m_pDoc(NULL) {}
	~CUndoStepReorder() { if (m_pDoc) m_pDoc->Release(); }
	void Init(CDocumentVectorImage* a_pDoc, std::vector<ULONG>& a_cShapeIDs, RECT const& a_rcDirty)
	{
		(m_pDoc = a_pDoc)->AddRef();
		std::swap(a_cShapeIDs, m_cShapeIDs);
		m_rcDirty = a_rcDirty;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->ObjectsReorder(m_cShapeIDs, m_rcDirty);
	}

private:
	CDocumentVectorImage* m_pDoc;
	std::vector<ULONG> m_cShapeIDs;
	RECT m_rcDirty;
};

typedef CUndoStepImpl<CUndoStepReorder> CUndoReorder;

