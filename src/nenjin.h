#if !defined(NENJIN_H)

#include "nenjin_platform.h"
#include "nenjin_intrinsic.h"
#include "nenjin_render.h"
// Engine internal functions and structs
#define Minimum(a, b) ((a < b) ? (a) : (b))
#define Maximum(a, b) ((a > b) ? (a) : (b))

struct memory_arena
{
	size_t size;
	size_t bytes_used;
	u8 *base_ptr;
};
internal void 
InitializeArena(memory_arena *arena, size_t size, u8 *storage) {
	arena->size = size;
	arena->base_ptr = storage;
	arena->bytes_used = 0;
}
// Macros that allow us to allocate memory for structs and arrays!
#define PushStruct(pool, type) (type *)PushSize_(pool, sizeof(type))
#define PushSize(pool, size) PushSize_(pool, size)
#define PushArray(pool, count, type) (type *) PushSize_(pool, (count)*sizeof(type))
// Allocates a chunk of memory in the pased memory arena. The size of the chunk
// is passed in, and the BytesUsed in the arena is updated accordingly.
// An assert is fired if the size of the desired allocation causes the 
// total arena size to overflow.
internal void * 
PushSize_(memory_arena *arena, size_t size) {
	Assert((arena->bytes_used + size) <= arena->size);
	void *result = arena->base_ptr + arena->bytes_used;
	arena->bytes_used += size;
	return result;
}
struct loaded_bitmap
{
	s32 width;
	s32 height;
	s32 width_in_bytes;
	u32 *memory;
};
struct font_bitmap
{
	loaded_bitmap bitmap;
	s32 x_offset;
	s32 y_offset;
	f32 font_size;
};
struct nenjin_state 
{
	bool32 run_emulator;
	memory_arena game_boy_arena;
	memory_arena bitmap_arena;
	nenjin_color font_color;
	font_bitmap font_map[128];
	// NOTE: The Game Boy bus CANNOT be a "stack" based thing, because the constructor does not get called! 
	// 		 Also, even after forcing it to be called, I had issues with memory access violations.
	//		 This is probably due to the way the bus class default intializes the CPU and PPU.
	// TODO(kaelan): Need to re-write the bus class!
	Bus *game_boy_bus;
    std::shared_ptr<Cartridge> gb_cart;
};
#define NENJIN_H
#endif
