#include "handmade.h"
#include "handmade_tile.cpp"

internal void SomeGradient(game_offscreen_buffer *Buffer ,int XOffset,int YOffset)
{
  uint8 *Row = (uint8 *)Buffer->Memory;
  uint32 *Pixel = (uint32 *)Row;

  for(int Y = 0; Y < Buffer->Height; ++Y)
  {
    Pixel = (uint32 *)Row;
    for(int X = 0; X < Buffer->Width; ++X)
    {
      uint8 Blue =(uint8)(X + XOffset);
      uint8 Green =(uint8)(Y + YOffset);

      *Pixel++ = ((Green << 8)| Blue);
    }
     Row += Buffer->Pitch;
  }
}

internal void DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax, real32 R, real32 G, real32 B)
{
  int32 MinX = RoundReal32ToInt32(vMin.X);
  int32 MaxX = RoundReal32ToInt32(vMax.X);
  int32 MinY = RoundReal32ToInt32(vMin.Y);
  int32 MaxY = RoundReal32ToInt32(vMax.Y);

  if(MinX < 0)
  {
    MinX = 0;
  }
  if(MaxX > Buffer->Width)
  {
    MaxX = Buffer->Width;
  }
  if(MinY < 0)
  {
    MinY = 0;
  }
  if(MaxY > Buffer->Height)
  {
    MaxY = Buffer->Height;
  }
 
  uint32 Color = (uint32)(RoundReal32ToUInt32(255 * R) << 16 | RoundReal32ToUInt32(255 * G) << 8 | RoundReal32ToUInt32(255 *B) << 0);
    
  uint8* Row = ((uint8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch); 

  for(int Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = MinX; X < MaxX; ++X)
    {
      *Pixel++ = Color;
    }
    Row += Buffer->Pitch;
  }
}

internal void RenderPlayer(game_offscreen_buffer *Buffer, int PlayerX, int PlayerY)
{
  uint32 Color = 0xFFFFFFFF;
  uint8 *EndOfBuffer = ((uint8 *)Buffer->Memory + Buffer->Height * Buffer->Pitch);
  int Left = PlayerX;
  int Rigth = PlayerX + 10;
  int Top = PlayerY;
  int Bottom = PlayerY + 10;
 
  for(int X = Left; X < Rigth; ++X)
  {
    uint8* Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + Top * Buffer->Pitch); 
    for(int Y = Top; Y < Bottom; ++Y)
    {
      if(Pixel >= Buffer->Memory && Pixel < EndOfBuffer)
      {
	*(uint32 *)Pixel = Color;
        Pixel += Buffer->Pitch;
      }
    }
  }
}
 
void GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 *SampleOut = SoundBuffer->Samples;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    for(uint16 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
#if 0
      real32 SineValue = sinf(GameState->tSine);
      int16 SampleValue =(int16)(SineValue*ToneVolume);
#else
      int16 SampleValue = 0;
#endif
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
#if 0
      GameState->tSine +=  2.0f*Pi32*1.0f/(real32)WavePeriod;
      if(GameState->tSine > 2.0 * Pi32)
      {
	GameState->tSine -= 2.0 * Pi32;
      }
#endif
    }
}

uint32 RandomNumberTable[] =
{
  0x1fb1ca2b, 0x010140e2, 0x0fe18c2d, 0x239217d9, 0x16429a54,
  0x25f567c4, 0x27a98a49, 0x2fd4b1ab, 0x07a61d61, 0x10d7d90e,
  0x2706b1da, 0x02f9d117, 0x0e26e72a, 0x11689541, 0x039e66b8,
  0x2c922530, 0x0226138a, 0x03945fb5, 0x2480642c, 0x0fd474cd,
  0x31b72ebc, 0x02e2e35d, 0x18d1a2e4, 0x3adfe9c8, 0x2bbc23ac,
  0x2a60421b, 0x37b12202, 0x2fc8260b, 0x0d8d7d85, 0x14664c8b,
  0x0537fa32, 0x32bde25c, 0x0f1a55b7, 0x23107167, 0x2a6effa5,
  0x338371bb, 0x04411e92, 0x062300dc, 0x0ff517bd, 0x0675f483,
  0x10b74de6, 0x24244dbc, 0x14279d96, 0x239cb484, 0x2ad0d1bd,
  0x23686f42, 0x3a2e6b22, 0x37dcffd7, 0x3524f3f6, 0x00575a8d,
  0x16fa59f6, 0x2fb54eb2, 0x32a99388, 0x27d68a18, 0x178e30b5,
  0x363c33e9, 0x061604bf, 0x0ecc4290, 0x1f24a7ec, 0x0b58d639,
  0x20c90d25, 0x2ffea680, 0x2b2b59c5, 0x19fafde0, 0x38dde781,
  0x1df80f84, 0x3af3d989, 0x04f201f7, 0x3249ae17, 0x18286756,
  0x232127dc, 0x0cb28376, 0x0b1ffbda, 0x06b407a4, 0x38c3f6ac,
  0x058667af, 0x30d1d095, 0x0b32c7c4, 0x3b1ba04f, 0x33c9939a,
  0x26f168bb, 0x157c58d5, 0x014eb6c1, 0x28104abd, 0x141e5982,
  0x19247b1b, 0x0b09670c, 0x1b5f680e, 0x124ec969, 0x23f92f1b,
  0x350c155d, 0x212d6f54, 0x2ca8d5e5, 0x1740523d, 0x1bd253fb,
  0x06f01c68, 0x0aa3a8d0, 0x19c51265, 0x07a3655c, 0x1556d799,
  0x1d6693a8, 0x373c0c69, 0x380dcc41, 0x238e229e, 0x1e778cf2,
  0x1ceb16fb, 0x2b8c20be, 0x332bde79, 0x39d0a7c1, 0x11c3c71c,
  0x17f876e0, 0x09aa2afb, 0x0c4495a8, 0x31b1a22d, 0x249d60d8,
  0x18aa922e, 0x3734b0d6, 0x3a899143, 0x17ff8b8d, 0x04532389,
  0x32f42402, 0x1ed53017, 0x1f0dc7b0, 0x17cb2dae, 0x3726b692,
  0x3b94ec84, 0x1e13f900, 0x235444fb, 0x33465a62, 0x2c4bf650,
  0x167dad84, 0x31a8e0e9, 0x00928289, 0x2b470f2a, 0x097fc1dc,
  0x163cdc20, 0x023706da, 0x336a5db5, 0x08c5cc1b, 0x33212158,
  0x2edfadee, 0x1d08f1c0, 0x1d3efcb6, 0x0cfa901e, 0x30f2ce7b,
  0x377169c5, 0x30765b84, 0x118d75b8, 0x1422a324, 0x3b01b81c,
  0x1b861c9c, 0x220891c3, 0x19a9a069, 0x2185b242, 0x34e118c0,
  0x263b9427, 0x365f5f37, 0x0a938332, 0x0f52a66b, 0x09c9fbf4,
  0x246e6492, 0x26a765d8, 0x13b64c90, 0x0e98d9ec, 0x3b04528f,
  0x1d3ecc58, 0x165a91a8, 0x1169920d, 0x130a7608, 0x08a8a526,
  0x194c32fc, 0x098efe44, 0x305d341c, 0x23faf8e0, 0x2ac600af,
  0x3ae41eb4, 0x3a84eceb, 0x0d0a6ddb, 0x130191c5, 0x30c3b263,
  0x0ed300c1, 0x18709554, 0x317deb4f, 0x06b57ae1, 0x29d9b80b,
  0x0960cbbf, 0x0434a73f, 0x13b59938, 0x02a7aad2, 0x197a8ef8,
  0x064860fe, 0x266eca99, 0x0520ce8f, 0x202cfba4, 0x17a8e22f,
  0x14d41637, 0x07aefb47, 0x38b8df94, 0x0d80882e, 0x01b26616,
  0x1a77ba9f, 0x1c89c9c6, 0x1e0a6e0c, 0x2994af89, 0x2402a196,
  0x27e40103, 0x387936e2, 0x351a1b97, 0x11be5cb7, 0x1a38e11f,
  0x140f4f71, 0x3a5cac79, 0x1796f90b, 0x2ef1f493, 0x1b83d8b9,
  0x208bf0bf, 0x25f9efed, 0x33455009, 0x34024573, 0x27fd40a6,
  0x09246230, 0x21f84d2b, 0x131f85a2, 0x20943d98
};

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

internal loaded_bitmap DEBUGLoadBMP(thread_context *Theard, debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{

  loaded_bitmap Result = {};
  debug_read_file_result ReadResult = ReadEntireFile(Theard, FileName);
  if(ReadResult.ContentSize > 0)
  {
    bitmap_header *BitMap = (bitmap_header *)ReadResult.Contents;
    uint32 *Pixel = (uint32 *)((uint8 *)ReadResult.Contents  + BitMap->BitmapOffset);

    Assert(BitMap->Compression == 3);
    
    uint32 RedMask = BitMap->RedMask;
    uint32 GreenMask = BitMap->GreenMask;
    uint32 BlueMask = BitMap ->BlueMask;
    uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

    bitscan_result RedShift = FindLastSignificantBit(RedMask);
    bitscan_result GreenShift = FindLastSignificantBit(GreenMask);
    bitscan_result BlueShift = FindLastSignificantBit(BlueMask);
    bitscan_result AlphaShift = FindLastSignificantBit(AlphaMask);


    uint32 *SourceDest = Pixel;
    for(int32 Y = 0; Y < BitMap->Width; ++Y)
    {
      for(int32 X = 0; X < BitMap->Height; ++X)
      {
	uint32 C = *SourceDest;
	*SourceDest++ = ((((C >> AlphaShift.Index) & 0xFF) << 24) |
	                (((C >> RedShift.Index) & 0xFF) << 16) |
			 (((C >> GreenShift.Index) & 0xFF) << 8) |
			 (((C >> BlueShift.Index) & 0xFF) << 0));
      }
    }
    
    Result.Pixels = Pixel;
    Result.Width = BitMap->Width;
    Result.Height = BitMap->Height;
  }
  return Result;
}

internal void TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY, real32 *tMin, real32 MinY, real32 MaxY)
{
  real32 tEpsilon = 0.01f;
  if(PlayerDeltaX != 0.0f)
  {
    real32 tResult = (WallX - RelX) / PlayerDeltaX;
    real32 Y = RelY  + tResult * PlayerDeltaY;
    if((tResult >= 0.0f) && (*tMin > tResult))
    {
      if((Y >= MinY) && (Y <= MaxY))
      {
	*tMin = Maximum(0.0f, tResult - tEpsilon);
      }
    }
  }
}

void MovePlayer(game_state *GameState, entity *Entity, real32 dt, v2 ddPlayer)
{
      real32 Velocity = 30.0f;
      tile_map *TileMap = GameState->World->TileMap;

      real32 ddPlayerLenght = LenghtSq(ddPlayer);
      if(ddPlayerLenght > 1.0f)
      {
	ddPlayer *= 1.0f/SquareRoot(ddPlayerLenght);
      }

      tile_map_position OldPlayerP = Entity->P;
      ddPlayer *= Velocity;
      ddPlayer += -8.0f*Entity->dvP;
      
      v2 PlayerDelta = 0.5f*ddPlayer*Square(dt) + Entity->dvP*dt;
      Entity->dvP = ddPlayer*dt + Entity->dvP;
      
      tile_map_position NewPlayerP = Offset(TileMap, OldPlayerP, PlayerDelta);

#if 0
      tile_map_position PlayerL = NewPlayerP;
      PlayerL.Offset.X -= 0.5f * Entity->Width;
      PlayerL = ReCanonicalizePosition(TileMap, PlayerL);
      
      tile_map_position PlayerR = NewPlayerP;
      PlayerR.Offset.X += 0.5f * Entity->Width;
      PlayerR = ReCanonicalizePosition(TileMap, PlayerR);

      bool32 Collided = false;
      tile_map_position CollP = {};
      if(!IsTileMapPointEmpty(TileMap, NewPlayerP))
      {
	Collided = true;
	CollP = NewPlayerP;
      }
      if(!IsTileMapPointEmpty(TileMap, PlayerL))
      {
	Collided = true;
	CollP = PlayerL;
      }
      if(!IsTileMapPointEmpty(TileMap, PlayerR))
      {
	Collided = true;
	CollP = PlayerR;
      }
      if(Collided)
      {
	v2 R = v2{0, 0};
	if(CollP.AbsTileX > Entity->P.AbsTileX)
	{
	  R = v2{-1, 0};
	}
	if(CollP.AbsTileX < Entity->P.AbsTileX)
	{
	  R = v2{1, 0};
	}
	if(CollP.AbsTileY > Entity->P.AbsTileY)
	{
	  R = v2{0, -1};
	}
	if(CollP.AbsTileY < Entity->P.AbsTileY)
	{
	  R = v2{0, 1};
	}
	Entity->dvP = Entity->dvP - 1*DotProduct(R, Entity->dvP)*R;
      }
      else
      {
	Entity->P = NewPlayerP;	
      }
#else
      /*uint32 MinTileX = Minimum(OldPlayerP.AbsTileX, NewPlayerP.AbsTileX);
      uint32 MinTileY = Minimum(OldPlayerP.AbsTileY, NewPlayerP.AbsTileY);
      uint32 OnePastLastMaxTileX = Maximum(OldPlayerP.AbsTileX, NewPlayerP.AbsTileX) + 1;
      uint32 OnePastLastMaxTileY = Maximum(OldPlayerP.AbsTileY, NewPlayerP.AbsTileY) + 1;
      */

      uint32 StartTileX = OldPlayerP.AbsTileX;
      uint32 StartTileY = OldPlayerP.AbsTileY;
      uint32 EndTileX = NewPlayerP.AbsTileX;
      uint32 EndTileY = NewPlayerP.AbsTileY;
      uint32 AbsTileZ = Entity->P.AbsTileZ;
      
      int32 DeltaX = SignOf(EndTileX - StartTileX);
      int32 DeltaY = SignOf(EndTileY - StartTileY);

      real32 tMin = 1.0f;
      uint32 AbsTileY = StartTileY;
      
      for(;;)
      {
	uint32 AbsTileX = StartTileX;
	for(;;)
	{
	  tile_map_position TestTileP =  CenteredTilePoint(AbsTileX, AbsTileY, AbsTileZ);
	  uint32 TileValue = GetTileValue(TileMap, TestTileP);
	  if(!IsTileValueEmpty(TileValue))
	  {
	    v2 MinCorner = -0.5f*v2{TileMap->TileSideInMeter, TileMap->TileSideInMeter};
	    v2 MaxCorner = 0.5f*v2{TileMap->TileSideInMeter, TileMap->TileSideInMeter};

	    tile_map_difference RelOldPlayerP = Subtract(TileMap, &OldPlayerP, &TestTileP);
	    v2 Rel = RelOldPlayerP.dXY;
	    TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y);
	    TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y);
	    TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X);
	    TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X);
	  }
	  if(AbsTileX == EndTileX)
	  {
	    break;
	  }
	  else
	  {
	    AbsTileX += DeltaX;
	  }
	}
	if(AbsTileY == EndTileY)
	{
	  break;
	}
	else
	{
	  AbsTileY += DeltaY;
	}
      }
      Entity->P = Offset(TileMap, OldPlayerP, tMin * PlayerDelta);	

#endif
      if(!AreOnSameTile(&OldPlayerP, &Entity->P))
      {
	uint32 NewTileValue = GetTileValue(TileMap, Entity->P);
	if(NewTileValue == 3)
	{
	  ++Entity->P.AbsTileZ;
	}
	else if(NewTileValue == 4)
	{
	  --Entity->P.AbsTileZ;
	}
      }
      if((Entity->dvP.X == 0.0f) && (Entity->dvP.Y == 0))
      {
	//Leave the last facing direction
      }
      else if(AbsoluteValue(Entity->dvP.X) > AbsoluteValue(Entity->dvP.Y))
      {
	if(Entity->dvP.X > 0)
	{
	  Entity->WizFacingDirection = 0;
	}
	else
	{
	  Entity->WizFacingDirection = 1;
	}
      }
      if(AbsoluteValue(Entity->dvP.X) < AbsoluteValue(Entity->dvP.Y))
      {
	if(Entity->dvP.Y > 0)
	{

	}
	else
	{

	}
      }
}


internal void DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 realX, real32 realY, int32 AlignX = 0, int32 AlignY = 0)
{
  realX -= (real32)AlignX;
  realY -= (real32)AlignY;
  
  int32 MinX = RoundReal32ToInt32(realX);
  int32 MaxX = RoundReal32ToInt32(MinX + (real32)Bitmap->Width);
  int32 MinY = RoundReal32ToInt32(realY);
  int32 MaxY = RoundReal32ToInt32(MinY + (real32)Bitmap->Height);

  int32 SourceOffsetX = 0;
  if(MinX < 0)
  {
    SourceOffsetX = -MinX;
    MinX = 0;
  }
  if(MaxX > Buffer->Width)
  {
    MaxX = Buffer->Width;
  }
  int32 SourceOffsetY = 0;
  if(MinY < 0)
  {
    SourceOffsetY = -MinY;
    MinY = 0; 
  }
  if(MaxY > Buffer->Height)
  {
    MaxY = Buffer->Height;
  }
 
  uint8* DestRow = ((uint8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch); 
  uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height - 1);
  SourceRow += -Bitmap->Width * SourceOffsetY + SourceOffsetX;
  
  for(int32 Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Dest = (uint32 *)DestRow;
    uint32 *Source = SourceRow;
    for(int32 X = MinX; X < MaxX; ++X)
    {
      //Linear Alpha Blending
      real32 A  = (real32)((*Source >> 24) & 0xFF) / 255.0f;
      real32 SR = (real32)((*Source >> 16) & 0xFF);
      real32 SG = (real32)((*Source >> 8) & 0xFF);
      real32 SB = (real32)((*Source >> 0) & 0xFF);

      real32 DR = (real32)((*Dest >> 16) & 0xFF);
      real32 DG = (real32)((*Dest >> 8) & 0xFF);
      real32 DB = (real32)((*Dest >> 0) & 0xFF);

      real32 R = (1.0f - A)*DR + A*SR;
      real32 G = (1.0f - A)*DG + A*SG;
      real32 B = (1.0f - A)*DB + A*SB;

      *Dest = (((uint32)(R + 0.5f) << 16) |
	       ((uint32)(G + 0.5f)) << 8 |
	       ((uint32)(B + 0.5f) << 0) );
      
      ++Dest;
      ++Source;
    }
    DestRow += Buffer->Pitch;
    SourceRow -= Bitmap->Width;
  }
}

inline entity *GetEntity(game_state *GameState, uint32 Index)
{
  entity *Entity = 0;
  if((Index > 0) && (Index < ArrayCount(GameState->Entities)))
  {
    Entity = &GameState->Entities[Index];
  }
  return Entity;
}

internal void InitializePlayers(game_state *GameState, uint32 EntityIndex)
{
  entity *Entity = GetEntity(GameState, EntityIndex);

  Entity->Exists = true;
  Entity->P.AbsTileX = 4;
  Entity->P.AbsTileY = 2;
  Entity->P.Offset_.X = 5.0f;
  Entity->P.Offset_.Y = 5.0f;
  Entity->Height = 1.4f;
  Entity->Width = 0.75f * Entity->Height;

  if(!GetEntity(GameState, GameState->CameraFollowEntityIndex))
  {
    GameState->CameraFollowEntityIndex = EntityIndex;
  }
}

internal uint32 AddEntity(game_state *GameState)
{
  uint32 EntityIndex = GameState->EntityCount++;
  Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
  entity *Entity = &GameState->Entities[EntityIndex];
  *Entity = {};
  return EntityIndex;
}


#if defined __cplusplus
extern "C"
#endif
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
  Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));

  game_state *GameState = (game_state *)Memory->PermanentStorage;
  
  if(!Memory->IsInitialized)
  { 
    /*GameState->ToneHz = 256;
    GameState->tSine = 0.0f;
    GameState->XOffset = 0;
    GameState->YOffset = 0;*/
    GameState->BackGround = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_img.bmp");
    GameState->Wizard.Wiz[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage1.bmp");
    GameState->Wizard.Wiz[1] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage2.bmp");
    

    GameState->CameraP.AbsTileX = 17/2;
    GameState->CameraP.AbsTileY = 9/2;

    InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *)Memory->PermanentStorage + sizeof(game_state));
    GameState->World = PushStruct(&GameState->WorldArena, world);

    world *World = GameState->World;
    World->TileMap = PushStruct(&GameState->WorldArena, tile_map);
    AddEntity(GameState);

    tile_map *TileMap = World->TileMap;
    
    TileMap->TileChunkCountX = 128;
    TileMap->TileChunkCountY = 128;
    TileMap->TileChunkCountZ = 2;
    TileMap->TileSideInMeter = 1.40f;
    TileMap->TileChunks = PushArray(&GameState->WorldArena,
				    TileMap->TileChunkCountX * TileMap->TileChunkCountY * TileMap->TileChunkCountZ,
				    tile_chunk);

    TileMap->ChunkShift = 4;
    TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
    TileMap->ChunkDim = (1 << TileMap->ChunkShift);
    

      
    uint32 TilesPerWidth = 17;
    uint32 TilesPerHeight = 9;
#if 0
    uint32 ScreenY = INT32_MAX / 2;
    uint32 ScreenX = INT32_MAX / 2;
#endif
    uint32 ScreenY = 0;
    uint32 ScreenX = 0;
    uint32 RandomNumberIndex = 0;
    bool32 TopDoor = false;
    bool32 RigthDoor = false;
    bool32 BottomDoor = false;
    bool32 LeftDoor = false;
    bool32 UpDoor = false;
    bool32 DownDoor = false;
    uint32 AbsTileZ = 0;

    for(uint32 ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex)
    {
      uint32 RandomChoice;
      if(UpDoor || DownDoor)
      {
	RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
      }
      else
      {
	RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
      }
      bool32 CreatedZDoor = false;
      if(RandomChoice == 2)
      {
	CreatedZDoor = true;
	if(AbsTileZ == 0)
	{
	  UpDoor = true;
	}
	else
	{
	  DownDoor = true;
	}
      }
      else if(RandomChoice == 1)
      {
	RigthDoor = true;
      }
      else
      {
	TopDoor = true;
      }
      
      for(uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
      {
	for(uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
	{
	  uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
	  uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;
	  uint32 TileValue = 1;
	  if(TileX == 0)
	  {
	    if(TileY == (TilesPerHeight / 2) && LeftDoor)
	    {
	      TileValue = 1;
	    }
	    else
	    {
	      TileValue = 2;
	    }
	  }
	  if(TileX == TilesPerWidth - 1)
	  {
	    if(TileY ==  (TilesPerHeight / 2) && RigthDoor)
	    {
	      TileValue = 1;
	    }
	    else
	    {
	      TileValue = 2;
	    }
	  }
	  if(TileY == 0)
	  {
	    if(TileX == (TilesPerWidth/2) && BottomDoor)
	    {
	      TileValue = 1;
	    }
	    else
	    {
	      TileValue = 2;
	    }
	  }
	  if(TileY == TilesPerHeight - 1)
	  {
	    if(TileX == (TilesPerWidth/2) && TopDoor)
	    {
	      TileValue = 1;
	    }
	    else
	    {
	      TileValue = 2;
	    }	  
	  }
	  if((TileX == 10) && (TileY == 7))
	  {
	    if(UpDoor)
	    {
	      TileValue = 3;
	    }
	    if(DownDoor)
	    {
	      TileValue = 4;
	    }
	  }	  
	  SetTileValue(&GameState->WorldArena, TileMap, AbsTileX, AbsTileY, AbsTileZ, TileValue);
	}
      }
      BottomDoor = TopDoor;
      LeftDoor = RigthDoor;

      if(CreatedZDoor)
      {
	UpDoor = !UpDoor;
	DownDoor = !DownDoor;	
      }
      else
      {
	DownDoor = false;
	UpDoor = false;
      }
      TopDoor = false;
      RigthDoor = false;
      if(RandomChoice == 2)
      {
	if(AbsTileZ == 0)
	{
	  AbsTileZ = 1;
	}
	else
	{
	  AbsTileZ = 0;
	}
      }
      else if(RandomChoice == 1)
      {
	ScreenX += 1;
      }
      else
      {
	ScreenY += 1;
      }
    }
         
    Memory->IsInitialized = true;
  }

  #define TILE_MAP_COUNT_X 256
  #define TILE_MAP_COUNT_Y 256

  world *World = GameState->World;
  tile_map *TileMap = World->TileMap;

  uint32 TileSideInPixel = 60;
  real32 MetersToPixels = (real32)TileSideInPixel / (real32)TileMap->TileSideInMeter;
  real32 LowerLeftX = -(real32)(TileSideInPixel / 2);
  real32 LowerLeftY = (real32)Buffer->Height;

  real32 ScreenCenterY = (real32)Buffer->Height * 0.5f;
  real32 ScreenCenterX = (real32)Buffer->Width * 0.5f;

  for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
  {
    game_controller_input *Controller = GetController(Input, ControllerIndex);
    entity *ControllingEntity = GetEntity(GameState, GameState->PlayerIndexForControllers[ControllerIndex]);
    if(ControllingEntity)
        {
      v2 ddPlayer = {};
      if(Controller -> IsAnalog)
      {
        ddPlayer = v2{Controller->AvarageStickX, Controller->AvarageStickY};
      }
      else
      {
        if(Controller->MoveLeft.EndedDown)
        {
      	ddPlayer.X = -1.0f;
        }
        if(Controller->MoveRight.EndedDown)
        {
      	ddPlayer.X = 1.0f;
        }
        if(Controller->MoveUp.EndedDown)
        {
          ddPlayer.Y = 1.0f;
        }
        if(Controller->MoveDown.EndedDown)
        {
      	ddPlayer.Y = -1.0f;
        }
      }
      MovePlayer(GameState, ControllingEntity, Input->dtForFrame, ddPlayer);
    }
    else
    {
      if(Controller->Start.EndedDown)
      {
	uint32 EntityIndex = AddEntity(GameState);;
	InitializePlayers(GameState, EntityIndex);	
	GameState->PlayerIndexForControllers[ControllerIndex] = EntityIndex;
      }
    }
  }

  entity *CameraFollowingEntity = GetEntity(GameState, GameState->CameraFollowEntityIndex);
  if(CameraFollowingEntity)
  {
    GameState->CameraP.AbsTileZ = CameraFollowingEntity->P.AbsTileZ;
    tile_map_difference Diff = Subtract(TileMap, &CameraFollowingEntity->P, &GameState->CameraP);
    if(Diff.dXY.X > (9.0f * TileMap->TileSideInMeter))
    {
      GameState->CameraP.AbsTileX += 17; 
    }
    if(Diff.dXY.X < -(9.0f * TileMap->TileSideInMeter))
    {
      GameState->CameraP.AbsTileX -= 17; 
    }
    if(Diff.dXY.Y > (5.0f * TileMap->TileSideInMeter))
    {
      GameState->CameraP.AbsTileY += 9; 
    }
    if(Diff.dXY.Y < -(5.0f * TileMap->TileSideInMeter))
    {
      GameState->CameraP.AbsTileY -= 9; 
    }
  }


  DrawBitmap(Buffer, &GameState->BackGround, 0, 0);

  
  for(int32 RelRow = -10; RelRow < 10; ++RelRow)
  {
    for(int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
    {
      uint32 Column = GameState->CameraP.AbsTileX + RelColumn;
      uint32 Row = GameState->CameraP.AbsTileY + RelRow;
      uint32 TileID = GetTileValue(TileMap, Column, Row, GameState->CameraP.AbsTileZ);
      real32 Grey = 0.5f;
      if(TileID > 1)
      {
        Grey = 0.5f;
        if(TileID == 2)
        {
	  Grey = 1.0f;
        }
	if(TileID > 2)
	{
	  Grey = 0.25f;
	}
        v2 Cen ={ (ScreenCenterX - MetersToPixels * GameState->CameraP.Offset_.X + ((real32)(TileSideInPixel) * RelColumn)),
		      (ScreenCenterY + MetersToPixels * GameState->CameraP.Offset_.Y - ((real32)(TileSideInPixel) * RelRow)) };
	v2 TileSide = { 0.5f * TileSideInPixel,  0.5f * TileSideInPixel };
        v2 Min = Cen - TileSide;
        v2 Max = Cen + TileSide;
        /*if(Row == Entity->P.AbsTileY && Column == Entity->P.AbsTileX)
        {
	  Grey = 0.0f;
        }
        */
        DrawRectangle(Buffer, Min, Max, Grey, Grey, Grey);
      }
    }
  }
  entity *Entity = GameState->Entities;
  for(uint32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex, ++Entity)
  {
    if(Entity->Exists)
    {
      tile_map_difference Diff = Subtract(TileMap, &Entity->P, &GameState->CameraP);
      real32 PlayerGroundX = ScreenCenterX + MetersToPixels*Diff.dXY.X;
      real32 PlayerGroundY = ScreenCenterY - MetersToPixels*Diff.dXY.Y;
      
      v2 PlayerLeftTop = {PlayerGroundX - (0.5f * Entity->Width * MetersToPixels),
      			 PlayerGroundY - Entity->Height * MetersToPixels};
      v2 PlayerWidthHeigth = {Entity->Width, Entity->Height};

      loaded_bitmap *Wizard = &GameState->Wizard.Wiz[Entity->WizFacingDirection];
      DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop + MetersToPixels * PlayerWidthHeigth, 1.0f, 0.0f, 0.0f);
      DrawBitmap(Buffer, Wizard, PlayerGroundX, PlayerGroundY, 50, 145);
    }
  }
}


#if defined __cplusplus
extern "C"
#endif
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state *GameState = (game_state*)Memory->PermanentStorage;
  GameOutputSound(GameState, SoundBuffer, 400);
}
