#include "handmade.h"
#include "handmade_world.cpp"
#include "handmade_sim_region.cpp"
#include "handmade_entity.cpp"

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

internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex)
{
  for(uint32 HashBucket = 0; HashBucket < ArrayCount(GameState->CollisionRuleHash); ++HashBucket)
  {
    for(pairwise_collision_rule **Rule = &GameState->CollisionRuleHash[HashBucket]; *Rule;)
    {
      if((*Rule)->StorageIndexA == StorageIndex || (*Rule)->StorageIndexB == StorageIndex)
      {
	pairwise_collision_rule *Removed = *Rule;
	*Rule = (*Rule)->NextInHash;

	Removed->NextInHash = GameState->FirstFreeCollisionRule;
	GameState->FirstFreeCollisionRule = Removed;	
      }
      else
      {
	Rule = &(*Rule)->NextInHash;
      }
    }
  }
}

internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 CanCollide)
{
  if(StorageIndexA > StorageIndexB)
  {
    uint32 Temp = StorageIndexA;
    StorageIndexA = StorageIndexB;
    StorageIndexB = Temp;
  }

  pairwise_collision_rule *Found = 0;
  uint32 HashBucket = StorageIndexA & (ArrayCount(GameState->CollisionRuleHash) -1);
  for(pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket]; Rule; Rule = Rule->NextInHash)
  {
    if((Rule->StorageIndexA == StorageIndexA) && (Rule->StorageIndexB == StorageIndexB))
      {
	Found = Rule;
	break;
      }
  }
  
  if(!Found)
  {
    Found = GameState->FirstFreeCollisionRule;
    if(Found)
    {
      GameState->FirstFreeCollisionRule = Found->NextInHash;
    }
    else
    {
      Found = PushStruct(&GameState->WorldArena, pairwise_collision_rule);
    }
    Found->NextInHash = GameState->CollisionRuleHash[HashBucket];
    GameState->CollisionRuleHash[HashBucket] = Found;
  }

  if(Found)
  {
    Found->StorageIndexA = StorageIndexA;
    Found->StorageIndexB = StorageIndexB;
    Found->CanCollide = CanCollide;
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

struct add_low_entity_result
{
  low_entity *Low;
  uint32 LowIndex;
};

internal add_low_entity_result AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{  
  Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
  uint32 EntityIndex = GameState->LowEntityCount++;

  low_entity *EntityLow = GameState->LowEntities + EntityIndex;
  *EntityLow = {};
  EntityLow->Sim.Type = Type;
  EntityLow->P = NullPosition();
  EntityLow->Sim.Collision = GameState->NullCollision;
  ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, P);

  add_low_entity_result Result;
  Result.Low = EntityLow;
  Result.LowIndex = EntityIndex;
  
  return Result;
}

internal add_low_entity_result AddGroundedEntity(game_state *GameState, entity_type Type, world_position P, sim_entity_collision_volume_group *Collision)
{
  add_low_entity_result Entity = AddLowEntity(GameState, Type, P);
  Entity.Low->Sim.Collision = Collision;

  return Entity;
}

internal void InitHitpoints(low_entity *EntityLow, uint32 HitpointCount)
{  
  Assert(HitpointCount <= ArrayCount(EntityLow->Sim.HitPoint));
  EntityLow->Sim.HitPointMax = HitpointCount;
  for(uint32 HitPointIndex = 0; HitPointIndex < HitpointCount; ++HitPointIndex)
  {
    hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
    HitPoint->Flags = 0;
    HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
  }
}

internal add_low_entity_result AddSword(game_state *GameState)
{
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());
  Entity.Low->Sim.Collision = GameState->SwordCollision;
  
  AddFlag(&Entity.Low->Sim, EntityFlag_Moveable);

  return Entity; 
}

/*internal add_low_entity_result AddStaff(game_state *GameState)
{
  add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Staff, NullPosition());

  Entity.Low->Sim.Dim.Y = 0.5f;
  Entity.Low->Sim.Dim.X = 1.0f;


  return Entity; 
}
*/
internal add_low_entity_result AddPlayers(game_state *GameState, uint32 Offset = 0)
{
  world_position P = GameState->CameraP;
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Hero, P, GameState->PlayerCollision);

  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
  AddFlag(&Entity.Low->Sim, EntityFlag_Moveable);

  add_low_entity_result Sword = AddSword(GameState);
  Entity.Low->Sim.Sword.Index = Sword.LowIndex;
  
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
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Wall, P, GameState->WallCollision);

  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);

  return Entity;
}

internal add_low_entity_result AddStair(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Stair, P, GameState->StairCollision);

  Entity.Low->Sim.WalkableDim = Entity.Low->Sim.Collision->TotalVolume.Dim.XY;
  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
  Entity.Low->Sim.WalkableHeight = GameState->World->TileDepthInMeters;

  return Entity;
}

internal add_low_entity_result AddMonster(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Monster, P, GameState->MonsterCollision);

  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
  InitHitpoints(Entity.Low, 3);

  return Entity; 
}


internal add_low_entity_result AddFamiliar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Familiar, P, GameState->FamiliarCollision);

  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
  AddFlag(&Entity.Low->Sim, EntityFlag_Moveable);

  return Entity; 
}

inline void PushPiece(entity_visible_piece_group *Group, loaded_bitmap *Bitmap, v2 Offset,
		       real32 OffsetZ, v2 Align, v2 Dim, v4 Color)
{
  Assert(Group->Count < ArrayCount(Group->Pieces));
  entity_visible_piece *Piece = Group->Pieces + Group->Count++;
  Piece->Bitmap = Bitmap;
  Piece->Offset = Group->GameState->MetersToPixels*v2{Offset.X, -Offset.Y} - Align;
  Piece->OffsetZ = OffsetZ;
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

inline void PushRect(entity_visible_piece_group *Group, v2 Offset, real32 OffsetZ, v2 Dim, v4 Color)
{
  PushPiece(Group, 0, Offset, OffsetZ, v2{0, 0}, Dim, Color);
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

inline void DrawHitpoints(sim_entity *Entity, entity_visible_piece_group *PieceGroup)
{
  if(Entity->HitPointMax >= 1)
  {
    v2 HealthDim = {0.3f, 0.3f};
    real32 SpacingX = 1.5f*HealthDim.X;
    real32 FirstX = 1.5f*(Entity->HitPointMax - 1);
    v2 HitP = {-0.5f*(Entity->HitPointMax - 1)*SpacingX, -0.3f};
    v2 dHitP = {SpacingX, 0.0f};
    for(uint32 HealthIndex = 0; HealthIndex < Entity->HitPointMax; ++HealthIndex)
    {
      hit_point *HitPoint = Entity->HitPoint + HealthIndex;
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

sim_entity_collision_volume_group *MakeSimpleGroundCollision(game_state *GameState, real32 DimX, real32 DimY, real32 DimZ)
{
  sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
  Group->VolumeCount = 1;
  Group->Volumes = PushArray(&GameState->WorldArena, Group->VolumeCount, sim_entity_collision_volume);
  Group->TotalVolume.OffsetP = V3(0, 0, 0.5f*DimZ);
  Group->TotalVolume.Dim = V3(DimX, DimY, DimZ);
  Group->Volumes[0] = Group->TotalVolume;

  return Group;
}

sim_entity_collision_volume_group *MakeNullCollision(game_state *GameState)
{
  sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
  Group->VolumeCount = 0;
  Group->Volumes = 0;
  Group->TotalVolume.OffsetP = V3(0, 0, 0);
  Group->TotalVolume.Dim = V3(0, 0, 0);

  return Group;
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
    InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *)Memory->PermanentStorage + sizeof(game_state));

    GameState->World = PushStruct(&GameState->WorldArena, world);

    world *World = GameState->World;
    AddLowEntity(GameState, EntityType_Null, NullPosition());
    
    InitializeWorld(World, 1.4f, 3.0f);
    GameState->BackGround = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_img.bmp");
    GameState->Wall = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/brick.bmp");
    GameState->Wizard.Wiz[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage1.bmp");
    GameState->Wizard.Wiz[1] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage2.bmp");
    GameState->Monster = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/monster.bmp");
    GameState->Sword = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/fireball.bmp");
    GameState->Staff = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/staff.bmp");
    GameState->Stair = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/staff.bmp");
    
    GameState->NullCollision = MakeNullCollision(GameState);
    GameState->SwordCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 0.1f);
    
    GameState->StairCollision = MakeSimpleGroundCollision(GameState, GameState->World->TileSideInMeter,
							  2.0f*GameState->World->TileSideInMeter,
							  1.1f*GameState->World->TileDepthInMeters);
    
    GameState->PlayerCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 1.2f);
    
    GameState->MonsterCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 0.5f);
    
    GameState->WallCollision = MakeSimpleGroundCollision(GameState, GameState->World->TileSideInMeter,
							 GameState->World->TileSideInMeter,
							 GameState->World->TileDepthInMeters);
    
    GameState->FamiliarCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 1.2f);
          
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

	  bool32 ShouldBeDoor = false;
	  if(TileX == 0)
	  {
	    if(TileY == (TilesPerHeight / 2) && LeftDoor)
	    {
	    }
	    else
	    {
	      ShouldBeDoor = true;
	    }
	  }
	  if(TileX == TilesPerWidth - 1)
	  {
	    if(TileY ==  (TilesPerHeight / 2) && RigthDoor)
	    {
	    }
	    else
	    {
	      ShouldBeDoor = true;
	    }
	  }
	  if(TileY == 0)
	  {
	    if(TileX == (TilesPerWidth/2) && BottomDoor)
	    {
	    }
	    else
	    {
	      ShouldBeDoor = true;
	    }
	  }
	  if(TileY == TilesPerHeight - 1)
	  {
	    if(TileX == (TilesPerWidth/2) && TopDoor)
	    {
	    }
	    else
	    {
	      ShouldBeDoor = true;
	    }	  
	  }
	  if(ShouldBeDoor)
	  {
	    if(AbsTileZ > 0)
	    {
	      int Here = 5;
	    }
	    AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
	  }
	  else if(CreatedZDoor)
	  {
	   if((TileX == 10) && (TileY == 5))
	   {
	     AddStair(GameState, AbsTileX, AbsTileY, DownDoor ? AbsTileZ - 1 : AbsTileZ);
	   }
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
    GameState->CameraP = NewCameraP;

    AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);
    AddFamiliar(GameState, CameraTileX - 2, CameraTileY + 2, CameraTileZ);

    //SetCamera(GameState, NewCameraP); 
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
    controlled_hero *ConHero = GameState->ControlledHeroes + ControllerIndex;;
    if(ConHero->EntityIndex == 0)
    {
      if(Controller->Start.EndedDown)
      {
	*ConHero = {};
	ConHero->EntityIndex = AddPlayers(GameState).LowIndex;
      }
    }
    else
    {
      ConHero->ddPlayer = {};
      ConHero->dvZ = 0;
      ConHero->dSword = {};

      if(Controller -> IsAnalog)
      {
        ConHero->ddPlayer = v2{Controller->AvarageStickX, Controller->AvarageStickY};
      }
      else
      {
        if(Controller->MoveLeft.EndedDown)
        {
	  ConHero->ddPlayer.X = -1.0f;
        }
        if(Controller->MoveRight.EndedDown)
        {
	  ConHero->ddPlayer.X = 1.0f;
        }
        if(Controller->MoveUp.EndedDown)
        {
          ConHero->ddPlayer.Y = 1.0f;
        }
        if(Controller->MoveDown.EndedDown)
        {
	  ConHero->ddPlayer.Y = -1.0f;
        }
      }
      if(Controller->Start.EndedDown)
      {
	//ConHero->dvZ = 3.0f;
      }
      ConHero->dSword = {};
      if(Controller->ActionUp.EndedDown)
      {
	ConHero->dSword = v2{0.0f, 1.0f};
      }
      if(Controller->ActionDown.EndedDown)
      {
	ConHero->dSword = v2{0.0f, -1.0f};
      }
      if(Controller->ActionLeft.EndedDown)
      {
	ConHero->dSword = v2{-1.0f, 0.0f};
      }
      if(Controller->ActionRight.EndedDown)
      {
	ConHero->dSword = v2{1.0f, 0.0f};
      }
      
    }
  }
  
  uint32 TileSpanX = 17*5;
  uint32 TileSpanY = 9*5;
  uint32 TileSpanZ = 1;
  
  rectangle3 CameraBounds = RectCentDim(V3(0 ,0, 0), World->TileSideInMeter*V3((real32)TileSpanX, (real32)TileSpanY, (real32)TileSpanZ));

  memory_arena SimArena;
  InitializeArena(&SimArena, Memory->TransientStorageSize, Memory->TransientStorage);
  real32 dtForFrame = Input->dtForFrame;
  sim_region *SimRegion = BeginSim(&SimArena, GameState, GameState->World, GameState->CameraP, CameraBounds, dtForFrame);
  
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
  sim_entity *Entity = SimRegion->Entities;
  for(uint32 EntityIndex = 0; EntityIndex < SimRegion->EntityCount; ++EntityIndex, ++Entity)
  {
    if(Entity->Updatable)
    {
      PieceGroup.Count = 0;
      
      real32 dt = Input->dtForFrame;

      move_spec MoveSpec = DefaultMoveSpec();
      v3 ddP = {};
      
      switch(Entity->Type)
      {
        case EntityType_Hero:
        {
      
      	for(uint32 ControllIndex = 0; ControllIndex < ArrayCount(GameState->ControlledHeroes); ++ControllIndex)
      	{
      	  controlled_hero *ConHero = GameState->ControlledHeroes + ControllIndex;
      	  if(ConHero->EntityIndex == Entity->StorageIndex)
      	  {
      	    if(ConHero->dvZ != 0.0f)
      	    {
      	      Entity->dvP.Z = ConHero->dvZ;
      	    }

      	    MoveSpec.UnitMaxAccelVector = true;
      	    MoveSpec.Speed = 50.0f;
      	    MoveSpec.Drag = 8.0f;
	    ddP = V3(ConHero->ddPlayer, 0);
	    
      	    if((ConHero->dSword.X != 0) || (ConHero->dSword.Y != 0))
      	    {
      	      sim_entity *Sword = Entity->Sword.Ptr;
      	      if(Sword && IsSet(Sword, EntityFlag_Nonspatial))
      	      {
      		Sword->DistanceLimit = 5.0f;
      		MakeEntitySpatial(Sword, Entity->P, 5.0f*V3(ConHero->dSword, 0));
		AddCollisionRule(GameState, Sword->StorageIndex, Entity->StorageIndex, false);

      	      }
      	      sim_entity *Staff = Entity->Staff.Ptr;
      	      if(Staff)
      	      {			
      		//Staff->P = Entity->P;
      		//Staff->DistanceLimit = 0.1f;
      		//Staff->dvP = 0.3f*ConHero->dSword;	 
      	      }
      	    }       
      	  }
      	}
      		
          //DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop + MetersToPixels * PlayerWidthHeigth, 1.0f, 0.0f, 0.0f);
	loaded_bitmap *Wizard = &GameState->Wizard.Wiz[Entity->WizFacingDirection];
	PushBitmap(&PieceGroup, Wizard, v2{0,0}, 0, v2{50, 145});
	DrawHitpoints(Entity, &PieceGroup);
        } break;
	
        case EntityType_Wall:
        {
          PushBitmap(&PieceGroup, &GameState->Wall, v2{0,0}, 0, v2{40, 80});
        } break;
	
        case EntityType_Monster:
        {
	  if(Entity->HitPointMax == 0)
	  {
	    MakeEntityNonSpatial(Entity);
	  }
	  else
	  {
	    PushBitmap(&PieceGroup, &GameState->Monster, v2{0,0}, 0, v2{40, 80});
	    DrawHitpoints(Entity, &PieceGroup);
	  }
        } break;
	
        case EntityType_Sword:
        {

	  MoveSpec.UnitMaxAccelVector = false;
	  MoveSpec.Speed = 0.0f;
	  MoveSpec.Drag = 0.0f;
  

	  if(Entity->DistanceLimit == 0)
	  {
	    ClearCollisionRulesFor(GameState, Entity->StorageIndex);
	    MakeEntityNonSpatial(Entity);
	  }
	  PushBitmap(&PieceGroup, &GameState->Sword, v2{0,0}, 0, v2{25, 25});
        } break;
	
        case EntityType_Familiar:
        {  
	  sim_entity *ClosestHero = 0;
	  real32 ClosestHeroDSq = Square(10.0f); //Maximum search radius
#if 0
	  sim_entity *TestEntity = SimRegion->Entities;
	  for(uint32 TestEntityIndex = 0; TestEntityIndex < SimRegion->EntityCount; ++TestEntityIndex, ++TestEntity)
	  {
	    if(TestEntity->Type == EntityType_Hero)
	    {
	      real32 TestDSq = LenghtSq(TestEntity->P - Entity->P);
	      if(ClosestHeroDSq > TestDSq)
	      {
		ClosestHero = TestEntity;
		ClosestHeroDSq = TestDSq;
	      }
	    }
	  }
#endif 
	  if(ClosestHero && (ClosestHeroDSq > Square(2.0f)))
	  {
	    real32 Acceleration = 0.5f;
	    real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
	    ddP = OneOverLength*(ClosestHero->P - Entity->P);
	  }

	  MoveSpec.UnitMaxAccelVector = true;
	  MoveSpec.Speed = 50.0f;
	  MoveSpec.Drag = 8.0f;

          loaded_bitmap *Wizard = &GameState->Wizard.Wiz[Entity->WizFacingDirection];
	  PushBitmap(&PieceGroup, Wizard, v2{0,0}, 0, v2{50, 145});
        } break;
	
        case EntityType_Staff:
        {
	  UpdateSword(SimRegion, Entity, dt);
	  PushBitmap(&PieceGroup, &GameState->Staff, v2{0.5f, 0}, 0, v2{25, 32});
        } break;

        case EntityType_Stair:
	{
	  PushRect(&PieceGroup, v2{0,0}, 0, Entity->WalkableDim, v4{1, 0.5f, 0, 1});
	  PushRect(&PieceGroup, v2{0,0}, Entity-> WalkableHeight, Entity->WalkableDim, v4{1, 1, 0, 1});
	} break;
	
        default:
        {
          InvalidCodePath;
        } break;
      }


      if(!IsSet(Entity, EntityFlag_Nonspatial) && IsSet(Entity, EntityFlag_Moveable))
      {
	MoveEntity(GameState, SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP);
      }


      for(uint32 PieceIndex = 0; PieceIndex < PieceGroup.Count; ++PieceIndex)
      {
	entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;
	v3 EntityBaseP = GetEntityGroundPoint(Entity);

	real32 ZFudge = (1.0f + 0.1f*(EntityBaseP.Z + Piece->OffsetZ));
      

	real32 EntityGroundX = ScreenCenterX + MetersToPixels*ZFudge*EntityBaseP.X;
	real32 EntityGroundY = ScreenCenterY - MetersToPixels*ZFudge*EntityBaseP.Y;
	real32 Z = -MetersToPixels*EntityBaseP.Z;
	
        v2 Center = {EntityGroundX + Piece->Offset.X,
		     EntityGroundY + Piece->Offset.Y + Z};
        if(Piece->Bitmap)
        {
	  DrawBitmap(Buffer, Piece->Bitmap, Center.X, Center.Y);
        }
        else
        {
	  v2 HalfDim = 0.5f*MetersToPixels*Piece->Dim;
	  DrawRectangle(Buffer, Center-HalfDim, Center+HalfDim, Piece->R, Piece->G, Piece->B);
        }
      }
    }
  }
  
  EndSim(SimRegion, GameState);

}


#if defined __cplusplus
extern "C"
#endif
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state *GameState = (game_state*)Memory->PermanentStorage;
  GameOutputSound(GameState, SoundBuffer, 400);
}
