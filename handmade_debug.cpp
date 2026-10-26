#include <stdio.h>

global_variable real32 AtY;
global_variable real32 LeftEdge;
global_variable real32 FontScale;
global_variable font_id FontID;
global_variable real32 GlobalWidth;
global_variable real32 GlobalHeight;

internal void DEBUGReset(game_assets *Assets, uint32 Width, uint32 Height)
{
  asset_vector MatchVector = {};
  asset_vector WeightVector = {};
  GlobalWidth = (real32)Width; 
  GlobalHeight = (real32)Height;
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

bool32 MyStrCmp(const char *A, const char *B)
{
    while(*A && (*A == *B))
    {
        ++A;
        ++B;
    }

    return (*A == *B);
}

internal void DEBUGOverlay(game_memory *Memory, game_input *Input)
{
#if H_INTERNAL
  debug_state *DebugState = (debug_state *)Memory->DebugStorage;
  if(DebugState && DEBUGRenderGroup)
  {
    v2 MouseP = v2{Input->MouseX,Input->MouseY};

    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
    {
      DebugState->Paused = !DebugState->Paused;
    }
    DEBUGTextLine("DEBUG CYCLE COUNT:\n");
    //DEBUGTextLine("/", v4{0.4f, 0.0f, 0.0f, 1.0f});
    //DEBUGTextLine("Ciao Roxie", v4{0.024f, 0.4f, 0.169f, 1.0f});
    render_group *RenderGroup = DEBUGRenderGroup;
    loaded_font *Font = PushFont(RenderGroup, FontID);
    if(Font)
    {
      hha_font *Info =  GetFontInfo(RenderGroup->Assets, FontID); 
#if 0
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
	  UpdateDebugStatistic(&CycleCount, (uint32)Counter->Snapshots[SnapshotIndex].CycleCount);

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

	if(Counter->BlockName)
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
	      real32 ThisProportion = Scale*(real32)Counter->Snapshots[SnapIndex].CycleCount;
	      real32 ThisHeight = ChartHeight*ThisProportion;
	      uint64 Cycles = Counter->Snapshots[SnapIndex].CycleCount;
	      
	      PushRect(RenderGroup, v3{ChartLeft + (real32)SnapIndex*BarWidth + 0.5f*BarWidth, ChartMinY + 0.5f*ThisHeight, 0.0f}, v2{BarWidth, ThisHeight}, v4{ThisProportion, 1.0f, 0.0f, 1.0f});
	    }
	  }
      
	  char TextBuffer[256];
	  _snprintf_s(TextBuffer, sizeof(TextBuffer), "%s(%d): %ucy %uh %ucy/h", Counter->BlockName, Counter->LineNumber,
		      (uint32)CycleCount.Avg, (uint32)HitCount.Avg, (uint32)(CycleOverHit.Avg));
	  DEBUGTextLine(TextBuffer);
	}
      }
#endif
      if(DebugState->FrameCount)
      {
	char TextBuffer[256];
	_snprintf_s(TextBuffer, sizeof(TextBuffer), "Last Frame Time: %0.02fms", DebugState->Frames[DebugState->FrameCount - 1].WallSecondsElapsed*1000);
	  DEBUGTextLine(TextBuffer);
      }
      
      real32 LaneWidth = 8.0f;
      uint32 LaneCount = DebugState->FrameBarLaneCount;
      real32 BarWidth = LaneWidth*LaneCount;
      real32 BarSpacing = BarWidth + 4.0f;
      real32 ChartLeft = LeftEdge + 10.0f; 
      real32 ChartHeight = 300.0f;
      real32 ChartWidth = BarSpacing*DebugState->FrameCount;
      real32 ChartMinY = -0.5f*GlobalHeight + 10.0f;
      real32 Scale = ChartHeight*DebugState->FrameBarScale;
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
      uint32 MaxFrame = DebugState->FrameCount;
      if(MaxFrame > 10)
      {
	MaxFrame = 10;
      }



      for(uint32 FrameIndex = 0; FrameIndex < MaxFrame; ++FrameIndex)
      {
	debug_frame *Frame = DebugState->Frames + DebugState->FrameCount - (FrameIndex + 1);
	real32 StackX = ChartLeft + BarSpacing*FrameIndex;
	real32 StackY = ChartMinY;
	debug_frame_region *HoveredRegion = 0;
	debug_record *HoveredRecord = 0;
	real32 HoveredHeight = FLT_MAX;

	for(uint32 RegionIndex = 0; RegionIndex < Frame->RegionCount; ++RegionIndex)
	{
	  debug_frame_region *Region = Frame->Regions + RegionIndex;
	  debug_record *Record = Region->Record;

	  v3 Color = Colors[RegionIndex%ArrayCount(Colors)];
	  real32 ThisMinY = StackY + Scale*Region->MinT;
	  real32 ThisMaxY = StackY + Scale*Region->MaxT;
	  	  
	  rectangle2 RegionRect = RectMinMax(v2{StackX + LaneWidth*Region->LaneIndex, ThisMinY},
					     v2{StackX + LaneWidth*(Region->LaneIndex + 1), ThisMaxY});
	  PushRect(RenderGroup, RegionRect, 0.0f, v4{Color, 1.0f});

	  
	  
	  if(IsInRectangle(RegionRect, MouseP))
	  {
	    real32 RegionHeight = AbsoluteValue(RegionRect.Max.y - RegionRect.Min.y);

	    if(RegionHeight < HoveredHeight)
	    {
	      HoveredHeight = RegionHeight;
	      HoveredRegion = Region;
	      HoveredRecord = Record;
	    }

	    if(HoveredRegion && HoveredRecord)
	   {
	     char TextBuffer[256];

	     _snprintf_s(
			 TextBuffer,
			 sizeof(TextBuffer),
			 "%32s: %10ucy [%s(%d)]",
			 HoveredRecord->BlockName,
			 (uint32)HoveredRegion->CycleCount,
			 HoveredRecord->Filename,
			 HoveredRecord->LineNumber
			 );

	     DEBUGTextLine(TextBuffer);
	   }	    
	  }   
	}
      } 
      PushRect(RenderGroup, v3{ChartLeft + 0.5f*ChartWidth, ChartMinY + ChartHeight, 0.0f}, v2{ChartWidth, 1.0f}, v4{1.0f, 1.0f, 1.0f, 1.0f});
    }
  }
#endif
}

#define DebugRecords_Main_Count __COUNTER__

global_variable debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
#if 0
void UpdateDebugRecords(debug_state *DebugState, uint32 CounterCount, debug_record *Record)
{
  for(uint32 CounterIndex = 0; CounterIndex < CounterCount; ++CounterIndex)
  {
    debug_record *Source = Record + CounterIndex;
    debug_counter_state *Dest = DebugState->CounterState + DebugState->CounterCount++;

    uint64 HitCount_CycleCount = AtomicExchangeU64(&Source->HitCount_CycleCount, 0);
    Dest->FileName = Source->Filename;
    Dest->BlockName = Source->BlockName;
    Dest->LineNumber = Source->LineNumber;
    Dest->Snapshots[DebugState->SnapshotIndex].HitCount = (uint32)(HitCount_CycleCount >> 32);
    Dest->Snapshots[DebugState->SnapshotIndex].CycleCount = (uint32)(HitCount_CycleCount & 0xFFFFFFFF);
  }
}
#endif

inline uint32 GetLaneFromTheardIndex(debug_state *DebugState, uint32 TheardIndex)
{
  uint32 Result = 0;
  return Result;
}

internal debug_theard *GetDebugTheard(debug_state *DebugState, uint32 TheardID)
{
  debug_theard *Result = 0;
  for(debug_theard *Theard = DebugState->FirstTheard; Theard; Theard = Theard->Next)
  {
    if(Theard->TheardID == TheardID)
    {
      Result = Theard;
      break;
    }
  }

  if(!Result)
  {
    Result = PushStruct(&DebugState->CollateArena, debug_theard);
    Result->Next = DebugState->FirstTheard;
    Result->TheardID = TheardID;
    Result->LaneIndex = DebugState->FrameBarLaneCount++;
    Result->FirstOpenBlock = 0;
    DebugState->FirstTheard = Result;
  }

  return Result;
}

internal debug_frame_region *AddRegion(debug_state *DebugState, debug_frame *CurrentFrame)
{
  Assert(CurrentFrame->RegionCount < MAX_REGION_PER_FRAME);
  debug_frame_region *Result = CurrentFrame->Regions + CurrentFrame->RegionCount++;

  return Result;
}

internal void CollateDebugRecord(debug_state *DebugState, uint32 IvalidEventArrayIndex)
{
  DebugState->Frames = PushArray(&DebugState->CollateArena, MAX_DEBUG_EVENT_ARRAY_COUNT*4, debug_frame);
  DebugState->FrameCount = 0;
  DebugState->FrameBarLaneCount = 0;
  DebugState->FrameBarScale = 1.0f / 60000000.0f;
  
  debug_frame *CurrentFrame = 0;
  for(uint32 EventArrayIndex = IvalidEventArrayIndex + 1; ; ++EventArrayIndex)
  {
    if(EventArrayIndex == MAX_DEBUG_EVENT_ARRAY_COUNT)
    {
      EventArrayIndex = 0;
    }
    
    if(EventArrayIndex == IvalidEventArrayIndex)
    {
      break;
    }

    for(uint32 EventIndex = 0; EventIndex < GlobalDebugTable->EventCount[EventArrayIndex]; ++EventIndex)
    {
      debug_event *Event = GlobalDebugTable->Events[EventArrayIndex] + EventIndex;
      debug_record *Source = GlobalDebugTable->Records[Event->TranslationUnit] + Event->DebugRecordIndex;
      if(Event->Type == DebugEvent_FrameMarker && (Event->Clock != 0))
      {
	if(CurrentFrame)
	{
	  CurrentFrame->EndClock = Event->Clock;
	  CurrentFrame->WallSecondsElapsed = Event->SecondsElapsed;
	  ++DebugState->FrameCount;
	  
	  real32 ClockRange = (real32)(CurrentFrame->EndClock - CurrentFrame->BeginClock);
#if 0
	  if(ClockRange > 0.0f)
	  {
	    real32 FrameBarScale = 1.0f/ClockRange;
	    if(DebugState->FrameBarScale > FrameBarScale)
	    {
	      DebugState->FrameBarScale = FrameBarScale;
	    }
	  }
#endif
	}
	
	CurrentFrame = DebugState->Frames + DebugState->FrameCount;
	CurrentFrame->BeginClock = Event->Clock;
	CurrentFrame->RegionCount = 0;
	CurrentFrame->EndClock = 0;
	CurrentFrame->Regions = PushArray(&DebugState->CollateArena, MAX_REGION_PER_FRAME, debug_frame_region);
      }
      else if(CurrentFrame)
      {
	uint32 FrameIndex = DebugState->FrameCount - 1;
	debug_theard *Theard = GetDebugTheard(DebugState, Event->TC.TheardIndex);
	uint64 RelativeClock = Event->Clock - CurrentFrame->BeginClock;
	if(Event->Type == DebugEvent_BeginBlock)
	{
	  open_debug_block *DebugBlock = DebugState->FirstFreeBlock;
	  if(DebugBlock)
	  {
	    DebugState->FirstFreeBlock = DebugBlock->NextFree;
	  }
	  else
	  {
	    DebugBlock = PushStruct(&DebugState->CollateArena, open_debug_block);
	  }
	  DebugBlock->StartingFrameIndex = FrameIndex;
	  DebugBlock->OpeningEvent = Event;
	  DebugBlock->Parent = Theard->FirstOpenBlock;
	  Theard->FirstOpenBlock = DebugBlock;
	  DebugBlock->NextFree = 0;
	}
	else if(Event->Type == DebugEvent_EndBlock)
	{
	  if(Theard->FirstOpenBlock)
	  {
	    open_debug_block *MatchingBlock = Theard->FirstOpenBlock;
	    debug_event *OpeningEvent = MatchingBlock->OpeningEvent;
	    if(OpeningEvent->TC.TheardIndex == Event->TC.TheardIndex &&
	       OpeningEvent->DebugRecordIndex == Event->DebugRecordIndex &&
	       OpeningEvent->TranslationUnit == Event->TranslationUnit)
	      {

		if(MatchingBlock->StartingFrameIndex == FrameIndex)
		{

		  if(Theard->FirstOpenBlock->Parent == 0)
		  {
		    real32 MinT = (real32)(OpeningEvent->Clock - CurrentFrame->BeginClock);
		    real32 MaxT = (real32)(Event->Clock - CurrentFrame->BeginClock);
		    real32 ThresholdT = 0.01f;
		    if((MaxT - MinT) > ThresholdT)
		    {
		      debug_frame_region *Region = AddRegion(DebugState, CurrentFrame);
		      Region->LaneIndex = Theard->LaneIndex;
		      Region->Record = Source;
		      Region->CycleCount = Event->Clock - OpeningEvent->Clock;
		      Region->MinT = MinT; 
		      Region->MaxT = MaxT;
		    }
		  }
		}
		else
		{
		  
		}
		Theard->FirstOpenBlock->NextFree = DebugState->FirstFreeBlock;
		DebugState->FirstFreeBlock = Theard->FirstOpenBlock;
		Theard->FirstOpenBlock = MatchingBlock->Parent;
	      }
	    else
	    {
	    
	    }
	  }
	}
	else
	{
	  //Invalid event type
	}
      }
    }
  }
#if 0
  debug_counter_state *CounterArray[MAX_TRANSLATION_UNITS];
  debug_counter_state *CurrentCounter = DebugState->CounterState;
  uint32 TotalRecordCount = 0;
  for(uint32 UnitIndex = 0; UnitIndex < MAX_TRANSLATION_UNITS; ++UnitIndex)
  {
    CounterArray[UnitIndex] = CurrentCounter;
    TotalRecordCount += GlobalDebugTable->RecordCount[UnitIndex];
    CurrentCounter += GlobalDebugTable->RecordCount[UnitIndex];
  }
  
  DebugState->CounterCount = TotalRecordCount;
  
  for(uint32 CounterIndex = 0; CounterIndex < DebugState->CounterCount; ++CounterIndex)
  {
    debug_counter_state *Dest = DebugState->CounterState + CounterIndex;   
    Dest->Snapshots[DebugState->SnapshotIndex].HitCount = 0;
    Dest->Snapshots[DebugState->SnapshotIndex].CycleCount = 0;
  }

  for(uint32 EventIndex = 0; EventIndex < EventCount; ++EventIndex)
  {
    debug_event *Event = Events + EventIndex;
    debug_record *Source = GlobalDebugTable->Records[Event->TranslationUnit] + Event->DebugRecordIndex;
    debug_counter_state *Dest = CounterArray[Event->TranslationUnit] + Event->DebugRecordIndex;   
    Dest->FileName = Source->Filename;
    Dest->LineNumber = Source->LineNumber;
    Dest->BlockName = Source->BlockName;
    
    if(Event->Type == DebugEvent_BeginBlock)
    {
      ++Dest->Snapshots[DebugState->SnapshotIndex].HitCount;
      Dest->Snapshots[DebugState->SnapshotIndex].CycleCount -= Event->Clock;
    }
    else if(Event->Type ==  DebugEvent_EndBlock)
    {
      Dest->Snapshots[DebugState->SnapshotIndex].CycleCount += Event->Clock;
    }
  }
#endif
}


#if defined __cplusplus
extern "C"
#endif
DEBUG_FRAME_END(DEBUGFrameEnd)
{
  GlobalDebugTable->RecordCount[0] = DebugRecords_Main_Count;
  GlobalDebugTable->RecordCount[2] = 0;
  
  ++GlobalDebugTable->CurrentEventArrayIndex;
  if(GlobalDebugTable->CurrentEventArrayIndex >= ArrayCount(GlobalDebugTable->Events))
  {
    GlobalDebugTable->CurrentEventArrayIndex = 0;
  }

  uint64 ArrayIndex_EventIndex = AtomicExchangeU64(&GlobalDebugTable->EventArrayIndex_EventIndex, (uint64)GlobalDebugTable->CurrentEventArrayIndex << 32);

  uint32 EventArrayIndex = ArrayIndex_EventIndex >> 32;
  uint32 EventCount = ArrayIndex_EventIndex & 0xFFFFFFFF;
  GlobalDebugTable->EventCount[EventArrayIndex] = EventCount;
  
  debug_state *DebugState = (debug_state *)Memory->DebugStorage;
  uint32 GameUpdatedBeginCount = 0;
  uint32 GameUpdatedEndCount = 0;
  
  if(DebugState)
  {
    if(!DebugState->Initialized)
    {
      InitializeArena(&DebugState->CollateArena, Memory->DebugStorageSize - sizeof(debug_state), DebugState + 1);
      DebugState->CollateTemp = BeginTemporaryMemory(&DebugState->CollateArena);
    }

    if(!DebugState->Paused)
    {
      EndTemporaryMemory(DebugState->CollateTemp);
      DebugState->CollateTemp = BeginTemporaryMemory(&DebugState->CollateArena);
      
      DebugState->FirstTheard = 0;
      DebugState->FirstFreeBlock = 0;
      
      CollateDebugRecord(DebugState, GlobalDebugTable->CurrentEventArrayIndex);
    }
  }

  return GlobalDebugTable;
}

