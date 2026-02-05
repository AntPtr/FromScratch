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
  EntityType_Space,
  EntityType_Hero,
  EntityType_Wall,
  EntityType_Familiar,
  EntityType_Monster,
  EntityType_Sword,
  EntityType_Staff,
  EntityType_Stair,
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
  EntityFlag_Collides = (1 << 0),
  EntityFlag_Nonspatial = (1 << 1),
  EntityFlag_Moveable = (1 << 2),
  EntityFlag_ZSupported = (1 << 3),
  EntityFlag_Traversable = (1<< 4),
  EntityFlag_Simming = (1 << 30),
};

struct sim_entity_collision_volume
{
  v3 OffsetP;
  v3 Dim;
};

struct sim_entity_collision_volume_group
{
  uint32 VolumeCount;
  sim_entity_collision_volume TotalVolume;
  sim_entity_collision_volume *Volumes;
};

struct sim_entity
{
  world_chunk *OldChunk;
  uint32 StorageIndex;
  bool32 Updatable;

  uint32 Flags;
  
  v3 P;

  //real32 Z;
  //real32 dvZ;
  entity_type Type;
  real32 WizFacingDirection;
  
  v3 dvP;

  sim_entity_collision_volume_group *Collision;
  
  real32 DistanceLimit;
  
  int32 dAbsTileZ;

  v2 WalkableDim;
  real32 WalkableHeight;

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

  real32 MaxEntityRadius;
  real32 MaxEntityVelocity;
  
  world_position Origin;
  rectangle3 Bounds;
  rectangle3 UpdatebleBounds;
  
  uint32 MaxEntityCount;
  uint32 EntityCount;
  sim_entity *Entities;

  sim_entity_hash Hash[4096];
};

#define HANDMADE_SIM_REGION_H
#endif
