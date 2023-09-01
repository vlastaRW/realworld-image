// DocumentEncoderSVG.cpp : Implementation of CDocumentEncoderSVG

#include "stdafx.h"
#include "DocumentTypeSVG.h"

#include <RWDocumentImageVector.h>
#include <RWDocumentImageRaster.h>
#include <MultiLanguageString.h>
#include <RWBaseEnumUtils.h>
#include "../RWCodecImagePNG/RWImageCodecPNG.h"
#include <boost/spirit.hpp>
using namespace boost::spirit;
#include <GammaCorrection.h>
#include <set>

extern GUID const CLSID_DocumentEncoderSVG = {0x8684c110, 0xeb9b, 0x4168, {0xa3, 0xaf, 0x87, 0x82, 0xbe, 0x10, 0x89, 0x4e}};


// CDocumentEncoderSVG

class ATL_NO_VTABLE CDocumentEncoderSVG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentEncoderSVG, &CLSID_DocumentEncoderSVG>,
	public IDocumentEncoder
{
public:
	CDocumentEncoderSVG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentEncoderSVG)

BEGIN_CATEGORY_MAP(CDocumentCodecWebP)
	IMPLEMENTED_CATEGORY(CATID_DocumentEncoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentEncoderSVG)
	COM_INTERFACE_ENTRY(IDocumentEncoder)
END_COM_MAP()


	// IDocumentEncoder methods
public:
	STDMETHOD(DocumentType)(IDocumentType** a_ppDocType)
	{
		if (a_ppDocType == NULL)
			return E_POINTER;

		try
		{
			*a_ppDocType = CDocumentTypeCreatorSVG::Create();
			return S_OK;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(DefaultConfig)(IConfig** UNREF(a_ppDefCfg))
	{
		return E_NOTIMPL;
	}
	STDMETHOD(CanSerialize)(IDocument* a_pDoc, BSTR* a_pbstrAspects)
	{
		try
		{
			CComPtr<IDocumentImage> pDocImg;
			a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
			if (a_pbstrAspects) *a_pbstrAspects = ::SysAllocString(ENCFEAT_IMAGE ENCFEAT_IMAGE_ALPHA ENCFEAT_IMAGE_LAYER L"[vecimg]");
			return pDocImg ? S_OK : S_FALSE;
		}
		catch (...)
		{
			return E_UNEXPECTED;
		}
	}
	STDMETHOD(Serialize)(IDocument* a_pDoc, IConfig* a_pCfg, IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl);
};

OBJECT_ENTRY_AUTO(CLSID_DocumentEncoderSVG, CDocumentEncoderSVG)


class COutputBufferData : public IReturnedData
{
public:
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }
	STDMETHOD(QueryInterface)(REFIID, void**) { return E_NOTIMPL; }
	STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pData)
	{
		if (a_nSize) m_cBuf.insert(m_cBuf.end(), a_pData, a_pData+a_nSize);
		return S_OK;
	}
	BYTE const* Data() const { return &m_cBuf[0]; }
	ULONG Length() const { return ULONG(m_cBuf.size()); }

private:
	std::vector<BYTE> m_cBuf;
};

class COutputHelper
{
public:
	COutputHelper(IReturnedData* a_pDst) : m_pDst(a_pDst)
	{
	}
	operator IReturnedData*() { return m_pDst; }

	COutputHelper& operator<<(char const* a_pszText)
	{
		size_t n = strlen(a_pszText);
		if (n) m_pDst->Write(ULONG(n), (BYTE*)a_pszText);
		return *this;
	}
	COutputHelper& operator<<(std::string const& a_cText)
	{
		if (!a_cText.empty()) m_pDst->Write(a_cText.length(), (BYTE*)a_cText.c_str());
		return *this;
	}
	COutputHelper& operator<<(wchar_t const* a_pszText)
	{
		CW2AEX<> str(a_pszText, CP_UTF8);
		size_t n = strlen(str);
		if (n) m_pDst->Write(ULONG(n), (BYTE*)str.operator LPSTR());
		return *this;
	}
	COutputHelper& operator<<(char const a_cChar)
	{
		m_pDst->Write(1, (BYTE*)&a_cChar);
		return *this;
	}
	COutputHelper& operator<<(float a_fNum)
	{
		char sz[32];
		sprintf(sz, "%g", a_fNum);
		m_pDst->Write(ULONG(strlen(sz)), (BYTE*)sz);
		return *this;
	}
	COutputHelper& operator<<(double a_fNum)
	{
		char sz[32];
		sprintf(sz, "%g", (float)a_fNum);
		m_pDst->Write(ULONG(strlen(sz)), (BYTE*)sz);
		return *this;
	}
	COutputHelper& operator<<(int a_iNum)
	{
		char sz[32];
		sprintf(sz, "%i", a_iNum);
		m_pDst->Write(ULONG(strlen(sz)), (BYTE*)sz);
		return *this;
	}
	COutputHelper& operator<<(COutputBufferData const& a_cBuf)
	{
		if (a_cBuf.Length()) m_pDst->Write(a_cBuf.Length(), a_cBuf.Data());
		return *this;
	}
	unsigned long PutData(unsigned char const * a_pData, unsigned long a_nBytes)
	{
		if (a_nBytes) m_pDst->Write(a_nBytes, (BYTE*)a_pData);
		return a_nBytes;
	}
	static char const* EOL() { return "\r\n"; }

private:
	IReturnedData* m_pDst;
};

class COutputBuffer : public COutputHelper, public COutputBufferData
{
public:
	COutputBuffer() : COutputHelper(static_cast<IReturnedData*>(this)) {}
};

struct SSVGNode
{
	std::string type;
	std::string id;
	std::string value;
	std::map<std::string, std::string> attributes; // other than id
	std::vector<SSVGNode> subNodes;

	void SetAttr(char const* name, char const* value, size_t length)
	{
		attributes[name].assign(value, value+length);
	}
	void SetAttr(char const* name, char const* value)
	{
		attributes[name] = value;
	}
	void SetAttr(char const* name, wchar_t const* value)
	{
		CW2AEX<> str(value, CP_UTF8);
		SetAttr(name, static_cast<char const*>(str));
	}
	void SetAttr(char const* name, float value)
	{
		char sz[16];
		sprintf(sz, "%g", value);
		SetAttr(name, sz);
	}
	void SetAttr(char const* name, float value, char const* unit)
	{
		char sz[64];
		sprintf(sz, "%g%s", value, unit);
		SetAttr(name, sz);
	}
	bool operator==(SSVGNode const& rhs) const
	{
		return type == rhs.type && id == rhs.id && value == rhs.value && attributes == rhs.attributes && subNodes == rhs.subNodes;
	}
	bool EqualsExceptID(SSVGNode const& rhs) const
	{
		return type == rhs.type && value == rhs.value && attributes == rhs.attributes && subNodes == rhs.subNodes;
	}

	void swap(SSVGNode& rhs)
	{
		type.swap(rhs.type);
		id.swap(rhs.id);
		value.swap(rhs.value);
		attributes.swap(rhs.attributes);
		subNodes.swap(rhs.subNodes);
	}
	void Write(int indent, COutputHelper& dst) const
	{
		static char const szInd[17] = "                ";
		int ind = indent*2;
		while (ind > 16)
		{
			dst << szInd;
			ind -= 16;
		}
		if (ind > 0)
			dst << szInd+(16-ind);
		dst << '<' << type;
		if (!id.empty())
			dst << " id=\"" << id << "\"";
		for (std::map<std::string, std::string>::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
		{
			dst << " " << i->first << "=\"";
			WriteEscaped(i->second, dst);
			dst << "\"";
		}
		if (subNodes.empty() && value.empty())
		{
			dst << " />" << dst.EOL();
		}
		else if (!subNodes.empty())
		{
			dst << '>' << dst.EOL();
			for (std::vector<SSVGNode>::const_iterator i = subNodes.begin(); i != subNodes.end(); ++i)
				i->Write(indent+1, dst);
			ind = indent*2;
			while (ind > 16)
			{
				dst << szInd;
				ind -= 16;
			}
			if (ind > 0)
				dst << szInd+(16-ind);
			dst << "</" << type << '>' << dst.EOL();
		}
		else
		{
			dst << '>';
			WriteEscaped(value, dst);
			dst << "</" << type << '>' << dst.EOL();
		}
	}

private:
	static void WriteEscaped(std::string const& str, COutputHelper& dst)
	{
		char const* beg = str.c_str();
		char const* const end = str.c_str()+str.length();
		char const* cur = beg;
		while (cur != end)
		{
			switch (*cur)
			{
			case '&':
				if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
				dst << "&amp;";
				beg = ++cur;
				break;
			case '\'':
				if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
				dst << "&apos;";
				beg = ++cur;
				break;
			case '\"':
				if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
				dst << "&quot;";
				beg = ++cur;
				break;
			case '\<':
				if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
				dst << "&lt;";
				beg = ++cur;
				break;
			case '\>':
				if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
				dst << "&gt;";
				beg = ++cur;
				break;
			default:
				++cur;
			}
		}
		if (beg != cur) dst.PutData(reinterpret_cast<BYTE const*>(beg), cur-beg);
	}
};

struct SSVGRootNode
{
	std::vector<SSVGNode> defs;
	std::vector<SSVGNode> nodes;

	void Write(COutputHelper& dst)
	{
		if (!defs.empty())
		{
			dst << "  <defs>" << dst.EOL();
			for (std::vector<SSVGNode>::const_iterator i = defs.begin(); i != defs.end(); ++i)
				i->Write(2, dst);
			dst << "  </defs>" << dst.EOL();
		}
		for (std::vector<SSVGNode>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			i->Write(1, dst);
	}
};

class CToolWindow :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditWindow
{
public:

BEGIN_COM_MAP(CToolWindow)
	COM_INTERFACE_ENTRY(IRasterImageEditWindow)
END_COM_MAP()

	// IRasterImageEditWindow methods
public:
	STDMETHOD(Size)(ULONG* a_pSizeX, ULONG* a_pSizeY)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(GetDefaultColor)(TRasterImagePixel* a_pDefault)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(GetImageTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, float a_fGamma, ULONG a_nStride, EImageTileIntent a_eIntent, TRasterImagePixel* a_pBuffer)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(GetSelectionInfo)(RECT* a_pBoundingRectangle, BOOL* a_bEntireRectangle)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(GetSelectionTile)(LONG a_nX, LONG a_nY, ULONG a_nSizeX, ULONG a_nSizeY, ULONG a_nStride, BYTE* a_pBuffer)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointsChanged)()
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlPointChanged)(ULONG a_nIndex)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(ControlLinesChanged)()
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(RectangleChanged)(RECT const* a_pChanged)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(ScrollWindow)(ULONG a_nScrollID, TPixelCoords const* a_pDelta)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(SetState)(ISharedState* a_pState)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(SetBrushState)(BSTR a_bstrStyleID, ISharedState* a_pState)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(Handle)(RWHWND* a_phWnd)
	{
		return E_UNEXPECTED;
	}
	STDMETHOD(Document)(IDocument** a_ppDocument)
	{
		return E_UNEXPECTED;
	}
};

static char const s_tBase64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
class CBase64Encoder : public IReturnedData
{
public:
	CBase64Encoder(IReturnedData* a_pBase) : m_pBase(a_pBase), m_nExtra(0) {}
	operator IReturnedData*() { return this; }
	~CBase64Encoder()
	{
		if (m_nExtra)
		{
			BYTE b[4] = {'=', '=', '=', '='};
			b[0] = s_tBase64Encode[(m_bExtra[0] >> 2) & 0x3f];
			if (m_nExtra == 2)
			{
				b[1] = s_tBase64Encode[((m_bExtra[0] << 4) & 0x30) | ((m_bExtra[1] >> 4) & 0x0f)];
				b[2] = s_tBase64Encode[(m_bExtra[1] << 2) & 0x3c];
			}
			else
			{
				b[1] = s_tBase64Encode[(m_bExtra[0] << 4) & 0x30];
			}
			m_pBase->Write(4, b);
		}
	}
	STDMETHOD_(ULONG, AddRef)() { return 2; }
	STDMETHOD_(ULONG, Release)() { return 1; }
	STDMETHOD(QueryInterface)(REFIID a_iid, void** a_pp)
	{
		if (IsEqualGUID(a_iid, __uuidof(IUnknown)) || IsEqualGUID(a_iid, __uuidof(IReturnedData)))
		{
			*a_pp = this;
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	STDMETHOD(Write)(ULONG a_nSize, BYTE const* a_pData)
	{
		if (a_nSize == 0) return S_FALSE;
		ULONG nTriplets = (a_nSize+m_nExtra)/3;
		if (nTriplets)
		{
			CAutoVectorPtr<BYTE> aOut(new BYTE[nTriplets*4]);
			BYTE* pOut = aOut;
			for (ULONG i = 0; i < nTriplets; ++i)
			{
				int const j = i*3;
				BYTE const b0 = j < m_nExtra ? m_bExtra[j] : a_pData[j-m_nExtra];
				BYTE const b1 = j+1 < m_nExtra ? m_bExtra[j+1] : a_pData[j+1-m_nExtra];
				BYTE const b2 = a_pData[j+2-m_nExtra];

				*pOut++ = s_tBase64Encode[(b0 >> 2) & 0x3f];
				*pOut++ = s_tBase64Encode[((b0 << 4) & 0x30) | ((b1 >> 4) & 0x0f)];
				*pOut++ = s_tBase64Encode[((b1 << 2) & 0x3c) | ((b2 >> 6) & 0x03)];
				*pOut++ = s_tBase64Encode[b2 & 0x3f];
			}
			m_pBase->Write(nTriplets*4, aOut);
		}
		m_nExtra = a_nSize+m_nExtra-nTriplets*3;
		if (m_nExtra)
		{
			if (a_nSize > 1)
				m_bExtra[0] = a_pData[a_nSize-m_nExtra];
			else if (m_nExtra == 1)
				m_bExtra[0] = a_pData[a_nSize-1];
			if (m_nExtra == 2)
				m_bExtra[1] = a_pData[a_nSize-1];
		}
		return S_OK;
	}

private:
	IReturnedData* m_pBase;
	BYTE m_bExtra[2];
	BYTE m_nExtra;
};

HRESULT SerializeRasterImage(COutputHelper& a_cOut, IDocument* a_pDoc)
{
	CComPtr<IDocumentImage> pDocImg;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
	if (pDocImg == NULL) return E_FAIL;
	CComPtr<IDocumentEncoder> pEnc;
	RWCoCreateInstance(pEnc, __uuidof(DocumentEncoderPNG));
	if (pEnc == NULL) return E_INVALIDARG;

	CComPtr<IDocument> pDoc;
	RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
	a_pDoc->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pDoc), NULL, NULL);
	CComPtr<IDocumentEditableImage> pImg;
	pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pImg));
	if (pImg == NULL) return E_FAIL;
	TImageSize tCanvas = {1, 1};
	TImagePoint tOffset = {0, 0};
	TImageSize tContent = {1, 1};
	pImg->CanvasGet(&tCanvas, NULL, &tOffset, &tContent, NULL);
	if (tCanvas.nX != tContent.nX || tCanvas.nY != tContent.nY || tOffset.nX || tOffset.nY)
	{
		TMatrix3x3f tMtx = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  -tOffset.nX, -tOffset.nY, 1.0f};
		pImg->CanvasSet(&tContent, NULL, &tMtx, NULL);
	}

	CComPtr<IConfig> pCfg;
	pEnc->DefaultConfig(&pCfg);
	a_cOut << "<image width=\"" << int(tContent.nX) << "\" height=\"" << int(tContent.nY) << "\"";
	if (tOffset.nX || tOffset.nY)
		a_cOut << " x=\"" << int(tOffset.nX) << "\" y=\"" << int(tOffset.nY) << "\"";
	a_cOut << " xlink:href=\"data:image/png;base64,";
	HRESULT hRes = pEnc->Serialize(pDoc, pCfg, CBase64Encoder(a_cOut), NULL, NULL);
	a_cOut << "\" />" << COutputHelper::EOL();
	return hRes;
}

class ATL_NO_VTABLE CPolygonCallback :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditToolPolygon
{
public:
	void Init(bool a_bClosed, SSVGNode& a_cNode)
	{
		m_pNode = &a_cNode;
		m_bClosed = a_bClosed;
	}

BEGIN_COM_MAP(CPolygonCallback)
	COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
END_COM_MAP()

	// IRasterImageEditToolPolygon methods
public:
	STDMETHOD(FromPolygon)(ULONG a_nCount, TRWPolygon const* a_pPolygons)
	{
		m_pNode->type = m_bClosed ? "polygon" : "polyline";
		std::vector<char> buffer;
		char sz[64];
		for (ULONG i = 0; i < 1/*a_nCount*/; ++i)
			 // TODO: handle case when multiple poygons are sent -> use <g> with subnodes
		{
			if (a_pPolygons[i].nVertices > 1)
			{
				bool bSpace = false;
				for (ULONG j = 0; j < a_pPolygons[i].nVertices; ++j)
				{
					if (bSpace)
						buffer.push_back(' ');
					else
						bSpace = true;
					sprintf(sz, "%g,%g", a_pPolygons[i].pVertices[j].fX, a_pPolygons[i].pVertices[j].fY);
					buffer.insert(buffer.end(), sz, sz+strlen(sz));
				}
			}
		}
		m_pNode->SetAttr("points", &(buffer[0]), buffer.size());
		return S_OK;
	}
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		m_pNode->type = "path";
		std::vector<char> buffer;
		char sz[256];
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			if (a_pPaths[i].nVertices > 1)
			{
				if (i) buffer.push_back(' ');
				sprintf(sz, "M %g %g", a_pPaths[i].pVertices[0].tPos.fX, a_pPaths[i].pVertices[0].tPos.fY);
				buffer.insert(buffer.end(), sz, sz+strlen(sz));
				for (ULONG j = 1; j < a_pPaths[i].nVertices; ++j)
				{
					if (a_pPaths[i].pVertices[j-1].tTanNext.fX == 0.0f && a_pPaths[i].pVertices[j-1].tTanNext.fY == 0.0f &&
						a_pPaths[i].pVertices[j].tTanPrev.fX == 0.0f && a_pPaths[i].pVertices[j].tTanPrev.fY == 0.0f)
						sprintf(sz, "L %g %g", a_pPaths[i].pVertices[j].tPos.fX, a_pPaths[i].pVertices[j].tPos.fY);
					else
						sprintf(sz, "C %g %g %g %g %g %g",
							a_pPaths[i].pVertices[j-1].tPos.fX+a_pPaths[i].pVertices[j-1].tTanNext.fX,
							a_pPaths[i].pVertices[j-1].tPos.fY+a_pPaths[i].pVertices[j-1].tTanNext.fY,
							a_pPaths[i].pVertices[j].tPos.fX+a_pPaths[i].pVertices[j].tTanPrev.fX,
							a_pPaths[i].pVertices[j].tPos.fY+a_pPaths[i].pVertices[j].tTanPrev.fY,
							a_pPaths[i].pVertices[j].tPos.fX, a_pPaths[i].pVertices[j].tPos.fY);
					buffer.insert(buffer.end(), sz, sz+strlen(sz));
				}
				if (m_bClosed || a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].dwFlags&8)
				{
					sprintf(sz, "C %g %g %g %g %g %g z",
						a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tPos.fX+a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tTanNext.fX,
						a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tPos.fY+a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tTanNext.fY,
						a_pPaths[i].pVertices[0].tPos.fX+a_pPaths[i].pVertices[0].tTanPrev.fX,
						a_pPaths[i].pVertices[0].tPos.fY+a_pPaths[i].pVertices[0].tTanPrev.fY,
						a_pPaths[i].pVertices[0].tPos.fX, a_pPaths[i].pVertices[0].tPos.fY);
					buffer.insert(buffer.end(), sz, sz+strlen(sz));
				}
			}
		}
		m_pNode->SetAttr("d", &(buffer[0]), buffer.size());
		return S_OK;
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }

private:
	bool m_bClosed;
	SSVGNode* m_pNode;
};

class ATL_NO_VTABLE CShapeCallback :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRasterImageEditToolPolygon
{
public:
	void Init(SSVGNode& a_cNode)
	{
		m_pNode = &a_cNode;
	}

BEGIN_COM_MAP(CShapeCallback)
	COM_INTERFACE_ENTRY(IRasterImageEditToolPolygon)
END_COM_MAP()

	// IRasterImageEditToolPolygon methods
public:
	STDMETHOD(FromPolygon)(ULONG a_nCount, TRWPolygon const* a_pPolygons) { return E_NOTIMPL; }
	STDMETHOD(ToPolygon)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }
	STDMETHOD(FromPath)(ULONG a_nCount, TRWPath const* a_pPaths)
	{
		m_pNode->SetAttr("fill-rule", "evenodd");
		std::vector<char> buffer;
		char sz[256];
		for (ULONG i = 0; i < a_nCount; ++i)
		{
			if (a_pPaths[i].nVertices > 1)
			{
				if (i) buffer.push_back(' ');
				sprintf(sz, "M %g %g", a_pPaths[i].pVertices[0].tPos.fX, a_pPaths[i].pVertices[0].tPos.fY);
				buffer.insert(buffer.end(), sz, sz+strlen(sz));
				for (ULONG j = 1; j < a_pPaths[i].nVertices; ++j)
				{
					if (a_pPaths[i].pVertices[j-1].tTanNext.fX == 0.0f && a_pPaths[i].pVertices[j-1].tTanNext.fY == 0.0f &&
						a_pPaths[i].pVertices[j].tTanPrev.fX == 0.0f && a_pPaths[i].pVertices[j].tTanPrev.fY == 0.0f)
						sprintf(sz, "L %g %g", a_pPaths[i].pVertices[j].tPos.fX, a_pPaths[i].pVertices[j].tPos.fY);
					else
						sprintf(sz, "C %g %g %g %g %g %g",
							a_pPaths[i].pVertices[j-1].tPos.fX+a_pPaths[i].pVertices[j-1].tTanNext.fX,
							a_pPaths[i].pVertices[j-1].tPos.fY+a_pPaths[i].pVertices[j-1].tTanNext.fY,
							a_pPaths[i].pVertices[j].tPos.fX+a_pPaths[i].pVertices[j].tTanPrev.fX,
							a_pPaths[i].pVertices[j].tPos.fY+a_pPaths[i].pVertices[j].tTanPrev.fY,
							a_pPaths[i].pVertices[j].tPos.fX, a_pPaths[i].pVertices[j].tPos.fY);
					buffer.insert(buffer.end(), sz, sz+strlen(sz));
				}
				sprintf(sz, "C %g %g %g %g %g %g z",
					a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tPos.fX+a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tTanNext.fX,
					a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tPos.fY+a_pPaths[i].pVertices[a_pPaths[i].nVertices-1].tTanNext.fY,
					a_pPaths[i].pVertices[0].tPos.fX+a_pPaths[i].pVertices[0].tTanPrev.fX,
					a_pPaths[i].pVertices[0].tPos.fY+a_pPaths[i].pVertices[0].tTanPrev.fY,
					a_pPaths[i].pVertices[0].tPos.fX, a_pPaths[i].pVertices[0].tPos.fY);
				buffer.insert(buffer.end(), sz, sz+strlen(sz));
			}
		}
		m_pNode->SetAttr("d", &(buffer[0]), buffer.size());
		return S_OK;
	}
	STDMETHOD(ToPath)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }

private:
	SSVGNode* m_pNode;
};

bool ComputeFill(CComBSTR const& bstrStyleID, CComBSTR const& bstrStyleParams, char const* fillName, char const* fillOpacityName, std::vector<SSVGNode>& defs, SSVGNode& node)
{
	if (bstrStyleID == L"SOLID")
	{
		TColor t = {0.0f, 0.0f, 0.0f, 1.0f};
		swscanf(bstrStyleParams, L"%f,%f,%f,%f", &t.fR, &t.fG, &t.fB, &t.fA);
		int nR = CGammaTables::ToSRGB(t.fR);
		int nG = CGammaTables::ToSRGB(t.fG);
		int nB = CGammaTables::ToSRGB(t.fB);
		//fOpacity = t.fA;
		char sz[32];
		sprintf(sz, "#%02x%02x%02x", nR, nG, nB);
		node.SetAttr(fillName, sz);
		if (t.fA < 1.0f)
			node.SetAttr(fillOpacityName, t.fA);
		return true;
	}

	if (bstrStyleID == L"LINEAR" || bstrStyleID == L"RADIAL")
	{
		SSVGNode grad;

		std::vector<std::pair<float, TColor> > aGradient;
		LPOLESTR psz = bstrStyleParams;
		while (true)
		{
			float f[5];
			int i = 0;
			for (; i < 5; ++i)
			{
				while (*psz == L' ' || *psz == L'\t' || *psz == L'\n' || *psz == L',') ++psz;
				LPOLESTR psz2 = psz;
				f[i] = wcstod(psz, &psz2);
				if (psz2 <= psz) break;
				psz = psz2;
			}
			if (i != 5)
				break;
			std::pair<float, TColor> t;
			t.first = f[0]/65535.0f;
			t.second.fR =  f[1];
			t.second.fG =  f[2];
			t.second.fB =  f[3];
			t.second.fA =  f[4];
			aGradient.push_back(t);
		}
		if (aGradient.size() < 2)
			return false;
		grad.subNodes.reserve(aGradient.size());
		for (std::vector<std::pair<float, TColor> >::const_iterator j = aGradient.begin(); j != aGradient.end(); ++j)
		{
			char sz[8];
			TColor t = j->second;
			if (j->second.fA == 0.0f)
			{
				if (j != aGradient.begin())
					t = (j-1)->second;
				else if (j != aGradient.begin()+(aGradient.size()-1))
					t = (j+1)->second;
			}
			int nR = CGammaTables::ToSRGB(t.fR);
			int nG = CGammaTables::ToSRGB(t.fG);
			int nB = CGammaTables::ToSRGB(t.fB);
			SSVGNode stop;
			stop.type = "stop";
			stop.SetAttr("offset", 100.0f*j->first, "%");
			sprintf(sz, "#%02x%02x%02x", nR, nG, nB);
			stop.SetAttr("stop-color", sz);
			if (j->second.fA < 1.0f)
				stop.SetAttr("stop-opacity", j->second.fA);
			grad.subNodes.resize(grad.subNodes.size()+1);
			stop.swap(grad.subNodes[grad.subNodes.size()-1]);
		}

		bool bAbsolute = wcsncmp(psz, L"ABSOLUTE", 8) == 0;
		while (*psz && *psz != L',') ++psz;
		while (*psz == L' ' || *psz == L'\t' || *psz == L'\n' || *psz == L',') ++psz;
		if (bstrStyleID == L"LINEAR")
		{
			float fX1 = 0.0f;
			float fY1 = 0.0f;
			float fX2 = 1.0f;
			float fY2 = 1.0f;
			swscanf(psz, L"%f,%f,%f,%f", &fX1, &fY1, &fX2, &fY2);

			grad.type = "linearGradient";
			if (bAbsolute)
			{
				grad.SetAttr("gradientUnits", "userSpaceOnUse");
				grad.SetAttr("x1", fX1);
				grad.SetAttr("y1", fY1);
				grad.SetAttr("x2", fX2);
				grad.SetAttr("y2", fY2);
			}
			else
			{
				grad.SetAttr("x1", 100.0f*fX1, "%");
				grad.SetAttr("y1", 100.0f*fY1, "%");
				grad.SetAttr("x2", 100.0f*fX2, "%");
				grad.SetAttr("y2", 100.0f*fY2, "%");
			}
		}
		else if (bstrStyleID == L"RADIAL")
		{
			float fX = 0.5f;
			float fY = 0.5f;
			float fR1 = 0.5f;
			float fR2 = 0.5f;
			float fA = 0.0f;
			int n = swscanf(psz, L"%f,%f,%f,%f,%f", &fX, &fY, &fR1, &fR2, &fA);
			grad.type = "radialGradient";
			if (bAbsolute)
			{
				grad.SetAttr("gradientUnits", "userSpaceOnUse");
				grad.SetAttr("cx", fX);
				grad.SetAttr("cy", fY);
				grad.SetAttr("r", fR1);
				if (n > 3 && fR1 != fR2)
				{
					char sz[256];
					sprintf(sz, "translate(%g,%g) scale(1,%g) rotate(%g) translate(%g,%g)", -fX, -fY, fR2, fA, fX, fY);
					grad.SetAttr("gradientTransform", sz);
				}
			}
			else
			{
				grad.SetAttr("cx", fX*100.0f, "%");
				grad.SetAttr("cy", fY*100.0f, "%");
				grad.SetAttr("r", fR1*100.0f, "%");
				// TODO: transform for relative coords
			}
		}
		int gradIndex = defs.size();
		for (size_t i = 0; i < defs.size(); ++i)
		{
			if (defs[i].EqualsExceptID(grad))
			{
				gradIndex = i;
				break;
			}
		}
		char sz[32];
		if (gradIndex == defs.size())
		{
			sprintf(sz, "G%i", gradIndex);
			grad.id = sz;
			defs.resize(defs.size()+1);
			grad.swap(defs[defs.size()-1]);
		}
		sprintf(sz, "url(#G%i)", gradIndex);
		node.SetAttr(fillName, sz);
		return true;
	}
	return false;
}

void ComputeOutline(TColor tOutlineColor, float fOutlineWidth, float fOutlinePos, EOutlineJoinType eOutlineJoins, std::vector<SSVGNode>& defs, SSVGNode& node)
{
	int nR = CGammaTables::ToSRGB(tOutlineColor.fR);
	int nG = CGammaTables::ToSRGB(tOutlineColor.fG);
	int nB = CGammaTables::ToSRGB(tOutlineColor.fB);
	char sz[10];
	sprintf(sz, "#%02x%02x%02x", nR, nG, nB);
	node.SetAttr("stroke", sz);
	if (fOutlineWidth != 1.0f)
		node.SetAttr("stroke-width", fOutlineWidth);
	if (fOutlinePos != 0.0f)
		node.SetAttr("realworld:stroke-pos", fOutlinePos);
	if (eOutlineJoins == EOJTRound) // EOJTMiter is default
		node.SetAttr("stroke-linejoin", "round");
	else if (eOutlineJoins == EOJTBevel)
		node.SetAttr("stroke-linejoin", "bevel");
	if (tOutlineColor.fA < 1.0f)
		node.SetAttr("stroke-opacity", tOutlineColor.fA);
}

void ComputeDash(BSTR bstrDash, float fWidth, SSVGNode& node)
{
	std::vector<int> aDash;
	wchar_t const* pS = bstrDash;
	bool bInDash = true;
	int nLen = 0;
	while (*pS)
	{
		if ((bInDash && *pS == L' ') || (!bInDash && *pS != L' '))
		{
			aDash.push_back(nLen);
			nLen = 1;
			bInDash = !bInDash;
		}
		else
		{
			++nLen;
		}
		++pS;
	}
	int nOffset = 0;
	if (aDash.empty())
		return;
	aDash.push_back(nLen);
	if (aDash[0] == 0)
	{
		nOffset -= aDash[1];
		if (aDash.size()&1)
			aDash.push_back(aDash[1]);
		else
			aDash[aDash.size()-1] += aDash[1];
		aDash.erase(aDash.begin(), aDash.begin()+2);
	}
	if (aDash.size()&1)
	{
		nOffset += aDash[aDash.size()-1];
		aDash[0] += aDash[aDash.size()-1];
		aDash.resize(aDash.size()-1);
	}
	if (nOffset)
		node.SetAttr("stroke-dashoffset", nOffset*fWidth);
	std::vector<char> buffer;
	for (std::vector<int>::const_iterator k = aDash.begin(); k != aDash.end(); ++k)
	{
		char sz[16];
		sprintf(sz, k == aDash.begin() ? "%g" : " %g", *k*fWidth);
		buffer.insert(buffer.end(), sz, sz+strlen(sz));
	}
	node.SetAttr("stroke-dasharray", &(buffer[0]), buffer.size());
}

void Unescape(std::wstring& a_str)
{
	size_t nLen = a_str.length();
	size_t iD = 0;
	size_t iS = 0;
	while (iS < nLen)
	{
		if (a_str[iS] == L'\\')
			++iS;
		if (iS == nLen)
			break;
		a_str[iD] = a_str[iS];
		++iS;
		++iD;
	}
	if (iD != iS)
		a_str.resize(iD);
}

HRESULT SerializeVectorImage(IDocumentVectorImage* pVI, std::set<std::string>* usedIds, std::vector<SSVGNode>& defs, std::vector<SSVGNode>& nodes)
{
	CComObjectStackEx<CToolWindow> cWindow;

	std::vector<ULONG> aIDs;
	pVI->ObjectIDs(&CEnumToVector<IEnum2UInts, ULONG>(aIDs));
	nodes.reserve(nodes.size()+aIDs.size());
	for (std::vector<ULONG>::const_iterator i = aIDs.begin(); i != aIDs.end(); ++i)
	{
		SSVGNode node;

		ERasterizationMode eRM = ERMSmooth;
		ECoordinatesMode eCM = ECMFloatingPoint;
		BOOL bFill = TRUE;
		BOOL bOutline = FALSE;
		float fOutlineWidth = 1.0f;
		TColor tOutlineColor = {0.0f, 0.0f, 0.0f, 1.0f};
		float fOutlinePos = 0.0f;
		EOutlineJoinType eOutlineJoins = EOJTRound;
		pVI->ObjectStateGet(*i, &bFill, &eRM, &eCM, &bOutline, &tOutlineColor, &fOutlineWidth, &fOutlinePos, &eOutlineJoins);
		CComBSTR bstrStyleID;
		CComBSTR bstrStyleParams;
		pVI->ObjectStyleGet(*i, &bstrStyleID, &bstrStyleParams);

		CComBSTR bstrToolID;
		CComBSTR bstrToolParams;
		pVI->ObjectGet(*i, &bstrToolID, &bstrToolParams);

		if (usedIds)
		{
			CComBSTR bstrName;
			pVI->ObjectNameGet(*i, &bstrName);
			if (bstrName.m_str && bstrName.Length())
			{
				size_t n = bstrName.Length();
				bool valid = (bstrName[0] >= L'A' && bstrName[0] <= L'Z') || (bstrName[0] >= L'a' && bstrName[0] <= L'z');
				for (size_t i = 1; valid && i < n; ++i)
				{
					valid =
						(bstrName[i] >= L'A' && bstrName[i] <= L'Z') ||
						(bstrName[i] >= L'a' && bstrName[i] <= L'z') ||
						(bstrName[i] >= L'0' && bstrName[i] <= L'9') ||
						bstrName[i] == L'-' || bstrName[i] == L'_' || bstrName[i] == L'.' || bstrName[i] == L':';
				}
				if (valid)
				{
					CW2AEX<> str(bstrName.m_str, CP_UTF8);
					std::string strName(static_cast<char const*>(str));
					if (usedIds->find(strName) == usedIds->end())
					{
						usedIds->insert(strName);
						std::swap(node.id, strName);
					}
				}
			}
		}

		if (pVI->ObjectIsEnabled(*i) != S_OK)
			node.SetAttr("visibility", "hidden");

		if (bstrToolID == L"RECTANGLE")
		{
			if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "fill", "fill-opacity", defs, node))
				node.SetAttr("fill", "none");
			if (bOutline && fOutlineWidth > 0.0f)
				ComputeOutline(tOutlineColor, fOutlineWidth, fOutlinePos, eOutlineJoins, defs, node);

			float fRadius = 0.0f;
			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fSizeX = 0.0f;
			float fSizeY = 0.0f;
			float fAngle = 0.0f;
			if (4 == swscanf(bstrToolParams, L"%f,%f,%f,%f,%f,%f", &fRadius, &fCenterX, &fCenterY, &fSizeX, &fSizeY, &fAngle))
				fSizeY = fSizeX;
			node.type = "rect";
			node.SetAttr("x", fCenterX-fSizeX);
			node.SetAttr("y", fCenterY-fSizeY);
			node.SetAttr("width", fSizeX*2.0f);
			node.SetAttr("height", fSizeY*2.0f);
			if (fRadius != 0.0f)
			{
				node.SetAttr("rx", fRadius);
				node.SetAttr("ry", fRadius);
			}
			if (fAngle != 0.0f)
			{
				char sz[256];
				sprintf(sz, "translate(%g,%g) rotate(%g) translate(%g,%g)", fCenterX, fCenterY, fAngle*180/3.14159265359f, -fCenterX, -fCenterY);
				node.SetAttr("transform", sz);
			}
		}
		else if (bstrToolID == L"ELLIPSE")
		{
			if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "fill", "fill-opacity", defs, node))
				node.SetAttr("fill", "none");
			if (bOutline && fOutlineWidth > 0.0f)
				ComputeOutline(tOutlineColor, fOutlineWidth, fOutlinePos, eOutlineJoins, defs, node);

			float fCenterX = 0.0f;
			float fCenterY = 0.0f;
			float fSizeX = 0.0f;
			float fSizeY = 0.0f;
			float fAngle = 0.0f;
			if (3 == swscanf(bstrToolParams, L"%f,%f,%f,%f,%f", &fCenterX, &fCenterY, &fSizeX, &fSizeY, &fAngle))
				fSizeY = fSizeX;
			node.type = "ellipse";
			node.SetAttr("cx", fCenterX);
			node.SetAttr("cy", fCenterY);
			node.SetAttr("rx", fSizeX);
			node.SetAttr("ry", fSizeY);
			if (fAngle != 0.0f)
			{
				char sz[256];
				sprintf(sz, "translate(%g,%g) rotate(%g) translate(%g,%g)", fCenterX, fCenterY, fAngle*180/3.14159265359f, -fCenterX, -fCenterY);
				node.SetAttr("transform", fCenterY-fSizeY);
			}
		}
		else if (bstrToolID == L"TEXT")
		{
			std::wstring strFont;
			bool bBold = false;
			bool bItalic = false;
			bool bDeg = false;
			float fSize = 0.0f;
			float fAngle = 0.0f;
			float fBend = 0.0f;
			char const* eAlign = "start";
			float fPosX = 0.0f;
			float fPosY = 0.0f;
			std::wstring strText;
			rule<scanner<wchar_t*> > cSep = *space_p>>L','>>*space_p;
			bool bParsed = parse(bstrToolParams.m_str, bstrToolParams.m_str+SysStringLen(bstrToolParams), *space_p>>
					confix_p(L'"', (*c_escape_ch_p)[assign_a(strFont)], L'"')>>cSep>>
					//(*negated_char_parser<chlit<wchar_t> >(L','))[assign_a(strFont)]>>cSep>>
					(!((str_p(L"\"BOLD\"")[assign_a(bBold, true)]|str_p(L"\"ITALIC\"")[assign_a(bItalic, true)]|str_p(L"\"BOLDITALIC\"")[assign_a(bBold, true)][assign_a(bItalic, true)]|str_p(L"\"NORMAL\"")) >> cSep))>>
					real_p[assign_a(fSize)]>>cSep>>
					real_p[assign_a(fPosX)]>>cSep>>
					real_p[assign_a(fPosY)]>>cSep>>
					(!(real_p[assign_a(fAngle)]>>cSep>>(!(real_p[assign_a(fBend)]>>cSep))))>>
					confix_p(L'"', (*c_escape_ch_p)[assign_a(strText)], L'"')>>
					(!(cSep >> (str_p(L"\"LEFT\"")[assign_a(eAlign, "start")]|str_p(L"\"CENTER\"")[assign_a(eAlign, "middle")]|str_p(L"\"RIGHT\"")[assign_a(eAlign, "end")]|str_p(L"\"JUSTIFY\"")[assign_a(eAlign, "start")])))
					>>*space_p).full;
			Unescape(strFont);
			Unescape(strText);
			if (bParsed && !strFont.empty() && strFont.length() < LF_FACESIZE && fSize > 0.0f)
			{
				node.type = "text";
				node.SetAttr("x", fPosX);
				node.SetAttr("y", fPosY);
				node.SetAttr("text-anchor", eAlign);
				node.SetAttr("font-family", strFont.c_str());
				node.SetAttr("font-size", fSize/**72.0f/96.0f*/);
				node.SetAttr("dominant-baseline", "text-before-edge");
				if (bBold)
					node.SetAttr("font-weight", "bold");
				if (bItalic)
					node.SetAttr("font-style", "italic");
				if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "fill", "fill-opacity", defs, node))
					node.SetAttr("fill", "none");
				if (bOutline && fOutlineWidth > 0.0f)
					ComputeOutline(tOutlineColor, fOutlineWidth, fOutlinePos, eOutlineJoins, defs, node);
				if (fAngle != 0.0f)
				{
					char sz[256];
					sprintf(sz, "translate(%g,%g) rotate(%g) translate(%g,%g)", fPosX, fPosY, fAngle, -fPosX, -fPosY);
					node.SetAttr("transform", sz);
				}
				CW2AEX<> str(strText.c_str(), CP_UTF8);
				node.value = static_cast<char const*>(str);
			}
		}
		else if (bstrToolID == L"POLYGON")
		{
			if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "fill", "fill-opacity", defs, node))
				node.SetAttr("fill", "none");
			if (bOutline && fOutlineWidth > 0.0f)
				ComputeOutline(tOutlineColor, fOutlineWidth, fOutlinePos, eOutlineJoins, defs, node);

			node.type = "polygon";
			node.SetAttr("fill-rule", "evenodd");
			std::vector<char> aBuffer;
			LPOLESTR psz = bstrToolParams;
			bool bSpace = false;
			while (true)
			{
				LPOLESTR psz2 = psz;
				float fX = wcstod(psz, &psz2);
				if (psz2 <= psz) break;
				psz = psz2;
				while (*psz == L' ' || *psz == L'\t' || *psz == L'\n' || *psz == L',') ++psz;
				psz2 = psz;
				float fY = wcstod(psz, &psz2);
				if (psz2 <= psz) break;
				psz = psz2;
				while (*psz == L' ' || *psz == L'\t' || *psz == L'\n' || *psz == L',') ++psz;
				if (bSpace)
					aBuffer.push_back(' ');
				else
					bSpace = true;
				char sz[32];
				sprintf(sz, "%g,%g", fX, fY);
				aBuffer.insert(aBuffer.end(), sz, sz+strlen(sz));
			}
			node.SetAttr("points", &(aBuffer[0]), aBuffer.size());
		}
		else if (bstrToolID == L"LINE" || bstrToolID == L"CURVE" || bstrToolID == L"STROKE")
		{
			if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "stroke", "stroke-opacity", defs, node))
				node.SetAttr("stroke", "none");
			node.SetAttr("fill", "none");

			CComPtr<IRasterImageEditToolsFactory> pTF;
			RWCoCreateInstance(pTF, __uuidof(RasterImageEditToolsShapes));
			CComPtr<IRasterImageEditTool> pTool;
			pTF->EditToolCreate(NULL, bstrToolID, NULL, &pTool);
			pTool->Init(&cWindow);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(pTool);
			pScripting->FromText(bstrToolParams);
			CComQIPtr<IEditToolLine> pLine(pTool);
			ELineCapMode eCapMode = ELCMRound;
			pLine->CapMode(&eCapMode);
			ELineJoinMode eJoinMode = ELJMRound;
			pLine->JoinMode(&eJoinMode);

			if (eCapMode == ELCMSquare) // ELCMButt is default
				node.SetAttr("stroke-linecap", "square");
			else if (eCapMode == ELCMRound)
				node.SetAttr("stroke-linecap", "round");
			if (eJoinMode == ELJMRound) // ELJMMiter is default
				node.SetAttr("stroke-linejoin", "round");
			else if (eJoinMode == ELJMBevel)
				node.SetAttr("stroke-linejoin", "bevel");

			bool bClosed = pLine->IsClosed() == S_OK;
			float fWidth = 1.0f;
			pLine->Width(&fWidth);
			if (fWidth != 1.0f)
				node.SetAttr("stroke-width", fWidth);

			CComBSTR bstrDash;
			pLine->DashPattern(&bstrDash);
			if (bstrDash.Length() > 1)
				ComputeDash(bstrDash, fWidth, node);

			CComObjectStackEx<CPolygonCallback> cPC;
			cPC.Init(bClosed, node);
			pLine->Polygon(&cPC);
		}
		else if (bstrToolID == L"SHAPE")
		{
			if (!bFill || !ComputeFill(bstrStyleID, bstrStyleParams, "fill", "fill-opacity", defs, node))
				node.SetAttr("fill", "none");
			if (bOutline && fOutlineWidth > 0.0f)
				ComputeOutline(tOutlineColor, fOutlineWidth, fOutlinePos, eOutlineJoins, defs, node);

			CComPtr<IRasterImageEditToolsFactory> pTF;
			RWCoCreateInstance(pTF, __uuidof(RasterImageEditToolsShapes));
			CComPtr<IRasterImageEditTool> pTool;
			pTF->EditToolCreate(NULL, bstrToolID, NULL, &pTool);
			pTool->Init(&cWindow);
			CComQIPtr<IRasterImageEditToolScripting> pScripting(pTool);
			pScripting->FromText(bstrToolParams);
			CComObjectStackEx<CShapeCallback> cSC;
			CComQIPtr<IRasterImageEditToolPolygon> pPolygon(pTool);
			cSC.Init(node);
			pPolygon->ToPath(&cSC);
			node.type = "path";
		}
		else
			continue;

		nodes.resize(nodes.size()+1);
		node.swap(nodes[nodes.size()-1]);
	}

	return S_OK;
}

HRESULT SerializeImage(IDocument* a_pDoc, std::set<std::string>* usedIds, std::vector<SSVGNode>& defs, std::vector<SSVGNode>& nodes);

HRESULT SerializeLayeredImage(IDocumentLayeredImage* a_pLI, std::set<std::string>* usedIds, std::vector<SSVGNode>& defs, std::vector<SSVGNode>& nodes)
{
	CComPtr<IEnumUnknowns> pLayers;
	a_pLI->LayersEnum(NULL, &pLayers);
	ULONG nLayers = 0;
	if (pLayers) pLayers->Size(&nLayers);
	nodes.reserve(nodes.size()+nLayers);
	for (ULONG i = 0; i < nLayers; ++i)
	{
		CComPtr<IComparable> pLayer;
		pLayers->Get(nLayers-1-i, &pLayer);
		CComPtr<ISubDocumentID> pSDID;
		if (pLayer) a_pLI->ItemFeatureGet(pLayer, __uuidof(ISubDocumentID), reinterpret_cast<void**>(&pSDID)); 
		CComPtr<IDocument> pSubDoc;
		if (pSDID) pSDID->SubDocumentGet(&pSubDoc);
		if (pSubDoc)
		{
			std::vector<SSVGNode> subNodes;
			HRESULT hr = SerializeImage(pSubDoc, usedIds, defs, subNodes);
			if (FAILED(hr)) continue;

			SSVGNode node;
			if (subNodes.size() == 1 && (subNodes[0].type == "image" || subNodes[0].type == "g"))
			{
				node.swap(subNodes[0]);
			}
			else
			{
				node.type = "g";
				node.subNodes.swap(subNodes);
			}

			BYTE bVisible = 1;
			ELayerBlend eBlend = EBEAlphaBlend;
			a_pLI->LayerPropsGet(pLayer, &eBlend, &bVisible);
			float fOpacity = 1.0f;
			CComPtr<IConfig> pEffects;
			a_pLI->LayerEffectGet(pLayer, &pEffects, &fOpacity);
			CComBSTR bstrName;
			a_pLI->LayerNameGet(pLayer, &bstrName);

			if (fOpacity < 1.0f)
				node.SetAttr("opacity", fOpacity);
			if (bVisible == 0)
				node.SetAttr("display", "none");
			if (bstrName.Length())
			{
				node.SetAttr("inkscape:groupmode", "layer");
				node.SetAttr("inkscape:label", bstrName.m_str);
			}

			nodes.resize(nodes.size()+1);
			node.swap(nodes[nodes.size()-1]);
		}
	}
	return S_OK;
}

HRESULT SerializeRasterImage(IDocument* a_pDoc, std::set<std::string>* usedIds, IDocumentImage* pDocImg, std::vector<SSVGNode>& nodes)
{
	CComPtr<IDocumentEncoder> pEnc;
	RWCoCreateInstance(pEnc, __uuidof(DocumentEncoderPNG));
	if (pEnc == NULL) return E_INVALIDARG;

	CComPtr<IDocument> pDoc;
	RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
	a_pDoc->DocumentCopy(NULL, CComQIPtr<IDocumentBase>(pDoc), NULL, NULL);
	CComPtr<IDocumentEditableImage> pImg;
	pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pImg));
	if (pImg == NULL) return E_FAIL;
	TImageSize tCanvas = {1, 1};
	TImagePoint tOffset = {0, 0};
	TImageSize tContent = {1, 1};
	pImg->CanvasGet(&tCanvas, NULL, &tOffset, &tContent, NULL);
	if (tCanvas.nX != tContent.nX || tCanvas.nY != tContent.nY || tOffset.nX || tOffset.nY)
	{
		TMatrix3x3f tMtx = {1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  -tOffset.nX, -tOffset.nY, 1.0f};
		pImg->CanvasSet(&tContent, NULL, &tMtx, NULL);
	}

	SSVGNode node;
	node.type = "image";
	node.SetAttr("width", int(tContent.nX));
	node.SetAttr("height", int(tContent.nY));
	if (tOffset.nX || tOffset.nY)
	{
		node.SetAttr("x", int(tOffset.nX));
		node.SetAttr("y", int(tOffset.nY));
	}
	COutputBufferData buffer;
	buffer.Write(22, reinterpret_cast<BYTE const*>("data:image/png;base64,"));
	CComPtr<IConfig> pCfg;
	pEnc->DefaultConfig(&pCfg);
	HRESULT hRes = pEnc->Serialize(pDoc, pCfg, CBase64Encoder(&buffer), NULL, NULL);
	if (FAILED(hRes))
		return hRes;
	node.SetAttr("xlink:href", reinterpret_cast<char const*>(buffer.Data()), buffer.Length());
	nodes.resize(nodes.size()+1);
	node.swap(nodes[nodes.size()-1]);
	return S_OK;
}


HRESULT SerializeImage(IDocument* a_pDoc, std::set<std::string>* usedIds, std::vector<SSVGNode>& defs, std::vector<SSVGNode>& nodes)
{
	CComPtr<IDocumentLayeredImage> pDocLI;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentLayeredImage), reinterpret_cast<void**>(&pDocLI));
	if (pDocLI)
		return SerializeLayeredImage(pDocLI, usedIds, defs, nodes);

	CComPtr<IDocumentVectorImage> pVI;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentVectorImage), reinterpret_cast<void**>(&pVI));
	if (pVI)
		return SerializeVectorImage(pVI, usedIds, defs, nodes);

	CComPtr<IDocumentImage> pI;
	a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pI));
	if (pI)
		return SerializeRasterImage(a_pDoc, usedIds, pI, nodes);

	return E_FAIL;
}

STDMETHODIMP CDocumentEncoderSVG::Serialize(IDocument* a_pDoc, IConfig* UNREF(a_pCfg), IReturnedData* a_pDst, IStorageFilter* a_pLocation, ITaskControl* a_pControl)
{
	try
	{
		CComPtr<IDocumentImage> pDocImg;
		a_pDoc->QueryFeatureInterface(__uuidof(IDocumentImage), reinterpret_cast<void**>(&pDocImg));
		if (pDocImg == NULL) return E_FAIL;
		TImageSize tSize = {1, 1};
		pDocImg->CanvasGet(&tSize, NULL, NULL, NULL, NULL);

		COutputHelper cHelper(a_pDst);
		cHelper << "<?xml version=\"1.0\"?>" << COutputHelper::EOL();
		cHelper << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << COutputHelper::EOL();
		cHelper << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\" xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\" xmlns:realworld=\"http://www.rw-designer.com/export-svg\" width=\"" << int(tSize.nX) << "px\" height=\"" << int(tSize.nY) << "px\">" << COutputHelper::EOL();

		SSVGRootNode root;

		std::set<std::string> usedIds;

		HRESULT hRes = SerializeImage(a_pDoc, &usedIds, root.defs, root.nodes);

		root.Write(cHelper);

		cHelper << "</svg>" << COutputHelper::EOL();

		return hRes;
	}
	catch (...)
	{
		return E_UNEXPECTED;
	}
}

