#include <Windows.h>
#include <malloc.h>
#include <stdio.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <stdlib.h>

#include "nenjin.cpp"
#include "nenjin_platform.h"
#include "win32_main.h"

/* TODO(kaelan): 
        - Determine if hot code reload is a thing that I want to support. 
        - Should the user be able to resize the window?
            - Enforce specific render resolutions? Like in a game?
        - Add UI basics, like file loading.
*/

// NOTE: Hot code reload will not work with the CPU, could make the CPU interpreter apart of win32_main.cpp translation unit.
//       Also need to remove all allocations from the emulator, which means removing all STL stuff like std::vector.
global_variable win32_offscreen_buffer global_back_buffer;
global_variable bool32 global_running;
global_variable WINDOWPLACEMENT global_prev_window_placement;
global_variable bool32 global_is_fullscreen;
global_variable HCURSOR global_cursor;
global_variable LARGE_INTEGER global_perf_counter_frequency;

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
                DEBUGPlatformFreeFileMemory(result.contents);
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
DEBUG_PLATFORM_FIND_ROM_FILE(DebugPlatfromFindROMFile) {
    #if 0
    HWND hwnd = GetActiveWindow();
    OPENFILENAMEA open_file;
    ZeroMemory(&open_file, sizeof(open_file));
    open_file.lStructSize = sizeof(open_file);
    open_file.hwndOwner = hwnd;
    open_file.lpstrFile = *file_name;
    // IMPORTANT Assign null terminator 
    open_file.lpstrFile[0] = '\0';
    // FIXME: Magic number
    open_file.nMaxFile = 260;
    open_file.lpstrFilter = "All\0*.*\0ROM (.gb)\0*.gb\0";
    open_file.nFilterIndex = 1;
    open_file.lpstrFileTitle = NULL;
    open_file.nMaxFileTitle = 0;
    open_file.lpstrInitialDir = NULL;
    open_file.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    GetOpenFileNameA(&open_file);
    // TODO(kaelan): This crashes the program if the user does not select a file.
    #else
    IFileDialog *file_dialog = 0;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&file_dialog));
    if(SUCCEEDED(hr))
    {
        // Get the flags.
        DWORD flags;
        hr = file_dialog->GetOptions(&flags);
        if(SUCCEEDED(hr))
        {
            // Shell items only??
            hr = file_dialog->SetOptions(flags | FOS_FORCEFILESYSTEM);
            if(SUCCEEDED(hr))
            {
                hr = file_dialog->SetDefaultExtension(L".gb");
                if(SUCCEEDED(hr))
                {
                    hr = file_dialog->Show(0);
                    if(SUCCEEDED(hr))
                    {
                        IShellItem *result;
                        hr = file_dialog->GetResult(&result);
                        if(SUCCEEDED(hr))
                        {
                            PWSTR file_path = 0;
                            hr = result->GetDisplayName(SIGDN_FILESYSPATH, &file_path);
                            if(SUCCEEDED(hr))
                            {
                                wcstombs(*file_name, file_path, 260);
                                result->Release();
                                CoTaskMemFree(file_path);
                            }
                        }
                    }
                }
            }
        }
    }
    file_dialog->Close(hr);
    file_dialog->ClearClientData();
    file_dialog->Release();
    #endif
    Assert(*file_name);
}
// Returns all of the ROM file names from the data/ROMs directory.
DEBUG_PLATFORM_GET_ROM_DIRECTORY(DEBUGPlatformGetROMDirectory) {
    WIN32_FIND_DATAA data;
    // Skip the . and .. strings
    HANDLE find = FindFirstFileA("../data/ROMs/*", &data);
    FindNextFileA(find, &data);
    s32 string_index = 0;
    while(FindNextFileA(find, &data) != 0)
    {
        char *string = data.cFileName;
        s32 string_length = StringLength(string);
        for(s32 index = 0; index < string_length; ++index)
        {
            string_array->strings[string_index].value[index] = string[index];
        }
        // Add null term.
        string_array->strings[string_index].value[string_length] = 0;
        ++string_index;
    }
    string_array->size = string_index;
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

#if 0
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
// TODO(kaelan): Determine if this code should just be removed?
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
#endif

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
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert("KeyMessage processed in callback rather than message loop.\n");
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
internal void
Win32ProcessKeyboardMessage(nenjin_button_state *new_state, bool32 is_down) {
    // Only process the input when the state has changed.
    if(is_down != new_state->ended_down)
    {
        new_state->ended_down = is_down;
        ++new_state->half_transition_count;
    }
}
// Handle all window messages that are passed to the emulator.
internal void
Win32ProcessMessageQueue(win32_state *state, nenjin_controller_input *keyboard_controller) {
    MSG message;
    while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        switch(message.message)
        {
            case WM_QUIT:
            {
                global_running = false;
            } break;
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                WPARAM w_param = message.wParam;
                LPARAM l_param = message.lParam;
                u32 vk_code = (u32)w_param;
                bool32 was_down = ((l_param & (1 << 30)) != 0);
                bool32 is_down = ((l_param & (1 << 31)) == 0);
                bool32 repeated = ((l_param & 0xffff) > 0);
                if(was_down != is_down)
                { // Check vk codes.
                    if(vk_code == 'W')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->up,is_down);
                    }
                    else if(vk_code == 'S')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->down, is_down);
                    }
                    else if(vk_code == 'A')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->left, is_down);
                    }
                    else if(vk_code == 'D')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->right, is_down);
                    }
                    else if(vk_code == 'G')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->a, is_down);
                    }
                    else if(vk_code == 'F')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->b, is_down);
                    }
                    else if(vk_code == 'J')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->start, is_down);
                    }
                    else if(vk_code == 'H')
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->select, is_down);
                    }
                    else if(vk_code == 'P')
                    {
                        if(is_down)
                        {
                            keyboard_controller->pause_emulator = !keyboard_controller->pause_emulator;
                        }
                    }
                    else if(vk_code == 'R')
                    {
                        if(is_down)
                        {
                            Win32ProcessKeyboardMessage(&keyboard_controller->reset, is_down);
                        }
                    }
                    else if(vk_code == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->rom_up, is_down);
                    }
                    else if(vk_code == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->rom_down, is_down);
                    }
                    else if(vk_code == VK_RETURN)
                    {
                        Win32ProcessKeyboardMessage(&keyboard_controller->rom_select, is_down);
                    }
                   	if(is_down)
					{
						bool32 alt_key_was_down = (((1 << 29) & l_param) != 0);
						if((vk_code== VK_F4) && alt_key_was_down) 
						{
							global_running = false;
						}
						// Toggle fullscreen mode!
						if((vk_code == VK_RETURN) && alt_key_was_down)
						{
							if(message.hwnd)
							{
								ToggleFullScreen(message.hwnd);
								global_is_fullscreen = !global_is_fullscreen;
							}
						}
                        // NOTE: This will open a file dialog when alt + F are pressed.
                        // TODO(kaelan): Need to make this a function that can be called in the engine.
                        // TODO(kaelan): Create a button in the nenjin_controller_input struct for this?
                        // IDEA: This should call DEBUGPlatformReadEntireFile once the file name has been grabbed.
                        //       Also, this function should be call DEBUGPlatformFindROMFile.
                        if((vk_code == 'F' && alt_key_was_down))
                        {
                            Win32ProcessKeyboardMessage(&keyboard_controller->load_rom, is_down);
                        }
                        if((vk_code == 'G' && alt_key_was_down))
                        {
                            Win32ProcessKeyboardMessage(&keyboard_controller->load_rom_abs, is_down);
                        }
					} 

                }
            } break;
            default:
            {
                // IMPORTANT
                TranslateMessage(&message);
                DispatchMessage(&message);
            } break;
        }

    }
}

// Returns the current wall clock time. Used for timing calculations.
inline LARGE_INTEGER
Win32GetWallClock(void) {
    LARGE_INTEGER result;
    QueryPerformanceCounter(&result);
    return result;
}
// Returns the difference in time between to time points.
inline f32
Win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    f32 result = ((f32)(end.QuadPart - start.QuadPart)) / (f32)global_perf_counter_frequency.QuadPart;
    return result; 
}

int WINAPI
wWinMain(HINSTANCE instance, HINSTANCE prev_instance, PWSTR command_line, int show_code) {
// Windows 11 sleep nonsense!

// Windows 11 locks away change of timer resolution, so we need to force allow
// changes to the timer resolution.
#ifndef PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION
#define PROCESS_POWER_THROTTLING_IGNORE_TIME_RESOLUTION 0x4
#endif
	PROCESS_POWER_THROTTLING_STATE power_state = {};
	power_state.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
	power_state.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
	power_state.StateMask = 0;
	SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &power_state, sizeof(power_state));

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
            // Grab the function pointer to NenjinUpdateAndRender
            nenjin_update_and_render *UpdateAndRender = NenjinUpdateAndRender;
            nenjin_memory engine_memory = {};
            engine_memory.permanent_storage_size = Megabytes(16);
            engine_memory.transient_storage_size = Megabytes(48);
            engine_memory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            engine_memory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            engine_memory.DEBUGPlatformFindROMFile = DebugPlatfromFindROMFile;
            engine_memory.DEBUGPlatformGetROMDirectory = DEBUGPlatformGetROMDirectory;
            engine_memory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
            win32_state platform_state = {};
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
                // Input vars
                nenjin_input engine_input[2] = {};
                nenjin_input *new_input = &engine_input[0];
                nenjin_input *old_input = &engine_input[1];

                // Setup engine timing/sleep settings.
                LARGE_INTEGER perf_counter_frequency_result;
                QueryPerformanceFrequency(&perf_counter_frequency_result);
                // This can be global since the frequency is set on boot. The program only needs to get this value one time.
                global_perf_counter_frequency = perf_counter_frequency_result;
                // Set scheduler granularity to 1 ms
                UINT desired_scheduler_ms = 1;
                // Check if our change to the scheduler went through.
                bool32 sleep_is_granular = (timeBeginPeriod(desired_scheduler_ms) == TIMERR_NOERROR);
                // Since the Game Boy updates it's screen at a fixed rate, the frame rate will be locked for now.
                // TODO(kaelan): const or DEFINE???
                // TODO(kaelan): Does the emulator need access to the update frequency??
                const f32 engine_update_freq_ms = 16.74f;
                // Convert our update hz in ms to seconds.
                f32 target_seconds_per_frame = engine_update_freq_ms/1000.0f;
                LARGE_INTEGER last_counter = Win32GetWallClock();
                while(global_running)
                {
                    new_input->d_time_for_frame = target_seconds_per_frame;
                    // Process keyboard input
                    // NOTE: Controller 0 is the keyboard. Gamepad support will be added, eventually.
                    nenjin_controller_input *new_keyboard_controller = GetController(new_input, 0);
                    nenjin_controller_input *old_keyboard_controller = GetController(old_input, 0);
                    // Clear new input buffer
                    *new_keyboard_controller = {};
                    new_keyboard_controller->is_connected = true;
                    new_keyboard_controller->pause_emulator = old_keyboard_controller->pause_emulator;
                    // Get the down state of the previous input.
                    for(int button_index = 0; button_index < ArrayCount(new_keyboard_controller->buttons); ++button_index)
                    {
                        new_keyboard_controller->buttons[button_index].ended_down = 
                        old_keyboard_controller->buttons[button_index].ended_down;
                    }
                    Win32ProcessMessageQueue(&platform_state, new_keyboard_controller);
                    // Process mouse input.
                    POINT mouse_p;
                    GetCursorPos(&mouse_p);
                    ScreenToClient(window, &mouse_p);
                    new_input->mouse_x = mouse_p.x;
                    new_input->mouse_y = mouse_p.y;
                    new_input->mouse_z  = 0;
                    Win32ProcessKeyboardMessage(&new_input->mouse_buttons[0], GetKeyState(VK_LBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&new_input->mouse_buttons[1], GetKeyState(VK_RBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&new_input->mouse_buttons[2], GetKeyState(VK_MBUTTON) & (1 << 15));
                    Win32ProcessKeyboardMessage(&new_input->mouse_buttons[3], GetKeyState(VK_XBUTTON1) & (1 << 15));
                    Win32ProcessKeyboardMessage(&new_input->mouse_buttons[4], GetKeyState(VK_XBUTTON2) & (1 << 15));
                    // Set controller to not connected.
                    // TODO(kaelan): Add controller support.
                    new_input->controllers[1].is_connected = false;
                    old_input->controllers[1].is_connected = false;


                    nenjin_offscreen_buffer engine_buffer = {};
                    engine_buffer.memory = global_back_buffer.memory;
                    engine_buffer.width = global_back_buffer.width;
                    engine_buffer.height = global_back_buffer.height;
                    engine_buffer.width_in_bytes = global_back_buffer.width_in_bytes;
                    engine_buffer.bytes_per_pixel = global_back_buffer.bytes_per_pixel;

                    UpdateAndRender(&engine_memory, engine_input, &engine_buffer);
                    LARGE_INTEGER work_counter = Win32GetWallClock();
                    f32 work_seconds_elapsed = Win32GetSecondsElapsed(last_counter, work_counter);

                    if(work_seconds_elapsed < target_seconds_per_frame)
                    {
                        // Only sleep if we could set the scheduler granularity to 1ms.
                        // TODO(kaelan): Check to see if there is a better way to sleep the program.
                        //               This method has some inconsistencies.
                        if(sleep_is_granular)
                        {
                            INT int_sleep_ms = (INT)(1000.0f*(target_seconds_per_frame-work_seconds_elapsed-1.0f));
                            DWORD sleep_ms = (int_sleep_ms < 0 ? 0 : (DWORD)int_sleep_ms);
                            if(sleep_ms > 0)
                            {
                                Sleep(sleep_ms);
                            }
                        }
                        f32 test_seconds_per_frame = Win32GetSecondsElapsed(last_counter, Win32GetWallClock());
                        // NOTE: This burns the rest of the cycles required to meet the frame time.
                        while(work_seconds_elapsed < target_seconds_per_frame)
                        {
                            work_seconds_elapsed = (Win32GetSecondsElapsed(last_counter, Win32GetWallClock()));
                        }
                    }
                    else
                    {
                        // Missed frame.
                    }
                    LARGE_INTEGER end_counter = Win32GetWallClock();
                    // Perf metrics.
                    f32 ms_per_frame = 1000.0f * Win32GetSecondsElapsed(last_counter, end_counter);
                    f32 fps = 1000.0f/ms_per_frame;
                    // Update last_counter so it can be re-used again in the next frame time calculation.
                    last_counter = end_counter;
                    // TODO(kaelan): Setup a fixed timestep for the emulator to run at!
                    HDC device_context = GetDC(window);
                    win32_window_dimensions win_dim = Win32GetWindowDimensions(window);
                    Win32DisplayBufferInWindow(device_context, &global_back_buffer, win_dim.width, win_dim.height);
                    ReleaseDC(window, device_context);

                    // Swap input buffers to preserve this frames input.
                    nenjin_input *temp = new_input;
                    new_input = old_input;
                    old_input = temp;

                    // Debug output 
                    #if 1
                    char Buffer[256];
                    _snprintf_s(Buffer, sizeof(Buffer), " %.02f FPS, %.02fms\n", fps, ms_per_frame);
                    OutputDebugStringA(Buffer);
                    #endif
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
