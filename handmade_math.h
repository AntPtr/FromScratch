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

struct v3
{
  union
  {
    struct
    {
      real32 X, Y, Z;
    };
    struct
    {
      real32 R, G, B;
    };
    struct
    {
      v2 XY;
      real32 Ignored0_;
    };
    struct
    {
      real32 Ignored1_;
      v2 YZ;
    };
    real32 E[3];
  };
  v3 &operator*=(real32 A);
  v3 &operator+=(v3 A);
};


struct v4
{
  union
  {
    struct
    {
      real32 X, Y, Z, W;
    };
    struct
    {
      real32 R, G, B, A;
    };
    real32 E[4];
  };
  v2 &operator*=(real32 A);
  v2 &operator+=(v2 A);
};

inline v3 V3(real32 X, real32 Y, real32 Z)
{
  v3 Result= {};
  Result.X = X;
  Result.Y = Y;
  Result.Z = Z;

  return Result;
}

inline v3 V3(v2 XY, real32 Z)
{
  v3 Result= {};
  Result.X = XY.X;
  Result.Y = XY.Y;
  Result.Z = Z;

  return Result;
}


//v2 operators
//
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

inline v3 Hadamard(v3 A, v3 B)
{
  v3 Result = {A.X*B.X, A.Y*B.Y, A.Z*B.Z};
  return Result;
}

inline real32 DotProduct(v3 A, v3 B)
{
  real32 Result;
  Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
  return Result;
}

inline real32 LenghtSq(v3 A)
{
  real32 Result;
  Result = DotProduct(A, A);
  return Result;
}

inline real32 Length(v3 A)
{
  real32 Result = SquareRoot(LenghtSq(A));
  return Result;
}

////
////

inline real32 Square(real32 X)
{
  real32 Result;
  Result = X*X;
  return Result;
}

inline real32 Lerp(real32 A, real32 t, real32 B)
{
  real32 Result = (1.0f - t)*A + t*B;
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

inline real32 Length(v2 A)
{
  real32 Result = SquareRoot(LenghtSq(A));
  return Result;
}

inline int32 SignOf(int32 Value)
{
  int32 Result;
  Result = (Value >= 0) ? 1 : -1;
  return Result;
}

inline real32 Clamp(real32 Min, real32 Value, real32 Max)
{
  real32 Result = Value;

  if(Result < Min)
  {
    Result = Min;
  }
  else if(Result > Max)
  {
    Result = Max;
  }
  return Result;
}

inline real32 Clamp01(real32 Value)
{
  real32 Result = Clamp(0.0f, Value, 1.0f);
  return Result;
}

struct rectangle2
{
  v2 Min;
  v2 Max;
};

struct rectangle3
{
  v3 Min;
  v3 Max;
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

inline rectangle2 AddRadiusTo(rectangle2 A, real32 RadiusW, real32 RadiusH)
{
  rectangle2 Result;

  Result.Min = A.Min - v2{RadiusW, RadiusH};
  Result.Max = A.Max + v2{RadiusW, RadiusH};
  
  return Result;
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

//
//v3 operators
//

v3 operator+(v3 A, v3 B)
{
  v3 Result;
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;
  Result.Z = A.Z + B.Z;
  return Result;
}

v3 operator-(v3 A, v3 B)
{
  v3 Result;
  Result.X = A.X - B.X;
  Result.Y = A.Y - B.Y;
  Result.Z = A.Z - B.Z;
  return Result;
}

v3 operator-(v3 A)
{
  v3 Result;
  Result.X = -A.X;
  Result.Y = -A.Y;
  Result.Z = -A.Z;
  return Result;
}

v3 operator*(real32 A, v3 B)
{
  v3 Result;
  Result.X = A*B.X;
  Result.Y = A*B.Y;
  Result.Z = A*B.Z;
  return Result;
}

v3 operator*(v3 A, real32 B)
{
  v3 Result = B * A;
  return Result;
}

v3 &v3::operator*=(real32 A)
{
  *this = A * *this;
  return *this;
}

v3 &v3::operator+=(v3 A)
{
  *this = *this + A;
  return *this;
}

inline v3 Clamp01(v3 Value)
{
  v3 Result;
  Result.X = Clamp01(Value.X);
  Result.Y = Clamp01(Value.Y);
  Result.Z = Clamp01(Value.Z);
  return Result;
}

inline v2 Clamp01(v2 Value)
{
  v2 Result;
  Result.X = Clamp01(Value.X);
  Result.Y = Clamp01(Value.Y);
  return Result;
}




//
//Rectangle3
//
inline rectangle3 RectMinMax(v3 Min, v3 Max)
{
  rectangle3 Rectangle = {};
  Rectangle.Min = Min;
  Rectangle.Max = Max;
  return Rectangle;
}

inline rectangle3 RectCenHalfDim(v3 Center, v3 HalfDim)
{
  rectangle3 Rectangle = {};
  Rectangle.Min = Center - HalfDim;
  Rectangle.Max = Center + HalfDim;
  return Rectangle;
}

inline rectangle3 RectMinDim(v3 Min, v3 Dim)
{
  rectangle3 Rectangle = {};
  Rectangle.Min = Min;
  Rectangle.Max = Min + Dim;
  return Rectangle;
}

inline rectangle3 RectCentDim(v3 Center, v3 Dim)
{
  rectangle3 Rectangle = RectCenHalfDim(Center, 0.5*Dim);
  
  return Rectangle;
}

inline rectangle3 AddRadiusTo(rectangle3 A, v3 Radius)
{
  rectangle3 Result;

  Result.Min = A.Min - Radius;
  Result.Max = A.Max + Radius;
  
  return Result;
}

inline rectangle3 Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;
    
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return Result;
}

inline bool32 IsInRectangle(rectangle3 Rectangle, v3 Test)
{
  bool32 Result = ((Test.X >= Rectangle.Min.X) &&
		   (Test.Y >= Rectangle.Min.Y) &&
		   (Test.Z >= Rectangle.Min.Z) &&
		   (Test.X < Rectangle.Max.X) &&
		   (Test.Y < Rectangle.Max.Y) &&
		   (Test.Z < Rectangle.Max.Z));
  return Result;
}

inline v3 GetMaxCorner(rectangle3 Rect)
{
  v3 Result = Rect.Max;
  return Result;
}

inline v3 GetMinCorner(rectangle3 Rect)
{
  v3 Result = Rect.Min;
  return Result;
}

inline v3 GetCenter(rectangle3 Rect)
{
  v3 Result = 0.5f*(Rect.Min + Rect.Max);
  return Result;
}

inline bool32 RectanglesIntersect(rectangle3 A, rectangle3 B)
{
  bool32 Result = !((B.Max.X <= A.Min.X) ||
		   (B.Min.X >= A.Max.X) ||
		   (B.Max.Y <= A.Min.Y) ||
		   (B.Min.Y >= A.Max.Y) ||
		   (B.Max.Z <= A.Min.Z) ||
		   (B.Min.Z >= A.Max.Z));
  return Result;
}

inline real32 SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
  real32 Result = N;
  if(Divisor != 0)
  {
    Result = Numerator / Divisor;
  }
  return Result;
}

inline real32 SafeRatio0(real32 Numerator, real32 Divisor)
{
  real32 Result;
  Result = SafeRatioN(Numerator, Divisor, 0.0f);
  return Result;
}

inline real32 SafeRatio1(real32 Numerator, real32 Divisor, real32 N)
{
  real32 Result;
  Result = SafeRatioN(Numerator, Divisor, 1.0f);
  return Result;
}

inline v3 GetBarycentric(rectangle3 Rect, v3 P)
{
  v3 Result;
  Result.X = SafeRatio0(P.X - Rect.Min.X, Rect.Max.X - Rect.Min.X);
  Result.Y = SafeRatio0(P.Y - Rect.Min.Y, Rect.Max.Y - Rect.Min.Y);
  Result.Z = SafeRatio0(P.Z - Rect.Min.Z, Rect.Max.Z - Rect.Min.Z);

  return Result;
}

inline v2 GetBarycentric(rectangle2 Rect, v2 P)
{
  v2 Result;
  Result.X = SafeRatio0(P.X - Rect.Min.X, Rect.Max.X - Rect.Min.X);
  Result.Y = SafeRatio0(P.Y - Rect.Min.Y, Rect.Max.Y - Rect.Min.Y);
  
  return Result;
}

inline rectangle2 ToRectangleXY(rectangle3 Rect)
{
  rectangle2 Result;
  Result.Min = Rect.Min.XY;
  Result.Max = Rect.Max.XY;

  return Result;
}

inline v2 V2i(int32 X, int32 Y)
{
  v2 Result = {(real32)X, (real32)Y};
  return Result;
}

inline v2 V2u(uint32 X, uint32 Y)
{
  v2 Result = {(real32)X, (real32)Y};
  return Result;
}

#define HANDMADE_MATH_H
#endif
