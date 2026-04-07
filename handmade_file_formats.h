#if !defined(HANDMADE_FILE_FORMATS_H)

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

struct hha_sound
{
  uint32 SampleCount;
  uint32 NextIDToPlay;
  uint32 ChannelCount;
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
  };
};

struct hha_asset_type
{
  uint32 TypeID;
  uint32 FirstAssetIndex;
  uint32 OnePastLastAssetIndex;
};

#pragma pack(pop)


#define HANDMADE_FILE_FORMATS_H
#endif
