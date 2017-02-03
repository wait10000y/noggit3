// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp> // math::vector_4d
#include <noggit/MapTile.h> // MapTile
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/Video.h> // GLuint
#include <noggit/WMOInstance.h>
#include <opengl/texture.hpp>

class MPQFile;
namespace math
{
  struct vector_4d;
}
class Brush;
class Alphamap;
class TextureSet;
class sExtendableArray;
class Frustum;

using StripType = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size

class MapChunk
{
private:
  float r;

  bool mBigAlpha;
  bool hasMCCV;

  int holes;

  unsigned int areaID;

  unsigned char mShadowMap[8 * 64];
  opengl::texture shadow;

  StripType *strip_with_holes;
  int strip_with_holeslen;

  bool water;

  math::vector_3d mNormals[mapbufsize];
  math::vector_3d mMinimap[mapbufsize];
  math::vector_4d mFakeShadows[mapbufsize];
  math::vector_3d mccv[mapbufsize];

  void initStrip();

  int indexNoLoD(int x, int y);
  int indexLoD(int x, int y);

public:
  MapChunk(MapTile* mt, MPQFile* f, bool bigAlpha);
  ~MapChunk();

  MapTile *mt;
  math::vector_3d vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;

  float xbase, ybase, zbase;

  unsigned int Flags;


  TextureSet* textureSet;

  GLuint vertices, normals, indices, minimap, minishadows, mccvEntry;

  math::vector_3d mVertices[mapbufsize];

  void draw ( Frustum const&
            , bool highlightPaintableChunks
            , bool draw_contour
            );
  //! \todo only this function should be public, all others should be called from it

  void drawContour();
  void intersect (math::ray const&, selection_result*);
  void drawLines (Frustum const&);
  void drawTextures();
  bool ChangeMCCV(math::vector_3d const& pos, float change, float radius, bool editMode);
  void SetWater(bool w);
  bool GetWater();

  void updateVerticesData();
  void recalcNorms();

  //! \todo implement Action stack for these
  bool changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  bool flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, int flattenType, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  bool blurTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType);

  void selectVertex(math::vector_3d const& pos, float radius, std::set<math::vector_3d*>& vertices);
  void fixVertices(std::set<math::vector_3d*>& selected);
  // for the vertex tool
  bool isBorderChunk(std::set<math::vector_3d*>& selected);

  //! \todo implement Action stack for these
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool canPaintTexture(scoped_blp_texture_reference texture);
  int addTexture(scoped_blp_texture_reference texture);
  void switchTexture(scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture);
  void eraseTextures();

  //! \todo implement Action stack for these
  bool isHole(int i, int j);
  void setHole(math::vector_3d const& pos, bool big, bool add);

  void setFlag(bool value);
  int getFlag();

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x, float z, math::vector_3d *V);
  float getHeight(int x, int z);
  float getMinHeight();

  void clearHeight();

  //! \todo this is ugly create a build struct or sth
  void save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::map<int, WMOInstance> &lObjectInstances, std::map<int, ModelInstance> &lModelInstances);

  // fix the gaps with the chunk to the left
  bool fixGapLeft(const MapChunk* chunk);
  // fix the gaps with the chunk above
  bool fixGapAbove(const MapChunk* chunk);

  void toBigAlpha(){ mBigAlpha = true;}
};