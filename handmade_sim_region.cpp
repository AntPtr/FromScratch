
inline v2 GetSimSpaceP(sim_region *SimRegion, low_entity *Stored)
{
  v2 Result = InvalidP;
  if(!IsSet(&Stored->Sim, EntityFlag_Nonspatial))
  {
    world_difference Diff = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
    Result = Diff.dXY;
  }
  return Result;
}


internal sim_entity_hash *GetHashFromStorageIndex(sim_region *SimRegion, uint32 StorageIndex)
{
  Assert(StorageIndex);
  
  sim_entity_hash *Result = 0;

  uint32 HashValue = StorageIndex;
  for(uint32 Offset = 0; Offset < ArrayCount(SimRegion->Hash); ++Offset)
  {
    uint32 HashMask = (ArrayCount(SimRegion->Hash) - 1);
    uint32 HashIndex = ((HashValue + Offset) & HashMask);
    sim_entity_hash *Entry = SimRegion->Hash + HashIndex;
    if((Entry->Index == StorageIndex) || (Entry->Index == 0))
    {
      Result = Entry;
      break;
    }
  }
  return Result;
}  

internal void MapStorageIndexToEntity(sim_region *SimRegion, uint32 StorageIndex, sim_entity *Entity)
{
  sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
  Assert((Entry->Index == 0) || (Entry->Index == StorageIndex));
  Entry->Index = StorageIndex;
  Entry->Ptr = Entity;
}

sim_entity *GetEntityByStorageIndex(sim_region *SimRegion, uint32 StorageIndex)
{
  sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
  sim_entity *Result =  Entry->Ptr;
  return Result;
}

internal sim_entity *AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v2 *SimP);

inline void LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference *Ref)
{
  if(Ref->Index)
  {
    sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
    if(Entry->Ptr == 0)
    {
      Entry->Index = Ref->Index;
      low_entity *LowEntity = GetLowEntity(GameState, Ref->Index);
      v2 P = GetSimSpaceP(SimRegion, LowEntity);
      Entry->Ptr = AddEntity(GameState, SimRegion, Ref->Index, LowEntity, &P);
    }
    Ref->Ptr = Entry->Ptr;
  }
}

internal sim_entity* AddEntityRaw(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source)
{
  Assert(StorageIndex);
  sim_entity *Entity = 0;

  sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
  if(Entry->Ptr == 0)
  {
    if(SimRegion->EntityCount < SimRegion->MaxEntityCount)
    {
      Entity = SimRegion->Entities + SimRegion->EntityCount++;
      Entry->Index = StorageIndex;
      Entry->Ptr = Entity;
      
      if(Source)
      {
        *Entity = Source->Sim;
        LoadEntityReference(GameState, SimRegion, &Entity->Sword);
    
        Assert(!IsSet(&Source->Sim, EntityFlag_Simming));
        AddFlag(&Source->Sim, EntityFlag_Simming);
      }
      Entity->Updatable = false;
      Entity->StorageIndex = StorageIndex;
    }
    else
    {
      InvalidCodePath;
    }
  }
  return Entity;
}

internal sim_entity *AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v2 *SimP)
{
  sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);
  if(Dest)
  {
    if(SimP)
    {
      Dest->P = *SimP;
      Dest->Updatable = IsInRectangle(SimRegion->Bounds, Dest->P);
    }
    else
    {
      Dest->P = GetSimSpaceP(SimRegion, Source);
    }
  }
  return Dest;
}

inline void StoredEntityReference(entity_reference *Ref)
{
  if(Ref->Ptr != 0)
  {
    Ref->Index = Ref->Ptr->StorageIndex;
  }
}

internal sim_region *BeginSim(memory_arena *SimArena, game_state *GameState, world *World, world_position Origin, rectangle2 Bounds)
{
  sim_region *SimRegion = PushStruct(SimArena, sim_region);
  ZeroStruct(SimRegion->Hash);

  real32 UpdateSafetyMargin = 1.0f;
  SimRegion->World = World;
  SimRegion->Origin = Origin;
  SimRegion->UpdatebleBounds = Bounds;
  SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatebleBounds, UpdateSafetyMargin, UpdateSafetyMargin);

  SimRegion->MaxEntityCount = 4096;
  SimRegion->EntityCount = 0;
  SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);
  
  world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
  world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));

  for(int32 ChunkY = MinChunkP.ChunkY; ChunkY < MaxChunkP.ChunkY; ++ChunkY)
  {
    for(int32 ChunkX = MinChunkP.ChunkX; ChunkX < MaxChunkP.ChunkX; ++ChunkX)
    {
      world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, SimRegion->Origin.ChunkZ);
      if(Chunk)
      {
	for(world_entity_block *Block = &Chunk->FirstBlock; Block; Block = Block->NextBlock)
        {
	  for(uint32 EntityIndexIndex = 0; EntityIndexIndex < Block->LowEntityCount; ++EntityIndexIndex)
          {
	    uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
	    low_entity *Low = GameState->LowEntities + LowEntityIndex;
	    if(!IsSet(&Low->Sim, EntityFlag_Nonspatial))
	    {
	      v2 SimSpaceP = GetSimSpaceP(SimRegion, Low);
	      if(IsInRectangle(SimRegion->Bounds, SimSpaceP))
	      {
		AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
	      }
	    }
          }
  	}
      }
    }
  }
  return SimRegion;
}


internal void EndSim(sim_region *SimRegion, game_state *GameState)
{
  sim_entity *Entity = SimRegion->Entities;
  for(uint32 EntityIndex = 0; EntityIndex < SimRegion->EntityCount; ++EntityIndex, ++Entity)
  {
    //Store entity to
    low_entity *Stored = GameState->LowEntities + Entity->StorageIndex;
    Assert(IsSet(&Stored->Sim, EntityFlag_Simming));
    Stored->Sim = *Entity;
    Assert(!IsSet(&Stored->Sim, EntityFlag_Simming));
    StoredEntityReference(&Stored->Sim.Sword);

    world_position NewP =IsSet(Entity, EntityFlag_Nonspatial) ? NullPosition() :
      MapIntoChunkSpace(GameState->World, SimRegion->Origin, Entity->P);
    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->StorageIndex, Stored, NewP);
    //Entity map Hash table
    if(Entity->StorageIndex == GameState->CameraFollowEntityIndex)
    {
      world_position NewCameraP = GameState->CameraP;
      NewCameraP.ChunkZ = Stored->P.ChunkZ;
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
      NewCameraP = Stored->P;
#endif
      GameState->CameraP = NewCameraP;

    }
  }
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


internal void MoveEntity(sim_region *SimRegion, sim_entity *Entity, real32 dt, MoveSpec *MoveSpec, v2 ddPlayer)
{
      Assert(!IsSet(Entity, EntityFlag_Nonspatial));
  
      real32 Velocity = 60.0f;
      world *World = SimRegion->World;
      if(MoveSpec->UnitMaxAccelVector)
      {
        real32 ddPlayerLenght = LenghtSq(ddPlayer);
        if(ddPlayerLenght > 1.0f)
        {
	  ddPlayer *= 1.0f/SquareRoot(ddPlayerLenght);
        }
      }
      v2 OldPlayerP = Entity->P;
      
      ddPlayer *= MoveSpec->Speed;
      ddPlayer += -MoveSpec->Drag*Entity->dvP;
      
      v2 PlayerDelta = 0.5f*ddPlayer*Square(dt) + Entity->dvP*dt;
      Entity->dvP = ddPlayer*dt + Entity->dvP;
      
      v2 NewPlayerP = OldPlayerP + PlayerDelta;
     
      for(uint32 Iteration = 0; Iteration < 4; ++Iteration)
      {
	real32 tMin = 1.0f;
	v2 WallNormal = v2{};
	sim_entity *HitEntity = 0;

	v2 DesiredPosition = Entity->P + PlayerDelta;
	if(IsSet(Entity, EntityFlag_Collides) && !IsSet(Entity, EntityFlag_Nonspatial))
	{
          for(uint32 TestHighEntityIndex = 0; TestHighEntityIndex < SimRegion->EntityCount; ++TestHighEntityIndex)
          {
	    sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
	    if(Entity != TestEntity)
	    {
	      if(IsSet(TestEntity, EntityFlag_Collides) && !IsSet(TestEntity, EntityFlag_Nonspatial))
	      {
	        real32 DiameterW = TestEntity->Width + Entity->Width;
	        real32 DiameterH = TestEntity->Height + Entity->Height;
	        
	        v2 MinCorner = {DiameterW * -0.5f, DiameterH *-0.5f};
	        v2 MaxCorner = {DiameterW * 0.5f, DiameterH * 0.5f};
	          
	        v2 Rel = Entity->P - TestEntity->P;
	        if(TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
	        {
	          WallNormal = v2{-1, 0};
		  HitEntity = TestEntity;
	        }
	        if(TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMin, MinCorner.Y, MaxCorner.Y))
	        {
	          WallNormal = v2{1, 0};
		  HitEntity = TestEntity;
	        }
	        if(TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
                {
	          WallNormal = v2{0, -1};
		  HitEntity = TestEntity;
	        }
	        if(TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMin, MinCorner.X, MaxCorner.X))
	        {
	          WallNormal = v2{0, 1};
		  HitEntity = TestEntity;		
	        }
	      }
	    }
          }
	}
	Entity->P += tMin * PlayerDelta;	

	if(HitEntity)
	{
	  Entity->dvP = Entity->dvP - 1*DotProduct(Entity->dvP, WallNormal)*WallNormal;
	  PlayerDelta = DesiredPosition - Entity->P;
	  PlayerDelta = PlayerDelta - 1*DotProduct(PlayerDelta, WallNormal)*WallNormal;
	  
	  //TODO: this is stairs
	  //Entity->AbsTileZ += HitLow->dAbsTileZ;
	}
	else
	{
	  break;
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
