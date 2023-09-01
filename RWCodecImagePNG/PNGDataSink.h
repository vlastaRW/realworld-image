#pragma once

class CPNGDataSink
{
public:
	CPNGDataSink(IImageSink* a_pSink) : m_pSink(a_pSink), m_nWritten(0)
	{
	}

	operator png_voidp()
	{
		return reinterpret_cast<png_voidp>(this);
	}

	static void PNGAPI PngRWFnc(png_structp a_pPNGStruct, png_bytep a_pBuffer, png_size_t a_nBufferLen)
	{
		CPNGDataSink* pThis = reinterpret_cast<CPNGDataSink*>(a_pPNGStruct->io_ptr);
		BYTE* pOut = NULL;
		pThis->m_pSink->BufferLock(pThis->m_nWritten, a_nBufferLen, &pOut);
		memcpy(pOut, a_pBuffer, a_nBufferLen);
		pThis->m_pSink->BufferUnlock(a_nBufferLen, pOut);
		pThis->m_nWritten += a_nBufferLen;
	}
	static void PNGAPI PngFlushFnc(png_structp)
	{
	}

private:
	IImageSink* m_pSink;
	ULONG m_nWritten;
};
