
#pragma once

#include <EditTool.h>


class CEditToolNull :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CEditToolBase<>
{
public:
	CEditToolNull() : m_hCursor(NULL)
	{
	}

	BEGIN_COM_MAP(CEditToolNull)
		COM_INTERFACE_ENTRY(IRasterImageEditTool)
	END_COM_MAP()

	// IRasterImageEditTool methods
public:
	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_tPos, HCURSOR* a_phCursor)
	{
		if (m_hCursor == NULL)
			m_hCursor = ::LoadCursor(NULL, IDC_NO);
		*a_phCursor = m_hCursor;
		return S_OK;
	}

private:
	HCURSOR m_hCursor;
};

