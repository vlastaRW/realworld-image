
#include "stdafx.h"
#include "RWViewImageRaster.h"
#include "EditTool.h"
#include "ImageZoom.h"
#include <math.h>


#define RATIO_BPFIX 15
#define RATIO_ONE (1 << RATIO_BPFIX)
#define RATIO_MASK (~(~0 << RATIO_BPFIX))
typedef unsigned short ILTRATIO;
// type of element of precalculated arrays for fast bilinear zoom
typedef struct
{
	long d;             // integer distance
	ILTRATIO s1;     // fixed point number of result of 2nd ratio
} ILTZOOMINBILINEARELEM;
// type of element of precalculated arrays for fast bilinear zoom
typedef struct
{
	long lt;   // lt = weight of the most left/top pixel
	long rb;   // rb = weight of the most right/bottom pixel
	DWORD i;   // i = number of pixels between the most left and the most right pixels
	DWORD d;   // d = the delta to calculate the next interval
} ILTZOOMOUTBILINEARELEM;
// Calculate ratio
// p1 - variable, where to store the result
// t - parameter <0; 1> for ratio
#define CALCULATE_RATIO(p1, t) \
	((p1) = (ILTRATIO) ((t) * RATIO_ONE + 0.5))
// Calculate difference between pixels
// d - variable, where to store the result difference
// newIndex - index of new position (it should be double)
// lastIndex - index of the last position (it should be double)
// size - size of element (such as pixel, cell in temporary memory, line of pixels, ...)
#define CALCULATE_BILINEAR_DIFFERENCE(d, newIndex, lastIndex, size) \
	((d) = (((long) (newIndex) - (long) (lastIndex)) * 3 - 5) * (size))

void Zoom24bppBilinearIn_InitArray(ILTZOOMINBILINEARELEM *arr, DWORD inSize, DWORD outSize, UINT stride, UINT elemSize)
{
	ILTZOOMINBILINEARELEM *tmp, *tmpMax;
	double delta, lastIndex1 = 0.0, lastIndex2, t, maxIndex;

	lastIndex1 = -0.5;
	delta = (double) inSize / (outSize - 1);
	tmp = arr;
	while (lastIndex1 < 0.0)
	{
		CALCULATE_RATIO (tmp->s1, 0.0);
		lastIndex1 += delta;
		tmp->d = -2 * elemSize;
		tmp++;
	}
	maxIndex = inSize - 1;
	while (lastIndex1 < maxIndex)
	{
		t = lastIndex1 - (DWORD) lastIndex1;
		CALCULATE_RATIO (tmp->s1, t);
		lastIndex2 = lastIndex1;
		lastIndex1 += delta;
		CALCULATE_BILINEAR_DIFFERENCE (tmp->d, lastIndex1, lastIndex2, elemSize);
		tmp++;
	}
	// step back (on the last two pixels)
	(tmp - 1)->d -= elemSize;
	tmpMax = arr + outSize;
	while (tmp < tmpMax)
	{
		CALCULATE_RATIO (tmp->s1, 1.0);
		tmp->d = -2 * elemSize;
		tmp++;
	}
	// to jump on the beginning of the next row
	(tmp - 1)->d += stride - (inSize - 2) * elemSize;
}

void Zoom24bppBilinearOut_InitArray(ILTZOOMOUTBILINEARELEM *arr, DWORD inSize, DWORD outSize, int elemSize, unsigned long one)
{
	unsigned long lrb;
	double d1, d2, d;
	ILTZOOMOUTBILINEARELEM *tmpMaxPtr, *tmp;

	tmpMaxPtr = arr + outSize;
	lrb = 0x00000000;
	d2 = 0.0;
	for (d = 1.0, tmp = arr; tmp < tmpMaxPtr; tmp++, d+= 1.0)
	{
		d1 = d2;
		tmp->lt = one - lrb;
		d2 = d * inSize / outSize;
		lrb = tmp->rb = (long) ((d2 - floor (d2)) * one + 0.5);
		tmp->i = (int) (floor (d2) - floor (d1)) - 1;
		tmp->d = (tmp->i + 1) * elemSize;
		if (lrb == 0)
		{
			tmp->rb = one;
			if (tmp->i > 0)
				tmp->i--;
		}
	}
}

#define TEMP_BILINEAR_ELEM_BPFIX 8
#define TEMP_BILINEAR_ELEM_ONE (1 << TEMP_BILINEAR_ELEM_BPFIX)
typedef unsigned short ILTTEMPBILINEARELEM;

// Round the intensity (bilinear interpolation)
#define RGB_BILINEAR_CASTING(a) ((unsigned char) (((a) + (1 << (RATIO_BPFIX + TEMP_BILINEAR_ELEM_BPFIX - 1))) >> (RATIO_BPFIX + TEMP_BILINEAR_ELEM_BPFIX)))
#define RGB_BILINEAR_CASTING_TEMP(a) ((unsigned char) (((a) + (1 << (TEMP_BILINEAR_ELEM_BPFIX - 1))) >> TEMP_BILINEAR_ELEM_BPFIX))
#define RGB_BILINEAR_CASTING_RATIO(a) ((unsigned char) (((a) + (1 << (RATIO_BPFIX - 1))) >> RATIO_BPFIX))

void BilinearZoomInRgbLine(unsigned char *pDst, TRasterImagePixel const *pSrc, ILTZOOMINBILINEARELEM *pArr, unsigned long uLength)
{
	unsigned char *pDstMax = pDst + uLength * 4;

	while (pDst < pDstMax)
	{			
		int b = pSrc->bB;
		int g = pSrc->bG;
		int r = pSrc->bR;
		int a = pSrc->bA;
		++pSrc;
		
		b = (b << RATIO_BPFIX) + (pSrc->bB - b) * pArr->s1;
		g = (g << RATIO_BPFIX) + (pSrc->bG - g) * pArr->s1;
		r = (r << RATIO_BPFIX) + (pSrc->bR - r) * pArr->s1;
		a = (a << RATIO_BPFIX) + (pSrc->bA - a) * pArr->s1;
		++pSrc;

		*pDst++ = RGB_BILINEAR_CASTING_RATIO(b);
		*pDst++ = RGB_BILINEAR_CASTING_RATIO(g);
		*pDst++ = RGB_BILINEAR_CASTING_RATIO(r);
		*pDst++ = RGB_BILINEAR_CASTING_RATIO(a);
		
		pSrc += pArr->d;
		pArr++;
	}
}

// Conversion of color intensity value into TEMP_ELEM number. Intensity value was multiplied by RATIO number.
#define RATIO_TO_TEMP_BILINEAR_ELEM(a) ((ILTTEMPBILINEARELEM) ((a + (1 << (RATIO_BPFIX - TEMP_BILINEAR_ELEM_BPFIX - 1))) >> (RATIO_BPFIX - TEMP_BILINEAR_ELEM_BPFIX)))

void BilinearZoomInRgbLine(ILTTEMPBILINEARELEM *pDst, TRasterImagePixel const *pSrc, ILTZOOMINBILINEARELEM *pArr, unsigned long uLength)
{
	ILTTEMPBILINEARELEM *pDstMax = pDst + uLength * 4;

	while (pDst < pDstMax)
	{			
		int b = pSrc->bB;
		int g = pSrc->bG;
		int r = pSrc->bR;
		int a = pSrc->bA;
		++pSrc;
		
		b = (b << RATIO_BPFIX) + (pSrc->bB - b) * pArr->s1;
		g = (g << RATIO_BPFIX) + (pSrc->bG - g) * pArr->s1;
		r = (r << RATIO_BPFIX) + (pSrc->bR - r) * pArr->s1;
		a = (a << RATIO_BPFIX) + (pSrc->bA - a) * pArr->s1;
		++pSrc;

		*pDst++ = RATIO_TO_TEMP_BILINEAR_ELEM(b);
		*pDst++ = RATIO_TO_TEMP_BILINEAR_ELEM(g);
		*pDst++ = RATIO_TO_TEMP_BILINEAR_ELEM(r);
		*pDst++ = RATIO_TO_TEMP_BILINEAR_ELEM(a);
		
		pSrc += pArr->d;
		pArr++;
	}
}

void CombineRgbLine (unsigned char *pDst, unsigned char *pSrc0, unsigned char *pSrc1, unsigned long uLength, ILTRATIO ratio)
{
	unsigned char *pDstMax = pDst + uLength * 3;

	while (pDst < pDstMax)
	{
		int b = *pSrc0++;
		int g = *pSrc0++;
		int r = *pSrc0++;

		b = (b << RATIO_BPFIX) + (*pSrc1++ - b) * ratio;
		g = (g << RATIO_BPFIX) + (*pSrc1++ - g) * ratio;
		r = (r << RATIO_BPFIX) + (*pSrc1++ - r) * ratio;
		
		*pDst++ = RGB_BILINEAR_CASTING_RATIO (b);
		*pDst++ = RGB_BILINEAR_CASTING_RATIO (g);
		*pDst++ = RGB_BILINEAR_CASTING_RATIO (r);
	}
}

void CombineRgbLine (unsigned char *pDst, ILTTEMPBILINEARELEM *pSrc0, ILTTEMPBILINEARELEM *pSrc1, unsigned long uLength, ILTRATIO ratio)
{
	unsigned char *pDstMax = pDst + uLength * 3;

	while (pDst < pDstMax)
	{
		int b = *pSrc0++;
		int g = *pSrc0++;
		int r = *pSrc0++;

		b = (b << RATIO_BPFIX) + (*pSrc1++ - b) * ratio;
		g = (g << RATIO_BPFIX) + (*pSrc1++ - g) * ratio;
		r = (r << RATIO_BPFIX) + (*pSrc1++ - r) * ratio;
		
		*pDst++ = RGB_BILINEAR_CASTING (b);
		*pDst++ = RGB_BILINEAR_CASTING (g);
		*pDst++ = RGB_BILINEAR_CASTING (r);
	}
}


void Zoom24bppBilinearXinYin(TDrawCoord const& a_tSrcSize, TRasterImagePixel const* a_pSrc, TDrawCoord const& a_tDstSize, TRasterImagePixel* a_pDst)
{
	if (a_tSrcSize.nX < 2 || a_tSrcSize.nY < 2)
	{
		//return Zoom24bppNearest(pInImage, pOutImage);
	}

	// allocate memory for fast zoom in x coordinate
	CAutoVectorPtr<ILTZOOMINBILINEARELEM> xArr(new ILTZOOMINBILINEARELEM[a_tDstSize.nX]);
	// init array for x zoom
	Zoom24bppBilinearIn_InitArray(xArr, a_tSrcSize.nX, a_tDstSize.nX, sizeof(TRasterImagePixel)*a_tSrcSize.nX, sizeof(TRasterImagePixel));

	// allocate memory for fast zoom in y coordinate
	CAutoVectorPtr<ILTZOOMINBILINEARELEM> yArr(new ILTZOOMINBILINEARELEM[a_tDstSize.nY]);
	// init array for y zoom
	Zoom24bppBilinearIn_InitArray(yArr, a_tSrcSize.nY, a_tDstSize.nY, sizeof(ILTTEMPBILINEARELEM)*4*a_tSrcSize.nY, sizeof(ILTTEMPBILINEARELEM)*4);

	// allocate memory for temporary data
	CAutoVectorPtr<ILTTEMPBILINEARELEM> pTmpData(new ILTTEMPBILINEARELEM[2*a_tDstSize.nX*4]);

	// main zoom
	ILTTEMPBILINEARELEM *pSrc0 = pTmpData;
	ILTTEMPBILINEARELEM *pSrc1 = pSrc0 + a_tDstSize.nX*4;

	for (unsigned int y = 0; y < 2; y++)
		BilinearZoomInRgbLine(pTmpData + y*a_tDstSize.nX*4, a_pSrc+y*a_tSrcSize.nX, xArr, a_tDstSize.nX);

	TRasterImagePixel* yDst = a_pDst;

	unsigned int nY = 0, nYPrevious = nY;

	for (unsigned int y = 0; y < a_tDstSize.nY; y++)
	{
		CombineRgbLine(yDst, pSrc0, pSrc1, a_tDstSize.nX, yArr[y].s1);
		yDst += a_tDstSize.nX;

		nY += (10 + yArr[y].d) / 6;

		if (nY > nYPrevious)
		{
			nYPrevious = nY;

			ILTTEMPBILINEARELEM *pSrcTmp = pSrc0;
			pSrc0 = pSrc1;
			pSrc1 = pSrcTmp;

			if (nY + 1 < pInImage->dwHeight)
				BilinearZoomInRgbLine (pSrc1, pInImage->pDataBeg + (nY + 1) * pInImage->uBytesPerLine, xArr, pOutImage->dwWidth);
		}
	}
}


void LinearZoom(TDrawCoord const& a_tSrcSize, TRasterImagePixel const* a_pSrc, TDrawCoord const& a_tDstSize, TRasterImagePixel* a_pDst)
{
	if (a_tSrcSize.nY < a_tDstSize.nY && a_tSrcSize.nX < a_tDstSize.nX)
	{
		Zoom24bppBilinearXinYin(a_tSrcSize, a_pSrc, a_tDstSize, a_pDst);
	}
	//else if (a_tSrcSize.nY > a_tDstSize.nY && a_tSrcSize.nX > a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearXoutYout (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY == a_tDstSize.nY && a_tSrcSize.nX < a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearXin (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY < a_tDstSize.nY && a_tSrcSize.nX == a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearYin (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY == a_tDstSize.nY && a_tSrcSize.nX > a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearXout (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY > a_tDstSize.nY && a_tSrcSize.nX == a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearYout (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY > a_tDstSize.nY && a_tSrcSize.nX < a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearXinYout (pInImage, pOutImage);
	//}
	//else if (a_tSrcSize.nY < a_tDstSize.nY && a_tSrcSize.nX > a_tDstSize.nX)
	//{
	//	return Zoom24bppBilinearXoutYin (pInImage, pOutImage);
	//}

//	memcpy (pOutImage->pDataBeg, pInImage->pDataBeg, a_tSrcSize.nY * pInImage->uBytesPerLine);
}

