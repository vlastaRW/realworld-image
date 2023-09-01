// DocumentEncoderBMP.cpp : Implementation of CDocumentEncoderBMP

#include "stdafx.h"
#include "DocumentEncoderBMP.h"

#include <MultiLanguageString.h>
#include <Win32LangEx.h>
#include <math.h>


// CDocumentEncoderBMP

STDMETHODIMP CDocumentEncoderBMP::DocumentType(IDocumentType** a_ppDocType)
{
	try
	{
		*a_ppDocType = NULL;
		*a_ppDocType = CDocumentTypeCreatorBMP::Create();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDocType ? E_UNEXPECTED : E_POINTER;
	}
}

#include "ConfigIDsBMPConvertor.h"
#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderBMPDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderBMPDlg>,
	public CDialogResize<CConfigGUIEncoderBMPDlg>
{
public:
	enum { IDC_CGBMP_FORMAT = 100, IDC_CGBMP_DITHER };

	BEGIN_DIALOG_EX(0, 0, 150, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Format:[0405]Formát:"), IDC_STATIC, 0, 2, 38, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGBMP_FORMAT, 40, 0, 50, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
		CONTROL_CHECKBOX(_T("[0409]Dithering[0405]Ditherování"), IDC_CGBMP_DITHER, 97, 1, 53, 10, WS_VISIBLE | WS_TABSTOP, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderBMPDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderBMPDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderBMPDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderBMPDlg)
		DLGRESIZE_CONTROL(IDC_CGBMP_FORMAT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_CGBMP_DITHER, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderBMPDlg)
		CONFIGITEM_COMBOBOX(IDC_CGBMP_FORMAT, CFGID_BMPFORMAT)
		CONFIGITEM_CHECKBOX(IDC_CGBMP_DITHER, CFGID_DITHERING)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

STDMETHODIMP CDocumentEncoderBMP::DefaultConfig(IConfig** a_ppDefCfg)
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

IConfig* CDocumentEncoderBMP::Config()
{
	CComPtr<IConfigWithDependencies> pCfgInit;
	RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

	CComBSTR cCFGID_BMPFORMAT(CFGID_BMPFORMAT);
	pCfgInit->ItemIns1ofN(cCFGID_BMPFORMAT, CMultiLanguageString::GetAuto(L"[0409]Format[0405]Formát"), CMultiLanguageString::GetAuto(L"[0409]Color channels, depth and compression.[0405]Barevná kanály, hloubka a komprese."), CConfigValue(CFGVAL_BMPFORMAT_32BIT), NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_32BIT), CMultiLanguageString::GetAuto(L"[0409]32-bits RGBA[0405]32-bitů RGBA"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_24BIT), CMultiLanguageString::GetAuto(L"[0409]24-bits RGB[0405]24-bitů RGB"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_8BIT), CMultiLanguageString::GetAuto(L"[0409]8-bits palette[0405]8-bitová paleta"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_8BITRLE), CMultiLanguageString::GetAuto(L"[0409]8-bits palette RLE compressed[0405]8-bitová paleta a RLE komprese"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_4BIT), CMultiLanguageString::GetAuto(L"[0409]4-bits palette[0405]4-bitová paleta"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_4BITRLE), CMultiLanguageString::GetAuto(L"[0409]4-bits palette RLE compressed[0405]4-bitová paleta a RLE komprese"), 0, NULL);
	pCfgInit->ItemOptionAdd(cCFGID_BMPFORMAT, CConfigValue(CFGVAL_BMPFORMAT_1BIT), CMultiLanguageString::GetAuto(L"[0409]1-bit palette[0405]1-bitová paleta"), 0, NULL);

	CComBSTR cCFGID_DITHERING(CFGID_DITHERING);
	TConfigOptionCondition tCondition;
	tCondition.bstrID = cCFGID_BMPFORMAT;
	tCondition.eConditionType = ECOCLess;
	tCondition.tValue = CConfigValue(CFGVAL_BMPFORMAT_24BIT);
	pCfgInit->ItemInsSimple(cCFGID_DITHERING, CMultiLanguageString::GetAuto(L"[0409]Dithering[0405]Ditherování"), CMultiLanguageString::GetAuto(L"[0409]Dithering method used when convering image to paletized format.[0405]Metoda převodu obrázku do nižší barevné hloubky."), CConfigValue(true), NULL, 1, &tCondition);

	CConfigCustomGUI<&CLSID_DocumentEncoderBMP, CConfigGUIEncoderBMPDlg>::FinalizeConfig(pCfgInit);

	return pCfgInit.Detach();
}

STDMETHODIMP CDocumentEncoderBMP::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA);
		return pDocImg ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

int static GetColors24(BYTE const* a_pRGB, int a_nSizeX, int a_nSizeY, int a_nLineSize, int a_nMaxColors, RGBQUAD* a_aColors, BYTE* a_pColorIndices)
{
	int nUsedColors = 0;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		BYTE const* pRGB = a_pRGB+(a_nSizeY-1-y)*a_nLineSize;
		for (int x = 0; x < a_nSizeX; ++x, pRGB += 3, ++a_pColorIndices)
		{
			int i;
			for (i = 0; i < nUsedColors; ++i)
			{
				if (a_aColors[i].rgbBlue == pRGB[0] && a_aColors[i].rgbGreen == pRGB[1] && a_aColors[i].rgbRed == pRGB[2])
				{
					*a_pColorIndices = i;
					break;
				}
			}
			if (i == nUsedColors)
			{
				if (nUsedColors < a_nMaxColors)
				{
					a_aColors[nUsedColors].rgbBlue = pRGB[0];
					a_aColors[nUsedColors].rgbGreen = pRGB[1];
					a_aColors[nUsedColors].rgbRed = pRGB[2];
					++nUsedColors;
					*a_pColorIndices = i;
				}
				else
				{
					return nUsedColors+1;
				}
			}
		}
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

template<int t_nBits = 8>
class CImageOcTree
{
public:
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
		if (m_sRoot.m_nPixels == 0)
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

				int const nMaxErr = 384<<FS_SHIFT;
				// prevent excessive color bleeding
				if (thisrerr[actX    ] > nMaxErr)
					thisrerr[actX    ] = nMaxErr;
				else if (thisrerr[actX    ] < -nMaxErr)
					thisrerr[actX    ] = -nMaxErr;
				if (thisgerr[actX    ] > nMaxErr)
					thisgerr[actX    ] = nMaxErr;
				else if (thisgerr[actX    ] < -nMaxErr)
					thisgerr[actX    ] = -nMaxErr;
				if (thisberr[actX    ] > nMaxErr)
					thisberr[actX    ] = nMaxErr;
				else if (thisberr[actX    ] < -nMaxErr)
					thisberr[actX    ] = -nMaxErr;

				if (nextrerr[actX + 2] > nMaxErr)
					nextrerr[actX + 2] = nMaxErr;
				else if (nextrerr[actX + 2] < -nMaxErr)
					nextrerr[actX + 2] = -nMaxErr;
				if (nextgerr[actX + 2] > nMaxErr)
					nextgerr[actX + 2] = nMaxErr;
				else if (nextgerr[actX + 2] < -nMaxErr)
					nextgerr[actX + 2] = -nMaxErr;
				if (nextberr[actX + 2] > nMaxErr)
					nextberr[actX + 2] = nMaxErr;
				else if (nextberr[actX + 2] < -nMaxErr)
					nextberr[actX + 2] = -nMaxErr;

				if (nextrerr[actX + 1] > nMaxErr)
					nextrerr[actX + 1] = nMaxErr;
				else if (nextrerr[actX + 1] < -nMaxErr)
					nextrerr[actX + 1] = -nMaxErr;
				if (nextgerr[actX + 1] > nMaxErr)
					nextgerr[actX + 1] = nMaxErr;
				else if (nextgerr[actX + 1] < -nMaxErr)
					nextgerr[actX + 1] = -nMaxErr;
				if (nextberr[actX + 1] > nMaxErr)
					nextberr[actX + 1] = nMaxErr;
				else if (nextberr[actX + 1] < -nMaxErr)
					nextberr[actX + 1] = -nMaxErr;

				if (nextrerr[actX    ] > nMaxErr)
					nextrerr[actX    ] = nMaxErr;
				else if (nextrerr[actX    ] < -nMaxErr)
					nextrerr[actX    ] = -nMaxErr;
				if (nextgerr[actX    ] > nMaxErr)
					nextgerr[actX    ] = nMaxErr;
				else if (nextgerr[actX    ] < -nMaxErr)
					nextgerr[actX    ] = -nMaxErr;
				if (nextberr[actX    ] > nMaxErr)
					nextberr[actX    ] = nMaxErr;
				else if (nextberr[actX    ] < -nMaxErr)
					nextberr[actX    ] = -nMaxErr;

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

static void RLE8Compress(BYTE const* a_pData, int a_nSizeX, int a_nSizeY, std::vector<BYTE>& a_cOut)
{
	BYTE aBuffer[256];
	int nInBuffer = 0;
	bool bSame = true;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < a_nSizeX; ++x, ++a_pData)
		{
			if (nInBuffer <= 1)
			{
				aBuffer[nInBuffer++] = *a_pData;
				bSame = aBuffer[0] == aBuffer[1];
				continue;
			}
			aBuffer[nInBuffer++] = *a_pData;
			if (bSame && (*a_pData != *aBuffer || nInBuffer == 255))
			{
				if (nInBuffer > 2)
				{
					if (nInBuffer == 255)
					{
						a_cOut.push_back(255);
						a_cOut.push_back(*aBuffer);
						nInBuffer = 0;
					}
					else
					{
						a_cOut.push_back(nInBuffer-1);
						a_cOut.push_back(*aBuffer);
						nInBuffer = 1;
						*aBuffer = *a_pData;
					}
				}
				else
				{
					bSame = false;
				}
			}
			if (!bSame && nInBuffer > 6)
			{
				if (nInBuffer >= 254)
				{
					a_cOut.push_back(0);
					a_cOut.push_back(254);
					std::copy(aBuffer, aBuffer+254, std::back_inserter(a_cOut));
					nInBuffer -= 254;
					*aBuffer = *a_pData;
				}
				else if (aBuffer[nInBuffer-1] == aBuffer[nInBuffer-2] &&
						 aBuffer[nInBuffer-1] == aBuffer[nInBuffer-3])
				{
					a_cOut.push_back(0);
					if (nInBuffer&1)
					{
						a_cOut.push_back(nInBuffer-3);
						std::copy(aBuffer, aBuffer+nInBuffer-3, std::back_inserter(a_cOut));
						aBuffer[0] = aBuffer[1] = aBuffer[2] = aBuffer[nInBuffer-1];
						nInBuffer = 3;
					}
					else
					{
						a_cOut.push_back(nInBuffer-2);
						std::copy(aBuffer, aBuffer+nInBuffer-2, std::back_inserter(a_cOut));
						aBuffer[0] = aBuffer[1] = aBuffer[nInBuffer-1];
						nInBuffer = 2;
					}
					bSame = true;
				}
			}
		}
		a_pData += ((a_nSizeX+3)&~3)-a_nSizeX;
		if (/*eol on each line*/true || y == a_nSizeY-1)
		{
			if (nInBuffer)
			{
				if (bSame)
				{
					a_cOut.push_back(nInBuffer);
					a_cOut.push_back(*aBuffer);
				}
				else if (nInBuffer == 2)
				{
					a_cOut.push_back(1);
					a_cOut.push_back(*aBuffer);
					a_cOut.push_back(1);
					a_cOut.push_back(aBuffer[1]);
				}
				else
				{
					a_cOut.push_back(0);
					a_cOut.push_back(nInBuffer);
					std::copy(aBuffer, aBuffer+nInBuffer, std::back_inserter(a_cOut));
					if (nInBuffer&1)
						a_cOut.push_back(0);
				}
				nInBuffer = 0;
			}
			a_cOut.push_back(0);
			a_cOut.push_back(y == a_nSizeY-1 ? 1 : 0); // 1=end of bitmap data
		}
	}
}

static void RLE4Compress(BYTE const* a_pData, int a_nSizeX, int a_nSizeY, std::vector<BYTE>& a_cOut)
{
	BYTE aBuffer[128];
	int nInBuffer = 0;
	bool bSame = true;
	for (int y = 0; y < a_nSizeY; ++y)
	{
		for (int x = 0; x < a_nSizeX; x+=2, ++a_pData)
		{
			if (nInBuffer <= 1)
			{
				aBuffer[nInBuffer++] = *a_pData;
				bSame = aBuffer[0] == aBuffer[1];
				continue;
			}
			aBuffer[nInBuffer++] = *a_pData;
			if (bSame && (*a_pData != *aBuffer || nInBuffer == 127))
			{
				if (nInBuffer > 2)
				{
					if (nInBuffer == 127)
					{
						a_cOut.push_back(x+1 == a_nSizeX ? 253 : 254);
						a_cOut.push_back(*aBuffer);
						nInBuffer = 0;
					}
					else
					{
						a_cOut.push_back((nInBuffer-1)<<1);
						a_cOut.push_back(*aBuffer);
						nInBuffer = 1;
						*aBuffer = *a_pData;
					}
				}
				else
				{
					bSame = false;
				}
			}
			if (!bSame && nInBuffer > 6)
			{
				if (nInBuffer >= 126)
				{
					a_cOut.push_back(0);
					a_cOut.push_back(x+1 == a_nSizeX ? 251 : 252);
					std::copy(aBuffer, aBuffer+126, std::back_inserter(a_cOut));
					nInBuffer -= 126;
					*aBuffer = *a_pData;
				}
				else if (aBuffer[nInBuffer-1] == aBuffer[nInBuffer-2] &&
						 aBuffer[nInBuffer-1] == aBuffer[nInBuffer-3])
				{
					a_cOut.push_back(0);
					if (nInBuffer&1)
					{
						a_cOut.push_back((nInBuffer-3)<<1);
						std::copy(aBuffer, aBuffer+nInBuffer-3, std::back_inserter(a_cOut));
						aBuffer[0] = aBuffer[1] = aBuffer[2] = aBuffer[nInBuffer-1];
						nInBuffer = 3;
					}
					else
					{
						a_cOut.push_back((nInBuffer-2)<<1);
						std::copy(aBuffer, aBuffer+nInBuffer-2, std::back_inserter(a_cOut));
						aBuffer[0] = aBuffer[1] = aBuffer[nInBuffer-1];
						nInBuffer = 2;
					}
					bSame = true;
				}
			}
		}
		a_pData += ((((a_nSizeX+1)>>1)+3)&~3)-((a_nSizeX+1)>>1);
		if (/*eol on each line*/true || y == a_nSizeY-1)
		{
			if (nInBuffer)
			{
				if (bSame)
				{
					a_cOut.push_back((nInBuffer<<1)-(a_nSizeX&1));
					a_cOut.push_back(*aBuffer);
				}
				else if (nInBuffer == 2)
				{
					a_cOut.push_back(0);
					a_cOut.push_back(4-(a_nSizeX&1));
					a_cOut.push_back(aBuffer[0]);
					a_cOut.push_back(aBuffer[1]);
				}
				else
				{
					a_cOut.push_back(0);
					a_cOut.push_back((nInBuffer<<1)-(a_nSizeX&1));
					std::copy(aBuffer, aBuffer+nInBuffer, std::back_inserter(a_cOut));
					if (nInBuffer&1)
						a_cOut.push_back(0);
				}
				nInBuffer = 0;
			}
			a_cOut.push_back(0);
			a_cOut.push_back(y == a_nSizeY-1 ? 1 : 0); // 1=end of bitmap data
		}
	}
}

HRESULT WriteBlock(IReturnedData* a_pDst, int a_nSizeX, int a_nSizeY, int a_nBitCount, int a_nCompression, BYTE const* a_pBegin, BYTE const* a_pEnd, int a_nColors, RGBQUAD const* a_pColors)
{
	std::vector<BYTE> cDst;
	if (a_nCompression == BI_RLE8)
	{
		RLE8Compress(a_pBegin, a_nSizeX, a_nSizeY, cDst);
		a_pBegin = &(cDst[0]);
		a_pEnd = &(cDst[cDst.size()-1])+1; // hacking
	}
	else if (a_nCompression == BI_RLE4)
	{
		RLE4Compress(a_pBegin, a_nSizeX, a_nSizeY, cDst);
		a_pBegin = &(cDst[0]);
		a_pEnd = &(cDst[cDst.size()-1])+1; // hacking
	}

	size_t const nTotalLen = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+a_nColors*sizeof(RGBQUAD)+(a_pEnd-a_pBegin);
	BITMAPFILEHEADER tFileHeader;
	ZeroMemory(&tFileHeader, sizeof tFileHeader);
	tFileHeader.bfType = 0x4d42;
	tFileHeader.bfSize = nTotalLen;
	tFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+a_nColors*sizeof(RGBQUAD);
	a_pDst->Write(sizeof tFileHeader, reinterpret_cast<BYTE const*>(&tFileHeader));
	BITMAPINFOHEADER tInfoHeader;
	ZeroMemory(&tInfoHeader, sizeof tInfoHeader);
	tInfoHeader.biBitCount = a_nBitCount;
	tInfoHeader.biCompression = a_nCompression;
	tInfoHeader.biPlanes = 1;
	tInfoHeader.biWidth = a_nSizeX;
	tInfoHeader.biHeight = a_nSizeY;
	tInfoHeader.biXPelsPerMeter = tInfoHeader.biYPelsPerMeter = 96*10000/254;
	tInfoHeader.biSizeImage = a_pEnd-a_pBegin;
	tInfoHeader.biClrUsed = tInfoHeader.biClrImportant = a_nColors;
	tInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	HRESULT hRes = a_pDst->Write(sizeof tInfoHeader, reinterpret_cast<BYTE const*>(&tInfoHeader));
	if (FAILED(hRes)) return hRes;
	hRes = a_pDst->Write(a_nColors*sizeof*a_pColors, reinterpret_cast<BYTE const*>(a_pColors));
	if (FAILED(hRes)) return hRes;
	hRes = a_pDst->Write(a_pEnd-a_pBegin, a_pBegin);
	return hRes;
}

class C4bitDst
{
public:
	class CAssign
	{
	public:
		CAssign(BYTE* a_pDst, int a_nShift) :
			m_pDst(a_pDst), m_nShift(a_nShift)
		{
		}
		void operator=(int a_nData)
		{
			*m_pDst = (*m_pDst & (0xf0>>m_nShift)) | ((a_nData&0x0f)<<m_nShift);
		}

	private:
		BYTE* m_pDst;
		int m_nShift;
	};

	C4bitDst(BYTE* a_pBuffer, ULONG a_nWidth, ULONG a_nLineLen) :
		m_pBuffer(a_pBuffer), m_nPos(0), m_nWidth(a_nWidth), m_nLineLen(a_nLineLen)
	{
	}
	CAssign operator*()
	{
		ULONG nY = m_nPos/m_nWidth;
		ULONG nX = m_nPos-nY*m_nWidth;
		return CAssign(m_pBuffer+nY*m_nLineLen+(nX>>1), ((~nX)&1) << 2);
	}
	void operator++()
	{
		++m_nPos;
	}
	void operator--()
	{
		--m_nPos;
	}
	void operator+=(int a_n)
	{
		m_nPos += a_n;
	}

private:
	BYTE* m_pBuffer;
	ULONG m_nPos;
	ULONG m_nWidth;
	ULONG m_nLineLen;
};

class C1bitDst
{
public:
	class CAssign
	{
	public:
		CAssign(BYTE* a_pDst, int a_nShift) :
			m_pDst(a_pDst), m_nShift(a_nShift)
		{
		}
		void operator=(int a_nData)
		{
			*m_pDst = (*m_pDst & ~(1<<m_nShift)) | ((a_nData&1)<<m_nShift);
		}

	private:
		BYTE* m_pDst;
		int m_nShift;
	};

	C1bitDst(BYTE* a_pBuffer, ULONG a_nWidth, ULONG a_nLineLen, ULONG a_nHeight) :
		m_pBuffer(a_pBuffer), m_nPos(0), m_nWidth(a_nWidth), m_nLineLen(a_nLineLen), m_nHeight(a_nHeight-1)
	{
	}
	CAssign operator*()
	{
		ULONG nY = m_nHeight-m_nPos/m_nWidth;
		ULONG nX = m_nPos%m_nWidth;
		return CAssign(m_pBuffer+nY*m_nLineLen+(nX>>3), (~nX)&7);
	}
	void operator++()
	{
		++m_nPos;
	}
	void operator--()
	{
		--m_nPos;
	}
	void operator+=(int a_n)
	{
		m_nPos += a_n;
	}

private:
	BYTE* m_pBuffer;
	ULONG m_nPos;
	ULONG m_nWidth;
	ULONG m_nLineLen;
	ULONG m_nHeight;
};

STDMETHODIMP CDocumentEncoderBMP::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	HRESULT hRes = S_OK;

	try
	{
		CConfigValue cFormat;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_BMPFORMAT), &cFormat);
		CConfigValue cDithering;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_DITHERING), &cDithering);

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI))
			return E_FAIL;

		ULONG const nSizeX = cBuffer.tSize.nX;
		ULONG const nSizeY = cBuffer.tSize.nY;
		ULONG const nPixelSize = cFormat.operator LONG() == CFGVAL_BMPFORMAT_32BIT ? 32 : 24;
		ULONG const nStride = (nSizeX*(nPixelSize>>3)+3)&0xfffffffc;

		BYTE const* pBuffer = cBuffer.aData;
		if (cFormat.operator LONG() != CFGVAL_BMPFORMAT_32BIT)
		{
			for (ULONG y = 0; y < nSizeY; ++y)
			{
				BYTE* pD = cBuffer.aData + y*nStride;
				BYTE const* pS = cBuffer.aData + y*cBuffer.nStride;
				for (ULONG x = 0; x < nSizeX; ++x, pS += 4, pD += 3)
				{
					pD[0] = pS[0];
					pD[1] = pS[1];
					pD[2] = pS[2];
				}
			}
		}

		switch (cFormat.operator LONG())
		{
		case CFGVAL_BMPFORMAT_32BIT:
		case CFGVAL_BMPFORMAT_24BIT:
			{
				ULONG const nRowLen = nStride;
				ULONG const nDataLen = nSizeY*nRowLen;
				ULONG const nTotalLen = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+nDataLen;
				BITMAPFILEHEADER tFileHeader;
				ZeroMemory(&tFileHeader, sizeof tFileHeader);
				tFileHeader.bfType = 0x4d42;
				tFileHeader.bfSize = nTotalLen;
				tFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
				a_pDst->Write(sizeof tFileHeader, reinterpret_cast<BYTE const*>(&tFileHeader));
				BITMAPINFOHEADER tInfoHeader;
				ZeroMemory(&tInfoHeader, sizeof tInfoHeader);
				tInfoHeader.biBitCount = nPixelSize;
				tInfoHeader.biCompression = BI_RGB;
				tInfoHeader.biPlanes = 1;
				tInfoHeader.biWidth = nSizeX;
				tInfoHeader.biHeight = nSizeY;
				tInfoHeader.biXPelsPerMeter = tInfoHeader.biYPelsPerMeter = 96*10000/254;
				tInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
				a_pDst->Write(sizeof tInfoHeader, reinterpret_cast<BYTE const*>(&tInfoHeader));
				for (ULONG i = 0; i < nSizeY; i++)
				{
					hRes = a_pDst->Write(nStride, pBuffer + (nSizeY-1-i)*nStride);
					if (FAILED(hRes)) break;
				}
			}
			break;
		case CFGVAL_BMPFORMAT_8BIT:
		case CFGVAL_BMPFORMAT_8BITRLE:
			{
				int const nColors = 256;
				RGBQUAD aColors[nColors];
				ZeroMemory(aColors, sizeof aColors);
				CAutoVectorPtr<BYTE> aColorIndices(new BYTE[((nSizeX+3)&~3) * nSizeY]);
				int nPalSize = GetColors24(pBuffer, nSizeX, nSizeY, nStride, nColors, aColors, aColorIndices);

				if (nPalSize <= nColors)
				{ // save image exactly
					if (((nSizeX+3)&~3) != nSizeX)
						for (int y = nSizeY-1; y > 0; --y)
							MoveMemory(aColorIndices.m_p+((nSizeX+3)&~3)*y, aColorIndices.m_p+nSizeX*y, nSizeX);
				}
				else
				{ // create palette
					nPalSize = nColors;
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

					if (cDithering.operator bool())
					{ // dithering
						DirectToIndexed(reinterpret_cast<TSimpleRGB const*>(pBuffer), aColors, nColors, nSizeX, nSizeY, nStride, aColorIndices.m_p, CIgnoreTransparency());
						CAutoVectorPtr<BYTE> aColorIndices2(new BYTE[((nSizeX+3)&~3) * nSizeY]);
						for (int y = nSizeY-1; y >= 0; --y)
							CopyMemory(aColorIndices2.m_p+((nSizeX+3)&~3)*(nSizeY-1-y), aColorIndices.m_p+nSizeX*y, nSizeX);
						std::swap(aColorIndices.m_p, aColorIndices2.m_p);
					}
					else
					{ // nearest color
						pSrc = pBuffer;
						for (int y = nSizeY-1; y >= 0; --y)
						{
							BYTE* pDst = aColorIndices.m_p+((nSizeX+3)&~3)*y;
							for (ULONG x = 0; x < nSizeX; ++x, pSrc += 3, ++pDst)
							{
								*pDst = static_cast<BYTE>(cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
							}
							pSrc += nLineDiff;
						}
					}
				}
				hRes = WriteBlock(a_pDst, nSizeX, nSizeY, 8, cFormat.operator LONG() == CFGVAL_BMPFORMAT_8BITRLE ? BI_RLE8 : BI_RGB, aColorIndices.m_p, aColorIndices.m_p+((nSizeX+3)&~3) * nSizeY, nPalSize, aColors);
			}
			break;
		case CFGVAL_BMPFORMAT_4BIT:
		case CFGVAL_BMPFORMAT_4BITRLE:
			{
				int const nColors = 16;
				RGBQUAD aColors[nColors];
				ZeroMemory(aColors, sizeof aColors);
				CAutoVectorPtr<BYTE> aColorIndices(new BYTE[nSizeX * nSizeY]);
				int nPalSize = GetColors24(pBuffer, nSizeX, nSizeY, nStride, nColors, aColors, aColorIndices);
				int nBytesPerLine = (((nSizeX+1)>>1)+3)&~3;

				if (nPalSize <= nColors)
				{ // save image exactly
					CAutoVectorPtr<BYTE> aColorIndices2(new BYTE[nBytesPerLine * nSizeY]);
					C4bitDst cDst(aColorIndices2.m_p, nSizeX, nBytesPerLine);
					for (ULONG y = 0; y < nSizeY; ++y)
					{
						BYTE const* pSrc = aColorIndices.m_p+y*nSizeX;
						for (ULONG x = 0; x < nSizeX; ++x, ++pSrc, ++cDst)
						{
							*cDst = *pSrc;
						}
					}
					std::swap(aColorIndices.m_p, aColorIndices2.m_p);
				}
				else
				{ // create palette
					nPalSize = nColors;
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

					C4bitDst cDst(aColorIndices.m_p, nSizeX, nBytesPerLine);
					if (cDithering.operator bool())
					{ // dithering
						DirectToIndexed(reinterpret_cast<TSimpleRGB const*>(pBuffer), aColors, nColors, nSizeX, nSizeY, nStride, cDst, CIgnoreTransparency());
						CAutoVectorPtr<BYTE> aColorIndices2(new BYTE[nBytesPerLine * nSizeY]);
						for (int y = nSizeY-1; y >= 0; --y)
							CopyMemory(aColorIndices2.m_p+nBytesPerLine*(nSizeY-1-y), aColorIndices.m_p+nBytesPerLine*y, nBytesPerLine);
						std::swap(aColorIndices.m_p, aColorIndices2.m_p);
					}
					else
					{ // nearest color
						for (int y = nSizeY-1; y >= 0; --y)
						{
							BYTE const* pSrc = pBuffer+y*nStride;
							for (ULONG x = 0; x < nSizeX; ++x, pSrc += 3, ++cDst)
							{
								*cDst = static_cast<BYTE>(cOcTree.FindColorIndex(*reinterpret_cast<TSimpleRGB const*>(pSrc)));
							}
						}
					}
				}
				hRes = WriteBlock(a_pDst, nSizeX, nSizeY, 4, cFormat.operator LONG() == CFGVAL_BMPFORMAT_4BITRLE ? BI_RLE4 : BI_RGB, aColorIndices.m_p, aColorIndices.m_p+nBytesPerLine*nSizeY, nPalSize, aColors);
			}
			break;
		case CFGVAL_BMPFORMAT_1BIT:
			if (cDithering.operator bool())
			{
				static RGBQUAD const s_aColors[2] =
				{
					{0, 0, 0, 0},
					{255, 255, 255, 0},
				};
				int nBytesPerLine = (((nSizeX+7)>>3)+3)&~3;
				CAutoVectorPtr<BYTE> aColorIndices(new BYTE[nBytesPerLine * nSizeY]);
				C1bitDst cDst(aColorIndices.m_p, nSizeX, nBytesPerLine, nSizeY);
				DirectToIndexed(reinterpret_cast<TSimpleRGB const*>(pBuffer), s_aColors, 2, nSizeX, nSizeY, nStride, cDst, CIgnoreTransparency());
				hRes = WriteBlock(a_pDst, nSizeX, nSizeY, 1, BI_RGB, aColorIndices.m_p, aColorIndices.m_p+nBytesPerLine*nSizeY, 2, s_aColors);
			}
			else
			{
				std::vector<DWORD> cData;
				for (ULONG y = 0; y < nSizeY; ++y)
				{
					BYTE const* pLine = pBuffer + (nSizeY-y-1)*nStride;
					for (ULONG x = 0; x < nSizeX; x+=32)
					{
						DWORD dw = 0;
						ULONG nBits = min(32, nSizeX-x);
						for (ULONG b = 0; b < nBits; ++b)
						{
							if ((ULONG(pLine[0])+pLine[1]+pLine[2]) >= 384)
							{
								dw |= 0x80000000>>b;
							}
							pLine += 3;
						}
						cData.push_back(((dw&0xff)<<24)|((dw&0xff00)<<8)|((dw&0xff0000)>>8)|((dw&0xff000000)>>24));
					}
				}
				size_t const nTotalLen = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+2*sizeof(RGBQUAD)+cData.size()*sizeof(DWORD);
				BITMAPFILEHEADER tFileHeader;
				ZeroMemory(&tFileHeader, sizeof tFileHeader);
				tFileHeader.bfType = 0x4d42;
				tFileHeader.bfSize = nTotalLen;
				tFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+2*sizeof(RGBQUAD);
				hRes = a_pDst->Write(sizeof tFileHeader, reinterpret_cast<BYTE const*>(&tFileHeader));
				if (FAILED(hRes)) break;
				BITMAPINFOHEADER tInfoHeader;
				ZeroMemory(&tInfoHeader, sizeof tInfoHeader);
				tInfoHeader.biBitCount = 1;
				tInfoHeader.biCompression = BI_RGB;
				tInfoHeader.biPlanes = 1;
				tInfoHeader.biWidth = nSizeX;
				tInfoHeader.biHeight = nSizeY;
				tInfoHeader.biXPelsPerMeter = tInfoHeader.biYPelsPerMeter = 96*10000/254;
				tInfoHeader.biSizeImage = cData.size()*sizeof(DWORD);
				tInfoHeader.biClrUsed = tInfoHeader.biClrImportant = 2;
				tInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
				hRes = a_pDst->Write(sizeof tInfoHeader, reinterpret_cast<BYTE const*>(&tInfoHeader));
				if (FAILED(hRes)) break;
				RGBQUAD aClrs[2];
				aClrs[0].rgbBlue = 0;
				aClrs[0].rgbGreen = 0;
				aClrs[0].rgbRed = 0;
				aClrs[0].rgbReserved = 0;
				aClrs[1].rgbBlue = 255;
				aClrs[1].rgbGreen = 255;
				aClrs[1].rgbRed = 255;
				aClrs[1].rgbReserved = 0;
				hRes = a_pDst->Write(sizeof aClrs, reinterpret_cast<BYTE const*>(aClrs));
				if (FAILED(hRes)) break;
				hRes = a_pDst->Write((cData.end()-cData.begin())<<2, reinterpret_cast<BYTE const*>(&(cData[0])));
			}
			break;
		default:
			hRes = E_FAIL;
		}
	}
	catch (...)
	{
		hRes = a_pDst == NULL ? E_POINTER : E_UNEXPECTED;
	}

	return hRes;
}

