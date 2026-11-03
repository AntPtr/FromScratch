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
  uint16 LaneIndex;
  uint16 ColorIndex;
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
  debug_record *Source;
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

  real32 AtY;
  real32 LeftEdge;
  real32 FontScale;
  font_id FontID;
  real32 GlobalWidth;
  real32 GlobalHeight;

  memory_arena CollateArena;
  temporary_memory CollateTemp;

  debug_frame *CollationFrame;
  uint32 CollationArrayIndex;
  uint32 FrameCount;
  uint32 FrameBarLaneCount;
  real32 FrameBarScale;
  debug_frame *Frames;

  debug_record *ScopedRecord;
  
  debug_theard *FirstTheard;
  open_debug_block *FirstFreeBlock;
};

struct render_group;
struct game_assets;
global_variable render_group *DEBUGRenderGroup;
internal void DEBUGReset(game_assets *Assets, uint32 Width, uint32 Height);
internal void DEBUGOverlay(game_input *Input);
internal void RefreshCollation(debug_state *DebugState);

#define HANDMADE_DEBUG_H
#endif
