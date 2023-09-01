
#include "stdafx.h"
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <PrintfLocalizedString.h>
#include <RWBaseEnumUtils.h>
#include <RWDocumentImageVector.h>
#if __has_include (<RWDocumentImageRendered.h>)
#define WITH_RENDERED
#include <RWDocumentImageRendered.h>
#endif
#include <RWDocumentAnimation.h>
#include <RWDocumentAnimationUtils.h>

#include <GammaCorrection.h>


static OLECHAR const CFGID_SELSYNC[] = L"FrameSyncID";
static OLECHAR const CFGID_STEPS[] = L"Steps";

#include <ConfigCustomGUIImpl.h>

class ATL_NO_VTABLE CConfigGUIMorphDlg :
	public CCustomConfigResourcelessWndImpl<CConfigGUIMorphDlg>,
	public CDialogResize<CConfigGUIMorphDlg>
{
public:
	enum
	{
		IDC_SYNCID = 100,
		IDC_STEPS,
		IDC_STEPS_UD,
	};

	BEGIN_DIALOG_EX(0, 0, 120, (M_Mode() == ECPMFull ? 28 : 12), 0)
		DIALOG_FONT_AUTO()
		DIALOG_STYLE(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|DS_CONTROL)
		DIALOG_EXSTYLE(0)
	END_DIALOG()

	BEGIN_CONTROLS_MAP()
	if (M_Mode() == ECPMFull)
	{
		CONTROL_LTEXT(_T("[0409]&Selection sync ID:[0405]Synchronizace výběru:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_SYNCID, 50, 0, 70, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, 0)
		CONTROL_LTEXT(_T("[0409]Steps:[0405]Kroky:"), IDC_STATIC, 0, 18, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_STEPS, 50, 16, 70, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_STEPS_UD, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 16, 10, 12, 0)
	}
	else
	{
		CONTROL_LTEXT(_T("[0409]Steps:[0405]Kroky:"), IDC_STATIC, 0, 2, 50, 8, WS_VISIBLE, 0)
		CONTROL_EDITTEXT(IDC_STEPS, 50, 0, 70, 12, WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT, 0)
		CONTROL_CONTROL(_T(""), IDC_STEPS_UD, UPDOWN_CLASS, UDS_ALIGNRIGHT | UDS_AUTOBUDDY | UDS_ARROWKEYS | UDS_NOTHOUSANDS | WS_VISIBLE, 108, 0, 10, 12, 0)
	}
	END_CONTROLS_MAP()

	BEGIN_MSG_MAP(CConfigGUIMorphDlg)
		CHAIN_MSG_MAP(CDialogResize<CConfigGUIMorphDlg>)
		CHAIN_MSG_MAP(CCustomConfigResourcelessWndImpl<CConfigGUIMorphDlg>)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	const _AtlDlgResizeMap* GetDlgResizeMap()
	{
		static const _AtlDlgResizeMap theMap[] =
		{
		DLGRESIZE_CONTROL(IDC_SYNCID, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STEPS, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_STEPS_UD, DLSZ_MOVE_X)
			{ -1, 0 },
		};
		return M_Mode() == ECPMFull ? theMap : theMap+1;
	}

	SCustomConfigControlMap const* GetCustomConfigControlMap()
	{
		static SCustomConfigControlMap const sMap[] =
		{
		CONFIGITEM_EDITBOX(IDC_SYNCID, CFGID_SELSYNC)
		CONFIGITEM_EDITBOX(IDC_STEPS, CFGID_STEPS)
		{ ECCCTInvalid, 0}
		};
		return M_Mode() == ECPMFull ? sMap : sMap+1;
	}

	LRESULT OnInitDialog(UINT UNREF(a_uMsg), WPARAM UNREF(a_wParam), LPARAM UNREF(a_lParam), BOOL& a_bHandled)
	{
		DlgResize_Init(false, false, 0);

		return 1;
	}
};


// CAnimationMorph

// {A89E52F8-A093-48cb-B0CD-4F00C0622244}
extern GUID const CLSID_AnimationMorph = {0xa89e52f8, 0xa093, 0x48cb, {0xb0, 0xcd, 0x4f, 0x0, 0xc0, 0x62, 0x22, 0x44}};

class ATL_NO_VTABLE CAnimationMorph :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CAnimationMorph, &CLSID_AnimationMorph>,
	public IDocumentOperation
{
public:
	CAnimationMorph()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CAnimationMorph)

BEGIN_CATEGORY_MAP(CAnimationMorph)
	IMPLEMENTED_CATEGORY(CATID_DocumentOperation)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CAnimationMorph)
	COM_INTERFACE_ENTRY(IDocumentOperation)
END_COM_MAP()


	// IDocumentOperation methods
public:
	STDMETHOD(NameGet)(IOperationManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL)
			return E_UNEXPECTED;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Animation - Morph[0405]Animace - morfování");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IOperationManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		try
		{
			*a_ppDefaultConfig = NULL;

			CComPtr<IConfigWithDependencies> pCfgInit;
			RWCoCreateInstance(pCfgInit, __uuidof(ConfigWithDependencies));

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_SELSYNC), CMultiLanguageString::GetAuto(L"[0409]Selection ID[0405]ID výběru"), NULL, CConfigValue(L"FRAME"), NULL, 0, NULL);

			pCfgInit->ItemInsSimple(CComBSTR(CFGID_STEPS), CMultiLanguageString::GetAuto(L"[0409]Steps[0405]Kroky"), NULL, CConfigValue(5L), NULL, 0, NULL);

			// finalize the initialization of the config
			CConfigCustomGUI<&CLSID_AnimationMorph, CConfigGUIMorphDlg>::FinalizeConfig(pCfgInit);

			*a_ppDefaultConfig = pCfgInit.Detach();

			return S_OK;
		}
		catch (...)
		{
			return a_ppDefaultConfig == NULL ? E_POINTER : E_UNEXPECTED;
		}
	}
	STDMETHOD(CanActivate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates)
	{
		try
		{
			if (a_pDocument == NULL)
				return S_FALSE;
			CComPtr<IDocumentAnimation> pDA;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDA));
			if (pDA != NULL)
				return S_OK;
			return S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Activate)(IOperationManager* a_pManager, IDocument* a_pDocument, IConfig* a_pConfig, IOperationContext* a_pStates, RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IDocumentAnimation> pDA;
			a_pDocument->QueryFeatureInterface(__uuidof(IDocumentAnimation), reinterpret_cast<void**>(&pDA));
			if (pDA == NULL)
				return E_FAIL;

			CConfigValue cSelID;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_SELSYNC), &cSelID);
			CConfigValue cSteps;
			a_pConfig->ItemValueGet(CComBSTR(CFGID_STEPS), &cSteps);

			CComBSTR bstrSyncID;
			pDA->StatePrefix(&bstrSyncID);
			if (bstrSyncID.Length())
			{
				bstrSyncID += cSelID.operator BSTR();
			}
			else
			{
				bstrSyncID.Attach(cSelID.Detach().bstrVal);
			}

			CComPtr<ISharedState> pState;
			a_pStates->StateGet(bstrSyncID, __uuidof(ISharedState), reinterpret_cast<void**>(&pState));

			CWriteLock<IDocument> cLock(a_pDocument);

			CComPtr<IEnumUnknowns> pSel;
			pDA->StateUnpack(pState, &pSel);

			ULONG nSel = 0;
			if (pSel) pSel->Size(&nSel);

			if (nSel == 0 || nSel > 2)
				return E_FAIL;

			CComPtr<IUnknown> p1st;
			pSel->Get(0, &p1st);
			CComPtr<IUnknown> p2nd;
			pSel->Get(1, &p2nd);

			CComPtr<IEnumUnknowns> pAll;
			pDA->FramesEnum(&pAll);
			ULONG nAll = 0;
			if (pAll) pAll->Size(&nAll);
			ULONG n1st = nAll;
			for (ULONG i = 0; i < nAll; ++i)
			{
				CComPtr<IUnknown> p;
				pAll->Get(i, &p);
				if (p.p == p1st.p)
				{
					n1st = i;
					break;
				}
				else if (p.p == p2nd.p)
				{
					n1st = i;
					std::swap(p1st.p, p2nd.p);
					break;
				}
			}
			if (n1st+1 >= nAll)
				return E_FAIL;
			if (p2nd)
			{
				CComPtr<IUnknown> p;
				pAll->Get(n1st+1, &p);
				if (p.p != p2nd.p)
					return E_FAIL;
			}
			else
			{
				pAll->Get(n1st+1, &p2nd);
			}
			CComPtr<IDocument> pDoc1;
			pDA->FrameGetDoc(p1st, &pDoc1);
			CComPtr<IDocument> pDoc2;
			pDA->FrameGetDoc(p2nd, &pDoc2);

			for (LONG f = 0; f < cSteps.operator LONG(); ++f)
			{
				CComPtr<IUnknown> pNew;
				pDA->FrameIns(p2nd, &CAnimationFrameCreatorDocument(pDoc1, true), &pNew);
				CComPtr<IDocument> pNewDoc;
				pDA->FrameGetDoc(pNew, &pNewDoc);
				float fPhase = (f+1.0f)/(cSteps.operator LONG()+1);
				MorphStep(pNewDoc, pDoc2, fPhase);
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	static void ParamListSplit(wchar_t const* psz, std::vector<wchar_t const*>& aTokens)
	{
		aTokens.push_back(psz);
		bool bDoubleQuotes = false;
		while (*psz)
		{
			if (bDoubleQuotes)
			{
				if (*psz == L'\"')
					bDoubleQuotes = false;
			}
			else if (*psz == L',')
			{
				aTokens.push_back(psz);
			}
			++psz;
		}
		aTokens.push_back(psz);
	}
	static bool IsNumber(wchar_t const* begin, wchar_t const* end, double& val)
	{
		if (begin == end)
			return false;
		if (*begin == L',')
			++begin;
		while (begin != end && *begin == L' ')
			++begin;
		if (begin == end)
			return false;
		wchar_t* pEnd = const_cast<wchar_t*>(begin);
		val = wcstod(begin, &pEnd); // assuming zero ended or non-numeric character ended string)
		if (pEnd == begin)
			return false;
		if (pEnd >= end)
			return true;
		while (pEnd != end && *pEnd == L' ')
			++pEnd;
		return pEnd == end;
	}
	static bool MorphParamList(wchar_t const* psz1, wchar_t const* psz2, float fWeight2, CComBSTR& bstrIterpolated)
	{
		std::vector<wchar_t const*> aTokens1;
		std::vector<wchar_t const*> aTokens2;
		ParamListSplit(psz1, aTokens1);
		ParamListSplit(psz2, aTokens2);
		if (aTokens1.size() != aTokens2.size())
			return false;
		std::vector<wchar_t> cOut;
		for (size_t i = 0; i < aTokens1.size()-1; ++i)
		{
			double v1;
			double v2;
			bool b1 = IsNumber(aTokens1[i], aTokens1[i+1], v1);
			bool b2 = IsNumber(aTokens2[i], aTokens2[i+1], v2);
			if (b1 != b2)
				return false;
			if (b1 && v1 != v2)
			{
				wchar_t sz[32];
				swprintf(sz, i == 0 ? L"%g" : L", %g", v1*(1.0-fWeight2)+v2*fWeight2);
				cOut.insert(cOut.end(), sz, sz+wcslen(sz));
			}
			else
			{
				cOut.insert(cOut.end(), aTokens1[i], aTokens1[i+1]);
			}
		}
		if (!cOut.empty())
			bstrIterpolated.Attach(SysAllocStringLen(&cOut[0], cOut.size()));
		return true;
	}

	static bool MorphConfigs(IConfig* pEffect1, IConfig* pEffect2, float fWeight2)
	{
		CComPtr<IEnumStrings> pES1;
		CComPtr<IEnumStrings> pES2;
		pEffect1->ItemIDsEnum(&pES1);
		pEffect2->ItemIDsEnum(&pES2);
		ULONG nES1 = 0;
		ULONG nES2 = 0;
		if (pES1) pES1->Size(&nES1);
		if (pES2) pES2->Size(&nES2);
		if (nES1 != nES2 || nES1 == 0)
			return false;
		std::vector<BSTR> aIDs;
		std::vector<TConfigValue> aVals;
		aIDs.reserve(nES1);
		aVals.reserve(nES1);
		bool bApply = true;
		try
		{
			float const fWeight1 = 1.0f-fWeight2;
			for (ULONG i = 0; i < nES1; ++i)
			{
				CComBSTR bstr1;
				CComBSTR bstr2;
				pES1->Get(i, &bstr1);
				pES2->Get(i, &bstr2);
				if (bstr1 != bstr2)
				{
					bApply = false;
					break;
				}
				CConfigValue cVal1;
				CConfigValue cVal2;
				pEffect1->ItemValueGet(bstr1, &cVal1);
				pEffect2->ItemValueGet(bstr2, &cVal2);
				if (cVal1 == cVal2)
					continue;
				if (cVal1.TypeGet() != ECVTInteger && cVal1.TypeGet() != ECVTFloat &&
					cVal1.TypeGet() != ECVTVector2 && cVal1.TypeGet() != ECVTVector3 &&
					cVal1.TypeGet() != ECVTVector4 && cVal1.TypeGet() != ECVTFloatColor)
					continue; // cannot interpolate other types
				CComPtr<IConfigItemOptions> pOptions;
				pEffect1->ItemGetUIInfo(bstr1, __uuidof(IConfigItemOptions), reinterpret_cast<void**>(&pOptions));
				if (pOptions)
					continue; // better not interpolate enums
				TConfigValue tVal1 = cVal1;
				TConfigValue const& tVal2 = cVal2;
				switch (cVal1.TypeGet())
				{
				case ECVTVector4:
					tVal1.vecVal[3] = tVal1.vecVal[3]*fWeight1 + tVal2.vecVal[3]*fWeight2;
				case ECVTVector3:
				case ECVTFloatColor:
					tVal1.vecVal[2] = tVal1.vecVal[2]*fWeight1 + tVal2.vecVal[2]*fWeight2;
				case ECVTVector2:
					tVal1.vecVal[1] = tVal1.vecVal[1]*fWeight1 + tVal2.vecVal[1]*fWeight2;
				case ECVTFloat:
					tVal1.fVal = tVal1.fVal*fWeight1 + tVal2.fVal*fWeight2;
					break;
				case ECVTInteger:
					tVal1.iVal = LONG(tVal1.iVal*fWeight1 + tVal2.iVal*fWeight2 + 0.5f);
					break;
				}
				aIDs.push_back(bstr1.Detach());
				aVals.push_back(tVal1);
			}
		}
		catch (...)
		{
			bApply = false;
		}
		bool bRet = false;
		if (bApply && !aIDs.empty())
		{
			pEffect1->ItemValuesSet(aIDs.size(), &(aIDs[0]), &(aVals[0]));
			bRet = true;
		}
		for (std::vector<BSTR>::iterator i = aIDs.begin(); i != aIDs.end(); ++i)
			SysFreeString(*i);
		for (std::vector<TConfigValue>::iterator i = aVals.begin(); i != aVals.end(); ++i)
			ConfigValueClear(*i);
		return bRet;
	}

#ifdef WITH_RENDERED
	static bool MorphStep(IDocumentRenderedImage* pDoc1, IDocumentRenderedImage* pDoc2, float fWeight2)
	{
		float fWeight1 = 1.0f-fWeight2;
		TVector3f tPosition1;
		TVector3f tPosition2;
		TQuaternionf tRotation1;
		TQuaternionf tRotation2;
		float fZoom1;
		float fZoom2;
		pDoc1->CameraGetView(&tPosition1, &tRotation1, &fZoom1);
		pDoc2->CameraGetView(&tPosition2, &tRotation2, &fZoom2);
		if (tPosition1.x != tPosition2.x || tPosition1.y != tPosition2.y || tPosition1.z != tPosition2.z ||
			tRotation1.fX != tRotation2.fX || tRotation1.fY != tRotation2.fY || tRotation1.fZ != tRotation2.fZ || tRotation1.fW != tRotation2.fW ||
			fZoom1 != fZoom2)
		{
			tPosition1.x = tPosition1.x*fWeight1 + tPosition2.x*fWeight2;
			tPosition1.y = tPosition1.y*fWeight1 + tPosition2.y*fWeight2;
			tPosition1.z = tPosition1.z*fWeight1 + tPosition2.z*fWeight2;
			tRotation1.fX = tRotation1.fX*fWeight1 + tRotation2.fX*fWeight2;
			tRotation1.fY = tRotation1.fY*fWeight1 + tRotation2.fY*fWeight2;
			tRotation1.fZ = tRotation1.fZ*fWeight1 + tRotation2.fZ*fWeight2;
			tRotation1.fW = tRotation1.fW*fWeight1 + tRotation2.fW*fWeight2;
			float const f = 1.0f/sqrtf(tRotation1.fX*tRotation1.fX + tRotation1.fY*tRotation1.fY + tRotation1.fZ*tRotation1.fZ + tRotation1.fW*tRotation1.fW);
			tRotation1.fX *= f;
			tRotation1.fY *= f;
			tRotation1.fZ *= f;
			tRotation1.fW *= f;
			fZoom1 = fZoom1*fWeight1 + fZoom2*fWeight2;
			pDoc1->CameraSetView(&tPosition1, &tRotation1, &fZoom1);
		}
		float fFOV1;
		float fFOV2;
		float fNear1;
		float fNear2;
		float fFar1;
		float fFar2;
		TVector2f tCenter1;
		TVector2f tCenter2;
		pDoc1->CameraGetProj(&fFOV1, &fNear1, &fFar1, &tCenter1);
		pDoc2->CameraGetProj(&fFOV2, &fNear2, &fFar2, &tCenter2);
		if (fFOV1 != fFOV2 || fNear1 != fNear2 || fFar1 != fFar2 || tCenter1.x != tCenter2.x || tCenter1.y != tCenter2.y)
		{
			fFOV1 = fFOV1*fWeight1 + fFOV2*fWeight2;
			fNear1 = fNear1*fWeight1 + fNear2*fWeight2;
			fFar1 = fFar1*fWeight1 + fFar2*fWeight2;
			tCenter1.x = tCenter1.x*fWeight1 + tCenter2.x*fWeight2;
			tCenter1.y = tCenter1.y*fWeight1 + tCenter2.y*fWeight2;
			pDoc1->CameraSetProj(&fFOV1, &fNear1, &fFar1, &tCenter1);
		}
		std::vector<ULONG> aIDs1;
		std::vector<ULONG> aIDs2;
		pDoc1->ObjectsEnum(CEnumToVector<IEnum2UInts, ULONG>(aIDs1));
		pDoc2->ObjectsEnum(CEnumToVector<IEnum2UInts, ULONG>(aIDs2));
		if (aIDs1.size() != aIDs2.size())
			return false;
		for (size_t i = 0; i < aIDs1.size(); ++i)
		{
			pDoc1->ObjectTransformGet(aIDs1[i], &tPosition1, &tRotation1, &fZoom1);
			pDoc2->ObjectTransformGet(aIDs2[i], &tPosition2, &tRotation2, &fZoom2);
			if (tPosition1.x != tPosition2.x || tPosition1.y != tPosition2.y || tPosition1.z != tPosition2.z ||
				tRotation1.fX != tRotation2.fX || tRotation1.fY != tRotation2.fY || tRotation1.fZ != tRotation2.fZ || tRotation1.fW != tRotation2.fW ||
				fZoom1 != fZoom2)
			{
				tPosition1.x = tPosition1.x*fWeight1 + tPosition2.x*fWeight2;
				tPosition1.y = tPosition1.y*fWeight1 + tPosition2.y*fWeight2;
				tPosition1.z = tPosition1.z*fWeight1 + tPosition2.z*fWeight2;
				tRotation1.fX = tRotation1.fX*fWeight1 + tRotation2.fX*fWeight2;
				tRotation1.fY = tRotation1.fY*fWeight1 + tRotation2.fY*fWeight2;
				tRotation1.fZ = tRotation1.fZ*fWeight1 + tRotation2.fZ*fWeight2;
				tRotation1.fW = tRotation1.fW*fWeight1 + tRotation2.fW*fWeight2;
				float const f = 1.0f/sqrtf(tRotation1.fX*tRotation1.fX + tRotation1.fY*tRotation1.fY + tRotation1.fZ*tRotation1.fZ + tRotation1.fW*tRotation1.fW);
				tRotation1.fX *= f;
				tRotation1.fY *= f;
				tRotation1.fZ *= f;
				tRotation1.fW *= f;
				fZoom1 = fZoom1*fWeight1 + fZoom2*fWeight2;
				pDoc1->ObjectTransformSet(aIDs1[i], &tPosition1, &tRotation1, &fZoom1);
			}
			CComPtr<IConfig> pEffect1;
			CComPtr<IConfig> pEffect2;
			pDoc1->ObjectConfigGet(aIDs1[i], &pEffect1);
			pDoc2->ObjectConfigGet(aIDs2[i], &pEffect2);
			if (pEffect1 == NULL || pEffect2 == NULL)
				continue;
			if (MorphConfigs(pEffect1, pEffect2, fWeight2))
				pDoc1->ObjectConfigSet(aIDs1[i], pEffect1);
		}
		return true;
	}
#endif

	static bool MorphStep(IDocumentVectorImage* pDoc1, IDocumentVectorImage* pDoc2, float fWeight2)
	{
		std::vector<ULONG> aIDs1;
		std::vector<ULONG> aIDs2;
		pDoc1->ObjectIDs(CEnumToVector<IEnum2UInts, ULONG>(aIDs1));
		pDoc2->ObjectIDs(CEnumToVector<IEnum2UInts, ULONG>(aIDs2));
		if (aIDs1.size() != aIDs2.size())
			return false;
		for (size_t i = 0; i < aIDs1.size(); ++i)
		{
			CComBSTR bstrID1;
			CComBSTR bstrID2;
			CComBSTR bstrParams1;
			CComBSTR bstrParams2;
			pDoc1->ObjectGet(aIDs1[i], &bstrID1, &bstrParams1);
			pDoc2->ObjectGet(aIDs2[i], &bstrID2, &bstrParams2);
			if (bstrID1 != bstrID2)
				return false;
			CComBSTR bstrIterpolated;
			if (!MorphParamList(bstrParams1, bstrParams2, fWeight2, bstrIterpolated))
				return false;
			if (bstrIterpolated != bstrParams1)
				pDoc1->ObjectSet(&aIDs1[i], bstrID1, bstrIterpolated);

			CComBSTR bstrSID1;
			CComBSTR bstrSID2;
			CComBSTR bstrSParams1;
			CComBSTR bstrSParams2;
			pDoc1->ObjectStyleGet(aIDs1[i], &bstrSID1, &bstrSParams1);
			pDoc2->ObjectStyleGet(aIDs2[i], &bstrSID2, &bstrSParams2);
			if (bstrSID1 == bstrSID2)
			{
				CComBSTR bstrSIterpolated;
				if (MorphParamList(bstrSParams1, bstrSParams2, fWeight2, bstrSIterpolated) && bstrSIterpolated != bstrSParams1)
					pDoc1->ObjectStyleSet(aIDs1[i], bstrSID1, bstrSIterpolated);
			}

			TColor tOutlineColor1;
			TColor tOutlineColor2;
			float fOutlineWidth1;
			float fOutlineWidth2;
			float fOutlinePos1;
			float fOutlinePos2;
			pDoc1->ObjectStateGet(aIDs1[i], NULL, NULL, NULL, NULL, &tOutlineColor1, &fOutlineWidth1, &fOutlinePos1, NULL);
			pDoc2->ObjectStateGet(aIDs2[i], NULL, NULL, NULL, NULL, &tOutlineColor2, &fOutlineWidth2, &fOutlinePos2, NULL);
			if (tOutlineColor1.fR != tOutlineColor2.fR || tOutlineColor1.fG != tOutlineColor2.fG ||
				tOutlineColor1.fB != tOutlineColor2.fB || tOutlineColor1.fA != tOutlineColor2.fA ||
				fOutlineWidth1 != fOutlineWidth2 || fOutlinePos1 != fOutlinePos2)
			{
				float fWeight1 = 1.0f-fWeight2;
				float fNewA = tOutlineColor1.fA*fWeight1+tOutlineColor2.fA*fWeight2;
				if (fNewA <= 0.0f)
				{
					tOutlineColor1.fR = tOutlineColor1.fG = tOutlineColor1.fB = tOutlineColor1.fA = 0.0f;
				}
				else
				{
					tOutlineColor1.fR = (tOutlineColor1.fR*tOutlineColor1.fA*fWeight1 + tOutlineColor2.fR*tOutlineColor2.fA*fWeight2)/fNewA;
					tOutlineColor1.fG = (tOutlineColor1.fG*tOutlineColor1.fA*fWeight1 + tOutlineColor2.fG*tOutlineColor2.fA*fWeight2)/fNewA;
					tOutlineColor1.fB = (tOutlineColor1.fB*tOutlineColor1.fA*fWeight1 + tOutlineColor2.fB*tOutlineColor2.fA*fWeight2)/fNewA;
					tOutlineColor1.fA = fNewA;
				}
				fOutlineWidth1 = fOutlineWidth1*fWeight1+fOutlineWidth2*fWeight2;
				fOutlinePos1 = fOutlinePos1*fWeight1+fOutlinePos2*fWeight2;
				pDoc1->ObjectStateSet(aIDs1[i], NULL, NULL, NULL, NULL, &tOutlineColor1, &fOutlineWidth1, &fOutlinePos1, NULL);
			}
		}
		return true;
	}

	static bool MorphStep(IDocumentRasterImage* pDoc1, IDocumentRasterImage* pDoc2, float fWeight2)
	{
		TImagePoint tOrigin1 = {0, 0};
		TImagePoint tOrigin2 = {0, 0};
		TImageSize tSize1 = {0, 0};
		TImageSize tSize2 = {0, 0};
		pDoc1->CanvasGet(NULL, NULL, &tOrigin1, &tSize1, NULL);
		pDoc2->CanvasGet(NULL, NULL, &tOrigin2, &tSize2, NULL);
		TImagePoint const tOrigin = {min(tOrigin1.nX, tOrigin2.nX), min(tOrigin1.nY, tOrigin2.nY)};
		TImagePoint const tEnd = {max(LONG(tOrigin1.nX+tSize1.nX), LONG(tOrigin2.nX+tSize2.nX)), max(LONG(tOrigin1.nY+tSize1.nY), LONG(tOrigin2.nY+tSize2.nY))};
		TImageSize const tSize = {tEnd.nX-tOrigin.nX, tEnd.nY-tOrigin.nY};
		ULONG const nPixels = tSize.nX*tSize.nY;
		if (nPixels == 0)
			return true;
		CAutoVectorPtr<TPixelChannel> pBuffer1(new TPixelChannel[nPixels*2]);
		TPixelChannel* pBuffer2 = pBuffer1+nPixels;
		pDoc1->TileGet(EICIRGBA, &tOrigin, &tSize, NULL, nPixels, pBuffer1, NULL, EIRIAccurate);
		pDoc2->TileGet(EICIRGBA, &tOrigin, &tSize, NULL, nPixels, pBuffer2, NULL, EIRIAccurate);
		{
			CComPtr<IGammaTableCache> pGTC;
			RWCoCreateInstance(pGTC, __uuidof(GammaTableCache));
			CGammaTables const*	pGT = pGTC->GetSRGBTable();
			ULONG const nW2 = ULONG(fWeight2*256.0f+0.5f);
			ULONG const nW1 = 256-nW2;
			TPixelChannel* pD = pBuffer1;
			TPixelChannel const* pS = pBuffer2;
			for (TPixelChannel* const pEnd = pD+nPixels; pD != pEnd; ++pD, ++pS)
			{
				ULONG const nA1 = pD->bA*nW1;
				ULONG const nR1 = pGT->m_aGamma[pD->bR]*nA1;
				ULONG const nG1 = pGT->m_aGamma[pD->bG]*nA1;
				ULONG const nB1 = pGT->m_aGamma[pD->bB]*nA1;
				ULONG const nA2 = pS->bA*nW2;
				ULONG const nR2 = pGT->m_aGamma[pS->bR]*nA2;
				ULONG const nG2 = pGT->m_aGamma[pS->bG]*nA2;
				ULONG const nB2 = pGT->m_aGamma[pS->bB]*nA2;
				ULONG const nA = nA1+nA2;
				if ((nA&0xff00) == 0)
				{
					pD->n = 0;
				}
				else
				{
					pD->bA = nA>>8;
					pD->bR = pGT->InvGamma((nR1+nR2)/nA);
					pD->bG = pGT->InvGamma((nG1+nG2)/nA);
					pD->bB = pGT->InvGamma((nB1+nB2)/nA);
				}
			}
		}
		pDoc1->TileSet(EICIRGBA, &tOrigin, &tSize, NULL, nPixels, pBuffer1, TRUE);
		return true;
	}

	static bool MorphStep(ISubDocumentsMgr* pDoc1, ISubDocumentsMgr* pDoc2, float fWeight2)
	{
		CComPtr<IEnumUnknowns> pIDs1;
		CComPtr<IEnumUnknowns> pIDs2;
		pDoc1->ItemsEnum(NULL, &pIDs1);
		pDoc2->ItemsEnum(NULL, &pIDs2);
		ULONG nIDs1 = 0;
		ULONG nIDs2 = 0;
		if (pIDs1) pIDs1->Size(&nIDs1);
		if (pIDs2) pIDs2->Size(&nIDs2);
		if (nIDs1 != nIDs2)
			return false;
		for (ULONG i = 0; i < nIDs1; ++i)
		{
			CComPtr<IComparable> pItem1;
			CComPtr<IComparable> pItem2;
			pIDs1->Get(i, &pItem1);
			pIDs2->Get(i, &pItem2);
			CComPtr<ISubDocumentID> pSDID1;
			CComPtr<ISubDocumentID> pSDID2;
			pDoc1->ItemFeatureGet(pItem1, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID1));
			pDoc2->ItemFeatureGet(pItem2, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID2));
			if (pSDID1 == NULL || pSDID2 == NULL)
				return false;
			CComPtr<IDocument> pSubDoc1;
			CComPtr<IDocument> pSubDoc2;
			pSDID1->SubDocumentGet(&pSubDoc1);
			pSDID2->SubDocumentGet(&pSubDoc2);
			if (!MorphStep(pSubDoc1, pSubDoc2, fWeight2))
				return false;
		}
		return true;
	}

	static bool MorphStep(IDocumentLayeredImage* pDoc1, IDocumentLayeredImage* pDoc2, float fWeight2)
	{
		CComPtr<IEnumUnknowns> pLayers1;
		CComPtr<IEnumUnknowns> pLayers2;
		pDoc1->LayersEnum(NULL, &pLayers1);
		pDoc2->LayersEnum(NULL, &pLayers2);
		ULONG nLayers1 = 0;
		ULONG nLayers2 = 0;
		if (pLayers1) pLayers1->Size(&nLayers1);
		if (pLayers2) pLayers2->Size(&nLayers2);
		if (nLayers1 != nLayers2)
			return false;
		for (ULONG i = 0; i < nLayers1; ++i)
		{
			CComPtr<IComparable> pItem1;
			CComPtr<IComparable> pItem2;
			pLayers1->Get(i, &pItem1);
			pLayers2->Get(i, &pItem2);
			CComPtr<IConfig> pEffect1;
			CComPtr<IConfig> pEffect2;
			pDoc1->LayerEffectGet(pItem1, &pEffect1, NULL);
			pDoc2->LayerEffectGet(pItem2, &pEffect2, NULL);
			if (pEffect1 == NULL || pEffect2 == NULL)
				continue;

			if (MorphConfigs(pEffect1, pEffect2, fWeight2))
				pDoc1->LayerEffectSet(pItem1, pEffect1);
		}
		return true;
	}

	static bool MorphStep(IDocument* pDoc1, IDocument* pDoc2, float fWeight2)
	{
		if (pDoc1 == NULL || pDoc2 == NULL)
			return false;
		{
			CComPtr<IDocumentLayeredImage> pDLI1;
			CComPtr<IDocumentLayeredImage> pDLI2;
			pDoc1->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI1));
			pDoc2->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDLI2));
			if (pDLI1 && pDLI2)
			{
				MorphStep(pDLI1, pDLI2, fWeight2); // only morph effects, other elements are handled via another interface below
				{
					CComPtr<ISubDocumentsMgr> pSDM1;
					CComPtr<ISubDocumentsMgr> pSDM2;
					pDoc1->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM1));
					pDoc2->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM2));
					if (pSDM1 && pSDM2)
						return MorphStep(pSDM1, pSDM2, fWeight2);
				}
			}
		}
#ifdef WITH_RENDERED
		{
			CComPtr<IDocumentRenderedImage> pDRI1;
			CComPtr<IDocumentRenderedImage> pDRI2;
			pDoc1->QueryFeatureInterface(__uuidof(IDocumentRenderedImage), reinterpret_cast<void**>(&pDRI1));
			pDoc2->QueryFeatureInterface(__uuidof(IDocumentRenderedImage), reinterpret_cast<void**>(&pDRI2));
			if (pDRI1 && pDRI2)
				return MorphStep(pDRI1, pDRI2, fWeight2);
		}
#endif
		{
			CComPtr<IDocumentVectorImage> pDVI1;
			CComPtr<IDocumentVectorImage> pDVI2;
			pDoc1->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI1));
			pDoc2->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pDVI2));
			if (pDVI1 && pDVI2)
				return MorphStep(pDVI1, pDVI2, fWeight2);
		}
		{
			CComPtr<IDocumentRasterImage> pDRI1;
			CComPtr<IDocumentRasterImage> pDRI2;
			pDoc1->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI1));
			pDoc2->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pDRI2));
			if (pDRI1 && pDRI2)
				return MorphStep(pDRI1, pDRI2, fWeight2);
		}
		{
			CComPtr<ISubDocumentsMgr> pSDM1;
			CComPtr<ISubDocumentsMgr> pSDM2;
			pDoc1->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM1));
			pDoc2->QueryFeatureInterface(__uuidof(ISubDocumentsMgr), reinterpret_cast<void**>(&pSDM2));
			if (pSDM1 && pSDM2)
				return MorphStep(pSDM1, pSDM2, fWeight2);
		}
		return false;
	}
};

OBJECT_ENTRY_AUTO(CLSID_AnimationMorph, CAnimationMorph)
