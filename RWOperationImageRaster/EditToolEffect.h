
#pragma once

#include "RasterImageEditWindowCallbackHelper.h"
#include "EditToolPixelMixer.h"
#include <math.h>
#include <StringParsing.h>
#include <SubjectNotImpl.h>
#include <RWBaseEnumUtils.h>
#include <RWDocumentImageRaster.h>

//#include <boost/spirit.hpp>
//using namespace boost::spirit;


struct CEditToolDataEffect
{
	//MIDL_INTERFACE("0CB2EEB0-B5D4-40A1-A723-0B290ED0CDFA")
	//ISharedStateToolData : public ISharedState
	//{
	//public:
	//	STDMETHOD_(CEditToolDataEffect const*, InternalData)() = 0;
	//};

	CEditToolDataEffect()
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		UINT nLen = SysStringLen(a_bstr);
		if (nLen < 2)
			return S_FALSE;

		UINT nCfgLen = *a_bstr;
		if (nLen < nCfgLen+1)
			return S_FALSE;

		bstrEffect.Attach(::SysAllocStringLen(a_bstr+1, nCfgLen));

		LPCOLESTR psz = a_bstr+1+nCfgLen;
		nLen -= 1+nCfgLen;
		CLSID tID = GUID_NULL;
		if (nLen < 36 || !GUIDFromString(psz, &tID) || IsEqualGUID(tID, GUID_NULL))
			return S_OK;
		pInternal = NULL;
		RWCoCreateInstance(pInternal, tID);
		if (pInternal)
		{
			CComBSTR bstr(psz+36);
			pInternal->FromText(bstr);
		}
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		ULONG nEffLen = SysStringLen(bstrEffect);
		CComBSTR bstr(nEffLen+1);
		bstr[0] = nEffLen;
		CopyMemory(bstr.m_str+1, bstrEffect.m_str, nEffLen*sizeof(wchar_t));
		if (pInternal)
		{
			CLSID tID = GUID_NULL;
			pInternal->CLSIDGet(&tID);
			if (!IsEqualGUID(tID, GUID_NULL))
			{
				wchar_t szTmp[64];
				StringFromGUID(tID, szTmp);
				CComBSTR bstrInt;
				pInternal->ToText(&bstrInt);
				if (bstrInt.Length())
				{
					bstr += szTmp;
					bstr += bstrInt;
				}
			}
		}
		*a_pbstr = bstr;
		bstr.Detach();
		return S_OK;
	}

	CComBSTR bstrEffect;
	CComPtr<ISharedState> pInternal;
};

MIDL_INTERFACE("47DD2C53-43A5-4DC2-9DE5-CF740B15234A")
ISharedStateEffect : public ISharedState
{
	STDMETHOD_(CEditToolDataEffect const*, InternalData)() = 0;
};

// {3369238C-99E0-496a-84BE-C185BB26DE0D}
extern __declspec(selectany) GUID const CLSID_SharedStateEffect = {0x3369238c, 0x99e0, 0x496a, {0x84, 0xbe, 0xc1, 0x85, 0xbb, 0x26, 0xde, 0xd}};

// CSharedStateEffect

class ATL_NO_VTABLE CSharedStateEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateEffect, &CLSID_SharedStateEffect>,
	public ISharedStateEffect
{
public:
	void Init(CEditToolDataEffect const& a_cData) { m_cData = a_cData; }

	DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CSharedStateEffect)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(ISharedStateEffect)
END_COM_MAP()

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID) { *a_pCLSID = CLSID_SharedStateEffect; return S_OK; }
	STDMETHOD(ToText)(BSTR* a_pbstrText) { return m_cData.ToString(a_pbstrText); }
	STDMETHOD(FromText)(BSTR a_bstrText) { return m_cData.FromString(a_bstrText); }

	// ISharedStateBubble methods
public:
    STDMETHOD_(CEditToolDataEffect const*, InternalData)() { return &m_cData; }

private:
	CEditToolDataEffect m_cData;
};

OBJECT_ENTRY_AUTO(CLSID_SharedStateEffect, CSharedStateEffect)


#include "EditToolEffectDlg.h"


class CEditToolEffect :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditTool,
	public IRasterImageEditToolScripting,
	public IRasterImageEditWindow
{
public:
	CEditToolEffect() : m_pCallback(NULL), m_bEffect(false), m_pReceived(NULL), m_eBlendingMode(EBMDrawOver), m_bstrID(L"Effect")
	{
		RWCoCreateInstance(m_pOpMgr, __uuidof(OperationManager));
		GetEffect(NULL, m_pEffect, m_bEffect, m_pOpMgr, m_bstrID);
	}
	~CEditToolEffect()
	{
		m_pTool = NULL;
		if (m_pCallback)
		{
			m_pCallback->SetOwner(NULL);
			m_pCallback->Release();
		}
	}
	void Init(IRasterImageEditTool* a_pInternalTool)
	{
		m_pTool = a_pInternalTool;
	}

	static HRESULT WINAPI QICustomToolInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CEditToolEffect* const p = reinterpret_cast<CEditToolEffect*>(pv);
		if (p->m_pTool)
			return p->m_pTool->QueryInterface(riid, ppv);
		return E_NOINTERFACE;
	}

	BEGIN_COM_MAP(CEditToolEffect)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
		// note: QI back to IRasterImageEditTool will give wrong results, but it is an unsupported scenario anyway
		COM_INTERFACE_ENTRY_FUNC_BLIND(0, QICustomToolInterface)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(IRasterImageEditTool::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		try
		{
			if (!m_bEffect)
				return m_pTool->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_pBuffer);

			HRESULT hRes = m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, EITIBackground, a_pBuffer);
			if (FAILED(hRes)) return hRes;

			// rectangle to redraw
			RECT const rcRedraw = { a_nX, a_nY, a_nX+a_nSizeX, a_nY+a_nSizeY };

			ULONG nImgSizeX = 0;
			ULONG nImgSizeY = 0;
			m_pWindow->Size(&nImgSizeX, &nImgSizeY);

			// needed extra
			RECT rcAdjusted = rcRedraw;
			CConfigValue cEffectID;
			m_pEffect->ItemValueGet(m_bstrID, &cEffectID);
			CComPtr<IConfig> pEffect;
			m_pEffect->SubConfigGet(m_bstrID, &pEffect);
			TImageSize tCanvas = {nImgSizeX, nImgSizeY};
			TRasterImageRect tAdjusted = {{rcRedraw.left, rcRedraw.top}, {rcRedraw.right, rcRedraw.bottom}};
			CLayerImageAdjustment cLIA(false, &tCanvas, &tAdjusted);
			if (SUCCEEDED(m_pOpMgr->Visit(m_pOpMgr, cEffectID, pEffect, &cLIA)))
			{
				rcAdjusted.left = tAdjusted.tTL.nX;
				rcAdjusted.top = tAdjusted.tTL.nY;
				rcAdjusted.right = tAdjusted.tBR.nX;
				rcAdjusted.bottom = tAdjusted.tBR.nY;
			}
			else
			{
				// operation does not support regional processing -> better mark the whole canvas as invalid
				rcAdjusted.left = 0;
				rcAdjusted.top = 0;
				rcAdjusted.right = nImgSizeX;
				rcAdjusted.bottom = nImgSizeY;
			}

			TImagePoint tOff = {rcAdjusted.left, rcAdjusted.top};
			TImageSize tSize = {rcAdjusted.right-rcAdjusted.left, rcAdjusted.bottom-rcAdjusted.top};
			CPixelChannel chDef(0UL);

			CComPtr<IDocument> pDoc;
			RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
			CComQIPtr<IDocumentBase> pBase(pDoc);

			CComPtr<IDocumentFactoryRasterImageCallback> pFct;
			RWCoCreateInstance(pFct, __uuidof(DocumentFactoryRasterImage));
			//TRasterImageRect tRect = {{rcRedraw.left, rcRedraw.top}, {rcRedraw.right, rcRedraw.bottom}};
			TRasterImageRect tRect = {{rcAdjusted.left, rcAdjusted.top}, {rcAdjusted.right, rcAdjusted.bottom}};
			pFct->Create(NULL, pBase, &tCanvas, NULL, 1, CChannelDefault(EICIRGBA), &tRect, &CRasterImageCallback(m_pTool, rcAdjusted));

			CComObjectStackEx<CLayerOperationContext> cLOC;
			m_pOpMgr->Activate(m_pOpMgr, pDoc, cEffectID, pEffect, &cLOC, NULL, 0);

			CComPtr<IDocumentImage> pImg;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));

			TImagePoint tOrig = {0, 0};
			TImageSize tCnt = {0, 0};
			pImg->CanvasGet(NULL, NULL, &tOrig, &tCnt, NULL);
			TImagePoint tEnd = {tOrig.nX+tCnt.nX, tOrig.nY+tCnt.nY};
			if (tCnt.nX == 0 || tCnt.nY == 0)
			{
				tOrig.nX = LONG_MIN;
				tOrig.nY = LONG_MIN;
				tEnd.nX = LONG_MAX;
				tEnd.nY = LONG_MAX;
			}

			if (tOrig.nX < a_nX) tOrig.nX = a_nX;
			if (tOrig.nY < a_nY) tOrig.nY = a_nY;
			if (tEnd.nX > LONG(a_nX+a_nSizeX)) tEnd.nX = a_nX+a_nSizeX;
			if (tEnd.nY > LONG(a_nY+a_nSizeY)) tEnd.nY = a_nY+a_nSizeY;

			if (tEnd.nX > tOrig.nX && tEnd.nY > tOrig.nY)
			{
				tCnt.nX = tEnd.nX-tOrig.nX;
				tCnt.nY = tEnd.nY-tOrig.nY;
				pImg->Inspect(EICIRGBA, &tOrig, &tCnt, &CNormalBlender(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer), NULL, EIRIAccurate);
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(IRasterImageEditTool::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return m_pWindow->GetSelectionInfo(a_pBoundingRectangle, a_bEntireRectangle);
	}
	STDMETHOD(IRasterImageEditTool::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return m_pWindow->GetSelectionTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_nStride, a_pBuffer);
	}

	STDMETHOD(Init)(IRasterImageEditWindow* a_pWindow)
	{
		m_pWindow = a_pWindow;
		CComObject<CCallbackHelper<IRasterImageEditWindow> >::CreateInstance(&m_pCallback);
		m_pCallback->AddRef();
		m_pCallback->SetOwner(this);
		m_pTool->Init(m_pCallback);
		return S_OK;
	}

	STDMETHOD(IRasterImageEditTool::SetState)(ISharedState* a_pState)
	{
		CComQIPtr<ISharedStateEffect> pSSE(a_pState);
		if (pSSE == NULL)
			return E_FAIL;
		CComBSTR bstrPrev = m_cData.bstrEffect;
		m_cData = *pSSE->InternalData();
		if (bstrPrev == m_cData.bstrEffect)
		{
			RECT rcDirty = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			m_pReceived = &rcDirty;
			if (m_cData.pInternal)
				m_pTool->SetState(m_cData.pInternal);
			m_pReceived = NULL;
			AdjustDirty(rcDirty);
			m_pWindow->RectangleChanged(&rcDirty);
		}
		else
		{
			RECT rcDirtyPrev = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			m_pTool->IsDirty(&rcDirtyPrev, NULL, NULL);
			AdjustDirty(rcDirtyPrev);

			RECT rcDummy = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			m_pReceived = &rcDummy;
			if (m_cData.pInternal)
				m_pTool->SetState(m_cData.pInternal);
			m_pReceived = NULL;

			CComPtr<IConfig> pEffect;
			bool bEffect = false;
			GetEffect(m_cData.bstrEffect, pEffect, bEffect, m_pOpMgr, m_bstrID);
			std::swap(pEffect.p, m_pEffect.p);
			std::swap(bEffect, m_bEffect);
			RECT rcDirtyNext = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
			m_pTool->IsDirty(&rcDirtyNext, NULL, NULL);
			AdjustDirty(rcDirtyNext);

			RECT rcDirty =
			{
				min(rcDirtyPrev.left, rcDirtyNext.left),
				min(rcDirtyPrev.top, rcDirtyNext.top),
				max(rcDirtyPrev.right, rcDirtyNext.right),
				max(rcDirtyPrev.bottom, rcDirtyNext.bottom),
			};
			m_pWindow->RectangleChanged(&rcDirty);
		}
		return S_OK;
	}
	STDMETHOD(SetOutline)(BYTE a_bOutline, float a_fOutlineWidth, float a_fOutlinePos, EOutlineJoinType a_eOutlineJoins, TColor const* a_pOutlineColor)
	{
		return m_pTool->SetOutline(a_bOutline, a_fOutlineWidth, a_fOutlinePos, a_eOutlineJoins, a_pOutlineColor);
	}
	STDMETHOD(SetBrush)(IRasterImageBrush* a_pBrush)
	{
		return m_pTool->SetBrush(a_pBrush);
	}
	STDMETHOD(SetGlobals)(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			RECT rc;
			if (S_OK == IsDirty(&rc, NULL, NULL))
				m_pWindow->RectangleChanged(&rc);
			return S_OK;
		}
		return m_pTool->SetGlobals(EBMReplace, a_eRasterizationMode, a_eCoordinatesMode);
	}

	STDMETHOD(Reset)()
	{
		return m_pTool->Reset();
	}
	STDMETHOD(PreTranslateMessage)(MSG const* a_pMsg)
	{
		return m_pTool->PreTranslateMessage(a_pMsg);
	}

	STDMETHOD(IsDirty)(RECT* a_pImageRect, BOOL* a_pOptimizeImageRect, RECT* a_pSelectionRect)
	{
		if (!m_bEffect || a_pImageRect == NULL)
			return m_pTool->IsDirty(a_pImageRect, a_pOptimizeImageRect, a_pSelectionRect);
		RECT rc = *a_pImageRect;
		HRESULT hRes = m_pTool->IsDirty(&rc, a_pOptimizeImageRect, a_pSelectionRect);
		if (rc.left < rc.right && rc.top < rc.bottom)
		{
			AdjustDirty(rc);
		}
		*a_pImageRect = rc;
		return hRes;
	}

	STDMETHOD(AdjustCoordinates)(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		return m_pTool->AdjustCoordinates(a_eKeysState, a_pPos, a_pPointerSize, a_pControlPointIndex, a_fPointSize);
	}
	STDMETHOD(ProcessInputEvent)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		return m_pTool->ProcessInputEvent(a_eKeysState, a_pPos, a_pPointerSize, a_fNormalPressure, a_fTangentPressure, a_fOrientation, a_fRotation, a_fZ, a_pMaxIdleTime);
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		return m_pTool->GetCursor(static_cast<EControlKeysState>(a_eKeysState&(~ECKSControl)), a_pPos, a_phCursor);
	}

	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		return m_pTool->GetControlPointCount(a_pCount);
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		return m_pTool->GetControlPoint(a_nIndex, a_pPos, a_pClass);
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		return m_pTool->SetControlPoint(a_nIndex, a_pPos, a_bFinished, a_fPointSize);
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return m_pTool->GetControlPointDesc(a_nIndex, a_ppDescription);
	}
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		return m_pTool->GetControlLines(a_pLines, a_nLineTypes);
	}
	STDMETHOD(PointTest)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, BYTE a_bAccurate, float a_fPointSize)
	{
		return m_pTool->PointTest(a_eKeysState, a_pPos, a_bAccurate, a_fPointSize);
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return m_pTool->Transform(a_pMatrix);
	}

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return m_pWindow->Size(a_pSizeX, a_pSizeY);
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		return m_pWindow->GetDefaultColor(a_pDefault);
	}
	STDMETHOD(IRasterImageEditWindow::GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		switch (a_eIntent)
		{
		case EITIBackground:
			{
				//TRasterImagePixel t;
				//t.bR = m_cData.tFill.fR < 0.0f ? 0 : (m_cData.tFill.fR > 1.0f ? 255 : m_cData.tFill.fR*255.0f+0.5f);
				//t.bG = m_cData.tFill.fG < 0.0f ? 0 : (m_cData.tFill.fG > 1.0f ? 255 : m_cData.tFill.fG*255.0f+0.5f);
				//t.bB = m_cData.tFill.fB < 0.0f ? 0 : (m_cData.tFill.fB > 1.0f ? 255 : m_cData.tFill.fB*255.0f+0.5f);
				//t.bA = m_cData.tFill.fA < 0.0f ? 0 : (m_cData.tFill.fA > 1.0f ? 255 : m_cData.tFill.fA*255.0f+0.5f);
				//for (ULONG y = 0; y < a_nSizeY; ++y)
				//	std::fill_n(a_pBuffer+a_nStride*y, a_nSizeX, t);
			}
			return S_OK;
		case EITIContent:
			return m_pWindow->GetImageTile(a_nX, a_nY, a_nSizeX, a_nSizeY, a_fGamma, a_nStride, a_eIntent, a_pBuffer);
		case EITIBoth:
		default:
			return E_FAIL;
		}
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		if (a_pBoundingRectangle)
		{
			a_pBoundingRectangle->left = 0;
			a_pBoundingRectangle->top = 0;
			m_pWindow->Size(reinterpret_cast<ULONG*>(&(a_pBoundingRectangle->right)), reinterpret_cast<ULONG*>(&(a_pBoundingRectangle->bottom)));
		}
		if (a_bEntireRectangle)
			*a_bEntireRectangle = TRUE;
		return S_OK;
	}
	STDMETHOD(IRasterImageEditWindow::GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		for (ULONG y = 0; y < a_nSizeY; ++y)
			FillMemory(a_pBuffer+y*a_nStride, a_nSizeX, 0xff);
		return S_OK;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return m_pWindow->ControlPointsChanged();
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return m_pWindow->ControlPointChanged(a_nIndex);
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return m_pWindow->ControlLinesChanged();
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		if (m_pReceived && a_pChanged)
		{
			*m_pReceived = *a_pChanged;
			return S_OK;
		}

		if (!m_bEffect || a_pChanged == NULL)
			return m_pWindow->RectangleChanged(a_pChanged);

		RECT rcAdjusted = *a_pChanged;
		AdjustDirty(rcAdjusted);
		return m_pWindow->RectangleChanged(&rcAdjusted);
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return m_pWindow->ScrollWindow(a_nScrollID, a_pDelta);
	}
	STDMETHOD(IRasterImageEditWindow::SetState)(ISharedState* a_pState)
	{
		m_cData.pInternal = a_pState;
		CComObject<CSharedStateEffect>* p = NULL;
		CComObject<CSharedStateEffect>::CreateInstance(&p);
		CComPtr<ISharedState> pState = p;
		p->Init(m_cData);
		return m_pWindow->SetState(pState);
	}
	STDMETHOD(IRasterImageEditWindow::SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		return m_pWindow->SetBrushState(a_bstrStyleID, a_pState);
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		return m_pWindow->Handle(a_phWnd);
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		return m_pWindow->Document(a_ppDocument);
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CComQIPtr<IRasterImageEditToolScripting> pInternal(m_pTool);
			if (pInternal == NULL)
				return S_FALSE;

			wchar_t const* p = a_bstrParams;
			wchar_t const* const pE = a_bstrParams+SysStringLen(a_bstrParams);
			while (p != pE && *p != L'\"') ++p;
			if (p == pE)
				return E_INVALIDARG;
			wchar_t const* pS = ++p;
			bool bEscaped = false;
			while (p != pE && (bEscaped || *p != L'\"'))
			{
				if (bEscaped)
					bEscaped = false;
				else
					bEscaped = *p == L'\\';
				++p;
			}
			CComBSTR bstrEffect;
			if (p == pE)
				return E_INVALIDARG;
			UnescapeString(pS, p-pS, bstrEffect);
			if (bstrEffect.Length() == 0)
				bstrEffect.Attach(SysAllocStringLen(pS, p-pS));
			++p;
			while (p != pE && (*p == L' ' || *p == L'\n' || *p == L'\r' || *p == L'\t')) ++p;
			if (p != pE)
			{
				if (*p == ',')
					++p;
				else
					return E_INVALIDARG;
			}
			while (p != pE && (*p == L' ' || *p == L'\n' || *p == L'\r' || *p == L'\t')) ++p;
			if (bstrEffect == m_cData.bstrEffect)
			{
				return p == pE ? S_OK : pInternal->FromText(CComBSTR(pE-p, p));
			}
			else
			{
				RECT rcDirtyPrev = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
				m_pTool->IsDirty(&rcDirtyPrev, NULL, NULL);
				AdjustDirty(rcDirtyPrev);

				m_cData.bstrEffect = bstrEffect;
				CComPtr<IConfig> pEffect;
				bool bEffect = false;
				GetEffect(m_cData.bstrEffect, pEffect, bEffect, m_pOpMgr, m_bstrID);
				std::swap(pEffect.p, m_pEffect.p);
				std::swap(bEffect, m_bEffect);
				CComObject<CSharedStateEffect>* pNew = NULL;
				CComObject<CSharedStateEffect>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				m_pWindow->SetState(pTmp);

				RECT rcDummy = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
				m_pReceived = &rcDummy;
				if (p != pE)
					pInternal->FromText(CComBSTR(pE-p, p));
				//if (m_cData.pInternal)
				//	m_pTool->SetState(m_cData.pInternal);
				m_pReceived = NULL;

				RECT rcDirtyNext = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
				m_pTool->IsDirty(&rcDirtyNext, NULL, NULL);
				AdjustDirty(rcDirtyNext);

				RECT rcDirty =
				{
					min(rcDirtyPrev.left, rcDirtyNext.left),
					min(rcDirtyPrev.top, rcDirtyNext.top),
					max(rcDirtyPrev.right, rcDirtyNext.right),
					max(rcDirtyPrev.bottom, rcDirtyNext.bottom),
				};
				m_pWindow->RectangleChanged(&rcDirty);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		try
		{
			*a_pbstrParams = NULL;
			CComQIPtr<IRasterImageEditToolScripting> pInternal(m_pTool);
			if (pInternal == NULL)
				return S_FALSE;
			CComBSTR bstrInt;
			if (S_OK != pInternal->ToText(&bstrInt))
				return S_FALSE;
			CComBSTR bstrEsc;
			EscapeString(m_cData.bstrEffect, bstrEsc);
			CComBSTR bstr("\"");
			if (bstrEsc.m_str != NULL)
				bstr += bstrEsc;
			else if (m_cData.bstrEffect.m_str != NULL)
				bstr += m_cData.bstrEffect;
			if (bstrInt.Length())
			{
				bstr += L"\", ";
				bstr += bstrInt;
			}
			else
				bstr += L"\"";
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	static void EscapeString(BSTR src, CComBSTR& dst)
	{
		int extra = 0;
		ULONG len = SysStringLen(src);
		for (ULONG i = 0; i < len; ++i)
		{
			if (src[i] == L'\r' || src[i] == L'\n' || src[i] == L'\"' || src[i] == L'\\' || src[i] == L'\0')
				++extra;
			else if (src[i] < L' ')
				extra += 5;
		}
		if (extra == 0)
			return;
		dst.Attach(::SysAllocStringLen(NULL, len+extra));
		wchar_t* p = dst;
		for (ULONG i = 0; i < len; ++i)
		{
			wchar_t const s = src[i];
			if (s == L'\r')
			{
				*p = L'\\';
				++p;
				*p = L'r';
				++p;
			}
			else if (s == L'\n')
			{
				*p = L'\\';
				++p;
				*p = L'n';
				++p;
			}
			else if (s == L'\"')
			{
				*p = L'\\';
				++p;
				*p = L'\"';
				++p;
			}
			else if (s == L'\\')
			{
				*p = L'\\';
				++p;
				*p = L'\\';
				++p;
			}
			else if (s == L'\0')
			{
				*p = L'\\';
				++p;
				*p = L'0';
				++p;
			}
			else if (s < L' ')
			{
				*p = L'\\';
				++p;
				*p = L'u';
				++p;
				*p = L'0';
				++p;
				*p = L'0';
				++p;
				*p = L'0'+(s>>4);
				++p;
				*p = (s&0xf)<10 ? L'0'+(s&0xf) : L'a'-10+(s&0xf);
				++p;
			}
			else
			{
				*p = src[i];
				++p;
			}
		}
	}
	static wchar_t CharVal(wchar_t c)
	{
		if (c > L'0' && c <= L'9')
			return c-L'0';
		if (c >= L'a' && c <= L'f')
			return c-L'a'+10;
		if (c >= L'A' && c <= L'F')
			return c-L'A'+10;
		return 0;
	}
	static void UnescapeString(BSTR src, CComBSTR& dst)
	{
		UnescapeString(src, SysStringLen(src), dst);
	}
	static void UnescapeString(wchar_t const* src, ULONG len, CComBSTR& dst)
	{
		int reduce = 0;
		for (ULONG i = 0; i < len; ++i)
		{
			if (src[i] != L'\\')
				continue;
			if (i+1 >= len)
				return; // error
			++i;
			if (src[i] == L'r' || src[i] == L'n' || src[i] == L'\\' || src[i] == L'\"' || src[i] == L'0')
			{
				++reduce;
				continue;
			}
			if (src[i] != L'u')
				return; // error
			if (i+5 >= len)
				return; // error
			reduce += 5;
		}
		if (reduce == 0)
			return;
		dst.Attach(::SysAllocStringLen(NULL, len-reduce));
		wchar_t* p = dst;
		for (ULONG i = 0; i < len; ++i)
		{
			if (src[i] != L'\\')
			{
				*p = src[i];
				++p;
				continue;
			}
			if (i+1 >= len)
			{
				dst.Empty();
				return; // error
			}
			++i;
			wchar_t const s = src[i];
			if (s == L'r')
			{
				*p = L'\r';
				++p;
				continue;
			}
			else if (s == L'n')
			{
				*p = L'\n';
				++p;
				continue;
			}
			else if (s == L'\"')
			{
				*p = L'\"';
				++p;
				continue;
			}
			else if (s == L'\\')
			{
				*p = L'\\';
				++p;
				continue;
			}
			else if (s == L'0')
			{
				*p = L'\0';
				++p;
				continue;
			}
			else if (s != L'u' || i+5 >= len)
			{
				dst.Empty();
				return; // error
			}
			*p = (CharVal(src[i+1])<<12)|(CharVal(src[i+2])<<8)|(CharVal(src[i+3])<<4)|CharVal(src[i+4]);
			i += 4;
		}
	}

private:
	class CRasterImageCallback : public CStackUnknown<IRasterImageCallback>
	{
	public:
		CRasterImageCallback(IRasterImageEditTool* pTool, RECT rcDirty) : pTool(pTool), rcDirty(rcDirty) {}

		// IRasterImageCallback methods
	public:
		STDMETHOD(Initalize)(ULONG a_nPixels, TPixelChannel* a_aPixels)
		{
			ZeroMemory(a_aPixels, a_nPixels*sizeof*a_aPixels);
			return pTool->GetImageTile(rcDirty.left, rcDirty.top, rcDirty.right-rcDirty.left, rcDirty.bottom-rcDirty.top, 2.2f, rcDirty.right-rcDirty.left, reinterpret_cast<TRasterImagePixel*>(a_aPixels));
		}

	private:
		IRasterImageEditTool* pTool;
		RECT rcDirty;
	};

	class ATL_NO_VTABLE CLayerOperationContext :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IOperationContext
	{
	public:
	BEGIN_COM_MAP(CLayerOperationContext)
		COM_INTERFACE_ENTRY(IOperationContext)
	END_COM_MAP()

		// IOperationContext methods
	public:
		STDMETHOD(StateGet)(BSTR a_bstrCategoryName, REFIID a_iid, void** a_ppState)
		{
			CStates::const_iterator i = m_cStates.find(a_bstrCategoryName);
			if (i == m_cStates.end())
				return E_RW_ITEMNOTFOUND;
			return i->second->QueryInterface(a_iid, a_ppState);
		}
		STDMETHOD(StateSet)(BSTR a_bstrCategoryName, ISharedState* a_pState)
		{
			if (a_pState)
				m_cStates[a_bstrCategoryName] = a_pState;
			else
				m_cStates.erase(a_bstrCategoryName);
			return S_OK;
		}
		STDMETHOD(IsCancelled)()
		{
			return S_FALSE;
		}
		STDMETHOD(GetOperationInfo)(ULONG* a_pItemIndex, ULONG* a_pItemsRemaining, ULONG* a_pStepIndex, ULONG* a_pStepsRemaining)
		{
			if (a_pItemIndex) *a_pItemIndex = 0;
			if (a_pItemsRemaining) *a_pItemsRemaining = 0;
			if (a_pStepIndex) *a_pStepIndex = 0;
			if (a_pStepsRemaining) *a_pStepsRemaining = 0;
			return S_OK;
		}
		STDMETHOD(SetErrorMessage)(ILocalizedString* a_pMessage)
		{
			return S_FALSE;
		}

	private:
		typedef std::map<CComBSTR, CComPtr<ISharedState> > CStates;

	private:
		CStates m_cStates;
	};

	class CLayerImageAdjustment :
		public IPlugInVisitor
	{
	public:
		CLayerImageAdjustment(bool a_bDirty, TImageSize const* a_pCanvas, TRasterImageRect* a_prc) : m_bDirty(a_bDirty), m_pCanvas(a_pCanvas), m_prc(a_prc) {}

		// IUnknown methods
	public:
		STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
		{
			if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IPlugInVisitor)))
			{
				*a_ppvObject = this;
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)() { return 2; }
		STDMETHOD_(ULONG, Release)() { return 1; }

		// IPlugInVisitor methods
	public:
		STDMETHOD(Run)(IUnknown* /*a_pManager*/, IConfig* a_pConfig, IUnknown* a_pOperation)
		{
			CComQIPtr<IRasterImageFilter> pFilter(a_pOperation);
			if (pFilter == NULL) return E_NOINTERFACE;
			return m_bDirty ? pFilter->AdjustDirtyRect(a_pConfig, m_pCanvas, m_prc) : pFilter->NeededToCompute(a_pConfig, m_pCanvas, m_prc);
		}

	private:
		bool m_bDirty;
		TImageSize const* m_pCanvas;
		TRasterImageRect* m_prc;
	};

	class CNormalBlender : public CStackUnknown<IImageVisitor>
	{
	public:
		CNormalBlender(LONG nOriginX, LONG nOriginY, ULONG nSizeX, ULONG nSizeY, ULONG nStride, TRasterImagePixel* a_pBuffer) :
			nOriginX(nOriginX), nOriginY(nOriginY), nSizeX(nSizeX), nSizeY(nSizeY), nStride(nStride), pBuffer(a_pBuffer) {}

		// IImageVisitor methods
	public:
		STDMETHOD(Visit)(ULONG a_nTiles, TImageTile const* a_aTiles, ITaskControl *a_pControl)
		{
			for (TImageTile const* pTile = a_aTiles; pTile != a_aTiles+a_nTiles; ++pTile)
			{
				LONG const y1 = max(nOriginY, pTile->tOrigin.nY);
				LONG const y2 = min(nOriginY+nSizeY, pTile->tOrigin.nY+pTile->tSize.nY);
				for (LONG y = y1; y < y2; ++y)
				{
					LONG const x1 = max(nOriginX, pTile->tOrigin.nX);
					LONG const x2 = min(nOriginX+nSizeX, pTile->tOrigin.nX+pTile->tSize.nX);
					TRasterImagePixel* pD = pBuffer + (y-nOriginY)*nStride + (x1-nOriginX);
					TRasterImagePixel const* pS = reinterpret_cast<TRasterImagePixel const*>(pTile->pData) + (y-pTile->tOrigin.nY)*pTile->tStride.nY + (x1-pTile->tOrigin.nX)*pTile->tStride.nX;
					TRasterImagePixel* const pDEnd = pD+(x2-x1);
					for (; pD < pDEnd; ++pD, pS+=pTile->tStride.nX)
					{
						CPixelMixerPaintOver::Mix(*pD, *pS);
					}
				}
			}
			return S_OK;
		}

	private:
		LONG const nOriginX;
		LONG const nOriginY;
		ULONG const nSizeX;
		ULONG const nSizeY;
		ULONG const nStride;
		TRasterImagePixel* pBuffer;
	};

	static void AdjustDirty(RECT& rc, IConfig* a_pEffect, bool bEffect, IOperationManager* pOpMgr, ULONG nCanvasX, ULONG nCanvasY, BSTR bstrCFGID_LAYEREFFECT)
	{
		if (!bEffect)
			return;
		CConfigValue cEffectID;
		a_pEffect->ItemValueGet(bstrCFGID_LAYEREFFECT, &cEffectID);
		CComPtr<IConfig> pEffect;
		a_pEffect->SubConfigGet(bstrCFGID_LAYEREFFECT, &pEffect);
		TImageSize tCanvas = {nCanvasX, nCanvasY};
		TRasterImageRect tDirty = {{rc.left, rc.top}, {rc.right, rc.bottom}};
		CLayerImageAdjustment cLIA(true, &tCanvas, &tDirty);
		if (SUCCEEDED(pOpMgr->Visit(pOpMgr, cEffectID, pEffect, &cLIA)))
		{
			rc.left = tDirty.tTL.nX;
			rc.top = tDirty.tTL.nY;
			rc.right = tDirty.tBR.nX;
			rc.bottom = tDirty.tBR.nY;
		}
		else
		{
			// operation does not support regional processing -> better mark the whole canvas as invalid
			rc.left = 0;
			rc.top = 0;
			rc.right = nCanvasX;
			rc.bottom = nCanvasY;
		}
	}
	void AdjustDirty(RECT& rc)
	{
		ULONG nX = 0;
		ULONG nY = 0;
		m_pWindow->Size(&nX, &nY);
		AdjustDirty(rc, m_pEffect, m_bEffect, m_pOpMgr, nX, nY, m_bstrID);
	}
	static void GetEffect(BSTR bstrEffect, CComPtr<IConfig>& pEffect, bool& bEffect, IOperationManager* pOpMgr, BSTR bstrID)
	{
		CComPtr<IConfigWithDependencies> pCfg;
		RWCoCreateInstance(pCfg, __uuidof(ConfigWithDependencies));
		pOpMgr->InsertIntoConfigAs(pOpMgr, pCfg, bstrID, NULL, NULL, 0, NULL);
		pCfg->Finalize(NULL);
		//pCfg->ItemValuesSet(1, &bstrID, CConfigValue(__uuidof(DocumentOperationRasterImageDropShadow)));
		pEffect.Attach(pCfg.Detach());
		bEffect = false;//true;//

		CComPtr<IConfigInMemory> pMem;
		RWCoCreateInstance(pMem, __uuidof(ConfigInMemory));
		ULONG const len = ::SysStringLen(bstrEffect);
		if (len > 0)
		{
			CAutoVectorPtr<char> szBuf;
			int nLen = WideCharToMultiByte(CP_UTF8, 0, bstrEffect, len, NULL, 0, NULL, NULL);
			if (nLen)
			{
				szBuf.Allocate(nLen+4);
				szBuf[0] = '\xef';
				szBuf[1] = '\xbb';
				szBuf[2] = '\xbf';
				WideCharToMultiByte(CP_UTF8, 0, bstrEffect, len, szBuf.m_p+3, nLen, NULL, NULL);
				szBuf[nLen+3] = '\0';
			}
			pMem->DataBlockSet(nLen+3, reinterpret_cast<BYTE const*>(szBuf.m_p));
			CopyConfigValues(pEffect, pMem);
			bEffect = !IsNoOperation(pEffect, bstrID);
		}
	}
	static bool IsNoOperation(IConfig* pEffect, BSTR bstrID)
	{
		if (pEffect == NULL)
			return true;
		CConfigValue cVal;
		pEffect->ItemValueGet(bstrID, &cVal);
		if (cVal.TypeGet() != ECVTGUID)
			return true;
		if (IsEqualGUID(cVal.operator const GUID &(), __uuidof(DocumentOperationNULL)))
			return true;
		if (!IsEqualGUID(cVal.operator const GUID &(), __uuidof(DocumentOperationSequence)))
			return false;
		CConfigValue steps;
		CComBSTR bstr(bstrID);
		bstr += L"\\SeqSteps";
		pEffect->ItemValueGet(bstr, &steps);
		for (LONG i = 0; i < steps.operator LONG(); ++i)
		{
			wchar_t sz[128];
			swprintf(sz, L"%s\\%08x\\SeqSkipStep", bstr, i);
			CConfigValue skip;
			pEffect->ItemValueGet(CComBSTR(sz), &skip);
			if (!skip)
				return false;
		}
		return true;
	}

private:
	CComPtr<IRasterImageEditTool> m_pTool;
	CComObject<CCallbackHelper<IRasterImageEditWindow> >* m_pCallback;
	CComPtr<IRasterImageEditWindow> m_pWindow;
	CEditToolDataEffect m_cData;
	CComPtr<IConfig> m_pEffect;
	bool m_bEffect;
	RECT* m_pReceived;
	EBlendingMode m_eBlendingMode;
	CComPtr<IOperationManager> m_pOpMgr;
	CComBSTR m_bstrID;
};

