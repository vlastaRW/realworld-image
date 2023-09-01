// MenuCommandsWindowRendering.h : Declaration of the CMenuCommandsWindowRendering

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>
#include <GammaCorrection.h>


// CMenuCommandsWindowRendering

class ATL_NO_VTABLE CMenuCommandsWindowRendering :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsWindowRendering, &CLSID_MenuCommandsWindowRendering>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsWindowRendering()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsWindowRendering)

BEGIN_CATEGORY_MAP(CMenuCommandsWindowRendering)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsWindowRendering)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID, UINT t_uIDIcon, EImageQuality t_eMode>
	class ATL_NO_VTABLE CMode :
		public CDocumentMenuCommandImpl<CMode<t_uIDName, t_uIDDesc, t_pIconID, t_uIDIcon, t_eMode>, t_uIDName, t_uIDDesc, t_pIconID, t_uIDIcon>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			EImageQuality eQuality;
			return pRIG && S_OK == pRIG->GetImageQuality(&eQuality) ? ((eQuality&EIQMethodMask) == t_eMode ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IEnumUnknownsInit> p;
				RWCoCreateInstance(p, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
				CComPtr<IRasterEditView> pRIG;
				p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
				EImageQuality eQuality;
				return pRIG && SUCCEEDED(pRIG->GetImageQuality(&eQuality)) ? pRIG->SetImageQuality(static_cast<EImageQuality>((eQuality&~EIQMethodMask)|t_eMode)) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};

	template<wchar_t const* t_pszName, wchar_t const* t_pszDesc, ULONG t_eStyle>
	class ATL_NO_VTABLE CStyle :
		public CDocumentMenuCommandMLImpl<CStyle<t_pszName, t_pszDesc, t_eStyle>, t_pszName, t_pszDesc, NULL, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(IconID)(GUID* a_pIconID)
		{
			if (a_pIconID == NULL)
				return E_POINTER;
			try
			{
				// {DF5FA80D-03C0-40CD-BC55-ED1352577EDE}
				static GUID const tID = {0xdf5fa80d, 0x3c0, 0x40cd, {0xbc, 0x55, 0xed, 0x13, 0x52, 0x57, 0x7e, 0xde}};
				*a_pIconID = tID;
				a_pIconID->Data1 ^= t_eStyle;
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
				static IRPolyPoint const main[] = { {0, 0}, {0, 256}, {256, 256}, {256, 0} };
				IRFill mainFillMat(t_eStyle == 0 ? 0xff000000|GetSysColor(COLOR_3DLIGHT) : (t_eStyle == 3 ? 0xff000000|CGammaTables::BlendSRGB(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_WINDOWTEXT), 0.9f) : 0xffffffff));
				IROutlinedFill mainMat(&mainFillMat, pSI->GetMaterial(ESMContrast));
				CIconRendererReceiver cRenderer(a_nSize);

				cRenderer(&canvas, itemsof(main), main, &mainMat);

				if (t_eStyle != 1)
				{
					static IRPolyPoint const box1[] = { {8, 8}, {8, 128}, {128, 128}, {128, 8} };
					static IRPolyPoint const box2[] = { {128, 128}, {128, 248}, {248, 248}, {248, 128} };
					IRFill boxMat(t_eStyle == 3 ? 0x1f000000|GetSysColor(COLOR_WINDOWTEXT) : 0xff000000|GetSysColor(COLOR_3DSHADOW));
					cRenderer(&canvas, itemsof(box1), box1, &boxMat);
					cRenderer(&canvas, itemsof(box2), box2, &boxMat);
				}

				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			ULONG eStyle;
			return pRIG && S_OK == pRIG->GetStyle(&eStyle) ? (eStyle == t_eStyle ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IEnumUnknownsInit> p;
				RWCoCreateInstance(p, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
				CComPtr<IRasterEditView> pRIG;
				p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
				return pRIG ? pRIG->SetStyle(t_eStyle) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};

	class ATL_NO_VTABLE CInvertedPixels :
		public CDocumentMenuCommandImpl<CInvertedPixels, IDS_MN_RENDERINVERTED, IDS_MD_RENDERINVERTED, NULL, 0>
	{
	public:
		void Init(IDesignerView* a_pView)
		{
			m_pView = a_pView;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			BYTE bInvert;
			return pRIG ? (S_OK == pRIG->GetInvertedPixels(&bInvert) && bInvert ? EMCSChecked : EMCSNormal) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<IEnumUnknownsInit> p;
				RWCoCreateInstance(p, __uuidof(EnumUnknowns));
				m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
				CComPtr<IRasterEditView> pRIG;
				p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
				BYTE bInvert;
				return pRIG && SUCCEEDED(pRIG->GetInvertedPixels(&bInvert)) ? pRIG->SetInvertedPixels(bInvert ? 0 : 1) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
	};
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsWindowRendering), CMenuCommandsWindowRendering)
