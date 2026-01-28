struct bilinear_sample
{
  uint32 A, B, C, D;
};

inline bilinear_sample BilinearSample(loaded_bitmap *Texture, int32 X, int32 Y)
{
  bilinear_sample Result;
  uint8 *TexelPtr = ((uint8 *)Texture->Memory) + Y*Texture->Pitch + X*sizeof(uint32);
  Result.A = *(uint32 *)(TexelPtr);
  Result.B = *(uint32 *)(TexelPtr + sizeof(uint32));
  Result.C = *(uint32 *)(TexelPtr + Texture->Pitch);
  Result.D = *(uint32 *)(TexelPtr + Texture->Pitch + sizeof(uint32));
  return Result;
}

inline v4 SRGB255ToLinear1(v4 C)
{
  v4 Result;
  real32 Inv255 = 1.0f / 255.0f;
  
  Result.r = Square(Inv255*C.r);
  Result.g = Square(Inv255*C.g);
  Result.b = Square(Inv255*C.b);
  Result.a = Inv255*C.a;

  return Result;
}

inline v4 UnscaleAndBiasNormal(v4 Normal)
{
  v4 Result;
  real32 Inv255 = 1.0f / 255.0f;
  
  Result.x = -1.0f + 2.0f*(Normal.x*Inv255);
  Result.y = -1.0f + 2.0f*(Normal.y*Inv255);
  Result.z = -1.0f + 2.0f*(Normal.z*Inv255);

  Result.w = Normal.w*Inv255;

  return Result;
}

inline v4 Linear1ToSRGB255(v4 C)
{
  v4 Result;
  real32 One255 = 255.0f;
  
  Result.r = One255*SquareRoot(C.r);
  Result.g = One255*SquareRoot(C.g);
  Result.b = One255*SquareRoot(C.b);
  Result.a = One255*C.a;

  return Result;
}

inline v4 Unpack4Bytes(uint32 Value)
{
  v4 Result;
  Result = v4{(real32)((Value >> 16) & 0xFF),
	      (real32)((Value >> 8) & 0xFF),
	      (real32)((Value >> 0) & 0xFF),
	      (real32)((Value >> 24) & 0xFF)};

  return Result;
}

inline v4 SRGBBilinearBlend(bilinear_sample TexelSample, real32 fX, real32 fY)
{
  v4 TexelA = Unpack4Bytes(TexelSample.A);		
  v4 TexelB = Unpack4Bytes(TexelSample.B);
  v4 TexelC = Unpack4Bytes(TexelSample.C); 		
  v4 TexelD = Unpack4Bytes(TexelSample.D);

  TexelA = SRGB255ToLinear1(TexelA);
  TexelB = SRGB255ToLinear1(TexelB);
  TexelC = SRGB255ToLinear1(TexelC);
  TexelD = SRGB255ToLinear1(TexelD);

  v4 Texel = Lerp(Lerp(TexelA, fX, TexelB), fY, Lerp(TexelC, fX, TexelD));

  return Texel;
}

internal void DrawRectangle(loaded_bitmap *Buffer, v2 vMin, v2 vMax, v4 Color, rectangle2i ClipRect, bool32 Even)
{
  real32 R = Color.r;
  real32 G = Color.g;
  real32 B = Color.b;
  real32 A = Color.a;

  rectangle2i FillRect;
  
  FillRect.MinX = RoundReal32ToInt32(vMin.x);
  FillRect.MaxX = RoundReal32ToInt32(vMax.x);
  FillRect.MinY = RoundReal32ToInt32(vMin.y);
  FillRect.MaxY = RoundReal32ToInt32(vMax.y);

  FillRect = Intersect(FillRect, ClipRect);
  if(!Even == (FillRect.MinY & 1))
  {
    FillRect.MinY += 1;
  }
 
  uint32 Color32 = (uint32)(RoundReal32ToUInt32(255 * A) << 24 |
			    RoundReal32ToUInt32(255 * R) << 16 |
			    RoundReal32ToUInt32(255 * G) << 8 |
			    RoundReal32ToUInt32(255 *B) << 0);
  
  uint8* Row = ((uint8 *)Buffer->Memory + FillRect.MinX*BITMAP_BYTES_PER_PIXEL + FillRect.MinY*Buffer->Pitch); 

  for(int Y = FillRect.MinY; Y < FillRect.MaxY; Y += 2)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = FillRect.MinX; X < FillRect.MaxX; ++X)
    {
      *Pixel++ = Color32;
    }
    Row += 2*Buffer->Pitch;
  }
}

inline v3 SampleFromEnvMap(v2 ScreenSpaceUV, v3 SampleDirection, real32 Roughness, environment_map *Map, real32 DistanceFromMapInZ)
{
  uint32 LODIndex = (uint32)(Roughness*((real32)ArrayCount(Map->LOD) - 1) + 0.5f);
  Assert(LODIndex < ArrayCount(Map->LOD));
  loaded_bitmap *LOD = &Map->LOD[LODIndex];

  //Assert(SampleDirection.y > 0.0f);

  real32 UVPerMeter = 0.01f;
  real32 C = (DistanceFromMapInZ*UVPerMeter) / SampleDirection.y;
  v2 Offset = C * v2{SampleDirection.x, SampleDirection.z};
  v2 UV = ScreenSpaceUV + Offset;

  UV.x = Clamp01(UV.x);
  UV.y = Clamp01(UV.y);
  
  real32 tX = ((UV.x*(real32)(LOD->Width - 2)));
  real32 tY = ((UV.y*(real32)(LOD->Height - 2)));
  
  int32 X = (int32)tX;
  int32 Y = (int32)tY;

  real32 fX = tX - (real32)X;
  real32 fY = tY - (real32)Y;

  bilinear_sample Sample = BilinearSample(LOD, X, Y);
  v3 Result =  SRGBBilinearBlend(Sample, fX, fY).xyz;
  
  return Result;
}

#define PushRenderElement(Group, Type) (Type*)PushRenderElement_(Group, sizeof(Type), RenderGroupEntryType_##Type)
inline void *PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
  void *Result = 0;
  Size += sizeof(render_group_entry_header);
  
  if((Group->PushBufferSize + Size) < Group->MaxPushBufferSize)
  {
     render_group_entry_header *Header = (render_group_entry_header*)(Group->PushBufferBase + Group->PushBufferSize);
    Header->Type = Type;
    Result = (uint8*)Header + sizeof(*Header);
    Group->PushBufferSize += Size;
  }
  else
  {
    InvalidCodePath;
  }
  
  return Result;
}
struct entity_basis_result
{
  v2 P;
  real32 Scale;
  bool32 Valid;
};

entity_basis_result GetRenderEntityBasisP(render_transform *Transform, v3 OriginalP)
{
  entity_basis_result Result = {};
  
  v3 P = OriginalP + Transform->OffsetP;

  if(!Transform->Orthographic)
  {
    real32 DistanceToPz = Transform->CameraDistanceAboveTarget - P.z;
    real32 NearClipPlane = 0.20f;

    v3 RawXY = V3(P.xy, 1.0f);
    if(DistanceToPz > NearClipPlane)
    {
      v3 ProjectedXY = (1.0f / DistanceToPz)*Transform->FocalLength*RawXY;
      Result.P = Transform->ScreenCenter + Transform->MetersToPixels*ProjectedXY.xy + v2{0.0f, Result.Scale};
      Result.Scale = Transform->MetersToPixels*ProjectedXY.z;
      Result.Valid = true;
    }
  }
  else
  {
    Result.P = Transform->ScreenCenter + Transform->MetersToPixels*P.xy;
    Result.Scale = Transform->MetersToPixels;
    Result.Valid = true;
  }
  return Result;
}

inline void PushBitmap(render_group *Group, loaded_bitmap *Bitmap, v3 Offset, real32 Height, v4 Color = v4{1, 1, 1, 1})
{
  v2 Size = v2{Height*Bitmap->WidthOverHeight, Height};
  v2 Align = Hadamard(Bitmap->AlignPercentage, Size);
  v3 P = Offset - V3(Align, 0);
  
  entity_basis_result Basis =  GetRenderEntityBasisP(&Group->Transform, P);
  if(Basis.Valid)
  {
    render_entry_bitmap *Entry = PushRenderElement(Group, render_entry_bitmap);
    if(Entry)
    {
      Entry->Size = Basis.Scale*Size;
      Entry->Bitmap = Bitmap;
      Entry->P = Basis.P;
      Entry->Color = Color*Group->GlobalAlpha;
    }
  }
}

inline void PushBitmap(render_group *Group, bitmap_id ID, v3 Offset, real32 Height, v4 Color = v4{1, 1, 1, 1})
{
  loaded_bitmap *Bitmap = GetBitmap(Group->Assets, ID);
  if(Bitmap)
  {
    PushBitmap(Group, Bitmap, Offset, Height, Color);
  }
  else
  {
    LaodBitmap(Group->Assets, ID);
    ++Group->MissingBitmapCounts;
  }
}


inline void PushRect(render_group *Group, v3 Offset, v2 Dim, v4 Color)
{
  v3 P = Offset - V3(0.5f*Dim, 0);
  entity_basis_result Basis = GetRenderEntityBasisP(&Group->Transform, P);
  if(Basis.Valid)
  {
    render_entry_rectangle *Rect = PushRenderElement(Group, render_entry_rectangle);
    if(Rect)
    {
      Rect->P = Basis.P;
      Rect->Color = Color;
      Rect->Dim = Basis.Scale*Dim;
    }
  }
}

inline void PushRectOutline(render_group *Group, v3 Offset, v2 Dim, v4 Color)
{
  real32 Thickness = 0.1f;
    
  PushRect(Group, Offset - v3{0, 0.5f*Dim.y, 0}, v2{Dim.x, Thickness}, Color);
  PushRect(Group, Offset + v3{0, 0.5f*Dim.y, 0}, v2{Dim.x, Thickness}, Color);

  PushRect(Group, Offset - v3{0.5f*Dim.x, 0, 0}, v2{Thickness, Dim.y}, Color);
  PushRect(Group, Offset + v3{0.5f*Dim.x, 0, 0}, v2{Thickness, Dim.y}, Color);  
}

inline void Clear(render_group *Group, v4 Color)
{
  render_entry_clear *Piece = PushRenderElement(Group, render_entry_clear);
  if(Piece)
  {
    Piece->Color = Color;
  }
}

inline void CoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
			     loaded_bitmap *Texture, loaded_bitmap *NormalMap,
			     environment_map *Top, environment_map *Middle, environment_map *Bottom) 
{
#if 0
  render_entry_coordinate_system *Piece = PushRenderElement(Group, render_entry_coordinate_system);
  entity_basis_result Basis = GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenDim);
  if(Basis.Valid)
  {
    if(Piece)
    {
      Piece->Origin = Origin;
      Piece->XAxis = XAxis;
      Piece->YAxis = YAxis;
      Piece->Color = Color;
      Piece->Texture = Texture;
      Piece->NormalMap = NormalMap;
      Piece->Top = Top;
      Piece->Middle = Middle;
      Piece->Bottom = Bottom;
    }
#endif    
}
internal void DrawRectangleSlow(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
				loaded_bitmap *Texture, loaded_bitmap *NormalMap,
				environment_map *Top, environment_map *Middle, environment_map *Bottom, real32 PixelsToMeter)
{
  BEGIN_TIMED_BLOCK(DrawRectangleSlowly);
  uint32 Color32 = (uint32)(RoundReal32ToUInt32(255 * Color.a) << 24 |
			    RoundReal32ToUInt32(255 * Color.r) << 16 |
			    RoundReal32ToUInt32(255 * Color.g) << 8 |
			    RoundReal32ToUInt32(255 * Color.b) << 0);

  int WidthMax = Buffer->Width - 1;
  int HeightMax = Buffer->Height -1;

  v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};

  int XMin = WidthMax;
  int XMax = 0;
  int YMin = HeightMax;
  int YMax = 0;

  real32 InvWidthMax = 1.0f / (real32)WidthMax;
  real32 InvHeightMax = 1.0f / (real32)HeightMax;

  real32 OriginZ = 0.0f;
  real32 OriginY = (Origin + 0.5f*XAxis + 0.5f*YAxis).y;
  real32 FixedCastY = InvHeightMax*OriginY;
  
  real32 XAxisLength = Length(XAxis);
  real32 YAxisLength = Length(YAxis);

  v2 NxAxis = (YAxisLength / XAxisLength)*XAxis;
  v2 NyAxis = (XAxisLength / YAxisLength)*YAxis;

  real32 NzScale = 0.5f*(XAxisLength + YAxisLength);

  real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
  real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

  //Premuliply color
  Color.rgb *= Color.a;
  
  for(uint32 Index = 0; Index < ArrayCount(P); ++Index)
  {
    v2 TestP = P[Index];
    int FloorX = FloorReal32ToInt32(P[Index].x);
    int CeilX = CeilReal32ToInt32(P[Index].x);
    int FloorY = FloorReal32ToInt32(P[Index].y);
    int CeilY = CeilReal32ToInt32(P[Index].y);

    
    if(XMin > FloorX) {XMin = FloorX;}
    if(XMax < CeilX) {XMax = CeilX;}
    if(YMin > FloorY) {YMin = FloorY;}
    if(YMax < CeilY) {YMax = CeilY;}
    
  }

  if(XMax > WidthMax) {XMax = WidthMax;}
  if(XMin < 0) {XMin = 0;}
  if(YMax > HeightMax) {YMax = HeightMax;}
  if(YMin < 0) {YMin = 0;}
  
  uint8* Row = ((uint8 *)Buffer->Memory + XMin*BITMAP_BYTES_PER_PIXEL + YMin*Buffer->Pitch); 

  for(int Y = YMin; Y < YMax; ++Y)
  { 
    uint32 *Pixel = (uint32 *)Row;
    for(int X = XMin; X < XMax; ++X)
    {
      v2 PixelP = V2i(X,Y);
      v2 d = PixelP - Origin;
      real32 Edge0 = DotProduct(d, -YAxis);
      real32 Edge1 = DotProduct(d - XAxis, XAxis);
      real32 Edge2 = DotProduct(d - XAxis - YAxis, YAxis);
      real32 Edge3 = DotProduct(d - YAxis, -XAxis);
      if((Edge0 < 0) && (Edge1 < 0) &&
	 (Edge2 < 0) && (Edge3 < 0))
      {
	real32 U = InvXAxisLengthSq*DotProduct(d, XAxis);
	real32 V = InvYAxisLengthSq*DotProduct(d, YAxis);

	real32 tX = 1.0f + ((U*(real32)(Texture->Width - 3)) + 0.5f);
	real32 tY = 1.0f + ((V*(real32)(Texture->Height - 3)) + 0.5f);

	v2 ScreenSpaceUV = v2{InvWidthMax*(real32)X, FixedCastY};

	real32 ZDiff = PixelsToMeter*((real32)Y - OriginY);
	
	int32 XMap = (int32)tX;
	int32 YMap = (int32)tY;

	real32 fX = tX - (real32)XMap;
	real32 fY = tY - (real32)YMap;

	bilinear_sample TexelSample = BilinearSample(Texture, XMap, YMap);

	v4 Texel = SRGBBilinearBlend(TexelSample, fX, fY);

	if(NormalMap)
	{
	  bilinear_sample NormalSample = BilinearSample(NormalMap, XMap, YMap);
	  
          v4 NormalA = Unpack4Bytes(NormalSample.A);		
          v4 NormalB = Unpack4Bytes(NormalSample.B);
          v4 NormalC = Unpack4Bytes(NormalSample.C); 		
          v4 NormalD = Unpack4Bytes(NormalSample.D);
	  
	  v4 Normal = Lerp(Lerp(NormalA, fX, NormalB), fY, Lerp(NormalC, fX, NormalD));
	  real32 Pz = OriginZ + ZDiff;
	  real32 MapZ = 2.0f;
	  
	  Normal = UnscaleAndBiasNormal(Normal);
	  Normal.xy = Normal.x*NxAxis + Normal.y*NyAxis;
	  Normal.z *= NzScale;
	  Normal.xyz = Normalize(Normal.xyz);

	  v3 BounceDirection = 2.0f*Normal.z*Normal.xyz;
	  BounceDirection.z -= 1.0f;
	  BounceDirection.z = -BounceDirection.z;

	  environment_map *FarMap = 0;
	  real32 tEnvMap = BounceDirection.y;
	  real32 tFarMap = 0.0f;
	  if(tEnvMap < -0.5f)
	  {
	    FarMap = Bottom;
	    tFarMap = -1.0f - 2.0f*tEnvMap;
	  }
	  else if(tEnvMap > 0.5f)
	  {
	    FarMap = Top;
	    tFarMap = 2.0f*(tEnvMap - 0.5f);
	  }

	  v3 LigthColor = v3{0, 0, 0}; //SampleFromEnvMap(ScreenSpaceUV, Normal.xyz, Normal.w, Middle);
	  if(FarMap)
	  {
	    tFarMap *= tFarMap;
	    tFarMap *= tFarMap;

	    real32 DistanceFromMapInZ = FarMap->Pz - Pz;
	    v3 FarMapColor = SampleFromEnvMap(ScreenSpaceUV, BounceDirection, Normal.w, FarMap, DistanceFromMapInZ);
	    LigthColor = Lerp(LigthColor, tFarMap, FarMapColor);
	  }

	  Texel.rgb = Texel.rgb + Texel.a*LigthColor;
	}
        Texel.r *= Color.r;
	Texel.g *= Color.g;
	Texel.b *= Color.b;
	Texel.a *= Color.a;

	Texel.r = Clamp01(Texel.r);
	Texel.g = Clamp01(Texel.g);
	Texel.b = Clamp01(Texel.b);


	v4 Dest = {(real32)((*Pixel >> 16) & 0xFF),
		   (real32)((*Pixel >> 8) & 0xFF),
		   (real32)((*Pixel >> 0) & 0xFF),
		   (real32)((*Pixel >> 24) & 0xFF)};

	Dest = SRGB255ToLinear1(Dest);
	real32 InvRSA = (1.0f - Texel.a);

	v4 Blended255 = {InvRSA*Dest.r + Texel.r,
			 InvRSA*Dest.g + Texel.g,
			 InvRSA*Dest.b + Texel.b,
			 (Texel.a + Dest.a - Texel.a*Dest.a)};
	

	Blended255 = Linear1ToSRGB255(Blended255);
	
	*Pixel = (((uint32)(Blended255.a + 0.5f) << 24) |
		  ((uint32)(Blended255.r + 0.5f) << 16) |
		  ((uint32)(Blended255.g + 0.5f) << 8) |
		  ((uint32)(Blended255.b + 0.5f) << 0));
      
      }
      ++Pixel;
    }
    Row += Buffer->Pitch;
  }
  END_TIMED_BLOCK(DrawRectangleSlowly);
}

internal void DrawRectangleQuickly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
				   loaded_bitmap *Texture, real32 PixelsToMeter, rectangle2i ClipRect, bool32 Even)
{
  BEGIN_TIMED_BLOCK(DrawRectangleQuickly);
  uint32 Color32 = (uint32)(RoundReal32ToUInt32(255 * Color.a) << 24 |
			    RoundReal32ToUInt32(255 * Color.r) << 16 |
			    RoundReal32ToUInt32(255 * Color.g) << 8 |
			    RoundReal32ToUInt32(255 * Color.b) << 0);

  v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};

  //real32 InvWidthMax = 1.0f / (real32)WidthMax;
  //real32 InvHeightMax = 1.0f / (real32)HeightMax;

  real32 OriginZ = 0.0f;
  real32 OriginY = (Origin + 0.5f*XAxis + 0.5f*YAxis).y;
  //real32 FixedCastY = InvHeightMax*OriginY;
  
  real32 XAxisLength = Length(XAxis);
  real32 YAxisLength = Length(YAxis);

  v2 NxAxis = (YAxisLength / XAxisLength)*XAxis;
  v2 NyAxis = (XAxisLength / YAxisLength)*YAxis;

  real32 NzScale = 0.5f*(XAxisLength + YAxisLength);

  real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
  real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

  //Premuliply color
  Color.rgb *= Color.a;

  rectangle2i FillRect = InvertedInfinityRectangle();
  
  for(uint32 Index = 0; Index < ArrayCount(P); ++Index)
  {
    v2 TestP = P[Index];
    int FloorX = FloorReal32ToInt32(P[Index].x);
    int CeilX = CeilReal32ToInt32(P[Index].x) + 1;
    int FloorY = FloorReal32ToInt32(P[Index].y);
    int CeilY = CeilReal32ToInt32(P[Index].y) + 1;

    
    if(FillRect.MinX > FloorX) {FillRect.MinX = FloorX;}
    if(FillRect.MaxX < CeilX) {FillRect.MaxX = CeilX;}
    if(FillRect.MinY > FloorY) {FillRect.MinY = FloorY;}
    if(FillRect.MaxY < CeilY) {FillRect.MaxY = CeilY;}
    
  }

  FillRect = Intersect(ClipRect, FillRect);

  if(!Even == (FillRect.MinY & 1))
  {
    FillRect.MinY += 1;
  }
  //This is for keeping pixels aligned by four
  int FillWidth = FillRect.MaxX - FillRect.MinX;
  int FillWidthAlign = FillWidth & 3;
  
  __m128i StartClipMask = _mm_set1_epi8(-1);
  __m128i EndClipMask = _mm_set1_epi8(-1);

  __m128i StartClipMasks[] =
    {
      _mm_slli_si128(StartClipMask, 4*0),
      _mm_slli_si128(StartClipMask, 4*1),
      _mm_slli_si128(StartClipMask, 4*2),
      _mm_slli_si128(StartClipMask, 4*3)
    };

  __m128i EndClipMasks[] =
    {
      _mm_srli_si128(EndClipMask, 4*0),
      _mm_srli_si128(EndClipMask, 4*3),
      _mm_srli_si128(EndClipMask, 4*2),
      _mm_srli_si128(EndClipMask, 4*1)
    };

  if(FillRect.MinX & 3)
  {
    FillRect.MinX = FillRect.MinX & ~3;
    StartClipMask = StartClipMasks[FillRect.MinX & 3];
  }

  if(FillRect.MaxX & 3)
  {
    FillRect.MaxX = (FillRect.MaxX & ~3) + 4;
    StartClipMask = StartClipMasks[FillRect.MaxX & 3];
  }
  
  v2 nXAxis = InvXAxisLengthSq*XAxis;
  v2 nYAxis = InvYAxisLengthSq*YAxis;
  real32 Inv255 = 1.0f / 255.0f;
  real32 One255 = 255.0f;

  __m128 Inv255_4x = _mm_set1_ps(Inv255);
  __m128 One = _mm_set1_ps(1.0f);
  __m128 Half = _mm_set1_ps(0.5f);
  __m128 One255_4x = _mm_set1_ps(255.0f);
  __m128 Zero = _mm_set1_ps(0.0f);
  __m128 Colorr_4x = _mm_set1_ps(Color.r);
  __m128 Colorg_4x = _mm_set1_ps(Color.g);
  __m128 Colorb_4x = _mm_set1_ps(Color.b);
  __m128 Colora_4x = _mm_set1_ps(Color.a);
  __m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
  __m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
  __m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
  __m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);
  __m128 Originx_4x = _mm_set1_ps(Origin.x);
  __m128 Originy_4x = _mm_set1_ps(Origin.y);
  __m128i MaskFF = _mm_set1_epi32(0xFF); 
  __m128 WidthM2 = _mm_set1_ps((real32)Texture->Width - 2);
  __m128 HeightM2 = _mm_set1_ps((real32)Texture->Height - 2);
  __m128 Four_4x = _mm_set1_ps(4.0f);
  __m128i TexturePitch4X = _mm_set1_epi32(Texture->Pitch);
  
  uint8* Row = ((uint8 *)Buffer->Memory + FillRect.MinX*BITMAP_BYTES_PER_PIXEL + FillRect.MinY*Buffer->Pitch);
  void* TexMem = Texture->Memory;
  int32 TexturePitch = Texture->Pitch;
  int32 RowAdvance = Buffer->Pitch*2;
  
  
  for(int Y = FillRect.MinY; Y < FillRect.MaxY; Y += 2)
  { 
    uint32 *Pixel = (uint32 *)Row;
    
    __m128 PixelPy = _mm_set1_ps((real32)Y);
    PixelPy = _mm_sub_ps(PixelPy, Originy_4x);

    
    __m128 PixelPx = _mm_set_ps(((real32)FillRect.MinX + 3), ((real32)FillRect.MinX + 2),
				((real32)FillRect.MinX + 1), ((real32)FillRect.MinX + 0));

    PixelPx = _mm_sub_ps(PixelPx, Originx_4x);

    __m128i ClipMask = StartClipMask;
    
    for(int XI = FillRect.MinX; XI < FillRect.MaxX; XI += 4)
    {
      __m128 Blendedr = _mm_set1_ps(0.0f);
      __m128 Blendedg = _mm_set1_ps(0.0f);
      __m128 Blendedb = _mm_set1_ps(0.0f);
      __m128 Blendeda = _mm_set1_ps(0.0f);
      
#define mmSquare(a) _mm_mul_ps(a, a)
#define M(a, i) ((float *)&a)[i]
#define Mi(a, i) ((int *)&a)[i]

      __m128 U = _mm_add_ps(_mm_mul_ps(PixelPx, nXAxisx_4x), _mm_mul_ps(PixelPy, nXAxisy_4x));
      __m128 V = _mm_add_ps(_mm_mul_ps(PixelPx, nYAxisx_4x), _mm_mul_ps(PixelPy, nYAxisy_4x));

      __m128i OriginalDest = _mm_loadu_si128((__m128i *)Pixel);
      __m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmpge_ps(U, Zero), _mm_cmple_ps(U, One)),
						      _mm_and_ps(_mm_cmpge_ps(V, Zero), _mm_cmple_ps(V, One))));

      WriteMask = _mm_and_si128(WriteMask, ClipMask);
      
      U = _mm_min_ps(_mm_max_ps(U, Zero), One);
      V = _mm_min_ps(_mm_max_ps(V, Zero), One);

      __m128 tX = _mm_add_ps(_mm_mul_ps(U, WidthM2), Half);
      __m128 tY = _mm_add_ps(_mm_mul_ps(V, HeightM2), Half);

      __m128i FetchX_4x = _mm_cvttps_epi32(tX);
      __m128i FetchY_4x = _mm_cvttps_epi32(tY);

      __m128 fX = _mm_sub_ps(tX, _mm_cvtepi32_ps(FetchX_4x));
      __m128 fY = _mm_sub_ps(tY, _mm_cvtepi32_ps(FetchY_4x));

      __m128i XMap = _mm_slli_epi32(FetchX_4x, 2);
      __m128i YMap = _mm_mullo_epi32(FetchY_4x, TexturePitch4X);
      __m128i Fetch4X = _mm_add_epi32(XMap, YMap);

      int32 Fetch0 = Mi(Fetch4X, 0);
      int32 Fetch1 = Mi(Fetch4X, 1);
      int32 Fetch2 = Mi(Fetch4X, 2);
      int32 Fetch3 = Mi(Fetch4X, 3);
	
      uint8 *TexelPtr0 = (uint8 *)TexMem + Fetch0;
      uint8 *TexelPtr1 = (uint8 *)TexMem + Fetch1;
      uint8 *TexelPtr2 = (uint8 *)TexMem + Fetch2;
      uint8 *TexelPtr3 = (uint8 *)TexMem + Fetch3;

      __m128i SampleA = _mm_setr_epi32(*(uint32 *)(TexelPtr0), *(uint32 *)(TexelPtr1),
				       *(uint32 *)(TexelPtr2), *(uint32 *)(TexelPtr3));

      __m128i SampleB = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + sizeof(uint32)), *(uint32 *)(TexelPtr1 + sizeof(uint32)),
				       *(uint32 *)(TexelPtr2 + sizeof(uint32)), *(uint32 *)(TexelPtr3 + sizeof(uint32)));

      __m128i SampleC = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch), *(uint32 *)(TexelPtr1 + TexturePitch),
				       *(uint32 *)(TexelPtr2 + TexturePitch), *(uint32 *)(TexelPtr3 + TexturePitch));

      __m128i SampleD = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch + sizeof(uint32)), *(uint32 *)(TexelPtr1 + TexturePitch + sizeof(uint32)),
				       *(uint32 *)(TexelPtr2 + TexturePitch + sizeof(uint32)), *(uint32 *)(TexelPtr3 + TexturePitch + sizeof(uint32)));
     
      
      //Unpack every colors from pixels samples
      __m128 TexelAr, TexelAg, TexelAb, TexelAa;
      TexelAr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 16), MaskFF));
      TexelAg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF));
      TexelAb = _mm_cvtepi32_ps(_mm_and_si128(SampleA, MaskFF));
      TexelAa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleA, 24), MaskFF));
      
      __m128 TexelBr, TexelBg, TexelBb, TexelBa;
      TexelBr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 16), MaskFF));
      TexelBg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 8), MaskFF));
      TexelBb = _mm_cvtepi32_ps(_mm_and_si128(SampleB, MaskFF));
      TexelBa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleB, 24), MaskFF));
  
      __m128 TexelCr, TexelCg, TexelCb, TexelCa;
      TexelCr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 16), MaskFF));
      TexelCg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 8), MaskFF));
      TexelCb = _mm_cvtepi32_ps(_mm_and_si128(SampleC, MaskFF));
      TexelCa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleC, 24), MaskFF));
     
      __m128 TexelDr, TexelDg, TexelDb, TexelDa;
      TexelDr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 16), MaskFF));
      TexelDg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 8), MaskFF));
      TexelDb = _mm_cvtepi32_ps(_mm_and_si128(SampleD, MaskFF));
      TexelDa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(SampleD, 24), MaskFF));

      //Load destination pixels
      __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
      __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
      __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest,MaskFF));
      __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));
      
      
      //From sRGB to linear 0-1 
      TexelAr = mmSquare(_mm_mul_ps(Inv255_4x, TexelAr));
      TexelAg = mmSquare(_mm_mul_ps(Inv255_4x, TexelAg));
      TexelAb = mmSquare(_mm_mul_ps(Inv255_4x, TexelAb));
      TexelAa = _mm_mul_ps(Inv255_4x, TexelAa);

      TexelBr = mmSquare(_mm_mul_ps(Inv255_4x, TexelBr));
      TexelBg = mmSquare(_mm_mul_ps(Inv255_4x, TexelBg));
      TexelBb = mmSquare(_mm_mul_ps(Inv255_4x, TexelBb));
      TexelBa = _mm_mul_ps(Inv255_4x, TexelBa);

      TexelCr = mmSquare(_mm_mul_ps(Inv255_4x, TexelCr));
      TexelCg = mmSquare(_mm_mul_ps(Inv255_4x, TexelCg));
      TexelCb = mmSquare(_mm_mul_ps(Inv255_4x, TexelCb));
      TexelCa = _mm_mul_ps(Inv255_4x, TexelCa);

      TexelDr = mmSquare(_mm_mul_ps(Inv255_4x, TexelDr));
      TexelDg = mmSquare(_mm_mul_ps(Inv255_4x, TexelDg));
      TexelDb = mmSquare(_mm_mul_ps(Inv255_4x, TexelDb));
      TexelDa = _mm_mul_ps(Inv255_4x, TexelDa);
	
      //Bilinear texture blend
      __m128 ifX = _mm_sub_ps(One, fX);
      __m128 ifY = _mm_sub_ps(One, fY);

      __m128 l0 = _mm_mul_ps(ifY, ifX);
      __m128 l1 = _mm_mul_ps(ifY, fX);
      __m128 l2 = _mm_mul_ps(fY, ifX);
      __m128 l3 = _mm_mul_ps(fY, fX);

      __m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)), _mm_add_ps(_mm_mul_ps(l2, TexelCr), _mm_mul_ps(l3, TexelDr)));
      __m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)), _mm_add_ps(_mm_mul_ps(l2, TexelCg), _mm_mul_ps(l3, TexelDg)));
      __m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)), _mm_add_ps(_mm_mul_ps(l2, TexelCb), _mm_mul_ps(l3, TexelDb)));
      __m128 Texela = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)), _mm_add_ps(_mm_mul_ps(l2, TexelCa), _mm_mul_ps(l3, TexelDa)));

      //Multiply by external color
      Texelr = _mm_mul_ps(Texelr, Colorr_4x);
      Texelg = _mm_mul_ps(Texelg, Colorg_4x);
      Texelb = _mm_mul_ps(Texelb, Colorb_4x);
      Texela = _mm_mul_ps(Texela, Colora_4x);

      //Clamp01
      Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), One);
      Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), One);
      Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), One);

      //Dest pixels from SRGB to linear 0 - 1
      Destr = mmSquare(_mm_mul_ps(Inv255_4x, Destr));
      Destg = mmSquare(_mm_mul_ps(Inv255_4x, Destg));
      Destb = mmSquare(_mm_mul_ps(Inv255_4x, Destb));
      Desta = _mm_mul_ps(Inv255_4x, Desta);

      //Blend with Dest
      __m128 InvTexela = _mm_sub_ps(One, Texela);
      Blendedr = _mm_add_ps(_mm_mul_ps(InvTexela, Destr), Texelr);
      Blendedg = _mm_add_ps(_mm_mul_ps(InvTexela, Destg), Texelg);
      Blendedb = _mm_add_ps(_mm_mul_ps(InvTexela, Destb), Texelb);
      Blendeda = _mm_add_ps(_mm_mul_ps(InvTexela, Desta), Texela);

      //From linear back to corrected sRGB255
      Blendedr = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedr));
      Blendedg = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedg));
      Blendedb = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedb));
      Blendeda = _mm_mul_ps(One255_4x, Blendeda);

      //Converting from floating point to int
      __m128i IntR = _mm_cvtps_epi32(Blendedr);
      __m128i IntG = _mm_cvtps_epi32(Blendedg);
      __m128i IntB = _mm_cvtps_epi32(Blendedb);
      __m128i IntA = _mm_cvtps_epi32(Blendeda);

      //Shift the 8 bit colors in the rigth position
      __m128i Sr = _mm_slli_epi32(IntR, 16);
      __m128i Sg = _mm_slli_epi32(IntG, 8);
      __m128i Sb = IntB;
      __m128i Sa = _mm_slli_epi32(IntA, 24);

      //Composing the all the 4 pixels
      __m128i Out = _mm_or_si128(_mm_or_si128(_mm_or_si128(Sr, Sg), Sb), Sa);

      //Writing the pixels
      __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out), _mm_andnot_si128(WriteMask, OriginalDest));
      _mm_storeu_si128((__m128i *)Pixel, MaskedOut);

      Pixel += 4;
      PixelPx = _mm_add_ps(PixelPx, Four_4x);
      if((XI + 8) < FillRect.MaxX)
      {
	ClipMask = _mm_set1_epi8(-1);
      }
      else
      {
	ClipMask = EndClipMask;
      }

    }
    Row += RowAdvance;
  }
  END_TIMED_BLOCK(DrawRectangleQuickly);
}

/*
internal void DrawRectangleOutline(loaded_bitmap* Buffer, v2 vMin, v2 vMax, v3 Color, real32 T = 2.0f)
{
  //Top and Bottom
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMax.x + T, vMin.y + T}, ToV4(Color, 1.0f));
  DrawRectangle(Buffer, v2{vMin.x - T, vMax.y - T}, v2{vMax.x + T, vMax.y + T}, ToV4(Color, 1.0f));
  //Left and Right
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMin.x + T, vMax.y + T}, ToV4(Color, 1.0f));
  DrawRectangle(Buffer, v2{vMax.x - T, vMin.y - T}, v2{vMax.x + T, vMax.y + T}, ToV4(Color, 1.0f));
}
*/

internal void DrawBitmap(loaded_bitmap *Buffer, loaded_bitmap *Bitmap, real32 realX, real32 realY, real32 CAlpha = 1.0f)
{
  
  int32 MinX = RoundReal32ToInt32(realX);
  int32 MaxX = MinX + Bitmap->Width;
  int32 MinY = RoundReal32ToInt32(realY);
  int32 MaxY = MinY + Bitmap->Height;

  int32 SourceOffsetX = 0;
  if(MinX < 0)
  {
    SourceOffsetX = -MinX;
    MinX = 0;
  }
  if(MaxX > Buffer->Width)
  {
    MaxX = Buffer->Width;
  }
  int32 SourceOffsetY = 0;
  if(MinY < 0)
  {
    SourceOffsetY = -MinY;
    MinY = 0; 
  }
  if(MaxY > Buffer->Height)
  {
    MaxY = Buffer->Height;
  }
  int32 BytesPerPixel = BITMAP_BYTES_PER_PIXEL;
  uint8 *DestRow = ((uint8 *)Buffer->Memory + MinX*BytesPerPixel + MinY*Buffer->Pitch); 
  uint8 *SourceRow = (uint8 *)Bitmap->Memory + SourceOffsetY*Bitmap->Pitch + BytesPerPixel*SourceOffsetX;
  
  for(int32 Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Dest = (uint32 *)DestRow;
    uint32 *Source = (uint32 *)SourceRow;
    for(int32 X = MinX; X < MaxX; ++X)
    {
      //Linear Alpha Blending
      v4 Texel ={(real32)((*Source >> 16) & 0xFF),
		 (real32)((*Source >> 8) & 0xFF),
		 (real32)((*Source >> 0) & 0xFF),
		 (real32)((*Source >> 24) & 0xFF)};

      Texel = SRGB255ToLinear1(Texel);

      Texel *= CAlpha;


      v4 D = {(real32)((*Dest >> 16) & 0xFF),
	      (real32)((*Dest >> 8) & 0xFF),
	      (real32)((*Dest >> 0) & 0xFF),
	      (real32)((*Dest >> 24) & 0xFF)};

      D = SRGB255ToLinear1(D);
        
      real32 InvRSA = (1.0f - Texel.a);
      
      v4 Result = {InvRSA*D.r + Texel.r,
		   InvRSA*D.g + Texel.g,
		   InvRSA*D.b + Texel.b,
		   (Texel.a + D.a - Texel.a*D.a)};

      Result = Linear1ToSRGB255(Result);

      *Dest = (((uint32)(Result.a + 0.5f) << 24) |
	       ((uint32)(Result.r + 0.5f) << 16) |
	       ((uint32)(Result.g + 0.5f) << 8) |
	       ((uint32)(Result.b + 0.5f) << 0));
      
      ++Dest;
      ++Source;
    }
    DestRow += Buffer->Pitch;
    SourceRow += Bitmap->Pitch;
  }
}

internal render_group *AllocateRenderGroup(game_assets *Assets, memory_arena *Arena, uint32 MaxPushBufferSize)
{
  render_group *Result = PushStruct(Arena, render_group);
  if(MaxPushBufferSize == 0)
  {
    MaxPushBufferSize =  (uint32)GetArenaSizeRemaing(Arena);
  }
  Result->PushBufferBase = (uint8 *)PushSize(Arena, MaxPushBufferSize);
 
  Result->MaxPushBufferSize = MaxPushBufferSize;
  Result->PushBufferSize = 0;
  Result->GlobalAlpha = 1.0f;  
  Result->Assets = Assets;
  
  Result->Transform.OffsetP = v3{0.0f, 0.0f, 0.0f};
  Result->Transform.Scale = 1.0f;

  Result->MissingBitmapCounts = 0;
  return Result;
}

inline void Perspective(render_group *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 MetersToPixels, real32 FocalLength, real32 DistanceAboveTarget)
{
  real32 PixelsToMeters = 1.0f / MetersToPixels;

  RenderGroup->MonitorHalfDimInMeters = v2{0.5f*PixelWidth*PixelsToMeters,
					   0.5f*PixelHeight*PixelsToMeters};
   
  RenderGroup->Transform.MetersToPixels = MetersToPixels; 
  RenderGroup->Transform.FocalLength = FocalLength;
  RenderGroup->Transform.CameraDistanceAboveTarget = DistanceAboveTarget;
   
  RenderGroup->Transform.ScreenCenter = v2{0.5f*PixelWidth,
					   0.5f*PixelHeight};

  RenderGroup->Transform.Orthographic = false;
}

inline void Orthographic(render_group *RenderGroup, int32 PixelWidth, int32 PixelHeight, real32 MetersToPixels)
{
  real32 PixelsToMeters = 1.0f / MetersToPixels;

  RenderGroup->MonitorHalfDimInMeters = v2{0.5f*PixelWidth*PixelsToMeters,
					   0.5f*PixelHeight*PixelsToMeters};
   
  RenderGroup->Transform.MetersToPixels = MetersToPixels; 
  RenderGroup->Transform.FocalLength = 1.0f;
  RenderGroup->Transform.CameraDistanceAboveTarget = 1.0f;
   
  RenderGroup->Transform.ScreenCenter = v2{0.5f*PixelWidth,
					   0.5f*PixelHeight};

  RenderGroup->Transform.Orthographic = true;
}

internal void RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget, rectangle2i ClipRect, bool Even)
{
  BEGIN_TIMED_BLOCK(RenderGroupToOutput);

  real32 NullPixelsToMeters = 1.0f;
 for(uint32 BaseAdress = 0; BaseAdress < RenderGroup->PushBufferSize; )
 {
   render_group_entry_header *Header = (render_group_entry_header *)(RenderGroup->PushBufferBase + BaseAdress);
   void *Data = (uint8 *)Header + sizeof(*Header);
   BaseAdress += sizeof(*Header);

   switch(Header->Type)
   {
     case RenderGroupEntryType_render_entry_clear:
     {
       render_entry_clear *Entry = (render_entry_clear *)Data;

       DrawRectangle(OutputTarget, v2{0, 0}, v2{(real32)OutputTarget->Width, (real32)OutputTarget->Height}, Entry->Color, ClipRect, Even);
       
       BaseAdress += sizeof(*Entry);
     } break;

     case RenderGroupEntryType_render_entry_bitmap:
     {
       render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
#if 0
       DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y);
#else
       DrawRectangleQuickly(OutputTarget, Entry->P,
			    v2{Entry->Size.x, 0},
			    v2{0, Entry->Size.y},
			    Entry->Color,
			    Entry->Bitmap, NullPixelsToMeters, ClipRect, Even);
#endif
       BaseAdress += sizeof(*Entry);
       
     } break;
       
     case RenderGroupEntryType_render_entry_rectangle:
     {
       render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
       DrawRectangle(OutputTarget, Entry->P, Entry->P + Entry->Dim, Entry->Color, ClipRect, Even);

       BaseAdress += sizeof(*Entry);
     } break;

     case RenderGroupEntryType_render_entry_coordinate_system:
     {
       render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Data;
#if 0
       v2 Dim = {4, 4};
       v2 P = Entry->Origin;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color);

       P = Entry->Origin + Entry->XAxis;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color);

       P = Entry->Origin + Entry->YAxis;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color);

       v2 Point = Entry->Point;
       Point = Entry->Origin + Point.x*Entry->XAxis + Point.y*Entry->YAxis;

       P = Entry->Origin + Entry->YAxis;
       DrawRectangleSlow(OutputTarget, Entry->Origin, Entry->XAxis, Entry->YAxis,
			 Entry->Color, Entry->Texture, Entry->NormalMap,
			 Entry->Top, Entry->Middle, Entry->Bottom, 1.0f / PixelsToMeters);
#endif
       BaseAdress += sizeof(*Entry);
     } break;

     InvalidDefaultCase;
   }  
 }
 END_TIMED_BLOCK(RenderGroupToOutput);
}

struct tile_render_work
{
  render_group  *RenderGroup;
  loaded_bitmap *OutputTarget;
  rectangle2i   ClipRect;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(DoTiledRenderWork)
{
  tile_render_work *Work = (tile_render_work *)Data;
  
  RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, true);
  RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, false);      
}

 internal void RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{

  rectangle2i ClipRect;
  ClipRect.MinX = 0;
  ClipRect.MaxX = OutputTarget->Width;
  ClipRect.MinY = 0;
  ClipRect.MaxY = OutputTarget->Height;

  tile_render_work Work;
  Work.RenderGroup = RenderGroup;
  Work.OutputTarget = OutputTarget;
  Work.ClipRect = ClipRect;

  DoTiledRenderWork(0, &Work);
}

internal void TiledRenderGroupToOutput(platform_work_queue *RenderQueue, render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
  int const TileCountX = 4;
  int const TileCountY = 4;

  int WorkCount = 0;

  tile_render_work WorkArray[TileCountX*TileCountY];

  int TileWidth = OutputTarget->Width / TileCountX;
  int TileHeight = OutputTarget->Height / TileCountY;

  TileWidth = ((TileWidth + 3) / 4) * 4; 
  
  for(int TileY = 0; TileY < TileCountY; ++TileY)
  {
    for(int TileX = 0; TileX < TileCountX; ++TileX)
    {
      tile_render_work *Work = WorkArray + WorkCount++;
      rectangle2i ClipRect;
      ClipRect.MinX = TileX*TileWidth;
      ClipRect.MaxX = ClipRect.MinX + TileWidth;
      ClipRect.MinY = TileY*TileHeight;
      ClipRect.MaxY = ClipRect.MinY + TileHeight;

      if(ClipRect.MaxX > OutputTarget->Width)
      {
	ClipRect.MaxX = OutputTarget->Width;
      }
      
      Work->RenderGroup = RenderGroup;
      Work->OutputTarget = OutputTarget;
      Work->ClipRect = ClipRect;

      PlatformAddEntry(RenderQueue, DoTiledRenderWork, Work);
    }
  }
  PlatformCompleteAllWork(RenderQueue);
}

#if 0
internal void TiledRenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
  int TileCountX = 4;
  int TileCountY = 4;

  int TileWidth = OutputTarget->Width / TileCountX;
  int TileHeight = OutputTarget->Height / TileCountY;
  
  for(int TileY = 0; TileY < TileCountY; ++TileY)
  {
    for(int TileX = 0; TileX < TileCountX; ++TileX)
    {
      rectangle2i ClipRect;
      ClipRect.MinX = TileX*TileWidth + 4;
      ClipRect.MaxX = ClipRect.MinX + TileWidth - 4;
      ClipRect.MinY = TileY*TileHeight + 4;
      ClipRect.MaxY = ClipRect.MinY + TileHeight - 4;
      RenderGroupToOutput(RenderGroup,OutputTarget, ClipRect, true);
      RenderGroupToOutput(RenderGroup,OutputTarget, ClipRect, false);      
    }
  }
}
#endif

inline v2 UnProjected(render_group *Group, v2 ProjectedXY, real32 AtDistanceFromTheCamera)
{
  v2 WorldXY = (AtDistanceFromTheCamera / Group->Transform.FocalLength)*ProjectedXY;
  return WorldXY;
}

inline rectangle2 GetCameraRectangleAtDistance(render_group *Group, real32 DistanceFromTheCamera)
{
  v2 RawXY = UnProjected(Group, Group->MonitorHalfDimInMeters, DistanceFromTheCamera);
  rectangle2 Result = RectCenHalfDim(v2{0, 0}, RawXY);
  return Result;
}

inline rectangle2 GetCameraRectangleAtTarget(render_group *Group)
{
  rectangle2 Result = GetCameraRectangleAtDistance(Group, Group->Transform.CameraDistanceAboveTarget);
  return Result;
}

inline bool32 AllResourcesArePresent(render_group *RenderGroup)
{
  bool32 Result = (RenderGroup->MissingBitmapCounts == 0);
  return Result;
}
