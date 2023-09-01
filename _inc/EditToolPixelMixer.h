
#pragma once

#include <GammaCorrection.h>


struct CPixelMixerReplace
{
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		a_tP1 = a_tP2;
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			a_tP1 = a_tP2;
		}
		else
		{
			// linear coverage-based blending for alpha channel
			ULONG const nInvCoverage = 255-a_nCoverage;
			ULONG const nWeigth = a_tP2.bA*a_nCoverage + a_tP1.bA*nInvCoverage;
			ULONG const bA1 = a_tP1.bA*nInvCoverage;
			a_tP1.bA = (nWeigth+(nWeigth>>7))>>8; // approximation of nWeigth/255;
			if (a_tP1.bA)
			{
				ULONG const bA2 = a_tP2.bA*a_nCoverage;
				a_tP1.bB = (a_tP1.bB*bA1 + a_tP2.bB*bA2)/nWeigth;
				a_tP1.bG = (a_tP1.bG*bA1 + a_tP2.bG*bA2)/nWeigth;
				a_tP1.bR = (a_tP1.bR*bA1 + a_tP2.bR*bA2)/nWeigth;
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_tP2.bA)
		{
			TRasterImagePixel const t = {a_tP2.bB*255/a_tP2.bA, a_tP2.bG*255/a_tP2.bA, a_tP2.bR*255/a_tP2.bA, a_tP2.bA};
			Mix(a_tP1, t, a_nCoverage);
		}
		else
		{
			Mix(a_tP1, a_tP2, a_nCoverage);
		}
	}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, WORD const* /*a_aGammaF*/, BYTE const* /*a_aGammaB*/)
	//{
	//	a_tP1 = a_tP2;
	//}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_nCoverage == 255)
	//	{
	//		a_tP1 = a_tP2;
	//	}
	//	else
	//	{
	//		// linear coverage-based blending for alpha channel
	//		ULONG const nInvCoverage = 255-a_nCoverage;
	//		ULONG const nWeigth = a_tP2.bA*a_nCoverage + a_tP1.bA*nInvCoverage;
	//		ULONG const bA1 = a_tP1.bA*nInvCoverage;
	//		a_tP1.bA = (nWeigth+(nWeigth>>7))>>8; // approximation of nWeigth/255;
	//		if (a_tP1.bA)
	//		{
	//			ULONG const bA2 = a_tP2.bA*a_nCoverage;
	//			a_tP1.bB = a_aGammaB[(a_aGammaF[a_tP1.bB]*bA1 + a_aGammaF[a_tP2.bB]*bA2)/nWeigth];
	//			a_tP1.bG = a_aGammaB[(a_aGammaF[a_tP1.bG]*bA1 + a_aGammaF[a_tP2.bG]*bA2)/nWeigth];
	//			a_tP1.bR = a_aGammaB[(a_aGammaF[a_tP1.bR]*bA1 + a_aGammaF[a_tP2.bR]*bA2)/nWeigth];
	//		}
	//		else
	//		{
	//			a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
	//		}
	//	}
	//}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, CGammaTables const* /*a_pGT*/)
	{
		a_tP1 = a_tP2;
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, CGammaTables const* a_pGT)
	{
		if (a_nCoverage == 255)
		{
			a_tP1 = a_tP2;
		}
		else
		{
			// linear coverage-based blending for alpha channel
			ULONG const nInvCoverage = 255-a_nCoverage;
			ULONG const nWeigth = a_tP2.bA*a_nCoverage + a_tP1.bA*nInvCoverage;
			ULONG const bA1 = a_tP1.bA*nInvCoverage;
			a_tP1.bA = (nWeigth+(nWeigth>>7))>>8; // approximation of nWeigth/255;
			if (a_tP1.bA)
			{
				ULONG const bA2 = a_tP2.bA*a_nCoverage;
				a_tP1.bB = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bB]*bA1 + a_pGT->m_aGamma[a_tP2.bB]*bA2)/nWeigth);
				a_tP1.bG = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bG]*bA1 + a_pGT->m_aGamma[a_tP2.bG]*bA2)/nWeigth);
				a_tP1.bR = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bR]*bA1 + a_pGT->m_aGamma[a_tP2.bR]*bA2)/nWeigth);
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
		}
	}
};

struct CPixelMixerPaintOver
{
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		if (a_tP2.bA == 255)
		{
			a_tP1 = a_tP2;
		}
		else
		{
			// blend pixels
			ULONG nNewA = a_tP2.bA*255 + (255-a_tP2.bA)*a_tP1.bA;
			if (nNewA)
			{
				ULONG const bA1 = (255-a_tP2.bA)*a_tP1.bA;
				ULONG const bA2 = a_tP2.bA*255;
				a_tP1.bB = (a_tP1.bB*bA1 + a_tP2.bB*bA2)/nNewA;
				a_tP1.bG = (a_tP1.bG*bA1 + a_tP2.bG*bA2)/nNewA;
				a_tP1.bR = (a_tP1.bR*bA1 + a_tP2.bR*bA2)/nNewA;
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
			a_tP1.bA = nNewA/255;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc);
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		if (a_tP2.bA == 255)
		{
			a_tP1 = a_tP2;
		}
		else
		{
			// blend pixels
			ULONG nNewA = a_tP2.bA*255 + (255-a_tP2.bA)*a_tP1.bA;
			if (nNewA)
			{
				ULONG const bA1 = (255-a_tP2.bA)*a_tP1.bA;
				a_tP1.bB = (a_tP1.bB*bA1 + a_tP2.bB*65025)/nNewA;
				a_tP1.bG = (a_tP1.bG*bA1 + a_tP2.bG*65025)/nNewA;
				a_tP1.bR = (a_tP1.bR*bA1 + a_tP2.bR*65025)/nNewA;
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
			a_tP1.bA = nNewA/255;
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			MixPM(a_tP1, a_tP2);
		}
		else
		{
			ULONG n = a_nCoverage+(a_nCoverage>>7);
			TRasterImagePixel const tSrc = {(n*a_tP2.bB)>>8, (n*a_tP2.bG)>>8, (n*a_tP2.bR)>>8, (n*a_tP2.bA)>>8};
			MixPM(a_tP1, tSrc);
		}
		//Mix(a_tP1, a_tP2, a_nCoverage);
	}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_tP2.bA == 255)
	//	{
	//		a_tP1 = a_tP2;
	//	}
	//	else
	//	{
	//		// blend pixels
	//		ULONG nNewA = a_tP2.bA*255 + (255-a_tP2.bA)*a_tP1.bA;
	//		if (nNewA)
	//		{
	//			ULONG const bA1 = (255-a_tP2.bA)*a_tP1.bA;
	//			ULONG const bA2 = a_tP2.bA*255;
	//			a_tP1.bB = a_aGammaB[(a_aGammaF[a_tP1.bB]*bA1 + a_aGammaF[a_tP2.bB]*bA2)/nNewA];
	//			a_tP1.bG = a_aGammaB[(a_aGammaF[a_tP1.bG]*bA1 + a_aGammaF[a_tP2.bG]*bA2)/nNewA];
	//			a_tP1.bR = a_aGammaB[(a_aGammaF[a_tP1.bR]*bA1 + a_aGammaF[a_tP2.bR]*bA2)/nNewA];
	//		}
	//		else
	//		{
	//			a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
	//		}
	//		a_tP1.bA = nNewA/255;
	//	}
	//}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_nCoverage == 255)
	//	{
	//		Mix(a_tP1, a_tP2, a_aGammaF, a_aGammaB);
	//	}
	//	else
	//	{
	//		TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
	//		Mix(a_tP1, tSrc, a_aGammaF, a_aGammaB);
	//	}
	//}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, CGammaTables const* a_pGT)
	{
		if (a_tP2.bA == 255)
		{
			a_tP1 = a_tP2;
		}
		else
		{
			// blend pixels
			ULONG nNewA = a_tP2.bA*255 + (255-a_tP2.bA)*a_tP1.bA;
			if (nNewA)
			{
				ULONG const bA1 = (255-a_tP2.bA)*a_tP1.bA;
				ULONG const bA2 = a_tP2.bA*255;
				a_tP1.bB = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bB]*bA1 + a_pGT->m_aGamma[a_tP2.bB]*bA2)/nNewA);
				a_tP1.bG = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bG]*bA1 + a_pGT->m_aGamma[a_tP2.bG]*bA2)/nNewA);
				a_tP1.bR = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP1.bR]*bA1 + a_pGT->m_aGamma[a_tP2.bR]*bA2)/nNewA);
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
			a_tP1.bA = nNewA/255;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, CGammaTables const* a_pGT)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2, a_pGT);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc, a_pGT);
		}
	}
};

struct CPixelMixerPaintUnder
{
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		if (a_tP1.bA == 0)
		{
			if (a_tP2.bA == 0)
			{
				// agg somtimes produces invalid pixels (a=0 + rgb!=0)
				static TRasterImagePixel const t0 = {0, 0, 0, 0};
				a_tP1 = t0;
			}
			else
			{
				a_tP1 = a_tP2;
			}
		}
		else if (a_tP1.bA != 255)
		{
			// blend pixels
			ULONG nNewA = a_tP1.bA*255 + (255-a_tP1.bA)*a_tP2.bA;
			if (nNewA)
			{
				ULONG const bA1 = (255-a_tP1.bA)*a_tP2.bA;
				ULONG const bA2 = a_tP1.bA*255;
				a_tP1.bB = (a_tP2.bB*bA1 + a_tP1.bB*bA2)/nNewA;
				a_tP1.bG = (a_tP2.bG*bA1 + a_tP1.bG*bA2)/nNewA;
				a_tP1.bR = (a_tP2.bR*bA1 + a_tP1.bR*bA2)/nNewA;
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
			a_tP1.bA = nNewA/255;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc);
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_tP2.bA)
		{
			TRasterImagePixel const t = {a_tP2.bB*255/a_tP2.bA, a_tP2.bG*255/a_tP2.bA, a_tP2.bR*255/a_tP2.bA, a_tP2.bA};
			Mix(a_tP1, t, a_nCoverage);
		}
		else
		{
			Mix(a_tP1, a_tP2, a_nCoverage);
		}
	}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_tP1.bA == 0)
	//	{
	//		if (a_tP2.bA == 0)
	//		{
	//			// agg somtimes produces invalid pixels (a=0 + rgb!=0)
	//			static TRasterImagePixel const t0 = {0, 0, 0, 0};
	//			a_tP1 = t0;
	//		}
	//		else
	//		{
	//			a_tP1 = a_tP2;
	//		}
	//	}
	//	else if (a_tP1.bA != 255)
	//	{
	//		// blend pixels
	//		ULONG nNewA = a_tP1.bA*255 + (255-a_tP1.bA)*a_tP2.bA;
	//		if (nNewA)
	//		{
	//			ULONG const bA1 = (255-a_tP1.bA)*a_tP2.bA;
	//			ULONG const bA2 = a_tP1.bA*255;
	//			a_tP1.bB = a_aGammaB[(a_aGammaF[a_tP2.bB]*bA1 + a_aGammaF[a_tP1.bB]*bA2)/nNewA];
	//			a_tP1.bG = a_aGammaB[(a_aGammaF[a_tP2.bG]*bA1 + a_aGammaF[a_tP1.bG]*bA2)/nNewA];
	//			a_tP1.bR = a_aGammaB[(a_aGammaF[a_tP2.bR]*bA1 + a_aGammaF[a_tP1.bR]*bA2)/nNewA];
	//		}
	//		else
	//		{
	//			a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
	//		}
	//		a_tP1.bA = nNewA/255;
	//	}
	//}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_nCoverage == 255)
	//	{
	//		Mix(a_tP1, a_tP2, a_aGammaF, a_aGammaB);
	//	}
	//	else
	//	{
	//		TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
	//		Mix(a_tP1, tSrc, a_aGammaF, a_aGammaB);
	//	}
	//}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, CGammaTables const* a_pGT)
	{
		if (a_tP1.bA == 0)
		{
			if (a_tP2.bA == 0)
			{
				// agg somtimes produces invalid pixels (a=0 + rgb!=0)
				static TRasterImagePixel const t0 = {0, 0, 0, 0};
				a_tP1 = t0;
			}
			else
			{
				a_tP1 = a_tP2;
			}
		}
		else if (a_tP1.bA != 255)
		{
			// blend pixels
			ULONG nNewA = a_tP1.bA*255 + (255-a_tP1.bA)*a_tP2.bA;
			if (nNewA)
			{
				ULONG const bA1 = (255-a_tP1.bA)*a_tP2.bA;
				ULONG const bA2 = a_tP1.bA*255;
				a_tP1.bB = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP2.bB]*bA1 + a_pGT->m_aGamma[a_tP1.bB]*bA2)/nNewA);
				a_tP1.bG = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP2.bG]*bA1 + a_pGT->m_aGamma[a_tP1.bG]*bA2)/nNewA);
				a_tP1.bR = a_pGT->InvGamma((a_pGT->m_aGamma[a_tP2.bR]*bA1 + a_pGT->m_aGamma[a_tP1.bR]*bA2)/nNewA);
			}
			else
			{
				a_tP1.bB = a_tP1.bG = a_tP1.bR = 0;
			}
			a_tP1.bA = nNewA/255;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, CGammaTables const* a_pGT)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2, a_pGT);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc, a_pGT);
		}
	}
};

struct CPixelMixerAdd
{
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		if (a_tP2.bA != 0)
		{
			ULONG nNewA = ULONG(a_tP2.bA) + ULONG(a_tP1.bA);
			if (nNewA > 255) nNewA = 255;
			ULONG const nNewB = (ULONG(a_tP2.bB)*ULONG(a_tP2.bA) + ULONG(a_tP1.bB)*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewG = (ULONG(a_tP2.bG)*ULONG(a_tP2.bA) + ULONG(a_tP1.bG)*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewR = (ULONG(a_tP2.bR)*ULONG(a_tP2.bA) + ULONG(a_tP1.bR)*ULONG(a_tP1.bA))/nNewA;
			a_tP1.bB = nNewB > 255 ? 255 : nNewB;
			a_tP1.bG = nNewG > 255 ? 255 : nNewG;
			a_tP1.bR = nNewR > 255 ? 255 : nNewR;
			a_tP1.bA = nNewA;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc);
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2)
	{
		if (a_tP2.bA != 0)
		{
			ULONG const nNewA = ULONG(a_tP2.bA) + ULONG(a_tP1.bA);
			ULONG const nNewB = (ULONG(a_tP2.bB) + ULONG(a_tP1.bB)*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewG = (ULONG(a_tP2.bG) + ULONG(a_tP1.bG)*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewR = (ULONG(a_tP2.bR) + ULONG(a_tP1.bR)*ULONG(a_tP1.bA))/nNewA;
			a_tP1.bB = nNewB > 255 ? 255 : nNewB;
			a_tP1.bG = nNewG > 255 ? 255 : nNewG;
			a_tP1.bR = nNewR > 255 ? 255 : nNewR;
			a_tP1.bA = nNewA > 255 ? 255 : nNewA;
		}
	}
	static void MixPM(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage)
	{
		if (a_nCoverage == 255)
		{
			MixPM(a_tP1, a_tP2);
		}
		else
		{
			ULONG n = a_nCoverage+(a_nCoverage>>7);
			TRasterImagePixel const tSrc = {(n*a_tP2.bB)>>8, (n*a_tP2.bG)>>8, (n*a_tP2.bR)>>8, (n*a_tP2.bA)>>8};
			MixPM(a_tP1, tSrc);
		}
		//Mix(a_tP1, a_tP2, a_nCoverage);
	}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_tP2.bA != 0)
	//	{
	//		ULONG nNewA = ULONG(a_tP2.bA) + ULONG(a_tP1.bA);
	//		if (nNewA > 255) nNewA = 255;
	//		ULONG const nNewB = (ULONG(a_aGammaF[a_tP2.bB])*ULONG(a_tP2.bA) + ULONG(a_aGammaF[a_tP1.bB])*ULONG(a_tP1.bA))/nNewA;
	//		ULONG const nNewG = (ULONG(a_aGammaF[a_tP2.bG])*ULONG(a_tP2.bA) + ULONG(a_aGammaF[a_tP1.bG])*ULONG(a_tP1.bA))/nNewA;
	//		ULONG const nNewR = (ULONG(a_aGammaF[a_tP2.bR])*ULONG(a_tP2.bA) + ULONG(a_aGammaF[a_tP1.bR])*ULONG(a_tP1.bA))/nNewA;
	//		a_tP1.bB = a_aGammaB[nNewB > 1023 ? 1023 : nNewB];
	//		a_tP1.bG = a_aGammaB[nNewG > 1023 ? 1023 : nNewG];
	//		a_tP1.bR = a_aGammaB[nNewR > 1023 ? 1023 : nNewR];
	//		a_tP1.bA = nNewA;
	//	}
	//}
	//static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, WORD const* a_aGammaF, BYTE const* a_aGammaB)
	//{
	//	if (a_nCoverage == 255)
	//	{
	//		Mix(a_tP1, a_tP2, a_aGammaF, a_aGammaB);
	//	}
	//	else
	//	{
	//		TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
	//		Mix(a_tP1, tSrc, a_aGammaF, a_aGammaB);
	//	}
	//}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, CGammaTables const* a_pGT)
	{
		if (a_tP2.bA != 0)
		{
			ULONG nNewA = ULONG(a_tP2.bA) + ULONG(a_tP1.bA);
			if (nNewA > 255) nNewA = 255;
			ULONG const nNewB = (ULONG(a_pGT->m_aGamma[a_tP2.bB])*ULONG(a_tP2.bA) + ULONG(a_pGT->m_aGamma[a_tP1.bB])*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewG = (ULONG(a_pGT->m_aGamma[a_tP2.bG])*ULONG(a_tP2.bA) + ULONG(a_pGT->m_aGamma[a_tP1.bG])*ULONG(a_tP1.bA))/nNewA;
			ULONG const nNewR = (ULONG(a_pGT->m_aGamma[a_tP2.bR])*ULONG(a_tP2.bA) + ULONG(a_pGT->m_aGamma[a_tP1.bR])*ULONG(a_tP1.bA))/nNewA;
			a_tP1.bB = a_pGT->InvGamma(nNewB > 65535 ? 65535 : nNewB);
			a_tP1.bG = a_pGT->InvGamma(nNewG > 65535 ? 65535 : nNewG);
			a_tP1.bR = a_pGT->InvGamma(nNewR > 65535 ? 65535 : nNewR);
			a_tP1.bA = nNewA;
		}
	}
	static void Mix(TRasterImagePixel& a_tP1, TRasterImagePixel const& a_tP2, ULONG a_nCoverage, CGammaTables const* a_pGT)
	{
		if (a_nCoverage == 255)
		{
			Mix(a_tP1, a_tP2, a_pGT);
		}
		else
		{
			TRasterImagePixel const tSrc = {a_tP2.bB, a_tP2.bG, a_tP2.bR, ((a_nCoverage+(a_nCoverage>>7))*a_tP2.bA)>>8};
			Mix(a_tP1, tSrc, a_pGT);
		}
	}
};
