
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

internal void ChangeVolume(audio_state *AudioState, playing_sound *Sound, real32 FadeDurationInSeconds, v2 Volume)
{
  if(FadeDurationInSeconds <= 0.0f)
  {
    Sound->CurrentVolume = Sound->TargetVolume = Volume; 
  }
  else
  {
    real32 OneOverFade = 1.0f / FadeDurationInSeconds;
    Sound->TargetVolume = Volume;
    Sound->dCurrentVolume = (Sound->TargetVolume - Sound->CurrentVolume)*OneOverFade;
  }
}

internal void OutputPlaySound(audio_state *AudioState, game_sound_output_buffer *SoundBuffer, game_assets *Assets, memory_arena *TempArena)
{  
  temporary_memory MixerMemory = BeginTemporaryMemory(TempArena);

  real32 SecondsPerSample = 1.0f / (real32)(SoundBuffer->SamplesPerSecond);
  real32 *Channel0 = PushArray(TempArena, SoundBuffer->SampleCount, real32);
  real32 *Channel1 = PushArray(TempArena, SoundBuffer->SampleCount, real32);
  
#define ChannelCount 2
  
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

    while(TotalSamplesToMix && !SoundFinished)
    {
      loaded_sound* LoadedSound = GetSound(Assets, PlayingSound->ID);
      if(LoadedSound)
      {
	asset_sound_info* Info = GetSoundInfo(Assets, PlayingSound->ID);
	PrefetchSound(Assets, Info->NextIDToPlay);

	v2 Volume = PlayingSound->CurrentVolume;
	v2 dVolume = SecondsPerSample*PlayingSound->dCurrentVolume; 
	
	uint32 SamplesToMix = TotalSamplesToMix;
	uint32 SamplesRemaing = LoadedSound->SampleCount - PlayingSound->SamplesPlayed;
	
	if(SamplesToMix > SamplesRemaing)
	{
	  SamplesToMix = SamplesRemaing;
	}

	bool32 VolumeEnded[ChannelCount] = {};

	for(uint32 ChannelIndex = 0; ChannelIndex < ArrayCount(VolumeEnded); ++ChannelIndex)
	{
	  if(dVolume.E[ChannelIndex] != 0.0f)
	  {
	    real32 DeltaVolume = PlayingSound->TargetVolume.E[ChannelIndex] - Volume.E[ChannelIndex];
	    uint32 VolumeSampleCount = (uint32)(DeltaVolume / dVolume.E[ChannelIndex] + 0.5f);
	    if(SamplesToMix > VolumeSampleCount)
	    {
	      SamplesToMix = VolumeSampleCount;
	      VolumeEnded[ChannelIndex] = true;
	    }
	  }
	}
	
	for(uint32 SampleIndex = PlayingSound->SamplesPlayed; SampleIndex < PlayingSound->SamplesPlayed + SamplesToMix; ++SampleIndex)
	{
	  real32 SampleValue = LoadedSound->Samples[0][SampleIndex];
	  *Dest0++ += SampleValue * Volume.E[0];
	  *Dest1++ += SampleValue * Volume.E[1];

	  Volume += dVolume;
	}

	PlayingSound->CurrentVolume = Volume;

	for(uint32 ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
	{
	  if(VolumeEnded[ChannelIndex])
	  {
	    PlayingSound->CurrentVolume.E[ChannelIndex] =
	      PlayingSound->TargetVolume.E[ChannelIndex];
	    PlayingSound->dCurrentVolume.E[ChannelIndex] = 0.0f;
	  }
	}
	
	PlayingSound->SamplesPlayed += SamplesToMix;
	TotalSamplesToMix -= SamplesToMix;

	if(PlayingSound->SamplesPlayed == LoadedSound->SampleCount)
	{
	  if(IsValid(Info->NextIDToPlay))
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
  PlayingSound->CurrentVolume = PlayingSound->TargetVolume = Volume;
  PlayingSound->dCurrentVolume = v2{0.0f, 0.0f};
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


