// MenuCommandsHandleSize.h : Declaration of the CMenuCommandsHandleSize

#include "stdafx.h"
#include "RWViewImageRaster.h"
#include <MultiLanguageString.h>
#include <DocumentMenuCommandImpl.h>
#include <IconRenderer.h>


extern GUID const TIconIDSmallHandles = {0x8bab936c, 0xf049, 0x4bba, {0x94, 0xfb, 0x4c, 0x4c, 0x98, 0xa4, 0x89, 0xfc}};
extern GUID const TIconIDNormalHandles = {0xbe3bbdd9, 0x8e96, 0x438a, {0xbb, 0xad, 0xe0, 0x31, 0x4, 0xe7, 0xd6, 0x6c}};
extern GUID const TIconIDLargeHandles = {0xc18b70a5, 0x69f, 0x4e2c, {0x8a, 0xfc, 0x2a, 0x61, 0x58, 0xc3, 0x48, 0x39}};

extern wchar_t const HANDLESIZE_NAME_SMALL[] = L"[0409]Smaller handles[0405]Menší body";
extern wchar_t const HANDLESIZE_NAME_NORMAL[] = L"[0409]Normal handles[0405]Střední body";
extern wchar_t const HANDLESIZE_NAME_LARGE[] = L"[0409]Larger handles[0405]Větší body";
extern wchar_t const HANDLESIZE_DESC[] = L"[0409]Set control handle size.[0405]Nastavit velikost kontrolních bodů.";

// CMenuCommandsHandleSize

class ATL_NO_VTABLE CMenuCommandsHandleSize :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsHandleSize>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsHandleSize()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CMenuCommandsHandleSize)

BEGIN_CATEGORY_MAP(CMenuCommandsHandleSize)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsHandleSize)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName)
	{
		if (a_ppOperationName == NULL)
			return E_POINTER;
		try
		{
			*a_ppOperationName = new CMultiLanguageString(L"[0409]Image Editor - Handle Size[0405]Obrázkový editor - velikost kontrolních bodů");
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig)
	{
		return E_NOTIMPL;
	}
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands)
	{
		try
		{
			*a_ppSubCommands = NULL;

			CComPtr<IEnumUnknownsInit> pItems;
			RWCoCreateInstance(pItems, __uuidof(EnumUnknowns));

			{
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_SMALL, HANDLESIZE_DESC, &TIconIDSmallHandles> >* p = NULL;
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_SMALL, HANDLESIZE_DESC, &TIconIDSmallHandles> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, 3.0f);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_NORMAL, HANDLESIZE_DESC, &TIconIDNormalHandles> >* p = NULL;
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_NORMAL, HANDLESIZE_DESC, &TIconIDNormalHandles> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, 4.0f);
				pItems->Insert(pTmp);
			}
			{
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_LARGE, HANDLESIZE_DESC, &TIconIDLargeHandles> >* p = NULL;
				CComObject<CDocumentMenuCommand<HANDLESIZE_NAME_LARGE, HANDLESIZE_DESC, &TIconIDLargeHandles> >::CreateInstance(&p);
				CComPtr<IDocumentMenuCommand> pTmp = p;
				p->Init(a_pView, 5.333f);
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
	template<wchar_t const* t_pszName, wchar_t const* t_pszDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuCommand :
		public CDocumentMenuCommandMLImpl<CDocumentMenuCommand<t_pszName, t_pszDesc, t_pIconID>, t_pszName, t_pszDesc, t_pIconID, 0>
	{
	public:
		void Init(IDesignerView* a_pView, float a_fSize)
		{
			m_pView = a_pView;
			m_fSize = a_fSize;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CIconRendererReceiver cRenderer(a_nSize);
				IRTarget target((m_fSize+0.5f)*0.125f);
				static IRCanvas const canvas = {0, 0, 256, 256, 0, 0, NULL, NULL};
				static IRPathPoint const circle[] =
				{
					{256, 128, 0, -70.6925, 0, 70.6925},
					{128, 0, -70.6925, 0, 70.6925, 0},
					{0, 128, 0, 70.6925, 0, -70.6925},
					{128, 256, 70.6925, 0, -70.6925, 0},
				};
				static IRFill const white(0xffdddddd);
				static IRFill const black(0xff000000);
				static IROutlinedFill const circleMat(&white, &black);
				cRenderer(&canvas, itemsof(circle), circle, &circleMat, &target);
				static IRPathPoint const shadow1[] =
				{
					{209.317, 46.6827, 44.9103, 44.9103, -9.34491, -9.34491},
					{209.317, 209.317, -44.9102, 44.9102, 44.9103, -44.9103},
					{46.6827, 209.317, -9.34491, -9.34491, 44.9102, 44.9103},
					{24.0553, 178.205, 36.262, 24.4362, 5.45592, 11.2584},
					{154.87, 154.87, 38.7582, -38.7582, -38.7582, 38.7582},
					{178.205, 24.0553, 11.2584, 5.45592, 24.4362, 36.262},
				};
				static IRFill const shadowMat(0x3f000000);
				cRenderer(&canvas, itemsof(shadow1), shadow1, &shadowMat, &target);
				static IRPathPoint const shadow2[] =
				{
					{212.853, 43.1472, 46.863, 46.8629, 0, 0},
					{212.853, 212.853, -46.8629, 46.8629, 46.863, -46.8629},
					{43.1472, 212.853, 0, 0, 46.8629, 46.8629},
					{42.4401, 212.146, 47.3213, 22.9391, 0, 0},
					{187.397, 187.397, 39.2802, -39.2802, -39.2802, 39.2802},
					{212.146, 42.4401, 0, 0, 22.9391, 47.3213},
				};
				cRenderer(&canvas, itemsof(shadow2), shadow2, &shadowMat, &target);
				static IRPathPoint const hilight1[] =
				{
					{110, 40, -10.9347, -10.9347, 10.9347, 10.9347},
					{55, 55, -19.1357, 19.1357, 19.1357, -19.1357},
					{40, 110, 10.9347, 10.9347, -10.9347, -10.9347},
					{95, 95, 19.1357, -19.1357, -19.1357, 19.1357},
				};
				static IRFill const hilightMat(0x3fffffff);
				cRenderer(&canvas, itemsof(hilight1), hilight1, &hilightMat, &target);
				static IRPathPoint const hilight2[] =
				{
					{87, 56, -4.68629, -4.68629, 4.68629, 4.68629},
					{63, 63, -8.59154, 8.59153, 8.59154, -8.59153},
					{56, 87, 4.68629, 4.68629, -4.68629, -4.68629},
					{80, 80, 8.59154, -8.59153, -8.59154, 8.59153},
				};
				cRenderer(&canvas, itemsof(hilight2), hilight2, &hilightMat, &target);
				*a_phIcon = cRenderer.get();
				return (*a_phIcon) ? S_OK : E_FAIL;
			}
			catch (...)
			{
				return a_phIcon ? E_UNEXPECTED : E_POINTER;
			}
		}
		EMenuCommandState IntState()
		{
			CComPtr<IEnumUnknownsInit> p;
			RWCoCreateInstance(p, __uuidof(EnumUnknowns));
			m_pView->QueryInterfaces(__uuidof(IRasterEditView), EQIFVisible, p);
			CComPtr<IRasterEditView> pRIG;
			p->Get(0, __uuidof(IRasterEditView), reinterpret_cast<void**>(&pRIG));
			float fSize = m_fSize;
			return pRIG && pRIG->CanSetHandleSize() == S_OK && SUCCEEDED(pRIG->GetHandleSize(&fSize)) ? (fabsf(fSize-m_fSize) < 1e-03f ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
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
				return pRIG ? pRIG->SetHandleSize(m_fSize) : E_FAIL;
			}
			catch (...)
			{
				return E_UNEXPECTED;
			}
		}

	private:
		CComPtr<IDesignerView> m_pView;
		float m_fSize;
	};
};

// {3E97A60D-2485-4bd4-AA6A-6DBFA26094FF}
static const GUID CLSID_MenuCommandsHandleSize = {0x3e97a60d, 0x2485, 0x4bd4, {0xaa, 0x6a, 0x6d, 0xbf, 0xa2, 0x60, 0x94, 0xff}};

OBJECT_ENTRY_AUTO(CLSID_MenuCommandsHandleSize, CMenuCommandsHandleSize)
