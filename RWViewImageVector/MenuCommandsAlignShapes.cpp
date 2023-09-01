
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <RWBaseEnumUtils.h>
#include "RWDocumentImageVector.h"
#define AUTOCANVASSUPPORT
#include <IconRenderer.h>


template<class T, OLECHAR const* t_pszName, OLECHAR const* t_pszDesc, GUID const* t_pIconID, size_t t_nPolys, IRPolygon const* t_pPolys>
class ATL_NO_VTABLE CAlignShapes : public CDocumentMenuCommandMLImpl<T, t_pszName, t_pszDesc, t_pIconID, 0>
{
public:
	void Init(IDocument* a_pDoc, IDocumentVectorImage* a_pDVI, IOperationContext* a_pSSM, BSTR a_bstrSyncID, BSTR a_bstrFocID)
	{
		m_pDoc = a_pDoc;
		m_pDVI = a_pDVI;
		m_pSSM = a_pSSM;
		m_bstrSyncID = a_bstrSyncID;
		m_bstrFocID = a_bstrFocID;
	}

	// IDocumentMenuCommand
public:
	EMenuCommandState IntState()
	{
		CComPtr<ISharedState> pState;
		m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> cIDs;
		if (pState)
			m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
		if (cIDs.size() > 1)
			return EMCSNormal;
		if (cIDs.empty())
			return EMCSDisabled;

		pState = NULL;
		m_pSSM->StateGet(m_bstrFocID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		std::vector<ULONG> cFocIDs;
		if (pState)
			m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cFocIDs));
		return !cFocIDs.empty() && cIDs[0] != cFocIDs[0] ? EMCSNormal : EMCSDisabled;
	}
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
	{
		try
		{
			*a_phIcon = NULL;
			if (t_nPolys == 0 || t_pPolys == NULL)
				return E_NOTIMPL;

			CComPtr<IStockIcons> pSI;
			RWCoCreateInstance(pSI, __uuidof(StockIcons));
			CComPtr<IIconRenderer> pIR;
			RWCoCreateInstance(pIR, __uuidof(IconRenderer));
			*a_phIcon = IconFromPolygon(pSI, pIR, t_nPolys, t_pPolys, a_nSize, 0.1f, 0.9f);
			return (*a_phIcon) ? S_OK : E_FAIL;
		}
		catch (...)
		{
			return a_phIcon ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<ISharedState> pState;
			m_pSSM->StateGet(m_bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> cIDs;
			m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				m_pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				return E_FAIL; // should not be visible

			CComPtr<ISharedState> pFocused;
			m_pSSM->StateGet(m_bstrFocID, __uuidof(ISharedState), reinterpret_cast<void**>(&pFocused));
			std::vector<ULONG> cFocIDs;
			if (pFocused) m_pDVI->StateUnpack(pFocused, &CEnumToVector<IEnum2UInts, ULONG>(cFocIDs));

			std::vector<std::pair<TPixelCoords, TPixelCoords> > cBounds(cIDs.size());
			std::vector<std::pair<TPixelCoords, TPixelCoords> > cFocused(cFocIDs.size());
			CWriteLock<IDocument> lock(m_pDoc);
			for (size_t i = 0; i < cIDs.size(); ++i)
				m_pDVI->ObjectBounds(cIDs[i], &cBounds[i].first, &cBounds[i].second);
			for (size_t i = 0; i < cFocIDs.size(); ++i)
				m_pDVI->ObjectBounds(cFocIDs[i], &cFocused[i].first, &cFocused[i].second);
			static_cast<T*>(this)->InternalExecute(m_pDVI, cIDs, cBounds, cFocused);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentVectorImage> m_pDVI;
	CComPtr<IOperationContext> m_pSSM;
	CComBSTR m_bstrSyncID;
	CComBSTR m_bstrFocID;
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPELEFT[] = L"[0409]Align lefts[0405]Zarovnat vlevo";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPELEFT[] = L"[0409]Align left sides of the selected objects.[0405]Vyrovnat levé hrany vybraných objektů.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPELEFT = {0xdb6107e3, 0xede6, 0x44e4, {0xa1, 0x7c, 0x37, 0xb6, 0xa6, 0x91, 0x56, 0x39}};
IRPolyPoint const ICONVTXS_ALIGNSHAPESLEFT1[] =
{
	{0.1f, 0.1f}, {0.4f, 0.1f}, {0.4f, 0.25f}, {0.1f, 0.25f},
};
IRPolyPoint const ICONVTXS_ALIGNSHAPESLEFT2[] =
{
	{0.1f, 0.35f}, {0.7f, 0.35f}, {0.7f, 0.5f}, {0.1f, 0.5f},
};
IRPolyPoint const ICONVTXS_ALIGNSHAPESLEFT3[] =
{
	{0.1f, 0.75f}, {0.15f, 0.85f}, {0.25f, 0.9f}, {0.35f, 0.85f}, {0.4f, 0.75f}, {0.35f, 0.65f}, {0.25f, 0.6f}, {0.15f, 0.65f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESLEFT[] = { {itemsof(ICONVTXS_ALIGNSHAPESLEFT1), ICONVTXS_ALIGNSHAPESLEFT1}, {itemsof(ICONVTXS_ALIGNSHAPESLEFT2), ICONVTXS_ALIGNSHAPESLEFT2}, {itemsof(ICONVTXS_ALIGNSHAPESLEFT3), ICONVTXS_ALIGNSHAPESLEFT3} };

#define ICONVTXS(x) sizeof(x)/sizeof(*x), x

class ATL_NO_VTABLE CAlignShapesLeft :
	public CAlignShapes<CAlignShapesLeft, CMDNAME_ALIGNSHAPELEFT, CMDDESC_ALIGNSHAPELEFT, &ICONID_ALIGNSHAPELEFT, ICONVTXS(ICONVTXS_ALIGNSHAPESLEFT)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMin = cBounds[0].first.fX;
		if (cFocused.empty())
		{
			for (size_t i = 1; i < cIDs.size(); ++i)
				if (cBounds[i].first.fX < fMin)
					fMin = cBounds[i].first.fX;
		}
		else
			fMin = cFocused[0].first.fX;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].first.fX != fMin)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, fMin-cBounds[i].first.fX, 0.0f, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPERIGHT[] = L"[0409]Align rights[0405]Zarovnat vpravo";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPERIGHT[] = L"[0409]Align right sides of the selected objects.[0405]Vyrovnat pravé hrany vybraných objektů.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPERIGHT = {0xa62e40f2, 0x3983, 0x48c1, {0x9c, 0xa0, 0xa3, 0x80, 0x40, 0xc5, 0x82, 0x1e}};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESRIGHT1[] =
{
	{0.9f, 0.1f}, {0.6f, 0.1f}, {0.6f, 0.25f}, {0.9f, 0.25f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESRIGHT2[] =
{
	{0.9f, 0.35f}, {0.3f, 0.35f}, {0.3f, 0.5f}, {0.9f, 0.5f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESRIGHT3[] =
{
	{0.9f, 0.75f}, {0.85f, 0.85f}, {0.75f, 0.9f}, {0.65f, 0.85f}, {0.6f, 0.75f}, {0.65f, 0.65f}, {0.75f, 0.6f}, {0.85f, 0.65f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESRIGHT[] = { {itemsof(ICONVTXS_ALIGNSHAPESRIGHT1), ICONVTXS_ALIGNSHAPESRIGHT1}, {itemsof(ICONVTXS_ALIGNSHAPESRIGHT2), ICONVTXS_ALIGNSHAPESRIGHT2}, {itemsof(ICONVTXS_ALIGNSHAPESRIGHT3), ICONVTXS_ALIGNSHAPESRIGHT3} };

class ATL_NO_VTABLE CAlignShapesRight :
	public CAlignShapes<CAlignShapesRight, CMDNAME_ALIGNSHAPERIGHT, CMDDESC_ALIGNSHAPERIGHT, &ICONID_ALIGNSHAPERIGHT, ICONVTXS(ICONVTXS_ALIGNSHAPESRIGHT)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMax = cBounds[0].second.fX;
		if (cFocused.empty())
		{
			for (size_t i = 1; i < cIDs.size(); ++i)
				if (cBounds[i].second.fX > fMax)
					fMax = cBounds[i].second.fX;
		}
		else
			fMax = cFocused[0].second.fX;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].second.fX != fMax)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, fMax-cBounds[i].second.fX, 0.0f, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPETOP[] = L"[0409]Align tops[0405]Zarovnat nahoru";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPETOP[] = L"[0409]Align top sides of the selected objects.[0405]Vyrovnat horní hrany vybraných objektů.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPETOP = {0xc408b446, 0x81f7, 0x4ca0, {0x94, 0x42, 0xe0, 0x12, 0x3b, 0xb9, 0x67, 0xab}};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESTOP1[] =
{
	{0.1f, 0.1f}, {0.1f, 0.4f}, {0.25f, 0.4f}, {0.25f, 0.1f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESTOP2[] =
{
	{0.35f, 0.1f}, {0.35f, 0.7f}, {0.5f, 0.7f}, {0.5f, 0.1f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESTOP3[] =
{
	{0.75f, 0.1f}, {0.85f, 0.15f}, {0.9f, 0.25f}, {0.85f, 0.35f}, {0.75f, 0.4f}, {0.65f, 0.35f}, {0.6f, 0.25f}, {0.65f, 0.15f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESTOP[] = { {itemsof(ICONVTXS_ALIGNSHAPESTOP1), ICONVTXS_ALIGNSHAPESTOP1}, {itemsof(ICONVTXS_ALIGNSHAPESTOP2), ICONVTXS_ALIGNSHAPESTOP2}, {itemsof(ICONVTXS_ALIGNSHAPESTOP3), ICONVTXS_ALIGNSHAPESTOP3} };

class ATL_NO_VTABLE CAlignShapesTop :
	public CAlignShapes<CAlignShapesTop, CMDNAME_ALIGNSHAPETOP, CMDDESC_ALIGNSHAPETOP, &ICONID_ALIGNSHAPETOP, ICONVTXS(ICONVTXS_ALIGNSHAPESTOP)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMin = cBounds[0].first.fY;
		if (cFocused.empty())
		{
			for (size_t i = 1; i < cIDs.size(); ++i)
				if (cBounds[i].first.fY < fMin)
					fMin = cBounds[i].first.fY;
		}
		else
			fMin = cFocused[0].first.fY;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].first.fY != fMin)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, fMin-cBounds[i].first.fY, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPEBOTTOM[] = L"[0409]Align bottoms[0405]Zarovnat dolů";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPEBOTTOM[] = L"[0409]Align bottom sides of the selected objects.[0405]Vyrovnat dolní hrany vybraných objektů.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPEBOTTOM = {0xec89907f, 0x77c, 0x4a99, {0x82, 0xae, 0x39, 0x33, 0xc3, 0x81, 0x30, 0x88}};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESBOTTOM1[] =
{
	{0.1f, 0.9f}, {0.1f, 0.6f}, {0.25f, 0.6f}, {0.25f, 0.9f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESBOTTOM2[] =
{
	{0.35f, 0.9f}, {0.35f, 0.3f}, {0.5f, 0.3f}, {0.5f, 0.9f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESBOTTOM3[] =
{
	{0.75f, 0.9f}, {0.85f, 0.85f}, {0.9f, 0.75f}, {0.85f, 0.65f}, {0.75f, 0.6f}, {0.65f, 0.65f}, {0.6f, 0.75f}, {0.65f, 0.85f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESBOTTOM[] = { {itemsof(ICONVTXS_ALIGNSHAPESBOTTOM1), ICONVTXS_ALIGNSHAPESBOTTOM1}, {itemsof(ICONVTXS_ALIGNSHAPESBOTTOM2), ICONVTXS_ALIGNSHAPESBOTTOM2}, {itemsof(ICONVTXS_ALIGNSHAPESBOTTOM3), ICONVTXS_ALIGNSHAPESBOTTOM3} };

class ATL_NO_VTABLE CAlignShapesBottom :
	public CAlignShapes<CAlignShapesBottom, CMDNAME_ALIGNSHAPEBOTTOM, CMDDESC_ALIGNSHAPEBOTTOM, &ICONID_ALIGNSHAPEBOTTOM, ICONVTXS(ICONVTXS_ALIGNSHAPESBOTTOM)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMax = cBounds[0].second.fY;
		if (cFocused.empty())
		{
			for (size_t i = 1; i < cIDs.size(); ++i)
				if (cBounds[i].second.fY > fMax)
					fMax = cBounds[i].second.fY;
		}
		else
			fMax = cFocused[0].second.fY;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].second.fY != fMax)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, fMax-cBounds[i].second.fY, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPEMIDX[] = L"[0409]Align centers horizontally[0405]Zarovnat na střed horizontálně";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPEMIDX[] = L"[0409]Align centers of the selected objects horizontally.[0405]Vyrovnat středy vybraných objektů horizontálně.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPEMIDX = {0xd04bc763, 0x4be, 0x49db, {0x83, 0x6a, 0x48, 0x1a, 0x24, 0x6, 0xdd, 0xfa}};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDX1[] =
{
	{0.35f, 0.1f}, {0.65f, 0.1f}, {0.65f, 0.25f}, {0.35f, 0.25f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDX2[] =
{
	{0.2f, 0.35f}, {0.8f, 0.35f}, {0.8f, 0.5f}, {0.2f, 0.5f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDX3[] =
{
	{0.35f, 0.75f}, {0.4f, 0.85f}, {0.5f, 0.9f}, {0.6f, 0.85f}, {0.65f, 0.75f}, {0.6f, 0.65f}, {0.5f, 0.6f}, {0.4f, 0.65f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESMIDX[] = { {itemsof(ICONVTXS_ALIGNSHAPESMIDX1), ICONVTXS_ALIGNSHAPESMIDX1}, {itemsof(ICONVTXS_ALIGNSHAPESMIDX2), ICONVTXS_ALIGNSHAPESMIDX2}, {itemsof(ICONVTXS_ALIGNSHAPESMIDX3), ICONVTXS_ALIGNSHAPESMIDX3} };

class ATL_NO_VTABLE CAlignShapesCenterX :
	public CAlignShapes<CAlignShapesCenterX, CMDNAME_ALIGNSHAPEMIDX, CMDDESC_ALIGNSHAPEMIDX, &ICONID_ALIGNSHAPEMIDX, ICONVTXS(ICONVTXS_ALIGNSHAPESMIDX)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMid;
		if (cFocused.empty())
		{
			std::vector<std::pair<TPixelCoords, TPixelCoords> > copy = cBounds;
			std::stable_sort(copy.begin(), copy.end(), cmprt());
			size_t mid = copy.size()>>1;
			fMid = copy[mid].first.fX+copy[mid].second.fX;
		}
		else
			fMid = cFocused[0].first.fX+cFocused[0].second.fX;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].first.fX+cBounds[i].second.fX != fMid)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, (fMid-cBounds[i].first.fX-cBounds[i].second.fX)*0.5f, 0.0f, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}

	struct cmprt
	{
		bool operator()(std::pair<TPixelCoords, TPixelCoords> const& l, std::pair<TPixelCoords, TPixelCoords> const& r) const
		{
			return (l.first.fX+l.second.fX) < (r.first.fX+r.second.fX);
		}
	};
};


extern __declspec(selectany) wchar_t const CMDNAME_ALIGNSHAPEMIDY[] = L"[0409]Align centers vertically[0405]Zarovnat na střed vertikálně";
extern __declspec(selectany) wchar_t const CMDDESC_ALIGNSHAPEMIDY[] = L"[0409]Align centers of the selected objects vertically.[0405]Vyrovnat středy vybraných objektů vertikálně.";
extern __declspec(selectany) GUID const ICONID_ALIGNSHAPEMIDY = {0xc3c7da61, 0xf946, 0x4941, {0xa7, 0x2c, 0xbf, 0xe7, 0xbc, 0x12, 0x18, 0xdc}};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDY1[] =
{
	{0.1f, 0.35f}, {0.1f, 0.65f}, {0.25f, 0.65f}, {0.25f, 0.35f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDY2[] =
{
	{0.35f, 0.2f}, {0.35f, 0.8f}, {0.5f, 0.8f}, {0.5f, 0.2f},
};
extern __declspec(selectany) IRPolyPoint const ICONVTXS_ALIGNSHAPESMIDY3[] =
{
	{0.75f, 0.35f}, {0.85f, 0.4f}, {0.9f, 0.5f}, {0.85f, 0.6f}, {0.75f, 0.65f}, {0.65f, 0.6f}, {0.6f, 0.5f}, {0.65f, 0.4f}, 
};
extern __declspec(selectany) IRPolygon const ICONVTXS_ALIGNSHAPESMIDY[] = { {itemsof(ICONVTXS_ALIGNSHAPESMIDY1), ICONVTXS_ALIGNSHAPESMIDY1}, {itemsof(ICONVTXS_ALIGNSHAPESMIDY2), ICONVTXS_ALIGNSHAPESMIDY2}, {itemsof(ICONVTXS_ALIGNSHAPESMIDY3), ICONVTXS_ALIGNSHAPESMIDY3} };

class ATL_NO_VTABLE CAlignShapesCenterY :
	public CAlignShapes<CAlignShapesCenterY, CMDNAME_ALIGNSHAPEMIDY, CMDDESC_ALIGNSHAPEMIDY, &ICONID_ALIGNSHAPEMIDY, ICONVTXS(ICONVTXS_ALIGNSHAPESMIDY)>
{
public:
	void InternalExecute(IDocumentVectorImage* pDVI, std::vector<ULONG> const& cIDs, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cBounds, std::vector<std::pair<TPixelCoords, TPixelCoords> > const& cFocused)
	{
		float fMid;
		if (cFocused.empty())
		{
			std::vector<std::pair<TPixelCoords, TPixelCoords> > copy = cBounds;
			std::stable_sort(copy.begin(), copy.end(), cmprt());
			size_t mid = copy.size()>>1;
			fMid = copy[mid].first.fY+copy[mid].second.fY;
		}
		else
			fMid = cFocused[0].first.fY+cFocused[0].second.fY;
		for (size_t i = 0; i < cIDs.size(); ++i)
			if (cBounds[i].first.fY+cBounds[i].second.fY != fMid)
			{
				TMatrix3x3f const t = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, (fMid-cBounds[i].first.fY-cBounds[i].second.fY)*0.5f, 1.0f};
				pDVI->ObjectTransform(cIDs[i], &t);
			}
	}

	struct cmprt
	{
		bool operator()(std::pair<TPixelCoords, TPixelCoords> const& l, std::pair<TPixelCoords, TPixelCoords> const& r) const
		{
			return (l.first.fY+l.second.fY) < (r.first.fY+r.second.fY);
		}
	};
};


static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_FOCSYNCGROUP[] = L"FocusSyncGroup";


// CMenuCommandsAlignShapes

extern GUID const CLSID_MenuCommandsAlignShapes = {0xfe4a3ba8, 0xcfad, 0x44c3, {0xab, 0x8f, 0xd, 0x4e, 0x63, 0x3a, 0xa9, 0x6d}};

class ATL_NO_VTABLE CMenuCommandsAlignShapes :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsAlignShapes, &CLSID_MenuCommandsAlignShapes>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsAlignShapes()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsAlignShapes)

BEGIN_CATEGORY_MAP(CMenuCommandsAlignShapes)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsAlignShapes)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		try
		{
			*a_ppOperationName = NULL;
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Vector Image - Align objects[0405]Vektorový obrázek - zarovnat objekty");
			return S_OK;
		}
		catch (...)
		{
			return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), CConfigValue(L"SHAPE"), NULL, 0, NULL);
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_FOCSYNCGROUP), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CMultiLanguageString::GetAuto(L"[0409]Focus ID[0405]ID zaměření"), CConfigValue(L"FOCUSEDSHAPE"), NULL, 0, NULL);

			// finalize the initialization of the config
			pCfgInit->Finalize(NULL);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			if (a_ppSubCommands == NULL) return E_POINTER;

			CComPtr<IDocumentVectorImage> pDVI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI == NULL)
				return E_FAIL;

			CConfigValue cSyncID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
			CConfigValue cFocID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_FOCSYNCGROUP), &cFocID);

			CComBSTR bstrState;
			pDVI->StatePrefix(&bstrState);
			CComBSTR bstrFocus = bstrState;

			bstrState.Append(cSyncID.operator BSTR());
			bstrFocus.Append(cFocID.operator BSTR());

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrState, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> cIDs;
			pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			if (cIDs.empty())
				return S_FALSE;


			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CAlignShapesLeft>* p = NULL;
				CComObject<CAlignShapesLeft>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CAlignShapesCenterX>* p = NULL;
				CComObject<CAlignShapesCenterX>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CAlignShapesRight>* p = NULL;
				CComObject<CAlignShapesRight>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CAlignShapesTop>* p = NULL;
				CComObject<CAlignShapesTop>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CAlignShapesCenterY>* p = NULL;
				CComObject<CAlignShapesCenterY>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CAlignShapesBottom>* p = NULL;
				CComObject<CAlignShapesBottom>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pDocument, pDVI, a_pStates, bstrState, bstrFocus);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsAlignShapes, CMenuCommandsAlignShapes)
