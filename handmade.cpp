#include "handmade.h"
#include "handmade_world.cpp"

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

internal bool32 TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY, real32 *tMin, real32 MinY, real32 MaxY)
{
  bool32 Hit = false;
  real32 tEpsilon = 0.001f;
  if(PlayerDeltaX != 0.0f)
  {
    real32 tResult = (WallX - RelX) / PlayerDeltaX;
    real32 Y = RelY  + tResult * PlayerDeltaY;
    if((tResult >= 0.0f) && (*tMin > tResult))
    {
      if((Y >= MinY) && (Y <= MaxY))
      {
	*tMin = Maximum(0.0f, tResult - tEpsilon);
	Hit = true;
      }
    }
  }
  
  return Hit;
}

inline v2 GetCameraSpaceP(game_state *GameState, low_entity *EntityLow)
{
  world_difference Diff = Subtract(GameState->World, &EntityLow->P, &GameState->CameraP);
  v2 Result = Diff.dXY;
  return Result;
}

internal high_entity *MakeEntityHighFrequency(game_state *GameState, low_entity *EntityLow, uint32 LowIndex, v2 CameraSpaceP)
{
  high_entity *HighEntity = 0;
  Assert(EntityLow->HighEntityIndex == 0);
  if(EntityLow->HighEntityIndex == 0)
  {
    if(GameState->HighEntityCount < ArrayCount(GameState->HighEntities_))
    {
      uint32 HighIndex = GameState->HighEntityCount++;
      HighEntity = GameState->HighEntities_ + HighIndex;
      HighEntity->P = CameraSpaceP;
      HighEntity->dvP = v2{0, 0};
      HighEntity->ChunkZ = EntityLow->P.ChunkZ;
      HighEntity->LowEntityIndex = LowIndex;
      HighEntity->WizFacingDirection = 0;
      
      EntityLow->HighEntityIndex = HighIndex;
    }
    else
    {
      InvalidCodePath;
    }
  }
  return HighEntity;
}

internal high_entity *MakeEntityHighFrequency(game_state *GameState, uint32 LowIndex)
{      
  low_entity *EntityLow = GameState->LowEntities + LowIndex;
  high_entity *EntityHigh = 0;
  if(EntityLow->HighEntityIndex)
  {
    EntityHigh = GameState->HighEntities_ + EntityLow->HighEntityIndex;
  }
  else
  {
    v2 CameraSpaceP = GetCameraSpaceP(GameState, EntityLow);
    EntityHigh = MakeEntityHighFrequency(GameState, EntityLow, LowIndex, CameraSpaceP);
  }
  return EntityHigh;
}

internal void MakeEntityLowFrequency(game_state *GameState, uint32 LowIndex)
{
  low_entity *LowEntity = &GameState->LowEntities[LowIndex];
  uint32 HighIndex = LowEntity->HighEntityIndex;
  if(HighIndex)
  {
    uint32 LastHighIndex = GameState->HighEntityCount - 1;
    if(HighIndex != LastHighIndex)
    {
      high_entity *LastEntity = GameState->HighEntities_ + LastHighIndex;
      high_entity *DelEntity = GameState->HighEntities_ + HighIndex;

      *DelEntity = *LastEntity;
      GameState->LowEntities[LastEntity->LowEntityIndex].HighEntityIndex = HighIndex; 
    }
    --GameState->HighEntityCount;
    LowEntity->HighEntityIndex = 0;
  }
}

inline low_entity *GetLowEntity(game_state *GameState, uint32 LowIndex)
{
  low_entity *Result = 0;
  if((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
  {
    Result = GameState->LowEntities + LowIndex;
  }
  return Result;
}

inline entity ForceEntityToHigh(game_state *GameState, uint32 LowIndex)
{
  entity Result = {};
  if((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
  {
    Result.LowIndex = LowIndex;
    Result.Low = GameState->LowEntities + LowIndex;
    Result.High = MakeEntityHighFrequency(GameState, LowIndex);
  }
  return Result;
}

struct add_low_entity_result
{
  low_entity *Low;
  uint32 LowIndex;
};

internal add_low_entity_result AddLowEntity(game_state *GameState, entity_type Type, world_position *P)
{  
  Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
  uint32 EntityIndex = GameState->LowEntityCount++;

  low_entity *EntityLow = GameState->LowEntities + EntityIndex;
  *EntityLow = {};
  EntityLow->Type = Type;
  
  ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, 0, P);

  add_low_entity_result Result;
  Result.Low = EntityLow;
  Result.LowIndex = EntityIndex;
  
  return Result;
}

inline void OffsetAndCheckFrequencyByArea(game_state *GameState, v2 Offset, rectangle2 CameraBounds)
{
  for(uint32 EntityIndex = 1; EntityIndex < GameState->HighEntityCount; )
  {
    high_entity *High = GameState->HighEntities_ + EntityIndex;
    low_entity *Low = GameState->LowEntities + High->LowEntityIndex;
    
    High->P += Offset;
      
    if(IsValid(&Low->P) && IsInRectangle(CameraBounds, High->P))
    {
      ++EntityIndex;
    }
    else
    {
      MakeEntityLowFrequency(GameState, High->LowEntityIndex);
    }
  }

}

internal void InitHitpoints(low_entity *EntityLow, uint32 HitpointCount)
{  
  Assert(HitpointCount <= ArrayCount(EntityLow->HitPoint));
  EntityLow->HitPointMax = HitpointCount;
  for(uint32 HitPointIndex = 0; HitPointIndex < HitpointCount; ++HitPointIndex)
  {
    hit_point *HitPoint = EntityLow->HitPoint + HitPointIndex;
    HitPoint->Flags = 0;
    HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
  }
}

internal add_low_entity_result AddSword(game_state *GameState)
{
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, 0);

  Entity.Low->Height = 0.5f;
  Entity.Low->Width = 1.0f;
  Entity.Low->Collides = false;

  return Entity; 
}

internal add_low_entity_result AddStaff(game_state *GameState)
{
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Staff, 0);

  Entity.Low->Height = 0.5f;
  Entity.Low->Width = 1.0f;
  Entity.Low->Collides = false;

  return Entity; 
}

internal add_low_entity_result AddPlayers(game_state *GameState, uint32 Offset = 0)
{
  world_position P = GameState->CameraP;
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, &P);
  Entity.Low->Width =  GameState->World->TileSideInMeter * 0.85f;
  Entity.Low->Height = GameState->World->TileSideInMeter * 0.85f;
  Entity.Low->Collides = true;

  add_low_entity_result Sword = AddSword(GameState);
  Entity.Low->SwordLowIndex = Sword.LowIndex;

  add_low_entity_result Staff = AddStaff(GameState);
  Entity.Low->StaffLowIndex = Staff.LowIndex;
  
  InitHitpoints(Entity.Low, 3);
  if(GameState->CameraFollowEntityIndex == 0)
  {
    GameState->CameraFollowEntityIndex = Entity.LowIndex;
  }
  return Entity;
}


internal add_low_entity_result AddWall(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, &P);

  Entity.Low->Height = GameState->World->TileSideInMeter;
  Entity.Low->Width =  GameState->World->TileSideInMeter;
  Entity.Low->Collides = true;

  return Entity;
}

internal add_low_entity_result AddMonster(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
   world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monster, &P);

  Entity.Low->Height = 0.5f;
  Entity.Low->Width = 1.0f;
  Entity.Low->Collides = true;
  InitHitpoints(Entity.Low, 3);

  return Entity; 
}


internal add_low_entity_result AddFamiliar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
   world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Familiar, &P);

  Entity.Low->Height = 0.5f;
  Entity.Low->Width = 1.0f;
  Entity.Low->Collides = false;

  return Entity; 
}

inline void PushPiece(entity_visible_piece_group *Group, loaded_bitmap *Bitmap, v2 Offset,
		       real32 OffsetZ, v2 Align, v2 Dim, v4 Color)
{
  Assert(Group->Count < ArrayCount(Group->Pieces));
  entity_visible_piece *Piece = Group->Pieces + Group->Count++;
  Piece->Bitmap = Bitmap;
  Piece->Offset = Group->GameState->MetersToPixels*v2{Offset.X, -Offset.Y} - Align;
  Piece->OffsetZ = Group->GameState->MetersToPixels*OffsetZ;
  Piece->R = Color.R;
  Piece->G = Color.G;
  Piece->B = Color.B;
  Piece->A = Color.A;
  Piece->Dim = Dim;
}

inline void PushBitmap(entity_visible_piece_group *Group, loaded_bitmap *Bitmap, v2 Offset,
			 real32 OffsetZ, v2 Align, real32 Alpha = 1.0f)
{
  PushPiece(Group, Bitmap, Offset, OffsetZ, Align, v2{0, 0},v4{1.0f, 1.0f, 1.0f, Alpha});
}

inline void PushRect(entity_visible_piece_group *Group, v2 Offset,			 real32 OffsetZ, v2 Dim, v4 Color)
{
  PushPiece(Group, 0, Offset, OffsetZ, v2{0, 0}, Dim, Color);
}

struct MoveSpec
{
  bool32 UnitMaxAccelVector;
  real32 Speed;
  real32 Drag;
};

MoveSpec DefaultMoveSpec()
{
  MoveSpec Result;
  Result.UnitMaxAccelVector = false;
  Result.Speed = 1.0f;
  Result.Drag = 0.0f;
  return Result;
}

void MoveEntity(game_state *GameState, entity Entity, real32 dt, MoveSpec *MoveSpec, v2 ddPlayer)
{
      real32 Velocity = 60.0f;
      world *World = GameState->World;
      if(MoveSpec->UnitMaxAccelVector)
      {
        real32 ddPlayerLenght = LenghtSq(ddPlayer);
        if(ddPlayerLenght > 1.0f)
        {
	  ddPlayer *= 1.0f/SquareRoot(ddPlayerLenght);
        }
      }
      v2 OldPlayerP = Entity.High->P;
      
      ddPlayer *= MoveSpec->Speed;
      ddPlayer += -MoveSpec->Drag*Entity.High->dvP;
      
      v2 PlayerDelta = 0.5f*ddPlayer*Square(dt) + Entity.High->dvP*dt;
      Entity.High->dvP = ddPlayer*dt + Entity.High->dvP;
      
      v2 NewPlayerP = OldPlayerP + PlayerDelta;
     
      for(uint32 Iteration = 0; Iteration < 4; ++Iteration)
      {
	real32 tMin = 1.0f;
	v2 WallNormal = v2{};
	uint32 HitEntityIndex = 0;

	v2 DesiredPosition = Entity.High->P + PlayerDelta;
	if(Entity.Low->Collides)
	{
          for(uint32 TestHighEntityIndex = 1; TestHighEntityIndex < GameState->HighEntityCount; ++TestHighEntityIndex)
          {
	    if(TestHighEntityIndex != Entity.Low->HighEntityIndex)
	    {
	      entity TestEntity;
	      TestEntity.High = GameState->HighEntities_ + TestHighEntityIndex;
	      TestEntity.LowIndex = TestEntity.High->LowEntityIndex;
	      TestEntity.Low = GameState->LowEntities + TestEntity.LowIndex;
	      if(TestEntity.Low->Collides)
	      {
	        real32 DiameterW = TestEntity.Low->Width + Entity.Low->Width;
	        real32 DiameterH = TestEntity.Low->Height + Entity.Low->Height;
	        
	        v2 MinCorner = {DiameterW * -0.5f, DiameterH *-0.5f};
	        v2 MaxCorner = {DiameterW * 0.5f, DiameterH * 0.5f};
	          
	        v2 Rel = Entity.High->P - TestEntity.High->P;
	        if(TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
	        {
	          WallNormal = v2{-1, 0};
	  	HitEntityIndex = TestHighEntityIndex;
	        }
	        if(TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
	        {
	          WallNormal = v2{1, 0};
	  	HitEntityIndex = TestHighEntityIndex;
	        }
	        if(TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
                {
	          WallNormal = v2{0, -1};
	  	HitEntityIndex = TestHighEntityIndex;
	        }
	        if(TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
	        {
	          WallNormal = v2{0, 1};
	  	HitEntityIndex = TestHighEntityIndex;		
	        }
	      }
	    }
          }
	}
	Entity.High->P += tMin * PlayerDelta;	

	if(HitEntityIndex)
	{
	  Entity.High->dvP = Entity.High->dvP - 1*DotProduct(Entity.High->dvP, WallNormal)*WallNormal;
	  PlayerDelta = DesiredPosition - Entity.High->P;
	  PlayerDelta = PlayerDelta - 1*DotProduct(PlayerDelta, WallNormal)*WallNormal;
	  
	  high_entity *HitHigh = GameState->HighEntities_ + HitEntityIndex;
	  low_entity *HitLow = GameState->LowEntities + HitHigh->LowEntityIndex;
	  //TODO: this is stairs
	  //Entity.High->AbsTileZ += HitLow->dAbsTileZ;
	}
	else
	{
	  break;
	}
      }
      if((Entity.High->dvP.X == 0.0f) && (Entity.High->dvP.Y == 0))
      {
	//Leave the last facing direction
      }
      else if(AbsoluteValue(Entity.High->dvP.X) > AbsoluteValue(Entity.High->dvP.Y))
      {
	if(Entity.High->dvP.X > 0)
	{
	  Entity.High->WizFacingDirection = 0;
	}
	else
	{
	  Entity.High->WizFacingDirection = 1;
	}
      }
      if(AbsoluteValue(Entity.High->dvP.X) < AbsoluteValue(Entity.High->dvP.Y))
      {
	if(Entity.High->dvP.Y > 0)
	{

	}
	else
	{

	}
      }
      world_position NewP = MapIntoChunkSpace(GameState->World, GameState->CameraP, Entity.High->P);

      ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex, Entity.Low, &Entity.Low->P, &NewP);
}


internal void DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 realX, real32 realY)
{
  
  int32 MinX = RoundReal32ToInt32(realX);
  int32 MaxX = MinX + Bitmap->Width;
  int32 MinY = RoundReal32ToInt32(realY);
  int32 MaxY = MinY + Bitmap->Height;

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

internal void SetCamera(game_state *GameState, world_position NewCameraP)
{
  world *World = GameState->World;
  world_difference dCameraP = Subtract(World, &NewCameraP, &GameState->CameraP);

  GameState->CameraP = NewCameraP;
  v2 EntityOffsetForFrame = -dCameraP.dXY;
  uint32 TileSpanX = 17*3;
  uint32 TileSpanY = 9*3;
  rectangle2 CameraBounds = RectCentDim(v2{0 ,0}, World->TileSideInMeter*v2{(real32)TileSpanX, (real32)TileSpanY});

  OffsetAndCheckFrequencyByArea(GameState, EntityOffsetForFrame, CameraBounds);
  //TODO: Change in terms of tile chunks
  world_position MinChunkP = MapIntoChunkSpace(World, NewCameraP, GetMinCorner(CameraBounds));
  world_position MaxChunkP = MapIntoChunkSpace(World, NewCameraP, GetMaxCorner(CameraBounds));

  for(int32 ChunkY = MinChunkP.ChunkY; ChunkY < MaxChunkP.ChunkY; ++ChunkY)
  {
    for(int32 ChunkX = MinChunkP.ChunkX; ChunkX < MaxChunkP.ChunkX; ++ChunkX)
    {
      world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, NewCameraP.ChunkZ);
      if(Chunk)
      {
	for(world_entity_block *Block = &Chunk->FirstBlock; Block; Block = Block->NextBlock)
        {
	  for(uint32 EntityIndexIndex = 0; EntityIndexIndex < Block->LowEntityCount; ++EntityIndexIndex)
          {
	    uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
	    low_entity *Low = GameState->LowEntities + LowEntityIndex;
            if(Low->HighEntityIndex == 0)
            {
	      v2 CameraSpaceP = GetCameraSpaceP(GameState, Low);
	      if(IsInRectangle(CameraBounds, CameraSpaceP))
	      {
		MakeEntityHighFrequency(GameState, Low, LowEntityIndex, CameraSpaceP);
	      }
            }
          }
  	}
      }
    }
  }
}

inline entity EntityFromHighIndex(game_state *GameState, uint32 HighEntityIndex)
{
  entity Result = {};

  if(HighEntityIndex)
  {
    Assert(HighEntityIndex < ArrayCount(GameState->HighEntities_));
    Result.High = GameState->HighEntities_ + HighEntityIndex;
    Result.LowIndex = Result.High->LowEntityIndex;
    Result.Low = GameState->LowEntities + Result.LowIndex;
  }

  return Result;
}

inline void UpdateFamiliar(game_state *GameState, entity Entity, real32 dt)
{
  entity ClosestHero = {};
  real32 ClosestHeroDSq = Square(10.0f); //Maximum search radius
  for(uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
  {
    entity TestEntity = EntityFromHighIndex(GameState, HighEntityIndex);

    if(TestEntity.Low->Type == EntityType_Hero)
    {
      real32 TestDSq = LenghtSq(TestEntity.High->P - Entity.High->P);
      if(ClosestHeroDSq > TestDSq)
      {
	ClosestHero = TestEntity;
	ClosestHeroDSq = TestDSq;
      }
    }
  }
  v2 ddP = {};
  if(ClosestHero.High && (ClosestHeroDSq > Square(2.0f)))
  {
    real32 Acceleration = 0.5f;
    real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
    ddP = OneOverLength*(ClosestHero.High->P - Entity.High->P);
    MoveSpec MoveSpec = DefaultMoveSpec();
    MoveSpec.UnitMaxAccelVector = true;
    MoveSpec.Speed = 50.0f;
    MoveSpec.Drag = 8.0f;
    MoveEntity(GameState, Entity, dt, &MoveSpec, ddP);
  }
}

inline void UpdateMonster(game_state *GameState, entity Entity, real32 dt)
{
}

inline void UpdateSword(game_state *GameState, entity Entity, real32 dt)
{
  MoveSpec MoveSpec = DefaultMoveSpec();
  MoveSpec.UnitMaxAccelVector = false;
  MoveSpec.Speed = 0.0f;
  MoveSpec.Drag = 0.0f;

  v2 OldP = Entity.High->P;
  MoveEntity(GameState, Entity, dt, &MoveSpec, v2{0 ,0});
  real32 DistanceTraveled = Length(Entity.High->P - OldP);

  Entity.Low->DistanceRemaining -= DistanceTraveled;
  if(Entity.Low->DistanceRemaining < 0)
  {
    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex, Entity.Low, &Entity.Low->P, 0);
  }
}

inline void UpdateStaff(game_state *GameState, entity Entity, real32 dt)
{
  MoveSpec MoveSpec = DefaultMoveSpec();
  MoveSpec.UnitMaxAccelVector = false;
  MoveSpec.Speed = 0.0f;
  MoveSpec.Drag = 0.0f;

  v2 OldP = Entity.High->P;
  MoveEntity(GameState, Entity, dt, &MoveSpec, v2{0 ,0});
  real32 DistanceTraveled = Length(Entity.High->P - OldP);

  Entity.Low->DistanceRemaining -= DistanceTraveled;
  if(Entity.Low->DistanceRemaining < 0)
  {
    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex, Entity.Low, &Entity.Low->P, 0);
  }
}


inline void DrawHitpoints(low_entity *LowEntity, entity_visible_piece_group *PieceGroup)
{
  if(LowEntity->HitPointMax >= 1)
  {
    v2 HealthDim = {0.3f, 0.3f};
    real32 SpacingX = 1.5f*HealthDim.X;
    real32 FirstX = 1.5f*(LowEntity->HitPointMax - 1);
    v2 HitP = {-0.5f*(LowEntity->HitPointMax - 1)*SpacingX, -0.3f};
    v2 dHitP = {SpacingX, 0.0f};
    for(uint32 HealthIndex = 0; HealthIndex < LowEntity->HitPointMax; ++HealthIndex)
    {
      hit_point *HitPoint = LowEntity->HitPoint + HealthIndex;
      v4 Color = v4{1.0f, 0.0f, 0.0f, 1.0f};
      
      if(HitPoint->FilledAmount == 0)
      {
	Color = v4{0.2f, 0.2f, 0.2f, 1.0f};
      }
      PushRect(PieceGroup, HitP, 0, HealthDim, Color);
      HitP += dHitP;
    }
  }
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
    GameState->Wall = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/brick.bmp");
    GameState->Wizard.Wiz[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage1.bmp");
    GameState->Wizard.Wiz[1] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage2.bmp");
    GameState->Monster = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/monster.bmp");
    GameState->Sword = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/fireball.bmp");
    GameState->Staff = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/staff.bmp");

    GameState->CameraP.ChunkX = 17/2;
    GameState->CameraP.ChunkY = 9/2;


    InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *)Memory->PermanentStorage + sizeof(game_state));
    GameState->World = PushStruct(&GameState->WorldArena, world);

    world *World = GameState->World;
    AddLowEntity(GameState, EntityType_Null, 0);
    GameState->HighEntityCount = 1;

    InitializeWorld(World, 1.4f);
          
    uint32 TilesPerWidth = 17;
    uint32 TilesPerHeight = 9;
    uint32 ScreenBaseX = 0;
    uint32 ScreenBaseY = 0;
    uint32 ScreenBaseZ = 0;
    uint32 ScreenX = ScreenBaseX;
    uint32 ScreenY = ScreenBaseY;
    uint32 AbsTileZ = ScreenBaseZ;
    uint32 RandomNumberIndex = 0;
    bool32 TopDoor = false;
    bool32 RigthDoor = false;
    bool32 BottomDoor = false;
    bool32 LeftDoor = false;
    bool32 UpDoor = false;
    bool32 DownDoor = false;

    for(uint32 ScreenIndex = 0; ScreenIndex < 2; ++ScreenIndex)
    {
      uint32 RandomChoice;
      //      if(UpDoor || DownDoor)
      {
	RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
      }
#if 0
      else
      {
	RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
      }
#endif
      bool32 CreatedZDoor = false;
      if(RandomChoice == 2)
      {
	CreatedZDoor = true;
	if(AbsTileZ == ScreenBaseZ)
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
	  if(TileValue == 2)
	  {
	    AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
	  }
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
	if(AbsTileZ == ScreenBaseZ)
	{
	  AbsTileZ = ScreenBaseZ + 1;
	}
	else
	{
	  AbsTileZ = ScreenBaseZ;
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
    world_position NewCameraP = {};
    uint32 CameraTileX = ScreenBaseX*TilesPerWidth + 17/2;
    uint32 CameraTileY = ScreenBaseY*TilesPerHeight + 9/2;
    uint32 CameraTileZ = ScreenBaseZ;
    NewCameraP = ChunkPositionFromTilePosition(GameState->World, CameraTileX, CameraTileY, CameraTileZ);

    AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);
    AddFamiliar(GameState, CameraTileX - 2, CameraTileY + 2, CameraTileZ);

    SetCamera(GameState, NewCameraP); 
    Memory->IsInitialized = true;
  }

  #define WORLD_COUNT_X 256
  #define WORLD_COUNT_Y 256

  world *World = GameState->World;

  uint32 TileSideInPixel = 60;
  GameState->MetersToPixels = (real32)TileSideInPixel / (real32)World->TileSideInMeter;
  real32 MetersToPixels = GameState->MetersToPixels;
  real32 LowerLeftX = -(real32)(TileSideInPixel / 2);
  real32 LowerLeftY = (real32)Buffer->Height;

  real32 ScreenCenterY = (real32)Buffer->Height * 0.5f;
  real32 ScreenCenterX = (real32)Buffer->Width * 0.5f;

  for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
  {
    game_controller_input *Controller = GetController(Input, ControllerIndex);
    uint32 LowIndex = GameState->PlayerIndexForControllers[ControllerIndex];
    if(LowIndex == 0)
    {
      if(Controller->Start.EndedDown)
      {
	uint32 EntityIndex = AddPlayers(GameState).LowIndex;

	GameState->PlayerIndexForControllers[ControllerIndex] = EntityIndex;
      }
    }
    else
    {
      v2 ddPlayer = {};
      entity ControllingEntity = ForceEntityToHigh(GameState, LowIndex);
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
      if(Controller->Start.EndedDown)
      {
	ControllingEntity.High->dvZ = 3.0f;
      }
      v2 dSword = {};
      if(Controller->ActionUp.EndedDown)
      {
	dSword = v2{0.0f, 1.0f};
      }
      if(Controller->ActionDown.EndedDown)
      {
	dSword = v2{0.0f, -1.0f};
      }
      if(Controller->ActionLeft.EndedDown)
      {
	dSword = v2{-1.0f, 0.0f};
      }
      if(Controller->ActionRight.EndedDown)
      {
	dSword = v2{1.0f, 0.0f};
      }
      
      MoveSpec MoveSpec = DefaultMoveSpec();
      MoveSpec.UnitMaxAccelVector = true;
      MoveSpec.Speed = 50.0f;
      MoveSpec.Drag = 8.0f;
      MoveEntity(GameState, ControllingEntity, Input->dtForFrame, &MoveSpec, ddPlayer);
      if((dSword.X != 0) || (dSword.Y != 0))
      {
	low_entity *LowSword = GetLowEntity(GameState, ControllingEntity.Low->SwordLowIndex);
	low_entity *LowStaff = GetLowEntity(GameState, ControllingEntity.Low->StaffLowIndex);
	if((LowSword && !IsValid(&LowSword->P)) && (LowStaff && !IsValid(&LowStaff->P)))
	{
	  world_position SwordP = ControllingEntity.Low->P;
	  ChangeEntityLocation(&GameState->WorldArena, GameState->World, ControllingEntity.Low->SwordLowIndex, LowSword, 0, &SwordP);

	  world_position StaffP = ControllingEntity.Low->P;
	  ChangeEntityLocation(&GameState->WorldArena, GameState->World, ControllingEntity.Low->StaffLowIndex, LowStaff, 0, &StaffP);

	  entity Sword = ForceEntityToHigh(GameState, ControllingEntity.Low->SwordLowIndex);
	  Sword.Low->DistanceRemaining = 5.0f;
	  Sword.High->dvP = 5.0f*dSword;
	  
	  entity Staff = ForceEntityToHigh(GameState, ControllingEntity.Low->StaffLowIndex);
	  Staff.Low->DistanceRemaining = 0.1f;
	  Staff.High->dvP = 0.3f*dSword;	  
	}
      }
    }
  }

  entity CameraFollowingEntity = ForceEntityToHigh(GameState, GameState->CameraFollowEntityIndex);
  v2 EntityOffsetForFrame = {};
  if(CameraFollowingEntity.High)
  {
    world_position NewCameraP = GameState->CameraP;
    NewCameraP.ChunkZ = CameraFollowingEntity.Low->P.ChunkZ;
#if 0
    if(CameraFollowingEntity.High->P.X > (9.0f * World->TileSideInMeter))
    {
      NewCameraP.ChunkX += 17; 
    }
    if(CameraFollowingEntity.High->P.X < -(9.0f * World->TileSideInMeter))
    {
      NewCameraP.ChunkX -= 17; 
    }
    if(CameraFollowingEntity.High->P.Y > (5.0f * World->TileSideInMeter))
    {
      NewCameraP.ChunkY += 9; 
    }
    if(CameraFollowingEntity.High->P.Y < -(5.0f * World->TileSideInMeter))
    {
      NewCameraP.ChunkY -= 9; 
    }
#else
    NewCameraP = CameraFollowingEntity.Low->P;
#endif

    SetCamera(GameState, NewCameraP);
  }

  
  DrawBitmap(Buffer, &GameState->BackGround, 0, 0);

#if 0  
  for(int32 RelRow = -10; RelRow < 10; ++RelRow)
  {
    for(int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
    {
      uint32 Column = GameState->CameraP.AbsTileX + RelColumn;
      uint32 Row = GameState->CameraP.AbsTileY + RelRow;
      uint32 TileID = GetTileValue(World, Column, Row, GameState->CameraP.AbsTileZ);
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
#endif
  entity_visible_piece_group PieceGroup;
  PieceGroup.GameState = GameState;
  for(uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
  {
    PieceGroup.Count = 0;

    high_entity *HighEntity = GameState->HighEntities_ + HighEntityIndex;
    low_entity *LowEntity = GameState->LowEntities + HighEntity->LowEntityIndex;
    entity Entity;
    Entity.LowIndex = HighEntity->LowEntityIndex;
    Entity.Low = LowEntity;
    Entity.High = HighEntity;
    //HighEntity->P += EntityOffsetForFrame;
    
    real32 dt = Input->dtForFrame;
    real32 ddZ = -9.8f;
    real32 PlayerDeltaZ = 0.5f*ddZ*Square(dt) + HighEntity->dvZ*dt;
    HighEntity->Z += PlayerDeltaZ;
    HighEntity->dvZ = ddZ*dt + HighEntity->dvZ;
    if(HighEntity->Z < 0)
    {
      HighEntity->Z = 0;
    }
      
    real32 PlayerGroundX = ScreenCenterX + MetersToPixels*HighEntity->P.X;
    real32 PlayerGroundY = ScreenCenterY - MetersToPixels*HighEntity->P.Y;
    real32 Z = -HighEntity->Z * MetersToPixels;
    
    v2 PlayerLeftTop = {PlayerGroundX - 0.5f * LowEntity->Width * MetersToPixels,
      			  PlayerGroundY - 0.5f * LowEntity->Height * MetersToPixels};
    v2 PlayerWidthHeigth = {LowEntity->Width, LowEntity->Height};

    switch(LowEntity->Type)
    {
      case EntityType_Hero:
      {
        //DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop + MetersToPixels * PlayerWidthHeigth, 1.0f, 0.0f, 0.0f);
        loaded_bitmap *Wizard = &GameState->Wizard.Wiz[HighEntity->WizFacingDirection];
        PushBitmap(&PieceGroup, Wizard, v2{0,0}, 0, v2{50, 145});
	DrawHitpoints(LowEntity, &PieceGroup);
      } break;
      case EntityType_Wall:
      {
        PushBitmap(&PieceGroup, &GameState->Wall, v2{0,0}, 0, v2{40, 80});
        //DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop + MetersToPixels * PlayerWidthHeigth, 1.0f, 1.0f, 1.0f);
      } break;
      case EntityType_Monster:
      {
	UpdateMonster(GameState, Entity, dt);
        PushBitmap(&PieceGroup, &GameState->Monster, v2{0,0}, 0, v2{40, 80});
	DrawHitpoints(LowEntity, &PieceGroup);
      } break;
      case EntityType_Sword:
      {
	UpdateSword(GameState, Entity, dt);
	PushBitmap(&PieceGroup, &GameState->Sword, v2{0,0}, 0, v2{25, 25});
      } break;
      case EntityType_Familiar:
      {
	UpdateFamiliar(GameState, Entity, dt);
        loaded_bitmap *Wizard = &GameState->Wizard.Wiz[HighEntity->WizFacingDirection];
	PushBitmap(&PieceGroup, Wizard, v2{0,0}, 0, v2{50, 145});
      } break;
      case EntityType_Staff:
      {
	UpdateSword(GameState, Entity, dt);
	PushBitmap(&PieceGroup, &GameState->Staff, v2{0.5f, 0}, 0, v2{25, 32});
      } break;
      default:
      {
        InvalidCodePath;
      } break;
    }
    for(uint32 PieceIndex = 0; PieceIndex < PieceGroup.Count; ++PieceIndex)
    {
      entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;
      v2 Center = {PlayerGroundX + Piece->Offset.X, PlayerGroundY + Piece->Offset.Y + Piece->OffsetZ + Z};
      if(Piece->Bitmap)
      {
	DrawBitmap(Buffer, Piece->Bitmap, PlayerGroundX + Piece->Offset.X,
		   PlayerGroundY + Piece->Offset.Y + Piece->OffsetZ + Z);
      }
      else
      {
	v2 HalfDim = 0.5f*MetersToPixels*Piece->Dim;
	DrawRectangle(Buffer, Center-HalfDim, Center+HalfDim, Piece->R, Piece->G, Piece->B);
      }
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
