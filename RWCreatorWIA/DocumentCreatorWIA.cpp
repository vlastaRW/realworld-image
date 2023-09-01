// DocumentCreatorWIA.cpp : Implementation of CDocumentCreatorWIA

#include "stdafx.h"
#include "DocumentCreatorWIA.h"

#include <IconRenderer.h>
#include "WIAUtils.h"
#include <sti.h>


// CDocumentCreatorWIA

STDMETHODIMP CDocumentCreatorWIA::State(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText)
{
	try
	{
		if (a_pEnableDocName)
			*a_pEnableDocName = TRUE;
		if (a_ppButtonText)
		{
			*a_ppButtonText = NULL;
			*a_ppButtonText = new CMultiLanguageString(L"[0409]Create[0405]Vytvoøit");
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

//static OLECHAR const CFGID_U3DROOTMODULE[] = L"Root";

class ATL_NO_VTABLE CConfigGUICaptureImage : 
	public CCustomConfigResourcelessWndImpl<CConfigGUICaptureImage>
{
public:
	BEGIN_DIALOG_EX(0, 0, 156, 20, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	enum
	{
		IDC_TITLE = 100,
		IDC_DOCUMENTATION,
	};
	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Capture image[0405]Zachytit obrázek"), IDC_TITLE, 0, 0, 156, 20, WS_VISIBLE|SS_ENDELLIPSIS, 0)
		CONTROL_LTEXT(_T("[0409]Use this wizard to capture an image from a webcam conected to your computer or from a WIA compatible scanner.[0405]Tento prùvodce umožòuje zachytit obrázek z webkamery pøipojené k tomuto poèítaèi nebo z WIA-kompatiblilního skeneru."), IDC_STATIC, 0, 24, 156, 40, WS_VISIBLE, 0)
		CONTROL_LTEXT(_T("[0409]More information[0405]Více informací"), IDC_DOCUMENTATION, 0, 68, 75, 8, WS_VISIBLE | WS_TABSTOP, 0)
		//CONTROL_CONTROL(_T(""), IDC_SEPLINE, WC_STATIC, SS_ETCHEDHORZ | WS_GROUP | WS_VISIBLE, 80, 41, 139, 1, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUICaptureImage)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUICaptureImage>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUICaptureImage)
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
		m_wndDocumentation.SetHyperLink(_T("http://www.rw-designer.com/capture-image-wizard"));
		return 1;  // Let the system set the focus
	}

private:
	CFont m_font;
	CHyperLink m_wndDocumentation;
};

STDMETHODIMP CDocumentCreatorWIA::Config(IConfig** a_ppConfig)
{
	try
	{
		*a_ppConfig = NULL;
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));

		//pCfg->ItemInsSimple(CComBSTR(CFGID_U3DROOTMODULE), CMultiLanguageString::GetAuto(L"[0409]Root module[0405]Koøenový modul"), CMultiLanguageString::GetAuto(L"[0409]Parameterizable[0405]Parametrizovatelný"), CConfigValue(0L), NULL, 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentCreatorWIA, CConfigGUICaptureImage>::FinalizeConfig(pCfg);

		*a_ppConfig = pCfg.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppConfig ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentCreatorWIA::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	if (a_phIcon == NULL)
		return E_POINTER;
	try
	{
		static IRPolyPoint const glass[] =
		{
			{60, 159}, {32, 212}, {224, 212}, {196, 159},
		};
		static IRPathPoint const bed[] =
		{
			{200, 148, 6, 0, 0, 0},
			{217, 158, 17, 24, -5.45301, -7.69836},
			{246, 200, 5.23303, 7.84955, -14, -21},
			{256, 220, 0, 10, 0, -7},
			{239, 232, 0, 0, 5, 0},
			{17, 232, -5, 0, 0, 0},
			{0, 220, 0, -7, 0, 10},
			{10, 200, 14, -21, -5.23303, 7.84955},
			{39, 158, 5.45301, -7.69836, -17, 24},
			{56, 148, 0, 0, -6, 0},
		};
		static IRPathPoint const front[] =
		{
			{236, 256, 19, 1, 0, 0},
			{256, 221, 0, 0, -2, 19},
			{128, 199, 0, 0, 0, 0},
			{0, 221, 2, 19, 0, 0},
			{20, 256, 0, 0, -19, 1},
		};
		static IRPolyPoint const press[] =
		{
			{37, 19}, {60, 136}, {196, 136}, {219, 19},
		};
		static IRPathPoint const lid[] =
		{
			{200, 151, 5, 0, 0, 0},
			{211, 140, 0, 0, -2.25268, 9.2155},
			{235, 40, 1.0243, -4.35326, 0, 0},
			{240, 14, 0, -12, 0, 6},
			{221, 0, 0, 0, 10, 0},
			{35, 0, -10, 0, 0, 0},
			{16, 14, 0, 6, 0, -12},
			{21, 40, 0, 0, -1.0243, -4.35326},
			{45, 140, 2.25268, 9.2155, 0, 0},
			{56, 151, 0, 0, -5, 0},
		};
		static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};

		CComPtr<IStockIcons> pSI;
		RWCoCreateInstance(pSI, __uuidof(StockIcons));
		CIconRendererReceiver cRenderer(a_nSize);
		IROutlinedFill plasticMat(pSI->GetMaterial(ESMAltBackground, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		IROutlinedFill paperMat(pSI->GetMaterial(ESMBackground, true), pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		IRFill glassFill(0xffd5ebf1);
		IROutlinedFill glassMat(&glassFill, pSI->GetMaterial(ESMContrast), 1.0f/80.0f, 0.4f);
		cRenderer(&canvas, itemsof(front), front, &plasticMat);
		cRenderer(&canvas, itemsof(bed), bed, &plasticMat);
		cRenderer(&canvas, itemsof(glass), glass, &glassMat);
		cRenderer(&canvas, itemsof(lid), lid, &plasticMat);
		cRenderer(&canvas, itemsof(press), press, &paperMat);
		*a_phIcon = cRenderer.get();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

//STDMETHODIMP CDocumentCreatorWIA::Size(ULONG* a_pnCount)
//{
//	try
//	{
//		// WIA works on Windows XP, Windows Me and higher
//		static ULONG nCreators = 2;
//		if (nCreators > 1)
//		{
//			OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
//			GetVersionEx(&tVersion);
//			nCreators = 
//				(tVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && (tVersion.dwMajorVersion > 4 || (tVersion.dwMajorVersion == 4 && tVersion.dwMinorVersion >= 90))) ||
//				(tVersion.dwPlatformId == VER_PLATFORM_WIN32_NT && (tVersion.dwMajorVersion > 5 || (tVersion.dwMajorVersion == 5 && tVersion.dwMinorVersion >= 1)))
//				? 1 : 0;
//		}
//		*a_pnCount = nCreators;
//
//		return S_OK;
//	}
//	catch (...)
//	{
//		return a_pnCount == NULL ? E_POINTER : E_UNEXPECTED;
//	}
//}

#include "../RWCodecImageBMP/RWImageCodecBMP.h"

STDMETHODIMP CDocumentCreatorWIA::Activate(RWHWND a_hParentWnd, LCID UNREF(a_tLocaleID), IConfig* UNREF(a_pConfig), IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase)
{
	try
	{
		CComObject<CDataCallback>* pDataCallback = NULL;
		CComObject<CDataCallback>::CreateInstance(&pDataCallback);
		CComPtr<IStorageFilter> pDataSrc = pDataCallback;

		// stupid WIA does not work in MTA
		SThreadInfo tThreadInfo;
		tThreadInfo.hRes = E_FAIL;
		tThreadInfo.hWnd = a_hParentWnd;
		tThreadInfo.pCallback = pDataCallback;
		DWORD dwThID;
		HANDLE hThread = CreateThread(NULL, 0, ThreadProc, &tThreadInfo, 0, &dwThID);
		AtlWaitWithMessageLoop(hThread);
		CloseHandle(hThread);

		if (tThreadInfo.hRes != S_OK)
		{
			return tThreadInfo.hRes;
		}

		CComPtr<IDocumentDecoder> pBMPDecoder;
		RWCoCreateInstance(pBMPDecoder, __uuidof(DocumentDecoderBMP));
		CComQIPtr<IDocumentBuilder> pBuilder(a_pBuilder);
		return pBMPDecoder->Parse(pDataCallback->GetLength(), pDataCallback->GetData(), pDataCallback, 1, &(pBuilder.p), a_bstrPrefix, a_pBase, NULL, NULL, NULL);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

DWORD WINAPI CDocumentCreatorWIA::ThreadProc(LPVOID a_pParam)
{
	SThreadInfo * const p = reinterpret_cast<SThreadInfo * const>(a_pParam);
	p->hRes = E_FAIL;
	CoInitialize(NULL);
	try
	{
		p->hRes = STADocumentCreate(p->hWnd, p->pCallback);
	}
	catch (...) {}
	CoUninitialize();
	return 0;
}

HRESULT CDocumentCreatorWIA::STADocumentCreate(HWND a_hWnd, IWiaDataCallback* a_pDataCallback)
{
	HRESULT hRes;

	CComPtr<IWiaDevMgr> pWiaDevMgr;
	pWiaDevMgr.CoCreateInstance(__uuidof(WiaDevMgr));

	// Display the device selection common dialog
	CComPtr<IWiaItem> pItemRoot;
	hRes = pWiaDevMgr->SelectDeviceDlg(a_hWnd, StiDeviceTypeDefault, 0, NULL, &pItemRoot);
    if (FAILED(hRes) || hRes == S_FALSE)
    {
        return hRes;
    }

	// Display the image selection common dialog 
	CComPtrArray<IWiaItem> ppIWiaItem;
	hRes = pItemRoot->DeviceDlg(a_hWnd, WIA_DEVICE_DIALOG_SINGLE_IMAGE, WIA_INTENT_NONE, &ppIWiaItem.Count(), &ppIWiaItem);
	if (FAILED(hRes) || hRes == S_FALSE)
	{
		return hRes;
	}
	if (ppIWiaItem.Count() < 1)
	{
		return S_FALSE;
	}

	CComQIPtr<IWiaPropertyStorage> pWiaPropertyStorage(ppIWiaItem[0]);
	if (pWiaPropertyStorage == NULL)
	{
		return E_NOINTERFACE;
	}

	CComQIPtr<IWiaDataTransfer> pIWiaDataTransfer(ppIWiaItem[0]);
	if (pIWiaDataTransfer == NULL)
	{
		return E_NOINTERFACE;
	}

	// Set the transfer type
	PROPSPEC specTymed;
	specTymed.ulKind = PRSPEC_PROPID;
	specTymed.propid = WIA_IPA_TYMED;

	PROPVARIANT varTymed;
	varTymed.vt = VT_I4;
	varTymed.lVal = TYMED_CALLBACK;

	hRes = pWiaPropertyStorage->WriteMultiple(1, &specTymed, &varTymed, WIA_IPA_FIRST);
	PropVariantClear(&varTymed);
	if (FAILED(hRes))
	{
		return hRes;
	}

	// Set the transfer format
	PROPSPEC specFormat;
	specFormat.ulKind = PRSPEC_PROPID;
	specFormat.propid = WIA_IPA_FORMAT;

	PROPVARIANT varFormat;
	varFormat.vt = VT_CLSID;
	varFormat.puuid = reinterpret_cast<CLSID*>(CoTaskMemAlloc(sizeof(CLSID)));
	*varFormat.puuid = WiaImgFmt_MEMORYBMP;

	hRes = pWiaPropertyStorage->WriteMultiple(1, &specFormat, &varFormat, WIA_IPA_FIRST);
	PropVariantClear(&varFormat);
	if (FAILED(hRes))
	{
		return hRes;
	}

	// Read the transfer buffer size from the device, default to 64K
	PROPSPEC specBufferSize;
	specBufferSize.ulKind = PRSPEC_PROPID;
	specBufferSize.propid = WIA_IPA_BUFFER_SIZE;

	LONG nBufferSize = 64 * 1024;
	hRes = ReadPropertyLong(pWiaPropertyStorage, &specBufferSize, &nBufferSize);
	if (FAILED(hRes))
	{
		nBufferSize = 64 * 1024;
	}

	// Choose double buffered transfer for better performance
	WIA_DATA_TRANSFER_INFO WiaDataTransferInfo = { 0 };
	WiaDataTransferInfo.ulSize        = sizeof(WIA_DATA_TRANSFER_INFO);
	WiaDataTransferInfo.ulBufferSize  = 2 * nBufferSize;
	WiaDataTransferInfo.bDoubleBuffer = TRUE;

	// Start the transfer
	hRes = pIWiaDataTransfer->idtGetBandedData(&WiaDataTransferInfo, a_pDataCallback);
	if (FAILED(hRes) || hRes == S_FALSE)
	{
		return hRes;
	}

	return S_OK;
}


// CDocumentCreatorWIA::CDataCallback

STDMETHODIMP CDocumentCreatorWIA::CDataCallback::BandedDataCallback(LONG a_lReason, LONG UNREF(a_lStatus), LONG UNREF(a_lPercentComplete), LONG a_lOffset, LONG a_lLength, LONG UNREF(a_lReserved), LONG UNREF(a_lResLength), PBYTE a_pbBuffer)
{
	try
	{
		switch (a_lReason)
		{
		case IT_MSG_DATA_HEADER:
			{
				PWIA_DATA_CALLBACK_HEADER pHeader = reinterpret_cast<PWIA_DATA_CALLBACK_HEADER>(a_pbBuffer);

				if (pHeader->guidFormatID != WiaImgFmt_MEMORYBMP)
					return E_FAIL; // unsupported format

				if (pHeader->lBufferSize != 0)
				{
					m_pData = new BYTE[sizeof(BITMAPFILEHEADER) + pHeader->lBufferSize];
					m_nAllocated = pHeader->lBufferSize;
				}
			}
			break;

		case IT_MSG_DATA:
			if (m_pData == NULL && a_lOffset == 0)
			{
				LONG nSize = BitmapUtil::GetBitmapSize(a_pbBuffer);
				m_pData = new BYTE[sizeof(BITMAPFILEHEADER)+nSize];
				m_nAllocated = nSize;
			}

			if (static_cast<ULONG>(a_lLength + a_lOffset) > m_nAllocated) // this should not be needed
			{
				LONG nNewSize = max(static_cast<ULONG>(a_lLength + a_lOffset), m_nAllocated+(m_nAllocated>>1)+256);
				BYTE* pData = new BYTE[sizeof(BITMAPFILEHEADER)+nNewSize];
				m_nAllocated = nNewSize;
				memcpy(pData, m_pData, sizeof(BITMAPFILEHEADER)+m_nLength);
				delete[] m_pData;
				m_pData = pData;
			}

			memcpy(m_pData + sizeof(BITMAPFILEHEADER) + a_lOffset, a_pbBuffer, a_lLength);
			if (m_nLength < static_cast<ULONG>(a_lOffset+a_lLength))
				m_nLength = static_cast<ULONG>(a_lOffset+a_lLength);
			break;

		case IT_MSG_STATUS:
			break;

		case IT_MSG_TERMINATION:
			if (m_pData != NULL)
			{
				// Some scroll-fed scanners may return 0 as the bitmap height
				// In this case, calculate the image height and modify the header

				BitmapUtil::FixBitmapHeight(m_pData+sizeof(BITMAPFILEHEADER), m_nLength, TRUE);
				BitmapUtil::FillBitmapFileHeader(m_pData+sizeof(BITMAPFILEHEADER), reinterpret_cast<BITMAPFILEHEADER*>(m_pData));
			}
			break;

		case IT_MSG_NEW_PAGE:
			if (m_pData != NULL)
			{
				return E_FAIL; // only one page allowed
			}
			break;
		}

		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentCreatorWIA::CDataCallback::ToText(IStorageFilter* UNREF(a_pRoot), BSTR* a_pbstrFilter)
{
	try
	{
		*a_pbstrFilter = SysAllocString(L"wia.bmp");
		return S_OK;
	}
	catch (...)
	{
		return a_pbstrFilter ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentCreatorWIA::CDataCallback::SubFilterGet(BSTR UNREF(a_bstrRelativeLocation), IStorageFilter** UNREF(a_ppFilter))
{
	return E_NOTIMPL;
}

