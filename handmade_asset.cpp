#if 0
#pragma pack(push, 1)
struct bitmap_header
{
  uint16 FileType;
  uint32 FileSize;
  uint16 Reserved1;
  uint16 Reserved2;
  uint32 BitmapOffset;
  uint32 Size;
  int32 Width;
  int32 Height;
  uint16 Planes;
  uint16 BitPerPixel;
  uint32 Compression;
  uint32 SizeOfBitmap;
  int32 HorzResolution;
  int32 VertResolution;
  uint32 ColorsUser;
  uint32 ColorsImportant;

  uint32 RedMask;
  uint32 GreenMask;
  uint32 BlueMask;
};
struct WAVE_header
{
    uint32 RIFFID;
    uint32 Size;
    uint32 WAVEID;
};

#define RIFF_CODE(a, b, c, d)(((uint32)(a) << 0)  | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))
enum
{
  WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
  WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
  WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
  WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
};

struct WAVE_chunk
{
  uint32 ID;
  uint32 Size;
};

struct WAVE_fmt
{
    uint16 wFormatTag;
    uint16 nChannels;
    uint32 nSamplesPerSec;
    uint32 nAvgBytesPerSec;
    uint16 nBlockAlign;
    uint16 wBitsPerSample;
    uint16 cbSize;
    uint16 wValidBitsPerSample;
    uint32 dwChannelMask;
    uint8 SubFormat[16];
};
#pragma pack(pop)

inline v2 TopDownAlign(loaded_bitmap *Bitmap, v2 Align)
{
  Align.y = (real32)(Bitmap->Height - 1) - Align.y;

  Align.x = SafeRatio0((real32)Align.x, (real32)Bitmap->Width);
  Align.y = SafeRatio0((real32)Align.y, (real32)Bitmap->Height);
  
  return Align;
}

internal loaded_bitmap DEBUGLoadBMP(char *FileName, v2 AlignPercentage = {0.5, 0.5})
{
  loaded_bitmap Result = {};
  debug_read_file_result ReadResult = DEBUGReadEntireFile(FileName);
  bitmap_header *BitMap = (bitmap_header *)ReadResult.Contents;
  uint32 *Pixel = (uint32 *)((uint8 *)ReadResult.Contents  + BitMap->BitmapOffset);
  if(ReadResult.ContentSize > 0)
  {

    Assert(BitMap->Compression == 3);
    
    uint32 RedMask = BitMap->RedMask;
    uint32 GreenMask = BitMap->GreenMask;
    uint32 BlueMask = BitMap ->BlueMask;
    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

    bitscan_result RedShift = FindLastSignificantBit(RedMask);
    bitscan_result GreenShift = FindLastSignificantBit(GreenMask);
    bitscan_result BlueShift = FindLastSignificantBit(BlueMask);
    bitscan_result AlphaShift = FindLastSignificantBit(AlphaMask);

    int32 RedShiftDown = RedShift.Index;
    int32 GreenShiftDown = GreenShift.Index;
    int32 BlueShiftDown = BlueShift.Index;
    int32 AlphaShiftDown = AlphaShift.Index;


    uint32 *SourceDest = Pixel;
    for(int32 Y = 0; Y < BitMap->Width; ++Y)
    {
      for(int32 X = 0; X < BitMap->Height; ++X)
      {
	uint32 C = *SourceDest;

	v4 Texel = {(real32)((C & RedMask) >> RedShiftDown),
		    (real32)((C & GreenMask) >> GreenShiftDown),
		    (real32)((C & BlueMask) >> BlueShiftDown),
		    (real32)((C & AlphaMask) >> AlphaShiftDown)};

	Texel = SRGB255ToLinear1(Texel);
	
#if 1
	Texel.rgb *= Texel.a;
#endif
	Texel = Linear1ToSRGB255(Texel);
	
	*SourceDest++ = (((uint32(Texel.a + 0.5f)) << 24) |
			 ((uint32(Texel.r + 0.5f)) << 16) |
			 ((uint32(Texel.g + 0.5f)) << 8) |
			 ((uint32(Texel.b + 0.5f)) << 0));
      }
    }
  Result.Memory = Pixel;
  Result.Width = BitMap->Width;
  Result.Height = BitMap->Height;
  Result.WidthOverHeight =  SafeRatio0((real32)Result.Width, (real32)Result.Height);
  Result.AlignPercentage = AlignPercentage;
  real32 PixelsToMeter = 1.0f / 42.0f;
  int32 BytesPerPixel = BITMAP_BYTES_PER_PIXEL;
  Result.Pitch = BitMap->Width*BytesPerPixel;
  
#if 0
  Result.Memory = (uint8 *)Result.Memory + Result.Pitch*(Result.Height - 1);
  Result.Pitch = -BitMap->Width*BytesPerPixel;
#endif
  }
  
  return Result;
}

/*
internal loaded_bitmap DEBUGLoadBMP(char *FileName)
{
  loaded_bitmap Result = DEBUGLoadBMP(FileName, 0, 0);
  Result.AlignPercentage =  v2{0.5f, 0.5f};
  return Result;
}
*/
struct riff_iterator
{
  uint8 *At;
  uint8 *Stop;
};

inline riff_iterator ParseChunkAt(void *At, void *Stop)
{
  riff_iterator Iter;

  Iter.At = (uint8 *)At;
  Iter.Stop = (uint8 *)Stop;
  
  return Iter;
}

inline riff_iterator GetNextChunk(riff_iterator Iter)
{
  WAVE_chunk* Chunk = (WAVE_chunk*)Iter.At;
  uint32 Size = (Chunk->Size + 1) & ~1;
  Iter.At += Size + sizeof(WAVE_chunk);

  return Iter;
}

inline bool32 IsValid(riff_iterator Iter)
{
  bool32 Result = (Iter.At < Iter.Stop);

  return Result;
}

inline void *GetChunkData(riff_iterator Iter)
{
  void *Result = Iter.At + sizeof(WAVE_chunk);

  return Result;
}

inline uint32 GetType(riff_iterator Iter)
{
  WAVE_chunk* Chunk = (WAVE_chunk*)Iter.At;
  uint32 Result = Chunk->ID;
  return Result;
}

inline uint32 GetDataChunkSize(riff_iterator Iter)
{
  WAVE_chunk* Chunk = (WAVE_chunk*)Iter.At;
  uint32 Result = Chunk->Size;
  return Result;
}

internal loaded_sound DEBUGLoadWAV(char* FileName, uint32 SectionSampleIndex, uint32 SectionSampleCount)
{
  loaded_sound Result = {};
  debug_read_file_result ReadResult = DEBUGReadEntireFile(FileName);
  if(ReadResult.ContentSize != 0)
  {
    WAVE_header* Header = (WAVE_header*)ReadResult.Contents;
    Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
    Assert(Header-> WAVEID == WAVE_ChunkID_WAVE);
    int16 *SampleData = 0;
    uint32 ChannelCount = 0;
    uint32 SampleDataSize = 0;
    for (riff_iterator Iter = ParseChunkAt((Header + 1), (uint8 *)(Header + 1) + Header->Size - 4); IsValid(Iter); Iter = GetNextChunk(Iter))
    {
      switch(GetType(Iter))
      {
        case WAVE_ChunkID_fmt:
	{
	  WAVE_fmt* fmt = (WAVE_fmt *)GetChunkData(Iter);
	  Assert(fmt->wFormatTag == 1);
	  Assert(fmt->nSamplesPerSec == 48000 || fmt->nSamplesPerSec == 44100);
	  Assert(fmt->wBitsPerSample == 16);
	  Assert(fmt->nBlockAlign == sizeof(int16)*fmt->nChannels);
	  ChannelCount = fmt->nChannels;
	} break;

        case WAVE_ChunkID_data:
        {
	  SampleData = (int16 *)GetChunkData(Iter);
	  SampleDataSize = GetDataChunkSize(Iter);
	}break;
      }
    }
    Assert(ChannelCount && SampleData);

    uint32 SampleCount = SampleDataSize / (ChannelCount*sizeof(int16));
    Result.ChannelCount = ChannelCount;

    if(ChannelCount == 1)
    {
      Result.Samples[0] = SampleData;
      Result.Samples[1] = 0;
    }
    else if(ChannelCount == 2)
    {
      Result.Samples[0] = SampleData;
      Result.Samples[1] = SampleData + SampleCount;
      for (uint32 SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
      {
	int16 Source = SampleData[SampleIndex*2];
	SampleData[SampleIndex*2] = SampleData[SampleIndex];
	SampleData[SampleIndex] = Source;
      }
    }
    else
    {
      Assert(!"Invalid number of channels in the WAV file");
    }
    //For now works just the left channel
    Result.ChannelCount = 1;
    bool32 AtEnd = true;
    if(SectionSampleCount)
    {
      SampleCount = SectionSampleCount;
      AtEnd = ((SectionSampleIndex + SectionSampleCount) == SampleCount);
      for(uint32 ChannelIndex = 0; ChannelIndex <  Result.ChannelCount; ++ChannelIndex)
      {
	Result.Samples[ChannelIndex] += SectionSampleIndex; 
      }
    }
    if(AtEnd)
    {
      uint32 SampleCountAlign8 = Align8(SampleCount);
      for(uint32 ChannelIndex = 0; ChannelIndex <  Result.ChannelCount; ++ChannelIndex)
      {
	for(uint32 SampleIndex = SampleCount; SampleIndex < SampleCount + 8; ++SampleIndex)
	{
	  Result.Samples[ChannelIndex][SampleIndex] = 0;
	}
      }
    }

    Result.SampleCount = SampleCount;
  }
  return Result;
}
#endif
struct load_asset_work
{
  task_with_memory *Task;
  asset *Asset;
  platform_file_handle *Handle;
  uint64 Offset;
  uint64 Size;
  void *Destination;
  uint32 FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
  load_asset_work *Work = (load_asset_work*)Data;
  
  Platform.ReadDataFromFile(Work->Handle, Work->Offset, Work->Size, Work->Destination);

  CompletePreviousWriteBeforeFutureWrites;

  if(!PlatformNoFileErrors(Work->Handle))
  {
    ZeroSize(Work->Size, Work->Destination);
  }
  
  Work->Asset->State = Work->FinalState;

  EndTaskWithMemory(Work->Task);
}


struct load_bitmap_work
{
  loaded_bitmap *Bitmap;
  game_assets *Assets;
  bitmap_id ID;
  task_with_memory *Task;
  asset_state FinalState;
};

#if 0
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapWork)
{
  load_bitmap_work *Work = (load_bitmap_work*)Data;

  hha_asset *HHAAsset = &Work->Assets->Assets[Work->ID.Value];
  hha_bitmap *Info = &HHAAsset->Bitmap;
  loaded_bitmap *Bitmap = Work->Bitmap;

  Bitmap->AlignPercentage = v2{Info->AlignPercentage[0], Info->AlignPercentage[1]};
  Bitmap->WidthOverHeight = (real32)Info->Dim[0] / (real32)Info->Dim[1];
  Bitmap->Width = Info->Dim[0];
  Bitmap->Height = Info->Dim[1];
  Bitmap->Pitch = 4*Info->Dim[0];
  Bitmap->Memory = Work->Assets->HHAContents + HHAAsset->DataOffset;

  CompletePreviousWriteBeforeFutureWrites;
  
  Work->Assets->Slots[Work->ID.Value].Bitmap = Work->Bitmap;
  Work->Assets->Slots[Work->ID.Value].State = Work->FinalState;

  
  EndTaskWithMemory(Work->Task);
}
#endif

inline platform_file_handle *GetFileHandleFor(game_assets *Assets, uint32 FileIndex)
{
  Assert(FileIndex < Assets->FileCount);

  platform_file_handle *Handle = Assets->Files[FileIndex].Handle;
  return Handle;
}

inline void *AcquireAssetMemory(game_assets *Assets, memory_index Size)
{
  void *Result = Platform.AllocateMemory(Size);
  if(Result)
  {
    Assets->TotalMemoryUsed += Size;
  }
  return Result;
}

inline void ReleaseAssetMemory(game_assets *Assets, memory_index Size, void *Memory)
{
  if(Memory)
  {
    Assets->TotalMemoryUsed -= Size;
  }
  Platform.DeallocateMemory(Memory);
}

struct asset_memory_size
{
  uint32 Total;
  uint32 Data;
  uint32 Section;
};

inline void InsertAssetHeaderAtFront(game_assets *Assets, asset_memory_header *Header)
{
  asset_memory_header *Sentinel = &Assets->LoadedAssetSentinel;

  Header->Next = Sentinel->Next;
  Header->Prev = Sentinel;
  
  Header->Next->Prev = Header;
  Header->Prev->Next = Header;
}

internal void AddAssetHeaderToList(uint32 AssetIndex, game_assets *Assets, asset_memory_size Size)
{
  asset_memory_header *Header = Assets->Assets[AssetIndex].Header;
  Header->AssetIndex = AssetIndex;
  Header->TotalSize = Size.Total;
  InsertAssetHeaderAtFront(Assets, Header);
}

internal void RemoveAssetHeaderFromList(asset_memory_header *Header)
{
  Header->Prev->Next = Header->Next;
  Header->Next->Prev = Header->Prev; 
}

internal void LoadBitmap(game_assets *Assets, bitmap_id ID, bool32 Locked) 
{
  asset *Asset = Assets->Assets + ID.Value;
  if(ID.Value && AtomicCompareExchangeUInt32((uint32 *)&Assets->Assets[ID.Value].State, AssetState_Unloaded, AssetState_Queued) == AssetState_Unloaded)
  {
    task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
    if(Task)
    {
      //asset *Asset = Assets->Assets + ID.Value;
      hha_bitmap *Info = &Asset->HHA.Bitmap;
      asset_memory_size Size = {};
    
      uint32 Width = SafeTruncateUInt16(Info->Dim[0]);
      uint32 Height = SafeTruncateUInt16(Info->Dim[1]);
      Size.Section = 4*Width;
      Size.Data = Height*Size.Section;
      Size.Total += Size.Data + sizeof(asset_memory_header);

      Asset->Header = (asset_memory_header *)AcquireAssetMemory(Assets, Size.Total);
      loaded_bitmap *Bitmap = &Asset->Header->Bitmap;
      Bitmap->AlignPercentage = v2{Info->AlignPercentage[0], Info->AlignPercentage[1]};
      Bitmap->WidthOverHeight = (real32)Info->Dim[0] / (real32)Info->Dim[1];
      Bitmap->Width = SafeTruncateUInt16(Info->Dim[0]);
      Bitmap->Height = SafeTruncateUInt16(Info->Dim[1]);
      Bitmap->Pitch = SafeTruncateInt16(Size.Section);
      Bitmap->Memory = (Asset->Header + 1);//PushSize(&Assets->Arena, MemorySize);

      //Copy(MemorySize, Assets->HHAContents + HHAAsset->DataOffset, Bitmap->Memory);

      load_asset_work *Work = PushStruct(&Task->Arena, load_asset_work);
      Work->Asset = Assets->Assets + ID.Value;
      Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
      Work->Offset = Asset->HHA.DataOffset;
      Work->Size = Size.Data;
      Work->Task = Task;
      Work->Destination = Bitmap->Memory;
      Work->FinalState = (AssetState_Loaded) | (Locked ? AssetState_Lock : 0);

      Asset->State |= AssetState_Lock;
      
      if(!Locked)
      {
	AddAssetHeaderToList(ID.Value, Assets, Size);
      }

      Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, Work);
    }
    else
    {
      Assets->Assets[ID.Value].State = AssetState_Unloaded;
    }
  }
}

internal uint32 BestMatchAsset(game_assets* Assets, asset_type_id TypeID, asset_vector *MatchVector, asset_vector *WeightVector)
{
  uint32 Result; 
  real32 BestDiff = Real32Maximum;
  asset_type* Type = Assets->AssetTypes + TypeID;
  for (uint32 AssetIndex = Type->FirstAssetIndex; AssetIndex < Type->OnePastLastAssetIndex; ++AssetIndex)
  {
    asset *Asset = Assets->Assets + AssetIndex;
    real32 TotalWeigthDifference = 0.0f;
    for (uint32 TagIndex = Asset->HHA.FirstTagIndex; TagIndex < Asset->HHA.OneLastPastTagIndex; ++TagIndex)
    {
      hha_tag* Tag = Assets->Tags + TagIndex;
      real32 A = MatchVector->E[Tag->ID];
      real32 B = Tag->Value;
      real32 D0 = AbsoluteValue(A - B);
      real32 D1 = AbsoluteValue(A - SignOf(A)*Assets->TagRange[Tag->ID] - B);
      real32 Difference = Minimum(D0, D1);
      real32 Weighted = WeightVector->E[Tag->ID] * Difference;
      TotalWeigthDifference += Weighted;
     }

     if (BestDiff > TotalWeigthDifference)
     {
       BestDiff = TotalWeigthDifference;
       Result = AssetIndex;
     }
  }
  return Result;
}

internal bitmap_id BestMatchBitmap(game_assets* Assets, asset_type_id TypeID, asset_vector* MatchVector, asset_vector* WeightVector)
{
  bitmap_id Result = {BestMatchAsset(Assets, TypeID, MatchVector, WeightVector)};
  return Result;
}

internal uint32 RandomAssetFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
  uint32 Result = 0;

  asset_type *Type = Assets->AssetTypes + TypeID;
  if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
  {
    uint32 Count = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
    uint32 Choice = RandomChoice(Series, Count);
    Result = Type->FirstAssetIndex + Choice;
  }
  
  return Result;
}

internal bitmap_id RandomBitmapFrom(game_assets* Assets, asset_type_id TypeID, random_series* Series)
{
  bitmap_id Result = {RandomAssetFrom(Assets, TypeID, Series)};
  return Result;
}

internal uint32 GetFirstAssetID(game_assets *Assets, asset_type_id TypeID)
{
  uint32 Result;

  asset_type *Type = Assets->AssetTypes + TypeID;
  if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
  {
    Result = Type->FirstAssetIndex;
  }
  
  return Result;
}

internal bitmap_id GetFirstBitmap(game_assets* Assets, asset_type_id TypeID)
{
  bitmap_id Result = {GetFirstAssetID(Assets, TypeID)};

  return Result;
}

internal sound_id GetFirstSound(game_assets* Assets, asset_type_id TypeID)
{
  sound_id Result = {GetFirstAssetID(Assets, TypeID)};

  return Result;
}

#if 0
struct load_sound_work
{
  game_assets* Assets;
  sound_id ID;
  task_with_memory* Task;
  loaded_sound *Sound;
  asset_state FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadSoundWork)
{
  load_sound_work* Work = (load_sound_work*)Data;
  hha_asset *HHAAsset = &Work->Assets->Assets[Work->ID.Value];
  hha_sound* Info = &HHAAsset->Sound;
  
  loaded_sound *Sound = Work->Sound;

  Sound->SampleCount = Info->SampleCount;
  Sound->ChannelCount = Info->ChannelCount;
  uint64 SampleDataOffset = HHAAsset->DataOffset;
  Assert(Sound->ChannelCount < ArrayCount(Sound->Samples));
  for(uint32 ChannelIndex = 0; ChannelIndex < Sound->ChannelCount; ++ChannelIndex)
  {
    Sound->Samples[ChannelIndex] = (int16 *)(Work->Assets->HHAContents + SampleDataOffset);
    SampleDataOffset += Sound->SampleCount*sizeof(uint16);
  }
  
  CompletePreviousWriteBeforeFutureWrites;

  Work->Assets->Assets[Work->ID.Value].Sound = Work->Sound;
  Work->Assets->Assets[Work->ID.Value].State = Work->FinalState;

  EndTaskWithMemory(Work->Task);
}
#endif

internal void LoadSound(game_assets *Assets, sound_id ID)
{
  asset *Asset = Assets->Assets + ID.Value;
  if (ID.Value && AtomicCompareExchangeUInt32((uint32*)&Assets->Assets[ID.Value].State, AssetState_Unloaded, AssetState_Queued) == AssetState_Unloaded)
  {
    task_with_memory* Task = BeginTaskWithMemory(Assets->TranState);
    if(Task)
    {
      //asset *Asset = Assets->Assets + ID.Value;
      hha_sound *Info = &Asset->HHA.Sound;
      asset_memory_size Size = {};
      Size.Section = Info->SampleCount*sizeof(int16);
      Size.Data = Info->ChannelCount*Size.Section;
      Size.Total += Size.Data + sizeof(asset_memory_header);

      Asset->Header = (asset_memory_header *)AcquireAssetMemory(Assets, Size.Total); 
      
      loaded_sound *Sound = &Asset->Header->Sound;
      Sound->SampleCount = Info->SampleCount;
      Sound->ChannelCount = Info->ChannelCount;
      uint32 ChannelSize = Size.Section; 
      void *Memory = (Asset->Header + 1);//PushSize(&Assets->Arena, MemorySize);


      int16 *SoundAt = (int16*)Memory;
      for(uint32 ChannelIndex = 0; ChannelIndex < Sound->ChannelCount; ++ChannelIndex)
      {
	Sound->Samples[ChannelIndex] = SoundAt;
	SoundAt += ChannelSize;
      }

      load_asset_work* Work = PushStruct(&Task->Arena, load_asset_work);
      Work->Asset = Assets->Assets + ID.Value;
      Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
      Work->Offset = Asset->HHA.DataOffset;
      Work->Size = Size.Data;
      Work->Task = Task;
      Work->Destination = Memory;
      Work->FinalState = (AssetState_Loaded);
      //Copy(MemorySize, Assets->HHAContents + HHAAsset->DataOffset, Memory);
      AddAssetHeaderToList(ID.Value, Assets, Size);
      
      Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, Work);
    }
  }
}

#if 0
internal void BeginAssetType(game_assets *Assets, asset_type_id TypeID)
{
  Assert(Assets->DEBUGAssetType == 0);
  Assets->DEBUGAssetType = Assets->AssetTypes + TypeID;
  Assets->DEBUGAssetType->FirstAssetIndex = Assets->DEBUGAssetCount;
  Assets->DEBUGAssetType->OnePastLastAssetIndex = Assets->DEBUGAssetType->FirstAssetIndex;
}

internal bitmap_id AddBitmapAsset(game_assets *Assets, char *FileName, v2 AlignPercentage = v2{0.5f, 0.5f})
{
  Assert(Assets->DEBUGAssetType);
  bitmap_id Result = {Assets->DEBUGAssetType->OnePastLastAssetIndex++};
  asset *Asset = Assets->Assets + Result.Value;
  Asset->FirstTagIndex = Assets->DEBUGTagCount;
  Asset->OneLastPastTagIndex = Asset->FirstTagIndex;
  Assets->DEBUGAsset = Asset;
  Asset->Bitmap.FileName = FileName;
  Asset->Bitmap.AlignPercentage = AlignPercentage;

  return Result;
}

internal sound_id AddSoundAsset(game_assets* Assets, char* FileName, uint32 FirstSampleIndex = 0, uint32 SampleCount = 0)
{
    Assert(Assets->DEBUGAssetType);
    sound_id Result = {Assets->DEBUGAssetType->OnePastLastAssetIndex++};
    asset* Asset = Assets->Assets + Result.Value;
    Asset->FirstTagIndex = Assets->DEBUGTagCount;
    Asset->OneLastPastTagIndex = Asset->FirstTagIndex;
    Asset->Sound.FileName = FileName;
    Asset->Sound.FirstSampleIndex = FirstSampleIndex;
    Asset->Sound.SampleCount = SampleCount;
    Asset->Sound.NextIDToPlay.Value = 0;
  
    Assets->DEBUGAsset = Asset;

    return Result;
}

internal void EndAssetType(game_assets *Assets)
{
  Assert(Assets->DEBUGAssetType);
  Assets->DEBUGAssetCount = Assets->DEBUGAssetType->OnePastLastAssetIndex;
  Assets->DEBUGAssetType = 0;
  Assets->DEBUGAsset = 0;
}

internal void AddTag(game_assets *Assets, asset_tag_id ID, real32 Value)
{ 
  Assert(Assets->DEBUGAsset);
  ++Assets->DEBUGAsset->OneLastPastTagIndex;
  asset_tag* Tag = Assets->Tags + Assets->DEBUGTagCount++;

  Tag->ID = ID;
  Tag->Value = Value;
}

#endif 

internal void PrefetchBitmap(game_assets* Assets, bitmap_id ID, bool32 Locked) { LoadBitmap(Assets, ID, Locked); }
internal void PrefetchSound(game_assets* Assets, sound_id ID) { LoadSound(Assets, ID); }

internal game_assets *AllocateGameAssets(memory_arena *Arena, memory_index Size, transient_state *TranState)
{
  game_assets *Assets = PushStruct(Arena, game_assets);
  SubArena(&Assets->Arena, &TranState->TranArena, Size);
  Assets->TranState = TranState;
  Assets->TotalMemoryUsed = 0;
  Assets->TargetMemoryUsed = Size;
  Assets->LoadedAssetSentinel.Next = &Assets->LoadedAssetSentinel; 
  Assets->LoadedAssetSentinel.Prev = &Assets->LoadedAssetSentinel;
  
  for(uint32 TagType = 0; TagType < Tag_Count; ++TagType)
  {
    Assets->TagRange[TagType] = 1000000.0f;
  }

  Assets->TagRange[Tag_Facing_Direction] = Tau32; 

  Assets->AssetCounts = 1;
  Assets->TagCounts = 1;  
//------------------------------------------------------------------------
#if 1
  platform_file_group *FileGroup = Platform.GetAllFilesOfTypeBegin("hha");
  Assets->FileCount = FileGroup->FileCount;
  Assets->Files = PushArray(Arena, Assets->FileCount, asset_file);

  for(uint32 FileIndex = 0; FileIndex < Assets->FileCount; ++FileIndex)
  {
    asset_file *File = Assets->Files + FileIndex;

    File->TagBase = Assets->TagCounts;

    ZeroStruct(File->Header);
    File->Handle = Platform.OpenNextFile(FileGroup);
    Platform.ReadDataFromFile(File->Handle, 0, sizeof(File->Header), &File->Header);
    uint32 AssetTypeArraySize = File->Header.AssetTypeCount*sizeof(hha_asset_type);
    File->AssetTypeArray = (hha_asset_type *)PushSize(Arena, AssetTypeArraySize);
    
    Platform.ReadDataFromFile(File->Handle, File->Header.AssetTypeOffset, AssetTypeArraySize, File->AssetTypeArray);

    if(PlatformNoFileErrors(File->Handle))
    {
      if(File->Header.HeaderCode != HHA_HEADER_CODE)
      {
	Platform.FileError(File->Handle, "HHA file has an invalid magic number.");
      }
      if(File->Header.Version >  HHA_VERSION)
      {
	Platform.FileError(File->Handle, "HHA file is of later verision");
      }
      if(PlatformNoFileErrors(File->Handle))
      {
	//Every first asset in the each asset and tag file is a null asset
	Assets->TagCounts += (File->Header.TagCount - 1);
	Assets->AssetCounts += (File->Header.AssetCount - 1);
      }
      else
      {
	InvalidCodePath;
      }
    }
    else
    {
      InvalidCodePath;
    }
  }
  
  Platform.GetAllFilesOfTypeEnd(FileGroup);

  Assets->Assets = PushArray(Arena, Assets->AssetCounts, asset);
  Assets->Assets = PushArray(Arena, Assets->AssetCounts, asset);
  Assets->Tags = PushArray(Arena, Assets->TagCounts, hha_tag);

  for(uint32 FileIndex = 0; FileIndex < Assets->FileCount; ++FileIndex)
  {
    asset_file *File = Assets->Files + FileIndex;
    if(PlatformNoFileErrors(File->Handle))
    {
      uint32 TagArraySize = sizeof(hha_tag)*(File->Header.TagCount - 1);
      Platform.ReadDataFromFile(File->Handle, File->Header.TagsOffset + sizeof(hha_tag), TagArraySize, Assets->Tags + File->TagBase);
    }
  }

  //Clear to zero the null asset and tag
  uint32 AssetCount = 0;
  ZeroStruct(*(Assets->Assets + AssetCount));
  ++AssetCount;

  ZeroStruct(Assets->Tags[0]);

  for(uint32 DestTypeID = 0; DestTypeID < Asset_Count; ++DestTypeID)
  {
    asset_type *DestType = Assets->AssetTypes + DestTypeID;
    DestType->FirstAssetIndex = AssetCount;
    
    for(uint32 FileIndex = 0; FileIndex < Assets->FileCount; ++FileIndex)
    {
      asset_file *File = Assets->Files + FileIndex;
      if(PlatformNoFileErrors(File->Handle))
      {
	for(uint32 SourceIndex = 0; SourceIndex < File->Header.AssetTypeCount; ++SourceIndex)
	{
	  hha_asset_type *SourceType = File->AssetTypeArray + SourceIndex;
	  if(SourceType->TypeID == DestTypeID)
	  {
	    uint32 AssetCountForType = (SourceType->OnePastLastAssetIndex - SourceType->FirstAssetIndex);

	    temporary_memory TempMem = BeginTemporaryMemory(&TranState->TranArena);
	    hha_asset *HHAAssetArray = PushArray(&TranState->TranArena, AssetCountForType, hha_asset);
	    Platform.ReadDataFromFile(File->Handle, File->Header.AssetOffset + SourceType->FirstAssetIndex*sizeof(hha_asset),
					   sizeof(hha_asset)*AssetCountForType, HHAAssetArray);
	    
	    for(uint32 AssetIndex = 0; AssetIndex < AssetCountForType; ++AssetIndex)
	    {
	      hha_asset *HHAAsset = HHAAssetArray + AssetIndex;
		
	      Assert(AssetCount < Assets->AssetCounts);
	      asset *Asset = Assets->Assets + AssetCount++;

	      Asset->FileIndex = FileIndex;
	      Asset->HHA = *HHAAsset;
	      if(Asset->HHA.FirstTagIndex == 0)
		{ 
		Asset->HHA.FirstTagIndex =  Asset->HHA.OneLastPastTagIndex = 0; 
	      }
	      else
	      {
		Asset->HHA.FirstTagIndex += File->TagBase - 1;
		Asset->HHA.OneLastPastTagIndex += File->TagBase - 1;
	      }
	    }
	    EndTemporaryMemory(TempMem);
	  }
	}
      }
    }
    DestType->OnePastLastAssetIndex = AssetCount;
  }
  //Assert(AssetCount == Assets->AssetCounts);
  //Assert(TagCount == Assets->TagCounts);
#endif
//------------------------------------------------------------------
#if 0
  debug_read_file_result ReadResult = Platform.DEBUGReadEntireFile("test.fam");
  if(ReadResult.ContentSize != 0)
  {
    hha_header *Header = (hha_header *)(ReadResult.Contents);
    Assert(Header->HeaderCode == HHA_HEADER_CODE);
    Assert(Header->Version == HHA_VERSION);
    
    Assets->TagCounts = Header->TagCount;
    Assets->AssetCounts = Header->AssetCount;
    Assets->Assets = (hha_asset *)((uint8 *)ReadResult.Contents + Header->AssetOffset); 
    Assets->Assets = PushArray(Arena, Assets->AssetCounts, asset);

    Assets->Tags = (hha_tag *)((uint8 *)ReadResult.Contents + Header->TagsOffset);
    hha_asset_type *HHAAssetTypes = (hha_asset_type *)((uint8 *)ReadResult.Contents + Header->AssetTypeOffset);


    for(uint32 AssetTypeIndex = 0; AssetTypeIndex < Header->AssetTypeCount; ++AssetTypeIndex)
    {
      hha_asset_type *Source = HHAAssetTypes + AssetTypeIndex;
      if(Source->TypeID < Asset_Count)
      {
	asset_type *Dest = Assets->AssetTypes + Source->TypeID;
	Assert(Dest->FirstAssetIndex == 0);
	Assert(Dest->OnePastLastAssetIndex == 0);
	Dest->FirstAssetIndex = Source->FirstAssetIndex;
	Dest->OnePastLastAssetIndex = Source->OnePastLastAssetIndex;
      }
    }

    Assets->HHAContents = (uint8 *)ReadResult.Contents;
    
  }
#endif
  
#if 0
  Assets->DEBUGAssetCount = 1;
  
  BeginAssetType(Assets, Asset_BackGround);
  AddBitmapAsset(Assets, "test/test_img.bmp", v2{0.5f, 0.5f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Wall);
  AddBitmapAsset(Assets, "test/brick.bmp", v2{0.5f, 0.0f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Monster);
  AddBitmapAsset(Assets, "test/monster.bmp", v2{0.4f, 0.05f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Sword);
  AddBitmapAsset(Assets, "test/fireball.bmp", v2{0.5f, 0.5f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Staff);
  AddBitmapAsset(Assets, "test/staff.bmp""test/staff.bmp", v2{0.5f, 0.5f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Stair);
  AddBitmapAsset(Assets, "test/staff.bmp", v2{0.5f, 0.5f});
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Grass);
  AddBitmapAsset(Assets, "test/Grass.bmp");
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Dirt);
  AddBitmapAsset(Assets, "test/Dirt.bmp");
  EndAssetType(Assets);

  real32 AngleLeft = 0.5f*Tau32;
  real32 AngleRigth = 0*Tau32;

  BeginAssetType(Assets, Asset_Wizard);
  AddBitmapAsset(Assets, "test/mage1.bmp", v2{0.5f, 0.05f});
  AddTag(Assets, Tag_Facing_Direction, AngleRigth);
  AddBitmapAsset(Assets, "test/mage2.bmp", v2{0.5f, 0.05f});
  AddTag(Assets, Tag_Facing_Direction, AngleLeft);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_FireSound);
  AddSoundAsset(Assets, "test/fire.wav");
  EndAssetType(Assets);

  /*
    Testing audio chunks streaming
  uint32 OneMusicChunk = 1*48000;
  uint32 TotalMusicSampleCount = 2*OneMusicChunk; //Check this in the debugger
  BeginAssetType(Assets, Asset_DungeonSound);
  asset *LastMusic = 0;
  asset *FirstMusic = 0;

  for(uint32 SampleIndex = 0; SampleIndex < TotalMusicSampleCount; SampleIndex += OneMusicChunk)
  {
    uint32 SampleCount = TotalMusicSampleCount - SampleIndex;
    if (SampleCount > OneMusicChunk)
    {
      SampleCount = OneMusicChunk;
    }
    asset* ThisMusic = AddSoundAsset(Assets, "test/dungeon.wav", SampleIndex, SampleCount);
    if(SampleIndex == 0)
    {
      FirstMusic = ThisMusic;
    }
    if (LastMusic)
    {
      Assets->SoundInfos[LastMusic->AssetID].NextIDToPlay.Value = ThisMusic->AssetID;
    }
    LastMusic = ThisMusic;
  }

  Assets->SoundInfos[LastMusic->AssetID].NextIDToPlay.Value = FirstMusic->AssetID;

  
  EndAssetType(Assets);
  */
  
  BeginAssetType(Assets, Asset_DungeonSound);
  AddSoundAsset(Assets, "test/dungeon.wav");
  EndAssetType(Assets);

  
  //Assets->Wizard.Wiz[0] = DEBUGLoadBMP("test/mage1.bmp", v2{0.5f, 0.05f});
  //Assets->Wizard.Wiz[1] = DEBUGLoadBMP("test/mage2.bmp", v2{0.5f, 0.05f});
#endif
  return Assets;
}

void MoveHeaderToFront(game_assets *Assets, asset *Asset)
{
  if(!IsLocked(Asset))
  {
    asset_memory_header *Header = Asset->Header;

    RemoveAssetHeaderFromList(Header);
    InsertAssetHeaderAtFront(Assets, Header);
  }
}

internal void EvictAsset(game_assets *Assets, asset_memory_header *Header)
{

  uint32 AssetIndex = Header->AssetIndex;
  asset *Asset = Assets->Assets + AssetIndex;
  Assert(!IsLocked(Asset));

  RemoveAssetHeaderFromList(Header);
  ReleaseAssetMemory(Assets, Asset->Header->TotalSize, Asset->Header);
  Asset->State = AssetState_Unloaded;
  Asset->Header = 0;    
}

internal void EvictAssetsAsNecessary(game_assets *Assets)
{
  while(Assets->TotalMemoryUsed > Assets->TargetMemoryUsed)
  {
    asset_memory_header *Header = Assets->LoadedAssetSentinel.Prev;
    if(Header != &Assets->LoadedAssetSentinel)
    {
      uint32 AssetIndex = Header->AssetIndex;
      asset *Asset = Assets->Assets + AssetIndex;
      if(GetState(Asset) >= AssetState_Loaded)
      {
	EvictAsset(Assets, Header);
      }
    }
    else
    {
      InvalidCodePath;
      break;
    }
  }
}
