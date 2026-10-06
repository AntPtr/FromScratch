#include <stdio.h>

global_variable real32 AtY;
global_variable real32 LeftEdge;
global_variable real32 FontScale;
global_variable font_id FontID;

void UpdateDebugRecords(debug_state *DebugState, uint32 CounterCount, debug_record *Record)
{
  for(uint32 CounterIndex = 0; CounterIndex < CounterCount; ++CounterIndex)
  {
    debug_record *Source = Record + CounterIndex;
    debug_counter_state *Dest = DebugState->CounterState + DebugState->CounterCount++;

    uint64 HitCount_CycleCount = AtomicExchangeU64(&Source->HitCount_CycleCount, 0);
    Dest->FileName = Source->Filename;
    Dest->FunctionName = Source->FunctionName;
    Dest->LineNumber = Source->LineNumber;
    Dest->Snapshots[DebugState->SnapshotIndex].HitCount = (uint32)(HitCount_CycleCount >> 32);
    Dest->Snapshots[DebugState->SnapshotIndex].CycleCount = (uint32)(HitCount_CycleCount & 0xFFFFFFFF);
  }
}

internal void DEBUGReset(game_assets *Assets, uint32 Width, uint32 Height)
{
  asset_vector MatchVector = {};
  asset_vector WeightVector = {};
  MatchVector.E[Tag_FontType] = (real32)FontType_Debug;
  WeightVector.E[Tag_FontType] = 1.0f;
  FontID = BestMatchFont(Assets, Asset_Fonts, &MatchVector, &WeightVector);
  FontScale = 1.0f;
  hha_font *Info = GetFontInfo(Assets, FontID);
  AtY = 0.5f*Height - FontScale*GetBaseLineY(Info);
  LeftEdge = -0.5f*Width; 
  Orthographic(DEBUGRenderGroup, Width, Height, 1.0f);  
}

internal void DEBUGTextLine(char *String, v4 Color = v4{1.0, 1.0, 1.0, 1.0})
{
  if(DEBUGRenderGroup)
  {
    render_group *RenderGroup = DEBUGRenderGroup;
    loaded_font *Font = PushFont(RenderGroup, FontID);
    if(Font)
    {
      hha_font *Info = GetFontInfo(RenderGroup->Assets, FontID);
      asset *DebugAsset = RenderGroup->Assets->Assets + FontID.Value;
      real32 AtX = LeftEdge;
      uint32 PrevCodePoint = 0;
      if(String[0] == '/')
      {
	uint32 CodePoint = 0x706B;
	
	real32 AdvanceX = GetHorizonatalAdvanceForPair(Info, Font, PrevCodePoint, CodePoint);
	AtX += AdvanceX*FontScale;
	bitmap_id BitmapID = GetBitmapForGlyph(RenderGroup->Assets, Info, Font, CodePoint);
	hha_bitmap *InfoBitmap = GetBitmapInfo(RenderGroup->Assets, BitmapID);
	//CharDim = FontScale*(real32)InfoBitmap->Dim[0];
	PushBitmap(RenderGroup, BitmapID, v3{AtX, AtY, 0}, FontScale*(real32)InfoBitmap->Dim[1], Color);  
	PrevCodePoint = CodePoint;
      }
      else
      {
	for(char *At = String; *At; ++At)
        {
	  uint32 CodePoint = *At;
	
	  //real32 CharDim = 10.0f;
	  real32 AdvanceX = GetHorizonatalAdvanceForPair(Info, Font, PrevCodePoint, CodePoint);
	  AtX += AdvanceX*FontScale;
	  if(CodePoint !=  ' ')
	  {
	    bitmap_id BitmapID = GetBitmapForGlyph(RenderGroup->Assets, Info, Font, CodePoint);
	    hha_bitmap *InfoBitmap = GetBitmapInfo(RenderGroup->Assets, BitmapID);
	    //CharDim = FontScale*(real32)InfoBitmap->Dim[0];
	    PushBitmap(RenderGroup, BitmapID, v3{AtX, AtY, 0}, FontScale*(real32)InfoBitmap->Dim[1], Color);
	  }
	  PrevCodePoint = CodePoint;
	}
      }
      
      AtY -= GetLineAdvance(Info)*FontScale;
    }
  }
}

struct debug_statistics
{
  real64 Min;
  real64 Max;
  real64 Avg;
  uint32 Count;
};

inline void BeginDebugStatistic(debug_statistics *Stats)
{
  Stats->Max = -Real32Maximum;
  Stats->Min = Real32Maximum;
  Stats->Avg = 0.0f;
  Stats->Count = 0;
}

inline void UpdateDebugStatistic(debug_statistics *Stats, real64 Value)
{
  if(Stats->Max <= Value)
  {
    Stats->Max = Value;
  }
  if(Stats->Min >= Value)
  {
    Stats->Min = Value;
  }
  Stats->Avg += Value;
  ++Stats->Count;
}

inline void EndDebugStatistic(debug_statistics *Stats)
{
  if(Stats->Count != 0)
  {
    Stats->Avg /= (real64)Stats->Count;
  }
  else
  {
    Stats->Min = Stats->Max = 0;
  }
}

internal void DEBUGOverlay(game_memory *Memory)
{
#if H_INTERNAL
  debug_state *DebugState = (debug_state *)Memory->DebugStorage;
  if(DebugState && DEBUGRenderGroup)
  {
    DEBUGTextLine("DEBUG CYCLE COUNT:\n");
    //DEBUGTextLine("/", v4{0.4f, 0.0f, 0.0f, 1.0f});
    //DEBUGTextLine("Ciao Roxie", v4{0.024f, 0.4f, 0.169f, 1.0f});
    render_group *RenderGroup = DEBUGRenderGroup;
    loaded_font *Font = PushFont(RenderGroup, FontID);
    if(Font)
    {
      hha_font *Info =  GetFontInfo(RenderGroup->Assets, FontID); 
      for(uint32 CounterIndex = 0; CounterIndex < DebugState->CounterCount; ++CounterIndex)
      {
	debug_counter_state *Counter = DebugState->CounterState + CounterIndex;
	/*
	uint32 HitCount = Counter->Snapshots[0].HitCount;
	uint32 CycleCount = Counter->Snapshots[0].CycleCount;
	*/
	debug_statistics HitCount, CycleCount, CycleOverHit;
	BeginDebugStatistic(&HitCount);
	BeginDebugStatistic(&CycleCount);
	BeginDebugStatistic(&CycleOverHit);
	for(uint32 SnapshotIndex = 0; SnapshotIndex < DEBUG_SNAPSHOT_COUNT; ++SnapshotIndex)
	{
	  UpdateDebugStatistic(&HitCount, Counter->Snapshots[SnapshotIndex].HitCount);
	  UpdateDebugStatistic(&CycleCount, Counter->Snapshots[SnapshotIndex].CycleCount);

	  real64 HOC = 0.0f;
	  if(Counter->Snapshots[SnapshotIndex].HitCount)
	  {
	    HOC = (real64)Counter->Snapshots[SnapshotIndex].CycleCount /  (real64)Counter->Snapshots[SnapshotIndex].HitCount;
	  }
	  UpdateDebugStatistic(&CycleOverHit, HOC);
	}
      
	EndDebugStatistic(&HitCount);
	EndDebugStatistic(&CycleCount);
	EndDebugStatistic(&CycleOverHit);

	if(Counter->FunctionName)
	{
	  if(CycleCount.Max > 0.0f)
	  {
	    real32 BarWidth = 4.0f;
	    real32 ChartLeft = 0.0f;
	    real32 ChartMinY = AtY;
	    real32 ChartHeight = Info->AscenderHeight*FontScale;
	    real32 Scale = 1.0f / (real32)CycleCount.Max;
	    for(uint32 SnapIndex = 0; SnapIndex < DEBUG_SNAPSHOT_COUNT; ++SnapIndex)
	    {
	      real32 ThisProportion = Scale*Counter->Snapshots[SnapIndex].CycleCount;
	      real32 ThisHeight = ChartHeight*ThisProportion;
	      PushRect(RenderGroup, v3{ChartLeft + (real32)SnapIndex*BarWidth + 0.5f*BarWidth, ChartMinY + 0.5f*ThisHeight, 0.0f}, v2{BarWidth, ThisHeight}, v4{ThisProportion, 1.0f, 0.0f, 1.0f});
	    }
	  }
      
	  char TextBuffer[256];
	  _snprintf_s(TextBuffer, sizeof(TextBuffer), "%s(%d): %ucy %uh %ucy/h", Counter->FunctionName, Counter->LineNumber,
		      (uint32)CycleCount.Avg, (uint32)HitCount.Avg, (uint32)(CycleOverHit.Avg));
	  DEBUGTextLine(TextBuffer);
	}
      }

      real32 BarWidth = 8.0f;
      real32 BarSpacing = 10.0f;
      real32 ChartLeft = LeftEdge + 10.0f; 
      real32 ChartHeight = 300.0f;
      real32 ChartWidth = BarSpacing*(real32)DEBUG_SNAPSHOT_COUNT;
      real32 ChartMinY = AtY - (ChartHeight + 400.0f);
      real32 Scale = 1.0f / 0.03333f;
      v3 Colors[] =
      {
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 1},
	{1, 1, 1},
	{0, 1, 1},
	{1, 0, 1},
	{1, 0.5f, 0},
	{0.5f, 0, 1},
	{0.5f, 1, 0},
	{0, 1, 0.5f},
	{0.5f, 0, 0.5f},
	{0, 0.5f, 1},
	{0, 0.5f, 1},
      };
      
      for(uint32 SnapshotIndex = 0; SnapshotIndex < DEBUG_SNAPSHOT_COUNT; ++SnapshotIndex)
      {
	debug_frame_timestamp *TimestampInfo = DebugState->FrameEndInfos + SnapshotIndex;
	real32 PrevTimestampSeconds = 0.0f;
	real32 StackY = ChartMinY;
	for(uint32 TimestampIndex = 0; TimestampIndex < TimestampInfo->TimestampCount; ++TimestampIndex)
	{
	  v3 Color = Colors[TimestampIndex%ArrayCount(Colors)];
	  debug_timestamp *Timestamp = TimestampInfo->Timestamps + TimestampIndex;
	  real32 ThisTimeStampSeconds = Timestamp->Seconds - PrevTimestampSeconds;
	  PrevTimestampSeconds = Timestamp->Seconds;
	  real32 ThisProportion = Scale*ThisTimeStampSeconds;
	  real32 ThisHeight = ChartHeight*ThisProportion;
	  PushRect(RenderGroup, v3{ChartLeft + (real32)SnapshotIndex*BarSpacing + 0.5f*BarWidth, StackY + 0.5f*ThisHeight, 0.0f}, v2{BarWidth, ThisHeight}, v4{Color, 1.0f});
	  StackY += ThisHeight;
	}
      }
      PushRect(RenderGroup, v3{ChartLeft + 0.5f*ChartWidth, ChartMinY + ChartHeight, 0.0f}, v2{ChartWidth, 1.0f}, v4{1.0f, 1.0f, 1.0f, 1.0f});
    }
  }
#endif
}

debug_record DebugRecordArray[__COUNTER__];

#if defined __cplusplus
extern "C"
#endif
DEBUG_FRAME_END(DEBUGFrameEnd)
{
  debug_state *DebugState = (debug_state *)Memory->DebugStorage;
  if(DebugState)
  {
    DebugState->CounterCount = 0;
    UpdateDebugRecords(DebugState, ArrayCount(DebugRecords_Main), DebugRecords_Main);

    DebugState->FrameEndInfos[DebugState->SnapshotIndex] = *Info;
    ++DebugState->SnapshotIndex;
    if(DebugState->SnapshotIndex > DEBUG_SNAPSHOT_COUNT)
    {
      DebugState->SnapshotIndex = 0;
    }
  }
}

