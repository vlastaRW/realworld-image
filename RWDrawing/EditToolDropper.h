
#pragma once

#include "EditTool.h"
#include "EditToolWithCtrlDropper.h"


class CEditToolDropper :
	public CEditToolMouseInput<CEditToolDropper>, // no direct tablet support
	public CRasterImageEditToolImpl< // IRasterImageEditTool implementation mapper
		CEditToolDropper, // T - the top level class for cross casting
		CRasterImageEditToolBase, // TResetHandler
		CRasterImageEditToolBase, // TDirtyHandler
		CRasterImageEditToolBase, // TImageTileHandler
		CRasterImageEditToolBase, // TSelectionTileHandler
		CRasterImageEditToolBase, // TColorsHandler
		CRasterImageEditToolBase, // TBrushHandler
		CRasterImageEditToolBase, // TGlobalsHandler
		CRasterImageEditToolBase, // TAdjustCoordsHandler
		CEditToolDropper, // TGetCursorHandler
		CEditToolMouseInput<CEditToolDropper>, // TProcessInputHandler
		CRasterImageEditToolBase, // TPreTranslateMessageHandler
		CRasterImageEditToolBase, // TControlPointsHandler
		CRasterImageEditToolBase // TControlLinesHandler
	>
{
	// IRasterImageEditTool methods
public:
	STDMETHOD(OnMouseDown)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		TRasterImagePixel tPixel;
		if (SUCCEEDED(M_Window()->GetImageTile(a_pPos->fX, a_pPos->fY, 1, 1, 2.2f, 1, EITIContent, &tPixel)))
		{
			TColor t = {powf(tPixel.bR/255.0f, 2.2f), powf(tPixel.bG/255.0f, 2.2f), powf(tPixel.bB/255.0f, 2.2f), tPixel.bA/255.0f};
			ActivateSolidFill(M_Window(), t);
		}
		return S_OK;
	}
	STDMETHOD(OnMouseMove)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos)
	{
		if (M_Dragging())
			return OnMouseDown(a_eKeysState, a_pPos);
		return S_OK;
	}

	STDMETHOD(GetCursor)(EControlKeysState a_eKeysState, TPixelCoords const* a_pPos, HCURSOR* a_phCursor)
	{
		ULONG nSizeX = 0;
		ULONG nSizeY = 0;
		M_Window()->Size(&nSizeX, &nSizeY);
		if (a_pPos->fX < 0.0f || a_pPos->fY < 0.0f || a_pPos->fX >= nSizeX || a_pPos->fY >= nSizeY)
		{
			static HCURSOR h = ::LoadCursor(NULL, IDC_NO);
			*a_phCursor = h;
			return S_OK;
		}
		else
		{
			static OSVERSIONINFO tVersion = { sizeof(OSVERSIONINFO), 0, 0, 0, 0, _T("") };
			if (tVersion.dwMajorVersion == 0)
				GetVersionEx(&tVersion);
			if (tVersion.dwMajorVersion >= 5)
			{
				TRasterImagePixel tPixel = {0, 0, 0, 0};
				M_Window()->GetImageTile(a_pPos->fX, a_pPos->fY, 1, 1, 2.2f, 1, EITIContent, &tPixel);
				*a_phCursor = CPixelColorPicker::CreateDropperCursor(RGB(tPixel.bR, tPixel.bG, tPixel.bB)|(ULONG(tPixel.bA)<<24));
				return S_FALSE;
			}
			static HCURSOR h = ::LoadCursor(NULL, IDC_ARROW);
			*a_phCursor = h;
			return S_OK;
		}
	}
	STDMETHOD(PointTest)(EControlKeysState UNREF(a_eKeysState), TPixelCoords const* UNREF(a_pPos), BYTE UNREF(a_bAccurate), float UNREF(a_fPointSize))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(Transform)(TMatrix3x3f const* a_pMatrix)
	{
		return E_NOTIMPL;
	}
};
