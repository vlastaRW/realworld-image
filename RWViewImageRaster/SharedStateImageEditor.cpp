// SharedStateImageEditor.h : Declaration of the CSharedStateImageEditor

#include "stdafx.h"
#include "RWViewImageRaster.h"


// CSharedStateImageEditor

class ATL_NO_VTABLE CSharedStateImageEditor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CSharedStateImageEditor>,
	public ISharedStateImageEditor
{
public:
	CSharedStateImageEditor()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CSharedStateImageEditor)

BEGIN_COM_MAP(CSharedStateImageEditor)
	COM_INTERFACE_ENTRY(ISharedState)
	COM_INTERFACE_ENTRY(ISharedStateImageEditor)
END_COM_MAP()


	// ISharedStateImageEditor methods
public:
	STDMETHOD_(ULONG, GetStyle)() { return m_nStyle; }
	STDMETHOD(SetStyle)(ULONG a_nStyle) { m_nStyle = a_nStyle; return S_OK; }
	STDMETHOD_(ULONG, GetGrid)() { return m_nGrid; }
	STDMETHOD(SetGrid)(ULONG a_nGrid) { m_nGrid = a_nGrid; return S_OK; }
	STDMETHOD_(EImageCompositon, GetCompositionMode)() { return m_eMode; }
	STDMETHOD(SetCompositionMode)(EImageCompositon a_eMode) { m_eMode = a_eMode; return S_OK; }
	STDMETHOD_(EImageQuality, GetImageQuality)() { return m_eQuality; }
	STDMETHOD(SetImageQuality)(EImageQuality a_eQuality) { m_eQuality = a_eQuality; return S_OK; }
	STDMETHOD_(BYTE, GetInvertedPixels)() { return m_bInverted; }
	STDMETHOD(SetInvertedPixels)(BYTE a_bInverting) { m_bInverted = a_bInverting; return S_OK; }
	STDMETHOD_(float, GetHandleSize)() { return m_fHandleSize; }
	STDMETHOD(SetHandleSize)(float a_fSize) { m_fHandleSize = a_fSize; return S_OK; }
	STDMETHOD_(BYTE, GetHideOnLeave)() { return m_bHideOnLeave; }
	STDMETHOD(SetHideOnLeave)(BYTE a_bHide) { m_bHideOnLeave = a_bHide; return S_OK; }

	// ISharedState methods
public:
	STDMETHOD(CLSIDGet)(CLSID* a_pCLSID)
	{
		if (a_pCLSID == NULL)
			return E_POINTER;
		*a_pCLSID = CLSID_SharedStateImageEditor;
		return S_OK;
	}
	STDMETHOD(ToText)(BSTR* a_pbstrText)
	{
		if (a_pbstrText == NULL)
			return E_POINTER;
		wchar_t buf[128];
		swprintf(buf, L"[ImageEditorState1]%i,%i,%i,%i,%i,%g,%i", m_nStyle, m_nGrid, int(m_eMode), int(m_eQuality), int(m_bInverted), m_fHandleSize, int(m_bHideOnLeave));
		*a_pbstrText = SysAllocString(buf);
		return S_OK;
	}
	STDMETHOD(FromText)(BSTR a_bstrText)
	{
		if (a_bstrText == NULL)
			return E_POINTER;
		ULONG len = SysStringLen(a_bstrText);
		if (len < 20 || wcsncmp(a_bstrText, L"[ImageEditorState1]", 19))
			return E_RW_INVALIDPARAM;
		int nStyle;
		int nGrid;
		int eMode;
		int eQuality;
		int bInverted;
		float fHandleSize;
		int bHideOnLeave;
		if (7 != swscanf(a_bstrText+19, L"%i,%i,%i,%i,%i,%g,%i", &nStyle, &nGrid, &eMode, &eQuality, &bInverted, &fHandleSize, &bHideOnLeave))
			return E_FAIL;
		m_nStyle = nStyle;
		m_nGrid = nGrid;
		m_eMode = static_cast<EImageCompositon>(eMode);
		m_eQuality = static_cast<EImageQuality>(eQuality);
		m_bInverted = bInverted;
		m_fHandleSize = fHandleSize;
		m_bHideOnLeave = bHideOnLeave;
		return S_OK;
	}

private:
	ULONG m_nStyle;
	ULONG m_nGrid;
	EImageCompositon m_eMode;
	EImageQuality m_eQuality;
	BYTE m_bInverted;
	float m_fHandleSize;
	BYTE m_bHideOnLeave;
};

OBJECT_ENTRY_AUTO(CLSID_SharedStateImageEditor, CSharedStateImageEditor)
