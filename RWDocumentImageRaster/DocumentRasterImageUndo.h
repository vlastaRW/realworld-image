
#pragma once

#include "RWDocumentImageRaster.h"
#include <DocumentUndoImpl.h>

class CDocumentRasterImage;

class CUndoStepCanvasSize : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TImageSize a_tSize)
	{
		m_pDoc = a_pDoc;
		m_tSize = a_tSize;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->CanvasSet(&m_tSize, NULL, NULL, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TImageSize m_tSize;
};

typedef CUndoStepImpl<CUndoStepCanvasSize> CUndoCanvasSize;


class CUndoStepResolution : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, TImageResolution a_tResolution)
	{
		m_pDoc = a_pDoc;
		m_tResolution = a_tResolution;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pDoc->CanvasSet(NULL, &m_tResolution, NULL, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	TImageResolution m_tResolution;
};

typedef CUndoStepImpl<CUndoStepResolution> CUndoResolution;


class CUndoStepContentMove : public CDocumentUndoStep
{
public:
	void Init(IDocumentEditableImage* a_pDoc, LONG a_nDX, LONG a_nDY)
	{
		m_pDoc = a_pDoc;
		m_nDX = a_nDX;
		m_nDY = a_nDY;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		TMatrix3x3f const t = {1.0, 0.0f, 0.0f,  0.0f, 1.0, 0.0f,  m_nDX, m_nDY, 1.0f};
		return m_pDoc->CanvasSet(NULL, NULL, &t, NULL);
	}

private:
	CComPtr<IDocumentEditableImage> m_pDoc;
	LONG m_nDX;
	LONG m_nDY;
};

typedef CUndoStepImpl<CUndoStepContentMove> CUndoContentMove;


class CUndoStepChannels : public CDocumentUndoStep
{
public:
	void Init(IDocumentRasterImage* a_pDoc, TPixelChannel a_tDefault)
	{
		m_pDoc = a_pDoc;
		m_tDefault = a_tDefault;
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		static EImageChannelID const tChID = EICIRGBA;
		return m_pDoc->ChannelsSet(EICIRGBA, &tChID, &m_tDefault);
	}

private:
	CComPtr<IDocumentRasterImage> m_pDoc;
	TPixelChannel m_tDefault;
};

typedef CUndoStepImpl<CUndoStepChannels> CUndoChannels;


class CUndoStepPixelTile : public CDocumentUndoStep
{
public:
	void Init(IDocumentRasterImage* a_pImage, TImagePoint a_tOrigin, TImageSize a_tSize, TPixelChannel* a_pPixels, fnDeleteBuffer a_pDeleter, bool a_bReplace)
	{
		m_pImage = a_pImage;
		m_tOrigin = a_tOrigin;
		m_tSize = a_tSize;
		m_pPixels = a_pPixels;
		m_pDeleter = a_pDeleter;
		m_bReplace = a_bReplace;
	}
	CUndoStepPixelTile() : m_pPixels(NULL)
	{
	}
	~CUndoStepPixelTile()
	{
		(*m_pDeleter)(m_pPixels);
	}

	// IDocumentUndoStep methods
public:
	STDMETHOD(Execute)()
	{
		return m_pImage->TileSet(EICIRGBA, &m_tOrigin, &m_tSize, NULL, m_tSize.nX*m_tSize.nY, m_pPixels, m_bReplace);
	}
	STDMETHOD(MemorySize)(ULONGLONG* a_pnSize)
	{
		*a_pnSize = 96+m_tSize.nX*m_tSize.nY*sizeof*m_pPixels;
		return S_OK;
	}

private:
	CComPtr<IDocumentRasterImage> m_pImage;
	TImagePoint m_tOrigin;
	TImageSize m_tSize;
	TPixelChannel* m_pPixels;
	fnDeleteBuffer m_pDeleter;
	bool m_bReplace;
};

typedef CUndoStepImpl<CUndoStepPixelTile> CUndoPixelTile;


