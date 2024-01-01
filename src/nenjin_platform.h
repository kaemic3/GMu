#if !defined(NENJIN_PLATFORM_H)
// This header file contains all of the code that interacts with the platform layer.
/*
	NOTE:
	NENJIN_INTERNAL:
	0 - Build for public release
	1 - Build for developer only

	NENJIN_SLOW:
	0 - No slow code allowed
	1 - Slow code welcome
*/
// Defines for compiler flags that are needed to remove flase squiggles
#define NENJIN_INTERNAL 1
#define NENJIN_SLOW 1

// Determine Compiler - used in nenjin_intrinsics
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif
#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else if __CLANG__
#undef COMPILER_LLVM
#define COMPILER_LLVM 1

// TODO(kaelan): More compilers!
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
// Custom types
typedef float f32;
typedef double f64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64; 
typedef int32_t bool32;

// Utils
#define local_persist static 
#define global_variable static
#define internal static
#define Pi32 3.14159265359f

// A platform independent Assert function. Crashes the program by writing to the 0 index of
// memory, if the Expression evaluates to false. Debugger will stop at the Assert.
#if NENJIN_SLOW
// Complete assertion macro
#define Assert(expression) if (!(expression)) {*(int *)0 = 0;}
#else
#define Assert(expression)
#endif
// Simple way to count the number of elements in an array.
// We grab the total size of the array, and divide by the size of the type inside it.
#define ArrayCount(array) (sizeof(array)/sizeof((array)[0]))
// Macros for quick conversion to large magnitudes of bytes
// TODO(kaelan): Should these be 64-bit?
#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)
// TODO(kaelan): Swap min max... macros???
// Asserts that the value passed is less than the max 32-bit uint.
inline u32 
SafeTruncateU64(u64 value) {
	Assert(value < 0xFFFFFFFF);
	u32 result = (u32)value;
	return result;
}
// Engine Definitions:

// Services that the platform layer provides to the engine.
// TODO(kaelan): Services that the engine provides to the platform layer.
//	(May expand in the future.)
// Needs to take four things: timing, controller/keyboard input, bitmap buffer to use, 
// sound buffer to use
typedef struct thread_context 
{
	int placeholder;
} thread_context;

typedef struct nenjin_offscreen_buffer 
{
	// Pixels are 32-bits wide, memory order BB GG RR XX
    void *memory;
    int width;
    int height;
    int width_in_bytes;
	int bytes_per_pixel;
} nenjin_offscreen_buffer;
typedef struct nenjin_button_state {
	int HalfTransitionCount;
	bool32 EndedDown;
} nenjin_button_state;
typedef struct nenjin_controller_input 
{
	bool32 IsConnected;
	bool32 Analog;
	// Joystick
	f32 LeftStickAverageX;
	f32 LeftStickAverageY;

	// Buttons
	// Union so buttons can be accessed via array or directly through members.
	union 
	{
		nenjin_button_state Buttons[12];
		struct 
		{
			nenjin_button_state MoveUp;
			nenjin_button_state MoveDown;
			nenjin_button_state MoveLeft;
			nenjin_button_state MoveRight;

			nenjin_button_state ActionUp;
			nenjin_button_state ActionDown;
			nenjin_button_state ActionLeft;
			nenjin_button_state ActionRight;

			nenjin_button_state LeftShoulder;
			nenjin_button_state RightShoulder;

			nenjin_button_state Start;
			nenjin_button_state Back;

			// All buttons must be added before this line
			nenjin_button_state Terminator;
		};
	};
} nenjin_controller_input;
typedef struct nenjin_input 
{
	nenjin_button_state MouseButtons[5];
	s32 MouseX, MouseY, MouseZ;
	f32 dtForFrame;
	// TODO(kaelan): Insert clock values here!
	nenjin_controller_input Controllers[5];
} nenjin_input;
inline nenjin_controller_input *
GetController(nenjin_input *Input, int ControllerIndex) {
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	nenjin_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}
// DEBUG I/O
// These functions currently do not protect against bad writes!!
// The write function overwrites the old file, it should create a new
// file and replace the old one after the write is confirmed good.
// IMPORTANT: Need to fix these!!
#if NENJIN_INTERNAL
typedef struct debug_read_file_result 
{
	u32 content_size;
	void *contents;
} debug_read_file_result;
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(thread_context *thread, char *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(thread_context *thread, char *file_name, u32 memory_size, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif
typedef struct nenjin_memory 
{
	bool32 is_initialized;
	u64 permanent_storage_size;
	void *permanent_storage;	// NOTE: Required to be cleared at startup.
	u64 transient_storage_size;
	void *transient_storage; // NOTE: Required to be cleared at startup.
	
	// Pointers to File/IO functions
	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
} nenjin_memory;

// Definitions for callback functions
#define NENJIN_UPDATE_AND_RENDER(name) void name(thread_context *thread, nenjin_memory *memory, nenjin_input *input, nenjin_offscreen_buffer *buffer)
typedef NENJIN_UPDATE_AND_RENDER(nenjin_update_and_render);
#ifdef __cplusplus
}
#endif

#define NENJIN_PLATFORM_H
#endif
// NOTE: Sound stuff.
#if 0
#define NENJIN_GET_SOUND_SAMPLES(name) void name(thread_context *Thread, nenjin_memory *Memory, nenjin_sound_output_buffer *SoundBuffer)
typedef NENJIN_GET_SOUND_SAMPLES(nenjin_get_sound_samples);
#endif
#if 0
typedef struct nenjin_sound_output_buffer 
{
	int SamplesPerSecond;
	int SampleCount;
	s16 *Samples;
} nenjin_sound_output_buffer;
#endif
