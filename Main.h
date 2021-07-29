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

// For rand_s().
#define _CRT_RAND_S

// Disable warning about structure padding.
#pragma warning(disable: 4820)

// Disable warning about Spectre/Meltdown CPU vulnerability.
#pragma warning(disable: 5045)

// Disable warning about function not inlined.
#pragma warning(disable: 4710)

// Disable warning about function being inlined.
#pragma warning(disable: 4711)

// Temporarily reduce warning level while including external 
// header files over which we have no control.
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

// Windows Multimedia library, we use it for timeBeginPeriod 
// to adjust the global system timer resolution.
#pragma comment(lib, "Winmm.lib")

#ifdef __AVX2__

// AVX (Advanced Vector Extensions)
// This advanced instruction set began appearing in CPUs circa 2013-2015.
#include <immintrin.h>

#endif

// Restore warning level to /Wall.
#pragma warning(pop)

// Maps the tiles in our overworld map to their numerical equivalents
// so we can decide which tiles we're allowed to walk on and which
// tiles block our player's movement, hurt or slow the player, etc.
#include "Tiles.h"

// In debug builds, a failed assertion crashes the entire program, which gives us an
// opportunity to break in with a debugger. In release builds, a failed assertion has
// no effect. My first version of this ASSERT macro was *(int *)0 = 0;, which worked
// in MSVC but Clang complained about it. Turns out, Clang might optimize it away
// causing the program to not ASSERT when it should. I could use *(volatile int *)0 = 0; 
// to avoid any possible optimization and remove the warning from Clang, but instead I
// updated both the Clang and MSVC versions to the UD2 CPU instruction, which is guarnateed
// to raise an exception and not get optimized away.
#ifdef _DEBUG

#ifdef CLANG

#define ASSERT(Expression, Message) if (!(Expression)) { __builtin_trap(); }

#else

#define ASSERT(Expression, Message) if (!(Expression)) { __ud2(); }

#endif

#else

#define ASSERT(Expression, Message) ((void)0);

#endif

#define GAME_NAME				"Game_B"

#define GAME_VER				"0.9a"

#define ASSET_FILE				"Assets.dat"

#define LOG_FILE_NAME GAME_NAME ".log"

// 384x240 is a 16:10 aspect ratio. Most monitors these days are 16:9. 
// So when the game runs at full screen, it will have to be centered with black bars on the sides.
#define GAME_RES_WIDTH	384

#define GAME_RES_HEIGHT	240

#define GAME_BPP		32

#define GAME_DRAWING_AREA_MEMORY_SIZE		(GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))

// 16.67 milliseconds is 60 frames per second.
#define TARGET_MICROSECONDS_PER_FRAME		16667ULL

// Every 120 frames/2 seconds we will calculate some statistics such as average FPS, CPU usage, etc.
#define CALCULATE_STATS_EVERY_X_FRAMES		120

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

// The number of steps the player must take after the previous random monster
// encounter before another random monster encounter can take place.
#define RANDOM_MONSTER_GRACE_PERIOD_STEPS 3

// The number of frames taken by the fade animation.
// This involves using shades of grey that aren't in the NES color palette,
// but it looks nicer.
#define FADE_DURATION_FRAMES 50

// Some pixel colors that conform to the original NES color palette.
#define COLOR_NES_WHITE	(PIXEL32) { .Bytes = 0xFFFCFCFC }

#define COLOR_NES_BLACK	(PIXEL32) { .Bytes = 0xFF000000 }


// Some pixel colors that are handy but didn't exist in the NES color palette.

#define COLOR_GRAY_0	(PIXEL32) { .Bytes = 0xFF202020 }




/////////// BEGIN GLOBAL ENUMS /////////////

// Which direction is the character facing?
// This is used to index into an array to provide walking animations.
typedef enum DIRECTION
{
	DOWN  = 0,

	LEFT  = 3,

	RIGHT = 6,

	UP    = 9

} DIRECTION;

// Used exclusively by LogMessageA
// LogLevel can be changed in the Windows registry.
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
// TODO: Do we want to enforce valid state transitions?
typedef enum GAMESTATE
{
	GAMESTATE_OPENINGSPLASHSCREEN,

	GAMESTATE_TITLESCREEN,

	GAMESTATE_CHARACTERNAMING,

	GAMESTATE_OVERWORLD,

	GAMESTATE_BATTLE,

	GAMESTATE_OPTIONSSCREEN,

	GAMESTATE_EXITYESNOSCREEN,

	GAMESTATE_GAMEPADUNPLUGGED,

	GAMESTATE_NEWGAMEAREYOUSURE

} GAMESTATE;

// These are the only supported types of assets that we can 
// extract from the compressed assets archive file.
// TODO: We probably don't even need this, we can just infer type from file extension.
typedef enum RESOURCE_TYPE
{
	RESOURCE_TYPE_WAV,

	RESOURCE_TYPE_OGG,

	RESOURCE_TYPE_TILEMAP,

	RESOURCE_TYPE_BMPX

} RESOURCE_TYPE;

// For use in the DrawWindow function.
typedef enum WINDOW_FLAGS
{
	WINDOW_FLAG_BORDERED = 1,				// 1 << 0

	WINDOW_FLAG_OPAQUE = 2,					// 1 << 1, 0b00000010

	WINDOW_FLAG_HORIZONTALLY_CENTERED = 4,	// 1 << 2, 0b00000100

	WINDOW_FLAG_VERTICALLY_CENTERED = 8,	// 1 << 3, 0b00001000

	WINDOW_FLAG_SHADOW = 16,				// 1 << 4, 0b00010000	
	
	WINDOW_FLAG_SHAKE = 32,					// 1 << 5, 0b00100000

	WINDOW_FLAG_ROUNDED_CORNERS = 64,		// 1 << 6, 0b01000000

	WINDOW_FLAG_THICK = 128					// 1 << 7, 0b10000000

} WINDOW_FLAGS;

/////////// END GLOBAL ENUMS /////////////

/////////// BEGIN GLOBAL STRUCTS /////////////

// Just like a POINT data structure, but unsigned.
typedef struct UPOINT
{
	uint16_t x;

	uint16_t y;

} UPOINT;

// Stores the current state of all player keyboard+gamepad input,
// as well as the previous state from the last frame, so we can tell
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
// or a font sheet, or even the back buffer itself.
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

// A rectangular subsection of our entire overworld.
// Might be a town, or a dungeon, or a cave, or whatever.
typedef struct GAMEAREA
{
	char* Name;

	RECT Area;

	GAMESOUND* Music;

} GAMEAREA;


// A 32-bit, 4-byte pixel. Each color is 0-255.
// This is a union so a PIXEL32 can either be referred to
// as an unsigned 32-bit whole, or as a struct with 4 separate 8-bit components.
typedef union PIXEL32 {

	struct Colors {
			
		uint8_t Blue;

		uint8_t Green;

		uint8_t Red;

		uint8_t Alpha;

	} Colors;

	DWORD Bytes;

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

	BOOL Active;

	GAMEBITMAP Sprite[3][12];	// 3 suits of armor, 12 sprites for each armor set.	

	UPOINT ScreenPos;			// Note screen position and world position are two different things.

	UPOINT WorldPos;

	// This will be non-zero when the player is in motion. If 0, player is standing still.
	uint8_t MovementRemaining;	

	// Is the player facing left, right, up or down?
	DIRECTION Direction;

	// This is used to allow the player to stand on a portal (or doorway, or stairs, etc.,) 
	// and teleport to the other end of the portal and stand on the portal without 
	// instantly being teleported back because they're currently standing on a portal.
	BOOL HasPlayerMovedSincePortal;	

	// SUIT_0, SUIT_1, or SUIT_2
	uint8_t CurrentArmor;		

	uint8_t SpriteIndex;	

	uint64_t StepsTaken;

	// 90 = 10% chance, 80 = 20% chance, etc. 100 = 0% chance.
	uint8_t RandomEncounterPercentage;

	uint64_t StepsSinceLastRandomMonsterEncounter;

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
// This includes everything from simple Yes/No choices, 
// to character naming dialogs, to vendor menus, etc.
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

/////////// BEGIN GLOBAL VARIABLE DECLARATIONS ///////////

// Every global variable that will be shared among multiple compilation units/*.c files
// needs the extern keyword. When using the extern keyword, the global variable must
// also be assigned a value at the beginning of one and only one *.c file. For these
// variables that will of course be Main.c. Unfortunately simultaneous declaration and
// assignment is not sufficient. (e.g. extern int gVar = 0;)

// Contains data such as number of frames rendered, memory usage, etc.
extern GAMEPERFDATA gPerformanceData;

// The "drawing surface" which we blit to the screen once per frame, 60 times per second.
extern GAMEBITMAP gBackBuffer;

// The smallest, most basic font.
extern GAMEBITMAP g6x7Font;

// Battle scenes are the 96x96 pictures that serve as the backdrop
// for... battle scenes. We draw a monster over the top of the battle scene.
extern GAMEBITMAP gBattleScene_Grasslands01;

extern GAMEBITMAP gBattleScene_Dungeon01;

extern GAMEBITMAP gMonsterSprite_Slime_001;

extern GAMEBITMAP gMonsterSprite_Rat_001;

// Consists of both the massive overworld bitmap and also the tilemap.
extern GAMEMAP gOverworld01;

// Used to track the current and previous game states. The game can only 
// be in one gamestate at a time.
extern GAMESTATE gCurrentGameState;

extern GAMESTATE gPreviousGameState;

// Holds the current state of all keyboard and gamepad input, as well as the 
// previous state of input from the last frame. This is used to tell whether
// the player is holding the button(s) down or not.
extern GAMEINPUT gGameInput;

// Noises and music.
extern GAMESOUND gSoundSplashScreen;

extern GAMESOUND gSoundMenuNavigate;

extern GAMESOUND gSoundMenuChoose;

extern GAMESOUND gMusicOverworld01;

extern GAMESOUND gMusicDungeon01;

extern GAMESOUND gMusicBattle01;

extern GAMESOUND gMusicBattleIntro01;

// Yours truly.
extern HERO gPlayer;

// Sound effects and music volume.
extern float gSFXVolume;

extern float gMusicVolume;

extern BOOL gMusicIsPaused;

// This will be -1 if there is no gamepad plugged in. It will
// be 0 if a gamepad is plugged into the first port.
extern int8_t gGamepadID;

// A global handle to the game window.
extern HWND gGameWindow;                   

// COM interfaces for the XAudio2 library.
// Having 4 source voices for sound effects means we can play up to 
// 4 sound effects simultaneously.
extern IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES];

// We only need 1 source voice for music because we never play overlapping music tracks.
extern IXAudio2SourceVoice* gXAudioMusicSourceVoice;

// These are tiles that the player can walk normally on. e.g. NOT water or lava or walls.
extern uint8_t gPassableTiles[3];

// Imagine the camera is 50 feet up the sky looking straight down over the player.
// Knowing the position of the camera is necessary to properly pan the overworld map as
// the player walks.
extern UPOINT gCamera;

// The background thread that loads assets from the compressed archive during the splash screen.
extern HANDLE gAssetLoadingThreadHandle;

// Input is briefly disabled during scene transitions, and also when the main game 
// window is out of focus.
extern BOOL gInputEnabled;

// This event gets signalled/set after the most essential assets have been loaded.
// "Essential" means the assets required to render the splash screen, which is
// the basic font and the weird noise that plays at the beginning of the splash screen.
extern HANDLE gEssentialAssetsLoadedEvent;

// Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain.
extern BOOL gGameIsRunning;

/////////// END GLOBAL VARIABLE DECLARATIONS ///////////

/////////// BEGIN FUNCTION DELCARATIONS /////////////

// Imported from Ntdll.dll, this is for using the undocumented Windows API 
// function NtQueryTimerResolution.
typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

extern _NtQueryTimerResolution NtQueryTimerResolution;

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);

DWORD CreateMainGameWindow(void);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void ResetEverythingForNewGame(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t x, _In_ int16_t y, _In_ int16_t BrightnessAdjustment);

void BlitBackgroundToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t BrightnessAdjustment);

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

void PlayGameMusic(_In_ GAMESOUND* GameSound, _In_ BOOL Loop, _In_ BOOL Immediate);

void PauseMusic(void);

void StopMusic(void);

BOOL MusicIsPlaying(void);

DWORD LoadAssetFromArchive(_In_ char* ArchiveName, _In_ char* AssetFileName, _In_ RESOURCE_TYPE ResourceType, _Inout_ void* Resource);

DWORD WINAPI AssetLoadingThreadProc(_In_ LPVOID lpParam);

// If WINDOW_FLAG_HORIZONTALLY_CENTERED is specified, the x coordinate is ignored and may be zero.
// If WINDOW_FLAG_VERTICALLY_CENTERED is specified, the y coordinate is ignored and may be zero.
// BackgroundColor is ignored and may be NULL if WINDOW_FLAG_OPAQUE is not set.
// BorderColor is ignored and may be NULL if WINDOW_FLAG_BORDERED is not set.
// Either the BORDERED or the OPAQUE flag needs to be set, or both, or else the window would just be
// transparent and invisible. The window border will cut into the inside of the window area.

void DrawWindow(
	_In_opt_ uint16_t x,
	_In_opt_ uint16_t y,
	_In_ int16_t Width,
	_In_ int16_t Height,
	_In_opt_ PIXEL32* BorderColor,
	_In_opt_ PIXEL32* BackgroundColor,
	_In_opt_ PIXEL32* ShadowColor,
	_In_ DWORD Flags);

void ApplyFadeIn(
	_In_ uint64_t FrameCounter, 
	_In_ PIXEL32 DefaultTextColor, 
	_Inout_ PIXEL32* TextColor, 
	_Inout_opt_ int16_t* BrightnessAdjustment);

int64_t FileSizeA(_In_ const char* FileName);