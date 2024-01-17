#if !defined(NENJIN_STRING_H)
// TODO(kaelan): This function is also in the win32_main.cpp. Should I create a string library?
internal s32
S32StringLength(char *string) {
    s32 result = 0;
    while(*string++)
    {
        ++result;
    }
    return result;
}
// Draw a string to a given position on the screen.
// TODO(kaelan): Need to determine where some of these string functions go.
//               This function for example, does not get used in the platform layer, but S32StringLength could. 
//               Should the functions that could be used in the platform layer go into nenjin_platform.h
internal void
DrawString(nenjin_offscreen_buffer *buffer, font_bitmap *font_map, f32 fx, f32 fy, char *string) {
    u32 string_length = S32StringLength(string);
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
// IMPORTANT: The string funcitons below are NOT general purpose!
// NOTE: For u8's the max size of the string would be 3.
internal void
ToHexStringU8(u8 value, char *dest) {
    const u32 string_size = 3;
    char buf[string_size];
    _itoa_s(value, buf, 16);
    // Prepend 0.
    if(value < 0x10)
    {
        buf[1] = buf[0];
        buf[0] = '0';
        buf[2] = 0;
    }
    StringCopy(string_size, buf, string_size, dest);
}
internal void
ToHexStringU16(u16 value, char *dest){
    const u32 string_size = 5;
    char buf[string_size];
    _itoa_s(value, buf, 16);
    // Prepend zeros.
    if(value < 0x10)
    {
        buf[3] = buf[0];
        buf[0] = '0';
        buf[1] = '0';
        buf[2] = '0';
        buf[4] = 0;
    }
    else if(value < 0x100)
    {
        buf[3] = buf[1];
        buf[2] = buf[0];
        buf[0] = '0';
        buf[1] = '0';
        buf[4] = 0;
    }
    else if(value < 0x1000)
    {
        buf[3] = buf[2];
        buf[2] = buf[1];
        buf[1] = buf[0];
        buf[0] = '0';
        buf[4] = 0;
    }
    // TODO(kaelan): Prefix 0's need to be added to the string.
    StringCopy(string_size, buf, string_size, dest);

}
internal void
ToFlagStringU8(u8 value, char* dest) {
    const s32 string_size = 9;
    char buf[string_size] = "";
    s32 bit = 0;
    for(s32 index = 7; index >= 0; --index)
    {
        ((value >> index) & 1) ? buf[bit] = '1' : buf[bit] = '0';
        ++bit;
    }
    buf[8] = 0;
    StringCopy(string_size, buf, string_size, dest);

}
#define NENJIN_STRING_H
#endif