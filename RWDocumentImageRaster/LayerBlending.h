
#pragma once

template<typename TPixel>
void MergeLayerLine(ELayerBlend a_eBlend, ULONG a_nSizeX, TPixel* a_pDst, TPixel const* a_pSrc1, TPixel const* a_pSrc2, CGammaTables const& a_cGamma)// bool a_bGamma, WORD const* a_pGammaF, BYTE const* a_pGammaB)
{
	TPixel* const pEnd = a_pDst+a_nSizeX;

	switch (a_eBlend)
	{
	case EBEAlphaBlend:
	case EBEGlass:
		if (true)//a_bGamma)
		{
			while (a_pDst < pEnd)
			{
				if (a_pSrc2->bA == 0)
				{
					*a_pDst = *a_pSrc1;
				}
				else
				{
					if (a_pSrc2->bA == 255 || a_pSrc1->bA == 0)
					{
						*a_pDst = *a_pSrc2;
					}
					else
					{
						ULONG nNewA = a_pSrc2->bA*255 + (255-a_pSrc2->bA)*static_cast<ULONG>(a_pSrc1->bA);
						if (nNewA)
						{
							a_pDst->bR = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc1->bR])*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bR])*a_pSrc2->bA*255)/nNewA);
							a_pDst->bG = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc1->bG])*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bG])*a_pSrc2->bA*255)/nNewA);
							a_pDst->bB = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc1->bB])*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bB])*a_pSrc2->bA*255)/nNewA);
						}
						else
						{
							a_pDst->bR = a_pDst->bG = a_pDst->bB = 0;
						}
						a_pDst->bA = nNewA/255;
					}
				}
				++a_pDst;
				++a_pSrc1;
				++a_pSrc2;
			}
		}
		else
		{
			while (a_pDst < pEnd)
			{
				if (a_pSrc2->bA == 0)
				{
					*a_pDst = *a_pSrc1;
				}
				else
				{
					if (a_pSrc2->bA == 255 || a_pSrc1->bA == 0)
					{
						*a_pDst = *a_pSrc2;
					}
					else
					{
						ULONG nNewA = a_pSrc2->bA*255 + (255-a_pSrc2->bA)*static_cast<ULONG>(a_pSrc1->bA);
						if (nNewA)
						{
							a_pDst->bR = (static_cast<ULONG>(a_pSrc1->bR)*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_pSrc2->bR)*a_pSrc2->bA*255)/nNewA;
							a_pDst->bG = (static_cast<ULONG>(a_pSrc1->bG)*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_pSrc2->bG)*a_pSrc2->bA*255)/nNewA;
							a_pDst->bB = (static_cast<ULONG>(a_pSrc1->bB)*(255-a_pSrc2->bA)*a_pSrc1->bA + static_cast<ULONG>(a_pSrc2->bB)*a_pSrc2->bA*255)/nNewA;
						}
						else
						{
							a_pDst->bR = a_pDst->bG = a_pDst->bB = 0;
						}
						a_pDst->bA = nNewA/255;
					}
				}
				++a_pDst;
				++a_pSrc1;
				++a_pSrc2;
			}
		}
		break;
	case EBEModulate:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_pSrc1->bR) * a_pSrc2->bR + 0x80;
                ULONG const nR = ((t >> 8) + t) >> 8;
                t = ULONG(a_pSrc1->bG) * a_pSrc2->bG + 0x80;
                ULONG const nG = ((t >> 8) + t) >> 8;
                t = ULONG(a_pSrc1->bB) * a_pSrc2->bB + 0x80;
                ULONG const nB = ((t >> 8) + t) >> 8;

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEScreen:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = (255-a_pSrc1->bR) * (255-a_pSrc2->bR) + 0x80;
                ULONG const nR = 255-(((t >> 8) + t) >> 8);
                t = (255-a_pSrc1->bG) * (255-a_pSrc2->bG) + 0x80;
                ULONG const nG = 255-(((t >> 8) + t) >> 8);
                t = (255-a_pSrc1->bB) * (255-a_pSrc2->bB) + 0x80;
                ULONG const nB = 255-(((t >> 8) + t) >> 8);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEAdd:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_pSrc1->bR) + a_pSrc2->bR;
                ULONG const nR = min(255, t);
                t = ULONG(a_pSrc1->bG) + a_pSrc2->bG;
                ULONG const nG = min(255, t);
                t = ULONG(a_pSrc1->bB) + a_pSrc2->bB;
                ULONG const nB = min(255, t);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBESubtract:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				LONG t = LONG(a_pSrc1->bR) - a_pSrc2->bR;
                ULONG const nR = max(0, t);
                t = LONG(a_pSrc1->bG) - a_pSrc2->bG;
                ULONG const nG = max(0, t);
                t = LONG(a_pSrc1->bB) - a_pSrc2->bB;
                ULONG const nB = max(0, t);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEAverage:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);
				a1 += a>>1;
				a2 += a>>1;

                a_pDst->bR = (a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEDifference:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG const nR = a_pSrc1->bR > a_pSrc2->bR ? (a_pSrc1->bR-a_pSrc2->bR) : (a_pSrc2->bR-a_pSrc1->bR);
                ULONG const nG = a_pSrc1->bG > a_pSrc2->bG ? (a_pSrc1->bG-a_pSrc2->bG) : (a_pSrc2->bG-a_pSrc1->bG);
                ULONG const nB = a_pSrc1->bB > a_pSrc2->bB ? (a_pSrc1->bB-a_pSrc2->bB) : (a_pSrc2->bB-a_pSrc1->bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEMinimum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = min(a_pSrc1->bR, a_pSrc2->bR);
                ULONG const nG = min(a_pSrc1->bG, a_pSrc2->bG);
                ULONG const nB = min(a_pSrc1->bB, a_pSrc2->bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEMaximum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = max(a_pSrc1->bR, a_pSrc2->bR);
                ULONG const nG = max(a_pSrc1->bG, a_pSrc2->bG);
                ULONG const nB = max(a_pSrc1->bB, a_pSrc2->bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEOverlay:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG nR;
				if (a_pSrc1->bR < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bR) * a_pSrc2->bR) << 1;
					nR = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bR) * (255-a_pSrc2->bR) << 1;
					nR = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nR > 255) nR = 255;

				ULONG nG;
				if (a_pSrc1->bG < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bG) * a_pSrc2->bG) << 1;
					nG = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bG) * (255-a_pSrc2->bG) << 1;
					nG = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nG > 255) nG = 255;

				ULONG nB;
				if (a_pSrc1->bB < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bB) * a_pSrc2->bB) << 1;
					nB = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bB) * (255-a_pSrc2->bB) << 1;
					nB = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nB > 255) nB = 255;

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceHue:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_pSrc2->bR == rgbmax2)
						nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
					else if (a_pSrc2->bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_pSrc1->bR == rgbmax1)
				//		nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_pSrc1->bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS1)/0xffff : nS1 - (nL1*nS1)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceSaturation:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_pSrc2->bR == rgbmax2)
				//		nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_pSrc2->bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_pSrc1->bR == rgbmax1)
						nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
					else if (a_pSrc1->bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceLuminance:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				//unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				//unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_pSrc2->bR == rgbmax2)
				//		nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_pSrc2->bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_pSrc1->bR == rgbmax1)
						nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
					else if (a_pSrc1->bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL2 + (nL2 <= 0x7fff ? (nL2*nS1)/0xffff : nS1 - (nL2*nS1)/0xffff);
				unsigned int m1 = (nL2<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceColor:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = *a_pSrc1;
			}
			else if (a_pSrc1->bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_pSrc2->bR == rgbmax2)
						nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
					else if (a_pSrc2->bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				//unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				//unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_pSrc1->bR == rgbmax1)
				//		nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_pSrc1->bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	case EBEMultiplyInvAlpha:
		while (a_pDst < pEnd)
		{
			a_pDst->bR = a_pSrc1->bR;
			a_pDst->bG = a_pSrc1->bG;
			a_pDst->bB = a_pSrc1->bB;
			a_pDst->bA = ULONG(a_pSrc1->bA)*(255-a_pSrc2->bA)/255;
			++a_pDst;
			++a_pSrc1;
			++a_pSrc2;
		}
		break;
	default:
		ATLASSERT(0);
	}
}


template<typename TPixel>
void MergeLayerLine(ELayerBlend a_eBlend, ULONG a_nSizeX, TPixel* a_pDst, TPixel const* a_pSrc1, TPixel const a_tSrc2, CGammaTables const& a_cGamma)//bool a_bGamma, WORD const* a_pGammaF, BYTE const* a_pGammaB)
{
	if (a_tSrc2.bA == 0)
	{
		if (a_pDst != a_pSrc1)
			CopyMemory(a_pDst, a_pSrc1, sizeof(TPixel)*a_nSizeX);
		return;
	}

	TPixel* const pEnd = a_pDst+a_nSizeX;

	switch (a_eBlend)
	{
	case EBEAlphaBlend:
	case EBEGlass:
		if (a_tSrc2.bA == 255)
		{
			std::fill_n(a_pDst, a_nSizeX, a_tSrc2);
		}
		else if (true)//a_bGamma)
		{
			ULONG const nA2_255 = a_tSrc2.bA*255;
			ULONG const nIA2 = 255-a_tSrc2.bA;
			ULONG const nR2 = nA2_255*a_cGamma.m_aGamma[a_tSrc2.bR];
			ULONG const nG2 = nA2_255*a_cGamma.m_aGamma[a_tSrc2.bG];
			ULONG const nB2 = nA2_255*a_cGamma.m_aGamma[a_tSrc2.bB];
			while (a_pDst < pEnd)
			{
				if (a_pSrc1->bA == 0)
				{
					*a_pDst = a_tSrc2;
				}
				else
				{
					ULONG nNewA = nA2_255 + nIA2*a_pSrc1->bA;
					a_pDst->bR = a_cGamma.InvGamma((a_cGamma.m_aGamma[a_pSrc1->bR]*nIA2*a_pSrc1->bA + nR2)/nNewA);
					a_pDst->bG = a_cGamma.InvGamma((a_cGamma.m_aGamma[a_pSrc1->bG]*nIA2*a_pSrc1->bA + nG2)/nNewA);
					a_pDst->bB = a_cGamma.InvGamma((a_cGamma.m_aGamma[a_pSrc1->bB]*nIA2*a_pSrc1->bA + nB2)/nNewA);
					a_pDst->bA = nNewA/255;
				}
				++a_pDst;
				++a_pSrc1;
			}
		}
		else
		{
			ULONG const nA2_255 = a_tSrc2.bA*255;
			ULONG const nIA2 = 255-a_tSrc2.bA;
			ULONG const nR2 = nA2_255*a_tSrc2.bR;
			ULONG const nG2 = nA2_255*a_tSrc2.bG;
			ULONG const nB2 = nA2_255*a_tSrc2.bB;
			while (a_pDst < pEnd)
			{
				if (a_pSrc1->bA == 0)
				{
					*a_pDst = a_tSrc2;
				}
				else
				{
					ULONG nNewA = nA2_255 + nIA2*a_pSrc1->bA;
					a_pDst->bR = (a_pSrc1->bR*nIA2*a_pSrc1->bA + nR2)/nNewA;
					a_pDst->bG = (a_pSrc1->bG*nIA2*a_pSrc1->bA + nG2)/nNewA;
					a_pDst->bB = (a_pSrc1->bB*nIA2*a_pSrc1->bA + nB2)/nNewA;
					a_pDst->bA = nNewA/255;
				}
				++a_pDst;
				++a_pSrc1;
			}
		}
		break;
	case EBEModulate:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_pSrc1->bR) * a_tSrc2.bR + 0x80;
                ULONG const nR = ((t >> 8) + t) >> 8;
                t = ULONG(a_pSrc1->bG) * a_tSrc2.bG + 0x80;
                ULONG const nG = ((t >> 8) + t) >> 8;
                t = ULONG(a_pSrc1->bB) * a_tSrc2.bB + 0x80;
                ULONG const nB = ((t >> 8) + t) >> 8;

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEScreen:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = (255-a_pSrc1->bR) * (255-a_tSrc2.bR) + 0x80;
                ULONG const nR = 255-(((t >> 8) + t) >> 8);
                t = (255-a_pSrc1->bG) * (255-a_tSrc2.bG) + 0x80;
                ULONG const nG = 255-(((t >> 8) + t) >> 8);
                t = (255-a_pSrc1->bB) * (255-a_tSrc2.bB) + 0x80;
                ULONG const nB = 255-(((t >> 8) + t) >> 8);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEAdd:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_pSrc1->bR) + a_tSrc2.bR;
                ULONG const nR = min(255, t);
                t = ULONG(a_pSrc1->bG) + a_tSrc2.bG;
                ULONG const nG = min(255, t);
                t = ULONG(a_pSrc1->bB) + a_tSrc2.bB;
                ULONG const nB = min(255, t);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBESubtract:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				LONG t = LONG(a_pSrc1->bR) - a_tSrc2.bR;
                ULONG const nR = max(0, t);
                t = LONG(a_pSrc1->bG) - a_tSrc2.bG;
                ULONG const nG = max(0, t);
                t = LONG(a_pSrc1->bB) - a_tSrc2.bB;
                ULONG const nB = max(0, t);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEAverage:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);
				a1 += a>>1;
				a2 += a>>1;

                a_pDst->bR = (a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEDifference:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG const nR = a_pSrc1->bR > a_tSrc2.bR ? (a_pSrc1->bR-a_tSrc2.bR) : (a_tSrc2.bR-a_pSrc1->bR);
                ULONG const nG = a_pSrc1->bG > a_tSrc2.bG ? (a_pSrc1->bG-a_tSrc2.bG) : (a_tSrc2.bG-a_pSrc1->bG);
                ULONG const nB = a_pSrc1->bB > a_tSrc2.bB ? (a_pSrc1->bB-a_tSrc2.bB) : (a_tSrc2.bB-a_pSrc1->bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEMinimum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = min(a_pSrc1->bR, a_tSrc2.bR);
                ULONG const nG = min(a_pSrc1->bG, a_tSrc2.bG);
                ULONG const nB = min(a_pSrc1->bB, a_tSrc2.bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEMaximum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = max(a_pSrc1->bR, a_tSrc2.bR);
                ULONG const nG = max(a_pSrc1->bG, a_tSrc2.bG);
                ULONG const nB = max(a_pSrc1->bB, a_tSrc2.bB);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEOverlay:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG nR;
				if (a_pSrc1->bR < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bR) * a_tSrc2.bR) << 1;
					nR = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bR) * (255-a_tSrc2.bR) << 1;
					nR = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nR > 255) nR = 255;

				ULONG nG;
				if (a_pSrc1->bG < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bG) * a_tSrc2.bG) << 1;
					nG = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bG) * (255-a_tSrc2.bG) << 1;
					nG = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nG > 255) nG = 255;

				ULONG nB;
				if (a_pSrc1->bB < 128)
				{
					ULONG const t = (ULONG(a_pSrc1->bB) * a_tSrc2.bB) << 1;
					nB = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_pSrc1->bB) * (255-a_tSrc2.bB) << 1;
					nB = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nB > 255) nB = 255;

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case ELBHLSReplaceHue:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_tSrc2.bR>a_tSrc2.bG ? (a_tSrc2.bR>a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG>a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				int const rgbmin2 = a_tSrc2.bR<a_tSrc2.bG ? (a_tSrc2.bR<a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG<a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_tSrc2.bR == rgbmax2)
						nH2 = 0xffff&((a_tSrc2.bG - a_tSrc2.bB)*0xffff / int(rgbdelta2*6));
					else if (a_tSrc2.bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_tSrc2.bB - a_tSrc2.bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_tSrc2.bR - a_tSrc2.bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_pSrc1->bR == rgbmax1)
				//		nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_pSrc1->bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS1)/0xffff : nS1 - (nL1*nS1)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case ELBHLSReplaceSaturation:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_tSrc2.bR>a_tSrc2.bG ? (a_tSrc2.bR>a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG>a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				int const rgbmin2 = a_tSrc2.bR<a_tSrc2.bG ? (a_tSrc2.bR<a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG<a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_tSrc2.bR == rgbmax2)
				//		nH2 = 0xffff&((a_tSrc2.bG - a_tSrc2.bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_tSrc2.bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_tSrc2.bB - a_tSrc2.bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_tSrc2.bR - a_tSrc2.bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_pSrc1->bR == rgbmax1)
						nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
					else if (a_pSrc1->bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case ELBHLSReplaceLuminance:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_tSrc2.bR>a_tSrc2.bG ? (a_tSrc2.bR>a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG>a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				int const rgbmin2 = a_tSrc2.bR<a_tSrc2.bG ? (a_tSrc2.bR<a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG<a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				//unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				//unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_tSrc2.bR == rgbmax2)
				//		nH2 = 0xffff&((a_tSrc2.bG - a_tSrc2.bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_tSrc2.bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_tSrc2.bB - a_tSrc2.bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_tSrc2.bR - a_tSrc2.bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_pSrc1->bR == rgbmax1)
						nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
					else if (a_pSrc1->bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL2 + (nL2 <= 0x7fff ? (nL2*nS1)/0xffff : nS1 - (nL2*nS1)/0xffff);
				unsigned int m1 = (nL2<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case ELBHLSReplaceColor:
		while (a_pDst < pEnd)
		{
			if (a_pSrc1->bA == 0)
			{
				*a_pDst = a_tSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_pSrc1->bA)*ULONG(a_tSrc2.bA);
				ULONG const nA = ULONG(a_tSrc2.bA) + ULONG(a_pSrc1->bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_tSrc2.bA)*a_pSrc1->bA;
				ULONG const a2 = (255-a_pSrc1->bA)*a_tSrc2.bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_tSrc2.bR>a_tSrc2.bG ? (a_tSrc2.bR>a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG>a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				int const rgbmin2 = a_tSrc2.bR<a_tSrc2.bG ? (a_tSrc2.bR<a_tSrc2.bB ? a_tSrc2.bR : a_tSrc2.bB) : (a_tSrc2.bG<a_tSrc2.bB ? a_tSrc2.bG : a_tSrc2.bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_tSrc2.bR == rgbmax2)
						nH2 = 0xffff&((a_tSrc2.bG - a_tSrc2.bB)*0xffff / int(rgbdelta2*6));
					else if (a_tSrc2.bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_tSrc2.bB - a_tSrc2.bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_tSrc2.bR - a_tSrc2.bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_pSrc1->bR>a_pSrc1->bG ? (a_pSrc1->bR>a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG>a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				int const rgbmin1 = a_pSrc1->bR<a_pSrc1->bG ? (a_pSrc1->bR<a_pSrc1->bB ? a_pSrc1->bR : a_pSrc1->bB) : (a_pSrc1->bG<a_pSrc1->bB ? a_pSrc1->bG : a_pSrc1->bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				//unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				//unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_pSrc1->bR == rgbmax1)
				//		nH1 = 0xffff&((a_pSrc1->bG - a_pSrc1->bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_pSrc1->bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_pSrc1->bB - a_pSrc1->bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_pSrc1->bR - a_pSrc1->bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_pSrc1->bR + a2*a_tSrc2.bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_pSrc1->bG + a2*a_tSrc2.bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_pSrc1->bB + a2*a_tSrc2.bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc1;
		}
		break;
	case EBEMultiplyInvAlpha:
		while (a_pDst < pEnd)
		{
			a_pDst->bR = a_pSrc1->bR;
			a_pDst->bG = a_pSrc1->bG;
			a_pDst->bB = a_pSrc1->bB;
			a_pDst->bA = ULONG(a_pSrc1->bA)*(255-a_tSrc2.bA)/255;
			++a_pDst;
			++a_pSrc1;
		}
		break;
	default:
		ATLASSERT(0);
	}
}


template<typename TPixel>
void MergeLayerLine(ELayerBlend a_eBlend, ULONG a_nSizeX, TPixel* a_pDst, TPixel const a_tSrc1, TPixel const* a_pSrc2, CGammaTables const& a_cGamma)// bool a_bGamma, WORD const* a_pGammaF, BYTE const* a_pGammaB)
{
	if (a_tSrc1.bA == 0 && a_eBlend != EBEMultiplyInvAlpha)
	{
		if (a_pDst != a_pSrc2)
			CopyMemory(a_pDst, a_pSrc2, sizeof(TPixel)*a_nSizeX);
		return;
	}

	TPixel* const pEnd = a_pDst+a_nSizeX;

	switch (a_eBlend)
	{
	case EBEAlphaBlend:
	case EBEGlass:
		if (true)//a_bGamma)
		{
			while (a_pDst < pEnd)
			{
				if (a_pSrc2->bA == 0)
				{
					*a_pDst = a_tSrc1;
				}
				else
				{
					if (a_pSrc2->bA == 255 || a_tSrc1.bA == 0)
					{
						*a_pDst = *a_pSrc2;
					}
					else
					{
						ULONG nNewA = a_pSrc2->bA*255 + (255-a_pSrc2->bA)*static_cast<ULONG>(a_tSrc1.bA);
						if (nNewA)
						{
							a_pDst->bR = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_tSrc1.bR])*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bR])*a_pSrc2->bA*255)/nNewA);
							a_pDst->bG = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_tSrc1.bG])*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bG])*a_pSrc2->bA*255)/nNewA);
							a_pDst->bB = a_cGamma.InvGamma((static_cast<ULONG>(a_cGamma.m_aGamma[a_tSrc1.bB])*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_cGamma.m_aGamma[a_pSrc2->bB])*a_pSrc2->bA*255)/nNewA);
						}
						else
						{
							a_pDst->bR = a_pDst->bG = a_pDst->bB = 0;
						}
						a_pDst->bA = nNewA/255;
					}
				}
				++a_pDst;
				++a_pSrc2;
			}
		}
		else
		{
			while (a_pDst < pEnd)
			{
				if (a_pSrc2->bA == 0)
				{
					*a_pDst = a_tSrc1;
				}
				else
				{
					if (a_pSrc2->bA == 255 || a_tSrc1.bA == 0)
					{
						*a_pDst = *a_pSrc2;
					}
					else
					{
						ULONG nNewA = a_pSrc2->bA*255 + (255-a_pSrc2->bA)*static_cast<ULONG>(a_tSrc1.bA);
						if (nNewA)
						{
							a_pDst->bR = (static_cast<ULONG>(a_tSrc1.bR)*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_pSrc2->bR)*a_pSrc2->bA*255)/nNewA;
							a_pDst->bG = (static_cast<ULONG>(a_tSrc1.bG)*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_pSrc2->bG)*a_pSrc2->bA*255)/nNewA;
							a_pDst->bB = (static_cast<ULONG>(a_tSrc1.bB)*(255-a_pSrc2->bA)*a_tSrc1.bA + static_cast<ULONG>(a_pSrc2->bB)*a_pSrc2->bA*255)/nNewA;
						}
						else
						{
							a_pDst->bR = a_pDst->bG = a_pDst->bB = 0;
						}
						a_pDst->bA = nNewA/255;
					}
				}
				++a_pDst;
				++a_pSrc2;
			}
		}
		break;
	case EBEModulate:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_tSrc1.bR) * a_pSrc2->bR + 0x80;
                ULONG const nR = ((t >> 8) + t) >> 8;
                t = ULONG(a_tSrc1.bG) * a_pSrc2->bG + 0x80;
                ULONG const nG = ((t >> 8) + t) >> 8;
                t = ULONG(a_tSrc1.bB) * a_pSrc2->bB + 0x80;
                ULONG const nB = ((t >> 8) + t) >> 8;

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEScreen:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = (255-a_tSrc1.bR) * (255-a_pSrc2->bR) + 0x80;
                ULONG const nR = 255-(((t >> 8) + t) >> 8);
                t = (255-a_tSrc1.bG) * (255-a_pSrc2->bG) + 0x80;
                ULONG const nG = 255-(((t >> 8) + t) >> 8);
                t = (255-a_tSrc1.bB) * (255-a_pSrc2->bB) + 0x80;
                ULONG const nB = 255-(((t >> 8) + t) >> 8);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEAdd:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG t = ULONG(a_tSrc1.bR) + a_pSrc2->bR;
                ULONG const nR = min(255, t);
                t = ULONG(a_tSrc1.bG) + a_pSrc2->bG;
                ULONG const nG = min(255, t);
                t = ULONG(a_tSrc1.bB) + a_pSrc2->bB;
                ULONG const nB = min(255, t);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBESubtract:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				LONG t = LONG(a_tSrc1.bR) - a_pSrc2->bR;
                ULONG const nR = max(0, t);
                t = LONG(a_tSrc1.bG) - a_pSrc2->bG;
                ULONG const nG = max(0, t);
                t = LONG(a_tSrc1.bB) - a_pSrc2->bB;
                ULONG const nB = max(0, t);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEAverage:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);
				a1 += a>>1;
				a2 += a>>1;

                a_pDst->bR = (a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEDifference:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG const nR = a_tSrc1.bR > a_pSrc2->bR ? (a_tSrc1.bR-a_pSrc2->bR) : (a_pSrc2->bR-a_tSrc1.bR);
                ULONG const nG = a_tSrc1.bG > a_pSrc2->bG ? (a_tSrc1.bG-a_pSrc2->bG) : (a_pSrc2->bG-a_tSrc1.bG);
                ULONG const nB = a_tSrc1.bB > a_pSrc2->bB ? (a_tSrc1.bB-a_pSrc2->bB) : (a_pSrc2->bB-a_tSrc1.bB);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEMinimum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = min(a_tSrc1.bR, a_pSrc2->bR);
                ULONG const nG = min(a_tSrc1.bG, a_pSrc2->bG);
                ULONG const nB = min(a_tSrc1.bB, a_pSrc2->bB);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEMaximum:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

                ULONG const nR = max(a_tSrc1.bR, a_pSrc2->bR);
                ULONG const nG = max(a_tSrc1.bG, a_pSrc2->bG);
                ULONG const nB = max(a_tSrc1.bB, a_pSrc2->bB);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEOverlay:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				ULONG nR;
				if (a_tSrc1.bR < 128)
				{
					ULONG const t = (ULONG(a_tSrc1.bR) * a_pSrc2->bR) << 1;
					nR = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_tSrc1.bR) * (255-a_pSrc2->bR) << 1;
					nR = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nR > 255) nR = 255;

				ULONG nG;
				if (a_tSrc1.bG < 128)
				{
					ULONG const t = (ULONG(a_tSrc1.bG) * a_pSrc2->bG) << 1;
					nG = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_tSrc1.bG) * (255-a_pSrc2->bG) << 1;
					nG = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nG > 255) nG = 255;

				ULONG nB;
				if (a_tSrc1.bB < 128)
				{
					ULONG const t = (ULONG(a_tSrc1.bB) * a_pSrc2->bB) << 1;
					nB = ((t >> 8) + t) >> 8;
				}
				else
				{
					ULONG const t = (255-a_tSrc1.bB) * (255-a_pSrc2->bB) << 1;
					nB = 255 - (((t >> 8) + t) >> 8);
				}
				//if (nB > 255) nB = 255;

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceHue:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_pSrc2->bR == rgbmax2)
						nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
					else if (a_pSrc2->bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_tSrc1.bR>a_tSrc1.bG ? (a_tSrc1.bR>a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG>a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				int const rgbmin1 = a_tSrc1.bR<a_tSrc1.bG ? (a_tSrc1.bR<a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG<a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_tSrc1.bR == rgbmax1)
				//		nH1 = 0xffff&((a_tSrc1.bG - a_tSrc1.bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_tSrc1.bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_tSrc1.bB - a_tSrc1.bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_tSrc1.bR - a_tSrc1.bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS1)/0xffff : nS1 - (nL1*nS1)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceSaturation:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_pSrc2->bR == rgbmax2)
				//		nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_pSrc2->bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_tSrc1.bR>a_tSrc1.bG ? (a_tSrc1.bR>a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG>a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				int const rgbmin1 = a_tSrc1.bR<a_tSrc1.bG ? (a_tSrc1.bR<a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG<a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_tSrc1.bR == rgbmax1)
						nH1 = 0xffff&((a_tSrc1.bG - a_tSrc1.bB)*0xffff / int(rgbdelta1*6));
					else if (a_tSrc1.bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_tSrc1.bB - a_tSrc1.bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_tSrc1.bR - a_tSrc1.bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceLuminance:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				//unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				//unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				//unsigned int nH2 = 0;
				//if (rgbdelta2)
				//	if (a_pSrc2->bR == rgbmax2)
				//		nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
				//	else if (a_pSrc2->bG == rgbmax2)
				//		nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
				//	else
				//		nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_tSrc1.bR>a_tSrc1.bG ? (a_tSrc1.bR>a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG>a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				int const rgbmin1 = a_tSrc1.bR<a_tSrc1.bG ? (a_tSrc1.bR<a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG<a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				unsigned int nH1 = 0;
				if (rgbdelta1)
					if (a_tSrc1.bR == rgbmax1)
						nH1 = 0xffff&((a_tSrc1.bG - a_tSrc1.bB)*0xffff / int(rgbdelta1*6));
					else if (a_tSrc1.bG == rgbmax1)
						nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_tSrc1.bB - a_tSrc1.bR)*0xffff) / int(rgbdelta1*6));
					else
						nH1 = 0xffff&((((rgbdelta1<<2) + a_tSrc1.bR - a_tSrc1.bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL2 + (nL2 <= 0x7fff ? (nL2*nS1)/0xffff : nS1 - (nL2*nS1)/0xffff);
				unsigned int m1 = (nL2<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH1+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH1);
				ULONG const nB = IntHLS(m1, m2, nH1-0x5555);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case ELBHLSReplaceColor:
		while (a_pDst < pEnd)
		{
			if (a_pSrc2->bA == 0)
			{
				*a_pDst = a_tSrc1;
			}
			else if (a_tSrc1.bA == 0)
			{
				*a_pDst = *a_pSrc2;
			}
			else
			{
				ULONG const nA12 = ULONG(a_tSrc1.bA)*ULONG(a_pSrc2->bA);
				ULONG const nA = ULONG(a_pSrc2->bA) + ULONG(a_tSrc1.bA) - (((nA12>>7)+nA12)>>8);
				ULONG const a1 = (255-a_pSrc2->bA)*a_tSrc1.bA;
				ULONG const a2 = (255-a_tSrc1.bA)*a_pSrc2->bA;
                ULONG const a = nA*255-(a1+a2);

				int const rgbmax2 = a_pSrc2->bR>a_pSrc2->bG ? (a_pSrc2->bR>a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG>a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				int const rgbmin2 = a_pSrc2->bR<a_pSrc2->bG ? (a_pSrc2->bR<a_pSrc2->bB ? a_pSrc2->bR : a_pSrc2->bB) : (a_pSrc2->bG<a_pSrc2->bB ? a_pSrc2->bG : a_pSrc2->bB);
				unsigned int const nL2 = (rgbmax2 + rgbmin2)<<7;
				unsigned int const rgbdelta2 = rgbmax2 - rgbmin2;
				unsigned int const nS2 = rgbdelta2 == 0 ? 0 : (nL2 <= 0x7fff ? (rgbdelta2*0xffff) / (rgbmax2 + rgbmin2) : (rgbdelta2*0xffff) / (510 - rgbmax2 - rgbmin2));

				unsigned int nH2 = 0;
				if (rgbdelta2)
					if (a_pSrc2->bR == rgbmax2)
						nH2 = 0xffff&((a_pSrc2->bG - a_pSrc2->bB)*0xffff / int(rgbdelta2*6));
					else if (a_pSrc2->bG == rgbmax2)
						nH2 = 0xffff&(((rgbdelta2 + rgbdelta2 + a_pSrc2->bB - a_pSrc2->bR)*0xffff) / int(rgbdelta2*6));
					else
						nH2 = 0xffff&((((rgbdelta2<<2) + a_pSrc2->bR - a_pSrc2->bG)*0xffff) / int(rgbdelta2*6));

				int const rgbmax1 = a_tSrc1.bR>a_tSrc1.bG ? (a_tSrc1.bR>a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG>a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				int const rgbmin1 = a_tSrc1.bR<a_tSrc1.bG ? (a_tSrc1.bR<a_tSrc1.bB ? a_tSrc1.bR : a_tSrc1.bB) : (a_tSrc1.bG<a_tSrc1.bB ? a_tSrc1.bG : a_tSrc1.bB);
				unsigned int const nL1 = (rgbmax1 + rgbmin1)<<7;
				//unsigned int const rgbdelta1 = rgbmax1 - rgbmin1;
				//unsigned int const nS1 = rgbdelta1 == 0 ? 0 : (nL1 <= 0x7fff ? (rgbdelta1*0xffff) / (rgbmax1 + rgbmin1) : (rgbdelta1*0xffff) / (510 - rgbmax1 - rgbmin1));

				//unsigned int nH1 = 0;
				//if (rgbdelta1)
				//	if (a_tSrc1.bR == rgbmax1)
				//		nH1 = 0xffff&((a_tSrc1.bG - a_tSrc1.bB)*0xffff / int(rgbdelta1*6));
				//	else if (a_tSrc1.bG == rgbmax1)
				//		nH1 = 0xffff&(((rgbdelta1 + rgbdelta1 + a_tSrc1.bB - a_tSrc1.bR)*0xffff) / int(rgbdelta1*6));
				//	else
				//		nH1 = 0xffff&((((rgbdelta1<<2) + a_tSrc1.bR - a_tSrc1.bG)*0xffff) / int(rgbdelta1*6));

				// back to rgb
				unsigned int m2 = nL1 + (nL1 <= 0x7fff ? (nL1*nS2)/0xffff : nS2 - (nL1*nS2)/0xffff);
				unsigned int m1 = (nL1<<1) - m2;
				ULONG const nR = IntHLS(m1, m2, nH2+0x5555);
				ULONG const nG = IntHLS(m1, m2, nH2);
				ULONG const nB = IntHLS(m1, m2, nH2-0x5555);

                a_pDst->bR = (a*nR + a1*a_tSrc1.bR + a2*a_pSrc2->bR)/(nA*255);
                a_pDst->bG = (a*nG + a1*a_tSrc1.bG + a2*a_pSrc2->bG)/(nA*255);
                a_pDst->bB = (a*nB + a1*a_tSrc1.bB + a2*a_pSrc2->bB)/(nA*255);
                a_pDst->bA = nA;
			}
			++a_pDst;
			++a_pSrc2;
		}
		break;
	case EBEMultiplyInvAlpha:
		while (a_pDst < pEnd)
		{
			a_pDst->bR = a_tSrc1.bR;
			a_pDst->bG = a_tSrc1.bG;
			a_pDst->bB = a_tSrc1.bB;
			a_pDst->bA = ULONG(a_tSrc1.bA)*(255-a_pSrc2->bA)/255;
			++a_pDst;
			++a_pSrc2;
		}
		break;
	default:
		ATLASSERT(0);
	}
}


