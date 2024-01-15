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
	int half_transition_count;
	bool32 ended_down;
} nenjin_button_state;
// NOTE: With this setup, keys can't really be re-bound easily. 
// TODO(kaelan): Move the key processing to the engine layer so keys can be re-bound?
typedef struct nenjin_controller_input 
{
	bool32 is_connected;
	// TODO(kaelan): Need to add controller support!
	bool32 analog;
	// Joystick
	f32 left_stick_average_x;
	f32 left_stick_average_y;
	bool32 pause_emulator;

	// Buttons available on the gb.
	union 
	{
		nenjin_button_state buttons[16];
		struct 
		{
			nenjin_button_state up;
			nenjin_button_state down;
			nenjin_button_state left;
			nenjin_button_state right;

			nenjin_button_state a;
			nenjin_button_state b;

			nenjin_button_state start;
			nenjin_button_state select;
			// TODO(kaelan): Do I want these set as buttons?
			nenjin_button_state step_frame;
			nenjin_button_state save_state;

			nenjin_button_state load_rom;
			nenjin_button_state reset;
			nenjin_button_state load_rom_abs;
			nenjin_button_state rom_up;
			nenjin_button_state rom_down;
			nenjin_button_state rom_select;

			// All buttons must be added before this line
			nenjin_button_state terminator;
		};
	};
} nenjin_controller_input;
typedef struct nenjin_input 
{
	nenjin_button_state mouse_buttons[5];
	s32 mouse_x, mouse_y, mouse_z;
	f32 d_time_for_frame;
	nenjin_controller_input controllers[2];
} nenjin_input;
inline nenjin_controller_input *
GetController(nenjin_input *input, int controller_index) {
	Assert(controller_index < ArrayCount(input->controllers));
	nenjin_controller_input *result = &input->controllers[controller_index];
	return result;
}
// TODO(kaelan): Need to figure out where this should actually go.
// NOTE: This struct is made so strings can be stored in an array
#define MAX_DIR_STRING MAX_PATH
typedef struct directory_string 
{
	char value[MAX_DIR_STRING];
} directory_string;
typedef struct directory_string_array
{
	directory_string *strings;
	s32 size;
} directory_string_array;
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
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *file_name)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_FIND_ROM_FILE(name) void name(char **file_name)
typedef DEBUG_PLATFORM_FIND_ROM_FILE(debug_platform_find_rom_file);

#define DEBUG_PLATFORM_GET_ROM_DIRECTORY(name) void name(directory_string_array *string_array)
typedef DEBUG_PLATFORM_GET_ROM_DIRECTORY(debug_platform_get_rom_directory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(char *file_name, u32 memory_size, void *memory)
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
	debug_platform_find_rom_file *DEBUGPlatformFindROMFile;
	debug_platform_get_rom_directory *DEBUGPlatformGetROMDirectory;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
} nenjin_memory;

// Definitions for callback functions
#define NENJIN_UPDATE_AND_RENDER(name) void name(nenjin_memory *memory, nenjin_input *input, nenjin_offscreen_buffer *buffer)
typedef NENJIN_UPDATE_AND_RENDER(nenjin_update_and_render);
#ifdef __cplusplus
}
#endif

#define NENJIN_PLATFORM_H
#endif
// NOTE: Sound stuff.
#if 0
#define NENJIN_GET_SOUND_SAMPLES(name) void name(nenjin_memory *Memory, nenjin_sound_output_buffer *SoundBuffer)
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
