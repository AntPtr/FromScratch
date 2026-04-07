#if !defined(HANDMADE_TEST_ASSET_BUILDER_H)
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include "handmade_asset_type_id.h"
#include "handmade_file_formats.h"
#include "handmade_intrisic.h"
#include "handmade_math.h"

#define internal static
#define Align8(value) ((value + 7) & ~7)

#define Assert(Expr) if(!(Expr)) {*(int *)0=0;}

struct bitmap_id
{
  uint32 Value;
};

struct sound_id
{
  uint32 Value;
};

enum asset_type
{
  AssetType_Sound,
  AssetType_Bitmap,
};

struct asset_source
{
  asset_type Type;
  char* FileName;
  uint32 FirstSampleIndex;
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
