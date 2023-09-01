// ScriptedTool.cpp : Implementation of CScriptedTool

#include "stdafx.h"
#include "ScriptedTool.h"

#include <StringParsing.h>
#include "ScriptedRasterImageEditWindow.h"
#include <RWDocumentImageVector.h>


// CScriptedTool

STDMETHODIMP CScriptedTool::put_BlendMode(int newVal)
{
	if (newVal == EBMDrawOver || newVal == EBMReplace || newVal == EBMDrawUnder || newVal == EBMAdd)
	{
		m_eBlendMode = static_cast<EBlendingMode>(newVal);
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_RasterizationMode(int newVal)
{
	if (newVal == ERMBinary || newVal == ERMSmooth)
	{
		m_eRasterMode = static_cast<ERasterizationMode>(newVal);
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_ShapeFillMode(int newVal)
{
	m_bOutline = newVal&1;
	m_bFill = (newVal&2)>>1;
	return S_OK;
}

STDMETHODIMP CScriptedTool::put_CoordinatesMode(int newVal)
{
	if (newVal == ECMFloatingPoint || newVal == ECMIntegral)
	{
		m_eCoordsMode = static_cast<ECoordinatesMode>(newVal);
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_OutlineWidth(float newVal)
{
	if (newVal >= 0.0f)
	{
		m_fOutlineWidth = newVal;
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_OutlinePos(float newVal)
{
	if (newVal >= -1.0f && newVal <= 1.0f)
	{
		m_fOutlinePos = newVal;
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_OutlineJoins(int newVal)
{
	if (newVal == EOJTMiter || newVal == EOJTRound || newVal == EOJTBevel)
	{
		m_eOutlineJoins = static_cast<EOutlineJoinType>(newVal);
		return S_OK;
	}
	return E_INVALIDARG;
}

STDMETHODIMP CScriptedTool::put_GammaOverride(float newVal)
{
	return S_OK;
}

STDMETHODIMP CScriptedTool::SetFillColor(float fR, float fG, float fB, float fA)
{
	try
	{
		InitManagers();

		m_pFillStyle = NULL;
		m_bstrFillStyleID = L"SOLID";
		m_pStyles->FillStyleCreate(m_bstrFillStyleID, NULL, &m_pFillStyle);
		CComQIPtr<IRasterImageEditToolScripting> pScriptParams(m_pFillStyle);
		wchar_t sz[64] = L"";
		swprintf(sz, L"%g,%g,%g,%g", fR, fG, fB, fA);
		m_bstrFillStyleParams = sz;
		pScriptParams->FromText(m_bstrFillStyleParams);
		m_bFill = TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedTool::SetOutlineColor(float fR, float fG, float fB, float fA)
{
	m_bOutline = TRUE;
	m_tOutlineColor.fR = fR;
	m_tOutlineColor.fG = fG;
	m_tOutlineColor.fB = fB;
	m_tOutlineColor.fA = fA;
	return S_OK;
}

STDMETHODIMP CScriptedTool::SetFillStyle(BSTR styleID, BSTR styleParams)
{
	try
	{
		InitManagers();

		m_pFillStyle = NULL;
		m_pStyles->FillStyleCreate(styleID, NULL, &m_pFillStyle);
		CComQIPtr<IRasterImageEditToolScripting> pScriptParams(m_pFillStyle);
		if (pScriptParams && styleParams)
			pScriptParams->FromText(styleParams);
		m_bstrFillStyleID = styleID;
		m_bstrFillStyleParams = styleParams;
		m_bFill = TRUE;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedTool::Execute(IUnknown* document, BSTR toolID, BSTR toolParams)
{
	try
	{
		CComQIPtr<IDocument> pDoc(document);
		if (pDoc == NULL)
			return E_INVALIDARG;
		CComPtr<IDocumentRasterImage> pRI;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentRasterImage), reinterpret_cast<void**>(&pRI));
		if (pRI)
		{
			InitManagers();

			CComPtr<IRasterImageEditTool> pTool;
			m_pTools->EditToolCreate(toolID, pDoc, &pTool);
			CComQIPtr<IRasterImageEditToolScripting> pScriptingTool(pTool);
			if (pScriptingTool == NULL)
				return E_FAIL;
			CComObject<CScriptedRasterImageEditWindow>* p = NULL;
			CComObject<CScriptedRasterImageEditWindow>::CreateInstance(&p);
			CComPtr<IRasterImageEditWindow> pEW = p;
			p->Init(pDoc, pRI, pTool);
			pTool->Init(pEW);
			if (m_pFillStyle)
			{
				pTool->SetBrush(m_pFillStyle);
			}
			pTool->SetGlobals(m_eBlendMode, m_eRasterMode, m_eCoordsMode);
			pTool->SetOutline(m_bOutline, m_fOutlineWidth, m_fOutlinePos, m_eOutlineJoins, &m_tOutlineColor);
			pScriptingTool->FromText(toolParams);
			p->ApplyChanges();
			return S_OK;
		}

		CComPtr<IDocumentVectorImage> pVI;
		pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pVI));
		if (pVI == NULL)
			return E_INVALIDARG;

		ULONG id = 0;
		pVI->ObjectSet(&id, toolID, toolParams);
		if (id)
		{
			pVI->ObjectStateSet(id, &m_bFill, &m_eRasterMode, &m_eCoordsMode, &m_bOutline, &m_tOutlineColor, &m_fOutlineWidth, &m_fOutlinePos, &m_eOutlineJoins);
			if (m_pFillStyle)
				pVI->ObjectStyleSet(id, m_bstrFillStyleID, m_bstrFillStyleParams);
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CScriptedTool::SetSelection(IUnknown* context, BSTR stateID)
{
	try
	{
		m_pStates = NULL;;
		context->QueryInterface(__uuidof(IOperationContext), reinterpret_cast<void**>(&m_pStates));
		m_bstrSelectionID = stateID;
		ATLASSERT(0);
		return E_NOTIMPL;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

#include <GammaCorrection.h>

STDMETHODIMP CScriptedTool::FromSRGB(int color, float* pColor)
{
	*pColor = CGammaTables::FromSRGB(color);
	return S_OK;
}
