// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/liquid_render.hpp>
#include <noggit/MapHeaders.h>

class MapChunk;
class sExtendableArray;


// handle liquids like oceans, lakes, rivers, slime, magma
class Liquid
{
public:

  Liquid(math::vector_3d const& base, MH2O_Information const& info, MH2O_HeightMask const& heightmask, std::uint64_t infomask);


  void save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const;

  void draw() { render->draw(); }
  void updateRender();
  void changeLiquidID(int id);
  
  void crop(MapChunk* chunk);
  void updateTransparency(MapChunk* chunk, float factor);


  float min() const { return _minimum; }
  float max() const { return _maximum; }
  int liquidID() const { return _liquid_id; }

  bool hasSubchunk(int x, int z) const;
  void setSubchunk(int x, int z, bool water);

private:
  int _liquid_id;
  int _flags;
  float _minimum;
  float _maximum;
  std::uint64_t _subchunks;
  std::vector<math::vector_3d> _vertices;
  std::vector<float> _depth;

private:
  bool mTransparency;  
  int xtiles, ytiles;
  math::vector_3d pos;
  float texRepeats;

  std::unique_ptr<liquid_render> render;
};