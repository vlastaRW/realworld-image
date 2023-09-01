
#pragma once

#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>


template<class T>
class CEditToolPolyLine :
	public IRasterImageEditToolContextMenu
{
public:
	typedef std::vector<TPixelCoords> CPolyLine;
	struct CVertexSource
	{
		CVertexSource(CEditToolPolyLine<T> const* a_pPolyLine) :
			m_nIndex(0), m_cPoints(a_pPolyLine->m_cPolyLine)
		{
			if (a_pPolyLine->m_nRemoveIndex < m_cPoints.size())
				m_cPoints.erase(m_cPoints.begin()+a_pPolyLine->m_nRemoveIndex);
			else if (a_pPolyLine->m_nExtraIndex)
				m_cPoints.insert(m_cPoints.begin()+a_pPolyLine->m_nExtraIndex, a_pPolyLine->m_tExtraPoint);
			m_nLastPoint = a_pPolyLine->m_bClosed && m_cPoints.size() > 2 ? agg::path_flags_close : 0;
		}

		CVertexSource(CPolyLine const& a_cPolyLine, unsigned a_nLastPoint = agg::path_flags_close) :
			m_nIndex(0), m_cPoints(a_cPolyLine), m_nLastPoint(a_nLastPoint)
		{
		}

		unsigned vertex(double* x, double* y)
		{
			if (m_nIndex < m_cPoints.size())
			{
				*x = m_cPoints[m_nIndex].fX;
				*y = m_cPoints[m_nIndex].fY;
				++m_nIndex;
				return (m_nIndex == 1) ? agg::path_cmd_move_to : agg::path_cmd_line_to;
			}
			else
			{
				return m_nIndex++ == m_cPoints.size() ? agg::path_cmd_end_poly|m_nLastPoint : agg::path_cmd_stop;
			}
		}
		void rewind(unsigned)
		{
			m_nIndex = 0;
		}

		size_t m_nIndex;
		CPolyLine m_cPoints;
		unsigned m_nLastPoint;
	};
	struct CVertexSourceHole
	{
		CVertexSourceHole(CPolyLine const& a_cPolyLine) :
			m_nIndex(0), m_cPoints(a_cPolyLine), m_nLastPoint(agg::path_flags_close)
		{
		}

		unsigned vertex(double* x, double* y)
		{
			if (m_nIndex < m_cPoints.size())
			{
				*x = m_cPoints[m_cPoints.size()-m_nIndex-1].fX;
				*y = m_cPoints[m_cPoints.size()-m_nIndex-1].fY;
				++m_nIndex;
				return (m_nIndex == 1) ? agg::path_cmd_move_to : agg::path_cmd_line_to;
			}
			else
			{
				return m_nIndex++ == m_cPoints.size() ? agg::path_cmd_end_poly|m_nLastPoint : agg::path_cmd_stop;
			}
		}
		void rewind(unsigned)
		{
			m_nIndex = 0;
		}

		size_t m_nIndex;
		CPolyLine m_cPoints;
		unsigned m_nLastPoint;
	};
	size_t M_RemoveIndex() const { return m_nRemoveIndex; }
	size_t M_ExtraIndex() const { return m_nExtraIndex; }
	TPixelCoords M_ExtraPoint() const { return m_tExtraPoint; }

public:
	CEditToolPolyLine(bool a_bClosed = false, bool a_bControlLines = false) :
		m_bClosed(a_bClosed), m_bControlLines(a_bControlLines),
		m_nRemoveIndex(-1), m_nExtraIndex(0)
	{
	}

	void SetPolyLineClosed(bool a_bClosed)
	{
		if (m_bClosed == a_bClosed)
			return;
		m_bClosed = a_bClosed;
		if (m_cPolyLine.size() <= 2)
			return;
		static_cast<T*>(this)->PolygonChanged();
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}
	void SetPolyLineControlLines(bool a_bEnabled)
	{
		if (m_bControlLines == a_bEnabled)
			return;
		m_bControlLines = a_bEnabled;
		if (m_cPolyLine.empty())
			return;
		static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}
	void SetPolyLine(CPolyLine& a_cNew)
	{
		std::swap(m_cPolyLine, a_cNew);
		static_cast<T*>(this)->PolygonChanged();
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}
	CPolyLine& M_PolyLine()
	{
		return m_cPolyLine;
	}

	void ResetPolyLine()
	{
		if (!m_cPolyLine.empty())
		{
			m_cPolyLine.clear();
			static_cast<T*>(this)->PolygonChanged();
			static_cast<T*>(this)->M_Window()->ControlPointsChanged();
			if (m_bControlLines)
				static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		}
	}

	HRESULT _GetControlPointCount(ULONG* a_pCount)
	{
		*a_pCount = !m_bControlLines || m_cPolyLine.empty() ? 0 : m_cPolyLine.size()*2-(m_bClosed && m_cPolyLine.size() > 2 ? 0 : 1);
		return S_OK;
	}
	void GetMidControlPoint(ULONG a_nIndex, ULONG UNREF(a_nExtraIndex), ULONG UNREF(a_nRemoveIndex), TPixelCoords* a_pPos)
	{
		a_pPos->fX = (m_cPolyLine[a_nIndex].fX+m_cPolyLine[(a_nIndex+1)%m_cPolyLine.size()].fX)*0.5f;
		a_pPos->fY = (m_cPolyLine[a_nIndex].fY+m_cPolyLine[(a_nIndex+1)%m_cPolyLine.size()].fY)*0.5f;
	}
	HRESULT _GetControlPoint(ULONG a_nIndex, TPixelCoords* a_pPos, ULONG* a_pClass)
	{
		bool const bClosed = m_bClosed && m_cPolyLine.size() > 2;
		if (m_cPolyLine.empty() || (bClosed ? a_nIndex > m_cPolyLine.size()*2 : a_nIndex >= m_cPolyLine.size()*2))
		if (m_cPolyLine.empty() || a_nIndex >= m_cPolyLine.size()*2)
			return E_RW_INDEXOUTOFRANGE;
		if (a_nIndex&1)
		{
			if (m_nExtraIndex == ((a_nIndex+1)>>1))
			{
				*a_pPos = m_tExtraPoint;
			}
			else
			{
				static_cast<T*>(this)->GetMidControlPoint(a_nIndex>>1, m_nExtraIndex, m_nRemoveIndex, a_pPos);
			}
			*a_pClass = 1;
		}
		else
		{
			*a_pPos = m_cPolyLine[a_nIndex>>1];
			*a_pClass = 0;
		}
		return S_OK;
	}
	HRESULT _AdjustCoordinates(EControlKeysState a_eKeysState, TPixelCoords* a_pPos, TPixelCoords const* a_pPointerSize, ULONG const* a_pControlPointIndex, float a_fPointSize)
	{
		if (a_pControlPointIndex == NULL || a_pPointerSize == NULL || (a_eKeysState&ECKSShiftControl) != ECKSShift)
			return S_OK;
		if (*a_pControlPointIndex&1)
			return S_OK;

		bool const bClosed = m_bClosed && m_cPolyLine.size() > 2;
		if (m_cPolyLine.empty() || (bClosed ? *a_pControlPointIndex > m_cPolyLine.size()*2 : *a_pControlPointIndex >= m_cPolyLine.size()*2))
			return E_RW_INDEXOUTOFRANGE;

		float const eps = a_fPointSize;
		// align x or y if close
		float xDif = eps*2;
		float xPos = 0;
		float yDif = eps*2;
		float yPos = 0;
		ULONG const index = (*a_pControlPointIndex)>>1;
		if (index < m_cPolyLine.size()-1 || bClosed)
		{
			TPixelCoords t = m_cPolyLine[(index+1)%m_cPolyLine.size()];
			if (fabsf(a_pPos->fX-t.fX) < eps && fabsf(a_pPos->fY-t.fY) > eps*2)
			{
				xDif = fabsf(a_pPos->fX-t.fX);
				xPos = t.fX;
			}
			if (fabsf(a_pPos->fY-t.fY) < eps && fabsf(a_pPos->fX-t.fX) > eps*2)
			{
				yDif = fabsf(a_pPos->fY-t.fY);
				yPos = t.fY;
			}
		}
		if (index > 0 || bClosed)
		{
			TPixelCoords t = m_cPolyLine[index ? index-1 : m_cPolyLine.size()-1];
			if (fabsf(a_pPos->fX-t.fX) < xDif && fabsf(a_pPos->fY-t.fY) > 10)
			{
				xDif = fabsf(a_pPos->fX-t.fX);
				xPos = t.fX;
			}
			if (fabsf(a_pPos->fY-t.fY) < yDif && fabsf(a_pPos->fX-t.fX) > 10)
			{
				yDif = fabsf(a_pPos->fY-t.fY);
				yPos = t.fY;
			}
		}

		if (xDif < eps) a_pPos->fX = xPos;
		if (yDif < eps) a_pPos->fY = yPos;

		return S_OK;
	}
	HRESULT _SetControlPoint(ULONG a_nIndex, TPixelCoords const* a_pPos, boolean a_bFinished, float a_fPointSize)
	{
		bool const bClosed = m_bClosed && m_cPolyLine.size() > 2;
		if (m_cPolyLine.empty() || (bClosed ? a_nIndex > m_cPolyLine.size()*2 : a_nIndex >= m_cPolyLine.size()*2))
			return E_RW_INDEXOUTOFRANGE;
		if (a_nIndex&1)
		{
			if (a_bFinished)
			{
				m_cPolyLine.insert(m_cPolyLine.begin()+((a_nIndex+1)>>1), *a_pPos);
				m_nExtraIndex = 0;
				static_cast<T*>(this)->PolygonChanged();
				static_cast<T*>(this)->M_Window()->ControlPointsChanged();
				if (m_bControlLines)
					static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			}
			else
			{
				m_nExtraIndex = (a_nIndex+1)>>1;
				m_tExtraPoint = *a_pPos;
				static_cast<T*>(this)->PolygonChanged();
				static_cast<T*>(this)->M_Window()->ControlPointsChanged();
				//static_cast<T*>(this)->M_Window()->ControlPointChanged(a_nIndex);
				if (m_bControlLines)
					static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			}
		}
		else
		{
			bool bRemove = false;
			ULONG i = a_nIndex>>1;
			//if ((i == 0 || i == (m_cPolyLine.size()-1)) && !bClosed)
			//	m_bClosed = true;
			if (i > 0 || bClosed)
			{
				TPixelCoords const& t = m_cPolyLine[i ? i-1 : m_cPolyLine.size()-1];
				if (sqrtf((t.fX-a_pPos->fX)*(t.fX-a_pPos->fX) + (t.fY-a_pPos->fY)*(t.fY-a_pPos->fY)) < a_fPointSize)
					bRemove = true;
			}
			if (i < (m_cPolyLine.size()-1) || bClosed)
			{
				TPixelCoords const& t = m_cPolyLine[i < (m_cPolyLine.size()-1) ? i+1 : 0];
				if (sqrtf((t.fX-a_pPos->fX)*(t.fX-a_pPos->fX) + (t.fY-a_pPos->fY)*(t.fY-a_pPos->fY)) < a_fPointSize)
					bRemove = true;
			}
			if (bRemove && a_bFinished)
			{
				m_nRemoveIndex = -1;
				if (m_cPolyLine.size() < 3)
					m_cPolyLine.clear();
				else
					m_cPolyLine.erase(m_cPolyLine.begin()+i);
				static_cast<T*>(this)->PolygonChanged();
				static_cast<T*>(this)->M_Window()->ControlPointsChanged();
				if (m_bControlLines)
					static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			}
			else
			{
				m_nRemoveIndex = bRemove ? i : -1;
				m_cPolyLine[i] = *a_pPos;
				static_cast<T*>(this)->PolygonChanged();
				static_cast<T*>(this)->M_Window()->ControlPointsChanged();
				//if (a_nIndex)
				//	static_cast<T*>(this)->M_Window()->ControlPointChanged(a_nIndex-1);
				//else if (bClosed)
				//	static_cast<T*>(this)->M_Window()->ControlPointChanged((m_cPolyLine.size()<<1)-1);
				//static_cast<T*>(this)->M_Window()->ControlPointChanged(a_nIndex);
				//if (a_nIndex < m_cPolyLine.size()*2-2 || bClosed)
				//	static_cast<T*>(this)->M_Window()->ControlPointChanged(a_nIndex+1);
				if (m_bControlLines)
					static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			}
		}
		return S_OK;
	}
	HRESULT _GetControlPointDesc(ULONG a_nIndex, ILocalizedString** a_ppDescription)
	{
		*a_ppDescription = NULL;
		bool const bClosed = m_bClosed && m_cPolyLine.size() > 2;
		if (m_cPolyLine.empty() || (bClosed ? a_nIndex > m_cPolyLine.size()*2 : a_nIndex >= m_cPolyLine.size()*2))
			return E_RW_INDEXOUTOFRANGE;
		*a_ppDescription = _SharedStringTable.GetString(a_nIndex&1 ? IDS_POINTDESC_POLYLINEEDGE : IDS_POINTDESC_POLYLINEVERTEX);
		return S_OK;
	}
	HRESULT _GetControlLines(IEditToolControlLines* a_pLines, ULONG a_nLineTypes)
	{
		if (m_cPolyLine.empty() || !m_bControlLines || 0 == (a_nLineTypes&ECLTSelection))
			return S_FALSE;
		a_pLines->MoveTo(m_cPolyLine[0].fX, m_cPolyLine[0].fY);
		CPolyLine::const_iterator iExtra = m_cPolyLine.begin()+m_nExtraIndex;
		for (CPolyLine::const_iterator i = m_cPolyLine.begin()+1; i != m_cPolyLine.end(); ++i)
		{
			if (iExtra == i)
				a_pLines->LineTo(m_tExtraPoint.fX, m_tExtraPoint.fY);
			else if (m_nRemoveIndex == i-m_cPolyLine.begin())
				continue;
			a_pLines->LineTo(i->fX, i->fY);
		}
		if (iExtra == m_cPolyLine.end())
			a_pLines->LineTo(m_tExtraPoint.fX, m_tExtraPoint.fY);
		if (m_cPolyLine.size() > 2 && m_bClosed)
			a_pLines->Close();
		return S_OK;
	}

	HRESULT _PreTranslateMessage(MSG const* a_pMsg)
	{
		if (m_cPolyLine.empty())
			return S_FALSE;

		if (a_pMsg->message != WM_KEYDOWN ||
			(a_pMsg->wParam != VK_LEFT && a_pMsg->wParam != VK_RIGHT &&
			 a_pMsg->wParam != VK_UP && a_pMsg->wParam != VK_DOWN))
			 return S_FALSE;
		{
			LONG nDX = 0;
			LONG nDY = 0;
			if (a_pMsg->wParam == VK_LEFT)
				nDX = -1;
			else if (a_pMsg->wParam == VK_RIGHT)
				nDX = 1;
			else if (a_pMsg->wParam == VK_UP)
				nDY = -1;
			else if (a_pMsg->wParam == VK_DOWN)
				nDY = 1;

			for (CPolyLine::iterator i = m_cPolyLine.begin(); i != m_cPolyLine.end(); ++i)
			{
				i->fX += nDX;
				i->fY += nDY;
			}
			static_cast<T*>(this)->PolygonChanged();
			static_cast<T*>(this)->M_Window()->ControlPointsChanged();
			if (m_bControlLines)
				static_cast<T*>(this)->M_Window()->ControlLinesChanged();
			return S_OK;
		}
		return S_FALSE;
	}

	static TPixelCoords TransformPixelCoords(TMatrix3x3f const& a_tMatrix, TPixelCoords const& a_tVector)
	{
		float const fW = 1.0f/(a_tMatrix._13*a_tVector.fX + a_tMatrix._23*a_tVector.fY + a_tMatrix._33);
		TPixelCoords const t =
		{
			fW*(a_tMatrix._11*a_tVector.fX + a_tMatrix._21*a_tVector.fY + a_tMatrix._31),
			fW*(a_tMatrix._12*a_tVector.fX + a_tMatrix._22*a_tVector.fY + a_tMatrix._32),
		};
		return t;
	}
	void TransformPolygon(TMatrix3x3f const* a_pMatrix)
	{
		for (CPolyLine::iterator i = m_cPolyLine.begin(); i != m_cPolyLine.end(); ++i)
			*i = TransformPixelCoords(*a_pMatrix, *i);
		static_cast<T*>(this)->PolygonChanged();
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
	}

	// IRasterImageEditToolContextMenu methods
public:
	class ATL_NO_VTABLE CDeleteVertex :
		public CDocumentMenuCommandImpl<CDeleteVertex, IDS_MENU_DELETEVERTEX_NAME, IDS_MENU_DELETEVERTEX_DESC, NULL, 0>
	{
	public:
		CDeleteVertex() : m_pTool(NULL)
		{
		}
		~CDeleteVertex()
		{
			if (m_pTool) m_pTool->Release();
		}
		void Init(CEditToolPolyLine<T>* a_pTool, ULONG a_nIndex)
		{
			(m_pTool = a_pTool)->AddRef();
			m_nIndex = a_nIndex;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return m_pTool->DeleteVertex(m_nIndex);
		}

	private:
		CEditToolPolyLine<T>* m_pTool;
		ULONG m_nIndex;
	};

	HRESULT DeleteVertex(ULONG a_nIndex)
	{
		if (static_cast<T*>(this)->M_Window() == NULL || a_nIndex >= m_cPolyLine.size())
			return S_FALSE;
		if (m_cPolyLine.size() < 3)
			m_cPolyLine.clear();
		else
			m_cPolyLine.erase(m_cPolyLine.begin()+a_nIndex);
		static_cast<T*>(this)->PolygonChanged();
		static_cast<T*>(this)->M_Window()->ControlPointsChanged();
		if (m_bControlLines)
			static_cast<T*>(this)->M_Window()->ControlLinesChanged();
		return S_OK;
	}

	STDMETHOD(CommandsEnum)(TPixelCoords const* a_pPixel, ULONG a_nControlPointIndex, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			if (a_nControlPointIndex&1)
				return S_FALSE;

			a_nControlPointIndex >>= 1;
			if (a_nControlPointIndex >= m_cPolyLine.size())
				return S_FALSE;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CDeleteVertex>* p = NULL;
				CComObject<CDeleteVertex>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(this, a_nControlPointIndex);
				pItems->Insert(pTmp);
			}
			//{
			//	CComObject<CCancelSelection>* p = NULL;
			//	CComObject<CCancelSelection>::CreateInstance(&p);
			//	CComPtr<IDocumentMenuCommand> pTmp = p;
			//	p->Init(this);
			//	pItems->Insert(pTmp);
			//}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CPolyLine m_cPolyLine;
	TPixelCoords m_tExtraPoint;
	size_t m_nRemoveIndex;
	size_t m_nExtraIndex;
	bool m_bClosed;
	bool m_bControlLines;
};
