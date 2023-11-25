#include "nenjin.h"

internal void
OutputSound(engine_sound_output_buffer *SoundBuffer, engine_state *EngineState, int ToneHz) {
	int16_t ToneVolume = 300;
	int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
	int16_t *SampleOut = SoundBuffer->Samples;
	for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
		f32 SineValue = sinf(EngineState->tSine);	// Generate sine value using std sinf
#if 1
		int16_t SampleValue = (int16_t)(SineValue*ToneVolume);
#else
		int16_t SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
		EngineState->tSine += 2.0f*Pi32*1.0f / (f32)WavePeriod; // Update sine wave position
		if(EngineState->tSine > 2.0f*Pi32)
			EngineState->tSine -= 2.0f*Pi32;
	}
}

// Populates the passed buffer with a blue and green gradient. It's position is relative
// to the params.
internal void
RenderWeirdGradient(engine_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset) {
    // Cast void pointer into uint8_t
    uint8_t* Row = (uint8_t*)Buffer->Memory;
    // Iterate through the whole bit map
    for (int Y = 0; Y < Buffer->Height; ++Y) {
        uint32_t *Pixel = (uint32_t *)Row;
        for (int X = 0; X < Buffer->Width; ++X) {
            uint8_t Blue = (uint8_t)(X + BlueOffset);
            uint8_t Green = (uint8_t)(Y + GreenOffset);
            *Pixel++ = (Green << 8 | Blue);
        }
		// Buffer->Pitch is the width of each row of our BitMap in bytes.
        Row += Buffer->WidthInBytes;
    }
}

internal void
RenderPlayer(engine_offscreen_buffer *Buffer, int PlayerX, int PlayerY, uint32_t Color){
	uint8_t *EndOfBuffer = (uint8_t *)Buffer->Memory + Buffer->Height*Buffer->WidthInBytes;
	int Top = PlayerY;
	int Bottom = PlayerY + 10;
	for(int X = PlayerX; X < PlayerX + 10; ++X) {
		uint8_t *Pixel = ((uint8_t *)Buffer->Memory + X*Buffer->BytesPerPixel + Top*Buffer->WidthInBytes);
		for(int Y = Top; Y < Bottom; ++Y) {
			if((Pixel + 4 >= Buffer->Memory) && (Pixel < EndOfBuffer)) {
				*(uint32_t *)Pixel = Color;
			}
			Pixel += Buffer->WidthInBytes;
		}
	}
}

extern "C" 
ENGINE_UPDATE_AND_RENDER(EngineUpdateAndRender) {
	// Assure that our Buttons union is aligned properly
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == 
			ArrayCount(Input->Controllers[0].Buttons));
	Assert((sizeof(engine_state) <= Memory->PermanentStorageSize));
	// Grab the pointer to our PermanentStorage.
	engine_state *EngineState = (engine_state *)Memory->PermanentStorage;
	if (!Memory->IsInitialized) {
		// Test BMP file
		char* FileName = __FILE__;
		// Load whole files instead of creating a stream for i/o
		debug_read_file_result FileRead = Memory->DEBUGPlatformReadEntireFile(Thread, FileName);
		if (FileRead.Contents) {
			Memory->DEBUGPlatformWriteEntireFile(Thread, "test.out", FileRead.ContentSize, FileRead.Contents);
			Memory->DEBUGPlatformFreeFileMemory(Thread, FileRead.Contents);
		}
		EngineState->ToneHz = 512;
		EngineState->PlayerX = 33;
		EngineState->PlayerY = 33;

		// May be more appropriate to do this in the platform layer.
		Memory->IsInitialized = true;
	}

	for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex) {
		engine_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->Analog) {
			// Use analog movement tuning
			EngineState->BlueOffset += (int)(4.0f*Controller->LeftStickAverageX);
			EngineState->ToneHz = 512 + (int)(128.0f*Controller->LeftStickAverageY);
		}
		else {
			if (Controller->MoveUp.EndedDown){
				EngineState->GreenOffset += (int)(-4.0f*Controller->LeftStickAverageY);
			}
			if (Controller->ActionDown.EndedDown) {
				EngineState->GreenOffset += 1;
			}
			if (Controller->ActionUp.EndedDown) {
				EngineState->GreenOffset -= 1;
			}
		}
		EngineState->PlayerX += (int)(4.0f*Controller->LeftStickAverageX);
		EngineState->PlayerY -= (int)(4.0f*Controller->LeftStickAverageY);
		if(EngineState->tJump > 0) {
			EngineState->PlayerY += (int)(5.0f*sinf(0.5f*Pi32*EngineState->tJump));
		}
		if(Controller->ActionDown.EndedDown) {
			EngineState->tJump = 4.0;
		}
		EngineState->tJump -= 0.033f;
	}
	RenderWeirdGradient(Buffer, EngineState->BlueOffset, EngineState->GreenOffset);
	RenderPlayer(Buffer, EngineState->PlayerX, EngineState->PlayerY, 0xFFFF0000);
	for(int MouseIndex = 0; MouseIndex < ArrayCount(Input->MouseButtons); ++MouseIndex) {
		if(Input->MouseButtons[MouseIndex].EndedDown) {
			RenderPlayer(Buffer, 40 + 20*MouseIndex, 40, 0x0000FFFF);
		}

	}
		RenderPlayer(Buffer, Input->MouseX, Input->MouseY, 0xFFFFFFFF);
}
extern "C"
ENGINE_GET_SOUND_SAMPLES(EngineGetSoundSamples) {
	Assert((sizeof(engine_state) <= Memory->PermanentStorageSize));
	engine_state *EngineState = (engine_state *)Memory->PermanentStorage;
	OutputSound(SoundBuffer, EngineState, EngineState->ToneHz);
}
