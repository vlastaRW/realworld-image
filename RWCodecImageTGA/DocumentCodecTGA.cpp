// DocumentEncoderTGA.cpp : Implementation of CDocumentEncoderTGA

#include "stdafx.h"
#include "DocumentCodecTGA.h"

#include <MultiLanguageString.h>


#pragma pack(push)
#pragma pack(1)
struct STGAHeader
{
   BYTE  idlength;
   BYTE  colourmaptype;
   BYTE  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   BYTE  colourmapdepth;
   short int x_origin;
   short int y_origin;
   unsigned short width;
   unsigned short height;
   BYTE  bitsperpixel;
   BYTE  imagedescriptor;
};
#pragma pack(pop)
enum ETGAType
{
	TGA_TYPE_MAPPED = 1,
	TGA_TYPE_COLOR = 2,
	TGA_TYPE_GRAY = 3
};
enum ETGACompression
{
	TGA_COMP_NONE = 0,
	TGA_COMP_RLE =1
};

// CDocumentEncoderTGA

static OLECHAR const CFGID_TGA_COMPRESSION[] = L"TGAComp";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIEncoderTGADlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIEncoderTGADlg>,
	public CDialogResize<CConfigGUIEncoderTGADlg>
{
public:
	enum { IDC_CGTGA_COMPRESSION = 100 };

	BEGIN_DIALOG_EX(0, 0, 100, 12, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Compression:[0405]Komprese:"), IDC_STATIC, 0, 2, 48, 8, WS_VISIBLE, 0)
		CONTROL_COMBOBOX(IDC_CGTGA_COMPRESSION, 50, 0, 50, 200, WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIEncoderTGADlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIEncoderTGADlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIEncoderTGADlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUIEncoderTGADlg)
		DLGRESIZE_CONTROL(IDC_CGTGA_COMPRESSION, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUIEncoderTGADlg)
		CONFIGITEM_COMBOBOX(IDC_CGTGA_COMPRESSION, CFGID_TGA_COMPRESSION)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}

};

STDMETHODIMP CDocumentEncoderTGA::DefaultConfig(IConfig** a_ppDefCfg)
{
	try
	{
		*a_ppDefCfg = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR cCFGID_TGA_COMPRESSION(CFGID_TGA_COMPRESSION);
		pCfgInit->ItemIns1ofN(cCFGID_TGA_COMPRESSION, CMultiLanguageString::GetAuto(L"[0409]Compression[0405]Komprese"), CMultiLanguageString::GetAuto(L"[0409]Compression[0405]Komprese"), CConfigValue(1L), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TGA_COMPRESSION, CConfigValue(0L), CMultiLanguageString::GetAuto(L"[0409]None[0405]Žádná"), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_TGA_COMPRESSION, CConfigValue(1L), CMultiLanguageString::GetAuto(L"[0409]RLE compression[0405]RLE komprese"), 0, NULL);

		CConfigCustomGUI<&CLSID_DocumentEncoderTGA, CConfigGUIEncoderTGADlg>::FinalizeConfig(pCfgInit);

		*a_ppDefCfg = pCfgInit.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppDefCfg == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentEncoderTGA::CanSerialize(IDocument* a_pDoc, BSTR* a_pbstrAspects)
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

STDMETHODIMP CDocumentEncoderTGA::Serialize(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* UNREF(a_pLocation), ITaskControl* UNREF(a_pControl))
{
	try
	{
		CConfigValue cCompression;
		a_pCfg->ItemValueGet(CComBSTR(CFGID_TGA_COMPRESSION), &cCompression);
		bool const bRLE = cCompression.operator LONG() == 1L;

		CComPtr<IDocumentImage> pI;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
		CBGRABuffer cBuffer;
		if (!cBuffer.Init(pI))
			return E_FAIL;

		if (!cBuffer.bAlpha)
			cBuffer.ToBGR();

		BYTE const* pBuffer = cBuffer.aData;
		ULONG const nSizeX = cBuffer.tSize.nX;
		ULONG const nSizeY = cBuffer.tSize.nY;
		ULONG const nStride = cBuffer.nStride;

		HRESULT hRes = S_OK;

		try
		{
			std::vector<BYTE> cOut;
			cOut.reserve(1024);
			STGAHeader sHead;
			sHead.idlength = 0;
			sHead.colourmaptype = 0;
			sHead.datatypecode = bRLE ? 10 : 2; // grayscale is 11 : 3
			sHead.colourmaporigin = 0;
			sHead.colourmaplength = 0;
			sHead.colourmapdepth = 0;
			sHead.x_origin = 0;
			sHead.y_origin = 0;//m_cImageFormat->atDims[1].nItems;
			sHead.width = nSizeX;
			sHead.height = nSizeY;
			sHead.bitsperpixel = cBuffer.bAlpha ? 32 : 24;
			sHead.imagedescriptor = cBuffer.bAlpha ? 8 : 0;//0x28 : 0x20;
			std::copy(reinterpret_cast<BYTE const*>(&sHead), reinterpret_cast<BYTE const*>((&sHead)+1), std::back_inserter(cOut));
			int const nBytesPP = cBuffer.bAlpha ? 4 : 3;
			int const nData = nBytesPP*nSizeX;
			for (ULONG y = 0; y < nSizeY; ++y)
			{
				BYTE const* p = pBuffer+(nSizeY-y-1)*nStride;
				if (bRLE)
				{
					int repeat = 0;
					int direct = 0;
					BYTE const* from = p;
					for (ULONG x = 1; x < nSizeX; ++x)
					{
						if (memcmp(p, p+nBytesPP, nBytesPP))
						{
							// next pixel is different
							if (repeat)
							{
								cOut.push_back(128+repeat);
								for (int i = 0; i < nBytesPP; ++i)
									cOut.push_back(from[i]);
								from = p + nBytesPP; // point to first different pixel
								repeat = 0;
								direct = 0;
							}
							else
							{
								++direct;
							}
						}
						else
						{
							// next pixel is the same
							if (direct)
							{
								cOut.push_back(direct - 1);
								for (int i = 0; i < nBytesPP*direct; ++i)
									cOut.push_back(from[i]);
								from = p; // point to first identical pixel
								direct = 0;
								repeat = 1;
							}
							else
							{
								++repeat;
							}
						}
						if (repeat == 128)
						{
							cOut.push_back(255);
							for (int i = 0; i < nBytesPP; ++i)
								cOut.push_back(from[i]);
							from = p + nBytesPP;
							direct = 0;
							repeat = 0;
						}
						else if (direct == 127)
						{
							cOut.push_back(direct - 1);
							for (int i = 0; i < nBytesPP*direct; ++i)
								cOut.push_back(from[i]);
							from = p + nBytesPP;
							direct = 0;
							repeat = 0;
						}

						p += nBytesPP;
					}

					if (repeat > 0)
					{
						cOut.push_back(128 + repeat);
						for (int i = 0; i < nBytesPP; ++i)
							cOut.push_back(from[i]);
					}
					else
					{
						cOut.push_back(direct);
						for (int i = 0; i < nBytesPP*(direct+1); ++i)
							cOut.push_back(from[i]);
					}
				}
				else
				{
					std::copy(p, p+nData, std::back_inserter(cOut));
				}
			}
			// write footer
			for (int i = 0; i < 8; ++i)
				cOut.push_back(0);
			for (char const* psz = "TRUEVISION-XFILE."; *psz; ++psz)
				cOut.push_back(*psz);
			cOut.push_back(0);

			hRes = a_pDst->Write(cOut.size(), &(cOut[0]));
		}
		catch (...)
		{
			hRes = E_UNEXPECTED;
		}

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

static bool rle_read(BYTE const*& pAct, BYTE const* pEnd, ULONG nPixelSize, ULONG nPixels, BYTE* a_pOut, LONG a_nBufStep, int& repeat, int& direct, BYTE* sample)
{
	for (ULONG x = 0; x < nPixels; ++x)
	{
		if (repeat == 0 && direct == 0)
		{
			if (pAct >= pEnd)
				return false;
			int head = *(pAct++);

			if (head >= 128)
            {
				repeat = head - 127;

				if (pAct+nPixelSize > pEnd)
					return false;
				for (ULONG i = 0; i < nPixelSize; ++i)
					sample[i] = pAct[i];
				pAct += nPixelSize;
			}
			else
			{
				direct = head + 1;
			}
		}

		if (repeat > 0)
		{
			for (ULONG k = 0; k < nPixelSize; ++k)
				a_pOut[k] = sample[k];

			--repeat;
		}
		else // direct > 0
		{
			if (pAct+nPixelSize > pEnd)
				return false;
			for (ULONG i = 0; i < nPixelSize; ++i)
				a_pOut[i] = pAct[i];
				pAct += nPixelSize;

			--direct;
		}

		a_pOut += a_nBufStep;
    }

	return true;
}

HRESULT CDocumentEncoderTGA::Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryRasterImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* UNREF(a_pControl))
{
	static char const aFooter[] = "TRUEVISION-XFILE.";
	if (a_nLen < sizeof(STGAHeader)+sizeof aFooter)
		return E_RW_UNKNOWNINPUTFORMAT;

	STGAHeader const* pHead = reinterpret_cast<STGAHeader const*>(a_pData);

	bool bValidTGA = strcmp(aFooter, reinterpret_cast<char const*>(a_pData+a_nLen-sizeof aFooter)) == 0;
	if (!bValidTGA)
	{
		CComBSTR bstrName;
		if (a_pLocation) a_pLocation->ToText(NULL, &bstrName);
		ULONG nName = bstrName.Length();
		if (nName < 4 || _wcsicmp(bstrName.m_str+nName-4, L".tga"))
			return E_RW_UNKNOWNINPUTFORMAT; // either valid footer or .tga extension is required
	}
	// TODO: check TGA version

	ETGAType eImgType;
	ETGACompression eImgComp;
	switch (pHead->datatypecode)
    {
	case 1: eImgType = TGA_TYPE_MAPPED;	eImgComp = TGA_COMP_NONE; break;
	case 2: eImgType = TGA_TYPE_COLOR;	eImgComp = TGA_COMP_NONE; break;
    case 3: eImgType = TGA_TYPE_GRAY;	eImgComp = TGA_COMP_NONE; break;
    case 9: eImgType = TGA_TYPE_MAPPED;	eImgComp = TGA_COMP_RLE; break;
    case 10:eImgType = TGA_TYPE_COLOR;	eImgComp = TGA_COMP_RLE; break;
    case 11:eImgType = TGA_TYPE_GRAY;	eImgComp = TGA_COMP_RLE; break;
    default:
      return E_RW_UNKNOWNINPUTFORMAT;
    }

	TImageSize const tSize = {pHead->width, pHead->height};
	ULONG nBPP = pHead->bitsperpixel;
	ULONG nAlpha = pHead->imagedescriptor&0xf;

	// hack to handle some existing files with incorrect headers, see GIMP bug #306675
	if (nAlpha == nBPP)
		nAlpha = 0;

	// hack to handle yet another flavor of incorrect headers, see GIMP bug #540969
	if (nAlpha == 0)
	{
		if (eImgType == TGA_TYPE_COLOR && nBPP == 32)
			nAlpha = 8;

		if (eImgType == TGA_TYPE_GRAY && nBPP == 16)
			nAlpha = 8;
	}

	ULONG nBytesPP = (nBPP+7)>>3;

	switch (eImgType)
	{
	case TGA_TYPE_MAPPED:
        //if (nBPP != 8)
			return E_RW_UNKNOWNINPUTFORMAT;
		break;
	case TGA_TYPE_COLOR:
		if (nBPP != 15 && nBPP != 16 && nBPP != 24 && nBPP != 32)
			return E_RW_UNKNOWNINPUTFORMAT;
		break;
	case TGA_TYPE_GRAY:
		if (nBPP != 8 && (nAlpha != 8 || (nBPP != 16 && nBPP != 15)))
			return E_RW_UNKNOWNINPUTFORMAT;
		break;
	}

	// Plausible but unhandled formats
	if (nBytesPP * 8 != nBPP && nBPP != 15)
		return E_RW_UNSUPPORTEDINPUTFORMAT;

	// Check that we have a color map only when we need it.
	if (eImgType == TGA_TYPE_MAPPED && pHead->colourmaptype != 1)
		return E_RW_UNKNOWNINPUTFORMAT;
	else if (eImgType != TGA_TYPE_MAPPED && pHead->colourmaptype != 0)
		return E_RW_UNKNOWNINPUTFORMAT;

	bool bHorFlip = pHead->imagedescriptor&0x10;
	bool bVerFlip = pHead->imagedescriptor&0x20;

	int repeat = 0;
	int direct = 0;
	BYTE sample[4];

	BYTE const* pSrcData = a_pData+sizeof*pHead+pHead->idlength;
	if (eImgComp == TGA_COMP_NONE && pSrcData+tSize.nX*tSize.nY*nBytesPP > a_pData+a_nLen)
		throw E_FAIL;

	CAutoVectorPtr<TPixelChannel> pBuffer(new TPixelChannel[tSize.nX*tSize.nY]);
	CAutoVectorPtr<BYTE> pLine(new BYTE[tSize.nX*nBytesPP]);
	ULONG nXA = 0;
	ULONG nXRGB = 0;

	for (ULONG y = 0; y < tSize.nY; ++y)
	{
		BYTE* pPixelDst = bHorFlip ? pLine.m_p+nBytesPP*(tSize.nX-1) : pLine.m_p;
		LONG nPixelStep = bHorFlip ? -LONG(nBytesPP) : nBytesPP;
		if (eImgComp == TGA_COMP_NONE)
		{
			for (ULONG x = 0; x < tSize.nX; ++x)
			{
				for (ULONG i = 0; i < nBytesPP; ++i)
					pPixelDst[i] = pSrcData[i];
				pPixelDst += nPixelStep;
				pSrcData += nBytesPP;
			}
		}
		else
		{
			if (!rle_read(pSrcData, a_pData+a_nLen, nBytesPP, tSize.nX, pPixelDst, nPixelStep, repeat, direct, sample))
				throw E_FAIL;
		}
		TPixelChannel* pRowDst = pBuffer+(bVerFlip ? y : tSize.nY-y-1)*tSize.nX;
		BYTE const* pPixelSrc = pLine.m_p;
		if (eImgType == TGA_TYPE_GRAY)
		{
			bool const bAlpha = nBPP > 8 && nAlpha;
			switch (nBPP)
			{
			case 16:
			case 15:
			case 8:
				for (ULONG x = 0; x < tSize.nX; ++x, ++pRowDst, pPixelSrc+=nBytesPP)
				{
					pRowDst->bR = pRowDst->bG = pRowDst->bB = *pPixelSrc;
					pRowDst->bA = bAlpha ? pPixelSrc[1] : 0xff;
				}
				break;
			}
		}
		else
		{
			switch (nBPP)
			{
			case 16:
				if (nAlpha)
				{
					for (ULONG x = 0; x < tSize.nX; ++x, ++pRowDst, pPixelSrc+=nBytesPP)
					{
						BYTE const bB = (pPixelSrc[0]&0x1f)<<3;
						BYTE const bG = ((pPixelSrc[0]&0xe0)>>2)|((pPixelSrc[1]&0x03)<<6);
						BYTE const bR = (pPixelSrc[1]&0x7c)<<1;
						pRowDst->bR = bR | (bR>>5);
						pRowDst->bG = bG | (bG>>5);
						pRowDst->bB = bB | (bB>>5);
						nXRGB |= bR|bG|bB;
						pRowDst->bA = ((pPixelSrc[1]&0x80)>>7)*0xff;
						nXA |= pRowDst->bA;
					}
					break;
				}
			case 15:
				for (ULONG x = 0; x < tSize.nX; ++x, ++pRowDst, pPixelSrc+=nBytesPP)
				{
					BYTE const bB = (pPixelSrc[0]&0x1f)<<3;
					BYTE const bG = ((pPixelSrc[0]&0xe0)>>2)|((pPixelSrc[1]&0x03)<<6);
					BYTE const bR = (pPixelSrc[1]&0x7c)<<1;
					pRowDst->bR = bR | (bR>>5);
					pRowDst->bG = bG | (bG>>5);
					pRowDst->bB = bB | (bB>>5);
					pRowDst->bA = 0xff;
				}
				break;
			case 24:
				for (ULONG x = 0; x < tSize.nX; ++x, ++pRowDst, pPixelSrc+=nBytesPP)
				{
					pRowDst->bB = pPixelSrc[0];
					pRowDst->bG = pPixelSrc[1];
					pRowDst->bR = pPixelSrc[2];
					pRowDst->bA = 0xff;
				}
				break;
			case 32:
				{
					BYTE const bA = nAlpha ? 0 : 0xff;
					for (ULONG x = 0; x < tSize.nX; ++x, ++pRowDst, pPixelSrc+=nBytesPP)
					{
						pRowDst->bB = pPixelSrc[0];
						pRowDst->bG = pPixelSrc[1];
						pRowDst->bR = pPixelSrc[2];
						pRowDst->bA = pPixelSrc[3]|bA;
					}
				}
				break;
			}
		}
	}
	if (nXA == 0 && nXRGB)
	{
		// possibly a file RGBA file without a real A channel -> make A=0xff
		TPixelChannel* p = pBuffer;
		for (TPixelChannel* const pEnd = p+tSize.nX*tSize.nY; p != pEnd; ++p)
			p->bA = 0xff;
	}

	HRESULT hRes = a_pBuilder->Create(a_bstrPrefix, a_pBase, &tSize, NULL, 1, CChannelDefault(EICIRGBA), 0.0f, CImageTile(tSize.nX, tSize.nY, pBuffer));
	if (SUCCEEDED(hRes))
	{
		if (a_pEncoderID) *a_pEncoderID = CLSID_DocumentEncoderTGA;
		if (a_ppEncoderCfg)
		{
			DefaultConfig(a_ppEncoderCfg);
			if (eImgComp == TGA_COMP_NONE)
			{
				CComBSTR bstr(CFGID_TGA_COMPRESSION);
				(*a_ppEncoderCfg)->ItemValuesSet(1, &(bstr.m_str), CConfigValue(0L));
			}
		}
	}
	return hRes;
}

