#if !defined(HANDMADE_ASSET_H)
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
  Tag_Facing_Direction, //Angle in radians

  Tag_Count,
};

struct asset_slot
{
  asset_state State;
  union
  {
    loaded_bitmap *Bitmap;
    loaded_sound *Sound;
  };
};

struct asset_vector
{
  real32 E[Tag_Count];
};

struct asset_tag
{
  uint32 ID;
  real32 Value;
};

struct asset_bitmap_info
{
  v2 AlignPercentage;
  char *FileName;
};

struct asset_sound_info
{
  char* FileName;
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

  real32 TagRange[Tag_Count];
  
  uint32 AssetCounts;
  asset *Assets;

  uint32 TagCounts;
  asset_tag *Tags;
  
  uint32 BitmapCounts;
  asset_slot *Bitmaps;
  asset_bitmap_info *BitmapInfos;
  
  uint32 SoundCounts;
  asset_sound_info *SoundInfos;
  asset_slot *Sounds;
  
  //wizard Wizard;

  uint32 DEBUGBitmapCount;
  uint32 DEBUGAssetCount;
  uint32 DEBUGTagCount;
  uint32 DEBUGSoundCount;
  asset_type *DEBUGAssetType;
  asset *DEBUGAsset;
};

struct bitmap_id
{
  uint32 Value;
};

struct wizard
{
  bitmap_id Wiz;
};

struct sound_id
{
  uint32 Value;
};

inline loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID)
{
  loaded_bitmap *Result = Assets->Bitmaps[ID.Value].Bitmap;

  return Result;
}

inline loaded_sound* GetSound(game_assets* Assets, sound_id ID)
{
    loaded_sound* Result = Assets->Sounds[ID.Value].Sound;

    return Result;
}

internal void LaodBitmap(game_assets *Assets, bitmap_id ID);
internal void LaodSound(game_assets *Assets, sound_id ID); 
internal task_with_memory *BeginTaskWithMemory(transient_state *TranState);
internal void EndTaskWithMemory(task_with_memory *Task);


#define HANDMADE_ASSET_H
#endif
