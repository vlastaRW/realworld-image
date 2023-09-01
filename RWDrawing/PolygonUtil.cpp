
#include "stdafx.h"
#include "RWDrawing.h"
#include "../../realworld-core/clipper/clipper.hpp"
#undef min
#undef max
#include "../../realworld-core/clipper/clipper.cpp"

bool SplitPolygon(std::vector<TPixelCoords> const& a_cInput, std::vector<std::vector<TPixelCoords> >& a_cOutput)
{
	ClipperLib::Path cSrc;
	cSrc.reserve(a_cInput.size());
	for (std::vector<TPixelCoords>::const_iterator j = a_cInput.begin(); j != a_cInput.end(); ++j)
	{
		ClipperLib::IntPoint pt(j->fX*1000.0+0.5, j->fY*1000.0+0.5);
		cSrc.push_back(pt);
	}

	ClipperLib::Paths cDst;
	ClipperLib::SimplifyPolygon(cSrc, cDst);

	a_cOutput.resize(cDst.size());
	for (ClipperLib::Paths::const_iterator i = cDst.begin(); i != cDst.end(); ++i)
	{
		std::vector<TPixelCoords>& cOut1 = a_cOutput[i-cDst.begin()];
		cOut1.reserve(i->size());
		for (ClipperLib::Path::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			TPixelCoords pt = {j->X*0.001, j->Y*0.001};
			cOut1.push_back(pt);
		}
	}

	return true;
}

bool SplitPolygons(std::vector<std::vector<TPixelCoords> > const& a_cInput, std::vector<std::vector<TPixelCoords> >& a_cOutput, bool a_nonZero)
{
	ClipperLib::Paths cSrc;
	cSrc.resize(a_cInput.size());
	for (std::vector<std::vector<TPixelCoords> >::const_iterator i = a_cInput.begin(); i != a_cInput.end(); ++i)
	{
		ClipperLib::Path& cSrc1 = cSrc[i-a_cInput.begin()];
		cSrc1.reserve(i->size());
		for (std::vector<TPixelCoords>::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			ClipperLib::IntPoint pt(j->fX*1024.0+0.5, j->fY*1024.0+0.5);
			cSrc1.push_back(pt);
		}
	}

	ClipperLib::Paths cDst;
	ClipperLib::SimplifyPolygons(cSrc, cDst, a_nonZero ? ClipperLib::pftNonZero : ClipperLib::pftEvenOdd);

	a_cOutput.resize(cDst.size());
	for (ClipperLib::Paths::const_iterator i = cDst.begin(); i != cDst.end(); ++i)
	{
		std::vector<TPixelCoords>& cOut1 = a_cOutput[i-cDst.begin()];
		cOut1.reserve(i->size());
		for (ClipperLib::Path::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			TPixelCoords pt = {j->X*0.0009765625, j->Y*0.0009765625};
			cOut1.push_back(pt);
		}
	}

	return true;
}

bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, EOutlineJoinType a_eJoinType, std::vector<std::vector<TPixelCoords> >& a_cOutput)
{
	ClipperLib::Paths cSrc;
	cSrc.resize(a_cInput.size());
	for (std::vector<std::vector<TPixelCoords> >::const_iterator i = a_cInput.begin(); i != a_cInput.end(); ++i)
	{
		ClipperLib::Path& cSrc1 = cSrc[i-a_cInput.begin()];
		cSrc1.reserve(i->size());
		for (std::vector<TPixelCoords>::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			ClipperLib::IntPoint pt(j->fX*1000.0+0.5, j->fY*1000.0+0.5);
			cSrc1.push_back(pt);
		}
	}

	ClipperLib::JoinType jt = a_eJoinType == EOJTMiter ? ClipperLib::jtMiter : (a_eJoinType == EOJTBevel ? ClipperLib::jtSquare : ClipperLib::jtRound);
	ClipperLib::Paths cDst;
	ClipperLib::ClipperOffset cOff(4.0f);
	cOff.AddPaths(cSrc, jt, ClipperLib::etClosedPolygon);
	cOff.Execute(cDst, -1000.0*a_fAmount);
	//ClipperLib::OffsetPolygons(cSrc, cDst, -1000.0*a_fAmount, ClipperLib::jtRound);

	a_cOutput.resize(cDst.size());
	for (ClipperLib::Paths::const_iterator i = cDst.begin(); i != cDst.end(); ++i)
	{
		std::vector<TPixelCoords>& cOut1 = a_cOutput[i-cDst.begin()];
		cOut1.reserve(i->size());
		for (ClipperLib::Path::const_iterator j = i->begin(); j != i->end(); ++j)
		{
			TPixelCoords pt = {j->X*0.001, j->Y*0.001};
			cOut1.push_back(pt);
		}
	}
	return true;
}

bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, std::vector<std::vector<TPixelCoords> >& a_cOutput)
{
	return ShrinkPolygon(a_cInput, a_fAmount, EOJTRound, a_cOutput);
}

