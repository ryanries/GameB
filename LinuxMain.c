#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <pulse/pulseaudio.h>
#include <pthread.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <linux/input.h>

#include "CommonMain.h"
#include "Platform.h"
#include "OpeningSplashScreen.h"
#include "TitleScreen.h"
#include "CharacterNamingScreen.h"
#include "Overworld.h"
#include "Battle.h"
#include "ExitYesNoScreen.h"
#include "OptionsScreen.h"
#include "NewGameAreYouSure.h"
#include "GamepadUnplugged.h"

#define BITS_TO_LONGS(x) \
        (((x) + 8 * sizeof (unsigned long) - 1) / (8 * sizeof (unsigned long)))

typedef void *(*thread_start_ptr_t)(void *);
typedef void (*swap_interval_ptr_t)(Display *, GLXDrawable, int);

typedef struct X11Window
{
    Display *display;
    Window window;
} X11Window;

static unsigned long test_bit(unsigned long bit, unsigned long *bitfield)
{
    unsigned long bits_per_element = sizeof(unsigned long) * 8;
    unsigned long index = bit / bits_per_element;
    bit = bit % bits_per_element;
    return (bitfield[index] >> bit) & 1;
}

static int find_gamepad(void)
{
    DIR *dir = opendir("/dev/input");
    struct dirent *ent;
    const char *event_str = "event";
    size_t event_str_len = strlen(event_str);
    const char *input_path = "/dev/input/";
    size_t input_path_len = strlen(input_path);
    char full_device_path[32];
    size_t max_entry_len = sizeof(full_device_path) - input_path_len - 1;
    memcpy(full_device_path, input_path, input_path_len);
    char *entry_str_start = full_device_path + input_path_len;
    while ((ent = readdir(dir)) != NULL) {
        size_t entry_len = strlen(ent->d_name);
        // Ignore non-event based devices (only look for /dev/input/event*)
        if (entry_len > max_entry_len || entry_len <= event_str_len || memcmp(ent->d_name, event_str, event_str_len) != 0) {
            continue;
        }
        memcpy(entry_str_start, ent->d_name, entry_len + 1);
        int fd = open(full_device_path, O_RDONLY);
        if (fd == -1) {
            continue;
        }
        unsigned long key_bitfield[BITS_TO_LONGS(KEY_CNT)];
        // Query device for which buttons it has.
        if (ioctl(fd, EVIOCGBIT(EV_KEY, BITS_TO_LONGS(KEY_CNT) * sizeof(unsigned long)), key_bitfield) == -1) {
            close(fd);
            continue;
        }
        // Filter out devices that don't have the buttons we need.
        if (!test_bit(BTN_A, key_bitfield) || !test_bit(BTN_Y, key_bitfield) || !test_bit(BTN_SELECT, key_bitfield)) {
            close(fd);
            continue;
        }
        unsigned long abs_bitfield[BITS_TO_LONGS(ABS_CNT)];
        // D-Pad is reported as an axis so check for that too.
        if (ioctl(fd, EVIOCGBIT(EV_ABS, BITS_TO_LONGS(ABS_CNT) * sizeof(unsigned long)), abs_bitfield) == -1) {
            close(fd);
            continue;
        }
        if (!test_bit(ABS_HAT0X, abs_bitfield) || !test_bit(ABS_HAT0Y, abs_bitfield)) {
            close(fd);
            continue;
        }
        closedir(dir);
        return fd;
    }
    closedir(dir);
    return -1;
}

void *asset_loading_proc(void *arg)
{
    if (LoadEssentialAssets() != ERROR_SUCCESS) {
        goto Fail;
    }
    gAssetThreadStatus.essential_assets_loaded = true;
    if (LoadNonEssentialAssets() != ERROR_SUCCESS) {
        goto Fail;
    }
    gAssetThreadStatus.completed = true;
    return NULL;

    Fail:
    gAssetThreadStatus.failed = true;
    gAssetThreadStatus.completed = true;
    return NULL;
}

static void RenderFrameGraphics(void)
{
    switch(gCurrentGameState) {
        case GAMESTATE_OPENINGSPLASHSCREEN:
            DrawOpeningSplashScreen();
            break;
        case GAMESTATE_GAMEPADUNPLUGGED:
            DrawGamepadUnplugged();
            break;
        case GAMESTATE_TITLESCREEN:
            DrawTitleScreen();
            break;
        case GAMESTATE_CHARACTERNAMING:
            DrawCharacterNaming();
            break;
        case GAMESTATE_OVERWORLD:
            DrawOverworld();
            break;
        case GAMESTATE_BATTLE:
            DrawBattle();
            break;
        case GAMESTATE_EXITYESNOSCREEN:
            DrawExitYesNoScreen();
            break;
        case GAMESTATE_OPTIONSSCREEN:
            DrawOptionsScreen();
            break;
        case GAMESTATE_NEWGAMEAREYOUSURE:
            DrawNewGameAreYouSure();
            break;
        default:
            ASSERT(false, "Gamestate not implemented!");
    }

    if (gPerformanceData.DisplayDebugInfo) {
        DrawDebugInfo();
    }

    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(
        (gPerformanceData.MonitorWidth / 2) - ((GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor) / 2),
        (gPerformanceData.MonitorHeight / 2) - ((GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor) / 2),
        GAME_RES_WIDTH * gPerformanceData.CurrentScaleFactor,
        GAME_RES_HEIGHT * gPerformanceData.CurrentScaleFactor
    );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, GAME_RES_WIDTH, GAME_RES_HEIGHT, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, gBackBuffer.Memory);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLES);

    // bottom triangle

    glTexCoord2f(0.0f, 0.0f);

    glVertex2i(-1, -1);    

    glTexCoord2f(1.0f, 0.0f);

    glVertex2i(1, -1);    

    glTexCoord2f(1.0f, 1.0f);

    glVertex2i(1, 1);    

    // top triangle

    glTexCoord2f(0.0f, 0.0f);

    glVertex2i(-1, -1);

    glTexCoord2f(1.0f, 1.0f);

    glVertex2i(1, 1);

    glTexCoord2f(0.0f, 1.0f);

    glVertex2i(-1, 1);

    glEnd();
}

void pulse_context_state_callback(pa_context *context, void *pulse_loop)
{
    pa_threaded_mainloop_signal(pulse_loop, 0);
}

void pulse_stream_state_callback(pa_stream *stream, void *pulse_loop)
{
    pa_threaded_mainloop_signal(pulse_loop, 0);
}

void pulse_write_callback(pa_stream *stream, size_t bytes, void *userdata)
{
    while (bytes > 0) {
        float *buffer;
        size_t buffer_size = bytes;
        pa_stream_begin_write(stream, (void**)&buffer, &buffer_size);
        if (buffer_size > bytes) {
            buffer_size = bytes;
        }
        bytes -= buffer_size;
        size_t samples = buffer_size / 4;
        memset(buffer, 0, buffer_size);
        if (!gMusicIsPaused) {
            for (size_t s = 0; s < samples && gBackgroundMusic[gMusicIndex].data != NULL; s++) {
                buffer[s] = (gBackgroundMusic[gMusicIndex].data[gBackgroundMusic[gMusicIndex].position] / 36728.0f) * gMusicVolume;
                gBackgroundMusic[gMusicIndex].position++;
                if (gBackgroundMusic[gMusicIndex].position >= gBackgroundMusic[gMusicIndex].samples) {
                    if (gBackgroundMusic[gMusicIndex].loop) {
                        gBackgroundMusic[gMusicIndex].position = 0;
                    } else {
                        memset(gBackgroundMusic + gMusicIndex, 0, sizeof(PulseSound));
                        gMusicIndex++;
                        if (gMusicIndex >= MUSIC_QUEUE_SIZE) {
                            gMusicIndex = 0;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < NUMBER_OF_SFX_SOURCE_VOICES; i++) {
            for (size_t s = 0; s < samples && gSFX[i].data != NULL; s++) {
                // Sound effects are mono so we copy them twice to convert to stereo.
                float sample = (gSFX[i].data[gSFX[i].position] / 36728.0f) * gSFXVolume;
                buffer[s] += sample;
                s++;
                buffer[s] += sample;
                gSFX[i].position++;
                if (gSFX[i].position >= gSFX[i].samples) {
                    memset(gSFX + i, 0, sizeof(PulseSound));
                }
            }
        }
        pa_stream_write(stream, buffer, buffer_size, NULL, 0, PA_SEEK_RELATIVE);
    }
}

static pa_threaded_mainloop *init_pulse_audio(void)
{
    pa_threaded_mainloop *pulse_loop = NULL;
    pa_mainloop_api *api = NULL;
    pa_context *context = NULL;
    pa_stream *stream = NULL;
    bool locked = false;

    pulse_loop = pa_threaded_mainloop_new();
    if (pulse_loop == NULL) {
        LogMessageA(LL_ERROR, "[%s] pa_threaded_mainloop_new failed", __FUNCTION__);
        goto Fail;
    }
    api = pa_threaded_mainloop_get_api(pulse_loop);
    if (api == NULL) {
        LogMessageA(LL_ERROR, "[%s] pa_threaded_mainloop_get_api failed", __FUNCTION__);
        goto Fail;
    }
    context = pa_context_new(api, GAME_NAME);
    if (context == NULL) {
        LogMessageA(LL_ERROR, "[%s] pa_context_new failed", __FUNCTION__);
        goto Fail;
    }
    pa_context_set_state_callback(context, pulse_context_state_callback, pulse_loop);
    pa_threaded_mainloop_lock(pulse_loop);
    locked = true;
    if (pa_threaded_mainloop_start(pulse_loop) != 0) {
        LogMessageA(LL_ERROR, "[%s] pa_threaded_mainloop_start failed", __FUNCTION__);
        goto Fail;
    }
    if (pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) != 0) {
        LogMessageA(LL_ERROR, "[%s] pa_context_connect failed", __FUNCTION__);
        goto Fail;
    }
    while (1) {
        pa_context_state_t context_state = pa_context_get_state(context);
        if (!PA_CONTEXT_IS_GOOD(context_state)) {
            LogMessageA(LL_ERROR, "[%s] Bad pulse audio context", __FUNCTION__);
            goto Fail;
        }
        if (context_state == PA_CONTEXT_READY) {
            break;
        }
        pa_threaded_mainloop_wait(pulse_loop);
    }
    pa_sample_spec spec;
    spec.format = PA_SAMPLE_FLOAT32LE;
    spec.rate = 44100;
    spec.channels = 2;
    stream = pa_stream_new(context, "Audio Stream", &spec, NULL);
    if (stream == NULL) {
        LogMessageA(LL_ERROR, "[%s] pa_stream_new failed", __FUNCTION__);
        goto Fail;
    }
    pa_stream_set_state_callback(stream, pulse_stream_state_callback, pulse_loop);
    pa_stream_set_write_callback(stream, pulse_write_callback, NULL);
    // I was noticing high latency on the audio particularly when naviagting the menus.
    // This requests a lower latency (-1 means use default value).
    pa_buffer_attr stream_attributes;
    stream_attributes.maxlength = -1;
    stream_attributes.tlength = pa_usec_to_bytes(16667, &spec); // 16.6 ms, time for 1 frame at 60fps
    stream_attributes.prebuf = -1;
    stream_attributes.minreq = -1;
    stream_attributes.fragsize = -1;
    if (pa_stream_connect_playback(stream, NULL, &stream_attributes, PA_STREAM_ADJUST_LATENCY, NULL, NULL) != 0) {
        LogMessageA(LL_ERROR, "[%s] pa_stream_connect_playback failed", __FUNCTION__);
        goto Fail;
    }
    while (1) {
        pa_stream_state_t stream_state = pa_stream_get_state(stream);
        if (!PA_STREAM_IS_GOOD(stream_state)) {
            LogMessageA(LL_ERROR, "[%s] Bad pulse audio stream");
            goto Fail;
        }
        if (stream_state == PA_STREAM_READY) {
            break;
        }
        pa_threaded_mainloop_wait(pulse_loop);
    }
    pa_threaded_mainloop_unlock(pulse_loop);

    return pulse_loop;

    Fail:

    if (stream) {
        pa_stream_disconnect(stream);
    }
    if (context) {
        pa_context_disconnect(context);
    }
    if (pulse_loop) {
        if (locked) {
            pa_threaded_mainloop_unlock(pulse_loop);
        }
        pa_threaded_mainloop_free(pulse_loop);
    }

    return NULL;
}

static X11Window init_x11(void)
{
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        LogMessageA(LL_ERROR, "[%s] XOpenDisplay failed", __FUNCTION__);
        exit(1);
    }
    Window window = XCreateSimpleWindow(
        display, DefaultRootWindow(display),
        0, 0, 640, 480, 0, 0, 0
    );

    // This is needed to prevent flickering on window resize.
    XSetWindowBackgroundPixmap(display, window, None);

    // Set window title with the game name.
    XStoreName(display, window, GAME_NAME);

    // Create the event for window close.
    Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(display, window, &wm_delete, 1);

    // Fullscreen
    Atom wm_state = XInternAtom(display, "_NET_WM_STATE", True);
    Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);

    XMapWindow(display, window);
    int glxAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    XVisualInfo *visual = glXChooseVisual(display, 0, glxAttributes);
    GLXContext context = glXCreateContext(display, visual, NULL, GL_TRUE);
    glXMakeCurrent(display, window, context);

    // Enable VSync
    swap_interval_ptr_t swap_ptr = (swap_interval_ptr_t)glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT");
    if (swap_ptr) {
        swap_ptr(display, window, 1);
    }

    Window root_window;
    int x;
    int y;
    unsigned int width;
    unsigned int height;
    unsigned int border_width;
    unsigned int depth;

    // Window will have re-sized to fullscreen by this point.  This returns the window width and height which should also be the monitor's resolution.
    XGetGeometry(display, window, &root_window, &x, &y, &width, &height, &border_width, &depth);

    for (uint8_t i = 1; i < 12; i++) {
        if (GAME_RES_WIDTH * i > width || GAME_RES_HEIGHT * i > height) {
            gPerformanceData.MaxScaleFactor = i - 1;
            break;
        }
    }

    if (gPerformanceData.CurrentScaleFactor == 0 || gPerformanceData.CurrentScaleFactor > gPerformanceData.MaxScaleFactor) {
        gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;
    }
    gPerformanceData.MonitorWidth = width;
    gPerformanceData.MonitorHeight = height;

    X11Window x11;
    x11.display = display;
    x11.window = window;
    return x11;
}

static uint8_t key_pressed(KeySym key_sym, uint8_t *key_map, Display *display)
{
    uint8_t key_code = XKeysymToKeycode(display, key_sym);
    return (key_map[key_code / 8] >> (key_code % 8)) & 1;
}

static void ProcessPlayerInput(X11Window x11)
{
    uint8_t key_map[32];
    XQueryKeymap(x11.display, (char *)key_map);
    gGameInput.EscapeKeyIsDown = key_pressed(XK_Escape, key_map, x11.display);
    gGameInput.DebugKeyIsDown = key_pressed(XK_F1, key_map, x11.display);
    gGameInput.LeftKeyIsDown = key_pressed(XK_Left, key_map, x11.display);
    gGameInput.RightKeyIsDown = key_pressed(XK_Right, key_map, x11.display);
    gGameInput.UpKeyIsDown = key_pressed(XK_Up, key_map, x11.display);
    gGameInput.DownKeyIsDown = key_pressed(XK_Down, key_map, x11.display);
    gGameInput.ChooseKeyIsDown = key_pressed(XK_Return, key_map, x11.display);
    gGameInput.InvKeyIsDown = key_pressed(XK_I, key_map, x11.display);

    /*
     * TODO: This is taking the "no library approach" to gamepad polling but it has downsides.
     * Different controllers report the buttons differently.  There is supposed to be a standard for gamepad mappings in the kernel but it's not always followed.
     * This is the mapping that my Xbox One S controller has over USB (and even this same controller repots some buttons differently when connected over Bluetooth...)
     * It's likely this won't work with other controllers.
     *
     * As an example: This controller reports its D-pad as if it were a hat on a joystick (the ABS is generally for analog sticks)
     * Other controllers will use the (probably correct) BTN_DPAD* mappings.
     *
     * To support more controllers, we will probably need a list of mappings and handle that somehow.
     * SDL does a pretty good job at this and presents controllers similarly to how XInput does on Windows.
     * We could borrow some of their code (it's zlib licensed) or even just link with it for controller support.
     */

    if (gGamepadID != -1) {
        unsigned long key_bitfield[BITS_TO_LONGS(KEY_CNT)];
        struct input_absinfo x_axis, y_axis;
        if (ioctl(gGamepadID, EVIOCGKEY(BITS_TO_LONGS(KEY_CNT) * sizeof(unsigned long)), key_bitfield) == -1
                || ioctl(gGamepadID, EVIOCGABS(ABS_HAT0X), &x_axis) == -1
                || ioctl(gGamepadID, EVIOCGABS(ABS_HAT0Y), &y_axis) == -1) {
            close(gGamepadID);
            gGamepadID = -1;
            gPreviousGameState = gCurrentGameState;
            gCurrentGameState = GAMESTATE_GAMEPADUNPLUGGED;
            LogMessageA(LL_WARNING, "[%s] Gamepad unplugged! Transitioning from game state %d to %d.",
                __FUNCTION__,
                gPreviousGameState,
                gCurrentGameState);
        } else {
            gGameInput.EscapeKeyIsDown |= test_bit(BTN_SELECT, key_bitfield);
            gGameInput.ChooseKeyIsDown |= test_bit(BTN_A, key_bitfield);
            gGameInput.InvKeyIsDown |= test_bit(BTN_Y, key_bitfield);
            if (x_axis.value == -1) {
                gGameInput.LeftKeyIsDown = 1;
            } else if (x_axis.value == 1) {
                gGameInput.RightKeyIsDown = 1;
            }
            if (y_axis.value == -1) {
                gGameInput.UpKeyIsDown = 1;
            } else if (y_axis.value == 1) {
                gGameInput.DownKeyIsDown = 1;
            }
        }
    }

    if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown) {
        gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
    }

    if (!gInputEnabled) {
        goto InputDisabled;
    }
    Window focus_window = 0;
    int ignored;
    XGetInputFocus(x11.display, &focus_window, &ignored);
    if (focus_window != x11.window) {
        goto InputDisabled;
    }

    switch(gCurrentGameState) {
        case GAMESTATE_OPENINGSPLASHSCREEN:
            PPI_OpeningSplashScreen();
            break;
        case GAMESTATE_GAMEPADUNPLUGGED:
            PPI_GamepadUnplugged();
            break;
        case GAMESTATE_TITLESCREEN:
            PPI_TitleScreen();
            break;
        case GAMESTATE_CHARACTERNAMING:
            PPI_CharacterNaming();
            break;
        case GAMESTATE_OVERWORLD:
            PPI_Overworld();
            break;
        case GAMESTATE_BATTLE:
            PPI_Battle();
            break;
        case GAMESTATE_OPTIONSSCREEN:
            PPI_OptionsScreen();
            break;
        case GAMESTATE_EXITYESNOSCREEN:
            PPI_ExitYesNo();
            break;
        case GAMESTATE_NEWGAMEAREYOUSURE:
            PPI_NewGameAreYouSure();
            break;
        default:
            ASSERT(false, "Unknown game state!");
    }

    InputDisabled:

    gGameInput.DebugKeyWasDown  = gGameInput.DebugKeyIsDown;
    gGameInput.LeftKeyWasDown   = gGameInput.LeftKeyIsDown;
    gGameInput.RightKeyWasDown  = gGameInput.RightKeyIsDown;
    gGameInput.UpKeyWasDown     = gGameInput.UpKeyIsDown;
    gGameInput.DownKeyWasDown   = gGameInput.DownKeyIsDown;
    gGameInput.ChooseKeyWasDown = gGameInput.ChooseKeyIsDown;
    gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;
    gGameInput.InvKeyWasDown     = gGameInput.InvKeyIsDown;
}

static bool create_thread(thread_start_ptr_t start_routine)
{
    pthread_t thread;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        return false;
    }
    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        pthread_attr_destroy(&attr);
        return false;
    }
    if (pthread_create(&thread, &attr, start_routine, NULL) != 0) {
        pthread_attr_destroy(&attr);
        return false;
    }
    pthread_attr_destroy(&attr);
    return true;
}

// Return memory usage in pages.  Multiply by page_size to get bytes.
static long get_memory_usage(void)
{
    char buffer[64];
    int fd = open("/proc/self/statm", O_RDONLY);
    if (fd == -1) {
        return 0;
    }
    ssize_t bytes_read = read(fd, buffer, 63);
    if (bytes_read < 1) {
        close(fd);
        return 0;
    }
    close(fd);
    buffer[bytes_read] = '\0';
    char *ptr = buffer;
    while (*ptr != ' ' && *ptr != '\0') {
        ptr++;
    }
    long residential = strtol(ptr, &ptr, 10);
    long shared = strtol(ptr, &ptr, 10);
    return residential - shared;
}

static uint32_t get_open_fds(void)
{
    DIR *dir = opendir("/proc/self/fd");
    if (dir == NULL) {
        return 0;
    }
    uint32_t count = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (ent->d_name[0] != '.') {
            count++;
        }
    }
    closedir(dir);
    return count;
}

int main(void)
{
    if (!create_thread(asset_loading_proc)) {
        LogMessageA(LL_ERROR, "[%s] Failed to create asset loading thread", __FUNCTION__);
        return 1;
    }
    srand(time(NULL));
    gBackBuffer.Memory = malloc(GAME_DRAWING_AREA_MEMORY_SIZE);
    if (gBackBuffer.Memory == NULL) {
        LogMessageA(LL_ERROR, "[%s] malloc failed", __FUNCTION__);
        return 1;
    }
    int page_size = getpagesize();
    int number_of_cpus = get_nprocs();
    LoadSettings();
    gGamepadID = find_gamepad();
    X11Window x11 = init_x11();
    gPulseLoop = init_pulse_audio();
    ResetEverythingForNewGame();

    int64_t raw_time = 0;
    int64_t cooked_time = 0;
    struct rusage cpu_start;
    getrusage(RUSAGE_SELF, &cpu_start);
    struct timespec frame_start;
    clock_gettime(CLOCK_MONOTONIC, &frame_start);
    struct timespec last_cpu_report = frame_start;
    while (1) {
        while (XPending(x11.display) > 0) {
            XEvent event;
            XNextEvent(x11.display, &event);
            if (event.type == ClientMessage) {
                return 0;
            }
        }
        ProcessPlayerInput(x11);
        RenderFrameGraphics();
        glXSwapBuffers(x11.display, x11.window);
        gPerformanceData.TotalFramesRendered++;
        struct timespec frame_end;
        clock_gettime(CLOCK_MONOTONIC, &frame_end);
        int64_t elapsed = frame_end.tv_sec - frame_start.tv_sec;
        elapsed *= 1000000000;
        elapsed += frame_end.tv_nsec - frame_start.tv_nsec;
        raw_time += elapsed;
        if (elapsed < TARGET_NANOSECONDS_PER_FRAME) {
            struct timespec sleep_time;
            sleep_time.tv_sec = 0;
            sleep_time.tv_nsec = TARGET_NANOSECONDS_PER_FRAME - elapsed;
            nanosleep(&sleep_time, NULL);
            clock_gettime(CLOCK_MONOTONIC, &frame_end);
            elapsed = frame_end.tv_sec - frame_start.tv_sec;
            elapsed *= 1000000000;
            elapsed += frame_end.tv_nsec - frame_start.tv_nsec;
        }
        cooked_time += elapsed;
        if (gPerformanceData.TotalFramesRendered % CALCULATE_STATS_EVERY_X_FRAMES == 0) {
            struct rusage cpu_end;
            getrusage(RUSAGE_SELF, &cpu_end);
            int64_t cpu_seconds = (cpu_end.ru_utime.tv_sec + cpu_end.ru_stime.tv_sec) - (cpu_start.ru_utime.tv_sec + cpu_start.ru_stime.tv_sec);
            int64_t cpu_microseconds = (cpu_end.ru_utime.tv_usec + cpu_end.ru_stime.tv_usec) - (cpu_start.ru_utime.tv_usec + cpu_start.ru_stime.tv_usec);
            int64_t cpu_nanoseconds = (cpu_seconds * 1000000000) + (cpu_microseconds * 1000);

            int64_t cpu_real_time = frame_end.tv_sec - last_cpu_report.tv_sec;
            cpu_real_time *= 1000000000;
            cpu_real_time += frame_end.tv_nsec - last_cpu_report.tv_nsec;

            gPerformanceData.CPUPercent = (double)cpu_nanoseconds / (double)cpu_real_time;
            gPerformanceData.CPUPercent /= number_of_cpus;
            gPerformanceData.CPUPercent *= 100.0;
            gPerformanceData.MemoryUsage = get_memory_usage() * page_size;
            gPerformanceData.RawFPSAverage = 1000000000.0f / ((float)raw_time / (float)CALCULATE_STATS_EVERY_X_FRAMES);
            gPerformanceData.CookedFPSAverage = 1000000000.0f / ((float)cooked_time / (float)CALCULATE_STATS_EVERY_X_FRAMES);
            gPerformanceData.HandleCount = get_open_fds();

            if (gGamepadID == -1) {
                gGamepadID = find_gamepad();
            }

            raw_time = 0;
            cooked_time = 0;
            last_cpu_report = frame_end;
            cpu_start = cpu_end;
        }
        frame_start = frame_end;
    }
    return 0;
}
