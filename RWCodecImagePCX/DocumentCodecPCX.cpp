// DocumentCodecPCX.cpp : Implementation of CDocumentCodecPCX

#include "stdafx.h"
#include "DocumentCodecPCX.h"

#include <MultiLanguageString.h>


// CDocumentCodecPCX

typedef struct _PcxHeader       /* Offset   Description            */
{
    BYTE   Id;                  /*  00h     Manufacturer ID        */
    BYTE   Version;             /*  01h     Version                */
    BYTE   Format;              /*  02h     Encoding Scheme        */
    BYTE   BitsPixelPlane;      /*  03h     Bits/Pixel/Plane       */
    WORD   Xmin;                /*  04h     X Start (upper left)   */
    WORD   Ymin;                /*  06h     Y Start (top)          */
    WORD   Xmax;                /*  08h     X End (lower right)    */
    WORD   Ymax;                /*  0Ah     Y End (bottom)         */
    WORD   Hdpi;                /*  0Ch     Horizontal Resolution  */
    WORD   Vdpi;                /*  0Eh     Vertical Resolution    */
    BYTE   EgaPalette[3*16];    /*  10h     16-Color EGA Palette   */
    BYTE   Reserved;            /*  40h     Reserved               */
    BYTE   NumberOfPlanes;      /*  41h     Number of Color Planes */
    WORD   BytesLinePlane;      /*  42h     Bytes/Line/Plane       */
    WORD   PaletteInfo;         /*  44h     Palette Interpretation */
    WORD   HScreenSize;         /*  46h     Horizontal Screen Size */
    WORD   VScreenSize;         /*  48h     Vertical Screen Size   */
    BYTE   Filler[54];          /*  4Ah     Reserved               */
} PCXHEADER;

struct CPcxDecodeContext
{
	CPcxDecodeContext(BYTE const* a_pBegin, BYTE const* a_pEnd) : m_p(a_pBegin), m_pEnd(a_pEnd), m_nLeft(0) {}

	template<class TOutIterator>
	bool Read(TOutIterator& a_tOut)
	{
		while (true)
		{
			if (m_nLeft)
			{
				--m_nLeft;
				if (a_tOut.Put(m_nVal))
					return true;
			}
			else if (m_p < m_pEnd)
			{
				if ((*m_p & 0xc0) == 0xc0)
				{
					if (m_p+1 == m_pEnd)
						return false;
					m_nLeft = *m_p & 0x3f;
					++m_p;
					m_nVal = *m_p;
					++m_p;
				}
				else
				{
					m_nLeft = 1;
					m_nVal = *m_p;
					++m_p;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}
	void LineBreak()
	{
		m_nLeft = 0;
	}

private:
	BYTE const* m_p;
	BYTE const* const m_pEnd;
	BYTE m_nLeft;
	BYTE m_nVal;
};

struct SSkipFillerOut
{
	SSkipFillerOut(size_t a_n) : m_n(a_n) {}
	bool Put(BYTE a_b)
	{
		if (m_n <= 1) return true;
		--m_n;
		return false;
	}

private:
	size_t m_n;
};

struct SChannelOut
{
	SChannelOut(BYTE* a_p, size_t a_n) : m_p(a_p), m_pEnd(a_p+(a_n<<2)) {}

	bool Put(BYTE a_b)
	{
		*m_p = a_b;
		m_p+=4;
		return m_p >= m_pEnd;
	}

private:
	BYTE* m_p;
	BYTE* const m_pEnd;
};

struct S8bitPalOut
{
	S8bitPalOut(TPixelChannel* a_p, size_t a_n, BYTE const* a_pPal) : m_p(a_p), m_pEnd(a_p+a_n), m_pPal(a_pPal) {}

	bool Put(BYTE a_b)
	{
		BYTE const* const p = m_pPal+a_b+a_b+a_b;
		m_p->bR = *p;
		m_p->bG = p[1];
		m_p->bB = p[2];
		m_p->bA = 0xff;
		++m_p;
		return m_p >= m_pEnd;
	}
	TPixelChannel* m_p;
	TPixelChannel* const m_pEnd;
	BYTE const* const m_pPal;
};

struct S4bitPalOut
{
	S4bitPalOut(TPixelChannel* a_p, size_t a_n, BYTE const* a_pPal) : m_p(a_p), m_pEnd(a_p+a_n), m_pPal(a_pPal) {}

	bool Put(BYTE a_b)
	{
		BYTE const b1 = a_b>>4;
		BYTE const b2 = a_b&0xf;
		BYTE const* p1 = m_pPal+b1+b1+b1;
		m_p->bR = *p1;
		m_p->bG = p1[1];
		m_p->bB = p1[2];
		m_p->bA = 0xff;
		++m_p;
		if (m_p >= m_pEnd) return true;
		BYTE const* p2 = m_pPal+b2+b2+b2;
		m_p->bR = *p2;
		m_p->bG = p2[1];
		m_p->bB = p2[2];
		m_p->bA = 0xff;
		++m_p;
		return m_p >= m_pEnd;
	}

private:
	TPixelChannel* m_p;
	TPixelChannel* const m_pEnd;
	BYTE const* const m_pPal;
};

struct S1bitPalOut
{
	S1bitPalOut(TPixelChannel* a_p, size_t a_n, BYTE const* a_pPal) : m_p(a_p), m_pEnd(a_p+a_n), m_pPal(a_pPal) {}

	bool Put(BYTE a_b)
	{
		for (BYTE i = 0; i < 8; ++i)
		{
			BYTE const* p1 = m_pPal+(((a_b&0x80)>>7)|((a_b&0x80)>>6));
			m_p->bR = *p1;
			m_p->bG = p1[1];
			m_p->bB = p1[2];
			m_p->bA = 0xff;
			++m_p;
			if (m_p >= m_pEnd) return true;
			a_b <<= 1;
		}
		return false;
	}

private:
	TPixelChannel* m_p;
	TPixelChannel* const m_pEnd;
	BYTE const* const m_pPal;
};

HRESULT CDocumentCodecPCX::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* UNREF(a_pLocation), IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	if (a_nLen < sizeof(PCXHEADER))
		return E_RW_UNKNOWNINPUTFORMAT;

	PCXHEADER const* pHeader = reinterpret_cast<PCXHEADER const*>(a_pData);
	if (pHeader == NULL || pHeader->Id != 0x0a || pHeader->Version == 1 || pHeader->Version > 5)
		return E_RW_UNKNOWNINPUTFORMAT;

	TImageSize const tSize = {pHeader->Xmax - pHeader->Xmin + 1, pHeader->Ymax - pHeader->Ymin + 1};
	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);

	int bpp = pHeader->BitsPixelPlane * pHeader->NumberOfPlanes;
	bool hasPal = false;
	if (bpp == 8 && pHeader->Version == 5)
		hasPal = a_nLen > 256*3 && a_pData[a_nLen-256*3-1] == 0x0c;
	static BYTE const a1bitPal[] = {0, 0, 0, 255, 255, 255};

	CPcxDecodeContext cCtx(a_pData+sizeof(PCXHEADER), a_pData+a_nLen-(hasPal ? 256*3+1 : 0));
	switch (bpp)
	{
	case 24:
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			TPixelChannel* p = pBuffer.m_p+y*tSize.nX;
			if (!cCtx.Read(SChannelOut(&(p->bR), tSize.nX))) return E_FAIL;
			if (tSize.nX < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-tSize.nX));
			if (!cCtx.Read(SChannelOut(&(p->bG), tSize.nX))) return E_FAIL;
			if (tSize.nX < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-tSize.nX));
			if (!cCtx.Read(SChannelOut(&(p->bB), tSize.nX))) return E_FAIL;
			if (tSize.nX < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-tSize.nX));
			for (TPixelChannel* const pEnd = p+tSize.nX; p < pEnd; ++p)
				p->bA = 0xff;
			//cCtx.LineBreak();
		}
		break;
	case 8:
		if (!hasPal)
			return E_RW_UNSUPPORTEDINPUTFORMAT;
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			if (!cCtx.Read(S8bitPalOut(pBuffer.m_p+y*tSize.nX, tSize.nX, a_pData+a_nLen-256*3)))
				return E_FAIL;
			if (tSize.nX < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-tSize.nX));
			//cCtx.LineBreak();
		}
		break;
	case 4:
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			if (!cCtx.Read(S4bitPalOut(pBuffer.m_p+y*tSize.nX, tSize.nX, pHeader->EgaPalette)))
				return E_FAIL;
			if (((tSize.nX+1)>>1) < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-((tSize.nX+1)>>1)));
		}
		break;
	case 1:
		for (ULONG y = 0; y < tSize.nY; ++y)
		{
			if (!cCtx.Read(S1bitPalOut(pBuffer.m_p+y*tSize.nX, tSize.nX, a1bitPal/*pHeader->EgaPalette*/)))
				return E_FAIL;
			if (((tSize.nX+7)>>3) < pHeader->BytesLinePlane) cCtx.Read(SSkipFillerOut(pHeader->BytesLinePlane-((tSize.nX+7)>>3)));
		}
		break;
	default:
		return E_RW_UNSUPPORTEDINPUTFORMAT;
	}

	if (a_pEncoderID) *a_pEncoderID = __uuidof(DocumentCodecPCX);

	return a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA, 0, 0, 0, 0), 0.0f, CImageTile(tSize.nX, tSize.nY, pBuffer));
}

/*VOID CDocumentCodecPCX::FillVGAPal()
{
	BYTE const* PalSource;
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
	pISPull->BufferLock(m_pInfoHeader.ImageDataSize- sizeof(VGAPal),sizeof(VGAPal),&PalSource);
	memcpy(&m_pVGAPal,PalSource,sizeof(VGAPal));
	pISPull->BufferUnlock(sizeof(VGAPal),PalSource);
}

VOID CDocumentCodecPCX::FillInfoHeader()
{

	PCXHEADER PcxHead;
	BYTE const* HeaderSource;
	TImageFormat myIF;
	m_pInputImage->FormatGet(&myIF);
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
    pISPull->BufferLock(0, sizeof(PCXHEADER), &HeaderSource);
	memcpy(&PcxHead,HeaderSource,sizeof(PCXHEADER));
    pISPull->BufferUnlock(sizeof(PCXHEADER), HeaderSource);
	m_pInfoHeader.bpp = PcxHead.BitsPixelPlane * PcxHead.NumberOfPlanes;
	m_pInfoHeader.ImageWidth  = myIF.atDims[0].nItems;
	m_pInfoHeader.ImageHeight = myIF.atDims[1].nItems;
	m_pInfoHeader.ImageDataSize = myIF.nDataSize;
	m_pInfoHeader.ImageSize = m_pInfoHeader.ImageWidth * m_pInfoHeader.ImageHeight;
	m_pInfoHeader.TotalBytes = PcxHead.BytesLinePlane * PcxHead.NumberOfPlanes;
	if (m_pInfoHeader.bpp ==24) m_pInfoHeader.LinePaddingSize = (m_pInfoHeader.TotalBytes / 3 - ((PcxHead.Xmax - PcxHead.Xmin) + 1)) * 3;
	else m_pInfoHeader.LinePaddingSize = ((PcxHead.BytesLinePlane * PcxHead.NumberOfPlanes) * (8/PcxHead.BitsPixelPlane)) -  ((PcxHead.Xmax - PcxHead.Xmin) + 1);
	m_pInfoHeader.PutSize = m_pInfoHeader.TotalBytes - m_pInfoHeader.LinePaddingSize;
	if ((m_pInfoHeader.bpp == 8) && (PcxHead.Version == 5))
	{
		BYTE const* TestByte;
		pISPull->BufferLock(m_pInfoHeader.ImageDataSize - (sizeof(VGAPal) + 1),1,&TestByte);

		if (*TestByte == 0x0C)
		{
			m_pInfoHeader.HasPal=true;
		}
		else m_pInfoHeader.HasPal=false;
		pISPull->BufferUnlock(1,TestByte);
	}
	else m_pInfoHeader.HasPal=false;

	memcpy(&m_pEGAPal,PcxHead.EgaPalette,sizeof(EGAPal));

}

VOID CDocumentCodecPCX::Get24(BYTE *Dest)
{
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
	BYTE const* DataSource;
	BYTE *DecData = new BYTE[m_pInfoHeader.ImageSize * 3 * sizeof(BYTE)]; 
    pISPull->BufferLock(sizeof(PCXHEADER),m_pInfoHeader.ImageDataSize - sizeof(PCXHEADER), &DataSource);

   	BYTE *Buffer = new BYTE[m_pInfoHeader.TotalBytes*sizeof(BYTE)];
	ULONG Lines;
   	for (Lines=0;Lines < m_pInfoHeader.ImageHeight;Lines++)
	{
	    PcxDecodeScanLine(Buffer,m_pInfoHeader.TotalBytes,&DataSource);
    	cparray(Buffer,DecData,m_pInfoHeader.PutSize,m_pInfoHeader.PutSize * Lines);
	}

   	delete[] Buffer;
	pISPull->BufferUnlock(sizeof(PCXHEADER),DataSource);
	BYTE *Temp;
	Temp=DecData;
	DecData++;
	
	for (Lines = 0;Lines < m_pInfoHeader.ImageHeight;Lines++)
	{
		ULONG Pixel=0;
		for (;Pixel < m_pInfoHeader.ImageWidth;Pixel++)
		{
		Dest[Lines * 3 * m_pInfoHeader.ImageWidth + 3 * Pixel]     = DecData[Lines * 3 * m_pInfoHeader.ImageWidth + Pixel];
		Dest[Lines * 3 * m_pInfoHeader.ImageWidth + 3 * Pixel + 1] = DecData[Lines * 3 * m_pInfoHeader.ImageWidth + Pixel + m_pInfoHeader.ImageWidth];
		Dest[Lines * 3 * m_pInfoHeader.ImageWidth + 3 * Pixel + 2] = DecData[Lines * 3 * m_pInfoHeader.ImageWidth + Pixel + 2 * m_pInfoHeader.ImageWidth];
		}
	}
	delete[] Temp;
}



VOID CDocumentCodecPCX::Get8(BYTE *Dest)
{
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
	BYTE const* DataSource;
	BYTE *DecData = new BYTE[m_pInfoHeader.ImageSize * sizeof(BYTE)]; 
    pISPull->BufferLock(sizeof(PCXHEADER),m_pInfoHeader.ImageDataSize - (sizeof(PCXHEADER) + sizeof(VGAPal) + 1), &DataSource);

   	BYTE *Buffer = new BYTE[m_pInfoHeader.TotalBytes*sizeof(BYTE)];
	ULONG Lines;
	 
   	for (Lines=0;Lines < m_pInfoHeader.ImageHeight;Lines++)
	{
	    PcxDecodeScanLine(Buffer,m_pInfoHeader.TotalBytes,&DataSource);
	   	cparray(Buffer,DecData,m_pInfoHeader.PutSize,m_pInfoHeader.PutSize * Lines);
	}

   	delete[] Buffer;
	pISPull->BufferUnlock(sizeof(PCXHEADER),DataSource);

	Lines = 0;
	BYTE *Temp;
	Temp=DecData;
	DecData++;

	while (Lines < m_pInfoHeader.ImageSize)
	{
		Dest[3*Lines]   = m_pVGAPal[DecData[Lines]].r;
		Dest[3*Lines+1] = m_pVGAPal[DecData[Lines]].g;
		Dest[3*Lines+2] = m_pVGAPal[DecData[Lines]].b;
		Lines++;
	}
	
	delete[] Temp;
}

VOID CDocumentCodecPCX::Get4(BYTE *Dest)
{
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
	BYTE const* DataSource;
	BYTE *DecData = new BYTE[m_pInfoHeader.ImageSize * sizeof(BYTE)]; 
    pISPull->BufferLock(sizeof(PCXHEADER),m_pInfoHeader.ImageDataSize - sizeof(PCXHEADER), &DataSource);

   	BYTE *Buffer = new BYTE[m_pInfoHeader.TotalBytes*sizeof(BYTE)];
	BYTE *tBuffer = new BYTE[m_pInfoHeader.TotalBytes * 2 * sizeof(BYTE)];
    ULONG Lines;
   	for (Lines=0;Lines < m_pInfoHeader.ImageHeight;Lines++)
	{
	    PcxDecodeScanLine(Buffer,m_pInfoHeader.TotalBytes,&DataSource);
	    Dec16(Buffer,tBuffer,m_pInfoHeader.TotalBytes);
    	cparray(tBuffer,DecData,m_pInfoHeader.ImageWidth,m_pInfoHeader.ImageWidth * Lines);
	}
    delete[] tBuffer; 
   	delete[] Buffer;
	pISPull->BufferUnlock(sizeof(PCXHEADER),DataSource);
	Lines=0;
	BYTE *Temp;
	Temp=DecData;
	DecData++;
	DecData++;
	
	while (Lines < m_pInfoHeader.ImageSize)
	{
		Dest[3*Lines]   = m_pEGAPal[DecData[Lines]].r;
		Dest[3*Lines+1] = m_pEGAPal[DecData[Lines]].g;
		Dest[3*Lines+2] = m_pEGAPal[DecData[Lines]].b;
		Lines++;
	}
	delete[] Temp;
}

VOID CDocumentCodecPCX::Get1(BYTE *Dest)
{
	CComPtr<IImageSourcePull> pISPull; 
    m_pInputImage->PullISGet(&pISPull);
	BYTE const* DataSource;
	BYTE *DecData = new BYTE[m_pInfoHeader.ImageSize * sizeof(BYTE)]; 
    pISPull->BufferLock(sizeof(PCXHEADER),m_pInfoHeader.ImageDataSize - sizeof(PCXHEADER), &DataSource);

   	BYTE *Buffer = new BYTE[m_pInfoHeader.TotalBytes*sizeof(BYTE)];
	BYTE *tBuffer = new BYTE[m_pInfoHeader.TotalBytes * 8 * sizeof(BYTE)];
	ULONG Lines;
	 
   	for (Lines=0;Lines < m_pInfoHeader.ImageHeight;Lines++)
	{
	    PcxDecodeScanLine(Buffer,m_pInfoHeader.TotalBytes,&DataSource);
		DecMono(Buffer,tBuffer,m_pInfoHeader.TotalBytes);
    	cparray(tBuffer,DecData,m_pInfoHeader.ImageWidth,m_pInfoHeader.ImageWidth * Lines);
	}
	delete[] tBuffer;
	delete[] Buffer;
   	pISPull->BufferUnlock(sizeof(PCXHEADER),DataSource);

	Lines = 0;
	BYTE *Temp;
	Temp=DecData;
	DecData++;

	while (Lines < m_pInfoHeader.ImageSize)
	{
		Dest[3*Lines]   = DecData[Lines] * 255;
		Dest[3*Lines+1] = DecData[Lines] * 255;
		Dest[3*Lines+2] = DecData[Lines] * 255;
		Lines++;
	}
	
	delete[] Temp;
}

*/