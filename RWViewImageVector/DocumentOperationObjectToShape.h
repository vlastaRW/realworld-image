// DocumentOperationObjectToShape.h : Declaration of the CDocumentOperationObjectToShape

#pragma once
#include <RWProcessing.h>
#include <RWDrawing.h>


extern __declspec(selectany) GUID const CLSID_DocumentOperationObjectToShape = { 0xcef65495, 0x18f6, 0x4347, { 0xad, 0x92, 0x62, 0xdf, 0x75, 0xa2, 0xff, 0x77 } };
class DECLSPEC_UUID("CEF65495-18F6-4347-AD92-62DF75A2FF77") DocumentOperationObjectToShape;


// CDocumentOperationObjectToShape

class ATL_NO_VTABLE CDocumentOperationObjectToShape :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentOperationObjectToShape, &CLSID_DocumentOperationObjectToShape>,
	public IDocumentOperation
{
public:
	CDocumentOperationObjectToShape()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentOperationObjectToShape)

BEGIN_CATEGORY_MAP(CDocumentOperationObjectToShape)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentOperationObjectToShape)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates);
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID);

private:
	class ATL_NO_VTABLE CToolWindow : 
		public CComObjectRootEx<CComMultiThreadModel>,
		public IRasterImageEditWindow
	{
	public:

	BEGIN_COM_MAP(CToolWindow)
		COM_INTERFACE_ENTRY(IRasterImageEditWindow)
	END_COM_MAP()

		// IRasterImageEditWindow methods
	public:
		STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY) { *a_pSizeX = 1; *a_pSizeY = 1; return S_OK; }
		STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault) { a_pDefault->bR = a_pDefault->bG = a_pDefault->bB = a_pDefault->bA = 0; return S_OK; }
		STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer) { return S_FALSE; }
		STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
		{
			if (a_pBoundingRectangle)
			{
				a_pBoundingRectangle->left = a_pBoundingRectangle->top = LONG_MIN;
				a_pBoundingRectangle->right = LONG_MAX;
				a_pBoundingRectangle->bottom = LONG_MAX;
			}
			if (a_bEntireRectangle)
				*a_bEntireRectangle = TRUE;
			return S_OK;
		}
		STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer) { return E_NOTIMPL; }
		STDMETHOD(ControlPointsChanged)() { return S_FALSE; }
		STDMETHOD(ControlPointChanged)(ULONG UNREF(a_nIndex)) { return S_FALSE; }
		STDMETHOD(ControlLinesChanged)() { return S_FALSE; }
		STDMETHOD(RectangleChanged)(RECT const* a_pChanged) { return S_FALSE; }
		STDMETHOD(ScrollWindow)(ULONG UNREF(a_nScrollID), TPixelCoords const* UNREF(a_pDelta)) { return E_NOTIMPL; }
		STDMETHOD(ApplyChanges)() { return S_FALSE;} 
		STDMETHOD(SetState)(ISharedState* a_pState) { return E_NOTIMPL; }
		STDMETHOD(SetColors)(TColor const* a_pColor1, TColor const* a_pColor2) { return E_NOTIMPL; }
		STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState) { return E_NOTIMPL; }
		STDMETHOD(Handle)(RWHWND* a_phWnd) { return E_NOTIMPL; }
		STDMETHOD(Document)(IDocument** a_ppDocument) { ATLASSERT(0); return E_NOTIMPL; }
		STDMETHOD(Checkpoint)() { return E_NOTIMPL; }
	};
};

OBJECT_ENTRY_AUTO(__uuidof(DocumentOperationObjectToShape), CDocumentOperationObjectToShape)
