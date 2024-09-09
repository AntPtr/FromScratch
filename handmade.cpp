#include "handmade.h"
#include "handmade_tile.cpp"

internal void SomeGradient(game_offscreen_buffer *Buffer ,int XOffset,int YOffset)
{
  uint8 *Row = (uint8 *)Buffer->Memory;
  uint32 *Pixel = (uint32 *)Row;

  for(int Y = 0; Y < Buffer->Height; ++Y)
  {
    Pixel = (uint32 *)Row;
    for(int X = 0; X < Buffer->Width; ++X)
    {
      uint8 Blue =(uint8)(X + XOffset);
      uint8 Green =(uint8)(Y + YOffset);

      *Pixel++ = ((Green << 8)| Blue);
    }
     Row += Buffer->Pitch;
  }
}

internal void DrawRectangle(game_offscreen_buffer *Buffer, real32 RealMinX, real32 RealMaxX, real32 RealMinY, real32 RealMaxY, real32 R, real32 G, real32 B)
{
  int32 MinX = RoundReal32ToInt32(RealMinX);
  int32 MaxX = RoundReal32ToInt32(RealMaxX);
  int32 MinY = RoundReal32ToInt32(RealMinY);
  int32 MaxY = RoundReal32ToInt32(RealMaxY);

  if(MinX < 0)
  {
    MinX = 0;
  }
  if(MaxX > Buffer->Width)
  {
    MaxX = Buffer->Width;
  }
  if(MinY < 0)
  {
    MinY = 0;
  }
  if(MaxY > Buffer->Height)
  {
    MaxY = Buffer->Height;
  }
 
  uint32 Color = (uint32)(RoundReal32ToUInt32(255 * R) << 16 | RoundReal32ToUInt32(255 * G) << 8 | RoundReal32ToUInt32(255 *B) << 0);
    
  uint8* Row = ((uint8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch); 

  for(int Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = MinX; X < MaxX; ++X)
    {
      *Pixel++ = Color;
    }
    Row += Buffer->Pitch;
  }
}

internal void RenderPlayer(game_offscreen_buffer *Buffer, int PlayerX, int PlayerY)
{
  uint32 Color = 0xFFFFFFFF;
  uint8 *EndOfBuffer = ((uint8 *)Buffer->Memory + Buffer->Height * Buffer->Pitch);
  int Left = PlayerX;
  int Rigth = PlayerX + 10;
  int Top = PlayerY;
  int Bottom = PlayerY + 10;
 
  for(int X = Left; X < Rigth; ++X)
  {
    uint8* Pixel = ((uint8 *)Buffer->Memory + X * Buffer->BytesPerPixel + Top * Buffer->Pitch); 
    for(int Y = Top; Y < Bottom; ++Y)
    {
      if(Pixel >= Buffer->Memory && Pixel < EndOfBuffer)
      {
	*(uint32 *)Pixel = Color;
        Pixel += Buffer->Pitch;
      }
    }
  }
}

void GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 *SampleOut = SoundBuffer->Samples;
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    for(uint16 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
#if 0
      real32 SineValue = sinf(GameState->tSine);
      int16 SampleValue =(int16)(SineValue*ToneVolume);
#else
      int16 SampleValue = 0;
#endif
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
#if 0
      GameState->tSine +=  2.0f*Pi32*1.0f/(real32)WavePeriod;
      if(GameState->tSine > 2.0 * Pi32)
      {
	GameState->tSine -= 2.0 * Pi32;
      }
#endif
    }
}

internal bool32 IsTileChunkPointEmpty (tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
  bool32 Empty = false;
  
  if(TileChunk)
  {         
    uint32 TileChunkValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    Empty = (TileChunkValue == 0);
  }
  return Empty;
}

#if defined __cplusplus
extern "C"
#endif
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
  Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));

  #define TILE_MAP_COUNT_X 256
  #define TILE_MAP_COUNT_Y 256

  uint32 TempTiles[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  
  tile_map TileMap;
  TileMap.TileSideInMeter = 1.40f;
  TileMap.TileSideInPixel = 60;
  TileMap.MetersToPixels = (real32)TileMap.TileSideInPixel / (real32)TileMap.TileSideInMeter;
  TileMap.TileChunkCountX = 1;
  TileMap.TileChunkCountY = 1;
  TileMap.ChunkDim = 256;

  real32 LowerLeftX = -(real32)(TileMap.TileSideInPixel / 2);
  real32 LowerLeftY = (real32)Buffer->Height;
  tile_chunk TileChunk;
  TileChunk.Tiles = (uint32 *)TempTiles;
  TileMap.TileChunks = &TileChunk;
  TileMap.ChunkShift = 8;
  TileMap.ChunkMask = 0xFF;
  
  game_state *GameState = (game_state *)Memory->PermanentStorage;
  
  if(!Memory->IsInitialized)
  { 
    /*GameState->ToneHz = 256;
    GameState->tSine = 0.0f;
    GameState->XOffset = 0;
    GameState->YOffset = 0;*/
    GameState->PlayerP.AbsTileX = 3;
    GameState->PlayerP.AbsTileY = 3;
    GameState->PlayerP.TileRelX = 5.0f;
    GameState->PlayerP.TileRelY = 5.0f;

    Memory->IsInitialized = true;
  }
  real32 ScreenCenterY = (real32)Buffer->Height * 0.5f;
  real32 ScreenCenterX = (real32)Buffer->Width * 0.5f;
  real32 PlayerHeight = 1.4f;
  real32 PlayerWidth = 0.75f * PlayerHeight;
  real32 PlayerLeft = ScreenCenterX  - (0.5f * PlayerWidth * TileMap.MetersToPixels);
  real32 PlayerTop = ScreenCenterY  - PlayerHeight  * TileMap.MetersToPixels;
  
  for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
  {
    game_controller_input *Controller = GetController(Input, ControllerIndex);
    
    if(Controller -> IsAnalog)
    {
      //GameState->XOffset += (int)(4.0f*Controller->AvarageStickX);
    }
    
    else
    {
      real32 dPlayerX = 0.0f;
      real32 dPlayerY = 0.0f;
      real32 Velocity = 2.0f;
      
      if(Controller->MoveLeft.EndedDown)
      {
	dPlayerX = -1.0f;
      }
      if(Controller->MoveRight.EndedDown)
      {
	dPlayerX = 1.0f;
      }
      if(Controller->MoveUp.EndedDown)
      {
        dPlayerY = 1.0f;
      }
      if(Controller->MoveDown.EndedDown)
      {
	dPlayerY = -1.0f;
      }
      if(Controller->ActionDown.EndedDown)
      {
	Velocity = 10.0f;
      }

      dPlayerX *= Velocity;
      dPlayerY *= Velocity;

      tile_map_position NewPlayerP = GameState->PlayerP;
      NewPlayerP.TileRelX += Input->dtForFrame * dPlayerX;
      NewPlayerP.TileRelY += Input->dtForFrame * dPlayerY;
      NewPlayerP = ReCanonicalizePosition(&TileMap, NewPlayerP);

      tile_map_position PlayerL = NewPlayerP;
      PlayerL.TileRelX -= 0.5f * PlayerWidth;
      PlayerL = ReCanonicalizePosition(&TileMap, PlayerL);
      
      tile_map_position PlayerR = NewPlayerP;
      PlayerR.TileRelX += 0.5f * PlayerWidth;
      PlayerR = ReCanonicalizePosition(&TileMap, PlayerR);

      if(IsTileMapPointEmpty(&TileMap, NewPlayerP) && IsTileMapPointEmpty(&TileMap, PlayerL) && IsTileMapPointEmpty(&TileMap, PlayerR))
      {
	GameState->PlayerP = NewPlayerP;
      }
      
    }
  }
  
  for(int32 RelRow = -10; RelRow < 10; ++RelRow)
  {
    for(int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
    {
      uint32 Column = GameState->PlayerP.AbsTileX + RelColumn;
      uint32 Row = GameState->PlayerP.AbsTileY + RelRow;
      uint32 TileID = GetTileValue(&TileMap, Column, Row);
      real32 Grey = 0.5f;
      if(TileID == 1)
      {
	Grey = 1.0f;
      }
      real32 CenX = ScreenCenterX - TileMap.MetersToPixels * GameState->PlayerP.TileRelX + ((real32)(TileMap.TileSideInPixel) * RelColumn);
      real32 CenY = ScreenCenterY + TileMap.MetersToPixels * GameState->PlayerP.TileRelY - ((real32)(TileMap.TileSideInPixel) * RelRow);
      real32 MinX = CenX - 0.5f * TileMap.TileSideInPixel;
      real32 MinY = CenY - 0.5f * TileMap.TileSideInPixel;
      real32 MaxX = CenX + 0.5f * TileMap.TileSideInPixel;
      real32 MaxY = CenY + 0.5f * TileMap.TileSideInPixel;
      if(Row == GameState->PlayerP.AbsTileY && Column == GameState->PlayerP.AbsTileX)
      {
	Grey = 0.0f;
      }

      DrawRectangle(Buffer, MinX, MaxX, MinY, MaxY, Grey, Grey, Grey);

    }
  }

  DrawRectangle(Buffer, PlayerLeft, PlayerLeft + PlayerWidth * TileMap.MetersToPixels, PlayerTop, PlayerTop + PlayerHeight  * TileMap.MetersToPixels, 1.0f, 1.0f, 0.0f);
  //DrawRectangle(Buffer, 0.0f, (real32)Buffer->Width, 0.0f, (real32)Buffer->Height, 1.0f, 0.0f, 1.0f);
  //DrawRectangle(Buffer, 10.0f, 30.0f, 10.0f, 30.f, 1.0f, 1.0f, 0.0f);

}


#if defined __cplusplus
extern "C"
#endif
GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state *GameState = (game_state*)Memory->PermanentStorage;
  GameOutputSound(GameState, SoundBuffer, 400);
}



