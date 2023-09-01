
#pragma once

#include <DocumentUndoImpl.h>

class CUndoStepLayerName : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, IComparable* a_pItem, LPCOLESTR a_pszName)
	{
		m_pDoc = a_pDoc;
		m_pItem = a_pItem;
		m_bstrName = a_pszName;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->LayerNameSet(m_pItem, m_bstrName);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	CComPtr<IComparable> m_pItem;
	CComBSTR m_bstrName;
};

typedef CUndoStepImpl<CUndoStepLayerName> CUndoLayerName;


class CUndoStepLayerEffect : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, CDocumentLayeredImage::CLayerEffects* a_pEffects)
	{
		m_pDoc = a_pDoc;
		m_nLayerID = a_nLayerID;
		m_cEffects.swap(*a_pEffects); // steals implementation, but that's OK since it is replaced by new ones in caller
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerEffectSet(m_nLayerID, m_cEffects, true);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nLayerID;
	CDocumentLayeredImage::CLayerEffects m_cEffects;
};

typedef CUndoStepImpl<CUndoStepLayerEffect> CUndoLayerEffect;


MIDL_INTERFACE("193AA1B2-5ED8-468F-8E79-CC4B928CDB4C")
IUndoStepLayerEffect : public IUnknown
{
public:
	STDMETHOD_(IDocumentLayeredImage*, Document)() = 0;
	STDMETHOD_(ULONG, EffectID)() = 0;
	STDMETHOD_(bool, ChangeVis)() = 0;
	STDMETHOD_(bool, ChangeOpID)() = 0;
};

class CUndoStepLayerEffectStepSet : public CDocumentUndoStep, public IUndoStepLayerEffect
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, ULONG a_nEffectID, bool const* a_pEnabled, GUID const* a_pOpID, IConfig* a_pOpCfg)
	{
		m_pDoc = a_pDoc;
		m_nLayerID = a_nLayerID;
		m_nEffectID = a_nEffectID;
		if (a_pEnabled)
		{
			m_pEnabled = &m_bEnabled;
			m_bEnabled = *a_pEnabled;
		}
		else
			m_pEnabled = NULL;
		if (a_pOpID)
		{
			m_pOpID = &m_tOpID;
			m_tOpID = *a_pOpID;
		}
		else
			m_pOpID = NULL;
		m_pOpCfg = a_pOpCfg;
	}

BEGIN_COM_MAP(CUndoStepLayerEffectStepSet)
	COM_INTERFACE_ENTRY(IDocumentUndoStep)
	COM_INTERFACE_ENTRY(IUndoStepLayerEffect)
END_COM_MAP()

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerEffectStepSet(m_nLayerID, m_nEffectID, m_pEnabled, m_pOpID, m_pOpCfg);
	}
	STDMETHOD(Merge)(IDocumentUndoStep* a_pNextStep, DWORD a_dwTimeDelta)
	{
		if (a_dwTimeDelta > 600)
			return E_FAIL;
		CComQIPtr<IUndoStepLayerEffect> pProps(a_pNextStep);
		if (pProps && pProps->Document() == m_pDoc && m_nEffectID == pProps->EffectID() && !pProps->ChangeVis() && !pProps->ChangeOpID())
		{
			return S_OK;
		}
		return E_FAIL;
	}

	// IUndoStepLayerEffect methods
public:
	STDMETHOD_(IDocumentLayeredImage*, Document)() { return m_pDoc; }
	STDMETHOD_(ULONG, EffectID)() { return m_nEffectID; }
	STDMETHOD_(bool, ChangeVis)() { return m_pEnabled; }
	STDMETHOD_(bool, ChangeOpID)() { return m_pOpID; }

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nLayerID;
	ULONG m_nEffectID;
	bool const* m_pEnabled;
	bool m_bEnabled;
	GUID const* m_pOpID;
	GUID m_tOpID;
	CComPtr<IConfig> m_pOpCfg;
};

typedef CUndoStepImpl<CUndoStepLayerEffectStepSet> CUndoLayerEffectStepSet;


class CUndoStepLayerEffectStepInsert : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, ULONG a_nEffectID)
	{
		m_pDoc = a_pDoc;
		m_nLayerID = a_nLayerID;
		m_nEffectID = a_nEffectID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerEffectStepDelete(m_nLayerID, m_nEffectID);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nLayerID;
	ULONG m_nEffectID;
};

typedef CUndoStepImpl<CUndoStepLayerEffectStepInsert> CUndoLayerEffectStepInsert;


class CUndoStepLayerEffectStepDelete : public CDocumentUndoStep
{
public:
	~CUndoStepLayerEffectStepDelete() { m_tSL.CleanUp(); }
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, ULONG a_nBeforeEffectID, CDocumentLayeredImage::SLayerEffect* a_pSL)
	{
		m_pDoc = a_pDoc;
		m_nLayerID = a_nLayerID;
		m_nBeforeEffectID = a_nBeforeEffectID;
		m_tSL.swap(*a_pSL);
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerEffectStepInsert(m_nLayerID, m_nBeforeEffectID, &m_tSL, NULL);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nLayerID;
	ULONG m_nBeforeEffectID;
	CDocumentLayeredImage::SLayerEffect m_tSL;
};

typedef CUndoStepImpl<CUndoStepLayerEffectStepDelete> CUndoLayerEffectStepDelete;


class CUndoStepLayerEffectsReorder : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nLayerID, std::vector<ULONG>* a_aOrder)
	{
		m_pDoc = a_pDoc;
		m_nLayerID = a_nLayerID;
		std::swap(*a_aOrder, m_aOrder);
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerEffectsReorder(m_nLayerID, m_aOrder);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nLayerID;
	std::vector<ULONG> m_aOrder;
};

typedef CUndoStepImpl<CUndoStepLayerEffectsReorder> CUndoLayerEffectsReorder;


class CUndoStepLayersReorder : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, std::vector<ULONG>* a_aOrder)
	{
		m_pDoc = a_pDoc;
		std::swap(*a_aOrder, m_aOrder);
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayersReorder(m_aOrder);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	std::vector<ULONG> m_aOrder;
};

typedef CUndoStepImpl<CUndoStepLayersReorder> CUndoLayersReorder;


class CUndoStepLayerProperties : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, IComparable* a_pItem, ELayerBlend a_eBlendingMode, bool a_bVisible)
	{
		m_pDoc = a_pDoc;
		m_pItem = a_pItem;
		m_eBlendingMode = a_eBlendingMode;
		m_bVisible = a_bVisible;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->LayerPropsSet(m_pItem, &m_eBlendingMode, &m_bVisible);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	CComPtr<IComparable> m_pItem;
	ELayerBlend m_eBlendingMode;
	BYTE m_bVisible;
};

typedef CUndoStepImpl<CUndoStepLayerProperties> CUndoLayerProperties;


class CUndoStepLayerInsert : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nUID)
	{
		m_pDoc = a_pDoc;
		m_nUID = a_nUID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		CComObject<CStructuredItemLayer>* p = NULL;
		CComObject<CStructuredItemLayer>::CreateInstance(&p);
		CComPtr<IUIItem> pItem = p;
		p->Init((CDocumentLayeredImage*)(m_pDoc.p), m_nUID);
		return m_pDoc->LayerDelete(pItem);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nUID;
};

typedef CUndoStepImpl<CUndoStepLayerInsert> CUndoLayerInsert;


class CUndoStepLayerMove : public CDocumentUndoStep
{
public:
	void Init(IDocumentLayeredImage* a_pDoc, ULONG a_nItemID, ULONG a_nUnderID)
	{
		m_pDoc = a_pDoc;
		m_nItem = a_nItemID;
		m_nUnder = a_nUnderID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		CComObject<CStructuredItemLayer>* p = NULL;
		CComObject<CStructuredItemLayer>::CreateInstance(&p);
		CComPtr<IUIItem> pItem = p;
		p->Init((CDocumentLayeredImage*)(m_pDoc.p), m_nItem);
		CComPtr<IUIItem> pUnder;
		if (m_nUnder != 0xffffffff)
		{
			CComObject<CStructuredItemLayer>* pU = NULL;
			CComObject<CStructuredItemLayer>::CreateInstance(&pU);
			pUnder = pU;
			pU->Init((CDocumentLayeredImage*)(m_pDoc.p), m_nUnder);
		}
		return m_pDoc->LayerMove(pItem, pUnder, ELIPBelow);
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	ULONG m_nItem;
	ULONG m_nUnder;
};

typedef CUndoStepImpl<CUndoStepLayerMove> CUndoLayerMove;


class CUndoStepLayerDelete : public CDocumentUndoStep
{
public:
	CUndoStepLayerDelete()
	{
		m_sLayer.pEffects = NULL;
	}
	~CUndoStepLayerDelete()
	{
		if (m_sLayer.pEffects)
			delete m_sLayer.pEffects;
	}
	void Init(CDocumentLayeredImage* a_pDoc, CDocumentLayeredImage::SLayer const* a_pLayer, IDocumentData* a_pData, ULONGLONG a_nSize, ULONG a_nUnderID)
	{
		m_pDoc = a_pDoc;
		m_nSize = a_nSize;
		m_sLayer = *a_pLayer;
		m_pData = a_pData;
		m_nUnder = a_nUnderID;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		try
		{
			return static_cast<CDocumentLayeredImage*>(m_pDoc.p)->LayerRestore(m_sLayer, m_pData, m_nUnder);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = m_nSize;
		return S_OK;
	}

private:
	CComPtr<IDocumentLayeredImage> m_pDoc;
	CDocumentLayeredImage::SLayer m_sLayer;
	CComPtr<IDocumentData> m_pData;
	ULONGLONG m_nSize;
	ULONG m_nUnder;
};

typedef CUndoStepImpl<CUndoStepLayerDelete> CUndoLayerDelete;


