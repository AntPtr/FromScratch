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

internal loaded_bitmap DEBUGLoadBMP(char *FileName, int32 AlignX, int32 TopDownAlignY)
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
  Result.AlignPercentage = TopDownAlign(&Result, V2i(AlignX, TopDownAlignY));
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

internal loaded_bitmap DEBUGLoadBMP(char *FileName)
{
  loaded_bitmap Result = DEBUGLoadBMP(FileName, 0, 0);
  Result.AlignPercentage =  v2{0.5f, 0.5f};
  return Result;
}


struct load_bitmap_work
{
  loaded_bitmap *Bitmap;
  game_assets *Assets;
  char *FileName;
  bitmap_id ID;
  task_with_memory *Task;
  bool32 HasAlignment;
  int32 AlignX;
  int32 TopDownAlignY;
  asset_state FinalState;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(LoadBitmapWork)
{
  load_bitmap_work *Work = (load_bitmap_work*)Data;

  if(Work->HasAlignment)
  {
    *Work->Bitmap = DEBUGLoadBMP(Work->FileName);
  }
  else
  {
    *Work->Bitmap = DEBUGLoadBMP(Work->FileName, Work->AlignX, Work->TopDownAlignY);
  }

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
      Work->FileName = "";
      Work->Task = Task;
      Work->HasAlignment = false;
      Work->Bitmap = PushStruct(&Assets->Arena, loaded_bitmap);
      Work->FinalState = AssetState_Loaded;
      
      switch(ID.Value)
      {
        case Asset_Monster:
	{
	  Work->FileName = "test/monster.bmp";
	  Work->HasAlignment = true;
	  Work->AlignX = 40;
	  Work->TopDownAlignY = 80;
	} break;
    
        case Asset_Sword:
	{
	  Work->FileName =  "test/fireball.bmp";
	  Work->HasAlignment = true;
	  Work->AlignX = 40;
	  Work->TopDownAlignY = 80;
	} break;
    
        case Asset_Staff:
	{
	  Work->FileName = "test/staff.bmp";
	  Work->HasAlignment = true;
	  Work->AlignX = 40;
	  Work->TopDownAlignY = 80;
	} break;
    
        case Asset_Stair:
	{
	 Work->FileName = "test/staff.bmp";
	 Work->HasAlignment = true;
	 Work->AlignX = 40;
	 Work->TopDownAlignY = 80;
	} break;
    
        case Asset_BackGround:
	{
	  Work->FileName = "test/test_img.bmp";
	  Work->HasAlignment = true;
	  Work->AlignX = 40;
	  Work->TopDownAlignY = 80;
	} break;

        case Asset_Wall:
	{
	  Work->FileName = "test/brick.bmp";
	  Work->HasAlignment = true;
	  Work->AlignX = 40;
	  Work->TopDownAlignY = 80;
	} break;
      }
      
      PlatformAddEntry(Assets->TranState->LowPriorityQueue, LoadBitmapWork, Work);

    }
  }
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

internal game_assets *AllocateGameAssets(memory_arena *Arena, memory_index Size, transient_state *TranState)
{
  game_assets *Assets = PushStruct(Arena, game_assets);
  SubArena(&Assets->Arena, &TranState->TranArena, Size);
  Assets->TranState = TranState;

  Assets->BitmapCounts = Asset_Count;
  Assets->Bitmaps = PushArray(Arena, Assets->BitmapCounts, asset_slot);

  Assets->SoundCounts = 1;
  Assets->Sounds = PushArray(Arena, Assets->SoundCounts, asset_slot);

  Assets->TagCounts = 0;
  Assets->Tags = 0;

  Assets->AssetCounts = Assets->BitmapCounts;
  Assets->Assets = PushArray(Arena, Assets->AssetCounts, asset);
  for(uint32 AssetID = 0; AssetID < Asset_Count; ++AssetID)
  {
    asset_type *Type = Assets->AssetTypes + AssetID;
    Type->FirstAssetIndex = AssetID;
    Type->OnePastLastAssetIndex = AssetID + 1;

    asset *Asset = Assets->Assets + Type->FirstAssetIndex;
    Asset->FirstTagIndex = 0;
    Asset->OneLastPastTagIndex = 0;
    Asset->SlotID = Type->FirstAssetIndex;
  }
  

  Assets->Wizard.Wiz[0] = DEBUGLoadBMP("test/mage1.bmp", 50, 145);
  Assets->Wizard.Wiz[1] = DEBUGLoadBMP("test/mage2.bmp", 50, 145);
  Assets->Grass[0] = DEBUGLoadBMP("test/Grass.bmp");
  Assets->Stones[0] = DEBUGLoadBMP("test/Dirt.bmp");

  return Assets;
}
