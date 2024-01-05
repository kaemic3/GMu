#include <Windows.h>
#include <malloc.h>

#include "nenjin_platform.h"
#include "win32_main.h"

/* TODO(kaelan): 
        - Determine if hot code reload is a thing that I want to support. 
        - Should the user be able to resize the window?
            - Enforce specific render resolutions? Like in a game?
        - Add UI basics, like file loading.
*/

// NOTE: Hot code reload will not work with the CPU, could make the CPU interpreter apart of win32_main.cpp translation unit.
//       This would then allow everything else to not need it! Can test ROMS in real time this way!
global_variable win32_offscreen_buffer global_back_buffer;
global_variable bool32 global_running;
global_variable WINDOWPLACEMENT global_prev_window_placement;
global_variable bool32 global_is_fullscreen;
global_variable HCURSOR global_cursor;

// Basic, kinda slow, string contatenation. Will need to create a proper string library at some point!
internal void
CatStrings(size_t a_count, char *a, size_t b_count, 
		   char *b, size_t dest_count, char *dest) {
	// TODO(kaelan): Dest bounds checking.
	for(size_t index = 0; index < a_count; ++index) 
	{
		*dest++ = *a++;
	}
	for(size_t index = 0; index < b_count; ++index) 
	{
		*dest++ = *b++;
	}
	*dest++ = 0;
}
internal int
StringLength(char *string) {
	int length = 0;
	while(*string++) 
	{
		++length;
	}
	return length;
}
// Used to direct file i/o. Lets us create path strings to specificy file locations to be written/read from/to.
internal void
Win32GetEXEDirectoryString(win32_state *state) {
	DWORD size_of_file_name = GetModuleFileNameA(0, state->exe_directory, sizeof(state->exe_directory));
	state->one_past_last_exe_dir_slash = state->exe_directory;
	for(char *scan = state->exe_directory; *scan; ++scan) 
	{
		if(*scan == '\\') 
		{
			state->one_past_last_exe_dir_slash= scan + 1;
		}	
	}
}
internal void
Win32BuildEXEDirectoryPath(win32_state *state, char *file_name, int dest_count, char *dest) {
	CatStrings(state->one_past_last_exe_dir_slash - state->exe_directory, state->exe_directory, 
			   StringLength(file_name), 
			   file_name, dest_count, dest);
}

// DEBUG I/O
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory) {
    VirtualFree(memory, 0, MEM_RELEASE);
}
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile) {
    debug_read_file_result result = {};
    // Creates a file handle that will read, allow other programs to read, and will only open the file
    // if it already exists.
    HANDLE file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, 
									OPEN_EXISTING, 0, 0);
    if(file_handle != INVALID_HANDLE_VALUE) 
	{
        LARGE_INTEGER file_size;
        if(GetFileSizeEx(file_handle, &file_size)) 
		{
            // TODO(kaelan): Defines for max values Us32Max
            u32 file_size_32 = SafeTruncateU64(file_size.QuadPart);
            // Keep in mind that small allocs should not use VirtualAlloc, only used here for testing.
            result.contents = VirtualAlloc(0, file_size_32, MEM_RESERVE|MEM_COMMIT, 
										   PAGE_READWRITE);
            if(result.contents) 
			{
                DWORD bytes_read;
                // Keep in mind, that ReadFile can only read in up to 32-bit sized files, so in order
                // to support large file sizes, we would need to loop.
                if(ReadFile(file_handle, result.contents, file_size_32, &bytes_read, 0) && 
				   file_size_32 == bytes_read) 
				{
                    result.content_size= file_size_32;
                }
            }
            else 
			{
                //Could not read the file
                // Free the memory
                DEBUGPlatformFreeFileMemory(thread, result.contents);
                result.contents = 0;
            }
        }
        else 
		{
            // Could not get file size
        }
        CloseHandle(file_handle);
    }
    else 
	{
        // Could not get Handle
    }
    return result;
}
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile) {
    bool32 result = false;
    HANDLE file_handle = CreateFileA(file_name, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(file_handle != INVALID_HANDLE_VALUE) 
	{
        DWORD bytes_written;
        if(WriteFile(file_handle, memory, memory_size, &bytes_written, 0)) 
		{
            result = (memory_size == bytes_written);
        }
        else 
		{
            // File not written to disk
        }
        CloseHandle(file_handle);
    }
    else 
	{
        // Could not get handle
    }
    return result;
}

// Hot code reload for engine.
// Use FindFirstFile API call to get the last write time of the passed file.
inline FILETIME
Win32GetLastWriteTime(char *file_name) {
	FILETIME result = {};
	WIN32_FILE_ATTRIBUTE_DATA file_data;
	if(GetFileAttributesExA(file_name, GetFileExInfoStandard, &file_data)) 
	{
		result = file_data.ftLastWriteTime;
	}
	return result;
}
// Load our DLL for the engine code.
internal win32_engine_code
Win32LoadEngineCode(char *source_dll_name, char *temp_dll_name, char *lock_file_name) {
	win32_engine_code result = {};
	WIN32_FILE_ATTRIBUTE_DATA ignored;
	// Wait for the lock file to be deleted. This is so VS can load our PDB for debugging!
	if(!GetFileAttributesExA(lock_file_name, GetFileExInfoStandard, &ignored)) 
	{
		result.dll_last_write_time = Win32GetLastWriteTime(source_dll_name);
		CopyFileA(source_dll_name, temp_dll_name, FALSE);
		result.engine_dll = LoadLibraryA(temp_dll_name);
		if(result.engine_dll) 
		{
			result.UpdateAndRender = (nenjin_update_and_render *)
				GetProcAddress(result.engine_dll, "NenjinUpdateAndRender");
			result.is_valid = (result.UpdateAndRender ? true : false);
		}
		else 
		{
	    }
	}
		if(!result.is_valid) 
	{
		result.UpdateAndRender = 0;
	}
	return result;
}
// Unload our DLL for the engine code.
internal void
Win32UnloadEngineCode(win32_engine_code *engine_code) {
	if(engine_code->engine_dll) 
	{
		FreeLibrary(engine_code->engine_dll);
		engine_code->engine_dll= 0;
	}
	engine_code->is_valid = false;	
	engine_code->UpdateAndRender = 0;
}

// NOTE: This function comes from Handmade hero episode 40, where
//		 Casey references Raymon Chen's fullscreen function from his blog.
// https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
internal void
ToggleFullScreen(HWND window) {
	DWORD window_style = GetWindowLong(window, GWL_STYLE);
	if(window_style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO monitor_info = {sizeof(monitor_info)};
		if (GetWindowPlacement(window, &global_prev_window_placement) &&
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
		{
			SetWindowLong(window, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW);
			RECT rc_monitor = monitor_info.rcMonitor;
			SetWindowPos(window, HWND_TOP, rc_monitor.left, rc_monitor.top, 
						 rc_monitor.right - rc_monitor.left,
						 rc_monitor.bottom - rc_monitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
		{
			SetWindowLong(window, GWL_STYLE, window_style | WS_OVERLAPPEDWINDOW);
			SetWindowPlacement(window, &global_prev_window_placement);
			SetWindowPos(window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
}

// Return the current window dimensions.
internal win32_window_dimensions
Win32GetWindowDimensions(HWND window) {
    RECT clinet_rect;
    GetClientRect(window, &clinet_rect);
    win32_window_dimensions window_dimensions;
    window_dimensions.width = clinet_rect.right - clinet_rect.left;
    window_dimensions.height = clinet_rect.bottom - clinet_rect.top;
    return window_dimensions;
}
// DIB - Device Independent BitMap
// Reallocates the buffer when the window is resized. 
// NOTE: This function does not draw anything!
internal void 
Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) {
    // First free the exisiting back buffer memory.
    if(buffer->memory) 
	{
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;
    buffer->bytes_per_pixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;  // This is negative so the pixels we input into the buffer memory go from top left to bottom right.
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;
    // Allocate memory for bitmap
    // 32 bits per pixel only for padding. RGB needs 24 (8 bits R, 8 bits G 8 bits B), but it is slower to access memory that is aligned to 24 bits
    // rather than 32 bits.
    int bitmap_memory_size = (buffer->width*buffer->height)*buffer->bytes_per_pixel;
    // Allocate memory for the Bitmap, read and write access
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    buffer->width_in_bytes = buffer->width*buffer->bytes_per_pixel; 
    // TODO(kaelan): Should this be a #define?
	// Set pixels to be 32-bit/4 bytes
	buffer->bytes_per_pixel = 4;
}
// Uses the StretchDIBits function to display the current back buffer to the window.
// TODO(kaelan): Accomodate different display resolutions.
internal void 
Win32DisplayBufferInWindow(HDC device_context, win32_offscreen_buffer *buffer, int window_width, int window_height) {
	if(global_is_fullscreen)
	{
		// Stretch to fill the screen when in fullscreen mode.
		// TODO(kaelan): Fix this so it scales properly when using windows scaling feature.
		StretchDIBits(device_context, 0, 0, buffer->width*2, buffer->height*2, 0, 0, buffer->width, buffer->height, buffer->memory, 
					  &buffer->info, DIB_RGB_COLORS, SRCCOPY);
	}
	else
	{
		// Clear the outer edges of the window that are not apart of the area we are drawing to.
		PatBlt(device_context, 0, buffer->height, window_width, window_height, BLACKNESS);
		PatBlt(device_context, buffer->width, 0, window_width, window_height, BLACKNESS);
		StretchDIBits(device_context, 0, 0, buffer->width, buffer->height, 0, 0, buffer->width, buffer->height, buffer->memory, 
				&buffer->info, DIB_RGB_COLORS, SRCCOPY);
	}
}


// Handle all callbacks from Windows.
internal LRESULT CALLBACK
Win32WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch(message)
    {
        case WM_QUIT:
        {
            global_running = false;
        } break;
        case WM_DESTROY:
        {
            global_running = false;
        } break;
        case WM_CLOSE:
        {
            global_running = false;
        } break;
        case WM_SETCURSOR:
        {
            global_cursor = SetCursor(global_cursor);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            win32_window_dimensions dim = Win32GetWindowDimensions(window);
            Win32DisplayBufferInWindow(device_context, &global_back_buffer, dim.width, dim.height);
            EndPaint(window, &paint);
        } break;
        // For any case we do not handle, let Windows take care of it with default behavior.
        default:
        {
            result = DefWindowProcA(window, message, w_param, l_param);
        } break;
    }
    return result;
}
// Handle all window messages that are passed to the emulator.
internal void
Win32ProcessMessageQueue(win32_state *state) {
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        #if 0
        switch(message.message)
        {

        }
        #endif

        // IMPORTANT
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

int WINAPI
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int show_code) {
// TODO(kaelan): Need to get the monitor resolution from Windows.
#define SCREEN_HORIZONTAL 1280 
#define SCREEN_VERTICAL 720 
    // Initial window class
    WNDCLASSA window_class = {};
    window_class.style = CS_HREDRAW | CS_VREDRAW; 
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "Win32MainWindowClass";
    window_class.hCursor = LoadCursor(0, IDC_ARROW);
    // Create a back buffer
    Win32ResizeDIBSection(&global_back_buffer, SCREEN_HORIZONTAL, SCREEN_VERTICAL);
    global_running = true;
    global_cursor = LoadCursor(0, IDC_ARROW);

    // Make sure the window class can register.
    if(RegisterClassA(&window_class))
    {
        // NOTE: Currently rendering at 720p.
        // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexa 
        HWND window = CreateWindowExA(/*WS_EX_TOPMOST|WS_EX_LAYERED*/0, window_class.lpszClassName, "GMu win32", 
									 WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
									 SCREEN_HORIZONTAL, SCREEN_VERTICAL, 0, 0, instance, 0);
        if(window) 
        {
            nenjin_memory engine_memory = {};
            engine_memory.permanent_storage_size = Megabytes(16);
            engine_memory.transient_storage_size = Megabytes(48);
            engine_memory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            engine_memory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            engine_memory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
            win32_state platform_state = {};
            win32_engine_code engine_code = {};
            thread_context thread = {};
            LPVOID base_address = (LPVOID *)Terabytes((u64)1);
            // NOTE: Trying 64 MiB for total memory allocated.
            platform_state.total_memory_size = engine_memory.permanent_storage_size + engine_memory.transient_storage_size;
            platform_state.engine_memory_block = VirtualAlloc(base_address, (size_t)platform_state.total_memory_size, MEM_RESERVE|MEM_COMMIT,
                                                              PAGE_READWRITE);
            engine_memory.permanent_storage = platform_state.engine_memory_block;
            // Move the pointer to the transient storage, according to the size of the permananent storage block.
            // NOTE: Cast the permanent_storage pointer to u8 so we can move in terms of bytes.
            engine_memory.transient_storage = ((u8 *)engine_memory.permanent_storage + engine_memory.permanent_storage_size);
            if(engine_memory.permanent_storage && engine_memory.transient_storage)
            {
                // Intialize strings for engine DLL's.
                Win32GetEXEDirectoryString(&platform_state);
                char dll_name[] = "nenjin.dll";
                char engine_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
                Win32BuildEXEDirectoryPath(&platform_state, dll_name, sizeof(engine_dll_full_path), engine_dll_full_path);
                char temp_dll_name[] = "nenjin_temp.dll";
                char temp_engine_dll_full_path[WIN32_STATE_FILE_NAME_COUNT];
                Win32BuildEXEDirectoryPath(&platform_state, temp_dll_name, sizeof(temp_engine_dll_full_path), temp_engine_dll_full_path);
                char lock_file_name[] = "lock.tmp";
                char lock_full_path[WIN32_STATE_FILE_NAME_COUNT];
                Win32BuildEXEDirectoryPath(&platform_state, lock_file_name, sizeof(lock_full_path), lock_full_path);
                engine_code = Win32LoadEngineCode(engine_dll_full_path, temp_engine_dll_full_path, lock_full_path);
                while(global_running)
                {
                    // Reload the engine dll if it has been re-built.
                    FILETIME new_dll_write_time = Win32GetLastWriteTime(engine_dll_full_path);
                    if(CompareFileTime(&new_dll_write_time, &engine_code.dll_last_write_time) == 1)
                    {
                        Win32UnloadEngineCode(&engine_code);
                        engine_code = Win32LoadEngineCode(engine_dll_full_path, temp_dll_name, lock_full_path);
                    }
                    nenjin_input *engine_input = 0;

                    nenjin_offscreen_buffer engine_buffer = {};
                    engine_buffer.memory = global_back_buffer.memory;
                    engine_buffer.width = global_back_buffer.width;
                    engine_buffer.height = global_back_buffer.height;
                    engine_buffer.width_in_bytes = global_back_buffer.width_in_bytes;
                    engine_buffer.bytes_per_pixel = global_back_buffer.bytes_per_pixel;
                    // TODO(kaelan): Setup a fixed timestep for the emulator to run at!
                    if(engine_code.UpdateAndRender)
                    {
                        engine_code.UpdateAndRender(&thread, &engine_memory, engine_input, &engine_buffer);
                    }
                    Win32ProcessMessageQueue(&platform_state);
                    HDC device_context = GetDC(window);
                    win32_window_dimensions win_dim = Win32GetWindowDimensions(window);
                    Win32DisplayBufferInWindow(device_context, &global_back_buffer, win_dim.width, win_dim.height);
                    ReleaseDC(window, device_context);
                }
            }
            else
            {
                // Logging, engine memory could not be allocated.
            }
        }
        else
        {
            // Logging, window failed to be created.
        }
        
    }
    else
    {
        // Logging, window class failed to register with Windows.
    }
    return 0;
}