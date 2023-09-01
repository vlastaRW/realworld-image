// DocumentOperationRasterImageBackgroundBlend.cpp : Implementation of CDocumentOperationRasterImageBackgroundBlend

#include "stdafx.h"
#include "DocumentOperationRasterImageBackgroundBlend.h"
#include <SharedStringTable.h>


static OLECHAR const CFGID_BACKGROUND[] = L"Background";
static OLECHAR const CFGID_THRESHOLD[] = L"Threshold";

#include <ConfigCustomGUIImpl.h>
#include <RWViewImageRaster.h>
#include <WTL_ColorPicker.h>

class ATL_NO_VTABLE CConfigGUIBackgroundBlendDlg :
	public CCustomConfigWndImpl<CConfigGUIBackgroundBlendDlg>,
	public CDialogResize<CConfigGUIBackgroundBlendDlg>
{
public:
	CConfigGUIBackgroundBlendDlg() : m_wndBackground(false)
	{
	}
	enum { IDD = IDD_CONFIGGUI_BACKGROUNDBLEND };

	BEGIN_MSG_MAP(CConfigGUIBackgroundBlendDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIBackgroundBlendDlg>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUIBackgroundBlendDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_HANDLER(IDC_CGPX_BACKGROUND, CButtonColorPicker::BCPN_SELCHANGE, OnBackgroundChanged)
		REFLECT_NOTIFICATIONS()
		if (uMsg == WM_RW_CFGSPLIT) { if (lParam) *reinterpret_cast<float*>(lParam) = 0.5f; return TRUE; }
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIBackgroundBlendDlg)
		DLGRESIZE_CONTROL(IDC_CGPX_THRESHOLD, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIBackgroundBlendDlg)
		CONFIGITEM_EDITBOX(IDC_CGPX_THRESHOLD, CFGID_THRESHOLD)
		CONFIGITEM_CONTEXTHELP(IDC_CGPX_BACKGROUND, CFGID_BACKGROUND)
	END_CONFIGITEM_MAP()

	void ExtraInitDialog()
	{
		// initialize color button
		m_wndBackground.m_tLocaleID = m_tLocaleID;
		m_wndBackground.SubclassWindow(GetDlgItem(IDC_CGPX_BACKGROUND));
		m_wndBackground.SetDefaultColor(CButtonColorPicker::SColor(RGB(212, 208, 200)));
	}
	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CConfigValue cValColor;
		M_Config()->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cValColor);
		m_wndBackground.SetColor(CButtonColorPicker::SColor(cValColor.operator LONG()));

		DlgResize_Init(false, false, 0);

		return 1;
	}
	LRESULT OnBackgroundChanged(WPARAM a_wParam, LPNMHDR a_pNMHdr, BOOL& a_bHandled)
	{
		try
		{
			CButtonColorPicker::NMCOLORBUTTON const* const pClrBtn = reinterpret_cast<CButtonColorPicker::NMCOLORBUTTON const* const>(a_pNMHdr);
			CComBSTR cCFGID_BACKGROUND_COLOR(CFGID_BACKGROUND);
			CConfigValue cValColor(static_cast<LONG>(pClrBtn->clr.ToCOLORREF()));
			BSTR aIDs[1];
			aIDs[0] = cCFGID_BACKGROUND_COLOR;
			TConfigValue aVals[1];
			aVals[0] = cValColor;
			M_Config()->ItemValuesSet(1, aIDs, aVals);
		}
		catch (...)
		{
		}

		return 0;
	}

private:
	CButtonColorPicker m_wndBackground;
};


// CDocumentOperationRasterImageBackgroundBlend

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEBACKGROUNDBLEND_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
//#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Set background[0405]Nastavit pozadí");
			return S_OK;
		}
		CConfigValue cBackground;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cBackground);
		CComPtr<INamedColors> pNC;
		RWCoCreateInstance(pNC, __uuidof(NamedColors));
		CComPtr<ILocalizedString> pColorName;
		if (pNC) pNC->ColorToName(0xff000000|cBackground.operator LONG(), &pColorName);
		if (pColorName == NULL)
		{
			*a_ppName = new CMultiLanguageString(L"[0409]Set background[0405]Nastavit pozadí");
			return S_OK;
		}
		CComObject<CPrintfLocalizedString>* pStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pStr);
		CComPtr<ILocalizedString> pTmp = pStr;
		pStr->Init(CMultiLanguageString::GetAuto(L"[0409]Set %s background[0405]Barva pozadí %s"), pColorName);
		*a_ppName = pTmp.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_BACKGROUND), _SharedStringTable.GetStringAuto(IDS_CFGID_BACKGROUND_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_BACKGROUND_HELP), CConfigValue(LONG(RGB(255, 255, 255))), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_THRESHOLD), _SharedStringTable.GetStringAuto(IDS_CFGID_THRESHOLD_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_THRESHOLD_HELP), CConfigValue(0L), NULL, CConfigValue(0L), CConfigValue(255L), CConfigValue(1L), 0, NULL);

		// finalize the initialization of the config
		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageBackgroundBlend, CConfigGUIBackgroundBlendDlg>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentRasterImage)/*, __uuidof(IRasterImageControl)*/};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <GammaCorrection.h>
#include "PixelLevelTask.h"

struct CBackgroundBlendOp
{
	CGammaTables const* pGT;
	ULONG bBgR;
	ULONG bBgG;
	ULONG bBgB;
	BYTE nThreshold;
	void Process(TPixelChannel* pD, TPixelChannel const* pS, size_t const s, size_t const n)
	{
		for (TPixelChannel* const pE = pD+n; pD != pE; ++pD, pS+=s)
		{
			if (pS->bA < nThreshold)
			{
				pD->n = 0;
			}
			else if (pS->bA == 0xff)
			{
				*pD = *pS;
			}
			else
			{
				ULONG const nA = (pS->bA<<16)/255;
				ULONG const nIA = (1<<16)-nA;
				TPixelChannel const t =
				{
					pGT->InvGamma((pGT->m_aGamma[pS->bB]*nA + bBgB*nIA)>>16),
					pGT->InvGamma((pGT->m_aGamma[pS->bG]*nA + bBgG*nIA)>>16),
					pGT->InvGamma((pGT->m_aGamma[pS->bR]*nA + bBgR*nIA)>>16),
					255,
				};
				*pD = t;
			}
		}
	}
};

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		CConfigValue cThreshold;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_THRESHOLD), &cThreshold);
		bool const bEverywhere = cThreshold.operator LONG() == 0;

		TImageSize tSize = {1, 1};
		TImagePoint tOrig = {0, 0};
		pRI->CanvasGet(NULL, NULL, &tOrig, &tSize, NULL);
		TRasterImageRect tR = {tOrig, {tOrig.nX+tSize.nX, tOrig.nY+tSize.nY}};

		CBackgroundBlendOp cOp;

		CComPtr<IGammaTableCache> pGTC;
		RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
		cOp.pGT = pGTC->GetSRGBTable();

		cOp.nThreshold = cThreshold.operator LONG();
		CConfigValue cBackground;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_BACKGROUND), &cBackground);
		cOp.bBgR = cOp.pGT ? cOp.pGT->m_aGamma[GetRValue(cBackground.operator LONG())] : GetRValue(cBackground.operator LONG());
		cOp.bBgG = cOp.pGT ? cOp.pGT->m_aGamma[GetGValue(cBackground.operator LONG())] : GetGValue(cBackground.operator LONG());
		cOp.bBgB = cOp.pGT ? cOp.pGT->m_aGamma[GetBValue(cBackground.operator LONG())] : GetBValue(cBackground.operator LONG());

		if (bEverywhere)
		{
			EImageChannelID eChID = EICIRGBA;
			TPixelChannel tChDef;
			tChDef.bA = 0xff;
			tChDef.bB = GetBValue(cBackground.operator LONG());
			tChDef.bG = GetGValue(cBackground.operator LONG());
			tChDef.bR = GetRValue(cBackground.operator LONG());
			pRI->ChannelsSet(1, &eChID, &tChDef);
		}

		if (tSize.nX*tSize.nY == 0)
			return S_FALSE;

		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[tSize.nX*tSize.nY]);

		CComPtr<IThreadPool> pThPool;
		if (tSize.nY >= 16 && tSize.nX*tSize.nY > 128*128)
			RWCoCreateInstance(pThPool, __uuidof(ThreadPool));

		{
			CComObjectStackEx<CPixelLevelTask<CBackgroundBlendOp> > cXform;
			cXform.Init(pRI, tR, pSrc.m_p, cOp);

			if (pThPool)
				pThPool->Execute(0, &cXform);
			else
				cXform.Execute(0, 1);
		}

		return pRI->TileSet(EICIRGBA, &tR.tTL, &tSize, NULL, tSize.nX*tSize.nY, pSrc, FALSE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageBackgroundBlend::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pConfig && a_pRect)
	{
		CConfigValue cThreshold;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_THRESHOLD), &cThreshold);
		bool const bEverywhere = cThreshold.operator LONG() == 0;
		if (bEverywhere)
		{
			a_pRect->tTL.nX = a_pRect->tTL.nY = LONG_MIN;
			a_pRect->tBR.nX = a_pRect->tBR.nY = LONG_MAX;
		}
	}
	return S_OK;
}
