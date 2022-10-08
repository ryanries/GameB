#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVX2__

// AVX (Advanced Vector Extensions)
// This advanced instruction set began appearing in CPUs circa 2013-2015.
#include <immintrin.h>

#endif

// Maps the tiles in our overworld map to their numerical equivalents
// so we can decide which tiles we're allowed to walk on and which
// tiles block our player's movement, hurt or slow the player, etc.
#include "Tiles.h"

#ifdef _WIN32

// Disable warning about structure padding.
#pragma warning(disable: 4820)

// Disable warning about Spectre/Meltdown CPU vulnerability.
#pragma warning(disable: 5045)

// Disable warning about function not inlined.
#pragma warning(disable: 4710)

// Disable warning about function being inlined.
#pragma warning(disable: 4711)

// Intrinsics such as __ud2.
#include <intrin.h>

// The Windows API.
#include <Windows.h>

#define OPENGL

// OpenGL 1.1, like it's 1996!
#ifdef OPENGL

#include <gl/GL.h>

#pragma comment(lib, "OpenGL32.lib")

#endif

// Audio library.
#include <xaudio2.h>

// Audio library.
#pragma comment(lib, "XAudio2.lib")

// Xbox 360 gamepad input
#include <Xinput.h>

// Xbox 360 gamepad input.
#pragma comment(lib, "XInput.lib")

// Process Status API, e.g. GetProcessMemoryInfo
#include <psapi.h>

// Windows Multimedia library, we use it for timeBeginPeriod 
// to adjust the global system timer resolution.
#pragma comment(lib, "Winmm.lib")

// Imported from Ntdll.dll, this is for using the undocumented Windows API 
// function NtQueryTimerResolution.
typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);

#else

/*
* Linux does not have this function (case insensitve string comapare).
* It is also used for comparing file names which are case sensitive in Linux.
* Use the case sensitive string compare instead.
*/
#define _stricmp(a, b) strcmp(a, b)

// Linux does not have memcpy_s.  Use memcpy for now.
#define memcpy_s(dst, dst_size, src, count) memcpy(dst, src, count)

// Linux does not have strncpy_s.  Use custom function.
#define strncpy_s(dst, dst_size, src, count) safe_strncpy(dst, dst_size, src, count)

// Linux does not have sprintf_s but snprintf is fairly safe.
#define sprintf_s(...) snprintf(__VA_ARGS__)

// Another non-standard function.  Use snprintf instead.  This assumes base is always 10.
#define _itoa_s(value, buffer, size, base) snprintf(buffer, size, "%d", (int)(value))

// These macros are not found in Linux either so define them here.
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define _countof(array) (sizeof(array) / sizeof(array[0]))

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3
#define BI_JPEG 4
#define BI_PNG 5

#define WAVE_FORMAT_PCM 1

#define ERROR_SUCCESS 0
#define ERROR_FILE_INVALID 1
#define ERROR_DATATYPE_MISMATCH 2
#define ERROR_INVALID_DATA 3
#define ERROR_BAD_COMPRESSION_BUFFER 4
#define ERROR_OUTOFMEMORY 5
#define ERROR_FILE_NOT_FOUND 6

typedef struct BITMAPINFOHEADER
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t  biPlanes;
    uint16_t  biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;

typedef struct RGBQUAD
{
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

typedef struct BITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD bmiColors[1];
} BITMAPINFO;

typedef struct WAVEFORMATEX
{
    uint16_t  wFormatTag;
    uint16_t  nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t  nBlockAlign;
    uint16_t  wBitsPerSample;
    uint16_t  cbSize;
} WAVEFORMATEX;

typedef struct POINT
{
    int x;
    int y;
} POINT;

typedef struct RECT
{
    int left;
    int top;
    int right;
    int bottom;
} RECT;

#define XAUDIO2_END_OF_STREAM 0 // Not used but gets set in Flags in XAUDIO2_BUFFER

// Only things used in Linux are AudioBytes and pAudioData.
// Just defining this here to avoid #ifdefs whenever we're dealing with GAMESOUND
typedef struct XAUDIO2_BUFFER
{
	uint32_t Flags;
	uint32_t AudioBytes;
	const uint8_t *pAudioData;
	uint32_t PlayBegin;
	uint32_t PlayLength;
	uint32_t LoopBegin;
	uint32_t LoopLength;
	uint32_t LoopCount;
	void *pContext;
} XAUDIO2_BUFFER;

#endif

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

	WINDOW_FLAG_THICK = 128,				// 1 << 7, 0b10000000

	WINDOW_FLAG_ALPHABLEND = 256			// 1 << 8, 0b1 00000000

} WINDOW_FLAGS;

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

typedef enum LOGLEVEL
{
	LL_NONE    = 0,

	LL_ERROR   = 1,	

	LL_WARNING = 2,

	LL_INFO    = 3,

	LL_DEBUG   = 4

} LOGLEVEL;

// Which direction is the character facing?
// This is used to index into an array to provide walking animations.
typedef enum DIRECTION
{
	DOWN  = 0,

	LEFT  = 3,

	RIGHT = 6,

	UP    = 9

} DIRECTION;

typedef enum ITEMCLASS
{
	ITEMCLASS_WEAPON,

	ITEMCLASS_ARMOR,

	ITEMCLASS_POTION

} ITEMCLASS;

// GAMEBITMAP is any sort of bitmap, which might be a sprite, or a background, 
// or a font sheet, or even the back buffer itself.
typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;

	void* Memory;

} GAMEBITMAP;

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

	int16_t InvKeyIsDown;

	int16_t DebugKeyWasDown;

	int16_t EscapeKeyWasDown;

	int16_t LeftKeyWasDown;

	int16_t RightKeyWasDown;

	int16_t UpKeyWasDown;

	int16_t DownKeyWasDown;

	int16_t ChooseKeyWasDown;

	int16_t InvKeyWasDown;

} GAMEINPUT;

// Miscellaneous statistics regarding hardware specs, program performance, etc.
typedef struct GAMEPERFDATA
{
	uint64_t TotalFramesRendered;

	float RawFPSAverage;

    float CookedFPSAverage;

	bool DisplayDebugInfo;

    uint32_t MinimumTimerResolution;

    uint32_t MaximumTimerResolution;

    uint32_t CurrentTimerResolution;

	uint32_t HandleCount;

    double CPUPercent;

	uint8_t MaxScaleFactor;

	uint8_t CurrentScaleFactor;

	size_t MemoryUsage;

	int MonitorWidth;

    int MonitorHeight;

} GAMEPERFDATA;

// Both players and NPCs have inventories, which is just an array of inventory items.
typedef struct ITEM
{
	char Name[24];

	int Id;

	ITEMCLASS Class;

	char Description[256];

	int Value;

	int Damage;

	int Defense;

} ITEM;

// Damage and cost should scale with the intelligence stat
typedef struct SPELL
{
	int Id; 

	char Name[24];

	int Damage;

	int Cost;

} SPELL;

// This is the player!
typedef struct HERO
{
	char Name[9];

	bool Active;

	GAMEBITMAP Sprite[3][12];	// 3 suits of armor, 12 sprites for each armor set.	

	POINT ScreenPos;			// Note screen position and world position are two different things.

	POINT WorldPos;

	// This will be non-zero when the player is in motion. If 0, player is standing still.
	unsigned int MovementRemaining;

	// Is the player facing left, right, up or down?
	DIRECTION Direction;

	// This is used to allow the player to stand on a portal (or doorway, or stairs, etc.,) 
	// and teleport to the other end of the portal and stand on the portal without 
	// instantly being teleported back because they're currently standing on a portal.
	bool HasPlayerMovedSincePortal;	

	// SUIT_0, SUIT_1, or SUIT_2
	unsigned int CurrentArmor;		

	unsigned int SpriteIndex;	

	unsigned int StepsTaken;

	// 90 = 10% chance, 80 = 20% chance, etc. 100 = 0% chance.
	unsigned int RandomEncounterPercentage;

	uint64_t StepsSinceLastRandomMonsterEncounter;
	
	int MaxHP;

	int HP;

	int Money;

	int MP;				// Magic points (start the game with no magic)

	int XP;				// How much physical damage you deal

	int Strength;		// How much physical damage you deal

	int Dexterity;		// Chance to hit with weapons

	int Luck;			// Stat that adds a little something extra

	int Intelligence;	// Spell damage and chance to hit

	int Evasion;		// Lowers enemy's chance to hit you, improves your Run chance

	int Defense;		// Damage mitigation

	ITEM Inventory[24];

	SPELL Spells[24];

} HERO;

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

// Every MENU in the game is a collection of MENUITEMs.
// This includes everything from simple Yes/No choices, 
// to character naming dialogs, to vendor menus, etc.
typedef struct MENUITEM
{
	char* Name;

	int16_t x;

	int16_t y;

	bool Enabled;

	void(*Action)(void);

} MENUITEM;

typedef struct MENU
{
	char* Name;

	uint8_t SelectedItem;

	uint8_t ItemCount;

	MENUITEM** Items;	

} MENU;

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

	uint32_t Bytes;

} PIXEL32;

typedef enum BLIT_FLAGS
{
	BLIT_FLAG_ALPHABLEND = 1,

	BLIT_FLAG_TEXT_SHADOW = 2

} BLIT_FLAGS;

// If smooth fades are enabled, the menu transitions will look nicer. Otherwise, we will use more "clunky" fade-ins,
// the kind of fade-ins that are more aesthetically reminiscent of the kind you might see on the classic NES or any
// old system with a more limited color palette. We will have to decide later which we prefer -- 
// the kind that look nicer, or the kind that are more "retro."
#define SMOOTH_FADES

#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP 32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define ASSET_FILE				"Assets.dat"
#define GAME_NAME				"Game_B"
#define GAME_VER				"0.9a"
#define LOG_FILE_NAME GAME_NAME ".log"

// 16.67 milliseconds is 60 frames per second.
#define TARGET_MICROSECONDS_PER_FRAME		16667ULL
#define TARGET_NANOSECONDS_PER_FRAME        (TARGET_MICROSECONDS_PER_FRAME * 1000)

// Every 120 frames/2 seconds we will calculate some statistics such as average FPS, CPU usage, etc.
#define CALCULATE_STATS_EVERY_X_FRAMES		120

// Every font sheet we use in this game must have the same format,
// which currently is just all 98 ASCII characters in a single row.
#define FONT_SHEET_CHARACTERS_PER_ROW 98

// This allows us to play up to 4 sound effects simultaneously
// using the XAudio2 library. 
#define NUMBER_OF_SFX_SOURCE_VOICES			4

// We have to temporarily disable input when transitioning from one menu to another, from one gamestate to another,
// but if we don't reenable the input rather quickly after that, it will not feel good for the player.
#define REENABLE_INPUT_AFTER_X_FRAMES_DELAY	10

// The 0th inventory slot will always hold the player's equipped armor.
// The 1st inventory slot will always hold the player's equipped weapon.
// The 2nd inventory slot will always hold the player's equipped shield.
// When the player gets a better item, it will be swapped out with another inventory slot.
#define EQUIPPED_ARMOR	0

#define EQUIPPED_WEAPON	1

#define EQUIPPED_SHIELD	2

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

// The number of steps the player must take after the previous random monster
// encounter before another random monster encounter can take place.
#define RANDOM_MONSTER_GRACE_PERIOD_STEPS 3

#define COLOR_NES_BLACK	(PIXEL32) { .Bytes = 0xFF000000 }

#ifdef _DEBUG

#if defined CLANG || defined __GNUC__

#define ASSERT(Expression, Message) if (!(Expression)) { __builtin_trap(); }

#else

#define ASSERT(Expression, Message) if (!(Expression)) { __ud2(); }

#endif

#else

#define ASSERT(Expression, Message) ((void)0);

#endif

// The "drawing surface" which we blit to the screen once per frame, 60 times per second.
extern GAMEBITMAP gBackBuffer;

// Input is briefly disabled during scene transitions, and also when the main game 
// window is out of focus.
extern bool gInputEnabled;

// The smallest, most basic font.
extern GAMEBITMAP g6x7Font;

extern GAMEBITMAP gPolePigLogo;

extern GAMEBITMAP gLightning01;

extern GAMEPERFDATA gPerformanceData;

// Noises and music.
extern GAMESOUND gSoundSplashScreen;

extern GAMESOUND gSoundMenuNavigate;

extern GAMESOUND gSoundMenuChoose;

extern GAMESOUND gMusicOverworld01;

extern GAMESOUND gMusicDungeon01;

extern GAMESOUND gMusicBattle01;

extern GAMESOUND gMusicBattleIntro01;

extern GAMESOUND gSoundHit01;

extern GAMESOUND gSoundMiss01;

extern GAMESOUND gMusicVictoryIntro;

extern GAMESOUND gMusicVictoryLoop;

// Holds the current state of all keyboard and gamepad input, as well as the 
// previous state of input from the last frame. This is used to tell whether
// the player is holding the button(s) down or not.
extern GAMEINPUT gGameInput;

// Used to track the current and previous game states. The game can only 
// be in one gamestate at a time.
extern GAMESTATE gCurrentGameState;

extern GAMESTATE gPreviousGameState;

extern HERO gPlayer;

// Consists of both the massive overworld bitmap and also the tilemap.
extern GAMEMAP gOverworld01;

// Battle scenes are the 96x96 pictures that serve as the backdrop
// for... battle scenes. We draw a monster over the top of the battle scene.
extern GAMEBITMAP gBattleScene_Grasslands01;

extern GAMEBITMAP gBattleScene_Dungeon01;

extern GAMEBITMAP gMonsterSprite_Slime_001;

extern GAMEBITMAP gMonsterSprite_Rat_001;

// Imagine the camera is 50 feet up the sky looking straight down over the player.
// Knowing the position of the camera is necessary to properly pan the overworld map as
// the player walks.
extern POINT gCamera;

// These are tiles that the player can walk normally on. e.g. NOT water or lava or walls.
extern uint8_t gPassableTiles[11];

// Sound effects and music volume.
extern float gSFXVolume;

extern float gMusicVolume;

extern int gGamepadID;

#ifdef _WIN32

// Set this to FALSE to exit the game immediately. This controls the main game loop in WinMain
extern BOOL gGameIsRunning;

#endif

void DrawDebugInfo(void);
uint32_t LoadEssentialAssets(void);
uint32_t LoadNonEssentialAssets(void);
void BlitBackground(GAMEBITMAP* GameBitmap, int ColorAdjust);
void DrawPlayerStatsWindow(int AlphaAdjust, int WindowShakeX, int WindowShakeY);
void ResetEverythingForNewGame(void);

void Blit32BppBitmapEx(
    GAMEBITMAP* GameBitmap, 
     int x,
    int y,
    int BlueAdjust,
    int GreenAdjust,
    int RedAdjust,
    int AlphaAdjust,
    uint32_t Flags);

void BlitStringEx(
    char* String,
    GAMEBITMAP* FontSheet,
    int x,
    int y,
    int BlueAdjust,
    int GreenAdjust,
    int RedAdjust,
    int AlphaAdjust,
    uint32_t Flags);

void DrawWindow(
    int x,
    int y,
    int Width,
    int Height,
    PIXEL32* BorderColor,
    PIXEL32* BackgroundColor,
    PIXEL32* ShadowColor,
    uint32_t Flags);
