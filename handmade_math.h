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

struct rectangle2
{
  v2 Min;
  v2 Max;
};

inline rectangle2 RectMinMax(v2 Min, v2 Max)
{
  rectangle2 Rectangle = {};
  Rectangle.Min = Min;
  Rectangle.Max = Max;
  return Rectangle;
}

inline rectangle2 RectCenHalfDim(v2 Center, v2 HalfDim)
{
  rectangle2 Rectangle = {};
  Rectangle.Min = Center - HalfDim;
  Rectangle.Max = Center + HalfDim;
  return Rectangle;
}

inline rectangle2 RectMinDim(v2 Min, v2 Dim)
{
  rectangle2 Rectangle = {};
  Rectangle.Min = Min;
  Rectangle.Max = Min + Dim;
  return Rectangle;
}

inline rectangle2 RectCentDim(v2 Center, v2 Dim)
{
  rectangle2 Rectangle = RectCenHalfDim(Center, 0.5*Dim);
  
  return Rectangle;
}

inline bool32 IsInRectangle(rectangle2 Rectangle, v2 Test)
{
  bool32 Result = ((Test.X >= Rectangle.Min.X) &&
		   (Test.Y >= Rectangle.Min.Y) &&
		   (Test.X < Rectangle.Max.X) &&
		   (Test.Y < Rectangle.Max.Y));
  return Result;
}

inline v2 GetMaxCorner(rectangle2 Rect)
{
  v2 Result = Rect.Max;
  return Result;
}

inline v2 GetMinCorner(rectangle2 Rect)
{
  v2 Result = Rect.Min;
  return Result;
}

inline v2 GetCenter(rectangle2 Rect)
{
  v2 Result = 0.5f*(Rect.Min + Rect.Max);
  return Result;
}

#define HANDMADE_MATH_H
#endif
