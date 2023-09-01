
#pragma once

//////////////////////////////////////////////////////////////////////////
//
// CComPtrArray
//

/*++

    CComPtrArray stores an array of COM interface pointers and performs
    reference counting through AddRef and Release methods. 
    
    CComPtrArray can be used with WiaGetImage and DeviceDlg functions 
    to provide automatic deallocation of the output arrays.

Methods

    CComPtrArray()
        Initializes the array pointer and the count to zero.

    CComPtrArray(int nCount)
        Allocates the array for with CoTaskMemAlloc for nCount items and 
        initializes the interface pointers to NULL

    Copy(const CComPtrArray& rhs)
        Allocates a new array with CoTaskMemAlloc, copies the interface 
        pointers and call AddRef() on the copied pointers.

  	Clear()
        Calls Release on the interface pointers in the array and
        deallocates the array with CoTaskMemFree

    CComPtrArray(const CComPtrArray& rhs)
        Calls the Copy method to copy the new contents.

    CComPtrArray &operator =(const CComPtrArray& rhs)
        Calls the Clear method to delete the current contents and calls 
        the Copy method to copy the new contents.

    ~CComPtrArray()
        Destructor, calls the Clear method

    operator T **()
        Returns the dereferenced value of the member pointer.

    bool operator!()
        Returns TRUE or FALSE, depending on whether the member pointer is 
        NULL or not.

    T ***operator&()
        Returns the address of the member pointer.

    LONG &Count()
        Returns a reference to the count.

--*/

template <class T>
class CComPtrArray
{
public:
    CComPtrArray()
    {
        m_pArray = NULL;
        m_nCount = 0;
    }

    explicit CComPtrArray(int nCount)
    {
        m_pArray = (T **) CoTaskMemAlloc(nCount * sizeof(T *));

        m_nCount = m_pArray == NULL ? 0 : nCount;

        for (int i = 0; i < m_nCount; ++i) 
        {
            m_pArray[i] = NULL;
        }
    }

    CComPtrArray(const CComPtrArray& rhs)
    {
        Copy(rhs);
    }

    ~CComPtrArray() 
    {
        Clear();
    }

    CComPtrArray &operator =(const CComPtrArray& rhs)
    {
        if (this != &rhs)
        {
            Clear();
            Copy(rhs);
        }

        return *this;
    }

    operator T **()
    {
        return m_pArray;
    }

    bool operator!()
    {
        return m_pArray == NULL;
    }

    T ***operator&()
    {
        return &m_pArray;
    }

    LONG &Count()
    {
        return m_nCount;
    }

	void Clear()
	{
        if (m_pArray != NULL) 
        {
            for (int i = 0; i < m_nCount; ++i) 
            {
                if (m_pArray[i] != NULL) 
                {
                    m_pArray[i]->Release();
                }
            }

            CoTaskMemFree(m_pArray);

            m_pArray = NULL;
            m_nCount = 0;
        }
	}

    void Copy(const CComPtrArray& rhs)
    {
        m_pArray = NULL;
        m_nCount = 0;

        if (rhs.m_pArray != NULL)
        {
            m_pArray = (T**) CoTaskMemAlloc(rhs.m_nCount * sizeof(T *));

            if (m_pArray != NULL)
            {
                m_nCount = rhs.m_nCount;

                for (int i = 0; i < m_nCount; ++i)
                {
                    m_pArray[i] = rhs.m_pArray[i];

                    if (m_pArray[i] != NULL)
                    {
                        m_pArray[i]->AddRef();
                    }
                }
            }
        }
    }

private:
    T    **m_pArray;
    LONG  m_nCount;
};


//////////////////////////////////////////////////////////////////////////
//
// ReadPropertyLong
//

/*++

    The ReadPropertyLong function reads a long integer value from a WIA 
    property storage


    HRESULT 
    ReadPropertyLong(
        IWiaPropertyStorage *pWiaPropertyStorage, 
        const PROPSPEC      *pPropSpec, 
        LONG                *plResult
    );

Parameters

    pWiaPropertyStorage
        [in] Pointer to the interface of the WIA property storage.

    PropSpec
        [in] Pointer to a PROPSPEC structure that specifies which 
        property is to be read. 

    plResult
        [out] Receives the value of the property specified by PropSpec


Return Values
    
    ReadPropertyLong returns S_OK on success or a standard COM error 
    if it fails for any reason.

--*/
inline HRESULT ReadPropertyLong(IWiaPropertyStorage *pWiaPropertyStorage, const PROPSPEC *pPropSpec, LONG *plResult)
{
	PROPVARIANT PropVariant;

	HRESULT hr = pWiaPropertyStorage->ReadMultiple(
		1, 
		pPropSpec, 
		&PropVariant
	);

	// Generally, the return value should be checked against S_FALSE.
	// If ReadMultiple returns S_FALSE, it means the property name or ID
	// had valid syntax, but it didn't exist in this property set, so
	// no properties were retrieved, and each PROPVARIANT structure is set 
	// to VT_EMPTY. But the following switch statement will handle this case
	// and return E_FAIL. So the caller of ReadPropertyLong does not need
	// to check for S_FALSE explicitly.

	if (SUCCEEDED(hr))
	{
		switch (PropVariant.vt)
		{
			case VT_I1:
			{
				*plResult = (LONG) PropVariant.cVal;

				hr = S_OK;

				break;
			}

			case VT_UI1:
			{
				*plResult = (LONG) PropVariant.bVal;

				hr = S_OK;

				break;
			}

			case VT_I2:
			{
				*plResult = (LONG) PropVariant.iVal;

				hr = S_OK;

				break;
			}

			case VT_UI2:
			{
				*plResult = (LONG) PropVariant.uiVal;

				hr = S_OK;

				break;
			}

			case VT_I4:
			{
				*plResult = (LONG) PropVariant.lVal;

				hr = S_OK;

				break;
			}

			case VT_UI4:
			{
				*plResult = (LONG) PropVariant.ulVal;

				hr = S_OK;

				break;
			}

			case VT_INT:
			{
				*plResult = (LONG) PropVariant.intVal;

				hr = S_OK;

				break;
			}

			case VT_UINT:
			{
				*plResult = (LONG) PropVariant.uintVal;

				hr = S_OK;

				break;
			}

			case VT_R4:
			{
				*plResult = (LONG) (PropVariant.fltVal + 0.5);

				hr = S_OK;

				break;
			}

			case VT_R8:
			{
				*plResult = (LONG) (PropVariant.dblVal + 0.5);

				hr = S_OK;

				break;
			}

			default:
			{
				hr = E_FAIL;

				break;
			}
		}
	}

	PropVariantClear(&PropVariant);

	return hr;
}

namespace BitmapUtil 
{

//////////////////////////////////////////////////////////////////////////
//
// GetBitmapHeaderSize
//

inline ULONG GetBitmapHeaderSize(LPCVOID pDib)
{
    ULONG nHeaderSize = *(PDWORD) pDib;

    switch (nHeaderSize)
	{
		case sizeof(BITMAPCOREHEADER):
        case sizeof(BITMAPINFOHEADER):
		case sizeof(BITMAPV4HEADER):
		case sizeof(BITMAPV5HEADER):
		{
			return nHeaderSize;
		}
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapLineWidthInBytes
//

inline ULONG GetBitmapLineWidthInBytes(ULONG nWidthInPixels, ULONG nBitCount)
{
    return (((nWidthInPixels * nBitCount) + 31) & ~31) >> 3;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapDimensions
//

inline BOOL GetBitmapDimensions(LPCVOID pDib, UINT *pWidth, UINT *pHeight)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return FALSE;
    }

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        if (pWidth != NULL)
        {
            *pWidth  = pbmch->bcWidth;
        }

        if (pHeight != NULL)
        {
            *pHeight = pbmch->bcHeight;
        }
    }
    else
    {
        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        if (pWidth != NULL)
        {
            *pWidth  = pbmih->biWidth;
        }

        if (pHeight != NULL)
        {
            *pHeight = abs(pbmih->biHeight);
        }
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapSize
//

inline ULONG GetBitmapSize(LPCVOID pDib)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return 0;
    }

    // Start the calculation with the header size

    ULONG nDibSize = nHeaderSize;

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // Add the color table size

        if (pbmch->bcBitCount <= 8)
        {
            nDibSize += sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
        }

        // Add the bitmap size

        ULONG nWidth = GetBitmapLineWidthInBytes(pbmch->bcWidth, pbmch->bcBitCount);

        nDibSize += nWidth * pbmch->bcHeight;
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // Add the color table size

        if (pbmih->biClrUsed != 0)
        {
            nDibSize += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nDibSize += sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
        }

        // Add the bitmap size

        if (pbmih->biSizeImage != 0)
        {
            nDibSize += pbmih->biSizeImage;
        }
        else
        {
            // biSizeImage must be specified for compressed bitmaps

            if (pbmih->biCompression != BI_RGB &&
                pbmih->biCompression != BI_BITFIELDS)
            {
                return 0;
            }

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmih->biWidth, pbmih->biBitCount);

            nDibSize += nWidth * abs(pbmih->biHeight);
        }

        // Consider special cases

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
            // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
            // bmiColors member contains three DWORD color masks.
            // For V4 or V5 headers, this info is included the header

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nDibSize += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
            // If this is a V5 header and an ICM profile is specified,
            // we need to consider the profile data size
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

            // if there is some padding before the profile data, add it

            if (pbV5h->bV5ProfileData > nDibSize)
            {
                nDibSize = pbV5h->bV5ProfileData;
            }

            // add the profile data size

            nDibSize += pbV5h->bV5ProfileSize;
        }
    }

    return nDibSize;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapOffsetBits
//

inline ULONG GetBitmapOffsetBits(LPCVOID pDib)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return 0;
    }

    // Start the calculation with the header size

    ULONG nOffsetBits = nHeaderSize;

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // Add the color table size

        if (pbmch->bcBitCount <= 8)
        {
            nOffsetBits += sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
        }
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // Add the color table size

        if (pbmih->biClrUsed != 0)
        {
            nOffsetBits += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nOffsetBits += sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
        }

        // Consider special cases

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
            // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
            // bmiColors member contains three DWORD color masks.
            // For V4 or V5 headers, this info is included in the header

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nOffsetBits += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
            // If this is a V5 header and an ICM profile is specified,
            // we need to consider the profile data size
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

            // if the profile data comes before the pixel data, add it

            if (pbV5h->bV5ProfileData <= nOffsetBits)
            {
                nOffsetBits += pbV5h->bV5ProfileSize;
            }
        }
    }

    return nOffsetBits;
}


//////////////////////////////////////////////////////////////////////////
//
// FixBitmapHeight
//

inline BOOL FixBitmapHeight(PVOID pDib, ULONG nSize, BOOL bTopDown)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return FALSE;
    }

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // fix the height value if necessary

        if (pbmch->bcHeight == 0)
        {
            // start the calculation with the header size

            ULONG nSizeImage = nSize - nHeaderSize;

            // subtract the color table size

            if (pbmch->bcBitCount <= 8)
            {
                nSizeImage -= sizeof(RGBTRIPLE) * (1 << pbmch->bcBitCount);
            }

            // calculate the height

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmch->bcWidth, pbmch->bcBitCount);

            if (nWidth == 0)
            {
                return FALSE;
            }

            LONG nHeight = nSizeImage / nWidth;

            pbmch->bcHeight = (WORD) nHeight;
        }
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // fix the height value if necessary

        if (pbmih->biHeight == 0)
        {
            // find the size of the image data

            ULONG nSizeImage;

            if (pbmih->biSizeImage != 0)
            {
                // if the size is specified in the header, take it

                nSizeImage = pbmih->biSizeImage;
            }
            else
            {
                // start the calculation with the header size

                nSizeImage = nSize - nHeaderSize;

                // subtract the color table size

                if (pbmih->biClrUsed != 0)
                {
                    nSizeImage -= sizeof(RGBQUAD) * pbmih->biClrUsed;
                }
                else if (pbmih->biBitCount <= 8)
                {
                    nSizeImage -= sizeof(RGBQUAD) * (1 << pbmih->biBitCount);
                }

                // Consider special cases

                if (nHeaderSize == sizeof(BITMAPINFOHEADER))
                {     
                    // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
                    // bmiColors member contains three DWORD color masks.
                    // For V4 or V5 headers, this info is included the header

                    if (pbmih->biCompression == BI_BITFIELDS)
                    {
                        nSizeImage -= 3 * sizeof(DWORD);
                    }
                }
                else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
                {
                    // If this is a V5 header and an ICM profile is specified,
                    // we need to consider the profile data size
            
                    PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

                    // add the profile data size

                    nSizeImage -= pbV5h->bV5ProfileSize;
                }

                // store the image size

                pbmih->biSizeImage = nSizeImage;
            }

            // finally, calculate the height

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmih->biWidth, pbmih->biBitCount);

            if (nWidth == 0)
            {
                return FALSE;
            }

			LONG nHeight = nSizeImage / nWidth;

            pbmih->biHeight = bTopDown ? -nHeight : nHeight;
        }
    }

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// FillBitmapFileHeader
//

inline BOOL FillBitmapFileHeader(LPCVOID pDib, PBITMAPFILEHEADER pbmfh)
{
    ULONG nSize = GetBitmapSize(pDib);

    if (nSize == 0)
    {
        return FALSE;
    }

    ULONG nOffset = GetBitmapOffsetBits(pDib);

    if (nOffset == 0)
    {
        return FALSE;
    }

    pbmfh->bfType      = MAKEWORD('B', 'M');
    pbmfh->bfSize      = sizeof(BITMAPFILEHEADER) + nSize;
    pbmfh->bfReserved1 = 0;
    pbmfh->bfReserved2 = 0;
    pbmfh->bfOffBits   = sizeof(BITMAPFILEHEADER) + nOffset;

    return TRUE;
}

}; // namespace BitmapUtil
