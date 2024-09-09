#if !defined(HANDMADE_TILE_H)

struct tile_map_position
{
  uint32 AbsTileX;
  uint32 AbsTileY;

  real32 TileRelX;
  real32 TileRelY;
};

struct tile_chunk_position
{
  uint32 TileChunkX;
  uint32 TileChunkY;

  uint32 RelTileX;
  uint32 RelTileY;
};

struct tile_chunk
{    
  uint32 *Tiles;
};

struct tile_map
{
  real32 TileSideInMeter;
  int32 TileSideInPixel;
  real32 MetersToPixels;

  int32 TileChunkCountX;
  int32 TileChunkCountY; 

  uint32 ChunkDim;
  
  uint32 ChunkShift;
  uint32 ChunkMask;
  
  tile_chunk *TileChunks;
};

#define HANDMADE_TILE_H
#endif
