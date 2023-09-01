// DocumentRasterImage.cpp : Implementation of CDocumentRasterImage

#include "stdafx.h"
#include "DocumentRasterImage.h"

#include <algorithm>
#include <RWImagingDocumentUtils.h>
#include <math.h>


// CDocumentRasterImage

void CDocumentRasterImage::Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault)
{
	m_tSize = a_tSize;
	m_fGamma = 2.2f;
	if (a_pDefault)
		m_tDefault = *a_pDefault;
	if (a_pResolution)
		m_tResolution = *a_pResolution;
}

void CDocumentRasterImage::Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault, TImagePoint const& a_tContentOrigin, TImageSize const& a_tContentSize, CAutoVectorPtr<TPixelChannel>& a_pPixels, float a_fGamma, IImageMetaData* a_pMetaData, ULONGLONG const* a_pTotalAlpha, ULONG a_nPreviewScale)
{
	m_bNoUndo = a_nPreviewScale > 1;
	m_tSize = a_tSize;
	m_fGamma = a_fGamma;
	if (a_pDefault)
		m_tDefault = *a_pDefault;
	if (a_pResolution)
		m_tResolution = *a_pResolution;
	m_tContentOrigin = m_tAllocOrigin = a_tContentOrigin;
	m_tContentSize = m_tAllocSize = a_tContentSize;
	m_pData = a_pPixels.Detach();
	TPixelChannel const* p = m_pData;
	if (a_pTotalAlpha)
		m_qwAlphaTotal = *a_pTotalAlpha;
	else
	{
		m_qwAlphaTotal = 0;
		for (TPixelChannel const* const pEnd = p+m_tContentSize.nX*m_tContentSize.nY; p < pEnd; ++p)
			m_qwAlphaTotal += p->bA;
	}
	m_bPrevAlphaState = m_qwAlphaTotal != ULONGLONG(m_tContentSize.nX*m_tContentSize.nY)*ULONGLONG(255);

	if (a_pMetaData)
		CopyMetaData(a_pMetaData);
}

void CDocumentRasterImage::Init(TImageSize const& a_tSize, TImageResolution const* a_pResolution, TPixelChannel const* a_pDefault, TImagePoint const& a_tContentOrigin, TImageSize const& a_tContentSize, TPixelChannel const* a_pPixels, float a_fGamma, IImageMetaData* a_pMetaData, ULONG const* a_pStride)
{
	ULONG nSize = a_tContentSize.nX*a_tContentSize.nY;
	CAutoVectorPtr<TPixelChannel> pPixels(nSize ? new TPixelChannel[nSize] : NULL);
	if (a_pStride && *a_pStride != a_tContentSize.nX)
		for (ULONG y = 0; y < a_tContentSize.nY; ++y)
			std::copy(a_pPixels+y**a_pStride, a_pPixels+a_tContentSize.nX+y**a_pStride, pPixels.m_p+y*a_tContentSize.nX);
	else
		std::copy(a_pPixels, a_pPixels+nSize, pPixels.m_p);
	Init(a_tSize, a_pResolution, a_pDefault, a_tContentOrigin, a_tContentSize, pPixels, a_fGamma, a_pMetaData);
}

STDMETHODIMP CDocumentRasterImage::DataCopy(BSTR a_bstrPrefix, IDocumentBase* a_pBase, CLSID* a_tPreviewEffectID, IConfig* a_pPreviewEffect)
{
	try
	{
		//CDocumentReadLock cLock(this); // TODO: should be read lock, but that causes problems with copying sub-doc (need hierarchical locks)

		CComObject<CDocumentRasterImage>* p = NULL;
		CComObject<CDocumentRasterImage>::CreateInstance(&p);
		CComPtr<IDocumentData> pTmp = p;
		CDocumentRasterImage* pDoc = p;

		pDoc->m_tSize = m_tSize;
		pDoc->m_fGamma = m_fGamma;
		pDoc->m_tContentSize = m_tContentSize;
		pDoc->m_tContentOrigin = m_tContentOrigin;
		pDoc->m_tDefault = m_tDefault;
		if (a_tPreviewEffectID == NULL)
		{
			pDoc->m_tAllocSize = m_tAllocSize;
			pDoc->m_tAllocOrigin = m_tAllocOrigin;
			pDoc->m_pData = new TPixelChannel[m_tAllocSize.nX*m_tAllocSize.nY];
			CopyMemory(pDoc->m_pData, m_pData, m_tAllocSize.nX*m_tAllocSize.nY*sizeof*m_pData);
			pDoc->m_qwAlphaTotal = m_qwAlphaTotal;
			pDoc->m_bNoUndo = m_bNoUndo;
		}
		else
		{
			int nFactor = 1;
			while (pDoc->m_tSize.nX*pDoc->m_tSize.nY > 0x50000)
			{
				pDoc->m_tSize.nX = (pDoc->m_tSize.nX+1)>>1;
				pDoc->m_tSize.nY = (pDoc->m_tSize.nY+1)>>1;
				pDoc->m_tContentSize.nX = (pDoc->m_tContentSize.nX+1)>>1;
				pDoc->m_tContentSize.nY = (pDoc->m_tContentSize.nY+1)>>1;
				nFactor <<= 1;
			}
			pDoc->m_tContentOrigin.nX = m_tContentOrigin.nX/nFactor;
			pDoc->m_tContentOrigin.nY = m_tContentOrigin.nY/nFactor;
			pDoc->m_tAllocOrigin = pDoc->m_tContentOrigin;
			pDoc->m_tAllocSize = pDoc->m_tContentSize;

			pDoc->m_pData = new TPixelChannel[pDoc->m_tAllocSize.nX*pDoc->m_tAllocSize.nY];
			pDoc->m_qwAlphaTotal = 0;
			TPixelChannel* pD = pDoc->m_pData;
			for (ULONG y = 0; y < pDoc->m_tContentSize.nY; ++y)
			{
				TPixelChannel const* pS = m_pData+(y*nFactor+m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX;
				for (ULONG x = 0; x < pDoc->m_tContentSize.nX; ++x)
				{
					*pD = *pS;
					pDoc->m_qwAlphaTotal += pS->bA;
					++pD;
					pS += nFactor;
				}
			}

			if (a_pPreviewEffect)
			{
				CComPtr<IRasterImageFilter> pRIF;
				RWCoCreateInstance(pRIF, *a_tPreviewEffectID);
				if (pRIF)
				{
					TMatrix3x3f trans =
					{
						1.0f/nFactor, 0, 0,
						0, 1.0f/nFactor, 0,
						0, 0, 1
					};
					pRIF->Transform(a_pPreviewEffect, &pDoc->m_tSize, &trans);
				}
			}

			pDoc->m_bNoUndo = true;
		}
		pDoc->m_tResolution = m_tResolution;
		//pDoc->UndoModeInit(a_eHint == EDCHForPreview ? EUMDisabled : M_UndoMode());
		if (M_MetaData())
			pDoc->CopyMetaData(M_MetaData());

		return a_pBase->DataBlockSet(a_bstrPrefix, pTmp);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <PrintfLocalizedString.h>
#include <MultiLanguageString.h>

STDMETHODIMP CDocumentRasterImage::QuickInfo(ULONG a_nInfoIndex, ILocalizedString** a_ppInfo)
{
	try
	{
		if (a_nInfoIndex != 0)
			return E_RW_INDEXOUTOFRANGE;
		*a_ppInfo = NULL;
		CComPtr<ILocalizedString> pTempl;
		pTempl.Attach(new CMultiLanguageString(L"[0409]%ix%i pixels[0405]%ix%i pixelů"));
		CComObject<CPrintfLocalizedString>* pPFStr = NULL;
		CComObject<CPrintfLocalizedString>::CreateInstance(&pPFStr);
		CComPtr<ILocalizedString> pStr = pPFStr;
		pPFStr->Init(pTempl, m_tSize.nX, m_tSize.nY);
		*a_ppInfo = pStr.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::MaximumUndoSize(ULONGLONG* a_pnMaximumSize)
{
	ULONGLONG n = m_tSize.nX*m_tSize.nY;
	n *= 4*3;
	*a_pnMaximumSize = min(0x10000000, n);
	return S_OK;
}

STDMETHODIMP CDocumentRasterImage::ResourcesManage(EDocumentResourceManager a_eActions, ULONGLONG* a_pValue)
{
	if (a_eActions & EDRMGetMemoryUsage && a_pValue)
	{
		// TODO: optimize allocated aread?
		*a_pValue = m_tAllocSize.nX*m_tAllocSize.nY*sizeof*m_pData;
	}
	return S_OK;
}

STDMETHODIMP CDocumentRasterImage::Aspects(IEnumEncoderAspects* a_pEnumAspects)
{
	try
	{
		float aVals[4] = {100.0f, 0.1f, 0.1f, 0.1f};
		BSTR aIDs[4];
		ULONG n = 0;
		aIDs[n++] = m_ENCFEAT_IMAGE;
		if (m_qwAlphaTotal != ULONGLONG(m_tContentSize.nX*m_tContentSize.nY)*ULONGLONG(255))
			aIDs[n++] = m_ENCFEAT_IMAGE_ALPHA;
		if (MetaDataPresent())
			aIDs[n++] = m_ENCFEAT_IMAGE_META;
		if (m_tContentSize.nX*m_tContentSize.nY && (
			m_tContentOrigin.nX < 0 || m_tContentOrigin.nY < 0 ||
			LONG(m_tContentOrigin.nX+m_tContentSize.nX) > LONG(m_tSize.nX) ||
			LONG(m_tContentOrigin.nY+m_tContentSize.nY) > LONG(m_tSize.nY)))
		{
			aVals[n] = 2.0f;
			aIDs[n++] = ENCFEAT_IMAGE_CANVAS;
		}
		a_pEnumAspects->Consume(0, n, aIDs, aVals);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::CanvasGet(TImageSize* a_pCanvasSize, TImageResolution* a_pResolution, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, EImageOpacity* a_pContentOpacity)
{
	try
	{
		if (a_pCanvasSize) *a_pCanvasSize = m_tSize;
		if (a_pResolution) *a_pResolution = m_tResolution;
		if (a_pContentOrigin) *a_pContentOrigin = m_tContentOrigin;
		if (a_pContentSize) *a_pContentSize = m_tContentSize;
		if (a_pContentOpacity)
		{
			if (m_qwAlphaTotal == 0)
				*a_pContentOpacity = EIOTransparent;
			else if (m_qwAlphaTotal == 255ULL*m_tContentSize.nX*m_tContentSize.nY)
				*a_pContentOpacity = EIOOpaque;
			else
				*a_pContentOpacity = EIOSemiTransparent;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::ChannelsGet(ULONG* a_pChannelIDs, float* a_pGamma, IEnumImageChannels* a_pChannelDefaults)
{
	try
	{
		if (a_pChannelIDs) *a_pChannelIDs = EICIRGBA;
		if (a_pGamma) *a_pGamma = m_fGamma;
		if (a_pChannelDefaults) a_pChannelDefaults->Consume(0, 1, CChannelDefault(EICIRGBA, m_tDefault));
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::TileGet(ULONG a_nChannelID, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel* a_pData, ITaskControl* UNREF(a_pControl), EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelID != EICIRGBA)
			return E_RW_INVALIDPARAM;
		return RGBAGetTileImpl(m_pThPool, m_tSize, m_tContentOrigin, m_tContentSize, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tAllocSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pStride, a_nPixels, a_pData);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::Inspect(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, IImageVisitor* a_pVisitor, ITaskControl* a_pControl, EImageRenderingIntent UNREF(a_eIntent))
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_RW_INVALIDPARAM;
		return RGBAInspectImpl(m_tContentOrigin, m_tContentSize, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tAllocSize.nX, m_tDefault, a_pOrigin, a_pSize, a_pVisitor, a_pControl);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::BufferLock(ULONG a_nChannelID, TImagePoint* a_pAllocOrigin, TImageSize* a_pAllocSize, TImagePoint* a_pContentOrigin, TImageSize* a_pContentSize, TPixelChannel const** a_ppBuffer, ITaskControl* a_pControl, EImageRenderingIntent a_eIntent)
{
	if (a_nChannelID != EICIRGBA)
		return E_RW_INVALIDPARAM;
	try
	{
		if (a_pAllocOrigin) *a_pAllocOrigin = m_tAllocOrigin;
		if (a_pAllocSize) *a_pAllocSize = m_tAllocSize;
		if (a_pContentOrigin) *a_pContentOrigin = m_tContentOrigin;
		if (a_pContentSize) *a_pContentSize = m_tContentSize;
		if (a_ppBuffer)
		{
			M_Base()->ReadLock();
			*a_ppBuffer = m_pData;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::BufferUnlock(ULONG a_nChannelID, TPixelChannel const* a_pBuffer)
{
	if (a_nChannelID != EICIRGBA || a_pBuffer != m_pData)
		return E_RW_INVALIDPARAM;
	try
	{
		M_Base()->ReadUnlock();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

ULONG gcd(ULONG a, ULONG b)
{
	while (a != b)
		if (a < b)
			b -= a*((b-1)/a);
		else
			a -= b*((a-1)/b);
	return a;
}

STDMETHODIMP CDocumentRasterImage::CanvasSet(TImageSize const* a_pSize, TImageResolution const* a_pResolution, TMatrix3x3f const* a_pContentTransform, IRasterImageTransformer* a_pHelper)
{
	try
	{
		CDocumentWriteLock cLock(this);
		if (a_pSize && (a_pSize->nX != m_tSize.nX || a_pSize->nY != m_tSize.nY))
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoCanvasSize::Add(M_Base(), this, m_tSize);
			}

			m_tSize = *a_pSize;
			m_bResized = true;
		}
		if (a_pResolution &&
			(m_tResolution.nNumeratorX != a_pResolution->nNumeratorX ||
			 m_tResolution.nDenominatorX != a_pResolution->nDenominatorX ||
			 m_tResolution.nNumeratorY != a_pResolution->nNumeratorY ||
			 m_tResolution.nDenominatorY != a_pResolution->nDenominatorY))
		{
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoResolution::Add(M_Base(), this, m_tResolution);
			}

			m_tResolution = *a_pResolution;
			m_bResolutionChange = true;
		}
		if (a_pContentTransform)
		{
			if (a_pContentTransform->_11 == 1.0f && a_pContentTransform->_12 == 0.0f && a_pContentTransform->_13 == 0.0f &&
				a_pContentTransform->_21 == 0.0f && a_pContentTransform->_22 == 1.0f && a_pContentTransform->_23 == 0.0f &&
				a_pContentTransform->_33 == 1.0f)
			{
				// move content rectangle
				LONG nDX = floor(a_pContentTransform->_31+0.5);
				LONG nDY = floor(a_pContentTransform->_32+0.5);
				if ((nDX || nDY) && m_tContentSize.nX && m_tContentSize.nY)
				{
					if (M_Base()->UndoEnabled() == S_OK)
					{
						CUndoContentMove::Add(M_Base(), this, -nDX, -nDY);
					}

					AddDirtyRect(m_tContentOrigin, m_tContentSize);
					m_tContentOrigin.nX += nDX;
					m_tContentOrigin.nY += nDY;
					m_tAllocOrigin.nX += nDX;
					m_tAllocOrigin.nY += nDY;
					AddDirtyRect(m_tContentOrigin, m_tContentSize);
				}
			}
			else
			{
				// resample content
				TVector2f canvas = {m_tSize.nX, m_tSize.nY};
				canvas = TransformVector2(*a_pContentTransform, canvas);
				TImageSize newCanvas = {canvas.x+0.5f, canvas.y+0.5f};
				if (newCanvas.nX == 0) newCanvas.nX = 1;
				if (newCanvas.nY == 0) newCanvas.nY = 1;
				TImageSize unit = {gcd(newCanvas.nX, m_tSize.nX), gcd(newCanvas.nY, m_tSize.nY)};

				TImagePoint tSrc00 = {unit.nX*LONG(floorf(float(m_tContentOrigin.nX)/unit.nX)), unit.nY*LONG(floorf(float(m_tContentOrigin.nY)/unit.nY))};
				TImagePoint tSrc11 = {unit.nX*LONG(ceilf(float(m_tContentOrigin.nX+m_tContentSize.nX)/unit.nX)), unit.nY*LONG(ceilf(float(m_tContentOrigin.nY+m_tContentSize.nY)/unit.nY))};

				TVector2f t00 = {tSrc00.nX, tSrc00.nY};
				TVector2f t11 = {tSrc11.nX, tSrc11.nY};
				TVector2f t01 = {t00.x, t11.y};
				TVector2f t10 = {t11.x, t00.y};
				t00 = TransformVector2(*a_pContentTransform, t00);
				t01 = TransformVector2(*a_pContentTransform, t01);
				t10 = TransformVector2(*a_pContentTransform, t10);
				t11 = TransformVector2(*a_pContentTransform, t11);
				TImagePoint tNewOrigin = {floor(t00.x), floor(t00.y)};
				if (floor(t01.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t01.x);
				if (floor(t01.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t01.y);
				if (floor(t10.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t10.x);
				if (floor(t10.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t10.y);
				if (floor(t11.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t11.x);
				if (floor(t11.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t11.y);
				TImagePoint tNewEnd = {ceil(t00.x), ceil(t00.y)};
				if (ceil(t01.x) > tNewEnd.nX) tNewEnd.nX = ceil(t01.x);
				if (ceil(t01.y) > tNewEnd.nY) tNewEnd.nY = ceil(t01.y);
				if (ceil(t10.x) > tNewEnd.nX) tNewEnd.nX = ceil(t10.x);
				if (ceil(t10.y) > tNewEnd.nY) tNewEnd.nY = ceil(t10.y);
				if (ceil(t11.x) > tNewEnd.nX) tNewEnd.nX = ceil(t11.x);
				if (ceil(t11.y) > tNewEnd.nY) tNewEnd.nY = ceil(t11.y);
				TImageSize const tNewSize = {tNewEnd.nX-tNewOrigin.nX, tNewEnd.nY-tNewOrigin.nY};
				ULONG const nPixels = tNewSize.nX*tNewSize.nY;
				CAutoVectorPtr<TPixelChannel> cResampled(nPixels ? new TPixelChannel[nPixels] : NULL);
				if (nPixels)
				{
					if (tSrc00.nX >= m_tAllocOrigin.nX && tSrc00.nY >= m_tAllocOrigin.nY &&
						tSrc11.nX <= LONG(m_tAllocOrigin.nX+m_tAllocSize.nX) &&
						tSrc11.nY <= LONG(m_tAllocOrigin.nY+m_tAllocSize.nY))
					{
						a_pHelper->ProcessTile(EICIRGBA, m_fGamma, &m_tDefault, a_pContentTransform, m_tAllocSize.nX*m_tContentSize.nY, m_pData+m_tAllocSize.nX*(m_tContentOrigin.nY-m_tAllocOrigin.nY)+m_tContentOrigin.nX-m_tAllocOrigin.nX, &m_tContentOrigin, &m_tContentSize, CImageStride(1, m_tAllocSize.nX), tNewSize.nX*tNewSize.nY, cResampled, &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));
					}
					else
					{
						TImageSize size = {tSrc11.nX-tSrc00.nX, tSrc11.nY-tSrc00.nY};
						CAutoVectorPtr<TPixelChannel> src(new TPixelChannel[size.nX*size.nY]);
						TileGet(EICIRGBA, &tSrc00, &size, NULL, size.nX*size.nY, src, NULL, EIRIAccurate);
						a_pHelper->ProcessTile(EICIRGBA, m_fGamma, &m_tDefault, a_pContentTransform, size.nX*size.nY, src, &tSrc00, &size, CImageStride(1, size.nX), tNewSize.nX*tNewSize.nY, cResampled, &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));
					}
				}

				TImagePoint tDst00 = {unit.nX*LONG(floorf(float(m_tContentOrigin.nX)/unit.nX)), unit.nY*LONG(floorf(float(m_tContentOrigin.nY)/unit.nY))};
				TImagePoint tDst11 = {unit.nX*LONG(ceilf(float(m_tContentOrigin.nX)/unit.nX)), unit.nY*LONG(ceilf(float(m_tContentOrigin.nY)/unit.nY))};
/*
				TVector2f t00 = {m_tContentOrigin.nX, m_tContentOrigin.nY};
				TVector2f t11 = {m_tContentOrigin.nX+m_tContentSize.nX, m_tContentOrigin.nY+m_tContentSize.nY};
				TVector2f t01 = {t00.x, t11.y};
				TVector2f t10 = {t11.x, t00.y};
				t00 = TransformVector2(*a_pContentTransform, t00);
				t01 = TransformVector2(*a_pContentTransform, t01);
				t10 = TransformVector2(*a_pContentTransform, t10);
				t11 = TransformVector2(*a_pContentTransform, t11);
				TImagePoint tNewOrigin = {floor(t00.x), floor(t00.y)};
				if (floor(t01.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t01.x);
				if (floor(t01.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t01.y);
				if (floor(t10.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t10.x);
				if (floor(t10.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t10.y);
				if (floor(t11.x) < tNewOrigin.nX) tNewOrigin.nX = floor(t11.x);
				if (floor(t11.y) < tNewOrigin.nY) tNewOrigin.nY = floor(t11.y);
				TImagePoint tNewEnd = {ceil(t00.x), ceil(t00.y)};
				if (ceil(t01.x) > tNewEnd.nX) tNewEnd.nX = ceil(t01.x);
				if (ceil(t01.y) > tNewEnd.nY) tNewEnd.nY = ceil(t01.y);
				if (ceil(t10.x) > tNewEnd.nX) tNewEnd.nX = ceil(t10.x);
				if (ceil(t10.y) > tNewEnd.nY) tNewEnd.nY = ceil(t10.y);
				if (ceil(t11.x) > tNewEnd.nX) tNewEnd.nX = ceil(t11.x);
				if (ceil(t11.y) > tNewEnd.nY) tNewEnd.nY = ceil(t11.y);
				TImageSize const tNewSize = {tNewEnd.nX-tNewOrigin.nX, tNewEnd.nY-tNewOrigin.nY};
				ULONG const nPixels = tNewSize.nX*tNewSize.nY;
				CAutoVectorPtr<TPixelChannel> cResampled(nPixels ? new TPixelChannel[nPixels] : NULL);
				if (nPixels)
					a_pHelper->ProcessTile(EICIRGBA, m_fGamma, &m_tDefault, a_pContentTransform, m_tAllocSize.nX*m_tContentSize.nY, m_pData+m_tAllocSize.nX*(m_tContentOrigin.nY-m_tAllocOrigin.nY)+m_tContentOrigin.nX-m_tAllocOrigin.nX, &m_tContentOrigin, &m_tContentSize, CImageStride(1, m_tAllocSize.nX), tNewSize.nX*tNewSize.nY, cResampled, &tNewOrigin, &tNewSize, CImageStride(1, tNewSize.nX));
*/
				if (M_Base()->UndoEnabled() == S_OK)
				{
					if (m_tAllocSize.nX == m_tContentSize.nX && m_tAllocSize.nY == m_tContentSize.nY)
					{
						CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, m_pData, m_pfnDeleter, true);
						m_pData = NULL;
					}
					else
					{
						CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);;
						CopyRectangle(cContent, m_tContentSize.nX, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
						CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, cContent.m_p, &DefDeleter, true);
						cContent.Detach();
					}
				}

				AddDirtyRect(m_tContentOrigin, m_tContentSize);
				AddDirtyRect(tNewOrigin, tNewSize);
				(*m_pfnDeleter)(m_pData);
				m_pData = cResampled.Detach();
				m_pfnDeleter = &DefDeleter;
				m_tContentOrigin = m_tAllocOrigin = tNewOrigin;
				m_tContentSize = m_tAllocSize = tNewSize;
				OptimizeContent();

			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::ChannelsSet(ULONG a_nChannels, EImageChannelID const* a_aChannelIDs, TPixelChannel const* a_aChannelDefaults)
{
	try
	{
		if (a_nChannels != 1 || a_aChannelIDs[0] != EICIRGBA)
			return E_FAIL;

		CDocumentWriteLock cLock(this);
		if (a_aChannelDefaults[0].n != m_tDefault.n)
		{
			if (m_tAllocSize.nX != m_tContentSize.nX || m_tAllocSize.nY != m_tContentSize.nY)
			{
				// if there is extra allocated size having the old background color, reallocate the buffer
				CAutoVectorPtr<TPixelChannel> cNew;
				if (m_tContentSize.nX*m_tContentSize.nY)
				{
					cNew.Attach(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);
					CopyRectangle(cNew.m_p, m_tContentSize.nX,
						m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
				}
				(*m_pfnDeleter)(m_pData);
				m_pData = cNew.Detach();
				m_pfnDeleter = &DefDeleter;
				m_tAllocSize = m_tContentSize;
				m_tAllocOrigin = m_tContentOrigin;
			}

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CUndoChannels::Add(M_Base(), this, m_tDefault);
			}

			m_tDefault = a_aChannelDefaults[0];
			m_bDefaultChange = true;
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void CDocumentRasterImage::AddDirtyRect(TImagePoint const& a_tOrigin, TImageSize const& a_tSize)
{
	if (a_tSize.nX*a_tSize.nY == 0)
		return;
	if (m_tDirtyTL.nX > a_tOrigin.nX) m_tDirtyTL.nX = a_tOrigin.nX;
	if (m_tDirtyTL.nY > a_tOrigin.nY) m_tDirtyTL.nY = a_tOrigin.nY;
	if (m_tDirtyBR.nX < LONG(a_tOrigin.nX+a_tSize.nX)) m_tDirtyBR.nX = a_tOrigin.nX+a_tSize.nX;
	if (m_tDirtyBR.nY < LONG(a_tOrigin.nY+a_tSize.nY)) m_tDirtyBR.nY = a_tOrigin.nY+a_tSize.nY;
}

void CDocumentRasterImage::CopyRectangle(TPixelChannel* a_pDst, ULONG const a_nDstStrideY, TPixelChannel const* a_pSrc, ULONG const a_nSrcSizeX, ULONG const a_nSrcSizeY, ULONG const a_nSrcStrideX, ULONG const a_nSrcStrideY)
{
	for (ULONG y = 0; y < a_nSrcSizeY; ++y)
	{
		if (a_nSrcStrideX == 1)
		{
			CopyMemory(a_pDst, a_pSrc, a_nSrcSizeX*sizeof*a_pDst);
			a_pSrc += a_nSrcStrideY;
			a_pDst += a_nDstStrideY;
		}
		else if (a_nSrcStrideX)
		{
			for (ULONG x = 0; x < a_nSrcSizeX; ++x, ++a_pDst, a_pSrc+=a_nSrcStrideX)
				*a_pDst = *a_pSrc;
			a_pSrc += a_nSrcStrideY-a_nSrcSizeX*a_nSrcStrideX;
			a_pDst += a_nDstStrideY-a_nSrcSizeX;
		}
		else
		{
			std::fill_n(a_pDst, a_nSrcSizeX, *a_pSrc);
			a_pSrc += a_nSrcStrideY;
			a_pDst += a_nDstStrideY;
		}
	}
}

void CDocumentRasterImage::OptimizeContent(bool a_bForce)
{
	if (m_pData == NULL)
		return;

	// top
	while (m_tContentSize.nY)
	{
		TPixelChannel const* p = m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX;
		TPixelChannel const* const pEnd = p+m_tContentSize.nX;
		for (; p != pEnd; ++p)
			if (p->n != m_tDefault.n)
				break;
		if (p == pEnd)
		{
			++m_tContentOrigin.nY;
			--m_tContentSize.nY;
		}
		else
		{
			break;
		}
	}
	if (m_tContentSize.nY == 0)
	{
		m_tContentSize.nX = 0;
		m_tAllocSize = TIMGSIZE_NULL;
		m_tAllocOrigin = m_tContentOrigin = TIMGPOINT_NULL;
		(*m_pfnDeleter)(m_pData);
		m_pData = NULL;
		return;
	}

	// bottom
	while (true)
	{
		TPixelChannel const* p = m_pData+(m_tContentOrigin.nY+m_tContentSize.nY-1-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX;
		TPixelChannel const* const pEnd = p+m_tContentSize.nX;
		for (; p != pEnd; ++p)
			if (p->n != m_tDefault.n)
				break;
		if (p == pEnd)
		{
			--m_tContentSize.nY;
		}
		else
		{
			break;
		}
	}
	// left
	while (true)
	{
		TPixelChannel const* p = m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX;
		TPixelChannel const* const pEnd = p+m_tContentSize.nY*m_tAllocSize.nX;
		for (; p != pEnd; p+=m_tAllocSize.nX)
			if (p->n != m_tDefault.n)
				break;
		if (p == pEnd)
		{
			++m_tContentOrigin.nX;
			--m_tContentSize.nX;
		}
		else
		{
			break;
		}
	}
	// right
	while (true)
	{
		TPixelChannel const* p = m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX+m_tContentSize.nX-1-m_tAllocOrigin.nX;
		TPixelChannel const* const pEnd = p+m_tContentSize.nY*m_tAllocSize.nX;
		for (; p != pEnd; p+=m_tAllocSize.nX)
			if (p->n != m_tDefault.n)
				break;
		if (p == pEnd)
		{
			--m_tContentSize.nX;
		}
		else
		{
			break;
		}
	}

	ULONG const nAllocSize = m_tAllocSize.nX*m_tAllocSize.nY;
	ULONG const nContentSize = m_tContentSize.nX*m_tContentSize.nY;
	if (a_bForce ? (nContentSize < nAllocSize) : (nAllocSize > 64*64 && nContentSize < nAllocSize/3))
	{
		CAutoVectorPtr<TPixelChannel> cData(nContentSize ? new TPixelChannel[nContentSize] : NULL);
		CopyRectangle(cData.m_p, m_tContentSize.nX,
			m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
		m_tAllocOrigin = m_tContentOrigin;
		m_tAllocSize = m_tContentSize;
		(*m_pfnDeleter)(m_pData);
		m_pData = cData.Detach();
		m_pfnDeleter = &DefDeleter;
	}
}

STDMETHODIMP CDocumentRasterImage::TileSet(ULONG a_nChannelIDs, TImagePoint const* a_pOrigin, TImageSize const* a_pSize, TImageStride const* a_pStride, ULONG a_nPixels, TPixelChannel const* a_pPixels, BYTE a_bDeleteOldContent)
{
	try
	{
		if (a_nChannelIDs != EICIRGBA)
			return E_INVALIDARG;

		if (a_bDeleteOldContent)
		{
			// discard old content

			if (a_nPixels == 0 || (a_nPixels == 1 && a_pPixels->n == m_tDefault.n))
			{
				// clear everyting
				CDocumentWriteLock cLock(this);
				if (m_tContentSize.nX*m_tContentSize.nY > 0)
				{
					if (M_Base()->UndoEnabled() == S_OK)
					{
						if (m_tAllocSize.nX == m_tContentSize.nX && m_tAllocSize.nY == m_tContentSize.nY)
						{
							CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, m_pData, m_pfnDeleter, true);
							m_pData = NULL;
						}
						else
						{
							CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);;
							CopyRectangle(cContent, m_tContentSize.nX, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
							CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, cContent.m_p, &DefDeleter, true);
							cContent.Detach();
						}
					}

					AddDirtyRect(m_tContentOrigin, m_tContentSize);

					m_tContentSize = m_tAllocSize = TIMGSIZE_NULL;
					m_tContentOrigin = m_tAllocOrigin = TIMGPOINT_NULL;
					(*m_pfnDeleter)(m_pData);
					m_pData = NULL;
					m_qwAlphaTotal = 0;
				}
				return S_OK;
			}

			CDocumentWriteLock cLock(this);

			static TImagePoint const tPt0 = TIMGPOINT_NULL;
			if (a_pOrigin == NULL)
			{
				a_pOrigin = &tPt0;
				if (a_pSize == NULL)
					a_pSize = &m_tSize;
			}
			else if (a_pSize == NULL)
				return E_RW_INVALIDPARAM;
			TImageStride const tStr = {1, a_pSize->nX};
			if (a_pStride == NULL)
				a_pStride = &tStr;

			if (a_nPixels < a_pSize->nY*a_pStride->nY)
				return E_RW_INVALIDPARAM;

			CAutoVectorPtr<TPixelChannel> cData(new TPixelChannel[a_pSize->nX*a_pSize->nY]);
			if (a_pStride->nY == a_pSize->nX)
			{
				// direct copy
				CopyMemory(cData.m_p, a_pPixels, a_pSize->nX*a_pSize->nY*sizeof*a_pPixels);
			}
			else if (a_pStride->nY == 0)
			{
				// simple fill
				std::fill_n(cData.m_p, a_pSize->nX*a_pSize->nY, *a_pPixels);
			}
			else
			{
				// something more complex
				TPixelChannel* pD = cData;
				for (ULONG y = 0; y < a_pSize->nY; ++y)
				{
					for (ULONG x = 0; x < a_pSize->nX; ++x, ++pD, a_pPixels+=a_pStride->nX)
						*pD = *a_pPixels;
					a_pPixels += a_pStride->nY-a_pSize->nX*a_pStride->nX;
				}
			}
			AddDirtyRect(m_tContentOrigin, m_tContentSize);
			AddDirtyRect(*a_pOrigin, *a_pSize);

			if (M_Base()->UndoEnabled() == S_OK)
			{
				if (m_tAllocSize.nX == m_tContentSize.nX && m_tAllocSize.nY == m_tContentSize.nY)
				{
					CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, m_pData, m_pfnDeleter, true);
					m_pData = NULL;
				}
				else
				{
					CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);;
					CopyRectangle(cContent, m_tContentSize.nX, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
					CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, cContent.m_p, &DefDeleter, true);
					cContent.Detach();
				}
			}

			(*m_pfnDeleter)(m_pData);
			m_pData = cData.Detach();
			m_pfnDeleter = &DefDeleter;
			m_tAllocSize = m_tContentSize = *a_pSize;
			m_tAllocOrigin = m_tContentOrigin = *a_pOrigin;
			OptimizeContent();
			return S_OK;
		}

		// combine old and new content

		CDocumentWriteLock cLock(this);

		static TImagePoint const tPt0 = TIMGPOINT_NULL;
		if (a_pOrigin == NULL)
		{
			a_pOrigin = &tPt0;
			if (a_pSize == NULL)
				a_pSize = &m_tSize;
		}
		else if (a_pSize == NULL)
			return E_RW_INVALIDPARAM;
		TImageStride const tStr = {1, a_pSize->nX};
		if (a_pStride == NULL)
			a_pStride = &tStr;

		if (a_nPixels < a_pSize->nY*a_pStride->nY)
		{
			ATLASSERT(0);
			return E_RW_INVALIDPARAM;
		}

		TImagePoint const tEnd = {a_pOrigin->nX+a_pSize->nX, a_pOrigin->nY+a_pSize->nY};
		TImagePoint const tContentEnd = {m_tContentOrigin.nX+m_tContentSize.nX, m_tContentOrigin.nY+m_tContentSize.nY};
		TImagePoint const tAllocEnd = {m_tAllocOrigin.nX+m_tAllocSize.nX, m_tAllocOrigin.nY+m_tAllocSize.nY};

		AddDirtyRect(*a_pOrigin, *a_pSize);

		if (a_pOrigin->nX > m_tContentOrigin.nX && a_pOrigin->nY > m_tContentOrigin.nY &&
			tEnd.nX < tContentEnd.nX && tEnd.nY < tContentEnd.nY)
		{
			// new content within old content (no need to recompute content boundaries)
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[a_pSize->nX*a_pSize->nY]);;
				CopyRectangle(cContent, a_pSize->nX, m_pData + a_pOrigin->nX-m_tAllocOrigin.nX + m_tAllocSize.nX*(a_pOrigin->nY-m_tAllocOrigin.nY), a_pSize->nX, a_pSize->nY, 1, m_tAllocSize.nX);
				CUndoPixelTile::Add(M_Base(), this, *a_pOrigin, *a_pSize, cContent.m_p, &DefDeleter, false);
				cContent.Detach();
			}

			CopyRectangle(m_pData + a_pOrigin->nX-m_tAllocOrigin.nX + m_tAllocSize.nX*(a_pOrigin->nY-m_tAllocOrigin.nY), m_tAllocSize.nX,
				a_pPixels, a_pSize->nX, a_pSize->nY, a_pStride->nX, a_pStride->nY);
		}
		else if (a_pOrigin->nX >= m_tAllocOrigin.nX && a_pOrigin->nY >= m_tAllocOrigin.nY &&
			tEnd.nX <= tAllocEnd.nX && tEnd.nY <= tAllocEnd.nY)
		{
			// no need to re-alloc
			if (M_Base()->UndoEnabled() == S_OK)
			{
				CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[a_pSize->nX*a_pSize->nY]);;
				CopyRectangle(cContent, a_pSize->nX, m_pData + a_pOrigin->nX-m_tAllocOrigin.nX + m_tAllocSize.nX*(a_pOrigin->nY-m_tAllocOrigin.nY), a_pSize->nX, a_pSize->nY, 1, m_tAllocSize.nX);
				CUndoPixelTile::Add(M_Base(), this, *a_pOrigin, *a_pSize, cContent.m_p, &DefDeleter, false);
				cContent.Detach();
			}

			CopyRectangle(m_pData + a_pOrigin->nX-m_tAllocOrigin.nX + m_tAllocSize.nX*(a_pOrigin->nY-m_tAllocOrigin.nY), m_tAllocSize.nX,
				a_pPixels, a_pSize->nX, a_pSize->nY, a_pStride->nX, a_pStride->nY);
			if (m_tContentOrigin.nX > a_pOrigin->nX) m_tContentOrigin.nX = a_pOrigin->nX;
			if (m_tContentOrigin.nY > a_pOrigin->nY) m_tContentOrigin.nY = a_pOrigin->nY;
			LONG const nEX = tContentEnd.nX < tEnd.nX ? tEnd.nX : tContentEnd.nX;
			LONG const nEY = tContentEnd.nY < tEnd.nY ? tEnd.nY : tContentEnd.nY;
			m_tContentSize.nX = nEX-m_tContentOrigin.nX;
			m_tContentSize.nY = nEY-m_tContentOrigin.nY;
			OptimizeContent();
		}
		else
		{
			TImagePoint const tNewOrig = {a_pOrigin->nX < m_tContentOrigin.nX ? a_pOrigin->nX : m_tContentOrigin.nX, a_pOrigin->nY < m_tContentOrigin.nY ? a_pOrigin->nY : m_tContentOrigin.nY};
			TImagePoint const tNewEnd = {tEnd.nX > tContentEnd.nX ? tEnd.nX : tContentEnd.nX, tEnd.nY > tContentEnd.nY ? tEnd.nY : tContentEnd.nY};
			TImageSize const tNewSize = {tNewEnd.nX-tNewOrig.nX, tNewEnd.nY-tNewOrig.nY};
			// TODO: optimize
			CAutoVectorPtr<TPixelChannel> cData(new TPixelChannel[tNewSize.nX*tNewSize.nY]);
			std::fill_n(cData.m_p, tNewSize.nX*tNewSize.nY, m_tDefault);
			CopyRectangle(cData.m_p + m_tContentOrigin.nX-tNewOrig.nX + tNewSize.nX*(m_tContentOrigin.nY-tNewOrig.nY), tNewSize.nX,
				m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);

			if (M_Base()->UndoEnabled() == S_OK)
			{
				CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[a_pSize->nX*a_pSize->nY]);;
				CopyRectangle(cContent, a_pSize->nX, cData.m_p + a_pOrigin->nX-tNewOrig.nX + tNewSize.nX*(a_pOrigin->nY-tNewOrig.nY), a_pSize->nX, a_pSize->nY, 1, tNewSize.nX);
				CUndoPixelTile::Add(M_Base(), this, *a_pOrigin, *a_pSize, cContent.m_p, &DefDeleter, false);
				cContent.Detach();
			}

			CopyRectangle(cData.m_p + a_pOrigin->nX-tNewOrig.nX + tNewSize.nX*(a_pOrigin->nY-tNewOrig.nY), tNewSize.nX,
				a_pPixels, a_pSize->nX, a_pSize->nY, a_pStride->nX, a_pStride->nY);

			(*m_pfnDeleter)(m_pData);
			m_pData = cData.Detach();
			m_pfnDeleter = &DefDeleter;
			m_tAllocOrigin = m_tContentOrigin = tNewOrig;
			m_tAllocSize = m_tContentSize = tNewSize;
			OptimizeContent();
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::BufferReplace(TImagePoint a_tAllocOrigin, TImageSize a_tAllocSize, TImagePoint const* a_pContentOrigin, TImageSize const* a_pContentSize, ULONGLONG const* a_pContentAlphaSum, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter)
{
	try
	{
		if (a_pPixels == NULL || (a_tAllocSize.nX*a_tAllocSize.nY == 1 && a_pPixels->n == m_tDefault.n))
		{
			// source is empty
			CDocumentWriteLock cLock(this);
			if (m_tContentSize.nX*m_tContentSize.nY > 0)
			{
				if (M_Base()->UndoEnabled() == S_OK)
				{
					if (m_tAllocSize.nX == m_tContentSize.nX && m_tAllocSize.nY == m_tContentSize.nY)
					{
						CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, m_pData, m_pfnDeleter, true);
						m_pData = NULL;
					}
					else
					{
						CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);;
						CopyRectangle(cContent, m_tContentSize.nX, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
						CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, cContent.m_p, &DefDeleter, true);
						cContent.Detach();
					}
				}

				AddDirtyRect(m_tContentOrigin, m_tContentSize);

				m_tContentSize = m_tAllocSize = TIMGSIZE_NULL;
				m_tContentOrigin = m_tAllocOrigin = TIMGPOINT_NULL;
				m_pfnDeleter(m_pData);
				m_pData = NULL;
				m_pfnDeleter = &DefDeleter;
				m_qwAlphaTotal = 0;
			}
			return S_OK;
		}

		if ((a_pContentOrigin == NULL && a_pContentSize != NULL) ||
			(a_pContentOrigin != NULL && a_pContentSize == NULL))
			return E_RW_INVALIDPARAM; // both content origin and size must be given or none of them

		CDocumentWriteLock cLock(this);

		static TImagePoint const tPt0 = TIMGPOINT_NULL;
		bool bNeedOptimize = false;
		if (a_pContentOrigin == NULL)
		{
			bNeedOptimize = true;
			a_pContentOrigin = &a_tAllocOrigin;
			a_pContentSize = &a_tAllocSize;
		}
		TImageStride const tStr = {1, a_tAllocSize.nX};

		AddDirtyRect(m_tContentOrigin, m_tContentSize);
		AddDirtyRect(*a_pContentOrigin, *a_pContentSize);

		if (M_Base()->UndoEnabled() == S_OK)
		{
			if (m_tAllocSize.nX == m_tContentSize.nX && m_tAllocSize.nY == m_tContentSize.nY)
			{
				CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, m_pData, m_pfnDeleter, true);
				m_pData = NULL;
			}
			else
			{
				CAutoVectorPtr<TPixelChannel> cContent(new TPixelChannel[m_tContentSize.nX*m_tContentSize.nY]);;
				CopyRectangle(cContent, m_tContentSize.nX, m_pData+(m_tContentOrigin.nY-m_tAllocOrigin.nY)*m_tAllocSize.nX+m_tContentOrigin.nX-m_tAllocOrigin.nX, m_tContentSize.nX, m_tContentSize.nY, 1, m_tAllocSize.nX);
				CUndoPixelTile::Add(M_Base(), this, m_tContentOrigin, m_tContentSize, cContent.m_p, &DefDeleter, true);
				cContent.Detach();
			}
		}

		(*m_pfnDeleter)(m_pData);
		m_pData = a_pPixels;
		m_pfnDeleter = a_pDeleter;
		m_tAllocSize = a_tAllocSize;
		m_tContentSize = *a_pContentSize;
		m_tAllocOrigin = a_tAllocOrigin;
		m_tContentOrigin = *a_pContentOrigin;
		if (a_pContentAlphaSum)
			m_qwAlphaTotal = *a_pContentAlphaSum;
		if (bNeedOptimize)
			OptimizeContent();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::BufferAllocate(TImageSize a_tSize, TPixelChannel** a_ppPixels, fnDeleteBuffer* a_ppDeleter)
{
	if (a_ppPixels == NULL || a_ppDeleter == NULL)
		return E_POINTER;
	try
	{
		*a_ppDeleter = &DefDeleter;
		if (a_tSize.nX*a_tSize.nY == 0)
		{
			*a_ppPixels = NULL;
			return S_FALSE;
		}
		*a_ppPixels = new TPixelChannel[a_tSize.nX*a_tSize.nY];
		// to be perfectly correct, this should lock the module and deleter should unlock, to properly support dll unloading, but...
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentRasterImage::ProcessTile(EComposedPreviewMode a_eMode, LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, TRasterImagePixel* a_pData)
{
	// TODO: apply color profile
	return S_OK;
}

#include <IconRenderer.h>
#include <GammaCorrection.h>

HICON CDocumentRasterImage::LayerIcon(ULONG a_nSize, IRTarget const* a_pTarget)
{
	//static IRPolyPoint const brick0[] = { {256, 256}, {144, 256}, {144, 200}, {200, 200}, {200, 88}, {256, 88} };
	//static IRPolyPoint const brick1[] = { {56, 144}, {56, 88}, {168, 88}, {168, 144}, {112, 144}, {112, 200}, {0, 200}, {0, 144} };
	//static IRPolyPoint const brick2[] = { {0, 0}, {224, 0}, {224, 56}, {0, 56} };
	//static IRPolyPoint const brick0[] = { {0, 256}, {0, 160}, {48, 160}, {48, 208}, {144, 208}, {144, 256} };
	//static IRPolyPoint const brick1[] = { {208, 176}, {160, 176}, {160, 128}, {208, 128}, {208, 80}, {256, 80}, {256, 224}, {208, 224} };
	//static IRPolyPoint const brick2[] = { {96, 96}, {48, 96}, {48, 0}, {96, 0}, {96, 48}, {144, 48}, {144, 144}, {96, 144} };

	//static IRPolyPoint const brick0[] = { {4, 252}, {4, 140}, {60, 140}, {60, 196}, {172, 196}, {172, 252} };
	//static IRPolyPoint const brick1[] = { {196, 172}, {140, 172}, {140, 116}, {196, 116}, {196, 60}, {252, 60}, {252, 228}, {196, 228} };
	//static IRPolyPoint const brick2[] = { {72, 60}, {72, 4}, {184, 4}, {184, 60}, {128, 60}, {128, 116}, {16, 116}, {16, 60} };
	//static IRGridItem const grid0X[] = {{0, 4}, {0, 172}};
	//static IRGridItem const grid0Y[] = {{0, 140}, {0, 252}};
	//static IRCanvas const canvas0 = {0, 0, 256, 256, itemsof(grid0X), itemsof(grid0Y), grid0X, grid0Y};
	//static IRGridItem const grid1X[] = {{0, 140}, {0, 252}};
	//static IRGridItem const grid1Y[] = {{0, 60}, {0, 228}};
	//static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(grid1X), itemsof(grid1Y), grid1X, grid1Y};
	//static IRGridItem const grid2X[] = {{0, 16}, {0, 184}};
	//static IRGridItem const grid2Y[] = {{0, 4}, {0, 116}};
	//static IRCanvas const canvas2 = {0, 0, 256, 256, itemsof(grid2X), itemsof(grid2Y), grid2X, grid2Y};
	//CComPtr<IStockIcons> pSI;
	//RWCoCreateInstance(pSI, __uuidof(StockIcons));
	//CIconRendererReceiver cRenderer(a_nSize);
	//cRenderer(&canvas0, itemsof(brick0), brick0, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//cRenderer(&canvas1, itemsof(brick1), brick1, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//cRenderer(&canvas2, itemsof(brick2), brick2, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//return cRenderer.get();

	//static IRPolyPoint const group[] = { {256, 64}, {256, 256}, {64, 256}, {64, 192}, {128, 192}, {128, 128}, {192, 128}, {192, 64} };
	//static IRPolyPoint const shade1[] = { {128, 192}, {192, 192}, {192, 128}, {252, 128}, {252, 192}, {192, 192}, {192, 252}, {128, 252} };
	//static IRPolyPoint const shade2[] = { {192, 192}, {252, 192}, {252, 252}, {192, 252} };
	//static IRGridItem const grid1[] = { {0, 64}, {0, 128}, {0, 192}, {0, 256} };
	//static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid1), itemsof(grid1), grid1, grid1};
	//static IRPolyPoint const pixel1[] = { {16, 112}, {80, 112}, {80, 176}, {16, 176} };
	//static IRGridItem const grid1X[] = {{0, 16}, {0, 80}};
	//static IRGridItem const grid1Y[] = {{0, 112}, {0, 176}};
	//static IRCanvas const canvas1 = {0, 0, 256, 256, itemsof(grid1X), itemsof(grid1Y), grid1X, grid1Y};
	//static IRPolyPoint const pixel2[] = { {96, 32}, {160, 32}, {160, 96}, {96, 96} };
	//static IRGridItem const grid2X[] = {{0, 96}, {0, 160}};
	//static IRGridItem const grid2Y[] = {{0, 32}, {0, 96}};
	//static IRCanvas const canvas2 = {0, 0, 256, 256, itemsof(grid2X), itemsof(grid2Y), grid2X, grid2Y};
	//static IRPolyPoint const pixel3[] = { {0, 16}, {64, 16}, {64, 80}, {0, 80} };
	//static IRGridItem const grid3X[] = {{0, 0}, {0, 64}};
	//static IRGridItem const grid3Y[] = {{0, 16}, {0, 80}};
	//static IRCanvas const canvas3 = {0, 0, 256, 256, itemsof(grid3X), itemsof(grid3Y), grid3X, grid3Y};
	//CComPtr<IStockIcons> pSI;
	//RWCoCreateInstance(pSI, __uuidof(StockIcons));
	//CIconRendererReceiver cRenderer(a_nSize);
	//cRenderer(&canvas, itemsof(group), group, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//IRFill shadow1Mat(0x3f000000);
	//cRenderer(&canvas, itemsof(shade1), shade1, &shadow1Mat, a_pTarget);
	//IRFill shadow2Mat(0x7f000000);
	//cRenderer(&canvas, itemsof(shade2), shade2, &shadow2Mat, a_pTarget);
	//cRenderer(&canvas1, itemsof(pixel1), pixel1, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//cRenderer(&canvas2, itemsof(pixel2), pixel2, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//cRenderer(&canvas3, itemsof(pixel3), pixel3, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//return cRenderer.get();

	static IRPolyPoint const group[] = { {256, 64+8}, {256, 256}, {64+8, 256}, {64+8, 192}, {128+8, 192}, {128+8, 128+8}, {192, 128+8}, {192, 64+8} };
	static IRPolyPoint const shade1[] = { {128, 192}, {192, 192}, {192, 128}, {256, 128}, {256, 192}, {192, 256}, {128, 256} };
	static IRPolyPoint const shade2[] = { {192, 192}, {256, 192}, {256, 256}, {192, 256} };
	static IRGridItem const grid1[] = { {0, 64}, {0, 128}, {0, 192}, {0, 256} };
	static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid1), itemsof(grid1), grid1, grid1};
	static IRPolyPoint const pixel1[] = { {33.9574, 97.5575}, {95.7767, 114.122}, {79.2122, 175.941}, {17.393, 159.377} };
	static IRPolyPoint const pixel2[] = { {119.937, 21.9889}, {181.756, 38.5533}, {165.192, 100.373}, {103.373, 83.8082} };
	static IRPolyPoint const pixel3[] = { {32.1488, 0.00510025}, {87.574, 32.0051}, {55.5744, 87.4308}, {0.1488, 55.4308} };
	static IRCanvas const canvasp = {0, 0, 256, 256, 0, 0, NULL, NULL};
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(group), group, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	//IRFill shadow1Mat(0x3f000000);
	//IRFill shadow2Mat(0x7f000000);
	IRFill mat1Fill(CGammaTables::BlendSRGBA(pSI->GetSRGBColor(ESMScheme2Color2), 0xff000000, 0.75f));
	IRFill mat2Fill(CGammaTables::BlendSRGBA(pSI->GetSRGBColor(ESMScheme2Color2), 0xff000000, 0.5f));
	IROutlinedFill shadow1Mat(&mat1Fill, pSI->GetMaterial(ESMContrast));
	IROutlinedFill shadow2Mat(&mat2Fill, pSI->GetMaterial(ESMContrast));
	cRenderer(&canvas, itemsof(shade1), shade1, &shadow1Mat, a_pTarget);
	cRenderer(&canvas, itemsof(shade2), shade2, &shadow2Mat, a_pTarget);
	cRenderer(&canvasp, itemsof(pixel1), pixel1, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	cRenderer(&canvasp, itemsof(pixel2), pixel2, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	cRenderer(&canvasp, itemsof(pixel3), pixel3, pSI->GetMaterial(ESMScheme2Color2), a_pTarget);
	return cRenderer.get();
}

