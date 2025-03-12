/*
  ================================================

  Codice educativo scritto da Antonio Pietroluongo
  Data: 15/05/2024
  Versione: 0.1
  
  ================================================
*/
#include "handmade.h"
#include <windows.h>
#include <stdio.h>
#include <xinput.h>
#include <dsound.h>
#include "handmade_win32.h"

global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable bool32 Running;
global_variable bool32 Sound;
global_variable bool32 GlobalPause;
global_variable int64 GlobalPerfCountFrequency;
global_variable bool32 DEBUGLoadCursor;
global_variable WINDOWPLACEMENT GLOBALWindowPosition = {sizeof(GLOBALWindowPosition)};


//Raymond Chen fullscreen method
void FullScreenToggle(HWND Window)
{
  DWORD Style = GetWindowLong(Window, GWL_STYLE);
  if (Style & WS_OVERLAPPEDWINDOW) {
    MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
    if (GetWindowPlacement(Window, &GLOBALWindowPosition) &&
        GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
    {
      SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
      SetWindowPos(Window, HWND_TOP,
                   MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                   MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                   MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                   SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
  }
  else
  {
    SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(Window, &GLOBALWindowPosition);
    SetWindowPos(Window, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
  }
}

internal void CatStrings(size_t SourceACount, char *SourceA, size_t SourceBCount, char *SourceB, size_t DestCount, char *Dest)
{
    // TODO: Dest bounds checking!
    for (int Index = 0;
         Index < SourceACount; 
         ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for (int Index = 0;
         Index < SourceBCount; 
         ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

internal int StringLength(char *String)
{
    int Count = 0;
    while (*String++) // While the character is not 0 (do not confuse with '0' the character!)
    {
        ++Count;
    }
    return Count;
}

internal void Win32GetEXEFilename(win32_state *State)
{
  DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFilename, sizeof(State->EXEFilename));
  State->OnePastLastEXEFilenameSlash = State->EXEFilename;
  for (char *Scan = State->EXEFilename; *Scan; ++Scan)
  {
    if (*Scan == '\\')
    {
        State->OnePastLastEXEFilenameSlash = Scan + 1;
    }
  }
}

internal void Win32BuildEXEPathFilename(win32_state *State, char *Filename, int DestCount, char *Dest)
{    
  CatStrings(State->OnePastLastEXEFilenameSlash - State->EXEFilename, State->EXEFilename, 
	     StringLength(Filename), Filename, DestCount, Dest);
}

internal void Win32GetInputFileLocation(win32_state *State, bool32 InputStream, int SlotIndex, int DestCount, char *Dest)
{
  char Name[64];
  wsprintf(Name, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
  Win32BuildEXEPathFilename(State, Name, DestCount, Dest);
}

internal win32_replay_buffer *Win32GetReplayBuffer(win32_state *State, unsigned int Index)
{
  Assert(Index < ArrayCount(State->ReplayBuffers));
  win32_replay_buffer *Result = &State->ReplayBuffers[Index];

  return Result;
}


internal void Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
  win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputRecordingIndex);
  if(ReplayBuffer->MemoryBlock)
  {
    State->InputRecordingIndex = InputRecordingIndex;
  
    char Filename[WIN32_STATE_FILENAME_COUNT];
    Win32GetInputFileLocation(State, true, InputRecordingIndex, WIN32_STATE_FILENAME_COUNT, Filename);
    State->RecordingHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    //DWORD BytesToWrite = (DWORD)State->TotalSize;
    //Assert(State->TotalSize == BytesToWrite);
    //DWORD BytesWritten;
    //WriteFile(State->RecordingHandle, State->GameMemoryBlock, BytesToWrite, &BytesWritten, 0);
#if 0
    LARGE_INTEGER FilePosition;
    FilePosition.QuadPart = State->TotalSize;
    SetFilePointerEx(State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
    CopyMemory(ReplayBuffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
  }
}

internal void Win32EndRecordingInput(win32_state *State)
{
  CloseHandle(State->RecordingHandle);
  State->InputRecordingIndex = 0;
}

internal void Win3RecordInput(win32_state *State, game_input* Input)
{
  DWORD BytesWritten;
  WriteFile(State->RecordingHandle, Input, sizeof(*Input), &BytesWritten, 0);
}

internal void Win32BeginInputPlayback(win32_state *State, int InputPlaybackIndex)
{
  win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputPlaybackIndex);
  if(ReplayBuffer->MemoryBlock)
  {
    State->InputPlaybackIndex = InputPlaybackIndex;
    
    char Filename[WIN32_STATE_FILENAME_COUNT];
    Win32GetInputFileLocation(State, true, InputPlaybackIndex, WIN32_STATE_FILENAME_COUNT, Filename);
    State->PlaybackHandle = CreateFileA(Filename, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    //DWORD BytesToRead = (DWORD)State->TotalSize;
    //Assert(State->TotalSize == BytesToRead);
    //DWORD BytesRead;
    //ReadFile(State->PlaybackHandle, State->GameMemoryBlock, BytesToRead, &BytesRead, 0);
#if 0
    LARGE_INTEGER FilePosition;
    FilePosition.QuadPart = State->TotalSize;
    SetFilePointerEx(State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif    
    CopyMemory(State->GameMemoryBlock, ReplayBuffer->MemoryBlock, State->TotalSize);
  }
}

internal void Win32EndInputPlayback(win32_state *State)
{
    CloseHandle(State->PlaybackHandle);
    State->InputPlaybackIndex = 0;
}

internal void Win32PlaybackInput(win32_state *State, game_input *Input)
{
  DWORD BytesRead = 0;
  if(ReadFile(State->PlaybackHandle, Input, sizeof(*Input), &BytesRead, 0))
  {
    if(BytesRead == 0)
    {
      //We've hit the end of the stream, go back to the beginning
      int PlayingIndex = State->InputPlaybackIndex;
      Win32EndInputPlayback(State);
      Win32BeginInputPlayback(State, PlayingIndex);
      ReadFile(State->PlaybackHandle, Input, sizeof(*Input), &BytesRead, 0);      
    }
  }
}

internal void Win32UnloadGameCode(win32_game_code *GameCode)
{
    if (GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }
    
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
  return (ERROR_DEVICE_NOT_CONNECTED);
}

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex,XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
  return (ERROR_DEVICE_NOT_CONNECTED);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputSetState XInputSetState_
#define XInputGetState XInputGetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

global_variable LPDIRECTSOUNDBUFFER SecondaryBuffer;

internal void Win32ClearSoundBuffer(win32_sound_output *SoundOutput)
{
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if(SUCCEEDED(SecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
  {
    uint8 *DestSample = (uint8 *)Region1;
    for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
    {
      *DestSample++ = 0;
    }
    DestSample = (uint8 *)Region2;
    for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
    {
      *DestSample++ = 0;
    }
    
    SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
  }
}

internal void Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD BytesToLock, DWORD BytesToWrite, game_sound_output_buffer *SourceBuffer)
{
  VOID *Region1;
  DWORD Region1Size;
  VOID *Region2;
  DWORD Region2Size;
  if(SUCCEEDED(SecondaryBuffer->Lock(BytesToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0)))
  {
    int16 *DestSample = (int16 *)Region1;
    DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
    DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
    int16 *SourceSample = SourceBuffer->Samples;
    for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
    {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      ++SoundOutput->RunningSampleIndex;
    }
	    
    DestSample = (int16 *)Region2;
	    
    for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
    {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      ++SoundOutput->RunningSampleIndex;
    }
    SecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
  }
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
  if(Memory)
  {
    VirtualFree(Memory, 0, MEM_RELEASE);
  }
}
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
  debug_read_file_result Result = {};
  HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if(FileHandle != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER FileSize;
    if(GetFileSizeEx(FileHandle, &FileSize))
    {
      uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
      Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
      if(Result.Contents)
      {
	DWORD BytesRead;
	if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (BytesRead == FileSize32))
	{
	  //File read successfully
	  Result.ContentSize = FileSize32;
	}
	else
	{
	   DEBUGPlatformFreeFileMemory(Thread ,Result.Contents);
	   Result.Contents = 0;
	}
      }
      else
      {
	//LOG
      }
    }
    CloseHandle(FileHandle);
  }
  else
  {
    //LOG
  }
  return Result;
}
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
  bool32 Result = false;
  HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
  if(FileHandle != INVALID_HANDLE_VALUE)
  {
    DWORD BytesWritten;
    if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
    {
      Result = (BytesWritten == MemorySize);
    }
    else
    {
       DEBUGPlatformFreeFileMemory(Thread ,Memory);
       Result = 0;
    }
   
    CloseHandle(FileHandle);
  }
  else
  {
    //LOG
  }
  return Result;
}

internal void Win32LoadXInput()
{
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if(!XInputLibrary)
  {
    XInputLibrary = LoadLibraryA("xinput1_3.dll");
  }
  if(XInputLibrary)
  {
    XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
  }
}

win32_window_dimension GetWindowDimension(HWND Window)
{
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  win32_window_dimension Result;
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Heigth = ClientRect.bottom - ClientRect.top;

  return Result;
}

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 SecondaryBufferSize)
{

  //Load the library
  HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
  if(DSoundLibrary)
  {
    direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
    LPDIRECTSOUND DirectSound;
    if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
    {
      WAVEFORMATEX WaveFormat{};
      WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
      WaveFormat.nChannels = 2;
      WaveFormat.wBitsPerSample = 16;
      WaveFormat.nSamplesPerSec = SamplesPerSecond;
      WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
      WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign; 
      WaveFormat.cbSize = 0;
      //Create primary buffer just for use the soundcard
      if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
      {
	LPDIRECTSOUNDBUFFER PrimaryBuffer;
        DSBUFFERDESC BufferDescription = {};
	BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
	BufferDescription.dwSize = sizeof(BufferDescription);
	if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
	{
	  if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
	  {
	  }
	  else
	  {
	  }
	}
	else
	{
	}
      }
      else
      {
      }
      //Create the secondary buffer for music
      DSBUFFERDESC SecBufferDescription = {};
      SecBufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;;
      SecBufferDescription.dwSize = sizeof(SecBufferDescription);
      SecBufferDescription.dwBufferBytes = SecondaryBufferSize;
      SecBufferDescription.lpwfxFormat = &WaveFormat;
      if(SUCCEEDED(DirectSound->CreateSoundBuffer(&SecBufferDescription, &SecondaryBuffer, 0)))
      {
	
      }
      else
      {
      }
    }
  }
  else
  {
  }
}

// Create a back buffer to draw in
internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer ,int Width, int Height)
{
  //Test if the BitMap is already allocated
  if(Buffer->Memory)
  {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }

  Buffer->BytesPerPixel = 4;
  
  Buffer->Width = Width;
  Buffer->Height = Height;
 
  Buffer->BitMapInfo.bmiHeader.biSize = sizeof(Buffer->BitMapInfo.bmiHeader);
  Buffer->BitMapInfo.bmiHeader.biWidth = Buffer->Width;
  Buffer->BitMapInfo.bmiHeader.biHeight = -Buffer->Height;
  Buffer->BitMapInfo.bmiHeader.biPlanes = 1;
  Buffer->BitMapInfo.bmiHeader.biBitCount = 32;
  Buffer->BitMapInfo.bmiHeader.biCompression = BI_RGB;

  int BitMapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
  Buffer->Memory = VirtualAlloc(0, BitMapMemorySize, MEM_COMMIT, PAGE_READWRITE);

  Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferWindow(win32_offscreen_buffer *Buffer , int WindowWidth, int WindowHeigth, HDC DeviceContex, int  X, int  Y, int Width, int Heigth)
{

  if(WindowHeigth >= Buffer->Height*1.5 && WindowWidth >= Buffer->Width*1.5)
  {
     StretchDIBits(DeviceContex,
		0, 0, WindowWidth, WindowHeigth,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory, &Buffer->BitMapInfo,
		DIB_RGB_COLORS, SRCCOPY);  
  }

  else
  {
    int OffsetX = 10;
    int OffsetY = 10;
    
    
    PatBlt(DeviceContex, 0, 0, WindowWidth, OffsetY, BLACKNESS);                             // top
    PatBlt(DeviceContex, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeigth, BLACKNESS); // bottom
    PatBlt(DeviceContex, 0, 0, OffsetX, WindowHeigth, BLACKNESS);                            // left
    PatBlt(DeviceContex, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeigth, BLACKNESS);                           
    
    StretchDIBits(DeviceContex,
    		OffsetX, OffsetY, Buffer->Width, Buffer->Height,
    		0, 0, Buffer->Width, Buffer->Height,
    		Buffer->Memory, &Buffer->BitMapInfo,
    		DIB_RGB_COLORS, SRCCOPY);
  }
}

internal void Win32ProcessXInputButtons(game_button_state *OldState, game_button_state *NewState, DWORD ButtonBit, DWORD ButtonState)
{
  NewState->EndedDown = ((ButtonState & ButtonBit) == ButtonBit);
  NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal void Win32ProcessKeyBoardButtons(game_button_state *NewState, bool32 IsDown)
{
  if(NewState->EndedDown != IsDown)
  {
    NewState->EndedDown = IsDown;
    ++NewState->HalfTransitionCount;
  }
}

internal void Win32MessageLoop(game_controller_input *KeyBoardController, win32_state *State)
{
   MSG Message;
        
   while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
   {
     switch(Message.message)
     {
       case WM_QUIT:
       {
	 Running = false;
       }break;
       case WM_SYSKEYDOWN:
       case WM_SYSKEYUP:
       case WM_KEYDOWN:
       case WM_KEYUP:
       {
         uint32 VKCode = (uint32)Message.wParam;
         bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
         bool32 IsDown =  ((Message.lParam & (1 << 31)) == 0);
	 if(WasDown != IsDown)
	 {
           if(VKCode == 'W')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->MoveUp, IsDown);
           }
           else if(VKCode == 'A')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->MoveLeft, IsDown);
           }
           else if(VKCode == 'S')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->MoveDown, IsDown);
           }
           else if(VKCode == 'D')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->MoveRight, IsDown);
           }
           else if(VKCode == 'Q')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->LeftShoulder, IsDown);
           }
           else if(VKCode == 'E')
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->RigthShoulder, IsDown);
           }
	   else if(VKCode == 'P')
           {
	     if(IsDown)
	     {
	       GlobalPause = !GlobalPause;
	     }
           }
	   else if(VKCode == 'L')
           {
	     if(IsDown)
	     {
	       if(State->InputPlaybackIndex == 0)
	       {
	         if(State->InputRecordingIndex == 0)
	         {
	         	 //Win32EndInputPlayback(State);
	         	 Win32BeginRecordingInput(State, 1);
	         }
	         else
	         {
	         	 Win32EndRecordingInput(State);
	         	 Win32BeginInputPlayback(State, 1);
	         }
	       }
	       else
	       {
		 Win32EndInputPlayback(State);
	       }
	     }
           }
           else if(VKCode == VK_UP)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->ActionUp, IsDown);
           }
           else if(VKCode == VK_LEFT)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->ActionLeft, IsDown);
           }
           else if(VKCode == VK_DOWN)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->ActionDown, IsDown);
           }
           else if(VKCode == VK_RIGHT)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->ActionRight, IsDown);
           }
           else if(VKCode == VK_ESCAPE)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->Back, IsDown);
           }
           else if(VKCode == VK_SPACE)
           {
	     Win32ProcessKeyBoardButtons(&KeyBoardController->Start, IsDown);	     
           }
	   if(IsDown)
	   {
             bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
             if((VKCode == VK_F4) && AltKeyWasDown)
             {
	       Running = false;
             }
	     if((VKCode == VK_RETURN) && AltKeyWasDown)
             {
	       FullScreenToggle(Message.hwnd);
             }
	   }
	 }
       }break;
     }
     TranslateMessage(&Message);
     DispatchMessage(&Message);
  }
}

internal real32 Win32ProcessXInputStick (SHORT Value, SHORT DeadZoneRange)
{
  real32 Result = 0;
  if(Value < -DeadZoneRange)
  {
    Result = ((real32)(Value + DeadZoneRange)) / (32768.0f - DeadZoneRange);
  }
  if(Value > DeadZoneRange)
  {
    Result = ((real32)(Value - DeadZoneRange)) / (32767.0f - DeadZoneRange);
  }
  return Result;
}

inline real32 Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
  real32 SecondsElapsed = ((real32)(End.QuadPart - Start.QuadPart ) / (real32)GlobalPerfCountFrequency);
  return SecondsElapsed;
}

inline LARGE_INTEGER Win32GetClock(void)
{
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return Result;
}

inline FILETIME Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
    {
      LastWriteTime = Data.ftLastWriteTime;
    }
    return (LastWriteTime);
}

internal win32_game_code Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
  win32_game_code Result = {};
  WIN32_FILE_ATTRIBUTE_DATA Ignored;
  
  if(!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
  {
    Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
    
    CopyFile(SourceDLLName, TempDLLName, FALSE);
    Result.GameCodeDLL = LoadLibraryA(TempDLLName);
    if(Result.GameCodeDLL)
    {
      Result.UpdateAndRender = 
        (game_update_and_render *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
      Result.GetSoundSamples = 
        (game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
      Result.IsValid = Result.GetSoundSamples && Result.UpdateAndRender;
    }
  }
  if (!Result.IsValid)
  {
    Result.UpdateAndRender = 0;
    Result.GetSoundSamples = 0;
  }

  return(Result);
}

internal void Win32DebugDrawVertical(win32_offscreen_buffer *Backbuffer, int X, int Top, int Bottom, uint32 Color)
{
  if(Top <= 0)
  {
    Top = 0;
  }
    
  if(Bottom > Backbuffer->Height)
  {
    Bottom = Backbuffer->Height;
  }
  if(X >= 0 && X <= Backbuffer->Width)
  {
    uint8 *Pixel = (uint8 *)Backbuffer->Memory + Top * Backbuffer->Pitch + X * Backbuffer->BytesPerPixel;
    for(int Y = Top; Y < Bottom; ++Y)
    {
      *(uint32 *)Pixel = Color;
      Pixel += Backbuffer->Pitch;
    }
  }
}

inline void Win32DrawSoundBufferMarker(win32_offscreen_buffer *Backbuffer, win32_sound_output *SoundOutput, real32 C, int PadX, int Top, int Bottom, DWORD Value, uint32 Color)
{
    real32 XReal = C * (real32)Value;
    int X = PadX + (int)XReal;
    Win32DebugDrawVertical(Backbuffer, X, Top, Bottom, Color);
}
#if 0
internal void  Win32DebugSyncDisplay(win32_offscreen_buffer *Backbuffer, int MarkerCount, win32_debug_time_marker *Markers, int CurrentMarkerIndex, win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame)
{
  int PadX = 16;
  int PadY = 16;

  int LineHeight = 64;
  
  real32 C = (real32)(Backbuffer->Width - (PadX * 2)) / (real32)SoundOutput->SecondaryBufferSize;
  for(int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
  {
    DWORD PlayColor = 0xFFFFFFFF; //White
    DWORD WriteColor = 0xFFFF0000; //Red
    DWORD ExpectedFlipColor = 0xFFFFFF00; // Yellow
    DWORD PlayWindowColor = 0xFFFF00FF;

        
    int Top = PadY;
    int Bottom = PadY + LineHeight;
    win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        
    Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
    Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
    Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
    Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
    Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
    Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);
        
    if(MarkerIndex == CurrentMarkerIndex)
    {
      Top += LineHeight + PadY;
      Bottom += LineHeight + PadY;
      Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
      Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColor);
      Top += LineHeight + PadY;
      Bottom += LineHeight + PadY;
      Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
      Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);
      Top += LineHeight + PadY;
      Bottom += LineHeight + PadY;

      Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, PadY, Bottom, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
    }
    Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
    Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor + (480 * SoundOutput->BytesPerSample), PlayWindowColor);
    Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColor);
  }
}
#endif



LRESULT MainWindowCallback(
  HWND    Windows,
  UINT    Message,
  WPARAM  WParam,
  LPARAM  LParam
)
{
  LRESULT Result = 0;
  switch(Message)
  { 
    case WM_SIZE:
    {
    } break;
    
    case WM_DESTROY:
    {
      Running = false;
      OutputDebugStringA("WM_DESTROY");
    } break;
    
    case WM_CLOSE:
    {
      Running = false;
      OutputDebugStringA("WM_CLOSE");
    } break;

    case WM_SETCURSOR:
    {
      if(DEBUGLoadCursor)
      {
	Result = DefWindowProcA(Windows, Message, WParam, LParam);
      }
      else
      {
	SetCursor(0);
      }
    } break;
    
    case WM_ACTIVATEAPP:
    {
#if 0
      if (WParam == TRUE)
      {
        SetLayeredWindowAttributes(Windows, RGB(0, 0, 0), 255, LWA_ALPHA);
      }
      else
      {
        SetLayeredWindowAttributes(Windows, RGB(0, 0, 0), 64, LWA_ALPHA);
      }
      
#endif
    } break;

    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContex =  BeginPaint(Windows, &Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      int Heigth = Paint.rcPaint.bottom - Paint.rcPaint.top;
      win32_window_dimension Dimension = GetWindowDimension(Windows);

      Win32DisplayBufferWindow(&GlobalBackBuffer , Dimension.Width, Dimension.Heigth, DeviceContex, X, Y, Width, Heigth);
      EndPaint(Windows, &Paint);
    } break;

    default:
    {
      Result = DefWindowProcA(Windows, Message, WParam, LParam);
    }break;
    
  }
  return Result;
}


int WINAPI wWinMain(HINSTANCE Instance,
		    HINSTANCE PrevInstance,
		    PWSTR CommandLine,
		    int ShoCode)
{

  Win32LoadXInput();
  WNDCLASSA WindowClass = {};
  win32_state Win32State = {};
  Win32ResizeDIBSection(&GlobalBackBuffer , 960, 540);
  thread_context Thread = {};

  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
  //WindowClass.hIcon;
  WindowClass.lpszClassName = "Finestra fatta da zero";

  UINT SchedulerMS = 1;
  bool32 SleepIsGranular = (timeBeginPeriod(SchedulerMS) == TIMERR_NOERROR);
  LARGE_INTEGER QueryPerfromanceResult;
  QueryPerformanceFrequency(&QueryPerfromanceResult);
  GlobalPerfCountFrequency = QueryPerfromanceResult.QuadPart;
  bool32 SoundIsValid = false;
  LARGE_INTEGER FlipWallClock = Win32GetClock();
  Win32GetEXEFilename(&Win32State);

#if H_INTERNAL
  DEBUGLoadCursor = true;
#endif

  char SourceGameCodeDLLFullPath[WIN32_STATE_FILENAME_COUNT];
  Win32BuildEXEPathFilename(&Win32State, "handmade.dll", 
			    sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
  
  char TempGameCodeDLLFullPath[WIN32_STATE_FILENAME_COUNT];
  Win32BuildEXEPathFilename(&Win32State, "handmade_temp.dll", 
			    sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);
  
  char LockFullPath[WIN32_STATE_FILENAME_COUNT];
  Win32BuildEXEPathFilename(&Win32State, "lock.tmp", 
			    sizeof(LockFullPath), LockFullPath);
  
  if(RegisterClassA(&WindowClass))
  {
    HWND Window = CreateWindowExA(
      0,
      //WS_EX_TOPMOST | WS_EX_LAYERED,
      WindowClass.lpszClassName,
      "Fatto da zero",
      WS_OVERLAPPEDWINDOW|WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      Instance,
      0
    );
    if(Window)
    {
      // HDC DeviceContex = GetDC(Window);
      Running = true;
      Sound = false;

      game_input Input[2] = {};
      game_input *NewInput = &Input[0];
      game_input *OldInput = &Input[1];

      int MonitorRefreshRate = 60;
      HDC RefreshDC = GetDC(Window);
      int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
      ReleaseDC(Window, RefreshDC);
      if(Win32RefreshRate > 1)
      {
	MonitorRefreshRate = Win32RefreshRate;
      }
      real32 GameRefresh (MonitorRefreshRate / 2.0f);
      real32 TargetSecPFrame = 1.0f / (real32)GameRefresh;
           
      win32_sound_output SoundOutput = {};
      SoundOutput.SamplePerSecond = 48000;
      SoundOutput.BytesPerSample = sizeof(int16)*2;
      SoundOutput.RunningSampleIndex = 0;
      SoundOutput.SecondaryBufferSize = SoundOutput.SamplePerSecond*SoundOutput.BytesPerSample;
      SoundOutput.SafetyBytes =(int)((real32)(SoundOutput.BytesPerSample) *(real32)(SoundOutput.SamplePerSecond) / GameRefresh) / 3;
      Win32InitDSound(Window, SoundOutput.SamplePerSecond, SoundOutput.SecondaryBufferSize);
      Win32ClearSoundBuffer(&SoundOutput);
      SecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

      int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
#if H_INTERNAL
      LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
      LPVOID BaseAddress = 0;
#endif
      game_memory GameMemory = {};
      GameMemory.PermanentStorageSize = Gigabytes(1);
      GameMemory.TransientStorageSize  = Gigabytes(1);
      uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
      Win32State.TotalSize = (GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize);
      Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
      GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
      GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
      GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
      GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
      GameMemory.TransientStorage = ((uint8 *) GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

      for(int ReplayIndex = 0; ReplayIndex < ArrayCount(Win32State.ReplayBuffers); ++ReplayIndex)
      {
	win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];
	Win32GetInputFileLocation(&Win32State, false, ReplayIndex, sizeof(ReplayBuffer->Filename), ReplayBuffer->Filename);          
	ReplayBuffer->FileHandle = CreateFileA(ReplayBuffer->Filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	LARGE_INTEGER MaxSize;
	MaxSize.QuadPart = Win32State.TotalSize;
        ReplayBuffer->MemoryMap = CreateFileMapping(ReplayBuffer->FileHandle, 0, PAGE_READWRITE, MaxSize.HighPart, MaxSize.LowPart, 0);
	ReplayBuffer->MemoryBlock = MapViewOfFile(ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
	if (ReplayBuffer->MemoryBlock)
	{
	  // All good
	}
	else
	{
	  // TODO(casey): Diagnostic
	}
      }
      
      if(Samples && GameMemory.PermanentStorage)
      {

        LARGE_INTEGER LastCounter = Win32GetClock();
        
        int64 LastCycleCount = __rdtsc();

	int DebugTimeMarkerIndex = 0;
	win32_debug_time_marker DebugTimeMarkers[30] = {};
	win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, LockFullPath);
	
        while(Running)
        {
	  FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
	  if(CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime))
	  {
	    Game.DLLLastWriteTime = NewDLLWriteTime;
	    Win32UnloadGameCode(&Game);
	    Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, LockFullPath);
	  }
	  NewInput->dtForFrame = TargetSecPFrame;
	  game_controller_input *NewKeyBoardController = GetController(NewInput, 0);
	  game_controller_input *OldKeyBoardController = GetController(OldInput, 0);
	  
	  game_controller_input ZeroController = {};
	  *NewKeyBoardController = ZeroController;
	  NewKeyBoardController->IsConnected = true;
	  
	  for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyBoardController->Buttons); ++ButtonIndex)
	  {
	    NewKeyBoardController->Buttons[ButtonIndex].EndedDown = OldKeyBoardController->Buttons[ButtonIndex].EndedDown;
	  }
	  
          
	  Win32MessageLoop(NewKeyBoardController, &Win32State);
	  
	  if(!GlobalPause)
	  {
	    POINT MouseP;
	    GetCursorPos(&MouseP);
	    ScreenToClient(Window, &MouseP);
	    NewInput->MouseX = MouseP.x;
            NewInput->MouseY = MouseP.y;
            NewInput->MouseZ = 0; // TODO: Mousewheel?
	    Win32ProcessKeyBoardButtons(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & 1 << 15);
	    Win32ProcessKeyBoardButtons(&NewInput->MouseButtons[1], GetKeyState(VK_MBUTTON) & 1 << 15);
	    Win32ProcessKeyBoardButtons(&NewInput->MouseButtons[2], GetKeyState(VK_RBUTTON) & 1 << 15);
	    Win32ProcessKeyBoardButtons(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & 1 << 15);
	    Win32ProcessKeyBoardButtons(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & 1 << 15);

	    DWORD MaxControllerCount = XUSER_MAX_COUNT;
            if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
            {
              MaxControllerCount = ArrayCount(NewInput->Controllers) - 1;
            }
            
            for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex)
            {
              game_controller_input *OldController = GetController(OldInput, ControllerIndex);
              game_controller_input *NewController = GetController(NewInput, ControllerIndex);
	    
              XINPUT_STATE ControllerState;
              if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
              {
                 //NOTE: The controller is plugged in
		NewController->IsConnected = true;
		NewController->IsAnalog = OldController->IsAnalog;
                
	        XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
	    
	        if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
	        {
	          NewController->AvarageStickY = 1.0f;
	          NewController->IsAnalog = false;
	        }
	    
	        if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
	        {
	          NewController->AvarageStickY = -1.0f;
	          NewController->IsAnalog = false;
	        }
	    
	        if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
	        {
	          NewController->AvarageStickY = -1.0f;
	          NewController->IsAnalog = false;
	        }
	    
	        if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
	        {
	          NewController->AvarageStickY = 1.0f;
	          NewController->IsAnalog = false;
	          NewController->IsAnalog = false;
	        }
	        
                NewController->AvarageStickX = Win32ProcessXInputStick(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                NewController->AvarageStickY = Win32ProcessXInputStick(Pad->sThumbLY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	        if((NewController->AvarageStickX != 0) || (NewController->AvarageStickY != 0))
	        {
	          NewController->IsAnalog = true;
	        }
	    
	        real32 Threshold = 0.5f;
	        Win32ProcessXInputButtons (&OldController->MoveLeft, &NewController->MoveLeft, 1, (NewController->AvarageStickX < -Threshold ? 1 : 0 ));
	        Win32ProcessXInputButtons (&OldController->MoveRight, &NewController->MoveRight, 1, (NewController->AvarageStickX > Threshold ? 1 : 0 ));
	        Win32ProcessXInputButtons (&OldController->MoveDown, &NewController->MoveDown, 1, (NewController->AvarageStickX < -Threshold ? 1 : 0 ));
	        Win32ProcessXInputButtons (&OldController->MoveUp, &NewController->MoveUp, 1, (NewController->AvarageStickX > Threshold ? 1 : 0 ));
	    
	        
                Win32ProcessXInputButtons (&OldController->ActionDown, &NewController->ActionDown, XINPUT_GAMEPAD_A, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->ActionRight, &NewController->ActionRight, XINPUT_GAMEPAD_B, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->ActionLeft, &NewController->ActionLeft, XINPUT_GAMEPAD_X, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->ActionUp, &NewController->ActionUp, XINPUT_GAMEPAD_Y, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->LeftShoulder, &NewController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->RigthShoulder, &NewController->RigthShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->Start, &NewController->Start, XINPUT_GAMEPAD_START, Pad->wButtons);
                Win32ProcessXInputButtons (&OldController->Back, &NewController->Back, XINPUT_GAMEPAD_BACK, Pad->wButtons);
                    
              }
              else
              {
	        NewController->IsConnected = false;
              }
            }        
            	      
            game_offscreen_buffer Buffer = {};
            Buffer.Memory = GlobalBackBuffer.Memory;
            Buffer.Width = GlobalBackBuffer.Width;
            Buffer.Height = GlobalBackBuffer.Height;
            Buffer.Pitch = GlobalBackBuffer.Pitch;
	    Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

	    if(Win32State.InputRecordingIndex)
	    {
	      Win3RecordInput(&Win32State, NewInput);
	    }
	    if(Win32State.InputPlaybackIndex)
	    {
	      Win32PlaybackInput(&Win32State, NewInput);
	    }

	    if(Game.UpdateAndRender)
	    {
	      Game.UpdateAndRender(&Thread ,&GameMemory, NewInput , &Buffer);
	    }
	    LARGE_INTEGER AudioWallClock = Win32GetClock();
	    real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

	    
	    DWORD PlayCursor = 0;
	    DWORD WriteCursor = 0;
	    if(SecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
	    {
	      if(!SoundIsValid)
	      {
	        SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
	        SoundIsValid = true;
	      }
	          
	      DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize);
	      DWORD ExpectedSoundBytesPerFrame =(int)(((real32)SoundOutput.BytesPerSample * (real32)SoundOutput.SamplePerSecond) / GameRefresh);
	      real32 SecondsLeftUntilFlip = TargetSecPFrame - FromBeginToAudioSeconds;
	      DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip / TargetSecPFrame) * (real32)ExpectedSoundBytesPerFrame);
	      DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
            
	      DWORD TargetCursor = 0;
	      bool32 CardIsLowLatency = true;
	      DWORD SafeWriteCursor = WriteCursor;
	      if(SafeWriteCursor < PlayCursor)
	      {
	        SafeWriteCursor += SoundOutput.SecondaryBufferSize;
	      }
	      Assert(SafeWriteCursor > PlayCursor);
	      SafeWriteCursor += SoundOutput.SafetyBytes;
	      bool32 AudioCardIsLowLatency = SafeWriteCursor < ExpectedFrameBoundaryByte;
	      if(AudioCardIsLowLatency)
	      {
	        TargetCursor = ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame;
	      }
	      else
	      {
	        TargetCursor = WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes;
	      }
	      TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;
    	    
	      DWORD BytesToWrite = 0;
	      if(ByteToLock > TargetCursor)
	      {
	        BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
	        BytesToWrite += TargetCursor;
	      }
	      else
	      {
	        BytesToWrite = TargetCursor - ByteToLock;
	      }
	    
	      game_sound_output_buffer SoundBuffer = {};
	      SoundBuffer.SamplesPerSecond = SoundOutput.SamplePerSecond;
	      SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
	      SoundBuffer.Samples = Samples;
	      if(Game.GetSoundSamples)
	      {
		Game.GetSoundSamples(&Thread ,&GameMemory, &SoundBuffer);
	      }
	      Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
#if H_INTERNAL
	      DWORD UnwrappedWriteCursor = WriteCursor;
	      if (UnwrappedWriteCursor < PlayCursor)
	      {
	        UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
	      }
	      DWORD AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
	      real32 AudioLatencySeconds = ((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) / (real32)SoundOutput.SamplePerSecond;
	      char TextBuffer[256];
	      sprintf_s(TextBuffer, sizeof(TextBuffer),"BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%.2fs)\n", ByteToLock, TargetCursor, BytesToWrite,
	    	    PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
	      win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
	      Marker->OutputPlayCursor = PlayCursor;
	      Marker->OutputWriteCursor = WriteCursor;
	      Marker->OutputLocation = ByteToLock;
	      Marker->OutputByteCount = BytesToWrite;
	      Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

	      OutputDebugStringA(TextBuffer);
#endif	    
	    }
	    else
	    {
	      SoundIsValid = false;
	    }
            
            LARGE_INTEGER WorkCounter = Win32GetClock();
	    real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
	    
            //int64 CycleElapsed = EndCycleCount - LastCycleCount;
 	    
             
	    real32 SecondsElapsedForFrame = WorkSecondsElapsed;
	    if(SecondsElapsedForFrame < TargetSecPFrame)
	    {
	    
	      if(SleepIsGranular)
	      {
	        DWORD SleepMS = (DWORD)(1000.0f * (TargetSecPFrame - SecondsElapsedForFrame));
	        if(SleepMS > 0)
	        {
	          Sleep(SleepMS);
	        }
	      }
	      real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetClock());
	      if(TestSecondsElapsedForFrame < TargetSecPFrame)
	      {
	        // TODO: LOG MISSED SLEEP HERE
	      }
	      while(SecondsElapsedForFrame < TargetSecPFrame)
	      {
	        SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetClock());
	      }	
	    }
	    else
	    {
	    }
	    LARGE_INTEGER EndCounter = Win32GetClock();
	    real32 MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);
	    int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
	    LastCounter = EndCounter;
	    
	    win32_window_dimension Dimension = GetWindowDimension(Window);
#if 0
	    Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers, DebugTimeMarkerIndex - 1, &SoundOutput, TargetSecPFrame);
#endif
	    HDC DeviceContex = GetDC(Window);
	   
            Win32DisplayBufferWindow(&GlobalBackBuffer, Dimension.Width, Dimension.Heigth, DeviceContex, 0, 0, Dimension.Width, Dimension.Heigth);
	    ReleaseDC(Window, DeviceContex);

	    FlipWallClock = Win32GetClock();
	    
#if H_INTERNAL
	    {
	      DWORD FlipPlayCursor = 0;
	      DWORD FlipWriteCursor = 0;
	      if (SUCCEEDED(SecondaryBuffer->GetCurrentPosition(&FlipPlayCursor, &FlipWriteCursor)))
	      {
	        Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
		win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];

	        Marker->FlipPlayCursor = FlipPlayCursor;
	        Marker->FlipWriteCursor = FlipWriteCursor;
	      }
	    }
#endif	    
	    int32 FPS = (int32)(GlobalPerfCountFrequency / CounterElapsed);
	    
	    char StrBuffer[256];
            sprintf_s(StrBuffer, "%.02fms/f - %dFPS\n", MSPerFrame, FPS);
            OutputDebugStringA(StrBuffer);
	    
            game_input *Temp = NewInput;
            NewInput = OldInput;
            OldInput = Temp;
	    
	    int64 EndCycleCount = __rdtsc();
            LastCycleCount = EndCycleCount;
#if H_INTERNAL
	    ++DebugTimeMarkerIndex;
	    if(DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers))
	    {
	      DebugTimeMarkerIndex = 0;
	    }
#endif
            }
	  }
      } 
      else
      {
	//TODO
      }
    }
    else
    {
      //TODO
    }
  }
  else
  {
    //TODO
  }
  
  return (0);
}

