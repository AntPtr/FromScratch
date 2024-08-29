/*
  ================================================

  Codice educativo scritto da Antonio Pietroluongo
  Data: 15/05/2024
  Versione: 0.1
  
  ================================================
*/
#include "handmade.h"
#include "handmade_intrisic.h"
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

inline tile_map *GetTileMap(world *World, int32 TileMapX, int32 TileMapY)
{
  tile_map *TileMap = 0;
    
  if ((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
      (TileMapY >= 0) && (TileMapY < World->TileMapCountY))
  {
    TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
  }
    
  return (TileMap);
}

inline int32 GetTileValueUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY)
{
  Assert(TileMap);
  Assert((TileX >= 0) && (TileX < World->CountX) && (TileY >= 0) && (TileY < World->CountY));
  
  int32 TileMapValue = TileMap->Tiles[TileY * World->CountX + TileX];
  return (TileMapValue);
}

internal bool32 IsTileMapPointEmpty (world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY)
{
  bool32 Empty = false;
  if(TileMap)
  {         
    if((TestTileX >= 0) && (TestTileX < World->CountX) && (TestTileY >= 0) && (TestTileY < World->CountY))
    {
      uint32 TileMapValue = GetTileValueUnchecked(World, TileMap, TestTileX, TestTileY);
      Empty = (TileMapValue == 0);
    }
  }
  return Empty;
}

inline canonical_position GetCanonicalPosition(world *World, raw_position Pos)
{
  canonical_position Result;

  Result.TileMapX = Pos.TileMapX;
  Result.TileMapY = Pos.TileMapY;

  real32 X = Pos.X - World->UpperLeftX;
  real32 Y = Pos.Y - World->UpperLeftY;
  
  Result.TileX = FloorReal32ToInt32(X / World->TileWidth);
  Result.TileY = FloorReal32ToInt32(Y / World->TileHeight);

  Result.TileRelX = X - Result.TileX * World->TileWidth;
  Result.TileRelY = Y - Result.TileY * World->TileHeight;

  Assert(Result.TileRelX >= 0);
  Assert(Result.TileRelY >= 0);
  Assert(Result.TileRelX < World->TileWidth);
  Assert(Result.TileRelY < World->TileHeight);

  if(Result.TileX < 0)
  {
    Result.TileX = World->CountX + Result.TileX;
    --Result.TileMapX;
  }

  if(Result.TileY < 0)
  {
    Result.TileY = World->CountY + Result.TileY;
    --Result.TileMapY;
  }

  if(Result.TileX >= World->CountX)
  {
    Result.TileX = Result.TileX - World->CountX;
    ++Result.TileMapX;
  }

  if(Result.TileY >= World->CountY)
  {
    Result.TileY = Result.TileY - World->CountY;
    ++Result.TileMapY;
  }

  return Result;
}

internal bool32 IsWorldPointEmpty(world *World, raw_position TestPos)
{
  bool32 Empty = false;
  canonical_position CanPos = GetCanonicalPosition(World, TestPos);
  tile_map *TileMap = GetTileMap(World, CanPos.TileMapX, CanPos.TileMapY);
  Empty = IsTileMapPointEmpty(World, TileMap, CanPos.TileX, CanPos.TileY);
   
  return (Empty);
}

#if defined __cplusplus
extern "C"
#endif
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
  Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));

  #define TILE_MAP_COUNT_X 17
  #define TILE_MAP_COUNT_Y 9

  uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 1, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 1, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0, 0},
      {1, 1, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 0, 0, 0,  0, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  
  uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
  {
      {1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 1},
      {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1},
  };
  

  tile_map TileMaps[2][2];

  TileMaps[0][0].Tiles = (uint32 *)Tiles00;
  TileMaps[0][1].Tiles = (uint32 *)Tiles10;
  TileMaps[1][0].Tiles = (uint32 *)Tiles01;
  TileMaps[1][1].Tiles = (uint32 *)Tiles11;

  world World;
  World.TileMapCountX = 2;
  World.TileMapCountY = 2;
  World.CountX = TILE_MAP_COUNT_X;
  World.CountY = TILE_MAP_COUNT_Y;
  World.TileWidth = 60;
  World.TileHeight = 60;
  World.UpperLeftX = -World.TileWidth / 2;
  World.UpperLeftY = 0;
  World.TileMaps = (tile_map *)TileMaps;
  
  game_state *GameState = (game_state *)Memory->PermanentStorage;
  
  if(!Memory->IsInitialized)
  { 
    /*GameState->ToneHz = 256;
    GameState->tSine = 0.0f;
    GameState->XOffset = 0;
    GameState->YOffset = 0;*/
    GameState->PlayerX = 190;
    GameState->PlayerY = 150;
    
    Memory->IsInitialized = true;
  }
  real32 PlayerWidth = World.TileWidth  * 0.75f;
  real32 PlayerHeight = World.TileHeight;
  real32 PlayerLeft = GameState->PlayerX - (0.5f * PlayerWidth);
  real32 PlayerTop = GameState->PlayerY - PlayerHeight;
  tile_map *TileMap = GetTileMap(&World, GameState->PlayerTileMapX, GameState->PlayerTileMapY);
  Assert(TileMap);

  
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
        dPlayerY = -1.0f;
      }
      if(Controller->MoveDown.EndedDown)
      {
	dPlayerY = 1.0f;
      }

      dPlayerX *= 64.0f;
      dPlayerY *= 64.0f;

      real32 NewPlayerX = GameState->PlayerX + Input->dtForFrame * dPlayerX;
      real32 NewPlayerY = GameState->PlayerY + Input->dtForFrame * dPlayerY;

      raw_position PlayerPos = {GameState->PlayerTileMapX, GameState->PlayerTileMapY, NewPlayerX, NewPlayerY};
      raw_position PlayerL = PlayerPos;
      PlayerL.X -= 0.5f * PlayerWidth;
      raw_position PlayerR = PlayerPos;
      PlayerR.X += 0.5f * PlayerWidth;

      if(IsWorldPointEmpty(&World, PlayerPos) && IsWorldPointEmpty(&World, PlayerL) && IsWorldPointEmpty(&World, PlayerR))
      {
	canonical_position CanPos = GetCanonicalPosition(&World, PlayerPos);
	GameState->PlayerTileMapX = CanPos.TileMapX;
	GameState->PlayerTileMapY = CanPos.TileMapY;
	
	GameState->PlayerX = World.UpperLeftX + World.TileWidth * CanPos.TileX + CanPos.TileRelX;
	GameState->PlayerY = World.UpperLeftY + World.TileHeight * CanPos.TileY + CanPos.TileRelY;  
      }
      
    }
    /*GameState->PlayerX += (int)(4.0f*Controller->AvarageStickX);
    GameState->PlayerY -= (int)(4.0f*Controller->AvarageStickY);
    if(GameState->Jump > 0)
    {
      GameState->PlayerY += (int)(10.0f * sinf(0.5f * Pi32 * GameState->Jump));
    }
    if(Controller->ActionDown.EndedDown)
    {
      GameState->Jump = 4.0f;
      }*/
  }
  /*GameState->Jump -= 0.033f;
  SomeGradient(Buffer, GameState->XOffset, GameState->YOffset);
  RenderPlayer(Buffer, GameState->PlayerX, GameState->PlayerY);
  
  for(int ButtonIndex = 0; ButtonIndex < ArrayCount(Input->MouseButtons); ++ButtonIndex)
  {
    if(Input->MouseButtons[ButtonIndex].EndedDown)
    {
      RenderPlayer(Buffer, 10 + 20 * ButtonIndex, 10);
    }
  }
  RenderPlayer(Buffer, Input->MouseX, Input->MouseY);*/
  //tile_map *TileMap = GetTileMap(&World, GameState->PlayerTileMapX, GameState->PlayerTileMapY);
  Assert(TileMap);
  for(int Row = 0; Row < TILE_MAP_COUNT_Y; ++Row)
  {
    for(int Column = 0; Column < TILE_MAP_COUNT_X; ++Column)
    {
      uint32 TileID = GetTileValueUnchecked(&World, TileMap, Column, Row);
      real32 Grey = 0.5f;
      if(TileID == 1)
      {
	Grey = 1.0f;
      }

      real32 MinX = World.UpperLeftX + ((real32)(World.TileWidth) * Column);
      real32 MinY = World.UpperLeftY + ((real32)(World.TileHeight) * Row);
      real32 MaxX = MinX + World.TileWidth;
      real32 MaxY = MinY + World.TileHeight;

      DrawRectangle(Buffer, MinX, MaxX, MinY, MaxY, Grey, Grey, Grey);

    }
  }

  DrawRectangle(Buffer, PlayerLeft, PlayerLeft + PlayerWidth, PlayerTop, PlayerTop + PlayerHeight, 1.0f, 1.0f, 0.0f);
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



