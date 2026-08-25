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
  Asset_Familiar,

  Asset_Count,
};

enum asset_state
{
  AssetState_Unloaded,
  AssetState_Queued,
  AssetState_Loaded,
  AssetState_Lock = 0x10000,
  AssetState_StateMask = 0xFFF,
  AssetState_TypeMask = 0xF000,
};

struct wizard
{
  bitmap_id Wiz;
};

enum asset_tag_id
{
  Tag_Smoothness,
  Tag_Flatness,
  Tag_Facing_Direction, //Angle in radians

  Tag_Count,
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
  uint32 FirstSampleIndex;
  uint32 SampleCount;
  sound_id NextIDToPlay;
};

struct asset_group
{
  uint32 FirstTagIndex;
  uint32 OnePastLastTagIndex;
};
 
struct asset_type
{
  uint32 FirstAssetIndex;
  uint32 OnePastLastAssetIndex;
};

struct asset_file
{
  platform_file_handle *Handle;
  hha_header Header;
  hha_asset_type *AssetTypeArray;
  uint32 TagBase;
};

struct asset_memory_header
{
  asset_memory_header *Next;
  asset_memory_header *Prev;

  uint32 AssetIndex;
  uint32 TotalSize;
  union
  {
    loaded_bitmap Bitmap;
    loaded_sound Sound;
  };  
};

struct asset
{
  uint32 State;
  asset_memory_header *Header;
  
  hha_asset HHA;
  uint32 FileIndex;
};



struct game_assets
{
  memory_arena Arena;
  struct transient_state *TranState;

  uint64 TargetMemoryUsed;
  uint64 TotalMemoryUsed;
  asset_memory_header LoadedAssetSentinel;
  
  asset_type AssetTypes[Asset_Count];

  real32 TagRange[Tag_Count];

  uint32 FileCount;
  asset_file *Files;
  
  uint32 AssetCounts;
  asset *Assets;

  uint32 TagCounts;
  hha_tag *Tags;
  
  //wizard Wizard;
#if 0
  uint8 *HHAContents;

  uint32 DEBUGAssetCount;
  uint32 DEBUGTagCount;
  asset_type *DEBUGAssetType;
  asset *DEBUGAsset;
#endif  
};

inline bool32 IsLocked(asset *Asset)
{
  bool32 Result = (Asset->State & AssetState_Lock);
  return Result;
}

inline uint32 GetType(asset *Asset)
{
  uint32 Result = Asset->State & AssetState_TypeMask;
  return Result;
}

inline uint32 GetState(asset *Asset)
{
  uint32 Result = Asset->State & AssetState_StateMask;
  return Result;
}

void MoveHeaderToFront(game_assets *Assets, asset *Asset);

inline loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID, bool32 MustBeLocked)
{
  asset *Asset = Assets->Assets + ID.Value;
  loaded_bitmap *Result = 0;
  if(GetState(Asset) >= AssetState_Loaded && (!MustBeLocked || IsLocked(Asset)))
  {
    CompletePreviousReadsBeforeFutureReads;
    Result = &Asset->Header->Bitmap;
    MoveHeaderToFront(Assets, Asset);
  }
  return Result;
}

inline loaded_sound* GetSound(game_assets* Assets, sound_id ID)
{
  asset *Asset = Assets->Assets + ID.Value;
  loaded_sound *Result = 0;
  if(GetState(Asset) >= AssetState_Loaded)
  {
    CompletePreviousReadsBeforeFutureReads;
    Result = &Asset->Header->Sound;
    MoveHeaderToFront(Assets, Asset);
  }
  return Result;
}

inline hha_sound* GetSoundInfo(game_assets* Assets, sound_id ID)
{
  //Assert(ID.Value < Assets->SoundCounts);
  hha_sound* Info = &Assets->Assets[ID.Value].HHA.Sound;

  return Info;
}


inline bool32 IsValid(bitmap_id ID)
{
  bool32 Result = (ID.Value != 0);

  return Result;
}

inline bool32 IsValid(sound_id ID)
{
  bool32 Result = (ID.Value != 0);

  return Result;
}

inline sound_id  GetNextSoundInChain(game_assets *Assets, sound_id ID)
{
  sound_id Result = {};
  hha_sound *Info = GetSoundInfo(Assets, ID);
  switch(Info->Chain)
  {
    case HHASoundChain_None:
    {
      //Nothing to do
    } break;
    
    case HHASoundChain_Loop:
    {
      Result = ID;
    } break;
    
    case HHASoundChain_Advance:
    {
      Result.Value = ID.Value + 1;
    } break;

    default:
    {
      
    }
  }

  return Result;
}

internal void LoadSound(game_assets *Assets, sound_id ID);
internal void LoadBitmap(game_assets *Assets, bitmap_id ID, bool32 Locked);
internal task_with_memory *BeginTaskWithMemory(transient_state *TranState);
internal void EndTaskWithMemory(task_with_memory *Task);

#define HANDMADE_ASSET_H
#endif
