// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <opengl/call_list.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>


Liquid::Liquid(math::vector_3d const& base, MH2O_Information const& info, MH2O_HeightMask const& heightmask, std::uint64_t infomask)
  : pos(base)
  , texRepeats(4.0f)
  , _flags(info.Flags)
  , _minimum(info.minHeight)
  , _maximum(info.maxHeight)
  , _liquid_id(info.LiquidType)
  , _subchunks(0)
  , render(new liquid_render(true))
{
  int offset = 0;
  for (int z = 0; z < info.height; ++z)
  {
    for (int x = 0; x < info.width; ++x)
    {
      setSubchunk(x + info.xOffset, z + info.yOffset, (infomask >> offset) & 1);
      offset++;
    }
  }

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      _depth.emplace_back(heightmask.mTransparency[z][x]/255.0f);
      _vertices.emplace_back( pos.x + LQ_DEFAULT_TILESIZE * x
                            , heightmask.mHeightValues[z][x]
                            , pos.z + LQ_DEFAULT_TILESIZE * z
                            );
    }
  }
  
  changeLiquidID(_liquid_id);
  updateRender();
}

void Liquid::save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const
{
  int min_x = 9, min_z = 9, max_x = 0, max_z = 0;
  bool filled = true;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (hasSubchunk(x, z))
      {
        min_x = std::min(x, min_x);
        min_z = std::min(z, min_z);
        max_x = std::max(x + 1, max_x);
        max_z = std::max(z + 1, max_z);
      }
      else
      {
        filled = false;
      }
    }
  }

  MH2O_Information info;
  std::uint64_t mask = 0;

  info.LiquidType = _liquid_id;
  info.Flags = _flags;
  info.minHeight = _minimum;
  info.maxHeight = _maximum;
  info.xOffset = min_x;
  info.yOffset = min_z;
  info.width = max_x - min_x;
  info.height = max_z - min_z;

  if (filled)
  {
    info.ofsInfoMask = 0;
  }
  else
  {
    std::uint64_t value = 1;
    for (int z = info.yOffset; z < info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x < info.xOffset + info.width; ++x)
      {
        if (hasSubchunk(x, z))
        {
          mask |= value;
        }
        value <<= 1;
      }
    }

    if (mask > 0)
    {
      info.ofsInfoMask = current_pos - base_pos;
      adt.Insert(current_pos, 8, reinterpret_cast<char*>(&mask));
      current_pos += 8;
    }
  } 

  info.ofsHeightMap = current_pos - base_pos;

  std::size_t heighmap_size = (info.width + 1) * (info.height +1) * (sizeof(char) + ((_liquid_id != 2) ? sizeof(float) : 0));
  adt.Extend(heighmap_size);

  if (_liquid_id != 2)
  {
    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        memcpy(adt.GetPointer<char>(current_pos), &_vertices[z * 9 + x].y, sizeof(float));
        current_pos += sizeof(float);
      }
    }
  }

  for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
  {
    for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
    {
      unsigned char depth = static_cast<unsigned char>(_depth[z * 9 + x] * 255.0f);
      memcpy(adt.GetPointer<char>(current_pos), &depth, sizeof(char));
      current_pos += sizeof(char);
    }
  }

  memcpy(adt.GetPointer<char>(info_pos), &info, sizeof(MH2O_Information));
  info_pos += sizeof(MH2O_Information);
}


void Liquid::changeLiquidID(int id)
{
  _liquid_id = id;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(_liquid_id);
    render->setTextures(lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames - 1));
    mTransparency = lLiquidTypeRow.getInt(LiquidTypeDB::ShaderType) & 1;
    //! \todo  Get texRepeats too.
  }
  catch (...)
  {
    // Fallback, when there is no information.
    render->setTextures("XTextures\\river\\lake_a.%d.blp");
    mTransparency = true;
  }
}

void Liquid::updateRender()
{
  opengl::call_list* draw_list = new opengl::call_list();
  draw_list->start_recording();

  gl.begin(GL_QUADS);

  gl.normal3f(0.0f, 1.0f, 0.0f);

  // draw tiles
  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      
      if (!hasSubchunk(i, j))
      {
        continue;
      }

      int index = j * 9 + i;

      float c;
      c = _depth[index];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, j / texRepeats);
      gl.vertex3fv(_vertices[index]);

      c = _depth[index+1];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, j / texRepeats);
      gl.vertex3fv(_vertices[index + 1]);

      c = _depth[index + 10];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f((i + 1) / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(_vertices[index + 10]);

      c = _depth[index + 9];
      glMultiTexCoord2f(GL_TEXTURE1, c, c);
      gl.texCoord2f(i / texRepeats, (j + 1) / texRepeats);
      gl.vertex3fv(_vertices[index + 9]);
    }
  }

  gl.end();

  draw_list->end_recording();

  render->changeDrawList(draw_list);
}


void Liquid::crop(MapChunk* chunk)
{
  bool changed = false;

  if (_maximum < chunk->getMinHeight())
  {
    _subchunks = 0;
    changed = true;
  }
  else
  {
    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (hasSubchunk(x, z))
        {
          int water_index = 9 * z + x, terrain_index = 17 * z + x;

          if (_vertices[water_index].y < chunk->mVertices[terrain_index].y
            && _vertices[water_index + 1].y < chunk->mVertices[terrain_index + 1].y
            && _vertices[water_index + 9].y < chunk->mVertices[terrain_index + 17].y
            && _vertices[water_index + 10].y < chunk->mVertices[terrain_index + 18].y
            )
          {
            setSubchunk(x, z, false);
            changed = true;
          }
        }
      }
    }
  }
  
  if (changed)
  {
    updateRender();
  }
}

void Liquid::updateTransparency(MapChunk* chunk, float factor)
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      float diff = _vertices[z * 9 + x].y - chunk->mVertices[z * 17 + x].y;
      _depth[z * 9 + x] = diff < 0.0f ? 0.0f : (std::min(1.0f, std::max(0.0f, (diff + 1.0f) * factor)));
    }
  }

  updateRender();
}

bool Liquid::hasSubchunk(int x, int z) const
{
  return (_subchunks >> (z * 8 + x)) & 1;
}

void Liquid::setSubchunk(int x, int z, bool water)
{
  std::uint64_t v = std::uint64_t(1) << (z*8+x);
  _subchunks = water ? (_subchunks | v) : (_subchunks & ~v);
}