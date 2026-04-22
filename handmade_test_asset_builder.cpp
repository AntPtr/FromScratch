#include "handmade_test_asset_builder.h"
struct loaded_bitmap
{
  int32 Width;
  int32 Height;
  void *Memory;
  int32 Pitch;

  void *Free;
};

struct loaded_sound
{
  uint32 SampleCount;
  uint32 ChannelCount;
  int16 *Samples[2];

  void *Free;
};

internal void BeginAssetType(game_assets *Assets, asset_type_id TypeID)
{
  Assert(Assets->DEBUGAssetType == 0);
  Assets->DEBUGAssetType = Assets->AssetTypes + TypeID;
  Assets->DEBUGAssetType->TypeID = TypeID;
  Assets->DEBUGAssetType->FirstAssetIndex = Assets->AssetCount;
  Assets->DEBUGAssetType->OnePastLastAssetIndex = Assets->DEBUGAssetType->FirstAssetIndex;
}

internal bitmap_id AddBitmapAsset(game_assets *Assets, char *FileName, real32 AlignPercentageX = 0.5f, real32 AlignPercentageY = 0.5f)
{
  Assert(Assets->DEBUGAssetType);
  bitmap_id Result = {Assets->DEBUGAssetType->OnePastLastAssetIndex++};
  asset_source *Source = Assets->AssetsSources + Result.Value;
  hha_asset *HHA = Assets->Assets + Result.Value;
  HHA->FirstTagIndex = Assets->TagCount;
  HHA->OneLastPastTagIndex = HHA->FirstTagIndex;
  HHA->Bitmap.AlignPercentage[0] = AlignPercentageX;
  HHA->Bitmap.AlignPercentage[1] = AlignPercentageY;
  Source->FileName = FileName;
  Source->Type = AssetType_Bitmap;

  Assets->AssetIndex = Result.Value;

  return Result;
}

internal sound_id AddSoundAsset(game_assets* Assets, char* FileName, uint32 FirstSampleIndex = 0, uint32 SampleCount = 0)
{
    Assert(Assets->DEBUGAssetType);
    sound_id Result = {Assets->DEBUGAssetType->OnePastLastAssetIndex++};
    asset_source *Source = Assets->AssetsSources + Result.Value;
    hha_asset *HHA = Assets->Assets + Result.Value;

    HHA->FirstTagIndex = Assets->TagCount;
    HHA->OneLastPastTagIndex = HHA->FirstTagIndex;
    HHA->Sound.SampleCount = SampleCount;
    HHA->Sound.NextIDToPlay.Value = 0;

    Source->FileName = FileName;
    Source->Type = AssetType_Sound;
    Source->FirstSampleIndex = FirstSampleIndex;
    
    Assets->AssetIndex = Result.Value;

    return Result;
}

internal void EndAssetType(game_assets *Assets)
{
  Assert(Assets->DEBUGAssetType);
  Assets->AssetCount = Assets->DEBUGAssetType->OnePastLastAssetIndex;
  Assets->DEBUGAssetType = 0;
  Assets->AssetIndex = 0;
}

internal void AddTag(game_assets *Assets, asset_tag_id ID, real32 Value)
{ 
  Assert(Assets->AssetIndex);
  hha_asset *HHA = Assets->Assets + Assets->AssetIndex;
  ++HHA->OneLastPastTagIndex;
  hha_tag* Tag = Assets->Tags + Assets->TagCount++;

  Tag->ID = ID;
  Tag->Value = Value;
}

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

struct entire_file
{
  uint32 ContentSize;
  void *Contents;
};

entire_file ReadEntireFile(char *FileName)
{
  entire_file Result = {};
  
  FILE *In = fopen(FileName, "rb");
  if(In)
  {
    fseek(In, 0, SEEK_END);
    Result.ContentSize = ftell(In);
    fseek(In, 0, SEEK_SET);
    
    Result.Contents = malloc(Result.ContentSize);
    fread(Result.Contents, Result.ContentSize, 1, In);
    fclose(In);
  }
  else
  {
    printf("ERROR: Can't find the file!");
  }  
  return Result;
}

#define BITMAP_BYTES_PER_PIXEL 4

internal loaded_bitmap LoadBMP(char *FileName)
{
  loaded_bitmap Result = {};
  entire_file ReadResult = ReadEntireFile(FileName);
  Result.Free = ReadResult.Contents;
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
    for(int32 Y = 0; Y < BitMap->Height; ++Y)
    {
      for(int32 X = 0; X < BitMap->Width; ++X)
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


internal loaded_sound LoadWAV(char* FileName, uint32 SectionSampleIndex, uint32 SectionSampleCount)
{
  loaded_sound Result = {};
  entire_file ReadResult = ReadEntireFile(FileName);
  Result.Free = ReadResult.Contents;

  if(ReadResult.ContentSize != 0)
  {
    WAVE_header* Header = (WAVE_header*)ReadResult.Contents;
    Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
    Assert(Header-> WAVEID == WAVE_ChunkID_WAVE);
    int16 *SampleData = 0;
    uint32 ChannelCount = 0;
    uint32 SampleDataSize = 0;
    for(riff_iterator Iter = ParseChunkAt((Header + 1), (uint8 *)(Header + 1) + Header->Size - 4); IsValid(Iter); Iter = GetNextChunk(Iter))
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
	/*for(uint32 SampleIndex = SampleCount; SampleIndex < SampleCount + 8; ++SampleIndex)
	{
	  Result.Samples[ChannelIndex][SampleIndex] = 0;
	  }*/
      }
    }

    Result.SampleCount = SampleCount;
  }
  return Result;
}


int main(int ArgCount, char **Args)
{
  game_assets Assets_;
  game_assets *Assets = &Assets_;

  Assets->AssetCount = 1;
  Assets->TagCount = 1;
  Assets->DEBUGAssetType = 0;
  Assets->AssetIndex = 0;
  
  BeginAssetType(Assets, Asset_BackGround);
  AddBitmapAsset(Assets, "test/test_img.bmp", 0.5f, 0.5f);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Wall);
  AddBitmapAsset(Assets, "test/brick.bmp", 0.5f, 0.0f);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Monster);
  AddBitmapAsset(Assets, "test/monster.bmp", 0.4f, 0.05f);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Sword);
  AddBitmapAsset(Assets, "test/fireball.bmp", 0.5f, 0.5f);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Stair);
  AddBitmapAsset(Assets, "test/staff.bmp", 0.5f, 0.5f);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Grass);
  AddBitmapAsset(Assets, "test/Grass.bmp");
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_Dirt);
  AddBitmapAsset(Assets, "test/Dirt.bmp");
  EndAssetType(Assets);
  
#define Tau32 6.28318530718f

  real32 AngleLeft = 0.5f*Tau32;
  real32 AngleRigth = 0*Tau32;

  BeginAssetType(Assets, Asset_Wizard);
  AddBitmapAsset(Assets, "test/mage1.bmp", 0.5f, 0.05f);
  AddTag(Assets, Tag_Facing_Direction, AngleRigth);
  AddBitmapAsset(Assets, "test/mage2.bmp", 0.5f, 0.05f);
  AddTag(Assets, Tag_Facing_Direction, AngleLeft);
  EndAssetType(Assets);

  BeginAssetType(Assets, Asset_FireSound);
  AddSoundAsset(Assets, "test/fire.wav");
  EndAssetType(Assets);
  
  BeginAssetType(Assets, Asset_DungeonSound);
  AddSoundAsset(Assets, "test/dungeon.wav");
  EndAssetType(Assets);

  FILE *Out = fopen("test.fam", "wb");
  if(Out)
  {
    hha_header Header = {};
    Header.HeaderCode = HHA_HEADER_CODE;
    Header.Version = HHA_VERSION;
    Header.TagCount = Assets->TagCount;
    Header.AssetCount = Assets->AssetCount;
    Header.AssetTypeCount = Asset_Count;

    uint32 TagArraySize = Header.TagCount*sizeof(hha_tag);
    uint32 AssetTypeArraySize = Header.AssetTypeCount*sizeof(hha_asset_type);
    uint32 AssetArraySize = Header.AssetCount*sizeof(hha_asset);
    
    Header.TagsOffset = sizeof(Header);
    Header.AssetTypeOffset = Header.TagsOffset + Header.TagCount*sizeof(hha_tag);
    Header.AssetOffset = Header.AssetTypeOffset + Header.AssetTypeCount*sizeof(hha_asset_type);
    
    fwrite(&Header, sizeof(Header), 1, Out);
    fwrite(Assets->Tags, TagArraySize, 1, Out);
    fwrite(Assets->AssetTypes, AssetTypeArraySize, 1, Out);
    //fwrite(&AssetArray, AssetArraySize, 1, Out);
    fseek(Out, AssetArraySize, SEEK_CUR);
    for(uint32 AssetIndex = 1; AssetIndex < Header.AssetCount; ++AssetIndex)
    {
      asset_source *Source = Assets->AssetsSources + AssetIndex;
      hha_asset *Dest = Assets->Assets + AssetIndex;
      Dest->DataOffset = ftell(Out);

      if(Source->Type == AssetType_Sound)
      {
	loaded_sound WAV = LoadWAV(Source->FileName, Source->FirstSampleIndex, Dest->Sound.SampleCount);
	Dest->Sound.SampleCount = WAV.SampleCount;
	Dest->Sound.ChannelCount = WAV.ChannelCount;
	for(uint32 ChannelIndex = 0; ChannelIndex < WAV.ChannelCount; ++ChannelIndex)
	{
	  fwrite(WAV.Samples[ChannelIndex], Dest->Sound.SampleCount*sizeof(int16), 1, Out);
	}
	free(WAV.Free);
      }
      else
      {
	Assert(Source->Type == AssetType_Bitmap);
	loaded_bitmap Bitmap = LoadBMP(Source->FileName);
	Dest->Bitmap.Dim[0] = Bitmap.Width;
	Dest->Bitmap.Dim[1] = Bitmap.Height;

	Assert((Bitmap.Width*4) == Bitmap.Pitch);
	fwrite(Bitmap.Memory, Bitmap.Width*Bitmap.Height*4, 1, Out);
	
	free(Bitmap.Free);
      }
    }
    fseek(Out, (uint32)Header.AssetOffset, SEEK_SET);
    fwrite(Assets->Assets, AssetArraySize, 1, Out);
    fclose(Out);
  }
  else
  {
    printf("ERROR: file can't be open!");
  }
#if 0
#endif
}
