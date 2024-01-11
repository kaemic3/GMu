#if !defined(NENJIN_STRING_H)
// TODO(kaelan): This function is also in the win32_main.cpp. Should I create a string library?
internal u32
U32StringLength(char *string) {
    u32 result = 0;
    while(*string++)
    {
        ++result;
    }
    return result;
}
// Draw a string to a given position on the screen.
internal void
DrawString(nenjin_offscreen_buffer *buffer, font_bitmap *font_map, f32 fx, f32 fy, char *string) {
    u32 string_length = U32StringLength(string);
    f32 x = fx;
    f32 y = fy;
    for(u32 string_index = 0; string_index < string_length; ++string_index)
    {
        DrawBitmap(buffer, &font_map[*string].bitmap, x + font_map[*string].x_offset, y + font_map[*string].y_offset);
        x += font_map[*string++].font_size;
    }
}
// TODO(kaelan): This function exists in win32_main.cpp. Pull this out into its own file?
internal void
CatString(size_t a_count, char *a, size_t b_count, 
		   char *b, size_t dest_count, char *dest) {
	// TODO(kaelan): Dest bounds checking.
	for(size_t index = 0; index < a_count; ++index) 
	{
		*dest++ = *a++;
	}
	for(size_t index = 0; index < b_count; ++index) 
	{
		*dest++ = *b++;
	}
	*dest++ = 0;
}
// TODO(kaelan): Enforce size safety.
internal void
StringCopy(u32 source_length, char *source, u32 dest_length, char *dest) {
    Assert(dest_length >= source_length);
    while(*source)
    {
        *dest++ = *source++;
    }
}
// NOTE: For u8's the max size of the string would be 3.
internal void
ToHexStringU8(u8 value, char *dest) {
    const u32 string_size = 3;
    char buf[string_size];
    _itoa_s(value, buf, 16);
    if(value < 16)
    {
        // Prepend 0.
        buf[1] = buf[0];
        buf[0] = '0';
        buf[2] = '\0';
    }
    StringCopy(string_size, buf, string_size, dest);
}
#define NENJIN_STRING_H
#endif