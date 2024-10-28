
inline void ReCanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *TileRel)
{
  int32 Offset = RoundReal32ToInt32(*TileRel / TileMap->TileSideInMeter);
  
  *Tile += Offset; 

  *TileRel -= Offset * TileMap->TileSideInMeter;

  Assert(*TileRel > -0.5001f * TileMap->TileSideInMeter);
  Assert(*TileRel < 0.5001f * TileMap->TileSideInMeter);
  
}

inline tile_map_position ReCanonicalizePosition(tile_map *Tile_Map, tile_map_position Pos)
{
  tile_map_position Result = Pos;
  ReCanonicalizeCoord(Tile_Map, &Result.AbsTileX, &Result.Offset_.X);
  ReCanonicalizeCoord(Tile_Map, &Result.AbsTileY, &Result.Offset_.Y);

  return Result;
}

inline tile_chunk *GetTileChunk(tile_map *TileMap, uint32 TileChunkX, uint32 TileChunkY, uint32 TileChunkZ)
{
  tile_chunk *TileChunk = 0;
    
  if ((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
      (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY) &&
      (TileChunkZ >= 0) && (TileChunkZ < TileMap->TileChunkCountZ))
  {
    TileChunk = &TileMap->TileChunks[TileChunkZ * TileMap->TileChunkCountY * TileMap->TileChunkCountX  + TileChunkY * TileMap->TileChunkCountX + TileChunkX];
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

inline void SetTileValueUnchecked(tile_map *Tile_Map, tile_chunk *TileChunk, uint32 TileX, uint32 TileY, uint32 TileValue)
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

}


inline tile_chunk_position GetChunkPositionFor(tile_map *Tile_Map, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  tile_chunk_position Result;
  Result.TileChunkX = AbsTileX >> Tile_Map->ChunkShift;
  Result.TileChunkY = AbsTileY >> Tile_Map->ChunkShift;
  Result.TileChunkZ = AbsTileZ;

  Result.RelTileX = AbsTileX & Tile_Map->ChunkMask;
  Result.RelTileY = AbsTileY & Tile_Map->ChunkMask;

  return Result;
}

inline uint32 GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  
  tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
  tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
  uint32 TileChunkValue = GetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
  
  return TileChunkValue;
}

inline uint32 GetTileValue(tile_map *TileMap, tile_map_position Pos)
{
  
  uint32 TileChunkValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
  
  return TileChunkValue;
}

inline bool32 IsTileValueEmpty(uint32 TileValue)
{
  bool32 Empty = (TileValue == 1) ||
                 (TileValue == 3) ||
                 (TileValue == 4);
   
  return (Empty);
}

inline bool32 IsTileMapPointEmpty(tile_map *Tile_Map, tile_map_position CanPos)
{
  uint32 TileChunkValue = GetTileValue(Tile_Map, CanPos.AbsTileX, CanPos.AbsTileY, CanPos.AbsTileZ);
  bool32 Empty = IsTileValueEmpty(TileChunkValue);

   
  return (Empty);
}

internal void SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue)
{
  tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
  tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);

  if(!TileChunk->Tiles)
  {
    uint32 TileCount = TileMap->ChunkDim * TileMap->ChunkDim;
    TileChunk->Tiles = PushArray(Arena, TileCount, uint32);
    for(uint32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
    {
      TileChunk->Tiles[TileIndex] = 1;
    }
  }

 SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}


inline bool32 AreOnSameTile(tile_map_position *PosA, tile_map_position *PosB)
{
  bool32 Result = ((PosA->AbsTileX == PosB->AbsTileX) &&
                  (PosA->AbsTileY == PosB->AbsTileY) &&
		  (PosA->AbsTileZ == PosB->AbsTileZ));
  return Result;
}

inline tile_map_difference Subtract(tile_map *TileMap, tile_map_position *A, tile_map_position * B)
{
  tile_map_difference Result;

  v2 dTileXY = {(real32)(A->AbsTileX) - (real32)(B->AbsTileX), (real32)(A->AbsTileY) - (real32)(B->AbsTileY)};
  
  real32 dTileZ = (real32)(A->AbsTileZ) - (real32)(B->AbsTileZ);

  Result.dXY = TileMap->TileSideInMeter *dTileXY + (A->Offset_ - B->Offset_);
  Result.dZ = TileMap->TileSideInMeter *dTileZ;

  return Result;
}

inline tile_map_position CenteredTilePoint(uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  tile_map_position Result = {};
  Result.AbsTileX = AbsTileX;
  Result.AbsTileY = AbsTileY;
  Result.AbsTileZ = AbsTileZ;

  return Result;
}

inline tile_map_position Offset(tile_map *TileMap, tile_map_position P, v2 Offset)
{
  P.Offset_ += Offset;
  P = ReCanonicalizePosition(TileMap, P);
  return P;
}
