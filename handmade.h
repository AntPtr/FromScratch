#if !defined(HANDMADE_H)
#include <math.h>
#include <stdint.h>


#define local_persist static
#define global_variable static
#define internal static
#define Pi32 3.14159265359f
#define Minimum(A, B) ((A < B) ? A : B)
#define Maximum(A, B) ((A > B) ? A : B)


#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#endif

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef size_t memory_index;

typedef int32 bool32;

typedef float real32;
typedef double real64;

#include "handmade_intrisic.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#if HANDMADE_SLOW
#define Assert(Expr) if(!(Expr)) {*(int *)0=0;}
#else
#define Assert(Expr)
#endif
#define InvalidCodePath Assert(!"InvalidCodePath")

struct thread_context
{
  int placeholder;
};

#if H_INTERNAL
struct debug_read_file_result
{
  uint32 ContentSize;
  void* Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name (thread_context *Thread ,void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name (thread_context *Thread ,char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name (thread_context *Thread ,char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

#define Kilobytes(value) (value*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

inline uint32 SafeTruncateUInt64 (uint64 Value)
{
  Assert(Value <= 0xFFFFFFFF);
  uint32 Result = (uint32)Value;
  return Result;
}

//Services that game provide for the platform layer
struct game_offscreen_buffer
{
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct game_sound_output_buffer
{
  int SamplesPerSecond;
  int SampleCount;
  int16 *Samples;
};

struct game_button_state
{
  int HalfTransitionCount;
  bool32 EndedDown;
};

struct game_controller_input
{
  bool32 IsAnalog;
  bool32 IsConnected;
  
  real32 AvarageStickX;
  real32 AvarageStickY;

  union
  {
    game_button_state Buttons[12];
    struct
    {
      game_button_state MoveUp;
      game_button_state MoveDown;
      game_button_state MoveLeft;
      game_button_state MoveRight;

      game_button_state ActionUp;
      game_button_state ActionDown;
      game_button_state ActionLeft;
      game_button_state ActionRight;
      
      game_button_state LeftShoulder;
      game_button_state RigthShoulder;

      game_button_state Start;
      game_button_state Back;

      //Just for checking that the size of struct match with Buttons array
      game_button_state Terminator;
    };
  };
};

struct game_input
{
  game_button_state MouseButtons[5];
  int32 MouseX, MouseY, MouseZ;
  real32 dtForFrame;
  game_controller_input Controllers[5];
};

inline game_controller_input *GetController(game_input *Input, int ControllerIndex)
{
  Assert(ControllerIndex < ArrayCount(Input->Controllers));
  game_controller_input *Result = &Input->Controllers[ControllerIndex];
  return Result;
}

struct memory_arena
{
  memory_index Size;
  uint8 *Base;
  memory_index Used;
};

/*struct world
{
  tile_map *TileMap; 
  };*/

struct loaded_bitmap
{
  int32 Width;
  int32 Height;
  uint32 *Pixels;
};

struct wizard
{
  loaded_bitmap Wiz[2];
};


/*struct high_entity
{
  v2 dvP;
  v2 P;
  uint32 ChunkZ;
  uint32 WizFacingDirection;

  real32 Z;
  real32 dvZ;
  uint32 LowEntityIndex;
};*/



#define HIT_POINT_SUB_COUNT 4


struct low_entity
{
  sim_entity Sim;
  world_position P;
};
/*
struct entity
{
  uint32 LowIndex;
  low_entity *Low;
  high_entity *High;
};
*/
struct entity_visible_piece
{
  loaded_bitmap *Bitmap;
  v2 Offset;
  real32 OffsetZ;
  real32 R, G, B, A;
  v2 Dim;
};

struct controlled_hero
{
  uint32 EntityIndex;
  v2 ddPlayer;
  v2 dSword;
  real32 dvZ;
};

struct pairwise_collision_rule
{
  bool32 CanCollide;
  uint32 StorageIndexA;
  uint32 StorageIndexB;

  pairwise_collision_rule *NextInHash;
};

struct game_state
{
  memory_arena WorldArena;
  world* World;
  world_position CameraP;

  uint32 PlayerCount;
  controlled_hero ControlledHeroes[ArrayCount(((game_input *)0)->Controllers)];

  uint32 LowEntityCount;
  low_entity LowEntities[100000];
  
  uint32 CameraFollowEntityIndex;
  real32 MetersToPixels;
  
  loaded_bitmap BackGround;
  loaded_bitmap Wall;
  loaded_bitmap Monster;
  loaded_bitmap Sword;
  loaded_bitmap Staff;
  loaded_bitmap Stair;
  wizard Wizard;

  pairwise_collision_rule *CollisionRuleHash[256];
  pairwise_collision_rule *FirstFreeCollisionRule;

  sim_entity_collision_volume_group *NullCollision;
  sim_entity_collision_volume_group *SwordCollision;
  sim_entity_collision_volume_group *PlayerCollision;
  sim_entity_collision_volume_group *StairCollision;
  sim_entity_collision_volume_group *MonsterCollision;
  sim_entity_collision_volume_group *WallCollision;
  sim_entity_collision_volume_group *FamiliarCollision;
};

struct entity_visible_piece_group
{
  game_state *GameState;
  uint32 Count;
  entity_visible_piece Pieces[32];
};

struct game_memory
{
  bool32 IsInitialized;

  uint64 PermanentStorageSize;
  void *PermanentStorage;

  uint64 TransientStorageSize;
  void *TransientStorage;

  debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
  debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
  debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};

internal void InitializeArena(memory_arena *Arena, memory_index Size, void* Base)
{
  Arena->Size = Size;
  Arena->Base = (uint8 *)Base;
  Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize(Arena, Count * sizeof(type))
internal void *PushSize(memory_arena *Arena, memory_index Size)
{
  Assert((Arena->Used + Size) <= Arena->Size );
  void *Result = Arena->Base + Arena->Used;
  Arena->Used += Size;
  return Result;
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &Instance)
inline void ZeroSize(memory_index Size, void *Ptr)
{
  uint8 *Byte = (uint8 *)Ptr;
  while(Size--)
  {
    *Byte++= 0;
  }
}

#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread ,game_memory *Memory, game_input *Input, game_offscreen_buffer* Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);


#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context *Thread ,game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

inline low_entity *GetLowEntity(game_state *GameState, uint32 LowIndex)
{
  low_entity *Result = 0;
  if((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
  {
    Result = GameState->LowEntities + LowIndex;
  }
  return Result;
}

internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);

internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex);

#define HANDMADE_H
#endif
