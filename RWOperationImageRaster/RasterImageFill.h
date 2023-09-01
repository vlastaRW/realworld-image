// RasterImageFill.h : Declaration of the CRasterImageFill

#pragma once
#include "resource.h"       // main symbols

#include "RWOperationImageRaster.h"
#include <RWProcessingTags.h>
#include <RWDocumentImageRaster.h>


// CRasterImageFill

class ATL_NO_VTABLE CRasterImageFill :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CRasterImageFill, &CLSID_RasterImageFill>,
	public IDocumentOperation,
	public CConfigDescriptorImpl,
	public CTrivialRasterImageFilter,
	public ICanvasInteractingOperation
{
public:
	CRasterImageFill()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CRasterImageFill)

BEGIN_CATEGORY_MAP(CRasterImageFill)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
	IMPLEMENTED_CATEGORY(CATID_TagLayerStyle)
	IMPLEMENTED_CATEGORY(CATID_TagImageEmbellishment)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CRasterImageFill)
	COM_INTERFACE_ENTRY(IDocumentOperation)
	COM_INTERFACE_ENTRY(IConfigDescriptor)
	COM_INTERFACE_ENTRY(ICanvasInteractingOperation)
	COM_INTERFACE_ENTRY(IRasterImageFilter)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

	// IConfigDescriptor methods
public:
	STDMETHOD(Name)(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName);
	STDMETHOD(PreviewIconID)(IUnknown* a_pContext, IConfig* a_pConfig, GUID* a_pIconID);
	STDMETHOD(PreviewIcon)(IUnknown* a_pContext, IConfig* a_pConfig, ULONG a_nSize, HICON* a_phIcon);

	static HICON GetDefaultIcon(ULONG a_nSize);

	// IRasterImageFilter methods
public:
	STDMETHOD(Transform)(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform);

	// ICanvasInteractingOperation
public:
	STDMETHOD(CreateWrapper)(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY, ICanvasInteractingWrapper** a_ppWrapper)
	{
		if (a_ppWrapper == NULL)
			return E_POINTER;
		try
		{
			CComObject<CWrapper>* p = NULL;
			CComObject<CWrapper>::CreateInstance(&p);
			CComPtr<ICanvasInteractingWrapper> pNew = p;
			p->Init(a_nSizeX, a_nSizeY, a_pConfig);
			*a_ppWrapper = pNew.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class ATL_NO_VTABLE CWrapper :
		public CComObjectRootEx<CComMultiThreadModel>,
		public ICanvasInteractingWrapper
	{
	public:
		IRasterImageBrush* Init(ULONG a_nSizeX, ULONG a_nSizeY, CConfigValue const& a_cStyleID, BSTR a_bstrParams);
		void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig);

	BEGIN_COM_MAP(CWrapper)
		COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
	END_COM_MAP()

		// ICanvasInteractingWrapper methods
	public:
		STDMETHOD_(ULONG, GetControlPointCount)();
		STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass);
		STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel);
		STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription);
		STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes);
		STDMETHOD(ToConfig)(IConfig* a_pConfig);

	private:
		ULONG m_nSizeX;
		ULONG m_nSizeY;
		CComPtr<IRasterImageBrush> m_pFS;
		CConfigValue m_cStyleID;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(RasterImageFill), CRasterImageFill)
