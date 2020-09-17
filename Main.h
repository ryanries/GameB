// Filename: Main.h
// Declarations that should be shared across multiple compilation units should go here.
//
// Project Codename: GameB
// TODO: Come up with a better name later.
// 2020 Joseph Ryan Ries <ryanries09@gmail.com>
// My YouTube series where we program an entire video game from scratch in C.
// Watch it on YouTube:    https://www.youtube.com/watch?v=3zFFrBSdBvA
// Follow along on GitHub: https://github.com/ryanries/GameB
// Find me on Twitter @JosephRyanRies 
// # License
// ----------
// The source code in this project is licensed under the MIT license.
// The media assets such as artwork, custom fonts, music and sound effects are licensed under a separate license.
// A copy of that license can be found in the 'Assets' directory.
// stb_vorbis by Sean Barrett is public domain and a copy of its license can be found in the stb_vorbis.c file.
// miniz by Rich Geldreich <richgel99@gmail.com> is public domain (or possibly MIT licensed) and a copy of its license can be found in the miniz.c file.

#pragma once

// Disable warning about structure padding.
#pragma warning(disable: 4820)

// Disable warning about Spectre/Meltdown CPU vulnerability.
#pragma warning(disable: 5045)

// Disable warning about function not inlined.
#pragma warning(disable: 4710)

// Temporarily reduce warning level while including header files over which we have no control.
#pragma warning(push, 3)	

// The Windows API.
#include <Windows.h>

// Audio library.
#include <xaudio2.h>

// Audio library.
#pragma comment(lib, "XAudio2.lib")

// stb_vorbis by Sean T. Barrett - http://nothings.org/
#include "stb_vorbis.h"

// String functions such as sprintf, etc.
#include <stdio.h>

// Process Status API, e.g. GetProcessMemoryInfo
#include <psapi.h>

// Xbox 360 gamepad input
#include <Xinput.h>

// Xbox 360 gamepad input.
#pragma comment(lib, "XInput.lib")

// Nicer data types, e.g., uint8_t, int32_t, etc.
#include <stdint.h>

// Windows Multimedia library, we use it for timeBeginPeriod to adjust the global system timer resolution.
#pragma comment(lib, "Winmm.lib")

// Valid options are SSE2, AVX, or nothing.
#define AVX

#ifdef AVX

// AVX (Advanced Vector Extensions)
#include <immintrin.h>

#elif defined SSE2

// SSE2 (Streaming SIMD Extensions)
#include <emmintrin.h>

#endif

// Restore warning level to /Wall.
#pragma warning(pop)

// Maps the tiles in our overworld map to their numerical equivalents
// so we can decide which tiles we're allowed to walk on and which
// tiles block our player's movement, etc.
#include "Tiles.h"

// In debug builds, a failed assertion crashes the entire program.
// In release builds, a failed assertion has no effect.
#ifdef _DEBUG

#define ASSERT(Expression, Message) if (!(Expression)) { *(int*)0 = 0; }

#else

#define ASSERT(Expression, Message) ((void)0);

#endif

#define GAME_NAME	"Game_B"

#define GAME_VER	"0.9a"

#define ASSET_FILE	"Assets.dat"

#define LOG_FILE_NAME GAME_NAME ".log"

// 384x240 is a 16:10 aspect ratio. Most monitors these days are 16:9. 
// So when the game runs at full screen, it will have to be centered with black bars on the sides.
#define GAME_RES_WIDTH	384

#define GAME_RES_HEIGHT	240

#define GAME_BPP		32

#define GAME_DRAWING_AREA_MEMORY_SIZE		(GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

#define CALCULATE_AVG_FPS_EVERY_X_FRAMES	120

#define TARGET_MICROSECONDS_PER_FRAME		16667ULL

// This allows us to play up to 4 sound effects simultaneously
// using the XAudio2 library. 
#define NUMBER_OF_SFX_SOURCE_VOICES			4

// The player will have different sets of armor, so he needs
// sprites for each direction and animation for each suit.
// So for example, gPlayer.Sprite[0][0] would represent the sprite
// for when the player is standing still facing downwards wearing armor #0.
// gPlayer.Sprite[1][0] is player standing still facing downwards in armor #1.
#define SUIT_0	0

#define SUIT_1	1

#define SUIT_2	2

#define FACING_DOWN_0	0

#define FACING_DOWN_1	1

#define FACING_DOWN_2	2

#define FACING_LEFT_0	3

#define FACING_LEFT_1	4

#define FACING_LEFT_2	5

#define FACING_RIGHT_0	6

#define FACING_RIGHT_1	7

#define FACING_RIGHT_2	8

#define FACING_UPWARD_0	9

#define FACING_UPWARD_1	10

#define FACING_UPWARD_2	11

// Every font sheet we use in this game must have the same format,
// which currently is just all 98 ASCII characters in a single row.
#define FONT_SHEET_CHARACTERS_PER_ROW 98

// This is for using the undocumented Windows API function NtQueryTimerResolution.
typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

_NtQueryTimerResolution NtQueryTimerResolution;

/////////// BEGIN GLOBAL ENUMS /////////////

typedef enum DIRECTION
{
	DOWN  = 0,

	LEFT  = 3,

	RIGHT = 6,

	UP    = 9

} DIRECTION;

// Used exclusively by LogMessageA
typedef enum LOGLEVEL
{
	LL_NONE    = 0,

	LL_ERROR   = 1,	

	LL_WARNING = 2,

	LL_INFO    = 3,

	LL_DEBUG   = 4

} LOGLEVEL;

// The game must always be in 1 and only 1 state at a time.
// The game starts in the "opening splash screen" state, then
// transitions to the title screen state, etc. Some 
// transitions are valid and others are not. E.g., we should not
// be able to transition from the battle gamestate directly to the
// splash screen gamestate or vice versa. We track this with the 
// gCurrentGameState and gPreviousGameState variables.
typedef enum GAMESTATE
{
	GAMESTATE_OPENINGSPLASHSCREEN,

	GAMESTATE_TITLESCREEN,

	GAMESTATE_CHARACTERNAMING,

	GAMESTATE_OVERWORLD,

	GAMESTATE_BATTLE,

	GAMESTATE_OPTIONSSCREEN,

	GAMESTATE_EXITYESNOSCREEN,

	GAMESTATE_GAMEPADUNPLUGGED

} GAMESTATE;

// These are the only supported types of assets that we can 
// extract from the compressed assets archive file.
typedef enum RESOURCE_TYPE
{
	RESOURCE_TYPE_WAV,

	RESOURCE_TYPE_OGG,

	RESOURCE_TYPE_TILEMAP,

	RESOURCE_TYPE_BMPX

} RESOURCE_TYPE;

/////////// END GLOBAL ENUMS /////////////

/////////// BEGIN GLOBAL STRUCTS /////////////

// Just like a POINT data structure, but unsigned.
typedef struct UPOINT
{
	uint16_t x;

	uint16_t y;

} UPOINT;

// Stores the current state of all player keyboard+gamepad input,
// as well as the input state of the previous state, so we can tell
// whether a button is being held down or not.
typedef struct GAMEINPUT
{
	int16_t EscapeKeyIsDown;

	int16_t DebugKeyIsDown;

	int16_t LeftKeyIsDown;

	int16_t RightKeyIsDown;

	int16_t UpKeyIsDown;

	int16_t DownKeyIsDown;

	int16_t ChooseKeyIsDown;

	int16_t DebugKeyWasDown;

	int16_t EscapeKeyWasDown;

	int16_t LeftKeyWasDown;

	int16_t RightKeyWasDown;

	int16_t UpKeyWasDown;

	int16_t DownKeyWasDown;

	int16_t ChooseKeyWasDown;

} GAMEINPUT;

// GAMEBITMAP is any sort of bitmap, which might be a sprite, or a background, 
// even the back buffer itself is a GAMEBITMAP.
typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;

	void* Memory;	                        

} GAMEBITMAP;

// Both sound effects and background music are GAMESOUNDs,
// we play them with the XAudio2 library.
typedef struct GAMESOUND
{
	WAVEFORMATEX WaveFormat;

	XAUDIO2_BUFFER Buffer;

} GAMESOUND;

// A 32-bit, 4-byte pixel. Each color goes 0-255.
typedef struct PIXEL32
{
	uint8_t Blue;

	uint8_t Green;

	uint8_t Red;

	uint8_t Alpha;

} PIXEL32;

// Miscellaneous statistics regarding hardware specs, program performance, etc.
typedef struct GAMEPERFDATA
{
	uint64_t TotalFramesRendered;	

	float RawFPSAverage;

	float CookedFPSAverage;	

	int64_t PerfFrequency;	

	MONITORINFO MonitorInfo;

	BOOL DisplayDebugInfo;	

	ULONG MinimumTimerResolution;

	ULONG MaximumTimerResolution;

	ULONG CurrentTimerResolution;

	DWORD HandleCount;

	PROCESS_MEMORY_COUNTERS_EX MemInfo;

	SYSTEM_INFO SystemInfo;

	int64_t CurrentSystemTime;

	int64_t PreviousSystemTime;

	double CPUPercent;

	uint8_t MaxScaleFactor;

	uint8_t CurrentScaleFactor;

} GAMEPERFDATA;

// A two-dimensional array consisting of integers which represent tiles that the 
// player may step on or move around in the overworld. Some of the tiles the 
// player pass through, some the player cannot. This tilemap is a *.tmx file that 
// is generated by the 3rd-party app named "Tiled." The file is an XML document
// that we parse with the LoadTileMap* function.
typedef struct TILEMAP
{
	uint16_t Width;

	uint16_t Height;

	uint8_t** Map;

} TILEMAP;

// A map, for example the overworld. It consists of a graphical component i.e. the GAMEBITMAP,
// and a TILEMAP component that contains the data for which tiles may be stepped on and which may not.
typedef struct GAMEMAP
{
	TILEMAP TileMap;

	GAMEBITMAP GameBitmap;

} GAMEMAP;

// This is the player!
typedef struct HERO
{
	char Name[9];

	GAMEBITMAP Sprite[3][12];	// 3 suits of armor, 12 sprites for each armor set.

	BOOL Active;

	UPOINT ScreenPos;			// Note screen position and world position are two different things.

	UPOINT WorldPos;

	uint8_t MovementRemaining;	// This will be non-zero when the player is in motion. If 0, player is standing still.

	DIRECTION Direction;

	uint8_t CurrentArmor;		// SUIT_0, SUIT_1, or SUIT_2

	uint8_t SpriteIndex;	

	// TODO: Figure out how the stats are going to work.
	int16_t HP;

	int32_t XP;

	int16_t Money;

	int16_t Strength;

	int16_t Luck;

	int16_t MP;

} HERO;

// Any value that will be saved to or loaded from the Windows registry goes here.
typedef struct REGISTRYPARAMS
{
	DWORD LogLevel;

	DWORD SFXVolume;

	DWORD MusicVolume;

	DWORD ScaleFactor;	

} REGISTRYPARAMS;

// Every MENU in the game is a collection of MENUITEMs.
// This includes everything from simple Yes/No choices, to character naming dialogs, to vendor menus, etc.
typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	BOOL Enabled;

	void(*Action)(void);

} MENUITEM;

typedef struct MENU
{
	char* Name;

	uint8_t SelectedItem;

	uint8_t ItemCount;

	MENUITEM** Items;

} MENU;

/////////// END GLOBAL STRUCTS /////////////



GAMEPERFDATA gPerformanceData;

// The "drawing surface" which we blit to the screen once per frame, 60 times per second.
GAMEBITMAP gBackBuffer;

GAMEBITMAP g6x7Font;

GAMEMAP gOverworld01;

GAMESTATE gCurrentGameState;

GAMESTATE gPreviousGameState;

GAMEINPUT gGameInput;

GAMESOUND gSoundSplashScreen;

GAMESOUND gSoundMenuNavigate;

GAMESOUND gSoundMenuChoose;

GAMESOUND gMusicOverworld01;

HERO gPlayer;

float gSFXVolume;

float gMusicVolume;

int8_t gGamepadID;

HWND gGameWindow;                   // A global handle to the game window.

IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

IXAudio2SourceVoice* gXAudioMusicSourceVoice;

uint8_t gPassableTiles[1];

UPOINT gCamera;

HANDLE gAssetLoadingThreadHandle;

// This event gets signalled/set after the most essential assets have been loaded.
// "Essential" means the assets required to render the splash screen.
HANDLE gEssentialAssetsLoadedEvent;

// Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain.
BOOL gGameIsRunning;

/////////// FUNCTION DELCARATIONS /////////////

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

DWORD InitializeHero(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y);

void BlitBackgroundToBuffer(_In_ GAMEBITMAP* GameBitmap);

void BlitStringToBuffer(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32* Color, _In_ uint16_t x, _In_ uint16_t y);

void RenderFrameGraphics(void);

DWORD LoadRegistryParameters(void);

DWORD SaveRegistryParameters(void);

void LogMessageA(_In_ LOGLEVEL LogLevel, _In_ char* Message, _In_ ...);

void DrawDebugInfo(void);

void FindFirstConnectedGamepad(void);

HRESULT InitializeSoundEngine(void);

DWORD LoadWavFromMemory(_In_ void* Buffer, _Inout_ GAMESOUND* GameSound);

DWORD LoadOggFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ GAMESOUND* GameSound);

DWORD LoadTilemapFromMemory(_In_ void* Buffer, _In_ uint32_t BufferSize, _Inout_ TILEMAP* TileMap);

DWORD Load32BppBitmapFromMemory(_In_ void* Buffer, _Inout_ GAMEBITMAP* GameBitmap);

void PlayGameSound(_In_ GAMESOUND* GameSound);

void PlayGameMusic(_In_ GAMESOUND* GameSound);

DWORD LoadAssetFromArchive(_In_ char* ArchiveName, _In_ char* AssetFileName, _In_ RESOURCE_TYPE ResourceType, _Inout_ void* Resource);

#ifdef AVX

void ClearScreen(_In_ __m256i* Color);

#elif defined SSE2

void ClearScreen(_In_ __m128i* Color);

#else

void ClearScreen(_In_ PIXEL32* Color);

#endif

DWORD AssetLoadingThreadProc(_In_ LPVOID lpParam);