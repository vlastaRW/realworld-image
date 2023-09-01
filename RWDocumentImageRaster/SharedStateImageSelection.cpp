// SharedStateImageSelection.cpp : Implementation of CSharedStateImageSelection

#include "stdafx.h"
#include "SharedStateImageSelection.h"


// CSharedStateImageSelection

STDMETHODIMP CSharedStateImageSelection::Init(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE const* a_pData)
{
	try
	{
		m_nX = a_nX;
		m_nY = a_nY;
		m_nSizeX = a_nSizeX;
		m_nSizeY = a_nSizeY;
		m_pData.Free();
		if (a_pData)
		{
			m_pData.Attach(new BYTE[a_nSizeX*a_nSizeY]);
			for (ULONG y = 0; y < a_nSizeY; ++y)
			{
				CopyMemory(m_pData.m_p+m_nSizeX*y, a_pData+a_nStride*y, a_nSizeX);
			}
		}
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateImageSelection::IsEmpty()
{
	return m_pData.m_p ? S_FALSE : S_OK;
}

STDMETHODIMP CSharedStateImageSelection::Bounds(LONG* a_pX, LONG* a_pY, ULONG* a_pSizeX, ULONG* a_pSizeY)
{
	try
	{
		if (a_pX) *a_pX = m_nX;
		if (a_pY) *a_pY = m_nY;
		if (a_pSizeX) *a_pSizeX = m_nSizeX;
		if (a_pSizeY) *a_pSizeY = m_nSizeY;
		return S_OK;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSharedStateImageSelection::GetTile(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pData)
{
	try
	{
		LONG const a_nX2 = a_nX+a_nSizeX;
		LONG const a_nY2 = a_nY+a_nSizeY;
		LONG const m_nX2 = m_nX+m_nSizeX;
		LONG const m_nY2 = m_nY+m_nSizeY;
		if (m_nX >= a_nX2 || m_nY >= a_nY2 || m_nX2 <= a_nX || m_nY2 <= a_nY)
		{
			for (ULONG y = 0; y < a_nSizeY; ++y)
				ZeroMemory(a_pData+a_nStride*y, a_nSizeX);
			return S_OK;
		}

		LONG y = a_nY;
		for (; y < m_nY; ++y, a_pData+=a_nStride)
			ZeroMemory(a_pData, a_nSizeX);
		LONG const nYEnd = m_nY2 < a_nY2 ? m_nY2 : a_nY2; 
		LONG const nXEnd = m_nX2 < a_nX2 ? m_nX2 : a_nX2; 
		for (; y < nYEnd; ++y, a_pData+=a_nStride-a_nSizeX)
		{
			LONG x = a_nX;
			if (a_nX < m_nX)
			{
				ZeroMemory(a_pData, m_nX-a_nX);
				a_pData += m_nX-a_nX;
				x = m_nX;
			}
			if (m_pData)
			{
				CopyMemory(a_pData, m_pData.m_p+(y-m_nY)*m_nSizeX+x-m_nX, nXEnd-x);
			}
			else
			{
				FillMemory(a_pData, nXEnd-x, 0xff);
			}
			a_pData += nXEnd-x;
			if (m_nX2 < a_nX2)
			{
				ZeroMemory(a_pData, a_nX2-m_nX2);
				a_pData += a_nX2-m_nX2;
			}
		}
		for (; y < a_nY2; ++y, a_pData+=a_nStride)
			ZeroMemory(a_pData, a_nSizeX);
		return S_OK;
	}
	catch (...)
	{
		return a_pData ? E_UNEXPECTED : E_POINTER;
	}
}

