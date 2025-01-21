#define WORLD_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define TILE_CHUNK_UNITIALIZED INT32_MAX
#define TILES_PER_CHUNK 16

inline bool32 IsCanonical(world *World, real32 TileRel)
{
  bool32 Result = ((TileRel >= -0.5f * World->ChunkSideInMeter) &&
		   (TileRel <= 0.5f * World->ChunkSideInMeter));
  return Result;
}

inline bool32 IsCanonical(world *World, v2 Offset)
{
  bool32 Result = (IsCanonical(World, Offset.X) && IsCanonical(World, Offset.Y));
  return Result;
}

inline void ReCanonicalizeCoord(world *World, int32 *Tile, real32 *TileRel)
{
  int32 Offset = RoundReal32ToInt32(*TileRel / World->ChunkSideInMeter);
  
  *Tile += Offset; 

  *TileRel -= Offset * World->ChunkSideInMeter;

  Assert(IsCanonical(World, *TileRel));
}

inline world_position MapIntoChunkSpace(world *World, world_position BasePos, v2 Offset)
{
  world_position Result = BasePos;
  Result.Offset_ += Offset;
  ReCanonicalizeCoord(World, &Result.ChunkX, &Result.Offset_.X);
  ReCanonicalizeCoord(World, &Result.ChunkY, &Result.Offset_.Y);

  return Result;
}

inline world_position ChunkPositionFromTilePosition(world *World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
  world_position Result = {};

  Result.ChunkX = AbsTileX / TILES_PER_CHUNK;
  Result.ChunkY = AbsTileY / TILES_PER_CHUNK;
  Result.ChunkZ = AbsTileZ / TILES_PER_CHUNK;

  Result.Offset_.X = (real32)(AbsTileX - (Result.ChunkX*TILES_PER_CHUNK)) * World->TileSideInMeter;
  Result.Offset_.Y = (real32)(AbsTileY - (Result.ChunkY*TILES_PER_CHUNK)) * World->TileSideInMeter;

  return Result;
}

inline world_chunk *GetWorldChunk(world *World, int32 ChunkX, int32 ChunkY, int32 ChunkZ, memory_arena *Arena = 0)
{
  world_chunk *WorldChunk = 0;

  Assert(ChunkX > -WORLD_CHUNK_SAFE_MARGIN);
  Assert(ChunkY > -WORLD_CHUNK_SAFE_MARGIN);
  Assert(ChunkZ > -WORLD_CHUNK_SAFE_MARGIN);
  Assert(ChunkX < WORLD_CHUNK_SAFE_MARGIN);
  Assert(ChunkY < WORLD_CHUNK_SAFE_MARGIN);
  Assert(ChunkZ < WORLD_CHUNK_SAFE_MARGIN);

  uint32 HashValue = 19*ChunkX + 7*ChunkY + 3*ChunkZ;
  
  uint32 HashSlot = HashValue & (ArrayCount(World->ChunksHash) - 1);
  world_chunk *Chunk = HashSlot + World->ChunksHash;
  do
  {
    if((ChunkX == Chunk->ChunkX) &&
       (ChunkY == Chunk->ChunkY) &&
       (ChunkZ == Chunk->ChunkZ))
    {
      break;
    }
    if(Arena && (Chunk->ChunkX != TILE_CHUNK_UNITIALIZED)  && (!Chunk->NextInHash))
    {
      Chunk->NextInHash = PushStruct(Arena, world_chunk);
      Chunk->ChunkX = 0;
      Chunk = Chunk->NextInHash;
    }
    if(Arena && Chunk->ChunkX == TILE_CHUNK_UNITIALIZED)
    {
      Chunk->ChunkX = ChunkX;
      Chunk->ChunkY = ChunkY;
      Chunk->ChunkZ = ChunkZ;     

      Chunk->NextInHash = 0;
      
      break;
    }
    Chunk = Chunk->NextInHash;
  }
  while(Chunk);
  
  return Chunk;
}

#if 0
inline world_chunk_position GetChunkPositionFor(world *World, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
  world_chunk_position Result;
  Result.ChunkX = AbsTileX >> World->ChunkShift;
  Result.ChunkY = AbsTileY >> World->ChunkShift;
  Result.ChunkZ = AbsTileZ;

  Result.RelTileX = AbsTileX & World->ChunkMask;
  Result.RelTileY = AbsTileY & World->ChunkMask;

  return Result;
}
#endif

inline bool32 IsTileValueEmpty(uint32 TileValue)
{
  bool32 Empty = (TileValue == 1) ||
                 (TileValue == 3) ||
                 (TileValue == 4);
   
  return (Empty);
}


inline world_difference Subtract(world *World, world_position *A, world_position * B)
{
  world_difference Result;

  v2 dTileXY = {(real32)(A->ChunkX) - (real32)(B->ChunkX), (real32)(A->ChunkY) - (real32)(B->ChunkY)};
  
  real32 dTileZ = (real32)(A->ChunkZ) - (real32)(B->ChunkZ);

  Result.dXY = World->ChunkSideInMeter *dTileXY + (A->Offset_ - B->Offset_);
  Result.dZ = World->ChunkSideInMeter *dTileZ;

  return Result;
}

inline world_position CenteredChunkPoint(uint32 ChunkX, uint32 ChunkY, uint32 ChunkZ)
{
  world_position Result = {};
  Result.ChunkX = ChunkX;
  Result.ChunkY = ChunkY;
  Result.ChunkZ = ChunkZ;

  return Result;
}

internal void InitializeWorld(world *World, real32 TileSideInMeters)
{
  World->TileSideInMeter = TileSideInMeters;
  World->ChunkSideInMeter = (real32)TILES_PER_CHUNK*TileSideInMeters;
  World->FirstFree = 0;
  for(uint32 ChunkIndex = 0; ChunkIndex < ArrayCount(World->ChunksHash); ++ChunkIndex)
  {
    World->ChunksHash[ChunkIndex].ChunkX = TILE_CHUNK_UNITIALIZED;
    World->ChunksHash[ChunkIndex].FirstBlock.LowEntityCount = 0;
  }
}

inline bool32 AreInTheSameChunk(world *World, world_position *PosA, world_position *PosB)
{
  Assert(IsCanonical(World, PosA->Offset_));
  Assert(IsCanonical(World, PosB->Offset_));

  bool32 Result = ((PosA->ChunkX == PosB->ChunkX) &&
                  (PosA->ChunkY == PosB->ChunkY) &&
		  (PosA->ChunkZ == PosB->ChunkZ));
  return Result;
}

inline void ChangeEntityLocation(memory_arena *Arena, world *World, uint32 LowEntityIndex, world_position *OldP, world_position *NewP)
{
  if(OldP && AreInTheSameChunk(World, OldP, NewP))
  {
    //Leave where it is
  }
  else
  {
    if(OldP)
    {
      //Extract the OldP entity block
      world_chunk *Chunk = GetWorldChunk(World, OldP->ChunkX, OldP->ChunkY, OldP->ChunkZ);
      Assert(Chunk);
      if(Chunk)
      {
	bool32 NotFound = true;
	world_entity_block *FirstBlock = &Chunk->FirstBlock;
	for(world_entity_block *Block = FirstBlock; Block && NotFound; Block = Block->NextBlock)
	{
	  for(uint32 Index = 0; (Index < Block->LowEntityCount ) && NotFound; ++Index)
	  {
	    if(Block->LowEntityIndex[Index] == LowEntityIndex)
	    {
	      Block->LowEntityIndex[Index] = FirstBlock->LowEntityIndex[--FirstBlock->LowEntityCount];
	      if(FirstBlock->LowEntityCount == 0)
	      {
		if(FirstBlock->NextBlock)
		{
		  world_entity_block *NextBlock = FirstBlock->NextBlock;
		  *FirstBlock = *NextBlock;
		  
		  NextBlock->NextBlock = World->FirstFree;
		  World->FirstFree = NextBlock;
		}
	      }
	      NotFound = false;
	    }
	  }
	}
      }
    }
    //Insert the enitity in the new block
    world_chunk *Chunk = GetWorldChunk(World, NewP->ChunkX, NewP->ChunkY, NewP->ChunkZ, Arena);
    world_entity_block *Block = &Chunk->FirstBlock;
    if(Block->LowEntityCount == ArrayCount(Block->LowEntityIndex))
    {
      //We are out of space, get a new block!
      world_entity_block *OldBlock = World->FirstFree;
      if(OldBlock)
      {
	World->FirstFree = OldBlock->NextBlock;
      }
      else
      {
	OldBlock = PushStruct(Arena, world_entity_block);
      }
      *OldBlock = *Block;
      Block->NextBlock = OldBlock;
      Block->LowEntityCount = 0;
    }
    Assert(Block->LowEntityCount < ArrayCount(Block->LowEntityIndex));
    Block->LowEntityIndex[Block->LowEntityCount++] = LowEntityIndex;
  }
}
