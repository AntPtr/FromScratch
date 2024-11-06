#if !defined(HANDMADE_MATH_H)

struct v2
{
  union
  {
    struct
    {
      real32 X, Y;
    };
      real32 E[2];
  };
  v2 &operator*=(real32 A);
  v2 &operator+=(v2 A);
};

v2 operator+(v2 A, v2 B)
{
  v2 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  return Result;
}

v2 operator-(v2 A, v2 B)
{
  v2 Result;
  Result.X = A.X - B.X;
  Result.Y = A.Y - B.Y;
  return Result;
}

v2 operator-(v2 A)
{
  v2 Result;
  Result.X = -A.X;
  Result.Y = -A.Y;
  return Result;
}

v2 operator*(real32 A, v2 B)
{
  v2 Result;
  Result.X = A * B.X;
  Result.Y = A * B.Y;
  return Result;
}

v2 operator*(v2 A, real32 B)
{
  v2 Result = B * A;;
  return Result;
}

v2 &v2::operator*=(real32 A)
{
  *this = A * *this;
  return *this;
}

v2 &v2::operator+=(v2 A)
{
  *this = *this + A;
  return *this;
}

inline real32 Square(real32 X)
{
  real32 Result;
  Result = X*X;
  return Result;
}

inline real32 DotProduct(v2 A, v2 B)
{
  real32 Result;
  Result = A.X*B.X + A.Y*B.Y;
  return Result;
}

inline real32 LenghtSq(v2 A)
{
  real32 Result;
  Result = DotProduct(A, A);
  return Result;
}

inline int32 SignOf(int32 Value)
{
  int32 Result;
  Result = (Value >= 0) ? 1 : -1;
  return Result;
}
#define HANDMADE_MATH_H
#endif
