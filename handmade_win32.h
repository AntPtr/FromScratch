#if !defined(HANDMADE_WIN32)
struct win32_offscreen_buffer
{
  BITMAPINFO BitMapInfo;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct win32_game_code
{
  HMODULE GameCodeDLL;
  
  //Must check before calling!
  game_update_and_render *UpdateAndRender;
  game_get_sound_samples *GetSoundSamples;

  FILETIME DLLLastWriteTime;
    
  bool32 IsValid;
};

struct win32_window_dimension
{
  int Width;
  int Heigth;
};

struct win32_sound_output
{
  int SamplePerSecond;
  int BytesPerSample;
  uint32 RunningSampleIndex;
  DWORD SecondaryBufferSize;
  DWORD SafetyBytes;
  real32 tSine;
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation; 
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
  
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

#define WIN32_STATE_FILENAME_COUNT MAX_PATH

struct win32_replay_buffer
{
  HANDLE FileHandle;
  HANDLE MemoryMap;

  char Filename[WIN32_STATE_FILENAME_COUNT];
  void *MemoryBlock;
};

struct win32_state
{
  int InputRecordingIndex;
  HANDLE RecordingHandle;

  win32_replay_buffer ReplayBuffers[4];

  HANDLE PlaybackHandle;
  int InputPlaybackIndex;

  uint64 TotalSize;
  void *GameMemoryBlock;

  char EXEFilename[WIN32_STATE_FILENAME_COUNT];
  char *OnePastLastEXEFilenameSlash;
};
#define HANDMADE_WIN32
#endif
