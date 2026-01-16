#if !defined(HANDMADE_RENDER_GROUP_H)
//All the coordinates passed to the render must be specified in world coordinate(meters) and not in pixels

struct loaded_bitmap
{
  v2 AlignPercentage;
  real32 WidthOverHeight;
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

enum render_group_entry_type
{
  RenderGroupEntryType_render_entry_clear,
  RenderGroupEntryType_render_entry_bitmap,
  RenderGroupEntryType_render_entry_rectangle,
  RenderGroupEntryType_render_entry_coordinate_system,
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
  v2 P;
  v2 Size;
  v4 Color;
};

struct render_entry_rectangle
{
  v2 P;
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

struct render_transform
{
  bool32 Orthographic;
  
  real32 FocalLength;
  real32 CameraDistanceAboveTarget;
  
  real32 MetersToPixels;
  v2 ScreenCenter;
  v3 OffsetP;
  real32 Scale;
};

struct render_group
{
  struct game_assets *Assets;
  
  real32 GlobalAlpha;
  
  render_transform Transform;
  
  v2 MonitorHalfDimInMeters;

  uint32 MaxPushBufferSize;
  uint32 PushBufferSize;
  uint8 *PushBufferBase;

  uint32 MissingBitmapCounts;
};

#define HANDMADE_RENDER_GROUP_H
#endif
