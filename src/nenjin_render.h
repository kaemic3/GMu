#if !defined(NENJIN_RENDER_H)
// NOTE: Colors are normalized to 1.0f.
struct nenjin_color
{
    f32 alpha;
    f32 red;
    f32 green;
    f32 blue;
};
struct gb_color_palette
{
    nenjin_color index_0;
    nenjin_color index_1;
    nenjin_color index_2;
    nenjin_color index_3;
};
// TODO(kaelan): Do we need bitmap support?
// TODO(kaelan): Add support for BITMAPV5HEADER, which seems like it will be the main BMP
//				 format that is used, at least in the modern day.
// pragma pack allows us to adjust the way the compiler packs our struct.
// This way we can read in the data directly to the struct with no issue!
#pragma pack(push, 1)
struct bitmap_header
{
	u16 file_type;
	u32 file_size;
	u16 reserved_1;
	u16 reserved_2;
	u32	bitmap_offset;
	u32 dib_size;
	s32 width;
	s32 height;
	u16 planes;
	u16 bits_per_pixel;
	u32 compression;
	u32 size_of_bitmap;
	s32 x_pels_per_meter;
	s32 y_pels_per_meter;
	u32 color_used;
	u32 color_important;
	u32 red_mask;
	u32 green_mask;
	u32 blue_mask;
	u32 alpha_mask;
};
#pragma pack(pop)
#define NENJIN_RENDER_H
#endif