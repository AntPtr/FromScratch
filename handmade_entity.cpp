MoveSpec DefaultMoveSpec()
{
  MoveSpec Result;
  Result.UnitMaxAccelVector = false;
  Result.Speed = 1.0f;
  Result.Drag = 0.0f;
  return Result;
}

inline void UpdateFamiliar(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
  sim_entity *ClosestHero = 0;
  real32 ClosestHeroDSq = Square(10.0f); //Maximum search radius

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
  v2 ddP = {};
  if(ClosestHero && (ClosestHeroDSq > Square(2.0f)))
  {
    real32 Acceleration = 0.5f;
    real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
    ddP = OneOverLength*(ClosestHero->P - Entity->P);
  }
  MoveSpec MoveSpec = DefaultMoveSpec();
  MoveSpec.UnitMaxAccelVector = true;
  MoveSpec.Speed = 50.0f;
  MoveSpec.Drag = 8.0f;
  MoveEntity(SimRegion, Entity, dt, &MoveSpec, ddP);
}

inline void UpdateMonster(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}

inline void UpdateSword(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
  if(IsSet(Entity, EntityFlag_Nonspatial))
  {
  }
  else
  {
    MoveSpec MoveSpec = DefaultMoveSpec();
    MoveSpec.UnitMaxAccelVector = false;
    MoveSpec.Speed = 0.0f;
    MoveSpec.Drag = 0.0f;

    v2 OldP = Entity->P;
    MoveEntity(SimRegion, Entity, dt, &MoveSpec, v2{0 ,0});
    real32 DistanceTraveled = Length(Entity->P - OldP);

    Entity->DistanceRemaining -= DistanceTraveled;
    if(Entity->DistanceRemaining < 0)
    {
      MakeEntityNonSpatial(Entity);
    }
  }
}

inline void UpdateStaff(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
  MoveSpec MoveSpec = DefaultMoveSpec();
  MoveSpec.UnitMaxAccelVector = false;
  MoveSpec.Speed = 0.0f;
  MoveSpec.Drag = 0.0f;

  v2 OldP = Entity->P;
  MoveEntity(SimRegion, Entity, dt, &MoveSpec, v2{0 ,0});
  real32 DistanceTraveled = Length(Entity->P - OldP);

  Entity->DistanceRemaining -= DistanceTraveled;
  if(Entity->DistanceRemaining < 0)
  {
    Assert("THIS SHOULD BE REMOVED HERE");
  }
}
