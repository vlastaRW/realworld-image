
#pragma once

#define WT_STYLUSPACKET WM_USER+0x7b

class CRWStylusSyncPlugin : public IStylusSyncPlugin
{
public:
	CRWStylusSyncPlugin(HWND a_hWnd) : m_nRef(1), m_hWnd(a_hWnd), m_iSent(0), m_iReceived(0)

	{
	    CoCreateFreeThreadedMarshaler(this, &m_pPunkFTMarshaller);
	}
	virtual ~CRWStylusSyncPlugin() 
	{
		if (m_pPunkFTMarshaller != NULL)
			m_pPunkFTMarshaller->Release(); 
	}

	void WindowDestroyed()
	{
		m_hWnd = NULL;
		Release();
	}

	// Detect what tablet context IDs will give us pressure data
	bool InitTablets(IRealTimeStylus* a_pRTS)
	{
		ULONG nTabletContexts = 0;
		TABLET_CONTEXT_ID* piTabletContexts = NULL; // who deletes this???
		HRESULT res = a_pRTS->GetAllTabletContextIds(&nTabletContexts, &piTabletContexts);
		for (ULONG i = 0; i < nTabletContexts; ++i)
		{
			IInkTablet* pInkTablet = NULL;
			if (SUCCEEDED(a_pRTS->GetTabletFromTabletContextId(piTabletContexts[i], &pInkTablet)))
			{
				float ScaleX = 1.0f, ScaleY = 1.0f;
				ULONG nPacketProps = 0;
				PACKET_PROPERTY* pPacketProps = NULL;
				a_pRTS->GetPacketDescriptionData(piTabletContexts[i], &ScaleX, &ScaleY, &nPacketProps, &pPacketProps);

				STabletProps tProps;
				tProps.id = piTabletContexts[i];
				tProps.iX = 0x10000;
				tProps.fX = 1.0f;
				tProps.iY = 0x10000;
				tProps.fY = 1.0f;
				tProps.iPressure = 0x10000;
				tProps.fPressure = 1.0f;
				for (ULONG j = 0; j < nPacketProps; ++j)
				{
					if (IsEqualGUID(pPacketProps[j].guid, GUID_PACKETPROPERTY_GUID_X))
					{
						tProps.iX = j;
						tProps.fX = 1.0f / pPacketProps[j].PropertyMetrics.nLogicalMax * ScaleX;
					}
					else if (IsEqualGUID(pPacketProps[j].guid, GUID_PACKETPROPERTY_GUID_Y))
					{
						tProps.iY = j;
						tProps.fY = 1.0f / pPacketProps[j].PropertyMetrics.nLogicalMax * ScaleY;
					}
					else if (IsEqualGUID(pPacketProps[j].guid, GUID_PACKETPROPERTY_GUID_NORMAL_PRESSURE))
					{
						tProps.iPressure = j;
						tProps.fPressure = 1.0f / pPacketProps[j].PropertyMetrics.nLogicalMax;
					}
				}
				CoTaskMemFree(pPacketProps);
				if (tProps.iX != 0x10000 && tProps.iY != 0x10000 && tProps.iPressure != 0x10000)
					m_aTablets.push_back(tProps);
			}
		}

		// If we can't get pressure information, no use in having the tablet context
		return !m_aTablets.empty();
	}

	// IUnknown methods
public:
	STDMETHOD_(ULONG, AddRef)()
	{
		return InterlockedIncrement(&m_nRef);
	}
	STDMETHOD_(ULONG, Release)()
	{
		ULONG nNewRef = InterlockedDecrement(&m_nRef);
		if (nNewRef == 0)
			delete this;

		return nNewRef;
	}
	STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj)
	{
		if (IsEqualGUID(riid, __uuidof(IStylusSyncPlugin)) || IsEqualGUID(riid, __uuidof(IUnknown)))
		{
			*ppvObj = this;
			AddRef();
			return S_OK;
		}
		else if (IsEqualGUID(riid, __uuidof(IMarshal)) && (m_pPunkFTMarshaller != NULL))
		{
			return m_pPunkFTMarshaller->QueryInterface(riid, ppvObj);
		}

		*ppvObj = NULL;
		return E_NOINTERFACE;
	}

	// Methods whose data we use
    STDMETHOD(Packets)(IRealTimeStylus* pStylus, StylusInfo const* pStylusInfo, ULONG cPktCount, ULONG cPktBuffLength, LONG* pPackets, ULONG* nOutPackets, LONG** ppOutPackets)
	{
		std::vector<STabletProps>::const_iterator i = m_aTablets.begin();
		while (i != m_aTablets.end() && i->id != pStylusInfo->tcid)
			++i;

		if (i == m_aTablets.end())
			return S_OK; // not one of interesting tablets

		// Get properties
		ULONG nProps = cPktBuffLength / cPktCount;

		// Pointer to the last packet in the batch - we don't really care about intermediates for this example
		LONG *pLastPacket = pPackets + (cPktBuffLength - nProps);
		
		m_sPacket.x = i->fX*pLastPacket[i->iX];
		m_sPacket.y = i->fY*pLastPacket[i->iY];
		m_sPacket.n = i->fPressure*pLastPacket[i->iPressure];
		if (m_iSent <= m_iReceived)
		{
			++m_iSent;
			::PostMessage(m_hWnd, WT_STYLUSPACKET, 0, 0);
		}
		//g_bInverted = (pStylusInfo->bIsInvertedCursor != 0);
		
		return S_OK;
	}
	STDMETHOD(StylusUp)(IRealTimeStylus* pStylus, StylusInfo const* pStylusInfo, ULONG cPropCountPerPkt, LONG* pPacket, LONG** ppInOutPkt)
	{
		std::vector<STabletProps>::const_iterator i = m_aTablets.begin();
		while (i != m_aTablets.end() && i->id != pStylusInfo->tcid)
			++i;

		if (i == m_aTablets.end())
			return S_OK; // not one of interesting tablets

		LONG *pLastPacket = pPacket;
		
		m_sPacket.x = i->fX*pLastPacket[i->iX];
		m_sPacket.y = i->fY*pLastPacket[i->iY];
		m_sPacket.n = i->fPressure*pLastPacket[i->iPressure];
		if (m_iSent <= m_iReceived)
		{
			++m_iSent;
			::PostMessage(m_hWnd, WT_STYLUSPACKET, 0, 0);
		}
		//g_bInverted = (pStylusInfo->bIsInvertedCursor != 0);
		
		return S_OK;
	}
    STDMETHOD(DataInterest)(RealTimeStylusDataInterest* pEventInterest)
	{
		*pEventInterest = static_cast<RealTimeStylusDataInterest>(RTSDI_Packets|RTSDI_StylusUp);
		return S_OK;
	}

    // Methods you can add if you need the alerts - don't forget to change DataInterest!
	STDMETHOD(StylusDown)(IRealTimeStylus*, const StylusInfo*, ULONG, LONG* _pPackets, LONG**) { return S_OK; }
    STDMETHOD(RealTimeStylusEnabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(RealTimeStylusDisabled)(IRealTimeStylus*, ULONG, const TABLET_CONTEXT_ID*) { return S_OK; }
    STDMETHOD(StylusInRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(StylusOutOfRange)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID) { return S_OK; }
    STDMETHOD(InAirPackets)(IRealTimeStylus*, const StylusInfo*, ULONG, ULONG, LONG*, ULONG*, LONG**) { return S_OK; }
    STDMETHOD(StylusButtonUp)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(StylusButtonDown)(IRealTimeStylus*, STYLUS_ID, const GUID*, POINT*) { return S_OK; }
    STDMETHOD(SystemEvent)(IRealTimeStylus*, TABLET_CONTEXT_ID, STYLUS_ID, SYSTEM_EVENT, SYSTEM_EVENT_DATA) { return S_OK; }
    STDMETHOD(TabletAdded)(IRealTimeStylus*, IInkTablet*) { return S_OK; }
    STDMETHOD(TabletRemoved)(IRealTimeStylus*, LONG) { return S_OK; }
    STDMETHOD(CustomStylusDataAdded)(IRealTimeStylus*, const GUID*, ULONG, const BYTE*) { return S_OK; }
    STDMETHOD(Error)(IRealTimeStylus*, IStylusPlugin*, RealTimeStylusDataInterest, HRESULT, LONG_PTR*) { return S_OK; }
    STDMETHOD(UpdateMapping)(IRealTimeStylus*) { return S_OK; }

public:
	struct SPacket
	{
		float x;
		float y;
		float n;
	};
	SPacket m_sPacket;
	int m_iSent;
	int m_iReceived;

private:
	struct STabletProps
	{
		TABLET_CONTEXT_ID id;
		ULONG iX;
		float fX;
		ULONG iY;
		float fY;
		ULONG iPressure;
		float fPressure;
	};

private:
    LONG m_nRef;
	HWND m_hWnd;
	std::vector<STabletProps> m_aTablets;
    IUnknown* m_pPunkFTMarshaller;  // free-threaded marshaller
};
