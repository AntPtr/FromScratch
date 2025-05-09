
inline v3 GetSimSpaceP(sim_region *SimRegion, low_entity *Stored)
{
  v3 Result = InvalidP;
  if(!IsSet(&Stored->Sim, EntityFlag_Nonspatial))
  {
    Result = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
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


internal bool32 CanCollide(game_state *GameState, sim_entity *A, sim_entity *B)
{
  bool32 Result = false;

  if(A->StorageIndex > B->StorageIndex)
  {
    sim_entity *Temp = A;
    A = B;
    B = Temp;
  }
  
  if(!IsSet(A, EntityFlag_Nonspatial) && !IsSet(B, EntityFlag_Nonspatial))
  {
    Result = true;
  }


  uint32 HashBucket = A->StorageIndex & (ArrayCount(GameState->CollisionRuleHash) -1);
  for(pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket]; Rule; Rule = Rule->NextInHash)
  {
    if((Rule->StorageIndexA == A->StorageIndex) && (Rule->StorageIndexB == B->StorageIndex))
      {
	Result = Rule->CanCollide;
	break;
      }
  }

  return Result;
}



internal bool32 HandleCollision(game_state *GameState, sim_entity *A, sim_entity *B)
{
  bool32 StopOnCollision = false;
  if(A->Type == EntityType_Sword)
  {
    AddCollisionRule(GameState, A->StorageIndex, B->StorageIndex, false);
    StopOnCollision = true;
  }
  else
  {
    StopOnCollision = false;
  }
  
  if(A->Type > B->Type)
  {
    sim_entity *Temp = A;
    A = B;
    B = Temp;
  }
  
  if((A->Type == EntityType_Monster) && (B->Type == EntityType_Sword))
  {
    if(A->HitPointMax > 0)
    {
      --A->HitPointMax;
    }
  }

  if((A->Type == EntityType_Stair) || (B->Type == EntityType_Stair))
  {
    StopOnCollision = false;
  }
  return StopOnCollision;
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

internal sim_entity *AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v3 *SimP);

inline void LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference *Ref)
{
  if(Ref->Index)
  {
    sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
    if(Entry->Ptr == 0)
    {
      Entry->Index = Ref->Index;
      low_entity *LowEntity = GetLowEntity(GameState, Ref->Index);
      v3 P = GetSimSpaceP(SimRegion, LowEntity);
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

internal bool32 EntityOverlapsRectangle(v3 P, sim_entity_collision_volume Volume, rectangle3 Rect)
{
  rectangle3 Grown = AddRadiusTo(Rect, 0.5f*Volume.Dim);
  bool32 Result = IsInRectangle(Grown, P + Volume.OffsetP);
  return Result;
}

internal sim_entity *AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v3 *SimP)
{
  sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);
  if(Dest)
  {
    if(SimP)
    {
      Dest->P = *SimP;
      Dest->Updatable = EntityOverlapsRectangle(Dest->P, Dest->Collision->TotalVolume, SimRegion->Bounds);
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

internal sim_region *BeginSim(memory_arena *SimArena, game_state *GameState, world *World, world_position Origin, rectangle3 Bounds,  real32 dt)
{
  sim_region *SimRegion = PushStruct(SimArena, sim_region);
  ZeroStruct(SimRegion->Hash);

  real32 UpdateSafetyMarginZ = 1.0f;
  SimRegion->MaxEntityRadius = 5.0f;
  SimRegion->MaxEntityVelocity = 30.0f;
  real32 UpdateSafetyMargin = SimRegion->MaxEntityRadius + dt*SimRegion->MaxEntityVelocity;
  
  SimRegion->World = World;
  SimRegion->Origin = Origin;
  SimRegion->UpdatebleBounds = AddRadiusTo(Bounds, V3(SimRegion->MaxEntityRadius,
						      SimRegion->MaxEntityRadius,
						      SimRegion->MaxEntityRadius));

  SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatebleBounds,
				  V3(UpdateSafetyMargin, UpdateSafetyMargin, UpdateSafetyMarginZ));

  SimRegion->MaxEntityCount = 4096;
  SimRegion->EntityCount = 0;
  SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);
  
  world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
  world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
  for(int32 ChunkZ = MinChunkP.ChunkZ; ChunkZ < MaxChunkP.ChunkZ; ++ChunkZ)
  {
    for(int32 ChunkY = MinChunkP.ChunkY; ChunkY < MaxChunkP.ChunkY; ++ChunkY)
    {
      for(int32 ChunkX = MinChunkP.ChunkX; ChunkX < MaxChunkP.ChunkX; ++ChunkX)
      {
        world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
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
		v3 SimSpaceP = GetSimSpaceP(SimRegion, Low);
		if(EntityOverlapsRectangle(SimSpaceP, Low->Sim.Collision->TotalVolume, SimRegion->Bounds))
		{
		  AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
		}
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
      real32 CameraZOffset = NewCameraP.Offset_.Z;
      NewCameraP = Stored->P;
      NewCameraP.Offset_.Z = CameraZOffset;
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

internal bool32 CanOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region)
{
  bool32 Result = false;
  if(Mover != Region)
  {
    if(Region->Type == EntityType_Stair)
    {
      Result = true;
    }
  }
  return Result;
}


internal void HandleOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region, real32 dt, real32 *Ground)
{
  if(Region->Type == EntityType_Stair)
  {
    *Ground = GetStairGroundPoint(Region, GetEntityGroundPoint(Mover));
  }
}

internal bool32 SpeculativeCollide(sim_entity *Mover, sim_entity *Region)
{
  bool32 Result = true;
  if(Region->Type == EntityType_Stair)
  {
    real32 StepHeight = 0.1f;
#if 0
    Result = ((AbsoluteValue(GetEntityGroundPoint(Mover).Z - Ground) > StepHeight) ||
	      ((Bary.Y > 0.1f) && (Bary.Y < 0.9f)));
#endif
    v3 MoverGroundPoint = GetEntityGroundPoint(Mover);
    real32 Ground = GetStairGroundPoint(Region, MoverGroundPoint);
    Result = (AbsoluteValue(MoverGroundPoint.Z - Ground) > StepHeight);
  }
  return Result;
}

internal void MoveEntity(game_state *GameState, sim_region *SimRegion, sim_entity *Entity, real32 dt, move_spec *MoveSpec, v3 ddPlayer)
{
  Assert(!IsSet(Entity, EntityFlag_Nonspatial));
  
  real32 Velocity = 60.0f;
  world *World = SimRegion->World;

  uint32 OverlappingCount = 0;
  //sim_entity *OverlappingEntities[16];
      
  if(MoveSpec->UnitMaxAccelVector)
  {
    real32 ddPlayerLenght = LenghtSq(ddPlayer);
    if(ddPlayerLenght > 1.0f)
    {
      ddPlayer *= 1.0f/SquareRoot(ddPlayerLenght);
    }
  }
  v3 OldPlayerP = Entity->P;
  v3 Drag =  -MoveSpec->Drag*Entity->dvP;
  Drag.Z = 0.0f;
  ddPlayer *= MoveSpec->Speed;
  ddPlayer += Drag;
  if(!IsSet(Entity, EntityFlag_ZSupported))
  {
    ddPlayer += V3(0, 0, -9.8f);//Gravity
  }
  v3 PlayerDelta = 0.5f*ddPlayer*Square(dt) + Entity->dvP*dt;
  Entity->dvP = ddPlayer*dt + Entity->dvP;

  Assert(LenghtSq(Entity->dvP) <= Square(SimRegion->MaxEntityVelocity));
      
  v3 NewPlayerP = OldPlayerP + PlayerDelta;

  real32 tMin = 1.0f;
      
  real32 DistanceRemainig = Entity->DistanceLimit;
  if(DistanceRemainig == 0)
  {
    DistanceRemainig = 10000.0f;
  }

  for(uint32 Iteration = 0; Iteration < 4; ++Iteration)
  {
    real32 PlayerDeltaLength = Length(PlayerDelta);
    if(PlayerDeltaLength > 0)
    {
         
      if(PlayerDeltaLength > DistanceRemainig)
      {
	tMin = (DistanceRemainig / PlayerDeltaLength);
      }
      v3 WallNormal = v3{};
      sim_entity *HitEntity = 0;
      bool32 StopsOnCollision = IsSet(Entity, EntityFlag_Collides);
	  
      v3 DesiredPosition = Entity->P + PlayerDelta;
      if(!IsSet(Entity, EntityFlag_Nonspatial))
      {
	for(uint32 TestHighEntityIndex = 0; TestHighEntityIndex < SimRegion->EntityCount; ++TestHighEntityIndex)
	{
	  sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
	  if(TestEntity->Type == EntityType_Stair)
	  {
	    int yes = 4;
	  }
	  if(CanCollide(GameState, Entity, TestEntity))
	  {
	    for(uint32 EntityVolumeIndex = 0; EntityVolumeIndex < Entity->Collision->VolumeCount; ++EntityVolumeIndex)
	    {
	      sim_entity_collision_volume *Volume = Entity->Collision->Volumes + EntityVolumeIndex;
	      for(uint32 TestVolumeIndex = 0; TestVolumeIndex < TestEntity->Collision->VolumeCount; ++TestVolumeIndex)
	      {
		sim_entity_collision_volume *TestVolume = TestEntity->Collision->Volumes + TestVolumeIndex;

	        v3 MinkowskiDiameter = {TestVolume->Dim.X + Volume->Dim.X,
					TestVolume->Dim.Y + Volume->Dim.Y,
					TestVolume->Dim.Z + Volume->Dim.Z};
                     
	        v3 MinCorner = -0.5f*MinkowskiDiameter;
	        v3 MaxCorner = 0.5f*MinkowskiDiameter;
                       
	        v3 Rel = (Entity->P + Volume->OffsetP) - (TestEntity->P + TestVolume->OffsetP);
	        if((Rel.Z >= MinCorner.Z) && (Rel.Z < MaxCorner.Z))
	        {
	          real32 tMinTest = tMin;
	          v3 TestWallNormal = v3{};
	          bool32 HitThis = false;
	          if(TestEntity->Type == EntityType_Stair)
	          {
	            int yes = 4;
	          }
	          if(TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMinTest, MinCorner.Y, MaxCorner.Y))
	          {
	            TestWallNormal = v3{-1, 0, 0};
	            HitThis = true;
	          }
	          if(TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, &tMinTest, MinCorner.Y, MaxCorner.Y))
	          {
	            TestWallNormal = v3{1, 0, 0};
	            HitThis = true;
	          }
	          if(TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMinTest, MinCorner.X, MaxCorner.X))
	          {
	            TestWallNormal = v3{0, -1, 0};
	            HitThis = true;
	          }
	          if(TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, &tMinTest, MinCorner.X, MaxCorner.X))
	          {
	            TestWallNormal = v3{0, 1, 0};
	            HitThis = true;		
	          }
	          if(HitThis)
	          {
	            v3 TestP = Entity->P + tMinTest*PlayerDelta;
	            if(SpeculativeCollide(Entity, TestEntity))
	            {
	          	tMin = tMinTest;
	          	WallNormal = TestWallNormal;
	          	HitEntity = TestEntity;
	            }
	          }
	        }
	      }  
	    }
	  }
	}
      }
      Entity->P += tMin * PlayerDelta;
      DistanceRemainig -= tMin*PlayerDeltaLength;  

      if(HitEntity)
      {
	PlayerDelta = DesiredPosition - Entity->P;
	bool32 StopOnCollision = HandleCollision(GameState, Entity, HitEntity);
	if(StopsOnCollision)
	{
	  PlayerDelta = PlayerDelta - 1*DotProduct(PlayerDelta, WallNormal)*WallNormal;
	  Entity->dvP = Entity->dvP - 1*DotProduct(Entity->dvP, WallNormal)*WallNormal;
	}
      }
      else
      {
	break;
      }
    }
    else
    {
      break;
    }
  }

  //This is for handling overlapping areas
  real32 Ground = 0.0f;
  {
    rectangle3 EntityRect = RectCentDim(Entity->P + Entity->Collision->TotalVolume.OffsetP, Entity->Collision->TotalVolume.Dim);
    for(uint32 TestHighEntityIndex = 0; TestHighEntityIndex < SimRegion->EntityCount; ++TestHighEntityIndex)
      {
	sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
	if(CanOverlap(GameState, Entity, TestEntity))
	{
	  rectangle3 TestEntityRect = RectCentDim(TestEntity->P + TestEntity->Collision->TotalVolume.OffsetP,
						  TestEntity->Collision->TotalVolume.Dim);
	  if(RectanglesIntersect(EntityRect, TestEntityRect))
	  {
	    HandleOverlap(GameState, Entity, TestEntity, dt, &Ground);
	  }
	}
      }
  }

  Ground += Entity->P.Z - GetEntityGroundPoint(Entity).Z;      
  if((Entity->P.Z <= Ground) || (IsSet(Entity, EntityFlag_ZSupported) && (Entity->dvP.Z == 0.0f)))
  {
    Entity->P.Z = Ground;
    Entity->dvP.Z = 0;
    AddFlag(Entity, EntityFlag_ZSupported);
  }
  else
  {
    ClearFlag(Entity, EntityFlag_ZSupported);
  }

  if(Entity->DistanceLimit != 0)
  {
    Entity->DistanceLimit = DistanceRemainig;
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
