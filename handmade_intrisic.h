#if !defined(HANDMADE_INTRINSIC_H)
#include "math.h"
inline int32 RoundReal32ToInt32(real32 Real32)
{
  int32 Result = (int32)roundf(Real32);
  return Result;
}

inline real32 SquareRoot(real32 Real32)
{
  real32 Result = sqrtf(Real32);
  return Result;
}

inline uint32 RoundReal32ToUInt32(real32 Real32)
{
  uint32 Result = (uint32)roundf(Real32);
  return Result;
}

inline int32 TruncateReal32ToInt32 (real32 Real32)
{
  int32 Result = (int32)Real32;
  return Result;
}

inline int32 FloorReal32ToInt32 (real32 Real32)
{
  int32 Result = (int32)floorf(Real32);
  return Result;
}

inline int32 CeilReal32ToInt32 (real32 Real32)
{
  int32 Result = (int32)ceilf(Real32);
  return Result;
}

struct bitscan_result
{
  bool32 Found;
  uint32 Index;
};

inline real32 AbsoluteValue(real32 Real32)
{
  real32 Result = (real32)fabs(Real32);
  return Result;
}

inline real32 Sin(real32 Angle)
{
  real32 Result = sinf(Angle);
  return Result;
}

inline real32 Cos(real32 Angle)
{
  real32 Result = cosf(Angle);
  return Result;
}

inline bitscan_result FindLastSignificantBit (uint32 Value)
{
  bitscan_result Result = {};
#if COMPILER_MSVC
  Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
  for(uint32 Test = 0; Test < 32; ++Test)
  {
    if((1 << Index) & Value)
    {
      Result.Index = Test;
      Result.Found = true;
      break;
    }
  }
#endif
  return Result;  
}

#define HANDMADE_INTRINSIC_H
#endif
