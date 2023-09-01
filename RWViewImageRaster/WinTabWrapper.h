
#pragma once

#include "wintab.h"
// packetdata must be defined before including pktdef.h.  This define
// determines the packet data messages which can be received by the
// program.
#define PACKETDATA (PK_CONTEXT | PK_CURSOR | PK_BUTTONS | PK_X | \
                    PK_Y | PK_TANGENT_PRESSURE | PK_NORMAL_PRESSURE | \
                    PK_ORIENTATION)
// don't know what the next define is for, but it makes things work.
#define PACKETMODE (PK_BUTTONS)
#include "pktdef.h"


class CWinTabWrapper :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IUnknown
{
public:
	CWinTabWrapper() : m_hWinTab(NULL),
		m_pfnWTInfo(NULL), m_pfnWTOpen(NULL), m_pfnWTClose(NULL), m_pfnWTPacket(NULL)
	{
	}
	~CWinTabWrapper()
	{
		if (m_hWinTab)
			FreeLibrary(m_hWinTab);
	}
	void Init()
	{
		m_hWinTab = LoadLibrary(_T("WINTAB32.DLL"));
		if (m_hWinTab == NULL)
			return;
		*(FARPROC*)&m_pfnWTInfo = GetProcAddress(m_hWinTab,
#ifdef _UNICODE
			"WTInfoW"
#else
			"WTInfoA"
#endif
			);
		*(FARPROC*)&m_pfnWTOpen = GetProcAddress(m_hWinTab,
#ifdef _UNICODE
			"WTOpenW"
#else
			"WTOpenA"
#endif
			);
		*(FARPROC*)&m_pfnWTClose = GetProcAddress(m_hWinTab, "WTClose");
		*(FARPROC*)&m_pfnWTPacket = GetProcAddress(m_hWinTab, "WTPacket");
	}

BEGIN_COM_MAP(CWinTabWrapper)
	COM_INTERFACE_ENTRY(IUnknown)
END_COM_MAP()

public:
	UINT WTInfo(UINT wCategory, UINT nIndex, LPVOID lpOutput) const
	{
		return m_pfnWTInfo ? m_pfnWTInfo(wCategory, nIndex, lpOutput) : 0;
	}
	HCTX WTOpen(HWND hWnd, LPLOGCONTEXT lpLogCtx, BOOL fEnable) const
	{
		return m_pfnWTOpen ? m_pfnWTOpen(hWnd, lpLogCtx, fEnable) : NULL;
	}
	BOOL WTClose(HCTX hCtx) const
	{
		return m_pfnWTClose ? m_pfnWTClose(hCtx) : FALSE;
	}
	BOOL WTPacket(HCTX hCtx, UINT wSerial, LPVOID lpPkt) const
	{
		return m_pfnWTPacket ? m_pfnWTPacket(hCtx, wSerial, lpPkt) : FALSE;
	}

private:
	HMODULE m_hWinTab;
	UINT (WINAPI *m_pfnWTInfo)(UINT wCategory, UINT nIndex, LPVOID lpOutput);
	HCTX (WINAPI *m_pfnWTOpen)(HWND hWnd, LPLOGCONTEXT lpLogCtx, BOOL fEnable);
	BOOL (WINAPI *m_pfnWTClose)(HCTX hCtx);
	BOOL (WINAPI *m_pfnWTPacket)(HCTX hCtx, UINT wSerial, LPVOID lpPkt);
};
