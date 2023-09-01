
#include "stdafx.h"
#include <RWProcessing.h>
#include <MultiLanguageString.h>
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <DocumentMenuCommandImpl.h>
#include <GammaCorrection.h>


// CMenuCommandsLayerDefault

// {3BD46680-771F-4060-B86A-C02C75CB1FE3}
extern GUID const CLSID_MenuCommandsLayerDefault = {0x3bd46680, 0x771f, 0x4060, {0xb8, 0x6a, 0xc0, 0x2c, 0x75, 0xcb, 0x1f, 0xe3}};
static const OLECHAR CFGID_SELECTIONSYNC[] = L"LayerID";

extern wchar_t const DEFAULTCOLOR_WHITE[] = L"[0409]White[0405]Bílá";
extern wchar_t const DEFAULTCOLOR_GRAY[] = L"[0409]Gray[0405]Šedá";
extern wchar_t const DEFAULTCOLOR_BLACK[] = L"[0409]Black[0405]Černá";
extern wchar_t const DEFAULTCOLOR_TRANSPARENT[] = L"[0409]Transparent[0405]Průhledná";
extern wchar_t const DEFAULTCOLOR_CUSTOM[] = L"[0409]Custom...[0405]Vlastní...";
extern wchar_t const DEFAULTCOLOR_NAME[] = L"[0409]Change default color[0405]Změnit výchozí barvu";
extern wchar_t const DEFAULTCOLOR_DESC[] = L"[0409]Change the default color of the current layer.[0405]Změnit výchozí barvu vybrané vrstvy.";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUILayerDefaultDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUILayerDefaultDlg>,
	public CDialogResize<CConfigGUILayerDefaultDlg>
{
public:
	enum
	{
		IDC_LAYERSYNCID = 100,
	};

	BEGIN_DIALOG_EX(0, 0, 180, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizace výběru:"), IDC_STATIC, 0, 2, 60, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_LAYERSYNCID, 60, 0, 119, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUILayerDefaultDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUILayerDefaultDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUILayerDefaultDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUILayerDefaultDlg)
		DLGRESIZE_CONTROL(IDC_LAYERSYNCID, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUILayerDefaultDlg)
		CONFIGITEM_EDITBOX(IDC_LAYERSYNCID, CFGID_SELECTIONSYNC)
	END_CONFIGITEM_MAP()


	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

class ATL_NO_VTABLE CMenuCommandsLayerDefault :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsLayerDefault, &CLSID_MenuCommandsLayerDefault>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsLayerDefault()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsLayerDefault)

BEGIN_CATEGORY_MAP(CMenuCommandsLayerDefault)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsLayerDefault)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL)
			return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Default Color[0405]Vrstvený obrázek - výchozí barva");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;
			CComPtr<IConfigWithDependencies> pCfg;
			RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

			pCfg->ItemInsSimple(CComBSTR(CFGID_SELECTIONSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection sync ID[0405]Synchronizace výběru"), CMultiLanguageString::GetAuto(L"[0409]Image selection is synchronized by the given ID.[0405]Vybraná oblast v obrázku je dopstupná a sychronizována přes zadané ID."), CConfigValue(L"LAYER"), NULL, 0, NULL);

			CConfigCustomGUI<&CLSID_MenuCommandsLayerDefault, CConfigGUILayerDefaultDlg>::FinalizeConfig(pCfg);

			*a_ppDefaultConfig = pCfg.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IDocumentLayeredImage> pDLI;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
			if (pDLI == NULL)
				return S_FALSE;

			CConfigValue cSelID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cSelID);
			CComBSTR bstrID;
			pDLI->StatePrefix(&bstrID);
			if (bstrID.Length())
			{
				bstrID += cSelID.operator BSTR();
			}
			else
			{
				bstrID = cSelID.operator BSTR();
			}

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));

			CComPtr<IEnumUnknowns> pItems;
			pDLI->StateUnpack(pState, &pItems);
			ULONG nItems = 0;
			if (pItems) pItems->Size(&nItems);
			if (nItems == 0)
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pCmds;
			RWCoCreateInstance(pCmds, __uuidof(EnumUnknowns));

			{
				CComObject<CSetColor>* p = NULL;
				CComObject<CSetColor>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, DEFAULTCOLOR_TRANSPARENT, CPixelChannel(0UL), a_pView);

				pCmds->Insert(p);
			}
			{
				CComObject<CSetColor>* p = NULL;
				CComObject<CSetColor>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, DEFAULTCOLOR_BLACK, CPixelChannel(ULONG(0xff000000)), a_pView);

				pCmds->Insert(p);
			}
			{
				CComObject<CSetColor>* p = NULL;
				CComObject<CSetColor>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, DEFAULTCOLOR_GRAY, CPixelChannel(ULONG(0xffbcbcbc)), a_pView);

				pCmds->Insert(p);
			}
			{
				CComObject<CSetColor>* p = NULL;
				CComObject<CSetColor>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, DEFAULTCOLOR_WHITE, CPixelChannel(ULONG(0xffffffff)), a_pView);

				pCmds->Insert(p);
			}
			{
				CComPtr<IDocumentMenuCommand> pSep;
				RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
				pCmds->Insert(pSep);
			}
			{
				CComObject<CSetCustomColor>* p = NULL;
				CComObject<CSetCustomColor>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;

				p->Init(a_pDocument, pDLI, pItems, a_pView);

				pCmds->Insert(p);
			}

			*a_ppSubCommands = pCmds.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	class ATL_NO_VTABLE CSetColor :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pItems, wchar_t const* a_pszColor, TPixelChannel a_tColor, IDesignerView* a_pView)
		{
			m_pItems = a_pItems;
			m_pszColor = a_pszColor;
			m_tColor = a_tColor;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
		}

	BEGIN_COM_MAP(CSetColor)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()


		// IDocumentMenuCommand methods
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(m_pszColor);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				*a_ppText = new CMultiLanguageString(DEFAULTCOLOR_DESC);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (a_pIconID == NULL)
				return E_POINTER;
			try
			{
				*a_pIconID = CLSID_MenuCommandsLayerDefault;
				a_pIconID->Data1 = m_tColor.n;
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				*a_phIcon = NULL;


				DWORD nXOR = a_nSize*a_nSize<<2;
				DWORD nAND = a_nSize*((((a_nSize+7)>>3)+3)&0xfffffffc);
				CAutoVectorPtr<BYTE> pIconRes(new BYTE[nXOR+nAND+sizeof BITMAPINFOHEADER]);
				ZeroMemory(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER);
				BITMAPINFOHEADER* pHead = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
				pHead->biSize = sizeof(BITMAPINFOHEADER);
				pHead->biWidth = a_nSize;
				pHead->biHeight = a_nSize<<1;
				pHead->biPlanes = 1;
				pHead->biBitCount = 32;
				pHead->biCompression = BI_RGB;
				pHead->biSizeImage = nXOR+nAND;
				pHead->biXPelsPerMeter = 0;
				pHead->biYPelsPerMeter = 0;
				DWORD *pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);

				DWORD dwOutline = 0xff000000|GetSysColor(COLOR_BTNTEXT);
				std::fill_n(pXOR+a_nSize+1, a_nSize-2, dwOutline);
				std::fill_n(pXOR+a_nSize*(a_nSize-2)+1, a_nSize-2, dwOutline);
				if (m_tColor.bA == 0xff)
				{
					for (ULONG y = 2; y < a_nSize-2; ++y)
					{
						pXOR[y*a_nSize+1] = dwOutline;
						std::fill_n(pXOR+y*a_nSize+2, a_nSize-4, m_tColor.n);
						pXOR[y*a_nSize+a_nSize-2] = dwOutline;
					}
				}
				else
				{
					DWORD dwClr1 = 0xff000000|GetSysColor(COLOR_3DLIGHT);
					DWORD dwClr2 = 0xff000000|GetSysColor(COLOR_3DSHADOW);
					for (ULONG y = 2; y < a_nSize>>1; ++y)
					{
						pXOR[y*a_nSize+1] = dwOutline;
						std::fill_n(pXOR+y*a_nSize+2, (a_nSize>>1)-2, dwClr1);
						std::fill_n(pXOR+y*a_nSize+(a_nSize>>1), a_nSize-2-(a_nSize>>1), dwClr2);
						pXOR[y*a_nSize+a_nSize-2] = dwOutline;
					}
					for (ULONG y = a_nSize>>1; y < a_nSize-2; ++y)
					{
						pXOR[y*a_nSize+1] = dwOutline;
						std::fill_n(pXOR+y*a_nSize+2, (a_nSize>>1)-2, dwClr2);
						std::fill_n(pXOR+y*a_nSize+(a_nSize>>1), a_nSize-2-(a_nSize>>1), dwClr1);
						pXOR[y*a_nSize+a_nSize-2] = dwOutline;
					}
				}

				pXOR = reinterpret_cast<DWORD *>(pIconRes+sizeof BITMAPINFOHEADER);
				BYTE *pAND = reinterpret_cast<BYTE*>(pXOR+(a_nSize*a_nSize));
				int nANDLine = ((((a_nSize+7)>>3)+3)&0xfffffffc);
				for (ULONG y = 0; y < a_nSize; ++y)
				{
					for (ULONG x = 0; x < a_nSize; ++x)
					{
						if (0 == (0xff000000&*pXOR))
						{
							pAND[x>>3] |= 0x80 >> (x&7);
						}
						++pXOR;
					}
					pAND += nANDLine;
				}
				*a_phIcon = CreateIconFromResourceEx(pIconRes.m_p, nXOR+nAND+sizeof BITMAPINFOHEADER, TRUE, 0x00030000, a_nSize, a_nSize, LR_DEFAULTCOLOR);
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Accelerator)(TCmdAccel* a_pAccel, TCmdAccel* a_pAuxAccel)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** UNREF(a_ppSubCommands))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				TPixelChannel tCurrent = m_tColor;
				bool bMatching = false;
				*a_peState = GetCurrentColor(m_pDocument, m_pDLI, m_pItems, &bMatching, &tCurrent) ? (bMatching && tCurrent.n == m_tColor.n ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			if (m_pView)
				m_pView->DeactivateAll(FALSE);

			return SetDefaultColor(m_pDocument, m_pDLI, m_pItems, m_tColor);
		}

	private:
		CComPtr<IEnumUnknowns> m_pItems;
		wchar_t const* m_pszColor;
		TPixelChannel m_tColor;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	class ATL_NO_VTABLE CSetCustomColor :
		public CDocumentMenuCommandMLImpl<CSetCustomColor, DEFAULTCOLOR_CUSTOM, DEFAULTCOLOR_DESC, NULL, 0>
	{
	public:
		void Init(IDocument* a_pDocument, IDocumentLayeredImage* a_pDLI, IEnumUnknowns* a_pItems, IDesignerView* a_pView)
		{
			m_pItems = a_pItems;
			m_pDocument = a_pDocument;
			m_pDLI = a_pDLI;
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				TPixelChannel dummy;
				*a_peState = GetCurrentColor(m_pDocument, m_pDLI, m_pItems, NULL, &dummy) ? EMCSNormal : EMCSDisabled;
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			if (m_pView)
				m_pView->DeactivateAll(FALSE);

			TPixelChannel tColor = {0, 0, 0, 0};
			if (!GetCurrentColor(m_pDocument, m_pDLI, m_pItems, NULL, &tColor))
				return S_FALSE;

			CComPtr<IColorWindow> pColorWindow;
			RWCoCreateInstance(pColorWindow, __uuidof(ColorWindow));
			if (pColorWindow == NULL)
				return E_FAIL;
			TColor tFloatColor = {CGammaTables::FromSRGB(tColor.bR), CGammaTables::FromSRGB(tColor.bG), CGammaTables::FromSRGB(tColor.bB), tColor.bA/255.0f};
			if (S_OK != pColorWindow->DoModal(a_hParent, a_tLocaleID, &tFloatColor, TRUE))
				return S_FALSE;

			tColor.bR = CGammaTables::ToSRGB(tFloatColor.fR);
			tColor.bG = CGammaTables::ToSRGB(tFloatColor.fG);
			tColor.bB = CGammaTables::ToSRGB(tFloatColor.fB);
			tColor.bA = tFloatColor.fA*255.0f + 0.5f;
			return SetDefaultColor(m_pDocument, m_pDLI, m_pItems, tColor);
		}

	private:
		CComPtr<IEnumUnknowns> m_pItems;
		CComPtr<IDocument> m_pDocument;
		CComPtr<IDocumentLayeredImage> m_pDLI;
		CComPtr<IDesignerView> m_pView;
	};

private:
	static bool GetCurrentColor(IDocument* pDocument, IDocumentLayeredImage* pDLI, IEnumUnknowns* pItems, bool* pMatching, TPixelChannel* pDst)
	{
		bool bFound = false;
		ULONG nItems = 0;
		pItems->Size(&nItems);
		for (ULONG i = 0; i < nItems; ++i)
		{
			CComPtr<IComparable> pLayer;
			pItems->Get(i, &pLayer);
			if (S_OK == pDLI->IsLayer(pLayer, NULL, NULL, NULL))
			{
				CComQIPtr<ISubDocumentID> pSDID(pLayer);
				CComPtr<IDocument> pSubDoc;
				if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
				CComPtr<IDocumentEditableImage> pDEI;
				if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
				if (pDEI)
				{
					TPixelChannel tOne;
					if (SUCCEEDED(pDEI->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tOne))))
					{
						if (bFound)
						{
							if (pDst->n != tOne.n)
							{
								*pMatching = false;
								return true;
							}
						}
						else
						{
							*pDst = tOne;
							if (pMatching == NULL)
								return true;
							bFound = true;
							*pMatching = true;
						}
					}
				}
			}
		}
		return bFound;
	}
	static HRESULT SetDefaultColor(IDocument* pDocument, IDocumentLayeredImage* pDLI, IEnumUnknowns* pItems, TPixelChannel m_tColor)
	{
		try
		{
			CWriteLock<IDocument> cLock(pDocument);
			CUndoBlock cUndo(pDocument, CMultiLanguageString::GetAuto(DEFAULTCOLOR_NAME));

			ULONG nItems = 0;
			pItems->Size(&nItems);
			for (ULONG i = 0; i < nItems; ++i)
			{
				CComPtr<IComparable> pLayer;
				pItems->Get(i, &pLayer);
				if (S_OK == pDLI->IsLayer(pLayer, NULL, NULL, NULL))
				{
					CComQIPtr<ISubDocumentID> pSDID(pLayer);
					CComPtr<IDocument> pSubDoc;
					if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
					CComPtr<IDocumentEditableImage> pDEI;
					if (pSubDoc) pSubDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
					EImageChannelID eID = EICIRGBA;
					if (pDEI) pDEI->ChannelsSet(1, &eID, &m_tColor);
				}
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsLayerDefault, CMenuCommandsLayerDefault)
