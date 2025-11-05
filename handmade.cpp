#include "handmade.h"
#include "handmade_render_group.h"
#include "handmade_render_group.cpp"
#include "handmade_world.cpp"
#include "handmade_sim_region.cpp"
#include "handmade_entity.cpp"
#include "handmade_random.h"

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

internal loaded_bitmap DEBUGLoadBMP(thread_context *Theard, debug_platform_read_entire_file *ReadEntireFile, char *FileName, int32 AlignX = 0, int32 TopDownAlignY = 0)
{

  loaded_bitmap Result = {};
  debug_read_file_result ReadResult = ReadEntireFile(Theard, FileName);
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
  Result.AlignX = AlignX;
  Result.AlignY = (BitMap->Height - 1) - TopDownAlignY;
  int32 BytesPerPixel = BITMAP_BYTES_PER_PIXEL;
  Result.Pitch = BitMap->Width*BytesPerPixel;
  
#if 0
  Result.Memory = (uint8 *)Result.Memory + Result.Pitch*(Result.Height - 1);
  Result.Pitch = -BitMap->Width*BytesPerPixel;
#endif
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

  Entity.Low->Sim.Dim.y = 0.5f;
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

inline world_position ChunkPositionFromTilePosition(world* World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ, v3 AdditionalOffset = v3{ 0.0f , 0.0f, 0.0f })
{
  world_position BasePos = {};

  real32 TileSideInMeter = 1.4f;
  real32 TileDepthInMeters = 3.0f;
  v3 TileDim = V3(TileSideInMeter, TileSideInMeter, TileDepthInMeters);
  v3 Offset = Hadamard(TileDim, V3((real32)AbsTileX, (real32)AbsTileY, (real32)AbsTileZ));

  world_position Result = MapIntoChunkSpace(World, BasePos, AdditionalOffset + Offset);
  Assert(IsCanonical(World, Result.Offset_));
  return Result;
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

  Entity.Low->Sim.WalkableDim = Entity.Low->Sim.Collision->TotalVolume.Dim.xy;
  AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
  Entity.Low->Sim.WalkableHeight = GameState->TypicalFloorHeight;

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

internal add_low_entity_result AddStandardRoom(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_position P =  ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
  add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Space, P, GameState->StandardRoomCollision);

  AddFlag(&Entity.Low->Sim, EntityFlag_Traversable);

  return Entity; 
}

inline void DrawHitpoints(sim_entity *Entity, render_group *RenderGroup)
{
  if(Entity->HitPointMax >= 1)
  {
    v2 HealthDim = {0.3f, 0.3f};
    real32 SpacingX = 1.5f*HealthDim.x;
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
      PushRect(RenderGroup, ToV3(HitP, 0), HealthDim, Color);
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

internal void FillGroundChunk(transient_state *TranState, game_state *GameState, ground_buffer *GroundBuffer, world_position *ChunkP)
{
  temporary_memory GroundMemory = BeginTemporaryMemory(&TranState->TranArena);
  render_group *RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), 1.0f);
  loaded_bitmap *Buffer = &GroundBuffer->Bitmap;

  GroundBuffer->P = *ChunkP;
  
  real32 Width = (real32)Buffer->Width;
  real32 Height = (real32)Buffer->Height;

  
  for(int32 ChunkOffsetY = -1; ChunkOffsetY <= 1; ++ChunkOffsetY)
  {
    for(int32 ChunkOffsetX = -1; ChunkOffsetX <= 1; ++ChunkOffsetX)
    {
      int32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
      int32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
      int32 ChunkZ = ChunkP->ChunkZ;
      
      random_series Series = Seed(124*ChunkX + 542*ChunkY + 322*ChunkZ);
      v2 Center = v2{ChunkOffsetX*Width, ChunkOffsetY*Height};
      for(uint32 Grass = 0; Grass < 100; ++Grass)
      {
	loaded_bitmap *Stamp;
	Stamp = GameState->Stones /*+RandomChoice(&Series, ArrayCount(GameState->Stones))*/;

	v2 Offset = {RandomUnilateral(&Series)*Width, RandomUnilateral(&Series)*Height};

	v2 BitmapCenter = 0.5f*V2i(Stamp->Width, Stamp->Height);

	real32 Radius = 5.0f;
	v2 P = Center + Offset - BitmapCenter;
	PushBitmap(RenderGroup, Stamp, ToV3(P, 0.0f));
      }
    }
  }

  for(int32 ChunkOffsetY = -1; ChunkOffsetY <= 1; ++ChunkOffsetY)
  {
    for(int32 ChunkOffsetX = -1; ChunkOffsetX <= 1; ++ChunkOffsetX)
    {
      int32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
      int32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
      int32 ChunkZ = ChunkP->ChunkZ;
      
      random_series Series = Seed(124*ChunkX + 542*ChunkY + 322*ChunkZ);
      v2 Center = v2{ChunkOffsetX*Width, ChunkOffsetY*Height};
      for(uint32 Grass = 0; Grass < 25; ++Grass)
      {
	loaded_bitmap *Stamp;
	Stamp = GameState->Grass /*+ RandomChoice(&Series, ArrayCount(GameState->Grass))*/;
	v2 Offset = {RandomUnilateral(&Series)*Width, RandomUnilateral(&Series)*Height};

	v2 BitmapCenter = 0.5f*V2i(Stamp->Width, Stamp->Height);

	real32 Radius = 5.0f;
	v2 P = Center + Offset - BitmapCenter;
	PushBitmap(RenderGroup, Stamp, ToV3(P, 0.0f));
      }
    }
  }
  RenderGroupToOutput(RenderGroup, Buffer);
  EndTemporaryMemory(GroundMemory);
}

internal void ClearBitmap(loaded_bitmap *Bitmap)
{
  if(Bitmap->Memory)
  {
    int32 TotalBitmapSize = Bitmap->Width*Bitmap->Height*BITMAP_BYTES_PER_PIXEL;
    ZeroSize(TotalBitmapSize, Bitmap->Memory);
  }
}

internal void MakeSphereNormalMap(loaded_bitmap *Bitmap, real32  Roughness)
{
  real32 InvHeight = 1.0f / (real32)(Bitmap->Height - 1);
  real32 InvWidth = 1.0f / (real32)(Bitmap->Width - 1);

  uint8 *Row = (uint8 *)Bitmap->Memory;

  for(int32 Y = 0; Y < Bitmap->Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row; 
    for(int32 X = 0; X < Bitmap->Width; ++X)
    {
      
      v2 BitmapUV = v2{InvWidth*(real32)X, InvHeight*(real32)Y};

      real32 Nx = 2.0f*BitmapUV.x - 1.0f;
      real32 Ny = 2.0f*BitmapUV.y - 1.0f;

      v3 Normal = {0, 0.7071f, 0.7071f};
      real32 Nz = 0.0f;
      real32 RootTerm = 1.0f - Nx*Nx - Ny*Ny;
      if(RootTerm >= 0.0f)
      {
	Nz = SquareRoot(RootTerm);
	Normal = v3{Nx, Ny, Nz};
      }


      v4 Color = v4{255.0f*(0.5f*(Normal.x + 1.0f)),
		    255.0f*(0.5f*(Normal.y + 1.0f)),
		    255.0f*(0.5f*(Normal.z + 1.0f)),
		    255.0f*Roughness};

      
      *Pixel++ = (((uint32)(Color.a + 0.5f) << 24) |
		  ((uint32)(Color.r + 0.5f) << 16) |
		  ((uint32)(Color.g + 0.5f) << 8) |
		  ((uint32)(Color.b + 0.5f) << 0));
    }
    Row += Bitmap->Pitch;
  }
}

internal void MakeSphereDiffuseMap(loaded_bitmap *Bitmap)
{
  real32 InvHeight = 1.0f / (real32)(Bitmap->Height - 1);
  real32 InvWidth = 1.0f / (real32)(Bitmap->Width - 1);

  uint8 *Row = (uint8 *)Bitmap->Memory;

  for(int32 Y = 0; Y < Bitmap->Height; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row; 
    for(int32 X = 0; X < Bitmap->Width; ++X)
    {
      
      v2 BitmapUV = v2{InvWidth*(real32)X, InvHeight*(real32)Y};

      real32 Nx = 2.0f*BitmapUV.x - 1.0f;
      real32 Ny = 2.0f*BitmapUV.y - 1.0f;

      real32 RootTerm = 1.0f - Nx*Nx - Ny*Ny;
      real32 Alpha = 0.0f;
      if(RootTerm >= 0.0f)
      {
	Alpha = 1.0;
      }

      v3 BaseColor = v3{0.0f, 0.0f, 0.0f};
      Alpha *= 255.0f;
      v4 Color = v4{Alpha*BaseColor.x,
		    Alpha*BaseColor.y,
		    Alpha*BaseColor.z,
		    Alpha};

      
      *Pixel++ = (((uint32)(Color.a + 0.5f) << 24) |
		  ((uint32)(Color.r + 0.5f) << 16) |
		  ((uint32)(Color.g + 0.5f) << 8) |
		  ((uint32)(Color.b + 0.5f) << 0));
    }
    Row += Bitmap->Pitch;
  }
}

internal loaded_bitmap MakeEmptyBitmap(memory_arena *Arena, int32 Width, int32 Height, bool32 ClearToZero = true)
{
  loaded_bitmap Result;
  Result.Width = Width;
  Result.Height = Height;
  Result.Pitch = Result.Width*BITMAP_BYTES_PER_PIXEL;
  int32 TotalBitmapSize = Width*Height*BITMAP_BYTES_PER_PIXEL;
  Result.Memory = PushSize(Arena, TotalBitmapSize);
  if(ClearToZero)
  {
    ClearBitmap(&Result);
  }
  return Result;
}

inline v2 TopDownAlign(loaded_bitmap *Bitmap, v2 Align)
{
  Align.y = (real32)(Bitmap->Height - 1) - Align.y;
  return Align;
}

#if 0
internal void RequestGroundBuffer(world_position CenterP, rectangle3 Bounds)
{
    Bounds = Offset(Bounds, CenterP.Offset_);
    CenterP.Offset_ = v3{0, 0, 0};
    for()
    {
    }
    FillGroundChunk(TranState, GameState, TranState->GroundBuffers, &GameState->CameraP);
}
#endif
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
 
    uint32 TilesPerWidth = 17;
    uint32 TilesPerHeight = 9;

    uint32 GroundBufferWidth = 256;
    uint32 GroundBufferHeight = 256;

    GameState->World = PushStruct(&GameState->WorldArena, world);

    world *World = GameState->World;
    AddLowEntity(GameState, EntityType_Null, NullPosition());
    GameState->TypicalFloorHeight = 3.0f;
    GameState->MetersToPixels = 48.0f;
    GameState->PixelsToMeters = 1.0f / GameState->MetersToPixels;
    v3 WorldChunkDimInMeters = {GameState->PixelsToMeters*(real32)GroundBufferWidth,
                                GameState->PixelsToMeters*(real32)GroundBufferHeight,
                                GameState->TypicalFloorHeight};
    InitializeWorld(World, WorldChunkDimInMeters);

    GameState->BackGround = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_img.bmp");
    GameState->Wall = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/brick.bmp", 40 ,80);
    GameState->Wizard.Wiz[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage1.bmp", 50, 145);
    GameState->Wizard.Wiz[1] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/mage2.bmp", 50, 145);
    GameState->Monster = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/monster.bmp", 40, 80);
    GameState->Sword = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/fireball.bmp", 25 ,25);
    GameState->Staff = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/staff.bmp");
    GameState->Stair = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/staff.bmp");
    GameState->Grass[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/Grass.bmp");
    GameState->Stones[0] = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/Dirt.bmp");
    
    
    GameState->NullCollision = MakeNullCollision(GameState);
    GameState->SwordCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 0.1f);
    

    real32 TileSideInMeter = 1.4f;
    real32 TileDepthInMeters = GameState->TypicalFloorHeight;
    GameState->StairCollision = MakeSimpleGroundCollision(GameState, TileSideInMeter,
							  2.0f*TileSideInMeter,
							  1.1f*TileDepthInMeters);
    
    GameState->PlayerCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 1.2f);
    
    GameState->MonsterCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 0.5f);
    
    GameState->WallCollision = MakeSimpleGroundCollision(GameState, TileSideInMeter,
							 TileSideInMeter,
							 TileDepthInMeters);

    GameState->StandardRoomCollision = MakeSimpleGroundCollision(GameState, TilesPerWidth*TileSideInMeter,
								 TilesPerHeight*TileSideInMeter,
								 0.9f*TileDepthInMeters);
    
    GameState->FamiliarCollision = MakeSimpleGroundCollision(GameState, 1.0f, 0.5f, 1.2f);
          
    uint32 ScreenBaseX = 0;
    uint32 ScreenBaseY = 0;
    uint32 ScreenBaseZ = 0;
    uint32 ScreenX = ScreenBaseX;
    uint32 ScreenY = ScreenBaseY;
    uint32 AbsTileZ = ScreenBaseZ;
    bool32 TopDoor = false;
    bool32 RigthDoor = false;
    bool32 BottomDoor = false;
    bool32 LeftDoor = false;
    bool32 UpDoor = false;
    bool32 DownDoor = false;

    random_series Series = {4567};

    for(uint32 ScreenIndex = 0; ScreenIndex < 20; ++ScreenIndex)
    {
      uint32 DoorDirection;
      if(UpDoor || DownDoor)
      {
	DoorDirection = RandomChoice(&Series, 2);
      }
      else
      {
	DoorDirection = RandomChoice(&Series, 3);
      }
      bool32 CreatedZDoor = false;
      if(DoorDirection == 2)
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
      else if(DoorDirection == 1)
      {
	RigthDoor = true;
      }
      else
      {
	TopDoor = true;
      }

      AddStandardRoom(GameState, ScreenX*TilesPerWidth + TilesPerWidth/2,
		      ScreenY*TilesPerHeight + TilesPerHeight/2,
		      AbsTileZ);
      
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
      if(DoorDirection == 2)
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
      else if(DoorDirection == 1)
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
  uint32 GroundBufferWidth = 256;
  uint32 GroundBufferHeight = 256;

  Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
  transient_state *TranState = (transient_state *)Memory->TransientStorage;
  if(!TranState->Initialized)
  {
    InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(transient_state), (uint8 *)Memory->TransientStorage + sizeof(transient_state));

    TranState->GroundBufferCount = 32;
    TranState->GroundBuffers = PushArray(&TranState->TranArena, TranState->GroundBufferCount, ground_buffer);
    for(uint32 GroundBufferIndex = 0; GroundBufferIndex < TranState->GroundBufferCount; ++GroundBufferIndex)
    {
      ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
      GroundBuffer->Bitmap = MakeEmptyBitmap(&TranState->TranArena, GroundBufferWidth, GroundBufferHeight, false);
      GroundBuffer->P = NullPosition();
    }
    GameState->TestDiffuse = MakeEmptyBitmap(&TranState->TranArena, 256, 256, 0);
    DrawRectangle(&GameState->TestDiffuse, v2{0, 0}, V2i(GameState->TestDiffuse.Width, GameState->TestDiffuse.Height), v4{0.4f, 0.4f, 0.4f, 1.0f});
    GameState->TestNormal = MakeEmptyBitmap(&TranState->TranArena, GameState->TestDiffuse.Width, GameState->TestDiffuse.Height, 0);
    MakeSphereNormalMap(&GameState->TestNormal, 0.0f);
    MakeSphereDiffuseMap(&GameState->TestDiffuse);


    TranState->EnvMapWidth = 512;
    TranState->EnvMapHeight = 256;
    for(uint32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
    {
      environment_map *Map = TranState->EnvMaps + MapIndex;
      uint32 Width = TranState->EnvMapWidth;
      uint32 Height = TranState->EnvMapHeight;
      for(uint32 LODIndex = 0; LODIndex < ArrayCount(Map->LOD); ++LODIndex)
      {
	Map->LOD[LODIndex] = MakeEmptyBitmap(&TranState->TranArena, Width, Height, true);
	Width >>= 1;
	Height >>= 1;
      }
    }
    
    TranState->Initialized = true;
  }

  if(Input->ExcutableReloaded)
  {
    for(uint32 GroundBufferIndex = 0; GroundBufferIndex < TranState->GroundBufferCount; ++GroundBufferIndex)
    {
      ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
      GroundBuffer->P = NullPosition();
    } 
  }
  
  #define WORLD_COUNT_X 256
  #define WORLD_COUNT_Y 256

  world *World = GameState->World;

  //uint32 TileSideInPixel = 60;
  //(real32)TileSideInPixel / (real32)World->TileSideInMeter
  real32 MetersToPixels = GameState->MetersToPixels;
  real32 PixelsToMeters = 1.0f/MetersToPixels;
  //real32 LowerLeftX = -(real32)(TileSideInPixel / 2);
  //real32 LowerLeftY = (real32)Buffer->Height;

  real32 ScreenCenterY = (real32)Buffer->Height*0.5f;
  real32 ScreenCenterX = (real32)Buffer->Width*0.5f;

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
	  ConHero->ddPlayer.x = -1.0f;
        }
        if(Controller->MoveRight.EndedDown)
        {
	  ConHero->ddPlayer.x = 1.0f;
        }
        if(Controller->MoveUp.EndedDown)
        {
          ConHero->ddPlayer.y = 1.0f;
        }
        if(Controller->MoveDown.EndedDown)
        {
	  ConHero->ddPlayer.y = -1.0f;
        }
      }
      if(Controller->Start.EndedDown)
      {
	//ConHero->dvZ = 3.0f;
      }
      ConHero->dSword = {};
      real32 ZoomRate = 0.0f;
      if(Controller->ActionUp.EndedDown)
      {
	ConHero->dSword = v2{0.0f, 1.0f};
	ZoomRate = 1.0f;
      }
      if(Controller->ActionDown.EndedDown)
      {
	ConHero->dSword = v2{0.0f, -1.0f};
	ZoomRate = -1.0f;
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
  temporary_memory RenderMemory = BeginTemporaryMemory(&TranState->TranArena);
  render_group *RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), GameState->MetersToPixels);

  loaded_bitmap DrawBuffer_ = {};
  loaded_bitmap* DrawBuffer = &DrawBuffer_;
  DrawBuffer->Width = Buffer->Width;
  DrawBuffer->Height = Buffer->Height;
  DrawBuffer->Pitch = Buffer->Pitch;
  DrawBuffer->Memory = Buffer->Memory;

  //DrawBitmap(DrawBuffer, &GameState->BackGround, 0, 0);
  Clear(RenderGroup, v4{0.2f, 0.2f, 0.2f});

  real32 ScreenCenX = 0.5f*(real32)DrawBuffer->Width;
  real32 ScreenCenY = 0.5f*(real32)DrawBuffer->Height;

  
  real32 ScreenWidthInMeters = DrawBuffer->Width*PixelsToMeters;
  real32 ScreenHeightInMeters = DrawBuffer->Height*PixelsToMeters;
  rectangle3 CameraBounds = RectCentDim(V3(0, 0, 0), V3(ScreenWidthInMeters, ScreenHeightInMeters, 0.0f));
  CameraBounds.Min.z = -3.0f*GameState->TypicalFloorHeight;
  CameraBounds.Max.z = 1.0f*GameState->TypicalFloorHeight;

#if 0
  for(uint32 GroundBufferIndex = 0; GroundBufferIndex < TranState->GroundBufferCount; ++GroundBufferIndex)
  {
    ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
    if(IsValid(&GroundBuffer->P))
    {
      loaded_bitmap *Bitmap = &GroundBuffer->Bitmap;
      v3 Delta = Subtract(GameState->World, &GroundBuffer->P, &GameState->CameraP);
      Bitmap->AlignX = Bitmap->Width/2;
      Bitmap->AlignY = Bitmap->Height/2;

      render_basis *Basis = PushStruct(&TranState->TranArena, render_basis);
      RenderGroup->DefaultBasis = Basis;
      Basis->P = Delta;
      PushBitmap(RenderGroup, Bitmap, V3(0, 0, 0));
    }
  }


  v2 ScreenCenter = v2{0.5f*(real32)DrawBuffer->Width, 0.5f*(real32)DrawBuffer->Height};
  {
    world_position MinChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMinCorner(CameraBounds));
    world_position MaxChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMaxCorner(CameraBounds));
    for (int32 ChunkZ = MinChunkP.ChunkZ; ChunkZ <= MaxChunkP.ChunkZ; ++ChunkZ)
    {
        for (int32 ChunkY = MinChunkP.ChunkY; ChunkY <= MaxChunkP.ChunkY; ++ChunkY)
        {
            for (int32 ChunkX = MinChunkP.ChunkX; ChunkX <= MaxChunkP.ChunkX; ++ChunkX)
            {
	        //world_chunk* Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
                //if (Chunk)
                {
		  world_position ChunkCenterP = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
		  v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);
		  v2 ScreenP = v2{ScreenCenter.x + MetersToPixels*RelP.x,
				  ScreenCenter.y - MetersToPixels*RelP.y};
		  v2 ScreenDim = MetersToPixels * World->ChunkDimInMeters.xy;
		  ground_buffer *FurthestBuffer = 0;
		  real32 FurthestBufferLengthSq = 0.0f;
		  for(uint32 GroundBufferIndex = 0; GroundBufferIndex < TranState->GroundBufferCount; ++GroundBufferIndex)
                  {
		    ground_buffer* GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
		    if(AreInTheSameChunk(World, &GroundBuffer->P, &ChunkCenterP))
                    {
		      FurthestBuffer = 0;
		      break;
		    }
		    else if(IsValid(&GroundBuffer->P))
                    {
		      v3 ReP = Subtract(World, &GroundBuffer->P, &GameState->CameraP);
		      real32 BufferLengthSq = LengthSq(ReP.xy);
		      if(FurthestBufferLengthSq < BufferLengthSq)
		      {
			FurthestBufferLengthSq = BufferLengthSq;
			FurthestBuffer = GroundBuffer;
		      }
		    }
		    else
		    {
		      FurthestBufferLengthSq = Real32Maximum;
		      FurthestBuffer = GroundBuffer;
		    }
		  }
		  if(FurthestBuffer)
		  {
		    FillGroundChunk(TranState, GameState, FurthestBuffer, &ChunkCenterP);
		  }
		  
		  //PushRectOutline(RenderGroup, RelP, World->ChunkDimInMeters.xy, v4{1.0f, 1.0f, 0.0f, 1.0f});
                }
            }
        }
    }
  }
#endif
  v3 SimBoundsExpansion = {15.0f, 15.0f, 0.0f};
  rectangle3 SimBounds = AddRadiusTo(CameraBounds, SimBoundsExpansion);

  temporary_memory SimMemory = BeginTemporaryMemory(&TranState->TranArena);

  
  real32 dtForFrame = Input->dtForFrame;
  world_position SimCenterP = GameState->CameraP;
  sim_region *SimRegion = BeginSim(&TranState->TranArena, GameState, GameState->World, SimCenterP, SimBounds, dtForFrame);
  v3 CameraP = Subtract(World, &GameState->CameraP, &SimCenterP);

  for(uint32 EntityIndex = 0; EntityIndex < SimRegion->EntityCount; ++EntityIndex)
  {
    sim_entity *Entity = SimRegion->Entities + EntityIndex;
    if(Entity->Updatable)
    {
      real32 dt = Input->dtForFrame;

      move_spec MoveSpec = DefaultMoveSpec();
      v3 ddP = {};

      render_basis *Basis =  PushStruct(&TranState->TranArena, render_basis);
      RenderGroup->DefaultBasis = Basis;

      v3 CameraRelGroundP = GetEntityGroundPoint(Entity) - CameraP;
      real32 FadeTopStartZ = 0.5f*GameState->TypicalFloorHeight;
      real32 FadeTopEndZ = 0.75f*GameState->TypicalFloorHeight;
      real32 FadeBottomStartZ = -2.0f*GameState->TypicalFloorHeight;
      real32 FadeBottomEndZ = -2.25f*GameState->TypicalFloorHeight;
      RenderGroup->GlobalAlpha = 1.0f;
      if(CameraRelGroundP.z > FadeTopStartZ)
      {
	RenderGroup->GlobalAlpha = Clamp01MapToRange(FadeTopEndZ, CameraRelGroundP.z, FadeTopStartZ);
      }
      else if(CameraRelGroundP.z < FadeBottomStartZ)
      {
	RenderGroup->GlobalAlpha = Clamp01MapToRange(FadeBottomEndZ, CameraRelGroundP.z, FadeBottomStartZ);
      }
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
      	      Entity->dvP.z = ConHero->dvZ;
      	    }

      	    MoveSpec.UnitMaxAccelVector = true;
      	    MoveSpec.Speed = 50.0f;
      	    MoveSpec.Drag = 8.0f;
	    ddP = V3(ConHero->ddPlayer, 0);
	    
      	    if((ConHero->dSword.x != 0) || (ConHero->dSword.y != 0))
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
      		
          //DrawRectangle(DrawBuffer, PlayerLeftTop, PlayerLeftTop + MetersToPixels * PlayerWidthHeigth, 1.0f, 0.0f, 0.0f);
	loaded_bitmap *Wizard = &GameState->Wizard.Wiz[Entity->WizFacingDirection];
	PushBitmap(RenderGroup, Wizard, v3{0, 0, 0});
	DrawHitpoints(Entity, RenderGroup);
        } break;
	
        case EntityType_Wall:
        {
          PushBitmap(RenderGroup, &GameState->Wall, v3{0, 0, 0});
        } break;
	
        case EntityType_Monster:
        {
	  if(Entity->HitPointMax == 0)
	  {
	    MakeEntityNonSpatial(Entity);
	  }
	  else
	  {
	    PushBitmap(RenderGroup, &GameState->Monster, v3{0, 0, 0});
	    DrawHitpoints(Entity, RenderGroup);
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
	  PushBitmap(RenderGroup, &GameState->Sword, v3{0, 0, 0});
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
	  PushBitmap(RenderGroup, Wizard, v3{0, 0, 0});
        } break;
	
        case EntityType_Staff:
        {
	  UpdateSword(SimRegion, Entity, dt);
	  PushBitmap(RenderGroup, &GameState->Staff, v3{0.5f, 0, 0});
        } break;

        case EntityType_Stair:
	{
	  PushRect(RenderGroup, v3{0, 0, 0}, Entity->WalkableDim, v4{1, 0.5f, 0, 1});
	  PushRect(RenderGroup, v3{0, 0, Entity-> WalkableHeight}, Entity->WalkableDim, v4{1, 1, 0, 1});
	} break;

        case EntityType_Space:
	{
	  for(uint32 VolumeIndex = 0; VolumeIndex < Entity->Collision->VolumeCount; ++VolumeIndex)
	  {
	    sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex; 
	    PushRectOutline(RenderGroup, Volume->OffsetP - v3{0, 0, 0.5f*Volume->Dim.z}, Volume->Dim.xy, v4{0.0f, 0.5f, 1, 1});
	  }
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

      Basis->P = GetEntityGroundPoint(Entity);
      RenderGroup->GlobalAlpha = 1.0f;
    }
  }

  GameState->Time += Input->dtForFrame;
#if 0
  real32 Angle = GameState->Time;
  real32 Disp = Cos(Angle)*50.0f;
  v2 Origin = ScreenCenter;
#if 1
  v2 XAxis = 100*v2{Cos(Angle), Sin(Angle)};
  v2 YAxis = v2{-XAxis.y, XAxis.x};
#else
  v2 XAxis = 100*v2{1, 0};
  v2 YAxis = 100*v2{0, 1};
#endif
  v3 MapColor[] = {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1}
  };

  int32 CheckerWidth = 16;
  int32 CheckerHeight = 16;
  
  for(uint32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
  {
    environment_map *Map = TranState->EnvMaps + MapIndex;
    loaded_bitmap *LOD = Map->LOD + 0;
    bool32 RowCheckerOn = false;
    for(int32 Y = 0; Y < LOD->Height; Y += CheckerHeight)
    {
      bool32 CheckerOn = RowCheckerOn;
      for(int32 X = 0; X < LOD->Width; X += CheckerWidth)
      {
	v4 Color = CheckerOn ? ToV4(MapColor[MapIndex], 1.0f) : v4{0, 0, 0, 1};
	v2 MinP = V2i(X, Y);
	v2 MaxP = MinP + V2i(CheckerWidth, CheckerHeight);
	DrawRectangle(LOD, MinP, MaxP, Color);
	CheckerOn = !CheckerOn;
      }
      RowCheckerOn = !RowCheckerOn;
    }
     
  }

  
  render_entry_coordinate_system* Entry = CoordinateSystem(RenderGroup, v2{Disp, 0} + Origin - 0.5f*XAxis - 0.5f*YAxis, XAxis, YAxis,
							   v4{1, 1, 1, 1}, &GameState->TestDiffuse,
							   &GameState->TestNormal,
							   TranState->EnvMaps + 2, TranState->EnvMaps + 1, TranState->EnvMaps + 0);

  v2 MapP = v2{0.0f, 0.0f};
  for(uint32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
  {
    environment_map *Map = TranState->EnvMaps + MapIndex;
    loaded_bitmap *LOD = Map->LOD + 0;

    XAxis = 0.5f*v2{(real32)LOD->Width, 0.0f};
    YAxis = 0.5f*v2{0.0f, (real32)LOD->Height};
    CoordinateSystem(RenderGroup, MapP, XAxis, YAxis, v4{1, 1, 1, 1}, LOD, 0, 0, 0, 0);
    MapP += YAxis + v2{0.0f, 6.0f};
    
  }

  TranState->EnvMaps[0].Pz = -2.0f;
  TranState->EnvMaps[1].Pz = 0.0f;
  TranState->EnvMaps[2].Pz = 2.0f;


  
  real32 X = Cos(Angle);
  if(X > 1 || X < 0)
  {
    X = 0;
  }
  Entry->Point = v2{X, 0.5f};
#endif
  RenderGroupToOutput(RenderGroup, DrawBuffer);
  
  EndSim(SimRegion, GameState);
  EndTemporaryMemory(SimMemory);
  EndTemporaryMemory(RenderMemory);

  CheckArena(&GameState->WorldArena);
  CheckArena(&TranState->TranArena);

}


#if defined __cplusplus
extern "C"
#endif
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state *GameState = (game_state*)Memory->PermanentStorage;
  GameOutputSound(GameState, SoundBuffer, 400);
}
