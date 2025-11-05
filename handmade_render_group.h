#if !defined(HANDMADE_RENDER_GROUP_H)
struct loaded_bitmap
{
  int32 AlignX, AlignY;
  
  int32 Width;
  int32 Height;
  void *Memory;
  int32 Pitch;
};

struct environment_map
{
  loaded_bitmap LOD[4];
  real32 Pz;
};

struct render_basis
{
  v3 P;
};

enum render_group_entry_type
{
  RenderGroupEntryType_render_entry_clear,
  RenderGroupEntryType_render_entry_bitmap,
  RenderGroupEntryType_render_entry_rectangle,
  RenderGroupEntryType_render_entry_coordinate_system,
};

struct render_entity_basis
{
  render_basis *Basis;
  v3 Offset;
};

struct render_group_entry_header
{
  render_group_entry_type Type;
};

struct render_entry_clear
{
  v4 Color;
};

struct render_entry_bitmap
{
  loaded_bitmap *Bitmap;
  render_entity_basis EntityBasis; 
  v4 Color;
};

struct render_entry_rectangle
{
  render_entity_basis EntityBasis;
  v4 Color;
  v2 Dim;
};

struct render_entry_coordinate_system
{

  v2 Origin;
  v2 XAxis;
  v2 YAxis;
  v4 Color;
  loaded_bitmap *Texture;
  loaded_bitmap *NormalMap;

  environment_map *Top;
  environment_map *Middle;
  environment_map *Bottom;
  
  v2 Point;
};

struct render_group
{
  real32 GlobalAlpha;
  
  render_basis *DefaultBasis;
  real32 MetersToPixel;

  uint32 MaxPushBufferSize;
  uint32 PushBufferSize;
  uint8 *PushBufferBase;
};

#define HANDMADE_RENDER_GROUP_H
#endif
