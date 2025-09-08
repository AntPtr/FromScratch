internal void DrawRectangle(loaded_bitmap *Buffer, v2 vMin, v2 vMax, real32 R, real32 G, real32 B)
{
  int32 MinX = RoundReal32ToInt32(vMin.X);
  int32 MaxX = RoundReal32ToInt32(vMax.X);
  int32 MinY = RoundReal32ToInt32(vMin.Y);
  int32 MaxY = RoundReal32ToInt32(vMax.Y);

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
 
  uint32 Color = (uint32)(RoundReal32ToUInt32(255 * R) << 16 | RoundReal32ToUInt32(255 * G) << 8 | RoundReal32ToUInt32(255 *B) << 0);
  
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


internal void DrawRectangleOutline(loaded_bitmap* Buffer, v2 vMin, v2 vMax, v3 Color, real32 T = 2.0f)
{
  //Top and Bottom
  DrawRectangle(Buffer, v2{vMin.X - T, vMin.Y - T}, v2{vMax.X + T, vMin.Y + T}, Color.R, Color.G, Color.B);
  DrawRectangle(Buffer, v2{vMin.X - T, vMax.Y - T}, v2{vMax.X + T, vMax.Y + T}, Color.R, Color.G, Color.B);
  //Left and Right
  DrawRectangle(Buffer, v2{vMin.X - T, vMin.Y - T}, v2{vMin.X + T, vMax.Y + T}, Color.R, Color.G, Color.B);
  DrawRectangle(Buffer, v2{vMax.X - T, vMin.Y - T}, v2{vMax.X + T, vMax.Y + T}, Color.R, Color.G, Color.B);
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
       BaseAdress += sizeof(render_entry_clear);
     } break;
       
     case RenderGroupEntryType_render_entry_rectangle:
     {
       render_entry_rectangle *Entry = (render_entry_rectangle *)Header;
       v3 EntityBaseP = Entry->Basis->P;
    
       real32 ZFudge = (1.0f + 0.1f*(EntityBaseP.Z + Entry->OffsetZ));
      

       real32 EntityGroundX = ScreenCenter.X + RenderGroup->MetersToPixel*ZFudge*EntityBaseP.X;
       real32 EntityGroundY = ScreenCenter.Y - RenderGroup->MetersToPixel*ZFudge*EntityBaseP.Y;
       real32 Z = -RenderGroup->MetersToPixel*EntityBaseP.Z;
       
       v2 Center = {EntityGroundX + Entry->Offset.X,
		    EntityGroundY + Entry->Offset.Y + Z};
       if(Entry->Bitmap)
       {
	 DrawBitmap(OutputTarget, Entry->Bitmap, Center.X, Center.Y);
       }
       else
       {
	 v2 HalfDim = 0.5f*RenderGroup->MetersToPixel*Entry->Dim;
	 DrawRectangle(OutputTarget, Center-HalfDim, Center+HalfDim, Entry->R, Entry->G, Entry->B);
       }

       BaseAdress += sizeof(render_entry_rectangle);
     } break;

     InvalidDefaultCase;
   }  
 }
}
