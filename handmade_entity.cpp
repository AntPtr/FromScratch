move_spec DefaultMoveSpec()
{
  move_spec Result;
  Result.UnitMaxAccelVector = false;
  Result.Speed = 1.0f;
  Result.Drag = 0.0f;
  return Result;
}

inline void UpdateFamiliar(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}

inline void UpdateMonster(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}

inline void UpdateSword(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}

inline void UpdateStaff(sim_region *SimRegion, sim_entity *Entity, real32 dt)
{
}
