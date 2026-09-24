#if !defined(HANDMADE_FILE_FORMATS_H)
#define MAX_FONT_CODEPOINT_COUNT (0x10FFFF + 1)

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef size_t memory_index;

typedef int32 bool32;

typedef float real32;
typedef double real64;

enum asset_type_id
{
  Asset_None,
  
  Asset_BackGround,
  Asset_Wall,
  Asset_Monster,
  Asset_Sword,
  Asset_Staff,
  Asset_Stair,
  Asset_Grass,
  Asset_Dirt,
  Asset_Wizard,
  Asset_FireSound,
  Asset_DungeonSound,
  Asset_Familiar,
  Asset_Fonts,
  Asset_FontGlyph,

  Asset_Count,
};

enum asset_tag_id
{
  Tag_Smoothness,
  Tag_Flatness,
  Tag_Facing_Direction, //Angle in radians
  Tag_UTFCodePoint,
  
  Tag_Count,
};

#define HHA_CODE(a, b, c, d)(((uint32)(a) << 0)  | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))

#pragma pack(push, 1)

struct hha_header
{
#define HHA_HEADER_CODE HHA_CODE('h', 'h', 'a', 'f')
  uint32 HeaderCode;

#define HHA_VERSION 0
  uint32 Version;

  uint32 TagCount;
  uint32 AssetCount;
  uint32 AssetTypeCount;

  uint64 TagsOffset;
  uint64 AssetOffset;
  uint64 AssetTypeOffset;
};
#pragma pack(pop)

struct sound_id
{
  uint32 Value;
};

struct bitmap_id
{
  uint32 Value;
};

struct font_id
{
  uint32 Value;
};

struct hha_tag
{
  uint32 ID;
  real32 Value;
};

struct hha_bitmap
{
  uint32 Dim[2];
  real32 AlignPercentage[2];
};

enum hha_sound_chain
{
  HHASoundChain_None,
  HHASoundChain_Loop,
  HHASoundChain_Advance,  
};

struct hha_sound
{
  uint32 SampleCount;
  uint32 Chain;
  uint32 ChannelCount;
};

struct hha_font
{
  uint32 GlyphCount;
  uint32 OnePastHighestCodepoint;
  real32 ExternalLeading;
  real32 AscenderHeight;
  real32 DescenderHeight;
};

struct hha_asset
{
  uint32 FirstTagIndex;
  uint32 OneLastPastTagIndex;
  uint64 DataOffset;
  union
  {
    hha_bitmap Bitmap;
    hha_sound Sound;
    hha_font Font;
  };
};

struct hha_asset_type
{
  uint32 TypeID;
  uint32 FirstAssetIndex;
  uint32 OnePastLastAssetIndex;
};

struct hha_font_glyph
{
  uint32 UnicodeCodePoint;
  bitmap_id BitmapID;
};

#define HANDMADE_FILE_FORMATS_H
#endif
