// GlobalConfigDefaultImageFormat.h : Declaration of the CGlobalConfigDefaultImageFormat

#pragma once
#include "RWDocumentImageRaster.h"



// CGlobalConfigDefaultImageFormat

class ATL_NO_VTABLE CGlobalConfigDefaultImageFormat :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGlobalConfigDefaultImageFormat, &CLSID_GlobalConfigDefaultImageFormat>,
	public IGlobalConfigFactory
{
public:
	CGlobalConfigDefaultImageFormat()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CGlobalConfigDefaultImageFormat)

BEGIN_CATEGORY_MAP(CGlobalConfigDefaultImageFormat)
	IMPLEMENTED_CATEGORY(CATID_GlobalConfigFactory)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CGlobalConfigDefaultImageFormat)
	COM_INTERFACE_ENTRY(IGlobalConfigFactory)
END_COM_MAP()


	// IGlobalConfigFactory methods
public:
	STDMETHOD(Interactive)(BYTE* a_pPriority);
	STDMETHOD(Name)(ILocalizedString** a_ppName);
	STDMETHOD(Description)(ILocalizedString** a_ppDesc);
	STDMETHOD(Config)(IConfig** a_ppConfig);

};

OBJECT_ENTRY_AUTO(__uuidof(GlobalConfigDefaultImageFormat), CGlobalConfigDefaultImageFormat)
