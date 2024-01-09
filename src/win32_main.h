#if !defined(WIN32_MAIN_H)
// This struct contains relevant screen back buffer variables.
// It is used to init and pass in all values related to the buffer.
// I believe Casey used the term "offscreen" to indicate that this 
// buffer is copied to the window. Not 100% sure why it is not called
// win32_back_buffer/win32_screen_back_buffer, or something similar.
struct win32_offscreen_buffer 
{
	// Pixels are 32-bits wide, memory order BB GG RR XX
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int width_in_bytes;
    // Each pixel is 4 bytes (32 bits)
    int bytes_per_pixel;
};
// Simple struct for storing window dimensions
struct win32_window_dimensions 
{
    int width;
    int height;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_state 
{
	u64 total_memory_size;
	void *engine_memory_block;
	// TODO(kaelan): Do I want to use hot code reload?
	char exe_directory[WIN32_STATE_FILE_NAME_COUNT];
	char *one_past_last_exe_dir_slash;
};
// NOTE: This is for hot code reload, which for this project, cannot be used in current form.
#if 0
struct win32_engine_code 
{
	HMODULE engine_dll;
	FILETIME dll_last_write_time;
	// Note: Either callback function here can be null.
	nenjin_update_and_render *UpdateAndRender;
	bool32 is_valid;
};
#endif
// NOTE: This is from handmade hero. Struct for sound output with DirectSound.
// TODO(kaelan): Add DirectSound support?? Possbily use wave out instead??
#if 0
// Vars needed to make a sound wave
struct win32_sound_output 
{
    int SamplesPerSecond;
    u32 RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample; // Two 16bit channels
    DWORD SecondaryBufferSize;
	DWORD SafetyBytes;
    int LatencySampleCount;
};
struct win32_debug_time_marker 
{
	DWORD OutputPlayCursor;
	DWORD OutputWriteCursor;
	DWORD OutputLocation;
	DWORD OutputByteCount;

	DWORD ExpectedFlipCursor;
	DWORD FlipPlayCursor;
	DWORD FlipWriteCursor;
};
#endif

#define WIN32_MAIN_H
#endif
