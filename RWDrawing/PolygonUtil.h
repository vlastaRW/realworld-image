
#pragma once

bool SplitPolygon(std::vector<TPixelCoords> const& a_cInput, std::vector<std::vector<TPixelCoords> >& a_cOutput);
bool SplitPolygons(std::vector<std::vector<TPixelCoords> > const& a_cInput, std::vector<std::vector<TPixelCoords> >& a_cOutput, bool a_nonZero = false);

bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, std::vector<std::vector<TPixelCoords> >& a_cOutput);
bool ShrinkPolygon(std::vector<std::vector<TPixelCoords> > const& a_cInput, float a_fAmount, EOutlineJoinType a_eJoinType, std::vector<std::vector<TPixelCoords> >& a_cOutput);

