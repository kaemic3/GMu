#include "nenjin_render.h"
// NOTE: Keeping coordinate system relative to the top left corner.
// TODO(kaelan): Need a function that will draw the PPU vram to the screen!
// TODO(kaelan): Need to add support for ttf fonts using STB!
// TODO(kaelan): Add alpha blending for all draw functions?

// NOTE: Game Boy screen is 160 x 144 pixels, and has a 10:9 aspect ratio.

internal void
DrawRectangle(nenjin_offscreen_buffer *buffer, f32 f_min_x, f32 f_min_y, f32 f_max_x, f32 f_max_y, f32 r, f32 g, f32 b) {
	s32 min_x = RoundFloat32ToS32(f_min_x);
	s32 max_x = RoundFloat32ToS32(f_max_x);
	s32 min_y = RoundFloat32ToS32(f_min_y);
	s32 max_y = RoundFloat32ToS32(f_max_y);
	if(min_x < 0) 
	{
		min_x = 0;
	}
	if(min_y < 0) 
	{
		min_y = 0;
	}
	if(max_x > buffer->width) 
	{
		max_x = buffer->width;
	}
	if(max_y > buffer->height)
	{
		max_y = buffer->height;
	}
	// Color bit pattern 0x AA RR GG BB
	u32 color = (u32)(RoundFloat32ToU32(r * 255.0f) << 16 | 
								RoundFloat32ToU32(g * 255.0f) << 8 | 
								RoundFloat32ToU32(b * 255.0f) << 0);
    // Point dest to the part of the back buffer that the rectangle will be drawn to.
    u8 *dest_row = (u8 *)buffer->memory + min_x*buffer->bytes_per_pixel + min_y*buffer->width_in_bytes;
	for(s32 y = min_y; y < max_y; ++y)
	{
        u32 *dest = (u32 *)dest_row;
		for(s32 x = min_x; x < max_x; ++x)
		{
			*dest++ = color;
		}
		dest_row += buffer->width_in_bytes;
	}
}
internal void
ClearBackBufferToBlack(nenjin_offscreen_buffer *buffer) {
    DrawRectangle(buffer, 0.0f, 0.0f, (f32)buffer->width, (f32)buffer->height, 0.0f, 0.0f, 0.0f);
}
// TODO(kaelan): Add an option to disable blending?
internal void
DrawBitmap(nenjin_offscreen_buffer *buffer, loaded_bitmap *bitmap, f32 fx, f32 fy) {
	s32 min_x = RoundFloat32ToS32(fx);
	s32 max_x = RoundFloat32ToS32(fx + bitmap->width);
	s32 min_y = RoundFloat32ToS32(fy);
	s32 max_y = RoundFloat32ToS32(fy + bitmap->height);

	s32 source_offset_x = 0;
	if(min_x < 0) 
	{
		source_offset_x = -min_x;
		min_x = 0;
	}
	s32 source_offset_y = 0;
	if(min_y < 0) 
	{
		source_offset_y = -min_y;
		min_y = 0;
	}
	if(max_x > buffer->width) 
	{
		max_x = buffer->width;
	}
	if(max_y > buffer->height)
	{
		max_y = buffer->height;
	}

	// NOTE: source_row is in pixels not bytes!
	u32 *source_row = bitmap->memory + bitmap->width*(bitmap->height-1);
	source_row += source_offset_x + (-source_offset_y * bitmap->width);
	u8 *dest_row = ((u8 *)buffer->memory + min_x*buffer->bytes_per_pixel + min_y*buffer->width_in_bytes);
	for(s32 y = min_y; y < max_y; ++y)
	{
		u32 *dest = (u32 *)dest_row;
		u32 *source = source_row;
		for(s32 x = min_x; x < max_x; ++x)
		{	
			// NOTE: This is very slow!!!!!!!
			// NOTE: The divide by 255.0f here, normalizes the Alpha channel. This is so we can do linear blending!
			f32 a = (f32)(((*source >> 24) & 0xff)) / 255.0f;
			f32 source_r = (f32)((*source >> 16) & 0xff);
			f32 source_g = (f32)((*source >> 8) & 0xff);
			f32 source_b = (f32)((*source >> 0) & 0xff);
			f32 dest_r = (f32)((*dest >> 16) & 0xff);
			f32 dest_g = (f32)((*dest >> 8) & 0xff);
			f32 dest_b = (f32)((*dest >> 0) & 0xff);
			// Linear blend equation.
			// NOTE: This is not premultiplied alpha.
			f32 r = (1.0f-a)*dest_r + (a*source_r);
			f32 g = (1.0f-a)*dest_g + (a*source_g);
			f32 b = (1.0f-a)*dest_b + (a*source_b);
			// Rebuild color and store at Dest.
			// NOTE: This uses rounding by truncation. We add 0.5f to change the bias since truncation will always round down.
			// NOTE: This method of rounding only works with positive numbers, which is fine for u32's.
			*dest++ = ((u32)(r + 0.5f) << 16) | ((u32)(g + 0.5f) << 8) | ((u32)(b + 0.5f) << 0);
			++source;
		}
		dest_row += buffer->width_in_bytes;
		// NOTE: We changed the SourceRow pointer to point at the end of the pixel buffer, so we need to
		// 		 decrement the pointer rather than increment it!
		// NOTE: Also notice that SourceRow is u32, so it is moving by pixels rather than bytes! 
		source_row -= bitmap->width;
	}
}
internal void
DrawBitmap(nenjin_offscreen_buffer *buffer, loaded_bitmap *bitmap, f32 fx, f32 fy, s32 align_x, s32 align_y) {
	f32 aligned_x = fx - (f32)align_x;
	f32 aligned_y = fy - (f32)align_y;
	DrawBitmap(buffer, bitmap, aligned_x, aligned_y);
}

// Load BMP file into memory!
// TODO(kaelan): ReadEntireFile will allocate memory using VirtualAlloc (windows). i.e. the memory
//				 for this function is allocated by the platform, for every file we read! 
//				 This memory should allocated to our exisiting memory store??
internal loaded_bitmap 
DEBUGLoadBMP(debug_platform_read_entire_file *ReadEntireFile, char *file_name) {
	loaded_bitmap result = {};
	debug_read_file_result file_contents = ReadEntireFile(file_name);
	// NOTE(kaelan): Looks like GIMP uses BITMAPV5HEADER. It encodes the color masks in the DIB header.
	// 				 This can be extracted to determine the order of pixels!
	// https://en.wikipedia.org/wiki/BMP_file_format 
	if(file_contents.contents && file_contents.content_size != 0) 
	{
		bitmap_header *header = (bitmap_header *)file_contents.contents;
		u32 *pixels = (u32 *)((u8 *)file_contents.contents + header->bitmap_offset);
		result.memory = pixels;
		result.width = header->width;
		result.height = header->height;
		// Make sure that we are loading a file that usees bitfields!
		Assert(header->compression == 3);

		s32 alpha_mask = header->alpha_mask;
		s32 red_mask = header->red_mask;
		s32 green_mask = header->green_mask;
		s32 blue_mask = header->blue_mask;
		// Use BitScan to find the shift value for each mask. We are trying to shift the corresponding value to the first eight bits,
		// and then shifting into place accoriding to how we want the memory pushed into the offscreen buffer.
		bit_scan_result alpha_scan = FindLeastSignificantSetBit(alpha_mask);
		bit_scan_result red_scan = FindLeastSignificantSetBit(red_mask);
		bit_scan_result green_scan = FindLeastSignificantSetBit(green_mask);
		bit_scan_result blue_scan = FindLeastSignificantSetBit(blue_mask);
		Assert(alpha_scan.found);
		Assert(red_scan.found);
		Assert(green_scan.found);
		Assert(blue_scan.found);

		u32 alpha_shift = alpha_scan.index;
		u32 red_shift = red_scan.index;
		u32 green_shift = green_scan.index;
		u32 blue_shift = blue_scan.index;

		u32 *source_dest = pixels;
		for(s32 Y = 0; Y < header->height; ++Y)
		{
			for(s32 X = 0; X < header->width; ++X)
			{
				u32 color = *source_dest;
				*source_dest++ = ((color >> alpha_shift) & 0xff) << 24 | ((color >> red_shift) & 0xff) << 16 | 
								((color >> green_shift) & 0xff) << 8 | ((color >> blue_shift) & 0xff) << 0;
			}
		}
	}
	else
	{
		Assert(false);
	}
	return result;
}
inline u32
NenjinColorToU32(nenjin_color *color) {
    u32 result = 0;
    f32 a = color->alpha;
    f32 r = color->red;
    f32 g = color->green;
    f32 b = color->blue;
    // De-normalize.
    result = (u32)(RoundFloat32ToU32(a * 255.0f) << 24 |
                   RoundFloat32ToU32(r * 255.0f) << 16 | 
				   RoundFloat32ToU32(g * 255.0f) << 8 | 
				   RoundFloat32ToU32(b * 255.0f) << 0);
    return result;
}
inline nenjin_color *
GetNenjinColor(u32 color_index, gb_color_palette *palette) {
	nenjin_color *result = 0;
	enum gb_color
    {
        WHITE, LIGHT_GRAY, DARK_GRAY, BLACK
    };
	switch(color_index)
    {
		case WHITE:
		{
			result = &palette->index_0;
		} break;
		case LIGHT_GRAY:
		{
			result = &palette->index_1;
		} break;
		case DARK_GRAY:
		{
			result = &palette->index_2;
		} break;
		case BLACK:
		{
			result = &palette->index_3;
		} break;
	}
	Assert(result);
	return result;
}
// TODO(kaelan): Create a DrawPixel funciton?
// TODO(kaelan): Need to scale the render up!
internal void
DrawGameBoyScreen(nenjin_offscreen_buffer *buffer, Bus *gb, gb_color_palette *palette, u32 scale_factor) {
    // GameBoy has 4 colors
    // 0x00 White 0x01 Light gray 0x02 Dark gray 0x03 Black
    // TODO(kaelan): Create a color palette for these?
    // NOTE: GameBoy screen size is 160x144 pixles
    u32 screen_width = 160;
    u32 screen_height = 144;
	u32 scaled_screen_width = scale_factor * screen_width;
	u32 scaled_screen_height = scale_factor * screen_height;
    u8 *dest_row = (u8*)buffer->memory;

	// Draw 4 X pixels and 4 Y pixels, for every pixel in gb screen memory.'
	// TODO(kaelan): At this point, it can be made to have a scale factor passed in, rather than having a hardcoded one.
	//				 Is this a good idea??
	u32 gb_screen_index = 0;
	for(u32 y = 1; y <= scaled_screen_height; ++y)
	{
		u32 *dest = (u32 *)dest_row;
		for(u32 x = 0; x < screen_width; ++x)
		{
			// Get gb color
			u32 screen_color = gb->screen[gb_screen_index];
			nenjin_color *screen_nenjin_color = GetNenjinColor(screen_color, palette);
			u32 u32_color = NenjinColorToU32(screen_nenjin_color);
			for(u32 index = 0; index < scale_factor; ++index)
			{
				*dest++ = u32_color;
			}
			++gb_screen_index;
		}

		if(y % scale_factor != 0)
		{
			gb_screen_index -= 160;
		}
        dest_row += buffer->width_in_bytes;
	}
}

