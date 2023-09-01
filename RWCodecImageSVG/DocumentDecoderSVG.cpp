
#include "stdafx.h"
#include "DocumentTypeSVG.h"
#include <RWDocumentImageRaster.h>
#include <RWDocumentImageVector.h>
#include <MultiLanguageString.h>
#include "rapidxml.hpp"
#include <GammaCorrection.h>
#include <boost/spirit.hpp>
using namespace boost::spirit;


void assign_string(std::string& str, char const* begin, char const* end)
{
	str.assign(begin, end);
}

// - apply transformation to stroke width
// - transform relative gradients
// - text element
// - import dash

extern __declspec(selectany) char const s_tBase64Decode[] =
{
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

// {E818F89B-176C-4f4d-B12C-1A6821771624}
extern GUID const CLSID_DocumentDecoderSVG = {0xe818f89b, 0x176c, 0x4f4d, {0xb1, 0x2c, 0x1a, 0x68, 0x21, 0x77, 0x16, 0x24}};
extern GUID const CLSID_DocumentEncoderSVG;

// CDocumentDecoderSVG

class ATL_NO_VTABLE CDocumentDecoderSVG :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CDocumentDecoderSVG, &CLSID_DocumentDecoderSVG>,
	public CDocumentDecoderImpl<CDocumentDecoderSVG, CDocumentTypeCreatorSVG, IDocumentFactoryLayeredImage>
{
public:
	CDocumentDecoderSVG()
	{
	}

DECLARE_NO_REGISTRY()
DECLARE_CLASSFACTORY_SINGLETON(CDocumentDecoderSVG)

BEGIN_CATEGORY_MAP(CDocumentDecoderSVG)
	IMPLEMENTED_CATEGORY(CATID_DocumentDecoder)
END_CATEGORY_MAP()

BEGIN_COM_MAP(CDocumentDecoderSVG)
	COM_INTERFACE_ENTRY(IDocumentDecoder)
END_COM_MAP()


public:
	HRESULT Parse(ULONG a_nLen, BYTE const* a_pData, IStorageFilter* a_pLocation, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, GUID* a_pEncoderID, IConfig** a_ppEncoderCfg, ITaskControl* a_pControl)
	{
		if (a_nLen < 9)
			return E_RW_UNKNOWNINPUTFORMAT;
		{
			BYTE const* p = a_pData;
			BYTE const* const pEnd = a_pData+min(a_nLen-5, 4096);
			while (p < pEnd && (p[0] != 's' || p[1] != 'v' || p[2] != 'g'))
				++p;
			if (p == pEnd)
				return E_RW_UNKNOWNINPUTFORMAT; // may produce false negative if svg file starts with a long comment
		}
		CAutoVectorPtr<char> data(new char[a_nLen+1]);
		CopyMemory(data.m_p, a_pData, a_nLen);
		data[a_nLen] = '\0';
		rapidxml::xml_document<> doc;
		try
		{
			doc.parse<0>(data.m_p);
		}
		catch (...)
		{
			return E_RW_UNKNOWNINPUTFORMAT;
		}
		rapidxml::xml_node<>* svg = doc.first_node("svg");
		if (svg == NULL)
			return E_RW_UNKNOWNINPUTFORMAT;

		TImageSize tSize = {256, 256};
		TImagePoint tOff = {0, 0};
		{
			int width = getIntAttr(svg, "width", 0);
			int height = getIntAttr(svg, "height", 0);
			rapidxml::xml_attribute<>* viewBox = svg->first_attribute("viewBox");
			float vbx0 = 0;
			float vby0 = 0;
			float vbx1 = 0;
			float vby1 = 0;
			bool vb = false;
			if (viewBox)
				vb = 4 == sscanf(viewBox->value(), "%g%g%g%g", &vbx0, &vby0, &vbx1, &vby1);
			if (vb && vbx1 > 0 && vby1 > 0)
			{
				tSize.nX = (float)(vbx1+0.5f);
				tSize.nY = (float)(vby1+0.5f);
				tOff.nX = vbx0;
				tOff.nY = vby0;
			}
			else if (width > 0 && height > 0)
			{
				tSize.nX = width;
				tSize.nY = height;
			}
			else if (width > 0)
				tSize.nX = tSize.nY = width;
			else if (height > 0)
				tSize.nX = tSize.nY = height;
		}
		rapidxml::xml_node<>* style = svg->first_node("style");
		std::map<std::string, std::string> classes;
		if (style)
			ParseCSS(style, classes);
		a_pBuilder->Create(a_bstrPrefix, a_pBase);
		TImageResolution tRes = {100, 254, 100, 254};
		CNodeTree tree;
		{
			SSVGStyle defStyle;
			defStyle.xform._31 = -tOff.nX;
			defStyle.xform._32 = -tOff.nY;
			CComPtr<IBezierPathUtils> pUtils;
			RWCoCreateInstance(pUtils, __uuidof(RasterImageEditToolsShapes));
			addNode(defStyle, classes, tSize, tree, pUtils, svg);
		}
		tree.simplify();
		{
			tree.toDoc(a_pBuilder, a_bstrPrefix, a_pBase, tSize, &tRes, 2.2f, CPixelChannel(0, 0, 0, 0));
		}

		a_pBuilder->SetSize(a_bstrPrefix, a_pBase, &tSize);

		if (a_pEncoderID)
			*a_pEncoderID = CLSID_DocumentEncoderSVG;
		//a_pBuilder->SetResolution(a_bstrPrefix, a_pBase, NULL);
		return S_OK;
	}

private:
	static int getInt(rapidxml::xml_attribute<>* attr, int defVal)
	{
		if (attr == NULL)
			return defVal;
		char* p = attr->value();
		if (p[0] == '\"')
			++p;
		int v = defVal;
		if (1 == sscanf(p, "%i", &v))
			return v;
		return defVal;
	}
	static bool parseLength(char const* p, float* val)
	{
		char* end = NULL;
		*val = strtod(p, &end);
		if (end <= p)
			return false;
		if (*end == 'c' && end[1] == 'm')
			*val *= 100.0f/2.54f; // 100 dpi
		if (*end == '%' && end[1] == '\0')
			*val /= 100.0f;
		return true;
	}
	static float getFloat(rapidxml::xml_attribute<>* attr, float defVal)
	{
		if (attr == NULL)
			return defVal;
		char* p = attr->value();
		if (p[0] == '\"')
			++p;
		float v = defVal;
		parseLength(p, &v);
		return v;
	}

	static int getIntAttr(rapidxml::xml_node<>* node, char const* name, int defVal)
	{
		if (node == NULL || name == NULL)
			return defVal;
		return getInt(node->first_attribute(name), defVal);
	}
	static float getFloatAttr(rapidxml::xml_node<>* node, char const* name, float defVal)
	{
		if (node == NULL || name == NULL)
			return defVal;
		return getFloat(node->first_attribute(name), defVal);
	}
	static rapidxml::xml_node<>* findNodeByID(rapidxml::xml_node<>* node, char const* name)
	{
		for (rapidxml::xml_node<>* sub = node->first_node(); sub; sub = sub->next_sibling())
		{
			rapidxml::xml_attribute<>* id = sub->first_attribute("id");
			if (id != NULL && strcmp(id->value(), name) == 0)
				return sub;
			rapidxml::xml_node<>* tmp = findNodeByID(sub, name);
			if (tmp)
				return tmp;
		}
		return NULL;
	}

	struct translate
	{
	public:
		translate(float const& x, float const& y, TMatrix3x3f& xform) :
			x(x), y(y), xform(xform)
		{
		}

		template<class IteratorT>
		void operator()(IteratorT, IteratorT) const
		{
			TMatrix3x3f const trn = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, x, y, 1.0f};
			TMatrix3x3f prev = xform;
			Matrix3x3fMultiply(trn, prev, &xform);

			//xform._31 += x;
			//xform._32 += y;

			//xform._31 += x*xform._11 + y*xform._21;
			//xform._32 += x*xform._12 + y*xform._22;
			//xform._33 += x*xform._13 + y*xform._23;
		}

	private:
		float const& x;
		float const& y;
		TMatrix3x3f& xform;
	};
	struct scale
	{
	public:
		scale(float const& x, float const& y, TMatrix3x3f& xform) :
			x(x), y(y), xform(xform)
		{
		}

		template<class IteratorT>
		void operator()(IteratorT, IteratorT) const
		{
			TMatrix3x3f const scl = {x, 0.0f, 0.0f, 0.0f, y, 0.0f, 0.0f, 0.0f, 1.0f};
			TMatrix3x3f prev = xform;
			Matrix3x3fMultiply(scl, prev, &xform);
			//xform._11 *= x;
			//xform._21 *= x;
			//xform._31 *= x;
			//xform._12 *= y;
			//xform._22 *= y;
			//xform._32 *= y;

			//xform._11 *= x;
			//xform._12 *= x;
			//xform._13 *= x;
			//xform._21 *= y;
			//xform._22 *= y;
			//xform._23 *= y;
		}

	private:
		float const& x;
		float const& y;
		TMatrix3x3f& xform;
	};
	struct skew
	{
	public:
		skew(float const& x, float const& y, TMatrix3x3f& xform) :
			x(x), y(y), xform(xform)
		{
		}

		template<class IteratorT>
		void operator()(IteratorT, IteratorT) const
		{
			float tx = tan(3.141592653f/180.0f*x);
			float ty = tan(3.141592653f/180.0f*y);
			TMatrix3x3f const skew = {1, ty, 0.0f, tx, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
			TMatrix3x3f prev = xform;
			Matrix3x3fMultiply(skew, prev, &xform);
			//xform._11 *= x;
			//xform._12 *= x;
			//xform._13 *= x;
			//xform._21 *= y;
			//xform._22 *= y;
			//xform._23 *= y;
		}

	private:
		float const& x;
		float const& y;
		TMatrix3x3f& xform;
	};
	struct rotate
	{
	public:
		rotate(float const& angle, float const& centerx, float const& centery, TMatrix3x3f& xform) :
			angle(angle), xform(xform), centerx(centerx), centery(centery)
		{
		}

		template<class IteratorT>
		void operator()(IteratorT, IteratorT) const
		{
			float const s = sinf(angle*3.14159265358979/180.0f);
			float const c = cosf(angle*3.14159265358979/180.0f);
			TMatrix3x3f rot = {c, s, 0.0f, -s, c, 0.0f, centerx*(1-c)+centery*s, centery*(1-c)-centerx*s, 1.0f};
			TMatrix3x3f prev = xform;
			Matrix3x3fMultiply(rot, prev, &xform);
		}

	private:
		float const& angle;
		float const& centerx;
		float const& centery;
		TMatrix3x3f& xform;
	};
	struct multiply
	{
	public:
		multiply(TMatrix3x3f const& matrix, TMatrix3x3f& xform) :
			matrix(matrix), xform(xform)
		{
		}

		template<class IteratorT>
		void operator()(IteratorT, IteratorT) const
		{
			TMatrix3x3f prev = xform;
			Matrix3x3fMultiply(prev, matrix, &xform);
		}

	private:
		TMatrix3x3f const& matrix;
		TMatrix3x3f& xform;
	};
	struct SSVGStyle
	{
		SSVGStyle() : xform(TMATRIX3X3F_IDENTITY), fillOpacity(1.0f), strokeOpacity(1.0f), strokeWidth(1.0f), strokePos(0.0f),
			opacity(1.0f), strokeCap("butt"), strokeJoin("miter"), fill("#000"), stroke("none"), displayNone(false), dashOffset(0.0f),
			fontSize(10.0f)
		{
		}
		SSVGStyle(SSVGStyle const& rhs) : xform(rhs.xform), fillOpacity(rhs.fillOpacity), fill(rhs.fill),
			strokeOpacity(rhs.strokeOpacity), strokeWidth(rhs.strokeWidth), strokePos(rhs.strokePos), stroke(rhs.stroke),
			opacity(rhs.opacity), strokeCap(rhs.strokeCap), strokeJoin(rhs.strokeJoin), displayNone(rhs.displayNone),
			dashArray(rhs.dashArray), dashOffset(rhs.dashOffset),
			fontSize(rhs.fontSize), fontFamily(rhs.fontFamily), fontWeight(rhs.fontWeight), fontStyle(rhs.fontStyle)
		{
		}

		static bool ParseTransform(char const* psz, size_t len, TMatrix3x3f& xform)
		{
			float transx = 0.0f;
			float transy = 0.0f;
			float scalex = 1.0f;
			float scaley = 1.0f;
			float angle = 0.0f;
			float rotx = 0.0f;
			float roty = 0.0f;
			float zero = 0.0f;
			float skewx = 0.0f;
			float skewy = 0.0f;
			TMatrix3x3f matrix = TMATRIX3X3F_IDENTITY;
			TMatrix3x3f composed = TMATRIX3X3F_IDENTITY;
			rule<scanner<char const*> > comma = +(space_p|ch_p(','));// optional comma // *space_p>>','>>*space_p;
			rule<scanner<char const*> > parl = *space_p>>'('>>*space_p;
			rule<scanner<char const*> > parr = *space_p>>')'>>*space_p;
			bool bParsed = parse(psz, psz+len, *(
					(*space_p>>(str_p("translate")>>parl>>real_p[assign_a(transx)]>>comma>>real_p[assign_a(transy)]>>parr)[translate(transx, transy, composed)])|
					(*space_p>>(str_p("translate")>>parl>>real_p[assign_a(transx)]>>parr)[translate(transx, zero, composed)])|
					(*space_p>>(str_p("skewX")>>parl>>real_p[assign_a(skewx)]>>parr)[skew(skewx, zero, composed)])|
					(*space_p>>(str_p("skewY")>>parl>>real_p[assign_a(skewy)]>>parr)[skew(zero, skewy, composed)])|
					(*space_p>>(str_p("scale")>>parl>>real_p[assign_a(scalex)]>>comma>>real_p[assign_a(scaley)]>>parr)[scale(scalex, scaley, composed)])|
					(*space_p>>(str_p("scale")>>parl>>real_p[assign_a(scalex)]>>parr)[scale(scalex, scalex, composed)])|
					(*space_p>>(str_p("rotate")>>parl>>real_p[assign_a(angle)]>>parr)[rotate(angle, zero, zero, composed)])|
					(*space_p>>(str_p("rotate")>>parl>>real_p[assign_a(angle)]>>comma>>real_p[assign_a(rotx)]>>comma>>real_p[assign_a(roty)]>>parr)[rotate(angle, rotx, roty, composed)])|
					(*space_p>>(str_p("matrix")>>parl>>real_p[assign_a(matrix._11)]>>comma>>real_p[assign_a(matrix._12)]>>comma>>real_p[assign_a(matrix._21)]>>comma>>real_p[assign_a(matrix._22)]>>comma>>real_p[assign_a(matrix._31)]>>comma>>real_p[assign_a(matrix._32)]>>parr)[multiply(matrix, composed)])
					)).full;
			if (bParsed)
			{
				TMatrix3x3f prev = xform;
				Matrix3x3fMultiply(composed, prev, &xform);
			}
			return bParsed;
		}

		void MergeCSS(char const* pBeg, char const* pEnd)
		{
			rule<scanner<char const*> > comma = *space_p>>','>>*space_p;
			rule<scanner<char const*> > parl = *space_p>>'('>>*space_p;
			rule<scanner<char const*> > parr = *space_p>>')'>>*space_p;
			rule<scanner<char const*> > colon = *space_p>>':'>>*space_p;

			rule<scanner<char const*> > ident = (alpha_p|ch_p('_'))>>*(alnum_p|ch_p('_'));
			rule<scanner<char const*> > url =
				str_p("url")>>parl>>
				*ch_p('\'')>>*ch_p('\"')>>ch_p('#')>>ident>>*ch_p('\"')>>*ch_p('\'')>>
				parr;
			float swScale = 1.0f;
			rule<scanner<char const*> > item = *space_p>>(
				(str_p("fill-opacity")>>colon>>real_p[assign_a(fillOpacity)])|
				(str_p("fill")>>colon>>((ch_p('#')>>*xdigit_p)|(str_p("rgb")>>parl>>int_p>>comma>>int_p>>comma>>int_p>>parr)|url|(*alpha_p))[assign_a(fill)])|
				(str_p("stroke-opacity")>>colon>>real_p[assign_a(strokeOpacity)])|
				(str_p("stroke")>>colon>>((ch_p('#')>>*xdigit_p)|(str_p("rgb")>>parl>>int_p>>comma>>int_p>>comma>>int_p>>parr)|url|(*alpha_p))[assign_a(stroke)])|
				(str_p("stroke-width")>>colon>>real_p[assign_a(strokeWidth)]>>*(str_p("cm")[assign_a(swScale, 100.0f/2.54f)]|str_p("px")))|
				(str_p("opacity")>>colon>>real_p[assign_a(opacity)])|
				(str_p("display")>>colon>>str_p("none")[assign_a(displayNone, true)])|
				(str_p("filter")>>colon>>url[assign_a(filter)])|
				((+(alpha_p|'-'))>>colon>>(*(~ch_p(';')))) // unknown attribute
				)>>*space_p;
			bool bParsed = parse(pBeg, pEnd, list_p(item, ch_p(';'))).full;
			strokeWidth *= swScale;
		}
		void MergeStyle(rapidxml::xml_node<>* node)
		{
			rapidxml::xml_attribute<>* at = node->first_attribute("transform");
			if (at)
				ParseTransform(at->value(), at->value_size(), xform);

			displayNone = false;
			filter.clear();
			at = node->first_attribute("style");
			if (at)
			{
				char const* pBeg = at->value();
				char const* pEnd = at->value()+at->value_size();
				MergeCSS(pBeg, pEnd);
			}

			at = node->first_attribute("fill");
			if (at)
				assign_string(fill, at->value(), at->value()+at->value_size());
			at = node->first_attribute("fill-opacity");
			if (at)
				sscanf(at->value(), "%g", &fillOpacity);
			at = node->first_attribute("stroke");
			if (at)
				assign_string(stroke, at->value(), at->value()+at->value_size());
			at = node->first_attribute("stroke-opacity");
			if (at)
				sscanf(at->value(), "%g", &strokeOpacity);
			at = node->first_attribute("opacity");
			if (at)
				sscanf(at->value(), "%g", &opacity);
			at = node->first_attribute("visibility");
			if (at && strcmp(at->value(), "hidden") == 0)
				displayNone = true;
			at = node->first_attribute("display");
			if (at && strcmp(at->value(), "none") == 0)
				displayNone = true;
			at = node->first_attribute("stroke-width");
			if (at)
				parseLength(at->value(), &strokeWidth);
			at = node->first_attribute("realworld:stroke-pos");
			if (at)
				parseLength(at->value(), &strokePos);
			at = node->first_attribute("stroke-linejoin");
			if (at)
				assign_string(strokeJoin, at->value(), at->value()+at->value_size());
			at = node->first_attribute("stroke-linecap");
			if (at)
				assign_string(strokeCap, at->value(), at->value()+at->value_size());
			at = node->first_attribute("stroke-dasharray");
			if (at)
			{
				dashArray.clear();
				float f;
				rule<scanner<char const*> > item = *space_p>>list_p(real_p[assign_a(f)][push_back_a(dashArray, f)], ch_p(',')|space_p);
				char const* pBeg = at->value();
				char const* pEnd = at->value()+at->value_size();
				bool bParsed = parse(pBeg, pEnd, item).full;
				if (!bParsed)
					dashArray.clear();
			}
			at = node->first_attribute("stroke-dashoffset");
			if (at)
				parseLength(at->value(), &dashOffset);
			at = node->first_attribute("font-size");
			if (at)
				parseLength(at->value(), &fontSize);
			at = node->first_attribute("font-family");
			if (at)
				assign_string(fontFamily, at->value(), at->value()+at->value_size());
			at = node->first_attribute("font-weight");
			if (at)
				assign_string(fontWeight, at->value(), at->value()+at->value_size());
			at = node->first_attribute("font-style");
			if (at)
				assign_string(fontStyle, at->value(), at->value()+at->value_size());
		}

		static bool GetSolidColor(char const* psz, TColor* solidColor)
		{
			size_t const len = strlen(psz);
			if ((len == 7 || len == 4) && psz[0] == '#')
			{
				int val = 0;
				if (1 == sscanf(psz+1, "%x", &val))
				{
					if (len == 4)
						val = ((val&0xf00)<<12)|((val&0xf00)<<8)|((val&0xf0)<<8)|((val&0xf0)<<4)|((val&0xf)<<4)|(val&0xf);
					solidColor->fB = CGammaTables::FromSRGB(val&0xff);
					solidColor->fG = CGammaTables::FromSRGB((val>>8)&0xff);
					solidColor->fR = CGammaTables::FromSRGB((val>>16)&0xff);
					solidColor->fA = 1.0f;
					return true;
				}
			}
			else if (len >= 10 && psz[0] == 'r' && psz[1] == 'g' && psz[2] == 'b')
			{
				int ir = 0;
				int ig = 0;
				int ib = 0;
				if (3 == sscanf(psz+3, "(%i,%i,%i)", &ir, &ig, &ib))
				{
					solidColor->fB = CGammaTables::FromSRGB(ib);
					solidColor->fG = CGammaTables::FromSRGB(ig);
					solidColor->fR = CGammaTables::FromSRGB(ir);
					solidColor->fA = 1.0f;
					return true;
				}
			}
			DWORD code = FromNamedColor(psz);
			if (code != NONAMEDCOLOR)
			{
				solidColor->fB = CGammaTables::FromSRGB(code&0xff);
				solidColor->fG = CGammaTables::FromSRGB((code>>8)&0xff);
				solidColor->fR = CGammaTables::FromSRGB((code>>16)&0xff);
				solidColor->fA = 1.0f;
				return true;
			}
			return false;
		}
		bool GetFill(CComBSTR& bstrFill, CComBSTR& bstrFillParam, rapidxml::xml_node<>* doc)
		{
			TColor solidColor;
			if (fill.compare("none") == 0)
				return false;
			else if (GetSolidColor(fill.c_str(), &solidColor))
			{
				wchar_t sz[64];
				swprintf(sz, L"%g,%g,%g,%g", solidColor.fR, solidColor.fG, solidColor.fB, solidColor.fA*fillOpacity*opacity);
				bstrFillParam = sz;
				bstrFill = L"SOLID";
				return true;
			}
			std::string url;
			rule<scanner<char const*> > ident = (alpha_p|ch_p('_'))>>*(alnum_p|ch_p('_'));
			bool bParsed = parse(fill.c_str(), fill.c_str()+fill.length(),
				*space_p>>str_p("url")>>*space_p>>ch_p('(')>>*space_p>>
				*ch_p('\'')>>*ch_p('\"')>>ch_p('#')>>ident[assign_a(url)]>>*ch_p('\"')>>*ch_p('\'')>>
				*space_p>>ch_p(')')>>*space_p).full;
			if (!url.empty())
			{
				if (GetRefUrl(bstrFill, bstrFillParam, url.c_str(), doc, fillOpacity*opacity))
					return true;
			}
			bstrFill = L"SOLID";
			bstrFillParam = L"0,0,0,1";
			return true;
		}
		struct SGradientData
		{
			SGradientData () :
				x1(0.0f), y1(0.0f), x2(1.0f), y2(0.0f),
				cx(0.5f), cy(0.5f), r(0.5f), fx(0.5f), fy(0.5f),
				absolute(false), gxf(TMATRIX3X3F_IDENTITY) {}
			// linear
			float x1;
			float y1;
			float x2;
			float y2;
			// radial
			float cx;
			float cy;
			float fx;
			float fy;
			float r;
			// shared
			bool absolute;
			TMatrix3x3f gxf;
			std::map<WORD, TColor> stops;
		};
		void GetGradientData(rapidxml::xml_node<>* node, rapidxml::xml_node<>* doc, SGradientData& data)
		{
			rapidxml::xml_attribute<>* href = node->first_attribute("xlink:href");
			if (href && href->value()[0] == '#')
			{
				rapidxml::xml_node<>* sub = findNodeByID(doc, href->value()+1);
				if (sub)
					GetGradientData(sub, doc, data);
			}

			rapidxml::xml_attribute<>* units = node->first_attribute("gradientUnits");
			if (units)
				data.absolute = strcmp(units->value(), "userSpaceOnUse") == 0;
			data.x1 = getFloatAttr(node, "x1", data.x1);
			data.y1 = getFloatAttr(node, "y1", data.y1);
			data.x2 = getFloatAttr(node, "x2", data.x2);
			data.y2 = getFloatAttr(node, "y2", data.y2);

			data.cx = getFloatAttr(node, "cx", data.cx);
			data.cy = getFloatAttr(node, "cy", data.cy);
			data.r = getFloatAttr(node, "r", data.r);
			data.fx = getFloatAttr(node, "fx", data.fx);
			data.fy = getFloatAttr(node, "fy", data.fy);

			rapidxml::xml_attribute<>* trans = node->first_attribute("gradientTransform");
			if (trans)
				ParseTransform(trans->value(), trans->value_size(), data.gxf);

			TColor solidColor = {0, 0, 0, 1};
			for (rapidxml::xml_node<>* stop = node->first_node("stop"); stop; stop = stop->next_sibling("stop"))
			{
				float stopOpacity = 1.0f;
				std::string stopColor("#000");

				rapidxml::xml_attribute<>* at = stop->first_attribute("style");
				if (at)
				{
					char const* pBeg = at->value();
					char const* pEnd = at->value()+at->value_size();
					rule<scanner<char const*> > comma = *space_p>>','>>*space_p;
					rule<scanner<char const*> > parl = *space_p>>'('>>*space_p;
					rule<scanner<char const*> > parr = *space_p>>')'>>*space_p;
					rule<scanner<char const*> > colon = *space_p>>':'>>*space_p;
					rule<scanner<char const*> > item = *space_p>>(
						(str_p("stop-opacity")>>colon>>real_p[assign_a(stopOpacity)])|
						(str_p("stop-color")>>colon>>((ch_p('#')>>*xdigit_p)|(str_p("rgb")>>parl>>int_p>>comma>>int_p>>comma>>int_p>>parr)|(*alpha_p))[assign_a(stopColor)])|
						((+(alpha_p|'-'))>>colon>>(*(~ch_p(';')))) // unknown attribute
						)>>*space_p;
					bool bParsed = parse(pBeg, pEnd, list_p(item, ch_p(';'))).full;
				}

				rapidxml::xml_attribute<>* color = stop->first_attribute("stop-color");
				if (color)
					assign_string(stopColor, color->value(), color->value()+color->value_size());
				stopOpacity = getFloatAttr(stop, "stop-opacity", stopOpacity);
				if (!stopColor.empty())
					GetSolidColor(stopColor.c_str(), &solidColor);

				float off = 0;
				rapidxml::xml_attribute<>* offset = stop->first_attribute("offset");
				if (offset)
				{
					sscanf(offset->value(), "%g", &off); // assuming %
					if (offset->value_size() > 1 && offset->value()[offset->value_size()-1] == '%')
						off /= 100.0f;
				}
				WORD st = off >= 0.0f && off <= 1.0f ? 65535*off : 0;
				solidColor.fA *= stopOpacity;
				data.stops[st] = solidColor;
			}
		}
		bool GetRefUrl(CComBSTR& bstrFill, CComBSTR& bstrFillParam, char const* name, rapidxml::xml_node<>* doc, float opacity)
		{
			rapidxml::xml_node<>* node = findNodeByID(doc, name);
			if (node == NULL)
				return false;
			SGradientData data;
			if (strcmp(node->name(), "linearGradient") == 0)
			{
				GetGradientData(node, doc, data);
				TMatrix3x3f gxf;
				if (data.absolute)
					Matrix3x3fMultiply(data.gxf, xform, &gxf);
				else
					gxf = data.gxf;
				TransformFloats(gxf, data.x1, data.y1);
				TransformFloats(gxf, data.x2, data.y2);

				if (data.stops.empty())
				{
					TColor solidColor = {0, 0, 0, 1};
					data.stops[0] = solidColor;
					solidColor.fR = solidColor.fG = solidColor.fB = 1.0f;
					data.stops[0xffff] = solidColor;
				}
				if (data.stops.find(0) == data.stops.end())
					data.stops[0] = data.stops.begin()->second;
				if (data.stops.find(0xffff) == data.stops.end())
					data.stops[0xffff] = data.stops.rbegin()->second;

				wchar_t sz[80];
				for (std::map<WORD, TColor>::const_iterator i = data.stops.begin(); i != data.stops.end(); ++i)
				{
					swprintf(sz, L"%i,%g,%g,%g,%g,", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA*opacity);
					bstrFillParam += sz;
				}
				swprintf(sz, L"\"%s\",%g,%g,%g,%g", data.absolute ? L"ABSOLUTE" : L"RELSHAPE", data.x1, data.y1, data.x2, data.y2);
				bstrFillParam += sz;
				bstrFill = L"LINEAR";
				return true;
			}
			else if (strcmp(node->name(), "radialGradient") == 0)
			{
				GetGradientData(node, doc, data);
				TMatrix3x3f gxf;
				if (data.absolute)
					Matrix3x3fMultiply(data.gxf, xform, &gxf);
				else
					gxf = data.gxf;
				TransformFloats(gxf, data.cx, data.cy);
				float rx, ry, angle;
				{
					TPixelCoords sx = {data.r, 0.0f};
					TPixelCoords dx;
					TransformDirection(gxf, sx, &dx);
					rx = sqrtf(dx.fX*dx.fX + dx.fY*dx.fY);
					angle = atan2f(dx.fY, dx.fX);
					TPixelCoords sy = {0.0f, data.r};
					TPixelCoords dy;
					TransformDirection(gxf, sy, &dy);
					ry = sqrtf(dy.fX*dy.fX + dy.fY*dy.fY);
				}

				if (data.stops.empty())
				{
					TColor solidColor = {0, 0, 0, 1};
					data.stops[0] = solidColor;
					solidColor.fR = solidColor.fG = solidColor.fB = 1.0f;
					data.stops[0xffff] = solidColor;
				}
				if (data.stops.find(0) == data.stops.end())
					data.stops[0] = data.stops.begin()->second;
				if (data.stops.find(0xffff) == data.stops.end())
					data.stops[0xffff] = data.stops.rbegin()->second;

				wchar_t sz[80];
				for (std::map<WORD, TColor>::const_iterator i = data.stops.begin(); i != data.stops.end(); ++i)
				{
					swprintf(sz, L"%i,%g,%g,%g,%g,", int(i->first), i->second.fR, i->second.fG, i->second.fB, i->second.fA*opacity);
					bstrFillParam += sz;
				}
				if (rx != ry)
					swprintf(sz, L"\"%s\",%g,%g,%g,%g,%g", data.absolute ? L"ABSOLUTE" : L"RELSHAPE", data.cx, data.cy, rx, ry/rx, angle);
				else
					swprintf(sz, L"\"%s\",%g,%g,%g", data.absolute ? L"ABSOLUTE" : L"RELSHAPE", data.cx, data.cy, rx);
				bstrFillParam += sz;
				bstrFill = L"RADIAL";
				return true;
			}
			return false;
		}

		bool GetStroke(CComBSTR& bstrFill, CComBSTR& bstrFillParam, TColor& solidColor, float& width, rapidxml::xml_node<>* doc)
		{
			if (stroke.compare("none") == 0)
				return false;

			width = memcmp(&xform, &TMATRIX3X3F_IDENTITY, sizeof(xform)) == 0 ? strokeWidth :
				strokeWidth*0.5f*(sqrtf(xform._11*xform._11+xform._12*xform._12)+sqrtf(xform._21*xform._21+xform._22*xform._22));

			if (GetSolidColor(stroke.c_str(), &solidColor))
			{
				solidColor.fA *= strokeOpacity*opacity;
				wchar_t sz[64];
				swprintf(sz, L"%g,%g,%g,%g", solidColor.fR, solidColor.fG, solidColor.fB, solidColor.fA);
				bstrFillParam = sz;
				bstrFill = L"SOLID";
				return true;
			}
			std::string url;
			rule<scanner<char const*> > ident = (alpha_p|ch_p('_'))>>*(alnum_p|ch_p('_'));
			bool bParsed = parse(stroke.c_str(), stroke.c_str()+stroke.length(),
				*space_p>>str_p("url")>>*space_p>>ch_p('(')>>*space_p>>
				*ch_p('\'')>>*ch_p('\"')>>ch_p('#')>>ident[assign_a(url)]>>*ch_p('\"')>>*ch_p('\'')>>
				*space_p>>ch_p(')')>>*space_p).full;
			if (!url.empty())
			{
				if (GetRefUrl(bstrFill, bstrFillParam, url.c_str(), doc, strokeOpacity*opacity))
					return true;
			}
			bstrFill = L"SOLID";
			bstrFillParam = L"0,0,0,1";
			return true;
		}

		void GetDash(CComBSTR& dst)
		{
			if (dashArray.empty())
				return;
			int total = 0;
			std::vector<int> copy;
			for (std::vector<float>::iterator i = dashArray.begin(); i != dashArray.end(); ++i)
			{
				int n = (int)(*i/strokeWidth+0.5f);
				if (n < 1) n = 1;
				total += n;
				copy.push_back(n);
			}
			if (copy.size()&1)
			{
				size_t s = copy.size();
				copy.resize(s*2);
				std::copy(copy.begin(), copy.begin()+s, copy.begin()+s);
				total *= 2;
			}
			dst.Attach(SysAllocStringLen(NULL, 4+total));
			wchar_t* p = dst;
			*p = L'\"';
			++p;
			wchar_t c = L'-';
			for (std::vector<int>::const_iterator i = copy.begin(); i != copy.end(); ++i)
			{
				for (int j = 0; j < *i; ++j, ++p)
					*p = c;
				c ^= L'-'^L' ';
			}
			int shift = (int)(dashOffset/strokeWidth+0.5f);
			shift = (total + (shift%total)) % total; // positive remainder
			if (shift != 0)
				std::rotate(dst.m_str+1, dst.m_str+1+shift, p);
			*p = L'\"';
			++p;
			*p = L',';
			++p;
			*p = L' ';
		}

		void GetFilter(IConfig** a_ppEffect, rapidxml::xml_node<>* doc)
		{
			if (filter.empty())
				return;
			std::string url;
			rule<scanner<char const*> > ident = (alpha_p|ch_p('_'))>>*(alnum_p|ch_p('_'));
			bool bParsed = parse(filter.c_str(), filter.c_str()+filter.length(),
				*space_p>>str_p("url")>>*space_p>>ch_p('(')>>*space_p>>
				*ch_p('\'')>>*ch_p('\"')>>ch_p('#')>>ident[assign_a(url)]>>*ch_p('\"')>>*ch_p('\'')>>
				*space_p>>ch_p(')')>>*space_p).full;
			if (!bParsed)
				return;
			rapidxml::xml_node<>* node = findNodeByID(doc, url.c_str());
			if (node == NULL)
				return;
			SGradientData data;
			if (strcmp(node->name(), "filter") != 0)
				return;
			rapidxml::xml_node<>* effect = node->first_node();
			while (effect)
			{
				if (strcmp(effect->name(), "feGaussianBlur") == 0)
				{
					float stdDev = getFloatAttr(effect, "stdDeviation", 0.0f);
					if (stdDev > 0.0f)
					{
						CComPtr<IConfigInMemory> pMemCfg;
						RWCoCreateInstance(pMemCfg, __uuidof(ConfigInMemory));
						CComBSTR bstr1(L"Effect");
						CComBSTR bstr2(L"Effect\\Direction");
						CComBSTR bstr3(L"Effect\\Radius");
						static CLSID const CLSID_DocumentOperationRasterImageBlur = {0x4DD76D14, 0x45A2, 0x4FE5, {0x98, 0x70, 0x0D, 0xC9, 0x4A, 0x53, 0xDF, 0x75}};
						CConfigValue val1(CLSID_DocumentOperationRasterImageBlur);
						CConfigValue val2(0L);// bidirectional
						CConfigValue val3(stdDev*2.0f);// radius
						BSTR aIDs[] = {bstr1, bstr2, bstr3};
						TConfigValue aVals[] = {val1, val2, val3};
						pMemCfg->ItemValuesSet(3, aIDs, aVals);
						*a_ppEffect = pMemCfg.Detach();
						return; // for now, only supporting single effect
					}
				}
				effect = effect->next_sibling();
			}
		}

		TMatrix3x3f xform;
		float fillOpacity;
		std::string fill;
		float strokeOpacity;
		std::string stroke;
		float strokeWidth;
		float strokePos;
		std::vector<float> dashArray;
		float dashOffset;
		float opacity;
		std::string strokeCap;
		std::string strokeJoin;
		bool displayNone;
		float fontSize;
		std::string fontFamily;
		std::string fontWeight;
		std::string fontStyle;
		std::string filter;
	};

	static void ParseCSS(rapidxml::xml_node<>* style, std::map<std::string, std::string>& classes)
	{
		char const* pBeg = style->value();
		char const* pEnd = style->value()+style->value_size();
		std::pair<std::string, std::string> cls;
		std::vector<std::pair<std::string, std::string> > clss;
		rule<scanner<char const*> > ident = (alpha_p|ch_p('-')|ch_p('_'))>>*(alnum_p|ch_p('-')|ch_p('_'));
		rule<scanner<char const*> > clas = *space_p>>
			(ch_p('.')>>(ident[assign_a(cls.first)]))>>*space_p>>ch_p('{')>>(*(~ch_p('}')))[assign_a(cls.second)]>>*space_p>>ch_p('}')>>*space_p;
		bool bParsed = parse(pBeg, pEnd, *(clas[push_back_a(clss, cls)])).full;
		classes.insert(clss.begin(), clss.end());
	}

	static void TransformDirection(TMatrix3x3f const& xform, TPixelCoords dir, TPixelCoords* res)
	{
		float const fW0 = 1.0f/(xform._33);
		float const fX0 = fW0*(xform._31);
		float const fY0 = fW0*(xform._32);

		float const fW = 1.0f/(xform._13*dir.fX + xform._23*dir.fY + xform._33);
		res->fX = fW*(xform._11*dir.fX + xform._21*dir.fY + xform._31)-fX0;
		res->fY = fW*(xform._12*dir.fX + xform._22*dir.fY + xform._32)-fY0;
	}
	static void TransformFloats(TMatrix3x3f const& xform, float& x, float& y)
	{
		float const fW = 1.0f/(xform._13*x + xform._23*y + xform._33);
		float const xx = fW*(xform._11*x + xform._21*y + xform._31);
		y = fW*(xform._12*x + xform._22*y + xform._32);
		x = xx;
	}

	class ATL_NO_VTABLE CShapeCallback :
		public CComObjectRootEx<CComMultiThreadModel>,
		public IRasterImageEditToolPolygon
	{
	public:
		CShapeCallback() : m_xform(NULL), m_degenerate(false)
		{
		}

		void Init(TMatrix3x3f const* a_xform)
		{
			m_xform = a_xform;
		}

		BSTR Params()
		{
			return m_bstr;
		}
		bool HasDegenerateParts()
		{
			return m_degenerate;
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
			CComBSTR bstr;
			OLECHAR sz[144];
			TRWPathPoint tmp;
			for (ULONG p = 0; p < a_nCount; ++p)
			{
				TRWPath const* path = a_pPaths+p;
				if (path->nVertices < 2)
					continue;
				if (path->nVertices == 2 && !m_degenerate)
				{
					if (path->pVertices[0].tTanNext.fX == 0.0f && path->pVertices[0].tTanNext.fY == 0.0f &&
						path->pVertices[0].tTanPrev.fX == 0.0f && path->pVertices[0].tTanPrev.fY == 0.0f &&
						path->pVertices[1].tTanNext.fX == 0.0f && path->pVertices[1].tTanNext.fY == 0.0f &&
						path->pVertices[1].tTanPrev.fX == 0.0f && path->pVertices[1].tTanPrev.fY == 0.0f)
						m_degenerate = true;
				}
				for (ULONG v = 0; v < path->nVertices; ++v)
				{
					TRWPathPoint const* i = path->pVertices+v;
					if (m_xform)
					{
						tmp.dwFlags = i->dwFlags;
						float const fW = 1.0f/(m_xform->_13*i->tPos.fX + m_xform->_23*i->tPos.fY + m_xform->_33);
						tmp.tPos.fX = fW*(m_xform->_11*i->tPos.fX + m_xform->_21*i->tPos.fY + m_xform->_31);
						tmp.tPos.fY = fW*(m_xform->_12*i->tPos.fX + m_xform->_22*i->tPos.fY + m_xform->_32);

						//TransformDirection(*m_xform, i->tTanNext, &tmp.tTanNext);
						//TransformDirection(*m_xform, i->tTanPrev, &tmp.tTanPrev);

						tmp.tTanNext.fX = m_xform->_11*i->tTanNext.fX + m_xform->_21*i->tTanNext.fY;
						tmp.tTanNext.fY = m_xform->_12*i->tTanNext.fX + m_xform->_22*i->tTanNext.fY;
						tmp.tTanPrev.fX = m_xform->_11*i->tTanPrev.fX + m_xform->_21*i->tTanPrev.fY;
						tmp.tTanPrev.fY = m_xform->_12*i->tTanPrev.fX + m_xform->_22*i->tTanPrev.fY;

						//tmp.tTanNext.fX = m_inverse._11*i->tTanNext.fX + m_inverse._12*i->tTanNext.fY;
						//tmp.tTanNext.fY = m_inverse._21*i->tTanNext.fX + m_inverse._22*i->tTanNext.fY;
						//tmp.tTanPrev.fX = m_inverse._11*i->tTanPrev.fX + m_inverse._12*i->tTanPrev.fY;
						//tmp.tTanPrev.fY = m_inverse._21*i->tTanPrev.fX + m_inverse._22*i->tTanPrev.fY;
						i = &tmp;
					}
					swprintf(sz, bstr == NULL ? L"\"%c\", %g, %g, %g, %g, %g, %g" : L", \"%c\", %g, %g, %g, %g, %g, %g", v+1 == path->nVertices ? ((i->dwFlags&1) ? L'O' : L'E') : (i->dwFlags&2 ? L'S' : L'C'), i->tPos.fX, i->tPos.fY, i->tTanNext.fX, i->tTanNext.fY, i->tTanPrev.fX, i->tTanPrev.fY);
					bstr += sz;
				}
			}
			std::swap(bstr.m_str, m_bstr.m_str);
			return S_OK;

		}
		STDMETHOD(ToPath)(IRasterImageEditToolPolygon* UNREF(a_pConsumer)) { return E_NOTIMPL; }

	private:
		CComBSTR m_bstr;
		TMatrix3x3f const* m_xform;
		bool m_degenerate;
	};

	struct CNodeTree
	{
		CNodeTree()
		{
		}
		~CNodeTree()
		{
			for (std::vector<SItem>::iterator i = m_items.begin(); i != m_items.end(); ++i)
			{
				SysFreeString(i->bstrName);
				if (i->pEffect)
					i->pEffect->Release();
				switch (i->eType)
				{
				case ETVectorObject:
					SysFreeString(i->vec.bstrToolID);
					SysFreeString(i->vec.bstrParams);
					SysFreeString(i->vec.bstrStyleID);
					SysFreeString(i->vec.bstrStyleParams);
					break;
				case ETRasterImage:
					delete[] i->rst.pData;
					break;
				case ETGroup:
					delete i->grp.tree;
					break;
				}
			}
		}
		void AddVectorObject(BSTR a_bstrName, bool a_bEnabled, BSTR a_bstrToolID, BSTR a_bstrParams, BSTR a_bstrStyleID, BSTR a_bstrStyleParams, float const* a_pOutlineWidth, float const* a_pOutlinePos, EOutlineJoinType const* a_pOutlineJoins, TColor const* a_pOutlineColor)//, ERasterizationMode const* a_pRasterizationMode, ECoordinatesMode const* a_pCoordinatesMode, boolean const* a_pEnabled
		{
			m_items.reserve(m_items.size()+1);
			SItem s;
			s.eType = ETVectorObject;
			s.bstrName = a_bstrName ? SysAllocString(a_bstrName) : NULL;
			s.enabled = a_bEnabled;
			s.pEffect = NULL;
			s.vec.bstrToolID = a_bstrToolID ? SysAllocString(a_bstrToolID) : NULL;
			s.vec.bstrParams = a_bstrParams ? SysAllocString(a_bstrParams) : NULL;
			s.vec.bstrStyleID = a_bstrStyleID ? SysAllocString(a_bstrStyleID) : NULL;
			s.vec.bstrStyleParams = a_bstrStyleParams ? SysAllocString(a_bstrStyleParams) : NULL;
			if (a_pOutlineWidth || a_pOutlinePos || a_pOutlineJoins || a_pOutlineColor)
			{
				s.vec.bUseOutline = true;
				s.vec.fOutlineWidth = *a_pOutlineWidth;
				s.vec.fOutlinePos = *a_pOutlinePos;
				s.vec.eOutlineJoins = a_pOutlineJoins ? *a_pOutlineJoins : EOJTRound;
				s.vec.tOutlineColor = *a_pOutlineColor;
			}
			else
			{
				s.vec.bUseOutline = false;
			}
			//s.vec.eRasterizationMode = *a_pRasterizationMode;
			//s.vec.eCoordinatesMode = *a_pCoordinatesMode;
			//s.vec.bEnabled = *a_pEnabled;
			m_items.push_back(s);
		}
		void AddRasterImage(BSTR a_bstrName, bool a_bEnabled, IConfig* pEffect, CAutoVectorPtr<BYTE>& data, size_t len, TMatrix3x3f const& trans)
		{
			m_items.reserve(m_items.size()+1);
			SItem s;
			s.eType = ETRasterImage;
			s.bstrName = a_bstrName ? SysAllocString(a_bstrName) : NULL;
			s.enabled = a_bEnabled;
			s.pEffect = pEffect;
			if (s.pEffect)
				s.pEffect->AddRef();
			s.rst.nSize = len;
			s.rst.pData = data.Detach();
			s.rst.tXform = trans;
			m_items.push_back(s);
		}
		CNodeTree& AddGroup(BSTR a_bstrName, bool a_bEnabled, IConfig* pEffect, bool important)
		{
			m_items.reserve(m_items.size()+1);
			SItem s;
			s.eType = ETGroup;
			s.bstrName = a_bstrName ? SysAllocString(a_bstrName) : NULL;
			s.enabled = a_bEnabled;
			s.pEffect = pEffect;
			if (s.pEffect)
				s.pEffect->AddRef();
			s.grp.tree = new CNodeTree;
			s.grp.important = important;
			m_items.push_back(s);
			return *s.grp.tree;
		}

		bool IsOnlyVectors() const
		{
			for (std::vector<SItem>::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
				if (i->eType != ETVectorObject)
					return false;
			return true;
		}

		void simplify()
		{
			// remove groups that contain single item
			for (std::vector<SItem>::iterator i = m_items.begin(); i != m_items.end(); ++i)
			{
				if (i->eType == ETGroup && !i->grp.important)
				{
					i->grp.tree->simplify();
					if (i->grp.tree->m_items.size() == 1)
					{
						CNodeTree* p = i->grp.tree;
						SItem s = p->m_items[0];
						p->m_items.clear();
						delete p;
						i->enabled &= s.enabled;
						i->eType = s.eType;
						switch (s.eType)
						{
						case ETVectorObject: i->vec = s.vec; break;
						case ETRasterImage: i->rst = s.rst; break;
						case ETGroup: i->grp = s.grp; break;
						}
						if (i->pEffect && s.pEffect)
							s.pEffect->Release();
						else if (s.pEffect)
							i->pEffect = s.pEffect;
						if (s.bstrName)
						{
							SysFreeString(i->bstrName);
							i->bstrName = s.bstrName;
						}
					}
				}
			}
			if (m_items.size() == 1 && m_items[0].eType == ETGroup && m_items[0].enabled && m_items[0].pEffect == NULL && m_items[0].bstrName == NULL)
			{
				std::vector<SItem> tmp;
				tmp.swap(m_items[0].grp.tree->m_items);
				m_items.clear();
				tmp.swap(m_items);
			}
			else if (m_items.size() == 1 && m_items[0].eType == ETGroup && m_items[0].enabled && m_items[0].grp.tree->m_items.size() == 1 && (m_items[0].pEffect == NULL || m_items[0].grp.tree->m_items[0].pEffect == NULL) && (m_items[0].bstrName == NULL || m_items[0].grp.tree->m_items[0].bstrName == NULL))
			{
				std::vector<SItem> tmp;
				IConfig* tmpEff = m_items[0].pEffect;
				BSTR tmpName = m_items[0].bstrName;
				tmp.swap(m_items[0].grp.tree->m_items);
				m_items.clear();
				tmp.swap(m_items);
				if (tmpEff) m_items[0].pEffect = tmpEff;
				if (tmpName) m_items[0].bstrName = tmpName;
			}
		}

		void toDoc(IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, TImageSize const& a_tCanvasSize, TImageResolution const* a_pResolution, float a_fGamma, TPixelChannel a_tDefault) const
		{
			std::vector<SItem>::const_iterator i = m_items.begin();
			std::vector<SItem>::const_iterator iVec = m_items.end();
			while (i != m_items.end())
			{
				if (i->eType != ETVectorObject && iVec != m_items.end())
				{
					AddVectorLayer(iVec, i, a_pBuilder, a_bstrPrefix, a_pBase, NULL, NULL, NULL, a_tCanvasSize, a_pResolution, a_fGamma, a_tDefault);
					iVec = m_items.end();
				}
				switch (i->eType)
				{
				case ETVectorObject:
					if (iVec == m_items.end())
						iVec = i;
					break;
				case ETRasterImage:
					{
						CComPtr<IInputManager> pIM;
						RWCoCreateInstance(pIM, __uuidof(InputManager));
						CComPtr<IDocumentBuilder> pBuilder;
						RWCoCreateInstance(pBuilder, __uuidof(DocumentFactoryRasterImage));
						CComPtr<IDocument> pDoc;
						RWCoCreateInstance(pDoc, __uuidof(DocumentBase));
						pIM->DocumentCreateDataEx(pBuilder, i->rst.nSize, i->rst.pData, NULL, NULL, CComQIPtr<IDocumentBase>(pDoc), NULL, NULL, NULL);
						{
							CComPtr<IDocumentEditableImage> pDEI;
							pDoc->QueryFeatureInterface(__uuidof(IDocumentEditableImage), reinterpret_cast<void**>(&pDEI));
							GUID const CLSID_BilinearTransformHelper = {0x7d22d397, 0xd956, 0x491e, { 0x9b, 0x55, 0x65, 0x20, 0x21, 0xdd, 0x6b, 0xac}};
							CComPtr<IRasterImageTransformer> pTrans;
							RWCoCreateInstance(pTrans, CLSID_BilinearTransformHelper);
							pDEI->CanvasSet(&a_tCanvasSize, NULL, &i->rst.tXform, pTrans);

							//CComPtr<IDocumentOperation> pTrans;
							//static GUID const CLSID_DocumentOperationRasterImagePerspective = {0xA13EC2D9, 0xD8F7, 0x4C05, {0xA9, 0x98, 0x1E, 0x43, 0x06, 0x1F, 0x3E, 0xFC}};
							//RWCoCreateInstance(pTrans, CLSID_DocumentOperationRasterImagePerspective);
							//CComPtr<IConfig> pCfg;
							//pTrans->ConfigCreate(NULL, &pCfg);
							//CComBSTR bstr1x(L"Pt1X");
							//CComBSTR bstr1y(L"Pt1Y");
							//CComBSTR bstr2x(L"Pt2X");
							//CComBSTR bstr2y(L"Pt2Y");
							//CComBSTR bstr3x(L"Pt3X");
							//CComBSTR bstr3y(L"Pt3Y");
							//CComBSTR bstr4x(L"Pt4X");
							//CComBSTR bstr4y(L"Pt4Y");
							//BSTR aIDs[] = {bstr1x, bstr1y, bstr2x, bstr2y, bstr3x, bstr3y, bstr4x, bstr4y};
							//TConfigValue aVals[8];
							//aVals[0].eTypeID = aVals[1].eTypeID = aVals[2].eTypeID = aVals[3].eTypeID = 
							//aVals[4].eTypeID = aVals[5].eTypeID = aVals[6].eTypeID = aVals[7].eTypeID = ECVTFloat;
							//aVals[0].fVal = i->rst.;
							//pCfg->ItemValuesSet(8, aIDs, aVals);
							//pTrans->Activate(NULL, pDoc, pCfg, NULL, NULL, 0);
						}
						TImageLayer tProps;
						tProps.bVisible = i->enabled;
						tProps.fOpacity = 1.0f;
						tProps.eBlend = EBEAlphaBlend;
						a_pBuilder->AddLayer(a_bstrPrefix, a_pBase, 0, &CImageLayerCreatorDocument(pDoc), i->bstrName, &tProps, i->pEffect);
					}
					break;
				case ETGroup:
					if (!i->grp.tree->m_items.empty())
					{
						if (i->grp.tree->IsOnlyVectors())
						{
							TImageLayer tProps;
							tProps.bVisible = i->enabled;
							tProps.fOpacity = 1.0f;
							tProps.eBlend = EBEAlphaBlend;
							AddVectorLayer(i->grp.tree->m_items.begin(), i->grp.tree->m_items.end(), a_pBuilder, a_bstrPrefix, a_pBase, i->bstrName, i->pEffect, &tProps, a_tCanvasSize, a_pResolution, a_fGamma, a_tDefault);
						}
						else
						{
							TImageLayer tProps;
							tProps.bVisible = i->enabled;
							tProps.fOpacity = 1.0f;
							tProps.eBlend = EBEAlphaBlend;
							a_pBuilder->AddLayer(a_bstrPrefix, a_pBase, 0, &CGroupCreator(a_tCanvasSize, a_pResolution, a_fGamma, a_tDefault, i->grp.tree), i->bstrName, &tProps, i->pEffect);
						}
					}
					break;
				}
				++i;
			}
			if (iVec != i)
				AddVectorLayer(iVec, i, a_pBuilder, a_bstrPrefix, a_pBase, NULL, NULL, NULL, a_tCanvasSize, a_pResolution, a_fGamma, a_tDefault);
		}

	private:
		struct SVectorObject
		{
			BSTR bstrToolID;
			BSTR bstrParams;
			BSTR bstrStyleID;
			BSTR bstrStyleParams;
			bool bUseOutline;
			float fOutlineWidth;
			float fOutlinePos;
			EOutlineJoinType eOutlineJoins;
			TColor tOutlineColor;
			//ERasterizationMode eRasterizationMode;
			//ECoordinatesMode eCoordinatesMode;
			//boolean bEnabled;
		};
		struct SRasterImage
		{
			TMatrix3x3f tXform;
			ULONG nSize;
			BYTE const* pData;
		};
		struct SGroup
		{
			CNodeTree* tree;
			bool important;
		};
		enum EType
		{
			ETVectorObject = 0,
			ETRasterImage,
			ETGroup,
		};
		struct SItem
		{
			EType eType;
			BSTR bstrName;
			bool enabled;
			IConfig* pEffect;
			union
			{
				SVectorObject vec;
				SRasterImage rst;
				SGroup grp;
			};
		};

		class CVectorCreator :
			public IImageLayerCreator
		{
		public:
			CVectorCreator(TImageSize const& a_tCanvasSize, TImageResolution const* a_pResolution, float a_fGamma, TPixelChannel a_tDefault, std::vector<SItem>::const_iterator b, std::vector<SItem>::const_iterator e) :
				m_tCanvasSize(a_tCanvasSize), m_pResolution(a_pResolution), m_fGamma(a_fGamma), m_tDefault(a_tDefault), m_b(b), m_e(e)
			{
			}

			// IUnknown methods
		public:
			STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
			{
				if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
				{
					*a_ppvObject = this;
					return S_OK;
				}
				return E_NOINTERFACE;
			}
			STDMETHOD_(ULONG, AddRef)() { return 2; }
			STDMETHOD_(ULONG, Release)() { return 1; }

			// IImageLayerCreator methods
		public:
			STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
			{
				CComPtr<IDocumentFactoryVectorImage> pFct;
				RWCoCreateInstance(pFct, __uuidof(DocumentFactoryVectorImage));
				static float const s_aBg[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				HRESULT hRes = pFct->Create(a_bstrID, a_pBase, &m_tCanvasSize, m_pResolution, s_aBg);
				if (FAILED(hRes)) return hRes;
				for (std::vector<SItem>::const_iterator i = m_b; i != m_e; ++i)
				{
					boolean enabled = i->enabled;
					if (i->vec.bUseOutline)
						pFct->AddObject(a_bstrID, a_pBase, i->bstrName, i->vec.bstrToolID, i->vec.bstrParams, i->vec.bstrStyleID, i->vec.bstrStyleParams, &i->vec.fOutlineWidth, &i->vec.fOutlinePos, &i->vec.eOutlineJoins, &i->vec.tOutlineColor, NULL, NULL, &enabled);
					else
						pFct->AddObject(a_bstrID, a_pBase, i->bstrName, i->vec.bstrToolID, i->vec.bstrParams, i->vec.bstrStyleID, i->vec.bstrStyleParams, NULL, NULL, NULL, NULL, NULL, NULL, &enabled);
				}
				return S_OK;
			}


		private:
			TImageSize m_tCanvasSize;
			TImageResolution const* m_pResolution;
			float m_fGamma;
			TPixelChannel m_tDefault;
			std::vector<SItem>::const_iterator m_b;
			std::vector<SItem>::const_iterator m_e;
		};

		class CGroupCreator :
			public IImageLayerCreator
		{
		public:
			CGroupCreator(TImageSize const& a_tCanvasSize, TImageResolution const* a_pResolution, float a_fGamma, TPixelChannel a_tDefault, CNodeTree const* a_pSubTree) :
				m_tCanvasSize(a_tCanvasSize), m_pResolution(a_pResolution), m_fGamma(a_fGamma), m_tDefault(a_tDefault), m_pSubTree(a_pSubTree)
			{
			}

			// IUnknown methods
		public:
			STDMETHOD(QueryInterface)(REFIID a_riid, void** a_ppvObject)
			{
				if (IsEqualIID(a_riid, IID_IUnknown) || IsEqualIID(a_riid, __uuidof(IImageLayerCreator)))
				{
					*a_ppvObject = this;
					return S_OK;
				}
				return E_NOINTERFACE;
			}
			STDMETHOD_(ULONG, AddRef)() { return 2; }
			STDMETHOD_(ULONG, Release)() { return 1; }

			// IImageLayerCreator methods
		public:
			STDMETHOD(Create)(BSTR a_bstrID, IDocumentBase* a_pBase)
			{
				CComPtr<IDocumentFactoryLayeredImage> pFct;
				RWCoCreateInstance(pFct, __uuidof(DocumentFactoryLayeredImage));
				static float const s_aBg[4] = {0.0f, 0.0f, 0.0f, 0.0f};
				HRESULT hRes = pFct->Create(a_bstrID, a_pBase);//, &m_tCanvasSize, m_pResolution, s_aBg);
				m_pSubTree->toDoc(pFct, a_bstrID, a_pBase, m_tCanvasSize, m_pResolution, m_fGamma, m_tDefault);
				pFct->SetSize(a_bstrID, a_pBase, &m_tCanvasSize);
				return S_OK;
			}

		private:
			TImageSize m_tCanvasSize;
			TImageResolution const* m_pResolution;
			float m_fGamma;
			TPixelChannel m_tDefault;
			CNodeTree const* m_pSubTree;
		};

		static void AddVectorLayer(std::vector<SItem>::const_iterator b, std::vector<SItem>::const_iterator e, IDocumentFactoryLayeredImage* a_pBuilder, BSTR a_bstrPrefix, IDocumentBase* a_pBase, BSTR bstrName, IConfig* pEffect, TImageLayer const* pProps, TImageSize const& a_tCanvasSize, TImageResolution const* a_pResolution, float a_fGamma, TPixelChannel a_tDefault)
		{
			a_pBuilder->AddLayer(a_bstrPrefix, a_pBase, 0, &CVectorCreator(a_tCanvasSize, a_pResolution, a_fGamma, a_tDefault, b, e), bstrName, pProps, pEffect);
		}

	private:
		std::vector<SItem> m_items;
	};

	static HRESULT addNode(SSVGStyle const& style, std::map<std::string, std::string> const& classes, TImageSize canvasSize, CNodeTree& tree, IBezierPathUtils* pUtils, rapidxml::xml_node<>* node)
	{
		float const fZero = 0.0f;
		rapidxml::xml_document<>* xmldoc = node->document();
		SSVGStyle myStyle = style;
		rapidxml::xml_attribute<>* clas = node->first_attribute("class");
		if (clas)
		{
			std::map<std::string, std::string>::const_iterator i = classes.find(clas->value());
			if (i != classes.end())
				myStyle.MergeCSS(i->second.c_str(), i->second.c_str()+i->second.length());
		}
		myStyle.MergeStyle(node);

		CComBSTR bstrID;
		rapidxml::xml_attribute<>* id = node->first_attribute("inkscape:label");
		if (id == NULL)
			id = node->first_attribute("id");
		if (id && *id->value())
		{
			char* p = id->value();
			int nLen = MultiByteToWideChar(CP_UTF8, 0, p, -1, NULL, 0);
			bstrID.Attach(SysAllocStringLen(NULL, nLen));
			MultiByteToWideChar(CP_UTF8, 0, p, -1, bstrID, nLen);
		}

		bool enabled = !myStyle.displayNone;

		CComPtr<IConfig> pEffect;
		myStyle.GetFilter(&pEffect, xmldoc);

		char const* name = node->name();
		if (strcmp(name, "path") == 0)
		{
			rapidxml::xml_attribute<>* data = node->first_attribute("d");
			if (data)
			{
				char* p = data->value();
				int nLen = MultiByteToWideChar(CP_UTF8, 0, p, -1, NULL, 0);
				CComBSTR bstr(nLen);
				MultiByteToWideChar(CP_UTF8, 0, p, -1, bstr, nLen);
				CComObjectStackEx<CShapeCallback> sc;
				TMatrix3x3f const* xform = memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0 ? &myStyle.xform : NULL;
				sc.Init(xform);
				pUtils->SVGToPolygon(bstr, &sc);

				CComBSTR bstrFill;
				CComBSTR bstrFillParam;
				bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
				CComBSTR bstrOutline;
				CComBSTR bstrOutlineParam;
				TColor tOutlineColor = {0, 0, 0, 1};
				float strokeWidth = 0.0f;
				bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);
				bool solidOutline = bstrOutline == L"SOLID";

				if (useFill)
				{
					if (useOutline && solidOutline && !sc.HasDegenerateParts())
					{
						std::string const& join = myStyle.strokeJoin;
						EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"SHAPE"), sc.Params(), bstrFill, bstrFillParam, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
					}
					else
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"SHAPE"), sc.Params(), bstrFill, bstrFillParam, NULL, NULL, NULL, NULL);
				}
				if (useOutline && (!solidOutline || !useFill || sc.HasDegenerateParts()))
				{
					std::string const& join = myStyle.strokeJoin;
					std::string const& cap = myStyle.strokeCap;
					CComBSTR dash;
					myStyle.GetDash(dash);
					wchar_t sz[64];
					swprintf(sz, L"\"CWIDTH\", %g, ", strokeWidth);
					CComBSTR params(sz);
					params += dash;
					if (join.compare("round") == 0)
						params += L"\"JROUND\", ";
					else if (join.compare("bevel") != 0)
						params += L"\"JMITER\", ";
					if (cap.compare("round") == 0)
						params += L"\"CROUND\", ";
					else if (cap.compare("butt") == 0)
						params += L"\"CBUTT\", ";
					params += sc.Params();
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"STROKE"), CComBSTR(params), bstrOutline, bstrOutlineParam, NULL, NULL, NULL, NULL);
				}
			}
		}
		else if (strcmp(name, "polygon") == 0 || strcmp(name, "polyline") == 0)
		{
			rapidxml::xml_attribute<>* data = node->first_attribute("points");
			if (data)
			{
				char const* p = data->value();
				std::vector<double> coords;
				bool bParsed = parse(data->value(), data->value()+data->value_size(),
					*(*(space_p|ch_p(','))>>real_p[push_back_a(coords)])>>*(space_p|ch_p(','))).full;
					//*space_p>>list_p(real_p[push_back_a(coords)], *(space_p|ch_p(',')))>>*space_p).full;

				if (coords.size()&1) // odd number of numbers
					coords.resize(coords.size()-1);

				TMatrix3x3f const* xform = memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0 ? &myStyle.xform : NULL;

				CComBSTR bstrCoords;
				for (size_t i = 0; i < coords.size(); i+=2)
				{
					float x = coords[i];
					float y = coords[i+1];
					if (xform)
					{
						float const fW = 1.0f/(xform->_13*x + xform->_23*y + xform->_33);
						float const t = fW*(xform->_11*x + xform->_21*y + xform->_31);
						y = fW*(xform->_12*x + xform->_22*y + xform->_32);
						x = t;
					}
					wchar_t sz[32];
					swprintf(sz, bstrCoords.m_str ? L",%g,%g" : L"%g,%g", x, y);
					bstrCoords += sz;
				}

				CComBSTR bstrFill;
				CComBSTR bstrFillParam;
				bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
				CComBSTR bstrOutline;
				CComBSTR bstrOutlineParam;
				TColor tOutlineColor = {0, 0, 0, 1};
				float strokeWidth = 0.0f;
				bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);
				bool solidOutline = bstrOutline == L"SOLID";

				if (useFill)
				{
					if (useOutline && solidOutline)
					{
						std::string const& join = myStyle.strokeJoin;
						EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"POLYGON"), bstrCoords, bstrFill, bstrFillParam, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
					}
					else
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"POLYGON"), bstrCoords, bstrFill, bstrFillParam, NULL, NULL, NULL, NULL);
				}
				if (useOutline && (!solidOutline || !useFill))
				{
					std::string const& join = myStyle.strokeJoin;
					std::string const& cap = myStyle.strokeCap;
					CComBSTR dash;
					myStyle.GetDash(dash);
					wchar_t sz[64];
					swprintf(sz, L"%g, ", strokeWidth);
					CComBSTR params(sz);
					params += dash;
					if (join.compare("round") == 0)
						params += L"\"JROUND\", ";
					else if (join.compare("bevel") != 0)
						params += L"\"JMITER\", ";
					if (cap.compare("round") == 0)
						params += L"\"CROUND\", ";
					else if (cap.compare("butt") == 0)
						params += L"\"CBUTT\", ";
					params += bstrCoords;
					if (strcmp(name, "polygon") == 0)
						params += L", \"CLOSE\"";
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"LINE"), CComBSTR(params), bstrOutline, bstrOutlineParam, NULL, NULL, NULL, NULL);
				}
			}
		}
		else if (strcmp(name, "line") == 0)
		{
			TVector2f v1 = { getFloatAttr(node, "x1", 0.0f), getFloatAttr(node, "y1", 0.0f) };
			TVector2f v2 = { getFloatAttr(node, "x2", 0.0f), getFloatAttr(node, "y2", 0.0f) };
			if (v1.x != v2.x || v1.y != v2.y)
			{
				if (memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0)
				{
					v1 = TransformVector2(myStyle.xform, v1);
					v2 = TransformVector2(myStyle.xform, v2);
				}

				CComBSTR bstrFill;
				CComBSTR bstrFillParam;
				bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
				CComBSTR bstrOutline;
				CComBSTR bstrOutlineParam;
				TColor tOutlineColor = {0, 0, 0, 1};
				float strokeWidth = 0.0f;
				bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);
				bool solidOutline = bstrOutline == L"SOLID";

				//if (useOutline && (!solidOutline || !useFill))
				{
					std::string const& join = myStyle.strokeJoin;
					std::string const& cap = myStyle.strokeCap;
					CComBSTR dash;
					myStyle.GetDash(dash);
					wchar_t sz[64];
					swprintf(sz, L"%g, ", strokeWidth);
					CComBSTR params(sz);
					params += dash;
					if (join.compare("round") == 0)
						params += L"\"JROUND\", ";
					else if (join.compare("bevel") != 0)
						params += L"\"JMITER\", ";
					if (cap.compare("round") == 0)
						params += L"\"CROUND\", ";
					else if (cap.compare("butt") == 0)
						params += L"\"CBUTT\", ";
					swprintf(sz, L"%g, %g, %g, %g", v1.x, v1.y, v2.x, v2.y);
					params += sz;
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"LINE"), CComBSTR(params), bstrOutline, bstrOutlineParam, NULL, NULL, NULL, NULL);
				}
			}
		}
		else if (strcmp(name, "circle") == 0 || strcmp(name, "ellipse") == 0)
		{
			float r = getFloatAttr(node, "r", -1.0f);
			float rx = getFloatAttr(node, "rx", -1.0f);
			float ry = getFloatAttr(node, "ry", -1.0f);
			TVector2f center = {getFloatAttr(node, "cx", 0.0f), getFloatAttr(node, "cy", 0.0f)};
			if (r > 0.0f && rx < 0.0f)
				rx = r;
			if (r > 0.0f && ry < 0.0f)
				ry = r;
			if (rx > 0.0f && ry > 0.0f)
			{
				float angle = 0.0f;
				if (memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0)
				{
					TVector2f dx = {center.x+rx, center.y};
					TVector2f dy = {center.x, center.y+ry};
					center = TransformVector2(myStyle.xform, center);
					dx = TransformVector2(myStyle.xform, dx);
					dy = TransformVector2(myStyle.xform, dy);
					dx.x -= center.x;
					dx.y -= center.y;
					dy.x -= center.x;
					dy.y -= center.y;
					angle = atan2f(dx.y, dx.x);
					rx = sqrtf(dx.x*dx.x + dx.y*dx.y);
					ry = sqrtf(dy.x*dy.x + dy.y*dy.y);
				}
				CComBSTR bstrFill;
				CComBSTR bstrFillParam;
				bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
				CComBSTR bstrOutline;
				CComBSTR bstrOutlineParam;
				TColor tOutlineColor = {0, 0, 0, 1};
				float strokeWidth = 0.0f;
				bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);

				wchar_t params[80];
				if (rx == ry && angle == 0.0f)
					swprintf(params, L"%g,%g,%g", center.x, center.y, rx);
				else if (angle == 0.0f)
					swprintf(params, L"%g,%g,%g,%g", center.x, center.y, rx, ry);
				else
					swprintf(params, L"%g,%g,%g,%g,%g", center.x, center.y, rx, ry, angle);

				if (useFill)
				{
					if (useOutline)
					{
						std::string const& join = myStyle.strokeJoin;
						EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"ELLIPSE"), CComBSTR(params), bstrFill, bstrFillParam, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
					}
					else
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"ELLIPSE"), CComBSTR(params), bstrFill, bstrFillParam, NULL, NULL, NULL, NULL);
				}
				else if (useOutline)
				{
					std::string const& join = myStyle.strokeJoin;
					EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"ELLIPSE"), CComBSTR(params), NULL, NULL, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
				}
			}
		}
		else if (strcmp(name, "rect") == 0)
		{
			float width = getFloatAttr(node, "width", canvasSize.nX);
			float height = getFloatAttr(node, "height", canvasSize.nY);
			float rx = getFloatAttr(node, "rx", -1.0f);
			float ry = getFloatAttr(node, "ry", -1.0f);
			TVector2f center = {getFloatAttr(node, "x", 0.0f), getFloatAttr(node, "y", 0.0f)};
			center.x += width*0.5f;
			center.y += height*0.5f;
			if (rx < 0.0f)
				rx = ry;
			if (ry < 0.0f)
				ry = rx;
			if (rx < 0.0f)
				rx = 0.0f;
			if (ry < 0.0f)
				ry = 0.0f;
			if (width > 0.0f && height > 0.0f)
			{
				float angle = 0.0f;
				if (memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0)
				{
					TVector2f dx = {center.x+width*0.5f, center.y};
					TVector2f dy = {center.x, center.y+height*0.5f};
					TVector2f drx = {center.x+rx, center.y};
					TVector2f dry = {center.x, center.y+ry};
					center = TransformVector2(myStyle.xform, center);
					dx = TransformVector2(myStyle.xform, dx);
					dy = TransformVector2(myStyle.xform, dy);
					drx = TransformVector2(myStyle.xform, drx);
					dry = TransformVector2(myStyle.xform, dry);
					dx.x -= center.x;
					dx.y -= center.y;
					dy.x -= center.x;
					dy.y -= center.y;
					drx.x -= center.x;
					drx.y -= center.y;
					dry.x -= center.x;
					dry.y -= center.y;
					angle = atan2f(dx.y, dx.x);
					width = sqrtf(dx.x*dx.x + dx.y*dx.y);
					height = sqrtf(dy.x*dy.x + dy.y*dy.y);
					rx = sqrtf(drx.x*drx.x + drx.y*drx.y);
					ry = sqrtf(dry.x*dry.x + dry.y*dry.y);
				}
				else
				{
					width *= 0.5f;
					height *= 0.5f;
				}
				CComBSTR bstrFill;
				CComBSTR bstrFillParam;
				bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
				CComBSTR bstrOutline;
				CComBSTR bstrOutlineParam;
				TColor tOutlineColor = {0, 0, 0, 1};
				float strokeWidth = 0.0f;
				bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);

				float r = rx == ry ? rx : sqrtf(rx*ry);
				if (r < 0.0f)
					r = 0.0f;

				//if (useOutline)
				//	r += myStyle.strokeWidth*0.5f;

				wchar_t params[80];
				if (width == height && angle == 0.0f)
					swprintf(params, L"%g,%g,%g,%g", r, center.x, center.y, width);
				else if (angle == 0.0f)
					swprintf(params, L"%g,%g,%g,%g,%g", r, center.x, center.y, width, height);
				else
					swprintf(params, L"%g,%g,%g,%g,%g,%g", r, center.x, center.y, width, height, angle);

				if (useFill)
				{
					if (useOutline)
					{
						std::string const& join = myStyle.strokeJoin;
						EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"RECTANGLE"), CComBSTR(params), bstrFill, bstrFillParam, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
					}
					else
						tree.AddVectorObject(bstrID, enabled, CComBSTR(L"RECTANGLE"), CComBSTR(params), bstrFill, bstrFillParam, NULL, NULL, NULL, NULL);
				}
				else if (useOutline)
				{
					std::string const& join = myStyle.strokeJoin;
					EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"RECTANGLE"), CComBSTR(params), NULL, NULL, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
				}
			}
		}
		else if (strcmp(name, "text") == 0)
		{
			float x = getFloatAttr(node, "x", 0.0f);
			float y = getFloatAttr(node, "y", 0.0f);
			char const* text = node->value();

			std::string font;
			DecodeFontFamily(myStyle.fontFamily, font);
			wchar_t const* align = L"";
			rapidxml::xml_attribute<>* ta = node->first_attribute("text-anchor");
			if (ta)
			{
				if (strcmp(ta->value(), "middle"))
					align = L", \"CENTER\"";
				if (strcmp(ta->value(), "end"))
					align = L", \"RIGHT\"";
			}
			rapidxml::xml_attribute<>* db = node->first_attribute("dominant-baseline");
			bool botAlign = db && strcmp(ta->value(), "text-before-edge");

			bool italic = myStyle.fontStyle == "italic";
			bool bold = myStyle.fontWeight == "bold";
			wchar_t const* fontStyle = italic ? (bold ? L",\"BOLDITALIC\"" : L",\"ITALIC\"") : (bold ? L",\"BOLD\"" : L"");

			float fontSize = myStyle.fontSize;//*96.0f/72.0f;

			if (memcmp(&myStyle.xform, &TMATRIX3X3F_IDENTITY, sizeof(myStyle.xform)) != 0)
			{
				TVector2f pos = {x, y};
				TVector2f p2 = TransformVector2(myStyle.xform, pos);
				x = p2.x;
				y = p2.y;
				if (!botAlign)
				{
					pos.y -= fontSize;
					p2 = TransformVector2(myStyle.xform, pos);
					fontSize = y-p2.y;
					y = p2.y;
				}
			}
			else
			{
				if (!botAlign)
					y -= fontSize;
			}

			CComBSTR bstrFill;
			CComBSTR bstrFillParam;
			bool useFill = myStyle.GetFill(bstrFill, bstrFillParam, xmldoc);
			CComBSTR bstrOutline;
			CComBSTR bstrOutlineParam;
			TColor tOutlineColor = {0, 0, 0, 1};
			float strokeWidth = 0.0f;
			bool useOutline = myStyle.GetStroke(bstrOutline, bstrOutlineParam, tOutlineColor, strokeWidth, xmldoc);

			wchar_t params[200];
			swprintf(params, L"\"%s\"%s,%g,%g,%g,\"", static_cast<wchar_t const*>(CA2W(font.c_str(), CP_UTF8)), fontStyle, fontSize, x, y);
			CComBSTR bstrParams = params;
			size_t extra = 0;
			size_t len = 0;
			for (char const* p = text; *p; ++p, ++len)
				if (*p == '\"')
					++extra;
			if (extra == 0)
			{
				bstrParams += static_cast<wchar_t const*>(CA2W(text, CP_UTF8));
			}
			else
			{
				CAutoVectorPtr<char> psz(new char[len+extra+1]);
				char* d = psz;
				for (char const* p = text; *p; ++p)
				{
					if (*p == '\"')
						*d++ = '\\';
					*d++ = *p;
				}
				*d = '\0';
				bstrParams += static_cast<wchar_t const*>(CA2W(psz.m_p, CP_UTF8));
			}
			bstrParams += L"\"";

			if (useFill)
			{
				if (useOutline)
				{
					std::string const& join = myStyle.strokeJoin;
					EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"TEXT"), bstrParams, bstrFill, bstrFillParam, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
				}
				else
					tree.AddVectorObject(bstrID, enabled, CComBSTR(L"TEXT"), bstrParams, bstrFill, bstrFillParam, NULL, NULL, NULL, NULL);
			}
			else if (useOutline)
			{
				std::string const& join = myStyle.strokeJoin;
				EOutlineJoinType jt = join.compare("round") == 0 ? EOJTRound : (join.compare("bevel") != 0 ? EOJTMiter : EOJTBevel);
				tree.AddVectorObject(bstrID, enabled, CComBSTR(L"TEXT"), CComBSTR(params), NULL, NULL, &strokeWidth, &myStyle.strokePos, &jt, &tOutlineColor);
			}
		}
		else if (strcmp(name, "image") == 0)
		{
			float w = getFloatAttr(node, "width", 0.0f);
			float h = getFloatAttr(node, "height", 0.0f);
			float x = getFloatAttr(node, "x", 0.0f);
			float y = getFloatAttr(node, "y", 0.0f);
			rapidxml::xml_attribute<>* href = node->first_attribute("xlink:href");
			if (href && strncmp(href->value(), "data:image/png;base64,", 22) == 0)
			{
				size_t encLen = href->value_size()-22;
				BYTE const* source = reinterpret_cast<BYTE const*>(href->value()+22);
				size_t decLen = (encLen*6+7)/8;
				CAutoVectorPtr<BYTE> decoded(new BYTE[decLen+1]);
				BYTE* dest = decoded;
				for (size_t i = 0; i < encLen; ++i)
				{
					BYTE c = s_tBase64Decode[source[i]];
					++i;
					BYTE c1 = s_tBase64Decode[source[i]];
					c = (c << 2) | ((c1 >> 4) & 0x3);
					*dest++ = c;
					if (++i < encLen)
					{
						c = source[i];
						if ('=' == c) // fillchar
							break;

						c = s_tBase64Decode[source[i]];
						c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
						*dest++ = c1;
					}

					if (++i < encLen)
					{
						c1 = source[i];
						if ('=' == c1) // fillchar
							break;

						c1 = s_tBase64Decode[source[i]];
						c = ((c << 6) & 0xc0) | c1;
						*dest++ = c;
					}
				}
				TMatrix3x3f tTrans = myStyle.xform;
				if (x != 0 || y != 0)
				{
					TMatrix3x3f tOffset = TMATRIX3X3F_IDENTITY;
					tOffset._31 = x;
					tOffset._32 = y;
					Matrix3x3fMultiply(tOffset, myStyle.xform, &tTrans);
				}
				tree.AddRasterImage(bstrID, enabled, pEffect, decoded, dest-decoded.m_p, tTrans);
			}
			//std::string url;
			//bool bParsed = parse(href.c_str(), href.c_str()+href.length(),
			//	*space_p>>str_p("url")>>*space_p>>ch_p('(')>>*space_p>>
			//	*ch_p('\'')>>*ch_p('\"')>>ch_p('#')>>(alpha_p>>*alnum_p)[assign_a(url)]>>*ch_p('\"')>>*ch_p('\'')>>
			//	*space_p>>ch_p(')')>>*space_p).full;
			//if (!url.empty())
		}
		else if (strcmp(name, "use") == 0)
		{
			rapidxml::xml_attribute<>* href = node->first_attribute("xlink:href");
			if (href && href->value()[0] == '#')
			{
				rapidxml::xml_node<>* sub = findNodeByID(xmldoc, href->value()+1);
				if (sub)
				{
					addNode(myStyle, classes, canvasSize, tree, pUtils, sub);
				}
			}
		}
		else if (strcmp(name, "a") == 0 || strcmp(name, "svg") == 0)
		{
			rapidxml::xml_node<>* sub = node->first_node();
			while (sub)
			{
				addNode(myStyle, classes, canvasSize, tree, pUtils, sub);
				sub = sub->next_sibling();
			}
		}
		else if (strcmp(name, "g") == 0)
		{
			rapidxml::xml_attribute<>* gm = node->first_attribute("inkscape:groupmode");
			CNodeTree& subTree = tree.AddGroup(bstrID, enabled, pEffect, gm && strcmp("layer", gm->value()) == 0);
			rapidxml::xml_node<>* sub = node->first_node();
			while (sub)
			{
				addNode(myStyle, classes, canvasSize, subTree, pUtils, sub);
				sub = sub->next_sibling();
			}
		}
		return S_OK;
		//return tree.AddVectorObject(NULL, CComBSTR(L"SHAPE"), bstrPath, CComBSTR(L"SOLID"), CComBSTR(szColor), NULL, NULL, NULL, NULL);
	}

	static void DecodeFontFamily(std::string const& src, std::string& dst)
	{
		if (src.empty())
		{
			dst = "Times New Roman";
			return;
		}
		std::vector<std::string> fonts;
		std::string tmp;
		rule<scanner<char const*> > item = *space_p>>(
					((*(~ch_p(',')))[assign_a(tmp)])[push_back_a(fonts, tmp)]|
					(ch_p('\"')>>(*(~ch_p('\"')))[assign_a(tmp)]>>ch_p('\"'))[push_back_a(fonts, tmp)]
					)>>*space_p;
		char const* pBeg = src.c_str();
		char const* pEnd = src.c_str()+src.length();
		bool bParsed = parse(pBeg, pEnd, list_p(item, ch_p(','))).full;
		if (fonts.empty())
		{
			dst = "Times New Roman";
			return;
		}
		dst = fonts[0];
		dst.erase(dst.find_last_not_of(" \t") + 1);
		if (dst == "serif" || dst == "ui-serif")
			dst = "Times New Roman";
		else if (dst == "sans-serif")
			dst = "Arial";
		else if (dst == "ui-sans-serif" || dst == "system-ui")
			dst = "Segoe UI";
		else if (dst == "monospace" || dst == "ui-monospace")
			dst = "Courier";
		else if (dst == "cursive" || dst == "ui-monospace")
			dst = "Comic Sans";
	}

	struct SNamedColor
	{
		char const* name;
		DWORD code;
	};
	static DWORD const NONAMEDCOLOR = 0x1000000;
	static DWORD FromNamedColor(char const* name)
	{
		static SNamedColor const s_namedColors[] =
		{
			{"aliceblue", 0xf0f8ff},
			{"antiquewhite", 0xfaebd7},
			{"aqua", 0x00ffff},
			{"aquamarine", 0x7fffd4},
			{"azure", 0xf0ffff},
			{"beige", 0xf5f5dc},
			{"bisque", 0xffe4c4},
			{"black", 0x000000},
			{"blanchedalmond", 0xffebcd},
			{"blue", 0x0000ff},
			{"blueviolet", 0x8a2be2},
			{"brown", 0xa52a2a},
			{"burlywood", 0xdeb887},
			{"cadetblue", 0x5f9ea0},
			{"chartreuse", 0x7fff00},
			{"chocolate", 0xd2691e},
			{"coral", 0xff7f50},
			{"cornflowerblue", 0x6495ed},
			{"cornsilk", 0xfff8dc},
			{"crimson", 0xdc143c},
			{"cyan", 0x00ffff},
			{"darkblue", 0x00008b},
			{"darkcyan", 0x008b8b},
			{"darkgoldenrod", 0xb8860b},
			{"darkgray", 0xa9a9a9},
			{"darkgreen", 0x006400},
			{"darkkhaki", 0xbdb76b},
			{"darkmagenta", 0x8b008b},
			{"darkolivegreen", 0x556b2f},
			{"darkorange", 0xff8c00},
			{"darkorchid", 0x9932cc},
			{"darkred", 0x8b0000},
			{"darksalmon", 0xe9967a},
			{"darkseagreen", 0x8fbc8f},
			{"darkslateblue", 0x483d8b},
			{"darkslategray", 0x2f4f4f},
			{"darkturquoise", 0x00ced1},
			{"darkviolet", 0x9400d3},
			{"deeppink", 0xff1493},
			{"deepskyblue", 0x00bfff},
			{"dimgray", 0x696969},
			{"dodgerblue", 0x1e90ff},
			{"firebrick", 0xb22222},
			{"floralwhite", 0xfffaf0},
			{"forestgreen", 0x228b22},
			{"fuchsia", 0xff00ff},
			{"gainsboro", 0xdcdcdc},
			{"ghostwhite", 0xf8f8ff},
			{"gold", 0xffd700},
			{"goldenrod", 0xdaa520},
			{"gray", 0x808080},
			{"grey", 0x808080},
			{"green", 0x008000},
			{"greenyellow", 0xadff2f},
			{"honeydew", 0xf0fff0},
			{"hotpink", 0xff69b4},
			{"indianred ", 0xcd5c5c},
			{"indigo", 0x4b0082},
			{"ivory", 0xfffff0},
			{"khaki", 0xf0e68c},
			{"lavender", 0xe6e6fa},
			{"lavenderblush", 0xfff0f5},
			{"lawngreen", 0x7cfc00},
			{"lemonchiffon", 0xfffacd},
			{"lightblue", 0xadd8e6},
			{"lightcoral", 0xf08080},
			{"lightcyan", 0xe0ffff},
			{"lightgoldenrodyellow", 0xfafad2},
			{"lightgrey", 0xd3d3d3},
			{"lightgreen", 0x90ee90},
			{"lightpink", 0xffb6c1},
			{"lightsalmon", 0xffa07a},
			{"lightseagreen", 0x20b2aa},
			{"lightskyblue", 0x87cefa},
			{"lightslategray", 0x778899},
			{"lightsteelblue", 0xb0c4de},
			{"lightyellow", 0xffffe0},
			{"lime", 0x00ff00},
			{"limegreen", 0x32cd32},
			{"linen", 0xfaf0e6},
			{"magenta", 0xff00ff},
			{"maroon", 0x800000},
			{"mediumaquamarine", 0x66cdaa},
			{"mediumblue", 0x0000cd},
			{"mediumorchid", 0xba55d3},
			{"mediumpurple", 0x9370d8},
			{"mediumseagreen", 0x3cb371},
			{"mediumslateblue", 0x7b68ee},
			{"mediumspringgreen", 0x00fa9a},
			{"mediumturquoise", 0x48d1cc},
			{"mediumvioletred", 0xc71585},
			{"midnightblue", 0x191970},
			{"mintcream", 0xf5fffa},
			{"mistyrose", 0xffe4e1},
			{"moccasin", 0xffe4b5},
			{"navajowhite", 0xffdead},
			{"navy", 0x000080},
			{"oldlace", 0xfdf5e6},
			{"olive", 0x808000},
			{"olivedrab", 0x6b8e23},
			{"orange", 0xffa500},
			{"orangered", 0xff4500},
			{"orchid", 0xda70d6},
			{"palegoldenrod", 0xeee8aa},
			{"palegreen", 0x98fb98},
			{"paleturquoise", 0xafeeee},
			{"palevioletred", 0xd87093},
			{"papayawhip", 0xffefd5},
			{"peachpuff", 0xffdab9},
			{"peru", 0xcd853f},
			{"pink", 0xffc0cb},
			{"plum", 0xdda0dd},
			{"powderblue", 0xb0e0e6},
			{"purple", 0x800080},
			{"red", 0xff0000},
			{"rosybrown", 0xbc8f8f},
			{"royalblue", 0x4169e1},
			{"saddlebrown", 0x8b4513},
			{"salmon", 0xfa8072},
			{"sandybrown", 0xf4a460},
			{"seagreen", 0x2e8b57},
			{"seashell", 0xfff5ee},
			{"sienna", 0xa0522d},
			{"silver", 0xc0c0c0},
			{"skyblue", 0x87ceeb},
			{"slateblue", 0x6a5acd},
			{"slategray", 0x708090},
			{"snow", 0xfffafa},
			{"springgreen", 0x00ff7f},
			{"steelblue", 0x4682b4},
			{"tan", 0xd2b48c},
			{"teal", 0x008080},
			{"thistle", 0xd8bfd8},
			{"tomato", 0xff6347},
			{"turquoise", 0x40e0d0},
			{"violet", 0xee82ee},
			{"wheat", 0xf5deb3},
			{"white", 0xffffff},
			{"whitesmoke", 0xf5f5f5},
			{"yellow", 0xffff00},
			{"yellowgreen", 0x9acd32},
		};
		for (SNamedColor const* i = s_namedColors; i != s_namedColors+itemsof(s_namedColors); ++i)
		{
			if (_stricmp(name, i->name) == 0)
				return i->code;
		}
		return NONAMEDCOLOR;
	}
};

OBJECT_ENTRY_AUTO(CLSID_DocumentDecoderSVG, CDocumentDecoderSVG)
