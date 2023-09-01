
#include "stdafx.h"
/*
#include "RWOperationImageRaster.h"
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>


static wchar_t const CFGID_CP_TOOLID[] = L"ToolID";
static wchar_t const CFGID_CP_TOOLSTATE[] = L"State";
static wchar_t const CFGID_CP_BOUNDS[] = L"Bounds";
static wchar_t const CFGID_CP_OPERATION[] = L"Operation";


// CDocumentOperationClipPath

class ATL_NO_VTABLE CDocumentOperationClipPath :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationClipPath>,
	public IDocumentOperation,
	public CTrivialRasterImageFilter,
	public CConfigDescriptorImpl
{
public:
	CDocumentOperationClipPath()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationClipPath)

BEGIN_CATEGORY_MAP(CDocumentOperationClipPath)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagMetaOp)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationClipPath)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName)
			return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Image - Clipping Path[0405]Image - ořezávací cesta");
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemIns1ofN(CComBSTR(CFGID_CP_TOOLID), CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), NULL);
			pCfgInit->ItemOp
			pCfgInit->ItemInsSimple(CComBSTR(CFGID_CP_TOOLSTATE), CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), CConfigValue(0.5f), NULL, CConfigValue(-5.0f), CConfigValue(5.0f), CConfigValue(0.05f), 0, NULL);
			a_pManager->InsertIntoConfigAs(a_pManager, pCfgInit, CComBSTR(CFGID_CP_OPERATION), CMultiLanguageString::GetAuto(L"[0409]Operation[0405]Operace"), CMultiLanguageString::GetAuto(L"[0409]Operation to be modified.[0405]Operace, které bude upravena."), 0, NULL);

			pCfgInit->Finalize(NULL);
			//CConfigCustomGUI<&CLSID_DocumentOperationRasterImageFade, CConfigGUIFadeDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			static IID const aFts[] = {__uuidof(IDocumentRasterImage)};
			return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
	}

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);

	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
	{
		try
		{
			CComPtr<IRasterImageFilter> pSubRIF;
			RWCoCreateInstance(pSubRIF, cVal);
			if (pSubRIF == NULL)
				return S_FALSE;

			CComBSTR bstr(CFGID_CP_OPERATION);
			CConfigValue cVal;
			a_pConfig->ItemValueGet(bstr, &cVal);

			CComPtr<IConfig> pCfg;
			a_pConfig->SubConfigGet(bstr, &pCfg);
			return pSubRIF->Transform(pCfg, a_pCanvas, a_pContentTransform);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(AdjustDirtyRect)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
	{
		try
		{
			CComPtr<IRasterImageFilter> pSubRIF;
			RWCoCreateInstance(pSubRIF, cVal);
			if (pSubRIF == NULL)
				return S_FALSE;

			CComBSTR bstr(CFGID_CP_OPERATION);
			CConfigValue cVal;
			a_pConfig->ItemValueGet(bstr, &cVal);
			CComBSTR bstr(CFGID_CP_BOUNDS);
			CConfigValue cBounds;
			a_pConfig->ItemValueGet(bstr, &cVal);
			if (cBounds[0] < cBounds[2] && cBounds[1] < cBounds[3])
			{
				if (a_pRect->tTL.nX < cBounds[0]) a_pRect->tTL.nX = cBounds[0];
				if (a_pRect->tTL.nY < cBounds[1]) a_pRect->tTL.nY = cBounds[1];
				if (a_pRect->tBR.nX < cBounds[2]) a_pRect->tBR.nX = cBounds[2];
				if (a_pRect->tBR.nY < cBounds[3]) a_pRect->tBR.nY = cBounds[3];
			}

			CComPtr<IConfig> pCfg;
			a_pConfig->SubConfigGet(bstr, &pCfg);
			return pSubRIF->AdjustDirtyRect(pCfg, a_pCanvas, a_pRect);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(NeededToCompute)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect);
	STDMETHOD(Process)(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix) { return E_NOTIMPL; }
};

// {2956E1B6-7082-4753-8ABA-1F182D9DA778}
GUID const CLSID_DocumentOperationClipPath = {0x2956e1b6, 0x7082, 0x4753, {0x8a, 0xba, 0x1f, 0x18, 0x2d, 0x9d, 0xa7, 0x78}};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationClipPath), CDocumentOperationClipPath)
*/