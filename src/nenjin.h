#if !defined(NENJIN_H)

#include "nenjin_platform.h"

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
#define PushStruct(Pool, type) (type *)PushSize_(Pool, sizeof(type))
#define PushArray(Pool, Count, type) (type *) PushSize_(Pool, (Count)*sizeof(type))
// Allocates a chunk of memory in the pased memory arena. The size of the chunk
// is passed in, and the BytesUsed in the arena is updated accordingly.
// An assert is fired if the size of the desired allocation causes the 
// total arena size to overflow.
// TODO(kaelan): Make a way to free memory from the pool???
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
	memory_arena bus_arena;
	memory_arena cartridge_arena;
	loaded_bitmap background;

};
#define NENJIN_H
#endif

