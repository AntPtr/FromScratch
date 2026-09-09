#if !defined(HANDMADE_ASSET_H)
/*
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
*/
enum asset_state
{
  AssetState_Unloaded,
  AssetState_Queued,
  AssetState_Loaded,
  AssetState_Operating,
  //AssetState_StateMask = 0xFFF,
  //AssetState_TypeMask = 0xF000,
};

struct wizard
{
  bitmap_id Wiz;
};

/*
enum asset_tag_id
{
  Tag_Smoothness,
  Tag_Flatness,
  Tag_Facing_Direction, //Angle in radians

  Tag_Count,
};
*/
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
  platform_file_handle Handle;
  hha_header Header;
  hha_asset_type *AssetTypeArray;
  uint32 TagBase;
};

struct asset_memory_header
{
  asset_memory_header *Next;
  asset_memory_header *Prev;

  uint32 GenerationID;
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

enum asset_memory_block_flags
{
  AssetMemory_Used = 0x1,
};

struct asset_memory_block
{
  asset_memory_block *Next;
  asset_memory_block *Prev;
  uint64 Flag;
  memory_index Size;
};

struct game_assets
{
  uint32 NextGenerationID;
  
  struct transient_state *TranState;
  asset_memory_block MemorySentinel;

  asset_memory_header LoadedAssetSentinel;
  
  asset_type AssetTypes[Asset_Count];

  real32 TagRange[Tag_Count];

  uint32 FileCount;
  asset_file *Files;
  
  uint32 AssetCounts;
  asset *Assets;

  uint32 TagCounts;
  hha_tag *Tags;

  uint32 OperationLock;

  uint32 InFlightGenerationCount;
  uint32 InFlightGenerations[16];
  //wizard Wizard;
#if 0
  uint8 *HHAContents;

  uint32 DEBUGAssetCount;
  uint32 DEBUGTagCount;
  asset_type *DEBUGAssetType;
  asset *DEBUGAsset;
#endif  
};

/*inline bool32 IsLocked(asset *Asset)
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
*/
void MoveHeaderToFront(game_assets *Assets, asset *Asset);

inline void BeginAssetLock(game_assets *Assets)
{
  for(;;)
  {
    if(AtomicCompareExchangeUInt32(&Assets->OperationLock, 1, 0) == 0)
    {
      break;
    }
  }
}

inline void EndAssetLock(game_assets *Assets)
{
  CompletePreviousWriteBeforeFutureWrites
  Assets->OperationLock = 0;
}

inline void InsertAssetHeaderAtFront(game_assets *Assets, asset_memory_header *Header)
{
  asset_memory_header *Sentinel = &Assets->LoadedAssetSentinel;

  Header->Next = Sentinel->Next;
  Header->Prev = Sentinel;
  
  Header->Next->Prev = Header;
  Header->Prev->Next = Header;
}

internal void RemoveAssetHeaderFromList(asset_memory_header *Header)
{
  Header->Prev->Next = Header->Next;
  Header->Next->Prev = Header->Prev;

  Header->Next = Header->Prev = 0;
}


inline asset_memory_header *GetAsset(game_assets *Assets, uint32 ID, uint32 GenerationID)
{
  asset *Asset = Assets->Assets + ID;
  asset_memory_header *Result = 0;

  BeginAssetLock(Assets);
 
  if(Asset->State == AssetState_Loaded)
  {
    Result = Asset->Header;
    RemoveAssetHeaderFromList(Result);
    InsertAssetHeaderAtFront(Assets, Result);

    
    if(Asset->Header->GenerationID < GenerationID)
    {
      Asset->Header->GenerationID = GenerationID;
    }
    
    CompletePreviousReadsBeforeFutureReads;      
  }
  
  EndAssetLock(Assets);     
  return Result;
}

inline loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID, uint32 GenerationID)
{
  asset_memory_header *Header = GetAsset(Assets, ID.Value, GenerationID);
  loaded_bitmap *Result = Header ? &Header->Bitmap : 0;
  return Result;
}

inline loaded_sound* GetSound(game_assets* Assets, sound_id ID, uint32 GenerationID)
{
  asset_memory_header *Header = GetAsset(Assets, ID.Value, GenerationID);
  loaded_sound *Result = Header ? &Header->Sound : 0;
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
      
    } break;
  }

  return Result;
}

internal void LoadSound(game_assets *Assets, sound_id ID);
internal void LoadBitmap(game_assets *Assets, bitmap_id ID, bool32 Immediate);
internal task_with_memory *BeginTaskWithMemory(transient_state *TranState);
internal void EndTaskWithMemory(task_with_memory *Task);

inline uint32 BeginGenerationID(game_assets *Assets)
{
  BeginAssetLock(Assets);
  
  uint32 Result = Assets->NextGenerationID++;
  Assets->InFlightGenerations[Assets->InFlightGenerationCount++] = Result;
  
  EndAssetLock(Assets);
  return Result;
}

inline void EndGenerationID(game_assets *Assets, uint32 GenerationID)
{
  BeginAssetLock(Assets);

  for(uint32 Index = 0; Index < Assets->InFlightGenerationCount; ++Index)
  {
    if(Assets->InFlightGenerations[Index] == GenerationID)
    {
      Assets->InFlightGenerations[Index] = Assets->InFlightGenerations[--Assets->InFlightGenerationCount];
      break;
    }
  }
  
  EndAssetLock(Assets);
}

#define HANDMADE_ASSET_H
#endif
