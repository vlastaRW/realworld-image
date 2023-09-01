// ScriptedTool.h : Declaration of the CScriptedTool

#pragma once
#include "resource.h"       // main symbols
#include "RWOperationImageRaster.h"

#include <RWDocumentImageRaster.h>
#include <RWViewImageRaster.h>


// CScriptedTool

class ATL_NO_VTABLE CScriptedTool : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<IScriptedTool, &IID_IScriptedTool, &LIBID_RWOperationImageRasterLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff>
{
	typedef IDispatchImpl<IScriptedTool, &IID_IScriptedTool, &LIBID_RWOperationImageRasterLib, /*wMajor =*/ 0xffff, /*wMinor =*/ 0xffff> CDispatchBase;

public:
	CScriptedTool() :
		m_eBlendMode(EBMDrawOver), m_eRasterMode(ERMSmooth), m_eCoordsMode(ECMFloatingPoint),
		m_fOutlineWidth(1.0f), m_fOutlinePos(-1.0f), m_eOutlineJoins(EOJTRound), m_bOutline(FALSE), m_bFill(TRUE)
	{
		m_tOutlineColor.fR = m_tOutlineColor.fG = m_tOutlineColor.fB = 0.0f; m_tOutlineColor.fA = 1.0f;
	}
	void Init(IRasterImageEditToolsManager* a_pTools, IRasterImageFillStyleManager* a_pStyles)
	{
		m_pTools = a_pTools;
		m_pStyles = a_pStyles;
	}

DECLARE_NOT_AGGREGATABLE(CScriptedTool)

BEGIN_COM_MAP(CScriptedTool)
	COM_INTERFACE_ENTRY(IScriptedTool)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	// IDispatch methods
public:
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
	{
		try
		{
			if (cNames == 1)
			{
				InitManagers();
				CComBSTR bstr(*rgszNames);
				if (m_pTools->SupportedStates(bstr, NULL, NULL, NULL, NULL) == S_OK)
				{
					for (CDispIDs::const_iterator i = m_cDispIDs.begin(); i != m_cDispIDs.end(); ++i)
					{
						if (_wcsicmp(*i, bstr) == 0)
						{
							*rgDispId = 100+(i-m_cDispIDs.begin());
							return S_OK;
						}
					}
					m_cDispIDs.push_back(bstr);
					*rgDispId = 99+m_cDispIDs.size();
					return S_OK;
				}
			}
			return CDispatchBase::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
	{
		try
		{
			if (dispIdMember >= 100 && dispIdMember < DISPID(100+m_cDispIDs.size()))
			{
				if (pDispParams->cArgs < 1 || (pDispParams->rgvarg[pDispParams->cArgs-1].vt != VT_UNKNOWN && pDispParams->rgvarg[pDispParams->cArgs-1].vt != VT_DISPATCH))
					return E_INVALIDARG;
				CComBSTR bstrPar;
				for (UINT i = 1; i < pDispParams->cArgs; ++i)
				{
					VARTYPE vt = pDispParams->rgvarg[pDispParams->cArgs-i-1].vt;
					if (vt == VT_I4 || vt == VT_I2 || vt == VT_I1)
					{
						wchar_t szTmp[32];
						swprintf(szTmp, bstrPar == NULL ? L"%i" : L",%i", pDispParams->rgvarg[pDispParams->cArgs-i-1].intVal);
						bstrPar += szTmp;
					}
					else if (vt == VT_UI4 || vt == VT_UI2 || vt == VT_UI1)
					{
						wchar_t szTmp[32];
						swprintf(szTmp, bstrPar == NULL ? L"%u" : L",%u", pDispParams->rgvarg[pDispParams->cArgs-i-1].uintVal);
						bstrPar += szTmp;
					}
					else if (vt == VT_R4)
					{
						wchar_t szTmp[32];
						swprintf(szTmp, bstrPar == NULL ? L"%g" : L",%g", pDispParams->rgvarg[pDispParams->cArgs-i-1].fltVal);
						bstrPar += szTmp;
					}
					else if (vt == VT_R8)
					{
						wchar_t szTmp[32];
						swprintf(szTmp, bstrPar == NULL ? L"%G" : L",%G", pDispParams->rgvarg[pDispParams->cArgs-i-1].dblVal);
						bstrPar += szTmp;
					}
					else if (vt == VT_DISPATCH)
					{
						IDispatch* pArray = pDispParams->rgvarg[pDispParams->cArgs-i-1].pdispVal;
						DISPPARAMS params;
						ZeroMemory(&params, sizeof params);
						CComVariant res;
						DISPID dl = 0;
						LPOLESTR ln = L"length";
						if (SUCCEEDED(pArray->GetIDsOfNames(IID_NULL, &ln, 1, LOCALE_USER_DEFAULT, &dl)) &&
							SUCCEEDED(pArray->Invoke(dl, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
						{
							res.ChangeType(VT_I4);
							LONG len = res.lVal;

							for (int j = 0; j < len; ++j)
							{
								OLECHAR szIndex[16];
								swprintf(szIndex, L"%i", j);
								LPOLESTR psz = szIndex;
								DISPID id = 0;
								res.Clear();
								if (SUCCEEDED(pArray->GetIDsOfNames(IID_NULL, &psz, 1, LOCALE_USER_DEFAULT, &id)) &&
									SUCCEEDED(pArray->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &res, NULL, NULL)))
								{
									if (res.vt == VT_I4 || res.vt == VT_I2 || res.vt == VT_I1)
									{
										res.ChangeType(VT_I4);
										wchar_t szTmp[32];
										swprintf(szTmp, bstrPar == NULL ? L"%i" : L",%i", res.intVal);
										bstrPar += szTmp;
									}
									else if (res.vt == VT_UI4 || res.vt == VT_UI2 || res.vt == VT_UI1)
									{
										res.ChangeType(VT_UI4);
										wchar_t szTmp[32];
										swprintf(szTmp, bstrPar == NULL ? L"%u" : L",%u", res.uintVal);
										bstrPar += szTmp;
									}
									else if (res.vt == VT_R4)
									{
										wchar_t szTmp[32];
										swprintf(szTmp, bstrPar == NULL ? L"%g" : L",%g", res.fltVal);
										bstrPar += szTmp;
									}
									else if (res.vt == VT_R8)
									{
										wchar_t szTmp[32];
										swprintf(szTmp, bstrPar == NULL ? L"%G" : L",%G", res.dblVal);
										bstrPar += szTmp;
									}
									else if (SUCCEEDED(res.ChangeType(VT_BSTR)))
									{
										bstrPar += bstrPar ? L",\"" : L"\"";
										ULONG nExtra = 0;
										if (res.bstrVal) for (LPCWSTR psz = res.bstrVal; *psz; ++psz)
											if (*psz == L'\"' || *psz == L'\\')
												++nExtra;
										if (nExtra)
										{
											CAutoVectorPtr<OLECHAR> sz(new OLECHAR[nExtra+SysStringLen(res.bstrVal)+1]);
											LPOLESTR pszD = sz;
											for (LPCWSTR psz = res.bstrVal; *psz; ++psz, ++pszD)
											{
												if (*psz == L'\"' || *psz == L'\\')
													*(pszD++) = L'\\';
												*pszD = *psz;
											}
											*pszD = L'\0';
											bstrPar += sz.m_p;
										}
										else
										{
											bstrPar += res.bstrVal;
										}
										bstrPar += L"\"";
									}
								}
							}
						}
					}
					else
					{
						CComVariant cVar;
						if (SUCCEEDED(cVar.ChangeType(VT_BSTR, pDispParams->rgvarg+pDispParams->cArgs-i-1)))
						{
							bstrPar += bstrPar ? L",\"" : L"\"";
							ULONG nExtra = 0;
							if (cVar.bstrVal) for (LPCWSTR psz = cVar.bstrVal; *psz; ++psz)
								if (*psz == L'\"' || *psz == L'\\')
									++nExtra;
							if (nExtra)
							{
								CAutoVectorPtr<OLECHAR> sz(new OLECHAR[nExtra+SysStringLen(cVar.bstrVal)+1]);
								LPOLESTR pszD = sz;
								for (LPCWSTR psz = cVar.bstrVal; *psz; ++psz, ++pszD)
								{
									if (*psz == L'\"' || *psz == L'\\')
										*(pszD++) = L'\\';
									*pszD = *psz;
								}
								*pszD = L'\0';
								bstrPar += sz.m_p;
							}
							else
							{
								bstrPar += cVar.bstrVal;
							}
							bstrPar += L"\"";
						}
					}
				}
				return Execute(pDispParams->rgvarg[pDispParams->cArgs-1].vt != VT_UNKNOWN ? pDispParams->rgvarg[pDispParams->cArgs-1].punkVal : pDispParams->rgvarg[pDispParams->cArgs-1].pdispVal, m_cDispIDs[dispIdMember-100], bstrPar);
				//pVarResult->vt = VT_DISPATCH;
				//(pVarResult->pdispVal = m_cInterfaces[dispIdMember-100].second)->AddRef();
				//return S_OK;
			}
			return CDispatchBase::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

	// IScriptedTool methods
public:
	STDMETHOD(get_BlendMode)(int* pVal) { *pVal = m_eBlendMode; return S_OK; };
	STDMETHOD(put_BlendMode)(int newVal);
	STDMETHOD(get_RasterizationMode)(int* pVal) { *pVal = m_eRasterMode; return S_OK; };
	STDMETHOD(put_RasterizationMode)(int newVal);
	STDMETHOD(get_ShapeFillMode)(int* pVal) { *pVal = (m_bOutline ? 1 : 0)|(m_bFill ? 2 : 0); return S_OK; };
	STDMETHOD(put_ShapeFillMode)(int newVal);
	STDMETHOD(get_CoordinatesMode)(int* pVal) { *pVal = m_eCoordsMode; return S_OK; };
	STDMETHOD(put_CoordinatesMode)(int newVal);
	STDMETHOD(get_OutlineWidth)(float* pVal) { *pVal = m_fOutlineWidth; return S_OK; };
	STDMETHOD(put_OutlineWidth)(float newVal);
	STDMETHOD(get_OutlinePos)(float* pVal) { *pVal = m_fOutlinePos; return S_OK; };
	STDMETHOD(put_OutlinePos)(float newVal);
	STDMETHOD(get_OutlineJoins)(int* pVal) { *pVal = m_eOutlineJoins; return S_OK; };
	STDMETHOD(put_OutlineJoins)(int newVal);
	STDMETHOD(get_GammaOverride)(float* pVal) { return S_OK; };
	STDMETHOD(put_GammaOverride)(float newVal);
	STDMETHOD(SetFillColor)(float fR, float fG, float fB, float fA);
	STDMETHOD(SetOutlineColor)(float fR, float fG, float fB, float fA);
	STDMETHOD(SetFillStyle)(BSTR styleID, BSTR styleParams);
	STDMETHOD(Execute)(IUnknown* document, BSTR toolID, BSTR toolParams);
	STDMETHOD(get_BMDrawOver)(int* pVal)	{ *pVal = EBMDrawOver;	return S_OK; }
	STDMETHOD(get_BMReplace)(int* pVal)		{ *pVal = EBMReplace;	return S_OK; }
	STDMETHOD(get_BMDrawUnder)(int* pVal)	{ *pVal = EBMDrawUnder;	return S_OK; }
	STDMETHOD(get_BMAdd)(int* pVal)			{ *pVal = EBMAdd;		return S_OK; }
	STDMETHOD(get_RMBinary)(int* pVal)		{ *pVal = ERMBinary;	return S_OK; }
	STDMETHOD(get_RMSmooth)(int* pVal)		{ *pVal = ERMSmooth;	return S_OK; }
	STDMETHOD(get_SFMOutline)(int* pVal)	{ *pVal = 1;			return S_OK; }
	STDMETHOD(get_SFMFilled)(int* pVal)		{ *pVal = 2;			return S_OK; }
	STDMETHOD(get_SFMCombined)(int* pVal)	{ *pVal = 3;			return S_OK; }
	STDMETHOD(get_CMArbitrary)(int* pVal)	{ *pVal = ECMFloatingPoint;	return S_OK; }
	STDMETHOD(get_CMIntegral)(int* pVal)	{ *pVal = ECMIntegral;	return S_OK; }
	STDMETHOD(get_OJMiter)(int* pVal)		{ *pVal = EOJTMiter;	return S_OK; }
	STDMETHOD(get_OJRound)(int* pVal)		{ *pVal = EOJTRound;	return S_OK; }
	STDMETHOD(get_OJBevel)(int* pVal)		{ *pVal = EOJTBevel;	return S_OK; }
	STDMETHOD(SetSelection)(IUnknown* context, BSTR stateID);
	STDMETHOD(FromSRGB)(int color, float* pColor);

	STDMETHOD(SetColor1)(float fR, float fG, float fB, float fA) // deprecated
	{ return SetFillColor(powf(fR, 2.2f), powf(fG, 2.2f), powf(fB, 2.2f), fA); }

private:
	typedef std::vector<CComBSTR> CDispIDs;

private:
	void InitManagers()
	{
		if (m_pTools == NULL || m_pStyles == NULL)
		{
			ObjectLock cLock(this);
			if (m_pTools == NULL)
				RWCoCreateInstance(m_pTools, __uuidof(RasterImageEditToolsManager));
			if (m_pStyles == NULL)
				RWCoCreateInstance(m_pStyles, __uuidof(RasterImageFillStyleManager));
		}
	}

private:
	CDispIDs m_cDispIDs;

	EBlendingMode m_eBlendMode;
	ERasterizationMode m_eRasterMode;
	ECoordinatesMode m_eCoordsMode;
	BOOL m_bFill;
	BOOL m_bOutline;
	TColor m_tOutlineColor;
	float m_fOutlineWidth;
	float m_fOutlinePos;
	EOutlineJoinType m_eOutlineJoins;
	CComPtr<IRasterImageBrush> m_pFillStyle;
	CComBSTR m_bstrFillStyleID;
	CComBSTR m_bstrFillStyleParams;
	CComPtr<IRasterImageEditToolsManager> m_pTools;
	CComPtr<IRasterImageFillStyleManager> m_pStyles;
	CComPtr<IOperationContext> m_pStates;
	CComBSTR m_bstrSelectionID;
};

