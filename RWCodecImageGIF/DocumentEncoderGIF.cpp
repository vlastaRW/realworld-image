// DocumentEncoderGIF.cpp : Implementation of CDocumentEncoderGIF

#include "stdafx.h"
#include "DocumentEncoderGIF.h"

#include <MultiLanguageString.h>
#include <math.h>
#include <RWDocumentAnimation.h>

/* "Disposal" method indicates how the image should be handled in the
   framebuffer before the subsequent image is displayed. */
typedef enum 
{
	DISPOSE_NOT_SPECIFIED      = 0,
	DISPOSE_KEEP               = 1, /* Leave it in the framebuffer */
	DISPOSE_OVERWRITE_BGCOLOR  = 2, /* Overwrite with background color */
	DISPOSE_OVERWRITE_PREVIOUS = 3  /* Save-under */
} gdispose;


// CDocumentEncoderGIF

STDMETHODIMP CDocumentEncoderGIF::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		CComPtr<IDocumentTypeWildcards> pDocType;
		RWCoCreateInstance(pDocType, __uuidof(DocumentTypeWildcards));
		CComBSTR bstrExt(L"gif");
		pDocType->InitEx(CMultiLanguageString::GetAuto(L"[0409]GIF image files[0405]Soubory obrázků GIF"), CMultiLanguageString::GetAuto(L"[0409]GIF Image[0405]Obrázek GIF"), 1, &(bstrExt.m_str), NULL, NULL, 0, CComBSTR(L"*.gif"));
		*a_ppDocType = pDocType.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

static OLECHAR const CFGID_INTERLACING[] = L"Interlacing";
static OLECHAR const CFGID_MAXCOLORS[] = L"MaxColors";
static OLECHAR const CFGID_DITHERING[] = L"Dithering";
#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderGIFDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderGIFDlg>,
	public CDialogResize<CConfigGUIEncoderGIFDlg>
{
public:
	enum { IDC_CGGIF_INTERLACE = 100, IDC_CGGIF_DITHER };

	BEGIN_DIALOG_EX(0, 0, 127, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		//CONTROL_LTEXT(_T("[0409]Format:[0405]Formát:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		//CONTROL_COMBOBOX(IDC_CGBMP_FORMAT, 40, 0, 50, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_CHECKBOX(_T("[0409]Dithering[0405]Ditherování"), IDC_CGGIF_DITHER, 0, 1, 60, 10, WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_CHECKBOX(_T("[0409]Interlacing[0405]Prokládání"), IDC_CGGIF_INTERLACE, 67, 1, 60, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderGIFDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderGIFDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderGIFDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderGIFDlg)
		DLGRESIZE_CONTROL(IDC_CGGIF_INTERLACE, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CGGIF_DITHER, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderGIFDlg)
		//CONFIGITEM_SLIDER_TRACKUPDATE(IDC_CGBMP_FORMAT, CFGID_MAXCOLORS)
		CONFIGITEM_CHECKBOX(IDC_CGGIF_DITHER, CFGID_DITHERING)
		CONFIGITEM_CHECKBOX(IDC_CGGIF_INTERLACE, CFGID_INTERLACING)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

IConfig* CDocumentEncoderGIF::Config()
{
	CComPtr<IConfigWithDependencies> pCfgInit;
	RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

	pCfgInit->ItemInsRanged(CComBSTR(CFGID_MAXCOLORS), CMultiLanguageString::GetAuto(L"[0409]Maximum Colors[0405]Maximum barev"), CMultiLanguageString::GetAuto(L"[0409]Maximum number of colors in palette. Lower this value to get smaller, but low-quality images.[0405]Maximální počet barev v paletě. Snižte tuto hodnotu, pokud chcete mít menší obrázky, ale s nižší kvalitou."), CConfigValue(256L), NULL, CConfigValue(2L), CConfigValue(256L), CConfigValue(1L), 0, NULL);
	pCfgInit->ItemInsSimple(CComBSTR(CFGID_DITHERING), CMultiLanguageString::GetAuto(L"[0409]Dithering[0405]Ditherování"), CMultiLanguageString::GetAuto(L"[0409]Method used for simulating continuous tones with groups of dots.[0405]Metoda použitá pro simulování spojitých barevných tónů pomocí skupin bodů."), CConfigValue(true), NULL, 0, NULL);
	pCfgInit->ItemInsSimple(CComBSTR(CFGID_INTERLACING), CMultiLanguageString::GetAuto(L"[0409]Interlacing[0405]Prokládání"), CMultiLanguageString::GetAuto(L"[0409]Enable this option to create interlaced images for web usage.[0405]Povolte tuto volbu, pokud chcete vytvořit prokládaný obrázek pro použití na internetu."), CConfigValue(false), NULL, 0, NULL);

	CConfigCustomGUI<&CLSID_DocumentEncoderGIF, CConfigGUIEncoderGIFDlg>::FinalizeConfig(pCfgInit);

	return pCfgInit.Detach();
}

STDMETHODIMP CDocumentEncoderGIF::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;
		*a_ppDefCfg = Config();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderGIF::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA ENCFEAT_ANIMATION);
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg)
			return S_OK;
		CComPtr<IDocumentAnimation> pDocAni;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDocAni));
		return pDocAni ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

template <int t_nChunkSize = 4096>
class CCachingDstStream
{
public:
	CCachingDstStream<t_nChunkSize>(IReturnedData* a_pDst) : m_pDst(a_pDst), m_nWritten(0), m_nInBuffer(0), m_bError(false)
	{
	}
	~CCachingDstStream<t_nChunkSize>() { Flush(); }
	bool m_bError;

	void Write(void const* a_pData, size_t a_nSize)
	{
		if (a_nSize >= t_nChunkSize)
		{
			Flush();
			InternWrite(a_pData, a_nSize);
		}
		else
		{
			if (a_nSize+m_nInBuffer > t_nChunkSize)
				Flush();
			memcpy(m_aBuffer+m_nInBuffer, a_pData, a_nSize);
			m_nInBuffer += a_nSize;
		}
	}

	void Flush()
	{
		if (m_nInBuffer)
		{
			InternWrite(m_aBuffer, m_nInBuffer);
			m_nInBuffer = 0;
		}
	}

private:
	void InternWrite(void const* a_pData, size_t a_nSize)
	{
		m_bError = m_bError || FAILED(m_pDst->Write(a_nSize, reinterpret_cast<BYTE const*>(a_pData)));
		m_nWritten += a_nSize;
	}
	IReturnedData* m_pDst;
	ULONG m_nWritten;
	BYTE m_aBuffer[t_nChunkSize];
	size_t m_nInBuffer;
};

static int GetColors24(BYTE const* a_pRGB, int a_nSizeX, int a_nSizeY, int a_nLineSize, int a_nMaxColors, RGBQUAD* a_aColors, BYTE* a_pColorIndices)
{
	int nLineDiff = a_nLineSize-a_nSizeX*3;
	int nUsedColors = 0;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < a_nSizeX; ++x, a_pRGB += 3, ++a_pColorIndices)
		{
			int i;
			for (i = 0; i < nUsedColors; ++i)
			{
				if (a_aColors[i].rgbBlue == a_pRGB[2] && a_aColors[i].rgbGreen == a_pRGB[1] && a_aColors[i].rgbRed == a_pRGB[0])
				{
					*a_pColorIndices = i;
					break;
				}
			}
			if (i == nUsedColors)
			{
				if (nUsedColors < a_nMaxColors)
				{
					a_aColors[nUsedColors].rgbBlue = a_pRGB[0];
					a_aColors[nUsedColors].rgbGreen = a_pRGB[1];
					a_aColors[nUsedColors].rgbRed = a_pRGB[2];
					++nUsedColors;
					*a_pColorIndices = i;
				}
				else
				{
					return nUsedColors+1;
				}
			}
		}
		a_pRGB += nLineDiff;
	}
	return nUsedColors;
}

static int GetColors32(BYTE const* a_pRGBA, int a_nSizeX, int a_nSizeY, int a_nLineSize, int a_nMaxColors, RGBQUAD* a_aColors, BYTE* a_pColorIndices, int* a_pTransparent)
{
	*a_pTransparent = -1;
	int nLineDiff = a_nLineSize-a_nSizeX*4;
	int nUsedColors = 0;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < a_nSizeX; ++x, a_pRGBA += 4, ++a_pColorIndices)
		{
			int i;
			if (a_pRGBA[3] <= 0x7f)
			{
				if (*a_pTransparent == -1)
				{
					i = *a_pTransparent = nUsedColors;
				}
				else
				{
					i = *a_pColorIndices = *a_pTransparent;
				}
			}
			else
			{
				for (i = 0; i < nUsedColors; ++i)
				{
					if (i != *a_pTransparent && a_aColors[i].rgbBlue == a_pRGBA[2] && a_aColors[i].rgbGreen == a_pRGBA[1] && a_aColors[i].rgbRed == a_pRGBA[0])
					{
						*a_pColorIndices = i;
						break;
					}
				}
			}
			if (i == nUsedColors)
			{
				if (nUsedColors < a_nMaxColors)
				{
					a_aColors[nUsedColors].rgbBlue = a_pRGBA[0];
					a_aColors[nUsedColors].rgbGreen = a_pRGBA[1];
					a_aColors[nUsedColors].rgbRed = a_pRGBA[2];
					++nUsedColors;
					*a_pColorIndices = i;
				}
				else
				{
					return nUsedColors+1;
				}
			}
		}
		a_pRGBA += nLineDiff;
	}
	return nUsedColors;
}


class TRandomMotherOfAll // random number generator
{
public:
	TRandomMotherOfAll(unsigned int seed)
	{
		RandomInit(seed);
	}

	void RandomInit(unsigned int seed)
	{
		int i;
		unsigned int s = seed;
		// make random numbers and put them into the buffer
		for (i=0; i<5; i++)
		{
			s = s * 29943829 - 1;
			x[i] = s * (1./(65536.*65536.));
		}
		// randomize some more
		for (i=0; i<19; i++)
			Random();
	}
	int IRandom(int min, int max)       // get integer random number in desired interval
	{
		int iinterval = max - min + 1;
		if (iinterval <= 0) return 0x80000000; // error
		int i = int(iinterval * Random());     // truncate
		if (i >= iinterval) i = iinterval-1;
		return min + i;
	}
	double Random()                     // get floating point random number
	{
		long double c;
		c = (long double)2111111111.0 * x[3] +
			1492.0 * (x[3] = x[2]) +
			1776.0 * (x[2] = x[1]) +
			5115.0 * (x[1] = x[0]) +
			x[4];
		x[4] = floorl(c);
		x[0] = c - x[4];
		x[4] = x[4] * (1./(65536.*65536.));
		return x[0];
	}

private:
	double x[5];                         // history buffer
};

struct TSimpleRGB
{
	BYTE bB;
	BYTE bG;
	BYTE bR;
};

struct TSimpleRGBA
{
	BYTE bB;
	BYTE bG;
	BYTE bR;
	BYTE bA;
};

#include "GIFCompressor.h"

template<int t_nBits = 8>
class CImageOcTree
{
public:
	void Reset()
	{
		m_sRoot.Reset();
	}
	void InsertPixel(TSimpleRGB a_tPixel)
	{
		int nIndR = static_cast<int>(a_tPixel.bR)<<3;
		int nIndG = static_cast<int>(a_tPixel.bG)<<2;
		int nIndB = static_cast<int>(a_tPixel.bB)<<1;
		SNode* pNode = &m_sRoot;
		pNode->Insert(a_tPixel);
		int iLevel;
		for (iLevel = 0; iLevel < t_nBits; iLevel++)
		{
			int iSubNode =
				((nIndR&(1<<(t_nBits+2-iLevel))) |
				 (nIndG&(1<<(t_nBits+1-iLevel))) |
				 (nIndB&(1<<(t_nBits-iLevel))))>>(t_nBits-iLevel);
			if (pNode->m_aSubNodes[iSubNode] == NULL)
			{
				pNode->m_aSubNodes[iSubNode] = new SNode(iLevel, a_tPixel);
			}
			else
			{
				pNode->m_aSubNodes[iSubNode]->Insert(a_tPixel);
			}
			pNode = pNode->m_aSubNodes[iSubNode];
		}
	}

	int GetPalette(RGBQUAD* a_pPalEntries, int a_nPalEntries)
	{
		if (m_sRoot.m_nPixels == 0 || a_nPalEntries == 0)
			return 0;

		// compute errors
		m_sRoot.PrepareNode();

		SNode const** aBuffer = reinterpret_cast<SNode const**>(_alloca(sizeof(SNode const*)*a_nPalEntries));
		aBuffer[0] = &m_sRoot;
		int nOccupied = 1;
		while (true)
		{
			int i;
			for (i = 0; i < nOccupied; i++)
			{
				int nChildren = aBuffer[i]->GetChildCount();
				if (aBuffer[i]->m_nPixels > 1 && nChildren > 0 && nChildren <= (a_nPalEntries-nOccupied+1))
					break;
			}
			if (i < nOccupied)
			{
				SNode const* pNode = aBuffer[i];
				// delete the node from buffer
				memcpy(aBuffer+i, aBuffer+i+1, sizeof(SNode const*)*(nOccupied-i-1));
				// generate sequence of new nodes
				SNode const* aSubNodes[8];
				int nSubNodes = 0;
				for (int j = 0; j < 8; j++)
				{
					if (pNode->m_aSubNodes[j])
					{
						aSubNodes[nSubNodes] = pNode->m_aSubNodes[j];
						int nChildren = aSubNodes[nSubNodes]->GetChildCount();
						while (nChildren == 1 && aSubNodes[nSubNodes]->m_nPixels)
						{
							for (int k = 0; k < 8; k++)
							{
								if (aSubNodes[nSubNodes]->m_aSubNodes[k])
								{
									aSubNodes[nSubNodes] = aSubNodes[nSubNodes]->m_aSubNodes[k];
									break;
								}
							}
							nChildren = aSubNodes[nSubNodes]->GetChildCount();
						}
						nSubNodes++;
					}
				}
				// short bubble-sort
				for (int i1 = 1; i1 < nSubNodes; i1++)
				{
					for (int i2 = nSubNodes-1; i2 >= i1; i2--)
					{
						if (aSubNodes[i2]->Weight() > aSubNodes[i2-1]->Weight())
						{
							SNode const* pTmp = aSubNodes[i2];
							aSubNodes[i2] = aSubNodes[i2-1];
							aSubNodes[i2-1] = pTmp;
						}
					}
				}
				// merge the sequences
				int iOldLast = nOccupied-2;
				int iNewLast = nSubNodes-1;
				nOccupied += nSubNodes-1;
				for (int iDstLast = nOccupied-1; iDstLast >= 0; iDstLast--) // the condition is acually redundant (one of the breaks will do the job)
				{
					if (iOldLast < 0)
					{
						// copy rest of new sequence to the destination
						memcpy(aBuffer, aSubNodes, sizeof(SNode const*)*(iNewLast+1));
						break;
					}
					else if (iNewLast < 0)
					{
						// nothing to do (old sequence is already in place)
						break;
					}
					else if (aBuffer[iOldLast]->Weight() <= aSubNodes[iNewLast]->Weight())
					{
						aBuffer[iDstLast] = aBuffer[iOldLast--];
					}
					else
					{
						aBuffer[iDstLast] = aSubNodes[iNewLast--];
					}
				}
			}
			else
			{
				break;
			}
		}

		m_nLastTimeStamp = 1;

		// generate the palette from the buffer
		for (int iEntry = 0; iEntry < nOccupied; iEntry++)
		{
			if (aBuffer[iEntry]->m_nPixels)
			{
				a_pPalEntries[iEntry].rgbRed = static_cast<BYTE>(aBuffer[iEntry]->m_nR / aBuffer[iEntry]->m_nPixels);
				a_pPalEntries[iEntry].rgbGreen = static_cast<BYTE>(aBuffer[iEntry]->m_nG / aBuffer[iEntry]->m_nPixels);
				a_pPalEntries[iEntry].rgbBlue = static_cast<BYTE>(aBuffer[iEntry]->m_nB / aBuffer[iEntry]->m_nPixels);
			}
			else
			{
				a_pPalEntries[iEntry].rgbRed = a_pPalEntries[iEntry].rgbGreen = a_pPalEntries[iEntry].rgbBlue = 0;
			}
			a_pPalEntries[iEntry].rgbReserved = 0;
			aBuffer[iEntry]->m_iIndex = iEntry;
			aBuffer[iEntry]->m_nTimeStamp = m_nLastTimeStamp;
		}

		return nOccupied;
	}

	int FindColorIndex(TSimpleRGB a_tPixel) const //must be called AFTER GetPalette
	{
		int nIndR = static_cast<int>(a_tPixel.bR)<<3;
		int nIndG = static_cast<int>(a_tPixel.bG)<<2;
		int nIndB = static_cast<int>(a_tPixel.bB)<<1;
		SNode const* pNode = &m_sRoot;
		int iLevel;
		for (iLevel = 0; pNode->m_nTimeStamp != m_nLastTimeStamp; iLevel++)
		{
			int iSubNode =
				((nIndR&(1<<(t_nBits+2-iLevel))) |
				 (nIndG&(1<<(t_nBits+1-iLevel))) |
				 (nIndB&(1<<(t_nBits-iLevel))))>>(t_nBits-iLevel);
			if (pNode->m_aSubNodes[iSubNode] == NULL)
			{
				ATLASSERT(0); // incorrect palette???
				return -1;
			}
			pNode = pNode->m_aSubNodes[iSubNode];
		}
		return pNode->m_iIndex;
	}
	int ColorCount() const
	{
		return m_sRoot.ColorCount();
	}

private:
	struct SNode
	{
		SNode() : m_nTimeStamp(0), m_iLevel(-1), m_nR(0), m_nG(0), m_nB(0), m_nPixels(0)
		{
			ZeroMemory(m_aSubNodes, sizeof m_aSubNodes);
		}
		SNode(int a_iLevel, TSimpleRGB a_tPixel) : m_nTimeStamp(0), m_iLevel(a_iLevel),
			m_nR(a_tPixel.bR), m_nG(a_tPixel.bG), m_nB(a_tPixel.bB), m_nPixels(1)
		{
			ZeroMemory(m_aSubNodes, sizeof m_aSubNodes);
		}
		~SNode()
		{
			for (size_t i = 0; i < itemsof(m_aSubNodes); i++)
				delete m_aSubNodes[i];
		}
		void Reset()
		{
			for (size_t i = 0; i < itemsof(m_aSubNodes); i++)
			{
				delete m_aSubNodes[i];
				m_aSubNodes[i] = 0;
			}
			m_nTimeStamp = 0;
			m_iLevel = -1;
			m_nR = m_nG = m_nB = 0;
			m_nPixels = 0;
		}
		void Insert(TSimpleRGB a_tPixel)
		{
			m_nR += a_tPixel.bR;
			m_nG += a_tPixel.bG;
			m_nB += a_tPixel.bB;
			m_nPixels++;
		}
		int GetChildCount() const
		{
			int iCount = 0;
			for (int i = 0; i < 8; i++)
			{
				if (m_aSubNodes[i])
					iCount++;
			}
			return iCount;
		}
		int ColorCount() const
		{
			if (m_iLevel == (t_nBits-1))
			{
				return 1;
			}
			int iCnt = 0;
			for (int i = 0; i < 8; i++)
			{
				if (m_aSubNodes[i])
					iCnt += m_aSubNodes[i]->ColorCount();
			}
			return iCnt;
		}

		double Weight() const
		{
			return m_fError;//m_nPixels << ((t_nBits-m_iLevel) >> 2);
			//return (m_nPixels << 4) + (t_nBits-m_iLevel);
		}
		void PrepareNode()
		{
			for (int j = 0; j < 8; j++)
			{
				if (m_aSubNodes[j])
					m_aSubNodes[j]->PrepareNode();
			}
			if (m_iLevel == (t_nBits-1))
			{
				m_bFinalR = static_cast<BYTE>(m_nR / m_nPixels);
				m_bFinalG = static_cast<BYTE>(m_nG / m_nPixels);
				m_bFinalB = static_cast<BYTE>(m_nB / m_nPixels);
				m_fError = 0.0;
			}
			else
			{
				m_bFinalR = static_cast<BYTE>((m_nR+(m_nPixels>>1)) / m_nPixels);
				m_bFinalG = static_cast<BYTE>((m_nG+(m_nPixels>>1)) / m_nPixels);
				m_bFinalB = static_cast<BYTE>((m_nB+(m_nPixels>>1)) / m_nPixels);
				m_fError = GetError(m_bFinalR, m_bFinalG, m_bFinalB);
			}
		}
		double GetError(int a_nR, int a_nG, int a_nB) const
		{
			if (m_iLevel == (t_nBits-1))
			{
				return ((a_nR-m_bFinalR)*(a_nR-m_bFinalR) + (a_nG-m_bFinalG)*(a_nG-m_bFinalG) + (a_nB-m_bFinalB)*(a_nB-m_bFinalB)) * m_nPixels;
			}
			else
			{
				double tmp = 0.0;
				for (int j = 0; j < 8; j++)
					if (m_aSubNodes[j])
						tmp += m_aSubNodes[j]->GetError(a_nR, a_nG, a_nB);
				return tmp;
			}
		}

		int m_iLevel;
		ULONG m_nR;
		ULONG m_nG;
		ULONG m_nB;
		ULONG m_nPixels;
		double m_fError;
		BYTE m_bFinalR;
		BYTE m_bFinalG;
		BYTE m_bFinalB;
		SNode* m_aSubNodes[8];

		mutable int m_iIndex;
		mutable ULONG m_nTimeStamp;
	};

private:
	SNode m_sRoot;
	mutable ULONG m_nLastTimeStamp;
};

struct CIgnoreTransparency
{
	template<class TInIterator, class TOutIterator>
	bool operator()(TInIterator a_pSrc, TOutIterator a_pDst) { return false; }
	int FixIndex(int a_nIndex)
	{
		return a_nIndex;
	}
};

struct CSkipTransparent
{
	template<class TInIterator, class TOutIterator>
	bool operator()(TInIterator a_pSrc, TOutIterator a_pDst)
	{
		if (a_pSrc->bA >= 0x80)
			return false;
		*a_pDst = 0;
		return true;
	}
	int FixIndex(int a_nIndex)
	{
		return a_nIndex+1;
	}
};

template<class TInIterator, class TOutIterator, class TTransparencyHandler>
static void DirectToIndexed(TInIterator a_pSrc, RGBQUAD const* a_pPalette, int a_nPalColors, int a_nSizeX, int a_nSizeY, int a_nLinearSize, TOutIterator a_pDst, TTransparencyHandler a_tTransHandler)
{
#define FS_SHIFT   10
	a_pDst += -a_nSizeX-1;

	CAutoVectorPtr<int> aErrs(new int[(2+a_nSizeX)*6]);
	int* thisrerr = aErrs;
	int* nextrerr = thisrerr+2+a_nSizeX;
	int* thisgerr = nextrerr+2+a_nSizeX;
	int* nextgerr = thisgerr+2+a_nSizeX;
	int* thisberr = nextgerr+2+a_nSizeX;
	int* nextberr = thisberr+2+a_nSizeX;
	TRandomMotherOfAll cRnd(42);
	for (int x = 0; x < 2+a_nSizeX; ++x)
	{
		// (random errors in [-1 .. 1])
		thisrerr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
		thisgerr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
		thisberr[x] = cRnd.IRandom(-(1<<FS_SHIFT), 1<<FS_SHIFT);
	}
	bool fs_direction = true;

	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < 2+a_nSizeX; ++x)
			nextrerr[x] = nextgerr[x] = nextberr[x] = 0;

		TInIterator pP;
		int actX;
		int lastX;
		if (fs_direction)
		{
			actX = 0;
			lastX = a_nSizeX;
			pP = reinterpret_cast<TInIterator>(reinterpret_cast<BYTE const*>(a_pSrc) + y*a_nLinearSize);
			a_pDst += a_nSizeX+1;
		}
		else
		{
			actX = a_nSizeX - 1;
			lastX = -1;
			pP = reinterpret_cast<TInIterator>(reinterpret_cast<BYTE const*>(a_pSrc) + y*a_nLinearSize)+actX;
			a_pDst += actX;
		}
		do
		{
			if (!a_tTransHandler(pP, a_pDst))
			{
				int ind;
				// Use Floyd-Steinberg errors to adjust actual color.
				int sr = pP->bR + ((thisrerr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
				int sg = pP->bG + ((thisgerr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
				int sb = pP->bB + ((thisberr[actX + 1] + (1<<(FS_SHIFT-1))) >> FS_SHIFT);
				//if (sr < 0) sr = 0;
				//else if (sr > 255) sr = 255;
				//if (sg < 0) sg = 0;
				//else if (sg > 255) sg = 255;
				//if (sb < 0) sb = 0;
				//else if (sb > 255) sb = 255;

				int dist = (sr-a_pPalette[0].rgbRed)*(sr-a_pPalette[0].rgbRed) + (sg-a_pPalette[0].rgbGreen)*(sg-a_pPalette[0].rgbGreen) + (sb-a_pPalette[0].rgbBlue)*(sb-a_pPalette[0].rgbBlue);
				ind = 0;
				for (int i = 1; i < a_nPalColors; ++i)
				{
					int r2 = a_pPalette[i].rgbRed;
					int g2 = a_pPalette[i].rgbGreen;
					int b2 = a_pPalette[i].rgbBlue;
					int newdist = (sr-r2)*(sr-r2) + (sg-g2)*(sg-g2) + (sb-b2)*(sb-b2);
					if (newdist < dist)
					{
						ind = i;
						dist = newdist;
					}
				}

				// Propagate Floyd-Steinberg error terms.
				if (fs_direction)
				{
					int err = thisrerr[actX + 1] + ((pP->bR - (int)a_pPalette[ind].rgbRed)<<FS_SHIFT);
					thisrerr[actX + 2] += ( err * 7 ) / 16;
					nextrerr[actX    ] += ( err * 3 ) / 16;
					nextrerr[actX + 1] += ( err * 5 ) / 16;
					nextrerr[actX + 2] += ( err     ) / 16;
					err = thisgerr[actX + 1] + ((pP->bG - (int)a_pPalette[ind].rgbGreen)<<FS_SHIFT);
					thisgerr[actX + 2] += ( err * 7 ) / 16;
					nextgerr[actX    ] += ( err * 3 ) / 16;
					nextgerr[actX + 1] += ( err * 5 ) / 16;
					nextgerr[actX + 2] += ( err     ) / 16;
					err = thisberr[actX + 1] + ((pP->bB - (int)a_pPalette[ind].rgbBlue)<<FS_SHIFT);
					thisberr[actX + 2] += ( err * 7 ) / 16;
					nextberr[actX    ] += ( err * 3 ) / 16;
					nextberr[actX + 1] += ( err * 5 ) / 16;
					nextberr[actX + 2] += ( err     ) / 16;
				}
				else
				{
					int err = thisrerr[actX + 1] + ((pP->bR - (int)a_pPalette[ind].rgbRed)<<FS_SHIFT);
					thisrerr[actX    ] += ( err * 7 ) / 16;
					nextrerr[actX + 2] += ( err * 3 ) / 16;
					nextrerr[actX + 1] += ( err * 5 ) / 16;
					nextrerr[actX    ] += ( err     ) / 16;
					err = thisgerr[actX + 1] + ((pP->bG - (int)a_pPalette[ind].rgbGreen)<<FS_SHIFT);
					thisgerr[actX    ] += ( err * 7 ) / 16;
					nextgerr[actX + 2] += ( err * 3 ) / 16;
					nextgerr[actX + 1] += ( err * 5 ) / 16;
					nextgerr[actX    ] += ( err     ) / 16;
					err = thisberr[actX + 1] + ((pP->bB - (int)a_pPalette[ind].rgbBlue)<<FS_SHIFT);
					thisberr[actX    ] += ( err * 7 ) / 16;
					nextberr[actX + 2] += ( err * 3 ) / 16;
					nextberr[actX + 1] += ( err * 5 ) / 16;
					nextberr[actX    ] += ( err     ) / 16;
				}

				// prevent excessive color bleeding
				if (thisrerr[actX    ] > (192<<FS_SHIFT))
					thisrerr[actX    ] = 192<<FS_SHIFT;
				else if (thisrerr[actX    ] < -(192<<FS_SHIFT))
					thisrerr[actX    ] = -(192<<FS_SHIFT);

				if (nextgerr[actX + 2] > (192<<FS_SHIFT))
					nextgerr[actX + 2] = 192<<FS_SHIFT;
				else if (nextgerr[actX + 2] < -(192<<FS_SHIFT))
					nextgerr[actX + 2] = -(192<<FS_SHIFT);

				if (nextgerr[actX + 1] > (192<<FS_SHIFT))
					nextgerr[actX + 1] = 192<<FS_SHIFT;
				else if (nextgerr[actX + 1] < -(192<<FS_SHIFT))
					nextgerr[actX + 1] = -(192<<FS_SHIFT);

				if (nextgerr[actX    ] > (192<<FS_SHIFT))
					nextgerr[actX    ] = 192<<FS_SHIFT;
				else if (nextgerr[actX    ] < -(192<<FS_SHIFT))
					nextgerr[actX    ] = -(192<<FS_SHIFT);

				*a_pDst = a_tTransHandler.FixIndex(ind);
			}

			if (fs_direction)
			{
				++actX;
				++pP;
				++a_pDst;
			}
			else
			{
				--actX;
				--pP;
				--a_pDst;
			}
		}
		while (actX != lastX);

		int* temperr = thisrerr;
		thisrerr = nextrerr;
		nextrerr = temperr;
		temperr = thisgerr;
		thisgerr = nextgerr;
		nextgerr = temperr;
		temperr = thisberr;
		thisberr = nextberr;
		nextberr = temperr;
		fs_direction = ! fs_direction;
	}
}

template<class CWriter>
void SaveGIF(int a_nPalSize, RGBQUAD const* a_aColors, int a_nSizeX, int a_nSizeY, BYTE const* a_aColorIndices, CWriter& a_cWriter, bool a_bInterlacing, int a_nTransparentIndex = -1)
{
	a_cWriter.Write(a_nTransparentIndex == -1 ? "GIF87a" : "GIF89a", 6);
	// screen descriptor
	a_cWriter.Write(&a_nSizeX, 2); // BIG endian assumed
	a_cWriter.Write(&a_nSizeY, 2); // BIG endian assumed
	BYTE datadepth = 0;
	for (int i = 2; i < a_nPalSize; i <<= 1)
		++datadepth;
	BYTE tmp = 0xf0 | datadepth;
	a_cWriter.Write(&tmp, 1);
	tmp = 0;
	a_cWriter.Write(&tmp, 1); // Background color = colortable index 0
	a_cWriter.Write(&tmp, 1); // Pixel aspect ratio - 0x00 is default

	for(int i = 0; i < a_nPalSize; ++i)
	{
		a_cWriter.Write(&a_aColors[i].rgbRed, 1);
		a_cWriter.Write(&a_aColors[i].rgbGreen, 1);
		a_cWriter.Write(&a_aColors[i].rgbBlue, 1);
	}
	while (a_nPalSize < (2<<datadepth))
	{
		BYTE black[3] = {0, 0, 0};
		a_cWriter.Write(black, 3);
		++a_nPalSize;
	}
	if (a_nTransparentIndex != -1)
	{
		tmp = '!';  a_cWriter.Write(&tmp, 1); // Extension Introducer
		tmp = 0xf9; a_cWriter.Write(&tmp, 1); // Graphic Control Label
		tmp = 0x04; a_cWriter.Write(&tmp, 1); // block size
		tmp = 0x01; a_cWriter.Write(&tmp, 1); // flags (->transparent)
		tmp = 0x00; a_cWriter.Write(&tmp, 1); a_cWriter.Write(&tmp, 1); // delay
		tmp = a_nTransparentIndex; a_cWriter.Write(&tmp, 1); // transparent index
		tmp = 0x00; a_cWriter.Write(&tmp, 1); // block terminator
	}
	tmp = ',';
	a_cWriter.Write(&tmp, 1);
	tmp = 0;
	a_cWriter.Write(&tmp, 1); // "Image Left"
	a_cWriter.Write(&tmp, 1);
	a_cWriter.Write(&tmp, 1); // "Image Top"
	a_cWriter.Write(&tmp, 1);
	a_cWriter.Write(&a_nSizeX, 2); // BIG endian assumed
	a_cWriter.Write(&a_nSizeY, 2); // BIG endian assumed

	tmp = a_bInterlacing ? 0x40 | datadepth : datadepth;
	a_cWriter.Write(&tmp, 1);

	// "Raster Data"
	gifcompressor<CWriter> gc(a_cWriter);
	gc.writedatablocks(a_aColorIndices, a_nSizeX*a_nSizeY, a_nSizeX, datadepth+1, a_bInterlacing);

	tmp = ';';
	a_cWriter.Write(&tmp, 1); // GIF terminator
}

HRESULT SaveSingleFrameGIF(IDocumentImage* pI, LONG nColors, bool bInterlacing, bool bDithering, IReturnedData* a_pDst)
{
	if (pI == NULL)
		return E_FAIL;
	CBGRABuffer cBuffer;
	if (!cBuffer.Init(pI))
		return E_FAIL;

	if (!cBuffer.bAlpha)
		cBuffer.ToBGR();

	BYTE const* pBuffer = cBuffer.aData;
	ULONG const nSizeX = cBuffer.tSize.nX;
	ULONG const nSizeY = cBuffer.tSize.nY;
	ULONG const nStride = cBuffer.nStride;

	if (nColors < 2)
		nColors = 256;
	RGBQUAD aColors[256];
	ZeroMemory(aColors, sizeof aColors);
	CAutoVectorPtr<BYTE> aColorIndices(new BYTE[nSizeX * nSizeY]);

	HRESULT hRes = S_OK;
	try
	{
		if (!cBuffer.bAlpha)
		{ // no alpha
			int nPalSize = GetColors24(pBuffer, nSizeX, nSizeY, nStride, nColors, aColors, aColorIndices);
			if (nPalSize < 2) nPalSize = 2;
			if (nPalSize <= nColors)
			{ // save image exactly
				CCachingDstStream<> cWriter(a_pDst);
				SaveGIF(nPalSize, aColors, nSizeX, nSizeY, aColorIndices, cWriter, bInterlacing);
				cWriter.Flush();
				if (cWriter.m_bError)
					hRes = E_FAIL;
			}
			else
			{ // create palette
				// TODO: create palette with extreme colors
				CImageOcTree<8> cOcTree;
				int nLineDiff = nStride-nSizeX*3;
				BYTE const* pSrc = pBuffer;
				for (ULONG y = 0; y < nSizeY; ++y)
				{
					for (ULONG x = 0; x < nSizeX; ++x, pSrc += 3)
					{
						cOcTree.InsertPixel(*reinterpret_cast<TSimpleRGB const*>(pSrc));
					}
					pSrc += nLineDiff;
				}

				cOcTree.GetPalette(aColors, nColors);

				if (bDithering)
				{ // dithering
					DirectToIndexed(reinterpret_cast<TSimpleRGB const*>(pBuffer), aColors, nColors, nSizeX, nSizeY, nStride, aColorIndices.m_p, CIgnoreTransparency());
				}
				else
				{ // nearest color
					pSrc = pBuffer;
					BYTE* pDst = aColorIndices.m_p;
					for (ULONG y = 0; y < nSizeY; ++y)
					{
						for (ULONG x = 0; x < nSizeX; ++x, pSrc += 3, ++pDst)
						{
							*pDst = static_cast<BYTE>(cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
						}
						pSrc += nLineDiff;
					}
				}

				CCachingDstStream<> cWriter(a_pDst);
				SaveGIF(nColors, aColors, nSizeX, nSizeY, aColorIndices, cWriter, bInterlacing);
				cWriter.Flush();
				if (cWriter.m_bError)
					hRes = E_FAIL;
			}
		}
		else
		{ // possible alpha channel
			int nTransparentIndex = -1;
			int nPalSize = GetColors32(pBuffer, nSizeX, nSizeY, nStride, nColors, aColors, aColorIndices, &nTransparentIndex);
			if (nPalSize < 2) nPalSize = 2;
			if (nPalSize <= nColors)
			{ // save image exactly
				CCachingDstStream<> cWriter(a_pDst);
				SaveGIF(nPalSize, aColors, nSizeX, nSizeY, aColorIndices, cWriter, bInterlacing, nTransparentIndex);
				cWriter.Flush();
				if (cWriter.m_bError)
					hRes = E_FAIL;
			}
			else
			{ // create palette
				// TODO: create palette with extreme colors
				CImageOcTree<8> cOcTree;
				int nLineDiff = nStride-nSizeX*4;
				BYTE const* pSrc = pBuffer;
				bool bTransparent = false;
				for (ULONG y = 0; y < nSizeY; ++y)
				{
					for (ULONG x = 0; x < nSizeX; ++x, pSrc += 4)
					{
						if (pSrc[3] >= 0x80)
							cOcTree.InsertPixel(*reinterpret_cast<TSimpleRGB const*>(pSrc));
						else
							bTransparent = true;
					}
					pSrc += nLineDiff;
				}

				int nTransparentIndex = -1;
				if (bTransparent)
				{
					nTransparentIndex = 0;
					cOcTree.GetPalette(aColors+1, nColors-1);
				}
				else
				{
					cOcTree.GetPalette(aColors, nColors);
				}

				if (bDithering)
				{ // dithering
					if (bTransparent)
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(pBuffer), aColors+1, nColors-1, nSizeX, nSizeY, nStride, aColorIndices.m_p, CSkipTransparent());
					else
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(pBuffer), aColors, nColors, nSizeX, nSizeY, nStride, aColorIndices.m_p, CIgnoreTransparency());
				}
				else
				{ // nearest color
					pSrc = pBuffer;
					BYTE* pDst = aColorIndices.m_p;
					for (ULONG y = 0; y < nSizeY; ++y)
					{
						for (ULONG x = 0; x < nSizeX; ++x, pSrc += 4, ++pDst)
						{
							if (bTransparent)
							{
								if (pSrc[3] >= 0x80)
									*pDst = 1+static_cast<BYTE>(cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
								else
									*pDst = 0;
							}
							else
							{
								*pDst = static_cast<BYTE>(cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
							}
						}
						pSrc += nLineDiff;
					}
				}

				CCachingDstStream<> cWriter(a_pDst);
				SaveGIF(nColors, aColors, nSizeX, nSizeY, aColorIndices, cWriter, bInterlacing, nTransparentIndex);
				cWriter.Flush();
				if (cWriter.m_bError)
					hRes = E_FAIL;
			}
		}
	}
	catch (...)
	{
		hRes = E_UNEXPECTED;
	}

	return hRes;
}

struct SAutoInt
{
	SAutoInt() : iVal(0) {};
	void operator++() { ++iVal; }
	int iVal;
};
typedef std::map<DWORD, SAutoInt> CColorCounts;
//CColorCounts cCounts;
//cCounts[*reinterpret_cast<DWORD*>(p)]++;
//nColors = cCounts.size();

struct SGIFFrame
{
	enum { ECompareMask = 0xfffcfcfc };
	SGIFFrame() : nSizeX(0), nSizeY(0), bAlpha(false), nDelay(10) {}
	HRESULT Init(IDocumentAnimation* a_pAni, IUnknown* a_pFrame, ULONG a_nAniSizeX, ULONG a_nAniSizeY)
	{
		CComPtr<IDocument> pFrameDoc;
		if (FAILED(a_pAni->FrameGetDoc(a_pFrame, &pFrameDoc)) || pFrameDoc == NULL)
			return E_FAIL;
		CComPtr<IDocumentImage> pFrameImg;
		pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
		if (pFrameImg == NULL)
			return E_FAIL;
		TImageSize tSize = {1, 1};
		TImageSize tContSize = {1, 1};
		TImagePoint tContOrig = {0, 0};
		pFrameImg->CanvasGet(&tSize, NULL, &tContOrig, &tContSize, NULL);
		TImagePoint tContEnd = {tContOrig.nX+tContSize.nX, tContOrig.nY+tContSize.nY};
		if (tContOrig.nX < 0) tContOrig.nX = 0;
		if (tContOrig.nY < 0) tContOrig.nY = 0;
		if (tContEnd.nX > LONG(a_nAniSizeX)) tContEnd.nX = LONG(a_nAniSizeX);
		if (tContEnd.nY > LONG(a_nAniSizeY)) tContEnd.nY = LONG(a_nAniSizeY);
		CPixelChannel tDefault;
		pFrameImg->ChannelsGet(NULL, NULL, CImageChannelDefaultGetter(EICIRGBA, &tDefault));
		nX0 = nY0 = 0;
		nSizeX = nX1 = a_nAniSizeX;
		nSizeY = nY1 = a_nAniSizeY;
		bDispose = DISPOSE_OVERWRITE_BGCOLOR;
		pData.Attach(new BYTE[nSizeX*nSizeY<<2]);
		if (tDefault.bA == 0 && (tContOrig.nX > 0 || tContOrig.nY > 0 || tContEnd.nX < LONG(nSizeX) || tContEnd.nX < LONG(nSizeY)))
		{
			bAlpha = true;
			if (tContOrig.nX < tContEnd.nX && tContOrig.nY < tContEnd.nY)
			{
				nX0 = tContOrig.nX;
				nY0 = tContOrig.nY;
				nX1 = tContEnd.nX;
				nY1 = tContEnd.nY;
			}
			else
			{
				// frame empty
				nX1 = nY1 = 1;
			}
		}
		else
		{
			bAlpha = false;
		}
		if (FAILED(pFrameImg->TileGet(EICIRGBA, NULL, CImageSize(nSizeX, nSizeY), NULL, nSizeX*nSizeY, reinterpret_cast<TPixelChannel*>(pData.m_p), NULL, EIRIAccurate)))
			return E_FAIL;

		BYTE* p = pData.m_p+((nSizeX*nY0+nX0)<<2);
		for (ULONG nY = nY0; nY < nY1; ++nY)
		{
			for (ULONG nX = nX0; nX < nX1; ++nX, p+=4)
			{
				if (p[3] < 0x80)
				{
					*reinterpret_cast<DWORD*>(p) = 0;
					bAlpha = true;
				}
				else
				{
					p[3] = 0xff;
				}
			}
			p += (nSizeX-nX1+nX0)<<2;
		}
		if (bAlpha)
			bAlpha = Trim();
		float fDelay = 0.1f;
		a_pAni->FrameGetTime(a_pFrame, &fDelay);
		nDelay = fDelay*100.0f+0.5f;
		return S_OK;
	}
	static bool RowEmpty(BYTE const* p, ULONG const n)
	{
		for (BYTE const* const pEnd = p+(n<<2); p < pEnd; p+=4)
			if (p[3])
				return false;
		return true;
	}
	static bool ColEmpty(BYTE const* p, ULONG const n, ULONG const line)
	{
		for (BYTE const* const pEnd = p+n*line; p < pEnd; p+=line)
			if (p[3])
				return false;
		return true;
	}
	bool Trim()
	{
		// check if there is an empty border and if yes, reduce the frame dimensions
		bool bChange = false;
		ULONG n;
		BYTE const* p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nY0; n < nY1; ++n, p+=nSizeX<<2)
			if (!RowEmpty(p, nX1-nX0))
				break;
		if (n == nY1)
		{
			// image is completely transparent, reduce it to single pixel
			nX0 = nY0 = 0;
			nX1 = nY1 = 1;
			return true;
		}
		if (n > nY0)
		{
			bChange = true;
			nY0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY1+(nX0<<2);
		for (n = nY1; n > nY0; --n, p-=nSizeX<<2)
			if (!RowEmpty(p-(nSizeX<<2), nX1-nX0))
				break;
		if (n < nY1)
		{
			bChange = true;
			nY1 = n;
		}

		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (n = nX0; n < nX1; ++n, p+=4)
			if (!ColEmpty(p, nY1-nY0, nSizeX<<2))
				break;
		if (n > nX0)
		{
			bChange = true;
			nX0 = n;
		}
		p = pData.m_p+(nSizeX<<2)*nY0+(nX1<<2);
		for (n = nX1; n > nX0; --n, p-=4)
			if (!ColEmpty(p-4, nY1-nY0, nSizeX<<2))
				break;
		if (n < nX1)
		{
			bChange = true;
			nX1 = n;
		}
		if (!bChange)
			return bAlpha;

		bool bNewAlpha = false;
		p = pData.m_p+(nSizeX<<2)*nY0+(nX0<<2);
		for (ULONG nY = nY0; nY < nY1; ++nY)
		{
			for (ULONG nX = nX0; nX < nX1; ++nX, p+=4)
			{
				if (p[3] == 0)
					bNewAlpha = true;
			}
			p += (nSizeX-nX1+nX0)<<2;
		}
		return bNewAlpha;
	}
	template<typename TIterator>
	static void OptimizeRectangles(TIterator a_tBegin, TIterator a_tEnd) // optimize logical screen sizes and disposal methods
	{
		if (a_tBegin == a_tEnd)
			return;
		TIterator p = a_tBegin;
		TIterator c = a_tBegin;
		TIterator b = a_tEnd;
		for (++c; c != a_tEnd; ++c, ++p)
		{
			DWORD const* pP = reinterpret_cast<DWORD const*>(p->pData.m_p);
			DWORD const* pC = reinterpret_cast<DWORD const*>(c->pData.m_p);
			LONG nX0 = 0;//min(p->nX0, c->nX0);
			LONG nX1 = c->nSizeX;//max(p->nX1, c->nX1);
			LONG nY0 = 0;//min(p->nY0, c->nY0);
			LONG nY1 = c->nSizeY;//max(p->nY1, c->nY1);
			LONG nChgX0 = nX1+1;
			LONG nChgX1 = nX0-1;
			LONG nChgY0 = nY1+1;
			LONG nChgY1 = nY0-1;
			ULONG nChg = 0;
			LONG nTrX0 = nX1+1;
			LONG nTrX1 = nX0-1;
			LONG nTrY0 = nY1+1;
			LONG nTrY1 = nY0-1;
			ULONG nTr = 0;
			ULONG n = nY0*c->nSizeX+nX0;
			for (LONG nY = nY0; nY < nY1; ++nY)
			{
				for (LONG nX = nX0; nX < nX1; ++nX, ++n)
				{
					if ((pP[n]^pC[n])&ECompareMask)
					{
						if (nChgX0 > nX) nChgX0 = nX;
						if (nChgY0 > nY) nChgY0 = nY;
						if (nChgX1 <= nX) nChgX1 = nX+1;
						if (nChgY1 <= nY) nChgY1 = nY+1;
						++nChg;
						if (pC[n] == 0)
						{
							if (nTrX0 > nX) nTrX0 = nX;
							if (nTrY0 > nY) nTrY0 = nY;
							if (nTrX1 <= nX) nTrX1 = nX+1;
							if (nTrY1 <= nY) nTrY1 = nY+1;
							++nTr;
						}
					}
				}
				n += c->nSizeX - (nX1-nX0);
			}
			if (nChgX0 >= nChgX1)
			{
				// frames are the same - reduce to 1 pixel and keep the previous one
				c->nX0 = c->nY0 = 0;
				c->nX1 = c->nY1 = 1;
				p->bDispose = DISPOSE_KEEP;
				b = p;
			}
			else
			{
				if (b != a_tEnd)
				{
					DWORD const* pB = reinterpret_cast<DWORD const*>(b->pData.m_p);
					LONG nX0 = 0;//min(b->nX0, c->nX0);
					LONG nX1 = c->nSizeX;// max(b->nX1, c->nX1);
					LONG nY0 = 0;//min(b->nY0, c->nY0);
					LONG nY1 = c->nSizeY;//max(b->nY1, c->nY1);
					LONG nBgrX0 = nX1+1;
					LONG nBgrX1 = nX0-1;
					LONG nBgrY0 = nY1+1;
					LONG nBgrY1 = nY0-1;
					ULONG nBgr = 0;
					ULONG n = nY0*c->nSizeX+nX0;
					bool bContinue = true;
					for (LONG nY = nY0; nY < nY1 && bContinue; ++nY)
					{
						for (LONG nX = nX0; nX < nX1; ++nX, ++n)
						{
							if ((pB[n]^pC[n])&ECompareMask)
							{
								if (pC[n] == 0)
								{
									bContinue = false;
									break; // cannot delete pixels with this method
								}
								if (nBgrX0 > nX) nBgrX0 = nX;
								if (nBgrY0 > nY) nBgrY0 = nY;
								if (nBgrX1 <= nX) nBgrX1 = nX+1;
								if (nBgrY1 <= nY) nBgrY1 = nY+1;
								++nBgr;
							}
						}
						n += c->nSizeX - (nX1-nX0);
					}
					if (bContinue)
					{
						if ((LONG((c->nX1-c->nX0)*(c->nY1-c->nY0)) > (nBgrX1-nBgrX0)*(nBgrY1-nBgrY0) && nTrX0 < nTrX1) ||
							(nChg > nBgr/*(nChgX1-nChgX0)*(nChgY1-nChgY0) > (nBgrX1-nBgrX0)*(nBgrY1-nBgrY0)*/ && nTrX0 >= nTrX1))
						{
							// TODO: compare pixel count?
							p->bDispose = DISPOSE_OVERWRITE_PREVIOUS;
							if (nBgrX0 >= nBgrX1)
							{
								// frames are the same - reduce to 1 pixel and keep the background
								c->nX0 = c->nY0 = 0;
								c->nX1 = c->nY1 = 1;
							}
							else
							{
								c->nX0 = nBgrX0;
								c->nX1 = nBgrX1;
								c->nY0 = nBgrY0;
								c->nY1 = nBgrY1;
							}
							continue;
						}
					}
				}
				if (nTrX0 >= nTrX1)
				{
					// no new transparent pixels
					if (LONG((c->nX1-c->nX0)*(c->nY1-c->nY0)) > (nChgX1-nChgX0)*(nChgY1-nChgY0))
					{
						// changed region is smaller than native region
						// TODO: compare amount of information instead
						// TODO: make same pixels transparent to improve compression
						c->nX0 = nChgX0;
						c->nX1 = nChgX1;
						c->nY0 = nChgY0;
						c->nY1 = nChgY1;
					}
					p->bDispose = DISPOSE_KEEP;
					b = p;
				}
				else
				{
					// new transparent pixels, check (and expand) previous frame
					if (LONG(p->nX0) > nTrX0) p->nX0 = nTrX0;
					if (LONG(p->nX1) < nTrX1) p->nX1 = nTrX1;
					if (LONG(p->nY0) > nTrY0) p->nY0 = nTrY0;
					if (LONG(p->nY1) < nTrY1) p->nY1 = nTrY1;
					if (nChgX0 > LONG(p->nX0)) nChgX0 = p->nX0;
					if (nChgX1 < LONG(p->nX1)) nChgX1 = p->nX1;
					if (nChgY0 > LONG(p->nY0)) nChgY0 = p->nY0;
					if (nChgY1 < LONG(p->nY1)) nChgY1 = p->nY1;
					// TODO: if all area of previous frame is transparent in current frame, the current frame needs not cover it
					if (LONG(c->nX0) < nChgX0) c->nX0 = nChgX0;
					if (LONG(c->nX1) > nChgX1) c->nX1 = nChgX1;
					if (LONG(c->nY0) < nChgY0) c->nY0 = nChgY0;
					if (LONG(c->nY1) > nChgY1) c->nY1 = nChgY1;
					if (c->nX0 >= c->nX1 || c->nY0 >= c->nY1) // empty frame
					{
						c->nX0 = c->nY0 = 0;
						c->nX1 = c->nY1 = 1;
					}
					b = a_tEnd;
				}
			}
		}
	}
	template<typename TIterator>
	static void OptimizePixels(TIterator a_tBegin, TIterator a_tEnd) // delete duplicate pixels
	{
		if (a_tBegin == a_tEnd)
			return;
		--a_tEnd;
		TIterator p = a_tEnd;
		TIterator c = a_tEnd;
		for (--p; c != a_tBegin; --c, --p)
		{
			TIterator p2 = p;
			while (p2->bDispose == DISPOSE_OVERWRITE_PREVIOUS) --p2;
			if (p2->bDispose != DISPOSE_KEEP)
				continue;
			DWORD const* pP = reinterpret_cast<DWORD const*>(p2->pData.m_p);
			DWORD* pC = reinterpret_cast<DWORD*>(c->pData.m_p);
			ULONG n = c->nY0*c->nSizeX+c->nX0;
			for (ULONG nY = c->nY0; nY < c->nY1; ++nY)
			{
				for (ULONG nX = c->nX0; nX < c->nX1; ++nX, ++n)
				{
					if (((pP[n]^pC[n])&ECompareMask) == 0)
					{
						pC[n] = 0;
					}
				}
				n += c->nSizeX - (c->nX1-c->nX0);
			}
		}
	}

	ULONG nSizeX;
	ULONG nSizeY;
	bool bAlpha;
	CAutoVectorPtr<BYTE> pData;
	ULONG nDelay;
	ULONG nX0;
	ULONG nY0;
	ULONG nX1;
	ULONG nY1;
	BYTE bDispose;
	CImageOcTree<8> cOcTree;
	ULONG nOcTree;
	double fLocAvgDiff;
	CAutoVectorPtr<RGBQUAD> pLocColors;
	ULONG nLocColors;
	LONG nLocTransparent;
	double fGlbAvgDiff;
	bool bUseLocal;
};

double GetAvgDiff(BYTE const* a_pData, ULONG const a_nSizeX, ULONG const a_nSizeY, ULONG const a_nLineDiff, CImageOcTree<8> const& a_cOcTree, RGBQUAD const* a_pPal)
{
	double dSum = 0.0;
	ULONG nCnt = 0;
	for (ULONG y = 0; y < a_nSizeY; ++y)
	{
		for (BYTE const* const pEnd = a_pData+(a_nSizeX<<2); a_pData != pEnd; a_pData+=4)
		{
			if (a_pData[3])
			{
				RGBQUAD const* p = a_pPal+a_cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(a_pData));
				dSum += sqrt(double((ULONG(p->rgbRed)-ULONG(a_pData[0]))*(ULONG(p->rgbRed)-ULONG(a_pData[0])) +
					(ULONG(p->rgbGreen)-ULONG(a_pData[1]))*(ULONG(p->rgbGreen)-ULONG(a_pData[1])) +
					(ULONG(p->rgbBlue)-ULONG(a_pData[2]))*(ULONG(p->rgbBlue)-ULONG(a_pData[2]))));
				++nCnt;
			}
		}
		a_pData += a_nLineDiff;
	}
	return nCnt ? dSum/nCnt : 0;
}

STDMETHODIMP CDocumentEncoderGIF::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CConfigValue cMaxColors;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_MAXCOLORS), &cMaxColors);
		CConfigValue cInterlacing;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_INTERLACING), &cInterlacing);
		CConfigValue cDithering;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_DITHERING), &cDithering);

		CReadLock<IDocument> cLock(a_pDoc);

		CComPtr<IDocumentAnimation> pA;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pA));
		if (pA == NULL)
		{
			CComPtr<IDocumentImage> pI;
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
			return SaveSingleFrameGIF(pI, cMaxColors, cInterlacing, cDithering, a_pDst);
		}
		CComPtr<IEnumUnknowns> pFrames;
		pA->FramesEnum(&pFrames);
		ULONG nFrames = 0;
		if (pFrames) pFrames->Size(&nFrames);
		if (nFrames == 0)
			return E_FAIL; // no frames...
		if (nFrames == 1)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(0, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame));
			CComPtr<IDocument> pDoc;
			pA->FrameGetDoc(pFrame, &pDoc);
			CComPtr<IDocumentImage> pImg;
			if (pDoc) pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImg));
			if (pImg == NULL) return E_FAIL;
			return SaveSingleFrameGIF(pImg, cMaxColors, cInterlacing, cDithering, a_pDst);
		}
		ULONG nLoopCount = 0;
		pA->LoopCountGet(&nLoopCount);
		CAutoVectorPtr<SGIFFrame> aGIFFrames(new SGIFFrame[nFrames]);
		// get animation size (bounding rectangle)
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			CComPtr<IDocument> pFrameDoc;
			if (pFrame) pA->FrameGetDoc(pFrame, &pFrameDoc);
			CComPtr<IDocumentImage> pFrameImg;
			if (pFrameDoc) pFrameDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pFrameImg));
			TImageSize tSize = {0, 0};
			if (pFrameImg) pFrameImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
			if (tSize.nX > nSizeX) nSizeX = tSize.nX;
			if (tSize.nY > nSizeY) nSizeY = tSize.nY;
		}
		if (nSizeX*nSizeY == 0)
			return E_FAIL;
		// read individual frames
		for (ULONG i = 0; i < nFrames; ++i)
		{
			CComPtr<IUnknown> pFrame;
			pFrames->Get(i, &pFrame);
			SGIFFrame& sF = aGIFFrames.m_p[i];
			if (FAILED(sF.Init(pA, pFrame, nSizeX, nSizeY)))
				return E_FAIL;
		}

		// find rectangles that change
		SGIFFrame::OptimizeRectangles(aGIFFrames.m_p, aGIFFrames.m_p+nFrames);
		// delete same pixels
		SGIFFrame::OptimizePixels(aGIFFrames.m_p, aGIFFrames.m_p+nFrames);

		// compute palettes
		RGBQUAD aGlbColors[256];
		ZeroMemory(aGlbColors, sizeof aGlbColors);
		int nGlbColors = 256;
		bool bGlbTrans = false;
		bool bGlbForAll = false;
		bool bLocForAll = false;
		CImageOcTree<8> cOcTreeGlb;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			SGIFFrame& sF = aGIFFrames.m_p[i];
			sF.bAlpha = false;
			ULONG const nLineDiff = (sF.nSizeX-sF.nX1+sF.nX0)<<2;
			BYTE const* p = sF.pData.m_p+((sF.nX0+sF.nY0*sF.nSizeX)<<2);
			for (ULONG nY = sF.nY0; nY < sF.nY1; ++nY)
			{
				for (ULONG nX = sF.nX0; nX < sF.nX1; ++nX, p+=4)
				{
					if (p[3])
					{
						cOcTreeGlb.InsertPixel(*reinterpret_cast<TSimpleRGB const*>(p));
						sF.cOcTree.InsertPixel(*reinterpret_cast<TSimpleRGB const*>(p));
					}
					else
					{
						sF.bAlpha = bGlbTrans = true;
					}
				}
				p += nLineDiff;
			}
		}

		int const nClrs = cOcTreeGlb.ColorCount();
		if (bGlbTrans)
		{
			nGlbColors = min(nClrs+1, 256);
			cOcTreeGlb.GetPalette(aGlbColors+1, nGlbColors-1);
		}
		else
		{
			nGlbColors = min(nClrs, 256);
			cOcTreeGlb.GetPalette(aGlbColors, nGlbColors);
		}
		bGlbForAll = nGlbColors == nClrs;//nClrs < 128*nFrames;//true;//nGlbColors == nClrs;
		if (!bGlbForAll)
		{
			ULONG nLoc = 0;
			//std::map<double, size_t> cErrors;
			for (ULONG i = 0; i < nFrames; ++i)
			{
				// compute average pixel differences for frames with global palette
				SGIFFrame& sF = aGIFFrames.m_p[i];
				sF.nOcTree = sF.cOcTree.ColorCount();
				ULONG nClrs = sF.bAlpha ? sF.nOcTree+1 : sF.nOcTree;
				sF.nLocColors = nClrs <= 256 ? nClrs : 256;
				sF.pLocColors.Allocate(sF.nLocColors);
				sF.nLocTransparent = sF.bAlpha ? 0 : -1;
				sF.cOcTree.GetPalette(sF.pLocColors.m_p+(sF.bAlpha ? 1 : 0), sF.nLocColors-(sF.bAlpha ? 1 : 0));
				sF.fLocAvgDiff = nClrs <= 256 ? 0.0 : GetAvgDiff(sF.pData.m_p+((sF.nX0+sF.nY0*sF.nSizeX)<<2), sF.nX1-sF.nX0, sF.nY1-sF.nY0, (sF.nSizeX-sF.nX1+sF.nX0)<<2, sF.cOcTree, sF.pLocColors.m_p+(sF.bAlpha ? 1 : 0));
				sF.fGlbAvgDiff = nClrs == 0 ? 0.0 : GetAvgDiff(sF.pData.m_p+((sF.nX0+sF.nY0*sF.nSizeX)<<2), sF.nX1-sF.nX0, sF.nY1-sF.nY0, (sF.nSizeX-sF.nX1+sF.nX0)<<2, cOcTreeGlb, aGlbColors+(bGlbTrans ? 1 : 0));

				ULONG nLocPalSize = 256;
				for (ULONG n = nClrs<<1; nLocPalSize > n; nLocPalSize>>=1);
				LONG nSaved = 3*(nLocPalSize-(256/nFrames));
				double dLoss = sF.fGlbAvgDiff-sF.fLocAvgDiff;
				double dFact = (768+nSaved)/1536;
				sF.bUseLocal = dLoss < 12.0*dFact && dLoss < sF.fLocAvgDiff*(1.0+0.5*dFact);
				if (sF.bUseLocal)
					++nLoc;
			}
			bGlbForAll = nLoc == 0;
			bLocForAll = nLoc >= (nFrames-1);
		}
		if (!bGlbForAll && !bLocForAll)
		{
			// re-comupte global palette - use only frames that will be using the global palette
			cOcTreeGlb.Reset();
			bGlbTrans = false;
			for (ULONG i = 0; i < nFrames; ++i)
			{
				SGIFFrame& sF = aGIFFrames.m_p[i];
				if (sF.bUseLocal)
					continue;
				ULONG const nLineDiff = (sF.nSizeX-sF.nX1+sF.nX0)<<2;
				BYTE const* p = sF.pData.m_p+((sF.nX0+sF.nY0*sF.nSizeX)<<2);
				for (ULONG nY = sF.nY0; nY < sF.nY1; ++nY)
				{
					for (ULONG nX = sF.nX0; nX < sF.nX1; ++nX, p+=4)
						if (p[3])
							cOcTreeGlb.InsertPixel(*reinterpret_cast<TSimpleRGB const*>(p));
						else
							bGlbTrans = true;
					p += nLineDiff;
				}
			}
			int const nClrs = cOcTreeGlb.ColorCount();
			if (bGlbTrans)
			{
				nGlbColors = min(nClrs+1, 256);
				cOcTreeGlb.GetPalette(aGlbColors+1, nGlbColors-1);
			}
			else
			{
				nGlbColors = min(nClrs, 256);
				cOcTreeGlb.GetPalette(aGlbColors, nGlbColors);
			}
		}

		CCachingDstStream<> cWriter(a_pDst);
		CAutoVectorPtr<BYTE> aColorIndices(new BYTE[nSizeX * nSizeY]);
		BYTE tmp;
		for (ULONG i = 0; i < nFrames; ++i)
		{
			SGIFFrame& sF = aGIFFrames.m_p[i];

			RGBQUAD const* aColors;
			int nColors = 256;
			int nTransparentIndex = -1;
			int nPalSize = 0;

			if (bGlbForAll || (!bLocForAll && !sF.bUseLocal))
			{
				if (cDithering)
				{ // dithering
					if (bGlbTrans)
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2)), aGlbColors+1, nGlbColors-1, sF.nX1-sF.nX0, sF.nY1-sF.nY0, sF.nSizeX<<2, aColorIndices.m_p, CSkipTransparent());
					else
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2)), aGlbColors, nGlbColors, sF.nX1-sF.nX0, sF.nY1-sF.nY0, sF.nSizeX<<2, aColorIndices.m_p, CIgnoreTransparency());
				}
				else
				{ // nearest color
					int nLineDiff = (sF.nSizeX-(sF.nX1-sF.nX0))<<2;
					BYTE const* pSrc = sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2);
					BYTE* pDst = aColorIndices.m_p;
					for (ULONG y = sF.nY0; y < sF.nY1; ++y)
					{
						for (ULONG x = sF.nX0; x < sF.nX1; ++x, pSrc += 4, ++pDst)
						{
							if (bGlbTrans)
							{
								if (pSrc[3])
									*pDst = 1+static_cast<BYTE>(cOcTreeGlb.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
								else
									*pDst = 0;
							}
							else
							{
								*pDst = static_cast<BYTE>(cOcTreeGlb.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
							}
						}
						pSrc += nLineDiff;
					}
				}
				nTransparentIndex = bGlbTrans ? 0 : -1;
				nPalSize = nColors = nGlbColors;
				aColors = aGlbColors;
			}
			else
			{
				bool bTransparent = sF.nLocTransparent != -1;
				if (cDithering)
				{ // dithering
					if (bTransparent)
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2)), sF.pLocColors+1, sF.nLocColors-1, sF.nX1-sF.nX0, sF.nY1-sF.nY0, sF.nSizeX<<2, aColorIndices.m_p, CSkipTransparent());
					else
						DirectToIndexed(reinterpret_cast<TSimpleRGBA const*>(sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2)), sF.pLocColors, sF.nLocColors, sF.nX1-sF.nX0, sF.nY1-sF.nY0, sF.nSizeX<<2, aColorIndices.m_p, CIgnoreTransparency());
				}
				else
				{ // nearest color
					int nLineDiff = (sF.nSizeX-(sF.nX1-sF.nX0))<<2;
					BYTE const* pSrc = sF.pData+((sF.nX0+sF.nY0*sF.nSizeX)<<2);
					BYTE* pDst = aColorIndices.m_p;
					for (ULONG y = sF.nY0; y < sF.nY1; ++y)
					{
						for (ULONG x = sF.nX0; x < sF.nX1; ++x, pSrc += 4, ++pDst)
						{
							if (bTransparent)
							{
								if (pSrc[3])
									*pDst = 1+static_cast<BYTE>(sF.cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
								else
									*pDst = 0;
							}
							else
							{
								*pDst = static_cast<BYTE>(sF.cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
							}
						}
						pSrc += nLineDiff;
					}
				}
				nTransparentIndex = sF.nLocTransparent;
				nPalSize = nColors = sF.nLocColors;
				aColors = sF.pLocColors;
			}

			BYTE datadepth = 0;
			for (int i = 2; i < nPalSize; i <<= 1)
				++datadepth;

			if (i == 0)
			{
				BYTE datadepth2 = datadepth;
				if (!bLocForAll && !bGlbForAll)
				{
					datadepth2 = 0;
					for (int i = 2; i < nGlbColors; i <<= 1)
						++datadepth2;
				}
				cWriter.Write("GIF89a", 6);
				// screen descriptor
				cWriter.Write(&nSizeX, 2); // BIG endian assumed
				cWriter.Write(&nSizeY, 2); // BIG endian assumed
				tmp = 0xf0 | datadepth2;
				cWriter.Write(&tmp, 1);
				tmp = 0;
				cWriter.Write(&tmp, 1); // Background color = colortable index 0
				cWriter.Write(&tmp, 1); // Pixel aspect ratio - 0x00 is default

				if (!bLocForAll && !bGlbForAll)
				{
					int nPalSize = nGlbColors;
					for (int i = 0; i < nPalSize; ++i)
					{
						cWriter.Write(&aGlbColors[i].rgbRed, 1);
						cWriter.Write(&aGlbColors[i].rgbGreen, 1);
						cWriter.Write(&aGlbColors[i].rgbBlue, 1);
					}
					while (nPalSize < (2<<datadepth2))
					{
						static BYTE const black[3] = {0, 0, 0};
						cWriter.Write(black, 3);
						++nPalSize;
					}
				}
				else
				{
					for (int i = 0; i < nPalSize; ++i)
					{
						cWriter.Write(&aColors[i].rgbRed, 1);
						cWriter.Write(&aColors[i].rgbGreen, 1);
						cWriter.Write(&aColors[i].rgbBlue, 1);
					}
					while (nPalSize < (2<<datadepth2))
					{
						static BYTE const black[3] = {0, 0, 0};
						cWriter.Write(black, 3);
						++nPalSize;
					}
				}
			}

			tmp = '!';  cWriter.Write(&tmp, 1); // Extension Introducer
			tmp = 0xf9; cWriter.Write(&tmp, 1); // Graphic Control Label
			tmp = 0x04; cWriter.Write(&tmp, 1); // block size
			tmp = (nTransparentIndex != -1 ? 0x01 : 0x00)|(sF.bDispose<<2); cWriter.Write(&tmp, 1); // flags (transparent, dispose)
			cWriter.Write(&sF.nDelay, 2); // delay - BIG endian assumed
			tmp = nTransparentIndex != -1 ? nTransparentIndex : 0; cWriter.Write(&tmp, 1); // transparent index
			tmp = 0x00; cWriter.Write(&tmp, 1); // block terminator
			if (i == 0 && nLoopCount != 1)
			{
				cWriter.Write("!\xff\x0bNETSCAPE2.0\x03\x01", 16);
				cWriter.Write(&nLoopCount, 2);
				static BYTE const b = 0;
				cWriter.Write(&b, 1);
			}
			tmp = ',';
			cWriter.Write(&tmp, 1);
			tmp = 0;
			cWriter.Write(&sF.nX0, 2); // "Image Left" - BIG endian assumed
			cWriter.Write(&sF.nY0, 2); // "Image Top" - BIG endian assumed
			ULONG nFrmX = sF.nX1-sF.nX0;
			cWriter.Write(&nFrmX, 2); // BIG endian assumed
			ULONG nFrmY = sF.nY1-sF.nY0;
			cWriter.Write(&nFrmY, 2); // BIG endian assumed

			bool const bLocPal = !bGlbForAll && ((i && bLocForAll) || (!bLocForAll && sF.bUseLocal));
			tmp = (cInterlacing ? 0x40 : 0) | (bLocPal ? 0x80 : 0) | datadepth;
			cWriter.Write(&tmp, 1);
			if (bLocPal)
			{
				for (int i = 0; i < nPalSize; ++i)
				{
					cWriter.Write(&aColors[i].rgbRed, 1);
					cWriter.Write(&aColors[i].rgbGreen, 1);
					cWriter.Write(&aColors[i].rgbBlue, 1);
				}
				while (nPalSize < (2<<datadepth))
				{
					static BYTE const black[3] = {0, 0, 0};
					cWriter.Write(black, 3);
					++nPalSize;
				}
			}

			// "Raster Data"
			gifcompressor<CCachingDstStream<> > gc(cWriter);
			gc.writedatablocks(aColorIndices, nFrmX*nFrmY, nFrmX, datadepth+1, cInterlacing);
		}
		tmp = ';';
		cWriter.Write(&tmp, 1); // GIF terminator
		cWriter.Flush();
		return cWriter.m_bError ? E_FAIL : S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

