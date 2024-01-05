#if !defined(NENJIN_H)

#include "nenjin_platform.h"
#include "nenjin_intrinsic.h"
#include "nenjin_render.h"

// Engine internal functions and structs

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
	u32 *pixels;
};
struct nenjin_state 
{
	memory_arena cartridge_arena;
	loaded_bitmap test_txt;
	Bus *game_boy_bus;
    std::shared_ptr<Cartridge> gb_cart;

};
#define NENJIN_H
#endif

