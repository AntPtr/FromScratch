#if !defined(HANDMADE_TEST_ASSET_BUILDER_H)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include "handmade_file_formats.h"
#include "handmade_intrisic.h"
#include "handmade_math.h"
#include <memory.h>

#define internal static
#define Align8(value) ((value + 7) & ~7)

#define Assert(Expr) if(!(Expr)) {*(int *)0=0;}
#define global_variable static

enum asset_type
{
  AssetType_Sound,
  AssetType_Bitmap,
  AssetType_Font,
  AssetType_FontGlyph,
};


struct loaded_font;
struct asset_source_font
{
  loaded_font *Font;
  char *FontName; 
};

struct asset_source_font_glyph
{
  loaded_font *Font;
  uint32 CodePoint;
};

struct asset_source_sound
{
  char *FileName;
  uint32 FirstSampleIndex;
};

struct asset_source_bitmap
{
  char *FileName;
};

struct asset_source
{
  asset_type Type;
  union
  {
    asset_source_bitmap Bitmap;
    asset_source_sound Sound;
    asset_source_font Font;
    asset_source_font_glyph Glyph;
  };
};

#define LARGE_NUMBER 4096
struct game_assets
{
  uint32 TagCount;
  hha_tag Tags[LARGE_NUMBER];
  
  uint32 AssetTypeCount;
  hha_asset_type AssetTypes[Asset_Count];

  uint32 AssetCount;
  asset_source AssetsSources[LARGE_NUMBER];
  hha_asset Assets[LARGE_NUMBER];
  
  hha_asset_type *DEBUGAssetType;
  uint32 AssetIndex;
};
#define HANDMADE_TEST_ASSET_BUILDER_H
#endif
