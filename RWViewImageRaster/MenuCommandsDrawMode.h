// MenuCommandsDrawMode.h : Declaration of the CMenuCommandsDrawMode

#pragma once
#include "resource.h"       // main symbols
#include "RWViewImageRaster.h"
#include <SharedStringTable.h>
#include <DocumentMenuCommandImpl.h>
#include "SharedStateToolMode.h"
#include <WeakSingleton.h>
#include <IconRenderer.h>


// CMenuCommandsDrawMode

class ATL_NO_VTABLE CMenuCommandsDrawMode :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMenuCommandsDrawMode, &CLSID_MenuCommandsDrawMode>,
	public IDocumentMenuCommands
{
public:
	CMenuCommandsDrawMode()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_WEAKSINGLETON(CMenuCommandsDrawMode)

BEGIN_CATEGORY_MAP(CMenuCommandsDrawMode)
	IMPLEMENTED_CATEGORY(CATID_DocumentMenuCommands)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CMenuCommandsDrawMode)
	COM_INTERFACE_ENTRY(IDocumentMenuCommands)
END_COM_MAP()


	// IDocumentMenuCommands methods
public:
	STDMETHOD(NameGet)(IMenuCommandsManager* a_pManager, ILocalizedString** a_ppOperationName);
	STDMETHOD(ConfigCreate)(IMenuCommandsManager* a_pManager, IConfig** a_ppDefaultConfig);
	STDMETHOD(CommandsEnum)(IMenuCommandsManager* a_pManager, IConfig* a_pConfig, IOperationContext* a_pStates, IDesignerView* a_pView, IDocument* a_pDocument, IEnumUnknowns** a_ppSubCommands);

private:
	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuRasterizationMode :
		public CDocumentMenuCommandImpl<CDocumentMenuRasterizationMode<t_uIDName, t_uIDDesc, t_pIconID>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, ERasterizationMode a_eMode, DWORD a_dwSupportedModes)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_eMode = a_eMode;
			m_dwSupportedModes = a_dwSupportedModes;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				static IRPolyPoint const nonsmooth[] = { {28, 162}, {94, 160}, {94, 94}, {160, 94}, {160, 28}, {226, 28}, {226, 94}, {160, 94}, {160, 160}, {94, 160}, {94, 226}, {28, 226} };
				static IRPolyPoint const smooth[] = { {30, 192}, {192, 30}, {226, 64}, {64, 226} };
				static IRGridItem const grid[] = { {0, 28}, {1, 94}, {1, 160}, {1, 226} };
				static IRCanvas const canvasNS = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
				static IRCanvas const canvasS = {0, 0, 256, 256, 0, 0, NULL, NULL};
				if (m_eMode == ERMSmooth)
					cRenderer(&canvasS, itemsof(smooth), smooth, pSI->GetMaterial(ESMContrast));
				else
					cRenderer(&canvasNS, itemsof(nonsmooth), nonsmooth, pSI->GetMaterial(ESMContrast));
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
			CComPtr<ISharedStateToolMode> pPrev;
			m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
			if (pPrev == NULL)
				return EMCSDisabled;
			ERasterizationMode eRM;
			pPrev->Get(NULL, NULL, NULL, NULL, &eRM, NULL, NULL, NULL, NULL, NULL, NULL);
			return m_dwSupportedModes&m_eMode ? (eRM == m_eMode || m_dwSupportedModes == m_eMode ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<ISharedStateToolMode> pPrev;
				m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
				CComObject<CSharedStateToolMode>* pNew = NULL;
				CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, &m_eMode, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
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
		ERasterizationMode m_eMode;
		DWORD m_dwSupportedModes;
	};

	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuBlendingMode :
		public CDocumentMenuCommandImpl<CDocumentMenuBlendingMode<t_uIDName, t_uIDDesc, t_pIconID>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, EBlendingMode a_eMode, DWORD a_dwSupportedModes)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_eMode = a_eMode;
			m_dwSupportedModes = a_dwSupportedModes;
		}

		// IDocumentMenuCommand
	public:
		STDMETHOD(Icon)(ULONG a_nSize, HICON* a_phIcon)
		{
			if (a_phIcon == NULL)
				return E_POINTER;
			try
			{
				CComPtr<IStockIcons> pSI;
				RWCoCreateInstance(pSI, __uuidof(StockIcons));
				CIconRendererReceiver cRenderer(a_nSize);
				static IRPathPoint const top[] =
				{
					{0, 24, 0, -13.2548, 0, 0},
					{24, 0, 0, 0, -13.2548, 0},
					{168, 0, 13.2548, 0, 0, 0},
					{192, 24, 0, 0, 0, -13.2548},
					{192, 168, 0, 13.2548, 0, 0},
					{168, 192, 0, 0, 13.2548, 0},
					{24, 192, -13.2548, 0, 0, 0},
					{0, 168, 0, 0, 0, 13.2548},
				};
				static IRPathPoint const cut[] =
				{
					{0, 24, 0, -13.2548, 0, 0},
					{24, 0, 0, 0, -13.2548, 0},
					{168, 0, 13.2548, 0, 0, 0},
					{192, 24, 0, 0, 0, -13.2548},
					{192, 72, 0, 0, 0, 0},
					{88, 72, -8.83656, 0, 0, 0},
					{72, 88, 0, 0, 0, -8.83656},
					{72, 192, 0, 0, 0, 0},
					{24, 192, -13.2548, 0, 0, 0},
					{0, 168, 0, 0, 0, 13.2548},
				};
				static IRPathPoint const bot[] =
				{
					{64, 88, 0, -13.2548, 0, 0},
					{88, 64, 0, 0, -13.2548, 0},
					{232, 64, 13.2548, 0, 0, 0},
					{256, 88, 0, 0, 0, -13.2548},
					{256, 232, 0, 13.2548, 0, 0},
					{232, 256, 0, 0, 13.2548, 0},
					{88, 256, -13.2548, 0, 0, 0},
					{64, 232, 0, 0, 0, 13.2548},
				};
				static IRGridItem const grid[] = { {0, 0}, {0, 64}, {0, 192}, {0, 256} };
				static IRCanvas const canvas = {0, 0, 256, 256, itemsof(grid), itemsof(grid), grid, grid};
				IRFill redFill(0xc0000000|(0xffffff&pSI->GetSRGBColor(ESMDelete)));
				IRFill blueFill(0xc0000000|(0xffffff&pSI->GetSRGBColor(ESMManipulate)));
				IROutlinedFill red(&redFill, pSI->GetMaterial(ESMContrast));
				IROutlinedFill blue(&blueFill, pSI->GetMaterial(ESMContrast));
				switch (m_eMode)
				{
				case EBMDrawUnder:
					cRenderer(&canvas, itemsof(bot), bot, &red);
					cRenderer(&canvas, itemsof(top), top, &blue);
					break;
				case EBMReplace:
					cRenderer(&canvas, itemsof(cut), cut, &blue);
					cRenderer(&canvas, itemsof(bot), bot, &red);
					break;
				default:
					cRenderer(&canvas, itemsof(top), top, &blue);
					cRenderer(&canvas, itemsof(bot), bot, &red);
					break;
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
			CComPtr<ISharedStateToolMode> pPrev;
			m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
			if (pPrev == NULL)
				return EMCSDisabled;
			EBlendingMode eBM;
			pPrev->Get(NULL, NULL, NULL, &eBM, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			return m_dwSupportedModes&m_eMode ? (eBM == m_eMode || m_dwSupportedModes == m_eMode ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<ISharedStateToolMode> pPrev;
				m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
				CComObject<CSharedStateToolMode>* pNew = NULL;
				CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (S_OK == pNew->Set(NULL, NULL, NULL, &m_eMode, NULL, NULL, NULL, NULL, NULL, NULL, NULL, pPrev))
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
		EBlendingMode m_eMode;
		DWORD m_dwSupportedModes;
	};

	template<UINT t_uIDName, UINT t_uIDDesc, GUID const* t_pIconID>
	class ATL_NO_VTABLE CDocumentMenuCoordinatesMode :
		public CDocumentMenuCommandImpl<CDocumentMenuCoordinatesMode<t_uIDName, t_uIDDesc, t_pIconID>, t_uIDName, t_uIDDesc, t_pIconID, 0>
	{
	public:
		void Init(IOperationContext* a_pStates, BSTR a_bstrSyncID, ECoordinatesMode a_eMode, DWORD a_dwSupportedModes)
		{
			m_pStates = a_pStates;
			m_bstrSyncID = a_bstrSyncID;
			m_eMode = a_eMode;
			m_dwSupportedModes = a_dwSupportedModes;
		}

		// IDocumentMenuCommand
	public:
		static void DrawCross(CIconRendererReceiver& cRenderer, ULONG x, ULONG y, ULONG len, DWORD color)
		{
			cRenderer.pixelRow(y)[x] = color;
			for (ULONG i = 1; i < len; ++i)
			{
				cRenderer.pixelRow(y)[x+i] = color;
				cRenderer.pixelRow(y)[x-i] = color;
				cRenderer.pixelRow(y+i)[x] = color;
				cRenderer.pixelRow(y-i)[x] = color;
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
				CIconRendererReceiver cRenderer(a_nSize);
				DWORD color = pSI->GetSRGBColor(ESMContrast);
				ULONG len = (a_nSize+4)/8;
				if (m_eMode == ECMFloatingPoint)
				{
					DrawCross(cRenderer, (3*a_nSize+8)/16, (4*a_nSize+8)/16, len, color);
					DrawCross(cRenderer, (11*a_nSize+8)/16, (2*a_nSize+8)/16, len, color);
					DrawCross(cRenderer, (2*a_nSize+8)/16, (12*a_nSize+8)/16, len, color);
					DrawCross(cRenderer, (13*a_nSize+8)/16, (10*a_nSize+8)/16, len, color);
				}
				else
				{
					DrawCross(cRenderer, a_nSize/4, a_nSize/4, len, color);
					DrawCross(cRenderer, a_nSize/4, a_nSize-a_nSize/4, len, color);
					DrawCross(cRenderer, a_nSize-a_nSize/4, a_nSize/4, len, color);
					DrawCross(cRenderer, a_nSize-a_nSize/4, a_nSize-a_nSize/4, len, color);
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
			CComPtr<ISharedStateToolMode> pPrev;
			m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
			if (pPrev == NULL)
				return EMCSDisabled;
			ECoordinatesMode eCM;
			pPrev->Get(NULL, NULL, NULL, NULL, NULL, &eCM, NULL, NULL, NULL, NULL, NULL);
			return m_dwSupportedModes&m_eMode ? (eCM == m_eMode || m_dwSupportedModes == m_eMode ? EMCSRadioChecked : EMCSRadio) : EMCSDisabled;
		}
		STDMETHOD(Execute)(RWHWND a_hParent, LCID a_tLocaleID)
		{
			try
			{
				CComPtr<ISharedStateToolMode> pPrev;
				m_pStates->StateGet(m_bstrSyncID, __uuidof(ISharedStateToolMode), reinterpret_cast<void**>(&pPrev));
				CComObject<CSharedStateToolMode>* pNew = NULL;
				CComObject<CSharedStateToolMode>::CreateInstance(&pNew);
				CComPtr<ISharedState> pTmp = pNew;
				if (S_OK == pNew->Set(NULL, NULL, NULL, NULL, NULL, &m_eMode, NULL, NULL, NULL, NULL, NULL, pPrev))
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
		ECoordinatesMode m_eMode;
		DWORD m_dwSupportedModes;
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

private:
	CComPtr<IRasterImageEditToolsManager> m_pToolManager;
};

OBJECT_ENTRY_AUTO(__uuidof(MenuCommandsDrawMode), CMenuCommandsDrawMode)
