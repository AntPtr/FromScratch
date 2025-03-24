#if !defined(HANDMADE_SIM_REGION_H)
struct move_spec
{
  bool32 UnitMaxAccelVector;
  real32 Speed;
  real32 Drag;
};

enum entity_type
{
  EntityType_Null,
  EntityType_Hero,
  EntityType_Wall,
  EntityType_Familiar,
  EntityType_Monster,
  EntityType_Sword,
  EntityType_Staff,
};

struct hit_point
{
  uint8 Flags;
  uint8 FilledAmount;
};

struct sim_entity;

union entity_reference
{
  sim_entity *Ptr;
  uint32 Index;
};

enum sim_entity_flags
{
  EntityFlag_Collides = (1 << 1),
  EntityFlag_Nonspatial = (1 << 2),

  EntityFlag_Simming = (1 << 30),
};

struct sim_entity
{
  uint32 StorageIndex;
  bool32 Updatable;

  uint32 Flags;
  
  v2 P;
  uint32 ChunkZ;

  real32 Z;
  real32 dvZ;
  entity_type Type;
  uint32 WizFacingDirection;
  
  v2 dvP;
  real32 Height;
  real32 Width;

  real32 DistanceLimit;
  
  //This is for ladders
  int32 dAbsTileZ;
  // uint32 HighEntityIndex;

  uint32 HitPointMax;
  hit_point HitPoint[16];

  entity_reference Sword;
  entity_reference Staff;
  
};

struct sim_entity_hash
{
  sim_entity *Ptr;
  uint32 Index;
};

struct sim_region
{
  world *World;
  
  world_position Origin;
  rectangle2 Bounds;
  rectangle2 UpdatebleBounds;
  
  uint32 MaxEntityCount;
  uint32 EntityCount;
  sim_entity *Entities;

  sim_entity_hash Hash[4096];
};

#define HANDMADE_SIM_REGION_H
#endif
