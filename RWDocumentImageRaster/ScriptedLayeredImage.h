
#pragma once

#include <StringParsing.h>


class ATL_NO_VTABLE CScriptedLayeredImage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedLayeredImage, &IID_IScriptedLayeredImage, &LIBID_RWDocumentImageRasterLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
public:
	CScriptedLayeredImage()
	{
	}
	void Init(IScriptingInterfaceManager* a_pMgr, IDocument* a_pDoc, IDocumentLayeredImage* a_pDIL/*, HWND a_hWnd, LCID a_tLocaleID*/)
	{
		m_pMgr = a_pMgr;
		m_pDoc = a_pDoc;
		m_pDIL = a_pDIL;
		m_hWnd = NULL;//a_hWnd;
		m_tLocaleID = GetThreadLocale();//a_tLocaleID;
		m_pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&m_pImg));
	}


DECLARE_NOT_AGGREGATABLE(CScriptedLayeredImage)

BEGIN_COM_MAP(CScriptedLayeredImage)
	COM_INTERFACE_ENTRY(IScriptedLayeredImage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_AGGREGATE(__uuidof(IDocument), m_pDoc.p)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IScriptedLayeredImage methods
public:
	STDMETHOD(get_Layers)(ULONG* pnCount)
	{
		try
		{
			*pnCount = 0;
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			if (pItems)
				pItems->Size(pnCount);
			return S_OK;
		}
		catch (...)
		{
			return pnCount/*ppArray*/ ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetLayer)(ULONG index, IScriptedDocument** document)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			CComPtr<ISubDocumentID> pSDI;
			m_pDIL->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDI));
			CComPtr<IDocument> pDoc;
			pSDI->SubDocumentGet(&pDoc);
			return m_pMgr->WrapDocument(m_pMgr, pDoc, document);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(MoveLayer)(ULONG index, LONG under)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			CComPtr<IComparable> pBefore;
			pItems->Get(under, __uuidof(IComparable), reinterpret_cast<void**>(&pBefore));
			return m_pDIL->LayerMove(pItem, pBefore, ELIPBelow);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DeleteLayer)(ULONG index)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			return m_pDIL->LayerDelete(pItem);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CreateLayer)(LONG under, IScriptedOpConfig* config, VARIANT builder, LONG* newLayer)
	{
		try
		{
			if (config == NULL)
				return E_POINTER;
			*newLayer = -1;
			CConfigValue cID;
			CComPtr<IConfig> pCfg;
			if (FAILED(config->GetOpIDAndConfig(&cID, &pCfg)))
				return E_UNEXPECTED;

			std::vector<CComPtr<IDocumentBuilder> > cBuilders;
			if (builder.vt == VT_BSTR)
			{
				CComPtr<IDocumentBuilder> pFact;
				ULONG const n = ::SysStringLen(builder.bstrVal);
				bool bGUID = false;
				GUID tBuilder = GUID_NULL;
				if (n == 36)
					bGUID = GUIDFromString(builder.bstrVal, &tBuilder);
				else if (n == 38 && builder.bstrVal[0] == L'{' && builder.bstrVal[36] == L'}')
					bGUID = GUIDFromString(builder.bstrVal+1, &tBuilder);
				if (bGUID)
				{
					if (FAILED(RWCoCreateInstance(pFact, tBuilder)))
						return E_FAIL;
					cBuilders.push_back(pFact);
				}
				else if (n > 0)
				{
					CComPtr<IPlugInCache> pPIC;
					RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
					CComPtr<IEnumUnknowns> pBuilders;
					if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
					ULONG nBuilders = 0;
					pBuilders->Size(&nBuilders);
					ULONG nBest = 0;
					for (ULONG i = 0; i < nBuilders; ++i)
					{
						CComPtr<IDocumentBuilder> pDB;
						pBuilders->Get(i, &pDB);
						CComPtr<ILocalizedString> pName;
						if (pDB == NULL) continue;
						pDB->TypeName(&pName);
						if (pName == NULL) continue;
						CComBSTR bstrName;
						pName->GetLocalized(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT), &bstrName);
						if (bstrName == builder.bstrVal)
						{
							cBuilders.push_back(pDB);
							break;
						}
					}
					if (cBuilders.empty())
						return E_FAIL;
				}
			}
			if (cBuilders.empty())
			{
				CComPtr<IPlugInCache> pPIC;
				RWCoCreateInstance(pPIC, __uuidof(PlugInCache));
				CComPtr<IEnumUnknowns> pBuilders;
				if (pPIC) pPIC->InterfacesEnum(CATID_DocumentBuilder, __uuidof(IDocumentBuilder), 0, &pBuilders, NULL);
				ULONG nBuilders = 0;
				pBuilders->Size(&nBuilders);
				ULONG nBest = 0;
				for (ULONG i = 0; i < nBuilders; ++i)
				{
					CComPtr<IDocumentBuilder> pDB;
					pBuilders->Get(i, &pDB);
					ULONG nPriority = EDPAverage;
					pDB->Priority(&nPriority);
					size_t j = 0;
					for (; j < cBuilders.size(); ++j)
					{
						ULONG nPriority2 = EDPAverage;
						cBuilders[j]->Priority(&nPriority2);
						if (nPriority2 < nPriority)
						{
							cBuilders.insert(cBuilders.begin()+j, pDB);
							break;
						}
					}
					if (j == cBuilders.size())
						cBuilders.push_back(pDB);
				}
			}

			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pBefore;
			pItems->Get(under, __uuidof(IComparable), reinterpret_cast<void**>(&pBefore));
			CComPtr<IComparable> pNew;
			m_pDIL->LayerInsert(pBefore, pBefore ? ELIPBelow : ELIPDefault, &CImageLayerCreatorWizard(cID, pCfg, m_hWnd, m_tLocaleID, cBuilders.size(), !cBuilders.empty() ? &(cBuilders[0].p) : NULL), &pNew);
			*newLayer = IndexFromID(pNew);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ImportLayer)(LONG under, BSTR bstrPath, LONG* newLayer)
	{
		try
		{
			*newLayer = -1;
			CStorageFilter pLoc(bstrPath);
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pBefore;
			pItems->Get(under, __uuidof(IComparable), reinterpret_cast<void**>(&pBefore));
			CComPtr<IComparable> pNew;
			m_pDIL->LayerInsert(pBefore, pBefore ? ELIPBelow : ELIPDefault, &CImageLayerCreatorStorage(pLoc), &pNew);
			*newLayer = IndexFromID(pNew);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(CopyToLayer)(LONG under, IUnknown* document, VARIANT convertToRaster, LONG* newLayer)
	{
		try
		{
			*newLayer = -1;
			CComQIPtr<IDocument> pDoc(document);
			if (pDoc == NULL)
				return E_INVALIDARG;
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pBefore;
			pItems->Get(under, __uuidof(IComparable), reinterpret_cast<void**>(&pBefore));
			bool bConvert = false;
			if (convertToRaster.vt != VT_UNKNOWN && convertToRaster.vt != VT_EMPTY)
			{
				CComVariant cVar;
				if (SUCCEEDED(cVar.ChangeType(VT_BOOL, &convertToRaster)))
				{
					bConvert = cVar.boolVal != 0;
				}
			}
			if (!bConvert)
			{
				CComPtr<IComparable> pNew;
				m_pDIL->LayerInsert(pBefore, pBefore ? ELIPBelow : ELIPDefault, &CImageLayerCreatorDocument(pDoc), &pNew);
				*newLayer = IndexFromID(pNew);
				return S_OK;
			}

			CComPtr<IDocumentImage> pDocImg;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
			TImageSize tSize = {0, 0};
			pDocImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			if (tSize.nX*tSize.nY == 0)
				return E_FAIL;
			CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);
			pDocImg->TileGet(EICIRGBA, NULL, NULL, NULL, tSize.nX*tSize.nY, pBuffer, NULL, EIRIPreview);

			CComPtr<IComparable> pNew;
			m_pDIL->LayerInsert(pBefore, pBefore ? ELIPBelow : ELIPDefault, &CImageLayerCreatorRasterImage(tSize, pBuffer), &pNew);
			*newLayer = IndexFromID(pNew);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetLayerName)(ULONG index, BSTR* pName)
	{
		try
		{
			*pName = NULL;
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			return m_pDIL->LayerNameGet(pItem, pName);
		}
		catch (...)
		{
			return pName ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetLayerName)(ULONG index, BSTR name)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			return m_pDIL->LayerNameSet(pItem, name);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetLayerStyle)(ULONG index, BSTR path)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			if (pItem == NULL)
				return E_INVALIDARG;
			CComPtr<IStorageManager> pSM;
			RWCoCreateInstance(pSM, __uuidof(StorageManager));
			CComPtr<IStorageFilter> pFlt;
			pSM->FilterCreate(path, EFTOpenExisting|EFTAccessRead|EFTShareRead, &pFlt);
			CComPtr<IDataSrcDirect> pSrc;
			if (pFlt) pFlt->SrcOpen(&pSrc);
			if (pSrc == NULL)
				return E_FAIL;
			ULONG nSize = 0;
			pSrc->SizeGet(&nSize);
			CDirectInputLock cData(pSrc, nSize);
			CComPtr<IConfigInMemory> pMemCfg;
			RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
			if (FAILED(pMemCfg->DataBlockSet(nSize, cData)))
				return E_FAIL;
			return m_pDIL->LayerEffectSet(pItem, pMemCfg);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetIndicesFromState)(IScriptedContext* context, BSTR state, IDispatch** ppArray)
	{
		try
		{
			*ppArray = NULL;

			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			ULONG nItems = 0;
			pItems->Size(&nItems);

			CComQIPtr<IOperationContext> pContext(context);
			CComPtr<ISharedState> pState;
			CComBSTR bstrPrefix;
			m_pDIL->StatePrefix(&bstrPrefix);
			if (bstrPrefix.Length())
			{
				bstrPrefix += state;
				state = bstrPrefix;
			}
			pContext->StateGet(state, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));
			CComPtr<IEnumUnknowns> pIDs;
			if (pState)
				m_pDIL->StateUnpack(pState, &pIDs);

			CComPtr<IJScriptArrayInit> p;
			m_pMgr->CreateJScriptArray(&p);

			ULONG nSel = 0;
			if (pIDs) pIDs->Size(&nSel);
			for (ULONG i = 0; i < nSel; ++i)
			{
				CComPtr<IComparable> pSel;
				pIDs->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pSel));
				for (ULONG j = 0; j < nItems; ++j)
				{
					CComPtr<IComparable> pItem;
					pItems->Get(j, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
					if (S_OK == pSel->Compare(pItem))
					{
						p->AddNumber(j);
						break;
					}
				}
			}
			*ppArray = p.Detach();
			return S_OK;
		}
		catch (...)
		{
			return ppArray ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(PutIndicesToState)(IScriptedContext* context, BSTR state, VARIANT* pSelectedItems)
	{
		try
		{
			CComPtr<ISharedState> pState;

			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);

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

					CAutoVectorPtr<IComparable*> aItems(len ? new IComparable*[len] : NULL);
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
							pItems->Get(res.intVal, aItems+nItems);
							if (aItems[nItems]) ++nItems;
						}
					}

					m_pDIL->StatePack(nItems, aItems.m_p, &pState);
				}
			}
			else if (pSelectedItems->vt == VT_I4)
			{
				CComPtr<IComparable> pItem;
				pItems->Get(pSelectedItems->intVal, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
				if (pItem)
					m_pDIL->StatePack(1, &(pItem.p), &pState);
			}
			CComBSTR bstrPrefix;
			m_pDIL->StatePrefix(&bstrPrefix);
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
	STDMETHOD(get_sizeX)(ULONG* pVal)
	{
		TImageSize tFmt;
		HRESULT hRes = m_pImg->CanvasGet(&tFmt, NULL, NULL, NULL, NULL);
		*pVal = tFmt.nX;
		return hRes;
	}
	STDMETHOD(put_sizeX)(ULONG val)
	{
		TImageSize tFmt;
		HRESULT hRes = m_pImg->CanvasGet(&tFmt, NULL, NULL, NULL, NULL);
		if (tFmt.nX != val)
		{
			tFmt.nX = val;
			return m_pImg->CanvasSet(&tFmt, NULL, NULL, NULL);
		}
		return S_FALSE;
	}
	STDMETHOD(get_sizeY)(ULONG* pVal)
	{
		TImageSize tFmt;
		HRESULT hRes = m_pImg->CanvasGet(&tFmt, NULL, NULL, NULL, NULL);
		*pVal = tFmt.nY;
		return hRes;
	}
	STDMETHOD(put_sizeY)(ULONG val)
	{
		TImageSize tFmt;
		HRESULT hRes = m_pImg->CanvasGet(&tFmt, NULL, NULL, NULL, NULL);
		if (tFmt.nY != val)
		{
			tFmt.nY = val;
			return m_pImg->CanvasSet(&tFmt, NULL, NULL, NULL);
		}
		return S_FALSE;
	}
	STDMETHOD(GetLayerBackground)(ULONG index, ULONG* pRgba)
	{
		try
		{
			*pRgba = 0;
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			CComPtr<ISubDocumentID> pSDI;
			m_pDIL->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDI));
			CComPtr<IDocument> pDoc;
			pSDI->SubDocumentGet(&pDoc);
			CComQIPtr<IDocumentEditableImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pImage));
			CPixelChannel tDef;
			pImage->ChannelsGet(NULL, NULL, &CImageChannelDefaultGetter(EICIRGBA, &tDef));
			*pRgba = tDef.bR|(ULONG(tDef.bG)<<8)|(ULONG(tDef.bB)<<16)|(ULONG(tDef.bA)<<24);
			return S_OK;
		}
		catch (...)
		{
			return pRgba ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetLayerBackground)(ULONG index, ULONG rgba)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			CComPtr<ISubDocumentID> pSDI;
			m_pDIL->ItemFeatureGet(pItem, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDI));
			CComPtr<IDocument> pDoc;
			pSDI->SubDocumentGet(&pDoc);
			CComQIPtr<IDocumentEditableImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pImage));
			CImageChannelReader cReader;
			pImage->ChannelsGet(NULL, NULL, &cReader);
			for (size_t i = 0; i < cReader.m_aChIDs.size(); ++i)
			{
				if (cReader.m_aChIDs[i] == EICIRGBA)
				{
					cReader.m_aDefs[i].bR = rgba;
					cReader.m_aDefs[i].bG = rgba>>8;
					cReader.m_aDefs[i].bB = rgba>>16;
					cReader.m_aDefs[i].bA = rgba>>24;
				}
			}
			pImage->ChannelsSet(cReader.m_aChIDs.size(), &(cReader.m_aChIDs[0]), &(cReader.m_aDefs[0]));
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHODIMP get_resolutionXNum(ULONG* pVal)
	{
		try
		{
			TImageResolution tRes = {100, 254, 100, 254};
			m_pImg->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
			*pVal = tRes.nNumeratorX;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHODIMP get_resolutionXDenom(ULONG* pVal)
	{
		try
		{
			TImageResolution tRes = {100, 254, 100, 254};
			m_pImg->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
			*pVal = tRes.nDenominatorX;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHODIMP get_resolutionYNum(ULONG* pVal)
	{
		try
		{
			TImageResolution tRes = {100, 254, 100, 254};
			m_pImg->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
			*pVal = tRes.nNumeratorY;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	STDMETHODIMP get_resolutionYDenom(ULONG* pVal)
	{
		try
		{
			TImageResolution tRes = {100, 254, 100, 254};
			m_pImg->CanvasGet(NULL, &tRes, NULL, NULL, NULL);
			*pVal = tRes.nDenominatorY;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHODIMP SetResolution(ULONG a_nResXNum, ULONG a_nResXDenom, ULONG a_nResYNum, ULONG a_nResYDenom)
	{
		try
		{
			TImageResolution const tRes = {a_nResXNum, a_nResXDenom, a_nResYNum, a_nResYDenom};
			return m_pImg->CanvasSet(NULL, &tRes, NULL, NULL);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(IsLayerEnabled)(ULONG index, boolean* pVal)
	{
		try
		{
			*pVal = VARIANT_TRUE;
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			return m_pDIL->LayerPropsGet(pItem, NULL, pVal);
		}
		catch (...)
		{
			return pVal ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(EnableLayer)(ULONG index, boolean enabled)
	{
		try
		{
			CComPtr<IEnumUnknowns> pItems;
			m_pDIL->ItemsEnum(NULL, &pItems);
			CComPtr<IComparable> pItem;
			pItems->Get(index, __uuidof(IComparable), reinterpret_cast<void**>(&pItem));
			BYTE b = enabled;
			return m_pDIL->LayerPropsSet(pItem, NULL, &b);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class CImageLayerCreatorWizard :
		public IImageLayerCreator
	{
	public:
		CImageLayerCreatorWizard(REFGUID a_tWizardGUID, IConfig* a_pConfig, HWND a_hWnd, LCID a_tLocaleID, ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders) :
			m_tWizardGUID(a_tWizardGUID), m_pConfig(a_pConfig), m_hWnd(a_hWnd), m_tLocaleID(a_tLocaleID), m_nBuilders(a_nBuilders), m_apBuilders(a_apBuilders)
		{
		}

		// IUnknown methods
	public:
		STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
		{
			if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
			{
				*a_ppvObject = this;
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)() { return 2; }
		STDMETHOD_(ULONG, Release)() { return 1; }

		// IImageLayerCreator methods
	public:
		STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
		{
			CComPtr<IDesignerWizard> pDC;
			RWCoCreateInstance(pDC, m_tWizardGUID);
			if (pDC == NULL)
				return E_FAIL;
			return pDC->Activate(m_hWnd, m_tLocaleID, m_pConfig, m_nBuilders, m_apBuilders, a_bstrID, a_pBase);
		}

	private:
		REFGUID m_tWizardGUID;
		CComPtr<IConfig> m_pConfig;
		HWND m_hWnd;
		LCID m_tLocaleID;
		ULONG m_nBuilders;
		IDocumentBuilder* const* m_apBuilders;
	};

	class CImageChannelReader : public IEnumImageChannels
	{
		// IUnknown methods
	public:
		STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
		{
			if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IEnumImageChannels)))
			{
				*a_ppvObject = this;
				return S_OK;
			}
			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG, AddRef)() { return 2; }
		STDMETHOD_(ULONG, Release)() { return 1; }

		// IEnumImageChannels methods
	public:
		STDMETHOD(Range)(ULONG* a_pStart, ULONG* a_pCount)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Consume)(ULONG a_nStart, ULONG a_nCount, TChannelDefault const* a_aChannelDefaults)
		{
			for (; a_nCount > 0; ++a_aChannelDefaults, --a_nCount)
			{
				m_aChIDs.push_back(a_aChannelDefaults->eID);
				m_aDefs.push_back(a_aChannelDefaults->tValue);
			}
			return S_OK;
		}

	public:
		std::vector<EImageChannelID> m_aChIDs;
		std::vector<TPixelChannel> m_aDefs;
	};

private:
	LONG IndexFromID(IComparable* a_pNew)
	{
		if (a_pNew == NULL)
			return -1;
		CComPtr<IEnumUnknowns> pItems;
		m_pDIL->ItemsEnum(NULL, &pItems);
		CComPtr<IComparable> pItem;
		for (ULONG i = 0; SUCCEEDED(pItems->Get(i, __uuidof(IComparable), reinterpret_cast<void**>(&pItem))); ++i)
		{
			if (S_OK == pItem->Compare(a_pNew))
				return i;
			pItem = NULL;
		}
		return -1;
	}

private:
	CComPtr<IScriptingInterfaceManager> m_pMgr;
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentLayeredImage> m_pDIL;
	CComPtr<IDocumentEditableImage> m_pImg;
	HWND m_hWnd;
	LCID m_tLocaleID;
};

