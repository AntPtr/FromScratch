#include "handmade_test_asset_builder.h"

#define USE_FONT_FROM_WINDOWS 1

#if USE_FONT_FROM_WINDOWS
#include <windows.h>
#else
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#endif

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

struct added_asset
{
  uint32 ID;
  hha_asset *HHA;
  asset_source *Source;
};

internal added_asset AddAsset(game_assets *Assets)
{
  Assert(Assets->DEBUGAssetType);
  uint32 Index = Assets->DEBUGAssetType->OnePastLastAssetIndex++;
  asset_source *Source = Assets->AssetsSources + Index;
  hha_asset *HHA = Assets->Assets + Index;
  HHA->FirstTagIndex = Assets->TagCount;
  HHA->OneLastPastTagIndex = HHA->FirstTagIndex;
  Assets->AssetIndex = Index;

  added_asset Result;
  Result.ID = Index;
  Result.HHA = HHA;
  Result.Source = Source;
  return Result;
}

internal bitmap_id AddBitmapAsset(game_assets *Assets, char *FileName, real32 AlignPercentageX = 0.5f, real32 AlignPercentageY = 0.5f)
{
  added_asset Asset = AddAsset(Assets);
  Asset.HHA->Bitmap.AlignPercentage[0] = AlignPercentageX;
  Asset.HHA->Bitmap.AlignPercentage[1] = AlignPercentageY;
  Asset.Source->Bitmap.FileName = FileName;
  Asset.Source->Type = AssetType_Bitmap;

  bitmap_id Result = {Asset.ID};
  return Result;
}

internal sound_id AddSoundAsset(game_assets* Assets, char* FileName, uint32 FirstSampleIndex = 0, uint32 SampleCount = 0)
{
  added_asset Asset = AddAsset(Assets);
  Asset.HHA->Sound.SampleCount = SampleCount;
  Asset.HHA->Sound.Chain = HHASoundChain_None;
  Asset.Source->Sound.FileName = FileName;
  Asset.Source->Type = AssetType_Sound;
  Asset.Source->Sound.FirstSampleIndex = FirstSampleIndex;

  sound_id Result = {Asset.ID};
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

struct loaded_font
{
  uint32 CodePointCount;
  real32 LineAdvance;
  HFONT Win32Handle;
  TEXTMETRIC TextMetric;
  bitmap_id *BitmapIDs;
  real32 *HorizontalAdvance;
};

#define MAX_FONT_WIDTH 1024;
#define MAX_FONT_HEIGHT 1024;
global_variable HDC GlobalFontDeviceContext = 0;
global_variable VOID *GlobalFontBits = 0;

internal loaded_font *LoadFont(char *FileName, char *FontName, uint32 CodePointCount)
{
  loaded_font *Result = (loaded_font *)malloc(sizeof(loaded_font));
  AddFontResourceExA(FileName, FR_PRIVATE, 0);
  int Height = 128;
  Result->Win32Handle = CreateFontA(Height, 0, 0, 0, FW_DONTCARE,
			   FALSE, FALSE, FALSE, DEFAULT_CHARSET,
			   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			   ANTIALIASED_QUALITY,
			   DEFAULT_PITCH|FW_DONTCARE, FontName);

  SelectObject(GlobalFontDeviceContext, Result->Win32Handle);
  GetTextMetrics(GlobalFontDeviceContext, &Result->TextMetric);
 
  Result->LineAdvance =(real32)(Result->TextMetric.tmHeight + Result->TextMetric.tmExternalLeading);
  Result->CodePointCount = CodePointCount;
  Result->BitmapIDs = (bitmap_id *)malloc(sizeof(bitmap_id)*CodePointCount);
  Result->HorizontalAdvance = (real32 *)malloc(sizeof(real32)*CodePointCount*CodePointCount);
  ABC *ABCs = (ABC *)malloc(sizeof(ABC)*CodePointCount);
  GetCharABCWidthsW(GlobalFontDeviceContext, 0, (Result->CodePointCount - 1), ABCs);
  
  for(uint32 CodePointIndex = 0; CodePointIndex < Result->CodePointCount; ++CodePointIndex)
  {
    ABC *This = ABCs + CodePointIndex;
    real32 W = (real32)This->abcA + (real32)This->abcB + (real32)This->abcC;
    for(uint32 OtherCodePoint = 0; OtherCodePoint < Result->CodePointCount; ++OtherCodePoint)
    {
      Result->HorizontalAdvance[CodePointIndex*Result->CodePointCount + OtherCodePoint] = (real32)W;
    }
  }

  DWORD KerningPairCount = GetKerningPairsW(GlobalFontDeviceContext, 0, 0);
  KERNINGPAIR *KerningPairs = (KERNINGPAIR *)malloc(sizeof(KERNINGPAIR)*KerningPairCount);
  GetKerningPairsW(GlobalFontDeviceContext, KerningPairCount, KerningPairs);

  for(DWORD KerningPairIndex = 0; KerningPairIndex < KerningPairCount; ++KerningPairIndex)
  {
    KERNINGPAIR *Pair = KerningPairs + KerningPairIndex;
    if((Pair->wFirst < Result->CodePointCount) && (Pair->wSecond < Result->CodePointCount))
    {
      Result->HorizontalAdvance[Pair->wFirst*Result->CodePointCount + Pair->wSecond] += (real32)Pair->iKernAmount;
    }
  }
  free(KerningPairs);
  return Result;
}

internal void InitializeFontDC()
{
  GlobalFontDeviceContext = CreateCompatibleDC(0);

  BITMAPINFO Info = {};    
  Info.bmiHeader.biSize = sizeof(Info.bmiHeader);
  Info.bmiHeader.biWidth = MAX_FONT_WIDTH;
  Info.bmiHeader.biHeight = MAX_FONT_HEIGHT;
  Info.bmiHeader.biPlanes = 1;
  Info.bmiHeader.biBitCount = 32;
  Info.bmiHeader.biCompression = BI_RGB;
  Info.bmiHeader.biSizeImage = 0;
  Info.bmiHeader.biXPelsPerMeter = 0;
  Info.bmiHeader.biYPelsPerMeter = 0;
  Info.bmiHeader.biClrUsed = 0;
  Info.bmiHeader.biClrImportant = 0;
  HBITMAP Bitmap = CreateDIBSection(GlobalFontDeviceContext, &Info, DIB_RGB_COLORS, &GlobalFontBits, 0, 0);
  SelectObject(GlobalFontDeviceContext, Bitmap);
  SetBkColor(GlobalFontDeviceContext, RGB(0, 0 ,0));
}

internal loaded_bitmap LoadGlyphBitmap(loaded_font *Font, uint32 CodePoint, hha_asset *Asset)
{
  loaded_bitmap Result = {};
  int MaxWidth = MAX_FONT_WIDTH;
  int MaxHeight = MAX_FONT_HEIGHT;
#if USE_FONT_FROM_WINDOWS
  
  SelectObject(GlobalFontDeviceContext, Font->Win32Handle);
#if 0
  ABC ThisABC;
  GetCharABCWidthsW(GlobalFontDeviceContext, CodePoint, CodePoint, &ThisABC);
#endif
  wchar_t CheesePoint = (wchar_t)CodePoint;

  memset(GlobalFontBits, 0x00, MaxWidth*MaxHeight*sizeof(uint32));
  
  SIZE Size;
  GetTextExtentPoint32W(GlobalFontDeviceContext, &CheesePoint, 1, &Size); 

  int PreStepX = 128;
  
  int BoundWidth = Size.cx + PreStepX*2;
  if(BoundWidth > MaxWidth)
  {
    BoundWidth = MaxWidth;
  }
  
  int BoundHeight = Size.cy;
  if(BoundHeight > MaxHeight)
  {
    BoundHeight = MaxHeight;
  }
  
  SetBkMode(GlobalFontDeviceContext, OPAQUE);
  SetBkColor(GlobalFontDeviceContext, RGB(0, 0, 0));
  SetTextColor(GlobalFontDeviceContext, RGB(255, 255, 255));
  PatBlt(GlobalFontDeviceContext, 0, 0, 1024, 1024, BLACKNESS);
  TextOutW(GlobalFontDeviceContext, PreStepX, 0, &CheesePoint, 1);

  int MinX = 10000;
  int MinY = 10000;
  int MaxX = -10000;
  int MaxY = -10000;

  uint32 *Row = (uint32 *)GlobalFontBits + MaxWidth*(MaxHeight - 1);
  
  for(int Y = 0; Y < BoundHeight; ++Y)
  {
    uint32 *Pixel = Row;
    for(int X = 0; X < BoundWidth; ++X)
    {
      //COLORREF Pixel = GetPixel(DeviceContext, X, Y);
      if(*Pixel != 0)
      {
	if(MinX > X)
	{
	  MinX = X;
	}
      
	if(MinY > Y)
        {
	  MinY = Y;
	}

	if(MaxX < X)
        {
	  MaxX = X;
	}

	if(MaxY < Y)
        {
	  MaxY = Y;
	}
      }
      ++Pixel;
    }
    Row -= MaxWidth;
  }
  
  if(MinX <= MaxX)
  {
    int Width = (MaxX - MinX) + 1;
    int Height = (MaxY - MinY) + 1;

    Result.Width = Width + 2;
    Result.Height = Height + 2;
    Result.Pitch = Result.Width*BITMAP_BYTES_PER_PIXEL;
    Result.Memory = malloc(Result.Height*Result.Pitch);
    Result.Free = Result.Memory;

    memset(Result.Memory, 0, Result.Height*Result.Pitch);
    
    uint8 *DestRow = (uint8 *)Result.Memory + (Result.Height - 1 - 1)*Result.Pitch;
    uint32 *SourceRow = (uint32 *)GlobalFontBits + MaxWidth*(MaxHeight - 1 - MinY);
    
    for(int Y = MinY; Y <= MaxY; ++Y)
    {
      uint32 *Dest = (uint32 *)DestRow + 1;
      uint32 *Source =  (uint32 *)SourceRow + MinX;
      for(int X = MinX; X <= MaxX; ++X)
      {
	uint32 Pixel = *Source;
	real32 Alpha = 0;
    
	Alpha = real32(Pixel & 0xFF);
	v4 Texel = {255.0f, 255.0f, 255.0f, Alpha};
	Texel = SRGB255ToLinear1(Texel);
	Texel.rgb *= Texel.a;
	Texel = Linear1ToSRGB255(Texel);
	*Dest++ = (((uint32(Texel.a + 0.5f)) << 24) |
		   ((uint32(Texel.r + 0.5f)) << 16) |
		   ((uint32(Texel.g + 0.5f)) << 8) |
		   ((uint32(Texel.b + 0.5f)) << 0));
	++Source;
      }
       DestRow -= Result.Pitch;
       SourceRow -= MaxWidth;
    }

    Asset->Bitmap.AlignPercentage[0] = (1.0f - (MinX - PreStepX)) / (real32)Result.Width;
    Asset->Bitmap.AlignPercentage[1] = (1.0f + (MaxY - (BoundHeight - Font->TextMetric.tmDescent))) / (real32)Result.Height;
  }
  
#else
  entire_file TTFFile = ReadEntireFile(FileName);
     
  stbtt_fontinfo Font;
  stbtt_InitFont(&Font, (uint8 *)(TTFFile.Contents), stbtt_GetFontOffsetForIndex((uint8 *)(TTFFile.Contents), 0));

  int Width, Height, XOffset, YOffset;
  uint8 *MonoBitmap = stbtt_GetCodepointBitmap(&Font, 0,stbtt_ScaleForPixelHeight(&Font, 120.0f), CodePoint, &Width, &Height, &XOffset, &YOffset);

  Result.Pitch = Width*BITMAP_BYTES_PER_PIXEL;
  Result.Width = Width;
  Result.Height = Height;
  Result.Memory = malloc(Result.Height*Result.Pitch);
  Result.Free = Result.Memory;

  
  uint8 *Source = MonoBitmap;
  uint8 *DestRow = (uint8 *)Result.Memory + (Height - 1)*Result.Pitch;
  
  for(int Y = 0; Y < Height; ++Y)
  {
    uint32 *Dest = (uint32 *)DestRow;
    for(int X = 0; X < Width; ++X)
    {
      uint8 Alpha = *Source++;
      *Dest++ = ((Alpha << 24)|
		 (Alpha << 16)|
		 (Alpha << 8)|
		 (Alpha << 0));
    }
    DestRow -= Result.Pitch; 
  }
  stbtt_FreeBitmap(MonoBitmap, 0);
  free(TTFFile.Contents);
#endif  
  return Result;    
}


internal void WriteHHA(game_assets *Assets, char *Filename)
{
  FILE *Out = fopen(Filename, "wb");
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
	loaded_sound WAV = LoadWAV(Source->Sound.FileName, Source->Sound.FirstSampleIndex, Dest->Sound.SampleCount);
	Dest->Sound.SampleCount = WAV.SampleCount;
	Dest->Sound.ChannelCount = WAV.ChannelCount;
	for(uint32 ChannelIndex = 0; ChannelIndex < WAV.ChannelCount; ++ChannelIndex)
	{
	  fwrite(WAV.Samples[ChannelIndex], Dest->Sound.SampleCount*sizeof(int16), 1, Out);
	}
	free(WAV.Free);
      }
      else if(Source->Type == AssetType_FontGlyph)
      {
	loaded_bitmap Bitmap = LoadGlyphBitmap(Source->Glyph.Font, Source->Glyph.CodePoint, Dest);
	Dest->Bitmap.Dim[0] = Bitmap.Width;
	Dest->Bitmap.Dim[1] = Bitmap.Height;
	Assert((Bitmap.Width*4) == Bitmap.Pitch);
	fwrite(Bitmap.Memory, Bitmap.Width*Bitmap.Height*4, 1, Out);	
	free(Bitmap.Free);
      }
      else if(Source->Type == AssetType_Font)
      {
	loaded_font *Font = Source->Font.Font; 
	uint32 CodePointsSize = sizeof(bitmap_id)*Font->CodePointCount;
	uint32 HorizontalAdvanceSize = sizeof(real32)*Font->CodePointCount*Font->CodePointCount;
	fwrite(Font->BitmapIDs, CodePointsSize, 1, Out);
	fwrite(Font->HorizontalAdvance, HorizontalAdvanceSize, 1, Out);
      }
      else
      {
	Assert(Source->Type == AssetType_Bitmap);
	loaded_bitmap Bitmap = LoadBMP(Source->Bitmap.FileName);
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
}

internal void FreeFont(loaded_font *Font)
{
  DeleteObject(Font->Win32Handle);
  free(Font);
}

internal bitmap_id AddCharcterAsset(game_assets *Assets, loaded_font *Font, uint32 CodePoint, real32 AlignPercentageX = 0.5f, real32 AlignPercentageY = 0.5f)
{
  added_asset Asset = AddAsset(Assets);
  Asset.HHA->Bitmap.AlignPercentage[0] = AlignPercentageX;
  Asset.HHA->Bitmap.AlignPercentage[1] = AlignPercentageY; 
  Asset.Source->Type = AssetType_FontGlyph;
  Asset.Source->Glyph.CodePoint = CodePoint;
  Asset.Source->Glyph.Font = Font;

  bitmap_id Result = {Asset.ID};
  return Result;
}

internal font_id AddFontAsset(game_assets *Assets, loaded_font *Font)
{
  added_asset Asset = AddAsset(Assets);
  Asset.HHA->Font.CodePointCount = Font->CodePointCount;
  Asset.HHA->Font.LineAdvance = Font->LineAdvance;
  
  Asset.Source->Font.Font = Font;
  Asset.Source->Type = AssetType_Font;
  
  font_id Result = {Asset.ID};
  return Result;
}


internal void Initialize(game_assets *Assets)
{
  Assets->AssetCount = 1;
  Assets->TagCount = 1;
  Assets->DEBUGAssetType = 0;
  Assets->AssetIndex = 0;

  Assets->AssetTypeCount = Asset_Count;
  memset(Assets->AssetTypes, 0, sizeof(Assets->AssetTypes));
}

internal void WriteHero()
{
#define Tau32 6.28318530718f

  game_assets Assets_;
  game_assets *Assets = &Assets_;

  Initialize(Assets);
  
  real32 AngleLeft = 0.5f*Tau32;
  real32 AngleRigth = 0*Tau32;

  BeginAssetType(Assets, Asset_Wizard);
  AddBitmapAsset(Assets, "test/mage1.bmp", 0.5f, 0.05f);
  AddTag(Assets, Tag_Facing_Direction, AngleRigth);
  AddBitmapAsset(Assets, "test/mage2.bmp", 0.5f, 0.05f);
  AddTag(Assets, Tag_Facing_Direction, AngleLeft);
  EndAssetType(Assets);

  WriteHHA(Assets, "test1.hha");
}

internal void WriteNonHero()
{
  game_assets Assets_;
  game_assets *Assets = &Assets_;

  Initialize(Assets);
  
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

  BeginAssetType(Assets, Asset_Familiar);
  AddBitmapAsset(Assets, "test/Hank.bmp");
  EndAssetType(Assets);
  
  WriteHHA(Assets, "test2.hha");
}

internal void WriteFont()
{
  game_assets Assets_;
  game_assets *Assets = &Assets_;
  Initialize(Assets);

  loaded_font *DebugFont = LoadFont( "c:/Windows/Fonts/arial.ttf", "Arial", ( '~' + 1)); 
  BeginAssetType(Assets, Asset_Fonts);
  AddFontAsset(Assets, DebugFont);
  EndAssetType(Assets);
  
  BeginAssetType(Assets, Asset_FontGlyph);
  for(uint32 Char = '!'; Char <= '~'; ++Char)
  {
    DebugFont->BitmapIDs[Char] = AddCharcterAsset(Assets, DebugFont, Char);
  }
  //FreeFont(DebugFont);
  EndAssetType(Assets);

  WriteHHA(Assets, "test_font.hha");
}

internal void WriteSounds()
{
  game_assets Assets_;
  game_assets *Assets = &Assets_;

  Initialize(Assets);
   
  BeginAssetType(Assets, Asset_FireSound);
  AddSoundAsset(Assets, "test/fire.wav");
  EndAssetType(Assets);
  
  BeginAssetType(Assets, Asset_DungeonSound);
  AddSoundAsset(Assets, "test/dungeon.wav");
  EndAssetType(Assets);

  WriteHHA(Assets, "test3.hha");

}

int main(int ArgCount, char **Args)
{
  InitializeFontDC();
  WriteFont();
  WriteHero();
  WriteNonHero();
  WriteSounds();
}
