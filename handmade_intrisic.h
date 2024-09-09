#if !defined(HANDMADE_INTRINSIC_H)
#include "math.h"
inline int32 RoundReal32ToInt32(real32 Real32)
{
  int32 Result =(int32)roundf(Real32);
  return Result;
}

inline uint32 RoundReal32ToUInt32(real32 Real32)
{
  uint32 Result =(uint32)roundf(Real32);
  return Result;
}

inline int32 TruncateReal32ToInt32 (real32 Real32)
{
  int32 Result = (int32)Real32;
  return Result;
}

inline int32 FloorReal32ToInt32 (real32 Real32)
{
  int32 Result = (int32) floorf(Real32);
  return Result;
}


#define HANDMADE_INTRINSIC_H
#endif
