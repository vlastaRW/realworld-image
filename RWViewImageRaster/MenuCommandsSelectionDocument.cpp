// MenuCommandsSelectionDocument.cpp : Implementation of CMenuCommandsSelectionDocument

#include "stdafx.h"
#include "MenuCommandsSelectionDocument.h"
#include <SharedStringTable.h>


const OLECHAR CFGID_SUBCOMMANDS[] = L"Commands";
const OLECHAR CFGID_ALLIMAGEONNOSEL[] = L"AllImageOnNoSel";
const OLECHAR CFGID_SELECTIONSYNC[] = L"SelectionSyncGroup";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUISelectionDocumentDlg :
	public CCustomConfigWndImpl<CConfigGUISelectionDocumentDlg>,
	public CDialogResize<CConfigGUISelectionDocumentDlg>
{
public:
	enum { IDD = IDD_CONFIGGUI_SELDOC };

	BEGIN_MSG_MAP(CConfigGUISelectionDocumentDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISelectionDocumentDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUISelectionDocumentDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISelectionDocumentDlg)
		DLGRESIZE_CONTROL(IDC_CG_IMAGEMASKID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPERATION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_ENTIREIMAGE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_OPCONFIG, DLSZ_SIZE_X|DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISelectionDocumentDlg)
		CONFIGITEM_EDITBOX(IDC_CG_IMAGEMASKID, CFGID_SELECTIONSYNC)
		CONFIGITEM_COMBOBOX(IDC_CG_OPERATION, CFGID_SUBCOMMANDS)
		CONFIGITEM_SUBCONFIG(IDC_CG_OPCONFIG, CFGID_SUBCOMMANDS)
		CONFIGITEM_CHECKBOX(IDC_CG_ENTIREIMAGE, CFGID_ALLIMAGEONNOSEL)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};


// CMenuCommandsSelectionDocument

STDMETHODIMP CMenuCommandsSelectionDocument::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_MENUCOMMANDSSELECTIONDOCUMENT_NAME);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSelectionDocument::ConfigCreate(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_2DEDIT_SELECTIONSYNC_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		a_pManager->InsertIntoConfigAs(a_pManager, pCfg, CComBSTR(CFGID_SUBCOMMANDS), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBCOMMANDS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SUBCOMMANDS_DESC), 0, NULL);
		pCfg->ItemInsSimple(CComBSTR(CFGID_ALLIMAGEONNOSEL), _SharedStringTable.GetStringAuto(IDS_CFGID_ALLIMAGEONOSEL_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_ALLIMAGEONOSEL_DESC), CConfigValue(true), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_MenuCommandsSelectionDocument, CConfigGUISelectionDocumentDlg>::FinalizeConfig(pCfg);

		*a_ppDefaultConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

class ATL_NO_VTABLE CViewDeactivatorBlocker :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDesignerView
{
public:
	void Init(IDesignerView* a_pView)
	{
		m_pView = a_pView;
	}

BEGIN_COM_MAP(CViewDeactivatorBlocker)
	COM_INTERFACE_ENTRY(IDesignerView)
END_COM_MAP()

	// IChildWindow methods
public:
	STDMETHOD(Handle)(RWHWND* a_pHandle)
	{ return m_pView->Handle(a_pHandle); }
	STDMETHOD(SendMessage)(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
	{ return m_pView->SendMessage(a_uMsg, a_wParam, a_lParam); }
	STDMETHOD(Show)(BOOL a_bShow)
	{ return m_pView->Show(a_bShow); }
	STDMETHOD(Move)(RECT const* a_prcPosition)
	{ return m_pView->Move(a_prcPosition); }
	STDMETHOD(Destroy)()
	{ return m_pView->Destroy(); }
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg, BOOL a_bBeforeAccel)
	{ return m_pView->PreTranslateMessage(a_pMsg, a_bBeforeAccel); }

	// IDesignerView methods
public:
	STDMETHOD(OptimumSize)(SIZE* a_pSize)
	{ return m_pView->OptimumSize(a_pSize); }
	STDMETHOD(QueryInterfaces)(REFIID a_iid, EQIFilter a_eFilter, IEnumUnknownsInit* a_pInterfaces)
	{ return m_pView->QueryInterfaces(a_iid, a_eFilter, a_pInterfaces); }
	STDMETHOD(OnIdle)()
	{ return m_pView->OnIdle(); }
	STDMETHOD(OnDeactivate)(BOOL a_bCancelChanges)
	{ return S_OK; }
	STDMETHOD(DeactivateAll)(BOOL a_bCancelChanges)
	{ return S_OK; }

private:
	CComPtr<IDesignerView> m_pView;
};

#include "SelectionDocument.h"
#include "SelectionBlockingOperationContext.h"

STDMETHODIMP CMenuCommandsSelectionDocument::CommandsEnum(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComBSTR bstrCFGID_SUBCOMMANDS(CFGID_SUBCOMMANDS);
		CConfigValue cSubItem;
		a_pConfig->ItemValueGet(bstrCFGID_SUBCOMMANDS, &cSubItem);
		CComPtr<IConfig> pSubCfg;
		a_pConfig->SubConfigGet(bstrCFGID_SUBCOMMANDS, &pSubCfg);
		CConfigValue cSyncID;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSyncID);

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));
		a_pView->QueryInterfaces(__uuidof(IRasterImageFloatingSelection), EQIFVisible, pItems);
		CComPtr<IRasterImageFloatingSelection> pFS;
		pItems->Get(0, __uuidof(IRasterImageFloatingSelection), reinterpret_cast<void**>(&pFS));
		CComPtr<IRasterImageEditToolFloatingSelection> pFSTool;
		if (pFS && S_OK == pFS->SelectionExists() && SUCCEEDED(pFS->GetSelectionTool(&pFSTool)) && pFSTool)
		{
			CComObject<CViewDeactivatorBlocker>* pView = NULL;
			CComObject<CViewDeactivatorBlocker>::CreateInstance(&pView);
			CComPtr<IDesignerView> pTmp = pView;
			pView->Init(a_pView);
			CComObject<CSelectionDocument>* pDoc = NULL;
			CComObject<CSelectionDocument>::CreateInstance(&pDoc);
			CComPtr<IDocument> pDocTmp = pDoc;
			pDoc->Init(pFSTool);
			CComObject<CSelectionBlockingOperationContext>* pStates = NULL;
			CComObject<CSelectionBlockingOperationContext>::CreateInstance(&pStates);
			CComPtr<IOperationContext> pStatesTmp = pStates;
			pStates->Init(cSyncID, a_pStates);
			return a_pManager->CommandsEnum(a_pManager, cSubItem, pSubCfg, pStatesTmp, pTmp, pDocTmp, a_ppSubCommands);
		}
		else
		{
			CConfigValue cRun;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_ALLIMAGEONNOSEL), &cRun);
			return cRun.operator bool() ? a_pManager->CommandsEnum(a_pManager, cSubItem, pSubCfg, a_pStates, a_pView, a_pDocument, a_ppSubCommands) : S_OK;
		}
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

