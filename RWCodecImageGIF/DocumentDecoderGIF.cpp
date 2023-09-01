// DocumentDecoderGIF.cpp : Implementation of CDocumentDecoderGIF

#include "stdafx.h"
#include "DocumentDecoderGIF.h"

#include <MultiLanguageString.h>
#include <RWDocumentAnimation.h>
#include <RWDocumentImageRaster.h>
#include <RWDocumentAnimationUtils.h>
#include "GIF2.h"


// CDocumentDecoderGIF

STDMETHODIMP CDocumentDecoderGIF::Priority(ULONG* a_pnPriority)
{
	try
	{
		*a_pnPriority = EDPAverage;
		return S_OK;
	}
	catch (...)
	{
		return a_pnPriority ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderGIF::DocumentType(IDocumentType** a_ppDocumentType)
{
	try
	{
		*a_ppDocumentType = NULL;
		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"gif");
		pDocType->InitEx(CMultiLanguageString::GetAuto(L"[0409]GIF image files[0405]Soubory obrázků GIF"), CMultiLanguageString::GetAuto(L"[0409]GIF Image[0405]Obrázek GIF"), 1, &(bstrExt.m_str), NULL, NULL, 0, CComBSTR(L"*.gif"));
		*a_ppDocumentType = pDocType.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocumentType ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CDocumentDecoderGIF::IsCompatible(ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders)
{
	try
	{
		for (ULONG i = 0; i < a_nBuilders; ++i)
		{
			CComPtr<IDocumentFactoryRasterImage> pI;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryRasterImage), reinterpret_cast<void**>(&pI));
			if (pI) return S_OK;
			CComPtr<IDocumentFactoryAnimation> pA;
			a_apBuilders[i]->QueryInterface(__uuidof(IDocumentFactoryAnimation), reinterpret_cast<void**>(&pA));
			if (pA) return S_OK;
		}
		return S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

int nsGIFDecoder2::BeginGIF(void* aClientData, PRUint32 aLogicalScreenWidth, PRUint32 aLogicalScreenHeight, PRUint8 aBackgroundRGBIndex)
{
	nsGIFDecoder2* p = reinterpret_cast<nsGIFDecoder2*>(aClientData);
	if (aLogicalScreenWidth == 0 || aLogicalScreenHeight == 0)
		return 1;
	p->m_bFrameAdded = false;
	delete[] p->m_pBackup;
	p->m_pBackup = NULL;
	//p->m_p->SizeSet(aLogicalScreenWidth, aLogicalScreenHeight);
	p->mLogicalScreenWidth = aLogicalScreenWidth;
	p->mLogicalScreenHeight = aLogicalScreenHeight;
	delete[] p->m_pBuffer;
	p->m_pBuffer = new BYTE[4*aLogicalScreenWidth*aLogicalScreenHeight];
	ZeroMemory(p->m_pBuffer, 4*aLogicalScreenWidth*aLogicalScreenHeight);
	p->mBackgroundRGBIndex = aBackgroundRGBIndex;
	return 0;
}

int nsGIFDecoder2::EndGIF(void* aClientData, int aAnimationLoopCount)
{
	nsGIFDecoder2* p = reinterpret_cast<nsGIFDecoder2*>(aClientData);
	if (p->m_pAni && p->m_bFrameAdded) p->m_pAni->SetLoopCount(p->m_bstrPrefix, p->m_pBase, aAnimationLoopCount < 0 ? 0 : (aAnimationLoopCount ? aAnimationLoopCount : 1));
	return 0;
}

int nsGIFDecoder2::BeginImageFrame(void* aClientData, PRUint32 /*aFrameNumber*/, PRUint32 aFrameXOffset, PRUint32 aFrameYOffset, PRUint32 aFrameWidth, PRUint32 aFrameHeight)
{
	nsGIFDecoder2* p = reinterpret_cast<nsGIFDecoder2*>(aClientData);
	if (p->m_bDelayedFrame)
	{
		if (p->m_aDelayTimeout == 0)
		{
			if (p->mGIFStruct.delay_time == 0 && (p->m_eBackupMode != DISPOSE_KEEP || !p->mGIFStruct.is_local_colormap_defined))// && (aFrameXOffset == 0 || aFrameYOffset == 0 && aFrameWidth == p->mLogicalScreenWidth && aFrameHeight == p->mLogicalScreenHeight))
				p->m_aDelayTimeout = p->m_nFix0Delay = 100;
		}
		if (p->m_aDelayTimeout)
		{
			p->m_hRes = p->m_pAni->Init(p->m_bstrPrefix, p->m_pBase);
			if (SUCCEEDED(p->m_hRes))
				p->m_hRes = p->m_pAni->AppendFrame(&CAnimationFrameCreatorRasterImage(p->mLogicalScreenWidth, p->mLogicalScreenHeight, reinterpret_cast<TPixelChannel const*>(p->m_pBuffer)), p->m_aDelayTimeout/1000.0f, p->m_bstrPrefix, p->m_pBase);
			p->m_bDelayedFrame = false;
			p->m_bFrameAdded = true;
		}
		p->DisposeFrame();
	}
	//p->mGIFStruct.x_offset = aFrameXOffset;
	//p->mGIFStruct.y_offset = aFrameYOffset;
	//p->mGIFStruct.width = aFrameWidth;
	//p->mGIFStruct.height = aFrameHeight;
	p->m_eBackupMode = p->mGIFStruct.disposal_method;
	p->m_nBackupOffX = aFrameXOffset;
	p->m_nBackupOffY = aFrameYOffset;
	p->m_nBackupSizeX = aFrameWidth;
	p->m_nBackupSizeY = aFrameHeight;
	switch (p->mGIFStruct.disposal_method)
	{
	case DISPOSE_NOT_SPECIFIED:
		break;
	case DISPOSE_KEEP:
		break;
	case DISPOSE_OVERWRITE_BGCOLOR:
		// will be cleared in DisposeFrame
		break;
	case DISPOSE_OVERWRITE_PREVIOUS:
		ATLASSERT(p->m_pBackup == NULL);
		delete[] p->m_pBackup;
		p->m_pBackup = new BYTE[aFrameWidth*aFrameHeight<<2];
		for (ULONG y = 0; y < aFrameHeight; ++y)
			CopyMemory(p->m_pBackup+(y*aFrameWidth<<2), p->m_pBuffer+((p->mLogicalScreenWidth*(y+aFrameYOffset)+aFrameXOffset)<<2), aFrameWidth<<2);
		break;
	}
	return 0;
}

void nsGIFDecoder2::DisposeFrame()
{
	switch (m_eBackupMode)
	{
	case DISPOSE_NOT_SPECIFIED:
		break;
	case DISPOSE_KEEP:
		break;
	case DISPOSE_OVERWRITE_BGCOLOR:
		for (ULONG y = 0; y < m_nBackupSizeY; ++y)
			ZeroMemory(m_pBuffer+((mLogicalScreenWidth*(y+m_nBackupOffY)+m_nBackupOffX)<<2), m_nBackupSizeX<<2);
		break;
	case DISPOSE_OVERWRITE_PREVIOUS:
		ATLASSERT(m_pBackup);
		if (m_pBackup)
		{
			for (ULONG y = 0; y < m_nBackupSizeY; ++y)
				CopyMemory(m_pBuffer+((mLogicalScreenWidth*(y+m_nBackupOffY)+m_nBackupOffX)<<2), m_pBackup+(y*m_nBackupSizeX<<2), m_nBackupSizeX<<2);
			delete[] m_pBackup;
			m_pBackup = NULL;
		}
		break;
	}
}

int nsGIFDecoder2::EndImageFrame(void* aClientData, PRUint32 /*aFrameNumber*/, PRUint32 aDelayTimeout)
{
	nsGIFDecoder2* p = reinterpret_cast<nsGIFDecoder2*>(aClientData);

	if (p->m_pAni == NULL)
	{
		if (!p->m_bFrameAdded) // animation not supported - ignore other frames
		{
			p->m_hRes = p->m_pImg->Create(p->m_bstrPrefix, p->m_pBase, CImageSize(p->mLogicalScreenWidth, p->mLogicalScreenHeight), NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(p->mLogicalScreenWidth, p->mLogicalScreenHeight, reinterpret_cast<TPixelChannel const*>(p->m_pBuffer)));
			p->m_bFrameAdded = true;
		}
		return 0;
	}

	if (!p->m_bFrameAdded)
	{
		// never add the first frame right away
		p->m_bDelayedFrame = true;
		p->m_aDelayTimeout = aDelayTimeout;
		return 0;
	}

	if (aDelayTimeout == 0)
		aDelayTimeout = p->m_nFix0Delay;

	if (aDelayTimeout && SUCCEEDED(p->m_hRes))
		p->m_hRes = p->m_pAni->AppendFrame(&CAnimationFrameCreatorRasterImage(p->mLogicalScreenWidth, p->mLogicalScreenHeight, reinterpret_cast<TPixelChannel const*>(p->m_pBuffer)), aDelayTimeout/1000.0f, p->m_bstrPrefix, p->m_pBase);

	p->DisposeFrame();

	return 0;
}

int nsGIFDecoder2::HaveDecodedRow(void* aClientData,
								  PRUint8* aRowBufPtr,   /* Pointer to single scanline temporary buffer */
								  int aRow,              /* Row number? */
								  int aDuplicateCount,   /* Number of times to duplicate the row? */
								  int aInterlacePass)
{
	nsGIFDecoder2* p = reinterpret_cast<nsGIFDecoder2*>(aClientData);

	if (aRowBufPtr)
	{
		// XXX map the data into colors
		int cmapsize = p->mGIFStruct.global_colormap_size;
		PRUint8* cmap = p->mGIFStruct.global_colormap;

		DWORD bgColor = 0xff000000;
		if (p->mGIFStruct.global_colormap &&
			p->mGIFStruct.screen_bgcolor < cmapsize)
		{
			PRUint32 bgIndex = p->mGIFStruct.screen_bgcolor * 3;
			bgColor |= cmap[bgIndex] << 16;
			bgColor |= cmap[bgIndex + 1] << 8;
			bgColor |= cmap[bgIndex + 2];
			//p->mImageFrame->SetBackgroundColor(bgColor);
		}
		if (p->mGIFStruct.is_local_colormap_defined)
		{
			cmapsize = p->mGIFStruct.local_colormap_size;
			cmap = p->mGIFStruct.local_colormap;
		}

		BYTE* rgbRowIndex = p->m_pBuffer+4*((aRow+p->mGIFStruct.y_offset)*p->mLogicalScreenWidth+p->mGIFStruct.x_offset);
		if (!cmap)
		{ // cmap could have null value if the global color table flag is 0
			ZeroMemory(rgbRowIndex, p->mGIFStruct.rowend-aRowBufPtr);
		}
		else
		{
			PRUint8* rowBufIndex = aRowBufPtr;
      
			while (rowBufIndex != p->mGIFStruct.rowend)
			{
				if (*rowBufIndex < cmapsize)
				{
					if (!p->mGIFStruct.is_transparent || p->mGIFStruct.tpixel != *rowBufIndex)
					{
						PRUint32 colorIndex = *rowBufIndex * 3;
						*rgbRowIndex++ = cmap[colorIndex + 2]; // red
						*rgbRowIndex++ = cmap[colorIndex + 1]; // green
						*rgbRowIndex++ = cmap[colorIndex];     // blue
						*rgbRowIndex++ = 0xff;
					}
					else
					{
						rgbRowIndex += 4;
					}
				}
				else
				{
					*rgbRowIndex++ = 0;                    // red
					*rgbRowIndex++ = 0;                    // green
					*rgbRowIndex++ = 0;                    // blue
					*rgbRowIndex++ = 0xff;
				}
				++rowBufIndex;
			}  
		}
	}

	return 0;
}

HRESULT nsGIFDecoder2::Finalize() const
{
	HRESULT hRes = m_hRes;
	if (mGIFStruct.state == gif_error)
		return E_FAIL;
	if (!m_bFrameAdded)
	{
		if (m_pImg)
		{
			hRes = m_pImg->Create(m_bstrPrefix, m_pBase, CImageSize(mLogicalScreenWidth, mLogicalScreenHeight), NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(mLogicalScreenWidth, mLogicalScreenHeight, reinterpret_cast<TPixelChannel const*>(m_pBuffer)));
		}
		else
		{
			hRes = m_pAni->Init(m_bstrPrefix, m_pBase);
			if (SUCCEEDED(hRes))
				hRes = m_pAni->AppendFrame(&CAnimationFrameCreatorRasterImage(mLogicalScreenWidth, mLogicalScreenHeight, reinterpret_cast<TPixelChannel const*>(m_pBuffer)), m_aDelayTimeout/1000.0f, m_bstrPrefix, m_pBase);
		}
	}
	return hRes;
}

#include "DocumentEncoderGIF.h"

STDMETHODIMP CDocumentDecoderGIF::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), ULONG a_nBuilders, IDocumentBuilder* const* a_apBuilders, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	try
	{
		CComPtr<IDocumentFactoryAnimation> pAni;
		CComPtr<IDocumentFactoryRasterImage> pRIF;
		for (ULONG i = 0; i < a_nBuilders && pRIF == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pRIF);
		for (ULONG i = 0; i < a_nBuilders && pAni == NULL; ++i)
			a_apBuilders[i]->QueryInterface(&pAni);
		if (pAni == NULL && pRIF == NULL)
			return E_FAIL;

		if (a_nLen == 0 || a_pData == NULL)
			return E_FAIL;

		gif_struct gs;
		nsGIFDecoder2 dec(pAni, pRIF, a_bstrPrefix, a_pBase, gs);
		GIFInit(&gs, &dec);
		PRStatus stat = gif_write(&gs, a_pData, a_nLen);
		gif_destroy(&gs);
		HRESULT hRes = dec.Finalize();
		if (FAILED(hRes) || stat != PR_SUCCESS)
		{
			a_pBase->DataBlockSet(a_bstrPrefix, NULL);
			hRes = E_FAIL;
		}
		else
		{
			if (a_pEncoderID) *a_pEncoderID = CLSID_DocumentEncoderGIF;
			if (a_ppEncoderCfg)
			{
				CComPtr<IConfig> pConfig;
				pConfig.Attach(CDocumentEncoderGIF::Config());
				if (gs.interlaced)
				{
					CComBSTR bstr(L"Interlacing");
					pConfig->ItemValuesSet(1, &(bstr.m_str), CConfigValue(true));
				}
				*a_ppEncoderCfg = pConfig.Detach();
			}
		}
		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

