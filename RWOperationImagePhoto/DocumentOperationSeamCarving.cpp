// DocumentOperationSeamCarving.cpp : Implementation of CDocumentOperationSeamCarving

#include "stdafx.h"
#include "DocumentOperationSeamCarving.h"
#include <SharedStringTable.h>
#include <RWDocumentImageRaster.h>


const OLECHAR CFGID_SC_SIZEX[] = L"SizeX";
const OLECHAR CFGID_SC_SIZEY[] = L"SizeY";
const OLECHAR CFGID_SC_SELMODE[] = L"Mode";
const LONG CFGVAL_SCSM_IGNORE = 0;
const LONG CFGVAL_SCSM_PROTECT = 1;
const LONG CFGVAL_SCSM_REMOVE = -1;
const OLECHAR CFGID_SC_SELSYCID[] = L"SyncID";


#include <ConfigCustomGUIImpl.h>


class ATL_NO_VTABLE CConfigGUISeamCarving :
	public CCustomConfigWndImpl<CConfigGUISeamCarving>,
	public CDialogResize<CConfigGUISeamCarving>
{
public:
	CConfigGUISeamCarving()
	{
	}

	enum { IDD = IDD_CONFIGGUI_SEAMCARVING };

	BEGIN_MSG_MAP(CConfigGUISeamCarving)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUISeamCarving>)
		CHAIN_MSG_MAP(CCustomConfigWndImpl<CConfigGUISeamCarving>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CConfigGUISeamCarving)
		DLGRESIZE_CONTROL(IDC_CGSC_SELMODE, DLSZ_DIVSIZE_X(2))
		DLGRESIZE_CONTROL(IDC_CGSC_SELID, DLSZ_DIVMOVE_X(2)|DLSZ_DIVSIZE_X(2))
	END_DLGRESIZE_MAP()

	BEGIN_CONFIGITEM_MAP(CConfigGUISeamCarving)
		CONFIGITEM_EDITBOX(IDC_CGSC_SIZEX, CFGID_SC_SIZEX)
		CONFIGITEM_EDITBOX(IDC_CGSC_SIZEY, CFGID_SC_SIZEY)
		CONFIGITEM_COMBOBOX(IDC_CGSC_SELMODE, CFGID_SC_SELMODE)
		CONFIGITEM_EDITBOX(IDC_CGSC_SELID, CFGID_SC_SELSYCID)
	END_CONFIGITEM_MAP()

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};


// CDocumentOperationSeamCarving

STDMETHODIMP CDocumentOperationSeamCarving::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGESEAMCARVING_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSeamCarving::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SC_SIZEX), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SIZEX_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SIZEX_DESC), CConfigValue(800L), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SC_SIZEY), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SIZEY_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SIZEY_DESC), CConfigValue(600L), NULL, 0, NULL);
		CComBSTR cCFGID_SC_SELMODE(CFGID_SC_SELMODE);
		pCfgInit->ItemIns1ofN(cCFGID_SC_SELMODE, _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SELMODE_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SELMODE_DESC), CConfigValue(CFGVAL_SCSM_IGNORE), NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SC_SELMODE, CConfigValue(CFGVAL_SCSM_IGNORE), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SCSM_IGNORE), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SC_SELMODE, CConfigValue(CFGVAL_SCSM_PROTECT), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SCSM_PROTECT), 0, NULL);
		pCfgInit->ItemOptionAdd(cCFGID_SC_SELMODE, CConfigValue(CFGVAL_SCSM_REMOVE), _SharedStringTable.GetStringAuto(IDS_CFGVAL_SCSM_REMOVE), 0, NULL);
		TConfigOptionCondition tCond;
		tCond.bstrID = cCFGID_SC_SELMODE;
		tCond.eConditionType = ECOCNotEqual;
		tCond.tValue = CConfigValue(CFGVAL_SCSM_IGNORE);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_SC_SELSYCID), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SELSYCID_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_SC_SELSYCID_DESC), CConfigValue(L"IMAGEMASK"), NULL, 1, &tCond);

		CConfigCustomGUI<&CLSID_DocumentOperationSeamCarving, CConfigGUISeamCarving>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationSeamCarving::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentRasterImage), __uuidof(IDocumentEditableImage)};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void Retarget(LONG a_nSizeX1, LONG a_nSizeY1, LONG a_nSizeX2, LONG a_nSizeY2, LONG a_nStride, TPixelChannel* a_pData, BYTE* a_pMask, int a_nMaskFactor);

STDMETHODIMP CDocumentOperationSeamCarving::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentRasterImage> pRI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI == NULL)
			return E_FAIL;

		TImageSize tSize = {1, 1};
		pRI->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		if (tSize.nX <= 2 || tSize.nY <= 2)
			return S_FALSE;

		CConfigValue cVal;
		// TODO: support insertion
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SC_SELMODE), &cVal);
		LONG const eMode = cVal;
		CAutoVectorPtr<BYTE> cMask;
		if (eMode != CFGVAL_SCSM_IGNORE)
		{
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SC_SELSYCID), &cVal);
			CComPtr<ISharedStateImageSelection> pSel;
			a_pStates->StateGet(cVal, __uuidof(ISharedStateImageSelection), reinterpret_cast<void**>(&pSel));
			if (pSel)
			{
				LONG nX = 0;
				LONG nY = 0;
				ULONG nDX = tSize.nX;
				ULONG nDY = tSize.nY;
				if (SUCCEEDED(pSel->Bounds(&nX, &nY, &nDX, &nDY)))
				{
					if (nX <= 0 && nY <= 0 && nDX >= tSize.nX && nDY >= tSize.nY && S_OK == pSel->IsEmpty())
					{
						// everything is selected
					}
					else
					{
						cMask.Allocate(tSize.nX*tSize.nY);
						pSel->GetTile(0, 0, tSize.nX, tSize.nY, tSize.nX, cMask.m_p);
					}
				}
			}
		}

		a_pConfig->ItemValueGet(CComBSTR(CFGID_SC_SIZEX), &cVal);
		LONG const nSizeX = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_SC_SIZEY), &cVal);
		LONG const nSizeY = cVal;

		if (nSizeX <= 2 || nSizeY <= 2)
			return S_FALSE;

		LONG nBufX = max(LONG(tSize.nX), nSizeX);
		LONG nBufY = max(LONG(tSize.nY), nSizeY);

		CAutoVectorPtr<TPixelChannel> pData(new TPixelChannel[nBufX*nBufY]);
		CWriteLock<IDocument> cLock(a_pDocument);
		TImageStride const tStride = {1, nBufX};
		HRESULT hRes = pRI->TileGet(EICIRGBA, NULL, &tSize, &tStride, tStride.nY*tSize.nY, pData, NULL, EIRIAccurate);
		if (FAILED(hRes)) return hRes;
		Retarget(tSize.nX, tSize.nY, nSizeX, nSizeY, nBufX, pData, cMask, eMode);
		TImageSize const tSize2 = {nSizeX, nSizeY};
		hRes = pRI->CanvasSet(&tSize2, NULL, NULL, NULL);
		if (FAILED(hRes)) return hRes;
		return pRI->TileSet(EICIRGBA, NULL, &tSize2, &tStride, tSize.nX*tSize.nY, pData, TRUE);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

void Retarget(LONG a_nSizeX1, LONG a_nSizeY1, LONG a_nSizeX2, LONG a_nSizeY2, LONG a_nStride, TPixelChannel* a_pData, BYTE* a_pMask, int a_nMaskFactor)
{
	LONG const nBufX = max(a_nSizeX1, a_nSizeX2);
	LONG const nBufY = max(a_nSizeY1, a_nSizeY2);
	int const nInsertDelta = 20;
	CAutoVectorPtr<int> pEnergy(new int[nBufX*nBufY]);

	// calculate energy
	{
		TPixelChannel* pSourcePixel = a_pData;
		int* pEnergyDataDest = pEnergy;
		for (LONG iY = 0; iY < a_nSizeY1; ++iY)
		{
			for (LONG iX = 0; iX < a_nSizeX1; ++iX)
			{
				LONG iDiffTotal = 0;
				LONG iDiffCount = 0;
				LONG const nYMax = min((iY+4), a_nSizeY1);
				for (LONG iYSample = iY < 3 ? 0 : (iY-3); iYSample < nYMax; ++iYSample)
				{
					LONG const nXMax = min((iX+4), a_nSizeX1);
					for(LONG iXSample = iX < 3 ? 0 : (iX-3); iXSample < nXMax; ++iXSample)
					{
						iDiffTotal += abs(int(pSourcePixel->bR) - int(a_pData[iYSample*a_nStride + iXSample].bR));
						iDiffTotal += abs(int(pSourcePixel->bG) - int(a_pData[iYSample*a_nStride + iXSample].bG));
						iDiffTotal += abs(int(pSourcePixel->bB) - int(a_pData[iYSample*a_nStride + iXSample].bB));
						iDiffTotal += abs(int(pSourcePixel->bA) - int(a_pData[iYSample*a_nStride + iXSample].bA));
						++iDiffCount;
					}
				}
				*pEnergyDataDest = ((iDiffTotal + (iDiffCount>>1))<<1) / iDiffCount;
				if (a_pMask)
				{
					*pEnergyDataDest -= a_nMaskFactor*(0xff-*a_pMask);
					++a_pMask;
				}

				++pSourcePixel;
				++pEnergyDataDest;
			}
			pSourcePixel += a_nStride-a_nSizeX1;
			pEnergyDataDest += nBufX-a_nSizeX1;
		}
	}

	while (a_nSizeX2 != a_nSizeX1 || a_nSizeY2 != a_nSizeY1)
	{
		enum ECarveAction { ECARemoveColumn, ECARemoveRow, ECAInsertColumn, ECAInsertRow } eAction;
		if (a_nSizeX2 < a_nSizeX1)
		{
			if (a_nSizeY2 < a_nSizeY1)
			{
				eAction = a_nSizeX2*a_nSizeY1 < a_nSizeY2*a_nSizeX1 ? ECARemoveColumn : ECARemoveRow;
			}
			else
			{
				eAction = ECARemoveColumn;
			}
		}
		else
		{
			if (a_nSizeY2 < a_nSizeY1)
			{
				eAction = ECARemoveRow;
			}
			else
			{
				eAction = a_nSizeX2*a_nSizeY1 < a_nSizeY2*a_nSizeX1 ? ECAInsertRow : ECAInsertColumn;
			}
		}
		if (eAction == ECARemoveColumn || eAction == ECAInsertColumn)
		{
			CAutoVectorPtr<int> pEnergySumX;
			CAutoVectorPtr<char> pPathX;
			if (pEnergySumX.m_p == NULL)
			{
				if (!pEnergySumX.Allocate(nBufX*nBufY) ||
					!pPathX.Allocate(nBufX*nBufY))
					return;// E_FAIL;

				int const* pEnergyDataDest = pEnergy+nBufX;
				int* pSum = pEnergySumX;
				char* pDir = pPathX;
				CopyMemory(pSum, pEnergy.m_p, a_nSizeX1*sizeof*pSum);
				ZeroMemory(pDir, a_nSizeX1);
				pSum += nBufX;
				pDir += nBufX;
				for (LONG iY = 1; iY < a_nSizeY1; ++iY)
				{
					if (pSum[-nBufX] <= pSum[1-nBufX])
					{
						*pSum = pSum[-nBufX]+*pEnergyDataDest;
						*pDir = 0;
					}
					else
					{
						*pSum = pSum[1-nBufX]+*pEnergyDataDest;
						*pDir = 1;
					}
					++pSum;
					++pDir;
					++pEnergyDataDest;
					for (LONG iX = 1; iX < a_nSizeX1-1; ++iX)
					{
						if (pSum[-nBufX] <= pSum[1-nBufX] && pSum[-nBufX] <= pSum[-1-nBufX])
						{
							*pSum = pSum[-nBufX]+*pEnergyDataDest;
							*pDir = 0;
						}
						else if (pSum[1-nBufX] <= pSum[-1-nBufX])
						{
							*pSum = pSum[1-nBufX]+*pEnergyDataDest;
							*pDir = 1;
						}
						else
						{
							*pSum = pSum[-1-nBufX]+*pEnergyDataDest;
							*pDir = -1;
						}
						++pSum;
						++pDir;
						++pEnergyDataDest;
					}
					if (pSum[-nBufX] <= pSum[-1-nBufX])
					{
						*pSum = pSum[-nBufX]+*pEnergyDataDest;
						*pDir = 0;
					}
					else
					{
						*pSum = pSum[-1-nBufX]+*pEnergyDataDest;
						*pDir = -1;
					}
					++pSum;
					++pDir;
					++pEnergyDataDest;
					pSum += nBufX-a_nSizeX1;
					pDir += nBufX-a_nSizeX1;
					pEnergyDataDest += nBufX-a_nSizeX1;
				}
			}

			// find minimum
			int const* pSum = pEnergySumX.m_p+(a_nSizeY1-1)*nBufX;
			int nMinSum = *pSum;
			int nMinIndex = 0;
			int const* const pSumEnd = pSum+a_nSizeX1;
			++pSum;
			while (pSum != pSumEnd)
			{
				if (*pSum < nMinSum)
				{
					nMinSum = *pSum;
					nMinIndex = pSum-(pEnergySumX.m_p+(a_nSizeY1-1)*nBufX);
				}
				++pSum;
			}

			if (eAction == ECARemoveColumn)
			{
				// remove vertical seam, update pEnergy
				char* pDir = pPathX;
				for (LONG iY = a_nSizeY1-1; iY >= 0; --iY)
				{
					TPixelChannel* p = a_pData+a_nStride*iY+nMinIndex;
					int* pE = pEnergy.m_p+nBufX*iY+nMinIndex;
					for (int i = a_nSizeX1-nMinIndex-1; i > 0; --i, ++p, ++pE)
					{
						p[0] = p[1];
						pE[0] = pE[1];
					}
					nMinIndex += pDir[nBufX*iY+nMinIndex];
				}

				--a_nSizeX1;
			}
			else // insert column
			{
				// duplicate vertical seam, update pEnergy
				char* pDir = pPathX;
				for (LONG iY = a_nSizeY1-1; iY >= 0; --iY)
				{
					TPixelChannel* p = a_pData+a_nStride*iY+a_nSizeX1-1;
					int* pE = pEnergy.m_p+nBufX*iY+a_nSizeX1-1;
					for (int i = a_nSizeX1-nMinIndex; i > 0; --i, --p, --pE)
					{
						p[1] = p[0];
						pE[1] = pE[0];
					}
					pE[2] = (pE[1] += max(pE[1], nInsertDelta));
					nMinIndex += pDir[nBufX*iY+nMinIndex];
				}

				++a_nSizeX1;
			}
		}
		else // process row
		{
			CAutoVectorPtr<int> pEnergySumY;
			CAutoVectorPtr<char> pPathY;
			if (pEnergySumY.m_p == NULL)
			{
				if (!pEnergySumY.Allocate(nBufX*nBufY) ||
					!pPathY.Allocate(nBufX*nBufY))
					return;// E_FAIL;

				int const* pEnergyDataDest = pEnergy+1;
				int* pSum = pEnergySumY;
				char* pDir = pPathY;
				for (LONG i = 0; i < a_nSizeY1; ++i)
				{
					pSum[i*nBufX] = pEnergy[i*nBufX];
					pDir[i*nBufX] = 0;
				}
				++pSum;
				++pDir;
				for (LONG iX = 1; iX < a_nSizeX1; ++iX)
				{
					if (pSum[-1] <= pSum[nBufX-1])
					{
						*pSum = pSum[-1]+*pEnergyDataDest;
						*pDir = 0;
					}
					else
					{
						*pSum = pSum[nBufX-1]+*pEnergyDataDest;
						*pDir = 1;
					}
					pSum += nBufX;
					pDir += nBufX;
					pEnergyDataDest += nBufX;
					for (LONG iY = 1; iY < a_nSizeY1-1; ++iY)
					{
						if (pSum[-1] <= pSum[nBufX-1] && pSum[-1] <= pSum[-nBufX-1])
						{
							*pSum = pSum[-1]+*pEnergyDataDest;
							*pDir = 0;
						}
						else if (pSum[nBufX-1] <= pSum[-nBufX-1])
						{
							*pSum = pSum[nBufX-1]+*pEnergyDataDest;
							*pDir = 1;
						}
						else
						{
							*pSum = pSum[-nBufX-1]+*pEnergyDataDest;
							*pDir = -1;
						}
						pSum += nBufX;
						pDir += nBufX;
						pEnergyDataDest += nBufX;
					}
					if (pSum[-1] <= pSum[-nBufX-1])
					{
						*pSum = pSum[-1]+*pEnergyDataDest;
						*pDir = 0;
					}
					else
					{
						*pSum = pSum[-nBufX-1]+*pEnergyDataDest;
						*pDir = -1;
					}
					pSum += nBufX;
					pDir += nBufX;
					pEnergyDataDest += nBufX;
					pSum += 1-a_nSizeY1*nBufX;
					pDir += 1-a_nSizeY1*nBufX;
					pEnergyDataDest += 1-a_nSizeY1*nBufX;
				}
			}

			// find minimum
			int const* pSum = pEnergySumY.m_p+a_nSizeX1-1;
			int nMinSum = *pSum;
			int nMinIndex = 0;
			pSum += nBufX;
			for (LONG i = 1; i < a_nSizeY1; ++i)
			{
				if (*pSum < nMinSum)
				{
					nMinSum = *pSum;
					nMinIndex = i;
				}
				pSum += nBufX;
			}

			if (eAction == ECARemoveRow)
			{
				// remove horizontal seam, update pEnergy
				char* pDir = pPathY;
				for (LONG iX = a_nSizeX1-1; iX >= 0; --iX)
				{
					TPixelChannel* p = a_pData+iX+a_nStride*nMinIndex;
					int* pE = pEnergy.m_p+iX+nBufX*nMinIndex;
					for (int i = a_nSizeY1-nMinIndex-1; i > 0; --i, p+=a_nStride, pE+=nBufX)
					{
						p[0] = p[a_nStride];
						pE[0] = pE[nBufX];
					}
					nMinIndex += pDir[iX+nBufX*nMinIndex];
				}

				--a_nSizeY1;
			}
			else
			{
				// insert horizontal seam, update pEnergy
				char* pDir = pPathY;
				for (LONG iX = a_nSizeX1-1; iX >= 0; --iX)
				{
					TPixelChannel* p = a_pData+iX+a_nStride*(a_nSizeY1-1);
					int* pE = pEnergy.m_p+iX+nBufX*(a_nSizeY1-1);
					for (int i = a_nSizeY1-nMinIndex; i > 0; --i, p-=a_nStride, pE-=nBufX)
					{
						p[a_nStride] = p[0];
						pE[nBufX] = pE[0];
					}
					pE[nBufX<<1] = (pE[nBufX] += max(pE[nBufX], nInsertDelta));
					nMinIndex += pDir[iX+nBufX*nMinIndex];
				}

				++a_nSizeY1;
			}
		}
	}
}
