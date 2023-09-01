
#pragma once

#include <StringParsing.h>
#include <math.h>


class ATL_NO_VTABLE CScriptedVectorImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedVectorImage, &IID_IScriptedVectorImage, &LIBID_RWDocumentImageVectorLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedVectorImage()
	{
	}
	void Init(IScriptingInterfaceManager* a_pMgr, IDocument* a_pDoc, IDocumentVectorImage* a_pDIV)
	{
		m_pMgr = a_pMgr;
		m_pDoc = a_pDoc;
		m_pDVI = a_pDIV;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&m_pDIE));
	}


DECLARE_NOT_AGGREGATABLE(CScriptedVectorImage)

BEGIN_COM_MAP(CScriptedVectorImage)
	COM_INTERFACE_ENTRY(IScriptedVectorImage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
END_COM_MAP()


	// IScriptedVectorImage methods
public:
	STDMETHOD(get_ElementIDs)(IDispatch** ppArray)
	{
		try
		{
			*ppArray = NULL;
			CComPtr<IJScriptArrayInit> p;
			m_pMgr->CreateJScriptArray(&p);
			std::vector<ULONG> cIDs;
			m_pDVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(cIDs));
			for (std::vector<ULONG>::const_iterator i = cIDs.begin(); i != cIDs.end(); ++i)
				p->AddNumber(*i);
			*ppArray = p.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppArray ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(DeleteElement)(ULONG objectID)
	{
		return m_pDVI->ObjectSet(&objectID, NULL, NULL);
	}
	STDMETHOD(CreateElement)(BSTR toolID, BSTR params, ULONG* pElementID)
	{
		try
		{
			*pElementID = 0;
			if (toolID == NULL)
				return E_INVALIDARG;

			return m_pDVI->ObjectSet(pElementID, toolID, params);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ModifyElement)(ULONG objectID, BSTR toolID, BSTR params)
	{
		return m_pDVI->ObjectSet(&objectID, toolID, params);
	}
	STDMETHOD(GetElementName)(ULONG objectID, BSTR* pName)
	{
		return m_pDVI->ObjectNameGet(objectID, pName);
	}
	STDMETHOD(SetElementName)(ULONG objectID, BSTR name)
	{
		return m_pDVI->ObjectNameSet(objectID, name);
	}
	STDMETHOD(GetElementStyleID)(ULONG objectID, BSTR* pStyleID)
	{
		return m_pDVI->ObjectStyleGet(objectID, pStyleID, NULL);
	}
	STDMETHOD(GetElementStyleParams)(ULONG objectID, BSTR* pStyleParams)
	{
		return m_pDVI->ObjectStyleGet(objectID, NULL, pStyleParams);
	}
	STDMETHOD(GetElementToolID)(ULONG objectID, BSTR* pToolID)
	{
		return m_pDVI->ObjectGet(objectID, pToolID, NULL);
	}
	STDMETHOD(GetElementToolParams)(ULONG objectID, BSTR* pToolParams)
	{
		return m_pDVI->ObjectGet(objectID, NULL, pToolParams);
	}
	STDMETHOD(SetElementStyle)(ULONG objectID, BSTR name, BSTR params)
	{
		if (name && *name)
		{
			BOOL const bFill = TRUE;
			m_pDVI->ObjectStateSet(objectID, &bFill, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			return m_pDVI->ObjectStyleSet(objectID, name, params);
		}
		else
		{
			BOOL const bFill = FALSE;
			return m_pDVI->ObjectStateSet(objectID, bFill, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		}
	}
	STDMETHOD(SetElementOutline)(ULONG objectID, float width, ULONG rgba)
	{
		if (width <= 0.0f)
		{
			BOOL const bOutline = FALSE;
			return m_pDVI->ObjectStateSet(objectID, NULL, NULL, NULL, &bOutline, NULL, NULL, NULL, NULL);
		}
		else
		{
			BOOL const bOutline = TRUE;
			TColor tColor =
			{
				CGammaTables::FromSRGB(rgba&0xff),
				CGammaTables::FromSRGB((rgba>>8)&0xff),
				CGammaTables::FromSRGB((rgba>>16)&0xff),
				((rgba>>24)&0xff)/255.0f
			};
			return m_pDVI->ObjectStateSet(objectID, NULL, NULL, NULL, &bOutline, &tColor, &width, NULL, NULL);
		}
	}
	STDMETHOD(ConvertToShape)(ULONG objectID)
	{
		try
		{
			CComBSTR bstrPathID(L"SHAPE");
			CComBSTR bstrPathParams;
			CComObjectStackEx<CToolWindow> cWnd;
			{
				CComBSTR bstrID;
				CComBSTR bstrParams;
				HRESULT hRes = m_pDVI->ObjectGet(objectID, &bstrID, &bstrParams);
				if (FAILED(hRes)) return hRes;
				CComPtr<IRasterImageEditToolsManager> pToolMgr;
				RWCoCreateInstance(pToolMgr, __uuidof(RasterImageEditToolsManager));
				CComPtr<IRasterImageEditTool> pTool;
				pToolMgr->EditToolCreate(bstrID, NULL, &pTool);
				if (pTool == NULL) return E_FAIL;
				CComQIPtr<IRasterImageEditToolScripting> pToolScript(pTool);
				if (pToolScript == NULL) return E_FAIL;
				CComQIPtr<IRasterImageEditToolPolygon> pToolPoly(pTool);
				if (pToolPoly == NULL) return E_FAIL;
				pTool->Init(&cWnd);
				pToolScript->FromText(bstrParams);

				CComPtr<IRasterImageEditTool> pPath;
				pToolMgr->EditToolCreate(bstrPathID, NULL, &pPath);
				CComQIPtr<IRasterImageEditToolScripting> pPathScript(pPath);
				CComQIPtr<IRasterImageEditToolPolygon> pPathPoly(pPath);
				pPath->Init(&cWnd);
				pToolPoly->ToPath(pPathPoly);
				pPathScript->ToText(&bstrPathParams);
			}

			return m_pDVI->ObjectSet(&objectID, bstrPathID, bstrPathParams);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetCanvas)(ULONG sizeX, ULONG sizeY, float scaleX, float scaleY, float offsetX, float offsetY)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(get_sizeX)(ULONG* pVal)
	{
		if (m_pDIE == NULL)
			return E_NOTIMPL;
		TImageSize tSize;
		m_pDIE->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		*pVal = tSize.nX;
		return S_OK;
	}
	STDMETHOD(put_sizeX)(ULONG val)
	{
		//if (m_pDIE == NULL)
			return E_NOTIMPL;
		//ULONG aSizes[2] = {1, 1};
		//m_pRIC->CanvasPropsGet(2, aSizes, NULL);
		//if (val != aSizes[0])
		//{
		//	aSizes[0] = val;
		//	return m_pRIC->CanvasPropsSet(2, aSizes, NULL, NULL);
		//}
		//return S_FALSE;
	}
	STDMETHOD(get_sizeY)(ULONG* pVal)
	{
		if (m_pDIE == NULL)
			return E_NOTIMPL;
		TImageSize tSize;
		m_pDIE->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
		*pVal = tSize.nY;
		return S_OK;
	}
	STDMETHOD(put_sizeY)(ULONG val)
	{
		//if (m_pRIC == NULL)
			return E_NOTIMPL;
		//ULONG aSizes[2] = {1, 1};
		//m_pRIC->CanvasPropsGet(2, aSizes, NULL);
		//if (val != aSizes[1])
		//{
		//	aSizes[1] = val;
		//	return m_pRIC->CanvasPropsSet(2, aSizes, NULL, NULL);
		//}
		//return S_FALSE;
	}
	STDMETHOD(get_background)(ULONG* pRgba)
	{
		//if (m_pRIC == NULL)
			return E_NOTIMPL;
		//float aRGBA[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		//m_pRIC->CanvasPropsGet(0, NULL, aRGBA);
		//ULONG n = 0;
		//for (BYTE b = 0; b < 4; ++b)
		//	n |= ULONG(aRGBA[b] <= 0.0f ? 0 : (aRGBA[b] >= 1.0f ? 255 : aRGBA[b]*255+0.5))<<(b*8);
		//*pRgba = n;
		//return S_OK;
	}
	STDMETHOD(put_background)(ULONG rgba)
	{
		//if (m_pRIC == NULL)
			return E_NOTIMPL;
		//float const aRGBA[4] = {(rgba&0xff)/255.0f, ((rgba>>8)&0xff)/255.0f, ((rgba>>16)&0xff)/255.0f, ((rgba>>24)&0xff)/255.0f};
		//return m_pRIC->CanvasPropsSet(0, NULL, NULL, aRGBA);
	}
	STDMETHOD(CopyElement)(IDispatch* sourceImage, ULONG objectID, ULONG* pElementID)
	{
		try
		{
			*pElementID = 0;
			CComPtr<IDocument> pDoc;
			CComPtr<IDocumentVectorImage> pVI;
			if (sourceImage)
			{
				sourceImage->QueryInterface(&pDoc);
				pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pVI));
			}
			else
			{
				pVI = m_pDVI;
			}
			if (pVI == NULL)
				return E_INVALIDARG;
			CComBSTR bstrToolID;
			CComBSTR bstrToolParams;
			if (FAILED(pVI->ObjectGet(objectID, &bstrToolID, &bstrToolParams)))
				return E_FAIL;

			if (FAILED(m_pDVI->ObjectSet(pElementID, bstrToolID, bstrToolParams)))
				return E_FAIL;

			CComBSTR bstrStyleID;
			CComBSTR bstrStyleParams;
			if (SUCCEEDED(pVI->ObjectStyleGet(objectID, &bstrStyleID, &bstrStyleParams)))
				m_pDVI->ObjectStyleSet(*pElementID, bstrStyleID, bstrStyleParams);

			BOOL bFill = TRUE;
			ERasterizationMode eRM = ERMSmooth;
			ECoordinatesMode eCM = ECMFloatingPoint;
			BOOL bOutline = FALSE;
			TColor tOutClr = {0.0f, 0.0f, 0.0f, 1.0f};
			float fOutWidth = 1.0f;
			float fOutPos = 0.0f;
			EOutlineJoinType eOutJoins = EOJTRound;
			if (SUCCEEDED(pVI->ObjectStateGet(objectID, &bFill, &eRM, &eCM, &bOutline, &tOutClr, &fOutWidth, &fOutPos, &eOutJoins)))
				m_pDVI->ObjectStateSet(*pElementID, &bFill, &eRM, &eCM, &bOutline, &tOutClr, &fOutWidth, &fOutPos, &eOutJoins);

			CComBSTR bstrName;
			if (SUCCEEDED(pVI->ObjectNameGet(objectID, &bstrName)))
				m_pDVI->ObjectNameSet(*pElementID, bstrName);

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHOD(GetIDsFromState)(IScriptedContext* context, BSTR state, IDispatch** ppArray)
	{
		try
		{
			*ppArray = NULL;

			CComBSTR bstrPrefix;
			m_pDVI->StatePrefix(&bstrPrefix);
			if (bstrPrefix.Length())
			{
				bstrPrefix += state;
				state = bstrPrefix;
			}

			CComQIPtr<IOperationContext> pContext(context);
			CComPtr<ISharedState> pState;
			pContext->StateGet(state, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			std::vector<ULONG> aItems;
			if (pState)
				m_pDVI->StateUnpack(pState, &CEnumToVector<IEnum2UInts, ULONG>(aItems));

			CComPtr<IJScriptArrayInit> p;
			m_pMgr->CreateJScriptArray(&p);

			for (std::vector<ULONG>::const_iterator i = aItems.begin(); i != aItems.end(); ++i)
				p->AddNumber(*i);
			*ppArray = p.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppArray ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(PutIDsToState)(IScriptedContext* context, BSTR state, VARIANT* pSelectedItems)
	{
		try
		{
			std::set<ULONG> cObjectIDs;
			m_pDVI->ObjectIDs(&CEnumToSet<IEnum2UInts, ULONG>(cObjectIDs));

			CComPtr<ISharedState> pState;
			if (pSelectedItems->vt == VT_DISPATCH)
			{
				DISPPARAMS params;
				ZeroMemory(&params, sizeof params);
				CComVariant res;
				DISPID dl = 0;
				LPOLESTR ln = L"length";
				if (SUCCEEDED(pSelectedItems->pdispVal->GetIDsOfNames(IID_NULL, &ln, 1, LOCALE_USER_DEFAULT, &dl)) &&
					SUCCEEDED(pSelectedItems->pdispVal->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
				{
					res.ChangeType(VT_I4);
					LONG len = res.lVal;

					CAutoVectorPtr<ULONG> aItems(len ? new ULONG[len] : NULL);
					ULONG nItems = 0;

					for (int j = 0; j < len; ++j)
					{
						OLECHAR szIndex[16];
						swprintf(szIndex, L"%i", j);
						LPOLESTR psz = szIndex;
						DISPID id = 0;
						res.Clear();
						if (SUCCEEDED(pSelectedItems->pdispVal->GetIDsOfNames(IID_NULL, &psz, 1, LOCALE_USER_DEFAULT, &id)) &&
							SUCCEEDED(pSelectedItems->pdispVal->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
						{
							res.ChangeType(VT_I4);
							if (cObjectIDs.find(res.intVal) != cObjectIDs.end())
							{
								aItems[nItems] = res.intVal;
								++nItems;
							}
						}
					}

					m_pDVI->StatePack(nItems, aItems.m_p, &pState);
				}
			}
			else if (pSelectedItems->vt == VT_I4)
			{
				if (cObjectIDs.find(pSelectedItems->intVal) != cObjectIDs.end())
				{
					ULONG nItem = pSelectedItems->intVal;
					m_pDVI->StatePack(1, &nItem, &pState);
				}
			}
			CComBSTR bstrPrefix;
			m_pDVI->StatePrefix(&bstrPrefix);
			if (bstrPrefix.Length())
			{
				bstrPrefix += state;
				state = bstrPrefix;
			}
			CComQIPtr<IOperationContext> pContext(context);
			return pState ? pContext->StateSet(state, pState) : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

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
		STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState) { return E_NOTIMPL; }
		STDMETHOD(Handle)(RWHWND* a_phWnd) { return E_NOTIMPL; }
		STDMETHOD(Document)(IDocument** a_ppDocument) { ATLASSERT(0); return E_NOTIMPL; }
		STDMETHOD(Checkpoint)() { return E_NOTIMPL; }
	};

private:
	CComPtr<IScriptingInterfaceManager> m_pMgr;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentVectorImage> m_pDVI;
	CComPtr<IDocumentEditableImage> m_pDIE;
};

