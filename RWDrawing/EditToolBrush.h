
#pragma once

#include "EditTool.h"
#include "EditToolPixelBuffer.h"
#include "EditToolWithBrush.h"
#include <math.h>
#include <Resampling.h>
#include <RWDocumentAnimation.h>

#include <boost/spirit.hpp>
using namespace boost::spirit;


struct CEditToolDataBrush
{
	MIDL_INTERFACE("D6CC50E5-729E-4AEA-AD81-F8CA2CD36FB5")
	ISharedStateToolData : public ISharedState
	{
	public:
		STDMETHOD_(CEditToolDataBrush const*, InternalData)() = 0;
	};

	enum EShape
	{
		ESRound = 0,
		ESSquare,
		ESDiagonal,
		ESCustom
	};

	CEditToolDataBrush() :
		eShape(ESRound), fSize(11.0f), fBlur(20.0f), fFlow(100.0f),
		bSize(true), bBlur(false), bFlow(false),
		bScatter(false), bRotate(true), bResize(false),
		bSmooth(false), fSmooth(1.0f)
	{
	}

	HRESULT FromString(BSTR a_bstr)
	{
		if (a_bstr == NULL)
			return S_FALSE;
		bSize = bBlur = bFlow = false;
		float fScatter = 0.0f;
		float fRotate = 1.0f;
		float fResize = 0.0f;
		float fSmoothing = -1.0f;
		bool bParsed = parse(a_bstr, a_bstr+SysStringLen(a_bstr),
				(str_p(L"ROUND")[assign_a(eShape, ESRound)]|str_p(L"SQUARE")[assign_a(eShape, ESSquare)]|str_p(L"DIAGONAL")[assign_a(eShape, ESDiagonal)]|
				 confix_p(L'"', (*c_escape_ch_p)[assign_a(strBrushPath)], L'"')[assign_a(eShape, ESCustom)]) >> ch_p(L',') >>
				real_p[assign_a(fSize)] >> (!(ch_p(L',') >> str_p(L"NP")[assign_a(bSize, true)])) >> ch_p(L',') >>
				real_p[assign_a(fBlur)] >> (!(ch_p(L',') >> str_p(L"NP")[assign_a(bBlur, true)])) >> ch_p(L',') >>
				real_p[assign_a(fFlow)] >> (!(ch_p(L',') >> str_p(L"NP")[assign_a(bFlow, true)])) >>
				!(ch_p(L',') >> real_p[assign_a(fScatter)]) >>
				!(ch_p(L',') >> real_p[assign_a(fRotate)]) >>
				!(ch_p(L',') >> real_p[assign_a(fResize)]) >>
				!(ch_p(L',') >> real_p[assign_a(fSmoothing)])
				).full;
		if (!bParsed)
			return E_INVALIDARG;
		bScatter = fScatter >= 0.5f;
		bRotate = fRotate >= 0.5f;
		bResize = fResize >= 0.5f;
		if (fSmoothing < 0)
		{
			bSmooth = false;
			fSmooth = -fSmoothing;
		}
		else
		{
			bSmooth = true;
			fSmooth = fSmoothing;
		}
		size_t nLen = strBrushPath.length();
		size_t iD = 0;
		size_t iS = 0;
		while (iS < nLen)
		{
			if (strBrushPath[iS] == L'\\')
				++iS;
			if (iS == nLen)
				break;
			strBrushPath[iD] = strBrushPath[iS];
			++iS;
			++iD;
		}
		if (iD != iS)
			strBrushPath.resize(iD);
		return S_OK;
	}
	HRESULT ToString(BSTR* a_pbstr)
	{
		*a_pbstr = NULL;
		ULONG nLen = 128;
		if (!strBrushPath.empty()) for (LPCWSTR psz = strBrushPath.c_str(); *psz; ++psz)
			if (*psz == L'\"' || *psz == L'\\')
				++nLen;
		CAutoVectorPtr<OLECHAR> sz(new OLECHAR[nLen]);
		LPOLESTR pszD = sz;
		if (eShape == ESCustom && !strBrushPath.empty())
		{
			*pszD++ = L'\"';
			for (LPCWSTR psz = strBrushPath.c_str(); *psz; ++psz, ++pszD)
			{
				if (*psz == L'\"' || *psz == L'\\')
					*(pszD++) = L'\\';
				*pszD = *psz;
			}
			*pszD++ = L'\"';
		}
		else
		{
			wcscpy(pszD, eShape == ESDiagonal ? L"DIAGONAL" : (eShape == ESSquare ? L"SQUARE" : L"ROUND"));
			pszD += wcslen(pszD);
		}
		swprintf(pszD, L",%g%s,%g%s,%g%s,%i,%i,%i,%g", fSize, bSize ? L",NP" : L"", fBlur, bBlur ? L",NP" : L"", fFlow, bFlow ? L",NP" : L"", bScatter ? 1 : 0, bRotate ? 1 : 0, bResize ? 1 : 0, bSmooth ? fSmooth : -fSmooth);
		*a_pbstr = SysAllocString(sz);
		return S_OK;
	}

	EShape eShape;
	float fSize;
	float fBlur;
	float fFlow;
	bool bSize;
	bool bBlur;
	bool bFlow;
	std::wstring strBrushPath;
	bool bScatter;
	bool bRotate;
	bool bResize;
	bool bSmooth;
	float fSmooth;
};

#include "EditToolBrushDlg.h"


class CEditToolBrush :
	public CEditToolPixelCoverageBuffer<CEditToolBrush>, // pixel coverage cache
	public CEditToolWithSolidBrush<CEditToolBrush>, // colors handling
	public CEditToolCustomPrecisionCursor<>, // cursor handler
	public CEditToolWithCtrlDropper<CEditToolBrush, CEditToolBrush, CRasterImageEditToolBase, CEditToolCustomPrecisionCursor<> >,
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolBrush, // T - the top level class for cross casting
		CEditToolBrush, // TResetHandler
		CEditToolPixelCoverageBuffer<CEditToolBrush>, // TDirtyHandler
		CEditToolPixelCoverageBuffer<CEditToolBrush>, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TOutlineHandler
		CEditToolWithSolidBrush<CEditToolBrush>, // TBrushHandler
		CEditToolBrush, // TGlobalsHandler
		CEditToolWithCtrlDropper<CEditToolBrush, CEditToolBrush, CRasterImageEditToolBase, CEditToolCustomPrecisionCursor<> >, // TAdjustCoordsHandler
		CEditToolWithCtrlDropper<CEditToolBrush, CEditToolBrush, CRasterImageEditToolBase, CEditToolCustomPrecisionCursor<> >, // TGetCursorHandler
		CEditToolWithCtrlDropper<CEditToolBrush, CEditToolBrush, CRasterImageEditToolBase, CEditToolCustomPrecisionCursor<> >, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CEditToolBrush// TControlLinesHandler
	>,
	public IRasterImageEditToolScripting
{
public:
	CEditToolBrush() : m_eBlendingMode(EBMDrawOver), m_fTrackLength(0.0f), m_fLastPressure(0.0f), m_fDrawnPressure(0.0f), m_bMouseLeft(true),
		m_nLastSeed(0), m_cRand(0)
	{
	}

	EBlendingMode M_BlendingMode() const
	{
		return m_eBlendingMode;
	}
	void PrepareShape()
	{
	}

	BEGIN_COM_MAP(CEditToolBrush)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
		COM_INTERFACE_ENTRY(IRasterImageEditToolScripting)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	HRESULT _Reset()
	{
		m_bMouseLeft = true;
		RECT const rc = M_DirtyRect();
		m_cSamples.clear();
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY, true);
		m_fLastPressure = m_fDrawnPressure = m_fTrackLength = 0.0f;
		if (rc.left < rc.right)
			M_Window()->RectangleChanged(&rc);
		return S_OK;
	}

	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		CComQIPtr<CEditToolDataBrush::ISharedStateToolData> pData(a_pState);
		if (pData)
		{
			m_cData = *(pData->InternalData());
			if (m_cData.eShape == CEditToolDataBrush::ESCustom)
			{
				if (m_strBrushPath != m_cData.strBrushPath)
					LoadBrush(m_cData.strBrushPath);
			}
			else
			{
				m_pBrushFrames.Free();
				m_pMonoBrushFrames.Free();
			}
			RECT rc = M_DirtyRect();
			RecomputeCache();
			RECT rc2 = M_DirtyRect();
			if (rc2.left > rc.left) rc2.left = rc.left;
			if (rc2.top > rc.top) rc2.top = rc.top;
			if (rc2.right < rc.right) rc2.right = rc.right;
			if (rc2.bottom < rc.bottom) rc2.bottom = rc.bottom;
			M_Window()->RectangleChanged(&rc2);
			M_Window()->ControlLinesChanged();
		}
		return S_OK;
	}
	HRESULT _SetGlobals(EBlendingMode a_eBlendingMode, ERasterizationMode a_eRasterizationMode, ECoordinatesMode a_eCoordinatesMode)
	{
		if (m_eBlendingMode != a_eBlendingMode)
		{
			m_eBlendingMode = a_eBlendingMode;
			M_Window()->RectangleChanged(&M_DirtyRect());
		}
		return S_OK;
	}

	HRESULT _ProcessInputEvent(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, TPixelCoords const* a_pPointerSize, float a_fNormalPressure, float a_fTangentPressure, float a_fOrientation, float a_fRotation, float a_fZ, DWORD* a_pMaxIdleTime)
	{
		if (a_pPos == NULL)
		{
			if (!m_bMouseLeft)
			{
				m_bMouseLeft = true;
				M_Window()->ControlLinesChanged();
			}
			return S_OK;
		}
		bool bEnd = false;
		if (a_eKeysState&ECKSShift && m_fLastPressure > 0.0f && a_fNormalPressure > 0.05f)
		{
			if (m_tCursorPos.fX != a_pPos->fX || m_tCursorPos.fY != a_pPos->fY)
			{
				m_tCursorPos = *a_pPos;
				M_Window()->ControlLinesChanged();
			}
			return S_OK; // draw straight lines with SHIFT down
		}
		else if (a_eKeysState&ECKSShift && m_fLastPressure > 0.0f)
		{
			a_fNormalPressure = m_fLastPressure;
			bEnd = true;
		}

		HRESULT hRes = S_OK;
		bool bPosChange = m_tLastPos.fX != a_pPos->fX || m_tLastPos.fY != a_pPos->fY || m_bMouseLeft;
		float const fSize = m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize;
		m_bMouseLeft = false;
		if (m_fLastPressure == 0.0f)
		{
			if (a_fNormalPressure > 0.05f)
			{
				// start drawing
				if (CEditToolPixelCoverageBuffer::IsDirty())
				{
					ATLASSERT(FALSE); // just in case
					CEditToolPixelCoverageBuffer::DeleteCachedData();
				}
				m_cSamples.clear();
				m_fTrackLength = 0.0f;
				m_fLastPressure = a_fNormalPressure;
				m_tCursorPos = m_tLastPos = *a_pPos;
				SSample s = {a_pPos->fX, a_pPos->fY, a_fNormalPressure};
				m_cSamples.push_back(s);

				if (m_nLastSeed == 0)
					m_nLastSeed = GetTickCount();
				m_cRand.RandomInit(m_nLastSeed);

				if (m_cData.eShape == CEditToolDataBrush::ESCustom && (m_pBrushFrames.m_p || m_pMonoBrushFrames.m_p))
					DrawBrush(*a_pPos, a_fNormalPressure, fSize);
				else
					DrawPattern(*a_pPos, a_fNormalPressure, fSize);
			}
			else
			{
				m_tCursorPos = m_tLastPos = *a_pPos;
			}
		}
		else
		{
			if (a_fNormalPressure < 0.05f)
			{
				m_nLastSeed = 0;
				// stop drawing
				//if (!m_cData.bSmooth)
					hRes = ETPAApply;//S_OK
				m_fLastPressure = 0.0f;
				m_tCursorPos = m_tLastPos = *a_pPos;
			}
			else
			{
				// continue drawing
				SSample s = {a_pPos->fX, a_pPos->fY, a_fNormalPressure};
				m_cSamples.push_back(s);
				while (true)
				{
					float fDX = a_pPos->fX-m_tLastPos.fX;
					float fDY = a_pPos->fY-m_tLastPos.fY;
					float fDP = a_fNormalPressure-m_fLastPressure;
					float fDist = sqrtf(fDX*fDX + fDY*fDY);
					if (fDist+m_fTrackLength < m_fDrawnPressure*fSize*0.25f)
					{
						m_tCursorPos = m_tLastPos = *a_pPos;
						m_fTrackLength += fDist;
						break;
					}
					float fLen = m_fDrawnPressure*fSize*0.25f-m_fTrackLength;
					TPixelCoords tPoint =
					{
						m_tLastPos.fX + fDX*fLen/fDist,
						m_tLastPos.fY + fDY*fLen/fDist
					};
					float fPressure = m_fLastPressure + fDP*fLen/fDist;
					if (m_cData.eShape == CEditToolDataBrush::ESCustom && (m_pBrushFrames.m_p || m_pMonoBrushFrames.m_p))
						DrawBrush(tPoint, fPressure, fSize);
					else
						DrawPattern(tPoint, fPressure, fSize);
					m_tCursorPos = m_tLastPos = tPoint;
					m_fLastPressure = fPressure;
					m_fTrackLength = 0.0f;
				}
				if (bEnd)
				{
					hRes = ETPAApply;
					m_fLastPressure = 0.0f;
					m_tCursorPos = m_tLastPos = *a_pPos;
				}
			}
		}
		if (bPosChange)
			M_Window()->ControlLinesChanged();
		return hRes;
	}

	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (!m_bMouseLeft) switch (m_cData.eShape)
		{
		case CEditToolDataBrush::ESRound:
			{
				int const nSteps = 12;
				float const fStep = 3.14159265f*2.0f/nSteps;
				TPixelCoords tCenter = m_tCursorPos;
				float fRadius = (m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize)*0.5f;
				a_pLines->MoveTo(tCenter.fX, tCenter.fY-fRadius);
				for (int i = 1; i < nSteps; ++i)
				{
					float fAngle = i*fStep;
					a_pLines->LineTo(tCenter.fX+fRadius*sinf(fAngle), tCenter.fY-fRadius*cosf(fAngle));
				}
				a_pLines->Close();
			}
			break;
		case CEditToolDataBrush::ESSquare:
			{
				TPixelCoords tCenter = m_tCursorPos;
				float fRadius = (m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize)*0.5f;
				a_pLines->MoveTo(tCenter.fX-fRadius, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius, tCenter.fY+fRadius);
				a_pLines->LineTo(tCenter.fX-fRadius, tCenter.fY+fRadius);
				a_pLines->Close();
			}
			break;
		case CEditToolDataBrush::ESDiagonal:
			{
				TPixelCoords tCenter = m_tCursorPos;
				float fRadius = (m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize)*0.5f;
				a_pLines->MoveTo(tCenter.fX+fRadius, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius*0.5f, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX-fRadius, tCenter.fY+fRadius*0.5f);
				a_pLines->LineTo(tCenter.fX-fRadius, tCenter.fY+fRadius);
				a_pLines->LineTo(tCenter.fX-fRadius*0.5f, tCenter.fY+fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius, tCenter.fY-fRadius*0.5f);
				a_pLines->Close();
			}
			break;
		case CEditToolDataBrush::ESCustom:
			{
				TPixelCoords tCenter = m_tCursorPos;
				float fRadius = (m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize)*0.5f;
				a_pLines->MoveTo(tCenter.fX-fRadius, tCenter.fY-fRadius*0.4f);
				a_pLines->LineTo(tCenter.fX-fRadius, tCenter.fY-fRadius*0.75f);
				a_pLines->LineTo(tCenter.fX-fRadius*0.75f, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX-fRadius*0.4f, tCenter.fY-fRadius);
				//a_pLines->Close();
				a_pLines->MoveTo(tCenter.fX+fRadius, tCenter.fY-fRadius*0.4f);
				a_pLines->LineTo(tCenter.fX+fRadius, tCenter.fY-fRadius*0.75f);
				a_pLines->LineTo(tCenter.fX+fRadius*0.75f, tCenter.fY-fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius*0.4f, tCenter.fY-fRadius);
				//a_pLines->Close();
				a_pLines->MoveTo(tCenter.fX-fRadius, tCenter.fY+fRadius*0.4f);
				a_pLines->LineTo(tCenter.fX-fRadius, tCenter.fY+fRadius*0.75f);
				a_pLines->LineTo(tCenter.fX-fRadius*0.75f, tCenter.fY+fRadius);
				a_pLines->LineTo(tCenter.fX-fRadius*0.4f, tCenter.fY+fRadius);
				//a_pLines->Close();
				a_pLines->MoveTo(tCenter.fX+fRadius, tCenter.fY+fRadius*0.4f);
				a_pLines->LineTo(tCenter.fX+fRadius, tCenter.fY+fRadius*0.75f);
				a_pLines->LineTo(tCenter.fX+fRadius*0.75f, tCenter.fY+fRadius);
				a_pLines->LineTo(tCenter.fX+fRadius*0.4f, tCenter.fY+fRadius);
				//a_pLines->Close();
			}
			break;
		}
		if (!m_bMouseLeft && (m_tCursorPos.fX != m_tLastPos.fX || m_tCursorPos.fY != m_tLastPos.fY))
		{
			a_pLines->MoveTo(m_tCursorPos.fX, m_tCursorPos.fY);
			a_pLines->LineTo(m_tLastPos.fX, m_tLastPos.fY);
		}

		return S_OK;
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return ETPAMissed|ETPAStartNew;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		if (a_pMatrix == NULL || memcmp(a_pMatrix, &TMATRIX3X3F_IDENTITY, sizeof(TMATRIX3X3F_IDENTITY)) == 0)
			return S_FALSE;
		RECT rc = M_DirtyRect();
		for (std::vector<SSample>::iterator i = m_cSamples.begin(); i != m_cSamples.end(); ++i)
		{
			TVector2f t = {i->x, i->y};
			t = TransformVector2(*a_pMatrix, t);
			i->x = t.x;
			i->y = t.y;
		}
		float fScaleX = 1.0f;
		float fScaleY = 1.0f;
		Matrix3x3fDecompose(*a_pMatrix, &fScaleX, &fScaleY, NULL, NULL);
		float const f = sqrtf(fScaleX*fScaleY);
		m_cData.fSize *= f;
		ULONG nX = 0;
		ULONG nY = 0;
		M_Window()->Size(&nX, &nY);
		InitImageTarget(nX, nY, true);
		RecomputeCache();
		RECT rc2 = M_DirtyRect();
		if (rc2.left > rc.left) rc2.left = rc.left;
		if (rc2.top > rc.top) rc2.top = rc.top;
		if (rc2.right < rc.right) rc2.right = rc.right;
		if (rc2.bottom < rc.bottom) rc2.bottom = rc.bottom;
		M_Window()->RectangleChanged(&rc2);
		M_Window()->ControlLinesChanged();
		{
			CComObject<CSharedStateToolData>* pNew = NULL;
			CComObject<CSharedStateToolData>::CreateInstance(&pNew);
			CComPtr<ISharedState> pTmp = pNew;
			pNew->Init(m_cData);
			M_Window()->SetState(pTmp);
		}
		return S_OK;
	}

	// IRasterImageEditToolScripting
public:
	STDMETHOD(FromText)(BSTR a_bstrParams)
	{
		try
		{
			if (a_bstrParams == NULL)
				return S_FALSE;
			CEditToolDataBrush::EShape eShape = CEditToolDataBrush::ESRound;
			bool bSize = false;
			bool bBlur = false;
			bool bFlow = false;
			float fSize = 11.0f;
			float fBlur = 0.2f;
			float fFlow = 1.0f;
			//float fScatter = 0.0f;
			//float fRotate = 1.0f;
			//float fResize = 0.0f;
			m_cData.bScatter = false;
			m_cData.bRotate = false;
			m_cData.bResize = false;
			std::wstring strBrushPath;
			std::vector<float> aCoord;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(a_bstrParams, a_bstrParams+SysStringLen(a_bstrParams),
					(((str_p(L"\"ROUND\"")[assign_a(eShape, CEditToolDataBrush::ESRound)]|str_p(L"\"SQUARE\"")[assign_a(eShape, CEditToolDataBrush::ESSquare)]|str_p(L"\"DIAGONAL\"")[assign_a(eShape, CEditToolDataBrush::ESDiagonal)]) >> cSep >>
					  real_p[assign_a(fSize)] >> (!(cSep >> str_p(L"\"NP\"")[assign_a(bSize, true)])) >> cSep >>
					  real_p[assign_a(fBlur)] >> (!(cSep >> str_p(L"\"NP\"")[assign_a(bBlur, true)])) >> cSep >>
					  real_p[assign_a(fFlow)] >> (!(cSep >> str_p(L"\"NP\"")[assign_a(bFlow, true)])))
					  |
				    (confix_p(L'"', (*c_escape_ch_p)[assign_a(strBrushPath)], L'"')[assign_a(eShape, CEditToolDataBrush::ESCustom)] >> cSep >> int_p[assign_a(m_nLastSeed)] >>
					  (!(cSep >> str_p(L"\"SCATTER\"")[assign_a(m_cData.bScatter, true)])) >>
					  (!(cSep >> str_p(L"\"ROTATE\"")[assign_a(m_cData.bRotate, true)])) >>
					  (!(cSep >> str_p(L"\"RESIZE\"")[assign_a(m_cData.bResize, true)])) >>
					  cSep >> real_p[assign_a(fSize)] >> (!(cSep >> str_p(L"\"NP\"")[assign_a(bSize, true)])) >>
					  cSep >> real_p[assign_a(fFlow)] >> (!(cSep >> str_p(L"\"NP\"")[assign_a(bFlow, true)]))
					  //real_p[assign_a(fScatter)] >> cSep >>
					  //real_p[assign_a(fRotate)] >> cSep >>
					  //real_p[assign_a(fResize)]
					 )) >>
					*(cSep >> real_p[push_back_a(aCoord)])
					).full;
			size_t nNrsPerPt = bSize || bBlur || bFlow ? 3 : 2;
			size_t nCoords = aCoord.size()/nNrsPerPt;
			if (!bParsed || nCoords < 2 || aCoord.size() != nCoords*nNrsPerPt)
				return E_INVALIDARG;
			m_cData.bBlur = bBlur;
			m_cData.bFlow = bFlow;
			m_cData.bSize = bSize;
			m_cData.fBlur = fBlur;
			m_cData.fFlow = fFlow;
			m_cData.fSize = fSize;
			m_cData.eShape = eShape;

			size_t nLen = strBrushPath.length();
			size_t iD = 0;
			size_t iS = 0;
			while (iS < nLen)
			{
				if (strBrushPath[iS] == L'\\')
					++iS;
				if (iS == nLen)
					break;
				strBrushPath[iD] = strBrushPath[iS];
				++iS;
				++iD;
			}
			if (iD != iS)
				strBrushPath.resize(iD);
			if (m_cData.strBrushPath != strBrushPath)
			{
				std::swap(m_cData.strBrushPath, strBrushPath);
				LoadBrush(m_cData.strBrushPath);
			}

			//m_cData.bScatter = fScatter >= 0.5f;
			//m_cData.bRotate = fRotate >= 0.5f;
			//m_cData.bResize = fResize >= 0.5f;
			TPixelCoords const tPS = {1.0f, 1.0f};
			TPixelCoords t;
			DWORD dw;
			UpdateCache();
			if (nNrsPerPt == 3)
			{
				// with pressure
				for (std::vector<float>::const_iterator i = aCoord.begin(); i != aCoord.end(); ++i)
				{
					t.fX = *i;
					++i;
					t.fY = *i;
					++i;
					ProcessInputEvent(ECKSNone, &t, &tPS, *i < 0.05f ? 0.05f : (*i > 1.0f ? 1.0f : *i), 0.0f, 0.0f, 0.0f, 0.0f, &dw);
				}
			}
			else
			{
				// without pressure
				for (std::vector<float>::const_iterator i = aCoord.begin(); i != aCoord.end(); ++i)
				{
					t.fX = *i;
					++i;
					t.fY = *i;
					ProcessInputEvent(ECKSNone, &t, &tPS, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, &dw);
				}
			}
			ProcessInputEvent(ECKSNone, &t, &tPS, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, &dw);
			{
				CComObject<CSharedStateToolData>* pNew = NULL;
				CComObject<CSharedStateToolData>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				pNew->Init(m_cData);
				M_Window()->SetState(pTmp);
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
		try
		{
			CComBSTR bstr;
			OLECHAR szBase[512];
			if (m_cData.eShape == CEditToolDataBrush::ESCustom)
			{
				_swprintf(szBase, L"\"%s\", %i%s%s%s, %g%s, %g%s",
				m_cData.strBrushPath.c_str(), m_nLastSeed,
					m_cData.bScatter ? L", \"SCATTER\"" : L"", m_cData.bRotate ? L", \"ROTATE\"" : L"",m_cData.bResize ? L", \"RESIZE\"" : L"",
					m_cData.fSize, m_cData.bSize ? L", \"NP\"" : L"", m_cData.fFlow, m_cData.bFlow ? L", \"NP\"" : L""
					);
			}
			else
			{
				_swprintf(szBase, L"\"%s\", %g%s, %g%s, %g%s",
					m_cData.eShape == CEditToolDataBrush::ESRound ? L"ROUND" : (m_cData.eShape == CEditToolDataBrush::ESSquare ? L"SQUARE" : L"DIAGONAL"),
					m_cData.fSize, m_cData.bSize ? L", \"NP\"" : L"", m_cData.fBlur, m_cData.bBlur ? L", \"NP\"" : L"", m_cData.fFlow, m_cData.bFlow ? L", \"NP\"" : L""
					);
			}
			if (m_cData.bSmooth)
			{
				return E_NOTIMPL;
			}
			else
			{
				CAutoVectorPtr<OLECHAR> coords(new OLECHAR[1+m_cSamples.size()*32]);
				OLECHAR* p = coords.m_p;
				if (m_cData.bSize || m_cData.bBlur || m_cData.bFlow)
				{
					for (std::vector<SSample>::const_iterator i = m_cSamples.begin(); i != m_cSamples.end(); ++i)
						p += _swprintf(p, L", %g, %g, %g", i->x, i->y, i->p);
				}
				else
				{
					for (std::vector<SSample>::const_iterator i = m_cSamples.begin(); i != m_cSamples.end(); ++i)
						p += _swprintf(p, L", %g, %g", i->x, i->y);
				}
				*p = L'\0';
				bstr.Attach(SysAllocStringLen(NULL, wcslen(szBase)+(p-coords.m_p)));
				wcscpy(bstr, szBase);
				wcscat(bstr, coords);
			}
			*a_pbstrParams = bstr.Detach();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	void RecomputeCache()
	{
		if (CEditToolPixelCoverageBuffer::IsDirty())
			CEditToolPixelCoverageBuffer::DeleteCachedData();
		if (m_cSamples.empty())
			return;
		float const fSize = m_cData.fSize < 0.1f ? 0.1f : m_cData.fSize;
		m_fTrackLength = 0.0f;
		if (m_nLastSeed == 0)
			m_nLastSeed = GetTickCount();
		m_cRand.RandomInit(m_nLastSeed);

		std::vector<SSample>::const_iterator i = m_cSamples.begin();
		m_fLastPressure = i->p;
		m_tLastPos.fX = i->x;
		m_tLastPos.fY = i->y;

		if (m_cData.eShape == CEditToolDataBrush::ESCustom && (m_pBrushFrames.m_p || m_pMonoBrushFrames.m_p))
			DrawBrush(m_tLastPos, i->p, fSize);
		else
			DrawPattern(m_tLastPos, i->p, fSize);

		for (++i; i != m_cSamples.end(); ++i)
		{

			while (true)
			{
				float fDX = i->x-m_tLastPos.fX;
				float fDY = i->y-m_tLastPos.fY;
				float fDP = i->p-m_fLastPressure;
				float fDist = sqrtf(fDX*fDX + fDY*fDY);
				if (fDist+m_fTrackLength < m_fDrawnPressure*fSize*0.25f)
				{
					m_tLastPos.fX = i->x;
					m_tLastPos.fY = i->y;
					m_fTrackLength += fDist;
					break;
				}
				float fLen = m_fDrawnPressure*fSize*0.25f-m_fTrackLength;
				TPixelCoords tPoint =
				{
					m_tLastPos.fX + fDX*fLen/fDist,
					m_tLastPos.fY + fDY*fLen/fDist
				};
				float fPressure = m_fLastPressure + fDP*fLen/fDist;
				if (m_cData.eShape == CEditToolDataBrush::ESCustom && (m_pBrushFrames.m_p || m_pMonoBrushFrames.m_p))
					DrawBrush(tPoint, fPressure, fSize);
				else
					DrawPattern(tPoint, fPressure, fSize);
				m_tLastPos = tPoint;
				m_fLastPressure = fPressure;
				m_fTrackLength = 0.0f;
			}
		}
		m_nLastSeed = 0;
		m_fLastPressure = 0.0f;
	}
	void DrawPattern(TPixelCoords const& a_tCoords, float const a_fPressure, float const a_fSize)
	{
		m_fDrawnPressure = m_cData.bSize ? a_fPressure : 1.0f;
		LONG const nX = a_tCoords.fX;
		LONG const nY = a_tCoords.fY;
		InitPattern(m_cData.bSize ? a_fSize*a_fPressure : a_fSize, m_cData.bBlur ? a_fPressure*(100.0f-m_cData.fBlur) : 100.0f-m_cData.fBlur, m_cData.bFlow ? m_cData.fFlow*a_fPressure : m_cData.fFlow, a_tCoords.fX-nX, a_tCoords.fY-nY);
		LONG const nY2 = nY+((m_szPattern.cy+1)>>1);
		LONG const nX1 = nX-((m_szPattern.cx-1)>>1);
		LONG const nX2 = nX+((m_szPattern.cx+1)>>1);
		BYTE const* pPattern = m_pPattern;
		for (LONG y = nY-((m_szPattern.cy-1)>>1); y < nY2; ++y)
		{
			for (LONG x = nX1; x < nX2; ++x)
			{
				MergePixel(x, y, *pPattern);
				++pPattern;
			}
		}
		RECT rc = {nX1, nY-((m_szPattern.cy-1)>>1), nX2, nY2};
		M_Window()->RectangleChanged(&rc);
	}
	static inline float GetCoverage(float a_fX, float a_fY, float a_fRad, float a_fBlurR, float a_fFact)
	{
		float const fR = sqrtf(a_fX*a_fX + a_fY*a_fY);
		if (fR > a_fRad)
			return 0.0f;
		if (fR <= a_fBlurR)
			return 1.0f;
		return (a_fRad-fR)*a_fFact;
	}
	void InitPattern(float a_fSize, float a_fBlur, float a_fFlow, float a_fOffsetX, float a_fOffsetY)
	{
		m_pPattern.Free();
		int nSize = 1+2*ceilf(a_fSize*0.5f-0.5f+max(fabsf(a_fOffsetX-0.5f), fabsf(a_fOffsetY)));
		int nRadEx = (nSize+1)>>1;
		float fCenterX = nRadEx-1.5f+a_fOffsetX;
		float fCenterY = nRadEx-1.5f+a_fOffsetY;
		m_pPattern.Allocate(nSize*nSize);
		m_szPattern.cx = m_szPattern.cy = nSize;
		float f = 255.0f/nRadEx;
		BYTE* pPat = m_pPattern;
		DWORD nMaxFlow = a_fFlow*0.01f*255.0f + 0.5f;
		switch (m_cData.eShape)
		{
		case CEditToolDataBrush::ESRound:
			{
				float fR = a_fSize*0.5f;
				float fR100 = max(0.0f, fR*a_fBlur*0.01f-0.5f);
				float fR0 = fR+0.5f;
				float fRSq100 = fR100*fR100;
				float fRSq0 = fR0*fR0;
				for (LONG y = 0; y < nSize; ++y)
				{
					for (LONG x = 0; x < nSize; ++x)
					{
						float fDistSq = (fCenterX-x)*(fCenterX-x) + (fCenterY-y)*(fCenterY-y);
						if (fDistSq <= fRSq100)
						{
							*pPat = nMaxFlow;
						}
						else if (fDistSq >= fRSq0)
						{
							*pPat = 0;
						}
						else
						{
							float fDist = sqrtf(fDistSq);
							*pPat = nMaxFlow-nMaxFlow*(fDist-fR100)/(fR0-fR100);
						}
						++pPat;
					}
				}
			}
			break;
		case CEditToolDataBrush::ESSquare:
			{
				float fR = a_fSize*0.5f;
				float fR100 = max(0.0f, fR*a_fBlur*0.01f-0.5f);
				float fR0 = fR+0.5f;
				for (LONG y = 0; y < nSize; ++y)
				{
					float fY;
					float fDist = fabsf(fCenterY-y);
					if (fDist <= fR100)
					{
						fY = 1.0f;
					}
					else if (fDist >= fR0)
					{
						fY = 0.0f;
					}
					else
					{
						fY = 1.0f-(fDist-fR100)/(fR0-fR100);
					}
					for (LONG x = 0; x < nSize; ++x)
					{
						float fDist = fabsf(fCenterX-x);
						if (fDist <= fR100)
						{
							*pPat = nMaxFlow*fY;
						}
						else if (fDist >= fR0)
						{
							*pPat = 0;
						}
						else
						{
							*pPat = nMaxFlow*(1.0f-(fDist-fR100)/(fR0-fR100))*fY;
						}
						++pPat;
					}
				}
			}
			break;
		case CEditToolDataBrush::ESDiagonal:
			{
				float fR = a_fSize*0.5f;
				float fR100 = max(0.0f, fR*a_fBlur*0.01f-0.5f);
				float fR0 = fR+0.5f;
				float fD = a_fSize*0.25f;
				float fD100 = max(0.0f, fD*a_fBlur*0.01f-0.78f);
				float fD0 = fD+0.78f;
				for (LONG y = 0; y < nSize; ++y)
				{
					float fY;
					float fDist = fabsf(fCenterY-y);
					if (fDist <= fR100)
					{
						fY = 1.0f;
					}
					else if (fDist >= fR0)
					{
						fY = 0.0f;
					}
					else
					{
						fY = 1.0f-(fDist-fR100)/(fR0-fR100);
					}
					for (LONG x = 0; x < nSize; ++x)
					{
						float fX;
						fDist = fabsf(fCenterX-x);
						if (fDist <= fR100)
						{
							fX = 1.0f;
						}
						else if (fDist >= fR0)
						{
							fX = 0.0f;
						}
						else
						{
							fX = 1.0f-(fDist-fR100)/(fR0-fR100);
						}
						fDist = fabsf(fCenterX-x-fCenterY+(nSize-1-y));
						if (fDist <= fD100)
						{
							*pPat = nMaxFlow*fY*fX;
						}
						else if (fDist >= fD0)
						{
							*pPat = 0;
						}
						else
						{
							*pPat = nMaxFlow*(1.0f-(fDist-fD100)/(fD0-fD100))*fY*fX;
						}
						++pPat;
					}
				}
			}
			break;
		default:
			FillMemory(m_pPattern.m_p, nSize*nSize, 100);
			break;
		}
	}


	// image brush
	static ULONG PerImageAllocSize(ULONG a_nX, ULONG a_nY)
	{
		ULONG nAlloc = 0;
		while (a_nX > 0 && a_nY > 0)
		{
			nAlloc += a_nX*a_nY;
			a_nX >>= 1;
			a_nY >>= 1;
		}
		return nAlloc;
	}
	void LoadBrush(std::wstring const& a_strBrushPath)
	{
		CComPtr<IInputManager> pIM;
		RWCoCreateInstance(pIM, __uuidof(InputManager));
		if (pIM == NULL) return;
		CComPtr<IDocument> pDoc;
		pIM->DocumentCreate(CStorageFilter(a_strBrushPath.c_str()), NULL, &pDoc);
		if (pDoc == NULL) return;
		CComPtr<IDocumentAnimation> pAnimation;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pAnimation));
		m_cBrushCumulativeTimes.clear();
		m_cBrushRelativeCoverage.clear();
		m_tBrushSize.cx = 0;
		m_tBrushSize.cy = 0;
		m_nBrushFrames = 0;
		m_nBrushLen = 0;
		m_pBrushFrames.Free();
		m_pMonoBrushFrames.Free();
		float fGamma = 0.0f;
		if (pAnimation)
		{
			ULONG nMaxX = 0;
			ULONG nMaxY = 0;
			std::vector<std::pair<TImageSize, CComPtr<IDocumentImage> > > cImages;
			CComPtr<IEnumUnknowns> pFrames;
			pAnimation->FramesEnum(&pFrames);
			ULONG nFrames = 0;
			if (pFrames) pFrames->Size(&nFrames);
			float fTotal = 0.0f;
			for (ULONG i = 0; i < nFrames; ++i)
			{
				CComPtr<IUnknown> pFrame;
				pFrames->Get(i, __uuidof(IUnknown), reinterpret_cast<void**>(&pFrame));
				std::pair<TImageSize, CComPtr<IDocumentImage> > t;
				t.first.nX = t.first.nY = 0;
				CComPtr<IDocument> pImgDoc;
				pAnimation->FrameGetDoc(pFrame, &pImgDoc);
				if (pImgDoc) pImgDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&t.second));
				if (t.second)
				{
					t.second->CanvasGet(&t.first, NULL, NULL, NULL, NULL);
					if (fGamma < 0.1f)
						t.second->ChannelsGet(NULL, &fGamma, NULL);
				}
				if (t.first.nX*t.first.nY == 0)
					continue;
				if (nMaxX < t.first.nX) nMaxX = t.first.nX;
				if (nMaxY < t.first.nY) nMaxY = t.first.nY;
				cImages.push_back(t);
				float fSecs = 0.1f;
				pAnimation->FrameGetTime(pFrame, &fSecs);
				fTotal += fSecs;
				m_cBrushCumulativeTimes.push_back(fTotal);
			}
			if (!cImages.empty())
			{
				ULONG const nImageLen = PerImageAllocSize(nMaxX, nMaxY);
				CAutoVectorPtr<TRasterImagePixel> cData(new TRasterImagePixel[cImages.size()*nImageLen]);
				m_nBrushFrames = cImages.size();
				m_nBrushLen = nImageLen;
				m_tBrushSize.cx = nMaxX;
				m_tBrushSize.cy = nMaxY;
				TImageSize const tSize = {nMaxX, nMaxY};
				TImageStride const tStride = {1, nMaxX};
				for (std::vector<std::pair<TImageSize, CComPtr<IDocumentImage> > >::const_iterator i = cImages.begin(); i != cImages.end(); ++i)
				{
					TImagePoint const tOrg = {LONG(i->first.nX-nMaxX)>>1, LONG(i->first.nY-nMaxY)>>1};
					i->second->TileGet(EICIRGBA, &tOrg, &tSize, &tStride, nMaxX*nMaxY, reinterpret_cast<TPixelChannel*>(cData.m_p+nImageLen*(i-cImages.begin())), NULL, EIRIPreview);
				}
				m_pBrushFrames.Attach(cData.Detach());
			}
		}
		else
		{
			CComPtr<IDocumentImage> pImage;
			pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pImage));
			if (pImage)
			{
				TImageSize tSize = {1, 1};
				pImage->CanvasGet(&tSize, NULL, NULL, NULL, NULL);
				pImage->ChannelsGet(NULL, &fGamma, NULL);
				ULONG const nBrushLen = PerImageAllocSize(tSize.nX, tSize.nY);
				CAutoVectorPtr<TRasterImagePixel> cData(new TRasterImagePixel[nBrushLen]);
				m_nBrushLen = nBrushLen;
				m_tBrushSize.cx = tSize.nX;
				m_tBrushSize.cy = tSize.nY;
				m_nBrushFrames = 1;
				pImage->TileGet(EICIRGBA, NULL, NULL, NULL, nBrushLen, reinterpret_cast<TPixelChannel*>(cData.m_p), NULL, EIRIPreview);
				m_pBrushFrames.Attach(cData.Detach());
				m_cBrushCumulativeTimes.push_back(1.0f);
			}
		}
		m_strBrushPath = a_strBrushPath;
		for (size_t i = 0; i < m_nBrushFrames; ++i)
		{
			// compute and save sum of alpha (total coverate of the image) to adjust flow later
			ULONG np0 = m_tBrushSize.cx;
			ULONG np1 = m_tBrushSize.cy;
			ULONG n0 = np0>>1;
			ULONG n1 = np1>>1;
			TRasterImagePixel* p = m_pBrushFrames+m_nBrushLen*i;
			while (n0 > 0 && n1 > 0)
			{
				CGammaResampling cRsmp(fGamma < 0.1f ? 2.2f : fGamma, n0, n1, np0, np1, p+np0*np1, p);
				cRsmp.Linear();
				p += np0*np1;
				np0 = n0;
				np1 = n1;
				n0 >>= 1;
				n1 >>= 1;
			}
			ULONG nTotal = 0;
			p = m_pBrushFrames+m_nBrushLen*i;
			for (TRasterImagePixel* const pEnd = p+m_tBrushSize.cx*m_tBrushSize.cy; p != pEnd; ++p)
				nTotal += p->bA;
			m_cBrushRelativeCoverage.push_back(sqrtf(float(nTotal/(255*3.1415*0.25*0.7*m_tBrushSize.cx*m_tBrushSize.cy))));
		}

		bool bAlphaOnly = true;
		TRasterImagePixel const* const pEnd = m_pBrushFrames.m_p+m_nBrushFrames*m_nBrushLen;
		for (TRasterImagePixel const* p = m_pBrushFrames; p != pEnd; ++p)
		{
			if (p->bR|p->bG|p->bB)
			{
				bAlphaOnly = false;
				break;
			}
		}
		if (bAlphaOnly)
		{
			m_pMonoBrushFrames.Allocate(m_nBrushFrames*m_nBrushLen);
			BYTE* pD = m_pMonoBrushFrames;
			for (TRasterImagePixel const* pS = m_pBrushFrames; pS != pEnd; ++pS, ++pD)
				*pD = pS->bA;
			m_pBrushFrames.Free();
			return;
		}

		bool bMonochrome = true;
		for (TRasterImagePixel const* p = m_pBrushFrames; p != pEnd; ++p)
		{
			if (p->bA != 0xff || p->bR != p->bG || p->bR != p->bB)
			{
				bMonochrome = false;
				break;
			}
		}
		if (bMonochrome)
		{
			m_pMonoBrushFrames.Allocate(m_nBrushFrames*m_nBrushLen);
			BYTE* pD = m_pMonoBrushFrames;
			for (TRasterImagePixel const* pS = m_pBrushFrames; pS != pEnd; ++pS, ++pD)
				*pD = pS->bR;
			m_pBrushFrames.Free();
		}
	}
	void DrawBrush(TPixelCoords const& a_tCoords, float const a_fPressure, float a_fSize)
	{
		m_fDrawnPressure = 300.0f/min(300.0f, max(m_cData.fFlow, 30.0f));
		float fAngle = m_cData.bRotate ? m_cRand.Random()*6.2831853f : 0.0f;
		float fDX = 0.0f;//(m_cRand.Random()-0.5)*0.5*a_fSize;
		float fDY = 0.0f;//(m_cRand.Random()-0.5)*0.5*a_fSize;
		if (m_cData.bScatter)
		{
			float fR = m_cRand.Random(); fR *= fR*0.5f*a_fSize;
			float fPhi = m_cRand.Random()*6.2831853f;
			fDX = cosf(fPhi)*fR;
			fDY = sinf(fPhi)*fR;
		}
		float fCover = 1.0f;
		if (m_cData.bResize)
		{
			fCover = m_cRand.Random()+0.5;
			a_fSize *= fCover;
		}
		ULONG nFrame = 0;
		if (m_cBrushCumulativeTimes.size() > 1)
		{
			float const fRandom = m_cRand.Random()*m_cBrushCumulativeTimes[m_cBrushCumulativeTimes.size()-1];
			while (nFrame+1 < m_nBrushFrames && fRandom > m_cBrushCumulativeTimes[nFrame]) ++nFrame;
		}
		fCover *= m_cBrushRelativeCoverage[nFrame];
		{
			// look on next frame and adjust fCover
			TRandomMotherOfAll cRand(m_cRand);
			if (m_cData.bRotate) cRand.Random();
			if (m_cData.bScatter) {cRand.Random(); cRand.Random();}
			float fCover2 = m_cData.bResize ? cRand.Random()+0.5 : 1.0f;
			ULONG nFrame2 = 0;
			float const fRandom = cRand.Random()*m_cBrushCumulativeTimes[m_cBrushCumulativeTimes.size()-1];
			while (nFrame2+1 < m_nBrushFrames && fRandom > m_cBrushCumulativeTimes[nFrame2]) ++nFrame2;
			fCover2 *= m_cBrushRelativeCoverage[nFrame2];
			fCover = (fCover+fCover2)*0.5f;
		}
		m_fDrawnPressure *= fCover;
		ULONG nSizeX = ceilf(a_fSize);
		ULONG nSizeY = nSizeX;
		if (m_tBrushSize.cx != m_tBrushSize.cy)
		{
			float f = sqrtf((a_fSize*a_fSize)/(m_tBrushSize.cx*m_tBrushSize.cy));
			nSizeX = ceilf(m_tBrushSize.cx*f);
			nSizeY = ceilf(m_tBrushSize.cy*f);
		}
		ULONG nBSX = m_tBrushSize.cx;
		ULONG nBSY = m_tBrushSize.cy;
		TRasterImagePixel const* pB = m_pBrushFrames+m_nBrushLen*nFrame;
		BYTE const* pMB = m_pMonoBrushFrames+m_nBrushLen*nFrame;
		while (nBSX > 1 && nBSY > 1 && nBSX*nBSY > nSizeX*nSizeY*6)
		{
			pB += nBSX*nBSY;
			pMB += nBSX*nBSY;
			nBSX >>= 1;
			nBSY >>= 1;
		}

		agg::trans_affine tr;
		tr = agg::trans_affine_translation(-a_tCoords.fX+fDX, -a_tCoords.fY+fDY);
		tr *= agg::trans_affine_rotation(fAngle/* * 3.1415926 / 180.0*/);
		tr *= agg::trans_affine_scaling(sqrtf((nBSX*nBSY)/(a_fSize*a_fSize)));
		tr *= agg::trans_affine_translation(nBSX*0.5, nBSY*0.5);
		RECT rc = {LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN};
		UpdateBounds(rc, tr, 0.0, 0.0);
		UpdateBounds(rc, tr, nBSX, 0.0);
		UpdateBounds(rc, tr, 0.0, nBSY);
		UpdateBounds(rc, tr, nBSX, nBSY);

		SIZE const tSize = {nBSX, nBSY};
		if (m_pBrushFrames.m_p)
			DrawSmooth(rc, tSize, pB, tr);
		else if (m_pMonoBrushFrames.m_p)
			DrawSmooth(rc, tSize, pMB, tr);

		M_Window()->RectangleChanged(&rc);
	}

	template<class TTransform>
	void UpdateBounds(RECT& rc, TTransform tr, double x, double y)
	{
		tr.inverse_transform(&x, &y);
		LONG nX1 = floor(x);
		LONG nX2 = ceil(x);
		if (rc.left > nX1) rc.left = nX1;
		if (rc.right < nX2) rc.right = nX2;
		LONG nY1 = floor(y);
		LONG nY2 = ceil(y);
		if (rc.top > nY1) rc.top = nY1;
		if (rc.bottom < nY2) rc.bottom = nY2;
	}

	class TRandomMotherOfAll // random number generator
	{
	public:
		TRandomMotherOfAll(unsigned int seed)
		{
			RandomInit(seed);
		}
		TRandomMotherOfAll(TRandomMotherOfAll const& a_rhs)
		{
			CopyMemory(x, a_rhs.x, sizeof x);
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

	template<class TTransform>
	void DrawSmooth(RECT const& a_rc, SIZE const a_szBrushSize, TRasterImagePixel const* a_pBrush, TTransform a_tr)
	{
		for (LONG y = a_rc.top; y < a_rc.bottom; ++y)
		{
			for (LONG x = a_rc.left; x < a_rc.right; ++x)
			{
				double xx = x+0.5;
				double yy = y+0.5;
				a_tr.transform(&xx, &yy);
				LONG xxx = LONG(xx+1.5)-2;
				LONG yyy = LONG(yy+1.5)-2;
				if (xxx >= -1 && xxx < a_szBrushSize.cx && yyy >= -1 && yyy < a_szBrushSize.cy)
				{
					ULONG nWX = 128*(xx-xxx-0.5);
					ULONG nWY = 128*(yy-yyy-0.5);
					ULONG nR = 0;
					ULONG nG = 0;
					ULONG nB = 0;
					ULONG nA = 0;
					ULONG nC = 0;
					TRasterImagePixel const* p;
					if (xxx >= 0)
					{
						if (xxx != a_szBrushSize.cx-1)
						{
							if (yyy >= 0)
							{
								if (yyy != a_szBrushSize.cy-1)
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const nW = (128-nWY)*(128-nWX);
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nW;
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
										ULONG const nW = nWY*(128-nWX);
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nW;
									}
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
										ULONG const nW = (128-nWY)*nWX;
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nW;
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
										ULONG const nW = nWY*nWX;
										ULONG const n = nW*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nW;
									}
								}
								else
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const n = ((128-nWX)<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*(128-nWX)*(128-nWY);
									}
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
										ULONG const n = (nWX<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nWX*(128-nWY);
									}
								}
							}
							else
							{
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
									ULONG const n = ((128-nWX)<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += 255*(128-nWX)*nWY;
								}
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
									ULONG const n = (nWX<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += 255*nWX*nWY;
								}
							}
						}
						else
						{
							if (yyy >= 0)
							{
								if (yyy != a_szBrushSize.cy-1)
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const n = ((128-nWY)<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*(128-nWY)*(128-nWX);
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
										ULONG const n = (nWY<<7)*p->bA;
										nA += n;
										nR += n*p->bR;
										nG += n*p->bG;
										nB += n*p->bB;
										nC += 255*nWY*(128-nWX);
									}
								}
								else
								{
									p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
									ULONG const n = ULONG(p->bA)<<14;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += 255*(128-nWX)*(128-nWY);
								}
							}
							else
							{
								p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += 255*(128-nWX)*nWY;
							}
						}
					}
					else
					{
						if (yyy >= 0)
						{
							if (yyy != a_szBrushSize.cy-1)
							{
								{
									p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
									ULONG const n = ((128-nWY)<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += 255*(128-nWY)*nWX;
								}
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
									ULONG const n = (nWY<<7)*p->bA;
									nA += n;
									nR += n*p->bR;
									nG += n*p->bG;
									nB += n*p->bB;
									nC += 255*nWY*nWX;
								}
							}
							else
							{
								p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
								ULONG const n = ULONG(p->bA)<<14;
								nA += n;
								nR += n*p->bR;
								nG += n*p->bG;
								nB += n*p->bB;
								nC += 255*nWX*(128-nWY);
							}
						}
						else
						{
							p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
							ULONG const n = ULONG(p->bA)<<14;
							nA += n;
							nR += n*p->bR;
							nG += n*p->bG;
							nB += n*p->bB;
							nC += 255*nWX*nWY;
						}
					}
					TRasterImagePixel t = {0, 0, 0, (nA>>14)*(nC>>14)/255};
					if (t.bA)
					{
						t.bR = (nR>>22)*255/(nA>>14);
						t.bG = (nG>>22)*255/(nA>>14);
						t.bB = (nB>>22)*255/(nA>>14);
						MergePixel(x, y, t);
					}
				}
			}
		}
	}

	template<class TTransform>
	void DrawSmooth(RECT const& a_rc, SIZE const a_szBrushSize, BYTE const* a_pBrush, TTransform a_tr)
	{
		for (LONG y = a_rc.top; y < a_rc.bottom; ++y)
		{
			for (LONG x = a_rc.left; x < a_rc.right; ++x)
			{
				double xx = x+0.5;
				double yy = y+0.5;
				a_tr.transform(&xx, &yy);
				LONG xxx = LONG(xx+1.5)-2;
				LONG yyy = LONG(yy+1.5)-2;
				if (xxx >= -1 && xxx < a_szBrushSize.cx && yyy >= -1 && yyy < a_szBrushSize.cy)
				{
					ULONG nWX = 128*(xx-xxx-0.5);
					ULONG nWY = 128*(yy-yyy-0.5);
					ULONG nA = 0;
					ULONG nC = 0;
					BYTE const* p;
					if (xxx >= 0)
					{
						if (xxx != a_szBrushSize.cx-1)
						{
							if (yyy >= 0)
							{
								if (yyy != a_szBrushSize.cy-1)
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const nW = (128-nWY)*(128-nWX);
										ULONG const n = nW**p;
										nA += n;
										nC += 255*nW;
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
										ULONG const nW = nWY*(128-nWX);
										ULONG const n = nW**p;
										nA += n;
										nC += 255*nW;
									}
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
										ULONG const nW = (128-nWY)*nWX;
										ULONG const n = nW**p;
										nA += n;
										nC += 255*nW;
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
										ULONG const nW = nWY*nWX;
										ULONG const n = nW**p;
										nA += n;
										nC += 255*nW;
									}
								}
								else
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const n = ((128-nWX)<<7)**p;
										nA += n;
										nC += 255*(128-nWX)*(128-nWY);
									}
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
										ULONG const n = (nWX<<7)**p;
										nA += n;
										nC += 255*nWX*(128-nWY);
									}
								}
							}
							else
							{
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
									ULONG const n = ((128-nWX)<<7)**p;
									nA += n;
									nC += 255*(128-nWX)*nWY;
								}
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
									ULONG const n = (nWX<<7)**p;
									nA += n;
									nC += 255*nWX*nWY;
								}
							}
						}
						else
						{
							if (yyy >= 0)
							{
								if (yyy != a_szBrushSize.cy-1)
								{
									{
										p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
										ULONG const n = ((128-nWY)<<7)**p;
										nA += n;
										nC += 255*(128-nWY)*(128-nWX);
									}
									{
										p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
										ULONG const n = (nWY<<7)**p;
										nA += n;
										nC += 255*nWY*(128-nWX);
									}
								}
								else
								{
									p = a_pBrush + yyy*a_szBrushSize.cx+xxx;
									ULONG const n = ULONG(*p)<<14;
									nA += n;
									nC += 255*(128-nWX)*(128-nWY);
								}
							}
							else
							{
								p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx;
								ULONG const n = ULONG(*p)<<14;
								nA += n;
								nC += 255*(128-nWX)*nWY;
							}
						}
					}
					else
					{
						if (yyy >= 0)
						{
							if (yyy != a_szBrushSize.cy-1)
							{
								{
									p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
									ULONG const n = ((128-nWY)<<7)**p;
									nA += n;
									nC += 255*(128-nWY)*nWX;
								}
								{
									p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
									ULONG const n = (nWY<<7)**p;
									nA += n;
									nC += 255*nWY*nWX;
								}
							}
							else
							{
								p = a_pBrush + yyy*a_szBrushSize.cx+xxx+1;
								ULONG const n = ULONG(*p)<<14;
								nA += n;
								nC += 255*nWX*(128-nWY);
							}
						}
						else
						{
							p = a_pBrush + (yyy+1)*a_szBrushSize.cx+xxx+1;
							ULONG const n = ULONG(*p)<<14;
							nA += n;
							nC += 255*nWX*nWY;
						}
					}
					ULONG const n = (nA>>14)*(nC>>14)/255;
					if (n)
						MergePixel(x, y, n);
				}
			}
		}
	}

private:
	TPixelCoords m_tLastPos;
	float m_fLastPressure;
	float m_fDrawnPressure;
	float m_fTrackLength;
	EBlendingMode m_eBlendingMode;
	CEditToolDataBrush m_cData;
	bool m_bMouseLeft;
	TPixelCoords m_tCursorPos;

	CAutoVectorPtr<BYTE> m_pPattern;
	SIZE m_szPattern;

	std::wstring m_strBrushPath;
	SIZE m_tBrushSize;
	ULONG m_nBrushFrames;
	ULONG m_nBrushLen;
	CAutoVectorPtr<TRasterImagePixel> m_pBrushFrames;
	CAutoVectorPtr<BYTE> m_pMonoBrushFrames;
	ULONG m_nLastSeed;
	TRandomMotherOfAll m_cRand;
	std::vector<float> m_cBrushCumulativeTimes;
	std::vector<float> m_cBrushRelativeCoverage;

	struct SSample { float x, y, p; };
	std::vector<SSample> m_cSamples;
};

