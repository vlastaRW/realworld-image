
#pragma once

#include <math.h>

#include "FillStyleGradientDlg.h"

class CFillStyleDiamondDlg : public CFillStyleGradientTemplDlg<CFillStyleDiamondDlg, CFillStyleDataDiamond>
{
	typedef CFillStyleGradientTemplDlg<CFillStyleDiamondDlg, CFillStyleDataDiamond> TBase;
public:
	enum
	{
		IDC_GRAD_SIDES_LABEL = IDC_GRAD_LAST,
		IDC_GRAD_SIDES
	};

	BEGIN_DIALOG_EX(0, 0, 100, 76, 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
		CONTROL_LTEXT(_T("[0409]Sides:[0405]Strany:"), IDC_GRAD_SIDES_LABEL, 5, 7, 35, 8, WS_VISIBLE, 0)
		CONTROL_CONTROL(_T(""), IDC_GRAD_SIDES, TRACKBAR_CLASS, TBS_BOTH | TBS_NOTICKS | WS_TABSTOP | WS_VISIBLE, 40, 5, 60, 12, 0)
		CONTROL_CHECKBOX(_T("[0409]Follow drawn shape[0405]Následovat kreslený tvar"), IDC_GRAD_SHAPECENTER, 5, 22, 50, 10, BS_AUTOCHECKBOX | WS_VISIBLE | WS_TABSTOP, 0)
		CONTROL_EDITTEXT(IDC_GRAD_POSITION, 56, 21, 28, 12, WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_LTEXT(_T("%"), IDC_GRAD_POSITION_UNIT, 88, 23, 8, 8, 0, 0)
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CFillStyleDiamondDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		CHAIN_MSG_MAP(TBase)
	END_MSG_MAP()

	BEGIN_DLGRESIZE_MAP(CFillStyleDiamondDlg)
		DLGRESIZE_CONTROL(IDC_GRAD_SIDES, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_GRADIENT, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_SHAPECENTER, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_COLOR, DLSZ_SIZE_X|DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDC_GRAD_POSITION, DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_GRAD_POSITION_UNIT, DLSZ_MOVE_X)
	END_DLGRESIZE_MAP()

	BEGIN_CTXHELP_MAP(CFillStyleDiamondDlg)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_SHAPECENTER, IDS_HELP_FSCENTER)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_GRADIENT, IDS_HELP_GRADIENTCTL)
		//CTXHELP_CONTROL_RESOURCE(IDC_GRAD_COLOR, IDS_HELP_GRADIENTSTOPCLR)
		CTXHELP_CONTROL_RESOURCE(IDC_GRAD_POSITION, IDS_HELP_GRADIENTSTOPPOS)
	END_CTXHELP_MAP()

	LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		m_wndSides = GetDlgItem(IDC_GRAD_SIDES);
		m_wndSides.SetRange(3, 9);
		a_bHandled = FALSE;
		return 0;
	}
	LRESULT OnHScroll(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
	{
		if (!m_bEnableUpdates)
			return 0;

		GUIToData();
		DataToGUI();
		DataToState();
		return 0;
	}

	void DataToGUI()
	{
		m_wndSides.SetPos(m_cData.nSides);
		TBase::DataToGUI();
	}

	void GUIToData()
	{
		m_cData.nSides = m_wndSides.GetPos();
		TBase::GUIToData();
	}

private:
	CTrackBarCtrl m_wndSides;
};


#include <boost/spirit.hpp>
using namespace boost::spirit;


class CFillStyleDiamond :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageBrush,
	public IRasterImageEditToolScripting
{
public:
	CFillStyleDiamond()
	{
		m_tRelCenter.fX = m_tRelCenter.fY = 0.5f;
		m_fRelRadius = 0.5f;
		m_bAbsValid = false;
		m_tCenterAuto.fX = m_tCenterAuto.fY = m_tCenter.fX = m_tCenter.fY = 0.0f;
		m_fSizeXAuto = m_fSizeYAuto = m_fRadius = 0.0f;
		m_fAngle = 0.0f;
		UpdateDirs();
	}

	BEGIN_COM_MAP(CFillStyleDiamond)
		COM_INTERFACE_ENTRY(IRasterImageBrush)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageBrush methods
public:
	STDMETHOD(Init)(IRasterImageBrushOwner* a_pOwner)
	{
		m_pOwner = a_pOwner;
		if (a_pOwner)
		{
			ULONG nX = 0;
			ULONG nY = 0;
			a_pOwner->Size(&nX, &nY);
			m_fSizeXAuto = m_tCenterAuto.fX = nX*0.5f;
			m_fSizeYAuto = m_tCenterAuto.fY = nY*0.5f;
			if (m_cData.bAutoCenter)
				m_bAbsValid = false;
		}
		UpdateDirs();
		return S_OK;
	}
	STDMETHOD(SetShapeBounds)(TPixelCoords const* a_pCenter, float a_fSizeX, float a_fSizeY, float UNREF(a_fAngle))
	{
		m_tCenterAuto = *a_pCenter;
		m_fSizeXAuto = a_fSizeX;
		m_fSizeYAuto = a_fSizeY;
		if (m_cData.bAutoCenter)
		{
			m_bAbsValid = false;
			if (m_pOwner)
			{
				m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CFillStyleDataDiamond::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			bool bPrevAutoCenter = m_cData.bAutoCenter;
			m_cData = *(pData->InternalData());
			Invalidate1DMapCache();
			if (m_pOwner)
				m_pOwner->RectangleChanged(NULL);
			m_bAbsValid = false;
			//m_tRelCenter.fX = m_tRelCenter.fY = 0.5f;
			//m_fRelRadius = 0.5f;
			if (!bPrevAutoCenter && m_cData.bAutoCenter)
				SetShapeBounds(&m_tCenterAuto, m_fSizeXAuto, m_fSizeYAuto, 0.0f);
			else if (bPrevAutoCenter && !m_cData.bAutoCenter && m_pOwner)
			{
				m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
			UpdateDirs();
		}
		return S_OK;
	}
	STDMETHOD(GetState)(ISharedState** a_ppState)
	{
		try
		{
			*a_ppState = NULL;
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			if (pNew->Init(m_cData))
			{
				m_pOwner->SetBrushState(pTmp);
			}
			*a_ppState = pTmp.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppState ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(NeedsPrepareShape)()
	{
		return S_FALSE;
	}
	STDMETHOD(PrepareShape)(RECT const* a_pBounds, ULONG a_nPaths, TRWPolygon const* a_pPaths)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(IsSolid)(RECT const* a_pRect)
	{
		return S_FALSE; // TODO: consider rectangle
	}
	STDMETHOD(GetBrushTile)(ULONG a_nX, ULONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, TRasterImagePixel* a_pBuffer)
	{
		TRasterImagePixel cMap[256];
		Update1DMap(cMap, a_fGamma);
		EnsureAbsValid();
		for (ULONG y = 0; y < a_nSizeY; ++y)
		{
			float const fDY = a_nY+y-m_tCenter.fY+0.5f;
			for (ULONG x = 0; x < a_nSizeX; ++x)
			{
				float const fDX = a_nX+x-m_tCenter.fX+0.5f;
				if (fDX*fDX + fDY*fDY > m_fRadius*m_fRadius)
				{
					*a_pBuffer = cMap[255];
				}
				else
				{
					float d = 0.0f;
					std::vector<TPixelCoords>::const_iterator i = m_aDirs.begin();
					for (std::vector<TPixelCoords>::const_iterator e = m_aDirs.end(); i != e; ++i)
					{
						float d2 = fDX*i->fX + fDY*i->fY;
						if (d2 > d)
							d = d2;
					}
					float fD = d/m_fRadius;
					if (fD > 1.0f) fD = 1.0f;
					*a_pBuffer = cMap[BYTE(fD*255.0f+0.5f)];
				}
				++a_pBuffer;
			}
			a_pBuffer += a_nStride-a_nSizeX;
		}
		return S_OK;
	}
	STDMETHOD(AdjustCoordinates)(TPixelCoords* a_pPos, ULONG a_nControlPointIndex)
	{
		return S_OK;
	}
	STDMETHOD(GetControlPointCount)(ULONG* a_pCount)
	{
		try
		{
			*a_pCount = 2;
			return S_OK;
		}
		catch (...)
		{
			return a_pCount ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(GetControlPoint)(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		try
		{
			if (a_nIndex >= 2)
				return E_RW_INDEXOUTOFRANGE;
			EnsureAbsValid();
			*a_pPos = m_tCenter;
			if (a_nIndex == 1)
			{
				a_pPos->fX += m_fRadius*cosf(m_fAngle);
				a_pPos->fY += m_fRadius*sinf(m_fAngle);
			}
			*a_pClass = 0;
			return S_OK;
		}
		catch (...)
		{
			return a_pPos ? E_UNEXPECTED : E_POINTER;
		}
	}
	STDMETHOD(SetControlPoint)(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bReleased, float a_fPointSize)
	{
		try
		{
			if (a_nIndex >= 2)
				return E_RW_INDEXOUTOFRANGE;
			EnsureAbsValid();
			if (a_nIndex == 0)
			{
				m_tCenter = *a_pPos;
				if (m_cData.bAutoCenter)
				{
					m_tRelCenter.fX = ((m_tCenter.fX-m_tCenterAuto.fX)/m_fSizeXAuto + 1.0f)*0.5f;
					m_tRelCenter.fY = ((m_tCenter.fY-m_tCenterAuto.fY)/m_fSizeYAuto + 1.0f)*0.5f;
				}
				else
				{
					ULONG nX = 0;
					ULONG nY = 0;
					if (m_pOwner) m_pOwner->Size(&nX, &nY);
					if (nX && nY)
					{
						m_tRelCenter.fX = m_tCenter.fX/nX;
						m_tRelCenter.fY = m_tCenter.fY/nY;
					}
				}
			}
			else
			{
				float const dist = (a_pPos->fX-m_tCenter.fX)*(a_pPos->fX-m_tCenter.fX) + (a_pPos->fY-m_tCenter.fY)*(a_pPos->fY-m_tCenter.fY);
				if (dist > 0.0f)
				{
					m_fRadius = sqrtf(dist);
					m_fAngle = atan2f(a_pPos->fY-m_tCenter.fY, a_pPos->fX-m_tCenter.fX);
					UpdateDirs();
					if (m_cData.bAutoCenter)
					{
						m_fRelRadius = m_fRadius/(m_fSizeXAuto+m_fSizeYAuto);
					}
					else
					{
						ULONG nX = 0;
						ULONG nY = 0;
						if (m_pOwner) m_pOwner->Size(&nX, &nY);
						if (nX && nY)
						{
							m_fRelRadius = m_fRadius/((nX+nY)*0.5f);
						}
					}
				}
			}
			if (m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				if (a_nIndex == 0)
					m_pOwner->ControlPointChanged(0);
				m_pOwner->ControlPointChanged(1);
				m_pOwner->ControlLinesChanged();
			}
			return S_OK;
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
	STDMETHOD(GetControlLines)(IEditToolControlLines* a_pLines)
	{
		EnsureAbsValid();
		a_pLines->MoveTo(m_tCenter.fX, m_tCenter.fY);
		a_pLines->LineTo(m_tCenter.fX+m_fRadius*cosf(m_fAngle), m_tCenter.fY+m_fRadius*sinf(m_fAngle));
		return S_OK;
	}

	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		if (!m_cData.bAutoCenter && a_pMatrix && memcmp(a_pMatrix, &TMATRIX3X3F_IDENTITY, sizeof*a_pMatrix))
		{
			float const fW1 = 1.0f/(a_pMatrix->_13*m_tCenter.fX + a_pMatrix->_23*m_tCenter.fY + a_pMatrix->_33);
			TPixelCoords const t1 =
			{
				fW1*(a_pMatrix->_11*m_tCenter.fX + a_pMatrix->_21*m_tCenter.fY + a_pMatrix->_31),
				fW1*(a_pMatrix->_12*m_tCenter.fX + a_pMatrix->_22*m_tCenter.fY + a_pMatrix->_32),
			};
			m_tCenter = t1;
			float fScale = sqrtf(sqrtf(a_pMatrix->_11*a_pMatrix->_11+a_pMatrix->_12*a_pMatrix->_12)*sqrtf(a_pMatrix->_21*a_pMatrix->_21+a_pMatrix->_22*a_pMatrix->_22));
			m_fRadius *= fScale;
			m_fAngle += atan2f(a_pMatrix->_21, a_pMatrix->_11);
			if (m_pOwner)
			{
				m_pOwner->RectangleChanged(NULL);
				m_pOwner->ControlPointsChanged();
				m_pOwner->ControlLinesChanged();
			}
			//UpdateAbsolutePositions();
		}
		return S_OK;
	}

	void EnsureAbsValid()
	{
		if (!m_bAbsValid)
		{
			if (m_cData.bAutoCenter)
			{
				m_tCenter.fX = m_tCenterAuto.fX+m_fSizeXAuto*(m_tRelCenter.fX*2.0f-1.0f);
				m_tCenter.fY = m_tCenterAuto.fY+m_fSizeYAuto*(m_tRelCenter.fY*2.0f-1.0f);
				m_fRadius = (m_fSizeXAuto+m_fSizeYAuto)*m_fRelRadius;
			}
			else
			{
				ULONG nX = 0;
				ULONG nY = 0;
				if (m_pOwner) m_pOwner->Size(&nX, &nY);
				m_tCenter.fX = nX*m_tRelCenter.fX;
				m_tCenter.fY = nY*m_tRelCenter.fY;
				m_fRadius = (nX+nY)*0.5f*m_fRelRadius;
			}
			m_bAbsValid = true;
		}
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CFillStyleDataDiamond cData = m_cData;
			TPixelCoords tCenter = {0.5f, 0.5f};
			float fRadius = 0.5f;
			float fAngle = 0.0f;
			int nMode = 2;
			int nSides = 4;
			CGradientColorPicker::CGradient cGradient;
			std::pair<WORD, CButtonColorPicker::SColor> t;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(*((int_p[assign_a(t.first)]>>ch_p(L',')>>real_p[assign_a(t.second.fR)]>>ch_p(L',')>>real_p[assign_a(t.second.fG)]>>ch_p(L',')>>real_p[assign_a(t.second.fB)]>>ch_p(L',')>>real_p[assign_a(t.second.fA)]>>ch_p(L','))[insert_at_a(cGradient, t.first, t.second)]))>>
					(((str_p(L"ABSOLUTE")|str_p(L"\"ABSOLUTE\""))[assign_a(nMode, 0)]|(str_p(L"RELIMAGE")|str_p(L"\"RELIMAGE\""))[assign_a(nMode, 1)]|(str_p(L"RELSHAPE")|str_p(L"\"RELSHAPE\""))[assign_a(nMode, 2)]))>>
					//((str_p(L"ABSOLUTE")[assign_a(nMode, 0)]|str_p(L"RELIMAGE")[assign_a(nMode, 1)]|str_p(L"RELSHAPE")[assign_a(nMode, 2)]))>>
					(ch_p(L',')>>int_p[assign_a(nSides)])>>
					(!(ch_p(L',')>>real_p[assign_a(tCenter.fX)]>>ch_p(L',')>>real_p[assign_a(tCenter.fY)]>>ch_p(L',')>>real_p[assign_a(fRadius)]>>ch_p(L',')>>real_p[assign_a(fAngle)]))
					).full;
			if (!bParsed)
				return E_INVALIDARG;
			m_cData.bAutoCenter = nMode == 2;
			if (cGradient.size() < 2)
			{
				cGradient.clear();
				cGradient[0] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f);
				cGradient[0xffff] = CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f);
			}
			m_fAngle = fAngle;
			std::swap(m_cData.cGradient, cGradient);
			if (nSides > 2 && nSides < 10)
				m_cData.nSides = nSides;
			Invalidate1DMapCache();
			if (nMode == 0)
			{
				m_tCenter = tCenter;
				m_fRadius = fRadius;
				m_bAbsValid = true;
			}
			else
			{
				m_tRelCenter = tCenter;
				m_fRelRadius = fRadius;
				m_bAbsValid = false;
			}
			UpdateDirs();
			if (cData != m_cData && m_pOwner)
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (pNew->Init(m_cData))
				{
					m_pOwner->SetBrushState(pTmp);
				}
			}

			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ToText)(BSTR* a_pbstrParams)
	{
		CComBSTR bstr;
		for (CGradientColorPicker::CGradient::const_iterator i = m_cData.cGradient.begin(); i != m_cData.cGradient.end(); ++i)
		{
			wchar_t sz[64];
			swprintf(sz, L"%i,%g,%g,%g,%g,", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA);
			bstr += sz;
		}
		wchar_t sz2[128];
		if (m_cData.bAutoCenter)
			swprintf(sz2, L"RELSHAPE,%i,%g,%g,%g,%g", m_cData.nSides, m_tRelCenter.fX, m_tRelCenter.fY, m_fRelRadius, m_fAngle);
		else
			swprintf(sz2, L"ABSOLUTE,%i,%g,%g,%g,%g", m_cData.nSides, m_tCenter.fX, m_tCenter.fY, m_fRadius, m_fAngle);
		bstr += sz2;
		*a_pbstrParams = bstr.Detach();
		return S_OK;
	}

private:
	void Invalidate1DMapCache()
	{
		m_bCacheInvalid = true;
	}
	void Update1DMap(TRasterImagePixel* a_pMap, float a_fGamma)
	{
		ObjectLock cLock(this);
		if (m_bCacheInvalid || m_cCacheMap.empty() || m_fCacheGamma != a_fGamma)
		{
			m_cCacheMap.resize(256);
			CButtonColorPicker::SColor aVals[256];
			CGradientColorPicker::RenderGradient(m_cData.cGradient, 256, 0, 256, aVals);
			for (ULONG i = 0; i < 256; ++i)
				*reinterpret_cast<DWORD*>(&(m_cCacheMap[i])) = aVals[i].ToBGRA(a_fGamma);
			m_fCacheGamma = a_fGamma;
			m_bCacheInvalid = false;
		}
		std::copy(m_cCacheMap.begin(), m_cCacheMap.end(), a_pMap);
	}

	void UpdateDirs()
	{
		m_aDirs.resize(m_cData.nSides);
		m_fNormalization = 1.0f/cosf(3.14159265359/m_cData.nSides); 
		for (int i = 0; i < m_cData.nSides; ++i)
		{
			m_aDirs[i].fX = m_fNormalization*cosf(m_fAngle+2*3.14159265359*(i+0.5)/m_cData.nSides);
			m_aDirs[i].fY = m_fNormalization*sinf(m_fAngle+2*3.14159265359*(i+0.5)/m_cData.nSides);
		}
	}

private:
	CComPtr<IRasterImageBrushOwner> m_pOwner;
	TPixelCoords m_tCenter;
	float m_fRadius;
	TPixelCoords m_tRelCenter;
	float m_fRelRadius;
	bool m_bAbsValid;
	TPixelCoords m_tCenterAuto;
	float m_fSizeXAuto;
	float m_fSizeYAuto;
	CFillStyleDataDiamond m_cData;
	float m_fAngle;
	std::vector<TPixelCoords> m_aDirs;
	float m_fNormalization;

	// cached gradient map
	std::vector<TRasterImagePixel> m_cCacheMap;
	float m_fCacheGamma;
	bool m_bCacheSwapColors;
	bool m_bCacheCustomGradient;
	bool m_bCacheInvalid;
};

