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

internal void DrawRectangle(loaded_bitmap *Buffer, v2 vMin, v2 vMax, v4 Color)
{
  real32 R = Color.r;
  real32 G = Color.g;
  real32 B = Color.b;
  real32 A = Color.a;
  
  int32 MinX = RoundReal32ToInt32(vMin.x);
  int32 MaxX = RoundReal32ToInt32(vMax.x);
  int32 MinY = RoundReal32ToInt32(vMin.y);
  int32 MaxY = RoundReal32ToInt32(vMax.y);

  if(MinX < 0)
  {
    MinX = 0;
  }
  if(MaxX > Buffer->Width)
  {
    MaxX = Buffer->Width;
  }
  if(MinY < 0)
  {
    MinY = 0;
  }
  if(MaxY > Buffer->Height)
  {
    MaxY = Buffer->Height;
  }
 
  uint32 Color32 = (uint32)(RoundReal32ToUInt32(255 * A) << 24 |
			  RoundReal32ToUInt32(255 * R) << 16 |
			  RoundReal32ToUInt32(255 * G) << 8 |
			  RoundReal32ToUInt32(255 *B) << 0);
  
  uint8* Row = ((uint8 *)Buffer->Memory + MinX*BITMAP_BYTES_PER_PIXEL + MinY*Buffer->Pitch); 

  for(int Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = MinX; X < MaxX; ++X)
    {
      *Pixel++ = Color32;
    }
    Row += Buffer->Pitch;
  }
}

inline v3 SampleFromEnvMap(v2 ScreenSpaceUV, v3 SampleDirection, real32 Roughness, environment_map *Map)
{
  uint32 LODIndex = (uint32)(Roughness*((real32)ArrayCount(Map->LOD) - 1) + 0.5f);
  Assert(LODIndex < ArrayCount(Map->LOD));
  loaded_bitmap *LOD = &Map->LOD[LODIndex];

  Assert(SampleDirection.y > 0.0f);

  real32 DistanceFromMapInZ = 1.0f;
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

internal void DrawRectangleSlow(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
				loaded_bitmap *Texture, loaded_bitmap *NormalMap,
				environment_map *Top, environment_map *Middle, environment_map *Bottom)
{
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
  if(XMin < 0) {XMax = 0;}
  if(YMax > HeightMax) {YMax = HeightMax;}
  if(YMin < 0) {YMax = 0;}
  
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

	v2 ScreenSpaceUV = v2{InvWidthMax*(real32)X, InvHeightMax*(real32)Y};
	
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

	  Normal = UnscaleAndBiasNormal(Normal);
	  Normal.xyz = Normalize(Normal.xyz);

	  v3 BounceDirection = 2.0f*Normal.z*Normal.xyz;
	  BounceDirection.z -= 1.0f;

#if 1
	  environment_map *FarMap = 0;
	  real32 tEnvMap = BounceDirection.y;
	  real32 tFarMap = 0.0f;
	  if(tEnvMap < -0.5f)
	  {
	    FarMap = Bottom;
	    tFarMap = -1.0f - 2.0f*tEnvMap;
	    BounceDirection.y = -BounceDirection.y; 
	  }
	  else if(tEnvMap > 0.5f)
	  {
	    FarMap = Top;
	    tFarMap = 2.0f*(tEnvMap - 0.5f);
	  }
	  
	  v3 LigthColor = v3{0, 0, 0}; //SampleFromEnvMap(ScreenSpaceUV, Normal.xyz, Normal.w, Middle);
	  if(FarMap)
	  {
	    v3 FarMapColor = SampleFromEnvMap(ScreenSpaceUV, BounceDirection, Normal.w, FarMap);
	    LigthColor = Lerp(LigthColor, tFarMap, FarMapColor);
	  }

	  Texel.rgb = Texel.rgb + Texel.a*LigthColor;
#else
	  Texel.rgb = v3{0.5f, 0.5f, 0.5f} + 0.5f*Normal.rgb;
	  Texel.a = 1.0f;
#endif
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
  
}


internal void DrawRectangleOutline(loaded_bitmap* Buffer, v2 vMin, v2 vMax, v3 Color, real32 T = 2.0f)
{
  //Top and Bottom
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMax.x + T, vMin.y + T}, ToV4(Color, 1.0f));
  DrawRectangle(Buffer, v2{vMin.x - T, vMax.y - T}, v2{vMax.x + T, vMax.y + T}, ToV4(Color, 1.0f));
  //Left and Right
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMin.x + T, vMax.y + T}, ToV4(Color, 1.0f));
  DrawRectangle(Buffer, v2{vMax.x - T, vMin.y - T}, v2{vMax.x + T, vMax.y + T}, ToV4(Color, 1.0f));
}


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

v2 GetRenderEntityBasisP(render_group *RenderGroup, render_entity_basis *EntityBasis, v2 ScreenCenter)
{
  v3 EntityBaseP = EntityBasis->Basis->P;
    
  real32 ZFudge = (1.0f + 0.1f*(EntityBaseP.z + EntityBasis->OffsetZ));
      

  real32 EntityGroundX = ScreenCenter.x + RenderGroup->MetersToPixel*ZFudge*EntityBaseP.x;
  real32 EntityGroundY = ScreenCenter.y - RenderGroup->MetersToPixel*ZFudge*EntityBaseP.y;
  real32 Z = -RenderGroup->MetersToPixel*EntityBaseP.z;
       
  v2 Center = {EntityGroundX + EntityBasis->Offset.x,
	       EntityGroundY + EntityBasis->Offset.y + Z};

  return Center;
}

internal render_group *AllocateRenderGroup(memory_arena *Arena, uint32 MaxPushBufferSize, real32 MetersToPixel)
{
  render_group *Result = PushStruct(Arena, render_group);
  Result->PushBufferBase = (uint8 *)PushSize(Arena, MaxPushBufferSize);
  Result->DefaultBasis = PushStruct(Arena, render_basis);
  Result->DefaultBasis->P = v3{0, 0, 0};
  Result->MetersToPixel = MetersToPixel;

  Result->MaxPushBufferSize = MaxPushBufferSize;
  Result->PushBufferSize = 0;

  return Result;
}

internal void RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
  v2 ScreenCenter = v2{0.5f*(real32)OutputTarget->Width, 0.5f*(real32)OutputTarget->Height};

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

       DrawRectangle(OutputTarget, v2{0, 0}, v2{(real32)OutputTarget->Width, (real32)OutputTarget->Height}, Entry->Color);
       
       BaseAdress += sizeof(render_entry_clear);
     } break;

     case RenderGroupEntryType_render_entry_bitmap:
     {
       render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
#if 0
       v2 P =  GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
       //Assert(Entry->Bitmap);
       DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y);
#endif  
       BaseAdress += sizeof(render_entry_bitmap);
     } break;
       
     case RenderGroupEntryType_render_entry_rectangle:
     {
       render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
       v2 P =  GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
       DrawRectangle(OutputTarget, P, P + Entry->Dim, Entry->Color);

       BaseAdress += sizeof(render_entry_rectangle);
     } break;

     case RenderGroupEntryType_render_entry_coordinate_system:
     {
       render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Data;
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
			 Entry->Top, Entry->Middle, Entry->Bottom);

       BaseAdress += sizeof(render_entry_coordinate_system);
     } break;

     InvalidDefaultCase;
   }  
 }
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

inline void PushPiece(render_group *Group, loaded_bitmap *Bitmap, v2 Offset,
		       real32 OffsetZ, v2 Align, v2 Dim, v4 Color)
{
  render_entry_bitmap *Piece = PushRenderElement(Group, render_entry_bitmap);
  Assert(Bitmap);
  if(Piece)
  {
    Piece->Bitmap = Bitmap;
    Piece->EntityBasis.Basis = Group->DefaultBasis;
    Piece->EntityBasis.Offset = Group->MetersToPixel*v2{Offset.x, -Offset.y} - Align;
    Piece->EntityBasis.OffsetZ = OffsetZ;
    Piece->Color = Color;
  }
}

inline void PushBitmap(render_group *Group, loaded_bitmap *Bitmap, v2 Offset,
			 real32 OffsetZ, v2 Align, real32 Alpha = 1.0f)
{
  PushPiece(Group, Bitmap, Offset, OffsetZ, Align, v2{0, 0},v4{1.0f, 1.0f, 1.0f, Alpha});
}

inline void PushRect(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim, v4 Color)
{
  render_entry_rectangle *Piece = PushRenderElement(Group, render_entry_rectangle);
  if(Piece)
  {
    v2 HalfDim = 0.5f*Group->MetersToPixel*Dim;
    Piece->EntityBasis.Basis = Group->DefaultBasis;
    Piece->EntityBasis.Offset = Group->MetersToPixel*v2{Offset.x, -Offset.y} - HalfDim;
    Piece->EntityBasis.OffsetZ = OffsetZ;
    Piece->Color = Color;
    Piece->Dim = Group->MetersToPixel*Dim;
  }
}

inline void PushRectOutline(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim, v4 Color)
{
  real32 Thickness = 0.1f;
    
  PushRect(Group, Offset - v2{0, 0.5f*Dim.y}, OffsetZ, v2{Dim.x, Thickness}, Color);
  PushRect(Group, Offset + v2{0, 0.5f*Dim.y}, OffsetZ, v2{Dim.x, Thickness}, Color);

  PushRect(Group, Offset - v2{0.5f*Dim.x, 0}, OffsetZ, v2{Thickness, Dim.y}, Color);
  PushRect(Group, Offset + v2{0.5f*Dim.x, 0}, OffsetZ, v2{Thickness, Dim.y}, Color);  
}

inline void Clear(render_group *Group, v4 Color)
{
  render_entry_clear *Piece = PushRenderElement(Group, render_entry_clear);
  if(Piece)
  {
    Piece->Color = Color;
  }
}

inline render_entry_coordinate_system *CoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
							loaded_bitmap *Texture, loaded_bitmap *NormalMap,
							environment_map *Top, environment_map *Middle, environment_map *Bottom) 
{
  render_entry_coordinate_system *Piece = PushRenderElement(Group, render_entry_coordinate_system);
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

  return Piece;
}

