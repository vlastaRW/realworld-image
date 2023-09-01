// MenuCommandsSwitchLayer.cpp : Implementation of CMenuCommandsSwitchLayer

#include "stdafx.h"
#include "MenuCommandsSwitchLayer.h"

#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include "ConfigGUILayerID.h"


// CMenuCommandsSwitchLayer

STDMETHODIMP CMenuCommandsSwitchLayer::NameGet(IMenuCommandsManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = new CMultiLanguageString(L"[0409]Layered Image - Switch Layer[0405]Vrstvený obrázek - přepnout vrstvu");
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CMenuCommandsSwitchLayer::ConfigCreate(IMenuCommandsManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	return CConfigGUILayerIDDlg::CreateConfig(a_ppDefaultConfig);
}

STDMETHODIMP CMenuCommandsSwitchLayer::CommandsEnum(IMenuCommandsManager* UNREF(a_pManager), IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
{
	try
	{
		*a_ppSubCommands = NULL;

		CComPtr<IDocumentLayeredImage> pDLI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI));
		if (pDLI == NULL)
			return E_FAIL;
		CComPtr<IStructuredItemsRichGUI> pRG;
		a_pDocument->QueryFeatureInterface(__uuidof(IStructuredItemsRichGUI), reinterpret_cast<void**>(&pRG));

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SELECTIONSYNC), &cVal);
		CComBSTR bstrID;
		pDLI->StatePrefix(&bstrID);
		if (bstrID.Length())
		{
			bstrID += cVal;
		}
		else
		{
			bstrID.Attach(cVal.Detach().bstrVal);
		}

		CComPtr<IEnumUnknownsInit> pItems;
		RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

		CComPtr<IEnumUnknowns> pLayers;
		pDLI->ItemsEnum(NULL, &pLayers);
		ULONG nLayers = 0;
		if (pLayers) pLayers->Size(&nLayers);
		for (ULONG i = 0; i < nLayers; ++i)
		{
			CComPtr<IComparable> pID;
			pLayers->Get(i, &pID);
			CComObject<CSwitchCommand>* p = NULL;
			CComObject<CSwitchCommand>::CreateInstance(&p);
			CComPtr<IDocumentMenuCommand> pTmp = p;
			p->Init(a_pStates, bstrID, pDLI, pID, pRG, i == 0);
			pItems->Insert(pTmp);
		}

		*a_ppSubCommands = pItems.Detach();
		return S_OK;
	}
	catch (...)
	{
		return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSwitchLayer::CSwitchCommand::Description(ILocalizedString** a_ppText)
{
	return E_NOTIMPL;
	//try
	//{
	//	*a_ppText = NULL;
	//	if (t_uIDDesc == 0)
	//		return E_NOTIMPL;
	//	*a_ppText = _SharedStringTable.GetString(t_uIDDesc);
	//	return S_OK;
	//}
	//catch (...)
	//{
	//	return a_ppText ? E_UNEXPECTED : E_POINTER;
	//}
}

STDMETHODIMP CMenuCommandsSwitchLayer::CSwitchCommand::IconID(GUID* a_pIconID)
{
	try
	{
		if (m_pRG == NULL)
			return E_NOTIMPL;
		*a_pIconID = GUID_NULL;
		DWORD* const pIDs = reinterpret_cast<DWORD*>(a_pIconID);
		pIDs[1] = reinterpret_cast<DWORD_PTR>(m_pLI.p);
		pIDs[2] = reinterpret_cast<DWORD_PTR>(m_pID.p);
		m_pRG->Thumbnail(m_pID, 0, 0, NULL, NULL, pIDs+3);
		return S_OK;
	}
	catch (...)
	{
		return a_pIconID ? E_UNEXPECTED : E_POINTER;
	}
}

STDMETHODIMP CMenuCommandsSwitchLayer::CSwitchCommand::Icon(ULONG a_nSize, HICON* a_phIcon)
{
	try
	{
		*a_phIcon = NULL;
		if (m_pRG == NULL)
			return E_NOTIMPL;

		ULONG const nSizeX = a_nSize;
		ULONG const nSizeY = a_nSize;
		ULONG const nMaskLineSize = (((nSizeX+7)>>3)+3)&~3;
		CAutoVectorPtr<BYTE> pIconRes(new BYTE[sizeof(BITMAPINFOHEADER)+nSizeX*nSizeY*4+nMaskLineSize*nSizeY]);
		BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pIconRes.m_p);
		pBIH->biSize = sizeof*pBIH;
		pBIH->biWidth = nSizeX;
		pBIH->biHeight = nSizeY<<1;
		pBIH->biPlanes = 1;
		pBIH->biBitCount = 32;
		pBIH->biCompression = BI_RGB;
		pBIH->biSizeImage = nSizeX*nSizeY*4+nMaskLineSize*nSizeY;
		pBIH->biXPelsPerMeter = 0x8000;
		pBIH->biYPelsPerMeter = 0x8000;
		pBIH->biClrUsed = 0;
		pBIH->biClrImportant = 0;
		DWORD* pXOR = reinterpret_cast<DWORD*>(pBIH+1);
		BYTE* pAND = reinterpret_cast<BYTE*>(pXOR+nSizeX*nSizeY);
		m_pRG->Thumbnail(m_pID, a_nSize, a_nSize, pXOR, NULL, NULL);
		for (ULONG y = 0; y < (nSizeY>>1); ++y)
		{
			DWORD* p1 = pXOR+y*nSizeX;
			DWORD* p2 = pXOR+(nSizeY-1-y)*nSizeX;
			for (ULONG x = 0; x < nSizeX; ++x, ++p1, ++p2)
				std::swap(*p1, *p2);
		}
		// create mask
		for (ULONG y = 0; y < nSizeY; ++y)
		{
			BYTE* pA = pAND+nMaskLineSize*y;
			DWORD* pC = pXOR+nSizeX*y;
			for (ULONG x = 0; x < nSizeX; ++x, ++pC)
			{
				BYTE* p = pA+(x>>3);
				if (*pC&0xff000000)
					*p &= ~(0x80 >> (x&7));
				else
					*p |= 0x80 >> (x&7);
			}
		}

		*a_phIcon = CreateIconFromResourceEx(pIconRes, sizeof(BITMAPINFOHEADER)+nSizeX*nSizeY*4+nMaskLineSize*nSizeY, TRUE, 0x00030000, nSizeX, nSizeY, LR_DEFAULTCOLOR);
		return (*a_phIcon) ? S_OK : E_FAIL;
	}
	catch (...)
	{
		return a_phIcon ? E_UNEXPECTED : E_POINTER;
	}
}
