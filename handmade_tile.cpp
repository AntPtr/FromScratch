
inline void ReCanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *TileRel)
{
  int32 Offset = RoundReal32ToInt32(*TileRel / TileMap->TileSideInMeter);
  
  *Tile += Offset; 

  *TileRel -= Offset * TileMap->TileSideInMeter;

  Assert(*TileRel >= -0.5f * TileMap->TileSideInMeter);
  Assert(*TileRel < 0.5f * TileMap->TileSideInMeter);
  
}

inline tile_map_position ReCanonicalizePosition(tile_map *Tile_Map, tile_map_position Pos)
{
  tile_map_position Result = Pos;
  ReCanonicalizeCoord(Tile_Map, &Result.AbsTileX, &Result.TileRelX);
  ReCanonicalizeCoord(Tile_Map, &Result.AbsTileY, &Result.TileRelY);

  return Result;
}

inline tile_chunk *GetTileChunk(tile_map *Tile_Map, int32 TileChunkX, int32 TileChunkY)
{
  tile_chunk *TileChunk = 0;
    
  if ((TileChunkX >= 0) && (TileChunkX < Tile_Map->TileChunkCountX) &&
      (TileChunkY >= 0) && (TileChunkY < Tile_Map->TileChunkCountY))
  {
    TileChunk = &Tile_Map->TileChunks[TileChunkY * Tile_Map->TileChunkCountX + TileChunkX];
  }
    
  return (TileChunk);
}

inline int32 GetTileValueUnchecked(tile_map *Tile_Map, tile_chunk *TileChunk, uint32 TileX, uint32 TileY)
{
  Assert(TileChunk);
  Assert((TileX < Tile_Map->ChunkDim) && (TileY < Tile_Map->ChunkDim));
  
  int32 TileChunkValue = TileChunk->Tiles[TileY * Tile_Map->ChunkDim + TileX];
  return (TileChunkValue);
}

inline uint32 GetTileValue (tile_map *Tile_Map, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
  uint32 TileValue = 0;
  
  if(TileChunk)
  {         
    TileValue = GetTileValueUnchecked(Tile_Map, TileChunk, TestTileX, TestTileY);
  }
  return TileValue;
}

inline tile_chunk_position GetChunkPositionFor(tile_map *Tile_Map, uint32 AbsTileX, uint32 AbsTileY)
{
  tile_chunk_position Result;
  Result.TileChunkX = AbsTileX >> Tile_Map->ChunkShift;
  Result.TileChunkY = AbsTileY >> Tile_Map->ChunkShift;
  Result.RelTileX = AbsTileX & Tile_Map->ChunkMask;
  Result.RelTileY = AbsTileY & Tile_Map->ChunkMask;

  return Result;
}

inline uint32 GetTileValue(tile_map *Tile_Map, uint32 AbsTileX, uint32 AbsTileY)
{
  bool32 Empty = false;
  
  tile_chunk_position ChunkPos = GetChunkPositionFor(Tile_Map, AbsTileX, AbsTileY);
  tile_chunk *TileMap = GetTileChunk(Tile_Map, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
  uint32 TileChunkValue = GetTileValue(Tile_Map, TileMap, ChunkPos.RelTileX, ChunkPos.RelTileY);
  
  return (TileChunkValue);
}

inline bool32 IsTileMapPointEmpty(tile_map *Tile_Map, tile_map_position CanPos)
{
  uint32 TileChunkValue = GetTileValue(Tile_Map, CanPos.AbsTileX, CanPos.AbsTileY);
  bool32 Empty = (TileChunkValue == 0);
   
  return (Empty);
}
