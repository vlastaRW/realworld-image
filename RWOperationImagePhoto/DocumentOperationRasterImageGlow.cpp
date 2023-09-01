// DocumentOperationRasterImageGlow.cpp : Implementation of CDocumentOperationRasterImageGlow

#include "stdafx.h"
#include "DocumentOperationRasterImageGlow.h"
#include <SharedStringTable.h>
#include <math.h>
#include <GammaCorrection.h>
#include <IconRenderer.h>


// CDocumentOperationRasterImageGlow

STDMETHODIMP CDocumentOperationRasterImageGlow::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_DOCOPGLOW_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

static OLECHAR const CFGID_SG_RADIUS[] = L"Radius";
static OLECHAR const CFGID_SG_INTENSITY[] = L"Intensity";
static OLECHAR const CFGID_SG_COLORED[] = L"Colored";
static OLECHAR const CFGID_SG_MASKID[] = L"MaskID";

#include <MultiLanguageString.h>
//#include <PrintfLocalizedString.h>
//#include <SimpleLocalizedString.h>

STDMETHODIMP CDocumentOperationRasterImageGlow::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Glow[0405]Záření";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		CConfigValue cColored;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SG_COLORED), &cColored);
		if (cColored.operator bool())
			pszName = L"[0409]Colored glow[0405]Barevné záření";
		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

HICON CDocumentOperationRasterImageGlow::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const aBulb[] =
	{
		{167.137, 187.492, 15.0445, -9.8824, -15.0445, 9.88242},
		{188.155, 155.739, -21.6987, -31.2117, 6.08835, 1.98294},
		{163.104, 46.5671, -21.8316, -33.2355, 19.4961, 29.68},
		{61.7251, 27.0167, -34.1587, 22.4381, 34.1587, -22.4381},
		{39.405, 127.822, 19.4961, 29.68, -21.8316, -33.2355},
		{129.649, 194.17, -0.598114, 6.37512, -20.0271, -32.3097},
	};
	static IRPath const tBulb = {itemsof(aBulb), aBulb};
	static IRPathPoint const aHilight[] =
	{
		{85, 51, -19.3029, 7.2386, 24, -9},
		{62, 95, 5, 26, -4.44494, -23.1137},
		{80, 78, 8.20975, -16.4195, -8, 16},
	};
	static IRPath const tHilight = {itemsof(aHilight), aHilight};
	static IRPathPoint const aContact[] =
	{
		{202.275, 240.983, 15.0445, -9.88242, -15.0445, 9.88242},
		{214.099, 215.27, 10.0991, -13.3795, -8.95622, 11.8654},
		{187.451, 218.417, -6.68646, 4.39218, 6.68646, -4.39218},
		{173.98, 241.623, 14.4464, -3.50729, -16.2899, 3.95483},
	};
	static IRPath const tContact = {itemsof(aContact), aContact};
	static IRPathPoint const aScrew1[] =
	{
		{173.177, 196.686, 15.0445, -9.8824, -15.0445, 9.88242},
		{204.437, 167.776, -4.55621, -23.329, 2.03763, 10.433},
		{160, 176.626, -20.0594, 13.1765, 20.0594, -13.1765},
		{134.23, 213.894, 8.76573, 6.01349, -19.6008, -13.4466},
	};
	static IRPath const tScrew1 = {itemsof(aScrew1), aScrew1};
	static IRPathPoint const aScrew2[] =
	{
		{184.157, 213.402, 12.5371, -8.23535, -12.5371, 8.23532},
		{212.361, 185.303, -4.55624, -23.329, 2.0376, 10.433},
		{170.981, 193.342, -17.552, 11.5295, 17.5519, -11.5295},
		{147.168, 228.127, 8.76573, 6.01349, -19.6008, -13.4466},
	};
	static IRPath const tScrew2 = {itemsof(aScrew2), aScrew2};
	static IRPathPoint const aScrew3[] =
	{
		{194.589, 229.282, 10.8655, -7.1373, -10.8655, 7.1373},
		{220.285, 202.831, -4.84305, -21.9441, 2.29089, 10.3804},
		{181.412, 209.223, -15.8803, 10.4314, 15.8803, -10.4314},
		{160.107, 242.36, 8.61679, 6.22504, -18.2159, -13.1598},
	};
	static IRPath const tScrew3 = {itemsof(aScrew3), aScrew3};
	static IRCanvas const tCanvas = {0, 0, 256, 256, 0, NULL, 0, NULL};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&tCanvas, 1, &tBulb, pSI->GetMaterial(ESMScheme1Color1));
	IRFill hl(0x3fffffff);
	cRenderer(&tCanvas, 1, &tHilight, &hl);//pSI->GetMaterial(ESMScheme1Color1));
	cRenderer(&tCanvas, 1, &tContact, pSI->GetMaterial(ESMInterior));
	cRenderer(&tCanvas, 1, &tScrew1, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&tCanvas, 1, &tScrew2, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&tCanvas, 1, &tScrew3, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}

#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUIGlow :
	public CCustomConfigResourcelessWndImpl<CConfigGUIGlow>,
	public CDialogResize<CConfigGUIGlow>
{
public:
	CConfigGUIGlow()
	{
	}

	enum { IDC_CG_RADIUS = 100, IDC_CG_RADIUSUNITS, IDC_CG_INTENSITYEDIT, IDC_CG_INTENSITYSLIDER, IDC_CG_COLORED, IDC_CG_MASKID, IDC_CG_MASKIDLABEL };

	BEGIN_DIALOG_EX(0, 0, 120, 42, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Glow radius:[0405]Poloměr:"), IDC_STATIC, 0, 2, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_RADIUS, 44, 0, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]pixels[0405]pixelů"), IDC_CG_RADIUSUNITS, 98, 2, 22, 8, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Intensity:[0405]Intenzita:"), IDC_STATIC, 0, 18, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_INTENSITYEDIT, 44, 16, 29, 12, WS_VISIBLE | WS_TABSTOP | ES_RIGHT | ES_AUTOHSCROLL, 0)
		CONTROL_CONTROL(_T(""), IDC_CG_INTENSITYSLIDER, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 73, 16, 47, 12, 0)
		CONTROL_CHECKBOX(_T("[0409]Colored[0405]Barevné"), IDC_CG_COLORED, 0, 32, 120, 10, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]Mask ID:[0405]ID masky:"), IDC_CG_MASKIDLABEL, 0, 48, 44, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_CG_MASKID, 44, 46, 50, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIGlow)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIGlow>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIGlow>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIGlow)
		DLGRESIZE_CONTROL(IDC_CG_RADIUS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_RADIUSUNITS, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CG_COLORED, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_INTENSITYSLIDER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CG_MASKID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIGlow)
		CONFIGITEM_EDITBOX(IDC_CG_RADIUS, CFGID_SG_RADIUS)
		CONFIGITEM_CHECKBOX(IDC_CG_COLORED, CFGID_SG_COLORED)
		CONFIGITEM_EDITBOX(IDC_CG_INTENSITYEDIT, CFGID_SG_INTENSITY)
		CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CG_INTENSITYSLIDER, CFGID_SG_INTENSITY)
		CONFIGITEM_EDITBOX(IDC_CG_MASKID, CFGID_SG_MASKID)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		CComPtr<IDocument> pDoc;
		GetParent().SendMessage(WM_RW_GETCFGDOC, 0, reinterpret_cast<LPARAM>(&pDoc));
		if (pDoc)
		{
			CEdit wnd = GetDlgItem(IDC_CG_MASKID);
			wnd.SetReadOnly(TRUE);
			wnd.ModifyStyle(WS_TABSTOP, 0);
			wnd.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CG_MASKIDLABEL).ShowWindow(SW_HIDE);
		}

		DlgResize_Init(false, false, 0);

		return 1;
	}
};


STDMETHODIMP CDocumentOperationRasterImageGlow::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SG_RADIUS), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_RADIUS_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_RADIUS_DESC), CConfigValue(10L), NULL, 0, NULL);
		pCfgInit->ItemInsRanged(CComBSTR(CFGID_SG_INTENSITY), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_INTENSITY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_INTENSITY_DESC), CConfigValue(50L), NULL, CConfigValue(0L), CConfigValue(100L), CConfigValue(1L), 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SG_COLORED), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_COLORED_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_COLORED_DESC), CConfigValue(true), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SG_MASKID), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_MASKID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SG_MASKID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 0, NULL);
		//pCfgInit->ItemInsRanged(CComBSTR(CFGID_SG_SHARPNESS), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_YPOSITION_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_WM_YPOSITION_DESC), CConfigValue(0.5f), NULL, CConfigValue(0.0f), CConfigValue(1.0f), CConfigValue(0.01f), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImageGlow, CConfigGUIGlow>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageGlow::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates))
{
	try
	{
		if (a_pDocument == NULL)
			return E_FAIL;

		static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
		return SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <math.h>
#include <agg_basics.h>
#include <agg_color_rgba.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rendering_buffer.h>
#include <agg_blur.h>

void Glow(TPixelChannel const* a_pS, TPixelChannel* a_pD, LONG a_nSizeX, LONG a_nSizeY, float a_fIntensity, LONG a_nRadius, bool a_bColored, BYTE const* a_pMask, CGammaTables const* a_pGamma)
{
	// isolate intensities
	BYTE aSigmoid[256];
	for (int i = 0; i < 256; ++i)
	{
		if (a_bColored && a_pGamma)
		{
			float f = (0.33f+a_fIntensity*0.67f) * 255.0f / (1 + expf (-20.0f * (a_pGamma->m_aGamma[i] / (255.0f*256.0f) - (0.67f-a_fIntensity*0.33f))));
			aSigmoid[i] = f < 0.0f ? 0 : (f > 255.0f ? 255 : unsigned(f+0.5f));
		}
		else
		{
			float f = (0.33f+a_fIntensity*0.67f) * 255.0f / (1 + expf (-20.0f * (i / 255.0f - (0.67f-a_fIntensity*0.33f))));
			aSigmoid[i] = f < 0.0f ? 0 : (f > 255.0f ? 255 : unsigned(f+0.5f));
		}
	}
	ULONG const nPixels = a_nSizeX*a_nSizeY;
	TPixelChannel const* const pSEnd = a_pS+nPixels;
	TPixelChannel* const pDEnd = a_pD+nPixels;
	TPixelChannel const* pS = a_pS;
	TPixelChannel* pD = a_pD;
	if (a_bColored)
	{
		while (pS != pSEnd)
		{
			pD->bA = pS->bA;
			if (pD->bA == 255)
			{
				pD->bR = aSigmoid[pS->bR];
				pD->bG = aSigmoid[pS->bG];
				pD->bB = aSigmoid[pS->bB];
			}
			else
			{
				pD->bR = (aSigmoid[pS->bR]*int(pD->bA))>>8;
				pD->bG = (aSigmoid[pS->bG]*int(pD->bA))>>8;
				pD->bB = (aSigmoid[pS->bB]*int(pD->bA))>>8;
			}
			pD->bA = pD->bR > pD->bG ? (pD->bR > pD->bB ? pD->bR : pD->bB) : (pD->bG > pD->bB ? pD->bG : pD->bB);
			++pD; ++pS;
		}
	}
	else
	{
		if (a_pGamma)
		{
			while (pS != pSEnd)
			{
				if (pS->bA == 255)
					pD->bA = pD->bR = pD->bG = pD->bB = aSigmoid[(4*a_pGamma->m_aGamma[pS->bG] + 3*a_pGamma->m_aGamma[pS->bR] + a_pGamma->m_aGamma[pS->bB] + 0x400)>>11];
				else
					pD->bA = pD->bR = pD->bG = pD->bB = (aSigmoid[(4*a_pGamma->m_aGamma[pS->bG] + 3*a_pGamma->m_aGamma[pS->bR] + a_pGamma->m_aGamma[pS->bB] + 0x400)>>11]*int(pS->bA))>>8;
				++pD; ++pS;
			}
		}
		else
		{
			while (pS != pSEnd)
			{
				if (pS->bA == 255)
					pD->bA = pD->bR = pD->bG = pD->bB = aSigmoid[(4*pS->bG + 3*pS->bR + pS->bB + 4)>>3];
				else
					pD->bA = pD->bR = pD->bG = pD->bB = (aSigmoid[(4*pS->bG + 3*pS->bR + pS->bB + 4)>>3]*int(pS->bA))>>8;
				++pD; ++pS;
			}
		}
	}

	// mask
	if (a_pMask)
	{
		pD = a_pD;
		while (pD != pDEnd)
		{
			pD->bR = (pD->bR*(*a_pMask+1))>>8;
			pD->bG = (pD->bG*(*a_pMask+1))>>8;
			pD->bB = (pD->bB*(*a_pMask+1))>>8;
			pD->bA = (pD->bA*(*a_pMask+1))>>8;
			++a_pMask;
			++pD;
		}
	}

	// blur
	agg::rendering_buffer rbuf(reinterpret_cast<agg::int8u*>(a_pD), a_nSizeX, a_nSizeY, a_nSizeX*sizeof*a_pD);
	agg::pixfmt_bgra32 pixf(rbuf);
	agg::stack_blur_rgba32(pixf, a_nRadius, a_nRadius);

	// blend
	pS = a_pS;
	pD = a_pD;
	while (pS != pSEnd)
	{
		if (pD->bA == 0)
		{
			*pD = *pS;
		}
		else if (a_pGamma)
		{
			if (pD->bA < 255 && pS->bA > 0)
			{
				ULONG nNewA = pD->bA*255 + (255-pD->bA)*static_cast<ULONG>(pS->bA);
				if (nNewA)
				{
					pD->bR = a_pGamma->InvGamma((a_pGamma->m_aGamma[pS->bR]*(255-pD->bA)*pS->bA + pD->bR*256*255*255)/nNewA);
					pD->bG = a_pGamma->InvGamma((a_pGamma->m_aGamma[pS->bG]*(255-pD->bA)*pS->bA + pD->bG*256*255*255)/nNewA);
					pD->bB = a_pGamma->InvGamma((a_pGamma->m_aGamma[pS->bB]*(255-pD->bA)*pS->bA + pD->bB*256*255*255)/nNewA);
				}
				else
				{
					pD->bR = pD->bG = pD->bB = 0;
				}
				pD->bA = nNewA/255;
			}
			else
			{
				pD->bR = a_pGamma->InvGamma((pD->bR*256*255)/pD->bA);
				pD->bG = a_pGamma->InvGamma((pD->bG*256*255)/pD->bA);
				pD->bB = a_pGamma->InvGamma((pD->bB*256*255)/pD->bA);
			}
		}
		else
		{
			if (pD->bA < 255 && pS->bA > 0)
			{
				ULONG nNewA = pD->bA*255 + (255-pD->bA)*static_cast<ULONG>(pS->bA);
				if (nNewA)
				{
					pD->bR = (static_cast<ULONG>(pS->bR)*(255-pD->bA)*pS->bA + static_cast<ULONG>(pD->bR)*65025)/nNewA;
					pD->bG = (static_cast<ULONG>(pS->bG)*(255-pD->bA)*pS->bA + static_cast<ULONG>(pD->bG)*65025)/nNewA;
					pD->bB = (static_cast<ULONG>(pS->bB)*(255-pD->bA)*pS->bA + static_cast<ULONG>(pD->bB)*65025)/nNewA;
				}
				else
				{
					pD->bR = pD->bG = pD->bB = 0;
				}
				pD->bA = nNewA/255;
			}
			else
			{
				pD->bR = (pD->bR*255)/pD->bA;
				pD->bG = (pD->bG*255)/pD->bA;
				pD->bB = (pD->bB*255)/pD->bA;
			}
		}
		++pD; ++pS;
	}
}

STDMETHODIMP CDocumentOperationRasterImageGlow::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SG_RADIUS), &cVal);
		LONG nRadius = cVal.operator LONG();

		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));

		TPixelChannel tDefault;
		tDefault.n = 0;
		pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		CComPtr<IGammaTableCache> pGTC;
		RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
		CGammaTables const* pGT = pGTC ? pGTC->GetSRGBTable() : NULL;

		TImagePoint tOrigin = {0, 0};
		TImageSize tSize = {0, 0};
		if (FAILED(pRI->CanvasGet(NULL, NULL, &tOrigin, &tSize, NULL)))
			return E_FAIL;

		if (tSize.nX*tSize.nY == 0)
			return S_FALSE; // TODO: update the default ( && tDefault.bA == 0 )

		TRasterImageRect tRect = {{tOrigin.nX-nRadius, tOrigin.nY-nRadius}, {tOrigin.nX+tSize.nX+nRadius+nRadius, tOrigin.nY+tSize.nY+nRadius+nRadius}};
		tSize.nX += nRadius+nRadius;
		tSize.nY += nRadius+nRadius;

		a_pConfig->ItemValueGet(CComBSTR(CFGID_SG_INTENSITY), &cVal);
		LONG nIntensity = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SG_COLORED), &cVal);
		bool bColored = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SG_MASKID), &cVal);
		CComPtr<ISharedStateImageSelection> pSel;
		a_pStates->StateGet(cVal, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pSel));
		CAutoVectorPtr<BYTE> cMask;
		if (pSel)
		{
			LONG nX = tRect.tTL.nX;
			LONG nY = tRect.tTL.nY;
			ULONG nDX = tRect.tBR.nX-tRect.tTL.nX;
			ULONG nDY = tRect.tBR.nY-tRect.tTL.nY;
			if (SUCCEEDED(pSel->Bounds(&nX, &nY, &nDX, &nDY)))
			{
				if (nX <= LONG(tRect.tTL.nX) && nY <= LONG(tRect.tTL.nY) &&
					nDX >= ULONG(tRect.tBR.nX-tRect.tTL.nX) && nDY >= ULONG(tRect.tBR.nY-tRect.tTL.nY) && S_OK == pSel->IsEmpty())
				{
					// everything is selected
				}
				else
				{
					nX = tRect.tTL.nX;
					nY = tRect.tTL.nY;
					nDX = (tRect.tBR.nX-tRect.tTL.nX);
					nDY = (tRect.tBR.nY-tRect.tTL.nY);
					cMask.Allocate(nDX*nDY);
					pSel->GetTile(nX, nY, nDX, nDY, nDX, cMask.m_p);
				}
			}
		}

		ULONG nPixels = tSize.nX*tSize.nY;
		CAutoVectorPtr<TPixelChannel> pSrc(new TPixelChannel[nPixels]);
		HRESULT hRes = pRI->TileGet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pSrc.m_p, NULL, EIRIAccurate);
		if (FAILED(hRes))
			return hRes;
		CAutoVectorPtr<TPixelChannel> pDst(new TPixelChannel[nPixels]);
		TPixelChannel* pD = pDst.m_p;
		TPixelChannel* pS = pSrc.m_p;
		LONG nSize = tSize.nX*tSize.nY;
		//if (bHasAlpha)
		//{
		//	for (TRasterImagePixel* p = pS; nSize; --nSize, ++p)
		//	{
		//		// premultiply
		//		p->bR = (unsigned(p->bR)*p->bA)/255;
		//		p->bG = (unsigned(p->bG)*p->bA)/255;
		//		p->bB = (unsigned(p->bB)*p->bA)/255;
		//	}
		//	Glow(pS, pD, tSize.n0, tSize.n1, nIntensity*0.01f, nRadius, bColored, cMask);
		//	nSize = tSize.n0*tSize.n1;
		//	if (bHasAlpha) for (TRasterImagePixel* p = pD; nSize; --nSize, ++p)
		//	{
		//		// demultiply
		//		if (p->bA)
		//		{
		//			p->bR = (unsigned(p->bR)*255)/p->bA;
		//			p->bG = (unsigned(p->bG)*255)/p->bA;
		//			p->bB = (unsigned(p->bB)*255)/p->bA;
		//		}
		//		else
		//		{
		//			p->bR = p->bG = p->bB = 0;
		//		}
		//	}
		//}
		//else
		{
			Glow(pS, pD, tSize.nX, tSize.nY, nIntensity*0.01f, nRadius, bColored, cMask, pGT);
		}
		return pRI->TileSet(EICIRGBA, &tRect.tTL, &tSize, NULL, nPixels, pDst.m_p, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImageGlow::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	if (a_pConfig == NULL || a_pContentTransform == NULL)
		return E_RW_INVALIDPARAM;
	float const f = Matrix3x3fDecomposeScale(*a_pContentTransform);
	if (f > 0.9999f && f < 1.0001f)
		return S_FALSE;
	CComBSTR bstrRADIUS(CFGID_SG_RADIUS);
	CConfigValue cVal;
	a_pConfig->ItemValueGet(bstrRADIUS, &cVal);
	if (cVal.TypeGet() != ECVTInteger) return E_FAIL;
	cVal = LONG(cVal.operator LONG()*f);
	return a_pConfig->ItemValuesSet(1, &(bstrRADIUS.m_str), cVal);
}

STDMETHODIMP CDocumentOperationRasterImageGlow::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	if (a_pRect && a_pConfig)
	{
		CComBSTR bstrRADIUS(CFGID_SG_RADIUS);
		CConfigValue cVal;
		a_pConfig->ItemValueGet(bstrRADIUS, &cVal);
		LONG nRadius = cVal;

		a_pRect->tTL.nX -= nRadius;
		a_pRect->tTL.nY -= nRadius;
		a_pRect->tBR.nX += nRadius;
		a_pRect->tBR.nY += nRadius;
	}
	return S_OK;
}
