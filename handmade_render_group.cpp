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

internal void DrawRectangle(loaded_bitmap *Buffer, v2 vMin, v2 vMax, real32 R, real32 G, real32 B, real32 A = 1.0f)
{
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
 
  uint32 Color = (uint32)(RoundReal32ToUInt32(255 * A) << 24 |
			  RoundReal32ToUInt32(255 * R) << 16 |
			  RoundReal32ToUInt32(255 * G) << 8 |
			  RoundReal32ToUInt32(255 *B) << 0);
  
  uint8* Row = ((uint8 *)Buffer->Memory + MinX*BITMAP_BYTES_PER_PIXEL + MinY*Buffer->Pitch); 

  for(int Y = MinY; Y < MaxY; ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
    for(int X = MinX; X < MaxX; ++X)
    {
      *Pixel++ = Color;
    }
    Row += Buffer->Pitch;
  }
}

internal void DrawRectangleSlow(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color, loaded_bitmap *Texture)
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

  real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
  real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

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
	
	int32 XMap = (int32)tX;
	int32 YMap = (int32)tY;

	real32 fX = tX - (real32)XMap;
	real32 fY = tY - (real32)YMap;

	uint8 *TexelPtr = ((uint8 *)Texture->Memory) + YMap*Texture->Pitch + XMap*sizeof(uint32);
	uint32 TexelPtrA = *(uint32 *)(TexelPtr);
	uint32 TexelPtrB = *(uint32 *)(TexelPtr + sizeof(uint32));
	uint32 TexelPtrC = *(uint32 *)(TexelPtr + Texture->Pitch);
	uint32 TexelPtrD = *(uint32 *)(TexelPtr + Texture->Pitch + sizeof(uint32));
	
        v4 TexelA = v4{(real32)((TexelPtrA >> 16) & 0xFF),
		       (real32)((TexelPtrA >> 8) & 0xFF),
		       (real32)((TexelPtrA >> 0) & 0xFF),
		       (real32)((TexelPtrA >> 24) & 0xFF)};
		
        v4 TexelB = v4{(real32)((TexelPtrB >> 16) & 0xFF),
		       (real32)((TexelPtrB >> 8) & 0xFF),
		       (real32)((TexelPtrB >> 0) & 0xFF),
		       (real32)((TexelPtrB >> 24) & 0xFF)};
		
        v4 TexelC = v4{(real32)((TexelPtrC >> 16) & 0xFF),
		       (real32)((TexelPtrC >> 8) & 0xFF),
		       (real32)((TexelPtrC >> 0) & 0xFF),
		       (real32)((TexelPtrC >> 24) & 0xFF)};
		
        v4 TexelD = v4{(real32)((TexelPtrD >> 16) & 0xFF),
		       (real32)((TexelPtrD >> 8) & 0xFF),
		       (real32)((TexelPtrD >> 0) & 0xFF),
		       (real32)((TexelPtrD >> 24) & 0xFF)};

	TexelA = SRGB255ToLinear1(TexelA);
	TexelB = SRGB255ToLinear1(TexelB);
	TexelC = SRGB255ToLinear1(TexelC);
	TexelD = SRGB255ToLinear1(TexelD);

	v4 Texel = Lerp(Lerp(TexelA, fX, TexelB), fY, Lerp(TexelC, fX, TexelD));

	real32 SA = Texel.a;
	real32 SR = Texel.r;
	real32 SG = Texel.g;
	real32 SB = Texel.b;
	
	real32 RSA = SA*Color.a; 

	v4 Dest = {(real32)((*Pixel >> 16) & 0xFF),
		   (real32)((*Pixel >> 8) & 0xFF),
		   (real32)((*Pixel >> 0) & 0xFF),
		   (real32)((*Pixel >> 24) & 0xFF)};

	Dest = SRGB255ToLinear1(Dest);
	  
	real32 RDA = Dest.a; 
	real32 InvRSA = (1.0f - RSA);

	v4 Blended255 = {InvRSA*Dest.r + Color.a*SR*Color.r,
			 InvRSA*Dest.g + Color.a*SG*Color.g,
			 InvRSA*Dest.b + Color.a*SB*Color.b,
			 (RSA + RDA - RSA*RDA)};
	

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
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMax.x + T, vMin.y + T}, Color.r, Color.g, Color.b);
  DrawRectangle(Buffer, v2{vMin.x - T, vMax.y - T}, v2{vMax.x + T, vMax.y + T}, Color.r, Color.g, Color.b);
  //Left and Right
  DrawRectangle(Buffer, v2{vMin.x - T, vMin.y - T}, v2{vMin.x + T, vMax.y + T}, Color.r, Color.g, Color.b);
  DrawRectangle(Buffer, v2{vMax.x - T, vMin.y - T}, v2{vMax.x + T, vMax.y + T}, Color.r, Color.g, Color.b);
}


internal void DrawBitmap(loaded_bitmap *Buffer, loaded_bitmap *Bitmap, real32 realX, real32 realY)
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
      real32 SA = (real32)((*Source >> 24) & 0xFF);
      real32 SR = (real32)((*Source >> 16) & 0xFF);
      real32 SG = (real32)((*Source >> 8) & 0xFF);
      real32 SB = (real32)((*Source >> 0) & 0xFF);
      real32 RSA = SA / 255.0f; 

      real32 DA = (real32)((*Dest >> 24) & 0xFF);
      real32 DR = (real32)((*Dest >> 16) & 0xFF);
      real32 DG = (real32)((*Dest >> 8) & 0xFF);
      real32 DB = (real32)((*Dest >> 0) & 0xFF);
      real32 RDA = DA / 255.0f;
      
      real32 InvRSA = (1.0f - RSA);
      real32 A = 255.0f*(RSA + RDA - RSA*RDA);
      real32 R = InvRSA*DR + SR;
      real32 G = InvRSA*DG + SG;
      real32 B = InvRSA*DB + SB;

      *Dest = (((uint32)(A + 0.5f) << 24) |
	       ((uint32)(R + 0.5f) << 16) |
	       ((uint32)(G + 0.5f) << 8) |
	       ((uint32)(B + 0.5f) << 0));
      
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

   switch(Header->Type)
   {
     case RenderGroupEntryType_render_entry_clear:
     {
       render_entry_clear *Entry = (render_entry_clear *)Header;

       DrawRectangle(OutputTarget, v2{0, 0}, v2{(real32)OutputTarget->Width, (real32)OutputTarget->Height}, Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
       
       BaseAdress += sizeof(render_entry_clear);
     } break;

     case RenderGroupEntryType_render_entry_bitmap:
     {
       render_entry_bitmap *Entry = (render_entry_bitmap *)Header;
       v2 P =  GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
       //Assert(Entry->Bitmap);
       DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y);
       
       BaseAdress += sizeof(render_entry_bitmap);
     } break;
       
     case RenderGroupEntryType_render_entry_rectangle:
     {
       render_entry_rectangle *Entry = (render_entry_rectangle *)Header;
       v2 P =  GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenCenter);
       DrawRectangle(OutputTarget, P, P + Entry->Dim, Entry->R, Entry->G, Entry->B);

       BaseAdress += sizeof(render_entry_rectangle);
     } break;

     case RenderGroupEntryType_render_entry_coordinate_system:
     {
       render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Header;
       v2 Dim = {4, 4};
       v2 P = Entry->Origin;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color.r, Entry->Color.g, Entry->Color.b);

       P = Entry->Origin + Entry->XAxis;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color.r, Entry->Color.g, Entry->Color.b);

       P = Entry->Origin + Entry->YAxis;
       DrawRectangle(OutputTarget, P - Dim, P + Dim, Entry->Color.r, Entry->Color.g, Entry->Color.b);

       v2 Point = Entry->Point;
       Point = Entry->Origin + Point.x*Entry->XAxis + Point.y*Entry->YAxis;

       P = Entry->Origin + Entry->YAxis;
       DrawRectangleSlow(OutputTarget, Entry->Origin, Entry->XAxis, Entry->YAxis, Entry->Color, Entry->Texture);

       BaseAdress += sizeof(render_entry_coordinate_system);
     } break;

     InvalidDefaultCase;
   }  
 }
}

#define PushRenderElement(Group, Type) (Type*)PushRenderElement_(Group, sizeof(Type), RenderGroupEntryType_##Type)
inline render_group_entry_header *PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
  render_group_entry_header *Result = 0;
  if((Group->PushBufferSize + Size) < Group->MaxPushBufferSize)
  {
    Result = (render_group_entry_header*)(Group->PushBufferBase + Group->PushBufferSize);
    Result->Type = Type;
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
    Piece->R = Color.r;
    Piece->G = Color.g;
    Piece->B = Color.g;
    Piece->A = Color.a;
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
    Piece->R = Color.r;
    Piece->G = Color.g;
    Piece->B = Color.b;
    Piece->A = Color.a;
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

inline render_entry_coordinate_system *CoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color, loaded_bitmap *Texture)
{
  render_entry_coordinate_system *Piece = PushRenderElement(Group, render_entry_coordinate_system);
  if(Piece)
  {
    Piece->Origin = Origin;
    Piece->XAxis = XAxis;
    Piece->YAxis = YAxis;
    Piece->Color = Color;
    Piece->Texture = Texture;
  }

  return Piece;
}

