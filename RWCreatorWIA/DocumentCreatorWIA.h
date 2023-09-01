// DocumentCreatorWIA.h : Declaration of the CDocumentCreatorWIA

#pragma once
#include "resource.h"       // main symbols
#include <RWConceptDesignerExtension.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#include <wia.h>

extern __declspec(selectany) IID const g_aSupported[] =
{
	__uuidof(IDocumentImage),
};

extern __declspec(selectany) wchar_t const DESWIZ_IMAGE_CAT[] = L"[0409]Images[0405]Obrázky";
extern __declspec(selectany) wchar_t const DESWIZ_CAPIMG_NAME[] = L"[0409]Capture image[0405]Zaznamenat obrázek";
extern __declspec(selectany) wchar_t const DESWIZ_CAPIMG_DESC[] = L"[0409]Acquire image from Windows Image Acquisition (WIA) compatible scanner or webcam.[0405]Poøídit obrázek pomocí skeneru nebo web-kamery kompatibilní se standardem Windows Image Acquisition (WIA).";

// {54FF5EE0-185B-4C58-8553-F72EBFE4B987}
extern __declspec(selectany) GUID const CLSID_DocumentCreatorWIA = {0x54ff5ee0, 0x185b, 0x4c58, {0x85, 0x53, 0xf7, 0x2e, 0xbf, 0xe4, 0xb9, 0x87}};


// CDocumentCreatorWIA

class ATL_NO_VTABLE CDocumentCreatorWIA : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentCreatorWIA, &CLSID_DocumentCreatorWIA>,
	public CDesignerWizardImpl<DESWIZ_CAPIMG_NAME, DESWIZ_CAPIMG_DESC, 0, DESWIZ_IMAGE_CAT, IDocumentFactoryRasterImage, 63>
{
public:
	CDocumentCreatorWIA()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentCreatorWIA)

BEGIN_CATEGORY_MAP(CDocumentCreatorWIA)
	IMPLEMENTED_CATEGORY(CATID_DesignerWizard)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentCreatorWIA)
	COM_INTERFACE_ENTRY(IDesignerWizard)
END_COM_MAP()


	// IDesignerWizard methods
public:
	STDMETHOD(State)(BOOLEAN* a_pEnableDocName, ILocalizedString** a_ppButtonText);
	STDMETHOD(Config)(IConfig** a_ppConfig);
	STDMETHOD(Activate)(RWHWND a_hParentWnd, LCID a_tLocaleID, IConfig* a_pConfig, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase);
	STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);

private:
	class ATL_NO_VTABLE CDataCallback : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IWiaDataCallback,
		public IStorageFilter
	{
	public:
		CDataCallback() : m_pData(NULL), m_nLength(0), m_nAllocated(0)
		{
		}
		~CDataCallback()
		{
			delete[] m_pData;
		}

	BEGIN_COM_MAP(CDataCallback)
		COM_INTERFACE_ENTRY(IWiaDataCallback)
		COM_INTERFACE_ENTRY(IStorageFilter)
	END_COM_MAP()

		// IWiaDataCallback interface
	public:
		STDMETHOD(BandedDataCallback)(LONG a_lReason, LONG a_lStatus, LONG a_lPercentComplete, LONG a_lOffset, LONG a_lLength, LONG a_lReserved, LONG a_lResLength, PBYTE a_pbBuffer);

		// IStorageFilter methods
	public:
		STDMETHOD(ToText)(IStorageFilter* a_pRoot, BSTR* a_pbstrFilter);
		STDMETHOD(SubFilterGet)(BSTR a_bstrRelativeLocation, IStorageFilter** a_ppFilter);
		STDMETHOD(SrcOpen)(IDataSrcDirect** a_ppSrc) { return E_NOTIMPL; }
		STDMETHOD(DstOpen)(IDataDstStream** a_ppDst) { return E_NOTIMPL; }

		// internal methods
	public:
		BYTE const* GetData() const
		{
			return m_pData;
		}
		ULONG GetLength() const
		{
			return m_nLength+sizeof(BITMAPFILEHEADER);
		}

	private:
		BYTE* m_pData;
		ULONG m_nLength;
		ULONG m_nAllocated;
	};

	struct SThreadInfo
	{
		IWiaDataCallback* pCallback;
		HWND hWnd;
		HRESULT hRes;
	};

private:
	static DWORD WINAPI ThreadProc(LPVOID a_pParam);
	static HRESULT STADocumentCreate(HWND a_hWnd, IWiaDataCallback* a_pDataCallback);
};

OBJECT_ENTRY_AUTO(CLSID_DocumentCreatorWIA, CDocumentCreatorWIA)
