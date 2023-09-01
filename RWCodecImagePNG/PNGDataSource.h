#pragma once

class CPNGDataSource
{
public:
	CPNGDataSource(ULONG a_nSize, BYTE const* a_pData) : m_nSize(a_nSize), m_pData(a_pData), m_nActPos(0)
	{
	}

	operator png_voidp()
	{
		return reinterpret_cast<png_voidp>(this);
	}

	static void PNGAPI PngRWPtr(png_structp a_pPNGStruct, png_bytep a_pBuffer, png_size_t a_nBufferLen)
	{
		CPNGDataSource* pThis = reinterpret_cast<CPNGDataSource*>(png_get_io_ptr(a_pPNGStruct));
		CopyMemory(a_pBuffer, pThis->m_pData+pThis->m_nActPos, a_nBufferLen);
		pThis->m_nActPos += a_nBufferLen;
	}

private:
	BYTE const* m_pData;
	DWORD m_nSize;
	DWORD m_nActPos;
};
