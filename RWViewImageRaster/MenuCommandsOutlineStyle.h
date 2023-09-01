// MenuCommandsOutlineStyle.h : Declaration of the CMenuCommandsOutlineStyle

#pragma once
#include "resource.h"       // main symbols

#include "RWViewImageRaster.h"
#include <WeakSingleton.h>
#include <MultiLanguageString.h>
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

extern __declspec(selectany) wchar_t const NAME_NOOUTLINE[] = L"[0409]No outline[0405]Bez obrysu";
extern __declspec(selectany) wchar_t const DESC_NOOUTLINE[] = L"[0409]Disable the outline on the drawn shape.[0405]Nekreslit obrys vytvořeného tvaru.";
extern __declspec(selectany) wchar_t const NAME_SOLIDOUTLINE[] = L"[0409]Solid outline[0405]Jednobarevný obrys";
extern __declspec(selectany) wchar_t const DESC_SOLIDOUTLINE[] = L"[0409]Solid outline[0405]Jednobarevný obrys";
extern __declspec(selectany) GUID const ICONID_NOPAINT = {0x3794256a, 0xc63, 0x4c6a, {0x9a, 0x88, 0xa1, 0xc5, 0x5b, 0xc, 0x27, 0xa3}};


// CMenuCommandsOutlineStyle

class ATL_NO_VTABLE CMenuCommandsOutlineStyle :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsOutlineStyle, &CLSID_MenuCommandsOutlineStyle>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsOutlineStyle()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsOutlineStyle)

BEGIN_CATEGORY_MAP(CMenuCommandsOutlineStyle)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsOutlineStyle)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	class ATL_NO_VTABLE CDocumentMenuNoOutlineStyle :
		public CDocumentMenuCommandMLImpl<CDocumentMenuNoOutlineStyle, NAME_NOOUTLINE, DESC_NOOUTLINE, &ICONID_NOPAINT, IDI_NO_PAINT>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, bool a_bActive)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bActive = a_bActive;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState() { return m_bActive ? EMCSRadioChecked : EMCSRadio; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		bool m_bActive;
	};
	class ATL_NO_VTABLE CDocumentMenuSolidOutlineStyle :
		public CDocumentMenuCommandMLImpl<CDocumentMenuSolidOutlineStyle, NAME_SOLIDOUTLINE, DESC_SOLIDOUTLINE, NULL, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, bool a_bActive, IRasterImageFillStyleManager* a_pManager)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bActive = a_bActive;
			m_pManager = a_pManager;
			m_bstrStyle = L"SOLID";
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState() { return m_bActive ? EMCSRadioChecked : EMCSRadio; }
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		bool m_bActive;
		CComPtr<IRasterImageFillStyleManager> m_pManager;
		CComBSTR m_bstrStyle;
	};

	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuOutlineWidth :
		public CDocumentMenuCommandImpl<CDocumentMenuOutlineWidth<t_uIDName, t_uIDDesc, t_pIconID>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, float a_fDelta, BOOL a_bEnabled)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_fDelta = a_fDelta;
			m_bEnabled = a_bEnabled;
		}

		// IDocumentMenuCommand
	public:
		EMenuCommandState IntState()
		{
			if (!m_bEnabled)
				return EMCSDisabled;
			CComPtr<ISharedStateToolMode> pPrev;
			m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
			if (pPrev == NULL)
				return EMCSDisabled;
			float fWidth = 1.0f;
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &fWidth, NULL, NULL);
			return fWidth+m_fDelta >= 0.0f ? EMCSNormal : EMCSDisabled;
		}
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				static IRPathPoint const shape[] =
				{
					{255, 196, 0, 0, -34.5234, -28.8196},
					{225, 233, -26.1961, -21.8679, 0, 0},
					{128, 198, -36.7922, 0, 36.7922, 0},
					{31, 233, 0, 0, 26.1961, -21.8679},
					{1, 196, 34.5233, -28.8194, 0, 0},
					{128, 150, 48.4882, 0, -48.4879, 0},
				};
				static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
				cRenderer(&canvas, itemsof(shape), shape, pSI->GetMaterial(ESMInterior));
				pSI->GetLayers(m_fDelta > 0.0f ? ESIPlus : ESIMinus, cRenderer, IRTarget(0.65f, 1, -1));
				*a_phIcon = cRenderer.get();
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<ISharedStateToolMode> pPrev;
				m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
				CComObject<CSharedStateToolMode>* pNew = NULL;
				CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
				// enable outline if it was not enabled prior to selection of outline width
				BOOL bOutline = 1;
				BOOL* pOutline = NULL;
				CComPtr<ISharedState> pTmp = pNew;
				float fWidth = 1.0f;
				pPrev->Get(NULL, NULL, NULL, NULL, NULL, NULL, &bOutline, NULL, &fWidth, NULL, NULL);
				if (bOutline == 0)
				{
					bOutline = 1;
					pOutline = &bOutline;
				}
				fWidth += GetAsyncKeyState(VK_CONTROL)&0x8000 ? 5.0f*m_fDelta : (GetAsyncKeyState(VK_SHIFT)&0x8000 ? 0.2f*m_fDelta : m_fDelta);
				if (fWidth < 0.0f) fWidth = 0.0f;
				if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, NULL, pOutline, NULL, &fWidth, NULL, NULL, pPrev))
					m_pStates->StateSet(m_bstrSyncID, pTmp);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		float m_fDelta;
		BOOL m_bEnabled;
	};

	class ATL_NO_VTABLE CDocumentMenuOutlineWidthPopup :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, BOOL a_bEnabled)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bEnabled = a_bEnabled;
		}

	BEGIN_COM_MAP(CDocumentMenuOutlineWidthPopup)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		STDMETHOD(Description)(ILocalizedString** a_ppText);
		STDMETHOD(IconID)(GUID* UNREF(a_pIconID))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Icon)(ULONG UNREF(a_nSize), HICON* UNREF(a_phIcon))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(Accelerator)(TCmdAccel* UNREF(a_pAccel), TCmdAccel* UNREF(a_pAuxAccel))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands);
		STDMETHOD(State)(EMenuCommandState* a_peState)
		{
			try
			{
				*a_peState = static_cast<EMenuCommandState>((3<<24)|EMCSShowButtonText|EMCSSubMenu/*|(m_bEnabled ? 0 : EMCSDisabled)*/);
				return S_OK;
			}
			catch (...)
			{
				return a_peState ? E_UNEXPECTED : E_POINTER;
			}
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			return E_NOTIMPL;
		}

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		BOOL m_bEnabled;
	};

	class ATL_NO_VTABLE CDocumentMenuConstantOutlineWidth :
		public CDocumentMenuCommandImpl<CDocumentMenuConstantOutlineWidth, 0/*IDS_MENU_OWCONST_NAME*/, IDS_MENU_OWCONST_DESC, NULL, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, BOOL a_bEnabled, float a_fWidth)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bEnabled = a_bEnabled;
			m_fWidth = a_fWidth;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText);
		EMenuCommandState IntState();
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		BOOL m_bEnabled;
		float m_fWidth;
	};

	class ATL_NO_VTABLE CDocumentMenuConstantOutlinePosition :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, BOOL a_bEnabled, float a_fPos)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bEnabled = a_bEnabled;
			m_fPos = a_fPos;
		}

	BEGIN_COM_MAP(CDocumentMenuConstantOutlinePosition)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				wchar_t const* const psz = m_fPos < 0.0f ? L"[0409]Inside[0405]Uvnitř" : (m_fPos > 0.0f ? L"[0409]Outside[0405]Venku" : L"[0409]Centered[0405]Na středu");
				*a_ppText = new CMultiLanguageString(psz);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* UNREF(a_pAccel), TCmdAccel* UNREF(a_pAuxAccel))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		BOOL m_bEnabled;
		float m_fPos;
	};

	class ATL_NO_VTABLE CDocumentMenuOutlineJoins :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IDocumentMenuCommand
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, BOOL a_bEnabled, EOutlineJoinType a_eJoins)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_bEnabled = a_bEnabled;
			m_eJoins = a_eJoins;
		}

	BEGIN_COM_MAP(CDocumentMenuOutlineJoins)
		COM_INTERFACE_ENTRY(IDocumentMenuCommand)
	END_COM_MAP()

		// IDocumentMenuCommand
	public:
		STDMETHOD(Name)(ILocalizedString** a_ppText)
		{
			if (a_ppText == NULL)
				return E_POINTER;
			try
			{
				wchar_t const* const psz = m_eJoins == EOJTBevel ? L"[0409]Bevel joins[0405]Oříznuté zlomy" : (m_eJoins == EOJTRound ? L"[0409]Round joins[0405]Kulaté zlomy" : L"[0409]Miter joins[0405]Ostré zlomy");
				*a_ppText = new CMultiLanguageString(psz);
				return S_OK;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}
		STDMETHOD(Description)(ILocalizedString** a_ppText)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(IconID)(GUID* a_pIconID);
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon);
		STDMETHOD(Accelerator)(TCmdAccel* UNREF(a_pAccel), TCmdAccel* UNREF(a_pAuxAccel))
		{
			return E_NOTIMPL;
		}
		STDMETHOD(SubCommands)(IEnumUnknowns** a_ppSubCommands)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(State)(EMenuCommandState* a_peState);
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID);

	private:
		CComPtr<IOperationContext> m_pStates;
		CComBSTR m_bstrSyncID;
		BOOL m_bEnabled;
		EOutlineJoinType m_eJoins;
	};

private:
	IRasterImageEditToolsManager* M_Manager()
	{
		if (m_pToolManager)
			return m_pToolManager;
		ObjectLock cLock(this);
		if (m_pToolManager)
			return m_pToolManager;
		RWCoCreateInstance(m_pToolManager, __uuidof(RasterImageEditToolsManager));
		return m_pToolManager;
	}

	IRasterImageFillStyleManager* M_StyleManager()
	{
		if (m_pStyleManager)
			return m_pStyleManager;
		ObjectLock cLock(this);
		if (m_pStyleManager == NULL)
			RWCoCreateInstance(m_pStyleManager, __uuidof(RasterImageFillStyleManager));
		return m_pStyleManager;
	}

private:
	CComPtr<IRasterImageEditToolsManager> m_pToolManager;
	CComPtr<IRasterImageFillStyleManager> m_pStyleManager;
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsOutlineStyle), CMenuCommandsOutlineStyle)
