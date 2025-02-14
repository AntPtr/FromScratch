#if !defined(HANDMADE_WORLD_H)
struct world_difference
{
  v2 dXY;
  real32 dZ;
};

struct world_position
{
  int32 ChunkX;
  int32 ChunkY;
  int32 ChunkZ;

  v2 Offset_;
};

/*struct tile_chunk_position
{
  int32 TileChunkX;
  int32 TileChunkY;
  int32 TileChunkZ;

  int32 RelTileX;
  int32 RelTileY;
  };*/

struct world_entity_block
{
  uint32 LowEntityCount;
  uint32 LowEntityIndex[16];
  bool32 Full;

  world_entity_block *NextBlock;
};

struct world_chunk
{
  int32 ChunkX;
  int32 ChunkY;
  int32 ChunkZ;
  
  uint32 *Tiles;

  world_chunk *NextInHash;

  world_entity_block FirstBlock;
};

struct world
{
  real32 ChunkSideInMeter;
  real32 TileSideInMeter;
  
  int32 ChunkDim;  
  int32 ChunkShift;
  int32 ChunkMask;

  world_entity_block *FirstFree;
  
  world_chunk ChunksHash[4096];
};
#define HANDMADE_WORLD_H
#endif
