#if !defined(HANDMADE_ASSET_H)
struct wizard
{
  loaded_bitmap Wiz[2];
};

enum asset_type_id
{
  Asset_None,
  
  Asset_BackGround,
  Asset_Wall,
  Asset_Monster,
  Asset_Sword,
  Asset_Staff,
  Asset_Stair,

  Asset_Count,
};

enum asset_state
{
  AssetState_Unloaded,
  AssetState_Queued,
  AssetState_Loaded,
  AssetState_Locked,
};

enum asset_tag_id
{
  Tag_Smoothness,
  Tag_Flatness,

  Tag_Count,
};

struct asset_slot
{
  asset_state State;
  loaded_bitmap *Bitmap;
};

struct asset_tag
{
  uint32 ID;
  real32 Value;
};

struct asset_bitmap_info
{
  
  v2 AlignPercentage;
  real32 WidthOverHeight;
  int32 Width;
  int32 Height;

  uint32 FirstTagIndex;
  uint32 OnePastLastTagIndex;
};

struct asset_group
{
  uint32 FirstTagIndex;
  uint32 OnePastLastTagIndex;
};

struct asset
{
  uint32 FirstTagIndex;
  uint32 OneLastPastTagIndex;
  uint32 SlotID;
};

struct asset_type
{
  uint32 FirstAssetIndex;
  uint32 OnePastLastAssetIndex;
};

struct game_assets
{
  memory_arena Arena;
  struct transient_state *TranState;
  
  asset_type AssetTypes[Asset_Count];

  uint32 AssetCounts;
  asset *Assets;

  uint32 TagCounts;
  asset_tag *Tags;
  
  uint32 BitmapCounts;
  asset_slot *Bitmaps;

  uint32 SoundCounts;
  asset_slot *Sounds;
  
  loaded_bitmap Grass[2];
  loaded_bitmap Stones[2];
  wizard Wizard;
};

struct bitmap_id
{
  int32 Value;
};

struct audio_id
{
  int32 Value;
};

inline loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID)
{
  loaded_bitmap *Result = Assets->Bitmaps[ID.Value].Bitmap;

  return Result;
}

internal void LaodBitmap(game_assets *Assets, bitmap_id ID);
internal void LaodSound(game_assets *Assets, audio_id ID); 
internal task_with_memory *BeginTaskWithMemory(transient_state *TranState);
internal void EndTaskWithMemory(task_with_memory *Task);


#define HANDMADE_ASSET_H
#endif
