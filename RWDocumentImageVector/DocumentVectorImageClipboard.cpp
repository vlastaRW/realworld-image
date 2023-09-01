
#include "stdafx.h"
#include "DocumentVectorImage.h"
#include <RWBaseEnumUtils.h>
#include <ReturnedData.h>
#include <DocumentName.h>
#include <algorithm>


STDMETHODIMP CDocumentVectorImage::ClipboardName(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		*a_ppName = new CMultiLanguageString(L"[0409]objects[0405]objekty");
		return S_OK;
	}
	catch (...)
	{
		return a_ppName ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentVectorImage::ClipboardIconID(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, GUID* a_pIconID)
{
	try
	{
		GUID const tID = {0x7fa8dc5f, 0x430a, 0x4381, {0xb1, 0xe0, 0xf0, 0x83, 0x2c, 0x6d, 0xe3, 0xb7}};
		*a_pIconID = tID;
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

#include <IconRenderer.h>

STDMETHODIMP CDocumentVectorImage::ClipboardIcon(ERichGUIClipboardAction a_eAction, ISharedState* a_pState, ULONG a_nSize, HICON* a_phIcon, BYTE* a_pOverlay)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	if (a_pOverlay) *a_pOverlay = TRUE;

	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	IRPathPoint const path0[] = {
		{191, 251, -35.781, 8.82858, 42.4643, -10.4776},
		{107, 206, -9.6546, -31.1093, 9, 29},
		{40, 132, -61, -22, 58.0262, 20.9275},
		{52, 8.99998, 56.4358, -16.1245, -56, 16},
		{242, 90, 33.2804, 82.1609, -32, -79},
		//{65, 5, 35.781, -8.82858, -42.4643, 10.4776},
		//{149, 50, 9.65461, 31.1093, -9, -29},
		//{216, 124, 61, 22, -58.0262, -20.9275},
		//{204, 247, -56.4358, 16.1245, 56, -16},
		//{14, 166, -33.2804, -82.1609, 32, 79},
	};
	IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(path0), path0, pSI->GetMaterial(ESMInterior), IRTarget(0.8f, -1, 1));
	*a_phIcon = cRenderer.get();
	return (*a_phIcon) ? S_OK : E_FAIL;
}

STDMETHODIMP CDocumentVectorImage::ClipboardCheck(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, ISharedState* a_pState)
{
	try
	{
		if (a_eAction == ERGCAPaste)
		{
			if (m_nClipboardFormat == 0)
				m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));
			return IsClipboardFormatAvailable(m_nClipboardFormat) ? S_OK : S_FALSE;
		}
		std::vector<ULONG> aSelected;
		StateUnpack(a_pState, CEnumToVector<IEnum2UInts, ULONG>(aSelected));
		CDocumentReadLock cLock(this);
		switch (a_eAction)
		{
		case ERGCADelete:
		case ERGCADuplicate:
		case ERGCACut:
		case ERGCACopy:				return aSelected.empty() ? S_FALSE : S_OK;
		case ERGCASelectAll:		return m_cElements.size() <= aSelected.size() ? S_FALSE : S_OK;
		case ERGCAInvertSelection:	return m_cElements.empty() ? S_FALSE : S_OK;
		}
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

bool CopyObjects(IDocumentVectorImage* a_pDst, IDocumentVectorImage* a_pSrc, std::vector<ULONG>& a_aIDs)
{
	for (std::vector<ULONG>::iterator i = a_aIDs.begin(); i != a_aIDs.end(); ++i)
	{
		CComBSTR bstrName;
		a_pSrc->ObjectNameGet(*i, &bstrName);
		CComBSTR bstrToolID;
		CComBSTR bstrToolParams;
		a_pSrc->ObjectGet(*i, &bstrToolID, &bstrToolParams);
		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;
		a_pSrc->ObjectStyleGet(*i, &bstrStyleID, &bstrStyleParams);
		bool bEnabled = a_pSrc->ObjectIsEnabled(*i) != S_FALSE;
		TColor tColor = {0.0f, 0.0f, 0.0f, 1.0f};
		ERasterizationMode eRM = ERMSmooth;
		ECoordinatesMode eCM = ECMFloatingPoint;
		BOOL bFill = TRUE;
		BOOL bOutline = FALSE;
		float fWidth = 1.0f;
		float fPos = 0.0f;
		EOutlineJoinType eJoins = EOJTRound;
		a_pSrc->ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
		ULONG nNew = 0;
		if (FAILED(a_pDst->ObjectSet(&nNew, bstrToolID, bstrToolParams)) || nNew == 0)
			return false;
		if (bstrName.m_str)
			a_pDst->ObjectNameSet(nNew, bstrName);
		a_pDst->ObjectStyleSet(nNew, bstrStyleID, bstrStyleParams);
		a_pDst->ObjectStateSet(nNew, &bFill, &eRM, &eCM, &bOutline, &tColor, &fWidth, &fPos, &eJoins);
		a_pDst->ObjectEnable(nNew, bEnabled);
		*i = nNew;
	}
	return true;
}

class CClipboardHandler
{
public:
	CClipboardHandler(HWND a_hWnd)
	{
		if (!::OpenClipboard(a_hWnd))
			throw E_FAIL;
	}
	~CClipboardHandler()
	{
		::CloseClipboard();
	}
};

STDMETHODIMP CDocumentVectorImage::ClipboardRun(ERichGUIClipboardAction a_eAction, RWHWND a_hWnd, LCID a_tLocaleID, ISharedState* a_pState, ISharedState** a_ppNewState)
{
	try
	{
		if (a_eAction == ERGCACut || a_eAction == ERGCACopy)
		{
			std::vector<ULONG> aSelected;
			StateUnpack(a_pState, CEnumToVector<IEnum2UInts, ULONG>(aSelected));
			if (m_nClipboardFormat == 0)
				m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));

			if (aSelected.empty())
				return S_FALSE;

			CAutoPtr<CDocumentReadLock> pRead;
			CAutoPtr<CDocumentWriteLock> pWrite;
			if (a_eAction == ERGCACut)
				pWrite.Attach(new CDocumentWriteLock(this));
			else
				pRead.Attach(new CDocumentReadLock(this));

			CClipboardHandler cClipboard(a_hWnd);
			EmptyClipboard();

			CComPtr<IDocumentBase> pBase;
			RWCoCreateInstance(pBase, __uuidof(DocumentBase));
			CComPtr<IDocumentFactoryVectorImage> pDFVI;
			RWCoCreateInstance(pDFVI, __uuidof(DocumentFactoryVectorImage));
			float a0[] = {0.0f, 0.0f, 0.0f, 0.0f};
			pDFVI->Create(NULL, pBase, CImageSize(256, 256), NULL, a0);
			std::set<ULONG> cSelected(aSelected.begin(), aSelected.end());
			for (std::vector<ULONG>::const_iterator i = m_cElementOrder.begin(); i != m_cElementOrder.end(); ++i)
			{
				if (cSelected.find(*i) == cSelected.end())
					continue;
				CElements::const_iterator e = m_cElements.find(*i);
				if (e == m_cElements.end())
					continue;
				boolean enabled = e->second.bEnabled;
				pDFVI->AddObject(NULL, pBase, e->second.bstrName, e->second.bstrToolID, e->second.bstrToolParams, e->second.bFill ? e->second.bstrStyleID.m_str : NULL, e->second.bFill ? e->second.bstrStyleParams.m_str : NULL, e->second.bOutline ? &e->second.fOutlineWidth : NULL, e->second.bOutline ? &e->second.fOutlinePos : NULL, e->second.bOutline ? &e->second.eOutlineJoins : NULL, e->second.bOutline ? &e->second.tOutlineColor : NULL, &e->second.eRasterizationMode, &e->second.eCoordinatesMode, &enabled);
			}
				
			CComQIPtr<IDocument> pDoc2(pBase);
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			CComBSTR bstrRVI(L"{51C87837-B028-4252-A3B3-940F80181770}");
			float const fWeight = 1.0f;
			GUID tID = GUID_NULL;
			CComPtr<IConfig> pCfg;
			pIM->FindBestEncoderEx(pDoc2, 1, &(bstrRVI.m_str), &fWeight, &tID, &pCfg);
			CComPtr<IDocumentEncoder> pEnc;
			if (IsEqualGUID(tID, GUID_NULL) || FAILED(RWCoCreateInstance(pEnc, tID)))
				return E_FAIL;
			CReturnedData cData;
			if (FAILED(pEnc->Serialize(pDoc2, pCfg, &cData, NULL, NULL)) || cData.size() == 0)
				return E_FAIL;

			HANDLE hMem = GlobalAlloc(GHND, cData.size());
			BYTE* pData = reinterpret_cast<BYTE*>(GlobalLock(hMem));
			CopyMemory(pData, cData.begin(), cData.size());
			GlobalUnlock(hMem);
			SetClipboardData(m_nClipboardFormat, hMem);
			if (a_eAction == ERGCACut)
			{
				for (std::vector<ULONG>::const_iterator i = aSelected.begin(); i != aSelected.end(); ++i)
				{
					ULONG n = *i;
					ObjectSet(&n, NULL, NULL);
				}
				//CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync);
				CComPtr<ISharedState> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(SharedStateEnum));
				if (a_ppNewState)
				{
					*a_ppNewState = pTmp;
					pTmp.Detach();
				}
				//CComQIPtr<IEnumUnknownsInit> pNewItems(pTmp);

				//CComPtr<ISharedState> pState;
				//m_pImage->StatePack(0, NULL, &pState);
				//m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
			}
			return S_OK;
		}

		if (a_eAction == EDVCAPaste)
		{
			CDocumentWriteLock cLock(this);

			if (m_nClipboardFormat == 0)
				m_nClipboardFormat = RegisterClipboardFormat(_T("RWObjects"));
			if (!IsClipboardFormatAvailable(m_nClipboardFormat))
				return E_FAIL;

			CClipboardHandler cClipboard(a_hWnd);

			HANDLE hMem = GetClipboardData(m_nClipboardFormat);
			if (hMem == NULL)
				return E_FAIL;

			CComPtr<IDocumentBase> pDocBase;
			RWCoCreateInstance(pDocBase, __uuidof(DocumentBase));
			CComQIPtr<IDocument> pDoc;

			BYTE const* pData = reinterpret_cast<BYTE const*>(GlobalLock(hMem));
			SIZE_T nSize = GlobalSize(hMem);
			try
			{
				CComObject<CDocumentName>* pName = NULL;
				CComObject<CDocumentName>::CreateInstance(&pName);
				CComPtr<IStorageFilter> pName2 = pName;
				pName->Init(L"pasted.rvi");
				CComPtr<IInputManager> pIM;
				RWCoCreateInstance(pIM, __uuidof(InputManager));
				CComPtr<IDocumentBuilder> pBuilder;
				RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryVectorImage));
				if (SUCCEEDED(pIM->DocumentCreateDataEx(pBuilder, nSize, pData, pName2, NULL, pDocBase, NULL, NULL, NULL)))
					pDoc = pDocBase;
			}
			catch (...)
			{
			}
			GlobalUnlock(hMem);
			hMem = NULL;
			CComPtr<IDocumentVectorImage> pDVI;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI));
			if (pDVI)
			{
				std::vector<ULONG> aIDs;
				pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aIDs));
				if (!aIDs.empty())
				{
					if (CopyObjects(this, pDVI, aIDs))
					{
						StatePack(aIDs.size(), aIDs.size() ? &(aIDs[0]) : NULL, a_ppNewState);

						//CSharedStateUndo<ISharedStateManager>::SaveState(m_pDoc, m_pStateMgr, m_bstrSelectionStateSync);
						//CComPtr<ISharedState> pState;
						//m_pImage->StatePack(aIDs.size(), &(aIDs[0]), &pState);
						//m_aSelected.clear();
						//m_eMS = EMSUnknown;// EMSModifying
						//m_pStateMgr->StateSet(m_bstrSelectionStateSync, pState);
					}
				}
			}
			return S_OK;
		}

		if (a_eAction == ERGCASelectAll)
		{
			CDocumentReadLock cLock(this);

			StatePack(m_cElementOrder.size(), m_cElementOrder.size() ? &(m_cElementOrder[0]) : NULL, a_ppNewState);

			return S_OK;
		}

		if (a_eAction == ERGCAInvertSelection)
		{
			CDocumentReadLock cLock(this);

			std::vector<ULONG> aSelected;
			StateUnpack(a_pState, CEnumToVector<IEnum2UInts, ULONG>(aSelected));

			std::vector<ULONG> aIDs;
			std::set<ULONG> cSel;
			std::copy(aSelected.begin(), aSelected.end(), std::inserter(cSel, cSel.end()));
			std::set<ULONG> cAll;
			std::copy(m_cElementOrder.begin(), m_cElementOrder.end(), std::inserter(cAll, cAll.end()));
			std::set_difference(cAll.begin(), cAll.end(), cSel.begin(), cSel.end(), std::back_inserter(aIDs));

			StatePack(aIDs.size(), aIDs.size() ? &(aIDs[0]) : NULL, a_ppNewState);

			return S_OK;
		}

		if (a_eAction == ERGCADelete)
		{
			std::set<ULONG> cFrames;
			StateUnpack(a_pState, &CEnumToSet<IEnum2UInts, ULONG>(cFrames));

			if (cFrames.empty())
				return S_FALSE;

			CDocumentWriteLock cLock(this);
			ULONG nNewSel = 0;
			ULONG nPrevFrame = 0;
			bool bSelectNext = false;
			for (CElementOrder::const_iterator i = m_cElementOrder.begin(); i != m_cElementOrder.end(); ++i)
			{
				if (cFrames.find(*i) != cFrames.end())
				{
					bSelectNext = true;
					if (nPrevFrame)
						std::swap(nPrevFrame, nNewSel);
				}
				else if (bSelectNext)
				{
					nNewSel = *i;
					bSelectNext = false;
				}
				if (nNewSel == 0)
					nPrevFrame = *i;
			}
			if (nNewSel)
			{
				StatePack(1, &nNewSel, a_ppNewState);
			}
			for (std::set<ULONG>::const_iterator i = cFrames.begin(); i != cFrames.end(); ++i)
			{
				ULONG n = *i;
				ObjectSet(&n, NULL, NULL);
			}
			return S_OK;
		}

		if (a_eAction == ERGCADuplicate)
		{
			std::set<ULONG> cFrames;
			StateUnpack(a_pState, &CEnumToSet<IEnum2UInts, ULONG>(cFrames));

			if (cFrames.empty())
				return S_FALSE;

			CDocumentWriteLock cLock(this);

			CAutoVectorPtr<ULONG> aNewSel(new ULONG[cFrames.size()]);
			ULONG* pNewSel = aNewSel;
			CElementOrder cElementOrder = m_cElementOrder;
			for (CElementOrder::const_iterator e = cElementOrder.begin(); e != cElementOrder.end(); ++e)
			{
				std::set<ULONG>::const_iterator i = cFrames.find(*e);
				if (i == cFrames.end())
					continue;

				CComBSTR bstrName;
				CComBSTR bstrToolID;
				CComBSTR bstrToolParams;
				CComBSTR bstrStyleID;
				CComBSTR bstrStyleParams;
				BOOL bFill = TRUE;
				ERasterizationMode eRM = ERMSmooth;
				ECoordinatesMode eCM = ECMFloatingPoint;
				BOOL bOutline = FALSE;
				TColor tOutline = {0.0f, 0.0f, 0.0f, 1.0f};
				float fOutline = 1.0f;
				float fOutlinePos = 0.0f;
				EOutlineJoinType eOutlineJoins = EOJTRound;
				ObjectGet(*i, &bstrToolID, &bstrToolParams);
				ObjectNameGet(*i, &bstrName);
				ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tOutline, &fOutline, &fOutlinePos, &eOutlineJoins);
				ObjectStyleGet(*i, &bstrStyleID, &bstrStyleParams);
				*pNewSel = 0;
				ObjectSet(pNewSel, bstrToolID, bstrToolParams);

				CElementOrder::const_iterator e2 = e;
				++e2;
				if (e2 != cElementOrder.end())
					ObjectsMove(1, pNewSel, *e2);
				if (bstrName.m_str)
				{
					CComBSTR bstrCopy;
					CMultiLanguageString::GetLocalized(L"[0409]Copy of %s[0405]Kopie - %s", a_tLocaleID, &bstrCopy);
					wchar_t szTmp[256] = L"";
					swprintf(szTmp, 256, bstrCopy, bstrName.m_str);
					szTmp[255] = L'\0';
					ObjectNameSet(*pNewSel, CComBSTR(szTmp));
				}
				ObjectStateSet(*pNewSel, &bFill, &eRM, &eCM, &bOutline, &tOutline, &fOutline, &fOutlinePos, &eOutlineJoins);
				ObjectStyleSet(*pNewSel, bstrStyleID, bstrStyleParams);
				++pNewSel;
			}
			StatePack(pNewSel - aNewSel.m_p, aNewSel, a_ppNewState);
			return S_OK;
		}

		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

