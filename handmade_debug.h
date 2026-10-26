#if !defined(HANDMADE_DEBUG_H)

#define DEBUG_SNAPSHOT_COUNT 120
struct debug_counter_snapshot
{
  uint32 HitCount;
  uint64 CycleCount;
};

struct debug_counter_state
{
  char *FileName;
  char *BlockName;

  uint32 LineNumber;
};

#define MAX_REGION_PER_FRAME 4096
struct debug_frame_region
{
  debug_record *Record;
  uint32 LaneIndex;
  uint64 CycleCount;
  real32 MinT;
  real32 MaxT;
};

struct debug_frame
{
  uint64 BeginClock;
  uint64 EndClock;
  real32 WallSecondsElapsed;
  uint32 RegionCount;
  debug_frame_region *Regions;
};

struct open_debug_block
{
  uint32 StartingFrameIndex;
  debug_event *OpeningEvent;
  open_debug_block *Parent;
  open_debug_block *NextFree;
};

struct debug_theard
{
  uint32 TheardID;
  uint32 LaneIndex;
  open_debug_block *FirstOpenBlock;
  
  debug_theard *Next;
};

struct debug_state
{
  bool32 Paused;
  bool32 Initialized;
  memory_arena CollateArena;
  temporary_memory CollateTemp;
  
  uint32 FrameCount;
  uint32 FrameBarLaneCount;
  real32 FrameBarScale;
  
  debug_frame *Frames;
  debug_theard *FirstTheard;
  open_debug_block *FirstFreeBlock;
};

struct render_group;
struct game_assets;
global_variable render_group *DEBUGRenderGroup;
internal void DEBUGReset(game_assets *Assets, uint32 Width, uint32 Height);
internal void DEBUGOverlay(game_memory *Memory, game_input *Input);

#define HANDMADE_DEBUG_H
#endif
