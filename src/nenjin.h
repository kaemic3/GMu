#if !defined(NENJIN_H)
/*
	NOTE:
	ENGINE_INTERNAL:
	0 - Build for public release
	1 - Build for developer only

	ENGINE_SLOW:
	0 - No slow code allowed
	1 - Slow code welcome
*/

// TODO: Implement sin ourselves
#include <math.h>   // For testing
#include <stdint.h>

#define local_persist static 
#define global_variable static
#define internal static

#define Pi32 3.14159265359f
// Used when we dont care if the bool is 1, just that it is not 0
typedef int32_t bool32;
// Typedef for specifying different bit width floating point types
typedef float f32;
typedef double f64;



// A platform independent Assert function. Crashes the program by writing to the 0 index of
// memory, if the Expression evaluates to false. Debugger will stop at the Assert.
#if ENGINE_SLOW
// Complete assertion macro
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif
// Simple way to count the number of elements in an array.
// We grab the total size of the array, and divide by the size of the type inside it.
#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))
// Macros for quick conversion to large magnitudes of bytes
// TODO: Should these be 64-bit?
#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)
// TODO: Swap min max... macros???
// Asserts that the value passed is less than the max 32-bit uint.
inline uint32_t SafeTruncateUInt64 (uint64_t Value) {
	Assert(Value < 0xFFFFFFFF);
	uint32_t Result = (uint32_t) Value;
	return Result;
}

// Engine Definitions:

// Services that the platform layer provides to the engine.

// TODO: Services that the engine provides to the platform layer.
//	(May expand in the future.)
// Needs to take four things: timing, controller/keyboard input, bitmap buffer to use, sound buffer to use
// TODO: In the future, rendering_specifically_ will become a three-tiered abstraction!!
struct thread_context {
	int Placeholder;
};

struct engine_offscreen_buffer {
	// Pixels are 32-bits wide, memory order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int WidthInBytes;
	int BytesPerPixel;
};
struct engine_sound_output_buffer {
	int SamplesPerSecond;
	int SampleCount;
	int16_t *Samples;
};

struct engine_button_state {
	int HalfTransitionCount;
	bool32 EndedDown;
};

struct engine_controller_input {
	bool32 IsConnected;
	bool32 Analog;
	// Joystick
	f32 LeftStickAverageX;
	f32 LeftStickAverageY;

	// Buttons
	// Union so buttons can be accessed via array or directly through members.
	union {
		engine_button_state Buttons[12];
		struct{
			engine_button_state MoveUp;
			engine_button_state MoveDown;
			engine_button_state MoveLeft;
			engine_button_state MoveRight;

			engine_button_state ActionUp;
			engine_button_state ActionDown;
			engine_button_state ActionLeft;
			engine_button_state ActionRight;

			engine_button_state LeftShoulder;
			engine_button_state RightShoulder;

			engine_button_state Start;
			engine_button_state Back;

			// All buttons must be added before this line
			engine_button_state Terminator;
		};
	};
};

struct engine_input {
	engine_button_state MouseButtons[5];
	int32_t MouseX, MouseY, MouseZ;
	// TODO: Insert clock values here!
	engine_controller_input Controllers[5];
};

// DEBUG I/O
// These functions currently do not protect against bad writes!!
// The write function overwrites the old file, it should create a new
// file and replace the old one after the write is confirmed good.
// IMPORTANT: Need to fix these!!
#if ENGINE_INTERNAL
struct debug_read_file_result {
	uint32_t ContentSize;
	void *Contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *Thread, char *FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *Thread, char *FileName, uint32_t MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

struct engine_memory {
	bool32 IsInitialized;
	uint64_t PermanentStorageSize;
	void *PermanentStorage;	// NOTE: Required to be cleared at startup.
	uint64_t TransientStorageSize;
	void *TransientStorage; // NOTE: Required to be cleared at startup.
	
	// Pointers to File/IO functions
	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;

};
// Definitions for callback functions
#define ENGINE_UPDATE_AND_RENDER(name) void name(thread_context *Thread, engine_memory *Memory, engine_input *Input, engine_offscreen_buffer *Buffer)
typedef ENGINE_UPDATE_AND_RENDER(engine_update_and_render);

#define ENGINE_GET_SOUND_SAMPLES(name) void name(thread_context *Thread, engine_memory *Memory, engine_sound_output_buffer *SoundBuffer)
typedef ENGINE_GET_SOUND_SAMPLES(engine_get_sound_samples);
// Engine internal functions and structs
struct engine_state {
	int BlueOffset;
	int GreenOffset;
	int ToneHz;
	f32 tSine;

	int PlayerX;
	int PlayerY;
	f32 tJump;
};

inline engine_controller_input *GetController(engine_input *Input, int ControllerIndex) {
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	engine_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}
internal void OutputSound(engine_sound_output_buffer *SoundBuffer, engine_state *EngineState, int ToneHz);
internal void RenderWeirdGradient(engine_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);
#define NENJIN_H
#endif
