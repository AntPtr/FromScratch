#if !defined(HANDMADE_RENDER_GROUP_H)
struct render_basis
{
  v3 P;
};

enum render_group_entry_type
{
  RenderGroupEntryType_render_entry_clear,
  RenderGroupEntryType_render_entry_rectangle,
};

struct render_group_entry_header
{
  render_group_entry_type Type;
};

struct render_entry_clear
{
  render_group_entry_header Header;
  real32 R, G, B, A;
};

struct render_entry_rectangle
{
  render_group_entry_header Header;
  render_basis *Basis;
  loaded_bitmap *Bitmap;
  v2 Offset;
  real32 OffsetZ;
  real32 R, G, B, A;
  v2 Dim;
};

struct render_group
{
  render_basis *DefaultBasis;
  real32 MetersToPixel;

  uint32 MaxPushBufferSize;
  uint32 PushBufferSize;
  uint8 *PushBufferBase;
};

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
  render_entry_rectangle *Piece = PushRenderElement(Group, render_entry_rectangle);
  if(Piece)
  {
    Piece->Bitmap = Bitmap;
    Piece->Basis = Group->DefaultBasis;
    Piece->Offset = Group->MetersToPixel*v2{Offset.X, -Offset.Y} - Align;
    Piece->OffsetZ = OffsetZ;
    Piece->R = Color.R;
    Piece->G = Color.G;
    Piece->B = Color.B;
    Piece->A = Color.A;
    Piece->Dim = Dim;
  }
}

inline void PushBitmap(render_group *Group, loaded_bitmap *Bitmap, v2 Offset,
			 real32 OffsetZ, v2 Align, real32 Alpha = 1.0f)
{
  PushPiece(Group, Bitmap, Offset, OffsetZ, Align, v2{0, 0},v4{1.0f, 1.0f, 1.0f, Alpha});
}

inline void PushRect(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim, v4 Color)
{
  PushPiece(Group, 0, Offset, OffsetZ, v2{0, 0}, Dim, Color);
}

inline void PushRectOutline(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim, v4 Color)
{
  real32 Thickness = 0.1f;
    
  PushPiece(Group, 0, Offset - v2{0, 0.5f*Dim.Y}, OffsetZ, v2{0, 0}, v2{Dim.X, Thickness}, Color);
  PushPiece(Group, 0, Offset + v2{0, 0.5f*Dim.Y}, OffsetZ, v2{0, 0}, v2{Dim.X, Thickness}, Color);

  PushPiece(Group, 0, Offset - v2{0.5f*Dim.X, 0}, OffsetZ, v2{0, 0}, v2{Thickness, Dim.Y}, Color);
  PushPiece(Group, 0, Offset + v2{0.5f*Dim.X, 0}, OffsetZ, v2{0, 0}, v2{Thickness, Dim.Y}, Color);  
}


#define HANDMADE_RENDER_GROUP_H
#endif
