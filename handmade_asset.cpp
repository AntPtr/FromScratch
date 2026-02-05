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

struct load_bitmap_work
{
  loaded_bitmap *Bitmap;
  game_assets *Assets;
  bitmap_id ID;
  task_with_memory *Task;
  asset_state FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapWork)
{
  load_bitmap_work *Work = (load_bitmap_work*)Data;

  asset_bitmap_info *Info = Work->Assets->BitmapInfos + Work->ID.Value;
  
  *Work->Bitmap = DEBUGLoadBMP(Info->FileName, Info->AlignPercentage);

  CompletePreviousWriteBeforeFutureWrites;
  
  Work->Assets->Bitmaps[Work->ID.Value].Bitmap = Work->Bitmap;
  Work->Assets->Bitmaps[Work->ID.Value].State = Work->FinalState;

  
  EndTaskWithMemory(Work->Task);
}

internal void LaodBitmap(game_assets *Assets, bitmap_id ID) 
{
  if(ID.Value && AtomicCompareExchangeUInt32((uint32 *)&Assets->Bitmaps[ID.Value].State, AssetState_Unloaded, AssetState_Queued) == AssetState_Unloaded)
  {
    task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
    if(Task)
    {
      load_bitmap_work *Work = PushStruct(&Task->Arena, load_bitmap_work);

      Work->Assets = Assets;
      Work->ID = ID;
      Work->Task = Task;
      Work->Bitmap = PushStruct(&Assets->Arena, loaded_bitmap);
      Work->FinalState = AssetState_Loaded;
      PlatformAddEntry(Assets->TranState->LowPriorityQueue, LoadBitmapWork, Work);

    }
  }
}

internal bitmap_id BestMatchAsset(game_assets* Assets, asset_type_id TypeID, asset_vector *MatchVector, asset_vector *WeightVector)
{
  bitmap_id Result; 
  real32 BestDiff = Real32Maximum;
  asset_type* Type = Assets->AssetTypes + TypeID;
  for (uint32 AssetIndex = Type->FirstAssetIndex; AssetIndex < Type->OnePastLastAssetIndex; ++AssetIndex)
  {
    asset *Asset = Assets->Assets + AssetIndex;
    real32 TotalWeigthDifference = 0.0f;
    for (uint32 TagIndex = Asset->FirstTagIndex; TagIndex < Asset->OneLastPastTagIndex; ++TagIndex)
    {
      asset_tag* Tag = Assets->Tags + TagIndex;
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
       Result.Value = Asset->SlotID;
     }
  }
  return Result;
}

internal bitmap_id RandomAssetFrom(game_assets *Assets, asset_type_id TypeID, random_series *Series)
{
  bitmap_id Result = {};

  asset_type *Type = Assets->AssetTypes + TypeID;
  if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
  {
    uint32 Count = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
    uint32 Choice = RandomChoice(Series, Count);
    asset *Asset = Assets->Assets + Type->FirstAssetIndex + Choice;
    Result.Value = Asset->SlotID;
  }
  
  return Result;
}

internal bitmap_id GetFirstBitmap(game_assets *Assets, asset_type_id TypeID)
{
  bitmap_id Result = {};

  asset_type *Type = Assets->AssetTypes + TypeID;;
  if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
  {
    asset *Asset = Assets->Assets + Type->FirstAssetIndex;
    Result.Value = Asset->SlotID;
  }
  
  return Result;
}

internal void LaodSound(game_assets *Assets, uint32 ID)
{
  
}

internal bitmap_id DEBUGAddBitmapInfo(game_assets *Assets, char *FileName, v2 AlignPercentage)
{
  Assert(Assets->DEBUGBitmapCount < Assets->BitmapCounts);
  bitmap_id ID = {Assets->DEBUGBitmapCount++};
  asset_bitmap_info *Info = Assets->BitmapInfos + ID.Value;

  Info->FileName = FileName;
  Info->AlignPercentage = AlignPercentage;

  return ID;
}

internal void BeginAssetType(game_assets *Assets, asset_type_id TypeID)
{
  Assert(Assets->DEBUGAssetType == 0);
  Assets->DEBUGAssetType = Assets->AssetTypes + TypeID;
  Assets->DEBUGAssetType->FirstAssetIndex = Assets->DEBUGAssetCount;
  Assets->DEBUGAssetType->OnePastLastAssetIndex = Assets->DEBUGAssetType->FirstAssetIndex;
}

internal void AddBitmapAsset(game_assets *Assets, char *FileName, v2 AlignPercentage = v2{0.5f, 0.5f})
{
  Assert(Assets->DEBUGAssetType);
  asset *Asset = Assets->Assets + Assets->DEBUGAssetType->OnePastLastAssetIndex++;
  Asset->FirstTagIndex = Assets->DEBUGTagCount;
  Asset->OneLastPastTagIndex = Asset->FirstTagIndex;
  Asset->SlotID = DEBUGAddBitmapInfo(Assets, FileName, AlignPercentage).Value;

  Assets->DEBUGAsset = Asset;
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


internal game_assets *AllocateGameAssets(memory_arena *Arena, memory_index Size, transient_state *TranState)
{
  game_assets *Assets = PushStruct(Arena, game_assets);
  SubArena(&Assets->Arena, &TranState->TranArena, Size);
  Assets->TranState = TranState;

  Assets->BitmapCounts = 256*Asset_Count;
  Assets->Bitmaps = PushArray(Arena, Assets->BitmapCounts, asset_slot);
  Assets->DEBUGBitmapCount = 1;
  Assets->BitmapInfos = PushArray(Arena, Assets->BitmapCounts, asset_bitmap_info);

  for(uint32 TagType = 0; TagType < Tag_Count; ++TagType)
  {
    Assets->TagRange[TagType] = 1000000.0f;
  }

  Assets->TagRange[Tag_Facing_Direction] = Tau32; 
  
  Assets->SoundCounts = 1;
  Assets->Sounds = PushArray(Arena, Assets->SoundCounts, asset_slot);

  Assets->TagCounts = Assets->BitmapCounts*2;
  Assets->Tags = PushArray(Arena, Assets->TagCounts, asset_tag);

  Assets->AssetCounts = Assets->SoundCounts + Assets->BitmapCounts;
  Assets->Assets = PushArray(Arena, Assets->AssetCounts, asset);

  Assets->DEBUGBitmapCount = 1;
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
  AddBitmapAsset(Assets, "test/staff.bmp", v2{0.5f, 0.5f});
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

  //Assets->Wizard.Wiz[0] = DEBUGLoadBMP("test/mage1.bmp", v2{0.5f, 0.05f});
  //Assets->Wizard.Wiz[1] = DEBUGLoadBMP("test/mage2.bmp", v2{0.5f, 0.05f});

  return Assets;
}
