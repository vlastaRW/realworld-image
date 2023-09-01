// DocumentEncoderRLI.h : Declaration of the CDocumentEncoderRLI

#pragma once
#include "resource.h"       // main symbols

#include "RWCodecImageRealWorld.h"


extern __declspec(selectany) OLECHAR const g_pszSupportedExtensionsRLI[] = L"rli";
extern __declspec(selectany) OLECHAR const g_pszShellIconPathRLI[] = L"%MODULE%,0";
extern __declspec(selectany) OLECHAR const g_pszFormatNameRLI[] = L"[0409]RealWorld layered image files[0405]Soubory vrstvených obrázků RealWorldu";
extern __declspec(selectany) OLECHAR const g_pszTypeNameRLI[] = L"[0409]RealWorld Layered Image[0405]Vrstvený obrázek RealWorldu";
typedef CDocumentTypeCreatorWildchars2<g_pszFormatNameRLI, g_pszTypeNameRLI, g_pszSupportedExtensionsRLI, IDI_RLI_FILETYPE, g_pszShellIconPathRLI> CDocumentTypeCreatorRLI;

extern __declspec(selectany) BYTE const RLI_HEADER[] = {'R', 'W', 'L', 'a', 'y', 'e', 'r', 'e', 'd', 'I', 'm', 'a', 'g', 'e', '_', '1'};
extern __declspec(selectany) DWORD const MARK_LAYER = mmioFOURCC('L', 'A', 'E', 'R');
extern __declspec(selectany) DWORD const MARK_NAME = mmioFOURCC('N', 'A', 'M', 'E');
extern __declspec(selectany) DWORD const MARK_PROPERTIES = mmioFOURCC('P', 'R', 'P', 'S');
extern __declspec(selectany) DWORD const MARK_BLENDING = mmioFOURCC('B', 'L', 'N', 'D');
extern __declspec(selectany) DWORD const MARK_EFFECT = mmioFOURCC('E', 'F', 'C', 'T');
extern __declspec(selectany) DWORD const MARK_DOCUMENT = mmioFOURCC('D', 'O', 'C', 'U');
extern __declspec(selectany) DWORD const MARK_METADATA = mmioFOURCC('M', 'E', 'T', 'A');
extern __declspec(selectany) DWORD const MARK_RESOLUTION = mmioFOURCC('R', 'S', 'L', 'T'); // deprecated
extern __declspec(selectany) DWORD const MARK_CANVASINFO = mmioFOURCC('C', 'N', 'V', 'S');
extern __declspec(selectany) DWORD const MARK_CHANNELS = mmioFOURCC('C', 'H', 'D', 'F');
extern __declspec(selectany) DWORD const MARK_FRAMEINFO = mmioFOURCC('F', 'R', 'M', 'I');
extern __declspec(selectany) DWORD const MARK_LOOPCOUNT = mmioFOURCC('L', 'O', 'O', 'P');


// CDocumentEncoderRLI

class ATL_NO_VTABLE CDocumentEncoderRLI :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderRLI, &CLSID_DocumentEncoderRLI>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderRLI()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderRLI)

BEGIN_CATEGORY_MAP(CDocumentEncoderRLI)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderRLI)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType);
	STDMETHOD(DefaultConfig)(IConfig** a_ppDefCfg);
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects);
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);

};

OBJECT_ENTRY_AUTO(__uuidof(DocumentEncoderRLI), CDocumentEncoderRLI)
