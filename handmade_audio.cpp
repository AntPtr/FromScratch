
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


internal void OutputPlaySound(audio_state *AudioState, game_sound_output_buffer *SoundBuffer, game_assets *Assets, memory_arena *TempArena)
{  
  temporary_memory MixerMemory = BeginTemporaryMemory(TempArena);
  
  real32 *Channel0 = PushArray(TempArena, SoundBuffer->SampleCount, real32);
  real32 *Channel1 = PushArray(TempArena, SoundBuffer->SampleCount, real32);

  {
    real32* Dest0 = Channel0;
    real32* Dest1 = Channel1;
    for (uint16 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
    {
      *Dest0++ = 0.0f;
      *Dest1++ = 0.0f;
    }
  }

   for(playing_sound **PlayingSoundPtr = &AudioState->FirstPlayingSound; *PlayingSoundPtr; )
  {
    playing_sound *PlayingSound = *PlayingSoundPtr;
    bool32 SoundFinished = false;
    uint32 TotalSamplesToMix = SoundBuffer->SampleCount;
    real32* Dest0 = Channel0;
    real32* Dest1 = Channel1;

    while (TotalSamplesToMix && !SoundFinished)
    {
      loaded_sound* LoadedSound = GetSound(Assets, PlayingSound->ID);
      if (LoadedSound)
      {
	asset_sound_info* Info = GetSoundInfo(Assets, PlayingSound->ID);
	PrefetchSound(Assets, Info->NextIDToPlay);

	real32 Volume0 = PlayingSound->Volumes[0];
	real32 Volume1 = PlayingSound->Volumes[1];
	uint32 SamplesToMix = TotalSamplesToMix;
	uint32 SamplesRemaing = LoadedSound->SampleCount - PlayingSound->SamplesPlayed;
	if (SamplesToMix > SamplesRemaing)
	{
	  SamplesToMix = SamplesRemaing;
	}
	for (uint32 SampleIndex = PlayingSound->SamplesPlayed; SampleIndex < PlayingSound->SamplesPlayed + SamplesToMix; ++SampleIndex)
	{
	  real32 SampleValue = LoadedSound->Samples[0][SampleIndex];
	  *Dest0++ += SampleValue * Volume0;
	  *Dest1++ += SampleValue * Volume1;
	}

	PlayingSound->SamplesPlayed += SamplesToMix;
	TotalSamplesToMix -= SamplesToMix;

	if (PlayingSound->SamplesPlayed == LoadedSound->SampleCount)
	{
	  if (IsValid(Info->NextIDToPlay))
	  {
	    PlayingSound->ID = Info->NextIDToPlay;
	    PlayingSound->SamplesPlayed = 0;
	  }
	  else
	  {
	    SoundFinished = true;
	  }
	}
      }
      else
      {
	LoadSound(Assets, PlayingSound->ID);
	break;
      }
    }
    if(SoundFinished)
    { 
      *PlayingSoundPtr = PlayingSound->Next;
      PlayingSound->Next = AudioState->FirstFreePlayingSound;
      AudioState->FirstFreePlayingSound = PlayingSound;      
    }
    else
    {
      PlayingSoundPtr = &PlayingSound->Next;
    }
  }
  
  int16 *SampleOut = SoundBuffer->Samples;

  real32 *Source0 = Channel0;
  real32 *Source1 = Channel1;

  for(uint16 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
  {
    real32 RealSample0 = *Source0++;
    real32 RealSample1 = *Source1++;

    if(RealSample0 > 32767.0f) RealSample0 = 32767.0f;
    if(RealSample0 < -32768.0f) RealSample0 = -32768.0f;

    if(RealSample1 > 32767.0f) RealSample1 = 32767.0f;
    if(RealSample1 < -32768.0f) RealSample1 = -32768.0f;

    int16 SampleValue0 = (int16)(RealSample0 + 0.5f);
    int16 SampleValue1 = (int16)(RealSample1 + 0.5f);


    *SampleOut++ = SampleValue0;
    *SampleOut++ = SampleValue1;
  }
  EndTemporaryMemory(MixerMemory);

}

internal playing_sound *PlaySound(audio_state *AudioState, sound_id SoundID, bool32 Loop, v2 Volume = {1.0f, 1.0f})
{
  if(!AudioState->FirstFreePlayingSound)
  {
    AudioState->FirstFreePlayingSound = PushStruct(AudioState->PermArena, playing_sound);
    AudioState->FirstFreePlayingSound->Next = 0;
  }
  playing_sound *PlayingSound = AudioState->FirstFreePlayingSound;
  AudioState->FirstFreePlayingSound = PlayingSound->Next;

  PlayingSound->SamplesPlayed = 0;
  PlayingSound->Volumes[0] = Volume.x;
  PlayingSound->Volumes[1] = Volume.y;
  PlayingSound->ID = SoundID;
  PlayingSound->Loop = Loop;
  
  PlayingSound->Next = AudioState->FirstPlayingSound;
  AudioState->FirstPlayingSound = PlayingSound;

  return PlayingSound;
}

internal void InitializeAudioState(audio_state *AudioState, memory_arena *PermArena)
{
  AudioState->PermArena = PermArena;
  AudioState->FirstPlayingSound = 0;
  AudioState->FirstFreePlayingSound = 0;
}


