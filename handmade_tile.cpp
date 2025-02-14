

/*
inline bool32 IsTileMapPointEmpty(tile_map *Tile_Map, tile_map_position CanPos)
{
  uint32 TileChunkValue = GetTileValue(Tile_Map, CanPos.AbsTileX, CanPos.AbsTileY, CanPos.AbsTileZ);
  bool32 Empty = IsTileValueEmpty(TileChunkValue);

   
  return (Empty);
}

internal void SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue)
{
  tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
  tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ, Arena);

 SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}

inline uint32 GetTileValue(tile_map *TileMap, tile_map_position Pos)
{
  
  uint32 TileChunkValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
  
  return TileChunkValue;
}

inline uint32 GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  
  tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
  tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
  uint32 TileChunkValue = GetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
  
  return TileChunkValue;
}

inline int32 GetTileValueUnchecked(tile_map *Tile_Map, tile_chunk *TileChunk, int32 TileX, int32 TileY)
{
  Assert(TileChunk);
  Assert((TileX < Tile_Map->ChunkDim) && (TileY < Tile_Map->ChunkDim));
  
  int32 TileChunkValue = TileChunk->Tiles[TileY * Tile_Map->ChunkDim + TileX];
  return (TileChunkValue);
}

inline void SetTileValueUnchecked(tile_map *Tile_Map, tile_chunk *TileChunk, int32 TileX, int32 TileY, uint32 TileValue)
{
  Assert(TileChunk);
  Assert((TileX < Tile_Map->ChunkDim) && (TileY < Tile_Map->ChunkDim));
  
  TileChunk->Tiles[TileY * Tile_Map->ChunkDim + TileX] = TileValue;
}

inline uint32 GetTileValue (tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
  uint32 TileValue = 0;
  
  if(TileChunk && TileChunk->Tiles)
  {         
    TileValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
  }
  return TileValue;
}

inline void SetTileValue (tile_map *Tile_Map, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
  
  if(TileChunk && TileChunk->Tiles)
  {         
    SetTileValueUnchecked(Tile_Map, TileChunk, TestTileX, TestTileY, TileValue);
  }

}*/

