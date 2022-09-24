#pragma once

#include "CommonMain.h"

#ifdef _WIN32

#define EssentialAssetsAreLoaded() (WaitForSingleObject(gEssentialAssetsLoadedEvent, 0) == WAIT_OBJECT_0)
#define AssetThreadCompleted() (WaitForSingleObject(gAssetLoadingThreadHandle, 0) == WAIT_OBJECT_0)
#define AssetThreadFailed() WIN32_AssetThreadFailed()

// Any value that will be saved to or loaded from the Windows registry goes here.
typedef struct REGISTRYPARAMS
{
	DWORD LogLevel;

	DWORD SFXVolume;

	DWORD MusicVolume;

	DWORD ScaleFactor;

	DWORD DebugKey;

} REGISTRYPARAMS;

// A global handle to the game window.
extern HWND gGameWindow;

// This critical section is used to synchronize LogMessageA between multiple threads.
extern CRITICAL_SECTION gLogCritSec;

// Any values that are read from and stored in Windows registry go in this struct.
extern REGISTRYPARAMS gRegistryParams;

// The background thread that loads assets from the compressed archive during the splash screen.
extern HANDLE gAssetLoadingThreadHandle;

// This event gets signalled/set after the most essential assets have been loaded.
// "Essential" means the assets required to render the splash screen, which is
// the basic font and the weird noise that plays at the beginning of the splash screen.
extern HANDLE gEssentialAssetsLoadedEvent;

// COM interfaces for the XAudio2 library.
// Having 4 source voices for sound effects means we can play up to 
// 4 sound effects simultaneously.
extern IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

// We only need 1 source voice for music because we never play overlapping music tracks.
extern IXAudio2SourceVoice* gXAudioMusicSourceVoice;

bool WIN32_AssetThreadFailed(void);

#else

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pulse/pulseaudio.h>
#include <pthread.h>

#define EssentialAssetsAreLoaded() gAssetThreadStatus.essential_assets_loaded
#define AssetThreadCompleted() gAssetThreadStatus.completed
#define AssetThreadFailed() gAssetThreadStatus.failed

#define MUSIC_QUEUE_SIZE 4

typedef struct PulseSound
{
    bool loop;
    size_t samples;
    size_t position;
    int16_t *data;
} PulseSound;

typedef struct AssetThreadStatus
{
    bool completed;
    bool failed;
    bool essential_assets_loaded;
} AssetThreadStatus;

extern pa_threaded_mainloop *gPulseLoop;
extern PulseSound gBackgroundMusic[MUSIC_QUEUE_SIZE];
extern PulseSound gSFX[NUMBER_OF_SFX_SOURCE_VOICES];
extern bool gMusicIsPaused;
extern int gMusicIndex;
extern volatile AssetThreadStatus gAssetThreadStatus;
extern pthread_mutex_t gLogMutex;

void safe_strncpy(char *restrict dst, size_t dst_size, const char *restrict src, size_t count);

#endif

void PlayGameSound(GAMESOUND* GameSound);
void PlayGameMusic(GAMESOUND* GameSound, bool Loop, bool Immediate);
void StopAllGameSounds(void);
void PauseMusic(void);
void StopMusic(void);
bool MusicIsPlaying(void);
void LogMessageA(LOGLEVEL LogLevel, char *Message, ...);
uint32_t LoadSettings(void);
uint32_t SaveSettings(void);
