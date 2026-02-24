#if !defined(HANDMADE_AUDIO_H)
struct playing_sound
{
  v2 CurrentVolume;
  v2 TargetVolume;
  v2 dCurrentVolume;
  
  uint32 SamplesPlayed;
  sound_id ID;
  playing_sound *Next;
  bool32 Loop;
};

struct audio_state
{
  memory_arena *PermArena;
  playing_sound *FirstPlayingSound;
  playing_sound *FirstFreePlayingSound;
};
#define HANDMADE_AUDIO_H
#endif
