#if !defined(HANDMADE_DEBUG_H)
#define TIMED_BLOCK__(Number, ...) timed_block TimedBlock##Number(__COUNTER__, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TIMED_BLOCK_(Number, ...) TIMED_BLOCK__(Number, ##__VA_ARGS__)
#define TIMED_BLOCK(...) TIMED_BLOCK_(__LINE__, ##__VA_ARGS__)

struct debug_record
{
  char *Filename;
  char *FunctionName;
  uint32 LineNumber;
  uint32 Reserved;
  uint64 HitCount_CycleCount;
};

debug_record DebugRecordArray[];

struct timed_block
{
  debug_record *Record;
  uint64 StartCycle;
  uint32 HitCount;
  
  timed_block(int Counter, char *Filename, int LineNumber, char *FunctionName, int HitCountInit = 1)
  {
    Record = DebugRecordArray + Counter;
    HitCount = HitCountInit;
    Record->Filename = Filename;
    Record->LineNumber = LineNumber;
    Record->FunctionName = FunctionName;
    StartCycle = __rdtsc();
  }
  
  ~timed_block()
  {
    uint64 Delta = (__rdtsc() - StartCycle) | ((uint64)HitCount << 32);
    AtomicAddU64(&Record->HitCount_CycleCount, Delta);
  }
  
};

#define HANDMADE_DEBUG_H
#endif
