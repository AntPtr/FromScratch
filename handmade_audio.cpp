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

internal void ChangeVolume(playing_sound *Sound, real32 FadeDurationInSeconds, v2 Volume)
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

internal void ChangePitch(playing_sound *Sound, real32 dSample)
{
  Sound->dSample = dSample;
}

internal void OutputPlaySound(audio_state *AudioState, game_sound_output_buffer *SoundBuffer, game_assets *Assets, memory_arena *TempArena)
{  
  temporary_memory MixerMemory = BeginTemporaryMemory(TempArena);

  real32 SecondsPerSample = 1.0f / (real32)(SoundBuffer->SamplesPerSecond);
  
  Assert((SoundBuffer->SampleCount & 3) == 0);
  uint32 ChunkCount = SoundBuffer->SampleCount / 4;

    
  __m128 *Channel0 = PushArray(TempArena, ChunkCount, __m128, 16);
  __m128 *Channel1 = PushArray(TempArena, ChunkCount, __m128, 16);
  
#define ChannelCount 2
  
  __m128 Zero = _mm_set1_ps(0.0f);
  __m128 One = _mm_set1_ps(1.0f);

  
  {
    __m128* Dest0 = Channel0;
    __m128* Dest1 = Channel1;
    for (uint16 SampleIndex = 0; SampleIndex < ChunkCount; ++SampleIndex)
    {
      _mm_store_ps((float *)Dest0++, Zero);
      _mm_store_ps((float *)Dest1++, Zero);
    }
  }

  for(playing_sound **PlayingSoundPtr = &AudioState->FirstPlayingSound; *PlayingSoundPtr; )
  {
    playing_sound *PlayingSound = *PlayingSoundPtr;
    bool32 SoundFinished = false;
    uint32 TotalChunksToMix = ChunkCount;
    __m128 *Dest0 = Channel0;
    __m128 *Dest1 = Channel1;

    while(TotalChunksToMix && !SoundFinished)
    {
      loaded_sound* LoadedSound = GetSound(Assets, PlayingSound->ID);
      if(LoadedSound)
      {
	asset_sound_info* Info = GetSoundInfo(Assets, PlayingSound->ID);
	PrefetchSound(Assets, Info->NextIDToPlay);

	v2 Volume = PlayingSound->CurrentVolume;
	v2 dVolume = SecondsPerSample*PlayingSound->dCurrentVolume;
	v2 dVolumeChunk = 4.0f*dVolume;

	real32 dSampleChunk = 4.0f*PlayingSound->dSample;
	real32 dSample = PlayingSound->dSample;
	
	//Channel0 volume 
	__m128 Volume0 = _mm_setr_ps(Volume.E[0] + 0.0f*dVolume.E[0],
				       Volume.E[0] + 1.0f*dVolume.E[0],
				       Volume.E[0] + 2.0f*dVolume.E[0],
				       Volume.E[0] + 3.0f*dVolume.E[0]);

	__m128 dVolume0 = _mm_set1_ps(dVolume.E[0]);
	__m128 dVolumeChunk0 = _mm_set1_ps(dVolumeChunk.E[0]);

	//Channel1 volume
	__m128 Volume1 = _mm_setr_ps(Volume.E[1] + 0.0f*dVolume.E[1],
				     Volume.E[1] + 1.0f*dVolume.E[1],
				     Volume.E[1] + 2.0f*dVolume.E[1],
				     Volume.E[1] + 3.0f*dVolume.E[1]);

	__m128 dVolume1 = _mm_set1_ps(dVolume.E[1]);
	__m128 dVolumeChunk1 = _mm_set1_ps(dVolumeChunk.E[1]);


	
	uint32 ChunksToMix = TotalChunksToMix;
	real32 RealChunksRemaing = (LoadedSound->SampleCount - RoundReal32ToInt32(PlayingSound->SamplesPlayed)) / dSampleChunk;
	uint32 ChunksRemaing = RoundReal32ToInt32(RealChunksRemaing);
	bool32 InputSoundEnded = false;

	if(ChunksToMix > ChunksRemaing)
	{
	  ChunksToMix = ChunksRemaing;
	  InputSoundEnded = true;
	}

	bool32 VolumeEnded[ChannelCount] = {};

	for(uint32 ChannelIndex = 0; ChannelIndex < ArrayCount(VolumeEnded); ++ChannelIndex)
	{
	  if(dVolumeChunk.E[ChannelIndex] != 0.0f)
	  {
	    real32 DeltaVolume = PlayingSound->TargetVolume.E[ChannelIndex] - Volume.E[ChannelIndex];
	    uint32 VolumeChunksCount = (uint32)(DeltaVolume / dVolumeChunk.E[ChannelIndex] + 0.5f);
	    if(ChunksToMix > VolumeChunksCount)
	    {
	      ChunksToMix = VolumeChunksCount;
	      VolumeEnded[ChannelIndex] = true;
	    }
	  }
	}

	real32 BeginSamplesPosition = PlayingSound->SamplesPlayed;
	real32 EndSamplesPosition = BeginSamplesPosition + ChunksToMix*dSampleChunk;
	real32 LoopC = (EndSamplesPosition - BeginSamplesPosition) / (real32)ChunksToMix;
	
	for(uint32 Index = 0; Index < ChunksToMix; ++Index)
	{
	  real32 SamplePosition = BeginSamplesPosition + LoopC*(real32)Index;

	  //Switch to 1 for enable linear samples blending 
#if 0
	  __m128 SamplePosF = _mm_setr_ps(SamplePosition + 0.0f*dSample,
					  SamplePosition + 1.0f*dSample,
					  SamplePosition + 2.0f*dSample,
					  SamplePosition + 3.0f*dSample);
	  
	  __m128i SampleIndex = _mm_cvttps_epi32(SamplePos);
	  __m128 Frac = _mm_sub_ps(SamplePos, _mm_cvtepi32_ps(SampleIndex));
	  
	  __m128 SampleValueF = _mm_setr_ps(LoadedSound->Samples[0][((int32 *)(&SampleIndex))[0]],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[1]],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[2]],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[3]]);

					   
	  __m128 SampleValueC = _mm_setr_ps(LoadedSound->Samples[0][((int32 *)(&SampleIndex))[0] + 1],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[1] + 1],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[2] + 1],
					    LoadedSound->Samples[0][((int32 *)(&SampleIndex))[3] + 1]);

	  __m128 SampleValue = _mm_add_ps(_mm_mul_ps(_mm_sub_ps(One, Frac), SampleValueF), _mm_mul_ps(Frac, SampleValueC));
#else
	  
	  __m128 SampleValue = _mm_setr_ps(LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 0.0f*dSample)],
					   LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 1.0f*dSample)],
					   LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 2.0f*dSample)],
					   LoadedSound->Samples[0][RoundReal32ToInt32(SamplePosition + 3.0f*dSample)]);

	    
#endif

	  __m128 D0 = _mm_load_ps((float *)&Dest0[0]);
	  __m128 D1 = _mm_load_ps((float *)&Dest1[0]);

	  D0 = _mm_add_ps(D0, (_mm_mul_ps(Volume0, SampleValue)));
	  D1 = _mm_add_ps(D1, (_mm_mul_ps(Volume1, SampleValue)));
	    
	  _mm_store_ps((float *)&Dest0[0], D0);
	  _mm_store_ps((float *)&Dest1[0], D1);
	  
	  ++Dest0;
	  ++Dest1;

	  Volume0 = _mm_add_ps(Volume0, dVolumeChunk0);
	  Volume1 = _mm_add_ps(Volume1, dVolumeChunk1);
	}

	PlayingSound->CurrentVolume.E[0] = ((real32 *)&Volume0)[0];
	PlayingSound->CurrentVolume.E[1] = ((real32 *)&Volume1)[1];

	for(uint32 ChannelIndex = 0; ChannelIndex < ChannelCount; ++ChannelIndex)
	{
	  if(VolumeEnded[ChannelIndex])
	  {
	    PlayingSound->CurrentVolume.E[ChannelIndex] =
	      PlayingSound->TargetVolume.E[ChannelIndex];
	    PlayingSound->dCurrentVolume.E[ChannelIndex] = 0.0f;
	  }
	}
	
	PlayingSound->SamplesPlayed = EndSamplesPosition;
	TotalChunksToMix -= ChunksToMix;

	if(InputSoundEnded)
	{
	  if(IsValid(Info->NextIDToPlay))
	  {
	    PlayingSound->ID = Info->NextIDToPlay;
	    PlayingSound->SamplesPlayed -= (real32)LoadedSound->SampleCount;
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
  
  __m128i *SampleOut = (__m128i *)SoundBuffer->Samples;

  __m128 *Source0 = Channel0;
  __m128 *Source1 = Channel1;

  for(uint32 SampleIndex = 0; SampleIndex < ChunkCount; ++SampleIndex)
  {
    __m128i L = _mm_cvtps_epi32(*Source0++);
    __m128i R = _mm_cvtps_epi32(*Source1++);

    __m128i LR0 = _mm_unpacklo_epi32(L, R);
    __m128i LR1 = _mm_unpackhi_epi32(L, R);

    __m128i S01 = _mm_packs_epi32(LR0, LR1);

    *SampleOut++ = S01;
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
  PlayingSound->dSample = 1.0f;
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


