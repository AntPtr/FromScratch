#if !defined(HANDMADE_H)
#include <math.h>
#include <stdint.h>

#define local_persist static
#define global_variable static
#define internal static
#define Pi32 3.14159265359f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef float real32;
typedef double real64;

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))

#if HANDMADE_SLOW
#define Assert(Expr) if(!(Expr)) {*(int *)0=0;}
#else
#define Assert(Expr)
#endif

struct thread_context
{
  int placeholder;
};

#if H_INTERNAL
struct debug_read_file_result
{
  uint32 ContentSize;
  void* Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name (thread_context *Thread ,void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name (thread_context *Thread ,char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name (thread_context *Thread ,char *Filename, uint32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

#define Kilobytes(value) (value*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

inline uint32 SafeTruncateUInt64 (uint64 Value)
{
  Assert(Value <= 0xFFFFFFFF);
  uint32 Result = (uint32)Value;
  return Result;
}

//Services that game provide for the platform layer
struct game_offscreen_buffer
{
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct game_sound_output_buffer
{
  int SamplesPerSecond;
  int SampleCount;
  int16 *Samples;
};

struct game_button_state
{
  int HalfTransitionCount;
  bool32 EndedDown;
};

struct game_controller_input
{
  bool32 IsAnalog;
  bool32 IsConnected;
  
  real32 AvarageStickX;
  real32 AvarageStickY;

  union
  {
    game_button_state Buttons[12];
    struct
    {
      game_button_state MoveUp;
      game_button_state MoveDown;
      game_button_state MoveLeft;
      game_button_state MoveRight;

      game_button_state ActionUp;
      game_button_state ActionDown;
      game_button_state ActionLeft;
      game_button_state ActionRight;
      
      game_button_state LeftShoulder;
      game_button_state RigthShoulder;

      game_button_state Start;
      game_button_state Back;

      //Just for checking that the size of struct match with Buttons array
      game_button_state Terminator;
    };
  };
};

struct game_input
{
  game_button_state MouseButtons[5];
  int32 MouseX, MouseY, MouseZ;
  real32 dtForFrame;
  game_controller_input Controllers[5];
};

inline game_controller_input *GetController(game_input *Input, int ControllerIndex)
{
  Assert(ControllerIndex < ArrayCount(Input->Controllers));
  game_controller_input *Result = &Input->Controllers[ControllerIndex];
  return Result;
}

struct game_state
{
  int32 PlayerTileMapX;
  int32 PlayerTileMapY;
  
  real32 PlayerX;
  real32 PlayerY;
};

struct tile_map
{    
  uint32 *Tiles;
};

struct world
{

  // TODO: Sparseness
  int32 TileMapCountX;
  int32 TileMapCountY; 

  tile_map *TileMaps;
  int32 CountX;
  int32 CountY;
    
  real32 UpperLeftX;
  real32 UpperLeftY;
  real32 TileWidth;
  real32 TileHeight;
};

struct canonical_position
{
  int32 TileMapX;
  int32 TileMapY;

  int32 TileX;
  int32 TileY;

  real32 TileRelX;
  real32 TileRelY;
};

struct raw_position
{
  int32 TileMapX;
  int32 TileMapY;

  real32 X;
  real32 Y;
};

struct game_memory
{
  bool32 IsInitialized;

  uint64 PermanentStorageSize;
  void *PermanentStorage;

  uint64 TransientStorageSize;
  void *TransientStorage;

  debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
  debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
  debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread ,game_memory *Memory, game_input *Input, game_offscreen_buffer* Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);


#define GAME_GET_SOUND_SAMPLES(name) void name(thread_context *Thread ,game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

 
#define HANDMADE_H
#endif