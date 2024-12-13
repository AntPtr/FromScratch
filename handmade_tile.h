#if !defined(HANDMADE_TILE_H)
struct tile_map_difference
{
  v2 dXY;
  real32 dZ;
};

struct tile_map_position
{
  int32 AbsTileX;
  int32 AbsTileY;
  int32 AbsTileZ;

  v2 Offset_;
};

struct tile_chunk_position
{
  int32 TileChunkX;
  int32 TileChunkY;
  int32 TileChunkZ;

  int32 RelTileX;
  int32 RelTileY;
};

struct tile_chunk
{
  int32 TileChunkX;
  int32 TileChunkY;
  int32 TileChunkZ;
  
  uint32 *Tiles;

  tile_chunk *NextInHash;
};

struct tile_map
{
  real32 TileSideInMeter;

  int32 ChunkDim;  
  int32 ChunkShift;
  int32 ChunkMask;
  
  tile_chunk TileChunksHash[4096];
};

#define HANDMADE_TILE_H
#endif
