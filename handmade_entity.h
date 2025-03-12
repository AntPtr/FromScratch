#if !defined(HANDMADE_ENTITY_H)

#define InvalidP v2{100000.0f, 100000.0f}

inline void AddFlag(sim_entity *Entity, uint32 Flags)
{
  Entity->Flags |= Flags;
}

inline void ClearFlag(sim_entity *Entity, uint32 Flags)
{
  Entity->Flags &= ~Flags;
}

inline void MakeEntityNonSpatial(sim_entity *Entity)
{
  AddFlag(Entity, EntityFlag_Nonspatial);
  Entity->P = InvalidP;
}

inline void MakeEntitySpatial(sim_entity *Entity, v2 P, v2 dP)
{
  ClearFlag(Entity, EntityFlag_Nonspatial);
  Entity->P = P;
  Entity->dvP = dP;
}

inline bool32 IsSet(sim_entity *Entity, uint32 Flags)
{
  bool32 Result = Entity->Flags & Flags;
  return Result;
}

#define HANDMADE_ENTITY_H
#endif
