// DocumentOperationRasterImagePerspective.cpp : Implementation of CDocumentOperationRasterImagePerspective

#include "stdafx.h"
#include "DocumentOperationRasterImagePerspective.h"
#include <SharedStringTable.h>

static OLECHAR const CFGID_TRANSFORM_MODE[] = L"Mode";
static LONG const CFGVAL_TM_QUAD = 0;
static LONG const CFGVAL_TM_AFFINE = 1;
static LONG const CFGVAL_TM_FILL = 2;

const OLECHAR CFGID_PT1X[] = L"Pt1X";
const OLECHAR CFGID_PT1Y[] = L"Pt1Y";
const OLECHAR CFGID_PT2X[] = L"Pt2X";
const OLECHAR CFGID_PT2Y[] = L"Pt2Y";
const OLECHAR CFGID_PT3X[] = L"Pt3X";
const OLECHAR CFGID_PT3Y[] = L"Pt3Y";
const OLECHAR CFGID_PT4X[] = L"Pt4X";
const OLECHAR CFGID_PT4Y[] = L"Pt4Y";

static OLECHAR const CFGID_TRANSFORM_SIZE[] = L"Size";
static OLECHAR const CFGID_TRANSFORM_ALIGN[] = L"Align";

static OLECHAR const CFGID_TRANSFORM_CENTER[] = L"Center";
static OLECHAR const CFGID_TRANSFORM_ANGLE[] = L"Angle";
static OLECHAR const CFGID_TRANSFORM_SCALE[] = L"Scale";
static OLECHAR const CFGID_TRANSFORM_SHEAR[] = L"Shear";
static OLECHAR const CFGID_TRANSFORM_TRANSLATION[] = L"Translation";

#include "ConfigGUIPerspective.h"

class ATL_NO_VTABLE CTransformationWrapper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ICanvasInteractingWrapper
{
public:
	void Init(ULONG a_nSizeX, ULONG a_nSizeY, LONG a_eMode, TPixelCoords const* a_pQuad)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;

		m_eMode = a_eMode;
		m_tQuad[0] = a_pQuad[0];
		m_tQuad[1] = a_pQuad[1];
		m_tQuad[2] = a_pQuad[2];
		m_tQuad[3] = a_pQuad[3];
	}
	void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;

		CConfigValue cMode;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_MODE), &cMode);
		m_eMode = cMode;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1X), &cVal);
		m_tQuad[0].fX = cVal.operator float()*a_nSizeX;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1Y), &cVal);
		m_tQuad[0].fY = cVal.operator float()*a_nSizeY;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2X), &cVal);
		m_tQuad[1].fX = cVal.operator float()*a_nSizeX;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2Y), &cVal);
		m_tQuad[1].fY = cVal.operator float()*a_nSizeY;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3X), &cVal);
		m_tQuad[2].fX = cVal.operator float()*a_nSizeX;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3Y), &cVal);
		m_tQuad[2].fY = cVal.operator float()*a_nSizeY;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4X), &cVal);
		m_tQuad[3].fX = cVal.operator float()*a_nSizeX;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4Y), &cVal);
		m_tQuad[3].fY = cVal.operator float()*a_nSizeY;
	}

BEGIN_COM_MAP(CTransformationWrapper)
	COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
END_COM_MAP()

	// ICanvasInteractingWrapper methods
public:
	STDMETHOD_(ULONG, GetControlPointCount)()
	{
		return 4;
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (a_nIndex < 4)
		{
			if (a_pPos)
				*a_pPos = m_tQuad[a_nIndex];
			if (a_pClass)
				*a_pClass = 1;
			return S_OK;
		}
		return E_RW_INDEXOUTOFRANGE;
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel)
	{
		if (a_ppNew == NULL)
			return E_POINTER;
		try
		{
			if (a_pNewSel)
				*a_pNewSel = a_nIndex;
			if (a_nIndex < 4)
			{
				if (a_pPos == NULL || (fabsf(a_pPos->fX-m_tQuad[a_nIndex].fX) < 0.01f && fabsf(a_pPos->fY-m_tQuad[a_nIndex].fY) < 0.01f))
				{
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				TPixelCoords tQuad[4] = {m_tQuad[0], m_tQuad[1], m_tQuad[2], m_tQuad[3]};
				tQuad[a_nIndex] = *a_pPos;
				CComObject<CTransformationWrapper>* p = NULL;
				CComObject<CTransformationWrapper>::CreateInstance(&p);
				CComPtr<ICanvasInteractingWrapper> pNew = p;
				p->Init(m_nSizeX, m_nSizeY, m_eMode, tQuad);
				*a_ppNew = pNew.Detach();
				return S_OK;
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		a_pLines->MoveTo(m_tQuad[0].fX, m_tQuad[0].fY);
		a_pLines->LineTo(m_tQuad[1].fX, m_tQuad[1].fY);
		a_pLines->LineTo(m_tQuad[2].fX, m_tQuad[2].fY);
		a_pLines->LineTo(m_tQuad[3].fX, m_tQuad[3].fY);
		a_pLines->LineTo(m_tQuad[0].fX, m_tQuad[0].fY);
		return S_OK;
	}

	STDMETHOD(ToConfig)(IConfig* a_pConfig)
	{
		CComBSTR cMode(CFGID_TRANSFORM_MODE);
		CComBSTR cPt1X(CFGID_PT1X);
		CComBSTR cPt1Y(CFGID_PT1Y);
		CComBSTR cPt2X(CFGID_PT2X);
		CComBSTR cPt2Y(CFGID_PT2Y);
		CComBSTR cPt3X(CFGID_PT3X);
		CComBSTR cPt3Y(CFGID_PT3Y);
		CComBSTR cPt4X(CFGID_PT4X);
		CComBSTR cPt4Y(CFGID_PT4Y);
		BSTR aIDs[9] = {cMode, cPt1X, cPt1Y, cPt2X, cPt2Y, cPt3X, cPt3Y, cPt4X, cPt4Y};
		TConfigValue aVals[9];
		aVals[0] = CConfigValue(m_eMode);
		aVals[1] = CConfigValue(m_tQuad[0].fX/m_nSizeX);
		aVals[2] = CConfigValue(m_tQuad[0].fY/m_nSizeY);
		aVals[3] = CConfigValue(m_tQuad[1].fX/m_nSizeX);
		aVals[4] = CConfigValue(m_tQuad[1].fY/m_nSizeY);
		aVals[5] = CConfigValue(m_tQuad[2].fX/m_nSizeX);
		aVals[6] = CConfigValue(m_tQuad[2].fY/m_nSizeY);
		aVals[7] = CConfigValue(m_tQuad[3].fX/m_nSizeX);
		aVals[8] = CConfigValue(m_tQuad[3].fY/m_nSizeY);
		return a_pConfig->ItemValuesSet(9, aIDs, aVals);
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	LONG m_eMode;
	TPixelCoords m_tQuad[4];
};

class ATL_NO_VTABLE CTransformationWrapperAffine :
	public CComObjectRootEx<CComMultiThreadModel>,
	public ICanvasInteractingWrapper
{
public:
	void Init(ULONG a_nSizeX, ULONG a_nSizeY, TVector2f a_tScale, float a_fRotation, TVector2f a_tTranslation)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;

		m_tScale = a_tScale;
		m_fRotation = a_fRotation;
		m_tTranslation = a_tTranslation;
	}
	void Init(ULONG a_nSizeX, ULONG a_nSizeY, IConfig* a_pConfig)
	{
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;

		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_ANGLE), &cVal);
		m_fRotation = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_SCALE), &cVal);
		m_tScale.x = cVal[0];
		m_tScale.y = cVal[1];
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_TRANSLATION), &cVal);
		m_tTranslation.x = cVal[0];
		m_tTranslation.y = cVal[1];
	}

BEGIN_COM_MAP(CTransformationWrapperAffine)
	COM_INTERFACE_ENTRY(ICanvasInteractingWrapper)
END_COM_MAP()

	// ICanvasInteractingWrapper methods
public:
	STDMETHOD_(ULONG, GetControlPointCount)()
	{
		return 10;
	}
	void SimpleToPts(TPixelCoords* a_aPixels) const
	{
		float const fCos = cosf(m_fRotation);
		float const fSin = sinf(m_fRotation);
		float const fXCos = m_nSizeX*0.5f*m_tScale.x*fCos;
		float const fXSin = m_nSizeX*0.5f*m_tScale.x*fSin;
		float const fYCos = m_nSizeY*0.5f*m_tScale.y*fCos;
		float const fYSin = m_nSizeY*0.5f*m_tScale.y*fSin;
		TPixelCoords tCenter = {m_nSizeX*0.5f+m_tTranslation.x, m_nSizeY*0.5f+m_tTranslation.y};
		a_aPixels[0].fX = tCenter.fX - fXCos + fYSin;
		a_aPixels[0].fY = tCenter.fY - fXSin - fYCos;
		a_aPixels[1].fX = tCenter.fX + fXCos + fYSin;
		a_aPixels[1].fY = tCenter.fY + fXSin - fYCos;
		a_aPixels[2].fX = tCenter.fX + fXCos - fYSin;
		a_aPixels[2].fY = tCenter.fY + fXSin + fYCos;
		a_aPixels[3].fX = tCenter.fX - fXCos - fYSin;
		a_aPixels[3].fY = tCenter.fY - fXSin + fYCos;
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		if (a_nIndex > 9)
			return E_RW_INDEXOUTOFRANGE;
		if (a_nIndex == 9)
		{
			a_pPos->fX = m_nSizeX*0.5f+m_tTranslation.x;
			a_pPos->fY = m_nSizeY*0.5f+m_tTranslation.y;
			*a_pClass = 0;
			return S_OK;
		}
		if (a_nIndex == 8)
		{
			a_pPos->fX = m_nSizeX*0.5f+m_tTranslation.x + 0.35f*m_nSizeX*m_tScale.x*cosf(m_fRotation);
			a_pPos->fY = m_nSizeY*0.5f+m_tTranslation.y + 0.35f*m_nSizeX*m_tScale.x*sinf(m_fRotation);
			*a_pClass = 2;
			return S_OK;
		}
		{
			TPixelCoords aPts[4];
			SimpleToPts(aPts);
			if (a_nIndex&1)
			{
				a_pPos->fX = 0.5f*(aPts[a_nIndex>>1].fX+aPts[((a_nIndex>>1)+1)&3].fX);
				a_pPos->fY = 0.5f*(aPts[a_nIndex>>1].fY+aPts[((a_nIndex>>1)+1)&3].fY);
				*a_pClass = 1;
			}
			else
			{
				a_pPos->fX = aPts[a_nIndex>>1].fX;
				a_pPos->fY = aPts[a_nIndex>>1].fY;
				*a_pClass = 0;
			}
			return S_OK;
		}
		return E_RW_INDEXOUTOFRANGE;
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize, ICanvasInteractingWrapper** a_ppNew, ULONG* a_pNewSel)
	{
		if (a_ppNew == NULL)
			return E_POINTER;
		try
		{
			if (a_pNewSel)
				*a_pNewSel = a_nIndex;
			if (a_nIndex >= 10)
				return E_RW_INDEXOUTOFRANGE;
			{
				float const fX = a_pPos->fX-(m_nSizeX*0.5f+m_tTranslation.x);
				float const fY = a_pPos->fY-(m_nSizeY*0.5f+m_tTranslation.y);
				if (a_nIndex == 9)
				{
					CComObject<CTransformationWrapperAffine>* p = NULL;
					CComObject<CTransformationWrapperAffine>::CreateInstance(&p);
					CComPtr<ICanvasInteractingWrapper> pNew = p;
					TVector2f tTranslation = {a_pPos->fX-(m_nSizeX*0.5f), a_pPos->fY-(m_nSizeY*0.5f)};
					p->Init(m_nSizeX, m_nSizeY, m_tScale, m_fRotation, tTranslation);
					*a_ppNew = pNew.Detach();
					return S_OK;
				}
				else if (a_nIndex == 8)
				{
					float fAngle = atan2(fY, fX);
					if (fabsf(fAngle-m_fRotation) > 1e-4f)
					{
						CComObject<CTransformationWrapperAffine>* p = NULL;
						CComObject<CTransformationWrapperAffine>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, m_tScale, fAngle, m_tTranslation);
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
				else
				{
					BYTE bChange = 0;
					// 0 1 2
					// 7   3
					// 6 5 4
					TPixelCoords tDirX = { cosf(m_fRotation),  sinf(m_fRotation)};
					TPixelCoords tDirY = {-sinf(m_fRotation),  cosf(m_fRotation)};
					TPixelCoords aPts[4];
					SimpleToPts(aPts);
					if (a_nIndex&1)
					{
						ULONG nPtL1 = ((a_nIndex-1)>>1)&3;
						ULONG nPtR1 = (nPtL1+1)&3;
						ULONG nPtL2 = (nPtL1-1)&3;
						ULONG nPtR2 = (nPtR1+1)&3;
						TPixelCoords tRef = {0.5f*(aPts[nPtL2].fX+aPts[nPtR2].fX), 0.5f*(aPts[nPtL2].fY+aPts[nPtR2].fY)};
						TPixelCoords tD = {a_pPos->fX-tRef.fX, a_pPos->fY-tRef.fY};
						TVector2f tScale = m_tScale;
						if (nPtR1&1)
						{
							float fD = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
							tD.fX = tDirY.fX*fD;
							tD.fY = tDirY.fY*fD;
							tScale.y = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY)/m_nSizeY;
						}
						else
						{
							float fD = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
							tD.fX = tDirX.fX*fD;
							tD.fY = tDirX.fY*fD;
							tScale.x = sqrtf(tD.fX*tD.fX + tD.fY*tD.fY)/m_nSizeY;
						}
						aPts[nPtL1].fX = aPts[nPtL2].fX + tD.fX;
						aPts[nPtL1].fY = aPts[nPtL2].fY + tD.fY;
						aPts[nPtR1].fX = aPts[nPtR2].fX + tD.fX;
						aPts[nPtR1].fY = aPts[nPtR2].fY + tD.fY;
						TVector2f tTranslation =
						{
							tRef.fX+tD.fX*0.5f-0.5f*m_nSizeX,
							tRef.fY+tD.fY*0.5f-0.5f*m_nSizeY,
						};
						CComObject<CTransformationWrapperAffine>* p = NULL;
						CComObject<CTransformationWrapperAffine>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, tScale, m_fRotation, tTranslation);
						*a_ppNew = pNew.Detach();
						return S_OK;
						//if (m_cData.bAspectLock)
						{
							//float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
							//if (nPtR1&1)
							//{
							//	m_fSizeX = m_fSizeY/fAspectLock;
							//	if (m_eCoordinatesMode == ECMIntegral)
							//	{
							//		aPts[nPtR1].fX = floorf(aPts[nPtL1].fX - tD.fY/fAspectLock+0.5f);
							//		aPts[nPtR1].fY = floorf(aPts[nPtL1].fY + tD.fX/fAspectLock+0.5f);
							//		aPts[nPtR2].fX = floorf(aPts[nPtL2].fX - tD.fY/fAspectLock+0.5f);
							//		aPts[nPtR2].fY = floorf(aPts[nPtL2].fY + tD.fX/fAspectLock+0.5f);
							//	}
							//	else
							//	{
							//		aPts[nPtR1].fX = aPts[nPtL1].fX - tD.fY/fAspectLock;
							//		aPts[nPtR1].fY = aPts[nPtL1].fY + tD.fX/fAspectLock;
							//		aPts[nPtR2].fX = aPts[nPtL2].fX - tD.fY/fAspectLock;
							//		aPts[nPtR2].fY = aPts[nPtL2].fY + tD.fX/fAspectLock;
							//	}
							//}
							//else
							//{
							//	m_fSizeY = m_fSizeX*fAspectLock;
							//	if (m_eCoordinatesMode == ECMIntegral)
							//	{
							//		aPts[nPtR1].fX = floorf(aPts[nPtL1].fX - tD.fY*fAspectLock+0.5f);
							//		aPts[nPtR1].fY = floorf(aPts[nPtL1].fY + tD.fX*fAspectLock+0.5f);
							//		aPts[nPtR2].fX = floorf(aPts[nPtL2].fX - tD.fY*fAspectLock+0.5f);
							//		aPts[nPtR2].fY = floorf(aPts[nPtL2].fY + tD.fX*fAspectLock+0.5f);
							//	}
							//	else
							//	{
							//		aPts[nPtR1].fX = aPts[nPtL1].fX - tD.fY*fAspectLock;
							//		aPts[nPtR1].fY = aPts[nPtL1].fY + tD.fX*fAspectLock;
							//		aPts[nPtR2].fX = aPts[nPtL2].fX - tD.fY*fAspectLock;
							//		aPts[nPtR2].fY = aPts[nPtL2].fY + tD.fX*fAspectLock;
							//	}
							//}
							//m_tCenter.fX = 0.25f*(aPts[0].fX + aPts[1].fX + aPts[2].fX + aPts[3].fX);
							//m_tCenter.fY = 0.25f*(aPts[0].fY + aPts[1].fY + aPts[2].fY + aPts[3].fY);
						}
					}
					else
					{
						ULONG nPt1 = a_nIndex>>1;
						ULONG nPt2 = (nPt1+2)&3;
						aPts[nPt1] = *a_pPos;
						TPixelCoords tD = {aPts[nPt2].fX-aPts[nPt1].fX, aPts[nPt2].fY-aPts[nPt1].fY};
						float fDX = tDirX.fX*tD.fX + tDirX.fY*tD.fY;
						float fDY = tDirY.fX*tD.fX + tDirY.fY*tD.fY;
						//if (m_cData.bAspectLock)
						//{
						//	float const fAspectLock = m_szSelSize.cy/float(m_szSelSize.cx);
						//	if (fabsf(fabsf(fDX)*fAspectLock-fabsf(fDY)) > 1e-4f)
						//	{
						//		float fSize = min(fabsf(fDX)*fAspectLock, fabsf(fDY));//sqrtf(tD.fX*tD.fX+tD.fY*tD.fY)/sqrtf(2.0f);
						//		if (m_eCoordinatesMode == ECMIntegral)
						//		{
						//			fDX = fDX < 0.0f ? floorf(-fSize/fAspectLock+0.5f) : floorf(fSize/fAspectLock+0.5f);
						//			fDY = fDY < 0.0f ? floorf(-fSize+0.5f) : floorf(fSize+0.5f);
						//		}
						//		else
						//		{
						//			fDX = fDX < 0.0f ? -fSize/fAspectLock : fSize/fAspectLock;
						//			fDY = fDY < 0.0f ? -fSize : fSize;
						//		}
						//		aPts[nPt1].fX = aPts[nPt2].fX - tDirX.fX*fDX - tDirY.fX*fDY;
						//		aPts[nPt1].fY = aPts[nPt2].fY - tDirX.fY*fDX - tDirY.fY*fDY;
						//	}
						//	//tD.fX = aPts[nPt2].fX-aPts[nPt1].fX;
						//	//tD.fY = aPts[nPt2].fY-aPts[nPt1].fY;
						//}
						TPixelCoords tDX = {tDirX.fX*fDX, tDirX.fY*fDX};
						TPixelCoords tDY = {tDirY.fX*fDY, tDirY.fY*fDY};
						aPts[nPt1^3].fX = aPts[nPt2].fX - tDX.fX;
						aPts[nPt1^3].fY = aPts[nPt2].fY - tDX.fY;
						aPts[nPt1^1].fX = aPts[nPt2].fX - tDY.fX;
						aPts[nPt1^1].fY = aPts[nPt2].fY - tDY.fY;
						TVector2f tTranslation =
						{
							0.5f*(aPts[nPt1].fX + aPts[nPt2].fX)-(m_nSizeX*0.5f),
							0.5f*(aPts[nPt1].fY + aPts[nPt2].fY)-(m_nSizeY*0.5f),
						};
						TVector2f tScale =
						{
							fabsf(fDX)/m_nSizeX,
							fabsf(fDY)/m_nSizeY,
						};
						CComObject<CTransformationWrapperAffine>* p = NULL;
						CComObject<CTransformationWrapperAffine>::CreateInstance(&p);
						CComPtr<ICanvasInteractingWrapper> pNew = p;
						p->Init(m_nSizeX, m_nSizeY, tScale, m_fRotation, tTranslation);
						*a_ppNew = pNew.Detach();
						return S_OK;
					}
					//// if the rectangle would have negative size, move it instead
					//{
					//	float fSizeX = (aPts[1].fX-aPts[0].fX)*tDirX.fX + (aPts[1].fY-aPts[0].fY)*tDirX.fY;
					//	if (fSizeX < 0.0f)
					//	{
					//		ULONG n = ((a_nIndex-2)>>2)&1;
					//		aPts[0^n] = aPts[1^n];
					//		aPts[3^n] = aPts[2^n];
					//		m_tCenter.fX = 0.5f*(aPts[1].fX + aPts[2].fX);
					//		m_tCenter.fY = 0.5f*(aPts[1].fY + aPts[2].fY);
					//		m_fSizeX = 0.0f;
					//	}
					//	float fSizeY = (aPts[3].fX-aPts[0].fX)*tDirY.fX + (aPts[3].fY-aPts[0].fY)*tDirY.fY;
					//	if (fSizeY < 0.0f)
					//	{
					//		ULONG n = ((a_nIndex>>2)&1)*3;
					//		aPts[3^n] = aPts[0^n];
					//		aPts[2^n] = aPts[1^n];
					//		m_tCenter.fX = 0.5f*(aPts[1].fX + aPts[0].fX);
					//		m_tCenter.fY = 0.5f*(aPts[1].fY + aPts[0].fY);
					//		m_fSizeY = 0.0f;
					//	}
					//}
					(*a_ppNew = this)->AddRef();
					return S_FALSE;
				}
			}
			return E_RW_INDEXOUTOFRANGE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(GetControlPointDesc)(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		TPixelCoords aPts[4];
		SimpleToPts(aPts);
		a_pLines->MoveTo(aPts[0].fX, aPts[0].fY);
		a_pLines->LineTo(aPts[1].fX, aPts[1].fY);
		a_pLines->LineTo(aPts[2].fX, aPts[2].fY);
		a_pLines->LineTo(aPts[3].fX, aPts[3].fY);
		a_pLines->LineTo(aPts[0].fX, aPts[0].fY);

		TPixelCoords const tCenter = {m_nSizeX*0.5f+m_tTranslation.x, m_nSizeY*0.5f+m_tTranslation.y};
		TPixelCoords const tOff = {tCenter.fX+0.35f*m_nSizeX*m_tScale.x*cosf(m_fRotation), tCenter.fY+0.35f*m_nSizeX*m_tScale.x*sinf(m_fRotation)};
		a_pLines->MoveTo(tCenter.fX, tCenter.fY);
		a_pLines->LineTo(tOff.fX, tOff.fY);
		return S_OK;
	}

	STDMETHOD(ToConfig)(IConfig* a_pConfig)
	{
		CComBSTR cMode(CFGID_TRANSFORM_MODE);
		CComBSTR cAngle(CFGID_TRANSFORM_ANGLE);
		CComBSTR cScale(CFGID_TRANSFORM_SCALE);
		CComBSTR cTranslation(CFGID_TRANSFORM_TRANSLATION);
		BSTR aIDs[4] = {cMode, cAngle, cScale, cTranslation};
		TConfigValue aVals[4];
		aVals[0] = CConfigValue(CFGVAL_TM_AFFINE);
		aVals[1] = CConfigValue(m_fRotation);
		aVals[2] = CConfigValue(m_tScale.x, m_tScale.y);
		aVals[3] = CConfigValue(m_tTranslation.x, m_tTranslation.y);
		return a_pConfig->ItemValuesSet(4, aIDs, aVals);
	}

private:
	ULONG m_nSizeX;
	ULONG m_nSizeY;
	TVector2f m_tScale;
	float m_fRotation;
	TVector2f m_tTranslation;
	//static TVector2f s_aCoords[8];
};

//TVector2f CTransformationWrapperAffine::s_aCoords[8] =
//{
//	{-1, -1},
//	{ 0, -1},
//	{ 1, -1},
//	{ 1,  0},
//	{ 1,  1},
//	{ 0,  1},
//	{-1,  1},
//	{-1,  0},
//};

// CDocumentOperationRasterImagePerspective

STDMETHODIMP CDocumentOperationRasterImagePerspective::NameGet(IOperationManager* UNREF(a_pManager), ILocalizedString** a_ppOperationName)
{
	try
	{
		*a_ppOperationName = NULL;
		*a_ppOperationName = _SharedStringTable.GetString(IDS_RASTERIMAGEPERSPECTIVE_NAME);
		return S_OK;
	}
	catch (...)
	{
		return a_ppOperationName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

#include <MultiLanguageString.h>

STDMETHODIMP CDocumentOperationRasterImagePerspective::Name(IUnknown* a_pContext, IConfig* a_pConfig, ILocalizedString** a_ppName)
{
	try
	{
		*a_ppName = NULL;
		LPCOLESTR pszName = L"[0409]Transformation[0405]Transformace";
		if (a_pConfig == NULL)
		{
			*a_ppName = new CMultiLanguageString(pszName);
			return S_OK;
		}
		float dest[8];
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1X), &cVal); dest[0] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1Y), &cVal); dest[1] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2X), &cVal); dest[2] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2Y), &cVal); dest[3] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3X), &cVal); dest[4] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3Y), &cVal); dest[5] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4X), &cVal); dest[6] = cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4Y), &cVal); dest[7] = cVal;
		if (dest[0] == 1.0f && dest[1] == 0 && dest[2] == 0 && dest[3] == 0 &&
			dest[4] == 0 && dest[5] == 1.0f && dest[6] == 1.0f && dest[7] == 1.0f)
		{
			pszName = L"[0409]Mirror[0405]Zrcadlit";
		}
		else if (dest[0] == 0 && dest[1] == 1.0f && dest[2] == 1.0f && dest[3] == 1.0f &&
				 dest[4] == 1.0f && dest[5] == 0 && dest[6] == 0 && dest[7] == 0)
		{
			pszName = L"[0409]Flip[0405]Převrátit";
		}

		*a_ppName = new CMultiLanguageString(pszName);
		return S_OK;
	}
	catch (...)
	{
		return a_ppName == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::ConfigCreate(IOperationManager* UNREF(a_pManager), IConfig** a_ppDefaultConfig)
{
	try
	{
		*a_ppDefaultConfig = NULL;

		CComPtr<IConfigWithDependencies> pCfgInit;
		RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

		CComBSTR bstrCFGID_TRANSFORM_MODE(CFGID_TRANSFORM_MODE);
		pCfgInit->ItemIns1ofN(bstrCFGID_TRANSFORM_MODE, CMultiLanguageString::GetAuto(L"[0409]Mode[0405]Mód"), NULL, CConfigValue(CFGVAL_TM_AFFINE), NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_TRANSFORM_MODE, CConfigValue(CFGVAL_TM_QUAD), CMultiLanguageString::GetAuto(L"[0409]Perspective transform[0405]Perspektivní transformace"), 0, NULL);
		pCfgInit->ItemOptionAdd(bstrCFGID_TRANSFORM_MODE, CConfigValue(CFGVAL_TM_AFFINE), CMultiLanguageString::GetAuto(L"[0409]Move, resize and rotate[0405]Posun, škálování a rotace"), 0, NULL);
		//pCfgInit->ItemOptionAdd(bstrCFGID_TRANSFORM_MODE, CConfigValue(CFGVAL_TM_FILL), CMultiLanguageString::GetAuto(L"[0409]Expand[0405]Roztáhnout"), 0, NULL);

		TConfigOptionCondition tCond;
		tCond.eConditionType = ECOCEqual;
		tCond.bstrID = bstrCFGID_TRANSFORM_MODE;
		tCond.tValue = CConfigValue(CFGVAL_TM_QUAD);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT1X), _SharedStringTable.GetStringAuto(IDS_CFGID_PT1X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTX_DESC), CConfigValue(0.1f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT1Y), _SharedStringTable.GetStringAuto(IDS_CFGID_PT1Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTY_DESC), CConfigValue(0.0f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT2X), _SharedStringTable.GetStringAuto(IDS_CFGID_PT2X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTX_DESC), CConfigValue(0.9f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT2Y), _SharedStringTable.GetStringAuto(IDS_CFGID_PT2Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTY_DESC), CConfigValue(0.3f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT3X), _SharedStringTable.GetStringAuto(IDS_CFGID_PT3X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTX_DESC), CConfigValue(0.9f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT3Y), _SharedStringTable.GetStringAuto(IDS_CFGID_PT3Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTY_DESC), CConfigValue(0.7f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT4X), _SharedStringTable.GetStringAuto(IDS_CFGID_PT4X_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTX_DESC), CConfigValue(0.1f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_PT4Y), _SharedStringTable.GetStringAuto(IDS_CFGID_PT4Y_NAME), _SharedStringTable.GetStringAuto(IDS_CFGID_PTY_DESC), CConfigValue(1.0f), NULL, 1, &tCond);

		tCond.tValue.iVal = CFGVAL_TM_AFFINE;
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSFORM_CENTER), CMultiLanguageString::GetAuto(L"[0409]Center[0405]Střed"), CMultiLanguageString::GetAuto(L"[0409]Center of rotation and scaling.[0405]Střed rotace a škálování."), CConfigValue(0.0f, 0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSFORM_ANGLE), CMultiLanguageString::GetAuto(L"[0409]Rotation[0405]Rotace"), CMultiLanguageString::GetAuto(L"[0409]Center of rotation and scaling.[0405]Střed rotace a škálování."), CConfigValue(0.0f), NULL, 1, &tCond);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSFORM_SCALE), CMultiLanguageString::GetAuto(L"[0409]Scale[0405]Škálování"), CMultiLanguageString::GetAuto(L"[0409]Center of rotation and scaling.[0405]Střed rotace a škálování."), CConfigValue(1.0f, 1.0f), NULL, 1, &tCond);
		//pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSFORM_SHEAR), CMultiLanguageString::GetAuto(L"[0409]Shear[0405]Zkosení"), CMultiLanguageString::GetAuto(L"[0409]Center of rotation and scaling.[0405]Střed rotace a škálování."), CConfigValue(0.0f, 0.0f), NULL, 0, NULL);
		pCfgInit->ItemInsSimple(CComBSTR(CFGID_TRANSFORM_TRANSLATION), CMultiLanguageString::GetAuto(L"[0409]Move[0405]Posun"), CMultiLanguageString::GetAuto(L"[0409]Center of rotation and scaling.[0405]Střed rotace a škálování."), CConfigValue(0.0f, 0.0f), NULL, 1, &tCond);

		CConfigCustomGUI<&CLSID_DocumentOperationRasterImagePerspective, CConfigGUIPerspective>::FinalizeConfig(pCfgInit);

		*a_ppDefaultConfig = pCfgInit.Detach();

		return S_OK;
	}
	catch (...)
	{
		return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::CreateWrapper(IConfig* a_pConfig, ULONG a_nSizeX, ULONG a_nSizeY, ICanvasInteractingWrapper** a_ppWrapper)
{
	if (a_ppWrapper == NULL)
		return E_POINTER;
	try
	{
		CComPtr<ICanvasInteractingWrapper> pNew;
		CConfigValue cMode;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_MODE), &cMode);
		if (cMode.operator LONG() == CFGVAL_TM_QUAD)
		{
			CComObject<CTransformationWrapper>* p = NULL;
			CComObject<CTransformationWrapper>::CreateInstance(&p);
			pNew = p;
			p->Init(a_nSizeX, a_nSizeY, a_pConfig);
		}
		else if (cMode.operator LONG() == CFGVAL_TM_AFFINE)
		{
			CComObject<CTransformationWrapperAffine>* p = NULL;
			CComObject<CTransformationWrapperAffine>::CreateInstance(&p);
			pNew = p;
			p->Init(a_nSizeX, a_nSizeY, a_pConfig);
		}
		*a_ppWrapper = pNew.Detach();
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::CanActivate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* UNREF(a_pConfig), IOperationContext* UNREF(a_pStates))
{
	try
	{
		static IID const aFts[] = {__uuidof(IDocumentEditableImage)};
		return (a_pDocument != NULL && SupportsAllFeatures(a_pDocument, itemsof(aFts), aFts)) ? S_OK : S_FALSE;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include "AGGBuffer.h"

#include <agg_trans_perspective.h>
#include <agg_trans_bilinear.h>
#include <agg_span_interpolator_trans.h>

// {7D22D397-D956-491e-9B55-652021DD6BAC}
extern GUID const CLSID_BilinearTransformHelper = {0x7d22d397, 0xd956, 0x491e, { 0x9b, 0x55, 0x65, 0x20, 0x21, 0xdd, 0x6b, 0xac}};

class ATL_NO_VTABLE CTransformHelperTransform : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTransformHelperTransform, &CLSID_BilinearTransformHelper>,
	public IRasterImageTransformer
{
public:
	CTransformHelperTransform()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CTransformHelperTransform)

BEGIN_COM_MAP(CTransformHelperTransform)
	COM_INTERFACE_ENTRY(IRasterImageTransformer)
END_COM_MAP()

	// IRasterImageTransformer methods
public:
	STDMETHOD(ProcessTile)(EImageChannelID a_eChannelID, float a_fGamma, TPixelChannel const* a_pDefault, TMatrix3x3f const* a_pContentTransform,
						   ULONG a_nSrcPixels, TPixelChannel const* a_pSrcData, TImagePoint const* a_pSrcOrigin, TImageSize const* a_pSrcSize, TImageStride const* a_pSrcStride,
						   ULONG a_nDstPixels, TPixelChannel* a_pDstData, TImagePoint const* a_pDstOrigin, TImageSize const* a_pDstSize, TImageStride const* a_pDstStride)
	{
		try
		{
			TImagePoint const tDstEnd = {a_pDstOrigin->nX+a_pDstSize->nX, a_pDstOrigin->nY+a_pDstSize->nY};
			TImagePoint const tSrcEnd = {a_pSrcOrigin->nX+a_pSrcSize->nX, a_pSrcOrigin->nY+a_pSrcSize->nY};

			// optimized processing for flip and mirror
			if (a_pContentTransform->_11 == -1.0f && a_pContentTransform->_12 == 0.0f && a_pContentTransform->_13 == 0.0f &&
				a_pContentTransform->_21 == 0.0f && a_pContentTransform->_22 == 1.0f && a_pContentTransform->_23 == 0.0f &&
				a_pContentTransform->_31 == LONG(a_pContentTransform->_31) && a_pContentTransform->_32 == 0.0f && a_pContentTransform->_33 == 1.0f)
			{
				// mirror (optimized code)
				LONG nDX = a_pContentTransform->_31-a_pDstOrigin->nX;
				ULONG nBefore = 0;
				ULONG nCopy = 0;
				ULONG nAfter = 0;
				if (nDX > tSrcEnd.nX)
				{
					nBefore = tSrcEnd.nX-nDX;
					if (nBefore > a_pDstSize->nX)
						nBefore = a_pDstSize->nX;
					nDX -= nBefore;
				}
				if (LONG(nDX-a_pDstSize->nX) < a_pSrcOrigin->nX)
				{
					nAfter = a_pSrcOrigin->nX-nDX+a_pDstSize->nX;
					if (nAfter+nBefore > a_pDstSize->nX)
						nAfter = a_pDstSize->nX-nBefore;
				}
				nCopy = a_pDstSize->nX-nBefore-nAfter;

				for (LONG nY = a_pDstOrigin->nY; nY < tDstEnd.nY; ++nY)
				{
					if (nY >= a_pSrcOrigin->nY && nY < tSrcEnd.nY)
					{
						TPixelChannel const* pSrc = a_pSrcData + a_pSrcStride->nY*(nY-a_pSrcOrigin->nY) + a_pSrcStride->nX*(nDX-a_pSrcOrigin->nX-1);
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nBefore; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nCopy; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX, pSrc-=a_pSrcStride->nX)
							*a_pDstData = *pSrc;
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nAfter; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					else
					{
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*a_pDstSize->nX; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					a_pDstData += a_pDstStride->nY-a_pDstStride->nX*a_pDstSize->nX;
				}
			}
			else if (a_pContentTransform->_11 == 1.0f && a_pContentTransform->_12 == 0.0f && a_pContentTransform->_13 == 0.0f &&
				a_pContentTransform->_21 == 0.0f && a_pContentTransform->_22 == -1.0f && a_pContentTransform->_23 == 0.0f &&
				a_pContentTransform->_31 == 0.0f && a_pContentTransform->_32 == LONG(a_pContentTransform->_32) && a_pContentTransform->_33 == 1.0f)
			{
				// flip (optimized code)
				ULONG nBefore = 0;
				ULONG nCopy = 0;
				ULONG nAfter = 0;
				if (a_pDstOrigin->nX < a_pSrcOrigin->nX)
				{
					nBefore = a_pSrcOrigin->nX-a_pDstOrigin->nX;
					if (nBefore > a_pDstSize->nX)
						nBefore = a_pDstSize->nX;
				}
				if (tDstEnd.nX > tSrcEnd.nX)
				{
					nAfter = tDstEnd.nX-tSrcEnd.nX;
					if (nAfter+nBefore > a_pDstSize->nX)
						nAfter = a_pDstSize->nX-nBefore;
				}
				nCopy = a_pDstSize->nX-nBefore-nAfter;

				for (LONG nY = a_pDstOrigin->nY; nY < tDstEnd.nY; ++nY)
				{
					LONG nDY = a_pContentTransform->_32-1-nY;
					if (nDY >= a_pSrcOrigin->nY && nDY < tSrcEnd.nY)
					{
						TPixelChannel const* pSrc = a_pSrcData + a_pSrcStride->nY*(nDY-a_pSrcOrigin->nY) + a_pSrcStride->nX*(a_pDstOrigin->nX < a_pSrcOrigin->nX ? a_pSrcOrigin->nX-a_pDstOrigin->nX : 0);
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nBefore; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nCopy; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX, pSrc+=a_pSrcStride->nX)
							*a_pDstData = *pSrc;
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*nAfter; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					else
					{
						for (TPixelChannel* pEnd = a_pDstData+a_pDstStride->nX*a_pDstSize->nX; a_pDstData != pEnd; a_pDstData+=a_pDstStride->nX)
							*a_pDstData = *a_pDefault;
					}
					a_pDstData += a_pDstStride->nY-a_pDstStride->nX*a_pDstSize->nX;
				}
			}
			else
			{
				TMatrix3x3f tInv;
				Matrix3x3fInverse(*a_pContentTransform, &tInv);
				TMatrix3x3f tMul;
				TMatrix3x3f tShift = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, a_pDstOrigin->nX, a_pDstOrigin->nY, 1.0f};
				Matrix3x3fMultiply(tShift, tInv, &tMul);
				agg::trans_perspective tr(
					tMul._11, tMul._12, tMul._13,
					tMul._21, tMul._22, tMul._23,
					tMul._31, tMul._32, tMul._33);

				CAutoPtr<CGammaTables> pGT;
				if (a_fGamma != 1.0f && a_fGamma >= 0.1f && a_fGamma <= 10.f)
					pGT.Attach(new CGammaTables(a_fGamma));

				typedef agg::span_allocator<agg::rgba16> span_alloc_type;
				span_alloc_type sa;
				agg::image_filter_hermite filter_kernel;
				agg::image_filter_lut filter(filter_kernel, false);

				typedef agg::span_interpolator_trans<agg::trans_perspective> interpolator_type;
				interpolator_type interpolator(tr);

				CRasterImagePreMpSrc img_accessor(a_pSrcData, a_pSrcOrigin, a_pSrcSize, a_pSrcStride, *a_pDefault, pGT);

				typedef agg::span_image_filter_rgba_2x2<CRasterImagePreMpSrc, interpolator_type> span_gen_type;
				span_gen_type sg(img_accessor, interpolator, filter);

				CRasterImageTarget cTarget(a_pDstData, a_pDstSize, a_pDstStride, pGT);
				agg::renderer_base<CRasterImageTarget> renb(cTarget);

				agg::renderer_scanline_aa<agg::renderer_base<CRasterImageTarget>, span_alloc_type, span_gen_type> ren(renb, sa, sg);
				agg::rasterizer_scanline_aa<> ras;
				agg::scanline_u8 sl;
				ras.reset();
				ras.move_to_d(0, 0);
				ras.line_to_d(a_pDstSize->nX, 0);
				ras.line_to_d(a_pDstSize->nX, a_pDstSize->nY);
				ras.line_to_d(0, a_pDstSize->nY);

				agg::render_scanlines(ras, sl, ren);
			}
			return S_OK;

		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
};

OBJECT_ENTRY_AUTO(CLSID_BilinearTransformHelper, CTransformHelperTransform)

TMatrix3x3f ParamsToMatrix(TImageSize tCanvas, IConfig* a_pConfig)
{
	TMatrix3x3f tMtx =
	{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};

	double dest[8];
	CConfigValue cVal;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1X), &cVal); dest[0] = cVal.operator float()*tCanvas.nX;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT1Y), &cVal); dest[1] = cVal.operator float()*tCanvas.nY;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2X), &cVal); dest[2] = cVal.operator float()*tCanvas.nX;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT2Y), &cVal); dest[3] = cVal.operator float()*tCanvas.nY;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3X), &cVal); dest[4] = cVal.operator float()*tCanvas.nX;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT3Y), &cVal); dest[5] = cVal.operator float()*tCanvas.nY;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4X), &cVal); dest[6] = cVal.operator float()*tCanvas.nX;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_PT4Y), &cVal); dest[7] = cVal.operator float()*tCanvas.nY;

	if (dest[0] == tCanvas.nX && dest[1] == 0 && dest[2] == 0 && dest[3] == 0 &&
		dest[4] == 0 && dest[5] == tCanvas.nY && dest[6] == tCanvas.nX && dest[7] == tCanvas.nY)
	{
		// mirror
		tMtx._11 = -1.0f;
		tMtx._31 = tCanvas.nX;
	}
	else if (dest[0] == 0 && dest[1] == tCanvas.nY && dest[2] == tCanvas.nX && dest[3] == tCanvas.nY &&
			 dest[4] == tCanvas.nX && dest[5] == 0 && dest[6] == 0 && dest[7] == 0)
	{
		// flip
		tMtx._22 = -1.0f;
		tMtx._32 = tCanvas.nY;
	}
	else
	{
		// arbitrary transformation
		agg::trans_perspective tr(0, 0, tCanvas.nX, tCanvas.nY, dest);
		if (fabs(tr.sx) <= agg::affine_epsilon || fabs(tr.sy) <= agg::affine_epsilon)//(!is_valid()) // fails for some valid cases
			throw E_FAIL;
		tMtx._11 = tr.sx;
		tMtx._12 = tr.shy;
		tMtx._13 = tr.w0;
		tMtx._21 = tr.shx;
		tMtx._22 = tr.sy;
		tMtx._23 = tr.w1;
		tMtx._31 = tr.tx;
		tMtx._32 = tr.ty;
		tMtx._33 = tr.w2;
	}

	return tMtx;
}

TMatrix3x3f CDocumentOperationRasterImagePerspective::GetMatrix(IDocumentImage* a_pImg, IConfig* a_pConfig)
{
	TImageSize tCanvas = {1, 1};
	a_pImg->CanvasGet(&tCanvas, NULL, NULL, NULL,/*&tOrigin, &tSize,*/ NULL);
	return GetMatrix(tCanvas, a_pConfig);
}

TMatrix3x3f CDocumentOperationRasterImagePerspective::GetMatrix(TImageSize a_tCanvas, IConfig* a_pConfig)
{
	TMatrix3x3f tMtx = TMATRIX3X3F_IDENTITY;
	CConfigValue cMode;
	a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_MODE), &cMode);
	if (cMode.operator LONG() == CFGVAL_TM_QUAD)
	{
		tMtx = ParamsToMatrix(a_tCanvas, a_pConfig);
	}
	else if (cMode.operator LONG() == CFGVAL_TM_AFFINE)
	{
		CConfigValue cVal;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_ANGLE), &cVal);
		float s = sinf(cVal);
		float c = cosf(cVal);
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_SCALE), &cVal);
		float sx = cVal[0];
		float sy = cVal[1];
		TMatrix3x3f tOff = {1, 0, 0, 0, 1, 0, -0.5f*a_tCanvas.nX, -0.5f*a_tCanvas.nY, 1};
		tMtx._11 = c*sx;
		tMtx._12 = s*sx;
		tMtx._21 = -s*sy;
		tMtx._22 = c*sy;
		tMtx._13 = tMtx._23 = 0.0f;
		a_pConfig->ItemValueGet(CComBSTR(CFGID_TRANSFORM_TRANSLATION), &cVal);
		tMtx._31 = cVal[0];
		tMtx._32 = cVal[1];
		tMtx._33 = 1.0f;
		TMatrix3x3f tTmp;
		Matrix3x3fMultiply(tOff, tMtx, &tTmp);
		tMtx = tTmp;
		tMtx._31 += a_tCanvas.nX*0.5f;
		tMtx._32 += a_tCanvas.nY*0.5f;

	}
	return tMtx;
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::Activate(IOperationManager* UNREF(a_pManager), IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* UNREF(a_pStates), RWHWND a_hParent, LCID a_tLocaleID)
{
	try
	{
		CComPtr<IDocumentEditableImage> pEI;
		a_pDocument->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
		if (pEI == NULL)
			return E_FAIL;

		CComObjectStackEx<CTransformHelperTransform> cTrn;

		TMatrix3x3f tMtx = GetMatrix(pEI, a_pConfig);

		return pEI->CanvasSet(NULL, NULL, &tMtx, &cTrn);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::Process(IDocument* a_pSrc, IConfig* a_pConfig, IDocumentBase* a_pDst, BSTR a_bstrPrefix)
{
	try
	{
		CComPtr<IDocumentEditableImage> pEI;
		a_pSrc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));
		if (pEI == NULL)
			return E_FAIL;

		a_pSrc->DocumentCopy(a_bstrPrefix, a_pDst, NULL, NULL);
		CComPtr<IDocument> pDstDoc;
		a_pDst->DataBlockDoc(a_bstrPrefix, &pDstDoc);
		pEI = NULL;
		pDstDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pEI));

		CComObjectStackEx<CTransformHelperTransform> cTrn;

		TMatrix3x3f tMtx = GetMatrix(pEI, a_pConfig);

		return pEI->CanvasSet(NULL, NULL, &tMtx, &cTrn);
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::Transform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f const* a_pContentTransform)
{
	return E_NOTIMPL;
}

void ComputeBounds(TMatrix3x3f tMtx, TRasterImageRect* a_pRect)
{
	TVector2f s11 = {a_pRect->tTL.nX, a_pRect->tTL.nY};
	TVector2f s12 = {a_pRect->tTL.nX, a_pRect->tBR.nY};
	TVector2f s21 = {a_pRect->tBR.nX, a_pRect->tTL.nY};
	TVector2f s22 = {a_pRect->tBR.nX, a_pRect->tBR.nY};
	TVector2f t11 = TransformVector2(tMtx, s11);
	TVector2f t12 = TransformVector2(tMtx, s12);
	TVector2f t21 = TransformVector2(tMtx, s21);
	TVector2f t22 = TransformVector2(tMtx, s22);
	float fX1 = min(t11.x, t12.x);
	if (fX1 > t21.x) fX1 = t21.x;
	if (fX1 > t22.x) fX1 = t22.x;
	float fY1 = min(t11.x, t12.y);
	if (fY1 > t21.y) fY1 = t21.y;
	if (fY1 > t22.y) fY1 = t22.y;
	float fX2 = max(t11.x, t12.x);
	if (fX2 < t21.x) fX2 = t21.x;
	if (fX2 < t22.x) fX2 = t22.x;
	float fY2 = max(t11.x, t12.y);
	if (fY2 < t21.y) fY2 = t21.y;
	if (fY2 < t22.y) fY2 = t22.y;

	a_pRect->tTL.nX = floorf(fX1);
	a_pRect->tTL.nY = floorf(fY1);
	a_pRect->tBR.nX = ceilf(fX2);
	a_pRect->tBR.nY = ceilf(fY2);
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::AdjustDirtyRect(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	try
	{
		if (a_pCanvas == NULL)
			return E_NOTIMPL;
		if (a_pRect)
		{
			TMatrix3x3f tMtx = GetMatrix(*a_pCanvas, a_pConfig);
			ComputeBounds(tMtx, a_pRect);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::NeededToCompute(IConfig* a_pConfig, TImageSize const* a_pCanvas, TRasterImageRect* a_pRect)
{
	try
	{
		if (a_pCanvas == NULL)
			return E_NOTIMPL;
		if (a_pRect)
		{
			TMatrix3x3f tMtx2 = GetMatrix(*a_pCanvas, a_pConfig);
			TMatrix3x3f tMtx;
			Matrix3x3fInverse(tMtx2, &tMtx);
			ComputeBounds(tMtx, a_pRect);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CDocumentOperationRasterImagePerspective::AdjustTransform(IConfig* a_pConfig, TImageSize const* a_pCanvas, TMatrix3x3f* a_pTransform)
{
	try
	{
		if (a_pCanvas == NULL)
			return E_NOTIMPL;
		if (a_pTransform)
			*a_pTransform = GetMatrix(*a_pCanvas, a_pConfig);
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <IconRenderer.h>

HICON CDocumentOperationRasterImagePerspective::GetDefaultIcon(ULONG a_nSize)
{
	CComPtr<IStockIcons> pSI;
	RWCoCreateInstance(pSI, __uuidof(StockIcons));
	static IRPathPoint const tl[] =
	{
		{96, 48, 0, 13.2548, 0, -13.2548},
		{72, 72, 0, 0, 13.2548, 0},
		{48, 72, 0, 0, 0, 0},
		{48, 96, 0, 13.2548, 0, 0},
		{24, 120, -13.2548, 0, 13.2548, 0},
		{0, 96, 0, 0, 0, 13.2548},
		{0, 48, 0, -13.2548, 0, 0},
		{24, 24, 0, 0, -13.2548, 0},
		{72, 24, 13.2548, 0, 0, 0},
	};
	static IRPathPoint const tr[] =
	{
		{256, 48, 0, 0, 0, -13.2548},
		{256, 96, 0, 13.2548, 0, 0},
		{232, 120, -13.2548, 0, 13.2548, 0},
		{208, 96, 0, 0, 0, 13.2548},
		{208, 72, 0, 0, 0, 0},
		{184, 72, -13.2548, 0, 0, 0},
		{160, 48, 0, -13.2548, 0, 13.2548},
		{184, 24, 0, 0, -13.2548, 0},
		{232, 24, 13.2548, 0, 0, 0},
	};
	static IRPathPoint const bl[] =
	{
		{48, 160, 0, 0, 0, -13.2548},
		{48, 184, 0, 0, 0, 0},
		{72, 184, 13.2548, 0, 0, 0},
		{96, 208, 0, 13.2548, 0, -13.2548},
		{72, 232, 0, 0, 13.2548, 0},
		{24, 232, -13.2548, 0, 0, 0},
		{0, 208, 0, 0, 0, 13.2548},
		{0, 160, 0, -13.2548, 0, 0},
		{24, 136, 13.2548, 0, -13.2548, 0},
	};
	static IRPathPoint const br[] =
	{
		{160, 208, 0, -13.2548, 0, 13.2548},
		{184, 184, 0, 0, -13.2548, 0},
		{208, 184, 0, 0, 0, 0},
		{208, 160, 0, -13.2548, 0, 0},
		{232, 136, 13.2548, 0, -13.2548, 0},
		{256, 160, 0, 0, 0, -13.2548},
		{256, 208, 0, 13.2548, 0, 0},
		{232, 232, 0, 0, 13.2548, 0},
		{184, 232, -13.2548, 0, 0, 0},
	};
	static IRGridItem gridX[] = { {0, 0}, {0, 48}, {0, 208}, {0, 256} };
	static IRGridItem gridY[] = { {0, 24}, {0, 72}, {0, 184}, {0, 232} };
	static IRCanvas canvas = {0, 0, 256, 256, itemsof(gridX), itemsof(gridY), gridX, gridY};
	CIconRendererReceiver cRenderer(a_nSize);
	cRenderer(&canvas, itemsof(tl), tl, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(tr), tr, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(bl), bl, pSI->GetMaterial(ESMScheme1Color2));
	cRenderer(&canvas, itemsof(br), br, pSI->GetMaterial(ESMScheme1Color2));
	return cRenderer.get();
}
