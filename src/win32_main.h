#if !defined(WIN32_MAIN_H)
// This struct contains relevant screen back buffer variables.
// It is used to init and pass in all values related to the buffer.
// I believe Casey used the term "offscreen" to indicate that this 
// buffer is copied to the window. Not 100% sure why it is not called
// win32_back_buffer/win32_screen_back_buffer, or something similar.
struct win32_offscreen_buffer {
	// Pixels are 32-bits wide, memory order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int WidthInBytes;
    // Each pixel is 4 bytes (32 bits)
    int BytesPerPixel;
};
// Simple struct for storing window dimensions
struct win32_window_dimensions {
    int Width;
    int Height;
};

// Vars needed to make a sound wave
struct win32_sound_output {
    int SamplesPerSecond;
    uint32_t RunningSampleIndex;
    int WavePeriod;
    int BytesPerSample; // Two 16bit channels
    DWORD SecondaryBufferSize;
	DWORD SafetyBytes;
    int LatencySampleCount;
};
struct win32_debug_time_marker {
	DWORD OutputPlayCursor;
	DWORD OutputWriteCursor;
	DWORD OutputLocation;
	DWORD OutputByteCount;

	DWORD ExpectedFlipCursor;
	DWORD FlipPlayCursor;
	DWORD FlipWriteCursor;
};

struct win32_engine_code {
	HMODULE EngineDLL;
	FILETIME DLLLastWriteTime;
	// Note: Either callback function here can be null.
	engine_update_and_render *UpdateAndRender;
	engine_get_sound_samples *GetSoundSamples;
	bool32 IsValid;
};
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer {
	HANDLE FileHandle;
	HANDLE MemoryMap;
	char FileDirectory[WIN32_STATE_FILE_NAME_COUNT];
	void *MemoryBlock;
};
struct win32_state {
	uint64_t TotalMemorySize;
	void *EngineMemoryBlock;
	win32_replay_buffer ReplayBuffers[4];

	HANDLE RecordingHandle;
	int InputRecordingIndex;

	HANDLE PlaybackHandle;
	int InputPlayingIndex;


	char EXEDirectory[WIN32_STATE_FILE_NAME_COUNT];
	char *OnePastLastEXEDirectorySlash;
};

#define WIN32_MAIN_H
#endif
