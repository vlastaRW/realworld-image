// DocumentOperationInsertFrame.cpp : Implementation of CDocumentOperationInsertFrame

#include "stdafx.h"
#include "DocumentOperationInsertFrame.h"
#include <SharedStringTable.h>
#include <SharedStateUndo.h>
#include <RWDocumentAnimation.h>


static OLECHAR const CFGID_INSERT_POS[] = L"InsertPos";
static LONG const CFGVAL_INSPOS_FIRST = 0;
static LONG const CFGVAL_INSPOS_LAST = 1;
static LONG const CFGVAL_INSPOS_BEFORE = 2;
static LONG const CFGVAL_INSPOS_AFTER = 3;
static OLECHAR const CFGID_INSERT_COPY[] = L"CopyFrame";
static OLECHAR const CFGID_SELSYNCGROUP[] = L"SelectionSyncGroup";
static OLECHAR const CFGID_IMGSYNCGROUP[] = L"ImageSyncGroup";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIInsertFrameDlg :
	public CCustomConfigWndImpl<CConfigGUIInsertFrameDlg>,
	public CDialogResize<CConfigGUIInsertFrameDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_INSERTFRAME };

	BEGIN_MSG_MAP(CConfigGUIInsertFrameDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIInsertFrameDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIInsertFrameDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIInsertFrameDlg)
		DLGRESIZE_CONTROL(IDC_CGIF_INSPOS_GRP, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGIF_SYNCID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGIF_IMGSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIInsertFrameDlg)
		CONFIGITEM_RADIO(IDC_CGIF_INSFIRST, CFGID_INSERT_POS, CFGVAL_INSPOS_FIRST)
		CONFIGITEM_RADIO(IDC_CGIF_INSLAST, CFGID_INSERT_POS, CFGVAL_INSPOS_LAST)
		CONFIGITEM_RADIO(IDC_CGIF_INSBEFORE, CFGID_INSERT_POS, CFGVAL_INSPOS_BEFORE)
		CONFIGITEM_RADIO(IDC_CGIF_INSAFTER, CFGID_INSERT_POS, CFGVAL_INSPOS_AFTER)
		CONFIGITEM_CHECKBOX(IDC_CGIF_COPY, CFGID_INSERT_COPY)
		CONFIGITEM_EDITBOX(IDC_CGIF_SYNCID, CFGID_SELSYNCGROUP)
		CONFIGITEM_EDITBOX(IDC_CGIF_IMGSYNCID, CFGID_IMGSYNCGROUP)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CComPtr<IDocument> pDoc;
		GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
		if (pDoc)
		{
			CEdit wnd1 = GetDlgItem(IDC_CGIF_SYNCID);
			wnd1.SetReadOnly(TRUE);
			wnd1.ModifyStyle(WS_TABSTOP, 0);
			CEdit wnd2 = GetDlgItem(IDC_CGIF_IMGSYNCID);
			wnd2.SetReadOnly(TRUE);
			wnd2.ModifyStyle(WS_TABSTOP, 0);
		}
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

// CDocumentOperationInsertFrame

STDMETHODIMP CDocumentOperationInsertFrame::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_OPINSERTFRAME_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationInsertFrame::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_INSERT_POS(CFGID_INSERT_POS);
		pCfgInit->ItemIns1ofN(cCFGID_INSERT_POS, _SharedStringTable.GetStringAuto(IDS_CFGID_INSERT_POS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_INSERT_POS_HELP), CConfigValue(CFGVAL_INSPOS_AFTER), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_INSERT_POS, CConfigValue(CFGVAL_INSPOS_FIRST), _SharedStringTable.GetStringAuto(IDS_CFGVAL_INSPOS_FIRST), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_INSERT_POS, CConfigValue(CFGVAL_INSPOS_LAST), _SharedStringTable.GetStringAuto(IDS_CFGVAL_INSPOS_LAST), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_INSERT_POS, CConfigValue(CFGVAL_INSPOS_BEFORE), _SharedStringTable.GetStringAuto(IDS_CFGVAL_INSPOS_BEFORE), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_INSERT_POS, CConfigValue(CFGVAL_INSPOS_AFTER), _SharedStringTable.GetStringAuto(IDS_CFGVAL_INSPOS_AFTER), 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_INSERT_COPY), _SharedStringTable.GetStringAuto(IDS_CFGID_INSERT_COPY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_INSERT_COPY_HELP), CConfigValue(true), NULL, 0, NULL);

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNCGROUP), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_HELP), CConfigValue(L"SELECTION"), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_IMGSYNCGROUP), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SELSYNCGROUP_HELP), CConfigValue(L""), NULL, 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DocumentOperationInsertFrame, CConfigGUIInsertFrameDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationInsertFrame::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
{
	try
	{
		if (a_pDocument == NULL)
			return S_FALSE;
		CComPtr<IDocumentAnimation> pAC;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAC));
		if (pAC == NULL)
			return S_FALSE;
		if (a_pConfig == NULL)
			return S_OK;
		CConfigValue cInsPos;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_POS), &cInsPos);
		CConfigValue cCopy;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_COPY), &cCopy);
		if ((cInsPos.operator LONG() == CFGVAL_INSPOS_FIRST || cInsPos.operator LONG() == CFGVAL_INSPOS_LAST) && !cCopy.operator bool())
			return S_OK;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CComPtr<ISharedState> pState;
		a_pStates->StateGet(cSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		CComPtr<IEnumUnknowns> pFrames;
		pAC->StateUnpack(pState, &pFrames);
		if (pFrames == NULL)
			return S_FALSE;
		ULONG nFrames = 0;
		pFrames->Size(&nFrames);
		return nFrames ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <RWDocumentAnimationUtils.h>
#if __has_include (<RWDocumentCursor.h>)
#include <RWDocumentCursor.h>
#define WITH_CURSOR
#endif

STDMETHODIMP CDocumentOperationInsertFrame::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentAnimation> pAC;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAC));
		if (pAC == NULL)
			return E_FAIL;
		CConfigValue cInsPos;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_POS), &cInsPos);
		CConfigValue cCopy;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_COPY), &cCopy);
		CComPtr<IUnknown> pBefore;
		CComPtr<IUnknown> pSource;
		CComPtr<IEnumUnknowns> pSelection;
		ULONG nSelection = 0;
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNCGROUP), &cSyncID);
		CConfigValue cImgSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_IMGSYNCGROUP), &cImgSyncID);
		CComPtr<ISharedState> pState;
		if (cSyncID.operator BSTR() && cSyncID.operator BSTR()[0])
			a_pStates->StateGet(cSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
		if (cCopy.operator bool() || cInsPos.operator LONG() == CFGVAL_INSPOS_BEFORE || cInsPos.operator LONG() == CFGVAL_INSPOS_AFTER)
		{
			pAC->StateUnpack(pState, &pSelection);
			if (pSelection == NULL)
				return E_FAIL;
			pSelection->Size(&nSelection);
			if (nSelection == 0)
				return E_FAIL;
		}
		if (cCopy.operator bool())
		{
			pSelection->Get(0, __uuidof(IUnknown), reinterpret_cast<void**>(&pSource));
		}
		CWriteLock<IBlockOperations> cLock(a_pDocument);
		CComPtr<IEnumUnknowns> pFrames;
		pAC->FramesEnum(&pFrames);
		switch (cInsPos.operator LONG())
		{
		case CFGVAL_INSPOS_FIRST:
			pFrames->Get(0, __uuidof(IUnknown), reinterpret_cast<void**>(&pBefore));
			break;
		case CFGVAL_INSPOS_LAST:
			break;
		case CFGVAL_INSPOS_BEFORE:
			pSelection->Get(0, __uuidof(IUnknown), reinterpret_cast<void**>(&pBefore));
			break;
		case CFGVAL_INSPOS_AFTER:
			{
				CComPtr<IUnknown> pAfter;
				pSelection->Get(0, __uuidof(IUnknown), reinterpret_cast<void**>(&pAfter));
				CComPtr<IEnumUnknowns> pFrames;
				pAC->FramesEnum(&pFrames);
				CComPtr<IUnknown> pTmp;
				for (ULONG i = 0; SUCCEEDED(pFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pTmp))); ++i, pTmp = NULL)
				{
					if (pTmp == pAfter)
					{
						pFrames->Get(i+1, __uuidof(IUnknown), reinterpret_cast<void**>(&pBefore));
					}
				}
			}
			break;
		}
		CComPtr<IUnknown> pNew;
		{
			CComPtr<IDocument> pSubDoc;
			if (pSource) pAC->FrameGetDoc(pSource, &pSubDoc);
			pAC->FrameIns(pBefore, pSubDoc ? &CAnimationFrameCreatorDocument(pSubDoc, true) : NULL, &pNew);
		}
		if (pNew == NULL)
			return E_FAIL;
		if (pState)
			CSharedStateUndo<IOperationContext>::SaveState(a_pDocument, a_pStates, cSyncID, pState);
		if (pSource)
		{
			float fTime = 0.1f;
			pAC->FrameGetTime(pSource, &fTime);
			pAC->FrameSetTime(pNew, fTime);
		}
		if (cSyncID.operator BSTR() && cSyncID.operator BSTR()[0])
		{
			CComPtr<IEnumUnknownsInit> pNewSelection;
			RWCoCreateInstance(pNewSelection, __uuidof(EnumUnknowns));
			pNewSelection->Insert(pNew);
			CComPtr<ISharedState> pNewState;
			pAC->StatePack(pNewSelection, &pNewState);
			if (pNewState)
			{
				a_pStates->StateSet(cSyncID, pNewState);
			}
#ifdef WITH_CURSOR
			if (cImgSyncID.operator BSTR() && cImgSyncID.operator BSTR()[0])
			{
				CComPtr<IUnknown> pFrame;
				if (pFrames) pFrames->Get(0, &pFrame);
				CComPtr<IDocument> pFrameDoc;
				if (pFrame) pAC->FrameGetDoc(pFrame, &pFrameDoc);
				CComPtr<IStructuredRoot> pSR;
				if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IStructuredRoot), reinterpret_cast<void**>(&pSR));
				CComPtr<ICursorDirectory> pID;
				if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(ICursorDirectory), reinterpret_cast<void**>(&pID));
				CComBSTR bstrPrefix;
				if (pSR && pID)
				{
					pSR->StatePrefix(&bstrPrefix);
					bstrPrefix += cImgSyncID.operator BSTR();
					CComPtr<ISharedState> pState;
					a_pStates->StateGet(bstrPrefix, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
					CComPtr<IEnumStructs> pFormats;
					if (pState) pID->StateUnpack(pState, &pFormats);
					ULONG nFormats = 0;
					if (pFormats) pFormats->Size(sizeof(TCursorFormat), &nFormats);
					if (nFormats)
					{
						CAutoVectorPtr<TCursorFormat> aFormats(new TCursorFormat[nFormats]);
						pFormats->GetMultiple(0, nFormats, sizeof(TCursorFormat), reinterpret_cast<BYTE*>(aFormats.m_p));
						pFrameDoc = NULL;
						pAC->FrameGetDoc(pNew, &pFrameDoc);
						pSR = NULL;
						if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IStructuredRoot), reinterpret_cast<void**>(&pSR));
						pID = NULL;
						if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(ICursorDirectory), reinterpret_cast<void**>(&pID));
						bstrPrefix.Empty();
						if (pSR && pID)
						{
							pSR->StatePrefix(&bstrPrefix);
							bstrPrefix += cImgSyncID.operator BSTR();
							pState = NULL;
							pID->StatePack(nFormats, aFormats, &pState);
							a_pStates->StateSet(bstrPrefix, pState);
						}
					}
				}
			}
#endif
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationInsertFrame::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Insert frame[0405]Vložit snímek");
			return S_OK;
		}

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_POS), &cVal);
		CComPtr<ILocalizedString> pWhere;
		switch (cVal.operator LONG())
		{
		case CFGVAL_INSPOS_FIRST: pWhere.Attach(_SharedStringTable.GetString(IDS_CFGVAL_INSPOS_FIRST)); break;
		case CFGVAL_INSPOS_LAST: pWhere.Attach(_SharedStringTable.GetString(IDS_CFGVAL_INSPOS_LAST)); break;
		case CFGVAL_INSPOS_BEFORE: pWhere.Attach(_SharedStringTable.GetString(IDS_CFGVAL_INSPOS_BEFORE)); break;
		case CFGVAL_INSPOS_AFTER: pWhere.Attach(_SharedStringTable.GetString(IDS_CFGVAL_INSPOS_AFTER)); break;
		}
		a_pConfig->ItemValueGet(CComBSTR(CFGID_INSERT_COPY), &cVal);
		if (cVal.operator bool())
		{
			CComPtr<ILocalizedString> pCopy;
			pCopy.Attach(_SharedStringTable.GetString(IDS_CFGID_INSERT_COPY_NAME));

			CComObject<CPrintfLocalizedString>* pStr = NULL;
			CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
			CComPtr<ILocalizedString> pTmp = pStr;
			CComPtr<ILocalizedString> pTempl;
			pTempl.Attach(new CSimpleLocalizedString(SysAllocString(L"%s - %s")));
			pStr->Init(pTempl, pWhere, pCopy);
			*a_ppName = pTmp.Detach();
		}
		else
		{
			*a_ppName = pWhere.Detach();
		}
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

