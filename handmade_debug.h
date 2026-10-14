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
  
  debug_counter_snapshot Snapshots[DEBUG_SNAPSHOT_COUNT];
};

struct debug_state
{
  uint32 SnapshotIndex;
  uint32 CounterCount;
  debug_counter_state CounterState[512];
};

struct render_group;
struct game_assets;
global_variable render_group *DEBUGRenderGroup;
internal void DEBUGReset(game_assets *Assets, uint32 Width, uint32 Height);
internal void DEBUGOverlay(game_memory *Memory);

#define HANDMADE_DEBUG_H
#endif
