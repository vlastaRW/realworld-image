
#pragma once

#include <DocumentMenuCommandImpl.h>
#include <math.h>
#include <WTL_ColorPicker.h>


class ATL_NO_VTABLE CMenuCommandDeleteAllColors :
	public CDocumentMenuCommandImpl<CMenuCommandDeleteAllColors, IDS_MENU_DELETECOLORS_NAME, IDS_MENU_DELETECOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			ULONG n = 0;
			m_pView->Count(&n);
			m_pView->Delete(0, n);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};

class ATL_NO_VTABLE CMenuCommandInsertColors :
	public CDocumentMenuCommandImpl<CMenuCommandInsertColors, IDS_MENU_INSERTCOLORS_NAME, IDS_MENU_INSERTCOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			TSwatchColor tColor1;
			TSwatchColor tColor2;
			m_pView->Defaults(&tColor1, &tColor2);

			ULONG nSteps = 16;
			CDlg cDlg(a_tLocaleID, &tColor1, &tColor2, &nSteps);
			if (cDlg.DoModal(a_hParent) == IDOK)
			{
				m_pView->Insert(0xffffffff, &tColor1);
				for (ULONG i = 1; i < nSteps && nSteps <= 256; ++i)
				{
					float const f2 = i/float(nSteps-1);
					float const f1 = 1.0-f2;
					TSwatchColor tColor = tColor1;
					tColor.fA = tColor1.fA*f1 + tColor2.fA*f2;
					if (tColor.fA != 0.0f)
					{
						tColor.f1 = (tColor1.f1*tColor1.fA*f1 + tColor2.f1*tColor2.fA*f2)/tColor.fA;
						tColor.f2 = (tColor1.f2*tColor1.fA*f1 + tColor2.f2*tColor2.fA*f2)/tColor.fA;
						tColor.f3 = (tColor1.f3*tColor1.fA*f1 + tColor2.f3*tColor2.fA*f2)/tColor.fA;
					}
					else
					{
						tColor.f1 = tColor.f2 = tColor.f3 = 0.0f;
					}
					m_pView->Insert(0xffffffff, &tColor);
				}
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	class CDlg :
		public Win32LangEx::CLangDialogImpl<CDlg>//,
		//public CDialogResize<CDlg>,
		//public CContextHelpDlg<CDlg>
	{
	public:
		enum { IDD = IDD_ADDSWATCHES };

		CDlg(LCID a_tLocaleID, TSwatchColor* a_pColor1, TSwatchColor* a_pColor2, ULONG* a_pSteps) :
			Win32LangEx::CLangDialogImpl<CDlg>(a_tLocaleID),
			m_pColor1(a_pColor1), m_pColor2(a_pColor2), m_pSteps(a_pSteps)
			//CContextHelpDlg<CDlg>(_T("http://wiki.rw-designer.com/Configure_Tool_Presets"))
		{
		}

	private:
		BEGIN_MSG_MAP(CDlg)
			//CHAIN_MSG_MAP(CContextHelpDlg<CDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnOKOrCancel)
			COMMAND_ID_HANDLER(IDCANCEL, OnOKOrCancel)
			//CHAIN_MSG_MAP(CDialogResize<CDlg>)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		//BEGIN_DLGRESIZE_MAP(CDlg)
		//	DLGRESIZE_CONTROL(IDHELP, DLSZ_MOVE_Y)
		//	DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		//	DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		//	DLGRESIZE_CONTROL(IDC_RT_CONFIG, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		//END_DLGRESIZE_MAP()

		//BEGIN_CTXHELP_MAP(CDlg)
		//	CTXHELP_CONTROL_RESOURCE(IDOK, IDS_HELP_IDOK)
		//	CTXHELP_CONTROL_RESOURCE(IDCANCEL, IDS_HELP_IDCANCEL)
		//	CTXHELP_CONTROL_RESOURCE(IDC_SHLASSOC_LIST, IDS_HELP_SHLASSOC_LIST)
		//END_CTXHELP_MAP()

		LRESULT OnInitDialog(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam, BOOL& a_bHandled)
		{
			m_wndColor1.SubclassWindow(GetDlgItem(IDC_ASG_COLOR1));
			m_wndColor1.SetDefaultColor(CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 1.0f));
			m_wndColor1.SetColor(CButtonColorPicker::SColor(m_pColor1->f1, m_pColor1->f2, m_pColor1->f3, m_pColor1->fA));
			m_wndColor2.SubclassWindow(GetDlgItem(IDC_ASG_COLOR2));
			m_wndColor2.SetDefaultColor(CButtonColorPicker::SColor(0.0f, 0.0f, 0.0f, 0.0f));
			m_wndColor2.SetColor(CButtonColorPicker::SColor(m_pColor2->f1, m_pColor2->f2, m_pColor2->f3, m_pColor2->fA));
			SetDlgItemInt(IDC_ASG_STEP_EDIT, *m_pSteps, FALSE);
			CUpDownCtrl wndUD(GetDlgItem(IDC_ASG_STEP_SPIN));
			wndUD.SetRange(1, 256);

			CenterWindow();

			//DlgResize_Init();

			return TRUE;
		}
		LRESULT OnOKOrCancel(WORD a_wNotifyCode, WORD wID, HWND a_hWndCtl, BOOL& a_bHandled)
		{
			if (wID == IDOK)
			{
				CButtonColorPicker::SColor sColor = m_wndColor1.GetColor();
				m_pColor1->eSpace = ESCSRGB;
				m_pColor1->f1 = sColor.fR;
				m_pColor1->f2 = sColor.fG;
				m_pColor1->f3 = sColor.fB;
				m_pColor1->fA = sColor.fA;
				sColor = m_wndColor2.GetColor();
				m_pColor2->eSpace = ESCSRGB;
				m_pColor2->f1 = sColor.fR;
				m_pColor2->f2 = sColor.fG;
				m_pColor2->f3 = sColor.fB;
				m_pColor2->fA = sColor.fA;
				*m_pSteps = GetDlgItemInt(IDC_ASG_STEP_EDIT);
			}
			EndDialog(wID);
			return 0;
		}


	private:
		TSwatchColor* m_pColor1;
		TSwatchColor* m_pColor2;
		ULONG* m_pSteps;
		CButtonColorPicker m_wndColor1;
		CButtonColorPicker m_wndColor2;
	};

private:
	CComPtr<IColorSwatchView> m_pView;
};

CComPtr<IEnumUnknowns> GetColorFormatFilter(bool a_bPhotoshop)
{
	CComPtr<IEnumUnknownsInit> pTmp;
	RWCoCreateInstance(pTmp, __uuidof(EnumUnknowns));
	CComPtr<IDocumentTypeWildcards> pFF;
	RWCoCreateInstance(pFF, __uuidof(DocumentTypeWildcards));
	CComBSTR bstrRWCS(L"rwcs");
	pFF->InitEx(_SharedStringTable.GetStringAuto(IDS_FORMATFILTER_RWCS), NULL, 1, &(bstrRWCS.m_str), NULL, NULL, 0, CComBSTR(L"*.rwcs"));
	pTmp->Insert(pFF);
	if (a_bPhotoshop)
	{
		pFF = NULL;
		RWCoCreateInstance(pFF, __uuidof(DocumentTypeWildcards));
		pFF->Init(_SharedStringTable.GetStringAuto(IDS_FORMATFILTER_ACO), CComBSTR(L"*.aco"));
		pTmp->Insert(pFF);
	}
	return pTmp.p;
}

class ATL_NO_VTABLE CMenuCommandLoadColors :
	public CDocumentMenuCommandImpl<CMenuCommandLoadColors, IDS_MENU_LOADCOLORS_NAME, IDS_MENU_LOADCOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IStorageManager> pSM;
			RWCoCreateInstance(pSM, __uuidof(StorageManager));
			CComPtr<IStorageFilter> pFlt;
			static const GUID tCtx = {0xbab1bdf0, 0xa751, 0x4437, {0x84, 0x1d, 0x8e, 0xef, 0x70, 0xf2, 0x1c, 0x32}};
			pSM->FilterCreateInteractivelyUID(NULL, EFTAccessRead|EFTOpenExisting|EFTShareRead, a_hParent, GetColorFormatFilter(true), NULL, tCtx, NULL, NULL, a_tLocaleID, &pFlt);
			if (pFlt == NULL)
				return S_FALSE;
			CComPtr<IDataSrcDirect> pSrc;
			pFlt->SrcOpen(&pSrc);
			ULONG nSize = 0;
			pSrc->SizeGet(&nSize);
			if (nSize == 0)
				return E_FAIL;
			CDirectInputLock cData(pSrc, nSize);
			BYTE const* p = cData;
			// check if it is .rwcs
			if (nSize >= 19 && strncmp(reinterpret_cast<char const*>(p), "RealWorldColors1:\r\n", 19) == 0)
			{
				char const* psz = reinterpret_cast<char const*>(p+19);
				char const* pszEnd = psz+nSize-19;
				TSwatchColor tColor;
				tColor.eSpace = ESCSRGB;
				tColor.bstrName = NULL;
				tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
				tColor.fA = 1.0f;
				float* aDsts[10];
				int nDsts = 0;
				while (psz < pszEnd)
				{
					switch (*psz)
					{
					case 'R':
						aDsts[nDsts++] = &tColor.f1;
						break;
					case 'G':
						aDsts[nDsts++] = &tColor.f2;
						break;
					case 'B':
						aDsts[nDsts++] = &tColor.f3;
						break;
					case 'A':
						aDsts[nDsts++] = &tColor.fA;
						break;
					case 'H':
						tColor.eSpace = ESCSHLS;
						aDsts[nDsts++] = &tColor.f1;
						break;
					case 'L':
						tColor.eSpace = ESCSHLS;
						aDsts[nDsts++] = &tColor.f2;
						break;
					case 'S':
						tColor.eSpace = ESCSHLS;
						aDsts[nDsts++] = &tColor.f3;
						break;
					case ',':
						break;
					case '|':
						{
							char const* psz2 = psz+1;
							while (psz2 < pszEnd && *psz != '\r' && *psz != '\n')
								++psz2;
							if (psz2 > psz)
							{
								int n = MultiByteToWideChar(CP_UTF8, 0, psz, psz2-psz, NULL, 0);
								tColor.bstrName = SysAllocStringLen(NULL, n);
								MultiByteToWideChar(CP_UTF8, 0, psz, psz2-psz, tColor.bstrName, n);
								psz = psz2;
							}
							else
							{
								break;
							}
						}
					case '\r':
					case '\n':
						if (psz+1 < pszEnd && psz[1] == *psz^('\r'^'\n'))
							++psz;
						m_pView->Insert(-1, &tColor);
						nDsts = 0;
						tColor.eSpace = ESCSRGB;
						tColor.f1 = tColor.f2 = tColor.f3 = tColor.f4 = 0.0f;
						tColor.fA = 1.0f;
						if (tColor.bstrName)
						{
							SysFreeString(tColor.bstrName);
							tColor.bstrName = NULL;
						}
						break;
					case ':':
						if (psz+1 < pszEnd)
							++psz;
					default:
						{
							char* pszNext = const_cast<char*>(psz);
							float f = strtod(psz, &pszNext);
							if (pszNext > psz)
							{
								for (int i = 0; i < nDsts; ++i)
									*(aDsts[i]) = f;
								psz = pszNext-1;
							}
							nDsts = 0;
						}
						break;
					}
					++psz;
				}
				m_pView->Insert(-1, &tColor);
				if (tColor.bstrName)
				{
					SysFreeString(tColor.bstrName);
					tColor.bstrName = NULL;
				}
				return S_OK;
			}

			// maybe it is Photoshop .aco
			if (nSize&1)
				return E_FAIL;
			ULONG nVer = (ULONG(p[0])<<8)|p[1];
			if (nVer != 1)
				return E_FAIL; // bad file format
			ULONG const nColors = (ULONG(p[2])<<8)|p[3];
			p += 4;
			if (nSize < 4+nColors*10)
				return E_FAIL; // file incomplete
			if (nSize >= 8+nColors*26 && ((ULONG(p[0+nColors*10])<<8)|p[1+nColors*10]) == 2 && ((ULONG(p[2+nColors*10])<<8)|p[3+nColors*10]) == nColors)
			{
				nVer = 2;
				p += 4+nColors*10;
			}
			// TODO: check file length (consistency)
			for (ULONG i = 0; i < nColors; ++i)
			{
				BSTR bstr = NULL;
				ULONG nLen = 0;
				if (nVer == 2)
				{
					nLen = (ULONG(p[12])<<8)|p[13];
					bstr = SysAllocStringLen(NULL, nLen-1);
					for (ULONG j = 0; j < nLen; ++j)
						bstr[j] = (ULONG(p[14+j+j])<<8)|p[15+j+j];
				}
				if (p[0] == 0 && p[1] == 0)
				{
					TSwatchColor tClr =
					{
						bstr, ESCSRGB,
						((int(p[2])<<8)|p[3])/65535.0f,
						((int(p[4])<<8)|p[5])/65535.0f,
						((int(p[6])<<8)|p[7])/65535.0f,
						0.0f,
						1.0f
					};
					m_pView->Insert(0xffffffff, &tClr);
				}
				else if (p[0] == 0 && p[1] == 1)
				{
					TSwatchColor tClr =
					{
						bstr, ESCSHLS,
						((int(p[2])<<8)|p[3])/65535.0f*360.0f,
						((int(p[6])<<8)|p[7])/65535.0f,
						((int(p[4])<<8)|p[5])/65535.0f,
						0.0f,
						1.0f
					};
					//HLS2RGB(tClr.f1, tClr.f3, tClr.f2, tClr.f1, tClr.f2, tClr.f3);
					m_pView->Insert(0xffffffff, &tClr);
				}
				else if (p[0] == 0 && p[1] == 7)
				{
					float a = short((short(p[4])<<8)|short(p[5]))/12750.0f;
					float b = short((short(p[6])<<8)|short(p[7]))/12750.0f;
					TSwatchColor tClr =
					{
						bstr, ESCSHLS,
						atan2f(b, a)*(180.0f/3.14159265f),
						((int(p[2])<<8)|p[3])/10000.0f,
						sqrtf(a*a + b*b),
						0.0f,
						1.0f
					};
					if (tClr.f1 < 0.0f) tClr.f1 += 360.0f;
					else if (tClr.f1 >= 360.0f) tClr.f1 -= 360.0f;
					//HLS2RGB(tClr.f1, tClr.f3, tClr.f2, tClr.f1, tClr.f2, tClr.f3);
					m_pView->Insert(0xffffffff, &tClr);
				}
				// TODO: implement other color spaces
				p += nVer == 2 ? (14 + nLen*2) : 10;
			}
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};

class ATL_NO_VTABLE CMenuCommandSaveColors :
	public CDocumentMenuCommandImpl<CMenuCommandSaveColors, IDS_MENU_SAVECOLORS_NAME, IDS_MENU_SAVECOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			CComPtr<IStorageManager> pSM;
			RWCoCreateInstance(pSM, __uuidof(StorageManager));
			CComPtr<IStorageFilter> pFlt;
			static const GUID tCtx = {0xbab1bdf0, 0xa751, 0x4437, {0x84, 0x1d, 0x8e, 0xef, 0x70, 0xf2, 0x1c, 0x32}};
			pSM->FilterCreateInteractivelyUID(NULL, EFTAccessWrite|EFTCreateNew, a_hParent, GetColorFormatFilter(false), NULL, tCtx, NULL, NULL, a_tLocaleID, &pFlt);
			if (pFlt == NULL)
				return S_FALSE;
			CComPtr<IDataDstStream> pDst;
			pFlt->DstOpen(&pDst);
			pDst->Write(19, reinterpret_cast<BYTE const*>("RealWorldColors1:\r\n"));
			ULONG nCount = 0;
			m_pView->Count(&nCount);
			for (ULONG i = 0; i < nCount; ++i)
			{
				TSwatchColor tColor;
				ZeroMemory(&tColor, sizeof tColor);
				m_pView->Value(i, &tColor);
				char const* pChnls = tColor.eSpace == ESCSRGB ? "RGBA" : "HLSA";
				float pVals[4] = {tColor.f1, tColor.f2, tColor.f3, tColor.fA};
				static float const pDefs[4] = {0.0f, 0.0f, 0.0f, 1.0f};
				char szTmp[128] = ""; // RG:0.5,B:1    G:1|Green
				char* psz = szTmp;
				for (int j = 0; j < 4; ++j)
				{
					if (pVals[j] != pDefs[j])
					{
						if (psz != szTmp)
							*(psz++) = ',';
						*(psz++) = pChnls[j];
						for (int k = j+1; k < 4; ++k)
						{
							if (pVals[k] != pDefs[k] && pVals[k] == pVals[j])
							{
								*(psz++) = pChnls[k];
								pVals[k] = pDefs[k];
							}
						}
						*(psz++) = ':';
						sprintf(psz, "%g", pVals[j]);
						psz += strlen(psz);
					}
				}
				if (tColor.bstrName && tColor.bstrName[0])
				{
					*(psz++) = '|';
					pDst->Write(psz-szTmp, reinterpret_cast<BYTE const*>(szTmp));
					int n = WideCharToMultiByte(CP_UTF8, 0, tColor.bstrName, SysStringLen(tColor.bstrName), NULL, 0, NULL, NULL);
					CAutoVectorPtr<char> aUTF8(new char[n+3]);
					WideCharToMultiByte(CP_UTF8, 0, tColor.bstrName, SysStringLen(tColor.bstrName), aUTF8, n+1, NULL, NULL);
					aUTF8[n] = '\r';
					aUTF8[n+1] = '\n';
					SysFreeString(tColor.bstrName);
					pDst->Write(i != nCount-1 ? n+2 : n, reinterpret_cast<BYTE const*>(aUTF8.m_p));
				}
				else
				{
					if (i != nCount-1)
					{
						*(psz++) = '\r';
						*(psz++) = '\n';
					}
					*psz = '\0';
					pDst->Write(psz-szTmp, reinterpret_cast<BYTE const*>(szTmp));
				}
			}
			pDst->Close();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};

enum EDefaultColorSwatches
{
	EDCSDefault = 0,
	EDCSWindows = 1,
	EDCSWeb = 2,
};

template<UINT t_nIDName, ESwatchType t_eSwatch>
class ATL_NO_VTABLE CMenuCommandPresetColors :
	public CDocumentMenuCommandImpl<CMenuCommandPresetColors<t_nIDName, t_eSwatch>, t_nIDName, IDS_MENU_PRESETCOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	EMenuCommandState IntState()
	{
		ULONG n = 0;
		m_pView->SwatchesGet(&n);
		return n&t_eSwatch ? EMCSChecked : EMCSNormal;
	}
	STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
	{
		try
		{
			ULONG n = ESTCustom;
			m_pView->SwatchesGet(&n);
			m_pView->SwatchesSet(n^t_eSwatch);
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};


class ATL_NO_VTABLE CMenuCommandStockColors :
	public CDocumentMenuCommandImpl<CMenuCommandStockColors, IDS_MENU_STOCKCOLORS_NAME, IDS_MENU_STOCKCOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	EMenuCommandState IntState() { return EMCSSubMenu; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_DEFAULT, ESTHues> >* p = NULL;
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_DEFAULT, ESTHues> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_WINDOWS, ESTWindows> >* p = NULL;
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_WINDOWS, ESTWindows> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_WEB, ESTWebSafe> >* p = NULL;
				CComObject<CMenuCommandPresetColors<IDS_MENU_PRESETCOLORS_WEB, ESTWebSafe> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};

class ATL_NO_VTABLE CMenuCommandCustomColors :
	public CDocumentMenuCommandImpl<CMenuCommandCustomColors, IDS_MENU_CUSTOMCOLORS_NAME, IDS_MENU_CUSTOMCOLORS_DESC, NULL, 0>
{
public:
	void Init(IColorSwatchView* a_pView)
	{
		m_pView = a_pView;
	}

	// IDocumentMenuCommand
public:
	EMenuCommandState IntState() { return EMCSSubMenu; }
	STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CMenuCommandInsertColors>* p = NULL;
				CComObject<CMenuCommandInsertColors>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMenuCommandDeleteAllColors>* p = NULL;
				CComObject<CMenuCommandDeleteAllColors>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}
			{
				CComPtr<IDocumentMenuCommand> pTmp;
				RWCoCreateInstance(pTmp, __uuidof(MenuCommandsSeparator));
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMenuCommandLoadColors>* p = NULL;
				CComObject<CMenuCommandLoadColors>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CMenuCommandSaveColors>* p = NULL;
				CComObject<CMenuCommandSaveColors>::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(m_pView);
				pItems->Insert(pTmp);
			}

			*a_ppSubCommands = pItems.Detach();
			return S_OK;
		}
		catch (...)
		{
			return a_ppSubCommands ? E_UNEXPECTED : E_POINTER;
		}
	}

private:
	CComPtr<IColorSwatchView> m_pView;
};

