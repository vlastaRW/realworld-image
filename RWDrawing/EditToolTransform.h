
#pragma once


#include "EditTool.h"
#include "EditToolScanlineBuffer.h"
#include "EditToolWithBrush.h"
#include "EditToolRectangularShape.h"

#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rounded_rect.h>
#include <agg_trans_affine.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_span_allocator.h>
#include <agg_scanline_u.h>
#include <agg_span_gradient.h>
#include <agg_conv_stroke.h>
#include <agg_conv_transform.h>
#include <agg_curves.h>


#include <agg_basics.h>
#include <agg_color_rgba.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>
#include <agg_trans_perspective.h>
#include <agg_trans_bilinear.h>
#include <agg_renderer_scanline.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_span_subdiv_adaptor.h>
#include <agg_span_allocator.h>
//#include <agg_span_image_resample_rgba.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_interpolator_trans.h>
#include <agg_image_accessors.h>

struct CEditToolDataTransformation
{
	MIDL_INTERFACE("DC497588-B34E-4C3C-928D-179D8AEAAB6E")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataTransformation const*, InternalData)() = 0;
	};
	enum EMode
	{
		EMSimple = 0,
		EMPerspective,
		EMBezier,
	};

	CEditToolDataTransformation() : eMode(EMSimple), bAspectLock(false)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_OK;
		if (wcsstr(a_bstr, L"SIMPLE"))
			eMode = EMSimple;
		else if (wcsstr(a_bstr, L"PERSPECTIVE"))
			eMode = EMPerspective;
		else if (wcsstr(a_bstr, L"BEZIER"))
			eMode = EMBezier;
		bAspectLock = wcsstr(a_bstr, L"LOCK");
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = SysAllocString(eMode == EMSimple ? (bAspectLock ? L"SIMPLE|LOCK" : L"SIMPLE") : (eMode == EMPerspective ? (bAspectLock ? L"PERSPECTIVE|LOCK" : L"PERSPECTIVE") : (bAspectLock ? L"BEZIER|LOCK" : L"BEZIER")));
		return S_OK;
	}

	EMode eMode;
	bool bAspectLock;
};

extern __declspec(selectany) wchar_t const FLIPSELECTION_NAME[] = L"[0409]Flip[0405]Převrátit";
extern __declspec(selectany) wchar_t const FLIPSELECTION_DESC[] = L"[0409]Turn the image upside down.[0405]Převrátit obrázek podle vodorovné osy.";
extern __declspec(selectany) wchar_t const MIRRORSELECTION_NAME[] = L"[0409]Mirror[0405]Zrcadlit";
extern __declspec(selectany) wchar_t const MIRRORSELECTION_DESC[] = L"[0409]Mirror the image from left to right.[0405]Převrátit obrázek podle svislé osy.";

#include "EditToolTransformDlg.h"


HICON GetToolIconTRANSFORM(ULONG a_nSize);

// copied from RWDocumentImageRaster.h to break circular dependency
// TODO: move elsewhere
typedef void (*fnDeleteBuffer)(TPixelChannel*);
MIDL_INTERFACE("0FA20C30-C04F-490F-9922-EA26EFFCB77F")
IDocumentRasterImage : public IDocumentEditableImage
{
public:
	virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TileSet(
		/* [in] */ ULONG a_nChannelIDs,
		/* [in] */ const TImagePoint * a_pOrigin,
		/* [in] */ const TImageSize * a_pSize,
		/* [in] */ const TImageStride * a_pStride,
		/* [in] */ ULONG a_nPixels,
		/* [size_is][in] */ const TPixelChannel * a_pPixels,
		/* [in] */ BYTE a_bDeleteOldContent) = 0;

	virtual /* [local][helpstring] */ HRESULT STDMETHODCALLTYPE BufferReplace(
		/* [in] */ TImagePoint a_tAllocOrigin,
		/* [in] */ TImageSize a_tAllocSize,
		/* [in] */ const TImagePoint* a_pContentOrigin,
		/* [in] */ const TImageSize* a_pContentSize,
		/* [in] */ const ULONGLONG* a_pContentAlphaSum,
		/* [in] */ TPixelChannel* a_pPixels,
		/* [in] */ fnDeleteBuffer a_pDeleter) = 0;

	virtual /* [local][helpstring] */ HRESULT STDMETHODCALLTYPE BufferAllocate(
		/* [in] */ TImageSize a_tSize,
		/* [out] */ TPixelChannel** a_ppPixels,
		/* [out] */ fnDeleteBuffer* a_ppDeleter) = 0;
};


class CEditToolTransformation :
	public CEditToolMouseInput<CEditToolTransformation>, // no direct tablet support
	public CEditToolCustomOrMoveCursor<CEditToolTransformation, GetToolIconTRANSFORM>, // cursor handler
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolTransformation, // T - the top level class for cross casting
		CEditToolTransformation, // TResetHandler
		CEditToolTransformation, // TDirtyHandler
		CEditToolTransformation, // TImageTileHandler
		CEditToolTransformation, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CEditToolTransformation, // TGlobalsHandler
		CEditToolTransformation, // TAdjustCoordsHandler
		CEditToolCustomOrMoveCursor<CEditToolTransformation, GetToolIconTRANSFORM>, // TGetCursorHandler
		CEditToolMouseInput<CEditToolTransformation>, // TProcessInputHandler
		CEditToolTransformation, // TPreTranslateMessageHandler
		CEditToolTransformation, // TControlPointsHandler
		CEditToolTransformation // TControlLinesHandler
	>,
	public IRasterImageEditToolFloatingSelection,
	public IRasterImageEditToolScripting,
	public IRasterImageEditToolContextMenu,
	public IDesignerViewStatusBar
{
public:
	CEditToolTransformation() :
		m_eBlendingMode(EBMDrawOver), m_eRasterizationMode(ERMSmooth), m_eCoordinatesMode(ECMFloatingPoint),
		m_rcPrev(RECT_EMPTY), m_rcDelete(RECT_EMPTY), m_bUseDeleteMask(false)
	{
		ResetRectangle();
		m_szSelSize.cx = m_szSelSize.cy = 0;
	}

	BEGIN_COM_MAP(CEditToolTransformation)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		COM_INTERFACE_ENTRY(IRasterImageEditToolFloatingSelection)
		COM_INTERFACE_ENTRY(IRasterImageEditToolContextMenu)
		COM_INTERFACE_ENTRY(IDesignerViewStatusBar)
	END_COM_MAP()

	// IRasterImageEditToolContextMenu methods
public:
	class ATL_NO_VTABLE CRestoreSelection :
		public CDocumentMenuCommandImpl<CRestoreSelection, IDS_MENU_RESTORESELECTION_NAME, IDS_MENU_RESTORESELECTION_DESC, NULL, 0>
	{
	public:
		CRestoreSelection() : m_pTool(NULL)
		{
		}
		~CRestoreSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolTransformation* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->RestoreSelection();
		}

	private:
		CEditToolTransformation* m_pTool;
	};
	class ATL_NO_VTABLE CStretchSelection :
		public CDocumentMenuCommandImpl<CStretchSelection, IDS_MENU_STRETCHSELECTION_NAME, IDS_MENU_STRETCHSELECTION_DESC, NULL, 0>
	{
	public:
		CStretchSelection() : m_pTool(NULL)
		{
		}
		~CStretchSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolTransformation* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->StretchSelection();
		}

	private:
		CEditToolTransformation* m_pTool;
	};
	class ATL_NO_VTABLE CCropToSelection :
		public CDocumentMenuCommandImpl<CCropToSelection, IDS_MENU_CROP2SELECTION_NAME, IDS_MENU_CROP2SELECTION_DESC, NULL, 0>
	{
	public:
		CCropToSelection() : m_pTool(NULL)
		{
		}
		~CCropToSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolTransformation* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->CropToSelection();
		}

	private:
		CEditToolTransformation* m_pTool;
	};
	class ATL_NO_VTABLE CFlipSelection :
		public CDocumentMenuCommandMLImpl<CFlipSelection, FLIPSELECTION_NAME, FLIPSELECTION_DESC, NULL, 0>
	{
	public:
		CFlipSelection() : m_pTool(NULL)
		{
		}
		~CFlipSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolTransformation* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->FlipSelection();
		}

	private:
		CEditToolTransformation* m_pTool;
	};
	class ATL_NO_VTABLE CMirrorSelection :
		public CDocumentMenuCommandMLImpl<CMirrorSelection, MIRRORSELECTION_NAME, MIRRORSELECTION_DESC, NULL, 0>
	{
	public:
		CMirrorSelection() : m_pTool(NULL)
		{
		}
		~CMirrorSelection()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolTransformation* a_pTool)
		{
			(m_pTool = a_pTool)->AddRef();
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->MirrorSelection();
		}

	private:
		CEditToolTransformation* m_pTool;
	};

	HRESULT RestoreSelection()
	{
		if (M_Window() == NULL || m_eState == ESClean)
			return S_FALSE;
		if (m_rcDelete.left < m_rcDelete.right && m_rcDelete.top < m_rcDelete.bottom)
			InitAsRectangle(m_rcDelete.left, m_rcDelete.top, m_rcDelete.right, m_rcDelete.bottom);
		else
			InitAsRectangle(0, 0, m_szSelSize.cx, m_szSelSize.cy);
		return S_OK;
	}
	HRESULT StretchSelection()
	{
		if (M_Window() == NULL || m_eState == ESClean)
			return S_FALSE;
		InitAsRectangle(0, 0, m_nSizeX, m_nSizeY);
		return S_OK;
	}
	HRESULT CropToSelection()
	{
		if (M_Window() == NULL || m_eState == ESClean)
			return S_FALSE;
		CComPtr<IDocument> pDoc;
		M_Window()->Document(&pDoc);
		CComPtr<IDocumentRasterImage> pRI;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI)
		{
			// simple rectangular crop
			TImageSize tSize = {m_szSelSize.cx, m_szSelSize.cy};
			CWriteLock<IDocument> cLock(pDoc);
			RECT const rc = GetBoundingBox();
			TMatrix3x3f const tMtx = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  -rc.left, -rc.top, 1.0f};
			pRI->CanvasSet(&tSize, NULL, &tMtx, NULL);
			pRI->TileSet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, reinterpret_cast<TPixelChannel*>(m_cSelData.m_p), TRUE);
		}
		return S_OK;
	}
	HRESULT FlipSelection()
	{
		if (m_cSelData.m_p == NULL)
			return S_FALSE;
		TRasterImagePixel* p1 = m_cSelData;
		TRasterImagePixel* p2 = p1+m_szSelSize.cx*(m_szSelSize.cy-1);
		//BYTE* pM1 = m_cSelMask;
		//BYTE* pM2 = pM1+m_szSelSize.cx*(m_szSelSize.cy-1);
		for (LONG y = 0; y < m_szSelSize.cy>>1; ++y)
		{
			for (LONG x = 0; x < m_szSelSize.cx; ++x, ++p1, ++p2)
			{
				TRasterImagePixel const t = *p1;
				*p1 = *p2;
				*p2 = t;
			}
			p2 -= m_szSelSize.cx<<1;
			//if (m_cSelMask.m_p)
			//{
			//	for (LONG x = 0; x < m_szSelSize.cx; ++x, ++pM1, ++pM2)
			//	{
			//		BYTE const t = *pM1;
			//		*pM1 = *pM2;
			//		*pM2 = t;
			//	}
			//	pM2 -= m_szSelSize.cx<<1;
			//}
		}
		RECT rc = GetBoundingBox();
		M_Window()->RectangleChanged(&rc);
		return S_OK;
	}
	HRESULT MirrorSelection()
	{
		if (m_cSelData.m_p == NULL)
			return S_FALSE;
		for (LONG y = 0; y < m_szSelSize.cy; ++y)
		{
			TRasterImagePixel* p1 = m_cSelData.m_p+m_szSelSize.cx*y;
			TRasterImagePixel* p2 = p1+m_szSelSize.cx-1;
			for (LONG x = 0; x < m_szSelSize.cx>>1; ++x, ++p1, --p2)
			{
				TRasterImagePixel const t = *p1;
				*p1 = *p2;
				*p2 = t;
			}
			p2 -= m_szSelSize.cx<<1;
			//if (m_cSelMask.m_p)
			//{
			//	BYTE* pM1 = m_cSelMask.m_p+m_szSelSize.cx*y;
			//	BYTE* pM2 = pM1+m_szSelSize.cx-1;
			//	for (LONG x = 0; x < m_szSelSize.cx>>1; ++x, ++pM1, --pM2)
			//	{
			//		BYTE const t = *pM1;
			//		*pM1 = *pM2;
			//		*pM2 = t;
			//	}
			//	pM2 -= m_szSelSize.cx<<1;
			//}
		}
		RECT rc = GetBoundingBox();
		M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	STDMETHOD(CommandsEnum)(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			if (_IsDirty(NULL, NULL, NULL) != S_OK)
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CRestoreSelection>* p = NULL;
				CComObject<CRestoreSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CStretchSelection>* p = NULL;
				CComObject<CStretchSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CCropToSelection>* p = NULL;
				CComObject<CCropToSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			{
				CComPtr<IDocumentMenuCommand> pSep;
				RWCoCreateInstance(pSep, __uuidof(MenuCommandsSeparator));
				pItems->Insert(pSep);
			}
			{
				CComObject<CFlipSelection>* p = NULL;
				CComObject<CFlipSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMirrorSelection>* p = NULL;
				CComObject<CMirrorSelection>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

	// IDesignerViewStatusBar
public:
	STDMETHOD(Update)(IDesignerStatusBar* a_pStatusBar)
	{
		OLECHAR szTmp[64];
		if (m_cSelData.m_p)
		{
			swprintf(szTmp, L"%ix%i", m_szSelSize.cx, m_szSelSize.cy);
		}
		else if (m_eState == ESDragging)
		{
			int nX1 = floorf(m_tDragStart.fX+0.5f);
			int nY1 = floorf(m_tDragStart.fY+0.5f);
			int nX2 = floorf(m_tDragLast.fX+0.5f);
			int nY2 = floorf(m_tDragLast.fY+0.5f);
			if (nX1 != nX2 || nY1 != nY2)
				swprintf(szTmp, L"%ix%i", abs(nX2-nX1), abs(nY2-nY1));
			else
				return S_FALSE;
		}
		else
			return S_FALSE;
		return a_pStatusBar->PaneSet(CComBSTR("TRANSFORMSIZE"), NULL, CComBSTR(szTmp), 60, -200);
	}



	ECoordinatesMode M_CoordinatesMode() const
	{
		return m_eCoordinatesMode;
	}

	template<class TPixelMixer, class TTransform>
	void DrawSimple(RECT const& a_rc, RECT const& a_rcDirty, TRasterImagePixel* a_pBuffer, ULONG a_nStride, TTransform a_tr, RECT const* a_pSrcLimit = NULL)
	{
		RECT rc;
		if (a_pSrcLimit == NULL)
		{
			rc.left = rc.top = 0;
			rc.right = m_szSelSize.cx;
			rc.bottom = m_szSelSize.cy;
			a_pSrcLimit = &rc;
		}
		for (LONG y = a_rc.top; y < a_rc.bottom; ++y)
		{
			TRasterImagePixel* pBuf = a_pBuffer+(y-a_rcDirty.top)*a_nStride+a_rc.left-a_rcDirty.left;
			for (LONG x = a_rc.left; x < a_rc.right; ++x, ++pBuf)
			{
				double xx = x+0.5;
				double yy = y+0.5;
				a_tr.transform(&xx, &yy);
				LONG xxx = LONG(xx+1.0)-1;
				LONG yyy = LONG(yy+1.0)-1;
				if (xxx >= a_pSrcLimit->left && xxx < a_pSrcLimit->right && yyy >= a_pSrcLimit->top && yyy < a_pSrcLimit->bottom)
				{
					if (m_bUseDeleteMask)
						TPixelMixer::Mix(*pBuf, m_cSelData[yyy*m_szSelSize.cx+xxx], m_cDeleteMask[yyy*m_szSelSize.cx+xxx]);
					else
						TPixelMixer::Mix(*pBuf, m_cSelData[yyy*m_szSelSize.cx+xxx]);
				}
			}
		}
	}

	template<class TPixelMixer, class TTransform>
	void DrawSmooth(RECT const& a_rc, RECT const& a_rcDirty, TRasterImagePixel* a_pBuffer, ULONG a_nStride, TTransform a_tr)
	{
		for (LONG y = a_rc.top; y < a_rc.bottom; ++y)
		{
			TRasterImagePixel* pBuf = a_pBuffer+(y-a_rcDirty.top)*a_nStride+a_rc.left-a_rcDirty.left;
			for (LONG x = a_rc.left; x < a_rc.right; ++x, ++pBuf)
			{
				double xx = x+0.5;
				double yy = y+0.5;
				a_tr.transform(&xx, &yy);
				LONG xxx = LONG(xx+1.5)-2;
				LONG yyy = LONG(yy+1.5)-2;
				if (xxx >= -1 && xxx < m_szSelSize.cx && yyy >= -1 && yyy < m_szSelSize.cy)
				{
					ULONG nWX = 128*(xx-xxx-0.5);
					ULONG nWY = 128*(yy-yyy-0.5);
					ULONG nR = 0;
					ULONG nG = 0;
					ULONG nB = 0;
					ULONG nA = 0;
					ULONG nC = 0;
					TRasterImagePixel* p;
					if (xxx >= 0)
					{
						if (xxx != m_szSelSize.cx-1)
						{
							if (yyy >= 0)
							{
								if (yyy != m_szSelSize.cy-1)
								{
									{
										p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx;
										ULONG const nW = (128-nWY)*(128-nWX);
										nA += nW*p->bA;
										ULONG const n = nW*p->bA+nW*(p->bA>>7);
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx]*nW : 255*nW;
									}
									{
										p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx;
										ULONG const nW = nWY*(128-nWX);
										nA += nW*p->bA;
										ULONG const n = nW*p->bA+nW*(p->bA>>7);
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nW : 255*nW;
									}
									{
										p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx+1;
										ULONG const nW = (128-nWY)*nWX;
										nA += nW*p->bA;
										ULONG const n = nW*p->bA+nW*(p->bA>>7);
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nW : 255*nW;
									}
									{
										p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx+1;
										ULONG const nW = nWY*nWX;
										nA += nW*p->bA;
										ULONG const n = nW*p->bA+nW*(p->bA>>7);
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nW : 255*nW;
									}
								}
								else
								{
									{
										p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx;
										ULONG const n = ((128-nWX)<<7)*p->bA;
										nA += n;
										ULONG const n2 = n+(128-nWX)*p->bA;
										nR += n2*p->bR;
										nG += n2*p->bG;
										nB += n2*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*(128-nWY) : 255*(128-nWX)*(128-nWY);
									}
									{
										p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx+1;
										ULONG const n = (nWX<<7)*p->bA;
										nA += n;
										ULONG const n2 = n+nWX*p->bA;
										nR += n2*p->bR;
										nG += n2*p->bG;
										nB += n2*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nWX*(128-nWY) : 255*nWX*(128-nWY);
									}
								}
							}
							else
							{
								{
									p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx;
									ULONG const n = ((128-nWX)<<7)*p->bA;
									nA += n;
									ULONG const n2 = n+(128-nWX)*p->bA;
									nR += n2*p->bR;
									nG += n2*p->bG;
									nB += n2*p->bB;
									nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*nWY : 255*(128-nWX)*nWY;
								}
								{
									p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx+1;
									ULONG const n = (nWX<<7)*p->bA;
									nA += n;
									ULONG const n2 = n+nWX*p->bA;
									nR += n2*p->bR;
									nG += n2*p->bG;
									nB += n2*p->bB;
									nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nWX*nWY : 255*nWX*nWY;
								}
							}
						}
						else
						{
							if (yyy >= 0)
							{
								if (yyy != m_szSelSize.cy-1)
								{
									{
										p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx;
										ULONG const n = ((128-nWY)<<7)*p->bA;
										nA += n;
										ULONG const n2 = n+(128-nWY)*p->bA;
										nR += n2*p->bR;
										nG += n2*p->bG;
										nB += n2*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWY)*(128-nWX) : 255*(128-nWY)*(128-nWX);
									}
									{
										p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx;
										ULONG const n = (nWY<<7)*p->bA;
										nA += n;
										ULONG const n2 = n+nWY*p->bA;
										nR += n2*p->bR;
										nG += n2*p->bG;
										nB += n2*p->bB;
										nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nWY*(128-nWX) : 255*nWY*(128-nWX);
									}
								}
								else
								{
									p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx;
									ULONG const n = ULONG(p->bA)<<14;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWX)*(128-nWY) : 255*(128-nWX)*(128-nWY);
								}
							}
							else
							{
								p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*nWY : 255*(128-nWX)*nWY;
							}
						}
					}
					else
					{
						if (yyy >= 0)
						{
							if (yyy != m_szSelSize.cy-1)
							{
								{
									p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx+1;
									ULONG const n = ((128-nWY)<<7)*p->bA;
									nA += n;
									ULONG const n2 = n+(128-nWY)*p->bA;
									nR += n2*p->bR;
									nG += n2*p->bG;
									nB += n2*p->bB;
									nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWY)*nWX : 255*(128-nWY)*nWX;
								}
								{
									p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx+1;
									ULONG const n = (nWY<<7)*p->bA;
									nA += n;
									ULONG const n2 = n+nWY*p->bA;
									nR += n2*p->bR;
									nG += n2*p->bG;
									nB += n2*p->bB;
									nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nWY*nWX : 255*nWY*nWX;
								}
							}
							else
							{
								p = m_cSelData.m_p + yyy*m_szSelSize.cx+xxx+1;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += m_bUseDeleteMask ? m_cDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nWX*(128-nWY) : 255*nWX*(128-nWY);
							}
						}
						else
						{
							p = m_cSelData.m_p + (yyy+1)*m_szSelSize.cx+xxx+1;
							ULONG const n = ULONG(p->bA)<<14;
							nA += n;
							nR += n*p->bR;
							nG += n*p->bG;
							nB += n*p->bB;
							nC += m_bUseDeleteMask ? m_cDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nWX*nWY : 255*nWX*nWY;
						}
					}
					TRasterImagePixel t = {0, 0, 0, nA>>14};
					if (t.bA)
					{
						t.bR = nR>>22;
						t.bG = nG>>22;
						t.bB = nB>>22;
					}
					TPixelMixer::MixPM(*pBuf, t, nC>>14);
				}
			}
		}
	}

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		if (m_rcDelete.left < m_rcDelete.right && m_rcDelete.top < m_rcDelete.bottom)
			M_Window()->RectangleChanged(&m_rcDelete);
		if (m_rcPrev.left < m_rcPrev.right && m_rcPrev.top < m_rcPrev.bottom)
			M_Window()->RectangleChanged(&m_rcPrev);
		ResetRectangle();
		m_nSizeX = 0;
		m_nSizeY = 0;
		M_Window()->Size(&m_nSizeX, &m_nSizeY);
		m_cSelData.Free();
		m_cDeleteMask.Free();
		m_rcDelete = RECT_EMPTY;
		m_bUseDeleteMask = false;
		if (m_szSelSize.cx == 0 && m_szSelSize.cy == 0)
		{
			RECT rcBounds = {0, 0, m_nSizeX, m_nSizeY};
			BOOL bEntire = TRUE;
			M_Window()->GetSelectionInfo(&rcBounds, &bEntire);
			if ((rcBounds.left != 0 || rcBounds.top != 0 ||
				rcBounds.right != m_nSizeX || rcBounds.bottom != m_nSizeY ||
				bEntire != TRUE) && rcBounds.left < rcBounds.right && rcBounds.top < rcBounds.bottom)
			{
				m_szSelSize.cx = rcBounds.right-rcBounds.left;
				m_szSelSize.cy = rcBounds.bottom-rcBounds.top;
				m_cSelData.Allocate(m_szSelSize.cx*m_szSelSize.cy);
				if (!bEntire)
				{
					m_cDeleteMask.Allocate(m_szSelSize.cx*m_szSelSize.cy);
					M_Window()->GetSelectionTile(rcBounds.left, rcBounds.top, rcBounds.right-rcBounds.left, rcBounds.bottom-rcBounds.top, rcBounds.right-rcBounds.left, m_cDeleteMask);
					m_bUseDeleteMask = true;
				}
				M_Window()->GetImageTile(rcBounds.left, rcBounds.top, rcBounds.right-rcBounds.left, rcBounds.bottom-rcBounds.top, 0.0f, rcBounds.right-rcBounds.left, EITIContent, m_cSelData);
				m_rcDelete = rcBounds;
				InitAsRectangle(rcBounds.left, rcBounds.top, rcBounds.right, rcBounds.bottom);
				m_eState = ESFloating;
				m_rcPrev = GetBoundingBox();
				RECT rcAll = {0, 0, m_nSizeX, m_nSizeY};
				M_Window()->RectangleChanged(&rcAll);
			}
		}
		else
		{
			m_szSelSize.cx = m_szSelSize.cy = 0;
		}
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		return S_OK;
	}

	template<typename TBuffer, typename TRenderer>
	void RenderBezier(TBuffer& cBB, TRenderer& renb, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY) const
	{
		renb.clip_box_naked(a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY);

		int const nStepsV = 16;
		int const nStepsU = 16;
		int nV1 = 0;
		for (int v = 0; v < nStepsV; ++v)
		{
			int nV2 = float(v+1)*m_szSelSize.cy/nStepsV+0.5f;
			if (nV2 == nV1) continue;
			int nU1 = 0;
			for (int u = 0; u < nStepsU; ++u)
			{
				int nU2 = float(u+1)*m_szSelSize.cx/nStepsU+0.5f;
				if (nU2 == nU1) continue;
				double d0[10] =
				{
					float(nU1)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy,
					float(nU2)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy,
					float(nU2)/m_szSelSize.cx, float(nV2)/m_szSelSize.cy,
					float(nU1)/m_szSelSize.cx, float(nV2)/m_szSelSize.cy,
					float(nU1)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy
				};
				double d1[10] =
				{
					float(nU1), float(nV1),
					float(nU2), float(nV1),
					float(nU2), float(nV2),
					float(nU1), float(nV2),
					float(nU1), float(nV1)
				};
				TPixelCoords t0 = BezierAt(d0[0], d0[1]);
				TPixelCoords t1 = BezierAt(d0[2], d0[3]);
				TPixelCoords t2 = BezierAt(d0[6], d0[7]);
				TPixelCoords t3 = BezierAt(d0[4], d0[5]);
				double d[10] = {t0.fX, t0.fY, t1.fX, t1.fY, t3.fX, t3.fY, t2.fX, t2.fY, t0.fX, t0.fY};
				//agg::trans_perspective/*bilinear*/ tr(d, nU1, nV1, nU2, nV2);
				agg::trans_affine tr2(d, d1);
				//if (isConvex(d) && tr.is_valid())
				{
					cBB.SetTransform(tr2);

					agg::rasterizer_scanline_aa<> ras;
					agg::scanline_p8 sl;

					ras.reset();
					ras.move_to_d(t0.fX, t0.fY);
					ras.line_to_d(t1.fX, t1.fY);
					ras.line_to_d(t3.fX, t3.fY);

					agg::span_allocator<TPixelChannel> span_alloc;
					agg::render_scanlines_aa(ras, sl, renb, span_alloc, cBB);
				}
				agg::trans_affine tr3(d+4, d1+4);
				{
					cBB.SetTransform(tr3);

					agg::rasterizer_scanline_aa<> ras;
					agg::scanline_p8 sl;

					ras.reset();
					ras.move_to_d(t3.fX, t3.fY);
					ras.line_to_d(t2.fX, t2.fY);
					ras.line_to_d(t0.fX, t0.fY);

					agg::span_allocator<TPixelChannel> span_alloc;
					agg::render_scanlines_aa(ras, sl, renb, span_alloc, cBB);
				}
				nU1 = nU2;
			}
			nV1 = nV2;
		}
	}

	HRESULT _GetImageTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		HRESULT hRes = M_Window()->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
		if (FAILED(hRes))
			return hRes;
		RECT rcDirty = {a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY};
		RECT rc =
		{
			max(rcDirty.left, m_rcDelete.left),
			max(rcDirty.top, m_rcDelete.top),
			min(rcDirty.right, m_rcDelete.right),
			min(rcDirty.bottom, m_rcDelete.bottom),
		};
		if (rc.left < rc.right && rc.top < rc.bottom)
		{
			TRasterImagePixel tDef = {0, 0, 0, 0};
			M_Window()->GetDefaultColor(&tDef);
			for (LONG y = rc.top; y < rc.bottom; ++y)
			{
				if (m_cDeleteMask.m_p)
				{
					TRasterImagePixel* pBuf = a_pBuffer+(y-rcDirty.top)*a_nStride+rc.left-rcDirty.left;
					BYTE* pDel = m_cDeleteMask.m_p+(y-m_rcDelete.top)*(m_rcDelete.right-m_rcDelete.left)+rc.left-m_rcDelete.left;
					for (LONG x = rc.left; x < rc.right; ++x)
					{
						CPixelMixerReplace::Mix(*pBuf, tDef, *pDel);
						//pBuf->bA = pBuf->bA*(255-*pDel)/255;
						//if (pBuf->bA == 0)
						//	pBuf->bR = pBuf->bG = pBuf->bB = 0;
						++pBuf;
						++pDel;
					}
				}
				else
				{
					std::fill_n(a_pBuffer+(y-rcDirty.top)*a_nStride+rc.left-rcDirty.left, (rc.right-rc.left), tDef);
					//ZeroMemory(a_pBuffer+(y-rcDirty.top)*a_nStride+rc.left-rcDirty.left, (rc.right-rc.left)*sizeof*a_pBuffer);
				}
			}
		}
		if (m_cSelData.m_p)
		{
			if (IsSimpleMove())
			{
				TPixelCoords tPt = m_aPixels[0];
				if (m_cData.eMode == CEditToolDataTransformation::EMSimple)
				{
					tPt.fX = m_tCenter.fX-m_fSizeX*0.5f;
					tPt.fY = m_tCenter.fY-m_fSizeY*0.5f;
				}
				// rectangle was only moved (no resizing, no rotation) - use optimized code
				rc.left = max(rcDirty.left, LONG(tPt.fX));
				rc.top = max(rcDirty.top, LONG(tPt.fY));
				rc.right = min(rcDirty.right, LONG(tPt.fX)+m_szSelSize.cx);
				rc.bottom = min(rcDirty.bottom, LONG(tPt.fY)+m_szSelSize.cy);

				for (LONG y = rc.top; y < rc.bottom; ++y)
				{
					TRasterImagePixel* pBuf = a_pBuffer+(y-rcDirty.top)*a_nStride+rc.left-rcDirty.left;
					TRasterImagePixel* pSel = m_cSelData.m_p+(y-LONG(tPt.fY))*m_szSelSize.cx+rc.left-LONG(tPt.fX);
					if (m_bUseDeleteMask)
					{
						BYTE* pDel = m_cDeleteMask.m_p+(y-LONG(tPt.fY))*(m_rcDelete.right-m_rcDelete.left)+rc.left-LONG(tPt.fX);
						switch (m_eBlendingMode)
						{
						case EBMDrawOver:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel, ++pDel)
								CPixelMixerPaintOver::Mix(*pBuf, *pSel, *pDel);
							break;
						case EBMReplace:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel, ++pDel)
								CPixelMixerReplace::Mix(*pBuf, *pSel, *pDel);
							break;
						case EBMDrawUnder:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel, ++pDel)
								CPixelMixerPaintUnder::Mix(*pBuf, *pSel, *pDel);
							break;
						}
					}
					else
					{
						switch (m_eBlendingMode)
						{
						case EBMDrawOver:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel)
								CPixelMixerPaintOver::Mix(*pBuf, *pSel);
							break;
						case EBMReplace:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel)
								CPixelMixerReplace::Mix(*pBuf, *pSel);
							break;
						case EBMDrawUnder:
							for (LONG x = rc.left; x < rc.right; ++x, ++pBuf, ++pSel)
								CPixelMixerPaintUnder::Mix(*pBuf, *pSel);
							break;
						}
					}
				}
			}
			else
			{
				RECT rc = GetBoundingBox();
				bool const bSrcSmall = m_szSelSize.cx < 2 || m_szSelSize.cy < 2;
				if (rc.left < rcDirty.left) rc.left = rcDirty.left;
				if (rc.top < rcDirty.top) rc.top = rcDirty.top;
				if (rc.right > rcDirty.right) rc.right = rcDirty.right;
				if (rc.bottom > rcDirty.bottom) rc.bottom = rcDirty.bottom;
				double dest[8] = {m_aPixels[0].fX, m_aPixels[0].fY, m_aPixels[1].fX, m_aPixels[1].fY, m_aPixels[2].fX, m_aPixels[2].fY, m_aPixels[3].fX, m_aPixels[3].fY};
				if (m_cData.eMode == CEditToolDataTransformation::EMBezier)
				{
					CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[a_nSizeX*a_nSizeY]);
					ZeroMemory(cBuffer.m_p, a_nSizeX*a_nSizeY*sizeof*cBuffer.m_p);
					CAutoVectorPtr<BYTE> cCovers(new BYTE[a_nSizeX*a_nSizeY]);
					ZeroMemory(cCovers.m_p, a_nSizeX*a_nSizeY*sizeof*cCovers.m_p);

					if (m_eRasterizationMode == ERMBinary || bSrcSmall)
					{
						CBezierBuffer<CSamplerSimple> cBB(cBuffer, cCovers, a_nX, a_nY, a_nSizeX, a_nSizeY, m_szSelSize, m_cSelData, m_bUseDeleteMask ? m_cDeleteMask.m_p : NULL);
						agg::renderer_base<CBezierBuffer<CSamplerSimple> > renb(cBB);
						RenderBezier(cBB, renb, a_nX, a_nY, a_nSizeX, a_nSizeY);
					}
					else
					{
						CBezierBuffer<CSamplerSmooth> cBB(cBuffer, cCovers, a_nX, a_nY, a_nSizeX, a_nSizeY, m_szSelSize, m_cSelData, m_bUseDeleteMask ? m_cDeleteMask.m_p : NULL);
						agg::renderer_base<CBezierBuffer<CSamplerSmooth> > renb(cBB);
						RenderBezier(cBB, renb, a_nX, a_nY, a_nSizeX, a_nSizeY);
					}

					CAutoPtr<CGammaTables> pGT;
					if (a_fGamma != 1.0f && a_fGamma >= 0.1f && a_fGamma <= 10.f)
						pGT.Attach(a_fGamma >= 2.1f && a_fGamma <= 2.3f ? new CGammaTables() : new CGammaTables(a_fGamma));

					if (m_eRasterizationMode == ERMBinary)
					{
						BYTE* pC = cCovers;
						for (BYTE* const pCE = pC+a_nSizeX*a_nSizeY; pC != pCE; ++pC)
							*pC = *pC < 0x80 ? 0 : 255;
					}

					TRasterImagePixel const* pS = cBuffer;
					BYTE const* pC = cCovers;
					TRasterImagePixel* pD = a_pBuffer;
					for (ULONG y = 0; y < a_nSizeY; ++y)
					{
						switch (m_eBlendingMode)
						{
						case EBMDrawOver:
							if (pGT.m_p)
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerPaintOver::Mix(*pD, *pS, *pC, pGT);
							else
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerPaintOver::Mix(*pD, *pS, *pC);
							break;
						case EBMReplace:
							if (pGT.m_p)
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerReplace::Mix(*pD, *pS, *pC, pGT);
							else
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerReplace::Mix(*pD, *pS, *pC);
							break;
						case EBMDrawUnder:
							if (pGT.m_p)
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerPaintUnder::Mix(*pD, *pS, *pC, pGT);
							else
								for (ULONG x = 0; x < a_nSizeX; ++x, ++pD, ++pS, ++pC)
									CPixelMixerPaintUnder::Mix(*pD, *pS, *pC);
							break;
						}
						pD += a_nStride-a_nSizeX;
					}
				}
				else if (m_cData.eMode == CEditToolDataTransformation::EMPerspective)
				{
					agg::trans_perspective tr(dest, 0, 0, m_szSelSize.cx, m_szSelSize.cy);
					if (tr.is_valid())
						switch (m_eBlendingMode)
						{
						case EBMDrawOver:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerPaintOver>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerPaintOver>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						case EBMReplace:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerReplace>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerReplace>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						case EBMDrawUnder:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerPaintUnder>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerPaintUnder>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						}
				}
				else
				{
					agg::trans_affine tr;
					tr = agg::trans_affine_translation(-m_tCenter.fX, -m_tCenter.fY);
					tr *= agg::trans_affine_rotation(-m_fAngle/* * 3.1415926 / 180.0*/);
					tr *= agg::trans_affine_scaling(m_szSelSize.cx/m_fSizeX, m_szSelSize.cy/m_fSizeY);
					tr *= agg::trans_affine_translation(m_szSelSize.cx*0.5, m_szSelSize.cy*0.5);
					//agg::trans_bilinear tr(dest, 0, 0, m_szSelSize.cx, m_szSelSize.cy);
					if (tr.is_valid())
						switch (m_eBlendingMode)
						{
						case EBMDrawOver:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerPaintOver>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerPaintOver>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						case EBMReplace:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerReplace>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerReplace>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						case EBMDrawUnder:
							if (m_eRasterizationMode == ERMBinary || bSrcSmall)
								DrawSimple<CPixelMixerPaintUnder>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							else
								DrawSmooth<CPixelMixerPaintUnder>(rc, rcDirty, a_pBuffer, a_nStride, tr);
							break;
						}
				}
			}
		}
		return S_OK;
	}
	HRESULT _IsDirty(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (a_pOptimizeImageRect)
			*a_pOptimizeImageRect = TRUE; // TODO: compare number of pixels in rectangle with number of pixels in scanline buffers
		if (a_pSelectionRect)
		{
			a_pSelectionRect->left = 0;
			a_pSelectionRect->right = m_nSizeX;
			a_pSelectionRect->top = 0;
			a_pSelectionRect->bottom = m_nSizeY;
		}
		if (a_pImageRect)
		{
			*a_pImageRect = GetBoundingBox();
			if (a_pImageRect->left > m_rcDelete.left) a_pImageRect->left = m_rcDelete.left;
			if (a_pImageRect->top > m_rcDelete.top) a_pImageRect->top = m_rcDelete.top;
			if (a_pImageRect->right < m_rcDelete.right) a_pImageRect->right = m_rcDelete.right;
			if (a_pImageRect->bottom < m_rcDelete.bottom) a_pImageRect->bottom = m_rcDelete.bottom;
		}
		return m_cSelData.m_p ? S_OK : S_FALSE;
	}
	HRESULT _GetSelectionInfo(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_pBoundingRectangle)
		{
			a_pBoundingRectangle->left = 0;
			a_pBoundingRectangle->right = m_nSizeX;
			a_pBoundingRectangle->top = 0;
			a_pBoundingRectangle->bottom = m_nSizeY;
		}
		if (a_bEntireRectangle)
		{
			*a_bEntireRectangle = TRUE;
		}
		return S_OK;
	}
	HRESULT _GetSelectionTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		for (ULONG y = 0; y < a_nSizeY; ++y)
			FillMemory(a_pBuffer+y*a_nStride, a_nSizeX, 0xff);
		return S_OK;
	}

	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (m_eState == ESClean || m_cSelData.m_p == NULL)
			return S_FALSE;

		if (a_pMsg->message != WM_KEYDOWN ||
			(a_pMsg->wParam != VK_LEFT && a_pMsg->wParam != VK_RIGHT &&
			 a_pMsg->wParam != VK_UP && a_pMsg->wParam != VK_DOWN && a_pMsg->wParam != VK_DELETE))
			 return S_FALSE;

		if (a_pMsg->wParam == VK_DELETE)
		{
			// delete the selected region and apply
			m_cSelData.Free();
			return ETPAApply;
		}

		LONG nDX = 0;
		LONG nDY = 0;
		if (a_pMsg->wParam == VK_LEFT)
			nDX = -1;
		else if (a_pMsg->wParam == VK_RIGHT)
			nDX = 1;
		else if (a_pMsg->wParam == VK_UP)
			nDY = -1;
		else if (a_pMsg->wParam == VK_DOWN)
			nDY = 1;

		if (m_cData.eMode == CEditToolDataTransformation::EMBezier)
		{
			for (int i = 0; i < 16; ++i)
			{
				m_aBezier[i].fX += nDX;
				m_aBezier[i].fY += nDY;
			}
		}
		else if (m_cData.eMode == CEditToolDataTransformation::EMPerspective)
		{
			for (int i = 0; i < 4; ++i)
			{
				m_aPixels[i].fX += nDX;
				m_aPixels[i].fY += nDY;
			}
		}
		else if (0x8000&GetAsyncKeyState(VK_SHIFT))
		{
			if (m_cData.bAspectLock)
			{
				float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
				if (nDX != 0)
					nDY = nDX*fAspectLock+0.5f;
				else
					nDX = nDY/fAspectLock+0.5f;
			}
			float const fCos = cosf(m_fAngle);
			float const fSin = sinf(m_fAngle);
			m_tCenter.fX += nDX*0.5f*fCos-nDY*0.5f*fSin;
			m_tCenter.fY += nDY*0.5f*fCos+nDX*0.5f*fSin;
			m_fSizeX += nDX;
			m_fSizeY += nDY;
		}
		else
		{
			m_tCenter.fX += nDX;
			m_tCenter.fY += nDY;
		}
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		ShapeChanged();
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataTransformation::ISharedStateToolData> pData(a_pState);
		if (pData == NULL)
			return S_FALSE;
		CEditToolDataTransformation cData = m_cData;
		m_cData = *(pData->InternalData());
		if (m_cSelData.m_p == NULL || m_eState == ESClean)
			return S_OK;
		bool bChange = false;
		if (cData.eMode != m_cData.eMode)
		{
			bChange = true;
			if (m_cData.eMode == CEditToolDataTransformation::EMBezier)
			{
				if (cData.eMode == CEditToolDataTransformation::EMSimple)
				{
					TPixelCoords aPts[4];
					SimpleToPts(aPts);
					InitBezier(aPts);
				}
				else if (cData.eMode == CEditToolDataTransformation::EMPerspective)
				{
					InitBezier(m_aPixels);
				}
			}
			else if (m_cData.eMode == CEditToolDataTransformation::EMPerspective)
			{
				if (cData.eMode == CEditToolDataTransformation::EMSimple)
				{
					SimpleToPts(m_aPixels);
				}
				else if (cData.eMode == CEditToolDataTransformation::EMBezier)
				{
					m_aPixels[0] = m_aBezier[0];
					m_aPixels[1] = m_aBezier[3];
					m_aPixels[2] = m_aBezier[15];
					m_aPixels[3] = m_aBezier[12];
				}
			}
			else if (m_cData.eMode == CEditToolDataTransformation::EMSimple)
			{
				TPixelCoords aPts[4];
				if (cData.eMode == CEditToolDataTransformation::EMPerspective)
				{
					aPts[0] = m_aPixels[0];
					aPts[1] = m_aPixels[1];
					aPts[2] = m_aPixels[3];
					aPts[3] = m_aPixels[2];
				}
				else if (cData.eMode == CEditToolDataTransformation::EMBezier)
				{
					aPts[0] = m_aBezier[0];
					aPts[1] = m_aBezier[3];
					aPts[2] = m_aBezier[12];
					aPts[3] = m_aBezier[15];
				}
				m_tCenter.fX = 0.25f*(aPts[0].fX + aPts[1].fX + aPts[2].fX + aPts[3].fX);
				m_tCenter.fY = 0.25f*(aPts[0].fY + aPts[1].fY + aPts[2].fY + aPts[3].fY);

				float f = sqrtf(fabsf(0.5f*(aPts[3].fX-aPts[0].fX)*(aPts[1].fY-aPts[2].fY)-0.5f*(aPts[3].fY-aPts[0].fY)*(aPts[1].fX-aPts[2].fX)));
				float fAspectLock;
				if (m_cData.bAspectLock)
				{
					fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
				}
				else
				{
					float fX = 0.5f*(
						sqrtf((aPts[1].fX-aPts[0].fX)*(aPts[1].fX-aPts[0].fX)+(aPts[1].fY-aPts[0].fY)*(aPts[1].fY-aPts[0].fY))+
						sqrtf((aPts[3].fX-aPts[2].fX)*(aPts[3].fX-aPts[2].fX)+(aPts[3].fY-aPts[2].fY)*(aPts[3].fY-aPts[2].fY)));
					float fY = 0.5f*(
						sqrtf((aPts[0].fX-aPts[2].fX)*(aPts[0].fX-aPts[2].fX)+(aPts[0].fY-aPts[2].fY)*(aPts[0].fY-aPts[2].fY))+
						sqrtf((aPts[3].fX-aPts[1].fX)*(aPts[3].fX-aPts[1].fX)+(aPts[3].fY-aPts[1].fY)*(aPts[3].fY-aPts[1].fY)));
					fAspectLock = fY/fX;
				}
				m_fAngle = atan2f(aPts[1].fY-aPts[0].fY, aPts[1].fX-aPts[0].fX);

				m_fSizeX = f/sqrtf(fAspectLock);
				m_fSizeY = f*sqrtf(fAspectLock);
				if (m_eCoordinatesMode == ECMIntegral)
				{
					m_fSizeX = floorf(m_fSizeX+0.5f);
					m_fSizeY = floorf(m_fSizeY+0.5f);
				}
			}
		}
		else if (CEditToolDataTransformation::EMSimple == m_cData.eMode && m_cData.bAspectLock && !cData.bAspectLock)
		{ // aspect lock activated
			bChange = true;
			float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
			float fSizeX = 0.5f*(m_fSizeX+m_fSizeY/fAspectLock);
			float fSizeY = fSizeX*fAspectLock;
			if (m_eCoordinatesMode == ECMIntegral)
			{
				fSizeX = floorf(fSizeX+0.5f);
				fSizeY = floorf(fSizeY+0.5f);
			}
			if (fSizeX != m_fSizeX || fSizeY != m_fSizeY)
			{
				m_fSizeX = fSizeX;
				m_fSizeY = fSizeY;
				if (m_eCoordinatesMode == ECMIntegral)
				{
					if (int(fSizeX+0.5f)&1)
						m_tCenter.fX = 0.5f + floorf(m_tCenter.fX);
					else
						m_tCenter.fX = floorf(m_tCenter.fX + 0.5f);
					if (int(fSizeY+0.5f)&1)
						m_tCenter.fY = 0.5f + floorf(m_tCenter.fY);
					else
						m_tCenter.fY = floorf(m_tCenter.fY + 0.5f);
				}
			}
		}
		if (bChange)
		{
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
			ShapeChanged();
		}
		return S_OK;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		bool bBlendingChange = m_eBlendingMode != a_eBlendingMode;
		bool bRasterizationChange = m_eRasterizationMode != a_eRasterizationMode;
		bool bCoordinatesChange = m_eCoordinatesMode != a_eCoordinatesMode;

		if (!bBlendingChange && !bRasterizationChange && !bCoordinatesChange)
			return S_FALSE;
		m_eBlendingMode = a_eBlendingMode;
		m_eRasterizationMode = a_eRasterizationMode;
		m_eCoordinatesMode = a_eCoordinatesMode;
		if (m_eState == ESClean || m_cSelData.m_p == NULL)
			return S_OK;
		bool bRedrawCache = bRasterizationChange;
		if (bCoordinatesChange && AdjustRectangleCoordinates())
		{
			ULONG n;
			_GetControlPointCount(&n);
			for (ULONG i = 0; i < n; ++i)
				M_Window()->ControlPointChanged(i);
			M_Window()->ControlLinesChanged();
			bRedrawCache = true;
		}
		if (bRedrawCache)
		{
			ShapeChanged();
		}
		else if (bBlendingChange)
		{
			M_Window()->RectangleChanged(&m_rcPrev);
		}
		return S_OK;
	}

	void ShapeChanged()
	{
		RECT rcPrev = m_rcPrev;
		RECT rc = GetBoundingBox();
		m_rcPrev = rc;
		if (rc.left > rcPrev.left) rc.left = rcPrev.left;
		if (rc.top > rcPrev.top) rc.top = rcPrev.top;
		if (rc.right < rcPrev.right) rc.right = rcPrev.right;
		if (rc.bottom < rcPrev.bottom) rc.bottom = rcPrev.bottom;
		M_Window()->RectangleChanged(&rc);
	}

	HRESULT _AdjustCoordinates(EControlKeysState UNREF(a_eKeysState), TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float UNREF(a_fPointSize))
	{
		if (m_eCoordinatesMode == ECMIntegral)
		{
			a_pPos->fX = floorf(a_pPos->fX+0.5f);
			a_pPos->fY = floorf(a_pPos->fY+0.5f);
		}
		return S_OK;
	}

	static inline float sign(TPixelCoords const& p1, TPixelCoords const& p2, TPixelCoords const& p3) { return (p1.fX - p3.fX) * (p2.fY - p3.fY) - (p2.fX - p3.fX) * (p1.fY - p3.fY); }
	static bool PointInTriangle(TPixelCoords const& pt, TPixelCoords const& v1, TPixelCoords const& v2, TPixelCoords const& v3)
	{
		bool b1 = sign(pt, v1, v2) < 0.0f;
		bool b2 = sign(pt, v2, v3) < 0.0f;
		bool b3 = sign(pt, v3, v1) < 0.0f;
		return b1 == b2 && b2 == b3;
	}
	bool HitTest(float a_fX, float a_fY) const
	{
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			{
				double const fCos = cos(-m_fAngle);
				double const fSin = sin(-m_fAngle);
				double const fX = a_fX-m_tCenter.fX;
				double const fY = a_fY-m_tCenter.fY;
				double const fX2 = fX*fCos - fY*fSin;
				double const fY2 = fY*fCos + fX*fSin;
				return fabs(fX2*2.0f) <= m_fSizeX && fabs(fY2*2.0f) <= m_fSizeY;
			}
		case CEditToolDataTransformation::EMPerspective:
			{
				double angles[5] =
				{
					atan2(m_aPixels[0].fX-a_fX, m_aPixels[0].fY-a_fY),
					atan2(m_aPixels[1].fX-a_fX, m_aPixels[1].fY-a_fY),
					atan2(m_aPixels[2].fX-a_fX, m_aPixels[2].fY-a_fY),
					atan2(m_aPixels[3].fX-a_fX, m_aPixels[3].fY-a_fY),
					atan2(m_aPixels[0].fX-a_fX, m_aPixels[0].fY-a_fY),
				};
				double fi = 0.0;
				for (int i = 0; i < 4; ++i)
				{
					double delta = angles[i+1]-angles[i];
					if (delta < 3.1415926535897932 && delta > -3.1415926535897932)
						fi += delta;
					else
						fi -= delta;
				}
				return fi > 3.1415926535897932 || fi < -3.1415926535897932;
			}
		case CEditToolDataTransformation::EMBezier:
			{
				float fMinX = m_aBezier[0].fX;
				float fMinY = m_aBezier[0].fY;
				float fMaxX = m_aBezier[0].fX;
				float fMaxY = m_aBezier[0].fY;
				for (int i = 1; i < 16; ++i)
				{
					if (fMinX > m_aBezier[i].fX) fMinX = m_aBezier[i].fX;
					if (fMinY > m_aBezier[i].fY) fMinY = m_aBezier[i].fY;
					if (fMaxX < m_aBezier[i].fX) fMaxX = m_aBezier[i].fX;
					if (fMaxY < m_aBezier[i].fY) fMaxY = m_aBezier[i].fY;
				}
				if (a_fX < fMinX || a_fX > fMaxX || a_fY < fMinY || a_fY > fMaxY)
					return false;
				TPixelCoords const tPt = {a_fX, a_fY};
				int const nStepsV = 16;
				int const nStepsU = 16;
				int nV1 = 0;
				for (int v = 0; v < nStepsV; ++v)
				{
					int nV2 = float(v+1)*m_szSelSize.cy/nStepsV+0.5f;
					if (nV2 == nV1) continue;
					int nU1 = 0;
					for (int u = 0; u < nStepsU; ++u)
					{
						int nU2 = float(u+1)*m_szSelSize.cx/nStepsU+0.5f;
						if (nU2 == nU1) continue;
						TPixelCoords t0 = BezierAt(float(nU1)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy);
						TPixelCoords t1 = BezierAt(float(nU2)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy);
						TPixelCoords t2 = BezierAt(float(nU1)/m_szSelSize.cx, float(nV2)/m_szSelSize.cy);
						TPixelCoords t3 = BezierAt(float(nU2)/m_szSelSize.cx, float(nV2)/m_szSelSize.cy);
						if (PointInTriangle(tPt, t0, t1, t3) || PointInTriangle(tPt, t0, t3, t2))
							return true;
						nU1 = nU2;
					}
					nV1 = nV2;
				}
				return false;
			}
			return false;
		}
		return false;
	}
	bool UseMoveCursor(TPixelCoords const* a_pPos) const
	{
		return m_eState == ESMoving || (m_eState != ESDragging && HitTest(a_pPos->fX, a_pPos->fY));
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* a_pPos, BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return HitTest(a_pPos->fX, a_pPos->fY) ? ETPAHit|ETPATransform : ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}

	DWORD MaxIdleTime()
	{
		return 0;
	}
	HRESULT OnMouseDown(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (m_eState == ESFloating)
		{
			if (HitTest(a_pPos->fX, a_pPos->fY))
			{
				m_eState = ESMoving;
				m_tDragLast = m_tDragStart = *a_pPos;
				return S_OK;
			}
			else
			{
				m_eState = ESDragging;
				ATLASSERT(0);
			}
		}
		ResetRectangle();
		m_tDragLast = m_tDragStart = *a_pPos;
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		m_eState = ESDragging;
		return S_OK;
	}
	HRESULT OnMouseUp(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		OnMouseMove(a_eKeysState, a_pPos);
		if (m_eState == ESDragging)
		{
			// this should not be neccessary
			m_cSelData.Free();
			m_cDeleteMask.Free();
			m_bUseDeleteMask = false;
			m_szSelSize.cx = m_szSelSize.cy = 0;

			RECT rcBounds =
			{
				floorf(min(m_tDragStart.fX, m_tDragLast.fX)+0.5f),
				floorf(min(m_tDragStart.fY, m_tDragLast.fY)+0.5f),
				floorf(max(m_tDragStart.fX, m_tDragLast.fX)+0.5f),
				floorf(max(m_tDragStart.fY, m_tDragLast.fY)+0.5f)
			};
			if (rcBounds.left < 0) rcBounds.left = 0;
			if (rcBounds.top < 0) rcBounds.top = 0;
			if (rcBounds.right > LONG(m_nSizeX)) rcBounds.right = m_nSizeX;
			if (rcBounds.bottom > LONG(m_nSizeY)) rcBounds.bottom = m_nSizeY;
			if (rcBounds.left < rcBounds.right && rcBounds.top < rcBounds.bottom)
			{
				m_rcDelete = rcBounds;
				InitAsRectangle(rcBounds.left, rcBounds.top, rcBounds.right, rcBounds.bottom);
				m_rcPrev = GetBoundingBox();
				m_szSelSize.cx = rcBounds.right-rcBounds.left;
				m_szSelSize.cy = rcBounds.bottom-rcBounds.top;
				m_cSelData.Allocate(m_szSelSize.cx*m_szSelSize.cy);
				M_Window()->GetImageTile(rcBounds.left, rcBounds.top, rcBounds.right-rcBounds.left, rcBounds.bottom-rcBounds.top, 0.0f, rcBounds.right-rcBounds.left, EITIContent, m_cSelData);
				M_Window()->RectangleChanged(&rcBounds);
				M_Window()->ControlPointsChanged();
				M_Window()->ControlLinesChanged();
				m_eState = ESFloating;
			}
			else
			{
				ResetRectangle();
			}
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
		}
		else if (m_eState == ESMoving)
		{
			m_eState = ESFloating;
		}
		return S_OK;
	}
	HRESULT OnMouseMove(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (a_pPos->fX == m_tDragLast.fX && a_pPos->fY == m_tDragLast.fY)
			return S_FALSE;
		m_tDragLast = *a_pPos;
		if (m_eState == ESDragging)
		{
			M_Window()->ControlLinesChanged();
			return S_OK;
		}
		else if (m_eState == ESMoving)
		{
			ULONG nMax = 0;
			switch (m_cData.eMode)
			{
			case CEditToolDataTransformation::EMSimple:
				m_tCenter.fX += a_pPos->fX-m_tDragStart.fX;
				m_tCenter.fY += a_pPos->fY-m_tDragStart.fY;
				nMax = 9;
				break;
			case CEditToolDataTransformation::EMPerspective:
				for (ULONG j = 0; j < 4; ++j)
				{
					m_aPixels[j].fX += a_pPos->fX-m_tDragStart.fX;
					m_aPixels[j].fY += a_pPos->fY-m_tDragStart.fY;
				}
				nMax = 8;
				break;
			case CEditToolDataTransformation::EMBezier:
				for (ULONG j = 0; j < 16; ++j)
				{
					m_aBezier[j].fX += a_pPos->fX-m_tDragStart.fX;
					m_aBezier[j].fY += a_pPos->fY-m_tDragStart.fY;
				}
				nMax = 16;
				break;
			}
			m_tDragStart = *a_pPos;
			for (ULONG i = 0; i < nMax; ++i) M_Window()->ControlPointChanged(i);
			M_Window()->ControlLinesChanged();
			ShapeChanged();
			return S_OK;
		}
		return S_OK;
	}
	HRESULT OnMouseLeave()
	{
		if (m_eState == ESDragging)
		{
			m_tDragLast = m_tDragStart;
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
		}
		return S_OK;
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = 0;
		if (m_eState > ESDragging) switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMBezier:		*a_pCount = 16; break;
		case CEditToolDataTransformation::EMPerspective:*a_pCount = 8; break;
		case CEditToolDataTransformation::EMSimple:		*a_pCount = 9; break;
		}
		return S_OK;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (m_cSelData.m_p == NULL || m_eState <= ESDragging)
			return E_RW_INDEXOUTOFRANGE;

		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			if (a_nIndex > 8)
				return E_RW_INDEXOUTOFRANGE;
			if (a_nIndex == 8)
			{
				a_pPos->fX = m_tCenter.fX + 0.35f*m_fSizeX*cosf(m_fAngle);
				a_pPos->fY = m_tCenter.fY + 0.35f*m_fSizeX*sinf(m_fAngle);
				*a_pClass = 2;
				return S_OK;
			}
			{
				TPixelCoords aPts[4];
				SimpleToPts(aPts);
				if (a_nIndex&1)
				{
					a_pPos->fX = 0.5f*(aPts[a_nIndex>>1].fX+aPts[((a_nIndex>>1)+1)&3].fX);
					a_pPos->fY = 0.5f*(aPts[a_nIndex>>1].fY+aPts[((a_nIndex>>1)+1)&3].fY);
					*a_pClass = 1;
				}
				else
				{
					a_pPos->fX = aPts[a_nIndex>>1].fX;
					a_pPos->fY = aPts[a_nIndex>>1].fY;
					*a_pClass = 0;
				}
				return S_OK;
			}
		case CEditToolDataTransformation::EMPerspective:
			if (a_nIndex >= 8)
				return E_RW_INDEXOUTOFRANGE;
			if (a_nIndex&1)
			{
				a_pPos->fX = 0.5f*(m_aPixels[a_nIndex>>1].fX+m_aPixels[((a_nIndex>>1)+1)&3].fX);
				a_pPos->fY = 0.5f*(m_aPixels[a_nIndex>>1].fY+m_aPixels[((a_nIndex>>1)+1)&3].fY);
				*a_pClass = 1;
			}
			else
			{
				a_pPos->fX = m_aPixels[a_nIndex>>1].fX;
				a_pPos->fY = m_aPixels[a_nIndex>>1].fY;
				*a_pClass = 0;
			}
			return S_OK;
		case CEditToolDataTransformation::EMBezier:
			if (a_nIndex >= 16)
				return E_RW_INDEXOUTOFRANGE;
			*a_pPos = IntersectingPoints() ? BezierAt((a_nIndex&3)/3.0, (a_nIndex>>2)/3.0) : m_aBezier[a_nIndex];
			*a_pClass = 0;
			return S_OK;
		}
		return E_UNEXPECTED;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMPerspective:
			if (a_nIndex >= 8)
				return E_RW_INDEXOUTOFRANGE;
			if (a_nIndex&1)
			{
				ULONG nPt1 = a_nIndex>>1;
				ULONG nPt2 = ((a_nIndex+1)>>1)&3;
				float fPX = 0.5f*(m_aPixels[nPt1].fX+m_aPixels[nPt2].fX);
				float fPY = 0.5f*(m_aPixels[nPt1].fY+m_aPixels[nPt2].fY);
				m_aPixels[nPt1].fX += a_pPos->fX-fPX;
				m_aPixels[nPt2].fX += a_pPos->fX-fPX;
				m_aPixels[nPt1].fY += a_pPos->fY-fPY;
				m_aPixels[nPt2].fY += a_pPos->fY-fPY;
				for (ULONG i = a_nIndex+6; i <= a_nIndex+10; ++i)
					M_Window()->ControlPointChanged(i&7);
			}
			else
			{
				m_aPixels[a_nIndex>>1] = *a_pPos;
				for (ULONG i = a_nIndex+7; i <= a_nIndex+9; ++i)
					M_Window()->ControlPointChanged(i&7);
			}
			M_Window()->ControlLinesChanged();
			ShapeChanged();
			break;
		case CEditToolDataTransformation::EMSimple:
			if (a_nIndex >= 9)
				return E_RW_INDEXOUTOFRANGE;
			{
				float const fX = a_pPos->fX-m_tCenter.fX;
				float const fY = a_pPos->fY-m_tCenter.fY;
				if (a_nIndex == 8)
				{
					float fAngle = atan2(fY, fX);
					if (m_eCoordinatesMode == ECMIntegral)
						fAngle = eStep*floorf(fAngle*180.0f/3.14159265f/eStep+0.5f)/180.0f*3.14159265f; // eStep degrees steps in integral mode
					if (fabsf(fAngle-m_fAngle) > 1e-4f)
					{
						m_fAngle = fAngle;
						for (ULONG i = 0; i < 9; ++i)
							M_Window()->ControlPointChanged(i);
						M_Window()->ControlLinesChanged();
						ShapeChanged();
					}
				}
				else
				{
					BYTE bChange = 0;
					// 0 1 2
					// 7   3
					// 6 5 4
					TPixelCoords tDirX = { cosf(m_fAngle),  sinf(m_fAngle)};
					TPixelCoords tDirY = {-sinf(m_fAngle),  cosf(m_fAngle)};
					TPixelCoords aPts[4];
					SimpleToPts(aPts);
					if (a_nIndex&1)
					{
						ULONG nPtL1 = ((a_nIndex-1)>>1)&3;
						ULONG nPtR1 = (nPtL1+1)&3;
						ULONG nPtL2 = (nPtL1-1)&3;
						ULONG nPtR2 = (nPtR1+1)&3;
						TPixelCoords tRef = {0.5f*(aPts[nPtL2].fX+aPts[nPtR2].fX), 0.5f*(aPts[nPtL2].fY+aPts[nPtR2].fY)};
						TPixelCoords tD = {a_pPos->fX-tRef.fX, a_pPos->fY-tRef.fY};
						if (nPtR1&1)
						{
							float fD = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
							tD.fX = tDirY.fX*fD;
							tD.fY = tDirY.fY*fD;
							m_fSizeY = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
						}
						else
						{
							float fD = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
							tD.fX = tDirX.fX*fD;
							tD.fY = tDirX.fY*fD;
							m_fSizeX = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY);
						}
						aPts[nPtL1].fX = aPts[nPtL2].fX + tD.fX;
						aPts[nPtL1].fY = aPts[nPtL2].fY + tD.fY;
						aPts[nPtR1].fX = aPts[nPtR2].fX + tD.fX;
						aPts[nPtR1].fY = aPts[nPtR2].fY + tD.fY;
						m_tCenter.fX = tRef.fX+tD.fX*0.5f;
						m_tCenter.fY = tRef.fY+tD.fY*0.5f;
						if (m_cData.bAspectLock)
						{
							float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
							if (nPtR1&1)
							{
								m_fSizeX = m_fSizeY/fAspectLock;
								if (m_eCoordinatesMode == ECMIntegral)
								{
									aPts[nPtR1].fX = floorf(aPts[nPtL1].fX - tD.fY/fAspectLock+0.5f);
									aPts[nPtR1].fY = floorf(aPts[nPtL1].fY + tD.fX/fAspectLock+0.5f);
									aPts[nPtR2].fX = floorf(aPts[nPtL2].fX - tD.fY/fAspectLock+0.5f);
									aPts[nPtR2].fY = floorf(aPts[nPtL2].fY + tD.fX/fAspectLock+0.5f);
								}
								else
								{
									aPts[nPtR1].fX = aPts[nPtL1].fX - tD.fY/fAspectLock;
									aPts[nPtR1].fY = aPts[nPtL1].fY + tD.fX/fAspectLock;
									aPts[nPtR2].fX = aPts[nPtL2].fX - tD.fY/fAspectLock;
									aPts[nPtR2].fY = aPts[nPtL2].fY + tD.fX/fAspectLock;
								}
							}
							else
							{
								m_fSizeY = m_fSizeX*fAspectLock;
								if (m_eCoordinatesMode == ECMIntegral)
								{
									aPts[nPtR1].fX = floorf(aPts[nPtL1].fX - tD.fY*fAspectLock+0.5f);
									aPts[nPtR1].fY = floorf(aPts[nPtL1].fY + tD.fX*fAspectLock+0.5f);
									aPts[nPtR2].fX = floorf(aPts[nPtL2].fX - tD.fY*fAspectLock+0.5f);
									aPts[nPtR2].fY = floorf(aPts[nPtL2].fY + tD.fX*fAspectLock+0.5f);
								}
								else
								{
									aPts[nPtR1].fX = aPts[nPtL1].fX - tD.fY*fAspectLock;
									aPts[nPtR1].fY = aPts[nPtL1].fY + tD.fX*fAspectLock;
									aPts[nPtR2].fX = aPts[nPtL2].fX - tD.fY*fAspectLock;
									aPts[nPtR2].fY = aPts[nPtL2].fY + tD.fX*fAspectLock;
								}
							}
							m_tCenter.fX = 0.25f*(aPts[0].fX + aPts[1].fX + aPts[2].fX + aPts[3].fX);
							m_tCenter.fY = 0.25f*(aPts[0].fY + aPts[1].fY + aPts[2].fY + aPts[3].fY);
						}
					}
					else
					{
						ULONG nPt1 = a_nIndex>>1;
						ULONG nPt2 = (nPt1+2)&3;
						aPts[nPt1] = *a_pPos;
						TPixelCoords tD = {aPts[nPt2].fX-aPts[nPt1].fX, aPts[nPt2].fY-aPts[nPt1].fY};
						float fDX = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
						float fDY = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
						if (m_cData.bAspectLock)
						{
							float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
							if (fabsf(fabsf(fDX)*fAspectLock-fabsf(fDY)) > 1e-4f)
							{
								float fSize = min(fabsf(fDX)*fAspectLock, fabsf(fDY));//sqrtf(tD.fX*tD.fX+tD.fY*tD.fY)/sqrtf(2.0f);
								if (m_eCoordinatesMode == ECMIntegral)
								{
									fDX = fDX < 0.0f ? floorf(-fSize/fAspectLock+0.5f) : floorf(fSize/fAspectLock+0.5f);
									fDY = fDY < 0.0f ? floorf(-fSize+0.5f) : floorf(fSize+0.5f);
								}
								else
								{
									fDX = fDX < 0.0f ? -fSize/fAspectLock : fSize/fAspectLock;
									fDY = fDY < 0.0f ? -fSize : fSize;
								}
								aPts[nPt1].fX = aPts[nPt2].fX - tDirX.fX*fDX - tDirY.fX*fDY;
								aPts[nPt1].fY = aPts[nPt2].fY - tDirX.fY*fDX - tDirY.fY*fDY;
							}
							//tD.fX = aPts[nPt2].fX-aPts[nPt1].fX;
							//tD.fY = aPts[nPt2].fY-aPts[nPt1].fY;
						}
						TPixelCoords tDX = {tDirX.fX*fDX, tDirX.fY*fDX};
						TPixelCoords tDY = {tDirY.fX*fDY, tDirY.fY*fDY};
						aPts[nPt1^3].fX = aPts[nPt2].fX - tDX.fX;
						aPts[nPt1^3].fY = aPts[nPt2].fY - tDX.fY;
						aPts[nPt1^1].fX = aPts[nPt2].fX - tDY.fX;
						aPts[nPt1^1].fY = aPts[nPt2].fY - tDY.fY;
						m_tCenter.fX = 0.5f*(aPts[nPt1].fX + aPts[nPt2].fX);
						m_tCenter.fY = 0.5f*(aPts[nPt1].fY + aPts[nPt2].fY);
						m_fSizeX = fabsf(fDX);
						m_fSizeY = fabsf(fDY);
					}
					// if the rectangle would have negative size, move it instead
					{
						float fSizeX = (aPts[1].fX-aPts[0].fX)*tDirX.fX + (aPts[1].fY-aPts[0].fY)*tDirX.fY;
						if (fSizeX < 0.0f)
						{
							ULONG n = ((a_nIndex-2)>>2)&1;
							aPts[0^n] = aPts[1^n];
							aPts[3^n] = aPts[2^n];
							m_tCenter.fX = 0.5f*(aPts[1].fX + aPts[2].fX);
							m_tCenter.fY = 0.5f*(aPts[1].fY + aPts[2].fY);
							m_fSizeX = 0.0f;
						}
						float fSizeY = (aPts[3].fX-aPts[0].fX)*tDirY.fX + (aPts[3].fY-aPts[0].fY)*tDirY.fY;
						if (fSizeY < 0.0f)
						{
							ULONG n = ((a_nIndex>>2)&1)*3;
							aPts[3^n] = aPts[0^n];
							aPts[2^n] = aPts[1^n];
							m_tCenter.fX = 0.5f*(aPts[1].fX + aPts[0].fX);
							m_tCenter.fY = 0.5f*(aPts[1].fY + aPts[0].fY);
							m_fSizeY = 0.0f;
						}
					}
					bChange = 0xff; // TODO: optimize

					for (int i = 0; bChange; bChange>>=1, ++i)
						if (bChange&1)
							M_Window()->ControlPointChanged(i);
					M_Window()->ControlPointChanged(8);
					M_Window()->ControlLinesChanged();
					ShapeChanged();
					return S_OK;
				}
				break;
			}
		case CEditToolDataTransformation::EMBezier:
			if (IntersectingPoints())
			{
				TPixelCoords aPts[16];
				for (int y = 0; y < 4; ++y)
					for (int x = 0; x < 4; ++x)
						aPts[x+y*4] = BezierAt(x/3.0, y/3.0);
				//if (a_nIndex == 0 || a_nIndex == 3 || a_nIndex == 12 || a_nIndex == 15)
				//{
				//	double d1[8] = {aPts[0].fX, aPts[0].fY, aPts[3].fX, aPts[3].fY, aPts[15].fX, aPts[15].fY, aPts[12].fX, aPts[12].fY};
				//	aPts[a_nIndex] = *a_pPos;
				//	double d2[8] = {aPts[0].fX, aPts[0].fY, aPts[3].fX, aPts[3].fY, aPts[15].fX, aPts[15].fY, aPts[12].fX, aPts[12].fY};
				//	agg::trans_perspective tr(d1, d2);
				//	for (ULONG i = 0; i < 16; ++i)
				//	{
				//		if (i == a_nIndex) continue;
				//		if (i == 0 || i == 3 || i == 12 || i == 15) continue;
				//		//if (((i&3)^(a_nIndex&3)) == 3 || ((i&12)^(a_nIndex&12)) == 12) continue;
				//		double x = aPts[i].fX;
				//		double y = aPts[i].fY;
				//		tr.transform(&x, &y);
				//		aPts[i].fX = x;
				//		aPts[i].fY = y;
				//	}
				//}
				//else
				{
					aPts[a_nIndex] = *a_pPos;
				}
				for (int x = 0; x < 4; ++x)
					PointsToBezier(aPts[x*4], aPts[x*4+1], aPts[x*4+2], aPts[x*4+3]);
				for (int y = 0; y < 4; ++y)
					PointsToBezier(aPts[y], aPts[y+4], aPts[y+8], aPts[y+12]);
				std::copy(aPts, aPts+16, m_aBezier);
				//for (ULONG i = 0; i < 16; ++i) M_Window()->ControlPointChanged(i);
				M_Window()->ControlPointChanged(a_nIndex);
			}
			else
			{
				m_aBezier[a_nIndex] = *a_pPos;
				M_Window()->ControlPointChanged(a_nIndex);
			}
			M_Window()->ControlLinesChanged();
			ShapeChanged();
		}
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_eState == ESDragging)
		{
			int nX1 = floorf(m_tDragStart.fX+0.5f);
			int nY1 = floorf(m_tDragStart.fY+0.5f);
			int nX2 = floorf(m_tDragLast.fX+0.5f);
			int nY2 = floorf(m_tDragLast.fY+0.5f);
			if (a_nLineTypes&ECLTHelp && (nX1 != nX2 || nY1 != nY2))
			{
				a_pLines->MoveTo(nX1, nY1);
				a_pLines->LineTo(nX2, nY1);
				a_pLines->MoveTo(nX1, nY1);
				a_pLines->LineTo(nX1, nY2);
				a_pLines->MoveTo(nX2, nY2);
				a_pLines->LineTo(nX2, nY1);
				a_pLines->MoveTo(nX2, nY2);
				a_pLines->LineTo(nX1, nY2);
			}
			return S_OK;
		}
		if (m_eState == ESClean || m_cSelData.m_p == NULL)
			return S_OK;
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			if (a_nLineTypes&ECLTHelp)
			{
				TPixelCoords aPts[4];
				SimpleToPts(aPts);
				a_pLines->MoveTo(aPts[0].fX, aPts[0].fY);
				a_pLines->LineTo(aPts[1].fX, aPts[1].fY);
				a_pLines->MoveTo(aPts[0].fX, aPts[0].fY);
				a_pLines->LineTo(aPts[3].fX, aPts[3].fY);
				a_pLines->MoveTo(aPts[2].fX, aPts[2].fY);
				a_pLines->LineTo(aPts[1].fX, aPts[1].fY);
				a_pLines->MoveTo(aPts[2].fX, aPts[2].fY);
				a_pLines->LineTo(aPts[3].fX, aPts[3].fY);

				TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle), m_fSizeX*sinf(m_fAngle)};
				a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY);
				a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
				a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
				for (int i = 0; i < 5; ++i)
				{
					TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle+i*0.04f), m_fSizeX*sinf(m_fAngle+i*0.04f)};
					a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
				}
				a_pLines->MoveTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
				for (int i = 0; i < 5; ++i)
				{
					TPixelCoords const tDirX = {m_fSizeX*cosf(m_fAngle-i*0.04f), m_fSizeX*sinf(m_fAngle-i*0.04f)};
					a_pLines->LineTo(m_tCenter.fX+0.35f*tDirX.fX, m_tCenter.fY+0.35f*tDirX.fY);
				}
			}
			break;
		case CEditToolDataTransformation::EMPerspective:
			if (a_nLineTypes&ECLTHelp)
			{
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->MoveTo(m_aPixels[0].fX, m_aPixels[0].fY);
				a_pLines->LineTo(m_aPixels[3].fX, m_aPixels[3].fY);
				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[1].fX, m_aPixels[1].fY);
				a_pLines->MoveTo(m_aPixels[2].fX, m_aPixels[2].fY);
				a_pLines->LineTo(m_aPixels[3].fX, m_aPixels[3].fY);
			}
			break;
		case CEditToolDataTransformation::EMBezier:
			if (a_nLineTypes&ECLTHelp)
			{
				if (IntersectingPoints())
				{
					TPixelCoords aPts[16];
					for (int y = 0; y < 4; ++y)
						for (int x = 0; x < 4; ++x)
							aPts[x+y*4] = BezierAt(x/3.0, y/3.0);
					double x = 0.0;
					double y = 0.0;
					bool bFirst = true;
					{
						TPixelCoords aSub[4] = {aPts[4], aPts[5], aPts[6], aPts[7]};
						PointsToBezier(aSub[0], aSub[1], aSub[2], aSub[3]);
						agg::curve4 c(aSub[0].fX, aSub[0].fY, aSub[1].fX, aSub[1].fY, aSub[2].fX, aSub[2].fY, aSub[3].fX, aSub[3].fY);
						while (!agg::is_stop(c.vertex(&x, &y)))
							if (bFirst) { a_pLines->MoveTo(x, y); bFirst = false; } else { a_pLines->LineTo(x, y); }
					}
					bFirst = true;
					{
						TPixelCoords aSub[4] = {aPts[8], aPts[9], aPts[10], aPts[11]};
						PointsToBezier(aSub[0], aSub[1], aSub[2], aSub[3]);
						agg::curve4 c(aSub[0].fX, aSub[0].fY, aSub[1].fX, aSub[1].fY, aSub[2].fX, aSub[2].fY, aSub[3].fX, aSub[3].fY);
						while (!agg::is_stop(c.vertex(&x, &y)))
							if (bFirst) { a_pLines->MoveTo(x, y); bFirst = false; } else { a_pLines->LineTo(x, y); }
					}
					bFirst = true;
					{
						TPixelCoords aSub[4] = {aPts[1], aPts[5], aPts[9], aPts[13]};
						PointsToBezier(aSub[0], aSub[1], aSub[2], aSub[3]);
						agg::curve4 c(aSub[0].fX, aSub[0].fY, aSub[1].fX, aSub[1].fY, aSub[2].fX, aSub[2].fY, aSub[3].fX, aSub[3].fY);
						while (!agg::is_stop(c.vertex(&x, &y)))
							if (bFirst) { a_pLines->MoveTo(x, y); bFirst = false; } else { a_pLines->LineTo(x, y); }
					}
					bFirst = true;
					{
						TPixelCoords aSub[4] = {aPts[2], aPts[6], aPts[10], aPts[14]};
						PointsToBezier(aSub[0], aSub[1], aSub[2], aSub[3]);
						agg::curve4 c(aSub[0].fX, aSub[0].fY, aSub[1].fX, aSub[1].fY, aSub[2].fX, aSub[2].fY, aSub[3].fX, aSub[3].fY);
						while (!agg::is_stop(c.vertex(&x, &y)))
							if (bFirst) { a_pLines->MoveTo(x, y); bFirst = false; } else { a_pLines->LineTo(x, y); }
					}
				}
				else
				{
					a_pLines->MoveTo(m_aBezier[0].fX, m_aBezier[0].fY);
					a_pLines->LineTo(m_aBezier[1].fX, m_aBezier[1].fY);
					a_pLines->MoveTo(m_aBezier[0].fX, m_aBezier[0].fY);
					a_pLines->LineTo(m_aBezier[4].fX, m_aBezier[4].fY);
					
					a_pLines->MoveTo(m_aBezier[3].fX, m_aBezier[3].fY);
					a_pLines->LineTo(m_aBezier[2].fX, m_aBezier[2].fY);
					a_pLines->MoveTo(m_aBezier[3].fX, m_aBezier[3].fY);
					a_pLines->LineTo(m_aBezier[7].fX, m_aBezier[7].fY);

					a_pLines->MoveTo(m_aBezier[12].fX, m_aBezier[12].fY);
					a_pLines->LineTo(m_aBezier[13].fX, m_aBezier[13].fY);
					a_pLines->MoveTo(m_aBezier[12].fX, m_aBezier[12].fY);
					a_pLines->LineTo(m_aBezier[8].fX, m_aBezier[8].fY);

					a_pLines->MoveTo(m_aBezier[15].fX, m_aBezier[15].fY);
					a_pLines->LineTo(m_aBezier[14].fX, m_aBezier[14].fY);
					a_pLines->MoveTo(m_aBezier[15].fX, m_aBezier[15].fY);
					a_pLines->LineTo(m_aBezier[11].fX, m_aBezier[11].fY);
				}
			}
			if (a_nLineTypes&ECLTSelection)
			{
				double x = 0.0;
				double y = 0.0;
				bool bFirst = true;
				{
					agg::curve4 c(m_aBezier[0].fX, m_aBezier[0].fY, m_aBezier[1].fX, m_aBezier[1].fY, m_aBezier[2].fX, m_aBezier[2].fY, m_aBezier[3].fX, m_aBezier[3].fY);
					while (!agg::is_stop(c.vertex(&x, &y)))
						if (bFirst) { a_pLines->MoveTo(x, y); bFirst = false; } else { a_pLines->LineTo(x, y); }
				}
				bFirst = true;
				{
					agg::curve4 c(m_aBezier[3].fX, m_aBezier[3].fY, m_aBezier[7].fX, m_aBezier[7].fY, m_aBezier[11].fX, m_aBezier[11].fY, m_aBezier[15].fX, m_aBezier[15].fY);
					while (!agg::is_stop(c.vertex(&x, &y)))
						if (bFirst) { bFirst = false; } else { a_pLines->LineTo(x, y); }
				}
				bFirst = true;
				{
					agg::curve4 c(m_aBezier[15].fX, m_aBezier[15].fY, m_aBezier[14].fX, m_aBezier[14].fY, m_aBezier[13].fX, m_aBezier[13].fY, m_aBezier[12].fX, m_aBezier[12].fY);
					while (!agg::is_stop(c.vertex(&x, &y)))
						if (bFirst) { bFirst = false; } else { a_pLines->LineTo(x, y); }
				}
				bFirst = true;
				{
					agg::curve4 c(m_aBezier[12].fX, m_aBezier[12].fY, m_aBezier[8].fX, m_aBezier[8].fY, m_aBezier[4].fX, m_aBezier[4].fY, m_aBezier[0].fX, m_aBezier[0].fY);
					while (!agg::is_stop(c.vertex(&x, &y)))
						if (bFirst) { bFirst = false; } else { a_pLines->LineTo(x, y); }
				}
			}
			break;
		}
		return S_OK;
	}
	static void inline ProcessCurve(RECT& rc, agg::curve4& c)
	{
		double x;
		double y;
		while (!agg::is_stop(c.vertex(&x, &y)))
		{
			LONG x1 = floor(x); if (x1 < rc.left) rc.left = x1;
			LONG y1 = floor(y); if (y1 < rc.top) rc.top = y1;
			LONG x2 = ceil(x); if (x2 > rc.right) rc.right = x2;
			LONG y2 = ceil(y); if (y2 > rc.bottom) rc.bottom = y2;
		}
	}
	RECT GetBoundingBox() const
	{
		if (m_cSelData.m_p == NULL || m_eState <= ESDragging)
			return RECT_EMPTY;
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			{
				float const fCos = cosf(m_fAngle);
				float const fSin = sinf(m_fAngle);
				float const fXCos = m_fSizeX*0.5f*fCos;
				float const fXSin = m_fSizeX*0.5f*fSin;
				float const fYCos = m_fSizeY*0.5f*fCos;
				float const fYSin = m_fSizeY*0.5f*fSin;
				float const fX1 = fabsf(fXCos + fYSin);
				float const fX2 = fabsf(fXCos - fYSin);
				float const fY1 = fabsf(fXSin + fYCos);
				float const fY2 = fabsf(fXSin - fYCos);
				RECT const rc = {floorf(m_tCenter.fX-(fX1 > fX2 ? fX1 : fX2)), floorf(m_tCenter.fY-(fY1 > fY2 ? fY1 : fY2)), ceilf(m_tCenter.fX+(fX1 > fX2 ? fX1 : fX2)), ceilf(m_tCenter.fY+(fY1 > fY2 ? fY1 : fY2))};
				return rc;
			}
		case CEditToolDataTransformation::EMPerspective:
			{
				TPixelCoords tMin = m_aPixels[0];
				TPixelCoords tMax = m_aPixels[0];
				for (int i = 1; i < 4; ++i)
				{
					if (tMin.fX > m_aPixels[i].fX) tMin.fX = m_aPixels[i].fX;
					if (tMin.fY > m_aPixels[i].fY) tMin.fY = m_aPixels[i].fY;
					if (tMax.fX < m_aPixels[i].fX) tMax.fX = m_aPixels[i].fX;
					if (tMax.fY < m_aPixels[i].fY) tMax.fY = m_aPixels[i].fY;
				}
				RECT const rc = {floorf(tMin.fX), floorf(tMin.fY), ceilf(tMax.fX), ceilf(tMax.fY)};
				return rc;
			}
		case CEditToolDataTransformation::EMBezier:
			{
				RECT rc = {floorf(m_aBezier[0].fX), floorf(m_aBezier[0].fY), ceilf(m_aBezier[0].fX), ceilf(m_aBezier[0].fY)};
				int const nStepsV = 16;
				int const nStepsU = 16;
				int nV1 = 0;
				for (int v = 0; v <= nStepsV; ++v)
				{
					int nV1 = float(v)*m_szSelSize.cy/nStepsV+0.5f;
					for (int u = 0; u <= nStepsU; ++u)
					{
						int nU1 = float(u)*m_szSelSize.cx/nStepsU+0.5f;
						TPixelCoords t = BezierAt(float(nU1)/m_szSelSize.cx, float(nV1)/m_szSelSize.cy);
						LONG x1 = floor(t.fX); if (x1 < rc.left) rc.left = x1;
						LONG y1 = floor(t.fY); if (y1 < rc.top) rc.top = y1;
						LONG x2 = ceil(t.fX); if (x2 > rc.right) rc.right = x2;
						LONG y2 = ceil(t.fY); if (y2 > rc.bottom) rc.bottom = y2;
					}
				}
				return rc;
			}
		}
		return RECT_EMPTY;
	}

	// IRasterImageEditToolScripting
public:
	void InitScriptSel(int a_nX1 = 0, int a_nY1 = 0, int a_nX2 = 0x7ffffff, int a_nY2 = 0x7ffffff)
	{
		if (a_nX1 > a_nX2)
		{
			int n = a_nX1;
			a_nX1 = a_nX2;
			a_nX2 = n;
		}
		if (a_nY1 > a_nY2)
		{
			int n = a_nY1;
			a_nY1 = a_nY2;
			a_nY2 = n;
		}
		if (a_nX1 < 0) a_nX1 = 0;
		if (a_nY1 < 0) a_nY1 = 0;
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		M_Window()->Size(&nSizeX, &nSizeY);
		if (a_nX2 > int(nSizeX)) a_nX2 = nSizeX;
		if (a_nY2 > int(nSizeY)) a_nY2 = nSizeY;
		m_cSelData.Free();
		m_cDeleteMask.Free();
		m_bUseDeleteMask = false;
		m_szSelSize.cx = m_szSelSize.cy = 0;
		m_rcDelete.left = a_nX1;
		m_rcDelete.top = a_nY1;
		m_rcDelete.right = a_nX2;
		m_rcDelete.bottom = a_nY2;
		m_szSelSize.cx = m_rcDelete.right-m_rcDelete.left;
		m_szSelSize.cy = m_rcDelete.bottom-m_rcDelete.top;
		m_cSelData.Allocate(m_szSelSize.cx*m_szSelSize.cy);
		M_Window()->GetImageTile(a_nX1, a_nY1, a_nX2-a_nX1, a_nY2-a_nY1, 0.0f, a_nX2-a_nX1, EITIContent, m_cSelData);
	}
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			float aVals[36];
			switch (swscanf(a_bstrParams, L"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
				aVals, aVals+1, aVals+2, aVals+3, aVals+4, aVals+5, aVals+6, aVals+7,
				aVals+8, aVals+9, aVals+10, aVals+11, aVals+12, aVals+13, aVals+14, aVals+15,
				aVals+16, aVals+17, aVals+18, aVals+19, aVals+20, aVals+21, aVals+22, aVals+23,
				aVals+24, aVals+25, aVals+26, aVals+27, aVals+28, aVals+29, aVals+30, aVals+31,
				aVals+32, aVals+33, aVals+34, aVals+35))
			{
			case 2: // whole: move only
				InitScriptSel();
				m_cData.eMode = CEditToolDataTransformation::EMSimple;
				{
					ULONG nSizeX = 0;
					ULONG nSizeY = 0;
					M_Window()->Size(&nSizeX, &nSizeY);
					m_tCenter.fX = aVals[0]+nSizeX*0.5f;
					m_tCenter.fY = aVals[1]+nSizeY*0.5f;
					m_fSizeX = nSizeX;
					m_fSizeY = nSizeY;
					m_fAngle = 0.0f;
				}
				break;
			case 5: // whole: move, scale, rotate
				InitScriptSel();
				m_cData.eMode = CEditToolDataTransformation::EMSimple;
				{
					ULONG nSizeX = 0;
					ULONG nSizeY = 0;
					M_Window()->Size(&nSizeX, &nSizeY);
					m_tCenter.fX = aVals[0]+nSizeX*0.5f;
					m_tCenter.fY = aVals[1]+nSizeY*0.5f;
					m_fSizeX = aVals[2];
					m_fSizeY = aVals[3];
					m_fAngle = aVals[4]*3.141593f/180.0f;
				}
				break;
			case 8: // whole: perspective
				InitScriptSel();
				m_cData.eMode = CEditToolDataTransformation::EMPerspective;
				m_aPixels[0].fX = aVals[0];
				m_aPixels[0].fY = aVals[1];
				m_aPixels[1].fX = aVals[2];
				m_aPixels[1].fY = aVals[3];
				m_aPixels[2].fX = aVals[4];
				m_aPixels[2].fY = aVals[5];
				m_aPixels[3].fX = aVals[6];
				m_aPixels[3].fY = aVals[7];
				break;
			case 6: // rect: move only
				InitScriptSel(aVals[0]+0.5f, aVals[1]+0.5f, aVals[2]+0.5f, aVals[3]+0.5f);
				m_cData.eMode = CEditToolDataTransformation::EMSimple;
				{
					ULONG nSizeX = 0;
					ULONG nSizeY = 0;
					M_Window()->Size(&nSizeX, &nSizeY);
					m_tCenter.fX = aVals[4]+nSizeX*0.5f;
					m_tCenter.fY = aVals[5]+nSizeY*0.5f;
					m_fSizeX = abs(int(aVals[2]+0.5f)-int(aVals[0]+0.5f));
					m_fSizeY = abs(int(aVals[3]+0.5f)-int(aVals[1]+0.5f));
					m_fAngle = 0.0f;
				}
				break;
			case 9: // rect: move, scale, rotate
				InitScriptSel(aVals[0]+0.5f, aVals[1]+0.5f, aVals[2]+0.5f, aVals[3]+0.5f);
				m_cData.eMode = CEditToolDataTransformation::EMSimple;
				{
					ULONG nSizeX = 0;
					ULONG nSizeY = 0;
					M_Window()->Size(&nSizeX, &nSizeY);
					m_tCenter.fX = aVals[4]+nSizeX*0.5f;
					m_tCenter.fY = aVals[5]+nSizeY*0.5f;
					m_fSizeX = aVals[6];
					m_fSizeY = aVals[7];
					m_fAngle = aVals[8]*3.141593f/180.0f;
				}
				break;
			case 12: // rect: perspective
				InitScriptSel(aVals[0]+0.5f, aVals[1]+0.5f, aVals[2]+0.5f, aVals[3]+0.5f);
				m_cData.eMode = CEditToolDataTransformation::EMPerspective;
				m_aPixels[0].fX = aVals[4];
				m_aPixels[0].fY = aVals[5];
				m_aPixels[1].fX = aVals[6];
				m_aPixels[1].fY = aVals[7];
				m_aPixels[2].fX = aVals[8];
				m_aPixels[2].fY = aVals[9];
				m_aPixels[3].fX = aVals[10];
				m_aPixels[3].fY = aVals[11];
				break;
			case 32: // whole: bezier
				InitScriptSel();
				m_cData.eMode = CEditToolDataTransformation::EMBezier;
				CopyMemory(m_aBezier, aVals, sizeof m_aBezier);
				break;
			case 36: // rect: bezier
				InitScriptSel(aVals[0]+0.5f, aVals[1]+0.5f, aVals[2]+0.5f, aVals[3]+0.5f);
				m_cData.eMode = CEditToolDataTransformation::EMBezier;
				CopyMemory(m_aBezier, aVals+4, sizeof m_aBezier);
				break;
			default:
				return E_FAIL;
			}
			m_eState = ESFloating;
			//ShapeChanged();
			M_Window()->RectangleChanged(NULL);
			M_Window()->ControlLinesChanged();
			M_Window()->ControlPointsChanged();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		return E_NOTIMPL;
	}

	// IRasterImageEditToolFloatingSelection methods
public:
	STDMETHOD(Set)(float a_fX, float a_fY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel const* a_pBuffer, BOOL a_bModifyExisting)
	{
		if (a_bModifyExisting && a_nSizeX == m_szSelSize.cx && a_nSizeY == m_szSelSize.cy)
		{
			for (ULONG y = 0; y < a_nSizeY; ++y)
				CopyMemory(m_cSelData.m_p+y*a_nSizeX, a_pBuffer+y*a_nStride, a_nSizeX*sizeof *a_pBuffer);
			M_Window()->RectangleChanged(&m_rcPrev);
			return S_OK;
		}

		m_cSelData.Free();
		m_cSelMask.Free();
		m_bUseDeleteMask = false;
		if (!a_bModifyExisting)
		{
			m_cDeleteMask.Free();
			m_rcDelete = RECT_EMPTY;
		}
		SIZE szPrev = m_szSelSize;
		m_szSelSize.cx = a_nSizeX;
		m_szSelSize.cy = a_nSizeY;
		if (a_nSizeX && a_nSizeY && a_pBuffer)
		{
			m_cSelData.Allocate(a_nSizeX*a_nSizeY);
			for (ULONG y = 0; y < a_nSizeY; ++y)
				CopyMemory(m_cSelData.m_p+y*a_nSizeX, a_pBuffer+y*a_nStride, a_nSizeX*sizeof *a_pBuffer);
		}
		if (a_bModifyExisting)
		{
			if (m_cData.eMode == CEditToolDataTransformation::EMSimple)
			{
				m_tCenter.fX = a_fX-szPrev.cx*0.5f+a_nSizeX*0.5f;
				m_tCenter.fY = a_fY-szPrev.cy*0.5f+a_nSizeY*0.5f;
				m_fSizeX = m_fSizeX*a_nSizeX/szPrev.cx;
				m_fSizeY = m_fSizeY*a_nSizeY/szPrev.cy;
			}
			else if (m_cData.eMode == CEditToolDataTransformation::EMPerspective)
			{
				double dest[8] = {m_aPixels[0].fX, m_aPixels[0].fY, m_aPixels[1].fX, m_aPixels[1].fY, m_aPixels[2].fX, m_aPixels[2].fY, m_aPixels[3].fX, m_aPixels[3].fY};
				agg::trans_perspective tr(0, 0, szPrev.cx, szPrev.cy, dest);
				float fX = a_fX-0.25*(m_aPixels[0].fX+m_aPixels[1].fX+m_aPixels[2].fX+m_aPixels[3].fX);
				float fY = a_fY-0.25*(m_aPixels[0].fY+m_aPixels[1].fY+m_aPixels[2].fY+m_aPixels[3].fY);
				double x, y;
				x = fX; y = fY; tr.transform(&x, &y);
				m_aPixels[0].fX = x;
				m_aPixels[0].fY = y;
				x = fX+a_nSizeX; y = fY; tr.transform(&x, &y);
				m_aPixels[1].fX = x;
				m_aPixels[1].fY = y;
				x = fX+a_nSizeX; y = fY+a_nSizeY; tr.transform(&x, &y);
				m_aPixels[2].fX = x;
				m_aPixels[2].fY = y;
				x = fX; y = fY+a_nSizeY; tr.transform(&x, &y);
				m_aPixels[3].fX = x;
				m_aPixels[3].fY = y;
			}
			ShapeChanged();
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
		}
		else
		{
			switch (m_cData.eMode)
			{
			case CEditToolDataTransformation::EMSimple:
				m_tCenter.fX = a_fX+a_nSizeX*0.5f;
				m_tCenter.fY = a_fY+a_nSizeY*0.5f;
				m_fSizeX = a_nSizeX;
				m_fSizeY = a_nSizeY;
				m_fAngle = 0.0f;
				break;
			case CEditToolDataTransformation::EMPerspective:
				m_aPixels[0].fX = a_fX;
				m_aPixels[0].fY = a_fY;
				m_aPixels[1].fX = a_fX+a_nSizeX;
				m_aPixels[1].fY = a_fY;
				m_aPixels[2].fX = a_fX+a_nSizeX;
				m_aPixels[2].fY = a_fY+a_nSizeY;
				m_aPixels[3].fX = a_fX;
				m_aPixels[3].fY = a_fY+a_nSizeY;
				break;
			case CEditToolDataTransformation::EMBezier:
				{
					TPixelCoords t[4] = { {a_fX, a_fY}, {a_fX+a_nSizeX, a_fY}, {a_fX+a_nSizeX, a_fY+a_nSizeY}, {a_fX, a_fY+a_nSizeY} };
					InitBezier(t);
				}
			}
			m_eState = ESFloating;
			ShapeChanged();
			M_Window()->ControlPointsChanged();
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}
	STDMETHOD(Position)(float* a_pX, float* a_pY)
	{
		if (m_cData.eMode == CEditToolDataTransformation::EMSimple)
		{
			*a_pX = m_tCenter.fX;
			*a_pY = m_tCenter.fY;
			return S_OK;
		}
		if (m_cData.eMode == CEditToolDataTransformation::EMPerspective)
		{
			*a_pX = 0.25*(m_aPixels[0].fX+m_aPixels[1].fX+m_aPixels[2].fX+m_aPixels[3].fX);
			*a_pY = 0.25*(m_aPixels[0].fY+m_aPixels[1].fY+m_aPixels[2].fY+m_aPixels[3].fY);
			return S_OK;
		}
		if (m_cData.eMode == CEditToolDataTransformation::EMBezier)
		{
			*a_pX = 0.25*(m_aBezier[0].fX+m_aBezier[3].fX+m_aBezier[12].fX+m_aBezier[15].fX);
			*a_pY = 0.25*(m_aBezier[0].fY+m_aBezier[3].fY+m_aBezier[12].fY+m_aBezier[15].fY);
			return S_OK;
		}
		return E_NOTIMPL;
	}
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		*a_pSizeX = m_szSelSize.cx;
		*a_pSizeY = m_szSelSize.cy;
		return S_OK;
	}
	STDMETHOD(Data)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			if (LONG(a_nX+a_nSizeX) > m_szSelSize.cx || LONG(a_nY+a_nSizeY) > m_szSelSize.cy)
				return E_RW_INVALIDPARAM;
			if (m_cDeleteMask.m_p)
			{
				for (ULONG y = 0; y < a_nSizeY; ++y)
				{
					TRasterImagePixel* pD = a_pBuffer+y*a_nStride;
					TRasterImagePixel const* pS = m_cSelData.m_p+(y+a_nY)*m_szSelSize.cx+a_nX;
					BYTE const* pM = m_cDeleteMask.m_p+(y+a_nY)*m_szSelSize.cx+a_nX;
					for (ULONG x = 0; x < a_nSizeX; ++x)
					{
						pD->bA = pS->bA*ULONG(*pM)/255;
						if (pD->bA)
						{
							pD->bR = pS->bR;
							pD->bG = pS->bG;
							pD->bB = pS->bB;
						}
						else
						{
							pD->bR = pD->bG = pD->bB = 0;
						}
						++pD;
						++pS;
						++pM;
					}
				}
			}
			else
			{
				for (ULONG y = 0; y < a_nSizeY; ++y)
					CopyMemory(a_pBuffer+y*a_nStride, m_cSelData.m_p+(y+a_nY)*m_szSelSize.cx+a_nX, a_nSizeX*sizeof *m_cSelData.m_p);
			}
			return S_OK;
		}
		catch (...)
		{
			return a_pBuffer ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Delete)()
	{
		m_szSelSize.cx = m_szSelSize.cy = 0;
		m_cSelData.Free();
		m_cSelMask.Free();
		m_bUseDeleteMask = false;
		return ETPAApply;
	}
	STDMETHOD(SelectAll)()
	{
		m_cSelData.Free();
		m_cDeleteMask.Free();
		m_bUseDeleteMask = false;
		m_rcDelete.left = 0;
		m_rcDelete.top = 0;
		m_rcDelete.right = m_nSizeX;
		m_rcDelete.bottom = m_nSizeY;
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			m_tCenter.fX = (m_fSizeX = m_nSizeX)*0.5f;
			m_tCenter.fY = (m_fSizeY = m_nSizeY)*0.5f;
			m_fAngle = 0.0f;
			break;
		case CEditToolDataTransformation::EMPerspective:
			m_aPixels[0].fX = 0.0f;
			m_aPixels[0].fY = 0.0f;
			m_aPixels[1].fX = m_nSizeX;
			m_aPixels[1].fY = 0.0f;
			m_aPixels[2].fX = m_nSizeX;
			m_aPixels[2].fY = m_nSizeY;
			m_aPixels[3].fX = 0.0f;
			m_aPixels[3].fY = m_nSizeY;
			break;
		case CEditToolDataTransformation::EMBezier:
			{
				TPixelCoords t[4] = { {0.0f, 0.0f}, {m_nSizeX, 0.0f}, {m_nSizeX, m_nSizeY}, {0.0f, m_nSizeY} };
				InitBezier(t);
			}
		}
		m_rcPrev = GetBoundingBox();
		m_szSelSize.cx = m_nSizeX;
		m_szSelSize.cy = m_nSizeY;
		m_cSelData.Allocate(m_szSelSize.cx*m_szSelSize.cy);
		M_Window()->GetImageTile(0, 0, m_nSizeX, m_nSizeY, 0.0f, m_nSizeX, EITIContent, m_cSelData);
		RECT rcAll = {0, 0, m_nSizeX, m_nSizeY};
		m_eState = ESFloating;
		M_Window()->RectangleChanged(&rcAll);
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		return S_OK;
	}

	void InitBezier(TPixelCoords const* pPts)
	{
		m_aBezier[0] = pPts[0];
		m_aBezier[3] = pPts[1];
		m_aBezier[12] = pPts[3];
		m_aBezier[15] = pPts[2];
		m_aBezier[4].fX = (m_aBezier[0].fX*2 + m_aBezier[12].fX)/3;
		m_aBezier[4].fY = (m_aBezier[0].fY*2 + m_aBezier[12].fY)/3;
		m_aBezier[8].fX = (m_aBezier[0].fX + m_aBezier[12].fX*2)/3;
		m_aBezier[8].fY = (m_aBezier[0].fY + m_aBezier[12].fY*2)/3;
		m_aBezier[7].fX = (m_aBezier[3].fX*2 + m_aBezier[15].fX)/3;
		m_aBezier[7].fY = (m_aBezier[3].fY*2 + m_aBezier[15].fY)/3;
		m_aBezier[11].fX = (m_aBezier[3].fX + m_aBezier[15].fX*2)/3;
		m_aBezier[11].fY = (m_aBezier[3].fY + m_aBezier[15].fY*2)/3;
		for (int i = 0; i < 16; i+=4)
		{
			m_aBezier[i+1].fX = (m_aBezier[i].fX*2 + m_aBezier[i+3].fX)/3;
			m_aBezier[i+1].fY = (m_aBezier[i].fY*2 + m_aBezier[i+3].fY)/3;
			m_aBezier[i+2].fX = (m_aBezier[i].fX + m_aBezier[i+3].fX*2)/3;
			m_aBezier[i+2].fY = (m_aBezier[i].fY + m_aBezier[i+3].fY*2)/3;
		}
	}

	TPixelCoords BezierAt(float a_fU, float a_fV) const
	{
		float const fU3 = a_fU*a_fU*a_fU;
		float const fU2 = a_fU*a_fU*(1.0f-a_fU)*3.0f;
		float const fU1 = a_fU*(1.0f-a_fU)*(1.0f-a_fU)*3.0f;
		float const fU0 = (1.0f-a_fU)*(1.0f-a_fU)*(1.0f-a_fU);
		float const fV3 = a_fV*a_fV*a_fV;
		float const fV2 = a_fV*a_fV*(1.0f-a_fV)*3.0f;
		float const fV1 = a_fV*(1.0f-a_fV)*(1.0f-a_fV)*3.0f;
		float const fV0 = (1.0f-a_fV)*(1.0f-a_fV)*(1.0f-a_fV);
		TPixelCoords const t =
		{
			m_aBezier[0].fX*fU0*fV0 +
			m_aBezier[1].fX*fU1*fV0 +
			m_aBezier[2].fX*fU2*fV0 +
			m_aBezier[3].fX*fU3*fV0 +
			m_aBezier[4].fX*fU0*fV1 +
			m_aBezier[5].fX*fU1*fV1 +
			m_aBezier[6].fX*fU2*fV1 +
			m_aBezier[7].fX*fU3*fV1 +
			m_aBezier[8].fX*fU0*fV2 +
			m_aBezier[9].fX*fU1*fV2 +
			m_aBezier[10].fX*fU2*fV2 +
			m_aBezier[11].fX*fU3*fV2 +
			m_aBezier[12].fX*fU0*fV3 +
			m_aBezier[13].fX*fU1*fV3 +
			m_aBezier[14].fX*fU2*fV3 +
			m_aBezier[15].fX*fU3*fV3,
			m_aBezier[0].fY*fU0*fV0 +
			m_aBezier[1].fY*fU1*fV0 +
			m_aBezier[2].fY*fU2*fV0 +
			m_aBezier[3].fY*fU3*fV0 +
			m_aBezier[4].fY*fU0*fV1 +
			m_aBezier[5].fY*fU1*fV1 +
			m_aBezier[6].fY*fU2*fV1 +
			m_aBezier[7].fY*fU3*fV1 +
			m_aBezier[8].fY*fU0*fV2 +
			m_aBezier[9].fY*fU1*fV2 +
			m_aBezier[10].fY*fU2*fV2 +
			m_aBezier[11].fY*fU3*fV2 +
			m_aBezier[12].fY*fU0*fV3 +
			m_aBezier[13].fY*fU1*fV3 +
			m_aBezier[14].fY*fU2*fV3 +
			m_aBezier[15].fY*fU3*fV3
		};
		return t;
	}
	bool IntersectingPoints() const { return true; }

	void PointsToBezier(TPixelCoords const& a_t0, TPixelCoords& a_t1, TPixelCoords& a_t2, TPixelCoords const& a_t3)
	{
		TPixelCoords tTmp;
		tTmp.fX = (2*(27*a_t1.fX - 8*a_t0.fX - a_t3.fX) - (27*a_t2.fX - a_t0.fX - 8*a_t3.fX))/18.0;
		tTmp.fY = (2*(27*a_t1.fY - 8*a_t0.fY - a_t3.fY) - (27*a_t2.fY - a_t0.fY - 8*a_t3.fY))/18.0;
		a_t2.fX = (2*(27*a_t2.fX - a_t0.fX - 8*a_t3.fX) - (27*a_t1.fX - 8*a_t0.fX - a_t3.fX))/18.0;
		a_t2.fY = (2*(27*a_t2.fY - a_t0.fY - 8*a_t3.fY) - (27*a_t1.fY - 8*a_t0.fY - a_t3.fY))/18.0;
		a_t1 = tTmp;
	}

	struct CSamplerSmooth
	{
		CSamplerSmooth(SIZE a_szSelSize, TRasterImagePixel const* a_pSelData, BYTE const* a_pDeleteMask) : m_szSelSize(a_szSelSize), m_pSelData(a_pSelData), m_pDeleteMask(a_pDeleteMask) {}

		void generate(TPixelChannel* a_pColors, int a_nX, int a_nY, int a_nLen)
		{
			for (TPixelChannel* const pEnd = a_pColors+a_nLen; a_pColors != pEnd; ++a_pColors, ++a_nX)
			{
				double xx = a_nX+0.5;
				double yy = a_nY+0.5;
				m_tr.transform(&xx, &yy);
				LONG xxx = LONG(xx+1.5)-2;
				LONG yyy = LONG(yy+1.5)-2;
				a_pColors->n = 0;
				if (xxx >= -1 && xxx < m_szSelSize.cx && yyy >= -1 && yyy < m_szSelSize.cy)
				{
					ULONG nWX = 128*(xx-xxx-0.5);
					ULONG nWY = 128*(yy-yyy-0.5);
					ULONG nR = 0;
					ULONG nG = 0;
					ULONG nB = 0;
					ULONG nA = 0;
					ULONG nC = 0;
					TRasterImagePixel const* p;
					if (xxx >= 0)
					{
						if (xxx != m_szSelSize.cx-1)
						{
							if (yyy >= 0)
							{
								if (yyy != m_szSelSize.cy-1)
								{
									{
										p = m_pSelData + yyy*m_szSelSize.cx+xxx;
										ULONG const nW = (128-nWY)*(128-nWX);
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx]*nW : 255*nW;
									}
									{
										p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx;
										ULONG const nW = nWY*(128-nWX);
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nW : 255*nW;
									}
									{
										p = m_pSelData + yyy*m_szSelSize.cx+xxx+1;
										ULONG const nW = (128-nWY)*nWX;
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nW : 255*nW;
									}
									{
										p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx+1;
										ULONG const nW = nWY*nWX;
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nW : 255*nW;
									}
								}
								else
								{
									{
										p = m_pSelData + yyy*m_szSelSize.cx+xxx;
										ULONG const n = ((128-nWX)<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*(128-nWY) : 255*(128-nWX)*(128-nWY);
									}
									{
										p = m_pSelData + yyy*m_szSelSize.cx+xxx+1;
										ULONG const n = (nWX<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nWX*(128-nWY) : 255*nWX*(128-nWY);
									}
								}
							}
							else
							{
								{
									p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx;
									ULONG const n = ((128-nWX)<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*nWY : 255*(128-nWX)*nWY;
								}
								{
									p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx+1;
									ULONG const n = (nWX<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nWX*nWY : 255*nWX*nWY;
								}
							}
						}
						else
						{
							if (yyy >= 0)
							{
								if (yyy != m_szSelSize.cy-1)
								{
									{
										p = m_pSelData + yyy*m_szSelSize.cx+xxx;
										ULONG const n = ((128-nWY)<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWY)*(128-nWX) : 255*(128-nWY)*(128-nWX);
									}
									{
										p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx;
										ULONG const n = (nWY<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nWY*(128-nWX) : 255*nWY*(128-nWX);
									}
								}
								else
								{
									p = m_pSelData + yyy*m_szSelSize.cx+xxx;
									ULONG const n = ULONG(p->bA)<<14;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWX)*(128-nWY) : 255*(128-nWX)*(128-nWY);
								}
							}
							else
							{
								p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*(128-nWX)*nWY : 255*(128-nWX)*nWY;
							}
						}
					}
					else
					{
						if (yyy >= 0)
						{
							if (yyy != m_szSelSize.cy-1)
							{
								{
									p = m_pSelData + yyy*m_szSelSize.cx+xxx+1;
									ULONG const n = ((128-nWY)<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx]*(128-nWY)*nWX : 255*(128-nWY)*nWX;
								}
								{
									p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx+1;
									ULONG const n = (nWY<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx]*nWY*nWX : 255*nWY*nWX;
								}
							}
							else
							{
								p = m_pSelData + yyy*m_szSelSize.cx+xxx+1;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += m_pDeleteMask ? m_pDeleteMask[yyy*m_szSelSize.cx+xxx+1]*nWX*(128-nWY) : 255*nWX*(128-nWY);
							}
						}
						else
						{
							p = m_pSelData + (yyy+1)*m_szSelSize.cx+xxx+1;
							ULONG const n = ULONG(p->bA)<<14;
							nA += n;
							nR += n*p->bR;
							nG += n*p->bG;
							nB += n*p->bB;
							nC += m_pDeleteMask ? m_pDeleteMask[(yyy+1)*m_szSelSize.cx+xxx+1]*nWX*nWY : 255*nWX*nWY;
						}
					}
					TRasterImagePixel t = {0, 0, 0, nA>>14};
					if (t.bA)
					{
						t.bR = nR/nA;
						t.bG = nG/nA;
						t.bB = nB/nA;
					}
					*a_pColors = *reinterpret_cast<TPixelChannel const*>(&t);
				}
			}
		}

		agg::trans_affine m_tr;
		SIZE m_szSelSize;
		TRasterImagePixel const* const m_pSelData;
		BYTE const* const m_pDeleteMask;
	};
	struct CSamplerSimple
	{
		CSamplerSimple(SIZE a_szSelSize, TRasterImagePixel const* a_pSelData, BYTE const* a_pDeleteMask) : m_szSelSize(a_szSelSize), m_pSelData(a_pSelData), m_pDeleteMask(a_pDeleteMask) {}

		void generate(TPixelChannel* a_pColors, int a_nX, int a_nY, int a_nLen)
		{
			for (TPixelChannel* const pEnd = a_pColors+a_nLen; a_pColors != pEnd; ++a_pColors, ++a_nX)
			{
				double xx = a_nX+0.5;
				double yy = a_nY+0.5;
				m_tr.transform(&xx, &yy);

				LONG xxx = LONG(xx+1.0)-1;
				LONG yyy = LONG(yy+1.0)-1;
				if (xxx >= 0 && xxx < m_szSelSize.cx && yyy >= 0 && yyy < m_szSelSize.cy)
				{
					*a_pColors = *reinterpret_cast<TPixelChannel const*>(m_pSelData + yyy*m_szSelSize.cx+xxx);
					if (m_pDeleteMask)
						a_pColors->bA = ((a_pColors->bA+(ULONG(a_pColors->bA)<<7))*m_pDeleteMask[yyy*m_szSelSize.cx+xxx])>>15;
				}
			}
		}

		agg::trans_affine m_tr;
		SIZE m_szSelSize;
		TRasterImagePixel const* const m_pSelData;
		BYTE const* const m_pDeleteMask;
	};

	template<typename TSampler>
	struct CBezierBuffer : public TSampler
	{
		typedef TPixelChannel color_type;
		typedef void row_data;
		typedef void span_data;

		CBezierBuffer(TRasterImagePixel* a_pBuffer, BYTE* const a_pCover, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, SIZE a_szSelSize, TRasterImagePixel const* a_pSelData, BYTE const* a_pDeleteMask) :
			m_pBuffer(a_pBuffer), m_pCover(a_pCover), m_nX(a_nX), m_nY(a_nY), m_nSizeX(a_nSizeX), m_nSizeY(a_nSizeY),
			TSampler(a_szSelSize, a_pSelData, a_pDeleteMask)
		{
		}
		void SetTransform(agg::trans_affine const& a_tr) { m_tr = a_tr; }
		int width() const { return m_nSizeX; }
		int height() const { return m_nSizeY; }
		void prepare()
		{
		}
		void blend_color_hspan(int x, int y, unsigned len, const color_type* colors, const agg::int8u* covers, agg::int8u cover)
		{
			if (y < m_nY || y >= LONG(m_nY+m_nSizeY))
				return;
			if (LONG(x+len) < m_nX || x >= LONG(m_nX+m_nSizeX))
				return;
			if (x < m_nX)
			{
				len -= m_nX-x;
				colors += m_nX-x;
				if (covers) covers += m_nX-x;
				x = m_nX;
			}
			if (LONG(x+len) > LONG(m_nX+m_nSizeX))
				len = m_nX+m_nSizeX-x;
			TRasterImagePixel* pD = m_pBuffer+(y-m_nY)*m_nSizeX+x-m_nX;
			BYTE* pC = m_pCover+(y-m_nY)*m_nSizeX+x-m_nX;
			if (covers)
			{
				for (TRasterImagePixel* const pEnd = pD+len; pD != pEnd; ++pD, ++pC, ++colors, ++covers)
				{
					if (*pC == 255)
						continue;
					if (*pC == 0)
					{
						*pD = *reinterpret_cast<TRasterImagePixel const*>(colors);
						*pC = *covers;
					}
					else if (ULONG(*pC) + *covers <= 255)
					{
						*pC += *covers;
					}
					else
					{
						*pC = 255;
					}
				}
			}
			else
			{
				for (TRasterImagePixel* const pEnd = pD+len; pD != pEnd; ++pD, ++pC, ++colors)
				{
					if (*pC == 255)
						continue;
					if (*pC == 0)
					{
						*pD = *reinterpret_cast<TRasterImagePixel const*>(colors);
						*pC = cover;
					}
					else if (ULONG(*pC) + cover <= 255)
					{
						*pC += cover;
					}
					else
					{
						*pC = 255;
					}
				}
			}
		}

		TRasterImagePixel* const m_pBuffer;
		BYTE* const m_pCover;
		LONG m_nX;
		LONG m_nY;
		ULONG m_nSizeX;
		ULONG m_nSizeY;
	};

	static bool isConvex(double const* p)
	{
		for (int i = 0; i < 8; i+=2)
		{
			if ((p[i]-p[(i+2)&7])*(p[(i+3)&7]-p[(i+5)&7]) - (p[i+1]-p[(i+3)&7])*(p[(i+2)&7]-p[(i+4)&7]) < 0.0f)
				return false;
		}
		return true;
	}

public:
	void ResetRectangle()
	{
		m_eState = ESClean;
		m_fSizeX = m_fSizeY = m_fAngle = m_tCenter.fX = m_tCenter.fY = 0.0f;
		ZeroMemory(m_aPixels, sizeof m_aPixels);
		ZeroMemory(m_aBezier, sizeof m_aBezier);
	}
	void InitAsRectangle(LONG a_nX1, LONG a_nY1, LONG a_nX2, LONG a_nY2)
	{
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			m_fAngle = 0.0f;
			m_tCenter.fX = (a_nX1+a_nX2)*0.5f;
			m_tCenter.fY = (a_nY1+a_nY2)*0.5f;
			m_fSizeX = abs(a_nX2-a_nX1);
			m_fSizeY = abs(a_nY2-a_nY1);
			break;
		case CEditToolDataTransformation::EMPerspective:
			m_aPixels[0].fX = a_nX1;
			m_aPixels[0].fY = a_nY1;
			m_aPixels[1].fX = a_nX2;
			m_aPixels[1].fY = a_nY1;
			m_aPixels[2].fX = a_nX2;
			m_aPixels[2].fY = a_nY2;
			m_aPixels[3].fX = a_nX1;
			m_aPixels[3].fY = a_nY2;
			break;
		case CEditToolDataTransformation::EMBezier:
			{
				TPixelCoords aPts[4] = { {a_nX1, a_nY1}, {a_nX2, a_nY1}, {a_nX2, a_nY2}, {a_nX1, a_nY2} };
				InitBezier(aPts);
			}
			break;
		}
		M_Window()->ControlPointsChanged();
		M_Window()->ControlLinesChanged();
		ShapeChanged();
	}
	bool IsSimpleMove() const
	{
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			return m_fAngle == 0.0f && m_fSizeX == m_szSelSize.cx && m_fSizeY == m_szSelSize.cy &&
				(m_tCenter.fX+m_fSizeX) == int(m_tCenter.fX+m_fSizeX) && (m_tCenter.fY+m_fSizeY) == int(m_tCenter.fY+m_fSizeY);
		case CEditToolDataTransformation::EMPerspective:
			return m_aPixels[0].fX == m_aPixels[3].fX && m_aPixels[1].fX == m_aPixels[2].fX &&
				   m_aPixels[0].fY == m_aPixels[1].fY && m_aPixels[2].fY == m_aPixels[3].fY &&
				   m_aPixels[0].fX == int(m_aPixels[0].fX) && m_aPixels[0].fY == int(m_aPixels[0].fY) &&
				   m_aPixels[1].fX-m_aPixels[0].fX == m_szSelSize.cx && m_aPixels[3].fY-m_aPixels[0].fY == m_szSelSize.cy;
		case CEditToolDataTransformation::EMBezier:
			return false;
		}
		return false;
	}

	static bool Adjust(float& a_f) { float const f = floorf(a_f+0.5f); if (fabsf(f-a_f) <= 1e-4f) return false; a_f = f; return true; }
	bool AdjustRectangleCoordinates()
	{
		if (m_eCoordinatesMode != ECMIntegral)
			return false;
		bool bChange = false;
		switch (m_cData.eMode)
		{
		case CEditToolDataTransformation::EMSimple:
			{
				float const fAngle = eStep*floorf(m_fAngle*180.0f/3.14159265f/eStep+0.5f)/180.0f*3.14159265f; // eStep degrees steps in integral mode
				if (fabsf(fAngle-m_fAngle) > 1e-4f)
				{
					bChange = true;
					m_fAngle = fAngle;
				}
				TPixelCoords t0 = {m_tCenter.fX-m_fSizeX*0.5f, m_tCenter.fY-m_fSizeY*0.5f};
				TPixelCoords t1 = {m_tCenter.fX+m_fSizeX*0.5f, m_tCenter.fY+m_fSizeY*0.5f};
				if (Adjust(t0.fX) || Adjust(t0.fY) || Adjust(t1.fX) || Adjust(t1.fY))
				{
					bChange = true;
					m_tCenter.fX = 0.5f*(t0.fX+t1.fX);
					m_tCenter.fY = 0.5f*(t0.fY+t1.fY);
					m_fSizeX = t1.fX-t0.fX;
					m_fSizeY = t1.fY-t0.fY;
				}
			}
			break;
		case CEditToolDataTransformation::EMPerspective:
			bChange =
				Adjust(m_aPixels[0].fX) || Adjust(m_aPixels[0].fY) ||
				Adjust(m_aPixels[1].fX) || Adjust(m_aPixels[1].fY) ||
				Adjust(m_aPixels[2].fX) || Adjust(m_aPixels[2].fY) ||
				Adjust(m_aPixels[3].fX) || Adjust(m_aPixels[3].fY);
			break;
		case CEditToolDataTransformation::EMBezier:
			bChange =
				Adjust(m_aBezier[0].fX) || Adjust(m_aBezier[1].fX) ||
				Adjust(m_aBezier[2].fX) || Adjust(m_aBezier[3].fX) ||
				Adjust(m_aBezier[12].fX) || Adjust(m_aBezier[13].fX) ||
				Adjust(m_aBezier[14].fX) || Adjust(m_aPixels[15].fX) ||
				Adjust(m_aBezier[0].fY) || Adjust(m_aBezier[4].fY) ||
				Adjust(m_aBezier[8].fY) || Adjust(m_aBezier[12].fY) ||
				Adjust(m_aBezier[3].fY) || Adjust(m_aBezier[7].fY) ||
				Adjust(m_aBezier[11].fY) || Adjust(m_aBezier[15].fY);
			break;
		}
		return bChange;
	}


private:
	void SimpleToPts(TPixelCoords* a_aPixels) const
	{
		float const fCos = cosf(m_fAngle);
		float const fSin = sinf(m_fAngle);
		float const fXCos = m_fSizeX*0.5f*fCos;
		float const fXSin = m_fSizeX*0.5f*fSin;
		float const fYCos = m_fSizeY*0.5f*fCos;
		float const fYSin = m_fSizeY*0.5f*fSin;
		a_aPixels[0].fX = m_tCenter.fX - fXCos + fYSin;
		a_aPixels[0].fY = m_tCenter.fY - fXSin - fYCos;
		a_aPixels[1].fX = m_tCenter.fX + fXCos + fYSin;
		a_aPixels[1].fY = m_tCenter.fY + fXSin - fYCos;
		a_aPixels[2].fX = m_tCenter.fX + fXCos - fYSin;
		a_aPixels[2].fY = m_tCenter.fY + fXSin + fYCos;
		a_aPixels[3].fX = m_tCenter.fX - fXCos - fYSin;
		a_aPixels[3].fY = m_tCenter.fY - fXSin + fYCos;
	}

private:
	enum EState
	{
		ESClean = 0,
		ESDragging,
		ESFloating,
		ESMoving
	};

	enum
	{
		eStep = 5 // rotation step in integral mode
	};

private:
	EBlendingMode m_eBlendingMode;
	ERasterizationMode m_eRasterizationMode;
	ECoordinatesMode m_eCoordinatesMode;
	CEditToolDataTransformation m_cData;

	// editing
	TPixelCoords m_tDragStart;
	TPixelCoords m_tDragLast;
	EState m_eState;

	// simple transform
	TPixelCoords m_tCenter;
	float m_fSizeX;
	float m_fSizeY;
	float m_fAngle;
	// perspective transform
	TPixelCoords m_aPixels[4];
	// bezier patch coords
	TPixelCoords m_aBezier[16];

	// updating
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	RECT m_rcPrev;

	// region to delete
	RECT m_rcDelete;
	CAutoVectorPtr<BYTE> m_cDeleteMask;

	// floating selection
	SIZE m_szSelSize;
	CAutoVectorPtr<TRasterImagePixel> m_cSelData;
	CAutoVectorPtr<BYTE> m_cSelMask;
	bool m_bUseDeleteMask;
};

