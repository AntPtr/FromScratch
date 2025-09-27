#if !defined(HANDMADE_ENTITY_H)

#define InvalidP v3{100000.0f, 100000.0f, 100000.0f}

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

inline void MakeEntitySpatial(sim_entity *Entity, v3 P, v3 dP)
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

internal v3 GetEntityGroundPoint(sim_entity *Entity, v3 ForEntity)
{
  v3 Result = ForEntity;
  return Result;
}

internal v3 GetEntityGroundPoint(sim_entity *Entity)
{
  v3 Result = GetEntityGroundPoint(Entity, Entity->P);
  return Result;
}

inline real32 GetStairGroundPoint
(sim_entity *Entity, v3 AtEntityGroundPoint)
{
  rectangle2 RegionRect = RectCentDim(Entity->P.xy, Entity->WalkableDim);
  v2 Bary = Clamp01(GetBarycentric(RegionRect, AtEntityGroundPoint.xy));
  real32 Result = Entity->P.z + Bary.y*Entity->WalkableHeight;
  
  return Result;
}

#define HANDMADE_ENTITY_H
#endif
