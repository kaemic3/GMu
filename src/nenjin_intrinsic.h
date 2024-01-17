#if !defined(NENJIN_INTRINSICS_H)
// TODO(kaelan): Add a preprocessor check for LLVM compiler intrinsics.
#if COMPILER_MSVC
#include <intrin.h>
#endif
#include <math.h>
//TODO(kaelan): Convert all of these functions to use intrinsics.
inline s32
RoundFloat32ToS32(f32 value) {
	// TODO(kaelan): Implement rounding for floats.
	// Use CRT for now
	s32 result = (s32)roundf(value);
	return result;
}
inline u32
RoundFloat32ToU32(f32 value) {
	u32 result = (u32)roundf(value);
	return result;
}
inline s32
FloorFloat32ToS32(f32 value) {
	s32 result = (s32)floorf(value);
	return result;
}
inline s32
TruncateFloat32ToS32(f32 value) {
	s32 result = (s32)value;
	return result;
}
inline f32
Sin(f32 angle) {
	f32 result = sinf(angle);
	return result;
}
inline f32
Cos(f32 angle) {
	f32 result = cosf(angle);
	return result;
}
inline f32
ATan2(f32 y, f32 x) {
	f32 result = atan2f(y, x);
	return result;
}
// Find the first bit set in the value, and return that bit index.
struct bit_scan_result 
{
	bool32 found;
	u32 index;
};
inline bit_scan_result 
FindLeastSignificantSetBit(u32 value) {
	bit_scan_result result = {};
	// NOTE: Both MSVC and CLANG have an intrinsic for this!
#if COMPILER_MSVC || COMPILER_LLVM 
	result.found = _BitScanForward((unsigned long *)&result.index, value);
#else
	for(u32 test = 0; test < 32; ++test)
	{
		if(value & (1 << test))
		{
			result.index = test;
			result.found = true;
			break;
		}
	}
#endif
	return result;
}
#define NENJIN_INTRINSICS_H
#endif