/* 
   This is not a final platform layer!
	
TODO:
   - Saved game locations
   - Getting a handle to our own executable file
   - Asset loading path
   - Threading (launch a thread)
   - Raw input (support for multiple keyboards)
   - Sleep/timeBeginPeriod
   - ClipCursor(Mulit-monitor suppport) 
   - Fullscreen support
   - WM_SETCURSOR (control cursor visibility)
   - QueryCancelAutoPlay
   - WMACTIVEAPP (for when we are not the active applcation)
   - Blit speed improvements (BitBlit)
   - Hardware acceleration (OpenGL or Direct3D or BOTH??)
   - GetKeyboardLayout (for French keyboards, international WASD support)
*/
#include "nenjin.h"

#include <windows.h>
#include <Xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <malloc.h>

#include "win32_main.h"
// Global for now.
global_variable bool32 GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable LARGE_INTEGER GlobalPerfCounterFrequency;

// XInputGetState

// Define function prototype
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
// Define a type from the function prototype macro
typedef X_INPUT_GET_STATE(x_input_get_state);   // x_input_get_state is the type  
// Define a stub function - the stub in this case acts as a temporary function for the function
// pointer to point to so the program does not crash when XInput is not available on the system.
X_INPUT_GET_STATE (XInputGetStateStub) {    // XInputGetStateStub is the function name
    return ERROR_DEVICE_NOT_CONNECTED;
}
// Function pointer
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
// Overwrite the XInputGetState from Xinput.h so it refers to our function pointer
#define XInputGetState XInputGetState_
// XInputSetState

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE (XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// DirectSound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);

// Basic, kinda slow, string contatenation. Will need to create a proper string library at some point!
internal void
CatStrings(size_t SourceACount, char *SourceA, size_t SourceBCount, 
		   char *SourceB, size_t DestCount, char *Dest) {
	// TODO: Dest bounds checking.
	for(size_t Index = 0; Index < SourceACount; ++Index) {
		*Dest++ = *SourceA++;
	}
	for(size_t Index = 0; Index < SourceBCount; ++Index) {
		*Dest++ = *SourceB++;
	}
	*Dest++ = 0;
}
internal int
StringLength(char *String) {
	int Length = 0;
	while(*String++) {
		++Length;
	}
	return Length;
}
// Used to direct file i/o. Lets us create path strings to specificy file locations to be written/read from/to.
internal void
Win32GetEXEDirectoryString(win32_state *Win32State) {
	DWORD SizeOfFileName = GetModuleFileNameA(0, Win32State->EXEDirectory, sizeof(Win32State->EXEDirectory));
	Win32State->OnePastLastEXEDirectorySlash = Win32State->EXEDirectory;
	for(char *Scan = Win32State->EXEDirectory; *Scan; ++Scan) {
		if(*Scan == '\\') {
			Win32State->OnePastLastEXEDirectorySlash = Scan + 1;
		}	
	}
}
internal void
Win32BuildEXEDirectoryPath(win32_state *Win32State, char *FileName, int DestCount, char *Dest) {
	CatStrings(Win32State->OnePastLastEXEDirectorySlash - Win32State->EXEDirectory, Win32State->EXEDirectory, 
			   StringLength(FileName), 
			   FileName, DestCount, Dest);
}

// DEBUG I/O

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory) {
    VirtualFree(Memory, 0, MEM_RELEASE);
}
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile) {
    debug_read_file_result Result = {};
    // Creates a file handle that will read, allow other programs to read, and will only open the file
    // if it already exists.
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, 
									OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize)) {
            // TODO: Defines for max values Uint32Max
            uint32_t FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            // Keep in mind that small allocs should not use VirtualAlloc, only used here for testing.
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, 
										   PAGE_READWRITE);
            if(Result.Contents) {
                DWORD BytesRead;
                // Keep in mind, that ReadFile can only read in up to 32-bit sized files, so in order
                // to support large file sizes, we would need to loop.
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && 
				   FileSize32 == BytesRead) {
                    Result.ContentSize = FileSize32;
                }
            }
            else {
                //Could not read the file
                // Free the memory
                DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                Result.Contents = 0;
            }
        }
        else {
            // Could not get file size
        }
        CloseHandle(FileHandle);
    }
    else {
        // Could not get Handle
    }
    return Result;
}
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile) {
    bool32 Result = false;
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE) {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)) {
            Result = (MemorySize == BytesWritten);
        }
        else {
            // File not written to disk
        }
        CloseHandle(FileHandle);
    }
    else {
        // Could not get handle
    }
    return Result;
}
// Use FindFirstFile API call to get the last write time of the passed file.
inline FILETIME
Win32GetLastWriteTime(char *FileName) {
	FILETIME Result = {};
	WIN32_FILE_ATTRIBUTE_DATA FileData;
	if(GetFileAttributesEx(FileName, GetFileExInfoStandard, &FileData)) {
		Result = FileData.ftLastWriteTime;
	}
	return Result;
}
// Load our DLL for the engine code.
internal win32_engine_code
Win32LoadEngineCode(char *SourceDLLName, char *TempDLLName) {
	win32_engine_code Result = {};

	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
	CopyFile(SourceDLLName, TempDLLName, FALSE);
	Result.EngineDLL = LoadLibraryA(TempDLLName);
	if(Result.EngineDLL) {
		Result.UpdateAndRender = (engine_update_and_render *)
			GetProcAddress(Result.EngineDLL, "EngineUpdateAndRender");
		Result.GetSoundSamples = (engine_get_sound_samples *)
			GetProcAddress(Result.EngineDLL, "EngineGetSoundSamples");
		Result.IsValid = (Result.UpdateAndRender && Result.GetSoundSamples);
	}
	else {
	}
	if(!Result.IsValid) {
		Result.UpdateAndRender = 0;
		Result.GetSoundSamples = 0;
	}
	return Result;
}
// Unload our DLL for the engine code.
internal void
Win32UnloadEngineCode(win32_engine_code *EngineCode) {
	if(EngineCode->EngineDLL) {
		FreeLibrary(EngineCode->EngineDLL);
		EngineCode->EngineDLL = 0;
	}
	EngineCode->IsValid = false;	
	EngineCode->UpdateAndRender = 0;
	EngineCode->GetSoundSamples = 0;
}
// Attempt to load the XInput library.
internal void
Win32LoadXInput(void) {
    // Try to load xinput1.4
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll"); 
    // If it fails to load, try xinput1.3: Maybe print out version that is chosen
    if(!XInputLibrary)
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    if(!XInputLibrary)
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    // If the XInput is loaded
    if(XInputLibrary) {
        // Get the address to the actual functions from Xinput.h - GetProcAddress returns
        // void pointers, so we need to cast our type
        XInputGetState = (x_input_get_state *)
			GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)
			GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else {
        OutputDebugStringA("Could not load XInput function\n");
    }
}
// Attempt to load and initialize DSound.
internal void 
Win32InitDSound(HWND Window, int32_t SamplesPerSecond, int32_t BufferSize) {
    // Load the Direct Sound library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if(DSoundLibrary) {
        // Get a DirectSound object - cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/mt708921(v=vs.85) 
        // Double check that this work on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        // DirectSound() does not a IDirectSound object, instead it will use &DirectSound as an out parameter for that.
        // We also need to check if we were able to create a DirectSound object with the SUCEEDED macro.
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))) {
            // Create a wave format for our sound output - This tells DirectSound about the kind of sound we want it to output
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            // Number of bits for a full sameple - 32 bits / 8 for bytes
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            // Size of a block times the number of samples per second
            WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign*WaveFormat.nSamplesPerSec;
            WaveFormat.cbSize = 0;
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))) {
                // Create a primary buffer
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                BufferDescription.dwSize = sizeof(BufferDescription);
                LPDIRECTSOUNDBUFFER PrimaryBuffer = {};
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))) {

                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))) {
                        // Format now set
                        OutputDebugStringA("PrimaryBuffer format set.\n");
                    }
                    else {
                        // Diagnositc
                    }
                }
                else {
                    // Diagnostic
                }
            }
            else {
                // Diagnostic
            }
            // Create a secondary buffer
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))) {
                OutputDebugStringA("SecondaryBuffer created successfully.\n");
            }
            // Start it playing!!!!
        }
        else {
            // Diagnostic
        }
    }
}
// Return the current window dimensions.
internal win32_window_dimensions
Win32GetWindowDimensions(HWND Window) {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    win32_window_dimensions WindowDimensions;
    WindowDimensions.Width = ClientRect.right - ClientRect.left;
    WindowDimensions.Height = ClientRect.bottom - ClientRect.top;
    return WindowDimensions;
}
// DIB - Device Independent BitMap
// Reallocates the buffer when the window is resized. Note that this only changes
// the size of the memory area, and not what is in the buffer per say.
internal void 
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height) {

    if(Buffer->Memory) {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;  // This is negative so the pixels we input into the buffer memory go from top left to bottom right.
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    // Allocate memory for bitmap
    // 32 bits per pixel only for padding. RGB needs 24 (8 bits R, 8 bits G 8 bits B), but it is slower to access memory that is aligned to 24 bits
    // rather than 32 bits.
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    // Allocate memory for the Bitmap, read and write access
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    // TODO: Probably clear this to black.
    Buffer->WidthInBytes = Buffer->Width*Buffer->BytesPerPixel; 
	// Set pixels to be 32-bit/4 bytes
	Buffer->BytesPerPixel = 4;
}
// Uses the StretchDIBits function to display the current back buffer to the window.
internal void 
Win32DisplayBufferInWindow(HDC DeviceContext, win32_offscreen_buffer *Buffer, int WindowWidth, 
		int WindowHeight) {
	// This can be stretched by passing the WindowWidth and WindowHeight params into StretchDIBits, instead
	// of only passing in the buffer width and height values. For the purposes of learning to write a renderer,
	// for now the image will not stretch to fill the window.
    StretchDIBits(DeviceContext, 0, 0, Buffer->Width, Buffer->Height, 0, 0, Buffer->Width, Buffer->Height, Buffer->Memory, 
			&Buffer->Info, DIB_RGB_COLORS, SRCCOPY);
}
// This handles Window messages. Closing, resizing etc.
internal LRESULT CALLBACK
Win32WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message) {
        case WM_SIZE:
        {

        } break;
        case WM_DESTROY:
            // Handle this as an error - recreate window?
            GlobalRunning = false;
            break;
        case WM_CLOSE:
            // Handle this with a message to the user?
            GlobalRunning = false;
            break;
        case WM_ACTIVATEAPP:
#if 0
			// This sets the window to have transparency when it is not the active window.
			if(WParam) {
				SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
			}
			else {
				SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 128, LWA_ALPHA);
			}
#endif
            break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: 
        {       
            Assert("KeyMessage processed in callback rather than message loop.\n");
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
            Win32DisplayBufferInWindow(DeviceContext, &GlobalBackbuffer, Dimensions.Width, Dimensions.Height);
            EndPaint(Window, &Paint);

        } break;
        default:
            // Returns default behavior for the current Message
            Result = DefWindowProcA(Window, Message, WParam, LParam);
            break;
    }
    return Result;
}
// Clears the secondary sound buffer. I assume it clears the buffer of garbage data.
internal void
Win32ClearSoundBuffer(win32_sound_output *SoundOutput) {
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size, &Region2, 
					&Region2Size, 0))) 
	{
		uint8_t *DestSample = (uint8_t *)Region1;
        // Assert that Region1Size/Region2Size is valid
        // TODO Collapse these loops
        for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex) {
			*DestSample++ = 0;
        }
		DestSample = (uint8_t *)Region2;
        // Assert that Region1Size/Region2Size is valid
        // TODO Collapse these loops
        for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex) {
			*DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

// Fills the secondary sound buffer using the members of the SoundOutput struct, and the ByteToLock and BytestoWrite based from the RunningIndex
internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, 
	engine_sound_output_buffer *SourceBuffer) {

    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;

    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, &Region1, &Region1Size, &Region2, &Region2Size, 0))) { 
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16_t *DestSample = (int16_t *)Region1;
		int16_t *SourceSample = SourceBuffer->Samples;
        // Assert that Region1Size/Region2Size is valid
        // TODO Collapse these loops
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16_t *)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}
// Updates our button state in for the engine.
internal void
Win32ProcessKeyboardMessage(engine_button_state *NewState, bool32 IsDown) {
    // We only process input when the key state has changed
	if(IsDown != NewState->EndedDown) {
		NewState->EndedDown = IsDown;
		++NewState->HalfTransitionCount;
	}
}
// Updates the button state for a controller input that is not analog.
internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState, DWORD ButtonBit, engine_button_state *OldState, 
		engine_button_state *NewState) {
	// Set Button->EndedDown to true if the corresponding button has been pressed.
	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	// If the old and new states are different, then a transition occured.
	// Change based on polling when we increase it
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}
// Processes the left stick analog value using the DeadZoneThreshold.
internal f32 
Win32ProcessXInputLeftStickValue(SHORT StickValue, SHORT DeadZoneThreshold) {
    f32 Result = 0;
    // Shifting the stick value to start at 0, relative to the passed DeadZoneThreshold.
    if(StickValue < -DeadZoneThreshold)
        Result = (f32)(StickValue + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold);
    else if(StickValue > DeadZoneThreshold)
        Result = (f32)(StickValue - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold);
    return Result;
}
// Input recording
internal void
Win32GetInputFileLocation(win32_state *Win32State, bool32 InputStream, int SlotIndex, int DestCount, char *Dest) {
	char Temp[WIN32_STATE_FILE_NAME_COUNT];
	wsprintf(Temp, "input_loop_%d_%s.Ef", SlotIndex, InputStream ? "input" : "state");
	Win32BuildEXEDirectoryPath(Win32State, Temp, DestCount, Dest);
}
internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *Win32State, int unsigned Index) {
	Assert(Index < ArrayCount(Win32State->ReplayBuffers));
	win32_replay_buffer *ReplayBuffer = &Win32State->ReplayBuffers[Index];
	return ReplayBuffer;
}
// Opens a file handle for writing our input. We also read in the engine state from Win32State.
// This block of engine state is written intially at startup. Each time this function is called,
// we overwrite the MemoryMap that contains our engine state.
internal void
Win32BeginRecordingInput(win32_state *Win32State, int InputRecordingIndex) {
	win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputRecordingIndex);
	// Assure that we have a valid memory block before trying to copy memory.
	if(ReplayBuffer->MemoryBlock) {
		Win32State->InputRecordingIndex = InputRecordingIndex;
		char FileName[WIN32_STATE_FILE_NAME_COUNT];
		Win32GetInputFileLocation(Win32State, true, InputRecordingIndex, StringLength(FileName), FileName);
		Win32State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0 ,0);

		CopyMemory(ReplayBuffer->MemoryBlock, Win32State->EngineMemoryBlock, Win32State->TotalMemorySize);
	}
	
}
// Close our file handle for recording, and reset the recording index.
internal void
Win32EndRecordingInput(win32_state *Win32State) {
	CloseHandle(Win32State->RecordingHandle);
	Win32State->InputRecordingIndex = 0;
}
// Opens our file handle that we recorded input to, and swaps the current engine state with the one we stored
// in our MemoryMap.
internal void
Win32BeginInputPlayback(win32_state *Win32State, int InputPlayingIndex) {
	win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputPlayingIndex);
	if(ReplayBuffer->MemoryBlock) {
		Win32State->InputPlayingIndex = InputPlayingIndex;
		char FileName[WIN32_STATE_FILE_NAME_COUNT];
		Win32GetInputFileLocation(Win32State, true, InputPlayingIndex, StringLength(FileName), FileName);
		Win32State->PlaybackHandle= CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0 ,0);

		CopyMemory(Win32State->EngineMemoryBlock ,ReplayBuffer->MemoryBlock, Win32State->TotalMemorySize);
	}
	
}
// Close our playback file handle, and reset our playing index.
internal void
Win32EndInputPlayback(win32_state *Win32State) {
	CloseHandle(Win32State->PlaybackHandle);
	Win32State->InputPlayingIndex = 0;
}
// Write to our input file, all inputs since Win32BeginRecordingInput has been called.
internal void
Win32RecordInput(win32_state *Win32State, engine_input *NewInput) {
	DWORD BytesRead;
	WriteFile(Win32State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
}
// Read the input file until there are no bytes left. Copy the input to the NewInput,
// which is what our engine uses for updating state. When the end of the file has been reached,
// replay the recording. The playback is closed in the Win32ProcessMessageQueue function.
internal void
Win32PlaybackInput(win32_state *Win32State, engine_input *NewInput) {
	DWORD BytesRead;
	if(ReadFile(Win32State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0)) {
		if(BytesRead == 0) {
			// Hit the end of the file
			int PlayingIndex = Win32State->InputPlayingIndex;
			Win32EndInputPlayback(Win32State);
			Win32BeginInputPlayback(Win32State, PlayingIndex);
			ReadFile(Win32State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
		}
		else {
		}
	}
}

// Handle the Window's messages we care about
internal void 
Win32ProcessMessageQueue(win32_state *Win32State, engine_controller_input *KeyboardController) {
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
        // Check for a quit message, and exit
        switch(Message.message) {
            case WM_QUIT:
                GlobalRunning = false;
                break;
            // Process keyboard input in our main loop
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP: {
                //https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes 
                WPARAM wParam = Message.wParam;
                LPARAM LParam = Message.lParam;
                uint32_t VKCode = (uint32_t)wParam;
                //https://learn.microsoft.com/en-us/previous-versions/ms912654(v=msdn.10) 
                bool32 WasDown = ((LParam & (1 << 30)) != 0); // Bit 30 is set when key is down before the message is sent
                bool32 IsDown = ((LParam & (1 << 31)) == 0);  // Bit 31 is set when a key is up
                // if to filter repeating key presses from holding down a key, we only want to process
                // messages when the key state has changed, i.e from up to down to up 
                if(WasDown != IsDown) {
                    if(VKCode == 'W') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
						OutputDebugStringA("W is down\n");
                    }
                    else if(VKCode == 'A') { 
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D') {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == 'Q') {

                    }
                    else if(VKCode == 'E') {

                    }
                    else if(VKCode == VK_UP) {

                    }
                    else if(VKCode == VK_LEFT) {

                    }
                    else if(VKCode == VK_DOWN) {

                    }
                    else if(VKCode == VK_RIGHT) {

                    }
                    else if(VKCode == VK_SPACE) {

                    }
                    else if(VKCode == VK_ESCAPE) {
                        OutputDebugStringA("ESCAPE: ");          
                        if (IsDown)
                            OutputDebugStringA("is down");
                        if (WasDown)
                            OutputDebugStringA("was down");
                        OutputDebugStringA("\n");
                    }
#if ENGINE_INTERNAL
					else if(VKCode == 'L') {
						if(IsDown) {
							if(Win32State->InputPlayingIndex == 0) {
								// If we are not playing an input loop.
								if(Win32State->InputRecordingIndex == 0) {
									Win32BeginRecordingInput(Win32State, 1);
								}
								else {
									Win32EndRecordingInput(Win32State);
									Win32BeginInputPlayback(Win32State, 1);
								}
							}
							// If we are playing an input loop.
							else {
								Win32EndInputPlayback(Win32State);
							}
						}
						
					}
#endif
                }

                bool32 AltKeyWasDown = (((1 << 29) & LParam) != 0);
                if ((VKCode == VK_F4) && AltKeyWasDown) {
                    GlobalRunning = false;
                }
            } break;
            default: {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}
// Returns the current wall clock time. Used for timing calculations.
inline LARGE_INTEGER
Win32GetWallClock(void) {
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}
// Returns the difference in time between to time points.
inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End) {
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart)) / (f32)GlobalPerfCounterFrequency.QuadPart;
    return Result; 
}
#if 0
// Draws a vertical line based on an X position, and a top and bottom Y position.
internal void
Win32DebugDrawVertical(win32_offscreen_buffer *BackBuffer, int X, int Top, int Bottom, uint32_t Color) {
	if(Top <= 0) Top = 0;
	if(Bottom >= BackBuffer->Height) Bottom = BackBuffer->Height;
	if((X >=0) && (X < BackBuffer->Width)) {
		uint8_t BytesPerPixel = (uint8_t)BackBuffer->BytesPerPixel;
		// Set the pointer to the X pos we want to draw to
		uint8_t *PixelDraw = (uint8_t *)BackBuffer->Memory + X*BytesPerPixel + Top*BackBuffer->WidthInBytes;
		for(int Y = Top; Y < Bottom; ++Y) {
			// Write the color data
			*(uint32_t *)PixelDraw = Color;
			// Adjust Y pos
			PixelDraw += BackBuffer->WidthInBytes;
		}
	}
}

// Helper: Calls our debug draw for sound buffer markers.
inline void
Win32DrawSoundBufferMarker(win32_offscreen_buffer *BackBuffer, DWORD Marker, 
						   win32_sound_output *SoundOutput, f32 C, int PadX, int Top, int Bottom, uint32_t Color) {
	f32 Xf32 = C * (f32)Marker;
    // Shift the start of the buffer to the right relative to the padding.
    int X = PadX + (int)Xf32;
	Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);
}

// Draw lines where the frame flips are relative to sound output.
internal void 
Win32DebugSyncDisplay(win32_offscreen_buffer *BackBuffer, int MarkerCount, win32_debug_time_marker *Markers, 
	int CurrentMarkerIndex, win32_sound_output *SoundOutput, f32 TargetSecondsPerFrame) {
    int PadX = 16;
    int PadY = 16;
	int LineHeight = 64;

    // Remove the padding from the buffer width
    // Ratio of buffer pixels / Sound buffer bytes
    f32 C = (f32)(BackBuffer->Width-2*PadX)/ (f32)SoundOutput->SecondaryBufferSize;
    for(int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex) {
		win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
		Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);
		DWORD PlayColor = 0xFFFFFFFF;
		DWORD WriteColor = 0x0000CCFF;    
		DWORD LocationColor = 0xD9AAE3;
		DWORD ByteCountColor = 0xFF00FF00;
		DWORD ExpectedFlipColor = 0xFFFFFF00;
		DWORD PlayWindowColor = 0xFFFF00FF;
		int Top = PadY;
		int Bottom = PadY + LineHeight;

		if(CurrentMarkerIndex == MarkerIndex) {
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			int FirstTop = Top;

			Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->OutputPlayCursor, SoundOutput, C, PadX, Top, 
									   Bottom, PlayColor);
			Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->OutputWriteCursor, SoundOutput, C, PadX, Top, 
									   Bottom, WriteColor);

			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;

			Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->OutputLocation, SoundOutput, C, PadX, Top, 
									   Bottom, LocationColor);
			Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, 
					SoundOutput, C, PadX, Top, Bottom, ByteCountColor);

			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->ExpectedFlipCursor, SoundOutput, C, PadX, FirstTop, 
									   Bottom, ExpectedFlipColor);

		}
		Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->FlipPlayCursor, SoundOutput, C, PadX, Top, Bottom, PlayColor);
		Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->FlipPlayCursor + 480 * SoundOutput->BytesPerSample, 
								   SoundOutput, C, PadX, Top, Bottom, PlayWindowColor);
		Win32DrawSoundBufferMarker(BackBuffer, ThisMarker->FlipWriteCursor, SoundOutput, C, PadX, Top, Bottom, WriteColor);
	}
}
#endif
int WINAPI
wWinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PWSTR CommandLine, int ShowCode) {
// Windows 11 locks away change of timer resolution, so we need to force allow
// changes to the timer resolution.
#ifndef PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION
#define PROCESS_POWER_THROTTLING_IGNORE_TIME_RESOLUTION 0x4
#endif
	PROCESS_POWER_THROTTLING_STATE State = {};
	State.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	State.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
	State.StateMask = 0;
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &State, sizeof(State));

	win32_state Win32State = {};
	thread_context Thread = {};
	Win32GetEXEDirectoryString(&Win32State);
	char EngineDLLFileName[] = "nenjin.dll";
	char EngineDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEDirectoryPath(&Win32State, EngineDLLFileName, sizeof(EngineDLLFullPath), EngineDLLFullPath);
	StringLength(EngineDLLFullPath);

	char TempDLLFileName[] = "nenjin_temp.dll";
	char TempDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32BuildEXEDirectoryPath(&Win32State, TempDLLFileName, sizeof(TempDLLFullPath), TempDLLFullPath);
    // Get the Frequency of QueryPerformanceCounter so it can be used
    LARGE_INTEGER PerfCounterFrequencyResult;
    QueryPerformanceFrequency(&PerfCounterFrequencyResult);
    GlobalPerfCounterFrequency = PerfCounterFrequencyResult;
    // Set the Windows scheduler granularity to 1ms so sleep works better.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
	win32_engine_code EngineCode = {};
	
    // Attempt to load the XInput library functions
    Win32LoadXInput();
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    WNDCLASSA WindowClass = {};
    // Makes sure the entire window is redrawn when it has been resized.
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32WindowProc;
    // Could also call GetModuleHandle(0) for hInstance
    WindowClass.hInstance = Instance;
    //  WindowClass.hIcon;
    WindowClass.lpszClassName = "win32TestWindowClass";
    // Technically RegisterClass can fail, but it is very unlikely
            // TOOD: Find a way to make this frame rate independent?
    if(RegisterClass(&WindowClass)) {
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa 
        HWND Window = CreateWindowEx(/*WS_EX_TOPMOST|WS_EX_LAYERED*/0, WindowClass.lpszClassName, "Test", 
									 WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
									 CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, Instance, 0);
        if(Window) {
			// TODO: Determine monitor refresh
			// TODO: Determine how fast we can render with software rendering, and change accordingly.
			HDC RefreshDC = GetDC(Window);
			// TODO: Get frame independent timing for movement and physics 
			// For now, lock at 60.
			int MonitorRefreshHz = 60;
			int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
			ReleaseDC(Window, RefreshDC);
			// TODO: Fix audio for high refresh rate. Currently, anything above 60 has bugged audio!
#if 0
			if(Win32RefreshRate > 1) {
				MonitorRefreshHz = Win32RefreshRate;
			}
#endif
			f32 EngineUpdateHz = (f32)MonitorRefreshHz;
			f32 TargetSecondsPerFrame = 1.0f / EngineUpdateHz;
            GlobalRunning = true;
            // Sound test
            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16_t)*2; // Two 16bit channels
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
			// TODO: Compute variance to see what the lowest value could be.
			SoundOutput.SafetyBytes = (int)(((f32)SoundOutput.SamplesPerSecond*(f32)SoundOutput.BytesPerSample/
					EngineUpdateHz)/3.0f);
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearSoundBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            // TODO: Handle different memory footprints
            // VirtualAlloc will initialize all memory to 0, unless the MEM_RESET flag is passed to it.
			int16_t *Samples = (int16_t *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, 
													   PAGE_READWRITE);
// If internal build, set the base address to 2 Terabytes for our allocation.
#if ENGINE_INTERNAL
            LPVOID BaseAddress = (LPVOID *) Terabytes((uint64_t)2);
#else
            LPVOID BaseAddress = 0;
#endif

            engine_memory EngineMemory = {};
            EngineMemory.PermanentStorageSize = Megabytes(64); // Macros in C default to 32 bit unless 
            EngineMemory.TransientStorageSize = Gigabytes(1);  // something in the expression requires more bits. 
			EngineMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
			EngineMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
			EngineMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;

            // Allocate memory for both the permanent and transient memory stores.
			// TODO: Handle various memory footprints
			// TODO: Use large MEM_LARGE_PAGES and call adjust token privilages when not on XP?
            Win32State.TotalMemorySize = EngineMemory.PermanentStorageSize + EngineMemory.TransientStorageSize;
			Win32State.EngineMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalMemorySize, 
														MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            EngineMemory.PermanentStorage = Win32State.EngineMemoryBlock;
            // We cast the EngineMemory.PermanentStorage pointer to uint8_t, so when we add the 
			// PermanentStorageSize, we offset in bytes.
            EngineMemory.TransientStorage = ((uint8_t *) EngineMemory.PermanentStorage + 
											EngineMemory.PermanentStorageSize);
			// Allocate memory for our ReplayBuffers
			// TODO(kaelan): This is taking too long. Should be shorter. Find out why Windows is taking so long.
			// TODO(kaelan): Should the engine state only be stored in memory?? Does it need to be written to disk?
			for(int ReplayBufferIndex = 0; ReplayBufferIndex < ArrayCount(Win32State.ReplayBuffers); ++ReplayBufferIndex) {
				win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayBufferIndex];
				Win32GetInputFileLocation(&Win32State, false, ReplayBufferIndex, 
										  StringLength(ReplayBuffer->FileDirectory), ReplayBuffer->FileDirectory);
				ReplayBuffer->FileHandle = CreateFileA(ReplayBuffer->FileDirectory, GENERIC_WRITE|GENERIC_READ, 0, 0, 
													   CREATE_ALWAYS, 0, 0);
				LARGE_INTEGER MaxSize;
				MaxSize.QuadPart = Win32State.TotalMemorySize;
				// CreateFileMapping needs the total size split into a high and low longs (32-bit).
				ReplayBuffer->MemoryMap = CreateFileMappingA(ReplayBuffer->FileHandle, 0, PAGE_READWRITE,
														    MaxSize.HighPart, MaxSize.LowPart, 0);
				ReplayBuffer->MemoryBlock = MapViewOfFile(ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 
														  0, 0, Win32State.TotalMemorySize);
				if(ReplayBuffer->MemoryBlock) {
					// Logging - Success
				}
				else {
					// Logging - Failed
				}
			}
            // Check to see if the memory for the Engine and the SoundBuffer allocated
            if(EngineMemory.PermanentStorage && EngineMemory.TransientStorage && Samples) {
                // Get a timestamp at the start of the loop
                 // Create 2 input structures so previous state of input can be saved.
                engine_input EngineInput[2] = {};
                engine_input *NewInput = &EngineInput[0];
                engine_input *OldInput = &EngineInput[1];

                LARGE_INTEGER LastCounter = Win32GetWallClock();
				LARGE_INTEGER FlipWallClock = Win32GetWallClock();

                int DebugTimeMarkersIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[30] = {};

                bool32 SoundIsValid = false;
				DWORD AudioLatencyBytes = 0;
				f32 AudioLatencySeconds = 0;
				
                uint64_t LastCycleCount = __rdtsc();
				// Load engine code
				EngineCode = Win32LoadEngineCode(EngineDLLFullPath, TempDLLFullPath);
                // Main Loop
                while(GlobalRunning) {
					FILETIME NewDLLWriteTime = Win32GetLastWriteTime(EngineDLLFullPath);
					if(CompareFileTime(&NewDLLWriteTime, &EngineCode.DLLLastWriteTime) == 1) {
						Win32UnloadEngineCode(&EngineCode);
						EngineCode = Win32LoadEngineCode(EngineDLLFullPath, 
														 TempDLLFullPath);
					}
					 // Get input
                    // Keyboard input
                    engine_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    engine_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    // Clear the keyboard input, but preserve the Button.EndedDown value.
                    // This will allow us to hold down keys!
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for (int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex) {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown = 
                        OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
                    Win32ProcessMessageQueue(&Win32State, NewKeyboardController);
					// Mouse input
					// TODO: Use a better API for getting mouse input
					POINT MouseP;
					GetCursorPos(&MouseP);
					ScreenToClient(Window, &MouseP);
					NewInput->MouseX = MouseP.x;
					NewInput->MouseY = MouseP.y;
					NewInput->MouseZ = 0;
					// GetKeyState returns a SHORT where if the highest bit is set the key is down.
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1], GetKeyState(VK_RBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2], GetKeyState(VK_MBUTTON) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
					// 26:29
                    // Controller Input 
                    // TODO: Should we poll this more frequently?
                    DWORD MaxControllerCount = XUSER_MAX_COUNT;
                    // A check to make sure that we support enough controllers in the case
                    // that MS changes the max controller count in the future!
                    if(MaxControllerCount > ArrayCount(NewInput->Controllers)-1) 
                        MaxControllerCount = ArrayCount(NewInput->Controllers)-1;

                    // Gamepad polling loop
                    for(DWORD ControllerIndex = 0; ControllerIndex < MaxControllerCount; ++ControllerIndex) {
                        DWORD OurControllerIndex = ControllerIndex + 1;
                        // Grab controller states
                        engine_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                        engine_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                        XINPUT_STATE ControllerState;
                        // Performance issue: this loop stalls when the current ControllerIndex is not connected
                        if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                            NewController->IsConnected = true;
							NewController->Analog = OldController->Analog;
                            // This controller is plugged in
                            // TODO: See if ControllerState.dwPacketNumber incrememnts to frequently
                            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                            // Input we care about
                            // TODO: This is a rectangular deadzone. Check to see if there is a round deadzone option.
                            NewController->LeftStickAverageX = Win32ProcessXInputLeftStickValue(Pad->sThumbLX, 
                            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            NewController->LeftStickAverageY = Win32ProcessXInputLeftStickValue(Pad->sThumbLY, 
                            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                            // Check to see if the input should be considered analog
                            if((NewController->LeftStickAverageX != 0.0f) || 
                                (NewController->LeftStickAverageY != 0.0f))
                                NewController->Analog = true; 

                            // DPad - Overwrite the stick averages
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP) {
                                NewController->LeftStickAverageY = 1;
                                NewController->Analog = false; 
                            }
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
                                NewController->LeftStickAverageY = -1;
                                NewController->Analog = false; 
                            }
                            if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
                                NewController->LeftStickAverageX = -1;
                                NewController->Analog = false; 
                            }
                            if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
                                NewController->LeftStickAverageX = 1;
                                NewController->Analog = false; 
                            }

                            f32 Threshold = 0.5f;
                            Win32ProcessXInputDigitalButton((NewController->LeftStickAverageX < -Threshold) ? 1 : 0, 
                            1, &OldController->MoveLeft, &NewController->MoveLeft);
                            Win32ProcessXInputDigitalButton((NewController->LeftStickAverageX > Threshold) ? 1 : 0, 
                            1, &OldController->MoveRight, &NewController->MoveRight);
                            Win32ProcessXInputDigitalButton((NewController->LeftStickAverageY < -Threshold) ? 1 : 0, 
                            1, &OldController->MoveDown, &NewController->MoveDown);
                            Win32ProcessXInputDigitalButton((NewController->LeftStickAverageY > Threshold) ? 1 : 0, 
                            1, &OldController->MoveUp, &NewController->MoveUp);

                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_A, 
                                    &OldController->ActionDown, &NewController->ActionDown);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_B, 
                                    &OldController->ActionRight, &NewController->ActionRight);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_X, 
                                    &OldController->ActionLeft, &NewController->ActionLeft);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_Y, 
                                    &OldController->ActionUp, &NewController->ActionUp);

                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, 
                                    &OldController->LeftShoulder, &NewController->LeftShoulder);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, 
                                    &OldController->RightShoulder, &NewController->RightShoulder);

                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_START, 
                                    &OldController->Start, &NewController->Start);
                            Win32ProcessXInputDigitalButton(Pad->wButtons, XINPUT_GAMEPAD_BACK, 
                                    &OldController->Back, &NewController->Back);
                        }
                        else {
                            // The controller is not available
                            NewController->IsConnected = false;
                        }
                    }
                    // Platform independent buffer!
                    engine_offscreen_buffer EngineBuffer = {};
                    // This line means that both the engine_offscreen_buffer EngineBuffer, and 
                    // our win32_offscreen_buffer GlobalBackBuffer share memory. We are
                    // copying the Memory member, which is a void pointer, AKA an address.
                    EngineBuffer.Memory = GlobalBackbuffer.Memory;
                    EngineBuffer.Width = GlobalBackbuffer.Width;
                    EngineBuffer.Height = GlobalBackbuffer.Height;
                    EngineBuffer.WidthInBytes = GlobalBackbuffer.WidthInBytes;
					EngineBuffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;
						
					if(Win32State.InputRecordingIndex) {
						Win32RecordInput(&Win32State, NewInput);
					}
					if(Win32State.InputPlayingIndex) {
						Win32PlaybackInput(&Win32State, NewInput);
					}
					// Because this call updates the GlobalBackbuffer via the memory pointer.
					if(EngineCode.UpdateAndRender) {
						EngineCode.UpdateAndRender(&Thread, &EngineMemory, EngineInput, &EngineBuffer);
					}
					/*
					   How sound output works: The idea is that we try to write one frame ahead of the current frame.
					   We can predict where a frame flip will occur in our engine by subtracting the time we enter the 
					   audio code, by the total TargetSecondsPerFrame. We can also calculate the 
					   ExpectedSoundBytesPerFrame using the number of samples per second, divided by the 
					   current EngineUpdateHz. The calculate the SecondsLeftUntilFlip by getting subtracting
					   the time when the audio code is entered by the TargetSecondsPerFrame. Then divide
					   the SecondsLeftUntilFlip by the TargetSecondsPerFrame, then multiply that quantity by
					   the ExpectedSoundBytesPerFrame to get the number of bytes left to the flip. We can add
					   this to the current PlayCursor position to get the ExpectedFrameBoundaryByte.

					   The idea is to try and begin writing as soon as the frame flips so the latency is minimized, and
					   as a fall back write so the next frame plays sound on time.

					*/
					LARGE_INTEGER AudioWallClock= Win32GetWallClock();
					f32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);
					DWORD PlayCursor;
					DWORD WriteCursor;
					if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK) {
						if(!SoundIsValid) {
							SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
							SoundIsValid = true;
						}
						// Compute how much sound to write and where in the buffer.
						// % SoundOutput.SecondaryBufferSize assures that the value of 
						// ByteToLock is always within the buffer size.
						// RunningSampleIndex increases beyond the size of the buffer, until it wraps around 
						// from overflow, so the using the 
						// % operation will make sure that the computed value is "mapped" properly into the buffer.
						// Remember, the buffer is only one second long, and it is circular.
						DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
											SoundOutput.SecondaryBufferSize);
						DWORD ExpectedSoundBytesPerFrame = (int)((f32)(SoundOutput.SamplesPerSecond*
									SoundOutput.BytesPerSample)/EngineUpdateHz);
						f32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
						DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame)*
													   (f32)ExpectedSoundBytesPerFrame);
						DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
						DWORD SafeWriteCursor = WriteCursor;
						if(SafeWriteCursor < PlayCursor) {
							// Unwrap the WriteCursor for the computation
							SafeWriteCursor += SoundOutput.SecondaryBufferSize;
						}
						// 2:08:13
						Assert(SafeWriteCursor >= PlayCursor);
						SafeWriteCursor += SoundOutput.SafetyBytes;
						bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);

						DWORD TargetCursor = 0;
						if(AudioCardIsLowLatency) {
							TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
						}
						else {
							TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
						}
						TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;
						
						DWORD BytesToWrite = 0;
						if(ByteToLock > TargetCursor) {
							BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
							BytesToWrite += TargetCursor;
						}
						else {
							BytesToWrite = TargetCursor- ByteToLock;
						}
						engine_sound_output_buffer EngineSoundBuffer = {};
						EngineSoundBuffer.SamplesPerSecond =  SoundOutput.SamplesPerSecond;
						EngineSoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
						EngineSoundBuffer.Samples = Samples;
						// Guard to null function pointer
						if(EngineCode.GetSoundSamples) {
							EngineCode.GetSoundSamples(&Thread, &EngineMemory, &EngineSoundBuffer);
						}
#if ENGINE_INTERNAL
						win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkersIndex];
						Marker->OutputPlayCursor = PlayCursor;
						Marker->OutputWriteCursor = WriteCursor;
						Marker->OutputLocation = ByteToLock;
						Marker->OutputByteCount = BytesToWrite;
						Marker->ExpectedFlipCursor = ExpectedFrameBoundaryByte;


						// The SoundBuffer is circular, meaning in some cases, the WriteCursor
						// will appear to be behind the PlayCursor. We can fix this for our calculation
						// using the logic below.
						DWORD UnwrappedWriteCursor = WriteCursor;
						if(PlayCursor > WriteCursor)
							UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
						AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
						AudioLatencySeconds = (((f32)AudioLatencyBytes / (f32)SoundOutput.BytesPerSample) / 
							(f32)SoundOutput.SamplesPerSecond);
#if 0
						char TextBuffer[256];
						_snprintf_s(TextBuffer, sizeof(TextBuffer), 
								"BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%f)s\n", 
								ByteToLock, TargetCursor, BytesToWrite, PlayCursor, WriteCursor, 
								AudioLatencyBytes, AudioLatencySeconds);
						OutputDebugStringA(TextBuffer);
#endif
#endif
						Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &EngineSoundBuffer);
					}
					else {
						SoundIsValid = false;
					}

                    LARGE_INTEGER WorkCounter = Win32GetWallClock();
                    f32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                    // TODO: Need to test!!
                    f32 SecondsElapsedForFrame = WorkSecondsElapsed;
                    if(SecondsElapsedForFrame < TargetSecondsPerFrame) {
                        // Only sleep if we can set granularity
                        if(SleepIsGranular) {
							// Calculate the sleep time needed to match the target frame time, but sleep 1 less MS.
							// Not sure if this a Windows 11 issue, but if we try to sleep the exact amount of time, 
							// there is a chance that we sleep too long.

							// First do calculation in INT to check for negative number.
                            INT INTSleepMs = (INT)(1000.0f*(TargetSecondsPerFrame - SecondsElapsedForFrame) -1.0f);
							// Cast to DWORD if not negative, otherwise set to 0.
							DWORD SleepMs = (INTSleepMs < 0 ? 0 : (DWORD)INTSleepMs);
                            if(SleepMs > 0)
                                Sleep(SleepMs);
                        }
						f32 TestSecondsPerFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
						// TODO: Log frame misses!
						//Assert(TestSecondsPerFrame < TargetSecondsPerFrame);
                        while (SecondsElapsedForFrame < TargetSecondsPerFrame) {
                            SecondsElapsedForFrame = (Win32GetSecondsElapsed(LastCounter, Win32GetWallClock()));
                        }
                    }
                    else {
                        // Missed frame!
                    }
                    LARGE_INTEGER EndCounter = Win32GetWallClock();

#if 0
                    f32 MSPerFrame = 1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter);
                    f32 FPS =  1000.0f / MSPerFrame;                   
#endif

                    LastCounter = EndCounter;
					// Overwrite LastCounter so we can see the difference in time relative 
					// to the last time the loop completed a cycle
                    win32_window_dimensions Dimensions = Win32GetWindowDimensions(Window);
					HDC DeviceContext = GetDC(Window);
                    Win32DisplayBufferInWindow(DeviceContext, &GlobalBackbuffer, Dimensions.Width, Dimensions.Height);
					ReleaseDC(Window, DeviceContext);
					FlipWallClock = Win32GetWallClock();
					                    // Debug code!!
#if ENGINE_INTERNAL
                    {
						DWORD PlayCursor;
						DWORD WriteCursor;
						if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK) {
							Assert(DebugTimeMarkersIndex < ArrayCount(DebugTimeMarkers));
							win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkersIndex];

							Marker->FlipPlayCursor = PlayCursor;
							Marker->FlipWriteCursor = WriteCursor;

						}
                    }
#endif
                    // Swap inputs
                    engine_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                    uint64_t EndCycleCount = __rdtsc();
                    uint64_t CyclesElapsed = EndCycleCount - LastCycleCount;
                    LastCycleCount = EndCycleCount;
#if 0
                    f32 MegaCyclesPerFrame = (f32)CyclesElapsed /(1000.0f*1000.0f); 

                    // Debug output 
                    char Buffer[256];
                    _snprintf_s(Buffer, sizeof(Buffer), " %.02f FPS, %.02fms, %f MegaCycles\n", FPS, MSPerFrame, MegaCyclesPerFrame);
                    OutputDebugStringA(Buffer);
#endif

#if ENGINE_INTERNAL
					++DebugTimeMarkersIndex;
					// If we exceed the size of the PlayCursor array, then wrap to the beginning.
					// Treat as if it was a circular buffer!
					if(DebugTimeMarkersIndex == ArrayCount(DebugTimeMarkers)){
						DebugTimeMarkersIndex = 0;
					}
#endif
                }
            }
			
        }
        else {
            //Logging - WindowHandle could not be made
        }
    }
    else {
        // Logging - RegisterClass failed
    }
    return 0;
}

