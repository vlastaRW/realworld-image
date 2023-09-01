// ImageFromClipboard.cpp : Implementation of CImageFromClipboard

#include "stdafx.h"
#include "ImageFromClipboard.h"
#include "DocumentLayeredImage.h"

#include <MultiLanguageString.h>
#include <ImageClipboardUtils.h>


// CImageFromClipboard

STDMETHODIMP CImageFromClipboard::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvořit");
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <ConfigCustomGUIImpl.h>
#include <XPGUI.h>

//static OLECHAR const CFGID_U3DPARAMETERS[] = L"Parameters";
//static OLECHAR const CFGID_U3DROOTMODULE[] = L"Root";

class ATL_NO_VTABLE CConfigGUIImageFromClipboard : 
	public CCustomConfigResourcelessWndImpl<CConfigGUIImageFromClipboard>
{
public:
	CConfigGUIImageFromClipboard() : m_hWndNextViewer(NULL)
	{
		m_tSize.nX = m_tSize.nY = 0;
	}

	BEGIN_DIALOG_EX(0, 0, 156, 20/*89*/, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
		IDC_DOCUMENTATION,
		IDC_PREVIEW,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Paste image[0405]Vložit obrázek"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]This wizard opens a raster image from the clipboard. Images can be placed on clipboard using the Copy or Copy Image command in graphic editors. Content of any window can be capured by pressing ALT+PrtScn keys while the window is active.[0405]Tento průvodce otevře obrázek se schránky. Obrázky lze umístit do schránky pomocí příkazu Kopírovat nebo Kopírovat obrázek v grafických editorech. Obsah jakéhokoli okna lze umístit do schránky stiskem kláves ALT+PrtScn zatímco je okno aktivní."), IDC_STATIC, 0, 24, 156, 48, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]More information[0405]Více informací"), IDC_DOCUMENTATION, 0, 76, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_LTEXT(_T("[0409]No image[0405]Zadny obrazek"), IDC_PREVIEW, 0, 88, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CONTROL(_T(""), IDC_PREVIEW, WC_STATIC, SS_OWNERDRAW | WS_VISIBLE, 0, 91, 156, 104, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIImageFromClipboard)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIImageFromClipboard>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCBChain)
		MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipBoard)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawPreview)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIImageFromClipboard)
		//CONFIGITEM_CHECKBOX(IDC_PARAMETRIC, CFGID_U3DPARAMETERS)
		//CONFIGITEM_CONTEXTHELP(IDC_ROOTFCC, CFGID_U3DROOTMODULE)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LOGFONT lf = {0};
		::GetObject(GetFont(), sizeof(lf), &lf);
		lf.lfWeight = FW_BOLD;
		lf.lfUnderline = 1;
		lf.lfHeight *= 2;
		GetDlgItem(IDC_TITLE).SetFont(m_font.CreateFontIndirect(&lf));

		m_wndDocumentation.SubclassWindow(GetDlgItem(IDC_DOCUMENTATION));
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/image-from-clipboard-wizard"));

		m_hWndNextViewer = SetClipboardViewer();

		return 1;  // Let the system set the focus
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		ChangeClipboardChain(m_hWndNextViewer);
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnChangeCBChain(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		if (reinterpret_cast<HWND>(a_wParam) == m_hWndNextViewer)
		{
			m_hWndNextViewer = reinterpret_cast<HWND>(a_lParam);
			return 0;
		}
		if (m_hWndNextViewer != NULL)
		{
			::SendMessage(m_hWndNextViewer, WM_CHANGECBCHAIN, a_wParam, a_lParam);
		}
		return 0;
	}
	LRESULT OnDrawClipBoard(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& UNREF(a_bHandled))
	{
		m_bCheckClipboard = true;
		GetDlgItem(IDC_PREVIEW).Invalidate(FALSE);
		if (m_hWndNextViewer != NULL)
		{
			::SendMessage(m_hWndNextViewer, WM_DRAWCLIPBOARD, a_wParam, a_lParam);
		}
		return 0;
	}
	LRESULT OnDrawPreview(UINT UNREF(a_uMsg), WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (a_wParam != IDC_PREVIEW)
		{
			a_bHandled = FALSE;
			return 0;
		}
		CWindow wnd = GetDlgItem(IDC_PREVIEW);
		RECT rcWnd;
		wnd.GetClientRect(&rcWnd);
		LPDRAWITEMSTRUCT pDrawItem = reinterpret_cast<LPDRAWITEMSTRUCT>(a_lParam);
		RECT rc = {0, 0, rcWnd.right, 1};
		FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
		rc.top = rcWnd.bottom-1; rc.bottom = rcWnd.bottom;
		FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
		rc.top = 1; --rc.bottom; rc.right = 1;
		FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
		rc.left = rcWnd.right-1; rc.right = rcWnd.right;
		FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DSHADOW+1));
		++rcWnd.left;
		++rcWnd.top;
		--rcWnd.right;
		--rcWnd.bottom;
		rc = rcWnd;
		++rc.left;
		++rc.top;
		--rc.right;
		--rc.bottom;
		if (m_bCheckClipboard)
		{
			m_bCheckClipboard = false;
			m_pBuffer.Free();
			m_tSize.nX = m_tSize.nY = 0;
			try
			{
				GetClipboardImage(m_hWnd, m_tSize, m_pBuffer);
				if (m_pBuffer.m_p)
				{
					COLORREF clrBG = GetSysColor(COLOR_3DFACE);
					ULONG nR = GetRValue(clrBG);
					ULONG nG = GetGValue(clrBG);
					ULONG nB = GetBValue(clrBG);
					TPixelChannel* const pEnd = m_pBuffer.m_p+m_tSize.nX*m_tSize.nY;
					for (TPixelChannel* p = m_pBuffer.m_p; p < pEnd; ++p)
					{
						if (p->bA != 255)
						{
							p->bB = (ULONG(p->bA)*p->bB+(255-p->bA)*nB)/255;
							p->bG = (ULONG(p->bA)*p->bG+(255-p->bA)*nG)/255;
							p->bR = (ULONG(p->bA)*p->bR+(255-p->bA)*nR)/255;
							p->bA = 255;
						}
					}
				}
			}
			catch (...)
			{
				m_pBuffer.Free();
			}
		}
		if (m_pBuffer && m_tSize.nX && m_tSize.nY)
		{
			LONG nBoundsX = rc.right-rc.left;
			LONG nBoundsY = rc.bottom-rc.top;
			LONG nDstX = nBoundsX;
			LONG nDstY = nBoundsY;
			if (m_tSize.nX*nBoundsY > m_tSize.nY*nBoundsX)
			{
				nDstY = m_tSize.nY*nDstX/m_tSize.nX;
			}
			else
			{
				nDstX = m_tSize.nX*nDstY/m_tSize.nY;
			}
			rc = rcWnd;
			rc.bottom = rcWnd.top+1+((nBoundsY-nDstY)>>1); 
			FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DFACE+1));
			rc.top = rc.bottom+nDstY; rc.bottom = rcWnd.bottom;
			FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DFACE+1));
			rc.bottom = rc.top; rc.top = rcWnd.top+1+((nBoundsY-nDstY)>>1);
			rc.left = rcWnd.left; rc.right = rcWnd.left+1+((nBoundsX-nDstX)>>1);
			FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DFACE+1));
			rc.left = rc.right+nDstX; rc.right = rcWnd.right;
			FillRect(pDrawItem->hDC, &rc, (HBRUSH)(COLOR_3DFACE+1));
			BITMAPINFO tBMI;
			ZeroMemory(&tBMI, sizeof tBMI);
			tBMI.bmiHeader.biSize = sizeof tBMI.bmiHeader;
			tBMI.bmiHeader.biWidth = m_tSize.nX;
			tBMI.bmiHeader.biHeight = -m_tSize.nY;
			tBMI.bmiHeader.biPlanes = 1;
			tBMI.bmiHeader.biBitCount = 32;
			tBMI.bmiHeader.biCompression = BI_RGB;
			int iPrev = SetStretchBltMode(pDrawItem->hDC, STRETCH_HALFTONE);
			StretchDIBits(pDrawItem->hDC, 2+((nBoundsX-nDstX)>>1), 2+((nBoundsY-nDstY)>>1), nDstX, nDstY, 0, 0, m_tSize.nX, m_tSize.nY, m_pBuffer.m_p, &tBMI, DIB_RGB_COLORS, SRCCOPY);
			SetStretchBltMode(pDrawItem->hDC, iPrev);
		}
		else
		{
			FillRect(pDrawItem->hDC, &rcWnd, (HBRUSH)(COLOR_3DFACE+1));
			CComBSTR bstrNoImg;
			CMultiLanguageString::GetLocalized(L"[0409]No image in clipboard.[0405]Ve schránce není obrázek.", m_tLocaleID, &bstrNoImg);
			COLORREF clrPrev = SetBkColor(pDrawItem->hDC, GetSysColor(COLOR_3DFACE));
			DrawText(pDrawItem->hDC, COLE2CT(bstrNoImg), -1, &rcWnd, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
			SetBkColor(pDrawItem->hDC, clrPrev);
		}
		return 1;
	}

private:
	CFont m_font;
	CHyperLink m_wndDocumentation;
	HWND m_hWndNextViewer;
	bool m_bCheckClipboard;
	TImageSize m_tSize;
	CAutoVectorPtr<TPixelChannel> m_pBuffer;
};

STDMETHODIMP CImageFromClipboard::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		//pCfg->ItemInsSimple(CComBSTR(CFGID_U3DPARAMETERS), CMultiLanguageString::GetAuto(L"[0409]Parameterizable[0405]Parametrizovatelný"), CMultiLanguageString::GetAuto(L"[0409]Allows defining parameters and using expressions instead of constants for higher re-usability of 3D objects.[0405]Umožní definovat parametry a používat výrazy místo konstant pro opakované využití 3D modelu."), CConfigValue(true), NULL, 0, NULL);
		//pCfg->ItemInsSimple(CComBSTR(CFGID_U3DROOTMODULE), CMultiLanguageString::GetAuto(L"[0409]Root module[0405]Kořenový modul"), CMultiLanguageString::GetAuto(L"[0409]Parameterizable[0405]Parametrizovatelný"), CConfigValue(0L), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_ImageFromClipboard, CConfigGUIImageFromClipboard>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

#include <DocumentName.h>

static UINT GetLayeredImageClipboardFormat()
{
	static UINT eClipboardFormat = 0;
	if (eClipboardFormat == 0)
		eClipboardFormat = RegisterClipboardFormat(_T("RWLayers"));
	return eClipboardFormat;
}

STDMETHODIMP CImageFromClipboard::Activate(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		UINT eClipboardFormat = GetLayeredImageClipboardFormat();

		if (eClipboardFormat && a_hParentWnd && IsClipboardFormatAvailable(eClipboardFormat))
		{
			CClipboardHandler cClipboard(a_hParentWnd);

			HANDLE hMem = GetClipboardData(eClipboardFormat);
			if (hMem == NULL)
				return E_FAIL;

			BYTE const* pData = reinterpret_cast<BYTE const*>(GlobalLock(hMem));
			SIZE_T nSize = GlobalSize(hMem);

			CComObject<CDocumentName>* pName = NULL;
			CComObject<CDocumentName>::CreateInstance(&pName);
			CComPtr<IStorageFilter> pName2 = pName;
			pName->Init(L"pasted.rli");
			CComPtr<IInputManager> pIM;
			RWCoCreateInstance(pIM, __uuidof(InputManager));
			CComPtr<IEnumUnknownsInit> pBuilders;
			RWCoCreateInstance(pBuilders, __uuidof(EnumUnknowns));
			pBuilders->InsertMultiple(a_nBuilders, reinterpret_cast<IUnknown* const*>(a_apBuilders));
			HRESULT hRes = pIM->DocumentCreateDataEx(pBuilders, nSize, pData, pName2, a_bstrPrefix, a_pBase, NULL, NULL, NULL);
			GlobalUnlock(hMem);
			return hRes;
		}

		if (a_hParentWnd == NULL || !IsClipboardFormatAvailable(CF_BITMAP)) 
			return E_FAIL;

		for (ULONG i = 0; i < a_nBuilders; ++i)
		{
			CComPtr<IDocumentFactoryRasterImage> p;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryRasterImage), reinterpret_cast<void**>(&p));
			if (p)
			{
				TImageSize tSize = {1, 1};
				CAutoVectorPtr<TPixelChannel> pPixels;
				GetClipboardImage(a_hParentWnd, tSize, pPixels);

				TImageTile t = {EICIRGBA, {0, 0}, tSize, {1, tSize.nX}, tSize.nX*tSize.nY, pPixels};
				return p->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 0.0f, &t);
			}
		}

		return E_FAIL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CImageFromClipboard::CanActivate()
{
	UINT eClipboardFormat = GetLayeredImageClipboardFormat();
	return (eClipboardFormat && IsClipboardFormatAvailable(eClipboardFormat)) || IsClipboardFormatAvailable(CF_BITMAP) ? S_OK : S_FALSE;
}

STDMETHODIMP CImageFromClipboard::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);

		static IRPathPoint const board[] =
		{
			{39, 44, 0, -8.83656, 0, 0},
			{55, 28, 0, 0, -8.83656, 0},
			{201, 28, 8.83656, 0, 0, 0},
			{217, 44, 0, 0, 0, -8.83656},
			{217, 240, 0, 8.83656, 0, 0},
			{201, 256, 0, 0, 8.83656, 0},
			{55, 256, -8.83656, 0, 0, 0},
			{39, 240, 0, 0, 0, 8.83656},
		};
		static IRPathPoint const clip[] =
		{
			{144, 14, 0, 0, -0.984474, -7.89232},
			{168, 14, 4.41828, 0, 0, 0},
			{176, 22, 0, 0, 0, -4.41828},
			{176, 56, 0, 4.41828, 0, 0},
			{168, 64, 0, 0, 4.41828, 0},
			{88, 64, -4.41828, 0, 0, 0},
			{80, 56, 0, 0, 0, 4.41828},
			{80, 22, 0, -4.41828, 0, 0},
			{88, 14, 0, 0, -4.41828, 0},
			{112, 14, 0.984475, -7.89232, 0, 0},
			{128, 0, 8.15903, 0, -8.15902, 0},
		};
		static IRGridItem const gridBoardX[] = {{0, 39}, {0, 217}};
		static IRGridItem const gridBoardY[] = {{0, 28}, {0, 256}};
		static IRCanvas const canvasBoard = {39, 0, 217, 256, itemsof(gridBoardX), itemsof(gridBoardY), gridBoardX, gridBoardY};
		static IRCanvas const canvasClip = {39, 0, 217, 256, 0, 0, NULL, NULL};

		IROutlinedFill boardMat(pSI->GetMaterial(ESMScheme2Color2, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		cRenderer(&canvasBoard, itemsof(board), board, &boardMat, IRTarget(1, -1, 0));

		pSI->GetLayers(ESIPicture|ESILarge, cRenderer, IRTarget(0.9375f, 1, 0.2f));

		IROutlinedFill clipMat(pSI->GetMaterial(ESMScheme2Color1, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		cRenderer(&canvasClip, itemsof(clip), clip, &clipMat, IRTarget(1, -1, 0));

		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}
