
#pragma once


class CScriptedRasterImageEditWindow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditWindow
{
public:
	void Init(IDocument* a_pDoc, IDocumentRasterImage* a_pRI, IRasterImageEditTool* a_pTool)
	{
		m_pDoc = a_pDoc;
		m_pRI = a_pRI;
		m_pTool = a_pTool;
	}

BEGIN_COM_MAP(CScriptedRasterImageEditWindow)
	COM_INTERFACE_ENTRY(IRasterImageEditWindow)
END_COM_MAP()

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		try
		{
			TImageSize tSize = {0, 0};
			m_pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			*a_pSizeX = tSize.nX;
			*a_pSizeY = tSize.nY;
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		return m_pRI->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, reinterpret_cast<TPixelChannel*>(a_pDefault)));
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent UNREF(a_eIntent), TRasterImagePixel* a_pBuffer)
	{
		try
		{
			TImagePoint const t1 = {a_nX, a_nY};
			TImageSize const t2 = {a_nSizeX, a_nSizeY};
			TImageStride const tS = {1, a_nStride};
			return m_pRI->TileGet(EICIRGBA, &t1, &t2, &tS, a_nSizeX*a_nSizeY, reinterpret_cast<TPixelChannel*>(a_pBuffer), NULL, EIRIAccurate);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		try
		{
			if (a_pBoundingRectangle)
			{
				TImageSize tSize = {0, 0};
				m_pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				a_pBoundingRectangle->left = 0;
				a_pBoundingRectangle->top = 0;
				a_pBoundingRectangle->right = tSize.nX;
				a_pBoundingRectangle->bottom = tSize.nY;
			}
			if (a_bEntireRectangle)
			{
				*a_bEntireRectangle = TRUE;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		try
		{
			FillMemory(a_pBuffer, a_nStride*a_nSizeY, 0xff);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return S_FALSE;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return S_FALSE;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return S_FALSE;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		return S_FALSE;
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return S_FALSE;
	}
	STDMETHOD(ApplyChanges)()
	{
		try
		{
			CComQIPtr<IRasterImageEditToolCustomApply> pCustomApply(m_pTool);
			if (pCustomApply)
			{
				HRESULT hRes = pCustomApply->ApplyChanges(TRUE);
				if (SUCCEEDED(hRes))
				{
					m_pTool->Reset();
					return hRes;
				}
			}
			TImageSize tSize = {0, 0};
			m_pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			float fGamma = 2.2f;
			m_pRI->ChannelsGet(NULL, &fGamma, NULL);
			RECT rcImage = {0x7fffffff, 0x7fffffff, 0x80000000, 0x80000000};
			BOOL bOptimize = FALSE;
			m_pTool->IsDirty(&rcImage, &bOptimize, NULL);
			if (rcImage.left < 0) rcImage.left = 0;
			if (rcImage.top < 0) rcImage.top = 0;
			if (rcImage.right > LONG(tSize.nX)) rcImage.right = tSize.nX;
			if (rcImage.bottom > LONG(tSize.nY)) rcImage.bottom = tSize.nY;
			if (rcImage.left < rcImage.right && rcImage.top < rcImage.bottom)
			{
				ULONG const nSizeX = (rcImage.right-rcImage.left);
				ULONG const nSizeY = (rcImage.bottom-rcImage.top);
				ULONG const nPixels = nSizeX*nSizeY;
				TImagePoint const t1 = {rcImage.left, rcImage.top};
				TImageSize const t21 = {rcImage.right-rcImage.left, rcImage.bottom-rcImage.top};
				if (bOptimize && nPixels > 64)
				{
					CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nPixels*2]);
					m_pTool->GetImageTile(rcImage.left, rcImage.top, nSizeX, nSizeY, fGamma, nSizeX, cBuffer);
					m_pRI->TileGet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel*>(cBuffer.m_p+nPixels), NULL, EIRIAccurate);
					ATLASSERT(sizeof(DWORD) == sizeof(TRasterImagePixel));
					DWORD* const p1 = reinterpret_cast<DWORD*>(cBuffer.m_p);
					DWORD const* const p2 = p1+nPixels;
					ULONG nDifferent = 0;
					for (ULONG i = 0; i < nPixels; ++i)
						if (p1[i] != p2[i])
							++nDifferent;
					if (nDifferent*3 > nPixels)
					{
						m_pRI->TileSet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(cBuffer.m_p), FALSE);
					}
					else
					{
						CWriteLock<IDocument> pLock(m_pDoc);
						ULONG i = 0;
						for (ULONG y = 0; y < nSizeY; ++y)
						{
							TImagePoint tPt = {0, y+t1.nY};
							TImageSize tSz = {0, 1};
							DWORD const* pStart = NULL;
							for (ULONG x = 0; x < nSizeX; ++x, ++i)
							{
								if (p1[i] != p2[i])
								{
									if (pStart == NULL)
									{
										tPt.nX = x+t1.nX;
										pStart = p1+i;
									}
								}
								else if (pStart)
								{
									tSz.nX = x+t1.nX-tPt.nX;
									m_pRI->TileSet(EICIRGBA, &tPt, &tSz, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(pStart), FALSE);
									pStart = NULL;
								}
							}
							if (pStart)
							{
								tSz.nX = nSizeX+t1.nX-tPt.nX;
								m_pRI->TileSet(EICIRGBA, &tPt, &tSz, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(pStart), FALSE);
								pStart = NULL;
							}
						}
					}
				}
				else
				{
					CAutoVectorPtr<TRasterImagePixel> cBuffer(new TRasterImagePixel[nPixels]);
					m_pTool->GetImageTile(rcImage.left, rcImage.top, nSizeX, nSizeY, fGamma, nSizeX, cBuffer);
					m_pRI->TileSet(EICIRGBA, &t1, &t21, NULL, nPixels, reinterpret_cast<TPixelChannel const*>(cBuffer.m_p), FALSE);
				}
			}
			m_pTool->Reset();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return S_FALSE;
	}
	STDMETHOD(SetColors)(TColor const* a_pColor1, TColor const* a_pColor2)
	{
		return S_FALSE; // TODO: update colors?
	}
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		return S_FALSE;
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		return E_NOTIMPL; // hm
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		try
		{
			*a_ppDocument = m_pDoc;
			(*a_ppDocument)->AddRef();
			return S_OK;
		}
		catch (...)
		{
			return a_ppDocument ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(Checkpoint)()
	{
		return E_NOTIMPL;
	}

private:
	CComPtr<IDocument> m_pDoc;
	CComPtr<IDocumentRasterImage> m_pRI;
	IRasterImageEditTool* m_pTool;
};

